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
#define V3_TO_BT(V) btVector3(V.X, V.Y, V.Z)
#define BT_TO_V3(V) Vitex::Trigonometry::Vector3(V.getX(), V.getY(), V.getZ())
#define Q4_TO_BT(V) btQuaternion(V.X, V.Y, V.Z, V.W)
#define BT_TO_Q4(V) Vitex::Trigonometry::Quaternion(V.getX(), V.getY(), V.getZ(), V.getW())

namespace
{
#ifdef VI_BULLET3
	class FindContactsHandler : public btCollisionWorld::ContactResultCallback
	{
	public:
		int(*Callback)(Vitex::Physics::ShapeContact*, const Vitex::Physics::CollisionBody&, const Vitex::Physics::CollisionBody&) = nullptr;

	public:
		btScalar addSingleResult(btManifoldPoint& Point, const btCollisionObjectWrapper* Object1, int PartId0, int Index0, const btCollisionObjectWrapper* Object2, int PartId1, int Index1) override
		{
			using namespace Vitex::Physics;
			using namespace Vitex::Trigonometry;
			VI_ASSERT(Callback && Object1 && Object1->getCollisionObject() && Object2 && Object2->getCollisionObject(), "collision objects are not in available condition");

			auto& PWOA = Point.getPositionWorldOnA();
			auto& PWOB = Point.getPositionWorldOnB();
			ShapeContact Contact;
			Contact.LocalPoint1 = Vector3(Point.m_localPointA.getX(), Point.m_localPointA.getY(), Point.m_localPointA.getZ());
			Contact.LocalPoint2 = Vector3(Point.m_localPointB.getX(), Point.m_localPointB.getY(), Point.m_localPointB.getZ());
			Contact.PositionWorld1 = Vector3(PWOA.getX(), PWOA.getY(), PWOA.getZ());
			Contact.PositionWorld2 = Vector3(PWOB.getX(), PWOB.getY(), PWOB.getZ());
			Contact.NormalWorld = Vector3(Point.m_normalWorldOnB.getX(), Point.m_normalWorldOnB.getY(), Point.m_normalWorldOnB.getZ());
			Contact.LateralFrictionDirection1 = Vector3(Point.m_lateralFrictionDir1.getX(), Point.m_lateralFrictionDir1.getY(), Point.m_lateralFrictionDir1.getZ());
			Contact.LateralFrictionDirection2 = Vector3(Point.m_lateralFrictionDir2.getX(), Point.m_lateralFrictionDir2.getY(), Point.m_lateralFrictionDir2.getZ());
			Contact.Distance = Point.m_distance1;
			Contact.CombinedFriction = Point.m_combinedFriction;
			Contact.CombinedRollingFriction = Point.m_combinedRollingFriction;
			Contact.CombinedSpinningFriction = Point.m_combinedSpinningFriction;
			Contact.CombinedRestitution = Point.m_combinedRestitution;
			Contact.AppliedImpulse = Point.m_appliedImpulse;
			Contact.AppliedImpulseLateral1 = Point.m_appliedImpulseLateral1;
			Contact.AppliedImpulseLateral2 = Point.m_appliedImpulseLateral2;
			Contact.ContactMotion1 = Point.m_contactMotion1;
			Contact.ContactMotion2 = Point.m_contactMotion2;
			Contact.ContactCFM = Point.m_contactCFM;
			Contact.CombinedContactStiffness = Point.m_combinedContactStiffness1;
			Contact.ContactERP = Point.m_contactERP;
			Contact.CombinedContactDamping = Point.m_combinedContactDamping1;
			Contact.FrictionCFM = Point.m_frictionCFM;
			Contact.PartId1 = Point.m_partId0;
			Contact.PartId2 = Point.m_partId1;
			Contact.Index1 = Point.m_index0;
			Contact.Index2 = Point.m_index1;
			Contact.ContactPointFlags = Point.m_contactPointFlags;
			Contact.LifeTime = Point.m_lifeTime;

			btCollisionObject* Body1 = (btCollisionObject*)Object1->getCollisionObject();
			btCollisionObject* Body2 = (btCollisionObject*)Object2->getCollisionObject();
			return (btScalar)Callback(&Contact, CollisionBody(Body1), CollisionBody(Body2));
		}
	};

	class FindRayContactsHandler : public btCollisionWorld::RayResultCallback
	{
	public:
		int(*Callback)(Vitex::Physics::RayContact*, const Vitex::Physics::CollisionBody&) = nullptr;

	public:
		btScalar addSingleResult(btCollisionWorld::LocalRayResult& RayResult, bool NormalInWorldSpace) override
		{
			using namespace Vitex::Physics;
			using namespace Vitex::Trigonometry;
			VI_ASSERT(Callback && RayResult.m_collisionObject, "collision objects are not in available condition");

			RayContact Contact;
			Contact.HitNormalLocal = BT_TO_V3(RayResult.m_hitNormalLocal);
			Contact.NormalInWorldSpace = NormalInWorldSpace;
			Contact.HitFraction = RayResult.m_hitFraction;
			Contact.ClosestHitFraction = m_closestHitFraction;

			btCollisionObject* Body1 = (btCollisionObject*)RayResult.m_collisionObject;
			return (btScalar)Callback(&Contact, CollisionBody(Body1));
		}
	};

	btTransform M16_TO_BT(const Vitex::Trigonometry::Matrix4x4& In)
	{
		btMatrix3x3 Offset;
		Offset[0][0] = In.Row[0];
		Offset[1][0] = In.Row[1];
		Offset[2][0] = In.Row[2];
		Offset[0][1] = In.Row[4];
		Offset[1][1] = In.Row[5];
		Offset[2][1] = In.Row[6];
		Offset[0][2] = In.Row[8];
		Offset[1][2] = In.Row[9];
		Offset[2][2] = In.Row[10];

		btTransform Result;
		Result.setBasis(Offset);

		Vitex::Trigonometry::Vector3 Position = In.Position();
		Result.setOrigin(V3_TO_BT(Position));

		return Result;
	}
#endif
	size_t OffsetOf64(const char* Source, char Dest)
	{
		VI_ASSERT(Source != nullptr, "source should be set");
		for (size_t i = 0; i < 64; i++)
		{
			if (Source[i] == Dest)
				return i;
		}

		return 63;
	}
	Vitex::Core::String EscapeText(const Vitex::Core::String& Data)
	{
		Vitex::Core::String Result = "\"";
		Result.append(Data).append("\"");
		return Result;
	}
}

namespace Vitex
{
	namespace Physics
	{
		CollisionBody::CollisionBody(btCollisionObject* Object) noexcept
		{
#ifdef VI_BULLET3
			btRigidBody* RigidObject = btRigidBody::upcast(Object);
			if (RigidObject != nullptr)
				Rigid = (RigidBody*)RigidObject->getUserPointer();

			btSoftBody* SoftObject = btSoftBody::upcast(Object);
			if (SoftObject != nullptr)
				Soft = (SoftBody*)SoftObject->getUserPointer();
#endif
		}

		HullShape::HullShape(Core::Vector<Trigonometry::Vertex>&& NewVertices, Core::Vector<int>&& NewIndices) noexcept : Vertices(std::move(NewVertices)), Indices(std::move(NewIndices)), Shape(nullptr)
		{
#ifdef VI_BULLET3
			Shape = Core::Memory::New<btConvexHullShape>();
			btConvexHullShape* Hull = (btConvexHullShape*)Shape;
			for (auto& Item : Vertices)
				Hull->addPoint(btVector3(Item.PositionX, Item.PositionY, Item.PositionZ), false);

			Hull->recalcLocalAabb();
			Hull->optimizeConvexHull();
			Hull->setMargin(0);
#endif
		}
		HullShape::HullShape(Core::Vector<Trigonometry::Vertex>&& NewVertices) noexcept : Vertices(std::move(NewVertices)), Shape(nullptr)
		{
#ifdef VI_BULLET3
			Shape = Core::Memory::New<btConvexHullShape>();
			btConvexHullShape* Hull = (btConvexHullShape*)Shape;
			Indices.reserve(Vertices.size());

			for (auto& Item : Vertices)
			{
				Hull->addPoint(btVector3(Item.PositionX, Item.PositionY, Item.PositionZ), false);
				Indices.push_back((int)Indices.size());
			}

			Hull->recalcLocalAabb();
			Hull->optimizeConvexHull();
			Hull->setMargin(0);
#endif
		}
		HullShape::HullShape(btCollisionShape* From) noexcept : Shape(nullptr)
		{
#ifdef VI_BULLET3
			VI_ASSERT(From != nullptr, "shape should be set");
			VI_ASSERT(From->getShapeType() == (int)Shape::Convex_Hull, "shape type should be convex hull");

			btConvexHullShape* Hull = Core::Memory::New<btConvexHullShape>();
			btConvexHullShape* Base = (btConvexHullShape*)From;
			Vertices.reserve((size_t)Base->getNumPoints());
			Indices.reserve((size_t)Base->getNumPoints());

			for (size_t i = 0; i < (size_t)Base->getNumPoints(); i++)
			{
				auto& Position = *(Base->getUnscaledPoints() + i);
				Hull->addPoint(Position, false);
				Vertices.push_back({ Position.x(), Position.y(), Position.z() });
				Indices.push_back((int)i);
			}

			Hull->recalcLocalAabb();
			Hull->optimizeConvexHull();
			Hull->setMargin(0);
#endif
		}
		HullShape::~HullShape() noexcept
		{
#ifdef VI_BULLET3
			Core::Memory::Delete(Shape);
#endif
		}
		const Core::Vector<Trigonometry::Vertex>& HullShape::GetVertices() const
		{
			return Vertices;
		}
		const Core::Vector<int>& HullShape::GetIndices() const
		{
			return Indices;
		}
		btCollisionShape* HullShape::GetShape() const
		{
			return Shape;
		}

		RigidBody::RigidBody(Simulator* Refer, const Desc& I) noexcept : Instance(nullptr), Engine(Refer), Initial(I), UserPointer(nullptr)
		{
			VI_ASSERT(Initial.Shape, "collision shape should be set");
			VI_ASSERT(Engine != nullptr, "simulator should be set");
#ifdef VI_BULLET3
			Initial.Shape = Engine->ReuseShape(Initial.Shape);
			if (!Initial.Shape)
			{
				Initial.Shape = Engine->TryCloneShape(I.Shape);
				if (!Initial.Shape)
					return;
			}

			btVector3 LocalInertia(0, 0, 0);
			Initial.Shape->setLocalScaling(V3_TO_BT(Initial.Scale));
			if (Initial.Mass > 0)
				Initial.Shape->calculateLocalInertia(Initial.Mass, LocalInertia);

			btQuaternion Rotation;
			Rotation.setEulerZYX(Initial.Rotation.Z, Initial.Rotation.Y, Initial.Rotation.X);

			btTransform BtTransform(Rotation, btVector3(Initial.Position.X, Initial.Position.Y, Initial.Position.Z));
			btRigidBody::btRigidBodyConstructionInfo Info(Initial.Mass, Core::Memory::New<btDefaultMotionState>(BtTransform), Initial.Shape, LocalInertia);
			Instance = Core::Memory::New<btRigidBody>(Info);
			Instance->setUserPointer(this);
			Instance->setGravity(Engine->GetWorld()->getGravity());

			if (Initial.Anticipation > 0 && Initial.Mass > 0)
			{
				Instance->setCcdMotionThreshold(Initial.Anticipation);
				Instance->setCcdSweptSphereRadius(Initial.Scale.Length() / 15.0f);
			}

			if (Instance->getWorldArrayIndex() == -1)
				Engine->GetWorld()->addRigidBody(Instance);
#endif
		}
		RigidBody::~RigidBody() noexcept
		{
#ifdef VI_BULLET3
			if (!Instance)
				return;

			int Constraints = Instance->getNumConstraintRefs();
			for (int i = 0; i < Constraints; i++)
			{
				btTypedConstraint* Constraint = Instance->getConstraintRef(i);
				if (Constraint != nullptr)
				{
					void* Ptr = Constraint->getUserConstraintPtr();
					if (Ptr != nullptr)
					{
						btTypedConstraintType Type = Constraint->getConstraintType();
						switch (Type)
						{
							case SLIDER_CONSTRAINT_TYPE:
								if (((SConstraint*)Ptr)->First == Instance)
									((SConstraint*)Ptr)->First = nullptr;
								else if (((SConstraint*)Ptr)->Second == Instance)
									((SConstraint*)Ptr)->Second = nullptr;
								break;
							default:
								break;
						}
					}

					Instance->removeConstraintRef(Constraint);
					Constraints--; i--;
				}
			}

			if (Instance->getMotionState())
			{
				btMotionState* Object = Instance->getMotionState();
				Core::Memory::Delete(Object);
				Instance->setMotionState(nullptr);
			}

			Instance->setCollisionShape(nullptr);
			Instance->setUserPointer(nullptr);
			if (Instance->getWorldArrayIndex() >= 0)
				Engine->GetWorld()->removeRigidBody(Instance);

			if (Initial.Shape)
				Engine->FreeShape(&Initial.Shape);

			Core::Memory::Delete(Instance);
#endif
		}
		RigidBody* RigidBody::Copy()
		{
			VI_ASSERT(Instance != nullptr, "rigidbody should be initialized");

			Desc I(Initial);
			I.Position = GetPosition();
			I.Rotation = GetRotation();
			I.Scale = GetScale();
			I.Mass = GetMass();
			I.Shape = Engine->TryCloneShape(I.Shape);

			RigidBody* Target = new RigidBody(Engine, I);
			Target->SetSpinningFriction(GetSpinningFriction());
			Target->SetContactDamping(GetContactDamping());
			Target->SetContactStiffness(GetContactStiffness());
			Target->SetActivationState(GetActivationState());
			Target->SetAngularDamping(GetAngularDamping());
			Target->SetAngularSleepingThreshold(GetAngularSleepingThreshold());
			Target->SetFriction(GetFriction());
			Target->SetRestitution(GetRestitution());
			Target->SetHitFraction(GetHitFraction());
			Target->SetLinearDamping(GetLinearDamping());
			Target->SetLinearSleepingThreshold(GetLinearSleepingThreshold());
			Target->SetCcdMotionThreshold(GetCcdMotionThreshold());
			Target->SetCcdSweptSphereRadius(GetCcdSweptSphereRadius());
			Target->SetContactProcessingThreshold(GetContactProcessingThreshold());
			Target->SetDeactivationTime(GetDeactivationTime());
			Target->SetRollingFriction(GetRollingFriction());
			Target->SetAngularFactor(GetAngularFactor());
			Target->SetAnisotropicFriction(GetAnisotropicFriction());
			Target->SetGravity(GetGravity());
			Target->SetLinearFactor(GetLinearFactor());
			Target->SetLinearVelocity(GetLinearVelocity());
			Target->SetAngularVelocity(GetAngularVelocity());
			Target->SetCollisionFlags(GetCollisionFlags());

			return Target;
		}
		void RigidBody::Push(const Trigonometry::Vector3& Velocity)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "rigidbody should be initialized");
			Instance->applyCentralImpulse(V3_TO_BT(Velocity));
#endif
		}
		void RigidBody::Push(const Trigonometry::Vector3& Velocity, const Trigonometry::Vector3& Torque)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "rigidbody should be initialized");
			Instance->applyCentralImpulse(V3_TO_BT(Velocity));
			Instance->applyTorqueImpulse(V3_TO_BT(Torque));
#endif
		}
		void RigidBody::Push(const Trigonometry::Vector3& Velocity, const Trigonometry::Vector3& Torque, const Trigonometry::Vector3& Center)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "rigidbody should be initialized");
			Instance->applyImpulse(V3_TO_BT(Velocity), V3_TO_BT(Center));
			Instance->applyTorqueImpulse(V3_TO_BT(Torque));
#endif
		}
		void RigidBody::PushKinematic(const Trigonometry::Vector3& Velocity)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "rigidbody should be initialized");

			btTransform Offset;
			Instance->getMotionState()->getWorldTransform(Offset);

			btVector3 Origin = Offset.getOrigin();
			Origin.setX(Origin.getX() + Velocity.X);
			Origin.setY(Origin.getY() + Velocity.Y);
			Origin.setZ(Origin.getZ() + Velocity.Z);

			Offset.setOrigin(Origin);
			Instance->getMotionState()->setWorldTransform(Offset);
#endif
		}
		void RigidBody::PushKinematic(const Trigonometry::Vector3& Velocity, const Trigonometry::Vector3& Torque)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "rigidbody should be initialized");

			btTransform Offset;
			Instance->getMotionState()->getWorldTransform(Offset);

			btScalar X, Y, Z;
			Offset.getRotation().getEulerZYX(Z, Y, X);

			Trigonometry::Vector3 Rotation(-X, -Y, Z);
			Offset.getBasis().setEulerZYX(Rotation.X + Torque.X, Rotation.Y + Torque.Y, Rotation.Z + Torque.Z);

			btVector3 Origin = Offset.getOrigin();
			Origin.setX(Origin.getX() + Velocity.X);
			Origin.setY(Origin.getY() + Velocity.Y);
			Origin.setZ(Origin.getZ() + Velocity.Z);

			Offset.setOrigin(Origin);
			Instance->getMotionState()->setWorldTransform(Offset);
