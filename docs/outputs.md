# Digital outputs (32x MOSFET)

## MOSFET part

Confirmed from a board close-up photo
(https://www.kincony.com/images/B32M/mosfet.JPG), which shows parts marked
`NCE60P10K`:

- NCE60P10K — plain discrete **P-channel power MOSFET**
  ([datasheet](https://www.lcsc.com/datasheet/lcsc_datasheet_2004201432_Wuxi-NCE-Power-Semiconductor-NCE60P10K_C502840.pdf)):
  -60V, -10A, R_DS(on) 0.12Ω, TO-252 package.

This is **not** a smart/protected driver IC — no integrated flyback clamping, no
charge pump, nothing beyond the bare transistor. KinCony's "10A driver IC"
wording on the product page just refers to this MOSFET.

## Topology: high-side switching

P-channel is the standard choice for a **high-side switch**: source tied to the
+12/24V input rail, drain feeding the output terminal, and the load's other leg
returned to common GND. This matches the B32M wiring diagram
(https://www.kincony.com/images/B32M/B32M_wiring_pix1000.jpg), where every load
example (relay coil, PLC, contactor, solenoid valve) has one leg on the switched
output pin and the other leg tied to a shared GND.

## Why the body diode does not protect against inductive loads

A MOSFET's body diode only clamps in one direction. For this P-channel FET:

- Body diode anode = drain (output pin)
- Body diode cathode = source (+V rail)

It only conducts when the output pin tries to swing **above** the +V rail.

When switching off an inductive load (relay coil, solenoid) wired to GND on the
far side, the collapsing magnetic field drives the output pin **below GND**, not
above +V. The body diode faces the wrong way to catch that — it provides no
protection against the actual flyback event in this topology.

**Conclusion: the board's own MOSFETs provide no protection against inductive
turn-off transients.** This isn't a guess — it follows directly from the
confirmed P-channel high-side topology and the body diode's fixed direction.

## Required mitigation

Add an external freewheeling diode across every inductive load (relay coil,
solenoid valve, motor, contactor), oriented:

- Cathode at the switched/output terminal
- Anode at GND

So it stays reverse-biased (non-conducting) during normal ON operation (output
pin near +V), and conducts to recirculate the coil current back to GND exactly
when the output node tries to swing negative during turn-off.

A 1N4001-1N4007 or a Schottky rated for the coil's current is sufficient for
typical relay/solenoid coils.
