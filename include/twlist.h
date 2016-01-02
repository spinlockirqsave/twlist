/// @file	twlist.h
/// @brief	twlist - doubly linked list with double pointer list head,
///		twhlist - doubly linked list with single pointer list head.
/// @author	Piotr Gregor piotrek.gregor at gmail.com
/// @version	0.1.1
/// @date	30 Dec 2015 11:12 AM
/// @copyright	LGPL


#ifndef TWLIST_H
#define TWLIST_H


#include <stdlib.h>             // everything


#define tw_container_of(ptr, type, member) ({                      \
        const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
        (type *)( (char *)__mptr - offsetof(type,member) );})


// move the poison pointer offset into some well-recognized area
// such as 0xdead000000000000
# define POISON_POINTER_DELTA 0

// These are non-NULL pointers that will result in page faults
#define TWLIST_POISON1  ((void *) 0x00100100 + POISON_POINTER_DELTA)
#define TWLIST_POISON2  ((void *) 0x00200200 + POISON_POINTER_DELTA)
 
#ifndef CONFIG_DEBUG_LIST
struct twlist_head
{
	struct twlist_head	*next, *prev;
};

struct twhlist_head
{
	struct twhlist_node	*first;
};

struct twhlist_node
{
	struct twhlist_node	*next, **pprev;
};
#else
typedef twlist_head_dbg twlist_head;
typedef twhlist_head_dbg twhlist_head;
typedef twhlist_node_dbg twhlist_node;
#endif

// Simple doubly linked list implementation.

#define TWLIST_HEAD_INIT(name) { &(name), &(name) }

#define TWLIST_HEAD(name) \
	struct twlist_head name = TWLIST_HEAD_INIT(name)

static inline void
TWINIT_LIST_HEAD(struct twlist_head *list)
{
	list->next = list;
	list->prev = list;
}

// Insert a new entry between two known consecutive entries.
// This is only for internal list manipulation where we know
// the prev/next entries already!
#ifndef CONFIG_DEBUG_LIST
static inline void
__twlist_add(struct twlist_head *new,
				struct twlist_head *prev,
				struct twlist_head *next)
{
	next->prev = new;
	new->next = next;
	new->prev = prev;
	prev->next = new;
}
#else
extern void
__twlist_add(struct twlist_head *new,
			struct twlist_head *prev,
			struct twlist_head *next);
#endif


/// @brief	Add a new entry after the specified head.
/// 		@new:	new entry to be added
/// 		@head:	list head to add it after
/// @details	This is good for implementing stacks.
static inline void
twlist_add(struct twlist_head *new,
				struct twlist_head *head)
{
		__twlist_add(new, head, head->next);
}


/// @brief	Add a new entry before the specified head.
/// 		@new:	new entry to be added
/// 		@head:	list head to add it before
/// @details	This is useful for implementing queues.
static inline void
twlist_add_tail(struct twlist_head *new,
				struct twlist_head *head)
{
		__twlist_add(new, head->prev, head);
}

/// @brief	Delete a list entry by making the prev/next entries
///		point to each other.
/// @details	This is only for internal list manipulation where we know
///		the prev/next entries already!
static inline void
__twlist_del(struct twlist_head * prev,
				struct twlist_head * next)
{
	next->prev = prev;
	prev->next = next;
}

/// @brief	Deletes entry from list.
/// 		@entry: the element to delete from the list.
///		Note: list_empty() on entry does not return
///		true after this, the entry is in an undefined state.
#ifndef CONFIG_DEBUG_LIST
static inline void
__twlist_del_entry(struct twlist_head *entry)
{
	__twlist_del(entry->prev, entry->next);
}

static inline void
twlist_del(struct twlist_head *entry)
{
	__twlist_del(entry->prev, entry->next);
	entry->next = TWLIST_POISON1;
	entry->prev = TWLIST_POISON2;
}
#else
extern void
__twlist_del_entry(struct twlist_head *entry);
extern void
twlist_del(struct twlist_head *entry);
#endif

