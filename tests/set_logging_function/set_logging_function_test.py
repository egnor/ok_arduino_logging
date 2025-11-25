import re

def test_set_logging_function(wokwi_output_dir):
    expected_regexs = [
        r'[\d.]+ \[set_logging_function\] Default log message',
        r'ALT: tag="set_logging_function" lev=1 millis=\d+ text="Alt function log message"',
        r'ALT: tag="set_logging_function" lev=3 millis=\d+ text="Alt function fatal message',
        r'  at: .*/set_logging_function.ino:\d+',
        r'  in: void setup\(\)"',
    ]

    log_lines = (wokwi_output_dir / "serial_log.txt").read_text().splitlines()
    assert "BEGIN-TEST" in log_lines
    test_log_lines = log_lines[log_lines.index("BEGIN-TEST") + 1:]
    for expect_rx, line in zip(expected_regexs, test_log_lines):
        assert re.fullmatch(expect_rx, line), f'"{line}" !~ /{expect_rx}/'
