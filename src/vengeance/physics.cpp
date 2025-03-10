#include "physics.h"
#ifdef VI_BULLET3
#pragma warning(disable: 4244)
#pragma warning(disable: 4305)
#include <btBulletDynamicsCommon.h>
#include <BulletSoftBody/btSoftRigidDynamicsWorld.h>
#include <BulletSoftBody/btDefaultSoftBodySolver.h>
#include <BulletSoftBody/btSoftBodyHelpers.h>
#include <BulletSoftBody/btSoftBodyRigidBodyCollisionConfiguration.h>
#endif
#ifdef VI_VECTORCLASS
#include "internal/vectorclass.hpp"
#endif
#define V3_TO_BT(v) btVector3(v.x, v.y, v.z)
#define BT_TO_V3(v) vitex::trigonometry::vector3(v.getX(), v.getY(), v.getZ())
#define Q4_TO_BT(v) btQuaternion(v.x, v.y, v.z, v.w)
#define BT_TO_Q4(v) vitex::trigonometry::quaternion(v.getX(), v.getY(), v.getZ(), v.getW())

namespace
{
#ifdef VI_BULLET3
	class find_contacts_handler : public btCollisionWorld::ContactResultCallback
	{
	public:
		int(*callback)(vitex::physics::shape_contact*, const vitex::physics::collision_body&, const vitex::physics::collision_body&) = nullptr;

	public:
		btScalar addSingleResult(btManifoldPoint& point, const btCollisionObjectWrapper* object1, int part_id0, int index0, const btCollisionObjectWrapper* object2, int part_id1, int index1) override
		{
			using namespace vitex::physics;
			using namespace vitex::trigonometry;
			VI_ASSERT(callback && object1 && object1->getCollisionObject() && object2 && object2->getCollisionObject(), "collision objects are not in available condition");

			auto& PWOA = point.getPositionWorldOnA();
			auto& PWOB = point.getPositionWorldOnB();
			shape_contact contact;
			contact.local_point1 = vector3(point.m_localPointA.getX(), point.m_localPointA.getY(), point.m_localPointA.getZ());
			contact.local_point2 = vector3(point.m_localPointB.getX(), point.m_localPointB.getY(), point.m_localPointB.getZ());
			contact.position_world1 = vector3(PWOA.getX(), PWOA.getY(), PWOA.getZ());
			contact.position_world2 = vector3(PWOB.getX(), PWOB.getY(), PWOB.getZ());
			contact.normal_world = vector3(point.m_normalWorldOnB.getX(), point.m_normalWorldOnB.getY(), point.m_normalWorldOnB.getZ());
			contact.lateral_friction_direction1 = vector3(point.m_lateralFrictionDir1.getX(), point.m_lateralFrictionDir1.getY(), point.m_lateralFrictionDir1.getZ());
			contact.lateral_friction_direction2 = vector3(point.m_lateralFrictionDir2.getX(), point.m_lateralFrictionDir2.getY(), point.m_lateralFrictionDir2.getZ());
			contact.distance = point.m_distance1;
			contact.combined_friction = point.m_combinedFriction;
			contact.combined_rolling_friction = point.m_combinedRollingFriction;
			contact.combined_spinning_friction = point.m_combinedSpinningFriction;
			contact.combined_restitution = point.m_combinedRestitution;
			contact.applied_impulse = point.m_appliedImpulse;
			contact.applied_impulse_lateral1 = point.m_appliedImpulseLateral1;
			contact.applied_impulse_lateral2 = point.m_appliedImpulseLateral2;
			contact.contact_motion1 = point.m_contactMotion1;
			contact.contact_motion2 = point.m_contactMotion2;
			contact.contact_cfm = point.m_contactCFM;
			contact.combined_contact_stiffness = point.m_combinedContactStiffness1;
			contact.contact_erp = point.m_contactERP;
			contact.combined_contact_damping = point.m_combinedContactDamping1;
			contact.friction_cfm = point.m_frictionCFM;
			contact.part_id1 = point.m_partId0;
			contact.part_id2 = point.m_partId1;
			contact.index1 = point.m_index0;
			contact.index2 = point.m_index1;
			contact.contact_point_flags = point.m_contactPointFlags;
			contact.life_time = point.m_lifeTime;

			btCollisionObject* body1 = (btCollisionObject*)object1->getCollisionObject();
			btCollisionObject* body2 = (btCollisionObject*)object2->getCollisionObject();
			return (btScalar)callback(&contact, collision_body(body1), collision_body(body2));
		}
	};

	class find_ray_contacts_handler : public btCollisionWorld::RayResultCallback
	{
	public:
		int(*callback)(vitex::physics::ray_contact*, const vitex::physics::collision_body&) = nullptr;

	public:
		btScalar addSingleResult(btCollisionWorld::LocalRayResult& ray_result, bool normal_in_world_space) override
		{
			using namespace vitex::physics;
			using namespace vitex::trigonometry;
			VI_ASSERT(callback && ray_result.m_collisionObject, "collision objects are not in available condition");

			ray_contact contact;
			contact.hit_normal_local = BT_TO_V3(ray_result.m_hitNormalLocal);
			contact.normal_in_world_space = normal_in_world_space;
			contact.hit_fraction = ray_result.m_hitFraction;
			contact.closest_hit_fraction = m_closestHitFraction;

			btCollisionObject* body1 = (btCollisionObject*)ray_result.m_collisionObject;
			return (btScalar)callback(&contact, collision_body(body1));
		}
	};

	btTransform M16_TO_BT(const vitex::trigonometry::matrix4x4& in)
	{
		btMatrix3x3 offset;
		offset[0][0] = in.row[0];
		offset[1][0] = in.row[1];
		offset[2][0] = in.row[2];
		offset[0][1] = in.row[4];
		offset[1][1] = in.row[5];
		offset[2][1] = in.row[6];
		offset[0][2] = in.row[8];
		offset[1][2] = in.row[9];
		offset[2][2] = in.row[10];

		btTransform result;
		result.setBasis(offset);

		vitex::trigonometry::vector3 position = in.position();
		result.setOrigin(V3_TO_BT(position));

		return result;
	}
#endif
	size_t offset_of64(const char* source, char dest)
	{
		VI_ASSERT(source != nullptr, "source should be set");
		for (size_t i = 0; i < 64; i++)
		{
			if (source[i] == dest)
				return i;
		}

		return 63;
	}
	vitex::core::string escape_text(const vitex::core::string& data)
	{
		vitex::core::string result = "\"";
		result.append(data).append("\"");
		return result;
	}
}

namespace vitex
{
	namespace physics
	{
		collision_body::collision_body(btCollisionObject* object) noexcept
		{
#ifdef VI_BULLET3
			btRigidBody* rigid_object = btRigidBody::upcast(object);
			if (rigid_object != nullptr)
				rigid = (rigid_body*)rigid_object->getUserPointer();

			btSoftBody* soft_object = btSoftBody::upcast(object);
			if (soft_object != nullptr)
				soft = (soft_body*)soft_object->getUserPointer();
#endif
		}

		hull_shape::hull_shape(core::vector<trigonometry::vertex>&& new_vertices, core::vector<int>&& new_indices) noexcept : vertices(std::move(new_vertices)), indices(std::move(new_indices)), shape(nullptr)
		{
#ifdef VI_BULLET3
			shape = core::memory::init<btConvexHullShape>();
			btConvexHullShape* hull = (btConvexHullShape*)shape;
			for (auto& item : vertices)
				hull->addPoint(btVector3(item.position_x, item.position_y, item.position_z), false);

			hull->recalcLocalAabb();
			hull->optimizeConvexHull();
			hull->setMargin(0);
#endif
		}
		hull_shape::hull_shape(core::vector<trigonometry::vertex>&& new_vertices) noexcept : vertices(std::move(new_vertices)), shape(nullptr)
		{
#ifdef VI_BULLET3
			shape = core::memory::init<btConvexHullShape>();
			btConvexHullShape* hull = (btConvexHullShape*)shape;
			indices.reserve(vertices.size());

			for (auto& item : vertices)
			{
				hull->addPoint(btVector3(item.position_x, item.position_y, item.position_z), false);
				indices.push_back((int)indices.size());
			}

			hull->recalcLocalAabb();
			hull->optimizeConvexHull();
			hull->setMargin(0);
#endif
		}
		hull_shape::hull_shape(btCollisionShape* from) noexcept : shape(nullptr)
		{
#ifdef VI_BULLET3
			VI_ASSERT(from != nullptr, "shape should be set");
			VI_ASSERT(from->getShapeType() == (int)shape::convex_hull, "shape type should be convex hull");

			btConvexHullShape* hull = core::memory::init<btConvexHullShape>();
			btConvexHullShape* base = (btConvexHullShape*)from;
			vertices.reserve((size_t)base->getNumPoints());
			indices.reserve((size_t)base->getNumPoints());

			for (size_t i = 0; i < (size_t)base->getNumPoints(); i++)
			{
				auto& position = *(base->getUnscaledPoints() + i);
				hull->addPoint(position, false);
				vertices.push_back({ position.x(), position.y(), position.z() });
				indices.push_back((int)i);
			}

			hull->recalcLocalAabb();
			hull->optimizeConvexHull();
			hull->setMargin(0);
#endif
		}
		hull_shape::~hull_shape() noexcept
		{
#ifdef VI_BULLET3
			core::memory::deinit(shape);
#endif
		}
		const core::vector<trigonometry::vertex>& hull_shape::get_vertices() const
		{
			return vertices;
		}
		const core::vector<int>& hull_shape::get_indices() const
		{
			return indices;
		}
		btCollisionShape* hull_shape::get_shape() const
		{
			return shape;
		}

		rigid_body::rigid_body(simulator* refer, const desc& i) noexcept : instance(nullptr), engine(refer), initial(i), user_pointer(nullptr)
		{
			VI_ASSERT(initial.shape, "collision shape should be set");
			VI_ASSERT(engine != nullptr, "simulator should be set");
#ifdef VI_BULLET3
			initial.shape = engine->reuse_shape(initial.shape);
			if (!initial.shape)
			{
				initial.shape = engine->try_clone_shape(i.shape);
				if (!initial.shape)
					return;
			}

			btVector3 local_inertia(0, 0, 0);
			initial.shape->setLocalScaling(V3_TO_BT(initial.scale));
			if (initial.mass > 0)
				initial.shape->calculateLocalInertia(initial.mass, local_inertia);

			btQuaternion rotation;
			rotation.setEulerZYX(initial.rotation.z, initial.rotation.y, initial.rotation.x);

			btTransform bt_transform(rotation, btVector3(initial.position.x, initial.position.y, initial.position.z));
			btRigidBody::btRigidBodyConstructionInfo info(initial.mass, core::memory::init<btDefaultMotionState>(bt_transform), initial.shape, local_inertia);
			instance = core::memory::init<btRigidBody>(info);
			instance->setUserPointer(this);
			instance->setGravity(engine->get_world()->getGravity());

			if (initial.anticipation > 0 && initial.mass > 0)
			{
				instance->setCcdMotionThreshold(initial.anticipation);
				instance->setCcdSweptSphereRadius(initial.scale.length() / 15.0f);
			}

			if (instance->getWorldArrayIndex() == -1)
				engine->get_world()->addRigidBody(instance);
#endif
		}
		rigid_body::~rigid_body() noexcept
		{
#ifdef VI_BULLET3
			if (!instance)
				return;

			int constraints = instance->getNumConstraintRefs();
			for (int i = 0; i < constraints; i++)
			{
				btTypedConstraint* constraint = instance->getConstraintRef(i);
				if (constraint != nullptr)
				{
					void* ptr = constraint->getUserConstraintPtr();
					if (ptr != nullptr)
					{
						btTypedConstraintType type = constraint->getConstraintType();
						switch (type)
						{
							case SLIDER_CONSTRAINT_TYPE:
								if (((sconstraint*)ptr)->first == instance)
									((sconstraint*)ptr)->first = nullptr;
								else if (((sconstraint*)ptr)->second == instance)
									((sconstraint*)ptr)->second = nullptr;
								break;
							default:
								break;
						}
					}

					instance->removeConstraintRef(constraint);
					constraints--; i--;
				}
			}

			if (instance->getMotionState())
			{
				btMotionState* object = instance->getMotionState();
				core::memory::deinit(object);
				instance->setMotionState(nullptr);
			}

			instance->setCollisionShape(nullptr);
			instance->setUserPointer(nullptr);
			if (instance->getWorldArrayIndex() >= 0)
				engine->get_world()->removeRigidBody(instance);

			if (initial.shape)
				engine->free_shape(&initial.shape);

			core::memory::deinit(instance);
#endif
		}
		rigid_body* rigid_body::copy()
		{
			VI_ASSERT(instance != nullptr, "rigidbody should be initialized");

			desc i(initial);
			i.position = get_position();
			i.rotation = get_rotation();
			i.scale = get_scale();
			i.mass = get_mass();
			i.shape = engine->try_clone_shape(i.shape);

			rigid_body* target = new rigid_body(engine, i);
			target->set_spinning_friction(get_spinning_friction());
			target->set_contact_damping(get_contact_damping());
			target->set_contact_stiffness(get_contact_stiffness());
			target->set_activation_state(get_activation_state());
			target->set_angular_damping(get_angular_damping());
			target->set_angular_sleeping_threshold(get_angular_sleeping_threshold());
			target->set_friction(get_friction());
			target->set_restitution(get_restitution());
			target->set_hit_fraction(get_hit_fraction());
			target->set_linear_damping(get_linear_damping());
			target->set_linear_sleeping_threshold(get_linear_sleeping_threshold());
			target->set_ccd_motion_threshold(get_ccd_motion_threshold());
			target->set_ccd_swept_sphere_radius(get_ccd_swept_sphere_radius());
			target->set_contact_processing_threshold(get_contact_processing_threshold());
			target->set_deactivation_time(get_deactivation_time());
			target->set_rolling_friction(get_rolling_friction());
			target->set_angular_factor(get_angular_factor());
			target->set_anisotropic_friction(get_anisotropic_friction());
			target->set_gravity(get_gravity());
			target->set_linear_factor(get_linear_factor());
			target->set_linear_velocity(get_linear_velocity());
			target->set_angular_velocity(get_angular_velocity());
			target->set_collision_flags(get_collision_flags());

			return target;
		}
		void rigid_body::push(const trigonometry::vector3& velocity)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "rigidbody should be initialized");
			instance->applyCentralImpulse(V3_TO_BT(velocity));
#endif
		}
		void rigid_body::push(const trigonometry::vector3& velocity, const trigonometry::vector3& torque)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "rigidbody should be initialized");
			instance->applyCentralImpulse(V3_TO_BT(velocity));
			instance->applyTorqueImpulse(V3_TO_BT(torque));
#endif
		}
		void rigid_body::push(const trigonometry::vector3& velocity, const trigonometry::vector3& torque, const trigonometry::vector3& center)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "rigidbody should be initialized");
			instance->applyImpulse(V3_TO_BT(velocity), V3_TO_BT(center));
			instance->applyTorqueImpulse(V3_TO_BT(torque));
#endif
		}
		void rigid_body::push_kinematic(const trigonometry::vector3& velocity)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "rigidbody should be initialized");

			btTransform offset;
			instance->getMotionState()->getWorldTransform(offset);

			btVector3 origin = offset.getOrigin();
			origin.setX(origin.getX() + velocity.x);
			origin.setY(origin.getY() + velocity.y);
			origin.setZ(origin.getZ() + velocity.z);

			offset.setOrigin(origin);
			instance->getMotionState()->setWorldTransform(offset);
#endif
		}
		void rigid_body::push_kinematic(const trigonometry::vector3& velocity, const trigonometry::vector3& torque)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "rigidbody should be initialized");

			btTransform offset;
			instance->getMotionState()->getWorldTransform(offset);

			btScalar x, y, z;
			offset.getRotation().getEulerZYX(z, y, x);

			trigonometry::vector3 rotation(-x, -y, z);
			offset.getBasis().setEulerZYX(rotation.x + torque.x, rotation.y + torque.y, rotation.z + torque.z);

			btVector3 origin = offset.getOrigin();
			origin.setX(origin.getX() + velocity.x);
			origin.setY(origin.getY() + velocity.y);
			origin.setZ(origin.getZ() + velocity.z);

			offset.setOrigin(origin);
			instance->getMotionState()->setWorldTransform(offset);
