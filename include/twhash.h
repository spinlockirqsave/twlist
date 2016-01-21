/// @file	twhash.c
/// @brief	Hashtable.
/// @details	Based on linux kernel hashtable.
/// @author	Piotr Gregor piotrek.gregor at gmail.com
/// @version	0.1.1
/// @date	30 Dec 2015 11:12 AM
/// @copyright	LGPLv2.1


#ifndef TWHASHTABLE_H
#define TWHASHTABLE_H


#include "twlist.h"


#include <stdint.h>


// TODO handle n < 0 case
#define twilog2(n)	\
(			\
	(	\
	(n) & (1ULL << 63) ? 63 :	\
	(n) & (1ULL << 62) ? 62 :	\
	(n) & (1ULL << 61) ? 61 :	\
	(n) & (1ULL << 60) ? 60 :	\
	(n) & (1ULL << 59) ? 59 :	\
	(n) & (1ULL << 58) ? 58 :	\
	(n) & (1ULL << 57) ? 57 :	\
	(n) & (1ULL << 56) ? 56 :	\
	(n) & (1ULL << 55) ? 55 :	\
	(n) & (1ULL << 54) ? 54 :	\
	(n) & (1ULL << 53) ? 53 :	\
	(n) & (1ULL << 52) ? 52 :	\
	(n) & (1ULL << 51) ? 51 :	\
	(n) & (1ULL << 50) ? 50 :	\
	(n) & (1ULL << 49) ? 49 :	\
	(n) & (1ULL << 48) ? 48 :	\
	(n) & (1ULL << 47) ? 47 :	\
	(n) & (1ULL << 46) ? 46 :	\
	(n) & (1ULL << 45) ? 45 :	\
	(n) & (1ULL << 44) ? 44 :	\
	(n) & (1ULL << 43) ? 43 :	\
	(n) & (1ULL << 42) ? 42 :	\
	(n) & (1ULL << 41) ? 41 :	\
	(n) & (1ULL << 40) ? 40 :	\
	(n) & (1ULL << 39) ? 39 :	\
	(n) & (1ULL << 38) ? 38 :	\
	(n) & (1ULL << 37) ? 37 :	\
	(n) & (1ULL << 36) ? 36 :	\
	(n) & (1ULL << 35) ? 35 :	\
	(n) & (1ULL << 34) ? 34 :	\
	(n) & (1ULL << 33) ? 33 :	\
	(n) & (1ULL << 32) ? 32 :	\
	(n) & (1ULL << 31) ? 31 :	\
	(n) & (1ULL << 30) ? 30 :	\
	(n) & (1ULL << 29) ? 29 :	\
	(n) & (1ULL << 28) ? 28 :	\
	(n) & (1ULL << 27) ? 27 :	\
	(n) & (1ULL << 26) ? 26 :	\
	(n) & (1ULL << 25) ? 25 :	\
	(n) & (1ULL << 24) ? 24 :	\
	(n) & (1ULL << 23) ? 23 :	\
	(n) & (1ULL << 22) ? 22 :	\
	(n) & (1ULL << 21) ? 21 :	\
	(n) & (1ULL << 20) ? 20 :	\
	(n) & (1ULL << 19) ? 19 :	\
	(n) & (1ULL << 18) ? 18 :	\
	(n) & (1ULL << 17) ? 17 :	\
	(n) & (1ULL << 16) ? 16 :	\
	(n) & (1ULL << 15) ? 15 :	\
	(n) & (1ULL << 14) ? 14 :	\
	(n) & (1ULL << 13) ? 13 :	\
	(n) & (1ULL << 12) ? 12 :	\
	(n) & (1ULL << 11) ? 11 :	\
	(n) & (1ULL << 10) ? 10 :	\
	(n) & (1ULL <<  9) ?  9 :	\
	(n) & (1ULL <<  8) ?  8 :	\
	(n) & (1ULL <<  7) ?  7 :	\
	(n) & (1ULL <<  6) ?  6 :	\
	(n) & (1ULL <<  5) ?  5 :	\
	(n) & (1ULL <<  4) ?  4 :	\
	(n) & (1ULL <<  3) ?  3 :	\
	(n) & (1ULL <<  2) ?  2 :	\
	(n) & (1ULL <<  1) ?  1 :	\
	0 				\
	)				\
 )

// 2^31 + 2^29 - 2^25 + 2^22 - 2^19 - 2^16 + 1
#define GOLDEN_RATIO_PRIME_32 0x9e370001UL

//  2^63 + 2^61 - 2^57 + 2^54 - 2^51 - 2^18 + 1
#define GOLDEN_RATIO_PRIME_64 0x9e37fffffffc0001UL

#define TWBITS_PER_LONG 32 
#if ULONG_MAX > UINT_MAX
#define TWBITS_PER_LONG 64
#endif
#if TWBITS_PER_LONG == 32
#define TWGOLDEN_RATIO_PRIME GOLDEN_RATIO_PRIME_32
#define twhash_long(val, bits) twhash_32(val, bits)
#elif TWBITS_PER_LONG == 64
#define twhash_long(val, bits) twhash_64(val, bits)
#define TWGOLDEN_RATIO_PRIME GOLDEN_RATIO_PRIME_64
#else
#error Wordsize not 32 or 64 TWBITS_PER_LONG
#endif

inline static uint64_t
twhash_64(uint64_t val, unsigned int bits)
{
	uint64_t hash = val;
	uint64_t n = hash;
	n <<= 18;
	hash -= n;
	n <<= 33;
	hash -= n;
	n <<= 3;
	hash += n;
	n <<= 3;
	hash -= n;
	n <<= 4;
	hash += n;
	n <<= 2;
	hash += n;

	// High bits are more random, so use them.
	return hash >> (64 - bits);
}