#endif
		}
		void RigidBody::Synchronize(Trigonometry::Transform* Transform, bool Kinematic)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "rigidbody should be initialized");
			btTransform& Base = Instance->getWorldTransform();
			if (!Kinematic)
			{
				btScalar X, Y, Z;
				const btVector3& Position = Base.getOrigin();
				const btVector3& Scale = Instance->getCollisionShape()->getLocalScaling();
				Base.getRotation().getEulerZYX(Z, Y, X);
				Transform->SetPosition(BT_TO_V3(Position));
				Transform->SetRotation(Trigonometry::Vector3(X, Y, Z));
				Transform->SetScale(BT_TO_V3(Scale));
			}
			else
			{
				Trigonometry::Transform::Spacing& Space = Transform->GetSpacing(Trigonometry::Positioning::Global);
				Base.setOrigin(V3_TO_BT(Space.Position));
				Base.getBasis().setEulerZYX(Space.Rotation.X, Space.Rotation.Y, Space.Rotation.Z);
				Instance->getCollisionShape()->setLocalScaling(V3_TO_BT(Space.Scale));
			}
#endif
		}
		void RigidBody::SetActivity(bool Active)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "rigidbody should be initialized");
			if (GetActivationState() == MotionState::Disable_Deactivation)
				return;

			if (Active)
			{
				Instance->forceActivationState((int)MotionState::Active);
				Instance->activate(true);
			}
			else
				Instance->forceActivationState((int)MotionState::Deactivation_Needed);
#endif
		}
		void RigidBody::SetAsGhost()
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "rigidbody should be initialized");
			Instance->setCollisionFlags(btCollisionObject::CF_NO_CONTACT_RESPONSE);
#endif
		}
		void RigidBody::SetAsNormal()
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "rigidbody should be initialized");
			Instance->setCollisionFlags(0);
#endif
		}
		void RigidBody::SetSelfPointer()
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "rigidbody should be initialized");
			Instance->setUserPointer(this);
#endif
		}
		void RigidBody::SetWorldTransform(btTransform* Value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "rigidbody should be initialized");
			VI_ASSERT(Value != nullptr, "transform should be set");
			Instance->setWorldTransform(*Value);
#endif
		}
		void RigidBody::SetActivationState(MotionState Value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "rigidbody should be initialized");
			Instance->forceActivationState((int)Value);
#endif
		}
		void RigidBody::SetAngularDamping(float Value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "rigidbody should be initialized");
			Instance->setDamping(Instance->getLinearDamping(), Value);
#endif
		}
		void RigidBody::SetAngularSleepingThreshold(float Value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "rigidbody should be initialized");
			Instance->setSleepingThresholds(Instance->getLinearSleepingThreshold(), Value);
#endif
		}
		void RigidBody::SetFriction(float Value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "rigidbody should be initialized");
			Instance->setFriction(Value);
#endif
		}
		void RigidBody::SetRestitution(float Value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "rigidbody should be initialized");
			Instance->setRestitution(Value);
#endif
		}
		void RigidBody::SetSpinningFriction(float Value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "rigidbody should be initialized");
			Instance->setSpinningFriction(Value);
#endif
		}
		void RigidBody::SetContactStiffness(float Value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "rigidbody should be initialized");
			Instance->setContactStiffnessAndDamping(Value, Instance->getContactDamping());
#endif
		}
		void RigidBody::SetContactDamping(float Value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "rigidbody should be initialized");
			Instance->setContactStiffnessAndDamping(Instance->getContactStiffness(), Value);
#endif
		}
		void RigidBody::SetHitFraction(float Value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "rigidbody should be initialized");
			Instance->setHitFraction(Value);
#endif
		}
		void RigidBody::SetLinearDamping(float Value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "rigidbody should be initialized");
			Instance->setDamping(Value, Instance->getAngularDamping());
#endif
		}
		void RigidBody::SetLinearSleepingThreshold(float Value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "rigidbody should be initialized");
			Instance->setSleepingThresholds(Value, Instance->getAngularSleepingThreshold());
#endif
		}
		void RigidBody::SetCcdMotionThreshold(float Value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "rigidbody should be initialized");
			Instance->setCcdMotionThreshold(Value);
#endif
		}
		void RigidBody::SetCcdSweptSphereRadius(float Value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "rigidbody should be initialized");
			Instance->setCcdSweptSphereRadius(Value);
#endif
		}
		void RigidBody::SetContactProcessingThreshold(float Value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "rigidbody should be initialized");
			Instance->setContactProcessingThreshold(Value);
#endif
		}
		void RigidBody::SetDeactivationTime(float Value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "rigidbody should be initialized");
			Instance->setDeactivationTime(Value);
#endif
		}
		void RigidBody::SetRollingFriction(float Value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "rigidbody should be initialized");
			Instance->setRollingFriction(Value);
#endif
		}
		void RigidBody::SetAngularFactor(const Trigonometry::Vector3& Value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "rigidbody should be initialized");
			Instance->setAngularFactor(V3_TO_BT(Value));
#endif
		}
		void RigidBody::SetAnisotropicFriction(const Trigonometry::Vector3& Value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "rigidbody should be initialized");
			Instance->setAnisotropicFriction(V3_TO_BT(Value));
#endif
		}
		void RigidBody::SetGravity(const Trigonometry::Vector3& Value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "rigidbody should be initialized");
			Instance->setGravity(V3_TO_BT(Value));
#endif
		}
		void RigidBody::SetLinearFactor(const Trigonometry::Vector3& Value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "rigidbody should be initialized");
			Instance->setLinearFactor(V3_TO_BT(Value));
#endif
		}
		void RigidBody::SetLinearVelocity(const Trigonometry::Vector3& Value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "rigidbody should be initialized");
			Instance->setLinearVelocity(V3_TO_BT(Value));
#endif
		}
		void RigidBody::SetAngularVelocity(const Trigonometry::Vector3& Value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "rigidbody should be initialized");
			Instance->setAngularVelocity(V3_TO_BT(Value));
#endif
		}
		void RigidBody::SetCollisionShape(btCollisionShape* Shape, Trigonometry::Transform* Transform)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "rigidbody should be initialized");
			btCollisionShape* Collision = Instance->getCollisionShape();
			Core::Memory::Delete(Collision);

			Instance->setCollisionShape(Shape);
			if (Transform)
				Synchronize(Transform, true);
#endif
		}
		void RigidBody::SetMass(float Mass)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr && Engine != nullptr, "rigidbody should be initialized");
			btVector3 Inertia = Engine->GetWorld()->getGravity();
			if (Instance->getWorldArrayIndex() >= 0)
				Engine->GetWorld()->removeRigidBody(Instance);

			Instance->setGravity(Inertia);
			Instance->getCollisionShape()->calculateLocalInertia(Mass, Inertia);
			Instance->setMassProps(Mass, Inertia);
			if (Instance->getWorldArrayIndex() == -1)
				Engine->GetWorld()->addRigidBody(Instance);

			SetActivity(true);
#endif
		}
		void RigidBody::SetCollisionFlags(size_t Flags)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "rigidbody should be initialized");
			Instance->setCollisionFlags((int)Flags);
#endif
		}
		MotionState RigidBody::GetActivationState() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "rigidbody should be initialized");
			return (MotionState)Instance->getActivationState();
#else
			return MotionState::Island_Sleeping;
#endif
		}
		Shape RigidBody::GetCollisionShapeType() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr && Instance->getCollisionShape() != nullptr, "rigidbody should be initialized");
			return (Shape)Instance->getCollisionShape()->getShapeType();
#else
			return Shape::Invalid;
#endif
		}
		Trigonometry::Vector3 RigidBody::GetAngularFactor() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "rigidbody should be initialized");
			btVector3 Value = Instance->getAngularFactor();
			return Trigonometry::Vector3(Value.getX(), Value.getY(), Value.getZ());
#else
			return 0;
#endif
		}
		Trigonometry::Vector3 RigidBody::GetAnisotropicFriction() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "rigidbody should be initialized");
			btVector3 Value = Instance->getAnisotropicFriction();
			return Trigonometry::Vector3(Value.getX(), Value.getY(), Value.getZ());
#else
			return 0;
#endif
		}
		Trigonometry::Vector3 RigidBody::GetGravity() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "rigidbody should be initialized");
			btVector3 Value = Instance->getGravity();
			return Trigonometry::Vector3(Value.getX(), Value.getY(), Value.getZ());
#else
			return 0;
#endif
		}
		Trigonometry::Vector3 RigidBody::GetLinearFactor() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "rigidbody should be initialized");
			btVector3 Value = Instance->getLinearFactor();
			return Trigonometry::Vector3(Value.getX(), Value.getY(), Value.getZ());
#else
			return 0;
#endif
		}
		Trigonometry::Vector3 RigidBody::GetLinearVelocity() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "rigidbody should be initialized");
			btVector3 Value = Instance->getLinearVelocity();
			return Trigonometry::Vector3(Value.getX(), Value.getY(), Value.getZ());
#else
			return 0;
#endif
		}
		Trigonometry::Vector3 RigidBody::GetAngularVelocity() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "rigidbody should be initialized");
			btVector3 Value = Instance->getAngularVelocity();
			return Trigonometry::Vector3(Value.getX(), Value.getY(), Value.getZ());
#else
			return 0;
#endif
		}
		Trigonometry::Vector3 RigidBody::GetScale() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr && Instance->getCollisionShape() != nullptr, "rigidbody should be initialized");
			btVector3 Value = Instance->getCollisionShape()->getLocalScaling();
			return Trigonometry::Vector3(Value.getX(), Value.getY(), Value.getZ());
#else
			return 0;
#endif
		}
		Trigonometry::Vector3 RigidBody::GetPosition() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "rigidbody should be initialized");
			btVector3 Value = Instance->getWorldTransform().getOrigin();
			return Trigonometry::Vector3(Value.getX(), Value.getY(), Value.getZ());
#else
			return 0;
#endif
		}
		Trigonometry::Vector3 RigidBody::GetRotation() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "rigidbody should be initialized");
			btScalar X, Y, Z;
			Instance->getWorldTransform().getBasis().getEulerZYX(Z, Y, X);
			return Trigonometry::Vector3(-X, -Y, Z);
#else
			return 0;
#endif
		}
		btTransform* RigidBody::GetWorldTransform() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "rigidbody should be initialized");
			return &Instance->getWorldTransform();
#else
			return nullptr;
#endif
		}
		btCollisionShape* RigidBody::GetCollisionShape() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "rigidbody should be initialized");
			return Instance->getCollisionShape();
#else
			return nullptr;
#endif
		}
		btRigidBody* RigidBody::Get() const
		{
#ifdef VI_BULLET3
			return Instance;
#else
			return nullptr;
#endif
		}
		bool RigidBody::IsGhost() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "rigidbody should be initialized");
			return (Instance->getCollisionFlags() & btCollisionObject::CF_NO_CONTACT_RESPONSE) != 0;
#else
			return false;
#endif
		}
		bool RigidBody::IsActive() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "rigidbody should be initialized");
			return Instance->isActive();
#else
			return false;
#endif
		}
		bool RigidBody::IsStatic() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "rigidbody should be initialized");
			return Instance->isStaticObject();
#else
			return true;
#endif
		}
		bool RigidBody::IsColliding() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "rigidbody should be initialized");
			return Instance->hasContactResponse();
#else
			return false;
#endif
		}
		float RigidBody::GetSpinningFriction() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "rigidbody should be initialized");
			return Instance->getSpinningFriction();
#else
			return 0;
#endif
		}
		float RigidBody::GetContactStiffness() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "rigidbody should be initialized");
			return Instance->getContactStiffness();
#else
			return 0;
#endif
		}
		float RigidBody::GetContactDamping() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "rigidbody should be initialized");
			return Instance->getContactDamping();
#else
			return 0;
#endif
		}
		float RigidBody::GetAngularDamping() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "rigidbody should be initialized");
			return Instance->getAngularDamping();
#else
			return 0;
#endif
		}
		float RigidBody::GetAngularSleepingThreshold() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "rigidbody should be initialized");
			return Instance->getAngularSleepingThreshold();
#else
			return 0;
#endif
		}
		float RigidBody::GetFriction() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "rigidbody should be initialized");
			return Instance->getFriction();
#else
			return 0;
#endif
		}
		float RigidBody::GetRestitution() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "rigidbody should be initialized");
			return Instance->getRestitution();
#else
			return 0;
#endif
		}
		float RigidBody::GetHitFraction() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "rigidbody should be initialized");
			return Instance->getHitFraction();
#else
			return 0;
#endif
		}
		float RigidBody::GetLinearDamping() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "rigidbody should be initialized");
			return Instance->getLinearDamping();
#else
			return 0;
#endif
		}
		float RigidBody::GetLinearSleepingThreshold() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "rigidbody should be initialized");
			return Instance->getLinearSleepingThreshold();
#else
			return 0;
#endif
		}
		float RigidBody::GetCcdMotionThreshold() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "rigidbody should be initialized");
			return Instance->getCcdMotionThreshold();
#else
			return 0;
#endif
		}
		float RigidBody::GetCcdSweptSphereRadius() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "rigidbody should be initialized");
			return Instance->getCcdSweptSphereRadius();
#else
			return 0;
#endif
		}
		float RigidBody::GetContactProcessingThreshold() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "rigidbody should be initialized");
			return Instance->getContactProcessingThreshold();
#else
			return 0;
#endif
		}
		float RigidBody::GetDeactivationTime() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "rigidbody should be initialized");
			return Instance->getDeactivationTime();
#else
			return 0;
#endif
		}
		float RigidBody::GetRollingFriction() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "rigidbody should be initialized");
			return Instance->getRollingFriction();
#else
			return 0;
#endif
		}
		float RigidBody::GetMass() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "rigidbody should be initialized");
			float Mass = Instance->getInvMass();
			return (Mass != 0.0f ? 1.0f / Mass : 0.0f);
#else
			return 0;
#endif
		}
		size_t RigidBody::GetCollisionFlags() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "rigidbody should be initialized");
			return Instance->getCollisionFlags();
#else
			return 0;
#endif
		}
		RigidBody::Desc& RigidBody::GetInitialState()
		{
			return Initial;
		}
		Simulator* RigidBody::GetSimulator() const
		{
			return Engine;
		}
		RigidBody* RigidBody::Get(btRigidBody* From)
		{
#ifdef VI_BULLET3
			VI_ASSERT(From != nullptr, "rigidbody should be initialized");
			return (RigidBody*)From->getUserPointer();
#else
			return nullptr;
#endif
		}

		SoftBody::SoftBody(Simulator* Refer, const Desc& I) noexcept : Instance(nullptr), Engine(Refer), Initial(I), UserPointer(nullptr)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Engine != nullptr, "engine should be set");
			VI_ASSERT(Engine->HasSoftBodySupport(), "soft body should be supported");

			btQuaternion Rotation;
			Rotation.setEulerZYX(Initial.Rotation.Z, Initial.Rotation.Y, Initial.Rotation.X);

			btTransform BtTransform(Rotation, btVector3(Initial.Position.X, Initial.Position.Y, -Initial.Position.Z));
			btSoftRigidDynamicsWorld* World = (btSoftRigidDynamicsWorld*)Engine->GetWorld();
			btSoftBodyWorldInfo& Info = World->getWorldInfo();
			HullShape* Hull = Initial.Shape.Convex.Hull;

			if (Initial.Shape.Convex.Enabled && Hull != nullptr)
			{
				auto& Positions = Hull->GetVertices();
				Core::Vector<btScalar> Vertices;
				Vertices.resize(Positions.size() * 3);

				for (size_t i = 0; i < Hull->GetVertices().size(); i++)
				{
					const Trigonometry::Vertex& V = Positions[i];
					Vertices[i * 3 + 0] = (btScalar)V.PositionX;
					Vertices[i * 3 + 1] = (btScalar)V.PositionY;
					Vertices[i * 3 + 2] = (btScalar)V.PositionZ;
				}

				auto& Indices = Hull->GetIndices();
				Instance = btSoftBodyHelpers::CreateFromTriMesh(Info, Vertices.data(), Indices.data(), (int)Indices.size() / 3, false);
			}
			else if (Initial.Shape.Ellipsoid.Enabled)
			{
				Instance = btSoftBodyHelpers::CreateEllipsoid(Info, V3_TO_BT(Initial.Shape.Ellipsoid.Center), V3_TO_BT(Initial.Shape.Ellipsoid.Radius), Initial.Shape.Ellipsoid.Count);
			}
			else if (Initial.Shape.Rope.Enabled)
			{
				int FixedAnchors = 0;
				if (Initial.Shape.Rope.StartFixed)
					FixedAnchors |= 1;

				if (Initial.Shape.Rope.EndFixed)
					FixedAnchors |= 2;

				Instance = btSoftBodyHelpers::CreateRope(Info, V3_TO_BT(Initial.Shape.Rope.Start), V3_TO_BT(Initial.Shape.Rope.End), Initial.Shape.Rope.Count, FixedAnchors);
			}
			else
			{
				int FixedCorners = 0;
				if (Initial.Shape.Patch.Corner00Fixed)
					FixedCorners |= 1;

				if (Initial.Shape.Patch.Corner01Fixed)
					FixedCorners |= 2;

				if (Initial.Shape.Patch.Corner10Fixed)
					FixedCorners |= 4;

				if (Initial.Shape.Patch.Corner11Fixed)
					FixedCorners |= 8;

				Instance = btSoftBodyHelpers::CreatePatch(Info, V3_TO_BT(Initial.Shape.Patch.Corner00), V3_TO_BT(Initial.Shape.Patch.Corner10), V3_TO_BT(Initial.Shape.Patch.Corner01), V3_TO_BT(Initial.Shape.Patch.Corner11), Initial.Shape.Patch.CountX, Initial.Shape.Patch.CountY, FixedCorners, Initial.Shape.Patch.GenerateDiagonals);
			}

			if (Initial.Anticipation > 0)
			{
				Instance->setCcdMotionThreshold(Initial.Anticipation);
				Instance->setCcdSweptSphereRadius(Initial.Scale.Length() / 15.0f);
			}

			SetConfig(Initial.Config);
			Instance->randomizeConstraints();
			Instance->setPose(true, true);
			Instance->getCollisionShape()->setMargin(0.04f);
			Instance->transform(BtTransform);
			Instance->setUserPointer(this);
			Instance->setTotalMass(100.0f, true);

			if (Instance->getWorldArrayIndex() == -1)
				World->addSoftBody(Instance);
