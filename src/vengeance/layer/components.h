#ifndef VI_LAYER_COMPONENTS_H
#define VI_LAYER_COMPONENTS_H
#include "../layer.h"

namespace Vitex
{
	namespace Layer
	{
		namespace Components
		{
			class VI_OUT SoftBody final : public Drawable
			{
			protected:
				Physics::HullShape* Hull = nullptr;
				Physics::SoftBody* Instance = nullptr;
				Core::Vector<Trigonometry::Vertex> Vertices;
				Core::Vector<int> Indices;

			public:
				Trigonometry::Vector2 TexCoord = 1.0f;
				bool Kinematic = false;
				bool Manage = true;

			public:
				SoftBody(Entity* Ref);
				~SoftBody() override;
				void Deserialize(Core::Schema* Node) override;
				void Serialize(Core::Schema* Node) override;
				void Synchronize(Core::Timer* Time) override;
				void Deactivate() override;
				float GetVisibility(const Viewer& View, float Distance) const override;
				size_t GetUnitBounds(Trigonometry::Vector3& Min, Trigonometry::Vector3& Max) const override;
				Core::Unique<Component> Copy(Entity* New) const override;
				void Load(Physics::HullShape* Shape, float Anticipation = 0.0f);
				void Load(const std::string_view& Path, float Anticipation = 0.0f, std::function<void()>&& Callback = nullptr);
				void LoadEllipsoid(const Physics::SoftBody::Desc::CV::SEllipsoid& Shape, float Anticipation = 0.0f);
				void LoadPatch(const Physics::SoftBody::Desc::CV::SPatch& Shape, float Anticipation = 0.0f);
				void LoadRope(const Physics::SoftBody::Desc::CV::SRope& Shape, float Anticipation = 0.0f);
				void Fill(Graphics::GraphicsDevice* Device, Graphics::ElementBuffer* IndexBuffer, Graphics::ElementBuffer* VertexBuffer);
				void Regenerate();
				void Clear();
				void SetTransform(const Trigonometry::Vector3& Position, const Trigonometry::Vector3& Scale, const Trigonometry::Vector3& Rotation);
				void SetTransform(bool Kinematic);
				Physics::SoftBody* GetBody();
				Core::Vector<Trigonometry::Vertex>& GetVertices();
				Core::Vector<int>& GetIndices();

			private:
				void DeserializeBody(Core::Schema* Node);

			public:
				VI_COMPONENT("soft_body_component");
			};

			class VI_OUT RigidBody final : public Component
			{
			private:
				Physics::HullShape* Hull = nullptr;
				Physics::RigidBody* Instance = nullptr;

			public:
				bool Kinematic = false;
				bool Manage = true;

			public:
				RigidBody(Entity* Ref);
				~RigidBody() override;
				void Deserialize(Core::Schema* Node) override;
				void Serialize(Core::Schema* Node) override;
				void Synchronize(Core::Timer* Time) override;
				void Deactivate() override;
				Core::Unique<Component> Copy(Entity* New) const override;
				void Load(btCollisionShape* Shape, float Mass, float Anticipation = 0.0f);
				void Load(const std::string_view& Path, float Mass, float Anticipation = 0.0f, std::function<void()>&& Callback = nullptr);
				void Clear();
				void SetTransform(const Trigonometry::Vector3& Position, const Trigonometry::Vector3& Scale, const Trigonometry::Vector3& Rotation);
				void SetTransform(bool Kinematic);
				void SetMass(float Mass);
				Physics::RigidBody* GetBody() const;

			private:
				void DeserializeBody(Core::Schema* Node);

			public:
				VI_COMPONENT("rigid_body_component");
			};

			class VI_OUT SliderConstraint final : public Component
			{
			private:
				Physics::SConstraint* Instance;
				Entity* Connection;

			public:
				SliderConstraint(Entity* Ref);
				~SliderConstraint() override;
				void Deserialize(Core::Schema* Node) override;
				void Serialize(Core::Schema* Node) override;
				Core::Unique<Component> Copy(Entity* New) const override;
				void Load(Entity* Other, bool IsGhosted, bool IsLinear);
				void Clear();
				Physics::SConstraint* GetConstraint() const;
				Entity* GetConnection() const;

			public:
				VI_COMPONENT("slider_constraint_component");
			};

			class VI_OUT Acceleration final : public Component
			{
			private:
				Physics::RigidBody* RigidBody = nullptr;

