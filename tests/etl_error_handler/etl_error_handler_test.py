import re

def test_etl_error_handler(wokwi_output_dir):
    expected_regexs = [
        r"[\d.]+ 💥 \[ETL\] FATAL vector:bounds",
        r"  at: .*vector\.h:\d+",
        r"  🚨 REBOOT IN 1 SEC 🚨",
    ]

    log_lines = (wokwi_output_dir / "serial_log.txt").read_text().splitlines()
    assert "BEGIN-TEST" in log_lines
    test_log_lines = log_lines[log_lines.index("BEGIN-TEST") + 1:]
    for expect_rx, line in zip(expected_regexs, test_log_lines):
        assert re.fullmatch(expect_rx, line), f'"{line}" !~ /{expect_rx}/'
