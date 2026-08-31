import dataclasses
import hashlib
import ok_logging_setup
import ok_subprocess_runner
import pytest
import re
import shutil
import subprocess
from pathlib import Path

ok_logging_setup.install({"OK_LOGGING_OUTPUT": "stdout"})
ok_logging_setup.skip_traceback_for(subprocess.CalledProcessError)

names_used: set[str] = set()


@dataclasses.dataclass
class WokwiResult:
    output_path: Path

    def output_lines(self) -> list[str]:
        return (self.output_path).read_text().splitlines()

    def assert_lines_match(
        self, find: str | re.Pattern | None, *follow: list[str | re.Pattern]
    ) -> None:
        find_rx = re.compile(find) if isinstance(find, str) else find
        follow_rx = [re.compile(m) if isinstance(m, str) else m for m in follow]
        for line in self.output_lines():
            if not find_rx and not follow_rx:
                break  # successful match
            elif not find_rx:
                rx = follow_rx.pop(0)
                assert rx.search(line), f"{line!r} !~ /{rx.pattern}/"
            elif find_rx.search(line):
                find_rx = None

        assert not find_rx, f"No {find_rx!r}"
        assert not follow_rx, f"No {follow_rx[0]!r}"


@pytest.fixture
def run_wokwi(request):
    def run(
        arduino_cli_extra: list[str] = [],
        wokwi_cli_extra: list[str] = [],
    ) -> WokwiResult:
        sketch_dir = Path(request.path).parent
        build_name = f"{request.module.__name__}.{request.node.name}"
        build_dir = sketch_dir / f"{build_name}.tmp"
        ok_subprocess_runner.run(
            "arduino-cli",
            "compile",
            f"--build-path={build_dir}/work",
            f"--output-dir={build_dir}",
            *arduino_cli_extra,
            cwd=sketch_dir,
        )

        output_path = build_dir / "output.txt"
        ok_subprocess_runner.run(
            "wokwi-cli",
            f"--elf={build_dir}/{sketch_dir.name}.ino.elf",
            "--scenario=scenario.yaml",
            f"--serial-log-file={output_path}",
            *wokwi_cli_extra,
            cwd=sketch_dir,
        )
        return WokwiResult(output_path)

    return run
