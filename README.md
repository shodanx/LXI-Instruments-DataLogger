# LXI Instruments Data Logger

## Introduction

LXI Instruments Data Logger is application for controlling LXI-compatible instruments, such as digital multimeters,  source measurement unit, electronic load, power sources, etc.

This application is based on open source [Liblxi](https://github.com/lxi-tools/liblxi) library.

I create these applications for use on my [Raspberry Pi Zero W](https://www.raspberrypi.org/products/raspberry-pi-zero-w/) board. It has many advantages, such as: autonomous data logging, very low power consumption, simple design and flexible configuration.


## Requriments and compile
- Liblxi 1.13
- Libconfig
- Pthread
- Libi2c
- Libncursesw
- GCC
- GNU Screen


Install requirements(Raspbian Buster Lite):
- apt-get install git screen libavahi-client3 libavahi-client-dev libxml2 libxml2-dev libconfig9 libconfig-dev libncursesw5 libncurses5-dev libncursesw5-dev libi2c-dev i2c-tools
- git clone https://github.com/shodanx/LXI-Instruments-DataLogger.git
- wget https://github.com/lxi-tools/liblxi/releases/download/v1.13/liblxi-1.13.tar.xz
- tar xf liblxi-1.13.tar.xz
- cd liblxi-1.13
- ./configure; make; make install


- Set correct UTF-8 locale at default in raspi-config.
- Run make.sh to compile the application.

## Usage
You can use run.sh script for start applitation into [GNU Screen](https://en.wikipedia.org/wiki/GNU_Screen) utility. It automaticaly run screen and attach last screen session. For screen deattach (autonomous use) you can press Ctrl-A D combination. At next run, script attach to last session automatically.
If screen resized use 'r' key to window update.
Press 'space' for pause measurements, or press 'q' for close application.

Measurements data saved into csv_save_dir. CSV file name generated automatically as current date and time.

Into 'channels' config section you can configure up to 16 different instruments for paralells measurements. Each instruments can be configured with different init-string, timeout, read-command, and connection settings.

Screen update rate slowdown to 100 ms refresh interval, for low CPU usage on high-speed measurements(NPLC lower 1).

## Texas Instruments TMP117 temperature sensor

This application also support read temperature data from high precision TI TMP117 temperature sensor(up to 4 sensor on one I2C bus).
This feature tested only on Raspberry Pi Zero W and 3B+.

For enable "repeated start" sequence of TMP117 on Raspberry Pi you need do follow steps: 

 1. Enable i2c support in raspi-config.
 2. Add line "i2c-bcm2708" into /etc/modules
 3. Create file /etc/modprobe.d/i2c.conf with content "options i2c-bcm2708 combined=1"
 4. Reboot Raspberry Pi.

To minimize SHE(Self-Heating Effect) with FlexPCB use 0x2A0 configuration word of TMP117. Also you can set "delay" in config to prevent frequent I2C reading, it also reduce SHE. I recommend set delay like as shown in Table 7 datasheet.


FlexPCB can be ordered here:

- [Long 400mm version](https://oshpark.com/shared_projects/rqIdFGTS) - High cost, low self-heating effect by Raspberry Pi board.

- [Short 150mm version](https://oshpark.com/shared_projects/LciP4Zpo) - Low cost, high self-heating effect by Raspberry Pi board.

Order that board with Flex option only!!!

## Arroyo TECSource support

Added support load current and temperature data from Arroyo Instruments TECSource series. (tested with TECSource 5305)

Connect TECSource via USB and use "Arroyo_5305_TECSource_port" config variable to configure port path to virtual USB.
Data refreshed every 1 second. After each reading of data from TECSource, it automatically back to "LOCAL" mode and unlock knob.

## Experimental D3 web graph feature.

You can install NGINX webserver for download CSV files and draw graph.
This example show how you can install Nginx, create file tree, mount tmpfs storage for CSV file(reduce sdcard write cycles):
- apt-get install nginx
- mkdir -p /var/www/csv /var/www/script
- cp LXI-Instruments-DataLogger/web_server/default /etc/nginx/sites-available/default
- cp LXI-Instruments-DataLogger/script/* /var/www/script
- service nginx restart
- grep -qxF 'tmpfs /var/www/csv tmpfs async,nodev,nosuid,size=100M 0 0' /etc/fstab || echo 'tmpfs /var/www/csv tmpfs async,nodev,nosuid,size=100M 0 0' >> /etc/fstab
- mount -a

As result you can browse CSV files on URL: http://RASPBERRY-PI-IP-ADDRESS/csv/

View graph on URL: http://RASPBERRY-PI-IP-ADDRESS/script/index.html?filename=/csv/NAME-OF-CSV-FILE

On each start application generate csv.js configuration file, with channel's table like as:
![](https://misrv.com/wp-content/uploads/2019/06/aa26c667-640b-49d1-b9b7-a48c79f798d7.png)

Where:
- curveTitle - Name of channel.
- channel - id channel in special format.
- offset - offset line an absolute value.
- scale - line scale coefficent.
- group - 0-don't group lines; 1-group lines at one Y axis scale.
- tspan - main temperature sensor for calculations.
- axis_is_ppm - Switch between float and PPM axis label.

Note: use syncfs=1 when CSV file placed to TMPFS partition, otherwise use syncfs=0 to save your sd-card. 

*Original D3 graph version created by TiN (Illya Tsemenko https://xdevs.com/). Modified by Shodan (Andrey Bykanov https://misrv.com/)

## Screenshot

Measure MOX112523100AK with multi channel feature:

![](https://misrv.com/wp-content/uploads/2019/06/logger.png)

Measure LM399AH 10.6V reference:

After some post-processing with Excel or other software you can build graph like this:
![](https://misrv.com/wp-content/uploads/2019/05/lm399_34410.png)

Or you can use Experimental D3 feature for on-line graph view:
![](https://misrv.com/wp-content/uploads/2019/05/acf73fe5-e32d-483e-8aa3-bb822b0d6f58-e1559315294172.png)

![](https://misrv.com/wp-content/uploads/2019/06/c1701e98-a768-4c82-aac0-a66656f6251a.png)

## Complete solution
![](https://misrv.com/wp-content/uploads/2019/05/FJIMG_20190519_085057.jpg)
![](https://misrv.com/wp-content/uploads/2019/05/FJIMG_20190519_085127.jpg)

## License and author

This code is released under GPL v3 license.

Author: Andrey Bykanov (aka Shodan)
E-Mail: adm@misrv.com
Location: Tula city, Russia.