			public:
				Trigonometry::Vector3 AmplitudeVelocity;
				Trigonometry::Vector3 AmplitudeTorque;
				Trigonometry::Vector3 ConstantVelocity;
				Trigonometry::Vector3 ConstantTorque;
				Trigonometry::Vector3 ConstantCenter;
				bool Kinematic = true;

			public:
				Acceleration(Entity* Ref);
				void Deserialize(Core::Schema * Node) override;
				void Serialize(Core::Schema * Node) override;
				void Activate(Component * New) override;
				void Update(Core::Timer * Time) override;
				Core::Unique<Component> Copy(Entity * New) const override;
				Physics::RigidBody* GetBody() const;

			public:
				VI_COMPONENT("acceleration_component");
			};

			class VI_OUT Model final : public Drawable
			{
			protected:
				Layer::Model* Instance = nullptr;

			public:
				Trigonometry::Vector2 TexCoord = 1.0f;

			public:
				Model(Entity* Ref);
				~Model() override;
				void Deserialize(Core::Schema* Node) override;
				void Serialize(Core::Schema* Node) override;
				float GetVisibility(const Viewer& View, float Distance) const override;
				size_t GetUnitBounds(Trigonometry::Vector3& Min, Trigonometry::Vector3& Max) const override;
				Core::Unique<Component> Copy(Entity* New) const override;
				void SetDrawable(Core::Unique<Layer::Model> Drawable);
				void SetMaterialFor(const std::string_view& Name, Material* Value);
				Layer::Model* GetDrawable();
				Material* GetMaterialFor(const std::string_view& Name);

			public:
				VI_COMPONENT("model_component");
			};

			class VI_OUT Skin final : public Drawable
			{
			protected:
				Layer::SkinModel* Instance = nullptr;

			public:
				Trigonometry::Vector2 TexCoord = 1.0f;
				PoseBuffer Skeleton;

			public:
				Skin(Entity* Ref);
				~Skin() override;
				void Deserialize(Core::Schema* Node) override;
				void Serialize(Core::Schema* Node) override;
				void Synchronize(Core::Timer* Time) override;
				float GetVisibility(const Viewer& View, float Distance) const override;
				size_t GetUnitBounds(Trigonometry::Vector3& Min, Trigonometry::Vector3& Max) const override;
				Core::Unique<Component> Copy(Entity* New) const override;
				void SetDrawable(Core::Unique<Layer::SkinModel> Drawable);
				void SetMaterialFor(const std::string_view& Name, Material* Value);
				Layer::SkinModel* GetDrawable();
				Material* GetMaterialFor(const std::string_view& Name);

			public:
				VI_COMPONENT("skin_component");
			};

			class VI_OUT Emitter final : public Drawable
			{
			protected:
				Graphics::InstanceBuffer * Instance = nullptr;

			public:
				Trigonometry::Vector3 Min = 0.0f;
				Trigonometry::Vector3 Max = 1.0f;
				bool Connected = false;
				bool QuadBased = false;

			public:
				Emitter(Entity* Ref);
				~Emitter() override;
				void Deserialize(Core::Schema* Node) override;
				void Serialize(Core::Schema* Node) override;
				void Activate(Component* New) override;
				size_t GetUnitBounds(Trigonometry::Vector3& Min, Trigonometry::Vector3& Max) const override;
				Core::Unique<Component> Copy(Entity* New) const override;
				Graphics::InstanceBuffer* GetBuffer();

			public:
				VI_COMPONENT("emitter_component");
			};

			class VI_OUT Decal final : public Drawable
			{
			public:
				Trigonometry::Vector2 TexCoord = 1.0f;

			public:
				Decal(Entity* Ref);
				void Deserialize(Core::Schema * Node) override;
				void Serialize(Core::Schema * Node) override;
				float GetVisibility(const Viewer& View, float Distance) const override;
				Core::Unique<Component> Copy(Entity * New) const override;

			public:
				VI_COMPONENT("decal_component");
			};

			class VI_OUT SkinAnimator final : public Component
			{
			private:
				Skin* Instance = nullptr;
				SkinAnimation* Animation = nullptr;

			public:
				AnimatorState State;

