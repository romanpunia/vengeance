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

namespace Vitex
{
	namespace Layer
	{
		namespace Components
		{
			static float GetVisibilityRadius(Entity* Base, const Viewer& View, float Distance)
			{
				float Visibility = 1.0f - Distance / View.FarPlane;
				if (Visibility <= 0.0f)
					return 0.0f;

				const Trigonometry::Matrix4x4& Box = Base->GetTransform()->GetBiasUnscaled();
				return Trigonometry::Geometric::IsCubeInFrustum(Box * View.ViewProjection, Base->GetRadius()) ? Visibility : 0.0f;
			}

			RigidBody::RigidBody(Entity* Ref) : Component(Ref, ActorSet::Synchronize)
			{
			}
			RigidBody::~RigidBody()
			{
				Clear();
			}
			void RigidBody::DeserializeBody(Core::Schema* Node)
			{
				size_t ActivationState;
				if (Series::Unpack(Node->Find("activation-state"), &ActivationState))
					Instance->SetActivationState((Physics::MotionState)ActivationState);

				float AngularDamping;
				if (Series::Unpack(Node->Find("angular-damping"), &AngularDamping))
					Instance->SetAngularDamping(AngularDamping);

				float AngularSleepingThreshold;
				if (Series::Unpack(Node->Find("angular-sleeping-threshold"), &AngularSleepingThreshold))
					Instance->SetAngularSleepingThreshold(AngularSleepingThreshold);

				float Friction;
				if (Series::Unpack(Node->Find("friction"), &Friction))
					Instance->SetFriction(Friction);

				float Restitution;
				if (Series::Unpack(Node->Find("restitution"), &Restitution))
					Instance->SetRestitution(Restitution);

				float HitFraction;
				if (Series::Unpack(Node->Find("hit-fraction"), &HitFraction))
					Instance->SetHitFraction(HitFraction);

				float LinearDamping;
				if (Series::Unpack(Node->Find("linear-damping"), &LinearDamping))
					Instance->SetLinearDamping(LinearDamping);

				float LinearSleepingThreshold;
				if (Series::Unpack(Node->Find("linear-sleeping-threshold"), &LinearSleepingThreshold))
					Instance->SetLinearSleepingThreshold(LinearSleepingThreshold);

				float CcdSweptSphereRadius;
				if (Series::Unpack(Node->Find("ccd-swept-sphere-radius"), &CcdSweptSphereRadius))
					Instance->SetCcdSweptSphereRadius(CcdSweptSphereRadius);

				float ContactProcessingThreshold;
				if (Series::Unpack(Node->Find("contact-processing-threshold"), &ContactProcessingThreshold))
					Instance->SetContactProcessingThreshold(ContactProcessingThreshold);

				float DeactivationTime;
				if (Series::Unpack(Node->Find("deactivation-time"), &DeactivationTime))
					Instance->SetDeactivationTime(DeactivationTime);

				float RollingFriction;
				if (Series::Unpack(Node->Find("rolling-friction"), &RollingFriction))
					Instance->SetRollingFriction(RollingFriction);

				float SpinningFriction;
				if (Series::Unpack(Node->Find("spinning-friction"), &SpinningFriction))
					Instance->SetSpinningFriction(SpinningFriction);

				float ContactStiffness;
				if (Series::Unpack(Node->Find("contact-stiffness"), &ContactStiffness))
					Instance->SetContactStiffness(ContactStiffness);

				float ContactDamping;
				if (Series::Unpack(Node->Find("contact-damping"), &ContactDamping))
					Instance->SetContactDamping(ContactDamping);

				Trigonometry::Vector3 AngularFactor;
				if (HeavySeries::Unpack(Node->Find("angular-factor"), &AngularFactor))
					Instance->SetAngularFactor(AngularFactor);

				Trigonometry::Vector3 AngularVelocity;
				if (HeavySeries::Unpack(Node->Find("angular-velocity"), &AngularVelocity))
					Instance->SetAngularVelocity(AngularVelocity);

				Trigonometry::Vector3 AnisotropicFriction;
				if (HeavySeries::Unpack(Node->Find("anisotropic-friction"), &AnisotropicFriction))
					Instance->SetAnisotropicFriction(AnisotropicFriction);

				Trigonometry::Vector3 Gravity;
				if (HeavySeries::Unpack(Node->Find("gravity"), &Gravity))
					Instance->SetGravity(Gravity);

				Trigonometry::Vector3 LinearFactor;
				if (HeavySeries::Unpack(Node->Find("linear-factor"), &LinearFactor))
					Instance->SetLinearFactor(LinearFactor);

				Trigonometry::Vector3 LinearVelocity;
				if (HeavySeries::Unpack(Node->Find("linear-velocity"), &LinearVelocity))
					Instance->SetLinearVelocity(LinearVelocity);

				size_t CollisionFlags;
				if (Series::Unpack(Node->Find("collision-flags"), &CollisionFlags))
					Instance->SetCollisionFlags(CollisionFlags);
			}
			void RigidBody::Deserialize(Core::Schema* Node)
			{
				VI_ASSERT(Node != nullptr, "schema should be set");

				Core::Schema* Shaping = Node->Find("shape");
				Core::Vector<Trigonometry::Vector3> Vertices;
				Core::String Path; size_t Type;
				float Mass = 0.0f, Anticipation = 0.0f;
				bool Extended = false;

				Series::Unpack(Node->Find("mass"), &Mass);
				Series::Unpack(Node->Find("ccd-motion-threshold"), &Anticipation);
				Series::Unpack(Node->Find("kinematic"), &Kinematic);
				Series::Unpack(Node->Find("manage"), &Manage);
				Series::Unpack(Node->Find("extended"), &Extended);

				if (!Extended)
					return;

				if (Shaping != nullptr)
				{
					if (Series::Unpack(Shaping->Find("path"), &Path))
					{
						Node->AddRef();
						return Load(Path, Mass, Anticipation, [this, Node]()
						{
							if (Instance != nullptr)
								DeserializeBody(Node);
							Node->Release();
						});
					}
					else if (Series::Unpack(Shaping->Find("type"), &Type))
					{
						btCollisionShape* Shape = Parent->GetScene()->GetSimulator()->CreateShape((Physics::Shape)Type);
						if (Shape != nullptr)
							Load(Shape, Mass, Anticipation);
					}
					else if (HeavySeries::Unpack(Shaping->Find("data"), &Vertices))
					{
						btCollisionShape* Shape = Parent->GetScene()->GetSimulator()->CreateConvexHull(Vertices);
						if (Shape != nullptr)
							Load(Shape, Mass, Anticipation);
					}
				}

				if (Instance != nullptr)
					DeserializeBody(Node);
			}
			void RigidBody::Serialize(Core::Schema* Node)
			{
				VI_ASSERT(Node != nullptr, "schema should be set");

				SceneGraph* Scene = Parent->GetScene();
				Series::Pack(Node->Set("kinematic"), Kinematic);
				Series::Pack(Node->Set("manage"), Manage);
				Series::Pack(Node->Set("extended"), Instance != nullptr);

				if (!Instance)
					return;

				Core::Schema* Shaping = Node->Set("shape");
				if (Instance->GetCollisionShapeType() == Physics::Shape::Convex_Hull)
				{
					Core::String Path = Scene->FindResourceId<Physics::HullShape>(Hull);
					if (Path.empty())
					{
						Core::Vector<Trigonometry::Vector3> Vertices = Scene->GetSimulator()->GetShapeVertices(Instance->GetCollisionShape());
						HeavySeries::Pack(Shaping->Set("data"), Vertices);
					}
					else
						Series::Pack(Shaping->Set("path"), Path);
				}
				else
					Series::Pack(Shaping->Set("type"), (size_t)Instance->GetCollisionShapeType());

				Series::Pack(Node->Set("mass"), Instance->GetMass());
				Series::Pack(Node->Set("ccd-motion-threshold"), Instance->GetCcdMotionThreshold());
				Series::Pack(Node->Set("activation-state"), (size_t)Instance->GetActivationState());
				Series::Pack(Node->Set("angular-damping"), Instance->GetAngularDamping());
				Series::Pack(Node->Set("angular-sleeping-threshold"), Instance->GetAngularSleepingThreshold());
				Series::Pack(Node->Set("friction"), Instance->GetFriction());
				Series::Pack(Node->Set("restitution"), Instance->GetRestitution());
				Series::Pack(Node->Set("hit-fraction"), Instance->GetHitFraction());
				Series::Pack(Node->Set("linear-damping"), Instance->GetLinearDamping());
				Series::Pack(Node->Set("linear-sleeping-threshold"), Instance->GetLinearSleepingThreshold());
				Series::Pack(Node->Set("ccd-swept-sphere-radius"), Instance->GetCcdSweptSphereRadius());
				Series::Pack(Node->Set("contact-processing-threshold"), Instance->GetContactProcessingThreshold());
				Series::Pack(Node->Set("deactivation-time"), Instance->GetDeactivationTime());
				Series::Pack(Node->Set("rolling-friction"), Instance->GetRollingFriction());
				Series::Pack(Node->Set("spinning-friction"), Instance->GetSpinningFriction());
				Series::Pack(Node->Set("contact-stiffness"), Instance->GetContactStiffness());
				Series::Pack(Node->Set("contact-damping"), Instance->GetContactDamping());
				HeavySeries::Pack(Node->Set("angular-factor"), Instance->GetAngularFactor());
				HeavySeries::Pack(Node->Set("angular-velocity"), Instance->GetAngularVelocity());
				HeavySeries::Pack(Node->Set("anisotropic-friction"), Instance->GetAnisotropicFriction());
				HeavySeries::Pack(Node->Set("gravity"), Instance->GetGravity());
				HeavySeries::Pack(Node->Set("linear-factor"), Instance->GetLinearFactor());
				HeavySeries::Pack(Node->Set("linear-velocity"), Instance->GetLinearVelocity());
				Series::Pack(Node->Set("collision-flags"), Instance->GetCollisionFlags());
			}
			void RigidBody::Synchronize(Core::Timer* Time)
			{
				if (Instance && Manage)
					Instance->Synchronize(Parent->GetTransform(), Kinematic);
			}
			void RigidBody::Deactivate()
			{
				if (Instance != nullptr)
					Instance->SetAsGhost();
			}
			void RigidBody::Load(btCollisionShape* Shape, float Mass, float Anticipation)
			{
				SceneGraph* Scene = Parent->GetScene();
				VI_ASSERT(Scene != nullptr, "scene should be set");
				VI_ASSERT(Shape != nullptr, "collision shape should be set");

				Physics::RigidBody::Desc I;
				I.Anticipation = Anticipation;
				I.Mass = Mass;
				I.Shape = Shape;

				Core::Memory::Release(Instance);
				Instance = Scene->GetSimulator()->CreateRigidBody(I, Parent->GetTransform());
				Instance->UserPointer = this;
				Instance->SetActivity(true);

				GetEntity()->GetTransform()->MakeDirty();
			}
			void RigidBody::Load(const std::string_view& Path, float Mass, float Anticipation, std::function<void()>&& Callback)
			{
				Parent->GetScene()->LoadResource<Physics::HullShape>(this, Path, [this, Mass, Anticipation, Callback = std::move(Callback)](ExpectsContent<Physics::HullShape*> NewHull)
				{
					Core::Memory::Release(Hull);
					Hull = NewHull.Or(nullptr);
					if (Hull != nullptr)
						Load(Hull->GetShape(), Mass, Anticipation);
					else
						Clear();

					if (Callback)
						Callback();
				});
			}
			void RigidBody::Clear()
			{
				Core::Memory::Release(Instance);
				Core::Memory::Release(Hull);
			}
			void RigidBody::SetTransform(const Trigonometry::Vector3& Position, const Trigonometry::Vector3& Scale, const Trigonometry::Vector3& Rotation)
			{
				if (!Instance)
					return;

				Trigonometry::Transform::Spacing Space;
				Space.Position = Position;
				Space.Rotation = Rotation;
				Space.Scale = Scale;

				auto* Transform = Parent->GetTransform();
				Transform->SetSpacing(Trigonometry::Positioning::Global, Space);
				Instance->Synchronize(Transform, true);
				Instance->SetActivity(true);
			}
			void RigidBody::SetTransform(bool Kinematics)
			{
				if (!Instance)
					return;

				auto* Transform = Parent->GetTransform();
				Instance->Synchronize(Transform, Kinematics);
				Instance->SetActivity(true);
			}
			void RigidBody::SetMass(float Mass)
			{
				if (Instance != nullptr)
					Instance->SetMass(Mass);
			}
			Component* RigidBody::Copy(Entity* New) const
			{
				RigidBody* Target = new RigidBody(New);
				Target->Kinematic = Kinematic;

				if (Instance != nullptr)
				{
					Target->Instance = Instance->Copy();
					Target->Instance->UserPointer = Target;
				}

				return Target;
			}
			Physics::RigidBody* RigidBody::GetBody() const
			{
				return Instance;
			}

