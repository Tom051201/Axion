#pragma once

#include <filesystem>

#include "AxionEngine/Vendor/yaml-cpp/include/yaml-cpp/yaml.h"

#include "AxionStudio/Source/scripting/VisualScriptGraph.h"

namespace Axion {

	class VisualScriptSerializer {
	public:

		static void serialize(const VisualGraph& graph, const std::filesystem::path& filepath);
		static bool deserialize(VisualGraph& outGraph, const std::filesystem::path& filepath);

		static void serializePin(YAML::Emitter& out, const Pin& pin);

		static PinKind pinKindFromString(const std::string& str);
		static std::string pinKindToString(const PinKind& kind);
		static PinType pinTypeFromString(const std::string& str);
		static std::string pinTypeToString(const PinType& type);
		static NodeType nodeTypeFromString(const std::string& str);
		static std::string nodeTypeToString(const NodeType& type);
	};

}
