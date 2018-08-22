/******************************************************************************
* Copyright 2017 James Fitzpatrick <james_fitzpatrick@outlook.com>           *
*                                                                            *
* Permission is hereby granted, free of charge, to any person obtaining a    *
* copy of this software and associated documentation files (the "Software"), *
* to deal in the Software without restriction, including without limitation  *
* the rights to use, copy, modify, merge, publish, distribute, sublicense,   *
* and/or sell copies of the Software, and to permit persons to whom the      *
* Software is furnished to do so, subject to the following conditions:       *
*                                                                            *
* The above copyright notice and this permission notice shall be included in *
* all copies or substantial portions of the Software.                        *
*                                                                            *
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR *
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,   *
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL    *
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER *
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING    *
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER        *
* DEALINGS IN THE SOFTWARE.                                                  *
******************************************************************************/

#include "r_renderer.h"

#include "g_shaderif.h"
#include "r_camera.h"
#include "r_scene.h"
#include "u_debug.h"
#include "u_defines.h"

static vk::DescriptorPool create_descriptor_pool(std::shared_ptr<GraphicsDevice> device, uint32_t set_count, uint32_t uniform_count)
{
    std::vector<vk::DescriptorPoolSize> pool_sizes{
        vk::DescriptorPoolSize(vk::DescriptorType::eUniformBuffer, uniform_count)
    };

    vk::DescriptorPoolCreateInfo create_info(
        vk::DescriptorPoolCreateFlags(0),
        set_count,
        (uint32_t)pool_sizes.size(), pool_sizes.data()
    );

    return device->device.createDescriptorPool(create_info);
}

