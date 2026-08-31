def test_serial_begin_esp32c3_usb(run_wokwi):
    run_wokwi().assert_lines_match(
        r"BEGIN-TEST",
        r"[\d.]+ \[serial_begin\] Note",
        r"TX-BUF=\d{4,}",  # confirms setTxBufferSize(4096) took effect
        r"END-TEST",
    )
