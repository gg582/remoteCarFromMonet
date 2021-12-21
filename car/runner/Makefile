
CFLAGS = -O2 -Wall -I../common

SRCS = runner.c

CMDS = $(patsubst %.c, %, $(SRCS))

all: $(CMDS)

%: %.c
	gcc $(CFLAGS) -o $@ $< 

clean:
	rm -f $(CMDS)

