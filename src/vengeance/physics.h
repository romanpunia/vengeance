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
typedef bool(*ContactDestroyedCallback)(void*);
typedef bool(*ContactProcessedCallback)(class btManifoldPoint&, void*, void*);
typedef void(*ContactStartedCallback)(class btPersistentManifold* const&);
typedef void(*ContactEndedCallback)(class btPersistentManifold* const&);

namespace Vitex
{
	namespace Physics
	{
		class RigidBody;

		class SoftBody;

		class Simulator;

		enum class Shape
		{
			Box,
			Triangle,
			Tetrahedral,
			Convex_Triangle_Mesh,
			Convex_Hull,
			Convex_Point_Cloud,
			Convex_Polyhedral,
			Implicit_Convex_Start,
			Sphere,
			Multi_Sphere,
			Capsule,
			Cone,
			Convex,
			Cylinder,
			Uniform_Scaling,
			Minkowski_Sum,
			Minkowski_Difference,
			Box_2D,
			Convex_2D,
			Custom_Convex,
			Concaves_Start,
			Triangle_Mesh,
			Triangle_Mesh_Scaled,
			Fast_Concave_Mesh,
			Terrain,
			Gimpact,
			Triangle_Mesh_Multimaterial,
			Empty,
			Static_Plane,
			Custom_Concave,
			Concaves_End,
			Compound,
			Softbody,
			HF_Fluid,
			HF_Fluid_Bouyant_Convex,
			Invalid,
			Count
		};

		enum class MotionState
		{
			Active = 1,
			Island_Sleeping = 2,
			Deactivation_Needed = 3,
			Disable_Deactivation = 4,
			Disable_Simulation = 5,
		};

		enum class SoftFeature
		{
			None,
			Node,
			Link,
			Face,
			Tetra
		};

		enum class SoftAeroModel
		{
			VPoint,
			VTwoSided,
			VTwoSidedLiftDrag,
			VOneSided,
			FTwoSided,
			FTwoSidedLiftDrag,
			FOneSided
		};

		enum class SoftCollision
		{
			RVS_Mask = 0x000f,
			SDF_RS = 0x0001,
			CL_RS = 0x0002,
			SDF_RD = 0x0003,
			SDF_RDF = 0x0004,
			SVS_Mask = 0x00F0,
			VF_SS = 0x0010,
			CL_SS = 0x0020,
			CL_Self = 0x0040,
			VF_DD = 0x0050,
			Default = SDF_RS | CL_SS
		};

		inline SoftCollision operator |(SoftCollision A, SoftCollision B)
		{
			return static_cast<SoftCollision>(static_cast<uint64_t>(A) | static_cast<uint64_t>(B));
		}

		typedef std::function<void(const struct CollisionBody&)> CollisionCallback;

		struct VI_OUT ShapeContact
		{
			Trigonometry::Vector3 LocalPoint1;
			Trigonometry::Vector3 LocalPoint2;
			Trigonometry::Vector3 PositionWorld1;
			Trigonometry::Vector3 PositionWorld2;
			Trigonometry::Vector3 NormalWorld;
			Trigonometry::Vector3 LateralFrictionDirection1;
			Trigonometry::Vector3 LateralFrictionDirection2;
			float Distance = 0.0f;
			float CombinedFriction = 0.0f;
			float CombinedRollingFriction = 0.0f;
			float CombinedSpinningFriction = 0.0f;
			float CombinedRestitution = 0.0f;
			float AppliedImpulse = 0.0f;
			float AppliedImpulseLateral1 = 0.0f;
			float AppliedImpulseLateral2 = 0.0f;
			float ContactMotion1 = 0.0f;
			float ContactMotion2 = 0.0f;
			float ContactCFM = 0.0f;
			float CombinedContactStiffness = 0.0f;
			float ContactERP = 0.0f;
			float CombinedContactDamping = 0.0f;
			float FrictionCFM = 0.0f;
			int PartId1 = 0;
			int PartId2 = 0;
			int Index1 = 0;
			int Index2 = 0;
			int ContactPointFlags = 0;
			int LifeTime = 0;
		};

		struct VI_OUT RayContact
		{
			Trigonometry::Vector3 HitNormalLocal;
			Trigonometry::Vector3 HitNormalWorld;
			Trigonometry::Vector3 HitPointWorld;
			Trigonometry::Vector3 RayFromWorld;
			Trigonometry::Vector3 RayToWorld;
			float HitFraction = 0.0f;
			float ClosestHitFraction = 0.0f;
			bool NormalInWorldSpace = false;
		};

