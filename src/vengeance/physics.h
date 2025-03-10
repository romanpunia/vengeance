#ifndef VI_PHYSICS_H
#define VI_PHYSICS_H
#include "trigonometry.h"

class btCollisionConfiguration;
class btBroadphaseInterface;
class btConstraintSolver;
class btDiscreteDynamicsWorld;
class btCollisionDispatcher;
class btSoftBodySolver;
class btRigidBody;
class btSoftBody;
class btTypedConstraint;
class btPoint2PointConstraint;
class btHingeConstraint;
class btSliderConstraint;
class btConeTwistConstraint;
class btGeneric6DofSpring2Constraint;
class btTransform;
class btCollisionObject;
class btBoxShape;
class btSphereShape;
class btCapsuleShape;
class btConeShape;
class btCylinderShape;
class btCollisionShape;
class btVector3;
typedef bool(*contact_destroyed_callback)(void*);
typedef bool(*contact_processed_callback)(class btManifoldPoint&, void*, void*);
typedef void(*contact_started_callback)(class btPersistentManifold* const&);
typedef void(*contact_ended_callback)(class btPersistentManifold* const&);

namespace vitex
{
	namespace physics
	{
		class rigid_body;

		class soft_body;

		class simulator;

		enum class shape
		{
			box,
			triangle,
			tetrahedral,
			convex_triangle_mesh,
			convex_hull,
			convex_point_cloud,
			convex_polyhedral,
			implicit_convex_start,
			sphere,
			multi_sphere,
			capsule,
			cone,
			convex,
			cylinder,
			uniform_scaling,
			minkowski_sum,
			minkowski_difference,
			box_2d,
			convex_2d,
			custom_convex,
			concaves_start,
			triangle_mesh,
			triangle_mesh_scaled,
			fast_concave_mesh,
			terrain,
			gimpact,
			triangle_mesh_multimaterial,
			empty,
			static_plane,
			custom_concave,
			concaves_end,
			compound,
			softbody,
			hf_fluid,
			hf_fluid_bouyant_convex,
			invalid,
			count
		};

		enum class motion_state
		{
			active = 1,
			island_sleeping = 2,
			deactivation_needed = 3,
			disable_deactivation = 4,
			disable_simulation = 5,
		};

		enum class soft_feature
		{
			none,
			node,
			link,
			face,
			tetra
		};

		enum class soft_aero_model
		{
			vpoint,
			vtwo_sided,
			vtwo_sided_lift_drag,
			vone_sided,
			ftwo_sided,
			ftwo_sided_lift_drag,
			fone_sided
		};

		enum class soft_collision
		{
			rvs_mask = 0x000f,
			sdf_rs = 0x0001,
			cl_rs = 0x0002,
			sdf_rd = 0x0003,
			sdf_rdf = 0x0004,
			svs_mask = 0x00F0,
			vf_ss = 0x0010,
			cl_ss = 0x0020,
			cl_self = 0x0040,
			vf_dd = 0x0050,
			defaults = sdf_rs | cl_ss
		};

		inline soft_collision operator |(soft_collision a, soft_collision b)
		{
			return static_cast<soft_collision>(static_cast<uint64_t>(a) | static_cast<uint64_t>(b));
		}

		typedef std::function<void(const struct collision_body&)> collision_callback;

		struct shape_contact
		{
			trigonometry::vector3 local_point1;
			trigonometry::vector3 local_point2;
			trigonometry::vector3 position_world1;
			trigonometry::vector3 position_world2;
			trigonometry::vector3 normal_world;
			trigonometry::vector3 lateral_friction_direction1;
			trigonometry::vector3 lateral_friction_direction2;
			float distance = 0.0f;
			float combined_friction = 0.0f;
			float combined_rolling_friction = 0.0f;
			float combined_spinning_friction = 0.0f;
			float combined_restitution = 0.0f;
			float applied_impulse = 0.0f;
			float applied_impulse_lateral1 = 0.0f;
			float applied_impulse_lateral2 = 0.0f;
			float contact_motion1 = 0.0f;
			float contact_motion2 = 0.0f;
			float contact_cfm = 0.0f;
			float combined_contact_stiffness = 0.0f;
			float contact_erp = 0.0f;
			float combined_contact_damping = 0.0f;
			float friction_cfm = 0.0f;
			int part_id1 = 0;
			int part_id2 = 0;
			int index1 = 0;
			int index2 = 0;
			int contact_point_flags = 0;
			int life_time = 0;
		};

