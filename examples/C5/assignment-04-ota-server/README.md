# Assignment 4 OTA server

Flash this on the board that will serve the update. It opens an access point and serves the student firmware over HTTP.

| | |
| --- | --- |
| SSID | `ESP32` |
| Password | `ESP32` |
| URL | `http://192.168.4.1/firmware.bin` |

`firmware.bin` is the student project (`assignment-04-ota`) built with the boot log changed to `Running firmware v2!`. `idf.py flash` writes that file into the `fw` data partition. It is not linked into this application, because the image is about 1 MB and would not fit beside the HTTP server in one app slot.

The student board joins this access point only when it has no Wi-Fi credentials in NVS. If Assignment 3 already stored a network, erase that board's flash and flash the student project again before the demo.

```bash
idf.py build flash monitor
```

To replace the image, rebuild `assignment-04-ota`, copy `build/assignment_04_ota.bin` over `firmware.bin`, and flash this project again. The file must be an ESP32-C5 app image and must fit in the `fw` partition (1536 KB).