#endif
		}
		SoftBody::~SoftBody() noexcept
		{
#ifdef VI_BULLET3
			if (!Instance)
				return;

			btSoftRigidDynamicsWorld* World = (btSoftRigidDynamicsWorld*)Engine->GetWorld();
			if (Instance->getWorldArrayIndex() >= 0)
				World->removeSoftBody(Instance);

			Instance->setUserPointer(nullptr);
			Core::Memory::Delete(Instance);
#endif
		}
		SoftBody* SoftBody::Copy()
		{
			VI_ASSERT(Instance != nullptr, "softbody should be initialized");

			Desc I(Initial);
			I.Position = GetCenterPosition();
			I.Rotation = GetRotation();
			I.Scale = GetScale();

			SoftBody* Target = new SoftBody(Engine, I);
			Target->SetSpinningFriction(GetSpinningFriction());
			Target->SetContactDamping(GetContactDamping());
			Target->SetContactStiffness(GetContactStiffness());
			Target->SetActivationState(GetActivationState());
			Target->SetFriction(GetFriction());
			Target->SetRestitution(GetRestitution());
			Target->SetHitFraction(GetHitFraction());
			Target->SetCcdMotionThreshold(GetCcdMotionThreshold());
			Target->SetCcdSweptSphereRadius(GetCcdSweptSphereRadius());
			Target->SetContactProcessingThreshold(GetContactProcessingThreshold());
			Target->SetDeactivationTime(GetDeactivationTime());
			Target->SetRollingFriction(GetRollingFriction());
			Target->SetAnisotropicFriction(GetAnisotropicFriction());
			Target->SetWindVelocity(GetWindVelocity());
			Target->SetContactStiffnessAndDamping(GetContactStiffness(), GetContactDamping());
			Target->SetTotalMass(GetTotalMass());
			Target->SetRestLengthScale(GetRestLengthScale());
			Target->SetVelocity(GetLinearVelocity());

			return Target;
		}
		void SoftBody::Activate(bool Force)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "softbody should be initialized");
			Instance->activate(Force);
#endif
		}
		void SoftBody::Synchronize(Trigonometry::Transform* Transform, bool Kinematic)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "softbody should be initialized");
			VI_ASSERT(Transform != nullptr, "transform should be set");
#ifdef VI_VECTORCLASS
			LOAD_VAL(_r1, 0.0f); LOAD_VAL(_r2, 0.0f);
			for (int i = 0; i < Instance->m_nodes.size(); i++)
			{
				auto& Node = Instance->m_nodes[i];
				_r2.store(Node.m_x.m_floats);
				_r1 += _r2;
			}

			_r1 /= (float)Instance->m_nodes.size();
			_r1.store_partial(3, (float*)&Center);
#else
			Center.Set(0);
			for (int i = 0; i < Instance->m_nodes.size(); i++)
			{
				auto& Node = Instance->m_nodes[i];
				Center.X += Node.m_x.x();
				Center.Y += Node.m_x.y();
				Center.Z += Node.m_x.z();
			}
			Center /= (float)Instance->m_nodes.size();
#endif
			if (!Kinematic)
			{
				btScalar X, Y, Z;
				Instance->getWorldTransform().getRotation().getEulerZYX(Z, Y, X);
				Transform->SetPosition(Center.InvZ());
				Transform->SetRotation(Trigonometry::Vector3(X, Y, Z));
			}
			else
			{
				Trigonometry::Vector3 Position = Transform->GetPosition().InvZ() - Center;
				if (Position.Length() > 0.005f)
					Instance->translate(V3_TO_BT(Position));
			}
#endif
		}
		void SoftBody::GetIndices(Core::Vector<int>* Result) const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "softbody should be initialized");
			VI_ASSERT(Result != nullptr, "result should be set");

			Core::UnorderedMap<btSoftBody::Node*, int> Nodes;
			for (int i = 0; i < Instance->m_nodes.size(); i++)
				Nodes.insert(std::make_pair(&Instance->m_nodes[i], i));

			for (int i = 0; i < Instance->m_faces.size(); i++)
			{
				btSoftBody::Face& Face = Instance->m_faces[i];
				for (uint32_t j = 0; j < 3; j++)
				{
					auto It = Nodes.find(Face.m_n[j]);
					if (It != Nodes.end())
						Result->push_back(It->second);
				}
			}
#endif
		}
		void SoftBody::GetVertices(Core::Vector<Trigonometry::Vertex>* Result) const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "softbody should be initialized");
			VI_ASSERT(Result != nullptr, "result should be set");

			static size_t PositionX = offsetof(Trigonometry::Vertex, PositionX);
			static size_t NormalX = offsetof(Trigonometry::Vertex, NormalX);

			size_t Size = (size_t)Instance->m_nodes.size();
			if (Result->size() != Size)
			{
				if (Initial.Shape.Convex.Enabled)
					*Result = Initial.Shape.Convex.Hull->GetVertices();
				else
					Result->resize(Size);
			}

			for (size_t i = 0; i < Size; i++)
			{
				auto* Node = &Instance->m_nodes[(int)i]; Trigonometry::Vertex& Position = Result->at(i);
				memcpy(&Position + PositionX, Node->m_x.m_floats, sizeof(float) * 3);
				memcpy(&Position + NormalX, Node->m_n.m_floats, sizeof(float) * 3);
			}
#endif
		}
		void SoftBody::GetBoundingBox(Trigonometry::Vector3* Min, Trigonometry::Vector3* Max) const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "softbody should be initialized");

			btVector3 bMin, bMax;
			Instance->getAabb(bMin, bMax);
			if (Min != nullptr)
				*Min = BT_TO_V3(bMin).InvZ();

			if (Max != nullptr)
				*Max = BT_TO_V3(bMax).InvZ();
#endif
		}
		void SoftBody::SetContactStiffnessAndDamping(float Stiffness, float Damping)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "softbody should be initialized");
			Instance->setContactStiffnessAndDamping(Stiffness, Damping);
#endif
		}
		void SoftBody::AddAnchor(int Node, RigidBody* Body, bool DisableCollisionBetweenLinkedBodies, float Influence)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "softbody should be initialized");
			VI_ASSERT(Body != nullptr, "body should be set");
			Instance->appendAnchor(Node, Body->Get(), DisableCollisionBetweenLinkedBodies, Influence);
#endif
		}
		void SoftBody::AddAnchor(int Node, RigidBody* Body, const Trigonometry::Vector3& LocalPivot, bool DisableCollisionBetweenLinkedBodies, float Influence)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "softbody should be initialized");
			VI_ASSERT(Body != nullptr, "body should be set");
			Instance->appendAnchor(Node, Body->Get(), V3_TO_BT(LocalPivot), DisableCollisionBetweenLinkedBodies, Influence);
#endif
		}
		void SoftBody::AddForce(const Trigonometry::Vector3& Force)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "softbody should be initialized");
			Instance->addForce(V3_TO_BT(Force));
#endif
		}
		void SoftBody::AddForce(const Trigonometry::Vector3& Force, int Node)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "softbody should be initialized");
			Instance->addForce(V3_TO_BT(Force), Node);
#endif
		}
		void SoftBody::AddAeroForceToNode(const Trigonometry::Vector3& WindVelocity, int NodeIndex)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "softbody should be initialized");
			Instance->addAeroForceToNode(V3_TO_BT(WindVelocity), NodeIndex);
#endif
		}
		void SoftBody::AddAeroForceToFace(const Trigonometry::Vector3& WindVelocity, int FaceIndex)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "softbody should be initialized");
			Instance->addAeroForceToFace(V3_TO_BT(WindVelocity), FaceIndex);
#endif
		}
		void SoftBody::AddVelocity(const Trigonometry::Vector3& Velocity)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "softbody should be initialized");
			Instance->addVelocity(V3_TO_BT(Velocity));
#endif
		}
		void SoftBody::SetVelocity(const Trigonometry::Vector3& Velocity)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "softbody should be initialized");
			Instance->setVelocity(V3_TO_BT(Velocity));
#endif
		}
		void SoftBody::AddVelocity(const Trigonometry::Vector3& Velocity, int Node)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "softbody should be initialized");
			Instance->addVelocity(V3_TO_BT(Velocity), Node);
#endif
		}
		void SoftBody::SetMass(int Node, float Mass)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "softbody should be initialized");
			Instance->setMass(Node, Mass);
#endif
		}
		void SoftBody::SetTotalMass(float Mass, bool FromFaces)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "softbody should be initialized");
			Instance->setTotalMass(Mass, FromFaces);
#endif
		}
		void SoftBody::SetTotalDensity(float Density)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "softbody should be initialized");
			Instance->setTotalDensity(Density);
#endif
		}
		void SoftBody::SetVolumeMass(float Mass)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "softbody should be initialized");
			Instance->setVolumeMass(Mass);
#endif
		}
		void SoftBody::SetVolumeDensity(float Density)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "softbody should be initialized");
			Instance->setVolumeDensity(Density);
#endif
		}
		void SoftBody::Translate(const Trigonometry::Vector3& Position)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "softbody should be initialized");
			Instance->translate(btVector3(Position.X, Position.Y, -Position.Z));
#endif
		}
		void SoftBody::Rotate(const Trigonometry::Vector3& Rotation)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "softbody should be initialized");
			btQuaternion Value;
			Value.setEulerZYX(Rotation.X, Rotation.Y, Rotation.Z);
			Instance->rotate(Value);
#endif
		}
		void SoftBody::Scale(const Trigonometry::Vector3& Scale)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "softbody should be initialized");
			Instance->scale(V3_TO_BT(Scale));
#endif
		}
		void SoftBody::SetRestLengthScale(float RestLength)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "softbody should be initialized");
			Instance->setRestLengthScale(RestLength);
#endif
		}
		void SoftBody::SetPose(bool Volume, bool Frame)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "softbody should be initialized");
			Instance->setPose(Volume, Frame);
#endif
		}
		float SoftBody::GetMass(int Node) const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "softbody should be initialized");
			return Instance->getMass(Node);
#else
			return 0;
#endif
		}
		float SoftBody::GetTotalMass() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "softbody should be initialized");
			return Instance->getTotalMass();
#else
			return 0;
#endif
		}
		float SoftBody::GetRestLengthScale() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "softbody should be initialized");
			return Instance->getRestLengthScale();
#else
			return 0;
#endif
		}
		float SoftBody::GetVolume() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "softbody should be initialized");
			return Instance->getVolume();
#else
			return 0;
#endif
		}
		int SoftBody::GenerateBendingConstraints(int Distance)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "softbody should be initialized");
			return Instance->generateBendingConstraints(Distance);
#else
			return 0;
#endif
		}
		void SoftBody::RandomizeConstraints()
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "softbody should be initialized");
			Instance->randomizeConstraints();
#endif
		}
		bool SoftBody::CutLink(int Node0, int Node1, float Position)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "softbody should be initialized");
			return Instance->cutLink(Node0, Node1, Position);
#else
			return false;
#endif
		}
		bool SoftBody::RayTest(const Trigonometry::Vector3& From, const Trigonometry::Vector3& To, RayCast& Result)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "softbody should be initialized");
			btSoftBody::sRayCast Cast;
			bool R = Instance->rayTest(V3_TO_BT(From), V3_TO_BT(To), Cast);
			Result.Body = Get(Cast.body);
			Result.Feature = (SoftFeature)Cast.feature;
			Result.Index = Cast.index;
			Result.Fraction = Cast.fraction;

			return R;
#else
			return false;
#endif
		}
		void SoftBody::SetWindVelocity(const Trigonometry::Vector3& Velocity)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "softbody should be initialized");
			Instance->setWindVelocity(V3_TO_BT(Velocity));
#endif
		}
		Trigonometry::Vector3 SoftBody::GetWindVelocity() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "softbody should be initialized");
			btVector3 Value = Instance->getWindVelocity();
			return BT_TO_V3(Value);
#else
			return 0;
#endif
		}
		void SoftBody::GetAabb(Trigonometry::Vector3& Min, Trigonometry::Vector3& Max) const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "softbody should be initialized");
			btVector3 BMin, BMax;
			Instance->getAabb(BMin, BMax);
			Min = BT_TO_V3(BMin);
			Max = BT_TO_V3(BMax);
#endif
		}
		void SoftBody::IndicesToPointers(const int* Map)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "softbody should be initialized");
			VI_ASSERT(Map != nullptr, "map should be set");
			Instance->indicesToPointers(Map);
#endif
		}
		void SoftBody::SetSpinningFriction(float Value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "softbody should be initialized");
			Instance->setSpinningFriction(Value);
#endif
		}
		Trigonometry::Vector3 SoftBody::GetLinearVelocity() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "softbody should be initialized");
			btVector3 Value = Instance->getInterpolationLinearVelocity();
			return Trigonometry::Vector3(Value.getX(), Value.getY(), Value.getZ());
#else
			return 0;
#endif
		}
		Trigonometry::Vector3 SoftBody::GetAngularVelocity() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "softbody should be initialized");
			btVector3 Value = Instance->getInterpolationAngularVelocity();
			return Trigonometry::Vector3(Value.getX(), Value.getY(), Value.getZ());
#else
			return 0;
#endif
		}
		Trigonometry::Vector3 SoftBody::GetCenterPosition() const
		{
#ifdef VI_BULLET3
			return Center;
#else
			return 0;
#endif
		}
		void SoftBody::SetActivity(bool Active)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "softbody should be initialized");
			if (GetActivationState() == MotionState::Disable_Deactivation)
				return;

			if (Active)
				Instance->forceActivationState((int)MotionState::Active);
			else
				Instance->forceActivationState((int)MotionState::Deactivation_Needed);
#endif
		}
		void SoftBody::SetAsGhost()
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "softbody should be initialized");
			Instance->setCollisionFlags(btCollisionObject::CF_NO_CONTACT_RESPONSE);
#endif
		}
		void SoftBody::SetAsNormal()
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "softbody should be initialized");
			Instance->setCollisionFlags(0);
#endif
		}
		void SoftBody::SetSelfPointer()
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "softbody should be initialized");
			Instance->setUserPointer(this);
#endif
		}
		void SoftBody::SetWorldTransform(btTransform* Value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "softbody should be initialized");
			VI_ASSERT(Value != nullptr, "transform should be set");
			Instance->setWorldTransform(*Value);
#endif
		}
		void SoftBody::SetActivationState(MotionState Value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "softbody should be initialized");
			Instance->forceActivationState((int)Value);
#endif
		}
		void SoftBody::SetFriction(float Value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "softbody should be initialized");
			Instance->setFriction(Value);
#endif
		}
		void SoftBody::SetRestitution(float Value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "softbody should be initialized");
			Instance->setRestitution(Value);
#endif
		}
		void SoftBody::SetContactStiffness(float Value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "softbody should be initialized");
			Instance->setContactStiffnessAndDamping(Value, Instance->getContactDamping());
#endif
		}
		void SoftBody::SetContactDamping(float Value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "softbody should be initialized");
			Instance->setContactStiffnessAndDamping(Instance->getContactStiffness(), Value);
#endif
		}
		void SoftBody::SetHitFraction(float Value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "softbody should be initialized");
			Instance->setHitFraction(Value);
#endif
		}
		void SoftBody::SetCcdMotionThreshold(float Value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "softbody should be initialized");
			Instance->setCcdMotionThreshold(Value);
#endif
		}
		void SoftBody::SetCcdSweptSphereRadius(float Value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "softbody should be initialized");
			Instance->setCcdSweptSphereRadius(Value);
#endif
		}
		void SoftBody::SetContactProcessingThreshold(float Value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "softbody should be initialized");
			Instance->setContactProcessingThreshold(Value);
#endif
		}
		void SoftBody::SetDeactivationTime(float Value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "softbody should be initialized");
			Instance->setDeactivationTime(Value);