			SoftBody::SoftBody(Entity* Ref) : Drawable(Ref, ActorSet::Synchronize, SoftBody::GetTypeId())
			{
			}
			SoftBody::~SoftBody()
			{
				Clear();
			}
			void SoftBody::DeserializeBody(Core::Schema* Node)
			{
				Core::Schema* Conf = Node->Get("config");
				if (Conf != nullptr)
				{
					Physics::SoftBody::Desc::SConfig I;
					Series::Unpack(Conf->Get("vcf"), &I.VCF);
					Series::Unpack(Conf->Get("dp"), &I.DP);
					Series::Unpack(Conf->Get("dg"), &I.DG);
					Series::Unpack(Conf->Get("lf"), &I.LF);
					Series::Unpack(Conf->Get("pr"), &I.PR);
					Series::Unpack(Conf->Get("vc"), &I.VC);
					Series::Unpack(Conf->Get("df"), &I.DF);
					Series::Unpack(Conf->Get("mt"), &I.MT);
					Series::Unpack(Conf->Get("chr"), &I.CHR);
					Series::Unpack(Conf->Get("khr"), &I.KHR);
					Series::Unpack(Conf->Get("shr"), &I.SHR);
					Series::Unpack(Conf->Get("ahr"), &I.AHR);
					Series::Unpack(Conf->Get("srhr-cl"), &I.SRHR_CL);
					Series::Unpack(Conf->Get("skhr-cl"), &I.SKHR_CL);
					Series::Unpack(Conf->Get("sshr-cl"), &I.SSHR_CL);
					Series::Unpack(Conf->Get("sr-splt-cl"), &I.SR_SPLT_CL);
					Series::Unpack(Conf->Get("sk-splt-cl"), &I.SK_SPLT_CL);
					Series::Unpack(Conf->Get("ss-splt-cl"), &I.SS_SPLT_CL);
					Series::Unpack(Conf->Get("max-volume"), &I.MaxVolume);
					Series::Unpack(Conf->Get("time-scale"), &I.TimeScale);
					Series::Unpack(Conf->Get("drag"), &I.Drag);
					Series::Unpack(Conf->Get("max-stress"), &I.MaxStress);
					Series::Unpack(Conf->Get("constraints"), &I.Constraints);
					Series::Unpack(Conf->Get("clusters"), &I.Clusters);
					Series::Unpack(Conf->Get("v-it"), &I.VIterations);
					Series::Unpack(Conf->Get("p-it"), &I.PIterations);
					Series::Unpack(Conf->Get("d-it"), &I.DIterations);
					Series::Unpack(Conf->Get("c-it"), &I.CIterations);
					Series::Unpack(Conf->Get("collisions"), &I.Collisions);

					size_t AeroModel;
					if (Series::Unpack(Conf->Get("aero-model"), &AeroModel))
						I.AeroModel = (Physics::SoftAeroModel)AeroModel;

					Instance->SetConfig(I);
				}

				size_t ActivationState;
				if (Series::Unpack(Node->Find("activation-state"), &ActivationState))
					Instance->SetActivationState((Physics::MotionState)ActivationState);

				float Friction;
				if (Series::Unpack(Node->Find("friction"), &Friction))
					Instance->SetFriction(Friction);

				float Restitution;
				if (Series::Unpack(Node->Find("restitution"), &Restitution))
					Instance->SetRestitution(Restitution);

				float HitFraction;
				if (Series::Unpack(Node->Find("hit-fraction"), &HitFraction))
					Instance->SetHitFraction(HitFraction);

				float CcdSweptSphereRadius;
				if (Series::Unpack(Node->Find("ccd-swept-sphere-radius"), &CcdSweptSphereRadius))
					Instance->SetCcdSweptSphereRadius(CcdSweptSphereRadius);

				float ContactProcessingThreshold;
				if (Series::Unpack(Node->Find("contact-processing-threshold"), &ContactProcessingThreshold))
					Instance->SetContactProcessingThreshold(ContactProcessingThreshold);

				float DeactivationTime;
				if (Series::Unpack(Node->Find("deactivation-time"), &DeactivationTime))
					Instance->SetDeactivationTime(DeactivationTime);

				float RollingFriction;
				if (Series::Unpack(Node->Find("rolling-friction"), &RollingFriction))
					Instance->SetRollingFriction(RollingFriction);

				float SpinningFriction;
				if (Series::Unpack(Node->Find("spinning-friction"), &SpinningFriction))
					Instance->SetSpinningFriction(SpinningFriction);

				float ContactStiffness;
				if (Series::Unpack(Node->Find("contact-stiffness"), &ContactStiffness))
					Instance->SetContactStiffness(ContactStiffness);

				float ContactDamping;
				if (Series::Unpack(Node->Find("contact-damping"), &ContactDamping))
					Instance->SetContactDamping(ContactDamping);

				Trigonometry::Vector3 AnisotropicFriction;
				if (HeavySeries::Unpack(Node->Find("anisotropic-friction"), &AnisotropicFriction))
					Instance->SetAnisotropicFriction(AnisotropicFriction);

				Trigonometry::Vector3 WindVelocity;
				if (HeavySeries::Unpack(Node->Find("wind-velocity"), &WindVelocity))
					Instance->SetWindVelocity(WindVelocity);

				float TotalMass;
				if (Series::Unpack(Node->Find("total-mass"), &TotalMass))
					Instance->SetTotalMass(TotalMass);

				float RestLengthScale;
				if (Series::Unpack(Node->Find("core-length-scale"), &RestLengthScale))
					Instance->SetRestLengthScale(RestLengthScale);
			}
			void SoftBody::Deserialize(Core::Schema* Node)
			{
				VI_ASSERT(Node != nullptr, "schema should be set");

				float Anticipation = 0.0f; bool Extended = false;
				uint32_t NewCategory = (uint32_t)GeoCategory::Opaque;
				Series::Unpack(Node->Find("ccd-motion-threshold"), &Anticipation);
				HeavySeries::Unpack(Node->Find("texcoord"), &TexCoord);
				Series::Unpack(Node->Find("extended"), &Extended);
				Series::Unpack(Node->Find("kinematic"), &Kinematic);
				Series::Unpack(Node->Find("manage"), &Manage);
				Series::Unpack(Node->Find("static"), &Static);
				Series::Unpack(Node->Find("category"), &NewCategory);
				SetCategory((GeoCategory)NewCategory);

				size_t Slot = 0;
				if (Series::Unpack(Node->Find("material"), &Slot))
					SetMaterial(nullptr, Parent->GetScene()->GetMaterial(Slot));

				if (!Extended)
					return;

				Core::Schema* Shaping = nullptr;
				if ((Shaping = Node->Find("shape")) != nullptr)
				{
					Core::String Path;
					if (Series::Unpack(Shaping->Find("path"), &Path))
					{
						Node->AddRef();
						return Load(Path, Anticipation, [this, Node]()
						{
							if (Instance != nullptr)
								DeserializeBody(Node);
							Node->Release();
						});
					}
				}
				else if ((Shaping = Node->Find("ellipsoid")) != nullptr)
				{
					Physics::SoftBody::Desc::CV::SEllipsoid Shape;
					HeavySeries::Unpack(Shaping->Get("center"), &Shape.Center);
					HeavySeries::Unpack(Shaping->Get("radius"), &Shape.Radius);
					Series::Unpack(Shaping->Get("count"), &Shape.Count);
					LoadEllipsoid(Shape, Anticipation);
				}
				else if ((Shaping = Node->Find("patch")) != nullptr)
				{
					Physics::SoftBody::Desc::CV::SPatch Shape;
					HeavySeries::Unpack(Shaping->Get("corner-00"), &Shape.Corner00);
					Series::Unpack(Shaping->Get("corner-00-fixed"), &Shape.Corner00Fixed);
					HeavySeries::Unpack(Shaping->Get("corner-01"), &Shape.Corner01);
					Series::Unpack(Shaping->Get("corner-01-fixed"), &Shape.Corner01Fixed);
					HeavySeries::Unpack(Shaping->Get("corner-10"), &Shape.Corner10);
					Series::Unpack(Shaping->Get("corner-10-fixed"), &Shape.Corner10Fixed);
					HeavySeries::Unpack(Shaping->Get("corner-11"), &Shape.Corner11);
					Series::Unpack(Shaping->Get("corner-11-fixed"), &Shape.Corner11Fixed);
					Series::Unpack(Shaping->Get("count-x"), &Shape.CountX);
					Series::Unpack(Shaping->Get("count-y"), &Shape.CountY);
					Series::Unpack(Shaping->Get("diagonals"), &Shape.GenerateDiagonals);
					LoadPatch(Shape, Anticipation);
				}
				else if ((Shaping = Node->Find("rope")) != nullptr)
				{
					Physics::SoftBody::Desc::CV::SRope Shape;
					HeavySeries::Unpack(Shaping->Get("start"), &Shape.Start);
					Series::Unpack(Shaping->Get("start-fixed"), &Shape.StartFixed);
					HeavySeries::Unpack(Shaping->Get("end"), &Shape.End);
					Series::Unpack(Shaping->Get("end-fixed"), &Shape.EndFixed);
					Series::Unpack(Shaping->Get("count"), &Shape.Count);
					LoadRope(Shape, Anticipation);
				}

				if (Instance != nullptr)
					DeserializeBody(Node);
			}
			void SoftBody::Serialize(Core::Schema* Node)
			{
				VI_ASSERT(Node != nullptr, "schema should be set");

				Material* Slot = GetMaterial();
				if (Slot != nullptr)
					Series::Pack(Node->Set("material"), Slot->Slot);

				HeavySeries::Pack(Node->Set("texcoord"), TexCoord);
				Series::Pack(Node->Set("category"), (uint32_t)GetCategory());
				Series::Pack(Node->Set("kinematic"), Kinematic);
				Series::Pack(Node->Set("manage"), Manage);
				Series::Pack(Node->Set("extended"), Instance != nullptr);
				Series::Pack(Node->Set("static"), Static);

				if (!Instance)
					return;

				Physics::SoftBody::Desc& I = Instance->GetInitialState();
				Core::Schema* Conf = Node->Set("config");
				Series::Pack(Conf->Set("aero-model"), (size_t)I.Config.AeroModel);
				Series::Pack(Conf->Set("vcf"), I.Config.VCF);
				Series::Pack(Conf->Set("dp"), I.Config.DP);
				Series::Pack(Conf->Set("dg"), I.Config.DG);
				Series::Pack(Conf->Set("lf"), I.Config.LF);
				Series::Pack(Conf->Set("pr"), I.Config.PR);
				Series::Pack(Conf->Set("vc"), I.Config.VC);
				Series::Pack(Conf->Set("df"), I.Config.DF);
				Series::Pack(Conf->Set("mt"), I.Config.MT);
				Series::Pack(Conf->Set("chr"), I.Config.CHR);
				Series::Pack(Conf->Set("khr"), I.Config.KHR);
				Series::Pack(Conf->Set("shr"), I.Config.SHR);
				Series::Pack(Conf->Set("ahr"), I.Config.AHR);
				Series::Pack(Conf->Set("srhr-cl"), I.Config.SRHR_CL);
				Series::Pack(Conf->Set("skhr-cl"), I.Config.SKHR_CL);
				Series::Pack(Conf->Set("sshr-cl"), I.Config.SSHR_CL);
				Series::Pack(Conf->Set("sr-splt-cl"), I.Config.SR_SPLT_CL);
				Series::Pack(Conf->Set("sk-splt-cl"), I.Config.SK_SPLT_CL);
				Series::Pack(Conf->Set("ss-splt-cl"), I.Config.SS_SPLT_CL);
				Series::Pack(Conf->Set("max-volume"), I.Config.MaxVolume);
				Series::Pack(Conf->Set("time-scale"), I.Config.TimeScale);
				Series::Pack(Conf->Set("drag"), I.Config.Drag);
				Series::Pack(Conf->Set("max-stress"), I.Config.MaxStress);
				Series::Pack(Conf->Set("constraints"), I.Config.Constraints);
				Series::Pack(Conf->Set("clusters"), I.Config.Clusters);
				Series::Pack(Conf->Set("v-it"), I.Config.VIterations);
				Series::Pack(Conf->Set("p-it"), I.Config.PIterations);
				Series::Pack(Conf->Set("d-it"), I.Config.DIterations);
				Series::Pack(Conf->Set("c-it"), I.Config.CIterations);
				Series::Pack(Conf->Set("collisions"), I.Config.Collisions);

				auto& Desc = Instance->GetInitialState();
				if (Desc.Shape.Convex.Enabled && Hull != nullptr)
				{
					if (Instance->GetCollisionShapeType() == Physics::Shape::Convex_Hull)
					{
						Core::String Path = Parent->GetScene()->FindResourceId<Physics::HullShape>(Hull);
						if (!Path.empty())
							Series::Pack(Node->Set("shape")->Set("path"), Path);
					}
				}
				else if (Desc.Shape.Ellipsoid.Enabled)
				{
					Core::Schema* Shape = Node->Set("ellipsoid");
					HeavySeries::Pack(Shape->Set("center"), Desc.Shape.Ellipsoid.Center);
					HeavySeries::Pack(Shape->Set("radius"), Desc.Shape.Ellipsoid.Radius);
					Series::Pack(Shape->Set("count"), Desc.Shape.Ellipsoid.Count);
				}
				else if (Desc.Shape.Patch.Enabled)
				{
					Core::Schema* Shape = Node->Set("patch");
					HeavySeries::Pack(Shape->Set("corner-00"), Desc.Shape.Patch.Corner00);
					Series::Pack(Shape->Set("corner-00-fixed"), Desc.Shape.Patch.Corner00Fixed);
					HeavySeries::Pack(Shape->Set("corner-01"), Desc.Shape.Patch.Corner01);
					Series::Pack(Shape->Set("corner-01-fixed"), Desc.Shape.Patch.Corner01Fixed);
					HeavySeries::Pack(Shape->Set("corner-10"), Desc.Shape.Patch.Corner10);
					Series::Pack(Shape->Set("corner-10-fixed"), Desc.Shape.Patch.Corner10Fixed);
					HeavySeries::Pack(Shape->Set("corner-11"), Desc.Shape.Patch.Corner11);
					Series::Pack(Shape->Set("corner-11-fixed"), Desc.Shape.Patch.Corner11Fixed);
					Series::Pack(Shape->Set("count-x"), Desc.Shape.Patch.CountX);
					Series::Pack(Shape->Set("count-y"), Desc.Shape.Patch.CountY);
					Series::Pack(Shape->Set("diagonals"), Desc.Shape.Patch.GenerateDiagonals);
				}
				else if (Desc.Shape.Rope.Enabled)
				{
					Core::Schema* Shape = Node->Set("rope");
					HeavySeries::Pack(Shape->Set("start"), Desc.Shape.Rope.Start);
					Series::Pack(Shape->Set("start-fixed"), Desc.Shape.Rope.StartFixed);
					HeavySeries::Pack(Shape->Set("end"), Desc.Shape.Rope.End);
					Series::Pack(Shape->Set("end-fixed"), Desc.Shape.Rope.EndFixed);
					Series::Pack(Shape->Set("count"), Desc.Shape.Rope.Count);
				}

				Series::Pack(Node->Set("ccd-motion-threshold"), Instance->GetCcdMotionThreshold());
				Series::Pack(Node->Set("activation-state"), (size_t)Instance->GetActivationState());
				Series::Pack(Node->Set("friction"), Instance->GetFriction());
				Series::Pack(Node->Set("restitution"), Instance->GetRestitution());
				Series::Pack(Node->Set("hit-fraction"), Instance->GetHitFraction());
				Series::Pack(Node->Set("ccd-swept-sphere-radius"), Instance->GetCcdSweptSphereRadius());
				Series::Pack(Node->Set("contact-processing-threshold"), Instance->GetContactProcessingThreshold());
				Series::Pack(Node->Set("deactivation-time"), Instance->GetDeactivationTime());
				Series::Pack(Node->Set("rolling-friction"), Instance->GetRollingFriction());
				Series::Pack(Node->Set("spinning-friction"), Instance->GetSpinningFriction());
				Series::Pack(Node->Set("contact-stiffness"), Instance->GetContactStiffness());
				Series::Pack(Node->Set("contact-damping"), Instance->GetContactDamping());
				HeavySeries::Pack(Node->Set("angular-velocity"), Instance->GetAngularVelocity());
				HeavySeries::Pack(Node->Set("anisotropic-friction"), Instance->GetAnisotropicFriction());
				HeavySeries::Pack(Node->Set("linear-velocity"), Instance->GetLinearVelocity());
				Series::Pack(Node->Set("collision-flags"), Instance->GetCollisionFlags());
				HeavySeries::Pack(Node->Set("wind-velocity"), Instance->GetWindVelocity());
				Series::Pack(Node->Set("total-mass"), Instance->GetTotalMass());
				Series::Pack(Node->Set("core-length-scale"), Instance->GetRestLengthScale());
			}
			void SoftBody::Synchronize(Core::Timer* Time)
			{
				if (!Instance)
					return;

				if (Manage)
				{
					auto* Transform = Parent->GetTransform();
					Instance->Synchronize(Transform, Kinematic);
					if (Instance->IsActive())
						Transform->MakeDirty();
				}

				Instance->GetVertices(&Vertices);
				if (Indices.empty())
					Instance->GetIndices(&Indices);
			}
			void SoftBody::Deactivate()
			{
				if (Instance != nullptr)
					Instance->SetAsGhost();
			}
			void SoftBody::Load(Physics::HullShape* Shape, float Anticipation)
			{
				SceneGraph* Scene = Parent->GetScene();
				VI_ASSERT(Scene != nullptr, "scene should be set");
				VI_ASSERT(Shape != nullptr, "collision shape should be set");
				Core::Memory::Release(Hull);
				Hull = Shape;

				Physics::SoftBody::Desc I;
				I.Anticipation = Anticipation;
				I.Shape.Convex.Hull = Hull;
				I.Shape.Convex.Enabled = true;

				Core::Memory::Release(Instance);
				Instance = Scene->GetSimulator()->CreateSoftBody(I, Parent->GetTransform());
				VI_PANIC(!Instance, "invalid simulator configuration to create soft bodies");

				Vertices.clear();
				Indices.clear();

				Instance->UserPointer = this;
				Instance->SetActivity(true);
				GetEntity()->GetTransform()->MakeDirty();
			}
			void SoftBody::Load(const std::string_view& Path, float Anticipation, std::function<void()>&& Callback)
			{
				Parent->GetScene()->LoadResource<Physics::HullShape>(this, Path, [this, Anticipation, Callback = std::move(Callback)](ExpectsContent<Physics::HullShape*> NewHull)
				{
					if (NewHull && *NewHull != nullptr)
						Load(*NewHull, Anticipation);
					else
						Clear();

					if (Callback)
						Callback();
				});
			}
			void SoftBody::LoadEllipsoid(const Physics::SoftBody::Desc::CV::SEllipsoid& Shape, float Anticipation)
			{
				SceneGraph* Scene = Parent->GetScene();
				VI_ASSERT(Scene != nullptr, "scene should be set");

				Physics::SoftBody::Desc I;
				I.Anticipation = Anticipation;
				I.Shape.Ellipsoid = Shape;
				I.Shape.Ellipsoid.Enabled = true;

				Core::Memory::Release(Instance);
				Instance = Scene->GetSimulator()->CreateSoftBody(I, Parent->GetTransform());
				VI_PANIC(!Instance, "invalid simulator configuration to create soft bodies");

				Vertices.clear();
				Indices.clear();

				Instance->UserPointer = this;
				Instance->SetActivity(true);
			}
			void SoftBody::LoadPatch(const Physics::SoftBody::Desc::CV::SPatch& Shape, float Anticipation)
			{
				SceneGraph* Scene = Parent->GetScene();
				VI_ASSERT(Scene != nullptr, "scene should be set");

				Physics::SoftBody::Desc I;
				I.Anticipation = Anticipation;
				I.Shape.Patch = Shape;
				I.Shape.Patch.Enabled = true;

				Core::Memory::Release(Instance);
				Instance = Scene->GetSimulator()->CreateSoftBody(I, Parent->GetTransform());
				VI_PANIC(!Instance, "invalid simulator configuration to create soft bodies");

				Vertices.clear();
				Indices.clear();

				Instance->UserPointer = this;
				Instance->SetActivity(true);
			}
			void SoftBody::LoadRope(const Physics::SoftBody::Desc::CV::SRope& Shape, float Anticipation)
			{
				SceneGraph* Scene = Parent->GetScene();
				VI_ASSERT(Scene != nullptr, "scene should be set");

				Physics::SoftBody::Desc I;
				I.Anticipation = Anticipation;
				I.Shape.Rope = Shape;
				I.Shape.Rope.Enabled = true;

				Core::Memory::Release(Instance);
				Instance = Scene->GetSimulator()->CreateSoftBody(I, Parent->GetTransform());
				VI_PANIC(!Instance, "invalid simulator configuration to create soft bodies");

				Vertices.clear();
				Indices.clear();

				Instance->UserPointer = this;
				Instance->SetActivity(true);
			}
			void SoftBody::Fill(Graphics::GraphicsDevice* Device, Graphics::ElementBuffer* IndexBuffer, Graphics::ElementBuffer* VertexBuffer)
			{
				Graphics::MappedSubresource Map;
				if (VertexBuffer != nullptr && !Vertices.empty())
				{
					Device->Map(VertexBuffer, Graphics::ResourceMap::Write_Discard, &Map);
					memcpy(Map.Pointer, (void*)Vertices.data(), Vertices.size() * sizeof(Trigonometry::Vertex));
					Device->Unmap(VertexBuffer, &Map);
				}

				if (IndexBuffer != nullptr && !Indices.empty())
				{
					Device->Map(IndexBuffer, Graphics::ResourceMap::Write_Discard, &Map);
					memcpy(Map.Pointer, (void*)Indices.data(), Indices.size() * sizeof(int));
					Device->Unmap(IndexBuffer, &Map);
				}
			}
			void SoftBody::Regenerate()
			{
				SceneGraph* Scene = Parent->GetScene();
				VI_ASSERT(Scene != nullptr, "scene should be set");

				if (!Instance)
					return;

				Physics::SoftBody::Desc I = Instance->GetInitialState();
				Core::Memory::Release(Instance);

				Instance = Scene->GetSimulator()->CreateSoftBody(I, Parent->GetTransform());
				VI_PANIC(!Instance, "invalid simulator configuration to create soft bodies");
			}
			void SoftBody::Clear()
			{
				Core::Memory::Release(Instance);
				Core::Memory::Release(Hull);
			}
			void SoftBody::SetTransform(const Trigonometry::Vector3& Position, const Trigonometry::Vector3& Scale, const Trigonometry::Vector3& Rotation)
			{
				if (!Instance)
					return;

				Trigonometry::Transform::Spacing Space;
				Space.Position = Position;
				Space.Rotation = Rotation;
				Space.Scale = Scale;

				auto* Transform = Parent->GetTransform();
				Transform->SetSpacing(Trigonometry::Positioning::Global, Space);
				Instance->Synchronize(Transform, true);
				Instance->SetActivity(true);
			}
			void SoftBody::SetTransform(bool Kinematics)
			{
				if (!Instance)
					return;

				auto* Transform = Parent->GetTransform();
				Instance->Synchronize(Transform, Kinematics);
				Instance->SetActivity(true);
			}
			float SoftBody::GetVisibility(const Viewer& View, float Distance) const
			{
				return Instance ? Component::GetVisibility(View, Distance) : 0.0f;
			}
			size_t SoftBody::GetUnitBounds(Trigonometry::Vector3& Min, Trigonometry::Vector3& Max) const
			{
				if (!Instance)
					return BOX_NONE;

				Trigonometry::Vector3 Center = Instance->GetCenterPosition();
				Instance->GetBoundingBox(&Min, &Max);

				Trigonometry::Vector3 Scale = (Max - Min) * 0.5f;
				Min = Center - Scale;
				Max = Center + Scale;

				return BOX_BODY;
			}
			Component* SoftBody::Copy(Entity* New) const
			{
				SoftBody* Target = new SoftBody(New);
				Target->SetCategory(GetCategory());
				Target->Kinematic = Kinematic;

				if (Instance != nullptr)
				{
					Target->Instance = Instance->Copy();
					Target->Instance->UserPointer = Target;
				}

				return Target;
			}
			Physics::SoftBody* SoftBody::GetBody()
			{
				return Instance;
			}
			Core::Vector<Trigonometry::Vertex>& SoftBody::GetVertices()
			{
				return Vertices;
			}
			Core::Vector<int>& SoftBody::GetIndices()
			{
				return Indices;
			}

