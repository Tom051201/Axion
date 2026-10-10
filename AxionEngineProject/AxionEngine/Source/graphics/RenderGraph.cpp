#include "axpch.h"
#include "RenderGraph.h"

#include "AxionEngine/Source/core/JobSystem.h"
#include "AxionEngine/Platform/directx12/DX12Context.h"

namespace Axion {

	////////////////////////////////////////////////////////////////////////////////
	///// RGPassBuilder ////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	void RGPassBuilder::read(RGResourceID id, ResourceState state) {
		m_pass->inputs.push_back({ id, state });
	}

	void RGPassBuilder::writeColor(RGResourceID id) {
		m_pass->outputs.push_back({ id, ResourceState::RenderTarget });
	}

	void RGPassBuilder::writeDepth(RGResourceID id) {
		m_pass->outputs.push_back({ id, ResourceState::DepthWrite });
	}

	void RGPassBuilder::write(RGResourceID id, ResourceState state) {
		m_pass->outputs.push_back({ id, state });
	}

	////////////////////////////////////////////////////////////////////////////////
	///// RenderGraph //////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	RenderGraph::~RenderGraph() {
		for (auto* pass : m_passes) {
			delete pass;
		}
		m_passes.clear();
	}

	RGResourceID RenderGraph::importResource(const std::string& name, FrameBuffer* framebuffer, ResourceState currentState) {
		for (auto& res : m_resources) {
			if (res.name == name && res.isImported) {
				res.importedFrameBuffer = framebuffer;
				res.initialState = currentState;
				res.currentState = currentState;
				return res.id;
			}
		}

		RGResourceID id = m_nextResourceId++;
		RGResource resource;
		resource.id = id;
		resource.name = name;
		resource.isImported = true;
		resource.importedFrameBuffer = framebuffer;
		resource.initialState = currentState;
		resource.currentState = currentState;

		m_resources.push_back(resource);
		return id;
	}

	RGResourceID RenderGraph::createResource(const std::string& name, const RGTextureDesc& desc) {
		for (auto& res : m_resources) {
			if (res.name == name && !res.isImported) {
				res.desc = desc;
				res.initialState = ResourceState::PixelShaderResource;
				res.currentState = ResourceState::PixelShaderResource;
				return res.id;
			}
		}

		RGResourceID id = m_nextResourceId++;
		RGResource resource;
		resource.id = id;
		resource.name = name;
		resource.isImported = false;
		resource.desc = desc;
		resource.initialState = ResourceState::PixelShaderResource;
		resource.currentState = ResourceState::PixelShaderResource;

		m_resources.push_back(resource);
		return id;
	}

	RGResource* RenderGraph::getResource(RGResourceID id) {
		for (auto& res : m_resources) {
			if (res.id == id) return &res;
		}
		return nullptr;
	}

	void RenderGraph::compile() {
		// -- Reset all resource states to their initial states --
		for (auto& res : m_resources) {
			res.currentState = res.initialState;
		}

		// -- Traverse every pass in the graph --
		for (auto* pass : m_passes) {
			pass->prePassBarriers.clear();

			// -- Process Inputs --
			for (const auto& input : pass->inputs) {
				RGResource* res = getResource(input.id);
				if (!res) continue;
				if (res->currentState != input.state) {
					pass->prePassBarriers.push_back({ res->id, res->currentState, input.state });
					res->currentState = input.state;
				}
			}

			// -- Process Outputs --
			for (const auto& output : pass->outputs) {
				RGResource* res = getResource(output.id);
				if (!res) continue;
				if (res->currentState != output.state) {
					pass->prePassBarriers.push_back({ res->id, res->currentState, output.state });
					res->currentState = output.state;
				}
			}
		}

		// -- Transient Resource Allocation and Pooling --
		for (auto& res : m_resources) {
			if (!res.isImported) {
				bool needsAllocation = false;

				if (!res.transientFrameBuffer) {
					needsAllocation = true;
				}
				else {
					const auto& spec = res.transientFrameBuffer->getSpecification();
					if (spec.width != res.desc.width || spec.height != res.desc.height ||
						spec.textureFormat != res.desc.format || spec.depthStencilFormat != res.desc.depthFormat) {
						needsAllocation = true;
					}
				}

				if (needsAllocation) {
					FrameBufferSpecification spec;
					spec.width = std::max(1u, res.desc.width);
					spec.height = std::max(1u, res.desc.height);
					spec.textureFormat = res.desc.format;
					spec.depthStencilFormat = res.desc.depthFormat;
					spec.clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
					spec.useEntityIDAttachment = true;

					res.transientFrameBuffer = FrameBuffer::create(spec);
					AX_CORE_LOG_INFO("RenderGraph allocated transient resource: {0} ({1}x{2})", res.name, spec.width, spec.height);
				}
			}
		}
	}

	void RenderGraph::execute() {
		std::vector<std::function<void(uint32_t)>> renderJobs;
		uint32_t contextWorkerIndex = 1;

		for (auto pass : m_passes) {
			if (pass->isCulled) continue;
			RenderContext* context = GraphicsContext::get()->acquireThreadContext(contextWorkerIndex++);

			renderJobs.push_back([this, pass, context](uint32_t threadId) {
				if (!pass->prePassBarriers.empty()) {
					for (const auto& barrier : pass->prePassBarriers) {
						RGResource* res = getResource(barrier.resourceId);
						if (!res) continue;

						FrameBuffer* physicalFb = getPhysicalFramebuffer(barrier.resourceId);
						if (!physicalFb) continue;

						bool isDepth = (barrier.afterState == ResourceState::DepthWrite || barrier.afterState == ResourceState::DepthRead);

						context->transitionResource(physicalFb, barrier.beforeState, barrier.afterState, isDepth);
					}
				}

				// -- Execute Actual GPU Work --
				if (pass->executeCallback) {
					pass->executeCallback(context);
				}

				context->end();
			});
		}

		JobSystem::executeAndWait(renderJobs);
	}

	FrameBuffer* RenderGraph::getPhysicalFramebuffer(RGResourceID id) {
		RGResource* res = getResource(id);
		if (!res) return nullptr;
		return res->isImported ? res->importedFrameBuffer : res->transientFrameBuffer.get();
	}

	void RenderGraph::clear() {
		for (auto* pass : m_passes) {
			delete pass;
		}
		m_passes.clear();
	}

}
