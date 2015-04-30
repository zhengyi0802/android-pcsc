#!/bin/sh

adb push ./reader.conf /etc/reader.conf
cd ../../../.. 
adb push out/target/product/passion/obj/lib/libifdsmartcardapi.so /system/lib
adb push out/target/product/passion/system/bin/testpcsc_sendapdu /system/bin
adb push out/target/product/passion/system/bin/testpcsc_testrun /system/bin
adb shell 