		struct VI_OUT CollisionBody
		{
			RigidBody* Rigid = nullptr;
			SoftBody* Soft = nullptr;

			CollisionBody(btCollisionObject* Object) noexcept;
		};

		class VI_OUT HullShape final : public Core::Reference<HullShape>
		{
		private:
			Core::Vector<Trigonometry::Vertex> Vertices;
			Core::Vector<int> Indices;
			btCollisionShape* Shape;

		public:
			HullShape(Core::Vector<Trigonometry::Vertex>&& NewVertices, Core::Vector<int>&& Indices) noexcept;
			HullShape(Core::Vector<Trigonometry::Vertex>&& NewVertices) noexcept;
			HullShape(btCollisionShape* From) noexcept;
			~HullShape() noexcept;
			const Core::Vector<Trigonometry::Vertex>& GetVertices() const;
			const Core::Vector<int>& GetIndices() const;
			btCollisionShape* GetShape() const;
		};

		class VI_OUT RigidBody final : public Core::Reference<RigidBody>
		{
			friend Simulator;

		public:
			struct Desc
			{
				btCollisionShape* Shape = nullptr;
				float Anticipation = 0, Mass = 0;
				Trigonometry::Vector3 Position;
				Trigonometry::Vector3 Rotation;
				Trigonometry::Vector3 Scale;
			};

		private:
			btRigidBody* Instance;
			Simulator* Engine;
			Desc Initial;

		public:
			CollisionCallback OnCollisionEnter;
			CollisionCallback OnCollisionExit;
			void* UserPointer;

		private:
			RigidBody(Simulator* Refer, const Desc& I) noexcept;

		public:
			~RigidBody() noexcept;
			Core::Unique<RigidBody> Copy();
			void Push(const Trigonometry::Vector3& Velocity);
			void Push(const Trigonometry::Vector3& Velocity, const Trigonometry::Vector3& Torque);
			void Push(const Trigonometry::Vector3& Velocity, const Trigonometry::Vector3& Torque, const Trigonometry::Vector3& Center);
			void PushKinematic(const Trigonometry::Vector3& Velocity);
			void PushKinematic(const Trigonometry::Vector3& Velocity, const Trigonometry::Vector3& Torque);
			void Synchronize(Trigonometry::Transform* Transform, bool Kinematic);
			void SetCollisionFlags(size_t Flags);
			void SetActivity(bool Active);
			void SetAsGhost();
			void SetAsNormal();
			void SetSelfPointer();
			void SetWorldTransform(btTransform* Value);
			void SetCollisionShape(btCollisionShape* Shape, Trigonometry::Transform* Transform);
			void SetMass(float Mass);
			void SetActivationState(MotionState Value);
			void SetAngularDamping(float Value);
			void SetAngularSleepingThreshold(float Value);
			void SetSpinningFriction(float Value);
			void SetContactStiffness(float Value);
			void SetContactDamping(float Value);
			void SetFriction(float Value);
			void SetRestitution(float Value);
			void SetHitFraction(float Value);
			void SetLinearDamping(float Value);
			void SetLinearSleepingThreshold(float Value);
			void SetCcdMotionThreshold(float Value);
			void SetCcdSweptSphereRadius(float Value);
			void SetContactProcessingThreshold(float Value);
			void SetDeactivationTime(float Value);
			void SetRollingFriction(float Value);
			void SetAngularFactor(const Trigonometry::Vector3& Value);
			void SetAnisotropicFriction(const Trigonometry::Vector3& Value);
			void SetGravity(const Trigonometry::Vector3& Value);
			void SetLinearFactor(const Trigonometry::Vector3& Value);
			void SetLinearVelocity(const Trigonometry::Vector3& Value);
			void SetAngularVelocity(const Trigonometry::Vector3& Value);
			MotionState GetActivationState() const;
			Shape GetCollisionShapeType() const;
			Trigonometry::Vector3 GetAngularFactor() const;
			Trigonometry::Vector3 GetAnisotropicFriction() const;
			Trigonometry::Vector3 GetGravity() const;
			Trigonometry::Vector3 GetLinearFactor() const;
			Trigonometry::Vector3 GetLinearVelocity() const;
			Trigonometry::Vector3 GetAngularVelocity() const;
			Trigonometry::Vector3 GetScale() const;
			Trigonometry::Vector3 GetPosition() const;
			Trigonometry::Vector3 GetRotation() const;
			btTransform* GetWorldTransform() const;
			btCollisionShape* GetCollisionShape() const;
			btRigidBody* Get() const;
			bool IsActive() const;
			bool IsStatic() const;
			bool IsGhost() const;
			bool IsColliding() const;
			float GetSpinningFriction() const;
			float GetContactStiffness() const;
			float GetContactDamping() const;
			float GetAngularDamping() const;
			float GetAngularSleepingThreshold() const;
			float GetFriction() const;
			float GetRestitution() const;
			float GetHitFraction() const;
			float GetLinearDamping() const;
			float GetLinearSleepingThreshold() const;
			float GetCcdMotionThreshold() const;
			float GetCcdSweptSphereRadius() const;
			float GetContactProcessingThreshold() const;
			float GetDeactivationTime() const;
			float GetRollingFriction() const;
			float GetMass() const;
			size_t GetCollisionFlags() const;
			Desc& GetInitialState();
			Simulator* GetSimulator() const;

