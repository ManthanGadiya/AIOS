# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added

- Project definition and documentation baseline:
  - PRD and subsystem specifications (docs/01 … docs/15)
  - ROADMAP with phased implementation plan
- Governance and collaboration documents:
  - AGENTS.md — development-agent workflow and project rules
  - TEAMMATES.md — team and collaboration tracker
  - STATUS.md — living project status
  - CONTRIBUTING.md — contribution guidelines
  - CHANGELOG.md — this file
- Canonical repository structure:
  - backend/ — C++ simulation engine (source of truth)
  - frontend/ — React dashboard
  - tests/ — unit and integration tests
- MIT license (Copyright 2026 Manthan Gadiya)

### Fixed

- Canonical documentation filenames confirmed as plural (`AGENTS.md`, `14_Demo_Scenarios.md`); stale references removed.
- Repository structure trees in docs/PRD.md and docs/ROADMAP.md aligned with the actual repository.
