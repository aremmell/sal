/*
 * tests.c
 *
 * Author:    Ryan M. Lederman <lederman@gmail.com>
 * Copyright: Copyright (c) 2004-2023
 * Version:   0.2.0
 * License:   The MIT License (MIT)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "tests.h"
#include <stdlib.h>

static const bal_test_data bal_tests[] = {
    {"init-cleanup-sanity", baltest_init_cleanup_sanity},
    {"error-sanity",        baltest_error_sanity}
};

int main(int argc, char** argv)
{
    BAL_UNUSED(argc);
    BAL_UNUSED(argv);

    size_t tests_total  = _bal_countof(bal_tests);
    size_t tests_run    = 0UL;
    size_t tests_passed = 0UL;

    _bal_start_all_tests(tests_total);

    for (size_t n = 0UL; n < tests_total; n++) {
        _bal_start_test(tests_total, tests_run, bal_tests[n].name);
        bool pass = bal_tests[n].func();
        _bal_end_test(tests_total, tests_run, bal_tests[n].name, pass);
        if (pass)
            tests_passed++;
        tests_run++;
    }

    _bal_end_all_tests(tests_total, tests_run, tests_passed);
    return tests_passed == tests_run ? EXIT_SUCCESS : EXIT_FAILURE;
}

/******************************************************************************\
 *                            Test Implementations                            *
\******************************************************************************/

bool baltest_init_cleanup_sanity(void)
{
    bool pass = true;

    /* initialize twice. first should succeed, second should fail. */
    _bal_test_msg("running bal_init twice in a row...");
    pass &= bal_init();
    pass &= !bal_init();

    /* clean up twice. same scenario. */
    _bal_test_msg("running bal_cleanup twice in a row...");
    pass &= bal_cleanup();
    pass &= !bal_cleanup();

    /* initialize after cleanup should succeed. */
    _bal_test_msg("running bal_init after bal_cleanup...");
    pass &= bal_init();

    /* cleanup after init should succeed. */
    _bal_test_msg("running bal_cleanup after bal_init...");
    pass &= bal_cleanup();

    return pass;
}

bool baltest_error_sanity(void)
{
    bool pass = true;

    static const struct {
        const int code;
        const char* const as_string;
    } error_dict[] = {
        {BAL_E_NULLPTR,    "BAL_E_NULLPTR"},    /* NULL pointer argument */
        {BAL_E_BADSTRING,  "BAL_E_BADSTRING"},  /* Invalid string argument */
        {BAL_E_BADSOCKET,  "BAL_E_BADSOCKET"},  /* Invalid bal_socket argument */
        {BAL_E_BADBUFLEN,  "BAL_E_BADBUFLEN"},  /* Invalid buffer length argument */
        {BAL_E_INVALIDARG, "BAL_E_INVALIDARG"}, /* Invalid argument */
        {BAL_E_NOTINIT,    "BAL_E_NOTINIT"},    /* libbal is not initialized */
        {BAL_E_DUPEINIT,   "BAL_E_DUPEINIT"},   /* libbal is already initialized */
        {BAL_E_ASNOTINIT,  "BAL_E_ASNOTINIT"},  /* Asynchronous I/O is not initialized */
        {BAL_E_ASDUPEINIT, "BAL_E_ASDUPEINIT"}, /* Asynchronous I/O is already initialized */
        {BAL_E_ASNOSOCKET, "BAL_E_ASNOSOCKET"}, /* Socket is not registered for asynchronous I/O events */
        {BAL_E_BADEVTMASK, "BAL_E_BADEVTMASK"}, /* Invalid asynchronous I/O event bitmask */
        {BAL_E_INTERNAL,   "BAL_E_INTERNAL"},   /* An internal error has occurred */
        {BAL_E_UNAVAIL,    "BAL_E_UNAVAIL"}     /* Feature is disabled or unavailable */
    };

    /* test the libbal-specific errors. */
    for (size_t n = 0UL; n < _bal_countof(error_dict); n++) {
        (void)_bal_handleerr(_bal_mk_error(error_dict[n].code));
        bal_error err = {0};

        /* without extended information. */
        int ret = bal_get_last_error(&err);
        pass &= (error_dict[n].code == ret == err.code);
        pass &= err.desc[0] != '\0';
        _bal_test_msg("%s = %s", error_dict[n].as_string, err.desc);

        /* with extended information. */
        ret = bal_get_last_error_ext(&err);
        pass &= (error_dict[n].code == ret == err.code);
        pass &= err.desc[0] != '\0';
        _bal_test_msg("%s (extended) = %s", error_dict[n].as_string, err.desc);
    }

    /* test OS-level errors. */
#if defined(__WIN__)
    int os_err = WSAEINTR;
    (void)_bal_handleerr(os_err);
#else
    int os_err = EINTR;
    (void)_bal_handleerr(os_err);
#endif

    bal_error err = {0};

    /* without extended information. */
    int ret = bal_get_last_error(&err);
    pass &= (os_err == ret == err.code);
    pass &= err.desc[0] != '\0';
    _bal_test_msg("%d = %s", os_err, err.desc);

    /* with extended information. */
    ret = bal_get_last_error_ext(&err);
    pass &= (os_err == ret == err.code);
    pass &= err.desc[0] != '\0';
    _bal_test_msg("%d (extended) = %s", os_err, err.desc);

    return pass;
}

/******************************************************************************\
 *                                Test Harness                                *
\******************************************************************************/

void _bal_start_all_tests(size_t total)
{
    printf("\n" WHITEB("running %zu " ULINE("libbal") " %s...") "\n\n", total,
        _TEST_PLURAL(total));
}

void _bal_start_test(size_t total, size_t run, const char* name)
{
    printf(WHITEB("(%zu/%zu) '%s'...") "\n\n", run + 1, total, name);
}

void _bal_test_msg(const char* format, ...)
{
    char tmp[1024] = {0};
    va_list args;

    va_start(args, format);
    (void)vsnprintf(tmp, sizeof(tmp), format, args);
    va_end(args);

    printf("\t%s\n", tmp);
}

void _bal_end_test(size_t total, size_t run, const char* name, bool pass)
{
    if (pass) {
        printf("\n" WHITEB("(%zu/%zu) '%s': ") GREEN("PASS") "\n\n", run + 1, total, name);
    } else {
        printf("\n" WHITEB("(%zu/%zu) '%s': ") RED("FAIL") "\n\n", run + 1, total, name);
    }
}

void _bal_end_all_tests(size_t total, size_t run, size_t passed)
{
    if (run == passed) {
        printf(GREENB("all %zu "  ULINE("libbal") " %s passed!") "\n", run,
            _TEST_PLURAL(run));
    } else {
        printf(REDB("%zu of %zu " ULINE("libbal") " %s failed") "\n", run - passed,
            total, _TEST_PLURAL(run - passed));
    }
}
