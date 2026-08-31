import re


def test_non_blocking(run_wokwi):
    run_wokwi().assert_lines_match(
        "BEGIN-TEST",
        "---CAPTURED---",
        r"[\d.]+ \[nb\] padded msg number 1",
        r"[\d.]+ \[nb\] padded msg number 2",
        r"[\d.]+ \[nb\] padded msg number 3",
        r"[\d.]+ \[nb\] padded msg number 4",
        r"[\d.]+ \[nb\] padded msg number 5",
        r"[\d.]+ ⏸️ BUF FULL",
        "---END---",
        "dropped_bytes=0",
        "---CAPTURED-2---",
        r"[\d.]+ \[nb\] after drain",
        "---END-2---",
        r"[\d.]+ 💥 \[nb\] FATAL end of test",
    )
