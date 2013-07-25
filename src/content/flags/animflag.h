#pragma once

#include <cstdint>

namespace Gorc {
namespace Flags {

enum class AnimFlag : uint32_t {
	Looping							= 0x1,
	SkipFirstFrame					= 0x2,
	SkipFirstTwoFrames				= 0x4
};

}
}