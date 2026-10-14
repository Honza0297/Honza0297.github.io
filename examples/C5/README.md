# ESP-IDF workshop examples — ESP32-C5

Reference ESP-IDF projects aligned with the **published** [ESP32-C5 workshop](../../C5/EN/) chapters (introduction + assignments **1–6**).

This sets the target to **esp32c5** and builds each project.

## Layout

| Workshop chapter | Directory |
| --- | --- |
| Assignment 1: Install ESP-IDF | *(no companion project - installation only)* |
| Assignment 2: Basics: Hello World & LED | [`assignment-02-led-components`](assignment-02-led-components/) |
| Assignment 3: Wi-Fi & Provisioning | [`assignment-03-wifi-provisioning`](assignment-03-wifi-provisioning/) |
| Assignment 4: OTA Updates (student board) | [`assignment-04-ota`](assignment-04-ota/) |
| Assignment 4: OTA server (second board) | [`assignment-04-ota-server`](assignment-04-ota-server/) |
| Assignment 5: ESP-NOW Chat Room | [`assignment-05-esp-now`](assignment-05-esp-now/) |
| Assignment 6: LP-core | [`assignment-06-low-power`](assignment-06-low-power/) |

Starting with Assignment 4, these projects assume you have a push-button wired to **GPIO0** (with an external pull-down resistor, other leg to **3V3**) and are using the onboard WS2812 LED on **GPIO27** for status — see Assignment 4's "Connecting a Button" section for the wiring diagram. Assignment 5 uses that LED and `idf.py monitor` for the chat. Assignment 6 adds two external LEDs, blue on **GPIO4** and green on **GPIO5**, and uses the same button on **GPIO0**.

The Assignment 4 server is a second board. Flash `assignment-04-ota-server` onto it; it opens the `ESP32` access point and serves the prebuilt `firmware.bin` to the student board.

