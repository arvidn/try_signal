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

#ifndef TRY_SIGNAL_HPP_INCLUDED
#define TRY_SIGNAL_HPP_INCLUDED

#include <atomic>
#include <setjmp.h>
#include <signal.h>

#include "signal_error_code.hpp"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#ifdef __GNUC__
#include <excpt.h>
#else
#include <eh.h>
#endif
#endif

namespace sig {

#if !defined _WIN32
	// linux

	namespace detail {

		extern thread_local sigjmp_buf* volatile jmpbuf;
		extern std::atomic_flag once;

		struct scoped_jmpbuf
		{
			scoped_jmpbuf(sigjmp_buf* ptr)
			{
				_previous_ptr = sig::detail::jmpbuf;
				sig::detail::jmpbuf = ptr;
			}
			~scoped_jmpbuf() { sig::detail::jmpbuf = _previous_ptr; }
			scoped_jmpbuf(scoped_jmpbuf const&) = delete;
			scoped_jmpbuf& operator=(scoped_jmpbuf const&) = delete;
		private:
			sigjmp_buf* _previous_ptr;
		};

		void handler(int signo, siginfo_t* si, void* ctx);
		void setup_handler();

	} // namespace detail

	template <typename F, typename... Args>
	auto try_signal(F&& f, Args... args) -> decltype(f(args...))
	{
		if (detail::once.test_and_set() == false) detail::setup_handler();

		sigjmp_buf buf;
		int const sig = sigsetjmp(buf, 1);
		// set the thread local jmpbuf pointer, and make sure it's cleared when we
		// leave the scope
		detail::scoped_jmpbuf scope(&buf);
		if (sig != 0)
			throw std::system_error(static_cast<sig::errors::error_code_enum>(sig));

		return f(args...);
	}

#elif defined __GNUC__
	// mingw

	namespace detail {
		sig::errors::error_code_enum map_exception_code(DWORD const exception_code);
		long CALLBACK handler(EXCEPTION_POINTERS* pointers);
		extern thread_local sigjmp_buf* volatile jmpbuf;

		struct scoped_handler
		{
			scoped_handler(jmp_buf* ptr)
			{
				_previous_ptr = sig::detail::jmpbuf;
				sig::detail::jmpbuf = ptr;
				_handle = AddVectoredExceptionHandler(1, detail::handler);
			}
			~scoped_handler()
			{
				RemoveVectoredExceptionHandler(_handle);
				sig::detail::jmpbuf = _previous_ptr;
			}
			scoped_handler(scoped_handler const&) = delete;
			scoped_handler& operator=(scoped_handler const&) = delete;
		private:
			void* _handle;
			jmp_buf* _previous_ptr;
		};
	} // namespace detail

	template <typename F, typename... Args>
	auto try_signal(F&& f, Args... args) -> decltype(f(args...))
	{
		jmp_buf buf;
		int const code = setjmp(buf, 1);
		// set the thread local jmpbuf pointer, and make sure it's cleared when we
		// leave the scope
		detail::scoped_handler scope(&buf);
		if (code != 0)
			throw std::system_error(detail::map_exception_code(code));

		return f(args...);
	}

#else
	// windows

	namespace detail {
		void se_translator(unsigned int, _EXCEPTION_POINTERS* ExceptionInfo);

		struct scoped_se_translator
		{
			scoped_se_translator()
			{ _prev_fun = _set_se_translator(se_translator); }

			~scoped_se_translator()
			{ _set_se_translator(_prev_fun); }

			scoped_se_translator(scoped_se_translator const&) = delete;
			scoped_se_translator& operator=(scoped_se_translator const&) = delete;

		private:
			void (*_prev_fun)(unsigned int, struct _EXCEPTION_POINTERS*);
		};
	}

	template <typename F, typename... Args>
	auto try_signal(F&& f, Args... args) -> decltype(f(args...))
	{
		detail::scoped_se_translator scope;
		return f(args...);
	}

#endif // _WIN32

} // namespace sig

#endif