		struct ray_contact
		{
			trigonometry::vector3 hit_normal_local;
			trigonometry::vector3 hit_normal_world;
			trigonometry::vector3 hit_point_world;
			trigonometry::vector3 ray_from_world;
			trigonometry::vector3 ray_to_world;
			float hit_fraction = 0.0f;
			float closest_hit_fraction = 0.0f;
			bool normal_in_world_space = false;
		};

		struct collision_body
		{
			rigid_body* rigid = nullptr;
			soft_body* soft = nullptr;

			collision_body(btCollisionObject* object) noexcept;
		};

		class hull_shape final : public core::reference<hull_shape>
		{
		private:
			core::vector<trigonometry::vertex> vertices;
			core::vector<int> indices;
			btCollisionShape* shape;

		public:
			hull_shape(core::vector<trigonometry::vertex>&& new_vertices, core::vector<int>&& indices) noexcept;
			hull_shape(core::vector<trigonometry::vertex>&& new_vertices) noexcept;
			hull_shape(btCollisionShape* from) noexcept;
			~hull_shape() noexcept;
			const core::vector<trigonometry::vertex>& get_vertices() const;
			const core::vector<int>& get_indices() const;
			btCollisionShape* get_shape() const;
		};

		class rigid_body final : public core::reference<rigid_body>
		{
			friend simulator;

		public:
			struct desc
			{
				btCollisionShape* shape = nullptr;
				float anticipation = 0, mass = 0;
				trigonometry::vector3 position;
				trigonometry::vector3 rotation;
				trigonometry::vector3 scale;
			};

		private:
			btRigidBody* instance;
			simulator* engine;
			desc initial;

		public:
			collision_callback on_collision_enter;
			collision_callback on_collision_exit;
			void* user_pointer;

		private:
			rigid_body(simulator* refer, const desc& i) noexcept;

		public:
			~rigid_body() noexcept;
			core::unique<rigid_body> copy();
			void push(const trigonometry::vector3& velocity);
			void push(const trigonometry::vector3& velocity, const trigonometry::vector3& torque);
			void push(const trigonometry::vector3& velocity, const trigonometry::vector3& torque, const trigonometry::vector3& center);
			void push_kinematic(const trigonometry::vector3& velocity);
			void push_kinematic(const trigonometry::vector3& velocity, const trigonometry::vector3& torque);
			void synchronize(trigonometry::transform* transform, bool kinematic);
			void set_collision_flags(size_t flags);
			void set_activity(bool active);
			void set_as_ghost();
			void set_as_normal();
			void set_self_pointer();
			void set_world_transform(btTransform* value);
			void set_collision_shape(btCollisionShape* shape, trigonometry::transform* transform);
			void set_mass(float mass);
			void set_activation_state(motion_state value);
			void set_angular_damping(float value);
			void set_angular_sleeping_threshold(float value);
			void set_spinning_friction(float value);
			void set_contact_stiffness(float value);
			void set_contact_damping(float value);
			void set_friction(float value);
			void set_restitution(float value);
			void set_hit_fraction(float value);
			void set_linear_damping(float value);
			void set_linear_sleeping_threshold(float value);
			void set_ccd_motion_threshold(float value);
			void set_ccd_swept_sphere_radius(float value);
			void set_contact_processing_threshold(float value);
			void set_deactivation_time(float value);
			void set_rolling_friction(float value);
			void set_angular_factor(const trigonometry::vector3& value);
			void set_anisotropic_friction(const trigonometry::vector3& value);
			void set_gravity(const trigonometry::vector3& value);
			void set_linear_factor(const trigonometry::vector3& value);
			void set_linear_velocity(const trigonometry::vector3& value);
			void set_angular_velocity(const trigonometry::vector3& value);
			motion_state get_activation_state() const;
			shape get_collision_shape_type() const;
			trigonometry::vector3 get_angular_factor() const;
			trigonometry::vector3 get_anisotropic_friction() const;
			trigonometry::vector3 get_gravity() const;
			trigonometry::vector3 get_linear_factor() const;
			trigonometry::vector3 get_linear_velocity() const;
			trigonometry::vector3 get_angular_velocity() const;
			trigonometry::vector3 get_scale() const;
			trigonometry::vector3 get_position() const;
			trigonometry::vector3 get_rotation() const;
			btTransform* get_world_transform() const;
			btCollisionShape* get_collision_shape() const;
			btRigidBody* get() const;
			bool is_active() const;
			bool is_static() const;
			bool is_ghost() const;
			bool is_colliding() const;
			float get_spinning_friction() const;
			float get_contact_stiffness() const;
			float get_contact_damping() const;
			float get_angular_damping() const;
			float get_angular_sleeping_threshold() const;
			float get_friction() const;
			float get_restitution() const;
			float get_hit_fraction() const;
			float get_linear_damping() const;
			float get_linear_sleeping_threshold() const;
			float get_ccd_motion_threshold() const;
			float get_ccd_swept_sphere_radius() const;
			float get_contact_processing_threshold() const;
			float get_deactivation_time() const;
			float get_rolling_friction() const;
			float get_mass() const;
			size_t get_collision_flags() const;
			desc& get_initial_state();
			simulator* get_simulator() const;

