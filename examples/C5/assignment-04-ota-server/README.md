# Assignment 4 OTA server

Flash this on the board that will serve the update. It joins the `ESP32-WS` router and serves the student firmware over HTTP.

| | |
| --- | --- |
| SSID | `ESP32-WS` |
| Password | `ESP32-WS` |
| URL | `http://192.168.0.100/firmware.bin` |

The address is static. There must not be a second access point named `ESP32-WS`, or clients join that one and cannot reach `192.168.0.100`.

`firmware.bin` is the student project (`assignment-04-ota`) built with the boot log changed to `Running firmware v2!`. `idf.py flash` writes that file into the `fw` data partition. It is not linked into this application, because the image is about 1 MB and would not fit beside the HTTP server in one app slot.

The student board joins `ESP32-WS` as well. Flash it again after changing `assignment-04-ota`, so it does not stay on a network stored by Assignment 3.

```bash
idf.py build flash monitor
```

To replace the image, rebuild `assignment-04-ota`, copy `build/assignment_04_ota.bin` over `firmware.bin`, and flash this project again. The file must be an ESP32-C5 app image and must fit in the `fw` partition (1536 KB).
