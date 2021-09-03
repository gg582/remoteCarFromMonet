
CFLAGS = -O2 -I../common

SRCS = left.c right.c forward.c backward.c stop.c garbage.c

CMDS = $(patsubst %.c, %, $(SRCS))

all: $(CMDS)

%: %.c
	gcc $(CFLAGS) -o $@ $< 

clean:
	rm -f $(CMDS)
