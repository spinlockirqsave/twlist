/// @file      twtest.c
/// @brief     Test suite for twlist.h and twhash.h.
/// @author    peterg
/// @version   0.1.1
/// @date      30 Dec 2015 11:12 AM
/// @copyright LGPL


#include "twlist.h"		// list
#include "twhash.h"		// hash, hashtable


#include <stdlib.h>             // everything
#include <stdio.h>              // most I/O
#include <sys/types.h>          // syscalls
#include <string.h>             // memset, etc.
#include <errno.h>		// error codes
#include <assert.h>		// assertion


// our test struct
struct gucio
{
	uint32_t	key;
	int		val;
	struct twlist_head	link;
	struct twhlist_head	hlink;
};

static void
twlist_test_list_head_init(void)
{
	struct twlist_head	h;

	TWINIT_LIST_HEAD(&h);
	assert(h.next == h.prev);
	assert(h.prev == &h);
}

static void
twlist_test_list_add(void)
{
	struct twlist_head	h;
	struct gucio		g;

	TWINIT_LIST_HEAD(&h);
	twlist_add(&g.link, &h);
	assert(h.next == &g.link);
	assert(h.prev == &g.link);
	assert(g.link.next == &h);
	assert(g.link.prev == &h);
}

static void
twlist_test_list_add_tail(void)
{
	struct twlist_head	h;
	struct gucio		g;

	TWINIT_LIST_HEAD(&h);
	twlist_add_tail(&g.link, &h);
	assert(h.next == &g.link);
	assert(h.prev == &g.link);
	assert(g.link.next == &h);
	assert(g.link.prev == &h);
}


static void
twlist_test_list_del(void)
{
	struct twlist_head	h, i;
	struct gucio		g;

	TWINIT_LIST_HEAD(&h);
	twlist_del(&h);
	assert(h.next == TWLIST_POISON1);
	assert(h.prev == TWLIST_POISON2);
	TWINIT_LIST_HEAD(&i);
	twlist_add_tail(&g.link, &i);
	assert(i.next == &g.link);
	assert(i.prev == &g.link);
	assert(g.link.next == &i);
	assert(g.link.prev == &i);
	twlist_del(&g.link);
	assert(i.next == i.prev);
	assert(i.prev == &i);
	assert(g.link.next == TWLIST_POISON1);
	assert(g.link.prev == TWLIST_POISON2);
}

static void
twlist_test_list_del_init(void)
{
	struct twlist_head	h;
	struct gucio		g;

	TWINIT_LIST_HEAD(&h);
	twlist_add_tail(&g.link, &h);
	twlist_del_init(&g.link);
	assert(h.next == h.prev);
	assert(h.prev == &h);
	assert(g.link.next == g.link.prev);
	assert(g.link.prev == &g.link);
}
static void
twlist_test(void)
{
	twlist_test_list_head_init();
	twlist_test_list_add();
	twlist_test_list_add_tail();
	twlist_test_list_del();
	twlist_test_list_del_init();
}

int
main(void)
{
	// test twlist
	twlist_test();

	return 0;
}
