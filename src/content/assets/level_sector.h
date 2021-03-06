#pragma once

#include "base/math/vector.h"
#include "base/math/box.h"
#include "base/content/assets/sound.h"
#include "content/flags/sector_flag.h"
#include "base/utility/maybe.h"

namespace gorc {
namespace content {
namespace assets {

class colormap;

class level_sector {
public:
    int number;
    flag_set<flags::sector_flag> flags;
    float ambient_light;
    float extra_light;
    int colormap_id;
    maybe<colormap const*> cmp;
    vector<3> tint;
    box<3> bounding_box;
    box<3> collide_box;
    sound const* ambient_sound = nullptr;
    float ambient_sound_volume = 0.0f;
    vector<3> center;
    float radius;
    std::vector<size_t> vertices;
    int first_surface;
    int surface_count;
    vector<3> thrust;
};

}
}
}
