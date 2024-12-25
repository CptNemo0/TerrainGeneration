#pragma once

#include <mutex>

namespace mutexes
{
	std::mutex device_mutex;
	std::mutex terrain_mutex;
}