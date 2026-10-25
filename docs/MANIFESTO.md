# 🧠 HanoiVM Manifesto — Recursive Symbolic Virtual Machine

This document declares the philosophy, architecture, and structure of the **HanoiVM** project—a recursive ternary virtual machine driven by symbolic computation, AI introspection, and modular ternary logic tiers (`T81`, `T243`, `T729`).

Built from the ground up using `.cweb` literate programming, **HanoiVM** is more than a runtime; it embodies an evolving philosophy of computation, rooted in recursion, enhanced by artificial intelligence, and capable of symbolic reasoning at the instruction level.

---

## 📜 Philosophy

At its foundation, HanoiVM explores:

* 🌐 **Recursion as architecture**, not merely algorithm
* 🧠 **Symbolism as instruction**, transcending data representation
* 🔺 **Ternary logic** as a naturally expressive AI system

Its tiered recursive execution model includes:

* **`T81`** — Efficient `uint81_t` operand logic
* **`T243`** — Stateful symbolic FSMs and BigInts
* **`T729`** — AI-driven tensor logic and recursive meta-opcodes

---

## 🔗 Synergy & Intent

* **TISC**: Ternary Instruction Set Compiler — symbolic IR to `.hvm`
* **Axion AI**: NLP/entropy-driven optimization and rollback layer
* **Pattern Engines**: Symbolic dispatch via `t81_patterns.cweb`
* **GPU/PCIe Dispatch**: CUDA, ROCm, and PCIe co-processing
* **Literate Infrastructure**: All modules built using `.cweb`

---

## 🧠 T81Lang Compiler Stack

* REPL-driven symbolic ternary language
* Compiles `.t81` into `.hvm` via multi-phase IR pipeline
* Integrated with Axion AI for entropy-based tier switching
* REPL, test harness, and compiler pipeline are now complete

---

## 📁 File Index

This is the updated file list with integrated `.cweb` modules for HanoiVM v0.9.2+:

| File                            | Description                                 |
| ------------------------------- | ------------------------------------------- |
| `README.md`                     | Project overview and usage                  |
| `MANIFESTO.md`                  | Project philosophy and file index           |
| `ROADMAP.md`                    | Phase-based progress tracker                |
| `CHANGELOG.md`                  | Version history and features                |
| `config.cweb`                   | Global settings and runtime toggles         |
| `entropy_monitor.cweb`          | Timer-based entropy alert system            |
| `symbolic_trace.cweb`           | Unified symbolic event reporting            |
| `recursion_exporter.cweb`       | JSON-export of recursion stack frames       |
| `logviewer.cweb`                | DebugFS log output for telemetry            |
| `nlp_query_bridge.cweb`         | NLP string to symbolic opcode bridge        |
| `stack_snapshot.cweb`           | Real-time ternary stack state export        |
| `axion-ai.cweb`                 | Kernel-layer rollback + AI execution hooks  |
| `advanced_ops.cweb`             | T81 symbolic operation extensions           |
| `advanced_ops_ext.cweb`         | T243/T729 symbolic op extensions (FSM, FFT) |
| `t81_patterns.cweb`             | Symbolic logic pattern template library     |
| `t81_stack.cweb`                | Low-level stack operations                  |
| `t81_test_suite.cweb`           | Unit tests for core ternary operations      |
| `t81_to_hvm.cweb`               | T81Lang to `.hvm` bytecode converter        |
| `t81asm.cweb`                   | Assembler for `.t81` source code            |
| `t81lang.cweb`                  | Language grammar and structure              |
| `t81lang_lexer.cweb`            | Lexer component for parsing                 |
| `t81lang_parser.cweb`           | AST and parsing logic                       |
| `t81lang_irgen.cweb`            | IR builder from parsed AST                  |
| `t81lang_compiler.cweb`         | Compiler orchestrator                       |
| `tisc_query_compiler.cweb`      | Query-to-TISC compiler module               |
| `tisc_backend.cweb`             | TISC IR → `.hvm` compiler backend           |
| `tisc_ir.cweb`                  | TISC IR data model                          |
| `tisc_stdlib.cweb`              | Base macros for TISC                        |
| `cuda_handle_request.cweb`      | NVIDIA CUDA symbolic tensor backend         |
| `gaia_handle_request.cweb`      | AMD ROCm symbolic tensor backend            |
| `axion-gaia-interface.cweb`     | AI symbolic interaction to ROCm             |
| `disassembler.cweb`             | Human-readable opcode decoding              |
| `recursive_tier_execution.cweb` | Demo: `T81` → `T243` → `T729` promotion     |
| `test_advanced_hvm.cweb`        | Symbolic recursion test suite               |
| `test_controlflow_hvm.cweb`     | Control flow opcode tests                   |
| `nist_encryption.cweb`          | AES/RSA support for secure ternary channels |
| `libt81.cweb`                   | T81 integer operations                      |
| `libt243.cweb`                  | BigInt support for T243 tier                |
| `libt729.cweb`                  | Tensor math for symbolic tier               |
| `simple_add.cweb`               | Minimal ternary example program             |
| `write_simple_add.cweb`         | Programmatic example generator              |
| `telemetry-cli.cweb`            | CLI interface for runtime introspection     |

---

## ⚠️ Final Word

**Recursion isn't merely a technique—it's a worldview.**
By combining symbolic intent, ternary logic, entropy awareness, and AI-assisted introspection, we believe HanoiVM sets a foundation for the next evolution in computation.

May this stack lead you further into recursion, safely.

---
