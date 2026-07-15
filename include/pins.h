#pragma once

// KinCony B32M pin map (ESP32-S3-WROOM-1U N16R8)
// Source: kincony.com forum I/O pin definition thread (tid=9089) and
// the official ESPHome device page (devices.esphome.io/devices/kincony-b32m).
// Verify against your board revision before relying on this in production.

// ---- I2C bus (shared by all onboard peripherals below) ----
constexpr int PIN_I2C_SDA = 8;
constexpr int PIN_I2C_SCL = 18;
constexpr uint32_t I2C_FREQ_HZ = 400000;

// I2C peripheral addresses
constexpr uint8_t ADDR_PCF8575_IN_1_16      = 0x22; // inputs 1-16
constexpr uint8_t ADDR_PCF8575_IN17_24_OUT9_16 = 0x25; // inputs 17-24, outputs 9-16
constexpr uint8_t ADDR_PCF8575_OUT17_32      = 0x24; // outputs 17-32
constexpr uint8_t ADDR_PCF8575_IN25_32_OUT1_8 = 0x26; // inputs 25-32, outputs 1-8
constexpr uint8_t ADDR_EEPROM_24C02   = 0x50;
constexpr uint8_t ADDR_RTC_DS3231     = 0x68;
constexpr uint8_t ADDR_OLED_SSD1306   = 0x3C;
constexpr uint8_t ADDR_ADC_ADS1115    = 0x48;

// ---- Ethernet (W5500), custom SPI bus ----
constexpr int PIN_ETH_SCK   = 1;
constexpr int PIN_ETH_MOSI  = 2;
constexpr int PIN_ETH_MISO  = 41;
constexpr int PIN_ETH_CS    = 42;
constexpr int PIN_ETH_INT   = 43;
constexpr int PIN_ETH_RST   = 44;

// ---- SD card, separate SPI bus ----
constexpr int PIN_SD_MOSI = 10;
constexpr int PIN_SD_SCK  = 11;
constexpr int PIN_SD_MISO = 12;
constexpr int PIN_SD_CS   = 9;

// ---- RS485 (Modbus) ----
constexpr int PIN_RS485_RX = 38;
constexpr int PIN_RS485_TX = 39;

// ---- Tuya module UART ----
constexpr int PIN_TUYA_RX = 17;
constexpr int PIN_TUYA_TX = 16;

// ---- Free GPIOs broken out on headers ----
constexpr int PIN_FREE_NO_PULLUP[] = {13, 14, 21};
constexpr int PIN_FREE_PULLUP[]    = {40, 48, 47, 7};

// ---- Analog inputs via ADS1115 channels ----
// A1, A2: 0-5V DC on ADS1115 channels 0,1
// A3, A4: 4-20mA DC on ADS1115 channels 2,3
constexpr uint8_t ADS_CH_A1_0_5V  = 0;
constexpr uint8_t ADS_CH_A2_0_5V  = 1;
constexpr uint8_t ADS_CH_A3_4_20MA = 2;
constexpr uint8_t ADS_CH_A4_4_20MA = 3;
