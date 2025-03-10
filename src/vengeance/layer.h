#ifndef VI_LAYER_H_EXTENSION
#define VI_LAYER_H_EXTENSION
#include <vitex/layer.h>
#include "graphics.h"
#include "audio.h"
#include "physics.h"

namespace vitex
{
	namespace layer
	{
		namespace gui
		{
			class context;
		}

		typedef graphics::depth_target_2d depth_map;
		typedef graphics::depth_target_cube depth_cube_map;
		typedef core::vector<depth_map*> depth_cascade_map;
		typedef std::function<void(core::timer*, struct viewer*)> render_callback;
		typedef std::function<void(const std::string_view&, core::variant_args&)> message_callback;
		typedef std::function<bool(class component*, const trigonometry::vector3&)> ray_callback;
		typedef std::function<bool(graphics::render_target*)> target_callback;

		class heavy_series;

		class scene_graph;

		class heavy_content_manager;

		class heavy_application;

		class entity;

		class component;

		class drawable;

		class primitive_cache;

		class render_system;

		class material;

		class skin_model;

		enum
		{
			USE_GRAPHICS = 1 << 3,
			USE_ACTIVITY = 1 << 4,
			USE_AUDIO = 1 << 5,
			MAX_STACK_DEPTH = 4,
			THRESHOLD_PER_ELEMENT = 48,
			THRESHOLD_PER_THREAD = 1
		};

		enum class render_opt
		{
			none = 0,
			transparent = 1,
			constant = 2,
			additive = 4,
			backfaces = 8
		};

		enum class render_culling
		{
			depth,
			depth_cube,
			disable
		};

		enum class render_state
		{
			geometry,
			depth,
			depth_cube
		};

		enum class geo_category
		{
			opaque,
			transparent,
			additive,
			count
		};

		enum class buffer_type
		{
			index = 0,
			vertex = 1
		};

		enum class target_type
		{
			main = 0,
			secondary = 1,
			count
		};

		enum class event_target
		{
			scene = 0,
			entity = 1,
			component = 2,
			listener = 3
		};

		enum class actor_set
		{
			none = 0,
			update = 1 << 0,
			synchronize = 1 << 1,
			animate = 1 << 2,
			message = 1 << 3,
			cullable = 1 << 4,
			drawable = 1 << 5
		};

		enum class actor_type
		{
			update,
			synchronize,
			animate,
			message,
			count
		};

		enum class task_type
		{
			processing,
			rendering,
			count
		};

		enum class composer_tag
		{
			component,
			renderer,
			effect,
			filter
		};

		enum class render_buffer_type
		{
			animation,
			render,
			view
		};

		inline actor_set operator |(actor_set a, actor_set b)
		{
			return static_cast<actor_set>(static_cast<size_t>(a) | static_cast<size_t>(b));
		}
		inline render_opt operator |(render_opt a, render_opt b)
		{
			return static_cast<render_opt>(static_cast<size_t>(a) | static_cast<size_t>(b));
		}

		struct ticker
		{
		private:
			float time;

		public:
			float delay;

		public:
			ticker() noexcept;
			bool tick_event(float elapsed_time);
			float get_time();
		};

		struct event
		{
			core::string name;
			core::variant_args args;

			event(const std::string_view& new_name) noexcept;
			event(const std::string_view& new_name, const core::variant_args& new_args) noexcept;
			event(const std::string_view& new_name, core::variant_args&& new_args) noexcept;
			event(const event& other) noexcept;
			event(event&& other) noexcept;
			event& operator= (const event& other) noexcept;
			event& operator= (event&& other) noexcept;
		};

		struct batch_data
		{
			graphics::element_buffer* instance_buffer;
			void* geometry_buffer;
			material* batch_material;
			size_t instances_count;
		};

		struct idx_snapshot
		{
			core::unordered_map<entity*, size_t> to;
			core::unordered_map<size_t, entity*> from;
		};

		struct visibility_query
		{
			geo_category category = geo_category::opaque;
			bool boundary_visible = false;
			bool query_pixels = false;
		};

		struct animator_state
		{
			bool paused = false;
			bool looped = false;
			bool blended = false;
			float duration = -1.0f;
			float rate = 1.0f;
			float time = 0.0f;
			int64_t frame = -1;
			int64_t clip = -1;

			float get_timeline(core::timer* time) const;
			float get_seconds_duration() const;
			float get_progress_total() const;
			float get_progress() const;
			bool is_playing() const;
		};

		struct spawner_properties
		{
			trigonometry::random_vector4 diffusion;
			trigonometry::random_vector3 position;
			trigonometry::random_vector3 velocity;
			trigonometry::random_vector3 noise;
			trigonometry::random_float rotation;
			trigonometry::random_float scale;
			trigonometry::random_float angular;
			int iterations = 1;
		};

		struct viewer
		{
			render_system* renderer = nullptr;
			render_culling culling = render_culling::depth;
			trigonometry::matrix4x4 cube_view_projection[6];
			trigonometry::matrix4x4 inv_view_projection;
			trigonometry::matrix4x4 view_projection;
			trigonometry::matrix4x4 projection;
			trigonometry::matrix4x4 view;
			trigonometry::vector3 inv_position;
			trigonometry::vector3 position;
			trigonometry::vector3 rotation;
			float far_plane = 0.0f;
			float near_plane = 0.0f;
			float ratio = 0.0f;
			float fov = 0.0f;

			void set(const trigonometry::matrix4x4& view, const trigonometry::matrix4x4& projection, const trigonometry::vector3& position, float fov, float ratio, float near, float far, render_culling type);
			void set(const trigonometry::matrix4x4& view, const trigonometry::matrix4x4& projection, const trigonometry::vector3& position, const trigonometry::vector3& rotation, float fov, float ratio, float near, float far, render_culling type);
		};

		struct attenuation
		{
			float radius = 10.0f;
			float C1 = 0.6f;
			float C2 = 0.6f;
		};

		struct subsurface
		{
			trigonometry::vector4 emission = { 0.0f, 0.0f, 0.0f, 0.0f };
			trigonometry::vector4 metallic = { 0.0f, 0.0f, 0.0f, 0.0f };
			trigonometry::vector4 penetration = { 0.75f, 0.75f, 0.75f, 0.0f };
			trigonometry::vector3 diffuse = { 1.0f, 1.0f, 1.0f };
			float fresnel = 0.0f;
			trigonometry::vector3 scattering = { 1.0f, 0.25f, 0.04f };
			float transparency = 0.0f;
			trigonometry::vector3 padding;
			float bias = 0.0f;
			trigonometry::vector2 roughness = { 1.0f, 0.0f };
			float refraction = 0.0f;
			float environment = 0.0f;
			trigonometry::vector2 occlusion = { 1.0f, 0.0f };
			float radius = 0.0f;
			float height = 0.0f;
		};

		struct sparse_index
		{
			core::pool<component*> data;
			trigonometry::cosmos index;
		};

		struct animation_buffer
		{
			trigonometry::matrix4x4 offsets[graphics::joints_size];
			trigonometry::vector3 padding;
			float animated = 0.0f;
		};

		struct render_buffer
		{
			struct instance
			{
				trigonometry::matrix4x4 transform;
				trigonometry::matrix4x4 world;
				trigonometry::vector2 texcoord;
				float diffuse = 0.0f;
				float normal = 0.0f;
				float height = 0.0f;
				float material_id = 0.0f;
			};

			trigonometry::matrix4x4 transform;
			trigonometry::matrix4x4 world;
			trigonometry::vector4 texcoord;
			float diffuse = 0.0f;
			float normal = 0.0f;
			float height = 0.0f;
			float material_id = 0.0f;
		};

		struct view_buffer
		{
			trigonometry::matrix4x4 inv_view_proj;
			trigonometry::matrix4x4 view_proj;
			trigonometry::matrix4x4 proj;
			trigonometry::matrix4x4 view;
			trigonometry::vector3 position;
			float far = 1000.0f;
			trigonometry::vector3 direction;
			float near = 0.1f;
		};

		struct pose_node
		{
			trigonometry::vector3 position;
			trigonometry::vector3 scale = trigonometry::vector3::one();
			trigonometry::quaternion rotation;
		};

		struct pose_data
		{
			pose_node frame;
			pose_node offset;
			pose_node defaults;
		};

		struct pose_matrices
		{
			trigonometry::matrix4x4 data[graphics::joints_size];
		};

		struct pose_buffer
		{
			core::unordered_map<graphics::skin_mesh_buffer*, pose_matrices> matrices;
			core::unordered_map<size_t, pose_data> offsets;

			void fill(skin_model* mesh);
			void fill(trigonometry::joint& next);
		};

