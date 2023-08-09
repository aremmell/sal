/*
 * balserver.cc
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
#include "balserver.hh"
#include <cstdio>

using namespace std;

int main(int argc, char** argv)
{
    BAL_UNUSED(argc);
    BAL_UNUSED(argv);

    if (!balcommon::initialize())
        return EXIT_FAILURE;

    int ret = bal_initialize();
    EXIT_IF_FAILED(ret, nullptr, "bal_initialize");

    bal_socket s;
    ret = bal_sock_create(&s, AF_INET, IPPROTO_TCP, SOCK_STREAM);
    EXIT_IF_FAILED(ret, nullptr, "bal_sock_create");

    ret = bal_bind(&s, balcommon::localaddr, balcommon::portnum);
    EXIT_IF_FAILED(ret, nullptr, "bal_bind");

    ret = bal_listen(&s, 0);
    EXIT_IF_FAILED(ret, nullptr, "bal_listen");

    printf("listening on %s:%s; ctrl+c to exit...\n",
        balcommon::localaddr, balcommon::portnum);

    do {
        //bal_socket client_sock   = {0};
        //bal_sockaddr client_addr = {0};

    //    bal_accept(&s, &client_sock, &client_addr);

        int yield = sched_yield();
        assert(0 == yield);
    } while (balcommon::should_run());


    ret = bal_close(&s);
    EXIT_IF_FAILED(ret, nullptr, "bal_close");

    ret = bal_finalize();
    EXIT_IF_FAILED(ret, nullptr, "bal_finalize");

    return EXIT_SUCCESS;
}


