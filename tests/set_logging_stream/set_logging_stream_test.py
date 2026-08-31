def test_set_logging_stream(run_wokwi):
    run_wokwi().assert_lines_match(
        "BEGIN-TEST",
        r"[\d.]+ \[set_logging_stream\] Alt serial log message",
        r"[\d.]+ 💥 \[set_logging_stream\] FATAL Alt serial fatal message",
        r"  at: .*/set_logging_stream.ino:\d+",
        r"  in: void setup\(\)",
        r"  🚨 REBOOT.*",
    )