			public:
				SkinAnimator(Entity* Ref);
				~SkinAnimator() noexcept override;
				void Deserialize(Core::Schema* Node) override;
				void Serialize(Core::Schema* Node) override;
				void Activate(Component* New) override;
				void Animate(Core::Timer* Time) override;
				Core::Unique<Component> Copy(Entity* New) const override;
				void SetAnimation(SkinAnimation* New);
				void Play(int64_t Clip = -1, int64_t Frame = -1);
				void Pause();
				void Stop();
				bool IsExists(int64_t Clip);
				bool IsExists(int64_t Clip, int64_t Frame);
				const Trigonometry::SkinAnimatorKey* GetFrame(int64_t Clip, int64_t Frame);
				const Core::Vector<Trigonometry::SkinAnimatorKey>* GetClip(int64_t Clip);
				int64_t GetClipByName(const std::string_view& Name) const;
				size_t GetClipsCount() const;
				Skin* GetSkin() const;
				SkinAnimation* GetAnimation() const;
				Core::String GetPath() const;

			private:
				void BlendAnimation(int64_t Clip, int64_t Frame);
				void SaveBindingState();

			public:
				VI_COMPONENT("skin_animator_component");
			};

			class VI_OUT KeyAnimator final : public Component
			{
			private:
				Core::String Reference;

			public:
				Core::Vector<Trigonometry::KeyAnimatorClip> Clips;
				Trigonometry::AnimatorKey Offset;
				Trigonometry::AnimatorKey Default;
				AnimatorState State;

			public:
				KeyAnimator(Entity* Ref);
				~KeyAnimator() override;
				void Deserialize(Core::Schema* Node) override;
				void Serialize(Core::Schema* Node) override;
				void Animate(Core::Timer* Time) override;
				Core::Unique<Component> Copy(Entity* New) const override;
				void LoadAnimation(const std::string_view& Path, std::function<void(bool)>&& Callback = nullptr);
				void ClearAnimation();
				void Play(int64_t Clip = -1, int64_t Frame = -1);
				void Pause();
				void Stop();
				bool IsExists(int64_t Clip);
				bool IsExists(int64_t Clip, int64_t Frame);
				Trigonometry::AnimatorKey* GetFrame(int64_t Clip, int64_t Frame);
				Core::Vector<Trigonometry::AnimatorKey>* GetClip(int64_t Clip);
				Core::String GetPath();

			private:
				void BlendAnimation(int64_t Clip, int64_t Frame);
				void SaveBindingState();

			public:
				VI_COMPONENT("key_animator_component");
			};

			class VI_OUT EmitterAnimator final : public Component
			{
			private:
				Emitter * Base = nullptr;

			public:
				Trigonometry::Vector4 Diffuse;
				Trigonometry::Vector3 Position;
				Trigonometry::Vector3 Velocity = 1.0f;
				SpawnerProperties Spawner;
				float RotationSpeed = 0.0f;
				float ScaleSpeed = 0.0f;
				float Noise = 0.0f;
				bool Simulate = false;

			public:
				EmitterAnimator(Entity* Ref);
				void Deserialize(Core::Schema * Node) override;
				void Serialize(Core::Schema * Node) override;
				void Activate(Component * New) override;
				void Animate(Core::Timer * Time) override;
				Core::Unique<Component> Copy(Entity * New) const override;
				Emitter* GetEmitter() const;

			protected:
				void AccurateSynchronization(float Step);
				void FastSynchronization(float Step);

			public:
				VI_COMPONENT("emitter_animator_component");
			};

			class VI_OUT FreeLook final : public Component
			{
			private:
				Trigonometry::Vector2 Position;

			public:
				Trigonometry::Vector2 Direction = Trigonometry::Vector2(1.0f, 1.0f);
				Graphics::KeyMap Rotate;
				float Sensivity;

			public:
				FreeLook(Entity* Ref);
				void Update(Core::Timer * Time) override;
				Core::Unique<Component> Copy(Entity * New) const override;

			public:
				VI_COMPONENT("free_look_component");
			};

			class VI_OUT Fly final : public Component
			{
			private:
				Trigonometry::Vector3 Velocity;

			public:
				struct MoveInfo
				{
					Trigonometry::Vector3 Axis = Trigonometry::Vector3(1.0f, 1.0f, -1.0f);
					float Faster = 320.3f;
					float Normal = 185.4f;
					float Slower = 32.6f;
					float Fading = 5.4f;
				} Moving;

