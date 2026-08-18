# Contributing to AIOS

Thank you for contributing to AIOS — an educational C++-based multiprogramming operating-system simulator with a React visualization dashboard.

Please read `AGENTS.md` first; it is the authoritative development-agent workflow for this repository.

---

# 1. Project at a Glance

* The C++ simulation engine is the source of truth.
* The React dashboard is the presentation and control layer.
* AI agents are simulated OS-managed workloads — the AI layer must not bypass core OS mechanisms.

---

# 2. Repository Structure

```text
AIOS/
├── README.md                  ← entry point / quick start
├── LICENSE
├── AGENTS.md                  ← development-agent workflow and project rules
├── TEAMMATES.md               ← team and collaboration tracker
├── STATUS.md                  ← living project status
├── CHANGELOG.md
├── CONTRIBUTING.md
├── docs/                      ← product and subsystem specifications
├── backend/                   ← C++ simulation engine (source of truth)
├── frontend/                  ← React dashboard
└── tests/                     ← unit and integration tests
```

---

# 3. Documentation First

Before implementing a subsystem:

1. Search `docs/` for the existing specification.
2. Read the relevant sections.
3. Inspect related code.
4. Implement according to the specification.

If documentation conflicts with implementation, identify and explain the conflict rather than silently choosing one.

---

# 4. Development Workflow

Every meaningful task follows:

```text
INSPECT
  ↓
UNDERSTAND
  ↓
PLAN
  ↓
IMPLEMENT
  ↓
TEST
  ↓
DOCUMENT
  ↓
UPDATE STATUS
  ↓
UPDATE TEAMMATES
  ↓
UPDATE README CURRENT STATUS
  ↓
COMMIT
```

Never skip inspection, and never implement first and investigate later.

---

# 5. Commit Conventions

* Commit after every meaningful, coherent change.
* Prefer one logical change per commit.
* Use conventional prefixes, for example:

```text
feat(cpu): implement instruction execution
feat(process): add PCB lifecycle
feat(memory): implement page table
feat(scheduler): add round robin policy
feat(ai): add simulated agent manager
feat(gui): add process dashboard
test(memory): add page fault tests
fix(ipc): correct blocked receiver wakeup
docs: add project status tracking
```

* Avoid vague messages such as `update`, `changes`, `final`, `done`.

---

# 6. Testing Policy

Before declaring a feature complete:

```text
Compile
 ↓
Unit tests
 ↓
Integration tests
 ↓
Relevant scenario
 ↓
Full regression when appropriate
```

Never mark a feature complete based only on compilation.

---

# 7. Scope Boundaries

Do not add real LLM scheduling, real hardware control, real autonomous agents, GPU scheduling, distributed OS features, production security, or real hardware drivers unless the project owner explicitly changes the scope.

---

# 8. Documentation Updates

After a meaningful change, update:

* `STATUS.md` — living project status
* `TEAMMATES.md` — team/collaboration tracking (if ownership or dependencies change)
* `README.md` — only the `## Current Status` / `### Quick Start` sections
