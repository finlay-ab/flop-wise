#pragma once

#include <cstdio>
#include <cstdlib>

inline void CheckFailed(const char* expr, const char* file, int line)
{
    std::fprintf(stderr, "CHECK failed: %s\n  at %s:%d\n", expr, file, line);
    std::exit(1);
}

#define CHECK(expr) ((expr) ? (void)0 : CheckFailed(#expr, __FILE__, __LINE__))