/// @brief	Deletes entry from twlist and reinitialize it.
///		@entry: the element to delete from the twlist.
static inline void
twlist_del_init(struct twlist_head *entry)
{
	__twlist_del_entry(entry);
	TWINIT_LIST_HEAD(entry);
}

/// @brief	Delete from one twlist and add as another's head
///		@twlist: the entry to move
///		@head: the head that will precede our entry
static inline void
twlist_move(struct twlist_head *twlist, struct twlist_head *head)
{
	__twlist_del_entry(twlist);
	twlist_add(twlist, head);
}

/// @brief	Delete from one twlist and add as another's tail
///		@twlist: the entry to move
///		@head: the head that will follow our entry
static inline void
twlist_move_tail(struct twlist_head *twlist,
						  struct twlist_head *head)
{
	__twlist_del_entry(twlist);
	twlist_add_tail(twlist, head);
}

/// @brief	Tests whether @list is the last entry in twlist @head
///		@list: the entry to test
///		@head: the head of the twlist which @list is member of
static inline int
twlist_is_last(const struct twlist_head *list,
					const struct twlist_head *head)
{
	return list->next == head;
}

/// @brief	Tests whether a twlist is empty.
///		@head: the twlist to test.
static inline int
twlist_empty(const struct twlist_head *head)
{
	return head->next == head;
}

/// @brief	Tests whether a twlist is empty and not being modified
///		@head: the twlist to test
/// @details	Tests whether a twlist is empty _and_ checks that
///		no other CPU might be in the process of modifying either
///		member (next or prev)
///		NOTE: using twlist_empty_careful() without synchronization
///		can only be safe if the only activity that can happen
///		to the twlist entry is twlist_del_init(). Eg. it
///		cannot be used if another CPU could re-twlist_add() it.
static inline int
twlist_empty_careful(const struct twlist_head *head)
{
		struct twlist_head *next = head->next;
			return (next == head) && (next == head->prev);
}

/// @brief	Rotate the twlist to the left
///		@head: the head of the twlist
static inline void
twlist_rotate_left(struct twlist_head *head)
{
	struct twlist_head *first;
	if (!twlist_empty(head))
	{
		first = head->next;
		twlist_move_tail(first, head);
	}
}

/// @brief	Tests whether a twlist has just one entry.
///		@head: the twlist to test.
static inline int
twlist_is_singular(const struct twlist_head *head)
{
	return !twlist_empty(head) && (head->next == head->prev);
}

static inline void
__twlist_cut_position(struct twlist_head *twlist,
						struct twlist_head *head,
						struct twlist_head *entry)
{
	struct twlist_head *new_first = entry->next;
	twlist->next = head->next;
	twlist->next->prev = twlist;
	twlist->prev = entry;
	entry->next = twlist;
	head->next = new_first;
	new_first->prev = head;
}

/// @brief	Cut a twlist into two.
///		@twlist: a new twlist to add all removed entries
///		@head: a twlist with entries
///		@entry: an entry within head, could be the head itself
///		and if so we won't cut the twlist
///		This helper moves the initial part of @head, up to and
///		including @entry, from @head to @twlist. You should
///		pass on @entry an element you know is on @head. @twlist
///		should be an empty twlist or a twlist you do not
///		care about losing its data.
static inline void twlist_cut_position(struct twlist_head *twlist,
						struct twlist_head *head,
						struct twlist_head *entry)
{
	if (twlist_empty(head))
		return;
	if (twlist_is_singular(head) &&
		(head->next != entry && head != entry))
	return;
	if (entry == head)
		TWINIT_LIST_HEAD(twlist);
	else
		__twlist_cut_position(twlist, head, entry);
}

static inline void __twlist_splice(const struct twlist_head *twlist,
						struct twlist_head *prev,
						struct twlist_head *next)
{
	struct twlist_head *first = twlist->next;
	struct twlist_head *last = twlist->prev;

