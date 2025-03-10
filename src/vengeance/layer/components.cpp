#include "components.h"
#include "renderers.h"
#include "../bindings.h"
#include "../audio/effects.h"

namespace
{
	enum
	{
		BOX_NONE = 0,
		BOX_GEOMETRY = 1,
		BOX_LIGHT = 2,
		BOX_BODY = 3,
		BOX_DYNAMIC = 4
	};
}

namespace vitex
{
	namespace layer
	{
		namespace components
		{
			static float get_visibility_radius(entity* base, const viewer& view, float distance)
			{
				float visibility = 1.0f - distance / view.far_plane;
				if (visibility <= 0.0f)
					return 0.0f;

				const trigonometry::matrix4x4& box = base->get_transform()->get_bias_unscaled();
				return trigonometry::geometric::is_cube_in_frustum(box * view.view_projection, base->get_radius()) ? visibility : 0.0f;
			}

			rigid_body::rigid_body(entity* ref) : component(ref, actor_set::synchronize)
			{
			}
			rigid_body::~rigid_body()
			{
				clear();
			}
			void rigid_body::deserialize_body(core::schema* node)
			{
				size_t activation_state;
				if (series::unpack_a(node->find("activation-state"), &activation_state))
					instance->set_activation_state((physics::motion_state)activation_state);

				float angular_damping;
				if (series::unpack(node->find("angular-damping"), &angular_damping))
					instance->set_angular_damping(angular_damping);

				float angular_sleeping_threshold;
				if (series::unpack(node->find("angular-sleeping-threshold"), &angular_sleeping_threshold))
					instance->set_angular_sleeping_threshold(angular_sleeping_threshold);

				float friction;
				if (series::unpack(node->find("friction"), &friction))
					instance->set_friction(friction);

				float restitution;
				if (series::unpack(node->find("restitution"), &restitution))
					instance->set_restitution(restitution);

				float hit_fraction;
				if (series::unpack(node->find("hit-fraction"), &hit_fraction))
					instance->set_hit_fraction(hit_fraction);

				float linear_damping;
				if (series::unpack(node->find("linear-damping"), &linear_damping))
					instance->set_linear_damping(linear_damping);

				float linear_sleeping_threshold;
				if (series::unpack(node->find("linear-sleeping-threshold"), &linear_sleeping_threshold))
					instance->set_linear_sleeping_threshold(linear_sleeping_threshold);

				float ccd_swept_sphere_radius;
				if (series::unpack(node->find("ccd-swept-sphere-radius"), &ccd_swept_sphere_radius))
					instance->set_ccd_swept_sphere_radius(ccd_swept_sphere_radius);

				float contact_processing_threshold;
				if (series::unpack(node->find("contact-processing-threshold"), &contact_processing_threshold))
					instance->set_contact_processing_threshold(contact_processing_threshold);

				float deactivation_time;
				if (series::unpack(node->find("deactivation-time"), &deactivation_time))
					instance->set_deactivation_time(deactivation_time);

				float rolling_friction;
				if (series::unpack(node->find("rolling-friction"), &rolling_friction))
					instance->set_rolling_friction(rolling_friction);

				float spinning_friction;
				if (series::unpack(node->find("spinning-friction"), &spinning_friction))
					instance->set_spinning_friction(spinning_friction);

				float contact_stiffness;
				if (series::unpack(node->find("contact-stiffness"), &contact_stiffness))
					instance->set_contact_stiffness(contact_stiffness);

				float contact_damping;
				if (series::unpack(node->find("contact-damping"), &contact_damping))
					instance->set_contact_damping(contact_damping);

				trigonometry::vector3 angular_factor;
				if (heavy_series::unpack(node->find("angular-factor"), &angular_factor))
					instance->set_angular_factor(angular_factor);

				trigonometry::vector3 angular_velocity;
				if (heavy_series::unpack(node->find("angular-velocity"), &angular_velocity))
					instance->set_angular_velocity(angular_velocity);

				trigonometry::vector3 anisotropic_friction;
				if (heavy_series::unpack(node->find("anisotropic-friction"), &anisotropic_friction))
					instance->set_anisotropic_friction(anisotropic_friction);

				trigonometry::vector3 gravity;
				if (heavy_series::unpack(node->find("gravity"), &gravity))
					instance->set_gravity(gravity);

				trigonometry::vector3 linear_factor;
				if (heavy_series::unpack(node->find("linear-factor"), &linear_factor))
					instance->set_linear_factor(linear_factor);

				trigonometry::vector3 linear_velocity;
				if (heavy_series::unpack(node->find("linear-velocity"), &linear_velocity))
					instance->set_linear_velocity(linear_velocity);

				size_t collision_flags;
				if (series::unpack_a(node->find("collision-flags"), &collision_flags))
					instance->set_collision_flags(collision_flags);
			}
			void rigid_body::deserialize(core::schema* node)
			{
				VI_ASSERT(node != nullptr, "schema should be set");

				core::schema* shaping = node->find("shape");
				core::vector<trigonometry::vector3> vertices;
				core::string path; size_t type;
				float mass = 0.0f, anticipation = 0.0f;
				bool extended = false;

				series::unpack(node->find("mass"), &mass);
				series::unpack(node->find("ccd-motion-threshold"), &anticipation);
				series::unpack(node->find("kinematic"), &kinematic);
				series::unpack(node->find("manage"), &manage);
				series::unpack(node->find("extended"), &extended);

				if (!extended)
					return;

				if (shaping != nullptr)
				{
					if (series::unpack(shaping->find("path"), &path))
					{
						node->add_ref();
						return load(path, mass, anticipation, [this, node]()
						{
							if (instance != nullptr)
								deserialize_body(node);
							node->release();
						});
					}
					else if (series::unpack_a(shaping->find("type"), &type))
					{
						btCollisionShape* shape = parent->get_scene()->get_simulator()->create_shape((physics::shape)type);
						if (shape != nullptr)
							load(shape, mass, anticipation);
					}
					else if (heavy_series::unpack(shaping->find("data"), &vertices))
					{
						btCollisionShape* shape = parent->get_scene()->get_simulator()->create_convex_hull(vertices);
						if (shape != nullptr)
							load(shape, mass, anticipation);
					}
				}

				if (instance != nullptr)
					deserialize_body(node);
			}
			void rigid_body::serialize(core::schema* node)
			{
				VI_ASSERT(node != nullptr, "schema should be set");

				scene_graph* scene = parent->get_scene();
				series::pack(node->set("kinematic"), kinematic);
				series::pack(node->set("manage"), manage);
				series::pack(node->set("extended"), instance != nullptr);

				if (!instance)
					return;

				core::schema* shaping = node->set("shape");
				if (instance->get_collision_shape_type() == physics::shape::convex_hull)
				{
					core::string path = scene->find_resource_id<physics::hull_shape>(hull);
					if (path.empty())
					{
						core::vector<trigonometry::vector3> vertices = scene->get_simulator()->get_shape_vertices(instance->get_collision_shape());
						heavy_series::pack(shaping->set("data"), vertices);
					}
					else
						series::pack(shaping->set("path"), path);
				}
				else
					series::pack(shaping->set("type"), (uint64_t)instance->get_collision_shape_type());

				series::pack(node->set("mass"), instance->get_mass());
				series::pack(node->set("ccd-motion-threshold"), instance->get_ccd_motion_threshold());
				series::pack(node->set("activation-state"), (uint64_t)instance->get_activation_state());
				series::pack(node->set("angular-damping"), instance->get_angular_damping());
				series::pack(node->set("angular-sleeping-threshold"), instance->get_angular_sleeping_threshold());
				series::pack(node->set("friction"), instance->get_friction());
				series::pack(node->set("restitution"), instance->get_restitution());
				series::pack(node->set("hit-fraction"), instance->get_hit_fraction());
				series::pack(node->set("linear-damping"), instance->get_linear_damping());
				series::pack(node->set("linear-sleeping-threshold"), instance->get_linear_sleeping_threshold());
				series::pack(node->set("ccd-swept-sphere-radius"), instance->get_ccd_swept_sphere_radius());
				series::pack(node->set("contact-processing-threshold"), instance->get_contact_processing_threshold());
				series::pack(node->set("deactivation-time"), instance->get_deactivation_time());
				series::pack(node->set("rolling-friction"), instance->get_rolling_friction());
				series::pack(node->set("spinning-friction"), instance->get_spinning_friction());
				series::pack(node->set("contact-stiffness"), instance->get_contact_stiffness());
				series::pack(node->set("contact-damping"), instance->get_contact_damping());
				heavy_series::pack(node->set("angular-factor"), instance->get_angular_factor());
				heavy_series::pack(node->set("angular-velocity"), instance->get_angular_velocity());
				heavy_series::pack(node->set("anisotropic-friction"), instance->get_anisotropic_friction());
				heavy_series::pack(node->set("gravity"), instance->get_gravity());
				heavy_series::pack(node->set("linear-factor"), instance->get_linear_factor());
				heavy_series::pack(node->set("linear-velocity"), instance->get_linear_velocity());
				series::pack(node->set("collision-flags"), (uint64_t)instance->get_collision_flags());
			}
			void rigid_body::synchronize(core::timer* time)
			{
				if (instance && manage)
					instance->synchronize(parent->get_transform(), kinematic);
			}
			void rigid_body::deactivate()
			{
				if (instance != nullptr)
					instance->set_as_ghost();
			}
			void rigid_body::load(btCollisionShape* shape, float mass, float anticipation)
			{
				scene_graph* scene = parent->get_scene();
				VI_ASSERT(scene != nullptr, "scene should be set");
				VI_ASSERT(shape != nullptr, "collision shape should be set");

				physics::rigid_body::desc i;
				i.anticipation = anticipation;
				i.mass = mass;
				i.shape = shape;

				core::memory::release(instance);
				instance = scene->get_simulator()->create_rigid_body(i, parent->get_transform());
				instance->user_pointer = this;
				instance->set_activity(true);

				get_entity()->get_transform()->make_dirty();
			}
			void rigid_body::load(const std::string_view& path, float mass, float anticipation, std::function<void()>&& callback)
			{
				parent->get_scene()->load_resource<physics::hull_shape>(this, path, [this, mass, anticipation, callback = std::move(callback)](expects_content<physics::hull_shape*> new_hull)
				{
					core::memory::release(hull);
					hull = new_hull.or_else(nullptr);
					if (hull != nullptr)
						load(hull->get_shape(), mass, anticipation);
					else
						clear();

					if (callback)
						callback();
				});
			}
			void rigid_body::clear()
			{
				core::memory::release(instance);
				core::memory::release(hull);
			}
			void rigid_body::set_transform(const trigonometry::vector3& position, const trigonometry::vector3& scale, const trigonometry::vector3& rotation)
			{
				if (!instance)
					return;

				trigonometry::transform::spacing space;
				space.position = position;
				space.rotation = rotation;
				space.scale = scale;

				auto* transform = parent->get_transform();
				transform->set_spacing(trigonometry::positioning::global, space);
				instance->synchronize(transform, true);
				instance->set_activity(true);
			}
			void rigid_body::set_transform(bool kinematics)
			{
				if (!instance)
					return;

				auto* transform = parent->get_transform();
				instance->synchronize(transform, kinematics);
				instance->set_activity(true);
			}
			void rigid_body::set_mass(float mass)
			{
				if (instance != nullptr)
					instance->set_mass(mass);
			}
			component* rigid_body::copy(entity* init) const
			{
				rigid_body* target = new rigid_body(init);
				target->kinematic = kinematic;

				if (instance != nullptr)
				{
					target->instance = instance->copy();
					target->instance->user_pointer = target;
				}

				return target;
			}
			physics::rigid_body* rigid_body::get_body() const
			{
				return instance;
			}

