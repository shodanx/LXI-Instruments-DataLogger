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


Install requirements(Raspbian Stretch Lite):
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

For high-speed measurements(NPLC lower 0.1) you can configure refresh speed devider 'screen_refresh_div' for low CPU usage in screen refresh code. As example screen_refresh_div=100 has refreshed screen after each 100 measurements, it may take up to 2 times faster data receiving.

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

Note: use syncfs=1 when CSV file placed to TMPFS partition, otherwise use syncfs=0 to save your sd-card. 

*Original version created by TiN (Illya Tsemenko https://xdevs.com/)
*Modified by Shodan (Andrey Bykanov https://misrv.com/)

## Screenshot

Measure LM399AH 10.6V reference:

![](https://misrv.com/wp-content/uploads/2019/05/cons_dl3.jpg)

Legend: 
- Sample number
- Timestamp
- Ambient Temperature from TMP117
- Channel 0 reading (34410A - output voltage)
- Channel 1 reading (E36313A - input current)

After some post-processing with Excel or other software you can build graph like this:
![](https://misrv.com/wp-content/uploads/2019/05/lm399_34410.png)

Or you can use Experimental D3 feature for on-line graph view:
![](https://misrv.com/wp-content/uploads/2019/05/acf73fe5-e32d-483e-8aa3-bb822b0d6f58-e1559315294172.png)

![](https://misrv.com/wp-content/uploads/2019/05/685aadb8-2b54-4e4b-8eb2-d40e07627619.png)

## Complete solution
![](https://misrv.com/wp-content/uploads/2019/05/FJIMG_20190519_085057.jpg)
![](https://misrv.com/wp-content/uploads/2019/05/FJIMG_20190519_085127.jpg)

## License and author

This code is released under GPL v3 license.

Author: Andrey Bykanov (aka Shodan)
E-Mail: adm@misrv.com
Location: Tula city, Russia.
