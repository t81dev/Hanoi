# 🧬 Compiler Pipeline — T81Lang → TISC

This document describes the flow from T81Lang source to TISC binary code, through lexical, parsing, and IR generation phases.

## Key Files
- `t81lang_lexer.cweb`
- `t81lang_parser.cweb`
- `t81lang_irgen.cweb`
- `tisc_backend.cweb`

## Stages
1. Tokenization with ternary language primitives
2. AST parsing
3. Intermediate Representation (IR)
4. Compilation into TISC for VM execution
