#ifndef VI_LAYER_RENDERERS_H
#define VI_LAYER_RENDERERS_H
#include "../layer.h"
#include "components.h"
#include "gui.h"

namespace vitex
{
	namespace layer
	{
		namespace renderers
		{
			class soft_body final : public geometry_renderer<components::soft_body>
			{
			private:
				struct
				{
					struct
					{
						graphics::shader* shader = nullptr;
						material::slots slotdata;
						uint32_t sampler = (uint32_t)-1;
						uint32_t materials = (uint32_t)-1;
					} depth;
					struct
					{
						graphics::shader* shader = nullptr;
						material::slots slotdata;
						uint32_t sampler = (uint32_t)-1;
						uint32_t materials = (uint32_t)-1;
						uint32_t viewer_buffer = (uint32_t)-1;
						uint32_t cube_buffer = (uint32_t)-1;
					} depth_cube;
					struct
					{
						graphics::shader* shader = nullptr;
						uint32_t object_buffer = (uint32_t)-1;
					} culling;
					struct
					{
						graphics::shader* shader = nullptr;
						material::slots slotdata;
						uint32_t sampler = (uint32_t)-1;
						uint32_t materials = (uint32_t)-1;
						uint32_t viewer_buffer = (uint32_t)-1;
					} geometry;
				} pipelines;

			private:
				graphics::depth_stencil_state* depth_stencil = nullptr;
				graphics::rasterizer_state* rasterizer = nullptr;
				graphics::blend_state* blend = nullptr;
				graphics::sampler_state* sampler = nullptr;
				graphics::element_buffer* vertex_buffer = nullptr;
				graphics::element_buffer* index_buffer = nullptr;
				graphics::element_buffer* group_buffer = nullptr;
				graphics::input_layout* layout[2];
				render_buffer::instance group;

			public:
				soft_body(render_system* lab);
				~soft_body();
				size_t cull_geometry(const viewer& view, const geometry_renderer::objects& chunk) override;
				size_t render_geometry(core::timer* time, const geometry_renderer::objects& chunk) override;
				size_t render_depth(core::timer* time, const geometry_renderer::objects& chunk) override;
				size_t render_depth_cube(core::timer* time, const geometry_renderer::objects& chunk, trigonometry::matrix4x4* view_projection) override;

			public:
				VI_COMPONENT("soft_body_renderer");
			};

			class model final : public geometry_renderer<components::model, graphics::mesh_buffer, render_buffer::instance>
			{
			private:
				struct
				{
					struct
					{
						graphics::shader* shader = nullptr;
						material::slots slotdata;
						uint32_t sampler = (uint32_t)-1;
						uint32_t materials = (uint32_t)-1;
					} depth;
					struct
					{
						graphics::shader* shader = nullptr;
						material::slots slotdata;
						uint32_t sampler = (uint32_t)-1;
						uint32_t materials = (uint32_t)-1;
						uint32_t viewer_buffer = (uint32_t)-1;
						uint32_t cube_buffer = (uint32_t)-1;
					} depth_cube;
					struct
					{
						graphics::shader* shader = nullptr;
						uint32_t object_buffer = (uint32_t)-1;
					} culling;
					struct
					{
						graphics::shader* shader = nullptr;
						material::slots slotdata;
						uint32_t sampler = (uint32_t)-1;
						uint32_t materials = (uint32_t)-1;
						uint32_t viewer_buffer = (uint32_t)-1;
					} geometry;
				} pipelines;

			private:
				graphics::depth_stencil_state* depth_stencil = nullptr;
				graphics::rasterizer_state* back_rasterizer = nullptr;
				graphics::rasterizer_state* front_rasterizer = nullptr;
				graphics::blend_state* blend = nullptr;
				graphics::sampler_state* sampler = nullptr;
				graphics::input_layout* layout[2];

