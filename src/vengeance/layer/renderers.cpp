#include "renderers.h"
#include "components.h"

namespace vitex
{
	namespace layer
	{
		namespace renderers
		{
			soft_body::soft_body(layer::render_system* lab) : geometry_renderer(lab), vertex_buffer(nullptr), index_buffer(nullptr)
			{
				VI_ASSERT(system != nullptr, "render system should be set");
				VI_ASSERT(system->get_device() != nullptr, "graphics device should be set");

				graphics::graphics_device* device = system->get_device();
				depth_stencil = device->get_depth_stencil_state("drw_srw_lt");
				rasterizer = device->get_rasterizer_state("so_co");
				blend = device->get_blend_state("bo_wrgba_one");
				sampler = device->get_sampler_state("a16_fa_wrap");
				layout[0] = device->get_input_layout("vx_base");
				layout[1] = device->get_input_layout("vxi_base");

				pipelines.culling.shader = *system->compile_shader("materials/material_model_culling", { });
				pipelines.culling.object_buffer = *device->get_shader_slot(pipelines.culling.shader, "ObjectBuffer");
				pipelines.depth.shader = *system->compile_shader("materials/material_model_depth", { });
				pipelines.depth.slotdata.diffuse_map = *device->get_shader_slot(pipelines.depth.shader, "DiffuseMap");
				pipelines.depth.sampler = *device->get_shader_sampler_slot(pipelines.depth.shader, "DiffuseMap", "Sampler");
				pipelines.depth.materials = *device->get_shader_slot(pipelines.depth.shader, "Materials");
				pipelines.depth.object_buffer = *device->get_shader_slot(pipelines.depth.shader, "ObjectBuffer");
				pipelines.depth_cube.shader = *system->compile_shader("materials/material_model_depth_cube", { }, sizeof(trigonometry::matrix4x4) * 6);
				pipelines.depth_cube.slotdata.diffuse_map = *device->get_shader_slot(pipelines.depth_cube.shader, "DiffuseMap");
				pipelines.depth_cube.sampler = *device->get_shader_sampler_slot(pipelines.depth_cube.shader, "DiffuseMap", "Sampler");
				pipelines.depth_cube.materials = *device->get_shader_slot(pipelines.depth_cube.shader, "Materials");
				pipelines.depth_cube.object_buffer = *device->get_shader_slot(pipelines.depth_cube.shader, "ObjectBuffer");
				pipelines.depth_cube.viewer_buffer = *device->get_shader_slot(pipelines.depth_cube.shader, "ViewerBuffer");
				pipelines.depth_cube.cube_buffer = *device->get_shader_slot(pipelines.depth_cube.shader, "CubeBuffer");
				pipelines.geometry.shader = *system->compile_shader("materials/material_model_geometry", { });
				pipelines.geometry.slotdata.diffuse_map = *device->get_shader_slot(pipelines.geometry.shader, "DiffuseMap");
				pipelines.geometry.slotdata.normal_map = *device->get_shader_slot(pipelines.geometry.shader, "NormalMap");
				pipelines.geometry.slotdata.metallic_map = *device->get_shader_slot(pipelines.geometry.shader, "MetallicMap");
				pipelines.geometry.slotdata.roughness_map = *device->get_shader_slot(pipelines.geometry.shader, "RoughnessMap");
				pipelines.geometry.slotdata.height_map = *device->get_shader_slot(pipelines.geometry.shader, "HeightMap");
				pipelines.geometry.slotdata.occlusion_map = *device->get_shader_slot(pipelines.geometry.shader, "OcclusionMap");
				pipelines.geometry.slotdata.emission_map = *device->get_shader_slot(pipelines.geometry.shader, "EmissionMap");
				pipelines.geometry.sampler = *device->get_shader_sampler_slot(pipelines.geometry.shader, "DiffuseMap", "Sampler");
				pipelines.geometry.materials = *device->get_shader_slot(pipelines.geometry.shader, "Materials");
				pipelines.geometry.object_buffer = *device->get_shader_slot(pipelines.geometry.shader, "ObjectBuffer");
				pipelines.geometry.viewer_buffer = *device->get_shader_slot(pipelines.geometry.shader, "ViewerBuffer");

				graphics::element_buffer* buffers[2];
				if (lab->compile_buffers(buffers, "soft-body", sizeof(trigonometry::vertex), 16384))
				{
					index_buffer = buffers[(size_t)buffer_type::index];
					vertex_buffer = buffers[(size_t)buffer_type::vertex];
				}

				graphics::element_buffer::desc desc = graphics::element_buffer::desc();
				desc.access_flags = graphics::cpu_access::write;
				desc.usage = graphics::resource_usage::dynamic;
				desc.bind_flags = graphics::resource_bind::vertex_buffer;
				desc.element_count = 1;
				desc.elements = (void*)&group;
				desc.element_width = sizeof(render_buffer::instance);
				group_buffer = device->create_element_buffer(desc).or_else(nullptr);
			}
			soft_body::~soft_body()
			{
				graphics::element_buffer* buffers[2];
				buffers[(size_t)buffer_type::index] = index_buffer;
				buffers[(size_t)buffer_type::vertex] = vertex_buffer;
				core::memory::release(group_buffer);

				system->free_buffers(buffers);
				system->free_shader(pipelines.geometry.shader);
				system->free_shader(pipelines.culling.shader);
				system->free_shader(pipelines.depth.shader);
				system->free_shader(pipelines.depth_cube.shader);
			}
			size_t soft_body::cull_geometry(const viewer& view, const geometry_renderer::objects& chunk)
			{
				VI_ASSERT(system->get_primitives() != nullptr, "primitives cache should be set");

				graphics::element_buffer* box[2];
				system->get_primitives()->get_box_buffers(box);

				graphics::graphics_device* device = system->get_device();
				system->set_constant_buffer(render_buffer_type::render, pipelines.culling.object_buffer, VI_VS);
				device->set_rasterizer_state(rasterizer);
				device->set_input_layout(layout[0]);
				device->set_shader(nullptr, VI_PS);
				device->set_shader(pipelines.culling.shader, VI_VS);

				size_t count = 0;
				if (system->precise_culling)
				{
					for (auto* base : chunk)
					{
						if (base->get_indices().empty() || !culling_begin(base))
							continue;

						base->fill(device, index_buffer, vertex_buffer);
						system->constants->render.world.identify();
						system->constants->render.transform = view.view_projection;
						system->update_constant_buffer(render_buffer_type::render);
						device->set_vertex_buffer(vertex_buffer);
						device->set_index_buffer(index_buffer, graphics::format::r32_uint);
						device->draw_indexed((uint32_t)base->get_indices().size(), 0, 0);
						culling_end();
						count++;
					}
				}
				else
				{
					device->set_vertex_buffer(box[(size_t)buffer_type::vertex]);
					device->set_index_buffer(box[(size_t)buffer_type::index], graphics::format::r32_uint);

					for (auto* base : chunk)
					{
						if (base->get_indices().empty() || !culling_begin(base))
							continue;

						system->constants->render.world = base->get_entity()->get_box();
						system->constants->render.transform = system->constants->render.world * view.view_projection;
						system->update_constant_buffer(render_buffer_type::render);
						device->draw_indexed((uint32_t)box[(size_t)buffer_type::index]->get_elements(), 0, 0);
						culling_end();

						count++;
					}
				}

				return count;
			}
			size_t soft_body::render_geometry(core::timer* time, const geometry_renderer::objects& chunk)
			{
				VI_ASSERT(system->get_scene() != nullptr, "scene should be set");
				graphics::graphics_device* device = system->get_device();
				bool constant = system->state.is_set(render_opt::constant);

				system->set_constant_buffer(render_buffer_type::render, pipelines.geometry.object_buffer, VI_VS | VI_PS);
				system->set_constant_buffer(render_buffer_type::view, pipelines.geometry.viewer_buffer, VI_VS | VI_PS);
				device->set_structure_buffer(system->get_material_buffer(), pipelines.geometry.materials, VI_PS);
				device->set_depth_stencil_state(depth_stencil);
				device->set_blend_state(blend);
				device->set_rasterizer_state(rasterizer);
				device->set_input_layout(layout[1]);
				device->set_sampler_state(sampler, pipelines.geometry.sampler, 7, VI_PS);
				device->set_shader(pipelines.geometry.shader, VI_VS | VI_PS);

				graphics::element_buffer* group_vertex_buffers[2];
				group_vertex_buffers[0] = vertex_buffer;
				group_vertex_buffers[1] = group_buffer;
				device->set_vertex_buffers(group_vertex_buffers, sizeof(group_vertex_buffers) / sizeof(*group_vertex_buffers));

				size_t count = 0;
				for (auto* base : chunk)
				{
					if ((constant && !base->constant) || base->get_indices().empty())
						continue;

					if (!system->try_geometry(base->get_material(), &pipelines.geometry.slotdata))
						continue;

					group.world.identify();
					group.transform = system->view.view_projection;
					group.texcoord = base->texcoord;
					base->fill(device, index_buffer, vertex_buffer);
					device->update_buffer(group_buffer, &group, sizeof(group));
					device->set_index_buffer(index_buffer, graphics::format::r32_uint);
					device->draw_indexed_instanced((uint32_t)base->get_indices().size(), 1, 0, 0, 0);
					count++;
				}

				static graphics::element_buffer* vertex_buffers[2] = { nullptr, nullptr };
				device->set_vertex_buffers(vertex_buffers, sizeof(vertex_buffers) / sizeof(*vertex_buffers));
				return count;
			}
			size_t soft_body::render_depth(core::timer* time, const geometry_renderer::objects& chunk)
			{
				VI_ASSERT(system->get_scene() != nullptr, "scene should be set");
				graphics::graphics_device* device = system->get_device();
				system->set_constant_buffer(render_buffer_type::render, pipelines.depth.object_buffer, VI_VS | VI_PS | VI_GS);
				device->set_structure_buffer(system->get_material_buffer(), pipelines.depth.materials, VI_PS);
				device->set_depth_stencil_state(depth_stencil);
				device->set_blend_state(blend);
				device->set_rasterizer_state(rasterizer);
				device->set_input_layout(layout[1]);
				device->set_sampler_state(sampler, pipelines.depth.sampler, 1, VI_PS);
				device->set_shader(pipelines.depth.shader, VI_VS | VI_PS);

				graphics::element_buffer* group_vertex_buffers[2];
				group_vertex_buffers[0] = vertex_buffer;
				group_vertex_buffers[1] = group_buffer;
				device->set_vertex_buffers(group_vertex_buffers, sizeof(group_vertex_buffers) / sizeof(*group_vertex_buffers));

				size_t count = 0;
				for (auto* base : chunk)
				{
					if (base->get_indices().empty())
						continue;

					if (!system->try_geometry(base->get_material(), &pipelines.depth.slotdata))
						continue;

					group.world.identify();
					group.transform = system->view.view_projection;
					group.texcoord = base->texcoord;
					base->fill(device, index_buffer, vertex_buffer);
					device->update_buffer(group_buffer, &group, sizeof(group));
					device->set_index_buffer(index_buffer, graphics::format::r32_uint);
					device->draw_indexed_instanced((uint32_t)base->get_indices().size(), 1, 0, 0, 0);

					count++;
				}

				static graphics::element_buffer* vertex_buffers[2] = { nullptr, nullptr };
				device->set_vertex_buffers(vertex_buffers, sizeof(vertex_buffers) / sizeof(*vertex_buffers));
				return count;
			}
			size_t soft_body::render_depth_cube(core::timer* time, const geometry_renderer::objects& chunk, trigonometry::matrix4x4* view_projection)
			{
				VI_ASSERT(system->get_scene() != nullptr, "scene should be set");
				graphics::graphics_device* device = system->get_device();
				system->set_constant_buffer(render_buffer_type::render, pipelines.depth_cube.object_buffer, VI_VS | VI_PS | VI_GS);
				system->set_constant_buffer(render_buffer_type::view, pipelines.depth_cube.viewer_buffer, VI_VS | VI_PS | VI_GS);
				device->set_structure_buffer(system->get_material_buffer(), pipelines.depth_cube.materials, VI_PS);
				device->set_depth_stencil_state(depth_stencil);
				device->set_blend_state(blend);
				device->set_rasterizer_state(rasterizer);
				device->set_input_layout(layout[1]);
				device->set_sampler_state(sampler, pipelines.depth_cube.sampler, 1, VI_PS);
				device->set_shader(pipelines.depth_cube.shader, VI_VS | VI_PS | VI_GS);
				device->set_buffer(pipelines.depth_cube.shader, pipelines.depth_cube.cube_buffer, VI_VS | VI_PS | VI_GS);
				device->update_buffer(pipelines.depth_cube.shader, view_projection);

				graphics::element_buffer* group_vertex_buffers[2];
				group_vertex_buffers[0] = vertex_buffer;
				group_vertex_buffers[1] = group_buffer;
				device->set_vertex_buffers(group_vertex_buffers, sizeof(group_vertex_buffers) / sizeof(*group_vertex_buffers));

				size_t count = 0;
				for (auto* base : chunk)
				{
					if (!base->get_body())
						continue;

					if (!system->try_geometry(base->get_material(), &pipelines.depth_cube.slotdata))
						continue;

					group.world.identify();
					group.texcoord = base->texcoord;
					base->fill(device, index_buffer, vertex_buffer);
					device->update_buffer(group_buffer, &group, sizeof(group));
					device->set_index_buffer(index_buffer, graphics::format::r32_uint);
					device->draw_indexed_instanced((uint32_t)base->get_indices().size(), 1, 0, 0, 0);

					count++;
				}

				static graphics::element_buffer* vertex_buffers[2] = { nullptr, nullptr };
				device->set_vertex_buffers(vertex_buffers, sizeof(vertex_buffers) / sizeof(*vertex_buffers));
				device->set_shader(nullptr, VI_GS);
				return count;
			}

