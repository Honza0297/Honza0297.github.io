---
title: "ESP-IDF and ESP32-C6 Workshop"
date: 2024-06-03T00:00:00+01:00
layout: default
---

# ESP-IDF and ESP32-C6 Workshop

Welcome to the Espressif IoT Development Framework (ESP-IDF) workshop!

## About this workshop

By participating in this workshop, you will gain a deeper understanding of how the Espressif IoT
Development Framework (ESP-IDF) and ESP32-C6 *System on a Chip* (SoC) work. Through a number of
hands-on assignments covering a range of essential skills, you will learn to create even better
and more efficient IoT applications for your projects!

Specifically, in this workshop we will go through the installation of the ESP-IDF framework.
Then we will move on to creating our own project, which we will try to build, upload, and monitor
using the ESP-IDF terminal. In the following chapters, we will learn how to connect to Wi-Fi using
provisioning, perform OTA updates, communicate via ESP-NOW, and finally some tricks for working
with the low-power core that is part of the ESP32-C6 SoC.

> This workshop will certainly bring a lot of interesting information even to those who use other
> chips from the ESP32 family. However, keep in mind that not all chips support everything that
> ESP32-C6 supports (like dual-band Wi-Fi 6 or the ULP RISC-V core)!

## Companion reference projects

This workshop has companion ESP-IDF projects following the same assignment numbering. They are
published under [examples/C6/](../../examples/C6/): open a project folder to read the files, or
download the `.zip` linked next to it. See [examples/C6/README.md](../../examples/C6/README.md)
for how to build them.

## Agenda

If you have met the [prerequisites](#prerequisites), we can start with the individual chapters:

- [Introduction: ESP-IDF Presentation](introduction/): Initial introduction to our ESP-IDF
  framework, followed by basic information about the ESP32-C6 chip and the devkit on which this
  tutorial will be demonstrated.
- [Assignment 1: Install ESP-IDF](assignment-1/): Installing the necessary tools to start
  development.
- [Assignment 2: Basics: Hello World & LED](assignment-2/): Verifying hardware connectivity and
  introducing the ESP-IDF build lifecycle via the terminal.
- [Assignment 3: Wi-Fi & Provisioning](assignment-3/): Understanding the Wi-Fi peripheral,
  dual-band capabilities, and connecting using BLE provisioning.
- [Assignment 4: OTA Updates](assignment-4/): Remote code updates using partition tables and HTTPS.
- [Assignment 5: ESP-NOW Chat Room](assignment-5/): Connectionless peer-to-peer communication.
- [Assignment 6: LP-core](assignment-6/): Demonstration of working with the low-power core for
  applications where energy consumption is an important factor.

## Prerequisites

To follow this workshop, you will need both hardware and software equipment.

Required hardware:

- Computer running Linux, Windows or macOS operating system
- ESP32-C6-DevKitC or ESP32-C6-DevKitM
- USB cable (supporting power + data) compatible with the devkit above
- Visual Studio Code

Required software:

- [Visual Studio Code](https://code.visualstudio.com/download)
- **ESP BLE Prov** app:
  - Android: [ESP BLE Prov](https://play.google.com/store/apps/details?id=com.espressif.provble&pcampaignid=web_share)
  - iOS: [ESP BLE Prov](https://apps.apple.com/us/app/esp-ble-provisioning/id1473590141)

## Time Requirements

**Estimated time: 120 min**

## Feedback

If you have any feedback about the workshop, feel free to start a new
[discussion on GitHub](https://github.com/espressif/developer-portal/discussions).

## Conclusion

We hope that the workshop will bring you knowledge that will serve as a solid foundation for your
future projects. Thank you for the time and effort you devoted to the workshop, and we look forward
to the projects you will create with our chips in the future!

---

[← Back to Workshop Portal](../../)
