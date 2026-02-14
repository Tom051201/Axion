#include "EditorResourceManager.h"

namespace Axion {

	std::unordered_map<std::string, Ref<Texture2D>> EditorResourceManager::s_texture2Ds;

	void EditorResourceManager::initialize(){

	}

	void EditorResourceManager::shutdown() {
		for (auto& t : s_texture2Ds) {
			t.second->release();
		}
		s_texture2Ds.clear();
	}

	Ref<Texture2D> EditorResourceManager::getIcon(const std::string& name) {
		if (s_texture2Ds.find(name) != s_texture2Ds.end()) {
			return s_texture2Ds[name];
		}
		return nullptr;
	}

	void EditorResourceManager::loadIcon(const std::string& name, const std::string& path) {
		if (s_texture2Ds.find(name) != s_texture2Ds.end()) {
			AX_CORE_LOG_WARN("EditorResourceManager: Overwriting existing icon named '{0}' with new path '{1}'", name, path);
		}
		s_texture2Ds[name] = Texture2D::create(path);
	}

}