		public:
			static rigid_body* get(btRigidBody* from);
		};

		class soft_body final : public core::reference<soft_body>
		{
			friend simulator;

		public:
			struct desc
			{
				struct cv
				{
					struct sconvex
					{
						hull_shape* hull = nullptr;
						bool enabled = false;
					} convex;

					struct srope
					{
						bool start_fixed = false;
						bool end_fixed = false;
						bool enabled = false;
						int count = 0;
						trigonometry::vector3 start = 0;
						trigonometry::vector3 end = 1;
					} rope;

					struct spatch
					{
						bool generate_diagonals = false;
						bool corner00_fixed = false;
						bool corner10_fixed = false;
						bool corner01_fixed = false;
						bool corner11_fixed = false;
						bool enabled = false;
						int count_x = 2;
						int count_y = 2;
						trigonometry::vector3 corner00 = trigonometry::vector3(0, 0);
						trigonometry::vector3 corner10 = trigonometry::vector3(1, 0);
						trigonometry::vector3 corner01 = trigonometry::vector3(0, 1);
						trigonometry::vector3 corner11 = trigonometry::vector3(1, 1);
					} patch;

					struct sellipsoid
					{
						trigonometry::vector3 center;
						trigonometry::vector3 radius = 1;
						int count = 3;
						bool enabled = false;
					} ellipsoid;
				} shape;

				struct sconfig
				{
					soft_aero_model aero_model = soft_aero_model::vpoint;
					float vcf = 1;
					float dp = 0;
					float dg = 0;
					float lf = 0;
					float pr = 1.0f;
					float vc = 0.1f;
					float df = 0.5f;
					float mt = 0.1f;
					float chr = 1;
					float khr = 0.1f;
					float shr = 1;
					float ahr = 0.7f;
					float srhr_cl = 0.1f;
					float skhr_cl = 1;
					float sshr_cl = 0.5f;
					float sr_splt_cl = 0.5f;
					float sk_splt_cl = 0.5f;
					float ss_splt_cl = 0.5f;
					float max_volume = 1;
					float time_scale = 1;
					float drag = 0;
					float max_stress = 0;
					int clusters = 0;
					int constraints = 2;
					int viterations = 10;
					int piterations = 2;
					int diterations = 0;
					int citerations = 4;
					int collisions = (int)(soft_collision::defaults | soft_collision::vf_ss);
				} config;

				float anticipation = 0;
				trigonometry::vector3 position;
				trigonometry::vector3 rotation;
				trigonometry::vector3 scale;
			};

			struct ray_cast
			{
				soft_body* body = nullptr;
				soft_feature feature = soft_feature::none;
				float fraction = 0;
				int index = 0;
			};

		private:
			btSoftBody* instance;
			simulator* engine;
			trigonometry::vector3 center;
			desc initial;

		public:
			collision_callback on_collision_enter;
			collision_callback on_collision_exit;
			void* user_pointer;

		private:
			soft_body(simulator* refer, const desc& i) noexcept;

		public:
			~soft_body() noexcept;
			core::unique<soft_body> copy();
			void activate(bool force);
			void synchronize(trigonometry::transform* transform, bool kinematic);
			void get_indices(core::vector<int>* indices) const;
			void get_vertices(core::vector<trigonometry::vertex>* vertices) const;
			void get_bounding_box(trigonometry::vector3* min, trigonometry::vector3* max) const;
			void set_contact_stiffness_and_damping(float stiffness, float damping);
			void add_anchor(int node, rigid_body* body, bool disable_collision_between_linked_bodies = false, float influence = 1);
			void add_anchor(int node, rigid_body* body, const trigonometry::vector3& local_pivot, bool disable_collision_between_linked_bodies = false, float influence = 1);
			void add_force(const trigonometry::vector3& force);
			void add_force(const trigonometry::vector3& force, int node);
			void add_aero_force_to_node(const trigonometry::vector3& wind_velocity, int node_index);
			void add_aero_force_to_face(const trigonometry::vector3& wind_velocity, int face_index);
			void add_velocity(const trigonometry::vector3& velocity);
			void set_velocity(const trigonometry::vector3& velocity);
			void add_velocity(const trigonometry::vector3& velocity, int node);
			void set_mass(int node, float mass);
			void set_total_mass(float mass, bool from_faces = false);
			void set_total_density(float density);
			void set_volume_mass(float mass);
			void set_volume_density(float density);
			void translate(const trigonometry::vector3& position);
			void rotate(const trigonometry::vector3& rotation);
			void scale(const trigonometry::vector3& scale);
			void set_rest_length_scale(float rest_length);
			void set_pose(bool volume, bool frame);
			float get_mass(int node) const;
			float get_total_mass() const;
			float get_rest_length_scale() const;
			float get_volume() const;
			int generate_bending_constraints(int distance);
			void randomize_constraints();
			bool cut_link(int node0, int node1, float position);
			bool ray_test(const trigonometry::vector3& from, const trigonometry::vector3& to, ray_cast& result);
			void set_wind_velocity(const trigonometry::vector3& velocity);
			trigonometry::vector3 get_wind_velocity() const;
			void get_aabb(trigonometry::vector3& min, trigonometry::vector3& max) const;
			void indices_to_pointers(const int* map = 0);
			void set_spinning_friction(float value);
			trigonometry::vector3 get_linear_velocity() const;
			trigonometry::vector3 get_angular_velocity() const;
			trigonometry::vector3 get_center_position() const;
			void set_activity(bool active);
			void set_as_ghost();
			void set_as_normal();
			void set_self_pointer();
			void set_world_transform(btTransform* value);
			void set_activation_state(motion_state value);
			void set_contact_stiffness(float value);
			void set_contact_damping(float value);
			void set_friction(float value);
			void set_restitution(float value);
			void set_hit_fraction(float value);
			void set_ccd_motion_threshold(float value);
			void set_ccd_swept_sphere_radius(float value);
			void set_contact_processing_threshold(float value);
			void set_deactivation_time(float value);
			void set_rolling_friction(float value);
			void set_anisotropic_friction(const trigonometry::vector3& value);
			void set_config(const desc::sconfig& conf);
			shape get_collision_shape_type() const;
			motion_state get_activation_state() const;
			trigonometry::vector3 get_anisotropic_friction() const;
			trigonometry::vector3 get_scale() const;
			trigonometry::vector3 get_position() const;
			trigonometry::vector3 get_rotation() const;
			btTransform* get_world_transform() const;
			btSoftBody* get() const;
			bool is_active() const;
			bool is_static() const;
			bool is_ghost() const;
			bool is_colliding() const;
			float get_spinning_friction() const;
			float get_contact_stiffness() const;
			float get_contact_damping() const;
			float get_friction() const;
			float get_restitution() const;
			float get_hit_fraction() const;
			float get_ccd_motion_threshold() const;
			float get_ccd_swept_sphere_radius() const;
			float get_contact_processing_threshold() const;
			float get_deactivation_time() const;
			float get_rolling_friction() const;
			size_t get_collision_flags() const;
			size_t get_vertices_count() const;
			desc& get_initial_state();
			simulator* get_simulator() const;

