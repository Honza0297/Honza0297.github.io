---
title: "Workshop: ESP-IDF a ESP32-C5"
date: 2024-09-30T00:00:00+01:00
layout: default
---

# Workshop: ESP-IDF a ESP32-C5

Vítejte na workshopu o Espressif IoT Development Frameworku (ESP-IDF)!

## O tomto workshopu

Účastí na tomto workshopu získáte hlubší pochopení toho, jak funguje Espressif IoT Development Framework (ESP-IDF) a ESP32-C5 *System on a Chip* (SoC). Díky mnoha praktickým úkolům, které pokrývají řadu základních dovedností, se naučíte vytvářet ještě lepší a efektivnější IoT aplikace pro vaše projekty!

Konkrétně si v tomto workshopu projdeme instalaci frameworku ESP-IDF. Poté se přesuneme k tvorbě vlastního projektu, který si zkusíme sestavit, nahrát a sledovat pomocí terminálu ESP-IDF. V dalších kapitolách se naučíme, jak se připojit k Wi-Fi pomocí provisioningu, provádět OTA aktualizace, komunikovat přes ESP-NOW a nakonec i pár triků pro práci s low-power jádrem, které je součástí SoC ESP32-C5.

> Tento workshop jistě přinese mnoho zajímavých informací i těm, kteří využívají ostatní čipy z rodiny ESP32. Mějte ovšem na paměti, že ne všechny čipy podporují všechno, co podporuje ESP32-C5 (například dual-band Wi-Fi 6 nebo ULP RISC-V jádro)!

## Doprovodné referenční projekty

K tomuto workshopu patří doprovodné ESP-IDF projekty se stejným číslováním úkolů. Jsou na webu ve složce [examples/C5/](../../examples/C5/): u každého projektu je složka se zdrojáky a odkaz na `.zip`. Návod ke sestavení je v [examples/C5/README.md](../../examples/C5/README.md).

## Agenda

Pokud jste splnili [prerekvizity](#prerekvizity), můžeme se pustit do jednotlivých kapitol:

- [Úvod: Představení ESP-IDF](introduction/): Úvodní představení našeho frameworku ESP-IDF, následované základními informacemi o čipu ESP32-C5 a devkitu, na kterém bude tento tutoriál demonstrovaný.
- [Úkol 1: Instalace ESP-IDF](assignment-1/): Instalace nezbytných nástrojů pro začátek vývoje.
- [Úkol 2: Základy: Hello World & LED](assignment-2/): Ověření připojení hardwaru a seznámení se s životním cyklem buildu ESP-IDF v terminálu.
- [Úkol 3: Wi-Fi a provisioning](assignment-3/): Wi-Fi periferie, dual-band a připojení pomocí BLE provisioningu.
- [Úkol 4: OTA aktualizace](assignment-4/): Vzdálené aktualizace kódu pomocí tabulek oddílů a HTTPS.
- [Úkol 5: ESP-NOW chatovací místnost](assignment-5/): Bezspojová komunikace mezi zařízeními.
- [Úkol 6: LP-core](assignment-6/): Ukázka práce s low-power jádrem pro aplikace, kde je spotřeba energie důležitým faktorem.

## Prerekvizity

Abyste zvládli tento workshop, budete potřebovat jak hardwarové, tak softwarové vybavení.

Potřebný hardware:

- Počítač s operačním systémem Linux, Windows nebo macOS
- ESP32-C5-DevKitC nebo ESP32-C5-DevKitM
- USB kabel (podporující napájení + data) kompatibilní s devkitem výše

Potřebný software:

- [Visual Studio Code](https://code.visualstudio.com/download) nebo jiné IDE pro úpravu C/C++. Tento workshop předpokládá použití Visual Studio Code.
- Aplikace **ESP BLE Prov**:
  - Android: [ESP BLE Prov](https://play.google.com/store/apps/details?id=com.espressif.provble&pcampaignid=web_share)
  - iOS: [ESP BLE Prov](https://apps.apple.com/us/app/esp-ble-provisioning/id1473590141)
- ESP-IDF, které se naučíte nainstalovat v [Úkolu 1: Instalace ESP-IDF](assignment-1/)

## Časová náročnost

**Odhadovaný čas: 120 min**

## Feedback

Pokud máte k workshopu jakýkoli feedback, napište mi přímo na jan.beran@espressif.com.

## Závěrem

Doufáme, že vám workshop přinese znalosti, které budou sloužit jako pevný základ pro vaše budoucí projekty. Díky za čas a úsilí, které jste workshopu věnovali, a těšíme se na projekty, které s našimi čipy v budoucnu vytvoříte!

---

[← Zpět na portál workshopů](../../)
