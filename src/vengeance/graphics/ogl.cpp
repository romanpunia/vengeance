#include "ogl.h"
#ifdef VI_GL
#define SHADER_VERTEX ".text.vertex.gz"
#define SHADER_PIXEL ".text.pixel.gz"
#define SHADER_GEOMETRY ".text.geometry.gz"
#define SHADER_COMPUTE ".text.compute.gz"
#define SHADER_HULL ".text.hull.gz"
#define SHADER_DOMAIN ".text.domain.gz"
#define REG_EXCHANGE(name, value) { if (regs.name == value) return; regs.name = value; }
#define REG_EXCHANGE_T2(name, value1, value2) { if (std::get<0>(regs.name) == value1 && std::get<1>(regs.name) == value2) return; regs.name = std::make_tuple(value1, value2); }
#define REG_EXCHANGE_T3(name, value1, value2, value3) { if (std::get<0>(regs.name) == value1 && std::get<1>(regs.name) == value2 && std::get<2>(regs.name) == value3) return; regs.name = std::make_tuple(value1, value2, value3); }
#define OGL_OFFSET(i) (GLvoid*)(i)
#define OGL_VOFFSET(i) ((char*)nullptr + (i))
#define OGL_INLINE(code) #code
#pragma warning(push)
#pragma warning(disable: 4996)

namespace
{
	template <class t>
	static inline void rehash(uint64_t& seed, const t& value)
	{
		seed ^= std::hash<t>()(value) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
	}
	static int64_t d3d_get_coord_y(int64_t y, int64_t height, int64_t window_height)
	{
		return window_height - height - y;
	}
	static int64_t ogl_get_coord_y(int64_t y, int64_t height, int64_t window_height)
	{
		return window_height - height - y;
	}
	static void ogl_copy_texture43(GLenum target, GLuint src_name, GLuint dest_name, GLint width, GLint height)
	{
		glCopyImageSubData(src_name, target, 0, 0, 0, 0, dest_name, target, 0, 0, 0, 0, width, height, 1);
	}
	static void ogl_copy_texture30(GLuint src_name, GLuint dest_name, GLint width, GLint height)
	{
		GLint last_frame_buffer = 0;
		glGetIntegerv(GL_FRAMEBUFFER_BINDING, &last_frame_buffer);

		GLuint frame_buffer = 0;
		glGenFramebuffers(1, &frame_buffer);
		glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
		glFramebufferTexture(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, src_name, 0);
		glFramebufferTexture(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, dest_name, 0);
		glNamedFramebufferDrawBuffer(frame_buffer, GL_COLOR_ATTACHMENT1);
		glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
		glBindFramebuffer(GL_FRAMEBUFFER, last_frame_buffer);
		glDeleteFramebuffers(1, &frame_buffer);
	}
	static void ogl_copy_texture_face_2d30(vitex::trigonometry::cube_face face, GLuint src_name, GLuint dest_name, GLint width, GLint height)
	{
		GLint last_frame_buffer = 0;
		glGetIntegerv(GL_FRAMEBUFFER_BINDING, &last_frame_buffer);

		GLuint frame_buffer = 0;
		glGenFramebuffers(1, &frame_buffer);
		glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
		glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + (uint32_t)face, src_name, 0);
		glFramebufferTexture(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, dest_name, 0);
		glNamedFramebufferDrawBuffer(frame_buffer, GL_COLOR_ATTACHMENT1);
		glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
		glBindFramebuffer(GL_FRAMEBUFFER, last_frame_buffer);
		glDeleteFramebuffers(1, &frame_buffer);
	}
	static vitex::graphics::graphics_exception get_exception(const char* scope_text)
	{
		GLenum result_code = glGetError();
		vitex::core::string text = scope_text;
		switch (result_code)
		{
			case GL_INVALID_ENUM:
				text += " CAUSING invalid enum parameter";
				break;
			case GL_INVALID_VALUE:
				text += " CAUSING invalid argument parameter";
				break;
			case GL_INVALID_OPERATION:
				text += " CAUSING invalid operation";
				break;
			case GL_STACK_OVERFLOW:
				text += " CAUSING stack overflow";
				break;
			case GL_STACK_UNDERFLOW:
				text += " CAUSING stack underflow";
				break;
			case GL_OUT_OF_MEMORY:
				text += " CAUSING out of memory";
				break;
			case GL_INVALID_FRAMEBUFFER_OPERATION:
				text += " CAUSING invalid frame buffer operation";
				break;
			case GL_CONTEXT_LOST:
				text += " CAUSING graphics system reset";
				break;
			case GL_TABLE_TOO_LARGE:
				text += " CAUSING table too large";
				break;
			case GL_NO_ERROR:
			default:
				text += " CAUSING internal graphics error";
				break;
		}
		return vitex::graphics::graphics_exception((int)result_code, std::move(text));
	}
}

namespace vitex
{
	namespace graphics
	{
		namespace ogl
		{
			ogl_frame_buffer::ogl_frame_buffer(GLuint targets) : count(targets)
			{
			}
			void ogl_frame_buffer::cleanup()
			{
				glDeleteFramebuffers(1, &buffer);
				glDeleteTextures(count, texture);
			}

			ogl_depth_stencil_state::ogl_depth_stencil_state(const desc& i) : depth_stencil_state(i)
			{
			}
			ogl_depth_stencil_state::~ogl_depth_stencil_state()
			{
			}
			void* ogl_depth_stencil_state::get_resource() const
			{
				return (void*)&state;
			}

			ogl_rasterizer_state::ogl_rasterizer_state(const desc& i) : rasterizer_state(i)
			{
			}
			ogl_rasterizer_state::~ogl_rasterizer_state()
			{
			}
			void* ogl_rasterizer_state::get_resource() const
			{
				return (void*)&state;
			}

			ogl_blend_state::ogl_blend_state(const desc& i) : blend_state(i)
			{
			}
			ogl_blend_state::~ogl_blend_state()
			{
			}
			void* ogl_blend_state::get_resource() const
			{
				return (void*)&state;
			}

			ogl_sampler_state::ogl_sampler_state(const desc& i) : sampler_state(i)
			{
			}
			ogl_sampler_state::~ogl_sampler_state()
			{
				glDeleteSamplers(1, &resource);
			}
			void* ogl_sampler_state::get_resource() const
			{
				return (void*)(intptr_t)resource;
			}

			ogl_input_layout::ogl_input_layout(const desc& i) : input_layout(i)
			{
				const auto layout_size = [this]()
				{
					size_t size = 0;
					for (auto item : vertex_layout)
						size += item.second.size();
					return size;
				};

				for (size_t j = 0; j < i.attributes.size(); j++)
				{
					const attribute& it = i.attributes[j];
					GLenum format = GL_INVALID_ENUM;
					size_t stride = it.aligned_byte_offset;
					GLint size = it.components;
					GLint normalize = GL_FALSE;
					size_t buffer_slot = (size_t)it.slot;
					bool per_vertex = it.per_vertex;

					switch (it.format)
					{
						case attribute_type::byte:
							format = GL_BYTE;
							normalize = GL_TRUE;
							break;
						case attribute_type::ubyte:
							format = GL_UNSIGNED_BYTE;
							normalize = GL_TRUE;
							break;
						case attribute_type::half:
							format = GL_HALF_FLOAT;
							break;
						case attribute_type::floatf:
							format = GL_FLOAT;
							break;
						case attribute_type::matrix:
							format = GL_FLOAT;
							size = 4;
							break;
						case attribute_type::intf:
							format = GL_INT;
							break;
						case attribute_type::uint:
							format = GL_UNSIGNED_INT;
							break;
						default:
							break;
					}

					auto& layout = vertex_layout[buffer_slot];
					if (it.format == attribute_type::matrix)
					{
						for (size_t k = 0; k < 4; k++)
						{
							size_t offset = layout_size(), substride = stride + sizeof(float) * size * k;
							layout.emplace_back([offset, format, normalize, substride, size, per_vertex](size_t width)
							{
								glEnableVertexAttribArray((GLuint)offset);
								glVertexAttribPointer((GLuint)offset, size, format, normalize, (GLsizei)width, OGL_VOFFSET(substride));
								glVertexAttribDivisor((GLuint)offset, per_vertex ? 0 : 1);
							});
						}
					}
					else
					{
						size_t offset = layout_size();
						layout.emplace_back([offset, format, normalize, stride, size, per_vertex](size_t width)
						{
							glEnableVertexAttribArray((GLuint)offset);
							glVertexAttribPointer((GLuint)offset, size, format, normalize, (GLsizei)width, OGL_VOFFSET(stride));
							glVertexAttribDivisor((GLuint)offset, per_vertex ? 0 : 1);
						});
					}
				}
			}
			ogl_input_layout::~ogl_input_layout()
			{
				for (auto& item : layouts)
					glDeleteVertexArrays(1, &item.second);

				if (dynamic_resource != GL_NONE)
					glDeleteVertexArrays(1, &dynamic_resource);
			}
			void* ogl_input_layout::get_resource() const
			{
				return (void*)this;
			}
			core::string ogl_input_layout::get_layout_hash(ogl_element_buffer** buffers, uint32_t count)
			{
				core::string hash;
				if (!buffers || !count)
					return hash;

				for (uint32_t i = 0; i < count; i++)
					hash += core::to_string((uintptr_t)(void*)buffers[i]);

				return hash;
			}

			ogl_shader::ogl_shader(const desc& i) : shader(i), compiled(false)
			{
			}
			ogl_shader::~ogl_shader()
			{
				for (auto& program : programs)
				{
					auto* device = program.second;
					auto it = device->regs.programs.begin();
					while (it != device->regs.programs.end())
					{
						if (it->second == program.first)
						{
							device->regs.programs.erase(it);
							it = device->regs.programs.begin();
						}
						else
							++it;
					}
					glDeleteProgram(program.first);
				}

				glDeleteShader(vertex_shader);
				glDeleteShader(pixel_shader);
				glDeleteShader(geometry_shader);
				glDeleteShader(domain_shader);
				glDeleteShader(hull_shader);
				glDeleteShader(compute_shader);
			}
			bool ogl_shader::is_valid() const
			{
				return compiled;
			}

			ogl_element_buffer::ogl_element_buffer(const desc& i) : element_buffer(i)
			{
			}
			ogl_element_buffer::~ogl_element_buffer()
			{
				for (auto& link : bindings)
				{
					auto* layout = link.second;
					if (layout->dynamic_resource == link.first)
						layout->dynamic_resource = GL_NONE;

					auto it = layout->layouts.begin();
					while (it != layout->layouts.end())
					{
						if (it->second == link.first)
						{
							layout->layouts.erase(it);
							it = layout->layouts.begin();
						}
						else
							++it;
					}
					glDeleteVertexArrays(1, &link.first);
				}

				glDeleteBuffers(1, &resource);
			}
			void* ogl_element_buffer::get_resource() const
			{
				return (void*)(intptr_t)resource;
			}

			ogl_mesh_buffer::ogl_mesh_buffer(const desc& i) : mesh_buffer(i)
			{
			}
			trigonometry::vertex* ogl_mesh_buffer::get_elements(graphics_device* device) const
			{
				VI_ASSERT(device != nullptr, "graphics device should be set");

				mapped_subresource resource;
				device->map(vertex_buffer, resource_map::write, &resource);

				trigonometry::vertex* vertices = core::memory::allocate<trigonometry::vertex>(sizeof(trigonometry::vertex) * (uint32_t)vertex_buffer->get_elements());
				memcpy(vertices, resource.pointer, (size_t)vertex_buffer->get_elements() * sizeof(trigonometry::vertex));

				device->unmap(vertex_buffer, &resource);
				return vertices;
			}

			ogl_skin_mesh_buffer::ogl_skin_mesh_buffer(const desc& i) : skin_mesh_buffer(i)
			{
			}
			trigonometry::skin_vertex* ogl_skin_mesh_buffer::get_elements(graphics_device* device) const
			{
				VI_ASSERT(device != nullptr, "graphics device should be set");

				mapped_subresource resource;
				device->map(vertex_buffer, resource_map::write, &resource);

				trigonometry::skin_vertex* vertices = core::memory::allocate<trigonometry::skin_vertex>(sizeof(trigonometry::skin_vertex) * (uint32_t)vertex_buffer->get_elements());
				memcpy(vertices, resource.pointer, (size_t)vertex_buffer->get_elements() * sizeof(trigonometry::skin_vertex));

				device->unmap(vertex_buffer, &resource);
				return vertices;
			}

			ogl_instance_buffer::ogl_instance_buffer(const desc& i) : instance_buffer(i)
			{
			}
			ogl_instance_buffer::~ogl_instance_buffer()
			{
				if (device != nullptr && sync)
					device->clear_buffer(this);
			}

			ogl_texture_2d::ogl_texture_2d() : texture_2d()
			{
			}
			ogl_texture_2d::ogl_texture_2d(const desc& i) : texture_2d(i)
			{
			}
			ogl_texture_2d::~ogl_texture_2d()
			{
				glDeleteTextures(1, &resource);
			}
			void* ogl_texture_2d::get_resource() const
			{
				return (void*)(intptr_t)resource;
			}

			ogl_texture_3d::ogl_texture_3d() : texture_3d()
			{
			}
			ogl_texture_3d::~ogl_texture_3d()
			{
				glDeleteTextures(1, &resource);
			}
			void* ogl_texture_3d::get_resource()
			{
				return (void*)(intptr_t)resource;
			}

			ogl_texture_cube::ogl_texture_cube() : texture_cube()
			{
			}
			ogl_texture_cube::ogl_texture_cube(const desc& i) : texture_cube(i)
			{
			}
			ogl_texture_cube::~ogl_texture_cube()
			{
				glDeleteTextures(1, &resource);
			}
			void* ogl_texture_cube::get_resource() const
			{
				return (void*)(intptr_t)resource;
			}

			ogl_depth_target_2d::ogl_depth_target_2d(const desc& i) : graphics::depth_target_2d(i)
			{
			}
			ogl_depth_target_2d::~ogl_depth_target_2d()
			{
				glDeleteFramebuffers(1, &frame_buffer);
				glDeleteTextures(1, &depth_texture);
			}
			void* ogl_depth_target_2d::get_resource() const
			{
				return (void*)(intptr_t)frame_buffer;
			}
			uint32_t ogl_depth_target_2d::get_width() const
			{
				return (uint32_t)viewarea.width;
			}
			uint32_t ogl_depth_target_2d::get_height() const
			{
				return (uint32_t)viewarea.height;
			}

			ogl_depth_target_cube::ogl_depth_target_cube(const desc& i) : graphics::depth_target_cube(i)
			{
			}
			ogl_depth_target_cube::~ogl_depth_target_cube()
			{
				glDeleteFramebuffers(1, &frame_buffer);
				glDeleteTextures(1, &depth_texture);
			}
			void* ogl_depth_target_cube::get_resource() const
			{
				return (void*)(intptr_t)frame_buffer;
			}
			uint32_t ogl_depth_target_cube::get_width() const
			{
				return (uint32_t)viewarea.width;
			}
			uint32_t ogl_depth_target_cube::get_height() const
			{
				return (uint32_t)viewarea.height;
			}

			ogl_render_target_2d::ogl_render_target_2d(const desc& i) : render_target_2d(i), frame_buffer(1)
			{
			}
			ogl_render_target_2d::~ogl_render_target_2d()
			{
				glDeleteTextures(1, &depth_texture);
				frame_buffer.cleanup();
			}
			void* ogl_render_target_2d::get_target_buffer() const
			{
				return (void*)&frame_buffer;
			}
			void* ogl_render_target_2d::get_depth_buffer() const
			{
				return (void*)(intptr_t)depth_texture;
			}
			uint32_t ogl_render_target_2d::get_width() const
			{
				return (uint32_t)viewarea.width;
			}
			uint32_t ogl_render_target_2d::get_height() const
			{
				return (uint32_t)viewarea.height;
			}

			ogl_multi_render_target_2d::ogl_multi_render_target_2d(const desc& i) : multi_render_target_2d(i), frame_buffer((GLuint)i.target)
			{
			}
			ogl_multi_render_target_2d::~ogl_multi_render_target_2d()
			{
				glDeleteTextures(1, &depth_texture);
				frame_buffer.cleanup();
			}
			void* ogl_multi_render_target_2d::get_target_buffer() const
			{
				return (void*)&frame_buffer;
			}
			void* ogl_multi_render_target_2d::get_depth_buffer() const
			{
				return (void*)(intptr_t)depth_texture;
			}
			uint32_t ogl_multi_render_target_2d::get_width() const
			{
				return (uint32_t)viewarea.width;
			}
			uint32_t ogl_multi_render_target_2d::get_height() const
			{
				return (uint32_t)viewarea.height;
			}

			ogl_render_target_cube::ogl_render_target_cube(const desc& i) : render_target_cube(i), frame_buffer(1)
			{
			}
			ogl_render_target_cube::~ogl_render_target_cube()
			{
				glDeleteTextures(1, &depth_texture);
				frame_buffer.cleanup();
			}
			void* ogl_render_target_cube::get_target_buffer() const
			{
				return (void*)&frame_buffer;
			}
			void* ogl_render_target_cube::get_depth_buffer() const
			{
				return (void*)(intptr_t)depth_texture;
			}
			uint32_t ogl_render_target_cube::get_width() const
			{
				return (uint32_t)viewarea.width;
			}
			uint32_t ogl_render_target_cube::get_height() const
			{
				return (uint32_t)viewarea.height;
			}

			ogl_multi_render_target_cube::ogl_multi_render_target_cube(const desc& i) : multi_render_target_cube(i), frame_buffer((GLuint)i.target)
			{
			}
			ogl_multi_render_target_cube::~ogl_multi_render_target_cube()
			{
				glDeleteTextures(1, &depth_texture);
				frame_buffer.cleanup();
			}
			void* ogl_multi_render_target_cube::get_target_buffer() const
			{
				return (void*)&frame_buffer;
			}
			void* ogl_multi_render_target_cube::get_depth_buffer() const
			{
				return (void*)(intptr_t)depth_texture;
			}
			uint32_t ogl_multi_render_target_cube::get_width() const
			{
				return (uint32_t)viewarea.width;
			}
			uint32_t ogl_multi_render_target_cube::get_height() const
			{
				return (uint32_t)viewarea.height;
			}

			ogl_cubemap::ogl_cubemap(const desc& i) : cubemap(i)
			{
				VI_ASSERT(i.source != nullptr, "source should be set");
				VI_ASSERT(i.target < i.source->get_target_count(), "targets count should be less than %i", (int)i.source->get_target_count());

				ogl_texture_2d* target = (ogl_texture_2d*)i.source->get_target_2d(i.target);
				VI_ASSERT(target != nullptr && target->resource != GL_NONE, "render target should be valid");
				VI_ASSERT(!((ogl_frame_buffer*)i.source->get_target_buffer())->backbuffer, "cannot copy from backbuffer directly");

				glGenFramebuffers(1, &frame_buffer);
				source = target->resource;
				options.size_format = ogl_device::get_sized_format(target->get_format_mode());
				options.format_mode = target->get_format_mode();
			}
			ogl_cubemap::~ogl_cubemap()
			{
				glDeleteFramebuffers(1, &frame_buffer);
			}

			ogl_query::ogl_query() : query()
			{
			}
			ogl_query::~ogl_query()
			{
				glDeleteQueries(1, &async);
			}
			void* ogl_query::get_resource() const
			{
				return (void*)(intptr_t)async;
			}

			ogl_device::ogl_device(const desc& i) : graphics_device(i), shader_version(""), window(i.window), context(nullptr)
			{
				if (!window)
				{
					VI_ASSERT(virtual_window != nullptr, "cannot initialize virtual activity for device");
					window = virtual_window;
				}
				else
				{
					core::memory::release(virtual_window);
					virtual_window = window;
					virtual_window->add_ref();
				}

				int r, g, b, a;
				get_back_buffer_size(i.buffer_format, &r, &g, &b, &a);
				VI_PANIC(video::glew::set_swap_parameters(r, g, b, a, i.debug), "OGL configuration failed");
				if (!window->get_handle())
				{
					window->apply_configuration(backend);
					VI_PANIC(window->get_handle(), "activity creation failed %s", window->get_error().c_str());
				}

				context = video::glew::create_context(window);
				VI_PANIC(context != nullptr, "OGL context creation failed %s", window->get_error().c_str());
				set_as_current_device();

				static const GLenum error_code = glewInit();
				VI_PANIC(error_code == GLEW_OK, "OGL extension layer initialization failed reason: %i", (int)error_code);
				if (i.debug)
				{
					glEnable(GL_DEBUG_OUTPUT);
#ifndef VI_APPLE
					glDebugMessageCallback(debug_message, nullptr);
					glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
#endif
				}

				regs.programs[get_program_hash()] = GL_NONE;
				set_shader_model(i.shader_mode == shader_model::any ? get_supported_shader_model() : i.shader_mode);
				resize_buffers(i.buffer_width, i.buffer_height);
				create_states();

				glEnable(GL_TEXTURE_2D);
				glEnable(GL_TEXTURE_3D);
				glEnable(GL_TEXTURE_CUBE_MAP);
			}
			ogl_device::~ogl_device()
			{
				release_proxy();
				for (auto& it : regs.programs)
					glDeleteProgram(it.second);

				glDeleteShader(immediate.vertex_shader);
				glDeleteShader(immediate.pixel_shader);
				glDeleteProgram(immediate.program);
				glDeleteVertexArrays(1, &immediate.vertex_array);
				glDeleteBuffers(1, &immediate.vertex_buffer);
				glDeleteBuffers(1, &immediate.constant_buffer);
				video::glew::destroy_context(context);
			}
			void ogl_device::set_as_current_device()
			{
				video::glew::set_context(window, context);
				set_vsync_mode(vsync_mode);
			}
			void ogl_device::set_shader_model(shader_model model)
			{
				shader_gen = model;
				if (shader_gen == shader_model::glsl_1_1_0)
					shader_version = "#version 110 core\n";
				else if (shader_gen == shader_model::glsl_1_2_0)
					shader_version = "#version 120 core\n";
				else if (shader_gen == shader_model::glsl_1_3_0)
					shader_version = "#version 130 core\n";
				else if (shader_gen == shader_model::glsl_1_4_0)
					shader_version = "#version 140 core\n";
				else if (shader_gen == shader_model::glsl_1_5_0)
					shader_version = "#version 150 core\n";
				else if (shader_gen == shader_model::glsl_3_3_0)
					shader_version = "#version 330 core\n";
				else if (shader_gen == shader_model::glsl_4_0_0)
					shader_version = "#version 400 core\n";
				else if (shader_gen == shader_model::glsl_4_1_0)
					shader_version = "#version 410 core\n";
				else if (shader_gen == shader_model::glsl_4_2_0)
					shader_version = "#version 420 core\n";
				else if (shader_gen == shader_model::glsl_4_3_0)
					shader_version = "#version 430 core\n";
				else if (shader_gen == shader_model::glsl_4_4_0)
					shader_version = "#version 440 core\n";
				else if (shader_gen == shader_model::glsl_4_5_0)
					shader_version = "#version 450 core\n";
				else if (shader_gen == shader_model::glsl_4_6_0)
					shader_version = "#version 460 core\n";
				else
					set_shader_model(shader_model::glsl_1_1_0);
			}
			void ogl_device::set_blend_state(blend_state* state)
			{
				VI_ASSERT(state != nullptr, "blend state should be set");
				ogl_blend_state* new_state = (ogl_blend_state*)state;
				ogl_blend_state* old_state = regs.blend;
				REG_EXCHANGE(blend, new_state);

				if (!old_state || old_state->state.alpha_to_coverage_enable != new_state->state.alpha_to_coverage_enable)
				{
					if (new_state->state.alpha_to_coverage_enable)
					{
						glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);
						glSampleCoverage(1.0f, GL_FALSE);
					}
					else
					{
						glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);
						glSampleCoverage(0.0f, GL_FALSE);
					}
				}

