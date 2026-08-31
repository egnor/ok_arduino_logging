def test_set_logging_function(run_wokwi):
    run_wokwi().assert_lines_match(
        r"BEGIN-TEST",
        r"[\d.]+ \[set_logging_function\] Default log message",
        r'ALT: tag="set_logging_function" lev=1 millis=\d+ text="Alt function log message"',
        r'ALT: tag="set_logging_function" lev=3 millis=\d+ text="Alt function fatal message',
        r"  at: .*/set_logging_function.ino:\d+",
        r'  in: void setup\(\)"',
    )
