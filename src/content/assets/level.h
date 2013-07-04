#pragma once

#include "content/asset.h"
#include "content/loaders/level_loader.h"

#include "framework/math/vector.h"
#include "level_header.h"
#include "level_adjoin.h"
#include "level_surface.h"
#include "level_sector.h"
#include "script.h"
#include "material.h"
#include "template.h"

#include <unordered_map>
#include <btBulletDynamicsCommon.h>

namespace Gorc {
namespace Content {
namespace Assets {

class Level : public Asset {
public:
	using Loader = Loaders::LevelLoader;

	LevelHeader Header;
	std::vector<std::tuple<std::string, float, float>> MaterialEntries;
	std::vector<Material const*> Materials;

	Colormap const* MasterColormap = nullptr;
	std::vector<Colormap const*> Colormaps;

	std::vector<std::string> ScriptEntries;
	std::vector<Script const*> Scripts;

	std::vector<std::unique_ptr<std::string>> CogStrings;
	std::vector<std::tuple<Script const*, std::vector<Cog::VM::Value>>> Cogs;

	std::vector<Math::Vector<3>> Vertices;
	std::vector<Math::Vector<2>> TextureVertices;
	std::vector<LevelAdjoin> Adjoins;
	std::vector<LevelSurface> Surfaces;
	std::vector<LevelSector> Sectors;

	std::unordered_map<std::string, Template> Templates;
	std::vector<Template> Things;
};

}
}
}
