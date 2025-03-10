#ifndef VI_LAYER_PROCESSORS_H_EXTENSION
#define VI_LAYER_PROCESSORS_H_EXTENSION
#include <vitex/layer/processors.h>
#include "../layer.h"

namespace vitex
{
	namespace layer
	{
		namespace processors
		{
			enum class mesh_opt : uint64_t
			{
				calc_tangent_space = 0x1,
				join_identical_vertices = 0x2,
				make_left_handed = 0x4,
				triangulate = 0x8,
				remove_component = 0x10,
				gen_normals = 0x20,
				gen_smooth_normals = 0x40,
				split_large_meshes = 0x80,
				pre_transform_vertices = 0x100,
				limit_bone_weights = 0x200,
				validate_data_structure = 0x400,
				improve_cache_locality = 0x800,
				remove_redundant_materials = 0x1000,
				fix_infacing_normals = 0x2000,
				sort_by_ptype = 0x8000,
				remove_degenerates = 0x10000,
				remove_invalid_data = 0x20000,
				remove_instances = 0x100000,
				gen_uv_coords = 0x40000,
				transform_uv_coords = 0x80000,
				optimize_meshes = 0x200000,
				optimize_graph = 0x400000,
				flip_uvs = 0x800000,
				flip_winding_order = 0x1000000,
				split_by_bone_count = 0x2000000,
				debone = 0x4000000,
				global_scale = 0x8000000,
				embed_textures = 0x10000000,
				force_gen_normals = 0x20000000,
				drop_normals = 0x40000000,
				gen_bounding_boxes = 0x80000000l
			};

			constexpr inline mesh_opt operator |(mesh_opt a, mesh_opt b)
			{
				return static_cast<mesh_opt>(static_cast<uint64_t>(a) | static_cast<uint64_t>(b));
			}

			enum class mesh_preset : uint64_t
			{
				defaults = (uint64_t)(mesh_opt::flip_winding_order | mesh_opt::triangulate | mesh_opt::calc_tangent_space | mesh_opt::validate_data_structure)
			};

			struct mesh_bone
			{
				size_t index;
				trigonometry::animator_key defaults;
			};

			struct mesh_joint
			{
				trigonometry::matrix4x4 local;
				size_t index;
				bool linking;
			};

			struct mesh_blob
			{
				core::unordered_map<size_t, size_t> joint_indices;
				core::vector<trigonometry::skin_vertex> vertices;
				core::vector<int32_t> indices;
				core::string name;
				trigonometry::matrix4x4 transform;
				size_t local_index = 0;
			};

			struct model_info
			{
				core::unordered_map<core::string, mesh_joint> joint_offsets;
				core::vector<mesh_blob> meshes;
				trigonometry::matrix4x4 transform;
				trigonometry::joint skeleton;
				trigonometry::vector3 min, max;
				float low = 0.0f, high = 0.0f;
				size_t global_index = 0;
			};

			struct model_channel
			{
				core::unordered_map<float, trigonometry::vector3> positions;
				core::unordered_map<float, trigonometry::vector3> scales;
				core::unordered_map<float, trigonometry::quaternion> rotations;
			};

			class material_processor final : public processor
			{
			public:
				material_processor(content_manager* manager);
				expects_content<core::unique<void>> duplicate(asset_cache* asset, const core::variant_args& args) override;
				expects_content<core::unique<void>> deserialize(core::stream* stream, size_t offset, const core::variant_args& args) override;
				expects_content<void> serialize(core::stream* stream, void* object, const core::variant_args& args) override;
				void free(asset_cache* asset) override;
			};

			class scene_graph_processor final : public processor
			{
			public:
				std::function<void(scene_graph*)> setup_callback;

			public:
				scene_graph_processor(content_manager* manager);
				expects_content<core::unique<void>> deserialize(core::stream* stream, size_t offset, const core::variant_args& args) override;
				expects_content<void> serialize(core::stream* stream, void* object, const core::variant_args& args) override;
			};

