#include "update_slide_ceiling_sky_aspect.h"
#include "game/constants.h"

gorc::game::world::animations::aspects::update_slide_ceiling_sky_aspect::update_slide_ceiling_sky_aspect(component_system& cs, level_model& model)
    : inner_join_aspect(cs), model(model) {
    return;
}

void gorc::game::world::animations::aspects::update_slide_ceiling_sky_aspect::update(time t, entity_id, components::slide_ceiling_sky& anim) {
    model.header.ceiling_sky_offset += anim.speed * static_cast<float>(t.elapsed_as_seconds() * rate_factor);
}