		public:
			static soft_body* get(btSoftBody* from);
		};

		class constraint : public core::reference<constraint>
		{
		protected:
			btRigidBody* first, * second;
			simulator* engine;

		public:
			void* user_pointer;

		protected:
			constraint(simulator* refer) noexcept;

		public:
			virtual ~constraint() noexcept = default;
			virtual core::unique<constraint> copy() const = 0;
			virtual btTypedConstraint* get() const = 0;
			virtual bool has_collisions() const = 0;
			void set_breaking_impulse_threshold(float value);
			void set_enabled(bool value);
			bool is_enabled() const;
			bool is_active() const;
			float get_breaking_impulse_threshold() const;
			btRigidBody* get_first() const;
			btRigidBody* get_second() const;
			simulator* get_simulator() const;
		};

		class pconstraint : public constraint
		{
			friend rigid_body;
			friend simulator;

		public:
			struct desc
			{
				rigid_body* target_a = nullptr;
				rigid_body* target_b = nullptr;
				trigonometry::vector3 pivot_a;
				trigonometry::vector3 pivot_b;
				bool collisions = true;
			};

		private:
			btPoint2PointConstraint* instance;
			desc state;

		private:
			pconstraint(simulator* refer, const desc& i) noexcept;

		public:
			~pconstraint() noexcept override;
			core::unique<constraint> copy() const override;
			btTypedConstraint* get() const override;
			bool has_collisions() const override;
			void set_pivot_a(const trigonometry::vector3& value);
			void set_pivot_b(const trigonometry::vector3& value);
			trigonometry::vector3 get_pivot_a() const;
			trigonometry::vector3 get_pivot_b() const;
			desc& get_state();
		};

		class hconstraint : public constraint
		{
			friend rigid_body;
			friend simulator;

		public:
			struct desc
			{
				rigid_body* target_a = nullptr;
				rigid_body* target_b = nullptr;
				bool references = false;
				bool collisions = true;
			};

		private:
			btHingeConstraint* instance;
			desc state;

		private:
			hconstraint(simulator* refer, const desc& i) noexcept;

		public:
			~hconstraint() noexcept override;
			core::unique<constraint> copy() const override;
			btTypedConstraint* get() const override;
			bool has_collisions() const override;
			void enable_angular_motor(bool enable, float target_velocity, float max_motor_impulse);
			void enable_motor(bool enable);
			void test_limit(const trigonometry::matrix4x4& a, const trigonometry::matrix4x4& b);
			void set_frames(const trigonometry::matrix4x4& a, const trigonometry::matrix4x4& b);
			void set_angular_only(bool value);
			void set_max_motor_impulse(float value);
			void set_motor_target_velocity(float value);
			void set_motor_target(float target_angle, float delta);
			void set_limit(float low, float high, float softness = 0.9f, float bias_factor = 0.3f, float relaxation_factor = 1.0f);
			void set_offset(bool value);
			void set_reference_to_a(bool value);
			void set_axis(const trigonometry::vector3& value);
			int get_solve_limit() const;
			float get_motor_target_velocity() const;
			float get_max_motor_impulse() const;
			float get_limit_sign() const;
			float get_hinge_angle() const;
			float get_hinge_angle(const trigonometry::matrix4x4& a, const trigonometry::matrix4x4& b) const;
			float get_lower_limit() const;
			float get_upper_limit() const;
			float get_limit_softness() const;
			float get_limit_bias_factor() const;
			float get_limit_relaxation_factor() const;
			bool has_limit() const;
			bool is_offset() const;
			bool is_reference_to_a() const;
			bool is_angular_only() const;
			bool is_angular_motor_enabled() const;
			desc& get_state();
		};

		class sconstraint : public constraint
		{
			friend rigid_body;
			friend simulator;

		public:
			struct desc
			{
				rigid_body* target_a = nullptr;
				rigid_body* target_b = nullptr;
				bool collisions = true;
				bool linear = true;
			};

		private:
			btSliderConstraint* instance;
			desc state;

		private:
			sconstraint(simulator* refer, const desc& i) noexcept;

