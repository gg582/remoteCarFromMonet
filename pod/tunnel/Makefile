CFLAGS = -O2 -w

SRCS = catd.c toss.c

CMDS = $(patsubst %.c, %, $(SRCS))

all: $(CMDS)

%: %.c
	gcc $(CFLAGS) -o $@ $< 

clean:
	rm -f $(CMDS)