#endif
		}
		void SoftBody::SetRollingFriction(float Value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "softbody should be initialized");
			Instance->setRollingFriction(Value);
#endif
		}
		void SoftBody::SetAnisotropicFriction(const Trigonometry::Vector3& Value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "softbody should be initialized");
			Instance->setAnisotropicFriction(V3_TO_BT(Value));
#endif
		}
		void SoftBody::SetConfig(const Desc::SConfig& Conf)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "softbody should be initialized");
			Initial.Config = Conf;
			Instance->m_cfg.aeromodel = (btSoftBody::eAeroModel::_)Initial.Config.AeroModel;
			Instance->m_cfg.kVCF = Initial.Config.VCF;
			Instance->m_cfg.kDP = Initial.Config.DP;
			Instance->m_cfg.kDG = Initial.Config.DG;
			Instance->m_cfg.kLF = Initial.Config.LF;
			Instance->m_cfg.kPR = Initial.Config.PR;
			Instance->m_cfg.kVC = Initial.Config.VC;
			Instance->m_cfg.kDF = Initial.Config.DF;
			Instance->m_cfg.kMT = Initial.Config.MT;
			Instance->m_cfg.kCHR = Initial.Config.CHR;
			Instance->m_cfg.kKHR = Initial.Config.KHR;
			Instance->m_cfg.kSHR = Initial.Config.SHR;
			Instance->m_cfg.kAHR = Initial.Config.AHR;
			Instance->m_cfg.kSRHR_CL = Initial.Config.SRHR_CL;
			Instance->m_cfg.kSKHR_CL = Initial.Config.SKHR_CL;
			Instance->m_cfg.kSSHR_CL = Initial.Config.SSHR_CL;
			Instance->m_cfg.kSR_SPLT_CL = Initial.Config.SR_SPLT_CL;
			Instance->m_cfg.kSK_SPLT_CL = Initial.Config.SK_SPLT_CL;
			Instance->m_cfg.kSS_SPLT_CL = Initial.Config.SS_SPLT_CL;
			Instance->m_cfg.maxvolume = Initial.Config.MaxVolume;
			Instance->m_cfg.timescale = Initial.Config.TimeScale;
			Instance->m_cfg.viterations = Initial.Config.VIterations;
			Instance->m_cfg.piterations = Initial.Config.PIterations;
			Instance->m_cfg.diterations = Initial.Config.DIterations;
			Instance->m_cfg.citerations = Initial.Config.CIterations;
			Instance->m_cfg.collisions = Initial.Config.Collisions;
			Instance->m_cfg.m_maxStress = Initial.Config.MaxStress;
			Instance->m_cfg.drag = Initial.Config.Drag;

			if (Initial.Config.Constraints > 0)
				Instance->generateBendingConstraints(Initial.Config.Constraints);

			if (Initial.Config.Clusters > 0)
				Instance->generateClusters(Initial.Config.Clusters);
#endif
		}
		MotionState SoftBody::GetActivationState() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "softbody should be initialized");
			return (MotionState)Instance->getActivationState();
#else
			return MotionState::Island_Sleeping;
#endif
		}
		Shape SoftBody::GetCollisionShapeType() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "softbody should be initialized");
			if (!Initial.Shape.Convex.Enabled)
				return Shape::Invalid;

			return Shape::Convex_Hull;
#else
			return Shape::Invalid;
#endif
		}
		Trigonometry::Vector3 SoftBody::GetAnisotropicFriction() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "softbody should be initialized");
			btVector3 Value = Instance->getAnisotropicFriction();
			return Trigonometry::Vector3(Value.getX(), Value.getY(), Value.getZ());
#else
			return 0;
#endif
		}
		Trigonometry::Vector3 SoftBody::GetScale() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "softbody should be initialized");
			btVector3 bMin, bMax;
			Instance->getAabb(bMin, bMax);
			btVector3 bScale = bMax - bMin;
			Trigonometry::Vector3 Scale = BT_TO_V3(bScale);

			return Scale.Div(2.0f).Abs();
#else
			return 0;
#endif
		}
		Trigonometry::Vector3 SoftBody::GetPosition() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "softbody should be initialized");
			btVector3 Value = Instance->getWorldTransform().getOrigin();
			return Trigonometry::Vector3(Value.getX(), Value.getY(), Value.getZ());
#else
			return 0;
#endif
		}
		Trigonometry::Vector3 SoftBody::GetRotation() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "softbody should be initialized");
			btScalar X, Y, Z;
			Instance->getWorldTransform().getBasis().getEulerZYX(Z, Y, X);
			return Trigonometry::Vector3(-X, -Y, Z);
#else
			return 0;
#endif
		}
		btTransform* SoftBody::GetWorldTransform() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "softbody should be initialized");
			return &Instance->getWorldTransform();
#else
			return nullptr;
#endif
		}
		btSoftBody* SoftBody::Get() const
		{
#ifdef VI_BULLET3
			return Instance;
#else
			return nullptr;
#endif
		}
		bool SoftBody::IsGhost() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "softbody should be initialized");
			return (Instance->getCollisionFlags() & btCollisionObject::CF_NO_CONTACT_RESPONSE) != 0;
#else
			return false;
#endif
		}
		bool SoftBody::IsActive() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "softbody should be initialized");
			return Instance->isActive();
#else
			return false;
#endif
		}
		bool SoftBody::IsStatic() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "softbody should be initialized");
			return Instance->isStaticObject();
#else
			return true;
#endif
		}
		bool SoftBody::IsColliding() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "softbody should be initialized");
			return Instance->hasContactResponse();
#else
			return false;
#endif
		}
		float SoftBody::GetSpinningFriction() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "softbody should be initialized");
			return Instance->getSpinningFriction();
#else
			return 0;
#endif
		}
		float SoftBody::GetContactStiffness() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "softbody should be initialized");
			return Instance->getContactStiffness();
#else
			return 0;
#endif
		}
		float SoftBody::GetContactDamping() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "softbody should be initialized");
			return Instance->getContactDamping();
#else
			return 0;
#endif
		}
		float SoftBody::GetFriction() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "softbody should be initialized");
			return Instance->getFriction();
#else
			return 0;
#endif
		}
		float SoftBody::GetRestitution() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "softbody should be initialized");
			return Instance->getRestitution();
#else
			return 0;
#endif
		}
		float SoftBody::GetHitFraction() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "softbody should be initialized");
			return Instance->getHitFraction();
#else
			return 0;
#endif
		}
		float SoftBody::GetCcdMotionThreshold() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "softbody should be initialized");
			return Instance->getCcdMotionThreshold();
#else
			return 0;
#endif
		}
		float SoftBody::GetCcdSweptSphereRadius() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "softbody should be initialized");
			return Instance->getCcdSweptSphereRadius();
#else
			return 0;
#endif
		}
		float SoftBody::GetContactProcessingThreshold() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "softbody should be initialized");
			return Instance->getContactProcessingThreshold();
#else
			return 0;
#endif
		}
		float SoftBody::GetDeactivationTime() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "softbody should be initialized");
			return Instance->getDeactivationTime();
#else
			return 0;
#endif
		}
		float SoftBody::GetRollingFriction() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "softbody should be initialized");
			return Instance->getRollingFriction();
#else
			return 0;
#endif
		}
		size_t SoftBody::GetCollisionFlags() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "softbody should be initialized");
			return Instance->getCollisionFlags();
#else
			return 0;
#endif
		}
		size_t SoftBody::GetVerticesCount() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "softbody should be initialized");
			return Instance->m_nodes.size();
#else
			return 0;
#endif
		}
		SoftBody::Desc& SoftBody::GetInitialState()
		{
			return Initial;
		}
		Simulator* SoftBody::GetSimulator() const
		{
#ifdef VI_BULLET3
			return Engine;
#else
			return nullptr;
#endif
		}
		SoftBody* SoftBody::Get(btSoftBody* From)
		{
#ifdef VI_BULLET3
			VI_ASSERT(From != nullptr, "softbody should be set");
			return (SoftBody*)From->getUserPointer();
#else
			return nullptr;
#endif
		}

		Constraint::Constraint(Simulator* Refer) noexcept : First(nullptr), Second(nullptr), Engine(Refer), UserPointer(nullptr)
		{
		}
		void Constraint::SetBreakingImpulseThreshold(float Value)
		{
#ifdef VI_BULLET3
			btTypedConstraint* Base = Get();
			VI_ASSERT(Base != nullptr, "typed constraint should be initialized");
			Base->setBreakingImpulseThreshold(Value);
#endif
		}
		void Constraint::SetEnabled(bool Value)
		{
#ifdef VI_BULLET3
			btTypedConstraint* Base = Get();
			VI_ASSERT(Base != nullptr, "typed constraint should be initialized");
			Base->setEnabled(Value);
#endif
		}
		btRigidBody* Constraint::GetFirst() const
		{
#ifdef VI_BULLET3
			return First;
#else
			return nullptr;
#endif
		}
		btRigidBody* Constraint::GetSecond() const
		{
#ifdef VI_BULLET3
			return Second;
#else
			return nullptr;
#endif
		}
		float Constraint::GetBreakingImpulseThreshold() const
		{
#ifdef VI_BULLET3
			btTypedConstraint* Base = Get();
			VI_ASSERT(Base != nullptr, "typed constraint should be initialized");
			return Base->getBreakingImpulseThreshold();
#else
			return 0;
#endif
		}
		bool Constraint::IsActive() const
		{
#ifdef VI_BULLET3
			btTypedConstraint* Base = Get();
			if (!Base || !First || !Second)
				return false;

			if (First != nullptr)
			{
				for (int i = 0; i < First->getNumConstraintRefs(); i++)
				{
					if (First->getConstraintRef(i) == Base)
						return true;
				}
			}

			if (Second != nullptr)
			{
				for (int i = 0; i < Second->getNumConstraintRefs(); i++)
				{
					if (Second->getConstraintRef(i) == Base)
						return true;
				}
			}

			return false;
#else
			return false;
#endif
		}
		bool Constraint::IsEnabled() const
		{
#ifdef VI_BULLET3
			btTypedConstraint* Base = Get();
			VI_ASSERT(Base != nullptr, "typed constraint should be initialized");
			return Base->isEnabled();
#else
			return false;
#endif
		}
		Simulator* Constraint::GetSimulator() const
		{
			return Engine;
		}

		PConstraint::PConstraint(Simulator* Refer, const Desc& I) noexcept : Constraint(Refer), Instance(nullptr), State(I)
		{
#ifdef VI_BULLET3
			VI_ASSERT(I.TargetA != nullptr, "target A rigidbody should be set");
			VI_ASSERT(Engine != nullptr, "simulator should be set");

			First = I.TargetA->Get();
			Second = (I.TargetB ? I.TargetB->Get() : nullptr);

			if (Second != nullptr)
				Instance = Core::Memory::New<btPoint2PointConstraint>(*First, *Second, V3_TO_BT(I.PivotA), V3_TO_BT(I.PivotB));
			else
				Instance = Core::Memory::New<btPoint2PointConstraint>(*First, V3_TO_BT(I.PivotA));

			Instance->setUserConstraintPtr(this);
			Engine->AddConstraint(this);
#endif
		}
		PConstraint::~PConstraint() noexcept
		{
#ifdef VI_BULLET3
			Engine->RemoveConstraint(this);
			Core::Memory::Delete(Instance);
#endif
		}
		Constraint* PConstraint::Copy() const
		{
			VI_ASSERT(Instance != nullptr, "p2p constraint should be initialized");
			PConstraint* Target = new PConstraint(Engine, State);
			Target->SetBreakingImpulseThreshold(GetBreakingImpulseThreshold());
			Target->SetEnabled(IsEnabled());
			Target->SetPivotA(GetPivotA());
			Target->SetPivotB(GetPivotB());

			return Target;
		}
		btTypedConstraint* PConstraint::Get() const
		{
#ifdef VI_BULLET3
			return Instance;
#else
			return nullptr;
#endif
		}
		bool PConstraint::HasCollisions() const
		{
			return State.Collisions;
		}
		void PConstraint::SetPivotA(const Trigonometry::Vector3& Value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "p2p constraint should be initialized");
			Instance->setPivotA(V3_TO_BT(Value));
			State.PivotA = Value;
#endif
		}
		void PConstraint::SetPivotB(const Trigonometry::Vector3& Value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "p2p constraint should be initialized");
			Instance->setPivotB(V3_TO_BT(Value));
			State.PivotB = Value;
#endif
		}
		Trigonometry::Vector3 PConstraint::GetPivotA() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "p2p constraint should be initialized");
			const btVector3& Value = Instance->getPivotInA();
			return BT_TO_V3(Value);
#else
			return 0;
#endif
		}
		Trigonometry::Vector3 PConstraint::GetPivotB() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "p2p constraint should be initialized");
			const btVector3& Value = Instance->getPivotInB();
			return BT_TO_V3(Value);
#else
			return 0;
#endif
		}
		PConstraint::Desc& PConstraint::GetState()
		{
			return State;
		}

		HConstraint::HConstraint(Simulator* Refer, const Desc& I) noexcept : Constraint(Refer), Instance(nullptr), State(I)
		{
#ifdef VI_BULLET3
			VI_ASSERT(I.TargetA != nullptr, "target A rigidbody should be set");
			VI_ASSERT(Engine != nullptr, "simulator should be set");

			First = I.TargetA->Get();
			Second = (I.TargetB ? I.TargetB->Get() : nullptr);

			if (Second != nullptr)
				Instance = Core::Memory::New<btHingeConstraint>(*First, *Second, btTransform::getIdentity(), btTransform::getIdentity(), I.References);
			else
				Instance = Core::Memory::New<btHingeConstraint>(*First, btTransform::getIdentity(), I.References);

			Instance->setUserConstraintPtr(this);
			Engine->AddConstraint(this);
#endif
		}
		HConstraint::~HConstraint() noexcept
		{
#ifdef VI_BULLET3
			Engine->RemoveConstraint(this);
			Core::Memory::Delete(Instance);
#endif
		}
		Constraint* HConstraint::Copy() const
		{
			VI_ASSERT(Instance != nullptr, "hinge constraint should be initialized");
			HConstraint* Target = new HConstraint(Engine, State);
			Target->SetBreakingImpulseThreshold(GetBreakingImpulseThreshold());
			Target->SetEnabled(IsEnabled());
			Target->EnableAngularMotor(IsAngularMotorEnabled(), GetMotorTargetVelocity(), GetMaxMotorImpulse());
			Target->SetAngularOnly(IsAngularOnly());
			Target->SetLimit(GetLowerLimit(), GetUpperLimit(), GetLimitSoftness(), GetLimitBiasFactor(), GetLimitRelaxationFactor());
			Target->SetOffset(IsOffset());

			return Target;
		}
		btTypedConstraint* HConstraint::Get() const
		{
#ifdef VI_BULLET3
			return Instance;
#else
			return nullptr;
#endif
		}
		bool HConstraint::HasCollisions() const
		{
			return State.Collisions;
		}
		void HConstraint::EnableAngularMotor(bool Enable, float TargetVelocity, float MaxMotorImpulse)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "hinge constraint should be initialized");
			Instance->enableAngularMotor(Enable, TargetVelocity, MaxMotorImpulse);
#endif
		}
		void HConstraint::EnableMotor(bool Enable)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "hinge constraint should be initialized");
			Instance->enableMotor(Enable);
#endif
		}
		void HConstraint::TestLimit(const Trigonometry::Matrix4x4& A, const Trigonometry::Matrix4x4& B)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "hinge constraint should be initialized");
			Instance->testLimit(M16_TO_BT(A), M16_TO_BT(B));
#endif
		}
		void HConstraint::SetFrames(const Trigonometry::Matrix4x4& A, const Trigonometry::Matrix4x4& B)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "hinge constraint should be initialized");
			Instance->setFrames(M16_TO_BT(A), M16_TO_BT(B));
#endif
		}
		void HConstraint::SetAngularOnly(bool Value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "hinge constraint should be initialized");
			Instance->setAngularOnly(Value);
#endif
		}
		void HConstraint::SetMaxMotorImpulse(float Value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "hinge constraint should be initialized");
			Instance->setMaxMotorImpulse(Value);
#endif
		}
		void HConstraint::SetMotorTargetVelocity(float Value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "hinge constraint should be initialized");
			Instance->setMotorTargetVelocity(Value);
#endif
		}
		void HConstraint::SetMotorTarget(float TargetAngle, float Delta)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "hinge constraint should be initialized");
			Instance->setMotorTarget(TargetAngle, Delta);
#endif
		}
		void HConstraint::SetLimit(float Low, float High, float Softness, float BiasFactor, float RelaxationFactor)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "hinge constraint should be initialized");
			Instance->setLimit(Low, High, Softness, BiasFactor, RelaxationFactor);
#endif
		}
		void HConstraint::SetOffset(bool Value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "hinge constraint should be initialized");
			Instance->setUseFrameOffset(Value);
#endif
		}
		void HConstraint::SetReferenceToA(bool Value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "hinge constraint should be initialized");
			Instance->setUseReferenceFrameA(Value);