#endif
		}
		void rigid_body::synchronize(trigonometry::transform* transform, bool kinematic)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "rigidbody should be initialized");
			btTransform& base = instance->getWorldTransform();
			if (!kinematic)
			{
				btScalar x, y, z;
				const btVector3& position = base.getOrigin();
				const btVector3& scale = instance->getCollisionShape()->getLocalScaling();
				base.getRotation().getEulerZYX(z, y, x);
				transform->set_position(BT_TO_V3(position));
				transform->set_rotation(trigonometry::vector3(x, y, z));
				transform->set_scale(BT_TO_V3(scale));
			}
			else
			{
				trigonometry::transform::spacing& space = transform->get_spacing(trigonometry::positioning::global);
				base.setOrigin(V3_TO_BT(space.position));
				base.getBasis().setEulerZYX(space.rotation.x, space.rotation.y, space.rotation.z);
				instance->getCollisionShape()->setLocalScaling(V3_TO_BT(space.scale));
			}
#endif
		}
		void rigid_body::set_activity(bool active)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "rigidbody should be initialized");
			if (get_activation_state() == motion_state::disable_deactivation)
				return;

			if (active)
			{
				instance->forceActivationState((int)motion_state::active);
				instance->activate(true);
			}
			else
				instance->forceActivationState((int)motion_state::deactivation_needed);
#endif
		}
		void rigid_body::set_as_ghost()
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "rigidbody should be initialized");
			instance->setCollisionFlags(btCollisionObject::CF_NO_CONTACT_RESPONSE);
#endif
		}
		void rigid_body::set_as_normal()
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "rigidbody should be initialized");
			instance->setCollisionFlags(0);
#endif
		}
		void rigid_body::set_self_pointer()
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "rigidbody should be initialized");
			instance->setUserPointer(this);
#endif
		}
		void rigid_body::set_world_transform(btTransform* value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "rigidbody should be initialized");
			VI_ASSERT(value != nullptr, "transform should be set");
			instance->setWorldTransform(*value);
#endif
		}
		void rigid_body::set_activation_state(motion_state value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "rigidbody should be initialized");
			instance->forceActivationState((int)value);
#endif
		}
		void rigid_body::set_angular_damping(float value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "rigidbody should be initialized");
			instance->setDamping(instance->getLinearDamping(), value);
#endif
		}
		void rigid_body::set_angular_sleeping_threshold(float value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "rigidbody should be initialized");
			instance->setSleepingThresholds(instance->getLinearSleepingThreshold(), value);
#endif
		}
		void rigid_body::set_friction(float value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "rigidbody should be initialized");
			instance->setFriction(value);
#endif
		}
		void rigid_body::set_restitution(float value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "rigidbody should be initialized");
			instance->setRestitution(value);
#endif
		}
		void rigid_body::set_spinning_friction(float value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "rigidbody should be initialized");
			instance->setSpinningFriction(value);
#endif
		}
		void rigid_body::set_contact_stiffness(float value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "rigidbody should be initialized");
			instance->setContactStiffnessAndDamping(value, instance->getContactDamping());
#endif
		}
		void rigid_body::set_contact_damping(float value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "rigidbody should be initialized");
			instance->setContactStiffnessAndDamping(instance->getContactStiffness(), value);
#endif
		}
		void rigid_body::set_hit_fraction(float value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "rigidbody should be initialized");
			instance->setHitFraction(value);
#endif
		}
		void rigid_body::set_linear_damping(float value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "rigidbody should be initialized");
			instance->setDamping(value, instance->getAngularDamping());
#endif
		}
		void rigid_body::set_linear_sleeping_threshold(float value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "rigidbody should be initialized");
			instance->setSleepingThresholds(value, instance->getAngularSleepingThreshold());
#endif
		}
		void rigid_body::set_ccd_motion_threshold(float value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "rigidbody should be initialized");
			instance->setCcdMotionThreshold(value);
#endif
		}
		void rigid_body::set_ccd_swept_sphere_radius(float value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "rigidbody should be initialized");
			instance->setCcdSweptSphereRadius(value);
#endif
		}
		void rigid_body::set_contact_processing_threshold(float value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "rigidbody should be initialized");
			instance->setContactProcessingThreshold(value);
#endif
		}
		void rigid_body::set_deactivation_time(float value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "rigidbody should be initialized");
			instance->setDeactivationTime(value);
#endif
		}
		void rigid_body::set_rolling_friction(float value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "rigidbody should be initialized");
			instance->setRollingFriction(value);
#endif
		}
		void rigid_body::set_angular_factor(const trigonometry::vector3& value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "rigidbody should be initialized");
			instance->setAngularFactor(V3_TO_BT(value));
#endif
		}
		void rigid_body::set_anisotropic_friction(const trigonometry::vector3& value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "rigidbody should be initialized");
			instance->setAnisotropicFriction(V3_TO_BT(value));
#endif
		}
		void rigid_body::set_gravity(const trigonometry::vector3& value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "rigidbody should be initialized");
			instance->setGravity(V3_TO_BT(value));
#endif
		}
		void rigid_body::set_linear_factor(const trigonometry::vector3& value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "rigidbody should be initialized");
			instance->setLinearFactor(V3_TO_BT(value));
#endif
		}
		void rigid_body::set_linear_velocity(const trigonometry::vector3& value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "rigidbody should be initialized");
			instance->setLinearVelocity(V3_TO_BT(value));
#endif
		}
		void rigid_body::set_angular_velocity(const trigonometry::vector3& value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "rigidbody should be initialized");
			instance->setAngularVelocity(V3_TO_BT(value));
#endif
		}
		void rigid_body::set_collision_shape(btCollisionShape* shape, trigonometry::transform* transform)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "rigidbody should be initialized");
			btCollisionShape* collision = instance->getCollisionShape();
			core::memory::deinit(collision);

			instance->setCollisionShape(shape);
			if (transform)
				synchronize(transform, true);
#endif
		}
		void rigid_body::set_mass(float mass)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr && engine != nullptr, "rigidbody should be initialized");
			btVector3 inertia = engine->get_world()->getGravity();
			if (instance->getWorldArrayIndex() >= 0)
				engine->get_world()->removeRigidBody(instance);

			instance->setGravity(inertia);
			instance->getCollisionShape()->calculateLocalInertia(mass, inertia);
			instance->setMassProps(mass, inertia);
			if (instance->getWorldArrayIndex() == -1)
				engine->get_world()->addRigidBody(instance);

			set_activity(true);
#endif
		}
		void rigid_body::set_collision_flags(size_t flags)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "rigidbody should be initialized");
			instance->setCollisionFlags((int)flags);
#endif
		}
		motion_state rigid_body::get_activation_state() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "rigidbody should be initialized");
			return (motion_state)instance->getActivationState();
#else
			return motion_state::island_sleeping;
#endif
		}
		shape rigid_body::get_collision_shape_type() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr && instance->getCollisionShape() != nullptr, "rigidbody should be initialized");
			return (shape)instance->getCollisionShape()->getShapeType();
#else
			return shape::invalid;
#endif
		}
		trigonometry::vector3 rigid_body::get_angular_factor() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "rigidbody should be initialized");
			btVector3 value = instance->getAngularFactor();
			return trigonometry::vector3(value.getX(), value.getY(), value.getZ());
#else
			return 0;
#endif
		}
		trigonometry::vector3 rigid_body::get_anisotropic_friction() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "rigidbody should be initialized");
			btVector3 value = instance->getAnisotropicFriction();
			return trigonometry::vector3(value.getX(), value.getY(), value.getZ());
#else
			return 0;
#endif
		}
		trigonometry::vector3 rigid_body::get_gravity() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "rigidbody should be initialized");
			btVector3 value = instance->getGravity();
			return trigonometry::vector3(value.getX(), value.getY(), value.getZ());
#else
			return 0;
#endif
		}
		trigonometry::vector3 rigid_body::get_linear_factor() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "rigidbody should be initialized");
			btVector3 value = instance->getLinearFactor();
			return trigonometry::vector3(value.getX(), value.getY(), value.getZ());
#else
			return 0;
#endif
		}
		trigonometry::vector3 rigid_body::get_linear_velocity() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "rigidbody should be initialized");
			btVector3 value = instance->getLinearVelocity();
			return trigonometry::vector3(value.getX(), value.getY(), value.getZ());
#else
			return 0;
#endif
		}
		trigonometry::vector3 rigid_body::get_angular_velocity() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "rigidbody should be initialized");
			btVector3 value = instance->getAngularVelocity();
			return trigonometry::vector3(value.getX(), value.getY(), value.getZ());
#else
			return 0;
#endif
		}
		trigonometry::vector3 rigid_body::get_scale() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr && instance->getCollisionShape() != nullptr, "rigidbody should be initialized");
			btVector3 value = instance->getCollisionShape()->getLocalScaling();
			return trigonometry::vector3(value.getX(), value.getY(), value.getZ());
#else
			return 0;
#endif
		}
		trigonometry::vector3 rigid_body::get_position() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "rigidbody should be initialized");
			btVector3 value = instance->getWorldTransform().getOrigin();
			return trigonometry::vector3(value.getX(), value.getY(), value.getZ());
#else
			return 0;
#endif
		}
		trigonometry::vector3 rigid_body::get_rotation() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "rigidbody should be initialized");
			btScalar x, y, z;
			instance->getWorldTransform().getBasis().getEulerZYX(z, y, x);
			return trigonometry::vector3(-x, -y, z);
#else
			return 0;
#endif
		}
		btTransform* rigid_body::get_world_transform() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "rigidbody should be initialized");
			return &instance->getWorldTransform();
#else
			return nullptr;
#endif
		}
		btCollisionShape* rigid_body::get_collision_shape() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "rigidbody should be initialized");
			return instance->getCollisionShape();
#else
			return nullptr;
#endif
		}
		btRigidBody* rigid_body::get() const
		{
#ifdef VI_BULLET3
			return instance;
#else
			return nullptr;
#endif
		}
		bool rigid_body::is_ghost() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "rigidbody should be initialized");
			return (instance->getCollisionFlags() & btCollisionObject::CF_NO_CONTACT_RESPONSE) != 0;
#else
			return false;
#endif
		}
		bool rigid_body::is_active() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "rigidbody should be initialized");
			return instance->isActive();
#else
			return false;
#endif
		}
		bool rigid_body::is_static() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "rigidbody should be initialized");
			return instance->isStaticObject();
#else
			return true;
#endif
		}
		bool rigid_body::is_colliding() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "rigidbody should be initialized");
			return instance->hasContactResponse();
#else
			return false;
#endif
		}
		float rigid_body::get_spinning_friction() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "rigidbody should be initialized");
			return instance->getSpinningFriction();
#else
			return 0;
#endif
		}
		float rigid_body::get_contact_stiffness() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "rigidbody should be initialized");
			return instance->getContactStiffness();
#else
			return 0;
#endif
		}
		float rigid_body::get_contact_damping() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "rigidbody should be initialized");
			return instance->getContactDamping();
#else
			return 0;
#endif
		}
		float rigid_body::get_angular_damping() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "rigidbody should be initialized");
			return instance->getAngularDamping();
#else
			return 0;
#endif
		}
		float rigid_body::get_angular_sleeping_threshold() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "rigidbody should be initialized");
			return instance->getAngularSleepingThreshold();
#else
			return 0;
#endif
		}
		float rigid_body::get_friction() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "rigidbody should be initialized");
			return instance->getFriction();
#else
			return 0;
#endif
		}
		float rigid_body::get_restitution() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "rigidbody should be initialized");
			return instance->getRestitution();
#else
			return 0;
#endif
		}
		float rigid_body::get_hit_fraction() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "rigidbody should be initialized");
			return instance->getHitFraction();
#else
			return 0;
#endif
		}
		float rigid_body::get_linear_damping() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "rigidbody should be initialized");
			return instance->getLinearDamping();
#else
			return 0;
#endif
		}
		float rigid_body::get_linear_sleeping_threshold() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "rigidbody should be initialized");
			return instance->getLinearSleepingThreshold();
#else
			return 0;
#endif
		}
		float rigid_body::get_ccd_motion_threshold() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "rigidbody should be initialized");
			return instance->getCcdMotionThreshold();
#else
			return 0;
#endif
		}
		float rigid_body::get_ccd_swept_sphere_radius() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "rigidbody should be initialized");
			return instance->getCcdSweptSphereRadius();
#else
			return 0;
#endif
		}
		float rigid_body::get_contact_processing_threshold() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "rigidbody should be initialized");
			return instance->getContactProcessingThreshold();
#else
			return 0;
#endif
		}
		float rigid_body::get_deactivation_time() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "rigidbody should be initialized");
			return instance->getDeactivationTime();
#else
			return 0;
#endif
		}
		float rigid_body::get_rolling_friction() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "rigidbody should be initialized");
			return instance->getRollingFriction();
#else
			return 0;
#endif
		}
		float rigid_body::get_mass() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "rigidbody should be initialized");
			float mass = instance->getInvMass();
			return (mass != 0.0f ? 1.0f / mass : 0.0f);
#else
			return 0;
#endif
		}
		size_t rigid_body::get_collision_flags() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "rigidbody should be initialized");
			return instance->getCollisionFlags();
#else
			return 0;
