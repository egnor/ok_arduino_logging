#include "ok_logging.h"

#if defined(ARDUINO_ARCH_ESP32) && CONFIG_TINYUSB_ENABLED
#include "esp32-hal-tinyusb.h"
#endif

#if defined(ARDUINO_ARCH_RP2040) && !defined(NO_USB)
#include <tusb.h>
#endif

#include <Arduino.h>

void ok_serial_begin(OkLoggingSerialOptions const& options) {
  Serial.begin(options.baud);

  #if ARDUINO_ARCH_ESP32
    //
    // All flavors of ESP32
    //

    Serial.setDebugOutput(true);  // Generally useful

    #if ARDUINO_USB_CDC_ON_BOOT  // Serial is some type of USB
      #if ARDUINO_USB_MODE  // CDC/JTAG-only hardware (Serial == HWCDC)
        if (options.rx_buffer_size > 0)
          Serial.setRxBufferSize(options.rx_buffer_size);
        if (options.tx_buffer_size > 0)
          Serial.setTxBufferSize(options.tx_buffer_size);
        if (options.tx_non_blocking)
          Serial.setTxTimeoutMs(0);

        // HWCDC has .isPlugged() and isConnected() from 2.0.15+
        #if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(2, 0, 15)
          if (options.connect_wait_millis > 0) {
            auto const start = millis();
            bool plug_seen = false;
            while (!Serial.isConnected()) {
              if (millis() - start > options.connect_wait_millis) break;
              plug_seen = plug_seen || Serial.isPlugged();
              if (!plug_seen && millis() - start > 100) break;
              delay(10);
            }
          }
        #endif
      #else  // USB OTG hardware (Serial == USBCDC)
        if (options.rx_buffer_size > 0)
          Serial.setRxBufferSize(options.rx_buffer_size);
        if (options.tx_non_blocking)
          Serial.setTxTimeoutMs(0);

        if (options.connect_wait_millis > 0) {
          auto const start = millis();
          bool plug_seen = false;
          while (!Serial) {
            if (millis() - start > options.connect_wait_millis) break;
            plug_seen = plug_seen || tud_connected();
            if (!plug_seen && millis() - start > 100) break;
            delay(10);
          }
        }
      #endif
    #else  // Serial is UART
      // No non-blocking support, and of course no connectivity detection.
      if (options.rx_buffer_size > 0)
        Serial.setRxBufferSize(options.rx_buffer_size);
      if (options.tx_buffer_size > 0)
        Serial.setTxBufferSize(options.tx_buffer_size);
    #endif

  #elif ARDUINO_ARCH_RP2040
    //
    // Earle Philhower's RP2040 core
    //

    // Sadly, the RP2040 core buffer size is hardcoded, and it has no true
    // nonblocking mode, but we can ignore flow control, so that's something...
    if (options.tx_non_blocking)
      Serial.ignoreFlowControl(true);

    #if !defined(NO_USB) && !defined(DISABLE_USB_SERIAL)
      if (options.connect_wait_millis > 0) {
        auto const start = millis();
        bool plug_seen = false;
        while (!Serial) {
          if (millis() - start > options.connect_wait_millis) break;
          plug_seen = plug_seen || tud_connected();
          if (!plug_seen && millis() - start > 100) break;
          delay(10);
        }
      }
    #endif
  #endif
}