Renderer::Renderer(std::shared_ptr<GraphicsDevice> device, std::shared_ptr<GraphicsWindow> window, std::shared_ptr<GraphicsDevmem> devmem, std::shared_ptr<GraphicsSwapchain> swapchain)
	: device(device), 
        window(window), 
        devmem(devmem), 
        swapchain(swapchain), 
        renderpass(std::make_shared<GraphicsRenderpass>(device, create_descriptor_pool(device, 8, 8)))
{
	attachments.color = this->create_attachment(vk::Format::eR8G8B8A8Unorm, vk::ImageUsageFlagBits::eColorAttachment, "Color");
	attachments.position = this->create_attachment(vk::Format::eR16G16B16A16Sfloat, vk::ImageUsageFlagBits::eColorAttachment, "Position");
	attachments.normal = this->create_attachment(vk::Format::eR16G16B16A16Sfloat, vk::ImageUsageFlagBits::eColorAttachment, "Normal");
	attachments.depth = this->create_attachment(pick_depth_buffer_format(device), vk::ImageUsageFlagBits::eDepthStencilAttachment, "Depth");

	// Presentation Attachment
	renderpass->add_attachment(vk::AttachmentDescription(
		vk::AttachmentDescriptionFlags(0),
		swapchain->get_swapchain_format().format, vk::SampleCountFlagBits::e1,
		vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore,
		vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare,
		vk::ImageLayout::eUndefined, vk::ImageLayout::ePresentSrcKHR)
	);
	// Color G-Buffer
	renderpass->add_attachment(vk::AttachmentDescription(
		vk::AttachmentDescriptionFlags(0),
		attachments.color.format, vk::SampleCountFlagBits::e1,
		vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore,
		vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare,
		vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal)
	);
	// Position G-Buffer
	renderpass->add_attachment(vk::AttachmentDescription(
		vk::AttachmentDescriptionFlags(0),
		attachments.position.format, vk::SampleCountFlagBits::e1,
		vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore,
		vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare,
		vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal)
	);
	// Normal G-Buffer
	renderpass->add_attachment(vk::AttachmentDescription(
		vk::AttachmentDescriptionFlags(0),
		attachments.normal.format, vk::SampleCountFlagBits::e1,
		vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore,
		vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare,
		vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal)
	);
	// Depth G-Buffer
	renderpass->add_attachment(vk::AttachmentDescription(
		vk::AttachmentDescriptionFlags(0),
		attachments.depth.format, vk::SampleCountFlagBits::e1,
		vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore,
		vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare,
		vk::ImageLayout::eDepthStencilAttachmentOptimal, vk::ImageLayout::eDepthStencilAttachmentOptimal)
	);

    // Render G Buffers
	std::vector<vk::AttachmentReference> subpass_attachments = {
		vk::AttachmentReference(0, vk::ImageLayout::eColorAttachmentOptimal),

		vk::AttachmentReference(1, vk::ImageLayout::eColorAttachmentOptimal),
		vk::AttachmentReference(2, vk::ImageLayout::eColorAttachmentOptimal),
		vk::AttachmentReference(3, vk::ImageLayout::eColorAttachmentOptimal),
	};
	
	vk::AttachmentReference depth_attachment(4, vk::ImageLayout::eDepthStencilAttachmentOptimal);

	renderpass->add_subpass(vk::SubpassDescription(
		vk::SubpassDescriptionFlags(0),
		vk::PipelineBindPoint::eGraphics,
		0, nullptr,
		(uint32_t)subpass_attachments.size(), subpass_attachments.data(), 
		nullptr, &depth_attachment,
		0, nullptr
	));

	// Render lighting
    subpass_attachments = {
        vk::AttachmentReference(0, vk::ImageLayout::eColorAttachmentOptimal),
    };

	renderpass->add_subpass(vk::SubpassDescription(
		vk::SubpassDescriptionFlags(0),
		vk::PipelineBindPoint::eGraphics,
		0, nullptr,
		(uint32_t)subpass_attachments.size(), subpass_attachments.data(),
		nullptr, nullptr,
		0, nullptr
	));

	renderpass->add_subpass_dependency(vk::SubpassDependency(
		VK_SUBPASS_EXTERNAL, 0,
		vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::PipelineStageFlagBits::eColorAttachmentOutput,
		vk::AccessFlags(0), vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite
	));
	renderpass->add_subpass_dependency(vk::SubpassDependency(
		0, 1,
		vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::PipelineStageFlagBits::eFragmentShader,
		vk::AccessFlagBits::eColorAttachmentWrite, vk::AccessFlagBits::eShaderRead
	));

	renderpass->create_renderpass();

    deferred_pipeline = this->create_deffered_pipeline();

    vk::SamplerCreateInfo sampler_create_info(
        vk::SamplerCreateFlags(0),
        vk::Filter::eNearest,
        vk::Filter::eNearest,
        vk::SamplerMipmapMode::eNearest,
        vk::SamplerAddressMode::eClampToEdge,
        vk::SamplerAddressMode::eClampToEdge,
        vk::SamplerAddressMode::eClampToEdge
    );
    this->deferred_sampler = this->device->device.createSampler(sampler_create_info);

    this->update_buffer_descriptor_sets();

	for (uint32_t i = 0; i < swapchain->get_image_count(); i++)
	{
		std::vector<vk::ImageView> views = {
			swapchain->get_image_view(i),
			attachments.color.view,
			attachments.position.view,
			attachments.normal.view,
			attachments.depth.view
		};
		framebuffers.push_back(renderpass->create_framebuffer(device->device, views, swapchain->get_extent()));
	}

	this->create_lighting_pass_resources();
}

Renderer::~Renderer()
{
    device->device.destroyDescriptorPool(this->deferred_descriptor_pool);
    device->device.destroySampler(this->deferred_sampler);
	for (const auto & framebuffer : framebuffers)
	{
		device->device.destroyFramebuffer(framebuffer);
	}
}

void Renderer::begin_renderpass(vk::CommandBuffer cmd, uint32_t image_index)
{
	vk::Rect2D screen_area(vk::Offset2D(0, 0), swapchain->get_extent());
	std::vector<vk::ClearValue> clear_values = { 
		vk::ClearColorValue(std::array<float, 4>{ 0.0f, 0.0f, 0.0f, 0.0f }),
		vk::ClearColorValue(std::array<float, 4>{ 0.0f, 0.0f, 0.0f, 0.0f }),
		vk::ClearColorValue(std::array<float, 4>{ 0.0f, 0.0f, 0.0f, 0.0f }),
		vk::ClearColorValue(std::array<float, 4>{ 0.0f, 0.0f, 0.0f, 0.0f }),
		vk::ClearDepthStencilValue(1.0f, 0)
	};

	renderpass->begin_renderpass(cmd, framebuffers[image_index], screen_area, clear_values, vk::SubpassContents::eSecondaryCommandBuffers);
}