			soft_body::soft_body(entity* ref) : drawable(ref, actor_set::synchronize, soft_body::get_type_id())
			{
			}
			soft_body::~soft_body()
			{
				clear();
			}
			void soft_body::deserialize_body(core::schema* node)
			{
				core::schema* conf = node->get("config");
				if (conf != nullptr)
				{
					physics::soft_body::desc::sconfig i;
					series::unpack(conf->get("vcf"), &i.vcf);
					series::unpack(conf->get("dp"), &i.dp);
					series::unpack(conf->get("dg"), &i.dg);
					series::unpack(conf->get("lf"), &i.lf);
					series::unpack(conf->get("pr"), &i.pr);
					series::unpack(conf->get("vc"), &i.vc);
					series::unpack(conf->get("df"), &i.df);
					series::unpack(conf->get("mt"), &i.mt);
					series::unpack(conf->get("chr"), &i.chr);
					series::unpack(conf->get("khr"), &i.khr);
					series::unpack(conf->get("shr"), &i.shr);
					series::unpack(conf->get("ahr"), &i.ahr);
					series::unpack(conf->get("srhr-cl"), &i.srhr_cl);
					series::unpack(conf->get("skhr-cl"), &i.skhr_cl);
					series::unpack(conf->get("sshr-cl"), &i.sshr_cl);
					series::unpack(conf->get("sr-splt-cl"), &i.sr_splt_cl);
					series::unpack(conf->get("sk-splt-cl"), &i.sk_splt_cl);
					series::unpack(conf->get("ss-splt-cl"), &i.ss_splt_cl);
					series::unpack(conf->get("max-volume"), &i.max_volume);
					series::unpack(conf->get("time-scale"), &i.time_scale);
					series::unpack(conf->get("drag"), &i.drag);
					series::unpack(conf->get("max-stress"), &i.max_stress);
					series::unpack(conf->get("constraints"), &i.constraints);
					series::unpack(conf->get("clusters"), &i.clusters);
					series::unpack(conf->get("v-it"), &i.viterations);
					series::unpack(conf->get("p-it"), &i.piterations);
					series::unpack(conf->get("d-it"), &i.diterations);
					series::unpack(conf->get("c-it"), &i.citerations);
					series::unpack(conf->get("collisions"), &i.collisions);

					size_t aero_model;
					if (series::unpack_a(conf->get("aero-model"), &aero_model))
						i.aero_model = (physics::soft_aero_model)aero_model;

					instance->set_config(i);
				}

				size_t activation_state;
				if (series::unpack_a(node->find("activation-state"), &activation_state))
					instance->set_activation_state((physics::motion_state)activation_state);

				float friction;
				if (series::unpack(node->find("friction"), &friction))
					instance->set_friction(friction);

				float restitution;
				if (series::unpack(node->find("restitution"), &restitution))
					instance->set_restitution(restitution);

				float hit_fraction;
				if (series::unpack(node->find("hit-fraction"), &hit_fraction))
					instance->set_hit_fraction(hit_fraction);

				float ccd_swept_sphere_radius;
				if (series::unpack(node->find("ccd-swept-sphere-radius"), &ccd_swept_sphere_radius))
					instance->set_ccd_swept_sphere_radius(ccd_swept_sphere_radius);

				float contact_processing_threshold;
				if (series::unpack(node->find("contact-processing-threshold"), &contact_processing_threshold))
					instance->set_contact_processing_threshold(contact_processing_threshold);

				float deactivation_time;
				if (series::unpack(node->find("deactivation-time"), &deactivation_time))
					instance->set_deactivation_time(deactivation_time);

				float rolling_friction;
				if (series::unpack(node->find("rolling-friction"), &rolling_friction))
					instance->set_rolling_friction(rolling_friction);

				float spinning_friction;
				if (series::unpack(node->find("spinning-friction"), &spinning_friction))
					instance->set_spinning_friction(spinning_friction);

				float contact_stiffness;
				if (series::unpack(node->find("contact-stiffness"), &contact_stiffness))
					instance->set_contact_stiffness(contact_stiffness);

				float contact_damping;
				if (series::unpack(node->find("contact-damping"), &contact_damping))
					instance->set_contact_damping(contact_damping);

				trigonometry::vector3 anisotropic_friction;
				if (heavy_series::unpack(node->find("anisotropic-friction"), &anisotropic_friction))
					instance->set_anisotropic_friction(anisotropic_friction);

				trigonometry::vector3 wind_velocity;
				if (heavy_series::unpack(node->find("wind-velocity"), &wind_velocity))
					instance->set_wind_velocity(wind_velocity);

				float total_mass;
				if (series::unpack(node->find("total-mass"), &total_mass))
					instance->set_total_mass(total_mass);

				float rest_length_scale;
				if (series::unpack(node->find("core-length-scale"), &rest_length_scale))
					instance->set_rest_length_scale(rest_length_scale);
			}
			void soft_body::deserialize(core::schema* node)
			{
				VI_ASSERT(node != nullptr, "schema should be set");

				float anticipation = 0.0f; bool extended = false;
				uint32_t new_category = (uint32_t)geo_category::opaque;
				series::unpack(node->find("ccd-motion-threshold"), &anticipation);
				heavy_series::unpack(node->find("texcoord"), &texcoord);
				series::unpack(node->find("extended"), &extended);
				series::unpack(node->find("kinematic"), &kinematic);
				series::unpack(node->find("manage"), &manage);
				series::unpack(node->find("static"), &constant);
				series::unpack(node->find("category"), &new_category);
				set_category((geo_category)new_category);

				size_t slot = 0;
				if (series::unpack_a(node->find("material"), &slot))
					set_material(nullptr, parent->get_scene()->get_material(slot));

				if (!extended)
					return;

				core::schema* shaping = nullptr;
				if ((shaping = node->find("shape")) != nullptr)
				{
					core::string path;
					if (series::unpack(shaping->find("path"), &path))
					{
						node->add_ref();
						return load(path, anticipation, [this, node]()
						{
							if (instance != nullptr)
								deserialize_body(node);
							node->release();
						});
					}
				}
				else if ((shaping = node->find("ellipsoid")) != nullptr)
				{
					physics::soft_body::desc::cv::sellipsoid shape;
					heavy_series::unpack(shaping->get("center"), &shape.center);
					heavy_series::unpack(shaping->get("radius"), &shape.radius);
					series::unpack(shaping->get("count"), &shape.count);
					load_ellipsoid(shape, anticipation);
				}
				else if ((shaping = node->find("patch")) != nullptr)
				{
					physics::soft_body::desc::cv::spatch shape;
					heavy_series::unpack(shaping->get("corner-00"), &shape.corner00);
					series::unpack(shaping->get("corner-00-fixed"), &shape.corner00_fixed);
					heavy_series::unpack(shaping->get("corner-01"), &shape.corner01);
					series::unpack(shaping->get("corner-01-fixed"), &shape.corner01_fixed);
					heavy_series::unpack(shaping->get("corner-10"), &shape.corner10);
					series::unpack(shaping->get("corner-10-fixed"), &shape.corner10_fixed);
					heavy_series::unpack(shaping->get("corner-11"), &shape.corner11);
					series::unpack(shaping->get("corner-11-fixed"), &shape.corner11_fixed);
					series::unpack(shaping->get("count-x"), &shape.count_x);
					series::unpack(shaping->get("count-y"), &shape.count_y);
					series::unpack(shaping->get("diagonals"), &shape.generate_diagonals);
					load_patch(shape, anticipation);
				}
				else if ((shaping = node->find("rope")) != nullptr)
				{
					physics::soft_body::desc::cv::srope shape;
					heavy_series::unpack(shaping->get("start"), &shape.start);
					series::unpack(shaping->get("start-fixed"), &shape.start_fixed);
					heavy_series::unpack(shaping->get("end"), &shape.end);
					series::unpack(shaping->get("end-fixed"), &shape.end_fixed);
					series::unpack(shaping->get("count"), &shape.count);
					load_rope(shape, anticipation);
				}

				if (instance != nullptr)
					deserialize_body(node);
			}
			void soft_body::serialize(core::schema* node)
			{
				VI_ASSERT(node != nullptr, "schema should be set");

				material* slot = get_material();
				if (slot != nullptr)
					series::pack(node->set("material"), (uint64_t)slot->slot);

				heavy_series::pack(node->set("texcoord"), texcoord);
				series::pack(node->set("category"), (uint32_t)get_category());
				series::pack(node->set("kinematic"), kinematic);
				series::pack(node->set("manage"), manage);
				series::pack(node->set("extended"), instance != nullptr);
				series::pack(node->set("static"), constant);

				if (!instance)
					return;

				physics::soft_body::desc& i = instance->get_initial_state();
				core::schema* conf = node->set("config");
				series::pack(conf->set("aero-model"), (uint64_t)i.config.aero_model);
				series::pack(conf->set("vcf"), i.config.vcf);
				series::pack(conf->set("dp"), i.config.dp);
				series::pack(conf->set("dg"), i.config.dg);
				series::pack(conf->set("lf"), i.config.lf);
				series::pack(conf->set("pr"), i.config.pr);
				series::pack(conf->set("vc"), i.config.vc);
				series::pack(conf->set("df"), i.config.df);
				series::pack(conf->set("mt"), i.config.mt);
				series::pack(conf->set("chr"), i.config.chr);
				series::pack(conf->set("khr"), i.config.khr);
				series::pack(conf->set("shr"), i.config.shr);
				series::pack(conf->set("ahr"), i.config.ahr);
				series::pack(conf->set("srhr-cl"), i.config.srhr_cl);
				series::pack(conf->set("skhr-cl"), i.config.skhr_cl);
				series::pack(conf->set("sshr-cl"), i.config.sshr_cl);
				series::pack(conf->set("sr-splt-cl"), i.config.sr_splt_cl);
				series::pack(conf->set("sk-splt-cl"), i.config.sk_splt_cl);
				series::pack(conf->set("ss-splt-cl"), i.config.ss_splt_cl);
				series::pack(conf->set("max-volume"), i.config.max_volume);
				series::pack(conf->set("time-scale"), i.config.time_scale);
				series::pack(conf->set("drag"), i.config.drag);
				series::pack(conf->set("max-stress"), i.config.max_stress);
				series::pack(conf->set("constraints"), i.config.constraints);
				series::pack(conf->set("clusters"), i.config.clusters);
				series::pack(conf->set("v-it"), i.config.viterations);
				series::pack(conf->set("p-it"), i.config.piterations);
				series::pack(conf->set("d-it"), i.config.diterations);
				series::pack(conf->set("c-it"), i.config.citerations);
				series::pack(conf->set("collisions"), i.config.collisions);

				auto& desc = instance->get_initial_state();
				if (desc.shape.convex.enabled && hull != nullptr)
				{
					if (instance->get_collision_shape_type() == physics::shape::convex_hull)
					{
						core::string path = parent->get_scene()->find_resource_id<physics::hull_shape>(hull);
						if (!path.empty())
							series::pack(node->set("shape")->set("path"), path);
					}
				}
				else if (desc.shape.ellipsoid.enabled)
				{
					core::schema* shape = node->set("ellipsoid");
					heavy_series::pack(shape->set("center"), desc.shape.ellipsoid.center);
					heavy_series::pack(shape->set("radius"), desc.shape.ellipsoid.radius);
					series::pack(shape->set("count"), desc.shape.ellipsoid.count);
				}
				else if (desc.shape.patch.enabled)
				{
					core::schema* shape = node->set("patch");
					heavy_series::pack(shape->set("corner-00"), desc.shape.patch.corner00);
					series::pack(shape->set("corner-00-fixed"), desc.shape.patch.corner00_fixed);
					heavy_series::pack(shape->set("corner-01"), desc.shape.patch.corner01);
					series::pack(shape->set("corner-01-fixed"), desc.shape.patch.corner01_fixed);
					heavy_series::pack(shape->set("corner-10"), desc.shape.patch.corner10);
					series::pack(shape->set("corner-10-fixed"), desc.shape.patch.corner10_fixed);
					heavy_series::pack(shape->set("corner-11"), desc.shape.patch.corner11);
					series::pack(shape->set("corner-11-fixed"), desc.shape.patch.corner11_fixed);
					series::pack(shape->set("count-x"), desc.shape.patch.count_x);
					series::pack(shape->set("count-y"), desc.shape.patch.count_y);
					series::pack(shape->set("diagonals"), desc.shape.patch.generate_diagonals);
				}
				else if (desc.shape.rope.enabled)
				{
					core::schema* shape = node->set("rope");
					heavy_series::pack(shape->set("start"), desc.shape.rope.start);
					series::pack(shape->set("start-fixed"), desc.shape.rope.start_fixed);
					heavy_series::pack(shape->set("end"), desc.shape.rope.end);
					series::pack(shape->set("end-fixed"), desc.shape.rope.end_fixed);
					series::pack(shape->set("count"), desc.shape.rope.count);
				}

				series::pack(node->set("ccd-motion-threshold"), instance->get_ccd_motion_threshold());
				series::pack(node->set("activation-state"), (uint64_t)instance->get_activation_state());
				series::pack(node->set("friction"), instance->get_friction());
				series::pack(node->set("restitution"), instance->get_restitution());
				series::pack(node->set("hit-fraction"), instance->get_hit_fraction());
				series::pack(node->set("ccd-swept-sphere-radius"), instance->get_ccd_swept_sphere_radius());
				series::pack(node->set("contact-processing-threshold"), instance->get_contact_processing_threshold());
				series::pack(node->set("deactivation-time"), instance->get_deactivation_time());
				series::pack(node->set("rolling-friction"), instance->get_rolling_friction());
				series::pack(node->set("spinning-friction"), instance->get_spinning_friction());
				series::pack(node->set("contact-stiffness"), instance->get_contact_stiffness());
				series::pack(node->set("contact-damping"), instance->get_contact_damping());
				heavy_series::pack(node->set("angular-velocity"), instance->get_angular_velocity());
				heavy_series::pack(node->set("anisotropic-friction"), instance->get_anisotropic_friction());
				heavy_series::pack(node->set("linear-velocity"), instance->get_linear_velocity());
				series::pack(node->set("collision-flags"), (uint64_t)instance->get_collision_flags());
				heavy_series::pack(node->set("wind-velocity"), instance->get_wind_velocity());
				series::pack(node->set("total-mass"), instance->get_total_mass());
				series::pack(node->set("core-length-scale"), instance->get_rest_length_scale());
			}
			void soft_body::synchronize(core::timer* time)
			{
				if (!instance)
					return;

				if (manage)
				{
					auto* transform = parent->get_transform();
					instance->synchronize(transform, kinematic);
					if (instance->is_active())
						transform->make_dirty();
				}

				instance->get_vertices(&vertices);
				if (indices.empty())
					instance->get_indices(&indices);
			}
			void soft_body::deactivate()
			{
				if (instance != nullptr)
					instance->set_as_ghost();
			}
			void soft_body::load(physics::hull_shape* shape, float anticipation)
			{
				scene_graph* scene = parent->get_scene();
				VI_ASSERT(scene != nullptr, "scene should be set");
				VI_ASSERT(shape != nullptr, "collision shape should be set");
				core::memory::release(hull);
				hull = shape;

				physics::soft_body::desc i;
				i.anticipation = anticipation;
				i.shape.convex.hull = hull;
				i.shape.convex.enabled = true;

				core::memory::release(instance);
				instance = scene->get_simulator()->create_soft_body(i, parent->get_transform());
				VI_PANIC(!instance, "invalid simulator configuration to create soft bodies");

				vertices.clear();
				indices.clear();

				instance->user_pointer = this;
				instance->set_activity(true);
				get_entity()->get_transform()->make_dirty();
			}
			void soft_body::load(const std::string_view& path, float anticipation, std::function<void()>&& callback)
			{
				parent->get_scene()->load_resource<physics::hull_shape>(this, path, [this, anticipation, callback = std::move(callback)](expects_content<physics::hull_shape*> new_hull)
				{
					if (new_hull && *new_hull != nullptr)
						load(*new_hull, anticipation);
					else
						clear();

					if (callback)
						callback();
				});
			}
			void soft_body::load_ellipsoid(const physics::soft_body::desc::cv::sellipsoid& shape, float anticipation)
			{
				scene_graph* scene = parent->get_scene();
				VI_ASSERT(scene != nullptr, "scene should be set");

				physics::soft_body::desc i;
				i.anticipation = anticipation;
				i.shape.ellipsoid = shape;
				i.shape.ellipsoid.enabled = true;

				core::memory::release(instance);
				instance = scene->get_simulator()->create_soft_body(i, parent->get_transform());
				VI_PANIC(!instance, "invalid simulator configuration to create soft bodies");

				vertices.clear();
				indices.clear();

				instance->user_pointer = this;
				instance->set_activity(true);
			}
			void soft_body::load_patch(const physics::soft_body::desc::cv::spatch& shape, float anticipation)
			{
				scene_graph* scene = parent->get_scene();
				VI_ASSERT(scene != nullptr, "scene should be set");

				physics::soft_body::desc i;
				i.anticipation = anticipation;
				i.shape.patch = shape;
				i.shape.patch.enabled = true;

				core::memory::release(instance);
				instance = scene->get_simulator()->create_soft_body(i, parent->get_transform());
				VI_PANIC(!instance, "invalid simulator configuration to create soft bodies");

				vertices.clear();
				indices.clear();

				instance->user_pointer = this;
				instance->set_activity(true);
			}
			void soft_body::load_rope(const physics::soft_body::desc::cv::srope& shape, float anticipation)
			{
				scene_graph* scene = parent->get_scene();
				VI_ASSERT(scene != nullptr, "scene should be set");

				physics::soft_body::desc i;
				i.anticipation = anticipation;
				i.shape.rope = shape;
				i.shape.rope.enabled = true;

				core::memory::release(instance);
				instance = scene->get_simulator()->create_soft_body(i, parent->get_transform());
				VI_PANIC(!instance, "invalid simulator configuration to create soft bodies");

				vertices.clear();
				indices.clear();

				instance->user_pointer = this;
				instance->set_activity(true);
			}
			void soft_body::fill(graphics::graphics_device* device, graphics::element_buffer* index_buffer, graphics::element_buffer* vertex_buffer)
			{
				graphics::mapped_subresource map;
				if (vertex_buffer != nullptr && !vertices.empty())
				{
					device->map(vertex_buffer, graphics::resource_map::write_discard, &map);
					memcpy(map.pointer, (void*)vertices.data(), vertices.size() * sizeof(trigonometry::vertex));
					device->unmap(vertex_buffer, &map);
				}

				if (index_buffer != nullptr && !indices.empty())
				{
					device->map(index_buffer, graphics::resource_map::write_discard, &map);
					memcpy(map.pointer, (void*)indices.data(), indices.size() * sizeof(int));
					device->unmap(index_buffer, &map);
				}
			}
			void soft_body::regenerate()
			{
				scene_graph* scene = parent->get_scene();
				VI_ASSERT(scene != nullptr, "scene should be set");

				if (!instance)
					return;

				physics::soft_body::desc i = instance->get_initial_state();
				core::memory::release(instance);

				instance = scene->get_simulator()->create_soft_body(i, parent->get_transform());
				VI_PANIC(!instance, "invalid simulator configuration to create soft bodies");
			}
			void soft_body::clear()
			{
				core::memory::release(instance);
				core::memory::release(hull);
			}
			void soft_body::set_transform(const trigonometry::vector3& position, const trigonometry::vector3& scale, const trigonometry::vector3& rotation)
			{
				if (!instance)
					return;

				trigonometry::transform::spacing space;
				space.position = position;
				space.rotation = rotation;
				space.scale = scale;

				auto* transform = parent->get_transform();
				transform->set_spacing(trigonometry::positioning::global, space);
				instance->synchronize(transform, true);
				instance->set_activity(true);
			}
			void soft_body::set_transform(bool kinematics)
			{
				if (!instance)
					return;

				auto* transform = parent->get_transform();
				instance->synchronize(transform, kinematics);
				instance->set_activity(true);
			}
			float soft_body::get_visibility(const viewer& view, float distance) const
			{
				return instance ? component::get_visibility(view, distance) : 0.0f;
			}
			size_t soft_body::get_unit_bounds(trigonometry::vector3& min, trigonometry::vector3& max) const
			{
				if (!instance)
					return BOX_NONE;

				trigonometry::vector3 center = instance->get_center_position();
				instance->get_bounding_box(&min, &max);

				trigonometry::vector3 scale = (max - min) * 0.5f;
				min = center - scale;
				max = center + scale;

				return BOX_BODY;
			}
			component* soft_body::copy(entity* init) const
			{
				soft_body* target = new soft_body(init);
				target->set_category(get_category());
				target->kinematic = kinematic;

				if (instance != nullptr)
				{
					target->instance = instance->copy();
					target->instance->user_pointer = target;
				}

				return target;
			}
			physics::soft_body* soft_body::get_body()
			{
				return instance;
			}
			core::vector<trigonometry::vertex>& soft_body::get_vertices()
			{
				return vertices;
			}
			core::vector<int>& soft_body::get_indices()
			{
				return indices;
			}