			class audio_clip_processor final : public processor
			{
			public:
				audio_clip_processor(content_manager* manager);
				~audio_clip_processor() override;
				expects_content<core::unique<void>> duplicate(asset_cache* asset, const core::variant_args& args) override;
				expects_content<core::unique<void>> deserialize(core::stream* stream, size_t offset, const core::variant_args& args) override;
				expects_content<core::unique<void>> deserialize_wave(core::stream* stream, size_t offset, const core::variant_args& args);
				expects_content<core::unique<void>> deserialize_ogg(core::stream* stream, size_t offset, const core::variant_args& args);
				void free(asset_cache* asset) override;
			};

			class texture_2d_processor final : public processor
			{
			public:
				texture_2d_processor(content_manager* manager);
				~texture_2d_processor() override;
				expects_content<core::unique<void>> duplicate(asset_cache* asset, const core::variant_args& args) override;
				expects_content<core::unique<void>> deserialize(core::stream* stream, size_t offset, const core::variant_args& args) override;
				void free(asset_cache* asset) override;
			};

			class shader_processor final : public processor
			{
			public:
				shader_processor(content_manager* manager);
				~shader_processor() override;
				expects_content<core::unique<void>> duplicate(asset_cache* asset, const core::variant_args& args) override;
				expects_content<core::unique<void>> deserialize(core::stream* stream, size_t offset, const core::variant_args& args) override;
				void free(asset_cache* asset) override;
			};

			class model_processor final : public processor
			{
			public:
				graphics::mesh_buffer::desc options;

			public:
				model_processor(content_manager* manager);
				~model_processor() override;
				expects_content<core::unique<void>> duplicate(asset_cache* asset, const core::variant_args& args) override;
				expects_content<core::unique<void>> deserialize(core::stream* stream, size_t offset, const core::variant_args& args) override;
				void free(asset_cache* asset) override;

			public:
				static expects_content<core::unique<core::schema>> import(core::stream * stream, uint64_t opts = (uint64_t)mesh_preset::defaults);
				static expects_content<model_info> import_for_immediate_use(core::stream* stream, uint64_t opts = (uint64_t)mesh_preset::defaults);
			};

			class skin_model_processor final : public processor
			{
			public:
				graphics::skin_mesh_buffer::desc options;

			public:
				skin_model_processor(content_manager* manager);
				~skin_model_processor() override;
				expects_content<core::unique<void>> duplicate(asset_cache* asset, const core::variant_args& args) override;
				expects_content<core::unique<void>> deserialize(core::stream* stream, size_t offset, const core::variant_args& args) override;
				void free(asset_cache* asset) override;
			};

			class skin_animation_processor final : public processor
			{
			public:
				skin_animation_processor(content_manager* manager);
				~skin_animation_processor() override;
				expects_content<core::unique<void>> duplicate(asset_cache* asset, const core::variant_args& args) override;
				expects_content<core::unique<void>> deserialize(core::stream* stream, size_t offset, const core::variant_args& args) override;
				void free(asset_cache* asset) override;

			public:
				static expects_content<core::schema*> import(core::stream * stream, uint64_t opts = (uint64_t)mesh_preset::defaults);
				static expects_content<core::vector<trigonometry::skin_animator_clip>> import_for_immediate_use(core::stream* stream, uint64_t opts = (uint64_t)mesh_preset::defaults);
			};

			class hull_shape_processor final : public processor
			{
			public:
				hull_shape_processor(content_manager* manager);
				~hull_shape_processor() override;
				expects_content<core::unique<void>> duplicate(asset_cache* asset, const core::variant_args& args) override;
				expects_content<core::unique<void>> deserialize(core::stream* stream, size_t offset, const core::variant_args& args) override;
				void free(asset_cache* asset) override;
			};
		}
	}
}
#endif