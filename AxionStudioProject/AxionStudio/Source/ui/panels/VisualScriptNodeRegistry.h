#pragma once

#include <string>
#include <vector>

#include <Silica/include/SWidget.h>

#include "AxionStudio/Source/scripting/VisualScriptGraph.h"

namespace Axion {

	struct VSPinDef {
		std::string name;
		PinType type;
		std::string defaultVal = "";
	};

	struct VSNodeDef {
		std::string name;
		std::string category;
		std::vector<VSPinDef> inputs;
		std::vector<VSPinDef> outputs;
	};

	class VSNodeRegistry {
	public:

		static const VSNodeDef& getNodeDef(NodeType type);
		static const std::vector<NodeType>& getAllNodeTypes();
		static Silica::Color getNodeColor(NodeType type);
		static Silica::Color getPinColor(PinType type);

	};

}
