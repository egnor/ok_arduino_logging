import re


def test_basic_logging(wokwi_output_dir):
    expected_regexs = [
        r"[\d.]+ 🕸️ \[aaa~~~bbb~~~ccc\] shown",
        r"[\d.]+ \[aaa~~~bbb~~~ccc~~~\] shown",
        r"[\d.]+ ⚠️ \[~~~aaa~~~bbb~~~ccc\] shown",
        r"[\d.]+ 💥 \[~~~aaa~~~bbb~~~ccc~~~\] FATAL shown",
        r"  at: .*/logging_config_symbol.ino:\d+",
        r"  in: void setup\(\)",
        r"  🚨 REBOOT IN 1 SEC 🚨",
    ]

    log_lines = (wokwi_output_dir / "serial_log.txt").read_text().splitlines()
    test_log_lines = log_lines[log_lines.index("BEGIN-TEST") + 1 :]
    for expect_rx, line in zip(expected_regexs, test_log_lines):
        assert re.fullmatch(expect_rx, line), f'"{line}" !~ /{expect_rx}/'
