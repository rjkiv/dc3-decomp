#!/usr/bin/env python3

###
# Generates a ctx.c file, usable for "Context" on https://decomp.me.
#
# Uses pcpp (a pure-Python C preprocessor) for proper include resolution,
# macro expansion, and conditional compilation.
#
# Usage:
#   python3 tools/decompctx.py src/file.cpp -o ctx.c -d ctx.c.d -I src -I src/system
#
# Requires: pip install pcpp (or see requirements.txt)
#
# Based on https://github.com/DarkRTA/rb3/blob/master/tools/decompctx.py
###

import argparse
import io
import os
import sys

from io import StringIO
from pcpp import CmdPreprocessor
from contextlib import redirect_stdout

script_dir = os.path.dirname(os.path.realpath(__file__))
root_dir = os.path.abspath(os.path.join(script_dir, ".."))

default_arguments = [
    "--compress",
    "--line-directive", "\n#line",
]

default_defines = {
    "_XBOX": "1",
    "_M_PPCBE": "1",
    "_MSC_VER": "1600",
    "_MSC_EXTENSIONS": "1",
    "_WIN32": "1",
    "_WCHAR_T_DEFINED": "1",
}

passthrough_defines = [
    # C/C++ standard — passed through so pcpp keeps both C/C++ branches
    # and MSVC evaluates them natively at compile time
    "__cplusplus",

    # Debug
    "NDEBUG",
    "MILO_DEBUG",

    # MSVC builtins
    "__declspec",
    "section",
    "dllexport",
    "dllimport",
    "noreturn",

    # STLport namespace control
    "_STLP_HAS_NO_NAMESPACES",
    "_STLP_USE_NAMESPACES",
    "_STLP_USE_OWN_NAMESPACE",
    "_STLP_NO_NAMESPACES",
]


class ContextPreprocessor(CmdPreprocessor):
    def __init__(self, args):
        self.in_directive = False
        self.include_deps = []
        super().__init__(args)

    def on_include_not_found(self, is_malformed, is_system_include, curdir, includepath):
        # Fix files that use <> for relative includes
        if not is_malformed and os.path.exists(os.path.join(curdir, includepath)):
            return curdir
        return super().on_include_not_found(
            is_malformed, is_system_include, curdir, includepath
        )

    def on_unknown_macro_in_expr(self, ident):
        if ident in passthrough_defines:
            return None
        return super().on_unknown_macro_in_expr(ident)

    def on_unknown_macro_in_defined_expr(self, tok):
        if tok.value in passthrough_defines:
            return None
        return super().on_unknown_macro_in_defined_expr(tok)

    def expand_macros(self, tokens, expanding_from=[]):
        # Don't expand macros outside of preprocessor directives
        if not self.in_directive:
            return tokens
        ret = super().expand_macros(tokens, expanding_from)
        self.in_directive = False
        return ret

    def evalexpr(self, tokens):
        self.in_directive = True
        return super().evalexpr(tokens)

    def include(self, tokens, original_line):
        self.in_directive = True
        return super().include(tokens, original_line)

    def on_file_open(self, is_system_include, includepath):
        # Try UTF-8 first, fall back to Latin-1 for files with non-UTF-8
        # characters (e.g. STLport copyright headers with accented names)
        for encoding in ("utf-8", "latin-1"):
            try:
                f = io.open(includepath, "r", encoding=encoding)
                data = f.read()
                f.close()
                if data.startswith("\ufeff"):
                    data = data[1:]
                sf = StringIO(data)
                sf.name = includepath
                self.include_deps.append(includepath)
                return sf
            except UnicodeDecodeError:
                continue
            except IOError:
                raise
        raise IOError(f"Cannot decode {includepath}")


def sanitize_path(path):
    return path.replace("\\", "/").replace(" ", "\\ ")


def main():
    # STLport headers create deep include chains
    sys.setrecursionlimit(10000)

    parser = argparse.ArgumentParser(
        description="Generate a context file for decomp.me using pcpp"
    )
    parser.add_argument("c_file", help="File from which to create context")
    parser.add_argument("-o", "--output", default="ctx.c", help="Output file")
    parser.add_argument("-d", "--depfile", help="Dependency file")
    parser.add_argument("-I", "--include", action="append", help="Include directory")
    parser.add_argument("-D", "--define", action="append", help="Macro definition")
    args = parser.parse_args()

    # Build pcpp arguments
    pcpp_args = ["pcpp"]

    # Include directories
    for inc_dir in args.include or []:
        pcpp_args.extend(("-I", inc_dir))

    # Collect explicit defines to avoid overriding with defaults
    known_defines = set()
    if args.define:
        for define in args.define:
            known_defines.add(define.split("=")[0])

    # Default defines (skip if explicitly provided)
    for name, value in default_defines.items():
        if name not in known_defines:
            pcpp_args.extend(("-D", f"{name}={value}"))

    # Extra defines from command line
    for define in args.define or []:
        pcpp_args.extend(("-D", define))

    # pcpp options
    pcpp_args.append("--passthru-defines")
    pcpp_args.extend(default_arguments)
    pcpp_args.append(args.c_file)

    # Run preprocessor
    with StringIO() as output_buf:
        with redirect_stdout(output_buf):
            processor = ContextPreprocessor(pcpp_args)

        if output_buf.tell() == 0:
            return

        output = output_buf.getvalue()

        # STLport remaps std to stlpmtx_std via #define, but native C++
        # headers (like xdk/LIBCMT/cmath) declare functions in literal
        # namespace std (because prolog #undefs std before native includes).
        # Bridge the gap so stlpmtx_std:: lookups find std:: declarations.
        output = "namespace stlpmtx_std { using namespace std; }\n" + output

        output_path = os.path.join(root_dir, args.output)
        with open(output_path, "w", encoding="utf-8", newline="\n") as f:
            f.write(output)

    # Write dependency file
    if args.depfile:
        san_root = sanitize_path(root_dir)
        if not san_root.endswith("/"):
            san_root += "/"

        def make_relative(path):
            return sanitize_path(path).replace(san_root, "")

        with open(os.path.join(root_dir, args.depfile), "w", encoding="utf-8") as f:
            f.write(make_relative(args.output) + ":")
            for dep in processor.include_deps:
                f.write(f" \\\n\t{make_relative(dep)}")


if __name__ == "__main__":
    main()
