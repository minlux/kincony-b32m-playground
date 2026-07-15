# kincony-b32m-playground

PlatformIO/Arduino playground for the [KinCony B32M](https://www.kincony.com/esp32-smart-controller-b32m-32ch-mosfet.html),
a 32-channel ESP32-S3 smart MOSFET controller (DIN-rail industrial/home-automation controller,
not an automotive ECU).

## Hardware summary

- **MCU:** ESP32-S3-WROOM-1U (N16R8) — 16MB flash, 8MB octal PSRAM
- **Power:** 12-24V DC in
- **Outputs:** 32× MOSFET, 10A per channel
- **Inputs:** 32× dry-contact (optocoupler isolated, up to 500m cable run)
- **Analog:** ADS1115 16-bit ADC — 2ch 0-5V, 2ch 4-20mA
- **Ethernet:** W5500, 100Mbps, custom SPI pins
- **Other onboard:** DS3231 RTC, SSD1306 OLED, 24C02 EEPROM, SD card slot, RS485,
  WiFi, Bluetooth, USB-C, Tuya module header
- **Size:** 264 x 83 x 56mm, DIN-rail mount

Full pin map is in [`include/pins.h`](include/pins.h). The 32 inputs and 32 outputs are
multiplexed across four PCF8575 16-bit I2C I/O expanders (0x22, 0x24, 0x25, 0x26) —
they are not directly wired to ESP32 GPIOs.

## Firmware options

This project targets **Arduino/PlatformIO** for custom firmware. KinCony also offers:

- **ESPHome** — official device page: https://devices.esphome.io/devices/kincony-b32m/
- **KCS v3** — KinCony's proprietary app/cloud firmware
- **Tasmota** — community alternative

## Getting started

```sh
pio run                # build
pio run -t upload      # flash over USB-C
pio device monitor     # serial monitor (115200 baud)
```

`src/main.cpp` is a smoke-test sketch: it walks a single output across all 32 channels,
reads back all 32 inputs, samples analog channel A1, reads the RTC, and prints/display
status once per second. Replace it with your own application logic.

## References

- Product page: https://www.kincony.com/esp32-smart-controller-b32m-32ch-mosfet.html
- I/O pin definitions (forum): https://www.kincony.com/forum/showthread.php?tid=9089
- ESPHome config example (forum): https://www.kincony.com/forum/showthread.php?tid=9091
- Wiring examples (forum): https://www.kincony.com/forum/printthread.php?tid=9261
- KCS v3 firmware guide: https://www.kincony.com/how-to-use-kcsv3-firmware-esp32-board.html

Pin numbers were transcribed from the forum documentation and community ESPHome config;
double-check against your specific board revision before wiring anything up.
