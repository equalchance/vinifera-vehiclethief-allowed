"""Compile the local probe using an existing x86 Vinifera build's flags."""
from pathlib import Path
import argparse
import ctypes
from ctypes import wintypes
import json
import os
import subprocess


def split_command(command):
    parse = ctypes.windll.shell32.CommandLineToArgvW
    parse.argtypes = [wintypes.LPCWSTR, ctypes.POINTER(ctypes.c_int)]
    parse.restype = ctypes.POINTER(wintypes.LPWSTR)
    free = ctypes.windll.kernel32.LocalFree
    free.argtypes = [wintypes.HLOCAL]
    free.restype = wintypes.HLOCAL
    count = ctypes.c_int()
    argv = parse(command, ctypes.byref(count))
    if not argv:
        raise ctypes.WinError()
    try:
        return [argv[index] for index in range(count.value)]
    finally:
        free(ctypes.cast(argv, wintypes.HLOCAL))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--build-dir', type=Path, required=True)
    parser.add_argument('--output-dir', type=Path, default=Path('probe-build'))
    options = parser.parse_args()
    if os.name != 'nt':
        parser.error('Use Windows and an x86 Native Tools prompt.')
    build = options.build_dir.resolve()
    output = options.output_dir.resolve()
    output.mkdir(parents=True, exist_ok=True)
    records = json.loads((build/'compile_commands.json').read_text())
    entry = next(record for record in records if record['file'].replace(chr(92),'/').endswith('/technotypeext.cpp'))
    argv = entry.get('arguments') or split_command(entry['command'])
    directory = Path(entry['directory'])
    original = Path(entry['file'])
    if not original.is_absolute():
        original = directory/original
    original = os.path.normcase(str(original.resolve()))
    flags, skip_next = [], False
    for argument in argv[1:]:
        if skip_next:
            skip_next = False
            continue
        lower = argument.lower()
        if lower in ('/fo','/fd'):
            skip_next = True
            continue
        if lower.startswith(('/fo','/fd')) or lower in ('-c','/c'):
            continue
        if lower.endswith('.cpp'):
            candidate = Path(argument)
            if not candidate.is_absolute():
                candidate = directory/candidate
            if os.path.normcase(str(candidate.resolve())) == original:
                continue
        flags.append(argument)
    source = Path(__file__).resolve().parents[1]/'diagnostics/native_execution_probe.cpp'
    command = [argv[0], *flags, '/LD', str(source), '/Fe'+str(output/'NativeProbe.dll'),
               '/Fo'+str(output/'NativeProbe.obj'), '/Fd'+str(output/'NativeProbe-compile.pdb'),
               '/link', '/PDB:'+str(output/'NativeProbe.pdb')]
    subprocess.run(command, cwd=directory, check=True)
    print('Built '+str(output/'NativeProbe.dll'))


if __name__ == '__main__':
    main()
