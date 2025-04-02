import os
import sys


def process_file(path):
    with open(path, "r") as f:
        content = f.read()
    s = content.lstrip()
    if not s.startswith("/* doctest"):
        return
    end = s.find("*/")
    if end == -1:
        return
    comment = s[: end + 2]
    lines = comment.splitlines()
    if not lines:
        return
    first = lines[0]
    if not first.startswith("/* doctest"):
        return
    target = first[len("/* doctest"):].strip()
    if target.startswith("/"):
        target = target[1:]
    if not target:
        return
    target = target.replace('/', '_')
    stripped_comment = "\n".join(lines[1:-1]).strip() if len(lines) >= 3 else ""
    out_file = os.path.join("sql", target + ".sql")
    os.makedirs(os.path.dirname(out_file), exist_ok=True)
    with open(out_file, "w") as out:
        out.write(stripped_comment + "\n\n")


def main():
    base = sys.argv[1] if len(sys.argv) > 1 else "."
    for root, _, files in os.walk(base):
        for f in files:
            if f.endswith(".c"):
                process_file(os.path.join(root, f))


if __name__ == "__main__":
    main()
