import contextlib
import importlib.util
import io
import json
import os
from pathlib import Path
import shutil
import struct
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location("prepare_region", ROOT / "tools/prepare_region.py")
intake = importlib.util.module_from_spec(spec)
spec.loader.exec_module(intake)


def synthetic_disc(game_id="GSNE8P", revision=0):
    """Only invented bytes: no original executable, files or media."""
    data = bytearray(0x1000)
    data[:6] = game_id.encode("ascii")
    data[7] = revision
    struct.pack_into(">I", data, 0x1c, 0xc2339f3d)
    struct.pack_into(">III", data, 0x420, 0x500, 0x800, 12)
    struct.pack_into(">I", data, 0x500, 0x100)
    struct.pack_into(">I", data, 0x590, 4)
    data[0x600:0x604] = b"FAKE"
    struct.pack_into(">III", data, 0x800, 0x01000000, 0, 1)
    return data


class PreparationTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        tool = os.environ.get("GC_INSPECT")
        if not tool or not Path(tool).is_file():
            raise RuntimeError("Build gc_inspect and run through CTest (GC_INSPECT is required)")
        cls.inspector = [str(Path(tool).resolve())]

    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.iso = self.root / "Own disc with spaces.iso"
        self.storage = self.root / "private"
        config = json.loads((ROOT / "projects.json").read_text(encoding="utf-8"))
        self.projects = config["projects"]
        self.project = next((p for p in self.projects if p["id"] == "sonic-adventure-2-battle"),
                            self.projects[0])
        self.iso.write_bytes(self.disc())

    def disc(self, region="NTSC-U", revision=0):
        return synthetic_disc(self.project["disc_ids"][region], revision)

    def prepare(self, region="NTSC-U", **kwargs):
        return intake.prepare(self.iso, self.project, region, self.inspector,
                              private_root=self.storage, **kwargs)

    def test_prepares_without_copying_or_modifying_original_bytes(self):
        before = self.iso.read_bytes()
        report = self.prepare()
        self.assertTrue(report["input_prepared"])
        self.assertFalse(report["native_game_build_available"])
        self.assertFalse(report["release_allowed"])
        self.assertEqual(report["validation"]["ps5_gameplay"], "not_tested")
        self.assertNotIn(str(self.root), json.dumps(report))
        files = list(self.storage.rglob("*"))
        self.assertEqual(sorted(p.name for p in files if p.is_file()),
                         ["current.json", "input.json", "metadata.json", "ps5-test-template.json"])
        record = json.loads(next(self.storage.rglob("input.json")).read_text())
        self.assertEqual(record["source_path"], str(self.iso.resolve()))
        self.assertFalse(record["original_data_copied"])
        self.assertEqual(self.iso.read_bytes(), before)

    def test_configured_project_ids_accept_only_their_regions(self):
        for project in self.projects:
            for region, disc in project["disc_ids"].items():
                with self.subTest(project=project["id"], region=region):
                    self.iso.write_bytes(synthetic_disc(disc))
                    report = intake.prepare(self.iso, project, region, self.inspector,
                                            private_root=self.storage)
                    self.assertEqual(report["disc_id"], disc)
                    self.assertEqual(report["region"], region)
                    self.assertFalse(report["release_allowed"])

    def test_pal_is_not_accepted_as_ntsc(self):
        self.iso.write_bytes(self.disc("PAL"))
        with self.assertRaisesRegex(intake.PreparationError, "Wrong game, region"):
            self.prepare()
        self.assertFalse(self.storage.exists())

    def test_other_game_is_rejected(self):
        other = "GM8E01" if self.project["disc_ids"]["NTSC-U"] != "GM8E01" else "GSNE8P"
        self.iso.write_bytes(synthetic_disc(other))
        with self.assertRaisesRegex(intake.PreparationError, "Wrong game"):
            self.prepare()

    def test_nonzero_disc_number_is_rejected(self):
        data = self.disc()
        data[6] = 1
        self.iso.write_bytes(data)
        with self.assertRaisesRegex(intake.PreparationError, "disc number"):
            self.prepare()

    def test_unknown_revision_is_recorded_but_never_marked_compatible(self):
        self.iso.write_bytes(self.disc(revision=7))
        report = self.prepare()
        self.assertEqual(report["disc_revision"], 7)
        self.assertEqual(report["validation"]["revision_compatibility"], "not_tested")

    def test_invalid_dol_rejected_by_real_c_inspector(self):
        data = self.disc()
        struct.pack_into(">I", data, 0x590, 0x100000)
        self.iso.write_bytes(data)
        with self.assertRaises(intake.PreparationError):
            self.prepare()
        self.assertFalse(self.storage.exists())

    def test_invalid_fst_rejected_by_real_c_inspector(self):
        data = self.disc()
        struct.pack_into(">I", data, 0x808, 0xffffffff)
        self.iso.write_bytes(data)
        with self.assertRaises(intake.PreparationError):
            self.prepare()

    def test_expected_hash_mismatch_rejected(self):
        with self.assertRaisesRegex(intake.PreparationError, "expected SHA-256"):
            self.prepare(expected_sha256="0" * 64)
        self.assertFalse(self.storage.exists())

    def test_expected_hash_match_and_repeat_are_idempotent(self):
        digest = intake.fingerprint(self.iso)["sha256"]
        self.prepare(expected_sha256=digest.upper())
        self.prepare(expected_sha256=digest)
        self.assertEqual(len(list(self.storage.rglob("input.json"))), 1)

    def test_report_template_never_claims_passed_tests_and_preserves_notes(self):
        report = self.prepare()
        path = next(self.storage.rglob("ps5-test-template.json"))
        template = json.loads(path.read_text())
        regional = template["regional_tests"]["NTSC-U"]
        self.assertEqual(regional["disc_sha256"], report["sha256"])
        self.assertEqual(regional["status"], "not_tested")
        self.assertIsNone(regional["build_id"])
        self.assertTrue(all(value is None for value in regional["checks"].values()))
        template["notes"] = "Tester notes must survive a repeated preparation"
        path.write_text(json.dumps(template))
        self.prepare()
        self.assertIn("notes", json.loads(path.read_text()))

    def test_modified_input_cannot_use_old_hash(self):
        report = self.prepare()
        data = bytearray(self.iso.read_bytes())
        data[-1] = 42
        self.iso.write_bytes(data)
        with self.assertRaises(intake.PreparationError):
            self.prepare(expected_sha256=report["sha256"])

    def test_concurrent_file_change_rejected(self):
        actual = intake.fingerprint
        def change_while_hashing(path):
            digest = actual(path)
            path.write_bytes(path.read_bytes() + b"changed")
            return digest
        with patch.object(intake, "fingerprint", change_while_hashing):
            with self.assertRaisesRegex(intake.PreparationError, "changed during"):
                self.prepare()

    def test_cli_with_paths_containing_spaces(self):
        repository = self.root / "Repo with spaces"
        (repository / "tools").mkdir(parents=True)
        shutil.copy2(ROOT / "tools/prepare_region.py", repository / "tools")
        shutil.copy2(ROOT / "projects.json", repository)
        result = subprocess.run([sys.executable, str(repository / "tools/prepare_region.py"),
                                 self.project["id"], "--iso", str(self.iso),
                                 "--inspector", self.inspector[0]],
                                text=True, capture_output=True, check=False)
        self.assertEqual(result.returncode, 0, result.stderr + result.stdout)
        self.assertTrue(json.loads(result.stdout)["input_prepared"])
        self.assertNotIn(str(self.root), result.stdout)
        self.assertTrue(list((repository / "private-inputs").rglob("input.json")))

    def test_cli_reports_failure_with_nonzero_exit(self):
        with patch.object(sys, "argv", ["prepare_region.py", self.project["id"],
                                        "--iso", str(self.iso), "--region", "PAL",
                                        "--inspector", self.inspector[0]]):
            output = io.StringIO()
            with contextlib.redirect_stdout(output):
                self.assertEqual(intake.main(), 2)
            self.assertFalse(json.loads(output.getvalue())["input_prepared"])


if __name__ == "__main__":
    unittest.main()
