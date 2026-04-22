# RAK4631 + RAK13302 Variant Notes

This variant targets a RAK19007 base with a RAK4631 core module and an external
RAK13302 1W LoRa module. The hardware is close to the RAK3401 radio layout, but
the MCU remains the RAK4631. Because the RAK13302 uses the WisBlock IO slot for
radio power, reset, interrupt, busy, and SPI, several inherited RAK4631/RAK3401
features are intentionally disabled or left undefined.

THE INITIAL COMMIT IS INTENDED TO BE A STRIPPED DOWN VERSION TO TEST BASIC 
FUNCTIONALITY BEFORE RE-ENABLING MORE ADVANCED FEATURES.  USE AT YOUR OWN RISK.


## Disabled or Removed Features

- `RAK_BOARD` is not defined. The RAK WisBlock GPS auto-detection path can probe
  `WB_IO2`, `WB_IO4`, and `WB_IO5`, which this variant reserves for the
  RAK13302 radio.
- `ENV_INCLUDE_GPS` is undefined. Generic GPS support can use Serial1 and related
  GPS paths that have not been reviewed for this RAK13302 pin assignment.
- GPS constants in `variant.h` are commented out. They should only be restored
  with a deliberate GPS pin review.
- `ENV_INCLUDE_RAK12035` is explicitly undefined and set to `0`. The RAK12035
  soil moisture module uses WisBlock IO pins that this variant reserves for the
  RAK13302 radio.
- NFC pins are commented out. `PIN_NFC1` and `PIN_NFC2` overlap pins used by the
  external radio.
- QSPI flash definitions were removed from `variant.h`. The copied
  RAK4631/RAK3401 QSPI pins overlap the RAK13302 SPI bus, so QSPI flash must not
  be enabled for this variant.
- `PIN_USER_BTN` and `PIN_USER_BTN_ANA` are not assigned. The old RAK4631 button
  mapping used pin `9`, which this variant reserves as the RAK13302 radio busy
  signal.
- The built-in RAK4631 SX1262 power control is held low through
  `SX126X_INTERNAL_4631_POWER_EN`. This keeps the internal radio off in favor of
  the external RAK13302 transmitter.

## RAK13302 Pin Ownership

The RAK13302 owns the WisBlock SPI pins and several IO pins:

- `WB_SPI_CLK`, `WB_SPI_CS`, `WB_SPI_MISO`, and `WB_SPI_MOSI` for the radio SPI
  bus.
- `WB_IO2` for the RAK13302 5V boost path on the RAK19007.
- `WB_IO3` for RAK13302 power/FEM control.
- `WB_IO4` for radio reset.
- `WB_IO5` for radio busy.
- `WB_IO6` for radio DIO1.

Any WisBlock module that needs those same pins should be considered incompatible
with this variant unless the hardware and firmware are reviewed together.

## Expansion Guidance

Treat the default SPI bus as reserved for the RAK13302 radio. This variant only
declares one SPI interface, and that interface is mapped to the radio bus. Do not
add SPI WisBlock modules unless their chip select, reset, interrupt, and MISO
behavior are reviewed against the RAK13302 radio usage.

Prefer I2C WisBlock modules for expansion. The primary I2C pins are separate
from the RAK13302 radio pins, and this variant also keeps the secondary I2C pins
defined. I2C modules should still be checked for any extra enable, interrupt,
reset, or address-select pins that may overlap the RAK13302-owned IO pins.

`WB_A0` / `PIN_A0` remains shared with battery voltage sensing, inherited from
the RAK4631 variant. The board class reads pin `5` as `PIN_VBAT_READ`, and the
variant also maps `WB_A0` / `PIN_A0` to pin `5`. Avoid external analog modules
on `A0` unless battery reporting and the external analog circuit are reviewed
together.

## Power Rail Behavior

This variant mirrors the RAK3401 power behavior for the external radio path.
`PIN_3V3_EN` is driven high during startup to enable the RAK19007 switched
`3V3_S` peripheral rail and the RAK13302 5V boost path. During system-off
shutdown, firmware drives both `SX126X_POWER_EN` and `PIN_3V3_EN` low.

That means shutdown disables more than the radio enable signal: anything powered
from the switched `3V3_S` rail may also be turned off. This is desirable for
low-power shutdown, but any additional module that expects to stay powered while
the MCU is in system-off should be reviewed against this behavior.

Firmware mirrors the RAK3401 behavior and does not detect the RAK13302 jumper
position. If the RAK13302 jumper is set to `EXT` and the module external 5V input
is used to power the board, validate shutdown current and possible power-backfeed
behavior on real hardware. In that configuration, driving `PIN_3V3_EN` low may
not fully remove power from the RAK13302/module power path.
