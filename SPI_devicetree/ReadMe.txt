Copy BB-SPI1-01-00A0.dts to BBB and compile it using dtc
	dtc -O dtb -o BB-SPI1-01-00A0.dtbo -b 0 -@ BB-SPI1-01-00A0.dts

Then copy the compiled file to /lib/firmware/
	cp BB-SPI1-01-00A0.dtbo /lib/firmware/

Then enable the device tree overlay:
	echo BB-SPI1-01 > /sys/devices/bone_capemgr.9/slots

Plug in your BBB to a host computer using the mini usb data cable.
Go to My Computer>BeagleBone Getting Started> and open uEnv.txt Copy and paste this command into the .txt file. Make sure to save your changes. (Ctrl+s)
	optargs=quiet drm.debug=7 capemgr.disable_partno=BB-BONELT-HDMI,BB-BONELT-HDMIN capemgr.enable_partno=BB-SPI1-01
After you save the changes, reboot your beaglebone black.

Make sure it is enabled. You should now have a new spidev-file in the folder /dev/
ls -al /dev/spidev1.0

If you don't see it, you can type the following to manually enable the device tree overlay.
	echo BB-SPI1-01 > /sys/devices/bone_capemgr.9/slots

You should also be able to see the pingroups:
	cat /sys/kernel/debug/pinctrl/44e10800.pinmux/pingroups