	first->prev = prev;
	prev->next = first;

	last->next = next;
	next->prev = last;
}

/// @brief	Join two lists, this is designed for stacks.
///		@twlist: the new list to add.
///		@head: the place to add it in the first list.
static inline void twlist_splice(const struct twlist_head *twlist,
						struct twlist_head *head)
{
	if (!twlist_empty(twlist))
		__twlist_splice(twlist, head, head->next);
}

/// @brief	Join two lists, each list being a queue.
///		@twlist: the new list to add.
///		@head: the place to add it in the first list.
static inline void twlist_splice_tail(struct twlist_head *twlist,
						struct twlist_head *head)
{
	if (!twlist_empty(twlist))
		__twlist_splice(twlist, head->prev, head);
}

/// @brief	Join two twlists and reinitialise the emptied list.
///		@twlist: the new list to add.
///		@head: the place to add it in the first list.
/// @details	The twlist at @twlist is reinitialised.
static inline void twlist_splice_init(struct twlist_head *twlist,
						struct twlist_head *head)
{
	if (!twlist_empty(twlist))
	{
		__twlist_splice(twlist, head, head->next);
		TWINIT_LIST_HEAD(twlist);
	}
}

/// @brief	Join two lists and reinitialise the emptied list.
///		@twlist: the new list to add.
///		@head: the place to add it in the first list.
/// @details	Each of the lists is a queue. The list at @twlist
///		is reinitialised
static inline void twlist_splice_tail_init(struct twlist_head *twlist,
						struct twlist_head *head)
{
	if (!twlist_empty(twlist))
	{
		__twlist_splice(twlist, head->prev, head);
		TWINIT_LIST_HEAD(twlist);
	}
}

/// @brief	Get the struct for this entry.
///		@ptr:	the &struct twlist_head pointer.
///		@type:	the type of the struct this is embedded in.
///		@member:	the name of the twlist_head within the struct.
#define twlist_entry(ptr, type, member) \
		tw_container_of(ptr, type, member)

/// @brief	Get the first element from a twlist.
///		@ptr:	the twlist head to take the element from.
///		@type:	the type of the struct this is embedded in.
///		@member:the name of the twlist_head within the struct.
/// @details	Note, that twlist is expected to be not empty.
#define twlist_first_entry(ptr, type, member) \
		twlist_entry((ptr)->next, type, member)

/// @brief	Get the last element from a twlist.
///		@ptr:	the twlist head to take the element from.
///		@type:	the type of the struct this is embedded in.
///		@member:	the name of the twlist_head within the struct.
/// @details	Note, that twlist is expected to be not empty.
#define twlist_last_entry(ptr, type, member) \
		twlist_entry((ptr)->prev, type, member)

/// @brief	Get the first element from a twlist.
///		@ptr:	the twlist head to take the element from.
///		@type:	the type of the struct this is embedded in.
///		@member:	the name of the twlist_head within the struct.
/// @details	Note that if the twlist is empty, it returns NULL.
#define twlist_first_entry_or_null(ptr, type, member) \
		(!twlist_empty(ptr) ? twlist_first_entry(ptr, type, member) : NULL)

/// @brief	Get the next element in twlist.
///		@pos:	the type * to cursor
///		@member:	the name of the twlist_head within the struct.
#define twlist_next_entry(pos, member) \
		twlist_entry((pos)->member.next, typeof(*(pos)), member)

/// @brief	Get the prev element in twlist.
///		@pos:	the type * to cursor
///		@member:	the name of the twlist_head within the struct.
#define twlist_prev_entry(pos, member) \
		twlist_entry((pos)->member.prev, typeof(*(pos)), member)

/// @brief	Iterate over a twlist.
///		@pos:	the &struct twlist_head to use as a loop cursor.
///		@head:	the head for your twlist.
#define twlist_for_each(pos, head) \
		for (pos = (head)->next; pos != (head); pos = pos->next)

