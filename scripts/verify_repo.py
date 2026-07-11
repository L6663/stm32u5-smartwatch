#!/usr/bin/env python3
"""Verify that the Git-ready STM32U5 repository is internally consistent."""

from __future__ import annotations

import sys
import xml.etree.ElementTree as ET
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
PROJECT = ROOT / "MDK-ARM" / "STM32U5_LVGL_Inteligentlight.uvprojx"
MAX_GITHUB_FILE_SIZE = 100 * 1024 * 1024

FORBIDDEN_SUFFIXES = {
    ".axf", ".hex", ".map", ".o", ".obj", ".d", ".crf", ".lst", ".dep"
}
FORBIDDEN_DIR_NAMES = {"Objects", "Listings", "__pycache__"}


def normalize_keil_path(raw: str) -> Path:
    value = raw.replace("\\", "/")
    return (PROJECT.parent / value).resolve()


def main() -> int:
    errors: list[str] = []

    required = [
        ROOT / "STM32U5_LVGL_Inteligentlight.ioc",
        PROJECT,
        ROOT / "Core" / "Src" / "main.c",
        ROOT / "Core" / "Src" / "app_freertos.c",
        ROOT / "LVGL" / "UI" / "ui.c",
        ROOT / "LVGL" / "porting" / "lv_port_disp.c",
        ROOT / "LVGL" / "porting" / "lv_port_indev.c",
    ]

    for path in required:
        if not path.exists():
            errors.append(f"Missing required file: {path.relative_to(ROOT)}")

    referenced = 0
    missing_refs: list[str] = []
    if PROJECT.exists():
        try:
            tree = ET.parse(PROJECT)
            for element in tree.getroot().iter("FilePath"):
                if not element.text:
                    continue
                referenced += 1
                resolved = normalize_keil_path(element.text)
                if not resolved.exists():
                    missing_refs.append(element.text)
        except ET.ParseError as exc:
            errors.append(f"Invalid Keil project XML: {exc}")

    if missing_refs:
        errors.extend(f"Missing Keil reference: {item}" for item in missing_refs)

    forbidden: list[str] = []
    oversized: list[str] = []
    file_count = 0
    total_bytes = 0

    for path in ROOT.rglob("*"):
        if any(part in FORBIDDEN_DIR_NAMES for part in path.parts):
            if path.is_dir():
                forbidden.append(str(path.relative_to(ROOT)))
            continue
        if not path.is_file():
            continue

        file_count += 1
        size = path.stat().st_size
        total_bytes += size

        if path.suffix.lower() in FORBIDDEN_SUFFIXES:
            forbidden.append(str(path.relative_to(ROOT)))
        if size >= MAX_GITHUB_FILE_SIZE:
            oversized.append(str(path.relative_to(ROOT)))

    if forbidden:
        errors.extend(f"Forbidden build artifact: {item}" for item in sorted(set(forbidden)))
    if oversized:
        errors.extend(f"GitHub oversized file: {item}" for item in oversized)

    print("STM32U5 SmartWatch repository verification")
    print(f"Root: {ROOT}")
    print(f"Files: {file_count}")
    print(f"Source size: {total_bytes / (1024 * 1024):.2f} MiB")
    print(f"Keil FilePath references: {referenced}")
    print(f"Missing Keil references: {len(missing_refs)}")
    print(f"Forbidden artifacts: {len(set(forbidden))}")
    print(f"Files >= 100 MiB: {len(oversized)}")

    if errors:
        print("\nFAILED")
        for item in errors:
            print(f"- {item}")
        return 1

    print("\nPASS")
    return 0


if __name__ == "__main__":
    sys.exit(main())
