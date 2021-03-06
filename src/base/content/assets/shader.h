#pragma once

#include <GL/glew.h>
#include "base/content/asset.h"
#include "base/content/loaders/shader_loader.h"

namespace gorc {
namespace content {
namespace assets {

class shader : public asset {
public:
    using loader = loaders::shader_loader;

    GLuint program = 0;

    shader(GLuint program);
    shader(const shader&) = delete;
    shader(shader&&);
    ~shader();

    const shader& operator=(const shader&) = delete;
};

}
}
}
