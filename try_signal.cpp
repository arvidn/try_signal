/*

Copyright (c) 2016, Arvid Norberg
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in
      the documentation and/or other materials provided with the distribution.
    * Neither the name of the author nor the names of its
      contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

*/

#include <cassert>
#include <system_error>
#include "signal_error_code.hpp"

#include <signal.h>
#include <setjmp.h>

#include "try_signal.hpp"

namespace sig {

try_signal::try_signal()
{
	// try_signal cannot be nested
	assert(jmpbuf == nullptr);

	struct sigaction sa;
	sa.sa_sigaction = &try_signal::handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_SIGINFO;
	sigaction(SIGSEGV, &sa, nullptr);
	sigaction(SIGBUS, &sa, nullptr);

	int const sig = sigsetjmp(buf, 1);
	jmpbuf = &buf;
	if (sig != 0)
	{
		fprintf(stderr, "sigsetjmp() returned %d\nthrowing exception\n", sig);
		throw std::system_error(static_cast<sig::errors::error_code_enum>(sig));
	}
	fprintf(stderr, "sigsetjmp() returned %d\nresuming\n", sig);
}

try_signal::~try_signal()
{
	fprintf(stderr, "~try_signal() clearing jmpbuf pointer\n");
	jmpbuf = nullptr;
}

void try_signal::handler(int const signo, siginfo_t* si, void* ctx)
{
	fprintf(stderr, "try_signal::handler(%d)\n", signo);
	if (jmpbuf)
	{
		fprintf(stderr, "siglongjmp(%d)\n", signo);
		siglongjmp(*jmpbuf, signo);
	}

	// this signal was not caused within the scope of a try_signal object,
	// invoke the default handler
	fprintf(stderr, "restoring default signal handler and re-raising\n");
	signal(signo, SIG_DFL);
	raise(signo);
}

thread_local sigjmp_buf* volatile try_signal::jmpbuf = nullptr;

} // namespace sig

