#pragma once

#include <cassert>

#define TODO() assert(true && "TODO!")

#define OPT_ASSIGN_OR_RETURN(target_, expr_)                                  \
do {                                                                          \
    auto temp_ = (expr_);                                                     \
    if (!temp_)                                                               \
        return std::nullopt;                                                  \
    target_ = std::move(*temp_);                                              \
} while (0)
