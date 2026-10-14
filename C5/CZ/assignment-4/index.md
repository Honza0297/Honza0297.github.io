---
title: "Workshop: ESP-IDF a ESP32-C5 - Úkol 4: OTA aktualizace"
date: 2024-09-30T00:00:00+01:00
layout: default
---

## Úkol 4: Aktualizace na dálku (OTA)

---

> **Poznámka:** Tohle je volitelná kapitola. Pokud nestíháte, můžete přeskočit rovnou na [Úkol 5: ESP-NOW chatovací místnost](../assignment-5/).

V reálném nasazení nebudete mít fyzický přístup k USB portu, abyste nahráli nový firmware. Aktualizace Over-The-Air (OTA) umožňují stáhnout a nainstalovat nový firmware přes síť, například Wi-Fi.

**Jak OTA na ESP32 funguje v kostce:** ve flashi nejsou jeden, ale dva aplikační sloty. Zařízení vždy běží z jednoho z nich (*aktivního* slotu) a ten nikdy nepřepisuje. Nový firmware se zapíše do druhého, neaktivního slotu, a teprve když je celý stažený a ověřený, zařízení přepne malý záznam „bootuj z tohoto slotu“ a restartuje se do něj. Když se během stahování něco pokazí, starý firmware je pořád neporušený a běží dál. Když je zapnutý rollback (viz bonus na konci), jde bootloader ještě o krok dál: pokud nový firmware po startu nepotvrdí, že funguje, bootloader se vrátí k předchozímu slotu, takže špatná aktualizace zařízení neznefunkční.

> **Pokračujte ve stejném projektu.** Pokračujte v projektu, který jste v Úkolu 3 naprovisionovali — OTA potřebuje zařízení, které se už umí připojit k Wi-Fi. Pokud ten krok uložil přihlašovací údaje do NVS, použijí se; pokud ne, deska se připojí k přístupovému bodu `ESP32`. Druhá deska, na kterou nahrajete serverový projekt, tento přístupový bod i aktualizační soubor poskytuje. `main.c` znovu nahradíme, ale `idf_component.yml` nechte být: už z Úkolu 2 tahá `led_strip` a za chvíli ho zase budeme potřebovat.

### 1. Tabulky oddílů (partition tables)

Aby OTA fungovalo, musí být flash paměť ESP32-C5 rozdělená na konkrétní oblasti, kterým se říká partitions. Typická OTA partition tabulka obsahuje:
- **nvs**: Non-Volatile Storage pro přihlašovací údaje k Wi-Fi a kalibrační data.
- **otadata**: ukládá, která aplikační partition je právě aktivní.
- **phy_init**: továrně kalibrovaná data rádia.
- **ota_0**: první aplikační slot — sem se firmware nahraje poprvé.
- **ota_1**: druhý aplikační slot, kam se stahuje nový firmware.

Při OTA aktualizaci zařízení stáhne nový firmware do neaktivní partition (té z `ota_0`/`ota_1`, která právě neběží). Když stahování dopadne, zapíše do partition `otadata`, že se má bootovat odtud, a restartuje se.

Tohle rozložení definujeme v souboru `partitions.csv` v kořeni projektu:

```text
# Name,   Type, SubType, Offset,  Size, Flags
nvs,      data, nvs,     ,        0x4000,
otadata,  data, ota,     ,        0x2000,
phy_init, data, phy,     ,        0x1000,
ota_0,    app,  ota_0,   ,        1500K,
ota_1,    app,  ota_1,   ,        1500K,
```

1500K na slot dává kódu OTA/TLS/Wi-Fi/LED v tomhle úkolu pohodlnou rezervu (skutečný binární soubor má zhruba 1,1 MB) a na 4MB flash se přitom vejdou oba sloty.

> **Tip:** Pokud nechcete CSV upravovat ručně, rozšíření ESP-IDF pro VS Code má GUI editor jak pro partition tabulku, tak pro obsah NVS.
>
> ![Editor partition tabulky v ESP-IDF](assets/nvs-1.webp)
>
> ![Editor NVS partition v ESP-IDF](assets/nvs-2.webp)

### 2. Konfigurace pomocí menuconfig

Aby ESP-IDF použilo naši partition tabulku, spusťte:

```bash
idf.py menuconfig
```

Přejděte do:
- **Partition Table**
  - **Partition Table**: vyberte `Custom partition table CSV`
  - **Custom partition CSV file**: napište `partitions.csv`
- **Serial Flasher Config**
  - **Flash size**: vyberte `4 MB` — dva sloty po 1500K plus `nvs`/`otadata`/`phy_init` se už nevejdou do výchozího předpokladu 2 MB.
