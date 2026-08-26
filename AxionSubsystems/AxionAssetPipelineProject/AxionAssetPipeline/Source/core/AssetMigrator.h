#pragma once

#include "AxionEngine/Source/project/Project.h"

namespace Axion::AAP {

	class AssetMigrator {
	public:

		static void upgradeLegacyAssets(const Shared<Project>& project);

	};

}
