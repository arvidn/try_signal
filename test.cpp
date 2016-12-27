#include <stdexcept>
#include "try_signal.hpp"

using sig::try_signal;

struct A
{
	A(char const* name) : _name(name) { fprintf(stderr, "%s\n", _name); }
	~A() { fprintf(stderr, "~%s\n", _name); }
	char const* _name;
};

int main() try
{
	A a1("a1");
	try_signal([] {
		A a2("a2");
		fprintf(stderr, "raise SIGSEGV\n");
#ifndef _WIN32
		raise(SIGSEGV);
#else
		RaiseException(EXCEPTION_IN_PAGE_ERROR, EXCEPTION_NONCONTINUABLE, 0, nullptr);
#endif
	});

	// return non-zero here because we don't expect this
	return 1;
}
catch (std::system_error const& e)
{
	fprintf(stderr, "exited with system_error exception: %s\n", e.what());

	// we expect this to happen, so return 0
	return e.code() == sig::errors::segmentation ? 0 : 1;
}

