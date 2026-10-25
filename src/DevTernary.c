/* DevTernary.c - VirtualBox PDM Device for Ternary Coprocessor */
#include <VBox/vmm/pdmdev.h>
#include <VBox/log.h>
#include <iprt/assert.h>
#include <iprt/string.h>
#include <iprt/mem.h>

/* MMIO Region Size and Register Offsets */
#define TERNARY_MMIO_SIZE 0x1000 // Expanded for multi-word operands
#define TERNARY_REG_INPUT_BASE 0x00 // Input trit array (81 trits, multiple words)
#define TERNARY_REG_OUTPUT_BASE 0x20 // Output trit array (81 trits)
#define TERNARY_REG_COMMAND 0x40 // Command register
#define TERNARY_REG_STATUS 0x44 // Status (e.g., error flags)
#define TERNARY_REG_OPERAND_COUNT 0x48 // Number of 32-bit words for input/output
#define TRIT_COUNT 81 // Match hanoivm_fsm.v (81 trits)
#define TRITS_PER_WORD 16 // 16 trits per uint32_t
#define WORDS_PER_OPERAND ((TRIT_COUNT + TRITS_PER_WORD - 1) / TRITS_PER_WORD) // 6 words
#define STACK_DEPTH 16 // Match hanoivm_fsm.v stack size (reduced from 729)

/* Ternary Commands (aligned with hanoivm_fsm.v) */
#define CMD_TERNARY_NOP 0x00 // No operation (added for alignment)
#define CMD_TERNARY_PUSH 0x01 // Push to ternary stack
#define CMD_TERNARY_POP 0x02 // Pop from ternary stack
#define CMD_TERNARY_ADD 0x03 // Ternary addition
#define CMD_TERNARY_NOT 0x04 // Ternary NOT
#define CMD_TERNARY_AND 0x05 // Ternary AND
#define CMD_TERNARY_SHA3 0x06 // SHA3 mining operation (software-only placeholder)

/* Ternary Data Type */
typedef int8_t TRIT; // -1, 0, +1 for balanced ternary
#define TRIT_MIN -1
#define TRIT_MAX 1

/* State Machine States (aligned with hanoivm_fsm.v) */
typedef enum {
    STATE_IDLE = 0,
    STATE_FETCH = 1,
    STATE_EXECUTE = 2,
    STATE_WRITEBACK = 3,
    STATE_DONE = 4
} TERNARY_STATE;

/* Device State */
typedef struct DEVTERNARY {
    TRIT input[TRIT_COUNT]; // Input trit array
    TRIT output[TRIT_COUNT]; // Output trit array
    TRIT stack[STACK_DEPTH][TRIT_COUNT]; // Ternary stack (16 x 81 trits)
    uint16_t stack_ptr; // Stack pointer (0 to STACK_DEPTH-1, reduced to uint16_t)
    uint32_t command; // Current command
    uint32_t status; // Status flags
    uint32_t operand_words; // Number of 32-bit words received
    uint32_t operand_count; // Expected number of words (fixed to 6 for 81 trits)
    TERNARY_STATE state; // Current state (added for FSM alignment)
    TRIT temp_result[TRIT_COUNT]; // Temporary result buffer (for EXECUTE state)
} DEVTERNARY, *PDEVTERNARY;

/* Status Flags */
#define STATUS_ERROR_INVALID_SIZE 0x1
#define STATUS_ERROR_INVALID_ADDR 0x2
#define STATUS_ERROR_STACK_OVERFLOW 0x4
#define STATUS_ERROR_STACK_UNDERFLOW 0x8
#define STATUS_ERROR_INVALID_COMMAND 0x10

/* Ternary Operations */
static TRIT ternary_add(TRIT a, TRIT b, TRIT *carry) {
    int sum = a + b;
    *carry = 0;
    if (sum < TRIT_MIN) { *carry = -1; return sum + 3; }
    if (sum > TRIT_MAX) { *carry = 1; return sum - 3; }
    return sum;
}

static TRIT ternary_not(TRIT a) {
    return -a;
}

static TRIT ternary_and(TRIT a, TRIT b) {
    return (a < b) ? a : b;
}

static void ternary_sha3(TRIT *input, TRIT *output, uint32_t count) {
    // Placeholder: Simulate SHA3 by scrambling trits
    // Note: Software-only, no equivalent in hanoivm_fsm.v; real SHA3 needs ternary Keccak
    for (uint32_t i = 0; i < count; i++) {
        output[i] = input[(i + 1) % count]; // Simple permutation
    }
}

