#include "gui.h"
#include <vitex/network.h>
#ifdef VI_RMLUI
#include <RmlUi/core.h>
#include <RmlUi/core/stream.h>
#include <RmlUi/core/RenderInterfaceCompatibility.h>
#include <source/core/StyleSheetFactory.h>
#include <source/core/ElementStyle.h>
#endif
#define RENDERS_FOR_DATA_EVENT 1
#define RENDERS_FOR_SIZE_EVENT 30
#pragma warning(push)
#pragma warning(disable: 4996)

namespace vitex
{
	namespace layer
	{
		namespace gui
		{
#ifdef VI_RMLUI
			struct geometry_buffer
			{
				graphics::element_buffer* vertex_buffer;
				graphics::element_buffer* index_buffer;
				graphics::texture_2d* texture;

				geometry_buffer() : vertex_buffer(nullptr), index_buffer(nullptr), texture(nullptr)
				{
				}
				~geometry_buffer()
				{
					core::memory::release(vertex_buffer);
					core::memory::release(index_buffer);
				}
			};

			class render_subsystem final : public Rml::RenderInterfaceCompatibility
			{
			private:
				struct
				{
					uint32_t diffuse_map = (uint32_t)-1;
					uint32_t sampler = (uint32_t)-1;
					uint32_t object = (uint32_t)-1;
				} slots;

			private:
				graphics::rasterizer_state* scissor_none_rasterizer;
				graphics::rasterizer_state* none_rasterizer;
				graphics::depth_stencil_state* less_depth_stencil;
				graphics::depth_stencil_state* none_depth_stencil;
				graphics::depth_stencil_state* scissor_depth_stencil;
				graphics::blend_state* alpha_blend;
				graphics::blend_state* colorless_blend;
				graphics::sampler_state* sampler;
				graphics::input_layout* layout;
				graphics::element_buffer* vertex_buffer;
				graphics::shader* shader;
				graphics::graphics_device* device;
				heavy_content_manager* content;
				render_constants* constants;
				trigonometry::matrix4x4 transform;
				trigonometry::matrix4x4 ortho;
				bool has_scissor;
				bool has_transform;

			public:
				graphics::texture_2d* background;

			public:
				render_subsystem() : Rml::RenderInterfaceCompatibility(), device(nullptr), content(nullptr), constants(nullptr), has_transform(false), has_scissor(false), background(nullptr)
				{
					shader = nullptr;
					vertex_buffer = nullptr;
					layout = nullptr;
					none_rasterizer = nullptr;
					scissor_none_rasterizer = nullptr;
					scissor_depth_stencil = nullptr;
					less_depth_stencil = nullptr;
					none_depth_stencil = nullptr;
					alpha_blend = nullptr;
					colorless_blend = nullptr;
					sampler = nullptr;
				}
				~render_subsystem() override
				{
					detach();
				}
				void RenderGeometry(Rml::Vertex* vertices, int vertices_size, int* indices, int indices_size, Rml::TextureHandle texture, const Rml::Vector2f& translation) override
				{
					VI_ASSERT(device != nullptr, "graphics device should be set");
					VI_ASSERT(vertices != nullptr, "vertices should be set");
					VI_ASSERT(indices != nullptr, "indices should be set");
					EnableScissorRegion(has_scissor);
					device->im_begin();
					device->im_topology(graphics::primitive_topology::triangle_list);
					device->im_texture((graphics::texture_2d*)texture);
					if (has_transform)
						device->im_transform(trigonometry::matrix4x4::create_translation(trigonometry::vector3(translation.x, translation.y)) * transform * ortho);
					else
						device->im_transform(trigonometry::matrix4x4::create_translation(trigonometry::vector3(translation.x, translation.y)) * ortho);

					for (int i = indices_size; i-- > 0;)
					{
						Rml::Vertex& v = vertices[indices[i]];
						device->im_emit();
						device->im_position(v.position.x, v.position.y, 0.0f);
						device->im_texcoord(v.tex_coord.x, v.tex_coord.y);
						device->im_color(v.colour.red / 255.0f, v.colour.green / 255.0f, v.colour.blue / 255.0f, v.colour.alpha / 255.0f);
					}
					device->im_end();
				}
				Rml::CompiledGeometryHandle CompileGeometry(Rml::Vertex* vertices, int vertices_count, int* indices, int indices_count, Rml::TextureHandle handle) override
				{
					VI_ASSERT(device != nullptr, "graphics device should be set");
					geometry_buffer* result = core::memory::init<geometry_buffer>();
					result->texture = (graphics::texture_2d*)handle;

					graphics::element_buffer::desc f = graphics::element_buffer::desc();
					f.access_flags = graphics::cpu_access::none;
					f.usage = graphics::resource_usage::defaults;
					f.bind_flags = graphics::resource_bind::vertex_buffer;
					f.element_count = (unsigned int)vertices_count;
					f.elements = (void*)vertices;
					f.element_width = sizeof(Rml::Vertex);
					result->vertex_buffer = *device->create_element_buffer(f);

					f = graphics::element_buffer::desc();
					f.access_flags = graphics::cpu_access::none;
					f.usage = graphics::resource_usage::defaults;
					f.bind_flags = graphics::resource_bind::index_buffer;
					f.element_count = (unsigned int)indices_count;
					f.element_width = sizeof(unsigned int);
					f.elements = (void*)indices;
					result->index_buffer = *device->create_element_buffer(f);

					return (Rml::CompiledGeometryHandle)result;
				}
				void RenderCompiledGeometry(Rml::CompiledGeometryHandle handle, const Rml::Vector2f& translation) override
				{
					geometry_buffer* buffer = (geometry_buffer*)handle;
					VI_ASSERT(device != nullptr, "graphics device should be set");
					VI_ASSERT(buffer != nullptr, "buffer should be set");

					constants->render.diffuse = (buffer->texture != nullptr ? 1.0f : 0.0f);
					if (has_transform)
						constants->render.transform = trigonometry::matrix4x4::create_translation(trigonometry::vector3(translation.x, translation.y)) * transform * ortho;
					else
						constants->render.transform = trigonometry::matrix4x4::create_translation(trigonometry::vector3(translation.x, translation.y)) * ortho;

					EnableScissorRegion(has_scissor);
					constants->set_updated_constant_buffer(render_buffer_type::render, slots.object, VI_VS | VI_PS);
					device->set_input_layout(layout);
					device->set_shader(shader, VI_VS | VI_PS);
					device->set_texture_2d(buffer->texture, slots.diffuse_map, VI_PS);
					device->set_vertex_buffer(buffer->vertex_buffer);
					device->set_index_buffer(buffer->index_buffer, graphics::format::r32_uint);
					device->draw_indexed((unsigned int)buffer->index_buffer->get_elements(), 0, 0);
				}
				void ReleaseCompiledGeometry(Rml::CompiledGeometryHandle handle) override
				{
					geometry_buffer* resource = (geometry_buffer*)handle;
					core::memory::deinit(resource);
				}
				void EnableScissorRegion(bool enable) override
				{
					VI_ASSERT(device != nullptr, "graphics device should be set");
					has_scissor = enable;
					ortho = trigonometry::matrix4x4::create_orthographic_off_center(0, (float)device->get_render_target()->get_width(), (float)device->get_render_target()->get_height(), 0.0f, -30000.0f, 10000.0f);
					device->set_sampler_state(sampler, slots.sampler, 1, VI_PS);
					device->set_blend_state(alpha_blend);
					if (enable)
					{
						if (has_transform)
						{
							device->set_rasterizer_state(none_rasterizer);
							device->set_depth_stencil_state(less_depth_stencil);
						}
						else
						{
							device->set_rasterizer_state(scissor_none_rasterizer);
							device->set_depth_stencil_state(none_depth_stencil);
						}
					}
					else
					{
						device->set_rasterizer_state(none_rasterizer);
						device->set_depth_stencil_state(none_depth_stencil);
					}
				}
				void SetScissorRegion(int x, int y, int width, int height) override
				{
					VI_ASSERT(device != nullptr, "graphics device should be set");
					if (!has_transform)
					{
						trigonometry::rectangle scissor;
						scissor.left = x;
						scissor.top = y;
						scissor.right = x + width;
						scissor.bottom = y + height;

						return device->set_scissor_rects(1, &scissor);
					}

					graphics::mapped_subresource subresource;
					if (device->map(vertex_buffer, graphics::resource_map::write_discard, &subresource))
					{
						float fWidth = (float)width;
						float fHeight = (float)height;
						float fX = (float)x;
						float fY = (float)y;

						Rml::Vertex vertices[6] =
						{
							{ Rml::Vector2f(fX, fY), Rml::ColourbPremultiplied(), Rml::Vector2f() },
							{ Rml::Vector2f(fX, fY + fHeight), Rml::ColourbPremultiplied(), Rml::Vector2f() },
							{ Rml::Vector2f(fX + fWidth, fY + fHeight), Rml::ColourbPremultiplied(), Rml::Vector2f() },
							{ Rml::Vector2f(fX, fY), Rml::ColourbPremultiplied(), Rml::Vector2f() },
							{ Rml::Vector2f(fX + fWidth, fY + fHeight), Rml::ColourbPremultiplied(), Rml::Vector2f() },
							{ Rml::Vector2f(fX + fWidth, fY), Rml::ColourbPremultiplied(), Rml::Vector2f() }
						};
						memcpy(subresource.pointer, vertices, sizeof(Rml::Vertex) * 6);
						device->unmap(vertex_buffer, &subresource);
					}

					constants->render.transform = transform * ortho;
					constants->update_constant_buffer(render_buffer_type::render);
					device->clear_depth();
					device->set_blend_state(colorless_blend);
					device->set_shader(shader, VI_VS | VI_PS);
					device->set_vertex_buffer(vertex_buffer);
					device->draw((unsigned int)vertex_buffer->get_elements(), 0);
					device->set_depth_stencil_state(scissor_depth_stencil);
					device->set_blend_state(alpha_blend);
				}
				bool LoadTexture(Rml::TextureHandle& handle, Rml::Vector2i& texture_dimensions, const Rml::String& source) override
				{
					VI_ASSERT(content != nullptr, "content manager should be set");
					auto result = content->load<graphics::texture_2d>(source);
					if (!result)
						return false;

					texture_dimensions.x = result->get_width();
					texture_dimensions.y = result->get_height();
					handle = (Rml::TextureHandle)*result;
					return true;
				}
				bool GenerateTexture(Rml::TextureHandle& handle, const Rml::byte* source, const Rml::Vector2i& source_dimensions) override
				{
					VI_ASSERT(device != nullptr, "graphics device should be set");
					VI_ASSERT(source != nullptr, "source should be set");

					graphics::texture_2d::desc f = graphics::texture_2d::desc();
					f.data = (void*)source;
					f.width = (unsigned int)source_dimensions.x;
					f.height = (unsigned int)source_dimensions.y;
					f.row_pitch = device->get_row_pitch(f.width);
					f.depth_pitch = device->get_depth_pitch(f.row_pitch, f.height);
					f.mip_levels = 1;

					graphics::texture_2d* result = *device->create_texture_2d(f);
					handle = (Rml::TextureHandle)result;
					return true;
				}
				void ReleaseTexture(Rml::TextureHandle handle) override
				{
					graphics::texture_2d* resource = (graphics::texture_2d*)handle;
					core::memory::release(resource);
				}
				void SetTransform(const Rml::Matrix4f* new_transform) override
				{
					has_transform = (new_transform != nullptr);
					if (has_transform)
						transform = utils::to_matrix(new_transform);
				}
				void attach(render_constants* new_constants, heavy_content_manager* new_content)
				{
					VI_ASSERT(new_constants != nullptr, "render constants should be set");
					VI_ASSERT(new_content != nullptr, "content manager should be set");
					VI_ASSERT(new_content->get_device() != nullptr, "graphics device should be set");

					detach();
					constants = new_constants;
					content = new_content;
					device = content->get_device();
					constants->add_ref();
					content->add_ref();
					device->add_ref();

					graphics::shader::desc i = graphics::shader::desc();
					if (device->get_section_data("materials/material_ui_element", &i))
					{
						shader = *device->create_shader(i);
						slots.diffuse_map = *device->get_shader_slot(shader, "DiffuseMap");
						slots.sampler = *device->get_shader_sampler_slot(shader, "DiffuseMap", "Sampler");
						slots.object = *device->get_shader_slot(shader, "ObjectBuffer");
					}

					graphics::element_buffer::desc f = graphics::element_buffer::desc();
					f.access_flags = graphics::cpu_access::write;
					f.usage = graphics::resource_usage::dynamic;
					f.bind_flags = graphics::resource_bind::vertex_buffer;
					f.element_count = (unsigned int)6;
					f.elements = (void*)nullptr;
					f.element_width = sizeof(Rml::Vertex);

					vertex_buffer = *device->create_element_buffer(f);
					layout = device->get_input_layout("vx_ui");
					scissor_none_rasterizer = device->get_rasterizer_state("sw_co");
					none_rasterizer = device->get_rasterizer_state("so_co");
					none_depth_stencil = device->get_depth_stencil_state("doo_soo_lt");
					less_depth_stencil = device->get_depth_stencil_state("drw_srw_lt");
					scissor_depth_stencil = device->get_depth_stencil_state("dro_srw_gte");
					alpha_blend = device->get_blend_state("bw_wrgba_source");
					colorless_blend = device->get_blend_state("bo_woooo_one");
					sampler = device->get_sampler_state("a16_fa_wrap");
					subsystem::get()->create_decorators(constants);
				}
				void detach()
				{
					subsystem::get()->release_decorators();
					core::memory::release(vertex_buffer);
					core::memory::release(shader);
					core::memory::release(constants);
					core::memory::release(content);
					core::memory::release(device);
				}
				void resize_buffers(int width, int height)
				{
					subsystem::get()->resize_decorators(width, height);
				}
				heavy_content_manager* get_content()
				{
					return content;
				}
				graphics::graphics_device* get_device()
				{
					return device;
				}
				trigonometry::matrix4x4* get_transform()
				{
					return has_transform ? &transform : nullptr;
				}
				trigonometry::matrix4x4* get_projection()
				{
					return &ortho;
				}
			};

			class file_subsystem final : public Rml::FileInterface
			{
			public:
				file_subsystem() : Rml::FileInterface()
				{
				}
				Rml::FileHandle Open(const Rml::String& path) override
				{
					core::string target = path;
					network::location origin(target);
					if (origin.protocol == "file" && !core::os::file::is_exists(target.c_str()))
					{
						heavy_content_manager* content = (subsystem::get()->get_render_interface() ? subsystem::get()->get_render_interface()->get_content() : nullptr);
						auto subpath = (content ? core::os::path::resolve(path, content->get_environment(), false) : core::os::path::resolve(path.c_str()));
						target = (subpath ? *subpath : path);
					}

					auto stream = core::os::file::open(target, core::file_mode::binary_read_only);
					if (!stream)
						return (Rml::FileHandle)nullptr;

					return (Rml::FileHandle)*stream;
				}
				void Close(Rml::FileHandle file) override
				{
					core::stream* stream = (core::stream*)file;
					core::memory::release(stream);
				}
				size_t Read(void* buffer, size_t size, Rml::FileHandle file) override
				{
					core::stream* stream = (core::stream*)file;
					VI_ASSERT(stream != nullptr, "stream should be set");
					return stream->read((uint8_t*)buffer, size).or_else(0);
				}
				bool Seek(Rml::FileHandle file, long offset, int origin) override
				{
					core::stream* stream = (core::stream*)file;
					VI_ASSERT(stream != nullptr, "stream should be set");
					return !!stream->seek((core::file_seek)origin, offset);
				}
				size_t Tell(Rml::FileHandle file) override
				{
					core::stream* stream = (core::stream*)file;
					VI_ASSERT(stream != nullptr, "stream should be set");
					return stream->tell().or_else(0);
				}
			};