- **Component config**
  - **ESP HTTPS OTA**
    - **Allow HTTP for OTA**: zapněte. Server v tomhle úkolu mluví čistým HTTP a `esp_https_ota()` odmítne URL s `http://`, dokud tahle volba není zapnutá. Nechte ji vypnutou, pokud později nasměrujete `.url` na skutečný HTTPS server.

Ukončete klávesou `Q` a uložte klávesou `Y`.

> Pokud nechcete menuconfig proklikávat, vložte tahle nastavení do souboru `sdkconfig.defaults` v kořeni projektu:
> ```text
> CONFIG_PARTITION_TABLE_CUSTOM=y
> CONFIG_PARTITION_TABLE_CUSTOM_FILENAME="partitions.csv"
> CONFIG_PARTITION_TABLE_FILENAME="partitions.csv"
> CONFIG_ESPTOOLPY_FLASHSIZE_4MB=y
> CONFIG_ESP_HTTPS_OTA_ALLOW_HTTP=y
> ```
>
> `CONFIG_ESP_HTTPS_OTA_ALLOW_HTTP` dovolí `esp_https_ota()` stahovat z čistého HTTP serveru na druhé desce. Nechte ji vypnutou, pokud později nasměrujete `.url` na skutečný HTTPS server.

### 3. Připojení tlačítka

Místo aby se deska aktualizovala hned po startu, počká, až stisknete fyzické tlačítko, a teprve pak začne něco stahovat. Je to bezpečnější (žádné překvapivé stahování uprostřed ukázky) a dobrá příležitost naučit se základní vstup na GPIO. Tohle samé tlačítko budeme znovu používat v Úkolech 5 a 6, takže se vyplatí zapojit ho pořádně hned teď.

Stačí libovolné jednoduché taktilní tlačítko se 2 nebo 4 vývody. U čtyřvývodového tlačítka jsou dva vývody na každé kratší straně uvnitř propojené:

![Zapojení taktilního tlačítka — dva vývody na každé straně jsou uvnitř propojené](assets/button.webp)

Zapojte ho takto:

- jeden vývod tlačítka -> **3V3**
- druhý vývod tlačítka -> **GPIO0**
- pull-down rezistor (např. 10 kΩ) mezi **GPIO0** a **GND**

Se stahovacím rezistorem čte `GPIO0` **LOW**, když je tlačítko puštěné, a **HIGH**, když je stisknuté (tlačítko ho tehdy spojí přímo s 3V3).

> `GPIO0` je na mnoha čipech ESP32 také strapping pin pro boot režim, ale to hraje roli jen v krátkém okamžiku při zapnutí nebo resetu. Stisk tlačítka za běhu (a nic jiného tady neděláme) na to nemá vliv.

### 4. Píšeme kód

Jako předtím to postavíme po částech, místo abychom všechno vložili najednou.

#### 4.1 Kostra

```c
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_https_ota.h"
#include "esp_http_client.h"
#include "esp_wifi.h"
#include "esp_crt_bundle.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "led_strip.h"

#define BUTTON_GPIO       GPIO_NUM_0
#define LED_GPIO          27
#define FALLBACK_SSID     "ESP32"
#define FALLBACK_PASSWORD "ESP32"

static const char *TAG = "ota_example";

// (A) LED helpers - reused from Assignment 2
// (B) Button GPIO setup
// (C) Wi-Fi event handler - also updates the LED
// (D) A task that polls the button and wakes up the OTA task
// (E) The OTA task itself - now waits for a notification instead of running immediately
// (F) app_main wires everything together
```

#### 4.2 Část (A)-(B): nastavení LED a tlačítka

```c
static led_strip_handle_t led_strip;
static TaskHandle_t s_ota_task_handle = NULL;

static void led_set_color(uint8_t r, uint8_t g, uint8_t b)
{
    led_strip_set_pixel(led_strip, 0, r, g, b);
    led_strip_refresh(led_strip);
}

static void configure_led(void)
{
    led_strip_config_t strip_config = {
        .strip_gpio_num = LED_GPIO,
        .max_leds = 1,
        .color_component_format = LED_STRIP_COLOR_COMPONENT_FMT_GRB,
        .led_model = LED_MODEL_WS2812,
        .flags.invert_out = false,
    };
    led_strip_rmt_config_t rmt_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = 10 * 1000 * 1000,
        .mem_block_symbols = 0,
        .flags.with_dma = false,
    };
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));
    led_strip_clear(led_strip);
}

static void configure_button(void)
{
    gpio_config_t io_conf = {
        .pin_bit_mask = 1ULL << BUTTON_GPIO,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE, // we use an external pull-down resistor
    };
    ESP_ERROR_CHECK(gpio_config(&io_conf));
}
```

