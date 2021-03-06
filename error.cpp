#include "common.h"

static void fail_helper(bool include_errno, char const *format, va_list args)
  __attribute((format(printf, 2, 0), noreturn));

static void fail_helper(bool include_errno, char const *format, va_list args) {
    fprintf(stderr, "%s: ", program_name);
    vfprintf(stderr, format, args);
    if (include_errno) {
        char const*const err_str = strerror(errno);
        fprintf(stderr, ": %s (errno = %d)", err_str ? err_str : "unknown error", errno);
    }
    putc('\n', stderr);
    exit(EXIT_FAILURE);
}

void fail(char const *format, ...) {
    va_list args;
    va_start(args, format);
    fail_helper(false, format, args);
}

void fail_if(bool condition, char const *format, ...) {
    va_list args;
    if (condition) {
        va_start(args, format);
        fail_helper(false, format, args);
    }
}

void errno_fail_if(bool condition, char const *format, ...) {
    va_list args;
    if (condition) {
        va_start(args, format);
        fail_helper(true, format, args);
    }
}

// Like errno_fail_if() but with a specified errno value
void errno_val_fail_if(bool condition, int errno_val, char const *format, ...) {
    va_list args;
    if (condition) {
        va_start(args, format);
        errno = errno_val;
        fail_helper(true, format, args);
    }
}