				if (new_state->state.independent_blend_enable)
				{
					for (uint32_t i = 0; i < 8; i++)
					{
						auto& base = new_state->state.render_target[i];
						if (old_state && memcmp(&old_state->state.render_target[i], &base, sizeof(render_target_blend_state)) == 0)
							continue;

						if (new_state->state.render_target[i].blend_enable)
							glEnablei(GL_BLEND, i);
						else
							glDisablei(GL_BLEND, i);

						glBlendEquationSeparatei(i, get_blend_operation(new_state->state.render_target[i].blend_operation_mode), get_blend_operation(new_state->state.render_target[i].blend_operation_alpha));
						glBlendFuncSeparatei(i, get_blend(new_state->state.render_target[i].src_blend), get_blend(new_state->state.render_target[i].dest_blend), get_blend(new_state->state.render_target[i].src_blend_alpha), get_blend(new_state->state.render_target[i].dest_blend_alpha));
					}
				}
				else
				{
					if (old_state != nullptr)
					{
						if (old_state->state.independent_blend_enable)
						{
							for (uint32_t i = 0; i < 8; i++)
								glDisablei(GL_BLEND, i);
						}

						if (memcmp(&old_state->state.render_target[0], &new_state->state.render_target[0], sizeof(render_target_blend_state)) == 0)
							return;
					}

					if (new_state->state.render_target[0].blend_enable)
						glEnable(GL_BLEND);
					else
						glDisable(GL_BLEND);

					glBlendEquationSeparate(get_blend_operation(new_state->state.render_target[0].blend_operation_mode), get_blend_operation(new_state->state.render_target[0].blend_operation_alpha));
					glBlendFuncSeparate(get_blend(new_state->state.render_target[0].src_blend), get_blend(new_state->state.render_target[0].dest_blend), get_blend(new_state->state.render_target[0].src_blend_alpha), get_blend(new_state->state.render_target[0].dest_blend_alpha));
				}
			}
			void ogl_device::set_rasterizer_state(rasterizer_state* state)
			{
				VI_ASSERT(state != nullptr, "rasterizer state should be set");
				ogl_rasterizer_state* new_state = (ogl_rasterizer_state*)state;
				ogl_rasterizer_state* old_state = regs.rasterizer;
				REG_EXCHANGE(rasterizer, new_state);

				bool was_multisampled = old_state ? (old_state->state.antialiased_line_enable || old_state->state.multisample_enable) : false;
				bool multisampled = (new_state->state.antialiased_line_enable || new_state->state.multisample_enable);
				if (!old_state || was_multisampled != multisampled)
				{
					if (multisampled)
						glEnable(GL_MULTISAMPLE);
					else
						glDisable(GL_MULTISAMPLE);
				}

				if (!old_state || old_state->state.cull_mode != new_state->state.cull_mode)
				{
					if (new_state->state.cull_mode == vertex_cull::back)
					{
						glCullFace(GL_FRONT);
						glEnable(GL_CULL_FACE);
					}
					else if (new_state->state.cull_mode == vertex_cull::front)
					{
						glCullFace(GL_BACK);
						glEnable(GL_CULL_FACE);
					}
					else
						glDisable(GL_CULL_FACE);
				}

				if (!old_state || old_state->state.scissor_enable != new_state->state.scissor_enable)
				{
					if (new_state->state.scissor_enable)
						glEnable(GL_SCISSOR_TEST);
					else
						glDisable(GL_SCISSOR_TEST);
				}

				if (!old_state || old_state->state.fill_mode != new_state->state.fill_mode)
				{
					if (new_state->state.fill_mode == surface_fill::solid)
						glPolygonMode(GL_BACK, GL_FILL);
					else
						glPolygonMode(GL_BACK, GL_LINE);
				}

				if (!old_state || old_state->state.front_counter_clockwise != new_state->state.front_counter_clockwise)
				{
					if (new_state->state.front_counter_clockwise)
						glFrontFace(GL_CW);
					else
						glFrontFace(GL_CCW);
				}
			}
			void ogl_device::set_depth_stencil_state(depth_stencil_state* state)
			{
				VI_ASSERT(state != nullptr, "depth stencil state should be set");
				ogl_depth_stencil_state* new_state = (ogl_depth_stencil_state*)state;
				ogl_depth_stencil_state* old_state = regs.depth_stencil;
				REG_EXCHANGE(depth_stencil, new_state);

				if (!old_state || old_state->state.depth_enable != new_state->state.depth_enable)
				{
					if (new_state->state.depth_enable)
						glEnable(GL_DEPTH_TEST);
					else
						glDisable(GL_DEPTH_TEST);
				}

				if (!old_state || old_state->state.stencil_enable != new_state->state.stencil_enable)
				{
					if (new_state->state.stencil_enable)
						glEnable(GL_STENCIL_TEST);
					else
						glDisable(GL_STENCIL_TEST);
				}

				if (old_state != nullptr)
				{
					if (old_state->state.depth_function != new_state->state.depth_function)
						glDepthFunc(get_comparison(new_state->state.depth_function));

					if (old_state->state.depth_write_mask != new_state->state.depth_write_mask)
						glDepthMask(new_state->state.depth_write_mask == depth_write::all ? GL_TRUE : GL_FALSE);

					if (old_state->state.stencil_write_mask != new_state->state.stencil_write_mask)
						glStencilMask((GLuint)new_state->state.stencil_write_mask);

					if (old_state->state.front_face_stencil_function != new_state->state.front_face_stencil_function)
						glStencilFuncSeparate(GL_FRONT, get_comparison(new_state->state.front_face_stencil_function), 0, 1);

					if (old_state->state.front_face_stencil_fail_operation != new_state->state.front_face_stencil_fail_operation || old_state->state.front_face_stencil_depth_fail_operation != new_state->state.front_face_stencil_depth_fail_operation || old_state->state.front_face_stencil_pass_operation != new_state->state.front_face_stencil_pass_operation)
						glStencilOpSeparate(GL_FRONT, get_stencil_operation(new_state->state.front_face_stencil_fail_operation), get_stencil_operation(new_state->state.front_face_stencil_depth_fail_operation), get_stencil_operation(new_state->state.front_face_stencil_pass_operation));

					if (old_state->state.back_face_stencil_function != new_state->state.back_face_stencil_function)
						glStencilFuncSeparate(GL_BACK, get_comparison(new_state->state.back_face_stencil_function), 0, 1);

					if (old_state->state.back_face_stencil_fail_operation != new_state->state.back_face_stencil_fail_operation || old_state->state.back_face_stencil_depth_fail_operation != new_state->state.back_face_stencil_depth_fail_operation || old_state->state.back_face_stencil_pass_operation != new_state->state.back_face_stencil_pass_operation)
						glStencilOpSeparate(GL_BACK, get_stencil_operation(new_state->state.back_face_stencil_fail_operation), get_stencil_operation(new_state->state.back_face_stencil_depth_fail_operation), get_stencil_operation(new_state->state.back_face_stencil_pass_operation));
				}
				else
				{
					glDepthFunc(get_comparison(new_state->state.depth_function));
					glDepthMask(new_state->state.depth_write_mask == depth_write::all ? GL_TRUE : GL_FALSE);
					glStencilMask((GLuint)new_state->state.stencil_write_mask);
					glStencilFuncSeparate(GL_FRONT, get_comparison(new_state->state.front_face_stencil_function), 0, 1);
					glStencilOpSeparate(GL_FRONT, get_stencil_operation(new_state->state.front_face_stencil_fail_operation), get_stencil_operation(new_state->state.front_face_stencil_depth_fail_operation), get_stencil_operation(new_state->state.front_face_stencil_pass_operation));
					glStencilFuncSeparate(GL_BACK, get_comparison(new_state->state.back_face_stencil_function), 0, 1);
					glStencilOpSeparate(GL_BACK, get_stencil_operation(new_state->state.back_face_stencil_fail_operation), get_stencil_operation(new_state->state.back_face_stencil_depth_fail_operation), get_stencil_operation(new_state->state.back_face_stencil_pass_operation));
				}
			}
			void ogl_device::set_input_layout(input_layout* state)
			{
				REG_EXCHANGE(layout, (ogl_input_layout*)state);
				set_vertex_buffers(nullptr, 0);
			}
			expects_graphics<void> ogl_device::set_shader(shader* resource, uint32_t type)
			{
				ogl_shader* iresource = (ogl_shader*)resource;
				bool update = false;

				if (type & (uint32_t)shader_type::vertex)
				{
					auto& item = regs.shaders[0];
					if (item != iresource)
					{
						if (iresource != nullptr && iresource->vertex_shader != GL_NONE)
							item = iresource;
						else
							item = nullptr;
						update = true;
					}
				}

				if (type & (uint32_t)shader_type::pixel)
				{
					auto& item = regs.shaders[1];
					if (item != iresource)
					{
						if (iresource != nullptr && iresource->pixel_shader != GL_NONE)
							item = iresource;
						else
							item = nullptr;
						update = true;
					}
				}

				if (type & (uint32_t)shader_type::geometry)
				{
					auto& item = regs.shaders[2];
					if (item != iresource)
					{
						if (iresource != nullptr && iresource->geometry_shader != GL_NONE)
							item = iresource;
						else
							item = nullptr;
						update = true;
					}
				}

				if (type & (uint32_t)shader_type::hull)
				{
					auto& item = regs.shaders[3];
					if (item != iresource)
					{
						if (iresource != nullptr && iresource->hull_shader != GL_NONE)
							item = iresource;
						else
							item = nullptr;
						update = true;
					}
				}

				if (type & (uint32_t)shader_type::domain)
				{
					auto& item = regs.shaders[4];
					if (item != iresource)
					{
						if (iresource != nullptr && iresource->domain_shader != GL_NONE)
							item = iresource;
						else
							item = nullptr;
						update = true;
					}
				}

				if (type & (uint32_t)shader_type::compute)
				{
					auto& item = regs.shaders[5];
					if (item != iresource)
					{
						if (iresource != nullptr && iresource->compute_shader != GL_NONE)
							item = iresource;
						else
							item = nullptr;
						update = true;
					}
				}

				if (!update)
					return core::expectation::met;

				uint64_t name = get_program_hash();
				auto it = regs.programs.find(name);
				if (it != regs.programs.end())
				{
					glUseProgramObjectARB(it->second);
					return core::expectation::met;
				}

				GLuint program = glCreateProgram();
				if (regs.shaders[0] != nullptr && regs.shaders[0]->vertex_shader != GL_NONE)
					glAttachShader(program, regs.shaders[0]->vertex_shader);

				if (regs.shaders[1] != nullptr && regs.shaders[1]->pixel_shader != GL_NONE)
					glAttachShader(program, regs.shaders[1]->pixel_shader);

				if (regs.shaders[2] != nullptr && regs.shaders[2]->geometry_shader != GL_NONE)
					glAttachShader(program, regs.shaders[2]->geometry_shader);

				if (regs.shaders[3] != nullptr && regs.shaders[3]->domain_shader != GL_NONE)
					glAttachShader(program, regs.shaders[3]->domain_shader);

				if (regs.shaders[4] != nullptr && regs.shaders[4]->hull_shader != GL_NONE)
					glAttachShader(program, regs.shaders[4]->hull_shader);

				if (regs.shaders[5] != nullptr && regs.shaders[5]->compute_shader != GL_NONE)
					glAttachShader(program, regs.shaders[5]->compute_shader);

				GLint status_code = 0;
				glLinkProgramARB(program);
				glGetProgramiv(program, GL_LINK_STATUS, &status_code);
				glUseProgramObjectARB(program);

				if (status_code != GL_TRUE)
				{
					GLint buffer_size = 0;
					glGetProgramiv(program, GL_INFO_LOG_LENGTH, &buffer_size);

					char* buffer = core::memory::allocate<char>(sizeof(char) * (buffer_size + 1));
					glGetProgramInfoLog(program, buffer_size, &buffer_size, buffer);
					core::string error_text(buffer, (size_t)buffer_size);
					core::memory::deallocate(buffer);

					glUseProgramObjectARB(GL_NONE);
					glDeleteProgram(program);
					program = GL_NONE;
					regs.programs[name] = program;
					return graphics_exception(std::move(error_text));
				}
				else
				{
					for (auto* base : regs.shaders)
					{
						if (base != nullptr)
							base->programs[program] = this;
					}
					regs.programs[name] = program;
					return core::expectation::met;
				}
			}
			void ogl_device::set_sampler_state(sampler_state* state, uint32_t slot, uint32_t count, uint32_t type)
			{
				VI_ASSERT(slot < units_size, "slot should be less than %i", (int)units_size);
				VI_ASSERT(count <= units_size && slot + count <= units_size, "count should be less than or equal %i", (int)units_size);

				ogl_sampler_state* iresource = (ogl_sampler_state*)state;
				GLuint new_state = (GLuint)(iresource ? iresource->resource : GL_NONE);
				uint32_t offset = slot + count;

				for (uint32_t i = slot; i < offset; i++)
				{
					auto& item = regs.samplers[i];
					if (item != new_state)
					{
						glBindSampler(i, new_state);
						item = new_state;
					}
				}
			}
			void ogl_device::set_buffer(shader* resource, uint32_t slot, uint32_t type)
			{
				VI_ASSERT(slot < units_size, "slot should be less than %i", (int)units_size);

				ogl_shader* iresource = (ogl_shader*)resource;
				glBindBufferBase(GL_UNIFORM_BUFFER, slot, iresource ? iresource->constant_buffer : GL_NONE);
			}
			void ogl_device::set_buffer(instance_buffer* resource, uint32_t slot, uint32_t type)
			{
				ogl_instance_buffer* iresource = (ogl_instance_buffer*)resource;
				set_structure_buffer(iresource ? iresource->elements : nullptr, slot, type);
			}
			void ogl_device::set_constant_buffer(element_buffer* resource, uint32_t slot, uint32_t type)
			{
				VI_ASSERT(slot < units_size, "slot should be less than %i", (int)units_size);
				glBindBufferBase(GL_UNIFORM_BUFFER, slot, resource ? ((ogl_element_buffer*)resource)->resource : GL_NONE);
			}
			void ogl_device::set_structure_buffer(element_buffer* resource, uint32_t slot, uint32_t type)
			{
				VI_ASSERT(slot < units_size, "slot should be less than %i", (int)units_size);
				glBindBufferBase(GL_SHADER_STORAGE_BUFFER, slot, resource ? ((ogl_element_buffer*)resource)->resource : GL_NONE);
			}
			void ogl_device::set_index_buffer(element_buffer* resource, format format_mode)
			{
				ogl_element_buffer* iresource = (ogl_element_buffer*)resource;
				REG_EXCHANGE_T2(index_buffer, iresource, format_mode);

				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iresource ? iresource->resource : GL_NONE);
				if (format_mode == format::r32_uint)
					regs.index_format = GL_UNSIGNED_INT;
				else if (format_mode == format::r16_uint)
					regs.index_format = GL_UNSIGNED_SHORT;
				else if (format_mode == format::r8_uint)
					regs.index_format = GL_UNSIGNED_BYTE;
				else
					regs.index_format = GL_UNSIGNED_INT;
			}
			void ogl_device::set_vertex_buffers(element_buffer** resources, uint32_t count, bool dynamic_linkage)
			{
				VI_ASSERT(resources != nullptr || !count, "invalid vertex buffer array pointer");
				VI_ASSERT(count <= units_size, "slot should be less than or equal to %i", (int)units_size);

				static ogl_element_buffer* iresources[units_size] = { nullptr };
				bool has_buffers = false;

				for (uint32_t i = 0; i < count; i++)
				{
					iresources[i] = (ogl_element_buffer*)resources[i];
					regs.vertex_buffers[i] = iresources[i];
					if (iresources[i] != nullptr)
						has_buffers = true;
				}

				set_index_buffer(nullptr, format::unknown);
				if (!count || !has_buffers || !regs.layout)
					return (void)glBindVertexArray(0);

				GLuint buffer = GL_NONE;
				if (!dynamic_linkage)
				{
					core::string hash = ogl_input_layout::get_layout_hash(iresources, count);
					auto it = regs.layout->layouts.find(hash);
					if (it == regs.layout->layouts.end())
					{
						glGenVertexArrays(1, &buffer);
						glBindVertexArray(buffer);
						regs.layout->layouts[hash] = buffer;
						dynamic_linkage = true;
					}
					else
						buffer = it->second;
				}
				else
				{
					if (regs.layout->dynamic_resource == GL_NONE)
					{
						glGenVertexArrays(1, &regs.layout->dynamic_resource);
						dynamic_linkage = true;
					}
					buffer = regs.layout->dynamic_resource;
				}

				glBindVertexArray(buffer);
				if (!dynamic_linkage)
					return;

				VI_ASSERT(count <= regs.layout->vertex_layout.size(), "too many vertex buffers are being bound: %" PRIu64 " out of %" PRIu64, (uint64_t)count, (uint64_t)regs.layout->vertex_layout.size());
				for (uint32_t i = 0; i < count; i++)
				{
					ogl_element_buffer* iresource = iresources[i];
					if (!iresource)
						continue;

					iresource->bindings[buffer] = regs.layout;
					glBindBuffer(GL_ARRAY_BUFFER, iresource->resource);
					for (auto& attribute : regs.layout->vertex_layout[i])
						attribute(iresource->stride);
				}
			}
			void ogl_device::set_texture_2d(texture_2d* resource, uint32_t slot, uint32_t type)
			{
				VI_ASSERT(!resource || !((ogl_texture_2d*)resource)->backbuffer, "resource 2d should not be back buffer texture");
				VI_ASSERT(slot < units_size, "slot should be less than %i", (int)units_size);

				ogl_texture_2d* iresource = (ogl_texture_2d*)resource;
				GLuint new_resource = (iresource ? iresource->resource : GL_NONE);
				if (regs.textures[slot] == new_resource)
					return;

				regs.bindings[slot] = GL_TEXTURE_2D;
				regs.textures[slot] = new_resource;
				glActiveTexture(GL_TEXTURE0 + slot);
				glBindTexture(GL_TEXTURE_2D, new_resource);
			}
			void ogl_device::set_texture_3d(texture_3d* resource, uint32_t slot, uint32_t type)
			{
				VI_ASSERT(slot < units_size, "slot should be less than %i", (int)units_size);

				ogl_texture_3d* iresource = (ogl_texture_3d*)resource;
				GLuint new_resource = (iresource ? iresource->resource : GL_NONE);
				if (regs.textures[slot] == new_resource)
					return;

				regs.bindings[slot] = GL_TEXTURE_3D;
				regs.textures[slot] = new_resource;
				glActiveTexture(GL_TEXTURE0 + slot);
				glBindTexture(GL_TEXTURE_3D, new_resource);
			}
			void ogl_device::set_texture_cube(texture_cube* resource, uint32_t slot, uint32_t type)
			{
				VI_ASSERT(slot < units_size, "slot should be less than %i", (int)units_size);

				ogl_texture_cube* iresource = (ogl_texture_cube*)resource;
				GLuint new_resource = (iresource ? iresource->resource : GL_NONE);
				if (regs.textures[slot] == new_resource)
					return;

				regs.bindings[slot] = GL_TEXTURE_CUBE_MAP;
				regs.textures[slot] = new_resource;
				glActiveTexture(GL_TEXTURE0 + slot);
				glBindTexture(GL_TEXTURE_CUBE_MAP, new_resource);
			}
			void ogl_device::set_writeable(element_buffer** resource, uint32_t slot, uint32_t count, bool computable)
			{
				VI_ASSERT(slot < units_size, "slot should be less than %i", (int)units_size);
				VI_ASSERT(count <= units_size && slot + count <= units_size, "count should be less than or equal %i", (int)units_size);
				VI_ASSERT(resource != nullptr, "resource should be set");
			}
			void ogl_device::set_writeable(texture_2d** resource, uint32_t slot, uint32_t count, bool computable)
			{
				VI_ASSERT(slot < units_size, "slot should be less than %i", (int)units_size);
				VI_ASSERT(count <= units_size && slot + count <= units_size, "count should be less than or equal %i", (int)units_size);
				VI_ASSERT(resource != nullptr, "resource should be set");

				for (uint32_t i = 0; i < count; i++)
				{
					ogl_texture_2d* iresource = (ogl_texture_2d*)resource[i];
					VI_ASSERT(!iresource || !iresource->backbuffer, "resource 2d #%i should not be back buffer texture", (int)i);

					glActiveTexture(GL_TEXTURE0 + slot + i);
					if (!iresource)
						glBindTexture(GL_TEXTURE_2D, GL_NONE);
					else
						glBindImageTexture(slot + i, iresource->resource, 0, GL_TRUE, 0, GL_READ_WRITE, iresource->format);
				}
			}
			void ogl_device::set_writeable(texture_3d** resource, uint32_t slot, uint32_t count, bool computable)
			{
				VI_ASSERT(slot < units_size, "slot should be less than %i", (int)units_size);
				VI_ASSERT(count <= units_size && slot + count <= units_size, "count should be less than or equal %i", (int)units_size);
				VI_ASSERT(resource != nullptr, "resource should be set");

				for (uint32_t i = 0; i < count; i++)
				{
					ogl_texture_3d* iresource = (ogl_texture_3d*)resource[i];
					glActiveTexture(GL_TEXTURE0 + slot + i);

					if (!iresource)
						glBindTexture(GL_TEXTURE_3D, GL_NONE);
					else
						glBindImageTexture(slot + i, iresource->resource, 0, GL_TRUE, 0, GL_READ_WRITE, iresource->format);
				}
			}
			void ogl_device::set_writeable(texture_cube** resource, uint32_t slot, uint32_t count, bool computable)
			{
				VI_ASSERT(slot < units_size, "slot should be less than %i", (int)units_size);
				VI_ASSERT(count <= units_size && slot + count <= units_size, "count should be less than or equal %i", (int)units_size);
				VI_ASSERT(resource != nullptr, "resource should be set");

				for (uint32_t i = 0; i < count; i++)
				{
					ogl_texture_cube* iresource = (ogl_texture_cube*)resource[i];
					glActiveTexture(GL_TEXTURE0 + slot + i);

					if (!iresource)
						glBindTexture(GL_TEXTURE_CUBE_MAP, GL_NONE);
					else
						glBindImageTexture(slot + i, iresource->resource, 0, GL_TRUE, 0, GL_READ_WRITE, iresource->format);
				}
			}
			void ogl_device::set_target(float r, float g, float b)
			{
				set_target(render_target, 0, r, g, b);
			}
			void ogl_device::set_target()
			{
				set_target(render_target, 0);
			}
			void ogl_device::set_target(depth_target_2d* resource)
			{
				VI_ASSERT(resource != nullptr, "resource should be set");
				ogl_depth_target_2d* iresource = (ogl_depth_target_2d*)resource;
				GLenum target = GL_NONE;
				glBindFramebuffer(GL_FRAMEBUFFER, iresource->frame_buffer);
				glDrawBuffers(1, &target);
				glViewport((GLuint)iresource->viewarea.top_left_x, (GLuint)iresource->viewarea.top_left_y, (GLuint)iresource->viewarea.width, (GLuint)iresource->viewarea.height);
			}
			void ogl_device::set_target(depth_target_cube* resource)
			{
				VI_ASSERT(resource != nullptr, "resource should be set");
				ogl_depth_target_cube* iresource = (ogl_depth_target_cube*)resource;
				GLenum target = GL_NONE;
				glBindFramebuffer(GL_FRAMEBUFFER, iresource->frame_buffer);
				glDrawBuffers(1, &target);
				glViewport((GLuint)iresource->viewarea.top_left_x, (GLuint)iresource->viewarea.top_left_y, (GLuint)iresource->viewarea.width, (GLuint)iresource->viewarea.height);
			}
			void ogl_device::set_target(graphics::render_target* resource, uint32_t target, float r, float g, float b)
			{
				VI_ASSERT(resource != nullptr, "resource should be set");
				VI_ASSERT(target < resource->get_target_count(), "targets count should be less than %i", (int)resource->get_target_count());

				ogl_frame_buffer* target_buffer = (ogl_frame_buffer*)resource->get_target_buffer();
				const viewport& viewarea = resource->get_viewport();

				VI_ASSERT(target_buffer != nullptr, "target buffer should be set");
				if (!target_buffer->backbuffer)
				{
					GLenum targets[8] = { GL_NONE };
					targets[target] = target_buffer->format[target];

					glBindFramebuffer(GL_FRAMEBUFFER, target_buffer->buffer);
					glDrawBuffers(resource->get_target_count(), targets);

				}
				else
				{
					glBindFramebuffer(GL_FRAMEBUFFER, GL_NONE);
					glDrawBuffer(GL_BACK);
				}

				glViewport((GLuint)viewarea.top_left_x, (GLuint)viewarea.top_left_y, (GLuint)viewarea.width, (GLuint)viewarea.height);
				glClearColor(r, g, b, 1.0f);
				glClear(GL_COLOR_BUFFER_BIT);
			}
			void ogl_device::set_target(graphics::render_target* resource, uint32_t target)
			{
				VI_ASSERT(resource != nullptr, "resource should be set");
				VI_ASSERT(target < resource->get_target_count(), "targets count should be less than %i", (int)resource->get_target_count());

				ogl_frame_buffer* target_buffer = (ogl_frame_buffer*)resource->get_target_buffer();
				const viewport& viewarea = resource->get_viewport();

				VI_ASSERT(target_buffer != nullptr, "target buffer should be set");
				if (!target_buffer->backbuffer)
				{
					GLenum targets[8] = { GL_NONE };
					targets[target] = target_buffer->format[target];

					glBindFramebuffer(GL_FRAMEBUFFER, target_buffer->buffer);
					glDrawBuffers(resource->get_target_count(), targets);
				}
				else
				{
					glBindFramebuffer(GL_FRAMEBUFFER, GL_NONE);
					glDrawBuffer(GL_BACK);
				}

				glViewport((GLuint)viewarea.top_left_x, (GLuint)viewarea.top_left_y, (GLuint)viewarea.width, (GLuint)viewarea.height);
			}
			void ogl_device::set_target(graphics::render_target* resource, float r, float g, float b)
			{
				VI_ASSERT(resource != nullptr, "resource should be set");
				ogl_frame_buffer* target_buffer = (ogl_frame_buffer*)resource->get_target_buffer();
				const viewport& viewarea = resource->get_viewport();

				VI_ASSERT(target_buffer != nullptr, "target buffer should be set");
				if (!target_buffer->backbuffer)
				{
					glBindFramebuffer(GL_FRAMEBUFFER, target_buffer->buffer);
					glDrawBuffers(resource->get_target_count(), target_buffer->format);
				}
				else
				{
					glBindFramebuffer(GL_FRAMEBUFFER, GL_NONE);
					glDrawBuffer(GL_BACK);
				}

				glViewport((GLuint)viewarea.top_left_x, (GLuint)viewarea.top_left_y, (GLuint)viewarea.width, (GLuint)viewarea.height);
				glClearColor(r, g, b, 1.0f);
				glClear(GL_COLOR_BUFFER_BIT);
			}
			void ogl_device::set_target(graphics::render_target* resource)
			{
				VI_ASSERT(resource != nullptr, "resource should be set");
				ogl_frame_buffer* target_buffer = (ogl_frame_buffer*)resource->get_target_buffer();
				const viewport& viewarea = resource->get_viewport();

				VI_ASSERT(target_buffer != nullptr, "target buffer should be set");
				if (!target_buffer->backbuffer)
				{
					glBindFramebuffer(GL_FRAMEBUFFER, target_buffer->buffer);
					glDrawBuffers(resource->get_target_count(), target_buffer->format);

				}
				else
				{
					glBindFramebuffer(GL_FRAMEBUFFER, GL_NONE);
					glDrawBuffer(GL_BACK);
				}

				glViewport((GLuint)viewarea.top_left_x, (GLuint)viewarea.top_left_y, (GLuint)viewarea.width, (GLuint)viewarea.height);
			}
			void ogl_device::set_target_map(graphics::render_target* resource, bool enabled[8])
			{
				VI_ASSERT(resource != nullptr, "resource should be set");
				VI_ASSERT(resource->get_target_count() > 1, "render target should have more than one targets");

				ogl_frame_buffer* target_buffer = (ogl_frame_buffer*)resource->get_target_buffer();
				const viewport& viewarea = resource->get_viewport();

				VI_ASSERT(target_buffer != nullptr, "target buffer should be set");
				if (!target_buffer->backbuffer)
				{
					GLenum targets[8] = { GL_NONE };
					GLuint count = resource->get_target_count();

					for (uint32_t i = 0; i < count; i++)
						targets[i] = (enabled[i] ? target_buffer->format[i] : GL_NONE);

					glBindFramebuffer(GL_FRAMEBUFFER, target_buffer->buffer);
					glDrawBuffers(count, targets);

				}
				else
				{
					glBindFramebuffer(GL_FRAMEBUFFER, GL_NONE);
					glDrawBuffer(GL_BACK);
				}

				glViewport((GLuint)viewarea.top_left_x, (GLuint)viewarea.top_left_y, (GLuint)viewarea.width, (GLuint)viewarea.height);
			}
			void ogl_device::set_target_rect(uint32_t width, uint32_t height)
			{
				VI_ASSERT(width > 0 && height > 0, "width and height should be greater than zero");
				glBindFramebuffer(GL_FRAMEBUFFER, GL_NONE);
				glViewport(0, 0, width, height);
			}
			void ogl_device::set_viewports(uint32_t count, viewport* value)
			{
				VI_ASSERT(count > 0 && value != nullptr, "at least one viewport should be set");
				glViewport((GLuint)value->top_left_x, (GLuint)value->top_left_y, (GLuint)value->width, (GLuint)value->height);
			}
			void ogl_device::set_scissor_rects(uint32_t count, trigonometry::rectangle* value)
			{
				VI_ASSERT(count > 0 && value != nullptr, "at least one scissor rect should be set");
				int64_t height = value->get_height();
				glScissor((GLuint)value->get_x(), (GLuint)ogl_get_coord_y(value->get_y(), height, (int64_t)window->get_height()), (GLuint)value->get_width(), (GLuint)height);
			}
			void ogl_device::set_primitive_topology(primitive_topology _Topology)
			{
				REG_EXCHANGE(primitive, _Topology);
				regs.draw_topology = get_primitive_topology_draw(_Topology);
				regs.primitive = _Topology;
			}
			void ogl_device::flush_texture(uint32_t slot, uint32_t count, uint32_t type)
			{
				VI_ASSERT(slot < units_size, "slot should be less than %i", (int)units_size);
				VI_ASSERT(count <= units_size && slot + count <= units_size, "count should be less than or equal %i", (int)units_size);

				for (uint32_t i = 0; i < count; i++)
				{
					auto& texture = regs.textures[i];
					if (texture == GL_NONE)
						continue;

					glActiveTexture(GL_TEXTURE0 + slot + i);
					glBindTexture(regs.bindings[i], GL_NONE);
					texture = GL_NONE;
				}
			}
			void ogl_device::flush_state()
			{
			}
			void ogl_device::clear_buffer(instance_buffer* resource)
			{
				VI_ASSERT(resource != nullptr, "resource should be set");
				ogl_instance_buffer* iresource = (ogl_instance_buffer*)resource;
				if (!iresource->sync)
					return;

				ogl_element_buffer* element = (ogl_element_buffer*)iresource->elements;
				glBindBuffer(element->flags, element->resource);
				GLvoid* data = glMapBuffer(element->flags, GL_WRITE_ONLY);
				memset(data, 0, iresource->element_width * iresource->element_limit);
				glUnmapBuffer(element->flags);
				glBindBuffer(element->flags, GL_NONE);
			}
			void ogl_device::clear_writable(texture_2d* resource)
			{
				clear_writable(resource, 0.0f, 0.0f, 0.0f);
			}
			void ogl_device::clear_writable(texture_2d* resource, float r, float g, float b)
			{
				VI_ASSERT(resource != nullptr, "resource should be set");
				VI_ASSERT(!((ogl_texture_2d*)resource)->backbuffer, "resource 2d should not be back buffer texture");

				ogl_texture_2d* iresource = (ogl_texture_2d*)resource;
				GLfloat clear_color[4] = { r, g, b, 0.0f };
				glClearTexImage(iresource->resource, 0, GL_RGBA, GL_FLOAT, &clear_color);
			}
			void ogl_device::clear_writable(texture_3d* resource)
			{
				clear_writable(resource, 0.0f, 0.0f, 0.0f);
			}
			void ogl_device::clear_writable(texture_3d* resource, float r, float g, float b)
			{
				VI_ASSERT(resource != nullptr, "resource should be set");
				ogl_texture_3d* iresource = (ogl_texture_3d*)resource;
				GLfloat clear_color[4] = { r, g, b, 0.0f };
				glClearTexImage(iresource->resource, 0, GL_RGBA, GL_FLOAT, &clear_color);
			}
			void ogl_device::clear_writable(texture_cube* resource)
			{
				clear_writable(resource, 0.0f, 0.0f, 0.0f);
			}
			void ogl_device::clear_writable(texture_cube* resource, float r, float g, float b)
			{
				VI_ASSERT(resource != nullptr, "resource should be set");
				ogl_texture_cube* iresource = (ogl_texture_cube*)resource;
				GLfloat clear_color[4] = { r, g, b, 0.0f };
				glClearTexImage(iresource->resource, 0, GL_RGBA, GL_FLOAT, &clear_color);
			}
			void ogl_device::clear(float r, float g, float b)
			{
				glClearColor(r, g, b, 1.0f);
				glClear(GL_COLOR_BUFFER_BIT);
			}
			void ogl_device::clear(graphics::render_target* resource, uint32_t target, float r, float g, float b)
			{
				VI_ASSERT(resource != nullptr, "resource should be set");
				ogl_frame_buffer* target_buffer = (ogl_frame_buffer*)resource->get_target_buffer();
				if (target_buffer->backbuffer)
					return clear(r, g, b);

				float clear_color[4] = { r, g, b, 0.0f };
				glClearBufferfv(GL_COLOR, target, clear_color);
			}
			void ogl_device::clear_depth()
			{
				clear_depth(render_target);
			}
			void ogl_device::clear_depth(depth_target_2d* resource)
			{
				VI_ASSERT(resource != nullptr, "resource should be set");
				ogl_depth_target_2d* iresource = (ogl_depth_target_2d*)resource;
				if (iresource->has_stencil_buffer)
					glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
				else
					glClear(GL_DEPTH_BUFFER_BIT);
			}
			void ogl_device::clear_depth(depth_target_cube* resource)
			{
				VI_ASSERT(resource != nullptr, "resource should be set");
				ogl_depth_target_cube* iresource = (ogl_depth_target_cube*)resource;
				if (iresource->has_stencil_buffer)
					glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
				else
					glClear(GL_DEPTH_BUFFER_BIT);
			}
			void ogl_device::clear_depth(graphics::render_target* resource)
			{
				VI_ASSERT(resource != nullptr, "resource should be set");
				glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
			}
			void ogl_device::draw_indexed(uint32_t count, uint32_t index_location, uint32_t base_location)
			{
				glDrawElements(regs.draw_topology, count, regs.index_format, OGL_OFFSET((size_t)index_location));
			}
			void ogl_device::draw_indexed(mesh_buffer* resource)
			{
				VI_ASSERT(resource != nullptr, "resource should be set");
				element_buffer* vertex_buffer = resource->get_vertex_buffer();
				element_buffer* index_buffer = resource->get_index_buffer();
				set_vertex_buffers(&vertex_buffer, 1);
				set_index_buffer(index_buffer, format::r32_uint);

				glDrawElements(regs.draw_topology, (GLsizei)index_buffer->get_elements(), GL_UNSIGNED_INT, nullptr);
			}
			void ogl_device::draw_indexed(skin_mesh_buffer* resource)
			{
				VI_ASSERT(resource != nullptr, "resource should be set");
				element_buffer* vertex_buffer = resource->get_vertex_buffer();
				element_buffer* index_buffer = resource->get_index_buffer();
				set_vertex_buffers(&vertex_buffer, 1);
				set_index_buffer(index_buffer, format::r32_uint);

				glDrawElements(regs.draw_topology, (GLsizei)index_buffer->get_elements(), GL_UNSIGNED_INT, nullptr);
			}
			void ogl_device::draw_indexed_instanced(uint32_t index_count_per_instance, uint32_t instance_count, uint32_t index_location, uint32_t vertex_location, uint32_t instance_location)
			{
				glDrawElementsInstanced(regs.draw_topology, index_count_per_instance, GL_UNSIGNED_INT, nullptr, instance_count);
			}
			void ogl_device::draw_indexed_instanced(element_buffer* instances, mesh_buffer* resource, uint32_t instance_count)
			{
				VI_ASSERT(instances != nullptr, "instances should be set");
				VI_ASSERT(resource != nullptr, "resource should be set");

				element_buffer* vertex_buffers[2] = { resource->get_vertex_buffer(), instances };
				element_buffer* index_buffer = resource->get_index_buffer();
				set_vertex_buffers(vertex_buffers, 2, true);
				set_index_buffer(index_buffer, format::r32_uint);

				glDrawElementsInstanced(regs.draw_topology, (GLsizei)index_buffer->get_elements(), GL_UNSIGNED_INT, nullptr, (GLsizei)instance_count);
			}
			void ogl_device::draw_indexed_instanced(element_buffer* instances, skin_mesh_buffer* resource, uint32_t instance_count)
			{
				VI_ASSERT(instances != nullptr, "instances should be set");
				VI_ASSERT(resource != nullptr, "resource should be set");

				element_buffer* vertex_buffers[2] = { resource->get_vertex_buffer(), instances };
				element_buffer* index_buffer = resource->get_index_buffer();
				set_vertex_buffers(vertex_buffers, 2, true);
				set_index_buffer(index_buffer, format::r32_uint);

				glDrawElementsInstanced(regs.draw_topology, (GLsizei)index_buffer->get_elements(), GL_UNSIGNED_INT, nullptr, (GLsizei)instance_count);
			}
			void ogl_device::draw(uint32_t count, uint32_t location)
			{
				glDrawArrays(regs.draw_topology, (GLint)location, (GLint)count);
			}
			void ogl_device::draw_instanced(uint32_t vertex_count_per_instance, uint32_t instance_count, uint32_t vertex_location, uint32_t instance_location)
			{
				glDrawArraysInstanced(regs.draw_topology, (GLint)vertex_location, (GLint)vertex_count_per_instance, (GLint)instance_count);
			}
			void ogl_device::dispatch(uint32_t group_x, uint32_t group_y, uint32_t group_z)
			{
				glDispatchCompute(group_x, group_y, group_z);
			}
			void ogl_device::get_viewports(uint32_t* count, viewport* out)
			{
				GLint viewport[4];
				glGetIntegerv(GL_VIEWPORT, viewport);
				if (count != nullptr)
					*count = 1;

				if (!out)
					return;

				out->top_left_x = (float)viewport[0];
				out->top_left_y = (float)d3d_get_coord_y((int64_t)viewport[1], (int64_t)viewport[3], (int64_t)window->get_height());
				out->width = (float)viewport[2];
				out->height = (float)viewport[3];
			}
			void ogl_device::get_scissor_rects(uint32_t* count, trigonometry::rectangle* out)
			{
				GLint rect[4];
				glGetIntegerv(GL_SCISSOR_BOX, rect);
				if (count != nullptr)
					*count = 1;

				if (!out)
					return;

				int64_t height = (int64_t)rect[1] - rect[3];
				out->left = rect[0];
				out->right = (int64_t)rect[2] + rect[0];
				out->top = d3d_get_coord_y(rect[1], height, (int64_t)window->get_height());
				out->bottom = height;
			}
			void ogl_device::query_begin(query* resource)
			{
				VI_ASSERT(resource != nullptr, "resource should be set");
				ogl_query* iresource = (ogl_query*)resource;
				glBeginQuery(iresource->predicate ? GL_ANY_SAMPLES_PASSED : GL_SAMPLES_PASSED, iresource->async);
			}
			void ogl_device::query_end(query* resource)
			{
				VI_ASSERT(resource != nullptr, "resource should be set");
				ogl_query* iresource = (ogl_query*)resource;
				glEndQuery(iresource->predicate ? GL_ANY_SAMPLES_PASSED : GL_SAMPLES_PASSED);
			}
			void ogl_device::generate_mips(texture_2d* resource)
			{
				VI_ASSERT(resource != nullptr, "resource should be set");
				ogl_texture_2d* iresource = (ogl_texture_2d*)resource;

				VI_ASSERT(!iresource->backbuffer, "resource 2d should not be back buffer texture");
				VI_ASSERT(iresource->resource != GL_NONE, "resource should be valid");
#ifdef glGenerateTextureMipmap
				glGenerateTextureMipmap(iresource->resource);
#elif glGenerateMipmap
				glBindTexture(GL_TEXTURE_2D, iresource->resource);
				glGenerateMipmap(GL_TEXTURE_2D);
				glBindTexture(GL_TEXTURE_2D, GL_NONE);
#endif
			}
			void ogl_device::generate_mips(texture_3d* resource)
			{
				VI_ASSERT(resource != nullptr, "resource should be set");
				ogl_texture_3d* iresource = (ogl_texture_3d*)resource;

				VI_ASSERT(iresource->resource != GL_NONE, "resource should be valid");
#ifdef glGenerateTextureMipmap
				glGenerateTextureMipmap(iresource->resource);
#endif
			}
			void ogl_device::generate_mips(texture_cube* resource)
			{
				VI_ASSERT(resource != nullptr, "resource should be set");
				ogl_texture_cube* iresource = (ogl_texture_cube*)resource;

				VI_ASSERT(iresource->resource != GL_NONE, "resource should be valid");
#ifdef glGenerateTextureMipmap
				glGenerateTextureMipmap(iresource->resource);
#elif glGenerateMipmap
				glBindTexture(GL_TEXTURE_CUBE_MAP, iresource->resource);
				glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
				glBindTexture(GL_TEXTURE_CUBE_MAP, GL_NONE);
#endif
			}
			bool ogl_device::im_begin()
			{
				if (immediate.vertex_buffer == GL_NONE && !create_direct_buffer(0))
					return false;

				primitives = primitive_topology::triangle_list;
				direct.transform = trigonometry::matrix4x4::identity();
				direct.padding = { 0, 0, 0, 1 };
				view_resource = nullptr;

				elements.clear();
				return true;
			}
			void ogl_device::im_transform(const trigonometry::matrix4x4& transform)
			{
				direct.transform = direct.transform * transform;
			}
			void ogl_device::im_topology(primitive_topology topology)
			{
				primitives = topology;
			}
			void ogl_device::im_emit()
			{
				elements.insert(elements.begin(), { 0, 0, 0, 0, 0, 1, 1, 1, 1 });
			}
			void ogl_device::im_texture(texture_2d* in)
			{
				view_resource = in;
				direct.padding.z = (in != nullptr);
			}
			void ogl_device::im_color(float x, float y, float z, float w)
			{
				VI_ASSERT(!elements.empty(), "vertex should already be emitted");
				auto& element = elements.front();
				element.cx = x;
				element.cy = y;
				element.cz = z;
				element.cw = w;
			}
			void ogl_device::im_intensity(float intensity)
			{
				direct.padding.w = intensity;
			}
			void ogl_device::im_texcoord(float x, float y)
			{
				VI_ASSERT(!elements.empty(), "vertex should already be emitted");
				auto& element = elements.front();
				element.tx = x;
				element.ty = 1.0f - y;
			}
			void ogl_device::im_texcoord_offset(float x, float y)
			{
				direct.padding.x = x;
				direct.padding.y = y;
			}
			void ogl_device::im_position(float x, float y, float z)
			{
				VI_ASSERT(!elements.empty(), "vertex should already be emitted");
				auto& element = elements.front();
				element.px = x;
				element.py = -y;
				element.pz = z;
			}
			bool ogl_device::im_end()
			{
				if (immediate.vertex_buffer == GL_NONE || elements.empty())
					return false;

				ogl_texture_2d* iresource = (ogl_texture_2d*)view_resource;
				if (elements.size() > max_elements && !create_direct_buffer(elements.size()))
					return false;

				GLint last_vao = GL_NONE, last_vbo = GL_NONE, last_ubo = GL_NONE, last_sampler = GL_NONE;
				glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &last_vao);
				glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last_vbo);
				glGetIntegerv(GL_UNIFORM_BUFFER_BINDING, &last_ubo);

				GLint last_program = GL_NONE, last_texture = GL_NONE;
				glGetIntegerv(GL_CURRENT_PROGRAM, &last_program);

				glBindBuffer(GL_UNIFORM_BUFFER, immediate.constant_buffer);
				glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(direct), &direct);
				glBindBuffer(GL_ARRAY_BUFFER, immediate.vertex_buffer);
				GLvoid* data = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
				memcpy(data, elements.data(), (size_t)elements.size() * sizeof(vertex));
				glUnmapBuffer(GL_ARRAY_BUFFER);

				const GLuint constant_slot = glGetUniformBlockIndex(immediate.program, "object");
				const GLuint image_slot = glGetUniformLocation(immediate.program, "diffuse");
				glUseProgram(immediate.program);
				glBindBufferBase(GL_UNIFORM_BUFFER, constant_slot, immediate.constant_buffer);
				glActiveTexture(GL_TEXTURE0 + image_slot);
				glGetIntegerv(GL_SAMPLER_BINDING, &last_sampler);
				glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
				glBindTexture(GL_TEXTURE_2D, iresource ? iresource->resource : GL_NONE);
				glBindSampler(image_slot, immediate.sampler);
				glBindVertexArray(immediate.vertex_array);
				glDrawArrays(get_primitive_topology_draw(primitives), 0, (GLsizei)elements.size());
				glBindVertexArray(last_vao);
				glBindBuffer(GL_ARRAY_BUFFER, last_vbo);
				glBindBuffer(GL_UNIFORM_BUFFER, last_ubo);
				glUseProgram(last_program);
				glBindSampler(image_slot, last_sampler);
				glBindTexture(GL_TEXTURE_2D, last_texture);

				return true;
			}
			bool ogl_device::has_explicit_slots() const
			{
				return (uint32_t)shader_gen > (uint32_t)shader_model::glsl_4_2_0;
			}
			expects_graphics<uint32_t> ogl_device::get_shader_slot(shader* resource, const std::string_view& name) const
			{
				VI_ASSERT(core::stringify::is_cstring(name), "name should be set");
				VI_ASSERT(resource != nullptr, "resource should be set");
				ogl_shader* iresource = (ogl_shader*)resource;
				if (iresource->programs.empty())
				{
					GLint prev_program = GL_NONE;
					glGetIntegerv(GL_CURRENT_PROGRAM, &prev_program);

					auto status = ((graphics_device*)this)->set_shader(resource, (uint32_t)shader_type::all);
					glUseProgramObjectARB(prev_program);
					if (!status)
						return status.error();
				}

				for (auto& program : iresource->programs)
				{
					GLint location = glGetUniformLocation(program.first, name.data());
					if (location != -1)
					{
						GLint binding = GL_INVALID_INDEX;
						glGetUniformiv(program.first, location, &binding);
						return binding != GL_INVALID_INDEX ? (uint32_t)binding : (uint32_t)location;
					}

					GLuint index = glGetUniformBlockIndex(program.first, name.data());
					if (index != GL_INVALID_INDEX)
					{
						GLint binding = GL_INVALID_INDEX;
						glGetActiveUniformBlockiv(program.first, index, GL_UNIFORM_BLOCK_BINDING, &binding);
						return binding != GL_INVALID_INDEX ? (uint32_t)binding : (uint32_t)index;
					}

					index = glGetProgramResourceIndex(program.first, GL_SHADER_STORAGE_BLOCK, name.data());
					if (index != GL_INVALID_INDEX)
						return (uint32_t)index;
				}
				return graphics_exception(GL_INVALID_INDEX, core::stringify::text("shader slot for variable %s not found", name.data()));
			}
			expects_graphics<uint32_t> ogl_device::get_shader_sampler_slot(shader* resource, const std::string_view& resource_name, const std::string_view& sampler_name) const
			{
				return get_shader_slot(resource, resource_name);
			}
			expects_graphics<void> ogl_device::submit()
			{
				VI_ASSERT(window != nullptr, "window should be set");
#ifdef VI_APPLE
				GLint last_frame_buffer = 0;
				glGetIntegerv(GL_FRAMEBUFFER_BINDING, &last_frame_buffer);
				glBindFramebuffer(GL_FRAMEBUFFER, 0);
				video::glew::perform_swap(window);
				glBindFramebuffer(GL_FRAMEBUFFER, last_frame_buffer);
#else
				video::glew::perform_swap(window);
#endif
				dispatch_queue();
				return core::expectation::met;
			}
			expects_graphics<void> ogl_device::map(element_buffer* resource, resource_map mode, mapped_subresource* map)
			{
				VI_ASSERT(resource != nullptr, "resource should be set");
				ogl_element_buffer* iresource = (ogl_element_buffer*)resource;
				glBindBuffer(iresource->flags, iresource->resource);

				GLint size;
				glGetBufferParameteriv(iresource->flags, GL_BUFFER_SIZE, &size);
				map->pointer = glMapBuffer(iresource->flags, ogl_device::get_resource_map(mode));
				map->row_pitch = get_row_pitch(1, (uint32_t)size);
				map->depth_pitch = get_depth_pitch(map->row_pitch, 1);
				if (!map->pointer)
					return get_exception("map element buffer");

				return core::expectation::met;
			}
			expects_graphics<void> ogl_device::map(texture_2d* resource, resource_map mode, mapped_subresource* map)
			{
				VI_ASSERT(resource != nullptr, "resource should be set");
				VI_ASSERT(map != nullptr, "map should be set");

				ogl_texture_2d* iresource = (ogl_texture_2d*)resource;
				GLint base_format = ogl_device::get_base_format(iresource->format_mode);
				GLuint width = iresource->get_width(), height = iresource->get_height();
				GLuint size = get_format_size(iresource->format_mode);

				bool read = mode == resource_map::read || mode == resource_map::read_write;
				bool write = mode == resource_map::write || mode == resource_map::write_discard || mode == resource_map::write_no_overwrite;

				if (read && !((uint32_t)iresource->access_flags & (uint32_t)cpu_access::read))
					return graphics_exception("resource cannot be mapped for reading");

				if (write && !((uint32_t)iresource->access_flags & (uint32_t)cpu_access::write))
					return graphics_exception("resource cannot be mapped for writing");

				map->pointer = core::memory::allocate<char>(width * height * size);
				map->row_pitch = get_row_pitch(width);
				map->depth_pitch = get_depth_pitch(map->row_pitch, height);

				if (map->pointer != nullptr && read)
				{
					glBindTexture(GL_TEXTURE_2D, iresource->resource);
					glGetTexImage(GL_TEXTURE_2D, 0, base_format, GL_UNSIGNED_BYTE, map->pointer);
				}

				if (!map->pointer)
					return get_exception("map texture 2d");

				return core::expectation::met;
			}
			expects_graphics<void> ogl_device::map(texture_3d* resource, resource_map mode, mapped_subresource* map)
			{
				VI_ASSERT(resource != nullptr, "resource should be set");
				VI_ASSERT(map != nullptr, "map should be set");

				ogl_texture_3d* iresource = (ogl_texture_3d*)resource;
				GLuint width = iresource->get_width(), height = iresource->get_height(), depth = iresource->get_depth();
				GLuint size = get_format_size(iresource->format_mode);

				bool read = mode == resource_map::read || mode == resource_map::read_write;
				bool write = mode == resource_map::write || mode == resource_map::write_discard || mode == resource_map::write_no_overwrite;

				if (read && !((uint32_t)iresource->access_flags & (uint32_t)cpu_access::read))
					return graphics_exception("resource cannot be mapped for reading");

				if (write && !((uint32_t)iresource->access_flags & (uint32_t)cpu_access::write))
					return graphics_exception("resource cannot be mapped for writing");

				map->pointer = core::memory::allocate<char>(width * height * depth * size);
				map->row_pitch = get_row_pitch(width);
				map->depth_pitch = get_depth_pitch(map->row_pitch, height * depth);

				if (map->pointer != nullptr && read)
				{
					glBindTexture(GL_TEXTURE_3D, iresource->resource);
					glGetTexImage(GL_TEXTURE_3D, 0, iresource->format, GL_UNSIGNED_BYTE, map->pointer);
				}

				if (!map->pointer)
					return get_exception("map texture 2d");

				return core::expectation::met;
			}
			expects_graphics<void> ogl_device::map(texture_cube* resource, resource_map mode, mapped_subresource* map)
			{
				VI_ASSERT(resource != nullptr, "resource should be set");
				VI_ASSERT(map != nullptr, "map should be set");

				ogl_texture_cube* iresource = (ogl_texture_cube*)resource;
				GLint base_format = ogl_device::get_base_format(iresource->format_mode);
				GLuint width = iresource->get_width(), height = iresource->get_height(), depth = 6;
				GLuint size = get_format_size(iresource->format_mode);

				bool read = mode == resource_map::read || mode == resource_map::read_write;
				bool write = mode == resource_map::write || mode == resource_map::write_discard || mode == resource_map::write_no_overwrite;

				if (read && !((uint32_t)iresource->access_flags & (uint32_t)cpu_access::read))
					return graphics_exception("resource cannot be mapped for reading");

				if (write && !((uint32_t)iresource->access_flags & (uint32_t)cpu_access::write))
					return graphics_exception("resource cannot be mapped for writing");

				map->pointer = core::memory::allocate<char>(width * height * depth * size);
				map->row_pitch = get_row_pitch(width);
				map->depth_pitch = get_depth_pitch(map->row_pitch, height * depth);

				if (map->pointer != nullptr && read)
				{
					glBindTexture(GL_TEXTURE_CUBE_MAP, iresource->resource);
					glGetTexImage(GL_TEXTURE_CUBE_MAP, 0, base_format, GL_UNSIGNED_BYTE, map->pointer);
				}

				if (!map->pointer)
					return get_exception("map texture 2d");

				return core::expectation::met;
			}
			expects_graphics<void> ogl_device::unmap(texture_2d* resource, mapped_subresource* map)
			{
				VI_ASSERT(resource != nullptr, "resource should be set");
				VI_ASSERT(map != nullptr, "map should be set");

				ogl_texture_2d* iresource = (ogl_texture_2d*)resource;
				if ((uint32_t)iresource->access_flags & (uint32_t)cpu_access::write)
				{
					GLint base_format = ogl_device::get_base_format(iresource->format_mode);
					GLuint width = iresource->get_width(), height = iresource->get_height();
					glBindTexture(GL_TEXTURE_2D, iresource->resource);
					glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, base_format, GL_UNSIGNED_BYTE, map->pointer);
				}

				core::memory::deallocate(map->pointer);
				return core::expectation::met;
			}
			expects_graphics<void> ogl_device::unmap(texture_3d* resource, mapped_subresource* map)
			{
				VI_ASSERT(resource != nullptr, "resource should be set");
				VI_ASSERT(map != nullptr, "map should be set");

				ogl_texture_3d* iresource = (ogl_texture_3d*)resource;
				if ((uint32_t)iresource->access_flags & (uint32_t)cpu_access::write)
				{
					GLint base_format = ogl_device::get_base_format(iresource->format_mode);
					GLuint width = iresource->get_width(), height = iresource->get_height(), depth = iresource->get_depth();
					glBindTexture(GL_TEXTURE_3D, iresource->resource);
					glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, width, height, depth, base_format, GL_UNSIGNED_BYTE, map->pointer);
				}

				core::memory::deallocate(map->pointer);
				return core::expectation::met;
			}
			expects_graphics<void> ogl_device::unmap(texture_cube* resource, mapped_subresource* map)
			{
				VI_ASSERT(resource != nullptr, "resource should be set");
				VI_ASSERT(map != nullptr, "map should be set");

				ogl_texture_cube* iresource = (ogl_texture_cube*)resource;
				if ((uint32_t)iresource->access_flags & (uint32_t)cpu_access::write)
				{
					GLint base_format = ogl_device::get_base_format(iresource->format_mode);
					GLuint width = iresource->get_width(), height = iresource->get_height(), depth = 6;
					glBindTexture(GL_TEXTURE_3D, iresource->resource);
					glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, width, height, depth, base_format, GL_UNSIGNED_BYTE, map->pointer);
				}

				core::memory::deallocate(map->pointer);
				return core::expectation::met;
			}
			expects_graphics<void> ogl_device::unmap(element_buffer* resource, mapped_subresource* map)
			{
				VI_ASSERT(resource != nullptr, "resource should be set");
				ogl_element_buffer* iresource = (ogl_element_buffer*)resource;
				glUnmapBuffer(iresource->flags);
				glBindBuffer(iresource->flags, GL_NONE);
				return core::expectation::met;
			}
			expects_graphics<void> ogl_device::update_constant_buffer(element_buffer* resource, void* data, size_t size)
			{
				VI_ASSERT(resource != nullptr, "resource should be set");
				ogl_element_buffer* iresource = (ogl_element_buffer*)resource;
				return copy_constant_buffer(iresource->resource, data, size);
			}
			expects_graphics<void> ogl_device::update_buffer(element_buffer* resource, void* data, size_t size)
			{
				VI_ASSERT(resource != nullptr, "resource should be set");
				ogl_element_buffer* iresource = (ogl_element_buffer*)resource;
				glBindBuffer(iresource->flags, iresource->resource);
				glBufferData(iresource->flags, (GLsizeiptr)size, data, GL_DYNAMIC_DRAW);
				glBindBuffer(iresource->flags, GL_NONE);
				return core::expectation::met;
			}
			expects_graphics<void> ogl_device::update_buffer(shader* resource, const void* data)
			{
				VI_ASSERT(resource != nullptr, "resource should be set");
				ogl_shader* iresource = (ogl_shader*)resource;
				return copy_constant_buffer(iresource->constant_buffer, (void*)data, iresource->constant_size);
			}
			expects_graphics<void> ogl_device::update_buffer(mesh_buffer* resource, trigonometry::vertex* data)
			{
				VI_ASSERT(resource != nullptr, "resource should be set");
				VI_ASSERT(data != nullptr, "data should be set");

				ogl_mesh_buffer* iresource = (ogl_mesh_buffer*)resource;
				mapped_subresource mapped_resource;
				auto map_status = map(iresource->vertex_buffer, resource_map::write, &mapped_resource);
				if (!map_status)
					return map_status;

				memcpy(mapped_resource.pointer, data, (size_t)iresource->vertex_buffer->get_elements() * sizeof(trigonometry::vertex));
				return unmap(iresource->vertex_buffer, &mapped_resource);
			}
			expects_graphics<void> ogl_device::update_buffer(skin_mesh_buffer* resource, trigonometry::skin_vertex* data)
			{
				VI_ASSERT(resource != nullptr, "resource should be set");
				VI_ASSERT(data != nullptr, "data should be set");

				ogl_skin_mesh_buffer* iresource = (ogl_skin_mesh_buffer*)resource;
				mapped_subresource mapped_resource;
				auto map_status = map(iresource->vertex_buffer, resource_map::write, &mapped_resource);
				if (!map_status)
					return map_status;

				memcpy(mapped_resource.pointer, data, (size_t)iresource->vertex_buffer->get_elements() * sizeof(trigonometry::skin_vertex));
				return unmap(iresource->vertex_buffer, &mapped_resource);
			}
			expects_graphics<void> ogl_device::update_buffer(instance_buffer* resource)
			{
				VI_ASSERT(resource != nullptr, "resource should be set");
				ogl_instance_buffer* iresource = (ogl_instance_buffer*)resource;
				if (iresource->array.size() <= 0 || iresource->array.size() > iresource->element_limit)
					return graphics_exception("instance buffer mapping error: invalid array size");

				ogl_element_buffer* element = (ogl_element_buffer*)iresource->elements;
				glBindBuffer(element->flags, element->resource);
				GLvoid* data = glMapBuffer(element->flags, GL_WRITE_ONLY);
				memcpy(data, iresource->array.data(), (size_t)iresource->array.size() * iresource->element_width);
				glUnmapBuffer(element->flags);
				glBindBuffer(element->flags, GL_NONE);
				return core::expectation::met;
			}
			expects_graphics<void> ogl_device::update_buffer_size(shader* resource, size_t size)
			{
				VI_ASSERT(resource != nullptr, "resource should be set");
				VI_ASSERT(size > 0, "size should be greater than zero");

				ogl_shader* iresource = (ogl_shader*)resource;
				if (iresource->constant_buffer != GL_NONE)
					glDeleteBuffers(1, &iresource->constant_buffer);

				auto new_buffer = create_constant_buffer(&iresource->constant_buffer, size);
				iresource->constant_size = (new_buffer ? size : 0);
				return new_buffer;
			}
			expects_graphics<void> ogl_device::update_buffer_size(instance_buffer* resource, size_t size)
			{
				VI_ASSERT(resource != nullptr, "resource should be set");
				VI_ASSERT(size > 0, "size should be greater than zero");

				ogl_instance_buffer* iresource = (ogl_instance_buffer*)resource;
				clear_buffer(iresource);
				core::memory::release(iresource->elements);
				iresource->element_limit = size;
				iresource->array.clear();
				iresource->array.reserve(iresource->element_limit);

				element_buffer::desc f = element_buffer::desc();
				f.access_flags = cpu_access::write;
				f.misc_flags = resource_misc::buffer_structured;
				f.usage = resource_usage::dynamic;
				f.bind_flags = resource_bind::shader_input;
				f.element_count = (uint32_t)iresource->element_limit;
				f.element_width = (uint32_t)iresource->element_width;
				f.structure_byte_stride = f.element_width;

				auto buffer = create_element_buffer(f);
				if (!buffer)
					return buffer.error();

				iresource->elements = *buffer;
				return core::expectation::met;
			}
			expects_graphics<void> ogl_device::copy_texture_2d(texture_2d* resource, texture_2d** result)
			{
				VI_ASSERT(resource != nullptr, "resource should be set");
				VI_ASSERT(result != nullptr, "result should be set");
				VI_ASSERT(!*result || !((ogl_texture_2d*)(*result))->backbuffer, "output resource 2d should not be back buffer");

				ogl_texture_2d* iresource = (ogl_texture_2d*)resource;
				if (iresource->backbuffer)
					return copy_back_buffer(result);

				VI_ASSERT(iresource->resource != GL_NONE, "resource should be valid");
				int last_texture = GL_NONE, width, height;
				glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
				glBindTexture(GL_TEXTURE_2D, iresource->resource);
				glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
				glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);

				if (!*result)
				{
					texture_2d::desc f;
					f.width = (uint32_t)width;
					f.height = (uint32_t)height;
					f.row_pitch = get_row_pitch(f.width);
					f.depth_pitch = get_depth_pitch(f.row_pitch, f.height);
					f.mip_levels = get_mip_level(f.width, f.height);

					auto new_texture = create_texture_2d(f);
					if (!new_texture)
						return new_texture.error();

					*result = (ogl_texture_2d*)*new_texture;
				}

				if (GLEW_VERSION_4_3)
					ogl_copy_texture43(GL_TEXTURE_2D, iresource->resource, ((ogl_texture_2d*)(*result))->resource, width, height);
				else
					ogl_copy_texture30(iresource->resource, ((ogl_texture_2d*)(*result))->resource, width, height);

				glBindTexture(GL_TEXTURE_2D, last_texture);
				return core::expectation::met;
			}
			expects_graphics<void> ogl_device::copy_texture_2d(graphics::render_target* resource, uint32_t target, texture_2d** result)
			{
				VI_ASSERT(resource != nullptr, "resource should be set");
				VI_ASSERT(result != nullptr, "result should be set");

				int last_texture = GL_NONE;
				glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
				ogl_frame_buffer* target_buffer = (ogl_frame_buffer*)resource->get_target_buffer();

				VI_ASSERT(target_buffer != nullptr, "target buffer should be set");
				if (target_buffer->backbuffer)
				{
					if (!*result)
					{
						auto new_texture = create_texture_2d();
						if (!new_texture)
							return new_texture.error();

						*result = (ogl_texture_2d*)*new_texture;
						glGenTextures(1, &((ogl_texture_2d*)(*result))->resource);
					}

					GLint viewport[4];
					glGetIntegerv(GL_VIEWPORT, viewport);
					glBindTexture(GL_TEXTURE_2D, ((ogl_texture_2d*)(*result))->resource);
					glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, (GLsizei)viewport[2], (GLsizei)viewport[3]);
					glBindTexture(GL_TEXTURE_2D, last_texture);
					return core::expectation::met;
				}

				ogl_texture_2d* source = (ogl_texture_2d*)resource->get_target_2d(target);
				if (!source)
					return graphics_exception("copy texture 2d: invalid target");

				int width, height;
				glBindTexture(GL_TEXTURE_2D, source->resource);
				glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
				glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);

				if (!*result)
				{
					texture_2d::desc f;
					f.width = (uint32_t)width;
					f.height = (uint32_t)height;
					f.row_pitch = get_row_pitch(f.width);
					f.depth_pitch = get_depth_pitch(f.row_pitch, f.height);
					f.mip_levels = get_mip_level(f.width, f.height);

					auto new_texture = create_texture_2d(f);
					if (!new_texture)
						return new_texture.error();

					*result = (ogl_texture_2d*)*new_texture;
				}

				if (GLEW_VERSION_4_3)
					ogl_copy_texture43(GL_TEXTURE_2D, source->resource, ((ogl_texture_2d*)(*result))->resource, width, height);
				else
					ogl_copy_texture30(source->resource, ((ogl_texture_2d*)(*result))->resource, width, height);

				glBindTexture(GL_TEXTURE_2D, last_texture);
				return core::expectation::met;
			}
			expects_graphics<void> ogl_device::copy_texture_2d(render_target_cube* resource, trigonometry::cube_face face, texture_2d** result)
			{
				VI_ASSERT(resource != nullptr, "resource should be set");
				VI_ASSERT(result != nullptr, "result should be set");

				ogl_render_target_cube* iresource = (ogl_render_target_cube*)resource;
				int last_texture = GL_NONE, width, height;
				glGetIntegerv(GL_TEXTURE_BINDING_CUBE_MAP, &last_texture);
				glBindTexture(GL_TEXTURE_CUBE_MAP, iresource->frame_buffer.texture[0]);
				glGetTexLevelParameteriv(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_TEXTURE_WIDTH, &width);
				glGetTexLevelParameteriv(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_TEXTURE_HEIGHT, &height);

				if (!*result)
				{
					texture_2d::desc f;
					f.width = (uint32_t)width;
					f.height = (uint32_t)height;
					f.row_pitch = get_row_pitch(f.width);
					f.depth_pitch = get_depth_pitch(f.row_pitch, f.height);
					f.mip_levels = get_mip_level(f.width, f.height);

					auto new_texture = create_texture_2d(f);
					if (!new_texture)
						return new_texture.error();

					*result = (ogl_texture_2d*)*new_texture;
				}

				ogl_copy_texture_face_2d30(face, iresource->frame_buffer.texture[0], ((ogl_texture_2d*)(*result))->resource, width, height);
				glBindTexture(GL_TEXTURE_CUBE_MAP, last_texture);
				return core::expectation::met;
			}
			expects_graphics<void> ogl_device::copy_texture_2d(multi_render_target_cube* resource, uint32_t cube, trigonometry::cube_face face, texture_2d** result)
			{
				VI_ASSERT(resource != nullptr, "resource should be set");
				VI_ASSERT(result != nullptr, "result should be set");

				ogl_multi_render_target_cube* iresource = (ogl_multi_render_target_cube*)resource;

				VI_ASSERT(cube < (uint32_t)iresource->target, "cube index should be less than %i", (int)iresource->target);
				int last_texture = GL_NONE, width, height;
				glGetIntegerv(GL_TEXTURE_BINDING_CUBE_MAP, &last_texture);
				glBindTexture(GL_TEXTURE_CUBE_MAP, iresource->frame_buffer.texture[cube]);
				glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
				glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);

				if (!*result)
				{
					texture_2d::desc f;
					f.width = (uint32_t)width;
					f.height = (uint32_t)height;
					f.row_pitch = get_row_pitch(f.width);
					f.depth_pitch = get_depth_pitch(f.row_pitch, f.height);
					f.mip_levels = get_mip_level(f.width, f.height);

					auto new_texture = create_texture_2d(f);
					if (!new_texture)
						return new_texture.error();

					*result = (ogl_texture_2d*)*new_texture;
				}

				ogl_copy_texture_face_2d30(face, iresource->frame_buffer.texture[cube], ((ogl_texture_2d*)(*result))->resource, width, height);
				glBindTexture(GL_TEXTURE_CUBE_MAP, last_texture);
				return core::expectation::met;
			}
			expects_graphics<void> ogl_device::copy_texture_cube(render_target_cube* resource, texture_cube** result)
			{
				VI_ASSERT(resource != nullptr, "resource should be set");
				VI_ASSERT(result != nullptr, "result should be set");

				ogl_render_target_cube* iresource = (ogl_render_target_cube*)resource;
				int last_texture = GL_NONE, width, height;
				glGetIntegerv(GL_TEXTURE_BINDING_CUBE_MAP, &last_texture);
				glBindTexture(GL_TEXTURE_CUBE_MAP, iresource->frame_buffer.texture[0]);
				glGetTexLevelParameteriv(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_TEXTURE_WIDTH, &width);
				glGetTexLevelParameteriv(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_TEXTURE_HEIGHT, &height);

				if (!*result)
				{
					texture_2d* faces[6] = { nullptr };
					for (uint32_t i = 0; i < 6; i++)
						copy_texture_2d(resource, i, &faces[i]);

					auto new_texture = create_texture_cube(faces);
					if (!new_texture)
					{
						for (uint32_t i = 0; i < 6; i++)
							core::memory::release(faces[i]);
						return new_texture.error();
					}
					else
					{
						*result = (ogl_texture_cube*)*new_texture;
						for (uint32_t i = 0; i < 6; i++)
							core::memory::release(faces[i]);
					}
				}
				else if (GLEW_VERSION_4_3)
					ogl_copy_texture43(GL_TEXTURE_CUBE_MAP, iresource->frame_buffer.texture[0], ((ogl_texture_cube*)(*result))->resource, width, height);
				else
					ogl_copy_texture30(iresource->frame_buffer.texture[0], ((ogl_texture_cube*)(*result))->resource, width, height);

				glBindTexture(GL_TEXTURE_CUBE_MAP, last_texture);
				return core::expectation::met;
			}
			expects_graphics<void> ogl_device::copy_texture_cube(multi_render_target_cube* resource, uint32_t cube, texture_cube** result)
			{
				VI_ASSERT(resource != nullptr, "resource should be set");
				VI_ASSERT(result != nullptr, "result should be set");

				ogl_multi_render_target_cube* iresource = (ogl_multi_render_target_cube*)resource;
				VI_ASSERT(cube < (uint32_t)iresource->target, "cube index should be less than %i", (int)iresource->target);
				int last_texture = GL_NONE, width, height;
				glGetIntegerv(GL_TEXTURE_BINDING_CUBE_MAP, &last_texture);
				glBindTexture(GL_TEXTURE_CUBE_MAP, iresource->frame_buffer.texture[cube]);
				glGetTexLevelParameteriv(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_TEXTURE_WIDTH, &width);
				glGetTexLevelParameteriv(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_TEXTURE_HEIGHT, &height);

				if (!*result)
				{
					texture_2d* faces[6] = { nullptr };
					for (uint32_t i = 0; i < 6; i++)
						copy_texture_2d(resource, cube, (trigonometry::cube_face)i, &faces[i]);

					auto new_texture = create_texture_cube(faces);
					if (!new_texture)
					{
						for (uint32_t i = 0; i < 6; i++)
							core::memory::release(faces[i]);
						return new_texture.error();
					}
					else
					{
						*result = (ogl_texture_cube*)*new_texture;
						for (uint32_t i = 0; i < 6; i++)
							core::memory::release(faces[i]);
					}
				}
				else if (GLEW_VERSION_4_3)
					ogl_copy_texture43(GL_TEXTURE_CUBE_MAP, iresource->frame_buffer.texture[cube], ((ogl_texture_cube*)(*result))->resource, width, height);
				else
					ogl_copy_texture30(iresource->frame_buffer.texture[cube], ((ogl_texture_cube*)(*result))->resource, width, height);

				glBindTexture(GL_TEXTURE_CUBE_MAP, last_texture);
				return core::expectation::met;
			}
			expects_graphics<void> ogl_device::copy_target(graphics::render_target* from, uint32_t from_target, graphics::render_target* to, uint32_t to_target)
			{
				VI_ASSERT(from != nullptr && to != nullptr, "from and to should be set");
				ogl_texture_2d* source_2d = (ogl_texture_2d*)from->get_target_2d(from_target);
				ogl_texture_cube* source_cube = (ogl_texture_cube*)from->get_target_cube(from_target);
				ogl_texture_2d* dest_2d = (ogl_texture_2d*)to->get_target_2d(to_target);
				ogl_texture_cube* dest_cube = (ogl_texture_cube*)to->get_target_cube(to_target);
				GLuint source = (source_2d ? source_2d->resource : (source_cube ? source_cube->resource : GL_NONE));
				GLuint dest = (dest_2d ? dest_2d->resource : (dest_cube ? dest_cube->resource : GL_NONE));

				VI_ASSERT(source != GL_NONE, "from should be set");
				VI_ASSERT(dest != GL_NONE, "to should be set");

				int last_texture = GL_NONE;
				uint32_t width = from->get_width();
				uint32_t height = from->get_height();
				glGetIntegerv(source_cube ? GL_TEXTURE_BINDING_CUBE_MAP : GL_TEXTURE_BINDING_2D, &last_texture);

				if (GLEW_VERSION_4_3)
				{
					if (source_cube != nullptr)
						ogl_copy_texture43(GL_TEXTURE_CUBE_MAP, source, dest, width, height);
					else
						ogl_copy_texture43(GL_TEXTURE_2D, source, dest, width, height);
				}
				else
					ogl_copy_texture30(source, dest, width, height);

				glBindTexture(source_cube ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D, last_texture);
				return core::expectation::met;
			}
			expects_graphics<void> ogl_device::cubemap_push(cubemap* resource, texture_cube* result)
			{
				VI_ASSERT(resource != nullptr, "resource should be set");
				VI_ASSERT(resource->is_valid(), "resource should be valid");
				VI_ASSERT(result != nullptr, "result should be set");

				ogl_cubemap* iresource = (ogl_cubemap*)resource;
				ogl_texture_cube* dest = (ogl_texture_cube*)result;
				iresource->dest = dest;

				if (dest->resource != GL_NONE)
					return core::expectation::met;

				GLint size = iresource->meta.size;
				dest->format_mode = iresource->options.format_mode;
				dest->format = iresource->options.size_format;
				dest->mip_levels = iresource->meta.mip_levels;
				dest->width = size;
				dest->height = size;

				glGenTextures(1, &dest->resource);
				glBindTexture(GL_TEXTURE_CUBE_MAP, dest->resource);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

				if (dest->mip_levels > 0)
				{
					glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
					glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
					glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 0);
					glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, dest->mip_levels - 1);
				}
				else
				{
					glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
					glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				}

				glTexStorage2D(GL_TEXTURE_CUBE_MAP, dest->mip_levels, dest->format, size, size);
				glBindTexture(GL_TEXTURE_CUBE_MAP, GL_NONE);
				return core::expectation::met;
			}
			expects_graphics<void> ogl_device::cubemap_face(cubemap* resource, trigonometry::cube_face face)
			{
				VI_ASSERT(resource != nullptr, "resource should be set");
				VI_ASSERT(resource->is_valid(), "resource should be valid");

				ogl_cubemap* iresource = (ogl_cubemap*)resource;
				ogl_texture_cube* dest = (ogl_texture_cube*)iresource->dest;
				VI_ASSERT(iresource->dest != nullptr, "result should be set");

				GLint last_frame_buffer = 0, size = iresource->meta.size;
				glGetIntegerv(GL_FRAMEBUFFER_BINDING, &last_frame_buffer);
				glBindFramebuffer(GL_FRAMEBUFFER, iresource->frame_buffer);
				glFramebufferTexture(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, iresource->source, 0);
				glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_CUBE_MAP_POSITIVE_X + (uint32_t)face, dest->resource, 0);
				glNamedFramebufferDrawBuffer(iresource->frame_buffer, GL_COLOR_ATTACHMENT1);
				glBlitFramebuffer(0, 0, size, size, 0, 0, size, size, GL_COLOR_BUFFER_BIT, GL_NEAREST);
				glBindFramebuffer(GL_FRAMEBUFFER, last_frame_buffer);
				return core::expectation::met;
			}
			expects_graphics<void> ogl_device::cubemap_pop(cubemap* resource)
			{
				VI_ASSERT(resource != nullptr, "resource should be set");
				VI_ASSERT(resource->is_valid(), "resource should be valid");

				ogl_cubemap* iresource = (ogl_cubemap*)resource;
				ogl_texture_cube* dest = (ogl_texture_cube*)iresource->dest;
				VI_ASSERT(iresource->dest != nullptr, "result should be set");
				if (iresource->meta.mip_levels > 0)
					generate_mips(dest);

				return core::expectation::met;
			}
			expects_graphics<void> ogl_device::copy_back_buffer(texture_2d** result)
			{
				VI_ASSERT(result != nullptr, "result should be set");
				VI_ASSERT(!*result || !((ogl_texture_2d*)(*result))->backbuffer, "output resource 2d should not be back buffer");
				ogl_texture_2d* texture = (ogl_texture_2d*)*result;
				if (!texture)
				{
					auto new_texture = create_texture_2d();
					if (!new_texture)
						return new_texture.error();

					glGenTextures(1, &texture->resource);
					*result = texture;
				}

				GLint viewport[4];
				glGetIntegerv(GL_VIEWPORT, viewport);
				glBindTexture(GL_TEXTURE_2D, texture->resource);
				glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 0, 0, (GLsizei)viewport[2], (GLsizei)viewport[3], 0);
				glGenerateMipmap(GL_TEXTURE_2D);
				return generate_texture(texture);
			}
			expects_graphics<void> ogl_device::rescale_buffers(uint32_t width, uint32_t height)
			{
				VI_ASSERT(window != nullptr, "window should be set");
				auto size = window->get_drawable_size(width, height);
				return resize_buffers((uint32_t)size.x, (uint32_t)size.y);
			}
			expects_graphics<void> ogl_device::resize_buffers(uint32_t width, uint32_t height)
			{
				render_target_2d::desc f = render_target_2d::desc();
				f.width = width;
				f.height = height;
				f.mip_levels = 1;
				f.misc_flags = resource_misc::none;
				f.format_mode = format::r8g8b8a8_unorm;
				f.usage = resource_usage::defaults;
				f.access_flags = cpu_access::none;
				f.bind_flags = resource_bind::render_target | resource_bind::shader_input;
				f.render_surface = (void*)this;
				core::memory::release(render_target);

				auto new_target = create_render_target_2d(f);
				if (!new_target)
					return new_target.error();

				render_target = *new_target;
				set_target();
				return core::expectation::met;
			}
			expects_graphics<void> ogl_device::generate_texture(texture_2d* resource)
			{
				VI_ASSERT(resource != nullptr, "resource should be set");
				ogl_texture_2d* iresource = (ogl_texture_2d*)resource;
				if (iresource->backbuffer)
				{
					iresource->width = render_target->get_width();
					iresource->height = render_target->get_height();
					return core::expectation::met;
				}

				VI_ASSERT(iresource->resource != GL_NONE, "resource should be valid");
				int width, height;
				glBindTexture(GL_TEXTURE_2D, iresource->resource);
				glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
				glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);
				glBindTexture(GL_TEXTURE_2D, GL_NONE);

				iresource->width = width;
				iresource->height = height;
				return core::expectation::met;
			}
			expects_graphics<void> ogl_device::generate_texture(texture_3d* resource)
			{
				VI_ASSERT(resource != nullptr, "resource should be set");
				ogl_texture_3d* iresource = (ogl_texture_3d*)resource;

				VI_ASSERT(iresource->resource != GL_NONE, "resource should be valid");
				int width, height, depth;
				glBindTexture(GL_TEXTURE_3D, iresource->resource);
				glGetTexLevelParameteriv(GL_TEXTURE_3D, 0, GL_TEXTURE_WIDTH, &width);
				glGetTexLevelParameteriv(GL_TEXTURE_3D, 0, GL_TEXTURE_HEIGHT, &height);
				glGetTexLevelParameteriv(GL_TEXTURE_3D, 0, GL_TEXTURE_DEPTH, &depth);
				glBindTexture(GL_TEXTURE_3D, GL_NONE);

				iresource->width = width;
				iresource->height = height;
				iresource->depth = depth;
				return core::expectation::met;
			}
			expects_graphics<void> ogl_device::generate_texture(texture_cube* resource)
			{
				VI_ASSERT(resource != nullptr, "resource should be set");
				ogl_texture_cube* iresource = (ogl_texture_cube*)resource;

				VI_ASSERT(iresource->resource != GL_NONE, "resource should be valid");
				int width, height;
				glBindTexture(GL_TEXTURE_CUBE_MAP, iresource->resource);
				glGetTexLevelParameteriv(GL_TEXTURE_CUBE_MAP, 0, GL_TEXTURE_WIDTH, &width);
				glGetTexLevelParameteriv(GL_TEXTURE_CUBE_MAP, 0, GL_TEXTURE_HEIGHT, &height);
				glBindTexture(GL_TEXTURE_CUBE_MAP, GL_NONE);

				iresource->width = width;
				iresource->height = height;
				return core::expectation::met;
			}
			expects_graphics<void> ogl_device::get_query_data(query* resource, size_t* result, bool flush)
			{
				VI_ASSERT(resource != nullptr, "resource should be set");
				VI_ASSERT(result != nullptr, "result should be set");

				ogl_query* iresource = (ogl_query*)resource;
				GLint available = 0;
				glGetQueryObjectiv(iresource->async, GL_QUERY_RESULT_AVAILABLE, &available);
				if (available == GL_FALSE)
					return get_exception("query data");

				GLint64 passing = 0;
				glGetQueryObjecti64v(iresource->async, GL_QUERY_RESULT, &passing);
				*result = (size_t)passing;
				return core::expectation::met;
			}
			expects_graphics<void> ogl_device::get_query_data(query* resource, bool* result, bool flush)
			{
				VI_ASSERT(resource != nullptr, "resource should be set");
				VI_ASSERT(result != nullptr, "result should be set");

				ogl_query* iresource = (ogl_query*)resource;
				GLint available = 0;
				glGetQueryObjectiv(iresource->async, GL_QUERY_RESULT_AVAILABLE, &available);
				if (available == GL_FALSE)
					return get_exception("query data");

				GLint data = 0;
				glGetQueryObjectiv(iresource->async, GL_QUERY_RESULT, &data);
				*result = (data == GL_TRUE);
				return core::expectation::met;
			}
			expects_graphics<depth_stencil_state*> ogl_device::create_depth_stencil_state(const depth_stencil_state::desc& i)
			{
				return new ogl_depth_stencil_state(i);
			}
			expects_graphics<blend_state*> ogl_device::create_blend_state(const blend_state::desc& i)
			{
				return new ogl_blend_state(i);
			}
			expects_graphics<rasterizer_state*> ogl_device::create_rasterizer_state(const rasterizer_state::desc& i)
			{
				return new ogl_rasterizer_state(i);
			}
			expects_graphics<sampler_state*> ogl_device::create_sampler_state(const sampler_state::desc& i)
			{
				GLuint device_state = 0;
				glGenSamplers(1, &device_state);
				glSamplerParameteri(device_state, GL_TEXTURE_WRAP_S, ogl_device::get_texture_address(i.address_u));
				glSamplerParameteri(device_state, GL_TEXTURE_WRAP_T, ogl_device::get_texture_address(i.address_v));
				glSamplerParameteri(device_state, GL_TEXTURE_WRAP_R, ogl_device::get_texture_address(i.address_w));
				glSamplerParameteri(device_state, GL_TEXTURE_MAG_FILTER, ogl_device::get_pixel_filter(i.filter, true));
				glSamplerParameteri(device_state, GL_TEXTURE_MIN_FILTER, ogl_device::get_pixel_filter(i.filter, false));
				glSamplerParameteri(device_state, GL_TEXTURE_COMPARE_MODE, ogl_device::is_comparator(i.filter) ? GL_COMPARE_REF_TO_TEXTURE : GL_NONE);
				glSamplerParameteri(device_state, GL_TEXTURE_COMPARE_FUNC, ogl_device::get_comparison(i.comparison_function));
				glSamplerParameterf(device_state, GL_TEXTURE_LOD_BIAS, i.mip_lod_bias);
				glSamplerParameterf(device_state, GL_TEXTURE_MAX_LOD, i.max_lod);
				glSamplerParameterf(device_state, GL_TEXTURE_MIN_LOD, i.min_lod);
				glSamplerParameterfv(device_state, GL_TEXTURE_BORDER_COLOR, (GLfloat*)i.border_color);
				glSamplerParameterf(device_state, GL_TEXTURE_MAX_ANISOTROPY, (float)i.max_anisotropy);

				ogl_sampler_state* result = new ogl_sampler_state(i);
				result->resource = device_state;
				return result;
			}
			expects_graphics<input_layout*> ogl_device::create_input_layout(const input_layout::desc& i)
			{
				return new ogl_input_layout(i);
			}
			expects_graphics<shader*> ogl_device::create_shader(const shader::desc& i)
			{
				shader::desc f(i);
				auto preprocess_status = preprocess(f);
				if (!preprocess_status)
					return graphics_exception(std::move(preprocess_status.error().message()));

				auto name = get_program_name(f);
				if (!name)
					return graphics_exception("shader name is not defined");

				core::uptr<ogl_shader> result = new ogl_shader(i);
				core::string program_name = std::move(*name);
				const char* data = nullptr;
				GLint size = 0, state = GL_TRUE;

				core::string vertex_entry = get_shader_main(shader_type::vertex);
				if (f.data.find(vertex_entry) != core::string::npos)
				{
					core::string stage = program_name + SHADER_VERTEX, bytecode;
					if (!get_program_cache(stage, &bytecode))
					{
						VI_DEBUG("[opengl] transpile %s vertex shader source", stage.c_str());
						bytecode = f.data;

						auto transpile_status = transpile(&bytecode, shader_type::vertex, shader_lang::glsl);
						if (!transpile_status)
							return transpile_status.error();

						data = bytecode.c_str();
						size = (GLint)bytecode.size();
						set_program_cache(stage, bytecode);
					}
					else
					{
						data = bytecode.c_str();
						size = (GLint)bytecode.size();
					}

					VI_DEBUG("[opengl] compile %s vertex shader bytecode", stage.c_str());
					result->vertex_shader = glCreateShader(GL_VERTEX_SHADER);
					glShaderSourceARB(result->vertex_shader, 1, &data, &size);
					glCompileShaderARB(result->vertex_shader);
					glGetShaderiv(result->vertex_shader, GL_COMPILE_STATUS, &state);
					if (state == GL_FALSE)
					{
						glGetShaderiv(result->vertex_shader, GL_INFO_LOG_LENGTH, &size);
						char* buffer = core::memory::allocate<char>(sizeof(char) * (size + 1));
						glGetShaderInfoLog(result->vertex_shader, size, &size, buffer);
						core::string error_text(buffer, (size_t)size);
						core::memory::deallocate(buffer);
						return graphics_exception(std::move(error_text));
					}
				}

				core::string pixel_entry = get_shader_main(shader_type::pixel);
				if (f.data.find(pixel_entry) != core::string::npos)
				{
					core::string stage = program_name + SHADER_PIXEL, bytecode;
					if (!get_program_cache(stage, &bytecode))
					{
						VI_DEBUG("[opengl] transpile %s pixel shader source", stage.c_str());
						bytecode = f.data;

						auto transpile_status = transpile(&bytecode, shader_type::pixel, shader_lang::glsl);
						if (!transpile_status)
							return transpile_status.error();

						data = bytecode.c_str();
						size = (GLint)bytecode.size();
						set_program_cache(stage, bytecode);
					}
					else
					{
						data = bytecode.c_str();
						size = (GLint)bytecode.size();
					}

					VI_DEBUG("[opengl] compile %s pixel shader bytecode", stage.c_str());
					result->pixel_shader = glCreateShader(GL_FRAGMENT_SHADER);
					glShaderSourceARB(result->pixel_shader, 1, &data, &size);
					glCompileShaderARB(result->pixel_shader);
					glGetShaderiv(result->pixel_shader, GL_COMPILE_STATUS, &state);
					if (state == GL_FALSE)
					{
						glGetShaderiv(result->pixel_shader, GL_INFO_LOG_LENGTH, &size);
						char* buffer = core::memory::allocate<char>(sizeof(char) * (size + 1));
						glGetShaderInfoLog(result->pixel_shader, size, &size, buffer);
						core::string error_text(buffer, (size_t)size);
						core::memory::deallocate(buffer);
						return graphics_exception(std::move(error_text));
					}
				}

				core::string geometry_entry = get_shader_main(shader_type::geometry);
				if (f.data.find(geometry_entry) != core::string::npos)
				{
					core::string stage = program_name + SHADER_GEOMETRY, bytecode;
					if (!get_program_cache(stage, &bytecode))
					{
						VI_DEBUG("[opengl] transpile %s geometry shader source", stage.c_str());
						bytecode = f.data;

						auto transpile_status = transpile(&bytecode, shader_type::geometry, shader_lang::glsl);
						if (!transpile_status)
							return transpile_status.error();

						data = bytecode.c_str();
						size = (GLint)bytecode.size();
						set_program_cache(stage, bytecode);
					}
					else
					{
						data = bytecode.c_str();
						size = (GLint)bytecode.size();
					}

					VI_DEBUG("[opengl] compile %s geometry shader bytecode", stage.c_str());
					result->geometry_shader = glCreateShader(GL_GEOMETRY_SHADER);
					glShaderSourceARB(result->geometry_shader, 1, &data, &size);
					glCompileShaderARB(result->geometry_shader);
					glGetShaderiv(result->geometry_shader, GL_COMPILE_STATUS, &state);
					if (state == GL_FALSE)
					{
						glGetShaderiv(result->geometry_shader, GL_INFO_LOG_LENGTH, &size);
						char* buffer = core::memory::allocate<char>(sizeof(char) * (size + 1));
						glGetShaderInfoLog(result->geometry_shader, size, &size, buffer);
						core::string error_text(buffer, (size_t)size);
						core::memory::deallocate(buffer);
						return graphics_exception(std::move(error_text));
					}
				}

				core::string compute_entry = get_shader_main(shader_type::compute);
				if (f.data.find(compute_entry) != core::string::npos)
				{
					core::string stage = program_name + SHADER_COMPUTE, bytecode;
					if (!get_program_cache(stage, &bytecode))
					{
						VI_DEBUG("[opengl] transpile %s compute shader source", stage.c_str());
						bytecode = f.data;

						auto transpile_status = transpile(&bytecode, shader_type::compute, shader_lang::glsl);
						if (!transpile_status)
							return transpile_status.error();

						data = bytecode.c_str();
						size = (GLint)bytecode.size();
						set_program_cache(stage, bytecode);
					}
					else
					{
						data = bytecode.c_str();
						size = (GLint)bytecode.size();
					}

					VI_DEBUG("[opengl] compile %s compute shader bytecode", stage.c_str());
					result->compute_shader = glCreateShader(GL_COMPUTE_SHADER);
					glShaderSourceARB(result->compute_shader, 1, &data, &size);
					glCompileShaderARB(result->compute_shader);
					glGetShaderiv(result->compute_shader, GL_COMPILE_STATUS, &state);
					if (state == GL_FALSE)
					{
						glGetShaderiv(result->compute_shader, GL_INFO_LOG_LENGTH, &size);
						char* buffer = core::memory::allocate<char>(sizeof(char) * (size + 1));
						glGetShaderInfoLog(result->compute_shader, size, &size, buffer);
						core::string error_text(buffer, (size_t)size);
						core::memory::deallocate(buffer);
						return graphics_exception(std::move(error_text));
					}
				}

				core::string hull_entry = get_shader_main(shader_type::hull);
				if (f.data.find(hull_entry) != core::string::npos)
				{
					core::string stage = program_name + SHADER_HULL, bytecode;
					if (!get_program_cache(stage, &bytecode))
					{
						VI_DEBUG("[opengl] transpile %s hull shader source", stage.c_str());
						bytecode = f.data;

						auto transpile_status = transpile(&bytecode, shader_type::hull, shader_lang::glsl);
						if (!transpile_status)
							return transpile_status.error();

						data = bytecode.c_str();
						size = (GLint)bytecode.size();
						set_program_cache(stage, bytecode);
					}
					else
					{
						data = bytecode.c_str();
						size = (GLint)bytecode.size();
					}

					VI_DEBUG("[opengl] compile %s hull shader bytecode", stage.c_str());
					result->hull_shader = glCreateShader(GL_TESS_CONTROL_SHADER);
					glShaderSourceARB(result->hull_shader, 1, &data, &size);
					glCompileShaderARB(result->hull_shader);
					glGetShaderiv(result->hull_shader, GL_COMPILE_STATUS, &state);
					if (state == GL_FALSE)
					{
						glGetShaderiv(result->hull_shader, GL_INFO_LOG_LENGTH, &size);
						char* buffer = core::memory::allocate<char>(sizeof(char) * (size + 1));
						glGetShaderInfoLog(result->hull_shader, size, &size, buffer);
						core::string error_text(buffer, (size_t)size);
						core::memory::deallocate(buffer);
						return graphics_exception(std::move(error_text));
					}
				}

				core::string domain_entry = get_shader_main(shader_type::domain);
				if (f.data.find(domain_entry) != core::string::npos)
				{
					core::string stage = program_name + SHADER_DOMAIN, bytecode;
					if (!get_program_cache(stage, &bytecode))
					{
						VI_DEBUG("[opengl] transpile %s domain shader source", stage.c_str());
						bytecode = f.data;

						auto transpile_status = transpile(&bytecode, shader_type::domain, shader_lang::glsl);
						if (!transpile_status)
							return transpile_status.error();

						data = bytecode.c_str();
						size = (GLint)bytecode.size();
						set_program_cache(stage, bytecode);
					}
					else
					{
						data = bytecode.c_str();
						size = (GLint)bytecode.size();
					}

					VI_DEBUG("[opengl] compile %s domain shader bytecode", stage.c_str());
					result->domain_shader = glCreateShader(GL_TESS_EVALUATION_SHADER);
					glShaderSourceARB(result->domain_shader, 1, &data, &size);
					glCompileShaderARB(result->domain_shader);
					glGetShaderiv(result->domain_shader, GL_COMPILE_STATUS, &state);
					if (state == GL_FALSE)
					{
						glGetShaderiv(result->domain_shader, GL_INFO_LOG_LENGTH, &size);
						char* buffer = core::memory::allocate<char>(sizeof(char) * (size + 1));
						glGetShaderInfoLog(result->domain_shader, size, &size, buffer);
						core::string error_text(buffer, (size_t)size);
						core::memory::deallocate(buffer);
						return graphics_exception(std::move(error_text));
					}
				}

				result->compiled = true;
				return result.reset();
			}
			expects_graphics<element_buffer*> ogl_device::create_element_buffer(const element_buffer::desc& i)
			{
				ogl_element_buffer* result = new ogl_element_buffer(i);
				result->flags = ogl_device::get_resource_bind(i.bind_flags);
				if ((size_t)i.misc_flags & (size_t)resource_misc::buffer_structured)
					result->flags = GL_SHADER_STORAGE_BUFFER;

				glGenBuffers(1, &result->resource);
				glBindBuffer(result->flags, result->resource);
				glBufferData(result->flags, i.element_count * i.element_width, i.elements, get_access_control(i.access_flags, i.usage));
				glBindBuffer(result->flags, GL_NONE);
				return result;
			}
			expects_graphics<mesh_buffer*> ogl_device::create_mesh_buffer(const mesh_buffer::desc& i)
			{
				element_buffer::desc f = element_buffer::desc();
				f.access_flags = i.access_flags;
				f.usage = i.usage;
				f.bind_flags = resource_bind::vertex_buffer;
				f.element_count = (uint32_t)i.elements.size();
				f.elements = (void*)i.elements.data();
				f.element_width = sizeof(trigonometry::vertex);

				auto new_vertex_buffer = create_element_buffer(f);
				if (!new_vertex_buffer)
					return new_vertex_buffer.error();

				f = element_buffer::desc();
				f.access_flags = i.access_flags;
				f.usage = i.usage;
				f.bind_flags = resource_bind::index_buffer;
				f.element_count = (uint32_t)i.indices.size();
				f.element_width = sizeof(int);
				f.elements = (void*)i.indices.data();

				auto new_index_buffer = create_element_buffer(f);
				if (!new_index_buffer)
				{
					core::memory::release(*new_vertex_buffer);
					return new_index_buffer.error();
				}

				ogl_mesh_buffer* result = new ogl_mesh_buffer(i);
				result->vertex_buffer = *new_vertex_buffer;
				result->index_buffer = *new_index_buffer;
				return result;
			}
			expects_graphics<mesh_buffer*> ogl_device::create_mesh_buffer(element_buffer* vertex_buffer, element_buffer* index_buffer)
			{
				VI_ASSERT(vertex_buffer != nullptr, "vertex buffer should be set");
				VI_ASSERT(index_buffer != nullptr, "index buffer should be set");
				ogl_mesh_buffer* result = new ogl_mesh_buffer(ogl_mesh_buffer::desc());
				result->vertex_buffer = vertex_buffer;
				result->index_buffer = index_buffer;
				return result;
			}
			expects_graphics<skin_mesh_buffer*> ogl_device::create_skin_mesh_buffer(const skin_mesh_buffer::desc& i)
			{
				element_buffer::desc f = element_buffer::desc();
				f.access_flags = i.access_flags;
				f.usage = i.usage;
				f.bind_flags = resource_bind::vertex_buffer;
				f.element_count = (uint32_t)i.elements.size();
				f.elements = (void*)i.elements.data();
				f.element_width = sizeof(trigonometry::skin_vertex);

				auto new_vertex_buffer = create_element_buffer(f);
				if (!new_vertex_buffer)
					return new_vertex_buffer.error();

				f = element_buffer::desc();
				f.access_flags = i.access_flags;
				f.usage = i.usage;
				f.bind_flags = resource_bind::index_buffer;
				f.element_count = (uint32_t)i.indices.size();
				f.element_width = sizeof(int);
				f.elements = (void*)i.indices.data();

				auto new_index_buffer = create_element_buffer(f);
				if (!new_index_buffer)
				{
					core::memory::release(*new_vertex_buffer);
					return new_index_buffer.error();
				}

				ogl_skin_mesh_buffer* result = new ogl_skin_mesh_buffer(i);
				result->vertex_buffer = *new_vertex_buffer;
				result->index_buffer = *new_index_buffer;
				return result;
			}
			expects_graphics<skin_mesh_buffer*> ogl_device::create_skin_mesh_buffer(element_buffer* vertex_buffer, element_buffer* index_buffer)
			{
				VI_ASSERT(vertex_buffer != nullptr, "vertex buffer should be set");
				VI_ASSERT(index_buffer != nullptr, "index buffer should be set");
				ogl_skin_mesh_buffer* result = new ogl_skin_mesh_buffer(ogl_skin_mesh_buffer::desc());
				result->vertex_buffer = vertex_buffer;
				result->index_buffer = index_buffer;
				return result;
			}
			expects_graphics<instance_buffer*> ogl_device::create_instance_buffer(const instance_buffer::desc& i)
			{
				element_buffer::desc f = element_buffer::desc();
				f.access_flags = cpu_access::write;
				f.misc_flags = resource_misc::buffer_structured;
				f.usage = resource_usage::dynamic;
				f.bind_flags = resource_bind::shader_input;
				f.element_count = i.element_limit;
				f.element_width = i.element_width;
				f.structure_byte_stride = f.element_width;

				auto new_buffer = create_element_buffer(f);
				if (!new_buffer)
					return new_buffer.error();

				ogl_instance_buffer* result = new ogl_instance_buffer(i);
				result->elements = *new_buffer;
				return result;
			}
			expects_graphics<texture_2d*> ogl_device::create_texture_2d()
			{
				return new ogl_texture_2d();
			}
			expects_graphics<texture_2d*> ogl_device::create_texture_2d(const texture_2d::desc& i)
			{
				ogl_texture_2d* result = new ogl_texture_2d(i);
				glGenTextures(1, &result->resource);
				glBindTexture(GL_TEXTURE_2D, result->resource);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_REPEAT);

				if (i.mip_levels > 0)
				{
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, i.mip_levels - 1);
				}
				else
				{
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				}

				GLint size_format = ogl_device::get_sized_format(result->format_mode);
				GLint base_format = ogl_device::get_base_format(result->format_mode);
				result->format = size_format;

				glTexImage2D(GL_TEXTURE_2D, 0, size_format, result->width, result->height, 0, base_format, GL_UNSIGNED_BYTE, i.data);
				if (result->mip_levels != 0)
					glGenerateMipmap(GL_TEXTURE_2D);

				glBindTexture(GL_TEXTURE_2D, GL_NONE);
				return result;
			}
			expects_graphics<texture_3d*> ogl_device::create_texture_3d()
			{
				return new ogl_texture_3d();
			}
			expects_graphics<texture_3d*> ogl_device::create_texture_3d(const texture_3d::desc& i)
			{
				ogl_texture_3d* result = new ogl_texture_3d();
				glGenTextures(1, &result->resource);
				glBindTexture(GL_TEXTURE_3D, result->resource);
				glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);

				if (i.mip_levels > 0)
				{
					glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
					glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
					glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_BASE_LEVEL, 0);
					glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAX_LEVEL, i.mip_levels - 1);
				}
				else
				{
					glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
					glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				}

				GLint size_format = ogl_device::get_sized_format(i.format_mode);
				GLint base_format = ogl_device::get_base_format(i.format_mode);
				result->format_mode = i.format_mode;
				result->format = size_format;
				result->mip_levels = i.mip_levels;
				result->width = i.width;
				result->height = i.height;
				result->depth = i.depth;

				glTexImage3D(GL_TEXTURE_3D, 0, size_format, result->width, result->height, result->depth, 0, base_format, GL_UNSIGNED_BYTE, nullptr);
				if (result->mip_levels != 0)
					glGenerateMipmap(GL_TEXTURE_3D);
				glBindTexture(GL_TEXTURE_3D, GL_NONE);

				return result;
			}
			expects_graphics<texture_cube*> ogl_device::create_texture_cube()
			{
				return new ogl_texture_cube();
			}
			expects_graphics<texture_cube*> ogl_device::create_texture_cube(const texture_cube::desc& i)
			{
				ogl_texture_cube* result = new ogl_texture_cube(i);
				glGenTextures(1, &result->resource);
				glBindTexture(GL_TEXTURE_CUBE_MAP, result->resource);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

				if (i.mip_levels > 0)
				{
					glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
					glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
					glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 0);
					glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, i.mip_levels - 1);
				}
				else
				{
					glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
					glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				}

				GLint size_format = ogl_device::get_sized_format(i.format_mode);
				GLint base_format = ogl_device::get_base_format(i.format_mode);
				result->format = size_format;

				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, size_format, result->width, result->height, 0, base_format, GL_UNSIGNED_BYTE, nullptr);
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, size_format, result->width, result->height, 0, base_format, GL_UNSIGNED_BYTE, nullptr);
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, size_format, result->width, result->height, 0, base_format, GL_UNSIGNED_BYTE, nullptr);
				glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, size_format, result->width, result->height, 0, base_format, GL_UNSIGNED_BYTE, nullptr);
				glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, size_format, result->width, result->height, 0, base_format, GL_UNSIGNED_BYTE, nullptr);
				glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, size_format, result->width, result->height, 0, base_format, GL_UNSIGNED_BYTE, nullptr);

				if (result->mip_levels != 0)
					glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

				glBindTexture(GL_TEXTURE_CUBE_MAP, GL_NONE);
				return result;
			}
			expects_graphics<texture_cube*> ogl_device::create_texture_cube(texture_2d* resource[6])
			{
				void* resources[6];
				for (uint32_t i = 0; i < 6; i++)
				{
					VI_ASSERT(resource[i] != nullptr, "face #%i should be set", (int)i);
					VI_ASSERT(!((ogl_texture_2d*)resource[i])->backbuffer, "resource 2d should not be back buffer texture");
					resources[i] = (void*)(ogl_texture_2d*)resource[i];
				}

				return create_texture_cube_internal(resources);
			}
			expects_graphics<texture_cube*> ogl_device::create_texture_cube(texture_2d* resource)
			{
				VI_ASSERT(resource != nullptr, "resource should be set");
				VI_ASSERT(!((ogl_texture_2d*)resource)->backbuffer, "resource 2d should not be back buffer texture");

				ogl_texture_2d* iresource = (ogl_texture_2d*)resource;
				uint32_t width = iresource->width / 4;
				uint32_t height = width;
				uint32_t mip_levels = get_mip_level(width, height);

				if (iresource->width % 4 != 0 || iresource->height % 3 != 0)
					return graphics_exception("create texture cube: width / height is invalid");

				if (iresource->mip_levels > mip_levels)
					iresource->mip_levels = mip_levels;

				ogl_texture_cube* result = new ogl_texture_cube();
				glGenTextures(1, &result->resource);
				glBindTexture(GL_TEXTURE_CUBE_MAP, result->resource);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

				if (iresource->mip_levels > 0)
				{
					glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
					glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
					glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 0);
					glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, iresource->mip_levels - 1);
				}
				else
				{
					glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
					glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				}

				GLint size_format = ogl_device::get_sized_format(result->format_mode);
				GLint base_format = ogl_device::get_base_format(result->format_mode);
				GLsizei size = sizeof(GLubyte) * width * height;
				GLubyte* pixels = core::memory::allocate<GLubyte>(size);
				result->format = size_format;
				result->format_mode = iresource->format_mode;
				result->width = iresource->width;
				result->height = iresource->height;
				result->mip_levels = iresource->mip_levels;

				glBindTexture(GL_TEXTURE_2D, iresource->resource);
				glBindTexture(GL_TEXTURE_CUBE_MAP, result->resource);

				glGetTextureSubImage(GL_TEXTURE_2D, 0, width * 2, height, 0, width, height, 0, base_format, GL_UNSIGNED_BYTE, size, pixels);
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, iresource->mip_levels, size_format, iresource->width, iresource->height, 0, base_format, GL_UNSIGNED_BYTE, pixels);

				glGetTextureSubImage(GL_TEXTURE_2D, 0, width, height * 2, 0, width, height, 0, base_format, GL_UNSIGNED_BYTE, size, pixels);
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, iresource->mip_levels, size_format, iresource->width, iresource->height, 0, base_format, GL_UNSIGNED_BYTE, pixels);

				glGetTextureSubImage(GL_TEXTURE_2D, 0, width * 4, height, 0, width, height, 0, base_format, GL_UNSIGNED_BYTE, size, pixels);
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, iresource->mip_levels, size_format, iresource->width, iresource->height, 0, base_format, GL_UNSIGNED_BYTE, pixels);

				glGetTextureSubImage(GL_TEXTURE_2D, 0, 0, height, 0, width, height, 0, base_format, GL_UNSIGNED_BYTE, size, pixels);
				glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, iresource->mip_levels, size_format, iresource->width, iresource->height, 0, base_format, GL_UNSIGNED_BYTE, pixels);

				glGetTextureSubImage(GL_TEXTURE_2D, 0, width, 0, 0, width, height, 0, base_format, GL_UNSIGNED_BYTE, size, pixels);
				glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, iresource->mip_levels, size_format, iresource->width, iresource->height, 0, base_format, GL_UNSIGNED_BYTE, pixels);

				glGetTextureSubImage(GL_TEXTURE_2D, 0, width, height, 0, width, height, 0, base_format, GL_UNSIGNED_BYTE, size, pixels);
				glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, iresource->mip_levels, size_format, iresource->width, iresource->height, 0, base_format, GL_UNSIGNED_BYTE, pixels);

				core::memory::deallocate(pixels);
				if (iresource->mip_levels != 0)
					glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

				glBindTexture(GL_TEXTURE_2D, GL_NONE);
				glBindTexture(GL_TEXTURE_CUBE_MAP, GL_NONE);
				return result;
			}
			expects_graphics<texture_cube*> ogl_device::create_texture_cube_internal(void* basis[6])
			{
				VI_ASSERT(basis[0] && basis[1] && basis[2] && basis[3] && basis[4] && basis[5], "all 6 faces should be set");
				ogl_texture_2d* resources[6];
				for (uint32_t i = 0; i < 6; i++)
					resources[i] = (ogl_texture_2d*)basis[i];

				ogl_texture_cube* result = new ogl_texture_cube();
				glGenTextures(1, &result->resource);
				glBindTexture(GL_TEXTURE_CUBE_MAP, result->resource);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

				if (resources[0]->mip_levels > 0)
				{
					glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
					glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
					glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 0);
					glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, resources[0]->mip_levels - 1);
				}
				else
				{
					glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
					glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				}

				GLint size_format = ogl_device::get_sized_format(result->format_mode);
				GLint base_format = ogl_device::get_base_format(result->format_mode);
				GLubyte* pixels = core::memory::allocate<GLubyte>(sizeof(GLubyte) * resources[0]->width * resources[0]->height);
				result->format = size_format;
				result->format_mode = resources[0]->format_mode;
				result->width = resources[0]->width;
				result->height = resources[0]->height;
				result->mip_levels = resources[0]->mip_levels;

				for (uint32_t i = 0; i < 6; i++)
				{
					ogl_texture_2d* ref = resources[i];
					glBindTexture(GL_TEXTURE_2D, ref->resource);
					glGetTexImage(GL_TEXTURE_2D, 0, base_format, GL_UNSIGNED_BYTE, pixels);
					glBindTexture(GL_TEXTURE_CUBE_MAP, result->resource);
					glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, ref->mip_levels, size_format, ref->width, ref->height, 0, base_format, GL_UNSIGNED_BYTE, pixels);
				}

				core::memory::deallocate(pixels);
				if (resources[0]->mip_levels != 0)
					glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

				glBindTexture(GL_TEXTURE_2D, GL_NONE);
				glBindTexture(GL_TEXTURE_CUBE_MAP, GL_NONE);
				return result;
			}
			expects_graphics<depth_target_2d*> ogl_device::create_depth_target_2d(const depth_target_2d::desc& i)
			{
				core::uptr<ogl_depth_target_2d> result = new ogl_depth_target_2d(i);
				glGenTextures(1, &result->depth_texture);
				glBindTexture(GL_TEXTURE_2D, result->depth_texture);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_GREATER);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
				glTexStorage2D(GL_TEXTURE_2D, 1, get_sized_format(i.format_mode), i.width, i.height);

				auto new_texture = create_texture_2d();
				if (!new_texture)
					return new_texture.error();

				ogl_texture_2d* depth_stencil = (ogl_texture_2d*)*new_texture;
				depth_stencil->format_mode = i.format_mode;
				depth_stencil->format = get_sized_format(i.format_mode);
				depth_stencil->mip_levels = 0;
				depth_stencil->width = i.width;
				depth_stencil->height = i.height;
				depth_stencil->resource = result->depth_texture;
				result->has_stencil_buffer = (i.format_mode != format::d16_unorm && i.format_mode != format::d32_float);
				result->resource = depth_stencil;

				glGenFramebuffers(1, &result->frame_buffer);
				glBindFramebuffer(GL_FRAMEBUFFER, result->frame_buffer);
				if (result->has_stencil_buffer)
					glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, result->depth_texture, 0);
				else
					glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, result->depth_texture, 0);
				glBindFramebuffer(GL_FRAMEBUFFER, GL_NONE);
				glBindTexture(GL_TEXTURE_2D, GL_NONE);

				result->viewarea.width = (float)i.width;
				result->viewarea.height = (float)i.height;
				result->viewarea.top_left_x = 0.0f;
				result->viewarea.top_left_y = 0.0f;
				result->viewarea.min_depth = 0.0f;
				result->viewarea.max_depth = 1.0f;
				return result.reset();
			}
			expects_graphics<depth_target_cube*> ogl_device::create_depth_target_cube(const depth_target_cube::desc& i)
			{
				core::uptr<ogl_depth_target_cube> result = new ogl_depth_target_cube(i);
				bool no_stencil = (i.format_mode == format::d16_unorm || i.format_mode == format::d32_float);
				GLenum size_format = get_sized_format(i.format_mode);
				GLenum component_format = (no_stencil ? GL_DEPTH_COMPONENT : GL_DEPTH_STENCIL);

				glGenTextures(1, &result->depth_texture);
				glBindTexture(GL_TEXTURE_CUBE_MAP, result->depth_texture);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_FUNC, GL_GREATER);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 0);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, 0);
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + 0, 0, size_format, i.size, i.size, 0, component_format, GL_UNSIGNED_BYTE, 0);
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + 1, 0, size_format, i.size, i.size, 0, component_format, GL_UNSIGNED_BYTE, 0);
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + 2, 0, size_format, i.size, i.size, 0, component_format, GL_UNSIGNED_BYTE, 0);
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + 3, 0, size_format, i.size, i.size, 0, component_format, GL_UNSIGNED_BYTE, 0);
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + 4, 0, size_format, i.size, i.size, 0, component_format, GL_UNSIGNED_BYTE, 0);
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + 5, 0, size_format, i.size, i.size, 0, component_format, GL_UNSIGNED_BYTE, 0);

				auto new_texture = create_texture_cube();
				if (!new_texture)
					return new_texture.error();

				ogl_texture_cube* depth_stencil = (ogl_texture_cube*)*new_texture;
				depth_stencil->format_mode = i.format_mode;
				depth_stencil->format = get_sized_format(i.format_mode);
				depth_stencil->mip_levels = 0;
				depth_stencil->width = i.size;
				depth_stencil->height = i.size;
				depth_stencil->resource = result->depth_texture;
				result->has_stencil_buffer = (i.format_mode != format::d16_unorm && i.format_mode != format::d32_float);
				result->resource = depth_stencil;

				glGenFramebuffers(1, &result->frame_buffer);
				glBindFramebuffer(GL_FRAMEBUFFER, result->frame_buffer);
				if (result->has_stencil_buffer)
					glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, result->depth_texture, 0);
				else
					glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, result->depth_texture, 0);
				glBindFramebuffer(GL_FRAMEBUFFER, GL_NONE);
				glBindTexture(GL_TEXTURE_CUBE_MAP, GL_NONE);

				result->viewarea.width = (float)i.size;
				result->viewarea.height = (float)i.size;
				result->viewarea.top_left_x = 0.0f;
				result->viewarea.top_left_y = 0.0f;
				result->viewarea.min_depth = 0.0f;
				result->viewarea.max_depth = 1.0f;
				return result.reset();
			}
			expects_graphics<render_target_2d*> ogl_device::create_render_target_2d(const render_target_2d::desc& i)
			{
				core::uptr<ogl_render_target_2d> result = new ogl_render_target_2d(i);
				if (!i.render_surface)
				{
					GLenum format = ogl_device::get_sized_format(i.format_mode);
					glGenTextures(1, &result->frame_buffer.texture[0]);
					glBindTexture(GL_TEXTURE_2D, result->frame_buffer.texture[0]);
					if (i.mip_levels > 0)
					{
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, i.mip_levels - 1);
					}
					else
					{
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
					}
					glTexStorage2D(GL_TEXTURE_2D, i.mip_levels, format, i.width, i.height);

					auto new_texture = create_texture_2d();
					if (!new_texture)
						return new_texture.error();

					ogl_texture_2d* frontbuffer = (ogl_texture_2d*)*new_texture;
					frontbuffer->format_mode = i.format_mode;
					frontbuffer->format = get_sized_format(i.format_mode);
					frontbuffer->mip_levels = i.mip_levels;
					frontbuffer->width = i.width;
					frontbuffer->height = i.height;
					frontbuffer->resource = result->frame_buffer.texture[0];
					result->resource = frontbuffer;

					glGenTextures(1, &result->depth_texture);
					glBindTexture(GL_TEXTURE_2D, result->depth_texture);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_GREATER);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
					glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH24_STENCIL8, i.width, i.height);

					new_texture = create_texture_2d();
					if (!new_texture)
						return new_texture.error();

					ogl_texture_2d* depth_stencil = (ogl_texture_2d*)*new_texture;
					depth_stencil->format_mode = format::d24_unorm_s8_uint;
					depth_stencil->format = GL_DEPTH24_STENCIL8;
					depth_stencil->mip_levels = 0;
					depth_stencil->width = i.width;
					depth_stencil->height = i.height;
					depth_stencil->resource = result->depth_texture;
					result->depth_stencil = depth_stencil;

					glGenFramebuffers(1, &result->frame_buffer.buffer);
					glBindFramebuffer(GL_FRAMEBUFFER, result->frame_buffer.buffer);
					glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, result->frame_buffer.texture[0], 0);
					glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, result->depth_texture, 0);
					glBindFramebuffer(GL_FRAMEBUFFER, GL_NONE);
					glBindTexture(GL_TEXTURE_2D, GL_NONE);
				}
				else if (i.render_surface == (void*)this)
				{
					auto new_texture = create_texture_2d();
					if (!new_texture)
						return new_texture.error();

					ogl_texture_2d* backbuffer = (ogl_texture_2d*)*new_texture;
					backbuffer->format_mode = i.format_mode;
					backbuffer->format = get_sized_format(i.format_mode);
					backbuffer->mip_levels = i.mip_levels;
					backbuffer->width = i.width;
					backbuffer->height = i.height;
					backbuffer->backbuffer = true;

					result->frame_buffer.backbuffer = true;
					result->resource = backbuffer;
				}

				result->viewarea.width = (float)i.width;
				result->viewarea.height = (float)i.height;
				result->viewarea.top_left_x = 0.0f;
				result->viewarea.top_left_y = 0.0f;
				result->viewarea.min_depth = 0.0f;
				result->viewarea.max_depth = 1.0f;
				return result.reset();
			}
			expects_graphics<multi_render_target_2d*> ogl_device::create_multi_render_target_2d(const multi_render_target_2d::desc& i)
			{
				core::uptr<ogl_multi_render_target_2d> result = new ogl_multi_render_target_2d(i);
				glGenFramebuffers(1, &result->frame_buffer.buffer);
				glBindFramebuffer(GL_FRAMEBUFFER, result->frame_buffer.buffer);

				for (uint32_t j = 0; j < (uint32_t)i.target; j++)
				{
					GLenum format = ogl_device::get_sized_format(i.format_mode[j]);
					glGenTextures(1, &result->frame_buffer.texture[j]);
					glBindTexture(GL_TEXTURE_2D, result->frame_buffer.texture[j]);
					if (i.mip_levels > 0)
					{
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, i.mip_levels - 1);
					}
					else
					{
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
					}
					glTexStorage2D(GL_TEXTURE_2D, i.mip_levels, format, i.width, i.height);

					auto new_texture = create_texture_2d();
					if (!new_texture)
						return new_texture.error();

					ogl_texture_2d* frontbuffer = (ogl_texture_2d*)*new_texture;
					frontbuffer->format_mode = i.format_mode[j];
					frontbuffer->format = format;
					frontbuffer->mip_levels = i.mip_levels;
					frontbuffer->width = i.width;
					frontbuffer->height = i.height;
					frontbuffer->resource = result->frame_buffer.texture[j];
					result->resource[j] = frontbuffer;

					result->frame_buffer.format[j] = GL_COLOR_ATTACHMENT0 + j;
					glFramebufferTexture(GL_FRAMEBUFFER, result->frame_buffer.format[j], result->frame_buffer.texture[j], 0);
				}

				glGenTextures(1, &result->depth_texture);
				glBindTexture(GL_TEXTURE_2D, result->depth_texture);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_GREATER);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
				glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH24_STENCIL8, i.width, i.height);

				auto new_texture = create_texture_2d();
				if (!new_texture)
					return new_texture.error();

				ogl_texture_2d* depth_stencil = (ogl_texture_2d*)*new_texture;
				depth_stencil->format_mode = format::d24_unorm_s8_uint;
				depth_stencil->format = GL_DEPTH24_STENCIL8;
				depth_stencil->mip_levels = i.mip_levels;
				depth_stencil->width = i.width;
				depth_stencil->height = i.height;
				depth_stencil->resource = result->depth_texture;
				result->depth_stencil = depth_stencil;

				glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, result->depth_texture, 0);
				glBindFramebuffer(GL_FRAMEBUFFER, GL_NONE);
				glBindTexture(GL_TEXTURE_2D, GL_NONE);

				result->viewarea.width = (float)i.width;
				result->viewarea.height = (float)i.height;
				result->viewarea.top_left_x = 0.0f;
				result->viewarea.top_left_y = 0.0f;
				result->viewarea.min_depth = 0.0f;
				result->viewarea.max_depth = 1.0f;
				return result.reset();
			}
			expects_graphics<render_target_cube*> ogl_device::create_render_target_cube(const render_target_cube::desc& i)
			{
				core::uptr<ogl_render_target_cube> result = new ogl_render_target_cube(i);
				GLint size_format = ogl_device::get_sized_format(i.format_mode);
				GLint base_format = ogl_device::get_base_format(i.format_mode);
				glGenTextures(1, &result->frame_buffer.texture[0]);
				glBindTexture(GL_TEXTURE_CUBE_MAP, result->frame_buffer.texture[0]);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
				if (i.mip_levels > 0)
				{
					glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
					glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
					glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 0);
					glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, i.mip_levels - 1);
				}
				else
				{
					glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
					glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				}
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + 0, 0, size_format, i.size, i.size, 0, base_format, GL_UNSIGNED_BYTE, 0);
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + 1, 0, size_format, i.size, i.size, 0, base_format, GL_UNSIGNED_BYTE, 0);
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + 2, 0, size_format, i.size, i.size, 0, base_format, GL_UNSIGNED_BYTE, 0);
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + 3, 0, size_format, i.size, i.size, 0, base_format, GL_UNSIGNED_BYTE, 0);
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + 4, 0, size_format, i.size, i.size, 0, base_format, GL_UNSIGNED_BYTE, 0);
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + 5, 0, size_format, i.size, i.size, 0, base_format, GL_UNSIGNED_BYTE, 0);

				auto new_texture = create_texture_cube();
				if (!new_texture)
					return new_texture.error();

				ogl_texture_cube* base = (ogl_texture_cube*)*new_texture;
				base->format_mode = i.format_mode;
				base->format = size_format;
				base->mip_levels = i.mip_levels;
				base->resource = result->frame_buffer.texture[0];
				base->width = i.size;
				base->height = i.size;
				result->resource = base;

				glGenTextures(1, &result->depth_texture);
				glBindTexture(GL_TEXTURE_CUBE_MAP, result->depth_texture);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_FUNC, GL_GREATER);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 0);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, 0);
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + 0, 0, GL_DEPTH24_STENCIL8, i.size, i.size, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, 0);
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + 1, 0, GL_DEPTH24_STENCIL8, i.size, i.size, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, 0);
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + 2, 0, GL_DEPTH24_STENCIL8, i.size, i.size, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, 0);
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + 3, 0, GL_DEPTH24_STENCIL8, i.size, i.size, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, 0);
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + 4, 0, GL_DEPTH24_STENCIL8, i.size, i.size, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, 0);
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + 5, 0, GL_DEPTH24_STENCIL8, i.size, i.size, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, 0);

				auto new_subtexture = create_texture_2d();
				if (!new_subtexture)
					return new_subtexture.error();

				ogl_texture_2d* depth_stencil = (ogl_texture_2d*)*new_subtexture;
				depth_stencil->format_mode = format::d24_unorm_s8_uint;
				depth_stencil->format = GL_DEPTH24_STENCIL8;
				depth_stencil->mip_levels = 0;
				depth_stencil->resource = result->depth_texture;
				depth_stencil->width = i.size;
				depth_stencil->height = i.size;
				result->depth_stencil = depth_stencil;

				glGenFramebuffers(1, &result->frame_buffer.buffer);
				glBindFramebuffer(GL_FRAMEBUFFER, result->frame_buffer.buffer);
				glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, result->frame_buffer.texture[0], 0);
				glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, result->depth_texture, 0);
				glBindFramebuffer(GL_FRAMEBUFFER, GL_NONE);
				glBindTexture(GL_TEXTURE_CUBE_MAP, GL_NONE);

				result->viewarea.width = (float)i.size;
				result->viewarea.height = (float)i.size;
				result->viewarea.top_left_x = 0.0f;
				result->viewarea.top_left_y = 0.0f;
				result->viewarea.min_depth = 0.0f;
				result->viewarea.max_depth = 1.0f;
				return result.reset();
			}
			expects_graphics<multi_render_target_cube*> ogl_device::create_multi_render_target_cube(const multi_render_target_cube::desc& i)
			{
				core::uptr<ogl_multi_render_target_cube> result = new ogl_multi_render_target_cube(i);
				glGenFramebuffers(1, &result->frame_buffer.buffer);
				glBindFramebuffer(GL_FRAMEBUFFER, result->frame_buffer.buffer);

				for (uint32_t j = 0; j < (uint32_t)i.target; j++)
				{
					GLint size_format = ogl_device::get_sized_format(i.format_mode[j]);
					GLint base_format = ogl_device::get_base_format(i.format_mode[j]);
					glGenTextures(1, &result->frame_buffer.texture[j]);
					glBindTexture(GL_TEXTURE_CUBE_MAP, result->frame_buffer.texture[j]);
					glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
					glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
					glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
					if (i.mip_levels > 0)
					{
						glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
						glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
						glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 0);
						glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, i.mip_levels - 1);
					}
					else
					{
						glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
						glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
					}
					glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + 0, 0, size_format, i.size, i.size, 0, base_format, GL_UNSIGNED_BYTE, 0);
					glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + 1, 0, size_format, i.size, i.size, 0, base_format, GL_UNSIGNED_BYTE, 0);
					glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + 2, 0, size_format, i.size, i.size, 0, base_format, GL_UNSIGNED_BYTE, 0);
					glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + 3, 0, size_format, i.size, i.size, 0, base_format, GL_UNSIGNED_BYTE, 0);
					glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + 4, 0, size_format, i.size, i.size, 0, base_format, GL_UNSIGNED_BYTE, 0);
					glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + 5, 0, size_format, i.size, i.size, 0, base_format, GL_UNSIGNED_BYTE, 0);

					auto new_texture = create_texture_cube();
					if (!new_texture)
						return new_texture.error();

					ogl_texture_cube* frontbuffer = (ogl_texture_cube*)*new_texture;
					frontbuffer->format_mode = i.format_mode[j];
					frontbuffer->format = size_format;
					frontbuffer->mip_levels = i.mip_levels;
					frontbuffer->width = i.size;
					frontbuffer->height = i.size;
					frontbuffer->resource = result->frame_buffer.texture[j];
					result->resource[j] = frontbuffer;

					result->frame_buffer.format[j] = GL_COLOR_ATTACHMENT0 + j;
					glFramebufferTexture(GL_FRAMEBUFFER, result->frame_buffer.format[j], result->frame_buffer.texture[j], 0);
				}

				glGenTextures(1, &result->depth_texture);
				glBindTexture(GL_TEXTURE_CUBE_MAP, result->depth_texture);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_FUNC, GL_GREATER);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 0);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, 0);
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + 0, 0, GL_DEPTH24_STENCIL8, i.size, i.size, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, 0);
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + 1, 0, GL_DEPTH24_STENCIL8, i.size, i.size, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, 0);
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + 2, 0, GL_DEPTH24_STENCIL8, i.size, i.size, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, 0);
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + 3, 0, GL_DEPTH24_STENCIL8, i.size, i.size, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, 0);
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + 4, 0, GL_DEPTH24_STENCIL8, i.size, i.size, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, 0);
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + 5, 0, GL_DEPTH24_STENCIL8, i.size, i.size, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, 0);

				auto new_texture = create_texture_2d();
				if (!new_texture)
					return new_texture.error();

				ogl_texture_2d* depth_stencil = (ogl_texture_2d*)*new_texture;
				depth_stencil->format_mode = format::d24_unorm_s8_uint;
				depth_stencil->format = GL_DEPTH24_STENCIL8;
				depth_stencil->mip_levels = 0;
				depth_stencil->width = i.size;
				depth_stencil->height = i.size;
				depth_stencil->resource = result->depth_texture;
				result->depth_stencil = depth_stencil;

				glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, result->depth_texture, 0);
				glBindFramebuffer(GL_FRAMEBUFFER, GL_NONE);
				glBindTexture(GL_TEXTURE_CUBE_MAP, GL_NONE);

				result->viewarea.width = (float)i.size;
				result->viewarea.height = (float)i.size;
				result->viewarea.top_left_x = 0.0f;
				result->viewarea.top_left_y = 0.0f;
				result->viewarea.min_depth = 0.0f;
				result->viewarea.max_depth = 1.0f;
				return result.reset();
			}
			expects_graphics<cubemap*> ogl_device::create_cubemap(const cubemap::desc& i)
			{
				return new ogl_cubemap(i);
			}
			expects_graphics<query*> ogl_device::create_query(const query::desc& i)
			{
				ogl_query* result = new ogl_query();
				result->predicate = i.predicate;
				glGenQueries(1, &result->async);

				return result;
			}
			primitive_topology ogl_device::get_primitive_topology() const
			{
				return regs.primitive;
			}
			shader_model ogl_device::get_supported_shader_model() const
			{
				const char* version = (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);
				if (!version)
					return shader_model::invalid;

				int major, minor;
				if (sscanf(version, "%i.%i", &major, &minor) != 2)
					return shader_model::invalid;

				if (major == 1)
				{
					if (minor <= 10)
						return shader_model::glsl_1_1_0;

					if (minor <= 20)
						return shader_model::glsl_1_2_0;

					if (minor <= 30)
						return shader_model::glsl_1_3_0;

					if (minor <= 40)
						return shader_model::glsl_1_4_0;

					if (minor <= 50)
						return shader_model::glsl_1_5_0;

					return shader_model::glsl_1_5_0;
				}
				else if (major == 2 || major == 3)
				{
					if (minor <= 30)
						return shader_model::glsl_3_3_0;

					return shader_model::glsl_1_5_0;
				}
				else if (major == 4)
				{
					if (minor <= 10)
						return shader_model::glsl_4_1_0;

					if (minor <= 20)
						return shader_model::glsl_4_2_0;

					if (minor <= 30)
						return shader_model::glsl_4_3_0;

					if (minor <= 40)
						return shader_model::glsl_4_4_0;

					if (minor <= 50)
						return shader_model::glsl_4_5_0;

					if (minor <= 60)
						return shader_model::glsl_4_6_0;

					return shader_model::glsl_4_6_0;
				}

				return shader_model::invalid;
			}
			void* ogl_device::get_device() const
			{
				return context;
			}
			void* ogl_device::get_context() const
			{
				return context;
			}
			bool ogl_device::is_valid() const
			{
				return context != nullptr;
			}
			void ogl_device::set_vsync_mode(vsync mode)
			{
				graphics_device::set_vsync_mode(mode);
				switch (vsync_mode)
				{
					case vsync::off:
						video::glew::set_swap_interval(0);
						break;
					case vsync::frequency_x1:
						video::glew::set_swap_interval(1);
						break;
					case vsync::frequency_x2:
					case vsync::frequency_x3:
					case vsync::frequency_x4:
						if (!video::glew::set_swap_interval(-1))
							video::glew::set_swap_interval(1);
						break;
				}
			}
			const std::string_view& ogl_device::get_shader_version()
			{
				return shader_version;
			}
			expects_graphics<void> ogl_device::copy_constant_buffer(GLuint buffer, void* data, size_t size)
			{
				VI_ASSERT(data != nullptr, "buffer should not be empty");
				if (!size)
					return graphics_exception("copy constant buffer: zero size");

				glBindBuffer(GL_UNIFORM_BUFFER, buffer);
				glBufferSubData(GL_UNIFORM_BUFFER, 0, size, data);
				glBindBuffer(GL_UNIFORM_BUFFER, GL_NONE);
				return core::expectation::met;
			}
			expects_graphics<void> ogl_device::create_constant_buffer(GLuint* buffer, size_t size)
			{
				VI_ASSERT(buffer != nullptr, "buffer should be set");
				glGenBuffers(1, buffer);
				glBindBuffer(GL_UNIFORM_BUFFER, *buffer);
				glBufferData(GL_UNIFORM_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
				glBindBuffer(GL_UNIFORM_BUFFER, GL_NONE);
				if (!glIsBuffer(*buffer))
					return get_exception("create constant buffer");

				return core::expectation::met;
			}
			expects_graphics<void> ogl_device::create_direct_buffer(size_t size)
			{
				max_elements = size + 1;
				set_input_layout(nullptr);
				set_vertex_buffers(nullptr, 0);
				set_index_buffer(nullptr, format::unknown);

				GLint status_code;
				if (!immediate.sampler)
				{
					ogl_sampler_state* state = (ogl_sampler_state*)get_sampler_state("a16_fa_wrap");
					if (state != nullptr)
						immediate.sampler = state->resource;
				}

				if (immediate.vertex_shader == GL_NONE)
				{
					static const char* vertex_shader_code = OGL_INLINE(
						uniform object
						{
							mat4 transform;
					vec4 padding;
						};

					layout(location = 0) in vec3 iPosition;
					layout(location = 1) in vec2 iTexcoord;
					layout(location = 2) in vec4 iColor;

					out vec2 oTexcoord;
					out vec4 oColor;

					void main()
					{
						gl_Position = transform * vec4(iPosition.xyz, 1.0);
						oTexcoord = iTexcoord;
						oColor = iColor;
					});

					core::string result = core::string(shader_version);
					result.append(vertex_shader_code);

					const char* subbuffer = result.data();
					GLint buffer_size = (GLint)result.size();
					immediate.vertex_shader = glCreateShader(GL_VERTEX_SHADER);
					glShaderSourceARB(immediate.vertex_shader, 1, &subbuffer, &buffer_size);
					glCompileShaderARB(immediate.vertex_shader);
					glGetShaderiv(immediate.vertex_shader, GL_COMPILE_STATUS, &status_code);
					if (status_code == GL_FALSE)
					{
						glGetShaderiv(immediate.vertex_shader, GL_INFO_LOG_LENGTH, &buffer_size);
						char* buffer = core::memory::allocate<char>(sizeof(char) * (buffer_size + 1));
						glGetShaderInfoLog(immediate.vertex_shader, buffer_size, &buffer_size, buffer);
						core::string error_text(buffer, (size_t)buffer_size);
						core::memory::deallocate(buffer);
						return graphics_exception(std::move(error_text));
					}
				}

				if (immediate.pixel_shader == GL_NONE)
				{
					static const char* pixel_shader_code = OGL_INLINE(
						uniform object
						{
							mat4 transform;
					vec4 padding;
						};
					uniform sampler2D diffuse;

					in vec2 oTexcoord;
					in vec4 oColor;

					out vec4 oResult;

					void main()
					{
						if (padding.z > 0)
							oResult = oColor * textureLod(diffuse, oTexcoord + padding.xy, 0.0) * padding.w;
						else
							oResult = oColor * padding.w;
					});

					core::string result = core::string(shader_version);
					result.append(pixel_shader_code);

					const char* subbuffer = result.data();
					GLint buffer_size = (GLint)result.size();
					immediate.pixel_shader = glCreateShader(GL_FRAGMENT_SHADER);
					glShaderSourceARB(immediate.pixel_shader, 1, &subbuffer, &buffer_size);
					glCompileShaderARB(immediate.pixel_shader);
					glGetShaderiv(immediate.pixel_shader, GL_COMPILE_STATUS, &status_code);
					if (status_code == GL_FALSE)
					{
						glGetShaderiv(immediate.pixel_shader, GL_INFO_LOG_LENGTH, &buffer_size);
						char* buffer = core::memory::allocate<char>(sizeof(char) * (buffer_size + 1));
						glGetShaderInfoLog(immediate.pixel_shader, buffer_size, &buffer_size, buffer);
						core::string error_text(buffer, (size_t)buffer_size);
						core::memory::deallocate(buffer);
						return graphics_exception(std::move(error_text));
					}
				}

				if (immediate.program == GL_NONE)
				{
					immediate.program = glCreateProgram();
					glAttachShader(immediate.program, immediate.vertex_shader);
					glAttachShader(immediate.program, immediate.pixel_shader);
					glLinkProgramARB(immediate.program);
					glGetProgramiv(immediate.program, GL_LINK_STATUS, &status_code);

					if (status_code == GL_FALSE)
					{
						GLint buffer_size = 0;
						glGetProgramiv(immediate.program, GL_INFO_LOG_LENGTH, &buffer_size);

						char* buffer = core::memory::allocate<char>(sizeof(char) * (buffer_size + 1));
						glGetProgramInfoLog(immediate.program, buffer_size, &buffer_size, buffer);
						core::string error_text(buffer, (size_t)buffer_size);
						core::memory::deallocate(buffer);

						glDeleteProgram(immediate.program);
						immediate.program = GL_NONE;
						return graphics_exception(std::move(error_text));
					}
				}

				if (immediate.vertex_array != GL_NONE)
					glDeleteVertexArrays(1, &immediate.vertex_array);

				if (immediate.vertex_buffer != GL_NONE)
					glDeleteBuffers(1, &immediate.vertex_buffer);

				if (immediate.constant_buffer != GL_NONE)
					glDeleteBuffers(1, &immediate.constant_buffer);

				glGenBuffers(1, &immediate.vertex_buffer);
				glGenVertexArrays(1, &immediate.vertex_array);
				glBindVertexArray(immediate.vertex_array);
				glBindBuffer(GL_ARRAY_BUFFER, immediate.vertex_buffer);
				glBufferData(GL_ARRAY_BUFFER, sizeof(vertex)* (max_elements + 1), elements.empty() ? nullptr : &elements[0], GL_DYNAMIC_DRAW);
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), OGL_OFFSET(0));
				glEnableVertexAttribArray(0);
				glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), OGL_OFFSET(sizeof(float) * 3));
				glEnableVertexAttribArray(1);
				glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(vertex), OGL_OFFSET(sizeof(float) * 5));
				glEnableVertexAttribArray(2);
				glBindVertexArray(0);

				auto status = create_constant_buffer(&immediate.constant_buffer, sizeof(direct_buffer));
				if (!status)
					return status;

				set_vertex_buffers(nullptr, 0);
				return core::expectation::met;
			}
			uint64_t ogl_device::get_program_hash()
			{
				static uint64_t seed = compute::crypto::random();
				uint64_t program_hash = seed;
				rehash<void*>(program_hash, regs.shaders[0]);
				rehash<void*>(program_hash, regs.shaders[1]);
				rehash<void*>(program_hash, regs.shaders[2]);
				rehash<void*>(program_hash, regs.shaders[3]);
				rehash<void*>(program_hash, regs.shaders[4]);
				rehash<void*>(program_hash, regs.shaders[5]);
				return program_hash;
			}
			core::string ogl_device::compile_state(GLuint handle)
			{
				GLint stat = 0, size = 0;
				glGetShaderiv(handle, GL_COMPILE_STATUS, &stat);
				glGetShaderiv(handle, GL_INFO_LOG_LENGTH, &size);

				if ((GLboolean)stat == GL_TRUE || !size)
					return "";

				GLchar* buffer = core::memory::allocate<GLchar>(sizeof(GLchar) * size);
				glGetShaderInfoLog(handle, size, NULL, buffer);

				core::string result((char*)buffer, size);
				core::memory::deallocate(buffer);

				return result;
			}
			bool ogl_device::is_comparator(pixel_filter value)
			{
				switch (value)
				{
					case pixel_filter::compare_min_mag_mip_point:
					case pixel_filter::compare_min_mag_point_mip_linear:
					case pixel_filter::compare_min_point_mag_linear_mip_point:
					case pixel_filter::compare_min_point_mag_mip_linear:
					case pixel_filter::compare_min_linear_mag_mip_point:
					case pixel_filter::compare_min_linear_mag_point_mip_linear:
					case pixel_filter::compare_min_mag_linear_mip_point:
					case pixel_filter::compare_min_mag_mip_linear:
					case pixel_filter::compare_anistropic:
						return true;
					default:
						return false;
				}
			}
			GLenum ogl_device::get_access_control(cpu_access access, resource_usage usage)
			{
				switch (usage)
				{
					case vitex::graphics::resource_usage::defaults:
						switch (access)
						{
							case vitex::graphics::cpu_access::read:
								return GL_STATIC_READ;
							case vitex::graphics::cpu_access::none:
							case vitex::graphics::cpu_access::write:
							default:
								return GL_STATIC_DRAW;
						}
					case vitex::graphics::resource_usage::immutable:
						return GL_STATIC_DRAW;
					case vitex::graphics::resource_usage::dynamic:
						switch (access)
						{
							case vitex::graphics::cpu_access::read:
								return GL_DYNAMIC_READ;
							case vitex::graphics::cpu_access::none:
							case vitex::graphics::cpu_access::write:
							default:
								return GL_DYNAMIC_DRAW;
						}
					case vitex::graphics::resource_usage::staging:
						return GL_DYNAMIC_READ;
					default:
						return GL_STATIC_DRAW;
				}
			}
			GLenum ogl_device::get_base_format(format value)
			{
				switch (value)
				{
					case format::r32g32b32a32_float:
					case format::r32g32b32a32_uint:
					case format::r32g32b32a32_sint:
					case format::r16g16b16a16_float:
					case format::r16g16b16a16_unorm:
					case format::r16g16b16a16_uint:
					case format::r16g16b16a16_snorm:
					case format::r16g16b16a16_sint:
					case format::r10g10b10a2_unorm:
					case format::r10g10b10a2_uint:
					case format::r8g8b8a8_unorm:
					case format::r8g8b8a8_unorm_srgb:
					case format::r8g8b8a8_uint:
					case format::r8g8b8a8_snorm:
					case format::r8g8b8a8_sint:
					case format::r9g9b9e5_share_dexp:
					case format::r8g8b8g8_unorm:
						return GL_RGBA;
					case format::r32g32b32_float:
					case format::r32g32b32_uint:
					case format::r32g32b32_sint:
					case format::r11g11b10_float:
						return GL_RGB;
					case format::r16g16_float:
					case format::r16g16_unorm:
					case format::r16g16_uint:
					case format::r16g16_snorm:
					case format::r16g16_sint:
					case format::r32g32_float:
					case format::r32g32_uint:
					case format::r32g32_sint:
					case format::r8g8_unorm:
					case format::r8g8_uint:
					case format::r8g8_snorm:
					case format::r8g8_sint:
						return GL_RG;
					case format::d24_unorm_s8_uint:
						return GL_DEPTH_STENCIL;
					case format::d32_float:
					case format::d16_unorm:
						return GL_DEPTH_COMPONENT;
					case format::r32_float:
					case format::r32_uint:
					case format::r32_sint:
					case format::r16_float:
					case format::r16_unorm:
					case format::r16_uint:
					case format::r16_snorm:
					case format::r16_sint:
					case format::r8_unorm:
					case format::r8_uint:
					case format::r8_snorm:
					case format::r8_sint:
					case format::a8_unorm:
					case format::r1_unorm:
						return GL_RED;
					default:
						return GL_RGBA;
				}
			}
			GLenum ogl_device::get_sized_format(format value)
			{
				switch (value)
				{
					case format::r32g32b32a32_float:
						return GL_RGBA32F;
					case format::r32g32b32a32_uint:
						return GL_RGBA32UI;
					case format::r32g32b32a32_sint:
						return GL_RGBA32I;
					case format::r32g32b32_float:
						return GL_RGB32F;
					case format::r32g32b32_uint:
						return GL_RGB32UI;
					case format::r32g32b32_sint:
						return GL_RGB32I;
					case format::r16g16b16a16_float:
						return GL_RGBA16F;
					case format::r16g16b16a16_unorm:
						return GL_RGBA16;
					case format::r16g16b16a16_uint:
						return GL_RGBA16UI;
					case format::r16g16b16a16_snorm:
						return GL_RGBA16_SNORM;
					case format::r16g16b16a16_sint:
						return GL_RGBA16I;
					case format::r32g32_float:
						return GL_RG32F;
					case format::r32g32_uint:
						return GL_RG32UI;
					case format::r32g32_sint:
						return GL_RG32I;
					case format::r10g10b10a2_unorm:
						return GL_RGB10_A2;
					case format::r10g10b10a2_uint:
						return GL_RGB10_A2UI;
					case format::r11g11b10_float:
						return GL_R11F_G11F_B10F;
					case format::r8g8b8a8_unorm:
						return GL_RGBA8;
					case format::r8g8b8a8_unorm_srgb:
						return GL_SRGB8_ALPHA8;
					case format::r8g8b8a8_uint:
						return GL_RGBA8UI;
					case format::r8g8b8a8_snorm:
						return GL_RGBA8I;
					case format::r8g8b8a8_sint:
						return GL_RGBA8I;
					case format::r16g16_float:
						return GL_RG16F;
					case format::r16g16_unorm:
						return GL_RG16;
					case format::r16g16_uint:
						return GL_RG16UI;
					case format::r16g16_snorm:
						return GL_RG16_SNORM;
					case format::r16g16_sint:
						return GL_RG16I;
					case format::d32_float:
						return GL_DEPTH_COMPONENT32F;
					case format::r32_float:
						return GL_R32F;
					case format::r32_uint:
						return GL_R32UI;
					case format::r32_sint:
						return GL_R32I;
					case format::d24_unorm_s8_uint:
						return GL_DEPTH24_STENCIL8;
					case format::r8g8_unorm:
						return GL_RG8;
					case format::r8g8_uint:
						return GL_RG8UI;
					case format::r8g8_snorm:
						return GL_RG8_SNORM;
					case format::r8g8_sint:
						return GL_RG8I;
					case format::r16_float:
						return GL_R16F;
					case format::d16_unorm:
						return GL_DEPTH_COMPONENT16;
					case format::r16_unorm:
						return GL_R16;
					case format::r16_uint:
						return GL_R16UI;
					case format::r16_snorm:
						return GL_R16_SNORM;
					case format::r16_sint:
						return GL_R16I;
					case format::r8_unorm:
						return GL_R8;
					case format::r8_uint:
						return GL_R8UI;
					case format::r8_snorm:
						return GL_R8_SNORM;
					case format::r8_sint:
						return GL_R8I;
					case format::r1_unorm:
						return GL_R8;
					case format::r9g9b9e5_share_dexp:
						return GL_RGB9_E5;
					case format::r8g8b8g8_unorm:
						return GL_RGB8;
					case format::a8_unorm:
#ifdef GL_ALPHA8_EXT
						return GL_ALPHA8_EXT;
#else
						return GL_R8;
#endif
					default:
						break;
				}

				return GL_RGB;
			}
			GLenum ogl_device::get_texture_address(texture_address value)
			{
				switch (value)
				{
					case texture_address::wrap:
						return GL_REPEAT;
					case texture_address::mirror:
						return GL_MIRRORED_REPEAT;
					case texture_address::clamp:
						return GL_CLAMP_TO_EDGE;
					case texture_address::border:
						return GL_CLAMP_TO_BORDER;
					case texture_address::mirror_once:
						return GL_MIRROR_CLAMP_TO_EDGE;
				}

				return GL_REPEAT;
			}
			GLenum ogl_device::get_comparison(comparison value)
			{
				switch (value)
				{
					case comparison::never:
						return GL_NEVER;
					case comparison::less:
						return GL_LESS;
					case comparison::equal:
						return GL_EQUAL;
					case comparison::less_equal:
						return GL_LEQUAL;
					case comparison::greater:
						return GL_GREATER;
					case comparison::not_equal:
						return GL_NOTEQUAL;
					case comparison::greater_equal:
						return GL_GEQUAL;
					case comparison::always:
						return GL_ALWAYS;
				}

				return GL_ALWAYS;
			}
			GLenum ogl_device::get_pixel_filter(pixel_filter value, bool mag)
			{
				switch (value)
				{
					case pixel_filter::min_mag_mip_point:
					case pixel_filter::compare_min_mag_mip_point:
						return (mag ? GL_NEAREST : GL_NEAREST);
					case pixel_filter::min_mag_point_mip_linear:
					case pixel_filter::compare_min_mag_point_mip_linear:
						return (mag ? GL_NEAREST : GL_NEAREST_MIPMAP_LINEAR);
					case pixel_filter::min_point_mag_linear_mip_point:
					case pixel_filter::compare_min_point_mag_linear_mip_point:
						return (mag ? GL_NEAREST : GL_LINEAR_MIPMAP_NEAREST);
					case pixel_filter::min_point_mag_mip_linear:
					case pixel_filter::compare_min_point_mag_mip_linear:
						return (mag ? GL_NEAREST : GL_LINEAR_MIPMAP_LINEAR);
					case pixel_filter::min_linear_mag_mip_point:
					case pixel_filter::compare_min_linear_mag_mip_point:
						return (mag ? GL_LINEAR : GL_NEAREST_MIPMAP_NEAREST);
					case pixel_filter::min_linear_mag_point_mip_linear:
					case pixel_filter::compare_min_linear_mag_point_mip_linear:
						return (mag ? GL_LINEAR : GL_NEAREST_MIPMAP_LINEAR);
					case pixel_filter::min_mag_linear_mip_point:
					case pixel_filter::compare_min_mag_linear_mip_point:
						return (mag ? GL_LINEAR : GL_LINEAR_MIPMAP_NEAREST);
					case pixel_filter::anistropic:
					case pixel_filter::compare_anistropic:
					case pixel_filter::min_mag_mip_linear:
					case pixel_filter::compare_min_mag_mip_linear:
						return (mag ? GL_LINEAR : GL_LINEAR_MIPMAP_LINEAR);
				}

				return GL_NEAREST;
			}
			GLenum ogl_device::get_blend_operation(blend_operation value)
			{
				switch (value)
				{
					case blend_operation::add:
						return GL_FUNC_ADD;
					case blend_operation::subtract:
						return GL_FUNC_SUBTRACT;
					case blend_operation::subtract_reverse:
						return GL_FUNC_REVERSE_SUBTRACT;
					case blend_operation::min:
						return GL_MIN;
					case blend_operation::max:
						return GL_MAX;
				}

				return GL_FUNC_ADD;
			}
			GLenum ogl_device::get_blend(blend value)
			{
				switch (value)
				{
					case blend::zero:
						return GL_ZERO;
					case blend::one:
						return GL_ONE;
					case blend::source_color:
						return GL_SRC_COLOR;
					case blend::source_color_invert:
						return GL_ONE_MINUS_SRC_COLOR;
					case blend::source_alpha:
						return GL_SRC_ALPHA;
					case blend::source_alpha_invert:
						return GL_ONE_MINUS_SRC_ALPHA;
					case blend::destination_alpha:
						return GL_DST_ALPHA;
					case blend::destination_alpha_invert:
						return GL_ONE_MINUS_DST_ALPHA;
					case blend::destination_color:
						return GL_DST_COLOR;
					case blend::destination_color_invert:
						return GL_ONE_MINUS_DST_COLOR;
					case blend::source_alpha_sat:
						return GL_SRC_ALPHA_SATURATE;
					case blend::blend_factor:
						return GL_CONSTANT_COLOR;
					case blend::blend_factor_invert:
						return GL_ONE_MINUS_CONSTANT_ALPHA;
					case blend::source1_color:
						return GL_SRC1_COLOR;
					case blend::source1_color_invert:
						return GL_ONE_MINUS_SRC1_COLOR;
					case blend::source1_alpha:
						return GL_SRC1_ALPHA;
					case blend::source1_alpha_invert:
						return GL_ONE_MINUS_SRC1_ALPHA;
				}

				return GL_ONE;
			}
			GLenum ogl_device::get_stencil_operation(stencil_operation value)
			{
				switch (value)
				{
					case stencil_operation::keep:
						return GL_KEEP;
					case stencil_operation::zero:
						return GL_ZERO;
					case stencil_operation::replace:
						return GL_REPLACE;
					case stencil_operation::sat_add:
						return GL_INCR_WRAP;
					case stencil_operation::sat_subtract:
						return GL_DECR_WRAP;
					case stencil_operation::invert:
						return GL_INVERT;
					case stencil_operation::add:
						return GL_INCR;
					case stencil_operation::subtract:
						return GL_DECR;
				}

				return GL_KEEP;
			}
			GLenum ogl_device::get_primitive_topology(primitive_topology value)
			{
				switch (value)
				{
					case primitive_topology::point_list:
						return GL_POINT;
					case primitive_topology::line_list:
					case primitive_topology::line_strip:
					case primitive_topology::line_list_adj:
					case primitive_topology::line_strip_adj:
						return GL_LINE;
					case primitive_topology::triangle_list:
					case primitive_topology::triangle_strip:
					case primitive_topology::triangle_list_adj:
					case primitive_topology::triangle_strip_adj:
						return GL_FILL;
					default:
						break;
				}

				return GL_FILL;
			}
			GLenum ogl_device::get_primitive_topology_draw(primitive_topology value)
			{
				switch (value)
				{
					case primitive_topology::point_list:
						return GL_POINTS;
					case primitive_topology::line_list:
						return GL_LINES;
					case primitive_topology::line_strip:
						return GL_LINE_STRIP;
					case primitive_topology::line_list_adj:
						return GL_LINES_ADJACENCY;
					case primitive_topology::line_strip_adj:
						return GL_LINE_STRIP_ADJACENCY;
					case primitive_topology::triangle_list:
						return GL_TRIANGLES;
					case primitive_topology::triangle_strip:
						return GL_TRIANGLE_STRIP;
					case primitive_topology::triangle_list_adj:
						return GL_TRIANGLES_ADJACENCY;
					case primitive_topology::triangle_strip_adj:
						return GL_TRIANGLE_STRIP_ADJACENCY;
					default:
						break;
				}

				return GL_TRIANGLES;
			}
			GLenum ogl_device::get_resource_bind(resource_bind value)
			{
				switch (value)
				{
					case resource_bind::vertex_buffer:
						return GL_ARRAY_BUFFER;
					case resource_bind::index_buffer:
						return GL_ELEMENT_ARRAY_BUFFER;
					case resource_bind::constant_buffer:
						return GL_UNIFORM_BUFFER;
					case resource_bind::shader_input:
						return GL_SHADER_STORAGE_BUFFER;
					case resource_bind::stream_output:
						return GL_TEXTURE_BUFFER;
					case resource_bind::render_target:
						return GL_DRAW_INDIRECT_BUFFER;
					case resource_bind::depth_stencil:
						return GL_DRAW_INDIRECT_BUFFER;
					case resource_bind::unordered_access:
						return GL_DISPATCH_INDIRECT_BUFFER;
				}

				return GL_ARRAY_BUFFER;
			}
			GLenum ogl_device::get_resource_map(resource_map value)
			{
				switch (value)
				{
					case resource_map::read:
						return GL_READ_ONLY;
					case resource_map::write:
						return GL_WRITE_ONLY;
					case resource_map::read_write:
						return GL_READ_WRITE;
					case resource_map::write_discard:
						return GL_WRITE_ONLY;
					case resource_map::write_no_overwrite:
						return GL_WRITE_ONLY;
				}

				return GL_READ_ONLY;
			}
			void ogl_device::get_back_buffer_size(format value, int* x, int* y, int* z, int* w)
			{
				VI_ASSERT(x && y && z && w, "xyzw should be set");
				switch (value)
				{
					case vitex::graphics::format::a8_unorm:
						*x = *y = *z = 0;
						*w = 8;
						break;
					case vitex::graphics::format::d24_unorm_s8_uint:
						*z = *w = 0;
						*y = 8;
						*x = 32;
						break;
					case vitex::graphics::format::d32_float:
						*y = *z = *w = 0;
						*x = 32;
						break;
					case vitex::graphics::format::r10g10b10a2_uint:
					case vitex::graphics::format::r10g10b10a2_unorm:
						*w = 2;
						*z = 10;
						*x = *y = 11;
						break;
					case vitex::graphics::format::r11g11b10_float:
						*w = 0;
						*z = 10;
						*x = *y = 11;
						break;
					case vitex::graphics::format::r16g16b16a16_float:
					case vitex::graphics::format::r16g16b16a16_sint:
					case vitex::graphics::format::r16g16b16a16_snorm:
					case vitex::graphics::format::r16g16b16a16_uint:
					case vitex::graphics::format::r16g16b16a16_unorm:
						*x = *y = *z = *w = 16;
						break;
					case vitex::graphics::format::r16g16_float:
					case vitex::graphics::format::r16g16_sint:
					case vitex::graphics::format::r16g16_snorm:
					case vitex::graphics::format::r16g16_uint:
					case vitex::graphics::format::r16g16_unorm:
						*z = *w = 0;
						*x = *y = 16;
						break;
					case vitex::graphics::format::r16_float:
					case vitex::graphics::format::r16_sint:
					case vitex::graphics::format::r16_snorm:
					case vitex::graphics::format::r16_uint:
					case vitex::graphics::format::r16_unorm:
					case vitex::graphics::format::d16_unorm:
						*y = *z = *w = 0;
						*x = 16;
						break;
					case vitex::graphics::format::r1_unorm:
						*y = *z = *w = 0;
						*x = 1;
						break;
					case vitex::graphics::format::r32g32b32a32_float:
					case vitex::graphics::format::r32g32b32a32_sint:
					case vitex::graphics::format::r32g32b32a32_uint:
						*x = *y = *z = *w = 32;
						break;
					case vitex::graphics::format::r32g32b32_float:
					case vitex::graphics::format::r32g32b32_sint:
					case vitex::graphics::format::r32g32b32_uint:
						*w = 0;
						*x = *y = *z = 32;
						break;
					case vitex::graphics::format::r32g32_float:
					case vitex::graphics::format::r32g32_sint:
					case vitex::graphics::format::r32g32_uint:
						*z = *w = 0;
						*x = *y = 32;
						break;
					case vitex::graphics::format::r32_float:
					case vitex::graphics::format::r32_sint:
					case vitex::graphics::format::r32_uint:
						*y = *z = *w = 0;
						*x = 32;
						break;
					case vitex::graphics::format::r8g8_sint:
					case vitex::graphics::format::r8g8_snorm:
					case vitex::graphics::format::r8g8_uint:
					case vitex::graphics::format::r8g8_unorm:
						*z = *w = 0;
						*x = *y = 8;
						break;
					case vitex::graphics::format::r8_sint:
					case vitex::graphics::format::r8_snorm:
					case vitex::graphics::format::r8_uint:
					case vitex::graphics::format::r8_unorm:
						*y = *z = *w = 0;
						*x = 8;
						break;
					case vitex::graphics::format::r9g9b9e5_share_dexp:
						break;
					case vitex::graphics::format::r8g8b8a8_sint:
					case vitex::graphics::format::r8g8b8a8_snorm:
					case vitex::graphics::format::r8g8b8a8_uint:
					case vitex::graphics::format::r8g8b8a8_unorm:
					case vitex::graphics::format::r8g8b8a8_unorm_srgb:
					case vitex::graphics::format::r8g8b8g8_unorm:
					default:
						*x = *y = *z = *w = 8;
						break;
				}
			}
			void ogl_device::debug_message(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* data)
			{
				static const GLuint null_texture_cannot_be_sampled = 131204;
				if (id == null_texture_cannot_be_sampled)
					return;

				const char* _Source, * _Type;
				switch (source)
				{
					case GL_DEBUG_SOURCE_API:
						_Source = "API";
						break;
					case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
						_Source = "WINDOW SYSTEM";
						break;
					case GL_DEBUG_SOURCE_SHADER_COMPILER:
						_Source = "SHADER COMPILER";
						break;
					case GL_DEBUG_SOURCE_THIRD_PARTY:
						_Source = "THIRD PARTY";
						break;
					case GL_DEBUG_SOURCE_APPLICATION:
						_Source = "APPLICATION";
						break;
					default:
						_Source = "GENERAL";
						break;
				}

				switch (type)
				{
					case GL_DEBUG_TYPE_ERROR:
						_Type = "ERROR";
						break;
					case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
						_Type = "DEPRECATED";
						break;
					case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
						_Type = "UDEFINED";
						break;
					case GL_DEBUG_TYPE_PORTABILITY:
						_Type = "PORTABILITY";
						break;
					case GL_DEBUG_TYPE_PERFORMANCE:
						_Type = "PERFORMANCE";
						break;
					case GL_DEBUG_TYPE_OTHER:
						_Type = "OTHER";
						break;
					case GL_DEBUG_TYPE_MARKER:
						_Type = "MARKER";
						break;
					default:
						_Type = "LOG";
						break;
				}

				switch (severity)
				{
					case GL_DEBUG_SEVERITY_HIGH:
						VI_ERR("[opengl] %s (%s:%d): %s", _Source, _Type, id, message);
						break;
					case GL_DEBUG_SEVERITY_MEDIUM:
						VI_WARN("[opengl] %s (%s:%d): %s", _Source, _Type, id, message);
						break;
					case GL_DEBUG_SEVERITY_LOW:
						VI_DEBUG("[opengl] %s (%s:%d): %s", _Source, _Type, id, message);
						break;
					case GL_DEBUG_SEVERITY_NOTIFICATION:
						VI_TRACE("[opengl] %s (%s:%d): %s", _Source, _Type, id, message);
						break;
				}

				(void)_Source;
				(void)_Type;
			}
		}
	}
}
#pragma warning(pop)
#endif
