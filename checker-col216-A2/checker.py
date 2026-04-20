from pathlib import Path
import subprocess
import tempfile
import difflib
import sys
import re

ROOT = Path(__file__).resolve().parent
COMPILER = ROOT / "../compiler.py"
BINARY = ROOT / "../main"

def read_expected(path):
    data = path.read_bytes()
    if data.startswith(b'\xff\xfe') or data.startswith(b'\xfe\xff'):
        return data.decode("utf-16")
    return data.decode("utf-8", errors="ignore")

def normalize(text):
    lines = text.splitlines()
    lines = [ln for ln in lines if not ln.startswith("=")]
    text = " ".join(lines)
    text = text.replace(".", " ").replace("|", " ").replace("+", " ").replace("[", " ").replace("]", " ")
    text = re.sub(r"\s+", " ", text).strip()
    text = text.lower()
    return text

def run_one(code_path, ans_path):
    with tempfile.TemporaryDirectory() as td:
        tmp_code = Path(td) / code_path.name
        tmp_code.write_bytes(code_path.read_bytes())
        p = subprocess.run(
            [sys.executable, str(COMPILER), str(tmp_code)],
            cwd=ROOT, capture_output=True, text=True
        )
        if p.returncode != 0:
            return False, f"Preprocess failed\n{p.stderr}"
        r = subprocess.run(
            [str(BINARY), str(tmp_code)],
            cwd=ROOT, capture_output=True, text=True
        )
        if r.returncode != 0:
            return False, f"Execution failed\n{r.stderr}"
        actual = normalize(r.stdout)
        expected = normalize(read_expected(ans_path))
        case_name = f"{code_path.parent.name}/{code_path.name}"
        if actual == expected:
            return True, f"PASS {case_name}"
        diff = "\n".join(difflib.unified_diff(
            expected.splitlines(),
            actual.splitlines(),
            n=3
        ))
        return False, f"FAIL {case_name}\n{diff[:2000]}"

def test_dirs():
    dirs = [p for p in ROOT.iterdir() if p.is_dir() and re.fullmatch(r"test\d+", p.name)]
    return sorted(dirs, key=lambda p: int(p.name[4:]))

def fetch_test_cases():
    pairs = []
    for program_dir in test_dirs():
        for code_path in sorted(program_dir.glob("code*.txt")):
            name = code_path.name
            if name.startswith("code") and name.endswith(".txt"):
                idx = name[4:-4]
                ans_path = program_dir / f"ans{idx}.txt"
                if ans_path.exists():
                    pairs.append((code_path, ans_path))
    return pairs

def main():
    if not BINARY.exists():
        print("Missing binary")
        return 2
    pairs = fetch_test_cases()
    if not pairs:
        print("No test cases")
        return 2
    fails = 0
    for c, a in pairs:
        ok, msg = run_one(c, a)
        print(msg)
        if not ok:
            fails += 1
    print(f"\nFailures: {fails}")
    return 1 if fails else 0

if __name__ == "__main__":
    raise SystemExit(main())