			SliderConstraint::SliderConstraint(Entity* Ref) : Component(Ref, ActorSet::None), Instance(nullptr), Connection(nullptr)
			{
			}
			SliderConstraint::~SliderConstraint()
			{
				Core::Memory::Release(Instance);
			}
			void SliderConstraint::Deserialize(Core::Schema* Node)
			{
				VI_ASSERT(Node != nullptr, "schema should be set");

				bool Extended, Ghost, Linear;
				Series::Unpack(Node->Find("extended"), &Extended);
				Series::Unpack(Node->Find("collision-state"), &Ghost);
				Series::Unpack(Node->Find("linear-state"), &Linear);

				if (!Extended)
					return;

				size_t ConnectionId = 0;
				if (Series::Unpack(Node->Find("connection"), &ConnectionId))
				{
					IdxSnapshot* Snapshot = Parent->GetScene()->Snapshot;
					if (Snapshot != nullptr)
					{
						auto It = Snapshot->From.find(ConnectionId);
						if (It != Snapshot->From.end())
							Connection = It->second;
					}
				}

				Load(Connection, Ghost, Linear);
				if (!Instance)
					return;

				float AngularMotorVelocity;
				if (Series::Unpack(Node->Find("angular-motor-velocity"), &AngularMotorVelocity))
					Instance->SetAngularMotorVelocity(AngularMotorVelocity);

				float LinearMotorVelocity;
				if (Series::Unpack(Node->Find("linear-motor-velocity"), &LinearMotorVelocity))
					Instance->SetLinearMotorVelocity(LinearMotorVelocity);

				float UpperLinearLimit;
				if (Series::Unpack(Node->Find("upper-linear-limit"), &UpperLinearLimit))
					Instance->SetUpperLinearLimit(UpperLinearLimit);

				float LowerLinearLimit;
				if (Series::Unpack(Node->Find("lower-linear-limit"), &LowerLinearLimit))
					Instance->SetLowerLinearLimit(LowerLinearLimit);

				float AngularDampingDirection;
				if (Series::Unpack(Node->Find("angular-damping-direction"), &AngularDampingDirection))
					Instance->SetAngularDampingDirection(AngularDampingDirection);

				float LinearDampingDirection;
				if (Series::Unpack(Node->Find("linear-damping-direction"), &LinearDampingDirection))
					Instance->SetLinearDampingDirection(LinearDampingDirection);

				float AngularDampingLimit;
				if (Series::Unpack(Node->Find("angular-damping-limit"), &AngularDampingLimit))
					Instance->SetAngularDampingLimit(AngularDampingLimit);

				float LinearDampingLimit;
				if (Series::Unpack(Node->Find("linear-damping-limit"), &LinearDampingLimit))
					Instance->SetLinearDampingLimit(LinearDampingLimit);

				float AngularDampingOrtho;
				if (Series::Unpack(Node->Find("angular-damping-ortho"), &AngularDampingOrtho))
					Instance->SetAngularDampingOrtho(AngularDampingOrtho);

				float LinearDampingOrtho;
				if (Series::Unpack(Node->Find("linear-damping-ortho"), &LinearDampingOrtho))
					Instance->SetLinearDampingOrtho(LinearDampingOrtho);

				float UpperAngularLimit;
				if (Series::Unpack(Node->Find("upper-angular-limit"), &UpperAngularLimit))
					Instance->SetUpperAngularLimit(UpperAngularLimit);

				float LowerAngularLimit;
				if (Series::Unpack(Node->Find("lower-angular-limit"), &LowerAngularLimit))
					Instance->SetLowerAngularLimit(LowerAngularLimit);

				float MaxAngularMotorForce;
				if (Series::Unpack(Node->Find("max-angular-motor-force"), &MaxAngularMotorForce))
					Instance->SetMaxAngularMotorForce(MaxAngularMotorForce);

				float MaxLinearMotorForce;
				if (Series::Unpack(Node->Find("max-linear-motor-force"), &MaxLinearMotorForce))
					Instance->SetMaxLinearMotorForce(MaxLinearMotorForce);

				float AngularRestitutionDirection;
				if (Series::Unpack(Node->Find("angular-restitution-direction"), &AngularRestitutionDirection))
					Instance->SetAngularRestitutionDirection(AngularRestitutionDirection);

				float LinearRestitutionDirection;
				if (Series::Unpack(Node->Find("linear-restitution-direction"), &LinearRestitutionDirection))
					Instance->SetLinearRestitutionDirection(LinearRestitutionDirection);

				float AngularRestitutionLimit;
				if (Series::Unpack(Node->Find("angular-restitution-limit"), &AngularRestitutionLimit))
					Instance->SetAngularRestitutionLimit(AngularRestitutionLimit);

				float LinearRestitutionLimit;
				if (Series::Unpack(Node->Find("linear-restitution-limit"), &LinearRestitutionLimit))
					Instance->SetLinearRestitutionLimit(LinearRestitutionLimit);

				float AngularRestitutionOrtho;
				if (Series::Unpack(Node->Find("angular-restitution-ortho"), &AngularRestitutionOrtho))
					Instance->SetAngularRestitutionOrtho(AngularRestitutionOrtho);

				float LinearRestitutionOrtho;
				if (Series::Unpack(Node->Find("linear-restitution-ortho"), &LinearRestitutionOrtho))
					Instance->SetLinearRestitutionOrtho(LinearRestitutionOrtho);

				float AngularSoftnessDirection;
				if (Series::Unpack(Node->Find("angular-softness-direction"), &AngularSoftnessDirection))
					Instance->SetAngularSoftnessDirection(AngularSoftnessDirection);

				float LinearSoftnessDirection;
				if (Series::Unpack(Node->Find("linear-softness-direction"), &LinearSoftnessDirection))
					Instance->SetLinearSoftnessDirection(LinearSoftnessDirection);

				float AngularSoftnessLimit;
				if (Series::Unpack(Node->Find("angular-softness-limit"), &AngularSoftnessLimit))
					Instance->SetAngularSoftnessLimit(AngularSoftnessLimit);

				float LinearSoftnessLimit;
				if (Series::Unpack(Node->Find("linear-softness-limit"), &LinearSoftnessLimit))
					Instance->SetLinearSoftnessLimit(LinearSoftnessLimit);

				float AngularSoftnessOrtho;
				if (Series::Unpack(Node->Find("angular-softness-ortho"), &AngularSoftnessOrtho))
					Instance->SetAngularSoftnessOrtho(AngularSoftnessOrtho);

				float LinearSoftnessOrtho;
				if (Series::Unpack(Node->Find("linear-softness-ortho"), &LinearSoftnessOrtho))
					Instance->SetLinearSoftnessOrtho(LinearSoftnessOrtho);

				bool PoweredAngularMotor;
				if (Series::Unpack(Node->Find("powered-angular-motor"), &PoweredAngularMotor))
					Instance->SetPoweredAngularMotor(PoweredAngularMotor);

				bool PoweredLinearMotor;
				if (Series::Unpack(Node->Find("powered-linear-motor"), &PoweredLinearMotor))
					Instance->SetPoweredLinearMotor(PoweredLinearMotor);

				bool Enabled;
				if (Series::Unpack(Node->Find("enabled"), &Enabled))
					Instance->SetEnabled(Enabled);
			}
			void SliderConstraint::Serialize(Core::Schema* Node)
			{
				VI_ASSERT(Node != nullptr, "schema should be set");

				Series::Pack(Node->Set("extended"), Instance != nullptr);
				if (!Instance)
					return;

				int64_t ConnectionId = -1;
				if (Connection != nullptr)
				{
					IdxSnapshot* Snapshot = Parent->GetScene()->Snapshot;
					if (Snapshot != nullptr)
					{
						auto It = Snapshot->To.find(Connection);
						if (It != Snapshot->To.end())
							ConnectionId = (int64_t)It->second;
					}
				}

				Series::Pack(Node->Set("collision-state"), Instance->GetState().Collisions);
				Series::Pack(Node->Set("linear-state"), Instance->GetState().Linear);
				Series::Pack(Node->Set("connection"), ConnectionId);
				Series::Pack(Node->Set("angular-motor-velocity"), Instance->GetAngularMotorVelocity());
				Series::Pack(Node->Set("linear-motor-velocity"), Instance->GetLinearMotorVelocity());
				Series::Pack(Node->Set("upper-linear-limit"), Instance->GetUpperLinearLimit());
				Series::Pack(Node->Set("lower-linear-limit"), Instance->GetLowerLinearLimit());
				Series::Pack(Node->Set("breaking-impulse-threshold"), Instance->GetBreakingImpulseThreshold());
				Series::Pack(Node->Set("angular-damping-direction"), Instance->GetAngularDampingDirection());
				Series::Pack(Node->Set("linear-amping-direction"), Instance->GetLinearDampingDirection());
				Series::Pack(Node->Set("angular-damping-limit"), Instance->GetAngularDampingLimit());
				Series::Pack(Node->Set("linear-damping-limit"), Instance->GetLinearDampingLimit());
				Series::Pack(Node->Set("angular-damping-ortho"), Instance->GetAngularDampingOrtho());
				Series::Pack(Node->Set("linear-damping-ortho"), Instance->GetLinearDampingOrtho());
				Series::Pack(Node->Set("upper-angular-limit"), Instance->GetUpperAngularLimit());
				Series::Pack(Node->Set("lower-angular-limit"), Instance->GetLowerAngularLimit());
				Series::Pack(Node->Set("max-angular-motor-force"), Instance->GetMaxAngularMotorForce());
				Series::Pack(Node->Set("max-linear-motor-force"), Instance->GetMaxLinearMotorForce());
				Series::Pack(Node->Set("angular-restitution-direction"), Instance->GetAngularRestitutionDirection());
				Series::Pack(Node->Set("linear-restitution-direction"), Instance->GetLinearRestitutionDirection());
				Series::Pack(Node->Set("angular-restitution-limit"), Instance->GetAngularRestitutionLimit());
				Series::Pack(Node->Set("linear-restitution-limit"), Instance->GetLinearRestitutionLimit());
				Series::Pack(Node->Set("angular-restitution-ortho"), Instance->GetAngularRestitutionOrtho());
				Series::Pack(Node->Set("linear-restitution-ortho"), Instance->GetLinearRestitutionOrtho());
				Series::Pack(Node->Set("angular-softness-direction"), Instance->GetAngularSoftnessDirection());
				Series::Pack(Node->Set("linear-softness-direction"), Instance->GetLinearSoftnessDirection());
				Series::Pack(Node->Set("angular-softness-limit"), Instance->GetAngularSoftnessLimit());
				Series::Pack(Node->Set("linear-softness-limit"), Instance->GetLinearSoftnessLimit());
				Series::Pack(Node->Set("angular-softness-ortho"), Instance->GetAngularSoftnessOrtho());
				Series::Pack(Node->Set("linear-softness-ortho"), Instance->GetLinearSoftnessOrtho());
				Series::Pack(Node->Set("powered-angular-motor"), Instance->GetPoweredAngularMotor());
				Series::Pack(Node->Set("powered-linear-motor"), Instance->GetPoweredLinearMotor());
				Series::Pack(Node->Set("enabled"), Instance->IsEnabled());
			}
			void SliderConstraint::Load(Entity* Other, bool IsGhosted, bool IsLinear)
			{
				SceneGraph* Scene = Parent->GetScene();
				VI_ASSERT(Scene != nullptr, "scene should be set");
				VI_ASSERT(Parent != Other, "parent should not be equal to other");

				Connection = Other;
				if (!Connection)
					return;

				RigidBody* FirstBody = Parent->GetComponent<RigidBody>();
				RigidBody* SecondBody = Connection->GetComponent<RigidBody>();
				if (!FirstBody || !SecondBody)
					return;

				Physics::SConstraint::Desc I;
				I.TargetA = FirstBody->GetBody();
				I.TargetB = SecondBody->GetBody();
				I.Collisions = !IsGhosted;
				I.Linear = IsLinear;

				if (I.TargetA && I.TargetB)
				{
					Core::Memory::Release(Instance);
					Instance = Scene->GetSimulator()->CreateSliderConstraint(I);
				}
			}
			void SliderConstraint::Clear()
			{
				Core::Memory::Release(Instance);
				Connection = nullptr;
			}
			Component* SliderConstraint::Copy(Entity* New) const
			{
				SliderConstraint* Target = new SliderConstraint(New);
				Target->Connection = Connection;

				if (!Instance)
					return Target;

				RigidBody* FirstBody = New->GetComponent<RigidBody>();
				if (!FirstBody)
					FirstBody = Parent->GetComponent<RigidBody>();

				if (!FirstBody)
					return Target;

				Physics::SConstraint::Desc I(Instance->GetState());
				Instance->GetState().TargetA = FirstBody->GetBody();
				Target->Instance = (Physics::SConstraint*)Instance->Copy();
				Instance->GetState() = I;

				return Target;
			}
			Physics::SConstraint* SliderConstraint::GetConstraint() const
			{
				return Instance;
			}
			Entity* SliderConstraint::GetConnection() const
			{
				return Connection;
			}

			Acceleration::Acceleration(Entity* Ref) : Component(Ref, ActorSet::Update)
			{
			}
			void Acceleration::Deserialize(Core::Schema* Node)
			{
				VI_ASSERT(Node != nullptr, "schema should be set");

				HeavySeries::Unpack(Node->Find("amplitude-velocity"), &AmplitudeVelocity);
				HeavySeries::Unpack(Node->Find("amplitude-torque"), &AmplitudeTorque);
				HeavySeries::Unpack(Node->Find("constant-velocity"), &ConstantVelocity);
				HeavySeries::Unpack(Node->Find("constant-torque"), &ConstantTorque);
				HeavySeries::Unpack(Node->Find("constant-center"), &ConstantCenter);
				Series::Unpack(Node->Find("kinematic"), &Kinematic);
			}
			void Acceleration::Serialize(Core::Schema* Node)
			{
				VI_ASSERT(Node != nullptr, "schema should be set");

				HeavySeries::Pack(Node->Set("amplitude-velocity"), AmplitudeVelocity);
				HeavySeries::Pack(Node->Set("amplitude-torque"), AmplitudeTorque);
				HeavySeries::Pack(Node->Set("constant-velocity"), ConstantVelocity);
				HeavySeries::Pack(Node->Set("constant-torque"), ConstantTorque);
				HeavySeries::Pack(Node->Set("constant-center"), ConstantCenter);
				Series::Pack(Node->Set("kinematic"), Kinematic);
			}
			void Acceleration::Activate(Component* New)
			{
				if (RigidBody != nullptr)
					return;

				Components::RigidBody* Component = Parent->GetComponent<Components::RigidBody>();
				if (Component != nullptr)
					RigidBody = Component->GetBody();
			}
			void Acceleration::Update(Core::Timer* Time)
			{
				if (!RigidBody)
					return;

				float Step = Time->GetStep();
				if (Kinematic)
				{
					RigidBody->SetLinearVelocity(ConstantVelocity);
					RigidBody->SetAngularVelocity(ConstantTorque);
				}
				else
					RigidBody->Push(ConstantVelocity * Step, ConstantTorque * Step, ConstantCenter);

				Trigonometry::Vector3 Force = RigidBody->GetLinearVelocity();
				Trigonometry::Vector3 Torque = RigidBody->GetAngularVelocity();
				Trigonometry::Vector3 ACT = ConstantTorque.Abs();
				Trigonometry::Vector3 ACV = ConstantVelocity.Abs();

				if (AmplitudeVelocity.X > 0 && Force.X > AmplitudeVelocity.X)
					ConstantVelocity.X = -ACV.X;
				else if (AmplitudeVelocity.X > 0 && Force.X < -AmplitudeVelocity.X)
					ConstantVelocity.X = ACV.X;

				if (AmplitudeVelocity.Y > 0 && Force.Y > AmplitudeVelocity.Y)
					ConstantVelocity.Y = -ACV.Y;
				else if (AmplitudeVelocity.Y > 0 && Force.Y < -AmplitudeVelocity.Y)
					ConstantVelocity.Y = ACV.Y;

				if (AmplitudeVelocity.Z > 0 && Force.Z > AmplitudeVelocity.Z)
					ConstantVelocity.Z = -ACV.Z;
				else if (AmplitudeVelocity.Z > 0 && Force.Z < -AmplitudeVelocity.Z)
					ConstantVelocity.Z = ACV.Z;

				if (AmplitudeTorque.X > 0 && Torque.X > AmplitudeTorque.X)
					ConstantTorque.X = -ACT.X;
				else if (AmplitudeTorque.X > 0 && Torque.X < -AmplitudeTorque.X)
					ConstantTorque.X = ACT.X;

				if (AmplitudeTorque.Y > 0 && Torque.Y > AmplitudeTorque.Y)
					ConstantTorque.Y = -ACT.Y;
				else if (AmplitudeTorque.Y > 0 && Torque.Y < -AmplitudeTorque.Y)
					ConstantTorque.Y = ACT.Y;

				if (AmplitudeTorque.Z > 0 && Torque.Z > AmplitudeTorque.Z)
					ConstantTorque.Z = -ACT.Z;
				else if (AmplitudeTorque.Z > 0 && Torque.Z < -AmplitudeTorque.Z)
					ConstantTorque.Z = ACT.Z;
			}
			Component* Acceleration::Copy(Entity* New) const
			{
				Acceleration* Target = new Acceleration(New);
				Target->Kinematic = Kinematic;
				Target->AmplitudeTorque = AmplitudeTorque;
				Target->AmplitudeVelocity = AmplitudeVelocity;
				Target->ConstantCenter = ConstantCenter;
				Target->ConstantTorque = ConstantTorque;
				Target->ConstantVelocity = ConstantVelocity;
				Target->RigidBody = RigidBody;

				return Target;
			}
			Physics::RigidBody* Acceleration::GetBody() const
			{
				return RigidBody;
			}

			Model::Model(Entity* Ref) : Drawable(Ref, ActorSet::None, Model::GetTypeId())
			{
			}
			Model::~Model()
			{
				Core::Memory::Release(Instance);
			}
			void Model::Deserialize(Core::Schema* Node)
			{
				VI_ASSERT(Node != nullptr, "schema should be set");

				uint32_t NewCategory = (uint32_t)GeoCategory::Opaque;
				HeavySeries::Unpack(Node->Find("texcoord"), &TexCoord);
				Series::Unpack(Node->Find("static"), &Static);
				Series::Unpack(Node->Find("category"), &NewCategory);
				SetCategory((GeoCategory)NewCategory);

				Core::String Path;
				if (!Series::Unpack(Node->Find("model"), &Path) || Path.empty())
					return;
				else
					Node->AddRef();

				SceneGraph* Scene = Parent->GetScene();
				Material* Invalid = Scene->GetInvalidMaterial();
				Invalid->AddRef();

				SetMaterial(Invalid);
				Scene->LoadResource<Layer::Model>(this, Path, [this, Node, Scene, Invalid](ExpectsContent<Layer::Model*> NewInstance)
				{
					Core::Memory::Release(Instance);
					Instance = NewInstance.Or(nullptr);
					Invalid->Release();
					ClearMaterials();

					if (Instance != nullptr)
					{
						for (auto&& Material : Node->FetchCollection("materials.material"))
						{
							Core::String Name; size_t Slot = 0;
							if (Series::Unpack(Material->Find("name"), &Name) && Series::Unpack(Material->Find("slot"), &Slot))
							{
								Graphics::MeshBuffer* Surface = Instance->FindMesh(Name);
								if (Surface != nullptr)
									Materials[Surface] = Scene->GetMaterial(Slot);
							}
						}
					}
					Node->Release();
				});
			}
			void Model::Serialize(Core::Schema* Node)
			{
				VI_ASSERT(Node != nullptr, "schema should be set");

				Series::Pack(Node->Set("model"), Parent->GetScene()->FindResourceId<Layer::Model>(Instance));
				HeavySeries::Pack(Node->Set("texcoord"), TexCoord);
				Series::Pack(Node->Set("category"), (uint32_t)GetCategory());
				Series::Pack(Node->Set("static"), Static);

				Core::Schema* Slots = Node->Set("materials", Core::Var::Array());
				for (auto&& Slot : Materials)
				{
					auto* Buffer = (Graphics::MeshBuffer*)Slot.first;
					if (Buffer != nullptr)
					{
						Core::Schema* Material = Slots->Set("material");
						Series::Pack(Material->Set("name"), Buffer->Name);
						if (Slot.second != nullptr)
							Series::Pack(Material->Set("slot"), Slot.second->Slot);
					}
				}
			}
			void Model::SetDrawable(Layer::Model* Drawable)
			{
				Core::Memory::Release(Instance);
				Instance = Drawable;
				ClearMaterials();

				if (!Instance)
					return;

				Instance->AddRef();
				for (auto* Item : Instance->Meshes)
					Materials[(void*)Item] = nullptr;
			}
			void Model::SetMaterialFor(const std::string_view& Name, Material* Value)
			{
				if (!Instance)
					return;

				auto* Mesh = Instance->FindMesh(Name);
				if (Mesh != nullptr)
					Materials[(void*)Mesh] = Value;
			}
			float Model::GetVisibility(const Viewer& View, float Distance) const
			{
				return Instance ? Component::GetVisibility(View, Distance) : 0.0f;
			}
			size_t Model::GetUnitBounds(Trigonometry::Vector3& Min, Trigonometry::Vector3& Max) const
			{
				if (!Instance)
					return BOX_NONE;

				Min = Instance->Min;
				Max = Instance->Max;
				return BOX_GEOMETRY;
			}
			Component* Model::Copy(Entity* New) const
			{
				Model* Target = new Model(New);
				Target->SetCategory(GetCategory());
				Target->Instance = Instance;
				Target->Materials = Materials;
				Target->TexCoord = TexCoord;

				if (Target->Instance != nullptr)
					Target->Instance->AddRef();

				return Target;
			}
			Layer::Model* Model::GetDrawable()
			{
				return Instance;
			}
			Material* Model::GetMaterialFor(const std::string_view& Name)
			{
				if (!Instance)
					return nullptr;

				auto* Mesh = Instance->FindMesh(Name);
				if (!Mesh)
					return nullptr;

				return Materials[(void*)Mesh];
			}

			Skin::Skin(Entity* Ref) : Drawable(Ref, ActorSet::Synchronize, Skin::GetTypeId())
			{
			}
			Skin::~Skin()
			{
				Core::Memory::Release(Instance);
			}
			void Skin::Deserialize(Core::Schema* Node)
			{
				VI_ASSERT(Node != nullptr, "schema should be set");
				uint32_t NewCategory = (uint32_t)GeoCategory::Opaque;
				HeavySeries::Unpack(Node->Find("texcoord"), &TexCoord);
				Series::Unpack(Node->Find("static"), &Static);
				Series::Unpack(Node->Find("category"), &NewCategory);
				SetCategory((GeoCategory)NewCategory);

				Core::String Path;
				if (!Series::Unpack(Node->Find("skin-model"), &Path) || Path.empty())
					return;
				else
					Node->AddRef();

				SceneGraph* Scene = Parent->GetScene();
				Material* Invalid = Scene->GetInvalidMaterial();
				Invalid->AddRef();

				SetMaterial(Invalid);
				Scene->LoadResource<Layer::SkinModel>(this, Path, [this, Node, Scene, Invalid](ExpectsContent<Layer::SkinModel*> NewInstance)
				{
					Core::Memory::Release(Instance);
					Instance = NewInstance.Or(nullptr);
					Invalid->Release();
					ClearMaterials();

					if (Instance != nullptr)
					{
						Skeleton.Fill(Instance);
						for (auto&& Material : Node->FetchCollection("materials.material"))
						{
							Core::String Name; size_t Slot = 0;
							if (Series::Unpack(Material->Find("name"), &Name) && Series::Unpack(Material->Find("slot"), &Slot))
							{
								Graphics::SkinMeshBuffer* Surface = Instance->FindMesh(Name);
								if (Surface != nullptr)
									Materials[Surface] = Scene->GetMaterial(Slot);
							}
						}
					}
					Node->Release();
				});
			}
			void Skin::Serialize(Core::Schema* Node)
			{
				VI_ASSERT(Node != nullptr, "schema should be set");

				Series::Pack(Node->Set("skin-model"), Parent->GetScene()->FindResourceId<Layer::SkinModel>(Instance));
				HeavySeries::Pack(Node->Set("texcoord"), TexCoord);
				Series::Pack(Node->Set("category"), (uint32_t)GetCategory());
				Series::Pack(Node->Set("static"), Static);

				Core::Schema* Slots = Node->Set("materials", Core::Var::Array());
				for (auto&& Slot : Materials)
				{
					if (Slot.first != nullptr)
					{
						Core::Schema* Material = Slots->Set("material");
						Series::Pack(Material->Set("name"), ((Graphics::MeshBuffer*)Slot.first)->Name);

						if (Slot.second != nullptr)
							Series::Pack(Material->Set("slot"), Slot.second->Slot);
					}
				}
			}
			void Skin::Synchronize(Core::Timer* Time)
			{
				if (Instance != nullptr)
					Instance->Synchronize(&Skeleton);
			}
			void Skin::SetDrawable(Layer::SkinModel* Drawable)
			{
				Core::Memory::Release(Instance);
				Instance = Drawable;
				ClearMaterials();

				if (!Instance)
					return;

				Instance->AddRef();
				Skeleton.Fill(Instance);
				for (auto* Item : Instance->Meshes)
					Materials[(void*)Item] = nullptr;
			}
			void Skin::SetMaterialFor(const std::string_view& Name, Material* Value)
			{
				if (!Instance)
					return;

				auto* Mesh = Instance->FindMesh(Name);
				if (Mesh != nullptr)
					Materials[(void*)Mesh] = Value;
			}
			float Skin::GetVisibility(const Viewer& View, float Distance) const
			{
				return Instance ? Component::GetVisibility(View, Distance) : 0.0f;
			}
			size_t Skin::GetUnitBounds(Trigonometry::Vector3& Min, Trigonometry::Vector3& Max) const
			{
				if (!Instance)
					return BOX_NONE;

				Min = Instance->Min;
				Max = Instance->Max;
				return BOX_GEOMETRY;
			}
			Component* Skin::Copy(Entity* New) const
			{
				Skin* Target = new Skin(New);
				Target->SetCategory(GetCategory());
				Target->Instance = Instance;
				Target->Materials = Materials;
				Target->Skeleton = Skeleton;

				if (Target->Instance != nullptr)
					Target->Instance->AddRef();

				return Target;
			}
			Layer::SkinModel* Skin::GetDrawable()
			{
				return Instance;
			}
			Material* Skin::GetMaterialFor(const std::string_view& Name)
			{
				if (!Instance)
					return nullptr;

				auto* Mesh = Instance->FindMesh(Name);
				if (!Mesh)
					return nullptr;

				return Materials[(void*)Mesh];
			}