		class heavy_series
		{
		public:
			static void pack(core::schema* v, const trigonometry::vector2& value);
			static void pack(core::schema* v, const trigonometry::vector3& value);
			static void pack(core::schema* v, const trigonometry::vector4& value);
			static void pack(core::schema* v, const trigonometry::quaternion& value);
			static void pack(core::schema* v, const trigonometry::matrix4x4& value);
			static void pack(core::schema* v, const attenuation& value);
			static void pack(core::schema* v, const animator_state& value);
			static void pack(core::schema* v, const spawner_properties& value);
			static void pack(core::schema* v, const trigonometry::key_animator_clip& value);
			static void pack(core::schema* v, const trigonometry::animator_key& value);
			static void pack(core::schema* v, const trigonometry::skin_animator_key& value);
			static void pack(core::schema* v, const trigonometry::element_vertex& value);
			static void pack(core::schema* v, const trigonometry::joint& value);
			static void pack(core::schema* v, const trigonometry::vertex& value);
			static void pack(core::schema* v, const trigonometry::skin_vertex& value);
			static void pack(core::schema* v, const ticker& value);
			static void pack(core::schema* v, const core::vector<trigonometry::vector2>& value);
			static void pack(core::schema* v, const core::vector<trigonometry::vector3>& value);
			static void pack(core::schema* v, const core::vector<trigonometry::vector4>& value);
			static void pack(core::schema* v, const core::vector<trigonometry::matrix4x4>& value);
			static void pack(core::schema* v, const core::vector<animator_state>& value);
			static void pack(core::schema* v, const core::vector<spawner_properties>& value);
			static void pack(core::schema* v, const core::vector<trigonometry::key_animator_clip>& value);
			static void pack(core::schema* v, const core::vector<trigonometry::animator_key>& value);
			static void pack(core::schema* v, const core::vector<trigonometry::element_vertex>& value);
			static void pack(core::schema* v, const core::vector<trigonometry::joint>& value);
			static void pack(core::schema* v, const core::vector<trigonometry::vertex>& value);
			static void pack(core::schema* v, const core::vector<trigonometry::skin_vertex>& value);
			static void pack(core::schema* v, const core::vector<ticker>& value);
			static bool unpack(core::schema* v, trigonometry::vector2* o);
			static bool unpack(core::schema* v, trigonometry::vector3* o);
			static bool unpack(core::schema* v, trigonometry::vector4* o);
			static bool unpack(core::schema* v, trigonometry::quaternion* o);
			static bool unpack(core::schema* v, trigonometry::matrix4x4* o);
			static bool unpack(core::schema* v, attenuation* o);
			static bool unpack(core::schema* v, animator_state* o);
			static bool unpack(core::schema* v, spawner_properties* o);
			static bool unpack(core::schema* v, trigonometry::key_animator_clip* o);
			static bool unpack(core::schema* v, trigonometry::animator_key* o);
			static bool unpack(core::schema* v, trigonometry::skin_animator_key* o);
			static bool unpack(core::schema* v, trigonometry::element_vertex* o);
			static bool unpack(core::schema* v, trigonometry::joint* o);
			static bool unpack(core::schema* v, trigonometry::vertex* o);
			static bool unpack(core::schema* v, trigonometry::skin_vertex* o);
			static bool unpack(core::schema* v, ticker* o);
			static bool unpack(core::schema* v, core::vector<trigonometry::vector2>* o);
			static bool unpack(core::schema* v, core::vector<trigonometry::vector3>* o);
			static bool unpack(core::schema* v, core::vector<trigonometry::vector4>* o);
			static bool unpack(core::schema* v, core::vector<trigonometry::matrix4x4>* o);
			static bool unpack(core::schema* v, core::vector<animator_state>* o);
			static bool unpack(core::schema* v, core::vector<spawner_properties>* o);
			static bool unpack(core::schema* v, core::vector<trigonometry::key_animator_clip>* o);
			static bool unpack(core::schema* v, core::vector<trigonometry::animator_key>* o);
			static bool unpack(core::schema* v, core::vector<trigonometry::element_vertex>* o);
			static bool unpack(core::schema* v, core::vector<trigonometry::joint>* o);
			static bool unpack(core::schema* v, core::vector<trigonometry::vertex>* o);
			static bool unpack(core::schema* v, core::vector<trigonometry::skin_vertex>* o);
			static bool unpack(core::schema* v, core::vector<ticker>* o);
		};

		class model final : public core::reference<model>
		{
		public:
			core::vector<graphics::mesh_buffer*> meshes;
			trigonometry::vector4 max;
			trigonometry::vector4 min;

		public:
			model() noexcept;
			~model() noexcept;
			void cleanup();
			graphics::mesh_buffer* find_mesh(const std::string_view& name);
		};

		class skin_model final : public core::reference<skin_model>
		{
		public:
			core::vector<graphics::skin_mesh_buffer*> meshes;
			trigonometry::joint skeleton;
			trigonometry::matrix4x4 inv_transform;
			trigonometry::matrix4x4 transform;
			trigonometry::vector4 max;
			trigonometry::vector4 min;

		public:
			skin_model() noexcept;
			~skin_model() noexcept;
			bool find_joint(const std::string_view& name, trigonometry::joint* output);
			bool find_joint(size_t index, trigonometry::joint* output);
			void synchronize(pose_buffer* map);
			void cleanup();
			graphics::skin_mesh_buffer* find_mesh(const std::string_view& name);

		private:
			void synchronize(pose_buffer* map, trigonometry::joint& next, const trigonometry::matrix4x4& parent_offset);
		};

		class skin_animation final : public core::reference<skin_animation>
		{
		private:
			core::vector<trigonometry::skin_animator_clip> clips;

		public:
			skin_animation(core::vector<trigonometry::skin_animator_clip>&& data) noexcept;
			~skin_animation() = default;
			const core::vector<trigonometry::skin_animator_clip>& get_clips();
			bool is_valid();
		};

		class material final : public core::reference<material>
		{
			friend heavy_series;
			friend render_system;
			friend scene_graph;

		public:
			struct slots
			{
				uint32_t diffuse_map = (uint32_t)-1;
				uint32_t normal_map = (uint32_t)-1;
				uint32_t metallic_map = (uint32_t)-1;
				uint32_t roughness_map = (uint32_t)-1;
				uint32_t height_map = (uint32_t)-1;
				uint32_t occlusion_map = (uint32_t)-1;
				uint32_t emission_map = (uint32_t)-1;
			};

		private:
			graphics::texture_2d* diffuse_map;
			graphics::texture_2d* normal_map;
			graphics::texture_2d* metallic_map;
			graphics::texture_2d* roughness_map;
			graphics::texture_2d* height_map;
			graphics::texture_2d* occlusion_map;
			graphics::texture_2d* emission_map;
			core::string name;
			scene_graph* scene;

		public:
			subsurface surface;
			size_t slot;

		public:
			material(scene_graph* new_scene = nullptr) noexcept;
			material(const material& other) noexcept;
			~material() noexcept;
			void set_name(const std::string_view& value);
			const core::string& get_name() const;
			void set_diffuse_map(graphics::texture_2d* init);
			graphics::texture_2d* get_diffuse_map() const;
			void set_normal_map(graphics::texture_2d* init);
			graphics::texture_2d* get_normal_map() const;
			void set_metallic_map(graphics::texture_2d* init);
			graphics::texture_2d* get_metallic_map() const;
			void set_roughness_map(graphics::texture_2d* init);
			graphics::texture_2d* get_roughness_map() const;
			void set_height_map(graphics::texture_2d* init);
			graphics::texture_2d* get_height_map() const;
			void set_occlusion_map(graphics::texture_2d* init);
			graphics::texture_2d* get_occlusion_map() const;
			void set_emission_map(graphics::texture_2d* init);
			graphics::texture_2d* get_emission_map() const;
			scene_graph* get_scene() const;
		};

		class component : public core::reference<component>
		{
			friend core::reference<component>;
			friend scene_graph;
			friend render_system;
			friend entity;

		protected:
			entity* parent;

		private:
			size_t set;
			bool indexed;
			bool active;

		public:
			virtual void serialize(core::schema* node);
			virtual void deserialize(core::schema* node);
			virtual void activate(component* init);
			virtual void deactivate();
			virtual void synchronize(core::timer* time);
			virtual void animate(core::timer* time);
			virtual void update(core::timer* time);
			virtual void message(const std::string_view& name, core::variant_args& args);
			virtual void movement();
			virtual size_t get_unit_bounds(trigonometry::vector3& min, trigonometry::vector3& max) const;
			virtual float get_visibility(const viewer& view, float distance) const;
			virtual core::unique<component> copy(entity* init) const = 0;
			entity* get_entity() const;
			void set_active(bool enabled);
			bool is_drawable() const;
			bool is_cullable() const;
			bool is_active() const;

		protected:
			component(entity* ref, actor_set rule) noexcept;
			virtual ~component() noexcept;

		public:
			VI_COMPONENT_ROOT("base_component");
		};

		class entity final : public core::reference<entity>
		{
			friend core::reference<entity>;
			friend scene_graph;
			friend render_system;

		private:
			struct
			{
				trigonometry::matrix4x4 box;
				trigonometry::vector3 min;
				trigonometry::vector3 max;
				float distance = 0.0f;
				float visibility = 0.0f;
			} snapshot;

			struct
			{
				core::unordered_map<uint64_t, component*> components;
				core::string name;
			} type;

		private:
			trigonometry::transform* transform;
			scene_graph* scene;
			bool active;

		public:
			void set_name(const std::string_view& value);
			void set_root(entity* parent);
			void update_bounds();
			void remove_component(uint64_t id);
			void remove_childs();
			component* add_component(core::unique<component> in);
			component* get_component(uint64_t id);
			size_t get_components_count() const;
			scene_graph* get_scene() const;
			entity* get_parent() const;
			entity* get_child(size_t index) const;
			trigonometry::transform* get_transform() const;
			const trigonometry::matrix4x4& get_box() const;
			const trigonometry::vector3& get_min() const;
			const trigonometry::vector3& get_max() const;
			const core::string& get_name() const;
			size_t get_childs_count() const;
			float get_visibility(const viewer& base) const;
			bool is_active() const;
			trigonometry::vector3 get_radius3() const;
			float get_radius() const;