		public:
			static RigidBody* Get(btRigidBody* From);
		};

		class VI_OUT SoftBody final : public Core::Reference<SoftBody>
		{
			friend Simulator;

		public:
			struct Desc
			{
				struct CV
				{
					struct SConvex
					{
						HullShape* Hull = nullptr;
						bool Enabled = false;
					} Convex;

					struct SRope
					{
						bool StartFixed = false;
						bool EndFixed = false;
						bool Enabled = false;
						int Count = 0;
						Trigonometry::Vector3 Start = 0;
						Trigonometry::Vector3 End = 1;
					} Rope;

					struct SPatch
					{
						bool GenerateDiagonals = false;
						bool Corner00Fixed = false;
						bool Corner10Fixed = false;
						bool Corner01Fixed = false;
						bool Corner11Fixed = false;
						bool Enabled = false;
						int CountX = 2;
						int CountY = 2;
						Trigonometry::Vector3 Corner00 = Trigonometry::Vector3(0, 0);
						Trigonometry::Vector3 Corner10 = Trigonometry::Vector3(1, 0);
						Trigonometry::Vector3 Corner01 = Trigonometry::Vector3(0, 1);
						Trigonometry::Vector3 Corner11 = Trigonometry::Vector3(1, 1);
					} Patch;

					struct SEllipsoid
					{
						Trigonometry::Vector3 Center;
						Trigonometry::Vector3 Radius = 1;
						int Count = 3;
						bool Enabled = false;
					} Ellipsoid;
				} Shape;

				struct SConfig
				{
					SoftAeroModel AeroModel = SoftAeroModel::VPoint;
					float VCF = 1;
					float DP = 0;
					float DG = 0;
					float LF = 0;
					float PR = 1.0f;
					float VC = 0.1f;
					float DF = 0.5f;
					float MT = 0.1f;
					float CHR = 1;
					float KHR = 0.1f;
					float SHR = 1;
					float AHR = 0.7f;
					float SRHR_CL = 0.1f;
					float SKHR_CL = 1;
					float SSHR_CL = 0.5f;
					float SR_SPLT_CL = 0.5f;
					float SK_SPLT_CL = 0.5f;
					float SS_SPLT_CL = 0.5f;
					float MaxVolume = 1;
					float TimeScale = 1;
					float Drag = 0;
					float MaxStress = 0;
					int Clusters = 0;
					int Constraints = 2;
					int VIterations = 10;
					int PIterations = 2;
					int DIterations = 0;
					int CIterations = 4;
					int Collisions = (int)(SoftCollision::Default | SoftCollision::VF_SS);
				} Config;

				float Anticipation = 0;
				Trigonometry::Vector3 Position;
				Trigonometry::Vector3 Rotation;
				Trigonometry::Vector3 Scale;
			};

			struct RayCast
			{
				SoftBody* Body = nullptr;
				SoftFeature Feature = SoftFeature::None;
				float Fraction = 0;
				int Index = 0;
			};

		private:
			btSoftBody* Instance;
			Simulator* Engine;
			Trigonometry::Vector3 Center;
			Desc Initial;

		public:
			CollisionCallback OnCollisionEnter;
			CollisionCallback OnCollisionExit;
			void* UserPointer;

		private:
			SoftBody(Simulator* Refer, const Desc& I) noexcept;

