def test_serial_begin_rp2040_nonusb(run_wokwi):
    run_wokwi().assert_lines_match(
        r"BEGIN-TEST",
        r"[\d.]+ \[serial_begin\] Note",
        r"END-TEST",
    )
