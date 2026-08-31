def test_etl_error_handler(run_wokwi):
    run_wokwi().assert_lines_match(
        r"BEGIN-TEST",
        r"[\d.]+ 💥 \[ETL\] FATAL vector:bounds",
        r"  at: .*vector\.h:\d+",
        r"  🚨 REBOOT IN 1 SEC 🚨",
    )
