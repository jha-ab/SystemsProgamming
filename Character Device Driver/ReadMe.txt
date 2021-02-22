

1) Compile driver module : $ make

2) Load module : $ sudo insmod char_driver.ko NUM_DEVICES=<num_devices>

3) Test driver :
	1) Compile userapp : $ make app
	2) Run userapp : $ sudo ./userapp <device_number>			
		where device_number identifies the id number of the device to be tested.   

	Note : userapp has to be executed with sudo privilege as the device files
		   in /dev/ are created in the driver with root privileges.
		   
4) Unload module : $ sudo rmmod char_driver

NOTE : 1. User app has been modified to work with specific version of linux kernel as well as to take commands in a loop until 'e' in entered. This is used to test sequential reads and writes also with lseek current offset options .

2. Lseek with option 2 (end) will shift offset to send of file I.e end of present data (not to end of ramdisk)

3. Lseek with an offset greater than Ramdisk size will extend Ramdisk size by offset + a new page


