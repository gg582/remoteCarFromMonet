
SUBDIRS = car/controller car/driver car/test car/runner pod/driver pod/tunnel

.PHONY: $(SUBDIRS)

all:
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir; \
	done

clean:
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir clean; \
	done
