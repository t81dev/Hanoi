# Makefile for HanoiVM + Axion Ecosystem + IR Emitter

# ========== Source Definitions ==========
CWEB_SOURCES = \
	hvm_loader.cweb \
	disassembler.cweb \
	hanoivm_vm.cweb \
	main_driver.cweb \
	write_simple_add.cweb \
	t81_test_suite.cweb \
	advanced_ops.cweb \
	t243bigint.cweb \
	t729tensor_ops.cweb \
	axion_ai.cweb \
	project_looking_glass.cweb \
	t81_ir_emitter.cweb

C_OBJS = $(CWEB_SOURCES:.cweb=.c)
H_OBJS = $(CWEB_SOURCES:.cweb=.h)

# ========== Default Build ==========
all: hanoivm write_simple_add t81_test_suite ir_emitter

# ========== CWEB Processing ==========
%.c %.h: %.cweb
	cweave $<
	ctangle $<

# ========== Core Executables ==========
hanoivm: hvm_loader.c disassembler.c hanoivm_vm.c main_driver.c advanced_ops.c t243bigint.c t729tensor_ops.c axion_ai.c
	gcc -o hanoivm $^ -Wall -Wextra -O2

write_simple_add: write_simple_add.c
	gcc -o $@ $< -Wall -Wextra -O2

t81_test_suite: t81_test_suite.c
	gcc -o $@ $< -Wall -Wextra -O2

ir_emitter: t81_ir_emitter.c
	gcc -o $@ $< -Wall -Wextra -O2

# ========== Run Targets ==========
run: write_simple_add hanoivm
	./write_simple_add
	./hanoivm simple_add.hvm --disasm

run-test-suite: t81_test_suite
	./t81_test_suite

run-ir-emitter: ir_emitter
	./ir_emitter

ghidra-test: t81_test_suite
	./t81_test_suite
	ghidra_disasm_plugin test_all_types.hvm > ghidra_output.log
	@echo "[✓] Disassembled test_all_types.hvm to ghidra_output.log"

# ========== All Validation ==========
check: all run-test-suite ghidra-test run-ir-emitter
	@echo "[✓] Full build + test + disasm + IR emitter check completed"

# ========== Clean ==========
clean:
	rm -f *.c *.h *.tex *.log *.scn *.dvi *.pdf *.o \
	      hanoivm write_simple_add t81_test_suite ir_emitter \
	      simple_add.hvm test_all_types.hvm mock_t81_module.ll \
	      ghidra_output.log

.PHONY: all clean run run-test-suite modules ghidra-test check run-ir-emitter
