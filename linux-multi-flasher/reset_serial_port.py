#!/usr/bin/env python
import sys, serial, time

print "Resetting serial port " + sys.argv[1]

ser = serial.Serial(sys.argv[1], 57600)

ser.setDTR(0)
time.sleep(0.1)
ser.setDTR(1)

ser.close()