			model::model(layer::render_system* lab) : geometry_renderer(lab)
			{
				VI_ASSERT(system != nullptr, "render system should be set");
				VI_ASSERT(system->get_device() != nullptr, "graphics device should be set");

				graphics::graphics_device* device = system->get_device();
				depth_stencil = device->get_depth_stencil_state("drw_srw_lt");
				back_rasterizer = device->get_rasterizer_state("so_cback");
				front_rasterizer = device->get_rasterizer_state("so_cfront");
				blend = device->get_blend_state("bo_wrgba_one");
				sampler = device->get_sampler_state("a16_fa_wrap");
				layout[0] = device->get_input_layout("vx_base");
				layout[1] = device->get_input_layout("vxi_base");

				pipelines.culling.shader = *system->compile_shader("materials/material_model_culling", { });
				pipelines.culling.object_buffer = *device->get_shader_slot(pipelines.culling.shader, "ObjectBuffer");
				pipelines.depth.shader = *system->compile_shader("materials/material_model_depth", { });
				pipelines.depth.slotdata.diffuse_map = *device->get_shader_slot(pipelines.depth.shader, "DiffuseMap");
				pipelines.depth.sampler = *device->get_shader_sampler_slot(pipelines.depth.shader, "DiffuseMap", "Sampler");
				pipelines.depth.materials = *device->get_shader_slot(pipelines.depth.shader, "Materials");
				pipelines.depth_cube.shader = *system->compile_shader("materials/material_model_depth_cube", { }, sizeof(trigonometry::matrix4x4) * 6);
				pipelines.depth_cube.slotdata.diffuse_map = *device->get_shader_slot(pipelines.depth_cube.shader, "DiffuseMap");
				pipelines.depth_cube.sampler = *device->get_shader_sampler_slot(pipelines.depth_cube.shader, "DiffuseMap", "Sampler");
				pipelines.depth_cube.materials = *device->get_shader_slot(pipelines.depth_cube.shader, "Materials");
				pipelines.depth_cube.viewer_buffer = *device->get_shader_slot(pipelines.depth_cube.shader, "ViewerBuffer");
				pipelines.depth_cube.cube_buffer = *device->get_shader_slot(pipelines.depth_cube.shader, "CubeBuffer");
				pipelines.geometry.shader = *system->compile_shader("materials/material_model_geometry", { });
				pipelines.geometry.slotdata.diffuse_map = *device->get_shader_slot(pipelines.geometry.shader, "DiffuseMap");
				pipelines.geometry.slotdata.normal_map = *device->get_shader_slot(pipelines.geometry.shader, "NormalMap");
				pipelines.geometry.slotdata.metallic_map = *device->get_shader_slot(pipelines.geometry.shader, "MetallicMap");
				pipelines.geometry.slotdata.roughness_map = *device->get_shader_slot(pipelines.geometry.shader, "RoughnessMap");
				pipelines.geometry.slotdata.height_map = *device->get_shader_slot(pipelines.geometry.shader, "HeightMap");
				pipelines.geometry.slotdata.occlusion_map = *device->get_shader_slot(pipelines.geometry.shader, "OcclusionMap");
				pipelines.geometry.slotdata.emission_map = *device->get_shader_slot(pipelines.geometry.shader, "EmissionMap");
				pipelines.geometry.sampler = *device->get_shader_sampler_slot(pipelines.geometry.shader, "DiffuseMap", "Sampler");
				pipelines.geometry.materials = *device->get_shader_slot(pipelines.geometry.shader, "Materials");
				pipelines.geometry.viewer_buffer = *device->get_shader_slot(pipelines.geometry.shader, "ViewerBuffer");
			}
			model::~model()
			{
				system->free_shader(pipelines.geometry.shader);
				system->free_shader(pipelines.culling.shader);
				system->free_shader(pipelines.depth.shader);
				system->free_shader(pipelines.depth_cube.shader);
			}
			void model::batch_geometry(components::model* base, geometry_renderer::batching& batch, size_t chunk)
			{
				auto* drawable = get_drawable(base);
				if (!base->constant && !system->state.is_set(render_opt::constant))
					return;

				render_buffer::instance data;
				data.texcoord = base->texcoord;

				auto& world = base->get_entity()->get_box();
				for (auto* mesh : drawable->meshes)
				{
					material* source = base->get_material(mesh);
					if (system->try_instance(source, data))
					{
						data.world = mesh->transform * world;
						data.transform = data.world * system->view.view_projection;
						batch.emplace(mesh, source, data, chunk);
					}
				}
			}
			size_t model::cull_geometry(const viewer& view, const geometry_renderer::objects& chunk)
			{
				VI_ASSERT(system->get_primitives() != nullptr, "primitive cache should be set");

				graphics::element_buffer* box[2];
				system->get_primitives()->get_box_buffers(box);

				graphics::graphics_device* device = system->get_device();
				system->set_constant_buffer(render_buffer_type::render, pipelines.culling.object_buffer, VI_VS);
				device->set_rasterizer_state(back_rasterizer);
				device->set_input_layout(layout[0]);
				device->set_shader(nullptr, VI_PS);
				device->set_shader(pipelines.culling.shader, VI_VS);

				size_t count = 0;
				if (system->precise_culling)
				{
					for (auto* base : chunk)
					{
						if (!culling_begin(base))
							continue;

						auto* drawable = get_drawable(base);
						auto& world = base->get_entity()->get_box();
						for (auto* mesh : drawable->meshes)
						{
							system->constants->render.world = mesh->transform * world;
							system->constants->render.transform = system->constants->render.world * view.view_projection;
							system->update_constant_buffer(render_buffer_type::render);
							device->draw_indexed(mesh);
						}

						culling_end();
						count++;
					}
				}
				else
				{
					device->set_vertex_buffer(box[(size_t)buffer_type::vertex]);
					device->set_index_buffer(box[(size_t)buffer_type::index], graphics::format::r32_uint);
					for (auto* base : chunk)
					{
						if (!culling_begin(base))
							continue;

						system->constants->render.world = base->get_entity()->get_box();
						system->constants->render.transform = system->constants->render.world * view.view_projection;
						system->update_constant_buffer(render_buffer_type::render);
						device->draw_indexed((uint32_t)box[(size_t)buffer_type::index]->get_elements(), 0, 0);
						culling_end();
						count++;
					}
				}

				return count;
			}
			size_t model::render_geometry_batched(core::timer* time, const geometry_renderer::groups& chunk)
			{
				VI_ASSERT(system->get_scene() != nullptr, "scene should be set");
				graphics::graphics_device* device = system->get_device();
				system->set_constant_buffer(render_buffer_type::view, pipelines.geometry.viewer_buffer, VI_VS | VI_PS);
				device->set_structure_buffer(system->get_material_buffer(), pipelines.geometry.materials, VI_PS);
				device->set_depth_stencil_state(depth_stencil);
				device->set_blend_state(blend);
				device->set_rasterizer_state(back_rasterizer);
				device->set_input_layout(layout[1]);
				device->set_sampler_state(sampler, pipelines.geometry.sampler, 7, VI_PS);
				device->set_shader(pipelines.geometry.shader, VI_VS | VI_PS);

				for (auto& group : chunk)
				{
					if (system->try_geometry(group->material_data, &pipelines.geometry.slotdata))
						device->draw_indexed_instanced(group->data_buffer, group->geometry_buffer, (uint32_t)group->instances.size());
				}

				static graphics::element_buffer* vertex_buffers[2] = { nullptr, nullptr };
				device->set_vertex_buffers(vertex_buffers, sizeof(vertex_buffers) / sizeof(*vertex_buffers));
				return chunk.size();
			}
			size_t model::render_depth_batched(core::timer* time, const geometry_renderer::groups& chunk)
			{
				VI_ASSERT(system->get_scene() != nullptr, "scene should be set");
				graphics::graphics_device* device = system->get_device();
				device->set_structure_buffer(system->get_material_buffer(), pipelines.depth.materials, VI_PS);
				device->set_depth_stencil_state(depth_stencil);
				device->set_blend_state(blend);
				device->set_rasterizer_state(system->state.is_set(render_opt::backfaces) ? front_rasterizer : back_rasterizer);
				device->set_input_layout(layout[1]);
				device->set_sampler_state(sampler, pipelines.depth.sampler, 1, VI_PS);
				device->set_shader(pipelines.depth.shader, VI_VS | VI_PS);

				for (auto& group : chunk)
				{
					if (system->try_geometry(group->material_data, &pipelines.depth.slotdata))
						device->draw_indexed_instanced(group->data_buffer, group->geometry_buffer, (uint32_t)group->instances.size());
				}

				static graphics::element_buffer* vertex_buffers[2] = { nullptr, nullptr };
				device->set_vertex_buffers(vertex_buffers, sizeof(vertex_buffers) / sizeof(*vertex_buffers));
				return chunk.size();
			}
			size_t model::render_depth_cube_batched(core::timer* time, const geometry_renderer::groups& chunk, trigonometry::matrix4x4* view_projection)
			{
				VI_ASSERT(system->get_scene() != nullptr, "scene should be set");
				graphics::graphics_device* device = system->get_device();
				system->set_constant_buffer(render_buffer_type::view, pipelines.depth_cube.viewer_buffer, VI_VS | VI_PS | VI_GS);
				device->set_structure_buffer(system->get_material_buffer(), pipelines.depth_cube.materials, VI_PS);
				device->set_depth_stencil_state(depth_stencil);
				device->set_blend_state(blend);
				device->set_rasterizer_state(system->state.is_set(render_opt::backfaces) ? front_rasterizer : back_rasterizer);
				device->set_input_layout(layout[1]);
				device->set_sampler_state(sampler, pipelines.depth_cube.sampler, 1, VI_PS);
				device->set_shader(pipelines.depth_cube.shader, VI_VS | VI_PS | VI_GS);
				device->set_buffer(pipelines.depth_cube.shader, pipelines.depth_cube.cube_buffer, VI_VS | VI_PS | VI_GS);
				device->update_buffer(pipelines.depth_cube.shader, view_projection);

				for (auto& group : chunk)
				{
					if (system->try_geometry(group->material_data, &pipelines.depth_cube.slotdata))
						device->draw_indexed_instanced(group->data_buffer, group->geometry_buffer, (uint32_t)group->instances.size());
				}

				static graphics::element_buffer* vertex_buffers[2] = { nullptr, nullptr };
				device->set_vertex_buffers(vertex_buffers, sizeof(vertex_buffers) / sizeof(*vertex_buffers));
				device->set_shader(nullptr, VI_GS);
				return chunk.size();
			}
			layer::model* model::get_drawable(components::model* base)
			{
				auto* drawable = base->get_drawable();
				if (!drawable)
					drawable = system->get_primitives()->get_box_model();

				return drawable;
			}

			skin::skin(layer::render_system* lab) : geometry_renderer(lab)
			{
				VI_ASSERT(system != nullptr, "render system should be set");
				VI_ASSERT(system->get_device() != nullptr, "graphics device should be set");

				graphics::graphics_device* device = system->get_device();
				depth_stencil = device->get_depth_stencil_state("drw_srw_lt");
				back_rasterizer = device->get_rasterizer_state("so_cback");
				front_rasterizer = device->get_rasterizer_state("so_cfront");
				blend = device->get_blend_state("bo_wrgba_one");
				sampler = device->get_sampler_state("a16_fa_wrap");
				layout = device->get_input_layout("vx_skin");

				pipelines.culling.shader = *system->compile_shader("materials/material_skin_culling", { });
				pipelines.culling.animation_buffer = *device->get_shader_slot(pipelines.culling.shader, "AnimationBuffer");
				pipelines.culling.object_buffer = *device->get_shader_slot(pipelines.culling.shader, "ObjectBuffer");
				pipelines.depth.shader = *system->compile_shader("materials/material_skin_depth", { });
				pipelines.depth.slotdata.diffuse_map = *device->get_shader_slot(pipelines.depth.shader, "DiffuseMap");
				pipelines.depth.sampler = *device->get_shader_sampler_slot(pipelines.depth.shader, "DiffuseMap", "Sampler");
				pipelines.depth.materials = *device->get_shader_slot(pipelines.depth.shader, "Materials");
				pipelines.depth.animation_buffer = *device->get_shader_slot(pipelines.depth.shader, "AnimationBuffer");
				pipelines.depth.object_buffer = *device->get_shader_slot(pipelines.depth.shader, "ObjectBuffer");
				pipelines.depth_cube.shader = *system->compile_shader("materials/material_skin_depth_cube", { }, sizeof(trigonometry::matrix4x4) * 6);
				pipelines.depth_cube.slotdata.diffuse_map = *device->get_shader_slot(pipelines.depth_cube.shader, "DiffuseMap");
				pipelines.depth_cube.sampler = *device->get_shader_sampler_slot(pipelines.depth_cube.shader, "DiffuseMap", "Sampler");
				pipelines.depth_cube.materials = *device->get_shader_slot(pipelines.depth_cube.shader, "Materials");
				pipelines.depth_cube.animation_buffer = *device->get_shader_slot(pipelines.depth_cube.shader, "AnimationBuffer");
				pipelines.depth_cube.object_buffer = *device->get_shader_slot(pipelines.depth_cube.shader, "ObjectBuffer");
				pipelines.depth_cube.viewer_buffer = *device->get_shader_slot(pipelines.depth_cube.shader, "ViewerBuffer");
				pipelines.depth_cube.cube_buffer = *device->get_shader_slot(pipelines.depth_cube.shader, "CubeBuffer");
				pipelines.geometry.shader = *system->compile_shader("materials/material_skin_geometry", { });
				pipelines.geometry.slotdata.diffuse_map = *device->get_shader_slot(pipelines.geometry.shader, "DiffuseMap");
				pipelines.geometry.slotdata.normal_map = *device->get_shader_slot(pipelines.geometry.shader, "NormalMap");
				pipelines.geometry.slotdata.metallic_map = *device->get_shader_slot(pipelines.geometry.shader, "MetallicMap");
				pipelines.geometry.slotdata.roughness_map = *device->get_shader_slot(pipelines.geometry.shader, "RoughnessMap");
				pipelines.geometry.slotdata.height_map = *device->get_shader_slot(pipelines.geometry.shader, "HeightMap");
				pipelines.geometry.slotdata.occlusion_map = *device->get_shader_slot(pipelines.geometry.shader, "OcclusionMap");
				pipelines.geometry.slotdata.emission_map = *device->get_shader_slot(pipelines.geometry.shader, "EmissionMap");
				pipelines.geometry.sampler = *device->get_shader_sampler_slot(pipelines.geometry.shader, "DiffuseMap", "Sampler");
				pipelines.geometry.materials = *device->get_shader_slot(pipelines.geometry.shader, "Materials");
				pipelines.geometry.animation_buffer = *device->get_shader_slot(pipelines.geometry.shader, "AnimationBuffer");
				pipelines.geometry.object_buffer = *device->get_shader_slot(pipelines.geometry.shader, "ObjectBuffer");
				pipelines.geometry.viewer_buffer = *device->get_shader_slot(pipelines.geometry.shader, "ViewerBuffer");
			}
			skin::~skin()
			{
				system->free_shader(pipelines.geometry.shader);
				system->free_shader(pipelines.culling.shader);
				system->free_shader(pipelines.depth.shader);
				system->free_shader(pipelines.depth_cube.shader);
			}
			size_t skin::cull_geometry(const viewer& view, const geometry_renderer::objects& chunk)
			{
				VI_ASSERT(system->get_primitives() != nullptr, "primitive cache should be set");

				graphics::element_buffer* box[2];
				system->get_primitives()->get_skin_box_buffers(box);

				graphics::graphics_device* device = system->get_device();
				system->set_constant_buffer(render_buffer_type::animation, pipelines.culling.animation_buffer, VI_VS);
				system->set_constant_buffer(render_buffer_type::render, pipelines.culling.object_buffer, VI_VS);
				device->set_rasterizer_state(back_rasterizer);
				device->set_input_layout(layout);
				device->set_shader(nullptr, VI_PS);
				device->set_shader(pipelines.culling.shader, VI_VS);

				size_t count = 0;
				if (system->precise_culling)
				{
					for (auto* base : chunk)
					{
						if (!culling_begin(base))
							continue;

						auto* drawable = get_drawable(base);
						auto& world = base->get_entity()->get_box();
						system->constants->animation.animated = (float)!drawable->skeleton.childs.empty();

						for (auto* mesh : drawable->meshes)
						{
							auto& matrices = base->skeleton.matrices[mesh];
							memcpy(system->constants->animation.offsets, matrices.data, sizeof(matrices.data));
							system->constants->render.world = mesh->transform * world;
							system->constants->render.transform = system->constants->render.world * view.view_projection;
							system->update_constant_buffer(render_buffer_type::animation);
							system->update_constant_buffer(render_buffer_type::render);
							device->draw_indexed(mesh);
						}

						culling_end();
						count++;
					}
				}
				else
				{
					device->set_vertex_buffer(box[(size_t)buffer_type::vertex]);
					device->set_index_buffer(box[(size_t)buffer_type::index], graphics::format::r32_uint);
					for (auto* base : chunk)
					{
						if (!culling_begin(base))
							continue;

						system->constants->animation.animated = (float)false;
						system->constants->render.world = base->get_entity()->get_box();
						system->constants->render.transform = system->constants->render.world * view.view_projection;
						system->update_constant_buffer(render_buffer_type::animation);
						system->update_constant_buffer(render_buffer_type::render);
						device->draw_indexed((uint32_t)box[(size_t)buffer_type::index]->get_elements(), 0, 0);
						culling_end();
						count++;
					}
				}

				return count;
			}
			size_t skin::render_geometry(core::timer* time, const geometry_renderer::objects& chunk)
			{
				VI_ASSERT(system->get_scene() != nullptr, "scene should be set");
				graphics::graphics_device* device = system->get_device();
				bool constant = system->state.is_set(render_opt::constant);

				system->set_constant_buffer(render_buffer_type::animation, pipelines.geometry.animation_buffer, VI_VS);
				system->set_constant_buffer(render_buffer_type::render, pipelines.geometry.object_buffer, VI_VS | VI_PS);
				system->set_constant_buffer(render_buffer_type::view, pipelines.geometry.viewer_buffer, VI_VS | VI_PS);
				device->set_structure_buffer(system->get_material_buffer(), pipelines.geometry.materials, VI_PS);
				device->set_depth_stencil_state(depth_stencil);
				device->set_blend_state(blend);
				device->set_rasterizer_state(back_rasterizer);
				device->set_input_layout(layout);
				device->set_sampler_state(sampler, pipelines.geometry.sampler, 7, VI_PS);
				device->set_shader(pipelines.geometry.shader, VI_VS | VI_PS);

				size_t count = 0;
				for (auto* base : chunk)
				{
					if (constant && !base->constant)
						continue;

					auto* drawable = get_drawable(base);
					auto& world = base->get_entity()->get_box();
					system->constants->animation.animated = (float)!drawable->skeleton.childs.empty();
					system->constants->render.texcoord = base->texcoord;

					for (auto* mesh : drawable->meshes)
					{
						if (!system->try_geometry(base->get_material(mesh), &pipelines.geometry.slotdata))
							continue;

						auto& matrices = base->skeleton.matrices[mesh];
						memcpy(system->constants->animation.offsets, matrices.data, sizeof(matrices.data));
						system->constants->render.world = mesh->transform * world;
						system->constants->render.transform = system->constants->render.world * system->view.view_projection;
						system->update_constant_buffer(render_buffer_type::animation);
						system->update_constant_buffer(render_buffer_type::render);
						device->draw_indexed(mesh);
					}
					count++;
				}

				return count;
			}
			size_t skin::render_depth(core::timer* time, const geometry_renderer::objects& chunk)
			{
				VI_ASSERT(system->get_scene() != nullptr, "scene should be set");
				graphics::graphics_device* device = system->get_device();
				system->set_constant_buffer(render_buffer_type::animation, pipelines.depth.animation_buffer, VI_VS);
				system->set_constant_buffer(render_buffer_type::render, pipelines.depth.object_buffer, VI_VS | VI_PS);
				device->set_structure_buffer(system->get_material_buffer(), pipelines.depth.materials, VI_PS);
				device->set_depth_stencil_state(depth_stencil);
				device->set_blend_state(blend);
				device->set_rasterizer_state(system->state.is_set(render_opt::backfaces) ? front_rasterizer : back_rasterizer);
				device->set_input_layout(layout);
				device->set_sampler_state(sampler, pipelines.depth.sampler, 1, VI_PS);
				device->set_shader(pipelines.depth.shader, VI_VS | VI_PS);

				size_t count = 0;
				for (auto* base : chunk)
				{
					auto* drawable = get_drawable(base);
					auto& world = base->get_entity()->get_box();
					system->constants->animation.animated = (float)!drawable->skeleton.childs.empty();
					system->constants->render.texcoord = base->texcoord;

					for (auto* mesh : drawable->meshes)
					{
						if (!system->try_geometry(base->get_material(mesh), &pipelines.depth.slotdata))
							continue;

						auto& matrices = base->skeleton.matrices[mesh];
						memcpy(system->constants->animation.offsets, matrices.data, sizeof(matrices.data));
						system->constants->render.world = mesh->transform * world;
						system->constants->render.transform = system->constants->render.world * system->view.view_projection;
						system->update_constant_buffer(render_buffer_type::animation);
						system->update_constant_buffer(render_buffer_type::render);
						device->draw_indexed(mesh);
					}

					count++;
				}

				return count;
			}
			size_t skin::render_depth_cube(core::timer* time, const geometry_renderer::objects& chunk, trigonometry::matrix4x4* view_projection)
			{
				VI_ASSERT(system->get_scene() != nullptr, "scene should be set");
				graphics::graphics_device* device = system->get_device();
				system->set_constant_buffer(render_buffer_type::animation, pipelines.depth_cube.animation_buffer, VI_VS);
				system->set_constant_buffer(render_buffer_type::render, pipelines.depth_cube.object_buffer, VI_VS | VI_PS);
				device->set_structure_buffer(system->get_material_buffer(), pipelines.depth_cube.materials, VI_PS);
				device->set_depth_stencil_state(depth_stencil);
				device->set_blend_state(blend);
				device->set_rasterizer_state(system->state.is_set(render_opt::backfaces) ? front_rasterizer : back_rasterizer);
				device->set_input_layout(layout);
				device->set_sampler_state(sampler, pipelines.depth_cube.sampler, 7, VI_PS);
				device->set_shader(pipelines.depth_cube.shader, VI_VS | VI_PS | VI_GS);
				device->set_buffer(pipelines.depth_cube.shader, pipelines.depth_cube.cube_buffer, VI_VS | VI_PS | VI_GS);
				device->update_buffer(pipelines.depth_cube.shader, view_projection);

				size_t count = 0;
				for (auto* base : chunk)
				{
					auto* drawable = get_drawable(base);
					auto& world = base->get_entity()->get_box();
					system->constants->animation.animated = (float)!drawable->skeleton.childs.empty();
					system->constants->render.texcoord = base->texcoord;

					for (auto* mesh : drawable->meshes)
					{
						if (!system->try_geometry(base->get_material(mesh), &pipelines.depth_cube.slotdata))
							continue;

						auto& matrices = base->skeleton.matrices[mesh];
						memcpy(system->constants->animation.offsets, matrices.data, sizeof(matrices.data));
						system->constants->render.world = mesh->transform * world;
						system->update_constant_buffer(render_buffer_type::animation);
						system->update_constant_buffer(render_buffer_type::render);
						device->draw_indexed(mesh);
					}

					count++;
				}

				device->set_shader(nullptr, VI_GS);
				return count;
			}
			layer::skin_model* skin::get_drawable(components::skin* base)
			{
				auto* drawable = base->get_drawable();
				if (!drawable)
					drawable = system->get_primitives()->get_skin_box_model();

				return drawable;
			}