			public:
				Graphics::KeyMap Forward = Graphics::KeyCode::W;
				Graphics::KeyMap Backward = Graphics::KeyCode::S;
				Graphics::KeyMap Right = Graphics::KeyCode::D;
				Graphics::KeyMap Left = Graphics::KeyCode::A;
				Graphics::KeyMap Up = Graphics::KeyCode::Space;
				Graphics::KeyMap Down = Graphics::KeyCode::Z;
				Graphics::KeyMap Fast = Graphics::KeyCode::LeftShift;
				Graphics::KeyMap Slow = Graphics::KeyCode::LeftControl;

			public:
				Fly(Entity* Ref);
				void Update(Core::Timer * Time) override;
				Core::Unique<Component> Copy(Entity * New) const override;

			private:
				Trigonometry::Vector3 GetSpeed(Graphics::Activity* Activity);

			public:
				VI_COMPONENT("fly_component");
			};

			class VI_OUT AudioSource final : public Component
			{
			private:
				Trigonometry::Vector3 LastPosition;
				Audio::AudioSource* Source;
				Audio::AudioSync Sync;

			public:
				AudioSource(Entity* Ref);
				~AudioSource() override;
				void Deserialize(Core::Schema* Node) override;
				void Serialize(Core::Schema* Node) override;
				void Synchronize(Core::Timer* Time) override;
				Core::Unique<Component> Copy(Entity* New) const override;
				void ApplyPlayingPosition();
				Audio::AudioSource* GetSource() const;
				Audio::AudioSync& GetSync();

			public:
				VI_COMPONENT("audio_source_component");
			};

			class VI_OUT AudioListener final : public Component
			{
			private:
				Trigonometry::Vector3 LastPosition;

			public:
				float Gain = 1.0f;

			public:
				AudioListener(Entity* Ref);
				~AudioListener() override;
				void Deserialize(Core::Schema* Node) override;
				void Serialize(Core::Schema* Node) override;
				void Synchronize(Core::Timer* Time) override;
				void Deactivate() override;
				Core::Unique<Component> Copy(Entity* New) const override;

			public:
				VI_COMPONENT("audio_listener_component");
			};

			class VI_OUT PointLight final : public Component
			{
			public:
				struct ShadowInfo
				{
					float Softness = 1.0f;
					float Distance = 100.0f;
					float Bias = 0.0f;
					uint32_t Iterations = 2;
					bool Enabled = false;
				} Shadow;

			private:
				Attenuation Size;

			public:
				CubicDepthMap* DepthMap = nullptr;
				Trigonometry::Matrix4x4 View;
				Trigonometry::Matrix4x4 Projection;
				Trigonometry::Vector3 Diffuse = 1.0f;
				float Emission = 1.0f;
				float Disperse = 0.0f;

			public:
				PointLight(Entity* Ref);
				void Deserialize(Core::Schema * Node) override;
				void Serialize(Core::Schema * Node) override;
				void Message(const std::string_view& Name, Core::VariantArgs& Args) override;
				size_t GetUnitBounds(Trigonometry::Vector3& Min, Trigonometry::Vector3& Max) const override;
				float GetVisibility(const Viewer& View, float Distance) const override;
				Core::Unique<Component> Copy(Entity * New) const override;
				void GenerateOrigin();
				void SetSize(const Attenuation& Value);
				const Attenuation& GetSize();

			public:
				VI_COMPONENT("point_light_component");
			};

			class VI_OUT SpotLight final : public Component
			{
			public:
				struct ShadowInfo
				{
					float Softness = 1.0f;
					float Distance = 100.0f;
					float Bias = 0.0f;
					uint32_t Iterations = 2;
					bool Enabled = false;
				} Shadow;

			private:
				Attenuation Size;

			public:
				LinearDepthMap* DepthMap = nullptr;
				Trigonometry::Matrix4x4 Projection;
				Trigonometry::Matrix4x4 View;
				Trigonometry::Vector3 Diffuse = 1.0f;
				float Emission = 1.0f;
				float Cutoff = 60.0f;
				float Disperse = 0.0f;

			public:
				SpotLight(Entity* Ref);
				void Deserialize(Core::Schema * Node) override;
				void Serialize(Core::Schema * Node) override;
				void Message(const std::string_view& Name, Core::VariantArgs& Args) override;
				void Synchronize(Core::Timer * Time) override;
				size_t GetUnitBounds(Trigonometry::Vector3& Min, Trigonometry::Vector3& Max) const override;
				float GetVisibility(const Viewer& View, float Distance) const override;
				Core::Unique<Component> Copy(Entity * New) const override;
				void GenerateOrigin();
				void SetSize(const Attenuation& Value);
				const Attenuation& GetSize();

