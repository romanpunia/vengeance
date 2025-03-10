#ifndef VI_LAYER_COMPONENTS_H
#define VI_LAYER_COMPONENTS_H
#include "../layer.h"

namespace vitex
{
	namespace layer
	{
		namespace components
		{
			class soft_body final : public drawable
			{
			protected:
				physics::hull_shape* hull = nullptr;
				physics::soft_body* instance = nullptr;
				core::vector<trigonometry::vertex> vertices;
				core::vector<int> indices;

			public:
				trigonometry::vector2 texcoord = 1.0f;
				bool kinematic = false;
				bool manage = true;

			public:
				soft_body(entity* ref);
				~soft_body() override;
				void deserialize(core::schema* node) override;
				void serialize(core::schema* node) override;
				void synchronize(core::timer* time) override;
				void deactivate() override;
				float get_visibility(const viewer& view, float distance) const override;
				size_t get_unit_bounds(trigonometry::vector3& min, trigonometry::vector3& max) const override;
				core::unique<component> copy(entity* init) const override;
				void load(physics::hull_shape* shape, float anticipation = 0.0f);
				void load(const std::string_view& path, float anticipation = 0.0f, std::function<void()>&& callback = nullptr);
				void load_ellipsoid(const physics::soft_body::desc::cv::sellipsoid& shape, float anticipation = 0.0f);
				void load_patch(const physics::soft_body::desc::cv::spatch& shape, float anticipation = 0.0f);
				void load_rope(const physics::soft_body::desc::cv::srope& shape, float anticipation = 0.0f);
				void fill(graphics::graphics_device* device, graphics::element_buffer* index_buffer, graphics::element_buffer* vertex_buffer);
				void regenerate();
				void clear();
				void set_transform(const trigonometry::vector3& position, const trigonometry::vector3& scale, const trigonometry::vector3& rotation);
				void set_transform(bool kinematic);
				physics::soft_body* get_body();
				core::vector<trigonometry::vertex>& get_vertices();
				core::vector<int>& get_indices();

			private:
				void deserialize_body(core::schema* node);

			public:
				VI_COMPONENT("soft_body_component");
			};

			class rigid_body final : public component
			{
			private:
				physics::hull_shape* hull = nullptr;
				physics::rigid_body* instance = nullptr;

			public:
				bool kinematic = false;
				bool manage = true;

			public:
				rigid_body(entity* ref);
				~rigid_body() override;
				void deserialize(core::schema* node) override;
				void serialize(core::schema* node) override;
				void synchronize(core::timer* time) override;
				void deactivate() override;
				core::unique<component> copy(entity* init) const override;
				void load(btCollisionShape* shape, float mass, float anticipation = 0.0f);
				void load(const std::string_view& path, float mass, float anticipation = 0.0f, std::function<void()>&& callback = nullptr);
				void clear();
				void set_transform(const trigonometry::vector3& position, const trigonometry::vector3& scale, const trigonometry::vector3& rotation);
				void set_transform(bool kinematic);
				void set_mass(float mass);
				physics::rigid_body* get_body() const;

			private:
				void deserialize_body(core::schema* node);

			public:
				VI_COMPONENT("rigid_body_component");
			};

			class slider_constraint final : public component
			{
			private:
				physics::sconstraint* instance;
				entity* connection;

			public:
				slider_constraint(entity* ref);
				~slider_constraint() override;
				void deserialize(core::schema* node) override;
				void serialize(core::schema* node) override;
				core::unique<component> copy(entity* init) const override;
				void load(entity* other, bool is_ghosted, bool is_linear);
				void clear();
				physics::sconstraint* get_constraint() const;
				entity* get_connection() const;

			public:
				VI_COMPONENT("slider_constraint_component");
			};

			class acceleration final : public component
			{
			private:
				physics::rigid_body* rigid_body = nullptr;

			public:
				trigonometry::vector3 amplitude_velocity;
				trigonometry::vector3 amplitude_torque;
				trigonometry::vector3 constant_velocity;
				trigonometry::vector3 constant_torque;
				trigonometry::vector3 constant_center;
				bool kinematic = true;

			public:
				acceleration(entity* ref);
				void deserialize(core::schema* node) override;
				void serialize(core::schema* node) override;
				void activate(component* init) override;
				void update(core::timer* time) override;
				core::unique<component> copy(entity* init) const override;
				physics::rigid_body* get_body() const;

