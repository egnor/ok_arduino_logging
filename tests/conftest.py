import ok_logging_setup
import ok_subprocess_runner
import pytest
import shutil
import subprocess
from pathlib import Path

ok_logging_setup.install({"OK_LOGGING_OUTPUT": "stdout"})
ok_logging_setup.skip_traceback_for(subprocess.CalledProcessError)


@pytest.fixture(scope="module")
def wokwi_output_dir(request):
    sketch_dir = Path(request.path).parent
    if (output_dir := sketch_dir / "output.tmp").is_dir():
        shutil.rmtree(output_dir)

    run = ok_subprocess_runner.SubprocessRunner(cwd=sketch_dir)
    run("arduino-cli", "compile", "--output-dir=output.tmp")
    run(
        "wokwi-cli",
        "--scenario=scenario.yaml",
        "--serial-log-file=output.tmp/serial_log.txt",
    )
    return output_dir
