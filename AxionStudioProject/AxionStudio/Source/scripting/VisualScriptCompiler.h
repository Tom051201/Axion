#pragma once
#include "axpch.h"

#include "AxionStudio/Source/scripting/VisualScriptGraph.h"

namespace Axion {

	class VisualScriptCompiler {
	public:

		static std::string compileGraph(const VisualGraph& graph);

	private:

		static std::string getInlineValueString(const Pin& pin);
		static std::string formatFloat(float value);
		static std::string resolvePinValue(const VisualGraph& graph, const Pin& inputPin);
		static void compileFlowExecution(const VisualGraph& graph, int currentFlowOutputPinID, std::stringstream& cs, int indentLevel);

	};

}
