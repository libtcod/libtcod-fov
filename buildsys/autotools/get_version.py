#!/usr/bin/env python3
"""
Print the version number for libtcod.

The --so flag can be used to get the library version number instead.
"""

import re
import sys
from pathlib import Path

RE_MAJOR = ".*#define TCODFOV_MAJOR_VERSION *([0-9]+)"
RE_MINOR = ".*#define TCODFOV_MINOR_VERSION *([0-9]+)"
RE_PATCH = ".*#define TCODFOV_PATCHLEVEL *([0-9]+)"
RE_VERSION = RE_MAJOR + RE_MINOR + RE_PATCH


def main():
    header = Path("../../include/libtcod-fov/version.h").read_text("utf-8")
    major, minor, patch = re.match(RE_VERSION, header, re.DOTALL).groups()
    if "--so" in sys.argv:
        print("{major}:{minor}".format(**locals()))
    else:
        print("{major}.{minor}.{patch}".format(**locals()))


if __name__ == "__main__":
    main()