			Emitter::Emitter(Entity* Ref) : Drawable(Ref, ActorSet::None, Emitter::GetTypeId())
			{
			}
			Emitter::~Emitter()
			{
				Core::Memory::Release(Instance);
			}
			void Emitter::Deserialize(Core::Schema* Node)
			{
				VI_ASSERT(Node != nullptr, "schema should be set");

				SceneGraph* Scene = Parent->GetScene(); size_t Slot = 0;
				if (Series::Unpack(Node->Find("material"), &Slot))
					SetMaterial(nullptr, Scene->GetMaterial(Slot));

				uint32_t NewCategory = (uint32_t)GeoCategory::Opaque;
				Series::Unpack(Node->Find("category"), &NewCategory);
				Series::Unpack(Node->Find("quad-based"), &QuadBased);
				Series::Unpack(Node->Find("connected"), &Connected);
				Series::Unpack(Node->Find("static"), &Static);
				HeavySeries::Unpack(Node->Find("min"), &Min);
				HeavySeries::Unpack(Node->Find("max"), &Max);
				SetCategory((GeoCategory)NewCategory);

				size_t Limit;
				if (Series::Unpack(Node->Find("limit"), &Limit) && Instance != nullptr)
				{
					auto& Dest = Instance->GetArray();
					HeavySeries::Unpack(Node->Find("elements"), &Dest);
					Scene->GetDevice()->UpdateBufferSize(Instance, Limit);
				}
			}
			void Emitter::Serialize(Core::Schema* Node)
			{
				VI_ASSERT(Node != nullptr, "schema should be set");

				Material* Slot = GetMaterial();
				if (Slot != nullptr)
					Series::Pack(Node->Set("material"), Slot->Slot);

				Series::Pack(Node->Set("category"), (uint32_t)GetCategory());
				Series::Pack(Node->Set("quad-based"), QuadBased);
				Series::Pack(Node->Set("connected"), Connected);
				Series::Pack(Node->Set("static"), Static);
				HeavySeries::Pack(Node->Set("min"), Min);
				HeavySeries::Pack(Node->Set("max"), Max);

				if (Instance != nullptr)
				{
					Series::Pack(Node->Set("limit"), Instance->GetElementLimit());
					HeavySeries::Pack(Node->Set("elements"), Instance->GetArray());
				}
			}
			void Emitter::Activate(Component* New)
			{
				if (Instance != nullptr)
					return;

				SceneGraph* Scene = Parent->GetScene();
				Graphics::InstanceBuffer::Desc I = Graphics::InstanceBuffer::Desc();
				I.ElementLimit = 1 << 10;

				auto Buffer = Scene->GetDevice()->CreateInstanceBuffer(I);
				if (Buffer)
					Instance = *Buffer;
			}
			size_t Emitter::GetUnitBounds(Trigonometry::Vector3& _Min, Trigonometry::Vector3& _Max) const
			{
				if (!Instance)
					return BOX_NONE;

				_Min = Min;
				_Max = Max;
				return BOX_DYNAMIC;
			}
			Component* Emitter::Copy(Entity* New) const
			{
				Emitter* Target = new Emitter(New);
				Target->SetCategory(GetCategory());
				Target->Connected = Connected;
				Target->Materials = Materials;
				Target->Min = Min;
				Target->Max = Max;

				auto& Dest = Target->Instance->GetArray();
				Dest = Instance->GetArray();

				return Target;
			}
			Graphics::InstanceBuffer* Emitter::GetBuffer()
			{
				return Instance;
			}

			Decal::Decal(Entity* Ref) : Drawable(Ref, ActorSet::None, Decal::GetTypeId())
			{
			}
			void Decal::Deserialize(Core::Schema* Node)
			{
				VI_ASSERT(Node != nullptr, "schema should be set");

				size_t Slot = 0;
				if (Series::Unpack(Node->Find("material"), &Slot))
					SetMaterial(nullptr, Parent->GetScene()->GetMaterial(Slot));

				uint32_t NewCategory = (uint32_t)GeoCategory::Opaque;
				HeavySeries::Unpack(Node->Find("texcoord"), &TexCoord);
				Series::Unpack(Node->Find("static"), &Static);
				Series::Unpack(Node->Find("category"), &NewCategory);
				SetCategory((GeoCategory)NewCategory);
			}
			void Decal::Serialize(Core::Schema* Node)
			{
				VI_ASSERT(Node != nullptr, "schema should be set");

				Material* Slot = GetMaterial();
				if (Slot != nullptr)
					Series::Pack(Node->Set("material"), Slot->Slot);

				HeavySeries::Pack(Node->Set("texcoord"), TexCoord);
				Series::Pack(Node->Set("static"), Static);
				Series::Pack(Node->Set("category"), (uint32_t)GetCategory());
			}
			float Decal::GetVisibility(const Viewer& View, float Distance) const
			{
				return GetVisibilityRadius(Parent, View, Distance);
			}
			Component* Decal::Copy(Entity* New) const
			{
				Decal* Target = new Decal(New);
				Target->TexCoord = TexCoord;
				Target->Materials = Materials;

				return Target;
			}

			SkinAnimator::SkinAnimator(Entity* Ref) : Component(Ref, ActorSet::Animate)
			{
			}
			SkinAnimator::~SkinAnimator() noexcept
			{
				Core::Memory::Release(Animation);
			}
			void SkinAnimator::Deserialize(Core::Schema* Node)
			{
				VI_ASSERT(Node != nullptr, "schema should be set");
				HeavySeries::Unpack(Node->Find("state"), &State);

				Core::String Path;
				if (!Series::Unpack(Node->Find("path"), &Path))
					return;

				Parent->GetScene()->LoadResource<Layer::SkinAnimation>(this, Path, [this](ExpectsContent<Layer::SkinAnimation*> Result)
				{
					Animation = Result.Or(nullptr);
				});
			}
			void SkinAnimator::Serialize(Core::Schema* Node)
			{
				VI_ASSERT(Node != nullptr, "schema should be set");
				HeavySeries::Pack(Node->Set("state"), State);
				Series::Pack(Node->Set("path"), Parent->GetScene()->FindResourceId<SkinAnimation>(Animation));
			}
			void SkinAnimator::Activate(Component* New)
			{
				Components::Skin* Base = Parent->GetComponent<Components::Skin>();
				if (Base != nullptr && Base->GetDrawable() != nullptr)
				{
					Instance = Base;
					SetActive(true);
				}
				else
				{
					Instance = nullptr;
					SetActive(false);
				}
			}
			void SkinAnimator::Animate(Core::Timer* Time)
			{
				if (!Animation || State.Paused)
					return;

				auto& Clips = Animation->GetClips();
				if (!State.Blended)
				{
					if (State.Clip < 0 || State.Clip >= (int64_t)Clips.size() || State.Frame < 0 || State.Frame >= (int64_t)Clips[(size_t)State.Clip].Keys.size())
						return;

					auto& Clip = Clips[(size_t)State.Clip];
					auto& NextKey = Clip.Keys[(size_t)State.Frame + 1 >= Clip.Keys.size() ? 0 : (size_t)State.Frame + 1];
					auto& PrevKey = Clip.Keys[(size_t)State.Frame];
					State.Duration = Clip.Duration;
					State.Rate = Clip.Rate;
					State.Time = State.GetTimeline(Time);

					float T = State.GetProgress();
					for (auto&& Pose : Instance->Skeleton.Offsets)
					{
						auto& Prev = PrevKey.Pose[Pose.first];
						auto& Next = NextKey.Pose[Pose.first];
						Pose.second.Frame.Position = Prev.Position.Lerp(Next.Position, T);
						Pose.second.Frame.Scale = Prev.Scale.Lerp(Next.Scale, T);
						Pose.second.Frame.Rotation = Prev.Rotation.sLerp(Next.Rotation, T);
						Pose.second.Offset = Pose.second.Frame;
					}

					if (State.GetProgressTotal() >= 1.0f)
					{
						if (State.Frame + 1 >= (int64_t)Clip.Keys.size())
						{
							if (!State.Looped)
								BlendAnimation(-1, -1);
							State.Frame = -1;
						}

						State.Time = 0.0f;
						if (State.Looped || State.Frame != -1)
							State.Frame++;
					}
				}
				else if (State.GetProgressTotal() < 1.0f)
				{
					auto* Keys = (IsExists(State.Clip, State.Frame) ? GetFrame(State.Clip, State.Frame) : nullptr);
					State.Time = State.GetTimeline(Time);
					float T = State.GetProgress();

					for (auto&& Pose : Instance->Skeleton.Offsets)
					{
						if (Keys != nullptr)
						{
							auto& Next = Keys->Pose[Pose.first];
							Pose.second.Offset.Position = Pose.second.Frame.Position.Lerp(Next.Position, T);
							Pose.second.Offset.Scale = Pose.second.Frame.Scale.Lerp(Next.Scale, T);
							Pose.second.Offset.Rotation = Pose.second.Frame.Rotation.sLerp(Next.Rotation, T);
						}
						else
						{
							Pose.second.Offset.Position = Pose.second.Frame.Position.Lerp(Pose.second.Default.Position, T);
							Pose.second.Offset.Scale = Pose.second.Frame.Scale.Lerp(Pose.second.Default.Scale, T);
							Pose.second.Offset.Rotation = Pose.second.Frame.Rotation.sLerp(Pose.second.Default.Rotation, T);
						}
					}
				}
				else
				{
					State.Blended = false;
					State.Time = 0.0f;
				}
			}
			void SkinAnimator::SetAnimation(SkinAnimation* New)
			{
				Core::Memory::Release(Animation);
				Animation = New;
				if (Animation != nullptr)
					Animation->AddRef();
			}
			void SkinAnimator::BlendAnimation(int64_t Clip, int64_t Frame)
			{
				State.Blended = true;
				State.Time = 0.0f;
				State.Frame = Frame;
				State.Clip = Clip;

				if (Animation != nullptr)
				{
					auto& Clips = Animation->GetClips();
					if (State.Clip >= 0 && (size_t)State.Clip < Clips.size())
					{
						auto& Next = Clips[(size_t)State.Clip];
						if (State.Frame < 0 || (size_t)State.Frame >= Next.Keys.size())
							State.Frame = -1;

						if (State.Duration <= 0.0f)
						{
							State.Duration = Next.Duration;
							State.Rate = Next.Rate;
						}
					}
					else
						State.Clip = -1;
				}
				else
					State.Clip = -1;
			}
			void SkinAnimator::SaveBindingState()
			{
				for (auto&& Pose : Instance->Skeleton.Offsets)
				{
					Pose.second.Frame = Pose.second.Default;
					Pose.second.Offset = Pose.second.Frame;
				}
			}
			void SkinAnimator::Stop()
			{
				State.Paused = false;
				BlendAnimation(-1, -1);
			}
			void SkinAnimator::Pause()
			{
				State.Paused = true;
			}
			void SkinAnimator::Play(int64_t Clip, int64_t Frame)
			{
				if (State.Paused)
				{
					State.Paused = false;
					return;
				}

				State.Time = 0.0f;
				State.Frame = (Frame == -1 ? 0 : Frame);
				State.Clip = (Clip == -1 ? 0 : Clip);

				if (!IsExists(State.Clip, State.Frame))
					return;

				if (Animation != nullptr)
				{
					auto& Clips = Animation->GetClips();
					if (State.Clip >= 0 && (size_t)State.Clip < Clips.size())
					{
						auto& Next = Clips[(size_t)State.Clip];
						if (State.Frame < 0 || (size_t)State.Frame >= Next.Keys.size())
							State.Frame = -1;
					}
					else
						State.Clip = -1;
				}
				else
					State.Clip = -1;

				SaveBindingState();
				if (State.GetProgress() < 1.0f)
					BlendAnimation(State.Clip, State.Frame);
			}
			bool SkinAnimator::IsExists(int64_t Clip)
			{
				if (!Animation)
					return false;

				auto& Clips = Animation->GetClips();
				return Clip >= 0 && (size_t)Clip < Clips.size();
			}
			bool SkinAnimator::IsExists(int64_t Clip, int64_t Frame)
			{
				if (!IsExists(Clip))
					return false;

				auto& Clips = Animation->GetClips();
				return Frame >= 0 && (size_t)Frame < Clips[(size_t)Clip].Keys.size();
			}
			const Trigonometry::SkinAnimatorKey* SkinAnimator::GetFrame(int64_t Clip, int64_t Frame)
			{
				VI_ASSERT(Animation != nullptr, "animation should be set");
				auto& Clips = Animation->GetClips();

				VI_ASSERT(Clip >= 0 && (size_t)Clip < Clips.size(), "clip index outside of range");
				VI_ASSERT(Frame >= 0 && (size_t)Frame < Clips[(size_t)Clip].Keys.size(), "frame index outside of range");
				return &Clips[(size_t)Clip].Keys[(size_t)Frame];
			}
			const Core::Vector<Trigonometry::SkinAnimatorKey>* SkinAnimator::GetClip(int64_t Clip)
			{
				VI_ASSERT(Animation != nullptr, "animation should be set");
				auto& Clips = Animation->GetClips();

				VI_ASSERT(Clip >= 0 && (size_t)Clip < Clips.size(), "clip index outside of range");
				return &Clips[(size_t)Clip].Keys;
			}
			Core::String SkinAnimator::GetPath() const
			{
				return Parent->GetScene()->FindResourceId<SkinAnimation>(Animation);
			}
			Component* SkinAnimator::Copy(Entity* New) const
			{
				SkinAnimator* Target = new SkinAnimator(New);
				Target->Animation = Animation;
				Target->State = State;

				if (Animation != nullptr)
					Animation->AddRef();

				return Target;
			}
			int64_t SkinAnimator::GetClipByName(const std::string_view& Name) const
			{
				if (!Animation)
					return -1;

				size_t Index = 0;
				for (auto& Item : Animation->GetClips())
				{
					if (Item.Name == Name)
						return (int64_t)Index;
					Index++;
				}

				return -1;
			}
			size_t SkinAnimator::GetClipsCount() const
			{
				if (!Animation)
					return 0;

				return Animation->GetClips().size();
			}
			SkinAnimation* SkinAnimator::GetAnimation() const
			{
				return Animation;
			}
			Skin* SkinAnimator::GetSkin() const
			{
				return Instance;
			}

			KeyAnimator::KeyAnimator(Entity* Ref) : Component(Ref, ActorSet::Animate)
			{
			}
			KeyAnimator::~KeyAnimator()
			{
			}
			void KeyAnimator::Deserialize(Core::Schema* Node)
			{
				VI_ASSERT(Node != nullptr, "schema should be set");
				HeavySeries::Unpack(Node->Find("state"), &State);
				HeavySeries::Unpack(Node->Find("offset"), &Offset);
				HeavySeries::Unpack(Node->Find("default"), &Default);

				Core::String Path;
				if (!Series::Unpack(Node->Find("path"), &Path) || Path.empty())
					HeavySeries::Unpack(Node->Find("animation"), &Clips);
				else
					LoadAnimation(Path, nullptr);
			}
			void KeyAnimator::Serialize(Core::Schema* Node)
			{
				VI_ASSERT(Node != nullptr, "schema should be set");
				HeavySeries::Pack(Node->Set("state"), State);
				HeavySeries::Pack(Node->Set("offset"), Offset);
				HeavySeries::Pack(Node->Set("default"), Default);

				if (Reference.empty())
					HeavySeries::Pack(Node->Set("animation"), Clips);
				else
					Series::Pack(Node->Set("path"), Reference);
			}
			void KeyAnimator::Animate(Core::Timer* Time)
			{
				auto* Transform = Parent->GetTransform();
				if (State.Paused)
					return;

				if (!State.Blended)
				{
					if (State.Clip < 0 || (size_t)State.Clip >= Clips.size() || State.Frame < 0 || (size_t)State.Frame >= Clips[(size_t)State.Clip].Keys.size())
						return;

					auto& Clip = Clips[(size_t)State.Clip];
					auto& NextKey = Clip.Keys[(size_t)State.Frame + 1 >= Clip.Keys.size() ? 0 : (size_t)State.Frame + 1];
					auto& PrevKey = Clip.Keys[(size_t)State.Frame];
					State.Duration = Clip.Duration;
					State.Rate = Clip.Rate * NextKey.Time;
					State.Time = State.GetTimeline(Time);

					float T = State.GetProgress();
					Offset.Position = PrevKey.Position.Lerp(NextKey.Position, T);
					Offset.Rotation = PrevKey.Rotation.Lerp(NextKey.Rotation, T);
					Offset.Scale = PrevKey.Scale.Lerp(NextKey.Scale, T);

					Transform->SetPosition(Offset.Position);
					Transform->SetRotation(Offset.Rotation.GetEuler());
					Transform->SetScale(Offset.Scale);

					if (State.GetProgressTotal() >= 1.0f)
					{
						if (State.Frame + 1 >= (int64_t)Clip.Keys.size())
						{
							if (!State.Looped)
								BlendAnimation(-1, -1);
							State.Frame = -1;
						}

						State.Time = 0.0f;
						if (State.Looped || State.Frame != -1)
							State.Frame++;
					}
				}
				else if (State.GetProgressTotal() < 1.0f)
				{
					auto* Next = (IsExists(State.Clip, State.Frame) ? GetFrame(State.Clip, State.Frame) : nullptr);
					if (!Next)
						Next = &Default;

					State.Time = State.GetTimeline(Time);
					float T = State.GetProgress();
					Transform->SetPosition(Offset.Position.Lerp(Next->Position, T));
					Transform->SetRotation(Offset.Rotation.Lerp(Next->Rotation, T).GetEuler());
					Transform->SetScale(Offset.Scale.Lerp(Next->Scale, T));
				}
				else
				{
					State.Blended = false;
					State.Time = 0.0f;
				}
			}
			void KeyAnimator::LoadAnimation(const std::string_view& Path, std::function<void(bool)>&& Callback)
			{
				auto* Scene = Parent->GetScene();
				auto Copy = Core::String(Path);
				Scene->LoadResource<Core::Schema>(this, Path, [this, Scene, Copy, Callback = std::move(Callback)](ExpectsContent<Core::Schema*> Result)
				{
					ClearAnimation();
					if (Result && *Result != nullptr)
					{
						if (HeavySeries::Unpack(*Result, &Clips))
							Reference = Scene->AsResourcePath(Copy);
						else
							Reference.clear();
						Core::Memory::Release(*Result);
					}
					else
						Reference.clear();

					if (Callback)
						Callback(!Reference.empty());
				});
			}
			void KeyAnimator::ClearAnimation()
			{
				Reference.clear();
				Clips.clear();
			}
			void KeyAnimator::BlendAnimation(int64_t Clip, int64_t Frame)
			{
				State.Blended = true;
				State.Time = 0.0f;
				State.Frame = Frame;
				State.Clip = Clip;

				if (State.Clip >= 0 && (size_t)State.Clip < Clips.size())
				{
					auto& Next = Clips[(size_t)State.Clip];
					if (State.Frame < 0 || (size_t)State.Frame >= Next.Keys.size())
						State.Frame = -1;
				}
				else
					State.Clip = -1;
			}
			void KeyAnimator::SaveBindingState()
			{
				auto& Space = Parent->GetTransform()->GetSpacing();
				Default.Position = Space.Position;
				Default.Rotation = Space.Rotation;
				Default.Scale = Space.Scale;
				Offset = Default;
			}
			void KeyAnimator::Stop()
			{
				State.Paused = false;
				BlendAnimation(-1, -1);
			}
			void KeyAnimator::Pause()
			{
				State.Paused = true;
			}
			void KeyAnimator::Play(int64_t Clip, int64_t Frame)
			{
				if (State.Paused)
				{
					State.Paused = false;
					return;
				}

				State.Time = 0.0f;
				State.Frame = (Frame == -1 ? 0 : Frame);
				State.Clip = (Clip == -1 ? 0 : Clip);

				if (!IsExists(State.Clip, State.Frame))
					return;

				if (State.Clip >= 0 && (size_t)State.Clip < Clips.size())
				{
					auto& Next = Clips[(size_t)State.Clip];
					if (State.Frame < 0 || (size_t)State.Frame >= Next.Keys.size())
						State.Frame = -1;

					if (State.Duration <= 0.0f)
					{
						State.Duration = Next.Duration;
						State.Rate = Next.Rate;
					}
				}
				else
					State.Clip = -1;

				SaveBindingState();
				if (State.GetProgress() < 1.0f)
					BlendAnimation(State.Clip, State.Frame);
			}
			bool KeyAnimator::IsExists(int64_t Clip)
			{
				return Clip >= 0 && (size_t)Clip < Clips.size();
			}
			bool KeyAnimator::IsExists(int64_t Clip, int64_t Frame)
			{
				if (!IsExists(Clip))
					return false;

				return Frame >= 0 && (size_t)Frame < Clips[(size_t)Clip].Keys.size();
			}
			Trigonometry::AnimatorKey* KeyAnimator::GetFrame(int64_t Clip, int64_t Frame)
			{
				VI_ASSERT(Clip >= 0 && (size_t)Clip < Clips.size(), "clip index outside of range");
				VI_ASSERT(Frame >= 0 && (size_t)Frame < Clips[(size_t)Clip].Keys.size(), "frame index outside of range");
				return &Clips[(size_t)Clip].Keys[(size_t)Frame];
			}
			Core::Vector<Trigonometry::AnimatorKey>* KeyAnimator::GetClip(int64_t Clip)
			{
				VI_ASSERT(Clip >= 0 && (size_t)Clip < Clips.size(), "clip index outside of range");
				return &Clips[(size_t)Clip].Keys;
			}
			Core::String KeyAnimator::GetPath()
			{
				return Reference;
			}
			Component* KeyAnimator::Copy(Entity* New) const
			{
				KeyAnimator* Target = new KeyAnimator(New);
				Target->Clips = Clips;
				Target->State = State;
				Target->Default = Default;
				Target->Offset = Offset;

				return Target;
			}

			EmitterAnimator::EmitterAnimator(Entity* Ref) : Component(Ref, ActorSet::Animate)
			{
				Spawner.Scale.Max = 1;
				Spawner.Scale.Min = 1;
				Spawner.Rotation.Max = 0;
				Spawner.Rotation.Min = 0;
				Spawner.Angular.Max = 0;
				Spawner.Angular.Min = 0;
				Spawner.Diffusion.Min = 1;
				Spawner.Diffusion.Max = 1;
				Spawner.Velocity.Min = 0;
				Spawner.Velocity.Max = 0;
				Spawner.Position.Min = -1;
				Spawner.Position.Max = 1;
				Spawner.Noise.Min = -1;
				Spawner.Noise.Max = 1;
				Spawner.Iterations = 1;
			}
			void EmitterAnimator::Deserialize(Core::Schema* Node)
			{
				VI_ASSERT(Node != nullptr, "schema should be set");

				HeavySeries::Unpack(Node->Find("diffuse"), &Diffuse);
				HeavySeries::Unpack(Node->Find("position"), &Position);
				HeavySeries::Unpack(Node->Find("velocity"), &Velocity);
				HeavySeries::Unpack(Node->Find("spawner"), &Spawner);
				Series::Unpack(Node->Find("noise"), &Noise);
				Series::Unpack(Node->Find("rotation-speed"), &RotationSpeed);
				Series::Unpack(Node->Find("scale-speed"), &ScaleSpeed);
				Series::Unpack(Node->Find("simulate"), &Simulate);
			}
			void EmitterAnimator::Serialize(Core::Schema* Node)
			{
				VI_ASSERT(Node != nullptr, "schema should be set");

				HeavySeries::Pack(Node->Set("diffuse"), Diffuse);
				HeavySeries::Pack(Node->Set("position"), Position);
				HeavySeries::Pack(Node->Set("velocity"), Velocity);
				HeavySeries::Pack(Node->Set("spawner"), Spawner);
				Series::Pack(Node->Set("noise"), Noise);
				Series::Pack(Node->Set("rotation-speed"), RotationSpeed);
				Series::Pack(Node->Set("scale-speed"), ScaleSpeed);
				Series::Pack(Node->Set("simulate"), Simulate);
			}
			void EmitterAnimator::Activate(Component* New)
			{
				Base = Parent->GetComponent<Emitter>();
				SetActive(Base != nullptr);
			}
			void EmitterAnimator::Animate(Core::Timer* Time)
			{
				if (!Simulate || !Base || !Base->GetBuffer())
					return;

				auto* Transform = Parent->GetTransform();
				auto& Array = Base->GetBuffer()->GetArray();
				Trigonometry::Vector3 Offset = Transform->GetPosition();

				for (int i = 0; i < Spawner.Iterations; i++)
				{
					if (Array.size() + 1 >= Array.capacity())
						break;

					Trigonometry::Vector3 FPosition = (Base->Connected ? Spawner.Position.Generate() : Spawner.Position.Generate() + Offset);
					Trigonometry::Vector3 FVelocity = Spawner.Velocity.Generate();
					Trigonometry::Vector4 FDiffusion = Spawner.Diffusion.Generate();

					Trigonometry::ElementVertex Element;
					Element.PositionX = FPosition.X;
					Element.PositionY = FPosition.Y;
					Element.PositionZ = FPosition.Z;
					Element.VelocityX = FVelocity.X;
					Element.VelocityY = FVelocity.Y;
					Element.VelocityZ = FVelocity.Z;
					Element.ColorX = FDiffusion.X;
					Element.ColorY = FDiffusion.Y;
					Element.ColorZ = FDiffusion.Z;
					Element.ColorW = FDiffusion.W;
					Element.Angular = Spawner.Angular.Generate();
					Element.Rotation = Spawner.Rotation.Generate();
					Element.Scale = Spawner.Scale.Generate();
					Array.emplace_back(std::move(Element));
				}

				float Step = Time->GetStep();
				if (Noise != 0.0f)
					AccurateSynchronization(Step);
				else
					FastSynchronization(Step);
				Transform->MakeDirty();
			}
			void EmitterAnimator::AccurateSynchronization(float Step)
			{
				auto& Array = Base->GetBuffer()->GetArray();
				float MinX = 0.0f, MaxX = 0.0f;
				float MinY = 0.0f, MaxY = 0.0f;
				float MinZ = 0.0f, MaxZ = 0.0f;
				auto Begin = Array.begin();
				auto End = Array.end();

				for (auto It = Begin; It != End;)
				{
					Trigonometry::Vector3 NextVelocity(It->VelocityX, It->VelocityY, It->VelocityZ);
					Trigonometry::Vector3 NextNoise = Spawner.Noise.Generate() / Noise;
					Trigonometry::Vector3 NextPosition(It->PositionX, It->PositionY, It->PositionZ);
					Trigonometry::Vector4 NextDiffuse(It->ColorX, It->ColorY, It->ColorZ, It->ColorW);
					NextVelocity -= (NextVelocity * Velocity) * Step;
					NextPosition += (NextVelocity + Position + NextNoise) * Step;
					NextDiffuse += Diffuse * Step;
					memcpy(&It->VelocityX, &NextVelocity, sizeof(float) * 3);
					memcpy(&It->PositionX, &NextPosition, sizeof(float) * 3);
					memcpy(&It->ColorX, &NextDiffuse, sizeof(float) * 4);
					It->Rotation += (It->Angular + RotationSpeed) * Step;
					It->Scale += ScaleSpeed * Step;

					if (It->ColorW > 0.001f && It->Scale > 0.001f)
					{
						if (It->PositionX < MinX)
							MinX = It->PositionX;
						else if (It->PositionX > MaxX)
							MaxX = It->PositionX;

						if (It->PositionY < MinY)
							MinY = It->PositionY;
						else if (It->PositionY > MaxY)
							MaxY = It->PositionY;

						if (It->PositionZ < MinZ)
							MinZ = It->PositionZ;
						else if (It->PositionZ > MaxZ)
							MaxZ = It->PositionZ;
						++It;
					}
					else
					{
						It = Array.erase(It);
						Begin = Array.begin();
						End = Array.end();
					}
				}

				Base->Min = Trigonometry::Vector3(MinX, MinY, MinZ);
				Base->Max = Trigonometry::Vector3(MaxX, MaxY, MaxZ);
			}
			void EmitterAnimator::FastSynchronization(float Step)
			{
				auto& Array = Base->GetBuffer()->GetArray();
				float MinX = 0.0f, MaxX = 0.0f;
				float MinY = 0.0f, MaxY = 0.0f;
				float MinZ = 0.0f, MaxZ = 0.0f;
				auto Begin = Array.begin();
				auto End = Array.end();

				for (auto It = Begin; It != End;)
				{
					Trigonometry::Vector3 NextVelocity(It->VelocityX, It->VelocityY, It->VelocityZ);
					Trigonometry::Vector3 NextPosition(It->PositionX, It->PositionY, It->PositionZ);
					Trigonometry::Vector4 NextDiffuse(It->ColorX, It->ColorY, It->ColorZ, It->ColorW);
					NextVelocity -= (NextVelocity * Velocity) * Step;
					NextPosition += (NextVelocity + Position) * Step;
					NextDiffuse += Diffuse * Step;
					memcpy(&It->VelocityX, &NextVelocity, sizeof(float) * 3);
					memcpy(&It->PositionX, &NextPosition, sizeof(float) * 3);
					memcpy(&It->ColorX, &NextDiffuse, sizeof(float) * 4);
					It->Rotation += (It->Angular + RotationSpeed) * Step;
					It->Scale += ScaleSpeed * Step;

					if (It->ColorW > 0.001f && It->Scale > 0.001f)
					{
						if (It->PositionX < MinX)
							MinX = It->PositionX;
						else if (It->PositionX > MaxX)
							MaxX = It->PositionX;

						if (It->PositionY < MinY)
							MinY = It->PositionY;
						else if (It->PositionY > MaxY)
							MaxY = It->PositionY;

						if (It->PositionZ < MinZ)
							MinZ = It->PositionZ;
						else if (It->PositionZ > MaxZ)
							MaxZ = It->PositionZ;
						++It;
					}
					else
					{
						It = Array.erase(It);
						Begin = Array.begin();
						End = Array.end();
					}
				}

				Base->Min = Trigonometry::Vector3(MinX, MinY, MinZ);
				Base->Max = Trigonometry::Vector3(MaxX, MaxY, MaxZ);
			}
			Component* EmitterAnimator::Copy(Entity* New) const
			{
				EmitterAnimator* Target = new EmitterAnimator(New);
				Target->Diffuse = Diffuse;
				Target->Position = Position;
				Target->Velocity = Velocity;
				Target->ScaleSpeed = ScaleSpeed;
				Target->RotationSpeed = RotationSpeed;
				Target->Spawner = Spawner;
				Target->Noise = Noise;
				Target->Simulate = Simulate;

				return Target;
			}
			Emitter* EmitterAnimator::GetEmitter() const
			{
				return Base;
			}

			FreeLook::FreeLook(Entity* Ref) : Component(Ref, ActorSet::Update), Rotate(Graphics::KeyCode::CursorRight), Sensivity(0.005f)
			{
			}
			void FreeLook::Update(Core::Timer* Time)
			{
				auto* Activity = Parent->GetScene()->GetActivity();
				if (!Activity)
					return;

				Trigonometry::Vector2 Cursor = Activity->GetGlobalCursorPosition();
				if (!Activity->IsKeyDown(Rotate))
				{
					Position = Cursor;
					return;
				}

				if (!Activity->IsKeyDownHit(Rotate))
				{
					auto* Transform = Parent->GetTransform();
					Trigonometry::Vector2 Next = (Cursor - Position) * Sensivity;
					Transform->Rotate(Trigonometry::Vector3(Next.Y, Next.X) * Direction);

					const Trigonometry::Vector3& Rotation = Transform->GetRotation();
					Transform->SetRotation(Rotation.SetX(Compute::Mathf::Clamp(Rotation.X, -1.57079632679f, 1.57079632679f)));
				}
				else
					Position = Cursor;

				if ((int)Cursor.X != (int)Position.X || (int)Cursor.Y != (int)Position.Y)
					Activity->SetGlobalCursorPosition(Position);
			}
			Component* FreeLook::Copy(Entity* New) const
			{
				FreeLook* Target = new FreeLook(New);
				Target->Position = Position;
				Target->Rotate = Rotate;
				Target->Sensivity = Sensivity;

				return Target;
			}

			Fly::Fly(Entity* Ref) : Component(Ref, ActorSet::Update)
			{
			}
			void Fly::Update(Core::Timer* Time)
			{
				auto* Activity = Parent->GetScene()->GetActivity();
				if (!Activity)
					return;

				auto* Transform = Parent->GetTransform();
				Trigonometry::Vector3 NewVelocity;

				if (Activity->IsKeyDown(Forward))
					NewVelocity += Transform->Forward().ViewSpace();
				else if (Activity->IsKeyDown(Backward))
					NewVelocity -= Transform->Forward().ViewSpace();

				if (Activity->IsKeyDown(Right))
					NewVelocity += Transform->Right().ViewSpace();
				else if (Activity->IsKeyDown(Left))
					NewVelocity -= Transform->Right().ViewSpace();

				if (Activity->IsKeyDown(Up))
					NewVelocity += Transform->Up().ViewSpace();
				else if (Activity->IsKeyDown(Down))
					NewVelocity -= Transform->Up().ViewSpace();

				float Step = Time->GetStep();
				if (NewVelocity.Length() > 0.001f)
					Velocity += GetSpeed(Activity) * NewVelocity * Step;

				if (Velocity.Length() > 0.001f)
				{
					Transform->Move(Velocity * Step);
					Velocity = Velocity.Lerp(Trigonometry::Vector3::Zero(), Moving.Fading * Step);
				}
				else
					Velocity = Trigonometry::Vector3::Zero();
			}
			Component* Fly::Copy(Entity* New) const
			{
				Fly* Target = new Fly(New);
				Target->Moving = Moving;
				Target->Velocity = Velocity;
				Target->Forward = Forward;
				Target->Backward = Backward;
				Target->Right = Right;
				Target->Left = Left;
				Target->Up = Up;
				Target->Down = Down;
				Target->Fast = Fast;
				Target->Slow = Slow;

				return Target;
			}
			Trigonometry::Vector3 Fly::GetSpeed(Graphics::Activity* Activity)
			{
				if (Activity->IsKeyDown(Fast))
					return Moving.Axis * Moving.Faster;

				if (Activity->IsKeyDown(Slow))
					return Moving.Axis * Moving.Slower;

				return Moving.Axis * Moving.Normal;
			}

			AudioSource::AudioSource(Entity* Ref) : Component(Ref, ActorSet::Synchronize)
			{
				Source = new Audio::AudioSource();
			}
			AudioSource::~AudioSource()
			{
				Core::Memory::Release(Source);
			}
			void AudioSource::Deserialize(Core::Schema* Node)
			{
				VI_ASSERT(Node != nullptr, "schema should be set");

				HeavySeries::Unpack(Node->Find("velocity"), &Sync.Velocity);
				HeavySeries::Unpack(Node->Find("direction"), &Sync.Direction);
				Series::Unpack(Node->Find("rolloff"), &Sync.Rolloff);
				Series::Unpack(Node->Find("cone-inner-angle"), &Sync.ConeInnerAngle);
				Series::Unpack(Node->Find("cone-outer-angle"), &Sync.ConeOuterAngle);
				Series::Unpack(Node->Find("cone-outer-gain"), &Sync.ConeOuterGain);
				Series::Unpack(Node->Find("distance"), &Sync.Distance);
				Series::Unpack(Node->Find("gain"), &Sync.Gain);
				Series::Unpack(Node->Find("pitch"), &Sync.Pitch);
				Series::Unpack(Node->Find("ref-distance"), &Sync.RefDistance);
				Series::Unpack(Node->Find("position"), &Sync.Position);
				Series::Unpack(Node->Find("relative"), &Sync.IsRelative);
				Series::Unpack(Node->Find("looped"), &Sync.IsLooped);
				Series::Unpack(Node->Find("distance"), &Sync.Distance);
				Series::Unpack(Node->Find("air-absorption"), &Sync.AirAbsorption);
				Series::Unpack(Node->Find("room-roll-off"), &Sync.RoomRollOff);

				Core::String Path;
				if (!Series::Unpack(Node->Find("audio-clip"), &Path) || Path.empty())
					return;

				Node->AddRef();
				Parent->GetScene()->LoadResource<Audio::AudioClip>(this, Path, [this, Node](ExpectsContent<Audio::AudioClip*>&& NewClip)
				{
					Source->SetClip(NewClip.Or(nullptr));
					for (auto& Effect : Node->FetchCollection("effects.effect"))
					{
						uint64_t Id;
						if (!Series::Unpack(Effect->Find("id"), &Id))
							continue;

						Audio::AudioEffect* Target = Core::Composer::Create<Audio::AudioEffect>(Id);
						if (!Target)
							continue;

						Core::Schema* Meta = Effect->Find("metadata");
						if (!Meta)
							Meta = Effect->Set("metadata");

						Target->Deserialize(Meta);
						Source->AddEffect(Target);
					}

					bool Autoplay;
					if (Series::Unpack(Node->Find("autoplay"), &Autoplay) && Autoplay && Source->GetClip())
						Source->Play();

					ApplyPlayingPosition();
					Synchronize(nullptr);
					Node->Release();
				});
			}
			void AudioSource::Serialize(Core::Schema* Node)
			{
				VI_ASSERT(Node != nullptr, "schema should be set");

				Core::Schema* Effects = Node->Set("effects", Core::Var::Array());
				for (auto* Effect : Source->GetEffects())
				{
					Core::Schema* Element = Effects->Set("effect");
					Series::Pack(Element->Set("id"), Effect->GetId());
					Effect->Serialize(Element->Set("metadata"));
				}

				Series::Pack(Node->Set("audio-clip"), Parent->GetScene()->FindResourceId<Audio::AudioClip>(Source->GetClip()));
				HeavySeries::Pack(Node->Set("velocity"), Sync.Velocity);
				HeavySeries::Pack(Node->Set("direction"), Sync.Direction);
				Series::Pack(Node->Set("rolloff"), Sync.Rolloff);
				Series::Pack(Node->Set("cone-inner-angle"), Sync.ConeInnerAngle);
				Series::Pack(Node->Set("cone-outer-angle"), Sync.ConeOuterAngle);
				Series::Pack(Node->Set("cone-outer-gain"), Sync.ConeOuterGain);
				Series::Pack(Node->Set("distance"), Sync.Distance);
				Series::Pack(Node->Set("gain"), Sync.Gain);
				Series::Pack(Node->Set("pitch"), Sync.Pitch);
				Series::Pack(Node->Set("ref-distance"), Sync.RefDistance);
				Series::Pack(Node->Set("position"), Sync.Position);
				Series::Pack(Node->Set("relative"), Sync.IsRelative);
				Series::Pack(Node->Set("looped"), Sync.IsLooped);
				Series::Pack(Node->Set("distance"), Sync.Distance);
				Series::Pack(Node->Set("autoplay"), Source->IsPlaying());
				Series::Pack(Node->Set("air-absorption"), Sync.AirAbsorption);
				Series::Pack(Node->Set("room-roll-off"), Sync.RoomRollOff);
			}
			void AudioSource::Synchronize(Core::Timer* Time)
			{
				auto* Transform = Parent->GetTransform();
				if (Transform->IsDirty())
				{
					const Trigonometry::Vector3& Position = Transform->GetPosition();
					Sync.Velocity = (Position - LastPosition) * Time->GetStep();
					LastPosition = Position;
				}
				else
					Sync.Velocity = 0.0f;

				if (Source->GetClip() != nullptr)
					Source->Synchronize(&Sync, Transform->GetPosition());
			}
			void AudioSource::ApplyPlayingPosition()
			{
				Audio::AudioContext::SetSourceData1F(Source->GetInstance(), Audio::SoundEx::Seconds_Offset, Sync.Position);
			}
			Component* AudioSource::Copy(Entity* New) const
			{
				AudioSource* Target = new AudioSource(New);
				Target->LastPosition = LastPosition;
				Target->Source->SetClip(Source->GetClip());
				Target->Sync = Sync;

				for (auto* Effect : Source->GetEffects())
					Target->Source->AddEffect(Effect->Copy());

				return Target;
			}
			Audio::AudioSource* AudioSource::GetSource() const
			{
				return Source;
			}
			Audio::AudioSync& AudioSource::GetSync()
			{
				return Sync;
			}

