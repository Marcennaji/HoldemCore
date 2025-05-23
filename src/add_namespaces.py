import os
import argparse
import subprocess
import re

def to_namespace(path_parts):
    return "::".join(["pkt"] + path_parts)

def is_comment_or_include_or_declaration(line):
    stripped = line.strip()
    return (
        not stripped or
        stripped.startswith("//") or
        stripped.startswith("/*") or
        stripped.startswith("*") or
        stripped.startswith("#include") or
        stripped.startswith("#pragma") or
        re.match(r'^(class|struct|enum|union)\s+\w+\s*;', stripped)
    )

def find_insertion_index(lines):
    in_block_comment = False
    for idx, line in enumerate(lines):
        stripped = line.strip()
        if in_block_comment:
            if "*/" in stripped:
                in_block_comment = False
            continue
        if stripped.startswith("/*"):
            in_block_comment = True
            continue
        if is_comment_or_include_or_declaration(line):
            continue
        return idx
    return -1  # fallback: no code found

def wrap_in_namespace(file_path, namespace_path):
    with open(file_path, 'r', encoding='utf-8') as f:
        lines = f.readlines()

    flat_ns = to_namespace(namespace_path)
    if any(f"namespace {flat_ns}" in line for line in lines):
        print(f"[SKIP] {file_path} already contains namespace {flat_ns}")
        return

    insert_at = find_insertion_index(lines)
    if insert_at == -1:
        print(f"[SKIP] {file_path}: no code found")
        return

    # Split the file
    preamble = lines[:insert_at]
    body = lines[insert_at:]

    indented_body = ["    " + line if line.strip() else "\n" for line in body]
    namespaced = [f"\nnamespace {flat_ns} {{\n\n"] + indented_body + [f"}} // namespace {flat_ns}\n"]

    with open(file_path, 'w', encoding='utf-8') as f:
        f.writelines(preamble + namespaced)

    try:
        subprocess.run(["clang-format", "-i", file_path], check=True)
        print(f"[OK] {file_path} (namespaced)")
    except Exception as e:
        print(f"[WARN] clang-format failed on {file_path}: {e}")

def is_excluded(abs_path, exclude_dirs):
    return any(abs_path.startswith(excluded) for excluded in exclude_dirs)

def process_directory(root_dir, exclude_dirs):
    for dirpath, _, filenames in os.walk(root_dir):
        abs_dirpath = os.path.abspath(dirpath)
        if is_excluded(abs_dirpath, exclude_dirs):
            continue
        for filename in filenames:
            if filename.endswith(('.cpp', '.h')):
                full_path = os.path.join(dirpath, filename)
                relative_path = os.path.relpath(full_path, root_dir)
                namespace_parts = os.path.dirname(relative_path).split(os.sep)
                if namespace_parts == ['.']:
                    namespace_parts = []
                wrap_in_namespace(full_path, namespace_parts)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Inject pkt::<namespace> into .cpp/.h files at the right spot.")
    parser.add_argument("root", help="Root directory (e.g., PokerTraining/src)")
    parser.add_argument("--exclude", action='append', default=[], help="Subdirectory to exclude (can be repeated)")
    args = parser.parse_args()

    root_dir = os.path.abspath(args.root)
    exclude_dirs = [os.path.abspath(os.path.join(root_dir, excl)) for excl in args.exclude]

    process_directory(root_dir, exclude_dirs)