		public:
			~SoftBody() noexcept;
			Core::Unique<SoftBody> Copy();
			void Activate(bool Force);
			void Synchronize(Trigonometry::Transform* Transform, bool Kinematic);
			void GetIndices(Core::Vector<int>* Indices) const;
			void GetVertices(Core::Vector<Trigonometry::Vertex>* Vertices) const;
			void GetBoundingBox(Trigonometry::Vector3* Min, Trigonometry::Vector3* Max) const;
			void SetContactStiffnessAndDamping(float Stiffness, float Damping);
			void AddAnchor(int Node, RigidBody* Body, bool DisableCollisionBetweenLinkedBodies = false, float Influence = 1);
			void AddAnchor(int Node, RigidBody* Body, const Trigonometry::Vector3& LocalPivot, bool DisableCollisionBetweenLinkedBodies = false, float Influence = 1);
			void AddForce(const Trigonometry::Vector3& Force);
			void AddForce(const Trigonometry::Vector3& Force, int Node);
			void AddAeroForceToNode(const Trigonometry::Vector3& WindVelocity, int NodeIndex);
			void AddAeroForceToFace(const Trigonometry::Vector3& WindVelocity, int FaceIndex);
			void AddVelocity(const Trigonometry::Vector3& Velocity);
			void SetVelocity(const Trigonometry::Vector3& Velocity);
			void AddVelocity(const Trigonometry::Vector3& Velocity, int Node);
			void SetMass(int Node, float Mass);
			void SetTotalMass(float Mass, bool FromFaces = false);
			void SetTotalDensity(float Density);
			void SetVolumeMass(float Mass);
			void SetVolumeDensity(float Density);
			void Translate(const Trigonometry::Vector3& Position);
			void Rotate(const Trigonometry::Vector3& Rotation);
			void Scale(const Trigonometry::Vector3& Scale);
			void SetRestLengthScale(float RestLength);
			void SetPose(bool Volume, bool Frame);
			float GetMass(int Node) const;
			float GetTotalMass() const;
			float GetRestLengthScale() const;
			float GetVolume() const;
			int GenerateBendingConstraints(int Distance);
			void RandomizeConstraints();
			bool CutLink(int Node0, int Node1, float Position);
			bool RayTest(const Trigonometry::Vector3& From, const Trigonometry::Vector3& To, RayCast& Result);
			void SetWindVelocity(const Trigonometry::Vector3& Velocity);
			Trigonometry::Vector3 GetWindVelocity() const;
			void GetAabb(Trigonometry::Vector3& Min, Trigonometry::Vector3& Max) const;
			void IndicesToPointers(const int* Map = 0);
			void SetSpinningFriction(float Value);
			Trigonometry::Vector3 GetLinearVelocity() const;
			Trigonometry::Vector3 GetAngularVelocity() const;
			Trigonometry::Vector3 GetCenterPosition() const;
			void SetActivity(bool Active);
			void SetAsGhost();
			void SetAsNormal();
			void SetSelfPointer();
			void SetWorldTransform(btTransform* Value);
			void SetActivationState(MotionState Value);
			void SetContactStiffness(float Value);
			void SetContactDamping(float Value);
			void SetFriction(float Value);
			void SetRestitution(float Value);
			void SetHitFraction(float Value);
			void SetCcdMotionThreshold(float Value);
			void SetCcdSweptSphereRadius(float Value);
			void SetContactProcessingThreshold(float Value);
			void SetDeactivationTime(float Value);
			void SetRollingFriction(float Value);
			void SetAnisotropicFriction(const Trigonometry::Vector3& Value);
			void SetConfig(const Desc::SConfig& Conf);
			Shape GetCollisionShapeType() const;
			MotionState GetActivationState() const;
			Trigonometry::Vector3 GetAnisotropicFriction() const;
			Trigonometry::Vector3 GetScale() const;
			Trigonometry::Vector3 GetPosition() const;
			Trigonometry::Vector3 GetRotation() const;
			btTransform* GetWorldTransform() const;
			btSoftBody* Get() const;
			bool IsActive() const;
			bool IsStatic() const;
			bool IsGhost() const;
			bool IsColliding() const;
			float GetSpinningFriction() const;
			float GetContactStiffness() const;
			float GetContactDamping() const;
			float GetFriction() const;
			float GetRestitution() const;
			float GetHitFraction() const;
			float GetCcdMotionThreshold() const;
			float GetCcdSweptSphereRadius() const;
			float GetContactProcessingThreshold() const;
			float GetDeactivationTime() const;
			float GetRollingFriction() const;
			size_t GetCollisionFlags() const;
			size_t GetVerticesCount() const;
			Desc& GetInitialState();
			Simulator* GetSimulator() const;

		public:
			static SoftBody* Get(btSoftBody* From);
		};

		class VI_OUT Constraint : public Core::Reference<Constraint>
		{
		protected:
			btRigidBody* First, * Second;
			Simulator* Engine;

		public:
			void* UserPointer;

		protected:
			Constraint(Simulator* Refer) noexcept;