			AudioListener::AudioListener(Entity* Ref) : Component(Ref, ActorSet::Synchronize)
			{
			}
			AudioListener::~AudioListener()
			{
				Deactivate();
			}
			void AudioListener::Deserialize(Core::Schema* Node)
			{
				VI_ASSERT(Node != nullptr, "schema should be set");
				Series::Unpack(Node->Find("gain"), &Gain);
			}
			void AudioListener::Serialize(Core::Schema* Node)
			{
				VI_ASSERT(Node != nullptr, "schema should be set");
				Series::Pack(Node->Set("gain"), Gain);
			}
			void AudioListener::Synchronize(Core::Timer* Time)
			{
				auto* Transform = Parent->GetTransform();
				if (Transform->IsDirty())
				{
					const Trigonometry::Vector3& Position = Transform->GetPosition();
					Trigonometry::Vector3 Velocity = (Position - LastPosition) * Time->GetStep();
					Trigonometry::Vector3 Rotation = Transform->GetRotation().dDirection();
					float LookAt[6] = { Rotation.X, Rotation.Y, Rotation.Z, 0.0f, 1.0f, 0.0f };
					LastPosition = Position;

					Audio::AudioContext::SetListenerData3F(Audio::SoundEx::Velocity, Velocity.X, Velocity.Y, Velocity.Z);
					Audio::AudioContext::SetListenerData3F(Audio::SoundEx::Position, -Position.X, -Position.Y, Position.Z);
					Audio::AudioContext::SetListenerDataVF(Audio::SoundEx::Orientation, LookAt);
				}

				Audio::AudioContext::SetListenerData1F(Audio::SoundEx::Gain, Gain);
			}
			void AudioListener::Deactivate()
			{
				float LookAt[6] = { 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f };
				Audio::AudioContext::SetListenerData3F(Audio::SoundEx::Velocity, 0.0f, 0.0f, 0.0f);
				Audio::AudioContext::SetListenerData3F(Audio::SoundEx::Position, 0.0f, 0.0f, 0.0f);
				Audio::AudioContext::SetListenerDataVF(Audio::SoundEx::Orientation, LookAt);
				Audio::AudioContext::SetListenerData1F(Audio::SoundEx::Gain, 0.0f);
			}
			Component* AudioListener::Copy(Entity* New) const
			{
				AudioListener* Target = new AudioListener(New);
				Target->LastPosition = LastPosition;
				Target->Gain = Gain;

				return Target;
			}

			PointLight::PointLight(Entity* Ref) : Component(Ref, ActorSet::Cullable)
			{
			}
			void PointLight::Deserialize(Core::Schema* Node)
			{
				VI_ASSERT(Node != nullptr, "schema should be set");
				HeavySeries::Unpack(Node->Find("projection"), &Projection);
				HeavySeries::Unpack(Node->Find("view"), &View);
				HeavySeries::Unpack(Node->Find("size"), &Size);
				HeavySeries::Unpack(Node->Find("diffuse"), &Diffuse);
				Series::Unpack(Node->Find("emission"), &Emission);
				Series::Unpack(Node->Find("disperse"), &Disperse);
				Series::Unpack(Node->Find("shadow-softness"), &Shadow.Softness);
				Series::Unpack(Node->Find("shadow-distance"), &Shadow.Distance);
				Series::Unpack(Node->Find("shadow-bias"), &Shadow.Bias);
				Series::Unpack(Node->Find("shadow-iterations"), &Shadow.Iterations);
				Series::Unpack(Node->Find("shadow-enabled"), &Shadow.Enabled);
			}
			void PointLight::Serialize(Core::Schema* Node)
			{
				VI_ASSERT(Node != nullptr, "schema should be set");
				HeavySeries::Pack(Node->Set("projection"), Projection);
				HeavySeries::Pack(Node->Set("view"), View);
				HeavySeries::Pack(Node->Set("size"), Size);
				HeavySeries::Pack(Node->Set("diffuse"), Diffuse);
				Series::Pack(Node->Set("emission"), Emission);
				Series::Pack(Node->Set("disperse"), Disperse);
				Series::Pack(Node->Set("shadow-softness"), Shadow.Softness);
				Series::Pack(Node->Set("shadow-distance"), Shadow.Distance);
				Series::Pack(Node->Set("shadow-bias"), Shadow.Bias);
				Series::Pack(Node->Set("shadow-iterations"), Shadow.Iterations);
				Series::Pack(Node->Set("shadow-enabled"), Shadow.Enabled);
			}
			void PointLight::Message(const std::string_view& Name, Core::VariantArgs& Args)
			{
				if (Name == "depth-flush")
					DepthMap = nullptr;
			}
			size_t PointLight::GetUnitBounds(Trigonometry::Vector3& Min, Trigonometry::Vector3& Max) const
			{
				Min = Size.Radius * -1.25f;
				Max = Size.Radius * 1.25f;
				return BOX_LIGHT;
			}
			float PointLight::GetVisibility(const Viewer& View, float Distance) const
			{
				return GetVisibilityRadius(Parent, View, Distance);
			}
			Component* PointLight::Copy(Entity* New) const
			{
				PointLight* Target = new PointLight(New);
				Target->Diffuse = Diffuse;
				Target->Emission = Emission;
				Target->Projection = Projection;
				Target->View = View;
				Target->Size = Size;
				memcpy(&Target->Shadow, &Shadow, sizeof(Shadow));

				return Target;
			}
			void PointLight::GenerateOrigin()
			{
				auto* Transform = Parent->GetTransform();
				View = Trigonometry::Matrix4x4::CreateTranslation(Transform->GetPosition());
				Projection = Trigonometry::Matrix4x4::CreatePerspective(90.0f, 1.0f, 0.1f, Shadow.Distance);
			}
			void PointLight::SetSize(const Attenuation& Value)
			{
				Size = Value;
				GetEntity()->GetTransform()->MakeDirty();
			}
			const Attenuation& PointLight::GetSize()
			{
				return Size;
			}

			SpotLight::SpotLight(Entity* Ref) : Component(Ref, ActorSet::Cullable | ActorSet::Synchronize)
			{
			}
			void SpotLight::Deserialize(Core::Schema* Node)
			{
				VI_ASSERT(Node != nullptr, "schema should be set");
				HeavySeries::Unpack(Node->Find("projection"), &Projection);
				HeavySeries::Unpack(Node->Find("view"), &View);
				HeavySeries::Unpack(Node->Find("size"), &Size);
				HeavySeries::Unpack(Node->Find("diffuse"), &Diffuse);
				Series::Unpack(Node->Find("emission"), &Emission);
				Series::Unpack(Node->Find("disperse"), &Disperse);
				Series::Unpack(Node->Find("cutoff"), &Cutoff);
				Series::Unpack(Node->Find("shadow-bias"), &Shadow.Bias);
				Series::Unpack(Node->Find("shadow-distance"), &Shadow.Distance);
				Series::Unpack(Node->Find("shadow-softness"), &Shadow.Softness);
				Series::Unpack(Node->Find("shadow-iterations"), &Shadow.Iterations);
				Series::Unpack(Node->Find("shadow-enabled"), &Shadow.Enabled);
			}
			void SpotLight::Serialize(Core::Schema* Node)
			{
				VI_ASSERT(Node != nullptr, "schema should be set");
				HeavySeries::Pack(Node->Set("projection"), Projection);
				HeavySeries::Pack(Node->Set("view"), View);
				HeavySeries::Pack(Node->Set("size"), Size);
				HeavySeries::Pack(Node->Set("diffuse"), Diffuse);
				Series::Pack(Node->Set("emission"), Emission);
				Series::Pack(Node->Set("disperse"), Disperse);
				Series::Pack(Node->Set("cutoff"), Cutoff);
				Series::Pack(Node->Set("shadow-bias"), Shadow.Bias);
				Series::Pack(Node->Set("shadow-distance"), Shadow.Distance);
				Series::Pack(Node->Set("shadow-softness"), Shadow.Softness);
				Series::Pack(Node->Set("shadow-iterations"), Shadow.Iterations);
				Series::Pack(Node->Set("shadow-enabled"), Shadow.Enabled);
			}
			void SpotLight::Message(const std::string_view& Name, Core::VariantArgs& Args)
			{
				if (Name == "depth-flush")
					DepthMap = nullptr;
			}
			void SpotLight::Synchronize(Core::Timer* Time)
			{
				Cutoff = Compute::Mathf::Clamp(Cutoff, 0.0f, 180.0f);
			}
			size_t SpotLight::GetUnitBounds(Trigonometry::Vector3& Min, Trigonometry::Vector3& Max) const
			{
				Min = Size.Radius * -1.25f;
				Max = Size.Radius * 1.25f;
				return BOX_LIGHT;
			}
			float SpotLight::GetVisibility(const Viewer& View, float Distance) const
			{
				return GetVisibilityRadius(Parent, View, Distance);
			}
			Component* SpotLight::Copy(Entity* New) const
			{
				SpotLight* Target = new SpotLight(New);
				Target->Diffuse = Diffuse;
				Target->Projection = Projection;
				Target->View = View;
				Target->Size = Size;
				Target->Cutoff = Cutoff;
				Target->Emission = Emission;
				memcpy(&Target->Shadow, &Shadow, sizeof(Shadow));

				return Target;
			}
			void SpotLight::GenerateOrigin()
			{
				auto* Transform = Parent->GetTransform();
				auto& Space = Transform->GetSpacing(Trigonometry::Positioning::Global);
				View = Trigonometry::Matrix4x4::CreateView(Space.Position, Space.Rotation.InvY());
				Projection = Trigonometry::Matrix4x4::CreatePerspective(Cutoff, 1, 0.1f, Shadow.Distance);
			}
			void SpotLight::SetSize(const Attenuation& Value)
			{
				Size = Value;
				GetEntity()->GetTransform()->MakeDirty();
			}
			const Attenuation& SpotLight::GetSize()
			{
				return Size;
			}

			LineLight::LineLight(Entity* Ref) : Component(Ref, ActorSet::Cullable)
			{
			}
			void LineLight::Deserialize(Core::Schema* Node)
			{
				VI_ASSERT(Node != nullptr, "schema should be set");
				HeavySeries::Unpack(Node->Find("diffuse"), &Diffuse);
				Series::Unpack(Node->Find("emission"), &Emission);
				Series::Unpack(Node->Find("disperse"), &Disperse);

				for (uint32_t i = 0; i < 6; i++)
				{
					HeavySeries::Unpack(Node->Find("projection-" + Core::ToString(i)), &Projection[i]);
					HeavySeries::Unpack(Node->Find("view-" + Core::ToString(i)), &View[i]);
				}

				for (uint32_t i = 0; i < 6; i++)
					Series::Unpack(Node->Find("shadow-distance-" + Core::ToString(i)), &Shadow.Distance[i]);

				Series::Unpack(Node->Find("shadow-cascades"), &Shadow.Cascades);
				Series::Unpack(Node->Find("shadow-far"), &Shadow.Far);
				Series::Unpack(Node->Find("shadow-near"), &Shadow.Near);
				Series::Unpack(Node->Find("shadow-bias"), &Shadow.Bias);
				Series::Unpack(Node->Find("shadow-softness"), &Shadow.Softness);
				Series::Unpack(Node->Find("shadow-iterations"), &Shadow.Iterations);
				Series::Unpack(Node->Find("shadow-enabled"), &Shadow.Enabled);
				HeavySeries::Unpack(Node->Find("rlh-emission"), &Sky.RlhEmission);
				HeavySeries::Unpack(Node->Find("mie-emission"), &Sky.MieEmission);
				Series::Unpack(Node->Find("rlh-height"), &Sky.RlhHeight);
				HeavySeries::Unpack(Node->Find("mie-height"), &Sky.MieEmission);
				Series::Unpack(Node->Find("mie-direction"), &Sky.MieDirection);
				Series::Unpack(Node->Find("inner-radius"), &Sky.InnerRadius);
				Series::Unpack(Node->Find("outer-radius"), &Sky.OuterRadius);
				Series::Unpack(Node->Find("sky-intensity"), &Sky.Intensity);
			}
			void LineLight::Serialize(Core::Schema* Node)
			{
				VI_ASSERT(Node != nullptr, "schema should be set");
				HeavySeries::Pack(Node->Set("diffuse"), Diffuse);
				Series::Pack(Node->Set("emission"), Emission);
				Series::Pack(Node->Set("disperse"), Disperse);

				for (uint32_t i = 0; i < 6; i++)
				{
					HeavySeries::Pack(Node->Set("projection-" + Core::ToString(i)), Projection[i]);
					HeavySeries::Pack(Node->Set("view-" + Core::ToString(i)), View[i]);
				}

				for (uint32_t i = 0; i < 6; i++)
					Series::Pack(Node->Set("shadow-distance-" + Core::ToString(i)), Shadow.Distance[i]);

				Series::Pack(Node->Set("shadow-cascades"), Shadow.Cascades);
				Series::Pack(Node->Set("shadow-far"), Shadow.Far);
				Series::Pack(Node->Set("shadow-near"), Shadow.Near);
				Series::Pack(Node->Set("shadow-bias"), Shadow.Bias);
				Series::Pack(Node->Set("shadow-softness"), Shadow.Softness);
				Series::Pack(Node->Set("shadow-iterations"), Shadow.Iterations);
				Series::Pack(Node->Set("shadow-enabled"), Shadow.Enabled);
				HeavySeries::Pack(Node->Set("rlh-emission"), Sky.RlhEmission);
				HeavySeries::Pack(Node->Set("mie-emission"), Sky.MieEmission);
				Series::Pack(Node->Set("rlh-height"), Sky.RlhHeight);
				HeavySeries::Pack(Node->Set("mie-height"), Sky.MieEmission);
				Series::Pack(Node->Set("mie-direction"), Sky.MieDirection);
				Series::Pack(Node->Set("inner-radius"), Sky.InnerRadius);
				Series::Pack(Node->Set("outer-radius"), Sky.OuterRadius);
				Series::Pack(Node->Set("sky-intensity"), Sky.Intensity);
			}
			void LineLight::Message(const std::string_view& Name, Core::VariantArgs& Args)
			{
				if (Name == "depth-flush")
					DepthMap = nullptr;
			}
			Component* LineLight::Copy(Entity* New) const
			{
				LineLight* Target = new LineLight(New);
				Target->Diffuse = Diffuse;
				Target->Emission = Emission;
				memcpy(Target->Projection, Projection, sizeof(Trigonometry::Matrix4x4) * 6);
				memcpy(Target->View, View, sizeof(Trigonometry::Matrix4x4) * 6);
				memcpy(&Target->Shadow, &Shadow, sizeof(Shadow));
				memcpy(&Target->Sky, &Sky, sizeof(Sky));

				return Target;
			}
			void LineLight::GenerateOrigin()
			{
				auto* Viewer = (Components::Camera*)Parent->GetScene()->GetCamera();
				Trigonometry::Vector3 Direction = Parent->GetTransform()->GetPosition().sNormalize();
				Trigonometry::Vector3 Position = Viewer->GetEntity()->GetTransform()->GetPosition();
				Trigonometry::Matrix4x4 LightView = Trigonometry::Matrix4x4::CreateLookAt(Position, Position - Direction, Trigonometry::Vector3::Up());
				Trigonometry::Matrix4x4 ViewToLight = Viewer->GetView().Inv() * LightView;
				float FieldOfView = Compute::Mathf::Deg2Rad() * Viewer->FieldOfView;
				float Aspect = Viewer->GetAspect();

				if (Shadow.Cascades > 6)
					return;

				for (uint32_t i = 0; i < Shadow.Cascades; i++)
				{
					float Near = (i < 1 ? 0.1f : Shadow.Distance[i - 1]), Far = Shadow.Distance[i];
					Trigonometry::Frustum8C Frustum(FieldOfView, Aspect, Near, Far);
					Frustum.Transform(ViewToLight);

					Trigonometry::Vector2 X, Y, Z;
					Frustum.GetBoundingBox(&X, &Y, &Z);

					if (Z.X > -1.0f)
						Z.X = -1.0f;
					if (Z.Y < 1.0f)
						Z.Y = 1.0f;

					Projection[i] = Trigonometry::Matrix4x4::CreateOrthographicOffCenter(X.X, X.Y, Y.X, Y.Y, Z.X * Shadow.Near, Z.Y * Shadow.Far);
					View[i] = LightView;
				}
			}

