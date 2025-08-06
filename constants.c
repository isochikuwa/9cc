#include "constants.h"

// レジスタ定義の実装
const char *ARG_REGISTERS_64[MAX_FUNCTION_ARGS] = {
    "rdi", "rsi", "rdx", "rcx", "r8", "r9"
};

const char *ARG_REGISTERS_32[MAX_FUNCTION_ARGS] = {
    "edi", "esi", "edx", "ecx", "r8d", "r9d"
};

const char *ARG_REGISTERS_8[MAX_FUNCTION_ARGS] = {
    "dil", "sil", "dl", "cl", "r8b", "r9b"
};
