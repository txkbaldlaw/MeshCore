#include <Arduino.h>
#include <Wire.h>

#include "RAK4631Board.h"

#ifdef NRF52_POWER_MANAGEMENT
// Static configuration for power management
// Values set in variant.h defines
const PowerMgtConfig power_config = {
  .lpcomp_ain_channel = PWRMGT_LPCOMP_AIN,
  .lpcomp_refsel = PWRMGT_LPCOMP_REFSEL,
  .voltage_bootlock = PWRMGT_VOLTAGE_BOOTLOCK
};

void RAK4631Board::initiateShutdown(uint8_t reason) {
  // Disable the external RAK13302 FEM and boost/peripheral rail before shutdown.
  digitalWrite(SX126X_POWER_EN, LOW);
  digitalWrite(PIN_3V3_EN, LOW);

#ifdef SX126X_INTERNAL_4631_POWER_EN
  digitalWrite(SX126X_INTERNAL_4631_POWER_EN, LOW);
#endif

  if (reason == SHUTDOWN_REASON_LOW_VOLTAGE ||
      reason == SHUTDOWN_REASON_BOOT_PROTECT) {
    configureVoltageWake(power_config.lpcomp_ain_channel, power_config.lpcomp_refsel);
  }

  enterSystemOff(reason);
}
#endif // NRF52_POWER_MANAGEMENT

void RAK4631Board::begin() {
  NRF52BoardDCDC::begin();
  pinMode(PIN_VBAT_READ, INPUT);
#ifdef PIN_USER_BTN
  pinMode(PIN_USER_BTN, INPUT_PULLUP);
#endif

#ifdef PIN_USER_BTN_ANA
  pinMode(PIN_USER_BTN_ANA, INPUT_PULLUP);
#endif

#if defined(PIN_BOARD_SDA) && defined(PIN_BOARD_SCL)
  Wire.setPins(PIN_BOARD_SDA, PIN_BOARD_SCL);
#endif

  Wire.begin();

#ifdef SX126X_INTERNAL_4631_POWER_EN
  pinMode(SX126X_INTERNAL_4631_POWER_EN, OUTPUT);
  digitalWrite(SX126X_INTERNAL_4631_POWER_EN, LOW);
#endif

  // PIN_3V3_EN (WB_IO2, P0.34) enables the RAK19007 switched peripheral rail
  // and the RAK13302 5V boost path. Keep it high while the external radio runs.
  pinMode(PIN_3V3_EN, OUTPUT);
  digitalWrite(PIN_3V3_EN, HIGH);

  pinMode(SX126X_POWER_EN, OUTPUT);
#ifdef NRF52_POWER_MANAGEMENT
  // Boot voltage protection check (may not return if voltage too low)
  // We need to call this after we configure SX126X_POWER_EN as output but before we pull high
  checkBootVoltage(&power_config);
#endif
  digitalWrite(SX126X_POWER_EN, HIGH);
  delay(10);   // give the RAK13302 SX1262/FEM power path some time to settle
}