			public:
				model(render_system* lab);
				~model() override;
				void batch_geometry(components::model* base, batching& batch, size_t chunk) override;
				size_t cull_geometry(const viewer& view, const geometry_renderer::objects& chunk) override;
				size_t render_geometry_batched(core::timer* time, const geometry_renderer::groups& chunk) override;
				size_t render_depth_batched(core::timer* time, const geometry_renderer::groups& chunk) override;
				size_t render_depth_cube_batched(core::timer* time, const geometry_renderer::groups& chunk, trigonometry::matrix4x4* view_projection) override;

			private:
				layer::model* get_drawable(components::model* base);

			public:
				VI_COMPONENT("model_renderer");
			};

			class skin final : public geometry_renderer<components::skin>
			{
			private:
				struct
				{
					struct
					{
						graphics::shader* shader = nullptr;
						material::slots slotdata;
						uint32_t sampler = (uint32_t)-1;
						uint32_t materials = (uint32_t)-1;
						uint32_t animation_buffer = (uint32_t)-1;
						uint32_t object_buffer = (uint32_t)-1;
					} depth;
					struct
					{
						graphics::shader* shader = nullptr;
						material::slots slotdata;
						uint32_t sampler = (uint32_t)-1;
						uint32_t materials = (uint32_t)-1;
						uint32_t animation_buffer = (uint32_t)-1;
						uint32_t object_buffer = (uint32_t)-1;
						uint32_t viewer_buffer = (uint32_t)-1;
						uint32_t cube_buffer = (uint32_t)-1;
					} depth_cube;
					struct
					{
						graphics::shader* shader = nullptr;
						uint32_t animation_buffer = (uint32_t)-1;
						uint32_t object_buffer = (uint32_t)-1;
					} culling;
					struct
					{
						graphics::shader* shader = nullptr;
						material::slots slotdata;
						uint32_t sampler = (uint32_t)-1;
						uint32_t materials = (uint32_t)-1;
						uint32_t animation_buffer = (uint32_t)-1;
						uint32_t object_buffer = (uint32_t)-1;
						uint32_t viewer_buffer = (uint32_t)-1;
					} geometry;
				} pipelines;

			private:
				graphics::depth_stencil_state* depth_stencil = nullptr;
				graphics::rasterizer_state* back_rasterizer = nullptr;
				graphics::rasterizer_state* front_rasterizer = nullptr;
				graphics::blend_state* blend = nullptr;
				graphics::sampler_state* sampler = nullptr;
				graphics::input_layout* layout = nullptr;

			public:
				skin(render_system* lab);
				~skin();
				size_t cull_geometry(const viewer& view, const geometry_renderer::objects& chunk) override;
				size_t render_geometry(core::timer* time, const geometry_renderer::objects& chunk) override;
				size_t render_depth(core::timer* time, const geometry_renderer::objects& chunk) override;
				size_t render_depth_cube(core::timer* time, const geometry_renderer::objects& chunk, trigonometry::matrix4x4* view_projection) override;

			private:
				layer::skin_model* get_drawable(components::skin* base);

			public:
				VI_COMPONENT("skin_renderer");
			};

