#!/usr/bin/python 

import serial

ser = serial.Serial(
    port='/dev/tnt1',\
    baudrate=115200,\
    parity=serial.PARITY_NONE,\
    stopbits=serial.STOPBITS_ONE,\
    bytesize=serial.EIGHTBITS,\
        timeout=0)

print("connected to: " + ser.portstr)
ser.flush();

while True:
    incoming = ser.readline()
    if incoming == '':
        continue
    print "-----------", incoming
    ser.write(incoming+ '\n')
    ser.write('OK\ni')
ser.close()
