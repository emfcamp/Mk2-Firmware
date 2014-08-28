#!/bin/bash
PORT=$1

echo "Starting to flash to port $PORT"

stty -F "/dev/$PORT" 1200
sleep 1

/home/marek/bin/arduino-1.5.7/hardware/tools/bossac -i -d --port=$1 -U true -e -w -b EMF2014.cpp.bin -R | grep "pages"