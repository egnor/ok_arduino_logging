import re
import re_assert
import textwrap

def test_basic_logging(sim_output_dir):
    matcher = re_assert.Matches(textwrap.dedent(r"""
        .*
        [\d.]+ \[log_tag\] Note message

        [\d.]+ \[log_tag\] Note with newlines before and after
        [\d.]+ \[log_tag\] Note with
        newlines in the
        middle

        [\d.]+ ⚠️ \[log_tag\] Error message
        [\d.]+ ⚠️ \[log_tag\] ERROR

        [\d.]+ ⚠️ \[log_tag\] Error with newlines before and after
        [\d.]+ 💥 \[log_tag\] FATAL Fatal message
          at: .*/basic_logging.ino:\d+
          in: void setup\(\)
          🚨 REBOOT IN 1 SEC 🚨
    """).lstrip(), re.DOTALL | re.MULTILINE)

    log = (sim_output_dir / "serial_log.txt").read_text().replace("\r", "")
    matcher.assert_matches(log)