		public:
			virtual ~Constraint() noexcept = default;
			virtual Core::Unique<Constraint> Copy() const = 0;
			virtual btTypedConstraint* Get() const = 0;
			virtual bool HasCollisions() const = 0;
			void SetBreakingImpulseThreshold(float Value);
			void SetEnabled(bool Value);
			bool IsEnabled() const;
			bool IsActive() const;
			float GetBreakingImpulseThreshold() const;
			btRigidBody* GetFirst() const;
			btRigidBody* GetSecond() const;
			Simulator* GetSimulator() const;
		};

		class VI_OUT PConstraint : public Constraint
		{
			friend RigidBody;
			friend Simulator;

		public:
			struct Desc
			{
				RigidBody* TargetA = nullptr;
				RigidBody* TargetB = nullptr;
				Trigonometry::Vector3 PivotA;
				Trigonometry::Vector3 PivotB;
				bool Collisions = true;
			};

		private:
			btPoint2PointConstraint* Instance;
			Desc State;

		private:
			PConstraint(Simulator* Refer, const Desc& I) noexcept;

		public:
			~PConstraint() noexcept override;
			Core::Unique<Constraint> Copy() const override;
			btTypedConstraint* Get() const override;
			bool HasCollisions() const override;
			void SetPivotA(const Trigonometry::Vector3& Value);
			void SetPivotB(const Trigonometry::Vector3& Value);
			Trigonometry::Vector3 GetPivotA() const;
			Trigonometry::Vector3 GetPivotB() const;
			Desc& GetState();
		};

		class VI_OUT HConstraint : public Constraint
		{
			friend RigidBody;
			friend Simulator;

		public:
			struct Desc
			{
				RigidBody* TargetA = nullptr;
				RigidBody* TargetB = nullptr;
				bool References = false;
				bool Collisions = true;
			};

		private:
			btHingeConstraint* Instance;
			Desc State;

		private:
			HConstraint(Simulator* Refer, const Desc& I) noexcept;

		public:
			~HConstraint() noexcept override;
			Core::Unique<Constraint> Copy() const override;
			btTypedConstraint* Get() const override;
			bool HasCollisions() const override;
			void EnableAngularMotor(bool Enable, float TargetVelocity, float MaxMotorImpulse);
			void EnableMotor(bool Enable);
			void TestLimit(const Trigonometry::Matrix4x4& A, const Trigonometry::Matrix4x4& B);
			void SetFrames(const Trigonometry::Matrix4x4& A, const Trigonometry::Matrix4x4& B);
			void SetAngularOnly(bool Value);
			void SetMaxMotorImpulse(float Value);
			void SetMotorTargetVelocity(float Value);
			void SetMotorTarget(float TargetAngle, float Delta);
			void SetLimit(float Low, float High, float Softness = 0.9f, float BiasFactor = 0.3f, float RelaxationFactor = 1.0f);
			void SetOffset(bool Value);
			void SetReferenceToA(bool Value);
			void SetAxis(const Trigonometry::Vector3& Value);
			int GetSolveLimit() const;
			float GetMotorTargetVelocity() const;
			float GetMaxMotorImpulse() const;
			float GetLimitSign() const;
			float GetHingeAngle() const;
			float GetHingeAngle(const Trigonometry::Matrix4x4& A, const Trigonometry::Matrix4x4& B) const;
			float GetLowerLimit() const;
			float GetUpperLimit() const;
			float GetLimitSoftness() const;
			float GetLimitBiasFactor() const;
			float GetLimitRelaxationFactor() const;
			bool HasLimit() const;
			bool IsOffset() const;
			bool IsReferenceToA() const;
			bool IsAngularOnly() const;
			bool IsAngularMotorEnabled() const;
			Desc& GetState();
		};

		class VI_OUT SConstraint : public Constraint
		{
			friend RigidBody;
			friend Simulator;

		public:
			struct Desc
			{
				RigidBody* TargetA = nullptr;
				RigidBody* TargetB = nullptr;
				bool Collisions = true;
				bool Linear = true;
			};

		private:
			btSliderConstraint* Instance;
			Desc State;

		private:
			SConstraint(Simulator* Refer, const Desc& I) noexcept;

