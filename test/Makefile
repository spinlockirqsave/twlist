CC			= gcc
CFLAGS			= -c -Wall -Wextra -D_GNU_SOURCE -Wfatal-errors -std=gnu99
SRCDIR 			= .
DEBUGOUTPUTDIR 		= build/debug
RELEASEOUTPUTDIR	= build/release
SOURCES			= twtest.c
INCLUDES		= -I. -I../include
_OBJECTS		= $(SOURCES:.c=.o)
DEBUGOBJECTS 		= $(patsubst %,$(DEBUGOUTPUTDIR)/%,$(_OBJECTS))
RELEASEOBJECTS 		= $(patsubst %,$(RELEASEOUTPUTDIR)/%,$(_OBJECTS))
DEBUGTARGET		= build/debug/twtest
RELEASETARGET		= build/release/twtest

debugall:	$(SOURCES) $(DEBUGTARGET)
releaseall:	$(SOURCES) $(RELEASETARGET)

# additional flags
# CONFIG_DEBUG_LIST	- extensive debugging of list with external debugging
# 			functions
debug:		CFLAGS += -DDEBUG -g
debug:		debugall

release:	CFLAGS += -DNDEBUG
release: 	releaseall

$(DEBUGTARGET): $(DEBUGOBJECTS) 
	$(CC) $(LDFLAGS) $(DEBUGOBJECTS) -o $@

$(RELEASETARGET): $(RELEASEOBJECTS) 
	$(CC) $(LDFLAGS) $(RELEASEOBJECTS) -o $@

$(DEBUGOUTPUTDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) $(INCLUDES) $< -o $@

$(RELEASEOUTPUTDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) $(INCLUDES) $< -o $@

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) $< -o $@

clean:
	rm -rf $(DEBUGOBJECTS) $(DEBUGTARGET) $(RELEASEOBJECTS) $(RELEASETARGET)
