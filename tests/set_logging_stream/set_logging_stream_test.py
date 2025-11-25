import re

def test_set_logging_stream(wokwi_output_dir):
    expected_regexs = [
        r"[\d.]+ \[set_logging_stream\] Alt serial log message",
        r"[\d.]+ 💥 \[set_logging_stream\] FATAL Alt serial fatal message",
        r"  at: .*/set_logging_stream.ino:\d+",
        r"  in: void setup\(\)",
        r"  🚨 REBOOT.*",
    ]

    log_lines = (wokwi_output_dir / "serial_log.txt").read_text().splitlines()
    assert "BEGIN-TEST" in log_lines
    test_log_lines = log_lines[log_lines.index("BEGIN-TEST") + 1:]
    for expect_rx, line in zip(expected_regexs, test_log_lines):
        assert re.fullmatch(expect_rx, line), f'"{line}" !~ /{expect_rx}/'