/* MMIO Write Handler */
static DECLCALLBACK(void) ternary_mmio_write(PPDMDEVINS pDevIns, void *pvUser,
                                            RTGCPHYS addr, uint32_t value, unsigned size) {
    PDEVTERNARY pThis = (PDEVTERNARY)pvUser;
    if (size != sizeof(uint32_t)) {
        pThis->status |= STATUS_ERROR_INVALID_SIZE;
        LogRel(("TernaryCoproc: Invalid write size %u\n", size));
        return;
    }

    if (pThis->state != STATE_IDLE) {
        LogRel(("TernaryCoproc: Write ignored, not in IDLE state (%d)\n", pThis->state));
        return;
    }

    if (addr >= TERNARY_REG_INPUT_BASE && addr < TERNARY_REG_INPUT_BASE + WORDS_PER_OPERAND * 4) {
        // Decode uint32_t to 16 trits, truncate last word to 162 bits (81 trits)
        uint32_t word_idx = (addr - TERNARY_REG_INPUT_BASE) / 4;
        uint32_t trit_offset = word_idx * TRITS_PER_WORD;
        if (trit_offset < TRIT_COUNT) {
            uint32_t max_trits = (word_idx == WORDS_PER_OPERAND - 1) ? 
                                 (TRIT_COUNT - trit_offset) : TRITS_PER_WORD; // Limit last word
            for (int i = 0; i < max_trits && trit_offset + i < TRIT_COUNT; i++) {
                uint8_t bits = (value >> (i * 2)) & 0x3;
                pThis->input[trit_offset + i] = (bits == 2) ? -1 : (bits == 1) ? 1 : 0;
            }
            pThis->operand_words++;
            if (pThis->operand_words >= pThis->operand_count) {
                LogRel(("TernaryCoproc: INPUT received %u words\n", pThis->operand_words));
                pThis->operand_words = 0;
                pThis->state = STATE_FETCH; // Move to FETCH
            }
        }
    } else if (addr == TERNARY_REG_COMMAND) {
        pThis->command = value;
        pThis->status = 0;
        pThis->state = STATE_FETCH; // Move to FETCH for command execution
        LogRel(("TernaryCoproc: Command 0x%x received\n", value));
    } else if (addr == TERNARY_REG_OPERAND_COUNT) {
        // Fix to 6 words to match hanoivm_fsm.v's 81-trit operand
        pThis->operand_count = WORDS_PER_OPERAND; // Ignore value, force 6 words
        pThis->operand_words = 0;
        LogRel(("TernaryCoproc: Set operand count to %u words\n", pThis->operand_count));
    } else {
        pThis->status |= STATUS_ERROR_INVALID_ADDR;
        LogRel(("TernaryCoproc: Unknown MMIO write at 0x%x\n", addr));
    }
}

/* MMIO Read Handler */
static DECLCALLBACK(uint32_t) ternary_mmio_read(PPDMDEVINS pDevIns, void *pvUser,
                                               RTGCPHYS addr, unsigned size) {
    PDEVTERNARY pThis = (PDEVTERNARY)pvUser;
    if (size != sizeof(uint32_t)) {
        pThis->status |= STATUS_ERROR_INVALID_SIZE;
        LogRel(("TernaryCoproc: Invalid read size %u\n", size));
        return 0;
    }

    if (addr >= TERNARY_REG_OUTPUT_BASE && addr < TERNARY_REG_OUTPUT_BASE + WORDS_PER_OPERAND * 4) {
        uint32_t word_idx = (addr - TERNARY_REG_OUTPUT_BASE) / 4;
        uint32_t trit_offset = word_idx * TRITS_PER_WORD;
        uint32_t value = 0;
        if (trit_offset < TRIT_COUNT) {
            uint32_t max_trits = (word_idx == WORDS_PER_OPERAND - 1) ? 
                                 (TRIT_COUNT - trit_offset) : TRITS_PER_WORD; // Limit last word
            for (int i = 0; i < max_trits && trit_offset + i < TRIT_COUNT; i++) {
                TRIT t = pThis->output[trit_offset + i];
                value |= (t == -1 ? 2 : t) << (i * 2);
            }
            LogRel(("TernaryCoproc: OUTPUT read 0x%x (word %u)\n", value, word_idx));
            return value;
        }
    } else if (addr == TERNARY_REG_STATUS) {
        LogRel(("TernaryCoproc: STATUS read 0x%x\n", pThis->status));
        return pThis->status;
    } else if (addr == TERNARY_REG_OPERAND_COUNT) {
        LogRel(("TernaryCoproc: OPERAND_COUNT read %u\n", pThis->operand_count));
        return pThis->operand_count;
    }

    pThis->status |= STATUS_ERROR_INVALID_ADDR;
    LogRel(("TernaryCoproc: Unknown MMIO read at 0x%x\n", addr));
    return 0;
}