			class main_subsystem final : public Rml::SystemInterface
			{
			private:
				core::unordered_map<core::string, translation_callback> translators;
				core::unordered_map<core::string, core::vector<font_info>> fonts;
				graphics::activity* activity;
				core::timer* time;

			public:
				main_subsystem() : Rml::SystemInterface(), activity(nullptr), time(nullptr)
				{
				}
				~main_subsystem()
				{
					detach();
				}
				void SetMouseCursor(const Rml::String& cursor_name) override
				{
					if (!activity)
						return;

					if (cursor_name == "none")
						activity->set_cursor(graphics::display_cursor::none);
					else if (cursor_name == "default")
						activity->set_cursor(graphics::display_cursor::arrow);
					else if (cursor_name == "move")
						activity->set_cursor(graphics::display_cursor::resize_all);
					else if (cursor_name == "pointer")
						activity->set_cursor(graphics::display_cursor::hand);
					else if (cursor_name == "text")
						activity->set_cursor(graphics::display_cursor::text_input);
					else if (cursor_name == "progress")
						activity->set_cursor(graphics::display_cursor::progress);
					else if (cursor_name == "wait")
						activity->set_cursor(graphics::display_cursor::wait);
					else if (cursor_name == "not-allowed")
						activity->set_cursor(graphics::display_cursor::no);
					else if (cursor_name == "crosshair")
						activity->set_cursor(graphics::display_cursor::crosshair);
					else if (cursor_name == "ns-resize")
						activity->set_cursor(graphics::display_cursor::resize_ns);
					else if (cursor_name == "ew-resize")
						activity->set_cursor(graphics::display_cursor::resize_ew);
					else if (cursor_name == "nesw-resize")
						activity->set_cursor(graphics::display_cursor::resize_nesw);
					else if (cursor_name == "nwse-resize")
						activity->set_cursor(graphics::display_cursor::resize_nwse);
				}
				void SetClipboardText(const Rml::String& text) override
				{
					if (activity != nullptr)
						activity->set_clipboard_text(text);
				}
				void GetClipboardText(Rml::String& text) override
				{
					if (activity != nullptr)
						text = activity->get_clipboard_text();
				}
				void ActivateKeyboard(Rml::Vector2f caret_position, float line_height) override
				{
					if (activity != nullptr)
						activity->set_screen_keyboard(true);
				}
				void DeactivateKeyboard() override
				{
					if (activity != nullptr)
						activity->set_screen_keyboard(false);
				}
				void JoinPath(Rml::String& result, const Rml::String& path1, const Rml::String& path2) override
				{
					heavy_content_manager* content = (subsystem::get()->get_render_interface() ? subsystem::get()->get_render_interface()->get_content() : nullptr);
					if (!content)
						return;

					core::string proto1, proto2;
					core::string fixed1 = get_fixed_path(path1, proto1);
					core::string fixed2 = get_fixed_path(path2, proto2);
					if (proto1 != "file" && proto2 == "file")
					{
						result.assign(path1);
						if (!core::stringify::ends_with(result, '/'))
							result.append(1, '/');

						result.append(fixed2);
						core::stringify::replace(result, "/////", "//");
						core::text_settle idx = core::stringify::find(result, "://");
						core::stringify::replace(result, "//", "/", idx.found ? idx.end : 0);
					}
					else if (proto1 == "file" && proto2 == "file")
					{
						auto path = core::os::path::resolve(fixed2, core::os::path::get_directory(fixed1.c_str()), true);
						if (!path)
							path = core::os::path::resolve(fixed2, content->get_environment(), true);
						if (path)
							result = *path;
					}
					else if (proto1 == "file" && proto2 != "file")
					{
						result.assign(path2);
						core::stringify::replace(result, "/////", "//");
					}
				}
				bool LogMessage(Rml::Log::Type type, const Rml::String& message) override
				{
					switch (type)
					{
						case Rml::Log::LT_ERROR:
							VI_ERR("[gui] %.*s", (int)message.size(), message.c_str());
							break;
						case Rml::Log::LT_WARNING:
							VI_WARN("[gui] %.*s", (int)message.size(), message.c_str());
							break;
						case Rml::Log::LT_INFO:
							VI_DEBUG("[gui] %.*s", (int)message.size(), message.c_str());
							break;
						case Rml::Log::LT_ASSERT:
							VI_TRACE("[gui] %.*s", (int)message.size(), message.c_str());
							break;
						default:
							break;
					}

					return true;
				}
				int TranslateString(Rml::String& result, const Rml::String& key_name) override
				{
					for (auto& item : translators)
					{
						if (!item.second)
							continue;

						result = item.second(key_name);
						if (!result.empty())
							return 1;
					}

					result = key_name;
					return 0;
				}
				double GetElapsedTime() override
				{
					if (!time)
						return 0.0;

					return (double)time->get_elapsed();
				}
				void attach(graphics::activity* new_activity, core::timer* new_time)
				{
					detach();
					activity = new_activity;
					time = new_time;
					if (activity != nullptr)
						activity->add_ref();
					if (time != nullptr)
						time->add_ref();
				}
				void detach()
				{
					core::memory::release(activity);
					core::memory::release(time);
				}
				void set_translator(const std::string_view& name, translation_callback&& callback)
				{
					auto it = translators.find(core::key_lookup_cast(name));
					if (it == translators.end())
						translators.insert(std::make_pair(core::string(name), std::move(callback)));
					else
						it->second = std::move(callback);
				}
				bool add_font_face(const std::string_view& path, bool use_as_fallback, font_weight weight)
				{
					auto it = fonts.find(core::key_lookup_cast(path));
					if (it != fonts.end())
					{
						for (auto& info : it->second)
						{
							if (info.fallback == use_as_fallback && info.weight == weight)
								return true;
						}
					}

					core::string target_path = core::string(path);
					if (!Rml::LoadFontFace(target_path, use_as_fallback, (Rml::Style::FontWeight)weight))
						return false;

					font_info info;
					info.fallback = use_as_fallback;
					info.weight = weight;
					if (it != fonts.end())
						it->second.push_back(info);
					else
						fonts[target_path].push_back(info);
					return true;
				}
				core::unordered_map<core::string, core::vector<font_info>>* get_font_faces()
				{
					return &fonts;
				}
				core::string get_fixed_path(const std::string_view& location, core::string& proto)
				{
					if (!core::stringify::find(location, "://").found)
					{
						proto = "file";
						return core::string(location);
					}

					Rml::URL target = Rml::URL(core::string(location));
					proto = target.GetProtocol();
					return target.GetPathedFileName();
				}
			};

			class scoped_context final : public Rml::Context
			{
			public:
				gui::context* basis;

			public:
				scoped_context(const std::string_view& name, Rml::RenderManager* render_manager, Rml::TextInputHandler* text_input_handler) : Rml::Context(Rml::String(name), render_manager, text_input_handler), basis(nullptr)
				{
				}
			};

			class context_instancer final : public Rml::ContextInstancer
			{
			public:
				Rml::ContextPtr InstanceContext(const Rml::String& name, Rml::RenderManager* render_manager, Rml::TextInputHandler* text_input_handler) override
				{
					return Rml::ContextPtr(core::memory::init<scoped_context>(name, render_manager, text_input_handler));
				}
				void ReleaseContext(Rml::Context* context) override
				{
					scoped_context* item = (scoped_context*)context;
					core::memory::deinit(item);
				}
				void Release() override
				{
					core::memory::deinit(this);
				}
			};

			class document_subsystem final : public Rml::ElementDocument
			{
			public:
				document_subsystem(const std::string_view& tag) : Rml::ElementDocument(Rml::String(tag))
				{
				}
				void LoadInlineScript(const Rml::String& content, const Rml::String& path, int line) override
				{
					scoped_context* scope = (scoped_context*)GetContext();
					VI_ASSERT(scope && scope->basis && scope->basis->compiler, "context should be scoped");

					scripting::compiler* compiler = scope->basis->compiler;
					if (!compiler->load_code(path + ":" + core::to_string(line), content))
						return;

					compiler->compile().when([scope, compiler](scripting::expects_vm<void>&& status)
					{
						if (!status)
							return;

						scripting::function main = compiler->get_module().get_function_by_decl("void main()");
						if (!main.is_valid())
							main = compiler->get_module().get_function_by_decl("void main(ui_context@+)");

						scripting::function_delegate delegatef(main);
						if (!delegatef.is_valid())
							return;

						bool has_arguments = main.get_args_count() > 0;
						delegatef([has_arguments, scope](scripting::immediate_context* context)
						{
							scope->basis->add_ref();
							if (has_arguments)
								context->set_arg_object(0, scope->basis);
						}, [scope](scripting::immediate_context*)
						{
							scope->basis->release();
						});
					});
				}
				void LoadExternalScript(const Rml::String& path) override
				{
					scoped_context* scope = (scoped_context*)GetContext();
					VI_ASSERT(scope && scope->basis && scope->basis->compiler, "context should be scoped");

					core::string where = path;
					core::stringify::replace(where, '|', ':');

					scripting::compiler* compiler = scope->basis->compiler;
					if (!compiler->load_file(where))
						return;

					compiler->compile().when([scope, compiler](scripting::expects_vm<void>&& status)
					{
						if (!status)
							return;

						scripting::function main = compiler->get_module().get_function_by_decl("void main()");
						if (!main.is_valid())
							main = compiler->get_module().get_function_by_decl("void main(ui_context@+)");

						scripting::function_delegate delegatef(main);
						if (!delegatef.is_valid())
							return;

						bool has_arguments = main.get_args_count() > 0;
						delegatef([has_arguments, scope](scripting::immediate_context* context)
						{
							scope->basis->add_ref();
							if (has_arguments)
								context->set_arg_object(0, scope->basis);
						}, [scope](scripting::immediate_context*)
						{
							scope->basis->release();
						});
					});
				}
			};

			class document_instancer final : public Rml::ElementInstancer
			{
			public:
				Rml::ElementPtr InstanceElement(Rml::Element*, const Rml::String& tag, const Rml::XMLAttributes&) override
				{
					return Rml::ElementPtr(core::memory::init<document_subsystem>(tag));
				}
				void ReleaseElement(Rml::Element* element) override
				{
					core::memory::deinit(element);
				}
			};

			class listener_subsystem final : public Rml::EventListener
			{
			public:
				scripting::function_delegate delegatef;
				core::string memory;
				ievent event_context;

			public:
				listener_subsystem(const std::string_view& code, Rml::Element* element) : memory(code)
				{
				}
				~listener_subsystem() = default;
				void OnDetach(Rml::Element* element) override
				{
					core::memory::deinit(this);
				}
				void ProcessEvent(Rml::Event& event) override
				{
					scoped_context* scope = (scoped_context*)event.GetCurrentElement()->GetContext();
					VI_ASSERT(scope && scope->basis && scope->basis->compiler, "context should be scoped");
					if (!compile_inline(scope))
						return;

					Rml::Event* ptr = Rml::Factory::InstanceEvent(event.GetTargetElement(), event.GetId(), event.GetType(), event.GetParameters(), event.IsInterruptible()).release();
					if (ptr != nullptr)
					{
						ptr->SetCurrentElement(event.GetCurrentElement());
						ptr->SetPhase(event.GetPhase());
					}

					event_context = ievent(ptr);
					scope->basis->add_ref();
					delegatef([this](scripting::immediate_context* context)
					{
						context->set_arg_object(0, &event_context);
					}, [this, scope, ptr](scripting::immediate_context*)
					{
						scope->basis->release();
						event_context = ievent();
						delete ptr;
					});
				}
				bool compile_inline(scoped_context* scope)
				{
					if (delegatef.is_valid())
						return true;

					auto hash = compute::crypto::hash_hex(compute::digests::MD5(), memory);
					if (!hash)
						return false;

					core::string name = "__vf" + *hash;
					core::string eval = "void " + name + "(ui_event&in event){\n";
					eval.append(memory);
					eval += "\n;}";

					scripting::function function = nullptr;
					scripting::library library = scope->basis->compiler->get_module();
					if (!library.compile_function(name.c_str(), eval.c_str(), -1, (size_t)scripting::compile_flags::add_to_module, &function))
						return false;

					delegatef = function;
					return delegatef.is_valid();
				}
			};

			class listener_instancer final : public Rml::EventListenerInstancer
			{
			public:
				Rml::EventListener* InstanceEventListener(const Rml::String& value, Rml::Element* element) override
				{
					return core::memory::init<listener_subsystem>(value, element);
				}
			};

			class event_subsystem final : public Rml::EventListener
			{
				friend ievent;

			private:
				event_callback listener;
				std::atomic<int> ref_count;

			public:
				event_subsystem(event_callback&& callback) : Rml::EventListener(), listener(std::move(callback)), ref_count(1)
				{
				}
				void OnAttach(Rml::Element*) override
				{
					ref_count++;
				}
				void OnDetach(Rml::Element*) override
				{
					if (!--ref_count)
						core::memory::deinit(this);
				}
				void ProcessEvent(Rml::Event& event) override
				{
					if (!listener)
						return;

					ievent basis(&event);
					listener(basis);
				}
			};
#endif
			gui_exception::gui_exception(core::string&& new_message)
			{
				error_message = std::move(new_message);
			}
			const char* gui_exception::type() const noexcept
			{
				return "gui_error";
			}

