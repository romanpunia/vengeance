#include "../gui.h"
#ifdef VI_RMLUI
#include <RmlUi/core.h>
#include <source/core/TransformState.h>

namespace vitex
{
	namespace layer
	{
		namespace gui
		{
			static class box_shadow_instancer* ibox_shadow = nullptr;
			static class box_blur_instancer* ibox_blur = nullptr;

			class decorator_utils
			{
			public:
				static void set_world_view_projection(render_constants* constants, Rml::Element* element, const Rml::Vector2f& position, const Rml::Vector2f& size, const trigonometry::vector2& mul = 1.0f)
				{
					VI_ASSERT(constants != nullptr, "graphics device should be set");
					VI_ASSERT(element != nullptr, "element should be set");

					trigonometry::vector3 scale(size.x / 2.0f, size.y / 2.0f);
					trigonometry::vector3 offset(position.x + scale.x, position.y + scale.y);
					trigonometry::matrix4x4& ortho = *subsystem::get()->get_projection();

					const Rml::TransformState* state = element->GetTransformState();
					if (state != nullptr && state->GetTransform() != nullptr)
					{
						trigonometry::matrix4x4 view = utils::to_matrix(state->GetTransform());
						constants->render.transform = trigonometry::matrix4x4::create_translated_scale(offset, scale + mul) * view * ortho;
						constants->render.world = (trigonometry::matrix4x4::create_translation(trigonometry::vector2(position.x, position.y)) * view * ortho).inv();
					}
					else
					{
						constants->render.transform = trigonometry::matrix4x4::create_translated_scale(offset, scale + mul) * ortho;
						constants->render.world = (trigonometry::matrix4x4::create_translation(trigonometry::vector2(position.x, position.y)) * ortho).inv();
					}
				}
			};

			class box_shadow_instancer final : public Rml::DecoratorInstancer
			{
			public:
				struct render_constant
				{
					trigonometry::vector4 color;
					trigonometry::vector4 radius;
					trigonometry::vector2 size;
					trigonometry::vector2 position;
					trigonometry::vector2 offset;
					float softness = 0.0f;
					float padding = 0.0f;
				} render_pass;

				struct
				{
					uint32_t object_buffer = (uint32_t)-1;
					uint32_t ui_buffer = (uint32_t)-1;
				} slots;

			public:
				Rml::PropertyId color;
				Rml::PropertyId offset_x;
				Rml::PropertyId offset_y;
				Rml::PropertyId softness;

			public:
				graphics::element_buffer* vertex_buffer;
				graphics::shader* shader;
				graphics::graphics_device* device;
				render_constants* constants;

			public:
				box_shadow_instancer(render_constants* new_constants);
				virtual ~box_shadow_instancer() override;
				Rml::SharedPtr<Rml::Decorator> InstanceDecorator(const Rml::String& name, const Rml::PropertyDictionary& props, const Rml::DecoratorInstancerInterface& interfacef) override;
			};

			class box_blur_instancer final : public Rml::DecoratorInstancer
			{
			public:
				struct render_constant
				{
					trigonometry::vector4 color;
					trigonometry::vector4 radius;
					trigonometry::vector2 texel;
					trigonometry::vector2 size;
					trigonometry::vector2 position;
					float softness = 0.0f;
					float alpha = 1.0f;
				} render_pass;

				struct
				{
					uint32_t diffuse_map = (uint32_t)-1;
					uint32_t object_buffer = (uint32_t)-1;
					uint32_t ui_buffer = (uint32_t)-1;
				} slots;

			public:
				Rml::PropertyId color;
				Rml::PropertyId softness;

			public:
				graphics::texture_2d* background;
				graphics::element_buffer* vertex_buffer;
				graphics::sampler_state* sampler;
				graphics::shader* shader;
				graphics::graphics_device* device;
				render_constants* constants;

			public:
				box_blur_instancer(render_constants* new_constants);
				virtual ~box_blur_instancer() override;
				Rml::SharedPtr<Rml::Decorator> InstanceDecorator(const Rml::String& name, const Rml::PropertyDictionary& props, const Rml::DecoratorInstancerInterface& interfacef) override;
			};

			class box_shadow final : public Rml::Decorator
			{
			private:
				trigonometry::vector4 color;
				trigonometry::vector2 offset;
				float softness;

