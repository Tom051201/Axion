#pragma once

#include "AxionEngine/Source/render/Shader.h"

namespace Axion {

	class OpenGL3Shader : public Shader {
	public:

		OpenGL3Shader();
		OpenGL3Shader(const ShaderSpecification& spec);
		~OpenGL3Shader() override;

		void release();

		void bind() const override;
		void unbind() const override;

		const std::string& getName() const override { return m_name; }

		void compileFromFile(const std::string& filePath) override;
		void recompile() override;
		void loadFromBytecode(const uint8_t* vsData, size_t vsSize, const uint8_t* psData, size_t psSize) override;

		static ShaderBytecode compileToBytecode(const std::string& filePath);

		int getBindPoint(const std::string& name) const override;
		uint32_t getTextureTableBindSlot() const override;

	private:

		uint32_t m_programID;
		std::string m_name;

		uint32_t compileStage(uint32_t type, const std::string& source);
		void linkProgram(uint32_t vertexShader, uint32_t pixelShader);

	};

}
