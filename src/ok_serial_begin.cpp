#include "ok_logging.h"

#if defined(ARDUINO_ARCH_ESP32) && CONFIG_TINYUSB_ENABLED
#include "esp32-hal-tinyusb.h"
#endif

#if defined(ARDUINO_ARCH_RP2040) && !defined(NO_USB)
#include <tusb.h>
#endif

#include <Arduino.h>

static OkLoggingContext OK_CONTEXT("ok_logging");

void ok_serial_begin(OkLoggingSerialOptions const& opts) {
  bool rx_buffer_unsupported = false;
  bool tx_buffer_unsupported = false;
  bool non_blocking_unsupported = false;
  bool connect_wait_unsupported = false;

  #if ARDUINO_ARCH_ESP32
    //
    // All flavors of ESP32 - buffer-size calls must come before .begin()
    //

    #if ARDUINO_USB_CDC_ON_BOOT  // Serial is some type of USB
      #if ARDUINO_USB_MODE  // CDC/JTAG-only hardware (Serial == HWCDC)
        if (opts.rx_buffer_size > 0)
          Serial.setRxBufferSize(opts.rx_buffer_size);
        if (opts.tx_buffer_size > 0)
          Serial.setTxBufferSize(opts.tx_buffer_size);
        Serial.begin(opts.baud);
        Serial.setDebugOutput(true);
        if (opts.tx_non_blocking)
          Serial.setTxTimeoutMs(0);

        // HWCDC has .isPlugged() and isConnected() from 2.0.15+
        if (opts.connect_wait_millis > 0) {
          #if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(2, 0, 15)
            auto const start = millis();
            bool plug_seen = false;
            while (!Serial.isConnected()) {
              if (millis() - start > opts.connect_wait_millis) break;
              plug_seen = plug_seen || Serial.isPlugged();
              if (!plug_seen && millis() - start > 100) break;
              delay(10);
            }
          #else
            connect_wait_unsupported = true;
          #endif
        }
      #else  // USB OTG hardware (Serial == USBCDC)
        if (opts.rx_buffer_size > 0)
          Serial.setRxBufferSize(opts.rx_buffer_size);
        if (opts.tx_buffer_size > 0)
          tx_buffer_unsupported = true;  // USBCDC has no setTxBufferSize
        Serial.begin(opts.baud);
        Serial.setDebugOutput(true);
        if (opts.tx_non_blocking)
          Serial.setTxTimeoutMs(0);

        if (opts.connect_wait_millis > 0) {
          auto const start = millis();
          bool plug_seen = false;
          while (!Serial) {
            if (millis() - start > opts.connect_wait_millis) break;
            plug_seen = plug_seen || tud_connected();
            if (!plug_seen && millis() - start > 100) break;
            delay(10);
          }
        }
      #endif
    #else  // Serial is UART
      if (opts.rx_buffer_size > 0)
        Serial.setRxBufferSize(opts.rx_buffer_size);
      if (opts.tx_buffer_size > 0)
        Serial.setTxBufferSize(opts.tx_buffer_size);
      if (opts.tx_non_blocking)
        non_blocking_unsupported = true;
      if (opts.connect_wait_millis > 0)
        connect_wait_unsupported = true;
      Serial.begin(opts.baud);
      Serial.setDebugOutput(true);
    #endif

  #elif ARDUINO_ARCH_RP2040
    //
    // Earle Philhower's RP2040 core. Limited buffer options.
    //

    if (opts.tx_buffer_size > 0)
      tx_buffer_unsupported = true;
    if (opts.tx_non_blocking)
      non_blocking_unsupported = true;

    #if defined(NO_USB) || defined(DISABLE_USB_SERIAL)  // Serial is UART
      if (opts.rx_buffer_size > 0)
        Serial.setFIFOSize(opts.rx_buffer_size);
      if (opts.connect_wait_millis > 0)
        connect_wait_unsupported = true;
      Serial.begin(opts.baud);
    #else  // Serial is USB CDC
      if (opts.rx_buffer_size > 0)
        rx_buffer_unsupported = true;
      Serial.begin(opts.baud);

      if (opts.connect_wait_millis > 0) {
        auto const start = millis();
        bool plug_seen = false;
        while (!Serial) {
          if (millis() - start > opts.connect_wait_millis) break;
          plug_seen = plug_seen || tud_connected();
          if (!plug_seen && millis() - start > 100) break;
          delay(10);
        }
      }
    #endif
  #endif

  if (rx_buffer_unsupported)
    OK_ERROR("Serial RX buffer request (%d) unsupported", opts.rx_buffer_size);
  if (tx_buffer_unsupported)
    OK_ERROR("Serial TX buffer request (%d) unsupported", opts.tx_buffer_size);
  if (non_blocking_unsupported)
    OK_ERROR("Serial non-blocking TX unsupported");
  if (connect_wait_unsupported)
    OK_ERROR("Serial connect-wait unsupported");
}