			public:
				box_shadow(const trigonometry::vector4& new_color, const trigonometry::vector2& new_offset, float new_softness) : color(new_color.div(255.0f)), offset(new_offset), softness(new_softness)
				{
				}
				virtual ~box_shadow() = default;
				Rml::DecoratorDataHandle GenerateElementData(Rml::Element* element, Rml::BoxArea paint_area) const override
				{
					return 1;
				}
				void ReleaseElementData(Rml::DecoratorDataHandle element_data) const override
				{
				}
				void RenderElement(Rml::Element* element, Rml::DecoratorDataHandle element_data) const override
				{
					VI_ASSERT(element != nullptr, "element should be set");
					Rml::Vector2f position = element->GetAbsoluteOffset(Rml::BoxArea::Padding).Round();
					Rml::Vector2f size = element->GetBox().GetSize(Rml::BoxArea::Padding).Round();
					float alpha = element->GetProperty<float>("opacity");
					float radius = softness * 0.85f;

					graphics::graphics_device* device = ibox_shadow->device;
					ibox_shadow->render_pass.position = trigonometry::vector2(position.x, position.y);
					ibox_shadow->render_pass.size = trigonometry::vector2(size.x, size.y);
					ibox_shadow->render_pass.color = color;
					ibox_shadow->render_pass.color.w *= alpha;
					ibox_shadow->render_pass.offset = offset;
					ibox_shadow->render_pass.softness = softness;
					ibox_shadow->render_pass.radius.x = element->GetProperty<float>("border-bottom-left-radius");
					ibox_shadow->render_pass.radius.y = element->GetProperty<float>("border-bottom-right-radius");
					ibox_shadow->render_pass.radius.z = element->GetProperty<float>("border-top-right-radius");
					ibox_shadow->render_pass.radius.w = element->GetProperty<float>("border-top-left-radius");

					render_constants* constants = ibox_shadow->constants;
					decorator_utils::set_world_view_projection(constants, element, position, size, offset.abs() + radius + 4096.0f);
					constants->set_updated_constant_buffer(render_buffer_type::render, ibox_shadow->slots.object_buffer, VI_VS | VI_PS);
					device->set_shader(ibox_shadow->shader, VI_VS | VI_PS);
					device->set_buffer(ibox_shadow->shader, ibox_shadow->slots.ui_buffer, VI_PS);
					device->set_vertex_buffer(ibox_shadow->vertex_buffer);
					device->update_buffer(ibox_shadow->shader, &ibox_shadow->render_pass);
					device->draw((unsigned int)ibox_shadow->vertex_buffer->get_elements(), 0);
				}
			};

			class box_blur final : public Rml::Decorator
			{
			private:
				trigonometry::vector4 color;
				float softness;

			public:
				box_blur(const trigonometry::vector4& new_color, float new_softness) : color(new_color.div(255)), softness(new_softness)
				{
				}
				virtual ~box_blur() = default;
				Rml::DecoratorDataHandle GenerateElementData(Rml::Element* element, Rml::BoxArea paint_area) const override
				{
					return 1;
				}
				void ReleaseElementData(Rml::DecoratorDataHandle element_data) const override
				{
				}
				void RenderElement(Rml::Element* element, Rml::DecoratorDataHandle element_data) const override
				{
					VI_ASSERT(element != nullptr, "element should be set");
					graphics::texture_2d* background = subsystem::get()->get_background();
					if (!background)
						return;

					Rml::Vector2i screen = element->GetContext()->GetDimensions();
					Rml::Vector2f position = element->GetAbsoluteOffset(Rml::BoxArea::Padding).Round();
					Rml::Vector2f size = element->GetBox().GetSize(Rml::BoxArea::Padding).Round();
					float alpha = element->GetProperty<float>("opacity");

					graphics::graphics_device* device = ibox_blur->device;
					ibox_blur->render_pass.color = color;
					ibox_blur->render_pass.texel = trigonometry::vector2((float)screen.x, (float)screen.y);
					ibox_blur->render_pass.position = trigonometry::vector2(position.x, position.y);
					ibox_blur->render_pass.size = trigonometry::vector2(size.x, size.y);
					ibox_blur->render_pass.softness = softness;
					ibox_blur->render_pass.alpha = alpha;
					ibox_blur->render_pass.radius.x = element->GetProperty<float>("border-bottom-left-radius");
					ibox_blur->render_pass.radius.y = element->GetProperty<float>("border-bottom-right-radius");
					ibox_blur->render_pass.radius.z = element->GetProperty<float>("border-top-right-radius");
					ibox_blur->render_pass.radius.w = element->GetProperty<float>("border-top-left-radius");

					render_constants* constants = ibox_blur->constants;
					decorator_utils::set_world_view_projection(constants, element, position, size);
					constants->set_updated_constant_buffer(render_buffer_type::render, ibox_blur->slots.object_buffer, VI_VS | VI_PS);
					device->copy_texture_2d(background, &ibox_blur->background);
					device->set_texture_2d(ibox_blur->background, ibox_blur->slots.diffuse_map, VI_PS);
					device->set_shader(ibox_blur->shader, VI_VS | VI_PS);
					device->set_buffer(ibox_blur->shader, ibox_blur->slots.ui_buffer, VI_PS);
					device->set_vertex_buffer(ibox_blur->vertex_buffer);
					device->update_buffer(ibox_blur->shader, &ibox_blur->render_pass);
					device->draw((unsigned int)ibox_blur->vertex_buffer->get_elements(), 0);
				}
			};

