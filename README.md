RAT_driver
==========

A userspace R.A.T. 7 driver for Linux.

This is a driver I hacked together at work to get my R.A.T.7 Albino working.  It's not pretty, but it works.
It depnds on libusb-0.1-4 Xlib and Xtest (xtest should be installed with x normally)

This needs to be updated to libusb-1.x.x but I've been too lazy to learn the new api.

The driver is set up for the Albino and the PRODUCT_ID macro in RAT_driver.c will have to be changed if you'd like to use a regular R.A.T.7.

To get the product id run:
 $ lsusb | grep Saitek

The output should be similar to:
 Bus 002 Device 004: ID 06a3:0cce Saitek PLC

The ID, 06a3:0cce, specifies the vendor id (06a3) first then the product id.

Once the proper product ID is set just run make and run the produced binary.  There isn't an install rule yet.

NOTE: I have not tested this for multiple monitors yet.  More than likely I'll need to do something with Xinerama.

NOTE: The snipe button is currently set to kill the mouse driver.  I was using it as a quick-kill during testing.
