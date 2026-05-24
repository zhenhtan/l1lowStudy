"""一键运行全部示例"""

from pathlib import Path
import subprocess
import sys


def run_file(file_path: Path) -> None:
    print(f"\n===== Running {file_path.name} =====")
    subprocess.run([sys.executable, str(file_path)], check=True)


def main() -> None:
    root = Path(__file__).parent
    files = [
        root / "01_basics.py",
        root / "02_oop.py",
        root / "03_functional.py",
        root / "04_iter_gen.py",
        root / "05_asyncio_demo.py",
        root / "06_algo.py",
        root / "07_threading_vs_multiprocessing.py",
        root / "08_copy_memory.py",
        root / "09_common_interview_scenarios.py",
        root / "10_class_members.py",
        root / "11_typing_examples.py",
    ]

    for f in files:
        run_file(f)


if __name__ == "__main__":
    main()
