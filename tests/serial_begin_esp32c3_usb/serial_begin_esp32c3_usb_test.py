import re

def test_serial_begin_esp32c3_usb(wokwi_output_dir):
    expected_regexs = [
        r"[\d.]+ \[serial_begin\] Note",
        r"END-TEST",
    ]

    log_lines = (wokwi_output_dir / "serial_log.txt").read_text().splitlines()
    test_log_lines = log_lines[log_lines.index("BEGIN-TEST") + 1:]
    for expect_rx, line in zip(expected_regexs, test_log_lines):
        assert re.fullmatch(expect_rx, line), f'"{line}" !~ /{expect_rx}/'
