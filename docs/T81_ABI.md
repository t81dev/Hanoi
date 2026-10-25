# T81_ABI.md — Application Binary Interface for the T81 Architecture

## 📌 Purpose
This document defines the calling convention, register usage, stack alignment, and memory layout for binaries compiled to the **T81 ternary instruction set** (`.t81asm`) and executed on the **HanoiVM**.

It enables interoperability between compilers, libraries, and runtime systems using the `t81-unknown-none` LLVM target triple.

---

## 🔧 Calling Convention

### 🎯 Function Argument Passing
| Argument Index | Register        |
|----------------|-----------------|
| Arg 0          | `R1`            |
| Arg 1          | `R2`            |
| Arg 2          | `R3`            |
| Arg 3          | `R4`            |
| Arg 4          | `R5`            |
| Arg 5          | `R6`            |
| Arg 6          | `R7`            |
| Arg 7          | `R8`            |
| >8 arguments   | Ternary Stack   |

- Registers `R1`–`R8` are used for the first 8 arguments.
- Remaining arguments are pushed right-to-left onto the ternary stack, aligned to 81-bit boundaries.

---

### 🔁 Return Value Convention
- The primary return value is placed in **`R0`**.
- Secondary return values (if any) spill to `R9`, `R10`, etc., or are returned via memory references passed in `R1`.

---

### 🧮 Stack and Frame Layout

- **Ternary Stack Alignment**: All frames aligned to **81 bits** (1 trit-word).
- **Frame Pointer**: `R80` is reserved as a logical frame pointer (`FP`).
- **Stack Pointer**: `R79` is used as a ternary stack pointer (`SP`).
- **Return Address**: Pushed to the stack prior to function body entry.

---

## 🧠 Register Usage Convention

| Register | Usage                    |
|----------|--------------------------|
| `R0`     | Return value             |
| `R1`–`R8`| Arguments                |
| `R9`–`R63`| Temporary/Caller-saved |
| `R64`–`R78`| Callee-saved (preserved across calls) |
| `R79`    | Stack Pointer (`SP`)     |
| `R80`    | Frame Pointer (`FP`)     |

---

## 🔁 Ternary Stack Growth
- The stack grows **"downward"** in ternary space: from high-index to low-index.
- Each frame contains:
  - Return address (1 trit-word)
  - Old `FP`
  - Saved callee registers
  - Local variables
  - Spilled arguments

---

## 🧬 Data Types & Alignment

| Type         | Size (bits) | Alignment (bits) |
|--------------|-------------|------------------|
| `i27`        | 27          | 27               |
| `i81`        | 81          | 81               |
| `t243_bigint`| Variable    | 81               |
| `t729_tensor`| Variable    | 243              |

- All types are aligned to ternary-compatible sizes.
- Use `i81` as the standard scalar unit.

---

## 🧪 Varargs and Ellipsis (`...`)
- Variadic functions store additional arguments on the ternary stack after `R8`.
- A frame offset pointer (passed in `R2`) is used to access the start of the varargs list.

---

## 🔍 Exception Handling (Planned)
- A reserved opcode (`TRAISE`) will trigger unwind.
- Exception frames will track SP, FP, and RA.
- Support for ternary symbolic unwind handlers (`TUNWIND`) is under consideration.

---

## 💬 Remarks
- This ABI is designed to support symbolic, recursive, and AI-assisted computation.
- `T81` is register-rich to support functional-style recursion with minimal stack mutation.
- Integration with entropy tags (`!entropy`) and symbolic dispatch is optional but recommended.

---

## 📎 Related Docs
- See [T81Backend.md](./T81Backend.md) for backend architecture.
- See [HanoiVM Project](https://github.com/t81dev/hanoi) for runtime execution.

---