			slider_constraint::slider_constraint(entity* ref) : component(ref, actor_set::none), instance(nullptr), connection(nullptr)
			{
			}
			slider_constraint::~slider_constraint()
			{
				core::memory::release(instance);
			}
			void slider_constraint::deserialize(core::schema* node)
			{
				VI_ASSERT(node != nullptr, "schema should be set");

				bool extended, ghost, linear;
				series::unpack(node->find("extended"), &extended);
				series::unpack(node->find("collision-state"), &ghost);
				series::unpack(node->find("linear-state"), &linear);

				if (!extended)
					return;

				size_t connection_id = 0;
				if (series::unpack_a(node->find("connection"), &connection_id))
				{
					idx_snapshot* snapshot = parent->get_scene()->snapshot;
					if (snapshot != nullptr)
					{
						auto it = snapshot->from.find(connection_id);
						if (it != snapshot->from.end())
							connection = it->second;
					}
				}

				load(connection, ghost, linear);
				if (!instance)
					return;

				float angular_motor_velocity;
				if (series::unpack(node->find("angular-motor-velocity"), &angular_motor_velocity))
					instance->set_angular_motor_velocity(angular_motor_velocity);

				float linear_motor_velocity;
				if (series::unpack(node->find("linear-motor-velocity"), &linear_motor_velocity))
					instance->set_linear_motor_velocity(linear_motor_velocity);

				float upper_linear_limit;
				if (series::unpack(node->find("upper-linear-limit"), &upper_linear_limit))
					instance->set_upper_linear_limit(upper_linear_limit);

				float lower_linear_limit;
				if (series::unpack(node->find("lower-linear-limit"), &lower_linear_limit))
					instance->set_lower_linear_limit(lower_linear_limit);

				float angular_damping_direction;
				if (series::unpack(node->find("angular-damping-direction"), &angular_damping_direction))
					instance->set_angular_damping_direction(angular_damping_direction);

				float linear_damping_direction;
				if (series::unpack(node->find("linear-damping-direction"), &linear_damping_direction))
					instance->set_linear_damping_direction(linear_damping_direction);

				float angular_damping_limit;
				if (series::unpack(node->find("angular-damping-limit"), &angular_damping_limit))
					instance->set_angular_damping_limit(angular_damping_limit);

				float linear_damping_limit;
				if (series::unpack(node->find("linear-damping-limit"), &linear_damping_limit))
					instance->set_linear_damping_limit(linear_damping_limit);

				float angular_damping_ortho;
				if (series::unpack(node->find("angular-damping-ortho"), &angular_damping_ortho))
					instance->set_angular_damping_ortho(angular_damping_ortho);

				float linear_damping_ortho;
				if (series::unpack(node->find("linear-damping-ortho"), &linear_damping_ortho))
					instance->set_linear_damping_ortho(linear_damping_ortho);

				float upper_angular_limit;
				if (series::unpack(node->find("upper-angular-limit"), &upper_angular_limit))
					instance->set_upper_angular_limit(upper_angular_limit);

				float lower_angular_limit;
				if (series::unpack(node->find("lower-angular-limit"), &lower_angular_limit))
					instance->set_lower_angular_limit(lower_angular_limit);

				float max_angular_motor_force;
				if (series::unpack(node->find("max-angular-motor-force"), &max_angular_motor_force))
					instance->set_max_angular_motor_force(max_angular_motor_force);

				float max_linear_motor_force;
				if (series::unpack(node->find("max-linear-motor-force"), &max_linear_motor_force))
					instance->set_max_linear_motor_force(max_linear_motor_force);

				float angular_restitution_direction;
				if (series::unpack(node->find("angular-restitution-direction"), &angular_restitution_direction))
					instance->set_angular_restitution_direction(angular_restitution_direction);

				float linear_restitution_direction;
				if (series::unpack(node->find("linear-restitution-direction"), &linear_restitution_direction))
					instance->set_linear_restitution_direction(linear_restitution_direction);

				float angular_restitution_limit;
				if (series::unpack(node->find("angular-restitution-limit"), &angular_restitution_limit))
					instance->set_angular_restitution_limit(angular_restitution_limit);

				float linear_restitution_limit;
				if (series::unpack(node->find("linear-restitution-limit"), &linear_restitution_limit))
					instance->set_linear_restitution_limit(linear_restitution_limit);

				float angular_restitution_ortho;
				if (series::unpack(node->find("angular-restitution-ortho"), &angular_restitution_ortho))
					instance->set_angular_restitution_ortho(angular_restitution_ortho);

				float linear_restitution_ortho;
				if (series::unpack(node->find("linear-restitution-ortho"), &linear_restitution_ortho))
					instance->set_linear_restitution_ortho(linear_restitution_ortho);

				float angular_softness_direction;
				if (series::unpack(node->find("angular-softness-direction"), &angular_softness_direction))
					instance->set_angular_softness_direction(angular_softness_direction);

				float linear_softness_direction;
				if (series::unpack(node->find("linear-softness-direction"), &linear_softness_direction))
					instance->set_linear_softness_direction(linear_softness_direction);

				float angular_softness_limit;
				if (series::unpack(node->find("angular-softness-limit"), &angular_softness_limit))
					instance->set_angular_softness_limit(angular_softness_limit);

				float linear_softness_limit;
				if (series::unpack(node->find("linear-softness-limit"), &linear_softness_limit))
					instance->set_linear_softness_limit(linear_softness_limit);

				float angular_softness_ortho;
				if (series::unpack(node->find("angular-softness-ortho"), &angular_softness_ortho))
					instance->set_angular_softness_ortho(angular_softness_ortho);

				float linear_softness_ortho;
				if (series::unpack(node->find("linear-softness-ortho"), &linear_softness_ortho))
					instance->set_linear_softness_ortho(linear_softness_ortho);

				bool powered_angular_motor;
				if (series::unpack(node->find("powered-angular-motor"), &powered_angular_motor))
					instance->set_powered_angular_motor(powered_angular_motor);

				bool powered_linear_motor;
				if (series::unpack(node->find("powered-linear-motor"), &powered_linear_motor))
					instance->set_powered_linear_motor(powered_linear_motor);

				bool enabled;
				if (series::unpack(node->find("enabled"), &enabled))
					instance->set_enabled(enabled);
			}
			void slider_constraint::serialize(core::schema* node)
			{
				VI_ASSERT(node != nullptr, "schema should be set");

				series::pack(node->set("extended"), instance != nullptr);
				if (!instance)
					return;

				int64_t connection_id = -1;
				if (connection != nullptr)
				{
					idx_snapshot* snapshot = parent->get_scene()->snapshot;
					if (snapshot != nullptr)
					{
						auto it = snapshot->to.find(connection);
						if (it != snapshot->to.end())
							connection_id = (int64_t)it->second;
					}
				}

				series::pack(node->set("collision-state"), instance->get_state().collisions);
				series::pack(node->set("linear-state"), instance->get_state().linear);
				series::pack(node->set("connection"), connection_id);
				series::pack(node->set("angular-motor-velocity"), instance->get_angular_motor_velocity());
				series::pack(node->set("linear-motor-velocity"), instance->get_linear_motor_velocity());
				series::pack(node->set("upper-linear-limit"), instance->get_upper_linear_limit());
				series::pack(node->set("lower-linear-limit"), instance->get_lower_linear_limit());
				series::pack(node->set("breaking-impulse-threshold"), instance->get_breaking_impulse_threshold());
				series::pack(node->set("angular-damping-direction"), instance->get_angular_damping_direction());
				series::pack(node->set("linear-amping-direction"), instance->get_linear_damping_direction());
				series::pack(node->set("angular-damping-limit"), instance->get_angular_damping_limit());
				series::pack(node->set("linear-damping-limit"), instance->get_linear_damping_limit());
				series::pack(node->set("angular-damping-ortho"), instance->get_angular_damping_ortho());
				series::pack(node->set("linear-damping-ortho"), instance->get_linear_damping_ortho());
				series::pack(node->set("upper-angular-limit"), instance->get_upper_angular_limit());
				series::pack(node->set("lower-angular-limit"), instance->get_lower_angular_limit());
				series::pack(node->set("max-angular-motor-force"), instance->get_max_angular_motor_force());
				series::pack(node->set("max-linear-motor-force"), instance->get_max_linear_motor_force());
				series::pack(node->set("angular-restitution-direction"), instance->get_angular_restitution_direction());
				series::pack(node->set("linear-restitution-direction"), instance->get_linear_restitution_direction());
				series::pack(node->set("angular-restitution-limit"), instance->get_angular_restitution_limit());
				series::pack(node->set("linear-restitution-limit"), instance->get_linear_restitution_limit());
				series::pack(node->set("angular-restitution-ortho"), instance->get_angular_restitution_ortho());
				series::pack(node->set("linear-restitution-ortho"), instance->get_linear_restitution_ortho());
				series::pack(node->set("angular-softness-direction"), instance->get_angular_softness_direction());
				series::pack(node->set("linear-softness-direction"), instance->get_linear_softness_direction());
				series::pack(node->set("angular-softness-limit"), instance->get_angular_softness_limit());
				series::pack(node->set("linear-softness-limit"), instance->get_linear_softness_limit());
				series::pack(node->set("angular-softness-ortho"), instance->get_angular_softness_ortho());
				series::pack(node->set("linear-softness-ortho"), instance->get_linear_softness_ortho());
				series::pack(node->set("powered-angular-motor"), instance->get_powered_angular_motor());
				series::pack(node->set("powered-linear-motor"), instance->get_powered_linear_motor());
				series::pack(node->set("enabled"), instance->is_enabled());
			}
			void slider_constraint::load(entity* other, bool is_ghosted, bool is_linear)
			{
				scene_graph* scene = parent->get_scene();
				VI_ASSERT(scene != nullptr, "scene should be set");
				VI_ASSERT(parent != other, "parent should not be equal to other");

				connection = other;
				if (!connection)
					return;

				rigid_body* first_body = parent->get_component<rigid_body>();
				rigid_body* second_body = connection->get_component<rigid_body>();
				if (!first_body || !second_body)
					return;

				physics::sconstraint::desc i;
				i.target_a = first_body->get_body();
				i.target_b = second_body->get_body();
				i.collisions = !is_ghosted;
				i.linear = is_linear;

				if (i.target_a && i.target_b)
				{
					core::memory::release(instance);
					instance = scene->get_simulator()->create_slider_constraint(i);
				}
			}
			void slider_constraint::clear()
			{
				core::memory::release(instance);
				connection = nullptr;
			}
			component* slider_constraint::copy(entity* init) const
			{
				slider_constraint* target = new slider_constraint(init);
				target->connection = connection;

				if (!instance)
					return target;

				rigid_body* first_body = init->get_component<rigid_body>();
				if (!first_body)
					first_body = parent->get_component<rigid_body>();

				if (!first_body)
					return target;

				physics::sconstraint::desc i(instance->get_state());
				instance->get_state().target_a = first_body->get_body();
				target->instance = (physics::sconstraint*)instance->copy();
				instance->get_state() = i;

				return target;
			}
			physics::sconstraint* slider_constraint::get_constraint() const
			{
				return instance;
			}
			entity* slider_constraint::get_connection() const
			{
				return connection;
			}

			acceleration::acceleration(entity* ref) : component(ref, actor_set::update)
			{
			}
			void acceleration::deserialize(core::schema* node)
			{
				VI_ASSERT(node != nullptr, "schema should be set");

				heavy_series::unpack(node->find("amplitude-velocity"), &amplitude_velocity);
				heavy_series::unpack(node->find("amplitude-torque"), &amplitude_torque);
				heavy_series::unpack(node->find("constant-velocity"), &constant_velocity);
				heavy_series::unpack(node->find("constant-torque"), &constant_torque);
				heavy_series::unpack(node->find("constant-center"), &constant_center);
				series::unpack(node->find("kinematic"), &kinematic);
			}
			void acceleration::serialize(core::schema* node)
			{
				VI_ASSERT(node != nullptr, "schema should be set");

				heavy_series::pack(node->set("amplitude-velocity"), amplitude_velocity);
				heavy_series::pack(node->set("amplitude-torque"), amplitude_torque);
				heavy_series::pack(node->set("constant-velocity"), constant_velocity);
				heavy_series::pack(node->set("constant-torque"), constant_torque);
				heavy_series::pack(node->set("constant-center"), constant_center);
				series::pack(node->set("kinematic"), kinematic);
			}
			void acceleration::activate(component* init)
			{
				if (rigid_body != nullptr)
					return;

				components::rigid_body* component = parent->get_component<components::rigid_body>();
				if (component != nullptr)
					rigid_body = component->get_body();
			}
			void acceleration::update(core::timer* time)
			{
				if (!rigid_body)
					return;

				float step = time->get_step();
				if (kinematic)
				{
					rigid_body->set_linear_velocity(constant_velocity);
					rigid_body->set_angular_velocity(constant_torque);
				}
				else
					rigid_body->push(constant_velocity * step, constant_torque * step, constant_center);

				trigonometry::vector3 force = rigid_body->get_linear_velocity();
				trigonometry::vector3 torque = rigid_body->get_angular_velocity();
				trigonometry::vector3 ACT = constant_torque.abs();
				trigonometry::vector3 ACV = constant_velocity.abs();

				if (amplitude_velocity.x > 0 && force.x > amplitude_velocity.x)
					constant_velocity.x = -ACV.x;
				else if (amplitude_velocity.x > 0 && force.x < -amplitude_velocity.x)
					constant_velocity.x = ACV.x;

				if (amplitude_velocity.y > 0 && force.y > amplitude_velocity.y)
					constant_velocity.y = -ACV.y;
				else if (amplitude_velocity.y > 0 && force.y < -amplitude_velocity.y)
					constant_velocity.y = ACV.y;

				if (amplitude_velocity.z > 0 && force.z > amplitude_velocity.z)
					constant_velocity.z = -ACV.z;
				else if (amplitude_velocity.z > 0 && force.z < -amplitude_velocity.z)
					constant_velocity.z = ACV.z;

				if (amplitude_torque.x > 0 && torque.x > amplitude_torque.x)
					constant_torque.x = -ACT.x;
				else if (amplitude_torque.x > 0 && torque.x < -amplitude_torque.x)
					constant_torque.x = ACT.x;

				if (amplitude_torque.y > 0 && torque.y > amplitude_torque.y)
					constant_torque.y = -ACT.y;
				else if (amplitude_torque.y > 0 && torque.y < -amplitude_torque.y)
					constant_torque.y = ACT.y;

				if (amplitude_torque.z > 0 && torque.z > amplitude_torque.z)
					constant_torque.z = -ACT.z;
				else if (amplitude_torque.z > 0 && torque.z < -amplitude_torque.z)
					constant_torque.z = ACT.z;
			}
			component* acceleration::copy(entity* init) const
			{
				acceleration* target = new acceleration(init);
				target->kinematic = kinematic;
				target->amplitude_torque = amplitude_torque;
				target->amplitude_velocity = amplitude_velocity;
				target->constant_center = constant_center;
				target->constant_torque = constant_torque;
				target->constant_velocity = constant_velocity;
				target->rigid_body = rigid_body;

				return target;
			}
			physics::rigid_body* acceleration::get_body() const
			{
				return rigid_body;
			}

			model::model(entity* ref) : drawable(ref, actor_set::none, model::get_type_id())
			{
			}
			model::~model()
			{
				core::memory::release(instance);
			}
			void model::deserialize(core::schema* node)
			{
				VI_ASSERT(node != nullptr, "schema should be set");

				uint32_t new_category = (uint32_t)geo_category::opaque;
				heavy_series::unpack(node->find("texcoord"), &texcoord);
				series::unpack(node->find("static"), &constant);
				series::unpack(node->find("category"), &new_category);
				set_category((geo_category)new_category);

				core::string path;
				if (!series::unpack(node->find("model"), &path) || path.empty())
					return;
				else
					node->add_ref();

				scene_graph* scene = parent->get_scene();
				material* invalid = scene->get_invalid_material();
				invalid->add_ref();

				set_material(invalid);
				scene->load_resource<layer::model>(this, path, [this, node, scene, invalid](expects_content<layer::model*> new_instance)
				{
					core::memory::release(instance);
					instance = new_instance.or_else(nullptr);
					invalid->release();
					clear_materials();

					if (instance != nullptr)
					{
						for (auto&& material : node->fetch_collection("materials.material"))
						{
							core::string name; size_t slot = 0;
							if (series::unpack(material->find("name"), &name) && series::unpack_a(material->find("slot"), &slot))
							{
								graphics::mesh_buffer* surface = instance->find_mesh(name);
								if (surface != nullptr)
									materials[surface] = scene->get_material(slot);
							}
						}
					}
					node->release();
				});
			}
			void model::serialize(core::schema* node)
			{
				VI_ASSERT(node != nullptr, "schema should be set");

				series::pack(node->set("model"), parent->get_scene()->find_resource_id<layer::model>(instance));
				heavy_series::pack(node->set("texcoord"), texcoord);
				series::pack(node->set("category"), (uint32_t)get_category());
				series::pack(node->set("static"), constant);

				core::schema* slots = node->set("materials", core::var::array());
				for (auto&& slot : materials)
				{
					auto* buffer = (graphics::mesh_buffer*)slot.first;
					if (buffer != nullptr)
					{
						core::schema* material = slots->set("material");
						series::pack(material->set("name"), buffer->name);
						if (slot.second != nullptr)
							series::pack(material->set("slot"), (uint64_t)slot.second->slot);
					}
				}
			}
			void model::set_drawable(layer::model* drawable)
			{
				core::memory::release(instance);
				instance = drawable;
				clear_materials();

				if (!instance)
					return;

				instance->add_ref();
				for (auto* item : instance->meshes)
					materials[(void*)item] = nullptr;
			}
			void model::set_material_for(const std::string_view& name, material* value)
			{
				if (!instance)
					return;

				auto* mesh = instance->find_mesh(name);
				if (mesh != nullptr)
					materials[(void*)mesh] = value;
			}
			float model::get_visibility(const viewer& view, float distance) const
			{
				return instance ? component::get_visibility(view, distance) : 0.0f;
			}
			size_t model::get_unit_bounds(trigonometry::vector3& min, trigonometry::vector3& max) const
			{
				if (!instance)
					return BOX_NONE;

				min = instance->min;
				max = instance->max;
				return BOX_GEOMETRY;
			}
			component* model::copy(entity* init) const
			{
				model* target = new model(init);
				target->set_category(get_category());
				target->instance = instance;
				target->materials = materials;
				target->texcoord = texcoord;

				if (target->instance != nullptr)
					target->instance->add_ref();

				return target;
			}
			layer::model* model::get_drawable()
			{
				return instance;
			}
			material* model::get_material_for(const std::string_view& name)
			{
				if (!instance)
					return nullptr;

				auto* mesh = instance->find_mesh(name);
				if (!mesh)
					return nullptr;

				return materials[(void*)mesh];
			}