			public:
				VI_COMPONENT("acceleration_component");
			};

			class model final : public drawable
			{
			protected:
				layer::model* instance = nullptr;

			public:
				trigonometry::vector2 texcoord = 1.0f;

			public:
				model(entity* ref);
				~model() override;
				void deserialize(core::schema* node) override;
				void serialize(core::schema* node) override;
				float get_visibility(const viewer& view, float distance) const override;
				size_t get_unit_bounds(trigonometry::vector3& min, trigonometry::vector3& max) const override;
				core::unique<component> copy(entity* init) const override;
				void set_drawable(core::unique<layer::model> drawable);
				void set_material_for(const std::string_view& name, material* value);
				layer::model* get_drawable();
				material* get_material_for(const std::string_view& name);

			public:
				VI_COMPONENT("model_component");
			};

			class skin final : public drawable
			{
			protected:
				layer::skin_model* instance = nullptr;

			public:
				trigonometry::vector2 texcoord = 1.0f;
				pose_buffer skeleton;

			public:
				skin(entity* ref);
				~skin() override;
				void deserialize(core::schema* node) override;
				void serialize(core::schema* node) override;
				void synchronize(core::timer* time) override;
				float get_visibility(const viewer& view, float distance) const override;
				size_t get_unit_bounds(trigonometry::vector3& min, trigonometry::vector3& max) const override;
				core::unique<component> copy(entity* init) const override;
				void set_drawable(core::unique<layer::skin_model> drawable);
				void set_material_for(const std::string_view& name, material* value);
				layer::skin_model* get_drawable();
				material* get_material_for(const std::string_view& name);

			public:
				VI_COMPONENT("skin_component");
			};

			class emitter final : public drawable
			{
			protected:
				graphics::instance_buffer* instance = nullptr;

			public:
				trigonometry::vector3 min = 0.0f;
				trigonometry::vector3 max = 1.0f;
				bool connected = false;
				bool quad_based = false;

			public:
				emitter(entity* ref);
				~emitter() override;
				void deserialize(core::schema* node) override;
				void serialize(core::schema* node) override;
				void activate(component* init) override;
				size_t get_unit_bounds(trigonometry::vector3& min, trigonometry::vector3& max) const override;
				core::unique<component> copy(entity* init) const override;
				graphics::instance_buffer* get_buffer();

			public:
				VI_COMPONENT("emitter_component");
			};

			class decal final : public drawable
			{
			public:
				trigonometry::vector2 texcoord = 1.0f;

			public:
				decal(entity* ref);
				void deserialize(core::schema* node) override;
				void serialize(core::schema* node) override;
				float get_visibility(const viewer& view, float distance) const override;
				core::unique<component> copy(entity* init) const override;

			public:
				VI_COMPONENT("decal_component");
			};

			class skin_animator final : public component
			{
			private:
				skin* instance = nullptr;
				skin_animation* animation = nullptr;

			public:
				animator_state state;

			public:
				skin_animator(entity* ref);
				~skin_animator() noexcept override;
				void deserialize(core::schema* node) override;
				void serialize(core::schema* node) override;
				void activate(component* init) override;
				void animate(core::timer* time) override;
				core::unique<component> copy(entity* init) const override;
				void set_animation(skin_animation* init);
				void play(int64_t clip = -1, int64_t frame = -1);
				void pause();
				void stop();
				bool is_exists(int64_t clip);
				bool is_exists(int64_t clip, int64_t frame);
				const trigonometry::skin_animator_key* get_frame(int64_t clip, int64_t frame);
				const core::vector<trigonometry::skin_animator_key>* get_clip(int64_t clip);
				int64_t get_clip_by_name(const std::string_view& name) const;
				size_t get_clips_count() const;
				skin* get_skin() const;
				skin_animation* get_animation() const;
				core::string get_path() const;

			private:
				void blend_animation(int64_t clip, int64_t frame);
				void save_binding_state();

			public:
				VI_COMPONENT("skin_animator_component");
			};

			class key_animator final : public component
			{
			private:
				core::string reference;

			public:
				core::vector<trigonometry::key_animator_clip> clips;
				trigonometry::animator_key offset;
				trigonometry::animator_key defaults;
				animator_state state;

			public:
				key_animator(entity* ref);
				~key_animator() override;
				void deserialize(core::schema* node) override;
				void serialize(core::schema* node) override;
				void animate(core::timer* time) override;
				core::unique<component> copy(entity* init) const override;
				void load_animation(const std::string_view& path, std::function<void(bool)>&& callback = nullptr);
				void clear_animation();
				void play(int64_t clip = -1, int64_t frame = -1);
				void pause();
				void stop();
				bool is_exists(int64_t clip);
				bool is_exists(int64_t clip, int64_t frame);
				trigonometry::animator_key* get_frame(int64_t clip, int64_t frame);
				core::vector<trigonometry::animator_key>* get_clip(int64_t clip);
				core::string get_path();

			private:
				void blend_animation(int64_t clip, int64_t frame);
				void save_binding_state();

			public:
				VI_COMPONENT("key_animator_component");
			};

			class emitter_animator final : public component
			{
			private:
				emitter* base = nullptr;

			public:
				trigonometry::vector4 diffuse;
				trigonometry::vector3 position;
				trigonometry::vector3 velocity = 1.0f;
				spawner_properties spawner;
				float rotation_speed = 0.0f;
				float scale_speed = 0.0f;
				float noise = 0.0f;
				bool simulate = false;

			public:
				emitter_animator(entity* ref);
				void deserialize(core::schema* node) override;
				void serialize(core::schema* node) override;
				void activate(component* init) override;
				void animate(core::timer* time) override;
				core::unique<component> copy(entity* init) const override;
				emitter* get_emitter() const;

			protected:
				void accurate_synchronization(float step);
				void fast_synchronization(float step);

			public:
				VI_COMPONENT("emitter_animator_component");
			};

			class free_look final : public component
			{
			private:
				trigonometry::vector2 position;

			public:
				trigonometry::vector2 direction = trigonometry::vector2(1.0f, 1.0f);
				graphics::key_map rotate;
				float sensivity;

			public:
				free_look(entity* ref);
				void update(core::timer* time) override;
				core::unique<component> copy(entity* init) const override;

			public:
				VI_COMPONENT("free_look_component");
			};

			class fly final : public component
			{
			private:
				trigonometry::vector3 velocity;

			public:
				struct move_info
				{
					trigonometry::vector3 axis = trigonometry::vector3(1.0f, 1.0f, -1.0f);
					float faster = 320.3f;
					float normal = 185.4f;
					float slower = 32.6f;
					float fading = 5.4f;
				} moving;

			public:
				graphics::key_map forward = graphics::key_code::w;
				graphics::key_map backward = graphics::key_code::s;
				graphics::key_map right = graphics::key_code::d;
				graphics::key_map left = graphics::key_code::a;
				graphics::key_map up = graphics::key_code::space;
				graphics::key_map down = graphics::key_code::z;
				graphics::key_map fast = graphics::key_code::left_shift;
				graphics::key_map slow = graphics::key_code::left_control;

			public:
				fly(entity* ref);
				void update(core::timer* time) override;
				core::unique<component> copy(entity* init) const override;

			private:
				trigonometry::vector3 get_speed(graphics::activity* activity);

			public:
				VI_COMPONENT("fly_component");
			};

			class audio_source final : public component
			{
			private:
				trigonometry::vector3 last_position;
				audio::audio_source* source;
				audio::audio_sync sync;

			public:
				audio_source(entity* ref);
				~audio_source() override;
				void deserialize(core::schema* node) override;
				void serialize(core::schema* node) override;
				void synchronize(core::timer* time) override;
				core::unique<component> copy(entity* init) const override;
				void apply_playing_position();
				audio::audio_source* get_source() const;
				audio::audio_sync& get_sync();

			public:
				VI_COMPONENT("audio_source_component");
			};

			class audio_listener final : public component
			{
			private:
				trigonometry::vector3 last_position;

			public:
				float gain = 1.0f;

			public:
				audio_listener(entity* ref);
				~audio_listener() override;
				void deserialize(core::schema* node) override;
				void serialize(core::schema* node) override;
				void synchronize(core::timer* time) override;
				void deactivate() override;
				core::unique<component> copy(entity* init) const override;

			public:
				VI_COMPONENT("audio_listener_component");
			};

			class point_light final : public component
			{
			public:
				struct shadow_info
				{
					float softness = 1.0f;
					float distance = 100.0f;
					float bias = 0.0f;
					uint32_t iterations = 2;
					bool enabled = false;
				} shadow;

			private:
				attenuation size;