			class emitter final : public geometry_renderer<components::emitter>
			{
			private:
				struct
				{
					struct
					{
						graphics::shader* shader = nullptr;
						material::slots slotdata;
						uint32_t sampler = (uint32_t)-1;
						uint32_t materials = (uint32_t)-1;
						uint32_t object_buffer = (uint32_t)-1;
						uint32_t elements = (uint32_t)-1;
					} depth;
					struct
					{
						graphics::shader* shader = nullptr;
						material::slots slotdata;
						uint32_t sampler = (uint32_t)-1;
						uint32_t materials = (uint32_t)-1;
						uint32_t object_buffer = (uint32_t)-1;
						uint32_t viewer_buffer = (uint32_t)-1;
						uint32_t quad = (uint32_t)-1;
						uint32_t elements = (uint32_t)-1;
					} depth_point;
					struct
					{
						graphics::shader* shader = nullptr;
						material::slots slotdata;
						uint32_t sampler = (uint32_t)-1;
						uint32_t materials = (uint32_t)-1;
						uint32_t object_buffer = (uint32_t)-1;
						uint32_t viewer_buffer = (uint32_t)-1;
						uint32_t quad = (uint32_t)-1;
						uint32_t elements = (uint32_t)-1;
					} depth_quad;
					struct
					{
						graphics::shader* shader = nullptr;
						material::slots slotdata;
						uint32_t sampler = (uint32_t)-1;
						uint32_t materials = (uint32_t)-1;
						uint32_t object_buffer = (uint32_t)-1;
						uint32_t elements = (uint32_t)-1;
					} geometry_opaque;
					struct
					{
						graphics::shader* shader = nullptr;
						material::slots slotdata;
						uint32_t sampler = (uint32_t)-1;
						uint32_t materials = (uint32_t)-1;
						uint32_t object_buffer = (uint32_t)-1;
						uint32_t elements = (uint32_t)-1;
					} geometry_transparent;
				} pipelines;

				struct
				{
					trigonometry::matrix4x4 face_view[6];
				} quad;

			private:
				graphics::depth_stencil_state* depth_stencil_opaque = nullptr;
				graphics::depth_stencil_state* depth_stencil_additive = nullptr;
				graphics::rasterizer_state* rasterizer = nullptr;
				graphics::blend_state* additive_blend = nullptr;
				graphics::blend_state* overwrite_blend = nullptr;
				graphics::sampler_state* sampler = nullptr;
				graphics::input_layout* layout = nullptr;

			public:
				emitter(render_system* lab);
				~emitter() override;
				size_t render_geometry(core::timer* time, const geometry_renderer::objects& chunk) override;
				size_t render_depth(core::timer* time, const geometry_renderer::objects& chunk) override;
				size_t render_depth_cube(core::timer* time, const geometry_renderer::objects& chunk, trigonometry::matrix4x4* view_projection) override;

			public:
				VI_COMPONENT("emitter_renderer");
			};

			class decal final : public geometry_renderer<components::decal>
			{
			private:
				struct
				{
					struct
					{
						graphics::shader* shader = nullptr;
						material::slots slotdata;
						uint32_t sampler = (uint32_t)-1;
						uint32_t materials = (uint32_t)-1;
						uint32_t object_buffer = (uint32_t)-1;
						uint32_t depth_map = (uint32_t)-1;
					} geometry;
				} pipelines;

			private:
				graphics::depth_stencil_state* depth_stencil = nullptr;
				graphics::rasterizer_state* rasterizer = nullptr;
				graphics::blend_state* blend = nullptr;
				graphics::sampler_state* sampler = nullptr;
				graphics::input_layout* layout = nullptr;

			public:
				decal(render_system* lab);
				~decal() override;
				size_t render_geometry(core::timer* time, const geometry_renderer::objects& chunk) override;

			public:
				VI_COMPONENT("decal_renderer");
			};

			class lighting final : public renderer
			{
			public:
				struct isurface_buffer
				{
					trigonometry::matrix4x4 transform;
					trigonometry::vector3 position;
					float range = 0.0f;
					trigonometry::vector3 lighting;
					float mips = 0.0f;
					trigonometry::vector3 scale;
					float parallax = 0.0f;
					trigonometry::vector3 attenuation;
					float infinity = 0.0f;
				};

				struct ipoint_buffer
				{
					trigonometry::matrix4x4 transform;
					trigonometry::vector4 attenuation;
					trigonometry::vector3 position;
					float range = 0.0f;
					trigonometry::vector3 lighting;
					float distance = 0.0f;
					float umbra = 0.0f;
					float softness = 0.0f;
					float bias = 0.0f;
					float iterations = 0.0f;
				};

