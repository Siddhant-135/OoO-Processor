from pathlib import Path
import argparse, subprocess, tempfile, sys, re

ROOT = Path(__file__).resolve().parent
PROGRAMS_DIR = ROOT 
COMPILER = ROOT / "../../compiler.py"
BINARY = ROOT / "../../main"
def run_program(code_path):
    with tempfile.TemporaryDirectory() as td:
        tmp = Path(td) / code_path.name
        tmp.write_bytes(code_path.read_bytes())

        res = subprocess.run(
            [sys.executable, str(COMPILER), str(tmp)],
            cwd=ROOT, capture_output=True, text=True
        )
        if res.returncode:
            raise RuntimeError(f"Preprocess failed for {code_path.name}\n{res.stderr.strip()}")

        res = subprocess.run(
            [str(BINARY), str(tmp)],
            cwd=ROOT, capture_output=True, text=True
        )
        if res.returncode:
            raise RuntimeError(f"Execution failed for {code_path.name}\n{res.stderr.strip()}")

        return res.stdout


def ans_path(code_path):
    m = re.fullmatch(r"code(\d+)\.txt", code_path.name)
    return code_path.with_name(f"ans{m.group(1)}.txt") if m \
        else code_path.with_name(f"{code_path.stem}_ans.txt")


def write_ans(path, content):
    path.write_bytes(content.encode("utf-16"))


def collect(code_file):
    return [code_file] if code_file else sorted(PROGRAMS_DIR.glob("code*.txt"))


def main():
    p = argparse.ArgumentParser(
        description="Generate ans files from code files"
    )
    p.add_argument("code_file", nargs="?", type=Path)
    args = p.parse_args()

    if not BINARY.exists():
        print("Missing binary")
        return 2

    files = collect(args.code_file)
    if not files:
        print("No code files found")
        return 2

    failures = 0
    for f in files:
        if not f.exists():
            print(f"Missing code file: {f}")
            failures += 1
            continue
        try:
            out = run_program(f)
            ap = ans_path(f)
            write_ans(ap, out)
            print(f"WROTE {ap.name} from {f.name}")
        except RuntimeError as e:
            failures += 1
            print(f"FAIL {f.name}\n{e}")

    print(f"\nFailures: {failures}")
    return 1 if failures else 0


if __name__ == "__main__":
    raise SystemExit(main())