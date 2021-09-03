#!/bin/bash

echo "Car controller begins"

(cd ../car/controller/;bg reader.sh;bg writer.sh)