				struct ispot_buffer
				{
					trigonometry::matrix4x4 transform;
					trigonometry::matrix4x4 view_projection;
					trigonometry::vector4 attenuation;
					trigonometry::vector3 direction;
					float cutoff = 0.0f;
					trigonometry::vector3 position;
					float range = 0.0f;
					trigonometry::vector3 lighting;
					float softness = 0.0f;
					float bias = 0.0f;
					float iterations = 0.0f;
					float umbra = 0.0f;
					float padding = 0.0f;
				};

				struct iline_buffer
				{
					trigonometry::matrix4x4 view_projection[6];
					trigonometry::matrix4x4 sky_offset;
					trigonometry::vector3 rlh_emission;
					float rlh_height = 0.0f;
					trigonometry::vector3 mie_emission;
					float mie_height = 0.0f;
					trigonometry::vector3 lighting;
					float softness = 0.0f;
					trigonometry::vector3 position;
					float cascades = 0.0f;
					float padding = 0.0f;
					float umbra = 0.0f;
					float bias = 0.0f;
					float iterations = 0.0f;
					float scatter_intensity = 0.0f;
					float planet_radius = 0.0f;
					float atmosphere_radius = 0.0f;
					float mie_direction = 0.0f;
				};

				struct iambient_buffer
				{
					trigonometry::matrix4x4 sky_offset;
					trigonometry::vector3 high_emission = 0.028f;
					float sky_emission = 0.0f;
					trigonometry::vector3 low_emission = 0.016f;
					float light_emission = 1.0f;
					trigonometry::vector3 sky_color = 1.0f;
					float fog_far_off = 0.1f;
					trigonometry::vector3 fog_color = { 0.5f, 0.6f, 0.7f };
					float fog_near_off = 0.1f;
					trigonometry::vector3 fog_far = 0.125f;
					float fog_amount = 0.0f;
					trigonometry::vector3 fog_near = 0.125f;
					float recursive = 1.0f;
				};

			protected:
				struct
				{
					struct
					{
						graphics::shader* shader = nullptr;
						uint32_t materials = (uint32_t)-1;
						uint32_t sampler = (uint32_t)-1;
						uint32_t viewer_buffer = (uint32_t)-1;
						uint32_t ambient_buffer = (uint32_t)-1;
						uint32_t diffuse_buffer = (uint32_t)-1;
						uint32_t normal_buffer = (uint32_t)-1;
						uint32_t depth_buffer = (uint32_t)-1;
						uint32_t surface_buffer = (uint32_t)-1;
						uint32_t light_map = (uint32_t)-1;
						uint32_t sky_map = (uint32_t)-1;
					} ambient;
					struct
					{
						graphics::shader* shader_base = nullptr;
						graphics::shader* shader_shadowed = nullptr;
						uint32_t materials = (uint32_t)-1;
						uint32_t sampler = (uint32_t)-1;
						uint32_t depth_sampler = (uint32_t)-1;
						uint32_t depth_less_sampler = (uint32_t)-1;
						uint32_t viewer_buffer = (uint32_t)-1;
						uint32_t point_buffer = (uint32_t)-1;
						uint32_t diffuse_buffer = (uint32_t)-1;
						uint32_t normal_buffer = (uint32_t)-1;
						uint32_t depth_buffer = (uint32_t)-1;
						uint32_t surface_buffer = (uint32_t)-1;
						uint32_t depth_map_less = (uint32_t)-1;
					} point;
					struct
					{
						graphics::shader* shader_base = nullptr;
						graphics::shader* shader_shadowed = nullptr;
						uint32_t materials = (uint32_t)-1;
						uint32_t sampler = (uint32_t)-1;
						uint32_t depth_sampler = (uint32_t)-1;
						uint32_t depth_less_sampler = (uint32_t)-1;
						uint32_t viewer_buffer = (uint32_t)-1;
						uint32_t spot_buffer = (uint32_t)-1;
						uint32_t diffuse_buffer = (uint32_t)-1;
						uint32_t normal_buffer = (uint32_t)-1;
						uint32_t depth_buffer = (uint32_t)-1;
						uint32_t surface_buffer = (uint32_t)-1;
						uint32_t depth_map_less = (uint32_t)-1;
					} spot;
					struct
					{
						graphics::shader* shader_base = nullptr;
						graphics::shader* shader_shadowed = nullptr;
						uint32_t materials = (uint32_t)-1;
						uint32_t sampler = (uint32_t)-1;
						uint32_t depth_less_sampler = (uint32_t)-1;
						uint32_t viewer_buffer = (uint32_t)-1;
						uint32_t line_buffer = (uint32_t)-1;
						uint32_t diffuse_buffer = (uint32_t)-1;
						uint32_t normal_buffer = (uint32_t)-1;
						uint32_t depth_buffer = (uint32_t)-1;
						uint32_t surface_buffer = (uint32_t)-1;
						uint32_t depth_map = (uint32_t)-1;
					} line;
					struct
					{
						graphics::shader* shader = nullptr;
						uint32_t materials = (uint32_t)-1;
						uint32_t sampler = (uint32_t)-1;
						uint32_t viewer_buffer = (uint32_t)-1;
						uint32_t environment_buffer = (uint32_t)-1;
						uint32_t normal_buffer = (uint32_t)-1;
						uint32_t depth_buffer = (uint32_t)-1;
						uint32_t surface_buffer = (uint32_t)-1;
						uint32_t environment_map = (uint32_t)-1;
					} surface;
				} pipelines;

				struct
				{
					graphics::graphics_device* device = nullptr;
					scene_graph* scene = nullptr;
				} state;

				struct
				{
					renderer_proxy<components::illuminator> illuminators;
					renderer_proxy<components::surface_light> surfaces;
					renderer_proxy<components::point_light> points;
					renderer_proxy<components::spot_light> spots;
					core::pool<component*>* lines;
				} lights;

			public:
				struct
				{
					ticker tick;
					float distance = 0.5f;
				} shadows;

				struct
				{
					graphics::multi_render_target_2d* merger = nullptr;
					graphics::render_target_2d* output = nullptr;
					graphics::render_target_2d* input = nullptr;
					graphics::cubemap* subresource = nullptr;
					graphics::texture_2d* face = nullptr;
					size_t size = 128;
				} surfaces;

			private:
				graphics::depth_stencil_state* depth_stencil_none = nullptr;
				graphics::depth_stencil_state* depth_stencil_greater = nullptr;
				graphics::depth_stencil_state* depth_stencil_less = nullptr;
				graphics::rasterizer_state* front_rasterizer = nullptr;
				graphics::rasterizer_state* back_rasterizer = nullptr;
				graphics::rasterizer_state* none_rasterizer = nullptr;
				graphics::blend_state* blend_additive = nullptr;
				graphics::blend_state* blend_overwrite = nullptr;
				graphics::blend_state* blend_overload = nullptr;
				graphics::sampler_state* depth_sampler = nullptr;
				graphics::sampler_state* depth_less_sampler = nullptr;
				graphics::sampler_state* wrap_sampler = nullptr;
				graphics::input_layout* layout = nullptr;
				graphics::texture_2d* sky_base = nullptr;
				graphics::texture_cube* sky_map = nullptr;
				isurface_buffer surface_buffer;
				ipoint_buffer point_buffer;
				ispot_buffer spot_buffer;
				iline_buffer line_buffer;

			public:
				graphics::texture_2d* lighting_map = nullptr;
				iambient_buffer ambient_buffer;

			public:
				lighting(render_system* lab);
				~lighting();
				size_t render_pass(core::timer* time) override;
				void deserialize(core::schema* node) override;
				void serialize(core::schema* node) override;
				void resize_buffers() override;
				void begin_pass(core::timer* time) override;
				void end_pass() override;
				void set_sky_map(graphics::texture_2d* cubemap);
				void set_surface_buffer_size(size_t size);
				graphics::texture_cube* get_sky_map();
				graphics::texture_2d* get_sky_base();

			private:
				float get_dominant(const trigonometry::vector3& axis);
				bool load_surface_buffer(isurface_buffer* dest, component* src, trigonometry::vector3& position, trigonometry::vector3& scale);
				bool load_point_buffer(ipoint_buffer* dest, component* src, trigonometry::vector3& position, trigonometry::vector3& scale, bool reposition);
				bool load_spot_buffer(ispot_buffer* dest, component* src, trigonometry::vector3& position, trigonometry::vector3& scale, bool reposition);
				bool load_line_buffer(iline_buffer* dest, component* src);
				void apply_light_culling(component* src, float range, trigonometry::vector3* position, trigonometry::vector3* scale);
				void render_result_buffers();
				void render_surface_maps(core::timer* time);
				void render_point_shadow_maps(core::timer* time);
				void render_spot_shadow_maps(core::timer* time);
				void render_line_shadow_maps(core::timer* time);
				void render_surface_lights();
				void render_point_lights();
				void render_spot_lights();
				void render_line_lights();
				void render_ambient();

			public:
				VI_COMPONENT("lighting_renderer");
			};

			class transparency final : public renderer
			{
			private:
				struct
				{
					graphics::shader* shader = nullptr;
					uint32_t materials = (uint32_t)-1;
					uint32_t sampler = (uint32_t)-1;
					uint32_t transparency_buffer = (uint32_t)-1;
					uint32_t diffuse_buffer = (uint32_t)-1;
					uint32_t depth_buffer = (uint32_t)-1;
					uint32_t ldiffuse_buffer = (uint32_t)-1;
					uint32_t lnormal_buffer = (uint32_t)-1;
					uint32_t ldepth_buffer = (uint32_t)-1;
					uint32_t lsurface_buffer = (uint32_t)-1;
				} pipeline;

			private:
				graphics::multi_render_target_2d* merger = nullptr;
				graphics::render_target_2d* input = nullptr;
				graphics::depth_stencil_state* depth_stencil = nullptr;
				graphics::rasterizer_state* rasterizer = nullptr;
				graphics::blend_state* blend = nullptr;
				graphics::sampler_state* sampler = nullptr;
				graphics::input_layout* layout = nullptr;
				float mip_levels[2] = { 0.0f, 0.0f };

			public:
				struct render_constant
				{
					trigonometry::vector3 padding;
					float mips = 0.0f;
				} render_data;

			public:
				transparency(render_system* lab);
				~transparency();
				void resize_buffers() override;
				size_t render_pass(core::timer* time) override;

			public:
				VI_COMPONENT("transparency_renderer");
			};

			class local_reflections final : public effect_renderer
			{
			private:
				struct
				{
					graphics::shader* gloss[2] = { nullptr };
					graphics::shader* reflectance = nullptr;
					graphics::shader* additive = nullptr;
				} pipelines;

			public:
				struct reflectance_buffer
				{
					float samples = 32.0f;
					float padding = 0.0f;
					float intensity = 1.0f;
					float distance = 16.0f;
				} reflectance;

				struct gloss_buffer
				{
					float padding = 0.0f;
					float deadzone = 0.05f;
					float mips = 0.0f;
					float cutoff = 0.95f;
					float texel[2] = { 1.0f, 1.0f };
					float samples = 48.000f;
					float blur = 64.000f;
				} gloss;

			public:
				local_reflections(render_system* lab);
				void deserialize(core::schema* node) override;
				void serialize(core::schema* node) override;
				void render_effect(core::timer* time) override;

			public:
				VI_COMPONENT("local_reflections_renderer");
			};

			class local_illumination final : public effect_renderer
			{
			private:
				struct
				{
					graphics::shader* stochastic = nullptr;
					graphics::shader* indirection = nullptr;
					graphics::shader* denoise[2] = { nullptr };
					graphics::shader* additive = nullptr;
				} pipelines;