			emitter::emitter(render_system* lab) : geometry_renderer(lab)
			{
				VI_ASSERT(system != nullptr, "render system should be set");
				VI_ASSERT(system->get_device() != nullptr, "graphics device should be set");

				graphics::graphics_device* device = system->get_device();
				depth_stencil_opaque = device->get_depth_stencil_state("drw_srw_lt");
				depth_stencil_additive = device->get_depth_stencil_state("dro_srw_lt");
				rasterizer = device->get_rasterizer_state("so_cback");
				additive_blend = device->get_blend_state("bw_wrgba_alpha");
				overwrite_blend = device->get_blend_state("bo_wrgba_one");
				sampler = device->get_sampler_state("a16_fa_wrap");

				pipelines.depth.shader = *system->compile_shader("materials/material_emitter_depth", { });
				pipelines.depth.slotdata.diffuse_map = *device->get_shader_slot(pipelines.depth.shader, "DiffuseMap");
				pipelines.depth.sampler = *device->get_shader_sampler_slot(pipelines.depth.shader, "DiffuseMap", "Sampler");
				pipelines.depth.materials = *device->get_shader_slot(pipelines.depth.shader, "Materials");
				pipelines.depth.object_buffer = *device->get_shader_slot(pipelines.depth.shader, "ObjectBuffer");
				pipelines.depth.elements = *device->get_shader_slot(pipelines.depth.shader, "Elements");
				pipelines.depth_point.shader = *system->compile_shader("materials/material_emitter_depth_point", { }, sizeof(quad));
				pipelines.depth_point.slotdata.diffuse_map = *device->get_shader_slot(pipelines.depth_point.shader, "DiffuseMap");
				pipelines.depth_point.sampler = *device->get_shader_sampler_slot(pipelines.depth_point.shader, "DiffuseMap", "Sampler");
				pipelines.depth_point.materials = *device->get_shader_slot(pipelines.depth_point.shader, "Materials");
				pipelines.depth_point.object_buffer = *device->get_shader_slot(pipelines.depth_point.shader, "ObjectBuffer");
				pipelines.depth_point.viewer_buffer = *device->get_shader_slot(pipelines.depth_point.shader, "ViewerBuffer");
				pipelines.depth_point.quad = *device->get_shader_slot(pipelines.depth_point.shader, "Quad");
				pipelines.depth_point.elements = *device->get_shader_slot(pipelines.depth_point.shader, "Elements");
				pipelines.depth_quad.shader = *system->compile_shader("materials/material_emitter_depth_quad", { }, sizeof(trigonometry::matrix4x4) * 6);
				pipelines.depth_quad.slotdata.diffuse_map = *device->get_shader_slot(pipelines.depth_quad.shader, "DiffuseMap");
				pipelines.depth_quad.sampler = *device->get_shader_sampler_slot(pipelines.depth_quad.shader, "DiffuseMap", "Sampler");
				pipelines.depth_quad.materials = *device->get_shader_slot(pipelines.depth_quad.shader, "Materials");
				pipelines.depth_quad.object_buffer = *device->get_shader_slot(pipelines.depth_quad.shader, "ObjectBuffer");
				pipelines.depth_quad.viewer_buffer = *device->get_shader_slot(pipelines.depth_quad.shader, "ViewerBuffer");
				pipelines.depth_quad.quad = *device->get_shader_slot(pipelines.depth_quad.shader, "Quad");
				pipelines.depth_quad.elements = *device->get_shader_slot(pipelines.depth_quad.shader, "Elements");
				pipelines.geometry_opaque.shader = *system->compile_shader("materials/material_emitter_geometry_opaque", { });
				pipelines.geometry_opaque.slotdata.diffuse_map = *device->get_shader_slot(pipelines.geometry_opaque.shader, "DiffuseMap");
				pipelines.geometry_opaque.slotdata.normal_map = *device->get_shader_slot(pipelines.geometry_opaque.shader, "NormalMap");
				pipelines.geometry_opaque.materials = *device->get_shader_slot(pipelines.geometry_opaque.shader, "Materials");
				pipelines.geometry_opaque.object_buffer = *device->get_shader_slot(pipelines.geometry_opaque.shader, "ObjectBuffer");
				pipelines.geometry_opaque.viewer_buffer = *device->get_shader_slot(pipelines.geometry_opaque.shader, "ViewerBuffer");
				pipelines.geometry_opaque.elements = *device->get_shader_slot(pipelines.geometry_opaque.shader, "Elements");
				pipelines.geometry_transparent.shader = *system->compile_shader("materials/material_emitter_geometry_transparent", { });
				pipelines.geometry_transparent.slotdata.diffuse_map = *device->get_shader_slot(pipelines.geometry_transparent.shader, "DiffuseMap");
				pipelines.geometry_transparent.slotdata.normal_map = *device->get_shader_slot(pipelines.geometry_transparent.shader, "NormalMap");
				pipelines.geometry_transparent.materials = *device->get_shader_slot(pipelines.geometry_transparent.shader, "Materials");
				pipelines.geometry_transparent.object_buffer = *device->get_shader_slot(pipelines.geometry_transparent.shader, "ObjectBuffer");
				pipelines.geometry_transparent.viewer_buffer = *device->get_shader_slot(pipelines.geometry_transparent.shader, "ViewerBuffer");
				pipelines.geometry_transparent.elements = *device->get_shader_slot(pipelines.geometry_transparent.shader, "Elements");
			}
			emitter::~emitter()
			{
				system->free_shader(pipelines.geometry_opaque.shader);
				system->free_shader(pipelines.geometry_transparent.shader);
				system->free_shader(pipelines.depth.shader);
				system->free_shader(pipelines.depth_point.shader);
				system->free_shader(pipelines.depth_quad.shader);
			}
			size_t emitter::render_geometry(core::timer* time, const geometry_renderer::objects& chunk)
			{
				VI_ASSERT(system->get_scene() != nullptr, "scene should be set");
				graphics::graphics_device* device = system->get_device();
				graphics::primitive_topology t = device->get_primitive_topology();
				graphics::shader* shader = nullptr;
				material::slots* slotdata = nullptr;
				uint32_t buffer_slot = (uint32_t)-1;
				viewer& view = system->view;
				bool constant = system->state.is_set(render_opt::constant);

				if (system->state.is_set(render_opt::additive))
				{
					buffer_slot = pipelines.geometry_transparent.elements;
					shader = pipelines.geometry_transparent.shader;
					slotdata = &pipelines.geometry_transparent.slotdata;
					system->set_constant_buffer(render_buffer_type::render, pipelines.geometry_transparent.object_buffer, VI_VS | VI_PS | VI_GS);
					system->set_constant_buffer(render_buffer_type::view, pipelines.geometry_transparent.viewer_buffer, VI_VS | VI_PS | VI_GS);
					device->set_depth_stencil_state(depth_stencil_additive);
					device->set_blend_state(additive_blend);
					device->set_sampler_state(sampler, pipelines.geometry_transparent.sampler, 2, VI_PS);
				}
				else
				{
					buffer_slot = pipelines.geometry_opaque.elements;
					shader = pipelines.geometry_opaque.shader;
					slotdata = &pipelines.geometry_opaque.slotdata;
					system->set_constant_buffer(render_buffer_type::render, pipelines.geometry_opaque.object_buffer, VI_VS | VI_PS | VI_GS);
					system->set_constant_buffer(render_buffer_type::view, pipelines.geometry_opaque.viewer_buffer, VI_VS | VI_PS | VI_GS);
					device->set_depth_stencil_state(depth_stencil_opaque);
					device->set_blend_state(overwrite_blend);
					device->set_sampler_state(sampler, pipelines.geometry_opaque.sampler, 2, VI_PS);
				}

				device->set_primitive_topology(graphics::primitive_topology::point_list);
				device->set_rasterizer_state(rasterizer);
				device->set_input_layout(layout);
				device->set_vertex_buffer(nullptr);
				device->set_shader(shader, VI_VS | VI_PS);

				size_t count = 0;
				for (auto* base : chunk)
				{
					if ((constant && !base->constant) || !base->get_buffer())
						continue;

					if (!system->try_geometry(base->get_material(), slotdata))
						continue;

					system->constants->render.world = view.projection;
					system->constants->render.transform = (base->quad_based ? view.view : view.view_projection);
					system->constants->render.texcoord = base->get_entity()->get_transform()->forward();
					if (base->connected)
						system->constants->render.transform = base->get_entity()->get_box() * system->constants->render.transform;

					device->set_buffer(base->get_buffer(), buffer_slot, VI_VS | VI_PS);
					device->set_shader(base->quad_based ? shader : nullptr, VI_GS);
					device->update_buffer(base->get_buffer());
					system->update_constant_buffer(render_buffer_type::render);
					device->draw((uint32_t)base->get_buffer()->get_array().size(), 0);

					count++;
				}

				device->set_buffer((graphics::instance_buffer*)nullptr, buffer_slot, VI_VS | VI_PS);
				device->set_shader(nullptr, VI_GS);
				device->set_primitive_topology(t);
				return count;
			}
			size_t emitter::render_depth(core::timer* time, const geometry_renderer::objects& chunk)
			{
				VI_ASSERT(system->get_scene() != nullptr, "scene should be set");
				graphics::graphics_device* device = system->get_device();
				graphics::primitive_topology t = device->get_primitive_topology();
				viewer& view = system->view;

				system->set_constant_buffer(render_buffer_type::render, pipelines.depth.object_buffer, VI_VS | VI_PS | VI_GS);
				device->set_primitive_topology(graphics::primitive_topology::point_list);
				device->set_depth_stencil_state(depth_stencil_opaque);
				device->set_blend_state(overwrite_blend);
				device->set_rasterizer_state(rasterizer);
				device->set_input_layout(layout);
				device->set_sampler_state(sampler, pipelines.depth.sampler, 1, VI_PS);
				device->set_shader(pipelines.depth.shader, VI_VS | VI_PS);
				device->set_vertex_buffer(nullptr);

				size_t count = 0;
				for (auto* base : chunk)
				{
					if (!base->get_buffer() || !system->try_geometry(base->get_material(), &pipelines.depth.slotdata))
						continue;

					system->constants->render.world = view.projection;
					system->constants->render.transform = (base->quad_based ? view.view : view.view_projection);
					if (base->connected)
						system->constants->render.transform = base->get_entity()->get_box() * system->constants->render.transform;

					device->set_buffer(base->get_buffer(), pipelines.depth.elements, VI_VS | VI_PS);
					device->set_shader(base->quad_based ? pipelines.depth.shader : nullptr, VI_GS);
					system->update_constant_buffer(render_buffer_type::render);
					device->draw((uint32_t)base->get_buffer()->get_array().size(), 0);

					count++;
				}

				device->set_buffer((graphics::instance_buffer*)nullptr, pipelines.depth.elements, VI_VS | VI_PS);
				device->set_shader(nullptr, VI_GS);
				device->set_primitive_topology(t);
				return count;
			}
			size_t emitter::render_depth_cube(core::timer* time, const geometry_renderer::objects& chunk, trigonometry::matrix4x4* view_projection)
			{
				VI_ASSERT(system->get_scene() != nullptr, "scene should be set");
				auto& source = system->view;
				quad.face_view[0] = trigonometry::matrix4x4::create_look_at(trigonometry::cube_face::positive_x, source.position);
				quad.face_view[1] = trigonometry::matrix4x4::create_look_at(trigonometry::cube_face::negative_x, source.position);
				quad.face_view[2] = trigonometry::matrix4x4::create_look_at(trigonometry::cube_face::positive_y, source.position);
				quad.face_view[3] = trigonometry::matrix4x4::create_look_at(trigonometry::cube_face::negative_y, source.position);
				quad.face_view[4] = trigonometry::matrix4x4::create_look_at(trigonometry::cube_face::positive_z, source.position);
				quad.face_view[5] = trigonometry::matrix4x4::create_look_at(trigonometry::cube_face::negative_z, source.position);

				graphics::graphics_device* device = system->get_device();
				graphics::primitive_topology t = device->get_primitive_topology();
				device->set_primitive_topology(graphics::primitive_topology::point_list);
				device->set_depth_stencil_state(depth_stencil_opaque);
				device->set_blend_state(overwrite_blend);
				device->set_rasterizer_state(rasterizer);
				device->set_input_layout(layout);
				device->set_vertex_buffer(nullptr);

				size_t count = 0;
				for (auto* base : chunk)
				{
					if (!base->get_buffer() || !system->try_geometry(base->get_material(), &pipelines.depth_quad.slotdata))
						continue;

					system->constants->render.world = (base->connected ? base->get_entity()->get_box() : trigonometry::matrix4x4::identity());
					if (base->quad_based)
					{
						system->set_constant_buffer(render_buffer_type::render, pipelines.depth_quad.object_buffer, VI_VS | VI_PS | VI_GS);
						system->set_constant_buffer(render_buffer_type::view, pipelines.depth_quad.viewer_buffer, VI_VS | VI_PS | VI_GS);
						device->set_sampler_state(sampler, pipelines.depth_quad.sampler, 1, VI_PS);
						device->set_buffer(pipelines.depth_quad.shader, pipelines.depth_quad.quad, VI_VS | VI_PS | VI_GS);
						device->set_shader(pipelines.depth_quad.shader, VI_VS | VI_PS | VI_GS);
						device->set_buffer(base->get_buffer(), pipelines.depth_quad.elements, VI_VS | VI_PS);
						device->update_buffer(pipelines.depth_quad.shader, &quad);
					}
					else
					{
						system->set_constant_buffer(render_buffer_type::render, pipelines.depth_point.object_buffer, VI_VS | VI_PS | VI_GS);
						system->set_constant_buffer(render_buffer_type::view, pipelines.depth_point.viewer_buffer, VI_VS | VI_PS | VI_GS);
						device->set_sampler_state(sampler, pipelines.depth_point.sampler, 1, VI_PS);
						device->set_buffer(pipelines.depth_point.shader, pipelines.depth_point.quad, VI_VS | VI_PS | VI_GS);
						device->set_shader(pipelines.depth_point.shader, VI_VS | VI_PS | VI_GS);
						device->set_buffer(base->get_buffer(), pipelines.depth_point.elements, VI_VS | VI_PS);
						device->update_buffer(pipelines.depth_point.shader, &quad);
					}
					system->update_constant_buffer(render_buffer_type::render);
					device->draw((uint32_t)base->get_buffer()->get_array().size(), 0);

					count++;
				}

				device->set_buffer((graphics::instance_buffer*)nullptr, pipelines.depth_quad.elements, VI_VS | VI_PS);
				device->set_buffer((graphics::instance_buffer*)nullptr, pipelines.depth_quad.quad, VI_VS | VI_PS);
				device->set_buffer((graphics::instance_buffer*)nullptr, pipelines.depth_point.elements, VI_VS | VI_PS);
				device->set_buffer((graphics::instance_buffer*)nullptr, pipelines.depth_point.quad, VI_VS | VI_PS);
				device->set_shader(nullptr, VI_GS);
				device->set_primitive_topology(t);
				return count;
			}

