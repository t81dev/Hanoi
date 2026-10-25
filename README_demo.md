# HanoiVM Public Demo: NLP → TISC → Ternary Stack

## Overview
HanoiVM demonstrates a ternary co-processor for AI, translating natural language queries into **TISC** (Ternary Instruction Set Computing) bytecode and executing them on a recursive ternary stack. This demo evaluates a ternary decision tree for symbolic AI reasoning, showcasing entropy-aware computation.

## Demo Task
- **Input**: NLP query (e.g., “Which path reduces entropy given these conditions?”).
- **Process**:
  1. `AxionCLI` parses NLP input via JSON.
  2. `TISCQueryCompiler` generates ternary bytecode (T81/T243 tiers).
  3. `HanoiVM` executes the bytecode, evaluating a ternary decision tree.
  4. **Output**: Ranked decision paths, entropy heatmap, and execution trace.
- **Visualization**:
  - CLI-based trit stack view.
  - Entropy heatmap (via Plotly).
  - Optional 3D trace (Three.js or Looking Glass, future).

## Setup
```bash
git clone https://github.com/t81dev/hanoi.git
cd hanoi
make axionctl
./axionctl --demo examples/nlp_decision_tree/demo.json
