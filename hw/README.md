# IoT semaphore -- build your hardware for Raspberry Pi 3

## Step 1 -- Flash your software
Download the latest version of raspbian from `https://www.raspberrypi.org/downloads/raspbian/` and flashing it in an SD card.

## Step 2 -- Hardware connection

![alt text](https://raw.githubusercontent.com/HiPeRT/IoT-Semaphore/master/hw/connections.jpg)


- Connect the power distribution board to a car battery using the supplied terminals.
- Connect the Raspberry Pi 3 to the 5V output of the power distribution board.
- Connect the COM connector of the semaphore (white wire) to the 12V output of the power distribution board.
- Connect the COM connector of each Relay of the control module to the GND of the power distribution board.
- Connect the Normally Open (NO) output of each Relay of the control module to the respective led of the semaphore (black, brown and grey wires).
- Power up the control module via the 5V output provided by the Raspberry Pi 3.
- Connect each GPIOs specified in the configuration file to the control module.


![alt text](https://raw.githubusercontent.com/HiPeRT/IoT-Semaphore/master/hw/semaphore.jpg)

## Controller schematics

If you want to build your own relay board, this is the schematic we used in our demo

![alt text](https://raw.githubusercontent.com/HiPeRT/IoT-Semaphore/master/hw/schematics/controller_schematic.png)