			skin::skin(entity* ref) : drawable(ref, actor_set::synchronize, skin::get_type_id())
			{
			}
			skin::~skin()
			{
				core::memory::release(instance);
			}
			void skin::deserialize(core::schema* node)
			{
				VI_ASSERT(node != nullptr, "schema should be set");
				uint32_t new_category = (uint32_t)geo_category::opaque;
				heavy_series::unpack(node->find("texcoord"), &texcoord);
				series::unpack(node->find("static"), &constant);
				series::unpack(node->find("category"), &new_category);
				set_category((geo_category)new_category);

				core::string path;
				if (!series::unpack(node->find("skin-model"), &path) || path.empty())
					return;
				else
					node->add_ref();

				scene_graph* scene = parent->get_scene();
				material* invalid = scene->get_invalid_material();
				invalid->add_ref();

				set_material(invalid);
				scene->load_resource<layer::skin_model>(this, path, [this, node, scene, invalid](expects_content<layer::skin_model*> new_instance)
				{
					core::memory::release(instance);
					instance = new_instance.or_else(nullptr);
					invalid->release();
					clear_materials();

					if (instance != nullptr)
					{
						skeleton.fill(instance);
						for (auto&& material : node->fetch_collection("materials.material"))
						{
							core::string name; size_t slot = 0;
							if (series::unpack(material->find("name"), &name) && series::unpack_a(material->find("slot"), &slot))
							{
								graphics::skin_mesh_buffer* surface = instance->find_mesh(name);
								if (surface != nullptr)
									materials[surface] = scene->get_material(slot);
							}
						}
					}
					node->release();
				});
			}
			void skin::serialize(core::schema* node)
			{
				VI_ASSERT(node != nullptr, "schema should be set");

				series::pack(node->set("skin-model"), parent->get_scene()->find_resource_id<layer::skin_model>(instance));
				heavy_series::pack(node->set("texcoord"), texcoord);
				series::pack(node->set("category"), (uint32_t)get_category());
				series::pack(node->set("static"), constant);

				core::schema* slots = node->set("materials", core::var::array());
				for (auto&& slot : materials)
				{
					if (slot.first != nullptr)
					{
						core::schema* material = slots->set("material");
						series::pack(material->set("name"), ((graphics::mesh_buffer*)slot.first)->name);

						if (slot.second != nullptr)
							series::pack(material->set("slot"), (uint64_t)slot.second->slot);
					}
				}
			}
			void skin::synchronize(core::timer* time)
			{
				if (instance != nullptr)
					instance->synchronize(&skeleton);
			}
			void skin::set_drawable(layer::skin_model* drawable)
			{
				core::memory::release(instance);
				instance = drawable;
				clear_materials();

				if (!instance)
					return;

				instance->add_ref();
				skeleton.fill(instance);
				for (auto* item : instance->meshes)
					materials[(void*)item] = nullptr;
			}
			void skin::set_material_for(const std::string_view& name, material* value)
			{
				if (!instance)
					return;

				auto* mesh = instance->find_mesh(name);
				if (mesh != nullptr)
					materials[(void*)mesh] = value;
			}
			float skin::get_visibility(const viewer& view, float distance) const
			{
				return instance ? component::get_visibility(view, distance) : 0.0f;
			}
			size_t skin::get_unit_bounds(trigonometry::vector3& min, trigonometry::vector3& max) const
			{
				if (!instance)
					return BOX_NONE;

				min = instance->min;
				max = instance->max;
				return BOX_GEOMETRY;
			}
			component* skin::copy(entity* init) const
			{
				skin* target = new skin(init);
				target->set_category(get_category());
				target->instance = instance;
				target->materials = materials;
				target->skeleton = skeleton;

				if (target->instance != nullptr)
					target->instance->add_ref();

				return target;
			}
			layer::skin_model* skin::get_drawable()
			{
				return instance;
			}
			material* skin::get_material_for(const std::string_view& name)
			{
				if (!instance)
					return nullptr;

				auto* mesh = instance->find_mesh(name);
				if (!mesh)
					return nullptr;

				return materials[(void*)mesh];
			}

			emitter::emitter(entity* ref) : drawable(ref, actor_set::none, emitter::get_type_id())
			{
			}
			emitter::~emitter()
			{
				core::memory::release(instance);
			}
			void emitter::deserialize(core::schema* node)
			{
				VI_ASSERT(node != nullptr, "schema should be set");

				scene_graph* scene = parent->get_scene(); size_t slot = 0;
				if (series::unpack_a(node->find("material"), &slot))
					set_material(nullptr, scene->get_material(slot));

				uint32_t new_category = (uint32_t)geo_category::opaque;
				series::unpack(node->find("category"), &new_category);
				series::unpack(node->find("quad-based"), &quad_based);
				series::unpack(node->find("connected"), &connected);
				series::unpack(node->find("static"), &constant);
				heavy_series::unpack(node->find("min"), &min);
				heavy_series::unpack(node->find("max"), &max);
				set_category((geo_category)new_category);

				size_t limit;
				if (series::unpack_a(node->find("limit"), &limit) && instance != nullptr)
				{
					auto& dest = instance->get_array();
					heavy_series::unpack(node->find("elements"), &dest);
					scene->get_device()->update_buffer_size(instance, limit);
				}
			}
			void emitter::serialize(core::schema* node)
			{
				VI_ASSERT(node != nullptr, "schema should be set");

				material* slot = get_material();
				if (slot != nullptr)
					series::pack(node->set("material"), (uint64_t)slot->slot);

				series::pack(node->set("category"), (uint32_t)get_category());
				series::pack(node->set("quad-based"), quad_based);
				series::pack(node->set("connected"), connected);
				series::pack(node->set("static"), constant);
				heavy_series::pack(node->set("min"), min);
				heavy_series::pack(node->set("max"), max);

				if (instance != nullptr)
				{
					series::pack(node->set("limit"), (uint64_t)instance->get_element_limit());
					heavy_series::pack(node->set("elements"), instance->get_array());
				}
			}
			void emitter::activate(component* init)
			{
				if (instance != nullptr)
					return;

				scene_graph* scene = parent->get_scene();
				graphics::instance_buffer::desc i = graphics::instance_buffer::desc();
				i.element_limit = 1 << 10;

				auto buffer = scene->get_device()->create_instance_buffer(i);
				if (buffer)
					instance = *buffer;
			}
			size_t emitter::get_unit_bounds(trigonometry::vector3& _Min, trigonometry::vector3& _Max) const
			{
				if (!instance)
					return BOX_NONE;

				_Min = min;
				_Max = max;
				return BOX_DYNAMIC;
			}
			component* emitter::copy(entity* init) const
			{
				emitter* target = new emitter(init);
				target->set_category(get_category());
				target->connected = connected;
				target->materials = materials;
				target->min = min;
				target->max = max;

				auto& dest = target->instance->get_array();
				dest = instance->get_array();

				return target;
			}
			graphics::instance_buffer* emitter::get_buffer()
			{
				return instance;
			}

			decal::decal(entity* ref) : drawable(ref, actor_set::none, decal::get_type_id())
			{
			}
			void decal::deserialize(core::schema* node)
			{
				VI_ASSERT(node != nullptr, "schema should be set");

				size_t slot = 0;
				if (series::unpack_a(node->find("material"), &slot))
					set_material(nullptr, parent->get_scene()->get_material(slot));

				uint32_t new_category = (uint32_t)geo_category::opaque;
				heavy_series::unpack(node->find("texcoord"), &texcoord);
				series::unpack(node->find("static"), &constant);
				series::unpack(node->find("category"), &new_category);
				set_category((geo_category)new_category);
			}
			void decal::serialize(core::schema* node)
			{
				VI_ASSERT(node != nullptr, "schema should be set");

				material* slot = get_material();
				if (slot != nullptr)
					series::pack(node->set("material"), (uint64_t)slot->slot);

				heavy_series::pack(node->set("texcoord"), texcoord);
				series::pack(node->set("static"), constant);
				series::pack(node->set("category"), (uint32_t)get_category());
			}
			float decal::get_visibility(const viewer& view, float distance) const
			{
				return get_visibility_radius(parent, view, distance);
			}
			component* decal::copy(entity* init) const
			{
				decal* target = new decal(init);
				target->texcoord = texcoord;
				target->materials = materials;

				return target;
			}

			skin_animator::skin_animator(entity* ref) : component(ref, actor_set::animate)
			{
			}
			skin_animator::~skin_animator() noexcept
			{
				core::memory::release(animation);
			}
			void skin_animator::deserialize(core::schema* node)
			{
				VI_ASSERT(node != nullptr, "schema should be set");
				heavy_series::unpack(node->find("state"), &state);

				core::string path;
				if (!series::unpack(node->find("path"), &path))
					return;

				parent->get_scene()->load_resource<layer::skin_animation>(this, path, [this](expects_content<layer::skin_animation*> result)
				{
					animation = result.or_else(nullptr);
				});
			}
			void skin_animator::serialize(core::schema* node)
			{
				VI_ASSERT(node != nullptr, "schema should be set");
				heavy_series::pack(node->set("state"), state);
				series::pack(node->set("path"), parent->get_scene()->find_resource_id<skin_animation>(animation));
			}
			void skin_animator::activate(component* init)
			{
				components::skin* base = parent->get_component<components::skin>();
				if (base != nullptr && base->get_drawable() != nullptr)
				{
					instance = base;
					set_active(true);
				}
				else
				{
					instance = nullptr;
					set_active(false);
				}
			}
			void skin_animator::animate(core::timer* time)
			{
				if (!animation || state.paused)
					return;

				auto& clips = animation->get_clips();
				if (!state.blended)
				{
					if (state.clip < 0 || state.clip >= (int64_t)clips.size() || state.frame < 0 || state.frame >= (int64_t)clips[(size_t)state.clip].keys.size())
						return;

					auto& clip = clips[(size_t)state.clip];
					auto& next_key = clip.keys[(size_t)state.frame + 1 >= clip.keys.size() ? 0 : (size_t)state.frame + 1];
					auto& prev_key = clip.keys[(size_t)state.frame];
					state.duration = clip.duration;
					state.rate = clip.rate;
					state.time = state.get_timeline(time);

					float t = state.get_progress();
					for (auto&& pose : instance->skeleton.offsets)
					{
						auto& prev = prev_key.pose[pose.first];
						auto& next = next_key.pose[pose.first];
						pose.second.frame.position = prev.position.lerp(next.position, t);
						pose.second.frame.scale = prev.scale.lerp(next.scale, t);
						pose.second.frame.rotation = prev.rotation.slerp(next.rotation, t);
						pose.second.offset = pose.second.frame;
					}

					if (state.get_progress_total() >= 1.0f)
					{
						if (state.frame + 1 >= (int64_t)clip.keys.size())
						{
							if (!state.looped)
								blend_animation(-1, -1);
							state.frame = -1;
						}

						state.time = 0.0f;
						if (state.looped || state.frame != -1)
							state.frame++;
					}
				}
				else if (state.get_progress_total() < 1.0f)
				{
					auto* keys = (is_exists(state.clip, state.frame) ? get_frame(state.clip, state.frame) : nullptr);
					state.time = state.get_timeline(time);
					float t = state.get_progress();

					for (auto&& pose : instance->skeleton.offsets)
					{
						if (keys != nullptr)
						{
							auto& next = keys->pose[pose.first];
							pose.second.offset.position = pose.second.frame.position.lerp(next.position, t);
							pose.second.offset.scale = pose.second.frame.scale.lerp(next.scale, t);
							pose.second.offset.rotation = pose.second.frame.rotation.slerp(next.rotation, t);
						}
						else
						{
							pose.second.offset.position = pose.second.frame.position.lerp(pose.second.defaults.position, t);
							pose.second.offset.scale = pose.second.frame.scale.lerp(pose.second.defaults.scale, t);
							pose.second.offset.rotation = pose.second.frame.rotation.slerp(pose.second.defaults.rotation, t);
						}
					}
				}
				else
				{
					state.blended = false;
					state.time = 0.0f;
				}
			}
			void skin_animator::set_animation(skin_animation* init)
			{
				core::memory::release(animation);
				animation = init;
				if (animation != nullptr)
					animation->add_ref();
			}
			void skin_animator::blend_animation(int64_t clip, int64_t frame)
			{
				state.blended = true;
				state.time = 0.0f;
				state.frame = frame;
				state.clip = clip;

				if (animation != nullptr)
				{
					auto& clips = animation->get_clips();
					if (state.clip >= 0 && (size_t)state.clip < clips.size())
					{
						auto& next = clips[(size_t)state.clip];
						if (state.frame < 0 || (size_t)state.frame >= next.keys.size())
							state.frame = -1;

						if (state.duration <= 0.0f)
						{
							state.duration = next.duration;
							state.rate = next.rate;
						}
					}
					else
						state.clip = -1;
				}
				else
					state.clip = -1;
			}
			void skin_animator::save_binding_state()
			{
				for (auto&& pose : instance->skeleton.offsets)
				{
					pose.second.frame = pose.second.defaults;
					pose.second.offset = pose.second.frame;
				}
			}
			void skin_animator::stop()
			{
				state.paused = false;
				blend_animation(-1, -1);
			}
			void skin_animator::pause()
			{
				state.paused = true;
			}
			void skin_animator::play(int64_t clip, int64_t frame)
			{
				if (state.paused)
				{
					state.paused = false;
					return;
				}

				state.time = 0.0f;
				state.frame = (frame == -1 ? 0 : frame);
				state.clip = (clip == -1 ? 0 : clip);

				if (!is_exists(state.clip, state.frame))
					return;

				if (animation != nullptr)
				{
					auto& clips = animation->get_clips();
					if (state.clip >= 0 && (size_t)state.clip < clips.size())
					{
						auto& next = clips[(size_t)state.clip];
						if (state.frame < 0 || (size_t)state.frame >= next.keys.size())
							state.frame = -1;
					}
					else
						state.clip = -1;
				}
				else
					state.clip = -1;

				save_binding_state();
				if (state.get_progress() < 1.0f)
					blend_animation(state.clip, state.frame);
			}
			bool skin_animator::is_exists(int64_t clip)
			{
				if (!animation)
					return false;

				auto& clips = animation->get_clips();
				return clip >= 0 && (size_t)clip < clips.size();
			}
			bool skin_animator::is_exists(int64_t clip, int64_t frame)
			{
				if (!is_exists(clip))
					return false;

				auto& clips = animation->get_clips();
				return frame >= 0 && (size_t)frame < clips[(size_t)clip].keys.size();
			}
			const trigonometry::skin_animator_key* skin_animator::get_frame(int64_t clip, int64_t frame)
			{
				VI_ASSERT(animation != nullptr, "animation should be set");
				auto& clips = animation->get_clips();

				VI_ASSERT(clip >= 0 && (size_t)clip < clips.size(), "clip index outside of range");
				VI_ASSERT(frame >= 0 && (size_t)frame < clips[(size_t)clip].keys.size(), "frame index outside of range");
				return &clips[(size_t)clip].keys[(size_t)frame];
			}
			const core::vector<trigonometry::skin_animator_key>* skin_animator::get_clip(int64_t clip)
			{
				VI_ASSERT(animation != nullptr, "animation should be set");
				auto& clips = animation->get_clips();

				VI_ASSERT(clip >= 0 && (size_t)clip < clips.size(), "clip index outside of range");
				return &clips[(size_t)clip].keys;
			}
			core::string skin_animator::get_path() const
			{
				return parent->get_scene()->find_resource_id<skin_animation>(animation);
			}
			component* skin_animator::copy(entity* init) const
			{
				skin_animator* target = new skin_animator(init);
				target->animation = animation;
				target->state = state;

				if (animation != nullptr)
					animation->add_ref();

				return target;
			}
			int64_t skin_animator::get_clip_by_name(const std::string_view& name) const
			{
				if (!animation)
					return -1;

				size_t index = 0;
				for (auto& item : animation->get_clips())
				{
					if (item.name == name)
						return (int64_t)index;
					index++;
				}

				return -1;
			}
			size_t skin_animator::get_clips_count() const
			{
				if (!animation)
					return 0;

				return animation->get_clips().size();
			}
			skin_animation* skin_animator::get_animation() const
			{
				return animation;
			}
			skin* skin_animator::get_skin() const
			{
				return instance;
			}

			key_animator::key_animator(entity* ref) : component(ref, actor_set::animate)
			{
			}
			key_animator::~key_animator()
			{
			}
			void key_animator::deserialize(core::schema* node)
			{
				VI_ASSERT(node != nullptr, "schema should be set");
				heavy_series::unpack(node->find("state"), &state);
				heavy_series::unpack(node->find("offset"), &offset);
				heavy_series::unpack(node->find("default"), &defaults);

				core::string path;
				if (!series::unpack(node->find("path"), &path) || path.empty())
					heavy_series::unpack(node->find("animation"), &clips);
				else
					load_animation(path, nullptr);
			}
			void key_animator::serialize(core::schema* node)
			{
				VI_ASSERT(node != nullptr, "schema should be set");
				heavy_series::pack(node->set("state"), state);
				heavy_series::pack(node->set("offset"), offset);
				heavy_series::pack(node->set("default"), defaults);

				if (reference.empty())
					heavy_series::pack(node->set("animation"), clips);
				else
					series::pack(node->set("path"), reference);
			}
			void key_animator::animate(core::timer* time)
			{
				auto* transform = parent->get_transform();
				if (state.paused)
					return;

				if (!state.blended)
				{
					if (state.clip < 0 || (size_t)state.clip >= clips.size() || state.frame < 0 || (size_t)state.frame >= clips[(size_t)state.clip].keys.size())
						return;

					auto& clip = clips[(size_t)state.clip];
					auto& next_key = clip.keys[(size_t)state.frame + 1 >= clip.keys.size() ? 0 : (size_t)state.frame + 1];
					auto& prev_key = clip.keys[(size_t)state.frame];
					state.duration = clip.duration;
					state.rate = clip.rate * next_key.time;
					state.time = state.get_timeline(time);

					float t = state.get_progress();
					offset.position = prev_key.position.lerp(next_key.position, t);
					offset.rotation = prev_key.rotation.lerp(next_key.rotation, t);
					offset.scale = prev_key.scale.lerp(next_key.scale, t);

					transform->set_position(offset.position);
					transform->set_rotation(offset.rotation.get_euler());
					transform->set_scale(offset.scale);

					if (state.get_progress_total() >= 1.0f)
					{
						if (state.frame + 1 >= (int64_t)clip.keys.size())
						{
							if (!state.looped)
								blend_animation(-1, -1);
							state.frame = -1;
						}

						state.time = 0.0f;
						if (state.looped || state.frame != -1)
							state.frame++;
					}
				}
				else if (state.get_progress_total() < 1.0f)
				{
					auto* next = (is_exists(state.clip, state.frame) ? get_frame(state.clip, state.frame) : nullptr);
					if (!next)
						next = &defaults;

					state.time = state.get_timeline(time);
					float t = state.get_progress();
					transform->set_position(offset.position.lerp(next->position, t));
					transform->set_rotation(offset.rotation.lerp(next->rotation, t).get_euler());
					transform->set_scale(offset.scale.lerp(next->scale, t));
				}
				else
				{
					state.blended = false;
					state.time = 0.0f;
				}
			}
			void key_animator::load_animation(const std::string_view& path, std::function<void(bool)>&& callback)
			{
				auto* scene = parent->get_scene();
				auto copy = core::string(path);
				scene->load_resource<core::schema>(this, path, [this, scene, copy, callback = std::move(callback)](expects_content<core::schema*> result)
				{
					clear_animation();
					if (result && *result != nullptr)
					{
						if (heavy_series::unpack(*result, &clips))
							reference = scene->as_resource_path(copy);
						else
							reference.clear();
						core::memory::release(*result);
					}
					else
						reference.clear();

					if (callback)
						callback(!reference.empty());
				});
			}
			void key_animator::clear_animation()
			{
				reference.clear();
				clips.clear();
			}
			void key_animator::blend_animation(int64_t clip, int64_t frame)
			{
				state.blended = true;
				state.time = 0.0f;
				state.frame = frame;
				state.clip = clip;

				if (state.clip >= 0 && (size_t)state.clip < clips.size())
				{
					auto& next = clips[(size_t)state.clip];
					if (state.frame < 0 || (size_t)state.frame >= next.keys.size())
						state.frame = -1;
				}
				else
					state.clip = -1;
			}
			void key_animator::save_binding_state()
			{
				auto& space = parent->get_transform()->get_spacing();
				defaults.position = space.position;
				defaults.rotation = space.rotation;
				defaults.scale = space.scale;
				offset = defaults;
			}
			void key_animator::stop()
			{
				state.paused = false;
				blend_animation(-1, -1);
			}
			void key_animator::pause()
			{
				state.paused = true;
			}
			void key_animator::play(int64_t clip, int64_t frame)
			{
				if (state.paused)
				{
					state.paused = false;
					return;
				}

				state.time = 0.0f;
				state.frame = (frame == -1 ? 0 : frame);
				state.clip = (clip == -1 ? 0 : clip);

				if (!is_exists(state.clip, state.frame))
					return;

				if (state.clip >= 0 && (size_t)state.clip < clips.size())
				{
					auto& next = clips[(size_t)state.clip];
					if (state.frame < 0 || (size_t)state.frame >= next.keys.size())
						state.frame = -1;

					if (state.duration <= 0.0f)
					{
						state.duration = next.duration;
						state.rate = next.rate;
					}
				}
				else
					state.clip = -1;

				save_binding_state();
				if (state.get_progress() < 1.0f)
					blend_animation(state.clip, state.frame);
			}
			bool key_animator::is_exists(int64_t clip)
			{
				return clip >= 0 && (size_t)clip < clips.size();
			}
			bool key_animator::is_exists(int64_t clip, int64_t frame)
			{
				if (!is_exists(clip))
					return false;

				return frame >= 0 && (size_t)frame < clips[(size_t)clip].keys.size();
			}
			trigonometry::animator_key* key_animator::get_frame(int64_t clip, int64_t frame)
			{
				VI_ASSERT(clip >= 0 && (size_t)clip < clips.size(), "clip index outside of range");
				VI_ASSERT(frame >= 0 && (size_t)frame < clips[(size_t)clip].keys.size(), "frame index outside of range");
				return &clips[(size_t)clip].keys[(size_t)frame];
			}
			core::vector<trigonometry::animator_key>* key_animator::get_clip(int64_t clip)
			{
				VI_ASSERT(clip >= 0 && (size_t)clip < clips.size(), "clip index outside of range");
				return &clips[(size_t)clip].keys;
			}
			core::string key_animator::get_path()
			{
				return reference;
			}
			component* key_animator::copy(entity* init) const
			{
				key_animator* target = new key_animator(init);
				target->clips = clips;
				target->state = state;
				target->defaults = defaults;
				target->offset = offset;

				return target;
			}