LED použijeme jako stavovou kontrolku s jednoduchým barevným kódem, který v tomhle úkolu postupně doplníme:
- **Žlutá** — připojuje se / znovu se připojuje k Wi-Fi
- **Modrá** — připraveno, čeká na tlačítko
- **Bílá** — stahuje aktualizaci
- **Zelená** — aktualizace se povedla, za chvíli restart
- **Červená** — aktualizace selhala, za pár sekund zpátky na modrou

#### 4.3 Část (C): Wi-Fi event handler

```c
static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGI(TAG, "Disconnected. Connecting to the AP again...");
        led_set_color(20, 20, 0); // yellow: (re)connecting
        esp_wifi_connect();
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "Connected with IP Address:" IPSTR, IP2STR(&event->ip_info.ip));
        ESP_LOGI(TAG, "Ready. Press the button to start the OTA update.");
        led_set_color(0, 0, 20); // blue: ready, waiting for the button
    }
}
```

Je to stejná logika opětovného připojení k Wi-Fi, kterou už znáte, jen rozšířená o přepínání LED na žlutou nebo modrou podle stavu spojení.

#### 4.4 Část (D): úloha tlačítka

```c
// Polls the button and wakes ota_task up on a press. A dedicated task keeps
// the debounce logic simple and out of the way of everything else.
static void button_task(void *pvParameter)
{
    bool was_pressed = false;
    while (1) {
        bool pressed = gpio_get_level(BUTTON_GPIO) == 1;
        if (pressed && !was_pressed) {
            vTaskDelay(pdMS_TO_TICKS(50)); // simple debounce
            if (gpio_get_level(BUTTON_GPIO) == 1) {
                ESP_LOGI(TAG, "Button pressed - triggering OTA update");
                xTaskNotifyGive(s_ota_task_handle);
            }
        }
        was_pressed = pressed;
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}
```

Místo GPIO přerušení (na začátek se v něm dá snadno splést) tahle úloha pin jen každých 20 ms kontroluje. Když signál přejde z nízké úrovně na vysokou, počká 50 ms a znovu ověří, že pin je pořád ve vysoké úrovni — jednoduchý a účinný způsob, jak ignorovat drobné elektrické odskoky mechanického spínače — a potom vzbudí `ota_task` pomocí `xTaskNotifyGive`.

#### 4.5 Část (E): OTA úloha

```c
static void ota_task(void *pvParameter)
{
    while (1) {
        // Block here until button_task wakes us up - no auto-update on boot anymore!
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        ESP_LOGI(TAG, "Starting OTA update");
        led_set_color(20, 20, 20); // white: downloading

        esp_http_client_config_t config = {
            .url = "http://192.168.4.1/firmware.bin",
            // In production, serve this over HTTPS and check the server certificate:
            // .crt_bundle_attach = esp_crt_bundle_attach,
        };
        esp_https_ota_config_t ota_config = {
            .http_config = &config,
        };

        ESP_LOGI(TAG, "Attempting to download update from %s", config.url);
        esp_err_t ret = esp_https_ota(&ota_config);
        if (ret == ESP_OK) {
            ESP_LOGI(TAG, "OTA succeeded, rebooting...");
            led_set_color(0, 20, 0); // green: success
            vTaskDelay(pdMS_TO_TICKS(1000));
            esp_restart();
        } else {
            ESP_LOGE(TAG, "Firmware upgrade failed");
            led_set_color(20, 0, 0); // red: failed
            vTaskDelay(pdMS_TO_TICKS(3000));
            led_set_color(0, 0, 20); // back to blue: ready, feel free to try again
        }
    }
}
```

`ulTaskNotifyTake(pdTRUE, portMAX_DELAY)` spí, dokud ho `button_task` neprobudí, místo aby stahování začalo hned po startu. URL je serverový projekt [`examples/C5/assignment-04-ota-server`](../../../examples/C5/assignment-04-ota-server/): ta deska je přístupový bod `ESP32` a poskytuje image na `http://192.168.4.1/firmware.bin`. `esp_https_ota()` odmítne čisté HTTP, dokud není nastaveno `CONFIG_ESP_HTTPS_OTA_ALLOW_HTTP`. V produktu by se image mělo podávat přes HTTPS a nastavit `.crt_bundle_attach`.

#### 4.6 Část (F): `app_main`