#endif
		}
		void HConstraint::SetAxis(const Trigonometry::Vector3& Value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "hinge constraint should be initialized");
			btVector3 Axis = V3_TO_BT(Value);
			Instance->setAxis(Axis);
#endif
		}
		int HConstraint::GetSolveLimit() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "hinge constraint should be initialized");
			return Instance->getSolveLimit();
#else
			return 0;
#endif
		}
		float HConstraint::GetMotorTargetVelocity() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "hinge constraint should be initialized");
			return Instance->getMotorTargetVelocity();
#else
			return 0;
#endif
		}
		float HConstraint::GetMaxMotorImpulse() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "hinge constraint should be initialized");
			return Instance->getMaxMotorImpulse();
#else
			return 0;
#endif
		}
		float HConstraint::GetLimitSign() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "hinge constraint should be initialized");
			return Instance->getLimitSign();
#else
			return 0;
#endif
		}
		float HConstraint::GetHingeAngle() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "hinge constraint should be initialized");
			return Instance->getHingeAngle();
#else
			return 0;
#endif
		}
		float HConstraint::GetHingeAngle(const Trigonometry::Matrix4x4& A, const Trigonometry::Matrix4x4& B) const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "hinge constraint should be initialized");
			return Instance->getHingeAngle(M16_TO_BT(A), M16_TO_BT(B));
#else
			return 0;
#endif
		}
		float HConstraint::GetLowerLimit() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "hinge constraint should be initialized");
			return Instance->getLowerLimit();
#else
			return 0;
#endif
		}
		float HConstraint::GetUpperLimit() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "hinge constraint should be initialized");
			return Instance->getUpperLimit();
#else
			return 0;
#endif
		}
		float HConstraint::GetLimitSoftness() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "hinge constraint should be initialized");
			return Instance->getLimitSoftness();
#else
			return 0;
#endif
		}
		float HConstraint::GetLimitBiasFactor() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "hinge constraint should be initialized");
			return Instance->getLimitBiasFactor();
#else
			return 0;
#endif
		}
		float HConstraint::GetLimitRelaxationFactor() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "hinge constraint should be initialized");
			return Instance->getLimitRelaxationFactor();
#else
			return 0;
#endif
		}
		bool HConstraint::HasLimit() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "hinge constraint should be initialized");
			return Instance->hasLimit();
#else
			return 0;
#endif
		}
		bool HConstraint::IsOffset() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "hinge constraint should be initialized");
			return Instance->getUseFrameOffset();
#else
			return 0;
#endif
		}
		bool HConstraint::IsReferenceToA() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "hinge constraint should be initialized");
			return Instance->getUseReferenceFrameA();
#else
			return 0;
#endif
		}
		bool HConstraint::IsAngularOnly() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "hinge constraint should be initialized");
			return Instance->getAngularOnly();
#else
			return 0;
#endif
		}
		bool HConstraint::IsAngularMotorEnabled() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "hinge constraint should be initialized");
			return Instance->getEnableAngularMotor();
#else
			return 0;
#endif
		}
		HConstraint::Desc& HConstraint::GetState()
		{
			return State;
		}

		SConstraint::SConstraint(Simulator* Refer, const Desc& I) noexcept : Constraint(Refer), Instance(nullptr), State(I)
		{
#ifdef VI_BULLET3
			VI_ASSERT(I.TargetA != nullptr, "target A rigidbody should be set");
			VI_ASSERT(Engine != nullptr, "simulator should be set");

			First = I.TargetA->Get();
			Second = (I.TargetB ? I.TargetB->Get() : nullptr);

			if (Second != nullptr)
				Instance = Core::Memory::New<btSliderConstraint>(*First, *Second, btTransform::getIdentity(), btTransform::getIdentity(), I.Linear);
			else
				Instance = Core::Memory::New<btSliderConstraint>(*First, btTransform::getIdentity(), I.Linear);

			Instance->setUserConstraintPtr(this);
			Engine->AddConstraint(this);
#endif
		}
		SConstraint::~SConstraint() noexcept
		{
#ifdef VI_BULLET3
			Engine->RemoveConstraint(this);
			Core::Memory::Delete(Instance);
#endif
		}
		Constraint* SConstraint::Copy() const
		{
			VI_ASSERT(Instance != nullptr, "slider constraint should be initialized");
			SConstraint* Target = new SConstraint(Engine, State);
			Target->SetBreakingImpulseThreshold(GetBreakingImpulseThreshold());
			Target->SetEnabled(IsEnabled());
			Target->SetAngularMotorVelocity(GetAngularMotorVelocity());
			Target->SetLinearMotorVelocity(GetLinearMotorVelocity());
			Target->SetUpperLinearLimit(GetUpperLinearLimit());
			Target->SetLowerLinearLimit(GetLowerLinearLimit());
			Target->SetAngularDampingDirection(GetAngularDampingDirection());
			Target->SetLinearDampingDirection(GetLinearDampingDirection());
			Target->SetAngularDampingLimit(GetAngularDampingLimit());
			Target->SetLinearDampingLimit(GetLinearDampingLimit());
			Target->SetAngularDampingOrtho(GetAngularDampingOrtho());
			Target->SetLinearDampingOrtho(GetLinearDampingOrtho());
			Target->SetUpperAngularLimit(GetUpperAngularLimit());
			Target->SetLowerAngularLimit(GetLowerAngularLimit());
			Target->SetMaxAngularMotorForce(GetMaxAngularMotorForce());
			Target->SetMaxLinearMotorForce(GetMaxLinearMotorForce());
			Target->SetAngularRestitutionDirection(GetAngularRestitutionDirection());
			Target->SetLinearRestitutionDirection(GetLinearRestitutionDirection());
			Target->SetAngularRestitutionLimit(GetAngularRestitutionLimit());
			Target->SetLinearRestitutionLimit(GetLinearRestitutionLimit());
			Target->SetAngularRestitutionOrtho(GetAngularRestitutionOrtho());
			Target->SetLinearRestitutionOrtho(GetLinearRestitutionOrtho());
			Target->SetAngularSoftnessDirection(GetAngularSoftnessDirection());
			Target->SetLinearSoftnessDirection(GetLinearSoftnessDirection());
			Target->SetAngularSoftnessLimit(GetAngularSoftnessLimit());
			Target->SetLinearSoftnessLimit(GetLinearSoftnessLimit());
			Target->SetAngularSoftnessOrtho(GetAngularSoftnessOrtho());
			Target->SetLinearSoftnessOrtho(GetLinearSoftnessOrtho());
			Target->SetPoweredAngularMotor(GetPoweredAngularMotor());
			Target->SetPoweredLinearMotor(GetPoweredLinearMotor());

			return Target;
		}
		btTypedConstraint* SConstraint::Get() const
		{
#ifdef VI_BULLET3
			return Instance;
#else
			return nullptr;
#endif
		}
		bool SConstraint::HasCollisions() const
		{
			return State.Collisions;
		}
		void SConstraint::SetAngularMotorVelocity(float Value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "slider constraint should be initialized");
			Instance->setTargetAngMotorVelocity(Value);
#endif
		}
		void SConstraint::SetLinearMotorVelocity(float Value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "slider constraint should be initialized");
			Instance->setTargetLinMotorVelocity(Value);
#endif
		}
		void SConstraint::SetUpperLinearLimit(float Value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "slider constraint should be initialized");
			Instance->setUpperLinLimit(Value);
#endif
		}
		void SConstraint::SetLowerLinearLimit(float Value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "slider constraint should be initialized");
			Instance->setLowerLinLimit(Value);
#endif
		}
		void SConstraint::SetAngularDampingDirection(float Value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "slider constraint should be initialized");
			Instance->setDampingDirAng(Value);
#endif
		}
		void SConstraint::SetLinearDampingDirection(float Value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "slider constraint should be initialized");
			Instance->setDampingDirLin(Value);
#endif
		}
		void SConstraint::SetAngularDampingLimit(float Value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "slider constraint should be initialized");
			Instance->setDampingLimAng(Value);
#endif
		}
		void SConstraint::SetLinearDampingLimit(float Value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "slider constraint should be initialized");
			Instance->setDampingLimLin(Value);
#endif
		}
		void SConstraint::SetAngularDampingOrtho(float Value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "slider constraint should be initialized");
			Instance->setDampingOrthoAng(Value);
#endif
		}
		void SConstraint::SetLinearDampingOrtho(float Value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "slider constraint should be initialized");
			Instance->setDampingOrthoLin(Value);
#endif
		}
		void SConstraint::SetUpperAngularLimit(float Value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "slider constraint should be initialized");
			Instance->setUpperAngLimit(Value);
#endif
		}
		void SConstraint::SetLowerAngularLimit(float Value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "slider constraint should be initialized");
			Instance->setLowerAngLimit(Value);
#endif
		}
		void SConstraint::SetMaxAngularMotorForce(float Value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "slider constraint should be initialized");
			Instance->setMaxAngMotorForce(Value);
#endif
		}
		void SConstraint::SetMaxLinearMotorForce(float Value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "slider constraint should be initialized");
			Instance->setMaxLinMotorForce(Value);
#endif
		}
		void SConstraint::SetAngularRestitutionDirection(float Value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "slider constraint should be initialized");
			Instance->setRestitutionDirAng(Value);
#endif
		}
		void SConstraint::SetLinearRestitutionDirection(float Value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "slider constraint should be initialized");
			Instance->setRestitutionDirLin(Value);
#endif
		}
		void SConstraint::SetAngularRestitutionLimit(float Value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "slider constraint should be initialized");
			Instance->setRestitutionLimAng(Value);
#endif
		}
		void SConstraint::SetLinearRestitutionLimit(float Value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "slider constraint should be initialized");
			Instance->setRestitutionLimLin(Value);
#endif
		}
		void SConstraint::SetAngularRestitutionOrtho(float Value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "slider constraint should be initialized");
			Instance->setRestitutionOrthoAng(Value);
#endif
		}
		void SConstraint::SetLinearRestitutionOrtho(float Value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "slider constraint should be initialized");
			Instance->setRestitutionOrthoLin(Value);
#endif
		}
		void SConstraint::SetAngularSoftnessDirection(float Value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "slider constraint should be initialized");
			Instance->setSoftnessDirAng(Value);
#endif
		}
		void SConstraint::SetLinearSoftnessDirection(float Value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "slider constraint should be initialized");
			Instance->setSoftnessDirLin(Value);
#endif
		}
		void SConstraint::SetAngularSoftnessLimit(float Value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "slider constraint should be initialized");
			Instance->setSoftnessLimAng(Value);
#endif
		}
		void SConstraint::SetLinearSoftnessLimit(float Value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "slider constraint should be initialized");
			Instance->setSoftnessLimLin(Value);
#endif
		}
		void SConstraint::SetAngularSoftnessOrtho(float Value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "slider constraint should be initialized");
			Instance->setSoftnessOrthoAng(Value);
#endif
		}
		void SConstraint::SetLinearSoftnessOrtho(float Value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "slider constraint should be initialized");
			Instance->setSoftnessOrthoLin(Value);
#endif
		}
		void SConstraint::SetPoweredAngularMotor(bool Value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "slider constraint should be initialized");
			Instance->setPoweredAngMotor(Value);
#endif
		}
		void SConstraint::SetPoweredLinearMotor(bool Value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "slider constraint should be initialized");
			Instance->setPoweredLinMotor(Value);
#endif
		}
		float SConstraint::GetAngularMotorVelocity() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "slider constraint should be initialized");
			return Instance->getTargetAngMotorVelocity();
#else
			return 0;
#endif
		}
		float SConstraint::GetLinearMotorVelocity() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "slider constraint should be initialized");
			return Instance->getTargetLinMotorVelocity();
#else
			return 0;
#endif
		}
		float SConstraint::GetUpperLinearLimit() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "slider constraint should be initialized");
			return Instance->getUpperLinLimit();
#else
			return 0;
#endif
		}
		float SConstraint::GetLowerLinearLimit() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "slider constraint should be initialized");
			return Instance->getLowerLinLimit();
#else
			return 0;
#endif
		}
		float SConstraint::GetAngularDampingDirection() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "slider constraint should be initialized");
			return Instance->getDampingDirAng();
#else
			return 0;
#endif
		}
		float SConstraint::GetLinearDampingDirection() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "slider constraint should be initialized");
			return Instance->getDampingDirLin();
#else
			return 0;
#endif
		}
		float SConstraint::GetAngularDampingLimit() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "slider constraint should be initialized");
			return Instance->getDampingLimAng();
#else
			return 0;
#endif
		}
		float SConstraint::GetLinearDampingLimit() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "slider constraint should be initialized");
			return Instance->getDampingLimLin();
#else
			return 0;
#endif
		}
		float SConstraint::GetAngularDampingOrtho() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "slider constraint should be initialized");
			return Instance->getDampingOrthoAng();
#else
			return 0;
#endif
		}
		float SConstraint::GetLinearDampingOrtho() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "slider constraint should be initialized");
			return Instance->getDampingOrthoLin();
#else
			return 0;
#endif
		}
		float SConstraint::GetUpperAngularLimit() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "slider constraint should be initialized");
			return Instance->getUpperAngLimit();
#else
			return 0;
#endif
		}
		float SConstraint::GetLowerAngularLimit() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "slider constraint should be initialized");
			return Instance->getLowerAngLimit();
#else
			return 0;
#endif
		}
		float SConstraint::GetMaxAngularMotorForce() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "slider constraint should be initialized");
			return Instance->getMaxAngMotorForce();
#else
			return 0;
#endif
		}
		float SConstraint::GetMaxLinearMotorForce() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "slider constraint should be initialized");
			return Instance->getMaxLinMotorForce();
#else
			return 0;
#endif
		}
		float SConstraint::GetAngularRestitutionDirection() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "slider constraint should be initialized");
			return Instance->getRestitutionDirAng();
#else
			return 0;
#endif
		}
		float SConstraint::GetLinearRestitutionDirection() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "slider constraint should be initialized");
			return Instance->getRestitutionDirLin();
#else
			return 0;
#endif
		}
		float SConstraint::GetAngularRestitutionLimit() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "slider constraint should be initialized");
			return Instance->getRestitutionLimAng();
#else
			return 0;
#endif
		}
		float SConstraint::GetLinearRestitutionLimit() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "slider constraint should be initialized");
			return Instance->getRestitutionLimLin();
#else
			return 0;
#endif
		}
		float SConstraint::GetAngularRestitutionOrtho() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "slider constraint should be initialized");
			return Instance->getRestitutionOrthoAng();
#else
			return 0;
#endif
		}
		float SConstraint::GetLinearRestitutionOrtho() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "slider constraint should be initialized");
			return Instance->getRestitutionOrthoLin();
#else
			return 0;
#endif
		}
		float SConstraint::GetAngularSoftnessDirection() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "slider constraint should be initialized");
			return Instance->getSoftnessDirAng();
#else
			return 0;
#endif
		}
		float SConstraint::GetLinearSoftnessDirection() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "slider constraint should be initialized");
			return Instance->getSoftnessDirLin();
#else
			return 0;
#endif
		}
		float SConstraint::GetAngularSoftnessLimit() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "slider constraint should be initialized");
			return Instance->getSoftnessLimAng();
#else
			return 0;
#endif
		}
		float SConstraint::GetLinearSoftnessLimit() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "slider constraint should be initialized");
			return Instance->getSoftnessLimLin();
#else
			return 0;
#endif
		}
		float SConstraint::GetAngularSoftnessOrtho() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "slider constraint should be initialized");
			return Instance->getSoftnessOrthoAng();
#else
			return 0;
#endif
		}
		float SConstraint::GetLinearSoftnessOrtho() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "slider constraint should be initialized");
			return Instance->getSoftnessOrthoLin();
#else
			return 0;
#endif
		}
		bool SConstraint::GetPoweredAngularMotor() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "slider constraint should be initialized");
			return Instance->getPoweredAngMotor();
#else
			return false;
#endif
		}
		bool SConstraint::GetPoweredLinearMotor() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "slider constraint should be initialized");
			return Instance->getPoweredLinMotor();
#else
			return false;
