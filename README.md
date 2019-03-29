# HTTP server with SSL support using OpenSSL

This example creates a SSL server that returns a simple HTML page when you visit its root URL.

See the `esp_https_server` component documentation for details.

## Certificates

You will need to approve a security exception in your browser. This is because of a self signed
certificate; this will be always the case, unless you preload the CA root into your browser/system
as trusted.

You can generate a new certificate using the OpenSSL command line tool:

```
openssl req -newkey rsa:2048 -nodes -keyout prvtkey.pem -x509 -days 3650 -out cacert.pem -subj "/CN=ESP32 HTTPS server example"
```

Expiry time and metadata fields can be adjusted in the invocation.

Please see the openssl man pages (man openssl-req) for more details.

It is **strongly recommended** to not reuse the example certificate in your application;
it is included only for demonstration.

## Build the static images to be stored
./mkspiffs/mkspiffs -c ./main/webdata/ -b 4096 -p 256 -s 0x100000 spiffs.bin
python ./esptool/esptool.py --chip esp32 --port /dev/ttyUSB0 --baud 115200 write_flash -z 0x180000 spiffs.bin 

## Errors
Getting error on first web page handshake.  This seems to have popped up after correctly serving css style page, maybe taking too long to respond but it does seem as if the error occurs before even entering the uri handlers