			void ivariant::convert(Rml::Variant* from, core::variant* to)
			{
#ifdef VI_RMLUI
				VI_ASSERT(from && to, "from and to should be set");
				switch (from->GetType())
				{
					case Rml::Variant::BOOL:
						*to = core::var::boolean(from->Get<bool>());
						break;
					case Rml::Variant::FLOAT:
					case Rml::Variant::DOUBLE:
						*to = core::var::number(from->Get<double>());
						break;
					case Rml::Variant::BYTE:
					case Rml::Variant::CHAR:
					case Rml::Variant::INT:
					case Rml::Variant::INT64:
						*to = core::var::integer(from->Get<int64_t>());
						break;
					case Rml::Variant::VECTOR2:
					{
						Rml::Vector2f t = from->Get<Rml::Vector2f>();
						*to = core::var::string(from_vector2(trigonometry::vector2(t.x, t.y)));
						break;
					}
					case Rml::Variant::VECTOR3:
					{
						Rml::Vector3f t = from->Get<Rml::Vector3f>();
						*to = core::var::string(from_vector3(trigonometry::vector3(t.x, t.y, t.z)));
						break;
					}
					case Rml::Variant::VECTOR4:
					{
						Rml::Vector4f t = from->Get<Rml::Vector4f>();
						*to = core::var::string(from_vector4(trigonometry::vector4(t.x, t.y, t.z, t.w)));
						break;
					}
					case Rml::Variant::STRING:
					case Rml::Variant::COLOURF:
					case Rml::Variant::COLOURB:
						*to = core::var::string(from->Get<core::string>());
						break;
					case Rml::Variant::VOIDPTR:
						*to = core::var::pointer(from->Get<void*>());
						break;
					default:
						*to = core::var::undefined();
						break;
				}
#endif
			}
			void ivariant::revert(core::variant* from, Rml::Variant* to)
			{
#ifdef VI_RMLUI
				VI_ASSERT(from && to, "from and to should be set");
				switch (from->get_type())
				{
					case core::var_type::null:
						*to = Rml::Variant((void*)nullptr);
						break;
					case core::var_type::string:
					{
						core::string blob = from->get_blob();
						int type = ivariant::get_vector_type(blob);
						if (type == 2)
						{
							trigonometry::vector2 t = ivariant::to_vector2(blob);
							*to = Rml::Variant(Rml::Vector2f(t.x, t.y));
						}
						else if (type == 3)
						{
							trigonometry::vector3 t = ivariant::to_vector3(blob);
							*to = Rml::Variant(Rml::Vector3f(t.x, t.y, t.z));
						}
						else if (type == 4)
						{
							trigonometry::vector4 t = ivariant::to_vector4(blob);
							*to = Rml::Variant(Rml::Vector4f(t.x, t.y, t.z, t.w));
						}
						else
							*to = Rml::Variant(from->get_blob());
						break;
					}
					case core::var_type::integer:
						*to = Rml::Variant(from->get_integer());
						break;
					case core::var_type::number:
						*to = Rml::Variant(from->get_number());
						break;
					case core::var_type::boolean:
						*to = Rml::Variant(from->get_boolean());
						break;
					case core::var_type::pointer:
						*to = Rml::Variant(from->get_pointer());
						break;
					default:
						to->Clear();
						break;
				}
#endif
			}
			trigonometry::vector4 ivariant::to_color4(const std::string_view& value)
			{
				VI_ASSERT(core::stringify::is_cstring(value), "value should be set");
				if (value.empty())
					return 0.0f;

				trigonometry::vector4 result;
				if (value[0] == '#')
				{
					if (value.size() == 4)
					{
						char buffer[7];
						buffer[0] = value[1];
						buffer[1] = value[1];
						buffer[2] = value[2];
						buffer[3] = value[2];
						buffer[4] = value[3];
						buffer[5] = value[3];
						buffer[6] = '\0';

						unsigned int r = 0, g = 0, b = 0;
						if (sscanf(buffer, "%02x%02x%02x", &r, &g, &b) == 3)
						{
							result.x = r / 255.0f;
							result.y = g / 255.0f;
							result.z = b / 255.0f;
							result.w = 1.0f;
						}
					}
					else
					{
						unsigned int r = 0, g = 0, b = 0, a = 255;
						if (sscanf(value.data(), "#%02x%02x%02x%02x", &r, &g, &b, &a) == 4)
						{
							result.x = r / 255.0f;
							result.y = g / 255.0f;
							result.z = b / 255.0f;
							result.w = a / 255.0f;
						}
					}
				}
				else
				{
					unsigned int r = 0, g = 0, b = 0, a = 255;
					if (sscanf(value.data(), "%u %u %u %u", &r, &g, &b, &a) == 4)
					{
						result.x = r / 255.0f;
						result.y = g / 255.0f;
						result.z = b / 255.0f;
						result.w = a / 255.0f;
					}
				}

				return result;
			}
			core::string ivariant::from_color4(const trigonometry::vector4& base, bool HEX)
			{
				if (!HEX)
					return core::stringify::text("%d %d %d %d", (unsigned int)(base.x * 255.0f), (unsigned int)(base.y * 255.0f), (unsigned int)(base.z * 255.0f), (unsigned int)(base.w * 255.0f));

				return core::stringify::text("#%02x%02x%02x%02x",
					(unsigned int)(base.x * 255.0f),
					(unsigned int)(base.y * 255.0f),
					(unsigned int)(base.z * 255.0f),
					(unsigned int)(base.w * 255.0f));
			}
			trigonometry::vector4 ivariant::to_color3(const std::string_view& value)
			{
				VI_ASSERT(core::stringify::is_cstring(value), "value should be set");
				if (value.empty())
					return 0.0f;

				trigonometry::vector4 result;
				if (value[0] == '#')
				{
					unsigned int r = 0, g = 0, b = 0;
					int fills = 0;

					if (value.size() == 4)
					{
						char buffer[7];
						buffer[0] = value[1];
						buffer[1] = value[1];
						buffer[2] = value[2];
						buffer[3] = value[2];
						buffer[4] = value[3];
						buffer[5] = value[3];
						buffer[6] = '\0';
						fills = sscanf(buffer, "%02x%02x%02x", &r, &g, &b);
					}
					else
						fills = sscanf(value.data(), "#%02x%02x%02x", &r, &g, &b);

					if (fills == 3)
					{
						result.x = r / 255.0f;
						result.y = g / 255.0f;
						result.z = b / 255.0f;
					}
				}
				else
				{
					unsigned int r = 0, g = 0, b = 0;
					if (sscanf(value.data(), "%u %u %u", &r, &g, &b) == 3)
					{
						result.x = r / 255.0f;
						result.y = g / 255.0f;
						result.z = b / 255.0f;
					}
				}

				return result;
			}
			core::string ivariant::from_color3(const trigonometry::vector4& base, bool HEX)
			{
				if (!HEX)
					return core::stringify::text("%d %d %d", (unsigned int)(base.x * 255.0f), (unsigned int)(base.y * 255.0f), (unsigned int)(base.z * 255.0f));

				return core::stringify::text("#%02x%02x%02x",
					(unsigned int)(base.x * 255.0f),
					(unsigned int)(base.y * 255.0f),
					(unsigned int)(base.z * 255.0f));
			}
			int ivariant::get_vector_type(const std::string_view& value)
			{
				if (value.size() < 2 || value[0] != 'v')
					return -1;

				if (value[1] == '2')
					return 2;

				if (value[1] == '3')
					return 3;

				if (value[1] == '4')
					return 4;

				return -1;
			}
			trigonometry::vector4 ivariant::to_vector4(const std::string_view& base)
			{
				VI_ASSERT(core::stringify::is_cstring(base), "value should be set");
				trigonometry::vector4 result;
				if (sscanf(base.data(), "v4 %f %f %f %f", &result.x, &result.y, &result.z, &result.w) != 4)
					return result;

				return result;
			}
			core::string ivariant::from_vector4(const trigonometry::vector4& base)
			{
				return core::stringify::text("v4 %f %f %f %f", base.x, base.y, base.z, base.w);
			}
			trigonometry::vector3 ivariant::to_vector3(const std::string_view& base)
			{
				VI_ASSERT(core::stringify::is_cstring(base), "value should be set");
				trigonometry::vector3 result;
				if (sscanf(base.data(), "v3 %f %f %f", &result.x, &result.y, &result.z) != 3)
					return result;

				return result;
			}
			core::string ivariant::from_vector3(const trigonometry::vector3& base)
			{
				return core::stringify::text("v3 %f %f %f", base.x, base.y, base.z);
			}
			trigonometry::vector2 ivariant::to_vector2(const std::string_view& base)
			{
				VI_ASSERT(core::stringify::is_cstring(base), "value should be set");
				trigonometry::vector2 result;
				if (sscanf(base.data(), "v2 %f %f", &result.x, &result.y) != 2)
					return result;

				return result;
			}
			core::string ivariant::from_vector2(const trigonometry::vector2& base)
			{
				return core::stringify::text("v2 %f %f", base.x, base.y);
			}

			ievent::ievent() : base(nullptr), owned(false)
			{
			}
			ievent::ievent(Rml::Event* ref) : base(ref), owned(false)
			{
			}
			ievent ievent::copy()
			{
#ifdef VI_RMLUI
				Rml::Event* ptr = Rml::Factory::InstanceEvent(base->GetTargetElement(), base->GetId(), base->GetType(), base->GetParameters(), base->IsInterruptible()).release();
				if (ptr != nullptr)
				{
					ptr->SetCurrentElement(base->GetCurrentElement());
					ptr->SetPhase(base->GetPhase());
				}

				ievent result(ptr);
				result.owned = true;
				return result;
#else
				return *this;
#endif
			}
			void ievent::release()
			{
#ifdef VI_RMLUI
				if (owned)
				{
					delete base;
					base = nullptr;
					owned = false;
				}
#endif
			}
			event_phase ievent::get_phase() const
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "event should be valid");
				return (event_phase)base->GetPhase();
#else
				return event_phase::none;
#endif
			}
			void ievent::set_phase(event_phase phase)
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "event should be valid");
				base->SetPhase((Rml::EventPhase)phase);
#endif
			}
			void ievent::set_current_element(const ielement& element)
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "event should be valid");
				base->SetCurrentElement(element.get_element());
#endif
			}
			ielement ievent::get_current_element() const
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "event should be valid");
				return base->GetCurrentElement();
#else
				return ielement();
#endif
			}
			ielement ievent::get_target_element() const
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "event should be valid");
				return base->GetTargetElement();
#else
				return ielement();
#endif
			}
			core::string ievent::get_type() const
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "event should be valid");
				return base->GetType();
#else
				return core::string();
#endif
			}
			void ievent::stop_propagation()
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "event should be valid");
				base->StopPropagation();
#endif
			}
			void ievent::stop_immediate_propagation()
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "event should be valid");
				base->StopImmediatePropagation();
#endif
			}
			bool ievent::is_interruptible() const
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "event should be valid");
				return base->IsInterruptible();
#else
				return false;
#endif
			}
			bool ievent::is_propagating() const
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "event should be valid");
				return base->IsPropagating();
#else
				return false;
#endif
			}
			bool ievent::is_immediate_propagating() const
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "event should be valid");
				return base->IsImmediatePropagating();
#else
				return false;
#endif
			}
			bool ievent::get_boolean(const std::string_view& key) const
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "event should be valid");
				return base->GetParameter<bool>(core::string(key), false);
#else
				return false;
#endif
			}
			int64_t ievent::get_integer(const std::string_view& key) const
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "event should be valid");
				return base->GetParameter<int64_t>(core::string(key), 0);
#else
				return 0;
#endif
			}
			double ievent::get_number(const std::string_view& key) const
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "event should be valid");
				return base->GetParameter<double>(core::string(key), 0.0);
#else
				return 0.0;
#endif
			}
			core::string ievent::get_string(const std::string_view& key) const
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "event should be valid");
				return base->GetParameter<Rml::String>(core::string(key), "");
#else
				return core::string();
#endif
			}
			trigonometry::vector2 ievent::get_vector2(const std::string_view& key) const
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "event should be valid");
				Rml::Vector2f result = base->GetParameter<Rml::Vector2f>(core::string(key), Rml::Vector2f());
				return trigonometry::vector2(result.x, result.y);
#else
				return trigonometry::vector2();
#endif
			}
			trigonometry::vector3 ievent::get_vector3(const std::string_view& key) const
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "event should be valid");
				Rml::Vector3f result = base->GetParameter<Rml::Vector3f>(core::string(key), Rml::Vector3f());
				return trigonometry::vector3(result.x, result.y, result.z);
#else
				return trigonometry::vector3();
#endif
			}
			trigonometry::vector4 ievent::get_vector4(const std::string_view& key) const
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "event should be valid");
				Rml::Vector4f result = base->GetParameter<Rml::Vector4f>(core::string(key), Rml::Vector4f());
				return trigonometry::vector4(result.x, result.y, result.z, result.w);
#else
				return trigonometry::vector4();
#endif
			}
			void* ievent::get_pointer(const std::string_view& key) const
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "event should be valid");
				return base->GetParameter<void*>(core::string(key), nullptr);
#else
				return nullptr;
#endif
			}
			Rml::Event* ievent::get_event() const
			{
				return base;
			}
			bool ievent::is_valid() const
			{
				return base != nullptr;
			}

			ielement::ielement() : base(nullptr)
			{
			}
			ielement::ielement(Rml::Element* ref) : base(ref)
			{
			}
			void ielement::release()
			{
#ifdef VI_RMLUI
				core::memory::deinit(base);
				base = nullptr;
#endif
			}
			ielement ielement::clone() const
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				Rml::ElementPtr ptr = base->Clone();
				Rml::Element* result = ptr.get();
				ptr.reset();

				return result;
#else
				return ielement();
#endif
			}
			void ielement::set_class(const std::string_view& class_name, bool activate)
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				base->SetClass(core::string(class_name), activate);
#endif
			}
			bool ielement::is_class_set(const std::string_view& class_name) const
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				return base->IsClassSet(core::string(class_name));
#else
				return false;
#endif
			}
			void ielement::set_class_names(const std::string_view& class_names)
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				base->SetClassNames(core::string(class_names));
#endif
			}
			core::string ielement::get_class_names() const
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				return base->GetClassNames();
#else
				return core::string();
#endif

			}
			core::string ielement::get_address(bool include_pseudo_classes, bool include_parents) const
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				return base->GetAddress(include_pseudo_classes, include_parents);
#else
				return core::string();
#endif

			}
			void ielement::set_offset(const trigonometry::vector2& offset, const ielement& offset_parent, bool offset_fixed)
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				base->SetOffset(Rml::Vector2f(offset.x, offset.y), offset_parent.get_element(), offset_fixed);
#endif
			}
			trigonometry::vector2 ielement::get_relative_offset(area type)
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				Rml::Vector2f result = base->GetRelativeOffset((Rml::BoxArea)type);
				return trigonometry::vector2(result.x, result.y);
#else
				return trigonometry::vector2();
#endif

			}
			trigonometry::vector2 ielement::get_absolute_offset(area type)
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				Rml::Vector2f result = base->GetAbsoluteOffset((Rml::BoxArea)type);
				return trigonometry::vector2(result.x, result.y);
#else
				return trigonometry::vector2();
#endif

			}
			void ielement::set_content_box(const trigonometry::vector2& content_box)
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				base->SetBox(Rml::Box(Rml::Vector2f(content_box.x, content_box.y)));
#endif
			}
			float ielement::get_baseline() const
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				return base->GetBaseline();
#else
				return 0.0f;
#endif

			}
			bool ielement::get_intrinsic_dimensions(trigonometry::vector2& dimensions, float& ratio)
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				Rml::Vector2f size;
				bool result = base->GetIntrinsicDimensions(size, ratio);
				dimensions = trigonometry::vector2(size.x, size.y);

				return result;
#else
				return false;
#endif

			}
			bool ielement::is_point_within_element(const trigonometry::vector2& point)
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				return base->IsPointWithinElement(Rml::Vector2f(point.x, point.y));
#else
				return false;
#endif

			}
			bool ielement::is_visible() const
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				return base->IsVisible();
#else
				return false;
#endif

			}
			float ielement::get_zindex() const
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				return base->GetZIndex();
#else
				return 0.0f;
#endif

			}
			bool ielement::set_property(const std::string_view& name, const std::string_view& value)
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				return base->SetProperty(core::string(name), core::string(value));
#else
				return false;
