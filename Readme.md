# Panel Painter

This is the C/C++ code for an esp8266 d1mini.

## Configure Upload path

Change the following in `platformio.ini`

        upload_port = /dev/cu.wchusbserial1420
        monitor_port = /dev/cu.wchusbserial1420

## Flash Web Client

Build the client and copy all files from `dist-gz/` to `data/`.

Then run

        pio run --target buildfs
        pio run --target uploadfs