			emitter_animator::emitter_animator(entity* ref) : component(ref, actor_set::animate)
			{
				spawner.scale.max = 1;
				spawner.scale.min = 1;
				spawner.rotation.max = 0;
				spawner.rotation.min = 0;
				spawner.angular.max = 0;
				spawner.angular.min = 0;
				spawner.diffusion.min = 1;
				spawner.diffusion.max = 1;
				spawner.velocity.min = 0;
				spawner.velocity.max = 0;
				spawner.position.min = -1;
				spawner.position.max = 1;
				spawner.noise.min = -1;
				spawner.noise.max = 1;
				spawner.iterations = 1;
			}
			void emitter_animator::deserialize(core::schema* node)
			{
				VI_ASSERT(node != nullptr, "schema should be set");

				heavy_series::unpack(node->find("diffuse"), &diffuse);
				heavy_series::unpack(node->find("position"), &position);
				heavy_series::unpack(node->find("velocity"), &velocity);
				heavy_series::unpack(node->find("spawner"), &spawner);
				series::unpack(node->find("noise"), &noise);
				series::unpack(node->find("rotation-speed"), &rotation_speed);
				series::unpack(node->find("scale-speed"), &scale_speed);
				series::unpack(node->find("simulate"), &simulate);
			}
			void emitter_animator::serialize(core::schema* node)
			{
				VI_ASSERT(node != nullptr, "schema should be set");

				heavy_series::pack(node->set("diffuse"), diffuse);
				heavy_series::pack(node->set("position"), position);
				heavy_series::pack(node->set("velocity"), velocity);
				heavy_series::pack(node->set("spawner"), spawner);
				series::pack(node->set("noise"), noise);
				series::pack(node->set("rotation-speed"), rotation_speed);
				series::pack(node->set("scale-speed"), scale_speed);
				series::pack(node->set("simulate"), simulate);
			}
			void emitter_animator::activate(component* init)
			{
				base = parent->get_component<emitter>();
				set_active(base != nullptr);
			}
			void emitter_animator::animate(core::timer* time)
			{
				if (!simulate || !base || !base->get_buffer())
					return;

				auto* transform = parent->get_transform();
				auto& array = base->get_buffer()->get_array();
				trigonometry::vector3 offset = transform->get_position();

				for (int i = 0; i < spawner.iterations; i++)
				{
					if (array.size() + 1 >= array.capacity())
						break;

					trigonometry::vector3 fposition = (base->connected ? spawner.position.generate() : spawner.position.generate() + offset);
					trigonometry::vector3 fvelocity = spawner.velocity.generate();
					trigonometry::vector4 fdiffusion = spawner.diffusion.generate();

					trigonometry::element_vertex element;
					element.position_x = fposition.x;
					element.position_y = fposition.y;
					element.position_z = fposition.z;
					element.velocity_x = fvelocity.x;
					element.velocity_y = fvelocity.y;
					element.velocity_z = fvelocity.z;
					element.color_x = fdiffusion.x;
					element.color_y = fdiffusion.y;
					element.color_z = fdiffusion.z;
					element.color_w = fdiffusion.w;
					element.angular = spawner.angular.generate();
					element.rotation = spawner.rotation.generate();
					element.scale = spawner.scale.generate();
					array.emplace_back(std::move(element));
				}

				float step = time->get_step();
				if (noise != 0.0f)
					accurate_synchronization(step);
				else
					fast_synchronization(step);
				transform->make_dirty();
			}
			void emitter_animator::accurate_synchronization(float step)
			{
				auto& array = base->get_buffer()->get_array();
				float min_x = 0.0f, max_x = 0.0f;
				float min_y = 0.0f, max_y = 0.0f;
				float min_z = 0.0f, max_z = 0.0f;
				auto begin = array.begin();
				auto end = array.end();

				for (auto it = begin; it != end;)
				{
					trigonometry::vector3 next_velocity(it->velocity_x, it->velocity_y, it->velocity_z);
					trigonometry::vector3 next_noise = spawner.noise.generate() / noise;
					trigonometry::vector3 next_position(it->position_x, it->position_y, it->position_z);
					trigonometry::vector4 next_diffuse(it->color_x, it->color_y, it->color_z, it->color_w);
					next_velocity -= (next_velocity * velocity) * step;
					next_position += (next_velocity + position + next_noise) * step;
					next_diffuse += diffuse * step;
					memcpy(&it->velocity_x, &next_velocity, sizeof(float) * 3);
					memcpy(&it->position_x, &next_position, sizeof(float) * 3);
					memcpy(&it->color_x, &next_diffuse, sizeof(float) * 4);
					it->rotation += (it->angular + rotation_speed) * step;
					it->scale += scale_speed * step;

					if (it->color_w > 0.001f && it->scale > 0.001f)
					{
						if (it->position_x < min_x)
							min_x = it->position_x;
						else if (it->position_x > max_x)
							max_x = it->position_x;

						if (it->position_y < min_y)
							min_y = it->position_y;
						else if (it->position_y > max_y)
							max_y = it->position_y;

						if (it->position_z < min_z)
							min_z = it->position_z;
						else if (it->position_z > max_z)
							max_z = it->position_z;
						++it;
					}
					else
					{
						it = array.erase(it);
						begin = array.begin();
						end = array.end();
					}
				}

				base->min = trigonometry::vector3(min_x, min_y, min_z);
				base->max = trigonometry::vector3(max_x, max_y, max_z);
			}
			void emitter_animator::fast_synchronization(float step)
			{
				auto& array = base->get_buffer()->get_array();
				float min_x = 0.0f, max_x = 0.0f;
				float min_y = 0.0f, max_y = 0.0f;
				float min_z = 0.0f, max_z = 0.0f;
				auto begin = array.begin();
				auto end = array.end();

				for (auto it = begin; it != end;)
				{
					trigonometry::vector3 next_velocity(it->velocity_x, it->velocity_y, it->velocity_z);
					trigonometry::vector3 next_position(it->position_x, it->position_y, it->position_z);
					trigonometry::vector4 next_diffuse(it->color_x, it->color_y, it->color_z, it->color_w);
					next_velocity -= (next_velocity * velocity) * step;
					next_position += (next_velocity + position) * step;
					next_diffuse += diffuse * step;
					memcpy(&it->velocity_x, &next_velocity, sizeof(float) * 3);
					memcpy(&it->position_x, &next_position, sizeof(float) * 3);
					memcpy(&it->color_x, &next_diffuse, sizeof(float) * 4);
					it->rotation += (it->angular + rotation_speed) * step;
					it->scale += scale_speed * step;

					if (it->color_w > 0.001f && it->scale > 0.001f)
					{
						if (it->position_x < min_x)
							min_x = it->position_x;
						else if (it->position_x > max_x)
							max_x = it->position_x;

						if (it->position_y < min_y)
							min_y = it->position_y;
						else if (it->position_y > max_y)
							max_y = it->position_y;

						if (it->position_z < min_z)
							min_z = it->position_z;
						else if (it->position_z > max_z)
							max_z = it->position_z;
						++it;
					}
					else
					{
						it = array.erase(it);
						begin = array.begin();
						end = array.end();
					}
				}

				base->min = trigonometry::vector3(min_x, min_y, min_z);
				base->max = trigonometry::vector3(max_x, max_y, max_z);
			}
			component* emitter_animator::copy(entity* init) const
			{
				emitter_animator* target = new emitter_animator(init);
				target->diffuse = diffuse;
				target->position = position;
				target->velocity = velocity;
				target->scale_speed = scale_speed;
				target->rotation_speed = rotation_speed;
				target->spawner = spawner;
				target->noise = noise;
				target->simulate = simulate;

				return target;
			}
			emitter* emitter_animator::get_emitter() const
			{
				return base;
			}

			free_look::free_look(entity* ref) : component(ref, actor_set::update), rotate(graphics::key_code::cursor_right), sensivity(0.005f)
			{
			}
			void free_look::update(core::timer* time)
			{
				auto* activity = parent->get_scene()->get_activity();
				if (!activity)
					return;

				trigonometry::vector2 cursor = activity->get_global_cursor_position();
				if (!activity->is_key_down(rotate))
				{
					position = cursor;
					return;
				}

				if (!activity->is_key_down_hit(rotate))
				{
					auto* transform = parent->get_transform();
					trigonometry::vector2 next = (cursor - position) * sensivity;
					transform->rotate(trigonometry::vector3(next.y, next.x) * direction);

					const trigonometry::vector3& rotation = transform->get_rotation();
					transform->set_rotation(rotation.set_x(compute::mathf::clamp(rotation.x, -1.57079632679f, 1.57079632679f)));
				}
				else
					position = cursor;

				if ((int)cursor.x != (int)position.x || (int)cursor.y != (int)position.y)
					activity->set_global_cursor_position(position);
			}
			component* free_look::copy(entity* init) const
			{
				free_look* target = new free_look(init);
				target->position = position;
				target->rotate = rotate;
				target->sensivity = sensivity;

				return target;
			}

			fly::fly(entity* ref) : component(ref, actor_set::update)
			{
			}
			void fly::update(core::timer* time)
			{
				auto* activity = parent->get_scene()->get_activity();
				if (!activity)
					return;

				auto* transform = parent->get_transform();
				trigonometry::vector3 new_velocity;

				if (activity->is_key_down(forward))
					new_velocity += transform->forward().view_space();
				else if (activity->is_key_down(backward))
					new_velocity -= transform->forward().view_space();

				if (activity->is_key_down(right))
					new_velocity += transform->right().view_space();
				else if (activity->is_key_down(left))
					new_velocity -= transform->right().view_space();

				if (activity->is_key_down(up))
					new_velocity += transform->up().view_space();
				else if (activity->is_key_down(down))
					new_velocity -= transform->up().view_space();

				float step = time->get_step();
				if (new_velocity.length() > 0.001f)
					velocity += get_speed(activity) * new_velocity * step;

				if (velocity.length() > 0.001f)
				{
					transform->move(velocity * step);
					velocity = velocity.lerp(trigonometry::vector3::zero(), moving.fading * step);
				}
				else
					velocity = trigonometry::vector3::zero();
			}
			component* fly::copy(entity* init) const
			{
				fly* target = new fly(init);
				target->moving = moving;
				target->velocity = velocity;
				target->forward = forward;
				target->backward = backward;
				target->right = right;
				target->left = left;
				target->up = up;
				target->down = down;
				target->fast = fast;
				target->slow = slow;

				return target;
			}
			trigonometry::vector3 fly::get_speed(graphics::activity* activity)
			{
				if (activity->is_key_down(fast))
					return moving.axis * moving.faster;

				if (activity->is_key_down(slow))
					return moving.axis * moving.slower;

				return moving.axis * moving.normal;
			}

			audio_source::audio_source(entity* ref) : component(ref, actor_set::synchronize)
			{
				source = new audio::audio_source();
			}
			audio_source::~audio_source()
			{
				core::memory::release(source);
			}
			void audio_source::deserialize(core::schema* node)
			{
				VI_ASSERT(node != nullptr, "schema should be set");

				heavy_series::unpack(node->find("velocity"), &sync.velocity);
				heavy_series::unpack(node->find("direction"), &sync.direction);
				series::unpack(node->find("rolloff"), &sync.rolloff);
				series::unpack(node->find("cone-inner-angle"), &sync.cone_inner_angle);
				series::unpack(node->find("cone-outer-angle"), &sync.cone_outer_angle);
				series::unpack(node->find("cone-outer-gain"), &sync.cone_outer_gain);
				series::unpack(node->find("distance"), &sync.distance);
				series::unpack(node->find("gain"), &sync.gain);
				series::unpack(node->find("pitch"), &sync.pitch);
				series::unpack(node->find("ref-distance"), &sync.ref_distance);
				series::unpack(node->find("position"), &sync.position);
				series::unpack(node->find("relative"), &sync.is_relative);
				series::unpack(node->find("looped"), &sync.is_looped);
				series::unpack(node->find("distance"), &sync.distance);
				series::unpack(node->find("air-absorption"), &sync.air_absorption);
				series::unpack(node->find("room-roll-off"), &sync.room_roll_off);

				core::string path;
				if (!series::unpack(node->find("audio-clip"), &path) || path.empty())
					return;

				node->add_ref();
				parent->get_scene()->load_resource<audio::audio_clip>(this, path, [this, node](expects_content<audio::audio_clip*>&& new_clip)
				{
					source->set_clip(new_clip.or_else(nullptr));
					for (auto& effect : node->fetch_collection("effects.effect"))
					{
						uint64_t id;
						if (!series::unpack(effect->find("id"), &id))
							continue;

						audio::audio_effect* target = core::composer::create<audio::audio_effect>(id);
						if (!target)
							continue;

						core::schema* meta = effect->find("metadata");
						if (!meta)
							meta = effect->set("metadata");

						target->deserialize(meta);
						source->add_effect(target);
					}

					bool autoplay;
					if (series::unpack(node->find("autoplay"), &autoplay) && autoplay && source->get_clip())
						source->play();

					apply_playing_position();
					synchronize(nullptr);
					node->release();
				});
			}
			void audio_source::serialize(core::schema* node)
			{
				VI_ASSERT(node != nullptr, "schema should be set");

				core::schema* effects = node->set("effects", core::var::array());
				for (auto* effect : source->get_effects())
				{
					core::schema* element = effects->set("effect");
					series::pack(element->set("id"), effect->get_id());
					effect->serialize(element->set("metadata"));
				}

				series::pack(node->set("audio-clip"), parent->get_scene()->find_resource_id<audio::audio_clip>(source->get_clip()));
				heavy_series::pack(node->set("velocity"), sync.velocity);
				heavy_series::pack(node->set("direction"), sync.direction);
				series::pack(node->set("rolloff"), sync.rolloff);
				series::pack(node->set("cone-inner-angle"), sync.cone_inner_angle);
				series::pack(node->set("cone-outer-angle"), sync.cone_outer_angle);
				series::pack(node->set("cone-outer-gain"), sync.cone_outer_gain);
				series::pack(node->set("distance"), sync.distance);
				series::pack(node->set("gain"), sync.gain);
				series::pack(node->set("pitch"), sync.pitch);
				series::pack(node->set("ref-distance"), sync.ref_distance);
				series::pack(node->set("position"), sync.position);
				series::pack(node->set("relative"), sync.is_relative);
				series::pack(node->set("looped"), sync.is_looped);
				series::pack(node->set("distance"), sync.distance);
				series::pack(node->set("autoplay"), source->is_playing());
				series::pack(node->set("air-absorption"), sync.air_absorption);
				series::pack(node->set("room-roll-off"), sync.room_roll_off);
			}
			void audio_source::synchronize(core::timer* time)
			{
				auto* transform = parent->get_transform();
				if (transform->is_dirty())
				{
					const trigonometry::vector3& position = transform->get_position();
					sync.velocity = (position - last_position) * time->get_step();
					last_position = position;
				}
				else
					sync.velocity = 0.0f;

				if (source->get_clip() != nullptr)
					source->synchronize(&sync, transform->get_position());
			}
			void audio_source::apply_playing_position()
			{
				audio::audio_context::set_source_data1f(source->get_instance(), audio::sound_ex::seconds_offset, sync.position);
			}
			component* audio_source::copy(entity* init) const
			{
				audio_source* target = new audio_source(init);
				target->last_position = last_position;
				target->source->set_clip(source->get_clip());
				target->sync = sync;

				for (auto* effect : source->get_effects())
					target->source->add_effect(effect->copy());

				return target;
			}
			audio::audio_source* audio_source::get_source() const
			{
				return source;
			}
			audio::audio_sync& audio_source::get_sync()
			{
				return sync;
			}