#endif

			}
			void ielement::remove_property(const std::string_view& name)
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				base->RemoveProperty(core::string(name));
#endif
			}
			core::string ielement::get_property(const std::string_view& name)
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				const Rml::Property* result = base->GetProperty(core::string(name));
				if (!result)
					return "";

				return result->ToString();
#else
				return core::string();
#endif
			}
			core::string ielement::get_local_property(const std::string_view& name)
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				const Rml::Property* result = base->GetLocalProperty(core::string(name));
				if (!result)
					return "";

				return result->ToString();
#else
				return core::string();
#endif
			}
			float ielement::resolve_numeric_property(float value, numeric_unit unit, float base_value)
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				return base->ResolveNumericValue(Rml::NumericValue(value, (Rml::Unit)unit), base_value);
#else
				return 0.0f;
#endif
			}
			trigonometry::vector2 ielement::get_containing_block()
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				Rml::Vector2f result = base->GetContainingBlock();
				return trigonometry::vector2(result.x, result.y);
#else
				return trigonometry::vector2();
#endif
			}
			position ielement::get_position()
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				return (position)base->GetPosition();
#else
				return position::constant;
#endif
			}
			floatf ielement::get_float()
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				return (floatf)base->GetFloat();
#else
				return floatf::none;
#endif
			}
			display ielement::get_display()
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				return (display)base->GetDisplay();
#else
				return display::none;
#endif
			}
			float ielement::get_line_height()
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				return base->GetLineHeight();
#else
				return 0.0f;
#endif
			}
			bool ielement::project(trigonometry::vector2& point) const noexcept
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				Rml::Vector2f offset(point.x, point.y);
				bool result = base->Project(offset);
				point = trigonometry::vector2(offset.x, offset.y);

				return result;
#else
				return false;
#endif
			}
			bool ielement::animate(const std::string_view& property_name, const std::string_view& target_value, float duration, timing_func func, timing_dir dir, int num_iterations, bool alternate_direction, float delay)
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				return base->Animate(core::string(property_name), Rml::Property(core::string(target_value), Rml::Unit::TRANSFORM), duration, Rml::Tween((Rml::Tween::Type)func, (Rml::Tween::Direction)dir), num_iterations, alternate_direction, delay);
#else
				return false;
#endif
			}
			bool ielement::add_animation_key(const std::string_view& property_name, const std::string_view& target_value, float duration, timing_func func, timing_dir dir)
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				return base->AddAnimationKey(core::string(property_name), Rml::Property(core::string(target_value), Rml::Unit::TRANSFORM), duration, Rml::Tween((Rml::Tween::Type)func, (Rml::Tween::Direction)dir));
#else
				return false;
#endif
			}
			void ielement::set_pseudo_class(const std::string_view& pseudo_class, bool activate)
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				base->SetPseudoClass(core::string(pseudo_class), activate);
#endif
			}
			bool ielement::is_pseudo_class_set(const std::string_view& pseudo_class) const
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				return base->IsPseudoClassSet(core::string(pseudo_class));
#else
				return false;
#endif
			}
			void ielement::set_attribute(const std::string_view& name, const std::string_view& value)
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				base->SetAttribute(core::string(name), core::string(value));
#endif
			}
			core::string ielement::get_attribute(const std::string_view& name)
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				return base->GetAttribute<core::string>(core::string(name), "");
#else
				return core::string();
#endif
			}
			bool ielement::has_attribute(const std::string_view& name) const
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				return base->HasAttribute(core::string(name));
#else
				return false;
#endif
			}
			void ielement::remove_attribute(const std::string_view& name)
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				base->RemoveAttribute(core::string(name));
#endif
			}
			ielement ielement::get_focus_leaf_node()
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				return base->GetFocusLeafNode();
#else
				return ielement();
#endif
			}
			core::string ielement::get_tag_name() const
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				return base->GetTagName();
#else
				return core::string();
#endif
			}
			core::string ielement::get_id() const
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				return base->GetId();
#else
				return core::string();
#endif
			}
			void ielement::set_id(const std::string_view& id)
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				base->SetId(core::string(id));
#endif
			}
			float ielement::get_absolute_left()
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				return base->GetAbsoluteLeft();
#else
				return 0.0f;
#endif
			}
			float ielement::get_absolute_top()
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				return base->GetAbsoluteTop();
#else
				return 0.0f;
#endif
			}
			float ielement::get_client_left()
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				return base->GetClientLeft();
#else
				return 0.0f;
#endif
			}
			float ielement::get_client_top()
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				return base->GetClientTop();
#else
				return 0.0f;
#endif
			}
			float ielement::get_client_width()
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				return base->GetClientWidth();
#else
				return 0.0f;
#endif
			}
			float ielement::get_client_height()
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				return base->GetClientHeight();
#else
				return 0.0f;
#endif
			}
			ielement ielement::get_offset_parent()
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				return base->GetOffsetParent();
#else
				return ielement();
#endif
			}
			float ielement::get_offset_left()
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				return base->GetOffsetLeft();
#else
				return 0.0f;
#endif
			}
			float ielement::get_offset_top()
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				return base->GetOffsetTop();
#else
				return 0.0f;
#endif
			}
			float ielement::get_offset_width()
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				return base->GetOffsetWidth();
#else
				return 0.0f;
#endif
			}
			float ielement::get_offset_height()
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				return base->GetOffsetHeight();
#else
				return 0.0f;
#endif
			}
			float ielement::get_scroll_left()
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				return base->GetScrollLeft();
#else
				return 0.0f;
#endif
			}
			void ielement::set_scroll_left(float scroll_left)
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				base->SetScrollLeft(scroll_left);
#endif
			}
			float ielement::get_scroll_top()
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				return base->GetScrollTop();
#else
				return 0.0f;
#endif
			}
			void ielement::set_scroll_top(float scroll_top)
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				base->SetScrollTop(scroll_top);
#endif
			}
			float ielement::get_scroll_width()
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				return base->GetScrollWidth();
#else
				return 0.0f;
#endif
			}
			float ielement::get_scroll_height()
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				return base->GetScrollHeight();
#else
				return 0.0f;
#endif
			}
			ielement_document ielement::get_owner_document() const
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				return base->GetOwnerDocument();
#else
				return ielement_document();
#endif
			}
			ielement ielement::get_parent_node() const
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				return base->GetParentNode();
#else
				return ielement();
#endif
			}
			ielement ielement::get_next_sibling() const
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				return base->GetNextSibling();
#else
				return ielement();
#endif
			}
			ielement ielement::get_previous_sibling() const
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				return base->GetPreviousSibling();
#else
				return ielement();
#endif
			}
			ielement ielement::get_first_child() const
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				return base->GetFirstChild();
#else
				return ielement();
#endif
			}
			ielement ielement::get_last_child() const
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				return base->GetLastChild();
#else
				return ielement();
#endif
			}
			ielement ielement::get_child(int index) const
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				return base->GetChild(index);
#else
				return ielement();
#endif
			}
			int ielement::get_num_children(bool include_non_dom_elements) const
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				return base->GetNumChildren(include_non_dom_elements);
#else
				return 0;
#endif
			}
			void ielement::get_inner_html(core::string& content) const
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				base->GetInnerRML(content);
#endif
			}
			core::string ielement::get_inner_html() const
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				return base->GetInnerRML();
#else
				return core::string();
#endif
			}
			void ielement::set_inner_html(const std::string_view& HTML)
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				base->SetInnerRML(core::string(HTML));
#endif
			}
			bool ielement::is_focused()
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				return base->IsPseudoClassSet("focus");
#else
				return false;
#endif
			}
			bool ielement::is_hovered()
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				return base->IsPseudoClassSet("hover");
#else
				return false;
#endif
			}
			bool ielement::is_active()
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				return base->IsPseudoClassSet("active");
#else
				return false;
#endif
			}
			bool ielement::is_checked()
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				return base->IsPseudoClassSet("checked");
#else
				return false;
#endif
			}
			bool ielement::focus()
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				return base->Focus();
#else
				return false;
#endif
			}
			void ielement::blur()
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				base->Blur();
#endif
			}
			void ielement::click()
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				base->Click();
#endif
			}
			void ielement::add_event_listener(const std::string_view& event, listener* source, bool in_capture_phase)
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				VI_ASSERT(source != nullptr && source->base != nullptr, "listener should be set");
				base->AddEventListener(core::string(event), source->base, in_capture_phase);
#endif
			}
			void ielement::remove_event_listener(const std::string_view& event, listener* source, bool in_capture_phase)
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				VI_ASSERT(source != nullptr && source->base != nullptr, "listener should be set");
				base->RemoveEventListener(core::string(event), source->base, in_capture_phase);
#endif
			}
			bool ielement::dispatch_event(const std::string_view& type, const core::variant_args& args)
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				Rml::Dictionary props;
				for (auto& item : args)
				{
					Rml::Variant& prop = props[item.first];
					ivariant::revert((core::variant*)&item.second, &prop);
				}

				return base->DispatchEvent(core::string(type), props);
#else
				return false;
#endif
			}
			void ielement::scroll_into_view(bool align_with_top)
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				base->ScrollIntoView(align_with_top);
#endif
			}
			ielement ielement::append_child(const ielement& element, bool dom_element)
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				return base->AppendChild(Rml::ElementPtr(element.get_element()), dom_element);
#else
				return ielement();
#endif
			}
			ielement ielement::insert_before(const ielement& element, const ielement& adjacent_element)
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				return base->InsertBefore(Rml::ElementPtr(element.get_element()), adjacent_element.get_element());
#else
				return ielement();
#endif
			}
			ielement ielement::replace_child(const ielement& inserted_element, const ielement& replaced_element)
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				Rml::ElementPtr ptr = base->ReplaceChild(Rml::ElementPtr(inserted_element.get_element()), replaced_element.get_element());
				Rml::Element* result = ptr.get();
				ptr.reset();

				return result;
#else
				return ielement();
#endif
			}
			ielement ielement::remove_child(const ielement& element)
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				Rml::ElementPtr ptr = base->RemoveChild(element.get_element());
				Rml::Element* result = ptr.get();
				ptr.reset();

				return result;
#else
				return ielement();
#endif
			}
			bool ielement::has_child_nodes() const
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				return base->HasChildNodes();
#else
				return false;
#endif
			}
			ielement ielement::get_element_by_id(const std::string_view& id)
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				return base->GetElementById(core::string(id));
#else
				return ielement();
#endif
			}
			ielement ielement::query_selector(const std::string_view& selector)
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				return base->QuerySelector(core::string(selector));
#else
				return ielement();
#endif
			}
			core::vector<ielement> ielement::query_selector_all(const std::string_view& selectors)
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				Rml::ElementList elements;
				base->QuerySelectorAll(elements, core::string(selectors));

				core::vector<ielement> result;
				result.reserve(elements.size());

				for (auto& item : elements)
					result.push_back(item);

				return result;
#else
				return core::vector<ielement>();
#endif
			}
			bool ielement::cast_form_color(trigonometry::vector4* ptr, bool alpha)
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				VI_ASSERT(ptr != nullptr, "ptr should be set");

				Rml::ElementFormControl* form = (Rml::ElementFormControl*)base;
				core::string value = form->GetValue();
				trigonometry::vector4 color = (alpha ? ivariant::to_color4(value) : ivariant::to_color3(value));

				if (alpha)
				{
					if (value[0] == '#')
					{
						if (value.size() > 9)
							form->SetValue(value.substr(0, 9));
					}
					else if (value.size() > 15)
						form->SetValue(value.substr(0, 15));
				}
				else
				{
					if (value[0] == '#')
					{
						if (value.size() > 7)
							form->SetValue(value.substr(0, 7));
					}
					else if (value.size() > 11)
						form->SetValue(value.substr(0, 11));
				}

				if (color == *ptr)
				{
					if (!value.empty() || form->IsPseudoClassSet("focus"))
						return false;

					if (alpha)
						form->SetValue(ivariant::from_color4(*ptr, true));
					else
						form->SetValue(ivariant::from_color3(*ptr, true));

					return true;
				}

				if (form->IsPseudoClassSet("focus"))
				{
					if (!alpha)
						*ptr = trigonometry::vector4(color.x, color.y, color.z, ptr->w);
					else
						*ptr = color;

					return true;
				}

				if (alpha)
					form->SetValue(ivariant::from_color4(*ptr, value.empty() ? true : value[0] == '#'));
				else
					form->SetValue(ivariant::from_color3(*ptr, value.empty() ? true : value[0] == '#'));

				return false;
#else
				return false;
#endif
			}
			bool ielement::cast_form_string(core::string* ptr)
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				VI_ASSERT(ptr != nullptr, "ptr should be set");

				Rml::ElementFormControl* form = (Rml::ElementFormControl*)base;
				core::string value = form->GetValue();
				if (value == *ptr)
					return false;

				if (form->IsPseudoClassSet("focus"))
				{
					*ptr = std::move(value);
					return true;
				}

				form->SetValue(*ptr);
				return false;
#else
				return false;
#endif
			}
			bool ielement::cast_form_pointer(void** ptr)
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				VI_ASSERT(ptr != nullptr, "ptr should be set");

				Rml::ElementFormControl* form = (Rml::ElementFormControl*)base;
				void* value = to_pointer(form->GetValue());
				if (value == *ptr)
					return false;

				if (form->IsPseudoClassSet("focus"))
				{
					*ptr = value;
					return true;
				}

				form->SetValue(from_pointer(*ptr));
				return false;
#else
				return false;
#endif
			}
			bool ielement::cast_form_int32(int32_t* ptr)
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				VI_ASSERT(ptr != nullptr, "ptr should be set");

				Rml::ElementFormControl* form = (Rml::ElementFormControl*)base;
				core::string value(form->GetValue());
				if (value.empty())
				{
					if (form->IsPseudoClassSet("focus"))
						return false;

					form->SetValue(core::to_string(*ptr));
					return false;
				}

				if (!core::stringify::has_integer(value))
				{
					core::stringify::replace_not_of(value, ".-0123456789", "");
					form->SetValue(value);
				}

				auto n = core::from_string<int32_t>(value);
				if (!n || *n == *ptr)
					return false;

				if (form->IsPseudoClassSet("focus"))
				{
					*ptr = *n;
					return true;
				}

				form->SetValue(core::to_string(*ptr));
				return false;
#else
				return false;
#endif
			}
			bool ielement::cast_form_uint32(uint32_t* ptr)
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				VI_ASSERT(ptr != nullptr, "ptr should be set");

				Rml::ElementFormControl* form = (Rml::ElementFormControl*)base;
				core::string value(form->GetValue());
				if (value.empty())
				{
					if (form->IsPseudoClassSet("focus"))
						return false;

					form->SetValue(core::to_string(*ptr));
					return false;
				}

				if (!core::stringify::has_integer(value))
				{
					core::stringify::replace_not_of(value, ".0123456789", "");
					form->SetValue(value);
				}

				auto n = core::from_string<uint32_t>(value);
				if (!n || *n == *ptr)
					return false;

				if (form->IsPseudoClassSet("focus"))
				{
					*ptr = *n;
					return true;
				}

				form->SetValue(core::to_string(*ptr));
				return false;
#else
				return false;
