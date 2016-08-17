#!/usr/bin/env python
#
# RPi2c - test i2c communication between an Arduino and a Raspberry Pi.
#
# Copyright (c) 2013 Carlos Rodrigues <cefrodrigues@gmail.com>
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#


from __future__ import division
from __future__ import print_function


import RPi.GPIO as GPIO
import smbus
import time
import sys


I2C_BUS = 1
I2C_SLAVE = 0x04

def decimal_check(value):
    if value > 255 or value < 0:
        return 255;
    else:
        return value;

def interpolate(value, a1, a2, b1, b2):
    # Normalize the value into a 0..1 interval...
    n = float(value - a1) / float(a2 - a1)

    # Scale the normalized value to the target interval...
    return b1 + (n * (b2 - b1))


if __name__ == '__main__':
    # Initialize the interrupt pin...
    GPIO.setmode(GPIO.BCM)

    # Initialize the RPi I2C bus...
    i2c = smbus.SMBus(I2C_BUS)

    while 1:
        try:
	    execute = int(input("Which function would you like to execute?\nSet Character (10)\nSet Color (20)\nSet Color Change Increment (21)\nChange Mode (30)\nStop (90)\nInput: "));
	    if execute == 10: #Set Character
	      function = 10;
	      new_char = ord(raw_input("Enter a single character to transmit: "));
	      try:
                  i2c.write_byte_data(I2C_SLAVE, function, new_char)
	      except IOError:
                  sys.stderr.write("*** error: sending led value ***\n")
	    elif execute == 20: #Set Color
              function = 20;
	      red = decimal_check(int(input("Enter a decimal value for red: ")));
	      sys.stdout.write("Red: %d\n" % red);
	      green = decimal_check(int(input("Enter a decimal value for green: ")));
	      sys.stdout.write("Green: %d\n" % green);
	      blue = decimal_check((input("Enter a decimal value for blue: ")));
	      sys.stdout.write("Blue: %d\n" % blue);

	      values = [blue, red, green];
	      try:
	          i2c.write_i2c_block_data(I2C_SLAVE, function, values);
	      except IOError:
                  sys.stderr.write("*** error: sending led value ***\n")
	    elif execute == 21: #Set Increment
	      function = 21;
	      red = decimal_check(int(input("Enter a decimal value for red: ")));
	      sys.stdout.write("Red: %d\n" % red);
	      green = decimal_check(int(input("Enter a decimal value for green: ")));
	      sys.stdout.write("Green: %d\n" % green);
	      blue = decimal_check((input("Enter a decimal value for blue: ")));
	      sys.stdout.write("Blue: %d\n" % blue);

	      values = [blue, red, green];
	      try:
	          i2c.write_i2c_block_data(I2C_SLAVE, function, values);
	      except IOError:
                  sys.stderr.write("*** error: sending led value ***\n")
	    elif execute == 30: #Stop
	      function = 30;
	      mode = decimal_check(int(input("Which mode?\nMorse (10)\nColor Wheel (20)\nInput: ")));
	      try:
	          i2c.write_byte_data(I2C_SLAVE, function, mode)
	      except IOError:
                  sys.stderr.write("*** error: sending led value ***\n")
	    elif execute == 90: #Stop
	      function = 90;
	      try:
	          i2c.write_byte_data(I2C_SLAVE, function, 0)
	      except IOError:
                  sys.stderr.write("*** error: sending led value ***\n")
            try:
	      sys.stdout.write("\nDone.\n\n");
                # Send the PWM value for the LED to the Arduino...
            except IOError:
                sys.stderr.write("*** error: sending led value ***\n")

        except KeyboardInterrupt:
            GPIO.cleanup()

# vim: set expandtab ts=4 sw=4:
