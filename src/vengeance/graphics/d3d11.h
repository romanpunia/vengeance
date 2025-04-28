#ifndef VI_GRAPHICS_D3D11_H
#define VI_GRAPHICS_D3D11_H
#include "../graphics.h"
#include <array>
#ifdef VI_MICROSOFT
#include <d3d11.h>
#include <d3d11shader.h>
#include <d3dcompiler.h>

namespace vitex
{
	namespace graphics
	{
		namespace d3d11
		{
			class d3d11_device;

			class d3d11_depth_stencil_state final : public depth_stencil_state
			{
				friend d3d11_device;

			public:
				ID3D11DepthStencilState* resource;

			public:
				d3d11_depth_stencil_state(const desc& i);
				~d3d11_depth_stencil_state() override;
				void* get_resource() const override;
			};

			class d3d11_rasterizer_state final : public rasterizer_state
			{
				friend d3d11_device;

			public:
				ID3D11RasterizerState* resource;

			public:
				d3d11_rasterizer_state(const desc& i);
				~d3d11_rasterizer_state() override;
				void* get_resource() const override;
			};

			class d3d11_blend_state final : public blend_state
			{
				friend d3d11_device;

			public:
				ID3D11BlendState* resource;

			public:
				d3d11_blend_state(const desc& i);
				~d3d11_blend_state() override;
				void* get_resource() const override;
			};

			class d3d11_sampler_state final : public sampler_state
			{
				friend d3d11_device;

			public:
				ID3D11SamplerState* resource;

			public:
				d3d11_sampler_state(const desc& i);
				~d3d11_sampler_state() override;
				void* get_resource() const override;
			};

			class d3d11_input_layout final : public input_layout
			{
				friend d3d11_device;

			public:
				d3d11_input_layout(const desc& i);
				~d3d11_input_layout() override;
				void* get_resource() const override;
			};

			class d3d11_shader final : public shader
			{
				friend d3d11_device;

			private:
				bool compiled;

			public:
				struct
				{
					ID3D11ShaderReflection* vertex_shader = nullptr;
					ID3D11ShaderReflection* pixel_shader = nullptr;
					ID3D11ShaderReflection* geometry_shader = nullptr;
					ID3D11ShaderReflection* domain_shader = nullptr;
					ID3D11ShaderReflection* hull_shader = nullptr;
					ID3D11ShaderReflection* compute_shader = nullptr;
				} reflection;

			public:
				ID3D11VertexShader* vertex_shader;
				ID3D11PixelShader* pixel_shader;
				ID3D11GeometryShader* geometry_shader;
				ID3D11DomainShader* domain_shader;
				ID3D11HullShader* hull_shader;
				ID3D11ComputeShader* compute_shader;
				ID3D11Buffer* constant_buffer;
				ID3D11InputLayout* vertex_layout;
				ID3DBlob* signature;

			public:
				d3d11_shader(const desc& i);
				~d3d11_shader() override;
				bool is_valid() const override;
			};

			class d3d11_element_buffer final : public element_buffer
			{
				friend d3d11_device;

			public:
				ID3D11UnorderedAccessView* access;
				ID3D11ShaderResourceView* resource;
				ID3D11Buffer* element;

			public:
				d3d11_element_buffer(const desc& i);
				~d3d11_element_buffer() override;
				void* get_resource() const override;
			};

			class d3d11_mesh_buffer final : public mesh_buffer
			{
				friend d3d11_device;

			public:
				d3d11_mesh_buffer(const desc& i);
				trigonometry::vertex* get_elements(graphics_device* device) const override;
			};

			class d3d11_skin_mesh_buffer final : public skin_mesh_buffer
			{
				friend d3d11_device;

			public:
				d3d11_skin_mesh_buffer(const desc& i);
				trigonometry::skin_vertex* get_elements(graphics_device* device) const override;
			};

			class d3d11_instance_buffer final : public instance_buffer
			{
				friend d3d11_device;

			public:
				ID3D11ShaderResourceView* resource;

			public:
				d3d11_instance_buffer(const desc& i);
				~d3d11_instance_buffer() override;
			};

			class d3d11_texture_2d final : public texture_2d
			{
				friend d3d11_device;

			public:
				ID3D11UnorderedAccessView* access;
				ID3D11ShaderResourceView* resource;
				ID3D11Texture2D* view;

			public:
				d3d11_texture_2d();
				d3d11_texture_2d(const desc& i);
				~d3d11_texture_2d() override;
				void* get_resource() const override;
			};

			class d3d11_texture_3d final : public texture_3d
			{
				friend d3d11_device;

			public:
				ID3D11UnorderedAccessView* access;
				ID3D11ShaderResourceView* resource;
				ID3D11Texture3D* view;

			public:
				d3d11_texture_3d();
				~d3d11_texture_3d() override;
				void* get_resource() override;
			};

			class d3d11_texture_cube final : public texture_cube
			{
				friend d3d11_device;

			public:
				ID3D11UnorderedAccessView* access;
				ID3D11ShaderResourceView* resource;
				ID3D11Texture2D* view;

			public:
				d3d11_texture_cube();
				d3d11_texture_cube(const desc& i);
				~d3d11_texture_cube() override;
				void* get_resource() const override;
			};

			class d3d11_depth_target_2d final : public depth_target_2d
			{
				friend d3d11_device;

			public:
				ID3D11DepthStencilView* depth_stencil_view;

			public:
				d3d11_depth_target_2d(const desc& i);
				~d3d11_depth_target_2d() override;
				void* get_resource() const override;
				uint32_t get_width() const override;
				uint32_t get_height() const override;
			};

			class d3d11_depth_target_cube final : public depth_target_cube
			{
				friend d3d11_device;

			public:
				ID3D11DepthStencilView* depth_stencil_view;

			public:
				d3d11_depth_target_cube(const desc& i);
				~d3d11_depth_target_cube() override;
				void* get_resource() const override;
				uint32_t get_width() const override;
				uint32_t get_height() const override;
			};

			class d3d11_render_target_2d final : public render_target_2d
			{
				friend d3d11_device;

			public:
				ID3D11DepthStencilView* depth_stencil_view;
				ID3D11RenderTargetView* render_target_view;
				ID3D11Texture2D* texture;

			public:
				d3d11_render_target_2d(const desc& i);
				~d3d11_render_target_2d() override;
				void* get_target_buffer() const override;
				void* get_depth_buffer() const override;
				uint32_t get_width() const override;
				uint32_t get_height() const override;
			};

			class d3d11_multi_render_target_2d final : public multi_render_target_2d
			{
				friend d3d11_device;

			public:
				ID3D11DepthStencilView* depth_stencil_view;
				ID3D11RenderTargetView* render_target_view[8];
				ID3D11Texture2D* texture[8];
				D3D11_TEXTURE2D_DESC information;

			public:
				d3d11_multi_render_target_2d(const desc& i);
				~d3d11_multi_render_target_2d() override;
				void* get_target_buffer() const override;
				void* get_depth_buffer() const override;
				uint32_t get_width() const override;
				uint32_t get_height() const override;

			private:
				void fill_view(uint32_t target, uint32_t mip_levels, uint32_t size);
			};

			class d3d11_render_target_cube final : public render_target_cube
			{
				friend d3d11_device;

			public:
				ID3D11DepthStencilView* depth_stencil_view;
				ID3D11RenderTargetView* render_target_view;
				ID3D11Texture2D* texture;

			public:
				d3d11_render_target_cube(const desc& i);
				~d3d11_render_target_cube() override;
				void* get_target_buffer() const override;
				void* get_depth_buffer() const override;
				uint32_t get_width() const override;
				uint32_t get_height() const override;
			};

			class d3d11_multi_render_target_cube final : public multi_render_target_cube
			{
				friend d3d11_device;

			public:
				ID3D11DepthStencilView* depth_stencil_view;
				ID3D11RenderTargetView* render_target_view[8];
				ID3D11Texture2D* texture[8];

			public:
				d3d11_multi_render_target_cube(const desc& i);
				~d3d11_multi_render_target_cube() override;
				void* get_target_buffer() const override;
				void* get_depth_buffer() const override;
				uint32_t get_width() const override;
				uint32_t get_height() const override;
			};

			class d3d11_cubemap final : public cubemap
			{
				friend d3d11_device;

			public:
				struct
				{
					D3D11_SHADER_RESOURCE_VIEW_DESC resource;
					D3D11_TEXTURE2D_DESC texture;
					D3D11_BOX region;
				} options;

			public:
				ID3D11Texture2D* merger;
				ID3D11Texture2D* source;

			public:
				d3d11_cubemap(const desc& i);
				~d3d11_cubemap() override;
			};

			class d3d11_query final : public query
			{
			public:
				ID3D11Query* async;

			public:
				d3d11_query();
				~d3d11_query() override;
				void* get_resource() const override;
			};

			class d3d11_device final : public graphics_device
			{
			private:
				struct
				{
					ID3D11VertexShader* vertex_shader = nullptr;
					ID3D11PixelShader* pixel_shader = nullptr;
					ID3D11InputLayout* vertex_layout = nullptr;
					ID3D11Buffer* constant_buffer = nullptr;
					ID3D11Buffer* vertex_buffer = nullptr;
					ID3D11SamplerState* sampler = nullptr;
				} immediate;

				struct
				{
					std::array<d3d11_shader*, 6> shaders = { };
					std::array<std::pair<ID3D11ShaderResourceView*, uint32_t>, units_size> resources = { };
					std::array<std::pair<d3d11_element_buffer*, uint32_t>, units_size> vertex_buffers = { };
					std::tuple<ID3D11SamplerState*, uint32_t, uint32_t> sampler = { nullptr, 0, 0 };
					std::tuple<d3d11_element_buffer*, format> index_buffer = { nullptr, format::unknown };
					ID3D11BlendState* blend = nullptr;
					ID3D11RasterizerState* rasterizer = nullptr;
					ID3D11DepthStencilState* depth_stencil = nullptr;
					d3d11_input_layout* layout = nullptr;
					primitive_topology primitive = primitive_topology::triangle_list;
				} regs;

				struct
				{
					std::string_view vertex = "";
					std::string_view pixel = "";
					std::string_view geometry = "";
					std::string_view hull = "";
					std::string_view domain = "";
					std::string_view compute = "";
				} models;

			public:
				ID3D11DeviceContext* immediate_context;
				ID3D11Device* context;
				IDXGISwapChain* swap_chain;
				DXGI_SWAP_CHAIN_DESC swap_chain_resource;
				D3D_FEATURE_LEVEL feature_level;
				D3D_DRIVER_TYPE driver_type;
				activity* window;