			decal::decal(render_system* lab) : geometry_renderer(lab)
			{
				VI_ASSERT(system != nullptr, "render system should be set");
				VI_ASSERT(system->get_device() != nullptr, "graphics device should be set");

				graphics::graphics_device* device = system->get_device();
				depth_stencil = device->get_depth_stencil_state("doo_soo_lt");
				rasterizer = device->get_rasterizer_state("so_cback");
				blend = device->get_blend_state("bw_wrgba_one");
				sampler = device->get_sampler_state("a16_fa_wrap");
				layout = device->get_input_layout("vx_shape");

				pipelines.geometry.shader = *system->compile_shader("materials/material_decal_geometry", { });
				pipelines.geometry.slotdata.diffuse_map = *device->get_shader_slot(pipelines.geometry.shader, "DiffuseMap");
				pipelines.geometry.depth_map = *device->get_shader_slot(pipelines.geometry.shader, "LDepthBuffer");
				pipelines.geometry.sampler = *device->get_shader_sampler_slot(pipelines.geometry.shader, "DiffuseMap", "Sampler");
				pipelines.geometry.materials = *device->get_shader_slot(pipelines.geometry.shader, "Materials");
				pipelines.geometry.object_buffer = *device->get_shader_slot(pipelines.geometry.shader, "ObjectBuffer");
			}
			decal::~decal()
			{
				system->free_shader(pipelines.geometry.shader);
			}
			size_t decal::render_geometry(core::timer* time, const geometry_renderer::objects& chunk)
			{
				VI_ASSERT(system->get_scene() != nullptr, "scene should be set");

				graphics::multi_render_target_2d* mrt = system->get_mrt(target_type::main);
				graphics::graphics_device* device = system->get_device();
				bool constant = system->state.is_set(render_opt::constant);

				graphics::element_buffer* box[2];
				system->get_primitives()->get_box_buffers(box);

				system->set_constant_buffer(render_buffer_type::render, pipelines.geometry.object_buffer, VI_VS | VI_PS);
				device->set_depth_stencil_state(depth_stencil);
				device->set_blend_state(blend);
				device->set_rasterizer_state(rasterizer);
				device->set_input_layout(layout);
				device->set_target(mrt, 0);
				device->set_sampler_state(sampler, pipelines.geometry.sampler, 8, VI_PS);
				device->set_shader(pipelines.geometry.shader, VI_VS | VI_PS);
				device->set_texture_2d(mrt->get_target(2), pipelines.geometry.depth_map, VI_PS);
				device->set_vertex_buffer(box[(size_t)buffer_type::vertex]);
				device->set_index_buffer(box[(size_t)buffer_type::index], graphics::format::r32_uint);

				size_t count = 0;
				for (auto* base : chunk)
				{
					if ((constant && !base->constant) || !system->try_geometry(base->get_material(), &pipelines.geometry.slotdata))
						continue;

					system->constants->render.transform = base->get_entity()->get_box() * system->view.view_projection;
					system->constants->render.world = system->constants->render.transform.inv();
					system->constants->render.texcoord = base->texcoord;
					system->update_constant_buffer(render_buffer_type::render);
					device->draw_indexed((uint32_t)box[(size_t)buffer_type::index]->get_elements(), 0, 0);
					count++;
				}

				device->set_texture_2d(nullptr, pipelines.geometry.depth_map, VI_PS);
				system->restore_output();
				return count;
			}

