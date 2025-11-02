import ok_logging_setup
import ok_subprocess_defaults
import pytest
import shutil
import subprocess
from pathlib import Path

ok_logging_setup.install({"OK_LOGGING_OUTPUT": "stdout"})
ok_logging_setup.skip_traceback_for(subprocess.CalledProcessError)

@pytest.fixture(scope="module")
def sim_output_dir(request):
    sketch_dir = Path(request.path).parent
    output_dir = sketch_dir / "output.tmp"
    if output_dir.is_dir(): shutil.rmtree(output_dir)

    sub = ok_subprocess_defaults.SubprocessDefaults(cwd=sketch_dir)
    sub.run("arduino-cli", "compile", "--output-dir=output.tmp")
    sub.run(
        "wokwi-cli",
        "--scenario=scenario.yaml",
        "--serial-log-file=output.tmp/serial_log.txt"
    )
    return output_dir
