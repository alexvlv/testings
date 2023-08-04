#pragma once

#include <stdint.h>

enum class TaskType : uint32_t {
	Command = 0,
	Stream,
	StreamAliveAfterEos,
	StreamUnique,
	StreamUniqueAliveAfterEos,
};