			SurfaceLight::SurfaceLight(Entity* Ref) : Component(Ref, ActorSet::Cullable), Projection(Trigonometry::Matrix4x4::CreatePerspective(90.0f, 1, 0.01f, 100.0f))
			{
			}
			SurfaceLight::~SurfaceLight()
			{
				Core::Memory::Release(DiffuseMapX[0]);
				Core::Memory::Release(DiffuseMapX[1]);
				Core::Memory::Release(DiffuseMapY[0]);
				Core::Memory::Release(DiffuseMapY[1]);
				Core::Memory::Release(DiffuseMapZ[0]);
				Core::Memory::Release(DiffuseMapZ[1]);
				Core::Memory::Release(DiffuseMap);
				Core::Memory::Release(Probe);
			}
			void SurfaceLight::Deserialize(Core::Schema* Node)
			{
				VI_ASSERT(Node != nullptr, "schema should be set");

				auto* Scene = Parent->GetScene(); Core::String Path;
				if (!Series::Unpack(Node->Find("diffuse-map"), &Path) || Path.empty())
				{
					if (Series::Unpack(Node->Find("diffuse-map-px"), &Path))
					{
						Scene->LoadResource<Graphics::Texture2D>(this, Path, [this](ExpectsContent<Graphics::Texture2D*>&& NewTexture)
						{
							Core::Memory::Release(DiffuseMapX[0]);
							if (NewTexture && *NewTexture != nullptr)
							{
								DiffuseMapX[0] = *NewTexture;
								DiffuseMapX[0]->AddRef();
							}
						});
					}

					if (Series::Unpack(Node->Find("diffuse-map-nx"), &Path))
					{
						Scene->LoadResource<Graphics::Texture2D>(this, Path, [this](ExpectsContent<Graphics::Texture2D*>&& NewTexture)
						{
							Core::Memory::Release(DiffuseMapX[1]);
							if (NewTexture && *NewTexture != nullptr)
							{
								DiffuseMapX[1] = *NewTexture;
								DiffuseMapX[1]->AddRef();
							}
						});
					}

					if (Series::Unpack(Node->Find("diffuse-map-py"), &Path))
					{
						Scene->LoadResource<Graphics::Texture2D>(this, Path, [this](ExpectsContent<Graphics::Texture2D*>&& NewTexture)
						{
							Core::Memory::Release(DiffuseMapY[0]);
							if (NewTexture && *NewTexture != nullptr)
							{
								DiffuseMapY[0] = *NewTexture;
								DiffuseMapY[0]->AddRef();
							}
						});
					}

					if (Series::Unpack(Node->Find("diffuse-map-ny"), &Path))
					{
						Scene->LoadResource<Graphics::Texture2D>(this, Path, [this](ExpectsContent<Graphics::Texture2D*>&& NewTexture)
						{
							Core::Memory::Release(DiffuseMapY[1]);
							if (NewTexture && *NewTexture != nullptr)
							{
								DiffuseMapY[1] = *NewTexture;
								DiffuseMapY[1]->AddRef();
							}
						});
					}

					if (Series::Unpack(Node->Find("diffuse-map-pz"), &Path))
					{
						Scene->LoadResource<Graphics::Texture2D>(this, Path, [this](ExpectsContent<Graphics::Texture2D*>&& NewTexture)
						{
							Core::Memory::Release(DiffuseMapZ[0]);
							if (NewTexture && *NewTexture != nullptr)
							{
								DiffuseMapZ[0] = *NewTexture;
								DiffuseMapZ[0]->AddRef();
							}
						});
					}

					if (Series::Unpack(Node->Find("diffuse-map-nz"), &Path))
					{
						Scene->LoadResource<Graphics::Texture2D>(this, Path, [this](ExpectsContent<Graphics::Texture2D*>&& NewTexture)
						{
							Core::Memory::Release(DiffuseMapZ[1]);
							if (NewTexture && *NewTexture != nullptr)
							{
								DiffuseMapZ[1] = *NewTexture;
								DiffuseMapZ[1]->AddRef();
							}
						});
					}
				}
				else
				{
					Scene->LoadResource<Graphics::Texture2D>(this, Path, [this](ExpectsContent<Graphics::Texture2D*>&& NewTexture)
					{
						Core::Memory::Release(DiffuseMap);
						if (NewTexture && *NewTexture != nullptr)
						{
							DiffuseMap = *NewTexture;
							DiffuseMap->AddRef();
						}
					});
				}

				Core::Vector<Trigonometry::Matrix4x4> Views;
				HeavySeries::Unpack(Node->Find("projection"), &Projection);
				HeavySeries::Unpack(Node->Find("view"), &Views);
				HeavySeries::Unpack(Node->Find("tick"), &Tick);
				HeavySeries::Unpack(Node->Find("diffuse"), &Diffuse);
				HeavySeries::Unpack(Node->Find("size"), &Size);
				Series::Unpack(Node->Find("emission"), &Emission);
				Series::Unpack(Node->Find("infinity"), &Infinity);
				Series::Unpack(Node->Find("parallax"), &Parallax);
				Series::Unpack(Node->Find("static-mask"), &StaticMask);

				size_t Count = Compute::Math<size_t>::Min(Views.size(), 6);
				for (size_t i = 0; i < Count; i++)
					View[i] = Views[i];

				if (!DiffuseMap)
					SetDiffuseMap(DiffuseMapX, DiffuseMapY, DiffuseMapZ);
				else
					SetDiffuseMap(DiffuseMap);
			}
			void SurfaceLight::Serialize(Core::Schema* Node)
			{
				VI_ASSERT(Node != nullptr, "schema should be set");

				auto* Scene = Parent->GetScene();
				if (!DiffuseMap)
				{
					Series::Pack(Node->Set("diffuse-map-px"), Scene->FindResourceId<Graphics::Texture2D>(DiffuseMapX[0]));
					Series::Pack(Node->Set("diffuse-map-nx"), Scene->FindResourceId<Graphics::Texture2D>(DiffuseMapX[1]));
					Series::Pack(Node->Set("diffuse-map-py"), Scene->FindResourceId<Graphics::Texture2D>(DiffuseMapY[0]));
					Series::Pack(Node->Set("diffuse-map-ny"), Scene->FindResourceId<Graphics::Texture2D>(DiffuseMapY[1]));
					Series::Pack(Node->Set("diffuse-map-pz"), Scene->FindResourceId<Graphics::Texture2D>(DiffuseMapZ[0]));
					Series::Pack(Node->Set("diffuse-map-nz"), Scene->FindResourceId<Graphics::Texture2D>(DiffuseMapZ[1]));
				}
				else
					Series::Pack(Node->Set("diffuse-map"), Scene->FindResourceId<Graphics::Texture2D>(DiffuseMap));

				Core::Vector<Trigonometry::Matrix4x4> Views;
				for (int64_t i = 0; i < 6; i++)
					Views.push_back(View[i]);

				HeavySeries::Pack(Node->Set("projection"), Projection);
				HeavySeries::Pack(Node->Set("view"), Views);
				HeavySeries::Pack(Node->Set("tick"), Tick);
				HeavySeries::Pack(Node->Set("size"), Size);
				HeavySeries::Pack(Node->Set("diffuse"), Diffuse);
				Series::Pack(Node->Set("emission"), Emission);
				Series::Pack(Node->Set("infinity"), Infinity);
				Series::Pack(Node->Set("parallax"), Parallax);
				Series::Pack(Node->Set("static-mask"), StaticMask);
			}
			size_t SurfaceLight::GetUnitBounds(Trigonometry::Vector3& Min, Trigonometry::Vector3& Max) const
			{
				Min = Size.Radius * -1.25f;
				Max = Size.Radius * 1.25f;
				return BOX_LIGHT;
			}
			float SurfaceLight::GetVisibility(const Viewer& View, float Distance) const
			{
				if (Infinity > 0.0f)
					return 1.0f;

				return GetVisibilityRadius(Parent, View, Distance);
			}
			Component* SurfaceLight::Copy(Entity* New) const
			{
				SurfaceLight* Target = new SurfaceLight(New);
				Target->Projection = Projection;
				Target->Diffuse = Diffuse;
				Target->Emission = Emission;
				Target->Size = Size;
				Target->Tick = Tick;
				memcpy(Target->View, View, 6 * sizeof(Trigonometry::Matrix4x4));

				if (!DiffuseMap)
					Target->SetDiffuseMap(DiffuseMapX, DiffuseMapY, DiffuseMapZ);
				else
					Target->SetDiffuseMap(DiffuseMap);

				return Target;
			}
			void SurfaceLight::SetProbeCache(Graphics::TextureCube* NewCache)
			{
				Probe = NewCache;
			}
			void SurfaceLight::SetSize(const Attenuation& Value)
			{
				Size = Value;
				GetEntity()->GetTransform()->MakeDirty();
			}
			bool SurfaceLight::SetDiffuseMap(Graphics::Texture2D* Map)
			{
				VI_ASSERT(Parent->GetScene()->GetDevice() != nullptr, "graphics device should be set");
				Core::Memory::Release(DiffuseMapX[0]);
				Core::Memory::Release(DiffuseMapX[1]);
				Core::Memory::Release(DiffuseMapY[0]);
				Core::Memory::Release(DiffuseMapY[1]);
				Core::Memory::Release(DiffuseMapZ[0]);
				Core::Memory::Release(DiffuseMapZ[1]);
				Core::Memory::Release(DiffuseMap);

				DiffuseMap = Map;
				if (!DiffuseMap)
					return false;

				DiffuseMap->AddRef();
				Core::Memory::Release(Probe);
				Probe = Parent->GetScene()->GetDevice()->CreateTextureCube(DiffuseMap).Or(nullptr);
				return Probe != nullptr;
			}
			bool SurfaceLight::SetDiffuseMap(Graphics::Texture2D* const MapX[2], Graphics::Texture2D* const MapY[2], Graphics::Texture2D* const MapZ[2])
			{
				VI_ASSERT(Parent->GetScene()->GetDevice() != nullptr, "graphics device should be set");
				Core::Memory::Release(DiffuseMapX[0]);
				Core::Memory::Release(DiffuseMapX[1]);
				Core::Memory::Release(DiffuseMapY[0]);
				Core::Memory::Release(DiffuseMapY[1]);
				Core::Memory::Release(DiffuseMapZ[0]);
				Core::Memory::Release(DiffuseMapZ[1]);
				Core::Memory::Release(DiffuseMap);
				if (!MapX[0] || !MapX[1] || !MapY[0] || !MapY[1] || !MapZ[0] || !MapZ[1])
					return false;

				Graphics::Texture2D* Resources[6];
				Resources[0] = DiffuseMapX[0] = MapX[0]; MapX[0]->AddRef();
				Resources[1] = DiffuseMapX[1] = MapX[1]; MapX[1]->AddRef();
				Resources[2] = DiffuseMapY[0] = MapY[0]; MapY[0]->AddRef();
				Resources[3] = DiffuseMapY[1] = MapY[1]; MapY[1]->AddRef();
				Resources[4] = DiffuseMapZ[0] = MapZ[0]; MapZ[0]->AddRef();
				Resources[5] = DiffuseMapZ[1] = MapZ[1]; MapZ[1]->AddRef();

				Core::Memory::Release(Probe);
				Probe = Parent->GetScene()->GetDevice()->CreateTextureCube(Resources).Or(nullptr);
				return Probe != nullptr;
			}
			bool SurfaceLight::IsImageBased() const
			{
				return DiffuseMapX[0] != nullptr || DiffuseMap != nullptr;
			}
			const Attenuation& SurfaceLight::GetSize()
			{
				return Size;
			}
			Graphics::TextureCube* SurfaceLight::GetProbeCache() const
			{
				return Probe;
			}
			Graphics::Texture2D* SurfaceLight::GetDiffuseMapXP()
			{
				return DiffuseMapX[0];
			}
			Graphics::Texture2D* SurfaceLight::GetDiffuseMapXN()
			{
				return DiffuseMapX[1];
			}
			Graphics::Texture2D* SurfaceLight::GetDiffuseMapYP()
			{
				return DiffuseMapY[0];
			}
			Graphics::Texture2D* SurfaceLight::GetDiffuseMapYN()
			{
				return DiffuseMapY[1];
			}
			Graphics::Texture2D* SurfaceLight::GetDiffuseMapZP()
			{
				return DiffuseMapZ[0];
			}
			Graphics::Texture2D* SurfaceLight::GetDiffuseMapZN()
			{
				return DiffuseMapZ[1];
			}
			Graphics::Texture2D* SurfaceLight::GetDiffuseMap()
			{
				return DiffuseMap;
			}

			Illuminator::Illuminator(Entity* Ref) : Component(Ref, ActorSet::Cullable), VoxelMap(nullptr), Regenerate(true)
			{
				Inside.Delay = 30.0;
				Outside.Delay = 10000.0;
				RayStep = 0.5f;
				MaxSteps = 256.0f;
				Distance = 12.0f;
				Radiance = 1.0f;
				Occlusion = 0.33f;
				Specular = 2.0f;
				Length = 1.0f;
				Margin = 3.828424f;
				Offset = -0.01f;
				Angle = 0.5;
				Bleeding = 0.33f;
			}
			void Illuminator::Deserialize(Core::Schema* Node)
			{
				VI_ASSERT(Node != nullptr, "schema should be set");

				Series::Unpack(Node->Find("inside-delay"), &Inside.Delay);
				Series::Unpack(Node->Find("outside-delay"), &Outside.Delay);
				Series::Unpack(Node->Find("ray-step"), &RayStep);
				Series::Unpack(Node->Find("max-steps"), &MaxSteps);
				Series::Unpack(Node->Find("distance"), &Distance);
				Series::Unpack(Node->Find("radiance"), &Radiance);
				Series::Unpack(Node->Find("length"), &Length);
				Series::Unpack(Node->Find("margin"), &Margin);
				Series::Unpack(Node->Find("offset"), &Offset);
				Series::Unpack(Node->Find("angle"), &Angle);
				Series::Unpack(Node->Find("occlusion"), &Occlusion);
				Series::Unpack(Node->Find("specular"), &Specular);
				Series::Unpack(Node->Find("bleeding"), &Bleeding);
			}
			void Illuminator::Serialize(Core::Schema* Node)
			{
				VI_ASSERT(Node != nullptr, "schema should be set");

				Series::Pack(Node->Set("inside-delay"), Inside.Delay);
				Series::Pack(Node->Set("outside-delay"), Outside.Delay);
				Series::Pack(Node->Set("ray-step"), RayStep);
				Series::Pack(Node->Set("max-steps"), MaxSteps);
				Series::Pack(Node->Set("distance"), Distance);
				Series::Pack(Node->Set("radiance"), Radiance);
				Series::Pack(Node->Set("length"), Length);
				Series::Pack(Node->Set("margin"), Margin);
				Series::Pack(Node->Set("offset"), Offset);
				Series::Pack(Node->Set("angle"), Angle);
				Series::Pack(Node->Set("occlusion"), Occlusion);
				Series::Pack(Node->Set("specular"), Specular);
				Series::Pack(Node->Set("bleeding"), Bleeding);
			}
			void Illuminator::Message(const std::string_view& Name, Core::VariantArgs& Args)
			{
				if (Name == "depth-flush")
				{
					VoxelMap = nullptr;
					Regenerate = true;
				}
			}
			Component* Illuminator::Copy(Entity* New) const
			{
				Illuminator* Target = new Illuminator(New);
				Target->Inside = Inside;
				Target->Outside = Outside;
				Target->RayStep = RayStep;
				Target->MaxSteps = MaxSteps;
				Target->Radiance = Radiance;
				Target->Length = Length;
				Target->Occlusion = Occlusion;
				Target->Specular = Specular;

				return Target;
			}

			Camera::Camera(Entity* Ref) : Component(Ref, ActorSet::Synchronize), Mode(ProjectionMode::Perspective), Renderer(new RenderSystem(Ref->GetScene(), this)), Viewport({ 0, 0, 512, 512, 0, 1 })
			{
			}
			Camera::~Camera()
			{
				Core::Memory::Release(Renderer);
			}
			void Camera::Activate(Component* New)
			{
				VI_ASSERT(Parent->GetScene()->GetDevice() != nullptr, "graphics device should be set");
				VI_ASSERT(Parent->GetScene()->GetDevice()->GetRenderTarget() != nullptr, "render target should be set");

				SceneGraph* Scene = Parent->GetScene();
				Viewport = Scene->GetDevice()->GetRenderTarget()->GetViewport();
				if (New == this)
					Renderer->Remount();
			}
			void Camera::Deserialize(Core::Schema* Node)
			{
				VI_ASSERT(Node != nullptr, "schema should be set");
				VI_ASSERT(Parent->GetScene()->GetDevice() != nullptr, "graphics device should be set");

				int _Mode = 0;
				if (Series::Unpack(Node->Find("mode"), &_Mode))
					Mode = (ProjectionMode)_Mode;

				HeavySeries::Unpack(Node->Find("projection"), &Projection);
				Series::Unpack(Node->Find("field-of-view"), &FieldOfView);
				Series::Unpack(Node->Find("far-plane"), &FarPlane);
				Series::Unpack(Node->Find("near-plane"), &NearPlane);
				Series::Unpack(Node->Find("width"), &Width);
				Series::Unpack(Node->Find("height"), &Height);
				Series::Unpack(Node->Find("occluder-skips"), &Renderer->OccluderSkips);
				Series::Unpack(Node->Find("occludee-skips"), &Renderer->OccludeeSkips);
				Series::Unpack(Node->Find("occlusion-skips"), &Renderer->OcclusionSkips);
				Series::Unpack(Node->Find("occlusion-cull"), &Renderer->OcclusionCulling);
				Series::Unpack(Node->Find("occludee-scaling"), &Renderer->OccludeeScaling);
				Series::Unpack(Node->Find("max-queries"), &Renderer->MaxQueries);

				Core::Vector<Core::Schema*> Renderers = Node->FetchCollection("renderers.renderer");
				for (auto& Render : Renderers)
				{
					uint64_t Id;
					if (!Series::Unpack(Render->Find("id"), &Id))
						continue;

					Layer::Renderer* Target = Core::Composer::Create<Layer::Renderer>(Id, Renderer);
					if (!Renderer->AddRenderer(Target))
						continue;

					Core::Schema* Meta = Render->Find("metadata");
					if (!Meta)
						Meta = Render->Set("metadata");

					Target->Deactivate();
					Target->Deserialize(Meta);
					Target->Activate();
					Series::Unpack(Render->Find("active"), &Target->Active);
				}
			}
			void Camera::Serialize(Core::Schema* Node)
			{
				VI_ASSERT(Node != nullptr, "schema should be set");

				Series::Pack(Node->Set("mode"), (int)Mode);
				HeavySeries::Pack(Node->Set("projection"), Projection);
				Series::Pack(Node->Set("field-of-view"), FieldOfView);
				Series::Pack(Node->Set("far-plane"), FarPlane);
				Series::Pack(Node->Set("near-plane"), NearPlane);
				Series::Pack(Node->Set("width"), Width);
				Series::Pack(Node->Set("height"), Height);
				Series::Pack(Node->Set("occluder-skips"), Renderer->OccluderSkips);
				Series::Pack(Node->Set("occludee-skips"), Renderer->OccludeeSkips);
				Series::Pack(Node->Set("occlusion-skips"), Renderer->OcclusionSkips);
				Series::Pack(Node->Set("occlusion-cull"), Renderer->OcclusionCulling);
				Series::Pack(Node->Set("occludee-scaling"), Renderer->OccludeeScaling);
				Series::Pack(Node->Set("max-queries"), Renderer->MaxQueries);

				Core::Schema* Renderers = Node->Set("renderers", Core::Var::Array());
				for (auto* Next : Renderer->GetRenderers())
				{
					Core::Schema* Render = Renderers->Set("renderer");
					Series::Pack(Render->Set("id"), Next->GetId());
					Series::Pack(Render->Set("active"), Next->Active);
					Next->Serialize(Render->Set("metadata"));
				}
			}
			void Camera::Synchronize(Core::Timer* Time)
			{
				float W = Width, H = Height;
				if (W <= 0 || H <= 0)
				{
					W = Viewport.Width;
					H = Viewport.Height;
				}

				if (Mode == ProjectionMode::Perspective)
					Projection = Trigonometry::Matrix4x4::CreatePerspective(FieldOfView, W / H, NearPlane, FarPlane);
				else if (Mode == ProjectionMode::Orthographic)
					Projection = Trigonometry::Matrix4x4::CreateOrthographic(W, H, NearPlane, FarPlane);
			}
			void Camera::GetViewer(Viewer* Output)
			{
				VI_ASSERT(Output != nullptr, "viewer should be set");

				auto& Space = Parent->GetTransform()->GetSpacing(Trigonometry::Positioning::Global);
				RenderCulling Culling = (Mode == ProjectionMode::Perspective ? RenderCulling::Linear : RenderCulling::Disable);

				Output->Set(GetView(), Projection, Space.Position, Space.Rotation, FieldOfView, GetAspect(), NearPlane, FarPlane, Culling);
				Output->Renderer = Renderer;
				View = *Output;
			}
			void Camera::ResizeBuffers()
			{
				SceneGraph* Scene = Parent->GetScene();
				Viewport = Scene->GetDevice()->GetRenderTarget()->GetViewport();
				for (auto* Next : Renderer->GetRenderers())
					Next->ResizeBuffers();
			}
			Viewer& Camera::GetViewer()
			{
				return View;
			}
			RenderSystem* Camera::GetRenderer()
			{
				return Renderer;
			}
			Trigonometry::Matrix4x4 Camera::GetProjection()
			{
				return Projection;
			}
			Trigonometry::Vector3 Camera::GetViewPosition()
			{
				auto* Transform = Parent->GetTransform();
				return Transform->GetPosition().InvX().InvY();
			}
			Trigonometry::Matrix4x4 Camera::GetViewProjection()
			{
				return GetView() * Projection;
			}
			Trigonometry::Matrix4x4 Camera::GetView()
			{
				auto& Space = Parent->GetTransform()->GetSpacing(Trigonometry::Positioning::Global);
				return Trigonometry::Matrix4x4::CreateView(Space.Position, Space.Rotation);
			}
			Trigonometry::Frustum8C Camera::GetFrustum8C()
			{
				Trigonometry::Frustum8C Result(Compute::Mathf::Deg2Rad() * FieldOfView, GetAspect(), NearPlane, FarPlane * 0.25f);
				Result.Transform(GetView().Inv());
				return Result;
			}
			Trigonometry::Frustum6P Camera::GetFrustum6P()
			{
				Trigonometry::Frustum6P Result(GetViewProjection());
				return Result;
			}
			Trigonometry::Ray Camera::GetScreenRay(const Trigonometry::Vector2& Position)
			{
				float W = Width, H = Height;
				if (W <= 0 || H <= 0)
				{
					Graphics::Viewport V = Renderer->GetDevice()->GetRenderTarget()->GetViewport();
					W = V.Width; H = V.Height;
				}

				return Trigonometry::Geometric::CreateCursorRay(Parent->GetTransform()->GetPosition(), Position, Trigonometry::Vector2(W, H), Projection.Inv(), GetView().Inv());
			}
			Trigonometry::Ray Camera::GetCursorRay()
			{
				auto* Activity = Parent->GetScene()->GetActivity();
				if (!Activity)
					return Trigonometry::Ray();

				return GetScreenRay(Activity->GetCursorPosition());
			}
			float Camera::GetDistance(Entity* Other)
			{
				VI_ASSERT(Other != nullptr, "other should be set");
				return Other->GetTransform()->GetPosition().Distance(View.Position);
			}
			float Camera::GetWidth()
			{
				float W = Width;
				if (W <= 0)
					W = Viewport.Width;

				return W;
			}
			float Camera::GetHeight()
			{
				float H = Height;
				if (H <= 0)
					H = Viewport.Height;

				return H;
			}
			float Camera::GetAspect()
			{
				float W = Width, H = Height;
				if (W <= 0 || H <= 0)
				{
					W = Viewport.Width;
					H = Viewport.Height;
				}

				return W / H;
			}
			bool Camera::RayTest(const Trigonometry::Ray& Ray, Entity* Other, Trigonometry::Vector3* Hit)
			{
				VI_ASSERT(Other != nullptr, "other should be set");
				return Trigonometry::Geometric::CursorRayTest(Ray, Other->GetTransform()->GetBias(), Hit);
			}
			bool Camera::RayTest(const Trigonometry::Ray& Ray, const Trigonometry::Matrix4x4& World, Trigonometry::Vector3* Hit)
			{
				return Trigonometry::Geometric::CursorRayTest(Ray, World, Hit);
			}
			Component* Camera::Copy(Entity* New) const
			{
				Camera* Target = new Camera(New);
				Target->FarPlane = FarPlane;
				Target->NearPlane = NearPlane;
				Target->Width = Width;
				Target->Height = Height;
				Target->Mode = Mode;
				Target->FieldOfView = FieldOfView;
				Target->Projection = Projection;

				return Target;
			}