			box_shadow_instancer::box_shadow_instancer(render_constants* new_constants) : shader(nullptr), device(nullptr), constants(new_constants)
			{
				VI_ASSERT(constants != nullptr, "render constants should be set");
				VI_ASSERT(constants->get_device() != nullptr, "graphics device should be set");
				device = constants->get_device();

				graphics::shader::desc i = graphics::shader::desc();
				if (device->get_section_data("materials/material_ui_box_shadow", &i))
				{
					shader = *device->create_shader(i);
					slots.object_buffer = *device->get_shader_slot(shader, "ObjectBuffer");
					slots.ui_buffer = *device->get_shader_slot(shader, "UiBuffer");
					device->update_buffer_size(shader, sizeof(render_pass));
				}

				Rml::Vertex elements[6];
				elements[0] = { Rml::Vector2f(-1.0f, -1.0f), Rml::ColourbPremultiplied(0, 0, 0, 0), Rml::Vector2f(-1, 0) };
				elements[1] = { Rml::Vector2f(-1.0f, 1.0f), Rml::ColourbPremultiplied(0, 0, 0, 0), Rml::Vector2f(-1, 1) };
				elements[2] = { Rml::Vector2f(1.0f, 1.0f), Rml::ColourbPremultiplied(0, 0, 0, 0), Rml::Vector2f(0, 1) };
				elements[3] = { Rml::Vector2f(-1.0f, -1.0f), Rml::ColourbPremultiplied(0, 0, 0, 0), Rml::Vector2f(-1, 0) };
				elements[4] = { Rml::Vector2f(1.0f, 1.0f), Rml::ColourbPremultiplied(0, 0, 0, 0), Rml::Vector2f(0, 1) };
				elements[5] = { Rml::Vector2f(1.0f, -1.0f), Rml::ColourbPremultiplied(0, 0, 0, 0), Rml::Vector2f(0, 0) };

				graphics::element_buffer::desc f = graphics::element_buffer::desc();
				f.access_flags = graphics::cpu_access::none;
				f.usage = graphics::resource_usage::defaults;
				f.bind_flags = graphics::resource_bind::vertex_buffer;
				f.element_count = 6;
				f.element_width = sizeof(Rml::Vertex);
				f.elements = &elements[0];

				vertex_buffer = *device->create_element_buffer(f);
				color = RegisterProperty("color", "#000").AddParser("color").GetId();
				softness = RegisterProperty("softness", "60").AddParser("number").GetId();
				offset_x = RegisterProperty("x", "0").AddParser("number").GetId();
				offset_y = RegisterProperty("y", "10").AddParser("number").GetId();
				RegisterShorthand("decorator", "x, y, softness, color", Rml::ShorthandType::FallThrough);
			}
			box_shadow_instancer::~box_shadow_instancer()
			{
				core::memory::release(shader);
				core::memory::release(vertex_buffer);
			}
			Rml::SharedPtr<Rml::Decorator> box_shadow_instancer::InstanceDecorator(const Rml::String& name, const Rml::PropertyDictionary& props, const Rml::DecoratorInstancerInterface& interfacef)
			{
				const Rml::Property* scolor = props.GetProperty(color);
				const Rml::Property* ssoftness = props.GetProperty(softness);
				const Rml::Property* soffset_x = props.GetProperty(offset_x);
				const Rml::Property* soffset_y = props.GetProperty(offset_y);

				Rml::Colourb icolor = scolor->Get<Rml::Colourb>();
				float isoftness = ssoftness->Get<float>();
				float ioffset_x = soffset_x->Get<float>();
				float ioffset_y = soffset_y->Get<float>();

				return Rml::MakeShared<box_shadow>(
					trigonometry::vector4(icolor.red, icolor.green, icolor.blue, icolor.alpha),
					trigonometry::vector2(ioffset_x, ioffset_y), isoftness);
			}