		public:
			~sconstraint() noexcept override;
			core::unique<constraint> copy() const override;
			btTypedConstraint* get() const override;
			bool has_collisions() const override;
			void set_angular_motor_velocity(float value);
			void set_linear_motor_velocity(float value);
			void set_upper_linear_limit(float value);
			void set_lower_linear_limit(float value);
			void set_angular_damping_direction(float value);
			void set_linear_damping_direction(float value);
			void set_angular_damping_limit(float value);
			void set_linear_damping_limit(float value);
			void set_angular_damping_ortho(float value);
			void set_linear_damping_ortho(float value);
			void set_upper_angular_limit(float value);
			void set_lower_angular_limit(float value);
			void set_max_angular_motor_force(float value);
			void set_max_linear_motor_force(float value);
			void set_angular_restitution_direction(float value);
			void set_linear_restitution_direction(float value);
			void set_angular_restitution_limit(float value);
			void set_linear_restitution_limit(float value);
			void set_angular_restitution_ortho(float value);
			void set_linear_restitution_ortho(float value);
			void set_angular_softness_direction(float value);
			void set_linear_softness_direction(float value);
			void set_angular_softness_limit(float value);
			void set_linear_softness_limit(float value);
			void set_angular_softness_ortho(float value);
			void set_linear_softness_ortho(float value);
			void set_powered_angular_motor(bool value);
			void set_powered_linear_motor(bool value);
			float get_angular_motor_velocity() const;
			float get_linear_motor_velocity() const;
			float get_upper_linear_limit() const;
			float get_lower_linear_limit() const;
			float get_angular_damping_direction() const;
			float get_linear_damping_direction() const;
			float get_angular_damping_limit() const;
			float get_linear_damping_limit() const;
			float get_angular_damping_ortho() const;
			float get_linear_damping_ortho() const;
			float get_upper_angular_limit() const;
			float get_lower_angular_limit() const;
			float get_max_angular_motor_force() const;
			float get_max_linear_motor_force() const;
			float get_angular_restitution_direction() const;
			float get_linear_restitution_direction() const;
			float get_angular_restitution_limit() const;
			float get_linear_restitution_limit() const;
			float get_angular_restitution_ortho() const;
			float get_linear_restitution_ortho() const;
			float get_angular_softness_direction() const;
			float get_linear_softness_direction() const;
			float get_angular_softness_limit() const;
			float get_linear_softness_limit() const;
			float get_angular_softness_ortho() const;
			float get_linear_softness_ortho() const;
			bool get_powered_angular_motor() const;
			bool get_powered_linear_motor() const;
			desc& get_state();
		};

		class ct_constraint : public constraint
		{
			friend rigid_body;
			friend simulator;

		public:
			struct desc
			{
				rigid_body* target_a = nullptr;
				rigid_body* target_b = nullptr;
				bool collisions = true;
			};

		private:
			btConeTwistConstraint* instance;
			desc state;

		private:
			ct_constraint(simulator* refer, const desc& i) noexcept;

		public:
			~ct_constraint() noexcept override;
			core::unique<constraint> copy() const override;
			btTypedConstraint* get() const override;
			bool has_collisions() const override;
			void enable_motor(bool value);
			void set_frames(const trigonometry::matrix4x4& a, const trigonometry::matrix4x4& b);
			void set_angular_only(bool value);
			void set_limit(int limit_index, float limit_value);
			void set_limit(float swing_span1, float swing_span2, float twist_span, float softness = 1.f, float bias_factor = 0.3f, float relaxation_factor = 1.0f);
			void set_damping(float value);
			void set_max_motor_impulse(float value);
			void set_max_motor_impulse_normalized(float value);
			void set_fix_thresh(float value);
			void set_motor_target(const trigonometry::quaternion& value);
			void set_motor_target_in_constraint_space(const trigonometry::quaternion& value);
			trigonometry::vector3 get_point_for_angle(float angle_in_radians, float length) const;
			trigonometry::quaternion get_motor_target() const;
			int get_solve_twist_limit() const;
			int get_solve_swing_limit() const;
			float get_twist_limit_sign() const;
			float get_swing_span1() const;
			float get_swing_span2() const;
			float get_twist_span() const;
			float get_limit_softness() const;
			float get_bias_factor() const;
			float get_relaxation_factor() const;
			float get_twist_angle() const;
			float get_limit(int value) const;
			float get_damping() const;
			float get_max_motor_impulse() const;
			float get_fix_thresh() const;
			bool is_motor_enabled() const;
			bool is_max_motor_impulse_normalized() const;
			bool is_past_swing_limit() const;
			bool is_angular_only() const;
			desc& get_state();
		};

