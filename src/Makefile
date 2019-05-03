CC=gcc
CFLAGS=-Wall -g

LFLAGS=-L../lib
LIBS=

SRCS=icshell.c cmdp.c process.c

IDIR=../include
_DEPS=
DEPS=$(pathsubst %,$(IDIR)/%,$(_DEPS))
INCLUDES=-I$(IDIR)

ODIR=obj
_OBJS=$(patsubst %.c,%.o,$(SRCS))
OBJS=$(patsubst %,$(ODIR)/%,$(_OBJS))

MAIN=icshell

.PHONY: all clean

all: $(MAIN)

$(ODIR)/%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $< 

$(MAIN): $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^ $(LFLAGS) $(LIBS)

clean: 
	rm -f $(ODIR)/*.o *~ $(MAIN)