		private:
			entity(scene_graph* new_scene) noexcept;
			~entity() noexcept;

		public:
			core::unordered_map<uint64_t, component*>::iterator begin()
			{
				return type.components.begin();
			}
			core::unordered_map<uint64_t, component*>::iterator end()
			{
				return type.components.end();
			}

		public:
			template <typename in>
			void remove_component()
			{
				remove_component(in::get_type_id());
			}
			template <typename in>
			in* add_component()
			{
				return (in*)add_component(new in(this));
			}
			template <typename in>
			in* get_component()
			{
				return (in*)get_component(in::get_type_id());
			}

		public:
			template <typename t>
			static bool sortout(t* a, t* b)
			{
				return a->parent->snapshot.distance - b->parent->snapshot.distance < 0;
			}
		};

		class drawable : public component
		{
			friend scene_graph;

		protected:
			core::unordered_map<void*, material*> materials;

		private:
			geo_category category;
			uint64_t source;

		public:
			float overlapping;
			bool constant;

		public:
			drawable(entity* ref, actor_set rule, uint64_t hash) noexcept;
			virtual ~drawable() noexcept;
			virtual void message(const std::string_view& name, core::variant_args& args) override;
			virtual void movement() override;
			virtual core::unique<component> copy(entity* init) const override = 0;
			void clear_materials();
			bool set_category(geo_category new_category);
			bool set_material(void* instance, material* value);
			bool set_material(material* value);
			geo_category get_category() const;
			int64_t get_slot(void* surface);
			int64_t get_slot();
			material* get_material(void* surface);
			material* get_material();
			const core::unordered_map<void*, material*>& get_materials();

		public:
			VI_COMPONENT("drawable_component");
		};

		class renderer : public core::reference<renderer>
		{
			friend scene_graph;

		protected:
			render_system* system;

		public:
			bool active;

		public:
			renderer(render_system* lab) noexcept;
			virtual ~renderer() noexcept;
			virtual void serialize(core::schema* node);
			virtual void deserialize(core::schema* node);
			virtual void clear_culling();
			virtual void resize_buffers();
			virtual void activate();
			virtual void deactivate();
			virtual void begin_pass(core::timer* time);
			virtual void end_pass();
			virtual bool has_category(geo_category category);
			virtual size_t render_prepass(core::timer* time);
			virtual size_t render_pass(core::timer* time) = 0;
			void set_renderer(render_system* new_system);
			render_system* get_renderer() const;

		public:
			VI_COMPONENT_ROOT("base_renderer");
		};

		class render_constants final : public core::reference<render_constants>
		{
		private:
			struct
			{
				graphics::element_buffer* buffers[3] = { nullptr };
				graphics::shader* basic_effect = nullptr;
				void* pointers[3] = { nullptr };
				size_t sizes[3] = { 0 };
			} binding;

		private:
			graphics::graphics_device* device;

		public:
			animation_buffer animation;
			render_buffer render;
			view_buffer view;

		public:
			render_constants(graphics::graphics_device* new_device) noexcept;
			~render_constants() noexcept;
			void set_constant_buffer(render_buffer_type buffer, uint32_t slot, uint32_t type);
			void set_updated_constant_buffer(render_buffer_type buffer, uint32_t slot, uint32_t type);
			void update_constant_buffer(render_buffer_type buffer);
			graphics::shader* get_basic_effect() const;
			graphics::graphics_device* get_device() const;
			graphics::element_buffer* get_constant_buffer(render_buffer_type buffer) const;
		};

		class render_system final : public core::reference<render_system>
		{
		public:
			struct rs_index
			{
				trigonometry::cosmos::iterator stack;
				trigonometry::frustum6p frustum;
				trigonometry::bounding bounds;
				core::vector<void*> queue;
			} indexing;

			struct rs_state
			{
				friend render_system;

			private:
				render_state target = render_state::geometry;
				render_opt options = render_opt::none;
				size_t top = 0;

			public:
				bool is(render_state state) const
				{
					return target == state;
				}
				bool is_set(render_opt option) const
				{
					return (size_t)options & (size_t)option;
				}
				bool is_top() const
				{
					return top <= 1;
				}
				bool is_subpass() const
				{
					return !is_top();
				}
				render_opt get_opts() const
				{
					return options;
				}
				render_state get() const
				{
					return target;
				}
			} state;

		protected:
			core::vector<renderer*> renderers;
			graphics::graphics_device* device;
			material* base_material;
			scene_graph* scene;
			component* owner;

		public:
			render_constants* constants;
			viewer view;
			size_t max_queries;
			size_t sorting_frequency;
			size_t occlusion_skips;
			size_t occluder_skips;
			size_t occludee_skips;
			float occludee_scaling;
			float overflow_visibility;
			float threshold;
			bool occlusion_culling;
			bool precise_culling;
			bool allow_input_lag;

		public:
			render_system(scene_graph* new_scene, component* new_component) noexcept;
			~render_system() noexcept;
			void set_view(const trigonometry::matrix4x4& view, const trigonometry::matrix4x4& projection, const trigonometry::vector3& position, float fov, float ratio, float near, float far, render_culling type);
			void clear_culling();
			void remove_renderers();
			void restore_view_buffer(viewer* view);
			void remount(renderer* target);
			void remount();
			void mount();
			void unmount();
			void move_renderer(uint64_t id, size_t offset);
			void remove_renderer(uint64_t id);
			void restore_output();
			void free_shader(const std::string_view& name, graphics::shader* shader);
			void free_shader(graphics::shader* shader);
			void free_buffers(const std::string_view& name, graphics::element_buffer** buffers);
			void free_buffers(graphics::element_buffer** buffers);
			void set_constant_buffer(render_buffer_type buffer, uint32_t slot, uint32_t type);
			void set_updated_constant_buffer(render_buffer_type buffer, uint32_t slot, uint32_t type);
			void update_constant_buffer(render_buffer_type buffer);
			void clear_materials();
			void fetch_visibility(component* base, visibility_query& data);
			size_t render(core::timer* time, render_state stage, render_opt options);
			bool try_instance(material* next, render_buffer::instance& target);
			bool try_geometry(material* next, material::slots* slotdata);
			bool has_category(geo_category category);
			graphics::expects_graphics<graphics::shader*> compile_shader(graphics::shader::desc& desc, size_t buffer_size = 0);
			graphics::expects_graphics<graphics::shader*> compile_shader(const std::string_view& section_name, core::vector<core::string>&& features, size_t buffer_size = 0);
			graphics::expects_graphics<void> compile_buffers(graphics::element_buffer** result, const std::string_view& name, size_t element_size, size_t elements_count);
			renderer* add_renderer(core::unique<renderer> in);
			renderer* get_renderer(uint64_t id);
			bool get_offset(uint64_t id, size_t& offset) const;
			core::vector<renderer*>& get_renderers();
			graphics::multi_render_target_2d* get_mrt(target_type type) const;
			graphics::render_target_2d* get_rt(target_type type) const;
			graphics::element_buffer* get_material_buffer() const;
			graphics::texture_2d** get_merger();
			graphics::graphics_device* get_device() const;
			graphics::shader* get_basic_effect() const;
			render_constants* get_constants() const;
			primitive_cache* get_primitives() const;
			scene_graph* get_scene() const;
			component* get_component() const;

		private:
			sparse_index& get_storage_wrapper(uint64_t section);
			void watch(core::vector<core::promise<void>>&& tasks);

		private:
			template <typename t, typename overlaps_function, typename match_function>
			void query_dispatch(trigonometry::cosmos& index, const overlaps_function& overlaps, const match_function& match)
			{
				indexing.stack.clear();
				if (!index.empty())
					indexing.stack.push_back(index.get_root());

				while (!indexing.stack.empty())
				{
					auto& next = index.get_node(indexing.stack.back());
					indexing.stack.pop_back();

					if (overlaps(next.bounds))
					{
						if (!next.is_leaf())
						{
							indexing.stack.push_back(next.left);
							indexing.stack.push_back(next.right);
						}
						else if (next.item != nullptr)
							match((t*)next.item);
					}
				}
			}
			template <typename t, typename overlaps_function, typename match_function>
			void parallel_query_dispatch(trigonometry::cosmos& index, const overlaps_function& overlaps, const match_function& match)
			{
				indexing.stack.clear();
				indexing.queue.clear();

				if (!index.empty())
					indexing.stack.push_back(index.get_root());

				while (!indexing.stack.empty())
				{
					auto& next = index.get_node(indexing.stack.back());
					indexing.stack.pop_back();

					if (overlaps(next.bounds))
					{
						if (!next.is_leaf())
						{
							indexing.stack.push_back(next.left);
							indexing.stack.push_back(next.right);
						}
						else if (next.item != nullptr)
							indexing.queue.push_back(next.item);
					}
				}

				if (indexing.queue.empty())
					return;

				watch(parallel::for_each(indexing.queue.begin(), indexing.queue.end(), THRESHOLD_PER_THREAD, [match](void* item)
				{
					match(parallel::get_thread_index(), (t*)item);
				}));
			}

		public:
			template <typename match_function>
			void query_group(uint64_t id, match_function&& callback)
			{
				auto& storage = get_storage_wrapper(id);
				switch (view.culling)
				{
					case render_culling::depth:
					{
						auto overlaps = [this](const trigonometry::bounding& bounds) { return indexing.frustum.overlaps_aabb(bounds); };
						query_dispatch<component, decltype(overlaps), decltype(callback)>(storage.index, overlaps, callback);
						break;
					}
					case render_culling::depth_cube:
					{
						auto overlaps = [this](const trigonometry::bounding& bounds) { return indexing.bounds.overlaps(bounds); };
						query_dispatch<component, decltype(overlaps), decltype(callback)>(storage.index, overlaps, callback);
						break;
					}
					default:
						std::for_each(storage.data.begin(), storage.data.end(), std::move(callback));
						break;
				}
			}
			template <typename init_function, typename match_function>
			void parallel_query_group(uint64_t id, init_function&& init_callback, match_function&& element_callback)
			{
				auto& storage = get_storage_wrapper(id);
				switch (view.culling)
				{
					case render_culling::depth:
					{
						auto overlaps = [this](const trigonometry::bounding& bounds) { return indexing.frustum.overlaps_aabb(bounds); };
						init_callback(parallel::get_threads());
						parallel_query_dispatch<component, decltype(overlaps), decltype(element_callback)>(storage.index, overlaps, element_callback);
						break;
					}
					case render_culling::depth_cube:
					{
						auto overlaps = [this](const trigonometry::bounding& bounds) { return indexing.bounds.overlaps(bounds); };
						init_callback(parallel::get_threads());
						parallel_query_dispatch<component, decltype(overlaps), decltype(element_callback)>(storage.index, overlaps, element_callback);
						break;
					}
					default:
						if (!storage.data.empty())
							watch(parallel::distribute(storage.data.begin(), storage.data.end(), std::move(init_callback), std::move(element_callback)));
						break;
				}
			}
			template <typename t, typename match_function>
			void query(match_function&& callback)
			{
				auto& storage = get_storage_wrapper(t::get_type_id());
				switch (view.culling)
				{
					case render_culling::depth:
					{
						auto overlaps = [this](const trigonometry::bounding& bounds) { return indexing.frustum.overlaps_aabb(bounds); };
						query_dispatch<t, decltype(overlaps), decltype(callback)>(storage.index, overlaps, callback);
						break;
					}
					case render_culling::depth_cube:
					{
						auto overlaps = [this](const trigonometry::bounding& bounds) { return indexing.bounds.overlaps(bounds); };
						query_dispatch<t, decltype(overlaps), decltype(callback)>(storage.index, overlaps, callback);
						break;
					}
					default:
						std::for_each(storage.data.begin(), storage.data.end(), std::move(callback));
						break;
				}
			}
			template <typename t, typename init_function, typename match_function>
			void parallel_query(init_function&& init_callback, match_function&& element_callback)
			{
				auto& storage = get_storage_wrapper(t::get_type_id());
				switch (view.culling)
				{
					case render_culling::depth:
					{
						auto overlaps = [this](const trigonometry::bounding& bounds) { return indexing.frustum.overlaps_aabb(bounds); };
						init_callback(parallel::get_threads());
						parallel_query_dispatch<t, decltype(overlaps), decltype(element_callback)>(storage.index, overlaps, element_callback);
						break;
					}
					case render_culling::depth_cube:
					{
						auto overlaps = [this](const trigonometry::bounding& bounds) { return indexing.bounds.overlaps(bounds); };
						init_callback(parallel::get_threads());
						parallel_query_dispatch<t, decltype(overlaps), decltype(element_callback)>(storage.index, overlaps, element_callback);
						break;
					}
					default:
						if (!storage.data.empty())
							watch(parallel::distribute(storage.data.begin(), storage.data.end(), THRESHOLD_PER_THREAD, std::move(init_callback), std::move(element_callback)));
						break;
				}
			}
			template <typename t>
			void remove_renderer()
			{
				remove_renderer(t::get_type_id());
			}
			template <typename t, typename... args>
			t* add_renderer(args&& ... data)
			{
				return (t*)add_renderer(new t(this, data...));
			}
			template <typename t>
			t* get_renderer()
			{
				return (t*)get_renderer(t::get_type_id());
			}
			template <typename in>
			int64_t get_offset()
			{
				size_t offset = 0;
				if (!get_offset(in::get_type_id(), offset))
					return -1;

				return (int64_t)offset;
			}
		};

		class shader_cache final : public core::reference<shader_cache>
		{
		public:
			struct scache
			{
				graphics::shader* shader;
				size_t count;
			};

		private:
			core::unordered_map<core::string, scache> cache;
			graphics::graphics_device* device;
			std::mutex exclusive;

		public:
			shader_cache(graphics::graphics_device* device) noexcept;
			~shader_cache() noexcept;
			graphics::expects_graphics<graphics::shader*> compile(const std::string_view& name, const graphics::shader::desc& desc, size_t buffer_size = 0);
			graphics::shader* get(const std::string_view& name);
			core::string find(graphics::shader* shader);
			const core::unordered_map<core::string, scache>& get_caches() const;
			bool has(const std::string_view& name);
			bool free(const std::string_view& name, graphics::shader* shader = nullptr);
			void clear_cache();
		};

		class primitive_cache final : public core::reference<primitive_cache>
		{
		public:
			struct scache
			{
				graphics::element_buffer* buffers[2];
				size_t count;
			};

		private:
			core::unordered_map<core::string, scache> cache;
			graphics::graphics_device* device;
			graphics::element_buffer* sphere[2];
			graphics::element_buffer* cube[2];
			graphics::element_buffer* box[2];
			graphics::element_buffer* skin_box[2];
			graphics::element_buffer* quad;
			model* box_model;
			skin_model* skin_box_model;
			std::recursive_mutex exclusive;

		public:
			primitive_cache(graphics::graphics_device* device) noexcept;
			~primitive_cache() noexcept;
			graphics::expects_graphics<void> compile(graphics::element_buffer** result, const std::string_view& name, size_t element_size, size_t elements_count);
			bool get(graphics::element_buffer** result, const std::string_view& name);
			bool has(const std::string_view& name);
			bool free(const std::string_view& name, graphics::element_buffer** buffers);
			core::string find(graphics::element_buffer** buffer);
			model* get_box_model();
			skin_model* get_skin_box_model();
			graphics::element_buffer* get_quad();
			graphics::element_buffer* get_sphere(buffer_type type);
			graphics::element_buffer* get_cube(buffer_type type);
			graphics::element_buffer* get_box(buffer_type type);
			graphics::element_buffer* get_skin_box(buffer_type type);
			const core::unordered_map<core::string, scache>& get_caches() const;
			void get_sphere_buffers(graphics::element_buffer** result);
			void get_cube_buffers(graphics::element_buffer** result);
			void get_box_buffers(graphics::element_buffer** result);
			void get_skin_box_buffers(graphics::element_buffer** result);
			void clear_cache();
		};

		class scene_graph final : public core::reference<scene_graph>
		{
			friend render_system;
			friend renderer;
			friend component;
			friend entity;
			friend drawable;

		public:
			struct desc
			{
				struct dependencies
				{
					graphics::graphics_device* device = nullptr;
					graphics::activity* activity = nullptr;
					scripting::virtual_machine* vm = nullptr;
					heavy_content_manager* content = nullptr;
					primitive_cache* primitives = nullptr;
					shader_cache* shaders = nullptr;
					render_constants* constants = nullptr;
				} shared;

				physics::simulator::desc simulator;
				size_t start_materials = 1ll << 8;
				size_t start_entities = 1ll << 8;
				size_t start_components = 1ll << 8;
				size_t grow_margin = 128;
				size_t max_updates = 256;
				size_t points_size = 256;
				size_t points_max = 4;
				size_t spots_size = 512;
				size_t spots_max = 8;
				size_t lines_size = 1024;
				size_t lines_max = 2;
				double grow_rate = 0.25f;
				float render_quality = 1.0f;
				bool enable_hdr = false;
				bool mutations = false;

				void add_ref();
				void release();
				static desc get(heavy_application* base);
			};

		private:
			struct
			{
				graphics::multi_render_target_2d* mrt[(size_t)target_type::count * 2];
				graphics::render_target_2d* rt[(size_t)target_type::count * 2];
				graphics::element_buffer* material_buffer;
				graphics::depth_stencil_state* depth_stencil;
				graphics::rasterizer_state* rasterizer;
				graphics::blend_state* blend;
				graphics::sampler_state* sampler;
				graphics::input_layout* layout;
				graphics::texture_2d* merger;
				core::vector<depth_cube_map*> points;
				core::vector<depth_map*> spots;
				core::vector<depth_cascade_map*> lines;
			} display;

			struct
			{
				uint32_t diffuse_map = (uint32_t)-1;
				uint32_t sampler = (uint32_t)-1;
				uint32_t object = (uint32_t)-1;
			} slots;

			struct
			{
				std::atomic<material*> defaults;
				float progress = 1.0f;
			} loading;

			struct
			{
				core::single_queue<core::promise<void>> queue[(size_t)task_type::count];
				std::mutex update[(size_t)task_type::count];
				bool is_rendering = false;
			} tasking;

		protected:
			core::unordered_map<core::string, core::unordered_set<message_callback*>> listeners;
			core::unordered_map<uint64_t, core::unordered_set<component*>> changes;
			core::unordered_map<uint64_t, sparse_index*> registry;
			core::unordered_map<component*, size_t> incomplete;
			core::single_queue<core::task_callback> transactions;
			core::single_queue<event> events;
			core::pool<component*> actors[(size_t)actor_type::count];
			core::pool<material*> materials;
			core::pool<entity*> entities;
			core::pool<entity*> dirty;
			physics::simulator* simulator;
			std::atomic<component*> camera;
			std::atomic<bool> active;
			std::mutex exclusive;
			desc conf;

		public:
			struct sg_statistics
			{
				size_t batching = 0;
				size_t sorting = 0;
				size_t instances = 0;
				size_t draw_calls = 0;
			} statistics;

		public:
			idx_snapshot* snapshot;

		public:
			scene_graph(const desc& i) noexcept;
			~scene_graph() noexcept;
			void configure(const desc& conf);
			void actualize();
			void resize_buffers();
			void submit();
			void dispatch(core::timer* time);
			void publish(core::timer* time);
			void publish_and_submit(core::timer* time, float r, float g, float b, bool is_parallel);
			void delete_material(core::unique<material> value);
			void remove_entity(core::unique<entity> entity);
			void delete_entity(core::unique<entity> entity);
			void set_camera(entity* camera);
			void ray_test(uint64_t section, const trigonometry::ray& origin, const ray_callback& callback);
			void script_hook(const std::string_view& name = "main");
			void set_active(bool enabled);
			void set_mrt(target_type type, bool clear);
			void set_rt(target_type type, bool clear);
			void swap_mrt(target_type type, graphics::multi_render_target_2d* init);
			void swap_rt(target_type type, graphics::render_target_2d* init);
			void clear_mrt(target_type type, bool color, bool depth);
			void clear_rt(target_type type, bool color, bool depth);
			void mutate(entity* parent, entity* child, const std::string_view& type);
			void mutate(entity* target, const std::string_view& type);
			void mutate(component* target, const std::string_view& type);
			void mutate(material* target, const std::string_view& type);
			void make_snapshot(idx_snapshot* result);
			void transaction(core::task_callback&& callback);
			void watch(task_type type, core::promise<void>&& awaitable);
			void watch(task_type type, core::vector<core::promise<void>>&& awaitables);
			void await(task_type type);
			void clear_culling();
			void reserve_materials(size_t size);
			void reserve_entities(size_t size);
			void reserve_components(uint64_t section, size_t size);
			void generate_depth_cascades(core::unique<depth_cascade_map>* result, uint32_t size) const;
			bool push_event(const std::string_view& event_name, core::variant_args&& args, bool propagate);
			bool push_event(const std::string_view& event_name, core::variant_args&& args, component* target);
			bool push_event(const std::string_view& event_name, core::variant_args&& args, entity* target);
			message_callback* set_listener(const std::string_view& event, message_callback&& callback);
			bool clear_listener(const std::string_view& event, message_callback* id);
			bool add_material(core::unique<material> base);
			void load_resource(uint64_t id, component* context, const std::string_view& path, const core::variant_args& keys, std::function<void(expects_content<void*>&&)>&& callback);
			core::string find_resource_id(uint64_t id, void* resource);
			material* get_invalid_material();
			material* add_material();
			material* clone_material(material* base);
			entity* get_entity(size_t entity);
			entity* get_last_entity();
			entity* get_camera_entity();
			component* get_component(uint64_t section, size_t component);
			component* get_camera();
			render_system* get_renderer();
			viewer get_camera_viewer() const;
			material* get_material(const std::string_view& material);
			material* get_material(size_t material);
			sparse_index& get_storage(uint64_t section);
			core::pool<component*>& get_components(uint64_t section);
			core::pool<component*>& get_actors(actor_type type);
			graphics::render_target_2d::desc get_desc_rt() const;
			graphics::multi_render_target_2d::desc get_desc_mrt() const;
			graphics::format get_format_mrt(unsigned int target) const;
			core::vector<entity*> clone_entity_as_array(entity* value);
			core::vector<entity*> query_by_parent(entity* parent) const;
			core::vector<entity*> query_by_name(const std::string_view& name) const;
			core::vector<component*> query_by_position(uint64_t section, const trigonometry::vector3& position, float radius);
			core::vector<component*> query_by_area(uint64_t section, const trigonometry::vector3& min, const trigonometry::vector3& max);
			core::vector<component*> query_by_match(uint64_t section, std::function<bool(const trigonometry::bounding&)>&& match_callback);
			core::vector<std::pair<component*, trigonometry::vector3>> query_by_ray(uint64_t section, const trigonometry::ray& origin);
			core::vector<depth_cube_map*>& get_points_mapping();
			core::vector<depth_map*>& get_spots_mapping();
			core::vector<depth_cascade_map*>& get_lines_mapping();
			const core::unordered_map<uint64_t, sparse_index*>& get_registry() const;
			core::string as_resource_path(const std::string_view& path);
			entity* add_entity();
			entity* clone_entity(entity* value);
			bool add_entity(core::unique<entity> entity);
			bool is_active() const;
			bool is_left_handed() const;
			bool is_indexed() const;
			bool is_busy(task_type type);
			size_t get_materials_count() const;
			size_t get_entities_count() const;
			size_t get_components_count(uint64_t section);
			bool has_entity(entity* entity) const;
			bool has_entity(size_t entity) const;
			graphics::multi_render_target_2d* get_mrt(target_type type) const;
			graphics::render_target_2d* get_rt(target_type type) const;
			graphics::texture_2d** get_merger();
			graphics::element_buffer* get_material_buffer() const;
			graphics::graphics_device* get_device() const;
			physics::simulator* get_simulator() const;
			graphics::activity* get_activity() const;
			render_constants* get_constants() const;
			shader_cache* get_shaders() const;
			primitive_cache* get_primitives() const;
			desc& get_conf();

		private:
			void step_simulate(core::timer* time);
			void step_synchronize(core::timer* time);
			void step_animate(core::timer* time);
			void step_gameplay(core::timer* time);
			void step_transactions();
			void step_events();
			void step_indexing();
			void step_finalize();

		protected:
			void load_component(component* base);
			void unload_component_all(component* base);
			bool unload_component(component* base);
			void register_component(component* base, bool verify);
			void unregister_component(component* base);
			void clone_entities(entity* instance, core::vector<entity*>* array);
			void generate_material_buffer();
			void generate_depth_buffers();
			void notify_cosmos(component* base);
			void clear_cosmos(component* base);
			void update_cosmos(sparse_index& storage, component* base);
			void fill_material_buffers();
			void resize_render_buffers();
			void register_entity(entity* in);
			bool unregister_entity(entity* in);
			bool resolve_event(event& data);
			void watch_movement(entity* base);
			void unwatch_movement(entity* base);
			entity* clone_entity_instance(entity* entity);

		public:
			template <typename t, typename match_function>
			core::vector<component*> query_by_match(match_function&& match_callback)
			{
				core::vector<component*> result;
				trigonometry::cosmos::iterator context;
				auto& storage = get_storage(t::get_type_id());
				auto enqueue = [&result](component* item) { result.push_back(item); };
				storage.index.template query_index<component, match_function, decltype(enqueue)>(context, std::move(match_callback), std::move(enqueue));

				return result;
			}
			template <typename t>
			core::vector<component*> query_by_position(const trigonometry::vector3& position, float radius)
			{
				return query_by_position(t::get_type_id(), position, radius);
			}
			template <typename t>
			core::vector<component*> query_by_area(const trigonometry::vector3& min, const trigonometry::vector3& max)
			{
				return query_by_area(t::get_type_id(), min, max);
			}
			template <typename t>
			core::vector<std::pair<component*, trigonometry::vector3>> query_by_ray(const trigonometry::ray& origin)
			{
				return query_by_ray(t::get_type_id(), origin);
			}
			template <typename t>
			void ray_test(const trigonometry::ray& origin, ray_callback&& callback)
			{
				ray_test(t::get_type_id(), origin, std::move(callback));
			}
			template <typename t>
			void load_resource(component* context, const std::string_view& path, std::function<void(expects_content<t*>&&)>&& callback)
			{
				load_resource<t>(context, path, core::variant_args(), std::move(callback));
			}
			template <typename t>
			void load_resource(component* context, const std::string_view& path, const core::variant_args& keys, std::function<void(expects_content<t*>&&)>&& callback)
			{
				VI_ASSERT(callback != nullptr, "callback should be set");
				load_resource((uint64_t)typeid(t).hash_code(), context, path, keys, [callback = std::move(callback)](expects_content<void*> object)
				{
					if (object)
						callback((t*)*object);
					else
						callback(object.error());
				});
			}
			template <typename t>
			core::string find_resource_id(t* resource)
			{
				return find_resource_id(typeid(t).hash_code(), (void*)resource);
			}
			template <typename t>
			sparse_index& get_storage()
			{
				return get_storage(t::get_type_id());
			}
			template <typename t>
			core::pool<component*>& get_components()
			{
				return get_components(t::get_type_id());
			}
		};

		class heavy_content_manager final : public content_manager
		{
		private:
			graphics::graphics_device* device;

		public:
			virtual ~heavy_content_manager() noexcept override = default;
			void set_device(graphics::graphics_device* new_device);
			graphics::graphics_device* get_device() const;
		};