			public:
				VI_COMPONENT("spot_light_component");
			};

			class VI_OUT LineLight final : public Component
			{
			public:
				struct SkyInfo
				{
					Trigonometry::Vector3 RlhEmission = Trigonometry::Vector3(0.0000055f, 0.000013f, 0.0000224f);
					Trigonometry::Vector3 MieEmission = 0.000021f;
					float RlhHeight = 8000.0f;
					float MieHeight = 1200.0f;
					float MieDirection = 0.94f;
					float InnerRadius = 6371000.0f;
					float OuterRadius = 6471000.0f;
					float Intensity = 7.0f;
				} Sky;

				struct ShadowInfo
				{
					float Distance[6] = { 25.0f, 50.0f, 100.0f, 175.0f, 250.0f, 325.0f };
					float Softness = 1.0f;
					float Bias = 0.0f;
					float Near = 6.0f;
					float Far = 9.0f;
					uint32_t Iterations = 2;
					uint32_t Cascades = 3;
					bool Enabled = false;
				} Shadow;

			public:
				CascadedDepthMap* DepthMap = nullptr;
				Trigonometry::Matrix4x4 Projection[6];
				Trigonometry::Matrix4x4 View[6];
				Trigonometry::Vector3 Diffuse = 1.0f;
				float Emission = 1.0f;
				float Disperse = 0.0f;

			public:
				LineLight(Entity* Ref);
				void Deserialize(Core::Schema * Node) override;
				void Serialize(Core::Schema * Node) override;
				void Message(const std::string_view& Name, Core::VariantArgs& Args) override;
				Core::Unique<Component> Copy(Entity * New) const override;
				void GenerateOrigin();

			public:
				VI_COMPONENT("line_light_component");
			};

			class VI_OUT SurfaceLight final : public Component
			{
			private:
				Graphics::Texture2D * DiffuseMapX[2] = { nullptr };
				Graphics::Texture2D* DiffuseMapY[2] = { nullptr };
				Graphics::Texture2D* DiffuseMapZ[2] = { nullptr };
				Graphics::Texture2D* DiffuseMap = nullptr;
				Graphics::TextureCube* Probe = nullptr;

			private:
				Attenuation Size;

			public:
				Trigonometry::Matrix4x4 View[6];
				Trigonometry::Matrix4x4 Projection;
				Trigonometry::Vector3 Offset = Trigonometry::Vector3(1.0f, 1.0f, 1.0f);
				Trigonometry::Vector3 Diffuse = 1.0f;
				Ticker Tick;
				float Emission = 1.0f;
				float Infinity = 0.0f;
				bool Parallax = false;
				bool Locked = false;
				bool StaticMask = false;

			public:
				SurfaceLight(Entity* Ref);
				~SurfaceLight() override;
				void Deserialize(Core::Schema* Node) override;
				void Serialize(Core::Schema* Node) override;
				size_t GetUnitBounds(Trigonometry::Vector3& Min, Trigonometry::Vector3& Max) const override;
				float GetVisibility(const Viewer& View, float Distance) const override;
				Core::Unique<Component> Copy(Entity* New) const override;
				void SetProbeCache(Core::Unique<Graphics::TextureCube> NewCache);
				void SetSize(const Attenuation& Value);
				bool SetDiffuseMap(Graphics::Texture2D* Map);
				bool SetDiffuseMap(Graphics::Texture2D* const MapX[2], Graphics::Texture2D* const MapY[2], Graphics::Texture2D* const MapZ[2]);
				bool IsImageBased() const;
				const Attenuation& GetSize();
				Graphics::TextureCube* GetProbeCache() const;
				Graphics::Texture2D* GetDiffuseMapXP();
				Graphics::Texture2D* GetDiffuseMapXN();
				Graphics::Texture2D* GetDiffuseMapYP();
				Graphics::Texture2D* GetDiffuseMapYN();
				Graphics::Texture2D* GetDiffuseMapZP();
				Graphics::Texture2D* GetDiffuseMapZN();
				Graphics::Texture2D* GetDiffuseMap();

			public:
				VI_COMPONENT("surface_light_component");
			};

