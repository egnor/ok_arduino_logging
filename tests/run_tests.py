#!/usr/bin/env python3

import argparse
import json
import logging
import ok_logging_setup
import ok_subprocess_defaults
import toml
from pathlib import Path

ok_logging_setup.install()
sub = ok_subprocess_defaults.SubprocessDefaults()

parser = argparse.ArgumentParser()
parser.add_argument("test_dir", nargs="*", type=Path)
args = parser.parse_args()

tests_dir = Path(__file__).parent
for test_dir in args.test_dir or tests_dir.glob("*_test"):
    logging.info(f"\n🧪 TEST: {test_dir}/")
    output_dir = tests_dir / "build.tmp" / test_dir.name
    sub.run("arduino-cli", "compile", test_dir, f"--output-dir={output_dir}")

    wokwi_config = {
      "version": 1,
      "firmware": test_dir.name + ".ino.bin",
      "elf": test_dir.name + ".ino.elf",
    }
    (output_dir / "wokwi.toml").write_text(toml.dumps({"wokwi": wokwi_config}))

    wokwi_diagram = {
      "parts": [{"type": "board-esp32-devkit-c-v4", "id": "esp", "attrs": {}}],
      "connections": [
        ["esp:TX", "$serialMonitor:RX", ""],
        ["esp:RX", "$serialMonitor:TX", ""],
      ],
    }
    (output_dir / "diagram.json").write_text(json.dumps(wokwi_diagram))

    serial_log_path = output_dir / "serial_log.txt"
    sub.run(
        "wokwi-cli", output_dir,
        "--expect-text=+++SIMDONE",
        "--fail-text=+++SIMFAIL",
        f"--serial-log-file={serial_log_path}",
    )