#endif
		}
		SConstraint::Desc& SConstraint::GetState()
		{
			return State;
		}

		CTConstraint::CTConstraint(Simulator* Refer, const Desc& I) noexcept : Constraint(Refer), Instance(nullptr), State(I)
		{
#ifdef VI_BULLET3
			VI_ASSERT(I.TargetA != nullptr, "target A rigidbody should be set");
			VI_ASSERT(Engine != nullptr, "simulator should be set");

			First = I.TargetA->Get();
			Second = (I.TargetB ? I.TargetB->Get() : nullptr);

			if (Second != nullptr)
				Instance = Core::Memory::New<btConeTwistConstraint>(*First, *Second, btTransform::getIdentity(), btTransform::getIdentity());
			else
				Instance = Core::Memory::New<btConeTwistConstraint>(*First, btTransform::getIdentity());

			Instance->setUserConstraintPtr(this);
			Engine->AddConstraint(this);
#endif
		}
		CTConstraint::~CTConstraint() noexcept
		{
#ifdef VI_BULLET3
			Engine->RemoveConstraint(this);
			Core::Memory::Delete(Instance);
#endif
		}
		Constraint* CTConstraint::Copy() const
		{
			VI_ASSERT(Instance != nullptr, "cone-twist constraint should be initialized");
			CTConstraint* Target = new CTConstraint(Engine, State);
			Target->SetBreakingImpulseThreshold(GetBreakingImpulseThreshold());
			Target->SetEnabled(IsEnabled());
			Target->EnableMotor(IsMotorEnabled());
			Target->SetAngularOnly(IsAngularOnly());
			Target->SetLimit(GetSwingSpan1(), GetSwingSpan2(), GetTwistSpan(), GetLimitSoftness(), GetBiasFactor(), GetRelaxationFactor());
			Target->SetDamping(GetDamping());
			Target->SetFixThresh(GetFixThresh());
			Target->SetMotorTarget(GetMotorTarget());

			if (IsMaxMotorImpulseNormalized())
				Target->SetMaxMotorImpulseNormalized(GetMaxMotorImpulse());
			else
				Target->SetMaxMotorImpulse(GetMaxMotorImpulse());

			return Target;
		}
		btTypedConstraint* CTConstraint::Get() const
		{
#ifdef VI_BULLET3
			return Instance;
#else
			return nullptr;
#endif
		}
		bool CTConstraint::HasCollisions() const
		{
			return State.Collisions;
		}
		void CTConstraint::EnableMotor(bool Value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "cone-twist constraint should be initialized");
			Instance->enableMotor(Value);
#endif
		}
		void CTConstraint::SetFrames(const Trigonometry::Matrix4x4& A, const Trigonometry::Matrix4x4& B)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "cone-twist constraint should be initialized");
			Instance->setFrames(M16_TO_BT(A), M16_TO_BT(B));
#endif
		}
		void CTConstraint::SetAngularOnly(bool Value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "cone-twist constraint should be initialized");
			Instance->setAngularOnly(Value);
#endif
		}
		void CTConstraint::SetLimit(int LimitIndex, float LimitValue)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "cone-twist constraint should be initialized");
			Instance->setLimit(LimitIndex, LimitValue);
#endif
		}
		void CTConstraint::SetLimit(float SwingSpan1, float SwingSpan2, float TwistSpan, float Softness, float BiasFactor, float RelaxationFactor)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "cone-twist constraint should be initialized");
			Instance->setLimit(SwingSpan1, SwingSpan2, TwistSpan, Softness, BiasFactor, RelaxationFactor);
#endif
		}
		void CTConstraint::SetDamping(float Value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "cone-twist constraint should be initialized");
			Instance->setDamping(Value);
#endif
		}
		void CTConstraint::SetMaxMotorImpulse(float Value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "cone-twist constraint should be initialized");
			Instance->setMaxMotorImpulse(Value);
#endif
		}
		void CTConstraint::SetMaxMotorImpulseNormalized(float Value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "cone-twist constraint should be initialized");
			Instance->setMaxMotorImpulseNormalized(Value);
#endif
		}
		void CTConstraint::SetFixThresh(float Value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "cone-twist constraint should be initialized");
			Instance->setFixThresh(Value);
#endif
		}
		void CTConstraint::SetMotorTarget(const Trigonometry::Quaternion& Value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "cone-twist constraint should be initialized");
			Instance->setMotorTarget(Q4_TO_BT(Value));
#endif
		}
		void CTConstraint::SetMotorTargetInConstraintSpace(const Trigonometry::Quaternion& Value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "cone-twist constraint should be initialized");
			Instance->setMotorTargetInConstraintSpace(Q4_TO_BT(Value));
#endif
		}
		Trigonometry::Vector3 CTConstraint::GetPointForAngle(float AngleInRadians, float Length) const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "cone-twist constraint should be initialized");
			btVector3 Value = Instance->GetPointForAngle(AngleInRadians, Length);
			return BT_TO_V3(Value);
#else
			return 0;
#endif
		}
		Trigonometry::Quaternion CTConstraint::GetMotorTarget() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "hinge constraint should be initialized");
			btQuaternion Value = Instance->getMotorTarget();
			return BT_TO_Q4(Value);
#else
			return Trigonometry::Quaternion();
#endif
		}
		int CTConstraint::GetSolveTwistLimit() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "hinge constraint should be initialized");
			return Instance->getSolveTwistLimit();
#else
			return 0;
#endif
		}
		int CTConstraint::GetSolveSwingLimit() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "hinge constraint should be initialized");
			return Instance->getSolveSwingLimit();
#else
			return 0;
#endif
		}
		float CTConstraint::GetTwistLimitSign() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "hinge constraint should be initialized");
			return Instance->getTwistLimitSign();
#else
			return 0;
#endif
		}
		float CTConstraint::GetSwingSpan1() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "hinge constraint should be initialized");
			return Instance->getSwingSpan1();
#else
			return 0;
#endif
		}
		float CTConstraint::GetSwingSpan2() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "hinge constraint should be initialized");
			return Instance->getSwingSpan2();
#else
			return 0;
#endif
		}
		float CTConstraint::GetTwistSpan() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "hinge constraint should be initialized");
			return Instance->getTwistSpan();
#else
			return 0;
#endif
		}
		float CTConstraint::GetLimitSoftness() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "hinge constraint should be initialized");
			return Instance->getLimitSoftness();
#else
			return 0;
#endif
		}
		float CTConstraint::GetBiasFactor() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "hinge constraint should be initialized");
			return Instance->getBiasFactor();
#else
			return 0;
#endif
		}
		float CTConstraint::GetRelaxationFactor() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "hinge constraint should be initialized");
			return Instance->getRelaxationFactor();
#else
			return 0;
#endif
		}
		float CTConstraint::GetTwistAngle() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "hinge constraint should be initialized");
			return Instance->getTwistAngle();
#else
			return 0;
#endif
		}
		float CTConstraint::GetLimit(int Value) const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "hinge constraint should be initialized");
			return Instance->getLimit(Value);
#else
			return 0;
#endif
		}
		float CTConstraint::GetDamping() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "hinge constraint should be initialized");
			return Instance->getDamping();
#else
			return 0;
#endif
		}
		float CTConstraint::GetMaxMotorImpulse() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "hinge constraint should be initialized");
			return Instance->getMaxMotorImpulse();
#else
			return 0;
#endif
		}
		float CTConstraint::GetFixThresh() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "hinge constraint should be initialized");
			return Instance->getFixThresh();
#else
			return 0;
#endif
		}
		bool CTConstraint::IsMotorEnabled() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "hinge constraint should be initialized");
			return Instance->isMotorEnabled();
#else
			return 0;
#endif
		}
		bool CTConstraint::IsMaxMotorImpulseNormalized() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "hinge constraint should be initialized");
			return Instance->isMaxMotorImpulseNormalized();
#else
			return 0;
#endif
		}
		bool CTConstraint::IsPastSwingLimit() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "hinge constraint should be initialized");
			return Instance->isPastSwingLimit();
#else
			return 0;
#endif
		}
		bool CTConstraint::IsAngularOnly() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "hinge constraint should be initialized");
			return Instance->getAngularOnly();
#else
			return 0;
#endif
		}
		CTConstraint::Desc& CTConstraint::GetState()
		{
			return State;
		}

		DF6Constraint::DF6Constraint(Simulator* Refer, const Desc& I) noexcept : Constraint(Refer), Instance(nullptr), State(I)
		{
#ifdef VI_BULLET3
			VI_ASSERT(I.TargetA != nullptr, "target A rigidbody should be set");
			VI_ASSERT(Engine != nullptr, "simulator should be set");

			First = I.TargetA->Get();
			Second = (I.TargetB ? I.TargetB->Get() : nullptr);

			if (Second != nullptr)
				Instance = Core::Memory::New<btGeneric6DofSpring2Constraint>(*First, *Second, btTransform::getIdentity(), btTransform::getIdentity());
			else
				Instance = Core::Memory::New<btGeneric6DofSpring2Constraint>(*First, btTransform::getIdentity());

			Instance->setUserConstraintPtr(this);
			Engine->AddConstraint(this);
#endif
		}
		DF6Constraint::~DF6Constraint() noexcept
		{
#ifdef VI_BULLET3
			Engine->RemoveConstraint(this);
			Core::Memory::Delete(Instance);
#endif
		}
		Constraint* DF6Constraint::Copy() const
		{
			VI_ASSERT(Instance != nullptr, "6-dof constraint should be initialized");
			DF6Constraint* Target = new DF6Constraint(Engine, State);
			Target->SetBreakingImpulseThreshold(GetBreakingImpulseThreshold());
			Target->SetEnabled(IsEnabled());
			Target->SetLinearLowerLimit(GetLinearLowerLimit());
			Target->SetLinearUpperLimit(GetLinearUpperLimit());
			Target->SetAngularLowerLimit(GetAngularLowerLimit());
			Target->SetAngularUpperLimit(GetAngularUpperLimit());
			Target->SetRotationOrder(GetRotationOrder());
			Target->SetAxis(GetAxis(0), GetAxis(1));

			return Target;
		}
		btTypedConstraint* DF6Constraint::Get() const
		{
#ifdef VI_BULLET3
			return Instance;
#else
			return nullptr;
#endif
		}
		bool DF6Constraint::HasCollisions() const
		{
			return State.Collisions;
		}
		void DF6Constraint::EnableMotor(int Index, bool OnOff)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "6-dof constraint should be initialized");
			Instance->enableMotor(Index, OnOff);
#endif
		}
		void DF6Constraint::EnableSpring(int Index, bool OnOff)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "6-dof constraint should be initialized");
			Instance->enableSpring(Index, OnOff);
#endif
		}
		void DF6Constraint::SetFrames(const Trigonometry::Matrix4x4& A, const Trigonometry::Matrix4x4& B)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "6-dof constraint should be initialized");
			Instance->setFrames(M16_TO_BT(A), M16_TO_BT(B));
#endif
		}
		void DF6Constraint::SetLinearLowerLimit(const Trigonometry::Vector3& Value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "6-dof constraint should be initialized");
			Instance->setLinearLowerLimit(V3_TO_BT(Value));
#endif
		}
		void DF6Constraint::SetLinearUpperLimit(const Trigonometry::Vector3& Value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "6-dof constraint should be initialized");
			Instance->setLinearUpperLimit(V3_TO_BT(Value));
#endif
		}
		void DF6Constraint::SetAngularLowerLimit(const Trigonometry::Vector3& Value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "6-dof constraint should be initialized");
			Instance->setAngularLowerLimit(V3_TO_BT(Value));
#endif
		}
		void DF6Constraint::SetAngularLowerLimitReversed(const Trigonometry::Vector3& Value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "6-dof constraint should be initialized");
			Instance->setAngularLowerLimitReversed(V3_TO_BT(Value));
#endif
		}
		void DF6Constraint::SetAngularUpperLimit(const Trigonometry::Vector3& Value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "6-dof constraint should be initialized");
			Instance->setAngularUpperLimit(V3_TO_BT(Value));
#endif
		}
		void DF6Constraint::SetAngularUpperLimitReversed(const Trigonometry::Vector3& Value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "6-dof constraint should be initialized");
			Instance->setAngularUpperLimitReversed(V3_TO_BT(Value));
#endif
		}
		void DF6Constraint::SetLimit(int Axis, float Low, float High)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "6-dof constraint should be initialized");
			Instance->setLimit(Axis, Low, High);
#endif
		}
		void DF6Constraint::SetLimitReversed(int Axis, float Low, float High)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "6-dof constraint should be initialized");
			Instance->setLimitReversed(Axis, Low, High);
#endif
		}
		void DF6Constraint::SetRotationOrder(Trigonometry::Rotator Order)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "6-dof constraint should be initialized");
			Instance->setRotationOrder((RotateOrder)Order);
#endif
		}
		void DF6Constraint::SetAxis(const Trigonometry::Vector3& A, const Trigonometry::Vector3& B)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "6-dof constraint should be initialized");
			Instance->setAxis(V3_TO_BT(A), V3_TO_BT(B));
#endif
		}
		void DF6Constraint::SetBounce(int Index, float Bounce)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "6-dof constraint should be initialized");
			Instance->setBounce(Index, Bounce);
#endif
		}
		void DF6Constraint::SetServo(int Index, bool OnOff)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "6-dof constraint should be initialized");
			Instance->setServo(Index, OnOff);
#endif
		}
		void DF6Constraint::SetTargetVelocity(int Index, float Velocity)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "6-dof constraint should be initialized");
			Instance->setTargetVelocity(Index, Velocity);
#endif
		}
		void DF6Constraint::SetServoTarget(int Index, float Target)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "6-dof constraint should be initialized");
			Instance->setServoTarget(Index, Target);
#endif
		}
		void DF6Constraint::SetMaxMotorForce(int Index, float Force)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "6-dof constraint should be initialized");
			Instance->setMaxMotorForce(Index, Force);
#endif
		}
		void DF6Constraint::SetStiffness(int Index, float Stiffness, bool LimitIfNeeded)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "6-dof constraint should be initialized");
			Instance->setStiffness(Index, Stiffness, LimitIfNeeded);
#endif
		}
		void DF6Constraint::SetEquilibriumPoint()
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "6-dof constraint should be initialized");
			Instance->setEquilibriumPoint();
#endif
		}
		void DF6Constraint::SetEquilibriumPoint(int Index)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "6-dof constraint should be initialized");
			Instance->setEquilibriumPoint(Index);
#endif
		}
		void DF6Constraint::SetEquilibriumPoint(int Index, float Value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "6-dof constraint should be initialized");
			Instance->setEquilibriumPoint(Index, Value);
#endif
		}
		Trigonometry::Vector3 DF6Constraint::GetAngularUpperLimit() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "6-dof constraint should be initialized");
			btVector3 Result;
			Instance->getAngularUpperLimit(Result);
			return BT_TO_V3(Result);
#else
			return 0;
#endif
		}
		Trigonometry::Vector3 DF6Constraint::GetAngularUpperLimitReversed() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "6-dof constraint should be initialized");
			btVector3 Result;
			Instance->getAngularUpperLimitReversed(Result);
			return BT_TO_V3(Result);
#else
			return 0;
#endif
		}
		Trigonometry::Vector3 DF6Constraint::GetAngularLowerLimit() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "6-dof constraint should be initialized");
			btVector3 Result;
			Instance->getAngularLowerLimit(Result);
			return BT_TO_V3(Result);
#else
			return 0;
#endif
		}
		Trigonometry::Vector3 DF6Constraint::GetAngularLowerLimitReversed() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "6-dof constraint should be initialized");
			btVector3 Result;
			Instance->getAngularLowerLimitReversed(Result);
			return BT_TO_V3(Result);
#else
			return 0;
#endif
		}
		Trigonometry::Vector3 DF6Constraint::GetLinearUpperLimit() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "6-dof constraint should be initialized");
			btVector3 Result;
			Instance->getLinearUpperLimit(Result);
			return BT_TO_V3(Result);
#else
			return 0;
#endif
		}
		Trigonometry::Vector3 DF6Constraint::GetLinearLowerLimit() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "6-dof constraint should be initialized");
			btVector3 Result;
			Instance->getLinearLowerLimit(Result);
			return BT_TO_V3(Result);
#else
			return 0;
#endif
		}
		Trigonometry::Vector3 DF6Constraint::GetAxis(int Value) const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "6-dof constraint should be initialized");
			btVector3 Result = Instance->getAxis(Value);
			return BT_TO_V3(Result);
#else
			return 0;
#endif
		}
		Trigonometry::Rotator DF6Constraint::GetRotationOrder() const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "6-dof constraint should be initialized");
			return (Trigonometry::Rotator)Instance->getRotationOrder();
#else
			return Trigonometry::Rotator::XYZ;
#endif
		}
		float DF6Constraint::GetAngle(int Value) const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "6-dof constraint should be initialized");
			return Instance->getAngle(Value);
#else
			return 0;
#endif
		}
		float DF6Constraint::GetRelativePivotPosition(int Value) const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "6-dof constraint should be initialized");
			return Instance->getRelativePivotPosition(Value);
#else
			return 0;
#endif
		}
		bool DF6Constraint::IsLimited(int LimitIndex) const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Instance != nullptr, "6-dof constraint should be initialized");
			return Instance->isLimited(LimitIndex);
#else
			return 0;
