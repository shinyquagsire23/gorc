#include "update_surface_light_aspect.h"

gorc::game::world::animations::aspects::update_surface_light_aspect::update_surface_light_aspect(component_system& cs, level_model& model)
    : inner_join_aspect(cs), model(model) {
    return;
}

void gorc::game::world::animations::aspects::update_surface_light_aspect::update(time t, entity_id, components::surface_light& anim) {
    // TODO: It may have been LEC's intention for this animation to loop.
    // Test it out, see if it was a bug all along.
    anim.anim_time += static_cast<float>(t.elapsed_as_seconds());
    float alpha = std::min(anim.anim_time / anim.change_time, 1.0f);
    model.surfaces[anim.surface].extra_light = (1.0f - alpha) * anim.start_light + alpha * anim.end_light;
}
