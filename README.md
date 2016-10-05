#xsensors 0.75 - Mystro256 fork

This is another release of xsensors, a program designed to display all
the related information from your motherboard sensors.  This information is
gathered via lm_sensors, the software drivers that actually gathers the 
sensor information.

![alt text](https://github.com/Mystro256/xsensors/raw/master/screenshot.png "Screenshot")

##Requirements:
- lm_sensors 3.0.0+ (http://www.lm-sensors.org)
- gtk+ 2 or 3 (http://www.gtk.org)
- cairo (https://www.cairographics.org)

##Installation:
```sh
./configure
make
make install
```

I would suggest running the detect script if you don't have lm_sensors
configured yet:
```sh
sudo sensors-detect
```
