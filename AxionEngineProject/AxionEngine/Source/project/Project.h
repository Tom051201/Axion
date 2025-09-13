#pragma once

#include <string>
#include <filesystem>

namespace Axion {

	struct ProjectSpecification {
		std::string name;
		std::string location;
		std::string author;
		std::string company;
		std::string description;
	};

	class Project {
	public:

		Project(const std::string& name);

		void setName(const std::string& name) { m_name = name; }
		void setProjectPath(const std::string& path) { m_projectPath = path; }
		void setAssetsPath(const std::string& path) { m_assetsPath = path; }
		void setScenesPath(const std::string& path) { m_scenesPath = path; }
		void setDefaultScene(const std::string& path) { m_defaultScene = path; }
		void setAuthor(const std::string& author) { m_author = author; }
		void setCompany(const std::string& company) { m_company = company; }
		void setDescription(const std::string& desc) { m_description = desc; }
		void setVersion(const std::string& version) { m_version = version; }
		void setEngineVersion(const std::string& version) { m_engineVersion = version; }

		const std::string& getName() const { return m_name; }
		const std::string& getProjectPath() const { return m_projectPath; }
		const std::string& getAssetsPath() const { return m_assetsPath; }
		const std::string& getScenesPath() const { return m_scenesPath; }
		const std::string& getDefaultScene() const { return m_defaultScene; }
		const std::string& getAuthor() const { return m_author; }
		const std::string& getCompany() const { return m_company; }
		const std::string& getDescription() const { return m_description; }
		const std::string& getVersion() const { return m_version; }
		const std::string& getEngineVersion() const { return m_engineVersion; }

		void save(const std::string& path);
		static Ref<Project> load(const std::string& path);

		static Ref<Project> createNew(const ProjectSpecification& spec);

	private:

		// ----- Required -----
		std::string m_name;
		std::string m_version;
		std::string m_engineVersion;
		std::string m_projectPath;
		std::string m_assetsPath;
		std::string m_scenesPath;

		// ----- Optional -----
		std::string m_author;
		std::string m_company;
		std::string m_description;
		std::string m_defaultScene;
	};

}
