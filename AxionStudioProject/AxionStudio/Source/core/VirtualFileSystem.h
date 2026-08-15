#pragma once

#include <string>
#include <filesystem>
#include <vector>

namespace Axion {

	struct VFSNode {
		std::string name;
		bool isDirectory = false;
		std::filesystem::path physicalPath;
		std::vector<std::shared_ptr<VFSNode>> children;
		VFSNode* parent = nullptr;
	};

	class VirtualFileSystem {
	public:

		VirtualFileSystem();
		~VirtualFileSystem() = default;

		void load(const std::filesystem::path& vfsFilePath);
		void save(const std::filesystem::path& vfsFilePath);

		void mirrorFromPhysical(const std::filesystem::path& physicalRoot);

		std::shared_ptr<VFSNode> getRoot() const { return m_root; }

		std::shared_ptr<VFSNode> createFolder(std::shared_ptr<VFSNode> parent, const std::string& name);
		std::shared_ptr<VFSNode> addFile(std::shared_ptr<VFSNode> parent, const std::filesystem::path& physicalPath);
		void moveNode(std::shared_ptr<VFSNode> node, std::shared_ptr<VFSNode> newParent);

	private:

		std::shared_ptr<VFSNode> m_root;

	};

}
