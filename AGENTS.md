# Agent navigation

1. Read `agent/module-index.json`.
2. Identify the module that owns the requested feature using roots and entrypoints.
3. Read only `agent/modules/<module-id>.json` for that module.
4. Inspect the relevant production code/configuration before accepting metadata or README claims.
5. Read `docs/architecture.md` only when architectural context is required.
6. For cross-module changes, inspect `agent/dependency-graph.json` before editing contracts.
7. Run affected domain tests/validators after production changes when execution is requested or available.
8. Run `scripts/validate_agent_contracts.py` when agent metadata changes.
9. Do not read every manifest unless the task is repository-wide.
10. If ownership is ambiguous, inspect roots and the dependency graph, then open only candidate manifests; do not guess.