#endif
		}
		rigid_body::desc& rigid_body::get_initial_state()
		{
			return initial;
		}
		simulator* rigid_body::get_simulator() const
		{
			return engine;
		}
		rigid_body* rigid_body::get(btRigidBody* from)
		{
#ifdef VI_BULLET3
			VI_ASSERT(from != nullptr, "rigidbody should be initialized");
			return (rigid_body*)from->getUserPointer();
#else
			return nullptr;
#endif
		}

		soft_body::soft_body(simulator* refer, const desc& i) noexcept : instance(nullptr), engine(refer), initial(i), user_pointer(nullptr)
		{
#ifdef VI_BULLET3
			VI_ASSERT(engine != nullptr, "engine should be set");
			VI_ASSERT(engine->has_soft_body_support(), "soft body should be supported");

			btQuaternion rotation;
			rotation.setEulerZYX(initial.rotation.z, initial.rotation.y, initial.rotation.x);

			btTransform bt_transform(rotation, btVector3(initial.position.x, initial.position.y, -initial.position.z));
			btSoftRigidDynamicsWorld* world = (btSoftRigidDynamicsWorld*)engine->get_world();
			btSoftBodyWorldInfo& info = world->getWorldInfo();
			hull_shape* hull = initial.shape.convex.hull;

			if (initial.shape.convex.enabled && hull != nullptr)
			{
				auto& positions = hull->get_vertices();
				core::vector<btScalar> vertices;
				vertices.resize(positions.size() * 3);

				for (size_t i = 0; i < hull->get_vertices().size(); i++)
				{
					const trigonometry::vertex& v = positions[i];
					vertices[i * 3 + 0] = (btScalar)v.position_x;
					vertices[i * 3 + 1] = (btScalar)v.position_y;
					vertices[i * 3 + 2] = (btScalar)v.position_z;
				}

				auto& indices = hull->get_indices();
				instance = btSoftBodyHelpers::CreateFromTriMesh(info, vertices.data(), indices.data(), (int)indices.size() / 3, false);
			}
			else if (initial.shape.ellipsoid.enabled)
			{
				instance = btSoftBodyHelpers::CreateEllipsoid(info, V3_TO_BT(initial.shape.ellipsoid.center), V3_TO_BT(initial.shape.ellipsoid.radius), initial.shape.ellipsoid.count);
			}
			else if (initial.shape.rope.enabled)
			{
				int fixed_anchors = 0;
				if (initial.shape.rope.start_fixed)
					fixed_anchors |= 1;

				if (initial.shape.rope.end_fixed)
					fixed_anchors |= 2;

				instance = btSoftBodyHelpers::CreateRope(info, V3_TO_BT(initial.shape.rope.start), V3_TO_BT(initial.shape.rope.end), initial.shape.rope.count, fixed_anchors);
			}
			else
			{
				int fixed_corners = 0;
				if (initial.shape.patch.corner00_fixed)
					fixed_corners |= 1;

				if (initial.shape.patch.corner01_fixed)
					fixed_corners |= 2;

				if (initial.shape.patch.corner10_fixed)
					fixed_corners |= 4;

				if (initial.shape.patch.corner11_fixed)
					fixed_corners |= 8;

				instance = btSoftBodyHelpers::CreatePatch(info, V3_TO_BT(initial.shape.patch.corner00), V3_TO_BT(initial.shape.patch.corner10), V3_TO_BT(initial.shape.patch.corner01), V3_TO_BT(initial.shape.patch.corner11), initial.shape.patch.count_x, initial.shape.patch.count_y, fixed_corners, initial.shape.patch.generate_diagonals);
			}

			if (initial.anticipation > 0)
			{
				instance->setCcdMotionThreshold(initial.anticipation);
				instance->setCcdSweptSphereRadius(initial.scale.length() / 15.0f);
			}

			set_config(initial.config);
			instance->randomizeConstraints();
			instance->setPose(true, true);
			instance->getCollisionShape()->setMargin(0.04f);
			instance->transform(bt_transform);
			instance->setUserPointer(this);
			instance->setTotalMass(100.0f, true);

			if (instance->getWorldArrayIndex() == -1)
				world->addSoftBody(instance);
#endif
		}
		soft_body::~soft_body() noexcept
		{
#ifdef VI_BULLET3
			if (!instance)
				return;

			btSoftRigidDynamicsWorld* world = (btSoftRigidDynamicsWorld*)engine->get_world();
			if (instance->getWorldArrayIndex() >= 0)
				world->removeSoftBody(instance);

			instance->setUserPointer(nullptr);
			core::memory::deinit(instance);
#endif
		}
		soft_body* soft_body::copy()
		{
			VI_ASSERT(instance != nullptr, "softbody should be initialized");

			desc i(initial);
			i.position = get_center_position();
			i.rotation = get_rotation();
			i.scale = get_scale();

			soft_body* target = new soft_body(engine, i);
			target->set_spinning_friction(get_spinning_friction());
			target->set_contact_damping(get_contact_damping());
			target->set_contact_stiffness(get_contact_stiffness());
			target->set_activation_state(get_activation_state());
			target->set_friction(get_friction());
			target->set_restitution(get_restitution());
			target->set_hit_fraction(get_hit_fraction());
			target->set_ccd_motion_threshold(get_ccd_motion_threshold());
			target->set_ccd_swept_sphere_radius(get_ccd_swept_sphere_radius());
			target->set_contact_processing_threshold(get_contact_processing_threshold());
			target->set_deactivation_time(get_deactivation_time());
			target->set_rolling_friction(get_rolling_friction());
			target->set_anisotropic_friction(get_anisotropic_friction());
			target->set_wind_velocity(get_wind_velocity());
			target->set_contact_stiffness_and_damping(get_contact_stiffness(), get_contact_damping());
			target->set_total_mass(get_total_mass());
			target->set_rest_length_scale(get_rest_length_scale());
			target->set_velocity(get_linear_velocity());

			return target;
		}
		void soft_body::activate(bool force)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "softbody should be initialized");
			instance->activate(force);
#endif
		}
		void soft_body::synchronize(trigonometry::transform* transform, bool kinematic)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "softbody should be initialized");
			VI_ASSERT(transform != nullptr, "transform should be set");
#ifdef VI_VECTORCLASS
			LOAD_VAL(_r1, 0.0f); LOAD_VAL(_r2, 0.0f);
			for (int i = 0; i < instance->m_nodes.size(); i++)
			{
				auto& node = instance->m_nodes[i];
				_r2.store(node.m_x.m_floats);
				_r1 += _r2;
			}

			_r1 /= (float)instance->m_nodes.size();
			_r1.store_partial(3, (float*)&center);
#else
			center.set(0);
			for (int i = 0; i < instance->m_nodes.size(); i++)
			{
				auto& node = instance->m_nodes[i];
				center.x += node.m_x.x();
				center.y += node.m_x.y();
				center.z += node.m_x.z();
			}
			center /= (float)instance->m_nodes.size();
#endif
			if (!kinematic)
			{
				btScalar x, y, z;
				instance->getWorldTransform().getRotation().getEulerZYX(z, y, x);
				transform->set_position(center.inv_z());
				transform->set_rotation(trigonometry::vector3(x, y, z));
			}
			else
			{
				trigonometry::vector3 position = transform->get_position().inv_z() - center;
				if (position.length() > 0.005f)
					instance->translate(V3_TO_BT(position));
			}
#endif
		}
		void soft_body::get_indices(core::vector<int>* result) const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "softbody should be initialized");
			VI_ASSERT(result != nullptr, "result should be set");

			core::unordered_map<btSoftBody::Node*, int> nodes;
			for (int i = 0; i < instance->m_nodes.size(); i++)
				nodes.insert(std::make_pair(&instance->m_nodes[i], i));

			for (int i = 0; i < instance->m_faces.size(); i++)
			{
				btSoftBody::Face& face = instance->m_faces[i];
				for (uint32_t j = 0; j < 3; j++)
				{
					auto it = nodes.find(face.m_n[j]);
					if (it != nodes.end())
						result->push_back(it->second);
				}
			}
#endif
		}
		void soft_body::get_vertices(core::vector<trigonometry::vertex>* result) const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "softbody should be initialized");
			VI_ASSERT(result != nullptr, "result should be set");

			static size_t position_x = offsetof(trigonometry::vertex, position_x);
			static size_t normal_x = offsetof(trigonometry::vertex, normal_x);

			size_t size = (size_t)instance->m_nodes.size();
			if (result->size() != size)
			{
				if (initial.shape.convex.enabled)
					*result = initial.shape.convex.hull->get_vertices();
				else
					result->resize(size);
			}

			for (size_t i = 0; i < size; i++)
			{
				auto* node = &instance->m_nodes[(int)i]; trigonometry::vertex& position = result->at(i);
				memcpy(&position + position_x, node->m_x.m_floats, sizeof(float) * 3);
				memcpy(&position + normal_x, node->m_n.m_floats, sizeof(float) * 3);
			}
#endif
		}
		void soft_body::get_bounding_box(trigonometry::vector3* min, trigonometry::vector3* max) const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "softbody should be initialized");

			btVector3 bMin, bMax;
			instance->getAabb(bMin, bMax);
			if (min != nullptr)
				*min = BT_TO_V3(bMin).inv_z();

			if (max != nullptr)
				*max = BT_TO_V3(bMax).inv_z();
#endif
		}
		void soft_body::set_contact_stiffness_and_damping(float stiffness, float damping)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "softbody should be initialized");
			instance->setContactStiffnessAndDamping(stiffness, damping);
#endif
		}
		void soft_body::add_anchor(int node, rigid_body* body, bool disable_collision_between_linked_bodies, float influence)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "softbody should be initialized");
			VI_ASSERT(body != nullptr, "body should be set");
			instance->appendAnchor(node, body->get(), disable_collision_between_linked_bodies, influence);
#endif
		}
		void soft_body::add_anchor(int node, rigid_body* body, const trigonometry::vector3& local_pivot, bool disable_collision_between_linked_bodies, float influence)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "softbody should be initialized");
			VI_ASSERT(body != nullptr, "body should be set");
			instance->appendAnchor(node, body->get(), V3_TO_BT(local_pivot), disable_collision_between_linked_bodies, influence);
#endif
		}
		void soft_body::add_force(const trigonometry::vector3& force)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "softbody should be initialized");
			instance->addForce(V3_TO_BT(force));
#endif
		}
		void soft_body::add_force(const trigonometry::vector3& force, int node)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "softbody should be initialized");
			instance->addForce(V3_TO_BT(force), node);
#endif
		}
		void soft_body::add_aero_force_to_node(const trigonometry::vector3& wind_velocity, int node_index)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "softbody should be initialized");
			instance->addAeroForceToNode(V3_TO_BT(wind_velocity), node_index);
#endif
		}
		void soft_body::add_aero_force_to_face(const trigonometry::vector3& wind_velocity, int face_index)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "softbody should be initialized");
			instance->addAeroForceToFace(V3_TO_BT(wind_velocity), face_index);
#endif
		}
		void soft_body::add_velocity(const trigonometry::vector3& velocity)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "softbody should be initialized");
			instance->addVelocity(V3_TO_BT(velocity));
#endif
		}
		void soft_body::set_velocity(const trigonometry::vector3& velocity)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "softbody should be initialized");
			instance->setVelocity(V3_TO_BT(velocity));
#endif
		}
		void soft_body::add_velocity(const trigonometry::vector3& velocity, int node)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "softbody should be initialized");
			instance->addVelocity(V3_TO_BT(velocity), node);
#endif
		}
		void soft_body::set_mass(int node, float mass)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "softbody should be initialized");
			instance->setMass(node, mass);
#endif
		}
		void soft_body::set_total_mass(float mass, bool from_faces)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "softbody should be initialized");
			instance->setTotalMass(mass, from_faces);
#endif
		}
		void soft_body::set_total_density(float density)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "softbody should be initialized");
			instance->setTotalDensity(density);
#endif
		}
		void soft_body::set_volume_mass(float mass)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "softbody should be initialized");
			instance->setVolumeMass(mass);
#endif
		}
		void soft_body::set_volume_density(float density)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "softbody should be initialized");
			instance->setVolumeDensity(density);
#endif
		}
		void soft_body::translate(const trigonometry::vector3& position)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "softbody should be initialized");
			instance->translate(btVector3(position.x, position.y, -position.z));
#endif
		}
		void soft_body::rotate(const trigonometry::vector3& rotation)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "softbody should be initialized");
			btQuaternion value;
			value.setEulerZYX(rotation.x, rotation.y, rotation.z);
			instance->rotate(value);
#endif
		}
		void soft_body::scale(const trigonometry::vector3& scale)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "softbody should be initialized");
			instance->scale(V3_TO_BT(scale));
#endif
		}
		void soft_body::set_rest_length_scale(float rest_length)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "softbody should be initialized");
			instance->setRestLengthScale(rest_length);
#endif
		}
		void soft_body::set_pose(bool volume, bool frame)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "softbody should be initialized");
			instance->setPose(volume, frame);
#endif
		}
		float soft_body::get_mass(int node) const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "softbody should be initialized");
			return instance->getMass(node);
#else
			return 0;
#endif
		}
		float soft_body::get_total_mass() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "softbody should be initialized");
			return instance->getTotalMass();
#else
			return 0;
#endif
		}
		float soft_body::get_rest_length_scale() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "softbody should be initialized");
			return instance->getRestLengthScale();
#else
			return 0;
#endif
		}
		float soft_body::get_volume() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "softbody should be initialized");
			return instance->getVolume();
#else
			return 0;
#endif
		}
		int soft_body::generate_bending_constraints(int distance)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "softbody should be initialized");
			return instance->generateBendingConstraints(distance);
#else
			return 0;
#endif
		}
		void soft_body::randomize_constraints()
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "softbody should be initialized");
			instance->randomizeConstraints();
#endif
		}
		bool soft_body::cut_link(int node0, int node1, float position)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "softbody should be initialized");
			return instance->cutLink(node0, node1, position);
#else
			return false;
#endif
		}
		bool soft_body::ray_test(const trigonometry::vector3& from, const trigonometry::vector3& to, ray_cast& result)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "softbody should be initialized");
			btSoftBody::sRayCast cast;
			bool r = instance->rayTest(V3_TO_BT(from), V3_TO_BT(to), cast);
			result.body = get(cast.body);
			result.feature = (soft_feature)cast.feature;
			result.index = cast.index;
			result.fraction = cast.fraction;

			return r;
#else
			return false;
#endif
		}
		void soft_body::set_wind_velocity(const trigonometry::vector3& velocity)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "softbody should be initialized");
			instance->setWindVelocity(V3_TO_BT(velocity));
#endif
		}
		trigonometry::vector3 soft_body::get_wind_velocity() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "softbody should be initialized");
			btVector3 value = instance->getWindVelocity();
			return BT_TO_V3(value);
#else
			return 0;
#endif
		}
		void soft_body::get_aabb(trigonometry::vector3& min, trigonometry::vector3& max) const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "softbody should be initialized");
			btVector3 bmin, bmax;
			instance->getAabb(bmin, bmax);
			min = BT_TO_V3(bmin);
			max = BT_TO_V3(bmax);
#endif
		}
		void soft_body::indices_to_pointers(const int* map)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "softbody should be initialized");
			VI_ASSERT(map != nullptr, "map should be set");
			instance->indicesToPointers(map);
#endif
		}
		void soft_body::set_spinning_friction(float value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "softbody should be initialized");
			instance->setSpinningFriction(value);
#endif
		}
		trigonometry::vector3 soft_body::get_linear_velocity() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "softbody should be initialized");
			btVector3 value = instance->getInterpolationLinearVelocity();
			return trigonometry::vector3(value.getX(), value.getY(), value.getZ());
#else
			return 0;
#endif
		}
		trigonometry::vector3 soft_body::get_angular_velocity() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "softbody should be initialized");
			btVector3 value = instance->getInterpolationAngularVelocity();
			return trigonometry::vector3(value.getX(), value.getY(), value.getZ());
#else
			return 0;
#endif
		}
		trigonometry::vector3 soft_body::get_center_position() const
		{
#ifdef VI_BULLET3
			return center;
#else
			return 0;
#endif
		}
		void soft_body::set_activity(bool active)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "softbody should be initialized");
			if (get_activation_state() == motion_state::disable_deactivation)
				return;

			if (active)
				instance->forceActivationState((int)motion_state::active);
			else
				instance->forceActivationState((int)motion_state::deactivation_needed);
#endif
		}
		void soft_body::set_as_ghost()
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "softbody should be initialized");
			instance->setCollisionFlags(btCollisionObject::CF_NO_CONTACT_RESPONSE);
#endif
		}
		void soft_body::set_as_normal()
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "softbody should be initialized");
			instance->setCollisionFlags(0);
#endif
		}
		void soft_body::set_self_pointer()
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "softbody should be initialized");
			instance->setUserPointer(this);
#endif
		}
		void soft_body::set_world_transform(btTransform* value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "softbody should be initialized");
			VI_ASSERT(value != nullptr, "transform should be set");
			instance->setWorldTransform(*value);
#endif
		}
		void soft_body::set_activation_state(motion_state value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "softbody should be initialized");
			instance->forceActivationState((int)value);
#endif
		}
		void soft_body::set_friction(float value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "softbody should be initialized");
			instance->setFriction(value);
#endif
		}
		void soft_body::set_restitution(float value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "softbody should be initialized");
			instance->setRestitution(value);
#endif
		}
		void soft_body::set_contact_stiffness(float value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "softbody should be initialized");
			instance->setContactStiffnessAndDamping(value, instance->getContactDamping());
#endif
		}
		void soft_body::set_contact_damping(float value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "softbody should be initialized");
			instance->setContactStiffnessAndDamping(instance->getContactStiffness(), value);
#endif
		}
		void soft_body::set_hit_fraction(float value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "softbody should be initialized");
			instance->setHitFraction(value);
#endif
		}
		void soft_body::set_ccd_motion_threshold(float value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "softbody should be initialized");
			instance->setCcdMotionThreshold(value);
#endif
		}
		void soft_body::set_ccd_swept_sphere_radius(float value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "softbody should be initialized");
			instance->setCcdSweptSphereRadius(value);
#endif
		}
		void soft_body::set_contact_processing_threshold(float value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "softbody should be initialized");
			instance->setContactProcessingThreshold(value);
