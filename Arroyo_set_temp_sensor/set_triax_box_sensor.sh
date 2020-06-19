#!/bin/sh

#Temp   Resistance
#9.734  19913
#24.992 10004
#45.344 4380

chmod 666 /dev/Z3805
stty -F /dev/Z3805 38400
echo TEC:CONST 1.071804607,2.410520717,0.7938615112 >/dev/Arroyo
echo TEC:PID 3,0.0005,100 >/dev/Arroyo
echo TEC:TRATE 0.1 >/dev/Arroyo
