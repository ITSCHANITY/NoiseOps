![WhatsApp Image 2026-02-07 at 1 03 42 AM (1)](https://github.com/user-attachments/assets/6f943263-805b-42b5-a672-a311f1e12852)

<img width="730" height="424" alt="ezgif-87e0d5184e1fcf6c" src="https://github.com/user-attachments/assets/593a7b77-f164-40a6-9153-d52cefb520a6" />

<img width="732" height="412" alt="ezgif-8a28a8ec069a4567" src="https://github.com/user-attachments/assets/ec7b858d-b7ce-449f-81c9-e9eb2bc9c4f7" />

# NoiseOps

NoiseOps is an open-source, Flipper Zero–style multi-tool built around the **ESP32-S3 (N16R8)**. It is a portable pentesting and embedded-engineering platform with an OLED UI driven by six buttons.

> **Note:** Some sketch, binary, and PCB files in this repository keep legacy file and folder names inherited from the original codebase this project was adapted from. Those names are left unchanged only so the build and flashing steps work as written — "NoiseOps" is the project name.

> **Disclaimer:** NoiseOps is intended for education, research, and testing on hardware you own or are authorized to test. Transmitting on RF bands and interacting with wireless devices may be regulated in your region. You are responsible for using it legally and responsibly.

This README is a two-part guide:

1. **[Build Guide](#part-1--build-guide)** — the parts, wiring, PCB, and how to flash the firmware.
2. **[User Guide](#part-2--user-guide)** — how to navigate and use each feature.

## Table of contents

- [Part 1 — Build Guide](#part-1--build-guide)
  - [1.1 What you need](#11-what-you-need)
  - [1.2 Pinout / wiring](#12-pinout--wiring)
  - [1.3 PCB](#13-pcb-optional)
  - [1.4 Install the Arduino toolchain](#14-install-the-arduino-toolchain)
  - [1.5 Install Arduino libraries](#15-install-arduino-libraries)
  - [1.6 Screen driver selection](#16-screen-driver-selection)
  - [1.7 Flashing the firmware](#17-flashing-the-firmware)
  - [1.8 First boot check](#18-first-boot-check)
- [Part 2 — User Guide](#part-2--user-guide)
  - [2.1 Controls and navigation](#21-controls-and-navigation)
  - [2.2 Main menu](#22-main-menu)
  - [2.3 Sub-GHz (CC1101)](#23-sub-ghz-cc1101)
  - [2.4 NFC (PN532)](#24-nfc-pn532)
  - [2.5 Wi-Fi](#25-wi-fi)
  - [2.6 Bluetooth LE](#26-bluetooth-le)
  - [2.7 GPIO utilities](#27-gpio-utilities)
  - [2.8 Bad USB (HID)](#28-bad-usb-hid)
  - [2.9 Apps](#29-apps)
  - [2.10 Settings](#210-settings)
- [Project layout](#project-layout)
- [Contributing](#contributing)
- [Credits](#credits)

---

# Part 1 — Build Guide

## 1.1 What you need

### Core board
- **ESP32-S3 N16R8** (16 MB flash / 8 MB PSRAM) — this is the required target chip.

### Display and controls
- **1.3" OLED (SH1106, 128×64, I2C)** — the firmware ships configured for the SH1106 controller used by most 1.3" panels.
  - If you use a **0.96"/1.54" SSD1306** panel instead, you must switch the display driver line in the firmware (see [1.6](#16-screen-driver-selection)).
- **6 push buttons** — Up, Down, Select, Back, Left, Right.

### Radios / peripherals
- **CC1101 433 MHz transceiver** — for the Sub-GHz tools.
- **PN532 NFC/RFID module** — for NFC. Must be set to **I2C mode** (see [1.5](#15-install-arduino-libraries)).
- **WS2812 / NeoPixel** single RGB LED (on-board status LED).

> **Note on optional modules:** The original hardware design also supports two nRF24L01 radios, a microSD card, and an IR transmitter/receiver. This firmware build has those menus hidden because they are not part of the core kit. The device runs fine without them. If you add them later, the pin assignments below still apply.

## 1.2 Pinout / wiring

Wire everything to the ESP32-S3 exactly as follows. **These pins are fixed in the firmware — do not change them.**

```text
ESP32-S3
│
├── I2C  (OLED + PN532 NFC share this bus)
│   ├── SDA  → GPIO8
│   └── SCL  → GPIO9
│
├── IR   (optional)
│   ├── TX   → GPIO35
│   └── RX   → GPIO36
│
├── nRF24 #1  (optional)
│   ├── CE   → GPIO10
│   ├── CSN  → GPIO11
│   ├── SCK  → GPIO18
│   ├── MISO → GPIO16
│   └── MOSI → GPIO17
│
├── nRF24 #2  (optional)
│   ├── CE   → GPIO12
│   ├── CSN  → GPIO13
│   ├── SCK  → GPIO18
│   ├── MISO → GPIO16
│   └── MOSI → GPIO17
│
├── CC1101 #1
│   ├── CS   → GPIO45
│   ├── GDO0 → GPIO21
│   ├── GDO2 → GPIO47
│   ├── SCK  → GPIO15
│   ├── MISO → GPIO3
│   └── MOSI → GPIO46
│
├── CC1101 #2  (optional)
│   ├── CS   → GPIO40
│   ├── GDO0 → GPIO41
│   ├── GDO2 → GPIO42
│   ├── SCK  → GPIO15
│   ├── MISO → GPIO3
│   └── MOSI → GPIO46
│
├── SD Card  (optional)
│   ├── CS   → GPIO37
│   ├── SCK  → GPIO14
│   ├── MISO → GPIO39
│   └── MOSI → GPIO38
│
├── RGB LED (NeoPixel) → GPIO48
│
└── Buttons
    ├── UP     → GPIO4
    ├── DOWN   → GPIO5
    ├── SELECT → GPIO6
    ├── BACK   → GPIO7
    ├── LEFT   → GPIO1
    └── RIGHT  → GPIO2
```

### Wiring tips
- The **OLED and PN532 both sit on the same I2C bus** (GPIO8/GPIO9). Connect both their SDA lines to GPIO8 and both SCL lines to GPIO9. Each has a different I2C address, so they coexist fine.
- Buttons are wired to ground; the firmware enables internal pull-ups (`INPUT_PULLUP`), so a button reads LOW when pressed. No external resistors needed.
- Give the CC1101 and PN532 a stable 3.3 V supply. Use short, clean wiring for the RF modules.

## 1.3 PCB (optional)

A ready-made PCB design is included under `HIZMOS-PCB-C/` as Gerber and drill files, plus an interactive BOM (`InteractiveBOM_HIZMOS.html`). You can order it from a fab house that accepts Gerbers. See `HIZMOS-PCB-C/How-to-order-PCB.txt` and the EasyEDA ordering docs: https://docs.easyeda.com/en/PCB/Order-PCB

If you are building on a breadboard or protoboard instead, just follow the pinout in [1.2](#12-pinout--wiring).

## 1.4 Install the Arduino toolchain

1. Install the **Arduino IDE** (2.x recommended).
2. Add ESP32 board support: open **File → Preferences**, add this to *Additional Boards Manager URLs*:
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
3. Open **Tools → Board → Boards Manager**, search **esp32**, and install the **Espressif Systems ESP32** package.
4. Select the board **ESP32S3 Dev Module** and set:
   - **Flash Size:** 16 MB
   - **PSRAM:** OPI PSRAM (matches the N16R8)
   - **USB CDC On Boot:** Enabled (needed for the Bad USB / HID features)

## 1.5 Install Arduino libraries

Install these through **Tools → Manage Libraries** (Library Manager):

| Library | Used for |
|---|---|
| **U8g2** | OLED display |
| **Adafruit PN532** | NFC reader (also pulls in **Adafruit BusIO**) |
| **RF24** | nRF24 radios (optional modules) |
| **Adafruit NeoPixel** | Status RGB LED |
| **IRremote** | Infrared (optional modules) |

The CC1101 driver (`ELECHOUSE_CC1101_SRC_DRV`) is already included in the sketch folder — no separate install needed.

> **PN532 mode:** Set your PN532 board's DIP switches / solder jumpers to **I2C** mode. The firmware talks to it over I2C at address `0x24`. If your board uses a different address, it will report "PN532 not found."

## 1.6 Screen driver selection

Open `HIZMOS_OLED_U8G2lib/HIZMOS_OLED_U8G2lib.ino`. Near the top you will find the display declaration:

- **For a 1.3" OLED (default, already active):**
  ```cpp
  U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);
  ```
- **For a 0.96" / 1.54" OLED:** comment out the SH1106 line above and uncomment the SSD1306 line:
  ```cpp
  U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);
  ```

Leave exactly one of these two lines active.

## 1.7 Flashing the firmware

You have two options.

### Option A — Compile and upload from Arduino IDE
1. Open `HIZMOS_OLED_U8G2lib/HIZMOS_OLED_U8G2lib.ino`.
2. Confirm the board settings ([1.4](#14-install-the-arduino-toolchain)) and the correct screen driver ([1.6](#16-screen-driver-selection)).
3. Plug in the ESP32-S3, select its port under **Tools → Port**, and click **Upload**.

### Option B — Flash the prebuilt binaries (ESP Web Tool)
Prebuilt binaries are provided for both screen types:
- `SH1106_HIZMOS_BIN_FILES/` — for the 1.3" OLED
- `SSD1306_HIZMOS_BIN_FILES/` — for the 0.96"/1.54" OLED

Flash with the browser-based tool:

1. Open https://esptool.spacehuhn.com/
2. Click **Connect** and select the ESP32-S3 serial port.
3. Click **Add File** and add the three binaries at these addresses:

   | Address | File |
   |----------|----------|
   | `0x0000` | `HIZMOS_OLED_U8G2lib.ino.bootloader.bin` |
   | `0x8000` | `HIZMOS_OLED_U8G2lib.ino.partitions.bin` |
   | `0x10000` | `HIZMOS_OLED_U8G2lib.ino.bin` |

4. Enable **Erase Before Flashing** (recommended).
5. Click **Program** and wait for it to finish, then reboot the device.

> Target chip: **ESP32-S3 (N16R8)**

## 1.8 First boot check

On power-up, NoiseOps runs a quick self-test and shows the status of your core hardware:

- **oled** — should read `work` (if you can see the screen, it works).
- **cc1101** — reads `work` when the CC1101 responds on SPI.
- **nfc** — reads `work` when the PN532 responds on I2C.

If a module shows `error`, re-check its wiring and power before continuing. You can re-run this test any time from **Settings → check sys devices**.

---

# Part 2 — User Guide

## 2.1 Controls and navigation

The whole UI is driven by six buttons:

| Button | Action |
|---|---|
| **Up / Down** | Move the selection through a menu |
| **Left / Right** | Adjust values (e.g. frequency, power) in some screens |
| **Select** | Open the highlighted item / confirm |
| **Back** | Leave the current screen and return to the previous menu |

When the device is idle on the home screen it plays an animation. Pressing any button brings up the main menu. From the main menu, scroll with Up/Down and press Select to enter a tool; press Back to leave a tool.

## 2.2 Main menu

This build shows the features supported by the core kit:

| Item | What it does |
|---|---|
| **subghz** | CC1101 433 MHz Sub-GHz tools |
| **gpio** | Hardware utilities (I2C scanner, button tester, etc.) |
| **wifi** | Wi-Fi scanning (ESP32-S3 built-in radio) |
| **ble** | Bluetooth LE tools (ESP32-S3 built-in radio) |
| **apps** | Standalone mini-apps and games |
| **settings** | Device info and configuration |
| **nfc** | PN532 NFC/RFID tools |
| **bad usb** | USB HID keyboard scripting (ESP32-S3 native USB) |

## 2.3 Sub-GHz (CC1101)

The Sub-GHz section uses the CC1101 for receiving and analyzing 433 MHz (and other ISM-band) signals.

- **Frequency analyzer / spectrum** — hops across the most common remote-control frequencies, locks onto the strongest signal, and shows its frequency and RSSI.
- **Histogram** — sweeps a band around the current frequency and draws signal strength as bars.
- **RF settings** — adjust:
  - **Modulation:** ASK/OOK, 2-FSK, GFSK, MSK (ASK/OOK is the default for most remotes)
  - **Frequency:** step through presets (300–925 MHz) or fine-tune with Left/Right
  - **Hopping:** auto-cycle through preset frequencies
  - **RX bandwidth** and **TX power**

Serial output (9600 baud) prints detailed logs for each action, which is handy while scanning.

## 2.4 NFC (PN532)

Enter **nfc** from the main menu.

- **Read tag** — hold a tag near the PN532 and the screen shows the card's **UID** in hex. The status LED turns green on a successful read. The UID is also printed over Serial. Press **Back** to exit.
  - If the reader isn't detected you'll see a "PN532 not found" screen — check that it's wired to I2C (SDA=GPIO8, SCL=GPIO9) and set to I2C mode.
- **Write Data / Saved Tags / Emulate Tag / P2P** — placeholders in this build; they show a "Not implemented yet" notice.

## 2.5 Wi-Fi

Scans for nearby Wi-Fi networks and lists them. Select a network to see details: SSID, RSSI, MAC (BSSID), and encryption type (Open/WEP/WPA/WPA2/WPA3). Uses the ESP32-S3's built-in radio, so no extra module is required.

## 2.6 Bluetooth LE

BLE tools using the ESP32-S3's built-in radio — including a BLE scanner that lists nearby devices, and a BLE mouse mode.

## 2.7 GPIO utilities

Hardware helper tools:

- **i2c scanner** — scans the I2C bus and reports the addresses it finds (useful for confirming the OLED and PN532 are detected, and identifying other I2C chips).
- **check sdcard** — reports whether an SD card is inserted (requires the optional SD module).
- Additional entries (gps, signal generator, button tester) are present as hooks for expansion.

## 2.8 Bad USB (HID)

Uses the ESP32-S3's native USB to act as a keyboard. Plug the device into a computer over USB and run scripted keystroke sequences. Requires **USB CDC On Boot: Enabled** in the board settings when building.

## 2.9 Apps

A set of standalone mini-apps and games:

- **calculator**
- **pomodoro timer**
- **wave creator**
- **snake game**
- **spacecraft game**
- **pass generator**

## 2.10 Settings

- **show usage** — data usage view
- **restart** — reboot the device (with confirmation)
- **batt info** — battery information
- **about** — firmware / project info
- **check sys devices** — re-run the boot self-test (OLED / CC1101 / NFC)
- *format sd* and *sd info* require the optional SD module.

---

## Project layout

```text
NoiseOps/                       (repository root)
├── HIZMOS_OLED_U8G2lib/        Arduino firmware (open the .ino of the same name)
├── SH1106_HIZMOS_BIN_FILES/    Prebuilt binaries for 1.3" OLED
├── SSD1306_HIZMOS_BIN_FILES/   Prebuilt binaries for 0.96"/1.54" OLED
├── HIZMOS-PCB-C/               PCB Gerber + drill files
├── InteractiveBOM_HIZMOS.html  Interactive bill of materials
├── README.md
└── LICENSE
```

The legacy file and folder names above are kept as-is so the build and flashing steps work without changes.

## Contributing

Contributions are welcome. To propose a change:

1. Fork the repository and create a feature branch.
2. Make your changes. If you touch the firmware, confirm it still compiles for the **ESP32-S3 Dev Module** and keep the existing pin assignments unchanged.
3. Open a pull request describing what you changed and how you tested it.

For bugs and feature ideas, please open an issue.

## Credits

This project is based on an existing open-source multi-tool firmware. The original code was created by a prior author; **NoiseOps** adapted and built upon that codebase — reworking the branding, boot and idle animations, menu, and hardware configuration for the ESP32-S3 (N16R8) build.

Credit for the underlying firmware goes to its original open-source author(s). NoiseOps is the adapted and maintained version.

See `LICENSE` for license terms.
