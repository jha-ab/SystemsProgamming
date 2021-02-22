Assignment 5

Instructions on how the module will be tested :

Copy the files char_driver.c, Makefile and userapp.c to a virtual linux machine
and follow the following steps:

1) Compile driver module : $ make

2) Load module : $ sudo insmod usbkbd.ko

3) Check the USB device number and replace in the script

4) Run script rebindUSBKBD.sh
		   
5) Test the driver

6) Unload module : $ sudo rmmod char_driver