			lighting::lighting(render_system* lab) : renderer(lab)
			{
				VI_ASSERT(system != nullptr, "render system should be set");
				VI_ASSERT(system->get_device() != nullptr, "graphics device should be set");
				shadows.tick.delay = 5;

				graphics::graphics_device* device = system->get_device();
				depth_stencil_none = device->get_depth_stencil_state("doo_soo_lt");
				depth_stencil_greater = device->get_depth_stencil_state("dro_sro_gte");
				depth_stencil_less = device->get_depth_stencil_state("dro_sro_lt");
				front_rasterizer = device->get_rasterizer_state("so_cfront");
				back_rasterizer = device->get_rasterizer_state("so_cback");
				none_rasterizer = device->get_rasterizer_state("so_co");
				blend_additive = device->get_blend_state("bw_wrgbo_one");
				blend_overwrite = device->get_blend_state("bo_woooo_one");
				blend_overload = device->get_blend_state("bo_wrgba_one");
				depth_sampler = device->get_sampler_state("a1_fl_clamp");
				depth_less_sampler = device->get_sampler_state("a1_fl_clamp_cmp_lt");
				wrap_sampler = device->get_sampler_state("a16_fa_wrap");
				layout = device->get_input_layout("vx_shape");

				pipelines.ambient.shader = *system->compile_shader("shading/lighting_ambient", { }, sizeof(iambient_buffer));
				pipelines.ambient.materials = *device->get_shader_slot(pipelines.ambient.shader, "Materials");
				pipelines.ambient.sampler = *device->get_shader_sampler_slot(pipelines.ambient.shader, "DiffuseBuffer", "Sampler");
				pipelines.ambient.viewer_buffer = *device->get_shader_slot(pipelines.ambient.shader, "ViewerBuffer");
				pipelines.ambient.ambient_buffer = *device->get_shader_slot(pipelines.ambient.shader, "AmbientBuffer");
				pipelines.ambient.diffuse_buffer = *device->get_shader_slot(pipelines.ambient.shader, "DiffuseBuffer");
				pipelines.ambient.normal_buffer = *device->get_shader_slot(pipelines.ambient.shader, "NormalBuffer");
				pipelines.ambient.depth_buffer = *device->get_shader_slot(pipelines.ambient.shader, "DepthBuffer");
				pipelines.ambient.surface_buffer = *device->get_shader_slot(pipelines.ambient.shader, "SurfaceBuffer");
				pipelines.ambient.light_map = *device->get_shader_slot(pipelines.ambient.shader, "LightMap");
				pipelines.ambient.sky_map = *device->get_shader_slot(pipelines.ambient.shader, "SkyMap");
				pipelines.point.shader_base = *system->compile_shader("shading/lighting_point", { }, sizeof(ipoint_buffer));
				pipelines.point.shader_shadowed = *system->compile_shader("shading/lighting_point", { "SHADOWED" });
				pipelines.point.materials = *device->get_shader_slot(pipelines.point.shader_shadowed, "Materials");
				pipelines.point.sampler = *device->get_shader_sampler_slot(pipelines.point.shader_shadowed, "DiffuseBuffer", "Sampler");
				pipelines.point.depth_sampler = *device->get_shader_sampler_slot(pipelines.point.shader_shadowed, "DepthMapLess", "DepthSampler");
				pipelines.point.depth_less_sampler = *device->get_shader_sampler_slot(pipelines.point.shader_shadowed, "DepthMapLess", "DepthLessSampler");
				pipelines.point.viewer_buffer = *device->get_shader_slot(pipelines.point.shader_shadowed, "ViewerBuffer");
				pipelines.point.point_buffer = *device->get_shader_slot(pipelines.point.shader_shadowed, "PointBuffer");
				pipelines.point.diffuse_buffer = *device->get_shader_slot(pipelines.point.shader_shadowed, "DiffuseBuffer");
				pipelines.point.normal_buffer = *device->get_shader_slot(pipelines.point.shader_shadowed, "NormalBuffer");
				pipelines.point.depth_buffer = *device->get_shader_slot(pipelines.point.shader_shadowed, "DepthBuffer");
				pipelines.point.surface_buffer = *device->get_shader_slot(pipelines.point.shader_shadowed, "SurfaceBuffer");
				pipelines.point.depth_map_less = *device->get_shader_slot(pipelines.point.shader_shadowed, "DepthMapLess");
				pipelines.spot.shader_base = *system->compile_shader("shading/lighting_spot", { }, sizeof(ispot_buffer));
				pipelines.spot.shader_shadowed = *system->compile_shader("shading/lighting_spot", { "SHADOWED" });
				pipelines.spot.materials = *device->get_shader_slot(pipelines.spot.shader_shadowed, "Materials");
				pipelines.spot.sampler = *device->get_shader_sampler_slot(pipelines.spot.shader_shadowed, "DiffuseBuffer", "Sampler");
				pipelines.spot.depth_sampler = *device->get_shader_sampler_slot(pipelines.spot.shader_shadowed, "DepthMapLess", "DepthSampler");
				pipelines.spot.depth_less_sampler = *device->get_shader_sampler_slot(pipelines.spot.shader_shadowed, "DepthMapLess", "DepthLessSampler");
				pipelines.spot.viewer_buffer = *device->get_shader_slot(pipelines.spot.shader_shadowed, "ViewerBuffer");
				pipelines.spot.spot_buffer = *device->get_shader_slot(pipelines.spot.shader_shadowed, "SpotBuffer");
				pipelines.spot.diffuse_buffer = *device->get_shader_slot(pipelines.spot.shader_shadowed, "DiffuseBuffer");
				pipelines.spot.normal_buffer = *device->get_shader_slot(pipelines.spot.shader_shadowed, "NormalBuffer");
				pipelines.spot.depth_buffer = *device->get_shader_slot(pipelines.spot.shader_shadowed, "DepthBuffer");
				pipelines.spot.surface_buffer = *device->get_shader_slot(pipelines.spot.shader_shadowed, "SurfaceBuffer");
				pipelines.spot.depth_map_less = *device->get_shader_slot(pipelines.spot.shader_shadowed, "DepthMapLess");
				pipelines.line.shader_base = *system->compile_shader("shading/lighting_line", { }, sizeof(iline_buffer));
				pipelines.line.shader_shadowed = *system->compile_shader("shading/lighting_line", { "SHADOWED" });
				pipelines.line.materials = *device->get_shader_slot(pipelines.line.shader_shadowed, "Materials");
				pipelines.line.sampler = *device->get_shader_sampler_slot(pipelines.line.shader_shadowed, "DiffuseBuffer", "Sampler");
				pipelines.line.depth_less_sampler = *device->get_shader_sampler_slot(pipelines.line.shader_shadowed, "DepthMap", "DepthLessSampler");
				pipelines.line.viewer_buffer = *device->get_shader_slot(pipelines.line.shader_shadowed, "ViewerBuffer");
				pipelines.line.line_buffer = *device->get_shader_slot(pipelines.line.shader_shadowed, "LineBuffer");
				pipelines.line.diffuse_buffer = *device->get_shader_slot(pipelines.line.shader_shadowed, "DiffuseBuffer");
				pipelines.line.normal_buffer = *device->get_shader_slot(pipelines.line.shader_shadowed, "NormalBuffer");
				pipelines.line.depth_buffer = *device->get_shader_slot(pipelines.line.shader_shadowed, "DepthBuffer");
				pipelines.line.surface_buffer = *device->get_shader_slot(pipelines.line.shader_shadowed, "SurfaceBuffer");
				pipelines.line.depth_map = *device->get_shader_slot(pipelines.line.shader_shadowed, "DepthMap[0]");
				pipelines.surface.shader = *system->compile_shader("shading/lighting_surface", { }, sizeof(isurface_buffer));
				pipelines.surface.materials = *device->get_shader_slot(pipelines.surface.shader, "Materials");
				pipelines.surface.sampler = *device->get_shader_sampler_slot(pipelines.surface.shader, "EnvironmentMap", "Sampler");
				pipelines.surface.viewer_buffer = *device->get_shader_slot(pipelines.surface.shader, "ViewerBuffer");
				pipelines.surface.environment_buffer = *device->get_shader_slot(pipelines.surface.shader, "EnvironmentBuffer");
				pipelines.surface.normal_buffer = *device->get_shader_slot(pipelines.surface.shader, "NormalBuffer");
				pipelines.surface.depth_buffer = *device->get_shader_slot(pipelines.surface.shader, "DepthBuffer");
				pipelines.surface.surface_buffer = *device->get_shader_slot(pipelines.surface.shader, "SurfaceBuffer");
				pipelines.surface.environment_map = *device->get_shader_slot(pipelines.surface.shader, "EnvironmentMap");
			}
			lighting::~lighting()
			{
				system->free_shader(pipelines.surface.shader);
				system->free_shader(pipelines.line.shader_base);
				system->free_shader(pipelines.line.shader_shadowed);
				system->free_shader(pipelines.spot.shader_base);
				system->free_shader(pipelines.spot.shader_shadowed);
				system->free_shader(pipelines.point.shader_base);
				system->free_shader(pipelines.point.shader_shadowed);
				system->free_shader(pipelines.ambient.shader);
				core::memory::release(surfaces.subresource);
				core::memory::release(surfaces.input);
				core::memory::release(surfaces.output);
				core::memory::release(surfaces.merger);
				core::memory::release(sky_base);
				core::memory::release(sky_map);
				core::memory::release(lighting_map);
			}
			void lighting::deserialize(core::schema* node)
			{
				VI_ASSERT(node != nullptr, "schema should be set");

				core::string path;
				if (series::unpack(node->find("sky-map"), &path) && !path.empty())
					system->get_scene()->load_resource<graphics::texture_2d>(system->get_component(), path, [this](expects_content<graphics::texture_2d*>&& texture) { this->set_sky_map(texture.or_else(nullptr)); });

				heavy_series::unpack(node->find("high-emission"), &ambient_buffer.high_emission);
				heavy_series::unpack(node->find("low-emission"), &ambient_buffer.low_emission);
				series::unpack(node->find("sky-emission"), &ambient_buffer.sky_emission);
				series::unpack(node->find("light-emission"), &ambient_buffer.light_emission);
				heavy_series::unpack(node->find("sky-color"), &ambient_buffer.sky_color);
				heavy_series::unpack(node->find("fog-color"), &ambient_buffer.fog_color);
				series::unpack(node->find("fog-amount"), &ambient_buffer.fog_amount);
				series::unpack(node->find("fog-far-off"), &ambient_buffer.fog_far_off);
				heavy_series::unpack(node->find("fog-far"), &ambient_buffer.fog_far);
				series::unpack(node->find("fog-near-off"), &ambient_buffer.fog_near_off);
				heavy_series::unpack(node->find("fog-near"), &ambient_buffer.fog_near);
				series::unpack(node->find("recursive"), &ambient_buffer.recursive);
				series::unpack(node->find("shadow-distance"), &shadows.distance);
				series::unpack_a(node->find("sf-size"), &surfaces.size);
			}
			void lighting::serialize(core::schema* node)
			{
				VI_ASSERT(node != nullptr, "schema should be set");

				series::pack(node->set("sky-map"), system->get_scene()->find_resource_id<graphics::texture_2d>(sky_base));
				heavy_series::pack(node->set("high-emission"), ambient_buffer.high_emission);
				heavy_series::pack(node->set("low-emission"), ambient_buffer.low_emission);
				series::pack(node->set("sky-emission"), ambient_buffer.sky_emission);
				series::pack(node->set("light-emission"), ambient_buffer.light_emission);
				heavy_series::pack(node->set("sky-color"), ambient_buffer.sky_color);
				heavy_series::pack(node->set("fog-color"), ambient_buffer.fog_color);
				series::pack(node->set("fog-amount"), ambient_buffer.fog_amount);
				series::pack(node->set("fog-far-off"), ambient_buffer.fog_far_off);
				heavy_series::pack(node->set("fog-far"), ambient_buffer.fog_far);
				series::pack(node->set("fog-near-off"), ambient_buffer.fog_near_off);
				heavy_series::pack(node->set("fog-near"), ambient_buffer.fog_near);
				series::pack(node->set("recursive"), ambient_buffer.recursive);
				series::pack(node->set("shadow-distance"), shadows.distance);
				series::pack(node->set("sf-size"), (uint64_t)surfaces.size);
			}
			void lighting::resize_buffers()
			{
				core::memory::release(lighting_map);
			}
			void lighting::begin_pass(core::timer* time)
			{
				if (system->state.is(render_state::depth) || system->state.is(render_state::depth_cube))
					return;

				auto& lines = system->get_scene()->get_components<components::line_light>();
				lights.illuminators.push(time, system);
				lights.surfaces.push(time, system);
				lights.points.push(time, system);
				lights.spots.push(time, system);
				lights.lines = &lines;
			}
			void lighting::end_pass()
			{
				if (system->state.is(render_state::depth) || system->state.is(render_state::depth_cube))
					return;

				lights.spots.pop();
				lights.points.pop();
				lights.surfaces.pop();
				lights.illuminators.pop();
			}
			void lighting::render_result_buffers()
			{
				graphics::multi_render_target_2d* mrt = system->get_mrt(target_type::main);
				graphics::render_target_2d* rt = (system->state.is_subpass() ? system->get_rt(target_type::secondary) : system->get_rt(target_type::main));
				graphics::element_buffer* cube[2];
				system->get_primitives()->get_cube_buffers(cube);
				state.device->copy_target(mrt, 0, rt, 0);
				state.device->set_target(mrt, 0, 0, 0, 0);
				state.device->set_input_layout(layout);
				state.device->set_vertex_buffer(cube[(size_t)buffer_type::vertex]);
				state.device->set_index_buffer(cube[(size_t)buffer_type::index], graphics::format::r32_uint);
				state.device->set_blend_state(blend_additive);

				render_surface_lights();
				render_point_lights();
				render_spot_lights();
				render_line_lights();
				render_ambient();
			}
			void lighting::render_surface_maps(core::timer* time)
			{
				auto& data = lights.surfaces.top();
				if (data.empty())
					return;

				if (!surfaces.merger || !surfaces.subresource || !surfaces.input || !surfaces.output)
					set_surface_buffer_size(surfaces.size);

				state.scene->swap_mrt(target_type::main, surfaces.merger);
				state.scene->set_mrt(target_type::main, false);

				float elapsed_time = time->get_elapsed_mills();
				for (auto* light : data)
				{
					if (light->is_image_based())
						continue;

					graphics::texture_cube* cache = light->get_probe_cache();
					if (!cache)
					{
						cache = *state.device->create_texture_cube();
						light->set_probe_cache(cache);
					}
					else if (!light->tick.tick_event(elapsed_time) || light->tick.delay <= 0.0)
						continue;

					state.device->cubemap_push(surfaces.subresource, cache);
					light->locked = true;

					trigonometry::vector3 position = light->get_entity()->get_transform()->get_position() * light->offset;
					for (uint32_t j = 0; j < 6; j++)
					{
						trigonometry::cube_face face = (trigonometry::cube_face)j;
						state.scene->clear_mrt(target_type::main, true, true);
						system->set_view(light->view[j] = trigonometry::matrix4x4::create_look_at(face, position), light->projection, position, 90.0f, 1.0f, 0.1f, light->get_size().radius, render_culling::depth);
						state.scene->statistics.draw_calls += system->render(time, render_state::geometry, light->static_mask ? render_opt::constant : render_opt::none);
						state.device->cubemap_face(surfaces.subresource, face);
					}

					light->locked = false;
					state.device->cubemap_pop(surfaces.subresource);
				}

				state.scene->swap_mrt(target_type::main, nullptr);
				system->restore_view_buffer(nullptr);
			}
			void lighting::render_point_shadow_maps(core::timer* time)
			{
				auto& buffers = state.scene->get_points_mapping(); size_t counter = 0;
				for (auto* light : lights.points.top())
				{
					if (counter >= buffers.size())
						break;

					light->depth_map = nullptr;
					if (!light->shadow.enabled)
						continue;

					depth_cube_map* target = buffers[counter++];
					light->generate_origin();
					light->depth_map = target;

					state.device->set_target(target);
					state.device->clear_depth(target);
					system->set_view(trigonometry::matrix4x4::identity(), light->projection, light->get_entity()->get_transform()->get_position(), 90.0f, 1.0f, 0.1f, light->shadow.distance, render_culling::depth_cube);
					state.scene->statistics.draw_calls += system->render(time, render_state::depth_cube, render_opt::none);
				}
			}
			void lighting::render_spot_shadow_maps(core::timer* time)
			{
				auto& buffers = state.scene->get_spots_mapping(); size_t counter = 0;
				for (auto* light : lights.spots.top())
				{
					if (counter >= buffers.size())
						break;

					light->depth_map_view = nullptr;
					if (!light->shadow.enabled)
						continue;

					depth_map* target = buffers[counter++];
					light->generate_origin();
					light->depth_map_view = target;

					state.device->set_target(target);
					state.device->clear_depth(target);
					system->set_view(light->view, light->projection, light->get_entity()->get_transform()->get_position(), light->cutoff, 1.0f, 0.1f, light->shadow.distance, render_culling::depth);
					state.scene->statistics.draw_calls += system->render(time, render_state::depth, render_opt::backfaces);
				}
			}
			void lighting::render_line_shadow_maps(core::timer* time)
			{
				auto& buffers = state.scene->get_lines_mapping(); size_t counter = 0;
				for (auto it = lights.lines->begin(); it != lights.lines->end(); ++it)
				{
					auto* light = (components::line_light*)*it;
					if (counter >= buffers.size())
						break;

					light->depth_map_view = nullptr;
					if (!light->shadow.enabled || light->shadow.cascades < 1 || light->shadow.cascades > 6)
						continue;

					depth_cascade_map*& target = buffers[counter++];
					if (!target || target->size() < light->shadow.cascades)
						state.scene->generate_depth_cascades(&target, light->shadow.cascades);

					light->generate_origin();
					light->depth_map_view = target;

					for (size_t i = 0; i < target->size(); i++)
					{
						depth_map* cascade = (*target)[i];
						state.device->set_target(cascade);
						state.device->clear_depth(cascade);

						system->set_view(light->view[i], light->projection[i], 0.0f, 90.0f, 1.0f, -system->view.far_plane, system->view.far_plane, render_culling::disable);
						state.scene->statistics.draw_calls += system->render(time, render_state::depth, render_opt::none);
					}
				}

				system->restore_view_buffer(nullptr);
			}
			void lighting::render_surface_lights()
			{
				bool recursive = ambient_buffer.recursive > 0.0f;
				if (lights.surfaces.top().empty() || !(recursive && system->state.is_subpass()))
					return;

				trigonometry::vector3 position, scale;
				graphics::multi_render_target_2d* mrt = system->get_mrt(target_type::main);
				graphics::render_target_2d* rt = (system->state.is_subpass() ? system->get_rt(target_type::secondary) : system->get_rt(target_type::main));
				graphics::element_buffer* cube[2];
				system->get_primitives()->get_cube_buffers(cube);
				system->set_constant_buffer(render_buffer_type::view, pipelines.surface.viewer_buffer, VI_VS | VI_PS);
				state.device->set_structure_buffer(system->get_material_buffer(), pipelines.surface.materials, VI_PS);
				state.device->set_sampler_state(wrap_sampler, pipelines.surface.sampler, 5, VI_PS);
				state.device->set_texture_2d(mrt->get_target(1), pipelines.surface.normal_buffer, VI_PS);
				state.device->set_texture_2d(mrt->get_target(2), pipelines.surface.depth_buffer, VI_PS);
				state.device->set_texture_2d(mrt->get_target(3), pipelines.surface.surface_buffer, VI_PS);
				state.device->set_buffer(pipelines.surface.shader, pipelines.surface.environment_buffer, VI_VS | VI_PS);
				state.device->set_shader(pipelines.surface.shader, VI_VS | VI_PS);
				for (auto* light : lights.surfaces.top())
				{
					if (!light->get_probe_cache())
						continue;
					else if (recursive && light->locked)
						continue;

					entity* base = light->get_entity();
					apply_light_culling(light, base->get_radius(), &position, &scale);
					load_surface_buffer(&surface_buffer, light, position, scale);

					surface_buffer.lighting *= base->get_visibility(system->view);
					state.device->set_texture_cube(light->get_probe_cache(), pipelines.surface.environment_map, VI_PS);
					state.device->update_buffer(pipelines.surface.shader, &surface_buffer);
					state.device->draw_indexed((uint32_t)cube[(size_t)buffer_type::index]->get_elements(), 0, 0);
				}
				state.device->flush_texture(pipelines.surface.normal_buffer, 4, VI_PS);
			}
			void lighting::render_point_lights()
			{
				if (lights.points.top().empty())
					return;

				trigonometry::vector3 position, scale;
				graphics::multi_render_target_2d* mrt = system->get_mrt(target_type::main);
				graphics::render_target_2d* rt = (system->state.is_subpass() ? system->get_rt(target_type::secondary) : system->get_rt(target_type::main));
				graphics::element_buffer* cube[2];
				system->get_primitives()->get_cube_buffers(cube);

				system->set_constant_buffer(render_buffer_type::view, pipelines.point.viewer_buffer, VI_VS | VI_PS);
				state.device->set_structure_buffer(system->get_material_buffer(), pipelines.point.materials, VI_PS);
				state.device->set_sampler_state(wrap_sampler, pipelines.point.sampler, 4, VI_PS);
				state.device->set_sampler_state(depth_sampler, pipelines.point.depth_sampler, 1, VI_PS);
				state.device->set_sampler_state(depth_less_sampler, pipelines.point.depth_less_sampler, 1, VI_PS);
				state.device->set_texture_2d(rt->get_target(), pipelines.point.diffuse_buffer, VI_PS);
				state.device->set_texture_2d(mrt->get_target(1), pipelines.point.normal_buffer, VI_PS);
				state.device->set_texture_2d(mrt->get_target(2), pipelines.point.depth_buffer, VI_PS);
				state.device->set_texture_2d(mrt->get_target(3), pipelines.point.surface_buffer, VI_PS);
				state.device->set_buffer(pipelines.point.shader_base, pipelines.point.point_buffer, VI_VS | VI_PS);
				for (auto* light : lights.points.top())
				{
					entity* base = light->get_entity();
					apply_light_culling(light, base->get_radius(), &position, &scale);
					if (load_point_buffer(&point_buffer, light, position, scale, false))
					{
						graphics::texture_cube* depth_map = light->depth_map->get_target();
						state.device->set_shader(pipelines.point.shader_shadowed, VI_VS | VI_PS);
						state.device->set_texture_cube(depth_map, pipelines.point.depth_map_less, VI_PS);
					}
					else
						state.device->set_shader(pipelines.point.shader_base, VI_VS | VI_PS);

					point_buffer.lighting *= base->get_visibility(system->view);
					state.device->update_buffer(pipelines.point.shader_base, &point_buffer);
					state.device->draw_indexed((uint32_t)cube[(size_t)buffer_type::index]->get_elements(), 0, 0);
				}
				state.device->flush_texture(pipelines.point.diffuse_buffer, 5, VI_PS);
			}
			void lighting::render_spot_lights()
			{
				if (lights.spots.top().empty())
					return;

				trigonometry::vector3 position, scale;
				graphics::multi_render_target_2d* mrt = system->get_mrt(target_type::main);
				graphics::render_target_2d* rt = (system->state.is_subpass() ? system->get_rt(target_type::secondary) : system->get_rt(target_type::main));
				graphics::element_buffer* cube[2];
				system->get_primitives()->get_cube_buffers(cube);

				system->set_constant_buffer(render_buffer_type::view, pipelines.spot.viewer_buffer, VI_VS | VI_PS);
				state.device->set_structure_buffer(system->get_material_buffer(), pipelines.spot.materials, VI_PS);
				state.device->set_sampler_state(wrap_sampler, pipelines.spot.sampler, 4, VI_PS);
				state.device->set_sampler_state(depth_sampler, pipelines.spot.depth_sampler, 1, VI_PS);
				state.device->set_sampler_state(depth_less_sampler, pipelines.spot.depth_less_sampler, 1, VI_PS);
				state.device->set_texture_2d(rt->get_target(), pipelines.spot.diffuse_buffer, VI_PS);
				state.device->set_texture_2d(mrt->get_target(1), pipelines.spot.normal_buffer, VI_PS);
				state.device->set_texture_2d(mrt->get_target(2), pipelines.spot.depth_buffer, VI_PS);
				state.device->set_texture_2d(mrt->get_target(3), pipelines.spot.surface_buffer, VI_PS);
				state.device->set_buffer(pipelines.spot.shader_base, pipelines.spot.spot_buffer, VI_VS | VI_PS);
				for (auto* light : lights.spots.top())
				{
					entity* base = light->get_entity();
					apply_light_culling(light, base->get_radius(), &position, &scale);
					if (load_spot_buffer(&spot_buffer, light, position, scale, false))
					{
						graphics::texture_2d* depth_map = light->depth_map_view->get_target();
						state.device->set_texture_2d(depth_map, pipelines.spot.depth_map_less, VI_PS);
						state.device->set_shader(pipelines.spot.shader_shadowed, VI_VS | VI_PS);
					}
					else
						state.device->set_shader(pipelines.spot.shader_base, VI_VS | VI_PS);

					spot_buffer.lighting *= base->get_visibility(system->view);
					state.device->update_buffer(pipelines.spot.shader_base, &spot_buffer);
					state.device->draw_indexed((uint32_t)cube[(size_t)buffer_type::index]->get_elements(), 0, 0);
				}
				state.device->flush_texture(pipelines.spot.diffuse_buffer, 5, VI_PS);
			}
			void lighting::render_line_lights()
			{
				if (lights.lines->empty())
					return;

				graphics::multi_render_target_2d* mrt = system->get_mrt(target_type::main);
				graphics::render_target_2d* rt = (system->state.is_subpass() ? system->get_rt(target_type::secondary) : system->get_rt(target_type::main));
				system->set_constant_buffer(render_buffer_type::view, pipelines.line.viewer_buffer, VI_VS | VI_PS);
				state.device->set_structure_buffer(system->get_material_buffer(), pipelines.line.materials, VI_PS);
				state.device->set_sampler_state(wrap_sampler, pipelines.line.sampler, 4, VI_PS);
				state.device->set_sampler_state(depth_less_sampler, pipelines.line.depth_less_sampler, 1, VI_PS);
				state.device->set_rasterizer_state(back_rasterizer);
				state.device->set_depth_stencil_state(depth_stencil_none);
				state.device->set_vertex_buffer(system->get_primitives()->get_quad());
				state.device->set_texture_2d(rt->get_target(), pipelines.line.diffuse_buffer, VI_PS);
				state.device->set_texture_2d(mrt->get_target(1), pipelines.line.normal_buffer, VI_PS);
				state.device->set_texture_2d(mrt->get_target(2), pipelines.line.depth_buffer, VI_PS);
				state.device->set_texture_2d(mrt->get_target(3), pipelines.line.surface_buffer, VI_PS);
				state.device->set_buffer(pipelines.line.shader_base, pipelines.line.line_buffer, VI_VS | VI_PS);
				for (auto it = lights.lines->begin(); it != lights.lines->end(); ++it)
				{
					auto* light = (components::line_light*)*it;
					if (load_line_buffer(&line_buffer, light))
					{
						uint32_t size = (uint32_t)line_buffer.cascades;
						for (uint32_t i = 0; i < size; i++)
							state.device->set_texture_2d((*light->depth_map_view)[i]->get_target(), pipelines.line.depth_map + i, VI_PS);
						for (uint32_t i = size; i < 6; i++)
							state.device->set_texture_2d((*light->depth_map_view)[size - 1]->get_target(), pipelines.line.depth_map + i, VI_PS);
						state.device->set_shader(pipelines.line.shader_shadowed, VI_VS | VI_PS);
					}
					else
						state.device->set_shader(pipelines.line.shader_base, VI_VS | VI_PS);

					state.device->update_buffer(pipelines.line.shader_base, &line_buffer);
					state.device->draw(6, 0);
				}
				state.device->flush_texture(pipelines.line.diffuse_buffer, 11, VI_PS);
			}
			void lighting::render_ambient()
			{
				graphics::multi_render_target_2d* mrt = system->get_mrt(target_type::main);
				graphics::render_target_2d* rt = (system->state.is_subpass() ? system->get_rt(target_type::secondary) : system->get_rt(target_type::main));
				ambient_buffer.sky_offset = system->view.projection.inv() * trigonometry::matrix4x4::create_rotation(system->view.rotation);
				system->set_constant_buffer(render_buffer_type::view, pipelines.ambient.viewer_buffer, VI_VS | VI_PS);
				state.device->set_structure_buffer(system->get_material_buffer(), pipelines.ambient.materials, VI_PS);
				state.device->copy_texture_2d(mrt, 0, &lighting_map);
				state.device->set_blend_state(blend_overload);
				state.device->set_rasterizer_state(back_rasterizer);
				state.device->set_depth_stencil_state(depth_stencil_none);
				state.device->set_sampler_state(wrap_sampler, pipelines.ambient.sampler, 6, VI_PS);
				state.device->set_texture_2d(rt->get_target(), pipelines.ambient.diffuse_buffer, VI_PS);
				state.device->set_texture_2d(mrt->get_target(1), pipelines.ambient.normal_buffer, VI_PS);
				state.device->set_texture_2d(mrt->get_target(2), pipelines.ambient.depth_buffer, VI_PS);
				state.device->set_texture_2d(mrt->get_target(3), pipelines.ambient.surface_buffer, VI_PS);
				state.device->set_texture_2d(lighting_map, pipelines.ambient.light_map, VI_PS);
				state.device->set_texture_cube(sky_map, pipelines.ambient.sky_map, VI_PS);
				state.device->set_vertex_buffer(system->get_primitives()->get_quad());
				state.device->set_shader(pipelines.ambient.shader, VI_VS | VI_PS);
				state.device->set_buffer(pipelines.ambient.shader, pipelines.ambient.ambient_buffer, VI_VS | VI_PS);
				state.device->update_buffer(pipelines.ambient.shader, &ambient_buffer);
				state.device->draw(6, 0);
				state.device->flush_texture(pipelines.ambient.diffuse_buffer, 4, VI_PS);
			}
			void lighting::set_sky_map(graphics::texture_2d* cubemap)
			{
				core::memory::release(sky_map);
				core::memory::release(sky_base);

				sky_base = cubemap;
				if (sky_base != nullptr)
				{
					sky_map = *system->get_device()->create_texture_cube(sky_base);
					sky_base->add_ref();
				}
			}
			void lighting::set_surface_buffer_size(size_t new_size)
			{
				VI_ASSERT(system->get_scene() != nullptr, "scene should be set");

				scene_graph* scene = system->get_scene();
				graphics::graphics_device* device = system->get_device();
				graphics::multi_render_target_2d::desc f1 = scene->get_desc_mrt();
				f1.mip_levels = device->get_mip_level((uint32_t)surfaces.size, (uint32_t)surfaces.size);
				f1.width = (uint32_t)surfaces.size;
				f1.height = (uint32_t)surfaces.size;
				surface_buffer.mips = (float)f1.mip_levels;
				surfaces.size = new_size;

				core::memory::release(surfaces.merger);
				surfaces.merger = *device->create_multi_render_target_2d(f1);

				graphics::cubemap::desc i;
				i.source = surfaces.merger;
				i.mip_levels = f1.mip_levels;
				i.size = (uint32_t)surfaces.size;

				core::memory::release(surfaces.subresource);
				surfaces.subresource = *device->create_cubemap(i);

				graphics::render_target_2d::desc f2 = scene->get_desc_rt();
				f2.mip_levels = f1.mip_levels;
				f2.width = f1.width;
				f2.height = f1.height;

				core::memory::release(surfaces.output);
				surfaces.output = *device->create_render_target_2d(f2);

				core::memory::release(surfaces.input);
				surfaces.input = *device->create_render_target_2d(f2);
			}
			size_t lighting::render_pass(core::timer* time)
			{
				VI_ASSERT(system->get_scene() != nullptr, "scene should be set");
				if (system->state.is_set(render_opt::additive))
					return 0;

				state.device = system->get_device();
				state.scene = system->get_scene();
				if (system->state.is(render_state::geometry))
				{
					if (!system->state.is_subpass() && !system->state.is_set(render_opt::transparent))
					{
						if (shadows.tick.tick_event(time->get_elapsed_mills()))
						{
							render_point_shadow_maps(time);
							render_spot_shadow_maps(time);
							render_line_shadow_maps(time);
							system->restore_view_buffer(nullptr);
						}
						render_surface_maps(time);
					}

					render_result_buffers();
					system->restore_output();
				}

				return 1;
			}
			float lighting::get_dominant(const trigonometry::vector3& axis)
			{
				float max = axis.x;
				if (axis.y > max)
					max = axis.y;

				if (axis.z > max)
					max = axis.z;

				return max;
			}
			bool lighting::load_surface_buffer(isurface_buffer* dest, component* src, trigonometry::vector3& position, trigonometry::vector3& scale)
			{
				components::surface_light* light = (components::surface_light*)src;
				auto* entity = light->get_entity();
				auto* transform = entity->get_transform();
				auto& size = light->get_size();

				dest->transform = trigonometry::matrix4x4::create_translated_scale(position, scale) * system->view.view_projection;
				dest->position = transform->get_position();
				dest->lighting = light->diffuse.mul(light->emission);
				dest->scale = transform->get_scale();
				dest->parallax = (light->parallax ? 1.0f : 0.0f);
				dest->infinity = light->infinity;
				dest->attenuation.x = size.C1;
				dest->attenuation.y = size.C2;
				dest->range = size.radius;

				return true;
			}
			bool lighting::load_point_buffer(ipoint_buffer* dest, component* src, trigonometry::vector3& position, trigonometry::vector3& scale, bool reposition)
			{
				components::point_light* light = (components::point_light*)src;
				auto* entity = light->get_entity();
				auto* transform = entity->get_transform();
				auto& size = light->get_size();

				dest->transform = trigonometry::matrix4x4::create_translated_scale(position, scale) * system->view.view_projection;
				dest->position = (reposition ? position : transform->get_position());
				dest->lighting = light->diffuse.mul(light->emission);
				dest->attenuation.x = size.C1;
				dest->attenuation.y = size.C2;
				dest->range = size.radius;

				if (!light->shadow.enabled || !light->depth_map)
				{
					dest->softness = 0.0f;
					return false;
				}

				dest->softness = light->shadow.softness <= 0 ? 0 : (float)state.scene->get_conf().points_size / light->shadow.softness;
				dest->bias = light->shadow.bias;
				dest->distance = light->shadow.distance;
				dest->iterations = (float)light->shadow.iterations;
				dest->umbra = light->disperse;
				return true;
			}
			bool lighting::load_spot_buffer(ispot_buffer* dest, component* src, trigonometry::vector3& position, trigonometry::vector3& scale, bool reposition)
			{
				components::spot_light* light = (components::spot_light*)src;
				auto* entity = light->get_entity();
				auto* transform = entity->get_transform();
				auto& size = light->get_size();

				dest->transform = trigonometry::matrix4x4::create_translated_scale(position, scale) * system->view.view_projection;
				dest->view_projection = light->view * light->projection;
				dest->direction = transform->get_rotation().ddirection();
				dest->position = (reposition ? position : transform->get_position());
				dest->lighting = light->diffuse.mul(light->emission);
				dest->cutoff = compute::mathf::cos(compute::mathf::deg2rad() * light->cutoff * 0.5f);
				dest->attenuation.x = size.C1;
				dest->attenuation.y = size.C2;
				dest->range = size.radius;

				if (!light->shadow.enabled || !light->depth_map_view)
				{
					dest->softness = 0.0f;
					return false;
				}

				dest->softness = light->shadow.softness <= 0 ? 0 : (float)state.scene->get_conf().spots_size / light->shadow.softness;
				dest->bias = light->shadow.bias;
				dest->iterations = (float)light->shadow.iterations;
				dest->umbra = light->disperse;
				return true;
			}
			bool lighting::load_line_buffer(iline_buffer* dest, component* src)
			{
				components::line_light* light = (components::line_light*)src;
				dest->position = light->get_entity()->get_transform()->get_position().snormalize();
				dest->lighting = light->diffuse.mul(light->emission);
				dest->rlh_emission = light->sky.rlh_emission;
				dest->rlh_height = light->sky.rlh_height;
				dest->mie_emission = light->sky.mie_emission;
				dest->mie_height = light->sky.mie_height;
				dest->scatter_intensity = light->sky.intensity;
				dest->planet_radius = light->sky.inner_radius;
				dest->atmosphere_radius = light->sky.outer_radius;
				dest->mie_direction = light->sky.mie_direction;
				dest->sky_offset = ambient_buffer.sky_offset;

				if (!light->shadow.enabled || !light->depth_map_view)
				{
					dest->softness = 0.0f;
					return false;
				}

				dest->softness = light->shadow.softness <= 0 ? 0 : (float)state.scene->get_conf().lines_size / light->shadow.softness;
				dest->iterations = (float)light->shadow.iterations;
				dest->umbra = light->disperse;
				dest->bias = light->shadow.bias;
				dest->cascades = (float)std::min(light->shadow.cascades, (uint32_t)light->depth_map_view->size());

				size_t size = (size_t)dest->cascades;
				for (size_t i = 0; i < size; i++)
					dest->view_projection[i] = light->view[i] * light->projection[i];

				return size > 0;
			}
			void lighting::apply_light_culling(component* src, float range, trigonometry::vector3* position, trigonometry::vector3* scale)
			{
				auto* transform = src->get_entity()->get_transform();
				*position = transform->get_position();
				*scale = (range > 0.0f ? range : transform->get_scale());

				bool front = trigonometry::geometric::has_point_intersected_cube(*position, scale->mul(1.01f), system->view.position);
				state.device->set_rasterizer_state(front ? front_rasterizer : back_rasterizer);
				state.device->set_depth_stencil_state(front ? depth_stencil_greater : depth_stencil_less);
			}
			graphics::texture_cube* lighting::get_sky_map()
			{
				if (!sky_base)
					return nullptr;

				return sky_map;
			}
			graphics::texture_2d* lighting::get_sky_base()
			{
				if (!sky_map)
					return nullptr;

				return sky_base;
			}