			class VI_OUT Illuminator final : public Component
			{
			public:
				Graphics::Texture3D* VoxelMap;
				Ticker Inside;
				Ticker Outside;
				float RayStep;
				float MaxSteps;
				float Distance;
				float Radiance;
				float Length;
				float Margin;
				float Offset;
				float Angle;
				float Occlusion;
				float Specular;
				float Bleeding;
				bool Regenerate;

			public:
				Illuminator(Entity* Ref);
				void Deserialize(Core::Schema* Node) override;
				void Serialize(Core::Schema* Node) override;
				void Message(const std::string_view& Name, Core::VariantArgs& Args) override;
				Core::Unique<Component> Copy(Entity* New) const override;

			public:
				VI_COMPONENT("illuminator_component");
			};

			class VI_OUT Camera final : public Component
			{
			public:
				enum class ProjectionMode
				{
					Perspective,
					Orthographic
				} Mode;

			protected:
				RenderSystem* Renderer = nullptr;
				Trigonometry::Matrix4x4 Projection;
				Graphics::Viewport Viewport;
				Viewer View;

			public:
				float NearPlane = 0.1f;
				float FarPlane = 250.0f;
				float Width = -1;
				float Height = -1;
				float FieldOfView = 75.0f;

			public:
				Camera(Entity* Ref);
				~Camera() override;
				void Deserialize(Core::Schema* Node) override;
				void Serialize(Core::Schema* Node) override;
				void Activate(Component* New) override;
				void Synchronize(Core::Timer* Time) override;
				Core::Unique<Component> Copy(Entity* New) const override;
				void GetViewer(Viewer* View);
				void ResizeBuffers();
				Viewer& GetViewer();
				RenderSystem* GetRenderer();
				Trigonometry::Matrix4x4 GetProjection();
				Trigonometry::Matrix4x4 GetViewProjection();
				Trigonometry::Matrix4x4 GetView();
				Trigonometry::Vector3 GetViewPosition();
				Trigonometry::Frustum8C GetFrustum8C();
				Trigonometry::Frustum6P GetFrustum6P();
				Trigonometry::Ray GetScreenRay(const Trigonometry::Vector2& Position);
				Trigonometry::Ray GetCursorRay();
				float GetDistance(Entity* Other);
				float GetWidth();
				float GetHeight();
				float GetAspect();
				bool RayTest(const Trigonometry::Ray& Ray, Entity* Other, Trigonometry::Vector3* Hit = nullptr);
				bool RayTest(const Trigonometry::Ray& Ray, const Trigonometry::Matrix4x4& World, Trigonometry::Vector3* Hit = nullptr);

			public:
				VI_COMPONENT("camera_component");
			};

			class VI_OUT Scriptable final : public Component
			{
			public:
				enum class SourceType
				{
					Resource,
					Memory
				};

				enum class InvokeType
				{
					Typeless,
					Normal
				};

			protected:
				struct
				{
					asIScriptFunction* Serialize = nullptr;
					asIScriptFunction* Deserialize = nullptr;
					asIScriptFunction* Awake = nullptr;
					asIScriptFunction* Asleep = nullptr;
					asIScriptFunction* Synchronize = nullptr;
					asIScriptFunction* Animate = nullptr;
					asIScriptFunction* Update = nullptr;
					asIScriptFunction* Message = nullptr;
				} Entry;

			protected:
				Scripting::Compiler* Compiler;
				Core::String Resource;
				Core::String Module;
				SourceType Source;
				InvokeType Invoke;

			public:
				Scriptable(Entity* Ref);
				~Scriptable() override;
				void Deserialize(Core::Schema* Node) override;
				void Serialize(Core::Schema* Node) override;
				void Activate(Component* New) override;
				void Deactivate() override;
				void Update(Core::Timer* Time) override;
				void Message(const std::string_view& Name, Core::VariantArgs& Args) override;
				Core::Unique<Component> Copy(Entity* New) const override;
				Scripting::ExpectsPromiseVM<Scripting::Execution> Call(const std::string_view& Name, size_t Args, Scripting::ArgsCallback&& OnArgs);
				Scripting::ExpectsPromiseVM<Scripting::Execution> Call(asIScriptFunction* Entry, Scripting::ArgsCallback&& OnArgs);
				Scripting::ExpectsPromiseVM<Scripting::Execution> CallEntry(const std::string_view& Name);
				Scripting::ExpectsPromiseVM<void> LoadSource();
				Scripting::ExpectsPromiseVM<void> LoadSource(SourceType Type, const std::string_view& Source);
				Scripting::ExpectsVM<size_t> GetPropertiesCount();
				Scripting::ExpectsVM<size_t> GetFunctionsCount();
				void SetInvocation(InvokeType Type);
				void UnloadSource();
				bool GetPropertyByName(const std::string_view& Name, Scripting::PropertyInfo* Result);
				bool GetPropertyByIndex(size_t Index, Scripting::PropertyInfo* Result);
				Scripting::Function GetFunctionByName(const std::string_view& Name, size_t Args);
				Scripting::Function GetFunctionByIndex(size_t Index, size_t Args);
				Scripting::Compiler* GetCompiler();
				SourceType GetSourceType();
				InvokeType GetInvokeType();
				const Core::String& GetSource();
				const Core::String& GetModuleName();