```c
void app_main(void)
{
    ESP_LOGI(TAG, "Running firmware v1!");

    // Initialize NVS
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    configure_led();
    configure_button();
    led_set_color(20, 20, 0); // yellow: connecting

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));

    // Prefer credentials stored in NVS by the Assignment 3 provisioning step.
    // If that never happened, connect to the fallback network instead.
    wifi_config_t wifi_config = { 0 };
    ESP_ERROR_CHECK(esp_wifi_get_config(WIFI_IF_STA, &wifi_config));
    if (wifi_config.sta.ssid[0] == '\0') {
        ESP_LOGW(TAG, "No Wi-Fi credentials in NVS, using fallback SSID \"%s\"", FALLBACK_SSID);
        wifi_config_t fallback = {
            .sta = {
                .ssid = FALLBACK_SSID,
                .password = FALLBACK_PASSWORD,
                .threshold.authmode = WIFI_AUTH_WEP, // "ESP32" is 5 characters; WPA2 needs 8
            },
        };
        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &fallback));
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());

    // Create the OTA task first so button_task has a valid handle to notify
    xTaskCreate(&ota_task, "ota_task", 8192, NULL, 5, &s_ota_task_handle);
    xTaskCreate(&button_task, "button_task", 2048, NULL, 5, NULL);
}
```

`esp_wifi_get_config()` přečte konfiguraci stanice, kterou Wi-Fi ovladač obnovil z NVS. Uložené SSID nechá být. Prázdné SSID znamená, že deska nikdy nebyla naprovisionovaná (nebo že se NVS smazalo), takže `esp_wifi_set_config()` nastaví `FALLBACK_SSID` a `FALLBACK_PASSWORD` — obojí `"ESP32"` — a ovladač to uloží do NVS stejně jako naprovisionovanou konfiguraci.

#### 4.7 Celý kód

Celý soubor je v [`examples/C5/assignment-04-ota`](../../../examples/C5/assignment-04-ota/) spolu s odpovídajícími `partitions.csv` a `idf_component.yml`. Můžete si ho odtud vzít, pokud nechcete všechno přepisovat, nebo podle něj zkontrolovat svou práci.

### 5. Build, flash a monitor

```bash
idf.py build flash monitor
```

Nejdřív nahrajte [serverový projekt](../../../examples/C5/assignment-04-ota-server/) na druhou desku. Sledujte LED studentské desky: **žlutá**, dokud se připojuje, potom **modrá**, jakmile Wi-Fi naběhne. Když v NVS nejsou přihlašovací údaje, připojí se k přístupovému bodu serveru `ESP32`. Stiskněte tlačítko — LED zbělá, dokud se image stahuje, potom zezelená a deska se restartuje do toho image. V monitoru by se pak mělo objevit `Running firmware v2!`. Když je serverová deska vypnutá, nebo tahle deska pořád používá síť uloženou v Úkolu 3, LED zčervená a vrátí se na modrou. Jak ten image vznikl a jak ho vyměnit, je v [kroku 7](#7-kde-lezi-novy-firmware).

### 7. Kde leží nový firmware {#7-kde-lezi-novy-firmware}

> Pokud tenhle workshop děláte s lektorem, řekne vám, kde firmware najít, a tuhle kapitolu můžete brát jen jako informaci. Pokud ho děláte sami, pokračujte.

Image, který tlačítko stahuje, už je sestavený. Leží v serverovém projektu jako `firmware.bin` a je to tentýž program, jen s logovacím řádkem změněným na `Running firmware v2!`. Nahrajte ten projekt na druhou desku:

```bash
cd examples/C5/assignment-04-ota-server
idf.py build flash monitor
```

Server otevře přístupový bod se jménem `ESP32` a heslem `ESP32` a soubor poskytuje na `http://192.168.4.1/firmware.bin`. Studentská deska se k tomu přístupovému bodu připojí jen tehdy, když v NVS nemá přihlašovací údaje k Wi-Fi. Pokud Úkol 3 už nějakou síť uložil, před touhle ukázkou smažte flash a studentský projekt nahrajte znovu, jinak deska zůstane na té síti a na `192.168.4.1` se nikdy nedostane.

**Výměna image.** Změňte řádek s verzí ve studentském projektu, sestavte ho a `build/assignment_04_ota.bin` zkopírujte přes `firmware.bin` v serverovém projektu. Server znovu nahrajte. Image musí být sestavený pro ESP32-C5 a musí se vejít do partition `fw` na serveru (1536 KB).

**Hostování na veřejném internetu.** Dejte `.bin` někam, kde už je HTTPS od veřejné certifikační autority (odkaz na `raw.githubusercontent.com`, nebo objekt na S3/GCS s veřejným čtením). Odkomentujte `.crt_bundle_attach = esp_crt_bundle_attach`, zrušte `CONFIG_ESP_HTTPS_OTA_ALLOW_HTTP` a `.url` namiřte na tu `https://` adresu. Certifikát podepsaný sám sebou v tom balíku není — pokud potřebujete lokální HTTPS server, vložte ho přes `.cert_pem`.

## Další krok

Pojďme se podívat na úplně jinou bezdrátovou technologii: ESP-NOW! Tlačítko a LED, které jste právě zapojili, jdou s námi.

[Úkol 5: ESP-NOW chatovací místnost](../assignment-5/)