			public:
				struct stochastic_buffer
				{
					float texel[2] = { 1.0f, 1.0f };
					float frame_id = 0.0f;
					float padding = 0.0f;
				} stochastic;

				struct indirection_buffer
				{
					float random[2] = { 0.0f, 0.0f };
					float samples = 10.0f;
					float distance = 3.0f;
					float initial = 0.0f;
					float cutoff = -0.05f;
					float attenuation = 1.0f;
					float swing = 0.333333f;
					float padding[3] = { 0.0f, 0.0f, 0.0f };
					float bias = 2.0f;
				} indirection;

				struct denoise_buffer
				{
					float padding[3] = { 0.0f, 0.0f, 0.0f };
					float cutoff = 0.95f;
					float texel[2] = { 1.0f, 1.0f };
					float samples = 32.000f;
					float blur = 16.000f;
				} denoise;

			private:
				graphics::texture_2d* emission_map = nullptr;

			public:
				uint32_t bounces = 1;

			public:
				local_illumination(render_system* lab);
				~local_illumination() override;
				void deserialize(core::schema* node) override;
				void serialize(core::schema* node) override;
				void render_effect(core::timer* time) override;
				void resize_effect() override;

			public:
				VI_COMPONENT("local_illumination_renderer");
			};

			class local_ambient final : public effect_renderer
			{
			private:
				struct
				{
					graphics::shader* shading = nullptr;
					graphics::shader* fibo[2] = { nullptr };
					graphics::shader* multiply = nullptr;
				} pipelines;

			public:
				struct shading_buffer
				{
					float samples = 4.0f;
					float intensity = 3.12f;
					float scale = 0.5f;
					float bias = 0.11f;
					float radius = 0.06f;
					float distance = 3.83f;
					float fade = 1.96f;
					float padding = 0.0f;
				} shading;

				struct fibo_buffer
				{
					float padding[3] = { 0.0f };
					float power = 1.000f;
					float texel[2] = { 1.0f, 1.0f };
					float samples = 14.000f;
					float blur = 8.000f;
				} fibo;

			public:
				local_ambient(render_system* lab);
				void deserialize(core::schema* node) override;
				void serialize(core::schema* node) override;
				void render_effect(core::timer* time) override;

			public:
				VI_COMPONENT("local_ambient_renderer");
			};

			class depth_of_field final : public effect_renderer
			{
			private:
				struct
				{
					float radius = 0.0f;
					float factor = 0.0f;
					float distance = 0.0f;
					float range = 0.0f;
				} state;

			public:
				struct focus_buffer
				{
					float texel[2] = { 1.0f / 512.0f };
					float radius = 1.0f;
					float bokeh = 8.0f;
					float padding[3] = { 0.0f };
					float scale = 1.0f;
					float near_distance = 0.0f;
					float near_range = 0.0f;
					float far_distance = 32.0f;
					float far_range = 8.0f;
				} focus;

			public:
				float distance = -1.0f;
				float radius = 1.5f;
				float time = 0.1f;

			public:
				depth_of_field(render_system* lab);
				void deserialize(core::schema* node) override;
				void serialize(core::schema* node) override;
				void render_effect(core::timer* time) override;
				void focus_at_nearest_target(float step);

			public:
				VI_COMPONENT("dof_renderer");
			};

			class motion_blur final : public effect_renderer
			{
			private:
				struct
				{
					graphics::shader* velocity = nullptr;
					graphics::shader* motion[2] = { nullptr };
				} pipelines;

			public:
				struct velocity_buffer
				{
					float padding[2] = { 0.0f };
					float power = 6.0f;
					float threshold = 0.50f;
				} velocity;

				struct motion_buffer
				{
					float texel[2] = { 1.0f, 1.0f };
					float samples = 14.000f;
					float motion = 10.0f;
				} motion;

			private:
				graphics::texture_2d* prev_diffuse_map = nullptr;
				graphics::texture_2d* velocity_map = nullptr;