			public:
				template <typename T>
				Scripting::ExpectsVM<void> SetTypePropertyByName(const std::string_view& Name, const T& Value)
				{
					VI_ASSERT(Compiler != nullptr, "compiler should be set");
					Scripting::Module Base = Compiler->GetModule();
					if (!Base.IsValid())
						return Scripting::VirtualException(Scripting::VirtualError::NO_MODULE);

					auto Index = Base.GetPropertyIndexByName(Name);
					if (!Index)
						return Index.Error();

					T* Address = (T*)Base.GetAddressOfProperty(*Index);
					if (!Address)
						return Scripting::VirtualException(Scripting::VirtualError::INVALID_OBJECT);

					*Address = Value;
					return Core::Expectation::Met;
				}
				template <typename T>
				Scripting::ExpectsVM<void> SetRefPropertyByName(const std::string_view& Name, T* Value)
				{
					VI_ASSERT(Compiler != nullptr, "compiler should be set");
					Scripting::Module Base = Compiler->GetModule();
					if (!Base.IsValid())
						return Scripting::VirtualException(Scripting::VirtualError::NO_MODULE);

					auto Index = Base.GetPropertyIndexByName(Name);
					if (!Index)
						return Index.Error();

					T** Address = (T**)Base.GetAddressOfProperty(*Index);
					if (!Address)
						return Scripting::VirtualException(Scripting::VirtualError::INVALID_OBJECT);

					Core::Memory::Release(*Address);
					*Address = Value;
					if (*Address != nullptr)
						(*Address)->AddRef();
					return Core::Expectation::Met;
				}
				template <typename T>
				Scripting::ExpectsVM<void> SetTypePropertyByIndex(size_t Index, const T& Value)
				{
					VI_ASSERT(Index >= 0, "index should be greater or equal to zero");
					VI_ASSERT(Compiler != nullptr, "compiler should be set");

					Scripting::Module Base = Compiler->GetModule();
					if (!Base.IsValid())
						return Scripting::VirtualException(Scripting::VirtualError::NO_MODULE);

					T* Address = (T*)Base.GetAddressOfProperty(Index);
					if (!Address)
						return Scripting::VirtualException(Scripting::VirtualError::INVALID_OBJECT);

					*Address = Value;
					return Core::Expectation::Met;
				}
				template <typename T>
				Scripting::ExpectsVM<void> SetRefPropertyByIndex(size_t Index, T* Value)
				{
					VI_ASSERT(Index >= 0, "index should be greater or equal to zero");
					VI_ASSERT(Compiler != nullptr, "compiler should be set");

					Scripting::Module Base = Compiler->GetModule();
					if (!Base.IsValid())
						return Scripting::VirtualException(Scripting::VirtualError::INVALID_CONFIGURATION);

					T** Address = (T**)Base.GetAddressOfProperty(Index);
					if (!Address)
						return Scripting::VirtualException(Scripting::VirtualError::INVALID_OBJECT);

					Core::Memory::Release(*Address);
					*Address = Value;
					if (*Address != nullptr)
						(*Address)->AddRef();
					return Core::Expectation::Met;
				}

			private:
				Scripting::ExpectsPromiseVM<Scripting::Execution> DeserializeCall(Core::Schema* Node);
				Scripting::ExpectsPromiseVM<Scripting::Execution> SerializeCall(Core::Schema* Node);
				void Protect();
				void Unprotect();

			public:
				VI_COMPONENT("scriptable_component");
			};
		}
	}
}
#endif