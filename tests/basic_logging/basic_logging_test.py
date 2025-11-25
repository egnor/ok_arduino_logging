import re

def test_basic_logging(wokwi_output_dir):
    expected_regexs = [
        r"[\d.]+ \[basic_logging\] Note message",
        r"",
        r"[\d.]+ \[basic_logging\] Note with newlines before and after",
        r"[\d.]+ \[basic_logging\] Note with",
        r"newlines in the",
        r"middle",
        r"",
        r"[\d.]+ ⚠️ \[basic_logging\] Error message",
        r"[\d.]+ ⚠️ \[basic_logging\] ERROR",
        r"",
        r"[\d.]+ ⚠️ \[basic_logging\] Error with newlines before and after",
        r"[\d.]+ 💥 \[basic_logging\] FATAL Fatal message",
        r"  at: .*/basic_logging.ino:\d+",
        r"  in: void setup\(\)",
        r"  🚨 REBOOT IN 1 SEC 🚨",
    ]

    log_lines = (wokwi_output_dir / "serial_log.txt").read_text().splitlines()
    test_log_lines = log_lines[log_lines.index("BEGIN-TEST") + 1:]
    for expect_rx, line in zip(expected_regexs, test_log_lines):
        assert re.fullmatch(expect_rx, line), f'"{line}" !~ /{expect_rx}/'
