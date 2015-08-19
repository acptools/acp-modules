#ifndef MODULES_ACP_UTILS_ANALOG_INCLUDE_ANALOGUTILS_H_
#define MODULES_ACP_UTILS_ANALOG_INCLUDE_ANALOGUTILS_H_

#include <acp/core.h>

namespace acp_utils {
	int analogReadWithSHDelay(uint8_t pin, unsigned int shdelay);
}

#endif /* MODULES_ACP_UTILS_ANALOG_INCLUDE_ANALOGUTILS_H_ */