/* State Machine Processing (emulates hanoivm_fsm.v) */
static void ternary_process_state(PPDMDEVINS pDevIns, PDEVTERNARY pThis) {
    switch (pThis->state) {
        case STATE_IDLE:
            // Wait for MMIO write to trigger FETCH
            break;

        case STATE_FETCH:
            // Simulate memory latency (optional: add delay cycles if needed)
            pThis->state = STATE_EXECUTE;
            LogRel(("TernaryCoproc: FETCH state\n"));
            break;

        case STATE_EXECUTE:
            switch (pThis->command) {
                case CMD_TERNARY_NOP:
                    // No operation, clear output
                    for (int i = 0; i < TRIT_COUNT; i++) {
                        pThis->temp_result[i] = 0;
                    }
                    LogRel(("TernaryCoproc: NOP executed\n"));
                    break;

                case CMD_TERNARY_ADD:
                    if (pThis->stack_ptr >= 2) {
                        TRIT carry = 0;
                        for (int i = 0; i < TRIT_COUNT; i++) {
                            pThis->temp_result[i] = ternary_add(pThis->stack[pThis->stack_ptr-1][i],
                                                               pThis->stack[pThis->stack_ptr-2][i], &carry);
                        }
                        pThis->stack_ptr--;
                        for (int i = 0; i < TRIT_COUNT; i++) {
                            pThis->stack[pThis->stack_ptr-1][i] = pThis->temp_result[i];
                        }
                        LogRel(("TernaryCoproc: ADD executed, carry=%d, sp=%u\n", carry, pThis->stack_ptr));
                    } else {
                        pThis->status |= STATUS_ERROR_STACK_UNDERFLOW;
                        LogRel(("TernaryCoproc: ADD failed: stack underflow\n"));
                    }
                    break;

                case CMD_TERNARY_NOT:
                    if (pThis->stack_ptr >= 1) {
                        for (int i = 0; i < TRIT_COUNT; i++) {
                            pThis->temp_result[i] = ternary_not(pThis->stack[pThis->stack_ptr-1][i]);
                        }
                        for (int i = 0; i < TRIT_COUNT; i++) {
                            pThis->stack[pThis->stack_ptr-1][i] = pThis->temp_result[i];
                        }
                        LogRel(("TernaryCoproc: NOT executed, sp=%u\n", pThis->stack_ptr));
                    } else {
                        pThis->status |= STATUS_ERROR_STACK_UNDERFLOW;
                        LogRel(("TernaryCoproc: NOT failed: stack underflow\n"));
                    }
                    break;

                case CMD_TERNARY_AND:
                    if (pThis->stack_ptr >= 2) {
                        for (int i = 0; i < TRIT_COUNT; i++) {
                            pThis->temp_result[i] = ternary_and(pThis->stack[pThis->stack_ptr-1][i],
                                                               pThis->stack[pThis->stack_ptr-2][i]);
                        }
                        pThis->stack_ptr--;
                        for (int i = 0; i < TRIT_COUNT; i++) {
                            pThis->stack[pThis->stack_ptr-1][i] = pThis->temp_result[i];
                        }
                        LogRel(("TernaryCoproc: AND executed, sp=%u\n", pThis->stack_ptr));
                    } else {
                        pThis->status |= STATUS_ERROR_STACK_UNDERFLOW;
                        LogRel(("TernaryCoproc: AND failed: stack underflow\n"));
                    }
                    break;

                case CMD_TERNARY_PUSH:
                    if (pThis->stack_ptr < STACK_DEPTH) {
                        for (int i = 0; i < TRIT_COUNT; i++) {
                            pThis->stack[pThis->stack_ptr][i] = pThis->input[i];
                            pThis->temp_result[i] = pThis->input[i]; // For WRITEBACK
                        }
                        pThis->stack_ptr++;
                        LogRel(("TernaryCoproc: PUSH to stack, sp=%u\n", pThis->stack_ptr));
                    } else {
                        pThis->status |= STATUS_ERROR_STACK_OVERFLOW;
                        LogRel(("TernaryCoproc: Stack overflow\n"));
                    }
                    break;

                case CMD_TERNARY_POP:
                    if (pThis->stack_ptr > 0) {
                        pThis->stack_ptr--;
                        for (int i = 0; i < TRIT_COUNT; i++) {
                            pThis->temp_result[i] = pThis->stack[pThis->stack_ptr][i];
                        }
                        LogRel(("TernaryCoproc: POP from stack, sp=%u\n", pThis->stack_ptr));
                    } else {
                        pThis->status |= STATUS_ERROR_STACK_UNDERFLOW;
                        LogRel(("TernaryCoproc: Stack underflow\n"));
                    }
                    break;

                case CMD_TERNARY_SHA3:
                    ternary_sha3(pThis->input, pThis->temp_result, TRIT_COUNT);
                    LogRel(("TernaryCoproc: SHA3 executed (software-only)\n"));
                    break;

                default:
                    pThis->status |= STATUS_ERROR_INVALID_COMMAND;
                    LogRel(("TernaryCoproc: Unknown command 0x%x\n", pThis->command));
                    break;
            }
            pThis->state = STATE_WRITEBACK;
            break;

        case STATE_WRITEBACK:
            for (int i = 0; i < TRIT_COUNT; i++) {
                pThis->output[i] = pThis->temp_result[i];
            }
            pThis->state = STATE_DONE;
            LogRel(("TernaryCoproc: WRITEBACK completed\n"));
            break;

        case STATE_DONE:
            pThis->state = STATE_IDLE;
            LogRel(("TernaryCoproc: DONE, returning to IDLE\n"));
            break;
    }
}