			public:
				d3d11_device(const desc& i);
				~d3d11_device() override;
				void set_as_current_device() override;
				void set_shader_model(shader_model model) override;
				void set_blend_state(blend_state* state) override;
				void set_rasterizer_state(rasterizer_state* state) override;
				void set_depth_stencil_state(depth_stencil_state* state) override;
				void set_input_layout(input_layout* state) override;
				expects_graphics<void> set_shader(shader* resource, uint32_t type) override;
				void set_sampler_state(sampler_state* state, uint32_t slot, uint32_t count, uint32_t type) override;
				void set_buffer(shader* resource, uint32_t slot, uint32_t type) override;
				void set_buffer(instance_buffer* resource, uint32_t slot, uint32_t type) override;
				void set_constant_buffer(element_buffer* resource, uint32_t slot, uint32_t type) override;
				void set_structure_buffer(element_buffer* resource, uint32_t slot, uint32_t type) override;
				void set_texture_2d(texture_2d* resource, uint32_t slot, uint32_t type) override;
				void set_texture_3d(texture_3d* resource, uint32_t slot, uint32_t type) override;
				void set_texture_cube(texture_cube* resource, uint32_t slot, uint32_t type) override;
				void set_index_buffer(element_buffer* resource, format format_mode) override;
				void set_vertex_buffers(element_buffer** resources, uint32_t count, bool dynamic_linkage = false) override;
				void set_writeable(element_buffer** resource, uint32_t slot, uint32_t count, bool computable) override;
				void set_writeable(texture_2d** resource, uint32_t slot, uint32_t count, bool computable) override;
				void set_writeable(texture_3d** resource, uint32_t slot, uint32_t count, bool computable) override;
				void set_writeable(texture_cube** resource, uint32_t slot, uint32_t count, bool computable) override;
				void set_target(float r, float g, float b) override;
				void set_target() override;
				void set_target(depth_target_2d* resource) override;
				void set_target(depth_target_cube* resource) override;
				void set_target(graphics::render_target* resource, uint32_t target, float r, float g, float b) override;
				void set_target(graphics::render_target* resource, uint32_t target) override;
				void set_target(graphics::render_target* resource, float r, float g, float b) override;
				void set_target(graphics::render_target* resource) override;
				void set_target_map(graphics::render_target* resource, bool enabled[8]) override;
				void set_target_rect(uint32_t width, uint32_t height) override;
				void set_viewports(uint32_t count, viewport* viewports) override;
				void set_scissor_rects(uint32_t count, trigonometry::rectangle* value) override;
				void set_primitive_topology(primitive_topology topology) override;
				void flush_texture(uint32_t slot, uint32_t count, uint32_t type) override;
				void flush_state() override;
				void clear_buffer(instance_buffer* resource) override;
				void clear_writable(texture_2d* resource) override;
				void clear_writable(texture_2d* resource, float r, float g, float b) override;
				void clear_writable(texture_3d* resource) override;
				void clear_writable(texture_3d* resource, float r, float g, float b) override;
				void clear_writable(texture_cube* resource) override;
				void clear_writable(texture_cube* resource, float r, float g, float b) override;
				void clear(float r, float g, float b) override;
				void clear(graphics::render_target* resource, uint32_t target, float r, float g, float b) override;
				void clear_depth() override;
				void clear_depth(depth_target_2d* resource) override;
				void clear_depth(depth_target_cube* resource) override;
				void clear_depth(graphics::render_target* resource) override;
				void draw_indexed(uint32_t count, uint32_t index_location, uint32_t base_location) override;
				void draw_indexed(mesh_buffer* resource) override;
				void draw_indexed(skin_mesh_buffer* resource) override;
				void draw_indexed_instanced(uint32_t index_count_per_instance, uint32_t instance_count, uint32_t index_location, uint32_t vertex_location, uint32_t instance_location) override;
				void draw_indexed_instanced(element_buffer* instances, mesh_buffer* resource, uint32_t instance_count) override;
				void draw_indexed_instanced(element_buffer* instances, skin_mesh_buffer* resource, uint32_t instance_count) override;
				void draw(uint32_t count, uint32_t location) override;
				void draw_instanced(uint32_t vertex_count_per_instance, uint32_t instance_count, uint32_t vertex_location, uint32_t instance_location) override;
				void dispatch(uint32_t group_x, uint32_t group_y, uint32_t group_z) override;
				void get_viewports(uint32_t* count, viewport* out) override;
				void get_scissor_rects(uint32_t* count, trigonometry::rectangle* out) override;
				void query_begin(query* resource) override;
				void query_end(query* resource) override;
				void generate_mips(texture_2d* resource) override;
				void generate_mips(texture_3d* resource) override;
				void generate_mips(texture_cube* resource) override;
				bool im_begin() override;
				void im_transform(const trigonometry::matrix4x4& transform) override;
				void im_topology(primitive_topology topology) override;
				void im_emit() override;
				void im_texture(texture_2d* in) override;
				void im_color(float x, float y, float z, float w) override;
				void im_intensity(float intensity) override;
				void im_texcoord(float x, float y) override;
				void im_texcoord_offset(float x, float y) override;
				void im_position(float x, float y, float z) override;
				bool im_end() override;
				bool has_explicit_slots() const override;
				expects_graphics<uint32_t> get_shader_slot(shader* resource, const std::string_view& name) const override;
				expects_graphics<uint32_t> get_shader_sampler_slot(shader* resource, const std::string_view& resource_name, const std::string_view& sampler_name) const override;
				expects_graphics<void> submit() override;
				expects_graphics<void> map(element_buffer* resource, resource_map mode, mapped_subresource* map) override;
				expects_graphics<void> map(texture_2d* resource, resource_map mode, mapped_subresource* map) override;
				expects_graphics<void> map(texture_3d* resource, resource_map mode, mapped_subresource* map) override;
				expects_graphics<void> map(texture_cube* resource, resource_map mode, mapped_subresource* map) override;
				expects_graphics<void> unmap(texture_2d* resource, mapped_subresource* map) override;
				expects_graphics<void> unmap(texture_3d* resource, mapped_subresource* map) override;
				expects_graphics<void> unmap(texture_cube* resource, mapped_subresource* map) override;
				expects_graphics<void> unmap(element_buffer* resource, mapped_subresource* map) override;
				expects_graphics<void> update_constant_buffer(element_buffer* resource, void* data, size_t size) override;
				expects_graphics<void> update_buffer(element_buffer* resource, void* data, size_t size) override;
				expects_graphics<void> update_buffer(shader* resource, const void* data) override;
				expects_graphics<void> update_buffer(mesh_buffer* resource, trigonometry::vertex* data) override;
				expects_graphics<void> update_buffer(skin_mesh_buffer* resource, trigonometry::skin_vertex* data) override;
				expects_graphics<void> update_buffer(instance_buffer* resource) override;
				expects_graphics<void> update_buffer_size(shader* resource, size_t size) override;
				expects_graphics<void> update_buffer_size(instance_buffer* resource, size_t size) override;
				expects_graphics<void> copy_texture_2d(texture_2d* resource, texture_2d** result) override;
				expects_graphics<void> copy_texture_2d(graphics::render_target* resource, uint32_t target, texture_2d** result) override;
				expects_graphics<void> copy_texture_2d(render_target_cube* resource, trigonometry::cube_face face, texture_2d** result) override;
				expects_graphics<void> copy_texture_2d(multi_render_target_cube* resource, uint32_t cube, trigonometry::cube_face face, texture_2d** result) override;
				expects_graphics<void> copy_texture_cube(render_target_cube* resource, texture_cube** result) override;
				expects_graphics<void> copy_texture_cube(multi_render_target_cube* resource, uint32_t cube, texture_cube** result) override;
				expects_graphics<void> copy_target(graphics::render_target* from, uint32_t from_target, graphics::render_target* to, uint32_t to_target) override;
				expects_graphics<void> cubemap_push(cubemap* resource, texture_cube* result) override;
				expects_graphics<void> cubemap_face(cubemap* resource, trigonometry::cube_face face) override;
				expects_graphics<void> cubemap_pop(cubemap* resource) override;
				expects_graphics<void> copy_back_buffer(texture_2d** result) override;
				expects_graphics<void> rescale_buffers(uint32_t width, uint32_t height) override;
				expects_graphics<void> resize_buffers(uint32_t width, uint32_t height) override;
				expects_graphics<void> generate_texture(texture_2d* resource) override;
				expects_graphics<void> generate_texture(texture_3d* resource) override;
				expects_graphics<void> generate_texture(texture_cube* resource) override;
				expects_graphics<void> get_query_data(query* resource, size_t* result, bool flush) override;
				expects_graphics<void> get_query_data(query* resource, bool* result, bool flush) override;
				expects_graphics<depth_stencil_state*> create_depth_stencil_state(const depth_stencil_state::desc& i) override;
				expects_graphics<blend_state*> create_blend_state(const blend_state::desc& i) override;
				expects_graphics<rasterizer_state*> create_rasterizer_state(const rasterizer_state::desc& i) override;
				expects_graphics<sampler_state*> create_sampler_state(const sampler_state::desc& i) override;
				expects_graphics<input_layout*> create_input_layout(const input_layout::desc& i) override;
				expects_graphics<shader*> create_shader(const shader::desc& i) override;
				expects_graphics<element_buffer*> create_element_buffer(const element_buffer::desc& i) override;
				expects_graphics<mesh_buffer*> create_mesh_buffer(const mesh_buffer::desc& i) override;
				expects_graphics<mesh_buffer*> create_mesh_buffer(element_buffer* vertex_buffer, element_buffer* index_buffer) override;
				expects_graphics<skin_mesh_buffer*> create_skin_mesh_buffer(const skin_mesh_buffer::desc& i) override;
				expects_graphics<skin_mesh_buffer*> create_skin_mesh_buffer(element_buffer* vertex_buffer, element_buffer* index_buffer) override;
				expects_graphics<instance_buffer*> create_instance_buffer(const instance_buffer::desc& i) override;
				expects_graphics<texture_2d*> create_texture_2d() override;
				expects_graphics<texture_2d*> create_texture_2d(const texture_2d::desc& i) override;
				expects_graphics<texture_3d*> create_texture_3d() override;
				expects_graphics<texture_3d*> create_texture_3d(const texture_3d::desc& i) override;
				expects_graphics<texture_cube*> create_texture_cube() override;
				expects_graphics<texture_cube*> create_texture_cube(const texture_cube::desc& i) override;
				expects_graphics<texture_cube*> create_texture_cube(texture_2d* resource[6]) override;
				expects_graphics<texture_cube*> create_texture_cube(texture_2d* resource) override;
				expects_graphics<depth_target_2d*> create_depth_target_2d(const depth_target_2d::desc& i) override;
				expects_graphics<depth_target_cube*> create_depth_target_cube(const depth_target_cube::desc& i) override;
				expects_graphics<render_target_2d*> create_render_target_2d(const render_target_2d::desc& i) override;
				expects_graphics<multi_render_target_2d*> create_multi_render_target_2d(const multi_render_target_2d::desc& i) override;
				expects_graphics<render_target_cube*> create_render_target_cube(const render_target_cube::desc& i) override;
				expects_graphics<multi_render_target_cube*> create_multi_render_target_cube(const multi_render_target_cube::desc& i) override;
				expects_graphics<cubemap*> create_cubemap(const cubemap::desc& i) override;
				expects_graphics<query*> create_query(const query::desc& i) override;
				primitive_topology get_primitive_topology() const override;
				shader_model get_supported_shader_model() const override;
				void* get_device() const override;
				void* get_context() const override;
				bool is_valid() const override;
				expects_graphics<void> create_direct_buffer(size_t size);
				expects_graphics<void> create_texture_2d(texture_2d* resource, DXGI_FORMAT format);
				expects_graphics<void> create_texture_cube(texture_cube* resource, DXGI_FORMAT format);
				expects_graphics<ID3D11InputLayout*> generate_input_layout(d3d11_shader* shader);
				HRESULT create_constant_buffer(ID3D11Buffer** buffer, size_t size);
				std::string_view get_compile_state(ID3DBlob* error);
				const std::string_view& get_vs_profile();
				const std::string_view& get_ps_profile();
				const std::string_view& get_gs_profile();
				const std::string_view& get_hs_profile();
				const std::string_view& get_cs_profile();
				const std::string_view& get_ds_profile();

			protected:
				expects_graphics<texture_cube*> create_texture_cube_internal(void* resource[6]) override;
			};
		}
	}
}
#endif
#endif
