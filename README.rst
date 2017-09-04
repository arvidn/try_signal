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
	#include <vector>
	#include <numeric>
	#include "try_signal.hpp"
	#include <fcntl.h>
	#include <unistd.h>
	#include <sys/mman.h>

	int main() try
	{
		int fd = open("test_file", O_RDWR);
		void* map = mmap(nullptr, 1024, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

		std::vector<char> buf(1024);
		std::iota(buf.begin(), buf.end(), 0);

		// disk full or access after EOF are reported as exceptions
		// src, dest, length
		sig::iovec iov = { buf.data(), map, buf.size() };
		sig::copy(&iov, 1);

		munmap(map, 1024);
		close(fd);
		return 0;
	}
	catch (std::exception const& e)
	{
		fprintf(stderr, "exited with exception: %s\n", e.what());
		return 1;
	}

