from pathlib import Path

VERSION = '0.1.0'

def doctestify(test):
    lines = test.splitlines()
    markdown_lines = []
    in_code_block = False

    for line in lines:
        if line.startswith("\\") or "-- pragma:hide" in line:
            continue
        if line.startswith("--") and not line.startswith("---"):
            if in_code_block:
                markdown_lines.append("```")
                in_code_block = False
            markdown_lines.append(line[3:])
        else:
            if not in_code_block:
                markdown_lines.append("``` postgres-console")
                in_code_block = True
            line = line.replace("\u21B5", " ")
            markdown_lines.append(line)

    if in_code_block:
        markdown_lines.append("```")

    return "\n".join(markdown_lines)

if __name__ == '__main__':
    import sys
    test = Path(sys.argv[1])
    infile = open(test, 'r')
    doc = Path('docs', *test.with_suffix('.md').parts[1:])
    outfile = open(doc, 'w+')
    outfile.write(doctestify(infile.read()))
