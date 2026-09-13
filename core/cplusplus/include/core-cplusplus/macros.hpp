#pragma once

#include <cassert>

#define TODO() assert(true && "TODO!")

#define OPT_ASSIGN_OR_RETURN(TARGET, EXPR)                                    \
do {                                                                          \
    auto TMP = (EXPR);                                                        \
    if (!TMP)                                                                 \
        return std::nullopt;                                                  \
    TARGET = std::move(*TMP);                                                 \
} while (0)
