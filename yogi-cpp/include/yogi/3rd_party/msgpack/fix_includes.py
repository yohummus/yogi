#!/usr/bin/env python3
import os
import sys
import re

SOURCE_DIRECTORY = os.path.dirname(sys.argv[0])
HEADER_FILE_EXTENSIONS = ['.h', '.hpp']


def get_header_files() -> [(str, str)]:
    files = []
    for dir, _, filenames in os.walk(SOURCE_DIRECTORY):
        for filename in filenames:
            if os.path.splitext(filename)[1] in HEADER_FILE_EXTENSIONS:
                files.append((dir, filename))
    return files


def fix_includes(dir, filename) -> None:
    path = os.path.join(dir, filename)
    with open(path, 'r') as file:
        content = file.read()

    for inc_stat in re.findall(r'(# *include *["<](msgpack/.*)[">])', content):
        orig_stat = inc_stat[0]
        orig_path = inc_stat[1]
        real_path = os.path.join(SOURCE_DIRECTORY, orig_path)
        new_path = os.path.relpath(real_path, dir).replace('\\', '/')
        new_stat = orig_stat.replace(orig_path, new_path)
        new_stat = new_stat.replace('<', '"')
        new_stat = new_stat.replace('>', '"')
        content = content.replace(orig_stat, new_stat)

    with open(path, 'w') as file:
        file.write(content)


if __name__ == "__main__":
    files = get_header_files()
    for file in files:
        fix_includes(file[0], file[1])