			audio_listener::audio_listener(entity* ref) : component(ref, actor_set::synchronize)
			{
			}
			audio_listener::~audio_listener()
			{
				deactivate();
			}
			void audio_listener::deserialize(core::schema* node)
			{
				VI_ASSERT(node != nullptr, "schema should be set");
				series::unpack(node->find("gain"), &gain);
			}
			void audio_listener::serialize(core::schema* node)
			{
				VI_ASSERT(node != nullptr, "schema should be set");
				series::pack(node->set("gain"), gain);
			}
			void audio_listener::synchronize(core::timer* time)
			{
				auto* transform = parent->get_transform();
				if (transform->is_dirty())
				{
					const trigonometry::vector3& position = transform->get_position();
					trigonometry::vector3 velocity = (position - last_position) * time->get_step();
					trigonometry::vector3 rotation = transform->get_rotation().ddirection();
					float look_at[6] = { rotation.x, rotation.y, rotation.z, 0.0f, 1.0f, 0.0f };
					last_position = position;

					audio::audio_context::set_listener_data3f(audio::sound_ex::velocity, velocity.x, velocity.y, velocity.z);
					audio::audio_context::set_listener_data3f(audio::sound_ex::position, -position.x, -position.y, position.z);
					audio::audio_context::set_listener_datavf(audio::sound_ex::orientation, look_at);
				}

				audio::audio_context::set_listener_data1f(audio::sound_ex::gain, gain);
			}
			void audio_listener::deactivate()
			{
				float look_at[6] = { 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f };
				audio::audio_context::set_listener_data3f(audio::sound_ex::velocity, 0.0f, 0.0f, 0.0f);
				audio::audio_context::set_listener_data3f(audio::sound_ex::position, 0.0f, 0.0f, 0.0f);
				audio::audio_context::set_listener_datavf(audio::sound_ex::orientation, look_at);
				audio::audio_context::set_listener_data1f(audio::sound_ex::gain, 0.0f);
			}
			component* audio_listener::copy(entity* init) const
			{
				audio_listener* target = new audio_listener(init);
				target->last_position = last_position;
				target->gain = gain;

				return target;
			}

			point_light::point_light(entity* ref) : component(ref, actor_set::cullable)
			{
			}
			void point_light::deserialize(core::schema* node)
			{
				VI_ASSERT(node != nullptr, "schema should be set");
				heavy_series::unpack(node->find("projection"), &projection);
				heavy_series::unpack(node->find("view"), &view);
				heavy_series::unpack(node->find("size"), &size);
				heavy_series::unpack(node->find("diffuse"), &diffuse);
				series::unpack(node->find("emission"), &emission);
				series::unpack(node->find("disperse"), &disperse);
				series::unpack(node->find("shadow-softness"), &shadow.softness);
				series::unpack(node->find("shadow-distance"), &shadow.distance);
				series::unpack(node->find("shadow-bias"), &shadow.bias);
				series::unpack(node->find("shadow-iterations"), &shadow.iterations);
				series::unpack(node->find("shadow-enabled"), &shadow.enabled);
			}
			void point_light::serialize(core::schema* node)
			{
				VI_ASSERT(node != nullptr, "schema should be set");
				heavy_series::pack(node->set("projection"), projection);
				heavy_series::pack(node->set("view"), view);
				heavy_series::pack(node->set("size"), size);
				heavy_series::pack(node->set("diffuse"), diffuse);
				series::pack(node->set("emission"), emission);
				series::pack(node->set("disperse"), disperse);
				series::pack(node->set("shadow-softness"), shadow.softness);
				series::pack(node->set("shadow-distance"), shadow.distance);
				series::pack(node->set("shadow-bias"), shadow.bias);
				series::pack(node->set("shadow-iterations"), shadow.iterations);
				series::pack(node->set("shadow-enabled"), shadow.enabled);
			}
			void point_light::message(const std::string_view& name, core::variant_args& args)
			{
				if (name == "depth-flush")
					depth_map = nullptr;
			}
			size_t point_light::get_unit_bounds(trigonometry::vector3& min, trigonometry::vector3& max) const
			{
				min = size.radius * -1.25f;
				max = size.radius * 1.25f;
				return BOX_LIGHT;
			}
			float point_light::get_visibility(const viewer& view, float distance) const
			{
				return get_visibility_radius(parent, view, distance);
			}
			component* point_light::copy(entity* init) const
			{
				point_light* target = new point_light(init);
				target->diffuse = diffuse;
				target->emission = emission;
				target->projection = projection;
				target->view = view;
				target->size = size;
				memcpy(&target->shadow, &shadow, sizeof(shadow));

				return target;
			}
			void point_light::generate_origin()
			{
				auto* transform = parent->get_transform();
				view = trigonometry::matrix4x4::create_translation(transform->get_position());
				projection = trigonometry::matrix4x4::create_perspective(90.0f, 1.0f, 0.1f, shadow.distance);
			}
			void point_light::set_size(const attenuation& value)
			{
				size = value;
				get_entity()->get_transform()->make_dirty();
			}
			const attenuation& point_light::get_size()
			{
				return size;
			}

			spot_light::spot_light(entity* ref) : component(ref, actor_set::cullable | actor_set::synchronize)
			{
			}
			void spot_light::deserialize(core::schema* node)
			{
				VI_ASSERT(node != nullptr, "schema should be set");
				heavy_series::unpack(node->find("projection"), &projection);
				heavy_series::unpack(node->find("view"), &view);
				heavy_series::unpack(node->find("size"), &size);
				heavy_series::unpack(node->find("diffuse"), &diffuse);
				series::unpack(node->find("emission"), &emission);
				series::unpack(node->find("disperse"), &disperse);
				series::unpack(node->find("cutoff"), &cutoff);
				series::unpack(node->find("shadow-bias"), &shadow.bias);
				series::unpack(node->find("shadow-distance"), &shadow.distance);
				series::unpack(node->find("shadow-softness"), &shadow.softness);
				series::unpack(node->find("shadow-iterations"), &shadow.iterations);
				series::unpack(node->find("shadow-enabled"), &shadow.enabled);
			}
			void spot_light::serialize(core::schema* node)
			{
				VI_ASSERT(node != nullptr, "schema should be set");
				heavy_series::pack(node->set("projection"), projection);
				heavy_series::pack(node->set("view"), view);
				heavy_series::pack(node->set("size"), size);
				heavy_series::pack(node->set("diffuse"), diffuse);
				series::pack(node->set("emission"), emission);
				series::pack(node->set("disperse"), disperse);
				series::pack(node->set("cutoff"), cutoff);
				series::pack(node->set("shadow-bias"), shadow.bias);
				series::pack(node->set("shadow-distance"), shadow.distance);
				series::pack(node->set("shadow-softness"), shadow.softness);
				series::pack(node->set("shadow-iterations"), shadow.iterations);
				series::pack(node->set("shadow-enabled"), shadow.enabled);
			}
			void spot_light::message(const std::string_view& name, core::variant_args& args)
			{
				if (name == "depth-flush")
					depth_map = nullptr;
			}
			void spot_light::synchronize(core::timer* time)
			{
				cutoff = compute::mathf::clamp(cutoff, 0.0f, 180.0f);
			}
			size_t spot_light::get_unit_bounds(trigonometry::vector3& min, trigonometry::vector3& max) const
			{
				min = size.radius * -1.25f;
				max = size.radius * 1.25f;
				return BOX_LIGHT;
			}
			float spot_light::get_visibility(const viewer& view, float distance) const
			{
				return get_visibility_radius(parent, view, distance);
			}
			component* spot_light::copy(entity* init) const
			{
				spot_light* target = new spot_light(init);
				target->diffuse = diffuse;
				target->projection = projection;
				target->view = view;
				target->size = size;
				target->cutoff = cutoff;
				target->emission = emission;
				memcpy(&target->shadow, &shadow, sizeof(shadow));

				return target;
			}
			void spot_light::generate_origin()
			{
				auto* transform = parent->get_transform();
				auto& space = transform->get_spacing(trigonometry::positioning::global);
				view = trigonometry::matrix4x4::create_view(space.position, space.rotation.inv_y());
				projection = trigonometry::matrix4x4::create_perspective(cutoff, 1, 0.1f, shadow.distance);
			}
			void spot_light::set_size(const attenuation& value)
			{
				size = value;
				get_entity()->get_transform()->make_dirty();
			}
			const attenuation& spot_light::get_size()
			{
				return size;
			}

			line_light::line_light(entity* ref) : component(ref, actor_set::cullable)
			{
			}
			void line_light::deserialize(core::schema* node)
			{
				VI_ASSERT(node != nullptr, "schema should be set");
				heavy_series::unpack(node->find("diffuse"), &diffuse);
				series::unpack(node->find("emission"), &emission);
				series::unpack(node->find("disperse"), &disperse);

				for (uint32_t i = 0; i < 6; i++)
				{
					heavy_series::unpack(node->find("projection-" + core::to_string(i)), &projection[i]);
					heavy_series::unpack(node->find("view-" + core::to_string(i)), &view[i]);
				}

				for (uint32_t i = 0; i < 6; i++)
					series::unpack(node->find("shadow-distance-" + core::to_string(i)), &shadow.distance[i]);

				series::unpack(node->find("shadow-cascades"), &shadow.cascades);
				series::unpack(node->find("shadow-far"), &shadow.far);
				series::unpack(node->find("shadow-near"), &shadow.near);
				series::unpack(node->find("shadow-bias"), &shadow.bias);
				series::unpack(node->find("shadow-softness"), &shadow.softness);
				series::unpack(node->find("shadow-iterations"), &shadow.iterations);
				series::unpack(node->find("shadow-enabled"), &shadow.enabled);
				heavy_series::unpack(node->find("rlh-emission"), &sky.rlh_emission);
				heavy_series::unpack(node->find("mie-emission"), &sky.mie_emission);
				series::unpack(node->find("rlh-height"), &sky.rlh_height);
				heavy_series::unpack(node->find("mie-height"), &sky.mie_emission);
				series::unpack(node->find("mie-direction"), &sky.mie_direction);
				series::unpack(node->find("inner-radius"), &sky.inner_radius);
				series::unpack(node->find("outer-radius"), &sky.outer_radius);
				series::unpack(node->find("sky-intensity"), &sky.intensity);
			}
			void line_light::serialize(core::schema* node)
			{
				VI_ASSERT(node != nullptr, "schema should be set");
				heavy_series::pack(node->set("diffuse"), diffuse);
				series::pack(node->set("emission"), emission);
				series::pack(node->set("disperse"), disperse);

				for (uint32_t i = 0; i < 6; i++)
				{
					heavy_series::pack(node->set("projection-" + core::to_string(i)), projection[i]);
					heavy_series::pack(node->set("view-" + core::to_string(i)), view[i]);
				}

				for (uint32_t i = 0; i < 6; i++)
					series::pack(node->set("shadow-distance-" + core::to_string(i)), shadow.distance[i]);

				series::pack(node->set("shadow-cascades"), shadow.cascades);
				series::pack(node->set("shadow-far"), shadow.far);
				series::pack(node->set("shadow-near"), shadow.near);
				series::pack(node->set("shadow-bias"), shadow.bias);
				series::pack(node->set("shadow-softness"), shadow.softness);
				series::pack(node->set("shadow-iterations"), shadow.iterations);
				series::pack(node->set("shadow-enabled"), shadow.enabled);
				heavy_series::pack(node->set("rlh-emission"), sky.rlh_emission);
				heavy_series::pack(node->set("mie-emission"), sky.mie_emission);
				series::pack(node->set("rlh-height"), sky.rlh_height);
				heavy_series::pack(node->set("mie-height"), sky.mie_emission);
				series::pack(node->set("mie-direction"), sky.mie_direction);
				series::pack(node->set("inner-radius"), sky.inner_radius);
				series::pack(node->set("outer-radius"), sky.outer_radius);
				series::pack(node->set("sky-intensity"), sky.intensity);
			}
			void line_light::message(const std::string_view& name, core::variant_args& args)
			{
				if (name == "depth-flush")
					depth_map = nullptr;
			}
			component* line_light::copy(entity* init) const
			{
				line_light* target = new line_light(init);
				target->diffuse = diffuse;
				target->emission = emission;
				memcpy(target->projection, projection, sizeof(trigonometry::matrix4x4) * 6);
				memcpy(target->view, view, sizeof(trigonometry::matrix4x4) * 6);
				memcpy(&target->shadow, &shadow, sizeof(shadow));
				memcpy(&target->sky, &sky, sizeof(sky));

				return target;
			}
			void line_light::generate_origin()
			{
				auto* viewer = (components::camera*)parent->get_scene()->get_camera();
				trigonometry::vector3 direction = parent->get_transform()->get_position().snormalize();
				trigonometry::vector3 position = viewer->get_entity()->get_transform()->get_position();
				trigonometry::matrix4x4 light_view = trigonometry::matrix4x4::create_look_at(position, position - direction, trigonometry::vector3::up());
				trigonometry::matrix4x4 view_to_light = viewer->get_view().inv() * light_view;
				uint32_t cascades = std::min<uint32_t>(shadow.cascades, 6);
				float size_box = 10.0f;
				float plane_box = 5.0f;
				for (uint32_t i = 0; i < cascades; i++)
				{
					float near = (i < 1 ? 0.1f : shadow.distance[i - 1]), far = shadow.distance[i];
					trigonometry::frustum8c frustum(compute::mathf::deg2rad() * 90.0f, 1.0f, near, far);
					frustum.transform(view_to_light);

					trigonometry::vector2 x, y, z;
					frustum.get_bounding_box(1.25f, &x, &y, &z);
					x = trigonometry::vector2(std::min(x.x, -size_box), std::max(x.y, size_box));
					y = trigonometry::vector2(std::min(y.x, -size_box), std::max(y.y, size_box));
					z = trigonometry::vector2(std::min(z.x, -plane_box), std::max(z.y, plane_box));

					projection[i] = trigonometry::matrix4x4::create_orthographic_off_center(x.x, x.y, y.x, y.y, z.x * shadow.near, z.y * shadow.far);
					view[i] = light_view;
				}
			}

