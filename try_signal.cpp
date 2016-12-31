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
#include <atomic>

#include <signal.h>
#include <setjmp.h>

#include "try_signal.hpp"

namespace sig {
namespace detail {

#if !defined _WIN32
// linux

void handler(int const signo, siginfo_t* si, void*)
{
	if (jmpbuf)
		siglongjmp(*jmpbuf, signo);

	// this signal was not caused within the scope of a try_signal object,
	// invoke the default handler
	signal(signo, SIG_DFL);
	raise(signo);
}

void setup_handler()
{
	struct sigaction sa;
	sa.sa_sigaction = &detail::handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_SIGINFO;
	sigaction(SIGSEGV, &sa, nullptr);
	sigaction(SIGBUS, &sa, nullptr);
}

thread_local sigjmp_buf* volatile jmpbuf = nullptr;
std::atomic_flag once = ATOMIC_FLAG_INIT;

#elif __GNUC__
// mingw

thread_local sigjmp_buf* volatile jmpbuf = nullptr;

long CALLBACK handler(EXCEPTION_POINTERS* pointers)
{
	if (jmpbuf)
		longjmp(*jmpbuf, pointers->ExceptionRecord.EceptionCode);
	return EXCEPTION_CONTINUE_SEARCH;
}
#else
// windows

void se_translator(unsigned int, _EXCEPTION_POINTERS* info)
{
	throw std::system_error(detail::map_exception_code(info->ExceptionRecord->ExceptionCode));
}

#endif // _WIN32

#if defined _WIN32
sig::errors::error_code_enum map_exception_code(DWORD const exception_code)
{
	switch (exception_code)
	{
		case EXCEPTION_ACCESS_VIOLATION:
		case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
		case EXCEPTION_GUARD_PAGE:
		case EXCEPTION_STACK_OVERFLOW:
		case EXCEPTION_FLT_STACK_CHECK:
		case EXCEPTION_IN_PAGE_ERROR:
			return sig::errors::segmentation;
		case EXCEPTION_BREAKPOINT:
		case EXCEPTION_SINGLE_STEP:
			return sig::errors::trap;
		case EXCEPTION_DATATYPE_MISALIGNMENT:
			return sig::errors::bus;
		case EXCEPTION_FLT_DENORMAL_OPERAND:
		case EXCEPTION_FLT_DIVIDE_BY_ZERO:
		case EXCEPTION_FLT_INEXACT_RESULT:
		case EXCEPTION_FLT_INVALID_OPERATION:
		case EXCEPTION_FLT_OVERFLOW:
		case EXCEPTION_FLT_UNDERFLOW:
		case EXCEPTION_INT_DIVIDE_BY_ZERO:
		case EXCEPTION_INT_OVERFLOW:
			return sig::errors::arithmetic_exception;
		case EXCEPTION_ILLEGAL_INSTRUCTION:
		case EXCEPTION_INVALID_DISPOSITION:
		case EXCEPTION_PRIV_INSTRUCTION:
		case EXCEPTION_NONCONTINUABLE_EXCEPTION:
		case STATUS_UNWIND_CONSOLIDATE:
			return sig::errors::illegal;
		case EXCEPTION_INVALID_HANDLE:
			return sig::errors::pipe;
		default:
			return sig::errors::illegal;
	}
}

#endif

} // namespace detail
} // namespace sig