			transparency::transparency(render_system* lab) : renderer(lab)
			{
				VI_ASSERT(system != nullptr, "render system should be set");
				VI_ASSERT(system->get_device() != nullptr, "graphics device should be set");

				graphics::graphics_device* device = system->get_device();
				depth_stencil = device->get_depth_stencil_state("doo_soo_lt");
				rasterizer = device->get_rasterizer_state("so_cback");
				blend = device->get_blend_state("bo_wrgba_one");
				sampler = device->get_sampler_state("a16_fa_wrap");
				layout = device->get_input_layout("vx_shape");

				pipeline.shader = *system->compile_shader("postprocessing/transparency", { }, sizeof(render_data));
				pipeline.materials = *device->get_shader_slot(pipeline.shader, "Materials");
				pipeline.sampler = *device->get_shader_sampler_slot(pipeline.shader, "DiffuseBuffer", "Sampler");
				pipeline.transparency_buffer = *device->get_shader_slot(pipeline.shader, "TransparencyBuffer");
				pipeline.diffuse_buffer = *device->get_shader_slot(pipeline.shader, "DiffuseBuffer");
				pipeline.depth_buffer = *device->get_shader_slot(pipeline.shader, "DepthBuffer");
				pipeline.ldiffuse_buffer = *device->get_shader_slot(pipeline.shader, "LDiffuseBuffer");
				pipeline.lnormal_buffer = *device->get_shader_slot(pipeline.shader, "LNormalBuffer");
				pipeline.ldepth_buffer = *device->get_shader_slot(pipeline.shader, "LDepthBuffer");
				pipeline.lsurface_buffer = *device->get_shader_slot(pipeline.shader, "LSurfaceBuffer");
			}
			transparency::~transparency()
			{
				system->free_shader(pipeline.shader);
				core::memory::release(merger);
				core::memory::release(input);
			}
			void transparency::resize_buffers()
			{
				VI_ASSERT(system->get_scene() != nullptr, "scene should be set");

				scene_graph* scene = system->get_scene();
				graphics::multi_render_target_2d::desc f1 = scene->get_desc_mrt();
				graphics::render_target_2d::desc f2 = scene->get_desc_rt();
				graphics::graphics_device* device = system->get_device();
				mip_levels[(size_t)target_type::main] = (float)f1.mip_levels;

				auto* renderer = system->get_renderer<lighting>();
				if (renderer != nullptr)
				{
					mip_levels[(size_t)target_type::secondary] = (float)device->get_mip_level((uint32_t)renderer->surfaces.size, (uint32_t)renderer->surfaces.size);
					f1.mip_levels = (uint32_t)mip_levels[(size_t)target_type::secondary];
					f1.width = (uint32_t)renderer->surfaces.size;
					f1.height = (uint32_t)renderer->surfaces.size;
					f2.mip_levels = f1.mip_levels;
					f2.width = f1.width;
					f2.height = f1.height;
				}

				core::memory::release(merger);
				merger = *device->create_multi_render_target_2d(f1);

				core::memory::release(input);
				input = *device->create_render_target_2d(f2);
			}
			size_t transparency::render_pass(core::timer* time)
			{
				VI_ASSERT(system->get_scene() != nullptr, "scene should be set");
				if (!system->state.is(render_state::geometry) || system->state.is_set(render_opt::transparent) || system->state.is_set(render_opt::additive))
					return 0;

				scene_graph* scene = system->get_scene();
				if (system->has_category(geo_category::additive))
					scene->statistics.draw_calls += system->render(time, render_state::geometry, system->state.get_opts() | render_opt::additive);

				if (!system->has_category(geo_category::transparent))
					return 0;

				graphics::multi_render_target_2d* main_mrt = system->get_mrt(target_type::main);
				graphics::multi_render_target_2d* mrt = (system->state.is_subpass() ? merger : system->get_mrt(target_type::secondary));
				graphics::render_target_2d* rt = (system->state.is_subpass() ? input : system->get_rt(target_type::main));
				graphics::graphics_device* device = system->get_device();
				render_data.mips = (system->state.is_subpass() ? mip_levels[(size_t)target_type::secondary] : mip_levels[(size_t)target_type::main]);

				scene->swap_mrt(target_type::main, mrt);
				scene->set_mrt(target_type::main, true);
				scene->statistics.draw_calls += system->render(time, render_state::geometry, system->state.get_opts() | render_opt::transparent);
				scene->swap_mrt(target_type::main, nullptr);

				device->set_structure_buffer(system->get_material_buffer(), pipeline.materials, VI_PS);
				device->copy_target(main_mrt, 0, rt, 0);
				device->generate_mips(rt->get_target());
				device->set_target(main_mrt, 0);
				device->clear(main_mrt, 0, 0, 0, 0);
				device->update_buffer(pipeline.shader, &render_data);
				device->set_depth_stencil_state(depth_stencil);
				device->set_blend_state(blend);
				device->set_rasterizer_state(rasterizer);
				device->set_input_layout(layout);
				device->set_sampler_state(sampler, pipeline.diffuse_buffer, 8, VI_PS);
				device->set_texture_2d(rt->get_target(), pipeline.diffuse_buffer, VI_PS);
				device->set_texture_2d(main_mrt->get_target(2), pipeline.depth_buffer, VI_PS);
				device->set_texture_2d(mrt->get_target(0), pipeline.ldiffuse_buffer, VI_PS);
				device->set_texture_2d(mrt->get_target(1), pipeline.lnormal_buffer, VI_PS);
				device->set_texture_2d(mrt->get_target(2), pipeline.ldepth_buffer, VI_PS);
				device->set_texture_2d(mrt->get_target(3), pipeline.lsurface_buffer, VI_PS);
				device->set_shader(pipeline.shader, VI_VS | VI_PS);
				device->set_buffer(pipeline.shader, pipeline.transparency_buffer, VI_VS | VI_PS);
				device->set_vertex_buffer(system->get_primitives()->get_quad());
				system->update_constant_buffer(render_buffer_type::render);
				device->draw(6, 0);
				device->flush_texture(pipeline.diffuse_buffer, 8, VI_PS);
				system->restore_output();
				return 1;
			}