		public:
			~SConstraint() noexcept override;
			Core::Unique<Constraint> Copy() const override;
			btTypedConstraint* Get() const override;
			bool HasCollisions() const override;
			void SetAngularMotorVelocity(float Value);
			void SetLinearMotorVelocity(float Value);
			void SetUpperLinearLimit(float Value);
			void SetLowerLinearLimit(float Value);
			void SetAngularDampingDirection(float Value);
			void SetLinearDampingDirection(float Value);
			void SetAngularDampingLimit(float Value);
			void SetLinearDampingLimit(float Value);
			void SetAngularDampingOrtho(float Value);
			void SetLinearDampingOrtho(float Value);
			void SetUpperAngularLimit(float Value);
			void SetLowerAngularLimit(float Value);
			void SetMaxAngularMotorForce(float Value);
			void SetMaxLinearMotorForce(float Value);
			void SetAngularRestitutionDirection(float Value);
			void SetLinearRestitutionDirection(float Value);
			void SetAngularRestitutionLimit(float Value);
			void SetLinearRestitutionLimit(float Value);
			void SetAngularRestitutionOrtho(float Value);
			void SetLinearRestitutionOrtho(float Value);
			void SetAngularSoftnessDirection(float Value);
			void SetLinearSoftnessDirection(float Value);
			void SetAngularSoftnessLimit(float Value);
			void SetLinearSoftnessLimit(float Value);
			void SetAngularSoftnessOrtho(float Value);
			void SetLinearSoftnessOrtho(float Value);
			void SetPoweredAngularMotor(bool Value);
			void SetPoweredLinearMotor(bool Value);
			float GetAngularMotorVelocity() const;
			float GetLinearMotorVelocity() const;
			float GetUpperLinearLimit() const;
			float GetLowerLinearLimit() const;
			float GetAngularDampingDirection() const;
			float GetLinearDampingDirection() const;
			float GetAngularDampingLimit() const;
			float GetLinearDampingLimit() const;
			float GetAngularDampingOrtho() const;
			float GetLinearDampingOrtho() const;
			float GetUpperAngularLimit() const;
			float GetLowerAngularLimit() const;
			float GetMaxAngularMotorForce() const;
			float GetMaxLinearMotorForce() const;
			float GetAngularRestitutionDirection() const;
			float GetLinearRestitutionDirection() const;
			float GetAngularRestitutionLimit() const;
			float GetLinearRestitutionLimit() const;
			float GetAngularRestitutionOrtho() const;
			float GetLinearRestitutionOrtho() const;
			float GetAngularSoftnessDirection() const;
			float GetLinearSoftnessDirection() const;
			float GetAngularSoftnessLimit() const;
			float GetLinearSoftnessLimit() const;
			float GetAngularSoftnessOrtho() const;
			float GetLinearSoftnessOrtho() const;
			bool GetPoweredAngularMotor() const;
			bool GetPoweredLinearMotor() const;
			Desc& GetState();
		};

		class VI_OUT CTConstraint : public Constraint
		{
			friend RigidBody;
			friend Simulator;

		public:
			struct Desc
			{
				RigidBody* TargetA = nullptr;
				RigidBody* TargetB = nullptr;
				bool Collisions = true;
			};

		private:
			btConeTwistConstraint* Instance;
			Desc State;

		private:
			CTConstraint(Simulator* Refer, const Desc& I) noexcept;

		public:
			~CTConstraint() noexcept override;
			Core::Unique<Constraint> Copy() const override;
			btTypedConstraint* Get() const override;
			bool HasCollisions() const override;
			void EnableMotor(bool Value);
			void SetFrames(const Trigonometry::Matrix4x4& A, const Trigonometry::Matrix4x4& B);
			void SetAngularOnly(bool Value);
			void SetLimit(int LimitIndex, float LimitValue);
			void SetLimit(float SwingSpan1, float SwingSpan2, float TwistSpan, float Softness = 1.f, float BiasFactor = 0.3f, float RelaxationFactor = 1.0f);
			void SetDamping(float Value);
			void SetMaxMotorImpulse(float Value);
			void SetMaxMotorImpulseNormalized(float Value);
			void SetFixThresh(float Value);
			void SetMotorTarget(const Trigonometry::Quaternion& Value);
			void SetMotorTargetInConstraintSpace(const Trigonometry::Quaternion& Value);
			Trigonometry::Vector3 GetPointForAngle(float AngleInRadians, float Length) const;
			Trigonometry::Quaternion GetMotorTarget() const;
			int GetSolveTwistLimit() const;
			int GetSolveSwingLimit() const;
			float GetTwistLimitSign() const;
			float GetSwingSpan1() const;
			float GetSwingSpan2() const;
			float GetTwistSpan() const;
			float GetLimitSoftness() const;
			float GetBiasFactor() const;
			float GetRelaxationFactor() const;
			float GetTwistAngle() const;
			float GetLimit(int Value) const;
			float GetDamping() const;
			float GetMaxMotorImpulse() const;
			float GetFixThresh() const;
			bool IsMotorEnabled() const;
			bool IsMaxMotorImpulseNormalized() const;
			bool IsPastSwingLimit() const;
			bool IsAngularOnly() const;
			Desc& GetState();
		};

