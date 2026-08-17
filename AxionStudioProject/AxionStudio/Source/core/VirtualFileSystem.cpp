#include "studiopch.h"
#include "VirtualFileSystem.h"

namespace Axion {

	VirtualFileSystem::VirtualFileSystem() {
		m_root = std::make_shared<VFSNode>();
		m_root->name = "Root";
		m_root->isDirectory = true;
	}

	std::shared_ptr<VFSNode> VirtualFileSystem::createFolder(std::shared_ptr<VFSNode> parent, const std::string& name) {
		auto node = std::make_shared<VFSNode>();
		node->name = name;
		node->isDirectory = true;
		node->parent = parent.get();
		parent->children.push_back(node);
		return node;
	}

	std::shared_ptr<VFSNode> VirtualFileSystem::addFile(std::shared_ptr<VFSNode> parent, const std::filesystem::path& physicalPath) {
		auto node = std::make_shared<VFSNode>();
		node->name = physicalPath.filename().string();
		node->isDirectory = false;
		node->physicalPath = physicalPath;
		node->parent = parent.get();
		parent->children.push_back(node);
		return node;
	}

	void VirtualFileSystem::moveNode(std::shared_ptr<VFSNode> node, std::shared_ptr<VFSNode> newParent) {
		if (!node || !newParent || !newParent->isDirectory || !node->parent) return;

		// -- Remove From Old Parent --
		auto& siblings = node->parent->children;
		siblings.erase(std::remove(siblings.begin(), siblings.end(), node), siblings.end());

		// -- Add To New Parent --
		node->parent = newParent.get();
		newParent->children.push_back(node);
	}

	void VirtualFileSystem::mirrorFromPhysical(const std::filesystem::path& physicalRoot) {
		m_root->children.clear();

		auto mirrorRecursive = [&](auto& self, const std::filesystem::path& currentPhysical, std::shared_ptr<VFSNode> currentVirtual) -> void {
			std::error_code ec;
			std::filesystem::directory_iterator it(currentPhysical, std::filesystem::directory_options::skip_permission_denied, ec);
			if (ec) return;

			for (const auto& entry : it) {
				std::string name = entry.path().filename().string();
				std::string ext = entry.path().extension().string();

				if (entry.is_directory()) {
					if (name[0] == '.' || name == "bin" || name == "obj" || name == "Export") continue;
					auto newFolder = createFolder(currentVirtual, name);
					self(self, entry.path(), newFolder);
				}
				else {
					if (ext == ".dll" || ext == ".pdb" || ext == ".csproj" || ext == ".sln" || ext == ".cache" || ext == ".axproj") continue;
					addFile(currentVirtual, entry.path());
				}
			}
		};

		mirrorRecursive(mirrorRecursive, physicalRoot, m_root);
	}

	// ----- YAML Serialization Helpers -----
	static void serializeNode(YAML::Emitter& out, const std::shared_ptr<VFSNode>& node) {
		out << YAML::BeginMap;
		out << YAML::Key << "Name" << YAML::Value << node->name;
		out << YAML::Key << "IsDirectory" << YAML::Value << node->isDirectory;
		if (!node->isDirectory) {
			out << YAML::Key << "PhysicalPath" << YAML::Value << node->physicalPath.string();
		}

		if (node->isDirectory && !node->children.empty()) {
			out << YAML::Key << "Children" << YAML::Value << YAML::BeginSeq;
			for (const auto& child : node->children) {
				serializeNode(out, child);
			}
			out << YAML::EndSeq;
		}
		out << YAML::EndMap;
	}

	static void deserializeNode(const YAML::Node& yamlNode, std::shared_ptr<VFSNode> parent) {
		for (auto childYaml : yamlNode) {
			auto node = std::make_shared<VFSNode>();
			node->name = childYaml["Name"].as<std::string>();
			node->isDirectory = childYaml["IsDirectory"].as<bool>();
			node->parent = parent.get();

			if (!node->isDirectory && childYaml["PhysicalPath"]) {
				node->physicalPath = childYaml["PhysicalPath"].as<std::string>();
			}

			parent->children.push_back(node);

			if (node->isDirectory && childYaml["Children"]) {
				deserializeNode(childYaml["Children"], node);
			}
		}
	}

	void VirtualFileSystem::save(const std::filesystem::path& vfsFilePath) {
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "VirtualFileSystem" << YAML::Value << YAML::BeginSeq;
		for (const auto& child : m_root->children) {
			serializeNode(out, child);
		}
		out << YAML::EndSeq;
		out << YAML::EndMap;

		std::ofstream fout(vfsFilePath);
		fout << out.c_str();
	}

	void VirtualFileSystem::load(const std::filesystem::path& vfsFilePath) {
		m_root->children.clear();
		if (!std::filesystem::exists(vfsFilePath)) return;

		try {
			YAML::Node data = YAML::LoadFile(vfsFilePath.string());
			if (auto vfsNode = data["VirtualFileSystem"]) {
				deserializeNode(vfsNode, m_root);
			}
		}
		catch (const YAML::Exception&) {
			// Fallback if file is corrupted
		}
	}
}