			public:
				motion_blur(render_system* lab);
				~motion_blur() override;
				void deserialize(core::schema* node) override;
				void serialize(core::schema* node) override;
				void render_effect(core::timer* time) override;
				void resize_effect() override;

			public:
				VI_COMPONENT("motion_blur_renderer");
			};

			class bloom final : public effect_renderer
			{
			private:
				struct
				{
					graphics::shader* bloom = nullptr;
					graphics::shader* fibo[2] = { nullptr };
					graphics::shader* additive = nullptr;
				} pipelines;

			public:
				struct extraction_buffer
				{
					float padding[2] = { 0.0f };
					float intensity = 8.0f;
					float threshold = 0.73f;
				} extraction;

				struct fibo_buffer
				{
					float padding[3] = { 0.0f };
					float power = 1.000f;
					float texel[2] = { 1.0f, 1.0f };
					float samples = 14.000f;
					float blur = 64.000f;
				} fibo;

			public:
				bloom(render_system* lab);
				void deserialize(core::schema* node) override;
				void serialize(core::schema* node) override;
				void render_effect(core::timer* time) override;

			public:
				VI_COMPONENT("bloom_renderer");
			};

			class tone final : public effect_renderer
			{
			private:
				struct
				{
					graphics::shader* luminance = nullptr;
					graphics::shader* tone = nullptr;
				} pipelines;

			public:
				struct luminance_buffer
				{
					float texel[2] = { 1.0f, 1.0f };
					float mips = 0.0f;
					float time = 0.0f;
				} luminance;

				struct mapping_buffer
				{
					float padding[2] = { 0.0f };
					float grayscale = -0.12f;
					float aces = 0.6f;
					float filmic = -0.12f;
					float lottes = 0.109f;
					float reinhard = -0.09f;
					float reinhard2 = -0.03f;
					float unreal = -0.13f;
					float uchimura = 1.0f;
					float ubrightness = 2.0f;
					float ucontrast = 1.0f;
					float ustart = 0.82f;
					float ulength = 0.4f;
					float ublack = 1.13f;
					float upedestal = 0.05f;
					float exposure = 0.0f;
					float eintensity = 0.9f;
					float egamma = 2.2f;
					float adaptation = 0.0f;
					float agray = 1.0f;
					float awhite = 1.0f;
					float ablack = 0.05f;
					float aspeed = 2.0f;
				} mapping;

			private:
				graphics::render_target_2d* lut_target = nullptr;
				graphics::texture_2d* lut_map = nullptr;

			public:
				tone(render_system* lab);
				~tone() override;
				void deserialize(core::schema* node) override;
				void serialize(core::schema* node) override;
				void render_effect(core::timer* time) override;

			private:
				void render_lut(core::timer* time);
				void set_lut_size(size_t size);

			public:
				VI_COMPONENT("tone_renderer");
			};

			class glitch final : public effect_renderer
			{
			public:
				struct distortion_buffer
				{
					float scan_line_jitter_displacement = 0;
					float scan_line_jitter_threshold = 0;
					float vertical_jump_amount = 0;
					float vertical_jump_time = 0;
					float color_drift_amount = 0;
					float color_drift_time = 0;
					float horizontal_shake = 0;
					float elapsed_time = 0;
				} distortion;

			public:
				float scan_line_jitter;
				float vertical_jump;
				float horizontal_shake;
				float color_drift;

			public:
				glitch(render_system* lab);
				void deserialize(core::schema* node) override;
				void serialize(core::schema* node) override;
				void render_effect(core::timer* time) override;

			public:
				VI_COMPONENT("glitch_renderer");
			};

			class user_interface final : public renderer
			{
			private:
				graphics::activity* activity;
				gui::context* context;

			public:
				user_interface(render_system* lab);
				user_interface(render_system* lab, gui::context* new_context, graphics::activity* new_activity);
				size_t render_pass(core::timer* time) override;
				gui::context* get_context();

			public:
				VI_COMPONENT("user_interface_renderer");
			};
		}
	}
}
#endif