		class VI_OUT DF6Constraint : public Constraint
		{
			friend RigidBody;
			friend Simulator;

		public:
			struct Desc
			{
				RigidBody* TargetA = nullptr;
				RigidBody* TargetB = nullptr;
				bool Collisions = true;
			};

		private:
			btGeneric6DofSpring2Constraint* Instance;
			Desc State;

		private:
			DF6Constraint(Simulator* Refer, const Desc& I) noexcept;

		public:
			~DF6Constraint() noexcept override;
			Core::Unique<Constraint> Copy() const override;
			btTypedConstraint* Get() const override;
			bool HasCollisions() const override;
			void EnableMotor(int Index, bool OnOff);
			void EnableSpring(int Index, bool OnOff);
			void SetFrames(const Trigonometry::Matrix4x4& A, const Trigonometry::Matrix4x4& B);
			void SetLinearLowerLimit(const Trigonometry::Vector3& Value);
			void SetLinearUpperLimit(const Trigonometry::Vector3& Value);
			void SetAngularLowerLimit(const Trigonometry::Vector3& Value);
			void SetAngularLowerLimitReversed(const Trigonometry::Vector3& Value);
			void SetAngularUpperLimit(const Trigonometry::Vector3& Value);
			void SetAngularUpperLimitReversed(const Trigonometry::Vector3& Value);
			void SetLimit(int Axis, float Low, float High);
			void SetLimitReversed(int Axis, float Low, float High);
			void SetRotationOrder(Trigonometry::Rotator Order);
			void SetAxis(const Trigonometry::Vector3& A, const Trigonometry::Vector3& B);
			void SetBounce(int Index, float Bounce);
			void SetServo(int Index, bool OnOff);
			void SetTargetVelocity(int Index, float Velocity);
			void SetServoTarget(int Index, float Target);
			void SetMaxMotorForce(int Index, float Force);
			void SetStiffness(int Index, float Stiffness, bool LimitIfNeeded = true);
			void SetEquilibriumPoint();
			void SetEquilibriumPoint(int Index);
			void SetEquilibriumPoint(int Index, float Value);
			Trigonometry::Vector3 GetAngularUpperLimit() const;
			Trigonometry::Vector3 GetAngularUpperLimitReversed() const;
			Trigonometry::Vector3 GetAngularLowerLimit() const;
			Trigonometry::Vector3 GetAngularLowerLimitReversed() const;
			Trigonometry::Vector3 GetLinearUpperLimit() const;
			Trigonometry::Vector3 GetLinearLowerLimit() const;
			Trigonometry::Vector3 GetAxis(int Value) const;
			Trigonometry::Rotator GetRotationOrder() const;
			float GetAngle(int Value) const;
			float GetRelativePivotPosition(int Value) const;
			bool IsLimited(int LimitIndex) const;
			Desc& GetState();
		};

		class VI_OUT Simulator final : public Core::Reference<Simulator>
		{
		public:
			struct Desc
			{
				Trigonometry::Vector3 WaterNormal;
				Trigonometry::Vector3 Gravity = Trigonometry::Vector3(0, -10, 0);
				float AirDensity = 1.2f;
				float WaterDensity = 0;
				float WaterOffset = 0;
				float MaxDisplacement = 1000;
				bool EnableSoftBody = false;
			};

		private:
			struct
			{
				float LastElapsedTime = 0.0f;
			} Timing;

		private:
			Core::UnorderedMap<void*, size_t> Shapes;
			btCollisionConfiguration* Collision;
			btBroadphaseInterface* Broadphase;
			btConstraintSolver* Solver;
			btDiscreteDynamicsWorld* World;
			btCollisionDispatcher* Dispatcher;
			btSoftBodySolver* SoftSolver;
			std::mutex Exclusive;

		public:
			float Speedup;
			bool Active;

