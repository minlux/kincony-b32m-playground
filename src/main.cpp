#include <Arduino.h>
#include <Wire.h>
#include <PCF8575.h>
#include <Adafruit_ADS1X15.h>
#include <RTClib.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "pins.h"

// 32 dry-contact inputs and 32 MOSFET outputs, split across four PCF8575
// 16-bit I/O expanders on the shared I2C bus. See include/pins.h for the
// address-to-channel mapping.
PCF8575 pcfIn1_16(ADDR_PCF8575_IN_1_16, &Wire);
PCF8575 pcfIn17_24_Out9_16(ADDR_PCF8575_IN17_24_OUT9_16, &Wire);
PCF8575 pcfOut17_32(ADDR_PCF8575_OUT17_32, &Wire);
PCF8575 pcfIn25_32_Out1_8(ADDR_PCF8575_IN25_32_OUT1_8, &Wire);

Adafruit_ADS1115 ads;
RTC_DS3231 rtc;
Adafruit_SSD1306 display(128, 64, &Wire, -1);

static uint32_t lastChaseMs = 0;
static uint8_t chaseStep = 0;

void setAllOutputs(bool on) {
  uint16_t mask = on ? 0x0000 : 0xFFFF; // PCF8575 outputs sink current, LOW = on
  pcfIn17_24_Out9_16.write16(mask);
  pcfOut17_32.write16(mask);
  pcfIn25_32_Out1_8.write16(mask);
}

void setup() {
  Serial.begin(115200);

  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL, I2C_FREQ_HZ);

  pcfIn1_16.begin();
  pcfIn17_24_Out9_16.begin();
  pcfOut17_32.begin();
  pcfIn25_32_Out1_8.begin();
  setAllOutputs(false);

  if (!ads.begin(ADDR_ADC_ADS1115, &Wire)) {
    Serial.println("ADS1115 not found");
  }

  if (!rtc.begin(&Wire)) {
    Serial.println("DS3231 not found");
  }

  if (!display.begin(SSD1306_SWITCHCAPVCC, ADDR_OLED_SSD1306)) {
    Serial.println("SSD1306 not found");
  } else {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("KinCony B32M");
    display.println("playground booting...");
    display.display();
  }
}

void loop() {
  uint32_t now = millis();

  // Chase a single output across channels 1-32 every 200ms as a smoke test.
  if (now - lastChaseMs >= 200) {
    lastChaseMs = now;
    setAllOutputs(false);
    if (chaseStep < 8) {
      pcfIn25_32_Out1_8.write(chaseStep, LOW); // outputs 1-8, active low
    } else if (chaseStep < 16) {
      pcfIn17_24_Out9_16.write(chaseStep - 8, LOW); // outputs 9-16
    } else {
      pcfOut17_32.write(chaseStep - 16, LOW); // outputs 17-32
    }
    chaseStep = (chaseStep + 1) % 32;
  }

  uint16_t in1_16 = pcfIn1_16.read16();
  uint16_t in17_24 = pcfIn17_24_Out9_16.read16() & 0x00FF; // low byte only
  uint16_t in25_32 = pcfIn25_32_Out1_8.read16() & 0x00FF;  // low byte only

  float voltsA1 = ads.computeVolts(ads.readADC_SingleEnded(ADS_CH_A1_0_5V));
  DateTime nowRtc = rtc.now();

  static uint32_t lastPrintMs = 0;
  if (now - lastPrintMs >= 1000) {
    lastPrintMs = now;
    Serial.printf("in1-16=%04X in17-24=%02X in25-32=%02X A1=%.2fV time=%02d:%02d:%02d\n",
                  in1_16, in17_24, in25_32, voltsA1,
                  nowRtc.hour(), nowRtc.minute(), nowRtc.second());

    display.clearDisplay();
    display.setCursor(0, 0);
    display.printf("%02d:%02d:%02d\n", nowRtc.hour(), nowRtc.minute(), nowRtc.second());
    display.printf("IN1-16  %04X\n", in1_16);
    display.printf("IN17-32 %02X%02X\n", in25_32, in17_24);
    display.printf("A1 %.2fV\n", voltsA1);
    display.display();
  }
}