			public:
				depth_cube_map* depth_map = nullptr;
				trigonometry::matrix4x4 view;
				trigonometry::matrix4x4 projection;
				trigonometry::vector3 diffuse = 1.0f;
				float emission = 1.0f;
				float disperse = 0.0f;

			public:
				point_light(entity* ref);
				void deserialize(core::schema* node) override;
				void serialize(core::schema* node) override;
				void message(const std::string_view& name, core::variant_args& args) override;
				size_t get_unit_bounds(trigonometry::vector3& min, trigonometry::vector3& max) const override;
				float get_visibility(const viewer& view, float distance) const override;
				core::unique<component> copy(entity* init) const override;
				void generate_origin();
				void set_size(const attenuation& value);
				const attenuation& get_size();

			public:
				VI_COMPONENT("point_light_component");
			};

			class spot_light final : public component
			{
			public:
				struct shadow_info
				{
					float softness = 1.0f;
					float distance = 100.0f;
					float bias = 0.0f;
					uint32_t iterations = 2;
					bool enabled = false;
				} shadow;

			private:
				attenuation size;

			public:
				depth_map* depth_map = nullptr;
				trigonometry::matrix4x4 projection;
				trigonometry::matrix4x4 view;
				trigonometry::vector3 diffuse = 1.0f;
				float emission = 1.0f;
				float cutoff = 60.0f;
				float disperse = 0.0f;

			public:
				spot_light(entity* ref);
				void deserialize(core::schema* node) override;
				void serialize(core::schema* node) override;
				void message(const std::string_view& name, core::variant_args& args) override;
				void synchronize(core::timer* time) override;
				size_t get_unit_bounds(trigonometry::vector3& min, trigonometry::vector3& max) const override;
				float get_visibility(const viewer& view, float distance) const override;
				core::unique<component> copy(entity* init) const override;
				void generate_origin();
				void set_size(const attenuation& value);
				const attenuation& get_size();

			public:
				VI_COMPONENT("spot_light_component");
			};

			class line_light final : public component
			{
			public:
				struct sky_info
				{
					trigonometry::vector3 rlh_emission = trigonometry::vector3(0.0000055f, 0.000013f, 0.0000224f);
					trigonometry::vector3 mie_emission = 0.000021f;
					float rlh_height = 8000.0f;
					float mie_height = 1200.0f;
					float mie_direction = 0.94f;
					float inner_radius = 6371000.0f;
					float outer_radius = 6471000.0f;
					float intensity = 7.0f;
				} sky;

				struct shadow_info
				{
					float distance[6] = { 25.0f, 50.0f, 100.0f, 175.0f, 250.0f, 325.0f };
					float softness = 1.0f;
					float bias = 0.0f;
					float near = 6.0f;
					float far = 9.0f;
					uint32_t iterations = 2;
					uint32_t cascades = 3;
					bool enabled = false;
				} shadow;

			public:
				depth_cascade_map* depth_map = nullptr;
				trigonometry::matrix4x4 projection[6];
				trigonometry::matrix4x4 view[6];
				trigonometry::vector3 diffuse = 1.0f;
				float emission = 1.0f;
				float disperse = 0.0f;

			public:
				line_light(entity* ref);
				void deserialize(core::schema* node) override;
				void serialize(core::schema* node) override;
				void message(const std::string_view& name, core::variant_args& args) override;
				core::unique<component> copy(entity* init) const override;
				void generate_origin();

			public:
				VI_COMPONENT("line_light_component");
			};

			class surface_light final : public component
			{
			private:
				graphics::texture_2d* diffuse_map_x[2] = { nullptr };
				graphics::texture_2d* diffuse_map_y[2] = { nullptr };
				graphics::texture_2d* diffuse_map_z[2] = { nullptr };
				graphics::texture_2d* diffuse_map = nullptr;
				graphics::texture_cube* probe = nullptr;

			private:
				attenuation size;

			public:
				trigonometry::matrix4x4 view[6];
				trigonometry::matrix4x4 projection;
				trigonometry::vector3 offset = trigonometry::vector3(1.0f, 1.0f, 1.0f);
				trigonometry::vector3 diffuse = 1.0f;
				ticker tick;
				float emission = 1.0f;
				float infinity = 0.0f;
				bool parallax = false;
				bool locked = false;
				bool static_mask = false;

