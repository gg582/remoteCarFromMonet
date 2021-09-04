#!/bin/bash

echo "Car controller begins"

(cd ../car/controller/;./reader.sh;./writer.sh)
