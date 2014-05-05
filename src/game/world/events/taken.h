#pragma once

#include "base/utility/entity_id.h"

namespace gorc {
namespace game {
namespace world {
namespace events {

class taken {
public:
    #include "taken.uid"

    entity_id taken_thing;
    entity_id taker;

    taken(entity_id taken_thing, entity_id taker);
};

}
}
}
}