			surface_light::surface_light(entity* ref) : component(ref, actor_set::cullable), projection(trigonometry::matrix4x4::create_perspective(90.0f, 1, 0.01f, 100.0f))
			{
			}
			surface_light::~surface_light()
			{
				core::memory::release(diffuse_map_x[0]);
				core::memory::release(diffuse_map_x[1]);
				core::memory::release(diffuse_map_y[0]);
				core::memory::release(diffuse_map_y[1]);
				core::memory::release(diffuse_map_z[0]);
				core::memory::release(diffuse_map_z[1]);
				core::memory::release(diffuse_map);
				core::memory::release(probe);
			}
			void surface_light::deserialize(core::schema* node)
			{
				VI_ASSERT(node != nullptr, "schema should be set");

				auto* scene = parent->get_scene(); core::string path;
				if (!series::unpack(node->find("diffuse-map"), &path) || path.empty())
				{
					if (series::unpack(node->find("diffuse-map-px"), &path))
					{
						scene->load_resource<graphics::texture_2d>(this, path, [this](expects_content<graphics::texture_2d*>&& new_texture)
						{
							core::memory::release(diffuse_map_x[0]);
							if (new_texture && *new_texture != nullptr)
							{
								diffuse_map_x[0] = *new_texture;
								diffuse_map_x[0]->add_ref();
							}
						});
					}

					if (series::unpack(node->find("diffuse-map-nx"), &path))
					{
						scene->load_resource<graphics::texture_2d>(this, path, [this](expects_content<graphics::texture_2d*>&& new_texture)
						{
							core::memory::release(diffuse_map_x[1]);
							if (new_texture && *new_texture != nullptr)
							{
								diffuse_map_x[1] = *new_texture;
								diffuse_map_x[1]->add_ref();
							}
						});
					}

					if (series::unpack(node->find("diffuse-map-py"), &path))
					{
						scene->load_resource<graphics::texture_2d>(this, path, [this](expects_content<graphics::texture_2d*>&& new_texture)
						{
							core::memory::release(diffuse_map_y[0]);
							if (new_texture && *new_texture != nullptr)
							{
								diffuse_map_y[0] = *new_texture;
								diffuse_map_y[0]->add_ref();
							}
						});
					}

					if (series::unpack(node->find("diffuse-map-ny"), &path))
					{
						scene->load_resource<graphics::texture_2d>(this, path, [this](expects_content<graphics::texture_2d*>&& new_texture)
						{
							core::memory::release(diffuse_map_y[1]);
							if (new_texture && *new_texture != nullptr)
							{
								diffuse_map_y[1] = *new_texture;
								diffuse_map_y[1]->add_ref();
							}
						});
					}

					if (series::unpack(node->find("diffuse-map-pz"), &path))
					{
						scene->load_resource<graphics::texture_2d>(this, path, [this](expects_content<graphics::texture_2d*>&& new_texture)
						{
							core::memory::release(diffuse_map_z[0]);
							if (new_texture && *new_texture != nullptr)
							{
								diffuse_map_z[0] = *new_texture;
								diffuse_map_z[0]->add_ref();
							}
						});
					}

					if (series::unpack(node->find("diffuse-map-nz"), &path))
					{
						scene->load_resource<graphics::texture_2d>(this, path, [this](expects_content<graphics::texture_2d*>&& new_texture)
						{
							core::memory::release(diffuse_map_z[1]);
							if (new_texture && *new_texture != nullptr)
							{
								diffuse_map_z[1] = *new_texture;
								diffuse_map_z[1]->add_ref();
							}
						});
					}
				}
				else
				{
					scene->load_resource<graphics::texture_2d>(this, path, [this](expects_content<graphics::texture_2d*>&& new_texture)
					{
						core::memory::release(diffuse_map);
						if (new_texture && *new_texture != nullptr)
						{
							diffuse_map = *new_texture;
							diffuse_map->add_ref();
						}
					});
				}

				core::vector<trigonometry::matrix4x4> views;
				heavy_series::unpack(node->find("projection"), &projection);
				heavy_series::unpack(node->find("view"), &views);
				heavy_series::unpack(node->find("tick"), &tick);
				heavy_series::unpack(node->find("diffuse"), &diffuse);
				heavy_series::unpack(node->find("size"), &size);
				series::unpack(node->find("emission"), &emission);
				series::unpack(node->find("infinity"), &infinity);
				series::unpack(node->find("parallax"), &parallax);
				series::unpack(node->find("static-mask"), &static_mask);

				size_t count = compute::math<size_t>::min(views.size(), 6);
				for (size_t i = 0; i < count; i++)
					view[i] = views[i];

				if (!diffuse_map)
					set_diffuse_map(diffuse_map_x, diffuse_map_y, diffuse_map_z);
				else
					set_diffuse_map(diffuse_map);
			}
			void surface_light::serialize(core::schema* node)
			{
				VI_ASSERT(node != nullptr, "schema should be set");

				auto* scene = parent->get_scene();
				if (!diffuse_map)
				{
					series::pack(node->set("diffuse-map-px"), scene->find_resource_id<graphics::texture_2d>(diffuse_map_x[0]));
					series::pack(node->set("diffuse-map-nx"), scene->find_resource_id<graphics::texture_2d>(diffuse_map_x[1]));
					series::pack(node->set("diffuse-map-py"), scene->find_resource_id<graphics::texture_2d>(diffuse_map_y[0]));
					series::pack(node->set("diffuse-map-ny"), scene->find_resource_id<graphics::texture_2d>(diffuse_map_y[1]));
					series::pack(node->set("diffuse-map-pz"), scene->find_resource_id<graphics::texture_2d>(diffuse_map_z[0]));
					series::pack(node->set("diffuse-map-nz"), scene->find_resource_id<graphics::texture_2d>(diffuse_map_z[1]));
				}
				else
					series::pack(node->set("diffuse-map"), scene->find_resource_id<graphics::texture_2d>(diffuse_map));

				core::vector<trigonometry::matrix4x4> views;
				for (int64_t i = 0; i < 6; i++)
					views.push_back(view[i]);

				heavy_series::pack(node->set("projection"), projection);
				heavy_series::pack(node->set("view"), views);
				heavy_series::pack(node->set("tick"), tick);
				heavy_series::pack(node->set("size"), size);
				heavy_series::pack(node->set("diffuse"), diffuse);
				series::pack(node->set("emission"), emission);
				series::pack(node->set("infinity"), infinity);
				series::pack(node->set("parallax"), parallax);
				series::pack(node->set("static-mask"), static_mask);
			}
			size_t surface_light::get_unit_bounds(trigonometry::vector3& min, trigonometry::vector3& max) const
			{
				min = size.radius * -1.25f;
				max = size.radius * 1.25f;
				return BOX_LIGHT;
			}
			float surface_light::get_visibility(const viewer& view, float distance) const
			{
				if (infinity > 0.0f)
					return 1.0f;

				return get_visibility_radius(parent, view, distance);
			}
			component* surface_light::copy(entity* init) const
			{
				surface_light* target = new surface_light(init);
				target->projection = projection;
				target->diffuse = diffuse;
				target->emission = emission;
				target->size = size;
				target->tick = tick;
				memcpy(target->view, view, 6 * sizeof(trigonometry::matrix4x4));

				if (!diffuse_map)
					target->set_diffuse_map(diffuse_map_x, diffuse_map_y, diffuse_map_z);
				else
					target->set_diffuse_map(diffuse_map);

				return target;
			}
			void surface_light::set_probe_cache(graphics::texture_cube* new_cache)
			{
				probe = new_cache;
			}
			void surface_light::set_size(const attenuation& value)
			{
				size = value;
				get_entity()->get_transform()->make_dirty();
			}
			bool surface_light::set_diffuse_map(graphics::texture_2d* map)
			{
				VI_ASSERT(parent->get_scene()->get_device() != nullptr, "graphics device should be set");
				core::memory::release(diffuse_map_x[0]);
				core::memory::release(diffuse_map_x[1]);
				core::memory::release(diffuse_map_y[0]);
				core::memory::release(diffuse_map_y[1]);
				core::memory::release(diffuse_map_z[0]);
				core::memory::release(diffuse_map_z[1]);
				core::memory::release(diffuse_map);

				diffuse_map = map;
				if (!diffuse_map)
					return false;

				diffuse_map->add_ref();
				core::memory::release(probe);
				probe = parent->get_scene()->get_device()->create_texture_cube(diffuse_map).or_else(nullptr);
				return probe != nullptr;
			}
			bool surface_light::set_diffuse_map(graphics::texture_2d* const map_x[2], graphics::texture_2d* const map_y[2], graphics::texture_2d* const map_z[2])
			{
				VI_ASSERT(parent->get_scene()->get_device() != nullptr, "graphics device should be set");
				core::memory::release(diffuse_map_x[0]);
				core::memory::release(diffuse_map_x[1]);
				core::memory::release(diffuse_map_y[0]);
				core::memory::release(diffuse_map_y[1]);
				core::memory::release(diffuse_map_z[0]);
				core::memory::release(diffuse_map_z[1]);
				core::memory::release(diffuse_map);
				if (!map_x[0] || !map_x[1] || !map_y[0] || !map_y[1] || !map_z[0] || !map_z[1])
					return false;

				graphics::texture_2d* resources[6];
				resources[0] = diffuse_map_x[0] = map_x[0]; map_x[0]->add_ref();
				resources[1] = diffuse_map_x[1] = map_x[1]; map_x[1]->add_ref();
				resources[2] = diffuse_map_y[0] = map_y[0]; map_y[0]->add_ref();
				resources[3] = diffuse_map_y[1] = map_y[1]; map_y[1]->add_ref();
				resources[4] = diffuse_map_z[0] = map_z[0]; map_z[0]->add_ref();
				resources[5] = diffuse_map_z[1] = map_z[1]; map_z[1]->add_ref();

				core::memory::release(probe);
				probe = parent->get_scene()->get_device()->create_texture_cube(resources).or_else(nullptr);
				return probe != nullptr;
			}
			bool surface_light::is_image_based() const
			{
				return diffuse_map_x[0] != nullptr || diffuse_map != nullptr;
			}
			const attenuation& surface_light::get_size()
			{
				return size;
			}
			graphics::texture_cube* surface_light::get_probe_cache() const
			{
				return probe;
			}
			graphics::texture_2d* surface_light::get_diffuse_map_xp()
			{
				return diffuse_map_x[0];
			}
			graphics::texture_2d* surface_light::get_diffuse_map_xn()
			{
				return diffuse_map_x[1];
			}
			graphics::texture_2d* surface_light::get_diffuse_map_yp()
			{
				return diffuse_map_y[0];
			}
			graphics::texture_2d* surface_light::get_diffuse_map_yn()
			{
				return diffuse_map_y[1];
			}
			graphics::texture_2d* surface_light::get_diffuse_map_zp()
			{
				return diffuse_map_z[0];
			}
			graphics::texture_2d* surface_light::get_diffuse_map_zn()
			{
				return diffuse_map_z[1];
			}
			graphics::texture_2d* surface_light::get_diffuse_map()
			{
				return diffuse_map;
			}

			illuminator::illuminator(entity* ref) : component(ref, actor_set::cullable), voxel_map(nullptr), regenerate(true)
			{
				inside.delay = 30.0;
				outside.delay = 10000.0;
				ray_step = 0.5f;
				max_steps = 256.0f;
				distance = 12.0f;
				radiance = 1.0f;
				occlusion = 0.33f;
				specular = 2.0f;
				length = 1.0f;
				margin = 3.828424f;
				offset = -0.01f;
				angle = 0.5;
				bleeding = 0.33f;
			}
			void illuminator::deserialize(core::schema* node)
			{
				VI_ASSERT(node != nullptr, "schema should be set");

				series::unpack(node->find("inside-delay"), &inside.delay);
				series::unpack(node->find("outside-delay"), &outside.delay);
				series::unpack(node->find("ray-step"), &ray_step);
				series::unpack(node->find("max-steps"), &max_steps);
				series::unpack(node->find("distance"), &distance);
				series::unpack(node->find("radiance"), &radiance);
				series::unpack(node->find("length"), &length);
				series::unpack(node->find("margin"), &margin);
				series::unpack(node->find("offset"), &offset);
				series::unpack(node->find("angle"), &angle);
				series::unpack(node->find("occlusion"), &occlusion);
				series::unpack(node->find("specular"), &specular);
				series::unpack(node->find("bleeding"), &bleeding);
			}
			void illuminator::serialize(core::schema* node)
			{
				VI_ASSERT(node != nullptr, "schema should be set");

				series::pack(node->set("inside-delay"), inside.delay);
				series::pack(node->set("outside-delay"), outside.delay);
				series::pack(node->set("ray-step"), ray_step);
				series::pack(node->set("max-steps"), max_steps);
				series::pack(node->set("distance"), distance);
				series::pack(node->set("radiance"), radiance);
				series::pack(node->set("length"), length);
				series::pack(node->set("margin"), margin);
				series::pack(node->set("offset"), offset);
				series::pack(node->set("angle"), angle);
				series::pack(node->set("occlusion"), occlusion);
				series::pack(node->set("specular"), specular);
				series::pack(node->set("bleeding"), bleeding);
			}
			void illuminator::message(const std::string_view& name, core::variant_args& args)
			{
				if (name == "depth-flush")
				{
					voxel_map = nullptr;
					regenerate = true;
				}
			}
			component* illuminator::copy(entity* init) const
			{
				illuminator* target = new illuminator(init);
				target->inside = inside;
				target->outside = outside;
				target->ray_step = ray_step;
				target->max_steps = max_steps;
				target->radiance = radiance;
				target->length = length;
				target->occlusion = occlusion;
				target->specular = specular;

				return target;
			}

			camera::camera(entity* ref) : component(ref, actor_set::synchronize), mode(projection_mode::perspective), renderer(new render_system(ref->get_scene(), this)), viewport({ 0, 0, 512, 512, 0, 1 })
			{
			}
			camera::~camera()
			{
				core::memory::release(renderer);
			}
			void camera::activate(component* init)
			{
				VI_ASSERT(parent->get_scene()->get_device() != nullptr, "graphics device should be set");
				VI_ASSERT(parent->get_scene()->get_device()->get_render_target() != nullptr, "render target should be set");

				scene_graph* scene = parent->get_scene();
				viewport = scene->get_device()->get_render_target()->get_viewport();
				if (init == this)
					renderer->remount();
			}
			void camera::deserialize(core::schema* node)
			{
				VI_ASSERT(node != nullptr, "schema should be set");
				VI_ASSERT(parent->get_scene()->get_device() != nullptr, "graphics device should be set");

				int _Mode = 0;
				if (series::unpack(node->find("mode"), &_Mode))
					mode = (projection_mode)_Mode;

				heavy_series::unpack(node->find("projection"), &projection);
				series::unpack(node->find("field-of-view"), &field_of_view);
				series::unpack(node->find("far-plane"), &far_plane);
				series::unpack(node->find("near-plane"), &near_plane);
				series::unpack(node->find("width"), &width);
				series::unpack(node->find("height"), &height);
				series::unpack_a(node->find("occluder-skips"), &renderer->occluder_skips);
				series::unpack_a(node->find("occludee-skips"), &renderer->occludee_skips);
				series::unpack_a(node->find("occlusion-skips"), &renderer->occlusion_skips);
				series::unpack(node->find("occlusion-cull"), &renderer->occlusion_culling);
				series::unpack(node->find("occludee-scaling"), &renderer->occludee_scaling);
				series::unpack_a(node->find("max-queries"), &renderer->max_queries);

				core::vector<core::schema*> renderers = node->fetch_collection("renderers.renderer");
				for (auto& render : renderers)
				{
					uint64_t id;
					if (!series::unpack(render->find("id"), &id))
						continue;

					layer::renderer* target = core::composer::create<layer::renderer>(id, renderer);
					if (!renderer->add_renderer(target))
						continue;

					core::schema* meta = render->find("metadata");
					if (!meta)
						meta = render->set("metadata");

					target->deactivate();
					target->deserialize(meta);
					target->activate();
					series::unpack(render->find("active"), &target->active);
				}
			}
			void camera::serialize(core::schema* node)
			{
				VI_ASSERT(node != nullptr, "schema should be set");

				series::pack(node->set("mode"), (int)mode);
				heavy_series::pack(node->set("projection"), projection);
				series::pack(node->set("field-of-view"), field_of_view);
				series::pack(node->set("far-plane"), far_plane);
				series::pack(node->set("near-plane"), near_plane);
				series::pack(node->set("width"), width);
				series::pack(node->set("height"), height);
				series::pack(node->set("occluder-skips"), (uint64_t)renderer->occluder_skips);
				series::pack(node->set("occludee-skips"), (uint64_t)renderer->occludee_skips);
				series::pack(node->set("occlusion-skips"), (uint64_t)renderer->occlusion_skips);
				series::pack(node->set("occlusion-cull"), renderer->occlusion_culling);
				series::pack(node->set("occludee-scaling"), renderer->occludee_scaling);
				series::pack(node->set("max-queries"), (uint64_t)renderer->max_queries);

				core::schema* renderers = node->set("renderers", core::var::array());
				for (auto* next : renderer->get_renderers())
				{
					core::schema* render = renderers->set("renderer");
					series::pack(render->set("id"), next->get_id());
					series::pack(render->set("active"), next->active);
					next->serialize(render->set("metadata"));
				}
			}
			void camera::synchronize(core::timer* time)
			{
				float w = width, h = height;
				if (w <= 0 || h <= 0)
				{
					w = viewport.width;
					h = viewport.height;
				}

				if (mode == projection_mode::perspective)
					projection = trigonometry::matrix4x4::create_perspective(field_of_view, w / h, near_plane, far_plane);
				else if (mode == projection_mode::orthographic)
					projection = trigonometry::matrix4x4::create_orthographic(w, h, near_plane, far_plane);
			}
			void camera::get_viewer(viewer* output)
			{
				VI_ASSERT(output != nullptr, "viewer should be set");

				auto& space = parent->get_transform()->get_spacing(trigonometry::positioning::global);
				render_culling culling = (mode == projection_mode::perspective ? render_culling::depth : render_culling::disable);

				output->set(get_view(), projection, space.position, space.rotation, field_of_view, get_aspect(), near_plane, far_plane, culling);
				output->renderer = renderer;
				view = *output;
			}
			void camera::resize_buffers()
			{
				scene_graph* scene = parent->get_scene();
				viewport = scene->get_device()->get_render_target()->get_viewport();
				for (auto* next : renderer->get_renderers())
					next->resize_buffers();
			}
			viewer& camera::get_viewer()
			{
				return view;
			}
			render_system* camera::get_renderer()
			{
				return renderer;
			}
			trigonometry::matrix4x4 camera::get_projection()
			{
				return projection;
			}
			trigonometry::vector3 camera::get_view_position()
			{
				auto* transform = parent->get_transform();
				return transform->get_position().inv_x().inv_y();
			}
			trigonometry::matrix4x4 camera::get_view_projection()
			{
				return get_view() * projection;
			}
			trigonometry::matrix4x4 camera::get_view()
			{
				auto& space = parent->get_transform()->get_spacing(trigonometry::positioning::global);
				return trigonometry::matrix4x4::create_view(space.position, space.rotation);
			}
			trigonometry::frustum8c camera::get_frustum8c()
			{
				trigonometry::frustum8c result(compute::mathf::deg2rad() * field_of_view, get_aspect(), near_plane, far_plane * 0.25f);
				result.transform(get_view().inv());
				return result;
			}
			trigonometry::frustum6p camera::get_frustum6p()
			{
				trigonometry::frustum6p result(get_view_projection());
				return result;
			}
			trigonometry::ray camera::get_screen_ray(const trigonometry::vector2& position)
			{
				float w = width, h = height;
				if (w <= 0 || h <= 0)
				{
					graphics::viewport v = renderer->get_device()->get_render_target()->get_viewport();
					w = v.width; h = v.height;
				}

				return trigonometry::geometric::create_cursor_ray(parent->get_transform()->get_position(), position, trigonometry::vector2(w, h), projection.inv(), get_view().inv());
			}
			trigonometry::ray camera::get_cursor_ray()
			{
				auto* activity = parent->get_scene()->get_activity();
				if (!activity)
					return trigonometry::ray();

				return get_screen_ray(activity->get_cursor_position());
			}
			float camera::get_distance(entity* other)
			{
				VI_ASSERT(other != nullptr, "other should be set");
				return other->get_transform()->get_position().distance(view.position);
			}
			float camera::get_width()
			{
				float w = width;
				if (w <= 0)
					w = viewport.width;

				return w;
			}
			float camera::get_height()
			{
				float h = height;
				if (h <= 0)
					h = viewport.height;

				return h;
			}
			float camera::get_aspect()
			{
				float w = width, h = height;
				if (w <= 0 || h <= 0)
				{
					w = viewport.width;
					h = viewport.height;
				}

				return w / h;
			}
			bool camera::ray_test(const trigonometry::ray& ray, entity* other, trigonometry::vector3* hit)
			{
				VI_ASSERT(other != nullptr, "other should be set");
				return trigonometry::geometric::cursor_ray_test(ray, other->get_transform()->get_bias(), hit);
			}
			bool camera::ray_test(const trigonometry::ray& ray, const trigonometry::matrix4x4& world, trigonometry::vector3* hit)
			{
				return trigonometry::geometric::cursor_ray_test(ray, world, hit);
			}
			component* camera::copy(entity* init) const
			{
				camera* target = new camera(init);
				target->far_plane = far_plane;
				target->near_plane = near_plane;
				target->width = width;
				target->height = height;
				target->mode = mode;
				target->field_of_view = field_of_view;
				target->projection = projection;

				return target;
			}

