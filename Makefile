
SUBDIRS = car/controller car/driver relay/driver relay/tunnel

.PHONY: $(SUBDIRS)

all:
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir; \
	done

clean:
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir clean; \
	done