			local_reflections::local_reflections(render_system* lab) : effect_renderer(lab)
			{
				pipelines.reflectance = *compile_effect("postprocessing/reflectance", { }, sizeof(reflectance));
				pipelines.gloss[0] = *compile_effect("postprocessing/gloss_x", { }, sizeof(gloss));
				pipelines.gloss[1] = *compile_effect("postprocessing/gloss_y", { });
				pipelines.additive = *compile_effect("postprocessing/additive", { });
			}
			void local_reflections::deserialize(core::schema* node)
			{
				VI_ASSERT(node != nullptr, "schema should be set");

				series::unpack(node->find("samples-1"), &reflectance.samples);
				series::unpack(node->find("samples-2"), &gloss.samples);
				series::unpack(node->find("intensity"), &reflectance.intensity);
				series::unpack(node->find("distance"), &reflectance.distance);
				series::unpack(node->find("cutoff"), &gloss.cutoff);
				series::unpack(node->find("blur"), &gloss.blur);
				series::unpack(node->find("deadzone"), &gloss.deadzone);
				series::unpack(node->find("mips"), &gloss.mips);
			}
			void local_reflections::serialize(core::schema* node)
			{
				VI_ASSERT(node != nullptr, "schema should be set");

				series::pack(node->set("samples-1"), reflectance.samples);
				series::pack(node->set("samples-2"), gloss.samples);
				series::pack(node->set("intensity"), reflectance.intensity);
				series::pack(node->set("distance"), reflectance.distance);
				series::pack(node->set("cutoff"), gloss.cutoff);
				series::pack(node->set("blur"), gloss.blur);
				series::pack(node->set("deadzone"), gloss.deadzone);
				series::pack(node->set("mips"), gloss.mips);
			}
			void local_reflections::render_effect(core::timer* time)
			{
				graphics::multi_render_target_2d* mrt = system->get_mrt(target_type::main);
				system->get_device()->generate_mips(mrt->get_target(0));

				gloss.mips = (float)get_mip_levels();
				gloss.texel[0] = 1.0f / get_width();
				gloss.texel[1] = 1.0f / get_height();

				render_merge(pipelines.reflectance, sampler_wrap, &reflectance);
				render_merge(pipelines.gloss[0], sampler_clamp, &gloss, 2);
				render_merge(pipelines.gloss[1], sampler_clamp, nullptr, 2);
				render_result(pipelines.additive, sampler_wrap);
			}

			local_illumination::local_illumination(render_system* lab) : effect_renderer(lab), emission_map(nullptr)
			{
				pipelines.stochastic = *compile_effect("postprocessing/stochastic", { }, sizeof(stochastic));
				pipelines.indirection = *compile_effect("postprocessing/indirection", { }, sizeof(indirection));
				pipelines.denoise[0] = *compile_effect("postprocessing/denoise_x", { }, sizeof(denoise));
				pipelines.denoise[1] = *compile_effect("postprocessing/denoise_y", { });
				pipelines.additive = *compile_effect("postprocessing/additive", { });
			}
			local_illumination::~local_illumination()
			{
				core::memory::release(emission_map);
			}
			void local_illumination::deserialize(core::schema* node)
			{
				VI_ASSERT(node != nullptr, "schema should be set");

				series::unpack(node->find("samples-1"), &indirection.samples);
				series::unpack(node->find("samples-2"), &denoise.samples);
				series::unpack(node->find("cutoff-1"), &indirection.cutoff);
				series::unpack(node->find("cutoff-2"), &denoise.cutoff);
				series::unpack(node->find("attenuation"), &indirection.attenuation);
				series::unpack(node->find("swing"), &indirection.swing);
				series::unpack(node->find("bias"), &indirection.bias);
				series::unpack(node->find("distance"), &indirection.distance);
				series::unpack(node->find("blur"), &denoise.blur);
			}
			void local_illumination::serialize(core::schema* node)
			{
				VI_ASSERT(node != nullptr, "schema should be set");

				series::pack(node->set("samples-1"), indirection.samples);
				series::pack(node->set("samples-2"), denoise.samples);
				series::pack(node->set("cutoff-1"), indirection.cutoff);
				series::pack(node->set("cutoff-2"), denoise.cutoff);
				series::pack(node->set("attenuation"), indirection.attenuation);
				series::pack(node->set("swing"), indirection.swing);
				series::pack(node->set("bias"), indirection.bias);
				series::pack(node->set("distance"), indirection.distance);
				series::pack(node->set("blur"), denoise.blur);
			}
			void local_illumination::render_effect(core::timer* time)
			{
				indirection.random[0] = compute::math<float>::random();
				indirection.random[1] = compute::math<float>::random();
				denoise.texel[0] = 1.0f / (float)get_width();
				denoise.texel[1] = 1.0f / (float)get_height();
				stochastic.texel[0] = (float)get_width();
				stochastic.texel[1] = (float)get_height();
				stochastic.frame_id++;

				float distance = indirection.distance;
				float swing = indirection.swing;
				float bias = indirection.bias;
				render_copy_from_main(0, emission_map);
				render_merge(pipelines.stochastic, sampler_wrap, &stochastic);
				render_texture(pipelines.indirection, "EmissionBuffer", emission_map);
				for (uint32_t i = 0; i < bounces; i++)
				{
					float bounce = (float)(i + 1);
					indirection.distance = distance * bounce;
					indirection.swing = swing / bounce;
					indirection.bias = bias * bounce;
					indirection.initial = i > 0 ? 0.0f : 1.0f;
					render_merge(pipelines.indirection, sampler_wrap, &indirection);
					if (i + 1 < bounces)
						render_copy_from_last(emission_map);
				}
				render_merge(pipelines.denoise[0], sampler_clamp, &denoise, 3);
				render_merge(pipelines.denoise[1], sampler_clamp, nullptr, 3);
				render_result(pipelines.additive, sampler_wrap);
				indirection.distance = distance;
				indirection.swing = swing;
				indirection.bias = bias;
			}
			void local_illumination::resize_effect()
			{
				VI_ASSERT(system->get_scene() != nullptr, "scene should be set");
				core::memory::release(emission_map);

				scene_graph* scene = system->get_scene();
				graphics::graphics_device* device = system->get_device();
				device->copy_texture_2d(scene->get_rt(target_type::main), 0, &emission_map);
			}

			local_ambient::local_ambient(render_system* lab) : effect_renderer(lab)
			{
				pipelines.shading = *compile_effect("postprocessing/shading", { }, sizeof(shading));
				pipelines.fibo[0] = *compile_effect("postprocessing/fibo_x", { }, sizeof(fibo));
				pipelines.fibo[1] = *compile_effect("postprocessing/fibo_y", { });
				pipelines.multiply = *compile_effect("postprocessing/multiply", { });
			}
			void local_ambient::deserialize(core::schema* node)
			{
				VI_ASSERT(node != nullptr, "schema should be set");

				series::unpack(node->find("samples-1"), &shading.samples);
				series::unpack(node->find("scale"), &shading.scale);
				series::unpack(node->find("intensity"), &shading.intensity);
				series::unpack(node->find("bias"), &shading.bias);
				series::unpack(node->find("radius"), &shading.radius);
				series::unpack(node->find("distance"), &shading.distance);
				series::unpack(node->find("fade"), &shading.fade);
				series::unpack(node->find("power"), &fibo.power);
				series::unpack(node->find("samples-2"), &fibo.samples);
				series::unpack(node->find("blur"), &fibo.blur);
			}
			void local_ambient::serialize(core::schema* node)
			{
				VI_ASSERT(node != nullptr, "schema should be set");

				series::pack(node->set("samples-1"), shading.samples);
				series::pack(node->set("scale"), shading.scale);
				series::pack(node->set("intensity"), shading.intensity);
				series::pack(node->set("bias"), shading.bias);
				series::pack(node->set("radius"), shading.radius);
				series::pack(node->set("distance"), shading.distance);
				series::pack(node->set("fade"), shading.fade);
				series::pack(node->set("power"), fibo.power);
				series::pack(node->set("samples-2"), fibo.samples);
				series::pack(node->set("blur"), fibo.blur);
			}
			void local_ambient::render_effect(core::timer* time)
			{
				fibo.texel[0] = 1.0f / get_width();
				fibo.texel[1] = 1.0f / get_height();

				render_merge(pipelines.shading, sampler_wrap, &shading);
				render_merge(pipelines.fibo[0], sampler_clamp, &fibo, 2);
				render_merge(pipelines.fibo[1], sampler_clamp, nullptr, 2);
				render_result(pipelines.multiply, sampler_wrap);
			}