			scriptable::scriptable(entity* ref) : component(ref, actor_set::update | actor_set::message), compiler(nullptr), source(source_type::resource), invoke(invoke_type::typeless)
			{
			}
			scriptable::~scriptable()
			{
				core::memory::release(compiler);
			}
			void scriptable::deserialize(core::schema* node)
			{
				VI_ASSERT(node != nullptr, "schema should be set");

				core::string type;
				if (series::unpack(node->find("source"), &type))
				{
					if (type == "memory")
						source = source_type::memory;
					else if (type == "resource")
						source = source_type::resource;
				}

				if (series::unpack(node->find("invoke"), &type))
				{
					if (type == "typeless")
						invoke = invoke_type::typeless;
					else if (type == "normal")
						invoke = invoke_type::normal;
				}

				if (!series::unpack(node->find("resource"), &resource) || resource.empty())
					return;

				auto success = load_source().get();
				if (success)
					deserialize_call(node).wait();
			}
			void scriptable::serialize(core::schema* node)
			{
				VI_ASSERT(node != nullptr, "schema should be set");

				if (source == source_type::memory)
					series::pack(node->set("source"), "memory");
				else if (source == source_type::resource)
					series::pack(node->set("source"), "resource");

				if (invoke == invoke_type::typeless)
					series::pack(node->set("invoke"), "typeless");
				else if (invoke == invoke_type::normal)
					series::pack(node->set("invoke"), "normal");

				series::pack(node->set("resource"), parent->get_scene()->as_resource_path(resource));
				serialize_call(node).wait();
			}
			void scriptable::activate(component* init)
			{
				if (!parent->get_scene()->is_active())
					return;

				call(entry.awake, [this, &init](scripting::immediate_context* context)
				{
					if (invoke == invoke_type::typeless)
						return;

					component* current = this;
					context->set_arg_object(0, current);
					context->set_arg_object(1, init);
				});
			}
			void scriptable::deactivate()
			{
				call(entry.asleep, [this](scripting::immediate_context* context)
				{
					if (invoke == invoke_type::typeless)
						return;

					component* current = this;
					context->set_arg_object(0, current);
				});
			}
			void scriptable::update(core::timer* time)
			{
				call(entry.update, [this, &time](scripting::immediate_context* context)
				{
					if (invoke == invoke_type::typeless)
						return;

					component* current = this;
					context->set_arg_object(0, current);
					context->set_arg_object(1, time);
				});
			}
			void scriptable::message(const std::string_view& name, core::variant_args& args)
			{
				call(entry.message, [this, name, args](scripting::immediate_context* context)
				{
					if (invoke == invoke_type::typeless)
						return;

					scripting::bindings::dictionary* map = scripting::bindings::dictionary::create(compiler->get_vm());
					if (map != nullptr)
					{
						int type_id = compiler->get_vm()->get_type_id_by_decl("variant");
						for (auto& item : args)
						{
							core::variant next = std::move(item.second);
							map->set(item.first, &next, type_id);
						}
					}

					component* current = this;
					context->set_arg_object(0, current);
					context->set_arg_object(1, (void*)&name);
					context->set_arg_object(2, map);
				});
			}
			component* scriptable::copy(entity* init) const
			{
				scriptable* target = new scriptable(init);
				target->invoke = invoke;
				target->load_source(source, resource);

				if (!compiler || !target->compiler)
					return target;

				scripting::library from = compiler->get_module();
				scripting::library to = target->compiler->get_module();
				scripting::virtual_machine* vm = compiler->get_vm();

				if (!from.is_valid() || !to.is_valid())
					return target;

				int count = (int)from.get_properties_count();
				for (int i = 0; i < count; i++)
				{
					scripting::property_info fSource;
					if (!from.get_property(i, &fSource))
						continue;

					scripting::property_info dest;
					if (!to.get_property(i, &dest))
						continue;

					if (fSource.type_id != dest.type_id)
						continue;

					if (fSource.type_id < (int)scripting::type_id::boolf || fSource.type_id >(int)scripting::type_id::doublef)
					{
						scripting::typeinfo type = vm->get_type_info_by_id(fSource.type_id);
						if (fSource.pointer != nullptr && type.is_valid())
						{
							void* object = vm->create_object_copy(fSource.pointer, type);
							if (object != nullptr)
								vm->assign_object(dest.pointer, object, type);
						}
					}
					else
					{
						auto size = vm->get_size_of_primitive_type(fSource.type_id);
						if (size)
							memcpy(dest.pointer, fSource.pointer, *size);
					}
				}

				return target;
			}
			scripting::expects_promise_vm<scripting::execution> scriptable::deserialize_call(core::schema* node)
			{
				core::schema* cache = node->find("cache");
				if (cache != nullptr)
				{
					for (auto& var : cache->get_childs())
					{
						int type_id = -1;
						if (!series::unpack(var->find("type"), &type_id))
							continue;

						switch ((scripting::type_id)type_id)
						{
							case scripting::type_id::boolf:
							{
								bool result = false;
								if (series::unpack(var->find("data"), &result))
									set_type_property_by_name(var->key.c_str(), result);
								break;
							}
							case scripting::type_id::int8:
							{
								int64_t result = 0;
								if (series::unpack(var->find("data"), &result))
									set_type_property_by_name(var->key.c_str(), (char)result);
								break;
							}
							case scripting::type_id::int16:
							{
								int64_t result = 0;
								if (series::unpack(var->find("data"), &result))
									set_type_property_by_name(var->key.c_str(), (short)result);
								break;
							}
							case scripting::type_id::int32:
							{
								int64_t result = 0;
								if (series::unpack(var->find("data"), &result))
									set_type_property_by_name(var->key.c_str(), (int)result);
								break;
							}
							case scripting::type_id::int64:
							{
								int64_t result = 0;
								if (series::unpack(var->find("data"), &result))
									set_type_property_by_name(var->key.c_str(), result);
								break;
							}
							case scripting::type_id::uint8:
							{
								int64_t result = 0;
								if (series::unpack(var->find("data"), &result))
									set_type_property_by_name(var->key.c_str(), (uint8_t)result);
								break;
							}
							case scripting::type_id::uint16:
							{
								int64_t result = 0;
								if (series::unpack(var->find("data"), &result))
									set_type_property_by_name(var->key.c_str(), (uint16_t)result);
								break;
							}
							case scripting::type_id::uint32:
							{
								int64_t result = 0;
								if (series::unpack(var->find("data"), &result))
									set_type_property_by_name(var->key.c_str(), (uint32_t)result);
								break;
							}
							case scripting::type_id::uint64:
							{
								int64_t result = 0;
								if (series::unpack(var->find("data"), &result))
									set_type_property_by_name(var->key.c_str(), (uint64_t)result);
								break;
							}
							case scripting::type_id::floatf:
							{
								float result = 0.0f;
								if (series::unpack(var->find("data"), &result))
									set_type_property_by_name(var->key.c_str(), result);
								break;
							}
							case scripting::type_id::doublef:
							{
								double result = 0.0;
								if (series::unpack(var->find("data"), &result))
									set_type_property_by_name(var->key.c_str(), result);
								break;
							}
							default:
							{
								core::string result;
								if (series::unpack(var->find("data"), &result))
									set_type_property_by_name(var->key.c_str(), result);
								break;
							}
						}
					}
				}

				return call(entry.deserialize, [this, node](scripting::immediate_context* context)
				{
					if (invoke == invoke_type::typeless)
						return;

					component* current = this;
					context->set_arg_object(0, current);
					context->set_arg_object(1, node);
				});
			}
			scripting::expects_promise_vm<scripting::execution> scriptable::serialize_call(core::schema* node)
			{
				auto has_properties = get_properties_count();
				if (has_properties)
				{
					size_t count = *has_properties;
					core::schema* cache = node->set("cache");
					for (size_t i = 0; i < count; i++)
					{
						scripting::property_info result;
						if (!get_property_by_index(i, &result) || result.name.empty() || !result.pointer)
							continue;

						core::schema* var = core::var::set::object();
						series::pack(var->set("type"), result.type_id);

						switch ((scripting::type_id)result.type_id)
						{
							case scripting::type_id::boolf:
								series::pack(var->set("data"), *(bool*)result.pointer);
								break;
							case scripting::type_id::int8:
								series::pack(var->set("data"), (int64_t) * (char*)result.pointer);
								break;
							case scripting::type_id::int16:
								series::pack(var->set("data"), (int64_t) * (short*)result.pointer);
								break;
							case scripting::type_id::int32:
								series::pack(var->set("data"), (int64_t) * (int*)result.pointer);
								break;
							case scripting::type_id::int64:
								series::pack(var->set("data"), *(int64_t*)result.pointer);
								break;
							case scripting::type_id::uint8:
								series::pack(var->set("data"), (int64_t) * (uint8_t*)result.pointer);
								break;
							case scripting::type_id::uint16:
								series::pack(var->set("data"), (int64_t) * (uint16_t*)result.pointer);
								break;
							case scripting::type_id::uint32:
								series::pack(var->set("data"), (int64_t) * (uint32_t*)result.pointer);
								break;
							case scripting::type_id::uint64:
								series::pack(var->set("data"), (int64_t) * (uint64_t*)result.pointer);
								break;
							case scripting::type_id::floatf:
								series::pack(var->set("data"), (double)*(float*)result.pointer);
								break;
							case scripting::type_id::doublef:
								series::pack(var->set("data"), *(double*)result.pointer);
								break;
							default:
							{
								scripting::typeinfo type = get_compiler()->get_vm()->get_type_info_by_id(result.type_id);
								if (type.is_valid() && type.get_name() == "string")
									series::pack(var->set("data"), *(core::string*)result.pointer);
								else
									core::memory::release(var);
								break;
							}
						}

						if (var != nullptr)
							cache->set(result.name, var);
					}
				}

				return call(entry.serialize, [this, &node](scripting::immediate_context* context)
				{
					if (invoke == invoke_type::typeless)
						return;

					component* current = this;
					context->set_arg_object(0, current);
					context->set_arg_object(1, node);
				});
			}
			scripting::expects_promise_vm<scripting::execution> scriptable::call(const std::string_view& name, size_t args, scripting::args_callback&& on_args)
			{
				if (!compiler)
					return scripting::expects_promise_vm<scripting::execution>(scripting::virtual_exception(scripting::virtual_error::invalid_configuration));

				return call(get_function_by_name(name, args).get_function(), std::move(on_args));
			}
			scripting::expects_promise_vm<scripting::execution> scriptable::call(asIScriptFunction* function, scripting::args_callback&& on_args)
			{
				if (!compiler)
					return scripting::expects_promise_vm<scripting::execution>(scripting::virtual_exception(scripting::virtual_error::invalid_configuration));

				scripting::function_delegate delegatef(function);
				if (!delegatef.is_valid())
					return scripting::expects_promise_vm<scripting::execution>(scripting::virtual_exception(scripting::virtual_error::no_function));

				protect();
				return delegatef(std::move(on_args)).then<scripting::expects_vm<scripting::execution>>([this](scripting::expects_vm<scripting::execution>&& result)
				{
					unprotect();
					return result;
				});
			}
			scripting::expects_promise_vm<scripting::execution> scriptable::call_entry(const std::string_view& name)
			{
				return call(get_function_by_name(name, invoke == invoke_type::typeless ? 0 : 1).get_function(), [this](scripting::immediate_context* context)
				{
					if (invoke == invoke_type::typeless)
						return;

					component* current = this;
					context->set_arg_object(0, current);
				});
			}
			scripting::expects_promise_vm<void> scriptable::load_source()
			{
				return load_source(source, resource);
			}
			scripting::expects_promise_vm<void> scriptable::load_source(source_type type, const std::string_view& data)
			{
				scene_graph* scene = parent->get_scene();
				if (!compiler)
				{
					auto* vm = scene->get_conf().shared.vm;
					if (!vm)
						return scripting::expects_promise_vm<void>(scripting::virtual_exception(scripting::virtual_error::invalid_configuration));

					compiler = vm->create_compiler();
					compiler->set_pragma_callback([this](compute::preprocessor*, const std::string_view& name, const core::vector<core::string>& args) -> compute::expects_preprocessor<void>
					{
						if (name == "name" && args.size() == 1)
							library = args[0];

						return core::expectation::met;
					});
				}

				source = type;
				resource = data;

				if (resource.empty())
				{
					entry.serialize = nullptr;
					entry.deserialize = nullptr;
					entry.awake = nullptr;
					entry.asleep = nullptr;
					entry.animate = nullptr;
					entry.synchronize = nullptr;
					entry.update = nullptr;
					entry.message = nullptr;
					compiler->clear();
					return scripting::expects_promise_vm<void>(scripting::virtual_exception(scripting::virtual_error::no_module));
				}

				auto status = compiler->prepare("base", source == source_type::resource ? resource : "anonymous", true, true);
				if (!status)
					return scripting::expects_promise_vm<void>(status);

				status = (source == source_type::resource ? compiler->load_file(resource) : compiler->load_code("anonymous", resource));
				if (!status)
					return scripting::expects_promise_vm<void>(status);

				return compiler->compile().then<scripting::expects_vm<void>>([this](scripting::expects_vm<void>&& result)
				{
					entry.animate = get_function_by_name("animate", invoke == invoke_type::typeless ? 0 : 3).get_function();
					entry.serialize = get_function_by_name("serialize", invoke == invoke_type::typeless ? 0 : 3).get_function();
					entry.deserialize = get_function_by_name("deserialize", invoke == invoke_type::typeless ? 0 : 3).get_function();
					entry.awake = get_function_by_name("awake", invoke == invoke_type::typeless ? 0 : 2).get_function();
					entry.asleep = get_function_by_name("asleep", invoke == invoke_type::typeless ? 0 : 1).get_function();
					entry.synchronize = get_function_by_name("synchronize", invoke == invoke_type::typeless ? 0 : 2).get_function();
					entry.update = get_function_by_name("update", invoke == invoke_type::typeless ? 0 : 2).get_function();
					entry.message = get_function_by_name("message", invoke == invoke_type::typeless ? 0 : 3).get_function();
					return result;
				});
			}
			scripting::expects_vm<size_t> scriptable::get_properties_count()
			{
				if (!compiler)
					return scripting::virtual_exception(scripting::virtual_error::invalid_configuration);

				scripting::library base = compiler->get_module();
				if (!base.is_valid())
					return scripting::virtual_exception(scripting::virtual_error::no_module);

				return base.get_properties_count();
			}
			scripting::expects_vm<size_t> scriptable::get_functions_count()
			{
				if (!compiler)
					return scripting::virtual_exception(scripting::virtual_error::invalid_configuration);

				scripting::library base = compiler->get_module();
				if (!base.is_valid())
					return scripting::virtual_exception(scripting::virtual_error::no_module);

				return base.get_function_count();
			}
			void scriptable::set_invocation(invoke_type type)
			{
				invoke = type;
			}
			void scriptable::unload_source()
			{
				load_source(source, "");
			}
			void scriptable::protect()
			{
				add_ref();
				get_entity()->add_ref();
			}
			void scriptable::unprotect()
			{
				get_entity()->release();
				release();
			}
			scripting::compiler* scriptable::get_compiler()
			{
				return compiler;
			}
			scripting::function scriptable::get_function_by_name(const std::string_view& name, size_t args)
			{
				VI_ASSERT(!name.empty(), "name should not be empty");
				if (!compiler)
					return nullptr;

				auto result = compiler->get_module().get_function_by_name(name);
				if (result.is_valid() && result.get_args_count() != args)
					return nullptr;

				return result;
			}
			scripting::function scriptable::get_function_by_index(size_t index, size_t args)
			{
				VI_ASSERT(index >= 0, "index should be greater or equal to zero");
				if (!compiler)
					return nullptr;

				auto result = compiler->get_module().get_function_by_index(index);
				if (result.is_valid() && result.get_args_count() != args)
					return nullptr;

				return result;
			}
			bool scriptable::get_property_by_name(const std::string_view& name, scripting::property_info* result)
			{
				if (!compiler)
					return false;

				scripting::library base = compiler->get_module();
				if (!base.is_valid())
					return false;

				auto index = base.get_property_index_by_name(name);
				if (!index || !base.get_property(*index, result))
					return false;

				return true;
			}
			bool scriptable::get_property_by_index(size_t index, scripting::property_info* result)
			{
				VI_ASSERT(index >= 0, "index should be greater or equal to zero");
				if (!compiler)
					return false;

				scripting::library base = compiler->get_module();
				if (!base.is_valid() || !base.get_property(index, result))
					return false;

				return true;
			}
			scriptable::source_type scriptable::get_source_type()
			{
				return source;
			}
			scriptable::invoke_type scriptable::get_invoke_type()
			{
				return invoke;
			}
			const core::string& scriptable::get_source()
			{
				return resource;
			}
			const core::string& scriptable::get_module_name()
			{
				return library;
			}
		}
	}
}
