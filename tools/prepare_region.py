"""Prepare private regional ISO inputs. Does not build, upload or run a game."""

import argparse
from datetime import datetime, timezone
import hashlib
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys
import tempfile


ROOT = Path(__file__).resolve().parents[1]


class PreparationError(Exception):
    pass


def run(command):
    result = subprocess.run(command, capture_output=True, text=True, encoding="utf-8",
                            errors="replace", check=False)
    if result.returncode:
        raise PreparationError(result.stderr.strip() or result.stdout.strip() or "Tool failed")
    return result.stdout


def wsl_path(path, distro):
    return run(["wsl.exe", "-d", distro, "--exec", "wslpath", "-a", "-u", str(path)]).strip()


def host_inspector(distro):
    """Build current sources and run synthetic tests before accepting real inputs."""
    if sys.platform == "win32":
        if not shutil.which("wsl.exe"):
            raise PreparationError("WSL is required on Windows, or supply --inspector with a native tool")
        prefix = ["wsl.exe", "-d", distro, "--exec"]
        source = wsl_path(ROOT, distro)
        convert = lambda path: wsl_path(path, distro)
    else:
        prefix = []
        source = str(ROOT)
        convert = str
    compiler = None
    candidates = (["/usr/bin/clang-18", "/usr/bin/clang", "/usr/bin/cc", "/usr/bin/gcc"]
                  if prefix else [shutil.which(name) for name in ("clang-18", "clang", "cc", "gcc")])
    for candidate in filter(None, candidates):
        try:
            run(prefix + [candidate, "--version"])
            compiler = candidate
            break
        except (PreparationError, OSError):
            continue
    if compiler is None:
        raise PreparationError("No C compiler found; install Clang or GCC in the selected Linux environment")
    build = source + "/build/host"
    commands = [
        ["cmake", "-S", source, "-B", build, "-G", "Ninja", "-DCMAKE_BUILD_TYPE=Debug",
         "-DCMAKE_C_COMPILER=" + compiler],
        ["cmake", "--build", build],
        ["ctest", "--test-dir", build, "--output-on-failure"],
    ]
    for command in commands:
        output = run(prefix + command)
        print(output, file=sys.stderr, end="" if output.endswith("\n") else "\n")
    return prefix + [build + "/gc_inspect"], convert


def stat_identity(path):
    stat = path.stat()
    return stat.st_size, stat.st_mtime_ns, stat.st_ctime_ns, stat.st_dev, stat.st_ino


def fingerprint(path):
    digests = {"sha256": hashlib.sha256(), "sha1": hashlib.sha1()}
    with path.open("rb") as source:
        for block in iter(lambda: source.read(4 * 1024 * 1024), b""):
            for digest in digests.values():
                digest.update(block)
    return {name: digest.hexdigest() for name, digest in digests.items()}


def atomic_json(path, value):
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = None
    try:
        with tempfile.NamedTemporaryFile(mode="w", encoding="utf-8", dir=path.parent,
                                         delete=False, suffix=".tmp") as target:
            temporary = Path(target.name)
            json.dump(value, target, indent=2, ensure_ascii=True)
            target.write("\n")
        os.replace(temporary, path)
    finally:
        if temporary is not None:
            temporary.unlink(missing_ok=True)