		class df6_constraint : public constraint
		{
			friend rigid_body;
			friend simulator;

		public:
			struct desc
			{
				rigid_body* target_a = nullptr;
				rigid_body* target_b = nullptr;
				bool collisions = true;
			};

		private:
			btGeneric6DofSpring2Constraint* instance;
			desc state;

		private:
			df6_constraint(simulator* refer, const desc& i) noexcept;

		public:
			~df6_constraint() noexcept override;
			core::unique<constraint> copy() const override;
			btTypedConstraint* get() const override;
			bool has_collisions() const override;
			void enable_motor(int index, bool on_off);
			void enable_spring(int index, bool on_off);
			void set_frames(const trigonometry::matrix4x4& a, const trigonometry::matrix4x4& b);
			void set_linear_lower_limit(const trigonometry::vector3& value);
			void set_linear_upper_limit(const trigonometry::vector3& value);
			void set_angular_lower_limit(const trigonometry::vector3& value);
			void set_angular_lower_limit_reversed(const trigonometry::vector3& value);
			void set_angular_upper_limit(const trigonometry::vector3& value);
			void set_angular_upper_limit_reversed(const trigonometry::vector3& value);
			void set_limit(int axis, float low, float high);
			void set_limit_reversed(int axis, float low, float high);
			void set_rotation_order(trigonometry::rotator order);
			void set_axis(const trigonometry::vector3& a, const trigonometry::vector3& b);
			void set_bounce(int index, float bounce);
			void set_servo(int index, bool on_off);
			void set_target_velocity(int index, float velocity);
			void set_servo_target(int index, float target);
			void set_max_motor_force(int index, float force);
			void set_stiffness(int index, float stiffness, bool limit_if_needed = true);
			void set_equilibrium_point();
			void set_equilibrium_point(int index);
			void set_equilibrium_point(int index, float value);
			trigonometry::vector3 get_angular_upper_limit() const;
			trigonometry::vector3 get_angular_upper_limit_reversed() const;
			trigonometry::vector3 get_angular_lower_limit() const;
			trigonometry::vector3 get_angular_lower_limit_reversed() const;
			trigonometry::vector3 get_linear_upper_limit() const;
			trigonometry::vector3 get_linear_lower_limit() const;
			trigonometry::vector3 get_axis(int value) const;
			trigonometry::rotator get_rotation_order() const;
			float get_angle(int value) const;
			float get_relative_pivot_position(int value) const;
			bool is_limited(int limit_index) const;
			desc& get_state();
		};

		class simulator final : public core::reference<simulator>
		{
		public:
			struct desc
			{
				trigonometry::vector3 water_normal;
				trigonometry::vector3 gravity = trigonometry::vector3(0, -10, 0);
				float air_density = 1.2f;
				float water_density = 0;
				float water_offset = 0;
				float max_displacement = 1000;
				bool enable_soft_body = false;
			};

		private:
			struct
			{
				float last_elapsed_time = 0.0f;
			} timing;

		private:
			core::unordered_map<void*, size_t> shapes;
			btCollisionConfiguration* collision;
			btBroadphaseInterface* broadphase;
			btConstraintSolver* solver;
			btDiscreteDynamicsWorld* world;
			btCollisionDispatcher* dispatcher;
			btSoftBodySolver* soft_solver;
			std::mutex exclusive;

		public:
			float speedup;
			bool active;