			public:
				surface_light(entity* ref);
				~surface_light() override;
				void deserialize(core::schema* node) override;
				void serialize(core::schema* node) override;
				size_t get_unit_bounds(trigonometry::vector3& min, trigonometry::vector3& max) const override;
				float get_visibility(const viewer& view, float distance) const override;
				core::unique<component> copy(entity* init) const override;
				void set_probe_cache(core::unique<graphics::texture_cube> new_cache);
				void set_size(const attenuation& value);
				bool set_diffuse_map(graphics::texture_2d* map);
				bool set_diffuse_map(graphics::texture_2d* const map_x[2], graphics::texture_2d* const map_y[2], graphics::texture_2d* const map_z[2]);
				bool is_image_based() const;
				const attenuation& get_size();
				graphics::texture_cube* get_probe_cache() const;
				graphics::texture_2d* get_diffuse_map_xp();
				graphics::texture_2d* get_diffuse_map_xn();
				graphics::texture_2d* get_diffuse_map_yp();
				graphics::texture_2d* get_diffuse_map_yn();
				graphics::texture_2d* get_diffuse_map_zp();
				graphics::texture_2d* get_diffuse_map_zn();
				graphics::texture_2d* get_diffuse_map();

			public:
				VI_COMPONENT("surface_light_component");
			};

			class illuminator final : public component
			{
			public:
				graphics::texture_3d* voxel_map;
				ticker inside;
				ticker outside;
				float ray_step;
				float max_steps;
				float distance;
				float radiance;
				float length;
				float margin;
				float offset;
				float angle;
				float occlusion;
				float specular;
				float bleeding;
				bool regenerate;

			public:
				illuminator(entity* ref);
				void deserialize(core::schema* node) override;
				void serialize(core::schema* node) override;
				void message(const std::string_view& name, core::variant_args& args) override;
				core::unique<component> copy(entity* init) const override;

			public:
				VI_COMPONENT("illuminator_component");
			};

			class camera final : public component
			{
			public:
				enum class projection_mode
				{
					perspective,
					orthographic
				} mode;

			protected:
				render_system* renderer = nullptr;
				trigonometry::matrix4x4 projection;
				graphics::viewport viewport;
				viewer view;

			public:
				float near_plane = 0.1f;
				float far_plane = 250.0f;
				float width = -1;
				float height = -1;
				float field_of_view = 75.0f;

			public:
				camera(entity* ref);
				~camera() override;
				void deserialize(core::schema* node) override;
				void serialize(core::schema* node) override;
				void activate(component* init) override;
				void synchronize(core::timer* time) override;
				core::unique<component> copy(entity* init) const override;
				void get_viewer(viewer* view);
				void resize_buffers();
				viewer& get_viewer();
				render_system* get_renderer();
				trigonometry::matrix4x4 get_projection();
				trigonometry::matrix4x4 get_view_projection();
				trigonometry::matrix4x4 get_view();
				trigonometry::vector3 get_view_position();
				trigonometry::frustum8c get_frustum8c();
				trigonometry::frustum6p get_frustum6p();
				trigonometry::ray get_screen_ray(const trigonometry::vector2& position);
				trigonometry::ray get_cursor_ray();
				float get_distance(entity* other);
				float get_width();
				float get_height();
				float get_aspect();
				bool ray_test(const trigonometry::ray& ray, entity* other, trigonometry::vector3* hit = nullptr);
				bool ray_test(const trigonometry::ray& ray, const trigonometry::matrix4x4& world, trigonometry::vector3* hit = nullptr);

			public:
				VI_COMPONENT("camera_component");
			};

			class scriptable final : public component
			{
			public:
				enum class source_type
				{
					resource,
					memory
				};

				enum class invoke_type
				{
					typeless,
					normal
				};

			protected:
				struct
				{
					asIScriptFunction* serialize = nullptr;
					asIScriptFunction* deserialize = nullptr;
					asIScriptFunction* awake = nullptr;
					asIScriptFunction* asleep = nullptr;
					asIScriptFunction* synchronize = nullptr;
					asIScriptFunction* animate = nullptr;
					asIScriptFunction* update = nullptr;
					asIScriptFunction* message = nullptr;
				} entry;

			protected:
				scripting::compiler* compiler;
				core::string resource;
				core::string library;
				source_type source;
				invoke_type invoke;

