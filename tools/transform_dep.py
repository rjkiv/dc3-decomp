#!/usr/bin/env python3

"""
Normalise MSVC /showIncludes output so Ninja can consume it on non-Windows
platforms. Reads from stdin, writes the transformed lines to stdout.
"""

import os
import sys
from platform import uname

wineprefix = os.path.join(os.environ.get("HOME", ""), ".wine")
if "WINEPREFIX" in os.environ:
    wineprefix = os.environ["WINEPREFIX"]
winedevices = os.path.join(wineprefix, "dosdevices")

INCLUDE_PREFIX = "Note: including file:"


def in_wsl() -> bool:
    return "microsoft-standard" in uname().release


def normalize_path_case(path: str) -> str:
    if not path or not os.path.isabs(path):
        return path
    if path == "/":
        return path

    pieces = [piece for piece in path.split("/") if piece]
    if not pieces:
        return "/"

    corrected = []
    for idx, piece in enumerate(pieces):
        parent = "/" if not corrected else "/" + "/".join(corrected)
        try:
            entries = os.listdir(parent)
        except OSError:
            corrected.extend(pieces[idx:])
            break

        match = next(
            (entry for entry in entries if entry.lower() == piece.lower()), None
        )
        if match is None:
            corrected.append(piece)
            corrected.extend(pieces[idx + 1 :])
            break

        corrected.append(match)

    result = "/" + "/".join(corrected)
    if path.endswith("/") and not result.endswith("/"):
        result += "/"
    return result


def resolve_windows_path(raw_path: str) -> str:
    stripped = raw_path.strip()
    if not stripped:
        return stripped

    # Handle paths with drive letters (e.g. C:\ or Z:\)
    if len(stripped) >= 2 and stripped[1] == ":":
        drive = stripped[0].lower()
        remainder = stripped[2:].lstrip("\\/")
        remainder = remainder.replace("\\", "/")

        if drive == "z":
            result = "/" + remainder.lstrip("/")
        elif in_wsl():
            result = os.path.join("/mnt", drive, remainder)
        else:
            drive_path = os.path.join(winedevices, f"{drive}:")
            if os.path.isdir(drive_path):
                result = os.path.join(drive_path, remainder)
                result = os.path.realpath(result)
            else:
                result = f"/mnt/{drive}/{remainder}"

        result = os.path.normpath(result)
        return normalize_path_case(result)

    result = stripped.replace("\\", "/")
    if os.path.isabs(result):
        result = os.path.normpath(result)
        return normalize_path_case(result)
    return result


def transform_line(line: str) -> str:
    newline = ""
    if line.endswith("\r\n"):
        newline = "\r\n"
        content = line[:-2]
    elif line.endswith("\n"):
        newline = "\n"
        content = line[:-1]
    else:
        content = line

    if not content.startswith(INCLUDE_PREFIX):
        return line

    remainder = content[len(INCLUDE_PREFIX) :]
    indent_len = len(remainder) - len(remainder.lstrip(" "))
    indent = remainder[:indent_len]
    path_segment = remainder[indent_len:]

    if not path_segment.strip():
        return line

    trailing_ws_len = len(path_segment) - len(path_segment.rstrip(" "))
    trailing_ws = path_segment[-trailing_ws_len:] if trailing_ws_len else ""
    resolved = resolve_windows_path(path_segment)

    return f"{INCLUDE_PREFIX}{indent}{resolved}{trailing_ws}{newline}"


def main() -> None:
    for line in sys.stdin:
        sys.stdout.write(transform_line(line))
    sys.stdout.flush()


if __name__ == "__main__":
    main()
