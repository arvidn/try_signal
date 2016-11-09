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
		raise(SIGSEGV);
	});


	// return non-zero here because we don't expect this
	return 1;
}
catch (std::exception const& e)
{
	fprintf(stderr, "exited with exception: %s\n", e.what());

	// we expect this to happen, so return 0
	return 0;
}

