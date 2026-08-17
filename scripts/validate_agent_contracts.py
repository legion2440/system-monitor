#!/usr/bin/env python3
from __future__ import annotations

import json
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
INDEX_PATH = ROOT / "agent/module-index.json"
GRAPH_PATH = ROOT / "agent/dependency-graph.json"
MODULE_DIR = ROOT / "agent/modules"
ERRORS: list[str] = []
WARNINGS: list[str] = []

def load_json(path: Path):
    try:
        return json.loads(path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as exc:
        ERRORS.append(f"{path.relative_to(ROOT)}: {exc}")
        return {}

def exists(rel: str) -> bool:
    return (ROOT / rel).exists()

def check_budget(path: Path, target: int, warning: int) -> None:
    try: size = path.stat().st_size
    except OSError: return
    if size > warning: WARNINGS.append(f"{path.relative_to(ROOT)} is {size} bytes; review navigation budget (warning > {warning})")
    elif size > target: WARNINGS.append(f"{path.relative_to(ROOT)} is {size} bytes; above target {target}")

def main() -> int:
    index = load_json(INDEX_PATH)
    graph = load_json(GRAPH_PATH)
    modules = index.get("modules", {})
    known = set(modules)
    check_budget(INDEX_PATH, 8 * 1024, 12 * 1024)
    declared_manifests: set[Path] = set()
    for module_id, spec in modules.items():
        manifest_rel = spec.get("manifest", "")
        manifest_path = ROOT / manifest_rel
        declared_manifests.add(manifest_path.resolve())
        manifest = load_json(manifest_path)
        if manifest.get("module_id") != module_id: ERRORS.append(f"{manifest_rel}: module_id does not match index key {module_id}")
        check_budget(manifest_path, 6 * 1024, 10 * 1024)
        for dependency in spec.get("dependencies", []):
            if dependency not in known: ERRORS.append(f"{module_id}: unknown dependency {dependency}")
        if spec.get("status") == "implemented":
            for rel in spec.get("roots", []):
                if not exists(rel): ERRORS.append(f"{module_id}: implemented root missing: {rel}")
            for rel in spec.get("entrypoints", []):
                if not exists(rel): ERRORS.append(f"{module_id}: implemented entrypoint missing: {rel}")
        for path_spec in manifest.get("paths", []):
            rel = path_spec.get("path", "")
            lifecycle = path_spec.get("lifecycle")
            provenance = path_spec.get("provenance")
            present = exists(rel)
            if lifecycle == "planned" and present: ERRORS.append(f"{module_id}: planned path exists: {rel}")
            if lifecycle in {"implemented", "deprecated"} and not present: ERRORS.append(f"{module_id}: {lifecycle} path missing: {rel}")
            if provenance == "generated":
                if not path_spec.get("generator") or not path_spec.get("check"): ERRORS.append(f"{module_id}: generated path lacks generator/check: {rel}")
                if lifecycle == "implemented" and path_spec.get("generator") and not exists(path_spec["generator"]): ERRORS.append(f"{module_id}: generator missing: {path_spec['generator']}")
            if lifecycle == "deprecated" and "legacy_consumers" not in path_spec: ERRORS.append(f"{module_id}: deprecated path lacks legacy_consumers: {rel}")
    actual_manifests = {p.resolve() for p in MODULE_DIR.glob("*.json")}
    for orphan in sorted(actual_manifests - declared_manifests): ERRORS.append(f"orphan manifest: {orphan.relative_to(ROOT)}")
    for missing in sorted(declared_manifests - actual_manifests): ERRORS.append(f"manifest declared by index is missing: {missing.relative_to(ROOT)}")
    index_edges = {(src, dst) for src, spec in modules.items() for dst in spec.get("dependencies", [])}
    graph_edges = set()
    for edge in graph.get("edges", []):
        if not isinstance(edge, list) or len(edge) != 2:
            ERRORS.append(f"invalid dependency edge: {edge!r}"); continue
        src, dst = edge; graph_edges.add((src, dst))
        if src not in known or dst not in known: ERRORS.append(f"dependency graph references unknown module: {src} -> {dst}")
    if index_edges != graph_edges:
        missing = index_edges - graph_edges; extra = graph_edges - index_edges
        if missing: ERRORS.append(f"dependency graph missing edges: {sorted(missing)}")
        if extra: ERRORS.append(f"dependency graph has extra edges: {sorted(extra)}")
    for warning in WARNINGS: print(f"[WARN] {warning}")
    for error in ERRORS: print(f"[ERROR] {error}")
    if ERRORS:
        print(f"[FAIL] Agent contract validation found {len(ERRORS)} issue(s).")
        return 1
    print("[OK] Agent index, manifests, paths and dependency graph are consistent.")
    return 0

if __name__ == "__main__": sys.exit(main())
