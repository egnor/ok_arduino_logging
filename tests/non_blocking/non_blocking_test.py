import re


def test_non_blocking(wokwi_output_dir):
    log_lines = (wokwi_output_dir / "serial_log.txt").read_text().splitlines()
    assert "BEGIN-TEST" in log_lines
    start = log_lines.index("BEGIN-TEST") + 1
    end = log_lines.index("---END-2---")
    body = log_lines[start : end + 1]

    # First captured block: msgs A..E went through, F triggered BUF FULL,
    # G was dropped silently (no second marker).
    cap1_start = body.index("---CAPTURED---") + 1
    cap1_end = body.index("---END---")
    cap1 = body[cap1_start:cap1_end]

    expected_cap1 = [
        r"[\d.]+ \[nb\] padded msg number 1",
        r"[\d.]+ \[nb\] padded msg number 2",
        r"[\d.]+ \[nb\] padded msg number 3",
        r"[\d.]+ \[nb\] padded msg number 4",
        r"[\d.]+ \[nb\] padded msg number 5",
        r"[\d.]+ ⏸️ BUF FULL",
    ]
    assert len(cap1) == len(expected_cap1), (
        f"first capture lines={cap1!r}, expected count={len(expected_cap1)}"
    )
    for expect_rx, line in zip(expected_cap1, cap1):
        assert re.fullmatch(expect_rx, line), f'"{line}" !~ /{expect_rx}/'

    # No bytes should have been dropped from the ring itself: when
    # backpressure was real the logger dropped whole messages instead of
    # writing partial bytes that overflow.
    dropped_line = next(l for l in body if l.startswith("dropped_bytes="))
    assert dropped_line == "dropped_bytes=0"

    # Second block: after a full drain, normal logging resumes.
    cap2_start = body.index("---CAPTURED-2---") + 1
    cap2_end = body.index("---END-2---")
    cap2 = body[cap2_start:cap2_end]
    assert len(cap2) == 1
    assert re.fullmatch(r"[\d.]+ \[nb\] after drain", cap2[0]), cap2
