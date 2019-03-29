#!/bin/bash
echo "This script flashes the static webitems to the esp32 per partition table"
./mkspiffs/mkspiffs -c ./main/webdata/ -b 4096 -p 256 -s 0x100000 spiffs.bin
python ./esptool/esptool.py --chip esp32 --port /dev/ttyUSB0 --baud 115200 write_flash -z 0x180000 spiffs.bin

