#pragma once

#include "object_data.h"
#include "base/math/vector.h"
#include "base/utility/maybe.h"

namespace gorc {
namespace game {
namespace world {
namespace physics {

class contact {
public:
    vector<3> position;
    vector<3> normal;
    vector<3> velocity;
    maybe<int> contact_thing_id;
    maybe<int> contact_surface_id;

    contact() = default;
    contact(const vector<3>& position, const vector<3>& normal, const vector<3>& velocity);
};

}
}
}
}
