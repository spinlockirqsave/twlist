///  @file      twtest.c
///  @brief     Test suite for twlist.h and twhash.h.
///  @author    peterg
///  @version   0.1.1
///  @date      30 Dec 2015 11:12 AM
///  @copyright GPL2


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
twlist_test_list_creation(void)
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
twlist_test(void)
{
	twlist_test_list_creation();
	twlist_test_list_add();
}

int
main(void)
{
	// test twlist
	twlist_test();

	return 0;
}