void Renderer::next_subpass(vk::CommandBuffer cmd)
{
	cmd.nextSubpass(vk::SubpassContents::eSecondaryCommandBuffers);
}

void Renderer::end_renderpass(vk::CommandBuffer cmd) const
{
	renderpass->end_renderpass(cmd);
}

std::vector<vk::CommandBuffer> Renderer::alloc_render_command_buffers() const
{
	return device->graphics_queue->allocate_command_buffers(swapchain->get_image_count(), vk::CommandBufferLevel::eSecondary);
}

void Renderer::start_secondary_command_buffer(vk::CommandBuffer cmd, uint32_t index, uint32_t subpass) const
{
	vk::CommandBufferInheritanceInfo inheritance_info(
		(vk::RenderPass) *renderpass.get(),
		subpass,
		framebuffers[index]
	);

	vk::CommandBufferBeginInfo begin_info(
		vk::CommandBufferUsageFlagBits::eSimultaneousUse | vk::CommandBufferUsageFlagBits::eRenderPassContinue,
		&inheritance_info
	);

	cmd.begin(begin_info);
}

void Renderer::end_secondary_command_buffer(vk::CommandBuffer cmd) const
{
	cmd.end();
}

void Renderer::render_final_image(const vk::CommandBuffer& cmd, uint32_t index)
{
	cmd.executeCommands(command_buffers[index]);
}

std::shared_ptr<GraphicsRenderpass> Renderer::get_renderpass() const
{
	return renderpass;
}

RenderAttachment Renderer::create_attachment(vk::Format format, vk::ImageUsageFlags usage, std::string attachment_name) const
{
	RenderAttachment attachment;
	vk::ImageAspectFlags aspect_mask;
	vk::ImageLayout layout = vk::ImageLayout::eUndefined;

    LOG_INFO("Creating render attachment '%s'", attachment_name.c_str());

	attachment.format = format;

	if (BITMASK_HAS(usage, vk::ImageUsageFlagBits::eColorAttachment))
	{
		aspect_mask = vk::ImageAspectFlagBits::eColor;
		layout = vk::ImageLayout::eColorAttachmentOptimal;
	}
	if (BITMASK_HAS(usage, vk::ImageUsageFlagBits::eDepthStencilAttachment))
	{
		aspect_mask = vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil;
		layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
	}

    DEBUG_ASSERT(layout != vk::ImageLayout::eUndefined);
	DEBUG_ASSERT((VkFlags) aspect_mask > 0);

	vk::ImageCreateInfo image_create_info(
		vk::ImageCreateFlags(0),
		vk::ImageType::e2D,
		format,
		vk::Extent3D(window->get_window_size().width, window->get_window_size().height, 1),
		1,
		1,
		vk::SampleCountFlagBits::e1,
		vk::ImageTiling::eOptimal,
		usage | vk::ImageUsageFlagBits::eInputAttachment | vk::ImageUsageFlagBits::eSampled,
		vk::SharingMode::eExclusive,
		0, nullptr,
		vk::ImageLayout::eUndefined
	);

	VmaAllocationCreateInfo alloc_create_info{};
	alloc_create_info.flags = 0;
	alloc_create_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	alloc_create_info.pUserData = STRING_TO_DATA(("Render Attachment: " + attachment_name).c_str());

	attachment.image = devmem->create_image(image_create_info, alloc_create_info);

	vk::ImageViewCreateInfo image_view_create_info(
		vk::ImageViewCreateFlags(0),
        attachment.image->image,
		vk::ImageViewType::e2D,
		format,
		vk::ComponentMapping(),
		vk::ImageSubresourceRange(
			aspect_mask,
			0, 1,
			0, 1
		)
	);

	attachment.view = attachment.image->create_image_view(image_view_create_info);

    if (layout == vk::ImageLayout::eDepthStencilAttachmentOptimal)
    {
        attachment.image->transition_layout(layout);
    }

    LOG_INFO("Created render attachment '%s' successfully", attachment_name.c_str());

	return attachment;
}