		class heavy_application : public core::singleton<heavy_application>
		{
		public:
			struct desc : application::desc
			{
				graphics::graphics_device::desc graphics_device;
				graphics::activity::desc activity;
				size_t advanced_usage =
					(size_t)USE_GRAPHICS |
					(size_t)USE_ACTIVITY |
					(size_t)USE_AUDIO;
				bool blocking_dispatch = true;
				bool cursor = true;
			};

		public:
			struct cache_info
			{
				shader_cache* shaders = nullptr;
				primitive_cache* primitives = nullptr;
			} cache;

		private:
			core::timer* internal_clock = nullptr;
			gui::context* internal_ui = nullptr;
			application_state state = application_state::terminated;
			int exit_code = 0;

		public:
			audio::audio_device* audio = nullptr;
			graphics::graphics_device* renderer = nullptr;
			graphics::activity* activity = nullptr;
			scripting::virtual_machine* vm = nullptr;
			render_constants* constants = nullptr;
			heavy_content_manager* content = nullptr;
			app_data* database = nullptr;
			scene_graph* scene = nullptr;
			desc control;

		public:
			heavy_application(desc* i) noexcept;
			virtual ~heavy_application() noexcept;
			virtual void key_event(graphics::key_code key, graphics::key_mod mod, int computed, int repeat, bool pressed);
			virtual void input_event(char* buffer, size_t length);
			virtual void wheel_event(int x, int y, bool normal);
			virtual void window_event(graphics::window_state new_state, int x, int y);
			virtual void dispatch(core::timer* time);
			virtual void publish(core::timer* time);
			virtual void composition();
			virtual void script_hook();
			virtual void initialize();
			virtual core::promise<void> startup();
			virtual core::promise<void> shutdown();
			gui::context* try_get_ui() const;
			gui::context* fetch_ui();
			application_state get_state() const;
			int start();
			void restart();
			void stop(int exit_code = 0);

		private:
			void loop_trigger();

		private:
			static bool status(heavy_application* app);
			static void compose();

		public:
			template <typename t, typename ...a>
			static int start_app(a... args)
			{
				core::uptr<t> app = new t(args...);
				int exit_code = app->start();
				VI_ASSERT(exit_code != EXIT_RESTART, "application cannot be restarted");
				return exit_code;
			}
			template <typename t, typename ...a>
			static int start_app_with_restart(a... args)
			{
			restart_app:
				core::uptr<t> app = new t(args...);
				int exit_code = app->start();
				if (exit_code == EXIT_RESTART)
					goto restart_app;

				return exit_code;
			}
		};

		template <typename geometry, typename instance>
		struct batching_group
		{
			core::vector<instance> instances;
			graphics::element_buffer* data_buffer = nullptr;
			geometry* geometry_buffer = nullptr;
			material* material_data = nullptr;
		};

		template <typename geometry, typename instance>
		struct batch_dispatchable
		{
			size_t name;
			geometry* data;
			material* surface;
			instance params;

			batch_dispatchable(size_t new_name, geometry* new_data, material* new_surface, const instance& new_params) noexcept : name(new_name), data(new_data), surface(new_surface), params(new_params)
			{
			}
		};

		template <typename geometry, typename instance>
		class batching_proxy
		{
		public:
			typedef batching_group<geometry, instance> batch_group;
			typedef batch_dispatchable<geometry, instance> dispatchable;

		public:
			core::single_queue<batch_group*>* cache = nullptr;
			core::vector<core::vector<dispatchable>>* queue = nullptr;
			core::vector<dispatchable>* instances = nullptr;
			core::vector<batch_group*> groups;

		public:
			void clear()
			{
				for (auto* group : groups)
				{
					group->instances.clear();
					cache->push(group);
				}

				queue->clear();
				groups.clear();
			}
			void prepare(size_t max_size)
			{
				if (max_size > 0)
					queue->resize(max_size);

				for (auto* group : groups)
				{
					group->instances.clear();
					cache->push(group);
				}
				groups.clear();
			}
			void emplace(geometry* data, material* surface, const instance& params, size_t chunk)
			{
				VI_ASSERT(chunk < queue->size(), "chunk index is out of range");
				(*queue)[chunk].emplace_back(get_key_id(data, surface), data, surface, params);
			}
			size_t compile(graphics::graphics_device* device)
			{
				VI_ASSERT(device != nullptr, "device should be set");
				populate_instances();
				populate_groups();
				compile_groups(device);
				return groups.size();
			}

		private:
			void populate_instances()
			{
				size_t total = 0;
				for (auto& context : *queue)
					total += context.size();

				instances->reserve(total);
				for (auto& context : *queue)
				{
					std::move(context.begin(), context.end(), std::back_inserter(*instances));
					context.clear();
				}

				VI_SORT(instances->begin(), instances->end(), [](dispatchable& a, dispatchable& b)
				{
					return a.name < b.name;
				});
			}
			void populate_groups()
			{
				size_t name = 0;
				batch_group* next = nullptr;
				for (auto& item : *instances)
				{
					if (next != nullptr && name == item.name)
					{
						next->instances.emplace_back(std::move(item.params));
						continue;
					}

					name = item.name;
					next = fetch_group();
					next->geometry_buffer = item.data;
					next->material_data = item.surface;
					next->instances.emplace_back(std::move(item.params));
					groups.push_back(next);
				}
				instances->clear();
			}
			void compile_groups(graphics::graphics_device* device)
			{
				for (auto* group : groups)
				{
					if (group->data_buffer && group->instances.size() < (size_t)group->data_buffer->get_elements())
					{
						device->update_buffer(group->data_buffer, (void*)group->instances.data(), sizeof(instance) * group->instances.size());
						continue;
					}

					graphics::element_buffer::desc desc = graphics::element_buffer::desc();
					desc.access_flags = graphics::cpu_access::write;
					desc.usage = graphics::resource_usage::dynamic;
					desc.bind_flags = graphics::resource_bind::vertex_buffer;
					desc.element_count = (unsigned int)group->instances.size();
					desc.elements = (void*)group->instances.data();
					desc.element_width = sizeof(instance);

					core::memory::release(group->data_buffer);
					group->data_buffer = device->create_element_buffer(desc).or_else(nullptr);
					if (!group->data_buffer)
						group->instances.clear();
				}
			}
			batch_group* fetch_group()
			{
				if (cache->empty())
					return core::memory::init<batch_group>();

				batch_group* result = cache->front();
				cache->pop();
				return result;
			}
			size_t get_key_id(geometry* data, material* surface)
			{
				std::hash<void*> hash;
				size_t seed = hash((void*)data);
				seed ^= hash((void*)surface) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
				return seed;
			}
		};

		template <typename t, typename geometry = char, typename instance = char, size_t max = (size_t)MAX_STACK_DEPTH>
		class renderer_proxy
		{
			static_assert(std::is_base_of<component, t>::value, "parameter must be derived from a component");

		public:
			typedef batching_proxy<geometry, instance> batching;
			typedef batching_group<geometry, instance> batch_group;
			typedef batch_dispatchable<geometry, instance> dispatchable;
			typedef std::pair<t*, visibility_query> query_group;
			typedef core::vector<batch_group*> groups;
			typedef core::vector<t*> storage;

		public:
			static const size_t depth = max;

		private:
			struct
			{
				core::vector<core::vector<dispatchable>> queue;
				core::vector<dispatchable> instances;
				core::single_queue<batch_group*> groups;
			} caching;

		private:
			batching batchers[max][(size_t)geo_category::count];
			storage data[max][(size_t)geo_category::count];
			core::vector<core::vector<query_group>> queries;
			size_t offset;

		public:
			storage culling;

		public:
			renderer_proxy() noexcept : offset(0)
			{
				for (size_t i = 0; i < (size_t)geo_category::count; ++i)
				{
					for (size_t j = 0; j < depth; ++j)
					{
						auto& next = batchers[j][i];
						next.queue = &caching.queue;
						next.instances = &caching.instances;
						next.cache = &caching.groups;
					}
				}
			}
			~renderer_proxy() noexcept
			{
				for (size_t i = 0; i < (size_t)geo_category::count; ++i)
				{
					for (size_t j = 0; j < depth; ++j)
						batchers[j][i].clear();
				}

				while (!caching.groups.empty())
				{
					auto* next = caching.groups.front();
					core::memory::release(next->data_buffer);
					core::memory::deinit(next);
					caching.groups.pop();
				}
			}
			batching& batcher(geo_category category = geo_category::opaque)
			{
				return batchers[offset > 0 ? offset - 1 : 0][(size_t)category];
			}
			groups& batches(geo_category category = geo_category::opaque)
			{
				return batchers[offset > 0 ? offset - 1 : 0][(size_t)category].groups;
			}
			storage& top(geo_category category = geo_category::opaque)
			{
				return data[offset > 0 ? offset - 1 : 0][(size_t)category];
			}
			bool push(core::timer* time, render_system* base, geo_category category = geo_category::opaque)
			{
				VI_ASSERT(base != nullptr, "render system should be present");
				VI_ASSERT(offset < max - 1, "storage heap stack overflow");

				storage* frame = data[offset++];
				if (base->state.is_subpass())
				{
					subcull(base, frame);
					return true;
				}

				bool assume_sorted = (time->get_frame_index() % base->sorting_frequency != 0);
				cullout(base, frame, assume_sorted);
				return !assume_sorted;
			}
			void pop()
			{
				VI_ASSERT(offset > 0, "storage heap stack underflow");
				offset--;
			}
			bool has_batching()
			{
				return !std::is_same<geometry, char>::value && !std::is_same<instance, char>::value;
			}

