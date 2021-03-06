#pragma once

#include "base/utility/entity_id.h"

namespace gorc {
namespace game {
namespace world {
namespace sounds {
namespace components {

class stop_when_destroyed {
public:
    #include "stop_when_destroyed.uid"

    entity_id sound;

    stop_when_destroyed(entity_id sound);
};

}
}
}
}
}