/* Periodic Processing (called by VirtualBox timer or MMIO handler) */
static DECLCALLBACK(void) ternary_timer(PPDMDEVINS pDevIns, void *pvUser) {
    PDEVTERNARY pThis = (PDEVTERNARY)pvUser;
    ternary_process_state(pDevIns, pThis);
}

/* Device Constructor */
static DECLCALLBACK(int) devTernaryConstruct(PPDMDEVINS pDevIns, int iInstance, PCFGMNODE pCfg) {
    RT_NOREF(iInstance, pCfg);
    PDMDEVINSR3 pDevInsR3 = PDMDEVINS_2_R3PTR(pDevIns);
    PDEVTERNARY pThis = PDMINS_2_DATA(pDevInsR3, PDEVTERNARY);

    int rc = PDMDevHlpMmioMap(pDevIns, 0, TERNARY_MMIO_SIZE, ternary_mmio_read,
                              ternary_mmio_write, pThis, "TernaryCoproc");
    if (RT_FAILURE(rc)) {
        LogRel(("TernaryCoproc: Failed to map MMIO, rc=%Rrc\n", rc));
        return rc;
    }

    memset(pThis, 0, sizeof(DEVTERNARY));
    pThis->stack_ptr = 0;
    pThis->status = 0;
    pThis->operand_count = WORDS_PER_OPERAND; // Fixed to 6 words (81 trits)
    pThis->state = STATE_IDLE; // Initialize state machine
    LogRel(("TernaryCoproc: Device initialized\n"));

    // Register timer for state machine processing
    rc = PDMDevHlpTimerCreate(pDevIns, TMCLOCK_VIRTUAL, ternary_timer, pThis,
                              TMTIMERCREATE_FLAGS_DEFAULT, "TernaryTimer", NULL);
    if (RT_FAILURE(rc)) {
        LogRel(("TernaryCoproc: Failed to create timer, rc=%Rrc\n", rc));
        return rc;
    }

    return VINF_SUCCESS;
}

/* Device Destructor */
static DECLCALLBACK(void) devTernaryDestruct(PPDMDEVINS pDevIns) {
    LogRel(("TernaryCoproc: Device destroyed\n"));
}

/* Device Registration */
const PDMDEVREG g_DeviceTernary = {
    .u32Version = PDM_DEVREG_VERSION,
    .szName = "TernaryCoproc",
    .fFlags = PDM_DEVREG_FLAGS_DEFAULT,
    .szDescription = "Virtual ternary co-processor (for HanoiVM)",
    .pfnConstruct = devTernaryConstruct,
    .pfnDestruct = devTernaryDestruct,
    .pfnRelocate = NULL,
    .pfnMemSetup = NULL,
    .pfnPowerOn = NULL,
    .pfnReset = NULL,
    .pfnSuspend = NULL,
    .pfnResume = NULL,
    .pfnAttach = NULL,
    .pfnDetach = NULL,
    .iInstance = 0,
    .cbInstance = sizeof(DEVTERNARY),
    .u32VersionEnd = PDM_DEVREG_VERSION
};