		public:
			Simulator(const Desc& I) noexcept;
			~Simulator() noexcept;
			void SetGravity(const Trigonometry::Vector3& Gravity);
			void SetLinearImpulse(const Trigonometry::Vector3& Impulse, bool RandomFactor = false);
			void SetLinearImpulse(const Trigonometry::Vector3& Impulse, int Start, int End, bool RandomFactor = false);
			void SetAngularImpulse(const Trigonometry::Vector3& Impulse, bool RandomFactor = false);
			void SetAngularImpulse(const Trigonometry::Vector3& Impulse, int Start, int End, bool RandomFactor = false);
			void SetOnCollisionEnter(ContactStartedCallback Callback);
			void SetOnCollisionExit(ContactEndedCallback Callback);
			void CreateLinearImpulse(const Trigonometry::Vector3& Impulse, bool RandomFactor = false);
			void CreateLinearImpulse(const Trigonometry::Vector3& Impulse, int Start, int End, bool RandomFactor = false);
			void CreateAngularImpulse(const Trigonometry::Vector3& Impulse, bool RandomFactor = false);
			void CreateAngularImpulse(const Trigonometry::Vector3& Impulse, int Start, int End, bool RandomFactor = false);
			void AddSoftBody(SoftBody* Body);
			void RemoveSoftBody(SoftBody* Body);
			void AddRigidBody(RigidBody* Body);
			void RemoveRigidBody(RigidBody* Body);
			void AddConstraint(Constraint* Constraint);
			void RemoveConstraint(Constraint* Constraint);
			void RemoveAll();
			void SimulateStep(float ElapsedTimeSeconds);
			void FindContacts(RigidBody* Body, int(*Callback)(ShapeContact*, const CollisionBody&, const CollisionBody&));
			bool FindRayContacts(const Trigonometry::Vector3& Start, const Trigonometry::Vector3& End, int(*Callback)(RayContact*, const CollisionBody&));
			Core::Unique<RigidBody> CreateRigidBody(const RigidBody::Desc& I);
			Core::Unique<RigidBody> CreateRigidBody(const RigidBody::Desc& I, Trigonometry::Transform* Transform);
			Core::Unique<SoftBody> CreateSoftBody(const SoftBody::Desc& I);
			Core::Unique<SoftBody> CreateSoftBody(const SoftBody::Desc& I, Trigonometry::Transform* Transform);
			Core::Unique<PConstraint> CreatePoint2PointConstraint(const PConstraint::Desc& I);
			Core::Unique<HConstraint> CreateHingeConstraint(const HConstraint::Desc& I);
			Core::Unique<SConstraint> CreateSliderConstraint(const SConstraint::Desc& I);
			Core::Unique<CTConstraint> CreateConeTwistConstraint(const CTConstraint::Desc& I);
			Core::Unique<DF6Constraint> Create6DoFConstraint(const DF6Constraint::Desc& I);
			btCollisionShape* CreateShape(Shape Type);
			btCollisionShape* CreateCube(const Trigonometry::Vector3& Scale = Trigonometry::Vector3(1, 1, 1));
			btCollisionShape* CreateSphere(float Radius = 1);
			btCollisionShape* CreateCapsule(float Radius = 1, float Height = 1);
			btCollisionShape* CreateCone(float Radius = 1, float Height = 1);
			btCollisionShape* CreateCylinder(const Trigonometry::Vector3& Scale = Trigonometry::Vector3(1, 1, 1));
			btCollisionShape* CreateConvexHull(Core::Vector<Trigonometry::SkinVertex>& Mesh);
			btCollisionShape* CreateConvexHull(Core::Vector<Trigonometry::Vertex>& Mesh);
			btCollisionShape* CreateConvexHull(Core::Vector<Trigonometry::Vector2>& Mesh);
			btCollisionShape* CreateConvexHull(Core::Vector<Trigonometry::Vector3>& Mesh);
			btCollisionShape* CreateConvexHull(Core::Vector<Trigonometry::Vector4>& Mesh);
			btCollisionShape* CreateConvexHull(btCollisionShape* From);
			btCollisionShape* TryCloneShape(btCollisionShape* Shape);
			btCollisionShape* ReuseShape(btCollisionShape* Shape);
			void FreeShape(Core::Unique<btCollisionShape*> Value);
			Core::Vector<Trigonometry::Vector3> GetShapeVertices(btCollisionShape* Shape) const;
			size_t GetShapeVerticesCount(btCollisionShape* Shape) const;
			float GetMaxDisplacement() const;
			float GetAirDensity() const;
			float GetWaterOffset() const;
			float GetWaterDensity() const;
			Trigonometry::Vector3 GetWaterNormal() const;
			Trigonometry::Vector3 GetGravity() const;
			ContactStartedCallback GetOnCollisionEnter() const;
			ContactEndedCallback GetOnCollisionExit() const;
			btCollisionConfiguration* GetCollision() const;
			btBroadphaseInterface* GetBroadphase() const;
			btConstraintSolver* GetSolver() const;
			btDiscreteDynamicsWorld* GetWorld() const;
			btCollisionDispatcher* GetDispatcher() const;
			btSoftBodySolver* GetSoftSolver() const;
			bool HasSoftBodySupport() const;
			int GetContactManifoldCount() const;

		public:
			static Simulator* Get(btDiscreteDynamicsWorld* From);
		};
	}
}
#endif