static inline uint32_t
twhash_32(uint32_t val, unsigned int bits)
{
	// On some cpus multiply is faster, on others gcc will do shifts
	uint32_t hash = val * GOLDEN_RATIO_PRIME_32;

	// High bits are more random, so use them.
	return hash >> (32 - bits);
}

static inline unsigned long
twhash_ptr(const void *ptr, unsigned int bits)
{
	return twhash_long((unsigned long)ptr, bits);
}

static inline uint32_t
twhash32_ptr(const void *ptr)
{
	unsigned long val = (unsigned long) ptr;

#if TWBITS_PER_LONG == 64
	val ^= (val >> 32);
#endif
	return (uint32_t) val;
}

#define TWARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

#define TWDEFINE_HASHTABLE(name, bits)				\
	struct twhlist_head name[1 << (bits)] =			\
		{ [0 ... ((1 << (bits)) - 1)] = TWHLIST_HEAD_INIT }

#define TWDECLARE_HASHTABLE(name, bits)	\
	struct twhlist_head name[1 << (bits)]

#define TWHASH_SIZE(name) (TWARRAY_SIZE(name))
#define TWHASH_BITS(name) twilog2(TWHASH_SIZE(name))

// Use twhash_32 when possible to allow for fast
// 32bit hashing in 64bit kernels.
#define twhash_min(val, bits)							\
	(sizeof(val) <= 4 ? twhash_32(val, bits) \
	 	: twhash_long(val, bits))

static inline void
__twhash_init(struct twhlist_head *ht, unsigned int sz)
{
	unsigned int i;

	for (i = 0; i < sz; i++)
		TWINIT_HLIST_HEAD(&ht[i]);
}

/// @brief	Initialize a hashtable.
/// @details	Calculates the size of the hashtable from the given parameter, otherwise
///		same as twhash_init_size.
///		This has to be a macro since HASH_BITS() will not work on pointers since
///		it calculates the size during preprocessing.
///		@twhashtable: hashtable to be initialized
#define twhash_init(hashtable) \
	__twhash_init(hashtable, TWHASH_SIZE(hashtable))

/// @brief	Add an object to a hashtable.
///		@hashtable: hashtable to add to
///		@node: the &struct twhlist_node of the object to be added
///		@key: the key of the object to be added
#define twhash_add(hashtable, node, key)	\
	twhlist_add_head(node, \
		&hashtable[twhash_min(key, TWHASH_BITS(hashtable))])

#define twhash_add_bits(hashtable, node, key, bits)	\
	twhlist_add_head(node, \
		&hashtable[twhash_min(key, bits)])

/// @brief	Check whether an object is in any hashtable.
///		@node: the &struct twhlist_node of the object to be checked
static inline int
twhash_hashed(struct twhlist_node *node)
{
	return !twhlist_unhashed(node);
}

static inline int
__twhash_empty(struct twhlist_head *ht, unsigned int sz)
{
	unsigned int i;

	for (i = 0; i < sz; i++)
		if (!twhlist_empty(&ht[i]))
			return -1;

	return 0;
}

/// @brief	Check whether a twhashtable is empty.
/// @details	This has to be a macro since HASH_BITS() will not work on pointers since
///		it calculates the size during preprocessing.
///		@hashtable: hashtable to check
#define twhash_empty(hashtable) \
	__twhash_empty(hashtable, TWHASH_SIZE(hashtable))

/// @brief	Remove an object from a hashtable.
///		@node: &struct twhlist_node of the object to remove
static inline void
twhash_del(struct twhlist_node *node)
{
		twhlist_del_init(node);
}

/// @brief	Iterate over a hashtable.
///		@name: hashtable to iterate
///		@bkt: integer to use as bucket loop cursor
///		@obj: the type * to use as a loop cursor for each entry
///		@member: the name of the twhlist_node within the struct
#define twhash_for_each(name, bkt, obj, member)			\
	for ((bkt) = 0, obj = NULL; \
		obj == NULL && (bkt) < TWHASH_SIZE(name); (bkt)++) \
			twhlist_for_each_entry(obj, &name[bkt], member)

/// @brief	Iterate over a hashtable safe against removal of
///		hash entry.
///		@name: hashtable to iterate
///		@bkt: integer to use as bucket loop cursor
///		@tmp: a &struct used for temporary storage
///		@obj: the type * to use as a loop cursor for each entry
///		@member: the name of the twhlist_node within the struct
#define twhash_for_each_safe(name, bkt, tmp, obj, member)			\
	for ((bkt) = 0, obj = NULL; obj == NULL && (bkt) < TWHASH_SIZE(name);\
							(bkt)++)\
		twhlist_for_each_entry_safe(obj, tmp, &name[bkt], member)

/// @brief	Iterate over all possible objects hashing to the
///		same bucket.
///		@name: hashtable to iterate
///		@obj: the type * to use as a loop cursor for each entry
///		@member: the name of the twhlist_node within the struct
///		@key: the key of the objects to iterate over
#define twhash_for_each_possible(name, obj, member, key)			\
	twhlist_for_each_entry(obj, \
		&name[twhash_min(key, TWHASH_BITS(name))], member)

/// @brief	Iterate over all possible objects hashing to the
///		same bucket safe against removals.
///		@name: hashtable to iterate
///		@obj: the type * to use as a loop cursor for each entry
///		@tmp: a &struct used for temporary storage
///		@member: the name of the twhlist_node within the struct
///		@key: the key of the objects to iterate over
#define twhash_for_each_possible_safe(name, obj, tmp, member, key)	\
	twhlist_for_each_entry_safe(obj, tmp,\
		&name[twhash_min(key, TWHASH_BITS(name))], member)


#endif	// TWHASHTBALE_H
