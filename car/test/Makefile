
CFLAGS = -O2 -I../common

SRCS = ul-test

CMDS = $(patsubst %.c, %, $(SRCS))

all: $(CMDS)

%: %.c
	gcc $(CFLAGS) -o $@ $< 

clean:
	rm -f $(CMDS)
