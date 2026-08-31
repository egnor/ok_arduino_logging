def test_basic_logging(run_wokwi):
    run_wokwi().assert_lines_match(
        r"BEGIN-TEST",
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
    )