			box_blur_instancer::box_blur_instancer(render_constants* new_constants) : background(nullptr), shader(nullptr), device(nullptr), constants(new_constants)
			{
				VI_ASSERT(constants != nullptr, "render constants should be set");
				VI_ASSERT(constants->get_device() != nullptr, "graphics device should be set");
				device = constants->get_device();

				graphics::shader::desc i = graphics::shader::desc();
				if (device->get_section_data("materials/material_ui_box_blur", &i))
				{
					shader = *device->create_shader(i);
					slots.diffuse_map = *device->get_shader_slot(shader, "DiffuseMap");
					slots.object_buffer = *device->get_shader_slot(shader, "ObjectBuffer");
					slots.ui_buffer = *device->get_shader_slot(shader, "UiBuffer");
					device->update_buffer_size(shader, sizeof(render_pass));
				}

				Rml::Vertex elements[6];
				elements[0] = { Rml::Vector2f(-1.0f, -1.0f), Rml::ColourbPremultiplied(0, 0, 0, 0), Rml::Vector2f(-1, 0) };
				elements[1] = { Rml::Vector2f(-1.0f, 1.0f), Rml::ColourbPremultiplied(0, 0, 0, 0), Rml::Vector2f(-1, 1) };
				elements[2] = { Rml::Vector2f(1.0f, 1.0f), Rml::ColourbPremultiplied(0, 0, 0, 0), Rml::Vector2f(0, 1) };
				elements[3] = { Rml::Vector2f(-1.0f, -1.0f), Rml::ColourbPremultiplied(0, 0, 0, 0), Rml::Vector2f(-1, 0) };
				elements[4] = { Rml::Vector2f(1.0f, 1.0f), Rml::ColourbPremultiplied(0, 0, 0, 0), Rml::Vector2f(0, 1) };
				elements[5] = { Rml::Vector2f(1.0f, -1.0f), Rml::ColourbPremultiplied(0, 0, 0, 0), Rml::Vector2f(0, 0) };

				graphics::element_buffer::desc f = graphics::element_buffer::desc();
				f.access_flags = graphics::cpu_access::none;
				f.usage = graphics::resource_usage::defaults;
				f.bind_flags = graphics::resource_bind::vertex_buffer;
				f.element_count = 6;
				f.element_width = sizeof(Rml::Vertex);
				f.elements = &elements[0];

				vertex_buffer = *device->create_element_buffer(f);
				sampler = device->get_sampler_state("a16_fa_wrap");
				color = RegisterProperty("color", "#fff").AddParser("color").GetId();
				softness = RegisterProperty("softness", "8").AddParser("number").GetId();
				RegisterShorthand("decorator", "softness, color", Rml::ShorthandType::FallThrough);
			}
			box_blur_instancer::~box_blur_instancer()
			{
				core::memory::release(shader);
				core::memory::release(vertex_buffer);
				core::memory::release(background);
			}
			Rml::SharedPtr<Rml::Decorator> box_blur_instancer::InstanceDecorator(const Rml::String& name, const Rml::PropertyDictionary& props, const Rml::DecoratorInstancerInterface& interfacef)
			{
				const Rml::Property* scolor = props.GetProperty(color);
				const Rml::Property* ssoftness = props.GetProperty(softness);

				Rml::Colourb icolor = scolor->Get<Rml::Colourb>();
				float isoftness = ssoftness->Get<float>();

				return Rml::MakeShared<box_blur>(trigonometry::vector4(icolor.red, icolor.green, icolor.blue, icolor.alpha), isoftness);
			}

			void subsystem::resize_decorators(int width, int height) noexcept
			{
				if (ibox_blur != nullptr)
					core::memory::release(ibox_blur->background);
			}
			void subsystem::create_decorators(render_constants* constants) noexcept
			{
				VI_ASSERT(constants != nullptr, "render constants should be set");
				release_decorators();

				ibox_shadow = core::memory::init<box_shadow_instancer>(constants);
				Rml::Factory::RegisterDecoratorInstancer("box-shadow", ibox_shadow);

				ibox_blur = core::memory::init<box_blur_instancer>(constants);
				Rml::Factory::RegisterDecoratorInstancer("box-blur", ibox_blur);
			}
			void subsystem::release_decorators() noexcept
			{
				core::memory::deinit(ibox_shadow);
				ibox_shadow = nullptr;

				core::memory::deinit(ibox_blur);
				ibox_blur = nullptr;
			}
		}
	}
}
#endif
