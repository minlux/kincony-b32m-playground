# Tuya module

The B32M has a header for a plug-in Tuya WiFi module, wired via UART
(`PIN_TUYA_RX` = GPIO17, `PIN_TUYA_TX` = GPIO16, see `include/pins.h`).

It's a separate, parallel control path to the ESP32-S3 itself: the main chip runs
its own firmware (ESPHome/Arduino/KCS v3) and talks to the Tuya module over that
UART so the device can also be controlled through Tuya's cloud, with no extra
backend work required. This buys:

- **Tuya Smart / Smart Life app** control from a phone
- **Alexa / Google Home** voice control (handled by Tuya's cloud integration)
- A pairing/provisioning flow via the board's "Tuya config" button

It's optional — `src/main.cpp` doesn't touch it. This doc is a reference for if/when
that's added.

## Official protocol docs (Tuya Developer Platform)

- [Serial Communication Protocol / "Tuya Cloud Universal Serial Port Access Protocol"](https://developer.tuya.com/en/docs/iot/tuya-cloud-universal-serial-port-access-protocol?id=K9hhi0xxtn9cb) — the frame spec
- [UART Communication (hardware design)](https://developer.tuya.com/en/docs/iot/mcu-access-hardware-design?id=Kaiuyozkmbgv0) — wiring/electrical side
- [MCU SDK Development guide](https://developer.tuya.com/en/docs/iot/mcu-access-guide?id=K9hrbv1ub5owo)

## Frame format

| Field | Size | Notes |
|---|---|---|
| Header | 2 bytes | fixed `0x55 0xAA` |
| Version | 1 byte | `0x03` typical |
| Command | 1 byte | see below |
| Data length | 2 bytes | big-endian |
| Data | N bytes | payload |
| Checksum | 1 byte | sum of all preceding bytes mod 256 |

## Key commands for a basic handshake

| ID | Purpose |
|---|---|
| `0x00` | Heartbeat (module→MCU every 1-15s, MCU echoes) |
| `0x01` | Module asks for Product ID + MCU firmware version |
| `0x02` | Working mode negotiation (who drives the pairing LED/reset) |
| `0x03` | WiFi/cloud network status |
| `0x04` | MCU→module: trigger WiFi reset/pairing |
| `0x07` | MCU→module: async data-point (DP) status report |
| `0x08` | Module→MCU: query all current DP values |

Handshake sequence: module sends heartbeat → MCU responds with version → module
queries product info (MCU replies with PID + version) → module queries working
mode → module reports network status → module queries DP status (MCU replies
with current values). After that it's event-driven DP read/write traffic.

Data points (DPs) are defined per-product on the Tuya IoT platform (an on/off
switch, a numeric value, etc.) when you register the device; the MCU
reports/receives changes keyed by DP ID.

## Ready-made library

[tuya/tuya-wifi-mcu-sdk-arduino-library](https://github.com/tuya/tuya-wifi-mcu-sdk-arduino-library)
implements this protocol already — give it a `Serial` port, your Product ID + MCU
version, define your DP table, and register callbacks for incoming DP writes.
Standard Arduino-style library, so it should work as a `lib_deps` git URL entry in
`platformio.ini` without hand-rolling the framing/checksum logic.

## Caveats

- Needs a Tuya IoT Platform developer account and a registered "Product" (with a
  matching DP schema) — the UART protocol alone only gets you talking to the WiFi
  module, not to the Tuya app/cloud.
- No KinCony-specific B32M documentation was found for this raw MCU protocol link.
  Their forum thread [tid=9090](https://www.kincony.com/forum/showthread.php?tid=9090)
  covers ESPHome + Tuya, but via a different mechanism than this raw serial
  protocol. This doc covers the generic Tuya module protocol used across all of
  KinCony's Tuya-equipped boards.
