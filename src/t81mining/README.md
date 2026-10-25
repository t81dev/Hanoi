# T81 Mining Coprocessor Project

Welcome to the **T81 Mining Coprocessor** — a modular, multi-core SHA3-driven architecture built on ternary logic and stack-based execution. This project simulates and implements a novel mining pipeline based on T81 ternary instructions, with full Verilog support and `.cweb` literate documentation.

---

## 📌 Project Features

- **Ternary FSM Core (`hanoivm_fsm.v`)**
  - T81 opcode execution (`NOP`, `PUSH`, `POP`, `ADD`)
  - 81-bit ternary operand and stack support

- **SHA3 Preprocessing (`t81_preprocessor.v`)**
  - Converts T81 data into 512-bit SHA3-friendly format
  - Supports entropy padding and future checksum insertion

- **Stub SHA3 Core (`sha3_core.v`)**
  - Simulated hashing logic for testbench/debugging
  - Placeholder for Keccak or real SHA3 integration

- **Mining Validator (`mining_controller.v`)**
  - Difficulty-based hash comparison
  - Signals `match_found` per core

- **Top-Level Mining Pipeline (`mining_pipeline.v`)**
  - FSM → Preprocessor → SHA → Validator
  - Parameterized for single or multi-core operation

- **Multi-Core Version (`mining_pipeline_multicore.v`)**
  - Dynamic core instancing with `NUM_CORES`
  - Parallel hashing and arbitration-ready architecture

- **Full Literate Testbench (`t81_mining_testbench.cweb`)**
  - `.cweb`-documented validation suite
  - Supports opcode testing, nonce iteration, dual-core tracing
  - Verilator/GTKWave compatible

---

## 📁 Directory Structure

```plaintext
├── hanoivm_fsm.v               # Ternary opcode FSM
├── t81_preprocessor.v          # T81 → SHA3 input
├── sha3_core.v                 # Stub SHA3 hash engine
├── mining_controller.v         # Mining validator
├── mining_pipeline.v           # Single-core pipeline
├── mining_pipeline_multicore.v # Multi-core variant
├── t81_mining_testbench.cweb   # Literate test suite
├── tb_mining_pipeline.v        # Verilog-extracted testbench
└── README.md                   # Project overview
```

---

## 🔧 How to Simulate

### Icarus Verilog + GTKWave
```bash
iverilog -o tb_pipeline \
  tb_mining_pipeline.v \
  mining_pipeline.v \
  hanoivm_fsm.v \
  t81_preprocessor.v \
  sha3_core.v \
  mining_controller.v

vvp tb_pipeline

gtkwave tb_mining_pipeline.vcd
```

### Verilator (Advanced)
Use `mining_pipeline_multicore.v` with Verilator + C++ driver for real-time simulation.

---

## 📚 Future Directions

- PCIe/AXI-Lite MMIO bus integration
- Opcode expansion (`SUB`, `CMP`, `MUL`, etc.)
- Real Keccak-f[1600] or SHA3-512 core integration
- Hardware synthesis (FPGA)
- Adaptive AI-powered opcode control via **Axion**

---

## 🔖 License
Copyleft (✲) 2025 — Designed with recursion, by recursion, for recursive minds.

---

For questions, forks, or collabs, please reach out or contribute via [GitHub](https://github.com/t81dev/hanoi).

_"Where binary halts, ternary begins."_
