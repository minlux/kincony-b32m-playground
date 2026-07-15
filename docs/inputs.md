# Digital inputs (32x dry contact)

Also covers the 7 "free GPIO" pins for fast/interrupt-driven input use cases
(e.g. pulse/incremental counters), since those are direct ESP32 connections
rather than I2C-polled like the 32 dry-contact inputs above.

## Field wiring

Confirmed from KinCony's own wiring diagram
(https://www.kincony.com/images/B32M/B32M_wiring_pix1000.jpg):

- Grounding the input terminal (0V, e.g. via a switch to GND) = **True / active**
- Applying DC12/24V to the input terminal = **False / inactive**

The intended wiring is a plain dry contact (switch) between the input terminal and
GND — not a voltage source. The image shows exactly this: a switch from GND to an
input terminal, matching the "0V = True" case.

## Optocoupler part

Confirmed from a board close-up photo
(https://www.kincony.com/images/B32M/pcf8575.JPG), which shows parts marked
`C519 / EL3H7` next to the `PCF8575TS` I/O expander:

- EL3H7 — Everlight 4-pin SSOP phototransistor optocoupler
  ([datasheet](http://www.mouser.com/datasheet/2/143/EL3H7_G-26376.pdf)):
  CTR 80-160%, 3750 Vrms isolation, 50mA max collector current, NPN
  phototransistor output.

One EL3H7 per input channel provides the galvanic isolation between the field
wiring and the PCF8575/ESP32 side. Note: input and output circuitry are
physically interleaved on the PCB, so these optocouplers can appear right next
to the NCE60P10K output MOSFETs (see `docs/outputs.md`) in board photos —
they are unrelated to output-side flyback protection.

## Hardware level: active-low

Despite the logical meaning above being active-high (closed contact = True), the
signal is actually **active-low** at the chip level:

- Each input has an optocoupler (EL3H7, see above) with an internal pull-up to
  the field-side 12-24V rail.
- The PCF8575 pin the ESP32 reads sits **HIGH** normally (contact open, or
  12-24V applied to the terminal — no voltage differential across the opto LED,
  so it doesn't conduct).
- Grounding the terminal creates the voltage differential, the opto LED
  conducts, and the phototransistor pulls the PCF8575 input pin **LOW**.

So: raw silicon reads active-low (LOW = contact closed = active), but the
intended/logical meaning is active-high (closed = True) — this must be inverted
in software.

## Confirmation from KinCony's ESPHome reference config

Forum thread tid=9091 — every input `binary_sensor` uses `inverted: true`:

```yaml
- platform: gpio
  name: "b32-input01"
  id: "b32_input01"
  pin:
    pcf8574: pcf8574_hub_in_1
    number: 8
    mode: INPUT
    inverted: true
```

This flips the raw LOW (grounded/active) into the logical `True`/"on" state
exposed to Home Assistant.

## Implication for this repo

`src/main.cpp` currently prints the raw `pcfIn*.read16()` bits **uninverted** —
a closed switch shows as bit `0`, not `1`. Any code that treats "input active"
as logical/human-intuitive (closed contact = true) needs to invert the raw bits
read from the PCF8575 expanders, the same way the ESPHome config does.

## Free GPIOs: interrupts / pulse counting

For an incremental counter (pulse source, rotary encoder, flow meter, tachometer,
etc.), use one of the **free GPIOs** (see `include/pins.h`), not the 32
dry-contact inputs above:

- No pull-up: GPIO13, 14, 21
- With pull-up: GPIO40, 48, 47, 7

Reasons:

- The 32 dry-contact inputs sit behind PCF8575 I2C expanders, polled over I2C.
  There's no documented PCF8575 `/INT` line wired back to the ESP32, so there's
  no real interrupt path there for fast/precise counting.
- The free GPIOs are direct ESP32 connections, so any of them can be used as a
  genuine interrupt source. On ESP32-S3, **any** GPIO can trigger an interrupt —
  its GPIO matrix routes any pin to any peripheral/interrupt line, unlike MCUs
  where only certain pins share fixed IRQ lines.

Two implementation options:

1. **`attachInterrupt()` + volatile counter** — simplest, fine for a button or
   low-rate pulse source (a few hundred Hz).
2. **Hardware PCNT peripheral** (ESP-IDF's `driver/pulse_cnt.h`, available since
   this project targets core 3.x / IDF5) — counts pulses entirely in hardware
   with no per-edge CPU/ISR overhead, plus built-in glitch filtering. Worth it
   if the signal is fast (e.g. a flow meter or high-RPM tachometer) or
   electrically noisy. This is the recommended approach for a real incremental
   counter.

Note: GPIO13/14/21 have no pull-up, so if the counter source is open-drain/
open-collector (e.g. a reed switch or hall sensor with open-drain output), use
`INPUT_PULLUP` in software or an external pull-up. The 40/48/47/7 group already
has one on the PCB.

## Free GPIOs: quadrature (A-B) encoders

The PCNT peripheral also does quadrature decoding natively in hardware — counting
both forward and backward — which is its classic textbook use case. No missed
counts even at high RPM, and no CPU involvement per edge.

Each PCNT unit has multiple channels, and each channel has a **pulse input** and
a **control input**. Quadrature wiring is symmetric:

- Channel 0: pulse = A, control = B → counts on every edge of A, direction
  decided by the level of B
- Channel 1: pulse = B, control = A → counts on every edge of B, direction
  decided by the level of A

Combining both channels into the same counter gives x4 decoding (4 counts per
encoder detent) with correct forward/backward direction, fully resolved in
hardware. This is exactly what ESP-IDF's official `pcnt` rotary-encoder example
does, and what Arduino wrapper libraries like `ESP32Encoder` build on.

Needs 2 free GPIOs per encoder (no fixed A/B pin requirement — any pair works,
since it's all routed through the GPIO matrix). With 7 free GPIOs total, up to
3 quadrature encoders fit with one spare pin left over.

Encoders are usually open-collector or push-pull outputs needing a stable idle
level, so prefer the pull-up group (GPIO40/48/47/7) unless the encoder already
drives both states actively.