		public:
			simulator(const desc& i) noexcept;
			~simulator() noexcept;
			void set_gravity(const trigonometry::vector3& gravity);
			void set_linear_impulse(const trigonometry::vector3& impulse, bool random_factor = false);
			void set_linear_impulse(const trigonometry::vector3& impulse, int start, int end, bool random_factor = false);
			void set_angular_impulse(const trigonometry::vector3& impulse, bool random_factor = false);
			void set_angular_impulse(const trigonometry::vector3& impulse, int start, int end, bool random_factor = false);
			void set_on_collision_enter(contact_started_callback callback);
			void set_on_collision_exit(contact_ended_callback callback);
			void create_linear_impulse(const trigonometry::vector3& impulse, bool random_factor = false);
			void create_linear_impulse(const trigonometry::vector3& impulse, int start, int end, bool random_factor = false);
			void create_angular_impulse(const trigonometry::vector3& impulse, bool random_factor = false);
			void create_angular_impulse(const trigonometry::vector3& impulse, int start, int end, bool random_factor = false);
			void add_soft_body(soft_body* body);
			void remove_soft_body(soft_body* body);
			void add_rigid_body(rigid_body* body);
			void remove_rigid_body(rigid_body* body);
			void add_constraint(constraint* constraint);
			void remove_constraint(constraint* constraint);
			void remove_all();
			void simulate_step(float elapsed_time_seconds);
			void find_contacts(rigid_body* body, int(*callback)(shape_contact*, const collision_body&, const collision_body&));
			bool find_ray_contacts(const trigonometry::vector3& start, const trigonometry::vector3& end, int(*callback)(ray_contact*, const collision_body&));
			core::unique<rigid_body> create_rigid_body(const rigid_body::desc& i);
			core::unique<rigid_body> create_rigid_body(const rigid_body::desc& i, trigonometry::transform* transform);
			core::unique<soft_body> create_soft_body(const soft_body::desc& i);
			core::unique<soft_body> create_soft_body(const soft_body::desc& i, trigonometry::transform* transform);
			core::unique<pconstraint> create_point2_point_constraint(const pconstraint::desc& i);
			core::unique<hconstraint> create_hinge_constraint(const hconstraint::desc& i);
			core::unique<sconstraint> create_slider_constraint(const sconstraint::desc& i);
			core::unique<ct_constraint> create_cone_twist_constraint(const ct_constraint::desc& i);
			core::unique<df6_constraint> create_6dof_constraint(const df6_constraint::desc& i);
			btCollisionShape* create_shape(shape type);
			btCollisionShape* create_cube(const trigonometry::vector3& scale = trigonometry::vector3(1, 1, 1));
			btCollisionShape* create_sphere(float radius = 1);
			btCollisionShape* create_capsule(float radius = 1, float height = 1);
			btCollisionShape* create_cone(float radius = 1, float height = 1);
			btCollisionShape* create_cylinder(const trigonometry::vector3& scale = trigonometry::vector3(1, 1, 1));
			btCollisionShape* create_convex_hull(core::vector<trigonometry::skin_vertex>& mesh);
			btCollisionShape* create_convex_hull(core::vector<trigonometry::vertex>& mesh);
			btCollisionShape* create_convex_hull(core::vector<trigonometry::vector2>& mesh);
			btCollisionShape* create_convex_hull(core::vector<trigonometry::vector3>& mesh);
			btCollisionShape* create_convex_hull(core::vector<trigonometry::vector4>& mesh);
			btCollisionShape* create_convex_hull(btCollisionShape* from);
			btCollisionShape* try_clone_shape(btCollisionShape* shape);
			btCollisionShape* reuse_shape(btCollisionShape* shape);
			void free_shape(core::unique<btCollisionShape*> value);
			core::vector<trigonometry::vector3> get_shape_vertices(btCollisionShape* shape) const;
			size_t get_shape_vertices_count(btCollisionShape* shape) const;
			float get_max_displacement() const;
			float get_air_density() const;
			float get_water_offset() const;
			float get_water_density() const;
			trigonometry::vector3 get_water_normal() const;
			trigonometry::vector3 get_gravity() const;
			contact_started_callback get_on_collision_enter() const;
			contact_ended_callback get_on_collision_exit() const;
			btCollisionConfiguration* get_collision() const;
			btBroadphaseInterface* get_broadphase() const;
			btConstraintSolver* get_solver() const;
			btDiscreteDynamicsWorld* get_world() const;
			btCollisionDispatcher* get_dispatcher() const;
			btSoftBodySolver* get_soft_solver() const;
			bool has_soft_body_support() const;
			int get_contact_manifold_count() const;

		public:
			static simulator* get(btDiscreteDynamicsWorld* from);
		};
	}
}
#endif