		private:
			void prepare(size_t threads)
			{
				queries.resize(threads);
				for (auto& queue : queries)
					queue.clear();
			}

		private:
			template <typename push_function>
			void dispatch(push_function&& callback)
			{
				for (auto& queue : queries)
					std::for_each(queue.begin(), queue.end(), std::move(callback));
			}
			template <class q = t>
			typename std::enable_if<std::is_base_of<drawable, q>::value>::type cullout(render_system* system, storage* top, bool assume_sorted)
			{
				VI_MEASURE(core::timings::frame);
				if (assume_sorted)
				{
					auto* scene = system->get_scene();
					scene->statistics.instances += culling.size();
					for (size_t i = 0; i < (size_t)geo_category::count; ++i)
						scene->statistics.instances += top[i].size();
					return;
				}

				for (size_t i = 0; i < (size_t)geo_category::count; ++i)
					top[i].clear();
				culling.clear();

				system->parallel_query<t>([this](size_t threads)
				{
					prepare(threads);
				}, [this, &system](size_t chunk, component* item)
				{
					visibility_query info;
					system->fetch_visibility(item, info);
					if (info.boundary_visible || info.query_pixels)
						queries[chunk].emplace_back(std::make_pair((t*)item, std::move(info)));
				});

				auto* scene = system->get_scene();
				scene->await(task_type::rendering);

				dispatch([this, top](query_group& group)
				{
					if (group.second.boundary_visible)
						top[(size_t)group.second.category].push_back(group.first);

					if (group.second.query_pixels)
						culling.push_back(group.first);
				});

				++scene->statistics.sorting;
				if (!has_batching())
				{
					for (size_t i = 0; i < (size_t)geo_category::count; ++i)
					{
						auto& array = top[i];
						scene->statistics.instances += array.size();
						scene->watch(task_type::rendering, parallel::enqueue([&array]()
						{
							VI_SORT(array.begin(), array.end(), entity::sortout<t>);
						}));
					}

					scene->statistics.instances += culling.size();
					scene->watch(task_type::rendering, parallel::enqueue([this]()
					{
						VI_SORT(culling.begin(), culling.end(), entity::sortout<t>);
					}));
					scene->await(task_type::rendering);
				}
				else
				{
					scene->statistics.instances += culling.size();
					for (size_t i = 0; i < (size_t)geo_category::count; ++i)
						scene->statistics.instances += top[i].size();
				}
			}
			template <class q = t>
			typename std::enable_if<!std::is_base_of<drawable, q>::value>::type cullout(render_system* system, storage* top, bool assume_sorted)
			{
				VI_MEASURE(core::timings::frame);
				auto& subframe = top[(size_t)geo_category::opaque];
				if (assume_sorted)
				{
					auto* scene = system->get_scene();
					scene->statistics.instances += subframe.size();
					return;
				}

				subframe.clear();
				system->parallel_query<t>([this](size_t threads)
				{
					prepare(threads);
				}, [this, &system](size_t chunk, component* item)
				{
					visibility_query info;
					system->fetch_visibility(item, info);
					if (info.boundary_visible)
						queries[chunk].emplace_back(std::make_pair((t*)item, std::move(info)));
				});

				auto* scene = system->get_scene();
				scene->await(task_type::rendering);

				dispatch([&subframe](query_group& group)
				{
					subframe.push_back(group.first);
				});

				++scene->statistics.sorting;
				scene->statistics.instances += subframe.size();
				if (!has_batching())
					VI_SORT(subframe.begin(), subframe.end(), entity::sortout<t>);
			}
			template <class q = t>
			typename std::enable_if<std::is_base_of<drawable, q>::value>::type subcull(render_system* system, storage* top)
			{
				VI_MEASURE(core::timings::frame);
				for (size_t i = 0; i < (size_t)geo_category::count; ++i)
					top[i].clear();

				system->parallel_query<t>([this](size_t threads)
				{
					prepare(threads);
				}, [this, &system](size_t chunk, component* item)
				{
					visibility_query info;
					system->fetch_visibility(item, info);
					if (info.boundary_visible)
						queries[chunk].emplace_back(std::make_pair((t*)item, std::move(info)));
				});

				auto* scene = system->get_scene();
				scene->await(task_type::rendering);

				dispatch([top](query_group& group)
				{
					top[(size_t)group.second.category].push_back(group.first);
				});

				++scene->statistics.sorting;
				if (!has_batching())
				{
					for (size_t i = 0; i < (size_t)geo_category::count; ++i)
					{
						auto& array = top[i];
						scene->statistics.instances += array.size();
						scene->watch(task_type::rendering, parallel::enqueue([&array]()
						{
							VI_SORT(array.begin(), array.end(), entity::sortout<t>);
						}));
					}
					scene->await(task_type::rendering);
				}
				else
				{
					for (size_t i = 0; i < (size_t)geo_category::count; ++i)
						scene->statistics.instances += top[i].size();
				}
			}
			template <class q = t>
			typename std::enable_if<!std::is_base_of<drawable, q>::value>::type subcull(render_system* system, storage* top)
			{
				auto& subframe = top[(size_t)geo_category::opaque];
				subframe.clear();

				system->parallel_query<t>([this](size_t threads)
				{
					prepare(threads);
				}, [this, &system](size_t chunk, component* item)
				{
					visibility_query info;
					system->fetch_visibility(item, info);
					if (info.boundary_visible)
						queries[chunk].emplace_back(std::make_pair((t*)item, std::move(info)));
				});

				auto* scene = system->get_scene();
				scene->await(task_type::rendering);

				dispatch([&subframe](query_group& group)
				{
					subframe.push_back(group.first);
				});

				++scene->statistics.sorting;
				scene->statistics.instances += subframe.size();
				if (!has_batching())
					VI_SORT(subframe.begin(), subframe.end(), entity::sortout<t>);
			}
		};

		template <typename t, typename geometry = char, typename instance = char>
		class geometry_renderer : public renderer
		{
			static_assert(std::is_base_of<drawable, t>::value, "component must be drawable to work within geometry renderer");

		public:
			typedef batching_group<geometry, instance> batch_group;
			typedef batching_proxy<geometry, instance> batching;
			typedef core::vector<batch_group*> groups;
			typedef core::vector<t*> objects;

		private:
			renderer_proxy<t, geometry, instance> proxy;
			std::function<void(t*, instance&, batching&)> upsert;
			core::unordered_map<t*, graphics::query*> active;
			core::single_queue<graphics::query*> inactive;
			graphics::depth_stencil_state* depth_stencil;
			graphics::blend_state* blend;
			graphics::query* current;
			size_t frame_top[3];
			bool skippable[2];

