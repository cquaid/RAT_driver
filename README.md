RAT_driver
==========

*A userspace R.A.T. 7 driver for Linux*

This is a driver I hacked together at work to get my R.A.T.7 Albino working.  It's not pretty, but it works.
It depnds on libusb-1.0-0 and uinput (most distro's kernels are compiled with uinput support.)

The driver requires uinput to be loaded:
<pre>
modprobe uinput
</pre>

uinput creates /dev/uinput or /dev/input/uinput typically... so set that in the makefile

The driver supports the R.A.T. 7 and the R.A.T. 7 Albino.

The currently driver requires root access to run.

To make the R.A.T. 7 Albino driver run:
<pre>
make Albino7
</pre>

To get the product id run:
<pre>
$ lsusb -d 06a3:
Bus 002 Device 004: ID 06a3:0cce Saitek PLC
</pre>

The ID, 06a3:0cce, specifies the vendor id (06a3) first then the product id.

Once the proper product ID is set just run make and run the produced binary.

To install for Albino7:
<pre>
make Albino7 && make install_Albino7
</pre>

To install for RAT7:
<pre>
make RAT7 && make install_RAT7
</pre>


The uninstall rules are similar to the install rules:
<pre>
make uninstall_RAT7
make uninstall_Albino7
</pre>

*Attributions*

Thanks to [xwavex](https://github.com/xwavex) for the DPI get/set code from his project [pyRAT](https://github.com/xwavex/pyRAT).
