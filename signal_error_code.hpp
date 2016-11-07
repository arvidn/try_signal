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

	enum error_code_enum
	{
		abort = SIGABRT,
		alarm = SIGALRM,
		floating_point_exception = SIGFPE,
		hangup = SIGHUP,
		illegal = SIGILL,
		interrupt = SIGINT,
		kill = SIGKILL,
		pipe = SIGPIPE,
		quite = SIGQUIT,
		segmentation = SIGSEGV,
		terminate = SIGTERM,
		user1 = SIGUSR1,
		user2 = SIGUSR2,
		child = SIGCHLD,
		cont = SIGCONT,
		stop = SIGSTOP,
		terminal_stop = SIGTSTP,
		terminal_in = SIGTTIN,
		terminal_out = SIGTTOU,
		bus = SIGBUS,
#ifdef SIGPOLL
		poll = SIGPOLL,
#endif
		profiler = SIGPROF,
		system_call = SIGSYS,
		trap = SIGTRAP,
		urgent_data = SIGURG,
		virtual_timer = SIGVTALRM,
		cpu_limit = SIGXCPU,
		file_size_limit = SIGXFSZ,
	};

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