/// @brief	Iterate over a twlist backwards.
///		@pos:	the &struct twlist_head to use as a loop cursor.
///		@head:	the head for your twlist.
#define twlist_for_each_prev(pos, head) \
		for (pos = (head)->prev; pos != (head); pos = pos->prev)

/// @brief	Iterate over a twlist safe against removal of twlist entry.
///		@pos:	the &struct twlist_head to use as a loop cursor.
///		@n:	another &struct twlist_head to use as
///			temporary storage
///		@head:	the head for your twlist.
#define twlist_for_each_safe(pos, n, head) \
		for (pos = (head)->next, n = pos->next; pos != (head); \
						pos = n, n = pos->next)

/// @brief	Iterate over a twlist backwards safe against removal
///		of twlist entry.
///		@pos:	the &struct twlist_head to use as a loop cursor
///		@n:	another &struct twlist_head to use as temporary storage
///		@head:	the head for your twlist
#define twlist_for_each_prev_safe(pos, n, head) 			\
		for (pos = (head)->prev, n = pos->prev; 		\
					     pos != (head); 		\
					     pos = n, n = pos->prev)

/// @brief	Iterate over twlist of given type
///		@pos:	the type * to use as a loop cursor
///		@head:	the head for your twlist
///		@member:the name of the twlist_head within the struct
#define twlist_for_each_entry(pos, head, member)				\
		for (pos = twlist_first_entry(head, typeof(*pos), member);	\
					     &pos->member != (head);		\
					     pos = twlist_next_entry(pos, member))

/// @brief	Iterate backwards over twlist of given type.
///		@pos:	the type * to use as a loop cursor
///		@head:	the head for your twlist
///		@member:the name of the twlist_head within the struct
#define twlist_for_each_entry_reverse(pos, head, member)			\
		for (pos = twlist_last_entry(head, typeof(*pos), member);	\
					     &pos->member != (head); 		\
					     pos = twlist_prev_entry(pos, member))

/// @brief	Prepare a pos entry for use in twlist_for_each_entry_continue().
///		@pos:	the type * to use as a start point
///		@head:	the head of the twlist
///		@member:the name of the twlist_head within the struct
/// @details	Prepares a pos entry for use as a start point in
///		twlist_for_each_entry_continue().
#define twlist_prepare_entry(pos, head, member) \
		((pos) ? : twlist_entry(head, typeof(*pos), member))

/// @brief	Continue iteration over twlist of given type.
///		@pos:	the type * to use as a loop cursor
///		@head:	the head for your twlist
///		@member:the name of the twlist_head within the struct
///		Continue to iterate over twlist of given type, continuing after
///		the current position.
#define twlist_for_each_entry_continue(pos, head, member) 			\
		for (pos = twlist_next_entry(pos, member);			\
					     &pos->member != (head);		\
					     pos = twlist_next_entry(pos, member))

/// @brief	Iterate backwards from the given point.
///		@pos:	the type * to use as a loop cursor
///		@head:	the head for your list
///		@member:the name of the twlist_head within the struct
/// @details	Start to iterate over list of given type backwards, continuing after
///		the current position.
#define twlist_for_each_entry_continue_reverse(pos, head, member)		\
		for (pos = twlist_prev_entry(pos, member);			\
					     &pos->member != (head);		\
					     pos = twlist_prev_entry(pos, member))

/// @brief	Iterate over twlist of given type from the current point.
///		@pos:	the type * to use as a loop cursor
///		@head:	the head for your twlist
///		@member:the name of the twlist_head within the struct
/// @details	Iterate over list of given type, continuing from current position.
#define twlist_for_each_entry_from(pos, head, member) 			\
		for (; &pos->member != (head);					\
					     pos = twlist_next_entry(pos, member))