#endif
		}
		DF6Constraint::Desc& DF6Constraint::GetState()
		{
			return State;
		}

		Simulator::Simulator(const Desc& I) noexcept : SoftSolver(nullptr), Speedup(1.0f), Active(true)
		{
#ifdef VI_BULLET3
			Broadphase = Core::Memory::New<btDbvtBroadphase>();
			Solver = Core::Memory::New<btSequentialImpulseConstraintSolver>();

			if (I.EnableSoftBody)
			{
				SoftSolver = Core::Memory::New<btDefaultSoftBodySolver>();
				Collision = Core::Memory::New<btSoftBodyRigidBodyCollisionConfiguration>();
				Dispatcher = Core::Memory::New<btCollisionDispatcher>(Collision);
				World = Core::Memory::New<btSoftRigidDynamicsWorld>(Dispatcher, Broadphase, Solver, Collision, SoftSolver);
				btSoftRigidDynamicsWorld* SoftWorld = (btSoftRigidDynamicsWorld*)World;
				SoftWorld->getDispatchInfo().m_enableSPU = true;

				btSoftBodyWorldInfo& Info = SoftWorld->getWorldInfo();
				Info.m_gravity = V3_TO_BT(I.Gravity);
				Info.water_normal = V3_TO_BT(I.WaterNormal);
				Info.water_density = I.WaterDensity;
				Info.water_offset = I.WaterOffset;
				Info.air_density = I.AirDensity;
				Info.m_maxDisplacement = I.MaxDisplacement;
			}
			else
			{
				Collision = Core::Memory::New<btDefaultCollisionConfiguration>();
				Dispatcher = Core::Memory::New<btCollisionDispatcher>(Collision);
				World = Core::Memory::New<btDiscreteDynamicsWorld>(Dispatcher, Broadphase, Solver, Collision);
			}

			World->setWorldUserInfo(this);
			World->setGravity(V3_TO_BT(I.Gravity));
			gContactAddedCallback = nullptr;
			gContactDestroyedCallback = nullptr;
			gContactProcessedCallback = nullptr;
			gContactStartedCallback = [](btPersistentManifold* const& Manifold) -> void
			{
				btCollisionObject* Body1 = (btCollisionObject*)Manifold->getBody0();
				btRigidBody* Rigid1 = btRigidBody::upcast(Body1);
				btSoftBody* Soft1 = btSoftBody::upcast(Body1);

				if (Rigid1 != nullptr)
				{
					RigidBody* Body = (RigidBody*)Rigid1->getUserPointer();
					if (Body != nullptr && Body->OnCollisionEnter)
						Body->OnCollisionEnter(CollisionBody((btCollisionObject*)Manifold->getBody1()));
				}
				else if (Soft1 != nullptr)
				{
					SoftBody* Body = (SoftBody*)Soft1->getUserPointer();
					if (Body != nullptr && Body->OnCollisionEnter)
						Body->OnCollisionEnter(CollisionBody((btCollisionObject*)Manifold->getBody1()));
				}
			};
			gContactEndedCallback = [](btPersistentManifold* const& Manifold) -> void
			{
				btCollisionObject* Body1 = (btCollisionObject*)Manifold->getBody0();
				btRigidBody* Rigid1 = btRigidBody::upcast(Body1);
				btSoftBody* Soft1 = btSoftBody::upcast(Body1);

				if (Rigid1 != nullptr)
				{
					RigidBody* Body = (RigidBody*)Rigid1->getUserPointer();
					if (Body != nullptr && Body->OnCollisionEnter)
						Body->OnCollisionExit(CollisionBody((btCollisionObject*)Manifold->getBody1()));
				}
				else if (Soft1 != nullptr)
				{
					SoftBody* Body = (SoftBody*)Soft1->getUserPointer();
					if (Body != nullptr && Body->OnCollisionEnter)
						Body->OnCollisionExit(CollisionBody((btCollisionObject*)Manifold->getBody1()));
				}
			};
#endif
		}
		Simulator::~Simulator() noexcept
		{
#ifdef VI_BULLET3
			RemoveAll();
			for (auto It = Shapes.begin(); It != Shapes.end(); ++It)
			{
				btCollisionShape* Item = (btCollisionShape*)It->first;
				Core::Memory::Delete(Item);
			}

			Core::Memory::Delete(Dispatcher);
			Core::Memory::Delete(Collision);
			Core::Memory::Delete(Solver);
			Core::Memory::Delete(Broadphase);
			Core::Memory::Delete(SoftSolver);
			Core::Memory::Delete(World);
#endif
		}
		void Simulator::SetGravity(const Trigonometry::Vector3& Gravity)
		{
#ifdef VI_BULLET3
			World->setGravity(V3_TO_BT(Gravity));
#endif
		}
		void Simulator::SetLinearImpulse(const Trigonometry::Vector3& Impulse, bool RandomFactor)
		{
#ifdef VI_BULLET3
			for (int i = 0; i < World->getNumCollisionObjects(); i++)
			{
				Trigonometry::Vector3 Velocity = Impulse * (RandomFactor ? Trigonometry::Vector3::Random() : 1);
				btRigidBody::upcast(World->getCollisionObjectArray()[i])->setLinearVelocity(V3_TO_BT(Velocity));
			}
#endif
		}
		void Simulator::SetLinearImpulse(const Trigonometry::Vector3& Impulse, int Start, int End, bool RandomFactor)
		{
#ifdef VI_BULLET3
			if (Start >= 0 && Start < World->getNumCollisionObjects() && End >= 0 && End < World->getNumCollisionObjects())
			{
				for (int i = Start; i < End; i++)
				{
					Trigonometry::Vector3 Velocity = Impulse * (RandomFactor ? Trigonometry::Vector3::Random() : 1);
					btRigidBody::upcast(World->getCollisionObjectArray()[i])->setLinearVelocity(V3_TO_BT(Velocity));
				}
			}
#endif
		}
		void Simulator::SetAngularImpulse(const Trigonometry::Vector3& Impulse, bool RandomFactor)
		{
#ifdef VI_BULLET3
			for (int i = 0; i < World->getNumCollisionObjects(); i++)
			{
				Trigonometry::Vector3 Velocity = Impulse * (RandomFactor ? Trigonometry::Vector3::Random() : 1);
				btRigidBody::upcast(World->getCollisionObjectArray()[i])->setAngularVelocity(V3_TO_BT(Velocity));
			}
#endif
		}
		void Simulator::SetAngularImpulse(const Trigonometry::Vector3& Impulse, int Start, int End, bool RandomFactor)
		{
#ifdef VI_BULLET3
			if (Start >= 0 && Start < World->getNumCollisionObjects() && End >= 0 && End < World->getNumCollisionObjects())
			{
				for (int i = Start; i < End; i++)
				{
					Trigonometry::Vector3 Velocity = Impulse * (RandomFactor ? Trigonometry::Vector3::Random() : 1);
					btRigidBody::upcast(World->getCollisionObjectArray()[i])->setAngularVelocity(V3_TO_BT(Velocity));
				}
			}
#endif
		}
		void Simulator::SetOnCollisionEnter(ContactStartedCallback Callback)
		{
#ifdef VI_BULLET3
			gContactStartedCallback = Callback;
#endif
		}
		void Simulator::SetOnCollisionExit(ContactEndedCallback Callback)
		{
#ifdef VI_BULLET3
			gContactEndedCallback = Callback;
#endif
		}
		void Simulator::CreateLinearImpulse(const Trigonometry::Vector3& Impulse, bool RandomFactor)
		{
#ifdef VI_BULLET3
			for (int i = 0; i < World->getNumCollisionObjects(); i++)
			{
				btRigidBody* Body = btRigidBody::upcast(World->getCollisionObjectArray()[i]);
				btVector3 Velocity = Body->getLinearVelocity();
				Velocity.setX(Velocity.getX() + Impulse.X * (RandomFactor ? Compute::Mathf::Random() : 1));
				Velocity.setY(Velocity.getY() + Impulse.Y * (RandomFactor ? Compute::Mathf::Random() : 1));
				Velocity.setZ(Velocity.getZ() + Impulse.Z * (RandomFactor ? Compute::Mathf::Random() : 1));
				btRigidBody::upcast(World->getCollisionObjectArray()[i])->setLinearVelocity(Velocity);
			}
#endif
		}
		void Simulator::CreateLinearImpulse(const Trigonometry::Vector3& Impulse, int Start, int End, bool RandomFactor)
		{
#ifdef VI_BULLET3
			if (Start >= 0 && Start < World->getNumCollisionObjects() && End >= 0 && End < World->getNumCollisionObjects())
			{
				for (int i = Start; i < End; i++)
				{
					btRigidBody* Body = btRigidBody::upcast(World->getCollisionObjectArray()[i]);
					btVector3 Velocity = Body->getLinearVelocity();
					Velocity.setX(Velocity.getX() + Impulse.X * (RandomFactor ? Compute::Mathf::Random() : 1));
					Velocity.setY(Velocity.getY() + Impulse.Y * (RandomFactor ? Compute::Mathf::Random() : 1));
					Velocity.setZ(Velocity.getZ() + Impulse.Z * (RandomFactor ? Compute::Mathf::Random() : 1));
					btRigidBody::upcast(World->getCollisionObjectArray()[i])->setLinearVelocity(Velocity);
				}
			}
#endif
		}
		void Simulator::CreateAngularImpulse(const Trigonometry::Vector3& Impulse, bool RandomFactor)
		{
#ifdef VI_BULLET3
			for (int i = 0; i < World->getNumCollisionObjects(); i++)
			{
				btRigidBody* Body = btRigidBody::upcast(World->getCollisionObjectArray()[i]);
				btVector3 Velocity = Body->getAngularVelocity();
				Velocity.setX(Velocity.getX() + Impulse.X * (RandomFactor ? Compute::Mathf::Random() : 1));
				Velocity.setY(Velocity.getY() + Impulse.Y * (RandomFactor ? Compute::Mathf::Random() : 1));
				Velocity.setZ(Velocity.getZ() + Impulse.Z * (RandomFactor ? Compute::Mathf::Random() : 1));
				btRigidBody::upcast(World->getCollisionObjectArray()[i])->setAngularVelocity(Velocity);
			}
#endif
		}
		void Simulator::CreateAngularImpulse(const Trigonometry::Vector3& Impulse, int Start, int End, bool RandomFactor)
		{
#ifdef VI_BULLET3
			if (Start >= 0 && Start < World->getNumCollisionObjects() && End >= 0 && End < World->getNumCollisionObjects())
			{
				for (int i = Start; i < End; i++)
				{
					btRigidBody* Body = btRigidBody::upcast(World->getCollisionObjectArray()[i]);
					btVector3 Velocity = Body->getAngularVelocity();
					Velocity.setX(Velocity.getX() + Impulse.X * (RandomFactor ? Compute::Mathf::Random() : 1));
					Velocity.setY(Velocity.getY() + Impulse.Y * (RandomFactor ? Compute::Mathf::Random() : 1));
					Velocity.setZ(Velocity.getZ() + Impulse.Z * (RandomFactor ? Compute::Mathf::Random() : 1));
					btRigidBody::upcast(World->getCollisionObjectArray()[i])->setAngularVelocity(Velocity);
				}
			}
#endif
		}
		void Simulator::AddSoftBody(SoftBody* Body)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Body != nullptr, "softbody should be set");
			VI_ASSERT(Body->Instance != nullptr, "softbody instance should be set");
			VI_ASSERT(Body->Instance->getWorldArrayIndex() == -1, "softbody should not be attached to other world");
			VI_ASSERT(HasSoftBodySupport(), "softbodies should be supported");
			VI_TRACE("[sim] add soft-body 0x%" PRIXPTR " on 0x%" PRIXPTR, (void*)Body, (void*)this);

			btSoftRigidDynamicsWorld* SoftWorld = (btSoftRigidDynamicsWorld*)World;
			SoftWorld->addSoftBody(Body->Instance);
#endif
		}
		void Simulator::RemoveSoftBody(SoftBody* Body)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Body != nullptr, "softbody should be set");
			VI_ASSERT(Body->Instance != nullptr, "softbody instance should be set");
			VI_ASSERT(Body->Instance->getWorldArrayIndex() >= 0, "softbody should be attached to world");
			VI_ASSERT(HasSoftBodySupport(), "softbodies should be supported");
			VI_TRACE("[sim] remove soft-body 0x%" PRIXPTR " on 0x%" PRIXPTR, (void*)Body, (void*)this);

			btSoftRigidDynamicsWorld* SoftWorld = (btSoftRigidDynamicsWorld*)World;
			SoftWorld->removeSoftBody(Body->Instance);
#endif
		}
		void Simulator::AddRigidBody(RigidBody* Body)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Body != nullptr, "rigidbody should be set");
			VI_ASSERT(Body->Instance != nullptr, "rigidbody instance should be set");
			VI_ASSERT(Body->Instance->getWorldArrayIndex() == -1, "rigidbody should not be attached to other world");
			VI_TRACE("[sim] add rigid-body 0x%" PRIXPTR " on 0x%" PRIXPTR, (void*)Body, (void*)this);
			World->addRigidBody(Body->Instance);
#endif
		}
		void Simulator::RemoveRigidBody(RigidBody* Body)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Body != nullptr, "rigidbody should be set");
			VI_ASSERT(Body->Instance != nullptr, "rigidbody instance should be set");
			VI_ASSERT(Body->Instance->getWorldArrayIndex() >= 0, "rigidbody should be attached to other world");
			VI_TRACE("[sim] remove rigid-body 0x%" PRIXPTR " on 0x%" PRIXPTR, (void*)Body, (void*)this);
			World->removeRigidBody(Body->Instance);
#endif
		}
		void Simulator::AddConstraint(Constraint* Constraint)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Constraint != nullptr, "slider constraint should be set");
			VI_ASSERT(Constraint->Get() != nullptr, "slider constraint instance should be set");
			VI_TRACE("[sim] add constraint 0x%" PRIXPTR " on 0x%" PRIXPTR, (void*)Constraint, (void*)this);
			World->addConstraint(Constraint->Get(), !Constraint->HasCollisions());
#endif
		}
		void Simulator::RemoveConstraint(Constraint* Constraint)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Constraint != nullptr, "slider constraint should be set");
			VI_ASSERT(Constraint->Get() != nullptr, "slider constraint instance should be set");
			VI_TRACE("[sim] remove constraint 0x%" PRIXPTR " on 0x%" PRIXPTR, (void*)Constraint, (void*)this);
			World->removeConstraint(Constraint->Get());
