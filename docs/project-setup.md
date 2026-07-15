# Project setup log

How this playground was bootstrapped, in order. Useful if you need to redo this on a
fresh machine or understand why the config looks the way it does.

## 1. Research the hardware

Looked up the KinCony B32M product page, forum I/O pin-definition thread, and the
community ESPHome device page to get the pinout right (see README.md References
section for the exact URLs). Key facts that drove the setup below:

- MCU is ESP32-S3-WROOM-1U (N16R8): 16MB flash, 8MB **octal** PSRAM.
- The 32 inputs / 32 outputs are not wired directly to ESP32 GPIOs — they sit behind
  four PCF8575 16-bit I2C I/O expanders (addresses 0x22, 0x24, 0x25, 0x26).
- Ethernet is a W5500 on a custom (non-default) SPI pin set, so it needs manual
  `SPI.begin()` rather than relying on board defaults.

## 2. Scaffold the PlatformIO project structure

```sh
mkdir -p /home/manuel/Projekte/kincony-b32m-playground/{src,include,lib,test}
```

Created:
- `platformio.ini` — board config + library dependencies
- `include/pins.h` — pin map transcribed from the forum docs
- `src/main.cpp` — smoke-test sketch (output chase, input read, ADC, RTC, OLED)
- `README.md` — hardware summary and reference links

## 3. Install PlatformIO CLI

Not installed via pip in the end — installed manually with the official installer
script:

```sh
curl -fsSL -o get-platformio.py https://raw.githubusercontent.com/platformio/platformio-core-installer/master/get-platformio.py
python3 get-platformio.py
```

This installs to `~/.platformio/penv/bin/pio` (not on `PATH` by default).

## 4. Build, and fix dependency/config issues found along the way

```sh
~/.platformio/penv/bin/pio run
```

Iterated on `platformio.ini` based on build errors:

1. `robtillaart/PCF8575@^0.4.5` didn't exist in the registry — the real latest was
   `0.3.0`. Fixed to `robtillaart/PCF8575@^0.3.0`.
2. `paulstoffregen/Ethernet@^2.0.2` — wrong package owner. The registry package is
   published as `arduino-libraries/Ethernet`, not `paulstoffregen/Ethernet`. Fixed to
   `arduino-libraries/Ethernet@^2.0.2`. (Checked first whether arduino-esp32's
   built-in `ETH.h` could handle the W5500 directly — it only supports RMII PHYs
   like LAN8720, not SPI-based W5500, so the third-party library is required.)
3. Board defaulted to a 4MB-flash partition table (`Flash: ... from 3342336 bytes`)
   even though `board_upload.flash_size = 16MB` was set — flash size and partition
   table are independent settings. Added
   `board_build.partitions = default_16MB.csv` (found under
   `~/.platformio/packages/framework-arduinoespressif32/tools/partitions/`) to
   actually use the available 16MB.

Useful commands used to diagnose the library issues:

```sh
~/.platformio/penv/bin/pio pkg search "PCF8575"
~/.platformio/penv/bin/pio pkg search "name:Ethernet"
find ~/.platformio/packages/framework-arduinoespressif32 -iname "ETH.h"
grep -rl "W5500" ~/.platformio/packages/framework-arduinoespressif32/libraries/
find ~/.platformio/packages/framework-arduinoespressif32/tools/partitions -iname "*16MB*"
```

Final successful build: 4.6% flash (300909 / 6553600 bytes), 5.9% RAM
(19492 / 327680 bytes).

## 5. Git

```sh
git init -q
git add -A
```

No commit was made — left staged for the user to review and commit.

## Open items / not yet verified on real hardware

- Pin map in `include/pins.h` is transcribed from forum docs, not verified against
  a physical board.
- `src/main.cpp` has not been flashed or run on real hardware yet.
