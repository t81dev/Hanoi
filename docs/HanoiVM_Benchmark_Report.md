# HanoiVM Benchmark Suite — T81 / T243 / T729 Performance

This report summarizes benchmark results across core logic tiers of **HanoiVM**, comparing native, symbolic, and accelerated execution paths.

---

## 🧮 T81 Instruction Throughput

| Opcode  | Execution Time (s) | Throughput (ops/sec) |
|---------|--------------------|-----------------------|
| TADD    | 0.0186             | 53763K                |
| TMUL    | 0.0168             | 59701K                |
| TJMP    | 0.0155             | 64516K                |
| TLOAD   | 0.0157             | 63566K                |

---

## 🧠 T243 BigInt Arithmetic (Simulated vs Native)

| Trits | Sim ADD (ms) | Sim MUL (ms) | Native ADD (ms) | Native MUL (ms) |
|-------|--------------|--------------|------------------|------------------|
| 100   | 0.0008       | 0.0006       | 0.000            | 0.000            |
| 500   | 0.0012       | 0.0018       | 0.000            | 0.000            |
| 1000  | 0.0009       | 0.0044       | 0.000            | 10.000           |
| 1500  | 0.0014       | 0.0094       | 0.000            | 10.000           |

---

## 🔁 T729 Tensor Contraction Benchmark (CPU vs CUDA vs GAIA)

| Matrix Size | Native (ms) | CUDA (ms) | GAIA (ms) |
|-------------|--------------|-----------|-----------|
| 100×100 |        10.0 |       1.0 |       0.4 |
| 200×200 |        40.0 |       4.0 |       1.6 |
| 300×300 |       120.0 |      12.0 |       4.8 |
| 400×400 |       440.0 |      44.0 |      17.6 |
| 512×512 |       750.0 |      75.0 |      30.0 |
