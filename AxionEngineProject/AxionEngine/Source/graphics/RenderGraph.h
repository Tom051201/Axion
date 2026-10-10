#pragma once

#include "AxionEngine/Source/core/Core.h"
#include "AxionEngine/Source/graphics/RenderContext.h"
#include "AxionEngine/Source/graphics/Framebuffer.h"
#include "AxionEngine/Source/graphics/Texture.h"

#include <string>
#include <vector>
#include <functional>

namespace Axion {

	using RGResourceID = uint32_t;
	constexpr RGResourceID RG_INVALID_RESOURCE = 0xFFFFFFFF;

	struct RGResourceAccess {
		RGResourceID id;
		ResourceState state;
	};

	struct RGBarrier {
		RGResourceID resourceId;
		ResourceState beforeState;
		ResourceState afterState;
	};

	struct RGTextureDesc {
		uint32_t width = 0;
		uint32_t height = 0;
		ColorFormat format = ColorFormat::RGBA8;
		DepthStencilFormat depthFormat = DepthStencilFormat::None;
		bool isDepth = false;
	};

	struct RGResource {
		RGResourceID id = RG_INVALID_RESOURCE;
		std::string name;

		bool isImported = false;
		FrameBuffer* importedFrameBuffer = nullptr;
		Ref<Texture2D> importedTexture = nullptr;

		Ref<FrameBuffer> transientFrameBuffer = nullptr;

		RGTextureDesc desc;

		ResourceState initialState = ResourceState::Common;
		ResourceState currentState = ResourceState::Common;
	};

	class RGPass;

	////////////////////////////////////////////////////////////////////////////////
	///// RGPassBuilder ////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	class RGPassBuilder {
	public:

		RGPassBuilder(RGPass* pass) : m_pass(pass) {}

		void read(RGResourceID id, ResourceState state = ResourceState::PixelShaderResource);
		void writeColor(RGResourceID id);
		void writeDepth(RGResourceID id);
		void write(RGResourceID id, ResourceState state);

	private:

		RGPass* m_pass;

	};

	////////////////////////////////////////////////////////////////////////////////
	///// RGPass ///////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	class RGPass {
	public:
		RGPass(const std::string& name) : m_name(name) {}

		const std::string& getName() const { return m_name; }

		std::function<void(RenderContext*)> executeCallback;

		std::vector<RGResourceAccess> inputs;
		std::vector<RGResourceAccess> outputs;
		std::vector<RGBarrier> prePassBarriers;

		uint32_t refCount = 0;
		bool isCulled = false;

	private:

		std::string m_name;

	};

	////////////////////////////////////////////////////////////////////////////////
	///// RenderGraph //////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	class RenderGraph {
	public:

		RenderGraph() = default;
		~RenderGraph();

		RGResourceID importResource(const std::string& name, FrameBuffer* framebuffer, ResourceState currentState = ResourceState::PixelShaderResource);
		RGResourceID createResource(const std::string& name, const RGTextureDesc& desc);

		template<typename SetupFunc, typename ExecuteFunc>
		void addPass(const std::string& name, SetupFunc setup, ExecuteFunc execute) {
			RGPass* pass = new RGPass(name);
			pass->executeCallback = execute;

			RGPassBuilder builder(pass);
			setup(builder);

			m_passes.push_back(pass);
		}

		void compile();
		void execute();
		void clear();

		RGResource* getResource(RGResourceID id);
		FrameBuffer* getPhysicalFramebuffer(RGResourceID id);

	private:

		std::vector<RGPass*> m_passes;
		std::vector<RGResource> m_resources;
		uint32_t m_nextResourceId = 0;

	};

}