			depth_of_field::depth_of_field(render_system* lab) : effect_renderer(lab)
			{
				*compile_effect("postprocessing/focus", { }, sizeof(focus));
			}
			void depth_of_field::deserialize(core::schema* node)
			{
				VI_ASSERT(node != nullptr, "schema should be set");

				series::unpack(node->find("distance"), &distance);
				series::unpack(node->find("time"), &time);
				series::unpack(node->find("base-radius"), &radius);
				series::unpack(node->find("radius"), &focus.radius);
				series::unpack(node->find("bokeh"), &focus.bokeh);
				series::unpack(node->find("scale"), &focus.scale);
				series::unpack(node->find("near-distance"), &focus.near_distance);
				series::unpack(node->find("near-range"), &focus.near_range);
				series::unpack(node->find("far-distance"), &focus.far_distance);
				series::unpack(node->find("far-range"), &focus.far_range);
			}
			void depth_of_field::serialize(core::schema* node)
			{
				VI_ASSERT(node != nullptr, "schema should be set");

				series::pack(node->set("distance"), distance);
				series::pack(node->set("time"), time);
				series::pack(node->set("base-radius"), radius);
				series::pack(node->set("radius"), focus.radius);
				series::pack(node->set("bokeh"), focus.bokeh);
				series::pack(node->set("scale"), focus.scale);
				series::pack(node->set("near-distance"), focus.near_distance);
				series::pack(node->set("near-range"), focus.near_range);
				series::pack(node->set("far-distance"), focus.far_distance);
				series::pack(node->set("far-range"), focus.far_range);
			}
			void depth_of_field::render_effect(core::timer* time)
			{
				VI_ASSERT(time != nullptr, "time should be set");
				if (distance > 0.0f)
					focus_at_nearest_target(time->get_step());

				focus.texel[0] = 1.0f / get_width();
				focus.texel[1] = 1.0f / get_height();
				render_result(nullptr, sampler_wrap, &focus);
			}
			void depth_of_field::focus_at_nearest_target(float step)
			{
				VI_ASSERT(system->get_scene() != nullptr, "scene should be set");

				trigonometry::ray origin;
				origin.origin = system->view.position;
				origin.direction = system->view.rotation.ddirection();

				bool change = false;
				for (auto& hit : system->get_scene()->query_by_ray<components::model>(origin))
				{
					float radius = hit.first->get_entity()->get_radius();
					float spacing = origin.origin.distance(hit.second) + radius / 2.0f;
					if (spacing <= focus.near_range || spacing + radius / 2.0f >= distance)
						continue;

					if (spacing < state.distance || state.distance <= 0.0f)
					{
						state.distance = spacing;
						state.range = radius;
						change = true;
					}
				};

				if (change)
				{
					state.radius = focus.radius;
					state.factor = 0.0f;
				}

				state.factor += time * step;
				if (state.factor > 1.0f)
					state.factor = 1.0f;

				if (state.distance > 0.0f)
				{
					state.distance += state.range / 2.0f + focus.far_range;
					focus.far_distance = state.distance;
					focus.radius = compute::math<float>::lerp(state.radius, radius, state.factor);
				}
				else
				{
					state.distance = 0.0f;
					if (state.factor >= 1.0f)
						focus.far_distance = state.distance;

					focus.radius = compute::math<float>::lerp(state.radius, 0.0f, state.factor);
				}

				if (focus.radius < 0.0f)
					focus.radius = 0.0f;
			}

			motion_blur::motion_blur(render_system* lab) : effect_renderer(lab), prev_diffuse_map(nullptr), velocity_map(nullptr)
			{
				pipelines.velocity = *compile_effect("postprocessing/velocity", { }, sizeof(velocity));
				pipelines.motion[0] = *compile_effect("postprocessing/motion_x", { }, sizeof(motion));
				pipelines.motion[1] = *compile_effect("postprocessing/motion_y", { });
			}
			motion_blur::~motion_blur()
			{
				core::memory::release(prev_diffuse_map);
				core::memory::release(velocity_map);
			}
			void motion_blur::deserialize(core::schema* node)
			{
				VI_ASSERT(node != nullptr, "schema should be set");

				series::unpack(node->find("power"), &velocity.power);
				series::unpack(node->find("threshold"), &velocity.threshold);
				series::unpack(node->find("samples"), &motion.samples);
				series::unpack(node->find("motion"), &motion.motion);
			}
			void motion_blur::serialize(core::schema* node)
			{
				VI_ASSERT(node != nullptr, "schema should be set");

				series::pack(node->set("power"), velocity.power);
				series::pack(node->set("threshold"), velocity.threshold);
				series::pack(node->set("samples"), motion.samples);
				series::pack(node->set("motion"), motion.motion);
			}
			void motion_blur::render_effect(core::timer* time)
			{
				VI_ASSERT(system->get_scene() != nullptr, "scene should be set");
				motion.texel[0] = 1.0f / get_width();
				motion.texel[1] = 1.0f / get_height();
				motion.motion = 10.0f;
				velocity.power = 6.0f;
				velocity.threshold = 0.50f;

				render_texture(pipelines.velocity, "PrevDiffuseBuffer", prev_diffuse_map);
				render_merge(pipelines.velocity, sampler_wrap, &velocity);
				render_copy_from_main(0, prev_diffuse_map);
				render_copy_from_last(velocity_map);
				render_copy_to_last(prev_diffuse_map);
				render_texture(pipelines.motion[0], "VelocityBuffer", velocity_map);
				render_merge(pipelines.motion[0], sampler_clamp, &motion);
				render_result(pipelines.motion[1], sampler_clamp);
			}
			void motion_blur::resize_effect()
			{
				core::memory::release(prev_diffuse_map);
				core::memory::release(velocity_map);

				scene_graph* scene = system->get_scene();
				auto* target = scene->get_mrt(target_type::main);
				auto* texture = target->get_target_2d(0);
				graphics::graphics_device* device = system->get_device();
				device->copy_texture_2d(target, 0, &prev_diffuse_map);
				device->copy_texture_2d(target, 0, &velocity_map);
			}

			bloom::bloom(render_system* lab) : effect_renderer(lab)
			{
				pipelines.bloom = *compile_effect("postprocessing/bloom", { }, sizeof(extraction));
				pipelines.fibo[0] = *compile_effect("postprocessing/fibo_x", { }, sizeof(fibo));
				pipelines.fibo[1] = *compile_effect("postprocessing/fibo_y", { });
				pipelines.additive = *compile_effect("postprocessing/additive", { });
			}
			void bloom::deserialize(core::schema* node)
			{
				series::unpack(node->find("intensity"), &extraction.intensity);
				series::unpack(node->find("threshold"), &extraction.threshold);
				series::unpack(node->find("power"), &fibo.power);
				series::unpack(node->find("samples"), &fibo.samples);
				series::unpack(node->find("blur"), &fibo.blur);
			}
			void bloom::serialize(core::schema* node)
			{
				series::pack(node->set("intensity"), extraction.intensity);
				series::pack(node->set("threshold"), extraction.threshold);
				series::pack(node->set("power"), fibo.power);
				series::pack(node->set("samples"), fibo.samples);
				series::pack(node->set("blur"), fibo.blur);
			}
			void bloom::render_effect(core::timer* time)
			{
				fibo.texel[0] = 1.0f / get_width();
				fibo.texel[1] = 1.0f / get_height();

				render_merge(pipelines.bloom, sampler_wrap, &extraction);
				render_merge(pipelines.fibo[0], sampler_clamp, &fibo, 3);
				render_merge(pipelines.fibo[1], sampler_clamp, nullptr, 3);
				render_result(pipelines.additive, sampler_wrap);
			}

			tone::tone(render_system* lab) : effect_renderer(lab)
			{
				pipelines.luminance = *compile_effect("postprocessing/luminance", { }, sizeof(luminance));
				pipelines.tone = *compile_effect("postprocessing/tone", { }, sizeof(mapping));
			}
			tone::~tone()
			{
				core::memory::release(lut_target);
				core::memory::release(lut_map);
			}
			void tone::deserialize(core::schema* node)
			{
				VI_ASSERT(node != nullptr, "schema should be set");

				series::unpack(node->find("grayscale"), &mapping.grayscale);
				series::unpack(node->find("aces"), &mapping.aces);
				series::unpack(node->find("filmic"), &mapping.filmic);
				series::unpack(node->find("lottes"), &mapping.lottes);
				series::unpack(node->find("reinhard"), &mapping.reinhard);
				series::unpack(node->find("reinhard2"), &mapping.reinhard2);
				series::unpack(node->find("unreal"), &mapping.unreal);
				series::unpack(node->find("uchimura"), &mapping.uchimura);
				series::unpack(node->find("ubrightness"), &mapping.ubrightness);
				series::unpack(node->find("usontrast"), &mapping.ucontrast);
				series::unpack(node->find("ustart"), &mapping.ustart);
				series::unpack(node->find("ulength"), &mapping.ulength);
				series::unpack(node->find("ublack"), &mapping.ublack);
				series::unpack(node->find("upedestal"), &mapping.upedestal);
				series::unpack(node->find("exposure"), &mapping.exposure);
				series::unpack(node->find("eintensity"), &mapping.eintensity);
				series::unpack(node->find("egamma"), &mapping.egamma);
				series::unpack(node->find("adaptation"), &mapping.adaptation);
				series::unpack(node->find("agray"), &mapping.agray);
				series::unpack(node->find("awhite"), &mapping.awhite);
				series::unpack(node->find("ablack"), &mapping.ablack);
				series::unpack(node->find("aspeed"), &mapping.aspeed);
			}
			void tone::serialize(core::schema* node)
			{
				VI_ASSERT(node != nullptr, "schema should be set");

				series::pack(node->set("grayscale"), mapping.grayscale);
				series::pack(node->set("aces"), mapping.aces);
				series::pack(node->set("filmic"), mapping.filmic);
				series::pack(node->set("lottes"), mapping.lottes);
				series::pack(node->set("reinhard"), mapping.reinhard);
				series::pack(node->set("reinhard2"), mapping.reinhard2);
				series::pack(node->set("unreal"), mapping.unreal);
				series::pack(node->set("uchimura"), mapping.uchimura);
				series::pack(node->set("ubrightness"), mapping.ubrightness);
				series::pack(node->set("usontrast"), mapping.ucontrast);
				series::pack(node->set("ustart"), mapping.ustart);
				series::pack(node->set("ulength"), mapping.ulength);
				series::pack(node->set("ublack"), mapping.ublack);
				series::pack(node->set("upedestal"), mapping.upedestal);
				series::pack(node->set("exposure"), mapping.exposure);
				series::pack(node->set("eintensity"), mapping.eintensity);
				series::pack(node->set("egamma"), mapping.egamma);
				series::pack(node->set("adaptation"), mapping.adaptation);
				series::pack(node->set("agray"), mapping.agray);
				series::pack(node->set("awhite"), mapping.awhite);
				series::pack(node->set("ablack"), mapping.ablack);
				series::pack(node->set("aspeed"), mapping.aspeed);
			}
			void tone::render_effect(core::timer* time)
			{
				if (mapping.adaptation > 0.0f)
					render_lut(time);

				render_result(pipelines.tone, sampler_wrap, &mapping);
			}
			void tone::render_lut(core::timer* time)
			{
				VI_ASSERT(time != nullptr, "time should be set");
				if (!lut_map || !lut_target)
					set_lut_size(1);

				graphics::multi_render_target_2d* mrt = system->get_mrt(target_type::main);
				graphics::graphics_device* device = system->get_device();
				device->generate_mips(mrt->get_target(0));
				device->copy_texture_2d(lut_target, 0, &lut_map);

				luminance.texel[0] = 1.0f / (float)get_width();
				luminance.texel[1] = 1.0f / (float)get_height();
				luminance.mips = (float)get_mip_levels();
				luminance.time = time->get_step() * mapping.aspeed;

				render_texture(pipelines.luminance, "LutBuffer", lut_map);
				render_output(lut_target);
				render_merge(pipelines.luminance, sampler_wrap, &luminance);
			}
			void tone::set_lut_size(size_t size)
			{
				VI_ASSERT(system->get_scene() != nullptr, "scene should be set");
				core::memory::release(lut_target);
				core::memory::release(lut_map);

				scene_graph* scene = system->get_scene();
				graphics::graphics_device* device = system->get_device();
				graphics::render_target_2d::desc rt = scene->get_desc_rt();
				rt.mip_levels = device->get_mip_level((uint32_t)size, (uint32_t)size);
				rt.format_mode = graphics::format::r16_float;
				rt.width = (uint32_t)size;
				rt.height = (uint32_t)size;

				lut_target = *device->create_render_target_2d(rt);
				device->copy_texture_2d(lut_target, 0, &lut_map);
			}

			glitch::glitch(render_system* lab) : effect_renderer(lab), scan_line_jitter(0), vertical_jump(0), horizontal_shake(0), color_drift(0)
			{
				*compile_effect("postprocessing/glitch", { }, sizeof(distortion));
			}
			void glitch::deserialize(core::schema* node)
			{
				VI_ASSERT(node != nullptr, "schema should be set");

				series::unpack(node->find("scanline-jitter"), &scan_line_jitter);
				series::unpack(node->find("vertical-jump"), &vertical_jump);
				series::unpack(node->find("horizontal-shake"), &horizontal_shake);
				series::unpack(node->find("color-drift"), &color_drift);
				series::unpack(node->find("horizontal-shake"), &horizontal_shake);
				series::unpack(node->find("elapsed-time"), &distortion.elapsed_time);
				series::unpack(node->find("scanline-jitter-displacement"), &distortion.scan_line_jitter_displacement);
				series::unpack(node->find("scanline-jitter-threshold"), &distortion.scan_line_jitter_threshold);
				series::unpack(node->find("vertical-jump-amount"), &distortion.vertical_jump_amount);
				series::unpack(node->find("vertical-jump-time"), &distortion.vertical_jump_time);
				series::unpack(node->find("color-drift-amount"), &distortion.color_drift_amount);
				series::unpack(node->find("color-drift-time"), &distortion.color_drift_time);
			}
			void glitch::serialize(core::schema* node)
			{
				VI_ASSERT(node != nullptr, "schema should be set");

				series::pack(node->set("scanline-jitter"), scan_line_jitter);
				series::pack(node->set("vertical-jump"), vertical_jump);
				series::pack(node->set("horizontal-shake"), horizontal_shake);
				series::pack(node->set("color-drift"), color_drift);
				series::pack(node->set("horizontal-shake"), horizontal_shake);
				series::pack(node->set("elapsed-time"), distortion.elapsed_time);
				series::pack(node->set("scanline-jitter-displacement"), distortion.scan_line_jitter_displacement);
				series::pack(node->set("scanline-jitter-threshold"), distortion.scan_line_jitter_threshold);
				series::pack(node->set("vertical-jump-amount"), distortion.vertical_jump_amount);
				series::pack(node->set("vertical-jump-time"), distortion.vertical_jump_time);
				series::pack(node->set("color-drift-amount"), distortion.color_drift_amount);
				series::pack(node->set("color-drift-time"), distortion.color_drift_time);
			}
			void glitch::render_effect(core::timer* time)
			{
				if (distortion.elapsed_time >= 32000.0f)
					distortion.elapsed_time = 0.0f;

				float step = time->get_step() * 10.0f;
				distortion.elapsed_time += step * 10.0f;
				distortion.vertical_jump_amount = vertical_jump;
				distortion.vertical_jump_time += step * vertical_jump * 11.3f;
				distortion.scan_line_jitter_threshold = compute::mathf::saturate(1.0f - scan_line_jitter * 1.2f);
				distortion.scan_line_jitter_displacement = 0.002f + compute::mathf::pow(scan_line_jitter, 3) * 0.05f;
				distortion.horizontal_shake = horizontal_shake * 0.2f;
				distortion.color_drift_amount = color_drift * 0.04f;
				distortion.color_drift_time = distortion.elapsed_time * 606.11f;
				render_result(nullptr, sampler_wrap, &distortion);
			}

			user_interface::user_interface(render_system* lab) : user_interface(lab, heavy_application::has_instance() ? heavy_application::get()->fetch_ui() : nullptr, heavy_application::has_instance() ? heavy_application::get()->activity : nullptr)
			{
			}
			user_interface::user_interface(render_system* lab, gui::context* new_context, graphics::activity* new_activity) : renderer(lab), activity(new_activity), context(new_context)
			{
				VI_ASSERT(system != nullptr, "render system should be set");
				VI_ASSERT(system->get_device() != nullptr, "graphics device should be set");
				VI_ASSERT(activity != nullptr, "activity should be set");
				VI_ASSERT(context != nullptr, "context should be set");
			}
			size_t user_interface::render_pass(core::timer* timer)
			{
				VI_ASSERT(context != nullptr, "context should be set");
				if (!system->state.is(render_state::geometry) || system->state.is_subpass() || system->state.is_set(render_opt::transparent) || system->state.is_set(render_opt::additive))
					return 0;

				context->update_events(activity);
				context->render_lists(system->get_mrt(target_type::main)->get_target(0));
				system->restore_output();

				return 1;
			}
			gui::context* user_interface::get_context()
			{
				return context;
			}
		}
	}
}