#endif
		}
		void Simulator::RemoveAll()
		{
#ifdef VI_BULLET3
			VI_TRACE("[sim] remove all collision objects on 0x%" PRIXPTR, (void*)this);
			for (int i = 0; i < World->getNumCollisionObjects(); i++)
			{
				btCollisionObject* Object = World->getCollisionObjectArray()[i];
				btRigidBody* Body = btRigidBody::upcast(Object);
				if (Body != nullptr)
				{
					auto* State = Body->getMotionState();
					Core::Memory::Delete(State);
					Body->setMotionState(nullptr);

					auto* Shape = Body->getCollisionShape();
					Core::Memory::Delete(Shape);
					Body->setCollisionShape(nullptr);
				}

				World->removeCollisionObject(Object);
				Core::Memory::Delete(Object);
			}
#endif
		}
		void Simulator::SimulateStep(float ElapsedTime)
		{
#ifdef VI_BULLET3
			if (!Active || Speedup <= 0.0f)
				return;

			VI_MEASURE(Core::Timings::Frame);
			float TimeStep = (Timing.LastElapsedTime > 0.0 ? std::max(0.0f, ElapsedTime - Timing.LastElapsedTime) : 0.0f);
			World->stepSimulation(TimeStep * Speedup, 0);
			Timing.LastElapsedTime = ElapsedTime;
#endif
		}
		void Simulator::FindContacts(RigidBody* Body, int(*Callback)(ShapeContact*, const CollisionBody&, const CollisionBody&))
		{
#ifdef VI_BULLET3
			VI_ASSERT(Callback != nullptr, "callback should not be empty");
			VI_ASSERT(Body != nullptr, "body should be set");
			VI_MEASURE(Core::Timings::Pass);

			FindContactsHandler Handler;
			Handler.Callback = Callback;
			World->contactTest(Body->Get(), Handler);
#endif
		}
		bool Simulator::FindRayContacts(const Trigonometry::Vector3& Start, const Trigonometry::Vector3& End, int(*Callback)(RayContact*, const CollisionBody&))
		{
#ifdef VI_BULLET3
			VI_ASSERT(Callback != nullptr, "callback should not be empty");
			VI_MEASURE(Core::Timings::Pass);

			FindRayContactsHandler Handler;
			Handler.Callback = Callback;

			World->rayTest(btVector3(Start.X, Start.Y, Start.Z), btVector3(End.X, End.Y, End.Z), Handler);
			return Handler.m_collisionObject != nullptr;
#else
			return false;
#endif
		}
		RigidBody* Simulator::CreateRigidBody(const RigidBody::Desc& I, Trigonometry::Transform* Transform)
		{
#ifdef VI_BULLET3
			if (!Transform)
				return CreateRigidBody(I);

			RigidBody::Desc F(I);
			F.Position = Transform->GetPosition();
			F.Rotation = Transform->GetRotation();
			F.Scale = Transform->GetScale();
			return CreateRigidBody(F);
#else
			return nullptr;
#endif
		}
		RigidBody* Simulator::CreateRigidBody(const RigidBody::Desc& I)
		{
#ifdef VI_BULLET3
			return new RigidBody(this, I);
#else
			return nullptr;
#endif
		}
		SoftBody* Simulator::CreateSoftBody(const SoftBody::Desc& I, Trigonometry::Transform* Transform)
		{
#ifdef VI_BULLET3
			if (!Transform)
				return CreateSoftBody(I);

			SoftBody::Desc F(I);
			F.Position = Transform->GetPosition();
			F.Rotation = Transform->GetRotation();
			F.Scale = Transform->GetScale();
			return CreateSoftBody(F);
#else
			return nullptr;
#endif
		}
		SoftBody* Simulator::CreateSoftBody(const SoftBody::Desc& I)
		{
#ifdef VI_BULLET3
			if (!HasSoftBodySupport())
				return nullptr;

			return new SoftBody(this, I);
#else
			return nullptr;
#endif
		}
		PConstraint* Simulator::CreatePoint2PointConstraint(const PConstraint::Desc& I)
		{
#ifdef VI_BULLET3
			return new PConstraint(this, I);
#else
			return nullptr;
#endif
		}
		HConstraint* Simulator::CreateHingeConstraint(const HConstraint::Desc& I)
		{
#ifdef VI_BULLET3
			return new HConstraint(this, I);
#else
			return nullptr;
#endif
		}
		SConstraint* Simulator::CreateSliderConstraint(const SConstraint::Desc& I)
		{
#ifdef VI_BULLET3
			return new SConstraint(this, I);
#else
			return nullptr;
#endif
		}
		CTConstraint* Simulator::CreateConeTwistConstraint(const CTConstraint::Desc& I)
		{
#ifdef VI_BULLET3
			return new CTConstraint(this, I);
#else
			return nullptr;
#endif
		}
		DF6Constraint* Simulator::Create6DoFConstraint(const DF6Constraint::Desc& I)
		{
#ifdef VI_BULLET3
			return new DF6Constraint(this, I);
#else
			return nullptr;
#endif
		}
		btCollisionShape* Simulator::CreateCube(const Trigonometry::Vector3& Scale)
		{
#ifdef VI_BULLET3
			btCollisionShape* Shape = Core::Memory::New<btBoxShape>(V3_TO_BT(Scale));
			VI_TRACE("[sim] save cube shape 0x%" PRIXPTR, (void*)Shape);
			Core::UMutex<std::mutex> Unique(Exclusive);
			Shapes[Shape] = 1;
			return Shape;
#else
			return nullptr;
#endif
		}
		btCollisionShape* Simulator::CreateSphere(float Radius)
		{
#ifdef VI_BULLET3
			btCollisionShape* Shape = Core::Memory::New<btSphereShape>(Radius);
			VI_TRACE("[sim] save sphere shape 0x%" PRIXPTR, (void*)Shape);
			Core::UMutex<std::mutex> Unique(Exclusive);
			Shapes[Shape] = 1;
			return Shape;
#else
			return nullptr;
#endif
		}
		btCollisionShape* Simulator::CreateCapsule(float Radius, float Height)
		{
#ifdef VI_BULLET3
			btCollisionShape* Shape = Core::Memory::New<btCapsuleShape>(Radius, Height);
			VI_TRACE("[sim] save capsule shape 0x%" PRIXPTR, (void*)Shape);
			Core::UMutex<std::mutex> Unique(Exclusive);
			Shapes[Shape] = 1;
			return Shape;
#else
			return nullptr;
#endif
		}
		btCollisionShape* Simulator::CreateCone(float Radius, float Height)
		{
#ifdef VI_BULLET3
			btCollisionShape* Shape = Core::Memory::New<btConeShape>(Radius, Height);
			VI_TRACE("[sim] save cone shape 0x%" PRIXPTR, (void*)Shape);
			Core::UMutex<std::mutex> Unique(Exclusive);
			Shapes[Shape] = 1;
			return Shape;
#else
			return nullptr;
#endif
		}
		btCollisionShape* Simulator::CreateCylinder(const Trigonometry::Vector3& Scale)
		{
#ifdef VI_BULLET3
			btCollisionShape* Shape = Core::Memory::New<btCylinderShape>(V3_TO_BT(Scale));
			VI_TRACE("[sim] save cylinder shape 0x%" PRIXPTR, (void*)Shape);
			Core::UMutex<std::mutex> Unique(Exclusive);
			Shapes[Shape] = 1;
			return Shape;
#else
			return nullptr;
#endif
		}
		btCollisionShape* Simulator::CreateConvexHull(Core::Vector<Trigonometry::SkinVertex>& Vertices)
		{
#ifdef VI_BULLET3
			btConvexHullShape* Shape = Core::Memory::New<btConvexHullShape>();
			for (auto It = Vertices.begin(); It != Vertices.end(); ++It)
				Shape->addPoint(btVector3(It->PositionX, It->PositionY, It->PositionZ), false);

			Shape->recalcLocalAabb();
			Shape->optimizeConvexHull();
			Shape->setMargin(0);

			VI_TRACE("[sim] save convext-hull shape 0x%" PRIXPTR " (%" PRIu64 " vertices)", (void*)Shape, (uint64_t)Vertices.size());
			Core::UMutex<std::mutex> Unique(Exclusive);
			Shapes[Shape] = 1;
			return Shape;
#else
			return nullptr;
#endif
		}
		btCollisionShape* Simulator::CreateConvexHull(Core::Vector<Trigonometry::Vertex>& Vertices)
		{
#ifdef VI_BULLET3
			btConvexHullShape* Shape = Core::Memory::New<btConvexHullShape>();
			for (auto It = Vertices.begin(); It != Vertices.end(); ++It)
				Shape->addPoint(btVector3(It->PositionX, It->PositionY, It->PositionZ), false);

			Shape->recalcLocalAabb();
			Shape->optimizeConvexHull();
			Shape->setMargin(0);

			VI_TRACE("[sim] save convext-hull shape 0x%" PRIXPTR " (%" PRIu64 " vertices)", (void*)Shape, (uint64_t)Vertices.size());
			Core::UMutex<std::mutex> Unique(Exclusive);
			Shapes[Shape] = 1;
			return Shape;
#else
			return nullptr;
#endif
		}
		btCollisionShape* Simulator::CreateConvexHull(Core::Vector<Trigonometry::Vector2>& Vertices)
		{
#ifdef VI_BULLET3
			btConvexHullShape* Shape = Core::Memory::New<btConvexHullShape>();
			for (auto It = Vertices.begin(); It != Vertices.end(); ++It)
				Shape->addPoint(btVector3(It->X, It->Y, 0), false);

			Shape->recalcLocalAabb();
			Shape->optimizeConvexHull();
			Shape->setMargin(0);

			VI_TRACE("[sim] save convext-hull shape 0x%" PRIXPTR " (%" PRIu64 " vertices)", (void*)Shape, (uint64_t)Vertices.size());
			Core::UMutex<std::mutex> Unique(Exclusive);
			Shapes[Shape] = 1;
			return Shape;
#else
			return nullptr;
#endif
		}
		btCollisionShape* Simulator::CreateConvexHull(Core::Vector<Trigonometry::Vector3>& Vertices)
		{
#ifdef VI_BULLET3
			btConvexHullShape* Shape = Core::Memory::New<btConvexHullShape>();
			for (auto It = Vertices.begin(); It != Vertices.end(); ++It)
				Shape->addPoint(btVector3(It->X, It->Y, It->Z), false);

			Shape->recalcLocalAabb();
			Shape->optimizeConvexHull();
			Shape->setMargin(0);

			VI_TRACE("[sim] save convext-hull shape 0x%" PRIXPTR " (%" PRIu64 " vertices)", (void*)Shape, (uint64_t)Vertices.size());
			Core::UMutex<std::mutex> Unique(Exclusive);
			Shapes[Shape] = 1;
			return Shape;
#else
			return nullptr;
#endif
		}
		btCollisionShape* Simulator::CreateConvexHull(Core::Vector<Trigonometry::Vector4>& Vertices)
		{
#ifdef VI_BULLET3
			btConvexHullShape* Shape = Core::Memory::New<btConvexHullShape>();
			for (auto It = Vertices.begin(); It != Vertices.end(); ++It)
				Shape->addPoint(btVector3(It->X, It->Y, It->Z), false);

			Shape->recalcLocalAabb();
			Shape->optimizeConvexHull();
			Shape->setMargin(0);

			VI_TRACE("[sim] save convext-hull shape 0x%" PRIXPTR " (%" PRIu64 " vertices)", (void*)Shape, (uint64_t)Vertices.size());
			Core::UMutex<std::mutex> Unique(Exclusive);
			Shapes[Shape] = 1;
			return Shape;
#else
			return nullptr;
#endif
		}
		btCollisionShape* Simulator::CreateConvexHull(btCollisionShape* From)
		{
#ifdef VI_BULLET3
			VI_ASSERT(From != nullptr, "shape should be set");
			VI_ASSERT(From->getShapeType() == (int)Shape::Convex_Hull, "shape type should be convex hull");

			btConvexHullShape* Hull = Core::Memory::New<btConvexHullShape>();
			btConvexHullShape* Base = (btConvexHullShape*)From;

			for (size_t i = 0; i < (size_t)Base->getNumPoints(); i++)
				Hull->addPoint(*(Base->getUnscaledPoints() + i), false);

			Hull->recalcLocalAabb();
			Hull->optimizeConvexHull();
			Hull->setMargin(0);

			VI_TRACE("[sim] save convext-hull shape 0x%" PRIXPTR " (%" PRIu64 " vertices)", (void*)Hull, (uint64_t)Base->getNumPoints());
			Core::UMutex<std::mutex> Unique(Exclusive);
			Shapes[Hull] = 1;
			return Hull;
#else
			return nullptr;
#endif
		}
		btCollisionShape* Simulator::CreateShape(Shape Wanted)
		{
#ifdef VI_BULLET3
			switch (Wanted)
			{
				case Shape::Box:
					return CreateCube();
				case Shape::Sphere:
					return CreateSphere();
				case Shape::Capsule:
					return CreateCapsule();
				case Shape::Cone:
					return CreateCone();
				case Shape::Cylinder:
					return CreateCylinder();
				default:
					return nullptr;
			}
#else
			return nullptr;
#endif
		}
		btCollisionShape* Simulator::TryCloneShape(btCollisionShape* Value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Value != nullptr, "shape should be set");
			Shape Type = (Shape)Value->getShapeType();
			if (Type == Shape::Box)
			{
				btBoxShape* Box = (btBoxShape*)Value;
				btVector3 Size = Box->getHalfExtentsWithMargin() / Box->getLocalScaling();
				return CreateCube(BT_TO_V3(Size));
			}
			else if (Type == Shape::Sphere)
			{
				btSphereShape* Sphere = (btSphereShape*)Value;
				return CreateSphere(Sphere->getRadius());
			}
			else if (Type == Shape::Capsule)
			{
				btCapsuleShape* Capsule = (btCapsuleShape*)Value;
				return CreateCapsule(Capsule->getRadius(), Capsule->getHalfHeight() * 2.0f);
			}
			else if (Type == Shape::Cone)
			{
				btConeShape* Cone = (btConeShape*)Value;
				return CreateCone(Cone->getRadius(), Cone->getHeight());
			}
			else if (Type == Shape::Cylinder)
			{
				btCylinderShape* Cylinder = (btCylinderShape*)Value;
				btVector3 Size = Cylinder->getHalfExtentsWithMargin() / Cylinder->getLocalScaling();
				return CreateCylinder(BT_TO_V3(Size));
			}
			else if (Type == Shape::Convex_Hull)
				return CreateConvexHull(Value);

			return nullptr;
#else
			return nullptr;
#endif
		}
		btCollisionShape* Simulator::ReuseShape(btCollisionShape* Value)
		{
#ifdef VI_BULLET3
			VI_ASSERT(Value != nullptr, "shape should be set");
			Core::UMutex<std::mutex> Unique(Exclusive);
			auto It = Shapes.find(Value);
			if (It == Shapes.end())
				return nullptr;

			It->second++;
			return Value;
#else
			return nullptr;
#endif
		}
		void Simulator::FreeShape(btCollisionShape** Value)
		{
#ifdef VI_BULLET3
			if (!Value || !*Value)
				return;

			Core::UMutex<std::mutex> Unique(Exclusive);
			auto It = Shapes.find(*Value);
			if (It == Shapes.end())
				return;

			*Value = nullptr;
			if (It->second-- > 1)
				return;

			btCollisionShape* Item = (btCollisionShape*)It->first;
			VI_TRACE("[sim] free shape 0x%" PRIXPTR, (void*)Item);
			Core::Memory::Delete(Item);
			Shapes.erase(It);
#endif
		}
		Core::Vector<Trigonometry::Vector3> Simulator::GetShapeVertices(btCollisionShape* Value) const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Value != nullptr, "shape should be set");
			auto Type = (Shape)Value->getShapeType();
			if (Type != Shape::Convex_Hull)
				return Core::Vector<Trigonometry::Vector3>();

			btConvexHullShape* Hull = (btConvexHullShape*)Value;
			btVector3* Points = Hull->getUnscaledPoints();
			size_t Count = Hull->getNumPoints();
			Core::Vector<Trigonometry::Vector3> Vertices;
			Vertices.reserve(Count);

			for (size_t i = 0; i < Count; i++)
			{
				btVector3& It = Points[i];
				Vertices.emplace_back(It.getX(), It.getY(), It.getZ());
			}

			return Vertices;
#else
			return Core::Vector<Trigonometry::Vector3>();
#endif
		}
		size_t Simulator::GetShapeVerticesCount(btCollisionShape* Value) const
		{
#ifdef VI_BULLET3
			VI_ASSERT(Value != nullptr, "shape should be set");
			auto Type = (Shape)Value->getShapeType();
			if (Type != Shape::Convex_Hull)
				return 0;

			btConvexHullShape* Hull = (btConvexHullShape*)Value;
			return Hull->getNumPoints();
#else
			return 0;
#endif
		}
		float Simulator::GetMaxDisplacement() const
		{
#ifdef VI_BULLET3
			if (!SoftSolver || !World)
				return 1000;

			return ((btSoftRigidDynamicsWorld*)World)->getWorldInfo().m_maxDisplacement;
#else
			return 1000;
#endif
		}
		float Simulator::GetAirDensity() const
		{
#ifdef VI_BULLET3
			if (!SoftSolver || !World)
				return 1.2f;

			return ((btSoftRigidDynamicsWorld*)World)->getWorldInfo().air_density;
#else
			return 1.2f;
#endif
		}
		float Simulator::GetWaterOffset() const
		{
#ifdef VI_BULLET3
			if (!SoftSolver || !World)
				return 0;

			return ((btSoftRigidDynamicsWorld*)World)->getWorldInfo().water_offset;
#else
			return 0;
#endif
		}
		float Simulator::GetWaterDensity() const
		{
#ifdef VI_BULLET3
			if (!SoftSolver || !World)
				return 0;

			return ((btSoftRigidDynamicsWorld*)World)->getWorldInfo().water_density;
#else
			return 0;
#endif
		}
		Trigonometry::Vector3 Simulator::GetWaterNormal() const
		{
#ifdef VI_BULLET3
			if (!SoftSolver || !World)
				return 0;

			btVector3 Value = ((btSoftRigidDynamicsWorld*)World)->getWorldInfo().water_normal;
			return BT_TO_V3(Value);
#else
			return 0;
#endif
		}
		Trigonometry::Vector3 Simulator::GetGravity() const
		{
#ifdef VI_BULLET3
			if (!World)
				return 0;

			btVector3 Value = World->getGravity();
			return BT_TO_V3(Value);
#else
			return 0;
#endif
		}
		ContactStartedCallback Simulator::GetOnCollisionEnter() const
		{
#ifdef VI_BULLET3
			return gContactStartedCallback;
#else
			return nullptr;
#endif
		}
		ContactEndedCallback Simulator::GetOnCollisionExit() const
		{
#ifdef VI_BULLET3
			return gContactEndedCallback;
#else
			return nullptr;
#endif
		}
		btCollisionConfiguration* Simulator::GetCollision() const
		{
#ifdef VI_BULLET3
			return Collision;
#else
			return nullptr;
#endif
		}
		btBroadphaseInterface* Simulator::GetBroadphase() const
		{
#ifdef VI_BULLET3
			return Broadphase;
#else
			return nullptr;
#endif
		}
		btConstraintSolver* Simulator::GetSolver() const
		{
#ifdef VI_BULLET3
			return Solver;
#else
			return nullptr;
#endif
		}
		btDiscreteDynamicsWorld* Simulator::GetWorld() const
		{
#ifdef VI_BULLET3
			return World;
#else
			return nullptr;
#endif
		}
		btCollisionDispatcher* Simulator::GetDispatcher() const
		{
#ifdef VI_BULLET3
			return Dispatcher;
#else
			return nullptr;
#endif
		}
		btSoftBodySolver* Simulator::GetSoftSolver() const
		{
#ifdef VI_BULLET3
			return SoftSolver;
#else
			return nullptr;
#endif
		}
		bool Simulator::HasSoftBodySupport() const
		{
#ifdef VI_BULLET3
			return SoftSolver != nullptr;
#else
			return false;
#endif
		}
		int Simulator::GetContactManifoldCount() const
		{
#ifdef VI_BULLET3
			if (!Dispatcher)
				return 0;

			return Dispatcher->getNumManifolds();
#else
			return 0;
#endif
		}
		Simulator* Simulator::Get(btDiscreteDynamicsWorld* From)
		{
#ifdef VI_BULLET3
			if (!From)
				return nullptr;

			return (Simulator*)From->getWorldUserInfo();
#else
			return nullptr;
#endif
		}
	}
}
