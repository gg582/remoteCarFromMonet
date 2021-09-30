cmd_/home/pi/remote-car/pod/driver/modules.order := {   echo /home/pi/remote-car/pod/driver/car.ko; :; } | awk '!x[$$0]++' - > /home/pi/remote-car/pod/driver/modules.order
