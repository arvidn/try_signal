try_signal
==========

.. image:: https://travis-ci.org/arvidn/try_signal.svg?branch=master
    :target: https://travis-ci.org/arvidn/try_signal

.. image:: https://ci.appveyor.com/api/projects/status/le8jjroaai8081f1?svg=true
	:target: https://ci.appveyor.com/project/arvidn/try-signal/branch/master

The ``try_signal`` library provide a simple abstraction over ``memcpy()`` that
reports errors as C++ exceptions. This is especially useful when performing disk
I/O via memory mapped files, where I/O errors are reported as ``SIGBUS`` and
``SIGSEGV`` or as structured exceptions on windows.

Example::

	#include <stdexcept>
	#include "try_signal.hpp"

	int main() try
	{
		
		try_signal([] {
			A a2("a2");
			// destructors might not be called
			// when exiting through a signal
			raise(SIGSEGV);
		});
		return 0;
	}
	catch (std::exception const& e)
	{
		fprintf(stderr, "exited with exception: %s\n", e.what());
		return 1;
	}

Prints::

	a1
	a2
	~a1
	Failed with exception: segmentation fault