#endif
		}
		void soft_body::set_deactivation_time(float value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "softbody should be initialized");
			instance->setDeactivationTime(value);
#endif
		}
		void soft_body::set_rolling_friction(float value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "softbody should be initialized");
			instance->setRollingFriction(value);
#endif
		}
		void soft_body::set_anisotropic_friction(const trigonometry::vector3& value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "softbody should be initialized");
			instance->setAnisotropicFriction(V3_TO_BT(value));
#endif
		}
		void soft_body::set_config(const desc::sconfig& conf)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "softbody should be initialized");
			initial.config = conf;
			instance->m_cfg.aeromodel = (btSoftBody::eAeroModel::_)initial.config.aero_model;
			instance->m_cfg.kVCF = initial.config.vcf;
			instance->m_cfg.kDP = initial.config.dp;
			instance->m_cfg.kDG = initial.config.dg;
			instance->m_cfg.kLF = initial.config.lf;
			instance->m_cfg.kPR = initial.config.pr;
			instance->m_cfg.kVC = initial.config.vc;
			instance->m_cfg.kDF = initial.config.df;
			instance->m_cfg.kMT = initial.config.mt;
			instance->m_cfg.kCHR = initial.config.chr;
			instance->m_cfg.kKHR = initial.config.khr;
			instance->m_cfg.kSHR = initial.config.shr;
			instance->m_cfg.kAHR = initial.config.ahr;
			instance->m_cfg.kSRHR_CL = initial.config.srhr_cl;
			instance->m_cfg.kSKHR_CL = initial.config.skhr_cl;
			instance->m_cfg.kSSHR_CL = initial.config.sshr_cl;
			instance->m_cfg.kSR_SPLT_CL = initial.config.sr_splt_cl;
			instance->m_cfg.kSK_SPLT_CL = initial.config.sk_splt_cl;
			instance->m_cfg.kSS_SPLT_CL = initial.config.ss_splt_cl;
			instance->m_cfg.maxvolume = initial.config.max_volume;
			instance->m_cfg.timescale = initial.config.time_scale;
			instance->m_cfg.viterations = initial.config.viterations;
			instance->m_cfg.piterations = initial.config.piterations;
			instance->m_cfg.diterations = initial.config.diterations;
			instance->m_cfg.citerations = initial.config.citerations;
			instance->m_cfg.collisions = initial.config.collisions;
			instance->m_cfg.m_maxStress = initial.config.max_stress;
			instance->m_cfg.drag = initial.config.drag;

			if (initial.config.constraints > 0)
				instance->generateBendingConstraints(initial.config.constraints);

			if (initial.config.clusters > 0)
				instance->generateClusters(initial.config.clusters);
#endif
		}
		motion_state soft_body::get_activation_state() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "softbody should be initialized");
			return (motion_state)instance->getActivationState();
#else
			return motion_state::island_sleeping;
#endif
		}
		shape soft_body::get_collision_shape_type() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "softbody should be initialized");
			if (!initial.shape.convex.enabled)
				return shape::invalid;

			return shape::convex_hull;
#else
			return shape::invalid;
#endif
		}
		trigonometry::vector3 soft_body::get_anisotropic_friction() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "softbody should be initialized");
			btVector3 value = instance->getAnisotropicFriction();
			return trigonometry::vector3(value.getX(), value.getY(), value.getZ());
#else
			return 0;
#endif
		}
		trigonometry::vector3 soft_body::get_scale() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "softbody should be initialized");
			btVector3 bMin, bMax;
			instance->getAabb(bMin, bMax);
			btVector3 bScale = bMax - bMin;
			trigonometry::vector3 scale = BT_TO_V3(bScale);

			return scale.div(2.0f).abs();
#else
			return 0;
#endif
		}
		trigonometry::vector3 soft_body::get_position() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "softbody should be initialized");
			btVector3 value = instance->getWorldTransform().getOrigin();
			return trigonometry::vector3(value.getX(), value.getY(), value.getZ());
#else
			return 0;
#endif
		}
		trigonometry::vector3 soft_body::get_rotation() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "softbody should be initialized");
			btScalar x, y, z;
			instance->getWorldTransform().getBasis().getEulerZYX(z, y, x);
			return trigonometry::vector3(-x, -y, z);
#else
			return 0;
#endif
		}
		btTransform* soft_body::get_world_transform() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "softbody should be initialized");
			return &instance->getWorldTransform();
#else
			return nullptr;
#endif
		}
		btSoftBody* soft_body::get() const
		{
#ifdef VI_BULLET3
			return instance;
#else
			return nullptr;
#endif
		}
		bool soft_body::is_ghost() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "softbody should be initialized");
			return (instance->getCollisionFlags() & btCollisionObject::CF_NO_CONTACT_RESPONSE) != 0;
#else
			return false;
#endif
		}
		bool soft_body::is_active() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "softbody should be initialized");
			return instance->isActive();
#else
			return false;
#endif
		}
		bool soft_body::is_static() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "softbody should be initialized");
			return instance->isStaticObject();
#else
			return true;
#endif
		}
		bool soft_body::is_colliding() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "softbody should be initialized");
			return instance->hasContactResponse();
#else
			return false;
#endif
		}
		float soft_body::get_spinning_friction() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "softbody should be initialized");
			return instance->getSpinningFriction();
#else
			return 0;
#endif
		}
		float soft_body::get_contact_stiffness() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "softbody should be initialized");
			return instance->getContactStiffness();
#else
			return 0;
#endif
		}
		float soft_body::get_contact_damping() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "softbody should be initialized");
			return instance->getContactDamping();
#else
			return 0;
#endif
		}
		float soft_body::get_friction() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "softbody should be initialized");
			return instance->getFriction();
#else
			return 0;
#endif
		}
		float soft_body::get_restitution() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "softbody should be initialized");
			return instance->getRestitution();
#else
			return 0;
#endif
		}
		float soft_body::get_hit_fraction() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "softbody should be initialized");
			return instance->getHitFraction();
#else
			return 0;
#endif
		}
		float soft_body::get_ccd_motion_threshold() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "softbody should be initialized");
			return instance->getCcdMotionThreshold();
#else
			return 0;
#endif
		}
		float soft_body::get_ccd_swept_sphere_radius() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "softbody should be initialized");
			return instance->getCcdSweptSphereRadius();
#else
			return 0;
#endif
		}
		float soft_body::get_contact_processing_threshold() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "softbody should be initialized");
			return instance->getContactProcessingThreshold();
#else
			return 0;
#endif
		}
		float soft_body::get_deactivation_time() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "softbody should be initialized");
			return instance->getDeactivationTime();
#else
			return 0;
#endif
		}
		float soft_body::get_rolling_friction() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "softbody should be initialized");
			return instance->getRollingFriction();
#else
			return 0;
#endif
		}
		size_t soft_body::get_collision_flags() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "softbody should be initialized");
			return instance->getCollisionFlags();
#else
			return 0;
#endif
		}
		size_t soft_body::get_vertices_count() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "softbody should be initialized");
			return instance->m_nodes.size();
#else
			return 0;
#endif
		}
		soft_body::desc& soft_body::get_initial_state()
		{
			return initial;
		}
		simulator* soft_body::get_simulator() const
		{
#ifdef VI_BULLET3
			return engine;
#else
			return nullptr;
#endif
		}
		soft_body* soft_body::get(btSoftBody* from)
		{
#ifdef VI_BULLET3
			VI_ASSERT(from != nullptr, "softbody should be set");
			return (soft_body*)from->getUserPointer();
#else
			return nullptr;
#endif
		}

		constraint::constraint(simulator* refer) noexcept : first(nullptr), second(nullptr), engine(refer), user_pointer(nullptr)
		{
		}
		void constraint::set_breaking_impulse_threshold(float value)
		{
#ifdef VI_BULLET3
			btTypedConstraint* base = get();
			VI_ASSERT(base != nullptr, "typed constraint should be initialized");
			base->setBreakingImpulseThreshold(value);
#endif
		}
		void constraint::set_enabled(bool value)
		{
#ifdef VI_BULLET3
			btTypedConstraint* base = get();
			VI_ASSERT(base != nullptr, "typed constraint should be initialized");
			base->setEnabled(value);
#endif
		}
		btRigidBody* constraint::get_first() const
		{
#ifdef VI_BULLET3
			return first;
#else
			return nullptr;
#endif
		}
		btRigidBody* constraint::get_second() const
		{
#ifdef VI_BULLET3
			return second;
#else
			return nullptr;
#endif
		}
		float constraint::get_breaking_impulse_threshold() const
		{
#ifdef VI_BULLET3
			btTypedConstraint* base = get();
			VI_ASSERT(base != nullptr, "typed constraint should be initialized");
			return base->getBreakingImpulseThreshold();
#else
			return 0;
#endif
		}
		bool constraint::is_active() const
		{
#ifdef VI_BULLET3
			btTypedConstraint* base = get();
			if (!base || !first || !second)
				return false;

			if (first != nullptr)
			{
				for (int i = 0; i < first->getNumConstraintRefs(); i++)
				{
					if (first->getConstraintRef(i) == base)
						return true;
				}
			}

			if (second != nullptr)
			{
				for (int i = 0; i < second->getNumConstraintRefs(); i++)
				{
					if (second->getConstraintRef(i) == base)
						return true;
				}
			}

			return false;
#else
			return false;
#endif
		}
		bool constraint::is_enabled() const
		{
#ifdef VI_BULLET3
			btTypedConstraint* base = get();
			VI_ASSERT(base != nullptr, "typed constraint should be initialized");
			return base->isEnabled();
#else
			return false;
#endif
		}
		simulator* constraint::get_simulator() const
		{
			return engine;
		}

		pconstraint::pconstraint(simulator* refer, const desc& i) noexcept : constraint(refer), instance(nullptr), state(i)
		{
#ifdef VI_BULLET3
			VI_ASSERT(i.target_a != nullptr, "target a rigidbody should be set");
			VI_ASSERT(engine != nullptr, "simulator should be set");

			first = i.target_a->get();
			second = (i.target_b ? i.target_b->get() : nullptr);

			if (second != nullptr)
				instance = core::memory::init<btPoint2PointConstraint>(*first, *second, V3_TO_BT(i.pivot_a), V3_TO_BT(i.pivot_b));
			else
				instance = core::memory::init<btPoint2PointConstraint>(*first, V3_TO_BT(i.pivot_a));

			instance->setUserConstraintPtr(this);
			engine->add_constraint(this);
#endif
		}
		pconstraint::~pconstraint() noexcept
		{
#ifdef VI_BULLET3
			engine->remove_constraint(this);
			core::memory::deinit(instance);
#endif
		}
		constraint* pconstraint::copy() const
		{
			VI_ASSERT(instance != nullptr, "p2p constraint should be initialized");
			pconstraint* target = new pconstraint(engine, state);
			target->set_breaking_impulse_threshold(get_breaking_impulse_threshold());
			target->set_enabled(is_enabled());
			target->set_pivot_a(get_pivot_a());
			target->set_pivot_b(get_pivot_b());

			return target;
		}
		btTypedConstraint* pconstraint::get() const
		{
#ifdef VI_BULLET3
			return instance;
#else
			return nullptr;
#endif
		}
		bool pconstraint::has_collisions() const
		{
			return state.collisions;
		}
		void pconstraint::set_pivot_a(const trigonometry::vector3& value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "p2p constraint should be initialized");
			instance->setPivotA(V3_TO_BT(value));
			state.pivot_a = value;
#endif
		}
		void pconstraint::set_pivot_b(const trigonometry::vector3& value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "p2p constraint should be initialized");
			instance->setPivotB(V3_TO_BT(value));
			state.pivot_b = value;
#endif
		}
		trigonometry::vector3 pconstraint::get_pivot_a() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "p2p constraint should be initialized");
			const btVector3& value = instance->getPivotInA();
			return BT_TO_V3(value);
#else
			return 0;
#endif
		}
		trigonometry::vector3 pconstraint::get_pivot_b() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "p2p constraint should be initialized");
			const btVector3& value = instance->getPivotInB();
			return BT_TO_V3(value);
#else
			return 0;
#endif
		}
		pconstraint::desc& pconstraint::get_state()
		{
			return state;
		}

		hconstraint::hconstraint(simulator* refer, const desc& i) noexcept : constraint(refer), instance(nullptr), state(i)
		{
#ifdef VI_BULLET3
			VI_ASSERT(i.target_a != nullptr, "target a rigidbody should be set");
			VI_ASSERT(engine != nullptr, "simulator should be set");

			first = i.target_a->get();
			second = (i.target_b ? i.target_b->get() : nullptr);

			if (second != nullptr)
				instance = core::memory::init<btHingeConstraint>(*first, *second, btTransform::getIdentity(), btTransform::getIdentity(), i.references);
			else
				instance = core::memory::init<btHingeConstraint>(*first, btTransform::getIdentity(), i.references);

			instance->setUserConstraintPtr(this);
			engine->add_constraint(this);
#endif
		}
		hconstraint::~hconstraint() noexcept
		{
#ifdef VI_BULLET3
			engine->remove_constraint(this);
			core::memory::deinit(instance);
#endif
		}
		constraint* hconstraint::copy() const
		{
			VI_ASSERT(instance != nullptr, "hinge constraint should be initialized");
			hconstraint* target = new hconstraint(engine, state);
			target->set_breaking_impulse_threshold(get_breaking_impulse_threshold());
			target->set_enabled(is_enabled());
			target->enable_angular_motor(is_angular_motor_enabled(), get_motor_target_velocity(), get_max_motor_impulse());
			target->set_angular_only(is_angular_only());
			target->set_limit(get_lower_limit(), get_upper_limit(), get_limit_softness(), get_limit_bias_factor(), get_limit_relaxation_factor());
			target->set_offset(is_offset());

			return target;
		}
		btTypedConstraint* hconstraint::get() const
		{
#ifdef VI_BULLET3
			return instance;
#else
			return nullptr;
#endif
		}
		bool hconstraint::has_collisions() const
		{
			return state.collisions;
		}
		void hconstraint::enable_angular_motor(bool enable, float target_velocity, float max_motor_impulse)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "hinge constraint should be initialized");
			instance->enableAngularMotor(enable, target_velocity, max_motor_impulse);
#endif
		}
		void hconstraint::enable_motor(bool enable)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "hinge constraint should be initialized");
			instance->enableMotor(enable);
#endif
		}
		void hconstraint::test_limit(const trigonometry::matrix4x4& a, const trigonometry::matrix4x4& b)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "hinge constraint should be initialized");
			instance->testLimit(M16_TO_BT(a), M16_TO_BT(b));
#endif
		}
		void hconstraint::set_frames(const trigonometry::matrix4x4& a, const trigonometry::matrix4x4& b)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "hinge constraint should be initialized");
			instance->setFrames(M16_TO_BT(a), M16_TO_BT(b));
#endif
		}
		void hconstraint::set_angular_only(bool value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "hinge constraint should be initialized");
			instance->setAngularOnly(value);
#endif
		}
		void hconstraint::set_max_motor_impulse(float value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "hinge constraint should be initialized");
			instance->setMaxMotorImpulse(value);
#endif
		}
		void hconstraint::set_motor_target_velocity(float value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "hinge constraint should be initialized");
			instance->setMotorTargetVelocity(value);
#endif
		}
		void hconstraint::set_motor_target(float target_angle, float delta)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "hinge constraint should be initialized");
			instance->setMotorTarget(target_angle, delta);
#endif
		}
		void hconstraint::set_limit(float low, float high, float softness, float bias_factor, float relaxation_factor)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "hinge constraint should be initialized");
			instance->setLimit(low, high, softness, bias_factor, relaxation_factor);
#endif
		}
		void hconstraint::set_offset(bool value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "hinge constraint should be initialized");
			instance->setUseFrameOffset(value);
#endif
		}
		void hconstraint::set_reference_to_a(bool value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "hinge constraint should be initialized");
			instance->setUseReferenceFrameA(value);