/// @brief	Iterate over twlist of given type safe against removal
///		of twlist entry.
///		@pos:	the type * to use as a loop cursor
///		@n:	another type * to use as temporary storage
///		@head:	the head for your list
///		@member:the name of the twlist_head within the struct.
#define twlist_for_each_entry_safe(pos, n, head, member)			\
		for (pos = twlist_first_entry(head, typeof(*pos), member),	\
			n = twlist_next_entry(pos, member);			\
			&pos->member != (head); 				\
			pos = n, n = twlist_next_entry(n, member))

/// @brief	Continue twlist iteration safe against removal.
///		@pos:	the type * to use as a loop cursor
///		@n:	another type * to use as temporary storage
///		@head:	the head for your twlist
///		@member:the name of the list head within the struct
/// @details	Iterate over twlist of given type, continuing after current point,
///		safe against removal of list entry.
#define twlist_for_each_entry_safe_continue(pos, n, head, member) 		\
		for (pos = twlist_next_entry(pos, member), 			\
			n = twlist_next_entry(pos, member);			\
			&pos->member != (head);					\
			pos = n, n = twlist_next_entry(n, member))

/// @brief	Iterate over twlist from current point safe against removal.
///		@pos:	the type * to use as a loop cursor
///		@n:	another type * to use as temporary storage
///		@head:	the head for your twlist
///		@member:the name of the twlist_head within the struct
/// @details	Iterate over twlist of given type from current point, safe against
///		removal of twlist entry.
#define twlist_for_each_entry_safe_from(pos, n, head, member) 	\
	for (n = twlist_next_entry(pos, member);			\
		&pos->member != (head);					\
		pos = n, n = twlist_next_entry(n, member))

/// @brief	Iterate backwards over twlist safe against removal.
///		@pos:	the type * to use as a loop cursor
///		@n:	another type * to use as temporary storage
///		@head:	the head for your twlist
///		@member:the name of the twlist_head within the struct
/// @details	Iterate backwards over twlist of given type, safe against removal
///		of twlist entry.
#define twlist_for_each_entry_safe_reverse(pos, n, head, member)		\
	for (pos = twlist_last_entry(head, typeof(*pos), member),		\
		n = twlist_prev_entry(pos, member);				\
		&pos->member != (head); 					\
		pos = n, n = twlist_prev_entry(n, member))

/// @brief	Double linked lists with a single pointer list head.
/// @details	Mostly useful for hash tables where the two pointer list
///		head is too wasteful. You lose the ability to access
///		the tail in O(1).

#define TWHLIST_HEAD_INIT { .first = NULL }
#define TWHLIST_HEAD(name) struct twhlist_head name = {  .first = NULL }
#define TWINIT_HLIST_HEAD(ptr) ((ptr)->first = NULL)
static inline void
TWINIT_HLIST_NODE(struct twhlist_node *h)
{
	h->next = NULL;
	h->pprev = NULL;
}

static inline int
twhlist_unhashed(const struct twhlist_node *h)
{
	return !h->pprev;
}

static inline int
twhlist_empty(const struct twhlist_head *h)
{
	return !h->first;
}

static inline void
__twhlist_del(struct twhlist_node *n)
{
	struct twhlist_node *next = n->next;
	struct twhlist_node **pprev = n->pprev;
	*pprev = next;
	if (next)
		next->pprev = pprev;
}

static inline void
twhlist_del(struct twhlist_node *n)
{
	__twhlist_del(n);
	n->next = TWLIST_POISON1;
	n->pprev = TWLIST_POISON2;
}

static inline void
twhlist_del_init(struct twhlist_node *n)
{
	if (!twhlist_unhashed(n))
	{
		__twhlist_del(n);
		TWINIT_HLIST_NODE(n);
	}
}

static inline void
twhlist_add_head(struct twhlist_node *n,
			struct twhlist_head *h)
{
	struct twhlist_node *first = h->first;
	n->next = first;
	if (first)
		first->pprev = &n->next;
	h->first = n;
	n->pprev = &h->first;
}

// next must be != NULL
static inline void
twhlist_add_before(struct twhlist_node *n,
				struct twhlist_node *next)
{
	n->pprev = next->pprev;
	n->next = next;
	next->pprev = &n->next;
	*(n->pprev) = n;
}

