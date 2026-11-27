# GCC Repository Build-Readiness Audit

**Repository:** `auraecosystem/Gcc`  
**Audited branch:** `main`  
**Audit date:** 2026-11-27  
**Scope:** repository structure, build scripts, Autotools/Autotest wiring, and GCC bootstrap configuration.

## Executive finding

The current `main` tree is **not yet a verifiable, buildable GCC source checkout**. It contains GCC-related notes and build snippets, a small Bison/Autotest calculator example, and a very sparse set of GCC-named paths. The recursive tree previously inspected contained 42 entries and was not truncated; several GCC-looking implementation files were one byte. The repository metadata reports a much larger stored size, but that does not establish that a usable GCC source tree is present in the checked-out tree.

A GCC bootstrap cannot be validated or responsibly launched until the repository contains a complete, internally consistent compiler source tree. This audit therefore records verified blockers rather than pretending that a GCC build succeeded.

## Verified findings

| Severity | File or area | Finding | Consequence |
| --- | --- | --- | --- |
| Blocker | Repository tree | No verifiable complete GCC source/configure tree is present in the inspected 42-entry tree | GCC configure/build cannot be run from this checkout |
| High | `Bash.sh` | Attempts to clone another GCC source repository and assumes an `original-gcc` reference checkout | Violates the local-source-only boundary and is not reproducible in this repository |
| High | `build.bison` | Runs `../configure` from a newly created `build/` directory | The referenced configure script is not present as a generated or checked-in root file |
| High | `Bootstrap.sh` | Generates Autotest from `tests/testsuite.at`, but the tree contains `test/Testsuite.at` | Bootstrap references a missing path |
| High | `test/Testsuite.at` | Includes `calc.at` and `bison-tests.at`, while the visible files are `Calc.at` and `Bison.at` | Case-sensitive filesystems cannot resolve those includes |
| High | `Makefile.am` | Distributes paths under `tests/` and `src/calc.y`; visible tree instead has `test/` and `src/Calc.y` | Distribution metadata references missing paths |
| High | `Makefile.am` | Adds `testsuite` to `TESTS` and invokes it again in `check-local` | Test runner may execute twice |
| High | `Run.sh` | Calls lowercase `bootstrap.sh`, while the tracked file is `Bootstrap.sh` | Fails on case-sensitive filesystems |
| High | `gcc/config.sh` | Uses `--disable-bootstrap` | Contradicts a bootstrap build objective |
| Medium | `.env` | Contains shell `export` syntax and a `make` command | It is not a clean dotenv-only configuration file |
| Medium | `build.sh` | Runs `make -j$(nproc)` without checking the current directory, configuration, or available command | Can fail unclearly or use an unintended build directory |
| Medium | `Test.at` | Calls Bison and compilation on `input.y`/ `input.c`, but does not create the grammar in that test | The test is not self-contained |
| Medium | `test/Calc.at` | The multiplication-labelled case runs `3+4` and expects `7`; it does not test multiplication | Test name and behavior disagree |
| Medium | `Verify.sh` | Assumes installation under `$HOME/gcc-install` without checking executable presence | Verification errors are not diagnostic |

## Build-system classification

The checked-in `configure.ac` identifies the package as `bison-autotest-example` and configures a small Autotools project. It is not the configure script for a full GCC source tree.

The following files are part of the Bison/Autotest example layer:

- `autom4te.m4`
- `setup.m4`
- `__init__.bison`
- `test/Testsuite.at`
- `test/Calc.at`
- `test/Bison.at`
- `configure.ac`
- `Makefile.am`

They should be treated as a standalone parser/test harness unless and until the repository's intended architecture says otherwise. They must not be presented as the GCC compiler implementation.

## GCC bootstrap configuration guidance

The documented `BUILD_CONFIG` values are GCC build configurations, not generic Autotools switches. They only have meaning when passed to a real GCC configure script from a complete GCC source tree.

Use one focused profile at a time during initial validation. For example:

```sh
BUILD_CONFIG=bootstrap-Og
```

For a diagnostic build, choose a single relevant debug or sanitizer profile supported by the target host. Do not combine every listed profile into one giant configuration string as a default. Validate the exact configuration names against the GCC version and host platform represented by the source tree.

Also keep these concepts separate:

- `BUILD_CONFIG` selects GCC's build configuration.
- `--enable-languages=c,c++` selects compiler front ends.
- `--disable-multilib` changes multilib support.
- `--prefix=...` selects the install destination.
- `--disable-bootstrap` disables the bootstrap process and is therefore unsuitable when the goal is to perform a bootstrap build.

## Safe repair order

1. **Establish source provenance inside this repository.** Identify the intended complete source tree and verify its tracked paths. Do not fetch, clone, mirror, or add an external source repository as an automatic repair.
2. **Separate the two build targets.** Decide whether the Bison/Autotest calculator is a standalone example or whether the repository is intended to contain GCC itself. Give each target its own explicit entrypoint and documentation.
3. **Repair the Autotest harness.** Normalize the `test/` path and filename case, make each test self-contained, and remove duplicate test-runner invocation.
4. **Repair shell entrypoints.** Use exact tracked filename case, strict shell settings, clear directory checks, and actionable errors.
5. **Make GCC configuration local-source-only.** Require an explicit local source directory containing GCC's configure script; never clone or fetch source as a side effect.
6. **Validate on a supported build host.** Run shell syntax checks, Autotools generation, parser tests, then GCC configure/build/bootstrap only after the full compiler source tree is verified.
7. **Record exact results.** Capture host OS/architecture, tool versions, configure arguments, build log, test summary, and install verification.

## Current validation status

- Repository metadata and tracked file contents: inspected.
- Build scripts and Autotools references: static review completed.
- GCC configure: **not run**; a complete GCC source tree is not verified.
- GCC bootstrap: **not run**.
- Bison/Autotest suite: **not run**; the current path/include inconsistencies must be corrected first.
- Files changed on `main`: none.
- External source repositories fetched or added: none.

## Exit criteria for a real GCC build

The project is ready for a GCC bootstrap attempt only when the selected local source directory contains, at minimum, a valid GCC `configure` entrypoint and the complete source/configuration subtrees expected by that GCC release, and the repository's scripts point to that exact local tree. A successful `make`, `make check`, and bootstrap completion must then be demonstrated by logs; a script existing in the repository is not proof of a successful build.
