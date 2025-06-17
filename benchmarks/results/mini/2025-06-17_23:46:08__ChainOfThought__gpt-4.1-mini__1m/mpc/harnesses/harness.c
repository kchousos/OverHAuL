#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "mpc.h"
#include "mpc.c"  // Include the mpc.c source to provide definitions for mpc functions

// LibFuzzer entry point
int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
    if (Size == 0) {
        return 0;
    }

    // Create a parser for C identifiers (as per mpc_ident)
    mpc_parser_t *ident = mpc_ident();
    if (!ident) {
        return 0;
    }

    mpc_result_t result;
    // Parse the fuzz input as the string to parse
    mpc_nparse("<fuzz>", (const char *)Data, Size, ident, &result);

    if (result.output) {
        // On success, free the resulting AST
        mpc_ast_delete((mpc_ast_t *)result.output);
    }

    if (result.error) {
        // On error, free the error info
        mpc_err_delete(result.error);
    }

    mpc_delete(ident);

    return 0;
}