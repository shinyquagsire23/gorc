#pragma once

#include "content/assets/puppet.h"
#include "game/flags/standing_material_type.h"

namespace gorc {
namespace game {
namespace world {
namespace components {

class puppet_animations {
public:
    #include "puppet_animations.uid"

    content::assets::puppet const &puppet;
    flags::puppet_mode_type puppet_mode_type;
    int actor_walk_animation = -1;

    puppet_animations(content::assets::puppet const &puppet,
                      flags::puppet_mode_type = flags::puppet_mode_type::unarmed);
};

}
}
}
}