void Renderer::update_buffer_descriptor_sets() const
{
    vk::DescriptorBufferInfo camera_buffer = Camera::get()->get_buffer_info();
    vk::DescriptorBufferInfo light_buffer = Scene::get()->get_light_data_info();

    vk::DescriptorImageInfo color_buffer_info(
        this->deferred_sampler,
        this->attachments.color.view,
        vk::ImageLayout::eColorAttachmentOptimal
    );
    vk::DescriptorImageInfo position_buffer_info(
        this->deferred_sampler,
        this->attachments.position.view,
        vk::ImageLayout::eColorAttachmentOptimal
    );
    vk::DescriptorImageInfo normal_buffer_info(
        this->deferred_sampler,
        this->attachments.normal.view,
        vk::ImageLayout::eColorAttachmentOptimal
    );

    std::vector<vk::WriteDescriptorSet> writes = 
    {
        vk::WriteDescriptorSet(
            vk::DescriptorSet(),
            0,
            0,
            1,
            vk::DescriptorType::eUniformBuffer,
            nullptr,
            &camera_buffer,
            nullptr
        ),
        vk::WriteDescriptorSet(
            vk::DescriptorSet(),
            1,
            0,
            1,
            vk::DescriptorType::eUniformBuffer,
            nullptr,
            &light_buffer,
            nullptr
        ),
        vk::WriteDescriptorSet(
            vk::DescriptorSet(),
            2, 
            0,
            1,
            vk::DescriptorType::eCombinedImageSampler,
            &color_buffer_info,
            nullptr,
            nullptr
        ),
        vk::WriteDescriptorSet(
            vk::DescriptorSet(),
            3,
            0,
            1,
            vk::DescriptorType::eCombinedImageSampler,
            &position_buffer_info,
            nullptr,
            nullptr
        ),
        vk::WriteDescriptorSet(
            vk::DescriptorSet(),
            4,
            0,
            1,
            vk::DescriptorType::eCombinedImageSampler,
            &normal_buffer_info,
            nullptr,
            nullptr
        )
    };

    this->deferred_pipeline->update_descriptor_sets(writes);
}

void Renderer::create_lighting_pass_resources()
{
	vk::GraphicsPipelineCreateInfo pipeline_create_info;
    
    vk::BufferCreateInfo buffer_create_info{
        vk::BufferCreateFlags(0),
        6 * sizeof(Vertex),
        vk::BufferUsageFlagBits::eVertexBuffer,
        vk::SharingMode::eExclusive
    };

    VmaAllocationCreateInfo buffer_alloc_info{};
    buffer_alloc_info.flags = 0;
    buffer_alloc_info.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
    buffer_alloc_info.pUserData = STRING_TO_DATA("Screen Vertex Buffer");

    this->screen_vertex_buffer = this->devmem->create_buffer(buffer_create_info, buffer_alloc_info);

    Vertex *data;
    screen_vertex_buffer->map_memory((void**)&data);
    data[0] = Vertex(glm::vec3(1.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), glm::vec2(1.0f, 1.0f));
    data[1] = Vertex(glm::vec3(-1.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 0.0f));
    data[2] = Vertex(glm::vec3(-1.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 1.0f));

    data[3] = Vertex(glm::vec3(1.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), glm::vec2(1.0f, 1.0f));
    data[4] = Vertex(glm::vec3(1.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), glm::vec2(1.0f, 0.0f));
    data[5] = Vertex(glm::vec3(-1.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 0.0f));
    screen_vertex_buffer->unmap_memory();

	// Create lighting pass command buffers
	command_buffers = device->graphics_queue->allocate_command_buffers(swapchain->get_image_count(), vk::CommandBufferLevel::eSecondary);

	for (uint32_t i = 0; i < swapchain->get_image_count(); i++)
	{
		vk::CommandBufferInheritanceInfo inheritance_info(
			(vk::RenderPass) *renderpass,
			1,
			framebuffers[i]
		);

		vk::CommandBufferBeginInfo begin_info(
			vk::CommandBufferUsageFlagBits::eSimultaneousUse | vk::CommandBufferUsageFlagBits::eRenderPassContinue,
			&inheritance_info
		);

		command_buffers[i].begin(begin_info);
        this->deferred_pipeline->bind_pipeline(command_buffers[i]);

        std::vector<vk::Buffer> vbufs{ screen_vertex_buffer->buffer };
        std::vector<vk::DeviceSize> voffsets{ 0 };
        command_buffers[i].bindVertexBuffers(0, (uint32_t)vbufs.size(), vbufs.data(), voffsets.data());

        command_buffers[i].draw(6, 1, 0, 0);
		command_buffers[i].end();
	}
}

