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

#ifndef SIGNAL_ERROR_CODE_HPP_INCLUDED
#define SIGNAL_ERROR_CODE_HPP_INCLUDED

#include <signal.h>
#include <system_error>

namespace sig {
namespace errors {

#ifdef _WIN32
#define SIG_ENUM(name, sig) name,
#else
#define SIG_ENUM(name, sig) name = sig,
#endif

	enum error_code_enum
	{
		SIG_ENUM(abort, SIGABRT)
		SIG_ENUM(alarm, SIGALRM)
		SIG_ENUM(arithmetic_exception, SIGFPE)
		SIG_ENUM(hangup, SIGHUP)
		SIG_ENUM(illegal, SIGILL)
		SIG_ENUM(interrupt, SIGINT)
		SIG_ENUM(kill, SIGKILL)
		SIG_ENUM(pipe, SIGPIPE)
		SIG_ENUM(quit, SIGQUIT)
		SIG_ENUM(segmentation, SIGSEGV)
		SIG_ENUM(terminate, SIGTERM)
		SIG_ENUM(user1, SIGUSR1)
		SIG_ENUM(user2, SIGUSR2)
		SIG_ENUM(child, SIGCHLD)
		SIG_ENUM(cont, SIGCONT)
		SIG_ENUM(stop, SIGSTOP)
		SIG_ENUM(terminal_stop, SIGTSTP)
		SIG_ENUM(terminal_in, SIGTTIN)
		SIG_ENUM(terminal_out, SIGTTOU)
		SIG_ENUM(bus, SIGBUS)
#ifdef SIGPOLL
		SIG_ENUM(poll, SIGPOLL)
#endif
		SIG_ENUM(profiler, SIGPROF)
		SIG_ENUM(system_call, SIGSYS)
		SIG_ENUM(trap, SIGTRAP)
		SIG_ENUM(urgent_data, SIGURG)
		SIG_ENUM(virtual_timer, SIGVTALRM)
		SIG_ENUM(cpu_limit, SIGXCPU)
		SIG_ENUM(file_size_limit, SIGXFSZ)
	};

#undef SIG_ENUM

	std::error_code make_error_code(error_code_enum e);

} // namespace errors

std::error_category& sig_category();

} // namespace sig

namespace std
{
template<>
struct is_error_code_enum<sig::errors::error_code_enum>
	: std::integral_constant<bool, true> {};
} // namespace std

#endif

