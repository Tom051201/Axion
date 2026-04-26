#pragma once

#include <string>
#include <filesystem>

#include "AxionEngine/Source/core/AssetRegistry.h"
#include "AxionEngine/Source/core/Version.h"

namespace Axion {

	struct ProjectSpecification {
		std::string name;
		std::filesystem::path location;
		std::string author;
		std::string company;
		std::string description;
		Version version = Version(1, 0, 0);
	};

	class Project {
	public:

		Project(const std::string& name);

		void setName(const std::string& name) { m_name = name; }
		void setProjectPath(const std::filesystem::path& path) { m_projectPath = path; }
		void setAssetsPath(const std::filesystem::path& path) { m_assetsPath = path; }
		void setDefaultScene(const std::filesystem::path& path) { m_defaultScene = path; }
		void setDefaultSceneUUID(UUID uuid) { m_defaultSceneUUID = uuid; }
		void setAuthor(const std::string& author) { m_author = author; }
		void setCompany(const std::string& company) { m_company = company; }
		void setDescription(const std::string& desc) { m_description = desc; }
		void setVersion(const Version& version) { m_version = version; }
		void setEngineVersion(const Version& version) { m_engineVersion = version; }
		void setAppIconPath(const std::filesystem::path& path) { m_appIconPath = path; }

		const std::string& getName() const { return m_name; }
		const std::filesystem::path& getProjectPath() const { return m_projectPath; }
		const std::filesystem::path& getAssetsPath() const { return m_assetsPath; }
		const std::filesystem::path& getDefaultScene() const { return m_defaultScene; }
		UUID getDefaultSceneUUID() const { return m_defaultSceneUUID; }
		const std::string& getAuthor() const { return m_author; }
		const std::string& getCompany() const { return m_company; }
		const std::string& getDescription() const { return m_description; }
		const Version& getVersion() const { return m_version; }
		const Version& getEngineVersion() const { return m_engineVersion; }
		const std::filesystem::path& getAppIconPath() const { return m_appIconPath; }

		Ref<AssetRegistry> getAssetRegistry() { return m_assetRegistry; }

		void save(const std::filesystem::path& path); // TODO: rename to save text
		static Ref<Project> load(const std::filesystem::path& path); // TODO: rename to loadText
		static Ref<Project> loadBinary(const std::filesystem::path& path);
		static Ref<Project> createNew(const ProjectSpecification& spec);
		static void generateScriptProject(const std::filesystem::path& projectDir);

	private:

		// ----- Required -----
		std::string m_name;
		Version m_version;
		Version m_engineVersion;
		std::filesystem::path m_projectPath;
		std::filesystem::path m_assetsPath;
		std::filesystem::path m_defaultScene;
		UUID m_defaultSceneUUID = UUID(0, 0);

		// ----- Optional -----
		std::string m_author;
		std::string m_company;
		std::string m_description;
		std::filesystem::path m_appIconPath;

		Ref<AssetRegistry> m_assetRegistry;
	};

}
