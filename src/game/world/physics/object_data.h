#pragma once

namespace gorc {
namespace game {
namespace world {
namespace physics {

class object_data {
public:
    virtual ~object_data();
};

class thing_object_data : public object_data {
public:
    int thing_id;
};

class surface_object_data : public object_data {
public:
    int sector_id;
    int surface_id;
};

}
}
}
}
