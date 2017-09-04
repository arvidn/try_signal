#include <stdexcept>
#include <array>

#include "try_signal.hpp"

int main()
{
	char const buf[] = "test...test";
	char dest[sizeof(buf)];

	{
		sig::iovec iov{buf, dest, sizeof(buf)};
		sig::copy(&iov, 1);
		if (!std::equal(buf, buf + sizeof(buf), dest)) {
			fprintf(stderr, "ERROR: buffer not copied correctly\n");
			return 1;
		}
	}

	try {
		std::array<sig::iovec, 2> iov{{{buf, dest, sizeof(buf)}
			, {nullptr, dest, 1}}};
		sig::copy(iov.data(), 2);
	}
	catch (std::system_error const& e)
	{
		if (e.code() != std::error_condition(sig::errors::segmentation)) {
			fprintf(stderr, "ERROR: expected segmentaiton violation error\n");
		}
		else {
			fprintf(stderr, "OK\n");
		}
		fprintf(stderr, "exited with system_error exception: %s\n", e.what());

		// we expect this to happen, so return 0
		return e.code() == std::error_condition(sig::errors::segmentation) ? 0 : 1;
	}

	// return non-zero here because we don't expect this
	fprintf(stderr, "ERROR: expected exit through exception\n");
	return 1;
}