			public:
				scriptable(entity* ref);
				~scriptable() override;
				void deserialize(core::schema* node) override;
				void serialize(core::schema* node) override;
				void activate(component* init) override;
				void deactivate() override;
				void update(core::timer* time) override;
				void message(const std::string_view& name, core::variant_args& args) override;
				core::unique<component> copy(entity* init) const override;
				scripting::expects_promise_vm<scripting::execution> call(const std::string_view& name, size_t args, scripting::args_callback&& on_args);
				scripting::expects_promise_vm<scripting::execution> call(asIScriptFunction* entry, scripting::args_callback&& on_args);
				scripting::expects_promise_vm<scripting::execution> call_entry(const std::string_view& name);
				scripting::expects_promise_vm<void> load_source();
				scripting::expects_promise_vm<void> load_source(source_type type, const std::string_view& source);
				scripting::expects_vm<size_t> get_properties_count();
				scripting::expects_vm<size_t> get_functions_count();
				void set_invocation(invoke_type type);
				void unload_source();
				bool get_property_by_name(const std::string_view& name, scripting::property_info* result);
				bool get_property_by_index(size_t index, scripting::property_info* result);
				scripting::function get_function_by_name(const std::string_view& name, size_t args);
				scripting::function get_function_by_index(size_t index, size_t args);
				scripting::compiler* get_compiler();
				source_type get_source_type();
				invoke_type get_invoke_type();
				const core::string& get_source();
				const core::string& get_module_name();

			public:
				template <typename t>
				scripting::expects_vm<void> set_type_property_by_name(const std::string_view& name, const t& value)
				{
					VI_ASSERT(compiler != nullptr, "compiler should be set");
					scripting::library base = compiler->get_module();
					if (!base.is_valid())
						return scripting::virtual_exception(scripting::virtual_error::no_module);

					auto index = base.get_property_index_by_name(name);
					if (!index)
						return index.error();

					t* address = (t*)base.get_address_of_property(*index);
					if (!address)
						return scripting::virtual_exception(scripting::virtual_error::invalid_object);

					*address = value;
					return core::expectation::met;
				}
				template <typename t>
				scripting::expects_vm<void> set_ref_property_by_name(const std::string_view& name, t* value)
				{
					VI_ASSERT(compiler != nullptr, "compiler should be set");
					scripting::library base = compiler->get_module();
					if (!base.is_valid())
						return scripting::virtual_exception(scripting::virtual_error::no_module);

					auto index = base.get_property_index_by_name(name);
					if (!index)
						return index.error();

					t** address = (t**)base.get_address_of_property(*index);
					if (!address)
						return scripting::virtual_exception(scripting::virtual_error::invalid_object);

					core::memory::release(*address);
					*address = value;
					if (*address != nullptr)
						(*address)->add_ref();
					return core::expectation::met;
				}
				template <typename t>
				scripting::expects_vm<void> set_type_property_by_index(size_t index, const t& value)
				{
					VI_ASSERT(index >= 0, "index should be greater or equal to zero");
					VI_ASSERT(compiler != nullptr, "compiler should be set");

					scripting::library base = compiler->get_module();
					if (!base.is_valid())
						return scripting::virtual_exception(scripting::virtual_error::no_module);

					t* address = (t*)base.get_address_of_property(index);
					if (!address)
						return scripting::virtual_exception(scripting::virtual_error::invalid_object);

					*address = value;
					return core::expectation::met;
				}
				template <typename t>
				scripting::expects_vm<void> set_ref_property_by_index(size_t index, t* value)
				{
					VI_ASSERT(index >= 0, "index should be greater or equal to zero");
					VI_ASSERT(compiler != nullptr, "compiler should be set");

					scripting::library base = compiler->get_module();
					if (!base.is_valid())
						return scripting::virtual_exception(scripting::virtual_error::invalid_configuration);

					t** address = (t**)base.get_address_of_property(index);
					if (!address)
						return scripting::virtual_exception(scripting::virtual_error::invalid_object);

					core::memory::release(*address);
					*address = value;
					if (*address != nullptr)
						(*address)->add_ref();
					return core::expectation::met;
				}

			private:
				scripting::expects_promise_vm<scripting::execution> deserialize_call(core::schema* node);
				scripting::expects_promise_vm<scripting::execution> serialize_call(core::schema* node);
				void protect();
				void unprotect();

			public:
				VI_COMPONENT("scriptable_component");
			};
		}
	}
}
#endif