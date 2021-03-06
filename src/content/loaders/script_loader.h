#pragma once

#include "base/content/loader.h"
#include "cog/compiler.h"

namespace gorc {
namespace content {
namespace loaders {

class script_loader : public loader {
    const cog::compiler& compiler;

public:
    static const std::vector<boost::filesystem::path> asset_root_path;

    script_loader(const cog::compiler& compiler);

    virtual std::unique_ptr<asset> deserialize(const boost::filesystem::path& filename,
            io::read_only_file& file, content_manager& manager, diagnostics::report& report) override;
};

}
}
}
