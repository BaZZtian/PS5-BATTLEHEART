import copy
import importlib.util
import json
import tempfile
import unittest
from pathlib import Path

spec = importlib.util.spec_from_file_location("release_gate", Path(__file__).parents[1] / "tools/release_gate.py")
gate = importlib.util.module_from_spec(spec)
spec.loader.exec_module(gate)


class ReleaseGateTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        (self.root / "sce_sys").mkdir()
        (self.root / "eboot.bin").write_bytes(b"synthetic test executable, not a game")
        (self.root / "sce_sys/param.json").write_text(json.dumps({"contentVersion": "01.000.000"}))
        self.project = json.loads((Path(__file__).parents[1] / "projects.json").read_text(encoding="utf-8"))["projects"][0]
        files = {p.relative_to(self.root).as_posix(): {"sha256": gate.sha256(p), "origin": "own_code", "license": "test-only"}
                 for p in self.root.rglob("*") if p.is_file()}
        build = gate.build_id({name: record["sha256"] for name, record in files.items()})
        self.evidence = {"files": files, "regional_tests": {
            region: {"status": "passed", "disc_id": disc, "firmware": "12.40", "build_id": build,
                     "disc_revision": 0, "disc_sha256": "a" * 64, "test_kind": "ps5-gameplay",
                     "tested_at": "synthetic-test", "evidence_reference": "synthetic-test",
                     "checks": {check: True for check in gate.BASE_CHECKS | {"chao_battle", "multiplayer"}}}
            for region, disc in self.project["disc_ids"].items()}}

    def check(self, evidence=None):
        return gate.check_release(self.root, self.evidence if evidence is None else evidence, self.project)[0]

    def test_complete_matching_evidence(self):
        self.assertEqual(self.check(), [])

    def test_pal_is_mandatory(self):
        del self.evidence["regional_tests"]["PAL"]
        self.assertTrue(any("PAL" in p for p in self.check()))

    def test_build_change_invalidates_tests(self):
        (self.root / "eboot.bin").write_bytes(b"different synthetic executable")
        problems = self.check()
        self.assertTrue(any("differs" in p for p in problems))
        self.assertTrue(any("another build" in p for p in problems))

    def test_unlisted_file_blocks_release(self):
        (self.root / "unexpected.txt").write_text("synthetic")
        self.assertTrue(any("Unlisted" in p for p in self.check()))

    def test_game_image_cannot_be_manifested_as_own_code(self):
        path = self.root / "original.ISO"
        path.write_bytes(b"not actually an ISO")
        self.evidence["files"][path.name] = {"sha256": gate.sha256(path), "origin": "own_code", "license": "test-only"}
        self.assertTrue(any("forbidden" in p for p in self.check()))

    def test_traversal_rejected(self):
        self.evidence["files"]["../outside.bin"] = {}
        self.assertTrue(any("Invalid package path" in p for p in self.check()))

    def test_missing_required_gameplay_checks_block_release(self):
        checks = (["chao_battle", "multiplayer"] if self.project["id"] == "sonic-adventure-2-battle"
                  else ["audio", "save_load"])
        for region, check in zip(["PAL", "NTSC-U"], checks):
            self.evidence["regional_tests"][region]["checks"].pop(check)
        problems = self.check()
        for check in checks:
            self.assertTrue(any(check in p for p in problems))

    def test_missing_content_version_blocks_updates(self):
        (self.root / "sce_sys/param.json").write_text("{}")
        self.assertTrue(any("contentVersion" in p for p in self.check()))

    def test_game_data_provenance_rejected(self):
        evidence = copy.deepcopy(self.evidence)
        evidence["files"]["eboot.bin"]["origin"] = "original_game"
        self.assertTrue(any("origin" in p for p in self.check(evidence)))

    def test_empty_manifest_blocks_release(self):
        self.assertTrue(self.check({}))

    def test_structure_check_cannot_replace_gameplay(self):
        self.evidence["regional_tests"]["NTSC-U"]["test_kind"] = "disc-structure"
        self.assertTrue(any("not a PS5 gameplay" in p for p in self.check()))

    def test_exact_disc_fingerprint_required(self):
        self.evidence["regional_tests"]["NTSC-U"].pop("disc_sha256")
        self.evidence["regional_tests"]["PAL"]["disc_revision"] = "unknown"
        problems = self.check()
        self.assertEqual(sum("precise disc revision" in p for p in problems), 2)


if __name__ == "__main__":
    unittest.main()