#endif
		}
		void hconstraint::set_axis(const trigonometry::vector3& value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "hinge constraint should be initialized");
			btVector3 axis = V3_TO_BT(value);
			instance->setAxis(axis);
#endif
		}
		int hconstraint::get_solve_limit() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "hinge constraint should be initialized");
			return instance->getSolveLimit();
#else
			return 0;
#endif
		}
		float hconstraint::get_motor_target_velocity() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "hinge constraint should be initialized");
			return instance->getMotorTargetVelocity();
#else
			return 0;
#endif
		}
		float hconstraint::get_max_motor_impulse() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "hinge constraint should be initialized");
			return instance->getMaxMotorImpulse();
#else
			return 0;
#endif
		}
		float hconstraint::get_limit_sign() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "hinge constraint should be initialized");
			return instance->getLimitSign();
#else
			return 0;
#endif
		}
		float hconstraint::get_hinge_angle() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "hinge constraint should be initialized");
			return instance->getHingeAngle();
#else
			return 0;
#endif
		}
		float hconstraint::get_hinge_angle(const trigonometry::matrix4x4& a, const trigonometry::matrix4x4& b) const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "hinge constraint should be initialized");
			return instance->getHingeAngle(M16_TO_BT(a), M16_TO_BT(b));
#else
			return 0;
#endif
		}
		float hconstraint::get_lower_limit() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "hinge constraint should be initialized");
			return instance->getLowerLimit();
#else
			return 0;
#endif
		}
		float hconstraint::get_upper_limit() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "hinge constraint should be initialized");
			return instance->getUpperLimit();
#else
			return 0;
#endif
		}
		float hconstraint::get_limit_softness() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "hinge constraint should be initialized");
			return instance->getLimitSoftness();
#else
			return 0;
#endif
		}
		float hconstraint::get_limit_bias_factor() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "hinge constraint should be initialized");
			return instance->getLimitBiasFactor();
#else
			return 0;
#endif
		}
		float hconstraint::get_limit_relaxation_factor() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "hinge constraint should be initialized");
			return instance->getLimitRelaxationFactor();
#else
			return 0;
#endif
		}
		bool hconstraint::has_limit() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "hinge constraint should be initialized");
			return instance->hasLimit();
#else
			return 0;
#endif
		}
		bool hconstraint::is_offset() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "hinge constraint should be initialized");
			return instance->getUseFrameOffset();
#else
			return 0;
#endif
		}
		bool hconstraint::is_reference_to_a() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "hinge constraint should be initialized");
			return instance->getUseReferenceFrameA();
#else
			return 0;
#endif
		}
		bool hconstraint::is_angular_only() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "hinge constraint should be initialized");
			return instance->getAngularOnly();
#else
			return 0;
#endif
		}
		bool hconstraint::is_angular_motor_enabled() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "hinge constraint should be initialized");
			return instance->getEnableAngularMotor();
#else
			return 0;
#endif
		}
		hconstraint::desc& hconstraint::get_state()
		{
			return state;
		}

		sconstraint::sconstraint(simulator* refer, const desc& i) noexcept : constraint(refer), instance(nullptr), state(i)
		{
#ifdef VI_BULLET3
			VI_ASSERT(i.target_a != nullptr, "target a rigidbody should be set");
			VI_ASSERT(engine != nullptr, "simulator should be set");

			first = i.target_a->get();
			second = (i.target_b ? i.target_b->get() : nullptr);

			if (second != nullptr)
				instance = core::memory::init<btSliderConstraint>(*first, *second, btTransform::getIdentity(), btTransform::getIdentity(), i.linear);
			else
				instance = core::memory::init<btSliderConstraint>(*first, btTransform::getIdentity(), i.linear);

			instance->setUserConstraintPtr(this);
			engine->add_constraint(this);
#endif
		}
		sconstraint::~sconstraint() noexcept
		{
#ifdef VI_BULLET3
			engine->remove_constraint(this);
			core::memory::deinit(instance);
#endif
		}
		constraint* sconstraint::copy() const
		{
			VI_ASSERT(instance != nullptr, "slider constraint should be initialized");
			sconstraint* target = new sconstraint(engine, state);
			target->set_breaking_impulse_threshold(get_breaking_impulse_threshold());
			target->set_enabled(is_enabled());
			target->set_angular_motor_velocity(get_angular_motor_velocity());
			target->set_linear_motor_velocity(get_linear_motor_velocity());
			target->set_upper_linear_limit(get_upper_linear_limit());
			target->set_lower_linear_limit(get_lower_linear_limit());
			target->set_angular_damping_direction(get_angular_damping_direction());
			target->set_linear_damping_direction(get_linear_damping_direction());
			target->set_angular_damping_limit(get_angular_damping_limit());
			target->set_linear_damping_limit(get_linear_damping_limit());
			target->set_angular_damping_ortho(get_angular_damping_ortho());
			target->set_linear_damping_ortho(get_linear_damping_ortho());
			target->set_upper_angular_limit(get_upper_angular_limit());
			target->set_lower_angular_limit(get_lower_angular_limit());
			target->set_max_angular_motor_force(get_max_angular_motor_force());
			target->set_max_linear_motor_force(get_max_linear_motor_force());
			target->set_angular_restitution_direction(get_angular_restitution_direction());
			target->set_linear_restitution_direction(get_linear_restitution_direction());
			target->set_angular_restitution_limit(get_angular_restitution_limit());
			target->set_linear_restitution_limit(get_linear_restitution_limit());
			target->set_angular_restitution_ortho(get_angular_restitution_ortho());
			target->set_linear_restitution_ortho(get_linear_restitution_ortho());
			target->set_angular_softness_direction(get_angular_softness_direction());
			target->set_linear_softness_direction(get_linear_softness_direction());
			target->set_angular_softness_limit(get_angular_softness_limit());
			target->set_linear_softness_limit(get_linear_softness_limit());
			target->set_angular_softness_ortho(get_angular_softness_ortho());
			target->set_linear_softness_ortho(get_linear_softness_ortho());
			target->set_powered_angular_motor(get_powered_angular_motor());
			target->set_powered_linear_motor(get_powered_linear_motor());

			return target;
		}
		btTypedConstraint* sconstraint::get() const
		{
#ifdef VI_BULLET3
			return instance;
#else
			return nullptr;
#endif
		}
		bool sconstraint::has_collisions() const
		{
			return state.collisions;
		}
		void sconstraint::set_angular_motor_velocity(float value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "slider constraint should be initialized");
			instance->setTargetAngMotorVelocity(value);
#endif
		}
		void sconstraint::set_linear_motor_velocity(float value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "slider constraint should be initialized");
			instance->setTargetLinMotorVelocity(value);
#endif
		}
		void sconstraint::set_upper_linear_limit(float value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "slider constraint should be initialized");
			instance->setUpperLinLimit(value);
#endif
		}
		void sconstraint::set_lower_linear_limit(float value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "slider constraint should be initialized");
			instance->setLowerLinLimit(value);
#endif
		}
		void sconstraint::set_angular_damping_direction(float value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "slider constraint should be initialized");
			instance->setDampingDirAng(value);
#endif
		}
		void sconstraint::set_linear_damping_direction(float value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "slider constraint should be initialized");
			instance->setDampingDirLin(value);
#endif
		}
		void sconstraint::set_angular_damping_limit(float value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "slider constraint should be initialized");
			instance->setDampingLimAng(value);
#endif
		}
		void sconstraint::set_linear_damping_limit(float value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "slider constraint should be initialized");
			instance->setDampingLimLin(value);
#endif
		}
		void sconstraint::set_angular_damping_ortho(float value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "slider constraint should be initialized");
			instance->setDampingOrthoAng(value);
#endif
		}
		void sconstraint::set_linear_damping_ortho(float value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "slider constraint should be initialized");
			instance->setDampingOrthoLin(value);
#endif
		}
		void sconstraint::set_upper_angular_limit(float value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "slider constraint should be initialized");
			instance->setUpperAngLimit(value);
#endif
		}
		void sconstraint::set_lower_angular_limit(float value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "slider constraint should be initialized");
			instance->setLowerAngLimit(value);
#endif
		}
		void sconstraint::set_max_angular_motor_force(float value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "slider constraint should be initialized");
			instance->setMaxAngMotorForce(value);
#endif
		}
		void sconstraint::set_max_linear_motor_force(float value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "slider constraint should be initialized");
			instance->setMaxLinMotorForce(value);
#endif
		}
		void sconstraint::set_angular_restitution_direction(float value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "slider constraint should be initialized");
			instance->setRestitutionDirAng(value);
#endif
		}
		void sconstraint::set_linear_restitution_direction(float value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "slider constraint should be initialized");
			instance->setRestitutionDirLin(value);
#endif
		}
		void sconstraint::set_angular_restitution_limit(float value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "slider constraint should be initialized");
			instance->setRestitutionLimAng(value);
#endif
		}
		void sconstraint::set_linear_restitution_limit(float value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "slider constraint should be initialized");
			instance->setRestitutionLimLin(value);
#endif
		}
		void sconstraint::set_angular_restitution_ortho(float value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "slider constraint should be initialized");
			instance->setRestitutionOrthoAng(value);
#endif
		}
		void sconstraint::set_linear_restitution_ortho(float value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "slider constraint should be initialized");
			instance->setRestitutionOrthoLin(value);
#endif
		}
		void sconstraint::set_angular_softness_direction(float value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "slider constraint should be initialized");
			instance->setSoftnessDirAng(value);
#endif
		}
		void sconstraint::set_linear_softness_direction(float value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "slider constraint should be initialized");
			instance->setSoftnessDirLin(value);
#endif
		}
		void sconstraint::set_angular_softness_limit(float value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "slider constraint should be initialized");
			instance->setSoftnessLimAng(value);
#endif
		}
		void sconstraint::set_linear_softness_limit(float value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "slider constraint should be initialized");
			instance->setSoftnessLimLin(value);
#endif
		}
		void sconstraint::set_angular_softness_ortho(float value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "slider constraint should be initialized");
			instance->setSoftnessOrthoAng(value);
#endif
		}
		void sconstraint::set_linear_softness_ortho(float value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "slider constraint should be initialized");
			instance->setSoftnessOrthoLin(value);
#endif
		}
		void sconstraint::set_powered_angular_motor(bool value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "slider constraint should be initialized");
			instance->setPoweredAngMotor(value);
#endif
		}
		void sconstraint::set_powered_linear_motor(bool value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "slider constraint should be initialized");
			instance->setPoweredLinMotor(value);
#endif
		}
		float sconstraint::get_angular_motor_velocity() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "slider constraint should be initialized");
			return instance->getTargetAngMotorVelocity();
#else
			return 0;
#endif
		}
		float sconstraint::get_linear_motor_velocity() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "slider constraint should be initialized");
			return instance->getTargetLinMotorVelocity();
#else
			return 0;
#endif
		}
		float sconstraint::get_upper_linear_limit() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "slider constraint should be initialized");
			return instance->getUpperLinLimit();
#else
			return 0;
#endif
		}
		float sconstraint::get_lower_linear_limit() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "slider constraint should be initialized");
			return instance->getLowerLinLimit();
#else
			return 0;
#endif
		}
		float sconstraint::get_angular_damping_direction() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "slider constraint should be initialized");
			return instance->getDampingDirAng();
#else
			return 0;
#endif
		}
		float sconstraint::get_linear_damping_direction() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "slider constraint should be initialized");
			return instance->getDampingDirLin();
#else
			return 0;
#endif
		}
		float sconstraint::get_angular_damping_limit() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "slider constraint should be initialized");
			return instance->getDampingLimAng();
#else
			return 0;
#endif
		}
		float sconstraint::get_linear_damping_limit() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "slider constraint should be initialized");
			return instance->getDampingLimLin();
#else
			return 0;
#endif
		}
		float sconstraint::get_angular_damping_ortho() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "slider constraint should be initialized");
			return instance->getDampingOrthoAng();
#else
			return 0;
#endif
		}
		float sconstraint::get_linear_damping_ortho() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "slider constraint should be initialized");
			return instance->getDampingOrthoLin();
#else
			return 0;
#endif
		}
		float sconstraint::get_upper_angular_limit() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "slider constraint should be initialized");
			return instance->getUpperAngLimit();
#else
			return 0;
#endif
		}
		float sconstraint::get_lower_angular_limit() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "slider constraint should be initialized");
			return instance->getLowerAngLimit();
#else
			return 0;
#endif
		}
		float sconstraint::get_max_angular_motor_force() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "slider constraint should be initialized");
			return instance->getMaxAngMotorForce();
#else
			return 0;
#endif
		}
		float sconstraint::get_max_linear_motor_force() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "slider constraint should be initialized");
			return instance->getMaxLinMotorForce();
#else
			return 0;
#endif
		}
		float sconstraint::get_angular_restitution_direction() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "slider constraint should be initialized");
			return instance->getRestitutionDirAng();
#else
			return 0;
#endif
		}
		float sconstraint::get_linear_restitution_direction() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "slider constraint should be initialized");
			return instance->getRestitutionDirLin();
#else
			return 0;
#endif
		}
		float sconstraint::get_angular_restitution_limit() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "slider constraint should be initialized");
			return instance->getRestitutionLimAng();
#else
			return 0;
#endif
		}
		float sconstraint::get_linear_restitution_limit() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "slider constraint should be initialized");
			return instance->getRestitutionLimLin();
#else
			return 0;
#endif
		}
		float sconstraint::get_angular_restitution_ortho() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "slider constraint should be initialized");
			return instance->getRestitutionOrthoAng();
#else
			return 0;
#endif
		}
		float sconstraint::get_linear_restitution_ortho() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "slider constraint should be initialized");
			return instance->getRestitutionOrthoLin();
#else
			return 0;
#endif
		}
		float sconstraint::get_angular_softness_direction() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "slider constraint should be initialized");
			return instance->getSoftnessDirAng();
#else
			return 0;
#endif
		}
		float sconstraint::get_linear_softness_direction() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "slider constraint should be initialized");
			return instance->getSoftnessDirLin();
#else
			return 0;
#endif
		}
		float sconstraint::get_angular_softness_limit() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "slider constraint should be initialized");
			return instance->getSoftnessLimAng();
#else
			return 0;
#endif
		}
		float sconstraint::get_linear_softness_limit() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "slider constraint should be initialized");
			return instance->getSoftnessLimLin();
#else
			return 0;
#endif
		}
		float sconstraint::get_angular_softness_ortho() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "slider constraint should be initialized");
			return instance->getSoftnessOrthoAng();
#else
			return 0;
#endif
		}
		float sconstraint::get_linear_softness_ortho() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "slider constraint should be initialized");
			return instance->getSoftnessOrthoLin();
#else
			return 0;
#endif
		}
		bool sconstraint::get_powered_angular_motor() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "slider constraint should be initialized");
			return instance->getPoweredAngMotor();
#else
			return false;
#endif
		}
		bool sconstraint::get_powered_linear_motor() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "slider constraint should be initialized");
			return instance->getPoweredLinMotor();
#else
			return false;
