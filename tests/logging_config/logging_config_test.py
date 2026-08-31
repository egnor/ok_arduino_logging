import re
import shlex


def test_config_default(run_wokwi):
    run_wokwi().assert_lines_match(
        "BEGIN-TEST",
        "OK_LOGGING_CONFIG=-undef-",
        "ok_logging_config=-null-",
        "tag=aaa~~~bbb~~~ccc min=1",
        "tag=aaa~~~bbb~~~ccc~~~ min=1",
        "tag=~~~aaa~~~bbb~~~ccc min=1",
        "tag=~~~aaa~~~bbb~~~ccc~~~ min=1",
        r"[\d.]+ 💥 \[logging_config\] FATAL END-TEST",
    )


def test_config_error_bad_level(run_wokwi):
    conf = "aaa*ccc=XYZZY"
    cpp_flag = shlex.quote(f"-DTEST_LOGGING_CONFIG={conf}")
    cli_flag = f"--build-property=compiler.cpp.extra_flags={cpp_flag}"
    run_wokwi(arduino_cli_extra=[cli_flag]).assert_lines_match(
        "BEGIN-TEST",
        "OK_LOGGING_CONFIG=-undef-",
        f"ok_logging_config={re.escape(conf)}",
        "tag=aaa~~~bbb~~~ccc min=0",  # Bad level is treated as DETAIL
        "tag=aaa~~~bbb~~~ccc~~~ min=1",
        "tag=~~~aaa~~~bbb~~~ccc min=1",
        "tag=~~~aaa~~~bbb~~~ccc~~~ min=1",
        r'[\d.]+ ⚠️ \[ok_logging\] Bad directive: "XYZZY"',
        f'  Full config: "{re.escape(conf)}"',
        r"[\d.]+ 💥 \[logging_config\] FATAL END-TEST",
    )


def test_config_error_early_catchall(run_wokwi):
    conf = "aaa*=ERROR,DETAIL,*ccc=FATAL"
    cpp_flag = shlex.quote(f"-DTEST_LOGGING_CONFIG={conf}")
    cli_flag = f"--build-property=compiler.cpp.extra_flags={cpp_flag}"
    run_wokwi(arduino_cli_extra=[cli_flag]).assert_lines_match(
        "BEGIN-TEST",
        "OK_LOGGING_CONFIG=-undef-",
        f"ok_logging_config={re.escape(conf)}",
        "tag=aaa~~~bbb~~~ccc min=2",  # other rules are still applied
        "tag=aaa~~~bbb~~~ccc~~~ min=2",
        "tag=~~~aaa~~~bbb~~~ccc min=3",  # even rules after the bad catchall
        "tag=~~~aaa~~~bbb~~~ccc~~~ min=1",  # bad catchall is not applied
        r'[\d.]+ ⚠️ \[ok_logging\] Bad directive: "DETAIL"',
        f'  Full config: "{re.escape(conf)}"',
        r"[\d.]+ 💥 \[logging_config\] FATAL END-TEST",
    )


def test_config_error_no_tag(run_wokwi):
    conf = "=DETAIL"
    cpp_flag = shlex.quote(f"-DTEST_LOGGING_CONFIG={conf}")
    cli_flag = f"--build-property=compiler.cpp.extra_flags={cpp_flag}"
    run_wokwi(arduino_cli_extra=[cli_flag]).assert_lines_match(
        "BEGIN-TEST",
        "OK_LOGGING_CONFIG=-undef-",
        f"ok_logging_config={re.escape(conf)}",
        "tag=aaa~~~bbb~~~ccc min=1",
        "tag=aaa~~~bbb~~~ccc~~~ min=1",
        "tag=~~~aaa~~~bbb~~~ccc min=1",
        "tag=~~~aaa~~~bbb~~~ccc~~~ min=1",
        r'[\d.]+ ⚠️ \[ok_logging\] Bad config: "=DETAIL"',
        r"[\d.]+ 💥 \[logging_config\] FATAL END-TEST",
    )


def test_config_glob_with_catchall(run_wokwi):
    conf = "aaa*ccc=DETAIL,ERROR"
    cpp_flag = shlex.quote(f"-DTEST_LOGGING_CONFIG={conf}")
    cli_flag = f"--build-property=compiler.cpp.extra_flags={cpp_flag}"
    run_wokwi(arduino_cli_extra=[cli_flag]).assert_lines_match(
        "BEGIN-TEST",
        "OK_LOGGING_CONFIG=-undef-",
        f"ok_logging_config={re.escape(conf)}",
        "tag=aaa~~~bbb~~~ccc min=0",
        "tag=aaa~~~bbb~~~ccc~~~ min=2",
        "tag=~~~aaa~~~bbb~~~ccc min=2",
        "tag=~~~aaa~~~bbb~~~ccc~~~ min=2",
        r"[\d.]+ 💥 \[logging_config\] FATAL END-TEST",
    )


def test_config_globs(run_wokwi):
    conf = "aaa*bbb*ccc=DETAIL,aaa*bbb*=NOTE,*bbb*ccc=ERROR,*bbb*=FATAL"
    cpp_flag = shlex.quote(f"-DTEST_LOGGING_CONFIG={conf}")
    cli_flag = f"--build-property=compiler.cpp.extra_flags={cpp_flag}"
    run_wokwi(arduino_cli_extra=[cli_flag]).assert_lines_match(
        "BEGIN-TEST",
        "OK_LOGGING_CONFIG=-undef-",
        f"ok_logging_config={re.escape(conf)}",
        "tag=aaa~~~bbb~~~ccc min=0",
        "tag=aaa~~~bbb~~~ccc~~~ min=1",
        "tag=~~~aaa~~~bbb~~~ccc min=2",
        "tag=~~~aaa~~~bbb~~~ccc~~~ min=3",
        r"[\d.]+ 💥 \[logging_config\] FATAL END-TEST",
    )


def test_config_macro(run_wokwi):
    conf = "aaa*bbb*ccc=ERROR"
    cpp_flag = shlex.quote(f"-DOK_LOGGING_CONFIG={conf}")
    cli_flag = f"--build-property=compiler.cpp.extra_flags={cpp_flag}"
    run_wokwi(arduino_cli_extra=[cli_flag]).assert_lines_match(
        "BEGIN-TEST",
        f"OK_LOGGING_CONFIG={re.escape(conf)}",
        f"ok_logging_config={re.escape(conf)}",
        "tag=aaa~~~bbb~~~ccc min=2",
        "tag=aaa~~~bbb~~~ccc~~~ min=1",
        "tag=~~~aaa~~~bbb~~~ccc min=1",
        "tag=~~~aaa~~~bbb~~~ccc~~~ min=1",
        r"[\d.]+ 💥 \[logging_config\] FATAL END-TEST",
    )
