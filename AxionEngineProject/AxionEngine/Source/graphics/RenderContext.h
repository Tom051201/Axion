#pragma once

namespace Axion {

	class FrameBuffer;

	enum class ResourceState {
		Common,
		RenderTarget,
		DepthWrite,
		DepthRead,
		PixelShaderResource,
		CopyDest,
		CopySource,
		Present
	};

	class RenderContext {
	public:

		virtual ~RenderContext() = default;

		virtual void begin() = 0;
		virtual void end() = 0;

		virtual void transitionResource(FrameBuffer* framebuffer, ResourceState beforeState, ResourceState afterState, bool isDepth) = 0;
		virtual void bindGlobalTextureTable(uint32_t slot) = 0;
		virtual void setTextureHandle(uint32_t slot, void* handle) = 0;
		virtual void unbindBuffers() = 0;
		virtual void draw(uint32_t vertexCount, uint32_t instanceCount = 1, uint32_t startVertex = 0, uint32_t startInstance = 0) = 0;
	};

}
