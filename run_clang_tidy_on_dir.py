#!/usr/bin/env python3

# usage example : python run_clang_tidy_dir.py src/core/interfaces --build-dir build/debug-widgets --checks readability-identifier-naming


import argparse
import subprocess
from pathlib import Path
import json
import os


def load_compile_commands(compile_commands_path):
    with open(compile_commands_path, encoding='utf-8') as f:
        data = json.load(f)
    return {Path(entry["file"]).resolve(): entry for entry in data}


def run_clang_tidy(file_path, build_dir, checks):
    print(f"🛠️  Running clang-tidy on: {file_path}")
    result = subprocess.run([
        "clang-tidy",
        str(file_path),
        "-p", str(build_dir),
        "-fix",
        f"-checks={checks}"
    ])
    if result.returncode != 0:
        print(f"❌ Error on file: {file_path}")


def main():
    parser = argparse.ArgumentParser(description="Run clang-tidy on all C++ files in a directory.")
    parser.add_argument("source_dir", type=Path, help="Directory to search for .cpp/.h files.")
    parser.add_argument("--build-dir", type=Path, default=Path("build/debug-widgets"), help="Path to build dir with compile_commands.json")
    parser.add_argument("--checks", default="readability-identifier-naming", help="Checks to apply")
    args = parser.parse_args()

    compile_commands_file = args.build_dir / "compile_commands.json"
    if not compile_commands_file.exists():
        print(f"❌ compile_commands.json not found at {compile_commands_file}")
        return

    db = load_compile_commands(compile_commands_file)

    cpp_headers = list(args.source_dir.rglob("*.cpp")) + list(args.source_dir.rglob("*.h"))
    if not cpp_headers:
        print(f"⚠️ No .cpp or .h files found in {args.source_dir}")
        return

    for file in cpp_headers:
        resolved = file.resolve()
        if resolved not in db:
            print(f"⏭️  (not in compile_commands.json): {file}, processing anyway")
        run_clang_tidy(resolved, args.build_dir, args.checks)
 

    print("✅ Done.")


if __name__ == "__main__":
    main()
