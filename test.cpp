#include <stdexcept>
#include <array>

#include "try_signal.hpp"

int main() try
{
	char const buf[] = "test...test";
	char dest[sizeof(buf)];

	std::array<sig::iovec, 2> iov{{{buf, dest, sizeof(buf)}
		, {nullptr, dest, 1}}};
	sig::copy(iov.data(), 2);

	// return non-zero here because we don't expect this
	return 1;
}
catch (std::system_error const& e)
{
	fprintf(stderr, "exited with system_error exception: %s\n", e.what());

	// we expect this to happen, so return 0
	return e.code() == sig::errors::segmentation ? 0 : 1;
}

