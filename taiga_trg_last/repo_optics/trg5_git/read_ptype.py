#!/usr/bin/env python3
"""Read particle type from the first event in a TAIGA *_i binary file."""

from __future__ import annotations

import argparse
import struct
from pathlib import Path
from typing import BinaryIO


_PTYPE_NAME = {
    1: "gamma",
    14: "proton",
    5626: "iron",
    6: "muon-",
}


def _read_exact(fobj: BinaryIO, size: int) -> bytes:
    data = fobj.read(size)
    if len(data) != size:
        raise EOFError(f"Unexpected EOF while reading {size} bytes")
    return data


def read_first_event_particle_type(path: str | Path) -> int:
    """Return particle type code stored in the first event header of *_i file."""
    p = Path(path)
    with p.open("rb") as f:
        main_header = struct.unpack("<i", _read_exact(f, 4))[0]
        if main_header not in (6, 41, 42, 43):
            raise ValueError(f"Unsupported file header: {main_header}")

        # Headers 42/43 contain one extra int (Nevent).
        if main_header in (42, 43):
            _read_exact(f, 4)

        subheader = struct.unpack("<i", _read_exact(f, 4))[0]
        if subheader == 52:
            vals = struct.unpack("<14d", _read_exact(f, struct.calcsize("<14d")))
        elif subheader == 53:
            vals = struct.unpack("<15d", _read_exact(f, struct.calcsize("<15d")))
        elif subheader == 54:
            vals = struct.unpack("<15f", _read_exact(f, struct.calcsize("<15f")))
        else:
            raise ValueError(f"Unsupported event subheader: {subheader}")

        ptype_code = int(round(float(vals[7])))
        return ptype_code


def particle_type_name(ptype_code: int) -> str:
    """Map particle type code to short name."""
    return _PTYPE_NAME.get(ptype_code, f"unknown({ptype_code})")


def main() -> None:
    parser = argparse.ArgumentParser(description="Read particle type from first event of *_i file")
    parser.add_argument("input_i", type=Path, help="Path to *_i file")
    args = parser.parse_args()

    ptype = read_first_event_particle_type(args.input_i)
    print(f"ptype={ptype} ({particle_type_name(ptype)})")


if __name__ == "__main__":
    main()