#endif
		}
		sconstraint::desc& sconstraint::get_state()
		{
			return state;
		}

		ct_constraint::ct_constraint(simulator* refer, const desc& i) noexcept : constraint(refer), instance(nullptr), state(i)
		{
#ifdef VI_BULLET3
			VI_ASSERT(i.target_a != nullptr, "target a rigidbody should be set");
			VI_ASSERT(engine != nullptr, "simulator should be set");

			first = i.target_a->get();
			second = (i.target_b ? i.target_b->get() : nullptr);

			if (second != nullptr)
				instance = core::memory::init<btConeTwistConstraint>(*first, *second, btTransform::getIdentity(), btTransform::getIdentity());
			else
				instance = core::memory::init<btConeTwistConstraint>(*first, btTransform::getIdentity());

			instance->setUserConstraintPtr(this);
			engine->add_constraint(this);
#endif
		}
		ct_constraint::~ct_constraint() noexcept
		{
#ifdef VI_BULLET3
			engine->remove_constraint(this);
			core::memory::deinit(instance);
#endif
		}
		constraint* ct_constraint::copy() const
		{
			VI_ASSERT(instance != nullptr, "cone-twist constraint should be initialized");
			ct_constraint* target = new ct_constraint(engine, state);
			target->set_breaking_impulse_threshold(get_breaking_impulse_threshold());
			target->set_enabled(is_enabled());
			target->enable_motor(is_motor_enabled());
			target->set_angular_only(is_angular_only());
			target->set_limit(get_swing_span1(), get_swing_span2(), get_twist_span(), get_limit_softness(), get_bias_factor(), get_relaxation_factor());
			target->set_damping(get_damping());
			target->set_fix_thresh(get_fix_thresh());
			target->set_motor_target(get_motor_target());

			if (is_max_motor_impulse_normalized())
				target->set_max_motor_impulse_normalized(get_max_motor_impulse());
			else
				target->set_max_motor_impulse(get_max_motor_impulse());

			return target;
		}
		btTypedConstraint* ct_constraint::get() const
		{
#ifdef VI_BULLET3
			return instance;
#else
			return nullptr;
#endif
		}
		bool ct_constraint::has_collisions() const
		{
			return state.collisions;
		}
		void ct_constraint::enable_motor(bool value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "cone-twist constraint should be initialized");
			instance->enableMotor(value);
#endif
		}
		void ct_constraint::set_frames(const trigonometry::matrix4x4& a, const trigonometry::matrix4x4& b)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "cone-twist constraint should be initialized");
			instance->setFrames(M16_TO_BT(a), M16_TO_BT(b));
#endif
		}
		void ct_constraint::set_angular_only(bool value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "cone-twist constraint should be initialized");
			instance->setAngularOnly(value);
#endif
		}
		void ct_constraint::set_limit(int limit_index, float limit_value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "cone-twist constraint should be initialized");
			instance->setLimit(limit_index, limit_value);
#endif
		}
		void ct_constraint::set_limit(float swing_span1, float swing_span2, float twist_span, float softness, float bias_factor, float relaxation_factor)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "cone-twist constraint should be initialized");
			instance->setLimit(swing_span1, swing_span2, twist_span, softness, bias_factor, relaxation_factor);
#endif
		}
		void ct_constraint::set_damping(float value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "cone-twist constraint should be initialized");
			instance->setDamping(value);
#endif
		}
		void ct_constraint::set_max_motor_impulse(float value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "cone-twist constraint should be initialized");
			instance->setMaxMotorImpulse(value);
#endif
		}
		void ct_constraint::set_max_motor_impulse_normalized(float value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "cone-twist constraint should be initialized");
			instance->setMaxMotorImpulseNormalized(value);
#endif
		}
		void ct_constraint::set_fix_thresh(float value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "cone-twist constraint should be initialized");
			instance->setFixThresh(value);
#endif
		}
		void ct_constraint::set_motor_target(const trigonometry::quaternion& value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "cone-twist constraint should be initialized");
			instance->setMotorTarget(Q4_TO_BT(value));
#endif
		}
		void ct_constraint::set_motor_target_in_constraint_space(const trigonometry::quaternion& value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "cone-twist constraint should be initialized");
			instance->setMotorTargetInConstraintSpace(Q4_TO_BT(value));
#endif
		}
		trigonometry::vector3 ct_constraint::get_point_for_angle(float angle_in_radians, float length) const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "cone-twist constraint should be initialized");
			btVector3 value = instance->GetPointForAngle(angle_in_radians, length);
			return BT_TO_V3(value);
#else
			return 0;
#endif
		}
		trigonometry::quaternion ct_constraint::get_motor_target() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "hinge constraint should be initialized");
			btQuaternion value = instance->getMotorTarget();
			return BT_TO_Q4(value);
#else
			return trigonometry::quaternion();
#endif
		}
		int ct_constraint::get_solve_twist_limit() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "hinge constraint should be initialized");
			return instance->getSolveTwistLimit();
#else
			return 0;
#endif
		}
		int ct_constraint::get_solve_swing_limit() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "hinge constraint should be initialized");
			return instance->getSolveSwingLimit();
#else
			return 0;
#endif
		}
		float ct_constraint::get_twist_limit_sign() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "hinge constraint should be initialized");
			return instance->getTwistLimitSign();
#else
			return 0;
#endif
		}
		float ct_constraint::get_swing_span1() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "hinge constraint should be initialized");
			return instance->getSwingSpan1();
#else
			return 0;
#endif
		}
		float ct_constraint::get_swing_span2() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "hinge constraint should be initialized");
			return instance->getSwingSpan2();
#else
			return 0;
#endif
		}
		float ct_constraint::get_twist_span() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "hinge constraint should be initialized");
			return instance->getTwistSpan();
#else
			return 0;
#endif
		}
		float ct_constraint::get_limit_softness() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "hinge constraint should be initialized");
			return instance->getLimitSoftness();
#else
			return 0;
#endif
		}
		float ct_constraint::get_bias_factor() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "hinge constraint should be initialized");
			return instance->getBiasFactor();
#else
			return 0;
#endif
		}
		float ct_constraint::get_relaxation_factor() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "hinge constraint should be initialized");
			return instance->getRelaxationFactor();
#else
			return 0;
#endif
		}
		float ct_constraint::get_twist_angle() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "hinge constraint should be initialized");
			return instance->getTwistAngle();
#else
			return 0;
#endif
		}
		float ct_constraint::get_limit(int value) const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "hinge constraint should be initialized");
			return instance->getLimit(value);
#else
			return 0;
#endif
		}
		float ct_constraint::get_damping() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "hinge constraint should be initialized");
			return instance->getDamping();
#else
			return 0;
#endif
		}
		float ct_constraint::get_max_motor_impulse() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "hinge constraint should be initialized");
			return instance->getMaxMotorImpulse();
#else
			return 0;
#endif
		}
		float ct_constraint::get_fix_thresh() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "hinge constraint should be initialized");
			return instance->getFixThresh();
#else
			return 0;
#endif
		}
		bool ct_constraint::is_motor_enabled() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "hinge constraint should be initialized");
			return instance->isMotorEnabled();
#else
			return 0;
#endif
		}
		bool ct_constraint::is_max_motor_impulse_normalized() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "hinge constraint should be initialized");
			return instance->isMaxMotorImpulseNormalized();
#else
			return 0;
#endif
		}
		bool ct_constraint::is_past_swing_limit() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "hinge constraint should be initialized");
			return instance->isPastSwingLimit();
#else
			return 0;
#endif
		}
		bool ct_constraint::is_angular_only() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "hinge constraint should be initialized");
			return instance->getAngularOnly();
#else
			return 0;
#endif
		}
		ct_constraint::desc& ct_constraint::get_state()
		{
			return state;
		}

		df6_constraint::df6_constraint(simulator* refer, const desc& i) noexcept : constraint(refer), instance(nullptr), state(i)
		{
#ifdef VI_BULLET3
			VI_ASSERT(i.target_a != nullptr, "target a rigidbody should be set");
			VI_ASSERT(engine != nullptr, "simulator should be set");

			first = i.target_a->get();
			second = (i.target_b ? i.target_b->get() : nullptr);

			if (second != nullptr)
				instance = core::memory::init<btGeneric6DofSpring2Constraint>(*first, *second, btTransform::getIdentity(), btTransform::getIdentity());
			else
				instance = core::memory::init<btGeneric6DofSpring2Constraint>(*first, btTransform::getIdentity());

			instance->setUserConstraintPtr(this);
			engine->add_constraint(this);
#endif
		}
		df6_constraint::~df6_constraint() noexcept
		{
#ifdef VI_BULLET3
			engine->remove_constraint(this);
			core::memory::deinit(instance);
#endif
		}
		constraint* df6_constraint::copy() const
		{
			VI_ASSERT(instance != nullptr, "6-dof constraint should be initialized");
			df6_constraint* target = new df6_constraint(engine, state);
			target->set_breaking_impulse_threshold(get_breaking_impulse_threshold());
			target->set_enabled(is_enabled());
			target->set_linear_lower_limit(get_linear_lower_limit());
			target->set_linear_upper_limit(get_linear_upper_limit());
			target->set_angular_lower_limit(get_angular_lower_limit());
			target->set_angular_upper_limit(get_angular_upper_limit());
			target->set_rotation_order(get_rotation_order());
			target->set_axis(get_axis(0), get_axis(1));

			return target;
		}
		btTypedConstraint* df6_constraint::get() const
		{
#ifdef VI_BULLET3
			return instance;
#else
			return nullptr;
#endif
		}
		bool df6_constraint::has_collisions() const
		{
			return state.collisions;
		}
		void df6_constraint::enable_motor(int index, bool on_off)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "6-dof constraint should be initialized");
			instance->enableMotor(index, on_off);
#endif
		}
		void df6_constraint::enable_spring(int index, bool on_off)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "6-dof constraint should be initialized");
			instance->enableSpring(index, on_off);
#endif
		}
		void df6_constraint::set_frames(const trigonometry::matrix4x4& a, const trigonometry::matrix4x4& b)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "6-dof constraint should be initialized");
			instance->setFrames(M16_TO_BT(a), M16_TO_BT(b));
#endif
		}
		void df6_constraint::set_linear_lower_limit(const trigonometry::vector3& value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "6-dof constraint should be initialized");
			instance->setLinearLowerLimit(V3_TO_BT(value));
#endif
		}
		void df6_constraint::set_linear_upper_limit(const trigonometry::vector3& value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "6-dof constraint should be initialized");
			instance->setLinearUpperLimit(V3_TO_BT(value));
#endif
		}
		void df6_constraint::set_angular_lower_limit(const trigonometry::vector3& value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "6-dof constraint should be initialized");
			instance->setAngularLowerLimit(V3_TO_BT(value));
#endif
		}
		void df6_constraint::set_angular_lower_limit_reversed(const trigonometry::vector3& value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "6-dof constraint should be initialized");
			instance->setAngularLowerLimitReversed(V3_TO_BT(value));
#endif
		}
		void df6_constraint::set_angular_upper_limit(const trigonometry::vector3& value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "6-dof constraint should be initialized");
			instance->setAngularUpperLimit(V3_TO_BT(value));
#endif
		}
		void df6_constraint::set_angular_upper_limit_reversed(const trigonometry::vector3& value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "6-dof constraint should be initialized");
			instance->setAngularUpperLimitReversed(V3_TO_BT(value));
#endif
		}
		void df6_constraint::set_limit(int axis, float low, float high)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "6-dof constraint should be initialized");
			instance->setLimit(axis, low, high);
#endif
		}
		void df6_constraint::set_limit_reversed(int axis, float low, float high)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "6-dof constraint should be initialized");
			instance->setLimitReversed(axis, low, high);
#endif
		}
		void df6_constraint::set_rotation_order(trigonometry::rotator order)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "6-dof constraint should be initialized");
			instance->setRotationOrder((RotateOrder)order);
#endif
		}
		void df6_constraint::set_axis(const trigonometry::vector3& a, const trigonometry::vector3& b)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "6-dof constraint should be initialized");
			instance->setAxis(V3_TO_BT(a), V3_TO_BT(b));
#endif
		}
		void df6_constraint::set_bounce(int index, float bounce)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "6-dof constraint should be initialized");
			instance->setBounce(index, bounce);
#endif
		}
		void df6_constraint::set_servo(int index, bool on_off)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "6-dof constraint should be initialized");
			instance->setServo(index, on_off);
#endif
		}
		void df6_constraint::set_target_velocity(int index, float velocity)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "6-dof constraint should be initialized");
			instance->setTargetVelocity(index, velocity);
#endif
		}
		void df6_constraint::set_servo_target(int index, float target)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "6-dof constraint should be initialized");
			instance->setServoTarget(index, target);
#endif
		}
		void df6_constraint::set_max_motor_force(int index, float force)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "6-dof constraint should be initialized");
			instance->setMaxMotorForce(index, force);
#endif
		}
		void df6_constraint::set_stiffness(int index, float stiffness, bool limit_if_needed)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "6-dof constraint should be initialized");
			instance->setStiffness(index, stiffness, limit_if_needed);
#endif
		}
		void df6_constraint::set_equilibrium_point()
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "6-dof constraint should be initialized");
			instance->setEquilibriumPoint();
#endif
		}
		void df6_constraint::set_equilibrium_point(int index)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "6-dof constraint should be initialized");
			instance->setEquilibriumPoint(index);
#endif
		}
		void df6_constraint::set_equilibrium_point(int index, float value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "6-dof constraint should be initialized");
			instance->setEquilibriumPoint(index, value);
#endif
		}
		trigonometry::vector3 df6_constraint::get_angular_upper_limit() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "6-dof constraint should be initialized");
			btVector3 result;
			instance->getAngularUpperLimit(result);
			return BT_TO_V3(result);
#else
			return 0;
#endif
		}
		trigonometry::vector3 df6_constraint::get_angular_upper_limit_reversed() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "6-dof constraint should be initialized");
			btVector3 result;
			instance->getAngularUpperLimitReversed(result);
			return BT_TO_V3(result);
#else
			return 0;
#endif
		}
		trigonometry::vector3 df6_constraint::get_angular_lower_limit() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "6-dof constraint should be initialized");
			btVector3 result;
			instance->getAngularLowerLimit(result);
			return BT_TO_V3(result);
#else
			return 0;
#endif
		}
		trigonometry::vector3 df6_constraint::get_angular_lower_limit_reversed() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "6-dof constraint should be initialized");
			btVector3 result;
			instance->getAngularLowerLimitReversed(result);
			return BT_TO_V3(result);
#else
			return 0;
#endif
		}
		trigonometry::vector3 df6_constraint::get_linear_upper_limit() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "6-dof constraint should be initialized");
			btVector3 result;
			instance->getLinearUpperLimit(result);
			return BT_TO_V3(result);
#else
			return 0;
#endif
		}
		trigonometry::vector3 df6_constraint::get_linear_lower_limit() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "6-dof constraint should be initialized");
			btVector3 result;
			instance->getLinearLowerLimit(result);
			return BT_TO_V3(result);
#else
			return 0;
#endif
		}
		trigonometry::vector3 df6_constraint::get_axis(int value) const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "6-dof constraint should be initialized");
			btVector3 result = instance->getAxis(value);
			return BT_TO_V3(result);
#else
			return 0;
#endif
		}
		trigonometry::rotator df6_constraint::get_rotation_order() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "6-dof constraint should be initialized");
			return (trigonometry::rotator)instance->getRotationOrder();
#else
			return trigonometry::rotator::xyz;
#endif
		}
		float df6_constraint::get_angle(int value) const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "6-dof constraint should be initialized");
			return instance->getAngle(value);
#else
			return 0;
#endif
		}
		float df6_constraint::get_relative_pivot_position(int value) const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "6-dof constraint should be initialized");
			return instance->getRelativePivotPosition(value);
#else
			return 0;
