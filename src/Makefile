CC=gcc
CFLAGS=-Wall -g

LFLAGS=-L../lib
LIBS=

OPTIONS=-D_POSIX_C_SOURCE=200809L

SRCS=icshell.c cmdp.c psignal.c process.c

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
	$(CC) $(CFLAGS) $(INCLUDES) $(OPTIONS) -c -o $@ $< 

$(MAIN): $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) $(OPTIONS) -o $@ $^ $(LFLAGS) $(LIBS)

clean: 
	rm -f $(ODIR)/*.o *~ $(MAIN)