#endif
			}
			bool ielement::cast_form_flag32(uint32_t* ptr, uint32_t mask)
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				VI_ASSERT(ptr != nullptr, "ptr should be set");

				bool value = (*ptr & mask);
				if (!cast_form_boolean(&value))
					return false;

				if (value)
					*ptr |= mask;
				else
					*ptr &= ~mask;

				return true;
#else
				return false;
#endif
			}
			bool ielement::cast_form_int64(int64_t* ptr)
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				VI_ASSERT(ptr != nullptr, "ptr should be set");

				Rml::ElementFormControl* form = (Rml::ElementFormControl*)base;
				core::string value(form->GetValue());
				if (value.empty())
				{
					if (form->IsPseudoClassSet("focus"))
						return false;

					form->SetValue(core::to_string(*ptr));
					return false;
				}

				if (!core::stringify::has_integer(value))
				{
					core::stringify::replace_not_of(value, ".-0123456789", "");
					form->SetValue(value);
				}

				auto n = core::from_string<int64_t>(value);
				if (!n || *n == *ptr)
					return false;

				if (form->IsPseudoClassSet("focus"))
				{
					*ptr = *n;
					return true;
				}

				form->SetValue(core::to_string(*ptr));
				return false;
#else
				return false;
#endif
			}
			bool ielement::cast_form_uint64(uint64_t* ptr)
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				VI_ASSERT(ptr != nullptr, "ptr should be set");

				Rml::ElementFormControl* form = (Rml::ElementFormControl*)base;
				core::string value(form->GetValue());
				if (value.empty())
				{
					if (form->IsPseudoClassSet("focus"))
						return false;

					form->SetValue(core::to_string(*ptr));
					return false;
				}

				if (!core::stringify::has_integer(value))
				{
					core::stringify::replace_not_of(value, ".0123456789", "");
					form->SetValue(value);
				}

				auto n = core::from_string<uint64_t>(value);
				if (!n || *n == *ptr)
					return false;

				if (form->IsPseudoClassSet("focus"))
				{
					*ptr = *n;
					return true;
				}

				form->SetValue(core::to_string(*ptr));
				return false;
#else
				return false;
#endif
			}
			bool ielement::cast_form_size(size_t* ptr)
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				VI_ASSERT(ptr != nullptr, "ptr should be set");

				Rml::ElementFormControl* form = (Rml::ElementFormControl*)base;
				core::string value(form->GetValue());
				if (value.empty())
				{
					if (form->IsPseudoClassSet("focus"))
						return false;

					form->SetValue(core::to_string(*ptr));
					return false;
				}

				if (!core::stringify::has_integer(value))
				{
					core::stringify::replace_not_of(value, ".0123456789", "");
					form->SetValue(value);
				}

				auto n = core::from_string<uint64_t>(value);
				if (!n || *n == (uint64_t)*ptr)
					return false;

				if (form->IsPseudoClassSet("focus"))
				{
					*ptr = (size_t)*n;
					return true;
				}

				form->SetValue(core::to_string(*ptr));
				return false;
#else
				return false;
#endif
			}
			bool ielement::cast_form_flag64(uint64_t* ptr, uint64_t mask)
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				VI_ASSERT(ptr != nullptr, "ptr should be set");

				bool value = (*ptr & mask);
				if (!cast_form_boolean(&value))
					return false;

				if (value)
					*ptr |= mask;
				else
					*ptr &= ~mask;

				return true;
#else
				return false;
#endif
			}
			bool ielement::cast_form_float(float* ptr)
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				VI_ASSERT(ptr != nullptr, "ptr should be set");

				Rml::ElementFormControl* form = (Rml::ElementFormControl*)base;
				core::string value(form->GetValue());
				if (value.empty())
				{
					if (form->IsPseudoClassSet("focus"))
						return false;

					form->SetValue(core::to_string(*ptr));
					return false;
				}

				if (!core::stringify::has_number(value))
				{
					core::stringify::replace_not_of(value, ".-0123456789", "");
					form->SetValue(value);
				}

				auto n = core::from_string<float>(value);
				if (!n || *n == *ptr)
					return false;

				if (form->IsPseudoClassSet("focus"))
				{
					*ptr = *n;
					return true;
				}

				form->SetValue(core::to_string(*ptr));
				return false;
#else
				return false;
#endif
			}
			bool ielement::cast_form_float(float* ptr, float mult)
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				VI_ASSERT(ptr != nullptr, "ptr should be set");

				*ptr *= mult;
				bool result = cast_form_float(ptr);
				*ptr /= mult;

				return result;
#else
				return false;
#endif
			}
			bool ielement::cast_form_double(double* ptr)
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				VI_ASSERT(ptr != nullptr, "ptr should be set");

				Rml::ElementFormControl* form = (Rml::ElementFormControl*)base;
				core::string value(form->GetValue());
				if (value.empty())
				{
					if (form->IsPseudoClassSet("focus"))
						return false;

					form->SetValue(core::to_string(*ptr));
					return false;
				}

				if (!core::stringify::has_number(value))
				{
					core::stringify::replace_not_of(value, ".-0123456789", "");
					form->SetValue(value);
				}

				auto n = core::from_string<double>(value);
				if (!n || *n == *ptr)
					return false;

				if (form->IsPseudoClassSet("focus"))
				{
					*ptr = *n;
					return true;
				}

				form->SetValue(core::to_string(*ptr));
				return false;
#else
				return false;
#endif
			}
			bool ielement::cast_form_boolean(bool* ptr)
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				VI_ASSERT(ptr != nullptr, "ptr should be set");

				Rml::ElementFormControl* form = (Rml::ElementFormControl*)base;
				bool b = form->HasAttribute("checked");
				if (b == *ptr)
					return false;

				if (form->IsPseudoClassSet("focus"))
				{
					*ptr = b;
					return true;
				}

				if (*ptr)
					form->SetAttribute<bool>("checked", true);
				else
					form->RemoveAttribute("checked");

				return false;
#else
				return false;
#endif
			}
			core::string ielement::get_form_name() const
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				Rml::ElementFormControl* form = (Rml::ElementFormControl*)base;
				return form->GetName();
#else
				return core::string();
#endif
			}
			void ielement::set_form_name(const std::string_view& name)
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				Rml::ElementFormControl* form = (Rml::ElementFormControl*)base;
				form->SetName(core::string(name));
#endif
			}
			core::string ielement::get_form_value() const
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				Rml::ElementFormControl* form = (Rml::ElementFormControl*)base;
				return form->GetValue();
#else
				return core::string();
#endif
			}
			void ielement::set_form_value(const std::string_view& value)
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				Rml::ElementFormControl* form = (Rml::ElementFormControl*)base;
				form->SetValue(core::string(value));
#endif
			}
			bool ielement::is_form_disabled() const
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				Rml::ElementFormControl* form = (Rml::ElementFormControl*)base;
				return form->IsDisabled();
#else
				return false;
#endif
			}
			void ielement::set_form_disabled(bool disable)
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				Rml::ElementFormControl* form = (Rml::ElementFormControl*)base;
				form->SetDisabled(disable);
#endif
			}
			Rml::Element* ielement::get_element() const
			{
				return base;
			}
			bool ielement::is_valid() const
			{
				return base != nullptr;
			}
			core::string ielement::from_pointer(void* ptr)
			{
				if (!ptr)
					return "0";

				return core::to_string((intptr_t)(void*)ptr);
			}
			void* ielement::to_pointer(const std::string_view& value)
			{
				if (value.empty())
					return nullptr;

				if (!core::stringify::has_integer(value))
					return nullptr;

				return (void*)(intptr_t)*core::from_string<int64_t>(value);
			}

			ielement_document::ielement_document() : ielement()
			{
			}
			ielement_document::ielement_document(Rml::ElementDocument* ref) : ielement((Rml::Element*)ref)
			{
			}
			void ielement_document::release()
			{
#ifdef VI_RMLUI
				Rml::ElementDocument* item = (Rml::ElementDocument*)base;
				core::memory::deinit(item);
				base = nullptr;
#endif
			}
			void ielement_document::set_title(const std::string_view& title)
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				((Rml::ElementDocument*)base)->SetTitle(core::string(title));
#endif
			}
			void ielement_document::pull_to_front()
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				((Rml::ElementDocument*)base)->PullToFront();
#endif
			}
			void ielement_document::push_to_back()
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				((Rml::ElementDocument*)base)->PushToBack();
#endif
			}
			void ielement_document::show(modal_flag modal, focus_flag focus)
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				((Rml::ElementDocument*)base)->Show((Rml::ModalFlag)modal, (Rml::FocusFlag)focus);
#endif
			}
			void ielement_document::hide()
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				((Rml::ElementDocument*)base)->Hide();
#endif
			}
			void ielement_document::close()
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				((Rml::ElementDocument*)base)->Close();
#endif
			}
			core::string ielement_document::get_title() const
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				return ((Rml::ElementDocument*)base)->GetTitle();
#else
				return core::string();
#endif
			}
			core::string ielement_document::get_source_url() const
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				return ((Rml::ElementDocument*)base)->GetSourceURL();
#else
				return core::string();
#endif
			}
			ielement ielement_document::create_element(const std::string_view& name)
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				Rml::ElementPtr ptr = ((Rml::ElementDocument*)base)->CreateElement(core::string(name));
				Rml::Element* result = ptr.get();
				ptr.reset();

				return result;
#else
				return ielement();
#endif
			}
			bool ielement_document::is_modal() const
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "element should be valid");
				return ((Rml::ElementDocument*)base)->IsModal();
#else
				return false;
#endif
			}
			Rml::ElementDocument* ielement_document::get_element_document() const
			{
				return (Rml::ElementDocument*)base;
			}

			trigonometry::matrix4x4 utils::to_matrix(const void* matrix) noexcept
			{
#ifdef VI_RMLUI
				VI_ASSERT(matrix != nullptr, "matrix should be set");
				const Rml::Matrix4f* new_transform = (const Rml::Matrix4f*)matrix;
				Rml::Vector4f row11 = new_transform->GetRow(0);
				trigonometry::matrix4x4 result;
				result.row[0] = row11.x;
				result.row[1] = row11.y;
				result.row[2] = row11.z;
				result.row[3] = row11.w;

				Rml::Vector4f row22 = new_transform->GetRow(1);
				result.row[4] = row22.x;
				result.row[5] = row22.y;
				result.row[6] = row22.z;
				result.row[7] = row22.w;

				Rml::Vector4f row33 = new_transform->GetRow(2);
				result.row[8] = row33.x;
				result.row[9] = row33.y;
				result.row[10] = row33.z;
				result.row[11] = row33.w;

				Rml::Vector4f row44 = new_transform->GetRow(3);
				result.row[12] = row44.x;
				result.row[13] = row44.y;
				result.row[14] = row44.z;
				result.row[15] = row44.w;

				return result.transpose();
#else
				return trigonometry::matrix4x4();
#endif
			}
			core::string utils::escape_html(const std::string_view& text) noexcept
			{
				core::string copy = core::string(text);
				core::stringify::replace(copy, "\r\n", "&nbsp;");
				core::stringify::replace(copy, "\n", "&nbsp;");
				core::stringify::replace(copy, "<", "&lt;");
				core::stringify::replace(copy, ">", "&gt;");
				return copy;
			}

			subsystem::subsystem() noexcept : context_factory(nullptr), document_factory(nullptr), listener_factory(nullptr), render_interface(nullptr), file_interface(nullptr), system_interface(nullptr), id(0)
			{
#ifdef VI_RMLUI
				render_interface = core::memory::init<render_subsystem>();
				Rml::SetRenderInterface(render_interface->GetAdaptedInterface());

				file_interface = core::memory::init<file_subsystem>();
				Rml::SetFileInterface(file_interface);

				system_interface = core::memory::init<main_subsystem>();
				Rml::SetSystemInterface(system_interface);

				Rml::Initialise();
				{
					context_factory = core::memory::init<context_instancer>();
					Rml::Factory::RegisterContextInstancer(context_factory);

					listener_factory = core::memory::init<listener_instancer>();
					Rml::Factory::RegisterEventListenerInstancer(listener_factory);

					document_factory = core::memory::init<document_instancer>();
					Rml::Factory::RegisterElementInstancer("body", document_factory);
				}
				create_elements();
#endif
			}
			subsystem::~subsystem() noexcept
			{
#ifdef VI_RMLUI
				Rml::Shutdown();
				core::memory::deinit(system_interface);
				system_interface = nullptr;

				core::memory::deinit(file_interface);
				file_interface = nullptr;

				core::memory::deinit(render_interface);
				render_interface = nullptr;

				core::memory::deinit(document_factory);
				document_factory = nullptr;

				core::memory::deinit(listener_factory);
				listener_factory = nullptr;

				core::memory::deinit(context_factory);
				context_factory = nullptr;

				release_elements();
				shared.release();
#endif
			}
			void subsystem::set_shared(scripting::virtual_machine* vm, graphics::activity* activity, render_constants* constants, heavy_content_manager* content, core::timer* time) noexcept
			{
#ifdef VI_RMLUI
				cleanup_shared();
				shared.vm = vm;
				shared.activity = activity;
				shared.constants = constants;
				shared.content = content;
				shared.time = time;
				shared.add_ref();

				if (render_interface != nullptr)
					render_interface->attach(constants, content);

				if (system_interface != nullptr)
					system_interface->attach(activity, time);
#endif
			}
			void subsystem::set_translator(const std::string_view& name, translation_callback&& callback) noexcept
			{
#ifdef VI_RMLUI
				VI_ASSERT(system_interface != nullptr, "system interface should be valid");
				system_interface->set_translator(core::string(name), std::move(callback));
#endif
			}
			void subsystem::cleanup_shared()
			{
#ifdef VI_RMLUI
				if (render_interface != nullptr)
					render_interface->detach();

				if (system_interface != nullptr)
					system_interface->detach();
#endif
				shared.release();
			}
			render_subsystem* subsystem::get_render_interface() noexcept
			{
				return render_interface;
			}
			file_subsystem* subsystem::get_file_interface() noexcept
			{
				return file_interface;
			}
			main_subsystem* subsystem::get_system_interface() noexcept
			{
				return system_interface;
			}
			graphics::graphics_device* subsystem::get_device() noexcept
			{
#ifdef VI_RMLUI
				VI_ASSERT(render_interface != nullptr, "render interface should be valid");
				return render_interface->get_device();
#else
				return nullptr;
#endif
			}
			graphics::texture_2d* subsystem::get_background() noexcept
			{
#ifdef VI_RMLUI
				VI_ASSERT(render_interface != nullptr, "render interface should be valid");
				return render_interface->background;
#else
				return nullptr;
#endif
			}
			trigonometry::matrix4x4* subsystem::get_transform() noexcept
			{
#ifdef VI_RMLUI
				VI_ASSERT(render_interface != nullptr, "render interface should be valid");
				return render_interface->get_transform();
#else
				return nullptr;
#endif
			}
			trigonometry::matrix4x4* subsystem::get_projection() noexcept
			{
#ifdef VI_RMLUI
				VI_ASSERT(render_interface != nullptr, "render interface should be valid");
				return render_interface->get_projection();
#else
				return nullptr;
#endif
			}

			data_model::data_model(Rml::DataModelConstructor* ref) : base(nullptr)
			{
#ifdef VI_RMLUI
				if (ref != nullptr)
					base = core::memory::init<Rml::DataModelConstructor>(*ref);
#endif
			}
			data_model::~data_model() noexcept
			{
#ifdef VI_RMLUI
				detach();
				for (auto item : props)
					core::memory::deinit(item.second);

				core::memory::deinit(base);
#endif
			}
			void data_model::set_detach_callback(std::function<void()>&& callback)
			{
				if (callback)
					callbacks.emplace_back(std::move(callback));
			}
			data_node* data_model::set_property(const std::string_view& name, const core::variant& value)
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "data node should be valid");
				data_node* result = get_property(name);
				if (result != nullptr)
				{
					result->set(value);
					return result;
				}

				core::string copy = core::string(name);
				result = core::memory::init<data_node>(this, name, value);
				if (!value.is_object())
				{
					if (base->BindFunc(copy, std::bind(&data_node::get_value, result, std::placeholders::_1), std::bind(&data_node::set_value, result, std::placeholders::_1)))
					{
						props[copy] = result;
						return result;
					}
				}
				else if (base->Bind(copy, result))
				{
					props[copy] = result;
					return result;
				}
				core::memory::deinit(result);
				return nullptr;
