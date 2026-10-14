---
title: "Workshop: ESP-IDF a ESP32-C5 - Úkol 5: ESP-NOW chatovací místnost"
date: 2024-09-30T00:00:00+01:00
layout: default
---

## Úkol 5: ESP-NOW chatovací místnost

---

Wi-Fi se skvěle hodí na připojení k internetu, ale potřebuje přístupový bod (router) a kvůli navazování spojení má režii navíc. Co když chcete, aby si několik desek ESP32 povídalo přímo mezi sebou?

### Co je vlastně ESP-NOW?

**ESP-NOW** je bezspojový protokol s malou režií, který Espressif postavil nad Wi-Fi rádiem. Než sáhneme na kód, hodí se vědět pár věcí:

- **Žádný router, žádný handshake.** Zařízení se „nepřipojují do sítě“ jako u běžné Wi-Fi — není tu asociace, DHCP ani IP adresa. Deska prostě odvysílá malý paket a kdokoli na stejném Wi-Fi kanálu ho může zachytit. Proto je to tak rychlé: mezi voláním odesílací funkce a byty ve vzduchu skoro žádná protokolová režie není.
- **Adresuje se MAC adresou, ne IP.** ESP-NOW o „peeru“ řeknete jeho Wi-Fi MAC adresou (každý čip má vlastní). Můžete použít i speciální broadcast adresu `FF:FF:FF:FF:FF:FF`, kterou přijme každá blízká deska, na které ESP-NOW běží — v podstatě skupinový chat bez serveru.
- **Zprávy jsou krátké a bez pořadí.** Jedno volání `esp_now_send()` uveze nejvýš **250 bajtů** (`ESP_NOW_MAX_DATA_LEN`). Není tu spojení, opakování ani pořadí — když to potřebujete, postavíte si to sami. Na krátký řádek textu mezi deskami 250 bajtů stačí.
- **Šifrovat jde, broadcast ale ne.** Peerům jedna ku jedné se dá zapnout šifrování. Provoz na broadcast adresu je vždy otevřený, protože není s kým sdílet klíč.
- **K čemu se to hodí?** Kdekoli mají blízká zařízení reagovat na sebe hned a bez infrastruktury: dálková ovládání, synchronizovaná světla, hejna senzorů, nebo — jako v tomhle úkolu — místnost plná workshopových desek, které si posílají řádky, co napíšete.

V tomhle úkolu si uděláme chatovací místnost. Zprávy ostatním deskám pošlete ze sériového monitoru — nebo jim budete ovládat LED!

> **Tentokrát začněte novým projektem.** ESP-NOW nepotřebuje Wi-Fi provisioning ani OTA kód z Úkolů 3–4:

```bash
idf.py create-project esp_now_chat
cd esp_now_chat
idf.py set-target esp32c5
idf.py add-dependency "espressif/led_strip^3.0.3"
```

Zkopírujte [`chatroom.c`](../../../examples/C5/assignment-05-esp-now/main/chatroom.c) a [`chatroom.h`](../../../examples/C5/assignment-05-esp-now/main/chatroom.h) do složky `main/`.

Do `main/CMakeLists.txt` přidejte `chatroom.c` a komponentu LED:

```cmake
idf_component_register(SRCS "esp_now_chat.c" "chatroom.c"
                       INCLUDE_DIRS "."
                       REQUIRES led_strip)
```

Nový hlavní soubor (`esp_now_chat.c`) začíná takhle:

```c
#include <stdio.h>

void app_main(void)
{

}
```

`idf.py create-project` pojmenuje zdrojový soubor podle projektu, takže váš je `main/esp_now_chat.c`, ne `main/main.c`.

### 1. Psaní kódu

Jako předtím postavíme `main/esp_now_chat.c` po částech, místo abychom ho vložili celý najednou. Chatovací místnost odsud volá tři funkce, které napíšete vy: broadcast, odeslání na jednu MAC adresu a zapamatování MAC adresy jako peeru. Sama `esp_now_*` nevolá.

#### 1.1 Kostra

```c
#include <stdio.h>

// (A) Headers and the broadcast address
// (B) Receive callback: hand the packet to the chat room
// (C) espnow_broadcast, espnow_send, espnow_add_peer

void app_main(void)
{
    // (D) Initialize NVS
    // (E) Turn the Wi-Fi radio on (station mode, no router)
    // (F) Initialize ESP-NOW, register the receive callback, add the broadcast peer
    // (G) Start the chat room
}
```

#### 1.2 Část (A): hlavičky a broadcast adresa

```c
#include <stdio.h>
#include <string.h>
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_now.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "chatroom.h"

/* Exactly 8 ASCII characters, unique in the room. */
#define THIS_NODE_NAME "node0001" // TODO change to your liking - but keep 8 chars!
_Static_assert(sizeof(THIS_NODE_NAME) == CHATROOM_NAME_LEN + 1,
               "THIS_NODE_NAME must be exactly 8 characters");

static const char *TAG = "espnow";

static const uint8_t s_broadcast_mac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
```

Broadcast MAC adresa je v protokolu ESP-NOW `FF:FF:FF:FF:FF:FF`. Každé další zařízení s ESP-NOW poslouchá zprávy poslané na tuhle adresu. `THIS_NODE_NAME` změňte na **přesně 8 znaků ASCII**, jiné než mají ostatní v místnosti (`node0001`, `node0002`, …).

#### 1.3 Část (B): callback pro příjem

