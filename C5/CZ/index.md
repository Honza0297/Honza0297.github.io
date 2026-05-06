---
title: "Workshop: ESP-IDF a ESP32-C5"
date: 2024-09-30T00:00:00+01:00
layout: default
---

Vítejte na workshopu o Espressif IoT Development frameworku (ESP-IDF)!

## O tomto workshopu

Účastí na tomto workshopu získáte hlubší pochopení toho, jak funguje Espressif IoT Development Framework (ESP-IDF) a ESP32-C5 *System on a Chip* (SoC). Díky mnoha praktickým úkolům, které vám pomůžou s různými základními dovednosti potřebnými pro programování (nejen) ESP32-C5, se naučíte vytvářet ještě lepší a efektivnější IoT aplikace pro vaše projekty!

Konkrétně si v tomto workshopu projdeme instalaci frameworku ESP-IDF a ESP-IDF pluginu do VSCode. Poté se přesuneme k tvorbě vlastního projektu, který si zkusíme sestavit, nahrát a debugovat. V dalších kapitolách se naučíme, jak ve vašich projektech používat komponenty, blikat s adresovatelnou LED, konfigurovat ESP-IDF, připojit se k Wi-Fi a nakonec i pár triků pro práci s low-power jádrem, které je součástí SoC ESP32-C5.

> Tento workshop jistě přinese mnoho zajímavých informací i těm, kteří využívají ostatní čipy z rodiny ESP32. Mějte ovšem na paměti, že ne všechny čipy podporují všechno, co podporuje ESP32-C5!

## Doprovodné referenční projekty / Companion reference projects

V repozitáři jsou i malé ESP-IDF projekty se stejným číslováním úkolů jako tento workshop (plus volitelný příklad protokolů/TLS). Jsou součástí tohoto repozitáře ve složce [examples/C5/](../../examples/C5/) — procházejte je přímo na GitHubu nebo naklonujte repozitář a spusťte `build-all.sh`. Popis je v [examples/C5/README.md](../../examples/C5/README.md).

## Agenda

Pokud jste splnili [prerekvizity](#prerekvizity) můžeme se pustit do jednotlivých kapitol:

- [Úvod: Představení ESP-IDF](introduction/): Úvodní představení našeho frameworku ESP-IDF, následované základními informacemi o čipu ESP32-C5 a devkitu, na kterém bude tento tutoriál demonstrovaný.
- [Úkol 1: Instalace ESP-IDF a VSCode](assignment-1): Instalace nezbytných nástrojů pro začátek vývoje.
- [Úkol 2: Vytvoření nového projektu s Komponenty](assignment-2): Jak efektivně vytvořit, strukturovat a organizovat projekt.
- [Úkol 3: Připojení k Wi-Fi](assignment-3): Naprostý základ pro většinu IoT projektů.
- [Úkol 4: Zkouška NVS (Non-Volatile Storage)](assignment-4): Ukládání perzistentních dat.
- [Úkol 5: Wi-Fi provisioning](assignment-5): Podrobnější práce s Wi-Fi: nastavování a konfigurace Wi-Fi sítí.
- [Úkol 6: Experimenty s low-power core](assignment-6): Ukázka práce s low-power core pro aplikace, kde je spotřeba energie důležitým faktorem.


## Prerekvizity

Abyste zvládli tento workshop, budete potřebovat jak hardwarové, tak softwarové vybavení.

Potřebný hardware:

- Počítač s operačním systémem Linux, Windows nebo macOS
- ESP32-C5-DevKitC nebo ESP32-C5-DevKitM
- USB kabel (podporující napájení + data) kompatibilní s devkitem výše
- Visual Studio Code s ESP-IDF pluginem (instalace je popsaná v další části)

Potřebný software:

- [Visual Studio Code](https://code.visualstudio.com/download)
- [ESP-IDF plugin pro VS Code](https://github.com/espressif/vscode-esp-idf-extension?tab=readme-ov-file#how-to-use)
- Aplikace **ESP BLE Prov**:
    - Android: [ESP BLE Prov](https://play.google.com/store/apps/details?id=com.espressif.provble&pcampaignid=web_share)
    - iOS: [ESP BLE Prov](https://apps.apple.com/us/app/esp-ble-provisioning/id1473590141)
## Časová náročnost

**Odhadovaný čas: 180 min**


## Feedback

Pokud máte k workshopu jakýkoli feedback, neváhejte založit novou [diskuzi na GitHubu](https://github.com/espressif/developer-portal/discussions).


## Závěrem

Doufáme, že vám workshop přinese znalosti, které budou sloužit jako pevný základ pro vaše budoucí projekty. Díky za čas a úsilí, které jste workshopu věnovali, a těšíme se na projekty, které s našimi čipy v budoucnu vytvoříte!

---