#else
				return nullptr;
#endif
			}
			data_node* data_model::set_property(const std::string_view& name, core::variant* value)
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "data node should be valid");
				VI_ASSERT(value != nullptr, "value should be set");

				data_node* sub = get_property(name);
				if (sub != nullptr)
				{
					sub->set(value);
					return sub;
				}

				core::string copy = core::string(name);
				data_node* result = core::memory::init<data_node>(this, name, value);
				if (base->Bind(copy, result))
				{
					props[copy] = result;
					return result;
				}

				core::memory::deinit(result);
				return nullptr;
#else
				return nullptr;
#endif
			}
			data_node* data_model::set_array(const std::string_view& name)
			{
				return set_property(name, core::var::array());
			}
			data_node* data_model::set_string(const std::string_view& name, const std::string_view& value)
			{
				return set_property(name, core::var::string(value));
			}
			data_node* data_model::set_integer(const std::string_view& name, int64_t value)
			{
				return set_property(name, core::var::integer(value));
			}
			data_node* data_model::set_float(const std::string_view& name, float value)
			{
				return set_property(name, core::var::number(value));
			}
			data_node* data_model::set_double(const std::string_view& name, double value)
			{
				return set_property(name, core::var::number(value));
			}
			data_node* data_model::set_boolean(const std::string_view& name, bool value)
			{
				return set_property(name, core::var::boolean(value));
			}
			data_node* data_model::set_pointer(const std::string_view& name, void* value)
			{
				return set_property(name, core::var::pointer(value));
			}
			data_node* data_model::get_property(const std::string_view& name)
			{
				auto it = props.find(core::key_lookup_cast(name));
				if (it != props.end())
					return it->second;

				return nullptr;
			}
			core::string data_model::get_string(const std::string_view& name)
			{
				data_node* result = get_property(name);
				if (!result)
					return "";

				return result->ref->get_blob();
			}
			int64_t data_model::get_integer(const std::string_view& name)
			{
				data_node* result = get_property(name);
				if (!result)
					return 0;

				return result->ref->get_integer();
			}
			float data_model::get_float(const std::string_view& name)
			{
				data_node* result = get_property(name);
				if (!result)
					return 0.0f;

				return (float)result->ref->get_number();
			}
			double data_model::get_double(const std::string_view& name)
			{
				data_node* result = get_property(name);
				if (!result)
					return 0.0;

				return result->ref->get_number();
			}
			bool data_model::get_boolean(const std::string_view& name)
			{
				data_node* result = get_property(name);
				if (!result)
					return false;

				return result->ref->get_boolean();
			}
			void* data_model::get_pointer(const std::string_view& name)
			{
				data_node* result = get_property(name);
				if (!result)
					return nullptr;

				return result->ref->get_pointer();
			}
			bool data_model::set_callback(const std::string_view& name, data_callback&& callback)
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "data node should be valid");
				VI_ASSERT(callback, "callback should not be empty");
				return base->BindEventCallback(core::string(name), [callback = std::move(callback)](Rml::DataModelHandle handle, Rml::Event& event, const Rml::VariantList& props)
				{
					core::variant_list args;
					args.resize(props.size());

					size_t i = 0;
					for (auto& item : props)
						ivariant::convert((Rml::Variant*)&item, &args[i++]);

					ievent basis(&event);
					callback(basis, args);
				});
#else
				return false;
#endif
			}
			bool data_model::set_unmount_callback(model_callback&& callback)
			{
				on_unmount = std::move(callback);
				return true;
			}
			void data_model::change(const std::string_view& variable_name)
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "data node should be valid");
				base->GetModelHandle().DirtyVariable(core::string(variable_name));
