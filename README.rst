try_signal
==========

.. image:: https://travis-ci.org/arvidn/try_signal.svg?branch=master
    :target: https://travis-ci.org/arvidn/try_signal

``try_signal`` is a simple abstraction to *locally* handle signals and turning
them into C++ exceptions. This is especially useful when performing disk I/O via
memory mapped files, where I/O errors are reported as ``SIGBUS`` and
``SIGSEGV``.

Example::

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
		{
			// try_signal will throw if a SIGBUS or SIGSEGV is caught during
			// its lifetime
			try_signal _try;

			A a2("a2");
			raise(SIGSEGV);
		}
		return 0;
	}
	catch (std::exception const& e)
	{
		fprintf(stderr, "Failed with exception: %s\n", e.what());
		return 1;
	}

Prints::

	a1
	a2
	~a2
	~a1
	Failed with exception: segmentation fault

