
SUBDIRS = car/controller car/driver car/test car/runner pod/driver pod/tunnel car/tunnel

.PHONY: $(SUBDIRS)

all:
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir; \
	done

clean:
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir clean; \
	rm -rf car/controller/backward \
	rm -rf car/controller/forward \
	rm -rf car/controller/garbage \
	rm -rf car/controller/left \
	rm -rf car/controller/right \
	rm -rf car/controller/stop \
	rm -rf car/real-car/RealCarRunner \
	rm -rf car/real-car/sendSensorValueToVirtCar \
	rm -rf car/runner/runner \
	rm -rf car/virt-cartun/motorRelayInPod \
	rm -rf car/virt-cartun/sensorRelayInPod; \
done
