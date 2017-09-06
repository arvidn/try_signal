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
#include <cstring> // for memcpy

#include <signal.h>
#include <setjmp.h>

#include <setjmp.h>
#include <signal.h>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#ifdef __GNUC__
#include <excpt.h>
#else
#include <eh.h>
#endif
#endif

#include "try_signal.hpp"

namespace sig {

#if !defined _WIN32
// linux

namespace {

thread_local sigjmp_buf* volatile jmpbuf = nullptr;
std::atomic_flag once = ATOMIC_FLAG_INIT;

struct scoped_jmpbuf
{
	scoped_jmpbuf(sigjmp_buf* ptr)
	{
		_previous_ptr = sig::jmpbuf;
		sig::jmpbuf = ptr;
	}
	~scoped_jmpbuf() { sig::jmpbuf = _previous_ptr; }
	scoped_jmpbuf(scoped_jmpbuf const&) = delete;
	scoped_jmpbuf& operator=(scoped_jmpbuf const&) = delete;
private:
	sigjmp_buf* _previous_ptr;
};

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
	sa.sa_sigaction = &sig::handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_SIGINFO;
	sigaction(SIGSEGV, &sa, nullptr);
	sigaction(SIGBUS, &sa, nullptr);
}

} // anonymous namespace

void copy(iovec const* iov, std::size_t num_vecs)
{
	if (sig::once.test_and_set() == false) sig::setup_handler();

	sigjmp_buf buf;
	int const sig = sigsetjmp(buf, 1);
	// set the thread local jmpbuf pointer, and make sure it's cleared when we
	// leave the scope
	sig::scoped_jmpbuf scope(&buf);
	if (sig != 0)
		throw std::system_error(static_cast<sig::errors::error_code_enum>(sig));

	for (iovec const* end = iov + num_vecs; iov != end; ++iov)
		std::memcpy(iov->dest, iov->src, iov->length);
}

#elif __GNUC__
// mingw

namespace {

thread_local jmp_buf* volatile jmpbuf = nullptr;

long CALLBACK handler(EXCEPTION_POINTERS* pointers)
{
	if (jmpbuf)
		longjmp(*jmpbuf, pointers->ExceptionRecord->ExceptionCode);
	return EXCEPTION_CONTINUE_SEARCH;
}

struct scoped_handler
{
	scoped_handler(jmp_buf* ptr)
	{
		_previous_ptr = sig::jmpbuf;
		sig::jmpbuf = ptr;
		_handle = AddVectoredExceptionHandler(1, sig::handler);
	}
	~scoped_handler()
	{
		RemoveVectoredExceptionHandler(_handle);
		sig::jmpbuf = _previous_ptr;
	}
	scoped_handler(scoped_handler const&) = delete;
	scoped_handler& operator=(scoped_handler const&) = delete;
private:
	void* _handle;
	jmp_buf* _previous_ptr;
};

} // anonymous namespace

void copy(iovec const* iov, std::size_t num_vecs)
{
		jmp_buf buf;
		int const code = setjmp(buf);
		// set the thread local jmpbuf pointer, and make sure it's cleared when we
		// leave the scope
		sig::scoped_handler scope(&buf);
		if (code != 0)
			throw std::system_error(std::error_code(code, seh_category()));

	for (iovec const* end = iov + num_vecs; iov != end; ++iov)
		std::memcpy(iov->dest, iov->src, iov->length);
}

#else
// windows

namespace {

	// these are the kinds of SEH exceptions we'll translate into C++ exceptions
	bool catch_error(int const code)
	{
		return code == EXCEPTION_IN_PAGE_ERROR
			|| code == EXCEPTION_ACCESS_VIOLATION
			|| code == EXCEPTION_ARRAY_BOUNDS_EXCEEDED;
	}
} // anonymous namespace

void copy(iovec const* iov, std::size_t num_vecs)
{
	__try {
		for (iovec const* end = iov + num_vecs; iov != end; ++iov)
			std::memcpy(iov->dest, iov->src, iov->length);
	}
	__except (catch_error(GetExceptionCode())
		? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
	{
		throw std::system_error(std::error_code(GetExceptionCode(), seh_category()));
	}
}

#endif // _WIN32

} // namespace sig