std::unique_ptr<GraphicsPipeline> Renderer::create_deffered_pipeline()
{
    auto create_info = std::make_unique<GraphicsPipelineCreateInfo>(this->device, "shaders/deferred.vert", "shaders/deferred.frag");
    create_info->viewports = { vk::Viewport(0, 0, (float) this->swapchain->get_extent().width,  (float) this->swapchain->get_extent().height) };
    create_info->scissors = { vk::Rect2D(vk::Offset2D(0, 0), this->swapchain->get_extent()) };

    std::vector<vk::DescriptorPoolSize> pool_sizes{
        vk::DescriptorPoolSize(vk::DescriptorType::eUniformBuffer, 2),
        vk::DescriptorPoolSize(vk::DescriptorType::eCombinedImageSampler, 3)
    };

    vk::DescriptorPoolCreateInfo descriptor_pool_create_info(
        vk::DescriptorPoolCreateFlags(0),
        1,
        (uint32_t)pool_sizes.size(), pool_sizes.data()
    );

    this->deferred_descriptor_pool = device->device.createDescriptorPool(descriptor_pool_create_info);

    std::vector<vk::DescriptorSetLayoutBinding> set_bindings = {
        vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex),
        vk::DescriptorSetLayoutBinding(1, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eFragment),
        vk::DescriptorSetLayoutBinding(2, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment),
        vk::DescriptorSetLayoutBinding(3, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment),
        vk::DescriptorSetLayoutBinding(4, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment),
    };

    vk::DescriptorSetLayoutCreateInfo set_create_info(
        vk::DescriptorSetLayoutCreateFlags(0),
        (uint32_t)set_bindings.size(), set_bindings.data()
    );

    create_info->descriptor_set_layout = device->device.createDescriptorSetLayout(set_create_info);

    vk::DescriptorSetAllocateInfo descriptor_set_alloc_info(
        deferred_descriptor_pool,
        1, &create_info->descriptor_set_layout
    );

    create_info->descriptor_set = device->device.allocateDescriptorSets(descriptor_set_alloc_info)[0];

    std::vector<vk::DescriptorSetLayout> descriptor_sets{
        create_info->descriptor_set_layout
    };

    vk::PipelineLayoutCreateInfo pipeline_layout_create_info(
        vk::PipelineLayoutCreateFlags(0),
        (uint32_t)descriptor_sets.size(), descriptor_sets.data(),
        0, nullptr
    );

    create_info->pipeline_layout = device->device.createPipelineLayout(pipeline_layout_create_info);

    create_info->color_attachments = {
        // Final Presented Image
        vk::PipelineColorBlendAttachmentState(
            false,
            vk::BlendFactor::eOne, vk::BlendFactor::eZero, vk::BlendOp::eAdd,
            vk::BlendFactor::eOne, vk::BlendFactor::eZero, vk::BlendOp::eAdd,
            vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
        )
    };

    create_info->subpass = 1;

    create_info->depth_enable = false;

    return this->renderpass->create_pipeline(std::move(create_info));
}

vk::Format Renderer::pick_depth_buffer_format(std::shared_ptr<GraphicsDevice> device)
{
	return vk::Format::eD24UnormS8Uint;
}


