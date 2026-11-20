import argparse
import os
import re
from pathlib import Path


def make_valid_identifier(name: str) -> str:
    identifier = re.sub(r"[^a-zA-Z0-9_]", "_", name)

    if not identifier:
        raise ValueError("C array name cannot be empty.")

    if identifier[0].isdigit():
        identifier = "_" + identifier

    return identifier


def file_to_header(
    input_file_path: str | Path,
    output_header_path: str | Path,
    output_source_path: str | Path,
    array_name: str,
    header_guard: str,
    namespace: str | None = None,
    bytes_per_line: int = 12,
):
    input_file_path = Path(input_file_path)
    output_header_path = Path(output_header_path)
    output_source_path = Path(output_source_path)

    file_content = input_file_path.read_bytes()

    output_header_path.parent.mkdir(parents=True, exist_ok=True)
    output_source_path.parent.mkdir(parents=True, exist_ok=True)

    with output_header_path.open("w", encoding="utf-8") as header_file:
        header_file.write(f"// Generated from {input_file_path.absolute()}\n")
        header_file.write(f"#ifndef {header_guard}\n")
        header_file.write(f"#define {header_guard}\n\n")

        header_file.write("#include <stddef.h>\n\n")

        if namespace is not None:
            header_file.write(f"namespace {namespace} {{\n\n")

        header_file.write(f"extern const unsigned char {array_name}[];\n")
        header_file.write(f"extern const size_t {array_name}_SIZE;\n\n")

        if namespace is not None:
            header_file.write(f"}} // namespace {namespace}\n\n")

        header_file.write(f"#endif // {header_guard}\n")

    with output_source_path.open("w", encoding="utf-8") as source_file:
        source_file.write(f"// Generated from {input_file_path.absolute()}\n")
        source_file.write(f'#include "{output_header_path.name}"\n\n')

        if namespace is not None:
            source_file.write(f"namespace {namespace} {{\n\n")

        source_file.write(f"const unsigned char {array_name}[] = {{\n")
        for offset in range(0, len(file_content), bytes_per_line):
            chunk = file_content[offset : offset + bytes_per_line]
            line = ", ".join(f"0x{b:02x}" for b in chunk)

            if offset + bytes_per_line < len(file_content):
                source_file.write(f"    {line},\n")
            else:
                source_file.write(f"    {line}\n")
        source_file.write("};\n\n")

        source_file.write(f"const size_t {array_name}_SIZE = sizeof({array_name});\n\n")

        if namespace is not None:
            source_file.write(f"}} // namespace {namespace}\n")


def parse_args():
    parser = argparse.ArgumentParser(description="Convert a file to a header")
    parser.add_argument("input_file_path", help="Input file")
    parser.add_argument("--output-dir", "-o", default=".", help="Output directory")
    parser.add_argument(
        "--output-name", "-n", help="Output filename for *.h and *.cc files"
    )
    parser.add_argument(
        "--array-name", "-a", help="Name of the array variable for the file content"
    )
    parser.add_argument("--header-guard", "-G", help="Header guard macro name")
    parser.add_argument("--namespace", "-N", help="Namespace for the generated header")
    parser.add_argument(
        "--bytes-per-line",
        type=int,
        default=12,
        help="Number of bytes per line in the generated array",
    )
    parser.add_argument(
        "--force", "-f", action="store_true", help="Force overwrite of existing files"
    )

    return parser.parse_args()


if __name__ == "__main__":
    args = parse_args()

    input_file_path = Path(args.input_file_path)
    output_dir = Path(args.output_dir)
    output_name = args.output_name
    array_name = args.array_name
    header_guard = args.header_guard
    namespace = args.namespace
    bytes_per_line = args.bytes_per_line

    if output_name is None:
        output_name = input_file_path.stem

    output_header_path = output_dir / (output_name + ".h")
    output_source_path = output_dir / (output_name + ".cc")

    if not args.force:
        if output_header_path.exists():
            raise FileExistsError(f"Header file already exists: {output_header_path}")
        if output_source_path.exists():
            raise FileExistsError(f"Source file already exists: {output_source_path}")

    if array_name is None:
        array_name = make_valid_identifier(output_name).upper()
    if header_guard is None:
        header_guard = (
            make_valid_identifier(os.path.relpath(output_header_path, ".")).upper()
            + "_"
        )

    file_to_header(
        input_file_path=input_file_path,
        output_header_path=output_header_path,
        output_source_path=output_source_path,
        array_name=array_name,
        header_guard=header_guard,
        namespace=namespace,
        bytes_per_line=bytes_per_line,
    )

    print(f"Generated header: {output_header_path}")
    print(f"Generated source: {output_source_path}")
    print(f"Array name: {array_name}")
    print(f"Header guard: {header_guard}")
    if namespace is not None:
        print(f"Namespace: {namespace}")
