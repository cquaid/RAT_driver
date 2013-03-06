RAT_driver
==========

*A userspace R.A.T. 7 driver for Linux*

This is a driver I hacked together at work to get my R.A.T.7 Albino working.  It's not pretty, but it works.
It depnds on libusb-0.1-4 Xlib and Xtest (xtest should be installed with x normally)

The currently driver requires root access to run.

This needs to be updated to libusb-1.x.x but I've been too lazy to learn the new api.

To use the R.A.T. 7 Albino, add -DALBINO7 to the Makefile's CFLAGS.

To get the product id run:
<pre>
$ lsusb -d 06a3:
Bus 002 Device 004: ID 06a3:0cce Saitek PLC
</pre>

The ID, 06a3:0cce, specifies the vendor id (06a3) first then the product id.

Once the proper product ID is set just run make and run the produced binary.  There isn't an install rule yet.
