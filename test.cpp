#include <stdexcept>
#include "try_signal.hpp"

using sig::try_signal;

struct A
{
	A(char const* name) : _name(name) { fprintf(stderr, "%s\n", _name); }
	~A() { fprintf(stderr, "~%s\n", _name); }
	char const* _name;
};

int main()
{
	try
	{
		A a1("a1");
		{
			try_signal _try;
			A a2("a2");
			raise(SIGSEGV);
		}
		return 0;
	}
	catch (std::exception const& e)
	{
		fprintf(stderr, "exited with exception: %s\n", e.what());
		return 1;
	}
}