static inline void
twhlist_add_after(struct twhlist_node *n,
			struct twhlist_node *next)
{
	next->next = n->next;
	n->next = next;
	next->pprev = &n->next;

	if(next->next)
		next->next->pprev  = &next->next;
}

// after that we'll appear to be on some hlist and hlist_del will work
static inline void
twhlist_add_fake(struct twhlist_node *n)
{
	n->pprev = &n->next;
}

// Move a list from one list head to another. Fixup the pprev
// reference of the first entry if it exists.
static inline void
twhlist_move_list(struct twhlist_head *old,
			struct twhlist_head *new)
{
	new->first = old->first;
	if (new->first)
		new->first->pprev = &new->first;
	old->first = NULL;
}

#define twhlist_entry(ptr, type, member) \
	tw_container_of(ptr,type,member)

#define twhlist_for_each(pos, head) \
	for (pos = (head)->first; pos ; pos = pos->next)

#define twhlist_for_each_safe(pos, n, head) \
	for (pos = (head)->first; pos && ({ n = pos->next; 1; }); \
		pos = n)

#define twhlist_entry_safe(ptr, type, member) \
	({ typeof(ptr) ____ptr = (ptr); \
		____ptr ? twhlist_entry(____ptr, type, member) : NULL; \
	})

/// @brief	Iterate over list of given type.
///		@pos:		the type * to use as a loop cursor.
///		@head:		the head for your list.
///		@member:	the name of the hlist_node within the struct.
#define twhlist_for_each_entry(pos, head, member)				\
	for (pos = twhlist_entry_safe((head)->first, typeof(*(pos)), member);\
		pos;							\
		pos = twhlist_entry_safe((pos)->member.next, 	\
		typeof(*(pos)), member))

/// @brief	Iterate over a twhlist continuing after current point.
///		@pos:		the type * to use as a loop cursor.
///		@member:	the name of the twhlist_node within the struct.
#define twhlist_for_each_entry_continue(pos, member)			\
	for (pos = hlist_entry_safe((pos)->member.next, typeof(*(pos)), member);\
		pos;								\
		pos = twhlist_entry_safe((pos)->member.next, 		\
			typeof(*(pos)), member))

/// @brief	Iterate over a twhlist continuing from current point.
///		@pos:		the type * to use as a loop cursor.
///		@member:	the name of the hlist_node within the struct.
#define twhlist_for_each_entry_from(pos, member)				\
	for (; pos;								\
	pos = twhlist_entry_safe((pos)->member.next, typeof(*(pos)), member))

/// @brief	Iterate over list of given type safe against removal of list entry.
///		@pos:		the type * to use as a loop cursor.
///		@n:		another &struct hlist_node to use as temporary storage
///		@head:		the head for your list.
///		@member:	the name of the hlist_node within the struct.
#define twhlist_for_each_entry_safe(pos, n, head, member) 		\
		for (pos = twhlist_entry_safe((head)->first, typeof(*pos), member);\
			pos && ({ n = pos->member.next; 1; });			\
			pos = twhlist_entry_safe(n, typeof(*pos), member))

/// @brief	First-in, first-out queue.
typedef struct twlist_head twfifo_queue;

static inline void
twfifo_enqueue(struct twlist_head *new,
				twfifo_queue *q)
{
	twlist_add_tail(new, (struct twlist_head *)q);
}

static inline struct twlist_head*
twfifo_dequeue(twfifo_queue *q)
{
	struct twlist_head *l; 
	if (twlist_empty((struct twlist_head *)q))
		return NULL;
	l = q->prev;
	twlist_del(l);
	return l;
}

#define twfifo_dequeue_entry(q, type, member) {(		\
	({ (twlist_empty((struct twlist_head *)q)) ?	\
		NULL;						\
		twlist_last_entry((struct twlist_head *)q,\
					 type, member) })	\

#endif // TWLIST_H