			Scriptable::Scriptable(Entity* Ref) : Component(Ref, ActorSet::Update | ActorSet::Message), Compiler(nullptr), Source(SourceType::Resource), Invoke(InvokeType::Typeless)
			{
			}
			Scriptable::~Scriptable()
			{
				Core::Memory::Release(Compiler);
			}
			void Scriptable::Deserialize(Core::Schema* Node)
			{
				VI_ASSERT(Node != nullptr, "schema should be set");

				Core::String Type;
				if (Series::Unpack(Node->Find("source"), &Type))
				{
					if (Type == "memory")
						Source = SourceType::Memory;
					else if (Type == "resource")
						Source = SourceType::Resource;
				}

				if (Series::Unpack(Node->Find("invoke"), &Type))
				{
					if (Type == "typeless")
						Invoke = InvokeType::Typeless;
					else if (Type == "normal")
						Invoke = InvokeType::Normal;
				}

				if (!Series::Unpack(Node->Find("resource"), &Resource) || Resource.empty())
					return;

				auto Success = LoadSource().Get();
				if (Success)
					DeserializeCall(Node).Wait();
			}
			void Scriptable::Serialize(Core::Schema* Node)
			{
				VI_ASSERT(Node != nullptr, "schema should be set");

				if (Source == SourceType::Memory)
					Series::Pack(Node->Set("source"), "memory");
				else if (Source == SourceType::Resource)
					Series::Pack(Node->Set("source"), "resource");

				if (Invoke == InvokeType::Typeless)
					Series::Pack(Node->Set("invoke"), "typeless");
				else if (Invoke == InvokeType::Normal)
					Series::Pack(Node->Set("invoke"), "normal");

				Series::Pack(Node->Set("resource"), Parent->GetScene()->AsResourcePath(Resource));
				SerializeCall(Node).Wait();
			}
			void Scriptable::Activate(Component* New)
			{
				if (!Parent->GetScene()->IsActive())
					return;

				Call(Entry.Awake, [this, &New](Scripting::ImmediateContext* Context)
				{
					if (Invoke == InvokeType::Typeless)
						return;

					Component* Current = this;
					Context->SetArgObject(0, Current);
					Context->SetArgObject(1, New);
				});
			}
			void Scriptable::Deactivate()
			{
				Call(Entry.Asleep, [this](Scripting::ImmediateContext* Context)
				{
					if (Invoke == InvokeType::Typeless)
						return;

					Component* Current = this;
					Context->SetArgObject(0, Current);
				});
			}
			void Scriptable::Update(Core::Timer* Time)
			{
				Call(Entry.Update, [this, &Time](Scripting::ImmediateContext* Context)
				{
					if (Invoke == InvokeType::Typeless)
						return;

					Component* Current = this;
					Context->SetArgObject(0, Current);
					Context->SetArgObject(1, Time);
				});
			}
			void Scriptable::Message(const std::string_view& Name, Core::VariantArgs& Args)
			{
				Call(Entry.Message, [this, Name, Args](Scripting::ImmediateContext* Context)
				{
					if (Invoke == InvokeType::Typeless)
						return;

					Scripting::Bindings::Dictionary* Map = Scripting::Bindings::Dictionary::Create(Compiler->GetVM());
					if (Map != nullptr)
					{
						int TypeId = Compiler->GetVM()->GetTypeIdByDecl("variant");
						for (auto& Item : Args)
						{
							Core::Variant Next = std::move(Item.second);
							Map->Set(Item.first, &Next, TypeId);
						}
					}

					Component* Current = this;
					Context->SetArgObject(0, Current);
					Context->SetArgObject(1, (void*)&Name);
					Context->SetArgObject(2, Map);
				});
			}
			Component* Scriptable::Copy(Entity* New) const
			{
				Scriptable* Target = new Scriptable(New);
				Target->Invoke = Invoke;
				Target->LoadSource(Source, Resource);

				if (!Compiler || !Target->Compiler)
					return Target;

				Scripting::Module From = Compiler->GetModule();
				Scripting::Module To = Target->Compiler->GetModule();
				Scripting::VirtualMachine* VM = Compiler->GetVM();

				if (!From.IsValid() || !To.IsValid())
					return Target;

				int Count = (int)From.GetPropertiesCount();
				for (int i = 0; i < Count; i++)
				{
					Scripting::PropertyInfo fSource;
					if (!From.GetProperty(i, &fSource))
						continue;

					Scripting::PropertyInfo Dest;
					if (!To.GetProperty(i, &Dest))
						continue;

					if (fSource.TypeId != Dest.TypeId)
						continue;

					if (fSource.TypeId < (int)Scripting::TypeId::BOOL || fSource.TypeId >(int)Scripting::TypeId::DOUBLE)
					{
						Scripting::TypeInfo Type = VM->GetTypeInfoById(fSource.TypeId);
						if (fSource.Pointer != nullptr && Type.IsValid())
						{
							void* Object = VM->CreateObjectCopy(fSource.Pointer, Type);
							if (Object != nullptr)
								VM->AssignObject(Dest.Pointer, Object, Type);
						}
					}
					else
					{
						auto Size = VM->GetSizeOfPrimitiveType(fSource.TypeId);
						if (Size)
							memcpy(Dest.Pointer, fSource.Pointer, *Size);
					}
				}

				return Target;
			}
			Scripting::ExpectsPromiseVM<Scripting::Execution> Scriptable::DeserializeCall(Core::Schema* Node)
			{
				Core::Schema* Cache = Node->Find("cache");
				if (Cache != nullptr)
				{
					for (auto& Var : Cache->GetChilds())
					{
						int TypeId = -1;
						if (!Series::Unpack(Var->Find("type"), &TypeId))
							continue;

						switch ((Scripting::TypeId)TypeId)
						{
							case Scripting::TypeId::BOOL:
							{
								bool Result = false;
								if (Series::Unpack(Var->Find("data"), &Result))
									SetTypePropertyByName(Var->Key.c_str(), Result);
								break;
							}
							case Scripting::TypeId::INT8:
							{
								int64_t Result = 0;
								if (Series::Unpack(Var->Find("data"), &Result))
									SetTypePropertyByName(Var->Key.c_str(), (char)Result);
								break;
							}
							case Scripting::TypeId::INT16:
							{
								int64_t Result = 0;
								if (Series::Unpack(Var->Find("data"), &Result))
									SetTypePropertyByName(Var->Key.c_str(), (short)Result);
								break;
							}
							case Scripting::TypeId::INT32:
							{
								int64_t Result = 0;
								if (Series::Unpack(Var->Find("data"), &Result))
									SetTypePropertyByName(Var->Key.c_str(), (int)Result);
								break;
							}
							case Scripting::TypeId::INT64:
							{
								int64_t Result = 0;
								if (Series::Unpack(Var->Find("data"), &Result))
									SetTypePropertyByName(Var->Key.c_str(), Result);
								break;
							}
							case Scripting::TypeId::UINT8:
							{
								int64_t Result = 0;
								if (Series::Unpack(Var->Find("data"), &Result))
									SetTypePropertyByName(Var->Key.c_str(), (uint8_t)Result);
								break;
							}
							case Scripting::TypeId::UINT16:
							{
								int64_t Result = 0;
								if (Series::Unpack(Var->Find("data"), &Result))
									SetTypePropertyByName(Var->Key.c_str(), (uint16_t)Result);
								break;
							}
							case Scripting::TypeId::UINT32:
							{
								int64_t Result = 0;
								if (Series::Unpack(Var->Find("data"), &Result))
									SetTypePropertyByName(Var->Key.c_str(), (uint32_t)Result);
								break;
							}
							case Scripting::TypeId::UINT64:
							{
								int64_t Result = 0;
								if (Series::Unpack(Var->Find("data"), &Result))
									SetTypePropertyByName(Var->Key.c_str(), (uint64_t)Result);
								break;
							}
							case Scripting::TypeId::FLOAT:
							{
								float Result = 0.0f;
								if (Series::Unpack(Var->Find("data"), &Result))
									SetTypePropertyByName(Var->Key.c_str(), Result);
								break;
							}
							case Scripting::TypeId::DOUBLE:
							{
								double Result = 0.0;
								if (Series::Unpack(Var->Find("data"), &Result))
									SetTypePropertyByName(Var->Key.c_str(), Result);
								break;
							}
							default:
							{
								Core::String Result;
								if (Series::Unpack(Var->Find("data"), &Result))
									SetTypePropertyByName(Var->Key.c_str(), Result);
								break;
							}
						}
					}
				}

				return Call(Entry.Deserialize, [this, Node](Scripting::ImmediateContext* Context)
				{
					if (Invoke == InvokeType::Typeless)
						return;

					Component* Current = this;
					Context->SetArgObject(0, Current);
					Context->SetArgObject(1, Node);
				});
			}
			Scripting::ExpectsPromiseVM<Scripting::Execution> Scriptable::SerializeCall(Core::Schema* Node)
			{
				auto HasProperties = GetPropertiesCount();
				if (HasProperties)
				{
					size_t Count = *HasProperties;
					Core::Schema* Cache = Node->Set("cache");
					for (size_t i = 0; i < Count; i++)
					{
						Scripting::PropertyInfo Result;
						if (!GetPropertyByIndex(i, &Result) || Result.Name.empty() || !Result.Pointer)
							continue;

						Core::Schema* Var = Core::Var::Set::Object();
						Series::Pack(Var->Set("type"), Result.TypeId);

						switch ((Scripting::TypeId)Result.TypeId)
						{
							case Scripting::TypeId::BOOL:
								Series::Pack(Var->Set("data"), *(bool*)Result.Pointer);
								break;
							case Scripting::TypeId::INT8:
								Series::Pack(Var->Set("data"), (int64_t)*(char*)Result.Pointer);
								break;
							case Scripting::TypeId::INT16:
								Series::Pack(Var->Set("data"), (int64_t)*(short*)Result.Pointer);
								break;
							case Scripting::TypeId::INT32:
								Series::Pack(Var->Set("data"), (int64_t)*(int*)Result.Pointer);
								break;
							case Scripting::TypeId::INT64:
								Series::Pack(Var->Set("data"), *(int64_t*)Result.Pointer);
								break;
							case Scripting::TypeId::UINT8:
								Series::Pack(Var->Set("data"), (int64_t)*(uint8_t*)Result.Pointer);
								break;
							case Scripting::TypeId::UINT16:
								Series::Pack(Var->Set("data"), (int64_t)*(uint16_t*)Result.Pointer);
								break;
							case Scripting::TypeId::UINT32:
								Series::Pack(Var->Set("data"), (int64_t)*(uint32_t*)Result.Pointer);
								break;
							case Scripting::TypeId::UINT64:
								Series::Pack(Var->Set("data"), (int64_t)*(uint64_t*)Result.Pointer);
								break;
							case Scripting::TypeId::FLOAT:
								Series::Pack(Var->Set("data"), (double)*(float*)Result.Pointer);
								break;
							case Scripting::TypeId::DOUBLE:
								Series::Pack(Var->Set("data"), *(double*)Result.Pointer);
								break;
							default:
							{
								Scripting::TypeInfo Type = GetCompiler()->GetVM()->GetTypeInfoById(Result.TypeId);
								if (Type.IsValid() && Type.GetName() == "string")
									Series::Pack(Var->Set("data"), *(Core::String*)Result.Pointer);
								else
									Core::Memory::Release(Var);
								break;
							}
						}

						if (Var != nullptr)
							Cache->Set(Result.Name, Var);
					}
				}

				return Call(Entry.Serialize, [this, &Node](Scripting::ImmediateContext* Context)
				{
					if (Invoke == InvokeType::Typeless)
						return;

					Component* Current = this;
					Context->SetArgObject(0, Current);
					Context->SetArgObject(1, Node);
				});
			}
			Scripting::ExpectsPromiseVM<Scripting::Execution> Scriptable::Call(const std::string_view& Name, size_t Args, Scripting::ArgsCallback&& OnArgs)
			{
				if (!Compiler)
					return Scripting::ExpectsPromiseVM<Scripting::Execution>(Scripting::VirtualException(Scripting::VirtualError::INVALID_CONFIGURATION));

				return Call(GetFunctionByName(Name, Args).GetFunction(), std::move(OnArgs));
			}
			Scripting::ExpectsPromiseVM<Scripting::Execution> Scriptable::Call(asIScriptFunction* Function, Scripting::ArgsCallback&& OnArgs)
			{
				if (!Compiler)
					return Scripting::ExpectsPromiseVM<Scripting::Execution>(Scripting::VirtualException(Scripting::VirtualError::INVALID_CONFIGURATION));

				Scripting::FunctionDelegate Delegate(Function);
				if (!Delegate.IsValid())
					return Scripting::ExpectsPromiseVM<Scripting::Execution>(Scripting::VirtualException(Scripting::VirtualError::NO_FUNCTION));

				Protect();
				return Delegate(std::move(OnArgs)).Then<Scripting::ExpectsVM<Scripting::Execution>>([this](Scripting::ExpectsVM<Scripting::Execution>&& Result)
				{
					Unprotect();
					return Result;
				});
			}
			Scripting::ExpectsPromiseVM<Scripting::Execution> Scriptable::CallEntry(const std::string_view& Name)
			{
				return Call(GetFunctionByName(Name, Invoke == InvokeType::Typeless ? 0 : 1).GetFunction(), [this](Scripting::ImmediateContext* Context)
				{
					if (Invoke == InvokeType::Typeless)
						return;

					Component* Current = this;
					Context->SetArgObject(0, Current);
				});
			}
			Scripting::ExpectsPromiseVM<void> Scriptable::LoadSource()
			{
				return LoadSource(Source, Resource);
			}
			Scripting::ExpectsPromiseVM<void> Scriptable::LoadSource(SourceType Type, const std::string_view& Data)
			{
				SceneGraph* Scene = Parent->GetScene();
				if (!Compiler)
				{
					auto* VM = Scene->GetConf().Shared.VM;
					if (!VM)
						return Scripting::ExpectsPromiseVM<void>(Scripting::VirtualException(Scripting::VirtualError::INVALID_CONFIGURATION));

					Compiler = VM->CreateCompiler();
					Compiler->SetPragmaCallback([this](Compute::Preprocessor*, const std::string_view& Name, const Core::Vector<Core::String>& Args) -> Compute::ExpectsPreprocessor<void>
					{
						if (Name == "name" && Args.size() == 1)
							Module = Args[0];

						return Core::Expectation::Met;
					});
				}

				Source = Type;
				Resource = Data;

				if (Resource.empty())
				{
					Entry.Serialize = nullptr;
					Entry.Deserialize = nullptr;
					Entry.Awake = nullptr;
					Entry.Asleep = nullptr;
					Entry.Animate = nullptr;
					Entry.Synchronize = nullptr;
					Entry.Update = nullptr;
					Entry.Message = nullptr;
					Compiler->Clear();
					return Scripting::ExpectsPromiseVM<void>(Scripting::VirtualException(Scripting::VirtualError::NO_MODULE));
				}

				auto Status = Compiler->Prepare("base", Source == SourceType::Resource ? Resource : "anonymous", true, true);
				if (!Status)
					return Scripting::ExpectsPromiseVM<void>(Status);

				Status = (Source == SourceType::Resource ? Compiler->LoadFile(Resource) : Compiler->LoadCode("anonymous", Resource));
				if (!Status)
					return Scripting::ExpectsPromiseVM<void>(Status);

				return Compiler->Compile().Then<Scripting::ExpectsVM<void>>([this](Scripting::ExpectsVM<void>&& Result)
				{
					Entry.Animate = GetFunctionByName("animate", Invoke == InvokeType::Typeless ? 0 : 3).GetFunction();
					Entry.Serialize = GetFunctionByName("serialize", Invoke == InvokeType::Typeless ? 0 : 3).GetFunction();
					Entry.Deserialize = GetFunctionByName("deserialize", Invoke == InvokeType::Typeless ? 0 : 3).GetFunction();
					Entry.Awake = GetFunctionByName("awake", Invoke == InvokeType::Typeless ? 0 : 2).GetFunction();
					Entry.Asleep = GetFunctionByName("asleep", Invoke == InvokeType::Typeless ? 0 : 1).GetFunction();
					Entry.Synchronize = GetFunctionByName("synchronize", Invoke == InvokeType::Typeless ? 0 : 2).GetFunction();
					Entry.Update = GetFunctionByName("update", Invoke == InvokeType::Typeless ? 0 : 2).GetFunction();
					Entry.Message = GetFunctionByName("message", Invoke == InvokeType::Typeless ? 0 : 3).GetFunction();
					return Result;
				});
			}
			Scripting::ExpectsVM<size_t> Scriptable::GetPropertiesCount()
			{
				if (!Compiler)
					return Scripting::VirtualException(Scripting::VirtualError::INVALID_CONFIGURATION);

				Scripting::Module Base = Compiler->GetModule();
				if (!Base.IsValid())
					return Scripting::VirtualException(Scripting::VirtualError::NO_MODULE);

				return Base.GetPropertiesCount();
			}
			Scripting::ExpectsVM<size_t> Scriptable::GetFunctionsCount()
			{
				if (!Compiler)
					return Scripting::VirtualException(Scripting::VirtualError::INVALID_CONFIGURATION);

				Scripting::Module Base = Compiler->GetModule();
				if (!Base.IsValid())
					return Scripting::VirtualException(Scripting::VirtualError::NO_MODULE);

				return Base.GetFunctionCount();
			}
			void Scriptable::SetInvocation(InvokeType Type)
			{
				Invoke = Type;
			}
			void Scriptable::UnloadSource()
			{
				LoadSource(Source, "");
			}
			void Scriptable::Protect()
			{
				AddRef();
				GetEntity()->AddRef();
			}
			void Scriptable::Unprotect()
			{
				GetEntity()->Release();
				Release();
			}
			Scripting::Compiler* Scriptable::GetCompiler()
			{
				return Compiler;
			}
			Scripting::Function Scriptable::GetFunctionByName(const std::string_view& Name, size_t Args)
			{
				VI_ASSERT(!Name.empty(), "name should not be empty");
				if (!Compiler)
					return nullptr;

				auto Result = Compiler->GetModule().GetFunctionByName(Name);
				if (Result.IsValid() && Result.GetArgsCount() != Args)
					return nullptr;

				return Result;
			}
			Scripting::Function Scriptable::GetFunctionByIndex(size_t Index, size_t Args)
			{
				VI_ASSERT(Index >= 0, "index should be greater or equal to zero");
				if (!Compiler)
					return nullptr;

				auto Result = Compiler->GetModule().GetFunctionByIndex(Index);
				if (Result.IsValid() && Result.GetArgsCount() != Args)
					return nullptr;

				return Result;
			}
			bool Scriptable::GetPropertyByName(const std::string_view& Name, Scripting::PropertyInfo* Result)
			{
				if (!Compiler)
					return false;

				Scripting::Module Base = Compiler->GetModule();
				if (!Base.IsValid())
					return false;

				auto Index = Base.GetPropertyIndexByName(Name);
				if (!Index || !Base.GetProperty(*Index, Result))
					return false;

				return true;
			}
			bool Scriptable::GetPropertyByIndex(size_t Index, Scripting::PropertyInfo* Result)
			{
				VI_ASSERT(Index >= 0, "index should be greater or equal to zero");
				if (!Compiler)
					return false;

				Scripting::Module Base = Compiler->GetModule();
				if (!Base.IsValid() || !Base.GetProperty(Index, Result))
					return false;

				return true;
			}
			Scriptable::SourceType Scriptable::GetSourceType()
			{
				return Source;
			}
			Scriptable::InvokeType Scriptable::GetInvokeType()
			{
				return Invoke;
			}
			const Core::String& Scriptable::GetSource()
			{
				return Resource;
			}
			const Core::String& Scriptable::GetModuleName()
			{
				return Module;
			}
		}
	}
}
