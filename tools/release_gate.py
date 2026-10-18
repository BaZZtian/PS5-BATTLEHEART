"""Validate a candidate folder and matching regional PS5 test evidence. Never publishes."""

import argparse
import hashlib
import json
import re
from pathlib import Path, PurePosixPath


FORBIDDEN_SUFFIXES = {
    ".iso", ".gcm", ".rvz", ".wia", ".wbfs", ".dol", ".rel", ".xex",
    ".nxpack", ".nxidx", ".mesp", ".prs", ".gvm", ".pvr", ".gvr",
    ".sfd", ".adx", ".thp", ".pak", ".stfs",
}
ORIGINS = {"own_code", "licensed_dependency", "original_artwork", "runtime_metadata", "license_text"}
BASE_CHECKS = {"boot", "gameplay", "audio", "controller", "save_load"}


def sha256(path):
    result = hashlib.sha256()
    with Path(path).open("rb") as source:
        for chunk in iter(lambda: source.read(1024 * 1024), b""):
            result.update(chunk)
    return result.hexdigest()


def build_id(files):
    encoded = json.dumps(files, sort_keys=True, separators=(",", ":")).encode("utf-8")
    return hashlib.sha256(encoded).hexdigest()


def check_release(candidate, evidence, project):
    root = Path(candidate).resolve()
    problems = []
    manifest = evidence.get("files", {})
    if not isinstance(manifest, dict) or not manifest:
        return ["No reviewed file manifest"], None
    digests = {}
    for relative, record in manifest.items():
        path = PurePosixPath(relative)
        if (not relative or path.is_absolute() or "\\" in relative or ":" in relative
                or any(part in {".", "..", ".git", "orig", "private-inputs"} for part in path.parts)
                or path.as_posix() != relative):
            problems.append(f"Invalid package path: {relative}")
            continue
        if path.suffix.lower() in FORBIDDEN_SUFFIXES:
            problems.append(f"Game-input format is forbidden in release: {relative}")
        item = root.joinpath(*path.parts)
        if not item.resolve().is_relative_to(root):
            problems.append(f"Path escapes candidate folder: {relative}")
            continue
        if any(parent.is_symlink() for parent in [item, *list(item.parents)[:-1]]):
            problems.append(f"Symbolic links are forbidden in candidate: {relative}")
            continue
        if not item.is_file():
            problems.append(f"Missing file: {relative}")
            continue
        if not isinstance(record, dict):
            problems.append(f"Invalid provenance record: {relative}")
            continue
        if record.get("origin") not in ORIGINS or not record.get("license"):
            problems.append(f"Unreviewed origin or missing license: {relative}")
        actual = sha256(item)
        digests[relative] = actual
        if record.get("sha256") != actual:
            problems.append(f"File differs from reviewed build: {relative}")
    for item in root.rglob("*"):
        relative = item.relative_to(root).as_posix()
        if item.is_symlink():
            problems.append(f"Symbolic link in candidate: {relative}")
        elif item.is_file() and relative not in manifest:
            problems.append(f"Unlisted package file: {relative}")
    for required in {"eboot.bin", "sce_sys/param.json"}:
        if required not in digests:
            problems.append(f"Missing required native PS5 file: {required}")
    param = root / "sce_sys" / "param.json"
    if "sce_sys/param.json" in digests:
        try:
            metadata = json.loads(param.read_text(encoding="utf-8"))
            version = metadata.get("contentVersion", "")
            if not isinstance(version, str) or not re.fullmatch(r"\d{2}\.\d{3}\.\d{3}", version):
                problems.append("Missing valid contentVersion for store updates")
        except (ValueError, OSError):
            problems.append("Invalid PS5 param.json")
    current_build = build_id(digests)
    required_checks = set(BASE_CHECKS)
    if project["id"] == "sonic-adventure-2-battle":
        required_checks.update({"chao_battle", "multiplayer"})
    regional = evidence.get("regional_tests", {})
    for region, disc_id in project["disc_ids"].items():
        proof = regional.get(region, {})
        if proof.get("status") != "passed":
            problems.append(f"No passed PS5 gameplay test: {region}")
            continue
        if proof.get("build_id") != current_build:
            problems.append(f"Regional evidence is for another build: {region}")
        if proof.get("disc_id") != disc_id or proof.get("firmware") != "12.40":
            problems.append(f"Wrong disc or target firmware in evidence: {region}")
        if (type(proof.get("disc_revision")) is not int
                or not 0 <= proof["disc_revision"] <= 255
                or not isinstance(proof.get("disc_sha256"), str)
                or not re.fullmatch(r"[a-f0-9]{64}", proof["disc_sha256"])):
            problems.append(f"Missing precise disc revision or SHA-256: {region}")
        if proof.get("test_kind") != "ps5-gameplay":
            problems.append(f"Evidence is not a PS5 gameplay test: {region}")
        if not proof.get("tested_at") or not proof.get("evidence_reference"):
            problems.append(f"Missing dated test reference: {region}")
        checks = proof.get("checks", {})
        for check in sorted(required_checks):
            if checks.get(check) is not True:
                problems.append(f"Missing gameplay check {check}: {region}")
    return problems, current_build


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("project")
    parser.add_argument("candidate", type=Path)
    parser.add_argument("evidence", type=Path)
    args = parser.parse_args()
    projects = json.loads((Path(__file__).parents[1] / "projects.json").read_text(encoding="utf-8"))
    project = next((p for p in projects["projects"] if p["id"] == args.project), None)
    if project is None:
        parser.error("Unknown project")
    try:
        evidence = json.loads(args.evidence.read_text(encoding="utf-8"))
        problems, current_build = check_release(args.candidate, evidence, project)
    except (OSError, ValueError, TypeError, AttributeError) as error:
        problems, current_build = [f"Invalid release input: {error}"], None
    print(json.dumps({"release_allowed": not problems, "build_id": current_build,
                      "problems": problems}, indent=2))
    return 2 if problems else 0


if __name__ == "__main__":
    raise SystemExit(main())