		public:
			geometry_renderer(render_system* lab) noexcept : renderer(lab), current(nullptr)
			{
				graphics::graphics_device* device = system->get_device();
				depth_stencil = device->get_depth_stencil_state("dro_soo_lte");
				blend = device->get_blend_state("bo_woooo_one");
				frame_top[0] = 0;
				skippable[0] = false;
				frame_top[1] = 0;
				skippable[1] = false;
				frame_top[2] = 0;
			}
			virtual ~geometry_renderer() noexcept override
			{
				for (auto& item : active)
					core::memory::release(item.second);

				while (!inactive.empty())
				{
					core::memory::release(inactive.front());
					inactive.pop();
				}
			}
			virtual void batch_geometry(t* base, batching& batch, size_t chunk)
			{
			}
			virtual size_t cull_geometry(const viewer& view, const objects& chunk)
			{
				return 0;
			}
			virtual size_t render_depth(core::timer* time_step, const objects& chunk)
			{
				return 0;
			}
			virtual size_t render_depth_batched(core::timer* time_step, const groups& chunk)
			{
				return 0;
			}
			virtual size_t render_depth_cube(core::timer* time_step, const objects& chunk, trigonometry::matrix4x4* view_projection)
			{
				return 0;
			}
			virtual size_t render_depth_cube_batched(core::timer* time_step, const groups& chunk, trigonometry::matrix4x4* view_projection)
			{
				return 0;
			}
			virtual size_t render_geometry(core::timer* time_step, const objects& chunk)
			{
				return 0;
			}
			virtual size_t render_geometry_batched(core::timer* time_step, const groups& chunk)
			{
				return 0;
			}
			virtual size_t render_geometry_prepass(core::timer* time_step, const objects& chunk)
			{
				return 0;
			}
			virtual size_t render_geometry_prepass_batched(core::timer* time_step, const groups& chunk)
			{
				return 0;
			}
			void clear_culling() override
			{
				for (auto& item : active)
					inactive.push(item.second);
				active.clear();
			}
			void begin_pass(core::timer* time) override
			{
				bool proceed = proxy.push(time, system);
				if (!system->allow_input_lag)
					proceed = true;

				if (!proceed || !proxy.has_batching())
					return;

				graphics::graphics_device* device = system->get_device();
				for (size_t i = 0; i < (size_t)geo_category::count; ++i)
				{
					auto& batcher = proxy.batcher((geo_category)i);
					auto& frame = proxy.top((geo_category)i);
					parallel::wail_all(parallel::distribute(frame.begin(), frame.end(), THRESHOLD_PER_THREAD, [&batcher](size_t threads)
					{
						batcher.prepare(threads);
					}, [this, &batcher](size_t thread, t* next)
					{
						batch_geometry(next, batcher, thread);
					}));

					auto* scene = system->get_scene();
					scene->statistics.batching += batcher.compile(device);
				}
			}
			void end_pass() override
			{
				proxy.pop();
			}
			bool has_category(geo_category category) override
			{
				return !proxy.top(category).empty();
			}
			size_t render_prepass(core::timer* time) override
			{
				size_t count = 0;
				if (!system->state.is(render_state::geometry))
					return count;

				geo_category category = geo_category::opaque;
				if (system->state.is_set(render_opt::transparent))
					category = geo_category::transparent;
				else if (system->state.is_set(render_opt::additive))
					category = geo_category::additive;

				VI_MEASURE(core::timings::frame);
				if (proxy.has_batching())
				{
					auto& frame = proxy.batches(category);
					if (!frame.empty())
						count += render_geometry_prepass_batched(time, frame);
				}
				else
				{
					auto& frame = proxy.top(category);
					if (!frame.empty())
						count += render_geometry_prepass(time, frame);
				}

				if (system->state.is_top())
					count += culling_pass();

				return count;
			}
			size_t render_pass(core::timer* time) override
			{
				size_t count = 0;
				if (system->state.is(render_state::geometry))
				{
					geo_category category = geo_category::opaque;
					if (system->state.is_set(render_opt::transparent))
						category = geo_category::transparent;
					else if (system->state.is_set(render_opt::additive))
						category = geo_category::additive;

					VI_MEASURE(core::timings::frame);
					if (proxy.has_batching())
					{
						auto& frame = proxy.batches(category);
						if (!frame.empty())
						{
							system->clear_materials();
							count += render_geometry_batched(time, frame);
						}
					}
					else
					{
						auto& frame = proxy.top(category);
						if (!frame.empty())
						{
							system->clear_materials();
							count += render_geometry(time, frame);
						}
					}
				}
				else if (system->state.is(render_state::depth))
				{
					if (!system->state.is_subpass())
						return 0;

					VI_MEASURE(core::timings::pass);
					if (proxy.has_batching())
					{
						auto& frame1 = proxy.batches(geo_category::opaque);
						auto& frame2 = proxy.batches(geo_category::transparent);
						if (!frame1.empty() || !frame2.empty())
							system->clear_materials();

						if (!frame1.empty())
							count += render_depth_batched(time, frame1);

						if (!frame2.empty())
							count += render_depth_batched(time, frame2);
					}
					else
					{
						auto& frame1 = proxy.top(geo_category::opaque);
						auto& frame2 = proxy.top(geo_category::transparent);
						if (!frame1.empty() || !frame2.empty())
							system->clear_materials();

						if (!frame1.empty())
							count += render_depth(time, frame1);

						if (!frame2.empty())
							count += render_depth(time, frame2);
					}
				}
				else if (system->state.is(render_state::depth_cube))
				{
					if (!system->state.is_subpass())
						return 0;

					VI_MEASURE(core::timings::pass);
					if (proxy.has_batching())
					{
						auto& frame1 = proxy.batches(geo_category::opaque);
						auto& frame2 = proxy.batches(geo_category::transparent);
						if (!frame1.empty() || !frame2.empty())
							system->clear_materials();

						if (!frame1.empty())
							count += render_depth_cube_batched(time, frame1, system->view.cube_view_projection);

						if (!frame2.empty())
							count += render_depth_cube_batched(time, frame2, system->view.cube_view_projection);
					}
					else
					{
						auto& frame1 = proxy.top(geo_category::opaque);
						auto& frame2 = proxy.top(geo_category::transparent);
						if (!frame1.empty() || !frame2.empty())
							system->clear_materials();

						if (!frame1.empty())
							count += render_depth_cube(time, frame1, system->view.cube_view_projection);

						if (!frame2.empty())
							count += render_depth_cube(time, frame2, system->view.cube_view_projection);
					}
				}

				return count;
			}
			size_t culling_pass()
			{
				if (!system->occlusion_culling)
					return 0;

				VI_MEASURE(core::timings::pass);
				graphics::graphics_device* device = system->get_device();
				size_t count = 0; size_t fragments = 0;

				for (auto it = active.begin(); it != active.end();)
				{
					auto* query = it->second;
					if (device->get_query_data(query, &fragments))
					{
						it->first->overlapping = (fragments > 0 ? 1.0f : 0.0f);
						it = active.erase(it);
						inactive.push(query);
					}
					else
						++it;
				}

				skippable[0] = (frame_top[0]++ < system->occluder_skips);
				if (!skippable[0])
					frame_top[0] = 0;

				skippable[1] = (frame_top[1]++ < system->occludee_skips);
				if (!skippable[1])
					frame_top[1] = 0;

				if (frame_top[2]++ >= system->occlusion_skips && !proxy.culling.empty())
				{
					auto view_projection = system->view.view_projection;
					system->view.view_projection = trigonometry::matrix4x4::create_scale(system->occludee_scaling) * view_projection;
					device->set_depth_stencil_state(depth_stencil);
					device->set_blend_state(blend);
					count += cull_geometry(system->view, proxy.culling);
					system->view.view_projection = view_projection;
				}

				return count;
			}
			bool culling_begin(t* base)
			{
				VI_ASSERT(base != nullptr, "base should be set");
				if (skippable[1] && base->overlapping < system->threshold)
					return false;
				else if (skippable[0] && base->overlapping >= system->threshold)
					return false;

				if (inactive.empty() && active.size() >= system->max_queries)
				{
					base->overlapping = system->overflow_visibility;
					return false;
				}

				if (active.find(base) != active.end())
					return false;

				graphics::graphics_device* device = system->get_device();
				if (inactive.empty())
				{
					graphics::query::desc i;
					i.predicate = false;

					current = device->create_query(i).or_else(nullptr);
					if (!current)
						return false;
				}
				else
				{
					current = inactive.front();
					inactive.pop();
				}

				active[base] = current;
				device->query_begin(current);
				return true;
			}
			bool culling_end()
			{
				VI_ASSERT(current != nullptr, "culling query must be started");
				if (!current)
					return false;

				system->get_device()->query_end(current);
				current = nullptr;

				return true;
			}

		public:
			VI_COMPONENT("geometry_renderer");
		};

		class effect_renderer : public renderer
		{
		protected:
			struct shader_data
			{
				struct
				{
					uint32_t diffuse_buffer = (uint32_t)-1;
					uint32_t normal_buffer = (uint32_t)-1;
					uint32_t depth_buffer = (uint32_t)-1;
					uint32_t surface_buffer = (uint32_t)-1;
					uint32_t image_buffer = (uint32_t)-1;
					uint32_t render_buffer = (uint32_t)-1;
					uint32_t sampler = (uint32_t)-1;
				} slots;
				core::unordered_map<core::string, uint32_t> offsets;
				core::unordered_set<uint32_t> regs;
				graphics::shader* effect = nullptr;
				core::string filename;
			};

			struct
			{
				uint32_t diffuse_map = (uint32_t)-1;
				uint32_t sampler = (uint32_t)-1;
				uint32_t object = (uint32_t)-1;
			} slots;

		protected:
			core::unordered_map<graphics::shader*, shader_data> effects;
			graphics::depth_stencil_state* depth_stencil;
			graphics::rasterizer_state* rasterizer;
			graphics::blend_state* blend;
			graphics::sampler_state* sampler_wrap;
			graphics::sampler_state* sampler_clamp;
			graphics::sampler_state* sampler_mirror;
			graphics::input_layout* layout;
			graphics::render_target_2d* output;
			graphics::render_target_2d* swap;

		public:
			effect_renderer(render_system* lab) noexcept;
			virtual ~effect_renderer() noexcept override;
			virtual void resize_effect();
			virtual void render_effect(core::timer* time) = 0;
			size_t render_pass(core::timer* time) override;
			void resize_buffers() override;
			unsigned int get_mip_levels() const;
			unsigned int get_width() const;
			unsigned int get_height() const;

		private:
			uint32_t texture_count_of(shader_data& data);

		protected:
			void render_copy_from_main(uint32_t slot, graphics::texture_2d* dest);
			void render_copy_to_main(uint32_t slot, graphics::texture_2d* src);
			void render_copy_from_last(graphics::texture_2d* dest);
			void render_copy_to_last(graphics::texture_2d* src);
			void render_output(graphics::render_target_2d* resource = nullptr);
			void render_texture(graphics::shader* effect, const std::string_view& name, graphics::texture_2d* resource = nullptr);
			void render_texture(graphics::shader* effect, const std::string_view& name, graphics::texture_3d* resource = nullptr);
			void render_texture(graphics::shader* effect, const std::string_view& name, graphics::texture_cube* resource = nullptr);
			void render_merge(graphics::shader* effect, graphics::sampler_state* sampler, void* buffer = nullptr, size_t count = 1);
			void render_result(graphics::shader* effect, graphics::sampler_state* sampler, void* buffer = nullptr);
			void render_result(graphics::sampler_state* sampler);
			void generate_mips();
			shader_data* get_effect_by_filename(const std::string_view& name);
			shader_data* get_effect_by_shader(graphics::shader* shader);
			graphics::expects_graphics<graphics::shader*> compile_effect(graphics::shader::desc& desc, size_t buffer_size = 0);
			graphics::expects_graphics<graphics::shader*> compile_effect(const std::string_view& section_name, core::vector<core::string>&& features, size_t buffer_size = 0);

		public:
			VI_COMPONENT("effect_renderer");
		};
	}
}
#endif