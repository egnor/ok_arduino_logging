import re

def test_basic_logging(sim_output_dir):
    expected_line_regexs = [
        r"[\d.]+ \[log_tag\] Note message",
        r"",
        r"[\d.]+ \[log_tag\] Note with newlines before and after",
        r"[\d.]+ \[log_tag\] Note with",
        r"newlines in the",
        r"middle",
        r"",
        r"[\d.]+ ⚠️ \[log_tag\] Error message",
        r"[\d.]+ ⚠️ \[log_tag\] ERROR",
        r"",
        r"[\d.]+ ⚠️ \[log_tag\] Error with newlines before and after",
        r"[\d.]+ 💥 \[log_tag\] FATAL Fatal message",
        r"  at: .*/basic_logging.ino:\d+",
        r"  in: void setup\(\)",
        r"  🚨 REBOOT IN 1 SEC 🚨",
    ]

    log_lines = (sim_output_dir / "serial_log.txt").read_text().splitlines()
    start_index = log_lines.index("BEGIN-TEST")
    for i, expected_regex in enumerate(expected_line_regexs):
        assert re.fullmatch(expected_regex, log_lines[start_index + 1 + i])