#endif
		}
		bool df6_constraint::is_limited(int limit_index) const
		{
#ifdef VI_BULLET3
			VI_ASSERT(instance != nullptr, "6-dof constraint should be initialized");
			return instance->isLimited(limit_index);
#else
			return 0;
#endif
		}
		df6_constraint::desc& df6_constraint::get_state()
		{
			return state;
		}

		simulator::simulator(const desc& i) noexcept : soft_solver(nullptr), speedup(1.0f), active(true)
		{
#ifdef VI_BULLET3
			broadphase = core::memory::init<btDbvtBroadphase>();
			solver = core::memory::init<btSequentialImpulseConstraintSolver>();

			if (i.enable_soft_body)
			{
				soft_solver = core::memory::init<btDefaultSoftBodySolver>();
				collision = core::memory::init<btSoftBodyRigidBodyCollisionConfiguration>();
				dispatcher = core::memory::init<btCollisionDispatcher>(collision);
				world = core::memory::init<btSoftRigidDynamicsWorld>(dispatcher, broadphase, solver, collision, soft_solver);
				btSoftRigidDynamicsWorld* soft_world = (btSoftRigidDynamicsWorld*)world;
				soft_world->getDispatchInfo().m_enableSPU = true;

				btSoftBodyWorldInfo& info = soft_world->getWorldInfo();
				info.m_gravity = V3_TO_BT(i.gravity);
				info.water_normal = V3_TO_BT(i.water_normal);
				info.water_density = i.water_density;
				info.water_offset = i.water_offset;
				info.air_density = i.air_density;
				info.m_maxDisplacement = i.max_displacement;
			}
			else
			{
				collision = core::memory::init<btDefaultCollisionConfiguration>();
				dispatcher = core::memory::init<btCollisionDispatcher>(collision);
				world = core::memory::init<btDiscreteDynamicsWorld>(dispatcher, broadphase, solver, collision);
			}

			world->setWorldUserInfo(this);
			world->setGravity(V3_TO_BT(i.gravity));
			gContactAddedCallback = nullptr;
			gContactDestroyedCallback = nullptr;
			gContactProcessedCallback = nullptr;
			gContactStartedCallback = [](btPersistentManifold* const& manifold) -> void
			{
				btCollisionObject* body1 = (btCollisionObject*)manifold->getBody0();
				btRigidBody* rigid1 = btRigidBody::upcast(body1);
				btSoftBody* soft1 = btSoftBody::upcast(body1);

				if (rigid1 != nullptr)
				{
					rigid_body* body = (rigid_body*)rigid1->getUserPointer();
					if (body != nullptr && body->on_collision_enter)
						body->on_collision_enter(collision_body((btCollisionObject*)manifold->getBody1()));
				}
				else if (soft1 != nullptr)
				{
					soft_body* body = (soft_body*)soft1->getUserPointer();
					if (body != nullptr && body->on_collision_enter)
						body->on_collision_enter(collision_body((btCollisionObject*)manifold->getBody1()));
				}
			};
			gContactEndedCallback = [](btPersistentManifold* const& manifold) -> void
			{
				btCollisionObject* body1 = (btCollisionObject*)manifold->getBody0();
				btRigidBody* rigid1 = btRigidBody::upcast(body1);
				btSoftBody* soft1 = btSoftBody::upcast(body1);

				if (rigid1 != nullptr)
				{
					rigid_body* body = (rigid_body*)rigid1->getUserPointer();
					if (body != nullptr && body->on_collision_enter)
						body->on_collision_exit(collision_body((btCollisionObject*)manifold->getBody1()));
				}
				else if (soft1 != nullptr)
				{
					soft_body* body = (soft_body*)soft1->getUserPointer();
					if (body != nullptr && body->on_collision_enter)
						body->on_collision_exit(collision_body((btCollisionObject*)manifold->getBody1()));
				}
			};
#endif
		}
		simulator::~simulator() noexcept
		{
#ifdef VI_BULLET3
			remove_all();
			for (auto it = shapes.begin(); it != shapes.end(); ++it)
			{
				btCollisionShape* item = (btCollisionShape*)it->first;
				core::memory::deinit(item);
			}

			core::memory::deinit(dispatcher);
			core::memory::deinit(collision);
			core::memory::deinit(solver);
			core::memory::deinit(broadphase);
			core::memory::deinit(soft_solver);
			core::memory::deinit(world);
#endif
		}
		void simulator::set_gravity(const trigonometry::vector3& gravity)
		{
#ifdef VI_BULLET3
			world->setGravity(V3_TO_BT(gravity));
#endif
		}
		void simulator::set_linear_impulse(const trigonometry::vector3& impulse, bool random_factor)
		{
#ifdef VI_BULLET3
			for (int i = 0; i < world->getNumCollisionObjects(); i++)
			{
				trigonometry::vector3 velocity = impulse * (random_factor ? trigonometry::vector3::random() : 1);
				btRigidBody::upcast(world->getCollisionObjectArray()[i])->setLinearVelocity(V3_TO_BT(velocity));
			}
#endif
		}
		void simulator::set_linear_impulse(const trigonometry::vector3& impulse, int start, int end, bool random_factor)
		{
#ifdef VI_BULLET3
			if (start >= 0 && start < world->getNumCollisionObjects() && end >= 0 && end < world->getNumCollisionObjects())
			{
				for (int i = start; i < end; i++)
				{
					trigonometry::vector3 velocity = impulse * (random_factor ? trigonometry::vector3::random() : 1);
					btRigidBody::upcast(world->getCollisionObjectArray()[i])->setLinearVelocity(V3_TO_BT(velocity));
				}
			}
#endif
		}
		void simulator::set_angular_impulse(const trigonometry::vector3& impulse, bool random_factor)
		{
#ifdef VI_BULLET3
			for (int i = 0; i < world->getNumCollisionObjects(); i++)
			{
				trigonometry::vector3 velocity = impulse * (random_factor ? trigonometry::vector3::random() : 1);
				btRigidBody::upcast(world->getCollisionObjectArray()[i])->setAngularVelocity(V3_TO_BT(velocity));
			}
#endif
		}
		void simulator::set_angular_impulse(const trigonometry::vector3& impulse, int start, int end, bool random_factor)
		{
#ifdef VI_BULLET3
			if (start >= 0 && start < world->getNumCollisionObjects() && end >= 0 && end < world->getNumCollisionObjects())
			{
				for (int i = start; i < end; i++)
				{
					trigonometry::vector3 velocity = impulse * (random_factor ? trigonometry::vector3::random() : 1);
					btRigidBody::upcast(world->getCollisionObjectArray()[i])->setAngularVelocity(V3_TO_BT(velocity));
				}
			}
#endif
		}
		void simulator::set_on_collision_enter(contact_started_callback callback)
		{
#ifdef VI_BULLET3
			gContactStartedCallback = callback;
#endif
		}
		void simulator::set_on_collision_exit(contact_ended_callback callback)
		{
#ifdef VI_BULLET3
			gContactEndedCallback = callback;
#endif
		}
		void simulator::create_linear_impulse(const trigonometry::vector3& impulse, bool random_factor)
		{
#ifdef VI_BULLET3
			for (int i = 0; i < world->getNumCollisionObjects(); i++)
			{
				btRigidBody* body = btRigidBody::upcast(world->getCollisionObjectArray()[i]);
				btVector3 velocity = body->getLinearVelocity();
				velocity.setX(velocity.getX() + impulse.x * (random_factor ? compute::mathf::random() : 1));
				velocity.setY(velocity.getY() + impulse.y * (random_factor ? compute::mathf::random() : 1));
				velocity.setZ(velocity.getZ() + impulse.z * (random_factor ? compute::mathf::random() : 1));
				btRigidBody::upcast(world->getCollisionObjectArray()[i])->setLinearVelocity(velocity);
			}
#endif
		}
		void simulator::create_linear_impulse(const trigonometry::vector3& impulse, int start, int end, bool random_factor)
		{
#ifdef VI_BULLET3
			if (start >= 0 && start < world->getNumCollisionObjects() && end >= 0 && end < world->getNumCollisionObjects())
			{
				for (int i = start; i < end; i++)
				{
					btRigidBody* body = btRigidBody::upcast(world->getCollisionObjectArray()[i]);
					btVector3 velocity = body->getLinearVelocity();
					velocity.setX(velocity.getX() + impulse.x * (random_factor ? compute::mathf::random() : 1));
					velocity.setY(velocity.getY() + impulse.y * (random_factor ? compute::mathf::random() : 1));
					velocity.setZ(velocity.getZ() + impulse.z * (random_factor ? compute::mathf::random() : 1));
					btRigidBody::upcast(world->getCollisionObjectArray()[i])->setLinearVelocity(velocity);
				}
			}
#endif
		}
		void simulator::create_angular_impulse(const trigonometry::vector3& impulse, bool random_factor)
		{
#ifdef VI_BULLET3
			for (int i = 0; i < world->getNumCollisionObjects(); i++)
			{
				btRigidBody* body = btRigidBody::upcast(world->getCollisionObjectArray()[i]);
				btVector3 velocity = body->getAngularVelocity();
				velocity.setX(velocity.getX() + impulse.x * (random_factor ? compute::mathf::random() : 1));
				velocity.setY(velocity.getY() + impulse.y * (random_factor ? compute::mathf::random() : 1));
				velocity.setZ(velocity.getZ() + impulse.z * (random_factor ? compute::mathf::random() : 1));
				btRigidBody::upcast(world->getCollisionObjectArray()[i])->setAngularVelocity(velocity);
			}
#endif
		}
		void simulator::create_angular_impulse(const trigonometry::vector3& impulse, int start, int end, bool random_factor)
		{
#ifdef VI_BULLET3
			if (start >= 0 && start < world->getNumCollisionObjects() && end >= 0 && end < world->getNumCollisionObjects())
			{
				for (int i = start; i < end; i++)
				{
					btRigidBody* body = btRigidBody::upcast(world->getCollisionObjectArray()[i]);
					btVector3 velocity = body->getAngularVelocity();
					velocity.setX(velocity.getX() + impulse.x * (random_factor ? compute::mathf::random() : 1));
					velocity.setY(velocity.getY() + impulse.y * (random_factor ? compute::mathf::random() : 1));
					velocity.setZ(velocity.getZ() + impulse.z * (random_factor ? compute::mathf::random() : 1));
					btRigidBody::upcast(world->getCollisionObjectArray()[i])->setAngularVelocity(velocity);
				}
			}
#endif
		}
		void simulator::add_soft_body(soft_body* body)
		{
#ifdef VI_BULLET3
			VI_ASSERT(body != nullptr, "softbody should be set");
			VI_ASSERT(body->instance != nullptr, "softbody instance should be set");
			VI_ASSERT(body->instance->getWorldArrayIndex() == -1, "softbody should not be attached to other world");
			VI_ASSERT(has_soft_body_support(), "softbodies should be supported");
			VI_TRACE("[sim] add soft-body 0x%" PRIXPTR " on 0x%" PRIXPTR, (void*)body, (void*)this);

			btSoftRigidDynamicsWorld* soft_world = (btSoftRigidDynamicsWorld*)world;
			soft_world->addSoftBody(body->instance);
#endif
		}
		void simulator::remove_soft_body(soft_body* body)
		{
#ifdef VI_BULLET3
			VI_ASSERT(body != nullptr, "softbody should be set");
			VI_ASSERT(body->instance != nullptr, "softbody instance should be set");
			VI_ASSERT(body->instance->getWorldArrayIndex() >= 0, "softbody should be attached to world");
			VI_ASSERT(has_soft_body_support(), "softbodies should be supported");
			VI_TRACE("[sim] remove soft-body 0x%" PRIXPTR " on 0x%" PRIXPTR, (void*)body, (void*)this);

			btSoftRigidDynamicsWorld* soft_world = (btSoftRigidDynamicsWorld*)world;
			soft_world->removeSoftBody(body->instance);
#endif
		}
		void simulator::add_rigid_body(rigid_body* body)
		{
#ifdef VI_BULLET3
			VI_ASSERT(body != nullptr, "rigidbody should be set");
			VI_ASSERT(body->instance != nullptr, "rigidbody instance should be set");
			VI_ASSERT(body->instance->getWorldArrayIndex() == -1, "rigidbody should not be attached to other world");
			VI_TRACE("[sim] add rigid-body 0x%" PRIXPTR " on 0x%" PRIXPTR, (void*)body, (void*)this);
			world->addRigidBody(body->instance);
#endif
		}
		void simulator::remove_rigid_body(rigid_body* body)
		{
#ifdef VI_BULLET3
			VI_ASSERT(body != nullptr, "rigidbody should be set");
			VI_ASSERT(body->instance != nullptr, "rigidbody instance should be set");
			VI_ASSERT(body->instance->getWorldArrayIndex() >= 0, "rigidbody should be attached to other world");
			VI_TRACE("[sim] remove rigid-body 0x%" PRIXPTR " on 0x%" PRIXPTR, (void*)body, (void*)this);
			world->removeRigidBody(body->instance);
#endif
		}
		void simulator::add_constraint(constraint* constraint)
		{
#ifdef VI_BULLET3
			VI_ASSERT(constraint != nullptr, "slider constraint should be set");
			VI_ASSERT(constraint->get() != nullptr, "slider constraint instance should be set");
			VI_TRACE("[sim] add constraint 0x%" PRIXPTR " on 0x%" PRIXPTR, (void*)constraint, (void*)this);
			world->addConstraint(constraint->get(), !constraint->has_collisions());
#endif
		}
		void simulator::remove_constraint(constraint* constraint)
		{
#ifdef VI_BULLET3
			VI_ASSERT(constraint != nullptr, "slider constraint should be set");
			VI_ASSERT(constraint->get() != nullptr, "slider constraint instance should be set");
			VI_TRACE("[sim] remove constraint 0x%" PRIXPTR " on 0x%" PRIXPTR, (void*)constraint, (void*)this);
			world->removeConstraint(constraint->get());
#endif
		}
		void simulator::remove_all()
		{
#ifdef VI_BULLET3
			VI_TRACE("[sim] remove all collision objects on 0x%" PRIXPTR, (void*)this);
			for (int i = 0; i < world->getNumCollisionObjects(); i++)
			{
				btCollisionObject* object = world->getCollisionObjectArray()[i];
				btRigidBody* body = btRigidBody::upcast(object);
				if (body != nullptr)
				{
					auto* state = body->getMotionState();
					core::memory::deinit(state);
					body->setMotionState(nullptr);

					auto* shape = body->getCollisionShape();
					core::memory::deinit(shape);
					body->setCollisionShape(nullptr);
				}

				world->removeCollisionObject(object);
				core::memory::deinit(object);
			}
#endif
		}
		void simulator::simulate_step(float elapsed_time)
		{
#ifdef VI_BULLET3
			if (!active || speedup <= 0.0f)
				return;

			VI_MEASURE(core::timings::frame);
			float time_step = (timing.last_elapsed_time > 0.0 ? std::max(0.0f, elapsed_time - timing.last_elapsed_time) : 0.0f);
			world->stepSimulation(time_step * speedup, 0);
			timing.last_elapsed_time = elapsed_time;
#endif
		}
		void simulator::find_contacts(rigid_body* body, int(*callback)(shape_contact*, const collision_body&, const collision_body&))
		{
#ifdef VI_BULLET3
			VI_ASSERT(callback != nullptr, "callback should not be empty");
			VI_ASSERT(body != nullptr, "body should be set");
			VI_MEASURE(core::timings::pass);

			find_contacts_handler handler;
			handler.callback = callback;
			world->contactTest(body->get(), handler);
#endif
		}
		bool simulator::find_ray_contacts(const trigonometry::vector3& start, const trigonometry::vector3& end, int(*callback)(ray_contact*, const collision_body&))
		{
#ifdef VI_BULLET3
			VI_ASSERT(callback != nullptr, "callback should not be empty");
			VI_MEASURE(core::timings::pass);

			find_ray_contacts_handler handler;
			handler.callback = callback;

			world->rayTest(btVector3(start.x, start.y, start.z), btVector3(end.x, end.y, end.z), handler);
			return handler.m_collisionObject != nullptr;
#else
			return false;
#endif
		}
		rigid_body* simulator::create_rigid_body(const rigid_body::desc& i, trigonometry::transform* transform)
		{
#ifdef VI_BULLET3
			if (!transform)
				return create_rigid_body(i);

			rigid_body::desc f(i);
			f.position = transform->get_position();
			f.rotation = transform->get_rotation();
			f.scale = transform->get_scale();
			return create_rigid_body(f);
#else
			return nullptr;
#endif
		}
		rigid_body* simulator::create_rigid_body(const rigid_body::desc& i)
		{
#ifdef VI_BULLET3
			return new rigid_body(this, i);
#else
			return nullptr;
#endif
		}
		soft_body* simulator::create_soft_body(const soft_body::desc& i, trigonometry::transform* transform)
		{
#ifdef VI_BULLET3
			if (!transform)
				return create_soft_body(i);

			soft_body::desc f(i);
			f.position = transform->get_position();
			f.rotation = transform->get_rotation();
			f.scale = transform->get_scale();
			return create_soft_body(f);
#else
			return nullptr;
#endif
		}
		soft_body* simulator::create_soft_body(const soft_body::desc& i)
		{
#ifdef VI_BULLET3
			if (!has_soft_body_support())
				return nullptr;

			return new soft_body(this, i);
#else
			return nullptr;
#endif
		}
		pconstraint* simulator::create_point2_point_constraint(const pconstraint::desc& i)
		{
#ifdef VI_BULLET3
			return new pconstraint(this, i);
#else
			return nullptr;
#endif
		}
		hconstraint* simulator::create_hinge_constraint(const hconstraint::desc& i)
		{
#ifdef VI_BULLET3
			return new hconstraint(this, i);
#else
			return nullptr;
#endif
		}
		sconstraint* simulator::create_slider_constraint(const sconstraint::desc& i)
		{
#ifdef VI_BULLET3
			return new sconstraint(this, i);
#else
			return nullptr;
#endif
		}
		ct_constraint* simulator::create_cone_twist_constraint(const ct_constraint::desc& i)
		{
#ifdef VI_BULLET3
			return new ct_constraint(this, i);
#else
			return nullptr;
#endif
		}
		df6_constraint* simulator::create_6dof_constraint(const df6_constraint::desc& i)
		{
#ifdef VI_BULLET3
			return new df6_constraint(this, i);
#else
			return nullptr;
#endif
		}
		btCollisionShape* simulator::create_cube(const trigonometry::vector3& scale)
		{
#ifdef VI_BULLET3
			btCollisionShape* shape = core::memory::init<btBoxShape>(V3_TO_BT(scale));
			VI_TRACE("[sim] save cube shape 0x%" PRIXPTR, (void*)shape);
			core::umutex<std::mutex> unique(exclusive);
			shapes[shape] = 1;
			return shape;
#else
			return nullptr;
#endif
		}
		btCollisionShape* simulator::create_sphere(float radius)
		{
#ifdef VI_BULLET3
			btCollisionShape* shape = core::memory::init<btSphereShape>(radius);
			VI_TRACE("[sim] save sphere shape 0x%" PRIXPTR, (void*)shape);
			core::umutex<std::mutex> unique(exclusive);
			shapes[shape] = 1;
			return shape;
#else
			return nullptr;
#endif
		}
		btCollisionShape* simulator::create_capsule(float radius, float height)
		{
#ifdef VI_BULLET3
			btCollisionShape* shape = core::memory::init<btCapsuleShape>(radius, height);
			VI_TRACE("[sim] save capsule shape 0x%" PRIXPTR, (void*)shape);
			core::umutex<std::mutex> unique(exclusive);
			shapes[shape] = 1;
			return shape;
#else
			return nullptr;
#endif
		}
		btCollisionShape* simulator::create_cone(float radius, float height)
		{
#ifdef VI_BULLET3
			btCollisionShape* shape = core::memory::init<btConeShape>(radius, height);
			VI_TRACE("[sim] save cone shape 0x%" PRIXPTR, (void*)shape);
			core::umutex<std::mutex> unique(exclusive);
			shapes[shape] = 1;
			return shape;
#else
			return nullptr;
#endif
		}
		btCollisionShape* simulator::create_cylinder(const trigonometry::vector3& scale)
		{
#ifdef VI_BULLET3
			btCollisionShape* shape = core::memory::init<btCylinderShape>(V3_TO_BT(scale));
			VI_TRACE("[sim] save cylinder shape 0x%" PRIXPTR, (void*)shape);
			core::umutex<std::mutex> unique(exclusive);
			shapes[shape] = 1;
			return shape;
#else
			return nullptr;
#endif
		}
		btCollisionShape* simulator::create_convex_hull(core::vector<trigonometry::skin_vertex>& vertices)
		{
#ifdef VI_BULLET3
			btConvexHullShape* shape = core::memory::init<btConvexHullShape>();
			for (auto it = vertices.begin(); it != vertices.end(); ++it)
				shape->addPoint(btVector3(it->position_x, it->position_y, it->position_z), false);

			shape->recalcLocalAabb();
			shape->optimizeConvexHull();
			shape->setMargin(0);

			VI_TRACE("[sim] save convext-hull shape 0x%" PRIXPTR " (%" PRIu64 " vertices)", (void*)shape, (uint64_t)vertices.size());
			core::umutex<std::mutex> unique(exclusive);
			shapes[shape] = 1;
			return shape;
#else
			return nullptr;
#endif
		}
		btCollisionShape* simulator::create_convex_hull(core::vector<trigonometry::vertex>& vertices)
		{
#ifdef VI_BULLET3
			btConvexHullShape* shape = core::memory::init<btConvexHullShape>();
			for (auto it = vertices.begin(); it != vertices.end(); ++it)
				shape->addPoint(btVector3(it->position_x, it->position_y, it->position_z), false);

			shape->recalcLocalAabb();
			shape->optimizeConvexHull();
			shape->setMargin(0);

			VI_TRACE("[sim] save convext-hull shape 0x%" PRIXPTR " (%" PRIu64 " vertices)", (void*)shape, (uint64_t)vertices.size());
			core::umutex<std::mutex> unique(exclusive);
			shapes[shape] = 1;
			return shape;
#else
			return nullptr;
#endif
		}
		btCollisionShape* simulator::create_convex_hull(core::vector<trigonometry::vector2>& vertices)
		{
#ifdef VI_BULLET3
			btConvexHullShape* shape = core::memory::init<btConvexHullShape>();
			for (auto it = vertices.begin(); it != vertices.end(); ++it)
				shape->addPoint(btVector3(it->x, it->y, 0), false);

			shape->recalcLocalAabb();
			shape->optimizeConvexHull();
			shape->setMargin(0);

			VI_TRACE("[sim] save convext-hull shape 0x%" PRIXPTR " (%" PRIu64 " vertices)", (void*)shape, (uint64_t)vertices.size());
			core::umutex<std::mutex> unique(exclusive);
			shapes[shape] = 1;
			return shape;
#else
			return nullptr;
#endif
		}
		btCollisionShape* simulator::create_convex_hull(core::vector<trigonometry::vector3>& vertices)
		{
#ifdef VI_BULLET3
			btConvexHullShape* shape = core::memory::init<btConvexHullShape>();
			for (auto it = vertices.begin(); it != vertices.end(); ++it)
				shape->addPoint(btVector3(it->x, it->y, it->z), false);

			shape->recalcLocalAabb();
			shape->optimizeConvexHull();
			shape->setMargin(0);

			VI_TRACE("[sim] save convext-hull shape 0x%" PRIXPTR " (%" PRIu64 " vertices)", (void*)shape, (uint64_t)vertices.size());
			core::umutex<std::mutex> unique(exclusive);
			shapes[shape] = 1;
			return shape;
#else
			return nullptr;
#endif
		}
		btCollisionShape* simulator::create_convex_hull(core::vector<trigonometry::vector4>& vertices)
		{
#ifdef VI_BULLET3
			btConvexHullShape* shape = core::memory::init<btConvexHullShape>();
			for (auto it = vertices.begin(); it != vertices.end(); ++it)
				shape->addPoint(btVector3(it->x, it->y, it->z), false);

			shape->recalcLocalAabb();
			shape->optimizeConvexHull();
			shape->setMargin(0);

			VI_TRACE("[sim] save convext-hull shape 0x%" PRIXPTR " (%" PRIu64 " vertices)", (void*)shape, (uint64_t)vertices.size());
			core::umutex<std::mutex> unique(exclusive);
			shapes[shape] = 1;
			return shape;
#else
			return nullptr;
#endif
		}
		btCollisionShape* simulator::create_convex_hull(btCollisionShape* from)
		{
#ifdef VI_BULLET3
			VI_ASSERT(from != nullptr, "shape should be set");
			VI_ASSERT(from->getShapeType() == (int)shape::convex_hull, "shape type should be convex hull");

			btConvexHullShape* hull = core::memory::init<btConvexHullShape>();
			btConvexHullShape* base = (btConvexHullShape*)from;

			for (size_t i = 0; i < (size_t)base->getNumPoints(); i++)
				hull->addPoint(*(base->getUnscaledPoints() + i), false);

			hull->recalcLocalAabb();
			hull->optimizeConvexHull();
			hull->setMargin(0);

			VI_TRACE("[sim] save convext-hull shape 0x%" PRIXPTR " (%" PRIu64 " vertices)", (void*)hull, (uint64_t)base->getNumPoints());
			core::umutex<std::mutex> unique(exclusive);
			shapes[hull] = 1;
			return hull;
#else
			return nullptr;
#endif
		}
		btCollisionShape* simulator::create_shape(shape wanted)
		{
#ifdef VI_BULLET3
			switch (wanted)
			{
				case shape::box:
					return create_cube();
				case shape::sphere:
					return create_sphere();
				case shape::capsule:
					return create_capsule();
				case shape::cone:
					return create_cone();
				case shape::cylinder:
					return create_cylinder();
				default:
					return nullptr;
			}
#else
			return nullptr;
#endif
		}
		btCollisionShape* simulator::try_clone_shape(btCollisionShape* value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(value != nullptr, "shape should be set");
			shape type = (shape)value->getShapeType();
			if (type == shape::box)
			{
				btBoxShape* box = (btBoxShape*)value;
				btVector3 size = box->getHalfExtentsWithMargin() / box->getLocalScaling();
				return create_cube(BT_TO_V3(size));
			}
			else if (type == shape::sphere)
			{
				btSphereShape* sphere = (btSphereShape*)value;
				return create_sphere(sphere->getRadius());
			}
			else if (type == shape::capsule)
			{
				btCapsuleShape* capsule = (btCapsuleShape*)value;
				return create_capsule(capsule->getRadius(), capsule->getHalfHeight() * 2.0f);
			}
			else if (type == shape::cone)
			{
				btConeShape* cone = (btConeShape*)value;
				return create_cone(cone->getRadius(), cone->getHeight());
			}
			else if (type == shape::cylinder)
			{
				btCylinderShape* cylinder = (btCylinderShape*)value;
				btVector3 size = cylinder->getHalfExtentsWithMargin() / cylinder->getLocalScaling();
				return create_cylinder(BT_TO_V3(size));
			}
			else if (type == shape::convex_hull)
				return create_convex_hull(value);

			return nullptr;
#else
			return nullptr;
#endif
		}
		btCollisionShape* simulator::reuse_shape(btCollisionShape* value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(value != nullptr, "shape should be set");
			core::umutex<std::mutex> unique(exclusive);
			auto it = shapes.find(value);
			if (it == shapes.end())
				return nullptr;

			it->second++;
			return value;
#else
			return nullptr;
#endif
		}
		void simulator::free_shape(btCollisionShape** value)
		{
#ifdef VI_BULLET3
			if (!value || !*value)
				return;

			core::umutex<std::mutex> unique(exclusive);
			auto it = shapes.find(*value);
			if (it == shapes.end())
				return;

			*value = nullptr;
			if (it->second-- > 1)
				return;

			btCollisionShape* item = (btCollisionShape*)it->first;
			VI_TRACE("[sim] free shape 0x%" PRIXPTR, (void*)item);
			core::memory::deinit(item);
			shapes.erase(it);
#endif
		}
		core::vector<trigonometry::vector3> simulator::get_shape_vertices(btCollisionShape* value) const
		{
#ifdef VI_BULLET3
			VI_ASSERT(value != nullptr, "shape should be set");
			auto type = (shape)value->getShapeType();
			if (type != shape::convex_hull)
				return core::vector<trigonometry::vector3>();

			btConvexHullShape* hull = (btConvexHullShape*)value;
			btVector3* points = hull->getUnscaledPoints();
			size_t count = hull->getNumPoints();
			core::vector<trigonometry::vector3> vertices;
			vertices.reserve(count);

			for (size_t i = 0; i < count; i++)
			{
				btVector3& it = points[i];
				vertices.emplace_back(it.getX(), it.getY(), it.getZ());
			}

			return vertices;
#else
			return core::vector<trigonometry::vector3>();
#endif
		}
		size_t simulator::get_shape_vertices_count(btCollisionShape* value) const
		{
#ifdef VI_BULLET3
			VI_ASSERT(value != nullptr, "shape should be set");
			auto type = (shape)value->getShapeType();
			if (type != shape::convex_hull)
				return 0;

			btConvexHullShape* hull = (btConvexHullShape*)value;
			return hull->getNumPoints();
#else
			return 0;
#endif
		}
		float simulator::get_max_displacement() const
		{
#ifdef VI_BULLET3
			if (!soft_solver || !world)
				return 1000;

			return ((btSoftRigidDynamicsWorld*)world)->getWorldInfo().m_maxDisplacement;
#else
			return 1000;
#endif
		}
		float simulator::get_air_density() const
		{
#ifdef VI_BULLET3
			if (!soft_solver || !world)
				return 1.2f;

			return ((btSoftRigidDynamicsWorld*)world)->getWorldInfo().air_density;
#else
			return 1.2f;
#endif
		}
		float simulator::get_water_offset() const
		{
#ifdef VI_BULLET3
			if (!soft_solver || !world)
				return 0;

			return ((btSoftRigidDynamicsWorld*)world)->getWorldInfo().water_offset;
#else
			return 0;
#endif
		}
		float simulator::get_water_density() const
		{
#ifdef VI_BULLET3
			if (!soft_solver || !world)
				return 0;

			return ((btSoftRigidDynamicsWorld*)world)->getWorldInfo().water_density;
#else
			return 0;
#endif
		}
		trigonometry::vector3 simulator::get_water_normal() const
		{
#ifdef VI_BULLET3
			if (!soft_solver || !world)
				return 0;

			btVector3 value = ((btSoftRigidDynamicsWorld*)world)->getWorldInfo().water_normal;
			return BT_TO_V3(value);
#else
			return 0;
#endif
		}
		trigonometry::vector3 simulator::get_gravity() const
		{
#ifdef VI_BULLET3
			if (!world)
				return 0;

			btVector3 value = world->getGravity();
			return BT_TO_V3(value);
#else
			return 0;
#endif
		}
		contact_started_callback simulator::get_on_collision_enter() const
		{
#ifdef VI_BULLET3
			return gContactStartedCallback;
#else
			return nullptr;
#endif
		}
		contact_ended_callback simulator::get_on_collision_exit() const
		{
#ifdef VI_BULLET3
			return gContactEndedCallback;
#else
			return nullptr;
#endif
		}
		btCollisionConfiguration* simulator::get_collision() const
		{
#ifdef VI_BULLET3
			return collision;
#else
			return nullptr;
#endif
		}
		btBroadphaseInterface* simulator::get_broadphase() const
		{
#ifdef VI_BULLET3
			return broadphase;
#else
			return nullptr;
#endif
		}
		btConstraintSolver* simulator::get_solver() const
		{
#ifdef VI_BULLET3
			return solver;
#else
			return nullptr;
#endif
		}
		btDiscreteDynamicsWorld* simulator::get_world() const
		{
#ifdef VI_BULLET3
			return world;
#else
			return nullptr;
#endif
		}
		btCollisionDispatcher* simulator::get_dispatcher() const
		{
#ifdef VI_BULLET3
			return dispatcher;
#else
			return nullptr;
#endif
		}
		btSoftBodySolver* simulator::get_soft_solver() const
		{
#ifdef VI_BULLET3
			return soft_solver;
#else
			return nullptr;
#endif
		}
		bool simulator::has_soft_body_support() const
		{
#ifdef VI_BULLET3
			return soft_solver != nullptr;
#else
			return false;
#endif
		}
		int simulator::get_contact_manifold_count() const
		{
#ifdef VI_BULLET3
			if (!dispatcher)
				return 0;

			return dispatcher->getNumManifolds();
#else
			return 0;
#endif
		}
		simulator* simulator::get(btDiscreteDynamicsWorld* from)
		{
#ifdef VI_BULLET3
			if (!from)
				return nullptr;

			return (simulator*)from->getWorldUserInfo();
#else
			return nullptr;
#endif
		}
	}
}
