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

#include <iostream>

#include <glm/glm.hpp>

#include "g_device.h"
#include "g_devmem.h"
#include "g_fence.h"
#include "g_renderpass.h"
#include "g_swapchain.h"
#include "g_window.h"
#include "r_camera.h"
#include "r_model.h"
#include "r_scene.h"
#include "u_debug.h"
#include "r_model_loader.h"

int main(int argc, char *argv[])
{
	LOG_INFO("Starting vulkan application");

	{
		std::shared_ptr<GraphicsWindow> window = std::make_shared<GraphicsWindow>("Vulkan FPS", 800, 800);
		std::shared_ptr<GraphicsDevice> device = std::make_shared<GraphicsDevice>(window);
		std::shared_ptr<GraphicsDevmem> devmem = std::make_shared<GraphicsDevmem>(device);
		std::shared_ptr<GraphicsSwapchain> swapchain = std::make_shared<GraphicsSwapchain>(window, device);

		std::shared_ptr<Camera> main_camera = std::make_shared<Camera>(
            devmem, 
            glm::radians(70.0f), 800.0f / 800.0f, 0.1f, 100.0f, 
            glm::vec3(0.0f, -0.7f, -5.0f), 
            glm::vec3(0.0f, 0.0f, 0.0f), 
            glm::vec3(0.0f, 1.0f, 0.0f)
        );
		Camera::set(main_camera);

		std::vector<vk::CommandBuffer> command_buffers = device->graphics_queue->allocate_command_buffers(swapchain->get_image_count());

        std::shared_ptr<Scene> main_scene = std::make_unique<Scene>(device, devmem);
        Scene::set(main_scene);

        std::shared_ptr<Renderer> renderer = std::make_shared<Renderer>(device, window, devmem, swapchain);

        std::unique_ptr<ModelLoader> model_loader = std::make_unique<ModelLoader>(device, devmem, renderer);

        std::unique_ptr<Model> chalet_model = model_loader->load_model("models/chalet.obj");
        chalet_model->set_position(glm::vec3(0, 0, 0));
        main_scene->add_model(std::move(chalet_model));

 /*       std::unique_ptr<Model> sphere_model1 = model_loader->load_model("models/sphere.obj");
        sphere_model1->set_position(glm::vec3({ 1, 0, 0 }));
        std::unique_ptr<Model> sphere_model2 = model_loader->load_model("models/sphere.obj");
        sphere_model2->set_position(glm::vec3({ -1, 1, 0 })); */

        std::unique_ptr<Model> plane_model1 = model_loader->load_model("models/plane.obj");
        plane_model1->set_position(glm::vec3({ -5, 0, 0 }));
        std::unique_ptr<Model> plane_model2 = model_loader->load_model("models/plane.obj");
        plane_model2->set_position(glm::vec3({ 0, 0, 0 }));
        std::unique_ptr<Model> plane_model3 = model_loader->load_model("models/plane.obj");
        plane_model3->set_position(glm::vec3({ 5, 0, 0 }));
        std::unique_ptr<Model> plane_model4 = model_loader->load_model("models/plane.obj");
        plane_model4->set_position(glm::vec3({ -5, 0, 5 }));
        std::unique_ptr<Model> plane_model5 = model_loader->load_model("models/plane.obj");
        plane_model5->set_position(glm::vec3({ 0, 0, 5 }));
        std::unique_ptr<Model> plane_model6 = model_loader->load_model("models/plane.obj");
        plane_model6->set_position(glm::vec3({ 5, 0, 5 }));
        std::unique_ptr<Model> plane_model7 = model_loader->load_model("models/plane.obj");
        plane_model7->set_position(glm::vec3({ -5, 0, -5 }));
        std::unique_ptr<Model> plane_model8 = model_loader->load_model("models/plane.obj");
        plane_model8->set_position(glm::vec3({ 0, 0, -5 }));
        std::unique_ptr<Model> plane_model9 = model_loader->load_model("models/plane.obj");
        plane_model9->set_position(glm::vec3({ 5, 0, -5 }));

        /* main_scene->add_model(std::move(sphere_model1));
        main_scene->add_model(std::move(sphere_model2));*/
        main_scene->add_model(std::move(plane_model1));
        main_scene->add_model(std::move(plane_model2));
        main_scene->add_model(std::move(plane_model3));
        main_scene->add_model(std::move(plane_model4));
        main_scene->add_model(std::move(plane_model5));
        main_scene->add_model(std::move(plane_model6));
        main_scene->add_model(std::move(plane_model7));
        main_scene->add_model(std::move(plane_model8));
        main_scene->add_model(std::move(plane_model9));

        std::vector<vk::Semaphore> acquire_semaphores;
        std::vector<vk::Semaphore> release_semaphores;
        std::vector<std::unique_ptr<GraphicsFence>> render_fences;

		// Record command buffers
		for (uint32_t i = 0; i < swapchain->get_image_count(); i++)
		{
            acquire_semaphores.push_back(device->create_semaphore());
            release_semaphores.push_back(device->create_semaphore());
            render_fences.emplace_back(std::make_unique<GraphicsFence>(device));

			vk::CommandBuffer cmd = command_buffers[i];
			cmd.begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eSimultaneousUse, nullptr));

            // Main render loop
			renderer->begin_renderpass(cmd, i);
			{
                main_scene->render_models(cmd, i);

				renderer->next_subpass(cmd);

				renderer->render_final_image(cmd, i);
			}
			renderer->end_renderpass(cmd);

			cmd.end();
		}

		LOG_INFO("Setup vulkan application");

		// Main window loop
		while (!window->should_close())
		{
			window->poll_events();
            glm::vec3 movement = glm::vec3();

            if (window->get_key_state(GLFW_KEY_A))
            {
                movement += glm::vec3(1.0f, 0.0f, 0.0f);
            }
            if (window->get_key_state(GLFW_KEY_D))
            {
                movement += glm::vec3(-1.0f, 0.0f, 0.0f);
            }
            if (window->get_key_state(GLFW_KEY_W))
            {
                movement += glm::vec3(0.0f, 0.0f, 1.0f);
            }
            if (window->get_key_state(GLFW_KEY_S))
            {
                movement += glm::vec3(0.0f, 0.0f, -1.0f);
            }

            // glm::vec2 mouse_delta = window->get_mouse_delta();

            movement *= 0.005f;
            main_camera->move(movement);

            if (window->get_key_state(GLFW_KEY_R))
            {
                main_camera->set_position(glm::vec3(0, 0, -5.0f));
            }
            if (window->get_key_state(GLFW_KEY_ESCAPE))
            {
                window->close();
            }

			uint32_t image = swapchain->aquire_image(device->device, acquire_semaphores[0]);

            if (render_fences[image]->get_status() != GraphicsFenceStatus::Reset)
            {
                render_fences[image]->wait();
                render_fences[image]->reset();
            }

			device->graphics_queue->submit_commands(
				{ command_buffers[image] },
				acquire_semaphores[0], release_semaphores[image],
                render_fences[image]
			);

			swapchain->present_image(device->device, image, release_semaphores[image]);
		}
        
        // Finish work before destroying context
        device->device.waitIdle();

        // Cleanup resources created
        for (uint32_t i = 0; i < swapchain->get_image_count(); i++)
        {
            device->device.destroySemaphore(acquire_semaphores[i]);
            device->device.destroySemaphore(release_semaphores[i]);
        }
	}

	LOG_INFO("Destroyed vulkan application");

	while (true);
}