def prepare(iso, project, region, inspector, convert=str, private_root=None, expected_sha256=None):
    iso = Path(iso).resolve(strict=True)
    if not iso.is_file() or iso.suffix.lower() not in {".iso", ".gcm"}:
        raise PreparationError("Supply your own uncompressed GameCube .iso or .gcm file")
    if region not in project["disc_ids"]:
        raise PreparationError("Region is not configured for this project")
    if expected_sha256 is not None and not re.fullmatch(r"[a-fA-F0-9]{64}", expected_sha256):
        raise PreparationError("Expected SHA-256 must contain exactly 64 hexadecimal characters")
    identity = stat_identity(iso)
    try:
        disc = json.loads(run([*inspector, convert(iso)]))
    except (ValueError, TypeError) as error:
        raise PreparationError("Inspector returned invalid metadata") from error
    if (not isinstance(disc, dict) or disc.get("game_id") != project["disc_ids"][region]
            or disc.get("region") != region or disc.get("disc") != 0):
        raise PreparationError("Wrong game, region or disc number; input was not registered")
    for key in ("revision", "fst_entries", "dol_offset", "dol_size"):
        if type(disc.get(key)) is not int or disc[key] < 0:
            raise PreparationError("Inspector metadata is incomplete")
    if disc["revision"] > 255 or disc["fst_entries"] < 1 or disc["dol_size"] <= 0x100:
        raise PreparationError("Invalid disc or executable metadata")
    digests = fingerprint(iso)
    if stat_identity(iso) != identity:
        raise PreparationError("Input changed during validation; retry with an unchanged local file")
    if expected_sha256 is not None and digests["sha256"] != expected_sha256.lower():
        raise PreparationError("Input does not match the expected SHA-256; not registered")
    metadata = {
        "schema": 1,
        "project": project["id"],
        "region": region,
        "disc_id": disc["game_id"],
        "disc_number": disc["disc"],
        "disc_revision": disc["revision"],
        "bytes": identity[0],
        **digests,
        "fst_entries": disc["fst_entries"],
        "dol_size": disc["dol_size"],
        "checked_at": datetime.now(timezone.utc).isoformat(),
        "validation": {
            "disc_structure": "passed",
            "boot_dol_bounds": "passed",
            "revision_compatibility": "not_tested",
            "ps5_gameplay": "not_tested",
        },
        "input_prepared": True,
        "native_game_build_available": False,
        "release_allowed": False,
    }
    storage = Path(private_root) if private_root is not None else ROOT / "private-inputs"
    folder = storage / project["id"] / region / digests["sha256"]
    atomic_json(folder / "metadata.json", metadata)
    checks = {name: None for name in ("boot", "gameplay", "audio", "controller", "save_load")}
    if project["id"] == "sonic-adventure-2-battle":
        checks.update({"chao_battle": None, "multiplayer": None})
    template = {
        "schema": 1, "project": project["id"], "regional_tests": {
            region: {
                "status": "not_tested", "test_kind": "ps5-gameplay",
                "disc_id": disc["game_id"], "disc_revision": disc["revision"],
                "disc_sha256": digests["sha256"], "firmware": "12.40",
                "build_id": None, "tested_at": None, "evidence_reference": None,
                "checks": checks,
            }
        }
    }
    template_path = folder / "ps5-test-template.json"
    if not template_path.exists():
        atomic_json(template_path, template)
    atomic_json(folder / "input.json", {
        "schema": 1, "source_path": str(iso), "metadata": metadata,
        "read_only": True, "original_data_copied": False,
    })
    atomic_json(folder.parent / "current.json", {"sha256": digests["sha256"]})
    return metadata


def main():
    config = json.loads((ROOT / "projects.json").read_text(encoding="utf-8"))
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("project", nargs="?", default=config["projects"][0]["id"],
                        choices=[p["id"] for p in config["projects"]])
    parser.add_argument("--iso", required=True, type=Path)
    parser.add_argument("--region", choices=config["required_release_regions"], default="NTSC-U")
    parser.add_argument("--expected-sha256")
    parser.add_argument("--wsl-distro", default="Ubuntu-24.04")
    parser.add_argument("--inspector", type=Path, help="Use an already built native gc_inspect (advanced)")
    args = parser.parse_args()
    project = next(p for p in config["projects"] if p["id"] == args.project)
    try:
        if args.inspector:
            inspector, convert = [str(args.inspector.resolve(strict=True))], str
        else:
            inspector, convert = host_inspector(args.wsl_distro)
        result = prepare(args.iso, project, args.region, inspector, convert,
                         expected_sha256=args.expected_sha256)
    except (PreparationError, OSError) as error:
        print(json.dumps({"input_prepared": False, "release_allowed": False, "error": str(error)}, indent=2))
        return 2
    print(json.dumps(result, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