#endif
			}
			bool data_model::has_changed(const std::string_view& variable_name) const
			{
#ifdef VI_RMLUI
				VI_ASSERT(is_valid(), "data node should be valid");
				return base->GetModelHandle().IsVariableDirty(core::string(variable_name));
#else
				return false;
#endif
			}
			void data_model::detach()
			{
				for (auto& item : callbacks)
					item();
				callbacks.clear();
			}
			bool data_model::is_valid() const
			{
				return base != nullptr;
			}
			Rml::DataModelConstructor* data_model::get()
			{
				return base;
			}

			data_node::data_node(data_model* model, const std::string_view& top_name, const core::variant& initial) noexcept : name(top_name), handle(model), order(nullptr), depth(0), safe(true)
			{
				ref = core::memory::init<core::variant>(initial);
			}
			data_node::data_node(data_model* model, const std::string_view& top_name, core::variant* reference) noexcept : name(top_name), ref(reference), handle(model), order(nullptr), depth(0), safe(false)
			{
			}
			data_node::data_node(const data_node& other) noexcept : childs(other.childs), name(other.name), handle(other.handle), order(other.order), depth(0), safe(other.safe)
			{
				if (safe)
					ref = core::memory::init<core::variant>(*other.ref);
				else
					ref = other.ref;
			}
			data_node::data_node(data_node&& other) noexcept : childs(std::move(other.childs)), name(std::move(other.name)), ref(other.ref), handle(other.handle), order(other.order), depth(other.depth), safe(other.safe)
			{
				other.ref = nullptr;
			}
			data_node::~data_node()
			{
				if (safe)
					core::memory::deinit(ref);
			}
			data_node& data_node::insert(size_t where, const core::variant_list& initial, std::pair<void*, size_t>* top)
			{
				VI_ASSERT(where <= childs.size(), "index outside of range");
				data_node result(handle, name, core::var::array());
				if (top != nullptr)
				{
					result.order = top->first;
					result.depth = top->second;
				}

				for (auto& item : initial)
					result.add(item);

				childs.insert(childs.begin() + where, std::move(result));
				if (top != nullptr)
					sort_tree();
				else if (handle != nullptr && !name.empty())
					handle->change(name);

				return childs.back();
			}
			data_node& data_node::insert(size_t where, const core::variant& initial, std::pair<void*, size_t>* top)
			{
				VI_ASSERT(where <= childs.size(), "index outside of range");
				data_node result(handle, name, initial);
				if (top != nullptr)
				{
					result.order = top->first;
					result.depth = top->second;
				}

				childs.insert(childs.begin() + where, std::move(result));
				if (top != nullptr)
					sort_tree();
				else if (handle != nullptr && !name.empty())
					handle->change(name);

				return childs.back();
			}
			data_node& data_node::insert(size_t where, core::variant* reference, std::pair<void*, size_t>* top)
			{
				VI_ASSERT(where <= childs.size(), "index outside of range");
				data_node result(handle, name, reference);
				if (top != nullptr)
				{
					result.order = top->first;
					result.depth = top->second;
				}

				childs.insert(childs.begin() + where, std::move(result));
				if (top != nullptr)
					sort_tree();
				else if (handle != nullptr && !name.empty())
					handle->change(name);

				return childs.back();
			}
			data_node& data_node::add(const core::variant_list& initial, std::pair<void*, size_t>* top)
			{
				data_node result(handle, name, core::var::array());
				if (top != nullptr)
				{
					result.order = top->first;
					result.depth = top->second;
				}

				for (auto& item : initial)
					result.add(item);

				childs.emplace_back(std::move(result));
				if (top != nullptr)
					sort_tree();
				else if (handle != nullptr && !name.empty())
					handle->change(name);

				return childs.back();
			}
			data_node& data_node::add(const core::variant& initial, std::pair<void*, size_t>* top)
			{
				data_node result(handle, name, initial);
				if (top != nullptr)
				{
					result.order = top->first;
					result.depth = top->second;
				}

				childs.emplace_back(std::move(result));
				if (top != nullptr)
					sort_tree();
				else if (handle != nullptr && !name.empty())
					handle->change(name);

				return childs.back();
			}
			data_node& data_node::add(core::variant* reference, std::pair<void*, size_t>* top)
			{
				data_node result(handle, name, reference);
				if (top != nullptr)
				{
					result.order = top->first;
					result.depth = top->second;
				}

				childs.emplace_back(std::move(result));
				if (top != nullptr)
					sort_tree();
				else if (handle != nullptr && !name.empty())
					handle->change(name);

				return childs.back();
			}
			data_node& data_node::at(size_t index)
			{
				VI_ASSERT(index < childs.size(), "index outside of range");
				return childs[index];
			}
			bool data_node::remove(size_t index)
			{
				VI_ASSERT(index < childs.size(), "index outside of range");
				childs.erase(childs.begin() + index);

				if (handle != nullptr && !name.empty())
					handle->change(name);

				return true;
			}
			bool data_node::clear()
			{
				childs.clear();
				if (handle != nullptr && !name.empty())
					handle->change(name);

				return true;
			}
			void data_node::sort_tree()
			{
				VI_SORT(childs.begin(), childs.end(), [](const data_node& a, const data_node& b)
				{
					double D1 = (double)(uintptr_t)a.get_seq_id() + 0.00000001 * (double)a.get_depth();
					double D2 = (double)(uintptr_t)b.get_seq_id() + 0.00000001 * (double)b.get_depth();
					return D1 < D2;
				});

				if (handle != nullptr && !name.empty())
					handle->change(name);
			}
			void data_node::set_top(void* seq_id, size_t nesting)
			{
				order = seq_id;
				depth = nesting;
			}
			void data_node::set(const core::variant& new_value)
			{
				if (*ref == new_value)
					return;

				*ref = new_value;
				if (handle != nullptr && !name.empty())
					handle->change(name);
			}
			void data_node::set(core::variant* new_reference)
			{
				if (!new_reference || new_reference == ref)
					return;

				if (safe)
					core::memory::deinit(ref);

				ref = new_reference;
				safe = false;

				if (handle != nullptr && !name.empty())
					handle->change(name);
			}
			void data_node::replace(const core::variant_list& data, std::pair<void*, size_t>* top)
			{
				childs.clear();
				for (auto& item : data)
					childs.emplace_back(data_node(handle, name, item));

				if (top != nullptr)
				{
					order = top->first;
					depth = top->second;
				}

				if (handle != nullptr && !name.empty())
					handle->change(name);
			}
			void data_node::set_string(const std::string_view& value)
			{
				set(core::var::string(value));
			}
			void data_node::set_vector2(const trigonometry::vector2& value)
			{
				set(core::var::string(ivariant::from_vector2(value)));
			}
			void data_node::set_vector3(const trigonometry::vector3& value)
			{
				set(core::var::string(ivariant::from_vector3(value)));
			}
			void data_node::set_vector4(const trigonometry::vector4& value)
			{
				set(core::var::string(ivariant::from_vector4(value)));
			}
			void data_node::set_integer(int64_t value)
			{
				set(core::var::integer(value));
			}
			void data_node::set_float(float value)
			{
				set(core::var::number(value));
			}
			void data_node::set_double(double value)
			{
				set(core::var::number(value));
			}
			void data_node::set_boolean(bool value)
			{
				set(core::var::boolean(value));
			}
			void data_node::set_pointer(void* value)
			{
				set(core::var::pointer(value));
			}
			size_t data_node::size() const
			{
				return childs.size();
			}
			size_t data_node::get_depth() const
			{
				return depth;
			}
			void* data_node::get_seq_id() const
			{
				return order;
			}
			const core::variant& data_node::get()
			{
				return *ref;
			}
			core::string data_node::get_string()
			{
				return ref->get_blob();
			}
			trigonometry::vector2 data_node::get_vector2()
			{
				return ivariant::to_vector2(ref->get_blob());
			}
			trigonometry::vector3 data_node::get_vector3()
			{
				return ivariant::to_vector3(ref->get_blob());
			}
			trigonometry::vector4 data_node::get_vector4()
			{
				return ivariant::to_vector4(ref->get_blob());
			}
			int64_t data_node::get_integer()
			{
				return ref->get_integer();
			}
			float data_node::get_float()
			{
				return (float)ref->get_number();
			}
			double data_node::get_double()
			{
				return ref->get_number();
			}
			bool data_node::get_boolean()
			{
				return ref->get_boolean();
			}
			void* data_node::get_pointer()
			{
				return ref->get_pointer();
			}
			void data_node::get_value(Rml::Variant& result)
			{
				ivariant::revert(ref, &result);
			}
			void data_node::set_value(const Rml::Variant& result)
			{
				ivariant::convert((Rml::Variant*)&result, ref);
			}
			void data_node::set_value_str(const core::string& value)
			{
				*ref = core::var::string(value);
			}
			void data_node::set_value_num(double value)
			{
				*ref = core::var::number(value);
			}
			void data_node::set_value_int(int64_t value)
			{
				*ref = core::var::integer(value);
			}
			int64_t data_node::get_value_size()
			{
				return (int64_t)size();
			}
			data_node& data_node::operator= (const data_node& other) noexcept
			{
				if (this == &other)
					return *this;

				this->~data_node();
				childs = other.childs;
				name = other.name;
				handle = other.handle;
				order = other.order;
				depth = other.depth;
				safe = other.safe;

				if (safe)
					ref = core::memory::init<core::variant>(*other.ref);
				else
					ref = other.ref;

				return *this;
			}
			data_node& data_node::operator= (data_node&& other) noexcept
			{
				if (this == &other)
					return *this;

				this->~data_node();
				childs = std::move(other.childs);
				name = std::move(other.name);
				ref = other.ref;
				handle = other.handle;
				name = other.name;
				order = other.order;
				depth = other.depth;
				safe = other.safe;

				other.ref = nullptr;
				return *this;
			}

			listener::listener(event_callback&& new_callback)
			{
#ifdef VI_RMLUI
				base = core::memory::init<event_subsystem>(std::move(new_callback));
#endif
			}
			listener::listener(const std::string_view& function_name)
			{
#ifdef VI_RMLUI
				base = core::memory::init<listener_subsystem>(function_name, nullptr);
#endif
			}
			listener::~listener() noexcept
			{
#ifdef VI_RMLUI
				base->OnDetach(nullptr);
#endif
			}

			context::context(const trigonometry::vector2& size) : compiler(nullptr), cursor(-1.0f), system(subsystem::get()), busy(0), skips(0)
			{
#ifdef VI_RMLUI
				base = (scoped_context*)Rml::CreateContext(core::to_string(subsystem::get()->id++), Rml::Vector2i((int)size.x, (int)size.y));
				VI_ASSERT(base != nullptr, "context cannot be created");
				base->basis = this;
				if (system->shared.vm != nullptr)
				{
					compiler = system->shared.vm->create_compiler();
					clear_scope();
				}
				system->add_ref();
#endif
			}
			context::context(graphics::graphics_device* device) : context(device&& device->get_render_target() ? trigonometry::vector2((float)device->get_render_target()->get_width(), (float)device->get_render_target()->get_height()) : trigonometry::vector2(512, 512))
			{
			}
			context::~context() noexcept
			{
#ifdef VI_RMLUI
				remove_data_models();
				Rml::RemoveContext(base->GetName());
				core::memory::release(compiler);
				core::memory::release(system);
#endif
			}
			void context::emit_key(graphics::key_code key, graphics::key_mod mod, int computed, int repeat, bool pressed)
			{
#ifdef VI_RMLUI
				if (key == graphics::key_code::cursor_left)
				{
					if (pressed)
					{
						if (base->ProcessMouseButtonDown(0, get_key_mod(mod)))
							inputs.cursor = true;
					}
					else
					{
						if (base->ProcessMouseButtonUp(0, get_key_mod(mod)))
							inputs.cursor = true;
					}
				}
				else if (key == graphics::key_code::cursor_right)
				{
					if (pressed)
					{
						if (base->ProcessMouseButtonDown(1, get_key_mod(mod)))
							inputs.cursor = true;
					}
					else
					{
						if (base->ProcessMouseButtonUp(1, get_key_mod(mod)))
							inputs.cursor = true;
					}
				}
				else if (key == graphics::key_code::cursor_middle)
				{
					if (pressed)
					{
						if (base->ProcessMouseButtonDown(2, get_key_mod(mod)))
							inputs.cursor = true;
					}
					else
					{
						if (base->ProcessMouseButtonUp(2, get_key_mod(mod)))
							inputs.cursor = true;
					}
				}
				else if (pressed)
				{
					if (base->ProcessKeyDown((Rml::Input::KeyIdentifier)get_key_code(key), get_key_mod(mod)))
						inputs.keys = true;
				}
				else if (base->ProcessKeyUp((Rml::Input::KeyIdentifier)get_key_code(key), get_key_mod(mod)))
					inputs.keys = true;
				skips = RENDERS_FOR_DATA_EVENT;
#endif
			}
			void context::emit_input(const char* buffer, int length)
			{
#ifdef VI_RMLUI
				VI_ASSERT(buffer != nullptr && length > 0, "buffer should be set");
				if (base->ProcessTextInput(core::string(buffer, length)))
					inputs.text = true;
				skips = RENDERS_FOR_DATA_EVENT;
#endif
			}
			void context::emit_wheel(int x, int y, bool normal, graphics::key_mod mod)
			{
#ifdef VI_RMLUI
				if (base->ProcessMouseWheel((float)-y, get_key_mod(mod)))
					inputs.scroll = true;
				skips = RENDERS_FOR_DATA_EVENT;
#endif
			}
			void context::emit_resize(int width, int height)
			{
#ifdef VI_RMLUI
				render_subsystem* renderer = subsystem::get()->get_render_interface();
				if (renderer != nullptr)
					renderer->resize_buffers(width, height);

				base->SetDimensions(Rml::Vector2i(width, height));
				skips = RENDERS_FOR_SIZE_EVENT;
#endif
			}
			void context::update_events(graphics::activity* activity)
			{
#ifdef VI_RMLUI
				inputs.keys = false;
				inputs.text = false;
				inputs.scroll = false;
				inputs.cursor = false;

				if (activity != nullptr)
				{
					cursor = activity->get_cursor_position();
					if (base->ProcessMouseMove((int)cursor.x, (int)cursor.y, get_key_mod(activity->get_key_mod_state())))
						inputs.cursor = true;
				}

				base->Update();
#endif
			}
			void context::render_lists(graphics::texture_2d* target)
			{
#ifdef VI_RMLUI
				VI_ASSERT(subsystem::get()->get_render_interface() != nullptr, "render interface should be valid");
				render_subsystem* renderer = subsystem::get()->get_render_interface();
				renderer->background = target;
				base->Render();
#endif
			}
			void context::clear_styles()
			{
#ifdef VI_RMLUI
				Rml::StyleSheetFactory::ClearStyleSheetCache();
#endif
			}
			bool context::clear_documents()
			{
#ifdef VI_RMLUI
				++busy;
				clear_scope();
				elements.clear();
				base->UnloadAllDocuments();
				--busy;

				return true;
#else
				return false;
#endif
			}
			bool context::is_loading()
			{
				return busy > 0;
			}
			bool context::is_input_focused()
			{
#ifdef VI_RMLUI
				Rml::Element* element = base->GetFocusElement();
				if (!element)
					return false;

				const Rml::String& tag = element->GetTagName();
				return tag == "input" || tag == "textarea" || tag == "select";
#else
				return false;
#endif
			}
			uint64_t context::calculate_idle_timeout_ms(uint64_t max_timeout)
			{
#ifdef VI_RMLUI
				double timeout = base->GetNextUpdateDelay();
				if (timeout <= 0.0 || (skips > 0 && --skips > 0))
					return 0;

				if (timeout > (double)max_timeout)
					return max_timeout;

				return (uint64_t)(1000.0 * timeout);
#else
				return 0;
#endif
			}
			core::unordered_map<core::string, core::vector<font_info>>* context::get_font_faces()
			{
#ifdef VI_RMLUI
				return subsystem::get()->get_system_interface()->get_font_faces();
#else
				return nullptr;
#endif
			}
			Rml::Context* context::get_context()
			{
#ifdef VI_RMLUI
				return base;
#else
				return nullptr;
#endif
			}
			trigonometry::vector2 context::get_dimensions() const
			{
#ifdef VI_RMLUI
				Rml::Vector2i size = base->GetDimensions();
				return trigonometry::vector2((float)size.x, (float)size.y);
#else
				return trigonometry::vector2();
#endif
			}
			void context::set_density_independent_pixel_ratio(float density_independent_pixel_ratio)
			{
#ifdef VI_RMLUI
				base->SetDensityIndependentPixelRatio(density_independent_pixel_ratio);
#endif
			}
			void context::enable_mouse_cursor(bool enable)
			{
#ifdef VI_RMLUI
				base->EnableMouseCursor(enable);
#endif
			}
			float context::get_density_independent_pixel_ratio() const
			{
#ifdef VI_RMLUI
				return base->GetDensityIndependentPixelRatio();
#else
				return 0.0f;
#endif
			}
			bool context::replace_html(const std::string_view& selector, const std::string_view& HTML, int index)
			{
#ifdef VI_RMLUI
				auto* current = base->GetDocument(index);
				if (!current)
					return false;

				auto target_ptr = current->QuerySelector(core::string(selector));
				if (!target_ptr)
					return false;

				target_ptr->SetInnerRML(core::string(HTML));
				return true;
#else
				return false;
#endif
			}
			expects_gui_exception<void> context::load_font_face(const std::string_view& path, bool use_as_fallback, font_weight weight)
			{
#ifdef VI_RMLUI
				VI_ASSERT(subsystem::get()->get_system_interface() != nullptr, "system interface should be set");
				core::string target_path = core::string(path);
				if (!subsystem::get()->get_system_interface()->add_font_face(target_path, use_as_fallback, weight))
					return gui_exception("initialize font face error: " + target_path);

				return core::expectation::met;
#else
				return gui_exception("unsupported");
#endif
			}
			expects_gui_exception<ielement_document> context::eval_html(const std::string_view& HTML, int index)
			{
#ifdef VI_RMLUI
				++busy;
				auto* current = base->GetDocument(index);
				if (!current)
				{
					current = base->LoadDocumentFromMemory("<html><body>" + core::string(HTML) + "</body></html>");
					if (!current)
					{
						--busy;
						return gui_exception("eval html: invalid argument");
					}
				}
				else
					current->SetInnerRML(core::string(HTML));

				--busy;
				return ielement_document(current);
#else
				return gui_exception("unsupported");
#endif
			}
			expects_gui_exception<ielement_document> context::add_css(const std::string_view& CSS, int index)
			{
#ifdef VI_RMLUI
				++busy;
				auto* current = base->GetDocument(index);
				if (current != nullptr)
				{
					auto head_ptr = current->QuerySelector("head");
					if (head_ptr != nullptr)
					{
						auto style_ptr = head_ptr->QuerySelector("style");
						if (!style_ptr)
						{
							auto style = current->CreateElement("style");
							style->SetInnerRML(core::string(CSS));
							head_ptr->AppendChild(std::move(style));
						}
						else
							style_ptr->SetInnerRML(core::string(CSS));
					}
					else
					{
						auto head = current->CreateElement("head");
						{
							auto style = current->CreateElement("style");
							style->SetInnerRML(core::string(CSS));
							head->AppendChild(std::move(style));
						}
						current->AppendChild(std::move(head));
					}
				}
				else
				{
					current = base->LoadDocumentFromMemory("<html><head><style>" + core::string(CSS) + "</style></head></html>");
					if (!current)
					{
						--busy;
						return gui_exception("add css: invalid argument");
					}
				}

				--busy;
				return ielement_document(current);
#else
				return gui_exception("unsupported");
#endif
			}
			expects_gui_exception<ielement_document> context::load_css(const std::string_view& path, int index)
			{
#ifdef VI_RMLUI
				++busy;
				auto* current = base->GetDocument(index);
				if (current != nullptr)
				{
					auto head_ptr = current->QuerySelector("head");
					if (!head_ptr)
					{
						auto head = current->CreateElement("head");
						head_ptr = current->AppendChild(std::move(head));
					}

					auto link = current->CreateElement("link");
					link->SetAttribute("type", "text/css");
					link->SetAttribute("href", core::string(path));
					head_ptr = current->AppendChild(std::move(link));
				}
				else
				{
					current = base->LoadDocumentFromMemory("<html><head><link type=\"text/css\" href=\"" + core::string(path) + "\" /></head></html>");
					if (!current)
					{
						--busy;
						return gui_exception("load css: invalid argument");
					}
				}

				--busy;
				return ielement_document(current);
#else
				return gui_exception("unsupported");
#endif
			}
			expects_gui_exception<ielement_document> context::load_document(const std::string_view& path, bool allow_includes)
			{
#ifdef VI_RMLUI
				++busy;
				if (on_mount)
					on_mount(this);

				heavy_content_manager* content = (subsystem::get()->get_render_interface() ? subsystem::get()->get_render_interface()->get_content() : nullptr);
				auto subpath = (content ? core::os::path::resolve(path, content->get_environment(), true) : core::os::path::resolve(path));
				auto target_path = subpath ? *subpath : path;
				auto file = core::os::file::read_as_string(target_path);
				if (!file)
				{
					--busy;
					return gui_exception("load document: invalid path");
				}

				core::string data = *file;
				decompose(data);
				if (allow_includes)
				{
					auto status = preprocess(target_path, data);
					if (!status)
					{
						--busy;
						return status.error();
					}
				}

				core::string location(target_path);
				core::stringify::replace(location, '\\', '/');
				auto* result = base->LoadDocumentFromMemory(data, "file:///" + location);
				--busy;
				if (!result)
					return gui_exception("load document: invalid argument");

				return ielement_document(result);
#else
				return gui_exception("unsupported");
#endif
			}
			expects_gui_exception<ielement_document> context::add_document_empty(const std::string_view& instancer_name)
			{
#ifdef VI_RMLUI
				auto* result = base->CreateDocument(core::string(instancer_name));
				if (!result)
					return gui_exception("add document: invalid argument");

				return ielement_document(result);
#else
				return gui_exception("unsupported");
#endif
			}
			expects_gui_exception<ielement_document> context::add_document(const std::string_view& HTML)
			{
#ifdef VI_RMLUI
				auto* result = base->LoadDocumentFromMemory(core::string(HTML));
				if (!result)
					return gui_exception("add document: invalid argument");

				return ielement_document(result);
#else
				return gui_exception("unsupported");
#endif
			}
			ielement_document context::get_document(const std::string_view& id)
			{
#ifdef VI_RMLUI
				return base->GetDocument(core::string(id));
#else
				return ielement_document();
#endif
			}
			ielement_document context::get_document(int index)
			{
#ifdef VI_RMLUI
				return base->GetDocument(index);
#else
				return ielement_document();
#endif
			}
			int context::get_num_documents() const
			{
#ifdef VI_RMLUI
				return base->GetNumDocuments();
#else
				return 0;
#endif
			}
			ielement context::get_element_by_id(const std::string_view& id, int doc_index)
			{
#ifdef VI_RMLUI
				Rml::ElementDocument* root = base->GetDocument(doc_index);
				if (!root)
					return nullptr;

				auto map = elements.find(doc_index);
				if (map == elements.end())
				{
					core::string copy = core::string(id);
					Rml::Element* element = root->GetElementById(copy);
					if (!element)
						return nullptr;

					elements[doc_index].insert(std::make_pair(copy, element));
					return element;
				}

				auto it = map->second.find(core::key_lookup_cast(id));
				if (it != map->second.end())
					return it->second;

				core::string copy = core::string(id);
				Rml::Element* element = root->GetElementById(copy);
				map->second.insert(std::make_pair(copy, element));

				return element;
#else
				return ielement();
#endif
			}
			ielement context::get_hover_element()
			{
#ifdef VI_RMLUI
				return base->GetHoverElement();
#else
				return ielement();
#endif
			}
			ielement context::get_focus_element()
			{
#ifdef VI_RMLUI
				return base->GetFocusElement();
#else
				return ielement();
#endif
			}
			ielement context::get_root_element()
			{
#ifdef VI_RMLUI
				return base->GetRootElement();
#else
				return ielement();
#endif
			}
			ielement context::get_element_at_point(const trigonometry::vector2& point, const ielement& ignore_element, const ielement& element) const
			{
#ifdef VI_RMLUI
				return base->GetElementAtPoint(Rml::Vector2f(point.x, point.y), ignore_element.get_element(), element.get_element());
#else
				return ielement();
#endif
			}
			void context::pull_document_to_front(const ielement_document& schema)
			{
#ifdef VI_RMLUI
				base->PullDocumentToFront(schema.get_element_document());
#endif
			}
			void context::push_document_to_back(const ielement_document& schema)
			{
#ifdef VI_RMLUI
				base->PushDocumentToBack(schema.get_element_document());
#endif
			}
			void context::unfocus_document(const ielement_document& schema)
			{
#ifdef VI_RMLUI
				base->UnfocusDocument(schema.get_element_document());
#endif
			}
			void context::add_event_listener(const std::string_view& event, listener* listener, bool in_capture_phase)
			{
#ifdef VI_RMLUI
				VI_ASSERT(listener != nullptr && listener->base != nullptr, "listener should be valid");
				base->AddEventListener(core::string(event), listener->base, in_capture_phase);
#endif
			}
			void context::remove_event_listener(const std::string_view& event, listener* listener, bool in_capture_phase)
			{
#ifdef VI_RMLUI
				VI_ASSERT(listener != nullptr && listener->base != nullptr, "listener should be valid");
				base->RemoveEventListener(core::string(event), listener->base, in_capture_phase);
#endif
			}
			bool context::is_mouse_interacting() const
			{
#ifdef VI_RMLUI
				return base->IsMouseInteracting();
#else
				return false;
#endif
			}
			bool context::was_input_used(uint32_t input_type_mask)
			{
#ifdef VI_RMLUI
				bool result = false;
				if (input_type_mask & (uint32_t)input_type::keys && inputs.keys)
					result = true;

				if (input_type_mask & (uint32_t)input_type::scroll && inputs.scroll)
					result = true;

				if (input_type_mask & (uint32_t)input_type::text && inputs.text)
					result = true;

				if (input_type_mask & (uint32_t)input_type::cursor && inputs.cursor)
					result = true;

				return result;
#else
				return false;
#endif
			}
			data_model* context::set_data_model(const std::string_view& name)
			{
#ifdef VI_RMLUI
				core::string copy = core::string(name);
				Rml::DataModelConstructor result = base->CreateDataModel(copy);
				if (!result)
					return nullptr;

				data_model* handle = new data_model(&result);
				if (auto type = result.RegisterStruct<data_node>())
				{
					result.RegisterArray<core::vector<data_node>>();
					type.RegisterMember("int", &data_node::get_integer, &data_node::set_value_int);
					type.RegisterMember("num", &data_node::get_double, &data_node::set_value_num);
					type.RegisterMember("str", &data_node::get_string, &data_node::set_value_str);
					type.RegisterMember("at", &data_node::childs);
					type.RegisterMember("all", &data_node::childs);
					type.RegisterMember("size", &data_node::get_value_size);
				}

				models[copy] = handle;
				return handle;
#else
				return nullptr;
#endif
			}
			data_model* context::get_data_model(const std::string_view& name)
			{
				auto it = models.find(core::key_lookup_cast(name));
				if (it != models.end())
					return it->second;

				return nullptr;
			}
			bool context::remove_data_model(const std::string_view& name)
			{
#ifdef VI_RMLUI
				core::string copy = core::string(name);
				if (!base->RemoveDataModel(copy))
					return false;

				auto it = models.find(copy);
				if (it != models.end())
				{
					if (it->second->on_unmount)
						it->second->on_unmount(this);

					core::memory::release(it->second);
					models.erase(it);
				}

				return true;
#else
				return false;
#endif
			}
			bool context::remove_data_models()
			{
#ifdef VI_RMLUI
				if (models.empty())
					return false;

				for (auto item : models)
				{
					base->RemoveDataModel(item.first);
					core::memory::release(item.second);
				}

				models.clear();
				return true;
#else
				return false;
#endif
			}
			void context::set_documents_base_tag(const std::string_view& tag)
			{
#ifdef VI_RMLUI
				base->SetDocumentsBaseTag(core::string(tag));
#endif
			}
			void context::set_mount_callback(model_callback&& callback)
			{
				on_mount = std::move(callback);
			}
			expects_gui_exception<void> context::preprocess(const std::string_view& path, core::string& buffer)
			{
				compute::preprocessor::desc features;
				features.conditions = false;
				features.defines = false;
				features.pragmas = false;

				compute::include_desc desc = compute::include_desc();
				desc.exts.push_back(".html");
				desc.exts.push_back(".htm");

				auto directory = core::os::directory::get_working();
				if (directory)
					desc.root = *directory;

				core::uptr<compute::preprocessor> processor = new compute::preprocessor();
				processor->set_include_options(desc);
				processor->set_features(features);
				processor->set_include_callback([this](compute::preprocessor* processor, const compute::include_result& file, core::string& output) -> compute::expects_preprocessor<compute::include_type>
				{
					if (file.library.empty() || (!file.is_file && !file.is_abstract))
						return compute::include_type::error;

					if (file.is_abstract && !file.is_file)
						return compute::include_type::error;

					auto data = core::os::file::read_as_string(file.library);
					if (!data)
						return compute::include_type::error;

					output.assign(*data);
					this->decompose(output);
					return compute::include_type::preprocess;
				});
#ifdef VI_RMLUI
				processor->set_pragma_callback([](compute::preprocessor* processor, const std::string_view& name, const core::vector<core::string>& args) -> compute::expects_preprocessor<void>
				{
					if (name != "fontface")
						return core::expectation::met;

					core::string path;
					font_weight weight = font_weight::any;
					bool use_as_fallback = false;
					for (auto& item : args)
					{
						if (core::stringify::starts_with(item, "path:"))
						{
							auto value = item.substr(5);
							if ((value.front() == '\"' && value.back() == '\"') || (value.front() == '\'' && value.back() == '\''))
								path = value.substr(1, value.size() - 2);
							else
								path = value;
						}
						else if (core::stringify::starts_with(item, "weight:"))
						{
							auto value = core::var::any(item.substr(7));
							if (value.is(core::var_type::string))
							{
								if (value.is_string("auto"))
									weight = font_weight::any;
								else if (value.is_string("normal"))
									weight = font_weight::normal;
								else if (value.is_string("bold"))
									weight = font_weight::bold;
							}
							else
								weight = (font_weight)std::min<uint16_t>(1000, (uint16_t)value.get_integer());
						}
						else if (core::stringify::starts_with(item, "fallback:"))
							use_as_fallback = core::var::any(item.substr(9)).get_boolean();
					}

					if (path.empty())
						return compute::preprocessor_exception(compute::preprocessor_error::include_error, 0, "font face path is invalid");

					Rml::String target_path;
					Rml::String current_path = core::os::path::get_directory(processor->get_current_file_path().c_str());
					Rml::GetSystemInterface()->JoinPath(target_path, Rml::StringUtilities::Replace(current_path, '|', ':'), Rml::StringUtilities::Replace(path, '|', ':'));
					auto status = context::load_font_face(target_path, use_as_fallback, weight);
					if (status)
						return core::expectation::met;

					return compute::preprocessor_exception(compute::preprocessor_error::include_error, 0, status.error().message());
				});
#endif
				auto status = processor->process(path, buffer);
				if (!status)
					return gui_exception(std::move(status.error().message()));

				return core::expectation::met;
			}
			void context::decompose(core::string& data)
			{
				core::text_settle start, end;
				while (end.end < data.size())
				{
					start = core::stringify::find(data, "<!--", end.end);
					if (!start.found)
						break;

					end = core::stringify::find(data, "-->", start.end);
					if (!end.found)
						break;

					if (!end.start || data[end.start - 1] == '-')
						continue;

					if (start.end > data.size() || data[start.end] == '-')
					{
						end.end = start.end;
						continue;
					}

					core::stringify::remove_part(data, start.start, end.end);
					end.end = start.start;
				}

				memset(&start, 0, sizeof(start));
				memset(&end, 0, sizeof(end));
				while (end.end < data.size())
				{
					start = core::stringify::find(data, "<!---", end.end);
					if (!start.found)
						break;

					end = core::stringify::find(data, "--->", start.end);
					if (!end.found)
						break;

					core::string expression = data.substr(start.end, end.start - start.end);
					core::stringify::trim(expression);
					core::stringify::replace_part(data, start.start, end.end, expression);
					end.end = start.start;
				}
			}
			void context::clear_scope()
			{
				if (!compiler)
					return;

				compiler->clear();
				compiler->prepare("__scope__", true);
			}
			core::string context::resolve_resource_path(const ielement& element, const std::string_view& path)
			{
#ifdef VI_RMLUI
				VI_ASSERT(subsystem::get()->get_system_interface() != nullptr, "system interface should be set");
				auto root_element = element.get_owner_document();
				core::string source_path = core::string(path);
				if (!root_element.is_valid())
					return source_path;

				Rml::String target_path;
				Rml::GetSystemInterface()->JoinPath(target_path, Rml::StringUtilities::Replace(root_element.get_source_url(), '|', ':'), Rml::StringUtilities::Replace(source_path, '|', ':'));
				return target_path;
#else
				return core::string(path);
#endif
			}
			core::string context::get_documents_base_tag()
			{
#ifdef VI_RMLUI
				return base->GetDocumentsBaseTag();
#else
				return core::string();
#endif
			}
			int context::get_key_code(graphics::key_code key)
			{
#ifdef VI_RMLUI
				using namespace Rml::Input;
				switch (key)
				{
					case graphics::key_code::space:
						return KI_SPACE;
					case graphics::key_code::d0:
						return KI_0;
					case graphics::key_code::d1:
						return KI_1;
					case graphics::key_code::d2:
						return KI_2;
					case graphics::key_code::d3:
						return KI_3;
					case graphics::key_code::d4:
						return KI_4;
					case graphics::key_code::d5:
						return KI_5;
					case graphics::key_code::d6:
						return KI_6;
					case graphics::key_code::d7:
						return KI_7;
					case graphics::key_code::d8:
						return KI_8;
					case graphics::key_code::d9:
						return KI_9;
					case graphics::key_code::a:
						return KI_A;
					case graphics::key_code::b:
						return KI_B;
					case graphics::key_code::c:
						return KI_C;
					case graphics::key_code::d:
						return KI_D;
					case graphics::key_code::e:
						return KI_E;
					case graphics::key_code::f:
						return KI_F;
					case graphics::key_code::g:
						return KI_G;
					case graphics::key_code::h:
						return KI_H;
					case graphics::key_code::i:
						return KI_I;
					case graphics::key_code::j:
						return KI_J;
					case graphics::key_code::k:
						return KI_K;
					case graphics::key_code::l:
						return KI_L;
					case graphics::key_code::m:
						return KI_M;
					case graphics::key_code::n:
						return KI_N;
					case graphics::key_code::o:
						return KI_O;
					case graphics::key_code::p:
						return KI_P;
					case graphics::key_code::q:
						return KI_Q;
					case graphics::key_code::r:
						return KI_R;
					case graphics::key_code::s:
						return KI_S;
					case graphics::key_code::t:
						return KI_T;
					case graphics::key_code::u:
						return KI_U;
					case graphics::key_code::v:
						return KI_V;
					case graphics::key_code::w:
						return KI_W;
					case graphics::key_code::x:
						return KI_X;
					case graphics::key_code::y:
						return KI_Y;
					case graphics::key_code::z:
						return KI_Z;
					case graphics::key_code::semicolon:
						return KI_OEM_1;
					case graphics::key_code::comma:
						return KI_OEM_COMMA;
					case graphics::key_code::minus:
						return KI_OEM_MINUS;
					case graphics::key_code::period:
						return KI_OEM_PERIOD;
					case graphics::key_code::slash:
						return KI_OEM_2;
					case graphics::key_code::left_bracket:
						return KI_OEM_4;
					case graphics::key_code::backslash:
						return KI_OEM_5;
					case graphics::key_code::right_bracket:
						return KI_OEM_6;
					case graphics::key_code::kp0:
						return KI_NUMPAD0;
					case graphics::key_code::kp1:
						return KI_NUMPAD1;
					case graphics::key_code::kp2:
						return KI_NUMPAD2;
					case graphics::key_code::kp3:
						return KI_NUMPAD3;
					case graphics::key_code::kp4:
						return KI_NUMPAD4;
					case graphics::key_code::kp5:
						return KI_NUMPAD5;
					case graphics::key_code::kp6:
						return KI_NUMPAD6;
					case graphics::key_code::kp7:
						return KI_NUMPAD7;
					case graphics::key_code::kp8:
						return KI_NUMPAD8;
					case graphics::key_code::kp9:
						return KI_NUMPAD9;
					case graphics::key_code::kp_enter:
						return KI_NUMPADENTER;
					case graphics::key_code::kp_multiply:
						return KI_MULTIPLY;
					case graphics::key_code::kp_plus:
						return KI_ADD;
					case graphics::key_code::kp_minus:
						return KI_SUBTRACT;
					case graphics::key_code::kp_period:
						return KI_DECIMAL;
					case graphics::key_code::kp_divide:
						return KI_DIVIDE;
					case graphics::key_code::kp_equals:
						return KI_OEM_NEC_EQUAL;
					case graphics::key_code::backspace:
						return KI_BACK;
					case graphics::key_code::tab:
						return KI_TAB;
					case graphics::key_code::clear:
						return KI_CLEAR;
					case graphics::key_code::defer:
						return KI_RETURN;
					case graphics::key_code::pause:
						return KI_PAUSE;
					case graphics::key_code::capslock:
						return KI_CAPITAL;
					case graphics::key_code::page_up:
						return KI_PRIOR;
					case graphics::key_code::page_down:
						return KI_NEXT;
					case graphics::key_code::end:
						return KI_END;
					case graphics::key_code::home:
						return KI_HOME;
					case graphics::key_code::left:
						return KI_LEFT;
					case graphics::key_code::up:
						return KI_UP;
					case graphics::key_code::right:
						return KI_RIGHT;
					case graphics::key_code::down:
						return KI_DOWN;
					case graphics::key_code::insert:
						return KI_INSERT;
					case graphics::key_code::deinit:
						return KI_DELETE;
					case graphics::key_code::help:
						return KI_HELP;
					case graphics::key_code::f1:
						return KI_F1;
					case graphics::key_code::f2:
						return KI_F2;
					case graphics::key_code::f3:
						return KI_F3;
					case graphics::key_code::f4:
						return KI_F4;
					case graphics::key_code::f5:
						return KI_F5;
					case graphics::key_code::f6:
						return KI_F6;
					case graphics::key_code::f7:
						return KI_F7;
					case graphics::key_code::f8:
						return KI_F8;
					case graphics::key_code::f9:
						return KI_F9;
					case graphics::key_code::f10:
						return KI_F10;
					case graphics::key_code::f11:
						return KI_F11;
					case graphics::key_code::f12:
						return KI_F12;
					case graphics::key_code::f13:
						return KI_F13;
					case graphics::key_code::f14:
						return KI_F14;
					case graphics::key_code::f15:
						return KI_F15;
					case graphics::key_code::num_lock_clear:
						return KI_NUMLOCK;
					case graphics::key_code::scroll_lock:
						return KI_SCROLL;
					case graphics::key_code::left_shift:
						return KI_LSHIFT;
					case graphics::key_code::right_shift:
						return KI_RSHIFT;
					case graphics::key_code::left_control:
						return KI_LCONTROL;
					case graphics::key_code::right_control:
						return KI_RCONTROL;
					case graphics::key_code::left_alt:
						return KI_LMENU;
					case graphics::key_code::right_alt:
						return KI_RMENU;
					case graphics::key_code::left_gui:
						return KI_LMETA;
					case graphics::key_code::right_gui:
						return KI_RMETA;
					default:
						return KI_UNKNOWN;
				}
#else
				return 0;
#endif
			}
			int context::get_key_mod(graphics::key_mod mod)
			{
#ifdef VI_RMLUI
				int result = 0;
				if ((size_t)mod & (size_t)graphics::key_mod::control)
					result |= Rml::Input::KM_CTRL;

				if ((size_t)mod & (size_t)graphics::key_mod::shift)
					result |= Rml::Input::KM_SHIFT;

				if ((size_t)mod & (size_t)graphics::key_mod::alt)
					result |= Rml::Input::KM_ALT;

				return result;
#else
				return 0;
#endif
			}
		}
	}
}
#pragma warning(pop)