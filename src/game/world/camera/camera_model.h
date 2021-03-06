#pragma once

#include "camera_state.h"
#include "current_camera_state.h"
#include "game/flags/camera_state_flag.h"
#include "base/utility/flag_set.h"
#include <vector>

namespace gorc {

namespace content {
namespace assets {
class model;
}
}

namespace game {
namespace world {
namespace camera {

class camera_model {
public:
    flag_set<flags::camera_state_flag> state_flags;

    std::vector<camera_state> cameras = {
            camera_state(), camera_state()
    };

    int current_camera = 0;
    current_camera_state current_computed_state;

    content::assets::model const* pov_model = nullptr;
    int pov_key_mix_id = -1;

    vector<3> waggle = make_zero_vector<3, float>();
    float waggle_speed = 0.0f;
    float waggle_time = 0.0;
};

}
}
}
}