ESP-NOW tuhle funkci zavolá z Wi-Fi úlohy, kdykoli přijde paket. Payload je surový buffer s délkou, ne céčkový řetězec. Tady ho nerozebíráme — to dělá `chatroom_on_recv`, která si z `src_addr` přečte i MAC adresu odesílatele.

```c
static void on_data_recv(const esp_now_recv_info_t *recv_info, const uint8_t *data, int data_len)
{
    if (recv_info == NULL || recv_info->src_addr == NULL) {
        return;
    }
    chatroom_on_recv(recv_info->src_addr, data, data_len);
}
```

#### 1.4 Část (C): odeslání a přidání peeru

`esp_now_send()` předá rádiu `len` bajtů a vrátí se. Nečeká, až je někdo přijme — není tu spojení, na které by se čekalo. Stejné volání je broadcast, nebo zpráva jedné desce, podle toho, jakou MAC adresu předáte.

Odeslání konkrétní desce funguje jen pro MAC adresu zaregistrovanou přes `esp_now_add_peer()`. Chatovací místnost to udělá, když nějakou desku uslyší poprvé. `channel = 0` znamená „kanál, na kterém rádio už je“. U broadcast peeru nelze zapnout `encrypt`.

```c
esp_err_t espnow_broadcast(const uint8_t *data, size_t len)
{
    return espnow_send(s_broadcast_mac, data, len);
}

esp_err_t espnow_send(const uint8_t mac[6], const uint8_t *data, size_t len)
{
    return esp_now_send(mac, data, len);
}

esp_err_t espnow_add_peer(const uint8_t mac[6])
{
    if (esp_now_is_peer_exist(mac)) {
        return ESP_OK;
    }

    esp_now_peer_info_t peer = { 0 };
    memcpy(peer.peer_addr, mac, 6);
    peer.channel = 0;   /* current channel */
    peer.encrypt = false;
    return esp_now_add_peer(&peer);
}
```

#### 1.5 Části (D)–(G): `app_main`

ESP-NOW potřebuje zapnutou Wi-Fi periferii v režimu station, i když se k routeru nikdy nepřipojíme. `esp_wifi_start()` rádio jen zapne. NVS musí být první, stejně jako v úkolech s Wi-Fi, protože ovladač Wi-Fi si do něj ukládá kalibrační data.

Broadcast peer se musí přidat před prvním odesláním, jinak `esp_now_send()` na `FF:FF:FF:FF:FF:FF` skončí chybou „peer not found“. `chatroom_start()` je klávesnice a kód, který vypisuje přijaté řádky; všechno, co vysílá, jde zpátky přes část (C).

```c
void app_main(void)
{
    // (D) Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // (E) Turn the Wi-Fi radio on (station mode, no router)
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());

    // (F) Initialize ESP-NOW, register the receive callback, add the broadcast peer
    ESP_ERROR_CHECK(esp_now_init());
    ESP_ERROR_CHECK(esp_now_register_recv_cb(on_data_recv));

    esp_now_peer_info_t broadcast = { 0 };
    memcpy(broadcast.peer_addr, s_broadcast_mac, 6);
    broadcast.channel = 0;
    broadcast.encrypt = false; /* broadcast traffic cannot be encrypted */
    ESP_ERROR_CHECK(esp_now_add_peer(&broadcast));

    // (G) Start the chat room
    chatroom_start(THIS_NODE_NAME);
    ESP_LOGI(TAG, "ESP-NOW is up. Type in this monitor.");
}
```

#### 1.6 Celý kód

[`examples/C5/assignment-05-esp-now`](../../../examples/C5/assignment-05-esp-now/) má tenhle program v `main/main.c` a k němu dvojici `chatroom.c` / `chatroom.h`. Příklad ten soubor jmenuje `main.c`; `create-project` ten váš pojmenoval `esp_now_chat.c`.

### 2. Build, flash a monitor

```bash
idf.py build flash monitor
```

`idf.py monitor` je sériový terminál oběma směry. Řádky, které deska vypíše, se ukážou tady, a klávesy, které stisknete, se pošlou na desku (kromě `Ctrl-]`, kterým monitor skončí, a `Ctrl-T`, kterým se otevře jeho menu). Deska znaky, které píšete, opisuje zpátky. Zpráva od někoho jiného se může vypsat doprostřed rozepsaného řádku; deska přesto dostane jen vaše stisky a Enter odešle řádek, který jste napsali.

Nahrajte druhou desku s jiným jménem, třeba `node0001` na vaší a `node0002` na jejich.

- Napište `hello` a stiskněte Enter. Jejich monitor vypíše `User node0001: hello`.
- Napište `red` (nebo `green`, `blue`, `white`, `on`, `off`). Jejich LED se změní a monitor vypíše `User node0001 set your LED to 20,0,0` (čísla, ne slovo). Každá složka je 0–255. Tahle slova jsou řádky `COMMAND_RED` a podobné na začátku `chatroom.c`; nové slovo je nový řádek v té tabulce.
- Až jejich deska něco odešle, napište `node0002:hello` nebo `node0002:blue` a dosáhnete jen na ni. Do té doby monitor hlásí, že takový kontakt není. ESP-NOW si pamatuje 20 peerů a jeden slot zabírá broadcast adresa, takže místnost udrží 19 jmen.

Řádek, který není ASCII, se odmítne. Paket delší než 250 bajtů se před odesláním ořízne. Deska svůj vlastní broadcast nepřijme.

## Další krok

Podíváme se na poslední vlastnost ESP32-C5: low-power jádro!

[Úkol 6: LP-core](../assignment-6/)
