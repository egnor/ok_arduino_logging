#!/usr/bin/env python3

import argparse
import logging
import ok_logging_setup
import ok_subprocess_defaults
import subprocess
from pathlib import Path

ok_logging_setup.install()
ok_logging_setup.skip_traceback_for(subprocess.CalledProcessError)
sub = ok_subprocess_defaults.SubprocessDefaults()

parser = argparse.ArgumentParser()
parser.add_argument("test_dir", nargs="*", type=Path)
args = parser.parse_args()

tests_dir = Path(__file__).parent
test_dirs = list(args.test_dir or tests_dir.glob("*_test"))
for test_dir in test_dirs:
    logging.info(f"\n🧪 TEST: {test_dir}/")
    sub.run("arduino-cli", "compile", test_dir, f"--export-binaries")
    sub.run("wokwi-cli", test_dir, "--scenario=scenario.yaml")

logging.info(f"\n🎉 All {len(test_dirs)} tests passed")
