---
title: "ESP-IDF and ESP32-C5 Workshop - Assignment 1: Install the Espressif-IDE"
date: 2024-06-30T00:00:00+01:00
layout: default
---

## Assignment 1: Install ESP-IDF and VSCode with ESP-IDF plugin

---

You will need [Visual Studio Code](https://code.visualstudio.com/) and the installed ESP-IDF
framework for this workshop. The first task will therefore be to install all the necessary tools.


To install ESP-IDF, we will use our new [EIM](https://docs.espressif.com/projects/idf-im-ui/en/latest/) installer and install the VSCode extension according to the instructions on the [GitHub pages of the ESP-IDF plugin for VS Code](https://github.com/espressif/vscode-esp-idf-extension?tab=readme-ov-file#how-to-use).

**Windows:**

### Windows Installation

1. Install [EIM](https://dl.espressif.com/dl/eim/index.html)
    * Press Windows key + R and type powershell in the dialog box that appears
    * After the terminal appears, enter: `winget install Espressif.eim`
2. Install ESP-IDF using EIM
    * Run EIM from the start menu or desktop icon
    * On the welcome screen, click `Start Installation` and then `Custom installation` and follow the wizard. On the version selection page, choose IDF version 5.5.4
    * During installation, the installer will also offer to install git or python if they are not already on your computer.
    * After installation is complete, return to the installer's version manager page and use the `Install Drivers` button in the bottom of the screen to install the necessary drivers.

3. Install [Visual Studio Code](https://code.visualstudio.com/download)
4. In VS Code, open **Extensions** (Ctrl + Shift + X or ⇧ + ⌘ + X)
5. Find the [ESP-IDF plugin](https://marketplace.visualstudio.com/items?itemName=espressif.esp-idf-extension) and install it
5. Wait for the espressif icon to appear in the left panel and click on it.
6. The extension will automatically find the installed ESP-IDF

**Linux and Mac:**

### Linux and Mac Installation

1. Install [EIM](https://dl.espressif.com/dl/eim/index.html)
    * If you choose installation via package manager (brew, apt, rpm), it will handle all [prerequisites](https://docs.espressif.com/projects/idf-im-ui/en/latest/prerequisites.html) for you.
    * If we choose the CLI version, we just need to call `eim install -i v5.5.4` now
    * If we chose the GUI version, we click on the `Start Installation` button on the welcome screen and then `Custom installation` and follow the wizard. On the version selection page, we choose IDF version 5.5.4


2. Install [Visual Studio Code](https://code.visualstudio.com/download)
3. In VS Code, open **Extensions** (Ctrl + Shift + X or ⇧ + ⌘ + X)
4. Find the [ESP-IDF plugin](https://marketplace.visualstudio.com/items?itemName=espressif.esp-idf-extension) and install it
5. Wait for the espressif icon to appear in the left panel and click on it.
6. The extension will automatically find the installed ESP-IDF

### Installing ESP BLE Prov

During one of the tasks, we will also try Wi-Fi provisioning. For this we will need an application
through which we will tell our ESP chip the SSID and password for the network.

You can either search for the app in the app store of the respective system, or through the links
below:

- Android: [ESP BLE Provisioning](https://play.google.com/store/apps/details?id=com.espressif.provble&pcampaignid=web_share)
- iOS: [ESP BLE Provisioning](https://apps.apple.com/us/app/esp-ble-provisioning/id1473590141)

### VSCode, ESP-IDF plugin and ESP-IDF itself

You may have noticed that during the ESP-IDF plugin installation, you also selected the version of
ESP-IDF itself. So how do the ESP-IDF plugin and ESP-IDF itself relate?

ESP-IDF is a **standalone framework** that can be used without VSCode and plugin. It is controlled
from the command line (so it has no GUI) and contains all the logic for building, flashing and
monitoring the application. So when you press e.g. the *Build* button in *ESP-IDF Explorer*
(don't worry, we'll explain what that means in the next lessons), the `idf.py build` command is
called in the end.

The ESP-IDF VSCode plugin is a **wrapper/adapter** that makes ESP-IDF functionality available in
VSCode. In other words, it connects VSCode and the ESP-IDF framework: it creates a GUI in the form
of *ESP-IDF Explorer* and at the same time ensures that the buttons perform their function, the
output of commands is correctly displayed in VSCode, that the framework configuration is correctly
reflected and so on. However, it needs the framework that runs in the background to function.

> **Info:** The ESP-IDF framework itself can be used in other ways than just with the VSCode
> plugin. It has support for [Clion](https://developer.espressif.com/blog/clion/),
> [Eclipse](https://developer.espressif.com/blog/eclipse-plugin-for-esp-idf/), and last but not
> least, you can use it standalone,
> [directly from the command line](https://developer.espressif.com/blog/getting-started-with-esp-idf/).
>
> However, in this workshop we will be using exclusively the **VSCode plugin**.

You should now have a working ESP-IDF plugin for VSCode and ESP-IDF set up. So we can start with
the second part of the tutorial:

[Assignment 2: Create a new project with Components](../assignment-2/)
