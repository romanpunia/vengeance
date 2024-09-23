#include "bindings.h"
#include "internal/types.hpp"
#ifdef VI_BINDINGS
#include "layer/processors.h"
#include "layer/components.h"
#include "layer/renderers.h"
#include "audio/effects.h"
#include "audio/filters.h"
#endif
#ifdef VI_ANGELSCRIPT
#include <angelscript.h>
#endif

namespace Vitex
{
	namespace Scripting
	{
		namespace Bindings
		{
			void HandleToHandleCast(void* From, void** To, int TypeId);
#ifdef VI_BINDINGS
			template <typename T>
			Core::String GetComponentName(T* Base)
			{
				return Base->GetName();
			}
			template <typename T>
			void PopulateComponent(RefClass& Class)
			{
				Class.SetMethodEx("string get_name() const", &GetComponentName<T>);
				Class.SetMethod("uint64 get_id() const", &T::GetId);
			}
			Core::VariantArgs ToVariantKeys(Core::Schema* Args);

			HeavyApplication::HeavyApplication(Desc& I, void* NewObject, int NewTypeId) noexcept : Layer::HeavyApplication(&I), ProcessedEvents(0), InitiatorType(nullptr), InitiatorObject(NewObject)
			{
				VirtualMachine* CurrentVM = VirtualMachine::Get();
				if (CurrentVM != nullptr && InitiatorObject != nullptr && ((NewTypeId & (int)TypeId::OBJHANDLE) || (NewTypeId & (int)TypeId::MASK_OBJECT)))
				{
					InitiatorType = CurrentVM->GetTypeInfoById(NewTypeId).GetTypeInfo();
					if (NewTypeId & (int)TypeId::OBJHANDLE)
						InitiatorObject = *(void**)InitiatorObject;
					CurrentVM->AddRefObject(InitiatorObject, InitiatorType);
				}
				else if (CurrentVM != nullptr && InitiatorObject != nullptr)
				{
					if (NewTypeId != (int)TypeId::VOIDF)
						Exception::Throw(Exception::Pointer(EXCEPTION_INVALIDINITIATOR));
					InitiatorObject = nullptr;
				}

				if (I.Usage & Layer::USE_SCRIPTING)
					VM = CurrentVM;

				if (I.Usage & Layer::USE_PROCESSING)
				{
					Content = new Layer::HeavyContentManager();
					Content->AddProcessor(new Layer::Processors::AssetProcessor(Content), VI_HASH(TYPENAME_ASSETFILE));
					Content->AddProcessor(new Layer::Processors::MaterialProcessor(Content), VI_HASH(TYPENAME_MATERIAL));
					Content->AddProcessor(new Layer::Processors::SceneGraphProcessor(Content), VI_HASH(TYPENAME_SCENEGRAPH));
					Content->AddProcessor(new Layer::Processors::AudioClipProcessor(Content), VI_HASH(TYPENAME_AUDIOCLIP));
					Content->AddProcessor(new Layer::Processors::Texture2DProcessor(Content), VI_HASH(TYPENAME_TEXTURE2D));
					Content->AddProcessor(new Layer::Processors::ShaderProcessor(Content), VI_HASH(TYPENAME_SHADER));
					Content->AddProcessor(new Layer::Processors::ModelProcessor(Content), VI_HASH(TYPENAME_MODEL));
					Content->AddProcessor(new Layer::Processors::SkinModelProcessor(Content), VI_HASH(TYPENAME_SKINMODEL));
					Content->AddProcessor(new Layer::Processors::SkinAnimationProcessor(Content), VI_HASH(TYPENAME_SKINANIMATION));
					Content->AddProcessor(new Layer::Processors::SchemaProcessor(Content), VI_HASH(TYPENAME_SCHEMA));
					Content->AddProcessor(new Layer::Processors::ServerProcessor(Content), VI_HASH(TYPENAME_HTTPSERVER));
					Content->AddProcessor(new Layer::Processors::HullShapeProcessor(Content), VI_HASH(TYPENAME_PHYSICSHULLSHAPE));
				}
			}
			HeavyApplication::~HeavyApplication() noexcept
			{
				VirtualMachine* CurrentVM = VM ? VM : VirtualMachine::Get();
				if (CurrentVM != nullptr && InitiatorObject != nullptr && InitiatorType != nullptr)
					CurrentVM->ReleaseObject(InitiatorObject, InitiatorType);

				OnKeyEvent.Release();
				OnInputEvent.Release();
				OnWheelEvent.Release();
				OnWindowEvent.Release();
				OnDispatch.Release();
				OnPublish.Release();
				OnComposition.Release();
				OnScriptHook.Release();
				OnInitialize.Release();
				OnStartup.Release();
				OnShutdown.Release();
				InitiatorObject = nullptr;
				InitiatorType = nullptr;
				VM = nullptr;
			}
			void HeavyApplication::SetOnKeyEvent(asIScriptFunction* Callback)
			{
				OnKeyEvent = FunctionDelegate(Callback);
			}
			void HeavyApplication::SetOnInputEvent(asIScriptFunction* Callback)
			{
				OnInputEvent = FunctionDelegate(Callback);
			}
			void HeavyApplication::SetOnWheelEvent(asIScriptFunction* Callback)
			{
				OnWheelEvent = FunctionDelegate(Callback);
			}
			void HeavyApplication::SetOnWindowEvent(asIScriptFunction* Callback)
			{
				OnWindowEvent = FunctionDelegate(Callback);
			}
			void HeavyApplication::SetOnDispatch(asIScriptFunction* Callback)
			{
				OnDispatch = FunctionDelegate(Callback);
			}
			void HeavyApplication::SetOnPublish(asIScriptFunction* Callback)
			{
				OnPublish = FunctionDelegate(Callback);
			}
			void HeavyApplication::SetOnComposition(asIScriptFunction* Callback)
			{
				OnComposition = FunctionDelegate(Callback);
			}
			void HeavyApplication::SetOnScriptHook(asIScriptFunction* Callback)
			{
				OnScriptHook = FunctionDelegate(Callback);
			}
			void HeavyApplication::SetOnInitialize(asIScriptFunction* Callback)
			{
				OnInitialize = FunctionDelegate(Callback);
			}
			void HeavyApplication::SetOnStartup(asIScriptFunction* Callback)
			{
				OnStartup = FunctionDelegate(Callback);
			}
			void HeavyApplication::SetOnShutdown(asIScriptFunction* Callback)
			{
				OnShutdown = FunctionDelegate(Callback);
			}
			void HeavyApplication::ScriptHook()
			{
				if (!OnScriptHook.IsValid())
					return;

				auto* Context = ImmediateContext::Get();
				VI_ASSERT(Context != nullptr, "application method cannot be called outside of script context");
				Context->ExecuteSubcall(OnScriptHook.Callable(), nullptr);
			}
			void HeavyApplication::KeyEvent(Graphics::KeyCode Key, Graphics::KeyMod Mod, int Virtual, int Repeat, bool Pressed)
			{
				if (!OnKeyEvent.IsValid())
					return;

				auto* Context = ImmediateContext::Get();
				VI_ASSERT(Context != nullptr, "application method cannot be called outside of script context");
				Context->ExecuteSubcall(OnKeyEvent.Callable(), [Key, Mod, Virtual, Repeat, Pressed](ImmediateContext* Context)
				{
					Context->SetArg32(0, (int)Key);
					Context->SetArg32(1, (int)Mod);
					Context->SetArg32(2, Virtual);
					Context->SetArg32(3, Repeat);
					Context->SetArg8(4, (unsigned char)Pressed);
				});
			}
			void HeavyApplication::InputEvent(char* Buffer, size_t Length)
			{
				if (!OnInputEvent.IsValid())
					return;

				std::string_view Text = std::string_view(Buffer, Length);
				auto* Context = ImmediateContext::Get();
				VI_ASSERT(Context != nullptr, "application method cannot be called outside of script context");
				Context->ExecuteSubcall(OnInputEvent.Callable(), [&Text](ImmediateContext* Context)
				{
					Context->SetArgObject(0, (void*)&Text);
				});
			}
			void HeavyApplication::WheelEvent(int X, int Y, bool Normal)
			{
				if (!OnWheelEvent.IsValid())
					return;

				auto* Context = ImmediateContext::Get();
				VI_ASSERT(Context != nullptr, "application method cannot be called outside of script context");
				Context->ExecuteSubcall(OnWheelEvent.Callable(), [X, Y, Normal](ImmediateContext* Context)
				{
					Context->SetArg32(0, X);
					Context->SetArg32(1, Y);
					Context->SetArg8(2, (unsigned char)Normal);
				});
			}
			void HeavyApplication::WindowEvent(Graphics::WindowState NewState, int X, int Y)
			{
				if (!OnWindowEvent.IsValid())
					return;

				auto* Context = ImmediateContext::Get();
				VI_ASSERT(Context != nullptr, "application method cannot be called outside of script context");
				Context->ExecuteSubcall(OnWindowEvent.Callable(), [NewState, X, Y](ImmediateContext* Context)
				{
					Context->SetArg32(0, (int)NewState);
					Context->SetArg32(1, X);
					Context->SetArg32(2, Y);
				});
			}
			void HeavyApplication::Composition()
			{
				if (!OnComposition.IsValid())
					return;

				auto* Context = ImmediateContext::Get();
				VI_ASSERT(Context != nullptr, "application method cannot be called outside of script context");
				Context->ExecuteSubcall(OnComposition.Callable(), nullptr);
			}
			void HeavyApplication::Dispatch(Core::Timer* Time)
			{
				auto* Loop = EventLoop::Get();
				if (Loop != nullptr)
					ProcessedEvents = Loop->Dequeue(VM);
				else
					ProcessedEvents = 0;

				if (OnDispatch.IsValid())
				{
					auto* Context = ImmediateContext::Get();
					VI_ASSERT(Context != nullptr, "application method cannot be called outside of script context");
					Context->ExecuteSubcall(OnDispatch.Callable(), [Time](ImmediateContext* Context)
					{
						Context->SetArgObject(0, (void*)Time);
					});
				}
			}
			void HeavyApplication::Publish(Core::Timer* Time)
			{
				if (!OnPublish.IsValid())
					return;

				auto* Context = ImmediateContext::Get();
				VI_ASSERT(Context != nullptr, "application method cannot be called outside of script context");
				Context->ExecuteSubcall(OnPublish.Callable(), [Time](ImmediateContext* Context)
				{
					Context->SetArgObject(0, (void*)Time);
				});
			}
			void HeavyApplication::Initialize()
			{
				if (!OnInitialize.IsValid())
					return;

				auto* Context = ImmediateContext::Get();
				VI_ASSERT(Context != nullptr, "application method cannot be called outside of script context");
				Context->ExecuteSubcall(OnInitialize.Callable(), nullptr);
			}
			Core::Promise<void> HeavyApplication::Startup()
			{
				if (!OnStartup.IsValid())
					return Core::Promise<void>::Null();

				VI_ASSERT(ImmediateContext::Get() != nullptr, "application method cannot be called outside of script context");
				auto Future = OnStartup(nullptr);
				if (!Future.IsPending())
					return Core::Promise<void>::Null();

				return Future.Then(std::function<void(ExpectsVM<Execution>&&)>(nullptr));
			}
			Core::Promise<void> HeavyApplication::Shutdown()
			{
				if (!OnShutdown.IsValid())
					return Core::Promise<void>::Null();

				VI_ASSERT(ImmediateContext::Get() != nullptr, "application method cannot be called outside of script context");
				auto Future = OnShutdown(nullptr);
				if (!Future.IsPending())
					return Core::Promise<void>::Null();

				return Future.Then(std::function<void(ExpectsVM<Execution>&&)>(nullptr));
			}
			size_t HeavyApplication::GetProcessedEvents() const
			{
				return ProcessedEvents;
			}
			bool HeavyApplication::HasProcessedEvents() const
			{
				return ProcessedEvents > 0;
			}
			bool HeavyApplication::RetrieveInitiatorObject(void* RefPointer, int RefTypeId) const
			{
				VirtualMachine* CurrentVM = VM ? VM : VirtualMachine::Get();
				if (!InitiatorObject || !InitiatorType || !RefPointer || !CurrentVM)
					return false;

				if (RefTypeId & (size_t)TypeId::OBJHANDLE)
				{
					CurrentVM->RefCastObject(InitiatorObject, InitiatorType, CurrentVM->GetTypeInfoById(RefTypeId), reinterpret_cast<void**>(RefPointer));
#ifdef VI_ANGELSCRIPT
					if (*(asPWORD*)RefPointer == 0)
						return false;
#endif
					return true;
				}
				else if (RefTypeId & (size_t)TypeId::MASK_OBJECT)
				{
					auto RefTypeInfo = CurrentVM->GetTypeInfoById(RefTypeId);
					if (InitiatorType == RefTypeInfo.GetTypeInfo())
					{
						CurrentVM->AssignObject(RefPointer, InitiatorObject, InitiatorType);
						return true;
					}
				}

				return false;
			}
			void* HeavyApplication::GetInitiatorObject() const
			{
				return InitiatorObject;
			}
			bool HeavyApplication::WantsRestart(int ExitCode)
			{
				return ExitCode == Layer::EXIT_RESTART;
			}

			Trigonometry::Vector2& Vector2MulEq1(Trigonometry::Vector2& A, const Trigonometry::Vector2& B)
			{
				return A *= B;
			}
			Trigonometry::Vector2& Vector2MulEq2(Trigonometry::Vector2& A, float B)
			{
				return A *= B;
			}
			Trigonometry::Vector2& Vector2DivEq1(Trigonometry::Vector2& A, const Trigonometry::Vector2& B)
			{
				return A /= B;
			}
			Trigonometry::Vector2& Vector2DivEq2(Trigonometry::Vector2& A, float B)
			{
				return A /= B;
			}
			Trigonometry::Vector2& Vector2AddEq1(Trigonometry::Vector2& A, const Trigonometry::Vector2& B)
			{
				return A += B;
			}
			Trigonometry::Vector2& Vector2AddEq2(Trigonometry::Vector2& A, float B)
			{
				return A += B;
			}
			Trigonometry::Vector2& Vector2SubEq1(Trigonometry::Vector2& A, const Trigonometry::Vector2& B)
			{
				return A -= B;
			}
			Trigonometry::Vector2& Vector2SubEq2(Trigonometry::Vector2& A, float B)
			{
				return A -= B;
			}
			Trigonometry::Vector2 Vector2Mul1(Trigonometry::Vector2& A, const Trigonometry::Vector2& B)
			{
				return A * B;
			}
			Trigonometry::Vector2 Vector2Mul2(Trigonometry::Vector2& A, float B)
			{
				return A * B;
			}
			Trigonometry::Vector2 Vector2Div1(Trigonometry::Vector2& A, const Trigonometry::Vector2& B)
			{
				return A / B;
			}
			Trigonometry::Vector2 Vector2Div2(Trigonometry::Vector2& A, float B)
			{
				return A / B;
			}
			Trigonometry::Vector2 Vector2Add1(Trigonometry::Vector2& A, const Trigonometry::Vector2& B)
			{
				return A + B;
			}
			Trigonometry::Vector2 Vector2Add2(Trigonometry::Vector2& A, float B)
			{
				return A + B;
			}
			Trigonometry::Vector2 Vector2Sub1(Trigonometry::Vector2& A, const Trigonometry::Vector2& B)
			{
				return A - B;
			}
			Trigonometry::Vector2 Vector2Sub2(Trigonometry::Vector2& A, float B)
			{
				return A - B;
			}
			Trigonometry::Vector2 Vector2Neg(Trigonometry::Vector2& A)
			{
				return -A;
			}
			bool Vector2Eq(Trigonometry::Vector2& A, const Trigonometry::Vector2& B)
			{
				return A == B;
			}
			int Vector2Cmp(Trigonometry::Vector2& A, const Trigonometry::Vector2& B)
			{
				if (A == B)
					return 0;

				return A > B ? 1 : -1;
			}

			Trigonometry::Vector3& Vector3MulEq1(Trigonometry::Vector3& A, const Trigonometry::Vector3& B)
			{
				return A *= B;
			}
			Trigonometry::Vector3& Vector3MulEq2(Trigonometry::Vector3& A, float B)
			{
				return A *= B;
			}
			Trigonometry::Vector3& Vector3DivEq1(Trigonometry::Vector3& A, const Trigonometry::Vector3& B)
			{
				return A /= B;
			}
			Trigonometry::Vector3& Vector3DivEq2(Trigonometry::Vector3& A, float B)
			{
				return A /= B;
			}
			Trigonometry::Vector3& Vector3AddEq1(Trigonometry::Vector3& A, const Trigonometry::Vector3& B)
			{
				return A += B;
			}
			Trigonometry::Vector3& Vector3AddEq2(Trigonometry::Vector3& A, float B)
			{
				return A += B;
			}
			Trigonometry::Vector3& Vector3SubEq1(Trigonometry::Vector3& A, const Trigonometry::Vector3& B)
			{
				return A -= B;
			}
			Trigonometry::Vector3& Vector3SubEq2(Trigonometry::Vector3& A, float B)
			{
				return A -= B;
			}
			Trigonometry::Vector3 Vector3Mul1(Trigonometry::Vector3& A, const Trigonometry::Vector3& B)
			{
				return A * B;
			}
			Trigonometry::Vector3 Vector3Mul2(Trigonometry::Vector3& A, float B)
			{
				return A * B;
			}
			Trigonometry::Vector3 Vector3Div1(Trigonometry::Vector3& A, const Trigonometry::Vector3& B)
			{
				return A / B;
			}
			Trigonometry::Vector3 Vector3Div2(Trigonometry::Vector3& A, float B)
			{
				return A / B;
			}
			Trigonometry::Vector3 Vector3Add1(Trigonometry::Vector3& A, const Trigonometry::Vector3& B)
			{
				return A + B;
			}
			Trigonometry::Vector3 Vector3Add2(Trigonometry::Vector3& A, float B)
			{
				return A + B;
			}
			Trigonometry::Vector3 Vector3Sub1(Trigonometry::Vector3& A, const Trigonometry::Vector3& B)
			{
				return A - B;
			}
			Trigonometry::Vector3 Vector3Sub2(Trigonometry::Vector3& A, float B)
			{
				return A - B;
			}
			Trigonometry::Vector3 Vector3Neg(Trigonometry::Vector3& A)
			{
				return -A;
			}
			bool Vector3Eq(Trigonometry::Vector3& A, const Trigonometry::Vector3& B)
			{
				return A == B;
			}
			int Vector3Cmp(Trigonometry::Vector3& A, const Trigonometry::Vector3& B)
			{
				if (A == B)
					return 0;

				return A > B ? 1 : -1;
			}

			Trigonometry::Vector4& Vector4MulEq1(Trigonometry::Vector4& A, const Trigonometry::Vector4& B)
			{
				return A *= B;
			}
			Trigonometry::Vector4& Vector4MulEq2(Trigonometry::Vector4& A, float B)
			{
				return A *= B;
			}
			Trigonometry::Vector4& Vector4DivEq1(Trigonometry::Vector4& A, const Trigonometry::Vector4& B)
			{
				return A /= B;
			}
			Trigonometry::Vector4& Vector4DivEq2(Trigonometry::Vector4& A, float B)
			{
				return A /= B;
			}
			Trigonometry::Vector4& Vector4AddEq1(Trigonometry::Vector4& A, const Trigonometry::Vector4& B)
			{
				return A += B;
			}
			Trigonometry::Vector4& Vector4AddEq2(Trigonometry::Vector4& A, float B)
			{
				return A += B;
			}
			Trigonometry::Vector4& Vector4SubEq1(Trigonometry::Vector4& A, const Trigonometry::Vector4& B)
			{
				return A -= B;
			}
			Trigonometry::Vector4& Vector4SubEq2(Trigonometry::Vector4& A, float B)
			{
				return A -= B;
			}
			Trigonometry::Vector4 Vector4Mul1(Trigonometry::Vector4& A, const Trigonometry::Vector4& B)
			{
				return A * B;
			}
			Trigonometry::Vector4 Vector4Mul2(Trigonometry::Vector4& A, float B)
			{
				return A * B;
			}
			Trigonometry::Vector4 Vector4Div1(Trigonometry::Vector4& A, const Trigonometry::Vector4& B)
			{
				return A / B;
			}
			Trigonometry::Vector4 Vector4Div2(Trigonometry::Vector4& A, float B)
			{
				return A / B;
			}
			Trigonometry::Vector4 Vector4Add1(Trigonometry::Vector4& A, const Trigonometry::Vector4& B)
			{
				return A + B;
			}
			Trigonometry::Vector4 Vector4Add2(Trigonometry::Vector4& A, float B)
			{
				return A + B;
			}
			Trigonometry::Vector4 Vector4Sub1(Trigonometry::Vector4& A, const Trigonometry::Vector4& B)
			{
				return A - B;
			}
			Trigonometry::Vector4 Vector4Sub2(Trigonometry::Vector4& A, float B)
			{
				return A - B;
			}
			Trigonometry::Vector4 Vector4Neg(Trigonometry::Vector4& A)
			{
				return -A;
			}
			bool Vector4Eq(Trigonometry::Vector4& A, const Trigonometry::Vector4& B)
			{
				return A == B;
			}
			int Vector4Cmp(Trigonometry::Vector4& A, const Trigonometry::Vector4& B)
			{
				if (A == B)
					return 0;

				return A > B ? 1 : -1;
			}

			Trigonometry::Matrix4x4 Matrix4x4Mul1(Trigonometry::Matrix4x4& A, const Trigonometry::Matrix4x4& B)
			{
				return A * B;
			}
			Trigonometry::Vector4 Matrix4x4Mul2(Trigonometry::Matrix4x4& A, const Trigonometry::Vector4& B)
			{
				return A * B;
			}
			bool Matrix4x4Eq(Trigonometry::Matrix4x4& A, const Trigonometry::Matrix4x4& B)
			{
				return A == B;
			}
			float& Matrix4x4GetRow(Trigonometry::Matrix4x4& Base, size_t Index)
			{
				return Base.Row[Index % 16];
			}

			Trigonometry::Vector3 QuaternionMul1(Trigonometry::Quaternion& A, const Trigonometry::Vector3& B)
			{
				return A * B;
			}
			Trigonometry::Quaternion QuaternionMul2(Trigonometry::Quaternion& A, const Trigonometry::Quaternion& B)
			{
				return A * B;
			}
			Trigonometry::Quaternion QuaternionMul3(Trigonometry::Quaternion& A, float B)
			{
				return A * B;
			}
			Trigonometry::Quaternion QuaternionAdd(Trigonometry::Quaternion& A, const Trigonometry::Quaternion& B)
			{
				return A + B;
			}
			Trigonometry::Quaternion QuaternionSub(Trigonometry::Quaternion& A, const Trigonometry::Quaternion& B)
			{
				return A - B;
			}

			Trigonometry::Vector4& Frustum8CGetCorners(Trigonometry::Frustum8C& Base, size_t Index)
			{
				return Base.Corners[Index % 8];
			}

			Trigonometry::Vector4& Frustum6PGetCorners(Trigonometry::Frustum6P& Base, size_t Index)
			{
				return Base.Planes[Index % 6];
			}

			size_t JointSize(Trigonometry::Joint& Base)
			{
				return Base.Childs.size();
			}
			Trigonometry::Joint& JointGetChilds(Trigonometry::Joint& Base, size_t Index)
			{
				if (Base.Childs.empty())
					return Base;

				return Base.Childs[Index % Base.Childs.size()];
			}

			size_t SkinAnimatorKeySize(Trigonometry::SkinAnimatorKey& Base)
			{
				return Base.Pose.size();
			}
			Trigonometry::AnimatorKey& SkinAnimatorKeyGetPose(Trigonometry::SkinAnimatorKey& Base, size_t Index)
			{
				if (Base.Pose.empty())
				{
					Base.Pose.push_back(Trigonometry::AnimatorKey());
					return Base.Pose.front();
				}

				return Base.Pose[Index % Base.Pose.size()];
			}

			size_t SkinAnimatorClipSize(Trigonometry::SkinAnimatorClip& Base)
			{
				return Base.Keys.size();
			}
			Trigonometry::SkinAnimatorKey& SkinAnimatorClipGetKeys(Trigonometry::SkinAnimatorClip& Base, size_t Index)
			{
				if (Base.Keys.empty())
				{
					Base.Keys.push_back(Trigonometry::SkinAnimatorKey());
					return Base.Keys.front();
				}

				return Base.Keys[Index % Base.Keys.size()];
			}

			size_t KeyAnimatorClipSize(Trigonometry::KeyAnimatorClip& Base)
			{
				return Base.Keys.size();
			}
			Trigonometry::AnimatorKey& KeyAnimatorClipGetKeys(Trigonometry::KeyAnimatorClip& Base, size_t Index)
			{
				if (Base.Keys.empty())
				{
					Base.Keys.push_back(Trigonometry::AnimatorKey());
					return Base.Keys.front();
				}

				return Base.Keys[Index % Base.Keys.size()];
			}

			void CosmosQueryIndex(Trigonometry::Cosmos* Base, asIScriptFunction* Overlaps, asIScriptFunction* Match)
			{
				ImmediateContext* Context = ImmediateContext::Get();
				FunctionDelegate OverlapsCallback(Overlaps);
				FunctionDelegate MatchCallback(Match);
				if (!Context || !OverlapsCallback.IsValid() || !MatchCallback.IsValid())
					return;

				Trigonometry::Cosmos::Iterator Iterator;
				Base->QueryIndex<void>(Iterator, [Context, &OverlapsCallback](const Trigonometry::Bounding& Box)
				{
					bool Result = false;
					Context->ExecuteSubcall(OverlapsCallback.Callable(), [&Box](ImmediateContext* Context)
					{
						Context->SetArgObject(0, (void*)&Box);
					}, [&Result](ImmediateContext* Context)
					{
						Result = (bool)Context->GetReturnByte();
					});
					return Result;
				}, [Context, &MatchCallback](void* Item)
				{
					Context->ExecuteSubcall(MatchCallback.Callable(), [Item](ImmediateContext* Context) { Context->SetArgAddress(0, Item); });
				});
			}

			Array* HullShapeGetVertices(Physics::HullShape* Base)
			{
				TypeInfo Type = VirtualMachine::Get()->GetTypeInfoByDecl(TYPENAME_ARRAY "<" TYPENAME_VERTEX ">@");
				return Array::Compose(Type.GetTypeInfo(), Base->GetVertices());
			}
			Array* HullShapeGetIndices(Physics::HullShape* Base)
			{
				TypeInfo Type = VirtualMachine::Get()->GetTypeInfoByDecl(TYPENAME_ARRAY "<int>@");
				return Array::Compose(Type.GetTypeInfo(), Base->GetIndices());
			}

			Physics::SoftBody::Desc::CV::SConvex& SoftBodySConvexCopy(Physics::SoftBody::Desc::CV::SConvex& Base, Physics::SoftBody::Desc::CV::SConvex& Other)
			{
				if (&Base == &Other)
					return Base;

				Core::Memory::Release(Base.Hull);
				Base = Other;
				if (Base.Hull != nullptr)
					Base.Hull->AddRef();

				return Base;
			}
			void SoftBodySConvexDestructor(Physics::SoftBody::Desc::CV::SConvex& Base)
			{
				Core::Memory::Release(Base.Hull);
			}

			Array* SoftBodyGetIndices(Physics::SoftBody* Base)
			{
				Core::Vector<int> Result;
				Base->GetIndices(&Result);

				TypeInfo Type = VirtualMachine::Get()->GetTypeInfoByDecl(TYPENAME_ARRAY "<int>@");
				return Array::Compose(Type.GetTypeInfo(), Result);
			}
			Array* SoftBodyGetVertices(Physics::SoftBody* Base)
			{
				Core::Vector<Trigonometry::Vertex> Result;
				Base->GetVertices(&Result);

				TypeInfo Type = VirtualMachine::Get()->GetTypeInfoByDecl(TYPENAME_ARRAY "<" TYPENAME_VERTEX ">@");
				return Array::Compose(Type.GetTypeInfo(), Result);
			}

			btCollisionShape* SimulatorCreateConvexHullSkinVertex(Physics::Simulator* Base, Array* Data)
			{
				auto Value = Array::Decompose<Trigonometry::SkinVertex>(Data);
				return Base->CreateConvexHull(Value);
			}
			btCollisionShape* SimulatorCreateConvexHullVertex(Physics::Simulator* Base, Array* Data)
			{
				auto Value = Array::Decompose<Trigonometry::Vertex>(Data);
				return Base->CreateConvexHull(Value);
			}
			btCollisionShape* SimulatorCreateConvexHullVector2(Physics::Simulator* Base, Array* Data)
			{
				auto Value = Array::Decompose<Trigonometry::Vector2>(Data);
				return Base->CreateConvexHull(Value);
			}
			btCollisionShape* SimulatorCreateConvexHullVector3(Physics::Simulator* Base, Array* Data)
			{
				auto Value = Array::Decompose<Trigonometry::Vector3>(Data);
				return Base->CreateConvexHull(Value);
			}
			btCollisionShape* SimulatorCreateConvexHullVector4(Physics::Simulator* Base, Array* Data)
			{
				auto Value = Array::Decompose<Trigonometry::Vector4>(Data);
				return Base->CreateConvexHull(Value);
			}
			Array* SimulatorGetShapeVertices(Physics::Simulator* Base, btCollisionShape* Shape)
			{
				TypeInfo Type = VirtualMachine::Get()->GetTypeInfoByDecl(TYPENAME_ARRAY "<" TYPENAME_VECTOR3 ">@");
				return Array::Compose(Type.GetTypeInfo(), Base->GetShapeVertices(Shape));
			}

			void AudioEffectSetFilter(Audio::AudioEffect* Base, Audio::AudioFilter* New)
			{
				Audio::AudioFilter* Copy = New;
				ExpectsWrapper::UnwrapVoid(Base->SetFilter(&Copy));
			}

			void AlertResult(Graphics::Alert& Base, asIScriptFunction* Callback)
			{
				ImmediateContext* Context = ImmediateContext::Get();
				FunctionDelegate Delegate(Callback);
				if (!Context || !Delegate.IsValid())
					return Base.Result(nullptr);

				Base.Result([Context, Delegate](int Id)
				{
					Context->ExecuteSubcall(Delegate.Callable(), [Id](ImmediateContext* Context)
					{
						Context->SetArg32(0, Id);
					});
				});
			}

			void ActivitySetAppStateChange(Graphics::Activity* Base, asIScriptFunction* Callback)
			{
				ImmediateContext* Context = ImmediateContext::Get();
				if (Context != nullptr && Callback != nullptr)
				{
					FunctionDelegate Delegate(Callback);
					Base->Callbacks.AppStateChange = [Context, Delegate](Graphics::AppState Type)
					{
						Context->ExecuteSubcall(Delegate.Callable(), [Type](ImmediateContext* Context)
						{
							Context->SetArg32(0, (int)Type);
						});
					};
				}
				else
					Base->Callbacks.AppStateChange = nullptr;
			}
			void ActivitySetWindowStateChange(Graphics::Activity* Base, asIScriptFunction* Callback)
			{
				ImmediateContext* Context = ImmediateContext::Get();
				if (Context != nullptr && Callback != nullptr)
				{
					FunctionDelegate Delegate(Callback);
					Base->Callbacks.WindowStateChange = [Context, Delegate](Graphics::WindowState State, int X, int Y)
					{
						Context->ExecuteSubcall(Delegate.Callable(), [State, X, Y](ImmediateContext* Context)
						{
							Context->SetArg32(0, (int)State);
							Context->SetArg32(1, X);
							Context->SetArg32(2, Y);
						});
					};
				}
				else
					Base->Callbacks.WindowStateChange = nullptr;
			}
			void ActivitySetKeyState(Graphics::Activity* Base, asIScriptFunction* Callback)
			{
				ImmediateContext* Context = ImmediateContext::Get();
				if (Context != nullptr && Callback != nullptr)
				{
					FunctionDelegate Delegate(Callback);
					Base->Callbacks.KeyState = [Context, Delegate](Graphics::KeyCode Code, Graphics::KeyMod Mod, int X, int Y, bool Value)
					{
						Context->ExecuteSubcall(Delegate.Callable(), [Code, Mod, X, Y, Value](ImmediateContext* Context)
						{
							Context->SetArg32(0, (int)Code);
							Context->SetArg32(1, (int)Mod);
							Context->SetArg32(2, X);
							Context->SetArg32(3, Y);
							Context->SetArg8(4, Value);
						});
					};
				}
				else
					Base->Callbacks.KeyState = nullptr;
			}
			void ActivitySetInputEdit(Graphics::Activity* Base, asIScriptFunction* Callback)
			{
				ImmediateContext* Context = ImmediateContext::Get();
				if (Context != nullptr && Callback != nullptr)
				{
					FunctionDelegate Delegate(Callback);
					Base->Callbacks.InputEdit = [Context, Delegate](char* Buffer, int X, int Y)
					{
						std::string_view Text = (Buffer ? Buffer : std::string_view());
						Context->ExecuteSubcall(Delegate.Callable(), [Text, X, Y](ImmediateContext* Context)
						{
							Context->SetArgObject(0, (void*)&Text);
							Context->SetArg32(1, X);
							Context->SetArg32(2, Y);
						});
					};
				}
				else
					Base->Callbacks.InputEdit = nullptr;
			}
			void ActivitySetInput(Graphics::Activity* Base, asIScriptFunction* Callback)
			{
				ImmediateContext* Context = ImmediateContext::Get();
				if (Context != nullptr && Callback != nullptr)
				{
					FunctionDelegate Delegate(Callback);
					Base->Callbacks.Input = [Context, Delegate](char* Buffer, int X)
					{
						std::string_view Text = (Buffer ? Buffer : std::string_view());
						Context->ExecuteSubcall(Delegate.Callable(), [Text, X](ImmediateContext* Context)
						{
							Context->SetArgObject(0, (void*)&Text);
							Context->SetArg32(1, X);
						});
					};
				}
				else
					Base->Callbacks.Input = nullptr;
			}
			void ActivitySetCursorMove(Graphics::Activity* Base, asIScriptFunction* Callback)
			{
				ImmediateContext* Context = ImmediateContext::Get();
				if (Context != nullptr && Callback != nullptr)
				{
					FunctionDelegate Delegate(Callback);
					Base->Callbacks.CursorMove = [Context, Delegate](int X, int Y, int Z, int W)
					{
						Context->ExecuteSubcall(Delegate.Callable(), [X, Y, Z, W](ImmediateContext* Context)
						{
							Context->SetArg32(0, X);
							Context->SetArg32(1, Y);
							Context->SetArg32(2, Z);
							Context->SetArg32(3, W);
						});
					};
				}
				else
					Base->Callbacks.CursorMove = nullptr;
			}
			void ActivitySetCursorWheelState(Graphics::Activity* Base, asIScriptFunction* Callback)
			{
				ImmediateContext* Context = ImmediateContext::Get();
				if (Context != nullptr && Callback != nullptr)
				{
					FunctionDelegate Delegate(Callback);
					Base->Callbacks.CursorWheelState = [Context, Delegate](int X, int Y, bool Z)
					{
						Context->ExecuteSubcall(Delegate.Callable(), [X, Y, Z](ImmediateContext* Context)
						{
							Context->SetArg32(0, X);
							Context->SetArg32(1, Y);
							Context->SetArg8(2, Z);
						});
					};
				}
				else
					Base->Callbacks.CursorWheelState = nullptr;
			}
			void ActivitySetJoyStickAxisMove(Graphics::Activity* Base, asIScriptFunction* Callback)
			{
				ImmediateContext* Context = ImmediateContext::Get();
				if (Context != nullptr && Callback != nullptr)
				{
					FunctionDelegate Delegate(Callback);
					Base->Callbacks.JoyStickAxisMove = [Context, Delegate](int X, int Y, int Z)
					{
						Context->ExecuteSubcall(Delegate.Callable(), [X, Y, Z](ImmediateContext* Context)
						{
							Context->SetArg32(0, X);
							Context->SetArg32(1, Y);
							Context->SetArg32(2, Z);
						});
					};
				}
				else
					Base->Callbacks.JoyStickAxisMove = nullptr;
			}
			void ActivitySetJoyStickBallMove(Graphics::Activity* Base, asIScriptFunction* Callback)
			{
				ImmediateContext* Context = ImmediateContext::Get();
				if (Context != nullptr && Callback != nullptr)
				{
					FunctionDelegate Delegate(Callback);
					Base->Callbacks.JoyStickBallMove = [Context, Delegate](int X, int Y, int Z, int W)
					{
						Context->ExecuteSubcall(Delegate.Callable(), [X, Y, Z, W](ImmediateContext* Context)
						{
							Context->SetArg32(0, X);
							Context->SetArg32(1, Y);
							Context->SetArg32(2, Z);
							Context->SetArg32(3, W);
						});
					};
				}
				else
					Base->Callbacks.JoyStickBallMove = nullptr;
			}
			void ActivitySetJoyStickHatMove(Graphics::Activity* Base, asIScriptFunction* Callback)
			{
				ImmediateContext* Context = ImmediateContext::Get();
				if (Context != nullptr && Callback != nullptr)
				{
					FunctionDelegate Delegate(Callback);
					Base->Callbacks.JoyStickHatMove = [Context, Delegate](Graphics::JoyStickHat Type, int X, int Y)
					{
						Context->ExecuteSubcall(Delegate.Callable(), [Type, X, Y](ImmediateContext* Context)
						{
							Context->SetArg32(0, (int)Type);
							Context->SetArg32(1, X);
							Context->SetArg32(2, Y);
						});
					};
				}
				else
					Base->Callbacks.JoyStickHatMove = nullptr;
			}
			void ActivitySetJoyStickKeyState(Graphics::Activity* Base, asIScriptFunction* Callback)
			{
				ImmediateContext* Context = ImmediateContext::Get();
				if (Context != nullptr && Callback != nullptr)
				{
					FunctionDelegate Delegate(Callback);
					Base->Callbacks.JoyStickKeyState = [Context, Delegate](int X, int Y, bool Z)
					{
						Context->ExecuteSubcall(Delegate.Callable(), [X, Y, Z](ImmediateContext* Context)
						{
							Context->SetArg32(0, X);
							Context->SetArg32(1, Y);
							Context->SetArg8(2, Z);
						});
					};
				}
				else
					Base->Callbacks.JoyStickKeyState = nullptr;
			}
			void ActivitySetJoyStickState(Graphics::Activity* Base, asIScriptFunction* Callback)
			{
				ImmediateContext* Context = ImmediateContext::Get();
				if (Context != nullptr && Callback != nullptr)
				{
					FunctionDelegate Delegate(Callback);
					Base->Callbacks.JoyStickState = [Context, Delegate](int X, bool Y)
					{
						Context->ExecuteSubcall(Delegate.Callable(), [X, Y](ImmediateContext* Context)
						{
							Context->SetArg32(0, X);
							Context->SetArg8(1, Y);
						});
					};
				}
				else
					Base->Callbacks.JoyStickState = nullptr;
			}
			void ActivitySetControllerAxisMove(Graphics::Activity* Base, asIScriptFunction* Callback)
			{
				ImmediateContext* Context = ImmediateContext::Get();
				if (Context != nullptr && Callback != nullptr)
				{
					FunctionDelegate Delegate(Callback);
					Base->Callbacks.ControllerAxisMove = [Context, Delegate](int X, int Y, int Z)
					{
						Context->ExecuteSubcall(Delegate.Callable(), [X, Y, Z](ImmediateContext* Context)
						{
							Context->SetArg32(0, X);
							Context->SetArg32(1, Y);
							Context->SetArg32(2, Z);
						});
					};
				}
				else
					Base->Callbacks.ControllerAxisMove = nullptr;
			}
			void ActivitySetControllerKeyState(Graphics::Activity* Base, asIScriptFunction* Callback)
			{
				ImmediateContext* Context = ImmediateContext::Get();
				if (Context != nullptr && Callback != nullptr)
				{
					FunctionDelegate Delegate(Callback);
					Base->Callbacks.ControllerKeyState = [Context, Delegate](int X, int Y, bool Z)
					{
						Context->ExecuteSubcall(Delegate.Callable(), [X, Y, Z](ImmediateContext* Context)
						{
							Context->SetArg32(0, X);
							Context->SetArg32(1, Y);
							Context->SetArg8(2, Z);
						});
					};
				}
				else
					Base->Callbacks.ControllerKeyState = nullptr;
			}
			void ActivitySetControllerState(Graphics::Activity* Base, asIScriptFunction* Callback)
			{
				ImmediateContext* Context = ImmediateContext::Get();
				if (Context != nullptr && Callback != nullptr)
				{
					FunctionDelegate Delegate(Callback);
					Base->Callbacks.ControllerState = [Context, Delegate](int X, int Y)
					{
						Context->ExecuteSubcall(Delegate.Callable(), [X, Y](ImmediateContext* Context)
						{
							Context->SetArg32(0, X);
							Context->SetArg32(1, Y);
						});
					};
				}
				else
					Base->Callbacks.ControllerState = nullptr;
			}
			void ActivitySetTouchMove(Graphics::Activity* Base, asIScriptFunction* Callback)
			{
				ImmediateContext* Context = ImmediateContext::Get();
				if (Context != nullptr && Callback != nullptr)
				{
					FunctionDelegate Delegate(Callback);
					Base->Callbacks.TouchMove = [Context, Delegate](int X, int Y, float Z, float W, float X1, float Y1, float Z1)
					{
						Context->ExecuteSubcall(Delegate.Callable(), [X, Y, Z, W, X1, Y1, Z1](ImmediateContext* Context)
						{
							Context->SetArg32(0, X);
							Context->SetArg32(1, Y);
							Context->SetArgFloat(2, Z);
							Context->SetArgFloat(3, W);
							Context->SetArgFloat(4, X1);
							Context->SetArgFloat(5, Y1);
							Context->SetArgFloat(6, Z1);
						});
					};
				}
				else
					Base->Callbacks.TouchMove = nullptr;
			}
			void ActivitySetTouchState(Graphics::Activity* Base, asIScriptFunction* Callback)
			{
				ImmediateContext* Context = ImmediateContext::Get();
				if (Context != nullptr && Callback != nullptr)
				{
					FunctionDelegate Delegate(Callback);
					Base->Callbacks.TouchState = [Context, Delegate](int X, int Y, float Z, float W, float X1, float Y1, float Z1, bool W1)
					{
						Context->ExecuteSubcall(Delegate.Callable(), [X, Y, Z, W, X1, Y1, Z1, W1](ImmediateContext* Context)
						{
							Context->SetArg32(0, X);
							Context->SetArg32(1, Y);
							Context->SetArgFloat(2, Z);
							Context->SetArgFloat(3, W);
							Context->SetArgFloat(4, X1);
							Context->SetArgFloat(5, Y1);
							Context->SetArgFloat(6, Z1);
							Context->SetArg8(7, W1);
						});
					};
				}
				else
					Base->Callbacks.TouchState = nullptr;
			}
			void ActivitySetGestureState(Graphics::Activity* Base, asIScriptFunction* Callback)
			{
				ImmediateContext* Context = ImmediateContext::Get();
				if (Context != nullptr && Callback != nullptr)
				{
					FunctionDelegate Delegate(Callback);
					Base->Callbacks.GestureState = [Context, Delegate](int X, int Y, int Z, float W, float X1, float Y1, bool Z1)
					{
						Context->ExecuteSubcall(Delegate.Callable(), [X, Y, Z, W, X1, Y1, Z1](ImmediateContext* Context)
						{
							Context->SetArg32(0, X);
							Context->SetArg32(1, Y);
							Context->SetArg32(2, Z);
							Context->SetArgFloat(3, W);
							Context->SetArgFloat(4, X1);
							Context->SetArgFloat(5, Y1);
							Context->SetArg8(6, Z1);
						});
					};
				}
				else
					Base->Callbacks.GestureState = nullptr;
			}
			void ActivitySetMultiGestureState(Graphics::Activity* Base, asIScriptFunction* Callback)
			{
				ImmediateContext* Context = ImmediateContext::Get();
				if (Context != nullptr && Callback != nullptr)
				{
					FunctionDelegate Delegate(Callback);
					Base->Callbacks.MultiGestureState = [Context, Delegate](int X, int Y, float Z, float W, float X1, float Y1)
					{
						Context->ExecuteSubcall(Delegate.Callable(), [X, Y, Z, W, X1, Y1](ImmediateContext* Context)
						{
							Context->SetArg32(0, X);
							Context->SetArg32(1, Y);
							Context->SetArgFloat(2, Z);
							Context->SetArgFloat(3, W);
							Context->SetArgFloat(4, X1);
							Context->SetArgFloat(5, Y1);
						});
					};
				}
				else
					Base->Callbacks.MultiGestureState = nullptr;
			}
			void ActivitySetDropFile(Graphics::Activity* Base, asIScriptFunction* Callback)
			{
				ImmediateContext* Context = ImmediateContext::Get();
				if (Context != nullptr && Callback != nullptr)
				{
					FunctionDelegate Delegate(Callback);
					Base->Callbacks.DropFile = [Context, Delegate](const std::string_view& Text)
					{
						Context->ExecuteSubcall(Delegate.Callable(), [Text](ImmediateContext* Context)
						{
							Context->SetArgObject(0, (void*)&Text);
						});
					};
				}
				else
					Base->Callbacks.DropFile = nullptr;
			}
			void ActivitySetDropText(Graphics::Activity* Base, asIScriptFunction* Callback)
			{
				ImmediateContext* Context = ImmediateContext::Get();
				if (Context != nullptr && Callback != nullptr)
				{
					FunctionDelegate Delegate(Callback);
					Base->Callbacks.DropText = [Context, Delegate](const std::string_view& Text)
					{
						Context->ExecuteSubcall(Delegate.Callable(), [Text](ImmediateContext* Context)
						{
							Context->SetArgObject(0, (void*)&Text);
						});
					};
				}
				else
					Base->Callbacks.DropText = nullptr;
			}

			Graphics::RenderTargetBlendState& BlendStateDescGetRenderTarget(Graphics::BlendState::Desc& Base, size_t Index)
			{
				return Base.RenderTarget[Index % 8];
			}

			float& SamplerStateDescGetBorderColor(Graphics::SamplerState::Desc& Base, size_t Index)
			{
				return Base.BorderColor[Index % 4];
			}

			Graphics::InputLayout::Desc& InputLayoutDescCopy(Graphics::InputLayout::Desc& Base, Graphics::InputLayout::Desc& Other)
			{
				if (&Base == &Other)
					return Base;

				Core::Memory::Release(Base.Source);
				Base = Other;
				if (Base.Source != nullptr)
					Base.Source->AddRef();

				return Base;
			}
			void InputLayoutDescDestructor(Graphics::InputLayout::Desc& Base)
			{
				Core::Memory::Release(Base.Source);
			}
			void InputLayoutDescSetAttributes(Graphics::InputLayout::Desc& Base, Array* Data)
			{
				Base.Attributes = Array::Decompose<Graphics::InputLayout::Attribute>(Data);
			}

			Array* InputLayoutGetAttributes(Graphics::InputLayout* Base)
			{
				TypeInfo Type = VirtualMachine::Get()->GetTypeInfoByDecl(TYPENAME_ARRAY "<" TYPENAME_INPUTLAYOUTATTRIBUTE ">@");
				return Array::Compose(Type.GetTypeInfo(), Base->GetAttributes());
			}

			void ShaderDescSetDefines(Graphics::Shader::Desc& Base, Array* Data)
			{
				Base.Defines = Array::Decompose<Core::String>(Data);
			}

			void MeshBufferDescSetElements(Graphics::MeshBuffer::Desc& Base, Array* Data)
			{
				Base.Elements = Array::Decompose<Trigonometry::Vertex>(Data);
			}
			void MeshBufferDescSetIndices(Graphics::MeshBuffer::Desc& Base, Array* Data)
			{
				Base.Indices = Array::Decompose<int>(Data);
			}

			void SkinMeshBufferDescSetElements(Graphics::SkinMeshBuffer::Desc& Base, Array* Data)
			{
				Base.Elements = Array::Decompose<Trigonometry::SkinVertex>(Data);
			}
			void SkinMeshBufferDescSetIndices(Graphics::SkinMeshBuffer::Desc& Base, Array* Data)
			{
				Base.Indices = Array::Decompose<int>(Data);
			}

			Graphics::InstanceBuffer::Desc& InstanceBufferDescCopy(Graphics::InstanceBuffer::Desc& Base, Graphics::InstanceBuffer::Desc& Other)
			{
				if (&Base == &Other)
					return Base;

				Core::Memory::Release(Base.Device);
				Base = Other;
				if (Base.Device != nullptr)
					Base.Device->AddRef();

				return Base;
			}
			void InstanceBufferDescDestructor(Graphics::InstanceBuffer::Desc& Base)
			{
				Core::Memory::Release(Base.Device);
			}

			void InstanceBufferSetArray(Graphics::InstanceBuffer* Base, Array* Data)
			{
				auto& Source = Base->GetArray();
				Source = Array::Decompose<Trigonometry::ElementVertex>(Data);
			}
			Array* InstanceBufferGetArray(Graphics::InstanceBuffer* Base)
			{
				TypeInfo Type = VirtualMachine::Get()->GetTypeInfoByDecl(TYPENAME_ARRAY "<" TYPENAME_ELEMENTVERTEX ">@");
				return Array::Compose(Type.GetTypeInfo(), Base->GetArray());
			}

			void MultiRenderTarget2DDescSetFormatMode(Graphics::MultiRenderTarget2D::Desc& Base, size_t Index, Graphics::Format Mode)
			{
				Base.FormatMode[Index % 8] = Mode;
			}

			void MultiRenderTargetCubeDescSetFormatMode(Graphics::MultiRenderTargetCube::Desc& Base, size_t Index, Graphics::Format Mode)
			{
				Base.FormatMode[Index % 8] = Mode;
			}

			Graphics::Cubemap::Desc& CubemapDescCopy(Graphics::Cubemap::Desc& Base, Graphics::Cubemap::Desc& Other)
			{
				if (&Base == &Other)
					return Base;

				Core::Memory::Release(Base.Source);
				Base = Other;
				if (Base.Source != nullptr)
					Base.Source->AddRef();

				return Base;
			}
			void CubemapDescDestructor(Graphics::Cubemap::Desc& Base)
			{
				Core::Memory::Release(Base.Source);
			}

			Graphics::GraphicsDevice::Desc& GraphicsDeviceDescCopy(Graphics::GraphicsDevice::Desc& Base, Graphics::GraphicsDevice::Desc& Other)
			{
				if (&Base == &Other)
					return Base;

				Core::Memory::Release(Base.Window);
				Base = Other;
				if (Base.Window != nullptr)
					Base.Window->AddRef();

				return Base;
			}
			void GraphicsDeviceDescDestructor(Graphics::GraphicsDevice::Desc& Base)
			{
				Core::Memory::Release(Base.Window);
			}

			void GraphicsDeviceSetVertexBuffers(Graphics::GraphicsDevice* Base, Array* Data, bool Value)
			{
				Core::Vector<Graphics::ElementBuffer*> Buffer = Array::Decompose<Graphics::ElementBuffer*>(Data);
				Base->SetVertexBuffers(Buffer.data(), (uint32_t)Buffer.size(), Value);
			}
			void GraphicsDeviceSetWriteable1(Graphics::GraphicsDevice* Base, Array* Data, uint32_t Slot, bool Value)
			{
				Core::Vector<Graphics::ElementBuffer*> Buffer = Array::Decompose<Graphics::ElementBuffer*>(Data);
				Base->SetWriteable(Buffer.data(), Slot, (uint32_t)Buffer.size(), Value);
			}
			void GraphicsDeviceSetWriteable2(Graphics::GraphicsDevice* Base, Array* Data, uint32_t Slot, bool Value)
			{
				Core::Vector<Graphics::Texture2D*> Buffer = Array::Decompose<Graphics::Texture2D*>(Data);
				Base->SetWriteable(Buffer.data(), Slot, (uint32_t)Buffer.size(), Value);
			}
			void GraphicsDeviceSetWriteable3(Graphics::GraphicsDevice* Base, Array* Data, uint32_t Slot, bool Value)
			{
				Core::Vector<Graphics::Texture3D*> Buffer = Array::Decompose<Graphics::Texture3D*>(Data);
				Base->SetWriteable(Buffer.data(), Slot, (uint32_t)Buffer.size(), Value);
			}
			void GraphicsDeviceSetWriteable4(Graphics::GraphicsDevice* Base, Array* Data, uint32_t Slot, bool Value)
			{
				Core::Vector<Graphics::TextureCube*> Buffer = Array::Decompose<Graphics::TextureCube*>(Data);
				Base->SetWriteable(Buffer.data(), Slot, (uint32_t)Buffer.size(), Value);
			}
			void GraphicsDeviceSetTargetMap(Graphics::GraphicsDevice* Base, Graphics::RenderTarget* Target, Array* Data)
			{
				Core::Vector<bool> Buffer = Array::Decompose<bool>(Data);
				while (Buffer.size() < 8)
					Buffer.push_back(false);

				bool Map[8];
				for (size_t i = 0; i < 8; i++)
					Map[i] = Buffer[i];

				Base->SetTargetMap(Target, Map);
			}
			void GraphicsDeviceSetViewports(Graphics::GraphicsDevice* Base, Array* Data)
			{
				Core::Vector<Graphics::Viewport> Buffer = Array::Decompose<Graphics::Viewport>(Data);
				Base->SetViewports((uint32_t)Buffer.size(), Buffer.data());
			}
			void GraphicsDeviceSetScissorRects(Graphics::GraphicsDevice* Base, Array* Data)
			{
				Core::Vector<Trigonometry::Rectangle> Buffer = Array::Decompose<Trigonometry::Rectangle>(Data);
				Base->SetScissorRects((uint32_t)Buffer.size(), Buffer.data());
			}
			bool GraphicsDeviceMap1(Graphics::GraphicsDevice* Base, Graphics::ElementBuffer* Resource, Graphics::ResourceMap Type, Graphics::MappedSubresource* Result)
			{
				return ExpectsWrapper::UnwrapVoid(Base->Map(Resource, Type, Result));
			}
			bool GraphicsDeviceMap2(Graphics::GraphicsDevice* Base, Graphics::Texture2D* Resource, Graphics::ResourceMap Type, Graphics::MappedSubresource* Result)
			{
				return ExpectsWrapper::UnwrapVoid(Base->Map(Resource, Type, Result));
			}
			bool GraphicsDeviceMap3(Graphics::GraphicsDevice* Base, Graphics::TextureCube* Resource, Graphics::ResourceMap Type, Graphics::MappedSubresource* Result)
			{
				return ExpectsWrapper::UnwrapVoid(Base->Map(Resource, Type, Result));
			}
			bool GraphicsDeviceMap4(Graphics::GraphicsDevice* Base, Graphics::Texture3D* Resource, Graphics::ResourceMap Type, Graphics::MappedSubresource* Result)
			{
				return ExpectsWrapper::UnwrapVoid(Base->Map(Resource, Type, Result));
			}
			bool GraphicsDeviceUnmap1(Graphics::GraphicsDevice* Base, Graphics::ElementBuffer* Resource, Graphics::MappedSubresource* Result)
			{
				return ExpectsWrapper::UnwrapVoid(Base->Unmap(Resource, Result));
			}
			bool GraphicsDeviceUnmap2(Graphics::GraphicsDevice* Base, Graphics::Texture2D* Resource, Graphics::MappedSubresource* Result)
			{
				return ExpectsWrapper::UnwrapVoid(Base->Unmap(Resource, Result));
			}
			bool GraphicsDeviceUnmap3(Graphics::GraphicsDevice* Base, Graphics::TextureCube* Resource, Graphics::MappedSubresource* Result)
			{
				return ExpectsWrapper::UnwrapVoid(Base->Unmap(Resource, Result));
			}
			bool GraphicsDeviceUnmap4(Graphics::GraphicsDevice* Base, Graphics::Texture3D* Resource, Graphics::MappedSubresource* Result)
			{
				return ExpectsWrapper::UnwrapVoid(Base->Unmap(Resource, Result));
			}
			bool GraphicsDeviceUpdateConstantBuffer(Graphics::GraphicsDevice* Base, Graphics::ElementBuffer* Resource, void* Data, size_t Size)
			{
				return ExpectsWrapper::UnwrapVoid(Base->UpdateConstantBuffer(Resource, Data, Size));
			}
			bool GraphicsDeviceUpdateBuffer1(Graphics::GraphicsDevice* Base, Graphics::ElementBuffer* Resource, void* Data, size_t Size)
			{
				return ExpectsWrapper::UnwrapVoid(Base->UpdateBuffer(Resource, Data, Size));
			}
			bool GraphicsDeviceUpdateBuffer2(Graphics::GraphicsDevice* Base, Graphics::Shader* Resource, const void* Data)
			{
				return ExpectsWrapper::UnwrapVoid(Base->UpdateBuffer(Resource, Data));
			}
			bool GraphicsDeviceUpdateBuffer3(Graphics::GraphicsDevice* Base, Graphics::MeshBuffer* Resource, Trigonometry::Vertex* Data)
			{
				return ExpectsWrapper::UnwrapVoid(Base->UpdateBuffer(Resource, Data));
			}
			bool GraphicsDeviceUpdateBuffer4(Graphics::GraphicsDevice* Base, Graphics::SkinMeshBuffer* Resource, Trigonometry::SkinVertex* Data)
			{
				return ExpectsWrapper::UnwrapVoid(Base->UpdateBuffer(Resource, Data));
			}
			bool GraphicsDeviceUpdateBuffer5(Graphics::GraphicsDevice* Base, Graphics::InstanceBuffer* Resource)
			{
				return ExpectsWrapper::UnwrapVoid(Base->UpdateBuffer(Resource));
			}
			bool GraphicsDeviceUpdateBufferSize1(Graphics::GraphicsDevice* Base, Graphics::Shader* Resource, size_t Size)
			{
				return ExpectsWrapper::UnwrapVoid(Base->UpdateBufferSize(Resource, Size));
			}
			bool GraphicsDeviceUpdateBufferSize2(Graphics::GraphicsDevice* Base, Graphics::InstanceBuffer* Resource, size_t Size)
			{
				return ExpectsWrapper::UnwrapVoid(Base->UpdateBufferSize(Resource, Size));
			}
			Array* GraphicsDeviceGetViewports(Graphics::GraphicsDevice* Base)
			{
				Core::Vector<Graphics::Viewport> Viewports;
				Viewports.resize(32);

				uint32_t Count = 0;
				Base->GetViewports(&Count, Viewports.data());
				Viewports.resize((size_t)Count);

				TypeInfo Type = VirtualMachine::Get()->GetTypeInfoByDecl(TYPENAME_ARRAY "<" TYPENAME_VIEWPORT ">@");
				return Array::Compose(Type.GetTypeInfo(), Viewports);
			}
			Array* GraphicsDeviceGetScissorRects(Graphics::GraphicsDevice* Base)
			{
				Core::Vector<Trigonometry::Rectangle> Rects;
				Rects.resize(32);

				uint32_t Count = 0;
				Base->GetScissorRects(&Count, Rects.data());
				Rects.resize((size_t)Count);

				TypeInfo Type = VirtualMachine::Get()->GetTypeInfoByDecl(TYPENAME_ARRAY "<" TYPENAME_RECTANGLE ">@");
				return Array::Compose(Type.GetTypeInfo(), Rects);
			}
			bool GraphicsDeviceGenerateTexture1(Graphics::GraphicsDevice* Base, Graphics::Texture2D* Resource)
			{
				return ExpectsWrapper::UnwrapVoid(Base->GenerateTexture(Resource));
			}
			bool GraphicsDeviceGenerateTexture2(Graphics::GraphicsDevice* Base, Graphics::Texture3D* Resource)
			{
				return ExpectsWrapper::UnwrapVoid(Base->GenerateTexture(Resource));
			}
			bool GraphicsDeviceGenerateTexture3(Graphics::GraphicsDevice* Base, Graphics::TextureCube* Resource)
			{
				return ExpectsWrapper::UnwrapVoid(Base->GenerateTexture(Resource));
			}
			bool GraphicsDeviceGetQueryData1(Graphics::GraphicsDevice* Base, Graphics::Query* Resource, size_t& Result, bool Flush)
			{
				return ExpectsWrapper::UnwrapVoid(Base->GetQueryData(Resource, &Result, Flush));
			}
			bool GraphicsDeviceGetQueryData2(Graphics::GraphicsDevice* Base, Graphics::Query* Resource, bool& Result, bool Flush)
			{
				return ExpectsWrapper::UnwrapVoid(Base->GetQueryData(Resource, &Result, Flush));
			}
			Graphics::MeshBuffer* GraphicsDeviceCreateMeshBuffer1(Graphics::GraphicsDevice* Base, const Graphics::MeshBuffer::Desc& Desc)
			{
				auto* Result = ExpectsWrapper::Unwrap(Base->CreateMeshBuffer(Desc), (Graphics::MeshBuffer*)nullptr);
				if (Result != nullptr)
					FunctionFactory::AtomicNotifyGC(TYPENAME_MESHBUFFER, Result);
				return Result;
			}
			Graphics::MeshBuffer* GraphicsDeviceCreateMeshBuffer2(Graphics::GraphicsDevice* Base, Graphics::ElementBuffer* VertexBuffer, Graphics::ElementBuffer* IndexBuffer)
			{
				auto* Result = ExpectsWrapper::Unwrap(Base->CreateMeshBuffer(VertexBuffer, IndexBuffer), (Graphics::MeshBuffer*)nullptr);
				if (!Result)
					return nullptr;

				if (VertexBuffer != nullptr)
					VertexBuffer->AddRef();

				if (IndexBuffer != nullptr)
					IndexBuffer->AddRef();

				FunctionFactory::AtomicNotifyGC(TYPENAME_MESHBUFFER, Result);
				return Result;
			}
			Graphics::SkinMeshBuffer* GraphicsDeviceCreateSkinMeshBuffer1(Graphics::GraphicsDevice* Base, const Graphics::SkinMeshBuffer::Desc& Desc)
			{
				auto* Result = ExpectsWrapper::Unwrap(Base->CreateSkinMeshBuffer(Desc), (Graphics::SkinMeshBuffer*)nullptr);
				if (Result != nullptr)
					FunctionFactory::AtomicNotifyGC(TYPENAME_SKINMESHBUFFER, Result);
				return Result;
			}
			Graphics::SkinMeshBuffer* GraphicsDeviceCreateSkinMeshBuffer2(Graphics::GraphicsDevice* Base, Graphics::ElementBuffer* VertexBuffer, Graphics::ElementBuffer* IndexBuffer)
			{
				auto* Result = ExpectsWrapper::Unwrap(Base->CreateSkinMeshBuffer(VertexBuffer, IndexBuffer), (Graphics::SkinMeshBuffer*)nullptr);
				if (!Result)
					return nullptr;

				if (VertexBuffer != nullptr)
					VertexBuffer->AddRef();

				if (IndexBuffer != nullptr)
					IndexBuffer->AddRef();

				FunctionFactory::AtomicNotifyGC(TYPENAME_SKINMESHBUFFER, Result);
				return Result;
			}
			Graphics::InstanceBuffer* GraphicsDeviceCreateInstanceBuffer(Graphics::GraphicsDevice* Base, const Graphics::InstanceBuffer::Desc& Desc)
			{
				auto* Result = ExpectsWrapper::Unwrap(Base->CreateInstanceBuffer(Desc), (Graphics::InstanceBuffer*)nullptr);
				if (Result != nullptr)
					FunctionFactory::AtomicNotifyGC(TYPENAME_INSTANCEBUFFER, Result);
				return Result;
			}
			Graphics::Texture2D* GraphicsDeviceCreateTexture2D1(Graphics::GraphicsDevice* Base)
			{
				return ExpectsWrapper::Unwrap(Base->CreateTexture2D(), (Graphics::Texture2D*)nullptr);
			}
			Graphics::Texture2D* GraphicsDeviceCreateTexture2D2(Graphics::GraphicsDevice* Base, const Graphics::Texture2D::Desc& Desc)
			{
				return ExpectsWrapper::Unwrap(Base->CreateTexture2D(Desc), (Graphics::Texture2D*)nullptr);
			}
			Graphics::Texture3D* GraphicsDeviceCreateTexture3D1(Graphics::GraphicsDevice* Base)
			{
				return ExpectsWrapper::Unwrap(Base->CreateTexture3D(), (Graphics::Texture3D*)nullptr);
			}
			Graphics::Texture3D* GraphicsDeviceCreateTexture3D2(Graphics::GraphicsDevice* Base, const Graphics::Texture3D::Desc& Desc)
			{
				return ExpectsWrapper::Unwrap(Base->CreateTexture3D(Desc), (Graphics::Texture3D*)nullptr);
			}
			Graphics::TextureCube* GraphicsDeviceCreateTextureCube1(Graphics::GraphicsDevice* Base)
			{
				return ExpectsWrapper::Unwrap(Base->CreateTextureCube(), (Graphics::TextureCube*)nullptr);
			}
			Graphics::TextureCube* GraphicsDeviceCreateTextureCube2(Graphics::GraphicsDevice* Base, const Graphics::TextureCube::Desc& Desc)
			{
				return ExpectsWrapper::Unwrap(Base->CreateTextureCube(Desc), (Graphics::TextureCube*)nullptr);
			}
			Graphics::TextureCube* GraphicsDeviceCreateTextureCube3(Graphics::GraphicsDevice* Base, Array* Data)
			{
				Core::Vector<Graphics::Texture2D*> Buffer = Array::Decompose<Graphics::Texture2D*>(Data);
				while (Buffer.size() < 6)
					Buffer.push_back(nullptr);

				Graphics::Texture2D* Map[6];
				for (size_t i = 0; i < 6; i++)
					Map[i] = Buffer[i];

				return ExpectsWrapper::Unwrap(Base->CreateTextureCube(Map), (Graphics::TextureCube*)nullptr);
			}
			Graphics::TextureCube* GraphicsDeviceCreateTextureCube4(Graphics::GraphicsDevice* Base, Graphics::Texture2D* Data)
			{
				return ExpectsWrapper::Unwrap(Base->CreateTextureCube(Data), (Graphics::TextureCube*)nullptr);
			}
			Graphics::Texture2D* GraphicsDeviceCopyTexture2D1(Graphics::GraphicsDevice* Base, Graphics::Texture2D* Source)
			{
				Graphics::Texture2D* Result = nullptr;
				ExpectsWrapper::UnwrapVoid(Base->CopyTexture2D(Source, &Result));
				return Result;
			}
			Graphics::Texture2D* GraphicsDeviceCopyTexture2D2(Graphics::GraphicsDevice* Base, Graphics::RenderTarget* Source, uint32_t Index)
			{
				Graphics::Texture2D* Result = nullptr;
				ExpectsWrapper::UnwrapVoid(Base->CopyTexture2D(Source, Index, &Result));
				return Result;
			}
			Graphics::Texture2D* GraphicsDeviceCopyTexture2D3(Graphics::GraphicsDevice* Base, Graphics::RenderTargetCube* Source, Trigonometry::CubeFace Face)
			{
				Graphics::Texture2D* Result = nullptr;
				ExpectsWrapper::UnwrapVoid(Base->CopyTexture2D(Source, Face, &Result));
				return Result;
			}
			Graphics::Texture2D* GraphicsDeviceCopyTexture2D4(Graphics::GraphicsDevice* Base, Graphics::MultiRenderTargetCube* Source, uint32_t Index, Trigonometry::CubeFace Face)
			{
				Graphics::Texture2D* Result = nullptr;
				ExpectsWrapper::UnwrapVoid(Base->CopyTexture2D(Source, Index, Face, &Result));
				return Result;
			}
			Graphics::TextureCube* GraphicsDeviceCopyTextureCube1(Graphics::GraphicsDevice* Base, Graphics::RenderTargetCube* Source)
			{
				Graphics::TextureCube* Result = nullptr;
				ExpectsWrapper::UnwrapVoid(Base->CopyTextureCube(Source, &Result));
				return Result;
			}
			Graphics::TextureCube* GraphicsDeviceCopyTextureCube2(Graphics::GraphicsDevice* Base, Graphics::MultiRenderTargetCube* Source, uint32_t Index)
			{
				Graphics::TextureCube* Result = nullptr;
				ExpectsWrapper::UnwrapVoid(Base->CopyTextureCube(Source, Index, &Result));
				return Result;
			}
			Graphics::Texture2D* GraphicsDeviceCopyBackBuffer(Graphics::GraphicsDevice* Base)
			{
				Graphics::Texture2D* Result = nullptr;
				ExpectsWrapper::UnwrapVoid(Base->CopyBackBuffer(&Result));
				return Result;
			}
			Graphics::GraphicsDevice* GraphicsDeviceCreate(Graphics::GraphicsDevice::Desc& Desc)
			{
				auto* Result = Graphics::GraphicsDevice::Create(Desc);
				FunctionFactory::AtomicNotifyGC(TYPENAME_GRAPHICSDEVICE, Result);
				return Result;
			}
			void GraphicsDeviceCompileBuiltinShaders(Array* Devices)
			{
				ExpectsWrapper::UnwrapVoid(Graphics::GraphicsDevice::CompileBuiltinShaders(Array::Decompose<Graphics::GraphicsDevice*>(Devices), nullptr));
			}

			Trigonometry::Matrix4x4& AnimationBufferGetOffsets(Layer::AnimationBuffer& Base, size_t Index)
			{
				return Base.Offsets[Index % Graphics::JOINTS_SIZE];
			}

			void PoseBufferSetOffset(Layer::PoseBuffer& Base, int64_t Index, const Layer::PoseData& Data)
			{
				Base.Offsets[Index] = Data;
			}
			void PoseBufferSetMatrix(Layer::PoseBuffer& Base, Graphics::SkinMeshBuffer* Mesh, size_t Index, const Trigonometry::Matrix4x4& Data)
			{
				Base.Matrices[Mesh].Data[Index % Graphics::JOINTS_SIZE] = Data;
			}
			Layer::PoseData& PoseBufferGetOffset(Layer::PoseBuffer& Base, int64_t Index)
			{
				return Base.Offsets[Index];
			}
			Trigonometry::Matrix4x4& PoseBufferGetMatrix(Layer::PoseBuffer& Base, Graphics::SkinMeshBuffer* Mesh, size_t Index)
			{
				return Base.Matrices[Mesh].Data[Index % Graphics::JOINTS_SIZE];
			}
			size_t PoseBufferGetOffsetsSize(Layer::PoseBuffer& Base)
			{
				return Base.Offsets.size();
			}
			size_t PoseBufferGetMatricesSize(Layer::PoseBuffer& Base, Graphics::SkinMeshBuffer* Mesh)
			{
				return Graphics::JOINTS_SIZE;
			}

			Array* ModelGetMeshes(Layer::Model* Base)
			{
				TypeInfo Type = VirtualMachine::Get()->GetTypeInfoByDecl(TYPENAME_ARRAY "<" TYPENAME_MESHBUFFER "@>@");
				return Array::Compose(Type.GetTypeInfo(), Base->Meshes);
			}
			void ModelSetMeshes(Layer::Model* Base, Array* Data)
			{
				Base->Meshes = Array::Decompose<Graphics::MeshBuffer*>(Data);
			}
			Array* SkinModelGetMeshes(Layer::SkinModel* Base)
			{
				TypeInfo Type = VirtualMachine::Get()->GetTypeInfoByDecl(TYPENAME_ARRAY "<" TYPENAME_SKINMESHBUFFER "@>@");
				return Array::Compose(Type.GetTypeInfo(), Base->Meshes);
			}
			void SkinModelSetMeshes(Layer::SkinModel* Base, Array* Data)
			{
				Base->Meshes = Array::Decompose<Graphics::SkinMeshBuffer*>(Data);
			}

			template <typename T>
			void PopulateAudioFilterBase(RefClass& Class, bool BaseCast = true)
			{
				if (BaseCast)
					Class.SetOperatorEx(Operators::Cast, 0, "void", "?&out", &HandleToHandleCast);

				Class.SetMethodEx("bool synchronize()", &VI_EXPECTIFY_VOID(T::Synchronize));
				Class.SetMethod("void deserialize(schema@+)", &T::Deserialize);
				Class.SetMethod("void serialize(schema@+)", &T::Serialize);
				Class.SetMethod("base_audio_filter@ copy()", &T::Copy);
				Class.SetMethod("audio_source@+ get_source()", &T::GetSource);
				PopulateComponent<T>(Class);
			}
			template <typename T, typename... Args>
			void PopulateAudioFilterInterface(RefClass& Class, const char* Constructor)
			{
				Class.SetConstructor<T, Args...>(Constructor);
				Class.SetDynamicCast<T, Audio::AudioFilter>("base_audio_filter@+", true);
				PopulateAudioFilterBase<T>(Class, false);
			}

			template <typename T>
			void PopulateAudioEffectBase(RefClass& Class, bool BaseCast = true)
			{
				if (BaseCast)
					Class.SetOperatorEx(Operators::Cast, 0, "void", "?&out", &HandleToHandleCast);

				Class.SetMethodEx("bool synchronize()", &VI_EXPECTIFY_VOID(T::Synchronize));
				Class.SetMethod("void deserialize(schema@+)", &T::Deserialize);
				Class.SetMethod("void serialize(schema@+)", &T::Serialize);
				Class.SetMethodEx("void set_filter(base_audio_filter@+)", &AudioEffectSetFilter);
				Class.SetMethod("base_audio_effect@ copy()", &T::Copy);
				Class.SetMethod("audio_source@+ get_filter()", &T::GetFilter);
				Class.SetMethod("audio_source@+ get_source()", &T::GetSource);
				PopulateComponent<T>(Class);
			}
			template <typename T, typename... Args>
			void PopulateAudioEffectInterface(RefClass& Class, const char* Constructor)
			{
				Class.SetConstructor<T, Args...>(Constructor);
				Class.SetDynamicCast<T, Audio::AudioEffect>("base_audio_effect@+", true);
				PopulateAudioEffectBase<T>(Class, false);
			}

			void EventSetArgs(Layer::Event& Base, Dictionary* Data)
			{
				TypeInfo Type = VirtualMachine::Get()->GetTypeInfoByDecl(TYPENAME_DICTIONARY "@");
				Base.Args = Dictionary::Decompose<Core::Variant>(Type.GetTypeId(), Data);
			}
			Dictionary* EventGetArgs(Layer::Event& Base)
			{
				TypeInfo Type = VirtualMachine::Get()->GetTypeInfoByDecl(TYPENAME_DICTIONARY "@");
				return Dictionary::Compose<Core::Variant>(Type.GetTypeId(), Base.Args);
			}

			Layer::Viewer& ViewerCopy(Layer::Viewer& Base, Layer::Viewer& Other)
			{
				if (&Base == &Other)
					return Base;

				Core::Memory::Release(Base.Renderer);
				Base = Other;
				if (Base.Renderer != nullptr)
					Base.Renderer->AddRef();

				return Base;
			}
			void ViewerDestructor(Layer::Viewer& Base)
			{
				Core::Memory::Release(Base.Renderer);
			}

			Array* SkinAnimationGetClips(Layer::SkinAnimation* Base)
			{
				TypeInfo Type = VirtualMachine::Get()->GetTypeInfoByDecl(TYPENAME_ARRAY "<" TYPENAME_SKINANIMATORCLIP ">@");
				return Array::Compose<Trigonometry::SkinAnimatorClip>(Type.GetTypeInfo(), Base->GetClips());
			}

			size_t SparseIndexGetSize(Layer::SparseIndex& Base)
			{
				return Base.Data.Size();
			}
			Layer::Component* SparseIndexGetData(Layer::SparseIndex& Base, size_t Index)
			{
				if (Index >= Base.Data.Size())
					return nullptr;

				return Base.Data[Index];
			}

			template <typename T>
			void ComponentMessage(T* Base, const std::string_view& Name, Core::Schema* Args)
			{
				auto Data = ToVariantKeys(Args);
				return Base->Message(Name, Data);
			}
			template <typename T>
			void PopulateComponentBase(RefClass& Class, bool BaseCast = true)
			{
				if (BaseCast)
					Class.SetOperatorEx(Operators::Cast, 0, "void", "?&out", &HandleToHandleCast);

				Class.SetMethod("void serialize(schema@+)", &Layer::Component::Serialize);
				Class.SetMethod("void deserialize(schema@+)", &Layer::Component::Deserialize);
				Class.SetMethod("void activate(base_component@+)", &Layer::Component::Activate);
				Class.SetMethod("void deactivate()", &Layer::Component::Deactivate);
				Class.SetMethod("void synchronize(clock_timer@+)", &Layer::Component::Synchronize);
				Class.SetMethod("void animate(clock_timer@+)", &Layer::Component::Animate);
				Class.SetMethod("void update(clock_timer@+)", &Layer::Component::Update);
				Class.SetMethodEx("void message(const string_view&in, schema@+)", &ComponentMessage<T>);
				Class.SetMethod("void movement()", &Layer::Component::Movement);
				Class.SetMethod("usize get_unit_bounds(vector3 &out, vector3 &out) const", &Layer::Component::GetUnitBounds);
				Class.SetMethod("float get_visibility(const viewer_t &in, float) const", &Layer::Component::GetVisibility);
				Class.SetMethod("base_component@+ copy(scene_entity@+) const", &Layer::Component::Copy);
				Class.SetMethod("scene_entity@+ get_entity() const", &Layer::Component::GetEntity);
				Class.SetMethod("void set_active(bool)", &Layer::Component::SetActive);
				Class.SetMethod("bool is_drawable() const", &Layer::Component::IsDrawable);
				Class.SetMethod("bool is_cullable() const", &Layer::Component::IsCullable);
				Class.SetMethod("bool is_active() const", &Layer::Component::IsActive);
				PopulateComponent<T>(Class);
			};
			template <typename T, typename... Args>
			void PopulateComponentInterface(RefClass& Class, const char* Constructor)
			{
				Class.SetConstructor<T, Args...>(Constructor);
				Class.SetDynamicCast<T, Layer::Component>("base_component@+", true);
				PopulateComponentBase<T>(Class, false);
			};

			void EntityRemoveComponent(Layer::Entity* Base, Layer::Component* Source)
			{
				if (Source != nullptr)
					Base->RemoveComponent(Source->GetId());
			}
			void EntityRemoveComponentById(Layer::Entity* Base, uint64_t Id)
			{
				Base->RemoveComponent(Id);
			}
			Layer::Component* EntityAddComponent(Layer::Entity* Base, Layer::Component* Source)
			{
				if (!Source)
					return nullptr;

				return Base->AddComponent(Source);
			}
			Layer::Component* EntityGetComponentById(Layer::Entity* Base, uint64_t Id)
			{
				return Base->GetComponent(Id);
			}
			Array* EntityGetComponents(Layer::Entity* Base)
			{
				Core::Vector<Layer::Component*> Components;
				Components.reserve(Base->GetComponentsCount());

				for (auto& Item : *Base)
					Components.push_back(Item.second);

				TypeInfo Type = VirtualMachine::Get()->GetTypeInfoByDecl(TYPENAME_ARRAY "<" TYPENAME_COMPONENT "@>@");
				return Array::Compose<Layer::Component*>(Type.GetTypeInfo(), Components);
			}

			template <typename T>
			void PopulateDrawableBase(RefClass& Class)
			{
				Class.SetProperty<Layer::Drawable>("float overlapping", &Layer::Drawable::Overlapping);
				Class.SetProperty<Layer::Drawable>("bool static", &Layer::Drawable::Static);
				Class.SetMethod("void clear_materials()", &Layer::Drawable::ClearMaterials);
				Class.SetMethod("bool set_category(geo_category)", &Layer::Drawable::SetCategory);
				Class.SetMethod<Layer::Drawable, bool, void*, Layer::Material*>("bool set_material(uptr@, material@+)", &Layer::Drawable::SetMaterial);
				Class.SetMethod<Layer::Drawable, bool, Layer::Material*>("bool set_material(material@+)", &Layer::Drawable::SetMaterial);
				Class.SetMethod("geo_category get_category() const", &Layer::Drawable::GetCategory);
				Class.SetMethod<Layer::Drawable, int64_t, void*>("int64 get_slot(uptr@)", &Layer::Drawable::GetSlot);
				Class.SetMethod<Layer::Drawable, int64_t>("int64 get_slot()", &Layer::Drawable::GetSlot);
				Class.SetMethod<Layer::Drawable, Layer::Material*, void*>("material@+ get_material(uptr@)", &Layer::Drawable::GetMaterial);
				Class.SetMethod<Layer::Drawable, Layer::Material*>("material@+ get_material()", &Layer::Drawable::GetMaterial);
				PopulateComponentBase<T>(Class, false);
			}
			template <typename T, typename... Args>
			void PopulateDrawableInterface(RefClass& Class, const char* Constructor)
			{
				Class.SetConstructor<T, Args...>(Constructor);
				Class.SetDynamicCast<T, Layer::Component>("base_component@+", true);
				PopulateDrawableBase<T>(Class);
			}

			template <typename T>
			void PopulateRendererBase(RefClass& Class, bool BaseCast = true)
			{
				if (BaseCast)
					Class.SetOperatorEx(Operators::Cast, 0, "void", "?&out", &HandleToHandleCast);

				Class.SetProperty<Layer::Renderer>("bool active", &Layer::Renderer::Active);
				Class.SetMethod("void serialize(schema@+)", &Layer::Renderer::Serialize);
				Class.SetMethod("void deserialize(schema@+)", &Layer::Renderer::Deserialize);
				Class.SetMethod("void clear_culling()", &Layer::Renderer::ClearCulling);
				Class.SetMethod("void resize_buffers()", &Layer::Renderer::ResizeBuffers);
				Class.SetMethod("void activate()", &Layer::Renderer::Activate);
				Class.SetMethod("void deactivate()", &Layer::Renderer::Deactivate);
				Class.SetMethod("void begin_pass()", &Layer::Renderer::BeginPass);
				Class.SetMethod("void end_pass()", &Layer::Renderer::EndPass);
				Class.SetMethod("bool has_category(geo_category)", &Layer::Renderer::HasCategory);
				Class.SetMethod("usize render_pass(clock_timer@+)", &Layer::Renderer::RenderPass);
				Class.SetMethod("void set_renderer(render_system@+)", &Layer::Renderer::SetRenderer);
				Class.SetMethod("render_system@+ get_renderer() const", &Layer::Renderer::GetRenderer);
				PopulateComponent<T>(Class);
			}
			template <typename T, typename... Args>
			void PopulateRendererInterface(RefClass& Class, const char* Constructor)
			{
				Class.SetConstructor<T, Args...>(Constructor);
				Class.SetDynamicCast<T, Layer::Renderer>("base_renderer@+", true);
				PopulateRendererBase<T>(Class, false);
			}

			void RenderSystemRestoreViewBuffer(Layer::RenderSystem* Base)
			{
				Base->RestoreViewBuffer(nullptr);
			}
			void RenderSystemMoveRenderer(Layer::RenderSystem* Base, Layer::Renderer* Source, size_t Offset)
			{
				if (Source != nullptr)
					Base->MoveRenderer(Source->GetId(), Offset);
			}
			void RenderSystemRemoveRenderer(Layer::RenderSystem* Base, Layer::Renderer* Source)
			{
				if (Source != nullptr)
					Base->RemoveRenderer(Source->GetId());
			}
			void RenderSystemFreeBuffers1(Layer::RenderSystem* Base, const std::string_view& Name, Graphics::ElementBuffer* Buffer1, Graphics::ElementBuffer* Buffer2)
			{
				Graphics::ElementBuffer* Buffers[2];
				Buffers[0] = Buffer1;
				Buffers[1] = Buffer2;
				Base->FreeBuffers(Name, Buffers);
			}
			void RenderSystemFreeBuffers2(Layer::RenderSystem* Base, const std::string_view& Name, Graphics::ElementBuffer* Buffer1, Graphics::ElementBuffer* Buffer2)
			{
				Graphics::ElementBuffer* Buffers[2];
				Buffers[0] = Buffer1;
				Buffers[1] = Buffer2;
				Base->FreeBuffers(Buffers);
			}
			Graphics::Shader* RenderSystemCompileShader1(Layer::RenderSystem* Base, Graphics::Shader::Desc& Desc, size_t BufferSize)
			{
				return ExpectsWrapper::Unwrap(Base->CompileShader(Desc, BufferSize), (Graphics::Shader*)nullptr);
			}
			Graphics::Shader* RenderSystemCompileShader2(Layer::RenderSystem* Base, const std::string_view& SectionName, size_t BufferSize)
			{
				return ExpectsWrapper::Unwrap(Base->CompileShader(SectionName, BufferSize), (Graphics::Shader*)nullptr);
			}
			Array* RenderSystemCompileBuffers(Layer::RenderSystem* Base, const std::string_view& Name, size_t ElementSize, size_t ElementsCount)
			{
				Core::Vector<Graphics::ElementBuffer*> Buffers;
				Buffers.push_back(nullptr);
				Buffers.push_back(nullptr);

				if (!ExpectsWrapper::UnwrapVoid(Base->CompileBuffers(Buffers.data(), Name, ElementSize, ElementsCount)))
					return nullptr;

				TypeInfo Type = VirtualMachine::Get()->GetTypeInfoByDecl(TYPENAME_ARRAY "<" TYPENAME_ELEMENTBUFFER "@>@");
				return Array::Compose(Type.GetTypeInfo(), Buffers);
			}
			bool RenderSystemAddRenderer(Layer::RenderSystem* Base, Layer::Renderer* Source)
			{
				if (!Source)
					return false;

				return Base->AddRenderer(Source) != nullptr;
			}
			Layer::Renderer* RenderSystemGetRenderer(Layer::RenderSystem* Base, uint64_t Id)
			{
				return Base->GetRenderer(Id);
			}
			Layer::Renderer* RenderSystemGetRendererByIndex(Layer::RenderSystem* Base, size_t Index)
			{
				auto& Data = Base->GetRenderers();
				if (Index >= Data.size())
					return nullptr;

				return Data[Index];
			}
			size_t RenderSystemGetRenderersCount(Layer::RenderSystem* Base)
			{
				return Base->GetRenderers().size();
			}
			void RenderSystemQueryGroup(Layer::RenderSystem* Base, uint64_t Id, asIScriptFunction* Callback)
			{
				ImmediateContext* Context = ImmediateContext::Get();
				FunctionDelegate Delegate(Callback);
				if (!Context || !Delegate.IsValid())
					return;

				Base->QueryGroup(Id, [Context, Delegate](Layer::Component* Item)
				{
					Context->ExecuteSubcall(Delegate.Callable(), [Item](ImmediateContext* Context)
					{
						Context->SetArgObject(0, (void*)Item);
					});
				});
			}

			Array* PrimitiveCacheCompile(Layer::PrimitiveCache* Base, const std::string_view& Name, size_t ElementSize, size_t ElementsCount)
			{
				Core::Vector<Graphics::ElementBuffer*> Buffers;
				Buffers.push_back(nullptr);
				Buffers.push_back(nullptr);

				if (!ExpectsWrapper::UnwrapVoid(Base->Compile(Buffers.data(), Name, ElementSize, ElementsCount)))
					return nullptr;

				TypeInfo Type = VirtualMachine::Get()->GetTypeInfoByDecl(TYPENAME_ARRAY "<" TYPENAME_ELEMENTBUFFER "@>@");
				return Array::Compose(Type.GetTypeInfo(), Buffers);
			}
			Array* PrimitiveCacheGet(Layer::PrimitiveCache* Base, const std::string_view& Name)
			{
				Core::Vector<Graphics::ElementBuffer*> Buffers;
				Buffers.push_back(nullptr);
				Buffers.push_back(nullptr);

				if (!Base->Get(Buffers.data(), Name))
					return nullptr;

				TypeInfo Type = VirtualMachine::Get()->GetTypeInfoByDecl(TYPENAME_ARRAY "<" TYPENAME_ELEMENTBUFFER "@>@");
				return Array::Compose(Type.GetTypeInfo(), Buffers);
			}
			bool PrimitiveCacheFree(Layer::PrimitiveCache* Base, const std::string_view& Name, Graphics::ElementBuffer* Buffer1, Graphics::ElementBuffer* Buffer2)
			{
				Graphics::ElementBuffer* Buffers[2];
				Buffers[0] = Buffer1;
				Buffers[1] = Buffer2;
				return Base->Free(Name, Buffers);
			}
			Core::String PrimitiveCacheFind(Layer::PrimitiveCache* Base, Graphics::ElementBuffer* Buffer1, Graphics::ElementBuffer* Buffer2)
			{
				Graphics::ElementBuffer* Buffers[2];
				Buffers[0] = Buffer1;
				Buffers[1] = Buffer2;
				return Base->Find(Buffers);
			}
			Array* PrimitiveCacheGetSphereBuffers(Layer::PrimitiveCache* Base)
			{
				Core::Vector<Graphics::ElementBuffer*> Buffers;
				Buffers.push_back(nullptr);
				Buffers.push_back(nullptr);
				Base->GetSphereBuffers(Buffers.data());

				TypeInfo Type = VirtualMachine::Get()->GetTypeInfoByDecl(TYPENAME_ARRAY "<" TYPENAME_ELEMENTBUFFER "@>@");
				return Array::Compose(Type.GetTypeInfo(), Buffers);
			}
			Array* PrimitiveCacheGetCubeBuffers(Layer::PrimitiveCache* Base)
			{
				Core::Vector<Graphics::ElementBuffer*> Buffers;
				Buffers.push_back(nullptr);
				Buffers.push_back(nullptr);
				Base->GetCubeBuffers(Buffers.data());

				TypeInfo Type = VirtualMachine::Get()->GetTypeInfoByDecl(TYPENAME_ARRAY "<" TYPENAME_ELEMENTBUFFER "@>@");
				return Array::Compose(Type.GetTypeInfo(), Buffers);
			}
			Array* PrimitiveCacheGetBoxBuffers(Layer::PrimitiveCache* Base)
			{
				Core::Vector<Graphics::ElementBuffer*> Buffers;
				Buffers.push_back(nullptr);
				Buffers.push_back(nullptr);
				Base->GetBoxBuffers(Buffers.data());

				TypeInfo Type = VirtualMachine::Get()->GetTypeInfoByDecl(TYPENAME_ARRAY "<" TYPENAME_ELEMENTBUFFER "@>@");
				return Array::Compose(Type.GetTypeInfo(), Buffers);
			}
			Array* PrimitiveCacheGetSkinBoxBuffers(Layer::PrimitiveCache* Base)
			{
				Core::Vector<Graphics::ElementBuffer*> Buffers;
				Buffers.push_back(nullptr);
				Buffers.push_back(nullptr);
				Base->GetSkinBoxBuffers(Buffers.data());

				TypeInfo Type = VirtualMachine::Get()->GetTypeInfoByDecl(TYPENAME_ARRAY "<" TYPENAME_ELEMENTBUFFER "@>@");
				return Array::Compose(Type.GetTypeInfo(), Buffers);
			}

			Layer::SceneGraph::Desc::Dependencies& SceneGraphSharedDescCopy(Layer::SceneGraph::Desc::Dependencies& Base, Layer::SceneGraph::Desc::Dependencies& Other)
			{
				if (&Base == &Other)
					return Base;

				Core::Memory::Release(Base.Device);
				Core::Memory::Release(Base.Activity);
				Core::Memory::Release(Base.VM);
				Core::Memory::Release(Base.Content);
				Core::Memory::Release(Base.Primitives);
				Core::Memory::Release(Base.Shaders);
				Base = Other;
				if (Base.Device != nullptr)
					Base.Device->AddRef();
				if (Base.Activity != nullptr)
					Base.Activity->AddRef();
				if (Base.VM != nullptr)
					Base.VM->AddRef();
				if (Base.Content != nullptr)
					Base.Content->AddRef();
				if (Base.Primitives != nullptr)
					Base.Primitives->AddRef();
				if (Base.Shaders != nullptr)
					Base.Shaders->AddRef();

				return Base;
			}
			void SceneGraphSharedDescDestructor(Layer::SceneGraph::Desc::Dependencies& Base)
			{
				Core::Memory::Release(Base.Device);
				Core::Memory::Release(Base.Activity);
				Core::Memory::Release(Base.VM);
				Core::Memory::Release(Base.Content);
				Core::Memory::Release(Base.Primitives);
				Core::Memory::Release(Base.Shaders);
			}

			void SceneGraphRayTest(Layer::SceneGraph* Base, uint64_t Id, const Trigonometry::Ray& Ray, asIScriptFunction* Callback)
			{
				ImmediateContext* Context = ImmediateContext::Get();
				FunctionDelegate Delegate(Callback);
				if (!Context || !Delegate.IsValid())
					return;

				Base->RayTest(Id, Ray, [Context, Delegate](Layer::Component* Source, const Trigonometry::Vector3& Where)
				{
					bool Result = false;
					Context->ExecuteSubcall(Delegate.Callable(), [&Source, &Where](ImmediateContext* Context)
					{
						Context->SetArgObject(0, (void*)Source);
						Context->SetArgObject(1, (void*)&Where);
					}, [&Result](ImmediateContext* Context)
					{
						Result = (bool)Context->GetReturnByte();
					});
					return Result;
				});
			}
			void SceneGraphMutate1(Layer::SceneGraph* Base, Layer::Entity* Source, Layer::Entity* Child, const std::string_view& Name)
			{
				Base->Mutate(Source, Child, Name);
			}
			void SceneGraphMutate2(Layer::SceneGraph* Base, Layer::Entity* Source, const std::string_view& Name)
			{
				Base->Mutate(Source, Name);
			}
			void SceneGraphMutate3(Layer::SceneGraph* Base, Layer::Component* Source, const std::string_view& Name)
			{
				Base->Mutate(Source, Name);
			}
			void SceneGraphMutate4(Layer::SceneGraph* Base, Layer::Material* Source, const std::string_view& Name)
			{
				Base->Mutate(Source, Name);
			}
			void SceneGraphTransaction(Layer::SceneGraph* Base, asIScriptFunction* Callback)
			{
				FunctionDelegate Delegate(Callback);
				if (!Delegate.IsValid())
					return;

				Base->Transaction([Delegate]() mutable
				{
					Delegate(nullptr).Wait();
				});
			}
			void SceneGraphPushEvent1(Layer::SceneGraph* Base, const std::string_view& Name, Core::Schema* Args, bool Propagate)
			{
				Base->PushEvent(Name, ToVariantKeys(Args), Propagate);
			}
			void SceneGraphPushEvent2(Layer::SceneGraph* Base, const std::string_view& Name, Core::Schema* Args, Layer::Component* Source)
			{
				Base->PushEvent(Name, ToVariantKeys(Args), Source);
			}
			void SceneGraphPushEvent3(Layer::SceneGraph* Base, const std::string_view& Name, Core::Schema* Args, Layer::Entity* Source)
			{
				Base->PushEvent(Name, ToVariantKeys(Args), Source);
			}
			void* SceneGraphSetListener(Layer::SceneGraph* Base, const std::string_view& Name, asIScriptFunction* Callback)
			{
				FunctionDelegate Delegate(Callback);
				if (!Delegate.IsValid())
					return nullptr;

				return Base->SetListener(Name, [Delegate](const std::string_view& Name, Core::VariantArgs& BaseArgs) mutable
				{
					Core::Schema* Args = Core::Var::Set::Object();
					Args->Reserve(BaseArgs.size());
					for (auto& Item : BaseArgs)
						Args->Set(Item.first, Item.second);

					Core::String Copy = Core::String(Name);
					Delegate([Copy, Args](ImmediateContext* Context)
					{
						Context->SetArgObject(0, (void*)&Copy);
						Context->SetArgObject(1, (void*)Args);
					}).When([Args](ExpectsVM<Execution>&&) { Args->Release(); });
				});
			}
			void SceneGraphLoadResource(Layer::SceneGraph* Base, uint64_t Id, Layer::Component* Source, const std::string_view& Path, Core::Schema* Args, asIScriptFunction* Callback)
			{
				FunctionDelegate Delegate(Callback);
				if (!Delegate.IsValid())
					return;

				Base->LoadResource(Id, Source, Path, ToVariantKeys(Args), [Delegate](Layer::ExpectsContent<void*>&& Resource) mutable
				{
					void* Address = Resource.Or(nullptr);
					Delegate([Address](ImmediateContext* Context)
					{
						Context->SetArgAddress(0, Address);
					});
				});
			}
			Array* SceneGraphGetComponents(Layer::SceneGraph* Base, uint64_t Id)
			{
				auto& Data = Base->GetComponents(Id);
				Core::Vector<Layer::Component*> Output;
				Output.reserve(Data.Size());

				for (auto* Next : Data)
					Output.push_back(Next);

				TypeInfo Type = VirtualMachine::Get()->GetTypeInfoByDecl(TYPENAME_ARRAY "<" TYPENAME_COMPONENT "@>@");
				return Array::Compose(Type.GetTypeInfo(), Output);
			}
			Array* SceneGraphGetActors(Layer::SceneGraph* Base, Layer::ActorType Source)
			{
				auto& Data = Base->GetActors(Source);
				Core::Vector<Layer::Component*> Output;
				Output.reserve(Data.Size());

				for (auto* Next : Data)
					Output.push_back(Next);

				TypeInfo Type = VirtualMachine::Get()->GetTypeInfoByDecl(TYPENAME_ARRAY "<" TYPENAME_COMPONENT "@>@");
				return Array::Compose(Type.GetTypeInfo(), Output);
			}
			Array* SceneGraphCloneEntityAsArray(Layer::SceneGraph* Base, Layer::Entity* Source)
			{
				TypeInfo Type = VirtualMachine::Get()->GetTypeInfoByDecl(TYPENAME_ARRAY "<" TYPENAME_ENTITY "@>@");
				return Array::Compose(Type.GetTypeInfo(), Base->CloneEntityAsArray(Source));
			}
			Array* SceneGraphQueryByParent(Layer::SceneGraph* Base, Layer::Entity* Source)
			{
				TypeInfo Type = VirtualMachine::Get()->GetTypeInfoByDecl(TYPENAME_ARRAY "<" TYPENAME_ENTITY "@>@");
				return Array::Compose(Type.GetTypeInfo(), Base->QueryByParent(Source));
			}
			Array* SceneGraphQueryByName(Layer::SceneGraph* Base, const std::string_view& Source)
			{
				TypeInfo Type = VirtualMachine::Get()->GetTypeInfoByDecl(TYPENAME_ARRAY "<" TYPENAME_ENTITY "@>@");
				return Array::Compose(Type.GetTypeInfo(), Base->QueryByName(Source));
			}
			Array* SceneGraphQueryByPosition(Layer::SceneGraph* Base, uint64_t Id, const Trigonometry::Vector3& Position, float Radius)
			{
				TypeInfo Type = VirtualMachine::Get()->GetTypeInfoByDecl(TYPENAME_ARRAY "<" TYPENAME_COMPONENT "@>@");
				return Array::Compose(Type.GetTypeInfo(), Base->QueryByPosition(Id, Position, Radius));
			}
			Array* SceneGraphQueryByArea(Layer::SceneGraph* Base, uint64_t Id, const Trigonometry::Vector3& Min, const Trigonometry::Vector3& Max)
			{
				TypeInfo Type = VirtualMachine::Get()->GetTypeInfoByDecl(TYPENAME_ARRAY "<" TYPENAME_COMPONENT "@>@");
				return Array::Compose(Type.GetTypeInfo(), Base->QueryByArea(Id, Min, Max));
			}
			Array* SceneGraphQueryByRay(Layer::SceneGraph* Base, uint64_t Id, const Trigonometry::Ray& Ray)
			{
				TypeInfo Type = VirtualMachine::Get()->GetTypeInfoByDecl(TYPENAME_ARRAY "<" TYPENAME_COMPONENT "@>@");
				return Array::Compose(Type.GetTypeInfo(), Base->QueryByRay(Id, Ray));
			}
			Array* SceneGraphQueryByMatch(Layer::SceneGraph* Base, uint64_t Id, asIScriptFunction* Callback)
			{
				ImmediateContext* Context = ImmediateContext::Get();
				FunctionDelegate Delegate(Callback);
				if (!Context || !Delegate.IsValid())
					return nullptr;

				TypeInfo Type = VirtualMachine::Get()->GetTypeInfoByDecl(TYPENAME_ARRAY "<" TYPENAME_COMPONENT "@>@");
				return Array::Compose(Type.GetTypeInfo(), Base->QueryByMatch(Id, [Context, Delegate](const Trigonometry::Bounding& Source)
				{
					bool Result = false;
					Context->ExecuteSubcall(Delegate.Callable(), [&Source](ImmediateContext* Context)
					{
						Context->SetArgObject(0, (void*)&Source);
					}, [&Result](ImmediateContext* Context)
					{
						Result = (bool)Context->GetReturnByte();
					});
					return Result;
				}));
			}

			HeavyApplication::CacheInfo& ApplicationCacheInfoCopy(HeavyApplication::CacheInfo& Base, HeavyApplication::CacheInfo& Other)
			{
				if (&Base == &Other)
					return Base;

				Core::Memory::Release(Base.Primitives);
				Core::Memory::Release(Base.Shaders);
				Base = Other;
				if (Base.Primitives != nullptr)
					Base.Primitives->AddRef();
				if (Base.Shaders != nullptr)
					Base.Shaders->AddRef();

				return Base;
			}
			void ApplicationCacheInfoDestructor(HeavyApplication::CacheInfo& Base)
			{
				Core::Memory::Release(Base.Primitives);
				Core::Memory::Release(Base.Shaders);
			}

			bool IElementDispatchEvent(Layer::GUI::IElement& Base, const std::string_view& Name, Core::Schema* Args)
			{
				Core::VariantArgs Data;
				if (Args != nullptr)
				{
					Data.reserve(Args->Size());
					for (auto Item : Args->GetChilds())
						Data[Item->Key] = Item->Value;
				}

				return Base.DispatchEvent(Name, Data);
			}
			Array* IElementQuerySelectorAll(Layer::GUI::IElement& Base, const std::string_view& Value)
			{
				TypeInfo Type = VirtualMachine::Get()->GetTypeInfoByDecl(TYPENAME_ARRAY "<" TYPENAME_ELEMENTNODE ">@");
				return Array::Compose(Type.GetTypeInfo(), Base.QuerySelectorAll(Value));
			}

			bool IElementDocumentDispatchEvent(Layer::GUI::IElementDocument& Base, const std::string_view& Name, Core::Schema* Args)
			{
				Core::VariantArgs Data;
				if (Args != nullptr)
				{
					Data.reserve(Args->Size());
					for (auto Item : Args->GetChilds())
						Data[Item->Key] = Item->Value;
				}

				return Base.DispatchEvent(Name, Data);
			}
			Array* IElementDocumentQuerySelectorAll(Layer::GUI::IElementDocument& Base, const std::string_view& Value)
			{
				TypeInfo Type = VirtualMachine::Get()->GetTypeInfoByDecl(TYPENAME_ARRAY "<" TYPENAME_ELEMENTNODE ">@");
				return Array::Compose(Type.GetTypeInfo(), Base.QuerySelectorAll(Value));
			}

			bool DataModelSetRecursive(Layer::GUI::DataNode* Node, Core::Schema* Data, size_t Depth)
			{
				size_t Index = 0;
				for (auto& Item : Data->GetChilds())
				{
					auto& Child = Node->Add(Item->Value);
					Child.SetTop((void*)(uintptr_t)Index++, Depth);
					if (Item->Value.IsObject())
						DataModelSetRecursive(&Child, Item, Depth + 1);
				}

				Node->SortTree();
				return true;
			}
			bool DataModelGetRecursive(Layer::GUI::DataNode* Node, Core::Schema* Data)
			{
				size_t Size = Node->Size();
				for (size_t i = 0; i < Size; i++)
				{
					auto& Child = Node->At(i);
					DataModelGetRecursive(&Child, Data->Push(Child.Get()));
				}

				return true;
			}
			bool DataModelSet(Layer::GUI::DataModel* Base, const std::string_view& Name, Core::Schema* Data)
			{
				if (!Data->Value.IsObject())
					return Base->SetProperty(Name, Data->Value) != nullptr;

				Layer::GUI::DataNode* Node = Base->SetArray(Name);
				if (!Node)
					return false;

				return DataModelSetRecursive(Node, Data, 0);
			}
			bool DataModelSetVar(Layer::GUI::DataModel* Base, const std::string_view& Name, const Core::Variant& Data)
			{
				return Base->SetProperty(Name, Data) != nullptr;
			}
			bool DataModelSetString(Layer::GUI::DataModel* Base, const std::string_view& Name, const std::string_view& Value)
			{
				return Base->SetString(Name, Value) != nullptr;
			}
			bool DataModelSetInteger(Layer::GUI::DataModel* Base, const std::string_view& Name, int64_t Value)
			{
				return Base->SetInteger(Name, Value) != nullptr;
			}
			bool DataModelSetFloat(Layer::GUI::DataModel* Base, const std::string_view& Name, float Value)
			{
				return Base->SetFloat(Name, Value) != nullptr;
			}
			bool DataModelSetDouble(Layer::GUI::DataModel* Base, const std::string_view& Name, double Value)
			{
				return Base->SetDouble(Name, Value) != nullptr;
			}
			bool DataModelSetBoolean(Layer::GUI::DataModel* Base, const std::string_view& Name, bool Value)
			{
				return Base->SetBoolean(Name, Value) != nullptr;
			}
			bool DataModelSetPointer(Layer::GUI::DataModel* Base, const std::string_view& Name, void* Value)
			{
				return Base->SetPointer(Name, Value) != nullptr;
			}
			bool DataModelSetCallback(Layer::GUI::DataModel* Base, const std::string_view& Name, asIScriptFunction* Callback)
			{
				FunctionDelegate Delegate(Callback);
				if (!Delegate.IsValid())
					return false;

				TypeInfo Type = VirtualMachine::Get()->GetTypeInfoByDecl(TYPENAME_ARRAY "<" TYPENAME_VARIANT ">@");
				return Base->SetCallback(Name, [Type, Delegate](Layer::GUI::IEvent& Wrapper, const Core::VariantList& Args) mutable
				{
					Layer::GUI::IEvent Event = Wrapper.Copy();
					Array* Data = Array::Compose(Type.GetTypeInfo(), Args);
					Delegate([Event, &Data](ImmediateContext* Context) mutable
					{
						Context->SetArgObject(0, &Event);
						Context->SetArgObject(1, &Data);
					}).When([Event](ExpectsVM<Execution>&&) mutable
					{
						Event.Release();
					});
				});
			}
			Core::Schema* DataModelGet(Layer::GUI::DataModel* Base, const std::string_view& Name)
			{
				Layer::GUI::DataNode* Node = Base->GetProperty(Name);
				if (!Node)
					return nullptr;

				Core::Schema* Result = new Core::Schema(Node->Get());
				if (Result->Value.IsObject())
					DataModelGetRecursive(Node, Result);

				return Result;
			}

			Layer::GUI::IElement ContextGetFocusElement(Layer::GUI::Context* Base, const Trigonometry::Vector2& Value)
			{
				return Base->GetElementAtPoint(Value);
			}
			void ContextEmitInput(Layer::GUI::Context* Base, const std::string_view& Data)
			{
				Base->EmitInput(Data.data(), (int)Data.size());
			}

			ModelListener::ModelListener(asIScriptFunction* NewCallback) noexcept : Delegate(), Base(new Layer::GUI::Listener(Bind(NewCallback)))
			{
			}
			ModelListener::ModelListener(const std::string_view& FunctionName) noexcept : Delegate(), Base(new Layer::GUI::Listener(FunctionName))
			{
			}
			ModelListener::~ModelListener() noexcept
			{
				Core::Memory::Release(Base);
			}
			FunctionDelegate& ModelListener::GetDelegate()
			{
				return Delegate;
			}
			Layer::GUI::EventCallback ModelListener::Bind(asIScriptFunction* Callback)
			{
				Delegate = FunctionDelegate(Callback);
				if (!Delegate.IsValid())
					return nullptr;

				ModelListener* Listener = this;
				return [Listener](Layer::GUI::IEvent& Wrapper)
				{
					Layer::GUI::IEvent Event = Wrapper.Copy();
					Listener->Delegate([Event](ImmediateContext* Context) mutable
					{
						Context->SetArgObject(0, &Event);
					}).When([Event](ExpectsVM<Execution>&&) mutable
					{
						Event.Release();
					});
				};
			}

			void ComponentsSoftBodyLoad(Layer::Components::SoftBody* Base, const std::string_view& Path, float Ant, asIScriptFunction* Callback)
			{
				FunctionDelegate Delegate(Callback);
				if (!Delegate.IsValid())
					return Base->Load(Path, Ant);

				Base->Load(Path, Ant, [Delegate]() mutable
				{
					Delegate(nullptr);
				});
			}

			void ComponentsRigidBodyLoad(Layer::Components::RigidBody* Base, const std::string_view& Path, float Mass, float Ant, asIScriptFunction* Callback)
			{
				FunctionDelegate Delegate(Callback);
				if (!Delegate.IsValid())
					return Base->Load(Path, Mass, Ant);

				Base->Load(Path, Mass, Ant, [Delegate]() mutable
				{
					Delegate(nullptr);
				});
			}

			void ComponentsKeyAnimatorLoadAnimation(Layer::Components::KeyAnimator* Base, const std::string_view& Path, asIScriptFunction* Callback)
			{
				FunctionDelegate Delegate(Callback);
				if (!Delegate.IsValid())
					return Base->LoadAnimation(Path);

				Base->LoadAnimation(Path, [Delegate](bool) mutable
				{
					Delegate(nullptr);
				});
			}
#endif
			bool HeavyRegistry::BindAddons(VirtualMachine* VM) noexcept
			{
				if (!Registry::BindAddons(VM))
					return false;

				VM->AddSystemAddon("trigonometry", { "string" }, &ImportTrigonometry);
				VM->AddSystemAddon("physics", { "trigonometry" }, &ImportPhysics);
				VM->AddSystemAddon("activity", { "string", "trigonometry" }, &ImportActivity);
				VM->AddSystemAddon("graphics", { "activity", "trigonometry" }, &ImportGraphics);
				VM->AddSystemAddon("audio", { "string", "trigonometry", "schema" }, &ImportAudio);
				VM->AddSystemAddon("audio-effects", { "audio" }, &ImportAudioEffects);
				VM->AddSystemAddon("audio-filters", { "audio" }, &ImportAudioFilters);
				VM->AddSystemAddon("engine", { "layer", "graphics", "audio", "physics", "ui" }, &ImportEngine);
				VM->AddSystemAddon("engine-components", { "engine" }, &ImportEngineComponents);
				VM->AddSystemAddon("engine-renderers", { "engine" }, &ImportEngineRenderers);
				VM->AddSystemAddon("ui-control", { "trigonometry", "schema", "array" }, &ImportUiControl);
				VM->AddSystemAddon("ui-model", { "ui-control", }, &ImportUiModel);
				VM->AddSystemAddon("ui", { "ui-model" }, &ImportUi);
				return true;
			}
			bool HeavyRegistry::ImportTrigonometry(VirtualMachine* VM) noexcept
			{
				VI_ASSERT(VM != nullptr && VM->GetEngine() != nullptr, "manager should be set");
#ifdef VI_BINDINGS
				auto VVertex = VM->SetPod<Trigonometry::Vertex>("vertex");
				VVertex->SetProperty<Trigonometry::Vertex>("float position_x", &Trigonometry::Vertex::PositionX);
				VVertex->SetProperty<Trigonometry::Vertex>("float position_y", &Trigonometry::Vertex::PositionY);
				VVertex->SetProperty<Trigonometry::Vertex>("float position_z", &Trigonometry::Vertex::PositionZ);
				VVertex->SetProperty<Trigonometry::Vertex>("float texcoord_x", &Trigonometry::Vertex::TexCoordX);
				VVertex->SetProperty<Trigonometry::Vertex>("float texcoord_y", &Trigonometry::Vertex::TexCoordY);
				VVertex->SetProperty<Trigonometry::Vertex>("float normal_x", &Trigonometry::Vertex::NormalX);
				VVertex->SetProperty<Trigonometry::Vertex>("float normal_y", &Trigonometry::Vertex::NormalY);
				VVertex->SetProperty<Trigonometry::Vertex>("float normal_z", &Trigonometry::Vertex::NormalZ);
				VVertex->SetProperty<Trigonometry::Vertex>("float tangent_x", &Trigonometry::Vertex::TangentX);
				VVertex->SetProperty<Trigonometry::Vertex>("float tangent_y", &Trigonometry::Vertex::TangentY);
				VVertex->SetProperty<Trigonometry::Vertex>("float tangent_z", &Trigonometry::Vertex::TangentZ);
				VVertex->SetProperty<Trigonometry::Vertex>("float bitangent_x", &Trigonometry::Vertex::BitangentX);
				VVertex->SetProperty<Trigonometry::Vertex>("float bitangent_y", &Trigonometry::Vertex::BitangentY);
				VVertex->SetProperty<Trigonometry::Vertex>("float bitangent_z", &Trigonometry::Vertex::BitangentZ);
				VVertex->SetConstructor<Trigonometry::Vertex>("void f()");

				auto VSkinVertex = VM->SetPod<Trigonometry::SkinVertex>("skin_vertex");
				VSkinVertex->SetProperty<Trigonometry::SkinVertex>("float position_x", &Trigonometry::SkinVertex::PositionX);
				VSkinVertex->SetProperty<Trigonometry::SkinVertex>("float position_y", &Trigonometry::SkinVertex::PositionY);
				VSkinVertex->SetProperty<Trigonometry::SkinVertex>("float position_z", &Trigonometry::SkinVertex::PositionZ);
				VSkinVertex->SetProperty<Trigonometry::SkinVertex>("float texcoord_x", &Trigonometry::SkinVertex::TexCoordX);
				VSkinVertex->SetProperty<Trigonometry::SkinVertex>("float texcoord_y", &Trigonometry::SkinVertex::TexCoordY);
				VSkinVertex->SetProperty<Trigonometry::SkinVertex>("float normal_x", &Trigonometry::SkinVertex::NormalX);
				VSkinVertex->SetProperty<Trigonometry::SkinVertex>("float normal_y", &Trigonometry::SkinVertex::NormalY);
				VSkinVertex->SetProperty<Trigonometry::SkinVertex>("float normal_z", &Trigonometry::SkinVertex::NormalZ);
				VSkinVertex->SetProperty<Trigonometry::SkinVertex>("float tangent_x", &Trigonometry::SkinVertex::TangentX);
				VSkinVertex->SetProperty<Trigonometry::SkinVertex>("float tangent_y", &Trigonometry::SkinVertex::TangentY);
				VSkinVertex->SetProperty<Trigonometry::SkinVertex>("float tangent_z", &Trigonometry::SkinVertex::TangentZ);
				VSkinVertex->SetProperty<Trigonometry::SkinVertex>("float bitangent_x", &Trigonometry::SkinVertex::BitangentX);
				VSkinVertex->SetProperty<Trigonometry::SkinVertex>("float bitangent_y", &Trigonometry::SkinVertex::BitangentY);
				VSkinVertex->SetProperty<Trigonometry::SkinVertex>("float bitangent_z", &Trigonometry::SkinVertex::BitangentZ);
				VSkinVertex->SetProperty<Trigonometry::SkinVertex>("float joint_index0", &Trigonometry::SkinVertex::JointIndex0);
				VSkinVertex->SetProperty<Trigonometry::SkinVertex>("float joint_index1", &Trigonometry::SkinVertex::JointIndex1);
				VSkinVertex->SetProperty<Trigonometry::SkinVertex>("float joint_index2", &Trigonometry::SkinVertex::JointIndex2);
				VSkinVertex->SetProperty<Trigonometry::SkinVertex>("float joint_index3", &Trigonometry::SkinVertex::JointIndex3);
				VSkinVertex->SetProperty<Trigonometry::SkinVertex>("float joint_bias0", &Trigonometry::SkinVertex::JointBias0);
				VSkinVertex->SetProperty<Trigonometry::SkinVertex>("float joint_bias1", &Trigonometry::SkinVertex::JointBias1);
				VSkinVertex->SetProperty<Trigonometry::SkinVertex>("float joint_bias2", &Trigonometry::SkinVertex::JointBias2);
				VSkinVertex->SetProperty<Trigonometry::SkinVertex>("float joint_bias3", &Trigonometry::SkinVertex::JointBias3);
				VSkinVertex->SetConstructor<Trigonometry::SkinVertex>("void f()");

				auto VShapeVertex = VM->SetPod<Trigonometry::ShapeVertex>("shape_vertex");
				VShapeVertex->SetProperty<Trigonometry::ShapeVertex>("float position_x", &Trigonometry::ShapeVertex::PositionX);
				VShapeVertex->SetProperty<Trigonometry::ShapeVertex>("float position_y", &Trigonometry::ShapeVertex::PositionY);
				VShapeVertex->SetProperty<Trigonometry::ShapeVertex>("float position_z", &Trigonometry::ShapeVertex::PositionZ);
				VShapeVertex->SetProperty<Trigonometry::ShapeVertex>("float texcoord_x", &Trigonometry::ShapeVertex::TexCoordX);
				VShapeVertex->SetProperty<Trigonometry::ShapeVertex>("float texcoord_y", &Trigonometry::ShapeVertex::TexCoordY);
				VShapeVertex->SetConstructor<Trigonometry::ShapeVertex>("void f()");

				auto VElementVertex = VM->SetPod<Trigonometry::ElementVertex>("element_vertex");
				VElementVertex->SetProperty<Trigonometry::ElementVertex>("float position_x", &Trigonometry::ElementVertex::PositionX);
				VElementVertex->SetProperty<Trigonometry::ElementVertex>("float position_y", &Trigonometry::ElementVertex::PositionY);
				VElementVertex->SetProperty<Trigonometry::ElementVertex>("float position_z", &Trigonometry::ElementVertex::PositionZ);
				VElementVertex->SetProperty<Trigonometry::ElementVertex>("float velocity_x", &Trigonometry::ElementVertex::VelocityX);
				VElementVertex->SetProperty<Trigonometry::ElementVertex>("float velocity_y", &Trigonometry::ElementVertex::VelocityY);
				VElementVertex->SetProperty<Trigonometry::ElementVertex>("float velocity_z", &Trigonometry::ElementVertex::VelocityZ);
				VElementVertex->SetProperty<Trigonometry::ElementVertex>("float color_x", &Trigonometry::ElementVertex::ColorX);
				VElementVertex->SetProperty<Trigonometry::ElementVertex>("float color_y", &Trigonometry::ElementVertex::ColorY);
				VElementVertex->SetProperty<Trigonometry::ElementVertex>("float color_z", &Trigonometry::ElementVertex::ColorZ);
				VElementVertex->SetProperty<Trigonometry::ElementVertex>("float color_w", &Trigonometry::ElementVertex::ColorW);
				VElementVertex->SetProperty<Trigonometry::ElementVertex>("float scale", &Trigonometry::ElementVertex::Scale);
				VElementVertex->SetProperty<Trigonometry::ElementVertex>("float rotation", &Trigonometry::ElementVertex::Rotation);
				VElementVertex->SetProperty<Trigonometry::ElementVertex>("float angular", &Trigonometry::ElementVertex::Angular);
				VElementVertex->SetConstructor<Trigonometry::ElementVertex>("void f()");

				auto VCubeFace = VM->SetEnum("cube_face");
				VCubeFace->SetValue("positive_x", (int)Trigonometry::CubeFace::PositiveX);
				VCubeFace->SetValue("negative_x", (int)Trigonometry::CubeFace::NegativeX);
				VCubeFace->SetValue("positive_y", (int)Trigonometry::CubeFace::PositiveY);
				VCubeFace->SetValue("negative_y", (int)Trigonometry::CubeFace::NegativeY);
				VCubeFace->SetValue("positive_z", (int)Trigonometry::CubeFace::PositiveZ);
				VCubeFace->SetValue("negative_z", (int)Trigonometry::CubeFace::NegativeZ);

				auto VVector2 = VM->SetPod<Trigonometry::Vector2>("vector2");
				VVector2->SetConstructor<Trigonometry::Vector2>("void f()");
				VVector2->SetConstructor<Trigonometry::Vector2, float, float>("void f(float, float)");
				VVector2->SetConstructor<Trigonometry::Vector2, float>("void f(float)");
				VVector2->SetProperty<Trigonometry::Vector2>("float x", &Trigonometry::Vector2::X);
				VVector2->SetProperty<Trigonometry::Vector2>("float y", &Trigonometry::Vector2::Y);
				VVector2->SetMethod("float length() const", &Trigonometry::Vector2::Length);
				VVector2->SetMethod("float sum() const", &Trigonometry::Vector2::Sum);
				VVector2->SetMethod("float dot(const vector2 &in) const", &Trigonometry::Vector2::Dot);
				VVector2->SetMethod("float distance(const vector2 &in) const", &Trigonometry::Vector2::Distance);
				VVector2->SetMethod("float hypotenuse() const", &Trigonometry::Vector2::Hypotenuse);
				VVector2->SetMethod("float look_at(const vector2 &in) const", &Trigonometry::Vector2::LookAt);
				VVector2->SetMethod("float cross(const vector2 &in) const", &Trigonometry::Vector2::Cross);
				VVector2->SetMethod("vector2 direction(float) const", &Trigonometry::Vector2::Direction);
				VVector2->SetMethod("vector2 inv() const", &Trigonometry::Vector2::Inv);
				VVector2->SetMethod("vector2 inv_x() const", &Trigonometry::Vector2::InvX);
				VVector2->SetMethod("vector2 inv_y() const", &Trigonometry::Vector2::InvY);
				VVector2->SetMethod("vector2 normalize() const", &Trigonometry::Vector2::Normalize);
				VVector2->SetMethod("vector2 snormalize() const", &Trigonometry::Vector2::sNormalize);
				VVector2->SetMethod("vector2 lerp(const vector2 &in, float) const", &Trigonometry::Vector2::Lerp);
				VVector2->SetMethod("vector2 slerp(const vector2 &in, float) const", &Trigonometry::Vector2::sLerp);
				VVector2->SetMethod("vector2 alerp(const vector2 &in, float) const", &Trigonometry::Vector2::aLerp);
				VVector2->SetMethod("vector2 rlerp() const", &Trigonometry::Vector2::rLerp);
				VVector2->SetMethod("vector2 abs() const", &Trigonometry::Vector2::Abs);
				VVector2->SetMethod("vector2 radians() const", &Trigonometry::Vector2::Radians);
				VVector2->SetMethod("vector2 degrees() const", &Trigonometry::Vector2::Degrees);
				VVector2->SetMethod("vector2 xy() const", &Trigonometry::Vector2::XY);
				VVector2->SetMethod<Trigonometry::Vector2, Trigonometry::Vector2, float>("vector2 mul(float) const", &Trigonometry::Vector2::Mul);
				VVector2->SetMethod<Trigonometry::Vector2, Trigonometry::Vector2, float, float>("vector2 mul(float, float) const", &Trigonometry::Vector2::Mul);
				VVector2->SetMethod<Trigonometry::Vector2, Trigonometry::Vector2, const Trigonometry::Vector2&>("vector2 mul(const vector2 &in) const", &Trigonometry::Vector2::Mul);
				VVector2->SetMethod("vector2 div(const vector2 &in) const", &Trigonometry::Vector2::Div);
				VVector2->SetMethod("vector2 set_x(float) const", &Trigonometry::Vector2::SetX);
				VVector2->SetMethod("vector2 set_y(float) const", &Trigonometry::Vector2::SetY);
				VVector2->SetMethod("void set(const vector2 &in) const", &Trigonometry::Vector2::Set);
				VVector2->SetOperatorEx(Operators::MulAssign, (uint32_t)Position::Left, "vector2&", "const vector2 &in", &Vector2MulEq1);
				VVector2->SetOperatorEx(Operators::MulAssign, (uint32_t)Position::Left, "vector2&", "float", &Vector2MulEq2);
				VVector2->SetOperatorEx(Operators::DivAssign, (uint32_t)Position::Left, "vector2&", "const vector2 &in", &Vector2DivEq1);
				VVector2->SetOperatorEx(Operators::DivAssign, (uint32_t)Position::Left, "vector2&", "float", &Vector2DivEq2);
				VVector2->SetOperatorEx(Operators::AddAssign, (uint32_t)Position::Left, "vector2&", "const vector2 &in", &Vector2AddEq1);
				VVector2->SetOperatorEx(Operators::AddAssign, (uint32_t)Position::Left, "vector2&", "float", &Vector2AddEq2);
				VVector2->SetOperatorEx(Operators::SubAssign, (uint32_t)Position::Left, "vector2&", "const vector2 &in", &Vector2SubEq1);
				VVector2->SetOperatorEx(Operators::SubAssign, (uint32_t)Position::Left, "vector2&", "float", &Vector2SubEq2);
				VVector2->SetOperatorEx(Operators::Mul, (uint32_t)Position::Const, "vector2", "const vector2 &in", &Vector2Mul1);
				VVector2->SetOperatorEx(Operators::Mul, (uint32_t)Position::Const, "vector2", "float", &Vector2Mul2);
				VVector2->SetOperatorEx(Operators::Div, (uint32_t)Position::Const, "vector2", "const vector2 &in", &Vector2Div1);
				VVector2->SetOperatorEx(Operators::Div, (uint32_t)Position::Const, "vector2", "float", &Vector2Div2);
				VVector2->SetOperatorEx(Operators::Add, (uint32_t)Position::Const, "vector2", "const vector2 &in", &Vector2Add1);
				VVector2->SetOperatorEx(Operators::Add, (uint32_t)Position::Const, "vector2", "float", &Vector2Add2);
				VVector2->SetOperatorEx(Operators::Sub, (uint32_t)Position::Const, "vector2", "const vector2 &in", &Vector2Sub1);
				VVector2->SetOperatorEx(Operators::Sub, (uint32_t)Position::Const, "vector2", "float", &Vector2Sub2);
				VVector2->SetOperatorEx(Operators::Neg, (uint32_t)Position::Const, "vector2", "", &Vector2Neg);
				VVector2->SetOperatorEx(Operators::Equals, (uint32_t)Position::Const, "bool", "const vector2 &in", &Vector2Eq);
				VVector2->SetOperatorEx(Operators::Cmp, (uint32_t)Position::Const, "int", "const vector2 &in", &Vector2Cmp);

				VM->BeginNamespace("vector2");
				VM->SetFunction("vector2 random()", &Trigonometry::Vector2::Random);
				VM->SetFunction("vector2 random_abs()", &Trigonometry::Vector2::RandomAbs);
				VM->SetFunction("vector2 one()", &Trigonometry::Vector2::One);
				VM->SetFunction("vector2 zero()", &Trigonometry::Vector2::Zero);
				VM->SetFunction("vector2 up()", &Trigonometry::Vector2::Up);
				VM->SetFunction("vector2 down()", &Trigonometry::Vector2::Down);
				VM->SetFunction("vector2 left()", &Trigonometry::Vector2::Left);
				VM->SetFunction("vector2 right()", &Trigonometry::Vector2::Right);
				VM->EndNamespace();

				auto VVector3 = VM->SetPod<Trigonometry::Vector3>("vector3");
				VVector3->SetConstructor<Trigonometry::Vector3>("void f()");
				VVector3->SetConstructor<Trigonometry::Vector3, float, float>("void f(float, float)");
				VVector3->SetConstructor<Trigonometry::Vector3, float, float, float>("void f(float, float, float)");
				VVector3->SetConstructor<Trigonometry::Vector3, float>("void f(float)");
				VVector3->SetConstructor<Trigonometry::Vector3, const Trigonometry::Vector2&>("void f(const vector2 &in)");
				VVector3->SetProperty<Trigonometry::Vector3>("float x", &Trigonometry::Vector3::X);
				VVector3->SetProperty<Trigonometry::Vector3>("float y", &Trigonometry::Vector3::Y);
				VVector3->SetProperty<Trigonometry::Vector3>("float z", &Trigonometry::Vector3::Z);
				VVector3->SetMethod("float length() const", &Trigonometry::Vector3::Length);
				VVector3->SetMethod("float sum() const", &Trigonometry::Vector3::Sum);
				VVector3->SetMethod("float dot(const vector3 &in) const", &Trigonometry::Vector3::Dot);
				VVector3->SetMethod("float distance(const vector3 &in) const", &Trigonometry::Vector3::Distance);
				VVector3->SetMethod("float hypotenuse() const", &Trigonometry::Vector3::Hypotenuse);
				VVector3->SetMethod("vector3 look_at(const vector3 &in) const", &Trigonometry::Vector3::LookAt);
				VVector3->SetMethod("vector3 cross(const vector3 &in) const", &Trigonometry::Vector3::Cross);
				VVector3->SetMethod("vector3 direction_horizontal() const", &Trigonometry::Vector3::hDirection);
				VVector3->SetMethod("vector3 direction_depth() const", &Trigonometry::Vector3::dDirection);
				VVector3->SetMethod("vector3 inv() const", &Trigonometry::Vector3::Inv);
				VVector3->SetMethod("vector3 inv_x() const", &Trigonometry::Vector3::InvX);
				VVector3->SetMethod("vector3 inv_y() const", &Trigonometry::Vector3::InvY);
				VVector3->SetMethod("vector3 inv_z() const", &Trigonometry::Vector3::InvZ);
				VVector3->SetMethod("vector3 normalize() const", &Trigonometry::Vector3::Normalize);
				VVector3->SetMethod("vector3 snormalize() const", &Trigonometry::Vector3::sNormalize);
				VVector3->SetMethod("vector3 lerp(const vector3 &in, float) const", &Trigonometry::Vector3::Lerp);
				VVector3->SetMethod("vector3 slerp(const vector3 &in, float) const", &Trigonometry::Vector3::sLerp);
				VVector3->SetMethod("vector3 alerp(const vector3 &in, float) const", &Trigonometry::Vector3::aLerp);
				VVector3->SetMethod("vector3 rlerp() const", &Trigonometry::Vector3::rLerp);
				VVector3->SetMethod("vector3 abs() const", &Trigonometry::Vector3::Abs);
				VVector3->SetMethod("vector3 radians() const", &Trigonometry::Vector3::Radians);
				VVector3->SetMethod("vector3 degrees() const", &Trigonometry::Vector3::Degrees);
				VVector3->SetMethod("vector3 view_space() const", &Trigonometry::Vector3::ViewSpace);
				VVector3->SetMethod("vector2 xy() const", &Trigonometry::Vector3::XY);
				VVector3->SetMethod("vector3 xyz() const", &Trigonometry::Vector3::XYZ);
				VVector3->SetMethod<Trigonometry::Vector3, Trigonometry::Vector3, float>("vector3 mul(float) const", &Trigonometry::Vector3::Mul);
				VVector3->SetMethod<Trigonometry::Vector3, Trigonometry::Vector3, const Trigonometry::Vector2&, float>("vector3 mul(const vector2 &in, float) const", &Trigonometry::Vector3::Mul);
				VVector3->SetMethod<Trigonometry::Vector3, Trigonometry::Vector3, const Trigonometry::Vector3&>("vector3 mul(const vector3 &in) const", &Trigonometry::Vector3::Mul);
				VVector3->SetMethod("vector3 div(const vector3 &in) const", &Trigonometry::Vector3::Div);
				VVector3->SetMethod("vector3 set_x(float) const", &Trigonometry::Vector3::SetX);
				VVector3->SetMethod("vector3 set_y(float) const", &Trigonometry::Vector3::SetY);
				VVector3->SetMethod("vector3 set_z(float) const", &Trigonometry::Vector3::SetZ);
				VVector3->SetMethod("void set(const vector3 &in) const", &Trigonometry::Vector3::Set);
				VVector3->SetOperatorEx(Operators::MulAssign, (uint32_t)Position::Left, "vector3&", "const vector3 &in", &Vector3MulEq1);
				VVector3->SetOperatorEx(Operators::MulAssign, (uint32_t)Position::Left, "vector3&", "float", &Vector3MulEq2);
				VVector3->SetOperatorEx(Operators::DivAssign, (uint32_t)Position::Left, "vector3&", "const vector3 &in", &Vector3DivEq1);
				VVector3->SetOperatorEx(Operators::DivAssign, (uint32_t)Position::Left, "vector3&", "float", &Vector3DivEq2);
				VVector3->SetOperatorEx(Operators::AddAssign, (uint32_t)Position::Left, "vector3&", "const vector3 &in", &Vector3AddEq1);
				VVector3->SetOperatorEx(Operators::AddAssign, (uint32_t)Position::Left, "vector3&", "float", &Vector3AddEq2);
				VVector3->SetOperatorEx(Operators::SubAssign, (uint32_t)Position::Left, "vector3&", "const vector3 &in", &Vector3SubEq1);
				VVector3->SetOperatorEx(Operators::SubAssign, (uint32_t)Position::Left, "vector3&", "float", &Vector3SubEq2);
				VVector3->SetOperatorEx(Operators::Mul, (uint32_t)Position::Const, "vector3", "const vector3 &in", &Vector3Mul1);
				VVector3->SetOperatorEx(Operators::Mul, (uint32_t)Position::Const, "vector3", "float", &Vector3Mul2);
				VVector3->SetOperatorEx(Operators::Div, (uint32_t)Position::Const, "vector3", "const vector3 &in", &Vector3Div1);
				VVector3->SetOperatorEx(Operators::Div, (uint32_t)Position::Const, "vector3", "float", &Vector3Div2);
				VVector3->SetOperatorEx(Operators::Add, (uint32_t)Position::Const, "vector3", "const vector3 &in", &Vector3Add1);
				VVector3->SetOperatorEx(Operators::Add, (uint32_t)Position::Const, "vector3", "float", &Vector3Add2);
				VVector3->SetOperatorEx(Operators::Sub, (uint32_t)Position::Const, "vector3", "const vector3 &in", &Vector3Sub1);
				VVector3->SetOperatorEx(Operators::Sub, (uint32_t)Position::Const, "vector3", "float", &Vector3Sub2);
				VVector3->SetOperatorEx(Operators::Neg, (uint32_t)Position::Const, "vector3", "", &Vector3Neg);
				VVector3->SetOperatorEx(Operators::Equals, (uint32_t)Position::Const, "bool", "const vector3 &in", &Vector3Eq);
				VVector3->SetOperatorEx(Operators::Cmp, (uint32_t)Position::Const, "int", "const vector3 &in", &Vector3Cmp);

				VM->BeginNamespace("vector3");
				VM->SetFunction("vector3 random()", &Trigonometry::Vector3::Random);
				VM->SetFunction("vector3 random_abs()", &Trigonometry::Vector3::RandomAbs);
				VM->SetFunction("vector3 one()", &Trigonometry::Vector3::One);
				VM->SetFunction("vector3 zero()", &Trigonometry::Vector3::Zero);
				VM->SetFunction("vector3 up()", &Trigonometry::Vector3::Up);
				VM->SetFunction("vector3 down()", &Trigonometry::Vector3::Down);
				VM->SetFunction("vector3 left()", &Trigonometry::Vector3::Left);
				VM->SetFunction("vector3 right()", &Trigonometry::Vector3::Right);
				VM->SetFunction("vector3 forward()", &Trigonometry::Vector3::Forward);
				VM->SetFunction("vector3 backward()", &Trigonometry::Vector3::Backward);
				VM->EndNamespace();

				auto VVector4 = VM->SetPod<Trigonometry::Vector4>("vector4");
				VVector4->SetConstructor<Trigonometry::Vector4>("void f()");
				VVector4->SetConstructor<Trigonometry::Vector4, float, float>("void f(float, float)");
				VVector4->SetConstructor<Trigonometry::Vector4, float, float, float>("void f(float, float, float)");
				VVector4->SetConstructor<Trigonometry::Vector4, float, float, float, float>("void f(float, float, float, float)");
				VVector4->SetConstructor<Trigonometry::Vector4, float>("void f(float)");
				VVector4->SetConstructor<Trigonometry::Vector4, const Trigonometry::Vector2&>("void f(const vector2 &in)");
				VVector4->SetConstructor<Trigonometry::Vector4, const Trigonometry::Vector3&>("void f(const vector3 &in)");
				VVector4->SetProperty<Trigonometry::Vector4>("float x", &Trigonometry::Vector4::X);
				VVector4->SetProperty<Trigonometry::Vector4>("float y", &Trigonometry::Vector4::Y);
				VVector4->SetProperty<Trigonometry::Vector4>("float z", &Trigonometry::Vector4::Z);
				VVector4->SetProperty<Trigonometry::Vector4>("float w", &Trigonometry::Vector4::W);
				VVector4->SetMethod("float length() const", &Trigonometry::Vector4::Length);
				VVector4->SetMethod("float sum() const", &Trigonometry::Vector4::Sum);
				VVector4->SetMethod("float dot(const vector4 &in) const", &Trigonometry::Vector4::Dot);
				VVector4->SetMethod("float distance(const vector4 &in) const", &Trigonometry::Vector4::Distance);
				VVector4->SetMethod("float cross(const vector4 &in) const", &Trigonometry::Vector4::Cross);
				VVector4->SetMethod("vector4 inv() const", &Trigonometry::Vector4::Inv);
				VVector4->SetMethod("vector4 inv_x() const", &Trigonometry::Vector4::InvX);
				VVector4->SetMethod("vector4 inv_y() const", &Trigonometry::Vector4::InvY);
				VVector4->SetMethod("vector4 inv_z() const", &Trigonometry::Vector4::InvZ);
				VVector4->SetMethod("vector4 inv_w() const", &Trigonometry::Vector4::InvW);
				VVector4->SetMethod("vector4 normalize() const", &Trigonometry::Vector4::Normalize);
				VVector4->SetMethod("vector4 snormalize() const", &Trigonometry::Vector4::sNormalize);
				VVector4->SetMethod("vector4 lerp(const vector4 &in, float) const", &Trigonometry::Vector4::Lerp);
				VVector4->SetMethod("vector4 slerp(const vector4 &in, float) const", &Trigonometry::Vector4::sLerp);
				VVector4->SetMethod("vector4 alerp(const vector4 &in, float) const", &Trigonometry::Vector4::aLerp);
				VVector4->SetMethod("vector4 rlerp() const", &Trigonometry::Vector4::rLerp);
				VVector4->SetMethod("vector4 abs() const", &Trigonometry::Vector4::Abs);
				VVector4->SetMethod("vector4 radians() const", &Trigonometry::Vector4::Radians);
				VVector4->SetMethod("vector4 degrees() const", &Trigonometry::Vector4::Degrees);
				VVector4->SetMethod("vector4 view_space() const", &Trigonometry::Vector4::ViewSpace);
				VVector4->SetMethod("vector2 xy() const", &Trigonometry::Vector4::XY);
				VVector4->SetMethod("vector3 xyz() const", &Trigonometry::Vector4::XYZ);
				VVector4->SetMethod("vector4 xyzw() const", &Trigonometry::Vector4::XYZW);
				VVector4->SetMethod<Trigonometry::Vector4, Trigonometry::Vector4, float>("vector4 mul(float) const", &Trigonometry::Vector4::Mul);
				VVector4->SetMethod<Trigonometry::Vector4, Trigonometry::Vector4, const Trigonometry::Vector2&, float, float>("vector4 mul(const vector2 &in, float, float) const", &Trigonometry::Vector4::Mul);
				VVector4->SetMethod<Trigonometry::Vector4, Trigonometry::Vector4, const Trigonometry::Vector3&, float>("vector4 mul(const vector3 &in, float) const", &Trigonometry::Vector4::Mul);
				VVector4->SetMethod<Trigonometry::Vector4, Trigonometry::Vector4, const Trigonometry::Vector4&>("vector4 mul(const vector4 &in) const", &Trigonometry::Vector4::Mul);
				VVector4->SetMethod("vector4 div(const vector4 &in) const", &Trigonometry::Vector4::Div);
				VVector4->SetMethod("vector4 set_x(float) const", &Trigonometry::Vector4::SetX);
				VVector4->SetMethod("vector4 set_y(float) const", &Trigonometry::Vector4::SetY);
				VVector4->SetMethod("vector4 set_z(float) const", &Trigonometry::Vector4::SetZ);
				VVector4->SetMethod("vector4 set_w(float) const", &Trigonometry::Vector4::SetW);
				VVector4->SetMethod("void set(const vector4 &in) const", &Trigonometry::Vector4::Set);
				VVector4->SetOperatorEx(Operators::MulAssign, (uint32_t)Position::Left, "vector4&", "const vector4 &in", &Vector4MulEq1);
				VVector4->SetOperatorEx(Operators::MulAssign, (uint32_t)Position::Left, "vector4&", "float", &Vector4MulEq2);
				VVector4->SetOperatorEx(Operators::DivAssign, (uint32_t)Position::Left, "vector4&", "const vector4 &in", &Vector4DivEq1);
				VVector4->SetOperatorEx(Operators::DivAssign, (uint32_t)Position::Left, "vector4&", "float", &Vector4DivEq2);
				VVector4->SetOperatorEx(Operators::AddAssign, (uint32_t)Position::Left, "vector4&", "const vector4 &in", &Vector4AddEq1);
				VVector4->SetOperatorEx(Operators::AddAssign, (uint32_t)Position::Left, "vector4&", "float", &Vector4AddEq2);
				VVector4->SetOperatorEx(Operators::SubAssign, (uint32_t)Position::Left, "vector4&", "const vector4 &in", &Vector4SubEq1);
				VVector4->SetOperatorEx(Operators::SubAssign, (uint32_t)Position::Left, "vector4&", "float", &Vector4SubEq2);
				VVector4->SetOperatorEx(Operators::Mul, (uint32_t)Position::Const, "vector4", "const vector4 &in", &Vector4Mul1);
				VVector4->SetOperatorEx(Operators::Mul, (uint32_t)Position::Const, "vector4", "float", &Vector4Mul2);
				VVector4->SetOperatorEx(Operators::Div, (uint32_t)Position::Const, "vector4", "const vector4 &in", &Vector4Div1);
				VVector4->SetOperatorEx(Operators::Div, (uint32_t)Position::Const, "vector4", "float", &Vector4Div2);
				VVector4->SetOperatorEx(Operators::Add, (uint32_t)Position::Const, "vector4", "const vector4 &in", &Vector4Add1);
				VVector4->SetOperatorEx(Operators::Add, (uint32_t)Position::Const, "vector4", "float", &Vector4Add2);
				VVector4->SetOperatorEx(Operators::Sub, (uint32_t)Position::Const, "vector4", "const vector4 &in", &Vector4Sub1);
				VVector4->SetOperatorEx(Operators::Sub, (uint32_t)Position::Const, "vector4", "float", &Vector4Sub2);
				VVector4->SetOperatorEx(Operators::Neg, (uint32_t)Position::Const, "vector4", "", &Vector4Neg);
				VVector4->SetOperatorEx(Operators::Equals, (uint32_t)Position::Const, "bool", "const vector4 &in", &Vector4Eq);
				VVector4->SetOperatorEx(Operators::Cmp, (uint32_t)Position::Const, "int", "const vector4 &in", &Vector4Cmp);

				VM->BeginNamespace("vector4");
				VM->SetFunction("vector4 random()", &Trigonometry::Vector4::Random);
				VM->SetFunction("vector4 random_abs()", &Trigonometry::Vector4::RandomAbs);
				VM->SetFunction("vector4 one()", &Trigonometry::Vector4::One);
				VM->SetFunction("vector4 zero()", &Trigonometry::Vector4::Zero);
				VM->SetFunction("vector4 up()", &Trigonometry::Vector4::Up);
				VM->SetFunction("vector4 down()", &Trigonometry::Vector4::Down);
				VM->SetFunction("vector4 left()", &Trigonometry::Vector4::Left);
				VM->SetFunction("vector4 right()", &Trigonometry::Vector4::Right);
				VM->SetFunction("vector4 forward()", &Trigonometry::Vector4::Forward);
				VM->SetFunction("vector4 backward()", &Trigonometry::Vector4::Backward);
				VM->SetFunction("vector4 unitW()", &Trigonometry::Vector4::UnitW);
				VM->EndNamespace();

				auto VMatrix4x4 = VM->SetPod<Trigonometry::Matrix4x4>("matrix4x4");
				VMatrix4x4->SetConstructor<Trigonometry::Matrix4x4>("void f()");
				VMatrix4x4->SetConstructor<Trigonometry::Matrix4x4, const Trigonometry::Vector4&, const Trigonometry::Vector4&, const Trigonometry::Vector4&, const Trigonometry::Vector4&>("void f(const vector4 &in, const vector4 &in, const vector4 &in, const vector4 &in)");
				VMatrix4x4->SetConstructor<Trigonometry::Matrix4x4, float, float, float, float, float, float, float, float, float, float, float, float, float, float, float, float>("void f(float, float, float, float, float, float, float, float, float, float, float, float, float, float, float, float)");
				VMatrix4x4->SetMethod("matrix4x4 inv() const", &Trigonometry::Matrix4x4::Inv);
				VMatrix4x4->SetMethod("matrix4x4 transpose() const", &Trigonometry::Matrix4x4::Transpose);
				VMatrix4x4->SetMethod("matrix4x4 set_scale(const vector3 &in) const", &Trigonometry::Matrix4x4::SetScale);
				VMatrix4x4->SetMethod("vector4 row11() const", &Trigonometry::Matrix4x4::Row11);
				VMatrix4x4->SetMethod("vector4 row22() const", &Trigonometry::Matrix4x4::Row22);
				VMatrix4x4->SetMethod("vector4 row33() const", &Trigonometry::Matrix4x4::Row33);
				VMatrix4x4->SetMethod("vector4 row44() const", &Trigonometry::Matrix4x4::Row44);
				VMatrix4x4->SetMethod("vector3 up() const", &Trigonometry::Matrix4x4::Up);
				VMatrix4x4->SetMethod("vector3 right() const", &Trigonometry::Matrix4x4::Right);
				VMatrix4x4->SetMethod("vector3 forward() const", &Trigonometry::Matrix4x4::Forward);
				VMatrix4x4->SetMethod("vector3 rotation_quaternion() const", &Trigonometry::Matrix4x4::RotationQuaternion);
				VMatrix4x4->SetMethod("vector3 rotation_euler() const", &Trigonometry::Matrix4x4::RotationEuler);
				VMatrix4x4->SetMethod("vector3 position() const", &Trigonometry::Matrix4x4::Position);
				VMatrix4x4->SetMethod("vector3 scale() const", &Trigonometry::Matrix4x4::Scale);
				VMatrix4x4->SetMethod("vector3 xy() const", &Trigonometry::Matrix4x4::XY);
				VMatrix4x4->SetMethod("vector3 xyz() const", &Trigonometry::Matrix4x4::XYZ);
				VMatrix4x4->SetMethod("vector3 xyzw() const", &Trigonometry::Matrix4x4::XYZW);
				VMatrix4x4->SetMethod("void identify()", &Trigonometry::Matrix4x4::Identify);
				VMatrix4x4->SetMethod("void set(const matrix4x4 &in)", &Trigonometry::Matrix4x4::Set);
				VMatrix4x4->SetMethod<Trigonometry::Matrix4x4, Trigonometry::Matrix4x4, const Trigonometry::Matrix4x4&>("matrix4x4 mul(const matrix4x4 &in) const", &Trigonometry::Matrix4x4::Mul);
				VMatrix4x4->SetMethod<Trigonometry::Matrix4x4, Trigonometry::Matrix4x4, const Trigonometry::Vector4&>("matrix4x4 mul(const vector4 &in) const", &Trigonometry::Matrix4x4::Mul);
				VMatrix4x4->SetOperatorEx(Operators::Index, (uint32_t)Position::Left, "float&", "usize", &Matrix4x4GetRow);
				VMatrix4x4->SetOperatorEx(Operators::Index, (uint32_t)Position::Const, "const float&", "usize", &Matrix4x4GetRow);
				VMatrix4x4->SetOperatorEx(Operators::Equals, (uint32_t)Position::Const, "bool", "const matrix4x4 &in", &Matrix4x4Eq);
				VMatrix4x4->SetOperatorEx(Operators::Mul, (uint32_t)Position::Const, "matrix4x4", "const matrix4x4 &in", &Matrix4x4Mul1);
				VMatrix4x4->SetOperatorEx(Operators::Mul, (uint32_t)Position::Const, "vector4", "const vector4 &in", &Matrix4x4Mul2);

				VM->BeginNamespace("matrix4x4");
				VM->SetFunction("matrix4x4 identity()", &Trigonometry::Matrix4x4::Identity);
				VM->SetFunction("matrix4x4 create_rotation_x(float)", &Trigonometry::Matrix4x4::CreateRotationX);
				VM->SetFunction("matrix4x4 create_rotation_y(float)", &Trigonometry::Matrix4x4::CreateRotationY);
				VM->SetFunction("matrix4x4 create_rotation_z(float)", &Trigonometry::Matrix4x4::CreateRotationZ);
				VM->SetFunction("matrix4x4 create_view(const vector3 &in, const vector3 &in)", &Trigonometry::Matrix4x4::CreateView);
				VM->SetFunction("matrix4x4 create_scale(const vector3 &in)", &Trigonometry::Matrix4x4::CreateScale);
				VM->SetFunction("matrix4x4 create_translated_scale(const vector3& in, const vector3 &in)", &Trigonometry::Matrix4x4::CreateTranslatedScale);
				VM->SetFunction("matrix4x4 create_translation(const vector3& in)", &Trigonometry::Matrix4x4::CreateTranslation);
				VM->SetFunction("matrix4x4 create_perspective(float, float, float, float)", &Trigonometry::Matrix4x4::CreatePerspective);
				VM->SetFunction("matrix4x4 create_perspective_rad(float, float, float, float)", &Trigonometry::Matrix4x4::CreatePerspectiveRad);
				VM->SetFunction("matrix4x4 create_orthographic(float, float, float, float)", &Trigonometry::Matrix4x4::CreateOrthographic);
				VM->SetFunction("matrix4x4 create_orthographic_off_center(float, float, float, float, float, float)", &Trigonometry::Matrix4x4::CreateOrthographicOffCenter);
				VM->SetFunction<Trigonometry::Matrix4x4(const Trigonometry::Vector3&)>("matrix4x4 createRotation(const vector3 &in)", &Trigonometry::Matrix4x4::CreateRotation);
				VM->SetFunction<Trigonometry::Matrix4x4(const Trigonometry::Vector3&, const Trigonometry::Vector3&, const Trigonometry::Vector3&)>("matrix4x4 create_rotation(const vector3 &in, const vector3 &in, const vector3 &in)", &Trigonometry::Matrix4x4::CreateRotation);
				VM->SetFunction<Trigonometry::Matrix4x4(const Trigonometry::Vector3&, const Trigonometry::Vector3&, const Trigonometry::Vector3&)>("matrix4x4 create_look_at(const vector3 &in, const vector3 &in, const vector3 &in)", &Trigonometry::Matrix4x4::CreateLookAt);
				VM->SetFunction<Trigonometry::Matrix4x4(Trigonometry::CubeFace, const Trigonometry::Vector3&)>("matrix4x4 create_look_at(cube_face, const vector3 &in)", &Trigonometry::Matrix4x4::CreateLookAt);
				VM->SetFunction<Trigonometry::Matrix4x4(const Trigonometry::Vector3&, const Trigonometry::Vector3&, const Trigonometry::Vector3&)>("matrix4x4 create(const vector3 &in, const vector3 &in, const vector3 &in)", &Trigonometry::Matrix4x4::Create);
				VM->SetFunction<Trigonometry::Matrix4x4(const Trigonometry::Vector3&, const Trigonometry::Vector3&)>("matrix4x4 create(const vector3 &in, const vector3 &in)", &Trigonometry::Matrix4x4::Create);
				VM->EndNamespace();

				auto VQuaternion = VM->SetPod<Trigonometry::Vector4>("quaternion");
				VQuaternion->SetConstructor<Trigonometry::Quaternion>("void f()");
				VQuaternion->SetConstructor<Trigonometry::Quaternion, float, float, float, float>("void f(float, float, float, float)");
				VQuaternion->SetConstructor<Trigonometry::Quaternion, const Trigonometry::Vector3&, float>("void f(const vector3 &in, float)");
				VQuaternion->SetConstructor<Trigonometry::Quaternion, const Trigonometry::Vector3&>("void f(const vector3 &in)");
				VQuaternion->SetConstructor<Trigonometry::Quaternion, const Trigonometry::Matrix4x4&>("void f(const matrix4x4 &in)");
				VQuaternion->SetProperty<Trigonometry::Quaternion>("float x", &Trigonometry::Quaternion::X);
				VQuaternion->SetProperty<Trigonometry::Quaternion>("float y", &Trigonometry::Quaternion::Y);
				VQuaternion->SetProperty<Trigonometry::Quaternion>("float z", &Trigonometry::Quaternion::Z);
				VQuaternion->SetProperty<Trigonometry::Quaternion>("float w", &Trigonometry::Quaternion::W);
				VQuaternion->SetMethod("void set_axis(const vector3 &in, float)", &Trigonometry::Quaternion::SetAxis);
				VQuaternion->SetMethod("void set_euler(const vector3 &in)", &Trigonometry::Quaternion::SetEuler);
				VQuaternion->SetMethod("void set_matrix(const matrix4x4 &in)", &Trigonometry::Quaternion::SetMatrix);
				VQuaternion->SetMethod("void set(const quaternion &in)", &Trigonometry::Quaternion::Set);
				VQuaternion->SetMethod("quaternion normalize() const", &Trigonometry::Quaternion::Normalize);
				VQuaternion->SetMethod("quaternion snormalize() const", &Trigonometry::Quaternion::sNormalize);
				VQuaternion->SetMethod("quaternion conjugate() const", &Trigonometry::Quaternion::Conjugate);
				VQuaternion->SetMethod("quaternion sub(const quaternion &in) const", &Trigonometry::Quaternion::Sub);
				VQuaternion->SetMethod("quaternion add(const quaternion &in) const", &Trigonometry::Quaternion::Add);
				VQuaternion->SetMethod("quaternion lerp(const quaternion &in, float) const", &Trigonometry::Quaternion::Lerp);
				VQuaternion->SetMethod("quaternion slerp(const quaternion &in, float) const", &Trigonometry::Quaternion::sLerp);
				VQuaternion->SetMethod("vector3 forward() const", &Trigonometry::Quaternion::Forward);
				VQuaternion->SetMethod("vector3 up() const", &Trigonometry::Quaternion::Up);
				VQuaternion->SetMethod("vector3 right() const", &Trigonometry::Quaternion::Right);
				VQuaternion->SetMethod("matrix4x4 get_matrix() const", &Trigonometry::Quaternion::GetMatrix);
				VQuaternion->SetMethod("vector3 get_euler() const", &Trigonometry::Quaternion::GetEuler);
				VQuaternion->SetMethod("float dot(const quaternion &in) const", &Trigonometry::Quaternion::Dot);
				VQuaternion->SetMethod("float length() const", &Trigonometry::Quaternion::Length);
				VQuaternion->SetMethod<Trigonometry::Quaternion, Trigonometry::Quaternion, float>("quaternion mul(float) const", &Trigonometry::Quaternion::Mul);
				VQuaternion->SetMethod<Trigonometry::Quaternion, Trigonometry::Quaternion, const Trigonometry::Quaternion&>("quaternion mul(const quaternion &in) const", &Trigonometry::Quaternion::Mul);
				VQuaternion->SetMethod<Trigonometry::Quaternion, Trigonometry::Vector3, const Trigonometry::Vector3&>("vector3 mul(const vector3 &in) const", &Trigonometry::Quaternion::Mul);
				VQuaternion->SetOperatorEx(Operators::Mul, (uint32_t)Position::Const, "vector3", "const vector3 &in", &QuaternionMul1);
				VQuaternion->SetOperatorEx(Operators::Mul, (uint32_t)Position::Const, "quaternion", "const quaternion &in", &QuaternionMul2);
				VQuaternion->SetOperatorEx(Operators::Mul, (uint32_t)Position::Const, "quaternion", "float", &QuaternionMul3);
				VQuaternion->SetOperatorEx(Operators::Add, (uint32_t)Position::Const, "quaternion", "const quaternion &in", &QuaternionAdd);
				VQuaternion->SetOperatorEx(Operators::Sub, (uint32_t)Position::Const, "quaternion", "const quaternion &in", &QuaternionSub);

				VM->BeginNamespace("quaternion");
				VM->SetFunction("quaternion create_euler_rotation(const vector3 &in)", &Trigonometry::Quaternion::CreateEulerRotation);
				VM->SetFunction("quaternion create_rotation(const matrix4x4 &in)", &Trigonometry::Quaternion::CreateRotation);
				VM->EndNamespace();

				auto VRectangle = VM->SetPod<Trigonometry::Rectangle>("rectangle");
				VRectangle->SetProperty<Trigonometry::Rectangle>("int64 left", &Trigonometry::Rectangle::Left);
				VRectangle->SetProperty<Trigonometry::Rectangle>("int64 top", &Trigonometry::Rectangle::Top);
				VRectangle->SetProperty<Trigonometry::Rectangle>("int64 right", &Trigonometry::Rectangle::Right);
				VRectangle->SetProperty<Trigonometry::Rectangle>("int64 bottom", &Trigonometry::Rectangle::Bottom);
				VRectangle->SetConstructor<Trigonometry::Rectangle>("void f()");

				auto VBounding = VM->SetPod<Trigonometry::Bounding>("bounding");
				VBounding->SetProperty<Trigonometry::Bounding>("vector3 lower", &Trigonometry::Bounding::Lower);
				VBounding->SetProperty<Trigonometry::Bounding>("vector3 upper", &Trigonometry::Bounding::Upper);
				VBounding->SetProperty<Trigonometry::Bounding>("vector3 center", &Trigonometry::Bounding::Center);
				VBounding->SetProperty<Trigonometry::Bounding>("float radius", &Trigonometry::Bounding::Radius);
				VBounding->SetProperty<Trigonometry::Bounding>("float volume", &Trigonometry::Bounding::Volume);
				VBounding->SetConstructor<Trigonometry::Bounding>("void f()");
				VBounding->SetConstructor<Trigonometry::Bounding, const Trigonometry::Vector3&, const Trigonometry::Vector3&>("void f(const vector3 &in, const vector3 &in)");
				VBounding->SetMethod("void merge(const bounding &in, const bounding &in)", &Trigonometry::Bounding::Merge);
				VBounding->SetMethod("bool contains(const bounding &in) const", &Trigonometry::Bounding::Contains);
				VBounding->SetMethod("bool overlaps(const bounding &in) const", &Trigonometry::Bounding::Overlaps);

				auto VRay = VM->SetPod<Trigonometry::Ray>("ray");
				VRay->SetProperty<Trigonometry::Ray>("vector3 origin", &Trigonometry::Ray::Origin);
				VRay->SetProperty<Trigonometry::Ray>("vector3 direction", &Trigonometry::Ray::Direction);
				VRay->SetConstructor<Trigonometry::Ray>("void f()");
				VRay->SetConstructor<Trigonometry::Ray, const Trigonometry::Vector3&, const Trigonometry::Vector3&>("void f(const vector3 &in, const vector3 &in)");
				VRay->SetMethod("vector3 get_point(float) const", &Trigonometry::Ray::GetPoint);
				VRay->SetMethod("bool intersects_plane(const vector3 &in, float) const", &Trigonometry::Ray::IntersectsPlane);
				VRay->SetMethod("bool intersects_sphere(const vector3 &in, float, bool = true) const", &Trigonometry::Ray::IntersectsSphere);
				VRay->SetMethod("bool intersects_aabb_at(const vector3 &in, const vector3 &in, vector3 &out) const", &Trigonometry::Ray::IntersectsAABBAt);
				VRay->SetMethod("bool intersects_aabb(const vector3 &in, const vector3 &in, vector3 &out) const", &Trigonometry::Ray::IntersectsAABB);
				VRay->SetMethod("bool intersects_obb(const matrix4x4 &in, vector3 &out) const", &Trigonometry::Ray::IntersectsOBB);

				auto VFrustum8C = VM->SetPod<Trigonometry::Frustum8C>("frustum_8c");
				VFrustum8C->SetConstructor<Trigonometry::Frustum8C>("void f()");
				VFrustum8C->SetConstructor<Trigonometry::Frustum8C, float, float, float, float>("void f(float, float, float, float)");
				VFrustum8C->SetMethod("void transform(const matrix4x4 &in) const", &Trigonometry::Frustum8C::Transform);
				VFrustum8C->SetMethod("void get_bounding_box(vector2 &out, vector2 &out, vector2 &out) const", &Trigonometry::Frustum8C::GetBoundingBox);
				VFrustum8C->SetOperatorEx(Operators::Index, (uint32_t)Position::Left, "vector4&", "usize", &Frustum8CGetCorners);
				VFrustum8C->SetOperatorEx(Operators::Index, (uint32_t)Position::Const, "const vector4&", "usize", &Frustum8CGetCorners);

				auto VFrustum6P = VM->SetPod<Trigonometry::Frustum6P>("frustum_6p");
				VFrustum6P->SetConstructor<Trigonometry::Frustum6P>("void f()");
				VFrustum6P->SetConstructor<Trigonometry::Frustum6P, const Trigonometry::Matrix4x4&>("void f(const matrix4x4 &in)");
				VFrustum6P->SetMethod("bool overlaps_aabb(const bounding &in) const", &Trigonometry::Frustum6P::OverlapsAABB);
				VFrustum6P->SetMethod("bool overlaps_sphere(const vector3 &in, float) const", &Trigonometry::Frustum6P::OverlapsSphere);
				VFrustum6P->SetOperatorEx(Operators::Index, (uint32_t)Position::Left, "vector4&", "usize", &Frustum6PGetCorners);
				VFrustum6P->SetOperatorEx(Operators::Index, (uint32_t)Position::Const, "const vector4&", "usize", &Frustum6PGetCorners);

				auto VJoint = VM->SetStructTrivial<Trigonometry::Joint>("joint");
				VJoint->SetProperty<Trigonometry::Joint>("string name", &Trigonometry::Joint::Name);
				VJoint->SetProperty<Trigonometry::Joint>("matrix4x4 global", &Trigonometry::Joint::Global);
				VJoint->SetProperty<Trigonometry::Joint>("matrix4x4 local", &Trigonometry::Joint::Local);
				VJoint->SetProperty<Trigonometry::Joint>("usize index", &Trigonometry::Joint::Index);
				VJoint->SetConstructor<Trigonometry::Joint>("void f()");
				VJoint->SetMethodEx("usize size() const", &JointSize);
				VJoint->SetOperatorEx(Operators::Index, (uint32_t)Position::Left, "joint&", "usize", &JointGetChilds);
				VJoint->SetOperatorEx(Operators::Index, (uint32_t)Position::Const, "const joint&", "usize", &JointGetChilds);

				auto VAnimatorKey = VM->SetPod<Trigonometry::AnimatorKey>("animator_key");
				VAnimatorKey->SetProperty<Trigonometry::AnimatorKey>("vector3 position", &Trigonometry::AnimatorKey::Position);
				VAnimatorKey->SetProperty<Trigonometry::AnimatorKey>("quaternion rotation", &Trigonometry::AnimatorKey::Rotation);
				VAnimatorKey->SetProperty<Trigonometry::AnimatorKey>("vector3 scale", &Trigonometry::AnimatorKey::Scale);
				VAnimatorKey->SetProperty<Trigonometry::AnimatorKey>("float time", &Trigonometry::AnimatorKey::Time);
				VAnimatorKey->SetConstructor<Trigonometry::AnimatorKey>("void f()");

				auto VSkinAnimatorKey = VM->SetStructTrivial<Trigonometry::SkinAnimatorKey>("skin_animator_key");
				VSkinAnimatorKey->SetProperty<Trigonometry::SkinAnimatorKey>("float time", &Trigonometry::SkinAnimatorKey::Time);
				VSkinAnimatorKey->SetConstructor<Trigonometry::AnimatorKey>("void f()");
				VSkinAnimatorKey->SetMethodEx("usize size() const", &SkinAnimatorKeySize);
				VSkinAnimatorKey->SetOperatorEx(Operators::Index, (uint32_t)Position::Left, "animator_key&", "usize", &SkinAnimatorKeyGetPose);
				VSkinAnimatorKey->SetOperatorEx(Operators::Index, (uint32_t)Position::Const, "const animator_key&", "usize", &SkinAnimatorKeyGetPose);

				auto VSkinAnimatorClip = VM->SetStructTrivial<Trigonometry::SkinAnimatorClip>("skin_animator_clip");
				VSkinAnimatorClip->SetProperty<Trigonometry::SkinAnimatorClip>("string name", &Trigonometry::SkinAnimatorClip::Name);
				VSkinAnimatorClip->SetProperty<Trigonometry::SkinAnimatorClip>("float duration", &Trigonometry::SkinAnimatorClip::Duration);
				VSkinAnimatorClip->SetProperty<Trigonometry::SkinAnimatorClip>("float rate", &Trigonometry::SkinAnimatorClip::Rate);
				VSkinAnimatorClip->SetConstructor<Trigonometry::SkinAnimatorClip>("void f()");
				VSkinAnimatorClip->SetMethodEx("usize size() const", &SkinAnimatorClipSize);
				VSkinAnimatorClip->SetOperatorEx(Operators::Index, (uint32_t)Position::Left, "skin_animator_key&", "usize", &SkinAnimatorClipGetKeys);
				VSkinAnimatorClip->SetOperatorEx(Operators::Index, (uint32_t)Position::Const, "const skin_animator_key&", "usize", &SkinAnimatorClipGetKeys);

				auto VKeyAnimatorClip = VM->SetStructTrivial<Trigonometry::KeyAnimatorClip>("key_animator_clip");
				VKeyAnimatorClip->SetProperty<Trigonometry::KeyAnimatorClip>("string name", &Trigonometry::KeyAnimatorClip::Name);
				VKeyAnimatorClip->SetProperty<Trigonometry::KeyAnimatorClip>("float duration", &Trigonometry::KeyAnimatorClip::Duration);
				VKeyAnimatorClip->SetProperty<Trigonometry::KeyAnimatorClip>("float rate", &Trigonometry::KeyAnimatorClip::Rate);
				VKeyAnimatorClip->SetConstructor<Trigonometry::KeyAnimatorClip>("void f()");
				VKeyAnimatorClip->SetMethodEx("usize size() const", &KeyAnimatorClipSize);
				VKeyAnimatorClip->SetOperatorEx(Operators::Index, (uint32_t)Position::Left, "animator_key&", "usize", &KeyAnimatorClipGetKeys);
				VKeyAnimatorClip->SetOperatorEx(Operators::Index, (uint32_t)Position::Const, "const animator_key&", "usize", &KeyAnimatorClipGetKeys);

				auto VRandomVector2 = VM->SetPod<Trigonometry::RandomVector2>("random_vector2");
				VRandomVector2->SetProperty<Trigonometry::RandomVector2>("vector2 min", &Trigonometry::RandomVector2::Min);
				VRandomVector2->SetProperty<Trigonometry::RandomVector2>("vector2 max", &Trigonometry::RandomVector2::Max);
				VRandomVector2->SetProperty<Trigonometry::RandomVector2>("bool intensity", &Trigonometry::RandomVector2::Intensity);
				VRandomVector2->SetProperty<Trigonometry::RandomVector2>("float accuracy", &Trigonometry::RandomVector2::Accuracy);
				VRandomVector2->SetConstructor<Trigonometry::RandomVector2>("void f()");
				VRandomVector2->SetConstructor<Trigonometry::RandomVector2, const Trigonometry::Vector2&, const Trigonometry::Vector2&, bool, float>("void f(const vector2 &in, const vector2 &in, bool, float)");
				VRandomVector2->SetMethod("vector2 generate()", &Trigonometry::RandomVector2::Generate);

				auto VRandomVector3 = VM->SetPod<Trigonometry::RandomVector3>("random_vector3");
				VRandomVector3->SetProperty<Trigonometry::RandomVector3>("vector3 min", &Trigonometry::RandomVector3::Min);
				VRandomVector3->SetProperty<Trigonometry::RandomVector3>("vector3 max", &Trigonometry::RandomVector3::Max);
				VRandomVector3->SetProperty<Trigonometry::RandomVector3>("bool intensity", &Trigonometry::RandomVector3::Intensity);
				VRandomVector3->SetProperty<Trigonometry::RandomVector3>("float accuracy", &Trigonometry::RandomVector3::Accuracy);
				VRandomVector3->SetConstructor<Trigonometry::RandomVector3>("void f()");
				VRandomVector3->SetConstructor<Trigonometry::RandomVector3, const Trigonometry::Vector3&, const Trigonometry::Vector3&, bool, float>("void f(const vector3 &in, const vector3 &in, bool, float)");
				VRandomVector3->SetMethod("vector3 generate()", &Trigonometry::RandomVector3::Generate);

				auto VRandomVector4 = VM->SetPod<Trigonometry::RandomVector4>("random_vector4");
				VRandomVector4->SetProperty<Trigonometry::RandomVector4>("vector4 min", &Trigonometry::RandomVector4::Min);
				VRandomVector4->SetProperty<Trigonometry::RandomVector4>("vector4 max", &Trigonometry::RandomVector4::Max);
				VRandomVector4->SetProperty<Trigonometry::RandomVector4>("bool intensity", &Trigonometry::RandomVector4::Intensity);
				VRandomVector4->SetProperty<Trigonometry::RandomVector4>("float accuracy", &Trigonometry::RandomVector4::Accuracy);
				VRandomVector4->SetConstructor<Trigonometry::RandomVector4>("void f()");
				VRandomVector4->SetConstructor<Trigonometry::RandomVector4, const Trigonometry::Vector4&, const Trigonometry::Vector4&, bool, float>("void f(const vector4 &in, const vector4 &in, bool, float)");
				VRandomVector4->SetMethod("vector4 generate()", &Trigonometry::RandomVector4::Generate);

				auto VRandomFloat = VM->SetPod<Trigonometry::RandomFloat>("random_float");
				VRandomFloat->SetProperty<Trigonometry::RandomFloat>("float min", &Trigonometry::RandomFloat::Min);
				VRandomFloat->SetProperty<Trigonometry::RandomFloat>("float max", &Trigonometry::RandomFloat::Max);
				VRandomFloat->SetProperty<Trigonometry::RandomFloat>("bool intensity", &Trigonometry::RandomFloat::Intensity);
				VRandomFloat->SetProperty<Trigonometry::RandomFloat>("float accuracy", &Trigonometry::RandomFloat::Accuracy);
				VRandomFloat->SetConstructor<Trigonometry::RandomFloat>("void f()");
				VRandomFloat->SetConstructor<Trigonometry::RandomFloat, float, float, bool, float>("void f(float, float, bool, float)");
				VRandomFloat->SetMethod("float generate()", &Trigonometry::RandomFloat::Generate);

				auto VPositioning = VM->SetEnum("positioning");
				VPositioning->SetValue("local", (int)Trigonometry::Positioning::Local);
				VPositioning->SetValue("global", (int)Trigonometry::Positioning::Global);

				auto VSpacing = VM->SetPod<Trigonometry::Transform::Spacing>("transform_spacing");
				VSpacing->SetProperty<Trigonometry::Transform::Spacing>("matrix4x4 offset", &Trigonometry::Transform::Spacing::Offset);
				VSpacing->SetProperty<Trigonometry::Transform::Spacing>("vector3 position", &Trigonometry::Transform::Spacing::Position);
				VSpacing->SetProperty<Trigonometry::Transform::Spacing>("vector3 rotation", &Trigonometry::Transform::Spacing::Rotation);
				VSpacing->SetProperty<Trigonometry::Transform::Spacing>("vector3 scale", &Trigonometry::Transform::Spacing::Scale);
				VSpacing->SetConstructor<Trigonometry::Transform::Spacing>("void f()");

				auto VTransform = VM->SetClass<Trigonometry::Transform>("transform", false);
				VTransform->SetProperty<Trigonometry::Transform>("uptr@ user_data", &Trigonometry::Transform::UserData);
				VTransform->SetConstructor<Trigonometry::Transform, void*>("transform@ f(uptr@)");
				VTransform->SetMethod("void synchronize()", &Trigonometry::Transform::Synchronize);
				VTransform->SetMethod("void move(const vector3 &in)", &Trigonometry::Transform::Move);
				VTransform->SetMethod("void rotate(const vector3 &in)", &Trigonometry::Transform::Rotate);
				VTransform->SetMethod("void rescale(const vector3 &in)", &Trigonometry::Transform::Rescale);
				VTransform->SetMethod("void localize(transform_spacing &in)", &Trigonometry::Transform::Localize);
				VTransform->SetMethod("void globalize(transform_spacing &in)", &Trigonometry::Transform::Globalize);
				VTransform->SetMethod("void specialize(transform_spacing &in)", &Trigonometry::Transform::Specialize);
				VTransform->SetMethod("void copy(transform@+) const", &Trigonometry::Transform::Copy);
				VTransform->SetMethod("void add_child(transform@+)", &Trigonometry::Transform::AddChild);
				VTransform->SetMethod("void remove_child(transform@+)", &Trigonometry::Transform::RemoveChild);
				VTransform->SetMethod("void remove_childs()", &Trigonometry::Transform::RemoveChilds);
				VTransform->SetMethod("void make_dirty()", &Trigonometry::Transform::MakeDirty);
				VTransform->SetMethod("void set_scaling(bool)", &Trigonometry::Transform::SetScaling);
				VTransform->SetMethod("void set_position(const vector3 &in)", &Trigonometry::Transform::SetPosition);
				VTransform->SetMethod("void set_rotation(const vector3 &in)", &Trigonometry::Transform::SetRotation);
				VTransform->SetMethod("void set_scale(const vector3 &in)", &Trigonometry::Transform::SetScale);
				VTransform->SetMethod("void set_spacing(positioning, transform_spacing &in)", &Trigonometry::Transform::SetSpacing);
				VTransform->SetMethod("void set_pivot(transform@+, transform_spacing &in)", &Trigonometry::Transform::SetPivot);
				VTransform->SetMethod("void set_root(transform@+)", &Trigonometry::Transform::SetRoot);
				VTransform->SetMethod("void get_bounds(matrix4x4 &in, vector3 &in, vector3 &in)", &Trigonometry::Transform::GetBounds);
				VTransform->SetMethod("bool has_root(transform@+) const", &Trigonometry::Transform::HasRoot);
				VTransform->SetMethod("bool has_child(transform@+) const", &Trigonometry::Transform::HasChild);
				VTransform->SetMethod("bool has_scaling() const", &Trigonometry::Transform::HasScaling);
				VTransform->SetMethod("bool is_dirty() const", &Trigonometry::Transform::IsDirty);
				VTransform->SetMethod("const matrix4x4& get_bias() const", &Trigonometry::Transform::GetBias);
				VTransform->SetMethod("const matrix4x4& get_bias_unscaled() const", &Trigonometry::Transform::GetBiasUnscaled);
				VTransform->SetMethod("const vector3& get_position() const", &Trigonometry::Transform::GetPosition);
				VTransform->SetMethod("const vector3& get_rotation() const", &Trigonometry::Transform::GetRotation);
				VTransform->SetMethod("const vector3& get_scale() const", &Trigonometry::Transform::GetScale);
				VTransform->SetMethod("vector3 forward() const", &Trigonometry::Transform::Forward);
				VTransform->SetMethod("vector3 right() const", &Trigonometry::Transform::Right);
				VTransform->SetMethod("vector3 up() const", &Trigonometry::Transform::Up);
				VTransform->SetMethod<Trigonometry::Transform, Trigonometry::Transform::Spacing&>("transform_spacing& get_spacing()", &Trigonometry::Transform::GetSpacing);
				VTransform->SetMethod<Trigonometry::Transform, Trigonometry::Transform::Spacing&, Trigonometry::Positioning>("transform_spacing& get_spacing(positioning)", &Trigonometry::Transform::GetSpacing);
				VTransform->SetMethod("transform@+ get_root() const", &Trigonometry::Transform::GetRoot);
				VTransform->SetMethod("transform@+ get_upper_root() const", &Trigonometry::Transform::GetUpperRoot);
				VTransform->SetMethod("transform@+ get_child(usize) const", &Trigonometry::Transform::GetChild);
				VTransform->SetMethod("usize get_childs_count() const", &Trigonometry::Transform::GetChildsCount);

				auto VNode = VM->SetPod<Trigonometry::Cosmos::Node>("cosmos_node");
				VNode->SetProperty<Trigonometry::Cosmos::Node>("bounding bounds", &Trigonometry::Cosmos::Node::Bounds);
				VNode->SetProperty<Trigonometry::Cosmos::Node>("usize parent", &Trigonometry::Cosmos::Node::Parent);
				VNode->SetProperty<Trigonometry::Cosmos::Node>("usize next", &Trigonometry::Cosmos::Node::Next);
				VNode->SetProperty<Trigonometry::Cosmos::Node>("usize left", &Trigonometry::Cosmos::Node::Left);
				VNode->SetProperty<Trigonometry::Cosmos::Node>("usize right", &Trigonometry::Cosmos::Node::Right);
				VNode->SetProperty<Trigonometry::Cosmos::Node>("uptr@ item", &Trigonometry::Cosmos::Node::Item);
				VNode->SetProperty<Trigonometry::Cosmos::Node>("int32 height", &Trigonometry::Cosmos::Node::Height);
				VNode->SetConstructor<Trigonometry::Cosmos::Node>("void f()");
				VNode->SetMethod("bool is_leaf() const", &Trigonometry::Cosmos::Node::IsLeaf);

				auto VCosmos = VM->SetStructTrivial<Trigonometry::Cosmos>("cosmos");
				VCosmos->SetFunctionDef("bool cosmos_query_overlaps_sync(const bounding &in)");
				VCosmos->SetFunctionDef("void cosmos_query_match_sync(uptr@)");
				VCosmos->SetConstructor<Trigonometry::Cosmos>("void f(usize = 16)");
				VCosmos->SetMethod("void reserve(usize)", &Trigonometry::Cosmos::Reserve);
				VCosmos->SetMethod("void clear()", &Trigonometry::Cosmos::Clear);
				VCosmos->SetMethod("void remove_item(uptr@)", &Trigonometry::Cosmos::RemoveItem);
				VCosmos->SetMethod("void insert_item(uptr@, const vector3 &in, const vector3 &in)", &Trigonometry::Cosmos::InsertItem);
				VCosmos->SetMethod("void update_item(uptr@, const vector3 &in, const vector3 &in, bool = false)", &Trigonometry::Cosmos::UpdateItem);
				VCosmos->SetMethod("const bounding& get_area(uptr@)", &Trigonometry::Cosmos::GetArea);
				VCosmos->SetMethod("usize get_nodes_count() const", &Trigonometry::Cosmos::GetNodesCount);
				VCosmos->SetMethod("usize get_height() const", &Trigonometry::Cosmos::GetHeight);
				VCosmos->SetMethod("usize get_max_balance() const", &Trigonometry::Cosmos::GetMaxBalance);
				VCosmos->SetMethod("usize get_root() const", &Trigonometry::Cosmos::GetRoot);
				VCosmos->SetMethod("const cosmos_node& get_root_node() const", &Trigonometry::Cosmos::GetRootNode);
				VCosmos->SetMethod("const cosmos_node& get_node(usize) const", &Trigonometry::Cosmos::GetNode);
				VCosmos->SetMethod("float get_volume_ratio() const", &Trigonometry::Cosmos::GetVolumeRatio);
				VCosmos->SetMethod("bool is_null(usize) const", &Trigonometry::Cosmos::IsNull);
				VCosmos->SetMethod("bool is_empty() const", &Trigonometry::Cosmos::Empty);
				VCosmos->SetMethodEx("void query_index(cosmos_query_overlaps_sync@, cosmos_query_match_sync@)", &CosmosQueryIndex);

				VM->BeginNamespace("geometric");
				VM->SetFunction("bool is_cube_in_frustum(const matrix4x4 &in, float)", &Trigonometry::Geometric::IsCubeInFrustum);
				VM->SetFunction("bool is_left_handed()", &Trigonometry::Geometric::IsLeftHanded);
				VM->SetFunction("bool has_sphere_intersected(const vector3 &in, float, const vector3 &in, float)", &Trigonometry::Geometric::HasSphereIntersected);
				VM->SetFunction("bool has_line_intersected(float, float, const vector3 &in, const vector3 &in, vector3 &out)", &Trigonometry::Geometric::HasLineIntersected);
				VM->SetFunction("bool has_line_intersected_cube(const vector3 &in, const vector3 &in, const vector3 &in, const vector3 &in)", &Trigonometry::Geometric::HasLineIntersectedCube);
				VM->SetFunction<bool(const Trigonometry::Vector3&, const Trigonometry::Vector3&, const Trigonometry::Vector3&, int)>("bool has_point_intersected_cube(const vector3 &in, const vector3 &in, const vector3 &in, int32)", &Trigonometry::Geometric::HasPointIntersectedCube);
				VM->SetFunction("bool has_point_intersected_rectangle(const vector3 &in, const vector3 &in, const vector3 &in)", &Trigonometry::Geometric::HasPointIntersectedRectangle);
				VM->SetFunction<bool(const Trigonometry::Vector3&, const Trigonometry::Vector3&, const Trigonometry::Vector3&)>("bool has_point_intersected_cube(const vector3 &in, const vector3 &in, const vector3 &in)", &Trigonometry::Geometric::HasPointIntersectedCube);
				VM->SetFunction("bool has_sb_intersected(transform@+, transform@+)", &Trigonometry::Geometric::HasSBIntersected);
				VM->SetFunction("bool has_obb_intersected(transform@+, transform@+)", &Trigonometry::Geometric::HasOBBIntersected);
				VM->SetFunction("bool has_aabb_intersected(transform@+, transform@+)", &Trigonometry::Geometric::HasAABBIntersected);
				VM->SetFunction("void matrix_rh_to_lh(matrix4x4 &out)", &Trigonometry::Geometric::MatrixRhToLh);
				VM->SetFunction("void set_left_handed(bool)", &Trigonometry::Geometric::SetLeftHanded);
				VM->SetFunction("ray create_cursor_ray(const vector3 &in, const vector2 &in, const vector2 &in, const matrix4x4 &in, const matrix4x4 &in)", &Trigonometry::Geometric::CreateCursorRay);
				VM->SetFunction<bool(const Trigonometry::Ray&, const Trigonometry::Vector3&, const Trigonometry::Vector3&, Trigonometry::Vector3*)>("bool cursor_ray_test(const ray &in, const vector3 &in, const vector3 &in, vector3 &out)", &Trigonometry::Geometric::CursorRayTest);
				VM->SetFunction<bool(const Trigonometry::Ray&, const Trigonometry::Matrix4x4&, Trigonometry::Vector3*)>("bool cursor_ray_test(const ray &in, const matrix4x4 &in, vector3 &out)", &Trigonometry::Geometric::CursorRayTest);
				VM->SetFunction("float fast_inv_sqrt(float)", &Trigonometry::Geometric::FastInvSqrt);
				VM->SetFunction("float fast_sqrt(float)", &Trigonometry::Geometric::FastSqrt);
				VM->SetFunction("float aabb_volume(const vector3 &in, const vector3 &in)", &Trigonometry::Geometric::AabbVolume);
				VM->SetFunction("float angle_distance(float, float)", &Trigonometry::Geometric::AngleDistance);
				VM->SetFunction("float angle_lerp(float, float, float)", &Trigonometry::Geometric::AngluarLerp);
				VM->EndNamespace();

				return true;
#else
				VI_ASSERT(false, "<trigonometry> is not loaded");
				return false;
#endif
			}
			bool HeavyRegistry::ImportActivity(VirtualMachine* VM) noexcept
			{
#ifdef VI_BINDINGS
				VI_ASSERT(VM != nullptr, "manager should be set");
				auto VAppState = VM->SetEnum("app_state");
				VAppState->SetValue("close_window", (int)Graphics::AppState::Close_Window);
				VAppState->SetValue("terminating", (int)Graphics::AppState::Terminating);
				VAppState->SetValue("low_memory", (int)Graphics::AppState::Low_Memory);
				VAppState->SetValue("enter_background_start", (int)Graphics::AppState::Enter_Background_Start);
				VAppState->SetValue("enter_background_end", (int)Graphics::AppState::Enter_Background_End);
				VAppState->SetValue("enter_foreground_start", (int)Graphics::AppState::Enter_Foreground_Start);
				VAppState->SetValue("enter_foreground_end", (int)Graphics::AppState::Enter_Foreground_End);

				auto VWindowState = VM->SetEnum("window_state");
				VWindowState->SetValue("show", (int)Graphics::WindowState::Show);
				VWindowState->SetValue("hide", (int)Graphics::WindowState::Hide);
				VWindowState->SetValue("expose", (int)Graphics::WindowState::Expose);
				VWindowState->SetValue("move", (int)Graphics::WindowState::Move);
				VWindowState->SetValue("resize", (int)Graphics::WindowState::Resize);
				VWindowState->SetValue("size_change", (int)Graphics::WindowState::Size_Change);
				VWindowState->SetValue("minimize", (int)Graphics::WindowState::Minimize);
				VWindowState->SetValue("maximize", (int)Graphics::WindowState::Maximize);
				VWindowState->SetValue("restore", (int)Graphics::WindowState::Restore);
				VWindowState->SetValue("enter", (int)Graphics::WindowState::Enter);
				VWindowState->SetValue("leave", (int)Graphics::WindowState::Leave);
				VWindowState->SetValue("focus", (int)Graphics::WindowState::Focus);
				VWindowState->SetValue("blur", (int)Graphics::WindowState::Blur);
				VWindowState->SetValue("close", (int)Graphics::WindowState::Close);

				auto VKeyCode = VM->SetEnum("key_code");
				VKeyCode->SetValue("a", (int)Graphics::KeyCode::A);
				VKeyCode->SetValue("b", (int)Graphics::KeyCode::B);
				VKeyCode->SetValue("c", (int)Graphics::KeyCode::C);
				VKeyCode->SetValue("d", (int)Graphics::KeyCode::D);
				VKeyCode->SetValue("e", (int)Graphics::KeyCode::E);
				VKeyCode->SetValue("f", (int)Graphics::KeyCode::F);
				VKeyCode->SetValue("g", (int)Graphics::KeyCode::G);
				VKeyCode->SetValue("h", (int)Graphics::KeyCode::H);
				VKeyCode->SetValue("i", (int)Graphics::KeyCode::I);
				VKeyCode->SetValue("j", (int)Graphics::KeyCode::J);
				VKeyCode->SetValue("k", (int)Graphics::KeyCode::K);
				VKeyCode->SetValue("l", (int)Graphics::KeyCode::L);
				VKeyCode->SetValue("m", (int)Graphics::KeyCode::M);
				VKeyCode->SetValue("n", (int)Graphics::KeyCode::N);
				VKeyCode->SetValue("o", (int)Graphics::KeyCode::O);
				VKeyCode->SetValue("p", (int)Graphics::KeyCode::P);
				VKeyCode->SetValue("q", (int)Graphics::KeyCode::Q);
				VKeyCode->SetValue("r", (int)Graphics::KeyCode::R);
				VKeyCode->SetValue("s", (int)Graphics::KeyCode::S);
				VKeyCode->SetValue("t", (int)Graphics::KeyCode::T);
				VKeyCode->SetValue("u", (int)Graphics::KeyCode::U);
				VKeyCode->SetValue("v", (int)Graphics::KeyCode::V);
				VKeyCode->SetValue("w", (int)Graphics::KeyCode::W);
				VKeyCode->SetValue("x", (int)Graphics::KeyCode::X);
				VKeyCode->SetValue("y", (int)Graphics::KeyCode::Y);
				VKeyCode->SetValue("z", (int)Graphics::KeyCode::Z);
				VKeyCode->SetValue("d1", (int)Graphics::KeyCode::D1);
				VKeyCode->SetValue("d2", (int)Graphics::KeyCode::D2);
				VKeyCode->SetValue("d3", (int)Graphics::KeyCode::D3);
				VKeyCode->SetValue("d4", (int)Graphics::KeyCode::D4);
				VKeyCode->SetValue("d5", (int)Graphics::KeyCode::D5);
				VKeyCode->SetValue("d6", (int)Graphics::KeyCode::D6);
				VKeyCode->SetValue("d7", (int)Graphics::KeyCode::D7);
				VKeyCode->SetValue("d8", (int)Graphics::KeyCode::D8);
				VKeyCode->SetValue("d9", (int)Graphics::KeyCode::D9);
				VKeyCode->SetValue("d0", (int)Graphics::KeyCode::D0);
				VKeyCode->SetValue("returns", (int)Graphics::KeyCode::Return);
				VKeyCode->SetValue("escape", (int)Graphics::KeyCode::Escape);
				VKeyCode->SetValue("backspace", (int)Graphics::KeyCode::Backspace);
				VKeyCode->SetValue("tab", (int)Graphics::KeyCode::Tab);
				VKeyCode->SetValue("space", (int)Graphics::KeyCode::Space);
				VKeyCode->SetValue("minus", (int)Graphics::KeyCode::Minus);
				VKeyCode->SetValue("equals", (int)Graphics::KeyCode::Equals);
				VKeyCode->SetValue("left_bracket", (int)Graphics::KeyCode::LeftBracket);
				VKeyCode->SetValue("right_bracket", (int)Graphics::KeyCode::RightBracket);
				VKeyCode->SetValue("backslash", (int)Graphics::KeyCode::Backslash);
				VKeyCode->SetValue("non_us_hash", (int)Graphics::KeyCode::NonUsHash);
				VKeyCode->SetValue("semicolon", (int)Graphics::KeyCode::Semicolon);
				VKeyCode->SetValue("apostrophe", (int)Graphics::KeyCode::Apostrophe);
				VKeyCode->SetValue("grave", (int)Graphics::KeyCode::Grave);
				VKeyCode->SetValue("comma", (int)Graphics::KeyCode::Comma);
				VKeyCode->SetValue("period", (int)Graphics::KeyCode::Period);
				VKeyCode->SetValue("slash", (int)Graphics::KeyCode::Slash);
				VKeyCode->SetValue("capslock", (int)Graphics::KeyCode::Capslock);
				VKeyCode->SetValue("f1", (int)Graphics::KeyCode::F1);
				VKeyCode->SetValue("f2", (int)Graphics::KeyCode::F2);
				VKeyCode->SetValue("f3", (int)Graphics::KeyCode::F3);
				VKeyCode->SetValue("f4", (int)Graphics::KeyCode::F4);
				VKeyCode->SetValue("f5", (int)Graphics::KeyCode::F5);
				VKeyCode->SetValue("f6", (int)Graphics::KeyCode::F6);
				VKeyCode->SetValue("f7", (int)Graphics::KeyCode::F7);
				VKeyCode->SetValue("f8", (int)Graphics::KeyCode::F8);
				VKeyCode->SetValue("f9", (int)Graphics::KeyCode::F9);
				VKeyCode->SetValue("f10", (int)Graphics::KeyCode::F10);
				VKeyCode->SetValue("f11", (int)Graphics::KeyCode::F11);
				VKeyCode->SetValue("f12", (int)Graphics::KeyCode::F12);
				VKeyCode->SetValue("print_screen", (int)Graphics::KeyCode::PrintScreen);
				VKeyCode->SetValue("scroll_lock", (int)Graphics::KeyCode::ScrollLock);
				VKeyCode->SetValue("pause", (int)Graphics::KeyCode::Pause);
				VKeyCode->SetValue("insert", (int)Graphics::KeyCode::Insert);
				VKeyCode->SetValue("home", (int)Graphics::KeyCode::Home);
				VKeyCode->SetValue("page_up", (int)Graphics::KeyCode::PageUp);
				VKeyCode->SetValue("delete", (int)Graphics::KeyCode::Delete);
				VKeyCode->SetValue("end", (int)Graphics::KeyCode::End);
				VKeyCode->SetValue("page_down", (int)Graphics::KeyCode::PageDown);
				VKeyCode->SetValue("right", (int)Graphics::KeyCode::Right);
				VKeyCode->SetValue("left", (int)Graphics::KeyCode::Left);
				VKeyCode->SetValue("down", (int)Graphics::KeyCode::Down);
				VKeyCode->SetValue("up", (int)Graphics::KeyCode::Up);
				VKeyCode->SetValue("num_lock_clear", (int)Graphics::KeyCode::NumLockClear);
				VKeyCode->SetValue("kp_divide", (int)Graphics::KeyCode::KpDivide);
				VKeyCode->SetValue("kp_multiply", (int)Graphics::KeyCode::KpMultiply);
				VKeyCode->SetValue("kp_minus", (int)Graphics::KeyCode::KpMinus);
				VKeyCode->SetValue("kp_plus", (int)Graphics::KeyCode::KpPlus);
				VKeyCode->SetValue("kp_enter", (int)Graphics::KeyCode::KpEnter);
				VKeyCode->SetValue("kp_1", (int)Graphics::KeyCode::Kp1);
				VKeyCode->SetValue("kp_2", (int)Graphics::KeyCode::Kp2);
				VKeyCode->SetValue("kp_3", (int)Graphics::KeyCode::Kp3);
				VKeyCode->SetValue("kp_4", (int)Graphics::KeyCode::Kp4);
				VKeyCode->SetValue("kp_5", (int)Graphics::KeyCode::Kp5);
				VKeyCode->SetValue("kp_6", (int)Graphics::KeyCode::Kp6);
				VKeyCode->SetValue("kp_7", (int)Graphics::KeyCode::Kp7);
				VKeyCode->SetValue("kp_8", (int)Graphics::KeyCode::Kp8);
				VKeyCode->SetValue("kp_9", (int)Graphics::KeyCode::Kp9);
				VKeyCode->SetValue("kp_0", (int)Graphics::KeyCode::Kp0);
				VKeyCode->SetValue("kp_period", (int)Graphics::KeyCode::KpPeriod);
				VKeyCode->SetValue("non_us_backslash", (int)Graphics::KeyCode::NonUsBackslash);
				VKeyCode->SetValue("app0", (int)Graphics::KeyCode::App0);
				VKeyCode->SetValue("power", (int)Graphics::KeyCode::Power);
				VKeyCode->SetValue("kp_equals", (int)Graphics::KeyCode::KpEquals);
				VKeyCode->SetValue("f13", (int)Graphics::KeyCode::F13);
				VKeyCode->SetValue("f14", (int)Graphics::KeyCode::F14);
				VKeyCode->SetValue("f15", (int)Graphics::KeyCode::F15);
				VKeyCode->SetValue("f16", (int)Graphics::KeyCode::F16);
				VKeyCode->SetValue("f17", (int)Graphics::KeyCode::F17);
				VKeyCode->SetValue("f18", (int)Graphics::KeyCode::F18);
				VKeyCode->SetValue("f19", (int)Graphics::KeyCode::F19);
				VKeyCode->SetValue("f20", (int)Graphics::KeyCode::F20);
				VKeyCode->SetValue("f21", (int)Graphics::KeyCode::F21);
				VKeyCode->SetValue("f22", (int)Graphics::KeyCode::F22);
				VKeyCode->SetValue("f23", (int)Graphics::KeyCode::F23);
				VKeyCode->SetValue("f24", (int)Graphics::KeyCode::F24);
				VKeyCode->SetValue("execute", (int)Graphics::KeyCode::Execute);
				VKeyCode->SetValue("help", (int)Graphics::KeyCode::Help);
				VKeyCode->SetValue("menu", (int)Graphics::KeyCode::Menu);
				VKeyCode->SetValue("select", (int)Graphics::KeyCode::Select);
				VKeyCode->SetValue("stop", (int)Graphics::KeyCode::Stop);
				VKeyCode->SetValue("again", (int)Graphics::KeyCode::Again);
				VKeyCode->SetValue("undo", (int)Graphics::KeyCode::Undo);
				VKeyCode->SetValue("cut", (int)Graphics::KeyCode::Cut);
				VKeyCode->SetValue("copy", (int)Graphics::KeyCode::Copy);
				VKeyCode->SetValue("paste", (int)Graphics::KeyCode::Paste);
				VKeyCode->SetValue("find", (int)Graphics::KeyCode::Find);
				VKeyCode->SetValue("mute", (int)Graphics::KeyCode::Mute);
				VKeyCode->SetValue("volume_up", (int)Graphics::KeyCode::VolumeUp);
				VKeyCode->SetValue("volume_down", (int)Graphics::KeyCode::VolumeDown);
				VKeyCode->SetValue("kp_comma", (int)Graphics::KeyCode::KpComma);
				VKeyCode->SetValue("kp_equals_as_400", (int)Graphics::KeyCode::KpEqualsAs400);
				VKeyCode->SetValue("international1", (int)Graphics::KeyCode::International1);
				VKeyCode->SetValue("international2", (int)Graphics::KeyCode::International2);
				VKeyCode->SetValue("international3", (int)Graphics::KeyCode::International3);
				VKeyCode->SetValue("international4", (int)Graphics::KeyCode::International4);
				VKeyCode->SetValue("international5", (int)Graphics::KeyCode::International5);
				VKeyCode->SetValue("international6", (int)Graphics::KeyCode::International6);
				VKeyCode->SetValue("international7", (int)Graphics::KeyCode::International7);
				VKeyCode->SetValue("international8", (int)Graphics::KeyCode::International8);
				VKeyCode->SetValue("international9", (int)Graphics::KeyCode::International9);
				VKeyCode->SetValue("lang1", (int)Graphics::KeyCode::Lang1);
				VKeyCode->SetValue("lang2", (int)Graphics::KeyCode::Lang2);
				VKeyCode->SetValue("lang3", (int)Graphics::KeyCode::Lang3);
				VKeyCode->SetValue("lang4", (int)Graphics::KeyCode::Lang4);
				VKeyCode->SetValue("lang5", (int)Graphics::KeyCode::Lang5);
				VKeyCode->SetValue("lang6", (int)Graphics::KeyCode::Lang6);
				VKeyCode->SetValue("lang7", (int)Graphics::KeyCode::Lang7);
				VKeyCode->SetValue("lang8", (int)Graphics::KeyCode::Lang8);
				VKeyCode->SetValue("lang9", (int)Graphics::KeyCode::Lang9);
				VKeyCode->SetValue("alterase", (int)Graphics::KeyCode::Alterase);
				VKeyCode->SetValue("sys_req", (int)Graphics::KeyCode::SysReq);
				VKeyCode->SetValue("cancel", (int)Graphics::KeyCode::Cancel);
				VKeyCode->SetValue("clear", (int)Graphics::KeyCode::Clear);
				VKeyCode->SetValue("prior", (int)Graphics::KeyCode::Prior);
				VKeyCode->SetValue("return2", (int)Graphics::KeyCode::Return2);
				VKeyCode->SetValue("separator", (int)Graphics::KeyCode::Separator);
				VKeyCode->SetValue("output", (int)Graphics::KeyCode::Output);
				VKeyCode->SetValue("operation", (int)Graphics::KeyCode::Operation);
				VKeyCode->SetValue("clear_again", (int)Graphics::KeyCode::ClearAgain);
				VKeyCode->SetValue("cr_select", (int)Graphics::KeyCode::CrSelect);
				VKeyCode->SetValue("ex_select", (int)Graphics::KeyCode::ExSelect);
				VKeyCode->SetValue("kp_00", (int)Graphics::KeyCode::Kp00);
				VKeyCode->SetValue("kp_000", (int)Graphics::KeyCode::Kp000);
				VKeyCode->SetValue("thousands_separator", (int)Graphics::KeyCode::ThousandsSeparator);
				VKeyCode->SetValue("decimals_separator", (int)Graphics::KeyCode::DecimalsSeparator);
				VKeyCode->SetValue("currency_unit", (int)Graphics::KeyCode::CurrencyUnit);
				VKeyCode->SetValue("currency_subunit", (int)Graphics::KeyCode::CurrencySubunit);
				VKeyCode->SetValue("kp_left_paren", (int)Graphics::KeyCode::KpLeftParen);
				VKeyCode->SetValue("kp_right_paren", (int)Graphics::KeyCode::KpRightParen);
				VKeyCode->SetValue("kp_left_brace", (int)Graphics::KeyCode::KpLeftBrace);
				VKeyCode->SetValue("kp_right_brace", (int)Graphics::KeyCode::KpRightBrace);
				VKeyCode->SetValue("kp_tab", (int)Graphics::KeyCode::KpTab);
				VKeyCode->SetValue("kp_backspace", (int)Graphics::KeyCode::KpBackspace);
				VKeyCode->SetValue("kp_a", (int)Graphics::KeyCode::KpA);
				VKeyCode->SetValue("kp_b", (int)Graphics::KeyCode::KpB);
				VKeyCode->SetValue("kp_c", (int)Graphics::KeyCode::KpC);
				VKeyCode->SetValue("kp_d", (int)Graphics::KeyCode::KpD);
				VKeyCode->SetValue("kp_e", (int)Graphics::KeyCode::KpE);
				VKeyCode->SetValue("kp_f", (int)Graphics::KeyCode::KpF);
				VKeyCode->SetValue("kp_xor", (int)Graphics::KeyCode::KpXOR);
				VKeyCode->SetValue("kp_power", (int)Graphics::KeyCode::KpPower);
				VKeyCode->SetValue("kp_percent", (int)Graphics::KeyCode::KpPercent);
				VKeyCode->SetValue("kp_less", (int)Graphics::KeyCode::KpLess);
				VKeyCode->SetValue("kp_greater", (int)Graphics::KeyCode::KpGreater);
				VKeyCode->SetValue("kp_ampersand", (int)Graphics::KeyCode::KpAmpersand);
				VKeyCode->SetValue("kp_dbl_ampersand", (int)Graphics::KeyCode::KpDBLAmpersand);
				VKeyCode->SetValue("kp_vertical_bar", (int)Graphics::KeyCode::KpVerticalBar);
				VKeyCode->SetValue("kp_dbl_vertical_bar", (int)Graphics::KeyCode::KpDBLVerticalBar);
				VKeyCode->SetValue("kp_colon", (int)Graphics::KeyCode::KpColon);
				VKeyCode->SetValue("kp_hash", (int)Graphics::KeyCode::KpHash);
				VKeyCode->SetValue("kp_space", (int)Graphics::KeyCode::KpSpace);
				VKeyCode->SetValue("kp_at", (int)Graphics::KeyCode::KpAt);
				VKeyCode->SetValue("kp_exclaim", (int)Graphics::KeyCode::KpExclaim);
				VKeyCode->SetValue("kp_mem_store", (int)Graphics::KeyCode::KpMemStore);
				VKeyCode->SetValue("kp_mem_recall", (int)Graphics::KeyCode::KpMemRecall);
				VKeyCode->SetValue("kp_mem_clear", (int)Graphics::KeyCode::KpMemClear);
				VKeyCode->SetValue("kp_mem_add", (int)Graphics::KeyCode::KpMemAdd);
				VKeyCode->SetValue("kp_mem_subtract", (int)Graphics::KeyCode::KpMemSubtract);
				VKeyCode->SetValue("kp_mem_multiply", (int)Graphics::KeyCode::KpMemMultiply);
				VKeyCode->SetValue("kp_mem_divide", (int)Graphics::KeyCode::KpMemDivide);
				VKeyCode->SetValue("kp_plus_minus", (int)Graphics::KeyCode::KpPlusMinus);
				VKeyCode->SetValue("kp_clear", (int)Graphics::KeyCode::KpClear);
				VKeyCode->SetValue("kp_clear_entry", (int)Graphics::KeyCode::KpClearEntry);
				VKeyCode->SetValue("kp_binary", (int)Graphics::KeyCode::KpBinary);
				VKeyCode->SetValue("kp_octal", (int)Graphics::KeyCode::KpOctal);
				VKeyCode->SetValue("kp_decimal", (int)Graphics::KeyCode::KpDecimal);
				VKeyCode->SetValue("kp_hexadecimal", (int)Graphics::KeyCode::KpHexadecimal);
				VKeyCode->SetValue("left_control", (int)Graphics::KeyCode::LeftControl);
				VKeyCode->SetValue("left_shift", (int)Graphics::KeyCode::LeftShift);
				VKeyCode->SetValue("left_alt", (int)Graphics::KeyCode::LeftAlt);
				VKeyCode->SetValue("left_gui", (int)Graphics::KeyCode::LeftGUI);
				VKeyCode->SetValue("right_control", (int)Graphics::KeyCode::RightControl);
				VKeyCode->SetValue("right_shift", (int)Graphics::KeyCode::RightShift);
				VKeyCode->SetValue("right_alt", (int)Graphics::KeyCode::RightAlt);
				VKeyCode->SetValue("right_gui", (int)Graphics::KeyCode::RightGUI);
				VKeyCode->SetValue("mode", (int)Graphics::KeyCode::Mode);
				VKeyCode->SetValue("audio_next", (int)Graphics::KeyCode::AudioNext);
				VKeyCode->SetValue("audio_prev", (int)Graphics::KeyCode::AudioPrev);
				VKeyCode->SetValue("audio_stop", (int)Graphics::KeyCode::AudioStop);
				VKeyCode->SetValue("audio_play", (int)Graphics::KeyCode::AudioPlay);
				VKeyCode->SetValue("audio_mute", (int)Graphics::KeyCode::AudioMute);
				VKeyCode->SetValue("media_select", (int)Graphics::KeyCode::MediaSelect);
				VKeyCode->SetValue("www", (int)Graphics::KeyCode::WWW);
				VKeyCode->SetValue("mail", (int)Graphics::KeyCode::Mail);
				VKeyCode->SetValue("calculator", (int)Graphics::KeyCode::Calculator);
				VKeyCode->SetValue("computer", (int)Graphics::KeyCode::Computer);
				VKeyCode->SetValue("ac_search", (int)Graphics::KeyCode::AcSearch);
				VKeyCode->SetValue("ac_home", (int)Graphics::KeyCode::AcHome);
				VKeyCode->SetValue("ac_back", (int)Graphics::KeyCode::AcBack);
				VKeyCode->SetValue("ac_forward", (int)Graphics::KeyCode::AcForward);
				VKeyCode->SetValue("ac_stop", (int)Graphics::KeyCode::AcStop);
				VKeyCode->SetValue("ac_refresh", (int)Graphics::KeyCode::AcRefresh);
				VKeyCode->SetValue("ac_bookmarks", (int)Graphics::KeyCode::AcBookmarks);
				VKeyCode->SetValue("brightness_down", (int)Graphics::KeyCode::BrightnessDown);
				VKeyCode->SetValue("brightness_up", (int)Graphics::KeyCode::BrightnessUp);
				VKeyCode->SetValue("display_switch", (int)Graphics::KeyCode::DisplaySwitch);
				VKeyCode->SetValue("kb_illum_toggle", (int)Graphics::KeyCode::KbIllumToggle);
				VKeyCode->SetValue("kb_illum_down", (int)Graphics::KeyCode::KbIllumDown);
				VKeyCode->SetValue("kb_illum_up", (int)Graphics::KeyCode::KbIllumUp);
				VKeyCode->SetValue("eject", (int)Graphics::KeyCode::Eject);
				VKeyCode->SetValue("sleep", (int)Graphics::KeyCode::Sleep);
				VKeyCode->SetValue("app1", (int)Graphics::KeyCode::App1);
				VKeyCode->SetValue("app2", (int)Graphics::KeyCode::App2);
				VKeyCode->SetValue("audio_rewind", (int)Graphics::KeyCode::AudioRewind);
				VKeyCode->SetValue("audio_fast_forward", (int)Graphics::KeyCode::AudioFastForward);
				VKeyCode->SetValue("cursor_left", (int)Graphics::KeyCode::CursorLeft);
				VKeyCode->SetValue("cursor_middle", (int)Graphics::KeyCode::CursorMiddle);
				VKeyCode->SetValue("cursor_right", (int)Graphics::KeyCode::CursorRight);
				VKeyCode->SetValue("cursor_x1", (int)Graphics::KeyCode::CursorX1);
				VKeyCode->SetValue("cursor_x2", (int)Graphics::KeyCode::CursorX2);
				VKeyCode->SetValue("none", (int)Graphics::KeyCode::None);

				auto VKeyMod = VM->SetEnum("key_mod");
				VKeyMod->SetValue("none", (int)Graphics::KeyMod::None);
				VKeyMod->SetValue("left_shift", (int)Graphics::KeyMod::LeftShift);
				VKeyMod->SetValue("right_shift", (int)Graphics::KeyMod::RightShift);
				VKeyMod->SetValue("left_control", (int)Graphics::KeyMod::LeftControl);
				VKeyMod->SetValue("right_control", (int)Graphics::KeyMod::RightControl);
				VKeyMod->SetValue("left_alt", (int)Graphics::KeyMod::LeftAlt);
				VKeyMod->SetValue("right_alt", (int)Graphics::KeyMod::RightAlt);
				VKeyMod->SetValue("left_gui", (int)Graphics::KeyMod::LeftGUI);
				VKeyMod->SetValue("right_gui", (int)Graphics::KeyMod::RightGUI);
				VKeyMod->SetValue("num", (int)Graphics::KeyMod::Num);
				VKeyMod->SetValue("caps", (int)Graphics::KeyMod::Caps);
				VKeyMod->SetValue("mode", (int)Graphics::KeyMod::Mode);
				VKeyMod->SetValue("reserved", (int)Graphics::KeyMod::Reserved);
				VKeyMod->SetValue("shift", (int)Graphics::KeyMod::Shift);
				VKeyMod->SetValue("control", (int)Graphics::KeyMod::Control);
				VKeyMod->SetValue("alt", (int)Graphics::KeyMod::Alt);
				VKeyMod->SetValue("gui", (int)Graphics::KeyMod::GUI);

				auto VAlertType = VM->SetEnum("alert_type");
				VAlertType->SetValue("none", (int)Graphics::AlertType::None);
				VAlertType->SetValue("error", (int)Graphics::AlertType::Error);
				VAlertType->SetValue("warning", (int)Graphics::AlertType::Warning);
				VAlertType->SetValue("info", (int)Graphics::AlertType::Info);

				auto VAlertConfirm = VM->SetEnum("alert_confirm");
				VAlertConfirm->SetValue("none", (int)Graphics::AlertConfirm::None);
				VAlertConfirm->SetValue("returns", (int)Graphics::AlertConfirm::Return);
				VAlertConfirm->SetValue("escape", (int)Graphics::AlertConfirm::Escape);

				auto VJoyStickHat = VM->SetEnum("joy_stick_hat");
				VJoyStickHat->SetValue("center", (int)Graphics::JoyStickHat::Center);
				VJoyStickHat->SetValue("up", (int)Graphics::JoyStickHat::Up);
				VJoyStickHat->SetValue("right", (int)Graphics::JoyStickHat::Right);
				VJoyStickHat->SetValue("down", (int)Graphics::JoyStickHat::Down);
				VJoyStickHat->SetValue("left", (int)Graphics::JoyStickHat::Left);
				VJoyStickHat->SetValue("right_up", (int)Graphics::JoyStickHat::Right_Up);
				VJoyStickHat->SetValue("right_down", (int)Graphics::JoyStickHat::Right_Down);
				VJoyStickHat->SetValue("left_up", (int)Graphics::JoyStickHat::Left_Up);
				VJoyStickHat->SetValue("left_down", (int)Graphics::JoyStickHat::Left_Down);

				auto VRenderBackend = VM->SetEnum("render_backend");
				VRenderBackend->SetValue("none", (int)Graphics::RenderBackend::None);
				VRenderBackend->SetValue("automatic", (int)Graphics::RenderBackend::Automatic);
				VRenderBackend->SetValue("d3d11", (int)Graphics::RenderBackend::D3D11);
				VRenderBackend->SetValue("ogl", (int)Graphics::RenderBackend::OGL);

				auto VDisplayCursor = VM->SetEnum("display_cursor");
				VDisplayCursor->SetValue("none", (int)Graphics::DisplayCursor::None);
				VDisplayCursor->SetValue("arrow", (int)Graphics::DisplayCursor::Arrow);
				VDisplayCursor->SetValue("text_input", (int)Graphics::DisplayCursor::TextInput);
				VDisplayCursor->SetValue("resize_all", (int)Graphics::DisplayCursor::ResizeAll);
				VDisplayCursor->SetValue("resize_ns", (int)Graphics::DisplayCursor::ResizeNS);
				VDisplayCursor->SetValue("resize_ew", (int)Graphics::DisplayCursor::ResizeEW);
				VDisplayCursor->SetValue("resize_nesw", (int)Graphics::DisplayCursor::ResizeNESW);
				VDisplayCursor->SetValue("resize_nwse", (int)Graphics::DisplayCursor::ResizeNWSE);
				VDisplayCursor->SetValue("hand", (int)Graphics::DisplayCursor::Hand);
				VDisplayCursor->SetValue("crosshair", (int)Graphics::DisplayCursor::Crosshair);
				VDisplayCursor->SetValue("wait", (int)Graphics::DisplayCursor::Wait);
				VDisplayCursor->SetValue("progress", (int)Graphics::DisplayCursor::Progress);
				VDisplayCursor->SetValue("no", (int)Graphics::DisplayCursor::No);

				auto VOrientationType = VM->SetEnum("orientation_type");
				VOrientationType->SetValue("unknown", (int)Graphics::OrientationType::Unknown);
				VOrientationType->SetValue("landscape", (int)Graphics::OrientationType::Landscape);
				VOrientationType->SetValue("landscape_flipped", (int)Graphics::OrientationType::LandscapeFlipped);
				VOrientationType->SetValue("portrait", (int)Graphics::OrientationType::Portrait);
				VOrientationType->SetValue("portrait_flipped", (int)Graphics::OrientationType::PortraitFlipped);

				auto VKeyMap = VM->SetPod<Graphics::KeyMap>("key_map");
				VKeyMap->SetProperty<Graphics::KeyMap>("key_code key", &Graphics::KeyMap::Key);
				VKeyMap->SetProperty<Graphics::KeyMap>("key_mod mod", &Graphics::KeyMap::Mod);
				VKeyMap->SetProperty<Graphics::KeyMap>("bool normal", &Graphics::KeyMap::Normal);
				VKeyMap->SetConstructor<Graphics::KeyMap>("void f()");
				VKeyMap->SetConstructor<Graphics::KeyMap, const Graphics::KeyCode&>("void f(const key_code &in)");
				VKeyMap->SetConstructor<Graphics::KeyMap, const Graphics::KeyMod&>("void f(const key_mod &in)");
				VKeyMap->SetConstructor<Graphics::KeyMap, const Graphics::KeyCode&, const Graphics::KeyMod&>("void f(const key_code &in, const key_mod &in)");

				auto VViewport = VM->SetPod<Graphics::Viewport>("viewport");
				VViewport->SetProperty<Graphics::Viewport>("float top_left_x", &Graphics::Viewport::TopLeftX);
				VViewport->SetProperty<Graphics::Viewport>("float top_left_y", &Graphics::Viewport::TopLeftY);
				VViewport->SetProperty<Graphics::Viewport>("float width", &Graphics::Viewport::Width);
				VViewport->SetProperty<Graphics::Viewport>("float height", &Graphics::Viewport::Height);
				VViewport->SetProperty<Graphics::Viewport>("float min_depth", &Graphics::Viewport::MinDepth);
				VViewport->SetProperty<Graphics::Viewport>("float max_depth", &Graphics::Viewport::MaxDepth);
				VViewport->SetConstructor<Graphics::Viewport>("void f()");

				auto VDisplayInfo = VM->SetPod<Graphics::DisplayInfo>("display_info");
				VDisplayInfo->SetProperty<Graphics::DisplayInfo>("string name", &Graphics::DisplayInfo::Name);
				VDisplayInfo->SetProperty<Graphics::DisplayInfo>("orientation_type orientation", &Graphics::DisplayInfo::Orientation);
				VDisplayInfo->SetProperty<Graphics::DisplayInfo>("float diagonal_dpi", &Graphics::DisplayInfo::DiagonalDPI);
				VDisplayInfo->SetProperty<Graphics::DisplayInfo>("float horizontal_dpi", &Graphics::DisplayInfo::HorizontalDPI);
				VDisplayInfo->SetProperty<Graphics::DisplayInfo>("float vertical_dpi", &Graphics::DisplayInfo::VerticalDPI);
				VDisplayInfo->SetProperty<Graphics::DisplayInfo>("uint32 pixel_format", &Graphics::DisplayInfo::PixelFormat);
				VDisplayInfo->SetProperty<Graphics::DisplayInfo>("uint32 physical_width", &Graphics::DisplayInfo::PhysicalWidth);
				VDisplayInfo->SetProperty<Graphics::DisplayInfo>("uint32 physical_height", &Graphics::DisplayInfo::PhysicalHeight);
				VDisplayInfo->SetProperty<Graphics::DisplayInfo>("uint32 refresh_rate", &Graphics::DisplayInfo::RefreshRate);
				VDisplayInfo->SetProperty<Graphics::DisplayInfo>("int32 width", &Graphics::DisplayInfo::Width);
				VDisplayInfo->SetProperty<Graphics::DisplayInfo>("int32 height", &Graphics::DisplayInfo::Height);
				VDisplayInfo->SetProperty<Graphics::DisplayInfo>("int32 x", &Graphics::DisplayInfo::X);
				VDisplayInfo->SetProperty<Graphics::DisplayInfo>("int32 y", &Graphics::DisplayInfo::Y);
				VDisplayInfo->SetConstructor<Graphics::DisplayInfo>("void f()");

				auto VActivity = VM->SetClass<Graphics::Activity>("activity", false);
				auto VAlert = VM->SetStructTrivial<Graphics::Alert>("activity_alert");
				VAlert->SetFunctionDef("void alert_sync(int)");
				VAlert->SetConstructor<Graphics::Alert, Graphics::Activity*>("void f(activity@+)");
				VAlert->SetMethod("void setup(alert_type, const string_view&in, const string_view&in)", &Graphics::Alert::Setup);
				VAlert->SetMethod("void button(alert_confirm, const string_view&in, int32)", &Graphics::Alert::Button);
				VAlert->SetMethodEx("void result(alert_sync@)", &AlertResult);

				auto VEventConsumers = VM->SetStructTrivial<Graphics::EventConsumers>("activity_event_consumers");
				VEventConsumers->SetConstructor<Graphics::EventConsumers>("void f()");
				VEventConsumers->SetMethod("void push(activity@+)", &Graphics::EventConsumers::Push);
				VEventConsumers->SetMethod("void pop(activity@+)", &Graphics::EventConsumers::Pop);
				VEventConsumers->SetMethod("activity@+ find(uint32) const", &Graphics::EventConsumers::Find);

				auto VSurface = VM->SetClass<Graphics::Surface>("activity_surface", false);
				VSurface->SetConstructor<Graphics::Surface>("activity_surface@ f()");
				VSurface->SetConstructor<Graphics::Surface, SDL_Surface*>("activity_surface@ f(uptr@)");
				VSurface->SetMethod("void set_handle(uptr@)", &Graphics::Surface::SetHandle);
				VSurface->SetMethod("void lock()", &Graphics::Surface::Lock);
				VSurface->SetMethod("void unlock()", &Graphics::Surface::Unlock);
				VSurface->SetMethod("int get_width()", &Graphics::Surface::GetWidth);
				VSurface->SetMethod("int get_height()", &Graphics::Surface::GetHeight);
				VSurface->SetMethod("int get_pitch()", &Graphics::Surface::GetPitch);
				VSurface->SetMethod("uptr@ get_pixels()", &Graphics::Surface::GetPixels);
				VSurface->SetMethod("uptr@ get_resource()", &Graphics::Surface::GetResource);

				auto VActivityDesc = VM->SetStructTrivial<Graphics::Activity::Desc>("activity_desc");
				VActivityDesc->SetProperty<Graphics::Activity::Desc>("string title", &Graphics::Activity::Desc::Title);
				VActivityDesc->SetProperty<Graphics::Activity::Desc>("uint32 inactive_sleep_ms", &Graphics::Activity::Desc::InactiveSleepMs);
				VActivityDesc->SetProperty<Graphics::Activity::Desc>("uint32 width", &Graphics::Activity::Desc::Width);
				VActivityDesc->SetProperty<Graphics::Activity::Desc>("uint32 height", &Graphics::Activity::Desc::Height);
				VActivityDesc->SetProperty<Graphics::Activity::Desc>("int32 x", &Graphics::Activity::Desc::X);
				VActivityDesc->SetProperty<Graphics::Activity::Desc>("int32 y", &Graphics::Activity::Desc::Y);
				VActivityDesc->SetProperty<Graphics::Activity::Desc>("bool fullscreen", &Graphics::Activity::Desc::Fullscreen);
				VActivityDesc->SetProperty<Graphics::Activity::Desc>("bool hidden", &Graphics::Activity::Desc::Hidden);
				VActivityDesc->SetProperty<Graphics::Activity::Desc>("bool borderless", &Graphics::Activity::Desc::Borderless);
				VActivityDesc->SetProperty<Graphics::Activity::Desc>("bool resizable", &Graphics::Activity::Desc::Resizable);
				VActivityDesc->SetProperty<Graphics::Activity::Desc>("bool minimized", &Graphics::Activity::Desc::Minimized);
				VActivityDesc->SetProperty<Graphics::Activity::Desc>("bool maximized", &Graphics::Activity::Desc::Maximized);
				VActivityDesc->SetProperty<Graphics::Activity::Desc>("bool centered", &Graphics::Activity::Desc::Centered);
				VActivityDesc->SetProperty<Graphics::Activity::Desc>("bool free_position", &Graphics::Activity::Desc::FreePosition);
				VActivityDesc->SetProperty<Graphics::Activity::Desc>("bool focused", &Graphics::Activity::Desc::Focused);
				VActivityDesc->SetProperty<Graphics::Activity::Desc>("bool render_even_if_inactive", &Graphics::Activity::Desc::RenderEvenIfInactive);
				VActivityDesc->SetProperty<Graphics::Activity::Desc>("bool gpu_as_renderer", &Graphics::Activity::Desc::GPUAsRenderer);
				VActivityDesc->SetProperty<Graphics::Activity::Desc>("bool high_dpi", &Graphics::Activity::Desc::HighDPI);
				VActivityDesc->SetConstructor<Graphics::Activity::Desc>("void f()");

				VActivity->SetProperty<Graphics::Activity>("activity_alert message", &Graphics::Activity::Message);
				VActivity->SetConstructor<Graphics::Activity, const Graphics::Activity::Desc&>("activity@ f(const activity_desc &in)");
				VActivity->SetFunctionDef("void app_state_change_sync(app_state)");
				VActivity->SetFunctionDef("void window_state_change_sync(window_state, int, int)");
				VActivity->SetFunctionDef("void key_state_sync(key_code, key_mod, int, int, bool)");
				VActivity->SetFunctionDef("void input_edit_sync(const string_view&in, int, int)");
				VActivity->SetFunctionDef("void input_sync(const string_view&in, int)");
				VActivity->SetFunctionDef("void cursor_move_sync(int, int, int, int)");
				VActivity->SetFunctionDef("void cursor_wheel_state_sync(int, int, bool)");
				VActivity->SetFunctionDef("void joy_stick_axis_move_sync(int, int, int)");
				VActivity->SetFunctionDef("void joy_stick_ball_move_sync(int, int, int, int)");
				VActivity->SetFunctionDef("void joy_stick_hat_move_sync(joy_stick_hat, int, int)");
				VActivity->SetFunctionDef("void joy_stick_key_state_sync(int, int, bool)");
				VActivity->SetFunctionDef("void joy_stick_state_sync(int, bool)");
				VActivity->SetFunctionDef("void controller_axis_move_sync(int, int, int)");
				VActivity->SetFunctionDef("void controller_key_state_sync(int, int, bool)");
				VActivity->SetFunctionDef("void controller_state_sync(int, int)");
				VActivity->SetFunctionDef("void touch_move_sync(int, int, float, float, float, float, float)");
				VActivity->SetFunctionDef("void touch_state_sync(int, int, float, float, float, float, float, bool)");
				VActivity->SetFunctionDef("void gesture_state_sync(int, int, int, float, float, float, bool)");
				VActivity->SetFunctionDef("void multi_gesture_state_sync(int, int, float, float, float, float)");
				VActivity->SetFunctionDef("void drop_file_sync(const string_view&in)");
				VActivity->SetFunctionDef("void drop_text_sync(const string_view&in)");
				VActivity->SetMethodEx("void set_app_state_change(app_state_change_sync@)", &ActivitySetAppStateChange);
				VActivity->SetMethodEx("void set_window_state_change(window_state_change_sync@)", &ActivitySetWindowStateChange);
				VActivity->SetMethodEx("void set_key_state(key_state_sync@)", &ActivitySetKeyState);
				VActivity->SetMethodEx("void set_input_edit(input_edit_sync@)", &ActivitySetInputEdit);
				VActivity->SetMethodEx("void set_input(input_sync@)", &ActivitySetInput);
				VActivity->SetMethodEx("void set_cursor_move(cursor_move_sync@)", &ActivitySetCursorMove);
				VActivity->SetMethodEx("void set_cursor_wheel_state(cursor_wheel_state_sync@)", &ActivitySetCursorWheelState);
				VActivity->SetMethodEx("void set_joy_stick_axis_move(joy_stick_axis_move_sync@)", &ActivitySetJoyStickAxisMove);
				VActivity->SetMethodEx("void set_joy_stick_ball_move(joy_stick_ball_move_sync@)", &ActivitySetJoyStickBallMove);
				VActivity->SetMethodEx("void set_joy_stick_hat_move(joy_stick_hat_move_sync@)", &ActivitySetJoyStickHatMove);
				VActivity->SetMethodEx("void set_joy_stickKeyState(joy_stick_key_state_sync@)", &ActivitySetJoyStickKeyState);
				VActivity->SetMethodEx("void set_joy_stickState(joy_stick_state_sync@)", &ActivitySetJoyStickState);
				VActivity->SetMethodEx("void set_controller_axis_move(controller_axis_move_sync@)", &ActivitySetControllerAxisMove);
				VActivity->SetMethodEx("void set_controller_key_state(controller_key_state_sync@)", &ActivitySetControllerKeyState);
				VActivity->SetMethodEx("void set_controller_state(controller_state_sync@)", &ActivitySetControllerState);
				VActivity->SetMethodEx("void set_touch_move(touch_move_sync@)", &ActivitySetTouchMove);
				VActivity->SetMethodEx("void set_touch_state(touch_state_sync@)", &ActivitySetTouchState);
				VActivity->SetMethodEx("void set_gesture_state(gesture_state_sync@)", &ActivitySetGestureState);
				VActivity->SetMethodEx("void set_multi_gesture_state(multi_gesture_state_sync@)", &ActivitySetMultiGestureState);
				VActivity->SetMethodEx("void set_drop_file(drop_file_sync@)", &ActivitySetDropFile);
				VActivity->SetMethodEx("void set_drop_text(drop_text_sync@)", &ActivitySetDropText);
				VActivity->SetMethod("void set_clipboard_text(const string_view&in)", &Graphics::Activity::SetClipboardText);
				VActivity->SetMethod<Graphics::Activity, void, const Trigonometry::Vector2&>("void set_cursor_position(const vector2 &in)", &Graphics::Activity::SetCursorPosition);
				VActivity->SetMethod<Graphics::Activity, void, float, float>("void set_cursor_position(float, float)", &Graphics::Activity::SetCursorPosition);
				VActivity->SetMethod<Graphics::Activity, void, const Trigonometry::Vector2&>("void set_global_cursor_position(const vector2 &in)", &Graphics::Activity::SetGlobalCursorPosition);
				VActivity->SetMethod<Graphics::Activity, void, float, float>("void set_global_cursor_position(float, float)", &Graphics::Activity::SetGlobalCursorPosition);
				VActivity->SetMethod("void set_key(key_code, bool)", &Graphics::Activity::SetKey);
				VActivity->SetMethod("void set_cursor(display_cursor)", &Graphics::Activity::SetCursor);
				VActivity->SetMethod("void set_cursor_visibility(bool)", &Graphics::Activity::SetCursorVisibility);
				VActivity->SetMethod("void set_grabbing(bool)", &Graphics::Activity::SetGrabbing);
				VActivity->SetMethod("void set_fullscreen(bool)", &Graphics::Activity::SetFullscreen);
				VActivity->SetMethod("void set_borderless(bool)", &Graphics::Activity::SetBorderless);
				VActivity->SetMethod("void set_icon(activity_surface@+)", &Graphics::Activity::SetIcon);
				VActivity->SetMethod("void set_title(const string_view&in)", &Graphics::Activity::SetTitle);
				VActivity->SetMethod("void set_screen_keyboard(bool)", &Graphics::Activity::SetScreenKeyboard);
				VActivity->SetMethod("void apply_configuration(render_backend)", &Graphics::Activity::ApplyConfiguration);
				VActivity->SetMethod("void hide()", &Graphics::Activity::Hide);
				VActivity->SetMethod("void show()", &Graphics::Activity::Show);
				VActivity->SetMethod("void maximize()", &Graphics::Activity::Maximize);
				VActivity->SetMethod("void minimize()", &Graphics::Activity::Minimize);
				VActivity->SetMethod("void focus()", &Graphics::Activity::Focus);
				VActivity->SetMethod("void move(int, int)", &Graphics::Activity::Move);
				VActivity->SetMethod("void resize(int, int)", &Graphics::Activity::Resize);
				VActivity->SetMethod("bool capture_key_map(key_map &out)", &Graphics::Activity::CaptureKeyMap);
				VActivity->SetMethod("bool dispatch(uint64 = 0, bool = true)", &Graphics::Activity::Dispatch);
				VActivity->SetMethod("bool is_fullscreen() const", &Graphics::Activity::IsFullscreen);
				VActivity->SetMethod("bool is_any_key_down() const", &Graphics::Activity::IsAnyKeyDown);
				VActivity->SetMethod("bool is_key_down(const key_map &in) const", &Graphics::Activity::IsKeyDown);
				VActivity->SetMethod("bool is_key_up(const key_map &in) const", &Graphics::Activity::IsKeyUp);
				VActivity->SetMethod("bool is_key_down_hit(const key_map &in) const", &Graphics::Activity::IsKeyDownHit);
				VActivity->SetMethod("bool is_key_up_hit(const key_map &in) const", &Graphics::Activity::IsKeyUpHit);
				VActivity->SetMethod("bool is_screen_keyboard_enabled() const", &Graphics::Activity::IsScreenKeyboardEnabled);
				VActivity->SetMethod("uint32 get_x() const", &Graphics::Activity::GetX);
				VActivity->SetMethod("uint32 get_y() const", &Graphics::Activity::GetY);
				VActivity->SetMethod("uint32 get_width() const", &Graphics::Activity::GetWidth);
				VActivity->SetMethod("uint32 get_height() const", &Graphics::Activity::GetHeight);
				VActivity->SetMethod("uint32 get_id() const", &Graphics::Activity::GetId);
				VActivity->SetMethod("float get_aspect_ratio() const", &Graphics::Activity::GetAspectRatio);
				VActivity->SetMethod("key_mod get_key_mod_state() const", &Graphics::Activity::GetKeyModState);
				VActivity->SetMethod("viewport get_viewport() const", &Graphics::Activity::GetViewport);
				VActivity->SetMethod("vector2 get_offset() const", &Graphics::Activity::GetOffset);
				VActivity->SetMethod("vector2 get_size() const", &Graphics::Activity::GetSize);
				VActivity->SetMethod("vector2 get_client_size() const", &Graphics::Activity::GetClientSize);
				VActivity->SetMethod("vector2 get_global_cursor_position() const", &Graphics::Activity::GetGlobalCursorPosition);
				VActivity->SetMethod<Graphics::Activity, Trigonometry::Vector2>("vector2 get_cursor_position() const", &Graphics::Activity::GetCursorPosition);
				VActivity->SetMethod<Graphics::Activity, Trigonometry::Vector2, float, float>("vector2 get_cursor_position(float, float) const", &Graphics::Activity::GetCursorPosition);
				VActivity->SetMethod<Graphics::Activity, Trigonometry::Vector2, const Trigonometry::Vector2&>("vector2 get_cursor_position(const vector2 &in) const", &Graphics::Activity::GetCursorPosition);
				VActivity->SetMethod("string get_clipboard_text() const", &Graphics::Activity::GetClipboardText);
				VActivity->SetMethod("string get_error() const", &Graphics::Activity::GetError);
				VActivity->SetMethod("activity_desc& get_options()", &Graphics::Activity::GetOptions);
				VActivity->SetMethodStatic("bool multi_dispatch(const activity_event_consumers&in, uint64 = 0, bool = true)", &Graphics::Activity::MultiDispatch);

				VM->BeginNamespace("alerts");
				VM->SetFunction("bool text(const string_view&in, const string_view&in, const string_view&in, string &out)", &Graphics::Alerts::Text);
				VM->SetFunction("bool password(const string_view&in, const string_view&in, string &out)", &Graphics::Alerts::Password);
				VM->SetFunction("bool save(const string_view&in, const string_view&in, const string_view&in, const string_view&in, string &out)", &Graphics::Alerts::Save);
				VM->SetFunction("bool open(const string_view&in, const string_view&in, const string_view&in, const string_view&in, bool, string &out)", &Graphics::Alerts::Open);
				VM->SetFunction("bool folder(const string_view&in, const string_view&in, string &out)", &Graphics::Alerts::Folder);
				VM->SetFunction("bool color(const string_view&in, const string_view&in, string &out)", &Graphics::Alerts::Color);
				VM->EndNamespace();

				VM->BeginNamespace("video");
				VM->SetFunction("uint32 get_display_count()", &Graphics::Video::GetDisplayCount);
				VM->SetFunction("bool get_display_info(uint32, display_info&out)", &Graphics::Video::GetDisplayInfo);
				VM->SetFunction("string get_key_code_as_string(key_code)", &Graphics::Video::GetKeyCodeAsString);
				VM->SetFunction("string get_key_mod_as_string(key_mod)", &Graphics::Video::GetKeyModAsString);
				VM->EndNamespace();

				return true;
#else
				VI_ASSERT(false, "<activity> is not loaded");
				return false;
#endif
			}
			bool HeavyRegistry::ImportPhysics(VirtualMachine* VM) noexcept
			{
#ifdef VI_BINDINGS
				VI_ASSERT(VM != nullptr, "manager should be set");
				auto VSimulator = VM->SetClass<Physics::Simulator>("physics_simulator", false);

				auto VShape = VM->SetEnum("physics_shape");
				VShape->SetValue("box", (int)Physics::Shape::Box);
				VShape->SetValue("triangle", (int)Physics::Shape::Triangle);
				VShape->SetValue("tetrahedral", (int)Physics::Shape::Tetrahedral);
				VShape->SetValue("convex_triangle_mesh", (int)Physics::Shape::Convex_Triangle_Mesh);
				VShape->SetValue("convex_hull", (int)Physics::Shape::Convex_Hull);
				VShape->SetValue("convex_point_cloud", (int)Physics::Shape::Convex_Point_Cloud);
				VShape->SetValue("convex_polyhedral", (int)Physics::Shape::Convex_Polyhedral);
				VShape->SetValue("implicit_convex_start", (int)Physics::Shape::Implicit_Convex_Start);
				VShape->SetValue("sphere", (int)Physics::Shape::Sphere);
				VShape->SetValue("multi_sphere", (int)Physics::Shape::Multi_Sphere);
				VShape->SetValue("capsule", (int)Physics::Shape::Capsule);
				VShape->SetValue("cone", (int)Physics::Shape::Cone);
				VShape->SetValue("convex", (int)Physics::Shape::Convex);
				VShape->SetValue("cylinder", (int)Physics::Shape::Cylinder);
				VShape->SetValue("uniform_scaling", (int)Physics::Shape::Uniform_Scaling);
				VShape->SetValue("minkowski_sum", (int)Physics::Shape::Minkowski_Sum);
				VShape->SetValue("minkowski_difference", (int)Physics::Shape::Minkowski_Difference);
				VShape->SetValue("box_2d", (int)Physics::Shape::Box_2D);
				VShape->SetValue("convex_2d", (int)Physics::Shape::Convex_2D);
				VShape->SetValue("custom_convex", (int)Physics::Shape::Custom_Convex);
				VShape->SetValue("concaves_start", (int)Physics::Shape::Concaves_Start);
				VShape->SetValue("triangle_mesh", (int)Physics::Shape::Triangle_Mesh);
				VShape->SetValue("triangle_mesh_scaled", (int)Physics::Shape::Triangle_Mesh_Scaled);
				VShape->SetValue("fast_concave_mesh", (int)Physics::Shape::Fast_Concave_Mesh);
				VShape->SetValue("terrain", (int)Physics::Shape::Terrain);
				VShape->SetValue("triangle_mesh_multimaterial", (int)Physics::Shape::Triangle_Mesh_Multimaterial);
				VShape->SetValue("empty", (int)Physics::Shape::Empty);
				VShape->SetValue("static_plane", (int)Physics::Shape::Static_Plane);
				VShape->SetValue("custom_concave", (int)Physics::Shape::Custom_Concave);
				VShape->SetValue("concaves_end", (int)Physics::Shape::Concaves_End);
				VShape->SetValue("compound", (int)Physics::Shape::Compound);
				VShape->SetValue("softbody", (int)Physics::Shape::Softbody);
				VShape->SetValue("hf_fluid", (int)Physics::Shape::HF_Fluid);
				VShape->SetValue("hf_fluid_bouyant_convex", (int)Physics::Shape::HF_Fluid_Bouyant_Convex);
				VShape->SetValue("invalid", (int)Physics::Shape::Invalid);

				auto VMotionState = VM->SetEnum("physics_motion_state");
				VMotionState->SetValue("active", (int)Physics::MotionState::Active);
				VMotionState->SetValue("island_sleeping", (int)Physics::MotionState::Island_Sleeping);
				VMotionState->SetValue("deactivation_needed", (int)Physics::MotionState::Deactivation_Needed);
				VMotionState->SetValue("disable_deactivation", (int)Physics::MotionState::Disable_Deactivation);
				VMotionState->SetValue("disable_simulation", (int)Physics::MotionState::Disable_Simulation);

				auto VSoftFeature = VM->SetEnum("physics_soft_feature");
				VSoftFeature->SetValue("none", (int)Physics::SoftFeature::None);
				VSoftFeature->SetValue("node", (int)Physics::SoftFeature::Node);
				VSoftFeature->SetValue("link", (int)Physics::SoftFeature::Link);
				VSoftFeature->SetValue("face", (int)Physics::SoftFeature::Face);
				VSoftFeature->SetValue("tetra", (int)Physics::SoftFeature::Tetra);

				auto VSoftAeroModel = VM->SetEnum("physics_soft_aero_model");
				VSoftAeroModel->SetValue("vpoint", (int)Physics::SoftAeroModel::VPoint);
				VSoftAeroModel->SetValue("vtwo_sided", (int)Physics::SoftAeroModel::VTwoSided);
				VSoftAeroModel->SetValue("vtwo_sided_lift_drag", (int)Physics::SoftAeroModel::VTwoSidedLiftDrag);
				VSoftAeroModel->SetValue("vone_sided", (int)Physics::SoftAeroModel::VOneSided);
				VSoftAeroModel->SetValue("ftwo_sided", (int)Physics::SoftAeroModel::FTwoSided);
				VSoftAeroModel->SetValue("ftwo_sided_lift_drag", (int)Physics::SoftAeroModel::FTwoSidedLiftDrag);
				VSoftAeroModel->SetValue("fone_sided", (int)Physics::SoftAeroModel::FOneSided);

				auto VSoftCollision = VM->SetEnum("physics_soft_collision");
				VSoftCollision->SetValue("rvs_mask", (int)Physics::SoftCollision::RVS_Mask);
				VSoftCollision->SetValue("sdf_rs", (int)Physics::SoftCollision::SDF_RS);
				VSoftCollision->SetValue("cl_rs", (int)Physics::SoftCollision::CL_RS);
				VSoftCollision->SetValue("sdf_rd", (int)Physics::SoftCollision::SDF_RD);
				VSoftCollision->SetValue("sdf_rdf", (int)Physics::SoftCollision::SDF_RDF);
				VSoftCollision->SetValue("svs_mask", (int)Physics::SoftCollision::SVS_Mask);
				VSoftCollision->SetValue("vf_ss", (int)Physics::SoftCollision::VF_SS);
				VSoftCollision->SetValue("cl_ss", (int)Physics::SoftCollision::CL_SS);
				VSoftCollision->SetValue("cl_self", (int)Physics::SoftCollision::CL_Self);
				VSoftCollision->SetValue("vf_dd", (int)Physics::SoftCollision::VF_DD);
				VSoftCollision->SetValue("default_t", (int)Physics::SoftCollision::Default);

				auto VRotator = VM->SetEnum("physics_rotator");
				VRotator->SetValue("xyz", (int)Trigonometry::Rotator::XYZ);
				VRotator->SetValue("xzy", (int)Trigonometry::Rotator::XZY);
				VRotator->SetValue("yxz", (int)Trigonometry::Rotator::YXZ);
				VRotator->SetValue("yzx", (int)Trigonometry::Rotator::YZX);
				VRotator->SetValue("zxy", (int)Trigonometry::Rotator::ZXY);
				VRotator->SetValue("zyx", (int)Trigonometry::Rotator::ZYX);

				auto VHullShape = VM->SetClass<Physics::HullShape>("physics_hull_shape", false);
				VHullShape->SetMethod("uptr@ get_shape()", &Physics::HullShape::GetShape);
				VHullShape->SetMethodEx("array<vertex>@ get_vertices()", &HullShapeGetVertices);
				VHullShape->SetMethodEx("array<int>@ get_indices()", &HullShapeGetIndices);

				auto VRigidBodyDesc = VM->SetPod<Physics::RigidBody::Desc>("physics_rigidbody_desc");
				VRigidBodyDesc->SetProperty<Physics::RigidBody::Desc>("uptr@ shape", &Physics::RigidBody::Desc::Shape);
				VRigidBodyDesc->SetProperty<Physics::RigidBody::Desc>("float anticipation", &Physics::RigidBody::Desc::Anticipation);
				VRigidBodyDesc->SetProperty<Physics::RigidBody::Desc>("float mass", &Physics::RigidBody::Desc::Mass);
				VRigidBodyDesc->SetProperty<Physics::RigidBody::Desc>("vector3 position", &Physics::RigidBody::Desc::Position);
				VRigidBodyDesc->SetProperty<Physics::RigidBody::Desc>("vector3 rotation", &Physics::RigidBody::Desc::Rotation);
				VRigidBodyDesc->SetProperty<Physics::RigidBody::Desc>("vector3 scale", &Physics::RigidBody::Desc::Scale);
				VRigidBodyDesc->SetConstructor<Physics::RigidBody::Desc>("void f()");

				auto VRigidBody = VM->SetClass<Physics::RigidBody>("physics_rigidbody", false);
				VRigidBody->SetMethod("physics_rigidbody@ copy()", &Physics::RigidBody::Copy);
				VRigidBody->SetMethod<Physics::RigidBody, void, const Trigonometry::Vector3&>("void push(const vector3 &in)", &Physics::RigidBody::Push);
				VRigidBody->SetMethod<Physics::RigidBody, void, const Trigonometry::Vector3&, const Trigonometry::Vector3&>("void push(const vector3 &in, const vector3 &in)", &Physics::RigidBody::Push);
				VRigidBody->SetMethod<Physics::RigidBody, void, const Trigonometry::Vector3&, const Trigonometry::Vector3&, const Trigonometry::Vector3&>("void push(const vector3 &in, const vector3 &in, const vector3 &in)", &Physics::RigidBody::Push);
				VRigidBody->SetMethod<Physics::RigidBody, void, const Trigonometry::Vector3&>("void push_kinematic(const vector3 &in)", &Physics::RigidBody::PushKinematic);
				VRigidBody->SetMethod<Physics::RigidBody, void, const Trigonometry::Vector3&, const Trigonometry::Vector3&>("void push_kinematic(const vector3 &in, const vector3 &in)", &Physics::RigidBody::PushKinematic);
				VRigidBody->SetMethod("void synchronize(transform@+, bool)", &Physics::RigidBody::Synchronize);
				VRigidBody->SetMethod("void set_collision_flags(usize)", &Physics::RigidBody::SetCollisionFlags);
				VRigidBody->SetMethod("void set_activity(bool)", &Physics::RigidBody::SetActivity);
				VRigidBody->SetMethod("void set_as_ghost()", &Physics::RigidBody::SetAsGhost);
				VRigidBody->SetMethod("void set_as_normal()", &Physics::RigidBody::SetAsNormal);
				VRigidBody->SetMethod("void set_self_pointer()", &Physics::RigidBody::SetSelfPointer);
				VRigidBody->SetMethod("void set_world_transform(uptr@)", &Physics::RigidBody::SetWorldTransform);
				VRigidBody->SetMethod("void set_collision_shape(uptr@, transform@+)", &Physics::RigidBody::SetCollisionShape);
				VRigidBody->SetMethod("void set_mass(float)", &Physics::RigidBody::SetMass);
				VRigidBody->SetMethod("void set_activation_state(physics_motion_state)", &Physics::RigidBody::SetActivationState);
				VRigidBody->SetMethod("void set_angular_damping(float)", &Physics::RigidBody::SetAngularDamping);
				VRigidBody->SetMethod("void set_angular_sleeping_threshold(float)", &Physics::RigidBody::SetAngularSleepingThreshold);
				VRigidBody->SetMethod("void set_spinning_friction(float)", &Physics::RigidBody::SetSpinningFriction);
				VRigidBody->SetMethod("void set_contact_stiffness(float)", &Physics::RigidBody::SetContactStiffness);
				VRigidBody->SetMethod("void set_contact_damping(float)", &Physics::RigidBody::SetContactDamping);
				VRigidBody->SetMethod("void set_friction(float)", &Physics::RigidBody::SetFriction);
				VRigidBody->SetMethod("void set_restitution(float)", &Physics::RigidBody::SetRestitution);
				VRigidBody->SetMethod("void set_hit_fraction(float)", &Physics::RigidBody::SetHitFraction);
				VRigidBody->SetMethod("void set_linear_damping(float)", &Physics::RigidBody::SetLinearDamping);
				VRigidBody->SetMethod("void set_linear_sleeping_threshold(float)", &Physics::RigidBody::SetLinearSleepingThreshold);
				VRigidBody->SetMethod("void set_ccd_motion_threshold(float)", &Physics::RigidBody::SetCcdMotionThreshold);
				VRigidBody->SetMethod("void set_ccd_swept_sphere_radius(float)", &Physics::RigidBody::SetCcdSweptSphereRadius);
				VRigidBody->SetMethod("void set_contact_processing_threshold(float)", &Physics::RigidBody::SetContactProcessingThreshold);
				VRigidBody->SetMethod("void set_deactivation_time(float)", &Physics::RigidBody::SetDeactivationTime);
				VRigidBody->SetMethod("void set_rolling_friction(float)", &Physics::RigidBody::SetRollingFriction);
				VRigidBody->SetMethod("void set_angular_factor(const vector3 &in)", &Physics::RigidBody::SetAngularFactor);
				VRigidBody->SetMethod("void set_anisotropic_friction(const vector3 &in)", &Physics::RigidBody::SetAnisotropicFriction);
				VRigidBody->SetMethod("void set_gravity(const vector3 &in)", &Physics::RigidBody::SetGravity);
				VRigidBody->SetMethod("void set_linear_factor(const vector3 &in)", &Physics::RigidBody::SetLinearFactor);
				VRigidBody->SetMethod("void set_linear_velocity(const vector3 &in)", &Physics::RigidBody::SetLinearVelocity);
				VRigidBody->SetMethod("void set_angular_velocity(const vector3 &in)", &Physics::RigidBody::SetAngularVelocity);
				VRigidBody->SetMethod("physics_motion_state get_activation_state() const", &Physics::RigidBody::GetActivationState);
				VRigidBody->SetMethod("physics_shape get_collision_shape_type() const", &Physics::RigidBody::GetCollisionShapeType);
				VRigidBody->SetMethod("vector3 get_angular_factor() const", &Physics::RigidBody::GetAngularFactor);
				VRigidBody->SetMethod("vector3 get_anisotropic_friction() const", &Physics::RigidBody::GetAnisotropicFriction);
				VRigidBody->SetMethod("vector3 get_Gravity() const", &Physics::RigidBody::GetGravity);
				VRigidBody->SetMethod("vector3 get_linear_factor() const", &Physics::RigidBody::GetLinearFactor);
				VRigidBody->SetMethod("vector3 get_linear_velocity() const", &Physics::RigidBody::GetLinearVelocity);
				VRigidBody->SetMethod("vector3 get_angular_velocity() const", &Physics::RigidBody::GetAngularVelocity);
				VRigidBody->SetMethod("vector3 get_scale() const", &Physics::RigidBody::GetScale);
				VRigidBody->SetMethod("vector3 get_position() const", &Physics::RigidBody::GetPosition);
				VRigidBody->SetMethod("vector3 get_rotation() const", &Physics::RigidBody::GetRotation);
				VRigidBody->SetMethod("uptr@ get_world_transform() const", &Physics::RigidBody::GetWorldTransform);
				VRigidBody->SetMethod("uptr@ get_collision_shape() const", &Physics::RigidBody::GetCollisionShape);
				VRigidBody->SetMethod("uptr@ get() const", &Physics::RigidBody::Get);
				VRigidBody->SetMethod("bool is_active() const", &Physics::RigidBody::IsActive);
				VRigidBody->SetMethod("bool is_static() const", &Physics::RigidBody::IsStatic);
				VRigidBody->SetMethod("bool is_ghost() const", &Physics::RigidBody::IsGhost);
				VRigidBody->SetMethod("bool Is_colliding() const", &Physics::RigidBody::IsColliding);
				VRigidBody->SetMethod("float get_spinning_friction() const", &Physics::RigidBody::GetSpinningFriction);
				VRigidBody->SetMethod("float get_contact_stiffness() const", &Physics::RigidBody::GetContactStiffness);
				VRigidBody->SetMethod("float get_contact_damping() const", &Physics::RigidBody::GetContactDamping);
				VRigidBody->SetMethod("float get_angular_damping() const", &Physics::RigidBody::GetAngularDamping);
				VRigidBody->SetMethod("float get_angular_sleeping_threshold() const", &Physics::RigidBody::GetAngularSleepingThreshold);
				VRigidBody->SetMethod("float get_friction() const", &Physics::RigidBody::GetFriction);
				VRigidBody->SetMethod("float get_restitution() const", &Physics::RigidBody::GetRestitution);
				VRigidBody->SetMethod("float get_hit_fraction() const", &Physics::RigidBody::GetHitFraction);
				VRigidBody->SetMethod("float get_linear_damping() const", &Physics::RigidBody::GetLinearDamping);
				VRigidBody->SetMethod("float get_linear_sleeping_threshold() const", &Physics::RigidBody::GetLinearSleepingThreshold);
				VRigidBody->SetMethod("float get_ccd_motion_threshold() const", &Physics::RigidBody::GetCcdMotionThreshold);
				VRigidBody->SetMethod("float get_ccd_swept_sphere_radius() const", &Physics::RigidBody::GetCcdSweptSphereRadius);
				VRigidBody->SetMethod("float get_contact_processing_threshold() const", &Physics::RigidBody::GetContactProcessingThreshold);
				VRigidBody->SetMethod("float get_deactivation_time() const", &Physics::RigidBody::GetDeactivationTime);
				VRigidBody->SetMethod("float get_rolling_friction() const", &Physics::RigidBody::GetRollingFriction);
				VRigidBody->SetMethod("float get_mass() const", &Physics::RigidBody::GetMass);
				VRigidBody->SetMethod("usize get_collision_flags() const", &Physics::RigidBody::GetCollisionFlags);
				VRigidBody->SetMethod("physics_rigidbody_desc& get_initial_state()", &Physics::RigidBody::GetInitialState);
				VRigidBody->SetMethod("physics_simulator@+ get_simulator() const", &Physics::RigidBody::GetSimulator);

				auto VSoftBodySConvex = VM->SetStruct<Physics::SoftBody::Desc::CV::SConvex>("physics_softbody_desc_cv_sconvex");
				VSoftBodySConvex->SetProperty<Physics::SoftBody::Desc::CV::SConvex>("physics_hull_shape@ hull", &Physics::SoftBody::Desc::CV::SConvex::Hull);
				VSoftBodySConvex->SetProperty<Physics::SoftBody::Desc::CV::SConvex>("bool enabled", &Physics::SoftBody::Desc::CV::SConvex::Enabled);
				VSoftBodySConvex->SetConstructor<Physics::SoftBody::Desc::CV::SConvex>("void f()");
				VSoftBodySConvex->SetOperatorCopyStatic(&SoftBodySConvexCopy);
				VSoftBodySConvex->SetDestructorEx("void f()", &SoftBodySConvexDestructor);

				auto VSoftBodySRope = VM->SetPod<Physics::SoftBody::Desc::CV::SRope>("physics_softbody_desc_cv_srope");
				VSoftBodySRope->SetProperty<Physics::SoftBody::Desc::CV::SRope>("bool start_fixed", &Physics::SoftBody::Desc::CV::SRope::StartFixed);
				VSoftBodySRope->SetProperty<Physics::SoftBody::Desc::CV::SRope>("bool end_fixed", &Physics::SoftBody::Desc::CV::SRope::EndFixed);
				VSoftBodySRope->SetProperty<Physics::SoftBody::Desc::CV::SRope>("bool enabled", &Physics::SoftBody::Desc::CV::SRope::Enabled);
				VSoftBodySRope->SetProperty<Physics::SoftBody::Desc::CV::SRope>("int count", &Physics::SoftBody::Desc::CV::SRope::Count);
				VSoftBodySRope->SetProperty<Physics::SoftBody::Desc::CV::SRope>("vector3 start", &Physics::SoftBody::Desc::CV::SRope::Start);
				VSoftBodySRope->SetProperty<Physics::SoftBody::Desc::CV::SRope>("vector3 end", &Physics::SoftBody::Desc::CV::SRope::End);
				VSoftBodySRope->SetConstructor<Physics::SoftBody::Desc::CV::SRope>("void f()");

				auto VSoftBodySPatch = VM->SetPod<Physics::SoftBody::Desc::CV::SPatch>("physics_softbody_desc_cv_spatch");
				VSoftBodySPatch->SetProperty<Physics::SoftBody::Desc::CV::SPatch>("bool generate_diagonals", &Physics::SoftBody::Desc::CV::SPatch::GenerateDiagonals);
				VSoftBodySPatch->SetProperty<Physics::SoftBody::Desc::CV::SPatch>("bool corner00_fixed", &Physics::SoftBody::Desc::CV::SPatch::Corner00Fixed);
				VSoftBodySPatch->SetProperty<Physics::SoftBody::Desc::CV::SPatch>("bool corner10_fixed", &Physics::SoftBody::Desc::CV::SPatch::Corner10Fixed);
				VSoftBodySPatch->SetProperty<Physics::SoftBody::Desc::CV::SPatch>("bool corner01_fixed", &Physics::SoftBody::Desc::CV::SPatch::Corner01Fixed);
				VSoftBodySPatch->SetProperty<Physics::SoftBody::Desc::CV::SPatch>("bool corner11_fixed", &Physics::SoftBody::Desc::CV::SPatch::Corner11Fixed);
				VSoftBodySPatch->SetProperty<Physics::SoftBody::Desc::CV::SPatch>("bool enabled", &Physics::SoftBody::Desc::CV::SPatch::Enabled);
				VSoftBodySPatch->SetProperty<Physics::SoftBody::Desc::CV::SPatch>("int count_x", &Physics::SoftBody::Desc::CV::SPatch::CountX);
				VSoftBodySPatch->SetProperty<Physics::SoftBody::Desc::CV::SPatch>("int count_y", &Physics::SoftBody::Desc::CV::SPatch::CountY);
				VSoftBodySPatch->SetProperty<Physics::SoftBody::Desc::CV::SPatch>("vector3 corner00", &Physics::SoftBody::Desc::CV::SPatch::Corner00);
				VSoftBodySPatch->SetProperty<Physics::SoftBody::Desc::CV::SPatch>("vector3 corner10", &Physics::SoftBody::Desc::CV::SPatch::Corner10);
				VSoftBodySPatch->SetProperty<Physics::SoftBody::Desc::CV::SPatch>("vector3 corner01", &Physics::SoftBody::Desc::CV::SPatch::Corner01);
				VSoftBodySPatch->SetProperty<Physics::SoftBody::Desc::CV::SPatch>("vector3 corner11", &Physics::SoftBody::Desc::CV::SPatch::Corner11);
				VSoftBodySPatch->SetConstructor<Physics::SoftBody::Desc::CV::SPatch>("void f()");

				auto VSoftBodySEllipsoid = VM->SetPod<Physics::SoftBody::Desc::CV::SEllipsoid>("physics_softbody_desc_cv_sellipsoid");
				VSoftBodySEllipsoid->SetProperty<Physics::SoftBody::Desc::CV::SEllipsoid>("vector3 center", &Physics::SoftBody::Desc::CV::SEllipsoid::Center);
				VSoftBodySEllipsoid->SetProperty<Physics::SoftBody::Desc::CV::SEllipsoid>("vector3 radius", &Physics::SoftBody::Desc::CV::SEllipsoid::Radius);
				VSoftBodySEllipsoid->SetProperty<Physics::SoftBody::Desc::CV::SEllipsoid>("int count", &Physics::SoftBody::Desc::CV::SEllipsoid::Count);
				VSoftBodySEllipsoid->SetProperty<Physics::SoftBody::Desc::CV::SEllipsoid>("bool enabled", &Physics::SoftBody::Desc::CV::SEllipsoid::Enabled);
				VSoftBodySEllipsoid->SetConstructor<Physics::SoftBody::Desc::CV::SEllipsoid>("void f()");

				auto VSoftBodyCV = VM->SetPod<Physics::SoftBody::Desc::CV>("physics_softbody_desc_cv");
				VSoftBodyCV->SetProperty<Physics::SoftBody::Desc::CV>("physics_softbody_desc_cv_sconvex convex", &Physics::SoftBody::Desc::CV::Convex);
				VSoftBodyCV->SetProperty<Physics::SoftBody::Desc::CV>("physics_softbody_desc_cv_srope rope", &Physics::SoftBody::Desc::CV::Rope);
				VSoftBodyCV->SetProperty<Physics::SoftBody::Desc::CV>("physics_softbody_desc_cv_spatch patch", &Physics::SoftBody::Desc::CV::Patch);
				VSoftBodyCV->SetProperty<Physics::SoftBody::Desc::CV>("physics_softbody_desc_cv_sellipsoid ellipsoid", &Physics::SoftBody::Desc::CV::Ellipsoid);
				VSoftBodyCV->SetConstructor<Physics::SoftBody::Desc::CV>("void f()");

				auto VSoftBodySConfig = VM->SetPod<Physics::SoftBody::Desc::SConfig>("physics_softbody_desc_config");
				VSoftBodySConfig->SetProperty<Physics::SoftBody::Desc::SConfig>("physics_soft_aero_model aero_model", &Physics::SoftBody::Desc::SConfig::AeroModel);
				VSoftBodySConfig->SetProperty<Physics::SoftBody::Desc::SConfig>("float vcf", &Physics::SoftBody::Desc::SConfig::VCF);
				VSoftBodySConfig->SetProperty<Physics::SoftBody::Desc::SConfig>("float dp", &Physics::SoftBody::Desc::SConfig::DP);
				VSoftBodySConfig->SetProperty<Physics::SoftBody::Desc::SConfig>("float dg", &Physics::SoftBody::Desc::SConfig::DG);
				VSoftBodySConfig->SetProperty<Physics::SoftBody::Desc::SConfig>("float lf", &Physics::SoftBody::Desc::SConfig::LF);
				VSoftBodySConfig->SetProperty<Physics::SoftBody::Desc::SConfig>("float pr", &Physics::SoftBody::Desc::SConfig::PR);
				VSoftBodySConfig->SetProperty<Physics::SoftBody::Desc::SConfig>("float vc", &Physics::SoftBody::Desc::SConfig::VC);
				VSoftBodySConfig->SetProperty<Physics::SoftBody::Desc::SConfig>("float df", &Physics::SoftBody::Desc::SConfig::DF);
				VSoftBodySConfig->SetProperty<Physics::SoftBody::Desc::SConfig>("float mt", &Physics::SoftBody::Desc::SConfig::MT);
				VSoftBodySConfig->SetProperty<Physics::SoftBody::Desc::SConfig>("float chr", &Physics::SoftBody::Desc::SConfig::CHR);
				VSoftBodySConfig->SetProperty<Physics::SoftBody::Desc::SConfig>("float khr", &Physics::SoftBody::Desc::SConfig::KHR);
				VSoftBodySConfig->SetProperty<Physics::SoftBody::Desc::SConfig>("float shr", &Physics::SoftBody::Desc::SConfig::SHR);
				VSoftBodySConfig->SetProperty<Physics::SoftBody::Desc::SConfig>("float ahr", &Physics::SoftBody::Desc::SConfig::AHR);
				VSoftBodySConfig->SetProperty<Physics::SoftBody::Desc::SConfig>("float srhr_cl", &Physics::SoftBody::Desc::SConfig::SRHR_CL);
				VSoftBodySConfig->SetProperty<Physics::SoftBody::Desc::SConfig>("float skhr_cl", &Physics::SoftBody::Desc::SConfig::SKHR_CL);
				VSoftBodySConfig->SetProperty<Physics::SoftBody::Desc::SConfig>("float sshr_cl", &Physics::SoftBody::Desc::SConfig::SSHR_CL);
				VSoftBodySConfig->SetProperty<Physics::SoftBody::Desc::SConfig>("float sr_splt_cl", &Physics::SoftBody::Desc::SConfig::SR_SPLT_CL);
				VSoftBodySConfig->SetProperty<Physics::SoftBody::Desc::SConfig>("float sk_splt_cl", &Physics::SoftBody::Desc::SConfig::SK_SPLT_CL);
				VSoftBodySConfig->SetProperty<Physics::SoftBody::Desc::SConfig>("float ss_splt_cl", &Physics::SoftBody::Desc::SConfig::SS_SPLT_CL);
				VSoftBodySConfig->SetProperty<Physics::SoftBody::Desc::SConfig>("float max_volume", &Physics::SoftBody::Desc::SConfig::MaxVolume);
				VSoftBodySConfig->SetProperty<Physics::SoftBody::Desc::SConfig>("float time_scale", &Physics::SoftBody::Desc::SConfig::TimeScale);
				VSoftBodySConfig->SetProperty<Physics::SoftBody::Desc::SConfig>("float drag", &Physics::SoftBody::Desc::SConfig::Drag);
				VSoftBodySConfig->SetProperty<Physics::SoftBody::Desc::SConfig>("float max_stress", &Physics::SoftBody::Desc::SConfig::MaxStress);
				VSoftBodySConfig->SetProperty<Physics::SoftBody::Desc::SConfig>("int clusters", &Physics::SoftBody::Desc::SConfig::Clusters);
				VSoftBodySConfig->SetProperty<Physics::SoftBody::Desc::SConfig>("int constraints", &Physics::SoftBody::Desc::SConfig::Constraints);
				VSoftBodySConfig->SetProperty<Physics::SoftBody::Desc::SConfig>("int viterations", &Physics::SoftBody::Desc::SConfig::VIterations);
				VSoftBodySConfig->SetProperty<Physics::SoftBody::Desc::SConfig>("int Piterations", &Physics::SoftBody::Desc::SConfig::PIterations);
				VSoftBodySConfig->SetProperty<Physics::SoftBody::Desc::SConfig>("int diterations", &Physics::SoftBody::Desc::SConfig::DIterations);
				VSoftBodySConfig->SetProperty<Physics::SoftBody::Desc::SConfig>("int citerations", &Physics::SoftBody::Desc::SConfig::CIterations);
				VSoftBodySConfig->SetProperty<Physics::SoftBody::Desc::SConfig>("int collisions", &Physics::SoftBody::Desc::SConfig::Collisions);
				VSoftBodySConfig->SetConstructor<Physics::SoftBody::Desc::SConfig>("void f()");

				auto VSoftBodyDesc = VM->SetPod<Physics::SoftBody::Desc>("physics_softbody_desc");
				VSoftBodyDesc->SetProperty<Physics::SoftBody::Desc>("physics_softbody_desc_cv shape", &Physics::SoftBody::Desc::Shape);
				VSoftBodyDesc->SetProperty<Physics::SoftBody::Desc>("physics_softbody_desc_config feature", &Physics::SoftBody::Desc::Config);
				VSoftBodyDesc->SetProperty<Physics::SoftBody::Desc>("float anticipation", &Physics::SoftBody::SoftBody::Desc::Anticipation);
				VSoftBodyDesc->SetProperty<Physics::SoftBody::Desc>("vector3 position", &Physics::SoftBody::SoftBody::Desc::Position);
				VSoftBodyDesc->SetProperty<Physics::SoftBody::Desc>("vector3 rotation", &Physics::SoftBody::SoftBody::Desc::Rotation);
				VSoftBodyDesc->SetProperty<Physics::SoftBody::Desc>("vector3 scale", &Physics::SoftBody::SoftBody::Desc::Scale);
				VSoftBodyDesc->SetConstructor<Physics::SoftBody::Desc>("void f()");

				auto VSoftBody = VM->SetClass<Physics::SoftBody>("physics_softbody", false);
				auto VSoftBodyRayCast = VM->SetPod<Physics::SoftBody::RayCast>("physics_softbody_raycast");
				VSoftBodyRayCast->SetProperty<Physics::SoftBody::RayCast>("physics_softbody@ body", &Physics::SoftBody::RayCast::Body);
				VSoftBodyRayCast->SetProperty<Physics::SoftBody::RayCast>("physics_soft_feature feature", &Physics::SoftBody::RayCast::Feature);
				VSoftBodyRayCast->SetProperty<Physics::SoftBody::RayCast>("float fraction", &Physics::SoftBody::RayCast::Fraction);
				VSoftBodyRayCast->SetProperty<Physics::SoftBody::RayCast>("int32 index", &Physics::SoftBody::RayCast::Index);
				VSoftBodyRayCast->SetConstructor<Physics::SoftBody::RayCast>("void f()");

				VSoftBody->SetMethod("physics_softbody@ copy()", &Physics::SoftBody::Copy);
				VSoftBody->SetMethod("void activate(bool)", &Physics::SoftBody::Activate);
				VSoftBody->SetMethod("void synchronize(transform@+, bool)", &Physics::SoftBody::Synchronize);
				VSoftBody->SetMethodEx("array<int32>@ GetIndices() const", &SoftBodyGetIndices);
				VSoftBody->SetMethodEx("array<vertex>@ GetVertices() const", &SoftBodyGetVertices);
				VSoftBody->SetMethod("void get_bounding_box(vector3 &out, vector3 &out) const", &Physics::SoftBody::GetBoundingBox);
				VSoftBody->SetMethod("void set_contact_stiffness_and_damping(float, float)", &Physics::SoftBody::SetContactStiffnessAndDamping);
				VSoftBody->SetMethod<Physics::SoftBody, void, int, Physics::RigidBody*, bool, float>("void add_anchor(int32, physics_rigidbody@+, bool = false, float = 1)", &Physics::SoftBody::AddAnchor);
				VSoftBody->SetMethod<Physics::SoftBody, void, int, Physics::RigidBody*, const Trigonometry::Vector3&, bool, float>("void add_anchor(int32, physics_rigidbody@+, const vector3 &in, bool = false, float = 1)", &Physics::SoftBody::AddAnchor);
				VSoftBody->SetMethod<Physics::SoftBody, void, const Trigonometry::Vector3&>("void add_force(const vector3 &in)", &Physics::SoftBody::AddForce);
				VSoftBody->SetMethod<Physics::SoftBody, void, const Trigonometry::Vector3&, int>("void add_force(const vector3 &in, int)", &Physics::SoftBody::AddForce);
				VSoftBody->SetMethod("void add_aero_force_to_node(const vector3 &in, int)", &Physics::SoftBody::AddAeroForceToNode);
				VSoftBody->SetMethod("void add_aero_force_to_face(const vector3 &in, int)", &Physics::SoftBody::AddAeroForceToFace);
				VSoftBody->SetMethod<Physics::SoftBody, void, const Trigonometry::Vector3&>("void add_velocity(const vector3 &in)", &Physics::SoftBody::AddVelocity);
				VSoftBody->SetMethod<Physics::SoftBody, void, const Trigonometry::Vector3&, int>("void add_velocity(const vector3 &in, int)", &Physics::SoftBody::AddVelocity);
				VSoftBody->SetMethod("void set_selocity(const vector3 &in)", &Physics::SoftBody::SetVelocity);
				VSoftBody->SetMethod("void set_mass(int, float)", &Physics::SoftBody::SetMass);
				VSoftBody->SetMethod("void set_total_mass(float, bool = false)", &Physics::SoftBody::SetTotalMass);
				VSoftBody->SetMethod("void set_total_density(float)", &Physics::SoftBody::SetTotalDensity);
				VSoftBody->SetMethod("void set_volume_mass(float)", &Physics::SoftBody::SetVolumeMass);
				VSoftBody->SetMethod("void set_volume_density(float)", &Physics::SoftBody::SetVolumeDensity);
				VSoftBody->SetMethod("void translate(const vector3 &in)", &Physics::SoftBody::Translate);
				VSoftBody->SetMethod("void rotate(const vector3 &in)", &Physics::SoftBody::Rotate);
				VSoftBody->SetMethod("void scale(const vector3 &in)", &Physics::SoftBody::Scale);
				VSoftBody->SetMethod("void set_rest_length_scale(float)", &Physics::SoftBody::SetRestLengthScale);
				VSoftBody->SetMethod("void set_pose(bool, bool)", &Physics::SoftBody::SetPose);
				VSoftBody->SetMethod("float get_mass(int) const", &Physics::SoftBody::GetMass);
				VSoftBody->SetMethod("float get_total_mass() const", &Physics::SoftBody::GetTotalMass);
				VSoftBody->SetMethod("float get_rest_length_scale() const", &Physics::SoftBody::GetRestLengthScale);
				VSoftBody->SetMethod("float get_volume() const", &Physics::SoftBody::GetVolume);
				VSoftBody->SetMethod("int generate_bending_constraints(int)", &Physics::SoftBody::GenerateBendingConstraints);
				VSoftBody->SetMethod("void randomize_constraints()", &Physics::SoftBody::RandomizeConstraints);
				VSoftBody->SetMethod("bool cut_link(int, int, float)", &Physics::SoftBody::CutLink);
				VSoftBody->SetMethod("bool ray_test(const vector3 &in, const vector3 &in, physics_softbody_raycast &out)", &Physics::SoftBody::RayTest);
				VSoftBody->SetMethod("void set_wind_velocity(const vector3 &in)", &Physics::SoftBody::SetWindVelocity);
				VSoftBody->SetMethod("vector3 get_wind_velocity() const", &Physics::SoftBody::GetWindVelocity);
				VSoftBody->SetMethod("void get_aabb(vector3 &out, vector3 &out) const", &Physics::SoftBody::GetAabb);
				VSoftBody->SetMethod("void set_spinning_friction(float)", &Physics::SoftBody::SetSpinningFriction);
				VSoftBody->SetMethod("vector3 get_linear_velocity() const", &Physics::SoftBody::GetLinearVelocity);
				VSoftBody->SetMethod("vector3 get_angular_velocity() const", &Physics::SoftBody::GetAngularVelocity);
				VSoftBody->SetMethod("vector3 get_center_position() const", &Physics::SoftBody::GetCenterPosition);
				VSoftBody->SetMethod("void set_activity(bool)", &Physics::SoftBody::SetActivity);
				VSoftBody->SetMethod("void set_as_ghost()", &Physics::SoftBody::SetAsGhost);
				VSoftBody->SetMethod("void set_as_normal()", &Physics::SoftBody::SetAsNormal);
				VSoftBody->SetMethod("void set_self_pointer()", &Physics::SoftBody::SetSelfPointer);
				VSoftBody->SetMethod("void set_world_transform(uptr@)", &Physics::SoftBody::SetWorldTransform);
				VSoftBody->SetMethod("void set_activation_state(physics_motion_state)", &Physics::SoftBody::SetActivationState);
				VSoftBody->SetMethod("void set_contact_stiffness(float)", &Physics::SoftBody::SetContactStiffness);
				VSoftBody->SetMethod("void set_contact_damping(float)", &Physics::SoftBody::SetContactDamping);
				VSoftBody->SetMethod("void set_friction(float)", &Physics::SoftBody::SetFriction);
				VSoftBody->SetMethod("void set_restitution(float)", &Physics::SoftBody::SetRestitution);
				VSoftBody->SetMethod("void set_hit_fraction(float)", &Physics::SoftBody::SetHitFraction);
				VSoftBody->SetMethod("void set_ccd_motion_threshold(float)", &Physics::SoftBody::SetCcdMotionThreshold);
				VSoftBody->SetMethod("void set_ccd_swept_sphere_radius(float)", &Physics::SoftBody::SetCcdSweptSphereRadius);
				VSoftBody->SetMethod("void set_contact_processing_threshold(float)", &Physics::SoftBody::SetContactProcessingThreshold);
				VSoftBody->SetMethod("void set_reactivation_time(float)", &Physics::SoftBody::SetDeactivationTime);
				VSoftBody->SetMethod("void set_rolling_friction(float)", &Physics::SoftBody::SetRollingFriction);
				VSoftBody->SetMethod("void set_anisotropic_friction(const vector3 &in)", &Physics::SoftBody::SetAnisotropicFriction);
				VSoftBody->SetMethod("void set_config(const physics_softbody_desc_config &in)", &Physics::SoftBody::SetConfig);
				VSoftBody->SetMethod("physics_shape get_collision_shape_type() const", &Physics::SoftBody::GetCollisionShapeType);
				VSoftBody->SetMethod("physics_motion_state get_activation_state() const", &Physics::SoftBody::GetActivationState);
				VSoftBody->SetMethod("vector3 get_anisotropic_friction() const", &Physics::SoftBody::GetAnisotropicFriction);
				VSoftBody->SetMethod("vector3 get_scale() const", &Physics::SoftBody::GetScale);
				VSoftBody->SetMethod("vector3 get_position() const", &Physics::SoftBody::GetPosition);
				VSoftBody->SetMethod("vector3 get_rotation() const", &Physics::SoftBody::GetRotation);
				VSoftBody->SetMethod("uptr@ get_world_transform() const", &Physics::SoftBody::GetWorldTransform);
				VSoftBody->SetMethod("uptr@ get() const", &Physics::SoftBody::Get);
				VSoftBody->SetMethod("bool is_active() const", &Physics::SoftBody::IsActive);
				VSoftBody->SetMethod("bool is_static() const", &Physics::SoftBody::IsStatic);
				VSoftBody->SetMethod("bool is_ghost() const", &Physics::SoftBody::IsGhost);
				VSoftBody->SetMethod("bool is_colliding() const", &Physics::SoftBody::IsColliding);
				VSoftBody->SetMethod("float get_spinning_friction() const", &Physics::SoftBody::GetSpinningFriction);
				VSoftBody->SetMethod("float get_contact_stiffness() const", &Physics::SoftBody::GetContactStiffness);
				VSoftBody->SetMethod("float get_contact_damping() const", &Physics::SoftBody::GetContactDamping);
				VSoftBody->SetMethod("float get_friction() const", &Physics::SoftBody::GetFriction);
				VSoftBody->SetMethod("float get_restitution() const", &Physics::SoftBody::GetRestitution);
				VSoftBody->SetMethod("float get_hit_fraction() const", &Physics::SoftBody::GetHitFraction);
				VSoftBody->SetMethod("float get_ccd_motion_threshold() const", &Physics::SoftBody::GetCcdMotionThreshold);
				VSoftBody->SetMethod("float get_ccd_swept_sphere_radius() const", &Physics::SoftBody::GetCcdSweptSphereRadius);
				VSoftBody->SetMethod("float get_contact_processing_threshold() const", &Physics::SoftBody::GetContactProcessingThreshold);
				VSoftBody->SetMethod("float get_deactivation_time() const", &Physics::SoftBody::GetDeactivationTime);
				VSoftBody->SetMethod("float get_rolling_friction() const", &Physics::SoftBody::GetRollingFriction);
				VSoftBody->SetMethod("usize get_collision_flags() const", &Physics::SoftBody::GetCollisionFlags);
				VSoftBody->SetMethod("usize get_vertices_count() const", &Physics::SoftBody::GetVerticesCount);
				VSoftBody->SetMethod("physics_softbody_desc& get_initial_state()", &Physics::SoftBody::GetInitialState);
				VSoftBody->SetMethod("physics_simulator@+ get_simulator() const", &Physics::SoftBody::GetSimulator);

				auto VConstraint = VM->SetClass<Physics::Constraint>("physics_constraint", false);
				VConstraint->SetMethod("physics_constraint@ copy() const", &Physics::Constraint::Copy);
				VConstraint->SetMethod("physics_simulator@+ get_simulator() const", &Physics::Constraint::GetSimulator);
				VConstraint->SetMethod("uptr@ get() const", &Physics::Constraint::Get);
				VConstraint->SetMethod("uptr@ get_first() const", &Physics::Constraint::GetFirst);
				VConstraint->SetMethod("uptr@ get_second() const", &Physics::Constraint::GetSecond);
				VConstraint->SetMethod("bool has_collisions() const", &Physics::Constraint::HasCollisions);
				VConstraint->SetMethod("bool is_enabled() const", &Physics::Constraint::IsEnabled);
				VConstraint->SetMethod("bool is_active() const", &Physics::Constraint::IsActive);
				VConstraint->SetMethod("void set_breaking_impulse_threshold(float)", &Physics::Constraint::SetBreakingImpulseThreshold);
				VConstraint->SetMethod("void set_enabled(bool)", &Physics::Constraint::SetEnabled);
				VConstraint->SetMethod("float get_breaking_impulse_threshold() const", &Physics::Constraint::GetBreakingImpulseThreshold);

				auto VPConstraintDesc = VM->SetPod<Physics::PConstraint::Desc>("physics_pconstraint_desc");
				VPConstraintDesc->SetProperty<Physics::PConstraint::Desc>("physics_rigidbody@ target_a", &Physics::PConstraint::Desc::TargetA);
				VPConstraintDesc->SetProperty<Physics::PConstraint::Desc>("physics_rigidbody@ target_b", &Physics::PConstraint::Desc::TargetB);
				VPConstraintDesc->SetProperty<Physics::PConstraint::Desc>("vector3 pivot_a", &Physics::PConstraint::Desc::PivotA);
				VPConstraintDesc->SetProperty<Physics::PConstraint::Desc>("vector3 pivot_b", &Physics::PConstraint::Desc::PivotB);
				VPConstraintDesc->SetProperty<Physics::PConstraint::Desc>("bool collisions", &Physics::PConstraint::Desc::Collisions);
				VPConstraintDesc->SetConstructor<Physics::PConstraint::Desc>("void f()");

				auto VPConstraint = VM->SetClass<Physics::PConstraint>("physics_pconstraint", false);
				VPConstraint->SetMethod("physics_pconstraint@ copy() const", &Physics::PConstraint::Copy);
				VPConstraint->SetMethod("physics_simulator@+ get_simulator() const", &Physics::PConstraint::GetSimulator);
				VPConstraint->SetMethod("uptr@ get() const", &Physics::PConstraint::Get);
				VPConstraint->SetMethod("uptr@ get_first() const", &Physics::PConstraint::GetFirst);
				VPConstraint->SetMethod("uptr@ get_second() const", &Physics::PConstraint::GetSecond);
				VPConstraint->SetMethod("bool has_collisions() const", &Physics::PConstraint::HasCollisions);
				VPConstraint->SetMethod("bool is_enabled() const", &Physics::PConstraint::IsEnabled);
				VPConstraint->SetMethod("bool is_active() const", &Physics::PConstraint::IsActive);
				VPConstraint->SetMethod("void set_breaking_impulse_threshold(float)", &Physics::PConstraint::SetBreakingImpulseThreshold);
				VPConstraint->SetMethod("void set_enabled(bool)", &Physics::PConstraint::SetEnabled);
				VPConstraint->SetMethod("float get_breaking_impulse_threshold() const", &Physics::PConstraint::GetBreakingImpulseThreshold);
				VPConstraint->SetMethod("void set_pivot_a(const vector3 &in)", &Physics::PConstraint::SetPivotA);
				VPConstraint->SetMethod("void set_pivot_b(const vector3 &in)", &Physics::PConstraint::SetPivotB);
				VPConstraint->SetMethod("vector3 get_pivot_a() const", &Physics::PConstraint::GetPivotA);
				VPConstraint->SetMethod("vector3 get_pivot_b() const", &Physics::PConstraint::GetPivotB);
				VPConstraint->SetMethod("physics_pconstraint_desc get_state() const", &Physics::PConstraint::GetState);

				auto VHConstraintDesc = VM->SetPod<Physics::HConstraint::Desc>("physics_hconstraint_desc");
				VHConstraintDesc->SetProperty<Physics::HConstraint::Desc>("physics_rigidbody@ target_a", &Physics::HConstraint::Desc::TargetA);
				VHConstraintDesc->SetProperty<Physics::HConstraint::Desc>("physics_rigidbody@ target_b", &Physics::HConstraint::Desc::TargetB);
				VHConstraintDesc->SetProperty<Physics::HConstraint::Desc>("bool references", &Physics::HConstraint::Desc::References);
				VHConstraintDesc->SetProperty<Physics::HConstraint::Desc>("bool collisions", &Physics::HConstraint::Desc::Collisions);
				VPConstraintDesc->SetConstructor<Physics::HConstraint::Desc>("void f()");

				auto VHConstraint = VM->SetClass<Physics::HConstraint>("physics_hconstraint", false);
				VHConstraint->SetMethod("physics_hconstraint@ copy() const", &Physics::HConstraint::Copy);
				VHConstraint->SetMethod("physics_simulator@+ get_simulator() const", &Physics::HConstraint::GetSimulator);
				VHConstraint->SetMethod("uptr@ get() const", &Physics::HConstraint::Get);
				VHConstraint->SetMethod("uptr@ get_first() const", &Physics::HConstraint::GetFirst);
				VHConstraint->SetMethod("uptr@ get_second() const", &Physics::HConstraint::GetSecond);
				VHConstraint->SetMethod("bool has_collisions() const", &Physics::HConstraint::HasCollisions);
				VHConstraint->SetMethod("bool is_enabled() const", &Physics::HConstraint::IsEnabled);
				VHConstraint->SetMethod("bool is_active() const", &Physics::HConstraint::IsActive);
				VHConstraint->SetMethod("void set_breaking_impulse_threshold(float)", &Physics::HConstraint::SetBreakingImpulseThreshold);
				VHConstraint->SetMethod("void set_enabled(bool)", &Physics::HConstraint::SetEnabled);
				VHConstraint->SetMethod("float get_breaking_impulse_threshold() const", &Physics::HConstraint::GetBreakingImpulseThreshold);
				VHConstraint->SetMethod("void enable_angular_motor(bool, float, float)", &Physics::HConstraint::EnableAngularMotor);
				VHConstraint->SetMethod("void enable_motor(bool)", &Physics::HConstraint::EnableMotor);
				VHConstraint->SetMethod("void test_limit(const matrix4x4 &in, const matrix4x4 &in)", &Physics::HConstraint::TestLimit);
				VHConstraint->SetMethod("void set_frames(const matrix4x4 &in, const matrix4x4 &in)", &Physics::HConstraint::SetFrames);
				VHConstraint->SetMethod("void set_angular_only(bool)", &Physics::HConstraint::SetAngularOnly);
				VHConstraint->SetMethod("void set_max_motor_impulse(float)", &Physics::HConstraint::SetMaxMotorImpulse);
				VHConstraint->SetMethod("void set_motor_target_velocity(float)", &Physics::HConstraint::SetMotorTargetVelocity);
				VHConstraint->SetMethod("void set_motor_target(float, float)", &Physics::HConstraint::SetMotorTarget);
				VHConstraint->SetMethod("void set_limit(float Low, float High, float Softness = 0.9f, float BiasFactor = 0.3f, float RelaxationFactor = 1.0f)", &Physics::HConstraint::SetLimit);
				VHConstraint->SetMethod("void set_offset(bool Value)", &Physics::HConstraint::SetOffset);
				VHConstraint->SetMethod("void set_reference_to_a(bool Value)", &Physics::HConstraint::SetReferenceToA);
				VHConstraint->SetMethod("void set_axis(const vector3 &in)", &Physics::HConstraint::SetAxis);
				VHConstraint->SetMethod("int get_solve_limit() const", &Physics::HConstraint::GetSolveLimit);
				VHConstraint->SetMethod("float get_motor_target_velocity() const", &Physics::HConstraint::GetMotorTargetVelocity);
				VHConstraint->SetMethod("float get_max_motor_impulse() const", &Physics::HConstraint::GetMaxMotorImpulse);
				VHConstraint->SetMethod("float get_limit_sign() const", &Physics::HConstraint::GetLimitSign);
				VHConstraint->SetMethod<Physics::HConstraint, float>("float get_hinge_angle() const", &Physics::HConstraint::GetHingeAngle);
				VHConstraint->SetMethod<Physics::HConstraint, float, const Trigonometry::Matrix4x4&, const Trigonometry::Matrix4x4&>("float get_hinge_angle(const matrix4x4 &in, const matrix4x4 &in) const", &Physics::HConstraint::GetHingeAngle);
				VHConstraint->SetMethod("float get_lower_limit() const", &Physics::HConstraint::GetLowerLimit);
				VHConstraint->SetMethod("float get_upper_limit() const", &Physics::HConstraint::GetUpperLimit);
				VHConstraint->SetMethod("float get_limit_softness() const", &Physics::HConstraint::GetLimitSoftness);
				VHConstraint->SetMethod("float get_limit_bias_factor() const", &Physics::HConstraint::GetLimitBiasFactor);
				VHConstraint->SetMethod("float get_limit_relaxation_factor() const", &Physics::HConstraint::GetLimitRelaxationFactor);
				VHConstraint->SetMethod("bool has_limit() const", &Physics::HConstraint::HasLimit);
				VHConstraint->SetMethod("bool is_offset() const", &Physics::HConstraint::IsOffset);
				VHConstraint->SetMethod("bool is_reference_to_a() const", &Physics::HConstraint::IsReferenceToA);
				VHConstraint->SetMethod("bool is_angular_only() const", &Physics::HConstraint::IsAngularOnly);
				VHConstraint->SetMethod("bool is_angular_motor_enabled() const", &Physics::HConstraint::IsAngularMotorEnabled);
				VHConstraint->SetMethod("physics_hconstraint_desc& get_state()", &Physics::HConstraint::GetState);

				auto VSConstraintDesc = VM->SetPod<Physics::SConstraint::Desc>("physics_sconstraint_desc");
				VSConstraintDesc->SetProperty<Physics::SConstraint::Desc>("physics_rigidbody@ target_a", &Physics::SConstraint::Desc::TargetA);
				VSConstraintDesc->SetProperty<Physics::SConstraint::Desc>("physics_rigidbody@ target_b", &Physics::SConstraint::Desc::TargetB);
				VSConstraintDesc->SetProperty<Physics::SConstraint::Desc>("bool linear", &Physics::SConstraint::Desc::Linear);
				VSConstraintDesc->SetProperty<Physics::SConstraint::Desc>("bool collisions", &Physics::SConstraint::Desc::Collisions);
				VSConstraintDesc->SetConstructor<Physics::SConstraint::Desc>("void f()");

				auto VSConstraint = VM->SetClass<Physics::SConstraint>("physics_sconstraint", false);
				VSConstraint->SetMethod("physics_sconstraint@ copy() const", &Physics::SConstraint::Copy);
				VSConstraint->SetMethod("physics_simulator@+ get_simulator() const", &Physics::SConstraint::GetSimulator);
				VSConstraint->SetMethod("uptr@ get() const", &Physics::SConstraint::Get);
				VSConstraint->SetMethod("uptr@ get_first() const", &Physics::SConstraint::GetFirst);
				VSConstraint->SetMethod("uptr@ get_second() const", &Physics::SConstraint::GetSecond);
				VSConstraint->SetMethod("bool has_collisions() const", &Physics::SConstraint::HasCollisions);
				VSConstraint->SetMethod("bool is_enabled() const", &Physics::SConstraint::IsEnabled);
				VSConstraint->SetMethod("bool is_active() const", &Physics::SConstraint::IsActive);
				VSConstraint->SetMethod("void set_breaking_impulse_threshold(float)", &Physics::SConstraint::SetBreakingImpulseThreshold);
				VSConstraint->SetMethod("void set_enabled(bool)", &Physics::SConstraint::SetEnabled);
				VSConstraint->SetMethod("float get_breaking_impulse_threshold() const", &Physics::SConstraint::GetBreakingImpulseThreshold);
				VSConstraint->SetMethod("void set_angular_motor_velocity(float)", &Physics::SConstraint::SetAngularMotorVelocity);
				VSConstraint->SetMethod("void set_linear_motor_velocity(float)", &Physics::SConstraint::SetLinearMotorVelocity);
				VSConstraint->SetMethod("void set_upper_linear_limit(float)", &Physics::SConstraint::SetUpperLinearLimit);
				VSConstraint->SetMethod("void set_lower_linear_limit(float)", &Physics::SConstraint::SetLowerLinearLimit);
				VSConstraint->SetMethod("void set_angular_damping_direction(float)", &Physics::SConstraint::SetAngularDampingDirection);
				VSConstraint->SetMethod("void set_linear_damping_direction(float)", &Physics::SConstraint::SetLinearDampingDirection);
				VSConstraint->SetMethod("void set_angular_damping_limit(float)", &Physics::SConstraint::SetAngularDampingLimit);
				VSConstraint->SetMethod("void set_linear_damping_limit(float)", &Physics::SConstraint::SetLinearDampingLimit);
				VSConstraint->SetMethod("void set_angular_damping_ortho(float)", &Physics::SConstraint::SetAngularDampingOrtho);
				VSConstraint->SetMethod("void set_linear_damping_ortho(float)", &Physics::SConstraint::SetLinearDampingOrtho);
				VSConstraint->SetMethod("void set_upper_angular_limit(float)", &Physics::SConstraint::SetUpperAngularLimit);
				VSConstraint->SetMethod("void set_lower_angular_limit(float)", &Physics::SConstraint::SetLowerAngularLimit);
				VSConstraint->SetMethod("void set_max_angular_motor_force(float)", &Physics::SConstraint::SetMaxAngularMotorForce);
				VSConstraint->SetMethod("void set_max_linear_motor_force(float)", &Physics::SConstraint::SetMaxLinearMotorForce);
				VSConstraint->SetMethod("void set_angular_restitution_direction(float)", &Physics::SConstraint::SetAngularRestitutionDirection);
				VSConstraint->SetMethod("void set_linear_restitution_direction(float)", &Physics::SConstraint::SetLinearRestitutionDirection);
				VSConstraint->SetMethod("void set_angular_restitution_limit(float)", &Physics::SConstraint::SetAngularRestitutionLimit);
				VSConstraint->SetMethod("void set_linear_restitution_limit(float)", &Physics::SConstraint::SetLinearRestitutionLimit);
				VSConstraint->SetMethod("void set_angular_restitution_ortho(float)", &Physics::SConstraint::SetAngularRestitutionOrtho);
				VSConstraint->SetMethod("void set_linear_restitution_ortho(float)", &Physics::SConstraint::SetLinearRestitutionOrtho);
				VSConstraint->SetMethod("void set_angular_softness_direction(float)", &Physics::SConstraint::SetAngularSoftnessDirection);
				VSConstraint->SetMethod("void SetLinearSoftness_direction(float)", &Physics::SConstraint::SetLinearSoftnessDirection);
				VSConstraint->SetMethod("void Set_angular_softness_limit(float)", &Physics::SConstraint::SetAngularSoftnessLimit);
				VSConstraint->SetMethod("void Set_linear_softness_limit(float)", &Physics::SConstraint::SetLinearSoftnessLimit);
				VSConstraint->SetMethod("void Set_angular_softness_ortho(float)", &Physics::SConstraint::SetAngularSoftnessOrtho);
				VSConstraint->SetMethod("void set_linear_softness_ortho(float)", &Physics::SConstraint::SetLinearSoftnessOrtho);
				VSConstraint->SetMethod("void set_powered_angular_motor(bool)", &Physics::SConstraint::SetPoweredAngularMotor);
				VSConstraint->SetMethod("void set_powered_linear_motor(bool)", &Physics::SConstraint::SetPoweredLinearMotor);
				VSConstraint->SetMethod("float getAngularMotorVelocity() const", &Physics::SConstraint::GetAngularMotorVelocity);
				VSConstraint->SetMethod("float get_linear_motor_velocity() const", &Physics::SConstraint::GetLinearMotorVelocity);
				VSConstraint->SetMethod("float get_upper_linear_limit() const", &Physics::SConstraint::GetUpperLinearLimit);
				VSConstraint->SetMethod("float get_lower_linear_limit() const", &Physics::SConstraint::GetLowerLinearLimit);
				VSConstraint->SetMethod("float get_angular_damping_direction() const", &Physics::SConstraint::GetAngularDampingDirection);
				VSConstraint->SetMethod("float get_linear_damping_direction() const", &Physics::SConstraint::GetLinearDampingDirection);
				VSConstraint->SetMethod("float get_angular_damping_limit() const", &Physics::SConstraint::GetAngularDampingLimit);
				VSConstraint->SetMethod("float get_linear_damping_limit() const", &Physics::SConstraint::GetLinearDampingLimit);
				VSConstraint->SetMethod("float get_angular_damping_ortho() const", &Physics::SConstraint::GetAngularDampingOrtho);
				VSConstraint->SetMethod("float get_linear_damping_ortho() const", &Physics::SConstraint::GetLinearDampingOrtho);
				VSConstraint->SetMethod("float get_upper_angular_limit() const", &Physics::SConstraint::GetUpperAngularLimit);
				VSConstraint->SetMethod("float get_lower_angular_limit() const", &Physics::SConstraint::GetLowerAngularLimit);
				VSConstraint->SetMethod("float get_max_angular_motor_force() const", &Physics::SConstraint::GetMaxAngularMotorForce);
				VSConstraint->SetMethod("float get_max_linear_motor_force() const", &Physics::SConstraint::GetMaxLinearMotorForce);
				VSConstraint->SetMethod("float get_angular_restitution_direction() const", &Physics::SConstraint::GetAngularRestitutionDirection);
				VSConstraint->SetMethod("float get_linear_restitution_direction() const", &Physics::SConstraint::GetLinearRestitutionDirection);
				VSConstraint->SetMethod("float get_angular_restitution_limit() const", &Physics::SConstraint::GetAngularRestitutionLimit);
				VSConstraint->SetMethod("float get_linear_restitution_limit() const", &Physics::SConstraint::GetLinearRestitutionLimit);
				VSConstraint->SetMethod("float get_angular_restitution_ortho() const", &Physics::SConstraint::GetAngularRestitutionOrtho);
				VSConstraint->SetMethod("float get_linearRestitution_ortho() const", &Physics::SConstraint::GetLinearRestitutionOrtho);
				VSConstraint->SetMethod("float get_angular_softness_direction() const", &Physics::SConstraint::GetAngularSoftnessDirection);
				VSConstraint->SetMethod("float get_linear_softness_direction() const", &Physics::SConstraint::GetLinearSoftnessDirection);
				VSConstraint->SetMethod("float get_angular_softness_limit() const", &Physics::SConstraint::GetAngularSoftnessLimit);
				VSConstraint->SetMethod("float get_linear_softness_limit() const", &Physics::SConstraint::GetLinearSoftnessLimit);
				VSConstraint->SetMethod("float get_angular_softness_ortho() const", &Physics::SConstraint::GetAngularSoftnessOrtho);
				VSConstraint->SetMethod("float get_linear_softness_ortho() const", &Physics::SConstraint::GetLinearSoftnessOrtho);
				VSConstraint->SetMethod("bool get_powered_angular_motor() const", &Physics::SConstraint::GetPoweredAngularMotor);
				VSConstraint->SetMethod("bool get_powered_linear_motor() const", &Physics::SConstraint::GetPoweredLinearMotor);
				VSConstraint->SetMethod("physics_sconstraint_desc& get_state()", &Physics::SConstraint::GetState);

				auto VCTConstraintDesc = VM->SetPod<Physics::CTConstraint::Desc>("physics_ctconstraint_desc");
				VCTConstraintDesc->SetProperty<Physics::CTConstraint::Desc>("physics_rigidbody@ target_a", &Physics::CTConstraint::Desc::TargetA);
				VCTConstraintDesc->SetProperty<Physics::CTConstraint::Desc>("physics_rigidbody@ target_b", &Physics::CTConstraint::Desc::TargetB);
				VCTConstraintDesc->SetProperty<Physics::CTConstraint::Desc>("bool collisions", &Physics::CTConstraint::Desc::Collisions);
				VCTConstraintDesc->SetConstructor<Physics::CTConstraint::Desc>("void f()");

				auto VCTConstraint = VM->SetClass<Physics::CTConstraint>("physics_ctconstraint", false);
				VCTConstraint->SetMethod("physics_ctconstraint@ copy() const", &Physics::CTConstraint::Copy);
				VCTConstraint->SetMethod("physics_simulator@+ get_simulator() const", &Physics::CTConstraint::GetSimulator);
				VCTConstraint->SetMethod("uptr@ get() const", &Physics::CTConstraint::Get);
				VCTConstraint->SetMethod("uptr@ get_first() const", &Physics::CTConstraint::GetFirst);
				VCTConstraint->SetMethod("uptr@ get_second() const", &Physics::CTConstraint::GetSecond);
				VCTConstraint->SetMethod("bool has_collisions() const", &Physics::CTConstraint::HasCollisions);
				VCTConstraint->SetMethod("bool is_enabled() const", &Physics::CTConstraint::IsEnabled);
				VCTConstraint->SetMethod("bool is_active() const", &Physics::CTConstraint::IsActive);
				VCTConstraint->SetMethod("void set_breaking_impulse_threshold(float)", &Physics::CTConstraint::SetBreakingImpulseThreshold);
				VCTConstraint->SetMethod("void set_enabled(bool)", &Physics::CTConstraint::SetEnabled);
				VCTConstraint->SetMethod("float get_breaking_impulse_threshold() const", &Physics::CTConstraint::GetBreakingImpulseThreshold);
				VCTConstraint->SetMethod("void enable_motor(bool)", &Physics::CTConstraint::EnableMotor);
				VCTConstraint->SetMethod("void set_frames(const matrix4x4 &in, const matrix4x4 &in)", &Physics::CTConstraint::SetFrames);
				VCTConstraint->SetMethod("void set_angular_only(bool)", &Physics::CTConstraint::SetAngularOnly);
				VCTConstraint->SetMethod<Physics::CTConstraint, void, int, float>("void set_limit(int, float)", &Physics::CTConstraint::SetLimit);
				VCTConstraint->SetMethod<Physics::CTConstraint, void, float, float, float, float, float, float>("void set_limit(float, float, float, float = 1.f, float = 0.3f, float = 1.0f)", &Physics::CTConstraint::SetLimit);
				VCTConstraint->SetMethod("void set_damping(float)", &Physics::CTConstraint::SetDamping);
				VCTConstraint->SetMethod("void set_max_motor_impulse(float)", &Physics::CTConstraint::SetMaxMotorImpulse);
				VCTConstraint->SetMethod("void set_max_motor_impulse_normalized(float)", &Physics::CTConstraint::SetMaxMotorImpulseNormalized);
				VCTConstraint->SetMethod("void set_fix_thresh(float)", &Physics::CTConstraint::SetFixThresh);
				VCTConstraint->SetMethod("void set_motor_target(const quaternion &in)", &Physics::CTConstraint::SetMotorTarget);
				VCTConstraint->SetMethod("void set_motor_target_in_constraint_space(const quaternion &in)", &Physics::CTConstraint::SetMotorTargetInConstraintSpace);
				VCTConstraint->SetMethod("vector3 get_point_for_angle(float, float) const", &Physics::CTConstraint::GetPointForAngle);
				VCTConstraint->SetMethod("quaternion get_motor_target() const", &Physics::CTConstraint::GetMotorTarget);
				VCTConstraint->SetMethod("int get_solve_twist_limit() const", &Physics::CTConstraint::GetSolveTwistLimit);
				VCTConstraint->SetMethod("int get_solve_swing_limit() const", &Physics::CTConstraint::GetSolveSwingLimit);
				VCTConstraint->SetMethod("float get_twist_limit_sign() const", &Physics::CTConstraint::GetTwistLimitSign);
				VCTConstraint->SetMethod("float get_swing_span1() const", &Physics::CTConstraint::GetSwingSpan1);
				VCTConstraint->SetMethod("float get_swing_span2() const", &Physics::CTConstraint::GetSwingSpan2);
				VCTConstraint->SetMethod("float get_twist_span() const", &Physics::CTConstraint::GetTwistSpan);
				VCTConstraint->SetMethod("float get_limit_softness() const", &Physics::CTConstraint::GetLimitSoftness);
				VCTConstraint->SetMethod("float get_bias_factor() const", &Physics::CTConstraint::GetBiasFactor);
				VCTConstraint->SetMethod("float get_relaxation_factor() const", &Physics::CTConstraint::GetRelaxationFactor);
				VCTConstraint->SetMethod("float get_twist_angle() const", &Physics::CTConstraint::GetTwistAngle);
				VCTConstraint->SetMethod("float get_limit(int) const", &Physics::CTConstraint::GetLimit);
				VCTConstraint->SetMethod("float get_damping() const", &Physics::CTConstraint::GetDamping);
				VCTConstraint->SetMethod("float get_max_motor_impulse() const", &Physics::CTConstraint::GetMaxMotorImpulse);
				VCTConstraint->SetMethod("float get_fix_thresh() const", &Physics::CTConstraint::GetFixThresh);
				VCTConstraint->SetMethod("bool is_motor_enabled() const", &Physics::CTConstraint::IsMotorEnabled);
				VCTConstraint->SetMethod("bool is_max_motor_impulse_normalized() const", &Physics::CTConstraint::IsMaxMotorImpulseNormalized);
				VCTConstraint->SetMethod("bool is_past_swing_limit() const", &Physics::CTConstraint::IsPastSwingLimit);
				VCTConstraint->SetMethod("bool is_angular_only() const", &Physics::CTConstraint::IsAngularOnly);
				VCTConstraint->SetMethod("physics_ctconstraint_desc& get_state()", &Physics::CTConstraint::GetState);

				auto VDF6ConstraintDesc = VM->SetPod<Physics::DF6Constraint::Desc>("physics_df6constraint_desc");
				VDF6ConstraintDesc->SetProperty<Physics::DF6Constraint::Desc>("physics_rigidbody@ target_a", &Physics::DF6Constraint::Desc::TargetA);
				VDF6ConstraintDesc->SetProperty<Physics::DF6Constraint::Desc>("physics_rigidbody@ target_b", &Physics::DF6Constraint::Desc::TargetB);
				VDF6ConstraintDesc->SetProperty<Physics::DF6Constraint::Desc>("bool collisions", &Physics::DF6Constraint::Desc::Collisions);
				VDF6ConstraintDesc->SetConstructor<Physics::DF6Constraint::Desc>("void f()");

				auto VDF6Constraint = VM->SetClass<Physics::DF6Constraint>("physics_df6constraint", false);
				VDF6Constraint->SetMethod("physics_df6constraint@ copy() const", &Physics::DF6Constraint::Copy);
				VDF6Constraint->SetMethod("physics_simulator@+ get_simulator() const", &Physics::DF6Constraint::GetSimulator);
				VDF6Constraint->SetMethod("uptr@ get() const", &Physics::DF6Constraint::Get);
				VDF6Constraint->SetMethod("uptr@ get_first() const", &Physics::DF6Constraint::GetFirst);
				VDF6Constraint->SetMethod("uptr@ get_second() const", &Physics::DF6Constraint::GetSecond);
				VDF6Constraint->SetMethod("bool has_collisions() const", &Physics::DF6Constraint::HasCollisions);
				VDF6Constraint->SetMethod("bool is_enabled() const", &Physics::DF6Constraint::IsEnabled);
				VDF6Constraint->SetMethod("bool is_active() const", &Physics::DF6Constraint::IsActive);
				VDF6Constraint->SetMethod("void set_breaking_impulse_threshold(float)", &Physics::DF6Constraint::SetBreakingImpulseThreshold);
				VDF6Constraint->SetMethod("void set_enabled(bool)", &Physics::DF6Constraint::SetEnabled);
				VDF6Constraint->SetMethod("float get_breaking_impulse_threshold() const", &Physics::DF6Constraint::GetBreakingImpulseThreshold);
				VDF6Constraint->SetMethod("void enable_motor(int, bool)", &Physics::DF6Constraint::EnableMotor);
				VDF6Constraint->SetMethod("void enable_spring(int, bool)", &Physics::DF6Constraint::EnableSpring);
				VDF6Constraint->SetMethod("void set_frames(const matrix4x4 &in, const matrix4x4 &in)", &Physics::DF6Constraint::SetFrames);
				VDF6Constraint->SetMethod("void set_linear_lower_limit(const vector3 &in)", &Physics::DF6Constraint::SetLinearLowerLimit);
				VDF6Constraint->SetMethod("void set_linear_upper_limit(const vector3 &in)", &Physics::DF6Constraint::SetLinearUpperLimit);
				VDF6Constraint->SetMethod("void set_angular_lower_limit(const vector3 &in)", &Physics::DF6Constraint::SetAngularLowerLimit);
				VDF6Constraint->SetMethod("void set_angular_lower_limit_reversed(const vector3 &in)", &Physics::DF6Constraint::SetAngularLowerLimitReversed);
				VDF6Constraint->SetMethod("void set_angular_upper_limit(const vector3 &in)", &Physics::DF6Constraint::SetAngularUpperLimit);
				VDF6Constraint->SetMethod("void set_angular_upper_limit_reversed(const vector3 &in)", &Physics::DF6Constraint::SetAngularUpperLimitReversed);
				VDF6Constraint->SetMethod("void set_limit(int, float, float)", &Physics::DF6Constraint::SetLimit);
				VDF6Constraint->SetMethod("void set_limit_reversed(int, float, float)", &Physics::DF6Constraint::SetLimitReversed);
				VDF6Constraint->SetMethod("void set_rotation_order(physics_rotator)", &Physics::DF6Constraint::SetRotationOrder);
				VDF6Constraint->SetMethod("void set_axis(const vector3 &in, const vector3 &in)", &Physics::DF6Constraint::SetAxis);
				VDF6Constraint->SetMethod("void set_bounce(int, float)", &Physics::DF6Constraint::SetBounce);
				VDF6Constraint->SetMethod("void set_servo(int, bool)", &Physics::DF6Constraint::SetServo);
				VDF6Constraint->SetMethod("void set_target_velocity(int, float)", &Physics::DF6Constraint::SetTargetVelocity);
				VDF6Constraint->SetMethod("void set_servo_target(int, float)", &Physics::DF6Constraint::SetServoTarget);
				VDF6Constraint->SetMethod("void set_max_motor_force(int, float)", &Physics::DF6Constraint::SetMaxMotorForce);
				VDF6Constraint->SetMethod("void set_stiffness(int, float, bool = true)", &Physics::DF6Constraint::SetStiffness);
				VDF6Constraint->SetMethod<Physics::DF6Constraint, void>("void set_equilibrium_point()", &Physics::DF6Constraint::SetEquilibriumPoint);
				VDF6Constraint->SetMethod<Physics::DF6Constraint, void, int>("void set_equilibrium_point(int)", &Physics::DF6Constraint::SetEquilibriumPoint);
				VDF6Constraint->SetMethod<Physics::DF6Constraint, void, int, float>("void set_equilibrium_point(int, float)", &Physics::DF6Constraint::SetEquilibriumPoint);
				VDF6Constraint->SetMethod("vector3 get_angular_upper_limit() const", &Physics::DF6Constraint::GetAngularUpperLimit);
				VDF6Constraint->SetMethod("vector3 get_angular_upper_limit_reversed() const", &Physics::DF6Constraint::GetAngularUpperLimitReversed);
				VDF6Constraint->SetMethod("vector3 get_angular_lower_limit() const", &Physics::DF6Constraint::GetAngularLowerLimit);
				VDF6Constraint->SetMethod("vector3 get_angular_lower_limit_reversed() const", &Physics::DF6Constraint::GetAngularLowerLimitReversed);
				VDF6Constraint->SetMethod("vector3 get_linear_upper_limit() const", &Physics::DF6Constraint::GetLinearUpperLimit);
				VDF6Constraint->SetMethod("vector3 get_linear_lower_limit() const", &Physics::DF6Constraint::GetLinearLowerLimit);
				VDF6Constraint->SetMethod("vector3 get_axis(int) const", &Physics::DF6Constraint::GetAxis);
				VDF6Constraint->SetMethod("physics_rotator get_rotation_order() const", &Physics::DF6Constraint::GetRotationOrder);
				VDF6Constraint->SetMethod("float get_angle(int) const", &Physics::DF6Constraint::GetAngle);
				VDF6Constraint->SetMethod("float get_relative_pivot_position(int) const", &Physics::DF6Constraint::GetRelativePivotPosition);
				VDF6Constraint->SetMethod("bool is_limited(int) const", &Physics::DF6Constraint::IsLimited);
				VDF6Constraint->SetMethod("physics_df6constraint_desc& get_state()", &Physics::DF6Constraint::GetState);

				auto VSimulatorDesc = VM->SetPod<Physics::Simulator::Desc>("physics_simulator_desc");
				VSimulatorDesc->SetProperty<Physics::Simulator::Desc>("vector3 water_normal", &Physics::Simulator::Desc::WaterNormal);
				VSimulatorDesc->SetProperty<Physics::Simulator::Desc>("vector3 gravity", &Physics::Simulator::Desc::Gravity);
				VSimulatorDesc->SetProperty<Physics::Simulator::Desc>("float air_density", &Physics::Simulator::Desc::AirDensity);
				VSimulatorDesc->SetProperty<Physics::Simulator::Desc>("float water_density", &Physics::Simulator::Desc::WaterDensity);
				VSimulatorDesc->SetProperty<Physics::Simulator::Desc>("float water_offset", &Physics::Simulator::Desc::WaterOffset);
				VSimulatorDesc->SetProperty<Physics::Simulator::Desc>("float max_displacement", &Physics::Simulator::Desc::MaxDisplacement);
				VSimulatorDesc->SetProperty<Physics::Simulator::Desc>("bool enable_soft_body", &Physics::Simulator::Desc::EnableSoftBody);
				VSimulatorDesc->SetConstructor<Physics::Simulator::Desc>("void f()");

				VSimulator->SetProperty<Physics::Simulator>("float speedup", &Physics::Simulator::Speedup);
				VSimulator->SetProperty<Physics::Simulator>("bool active", &Physics::Simulator::Active);
				VSimulator->SetConstructor<Physics::Simulator, const Physics::Simulator::Desc&>("physics_simulator@ f(const physics_simulator_desc &in)");
				VSimulator->SetMethod("void set_gravity(const vector3 &in)", &Physics::Simulator::SetGravity);
				VSimulator->SetMethod<Physics::Simulator, void, const Trigonometry::Vector3&, bool>("void set_linear_impulse(const vector3 &in, bool = false)", &Physics::Simulator::SetLinearImpulse);
				VSimulator->SetMethod<Physics::Simulator, void, const Trigonometry::Vector3&, int, int, bool>("void set_linear_impulse(const vector3 &in, int, int, bool = false)", &Physics::Simulator::SetLinearImpulse);
				VSimulator->SetMethod<Physics::Simulator, void, const Trigonometry::Vector3&, bool>("void set_angular_impulse(const vector3 &in, bool = false)", &Physics::Simulator::SetAngularImpulse);
				VSimulator->SetMethod<Physics::Simulator, void, const Trigonometry::Vector3&, int, int, bool>("void set_angular_impulse(const vector3 &in, int, int, bool = false)", &Physics::Simulator::SetAngularImpulse);
				VSimulator->SetMethod<Physics::Simulator, void, const Trigonometry::Vector3&, bool>("void create_linear_impulse(const vector3 &in, bool = false)", &Physics::Simulator::CreateLinearImpulse);
				VSimulator->SetMethod<Physics::Simulator, void, const Trigonometry::Vector3&, int, int, bool>("void create_linear_impulse(const vector3 &in, int, int, bool = false)", &Physics::Simulator::CreateLinearImpulse);
				VSimulator->SetMethod<Physics::Simulator, void, const Trigonometry::Vector3&, bool>("void create_angular_impulse(const vector3 &in, bool = false)", &Physics::Simulator::CreateAngularImpulse);
				VSimulator->SetMethod<Physics::Simulator, void, const Trigonometry::Vector3&, int, int, bool>("void create_angular_impulse(const vector3 &in, int, int, bool = false)", &Physics::Simulator::CreateAngularImpulse);
				VSimulator->SetMethod("void add_softbody(physics_softbody@+)", &Physics::Simulator::AddSoftBody);
				VSimulator->SetMethod("void remove_softbody(physics_softbody@+)", &Physics::Simulator::RemoveSoftBody);
				VSimulator->SetMethod("void add_rigidbody(physics_rigidbody@+)", &Physics::Simulator::AddRigidBody);
				VSimulator->SetMethod("void remove_rigidbody(physics_rigidbody@+)", &Physics::Simulator::RemoveRigidBody);
				VSimulator->SetMethod("void add_constraint(physics_constraint@+)", &Physics::Simulator::AddConstraint);
				VSimulator->SetMethod("void remove_constraint(physics_constraint@+)", &Physics::Simulator::RemoveConstraint);
				VSimulator->SetMethod("void remove_all()", &Physics::Simulator::RemoveAll);
				VSimulator->SetMethod("void simulate_step(float)", &Physics::Simulator::SimulateStep);
				VSimulator->SetMethod<Physics::Simulator, Physics::RigidBody*, const Physics::RigidBody::Desc&>("physics_rigidbody@ create_rigidbody(const physics_rigidbody_desc &in)", &Physics::Simulator::CreateRigidBody);
				VSimulator->SetMethod<Physics::Simulator, Physics::RigidBody*, const Physics::RigidBody::Desc&, Trigonometry::Transform*>("physics_rigidbody@ create_rigidbody(const physics_rigidbody_desc &in, transform@+)", &Physics::Simulator::CreateRigidBody);
				VSimulator->SetMethod<Physics::Simulator, Physics::SoftBody*, const Physics::SoftBody::Desc&>("physics_softbody@ create_softbody(const physics_softbody_desc &in)", &Physics::Simulator::CreateSoftBody);
				VSimulator->SetMethod<Physics::Simulator, Physics::SoftBody*, const Physics::SoftBody::Desc&, Trigonometry::Transform*>("physics_softbody@ create_softbody(const physics_softbody_desc &in, transform@+)", &Physics::Simulator::CreateSoftBody);
				VSimulator->SetMethod("physics_pconstraint@ create_pconstraint(const physics_pconstraint_desc &in)", &Physics::Simulator::CreatePoint2PointConstraint);
				VSimulator->SetMethod("physics_hconstraint@ create_hconstraint(const physics_hconstraint_desc &in)", &Physics::Simulator::CreateHingeConstraint);
				VSimulator->SetMethod("physics_sconstraint@ create_sconstraint(const physics_sconstraint_desc &in)", &Physics::Simulator::CreateSliderConstraint);
				VSimulator->SetMethod("physics_ctconstraint@ create_ctconstraint(const physics_ctconstraint_desc &in)", &Physics::Simulator::CreateConeTwistConstraint);
				VSimulator->SetMethod("physics_df6constraint@ create_df6constraint(const physics_df6constraint_desc &in)", &Physics::Simulator::Create6DoFConstraint);
				VSimulator->SetMethod("uptr@ create_shape(physics_shape)", &Physics::Simulator::CreateShape);
				VSimulator->SetMethod("uptr@ create_cube(const vector3 &in = vector3(1, 1, 1))", &Physics::Simulator::CreateCube);
				VSimulator->SetMethod("uptr@ create_sphere(float = 1)", &Physics::Simulator::CreateSphere);
				VSimulator->SetMethod("uptr@ create_capsule(float = 1, float = 1)", &Physics::Simulator::CreateCapsule);
				VSimulator->SetMethod("uptr@ create_cone(float = 1, float = 1)", &Physics::Simulator::CreateCone);
				VSimulator->SetMethod("uptr@ create_cylinder(const vector3 &in = vector3(1, 1, 1))", &Physics::Simulator::CreateCylinder);
				VSimulator->SetMethodEx("uptr@ create_convex_hull(array<skin_vertex>@+)", &SimulatorCreateConvexHullSkinVertex);
				VSimulator->SetMethodEx("uptr@ create_convex_hull(array<vertex>@+)", &SimulatorCreateConvexHullVertex);
				VSimulator->SetMethodEx("uptr@ create_convex_hull(array<vector2>@+)", &SimulatorCreateConvexHullVector2);
				VSimulator->SetMethodEx("uptr@ create_convex_hull(array<vector3>@+)", &SimulatorCreateConvexHullVector3);
				VSimulator->SetMethodEx("uptr@ create_convex_hull(array<vector4>@+)", &SimulatorCreateConvexHullVector4);
				VSimulator->SetMethod<Physics::Simulator, btCollisionShape*, btCollisionShape*>("uptr@ create_convex_hull(uptr@)", &Physics::Simulator::CreateConvexHull);
				VSimulator->SetMethod("uptr@ try_clone_shape(uptr@)", &Physics::Simulator::TryCloneShape);
				VSimulator->SetMethod("uptr@ reuse_shape(uptr@)", &Physics::Simulator::ReuseShape);
				VSimulator->SetMethod("void free_shape(uptr@)", &Physics::Simulator::FreeShape);
				VSimulator->SetMethodEx("array<vector3>@ get_shape_vertices(uptr@) const", &SimulatorGetShapeVertices);
				VSimulator->SetMethod("usize get_shape_vertices_count(uptr@) const", &Physics::Simulator::GetShapeVerticesCount);
				VSimulator->SetMethod("float get_max_displacement() const", &Physics::Simulator::GetMaxDisplacement);
				VSimulator->SetMethod("float get_air_density() const", &Physics::Simulator::GetAirDensity);
				VSimulator->SetMethod("float get_water_offset() const", &Physics::Simulator::GetWaterOffset);
				VSimulator->SetMethod("float get_water_density() const", &Physics::Simulator::GetWaterDensity);
				VSimulator->SetMethod("vector3 get_water_normal() const", &Physics::Simulator::GetWaterNormal);
				VSimulator->SetMethod("vector3 get_gravity() const", &Physics::Simulator::GetGravity);
				VSimulator->SetMethod("bool has_softbody_support() const", &Physics::Simulator::HasSoftBodySupport);
				VSimulator->SetMethod("int get_contact_manifold_count() const", &Physics::Simulator::GetContactManifoldCount);

				VConstraint->SetDynamicCast<Physics::Constraint, Physics::PConstraint>("physics_pconstraint@+");
				VConstraint->SetDynamicCast<Physics::Constraint, Physics::HConstraint>("physics_hconstraint@+");
				VConstraint->SetDynamicCast<Physics::Constraint, Physics::SConstraint>("physics_sconstraint@+");
				VConstraint->SetDynamicCast<Physics::Constraint, Physics::CTConstraint>("physics_ctconstraint@+");
				VConstraint->SetDynamicCast<Physics::Constraint, Physics::DF6Constraint>("physics_df6constraint@+");
				VPConstraint->SetDynamicCast<Physics::PConstraint, Physics::Constraint>("physics_constraint@+", true);
				VHConstraint->SetDynamicCast<Physics::HConstraint, Physics::Constraint>("physics_constraint@+", true);
				VSConstraint->SetDynamicCast<Physics::SConstraint, Physics::Constraint>("physics_constraint@+", true);
				VCTConstraint->SetDynamicCast<Physics::CTConstraint, Physics::Constraint>("physics_constraint@+", true);
				VDF6Constraint->SetDynamicCast<Physics::DF6Constraint, Physics::Constraint>("physics_constraint@+", true);

				return true;
#else
				VI_ASSERT(false, "<physics> is not loaded");
				return false;
#endif
			}
			bool HeavyRegistry::ImportGraphics(VirtualMachine* VM) noexcept
			{
#ifdef VI_BINDINGS
				VI_ASSERT(VM != nullptr, "manager should be set");
				auto VVSync = VM->SetEnum("vsync");
				VVSync->SetValue("off", (int)Graphics::VSync::Off);
				VVSync->SetValue("frequency_x1", (int)Graphics::VSync::Frequency_X1);
				VVSync->SetValue("frequency_x2", (int)Graphics::VSync::Frequency_X2);
				VVSync->SetValue("frequency_x3", (int)Graphics::VSync::Frequency_X3);
				VVSync->SetValue("frequency_x4", (int)Graphics::VSync::Frequency_X4);
				VVSync->SetValue("on", (int)Graphics::VSync::On);

				auto VSurfaceTarget = VM->SetEnum("surface_target");
				VSurfaceTarget->SetValue("t0", (int)Graphics::SurfaceTarget::T0);
				VSurfaceTarget->SetValue("t1", (int)Graphics::SurfaceTarget::T1);
				VSurfaceTarget->SetValue("t2", (int)Graphics::SurfaceTarget::T2);
				VSurfaceTarget->SetValue("t3", (int)Graphics::SurfaceTarget::T3);
				VSurfaceTarget->SetValue("t4", (int)Graphics::SurfaceTarget::T4);
				VSurfaceTarget->SetValue("t5", (int)Graphics::SurfaceTarget::T5);
				VSurfaceTarget->SetValue("t6", (int)Graphics::SurfaceTarget::T6);
				VSurfaceTarget->SetValue("t7", (int)Graphics::SurfaceTarget::T7);

				auto VPrimitiveTopology = VM->SetEnum("primitive_topology");
				VPrimitiveTopology->SetValue("invalid", (int)Graphics::PrimitiveTopology::Invalid);
				VPrimitiveTopology->SetValue("point_list", (int)Graphics::PrimitiveTopology::Point_List);
				VPrimitiveTopology->SetValue("line_list", (int)Graphics::PrimitiveTopology::Line_List);
				VPrimitiveTopology->SetValue("line_strip", (int)Graphics::PrimitiveTopology::Line_Strip);
				VPrimitiveTopology->SetValue("triangle_list", (int)Graphics::PrimitiveTopology::Triangle_List);
				VPrimitiveTopology->SetValue("triangle_strip", (int)Graphics::PrimitiveTopology::Triangle_Strip);
				VPrimitiveTopology->SetValue("line_list_adj", (int)Graphics::PrimitiveTopology::Line_List_Adj);
				VPrimitiveTopology->SetValue("line_strip_adj", (int)Graphics::PrimitiveTopology::Line_Strip_Adj);
				VPrimitiveTopology->SetValue("triangle_list_adj", (int)Graphics::PrimitiveTopology::Triangle_List_Adj);
				VPrimitiveTopology->SetValue("triangle_strip_adj", (int)Graphics::PrimitiveTopology::Triangle_Strip_Adj);

				auto VFormat = VM->SetEnum("surface_format");
				VFormat->SetValue("unknown", (int)Graphics::Format::Unknown);
				VFormat->SetValue("A8_unorm", (int)Graphics::Format::A8_Unorm);
				VFormat->SetValue("D16_unorm", (int)Graphics::Format::D16_Unorm);
				VFormat->SetValue("D24_unorm_S8_uint", (int)Graphics::Format::D24_Unorm_S8_Uint);
				VFormat->SetValue("D32_float", (int)Graphics::Format::D32_Float);
				VFormat->SetValue("R10G10B10A2_uint", (int)Graphics::Format::R10G10B10A2_Uint);
				VFormat->SetValue("R10G10B10A2_unorm", (int)Graphics::Format::R10G10B10A2_Unorm);
				VFormat->SetValue("R11G11B10_float", (int)Graphics::Format::R11G11B10_Float);
				VFormat->SetValue("R16G16B16A16_float", (int)Graphics::Format::R16G16B16A16_Float);
				VFormat->SetValue("R16G16B16A16_sint", (int)Graphics::Format::R16G16B16A16_Sint);
				VFormat->SetValue("R16G16B16A16_snorm", (int)Graphics::Format::R16G16B16A16_Snorm);
				VFormat->SetValue("R16G16B16A16_uint", (int)Graphics::Format::R16G16B16A16_Uint);
				VFormat->SetValue("R16G16B16A16_unorm", (int)Graphics::Format::R16G16B16A16_Unorm);
				VFormat->SetValue("R16G16_float", (int)Graphics::Format::R16G16_Float);
				VFormat->SetValue("R16G16_sint", (int)Graphics::Format::R16G16_Sint);
				VFormat->SetValue("R16G16_snorm", (int)Graphics::Format::R16G16_Snorm);
				VFormat->SetValue("R16G16_uint", (int)Graphics::Format::R16G16_Uint);
				VFormat->SetValue("R16G16_unorm", (int)Graphics::Format::R16G16_Unorm);
				VFormat->SetValue("R16_float", (int)Graphics::Format::R16_Float);
				VFormat->SetValue("R16_sint", (int)Graphics::Format::R16_Sint);
				VFormat->SetValue("R16_snorm", (int)Graphics::Format::R16_Snorm);
				VFormat->SetValue("R16_uint", (int)Graphics::Format::R16_Uint);
				VFormat->SetValue("R16_unorm", (int)Graphics::Format::R16_Unorm);
				VFormat->SetValue("R1_unorm", (int)Graphics::Format::R1_Unorm);
				VFormat->SetValue("R32G32B32A32_float", (int)Graphics::Format::R32G32B32A32_Float);
				VFormat->SetValue("R32G32B32A32_sint", (int)Graphics::Format::R32G32B32A32_Sint);
				VFormat->SetValue("R32G32B32A32_uint", (int)Graphics::Format::R32G32B32A32_Uint);
				VFormat->SetValue("R32G32B32_float", (int)Graphics::Format::R32G32B32_Float);
				VFormat->SetValue("R32G32B32_sint", (int)Graphics::Format::R32G32B32_Sint);
				VFormat->SetValue("R32G32B32_uint", (int)Graphics::Format::R32G32B32_Uint);
				VFormat->SetValue("R32G32_float", (int)Graphics::Format::R32G32_Float);
				VFormat->SetValue("R32G32_sint", (int)Graphics::Format::R32G32_Sint);
				VFormat->SetValue("R32G32_uint", (int)Graphics::Format::R32G32_Uint);
				VFormat->SetValue("R32_float", (int)Graphics::Format::R32_Float);
				VFormat->SetValue("R32_sint", (int)Graphics::Format::R32_Sint);
				VFormat->SetValue("R32_uint", (int)Graphics::Format::R32_Uint);
				VFormat->SetValue("R8G8B8A8_sint", (int)Graphics::Format::R8G8B8A8_Sint);
				VFormat->SetValue("R8G8B8A8_snorm", (int)Graphics::Format::R8G8B8A8_Snorm);
				VFormat->SetValue("R8G8B8A8_uint", (int)Graphics::Format::R8G8B8A8_Uint);
				VFormat->SetValue("R8G8B8A8_unorm", (int)Graphics::Format::R8G8B8A8_Unorm);
				VFormat->SetValue("R8G8B8A8_unorm_SRGB", (int)Graphics::Format::R8G8B8A8_Unorm_SRGB);
				VFormat->SetValue("R8G8_B8G8_unorm", (int)Graphics::Format::R8G8_B8G8_Unorm);
				VFormat->SetValue("R8G8_sint", (int)Graphics::Format::R8G8_Sint);
				VFormat->SetValue("R8G8_snorm", (int)Graphics::Format::R8G8_Snorm);
				VFormat->SetValue("R8G8_uint", (int)Graphics::Format::R8G8_Uint);
				VFormat->SetValue("R8G8_unorm", (int)Graphics::Format::R8G8_Unorm);
				VFormat->SetValue("R8_sint", (int)Graphics::Format::R8_Sint);
				VFormat->SetValue("R8_snorm", (int)Graphics::Format::R8_Snorm);
				VFormat->SetValue("R8_uint", (int)Graphics::Format::R8_Uint);
				VFormat->SetValue("R8_unorm", (int)Graphics::Format::R8_Unorm);
				VFormat->SetValue("R9G9B9E5_share_dexp", (int)Graphics::Format::R9G9B9E5_Share_Dexp);

				auto VResourceMap = VM->SetEnum("resource_map");
				VResourceMap->SetValue("read", (int)Graphics::ResourceMap::Read);
				VResourceMap->SetValue("write", (int)Graphics::ResourceMap::Write);
				VResourceMap->SetValue("read_write", (int)Graphics::ResourceMap::Read_Write);
				VResourceMap->SetValue("write_discard", (int)Graphics::ResourceMap::Write_Discard);
				VResourceMap->SetValue("write_no_overwrite", (int)Graphics::ResourceMap::Write_No_Overwrite);

				auto VResourceUsage = VM->SetEnum("resource_usage");
				VResourceUsage->SetValue("default_t", (int)Graphics::ResourceUsage::Default);
				VResourceUsage->SetValue("immutable", (int)Graphics::ResourceUsage::Immutable);
				VResourceUsage->SetValue("dynamic", (int)Graphics::ResourceUsage::Dynamic);
				VResourceUsage->SetValue("staging", (int)Graphics::ResourceUsage::Staging);

				auto VShaderModel = VM->SetEnum("shader_model");
				VShaderModel->SetValue("invalid", (int)Graphics::ShaderModel::Invalid);
				VShaderModel->SetValue("auto_t", (int)Graphics::ShaderModel::Auto);
				VShaderModel->SetValue("HLSL_1_0", (int)Graphics::ShaderModel::HLSL_1_0);
				VShaderModel->SetValue("HLSL_2_0", (int)Graphics::ShaderModel::HLSL_2_0);
				VShaderModel->SetValue("HLSL_3_0", (int)Graphics::ShaderModel::HLSL_3_0);
				VShaderModel->SetValue("HLSL_4_0", (int)Graphics::ShaderModel::HLSL_4_0);
				VShaderModel->SetValue("HLSL_4_1", (int)Graphics::ShaderModel::HLSL_4_1);
				VShaderModel->SetValue("HLSL_5_0", (int)Graphics::ShaderModel::HLSL_5_0);
				VShaderModel->SetValue("GLSL_1_1_0", (int)Graphics::ShaderModel::GLSL_1_1_0);
				VShaderModel->SetValue("GLSL_1_2_0", (int)Graphics::ShaderModel::GLSL_1_2_0);
				VShaderModel->SetValue("GLSL_1_3_0", (int)Graphics::ShaderModel::GLSL_1_3_0);
				VShaderModel->SetValue("GLSL_1_4_0", (int)Graphics::ShaderModel::GLSL_1_4_0);
				VShaderModel->SetValue("GLSL_1_5_0", (int)Graphics::ShaderModel::GLSL_1_5_0);
				VShaderModel->SetValue("GLSL_3_3_0", (int)Graphics::ShaderModel::GLSL_3_3_0);
				VShaderModel->SetValue("GLSL_4_0_0", (int)Graphics::ShaderModel::GLSL_4_0_0);
				VShaderModel->SetValue("GLSL_4_1_0", (int)Graphics::ShaderModel::GLSL_4_1_0);
				VShaderModel->SetValue("GLSL_4_2_0", (int)Graphics::ShaderModel::GLSL_4_2_0);
				VShaderModel->SetValue("GLSL_4_3_0", (int)Graphics::ShaderModel::GLSL_4_3_0);
				VShaderModel->SetValue("GLSL_4_4_0", (int)Graphics::ShaderModel::GLSL_4_4_0);
				VShaderModel->SetValue("GLSL_4_5_0", (int)Graphics::ShaderModel::GLSL_4_5_0);
				VShaderModel->SetValue("GLSL_4_6_0", (int)Graphics::ShaderModel::GLSL_4_6_0);

				auto VResourceBind = VM->SetEnum("resource_bind");
				VResourceBind->SetValue("vertex_buffer", (int)Graphics::ResourceBind::Vertex_Buffer);
				VResourceBind->SetValue("index_buffer", (int)Graphics::ResourceBind::Index_Buffer);
				VResourceBind->SetValue("constant_buffer", (int)Graphics::ResourceBind::Constant_Buffer);
				VResourceBind->SetValue("shader_input", (int)Graphics::ResourceBind::Shader_Input);
				VResourceBind->SetValue("stream_output", (int)Graphics::ResourceBind::Stream_Output);
				VResourceBind->SetValue("render_target", (int)Graphics::ResourceBind::Render_Target);
				VResourceBind->SetValue("depth_stencil", (int)Graphics::ResourceBind::Depth_Stencil);
				VResourceBind->SetValue("unordered_access", (int)Graphics::ResourceBind::Unordered_Access);

				auto VCPUAccess = VM->SetEnum("cpu_access");
				VCPUAccess->SetValue("none", (int)Graphics::CPUAccess::None);
				VCPUAccess->SetValue("write", (int)Graphics::CPUAccess::Write);
				VCPUAccess->SetValue("read", (int)Graphics::CPUAccess::Read);

				auto VDepthWrite = VM->SetEnum("depth_write");
				VDepthWrite->SetValue("zero", (int)Graphics::DepthWrite::Zero);
				VDepthWrite->SetValue("all", (int)Graphics::DepthWrite::All);

				auto VComparison = VM->SetEnum("comparison");
				VComparison->SetValue("never", (int)Graphics::Comparison::Never);
				VComparison->SetValue("less", (int)Graphics::Comparison::Less);
				VComparison->SetValue("equal", (int)Graphics::Comparison::Equal);
				VComparison->SetValue("less_equal", (int)Graphics::Comparison::Less_Equal);
				VComparison->SetValue("greater", (int)Graphics::Comparison::Greater);
				VComparison->SetValue("not_equal", (int)Graphics::Comparison::Not_Equal);
				VComparison->SetValue("greater_equal", (int)Graphics::Comparison::Greater_Equal);
				VComparison->SetValue("always", (int)Graphics::Comparison::Always);

				auto VStencilOperation = VM->SetEnum("stencil_operation");
				VStencilOperation->SetValue("keep", (int)Graphics::StencilOperation::Keep);
				VStencilOperation->SetValue("zero", (int)Graphics::StencilOperation::Zero);
				VStencilOperation->SetValue("replace", (int)Graphics::StencilOperation::Replace);
				VStencilOperation->SetValue("sat_add", (int)Graphics::StencilOperation::SAT_Add);
				VStencilOperation->SetValue("sat_subtract", (int)Graphics::StencilOperation::SAT_Subtract);
				VStencilOperation->SetValue("invert", (int)Graphics::StencilOperation::Invert);
				VStencilOperation->SetValue("add", (int)Graphics::StencilOperation::Add);
				VStencilOperation->SetValue("subtract", (int)Graphics::StencilOperation::Subtract);

				auto VBlend = VM->SetEnum("blend_t");
				VBlend->SetValue("zero", (int)Graphics::Blend::Zero);
				VBlend->SetValue("one", (int)Graphics::Blend::One);
				VBlend->SetValue("source_color", (int)Graphics::Blend::Source_Color);
				VBlend->SetValue("source_color_invert", (int)Graphics::Blend::Source_Color_Invert);
				VBlend->SetValue("source_alpha", (int)Graphics::Blend::Source_Alpha);
				VBlend->SetValue("source_alpha_invert", (int)Graphics::Blend::Source_Alpha_Invert);
				VBlend->SetValue("destination_alpha", (int)Graphics::Blend::Destination_Alpha);
				VBlend->SetValue("destination_alpha_invert", (int)Graphics::Blend::Destination_Alpha_Invert);
				VBlend->SetValue("destination_color", (int)Graphics::Blend::Destination_Color);
				VBlend->SetValue("destination_color_invert", (int)Graphics::Blend::Destination_Color_Invert);
				VBlend->SetValue("source_alpha_sat", (int)Graphics::Blend::Source_Alpha_SAT);
				VBlend->SetValue("blend_factor", (int)Graphics::Blend::Blend_Factor);
				VBlend->SetValue("blend_factor_invert", (int)Graphics::Blend::Blend_Factor_Invert);
				VBlend->SetValue("source1_color", (int)Graphics::Blend::Source1_Color);
				VBlend->SetValue("source1_color_invert", (int)Graphics::Blend::Source1_Color_Invert);
				VBlend->SetValue("source1_alpha", (int)Graphics::Blend::Source1_Alpha);
				VBlend->SetValue("source1_alpha_invert", (int)Graphics::Blend::Source1_Alpha_Invert);

				auto VSurfaceFill = VM->SetEnum("surface_fill");
				VSurfaceFill->SetValue("wireframe", (int)Graphics::SurfaceFill::Wireframe);
				VSurfaceFill->SetValue("solid", (int)Graphics::SurfaceFill::Solid);

				auto VPixelFilter = VM->SetEnum("pixel_filter");
				VPixelFilter->SetValue("min_mag_mip_point", (int)Graphics::PixelFilter::Min_Mag_Mip_Point);
				VPixelFilter->SetValue("min_mag_point_mip_linear", (int)Graphics::PixelFilter::Min_Mag_Point_Mip_Linear);
				VPixelFilter->SetValue("min_point_mag_linear_mip_point", (int)Graphics::PixelFilter::Min_Point_Mag_Linear_Mip_Point);
				VPixelFilter->SetValue("min_point_mag_mip_linear", (int)Graphics::PixelFilter::Min_Point_Mag_Mip_Linear);
				VPixelFilter->SetValue("min_linear_mag_mip_point", (int)Graphics::PixelFilter::Min_Linear_Mag_Mip_Point);
				VPixelFilter->SetValue("min_linear_mag_point_mip_linear", (int)Graphics::PixelFilter::Min_Linear_Mag_Point_Mip_Linear);
				VPixelFilter->SetValue("min_mag_linear_mip_point", (int)Graphics::PixelFilter::Min_Mag_Linear_Mip_Point);
				VPixelFilter->SetValue("min_mag_mip_minear", (int)Graphics::PixelFilter::Min_Mag_Mip_Linear);
				VPixelFilter->SetValue("anistropic", (int)Graphics::PixelFilter::Anistropic);
				VPixelFilter->SetValue("compare_min_mag_mip_point", (int)Graphics::PixelFilter::Compare_Min_Mag_Mip_Point);
				VPixelFilter->SetValue("compare_min_mag_point_mip_linear", (int)Graphics::PixelFilter::Compare_Min_Mag_Point_Mip_Linear);
				VPixelFilter->SetValue("compare_min_point_mag_linear_mip_point", (int)Graphics::PixelFilter::Compare_Min_Point_Mag_Linear_Mip_Point);
				VPixelFilter->SetValue("compare_min_point_mag_mip_linear", (int)Graphics::PixelFilter::Compare_Min_Point_Mag_Mip_Linear);
				VPixelFilter->SetValue("compare_min_linear_mag_mip_point", (int)Graphics::PixelFilter::Compare_Min_Linear_Mag_Mip_Point);
				VPixelFilter->SetValue("compare_min_linear_mag_point_mip_linear", (int)Graphics::PixelFilter::Compare_Min_Linear_Mag_Point_Mip_Linear);
				VPixelFilter->SetValue("compare_min_mag_linear_mip_point", (int)Graphics::PixelFilter::Compare_Min_Mag_Linear_Mip_Point);
				VPixelFilter->SetValue("compare_min_mag_mip_linear", (int)Graphics::PixelFilter::Compare_Min_Mag_Mip_Linear);
				VPixelFilter->SetValue("compare_anistropic", (int)Graphics::PixelFilter::Compare_Anistropic);

				auto VTextureAddress = VM->SetEnum("texture_address");
				VTextureAddress->SetValue("wrap", (int)Graphics::TextureAddress::Wrap);
				VTextureAddress->SetValue("mirror", (int)Graphics::TextureAddress::Mirror);
				VTextureAddress->SetValue("clamp", (int)Graphics::TextureAddress::Clamp);
				VTextureAddress->SetValue("border", (int)Graphics::TextureAddress::Border);
				VTextureAddress->SetValue("mirror_once", (int)Graphics::TextureAddress::Mirror_Once);

				auto VColorWriteEnable = VM->SetEnum("color_write_enable");
				VColorWriteEnable->SetValue("red", (int)Graphics::ColorWriteEnable::Red);
				VColorWriteEnable->SetValue("green", (int)Graphics::ColorWriteEnable::Green);
				VColorWriteEnable->SetValue("blue", (int)Graphics::ColorWriteEnable::Blue);
				VColorWriteEnable->SetValue("alpha", (int)Graphics::ColorWriteEnable::Alpha);
				VColorWriteEnable->SetValue("all", (int)Graphics::ColorWriteEnable::All);

				auto VBlendOperation = VM->SetEnum("blend_operation");
				VBlendOperation->SetValue("add", (int)Graphics::BlendOperation::Add);
				VBlendOperation->SetValue("subtract", (int)Graphics::BlendOperation::Subtract);
				VBlendOperation->SetValue("subtract_reverse", (int)Graphics::BlendOperation::Subtract_Reverse);
				VBlendOperation->SetValue("min", (int)Graphics::BlendOperation::Min);
				VBlendOperation->SetValue("max", (int)Graphics::BlendOperation::Max);

				auto VVertexCull = VM->SetEnum("vertex_cull");
				VVertexCull->SetValue("none", (int)Graphics::VertexCull::None);
				VVertexCull->SetValue("front", (int)Graphics::VertexCull::Front);
				VVertexCull->SetValue("back", (int)Graphics::VertexCull::Back);

				auto VShaderCompile = VM->SetEnum("shader_compile");
				VShaderCompile->SetValue("debug", (int)Graphics::ShaderCompile::Debug);
				VShaderCompile->SetValue("skip_validation", (int)Graphics::ShaderCompile::Skip_Validation);
				VShaderCompile->SetValue("skip_optimization", (int)Graphics::ShaderCompile::Skip_Optimization);
				VShaderCompile->SetValue("matrix_row_major", (int)Graphics::ShaderCompile::Matrix_Row_Major);
				VShaderCompile->SetValue("matrix_column_major", (int)Graphics::ShaderCompile::Matrix_Column_Major);
				VShaderCompile->SetValue("partial_precision", (int)Graphics::ShaderCompile::Partial_Precision);
				VShaderCompile->SetValue("foe_vs_no_opt", (int)Graphics::ShaderCompile::FOE_VS_No_OPT);
				VShaderCompile->SetValue("foe_ps_no_opt", (int)Graphics::ShaderCompile::FOE_PS_No_OPT);
				VShaderCompile->SetValue("no_preshader", (int)Graphics::ShaderCompile::No_Preshader);
				VShaderCompile->SetValue("avoid_flow_control", (int)Graphics::ShaderCompile::Avoid_Flow_Control);
				VShaderCompile->SetValue("prefer_flow_control", (int)Graphics::ShaderCompile::Prefer_Flow_Control);
				VShaderCompile->SetValue("enable_strictness", (int)Graphics::ShaderCompile::Enable_Strictness);
				VShaderCompile->SetValue("enable_backwards_compatibility", (int)Graphics::ShaderCompile::Enable_Backwards_Compatibility);
				VShaderCompile->SetValue("ieee_strictness", (int)Graphics::ShaderCompile::IEEE_Strictness);
				VShaderCompile->SetValue("optimization_level0", (int)Graphics::ShaderCompile::Optimization_Level0);
				VShaderCompile->SetValue("optimization_level1", (int)Graphics::ShaderCompile::Optimization_Level1);
				VShaderCompile->SetValue("optimization_level2", (int)Graphics::ShaderCompile::Optimization_Level2);
				VShaderCompile->SetValue("optimization_level3", (int)Graphics::ShaderCompile::Optimization_Level3);
				VShaderCompile->SetValue("reseed_x16", (int)Graphics::ShaderCompile::Reseed_X16);
				VShaderCompile->SetValue("reseed_x17", (int)Graphics::ShaderCompile::Reseed_X17);
				VShaderCompile->SetValue("picky", (int)Graphics::ShaderCompile::Picky);

				auto VResourceMisc = VM->SetEnum("resource_misc");
				VResourceMisc->SetValue("none", (int)Graphics::ResourceMisc::None);
				VResourceMisc->SetValue("generate_mips", (int)Graphics::ResourceMisc::Generate_Mips);
				VResourceMisc->SetValue("shared", (int)Graphics::ResourceMisc::Shared);
				VResourceMisc->SetValue("texture_cube", (int)Graphics::ResourceMisc::Texture_Cube);
				VResourceMisc->SetValue("draw_indirect_args", (int)Graphics::ResourceMisc::Draw_Indirect_Args);
				VResourceMisc->SetValue("buffer_allow_raw_views", (int)Graphics::ResourceMisc::Buffer_Allow_Raw_Views);
				VResourceMisc->SetValue("buffer_structured", (int)Graphics::ResourceMisc::Buffer_Structured);
				VResourceMisc->SetValue("clamp", (int)Graphics::ResourceMisc::Clamp);
				VResourceMisc->SetValue("shared_keyed_mutex", (int)Graphics::ResourceMisc::Shared_Keyed_Mutex);
				VResourceMisc->SetValue("gdi_compatible", (int)Graphics::ResourceMisc::GDI_Compatible);
				VResourceMisc->SetValue("shared_nt_handle", (int)Graphics::ResourceMisc::Shared_NT_Handle);
				VResourceMisc->SetValue("restricted_content", (int)Graphics::ResourceMisc::Restricted_Content);
				VResourceMisc->SetValue("restrict_shared", (int)Graphics::ResourceMisc::Restrict_Shared);
				VResourceMisc->SetValue("restrict_shared_driver", (int)Graphics::ResourceMisc::Restrict_Shared_Driver);
				VResourceMisc->SetValue("guarded", (int)Graphics::ResourceMisc::Guarded);
				VResourceMisc->SetValue("tile_pool", (int)Graphics::ResourceMisc::Tile_Pool);
				VResourceMisc->SetValue("tiled", (int)Graphics::ResourceMisc::Tiled);

				auto VShaderType = VM->SetEnum("shader_type");
				VShaderType->SetValue("vertex", (int)Graphics::ShaderType::Vertex);
				VShaderType->SetValue("pixel", (int)Graphics::ShaderType::Pixel);
				VShaderType->SetValue("geometry", (int)Graphics::ShaderType::Geometry);
				VShaderType->SetValue("hull", (int)Graphics::ShaderType::Hull);
				VShaderType->SetValue("domain", (int)Graphics::ShaderType::Domain);
				VShaderType->SetValue("compute", (int)Graphics::ShaderType::Compute);
				VShaderType->SetValue("all", (int)Graphics::ShaderType::All);

				auto VShaderLang = VM->SetEnum("shader_lang");
				VShaderLang->SetValue("none", (int)Graphics::ShaderLang::None);
				VShaderLang->SetValue("spv", (int)Graphics::ShaderLang::SPV);
				VShaderLang->SetValue("msl", (int)Graphics::ShaderLang::MSL);
				VShaderLang->SetValue("hlsl", (int)Graphics::ShaderLang::HLSL);
				VShaderLang->SetValue("glsl", (int)Graphics::ShaderLang::GLSL);
				VShaderLang->SetValue("glsl_es", (int)Graphics::ShaderLang::GLSL_ES);

				auto VAttributeType = VM->SetEnum("attribute_type");
				VAttributeType->SetValue("byte_t", (int)Graphics::AttributeType::Byte);
				VAttributeType->SetValue("ubyte_t", (int)Graphics::AttributeType::Ubyte);
				VAttributeType->SetValue("half_t", (int)Graphics::AttributeType::Half);
				VAttributeType->SetValue("float_t", (int)Graphics::AttributeType::Float);
				VAttributeType->SetValue("int_t", (int)Graphics::AttributeType::Int);
				VAttributeType->SetValue("uint_t", (int)Graphics::AttributeType::Uint);
				VAttributeType->SetValue("matrix_t", (int)Graphics::AttributeType::Matrix);

				auto VMappedSubresource = VM->SetPod<Graphics::MappedSubresource>("mapped_subresource");
				VMappedSubresource->SetProperty<Graphics::MappedSubresource>("uptr@ pointer", &Graphics::MappedSubresource::Pointer);
				VMappedSubresource->SetProperty<Graphics::MappedSubresource>("uint32 row_pitch", &Graphics::MappedSubresource::RowPitch);
				VMappedSubresource->SetProperty<Graphics::MappedSubresource>("uint32 depth_pitch", &Graphics::MappedSubresource::DepthPitch);
				VMappedSubresource->SetConstructor<Graphics::MappedSubresource>("void f()");

				auto VRenderTargetBlendState = VM->SetPod<Graphics::RenderTargetBlendState>("render_target_blend_state");
				VRenderTargetBlendState->SetProperty<Graphics::RenderTargetBlendState>("blend_t src_blend", &Graphics::RenderTargetBlendState::SrcBlend);
				VRenderTargetBlendState->SetProperty<Graphics::RenderTargetBlendState>("blend_t dest_blend", &Graphics::RenderTargetBlendState::DestBlend);
				VRenderTargetBlendState->SetProperty<Graphics::RenderTargetBlendState>("blend_operation blend_operation_mode", &Graphics::RenderTargetBlendState::BlendOperationMode);
				VRenderTargetBlendState->SetProperty<Graphics::RenderTargetBlendState>("blend_t src_blend_alpha", &Graphics::RenderTargetBlendState::SrcBlendAlpha);
				VRenderTargetBlendState->SetProperty<Graphics::RenderTargetBlendState>("blend_t dest_blend_alpha", &Graphics::RenderTargetBlendState::DestBlendAlpha);
				VRenderTargetBlendState->SetProperty<Graphics::RenderTargetBlendState>("blend_operation blend_operation_alpha", &Graphics::RenderTargetBlendState::BlendOperationAlpha);
				VRenderTargetBlendState->SetProperty<Graphics::RenderTargetBlendState>("uint8 render_target_write_mask", &Graphics::RenderTargetBlendState::RenderTargetWriteMask);
				VRenderTargetBlendState->SetProperty<Graphics::RenderTargetBlendState>("bool blend_enable", &Graphics::RenderTargetBlendState::BlendEnable);
				VRenderTargetBlendState->SetConstructor<Graphics::RenderTargetBlendState>("void f()");

				auto VSurface = VM->SetClass<Graphics::Surface>("surface_handle", false);
				VSurface->SetConstructor<Graphics::Surface>("surface_handle@ f()");
				VSurface->SetConstructor<Graphics::Surface, SDL_Surface*>("surface_handle@ f(uptr@)");
				VSurface->SetMethod("void set_handle(uptr@)", &Graphics::Surface::SetHandle);
				VSurface->SetMethod("void lock()", &Graphics::Surface::Lock);
				VSurface->SetMethod("void unlock()", &Graphics::Surface::Unlock);
				VSurface->SetMethod("int get_width() const", &Graphics::Surface::GetWidth);
				VSurface->SetMethod("int get_height() const", &Graphics::Surface::GetHeight);
				VSurface->SetMethod("int get_pitch() const", &Graphics::Surface::GetPitch);
				VSurface->SetMethod("uptr@ get_pixels() const", &Graphics::Surface::GetPixels);
				VSurface->SetMethod("uptr@ get_resource() const", &Graphics::Surface::GetResource);

				auto VDepthStencilStateDesc = VM->SetPod<Graphics::DepthStencilState::Desc>("depth_stencil_state_desc");
				VDepthStencilStateDesc->SetProperty<Graphics::DepthStencilState::Desc>("stencil_operation front_face_stencil_fail_operation", &Graphics::DepthStencilState::Desc::FrontFaceStencilFailOperation);
				VDepthStencilStateDesc->SetProperty<Graphics::DepthStencilState::Desc>("stencil_operation front_face_stencil_depth_fail_operation", &Graphics::DepthStencilState::Desc::FrontFaceStencilDepthFailOperation);
				VDepthStencilStateDesc->SetProperty<Graphics::DepthStencilState::Desc>("stencil_operation front_face_stencil_pass_operation", &Graphics::DepthStencilState::Desc::FrontFaceStencilPassOperation);
				VDepthStencilStateDesc->SetProperty<Graphics::DepthStencilState::Desc>("comparison front_face_stencil_function", &Graphics::DepthStencilState::Desc::FrontFaceStencilFunction);
				VDepthStencilStateDesc->SetProperty<Graphics::DepthStencilState::Desc>("stencil_operation back_face_stencil_fail_operation", &Graphics::DepthStencilState::Desc::BackFaceStencilFailOperation);
				VDepthStencilStateDesc->SetProperty<Graphics::DepthStencilState::Desc>("stencil_operation back_face_stencil_depth_fail_operation", &Graphics::DepthStencilState::Desc::BackFaceStencilDepthFailOperation);
				VDepthStencilStateDesc->SetProperty<Graphics::DepthStencilState::Desc>("stencil_operation back_face_stencil_pass_operation", &Graphics::DepthStencilState::Desc::BackFaceStencilPassOperation);
				VDepthStencilStateDesc->SetProperty<Graphics::DepthStencilState::Desc>("comparison back_face_stencil_function", &Graphics::DepthStencilState::Desc::BackFaceStencilFunction);
				VDepthStencilStateDesc->SetProperty<Graphics::DepthStencilState::Desc>("depth_write depth_write_mask", &Graphics::DepthStencilState::Desc::DepthWriteMask);
				VDepthStencilStateDesc->SetProperty<Graphics::DepthStencilState::Desc>("comparison depth_function", &Graphics::DepthStencilState::Desc::DepthFunction);
				VDepthStencilStateDesc->SetProperty<Graphics::DepthStencilState::Desc>("uint8 stencil_read_mask", &Graphics::DepthStencilState::Desc::StencilReadMask);
				VDepthStencilStateDesc->SetProperty<Graphics::DepthStencilState::Desc>("uint8 stencil_write_mask", &Graphics::DepthStencilState::Desc::StencilWriteMask);
				VDepthStencilStateDesc->SetProperty<Graphics::DepthStencilState::Desc>("bool depth_enable", &Graphics::DepthStencilState::Desc::DepthEnable);
				VDepthStencilStateDesc->SetProperty<Graphics::DepthStencilState::Desc>("bool stencil_enable", &Graphics::DepthStencilState::Desc::StencilEnable);
				VDepthStencilStateDesc->SetConstructor<Graphics::DepthStencilState::Desc>("void f()");

				auto VDepthStencilState = VM->SetClass<Graphics::DepthStencilState>("depth_stencil_state", false);
				VDepthStencilState->SetMethod("uptr@ get_resource() const", &Graphics::DepthStencilState::GetResource);
				VDepthStencilState->SetMethod("depth_stencil_state_desc get_state() const", &Graphics::DepthStencilState::GetState);

				auto VRasterizerStateDesc = VM->SetPod<Graphics::RasterizerState::Desc>("rasterizer_state_desc");
				VRasterizerStateDesc->SetProperty<Graphics::RasterizerState::Desc>("surface_fill fill_mode", &Graphics::RasterizerState::Desc::FillMode);
				VRasterizerStateDesc->SetProperty<Graphics::RasterizerState::Desc>("vertex_cull cull_mode", &Graphics::RasterizerState::Desc::CullMode);
				VRasterizerStateDesc->SetProperty<Graphics::RasterizerState::Desc>("float depth_bias_clamp", &Graphics::RasterizerState::Desc::DepthBiasClamp);
				VRasterizerStateDesc->SetProperty<Graphics::RasterizerState::Desc>("float slope_scaled_depth_bias", &Graphics::RasterizerState::Desc::SlopeScaledDepthBias);
				VRasterizerStateDesc->SetProperty<Graphics::RasterizerState::Desc>("int32 depth_bias", &Graphics::RasterizerState::Desc::DepthBias);
				VRasterizerStateDesc->SetProperty<Graphics::RasterizerState::Desc>("bool front_counter_clockwise", &Graphics::RasterizerState::Desc::FrontCounterClockwise);
				VRasterizerStateDesc->SetProperty<Graphics::RasterizerState::Desc>("bool depth_clip_enable", &Graphics::RasterizerState::Desc::DepthClipEnable);
				VRasterizerStateDesc->SetProperty<Graphics::RasterizerState::Desc>("bool scissor_enable", &Graphics::RasterizerState::Desc::ScissorEnable);
				VRasterizerStateDesc->SetProperty<Graphics::RasterizerState::Desc>("bool multisample_enable", &Graphics::RasterizerState::Desc::MultisampleEnable);
				VRasterizerStateDesc->SetProperty<Graphics::RasterizerState::Desc>("bool antialiased_line_enable", &Graphics::RasterizerState::Desc::AntialiasedLineEnable);
				VRasterizerStateDesc->SetConstructor<Graphics::RasterizerState::Desc>("void f()");

				auto VRasterizerState = VM->SetClass<Graphics::RasterizerState>("rasterizer_state", false);
				VRasterizerState->SetMethod("uptr@ get_resource() const", &Graphics::RasterizerState::GetResource);
				VRasterizerState->SetMethod("rasterizer_state_desc get_state() const", &Graphics::RasterizerState::GetState);

				auto VBlendStateDesc = VM->SetPod<Graphics::BlendState::Desc>("blend_state_desc");
				VBlendStateDesc->SetProperty<Graphics::BlendState::Desc>("bool alpha_to_coverage_enable", &Graphics::BlendState::Desc::AlphaToCoverageEnable);
				VBlendStateDesc->SetProperty<Graphics::BlendState::Desc>("bool independent_blend_enable", &Graphics::BlendState::Desc::IndependentBlendEnable);
				VBlendStateDesc->SetConstructor<Graphics::BlendState::Desc>("void f()");
				VBlendStateDesc->SetOperatorEx(Operators::Index, (uint32_t)Position::Left, "render_target_blend_state&", "usize", &BlendStateDescGetRenderTarget);
				VBlendStateDesc->SetOperatorEx(Operators::Index, (uint32_t)Position::Const, "const render_target_blend_state&", "usize", &BlendStateDescGetRenderTarget);

				auto VBlendState = VM->SetClass<Graphics::BlendState>("blend_state", false);
				VBlendState->SetMethod("uptr@ get_resource() const", &Graphics::BlendState::GetResource);
				VBlendState->SetMethod("blend_state_desc get_state() const", &Graphics::BlendState::GetState);

				auto VSamplerStateDesc = VM->SetPod<Graphics::SamplerState::Desc>("sampler_state_desc");
				VSamplerStateDesc->SetProperty<Graphics::SamplerState::Desc>("comparison comparison_function", &Graphics::SamplerState::Desc::ComparisonFunction);
				VSamplerStateDesc->SetProperty<Graphics::SamplerState::Desc>("texture_address address_u", &Graphics::SamplerState::Desc::AddressU);
				VSamplerStateDesc->SetProperty<Graphics::SamplerState::Desc>("texture_address address_v", &Graphics::SamplerState::Desc::AddressV);
				VSamplerStateDesc->SetProperty<Graphics::SamplerState::Desc>("texture_address address_w", &Graphics::SamplerState::Desc::AddressW);
				VSamplerStateDesc->SetProperty<Graphics::SamplerState::Desc>("pixel_filter filter", &Graphics::SamplerState::Desc::Filter);
				VSamplerStateDesc->SetProperty<Graphics::SamplerState::Desc>("float mip_lod_bias", &Graphics::SamplerState::Desc::MipLODBias);
				VSamplerStateDesc->SetProperty<Graphics::SamplerState::Desc>("uint32 max_anisotropy", &Graphics::SamplerState::Desc::MaxAnisotropy);
				VSamplerStateDesc->SetProperty<Graphics::SamplerState::Desc>("float min_lod", &Graphics::SamplerState::Desc::MinLOD);
				VSamplerStateDesc->SetProperty<Graphics::SamplerState::Desc>("float max_lod", &Graphics::SamplerState::Desc::MaxLOD);
				VSamplerStateDesc->SetConstructor<Graphics::SamplerState::Desc>("void f()");
				VSamplerStateDesc->SetOperatorEx(Operators::Index, (uint32_t)Position::Left, "float&", "usize", &SamplerStateDescGetBorderColor);
				VSamplerStateDesc->SetOperatorEx(Operators::Index, (uint32_t)Position::Const, "const float&", "usize", &SamplerStateDescGetBorderColor);

				auto VSamplerState = VM->SetClass<Graphics::SamplerState>("sampler_state", false);
				VSamplerState->SetMethod("uptr@ get_resource() const", &Graphics::SamplerState::GetResource);
				VSamplerState->SetMethod("sampler_state_desc get_state() const", &Graphics::SamplerState::GetState);

				auto VInputLayoutAttribute = VM->SetStructTrivial<Graphics::InputLayout::Attribute>("input_layout_attribute");
				VInputLayoutAttribute->SetProperty<Graphics::InputLayout::Attribute>("string semantic_name", &Graphics::InputLayout::Attribute::SemanticName);
				VInputLayoutAttribute->SetProperty<Graphics::InputLayout::Attribute>("uint32 semantic_index", &Graphics::InputLayout::Attribute::SemanticIndex);
				VInputLayoutAttribute->SetProperty<Graphics::InputLayout::Attribute>("attribute_type format", &Graphics::InputLayout::Attribute::Format);
				VInputLayoutAttribute->SetProperty<Graphics::InputLayout::Attribute>("uint32 components", &Graphics::InputLayout::Attribute::Components);
				VInputLayoutAttribute->SetProperty<Graphics::InputLayout::Attribute>("uint32 aligned_byte_offset", &Graphics::InputLayout::Attribute::AlignedByteOffset);
				VInputLayoutAttribute->SetProperty<Graphics::InputLayout::Attribute>("uint32 slot", &Graphics::InputLayout::Attribute::Slot);
				VInputLayoutAttribute->SetProperty<Graphics::InputLayout::Attribute>("bool per_vertex", &Graphics::InputLayout::Attribute::PerVertex);
				VInputLayoutAttribute->SetConstructor<Graphics::InputLayout::Attribute>("void f()");

				auto VShader = VM->SetClass<Graphics::Shader>("shader", false);
				VShader->SetMethod("bool is_valid() const", &Graphics::Shader::IsValid);

				auto VInputLayoutDesc = VM->SetStruct<Graphics::InputLayout::Desc>("input_layout_desc");
				VInputLayoutDesc->SetProperty<Graphics::InputLayout::Desc>("shader@ source", &Graphics::InputLayout::Desc::Source);
				VInputLayoutDesc->SetConstructor<Graphics::InputLayout::Desc>("void f()");
				VInputLayoutDesc->SetOperatorCopyStatic(&InputLayoutDescCopy);
				VInputLayoutDesc->SetDestructorEx("void f()", &InputLayoutDescDestructor);
				VInputLayoutDesc->SetMethodEx("void set_attributes(array<input_layout_attribute>@+)", &InputLayoutDescSetAttributes);

				auto VInputLayout = VM->SetClass<Graphics::InputLayout>("input_layout", false);
				VInputLayout->SetMethod("uptr@ get_resource() const", &Graphics::InputLayout::GetResource);
				VInputLayout->SetMethodEx("array<input_layout_attribute>@ get_attributes() const", &InputLayoutGetAttributes);

				auto VShaderDesc = VM->SetStructTrivial<Graphics::Shader::Desc>("shader_desc");
				VShaderDesc->SetProperty<Graphics::Shader::Desc>("string filename", &Graphics::Shader::Desc::Filename);
				VShaderDesc->SetProperty<Graphics::Shader::Desc>("string data", &Graphics::Shader::Desc::Data);
				VShaderDesc->SetProperty<Graphics::Shader::Desc>("shader_type stage", &Graphics::Shader::Desc::Stage);
				VShaderDesc->SetConstructor<Graphics::Shader::Desc>("void f()");
				VShaderDesc->SetMethodEx("void set_defines(array<input_layout_attribute>@+)", &ShaderDescSetDefines);

				auto VElementBufferDesc = VM->SetStructTrivial<Graphics::ElementBuffer::Desc>("element_buffer_desc");
				VElementBufferDesc->SetProperty<Graphics::ElementBuffer::Desc>("uptr@ elements", &Graphics::ElementBuffer::Desc::Elements);
				VElementBufferDesc->SetProperty<Graphics::ElementBuffer::Desc>("uint32 structure_byte_stride", &Graphics::ElementBuffer::Desc::StructureByteStride);
				VElementBufferDesc->SetProperty<Graphics::ElementBuffer::Desc>("uint32 element_width", &Graphics::ElementBuffer::Desc::ElementWidth);
				VElementBufferDesc->SetProperty<Graphics::ElementBuffer::Desc>("uint32 element_count", &Graphics::ElementBuffer::Desc::ElementCount);
				VElementBufferDesc->SetProperty<Graphics::ElementBuffer::Desc>("cpu_access access_flags", &Graphics::ElementBuffer::Desc::AccessFlags);
				VElementBufferDesc->SetProperty<Graphics::ElementBuffer::Desc>("resource_usage usage", &Graphics::ElementBuffer::Desc::Usage);
				VElementBufferDesc->SetProperty<Graphics::ElementBuffer::Desc>("resource_bind bind_flags", &Graphics::ElementBuffer::Desc::BindFlags);
				VElementBufferDesc->SetProperty<Graphics::ElementBuffer::Desc>("resource_misc misc_flags", &Graphics::ElementBuffer::Desc::MiscFlags);
				VElementBufferDesc->SetProperty<Graphics::ElementBuffer::Desc>("bool writable", &Graphics::ElementBuffer::Desc::Writable);
				VElementBufferDesc->SetConstructor<Graphics::ElementBuffer::Desc>("void f()");

				auto VElementBuffer = VM->SetClass<Graphics::ElementBuffer>("element_buffer", false);
				VElementBuffer->SetMethod("uptr@ get_resource() const", &Graphics::ElementBuffer::GetResource);
				VElementBuffer->SetMethod("usize get_elements() const", &Graphics::ElementBuffer::GetElements);
				VElementBuffer->SetMethod("usize get_stride() const", &Graphics::ElementBuffer::GetStride);

				auto VMeshBufferDesc = VM->SetStructTrivial<Graphics::MeshBuffer::Desc>("mesh_buffer_desc");
				VMeshBufferDesc->SetProperty<Graphics::MeshBuffer::Desc>("cpu_access access_flags", &Graphics::MeshBuffer::Desc::AccessFlags);
				VMeshBufferDesc->SetProperty<Graphics::MeshBuffer::Desc>("resource_usage usage", &Graphics::MeshBuffer::Desc::Usage);
				VMeshBufferDesc->SetConstructor<Graphics::MeshBuffer::Desc>("void f()");
				VMeshBufferDesc->SetMethodEx("void set_elements(array<vertex>@+)", &MeshBufferDescSetElements);
				VMeshBufferDesc->SetMethodEx("void set_indices(array<int>@+)", &MeshBufferDescSetIndices);

				auto VMeshBuffer = VM->SetClass<Graphics::MeshBuffer>("mesh_buffer", true);
				VMeshBuffer->SetProperty<Graphics::MeshBuffer>("matrix4x4 transform", &Graphics::MeshBuffer::Transform);
				VMeshBuffer->SetProperty<Graphics::MeshBuffer>("string name", &Graphics::MeshBuffer::Name);
				VMeshBuffer->SetMethod("element_buffer@+ get_vertex_buffer() const", &Graphics::MeshBuffer::GetVertexBuffer);
				VMeshBuffer->SetMethod("element_buffer@+ get_index_buffer() const", &Graphics::MeshBuffer::GetIndexBuffer);
				VMeshBuffer->SetEnumRefsEx<Graphics::MeshBuffer>([](Graphics::MeshBuffer* Base, asIScriptEngine* VM) { });
				VMeshBuffer->SetReleaseRefsEx<Graphics::MeshBuffer>([](Graphics::MeshBuffer* Base, asIScriptEngine*) { });

				auto VSkinMeshBufferDesc = VM->SetStructTrivial<Graphics::SkinMeshBuffer::Desc>("skin_mesh_buffer_desc");
				VSkinMeshBufferDesc->SetProperty<Graphics::SkinMeshBuffer::Desc>("cpu_access access_flags", &Graphics::SkinMeshBuffer::Desc::AccessFlags);
				VSkinMeshBufferDesc->SetProperty<Graphics::SkinMeshBuffer::Desc>("resource_usage usage", &Graphics::SkinMeshBuffer::Desc::Usage);
				VSkinMeshBufferDesc->SetConstructor<Graphics::SkinMeshBuffer::Desc>("void f()");
				VSkinMeshBufferDesc->SetMethodEx("void set_elements(array<vertex>@+)", &SkinMeshBufferDescSetElements);
				VSkinMeshBufferDesc->SetMethodEx("void set_indices(array<int>@+)", &SkinMeshBufferDescSetIndices);

				auto VSkinMeshBuffer = VM->SetClass<Graphics::SkinMeshBuffer>("skin_mesh_buffer", true);
				VSkinMeshBuffer->SetProperty<Graphics::SkinMeshBuffer>("matrix4x4 transform", &Graphics::SkinMeshBuffer::Transform);
				VSkinMeshBuffer->SetProperty<Graphics::SkinMeshBuffer>("string name", &Graphics::SkinMeshBuffer::Name);
				VSkinMeshBuffer->SetMethod("element_buffer@+ get_vertex_buffer() const", &Graphics::SkinMeshBuffer::GetVertexBuffer);
				VSkinMeshBuffer->SetMethod("element_buffer@+ get_index_buffer() const", &Graphics::SkinMeshBuffer::GetIndexBuffer);
				VSkinMeshBuffer->SetEnumRefsEx<Graphics::SkinMeshBuffer>([](Graphics::SkinMeshBuffer* Base, asIScriptEngine* VM) { });
				VSkinMeshBuffer->SetReleaseRefsEx<Graphics::SkinMeshBuffer>([](Graphics::SkinMeshBuffer* Base, asIScriptEngine*) { });

				auto VGraphicsDevice = VM->SetClass<Graphics::GraphicsDevice>("graphics_device", true);
				auto VInstanceBufferDesc = VM->SetStruct<Graphics::InstanceBuffer::Desc>("instance_buffer_desc");
				VInstanceBufferDesc->SetProperty<Graphics::InstanceBuffer::Desc>("graphics_device@ device", &Graphics::InstanceBuffer::Desc::Device);
				VInstanceBufferDesc->SetProperty<Graphics::InstanceBuffer::Desc>("uint32 element_width", &Graphics::InstanceBuffer::Desc::ElementWidth);
				VInstanceBufferDesc->SetProperty<Graphics::InstanceBuffer::Desc>("uint32 element_limit", &Graphics::InstanceBuffer::Desc::ElementLimit);
				VInstanceBufferDesc->SetConstructor<Graphics::InstanceBuffer::Desc>("void f()");
				VInstanceBufferDesc->SetOperatorCopyStatic(&InstanceBufferDescCopy);
				VInstanceBufferDesc->SetDestructorEx("void f()", &InstanceBufferDescDestructor);

				auto VInstanceBuffer = VM->SetClass<Graphics::InstanceBuffer>("instance_buffer", true);
				VInstanceBuffer->SetMethodEx("void set_array(array<element_vertex>@+)", &InstanceBufferSetArray);
				VInstanceBuffer->SetMethodEx("array<element_vertex>@ get_array() const", &InstanceBufferGetArray);
				VInstanceBuffer->SetMethod("element_buffer@+ get_elements() const", &Graphics::InstanceBuffer::GetElements);
				VInstanceBuffer->SetMethod("graphics_device@+ get_device() const", &Graphics::InstanceBuffer::GetDevice);
				VInstanceBuffer->SetMethod("usize get_element_limit() const", &Graphics::InstanceBuffer::GetElementLimit);
				VInstanceBuffer->SetEnumRefsEx<Graphics::InstanceBuffer>([](Graphics::InstanceBuffer* Base, asIScriptEngine* VM) { });
				VInstanceBuffer->SetReleaseRefsEx<Graphics::InstanceBuffer>([](Graphics::InstanceBuffer* Base, asIScriptEngine*) { });

				auto VTexture2DDesc = VM->SetPod<Graphics::Texture2D::Desc>("texture_2d_desc");
				VTexture2DDesc->SetProperty<Graphics::Texture2D::Desc>("cpu_access access_flags", &Graphics::Texture2D::Desc::AccessFlags);
				VTexture2DDesc->SetProperty<Graphics::Texture2D::Desc>("surface_format format_mode", &Graphics::Texture2D::Desc::FormatMode);
				VTexture2DDesc->SetProperty<Graphics::Texture2D::Desc>("resource_usage usage", &Graphics::Texture2D::Desc::Usage);
				VTexture2DDesc->SetProperty<Graphics::Texture2D::Desc>("resource_bind bind_flags", &Graphics::Texture2D::Desc::BindFlags);
				VTexture2DDesc->SetProperty<Graphics::Texture2D::Desc>("resource_misc misc_flags", &Graphics::Texture2D::Desc::MiscFlags);
				VTexture2DDesc->SetProperty<Graphics::Texture2D::Desc>("uptr@ data", &Graphics::Texture2D::Desc::Data);
				VTexture2DDesc->SetProperty<Graphics::Texture2D::Desc>("uint32 row_pitch", &Graphics::Texture2D::Desc::RowPitch);
				VTexture2DDesc->SetProperty<Graphics::Texture2D::Desc>("uint32 depth_pitch", &Graphics::Texture2D::Desc::DepthPitch);
				VTexture2DDesc->SetProperty<Graphics::Texture2D::Desc>("uint32 width", &Graphics::Texture2D::Desc::Width);
				VTexture2DDesc->SetProperty<Graphics::Texture2D::Desc>("uint32 height", &Graphics::Texture2D::Desc::Height);
				VTexture2DDesc->SetProperty<Graphics::Texture2D::Desc>("int32 mip_levels", &Graphics::Texture2D::Desc::MipLevels);
				VTexture2DDesc->SetProperty<Graphics::Texture2D::Desc>("bool writable", &Graphics::Texture2D::Desc::Writable);
				VTexture2DDesc->SetConstructor<Graphics::Texture2D::Desc>("void f()");

				auto VTexture2D = VM->SetClass<Graphics::Texture2D>("texture_2d", false);
				VTexture2D->SetMethod("uptr@ get_resource() const", &Graphics::Texture2D::GetResource);
				VTexture2D->SetMethod("cpu_access get_access_flags() const", &Graphics::Texture2D::GetAccessFlags);
				VTexture2D->SetMethod("surface_format get_format_mode() const", &Graphics::Texture2D::GetFormatMode);
				VTexture2D->SetMethod("resource_usage get_usage() const", &Graphics::Texture2D::GetUsage);
				VTexture2D->SetMethod("resource_bind get_binding() const", &Graphics::Texture2D::GetBinding);
				VTexture2D->SetMethod("uint32 get_width() const", &Graphics::Texture2D::GetWidth);
				VTexture2D->SetMethod("uint32 get_height() const", &Graphics::Texture2D::GetHeight);
				VTexture2D->SetMethod("uint32 get_mip_levels() const", &Graphics::Texture2D::GetMipLevels);

				auto VTexture3DDesc = VM->SetPod<Graphics::Texture3D::Desc>("texture_3d_desc");
				VTexture3DDesc->SetProperty<Graphics::Texture3D::Desc>("cpu_access access_flags", &Graphics::Texture3D::Desc::AccessFlags);
				VTexture3DDesc->SetProperty<Graphics::Texture3D::Desc>("surface_format format_mode", &Graphics::Texture3D::Desc::FormatMode);
				VTexture3DDesc->SetProperty<Graphics::Texture3D::Desc>("resource_usage usage", &Graphics::Texture3D::Desc::Usage);
				VTexture3DDesc->SetProperty<Graphics::Texture3D::Desc>("resource_bind bind_flags", &Graphics::Texture3D::Desc::BindFlags);
				VTexture3DDesc->SetProperty<Graphics::Texture3D::Desc>("resource_misc misc_flags", &Graphics::Texture3D::Desc::MiscFlags);
				VTexture3DDesc->SetProperty<Graphics::Texture3D::Desc>("uint32 width", &Graphics::Texture3D::Desc::Width);
				VTexture3DDesc->SetProperty<Graphics::Texture3D::Desc>("uint32 height", &Graphics::Texture3D::Desc::Height);
				VTexture3DDesc->SetProperty<Graphics::Texture3D::Desc>("uint32 depth", &Graphics::Texture3D::Desc::Depth);
				VTexture3DDesc->SetProperty<Graphics::Texture3D::Desc>("int32 mip_levels", &Graphics::Texture3D::Desc::MipLevels);
				VTexture3DDesc->SetProperty<Graphics::Texture3D::Desc>("bool writable", &Graphics::Texture3D::Desc::Writable);
				VTexture3DDesc->SetConstructor<Graphics::Texture3D::Desc>("void f()");

				auto VTexture3D = VM->SetClass<Graphics::Texture3D>("texture_3d", false);
				VTexture3D->SetMethod("uptr@ get_resource() const", &Graphics::Texture3D::GetResource);
				VTexture3D->SetMethod("cpu_access get_access_flags() const", &Graphics::Texture3D::GetAccessFlags);
				VTexture3D->SetMethod("surface_format get_format_mode() const", &Graphics::Texture3D::GetFormatMode);
				VTexture3D->SetMethod("resource_usage get_usage() const", &Graphics::Texture3D::GetUsage);
				VTexture3D->SetMethod("resource_bind get_binding() const", &Graphics::Texture3D::GetBinding);
				VTexture3D->SetMethod("uint32 get_width() const", &Graphics::Texture3D::GetWidth);
				VTexture3D->SetMethod("uint32 get_height() const", &Graphics::Texture3D::GetHeight);
				VTexture3D->SetMethod("uint32 get_depth() const", &Graphics::Texture3D::GetDepth);
				VTexture3D->SetMethod("uint32 get_mip_levels() const", &Graphics::Texture3D::GetMipLevels);

				auto VTextureCubeDesc = VM->SetPod<Graphics::TextureCube::Desc>("texture_cube_desc");
				VTextureCubeDesc->SetProperty<Graphics::TextureCube::Desc>("cpu_access access_flags", &Graphics::TextureCube::Desc::AccessFlags);
				VTextureCubeDesc->SetProperty<Graphics::TextureCube::Desc>("surface_format format_mode", &Graphics::TextureCube::Desc::FormatMode);
				VTextureCubeDesc->SetProperty<Graphics::TextureCube::Desc>("resource_usage usage", &Graphics::TextureCube::Desc::Usage);
				VTextureCubeDesc->SetProperty<Graphics::TextureCube::Desc>("resource_bind bind_flags", &Graphics::TextureCube::Desc::BindFlags);
				VTextureCubeDesc->SetProperty<Graphics::TextureCube::Desc>("resource_misc misc_flags", &Graphics::TextureCube::Desc::MiscFlags);
				VTextureCubeDesc->SetProperty<Graphics::TextureCube::Desc>("uint32 width", &Graphics::TextureCube::Desc::Width);
				VTextureCubeDesc->SetProperty<Graphics::TextureCube::Desc>("uint32 height", &Graphics::TextureCube::Desc::Height);
				VTextureCubeDesc->SetProperty<Graphics::TextureCube::Desc>("int32 mip_levels", &Graphics::TextureCube::Desc::MipLevels);
				VTextureCubeDesc->SetProperty<Graphics::TextureCube::Desc>("bool writable", &Graphics::TextureCube::Desc::Writable);
				VTextureCubeDesc->SetConstructor<Graphics::TextureCube::Desc>("void f()");

				auto VTextureCube = VM->SetClass<Graphics::TextureCube>("texture_cube", false);
				VTextureCube->SetMethod("uptr@ get_resource() const", &Graphics::TextureCube::GetResource);
				VTextureCube->SetMethod("cpu_access get_access_flags() const", &Graphics::TextureCube::GetAccessFlags);
				VTextureCube->SetMethod("surface_format get_format_mode() const", &Graphics::TextureCube::GetFormatMode);
				VTextureCube->SetMethod("resource_usage get_usage() const", &Graphics::TextureCube::GetUsage);
				VTextureCube->SetMethod("resource_bind get_binding() const", &Graphics::TextureCube::GetBinding);
				VTextureCube->SetMethod("uint32 get_width() const", &Graphics::TextureCube::GetWidth);
				VTextureCube->SetMethod("uint32 get_height() const", &Graphics::TextureCube::GetHeight);
				VTextureCube->SetMethod("uint32 get_mip_levels() const", &Graphics::TextureCube::GetMipLevels);

				auto VDepthTarget2DDesc = VM->SetPod<Graphics::DepthTarget2D::Desc>("depth_target_2d_desc");
				VDepthTarget2DDesc->SetProperty<Graphics::DepthTarget2D::Desc>("cpu_access access_flags", &Graphics::DepthTarget2D::Desc::AccessFlags);
				VDepthTarget2DDesc->SetProperty<Graphics::DepthTarget2D::Desc>("resource_usage usage", &Graphics::DepthTarget2D::Desc::Usage);
				VDepthTarget2DDesc->SetProperty<Graphics::DepthTarget2D::Desc>("surface_format format_mode", &Graphics::DepthTarget2D::Desc::FormatMode);
				VDepthTarget2DDesc->SetProperty<Graphics::DepthTarget2D::Desc>("uint32 width", &Graphics::DepthTarget2D::Desc::Width);
				VDepthTarget2DDesc->SetProperty<Graphics::DepthTarget2D::Desc>("uint32 height", &Graphics::DepthTarget2D::Desc::Height);
				VDepthTarget2DDesc->SetConstructor<Graphics::DepthTarget2D::Desc>("void f()");

				auto VDepthTarget2D = VM->SetClass<Graphics::DepthTarget2D>("depth_target_2d", false);
				VDepthTarget2D->SetMethod("uptr@ get_resource() const", &Graphics::DepthTarget2D::GetResource);
				VDepthTarget2D->SetMethod("uint32 get_width() const", &Graphics::DepthTarget2D::GetWidth);
				VDepthTarget2D->SetMethod("uint32 get_height() const", &Graphics::DepthTarget2D::GetHeight);
				VDepthTarget2D->SetMethod("texture_2d@+ get_target() const", &Graphics::DepthTarget2D::GetTarget);
				VDepthTarget2D->SetMethod("const viewport& get_viewport() const", &Graphics::DepthTarget2D::GetViewport);

				auto VDepthTargetCubeDesc = VM->SetPod<Graphics::DepthTargetCube::Desc>("depth_target_cube_desc");
				VDepthTargetCubeDesc->SetProperty<Graphics::DepthTargetCube::Desc>("cpu_access access_flags", &Graphics::DepthTargetCube::Desc::AccessFlags);
				VDepthTargetCubeDesc->SetProperty<Graphics::DepthTargetCube::Desc>("resource_usage usage", &Graphics::DepthTargetCube::Desc::Usage);
				VDepthTargetCubeDesc->SetProperty<Graphics::DepthTargetCube::Desc>("surface_format format_mode", &Graphics::DepthTargetCube::Desc::FormatMode);
				VDepthTargetCubeDesc->SetProperty<Graphics::DepthTargetCube::Desc>("uint32 size", &Graphics::DepthTargetCube::Desc::Size);
				VDepthTargetCubeDesc->SetConstructor<Graphics::DepthTargetCube::Desc>("void f()");

				auto VDepthTargetCube = VM->SetClass<Graphics::DepthTargetCube>("depth_target_cube", false);
				VDepthTargetCube->SetMethod("uptr@ get_resource() const", &Graphics::DepthTargetCube::GetResource);
				VDepthTargetCube->SetMethod("uint32 get_width() const", &Graphics::DepthTargetCube::GetWidth);
				VDepthTargetCube->SetMethod("uint32 get_height() const", &Graphics::DepthTargetCube::GetHeight);
				VDepthTargetCube->SetMethod("texture_2d@+ get_target() const", &Graphics::DepthTargetCube::GetTarget);
				VDepthTargetCube->SetMethod("const viewport& get_viewport() const", &Graphics::DepthTargetCube::GetViewport);

				auto VRenderTarget = VM->SetClass<Graphics::RenderTarget>("render_target", false);
				VRenderTarget->SetMethod("uptr@ get_target_buffer() const", &Graphics::RenderTarget::GetTargetBuffer);
				VRenderTarget->SetMethod("uptr@ get_depth_buffer() const", &Graphics::RenderTarget::GetDepthBuffer);
				VRenderTarget->SetMethod("uint32 get_width() const", &Graphics::RenderTarget::GetWidth);
				VRenderTarget->SetMethod("uint32 get_height() const", &Graphics::RenderTarget::GetHeight);
				VRenderTarget->SetMethod("uint32 get_target_count() const", &Graphics::RenderTarget::GetTargetCount);
				VRenderTarget->SetMethod("texture_2d@+ get_target_2d(uint32) const", &Graphics::RenderTarget::GetTarget2D);
				VRenderTarget->SetMethod("texture_cube@+ get_target_cube(uint32) const", &Graphics::RenderTarget::GetTargetCube);
				VRenderTarget->SetMethod("texture_2d@+ get_depth_stencil() const", &Graphics::RenderTarget::GetDepthStencil);
				VRenderTarget->SetMethod("const viewport& get_viewport() const", &Graphics::RenderTarget::GetViewport);

				auto VRenderTarget2DDesc = VM->SetPod<Graphics::RenderTarget2D::Desc>("render_target_2d_desc");
				VRenderTarget2DDesc->SetProperty<Graphics::RenderTarget2D::Desc>("cpu_access access_flags", &Graphics::RenderTarget2D::Desc::AccessFlags);
				VRenderTarget2DDesc->SetProperty<Graphics::RenderTarget2D::Desc>("surface_format format_mode", &Graphics::RenderTarget2D::Desc::FormatMode);
				VRenderTarget2DDesc->SetProperty<Graphics::RenderTarget2D::Desc>("resource_usage usage", &Graphics::RenderTarget2D::Desc::Usage);
				VRenderTarget2DDesc->SetProperty<Graphics::RenderTarget2D::Desc>("resource_bind bind_flags", &Graphics::RenderTarget2D::Desc::BindFlags);
				VRenderTarget2DDesc->SetProperty<Graphics::RenderTarget2D::Desc>("resource_misc misc_flags", &Graphics::RenderTarget2D::Desc::MiscFlags);
				VRenderTarget2DDesc->SetProperty<Graphics::RenderTarget2D::Desc>("uptr@ render_surface", &Graphics::RenderTarget2D::Desc::RenderSurface);
				VRenderTarget2DDesc->SetProperty<Graphics::RenderTarget2D::Desc>("uint32 width", &Graphics::RenderTarget2D::Desc::Width);
				VRenderTarget2DDesc->SetProperty<Graphics::RenderTarget2D::Desc>("uint32 height", &Graphics::RenderTarget2D::Desc::Height);
				VRenderTarget2DDesc->SetProperty<Graphics::RenderTarget2D::Desc>("uint32 mip_levels", &Graphics::RenderTarget2D::Desc::MipLevels);
				VRenderTarget2DDesc->SetProperty<Graphics::RenderTarget2D::Desc>("bool depth_stencil", &Graphics::RenderTarget2D::Desc::DepthStencil);
				VRenderTarget2DDesc->SetConstructor<Graphics::RenderTarget2D::Desc>("void f()");

				auto VRenderTarget2D = VM->SetClass<Graphics::RenderTarget2D>("render_target_2d", false);
				VRenderTarget2D->SetMethod("uptr@ get_target_buffer() const", &Graphics::RenderTarget2D::GetTargetBuffer);
				VRenderTarget2D->SetMethod("uptr@ get_depth_buffer() const", &Graphics::RenderTarget2D::GetDepthBuffer);
				VRenderTarget2D->SetMethod("uint32 get_width() const", &Graphics::RenderTarget2D::GetWidth);
				VRenderTarget2D->SetMethod("uint32 get_height() const", &Graphics::RenderTarget2D::GetHeight);
				VRenderTarget2D->SetMethod("uint32 get_target_count() const", &Graphics::RenderTarget2D::GetTargetCount);
				VRenderTarget2D->SetMethod("texture_2d@+ get_target_2d(uint32) const", &Graphics::RenderTarget2D::GetTarget2D);
				VRenderTarget2D->SetMethod("texture_cube@+ get_target_cube(uint32) const", &Graphics::RenderTarget2D::GetTargetCube);
				VRenderTarget2D->SetMethod("texture_2d@+ get_depth_stencil() const", &Graphics::RenderTarget2D::GetDepthStencil);
				VRenderTarget2D->SetMethod("const viewport& get_viewport() const", &Graphics::RenderTarget2D::GetViewport);
				VRenderTarget2D->SetMethod("texture_2d@+ get_target() const", &Graphics::RenderTarget2D::GetTarget);

				auto VMultiRenderTarget2DDesc = VM->SetPod<Graphics::MultiRenderTarget2D::Desc>("multi_render_target_2d_desc");
				VMultiRenderTarget2DDesc->SetProperty<Graphics::MultiRenderTarget2D::Desc>("cpu_access access_flags", &Graphics::MultiRenderTarget2D::Desc::AccessFlags);
				VMultiRenderTarget2DDesc->SetProperty<Graphics::MultiRenderTarget2D::Desc>("surface_target target", &Graphics::MultiRenderTarget2D::Desc::Target);
				VMultiRenderTarget2DDesc->SetProperty<Graphics::MultiRenderTarget2D::Desc>("resource_usage usage", &Graphics::MultiRenderTarget2D::Desc::Usage);
				VMultiRenderTarget2DDesc->SetProperty<Graphics::MultiRenderTarget2D::Desc>("resource_bind bind_flags", &Graphics::MultiRenderTarget2D::Desc::BindFlags);
				VMultiRenderTarget2DDesc->SetProperty<Graphics::MultiRenderTarget2D::Desc>("resource_misc misc_flags", &Graphics::MultiRenderTarget2D::Desc::MiscFlags);
				VMultiRenderTarget2DDesc->SetProperty<Graphics::MultiRenderTarget2D::Desc>("uint32 width", &Graphics::MultiRenderTarget2D::Desc::Width);
				VMultiRenderTarget2DDesc->SetProperty<Graphics::MultiRenderTarget2D::Desc>("uint32 height", &Graphics::MultiRenderTarget2D::Desc::Height);
				VMultiRenderTarget2DDesc->SetProperty<Graphics::MultiRenderTarget2D::Desc>("uint32 mip_levels", &Graphics::MultiRenderTarget2D::Desc::MipLevels);
				VMultiRenderTarget2DDesc->SetProperty<Graphics::MultiRenderTarget2D::Desc>("bool depth_stencil", &Graphics::MultiRenderTarget2D::Desc::DepthStencil);
				VMultiRenderTarget2DDesc->SetConstructor<Graphics::MultiRenderTarget2D::Desc>("void f()");
				VMultiRenderTarget2DDesc->SetMethodEx("void set_format_mode(usize, surface_format)", &MultiRenderTarget2DDescSetFormatMode);

				auto VMultiRenderTarget2D = VM->SetClass<Graphics::MultiRenderTarget2D>("multi_render_target_2d", false);
				VMultiRenderTarget2D->SetMethod("uptr@ get_target_buffer() const", &Graphics::MultiRenderTarget2D::GetTargetBuffer);
				VMultiRenderTarget2D->SetMethod("uptr@ get_depth_buffer() const", &Graphics::MultiRenderTarget2D::GetDepthBuffer);
				VMultiRenderTarget2D->SetMethod("uint32 get_width() const", &Graphics::MultiRenderTarget2D::GetWidth);
				VMultiRenderTarget2D->SetMethod("uint32 get_height() const", &Graphics::MultiRenderTarget2D::GetHeight);
				VMultiRenderTarget2D->SetMethod("uint32 get_target_count() const", &Graphics::MultiRenderTarget2D::GetTargetCount);
				VMultiRenderTarget2D->SetMethod("texture_2d@+ get_target_2d(uint32) const", &Graphics::MultiRenderTarget2D::GetTarget2D);
				VMultiRenderTarget2D->SetMethod("texture_cube@+ get_target_cube(uint32) const", &Graphics::MultiRenderTarget2D::GetTargetCube);
				VMultiRenderTarget2D->SetMethod("texture_2d@+ get_depth_stencil() const", &Graphics::MultiRenderTarget2D::GetDepthStencil);
				VMultiRenderTarget2D->SetMethod("const viewport& get_viewport() const", &Graphics::MultiRenderTarget2D::GetViewport);
				VMultiRenderTarget2D->SetMethod("texture_2d@+ get_target(uint32) const", &Graphics::MultiRenderTarget2D::GetTarget);

				auto VRenderTargetCubeDesc = VM->SetPod<Graphics::RenderTargetCube::Desc>("render_target_cube_desc");
				VRenderTargetCubeDesc->SetProperty<Graphics::RenderTargetCube::Desc>("cpu_access access_flags", &Graphics::RenderTargetCube::Desc::AccessFlags);
				VRenderTargetCubeDesc->SetProperty<Graphics::RenderTargetCube::Desc>("surface_format format_mode", &Graphics::RenderTargetCube::Desc::FormatMode);
				VRenderTargetCubeDesc->SetProperty<Graphics::RenderTargetCube::Desc>("resource_usage usage", &Graphics::RenderTargetCube::Desc::Usage);
				VRenderTargetCubeDesc->SetProperty<Graphics::RenderTargetCube::Desc>("resource_bind bind_flags", &Graphics::RenderTargetCube::Desc::BindFlags);
				VRenderTargetCubeDesc->SetProperty<Graphics::RenderTargetCube::Desc>("resource_misc misc_flags", &Graphics::RenderTargetCube::Desc::MiscFlags);
				VRenderTargetCubeDesc->SetProperty<Graphics::RenderTargetCube::Desc>("uint32 size", &Graphics::RenderTargetCube::Desc::Size);
				VRenderTargetCubeDesc->SetProperty<Graphics::RenderTargetCube::Desc>("uint32 mip_levels", &Graphics::RenderTargetCube::Desc::MipLevels);
				VRenderTargetCubeDesc->SetProperty<Graphics::RenderTargetCube::Desc>("bool depth_stencil", &Graphics::RenderTargetCube::Desc::DepthStencil);
				VRenderTargetCubeDesc->SetConstructor<Graphics::RenderTargetCube::Desc>("void f()");

				auto VRenderTargetCube = VM->SetClass<Graphics::RenderTargetCube>("render_target_cube", false);
				VRenderTargetCube->SetMethod("uptr@ get_target_buffer() const", &Graphics::RenderTargetCube::GetTargetBuffer);
				VRenderTargetCube->SetMethod("uptr@ get_depth_buffer() const", &Graphics::RenderTargetCube::GetDepthBuffer);
				VRenderTargetCube->SetMethod("uint32 get_width() const", &Graphics::RenderTargetCube::GetWidth);
				VRenderTargetCube->SetMethod("uint32 get_height() const", &Graphics::RenderTargetCube::GetHeight);
				VRenderTargetCube->SetMethod("uint32 get_target_count() const", &Graphics::RenderTargetCube::GetTargetCount);
				VRenderTargetCube->SetMethod("texture_2d@+ get_target_2d(uint32) const", &Graphics::RenderTargetCube::GetTarget2D);
				VRenderTargetCube->SetMethod("texture_cube@+ get_target_cube(uint32) const", &Graphics::RenderTargetCube::GetTargetCube);
				VRenderTargetCube->SetMethod("texture_2d@+ get_depth_stencil() const", &Graphics::RenderTargetCube::GetDepthStencil);
				VRenderTargetCube->SetMethod("const viewport& get_viewport() const", &Graphics::RenderTargetCube::GetViewport);
				VRenderTargetCube->SetMethod("texture_cube@+ get_target() const", &Graphics::RenderTargetCube::GetTarget);

				auto VMultiRenderTargetCubeDesc = VM->SetPod<Graphics::MultiRenderTargetCube::Desc>("multi_render_target_cube_desc");
				VMultiRenderTargetCubeDesc->SetProperty<Graphics::MultiRenderTargetCube::Desc>("cpu_access access_flags", &Graphics::MultiRenderTargetCube::Desc::AccessFlags);
				VMultiRenderTargetCubeDesc->SetProperty<Graphics::MultiRenderTargetCube::Desc>("surface_target target", &Graphics::MultiRenderTargetCube::Desc::Target);
				VMultiRenderTargetCubeDesc->SetProperty<Graphics::MultiRenderTargetCube::Desc>("resource_usage usage", &Graphics::MultiRenderTargetCube::Desc::Usage);
				VMultiRenderTargetCubeDesc->SetProperty<Graphics::MultiRenderTargetCube::Desc>("resource_bind bind_flags", &Graphics::MultiRenderTargetCube::Desc::BindFlags);
				VMultiRenderTargetCubeDesc->SetProperty<Graphics::MultiRenderTargetCube::Desc>("resource_misc misc_flags", &Graphics::MultiRenderTargetCube::Desc::MiscFlags);
				VMultiRenderTargetCubeDesc->SetProperty<Graphics::MultiRenderTargetCube::Desc>("uint32 size", &Graphics::MultiRenderTargetCube::Desc::Size);
				VMultiRenderTargetCubeDesc->SetProperty<Graphics::MultiRenderTargetCube::Desc>("uint32 mip_levels", &Graphics::MultiRenderTargetCube::Desc::MipLevels);
				VMultiRenderTargetCubeDesc->SetProperty<Graphics::MultiRenderTargetCube::Desc>("bool depth_stencil", &Graphics::MultiRenderTargetCube::Desc::DepthStencil);
				VMultiRenderTargetCubeDesc->SetConstructor<Graphics::MultiRenderTargetCube::Desc>("void f()");
				VMultiRenderTargetCubeDesc->SetMethodEx("void set_format_mode(usize, surface_format)", &MultiRenderTargetCubeDescSetFormatMode);

				auto VMultiRenderTargetCube = VM->SetClass<Graphics::MultiRenderTargetCube>("multi_render_target_cube", false);
				VMultiRenderTargetCube->SetMethod("uptr@ get_target_buffer() const", &Graphics::MultiRenderTargetCube::GetTargetBuffer);
				VMultiRenderTargetCube->SetMethod("uptr@ get_depth_buffer() const", &Graphics::MultiRenderTargetCube::GetDepthBuffer);
				VMultiRenderTargetCube->SetMethod("uint32 get_width() const", &Graphics::MultiRenderTargetCube::GetWidth);
				VMultiRenderTargetCube->SetMethod("uint32 get_height() const", &Graphics::MultiRenderTargetCube::GetHeight);
				VMultiRenderTargetCube->SetMethod("uint32 get_target_count() const", &Graphics::MultiRenderTargetCube::GetTargetCount);
				VMultiRenderTargetCube->SetMethod("texture_2d@+ get_target_2d(uint32) const", &Graphics::MultiRenderTargetCube::GetTarget2D);
				VMultiRenderTargetCube->SetMethod("texture_cube@+ get_target_cube(uint32) const", &Graphics::MultiRenderTargetCube::GetTargetCube);
				VMultiRenderTargetCube->SetMethod("texture_2d@+ get_depth_stencil() const", &Graphics::MultiRenderTargetCube::GetDepthStencil);
				VMultiRenderTargetCube->SetMethod("const viewport& get_viewport() const", &Graphics::MultiRenderTargetCube::GetViewport);
				VMultiRenderTargetCube->SetMethod("texture_cube@+ get_target(uint32) const", &Graphics::MultiRenderTargetCube::GetTarget);

				auto VCubemapDesc = VM->SetStruct<Graphics::Cubemap::Desc>("cubemap_desc");
				VCubemapDesc->SetProperty<Graphics::Cubemap::Desc>("render_target@ source", &Graphics::Cubemap::Desc::Source);
				VCubemapDesc->SetProperty<Graphics::Cubemap::Desc>("uint32 target", &Graphics::Cubemap::Desc::Target);
				VCubemapDesc->SetProperty<Graphics::Cubemap::Desc>("uint32 size", &Graphics::Cubemap::Desc::Size);
				VCubemapDesc->SetProperty<Graphics::Cubemap::Desc>("uint32 mip_levels", &Graphics::Cubemap::Desc::MipLevels);
				VCubemapDesc->SetConstructor<Graphics::Cubemap::Desc>("void f()");
				VCubemapDesc->SetOperatorCopyStatic(&CubemapDescCopy);
				VCubemapDesc->SetDestructorEx("void f()", &CubemapDescDestructor);

				auto VCubemap = VM->SetClass<Graphics::Cubemap>("cubemap", false);
				VCubemap->SetMethod("bool is_valid() const", &Graphics::Cubemap::IsValid);

				auto VQueryDesc = VM->SetPod<Graphics::Query::Desc>("visibility_query_desc");
				VQueryDesc->SetProperty<Graphics::Query::Desc>("bool predicate", &Graphics::Query::Desc::Predicate);
				VQueryDesc->SetProperty<Graphics::Query::Desc>("bool auto_pass", &Graphics::Query::Desc::AutoPass);
				VQueryDesc->SetConstructor<Graphics::Query::Desc>("void f()");

				auto VQuery = VM->SetClass<Graphics::Query>("visibility_query", false);
				VQuery->SetMethod("uptr@ get_resource() const", &Graphics::Query::GetResource);

				auto VGraphicsDeviceDesc = VM->SetStruct<Graphics::GraphicsDevice::Desc>("graphics_device_desc");
				VGraphicsDeviceDesc->SetProperty<Graphics::GraphicsDevice::Desc>("render_backend backend", &Graphics::GraphicsDevice::Desc::Backend);
				VGraphicsDeviceDesc->SetProperty<Graphics::GraphicsDevice::Desc>("shader_model shader_mode", &Graphics::GraphicsDevice::Desc::ShaderMode);
				VGraphicsDeviceDesc->SetProperty<Graphics::GraphicsDevice::Desc>("surface_format buffer_format", &Graphics::GraphicsDevice::Desc::BufferFormat);
				VGraphicsDeviceDesc->SetProperty<Graphics::GraphicsDevice::Desc>("vsync vsync_mode", &Graphics::GraphicsDevice::Desc::VSyncMode);
				VGraphicsDeviceDesc->SetProperty<Graphics::GraphicsDevice::Desc>("string cache_directory", &Graphics::GraphicsDevice::Desc::CacheDirectory);
				VGraphicsDeviceDesc->SetProperty<Graphics::GraphicsDevice::Desc>("int32 is_windowed", &Graphics::GraphicsDevice::Desc::IsWindowed);
				VGraphicsDeviceDesc->SetProperty<Graphics::GraphicsDevice::Desc>("bool shader_cache", &Graphics::GraphicsDevice::Desc::ShaderCache);
				VGraphicsDeviceDesc->SetProperty<Graphics::GraphicsDevice::Desc>("bool debug", &Graphics::GraphicsDevice::Desc::Debug);
				VGraphicsDeviceDesc->SetProperty<Graphics::GraphicsDevice::Desc>("bool blit_rendering", &Graphics::GraphicsDevice::Desc::BlitRendering);
				VGraphicsDeviceDesc->SetProperty<Graphics::GraphicsDevice::Desc>("uint32 presentation_flags", &Graphics::GraphicsDevice::Desc::PresentationFlags);
				VGraphicsDeviceDesc->SetProperty<Graphics::GraphicsDevice::Desc>("uint32 compilation_flags", &Graphics::GraphicsDevice::Desc::CompilationFlags);
				VGraphicsDeviceDesc->SetProperty<Graphics::GraphicsDevice::Desc>("uint32 creation_flags", &Graphics::GraphicsDevice::Desc::CreationFlags);
				VGraphicsDeviceDesc->SetProperty<Graphics::GraphicsDevice::Desc>("uint32 buffer_width", &Graphics::GraphicsDevice::Desc::BufferWidth);
				VGraphicsDeviceDesc->SetProperty<Graphics::GraphicsDevice::Desc>("uint32 buffer_height", &Graphics::GraphicsDevice::Desc::BufferHeight);
				VGraphicsDeviceDesc->SetProperty<Graphics::GraphicsDevice::Desc>("activity@ window", &Graphics::GraphicsDevice::Desc::Window);
				VGraphicsDeviceDesc->SetConstructor<Graphics::GraphicsDevice::Desc>("void f()");
				VGraphicsDeviceDesc->SetOperatorCopyStatic(&GraphicsDeviceDescCopy);
				VGraphicsDeviceDesc->SetDestructorEx("void f()", &GraphicsDeviceDescDestructor);

				VGraphicsDevice->SetMethod("void set_as_current_device()", &Graphics::GraphicsDevice::SetAsCurrentDevice);
				VGraphicsDevice->SetMethod("void set_shader_model(shader_model)", &Graphics::GraphicsDevice::SetShaderModel);
				VGraphicsDevice->SetMethod("void set_blend_state(blend_state@+)", &Graphics::GraphicsDevice::SetBlendState);
				VGraphicsDevice->SetMethod("void set_rasterizer_state(rasterizer_state@+)", &Graphics::GraphicsDevice::SetRasterizerState);
				VGraphicsDevice->SetMethod("void set_depth_stencil_state(depth_stencil_state@+)", &Graphics::GraphicsDevice::SetDepthStencilState);
				VGraphicsDevice->SetMethod("void set_input_layout(input_layout@+)", &Graphics::GraphicsDevice::SetInputLayout);
				VGraphicsDevice->SetMethodEx("bool set_shader(shader@+, uint32)", &VI_EXPECTIFY_VOID(Graphics::GraphicsDevice::SetShader));
				VGraphicsDevice->SetMethod("void set_sampler_state(sampler_state@+, uint32, uint32, uint32)", &Graphics::GraphicsDevice::SetSamplerState);
				VGraphicsDevice->SetMethod<Graphics::GraphicsDevice, void, Graphics::Shader*, uint32_t, uint32_t>("void set_buffer(shader@+, uint32, uint32)", &Graphics::GraphicsDevice::SetBuffer);
				VGraphicsDevice->SetMethod<Graphics::GraphicsDevice, void, Graphics::InstanceBuffer*, uint32_t, uint32_t>("void set_buffer(instance_buffer@+, uint32, uint32)", &Graphics::GraphicsDevice::SetBuffer);
				VGraphicsDevice->SetMethod("void set_constant_buffer(element_buffer@+, uint32, uint32)", &Graphics::GraphicsDevice::SetConstantBuffer);
				VGraphicsDevice->SetMethod("void set_structure_buffer(element_buffer@+, uint32, uint32)", &Graphics::GraphicsDevice::SetStructureBuffer);
				VGraphicsDevice->SetMethod("void set_texture_2d(texture_2d@+, uint32, uint32)", &Graphics::GraphicsDevice::SetTexture2D);
				VGraphicsDevice->SetMethod("void set_texture_3d(texture_3d@+, uint32, uint32)", &Graphics::GraphicsDevice::SetTexture3D);
				VGraphicsDevice->SetMethod("void set_texture_cube(texture_cube@+, uint32, uint32)", &Graphics::GraphicsDevice::SetTextureCube);
				VGraphicsDevice->SetMethod("void set_index_buffer(element_buffer@+, surface_format)", &Graphics::GraphicsDevice::SetIndexBuffer);
				VGraphicsDevice->SetMethodEx("void set_vertex_buffers(array<element_buffer@>@+, bool = false)", &GraphicsDeviceSetVertexBuffers);
				VGraphicsDevice->SetMethodEx("void set_writeable(array<element_buffer@>@+, uint32, uint32, bool)", &GraphicsDeviceSetWriteable1);
				VGraphicsDevice->SetMethodEx("void set_writeable(array<texture_2d@>@+, uint32, uint32, bool)", &GraphicsDeviceSetWriteable2);
				VGraphicsDevice->SetMethodEx("void set_writeable(array<texture_3d@>@+, uint32, uint32, bool)", &GraphicsDeviceSetWriteable3);
				VGraphicsDevice->SetMethodEx("void set_writeable(array<texture_cube@>@+, uint32, uint32, bool)", &GraphicsDeviceSetWriteable4);
				VGraphicsDevice->SetMethod<Graphics::GraphicsDevice, void, float, float, float>("void set_target(float, float, float)", &Graphics::GraphicsDevice::SetTarget);
				VGraphicsDevice->SetMethod<Graphics::GraphicsDevice, void>("void set_target()", &Graphics::GraphicsDevice::SetTarget);
				VGraphicsDevice->SetMethod<Graphics::GraphicsDevice, void, Graphics::DepthTarget2D*>("void set_target(depth_target_2d@+)", &Graphics::GraphicsDevice::SetTarget);
				VGraphicsDevice->SetMethod<Graphics::GraphicsDevice, void, Graphics::DepthTargetCube*>("void set_target(depth_target_cube@+)", &Graphics::GraphicsDevice::SetTarget);
				VGraphicsDevice->SetMethod<Graphics::GraphicsDevice, void, Graphics::RenderTarget*, uint32_t, float, float, float>("void set_target(render_target@+, uint32, float, float, float)", &Graphics::GraphicsDevice::SetTarget);
				VGraphicsDevice->SetMethod<Graphics::GraphicsDevice, void, Graphics::RenderTarget*, uint32_t>("void set_target(render_target@+, uint32)", &Graphics::GraphicsDevice::SetTarget);
				VGraphicsDevice->SetMethod<Graphics::GraphicsDevice, void, Graphics::RenderTarget*, float, float, float>("void set_target(render_target@+, float, float, float)", &Graphics::GraphicsDevice::SetTarget);
				VGraphicsDevice->SetMethod<Graphics::GraphicsDevice, void, Graphics::RenderTarget*>("void set_target(render_target@+)", &Graphics::GraphicsDevice::SetTarget);
				VGraphicsDevice->SetMethodEx("void set_target_map(render_target@+, array<bool>@+)", &GraphicsDeviceSetTargetMap);
				VGraphicsDevice->SetMethod("void set_target_rect(uint32, uint32)", &Graphics::GraphicsDevice::SetTargetRect);
				VGraphicsDevice->SetMethodEx("void set_viewports(array<viewport>@+)", &GraphicsDeviceSetViewports);
				VGraphicsDevice->SetMethodEx("void set_scissor_rects(array<rectangle>@+)", &GraphicsDeviceSetScissorRects);
				VGraphicsDevice->SetMethod("void set_primitive_topology(primitive_topology)", &Graphics::GraphicsDevice::SetPrimitiveTopology);
				VGraphicsDevice->SetMethod("void flush_texture(uint32, uint32, uint32)", &Graphics::GraphicsDevice::FlushTexture);
				VGraphicsDevice->SetMethod("void flush_state()", &Graphics::GraphicsDevice::FlushState);
				VGraphicsDevice->SetMethodEx("bool map(element_buffer@+, resource_map, mapped_subresource &out)", &GraphicsDeviceMap1);
				VGraphicsDevice->SetMethodEx("bool map(texture_2d@+, resource_map, mapped_subresource &out)", &GraphicsDeviceMap2);
				VGraphicsDevice->SetMethodEx("bool map(texture_3d@+, resource_map, mapped_subresource &out)", &GraphicsDeviceMap3);
				VGraphicsDevice->SetMethodEx("bool map(texture_cube@+, resource_map, mapped_subresource &out)", &GraphicsDeviceMap4);
				VGraphicsDevice->SetMethodEx("bool unmap(texture_2d@+, mapped_subresource &in)", &GraphicsDeviceUnmap1);
				VGraphicsDevice->SetMethodEx("bool unmap(texture_3d@+, mapped_subresource &in)", &GraphicsDeviceUnmap2);
				VGraphicsDevice->SetMethodEx("bool unmap(texture_cube@+, mapped_subresource &in)", &GraphicsDeviceUnmap3);
				VGraphicsDevice->SetMethodEx("bool unmap(element_buffer@+, mapped_subresource &in)", &GraphicsDeviceUnmap4);
				VGraphicsDevice->SetMethodEx("bool update_constant_buffer(element_buffer@+, uptr@, usize)", &GraphicsDeviceUpdateConstantBuffer);
				VGraphicsDevice->SetMethodEx("bool update_buffer(element_buffer@+, uptr@, usize)", &GraphicsDeviceUpdateBuffer1);
				VGraphicsDevice->SetMethodEx("bool update_buffer(shader@+, uptr@)", &GraphicsDeviceUpdateBuffer2);
				VGraphicsDevice->SetMethodEx("bool update_buffer(mesh_buffer@+, uptr@)", &GraphicsDeviceUpdateBuffer3);
				VGraphicsDevice->SetMethodEx("bool update_buffer(skin_mesh_buffer@+, uptr@)", &GraphicsDeviceUpdateBuffer4);
				VGraphicsDevice->SetMethodEx("bool update_buffer(instance_buffer@+)", &GraphicsDeviceUpdateBuffer5);
				VGraphicsDevice->SetMethodEx("bool update_buffer_size(shader@+, usize)", &GraphicsDeviceUpdateBufferSize1);
				VGraphicsDevice->SetMethodEx("bool update_buffer_size(instance_buffer@+, usize)", &GraphicsDeviceUpdateBufferSize2);
				VGraphicsDevice->SetMethod("void clear_buffer(instance_buffer@+)", &Graphics::GraphicsDevice::ClearBuffer);
				VGraphicsDevice->SetMethod<Graphics::GraphicsDevice, void, Graphics::Texture2D*>("void clear_writable(texture_2d@+)", &Graphics::GraphicsDevice::ClearWritable);
				VGraphicsDevice->SetMethod<Graphics::GraphicsDevice, void, Graphics::Texture2D*, float, float, float>("void clear_writable(texture_2d@+, float, float, float)", &Graphics::GraphicsDevice::ClearWritable);
				VGraphicsDevice->SetMethod<Graphics::GraphicsDevice, void, Graphics::Texture3D*>("void clear_writable(texture_3d@+)", &Graphics::GraphicsDevice::ClearWritable);
				VGraphicsDevice->SetMethod<Graphics::GraphicsDevice, void, Graphics::Texture3D*, float, float, float>("void clear_writable(texture_3d@+, float, float, float)", &Graphics::GraphicsDevice::ClearWritable);
				VGraphicsDevice->SetMethod<Graphics::GraphicsDevice, void, Graphics::TextureCube*>("void clear_writable(texture_cube@+)", &Graphics::GraphicsDevice::ClearWritable);
				VGraphicsDevice->SetMethod<Graphics::GraphicsDevice, void, Graphics::TextureCube*, float, float, float>("void clear_writable(texture_cube@+, float, float, float)", &Graphics::GraphicsDevice::ClearWritable);
				VGraphicsDevice->SetMethod<Graphics::GraphicsDevice, void, float, float, float>("void clear(float, float, float)", &Graphics::GraphicsDevice::Clear);
				VGraphicsDevice->SetMethod<Graphics::GraphicsDevice, void, Graphics::RenderTarget*, uint32_t, float, float, float>("void clear(render_target@+, uint32, float, float, float)", &Graphics::GraphicsDevice::Clear);
				VGraphicsDevice->SetMethod<Graphics::GraphicsDevice, void>("void clear_depth()", &Graphics::GraphicsDevice::ClearDepth);
				VGraphicsDevice->SetMethod<Graphics::GraphicsDevice, void, Graphics::DepthTarget2D*>("void clear_depth(depth_target_2d@+)", &Graphics::GraphicsDevice::ClearDepth);
				VGraphicsDevice->SetMethod<Graphics::GraphicsDevice, void, Graphics::DepthTargetCube*>("void clear_depth(depth_target_cube@+)", &Graphics::GraphicsDevice::ClearDepth);
				VGraphicsDevice->SetMethod<Graphics::GraphicsDevice, void, Graphics::RenderTarget*>("void clear_depth(render_target@+)", &Graphics::GraphicsDevice::ClearDepth);
				VGraphicsDevice->SetMethod<Graphics::GraphicsDevice, void, uint32_t, uint32_t, uint32_t>("void draw_indexed(uint32, uint32, uint32)", &Graphics::GraphicsDevice::DrawIndexed);
				VGraphicsDevice->SetMethod<Graphics::GraphicsDevice, void, Graphics::MeshBuffer*>("void draw_indexed(mesh_buffer@+)", &Graphics::GraphicsDevice::DrawIndexed);
				VGraphicsDevice->SetMethod<Graphics::GraphicsDevice, void, Graphics::SkinMeshBuffer*>("void draw_indexed(skin_mesh_buffer@+)", &Graphics::GraphicsDevice::DrawIndexed);
				VGraphicsDevice->SetMethod<Graphics::GraphicsDevice, void, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t>("void draw_indexed_instanced(uint32, uint32, uint32, uint32, uint32)", &Graphics::GraphicsDevice::DrawIndexedInstanced);
				VGraphicsDevice->SetMethod<Graphics::GraphicsDevice, void, Graphics::ElementBuffer*, Graphics::MeshBuffer*, uint32_t>("void draw_indexed_instanced(element_buffer@+, mesh_buffer@+, uint32)", &Graphics::GraphicsDevice::DrawIndexedInstanced);
				VGraphicsDevice->SetMethod<Graphics::GraphicsDevice, void, Graphics::ElementBuffer*, Graphics::SkinMeshBuffer*, uint32_t>("void draw_indexed_instanced(element_buffer@+, skin_mesh_buffer@+, uint32)", &Graphics::GraphicsDevice::DrawIndexedInstanced);
				VGraphicsDevice->SetMethod("void draw(uint32, uint32)", &Graphics::GraphicsDevice::Draw);
				VGraphicsDevice->SetMethod("void draw_instanced(uint32, uint32, uint32, uint32)", &Graphics::GraphicsDevice::DrawInstanced);
				VGraphicsDevice->SetMethod("void dispatch(uint32, uint32, uint32)", &Graphics::GraphicsDevice::Dispatch);
				VGraphicsDevice->SetMethodEx("texture_2d@ copy_texture_2d(texture_2d@+)", &GraphicsDeviceCopyTexture2D1);
				VGraphicsDevice->SetMethodEx("texture_2d@ copy_texture_2d(render_target@+, uint32)", &GraphicsDeviceCopyTexture2D2);
				VGraphicsDevice->SetMethodEx("texture_2d@ copy_texture_2d(render_target_cube@+, cube_face)", &GraphicsDeviceCopyTexture2D3);
				VGraphicsDevice->SetMethodEx("texture_2d@ copy_texture_2d(multi_render_target_cube@+, uint32, cube_face)", &GraphicsDeviceCopyTexture2D4);
				VGraphicsDevice->SetMethodEx("texture_cube@ copy_texture_cube(render_target_cube@+)", &GraphicsDeviceCopyTextureCube1);
				VGraphicsDevice->SetMethodEx("texture_cube@ copy_texture_cube(multi_render_target_cube@+, uint32)", &GraphicsDeviceCopyTextureCube2);
				VGraphicsDevice->SetMethodEx("bool copy_target(render_target@+, uint32, render_target@+, uint32)", &VI_EXPECTIFY_VOID(Graphics::GraphicsDevice::CopyTarget));
				VGraphicsDevice->SetMethodEx("texture_2d@ copy_back_buffer()", &GraphicsDeviceCopyBackBuffer);
				VGraphicsDevice->SetMethodEx("bool cubemap_push(cubemap@+, texture_cube@+)", &VI_EXPECTIFY_VOID(Graphics::GraphicsDevice::CubemapPush));
				VGraphicsDevice->SetMethodEx("bool cubemap_face(cubemap@+, cube_face)", &VI_EXPECTIFY_VOID(Graphics::GraphicsDevice::CubemapFace));
				VGraphicsDevice->SetMethodEx("bool cubemap_pop(cubemap@+)", &VI_EXPECTIFY_VOID(Graphics::GraphicsDevice::CubemapPop));
				VGraphicsDevice->SetMethodEx("array<viewport>@ get_viewports()", &GraphicsDeviceGetViewports);
				VGraphicsDevice->SetMethodEx("array<rectangle>@ get_scissor_rects()", &GraphicsDeviceGetScissorRects);
				VGraphicsDevice->SetMethodEx("bool resize_buffers(uint32, uint32)", &VI_EXPECTIFY_VOID(Graphics::GraphicsDevice::ResizeBuffers));
				VGraphicsDevice->SetMethodEx("bool generate_texture(texture_2d@+)", &GraphicsDeviceGenerateTexture1);
				VGraphicsDevice->SetMethodEx("bool generate_texture(texture_3d@+)", &GraphicsDeviceGenerateTexture2);
				VGraphicsDevice->SetMethodEx("bool generate_texture(texture_cube@+)", &GraphicsDeviceGenerateTexture3);
				VGraphicsDevice->SetMethodEx("bool get_query_data(visibility_query@+, usize &out, bool = true)", &GraphicsDeviceGetQueryData1);
				VGraphicsDevice->SetMethodEx("bool get_query_data(visibility_query@+, bool &out, bool = true)", &GraphicsDeviceGetQueryData2);
				VGraphicsDevice->SetMethod("void query_begin(visibility_query@+)", &Graphics::GraphicsDevice::QueryBegin);
				VGraphicsDevice->SetMethod("void query_end(visibility_query@+)", &Graphics::GraphicsDevice::QueryEnd);
				VGraphicsDevice->SetMethod<Graphics::GraphicsDevice, void, Graphics::Texture2D*>("void generate_mips(texture_2d@+)", &Graphics::GraphicsDevice::GenerateMips);
				VGraphicsDevice->SetMethod<Graphics::GraphicsDevice, void, Graphics::Texture3D*>("void generate_mips(texture_3d@+)", &Graphics::GraphicsDevice::GenerateMips);
				VGraphicsDevice->SetMethod<Graphics::GraphicsDevice, void, Graphics::TextureCube*>("void generate_mips(texture_cube@+)", &Graphics::GraphicsDevice::GenerateMips);
				VGraphicsDevice->SetMethod("bool im_begin()", &Graphics::GraphicsDevice::ImBegin);
				VGraphicsDevice->SetMethod("void im_transform(const matrix4x4 &in)", &Graphics::GraphicsDevice::ImTransform);
				VGraphicsDevice->SetMethod("void im_topology(primitive_topology)", &Graphics::GraphicsDevice::ImTopology);
				VGraphicsDevice->SetMethod("void im_emit()", &Graphics::GraphicsDevice::ImEmit);
				VGraphicsDevice->SetMethod("void im_texture(texture_2d@+)", &Graphics::GraphicsDevice::ImTexture);
				VGraphicsDevice->SetMethod("void im_color(float, float, float, float)", &Graphics::GraphicsDevice::ImColor);
				VGraphicsDevice->SetMethod("void im_intensity(float)", &Graphics::GraphicsDevice::ImIntensity);
				VGraphicsDevice->SetMethod("void im_texcoord(float, float)", &Graphics::GraphicsDevice::ImTexCoord);
				VGraphicsDevice->SetMethod("void im_texcoord_offset(float, float)", &Graphics::GraphicsDevice::ImTexCoordOffset);
				VGraphicsDevice->SetMethod("void im_position(float, float, float)", &Graphics::GraphicsDevice::ImPosition);
				VGraphicsDevice->SetMethod("bool im_end()", &Graphics::GraphicsDevice::ImEnd);
				VGraphicsDevice->SetMethodEx("bool submit()", &VI_EXPECTIFY_VOID(Graphics::GraphicsDevice::Submit));
				VGraphicsDevice->SetMethodEx("depth_stencil_state@ create_depth_stencil_state(const depth_stencil_state_desc &in)", &VI_EXPECTIFY(Graphics::GraphicsDevice::CreateDepthStencilState));
				VGraphicsDevice->SetMethodEx("blend_state@ create_blend_state(const blend_state_desc &in)", &VI_EXPECTIFY(Graphics::GraphicsDevice::CreateBlendState));
				VGraphicsDevice->SetMethodEx("rasterizer_state@ create_rasterizer_state(const rasterizer_state_desc &in)", &VI_EXPECTIFY(Graphics::GraphicsDevice::CreateRasterizerState));
				VGraphicsDevice->SetMethodEx("sampler_state@ create_sampler_state(const sampler_state_desc &in)", &VI_EXPECTIFY(Graphics::GraphicsDevice::CreateSamplerState));
				VGraphicsDevice->SetMethodEx("input_layout@ create_input_layout(const input_layout_desc &in)", &VI_EXPECTIFY(Graphics::GraphicsDevice::CreateInputLayout));
				VGraphicsDevice->SetMethodEx("shader@ create_shader(const shader_desc &in)", &VI_EXPECTIFY(Graphics::GraphicsDevice::CreateShader));
				VGraphicsDevice->SetMethodEx("element_buffer@ create_element_buffer(const element_buffer_desc &in)", &VI_EXPECTIFY(Graphics::GraphicsDevice::CreateElementBuffer));
				VGraphicsDevice->SetMethodEx("mesh_buffer@ create_mesh_buffer(const mesh_buffer_desc &in)", &GraphicsDeviceCreateMeshBuffer1);
				VGraphicsDevice->SetMethodEx("mesh_buffer@ create_mesh_buffer(element_buffer@+, element_buffer@+)", &GraphicsDeviceCreateMeshBuffer2);
				VGraphicsDevice->SetMethodEx("skin_mesh_buffer@ create_skin_mesh_buffer(const skin_mesh_buffer_desc &in)", &GraphicsDeviceCreateSkinMeshBuffer1);
				VGraphicsDevice->SetMethodEx("skin_mesh_buffer@ create_skin_mesh_buffer(element_buffer@+, element_buffer@+)", &GraphicsDeviceCreateSkinMeshBuffer2);
				VGraphicsDevice->SetMethodEx("instance_buffer@ create_instance_buffer(const instance_buffer_desc &in)", &GraphicsDeviceCreateInstanceBuffer);
				VGraphicsDevice->SetMethodEx("texture_2d@ create_texture_2d()", &GraphicsDeviceCreateTexture2D1);
				VGraphicsDevice->SetMethodEx("texture_2d@ create_texture_2d(const texture_2d_desc &in)", &GraphicsDeviceCreateTexture2D2);
				VGraphicsDevice->SetMethodEx("texture_3d@ create_texture_3d()", &GraphicsDeviceCreateTexture3D1);
				VGraphicsDevice->SetMethodEx("texture_3d@ create_texture_3d(const texture_3d_desc &in)", &GraphicsDeviceCreateTexture3D2);
				VGraphicsDevice->SetMethodEx("texture_cube@ create_texture_cube()", &GraphicsDeviceCreateTextureCube1);
				VGraphicsDevice->SetMethodEx("texture_cube@ create_texture_cube(const texture_cube_desc &in)", &GraphicsDeviceCreateTextureCube2);
				VGraphicsDevice->SetMethodEx("texture_cube@ create_texture_cube(array<texture_2d@>@+)", &GraphicsDeviceCreateTextureCube3);
				VGraphicsDevice->SetMethodEx("texture_cube@ create_texture_cube(texture_2d@+)", &GraphicsDeviceCreateTextureCube4);
				VGraphicsDevice->SetMethodEx("depth_target_2d@ create_depth_target_2d(const depth_target_2d_desc &in)", &VI_EXPECTIFY(Graphics::GraphicsDevice::CreateDepthTarget2D));
				VGraphicsDevice->SetMethodEx("depth_target_cube@ create_depth_target_cube(const depth_target_cube_desc &in)", &VI_EXPECTIFY(Graphics::GraphicsDevice::CreateDepthTargetCube));
				VGraphicsDevice->SetMethodEx("render_target_2d@ create_render_target_2d(const render_target_2d_desc &in)", &VI_EXPECTIFY(Graphics::GraphicsDevice::CreateRenderTarget2D));
				VGraphicsDevice->SetMethodEx("multi_render_target_2d@ create_multi_render_target_2d(const multi_render_target_2d_desc &in)", &VI_EXPECTIFY(Graphics::GraphicsDevice::CreateMultiRenderTarget2D));
				VGraphicsDevice->SetMethodEx("render_target_cube@ create_render_target_cube(const render_target_cube_desc &in)", &VI_EXPECTIFY(Graphics::GraphicsDevice::CreateRenderTargetCube));
				VGraphicsDevice->SetMethodEx("multi_render_target_cube@ create_multi_render_target_cube(const multi_render_target_cube_desc &in)", &VI_EXPECTIFY(Graphics::GraphicsDevice::CreateMultiRenderTargetCube));
				VGraphicsDevice->SetMethodEx("cubemap@ create_cubemap(const cubemap_desc &in)", &VI_EXPECTIFY(Graphics::GraphicsDevice::CreateCubemap));
				VGraphicsDevice->SetMethodEx("visibility_query@ create_query(const visibility_query_desc &in)", &VI_EXPECTIFY(Graphics::GraphicsDevice::CreateQuery));
				VGraphicsDevice->SetMethodEx("activity_surface@ create_surface(texture_2d@+)", &VI_EXPECTIFY(Graphics::GraphicsDevice::CreateSurface));
				VGraphicsDevice->SetMethod("primitive_topology get_primitive_topology() const", &Graphics::GraphicsDevice::GetPrimitiveTopology);
				VGraphicsDevice->SetMethod("shader_model get_supported_shader_model()  const", &Graphics::GraphicsDevice::GetSupportedShaderModel);
				VGraphicsDevice->SetMethod("uptr@ get_device() const", &Graphics::GraphicsDevice::GetDevice);
				VGraphicsDevice->SetMethod("uptr@ get_context() const", &Graphics::GraphicsDevice::GetContext);
				VGraphicsDevice->SetMethod("bool is_valid() const", &Graphics::GraphicsDevice::IsValid);
				VGraphicsDevice->SetMethod("void set_vertex_buffer(element_buffer@+)", &Graphics::GraphicsDevice::SetVertexBuffer);
				VGraphicsDevice->SetMethod("void set_shader_cache(bool)", &Graphics::GraphicsDevice::SetShaderCache);
				VGraphicsDevice->SetMethod("void set_vsync_mode(vsync)", &Graphics::GraphicsDevice::SetVSyncMode);
				VGraphicsDevice->SetMethod("bool preprocess(shader_desc &in)", &Graphics::GraphicsDevice::Preprocess);
				VGraphicsDevice->SetMethod("bool transpile(string &out, shader_type, shader_lang)", &Graphics::GraphicsDevice::Transpile);
				VGraphicsDevice->SetMethod("bool add_section(const string_view&in, const string_view&in)", &Graphics::GraphicsDevice::AddSection);
				VGraphicsDevice->SetMethod("bool remove_section(const string_view&in)", &Graphics::GraphicsDevice::RemoveSection);
				VGraphicsDevice->SetMethod("bool get_section_data(const string_view&in, shader_desc &out)", &Graphics::GraphicsDevice::GetSectionData);
				VGraphicsDevice->SetMethod("bool is_left_handed() const", &Graphics::GraphicsDevice::IsLeftHanded);
				VGraphicsDevice->SetMethod("string get_shader_main(shader_type) const", &Graphics::GraphicsDevice::GetShaderMain);
				VGraphicsDevice->SetMethod("depth_stencil_state@+ get_depth_stencil_state(const string_view&in)", &Graphics::GraphicsDevice::GetDepthStencilState);
				VGraphicsDevice->SetMethod("blend_state@+ get_blend_state(const string_view&in)", &Graphics::GraphicsDevice::GetBlendState);
				VGraphicsDevice->SetMethod("rasterizer_state@+ get_rasterizer_state(const string_view&in)", &Graphics::GraphicsDevice::GetRasterizerState);
				VGraphicsDevice->SetMethod("sampler_state@+ get_sampler_state(const string_view&in)", &Graphics::GraphicsDevice::GetSamplerState);
				VGraphicsDevice->SetMethod("input_layout@+ get_input_layout(const string_view&in)", &Graphics::GraphicsDevice::GetInputLayout);
				VGraphicsDevice->SetMethod("shader_model get_shader_model() const", &Graphics::GraphicsDevice::GetShaderModel);
				VGraphicsDevice->SetMethod("render_target_2d@+ get_render_target()", &Graphics::GraphicsDevice::GetRenderTarget);
				VGraphicsDevice->SetMethod("render_backend get_backend() const", &Graphics::GraphicsDevice::GetBackend);
				VGraphicsDevice->SetMethod("uint32 get_format_size(surface_format) const", &Graphics::GraphicsDevice::GetFormatSize);
				VGraphicsDevice->SetMethod("uint32 get_present_flags() const", &Graphics::GraphicsDevice::GetPresentFlags);
				VGraphicsDevice->SetMethod("uint32 get_compile_flags() const", &Graphics::GraphicsDevice::GetCompileFlags);
				VGraphicsDevice->SetMethod("uint32 get_mip_level(uint32, uint32) const", &Graphics::GraphicsDevice::GetMipLevel);
				VGraphicsDevice->SetMethod("vsync get_vsync_mode() const", &Graphics::GraphicsDevice::GetVSyncMode);
				VGraphicsDevice->SetMethod("bool is_debug() const", &Graphics::GraphicsDevice::IsDebug);
				VGraphicsDevice->SetMethodStatic("graphics_device@ create(graphics_device_desc &in)", &GraphicsDeviceCreate);
				VGraphicsDevice->SetMethodStatic("void compile_buildin_shaders(array<graphics_device@>@+)", &GraphicsDeviceCompileBuiltinShaders);
				VGraphicsDevice->SetEnumRefsEx<Graphics::GraphicsDevice>([](Graphics::GraphicsDevice* Base, asIScriptEngine* VM) { });
				VGraphicsDevice->SetReleaseRefsEx<Graphics::GraphicsDevice>([](Graphics::GraphicsDevice* Base, asIScriptEngine*) { });

				VRenderTarget->SetDynamicCast<Graphics::RenderTarget, Graphics::RenderTarget2D>("render_target_2d@+");
				VRenderTarget->SetDynamicCast<Graphics::RenderTarget, Graphics::RenderTargetCube>("render_target_cube@+");
				VRenderTarget->SetDynamicCast<Graphics::RenderTarget, Graphics::MultiRenderTarget2D>("multi_render_target_2d@+");
				VRenderTarget->SetDynamicCast<Graphics::RenderTarget, Graphics::MultiRenderTargetCube>("multi_render_target_cube@+");
				VRenderTarget2D->SetDynamicCast<Graphics::RenderTarget2D, Graphics::RenderTarget>("render_target@+", true);
				VMultiRenderTarget2D->SetDynamicCast<Graphics::MultiRenderTarget2D, Graphics::RenderTarget>("render_target@+", true);
				VRenderTargetCube->SetDynamicCast<Graphics::RenderTargetCube, Graphics::RenderTarget>("render_target@+", true);
				VMultiRenderTargetCube->SetDynamicCast<Graphics::MultiRenderTargetCube, Graphics::RenderTarget>("render_target@+", true);

				return true;
#else
				VI_ASSERT(false, "<graphics> is not loaded");
				return false;
#endif
			}
			bool HeavyRegistry::ImportAudio(VirtualMachine* VM) noexcept
			{
#ifdef VI_BINDINGS
				VI_ASSERT(VM != nullptr, "manager should be set");
				VI_TYPEREF(AudioSource, "audio_source");

				auto VSoundDistanceModel = VM->SetEnum("sound_distance_model");
				VSoundDistanceModel->SetValue("invalid", (int)Audio::SoundDistanceModel::Invalid);
				VSoundDistanceModel->SetValue("invert", (int)Audio::SoundDistanceModel::Invert);
				VSoundDistanceModel->SetValue("invert_clamp", (int)Audio::SoundDistanceModel::Invert_Clamp);
				VSoundDistanceModel->SetValue("linear", (int)Audio::SoundDistanceModel::Linear);
				VSoundDistanceModel->SetValue("linear_clamp", (int)Audio::SoundDistanceModel::Linear_Clamp);
				VSoundDistanceModel->SetValue("exponent", (int)Audio::SoundDistanceModel::Exponent);
				VSoundDistanceModel->SetValue("exponent_clamp", (int)Audio::SoundDistanceModel::Exponent_Clamp);

				auto VSoundEx = VM->SetEnum("sound_ex");
				VSoundEx->SetValue("source_relative", (int)Audio::SoundEx::Source_Relative);
				VSoundEx->SetValue("cone_inner_angle", (int)Audio::SoundEx::Cone_Inner_Angle);
				VSoundEx->SetValue("cone_outer_angle", (int)Audio::SoundEx::Cone_Outer_Angle);
				VSoundEx->SetValue("pitch", (int)Audio::SoundEx::Pitch);
				VSoundEx->SetValue("position", (int)Audio::SoundEx::Position);
				VSoundEx->SetValue("direction", (int)Audio::SoundEx::Direction);
				VSoundEx->SetValue("velocity", (int)Audio::SoundEx::Velocity);
				VSoundEx->SetValue("looping", (int)Audio::SoundEx::Looping);
				VSoundEx->SetValue("buffer", (int)Audio::SoundEx::Buffer);
				VSoundEx->SetValue("gain", (int)Audio::SoundEx::Gain);
				VSoundEx->SetValue("min_gain", (int)Audio::SoundEx::Min_Gain);
				VSoundEx->SetValue("max_gain", (int)Audio::SoundEx::Max_Gain);
				VSoundEx->SetValue("orientation", (int)Audio::SoundEx::Orientation);
				VSoundEx->SetValue("channel_mask", (int)Audio::SoundEx::Channel_Mask);
				VSoundEx->SetValue("source_state", (int)Audio::SoundEx::Source_State);
				VSoundEx->SetValue("initial", (int)Audio::SoundEx::Initial);
				VSoundEx->SetValue("playing", (int)Audio::SoundEx::Playing);
				VSoundEx->SetValue("paused", (int)Audio::SoundEx::Paused);
				VSoundEx->SetValue("stopped", (int)Audio::SoundEx::Stopped);
				VSoundEx->SetValue("buffers_queued", (int)Audio::SoundEx::Buffers_Queued);
				VSoundEx->SetValue("buffers_processed", (int)Audio::SoundEx::Buffers_Processed);
				VSoundEx->SetValue("seconds_offset", (int)Audio::SoundEx::Seconds_Offset);
				VSoundEx->SetValue("sample_offset", (int)Audio::SoundEx::Sample_Offset);
				VSoundEx->SetValue("byte_offset", (int)Audio::SoundEx::Byte_Offset);
				VSoundEx->SetValue("source_type", (int)Audio::SoundEx::Source_Type);
				VSoundEx->SetValue("static", (int)Audio::SoundEx::Static);
				VSoundEx->SetValue("streaming", (int)Audio::SoundEx::Streaming);
				VSoundEx->SetValue("undetermined", (int)Audio::SoundEx::Undetermined);
				VSoundEx->SetValue("format_mono8", (int)Audio::SoundEx::Format_Mono8);
				VSoundEx->SetValue("format_mono16", (int)Audio::SoundEx::Format_Mono16);
				VSoundEx->SetValue("format_stereo8", (int)Audio::SoundEx::Format_Stereo8);
				VSoundEx->SetValue("format_stereo16", (int)Audio::SoundEx::Format_Stereo16);
				VSoundEx->SetValue("reference_distance", (int)Audio::SoundEx::Reference_Distance);
				VSoundEx->SetValue("rolloff_gactor", (int)Audio::SoundEx::Rolloff_Factor);
				VSoundEx->SetValue("cone_outer_gain", (int)Audio::SoundEx::Cone_Outer_Gain);
				VSoundEx->SetValue("max_distance", (int)Audio::SoundEx::Max_Distance);
				VSoundEx->SetValue("frequency", (int)Audio::SoundEx::Frequency);
				VSoundEx->SetValue("bits", (int)Audio::SoundEx::Bits);
				VSoundEx->SetValue("channels", (int)Audio::SoundEx::Channels);
				VSoundEx->SetValue("size", (int)Audio::SoundEx::Size);
				VSoundEx->SetValue("unused", (int)Audio::SoundEx::Unused);
				VSoundEx->SetValue("pending", (int)Audio::SoundEx::Pending);
				VSoundEx->SetValue("processed", (int)Audio::SoundEx::Processed);
				VSoundEx->SetValue("invalid_name", (int)Audio::SoundEx::Invalid_Name);
				VSoundEx->SetValue("illegal_enum", (int)Audio::SoundEx::Illegal_Enum);
				VSoundEx->SetValue("invalid_enum", (int)Audio::SoundEx::Invalid_Enum);
				VSoundEx->SetValue("invalid_value", (int)Audio::SoundEx::Invalid_Value);
				VSoundEx->SetValue("illegal_command", (int)Audio::SoundEx::Illegal_Command);
				VSoundEx->SetValue("invalid_operation", (int)Audio::SoundEx::Invalid_Operation);
				VSoundEx->SetValue("out_of_memory", (int)Audio::SoundEx::Out_Of_Memory);
				VSoundEx->SetValue("vendor", (int)Audio::SoundEx::Vendor);
				VSoundEx->SetValue("version", (int)Audio::SoundEx::Version);
				VSoundEx->SetValue("renderer", (int)Audio::SoundEx::Renderer);
				VSoundEx->SetValue("extentions", (int)Audio::SoundEx::Extentions);
				VSoundEx->SetValue("doppler_factor", (int)Audio::SoundEx::Doppler_Factor);
				VSoundEx->SetValue("doppler_velocity", (int)Audio::SoundEx::Doppler_Velocity);
				VSoundEx->SetValue("speed_of_sound", (int)Audio::SoundEx::Speed_Of_Sound);

				auto VAudioSync = VM->SetPod<Audio::AudioSync>("audio_sync");
				VAudioSync->SetProperty<Audio::AudioSync>("vector3 direction", &Audio::AudioSync::Direction);
				VAudioSync->SetProperty<Audio::AudioSync>("vector3 velocity", &Audio::AudioSync::Velocity);
				VAudioSync->SetProperty<Audio::AudioSync>("float cone_inner_angle", &Audio::AudioSync::ConeInnerAngle);
				VAudioSync->SetProperty<Audio::AudioSync>("float cone_outer_angle", &Audio::AudioSync::ConeOuterAngle);
				VAudioSync->SetProperty<Audio::AudioSync>("float cone_outer_gain", &Audio::AudioSync::ConeOuterGain);
				VAudioSync->SetProperty<Audio::AudioSync>("float pitch", &Audio::AudioSync::Pitch);
				VAudioSync->SetProperty<Audio::AudioSync>("float gain", &Audio::AudioSync::Gain);
				VAudioSync->SetProperty<Audio::AudioSync>("float ref_distance", &Audio::AudioSync::RefDistance);
				VAudioSync->SetProperty<Audio::AudioSync>("float distance", &Audio::AudioSync::Distance);
				VAudioSync->SetProperty<Audio::AudioSync>("float rolloff", &Audio::AudioSync::Rolloff);
				VAudioSync->SetProperty<Audio::AudioSync>("float position", &Audio::AudioSync::Position);
				VAudioSync->SetProperty<Audio::AudioSync>("float air_absorption", &Audio::AudioSync::AirAbsorption);
				VAudioSync->SetProperty<Audio::AudioSync>("float room_roll_off", &Audio::AudioSync::RoomRollOff);
				VAudioSync->SetProperty<Audio::AudioSync>("bool is_relative", &Audio::AudioSync::IsRelative);
				VAudioSync->SetProperty<Audio::AudioSync>("bool is_looped", &Audio::AudioSync::IsLooped);
				VAudioSync->SetConstructor<Audio::AudioSync>("void f()");

				VM->BeginNamespace("audio_context");
				VM->SetFunction("bool initialize()", &VI_SEXPECTIFY_VOID(Audio::AudioContext::Initialize));
				VM->SetFunction("bool generate_buffers(int32, uint32 &out)", &VI_SEXPECTIFY_VOID(Audio::AudioContext::GenerateBuffers));
				VM->SetFunction("bool set_source_data_3f(uint32, sound_ex, float, float, float)", &VI_SEXPECTIFY_VOID(Audio::AudioContext::SetSourceData3F));
				VM->SetFunction("bool get_source_data_3f(uint32, sound_ex, float &out, float &out, float &out)", &VI_SEXPECTIFY_VOID(Audio::AudioContext::GetSourceData3F));
				VM->SetFunction("bool set_source_data_1f(uint32, sound_ex, float)", &VI_SEXPECTIFY_VOID(Audio::AudioContext::SetSourceData1F));
				VM->SetFunction("bool get_source_data_1f(uint32, sound_ex, float &out)", &VI_SEXPECTIFY_VOID(Audio::AudioContext::GetSourceData1F));
				VM->SetFunction("bool set_source_data_3i(uint32, sound_ex, int32, int32, int32)", &VI_SEXPECTIFY_VOID(Audio::AudioContext::SetSourceData3I));
				VM->SetFunction("bool get_source_data_3i(uint32, sound_ex, int32 &out, int32 &out, int32 &out)", &VI_SEXPECTIFY_VOID(Audio::AudioContext::GetSourceData3I));
				VM->SetFunction("bool set_source_data_1i(uint32, sound_ex, int32)", &VI_SEXPECTIFY_VOID(Audio::AudioContext::SetSourceData1I));
				VM->SetFunction("bool get_source_data_1i(uint32, sound_ex, int32 &out)", &VI_SEXPECTIFY_VOID(Audio::AudioContext::GetSourceData1I));
				VM->SetFunction("bool set_listener_data_3f(sound_ex, float, float, float)", &VI_SEXPECTIFY_VOID(Audio::AudioContext::SetListenerData3F));
				VM->SetFunction("bool get_listener_data_3f(sound_ex, float &out, float &out, float &out)", &VI_SEXPECTIFY_VOID(Audio::AudioContext::GetListenerData3F));
				VM->SetFunction("bool set_listener_data_1f(sound_ex, float)", &VI_SEXPECTIFY_VOID(Audio::AudioContext::SetListenerData1F));
				VM->SetFunction("bool get_listener_data_1f(sound_ex, float &out)", &VI_SEXPECTIFY_VOID(Audio::AudioContext::GetListenerData1F));
				VM->SetFunction("bool set_listener_data_3i(sound_ex, int32, int32, int32)", &VI_SEXPECTIFY_VOID(Audio::AudioContext::SetListenerData3I));
				VM->SetFunction("bool get_listener_data_3i(sound_ex, int32 &out, int32 &out, int32 &out)", &VI_SEXPECTIFY_VOID(Audio::AudioContext::GetListenerData3I));
				VM->SetFunction("bool set_listener_data_1i(sound_ex, int32)", &VI_SEXPECTIFY_VOID(Audio::AudioContext::SetListenerData1I));
				VM->SetFunction("bool get_listener_data_1i(sound_ex, int32 &out)", &VI_SEXPECTIFY_VOID(Audio::AudioContext::GetListenerData1I));
				VM->EndNamespace();

				auto VAudioSource = VM->SetClass<Audio::AudioSource>("audio_source", true);
				auto VAudioFilter = VM->SetClass<Audio::AudioFilter>("base_audio_filter", false);
				PopulateAudioFilterBase<Audio::AudioFilter>(*VAudioFilter);

				auto VAudioEffect = VM->SetClass<Audio::AudioEffect>("base_audio_effect", false);
				PopulateAudioEffectBase<Audio::AudioEffect>(*VAudioEffect);

				auto VAudioClip = VM->SetClass<Audio::AudioClip>("audio_clip", false);
				VAudioClip->SetConstructor<Audio::AudioClip, int, int>("audio_clip@ f(int, int)");
				VAudioClip->SetMethod("float length() const", &Audio::AudioClip::Length);
				VAudioClip->SetMethod("bool is_mono() const", &Audio::AudioClip::IsMono);
				VAudioClip->SetMethod("uint32 get_buffer() const", &Audio::AudioClip::GetBuffer);
				VAudioClip->SetMethod("int32 get_format() const", &Audio::AudioClip::GetFormat);

				VAudioSource->SetGcConstructor<Audio::AudioSource, AudioSource>("audio_source@ f()");
				VAudioSource->SetMethod("int64 add_effect(base_audio_effect@+)", &Audio::AudioSource::AddEffect);
				VAudioSource->SetMethodEx("bool remove_effect(usize)", &VI_EXPECTIFY_VOID(Audio::AudioSource::RemoveEffect));
				VAudioSource->SetMethodEx("bool remove_effect_by_id(uint64)", &VI_EXPECTIFY_VOID(Audio::AudioSource::RemoveEffectById));
				VAudioSource->SetMethodEx("bool set_clip(audio_clip@+)", &VI_EXPECTIFY_VOID(Audio::AudioSource::SetClip));
				VAudioSource->SetMethodEx("bool synchronize(audio_sync &in, const vector3 &in)", &VI_EXPECTIFY_VOID(Audio::AudioSource::Synchronize));
				VAudioSource->SetMethodEx("bool reset()", &VI_EXPECTIFY_VOID(Audio::AudioSource::Reset));
				VAudioSource->SetMethodEx("bool pause()", &VI_EXPECTIFY_VOID(Audio::AudioSource::Pause));
				VAudioSource->SetMethodEx("bool play()", &VI_EXPECTIFY_VOID(Audio::AudioSource::Play));
				VAudioSource->SetMethodEx("bool stop()", &VI_EXPECTIFY_VOID(Audio::AudioSource::Stop));
				VAudioSource->SetMethod("bool is_playing() const", &Audio::AudioSource::IsPlaying);
				VAudioSource->SetMethod("usize get_effects_count() const", &Audio::AudioSource::GetEffectsCount);
				VAudioSource->SetMethod("uint32 get_instance() const", &Audio::AudioSource::GetInstance);
				VAudioSource->SetMethod("audio_clip@+ get_clip() const", &Audio::AudioSource::GetClip);
				VAudioSource->SetMethod<Audio::AudioSource, Audio::AudioEffect*, uint64_t>("base_audio_effect@+ get_effect(uint64) const", &Audio::AudioSource::GetEffect);
				VAudioSource->SetEnumRefsEx<Audio::AudioSource>([](Audio::AudioSource* Base, asIScriptEngine* VM)
				{
					for (auto* Item : Base->GetEffects())
						FunctionFactory::GCEnumCallback(VM, Item);
				});
				VAudioSource->SetReleaseRefsEx<Audio::AudioSource>([](Audio::AudioSource* Base, asIScriptEngine*)
				{
					Base->RemoveEffects();
				});

				auto VAudioDevice = VM->SetClass<Audio::AudioDevice>("audio_device", false);
				VAudioDevice->SetConstructor<Audio::AudioDevice>("audio_device@ f()");
				VAudioDevice->SetMethodEx("bool offset(audio_source@+, float &out, bool)", &VI_EXPECTIFY_VOID(Audio::AudioDevice::Offset));
				VAudioDevice->SetMethodEx("bool velocity(audio_source@+, vector3 &out, bool)", &VI_EXPECTIFY_VOID(Audio::AudioDevice::Velocity));
				VAudioDevice->SetMethodEx("bool position(audio_source@+, vector3 &out, bool)", &VI_EXPECTIFY_VOID(Audio::AudioDevice::Position));
				VAudioDevice->SetMethodEx("bool direction(audio_source@+, vector3 &out, bool)", &VI_EXPECTIFY_VOID(Audio::AudioDevice::Direction));
				VAudioDevice->SetMethodEx("bool relative(audio_source@+, int &out, bool)", &VI_EXPECTIFY_VOID(Audio::AudioDevice::Relative));
				VAudioDevice->SetMethodEx("bool pitch(audio_source@+, float &out, bool)", &VI_EXPECTIFY_VOID(Audio::AudioDevice::Pitch));
				VAudioDevice->SetMethodEx("bool gain(audio_source@+, float &out, bool)", &VI_EXPECTIFY_VOID(Audio::AudioDevice::Gain));
				VAudioDevice->SetMethodEx("bool loop(audio_source@+, int &out, bool)", &VI_EXPECTIFY_VOID(Audio::AudioDevice::Loop));
				VAudioDevice->SetMethodEx("bool cone_inner_angle(audio_source@+, float &out, bool)", &VI_EXPECTIFY_VOID(Audio::AudioDevice::ConeInnerAngle));
				VAudioDevice->SetMethodEx("bool cone_outer_angle(audio_source@+, float &out, bool)", &VI_EXPECTIFY_VOID(Audio::AudioDevice::ConeOuterAngle));
				VAudioDevice->SetMethodEx("bool cone_outer_gain(audio_source@+, float &out, bool)", &VI_EXPECTIFY_VOID(Audio::AudioDevice::ConeOuterGain));
				VAudioDevice->SetMethodEx("bool distance(audio_source@+, float &out, bool)", &VI_EXPECTIFY_VOID(Audio::AudioDevice::Distance));
				VAudioDevice->SetMethodEx("bool ref_distance(audio_source@+, float &out, bool)", &VI_EXPECTIFY_VOID(Audio::AudioDevice::RefDistance));
				VAudioDevice->SetMethodEx("bool set_distance_model(sound_distance_model)", &VI_EXPECTIFY_VOID(Audio::AudioDevice::SetDistanceModel));
				VAudioDevice->SetMethod("void display_audio_log() const", &Audio::AudioDevice::DisplayAudioLog);
				VAudioDevice->SetMethod("bool is_valid() const", &Audio::AudioDevice::IsValid);

				return true;
#else
				VI_ASSERT(false, "<audio> is not loaded");
				return false;
#endif
			}
			bool HeavyRegistry::ImportAudioEffects(VirtualMachine* VM) noexcept
			{
#ifdef VI_BINDINGS
				VI_ASSERT(VM != nullptr, "manager should be set");

				auto VReverb = VM->SetClass<Audio::Effects::Reverb>("reverb_effect", false);
				VReverb->SetProperty<Audio::Effects::Reverb>("vector3 late_reverb_pan", &Audio::Effects::Reverb::LateReverbPan);
				VReverb->SetProperty<Audio::Effects::Reverb>("vector3 reflections_pan", &Audio::Effects::Reverb::ReflectionsPan);
				VReverb->SetProperty<Audio::Effects::Reverb>("float density", &Audio::Effects::Reverb::Density);
				VReverb->SetProperty<Audio::Effects::Reverb>("float diffusion", &Audio::Effects::Reverb::Diffusion);
				VReverb->SetProperty<Audio::Effects::Reverb>("float gain", &Audio::Effects::Reverb::Gain);
				VReverb->SetProperty<Audio::Effects::Reverb>("float gain_hf", &Audio::Effects::Reverb::GainHF);
				VReverb->SetProperty<Audio::Effects::Reverb>("float gain_lf", &Audio::Effects::Reverb::GainLF);
				VReverb->SetProperty<Audio::Effects::Reverb>("float decay_time", &Audio::Effects::Reverb::DecayTime);
				VReverb->SetProperty<Audio::Effects::Reverb>("float decay_hf_ratio", &Audio::Effects::Reverb::DecayHFRatio);
				VReverb->SetProperty<Audio::Effects::Reverb>("float decay_lf_ratio", &Audio::Effects::Reverb::DecayLFRatio);
				VReverb->SetProperty<Audio::Effects::Reverb>("float reflections_gain", &Audio::Effects::Reverb::ReflectionsGain);
				VReverb->SetProperty<Audio::Effects::Reverb>("float reflections_delay", &Audio::Effects::Reverb::ReflectionsDelay);
				VReverb->SetProperty<Audio::Effects::Reverb>("float late_reverb_gain", &Audio::Effects::Reverb::LateReverbGain);
				VReverb->SetProperty<Audio::Effects::Reverb>("float late_reverb_delay", &Audio::Effects::Reverb::LateReverbDelay);
				VReverb->SetProperty<Audio::Effects::Reverb>("float echo_time", &Audio::Effects::Reverb::EchoTime);
				VReverb->SetProperty<Audio::Effects::Reverb>("float echo_depth", &Audio::Effects::Reverb::EchoDepth);
				VReverb->SetProperty<Audio::Effects::Reverb>("float modulation_time", &Audio::Effects::Reverb::ModulationTime);
				VReverb->SetProperty<Audio::Effects::Reverb>("float modulation_depth", &Audio::Effects::Reverb::ModulationDepth);
				VReverb->SetProperty<Audio::Effects::Reverb>("float air_absorption_gain_hf", &Audio::Effects::Reverb::AirAbsorptionGainHF);
				VReverb->SetProperty<Audio::Effects::Reverb>("float hf_reference", &Audio::Effects::Reverb::HFReference);
				VReverb->SetProperty<Audio::Effects::Reverb>("float lf_reference", &Audio::Effects::Reverb::LFReference);
				VReverb->SetProperty<Audio::Effects::Reverb>("float room_rolloff_factor", &Audio::Effects::Reverb::RoomRolloffFactor);
				VReverb->SetProperty<Audio::Effects::Reverb>("bool is_decay_hf_limited", &Audio::Effects::Reverb::IsDecayHFLimited);
				PopulateAudioEffectInterface<Audio::Effects::Reverb>(*VReverb, "reverb_effect@+ f()");

				auto VChorus = VM->SetClass<Audio::Effects::Chorus>("chorus_effect", false);
				VChorus->SetProperty<Audio::Effects::Chorus>("float rate", &Audio::Effects::Chorus::Rate);
				VChorus->SetProperty<Audio::Effects::Chorus>("float depth", &Audio::Effects::Chorus::Depth);
				VChorus->SetProperty<Audio::Effects::Chorus>("float feedback", &Audio::Effects::Chorus::Feedback);
				VChorus->SetProperty<Audio::Effects::Chorus>("float delay", &Audio::Effects::Chorus::Delay);
				VChorus->SetProperty<Audio::Effects::Chorus>("int32 waveform", &Audio::Effects::Chorus::Waveform);
				VChorus->SetProperty<Audio::Effects::Chorus>("int32 phase", &Audio::Effects::Chorus::Phase);
				PopulateAudioEffectInterface<Audio::Effects::Chorus>(*VChorus, "chorus_effect@+ f()");

				auto VDistortion = VM->SetClass<Audio::Effects::Distortion>("distortion_effect", false);
				VDistortion->SetProperty<Audio::Effects::Distortion>("float edge", &Audio::Effects::Distortion::Edge);
				VDistortion->SetProperty<Audio::Effects::Distortion>("float gain", &Audio::Effects::Distortion::Gain);
				VDistortion->SetProperty<Audio::Effects::Distortion>("float lowpass_cut_off", &Audio::Effects::Distortion::LowpassCutOff);
				VDistortion->SetProperty<Audio::Effects::Distortion>("float eq_center", &Audio::Effects::Distortion::EQCenter);
				VDistortion->SetProperty<Audio::Effects::Distortion>("float eq_bandwidth", &Audio::Effects::Distortion::EQBandwidth);
				PopulateAudioEffectInterface<Audio::Effects::Distortion>(*VDistortion, "distortion_effect@+ f()");

				auto VEcho = VM->SetClass<Audio::Effects::Echo>("echo_effect", false);
				VEcho->SetProperty<Audio::Effects::Echo>("float delay", &Audio::Effects::Echo::Delay);
				VEcho->SetProperty<Audio::Effects::Echo>("float lr_delay", &Audio::Effects::Echo::LRDelay);
				VEcho->SetProperty<Audio::Effects::Echo>("float damping", &Audio::Effects::Echo::Damping);
				VEcho->SetProperty<Audio::Effects::Echo>("float feedback", &Audio::Effects::Echo::Feedback);
				VEcho->SetProperty<Audio::Effects::Echo>("float spread", &Audio::Effects::Echo::Spread);
				PopulateAudioEffectInterface<Audio::Effects::Echo>(*VEcho, "echo_effect@+ f()");

				auto VFlanger = VM->SetClass<Audio::Effects::Flanger>("flanger_effect", false);
				VFlanger->SetProperty<Audio::Effects::Flanger>("float rate", &Audio::Effects::Flanger::Rate);
				VFlanger->SetProperty<Audio::Effects::Flanger>("float depth", &Audio::Effects::Flanger::Depth);
				VFlanger->SetProperty<Audio::Effects::Flanger>("float feedback", &Audio::Effects::Flanger::Feedback);
				VFlanger->SetProperty<Audio::Effects::Flanger>("float delay", &Audio::Effects::Flanger::Delay);
				VFlanger->SetProperty<Audio::Effects::Flanger>("int32 waveform", &Audio::Effects::Flanger::Waveform);
				VFlanger->SetProperty<Audio::Effects::Flanger>("int32 phase", &Audio::Effects::Flanger::Phase);
				PopulateAudioEffectInterface<Audio::Effects::Flanger>(*VFlanger, "flanger_effect@+ f()");

				auto VFrequencyShifter = VM->SetClass<Audio::Effects::FrequencyShifter>("frequency_shifter_effect", false);
				VFrequencyShifter->SetProperty<Audio::Effects::FrequencyShifter>("float frequency", &Audio::Effects::FrequencyShifter::Frequency);
				VFrequencyShifter->SetProperty<Audio::Effects::FrequencyShifter>("int32 left_direction", &Audio::Effects::FrequencyShifter::LeftDirection);
				VFrequencyShifter->SetProperty<Audio::Effects::FrequencyShifter>("int32 right_direction", &Audio::Effects::FrequencyShifter::RightDirection);
				PopulateAudioEffectInterface<Audio::Effects::FrequencyShifter>(*VFrequencyShifter, "frequency_shifter_effect@+ f()");

				auto VVocalMorpher = VM->SetClass<Audio::Effects::VocalMorpher>("vocal_morpher_effect", false);
				VVocalMorpher->SetProperty<Audio::Effects::VocalMorpher>("float rate", &Audio::Effects::VocalMorpher::Rate);
				VVocalMorpher->SetProperty<Audio::Effects::VocalMorpher>("int32 phonemea", &Audio::Effects::VocalMorpher::Phonemea);
				VVocalMorpher->SetProperty<Audio::Effects::VocalMorpher>("int32 phonemea_coarse_tuning", &Audio::Effects::VocalMorpher::PhonemeaCoarseTuning);
				VVocalMorpher->SetProperty<Audio::Effects::VocalMorpher>("int32 phonemeb", &Audio::Effects::VocalMorpher::Phonemeb);
				VVocalMorpher->SetProperty<Audio::Effects::VocalMorpher>("int32 phonemeb_coarse_tuning", &Audio::Effects::VocalMorpher::PhonemebCoarseTuning);
				VVocalMorpher->SetProperty<Audio::Effects::VocalMorpher>("int32 waveform", &Audio::Effects::VocalMorpher::Waveform);
				PopulateAudioEffectInterface<Audio::Effects::VocalMorpher>(*VVocalMorpher, "vocal_morpher_effect@+ f()");

				auto VPitchShifter = VM->SetClass<Audio::Effects::PitchShifter>("pitch_shifter_effect", false);
				VPitchShifter->SetProperty<Audio::Effects::PitchShifter>("int32 coarse_tune", &Audio::Effects::PitchShifter::CoarseTune);
				VPitchShifter->SetProperty<Audio::Effects::PitchShifter>("int32 fine_tune", &Audio::Effects::PitchShifter::FineTune);
				PopulateAudioEffectInterface<Audio::Effects::PitchShifter>(*VPitchShifter, "pitch_shifter_effect@+ f()");

				auto VRingModulator = VM->SetClass<Audio::Effects::RingModulator>("ring_modulator_effect", false);
				VRingModulator->SetProperty<Audio::Effects::RingModulator>("float frequency", &Audio::Effects::RingModulator::Frequency);
				VRingModulator->SetProperty<Audio::Effects::RingModulator>("float highpass_cut_off", &Audio::Effects::RingModulator::HighpassCutOff);
				VRingModulator->SetProperty<Audio::Effects::RingModulator>("int32 waveform", &Audio::Effects::RingModulator::Waveform);
				PopulateAudioEffectInterface<Audio::Effects::RingModulator>(*VRingModulator, "ring_modulator_effect@+ f()");

				auto VAutowah = VM->SetClass<Audio::Effects::Autowah>("autowah_effect", false);
				VAutowah->SetProperty<Audio::Effects::Autowah>("float attack_time", &Audio::Effects::Autowah::AttackTime);
				VAutowah->SetProperty<Audio::Effects::Autowah>("float release_time", &Audio::Effects::Autowah::ReleaseTime);
				VAutowah->SetProperty<Audio::Effects::Autowah>("float resonance", &Audio::Effects::Autowah::Resonance);
				VAutowah->SetProperty<Audio::Effects::Autowah>("float peak_gain", &Audio::Effects::Autowah::PeakGain);
				PopulateAudioEffectInterface<Audio::Effects::Autowah>(*VAutowah, "autowah_effect@+ f()");

				auto VCompressor = VM->SetClass<Audio::Effects::Compressor>("compressor_effect", false);
				PopulateAudioEffectInterface<Audio::Effects::Compressor>(*VCompressor, "compressor_effect@+ f()");

				auto VEqualizer = VM->SetClass<Audio::Effects::Equalizer>("equalizer_effect", false);
				VEqualizer->SetProperty<Audio::Effects::Equalizer>("float low_gain", &Audio::Effects::Equalizer::LowGain);
				VEqualizer->SetProperty<Audio::Effects::Equalizer>("float low_cut_off", &Audio::Effects::Equalizer::LowCutOff);
				VEqualizer->SetProperty<Audio::Effects::Equalizer>("float mid1_gain", &Audio::Effects::Equalizer::Mid1Gain);
				VEqualizer->SetProperty<Audio::Effects::Equalizer>("float mid1_center", &Audio::Effects::Equalizer::Mid1Center);
				VEqualizer->SetProperty<Audio::Effects::Equalizer>("float mid1_width", &Audio::Effects::Equalizer::Mid1Width);
				VEqualizer->SetProperty<Audio::Effects::Equalizer>("float mid2_gain", &Audio::Effects::Equalizer::Mid2Gain);
				VEqualizer->SetProperty<Audio::Effects::Equalizer>("float mid2_center", &Audio::Effects::Equalizer::Mid2Center);
				VEqualizer->SetProperty<Audio::Effects::Equalizer>("float mid2_width", &Audio::Effects::Equalizer::Mid2Width);
				VEqualizer->SetProperty<Audio::Effects::Equalizer>("float high_gain", &Audio::Effects::Equalizer::HighGain);
				VEqualizer->SetProperty<Audio::Effects::Equalizer>("float high_cut_off", &Audio::Effects::Equalizer::HighCutOff);
				PopulateAudioEffectInterface<Audio::Effects::Equalizer>(*VEqualizer, "equalizer_effect@+ f()");

				return true;
#else
				VI_ASSERT(false, "<audio/effects> is not loaded");
				return false;
#endif
			}
			bool HeavyRegistry::ImportAudioFilters(VirtualMachine* VM) noexcept
			{
#ifdef VI_BINDINGS
				VI_ASSERT(VM != nullptr, "manager should be set");

				auto VLowpass = VM->SetClass<Audio::Filters::Lowpass>("lowpass_filter", false);
				VLowpass->SetProperty<Audio::Filters::Lowpass>("float gain_hf", &Audio::Filters::Lowpass::GainHF);
				VLowpass->SetProperty<Audio::Filters::Lowpass>("float gain", &Audio::Filters::Lowpass::Gain);
				PopulateAudioFilterInterface<Audio::Filters::Lowpass>(*VLowpass, "lowpass_filter@+ f()");

				auto VHighpass = VM->SetClass<Audio::Filters::Highpass>("highpass_filter", false);
				VHighpass->SetProperty<Audio::Filters::Highpass>("float gain_lf", &Audio::Filters::Highpass::GainLF);
				VHighpass->SetProperty<Audio::Filters::Highpass>("float gain", &Audio::Filters::Highpass::Gain);
				PopulateAudioFilterInterface<Audio::Filters::Highpass>(*VHighpass, "highpass_filter@+ f()");

				auto VBandpass = VM->SetClass<Audio::Filters::Bandpass>("bandpass_filter", false);
				VBandpass->SetProperty<Audio::Filters::Bandpass>("float gain_hf", &Audio::Filters::Bandpass::GainHF);
				VBandpass->SetProperty<Audio::Filters::Bandpass>("float gain_lf", &Audio::Filters::Bandpass::GainLF);
				VBandpass->SetProperty<Audio::Filters::Bandpass>("float gain", &Audio::Filters::Bandpass::Gain);
				PopulateAudioFilterInterface<Audio::Filters::Bandpass>(*VBandpass, "bandpass_filter@+ f()");

				return true;
#else
				VI_ASSERT(false, "<audio/filters> is not loaded");
				return false;
#endif
			}
			bool HeavyRegistry::ImportEngine(VirtualMachine* VM) noexcept
			{
#ifdef VI_BINDINGS
				VI_ASSERT(VM != nullptr, "manager should be set");
				VI_TYPEREF(Material, "material");
				VI_TYPEREF(Model, "model");
				VI_TYPEREF(SkinModel, "skin_model");
				VI_TYPEREF(RenderSystem, "render_system");
				VI_TYPEREF(ShaderCache, "shader_cache");
				VI_TYPEREF(PrimitiveCache, "primitive_cache");
				VI_TYPEREF(SceneGraph, "scene_graph");
				VI_TYPEREF(HeavyApplicationName, "heavy_application");

				auto VHeavyApplicationUse = VM->SetEnum("heavy_application_use");
				VHeavyApplicationUse->SetValue("scripting", (int)Layer::USE_SCRIPTING);
				VHeavyApplicationUse->SetValue("processing", (int)Layer::USE_PROCESSING);
				VHeavyApplicationUse->SetValue("networking", (int)Layer::USE_NETWORKING);
				VHeavyApplicationUse->SetValue("graphics", (int)Layer::USE_GRAPHICS);
				VHeavyApplicationUse->SetValue("activity", (int)Layer::USE_ACTIVITY);
				VHeavyApplicationUse->SetValue("audio", (int)Layer::USE_AUDIO);

				auto VRenderOpt = VM->SetEnum("render_opt");
				VRenderOpt->SetValue("none_t", (int)Layer::RenderOpt::None);
				VRenderOpt->SetValue("transparent_t", (int)Layer::RenderOpt::Transparent);
				VRenderOpt->SetValue("static_t", (int)Layer::RenderOpt::Static);
				VRenderOpt->SetValue("additive_t", (int)Layer::RenderOpt::Additive);
				VRenderOpt->SetValue("backfaces_t", (int)Layer::RenderOpt::Backfaces);

				auto VRenderCulling = VM->SetEnum("render_culling");
				VRenderCulling->SetValue("linear_t", (int)Layer::RenderCulling::Linear);
				VRenderCulling->SetValue("cubic_t", (int)Layer::RenderCulling::Cubic);
				VRenderCulling->SetValue("disable_t", (int)Layer::RenderCulling::Disable);

				auto VRenderState = VM->SetEnum("render_state");
				VRenderState->SetValue("geometric_t", (int)Layer::RenderState::Geometric);
				VRenderState->SetValue("voxelization_t", (int)Layer::RenderState::Voxelization);
				VRenderState->SetValue("linearization_t", (int)Layer::RenderState::Linearization);
				VRenderState->SetValue("cubic_t", (int)Layer::RenderState::Cubic);

				auto VGeoCategory = VM->SetEnum("geo_category");
				VGeoCategory->SetValue("opaque_t", (int)Layer::GeoCategory::Opaque);
				VGeoCategory->SetValue("transparent_t", (int)Layer::GeoCategory::Transparent);
				VGeoCategory->SetValue("additive_t", (int)Layer::GeoCategory::Additive);
				VGeoCategory->SetValue("count_t", (int)Layer::GeoCategory::Count);

				auto VBufferType = VM->SetEnum("buffer_type");
				VBufferType->SetValue("index_t", (int)Layer::BufferType::Index);
				VBufferType->SetValue("vertex_t", (int)Layer::BufferType::Vertex);

				auto VTargetType = VM->SetEnum("target_type");
				VTargetType->SetValue("main_t", (int)Layer::TargetType::Main);
				VTargetType->SetValue("secondary_t", (int)Layer::TargetType::Secondary);
				VTargetType->SetValue("count_t", (int)Layer::TargetType::Count);

				auto VVoxelType = VM->SetEnum("voxel_type");
				VVoxelType->SetValue("diffuse_t", (int)Layer::VoxelType::Diffuse);
				VVoxelType->SetValue("normal_t", (int)Layer::VoxelType::Normal);
				VVoxelType->SetValue("surface_t", (int)Layer::VoxelType::Surface);

				auto VEventTarget = VM->SetEnum("event_target");
				VEventTarget->SetValue("scene_target", (int)Layer::EventTarget::Scene);
				VEventTarget->SetValue("entity_target", (int)Layer::EventTarget::Entity);
				VEventTarget->SetValue("component_target", (int)Layer::EventTarget::Component);
				VEventTarget->SetValue("listener_target", (int)Layer::EventTarget::Listener);

				auto VActorSet = VM->SetEnum("actor_set");
				VActorSet->SetValue("none_t", (int)Layer::ActorSet::None);
				VActorSet->SetValue("update_t", (int)Layer::ActorSet::Update);
				VActorSet->SetValue("synchronize_t", (int)Layer::ActorSet::Synchronize);
				VActorSet->SetValue("animate_t", (int)Layer::ActorSet::Animate);
				VActorSet->SetValue("message_t", (int)Layer::ActorSet::Message);
				VActorSet->SetValue("cullable_t", (int)Layer::ActorSet::Cullable);
				VActorSet->SetValue("drawable_t", (int)Layer::ActorSet::Drawable);

				auto VActorType = VM->SetEnum("actor_type");
				VActorType->SetValue("update_t", (int)Layer::ActorType::Update);
				VActorType->SetValue("synchronize_t", (int)Layer::ActorType::Synchronize);
				VActorType->SetValue("animate_t", (int)Layer::ActorType::Animate);
				VActorType->SetValue("message_t", (int)Layer::ActorType::Message);
				VActorType->SetValue("count_t", (int)Layer::ActorType::Count);

				auto VComposerTag = VM->SetEnum("composer_tag");
				VComposerTag->SetValue("component_t", (int)Layer::ComposerTag::Component);
				VComposerTag->SetValue("renderer_t", (int)Layer::ComposerTag::Renderer);
				VComposerTag->SetValue("effect_t", (int)Layer::ComposerTag::Effect);
				VComposerTag->SetValue("filter_t", (int)Layer::ComposerTag::Filter);

				auto VRenderBufferType = VM->SetEnum("render_buffer_type");
				VRenderBufferType->SetValue("Animation", (int)Layer::RenderBufferType::Animation);
				VRenderBufferType->SetValue("Render", (int)Layer::RenderBufferType::Render);
				VRenderBufferType->SetValue("View", (int)Layer::RenderBufferType::View);

				auto VPoseNode = VM->SetPod<Layer::PoseNode>("pose_node");
				VPoseNode->SetProperty<Layer::PoseNode>("vector3 position", &Layer::PoseNode::Position);
				VPoseNode->SetProperty<Layer::PoseNode>("vector3 scale", &Layer::PoseNode::Scale);
				VPoseNode->SetProperty<Layer::PoseNode>("quaternion rotation", &Layer::PoseNode::Rotation);
				VPoseNode->SetConstructor<Layer::PoseNode>("void f()");

				auto VPoseData = VM->SetPod<Layer::PoseData>("pose_data");
				VPoseData->SetProperty<Layer::PoseData>("pose_node frame_pose", &Layer::PoseData::Frame);
				VPoseData->SetProperty<Layer::PoseData>("pose_node offset_pose", &Layer::PoseData::Offset);
				VPoseData->SetProperty<Layer::PoseData>("pose_node default_pose", &Layer::PoseData::Default);
				VPoseData->SetConstructor<Layer::PoseData>("void f()");

				auto VAnimationBuffer = VM->SetPod<Layer::AnimationBuffer>("animation_buffer");
				VAnimationBuffer->SetProperty<Layer::AnimationBuffer>("vector3 padding", &Layer::AnimationBuffer::Padding);
				VAnimationBuffer->SetProperty<Layer::AnimationBuffer>("float animated", &Layer::AnimationBuffer::Animated);
				VAnimationBuffer->SetConstructor<Layer::AnimationBuffer>("void f()");
				VAnimationBuffer->SetOperatorEx(Operators::Index, (uint32_t)Position::Left, "matrix4x4&", "usize", &AnimationBufferGetOffsets);
				VAnimationBuffer->SetOperatorEx(Operators::Index, (uint32_t)Position::Const, "const matrix4x4&", "usize", &AnimationBufferGetOffsets);

				auto VRenderBufferInstance = VM->SetPod<Layer::RenderBuffer::Instance>("render_buffer_instance");
				VRenderBufferInstance->SetProperty<Layer::RenderBuffer::Instance>("matrix4x4 transform", &Layer::RenderBuffer::Instance::Transform);
				VRenderBufferInstance->SetProperty<Layer::RenderBuffer::Instance>("matrix4x4 world", &Layer::RenderBuffer::Instance::World);
				VRenderBufferInstance->SetProperty<Layer::RenderBuffer::Instance>("vector2 texcoord", &Layer::RenderBuffer::Instance::TexCoord);
				VRenderBufferInstance->SetProperty<Layer::RenderBuffer::Instance>("float diffuse", &Layer::RenderBuffer::Instance::Diffuse);
				VRenderBufferInstance->SetProperty<Layer::RenderBuffer::Instance>("float normal", &Layer::RenderBuffer::Instance::Normal);
				VRenderBufferInstance->SetProperty<Layer::RenderBuffer::Instance>("float height", &Layer::RenderBuffer::Instance::Height);
				VRenderBufferInstance->SetProperty<Layer::RenderBuffer::Instance>("float material_id", &Layer::RenderBuffer::Instance::MaterialId);
				VRenderBufferInstance->SetConstructor<Layer::RenderBuffer::Instance>("void f()");

				auto VRenderBuffer = VM->SetPod<Layer::RenderBuffer>("render_buffer");
				VRenderBuffer->SetProperty<Layer::RenderBuffer>("matrix4x4 transform", &Layer::RenderBuffer::Transform);
				VRenderBuffer->SetProperty<Layer::RenderBuffer>("matrix4x4 world", &Layer::RenderBuffer::World);
				VRenderBuffer->SetProperty<Layer::RenderBuffer>("vector4 texcoord", &Layer::RenderBuffer::TexCoord);
				VRenderBuffer->SetProperty<Layer::RenderBuffer>("float diffuse", &Layer::RenderBuffer::Diffuse);
				VRenderBuffer->SetProperty<Layer::RenderBuffer>("float normal", &Layer::RenderBuffer::Normal);
				VRenderBuffer->SetProperty<Layer::RenderBuffer>("float height", &Layer::RenderBuffer::Height);
				VRenderBuffer->SetProperty<Layer::RenderBuffer>("float material_id", &Layer::RenderBuffer::MaterialId);
				VRenderBuffer->SetConstructor<Layer::RenderBuffer>("void f()");

				auto VViewBuffer = VM->SetPod<Layer::ViewBuffer>("view_buffer");
				VViewBuffer->SetProperty<Layer::ViewBuffer>("matrix4x4 inv_view_proj", &Layer::ViewBuffer::InvViewProj);
				VViewBuffer->SetProperty<Layer::ViewBuffer>("matrix4x4 view_proj", &Layer::ViewBuffer::ViewProj);
				VViewBuffer->SetProperty<Layer::ViewBuffer>("matrix4x4 proj", &Layer::ViewBuffer::Proj);
				VViewBuffer->SetProperty<Layer::ViewBuffer>("matrix4x4 view", &Layer::ViewBuffer::View);
				VViewBuffer->SetProperty<Layer::ViewBuffer>("vector3 position", &Layer::ViewBuffer::Position);
				VViewBuffer->SetProperty<Layer::ViewBuffer>("float far", &Layer::ViewBuffer::Far);
				VViewBuffer->SetProperty<Layer::ViewBuffer>("vector3 direction", &Layer::ViewBuffer::Direction);
				VViewBuffer->SetProperty<Layer::ViewBuffer>("float near", &Layer::ViewBuffer::Near);
				VViewBuffer->SetConstructor<Layer::ViewBuffer>("void f()");

				auto VSkinModel = VM->SetClass<Layer::SkinModel>("skin_model", true);
				auto VPoseBuffer = VM->SetStructTrivial<Layer::PoseBuffer>("pose_buffer");
				VPoseBuffer->SetMethodEx("void set_offset(int64, const pose_data &in)", &PoseBufferSetOffset);
				VPoseBuffer->SetMethodEx("void set_matrix(skin_mesh_buffer@+, usize, const matrix4x4 &in)", &PoseBufferSetMatrix);
				VPoseBuffer->SetMethodEx("pose_data& get_offset(int64)", &PoseBufferGetOffset);
				VPoseBuffer->SetMethodEx("matrix4x4& get_matrix(skin_mesh_buffer@+, usize)", &PoseBufferGetMatrix);
				VPoseBuffer->SetMethodEx("usize get_offsets_size()", &PoseBufferGetOffsetsSize);
				VPoseBuffer->SetMethodEx("usize get_matrices_size(skin_mesh_buffer@+)", &PoseBufferGetMatricesSize);
				VPoseBuffer->SetConstructor<Layer::PoseBuffer>("void f()");

				auto VTicker = VM->SetStructTrivial<Layer::Ticker>("clock_ticker");
				VTicker->SetProperty<Layer::Ticker>("float delay", &Layer::Ticker::Delay);
				VTicker->SetConstructor<Layer::Ticker>("void f()");
				VTicker->SetMethod("bool tick_event(float)", &Layer::Ticker::TickEvent);
				VTicker->SetMethod("float get_time() const", &Layer::Ticker::GetTime);

				auto VEvent = VM->SetStructTrivial<Layer::Event>("scene_event");
				VEvent->SetProperty<Layer::Event>("string name", &Layer::Event::Name);
				VEvent->SetConstructor<Layer::Event, const std::string_view&>("void f(const string_view&in)");
				VEvent->SetMethodEx("void set_args(dictionary@+)", &EventSetArgs);
				VEvent->SetMethodEx("dictionary@ get_args() const", &EventGetArgs);

				auto VMaterial = VM->SetClass<Layer::Material>("material", true);
				auto VBatchData = VM->SetStructTrivial<Layer::BatchData>("batch_data");
				VBatchData->SetProperty<Layer::BatchData>("element_buffer@ instances_buffer", &Layer::BatchData::InstanceBuffer);
				VBatchData->SetProperty<Layer::BatchData>("uptr@ geometry_buffer", &Layer::BatchData::GeometryBuffer);
				VBatchData->SetProperty<Layer::BatchData>("material@ batch_material", &Layer::BatchData::BatchMaterial);
				VBatchData->SetProperty<Layer::BatchData>("usize instances_count", &Layer::BatchData::InstancesCount);
				VBatchData->SetConstructor<Layer::BatchData>("void f()");

				auto VVisibilityQuery = VM->SetPod<Layer::VisibilityQuery>("scene_visibility_query");
				VVisibilityQuery->SetProperty<Layer::VisibilityQuery>("geo_category category", &Layer::VisibilityQuery::Category);
				VVisibilityQuery->SetProperty<Layer::VisibilityQuery>("bool boundary_visible", &Layer::VisibilityQuery::BoundaryVisible);
				VVisibilityQuery->SetProperty<Layer::VisibilityQuery>("bool query_pixels", &Layer::VisibilityQuery::QueryPixels);
				VVisibilityQuery->SetConstructor<Layer::VisibilityQuery>("void f()");

				auto VAnimatorState = VM->SetPod<Layer::AnimatorState>("animator_state");
				VAnimatorState->SetProperty<Layer::AnimatorState>("bool paused", &Layer::AnimatorState::Paused);
				VAnimatorState->SetProperty<Layer::AnimatorState>("bool looped", &Layer::AnimatorState::Looped);
				VAnimatorState->SetProperty<Layer::AnimatorState>("bool blended", &Layer::AnimatorState::Blended);
				VAnimatorState->SetProperty<Layer::AnimatorState>("float duration", &Layer::AnimatorState::Duration);
				VAnimatorState->SetProperty<Layer::AnimatorState>("float rate", &Layer::AnimatorState::Rate);
				VAnimatorState->SetProperty<Layer::AnimatorState>("float time", &Layer::AnimatorState::Time);
				VAnimatorState->SetProperty<Layer::AnimatorState>("int64 frame", &Layer::AnimatorState::Frame);
				VAnimatorState->SetProperty<Layer::AnimatorState>("int64 clip", &Layer::AnimatorState::Clip);
				VAnimatorState->SetConstructor<Layer::AnimatorState>("void f()");
				VAnimatorState->SetMethod("float get_timeline(clock_timer@+) const", &Layer::AnimatorState::GetTimeline);
				VAnimatorState->SetMethod("float get_seconds_duration() const", &Layer::AnimatorState::GetSecondsDuration);
				VAnimatorState->SetMethod("float get_progress() const", &Layer::AnimatorState::GetProgress);
				VAnimatorState->SetMethod("bool is_playing() const", &Layer::AnimatorState::IsPlaying);

				auto VSpawnerProperties = VM->SetPod<Layer::SpawnerProperties>("spawner_properties");
				VSpawnerProperties->SetProperty<Layer::SpawnerProperties>("random_vector4 diffusion", &Layer::SpawnerProperties::Diffusion);
				VSpawnerProperties->SetProperty<Layer::SpawnerProperties>("random_vector3 position", &Layer::SpawnerProperties::Position);
				VSpawnerProperties->SetProperty<Layer::SpawnerProperties>("random_vector3 velocity", &Layer::SpawnerProperties::Velocity);
				VSpawnerProperties->SetProperty<Layer::SpawnerProperties>("random_vector3 noise", &Layer::SpawnerProperties::Noise);
				VSpawnerProperties->SetProperty<Layer::SpawnerProperties>("random_float rotation", &Layer::SpawnerProperties::Rotation);
				VSpawnerProperties->SetProperty<Layer::SpawnerProperties>("random_float scale", &Layer::SpawnerProperties::Scale);
				VSpawnerProperties->SetProperty<Layer::SpawnerProperties>("random_float angular", &Layer::SpawnerProperties::Angular);
				VSpawnerProperties->SetProperty<Layer::SpawnerProperties>("int32 iterations", &Layer::SpawnerProperties::Iterations);
				VSpawnerProperties->SetConstructor<Layer::SpawnerProperties>("void f()");

				auto VRenderConstants = VM->SetClass<Layer::RenderConstants>("render_constants", false);
				VRenderConstants->SetProperty<Layer::RenderConstants>("animation_buffer animation", &Layer::RenderConstants::Animation);
				VRenderConstants->SetProperty<Layer::RenderConstants>("render_buffer render", &Layer::RenderConstants::Render);
				VRenderConstants->SetProperty<Layer::RenderConstants>("view_buffer view", &Layer::RenderConstants::View);
				VRenderConstants->SetConstructor<Layer::RenderConstants, Graphics::GraphicsDevice*>("render_constants@ f()");
				VRenderConstants->SetMethod("void set_constant_buffers()", &Layer::RenderConstants::SetConstantBuffers);
				VRenderConstants->SetMethod("void update_constant_buffer(render_buffer_type)", &Layer::RenderConstants::UpdateConstantBuffer);
				VRenderConstants->SetMethod("shader@+ get_basic_effect() const", &Layer::RenderConstants::GetBasicEffect);
				VRenderConstants->SetMethod("graphics_device@+ get_device() const", &Layer::RenderConstants::GetDevice);
				VRenderConstants->SetMethod("element_buffer@+ get_constant_buffer(render_buffer_type) const", &Layer::RenderConstants::GetConstantBuffer);

				auto VRenderSystem = VM->SetClass<Layer::RenderSystem>("render_system", true);
				auto VViewer = VM->SetStruct<Layer::Viewer>("viewer_t");
				VViewer->SetProperty<Layer::Viewer>("render_system@ renderer", &Layer::Viewer::Renderer);
				VViewer->SetProperty<Layer::Viewer>("render_culling culling", &Layer::Viewer::Culling);
				VViewer->SetProperty<Layer::Viewer>("matrix4x4 inv_view_projection", &Layer::Viewer::InvViewProjection);
				VViewer->SetProperty<Layer::Viewer>("matrix4x4 view_projection", &Layer::Viewer::ViewProjection);
				VViewer->SetProperty<Layer::Viewer>("matrix4x4 projection", &Layer::Viewer::Projection);
				VViewer->SetProperty<Layer::Viewer>("matrix4x4 view", &Layer::Viewer::View);
				VViewer->SetProperty<Layer::Viewer>("vector3 inv_position", &Layer::Viewer::InvPosition);
				VViewer->SetProperty<Layer::Viewer>("vector3 position", &Layer::Viewer::Position);
				VViewer->SetProperty<Layer::Viewer>("vector3 rotation", &Layer::Viewer::Rotation);
				VViewer->SetProperty<Layer::Viewer>("float far_plane", &Layer::Viewer::FarPlane);
				VViewer->SetProperty<Layer::Viewer>("float near_plane", &Layer::Viewer::NearPlane);
				VViewer->SetProperty<Layer::Viewer>("float ratio", &Layer::Viewer::Ratio);
				VViewer->SetProperty<Layer::Viewer>("float fov", &Layer::Viewer::Fov);
				VViewer->SetConstructor<Layer::Viewer>("void f()");
				VViewer->SetOperatorCopyStatic(&ViewerCopy);
				VViewer->SetDestructorEx("void f()", &ViewerDestructor);
				VViewer->SetMethod<Layer::Viewer, void, const Trigonometry::Matrix4x4&, const Trigonometry::Matrix4x4&, const Trigonometry::Vector3&, float, float, float, float, Layer::RenderCulling>("void set(const matrix4x4 &in, const matrix4x4 &in, const vector3 &in, float, float, float, float, render_culling)", &Layer::Viewer::Set);
				VViewer->SetMethod<Layer::Viewer, void, const Trigonometry::Matrix4x4&, const Trigonometry::Matrix4x4&, const Trigonometry::Vector3&, const Trigonometry::Vector3&, float, float, float, float, Layer::RenderCulling>("void set(const matrix4x4 &in, const matrix4x4 &in, const vector3 &in, const vector3 &in, float, float, float, float, render_culling)", &Layer::Viewer::Set);

				auto VAttenuation = VM->SetPod<Layer::Attenuation>("attenuation");
				VAttenuation->SetProperty<Layer::Attenuation>("float radius", &Layer::Attenuation::Radius);
				VAttenuation->SetProperty<Layer::Attenuation>("float c1", &Layer::Attenuation::C1);
				VAttenuation->SetProperty<Layer::Attenuation>("float c2", &Layer::Attenuation::C2);
				VAttenuation->SetConstructor<Layer::Attenuation>("void f()");

				auto VSubsurface = VM->SetPod<Layer::Subsurface>("subsurface");
				VSubsurface->SetProperty<Layer::Subsurface>("vector4 emission", &Layer::Subsurface::Emission);
				VSubsurface->SetProperty<Layer::Subsurface>("vector4 metallic", &Layer::Subsurface::Metallic);
				VSubsurface->SetProperty<Layer::Subsurface>("vector4 penetration", &Layer::Subsurface::Penetration);
				VSubsurface->SetProperty<Layer::Subsurface>("vector3 diffuse", &Layer::Subsurface::Diffuse);
				VSubsurface->SetProperty<Layer::Subsurface>("float fresnel", &Layer::Subsurface::Fresnel);
				VSubsurface->SetProperty<Layer::Subsurface>("vector3 scattering", &Layer::Subsurface::Scattering);
				VSubsurface->SetProperty<Layer::Subsurface>("float transparency", &Layer::Subsurface::Transparency);
				VSubsurface->SetProperty<Layer::Subsurface>("vector3 padding", &Layer::Subsurface::Padding);
				VSubsurface->SetProperty<Layer::Subsurface>("float bias", &Layer::Subsurface::Bias);
				VSubsurface->SetProperty<Layer::Subsurface>("vector2 roughness", &Layer::Subsurface::Roughness);
				VSubsurface->SetProperty<Layer::Subsurface>("float refraction", &Layer::Subsurface::Refraction);
				VSubsurface->SetProperty<Layer::Subsurface>("float environment", &Layer::Subsurface::Environment);
				VSubsurface->SetProperty<Layer::Subsurface>("vector2 occlusion", &Layer::Subsurface::Occlusion);
				VSubsurface->SetProperty<Layer::Subsurface>("float radius", &Layer::Subsurface::Radius);
				VSubsurface->SetProperty<Layer::Subsurface>("float height", &Layer::Subsurface::Height);
				VSubsurface->SetConstructor<Layer::Subsurface>("void f()");

				auto VSkinAnimation = VM->SetClass<Layer::SkinAnimation>("skin_animation", false);
				VSkinAnimation->SetMethodEx("array<skin_animator_clip>@+ get_clips() const", &SkinAnimationGetClips);
				VSkinAnimation->SetMethod("bool is_valid() const", &Layer::SkinAnimation::IsValid);

				auto VSceneGraph = VM->SetClass<Layer::SceneGraph>("scene_graph", true);
				VMaterial->SetProperty<Layer::Material>("subsurface surface", &Layer::Material::Surface);
				VMaterial->SetProperty<Layer::Material>("usize slot", &Layer::Material::Slot);
				VMaterial->SetGcConstructor<Layer::Material, Material, Layer::SceneGraph*>("material@ f(scene_graph@+)");
				VMaterial->SetMethod("void set_name(const string_view&in)", &Layer::Material::SetName);
				VMaterial->SetMethod("const string& get_name(const string_view&in)", &Layer::Material::GetName);
				VMaterial->SetMethod("void set_diffuse_map(texture_2d@+)", &Layer::Material::SetDiffuseMap);
				VMaterial->SetMethod("texture_2d@+ get_diffuse_map() const", &Layer::Material::GetDiffuseMap);
				VMaterial->SetMethod("void set_normal_map(texture_2d@+)", &Layer::Material::SetNormalMap);
				VMaterial->SetMethod("texture_2d@+ get_normal_map() const", &Layer::Material::GetNormalMap);
				VMaterial->SetMethod("void set_metallic_map(texture_2d@+)", &Layer::Material::SetMetallicMap);
				VMaterial->SetMethod("texture_2d@+ get_metallic_map() const", &Layer::Material::GetMetallicMap);
				VMaterial->SetMethod("void set_roughness_map(texture_2d@+)", &Layer::Material::SetRoughnessMap);
				VMaterial->SetMethod("texture_2d@+ get_roughness_map() const", &Layer::Material::GetRoughnessMap);
				VMaterial->SetMethod("void set_height_map(texture_2d@+)", &Layer::Material::SetHeightMap);
				VMaterial->SetMethod("texture_2d@+ get_height_map() const", &Layer::Material::GetHeightMap);
				VMaterial->SetMethod("void set_occlusion_map(texture_2d@+)", &Layer::Material::SetOcclusionMap);
				VMaterial->SetMethod("texture_2d@+ get_occlusion_map() const", &Layer::Material::GetOcclusionMap);
				VMaterial->SetMethod("void set_emission_map(texture_2d@+)", &Layer::Material::SetEmissionMap);
				VMaterial->SetMethod("texture_2d@+ get_emission_map() const", &Layer::Material::GetEmissionMap);
				VMaterial->SetMethod("scene_graph@+ get_scene() const", &Layer::Material::GetScene);
				VMaterial->SetEnumRefsEx<Layer::Material>([](Layer::Material* Base, asIScriptEngine* VM)
				{
					FunctionFactory::GCEnumCallback(VM, Base->GetDiffuseMap());
					FunctionFactory::GCEnumCallback(VM, Base->GetNormalMap());
					FunctionFactory::GCEnumCallback(VM, Base->GetMetallicMap());
					FunctionFactory::GCEnumCallback(VM, Base->GetRoughnessMap());
					FunctionFactory::GCEnumCallback(VM, Base->GetHeightMap());
					FunctionFactory::GCEnumCallback(VM, Base->GetEmissionMap());
				});
				VMaterial->SetReleaseRefsEx<Layer::Material>([](Layer::Material* Base, asIScriptEngine*)
				{
					Base->~Material();
				});

				auto VComponent = VM->SetClass<Layer::Component>("base_component", false);
				auto VSparseIndex = VM->SetStructTrivial<Layer::SparseIndex>("sparse_index");
				VSparseIndex->SetProperty<Layer::SparseIndex>("cosmos index", &Layer::SparseIndex::Index);
				VSparseIndex->SetMethodEx("usize size() const", &SparseIndexGetSize);
				VSparseIndex->SetOperatorEx(Operators::Index, (uint32_t)Position::Left, "base_component@+", "usize", &SparseIndexGetData);
				VSparseIndex->SetConstructor<Layer::SparseIndex>("void f()");

				VM->BeginNamespace("content_heavy_series");
				VM->SetFunction<void(Core::Schema*, const Trigonometry::Vector2&)>("void pack(schema@+, const vector2 &in)", &Layer::HeavySeries::Pack);
				VM->SetFunction<void(Core::Schema*, const Trigonometry::Vector3&)>("void pack(schema@+, const vector3 &in)", &Layer::HeavySeries::Pack);
				VM->SetFunction<void(Core::Schema*, const Trigonometry::Vector4&)>("void pack(schema@+, const vector4 &in)", &Layer::HeavySeries::Pack);
				VM->SetFunction<void(Core::Schema*, const Trigonometry::Vector4&)>("void pack(schema@+, const quaternion &in)", &Layer::HeavySeries::Pack);
				VM->SetFunction<void(Core::Schema*, const Trigonometry::Matrix4x4&)>("void pack(schema@+, const matrix4x4 &in)", &Layer::HeavySeries::Pack);
				VM->SetFunction<void(Core::Schema*, const Layer::Attenuation&)>("void pack(schema@+, const attenuation &in)", &Layer::HeavySeries::Pack);
				VM->SetFunction<void(Core::Schema*, const Layer::AnimatorState&)>("void pack(schema@+, const animator_state &in)", &Layer::HeavySeries::Pack);
				VM->SetFunction<void(Core::Schema*, const Layer::SpawnerProperties&)>("void pack(schema@+, const spawner_properties &in)", &Layer::HeavySeries::Pack);
				VM->SetFunction<void(Core::Schema*, const Trigonometry::SkinAnimatorKey&)>("void pack(schema@+, const skin_animator_key &in)", &Layer::HeavySeries::Pack);
				VM->SetFunction<void(Core::Schema*, const Trigonometry::KeyAnimatorClip&)>("void pack(schema@+, const key_animator_clip &in)", &Layer::HeavySeries::Pack);
				VM->SetFunction<void(Core::Schema*, const Trigonometry::AnimatorKey&)>("void pack(schema@+, const animator_key &in)", &Layer::HeavySeries::Pack);
				VM->SetFunction<void(Core::Schema*, const Trigonometry::ElementVertex&)>("void pack(schema@+, const element_vertex &in)", &Layer::HeavySeries::Pack);
				VM->SetFunction<void(Core::Schema*, const Trigonometry::Joint&)>("void pack(schema@+, const joint &in)", &Layer::HeavySeries::Pack);
				VM->SetFunction<void(Core::Schema*, const Trigonometry::Vertex&)>("void pack(schema@+, const vertex &in)", &Layer::HeavySeries::Pack);
				VM->SetFunction<void(Core::Schema*, const Trigonometry::SkinVertex&)>("void pack(schema@+, const skin_vertex &in)", &Layer::HeavySeries::Pack);
				VM->SetFunction<void(Core::Schema*, const Layer::Ticker&)>("void pack(schema@+, const clock_ticker &in)", &Layer::HeavySeries::Pack);
				VM->SetFunction<bool(Core::Schema*, Trigonometry::Vector2*)>("bool unpack(schema@+, vector2 &out)", &Layer::HeavySeries::Unpack);
				VM->SetFunction<bool(Core::Schema*, Trigonometry::Vector3*)>("bool unpack(schema@+, vector3 &out)", &Layer::HeavySeries::Unpack);
				VM->SetFunction<bool(Core::Schema*, Trigonometry::Vector4*)>("bool unpack(schema@+, vector4 &out)", &Layer::HeavySeries::Unpack);
				VM->SetFunction<bool(Core::Schema*, Trigonometry::Vector4*)>("bool unpack(schema@+, quaternion &out)", &Layer::HeavySeries::Unpack);
				VM->SetFunction<bool(Core::Schema*, Trigonometry::Matrix4x4*)>("bool unpack(schema@+, matrix4x4 &out)", &Layer::HeavySeries::Unpack);
				VM->SetFunction<bool(Core::Schema*, Layer::Attenuation*)>("bool unpack(schema@+, attenuation &out)", &Layer::HeavySeries::Unpack);
				VM->SetFunction<bool(Core::Schema*, Layer::AnimatorState*)>("bool unpack(schema@+, animator_state &out)", &Layer::HeavySeries::Unpack);
				VM->SetFunction<bool(Core::Schema*, Layer::SpawnerProperties*)>("bool unpack(schema@+, spawner_properties &out)", &Layer::HeavySeries::Unpack);
				VM->SetFunction<bool(Core::Schema*, Trigonometry::SkinAnimatorKey*)>("bool unpack(schema@+, skin_animator_key &out)", &Layer::HeavySeries::Unpack);
				VM->SetFunction<bool(Core::Schema*, Trigonometry::KeyAnimatorClip*)>("bool unpack(schema@+, key_animator_clip &out)", &Layer::HeavySeries::Unpack);
				VM->SetFunction<bool(Core::Schema*, Trigonometry::AnimatorKey*)>("bool unpack(schema@+, animator_key &out)", &Layer::HeavySeries::Unpack);
				VM->SetFunction<bool(Core::Schema*, Trigonometry::ElementVertex*)>("bool unpack(schema@+, element_vertex &out)", &Layer::HeavySeries::Unpack);
				VM->SetFunction<bool(Core::Schema*, Trigonometry::Joint*)>("bool unpack(schema@+, joint &out)", &Layer::HeavySeries::Unpack);
				VM->SetFunction<bool(Core::Schema*, Trigonometry::Vertex*)>("bool unpack(schema@+, vertex &out)", &Layer::HeavySeries::Unpack);
				VM->SetFunction<bool(Core::Schema*, Trigonometry::SkinVertex*)>("bool unpack(schema@+, skin_vertex &out)", &Layer::HeavySeries::Unpack);
				VM->SetFunction<bool(Core::Schema*, Layer::Ticker*)>("bool unpack(schema@+, clock_ticker &out)", &Layer::HeavySeries::Unpack);
				VM->EndNamespace();

				auto VModel = VM->SetClass<Layer::Model>("model", true);
				VModel->SetProperty<Layer::Model>("vector4 max", &Layer::Model::Max);
				VModel->SetProperty<Layer::Model>("vector4 min", &Layer::Model::Min);
				VModel->SetGcConstructor<Layer::Model, Model>("model@ f()");
				VModel->SetMethod("mesh_buffer@+ find_mesh(const string_view&in) const", &Layer::Model::FindMesh);
				VModel->SetMethodEx("array<mesh_buffer@>@ get_meshes() const", &ModelGetMeshes);
				VModel->SetMethodEx("void set_meshes(array<mesh_buffer@>@+)", &ModelSetMeshes);
				VModel->SetEnumRefsEx<Layer::Model>([](Layer::Model* Base, asIScriptEngine* VM)
				{
					for (auto* Item : Base->Meshes)
						FunctionFactory::GCEnumCallback(VM, Item);
				});
				VModel->SetReleaseRefsEx<Layer::Model>([](Layer::Model* Base, asIScriptEngine*)
				{
					Base->Cleanup();
				});

				VSkinModel->SetProperty<Layer::SkinModel>("joint skeleton", &Layer::SkinModel::Skeleton);
				VSkinModel->SetProperty<Layer::SkinModel>("matrix4x4 inv_transform", &Layer::SkinModel::InvTransform);
				VSkinModel->SetProperty<Layer::SkinModel>("matrix4x4 base_transform", &Layer::SkinModel::Transform);
				VSkinModel->SetProperty<Layer::SkinModel>("vector4 max", &Layer::SkinModel::Max);
				VSkinModel->SetProperty<Layer::SkinModel>("vector4 min", &Layer::SkinModel::Min);
				VSkinModel->SetGcConstructor<Layer::SkinModel, SkinModel>("skin_model@ f()");
				VSkinModel->SetMethod<Layer::SkinModel, bool, const std::string_view&, Trigonometry::Joint*>("bool find_joint(const string_view&in, joint &out) const", &Layer::SkinModel::FindJoint);
				VSkinModel->SetMethod<Layer::SkinModel, bool, size_t, Trigonometry::Joint*>("bool find_joint(usize, joint &out) const", &Layer::SkinModel::FindJoint);
				VSkinModel->SetMethod("skin_mesh_buffer@+ find_mesh(const string_view&in) const", &Layer::SkinModel::FindMesh);
				VSkinModel->SetMethodEx("array<skin_mesh_buffer@>@ get_meshes() const", &SkinModelGetMeshes);
				VSkinModel->SetMethodEx("void set_meshes(array<skin_mesh_buffer@>@+)", &SkinModelSetMeshes);
				VSkinModel->SetEnumRefsEx<Layer::SkinModel>([](Layer::SkinModel* Base, asIScriptEngine* VM)
				{
					for (auto* Item : Base->Meshes)
						FunctionFactory::GCEnumCallback(VM, Item);
				});
				VSkinModel->SetReleaseRefsEx<Layer::SkinModel>([](Layer::SkinModel* Base, asIScriptEngine*)
				{
					Base->Cleanup();
				});

				auto VEntity = VM->SetClass<Layer::Entity>("scene_entity", true);
				PopulateComponentBase<Layer::Component>(*VComponent);

				VEntity->SetMethod("void set_name(const string_view&in)", &Layer::Entity::SetName);
				VEntity->SetMethod("void set_root(scene_entity@+)", &Layer::Entity::SetRoot);
				VEntity->SetMethod("void update_bounds()", &Layer::Entity::UpdateBounds);
				VEntity->SetMethod("void remove_childs()", &Layer::Entity::RemoveChilds);
				VEntity->SetMethodEx("void remove_component(base_component@+)", &EntityRemoveComponent);
				VEntity->SetMethodEx("void remove_component(uint64)", &EntityRemoveComponentById);
				VEntity->SetMethodEx("base_component@+ add_component(base_component@+)", &EntityAddComponent);
				VEntity->SetMethodEx("base_component@+ get_component(uint64) const", &EntityGetComponentById);
				VEntity->SetMethodEx("array<base_component@>@ get_components() const", &EntityGetComponents);
				VEntity->SetMethod("scene_graph@+ get_scene() const", &Layer::Entity::GetScene);
				VEntity->SetMethod("scene_entity@+ get_parent() const", &Layer::Entity::GetParent);
				VEntity->SetMethod("scene_entity@+ get_child(usize) const", &Layer::Entity::GetChild);
				VEntity->SetMethod("transform@+ get_transform() const", &Layer::Entity::GetTransform);
				VEntity->SetMethod("const matrix4x4& get_box() const", &Layer::Entity::GetBox);
				VEntity->SetMethod("const vector3& get_min() const", &Layer::Entity::GetMin);
				VEntity->SetMethod("const vector3& get_max() const", &Layer::Entity::GetMax);
				VEntity->SetMethod("const string& get_name() const", &Layer::Entity::GetName);
				VEntity->SetMethod("usize get_components_count() const", &Layer::Entity::GetComponentsCount);
				VEntity->SetMethod("usize get_childs_count() const", &Layer::Entity::GetComponentsCount);
				VEntity->SetMethod("float get_visibility(const viewer_t &in) const", &Layer::Entity::GetVisibility);
				VEntity->SetMethod("float is_active() const", &Layer::Entity::IsActive);
				VEntity->SetMethod("vector3 get_radius3() const", &Layer::Entity::GetRadius3);
				VEntity->SetMethod("float get_radius() const", &Layer::Entity::GetRadius);
				VEntity->SetEnumRefsEx<Layer::Entity>([](Layer::Entity* Base, asIScriptEngine* VM)
				{
					for (auto& Item : *Base)
						FunctionFactory::GCEnumCallback(VM, Item.second);
				});
				VEntity->SetReleaseRefsEx<Layer::Entity>([](Layer::Entity* Base, asIScriptEngine*) { });

				auto VDrawable = VM->SetClass<Layer::Drawable>("drawable_component", false);
				PopulateDrawableBase<Layer::Drawable>(*VDrawable);

				auto VRenderer = VM->SetClass<Layer::Renderer>("base_renderer", false);
				PopulateRendererBase<Layer::Renderer>(*VRenderer);

				auto VRsState = VM->SetPod<Layer::RenderSystem::RsState>("rs_state");
				VRsState->SetMethod("bool is_state(render_state) const", &Layer::RenderSystem::RsState::Is);
				VRsState->SetMethod("bool is_set(render_opt) const", &Layer::RenderSystem::RsState::IsSet);
				VRsState->SetMethod("bool is_top() const", &Layer::RenderSystem::RsState::IsTop);
				VRsState->SetMethod("bool is_subpass() const", &Layer::RenderSystem::RsState::IsSubpass);
				VRsState->SetMethod("render_opt get_opts() const", &Layer::RenderSystem::RsState::GetOpts);
				VRsState->SetMethod("render_state get_state() const", &Layer::RenderSystem::RsState::Get);

				auto VPrimitiveCache = VM->SetClass<Layer::Renderer>("primitive_cache", true);
				VRenderSystem->SetFunctionDef("void overlapping_result_sync(base_component@+)");
				VRenderSystem->SetProperty<Layer::RenderSystem>("rs_state state", &Layer::RenderSystem::State);
				VRenderSystem->SetProperty<Layer::RenderSystem>("viewer_t view", &Layer::RenderSystem::View);
				VRenderSystem->SetProperty<Layer::RenderSystem>("usize max_queries", &Layer::RenderSystem::MaxQueries);
				VRenderSystem->SetProperty<Layer::RenderSystem>("usize sorting_frequency", &Layer::RenderSystem::SortingFrequency);
				VRenderSystem->SetProperty<Layer::RenderSystem>("usize occlusion_skips", &Layer::RenderSystem::OcclusionSkips);
				VRenderSystem->SetProperty<Layer::RenderSystem>("usize occluder_skips", &Layer::RenderSystem::OccluderSkips);
				VRenderSystem->SetProperty<Layer::RenderSystem>("usize occludee_skips", &Layer::RenderSystem::OccludeeSkips);
				VRenderSystem->SetProperty<Layer::RenderSystem>("float overflow_visibility", &Layer::RenderSystem::OverflowVisibility);
				VRenderSystem->SetProperty<Layer::RenderSystem>("float threshold", &Layer::RenderSystem::Threshold);
				VRenderSystem->SetProperty<Layer::RenderSystem>("bool occlusion_culling", &Layer::RenderSystem::OcclusionCulling);
				VRenderSystem->SetProperty<Layer::RenderSystem>("bool precise_culling", &Layer::RenderSystem::PreciseCulling);
				VRenderSystem->SetProperty<Layer::RenderSystem>("bool allow_input_lag", &Layer::RenderSystem::AllowInputLag);
				VRenderSystem->SetGcConstructor<Layer::RenderSystem, RenderSystem, Layer::SceneGraph*, Layer::Component*>("render_system@ f(scene_graph@+, base_component@+)");
				VRenderSystem->SetMethod("void set_view(const matrix4x4 &in, const matrix4x4 &in, const vector3 &in, float, float, float, float, render_culling)", &Layer::RenderSystem::SetView);
				VRenderSystem->SetMethod("void clear_culling()", &Layer::RenderSystem::ClearCulling);
				VRenderSystem->SetMethodEx("void restore_view_buffer()", &RenderSystemRestoreViewBuffer);
				VRenderSystem->SetMethod("void restore_view_buffer(viewer_t &out)", &Layer::RenderSystem::RestoreViewBuffer);
				VRenderSystem->SetMethod<Layer::RenderSystem, void, Layer::Renderer*>("void remount(base_renderer@+)", &Layer::RenderSystem::Remount);
				VRenderSystem->SetMethod<Layer::RenderSystem, void>("void remount()", &Layer::RenderSystem::Remount);
				VRenderSystem->SetMethod("void mount()", &Layer::RenderSystem::Mount);
				VRenderSystem->SetMethod("void unmount()", &Layer::RenderSystem::Unmount);
				VRenderSystem->SetMethod("void move_renderer(uint64, usize)", &Layer::RenderSystem::MoveRenderer);
				VRenderSystem->SetMethod<Layer::RenderSystem, void, uint64_t>("void remove_renderer(uint64)", &Layer::RenderSystem::RemoveRenderer);
				VRenderSystem->SetMethodEx("void move_renderer(base_renderer@+, usize)", &RenderSystemMoveRenderer);
				VRenderSystem->SetMethodEx("void remove_renderer(base_renderer@+, usize)", &RenderSystemRemoveRenderer);
				VRenderSystem->SetMethod("void restore_output()", &Layer::RenderSystem::RestoreOutput);
				VRenderSystem->SetMethod<Layer::RenderSystem, void, const std::string_view&, Graphics::Shader*>("void free_shader(const string_view&in, shader@+)", &Layer::RenderSystem::FreeShader);
				VRenderSystem->SetMethod<Layer::RenderSystem, void, Graphics::Shader*>("void free_shader(shader@+)", &Layer::RenderSystem::FreeShader);
				VRenderSystem->SetMethodEx("void free_buffers(const string_view&in, element_buffer@+, element_buffer@+)", &RenderSystemFreeBuffers1);
				VRenderSystem->SetMethodEx("void free_buffers(element_buffer@+, element_buffer@+)", &RenderSystemFreeBuffers2);
				VRenderSystem->SetMethod("void update_constant_buffer(render_buffer_type)", &Layer::RenderSystem::UpdateConstantBuffer);
				VRenderSystem->SetMethod("void clear_materials()", &Layer::RenderSystem::ClearMaterials);
				VRenderSystem->SetMethod("void fetch_visibility(base_component@+, scene_visibility_query &out)", &Layer::RenderSystem::FetchVisibility);
				VRenderSystem->SetMethod("usize render(clock_timer@+, render_state, render_opt)", &Layer::RenderSystem::Render);
				VRenderSystem->SetMethod("bool try_instance(material@+, render_buffer_instance &out)", &Layer::RenderSystem::TryInstance);
				VRenderSystem->SetMethod("bool try_geometry(material@+, bool)", &Layer::RenderSystem::TryGeometry);
				VRenderSystem->SetMethod("bool has_category(geo_category)", &Layer::RenderSystem::HasCategory);
				VRenderSystem->SetMethodEx("shader@+ compile_shader(shader_desc &in, usize = 0)", &RenderSystemCompileShader1);
				VRenderSystem->SetMethodEx("shader@+ compile_shader(const string_view&in, usize = 0)", &RenderSystemCompileShader2);
				VRenderSystem->SetMethodEx("array<element_buffer@>@ compile_buffers(const string_view&in, usize, usize)", &RenderSystemCompileBuffers);
				VRenderSystem->SetMethodEx("bool add_renderer(base_renderer@+)", &RenderSystemAddRenderer);
				VRenderSystem->SetMethodEx("base_renderer@+ get_renderer(uint64) const", &RenderSystemGetRenderer);
				VRenderSystem->SetMethodEx("base_renderer@+ get_renderer_by_index(usize) const", &RenderSystemGetRendererByIndex);
				VRenderSystem->SetMethodEx("usize get_renderers_count() const", &RenderSystemGetRenderersCount);
				VRenderSystem->SetMethod<Layer::RenderSystem, bool, uint64_t, size_t&>("bool get_offset(uint64, usize &out) const", &Layer::RenderSystem::GetOffset);
				VRenderSystem->SetMethod("multi_render_target_2d@+ get_mrt(target_type)", &Layer::RenderSystem::GetMRT);
				VRenderSystem->SetMethod("render_target_2d@+ get_rt(target_type)", &Layer::RenderSystem::GetRT);
				VRenderSystem->SetMethod("graphics_device@+ get_device()", &Layer::RenderSystem::GetDevice);
				VRenderSystem->SetMethod("primitive_cache@+ get_primitives()", &Layer::RenderSystem::GetPrimitives);
				VRenderSystem->SetMethod("render_constants@+ get_constants()", &Layer::RenderSystem::GetConstants);
				VRenderSystem->SetMethod("shader@+ get_basic_effect()", &Layer::RenderSystem::GetBasicEffect);
				VRenderSystem->SetMethod("scene_graph@+ get_scene()", &Layer::RenderSystem::GetScene);
				VRenderSystem->SetMethod("base_component@+ get_component()", &Layer::RenderSystem::GetComponent);
				VRenderSystem->SetMethodEx("void query_group(uint64, overlapping_result_sync@)", &RenderSystemQueryGroup);
				VRenderSystem->SetEnumRefsEx<Layer::RenderSystem>([](Layer::RenderSystem* Base, asIScriptEngine* VM)
				{
					for (auto* Item : Base->GetRenderers())
						FunctionFactory::GCEnumCallback(VM, Item);
				});
				VRenderSystem->SetReleaseRefsEx<Layer::RenderSystem>([](Layer::RenderSystem* Base, asIScriptEngine*)
				{
					Base->RemoveRenderers();
				});

				auto VShaderCache = VM->SetClass<Layer::ShaderCache>("shader_cache", true);
				VShaderCache->SetGcConstructor<Layer::ShaderCache, ShaderCache, Graphics::GraphicsDevice*>("shader_cache@ f()");
				VShaderCache->SetMethodEx("shader@+ compile(const string_view&in, const shader_desc &in, usize = 0)", &VI_EXPECTIFY(Layer::ShaderCache::Compile));
				VShaderCache->SetMethod("shader@+ get(const string_view&in)", &Layer::ShaderCache::Get);
				VShaderCache->SetMethod("string find(shader@+)", &Layer::ShaderCache::Find);
				VShaderCache->SetMethod("bool has(const string_view&in)", &Layer::ShaderCache::Has);
				VShaderCache->SetMethod("bool free(const string_view&in, shader@+ = null)", &Layer::ShaderCache::Free);
				VShaderCache->SetMethod("void clear_cache()", &Layer::ShaderCache::ClearCache);
				VShaderCache->SetEnumRefsEx<Layer::ShaderCache>([](Layer::ShaderCache* Base, asIScriptEngine* VM)
				{
					for (auto& Item : Base->GetCaches())
						FunctionFactory::GCEnumCallback(VM, Item.second.Shader);
				});
				VShaderCache->SetReleaseRefsEx<Layer::ShaderCache>([](Layer::ShaderCache* Base, asIScriptEngine*)
				{
					Base->ClearCache();
				});

				VPrimitiveCache->SetGcConstructor<Layer::PrimitiveCache, PrimitiveCache, Graphics::GraphicsDevice*>("primitive_cache@ f()");
				VPrimitiveCache->SetMethodEx("array<element_buffer@>@ compile(const string_view&in, usize, usize)", &PrimitiveCacheCompile);
				VPrimitiveCache->SetMethodEx("array<element_buffer@>@ get(const string_view&in) const", &PrimitiveCacheGet);
				VPrimitiveCache->SetMethod("bool has(const string_view&in) const", &Layer::PrimitiveCache::Has);
				VPrimitiveCache->SetMethodEx("bool free(const string_view&in, element_buffer@+, element_buffer@+)", &PrimitiveCacheFree);
				VPrimitiveCache->SetMethodEx("string find(element_buffer@+, element_buffer@+) const", &PrimitiveCacheFind);
				VPrimitiveCache->SetMethod("model@+ get_box_model() const", &Layer::PrimitiveCache::GetBoxModel);
				VPrimitiveCache->SetMethod("skin_model@+ get_skin_box_model() const", &Layer::PrimitiveCache::GetSkinBoxModel);
				VPrimitiveCache->SetMethod("element_buffer@+ get_quad() const", &Layer::PrimitiveCache::GetQuad);
				VPrimitiveCache->SetMethod("element_buffer@+ get_sphere(buffer_type) const", &Layer::PrimitiveCache::GetSphere);
				VPrimitiveCache->SetMethod("element_buffer@+ get_cube(buffer_type) const", &Layer::PrimitiveCache::GetCube);
				VPrimitiveCache->SetMethod("element_buffer@+ get_box(buffer_type) const", &Layer::PrimitiveCache::GetBox);
				VPrimitiveCache->SetMethod("element_buffer@+ get_skin_box(buffer_type) const", &Layer::PrimitiveCache::GetSkinBox);
				VPrimitiveCache->SetMethodEx("array<element_buffer@>@ get_sphere_buffers() const", &PrimitiveCacheGetSphereBuffers);
				VPrimitiveCache->SetMethodEx("array<element_buffer@>@ get_cube_buffers() const", &PrimitiveCacheGetCubeBuffers);
				VPrimitiveCache->SetMethodEx("array<element_buffer@>@ get_box_buffers() const", &PrimitiveCacheGetBoxBuffers);
				VPrimitiveCache->SetMethodEx("array<element_buffer@>@ get_skin_box_buffers() const", &PrimitiveCacheGetSkinBoxBuffers);
				VPrimitiveCache->SetMethod("void clear_cache()", &Layer::PrimitiveCache::ClearCache);
				VPrimitiveCache->SetEnumRefsEx<Layer::PrimitiveCache>([](Layer::PrimitiveCache* Base, asIScriptEngine* VM)
				{
					FunctionFactory::GCEnumCallback(VM, Base->GetSphere(Layer::BufferType::Vertex));
					FunctionFactory::GCEnumCallback(VM, Base->GetSphere(Layer::BufferType::Index));
					FunctionFactory::GCEnumCallback(VM, Base->GetCube(Layer::BufferType::Vertex));
					FunctionFactory::GCEnumCallback(VM, Base->GetCube(Layer::BufferType::Index));
					FunctionFactory::GCEnumCallback(VM, Base->GetBox(Layer::BufferType::Vertex));
					FunctionFactory::GCEnumCallback(VM, Base->GetBox(Layer::BufferType::Index));
					FunctionFactory::GCEnumCallback(VM, Base->GetSkinBox(Layer::BufferType::Vertex));
					FunctionFactory::GCEnumCallback(VM, Base->GetSkinBox(Layer::BufferType::Index));
					FunctionFactory::GCEnumCallback(VM, Base->GetQuad());

					for (auto& Item : Base->GetCaches())
					{
						FunctionFactory::GCEnumCallback(VM, Item.second.Buffers[0]);
						FunctionFactory::GCEnumCallback(VM, Item.second.Buffers[1]);
					}
				});
				VPrimitiveCache->SetReleaseRefsEx<Layer::PrimitiveCache>([](Layer::PrimitiveCache* Base, asIScriptEngine*)
				{
					Base->ClearCache();
				});

				auto VSceneGraphSharedDesc = VM->SetStruct<Layer::SceneGraph::Desc>("scene_graph_shared_desc");
				VSceneGraphSharedDesc->SetProperty<Layer::SceneGraph::Desc::Dependencies>("graphics_device@ device", &Layer::SceneGraph::Desc::Dependencies::Device);
				VSceneGraphSharedDesc->SetProperty<Layer::SceneGraph::Desc::Dependencies>("activity@ window", &Layer::SceneGraph::Desc::Dependencies::Activity);
				VSceneGraphSharedDesc->SetProperty<Layer::SceneGraph::Desc::Dependencies>("content_manager@ content", &Layer::SceneGraph::Desc::Dependencies::Content);
				VSceneGraphSharedDesc->SetProperty<Layer::SceneGraph::Desc::Dependencies>("primitive_cache@ primitives", &Layer::SceneGraph::Desc::Dependencies::Primitives);
				VSceneGraphSharedDesc->SetProperty<Layer::SceneGraph::Desc::Dependencies>("shader_cache@ shaders", &Layer::SceneGraph::Desc::Dependencies::Shaders);
				VSceneGraphSharedDesc->SetConstructor<Layer::SceneGraph::Desc::Dependencies>("void f()");
				VSceneGraphSharedDesc->SetOperatorCopyStatic(&SceneGraphSharedDescCopy);
				VSceneGraphSharedDesc->SetDestructorEx("void f()", &SceneGraphSharedDescDestructor);

				auto VHeavyApplication = VM->SetClass<Application>("heavy_application", true);
				auto VSceneGraphDesc = VM->SetStructTrivial<Layer::SceneGraph::Desc>("scene_graph_desc");
				VSceneGraphDesc->SetProperty<Layer::SceneGraph::Desc>("scene_graph_shared_desc shared", &Layer::SceneGraph::Desc::Shared);
				VSceneGraphDesc->SetProperty<Layer::SceneGraph::Desc>("physics_simulator_desc simulator", &Layer::SceneGraph::Desc::Simulator);
				VSceneGraphDesc->SetProperty<Layer::SceneGraph::Desc>("usize start_materials", &Layer::SceneGraph::Desc::StartMaterials);
				VSceneGraphDesc->SetProperty<Layer::SceneGraph::Desc>("usize start_entities", &Layer::SceneGraph::Desc::StartEntities);
				VSceneGraphDesc->SetProperty<Layer::SceneGraph::Desc>("usize start_components", &Layer::SceneGraph::Desc::StartComponents);
				VSceneGraphDesc->SetProperty<Layer::SceneGraph::Desc>("usize grow_margin", &Layer::SceneGraph::Desc::GrowMargin);
				VSceneGraphDesc->SetProperty<Layer::SceneGraph::Desc>("usize max_updates", &Layer::SceneGraph::Desc::MaxUpdates);
				VSceneGraphDesc->SetProperty<Layer::SceneGraph::Desc>("usize points_size", &Layer::SceneGraph::Desc::PointsSize);
				VSceneGraphDesc->SetProperty<Layer::SceneGraph::Desc>("usize points_max", &Layer::SceneGraph::Desc::PointsMax);
				VSceneGraphDesc->SetProperty<Layer::SceneGraph::Desc>("usize spots_size", &Layer::SceneGraph::Desc::SpotsSize);
				VSceneGraphDesc->SetProperty<Layer::SceneGraph::Desc>("usize spots_max", &Layer::SceneGraph::Desc::SpotsMax);
				VSceneGraphDesc->SetProperty<Layer::SceneGraph::Desc>("usize lines_size", &Layer::SceneGraph::Desc::LinesSize);
				VSceneGraphDesc->SetProperty<Layer::SceneGraph::Desc>("usize lines_max", &Layer::SceneGraph::Desc::LinesMax);
				VSceneGraphDesc->SetProperty<Layer::SceneGraph::Desc>("usize voxels_size", &Layer::SceneGraph::Desc::VoxelsSize);
				VSceneGraphDesc->SetProperty<Layer::SceneGraph::Desc>("usize voxels_max", &Layer::SceneGraph::Desc::VoxelsMax);
				VSceneGraphDesc->SetProperty<Layer::SceneGraph::Desc>("usize voxels_mips", &Layer::SceneGraph::Desc::VoxelsMips);
				VSceneGraphDesc->SetProperty<Layer::SceneGraph::Desc>("double grow_rate", &Layer::SceneGraph::Desc::GrowRate);
				VSceneGraphDesc->SetProperty<Layer::SceneGraph::Desc>("float render_quality", &Layer::SceneGraph::Desc::RenderQuality);
				VSceneGraphDesc->SetProperty<Layer::SceneGraph::Desc>("bool enable_hdr", &Layer::SceneGraph::Desc::EnableHDR);
				VSceneGraphDesc->SetProperty<Layer::SceneGraph::Desc>("bool mutations", &Layer::SceneGraph::Desc::Mutations);
				VSceneGraphDesc->SetConstructor<Layer::SceneGraph::Desc>("void f()");
				VSceneGraphDesc->SetMethodStatic("scene_graph_desc get(heavy_application@+)", &Layer::SceneGraph::Desc::Get);

				auto VSceneGraphStatistics = VM->SetPod<Layer::SceneGraph::SgStatistics>("scene_graph_statistics");
				VSceneGraphStatistics->SetProperty<Layer::SceneGraph::SgStatistics>("usize batching", &Layer::SceneGraph::SgStatistics::Batching);
				VSceneGraphStatistics->SetProperty<Layer::SceneGraph::SgStatistics>("usize sorting", &Layer::SceneGraph::SgStatistics::Sorting);
				VSceneGraphStatistics->SetProperty<Layer::SceneGraph::SgStatistics>("usize instances", &Layer::SceneGraph::SgStatistics::Instances);
				VSceneGraphStatistics->SetProperty<Layer::SceneGraph::SgStatistics>("usize draw_calls", &Layer::SceneGraph::SgStatistics::DrawCalls);

				VSceneGraph->SetGcConstructor<Layer::SceneGraph, SceneGraph, const Layer::SceneGraph::Desc&>("scene_graph@ f(const scene_graph_desc &in)");
				VSceneGraph->SetFunctionDef("bool ray_test_sync(base_component@+, const vector3 &in)");
				VSceneGraph->SetFunctionDef("void transaction_sync()");
				VSceneGraph->SetFunctionDef("void event_async(const string&in, schema@+)");
				VSceneGraph->SetFunctionDef("void match_sync(const bounding &in)");
				VSceneGraph->SetFunctionDef("void resource_async(uptr@)");
				VSceneGraph->SetProperty("scene_graph_statistics statistics", &Layer::SceneGraph::Statistics);
				VSceneGraph->SetMethod("void configure(const scene_graph_desc &in)", &Layer::SceneGraph::Configure);
				VSceneGraph->SetMethod("void actualize()", &Layer::SceneGraph::Actualize);
				VSceneGraph->SetMethod("void resize_buffers()", &Layer::SceneGraph::ResizeBuffers);
				VSceneGraph->SetMethod("void submit()", &Layer::SceneGraph::Submit);
				VSceneGraph->SetMethod("void dispatch(clock_timer@+)", &Layer::SceneGraph::Dispatch);
				VSceneGraph->SetMethod("void publish(clock_timer@+)", &Layer::SceneGraph::Publish);
				VSceneGraph->SetMethod("void publish_and_submit(clock_timer@+, float, float, float, bool)", &Layer::SceneGraph::PublishAndSubmit);
				VSceneGraph->SetMethod("void delete_material(material@+)", &Layer::SceneGraph::DeleteMaterial);
				VSceneGraph->SetMethod("void remove_entity(scene_entity@+)", &Layer::SceneGraph::RemoveEntity);
				VSceneGraph->SetMethod("void delete_entity(scene_entity@+)", &Layer::SceneGraph::DeleteEntity);
				VSceneGraph->SetMethod("void set_camera(scene_entity@+)", &Layer::SceneGraph::SetCamera);
				VSceneGraph->SetMethodEx("void ray_test(uint64, const ray &in, ray_test_sync@)", &SceneGraphRayTest);
				VSceneGraph->SetMethod("void script_hook(const string_view&in = \"main\")", &Layer::SceneGraph::ScriptHook);
				VSceneGraph->SetMethod("void set_active(bool)", &Layer::SceneGraph::SetActive);
				VSceneGraph->SetMethod("void set_mrt(target_type, bool)", &Layer::SceneGraph::SetMRT);
				VSceneGraph->SetMethod("void set_rt(target_type, bool)", &Layer::SceneGraph::SetRT);
				VSceneGraph->SetMethod("void swap_mrt(target_type, multi_render_target_2d@+)", &Layer::SceneGraph::SwapMRT);
				VSceneGraph->SetMethod("void swap_rt(target_type, render_target_2d@+)", &Layer::SceneGraph::SwapRT);
				VSceneGraph->SetMethod("void clear_mrt(target_type, bool, bool)", &Layer::SceneGraph::ClearMRT);
				VSceneGraph->SetMethod("void clear_rt(target_type, bool, bool)", &Layer::SceneGraph::ClearRT);
				VSceneGraph->SetMethodEx("void mutate(scene_entity@+, scene_entity@+, const string_view&in)", &SceneGraphMutate1);
				VSceneGraph->SetMethodEx("void mutate(scene_entity@+, const string_view&in)", &SceneGraphMutate2);
				VSceneGraph->SetMethodEx("void mutate(base_component@+, const string_view&in)", &SceneGraphMutate3);
				VSceneGraph->SetMethodEx("void mutate(material@+, const string_view&in)", &SceneGraphMutate4);
				VSceneGraph->SetMethodEx("void transaction(transaction_sync@)", &SceneGraphTransaction);
				VSceneGraph->SetMethod("void clear_culling()", &Layer::SceneGraph::ClearCulling);
				VSceneGraph->SetMethod("void reserve_materials(usize)", &Layer::SceneGraph::ReserveMaterials);
				VSceneGraph->SetMethod("void reserve_entities(usize)", &Layer::SceneGraph::ReserveEntities);
				VSceneGraph->SetMethod("void reserve_components(uint64, usize)", &Layer::SceneGraph::ReserveComponents);
				VSceneGraph->SetMethodEx("bool push_event(const string_view&in, schema@+, bool)", &SceneGraphPushEvent1);
				VSceneGraph->SetMethodEx("bool push_event(const string_view&in, schema@+, base_component@+)", &SceneGraphPushEvent2);
				VSceneGraph->SetMethodEx("bool push_event(const string_view&in, schema@+, scene_entity@+)", &SceneGraphPushEvent3);
				VSceneGraph->SetMethodEx("uptr@ set_listener(const string_view&in, event_async@)", &SceneGraphSetListener);
				VSceneGraph->SetMethod("bool clear_listener(const string_view&in, uptr@)", &Layer::SceneGraph::ClearListener);
				VSceneGraph->SetMethod("material@+ get_invalid_material()", &Layer::SceneGraph::GetInvalidMaterial);
				VSceneGraph->SetMethod<Layer::SceneGraph, bool, Layer::Material*>("bool add_material(material@+)", &Layer::SceneGraph::AddMaterial);
				VSceneGraph->SetMethod<Layer::SceneGraph, Layer::Material*>("material@+ add_material()", &Layer::SceneGraph::AddMaterial);
				VSceneGraph->SetMethodEx("void load_resource(uint64, base_component@+, const string_view&in, schema@+, resource_async@)", &SceneGraphLoadResource);
				VSceneGraph->SetMethod<Layer::SceneGraph, Core::String, uint64_t, void*>("string find_resource_id(uint64, uptr@)", &Layer::SceneGraph::FindResourceId);
				VSceneGraph->SetMethod("material@+ clone_material(material@+)", &Layer::SceneGraph::CloneMaterial);
				VSceneGraph->SetMethod("scene_entity@+ get_entity(usize) const", &Layer::SceneGraph::GetEntity);
				VSceneGraph->SetMethod("scene_entity@+ get_last_entity() const", &Layer::SceneGraph::GetLastEntity);
				VSceneGraph->SetMethod("scene_entity@+ get_camera_entity() const", &Layer::SceneGraph::GetCameraEntity);
				VSceneGraph->SetMethod("base_component@+ get_component(uint64, usize) const", &Layer::SceneGraph::GetComponent);
				VSceneGraph->SetMethod("base_component@+ get_camera() const", &Layer::SceneGraph::GetCamera);
				VSceneGraph->SetMethod("render_system@+ get_renderer() const", &Layer::SceneGraph::GetRenderer);
				VSceneGraph->SetMethod("viewer_t get_camera_viewer() const", &Layer::SceneGraph::GetCameraViewer);
				VSceneGraph->SetMethod<Layer::SceneGraph, Layer::Material*, const std::string_view&>("material@+ get_material(const string_view&in) const", &Layer::SceneGraph::GetMaterial);
				VSceneGraph->SetMethod<Layer::SceneGraph, Layer::Material*, size_t>("material@+ get_material(usize) const", &Layer::SceneGraph::GetMaterial);
				VSceneGraph->SetMethod<Layer::SceneGraph, Layer::SparseIndex&, uint64_t>("sparse_index& get_storage(uint64) const", &Layer::SceneGraph::GetStorage);
				VSceneGraph->SetMethodEx("array<base_component@>@ get_components(uint64) const", &SceneGraphGetComponents);
				VSceneGraph->SetMethodEx("array<base_component@>@ get_actors(actor_type) const", &SceneGraphGetActors);
				VSceneGraph->SetMethod("render_target_2d_desc get_desc_rt() const", &Layer::SceneGraph::GetDescRT);
				VSceneGraph->SetMethod("multi_render_target_2d_desc get_desc_mrt() const", &Layer::SceneGraph::GetDescMRT);
				VSceneGraph->SetMethod("surface_format get_format_mrt(uint32) const", &Layer::SceneGraph::GetFormatMRT);
				VSceneGraph->SetMethodEx("array<scene_entity@>@ clone_entity_as_array(scene_entity@+)", &SceneGraphCloneEntityAsArray);
				VSceneGraph->SetMethodEx("array<scene_entity@>@ query_by_parent(scene_entity@+) const", &SceneGraphQueryByParent);
				VSceneGraph->SetMethodEx("array<scene_entity@>@ query_by_name(const string_view&in) const", &SceneGraphQueryByName);
				VSceneGraph->SetMethodEx("array<base_component@>@ query_by_position(uint64, const vector3 &in, float) const", &SceneGraphQueryByPosition);
				VSceneGraph->SetMethodEx("array<base_component@>@ query_by_area(uint64, const vector3 &in, const vector3 &in) const", &SceneGraphQueryByArea);
				VSceneGraph->SetMethodEx("array<base_component@>@ query_by_ray(uint64, const ray &in) const", &SceneGraphQueryByRay);
				VSceneGraph->SetMethodEx("array<base_component@>@ query_by_match(uint64, match_sync@) const", &SceneGraphQueryByMatch);
				VSceneGraph->SetMethod("string as_resource_path(const string_view&in) const", &Layer::SceneGraph::AsResourcePath);
				VSceneGraph->SetMethod<Layer::SceneGraph, Layer::Entity*>("scene_entity@+ add_entity()", &Layer::SceneGraph::AddEntity);
				VSceneGraph->SetMethod("scene_entity@+ clone_entity(scene_entity@+)", &Layer::SceneGraph::CloneEntity);
				VSceneGraph->SetMethod<Layer::SceneGraph, bool, Layer::Entity*>("bool add_entity(scene_entity@+)", &Layer::SceneGraph::AddEntity);
				VSceneGraph->SetMethod("bool is_active() const", &Layer::SceneGraph::IsActive);
				VSceneGraph->SetMethod("bool is_left_handed() const", &Layer::SceneGraph::IsLeftHanded);
				VSceneGraph->SetMethod("bool is_indexed() const", &Layer::SceneGraph::IsIndexed);
				VSceneGraph->SetMethod("bool is_busy() const", &Layer::SceneGraph::IsBusy);
				VSceneGraph->SetMethod("usize get_materials_count() const", &Layer::SceneGraph::GetMaterialsCount);
				VSceneGraph->SetMethod("usize get_entities_count() const", &Layer::SceneGraph::GetEntitiesCount);
				VSceneGraph->SetMethod("usize get_components_count(uint64) const", &Layer::SceneGraph::GetComponentsCount);
				VSceneGraph->SetMethod<Layer::SceneGraph, bool, Layer::Entity*>("bool has_entity(scene_entity@+) const", &Layer::SceneGraph::HasEntity);
				VSceneGraph->SetMethod<Layer::SceneGraph, bool, size_t>("bool has_entity(usize) const", &Layer::SceneGraph::HasEntity);
				VSceneGraph->SetMethod("multi_render_target_2d@+ get_mrt() const", &Layer::SceneGraph::GetMRT);
				VSceneGraph->SetMethod("render_target_2d@+ get_rt() const", &Layer::SceneGraph::GetRT);
				VSceneGraph->SetMethod("element_buffer@+ get_structure() const", &Layer::SceneGraph::GetStructure);
				VSceneGraph->SetMethod("graphics_device@+ get_device() const", &Layer::SceneGraph::GetDevice);
				VSceneGraph->SetMethod("physics_simulator@+ get_simulator() const", &Layer::SceneGraph::GetSimulator);
				VSceneGraph->SetMethod("activity@+ get_activity() const", &Layer::SceneGraph::GetActivity);
				VSceneGraph->SetMethod("render_constants@+ get_constants() const", &Layer::SceneGraph::GetConstants);
				VSceneGraph->SetMethod("shader_cache@+ get_shaders() const", &Layer::SceneGraph::GetShaders);
				VSceneGraph->SetMethod("primitive_cache@+ get_primitives() const", &Layer::SceneGraph::GetPrimitives);
				VSceneGraph->SetMethod("scene_graph_desc& get_conf()", &Layer::SceneGraph::GetConf);
				VSceneGraph->SetEnumRefsEx<Layer::SceneGraph>([](Layer::SceneGraph* Base, asIScriptEngine* VM)
				{
					auto& Conf = Base->GetConf();
					FunctionFactory::GCEnumCallback(VM, Conf.Shared.Shaders);
					FunctionFactory::GCEnumCallback(VM, Conf.Shared.Primitives);
					FunctionFactory::GCEnumCallback(VM, Conf.Shared.Content);
					FunctionFactory::GCEnumCallback(VM, Conf.Shared.Device);
					FunctionFactory::GCEnumCallback(VM, Conf.Shared.Activity);
					FunctionFactory::GCEnumCallback(VM, Conf.Shared.VM);

					size_t Materials = Base->GetMaterialsCount();
					for (size_t i = 0; i < Materials; i++)
						FunctionFactory::GCEnumCallback(VM, Base->GetMaterial(i));

					size_t Entities = Base->GetEntitiesCount();
					for (size_t i = 0; i < Entities; i++)
						FunctionFactory::GCEnumCallback(VM, Base->GetEntity(i));

					for (auto& Item : Base->GetRegistry())
					{
						for (auto* Next : Item.second->Data)
							FunctionFactory::GCEnumCallback(VM, Next);
					}
				});
				VSceneGraph->SetReleaseRefsEx<Layer::SceneGraph>([](Layer::SceneGraph* Base, asIScriptEngine*) { });

				auto VHeavyApplicationCacheInfo = VM->SetStruct<HeavyApplication::CacheInfo>("heavy_application_cache_info");
				VHeavyApplicationCacheInfo->SetProperty<HeavyApplication::CacheInfo>("shader_cache@ shaders", &HeavyApplication::CacheInfo::Shaders);
				VHeavyApplicationCacheInfo->SetProperty<HeavyApplication::CacheInfo>("primitive_cache@ primitives", &HeavyApplication::CacheInfo::Primitives);
				VHeavyApplicationCacheInfo->SetConstructor<HeavyApplication::CacheInfo>("void f()");
				VHeavyApplicationCacheInfo->SetOperatorCopyStatic(&ApplicationCacheInfoCopy);
				VHeavyApplicationCacheInfo->SetDestructorEx("void f()", &ApplicationCacheInfoDestructor);

				auto VHeavyApplicationDesc = VM->SetStructTrivial<HeavyApplication::Desc>("heavy_application_desc");
				VHeavyApplicationDesc->SetProperty<Application::Desc>("application_frame_info refreshrate", &Application::Desc::Refreshrate);
				VHeavyApplicationDesc->SetProperty<HeavyApplication::Desc>("graphics_device_desc graphics", &HeavyApplication::Desc::GraphicsDevice);
				VHeavyApplicationDesc->SetProperty<HeavyApplication::Desc>("activity_desc window", &HeavyApplication::Desc::Activity);
				VHeavyApplicationDesc->SetProperty<Application::Desc>("schedule_policy scheduler", &Application::Desc::Scheduler);
				VHeavyApplicationDesc->SetProperty<Application::Desc>("string preferences", &Application::Desc::Preferences);
				VHeavyApplicationDesc->SetProperty<Application::Desc>("string environment", &Application::Desc::Environment);
				VHeavyApplicationDesc->SetProperty<Application::Desc>("string directory", &Application::Desc::Directory);
				VHeavyApplicationDesc->SetProperty<Application::Desc>("usize polling_timeout", &Application::Desc::PollingTimeout);
				VHeavyApplicationDesc->SetProperty<Application::Desc>("usize polling_events", &Application::Desc::PollingEvents);
				VHeavyApplicationDesc->SetProperty<Application::Desc>("usize threads", &Application::Desc::Threads);
				VHeavyApplicationDesc->SetProperty<Application::Desc>("usize usage", &Application::Desc::Usage);
				VHeavyApplicationDesc->SetProperty<HeavyApplication::Desc>("usize advanced_usage", &HeavyApplication::Desc::AdvancedUsage);
				VHeavyApplicationDesc->SetProperty<HeavyApplication::Desc>("bool blocking_dispatch", &HeavyApplication::Desc::BlockingDispatch);
				VHeavyApplicationDesc->SetProperty<Application::Desc>("bool daemon", &Application::Desc::Daemon);
				VHeavyApplicationDesc->SetProperty<HeavyApplication::Desc>("bool cursor", &HeavyApplication::Desc::Cursor);
				VHeavyApplicationDesc->SetConstructor<HeavyApplication::Desc>("void f()");

				VHeavyApplication->SetFunctionDef("void key_event_sync(key_code, key_mod, int32, int32, bool)");
				VHeavyApplication->SetFunctionDef("void input_event_sync(const string_view&in)");
				VHeavyApplication->SetFunctionDef("void wheel_event_sync(int, int, bool)");
				VHeavyApplication->SetFunctionDef("void window_event_sync(window_state, int, int)");
				VHeavyApplication->SetProperty<Layer::HeavyApplication>("heavy_application_cache_info cache", &Layer::HeavyApplication::Cache);
				VHeavyApplication->SetProperty<Layer::HeavyApplication>("audio_device@ audio", &Layer::HeavyApplication::Audio);
				VHeavyApplication->SetProperty<Layer::HeavyApplication>("graphics_device@ renderer", &Layer::HeavyApplication::Renderer);
				VHeavyApplication->SetProperty<Layer::HeavyApplication>("activity@ window", &Layer::HeavyApplication::Activity);
				VHeavyApplication->SetProperty<Layer::HeavyApplication>("render_constants@ constants", &Layer::HeavyApplication::Constants);
				VHeavyApplication->SetProperty<Layer::HeavyApplication>("content_manager@ content", &Layer::HeavyApplication::Content);
				VHeavyApplication->SetProperty<Layer::HeavyApplication>("app_data@ database", &Layer::HeavyApplication::Database);
				VHeavyApplication->SetProperty<Layer::HeavyApplication>("scene_graph@ scene", &Layer::HeavyApplication::Scene);
				VHeavyApplication->SetProperty<Layer::HeavyApplication>("heavy_application_desc control", &Layer::HeavyApplication::Control);
				VHeavyApplication->SetGcConstructor<HeavyApplication, HeavyApplicationName, HeavyApplication::Desc&, void*, int>("heavy_application@ f(heavy_application_desc &in, ?&in)");
				VHeavyApplication->SetMethod("void set_on_key_event(key_event_sync@)", &HeavyApplication::SetOnKeyEvent);
				VHeavyApplication->SetMethod("void set_on_input_event(input_event_sync@)", &HeavyApplication::SetOnInputEvent);
				VHeavyApplication->SetMethod("void set_on_wheel_event(wheel_event_sync@)", &HeavyApplication::SetOnWheelEvent);
				VHeavyApplication->SetMethod("void set_on_window_event(window_event_sync@)", &HeavyApplication::SetOnWindowEvent);
				VHeavyApplication->SetMethod("void set_on_dispatch(dispatch_sync@)", &HeavyApplication::SetOnDispatch);
				VHeavyApplication->SetMethod("void set_on_publish(publish_sync@)", &HeavyApplication::SetOnPublish);
				VHeavyApplication->SetMethod("void set_on_composition(composition_sync@)", &HeavyApplication::SetOnComposition);
				VHeavyApplication->SetMethod("void set_on_script_hook(script_hook_sync@)", &HeavyApplication::SetOnScriptHook);
				VHeavyApplication->SetMethod("void set_on_initialize(initialize_sync@)", &HeavyApplication::SetOnInitialize);
				VHeavyApplication->SetMethod("void set_on_startup(startup_sync@)", &HeavyApplication::SetOnStartup);
				VHeavyApplication->SetMethod("void set_on_shutdown(shutdown_sync@)", &HeavyApplication::SetOnShutdown);
				VHeavyApplication->SetMethod("int start()", &Layer::HeavyApplication::Start);
				VHeavyApplication->SetMethod("int restart()", &Layer::HeavyApplication::Restart);
				VHeavyApplication->SetMethod("void stop(int = 0)", &Layer::HeavyApplication::Stop);
				VHeavyApplication->SetMethod("application_state get_state() const", &Layer::HeavyApplication::GetState);
				VHeavyApplication->SetMethod("uptr@ get_initiator() const", &HeavyApplication::GetInitiatorObject);
				VHeavyApplication->SetMethod("usize get_processed_events() const", &HeavyApplication::GetProcessedEvents);
				VHeavyApplication->SetMethod("bool has_processed_events() const", &HeavyApplication::HasProcessedEvents);
				VHeavyApplication->SetMethod("bool retrieve_initiator(?&out) const", &HeavyApplication::RetrieveInitiatorObject);
				VHeavyApplication->SetMethod("ui_context@+ try_get_ui() const", &HeavyApplication::TryGetUI);
				VHeavyApplication->SetMethod("ui_context@+ fetch_ui()", &HeavyApplication::FetchUI);
				VHeavyApplication->SetMethodStatic("heavy_application@+ get()", &Layer::HeavyApplication::Get);
				VHeavyApplication->SetMethodStatic("bool wants_restart(int)", &HeavyApplication::WantsRestart);
				VHeavyApplication->SetEnumRefsEx<HeavyApplication>([](HeavyApplication* Base, asIScriptEngine* VM)
				{
					FunctionFactory::GCEnumCallback(VM, Base->Audio);
					FunctionFactory::GCEnumCallback(VM, Base->Renderer);
					FunctionFactory::GCEnumCallback(VM, Base->Activity);
					FunctionFactory::GCEnumCallback(VM, Base->VM);
					FunctionFactory::GCEnumCallback(VM, Base->Content);
					FunctionFactory::GCEnumCallback(VM, Base->Database);
					FunctionFactory::GCEnumCallback(VM, Base->Scene);
					FunctionFactory::GCEnumCallback(VM, Base->GetInitiatorObject());
					FunctionFactory::GCEnumCallback(VM, &Base->OnKeyEvent);
					FunctionFactory::GCEnumCallback(VM, &Base->OnInputEvent);
					FunctionFactory::GCEnumCallback(VM, &Base->OnWheelEvent);
					FunctionFactory::GCEnumCallback(VM, &Base->OnWindowEvent);
					FunctionFactory::GCEnumCallback(VM, &Base->OnDispatch);
					FunctionFactory::GCEnumCallback(VM, &Base->OnPublish);
					FunctionFactory::GCEnumCallback(VM, &Base->OnComposition);
					FunctionFactory::GCEnumCallback(VM, &Base->OnScriptHook);
					FunctionFactory::GCEnumCallback(VM, &Base->OnInitialize);
					FunctionFactory::GCEnumCallback(VM, &Base->OnStartup);
					FunctionFactory::GCEnumCallback(VM, &Base->OnShutdown);
				});
				VHeavyApplication->SetReleaseRefsEx<HeavyApplication>([](HeavyApplication* Base, asIScriptEngine*)
				{
					Base->~HeavyApplication();
				});

				return true;
#else
				VI_ASSERT(false, "<engine> is not loaded");
				return false;
#endif
			}
			bool HeavyRegistry::ImportEngineComponents(VirtualMachine* VM) noexcept
			{
#ifdef VI_BINDINGS
				VI_ASSERT(VM != nullptr, "manager should be set");
				VM->SetFunctionDef("void component_resource_event()");

				auto VSoftBody = VM->SetClass<Layer::Components::SoftBody>("soft_body_component", false);
				VSoftBody->SetProperty<Layer::Components::SoftBody>("vector2 texcoord", &Layer::Components::SoftBody::TexCoord);
				VSoftBody->SetProperty<Layer::Components::SoftBody>("bool kinematic", &Layer::Components::SoftBody::Kinematic);
				VSoftBody->SetProperty<Layer::Components::SoftBody>("bool manage", &Layer::Components::SoftBody::Manage);
				VSoftBody->SetMethodEx("void load(const string_view&in, float = 0.0f, component_resource_event@ = null)", &ComponentsSoftBodyLoad);
				VSoftBody->SetMethod<Layer::Components::SoftBody, void, Physics::HullShape*, float>("void load(physics_hull_shape@+, float = 0.0f)", &Layer::Components::SoftBody::Load);
				VSoftBody->SetMethod("void load_ellipsoid(const physics_softbody_desc_cv_sconvex &in, float)", &Layer::Components::SoftBody::LoadEllipsoid);
				VSoftBody->SetMethod("void load_patch(const physics_softbody_desc_cv_spatch &in, float)", &Layer::Components::SoftBody::LoadPatch);
				VSoftBody->SetMethod("void load_rope(const physics_softbody_desc_cv_srope &in, float)", &Layer::Components::SoftBody::LoadRope);
				VSoftBody->SetMethod("void fill(graphics_device@+, element_buffer@+, element_buffer@+)", &Layer::Components::SoftBody::Fill);
				VSoftBody->SetMethod("void regenerate()", &Layer::Components::SoftBody::Regenerate);
				VSoftBody->SetMethod("void clear()", &Layer::Components::SoftBody::Clear);
				VSoftBody->SetMethod<Layer::Components::SoftBody, void, const Trigonometry::Vector3&, const Trigonometry::Vector3&, const Trigonometry::Vector3&>("void set_transform(const vector3 &in, const vector3 &in, const vector3 &in)", &Layer::Components::SoftBody::SetTransform);
				VSoftBody->SetMethod<Layer::Components::SoftBody, void, bool>("void set_transform(bool)", &Layer::Components::SoftBody::SetTransform);
				VSoftBody->SetMethod("physics_softbody@+ get_body() const", &Layer::Components::SoftBody::GetBody);
				PopulateDrawableInterface<Layer::Components::SoftBody, Layer::Entity*>(*VSoftBody, "soft_body_component@+ f(scene_entity@+)");

				auto VRigidBody = VM->SetClass<Layer::Components::RigidBody>("rigid_body_component", false);
				VRigidBody->SetProperty<Layer::Components::RigidBody>("bool kinematic", &Layer::Components::RigidBody::Kinematic);
				VRigidBody->SetProperty<Layer::Components::RigidBody>("bool manage", &Layer::Components::RigidBody::Manage);
				VRigidBody->SetMethodEx("void load(const string_view&in, float, float = 0.0f, component_resource_event@ = null)", &ComponentsRigidBodyLoad);
				VRigidBody->SetMethod<Layer::Components::RigidBody, void, btCollisionShape*, float, float>("void load(uptr@, float, float = 0.0f)", &Layer::Components::RigidBody::Load);
				VRigidBody->SetMethod("void clear()", &Layer::Components::RigidBody::Clear);
				VRigidBody->SetMethod("void set_mass(float)", &Layer::Components::RigidBody::SetMass);
				VRigidBody->SetMethod<Layer::Components::RigidBody, void, const Trigonometry::Vector3&, const Trigonometry::Vector3&, const Trigonometry::Vector3&>("void set_transform(const vector3 &in, const vector3 &in, const vector3 &in)", &Layer::Components::RigidBody::SetTransform);
				VRigidBody->SetMethod<Layer::Components::RigidBody, void, bool>("void set_transform(bool)", &Layer::Components::RigidBody::SetTransform);
				VRigidBody->SetMethod("physics_rigidbody@+ get_body() const", &Layer::Components::RigidBody::GetBody);
				PopulateComponentInterface<Layer::Components::RigidBody, Layer::Entity*>(*VRigidBody, "rigid_body_component@+ f(scene_entity@+)");

				auto VSliderConstraint = VM->SetClass<Layer::Components::SliderConstraint>("slider_constraint_component", false);
				VSliderConstraint->SetMethod("void load(scene_entity@+, bool, bool)", &Layer::Components::SliderConstraint::Load);
				VSliderConstraint->SetMethod("void clear()", &Layer::Components::SliderConstraint::Clear);
				VSliderConstraint->SetMethod("physics_rigidbody@+ get_constraint() const", &Layer::Components::SliderConstraint::GetConstraint);
				VSliderConstraint->SetMethod("scene_entity@+ get_connection() const", &Layer::Components::SliderConstraint::GetConnection);
				PopulateComponentInterface<Layer::Components::SliderConstraint, Layer::Entity*>(*VSliderConstraint, "slider_constraint_component@+ f(scene_entity@+)");

				auto VAcceleration = VM->SetClass<Layer::Components::Acceleration>("acceleration_component", false);
				VAcceleration->SetProperty<Layer::Components::Acceleration>("vector3 amplitude_velocity", &Layer::Components::Acceleration::AmplitudeVelocity);
				VAcceleration->SetProperty<Layer::Components::Acceleration>("vector3 amplitude_torque", &Layer::Components::Acceleration::AmplitudeTorque);
				VAcceleration->SetProperty<Layer::Components::Acceleration>("vector3 constant_velocity", &Layer::Components::Acceleration::ConstantVelocity);
				VAcceleration->SetProperty<Layer::Components::Acceleration>("vector3 constant_torque", &Layer::Components::Acceleration::ConstantTorque);
				VAcceleration->SetProperty<Layer::Components::Acceleration>("vector3 constant_center", &Layer::Components::Acceleration::ConstantCenter);
				VAcceleration->SetProperty<Layer::Components::Acceleration>("bool kinematic", &Layer::Components::Acceleration::Kinematic);
				VAcceleration->SetMethod("rigid_body_component@+ get_body() const", &Layer::Components::Acceleration::GetBody);
				PopulateComponentInterface<Layer::Components::Acceleration, Layer::Entity*>(*VAcceleration, "acceleration_component@+ f(scene_entity@+)");

				auto VModel = VM->SetClass<Layer::Components::Model>("model_component", false);
				VModel->SetProperty<Layer::Components::Model>("vector2 texcoord", &Layer::Components::Model::TexCoord);
				VModel->SetMethod("void set_drawable(model@+)", &Layer::Components::Model::SetDrawable);
				VModel->SetMethod("void set_material_for(const string_view&in, material@+)", &Layer::Components::Model::SetMaterialFor);
				VModel->SetMethod("model@+ get_drawable() const", &Layer::Components::Model::GetDrawable);
				VModel->SetMethod("material@+ get_material_for(const string_view&in)", &Layer::Components::Model::GetMaterialFor);
				PopulateDrawableInterface<Layer::Components::Model, Layer::Entity*>(*VModel, "model_component@+ f(scene_entity@+)");

				auto VSkin = VM->SetClass<Layer::Components::Skin>("skin_component", false);
				VSkin->SetProperty<Layer::Components::Skin>("vector2 texcoord", &Layer::Components::Skin::TexCoord);
				VSkin->SetProperty<Layer::Components::Skin>("pose_buffer skeleton", &Layer::Components::Skin::Skeleton);
				VSkin->SetMethod("void set_drawable(skin_model@+)", &Layer::Components::Skin::SetDrawable);
				VSkin->SetMethod("void set_material_for(const string_view&in, material@+)", &Layer::Components::Skin::SetMaterialFor);
				VSkin->SetMethod("skin_model@+ get_drawable() const", &Layer::Components::Skin::GetDrawable);
				VSkin->SetMethod("material@+ get_material_for(const string_view&in)", &Layer::Components::Skin::GetMaterialFor);
				PopulateDrawableInterface<Layer::Components::Skin, Layer::Entity*>(*VSkin, "skin_component@+ f(scene_entity@+)");

				auto VEmitter = VM->SetClass<Layer::Components::Emitter>("emitter_component", false);
				VEmitter->SetProperty<Layer::Components::Emitter>("vector3 min", &Layer::Components::Emitter::Min);
				VEmitter->SetProperty<Layer::Components::Emitter>("vector3 max", &Layer::Components::Emitter::Max);
				VEmitter->SetProperty<Layer::Components::Emitter>("bool connected", &Layer::Components::Emitter::Connected);
				VEmitter->SetProperty<Layer::Components::Emitter>("bool quad_based", &Layer::Components::Emitter::QuadBased);
				VEmitter->SetMethod("instance_buffer@+ get_buffer() const", &Layer::Components::Emitter::GetBuffer);
				PopulateDrawableInterface<Layer::Components::Emitter, Layer::Entity*>(*VEmitter, "emitter_component@+ f(scene_entity@+)");

				auto VDecal = VM->SetClass<Layer::Components::Decal>("decal_component", false);
				VDecal->SetProperty<Layer::Components::Decal>("vector2 texcoord", &Layer::Components::Decal::TexCoord);
				PopulateDrawableInterface<Layer::Components::Decal, Layer::Entity*>(*VDecal, "decal_component@+ f(scene_entity@+)");

				auto VSkinAnimator = VM->SetClass<Layer::Components::SkinAnimator>("skin_animator_component", false);
				VSkinAnimator->SetProperty<Layer::Components::SkinAnimator>("animator_state state", &Layer::Components::SkinAnimator::State);
				VSkinAnimator->SetMethod("void set_animation(skin_animation@+)", &Layer::Components::SkinAnimator::SetAnimation);
				VSkinAnimator->SetMethod("void play(int64 = -1, int64 = -1)", &Layer::Components::SkinAnimator::Play);
				VSkinAnimator->SetMethod("void pause()", &Layer::Components::SkinAnimator::Pause);
				VSkinAnimator->SetMethod("void stop()", &Layer::Components::SkinAnimator::Stop);
				VSkinAnimator->SetMethod<Layer::Components::SkinAnimator, bool, int64_t>("bool is_exists(int64) const", &Layer::Components::SkinAnimator::IsExists);
				VSkinAnimator->SetMethod<Layer::Components::SkinAnimator, bool, int64_t, int64_t>("bool is_exists(int64, int64) const", &Layer::Components::SkinAnimator::IsExists);
				VSkinAnimator->SetMethod("skin_animator_key& get_frame(int64, int64) const", &Layer::Components::SkinAnimator::GetFrame);
				VSkinAnimator->SetMethod("skin_component@+ get_skin() const", &Layer::Components::SkinAnimator::GetSkin);
				VSkinAnimator->SetMethod("skin_animation@+ get_animation() const", &Layer::Components::SkinAnimator::GetAnimation);
				VSkinAnimator->SetMethod("string get_path() const", &Layer::Components::SkinAnimator::GetPath);
				VSkinAnimator->SetMethod("int64 get_clip_by_name(const string_view&in) const", &Layer::Components::SkinAnimator::GetClipByName);
				VSkinAnimator->SetMethod("usize get_clips_count() const", &Layer::Components::SkinAnimator::GetClipsCount);
				PopulateComponentInterface<Layer::Components::SkinAnimator, Layer::Entity*>(*VSkinAnimator, "skin_animator_component@+ f(scene_entity@+)");

				auto VKeyAnimator = VM->SetClass<Layer::Components::KeyAnimator>("key_animator_component", false);
				VKeyAnimator->SetProperty<Layer::Components::KeyAnimator>("animator_key offset_pose", &Layer::Components::KeyAnimator::Offset);
				VKeyAnimator->SetProperty<Layer::Components::KeyAnimator>("animator_key default_pose", &Layer::Components::KeyAnimator::Default);
				VKeyAnimator->SetProperty<Layer::Components::KeyAnimator>("animator_state state", &Layer::Components::KeyAnimator::State);
				VKeyAnimator->SetMethodEx("void load_animation(const string_view&in, component_resource_event@ = null)", &ComponentsKeyAnimatorLoadAnimation);
				VKeyAnimator->SetMethod("void clear_animation()", &Layer::Components::KeyAnimator::ClearAnimation);
				VKeyAnimator->SetMethod("void play(int64 = -1, int64 = -1)", &Layer::Components::KeyAnimator::Play);
				VKeyAnimator->SetMethod("void pause()", &Layer::Components::KeyAnimator::Pause);
				VKeyAnimator->SetMethod("void stop()", &Layer::Components::KeyAnimator::Stop);
				VKeyAnimator->SetMethod<Layer::Components::KeyAnimator, bool, int64_t>("bool is_exists(int64) const", &Layer::Components::KeyAnimator::IsExists);
				VKeyAnimator->SetMethod<Layer::Components::KeyAnimator, bool, int64_t, int64_t>("bool is_exists(int64, int64) const", &Layer::Components::KeyAnimator::IsExists);
				VKeyAnimator->SetMethod("animator_key& get_frame(int64, int64) const", &Layer::Components::KeyAnimator::GetFrame);
				VKeyAnimator->SetMethod("string get_path() const", &Layer::Components::KeyAnimator::GetPath);
				PopulateComponentInterface<Layer::Components::KeyAnimator, Layer::Entity*>(*VKeyAnimator, "key_animator_component@+ f(scene_entity@+)");

				auto VEmitterAnimator = VM->SetClass<Layer::Components::EmitterAnimator>("emitter_animator_component", false);
				VEmitterAnimator->SetProperty<Layer::Components::EmitterAnimator>("vector4 diffuse", &Layer::Components::EmitterAnimator::Diffuse);
				VEmitterAnimator->SetProperty<Layer::Components::EmitterAnimator>("vector3 position", &Layer::Components::EmitterAnimator::Position);
				VEmitterAnimator->SetProperty<Layer::Components::EmitterAnimator>("vector3 velocity", &Layer::Components::EmitterAnimator::Velocity);
				VEmitterAnimator->SetProperty<Layer::Components::EmitterAnimator>("spawner_properties spawner", &Layer::Components::EmitterAnimator::Spawner);
				VEmitterAnimator->SetProperty<Layer::Components::EmitterAnimator>("float rotation_speed", &Layer::Components::EmitterAnimator::RotationSpeed);
				VEmitterAnimator->SetProperty<Layer::Components::EmitterAnimator>("float scale_speed", &Layer::Components::EmitterAnimator::ScaleSpeed);
				VEmitterAnimator->SetProperty<Layer::Components::EmitterAnimator>("float noise", &Layer::Components::EmitterAnimator::Noise);
				VEmitterAnimator->SetProperty<Layer::Components::EmitterAnimator>("bool simulate", &Layer::Components::EmitterAnimator::Simulate);
				VEmitterAnimator->SetMethod("emitter_component@+ get_emitter() const", &Layer::Components::EmitterAnimator::GetEmitter);
				PopulateComponentInterface<Layer::Components::EmitterAnimator, Layer::Entity*>(*VEmitterAnimator, "emitter_animator_component@+ f(scene_entity@+)");

				auto VFreeLook = VM->SetClass<Layer::Components::FreeLook>("free_look_component", false);
				VFreeLook->SetProperty<Layer::Components::FreeLook>("vector2 direction", &Layer::Components::FreeLook::Direction);
				VFreeLook->SetProperty<Layer::Components::FreeLook>("key_map rotate", &Layer::Components::FreeLook::Rotate);
				VFreeLook->SetProperty<Layer::Components::FreeLook>("float sensivity", &Layer::Components::FreeLook::Sensivity);
				PopulateComponentInterface<Layer::Components::FreeLook, Layer::Entity*>(*VFreeLook, "free_look_component@+ f(scene_entity@+)");

				auto VFlyMoveInfo = VM->SetPod<Layer::Components::Fly::MoveInfo>("fly_move_info");
				VFlyMoveInfo->SetProperty<Layer::Components::Fly::MoveInfo>("vector3 axis", &Layer::Components::Fly::MoveInfo::Axis);
				VFlyMoveInfo->SetProperty<Layer::Components::Fly::MoveInfo>("float faster", &Layer::Components::Fly::MoveInfo::Faster);
				VFlyMoveInfo->SetProperty<Layer::Components::Fly::MoveInfo>("float normal", &Layer::Components::Fly::MoveInfo::Normal);
				VFlyMoveInfo->SetProperty<Layer::Components::Fly::MoveInfo>("float slower", &Layer::Components::Fly::MoveInfo::Slower);
				VFlyMoveInfo->SetProperty<Layer::Components::Fly::MoveInfo>("float fading", &Layer::Components::Fly::MoveInfo::Fading);

				auto VFly = VM->SetClass<Layer::Components::Fly>("fly_component", false);
				VFly->SetProperty<Layer::Components::Fly>("fly_move_info moving", &Layer::Components::Fly::Moving);
				VFly->SetProperty<Layer::Components::Fly>("key_map forward", &Layer::Components::Fly::Forward);
				VFly->SetProperty<Layer::Components::Fly>("key_map backward", &Layer::Components::Fly::Backward);
				VFly->SetProperty<Layer::Components::Fly>("key_map right", &Layer::Components::Fly::Right);
				VFly->SetProperty<Layer::Components::Fly>("key_map left", &Layer::Components::Fly::Left);
				VFly->SetProperty<Layer::Components::Fly>("key_map up", &Layer::Components::Fly::Up);
				VFly->SetProperty<Layer::Components::Fly>("key_map down", &Layer::Components::Fly::Down);
				VFly->SetProperty<Layer::Components::Fly>("key_map fast", &Layer::Components::Fly::Fast);
				VFly->SetProperty<Layer::Components::Fly>("key_map slow", &Layer::Components::Fly::Slow);
				PopulateComponentInterface<Layer::Components::Fly, Layer::Entity*>(*VFly, "fly_component@+ f(scene_entity@+)");

				auto VAudioSource = VM->SetClass<Layer::Components::AudioSource>("audio_source_component", false);
				VAudioSource->SetMethod("void apply_playing_position()", &Layer::Components::AudioSource::ApplyPlayingPosition);
				VAudioSource->SetMethod("audio_source@+ get_source() const", &Layer::Components::AudioSource::GetSource);
				VAudioSource->SetMethod("audio_sync& get_sync()", &Layer::Components::AudioSource::GetSync);
				PopulateComponentInterface<Layer::Components::AudioSource, Layer::Entity*>(*VAudioSource, "audio_source_component@+ f(scene_entity@+)");

				auto VAudioListener = VM->SetClass<Layer::Components::AudioListener>("audio_listener_component", false);
				VAudioListener->SetProperty<Layer::Components::AudioListener>("float gain", &Layer::Components::AudioListener::Gain);
				PopulateComponentInterface<Layer::Components::AudioListener, Layer::Entity*>(*VAudioListener, "audio_listener_component@+ f(scene_entity@+)");

				auto VPointLightShadowInfo = VM->SetPod<Layer::Components::PointLight::ShadowInfo>("point_light_shadow_info");
				VPointLightShadowInfo->SetProperty<Layer::Components::PointLight::ShadowInfo>("float softness", &Layer::Components::PointLight::ShadowInfo::Softness);
				VPointLightShadowInfo->SetProperty<Layer::Components::PointLight::ShadowInfo>("float distance", &Layer::Components::PointLight::ShadowInfo::Distance);
				VPointLightShadowInfo->SetProperty<Layer::Components::PointLight::ShadowInfo>("float bias", &Layer::Components::PointLight::ShadowInfo::Bias);
				VPointLightShadowInfo->SetProperty<Layer::Components::PointLight::ShadowInfo>("uint32 iterations", &Layer::Components::PointLight::ShadowInfo::Iterations);
				VPointLightShadowInfo->SetProperty<Layer::Components::PointLight::ShadowInfo>("bool enabled", &Layer::Components::PointLight::ShadowInfo::Enabled);

				auto VPointLight = VM->SetClass<Layer::Components::PointLight>("point_light_component", false);
				VPointLight->SetProperty<Layer::Components::PointLight>("point_light_shadow_info shadow", &Layer::Components::PointLight::Shadow);
				VPointLight->SetProperty<Layer::Components::PointLight>("matrix4x4 view", &Layer::Components::PointLight::View);
				VPointLight->SetProperty<Layer::Components::PointLight>("matrix4x4 projection", &Layer::Components::PointLight::Projection);
				VPointLight->SetProperty<Layer::Components::PointLight>("vector3 diffuse", &Layer::Components::PointLight::Diffuse);
				VPointLight->SetProperty<Layer::Components::PointLight>("float emission", &Layer::Components::PointLight::Emission);
				VPointLight->SetProperty<Layer::Components::PointLight>("float disperse", &Layer::Components::PointLight::Disperse);
				VPointLight->SetMethod("void generate_origin()", &Layer::Components::PointLight::GenerateOrigin);
				VPointLight->SetMethod("void set_size(const attenuation &in)", &Layer::Components::PointLight::SetSize);
				VPointLight->SetMethod("const attenuation& get_size() const", &Layer::Components::PointLight::SetSize);
				PopulateComponentInterface<Layer::Components::PointLight, Layer::Entity*>(*VPointLight, "point_light_component@+ f(scene_entity@+)");

				auto VSpotLightShadowInfo = VM->SetPod<Layer::Components::SpotLight::ShadowInfo>("spot_light_shadow_info");
				VSpotLightShadowInfo->SetProperty<Layer::Components::SpotLight::ShadowInfo>("float softness", &Layer::Components::SpotLight::ShadowInfo::Softness);
				VSpotLightShadowInfo->SetProperty<Layer::Components::SpotLight::ShadowInfo>("float distance", &Layer::Components::SpotLight::ShadowInfo::Distance);
				VSpotLightShadowInfo->SetProperty<Layer::Components::SpotLight::ShadowInfo>("float bias", &Layer::Components::SpotLight::ShadowInfo::Bias);
				VSpotLightShadowInfo->SetProperty<Layer::Components::SpotLight::ShadowInfo>("uint32 iterations", &Layer::Components::SpotLight::ShadowInfo::Iterations);
				VSpotLightShadowInfo->SetProperty<Layer::Components::SpotLight::ShadowInfo>("bool enabled", &Layer::Components::SpotLight::ShadowInfo::Enabled);

				auto VSpotLight = VM->SetClass<Layer::Components::SpotLight>("spot_light_component", false);
				VSpotLight->SetProperty<Layer::Components::SpotLight>("spot_light_shadow_info shadow", &Layer::Components::SpotLight::Shadow);
				VSpotLight->SetProperty<Layer::Components::SpotLight>("matrix4x4 view", &Layer::Components::SpotLight::View);
				VSpotLight->SetProperty<Layer::Components::SpotLight>("matrix4x4 projection", &Layer::Components::SpotLight::Projection);
				VSpotLight->SetProperty<Layer::Components::SpotLight>("vector3 diffuse", &Layer::Components::SpotLight::Diffuse);
				VSpotLight->SetProperty<Layer::Components::SpotLight>("float emission", &Layer::Components::SpotLight::Emission);
				VSpotLight->SetProperty<Layer::Components::SpotLight>("float cutoff", &Layer::Components::SpotLight::Cutoff);
				VSpotLight->SetProperty<Layer::Components::SpotLight>("float disperse", &Layer::Components::SpotLight::Disperse);
				VSpotLight->SetMethod("void generate_origin()", &Layer::Components::SpotLight::GenerateOrigin);
				VSpotLight->SetMethod("void set_size(const attenuation &in)", &Layer::Components::SpotLight::SetSize);
				VSpotLight->SetMethod("const attenuation& get_size() const", &Layer::Components::SpotLight::SetSize);
				PopulateComponentInterface<Layer::Components::SpotLight, Layer::Entity*>(*VSpotLight, "spot_light_component@+ f(scene_entity@+)");

				auto VLineLightSkyInfo = VM->SetPod<Layer::Components::LineLight::SkyInfo>("line_light_sky_info");
				VLineLightSkyInfo->SetProperty<Layer::Components::LineLight::SkyInfo>("vector3 elh_emission", &Layer::Components::LineLight::SkyInfo::RlhEmission);
				VLineLightSkyInfo->SetProperty<Layer::Components::LineLight::SkyInfo>("vector3 mie_emission", &Layer::Components::LineLight::SkyInfo::MieEmission);
				VLineLightSkyInfo->SetProperty<Layer::Components::LineLight::SkyInfo>("float rlh_height", &Layer::Components::LineLight::SkyInfo::RlhHeight);
				VLineLightSkyInfo->SetProperty<Layer::Components::LineLight::SkyInfo>("float mie_height", &Layer::Components::LineLight::SkyInfo::MieHeight);
				VLineLightSkyInfo->SetProperty<Layer::Components::LineLight::SkyInfo>("float mie_direction", &Layer::Components::LineLight::SkyInfo::MieDirection);
				VLineLightSkyInfo->SetProperty<Layer::Components::LineLight::SkyInfo>("float inner_radius", &Layer::Components::LineLight::SkyInfo::InnerRadius);
				VLineLightSkyInfo->SetProperty<Layer::Components::LineLight::SkyInfo>("float outer_radius", &Layer::Components::LineLight::SkyInfo::OuterRadius);
				VLineLightSkyInfo->SetProperty<Layer::Components::LineLight::SkyInfo>("float intensity", &Layer::Components::LineLight::SkyInfo::Intensity);

				auto VLineLightShadowInfo = VM->SetPod<Layer::Components::LineLight::ShadowInfo>("line_light_shadow_info");
				VLineLightShadowInfo->SetPropertyArray<Layer::Components::LineLight::ShadowInfo>("float distance", &Layer::Components::LineLight::ShadowInfo::Distance, 6);
				VLineLightShadowInfo->SetProperty<Layer::Components::LineLight::ShadowInfo>("float softness", &Layer::Components::LineLight::ShadowInfo::Softness);
				VLineLightShadowInfo->SetProperty<Layer::Components::LineLight::ShadowInfo>("float bias", &Layer::Components::LineLight::ShadowInfo::Bias);
				VLineLightShadowInfo->SetProperty<Layer::Components::LineLight::ShadowInfo>("float near", &Layer::Components::LineLight::ShadowInfo::Near);
				VLineLightShadowInfo->SetProperty<Layer::Components::LineLight::ShadowInfo>("float far", &Layer::Components::LineLight::ShadowInfo::Far);
				VLineLightShadowInfo->SetProperty<Layer::Components::LineLight::ShadowInfo>("uint32 iterations", &Layer::Components::LineLight::ShadowInfo::Iterations);
				VLineLightShadowInfo->SetProperty<Layer::Components::LineLight::ShadowInfo>("uint32 cascades", &Layer::Components::LineLight::ShadowInfo::Cascades);
				VLineLightShadowInfo->SetProperty<Layer::Components::LineLight::ShadowInfo>("bool enabled", &Layer::Components::LineLight::ShadowInfo::Enabled);

				auto VLineLight = VM->SetClass<Layer::Components::LineLight>("line_light_component", false);
				VLineLight->SetProperty<Layer::Components::LineLight>("line_light_sky_info sky", &Layer::Components::LineLight::Sky);
				VLineLight->SetProperty<Layer::Components::LineLight>("line_light_shadow_info shadow", &Layer::Components::LineLight::Shadow);
				VLineLight->SetPropertyArray<Layer::Components::LineLight>("matrix4x4 projection", &Layer::Components::LineLight::Projection, 6);
				VLineLight->SetPropertyArray<Layer::Components::LineLight>("matrix4x4 view", &Layer::Components::LineLight::View, 6);
				VLineLight->SetProperty<Layer::Components::LineLight>("vector3 diffuse", &Layer::Components::LineLight::Diffuse);
				VLineLight->SetProperty<Layer::Components::LineLight>("float emission", &Layer::Components::LineLight::Emission);
				VLineLight->SetMethod("void generate_origin()", &Layer::Components::LineLight::GenerateOrigin);
				PopulateComponentInterface<Layer::Components::LineLight, Layer::Entity*>(*VLineLight, "line_light_component@+ f(scene_entity@+)");

				auto VSurfaceLight = VM->SetClass<Layer::Components::SurfaceLight>("surface_light_component", false);
				VSurfaceLight->SetPropertyArray<Layer::Components::SurfaceLight>("matrix4x4 view", &Layer::Components::SurfaceLight::View, 6);
				VSurfaceLight->SetProperty<Layer::Components::SurfaceLight>("matrix4x4 projection", &Layer::Components::SurfaceLight::Projection);
				VSurfaceLight->SetProperty<Layer::Components::SurfaceLight>("vector3 offset", &Layer::Components::SurfaceLight::Offset);
				VSurfaceLight->SetProperty<Layer::Components::SurfaceLight>("vector3 diffuse", &Layer::Components::SurfaceLight::Diffuse);
				VSurfaceLight->SetProperty<Layer::Components::SurfaceLight>("clock_ticker tick", &Layer::Components::SurfaceLight::Tick);
				VSurfaceLight->SetProperty<Layer::Components::SurfaceLight>("float emission", &Layer::Components::SurfaceLight::Emission);
				VSurfaceLight->SetProperty<Layer::Components::SurfaceLight>("float infinity", &Layer::Components::SurfaceLight::Infinity);
				VSurfaceLight->SetProperty<Layer::Components::SurfaceLight>("bool parallax", &Layer::Components::SurfaceLight::Parallax);
				VSurfaceLight->SetProperty<Layer::Components::SurfaceLight>("bool locked", &Layer::Components::SurfaceLight::Locked);
				VSurfaceLight->SetProperty<Layer::Components::SurfaceLight>("bool static_mask", &Layer::Components::SurfaceLight::StaticMask);
				VSurfaceLight->SetMethod("void set_probe_cache(texture_cube@+)", &Layer::Components::SurfaceLight::SetProbeCache);
				VSurfaceLight->SetMethod("void set_size(const attenuation &in)", &Layer::Components::SurfaceLight::SetSize);
				VSurfaceLight->SetMethod<Layer::Components::SurfaceLight, bool, Graphics::Texture2D*>("void set_diffuse_map(texture_2d@+)", &Layer::Components::SurfaceLight::SetDiffuseMap);
				VSurfaceLight->SetMethod("bool is_image_based() const", &Layer::Components::SurfaceLight::IsImageBased);
				VSurfaceLight->SetMethod("const attenuation& get_size() const", &Layer::Components::SurfaceLight::GetSize);
				VSurfaceLight->SetMethod("texture_cube@+ get_probe_cache() const", &Layer::Components::SurfaceLight::GetProbeCache);
				VSurfaceLight->SetMethod("texture_2d@+ get_diffuse_map_xp() const", &Layer::Components::SurfaceLight::GetDiffuseMapXP);
				VSurfaceLight->SetMethod("texture_2d@+ get_diffuse_map_xn() const", &Layer::Components::SurfaceLight::GetDiffuseMapXN);
				VSurfaceLight->SetMethod("texture_2d@+ get_diffuse_map_yp() const", &Layer::Components::SurfaceLight::GetDiffuseMapYP);
				VSurfaceLight->SetMethod("texture_2d@+ get_diffuse_map_yn() const", &Layer::Components::SurfaceLight::GetDiffuseMapYN);
				VSurfaceLight->SetMethod("texture_2d@+ get_diffuse_map_zp() const", &Layer::Components::SurfaceLight::GetDiffuseMapZP);
				VSurfaceLight->SetMethod("texture_2d@+ get_diffuse_map_zn() const", &Layer::Components::SurfaceLight::GetDiffuseMapZN);
				VSurfaceLight->SetMethod("texture_2d@+ get_diffuse_map() const", &Layer::Components::SurfaceLight::GetDiffuseMap);
				PopulateComponentInterface<Layer::Components::SurfaceLight, Layer::Entity*>(*VSurfaceLight, "surface_light_component@+ f(scene_entity@+)");

				auto VIlluminator = VM->SetClass<Layer::Components::Illuminator>("illuminator_component", false);
				VIlluminator->SetProperty<Layer::Components::Illuminator>("clock_ticker inside", &Layer::Components::Illuminator::Inside);
				VIlluminator->SetProperty<Layer::Components::Illuminator>("clock_ticker outside", &Layer::Components::Illuminator::Outside);
				VIlluminator->SetProperty<Layer::Components::Illuminator>("float ray_step", &Layer::Components::Illuminator::RayStep);
				VIlluminator->SetProperty<Layer::Components::Illuminator>("float max_steps", &Layer::Components::Illuminator::MaxSteps);
				VIlluminator->SetProperty<Layer::Components::Illuminator>("float distance", &Layer::Components::Illuminator::Distance);
				VIlluminator->SetProperty<Layer::Components::Illuminator>("float radiance", &Layer::Components::Illuminator::Radiance);
				VIlluminator->SetProperty<Layer::Components::Illuminator>("float length", &Layer::Components::Illuminator::Length);
				VIlluminator->SetProperty<Layer::Components::Illuminator>("float margin", &Layer::Components::Illuminator::Margin);
				VIlluminator->SetProperty<Layer::Components::Illuminator>("float offset", &Layer::Components::Illuminator::Offset);
				VIlluminator->SetProperty<Layer::Components::Illuminator>("float angle", &Layer::Components::Illuminator::Angle);
				VIlluminator->SetProperty<Layer::Components::Illuminator>("float occlusion", &Layer::Components::Illuminator::Occlusion);
				VIlluminator->SetProperty<Layer::Components::Illuminator>("float specular", &Layer::Components::Illuminator::Specular);
				VIlluminator->SetProperty<Layer::Components::Illuminator>("float bleeding", &Layer::Components::Illuminator::Bleeding);
				VIlluminator->SetProperty<Layer::Components::Illuminator>("bool regenerate", &Layer::Components::Illuminator::Regenerate);
				PopulateComponentInterface<Layer::Components::Illuminator, Layer::Entity*>(*VIlluminator, "illuminator_component@+ f(scene_entity@+)");

				auto VCameraProjection = VM->SetEnum("camera_projection");
				VCameraProjection->SetValue("perspective", (int)Layer::Components::Camera::ProjectionMode::Perspective);
				VCameraProjection->SetValue("orthographic", (int)Layer::Components::Camera::ProjectionMode::Orthographic);

				auto VCamera = VM->SetClass<Layer::Components::Camera>("camera_component", false);
				VCamera->SetProperty<Layer::Components::Camera>("camera_projection mode", &Layer::Components::Camera::Mode);
				VCamera->SetProperty<Layer::Components::Camera>("float near_plane", &Layer::Components::Camera::NearPlane);
				VCamera->SetProperty<Layer::Components::Camera>("float far_plane", &Layer::Components::Camera::FarPlane);
				VCamera->SetProperty<Layer::Components::Camera>("float width", &Layer::Components::Camera::Width);
				VCamera->SetProperty<Layer::Components::Camera>("float height", &Layer::Components::Camera::Height);
				VCamera->SetProperty<Layer::Components::Camera>("float field_of_view", &Layer::Components::Camera::FieldOfView);
				VCamera->SetMethod<Layer::Components::Camera, void, Layer::Viewer*>("void get_viewer(viewer_t &out)", &Layer::Components::Camera::GetViewer);
				VCamera->SetMethod("void resize_buffers()", &Layer::Components::Camera::ResizeBuffers);
				VCamera->SetMethod<Layer::Components::Camera, Layer::Viewer&>("viewer_t& get_viewer()", &Layer::Components::Camera::GetViewer);
				VCamera->SetMethod("render_system@+ get_renderer() const", &Layer::Components::Camera::GetRenderer);
				VCamera->SetMethod("matrix4x4 get_projection() const", &Layer::Components::Camera::GetProjection);
				VCamera->SetMethod("matrix4x4 get_view_projection() const", &Layer::Components::Camera::GetViewProjection);
				VCamera->SetMethod("matrix4x4 get_view() const", &Layer::Components::Camera::GetView);
				VCamera->SetMethod("vector3 get_view_position() const", &Layer::Components::Camera::GetViewPosition);
				VCamera->SetMethod("frustum_8c get_frustum_8c() const", &Layer::Components::Camera::GetFrustum8C);
				VCamera->SetMethod("frustum_6p get_frustum_6p() const", &Layer::Components::Camera::GetFrustum6P);
				VCamera->SetMethod("ray get_screen_ray(const vector2 &in) const", &Layer::Components::Camera::GetScreenRay);
				VCamera->SetMethod("ray get_cursor_ray() const", &Layer::Components::Camera::GetCursorRay);
				VCamera->SetMethod("float get_distance(scene_entity@+) const", &Layer::Components::Camera::GetDistance);
				VCamera->SetMethod("float get_width() const", &Layer::Components::Camera::GetWidth);
				VCamera->SetMethod("float get_height() const", &Layer::Components::Camera::GetHeight);
				VCamera->SetMethod("float get_aspect() const", &Layer::Components::Camera::GetAspect);
				VCamera->SetMethod<Layer::Components::Camera, bool, const Trigonometry::Ray&, Layer::Entity*, Trigonometry::Vector3*>("bool ray_test(const ray &in, scene_entity@+, vector3 &out)", &Layer::Components::Camera::RayTest);
				VCamera->SetMethod<Layer::Components::Camera, bool, const Trigonometry::Ray&, const Trigonometry::Matrix4x4&, Trigonometry::Vector3*>("bool ray_test(const ray &in, const matrix4x4 &in, vector3 &out)", &Layer::Components::Camera::RayTest);
				PopulateComponentInterface<Layer::Components::Camera, Layer::Entity*>(*VCamera, "camera_component@+ f(scene_entity@+)");

				auto VScriptable = VM->SetClass<Layer::Components::Scriptable>("scriptable_component", false);
				PopulateComponentInterface<Layer::Components::Scriptable, Layer::Entity*>(*VScriptable, "scriptable_component@+ f(scene_entity@+)");

				return true;
#else
				VI_ASSERT(false, "<components> is not loaded");
				return false;
#endif
			}
			bool HeavyRegistry::ImportEngineRenderers(VirtualMachine* VM) noexcept
			{
#ifdef VI_BINDINGS
				VI_ASSERT(VM != nullptr, "manager should be set");

				auto VSoftBody = VM->SetClass<Layer::Renderers::SoftBody>("soft_body_renderer", false);
				PopulateRendererInterface<Layer::Renderers::SoftBody, Layer::RenderSystem*>(*VSoftBody, "soft_body_renderer@+ f(render_system@+)");

				auto VModel = VM->SetClass<Layer::Renderers::Model>("model_renderer", false);
				PopulateRendererInterface<Layer::Renderers::Model, Layer::RenderSystem*>(*VModel, "model_renderer@+ f(render_system@+)");

				auto VSkin = VM->SetClass<Layer::Renderers::Skin>("skin_renderer", false);
				PopulateRendererInterface<Layer::Renderers::Skin, Layer::RenderSystem*>(*VSkin, "skin_renderer@+ f(render_system@+)");

				auto VEmitter = VM->SetClass<Layer::Renderers::Emitter>("emitter_renderer", false);
				PopulateRendererInterface<Layer::Renderers::Emitter, Layer::RenderSystem*>(*VEmitter, "emitter_renderer@+ f(render_system@+)");

				auto VDecal = VM->SetClass<Layer::Renderers::Decal>("decal_renderer", false);
				PopulateRendererInterface<Layer::Renderers::Decal, Layer::RenderSystem*>(*VDecal, "decal_renderer@+ f(render_system@+)");

				auto VLightingISurfaceLight = VM->SetPod<Layer::Renderers::Lighting::ISurfaceLight>("lighting_surface_light");
				VLightingISurfaceLight->SetProperty<Layer::Renderers::Lighting::ISurfaceLight>("matrix4x4 world", &Layer::Renderers::Lighting::ISurfaceLight::Transform);
				VLightingISurfaceLight->SetProperty<Layer::Renderers::Lighting::ISurfaceLight>("vector3 position", &Layer::Renderers::Lighting::ISurfaceLight::Position);
				VLightingISurfaceLight->SetProperty<Layer::Renderers::Lighting::ISurfaceLight>("float range", &Layer::Renderers::Lighting::ISurfaceLight::Range);
				VLightingISurfaceLight->SetProperty<Layer::Renderers::Lighting::ISurfaceLight>("vector3 lighting", &Layer::Renderers::Lighting::ISurfaceLight::Lighting);
				VLightingISurfaceLight->SetProperty<Layer::Renderers::Lighting::ISurfaceLight>("float mips", &Layer::Renderers::Lighting::ISurfaceLight::Mips);
				VLightingISurfaceLight->SetProperty<Layer::Renderers::Lighting::ISurfaceLight>("vector3 scale", &Layer::Renderers::Lighting::ISurfaceLight::Scale);
				VLightingISurfaceLight->SetProperty<Layer::Renderers::Lighting::ISurfaceLight>("float parallax", &Layer::Renderers::Lighting::ISurfaceLight::Parallax);
				VLightingISurfaceLight->SetProperty<Layer::Renderers::Lighting::ISurfaceLight>("vector3 attenuations", &Layer::Renderers::Lighting::ISurfaceLight::Attenuation);
				VLightingISurfaceLight->SetProperty<Layer::Renderers::Lighting::ISurfaceLight>("float infinity", &Layer::Renderers::Lighting::ISurfaceLight::Infinity);

				auto VLightingIPointLight = VM->SetPod<Layer::Renderers::Lighting::IPointLight>("lighting_point_light");
				VLightingIPointLight->SetProperty<Layer::Renderers::Lighting::IPointLight>("matrix4x4 world", &Layer::Renderers::Lighting::IPointLight::Transform);
				VLightingIPointLight->SetProperty<Layer::Renderers::Lighting::IPointLight>("vector4 attenuations", &Layer::Renderers::Lighting::IPointLight::Attenuation);
				VLightingIPointLight->SetProperty<Layer::Renderers::Lighting::IPointLight>("vector3 position", &Layer::Renderers::Lighting::IPointLight::Position);
				VLightingIPointLight->SetProperty<Layer::Renderers::Lighting::IPointLight>("float range", &Layer::Renderers::Lighting::IPointLight::Range);
				VLightingIPointLight->SetProperty<Layer::Renderers::Lighting::IPointLight>("vector3 lighting", &Layer::Renderers::Lighting::IPointLight::Lighting);
				VLightingIPointLight->SetProperty<Layer::Renderers::Lighting::IPointLight>("float distance", &Layer::Renderers::Lighting::IPointLight::Distance);
				VLightingIPointLight->SetProperty<Layer::Renderers::Lighting::IPointLight>("float umbra", &Layer::Renderers::Lighting::IPointLight::Umbra);
				VLightingIPointLight->SetProperty<Layer::Renderers::Lighting::IPointLight>("float softness", &Layer::Renderers::Lighting::IPointLight::Softness);
				VLightingIPointLight->SetProperty<Layer::Renderers::Lighting::IPointLight>("float bias", &Layer::Renderers::Lighting::IPointLight::Bias);
				VLightingIPointLight->SetProperty<Layer::Renderers::Lighting::IPointLight>("float iterations", &Layer::Renderers::Lighting::IPointLight::Iterations);

				auto VLightingISpotLight = VM->SetPod<Layer::Renderers::Lighting::ISpotLight>("lighting_spot_light");
				VLightingISpotLight->SetProperty<Layer::Renderers::Lighting::ISpotLight>("matrix4x4 world", &Layer::Renderers::Lighting::ISpotLight::Transform);
				VLightingISpotLight->SetProperty<Layer::Renderers::Lighting::ISpotLight>("matrix4x4 view_projection", &Layer::Renderers::Lighting::ISpotLight::ViewProjection);
				VLightingISpotLight->SetProperty<Layer::Renderers::Lighting::ISpotLight>("vector4 attenuations", &Layer::Renderers::Lighting::ISpotLight::Attenuation);
				VLightingISpotLight->SetProperty<Layer::Renderers::Lighting::ISpotLight>("vector3 direction", &Layer::Renderers::Lighting::ISpotLight::Direction);
				VLightingISpotLight->SetProperty<Layer::Renderers::Lighting::ISpotLight>("float cutoff", &Layer::Renderers::Lighting::ISpotLight::Cutoff);
				VLightingISpotLight->SetProperty<Layer::Renderers::Lighting::ISpotLight>("vector3 position", &Layer::Renderers::Lighting::ISpotLight::Position);
				VLightingISpotLight->SetProperty<Layer::Renderers::Lighting::ISpotLight>("float range", &Layer::Renderers::Lighting::ISpotLight::Range);
				VLightingISpotLight->SetProperty<Layer::Renderers::Lighting::ISpotLight>("vector3 lighting", &Layer::Renderers::Lighting::ISpotLight::Lighting);
				VLightingISpotLight->SetProperty<Layer::Renderers::Lighting::ISpotLight>("float softness", &Layer::Renderers::Lighting::ISpotLight::Softness);
				VLightingISpotLight->SetProperty<Layer::Renderers::Lighting::ISpotLight>("float bias", &Layer::Renderers::Lighting::ISpotLight::Bias);
				VLightingISpotLight->SetProperty<Layer::Renderers::Lighting::ISpotLight>("float iterations", &Layer::Renderers::Lighting::ISpotLight::Iterations);
				VLightingISpotLight->SetProperty<Layer::Renderers::Lighting::ISpotLight>("float umbra", &Layer::Renderers::Lighting::ISpotLight::Umbra);
				VLightingISpotLight->SetProperty<Layer::Renderers::Lighting::ISpotLight>("float padding", &Layer::Renderers::Lighting::ISpotLight::Padding);

				auto VLightingILineLight = VM->SetPod<Layer::Renderers::Lighting::ILineLight>("lighting_line_light");
				VLightingILineLight->SetPropertyArray<Layer::Renderers::Lighting::ILineLight>("matrix4x4 view_projection", &Layer::Renderers::Lighting::ILineLight::ViewProjection, 6);
				VLightingILineLight->SetProperty<Layer::Renderers::Lighting::ILineLight>("matrix4x4 sky_offset", &Layer::Renderers::Lighting::ILineLight::SkyOffset);
				VLightingILineLight->SetProperty<Layer::Renderers::Lighting::ILineLight>("vector3 rlh_emission", &Layer::Renderers::Lighting::ILineLight::RlhEmission);
				VLightingILineLight->SetProperty<Layer::Renderers::Lighting::ILineLight>("float rlh_height", &Layer::Renderers::Lighting::ILineLight::RlhHeight);
				VLightingILineLight->SetProperty<Layer::Renderers::Lighting::ILineLight>("vector3 mie_emission", &Layer::Renderers::Lighting::ILineLight::MieEmission);
				VLightingILineLight->SetProperty<Layer::Renderers::Lighting::ILineLight>("float mie_height", &Layer::Renderers::Lighting::ILineLight::MieHeight);
				VLightingILineLight->SetProperty<Layer::Renderers::Lighting::ILineLight>("vector3 lighting", &Layer::Renderers::Lighting::ILineLight::Lighting);
				VLightingILineLight->SetProperty<Layer::Renderers::Lighting::ILineLight>("float softness", &Layer::Renderers::Lighting::ILineLight::Softness);
				VLightingILineLight->SetProperty<Layer::Renderers::Lighting::ILineLight>("vector3 position", &Layer::Renderers::Lighting::ILineLight::Position);
				VLightingILineLight->SetProperty<Layer::Renderers::Lighting::ILineLight>("float cascades", &Layer::Renderers::Lighting::ILineLight::Cascades);
				VLightingILineLight->SetProperty<Layer::Renderers::Lighting::ILineLight>("vector2 padding", &Layer::Renderers::Lighting::ILineLight::Padding);
				VLightingILineLight->SetProperty<Layer::Renderers::Lighting::ILineLight>("float bias", &Layer::Renderers::Lighting::ILineLight::Bias);
				VLightingILineLight->SetProperty<Layer::Renderers::Lighting::ILineLight>("float iterations", &Layer::Renderers::Lighting::ILineLight::Iterations);
				VLightingILineLight->SetProperty<Layer::Renderers::Lighting::ILineLight>("float scatter_intensity", &Layer::Renderers::Lighting::ILineLight::ScatterIntensity);
				VLightingILineLight->SetProperty<Layer::Renderers::Lighting::ILineLight>("float planet_radius", &Layer::Renderers::Lighting::ILineLight::PlanetRadius);
				VLightingILineLight->SetProperty<Layer::Renderers::Lighting::ILineLight>("float atmosphere_radius", &Layer::Renderers::Lighting::ILineLight::AtmosphereRadius);
				VLightingILineLight->SetProperty<Layer::Renderers::Lighting::ILineLight>("float mie_direction", &Layer::Renderers::Lighting::ILineLight::MieDirection);

				auto VLightingIAmbientLight = VM->SetPod<Layer::Renderers::Lighting::IAmbientLight>("lighting_ambient_light");
				VLightingIAmbientLight->SetProperty<Layer::Renderers::Lighting::IAmbientLight>("matrix4x4 sky_offset", &Layer::Renderers::Lighting::IAmbientLight::SkyOffset);
				VLightingIAmbientLight->SetProperty<Layer::Renderers::Lighting::IAmbientLight>("vector3 high_emission", &Layer::Renderers::Lighting::IAmbientLight::HighEmission);
				VLightingIAmbientLight->SetProperty<Layer::Renderers::Lighting::IAmbientLight>("float sky_emission", &Layer::Renderers::Lighting::IAmbientLight::SkyEmission);
				VLightingIAmbientLight->SetProperty<Layer::Renderers::Lighting::IAmbientLight>("vector3 low_emission", &Layer::Renderers::Lighting::IAmbientLight::LowEmission);
				VLightingIAmbientLight->SetProperty<Layer::Renderers::Lighting::IAmbientLight>("float light_emission", &Layer::Renderers::Lighting::IAmbientLight::LightEmission);
				VLightingIAmbientLight->SetProperty<Layer::Renderers::Lighting::IAmbientLight>("vector3 sky_color", &Layer::Renderers::Lighting::IAmbientLight::SkyColor);
				VLightingIAmbientLight->SetProperty<Layer::Renderers::Lighting::IAmbientLight>("float fog_far_off", &Layer::Renderers::Lighting::IAmbientLight::FogFarOff);
				VLightingIAmbientLight->SetProperty<Layer::Renderers::Lighting::IAmbientLight>("vector3 fog_color", &Layer::Renderers::Lighting::IAmbientLight::FogColor);
				VLightingIAmbientLight->SetProperty<Layer::Renderers::Lighting::IAmbientLight>("float fog_near_off", &Layer::Renderers::Lighting::IAmbientLight::FogNearOff);
				VLightingIAmbientLight->SetProperty<Layer::Renderers::Lighting::IAmbientLight>("vector3 fog_far", &Layer::Renderers::Lighting::IAmbientLight::FogFar);
				VLightingIAmbientLight->SetProperty<Layer::Renderers::Lighting::IAmbientLight>("float fog_amount", &Layer::Renderers::Lighting::IAmbientLight::FogAmount);
				VLightingIAmbientLight->SetProperty<Layer::Renderers::Lighting::IAmbientLight>("vector3 fog_near", &Layer::Renderers::Lighting::IAmbientLight::FogNear);
				VLightingIAmbientLight->SetProperty<Layer::Renderers::Lighting::IAmbientLight>("float recursive", &Layer::Renderers::Lighting::IAmbientLight::Recursive);

				auto VLightingIVoxelBuffer = VM->SetPod<Layer::Renderers::Lighting::IVoxelBuffer>("lighting_voxel_buffer");
				VLightingIVoxelBuffer->SetProperty<Layer::Renderers::Lighting::IVoxelBuffer>("matrix4x4 world", &Layer::Renderers::Lighting::IVoxelBuffer::Transform);
				VLightingIVoxelBuffer->SetProperty<Layer::Renderers::Lighting::IVoxelBuffer>("vector3 center", &Layer::Renderers::Lighting::IVoxelBuffer::Center);
				VLightingIVoxelBuffer->SetProperty<Layer::Renderers::Lighting::IVoxelBuffer>("float ray_step", &Layer::Renderers::Lighting::IVoxelBuffer::RayStep);
				VLightingIVoxelBuffer->SetProperty<Layer::Renderers::Lighting::IVoxelBuffer>("vector3 size", &Layer::Renderers::Lighting::IVoxelBuffer::Size);
				VLightingIVoxelBuffer->SetProperty<Layer::Renderers::Lighting::IVoxelBuffer>("float mips", &Layer::Renderers::Lighting::IVoxelBuffer::Mips);
				VLightingIVoxelBuffer->SetProperty<Layer::Renderers::Lighting::IVoxelBuffer>("vector3 scale", &Layer::Renderers::Lighting::IVoxelBuffer::Scale);
				VLightingIVoxelBuffer->SetProperty<Layer::Renderers::Lighting::IVoxelBuffer>("float max_steps", &Layer::Renderers::Lighting::IVoxelBuffer::MaxSteps);
				VLightingIVoxelBuffer->SetProperty<Layer::Renderers::Lighting::IVoxelBuffer>("vector3 lights", &Layer::Renderers::Lighting::IVoxelBuffer::Lights);
				VLightingIVoxelBuffer->SetProperty<Layer::Renderers::Lighting::IVoxelBuffer>("float radiance", &Layer::Renderers::Lighting::IVoxelBuffer::Radiance);
				VLightingIVoxelBuffer->SetProperty<Layer::Renderers::Lighting::IVoxelBuffer>("float margin", &Layer::Renderers::Lighting::IVoxelBuffer::Margin);
				VLightingIVoxelBuffer->SetProperty<Layer::Renderers::Lighting::IVoxelBuffer>("float offset", &Layer::Renderers::Lighting::IVoxelBuffer::Offset);
				VLightingIVoxelBuffer->SetProperty<Layer::Renderers::Lighting::IVoxelBuffer>("float angle", &Layer::Renderers::Lighting::IVoxelBuffer::Angle);
				VLightingIVoxelBuffer->SetProperty<Layer::Renderers::Lighting::IVoxelBuffer>("float length", &Layer::Renderers::Lighting::IVoxelBuffer::Length);
				VLightingIVoxelBuffer->SetProperty<Layer::Renderers::Lighting::IVoxelBuffer>("float distance", &Layer::Renderers::Lighting::IVoxelBuffer::Distance);
				VLightingIVoxelBuffer->SetProperty<Layer::Renderers::Lighting::IVoxelBuffer>("float occlusion", &Layer::Renderers::Lighting::IVoxelBuffer::Occlusion);
				VLightingIVoxelBuffer->SetProperty<Layer::Renderers::Lighting::IVoxelBuffer>("float specular", &Layer::Renderers::Lighting::IVoxelBuffer::Specular);
				VLightingIVoxelBuffer->SetProperty<Layer::Renderers::Lighting::IVoxelBuffer>("float bleeding", &Layer::Renderers::Lighting::IVoxelBuffer::Bleeding);

				auto VLighting = VM->SetClass<Layer::Renderers::Lighting>("lighting_renderer", false);
				VLighting->SetProperty<Layer::Renderers::Lighting>("lighting_ambient_light ambient_light", &Layer::Renderers::Lighting::AmbientLight);
				VLighting->SetProperty<Layer::Renderers::Lighting>("lighting_voxel_buffer voxel_buffer", &Layer::Renderers::Lighting::VoxelBuffer);
				VLighting->SetProperty<Layer::Renderers::Lighting>("bool enable_gi", &Layer::Renderers::Lighting::EnableGI);
				VLighting->SetMethod("void set_sky_map(texture_2d@+)", &Layer::Renderers::Lighting::SetSkyMap);
				VLighting->SetMethod("void set_surface_buffer_size(usize)", &Layer::Renderers::Lighting::SetSurfaceBufferSize);
				VLighting->SetMethod("texture_cube@+ get_sky_map() const", &Layer::Renderers::Lighting::GetSkyMap);
				VLighting->SetMethod("texture_2d@+ get_sky_base() const", &Layer::Renderers::Lighting::GetSkyBase);
				PopulateRendererInterface<Layer::Renderers::Lighting, Layer::RenderSystem*>(*VLighting, "lighting_renderer@+ f(render_system@+)");

				auto VTransparencyRenderConstant = VM->SetPod<Layer::Renderers::Transparency::RenderConstant>("transparency_render_constant");
				VTransparencyRenderConstant->SetProperty<Layer::Renderers::Transparency::RenderConstant>("vector3 padding", &Layer::Renderers::Transparency::RenderConstant::Padding);
				VTransparencyRenderConstant->SetProperty<Layer::Renderers::Transparency::RenderConstant>("float mips", &Layer::Renderers::Transparency::RenderConstant::Mips);

				auto VTransparency = VM->SetClass<Layer::Renderers::Transparency>("transparency_renderer", false);
				VTransparency->SetProperty<Layer::Renderers::Transparency>("transparency_render_constant render_data", &Layer::Renderers::Transparency::RenderData);
				PopulateRendererInterface<Layer::Renderers::Transparency, Layer::RenderSystem*>(*VTransparency, "transparency_renderer@+ f(render_system@+)");

				auto VSSRReflectanceBuffer = VM->SetPod<Layer::Renderers::SSR::ReflectanceBuffer>("ssr_reflectance_buffer");
				VSSRReflectanceBuffer->SetProperty<Layer::Renderers::SSR::ReflectanceBuffer>("float samples", &Layer::Renderers::SSR::ReflectanceBuffer::Samples);
				VSSRReflectanceBuffer->SetProperty<Layer::Renderers::SSR::ReflectanceBuffer>("float padding", &Layer::Renderers::SSR::ReflectanceBuffer::Padding);
				VSSRReflectanceBuffer->SetProperty<Layer::Renderers::SSR::ReflectanceBuffer>("float intensity", &Layer::Renderers::SSR::ReflectanceBuffer::Intensity);
				VSSRReflectanceBuffer->SetProperty<Layer::Renderers::SSR::ReflectanceBuffer>("float distance", &Layer::Renderers::SSR::ReflectanceBuffer::Distance);

				auto VSSRGlossBuffer = VM->SetPod<Layer::Renderers::SSR::GlossBuffer>("ssr_gloss_buffer");
				VSSRGlossBuffer->SetProperty<Layer::Renderers::SSR::GlossBuffer>("float padding", &Layer::Renderers::SSR::GlossBuffer::Padding);
				VSSRGlossBuffer->SetProperty<Layer::Renderers::SSR::GlossBuffer>("float deadzone", &Layer::Renderers::SSR::GlossBuffer::Deadzone);
				VSSRGlossBuffer->SetProperty<Layer::Renderers::SSR::GlossBuffer>("float mips", &Layer::Renderers::SSR::GlossBuffer::Mips);
				VSSRGlossBuffer->SetProperty<Layer::Renderers::SSR::GlossBuffer>("float cutoff", &Layer::Renderers::SSR::GlossBuffer::Cutoff);
				VSSRGlossBuffer->SetPropertyArray<Layer::Renderers::SSR::GlossBuffer>("float texel", &Layer::Renderers::SSR::GlossBuffer::Texel, 2);
				VSSRGlossBuffer->SetProperty<Layer::Renderers::SSR::GlossBuffer>("float samples", &Layer::Renderers::SSR::GlossBuffer::Samples);
				VSSRGlossBuffer->SetProperty<Layer::Renderers::SSR::GlossBuffer>("float blur", &Layer::Renderers::SSR::GlossBuffer::Blur);

				auto VSSR = VM->SetClass<Layer::Renderers::SSR>("ssr_renderer", false);
				VSSR->SetProperty<Layer::Renderers::SSR>("ssr_reflectance_buffer reflectance", &Layer::Renderers::SSR::Reflectance);
				VSSR->SetProperty<Layer::Renderers::SSR>("ssr_gloss_buffer gloss", &Layer::Renderers::SSR::Gloss);
				PopulateRendererInterface<Layer::Renderers::SSR, Layer::RenderSystem*>(*VSSR, "ssr_renderer@+ f(render_system@+)");

				auto VSSGIStochasticBuffer = VM->SetPod<Layer::Renderers::SSGI::StochasticBuffer>("ssgi_stochastic_buffer");
				VSSGIStochasticBuffer->SetPropertyArray<Layer::Renderers::SSGI::StochasticBuffer>("float texel", &Layer::Renderers::SSGI::StochasticBuffer::Texel, 2);
				VSSGIStochasticBuffer->SetProperty<Layer::Renderers::SSGI::StochasticBuffer>("float frame_id", &Layer::Renderers::SSGI::StochasticBuffer::FrameId);
				VSSGIStochasticBuffer->SetProperty<Layer::Renderers::SSGI::StochasticBuffer>("float padding", &Layer::Renderers::SSGI::StochasticBuffer::Padding);

				auto VSSGIIndirectionBuffer = VM->SetPod<Layer::Renderers::SSGI::IndirectionBuffer>("ssgi_indirection_buffer");
				VSSGIIndirectionBuffer->SetPropertyArray<Layer::Renderers::SSGI::IndirectionBuffer>("float random", &Layer::Renderers::SSGI::IndirectionBuffer::Random, 2);
				VSSGIIndirectionBuffer->SetProperty<Layer::Renderers::SSGI::IndirectionBuffer>("float samples", &Layer::Renderers::SSGI::IndirectionBuffer::Samples);
				VSSGIIndirectionBuffer->SetProperty<Layer::Renderers::SSGI::IndirectionBuffer>("float distance", &Layer::Renderers::SSGI::IndirectionBuffer::Distance);
				VSSGIIndirectionBuffer->SetProperty<Layer::Renderers::SSGI::IndirectionBuffer>("float initial", &Layer::Renderers::SSGI::IndirectionBuffer::Initial);
				VSSGIIndirectionBuffer->SetProperty<Layer::Renderers::SSGI::IndirectionBuffer>("float cutoff", &Layer::Renderers::SSGI::IndirectionBuffer::Cutoff);
				VSSGIIndirectionBuffer->SetProperty<Layer::Renderers::SSGI::IndirectionBuffer>("float attenuations", &Layer::Renderers::SSGI::IndirectionBuffer::Attenuation);
				VSSGIIndirectionBuffer->SetProperty<Layer::Renderers::SSGI::IndirectionBuffer>("float swing", &Layer::Renderers::SSGI::IndirectionBuffer::Swing);
				VSSGIIndirectionBuffer->SetPropertyArray<Layer::Renderers::SSGI::IndirectionBuffer>("float padding", &Layer::Renderers::SSGI::IndirectionBuffer::Padding, 3);
				VSSGIIndirectionBuffer->SetProperty<Layer::Renderers::SSGI::IndirectionBuffer>("float bias", &Layer::Renderers::SSGI::IndirectionBuffer::Bias);

				auto VSSGIDenoiseBuffer = VM->SetPod<Layer::Renderers::SSGI::DenoiseBuffer>("ssgi_denoise_buffer");
				VSSGIDenoiseBuffer->SetPropertyArray<Layer::Renderers::SSGI::DenoiseBuffer>("float padding", &Layer::Renderers::SSGI::DenoiseBuffer::Padding, 3);
				VSSGIDenoiseBuffer->SetProperty<Layer::Renderers::SSGI::DenoiseBuffer>("float cutoff", &Layer::Renderers::SSGI::DenoiseBuffer::Cutoff);
				VSSGIDenoiseBuffer->SetPropertyArray<Layer::Renderers::SSGI::DenoiseBuffer>("float texel", &Layer::Renderers::SSGI::DenoiseBuffer::Texel, 2);
				VSSGIDenoiseBuffer->SetProperty<Layer::Renderers::SSGI::DenoiseBuffer>("float samples", &Layer::Renderers::SSGI::DenoiseBuffer::Samples);
				VSSGIDenoiseBuffer->SetProperty<Layer::Renderers::SSGI::DenoiseBuffer>("float blur", &Layer::Renderers::SSGI::DenoiseBuffer::Blur);

				auto VSSGI = VM->SetClass<Layer::Renderers::SSGI>("ssgi_renderer", false);
				VSSGI->SetProperty<Layer::Renderers::SSGI>("ssgi_stochastic_buffer stochastic", &Layer::Renderers::SSGI::Stochastic);
				VSSGI->SetProperty<Layer::Renderers::SSGI>("ssgi_indirection_buffer indirection", &Layer::Renderers::SSGI::Indirection);
				VSSGI->SetProperty<Layer::Renderers::SSGI>("ssgi_denoise_buffer denoise", &Layer::Renderers::SSGI::Indirection);
				VSSGI->SetProperty<Layer::Renderers::SSGI>("uint32 bounces", &Layer::Renderers::SSGI::Bounces);
				PopulateRendererInterface<Layer::Renderers::SSGI, Layer::RenderSystem*>(*VSSGI, "ssgi_renderer@+ f(render_system@+)");

				auto VSSAOShadingBuffer = VM->SetPod<Layer::Renderers::SSAO::ShadingBuffer>("ssao_shading_buffer");
				VSSAOShadingBuffer->SetProperty<Layer::Renderers::SSAO::ShadingBuffer>("float samples", &Layer::Renderers::SSAO::ShadingBuffer::Samples);
				VSSAOShadingBuffer->SetProperty<Layer::Renderers::SSAO::ShadingBuffer>("float intensity", &Layer::Renderers::SSAO::ShadingBuffer::Intensity);
				VSSAOShadingBuffer->SetProperty<Layer::Renderers::SSAO::ShadingBuffer>("float scale", &Layer::Renderers::SSAO::ShadingBuffer::Scale);
				VSSAOShadingBuffer->SetProperty<Layer::Renderers::SSAO::ShadingBuffer>("float bias", &Layer::Renderers::SSAO::ShadingBuffer::Bias);
				VSSAOShadingBuffer->SetProperty<Layer::Renderers::SSAO::ShadingBuffer>("float radius", &Layer::Renderers::SSAO::ShadingBuffer::Radius);
				VSSAOShadingBuffer->SetProperty<Layer::Renderers::SSAO::ShadingBuffer>("float distance", &Layer::Renderers::SSAO::ShadingBuffer::Distance);
				VSSAOShadingBuffer->SetProperty<Layer::Renderers::SSAO::ShadingBuffer>("float fade", &Layer::Renderers::SSAO::ShadingBuffer::Fade);
				VSSAOShadingBuffer->SetProperty<Layer::Renderers::SSAO::ShadingBuffer>("float padding", &Layer::Renderers::SSAO::ShadingBuffer::Padding);

				auto VSSAOFiboBuffer = VM->SetPod<Layer::Renderers::SSAO::FiboBuffer>("ssao_fibo_buffer");
				VSSAOFiboBuffer->SetPropertyArray<Layer::Renderers::SSAO::FiboBuffer>("float padding", &Layer::Renderers::SSAO::FiboBuffer::Padding, 3);
				VSSAOFiboBuffer->SetProperty<Layer::Renderers::SSAO::FiboBuffer>("float power", &Layer::Renderers::SSAO::FiboBuffer::Power);
				VSSAOFiboBuffer->SetPropertyArray<Layer::Renderers::SSAO::FiboBuffer>("float texel", &Layer::Renderers::SSAO::FiboBuffer::Texel, 2);
				VSSAOFiboBuffer->SetProperty<Layer::Renderers::SSAO::FiboBuffer>("float samples", &Layer::Renderers::SSAO::FiboBuffer::Samples);
				VSSAOFiboBuffer->SetProperty<Layer::Renderers::SSAO::FiboBuffer>("float blur", &Layer::Renderers::SSAO::FiboBuffer::Blur);

				auto VSSAO = VM->SetClass<Layer::Renderers::SSAO>("ssao_renderer", false);
				VSSAO->SetProperty<Layer::Renderers::SSAO>("ssao_shading_buffer shading", &Layer::Renderers::SSAO::Shading);
				VSSAO->SetProperty<Layer::Renderers::SSAO>("ssao_fibo_buffer fibo", &Layer::Renderers::SSAO::Fibo);
				PopulateRendererInterface<Layer::Renderers::SSAO, Layer::RenderSystem*>(*VSSAO, "ssao_renderer@+ f(render_system@+)");

				auto VDoFFocusBuffer = VM->SetPod<Layer::Renderers::DoF::FocusBuffer>("dof_focus_buffer");
				VDoFFocusBuffer->SetPropertyArray<Layer::Renderers::DoF::FocusBuffer>("float texel", &Layer::Renderers::DoF::FocusBuffer::Texel, 2);
				VDoFFocusBuffer->SetProperty<Layer::Renderers::DoF::FocusBuffer>("float radius", &Layer::Renderers::DoF::FocusBuffer::Radius);
				VDoFFocusBuffer->SetProperty<Layer::Renderers::DoF::FocusBuffer>("float bokeh", &Layer::Renderers::DoF::FocusBuffer::Bokeh);
				VDoFFocusBuffer->SetPropertyArray<Layer::Renderers::DoF::FocusBuffer>("float padding", &Layer::Renderers::DoF::FocusBuffer::Padding, 3);
				VDoFFocusBuffer->SetProperty<Layer::Renderers::DoF::FocusBuffer>("float scale", &Layer::Renderers::DoF::FocusBuffer::Scale);
				VDoFFocusBuffer->SetProperty<Layer::Renderers::DoF::FocusBuffer>("float near_distance", &Layer::Renderers::DoF::FocusBuffer::NearDistance);
				VDoFFocusBuffer->SetProperty<Layer::Renderers::DoF::FocusBuffer>("float near_range", &Layer::Renderers::DoF::FocusBuffer::NearRange);
				VDoFFocusBuffer->SetProperty<Layer::Renderers::DoF::FocusBuffer>("float far_distance", &Layer::Renderers::DoF::FocusBuffer::FarDistance);
				VDoFFocusBuffer->SetProperty<Layer::Renderers::DoF::FocusBuffer>("float far_range", &Layer::Renderers::DoF::FocusBuffer::FarRange);

				auto VDoF = VM->SetClass<Layer::Renderers::DoF>("dof_renderer", false);
				VDoF->SetProperty<Layer::Renderers::DoF>("dof_focus_buffer focus", &Layer::Renderers::DoF::Focus);
				VDoF->SetProperty<Layer::Renderers::DoF>("float distance", &Layer::Renderers::DoF::Distance);
				VDoF->SetProperty<Layer::Renderers::DoF>("float radius", &Layer::Renderers::DoF::Radius);
				VDoF->SetProperty<Layer::Renderers::DoF>("float time", &Layer::Renderers::DoF::Time);
				VDoF->SetMethod("void focus_at_nearest_target(float)", &Layer::Renderers::DoF::FocusAtNearestTarget);
				PopulateRendererInterface<Layer::Renderers::DoF, Layer::RenderSystem*>(*VDoF, "dof_renderer@+ f(render_system@+)");

				auto VMotionBlurVelocityBuffer = VM->SetPod<Layer::Renderers::MotionBlur::VelocityBuffer>("motion_blur_velocity_buffer");
				VMotionBlurVelocityBuffer->SetProperty<Layer::Renderers::MotionBlur::VelocityBuffer>("matrix4x4 last_view_projection", &Layer::Renderers::MotionBlur::VelocityBuffer::LastViewProjection);

				auto VMotionBlurMotionBuffer = VM->SetPod<Layer::Renderers::MotionBlur::MotionBuffer>("motion_blur_motion_buffer");
				VMotionBlurMotionBuffer->SetProperty<Layer::Renderers::MotionBlur::MotionBuffer>("float samples", &Layer::Renderers::MotionBlur::MotionBuffer::Samples);
				VMotionBlurMotionBuffer->SetProperty<Layer::Renderers::MotionBlur::MotionBuffer>("float blur", &Layer::Renderers::MotionBlur::MotionBuffer::Blur);
				VMotionBlurMotionBuffer->SetProperty<Layer::Renderers::MotionBlur::MotionBuffer>("float motion", &Layer::Renderers::MotionBlur::MotionBuffer::Motion);
				VMotionBlurMotionBuffer->SetProperty<Layer::Renderers::MotionBlur::MotionBuffer>("float padding", &Layer::Renderers::MotionBlur::MotionBuffer::Padding);

				auto VMotionBlur = VM->SetClass<Layer::Renderers::MotionBlur>("motion_blur_renderer", false);
				VMotionBlur->SetProperty<Layer::Renderers::MotionBlur>("motion_blur_velocity_buffer velocity", &Layer::Renderers::MotionBlur::Velocity);
				VMotionBlur->SetProperty<Layer::Renderers::MotionBlur>("motion_blur_motion_buffer motion", &Layer::Renderers::MotionBlur::Motion);
				PopulateRendererInterface<Layer::Renderers::MotionBlur, Layer::RenderSystem*>(*VMotionBlur, "motion_blur_renderer@+ f(render_system@+)");

				auto VBloomExtractionBuffer = VM->SetPod<Layer::Renderers::Bloom::ExtractionBuffer>("bloom_extraction_buffer");
				VBloomExtractionBuffer->SetPropertyArray<Layer::Renderers::Bloom::ExtractionBuffer>("float padding", &Layer::Renderers::Bloom::ExtractionBuffer::Padding, 2);
				VBloomExtractionBuffer->SetProperty<Layer::Renderers::Bloom::ExtractionBuffer>("float intensity", &Layer::Renderers::Bloom::ExtractionBuffer::Intensity);
				VBloomExtractionBuffer->SetProperty<Layer::Renderers::Bloom::ExtractionBuffer>("float threshold", &Layer::Renderers::Bloom::ExtractionBuffer::Threshold);

				auto VBloomFiboBuffer = VM->SetPod<Layer::Renderers::Bloom::FiboBuffer>("bloom_fibo_buffer");
				VBloomFiboBuffer->SetPropertyArray<Layer::Renderers::Bloom::FiboBuffer>("float padding", &Layer::Renderers::Bloom::FiboBuffer::Padding, 3);
				VBloomFiboBuffer->SetProperty<Layer::Renderers::Bloom::FiboBuffer>("float power", &Layer::Renderers::Bloom::FiboBuffer::Power);
				VBloomFiboBuffer->SetPropertyArray<Layer::Renderers::Bloom::FiboBuffer>("float texel", &Layer::Renderers::Bloom::FiboBuffer::Texel, 2);
				VBloomFiboBuffer->SetProperty<Layer::Renderers::Bloom::FiboBuffer>("float samples", &Layer::Renderers::Bloom::FiboBuffer::Samples);
				VBloomFiboBuffer->SetProperty<Layer::Renderers::Bloom::FiboBuffer>("float blur", &Layer::Renderers::Bloom::FiboBuffer::Blur);

				auto VBloom = VM->SetClass<Layer::Renderers::Bloom>("bloom_renderer", false);
				VBloom->SetProperty<Layer::Renderers::Bloom>("bloom_extraction_buffer extraction", &Layer::Renderers::Bloom::Extraction);
				VBloom->SetProperty<Layer::Renderers::Bloom>("bloom_fibo_buffer fibo", &Layer::Renderers::Bloom::Fibo);
				PopulateRendererInterface<Layer::Renderers::Bloom, Layer::RenderSystem*>(*VBloom, "bloom_renderer@+ f(render_system@+)");

				auto VToneLuminanceBuffer = VM->SetPod<Layer::Renderers::Tone::LuminanceBuffer>("tone_luminance_buffer");
				VToneLuminanceBuffer->SetPropertyArray<Layer::Renderers::Tone::LuminanceBuffer>("float texel", &Layer::Renderers::Tone::LuminanceBuffer::Texel, 2);
				VToneLuminanceBuffer->SetProperty<Layer::Renderers::Tone::LuminanceBuffer>("float mips", &Layer::Renderers::Tone::LuminanceBuffer::Mips);
				VToneLuminanceBuffer->SetProperty<Layer::Renderers::Tone::LuminanceBuffer>("float time", &Layer::Renderers::Tone::LuminanceBuffer::Time);

				auto VToneMappingBuffer = VM->SetPod<Layer::Renderers::Tone::MappingBuffer>("tone_mapping_buffer");
				VToneMappingBuffer->SetPropertyArray<Layer::Renderers::Tone::MappingBuffer>("float padding", &Layer::Renderers::Tone::MappingBuffer::Padding, 2);
				VToneMappingBuffer->SetProperty<Layer::Renderers::Tone::MappingBuffer>("float grayscale", &Layer::Renderers::Tone::MappingBuffer::Grayscale);
				VToneMappingBuffer->SetProperty<Layer::Renderers::Tone::MappingBuffer>("float aces", &Layer::Renderers::Tone::MappingBuffer::ACES);
				VToneMappingBuffer->SetProperty<Layer::Renderers::Tone::MappingBuffer>("float filmic", &Layer::Renderers::Tone::MappingBuffer::Filmic);
				VToneMappingBuffer->SetProperty<Layer::Renderers::Tone::MappingBuffer>("float lottes", &Layer::Renderers::Tone::MappingBuffer::Lottes);
				VToneMappingBuffer->SetProperty<Layer::Renderers::Tone::MappingBuffer>("float reinhard", &Layer::Renderers::Tone::MappingBuffer::Reinhard);
				VToneMappingBuffer->SetProperty<Layer::Renderers::Tone::MappingBuffer>("float reinhard2", &Layer::Renderers::Tone::MappingBuffer::Reinhard2);
				VToneMappingBuffer->SetProperty<Layer::Renderers::Tone::MappingBuffer>("float unreal", &Layer::Renderers::Tone::MappingBuffer::Unreal);
				VToneMappingBuffer->SetProperty<Layer::Renderers::Tone::MappingBuffer>("float uchimura", &Layer::Renderers::Tone::MappingBuffer::Uchimura);
				VToneMappingBuffer->SetProperty<Layer::Renderers::Tone::MappingBuffer>("float ubrightness", &Layer::Renderers::Tone::MappingBuffer::UBrightness);
				VToneMappingBuffer->SetProperty<Layer::Renderers::Tone::MappingBuffer>("float ucontrast", &Layer::Renderers::Tone::MappingBuffer::UContrast);
				VToneMappingBuffer->SetProperty<Layer::Renderers::Tone::MappingBuffer>("float ustart", &Layer::Renderers::Tone::MappingBuffer::UStart);
				VToneMappingBuffer->SetProperty<Layer::Renderers::Tone::MappingBuffer>("float ulength", &Layer::Renderers::Tone::MappingBuffer::ULength);
				VToneMappingBuffer->SetProperty<Layer::Renderers::Tone::MappingBuffer>("float ublack", &Layer::Renderers::Tone::MappingBuffer::UBlack);
				VToneMappingBuffer->SetProperty<Layer::Renderers::Tone::MappingBuffer>("float upedestal", &Layer::Renderers::Tone::MappingBuffer::UPedestal);
				VToneMappingBuffer->SetProperty<Layer::Renderers::Tone::MappingBuffer>("float exposure", &Layer::Renderers::Tone::MappingBuffer::Exposure);
				VToneMappingBuffer->SetProperty<Layer::Renderers::Tone::MappingBuffer>("float eintensity", &Layer::Renderers::Tone::MappingBuffer::EIntensity);
				VToneMappingBuffer->SetProperty<Layer::Renderers::Tone::MappingBuffer>("float egamma", &Layer::Renderers::Tone::MappingBuffer::EGamma);
				VToneMappingBuffer->SetProperty<Layer::Renderers::Tone::MappingBuffer>("float adaptation", &Layer::Renderers::Tone::MappingBuffer::Adaptation);
				VToneMappingBuffer->SetProperty<Layer::Renderers::Tone::MappingBuffer>("float ahray", &Layer::Renderers::Tone::MappingBuffer::AGray);
				VToneMappingBuffer->SetProperty<Layer::Renderers::Tone::MappingBuffer>("float awhite", &Layer::Renderers::Tone::MappingBuffer::AWhite);
				VToneMappingBuffer->SetProperty<Layer::Renderers::Tone::MappingBuffer>("float ablack", &Layer::Renderers::Tone::MappingBuffer::ABlack);
				VToneMappingBuffer->SetProperty<Layer::Renderers::Tone::MappingBuffer>("float aspeed", &Layer::Renderers::Tone::MappingBuffer::ASpeed);

				auto VTone = VM->SetClass<Layer::Renderers::Tone>("tone_renderer", false);
				VTone->SetProperty<Layer::Renderers::Tone>("tone_luminance_buffer luminance", &Layer::Renderers::Tone::Luminance);
				VTone->SetProperty<Layer::Renderers::Tone>("tone_mapping_buffer mapping", &Layer::Renderers::Tone::Mapping);
				PopulateRendererInterface<Layer::Renderers::Tone, Layer::RenderSystem*>(*VTone, "tone_renderer@+ f(render_system@+)");

				auto VGlitchDistortionBuffer = VM->SetPod<Layer::Renderers::Glitch::DistortionBuffer>("glitch_distortion_buffer");
				VGlitchDistortionBuffer->SetProperty<Layer::Renderers::Glitch::DistortionBuffer>("float scan_line_jitter_displacement", &Layer::Renderers::Glitch::DistortionBuffer::ScanLineJitterDisplacement);
				VGlitchDistortionBuffer->SetProperty<Layer::Renderers::Glitch::DistortionBuffer>("float scan_line_jitter_threshold", &Layer::Renderers::Glitch::DistortionBuffer::ScanLineJitterThreshold);
				VGlitchDistortionBuffer->SetProperty<Layer::Renderers::Glitch::DistortionBuffer>("float vertical_jump_amount", &Layer::Renderers::Glitch::DistortionBuffer::VerticalJumpAmount);
				VGlitchDistortionBuffer->SetProperty<Layer::Renderers::Glitch::DistortionBuffer>("float vertical_jump_time", &Layer::Renderers::Glitch::DistortionBuffer::VerticalJumpTime);
				VGlitchDistortionBuffer->SetProperty<Layer::Renderers::Glitch::DistortionBuffer>("float color_drift_amount", &Layer::Renderers::Glitch::DistortionBuffer::ColorDriftAmount);
				VGlitchDistortionBuffer->SetProperty<Layer::Renderers::Glitch::DistortionBuffer>("float color_drift_time", &Layer::Renderers::Glitch::DistortionBuffer::ColorDriftTime);
				VGlitchDistortionBuffer->SetProperty<Layer::Renderers::Glitch::DistortionBuffer>("float horizontal_shake", &Layer::Renderers::Glitch::DistortionBuffer::HorizontalShake);
				VGlitchDistortionBuffer->SetProperty<Layer::Renderers::Glitch::DistortionBuffer>("float elapsed_time", &Layer::Renderers::Glitch::DistortionBuffer::ElapsedTime);

				auto VGlitch = VM->SetClass<Layer::Renderers::Glitch>("glitch_renderer", false);
				VGlitch->SetProperty<Layer::Renderers::Glitch>("glitch_distortion_buffer distortion", &Layer::Renderers::Glitch::Distortion);
				VGlitch->SetProperty<Layer::Renderers::Glitch>("float scan_line_jitter", &Layer::Renderers::Glitch::ScanLineJitter);
				VGlitch->SetProperty<Layer::Renderers::Glitch>("float vertical_jump", &Layer::Renderers::Glitch::VerticalJump);
				VGlitch->SetProperty<Layer::Renderers::Glitch>("float horizontal_shake", &Layer::Renderers::Glitch::HorizontalShake);
				VGlitch->SetProperty<Layer::Renderers::Glitch>("float color_drift", &Layer::Renderers::Glitch::ColorDrift);
				PopulateRendererInterface<Layer::Renderers::Glitch, Layer::RenderSystem*>(*VGlitch, "glitch_renderer@+ f(render_system@+)");

				auto VUserInterface = VM->SetClass<Layer::Renderers::UserInterface>("ui_renderer", false);
				VUserInterface->SetMethod("ui_context@+ get_context() const", &Layer::Renderers::UserInterface::GetContext);
				PopulateRendererInterface<Layer::Renderers::UserInterface, Layer::RenderSystem*>(*VUserInterface, "ui_renderer@+ f(render_system@+)");

				return true;
#else
				VI_ASSERT(false, "<renderers> is not loaded");
				return false;
#endif
			}
			bool HeavyRegistry::ImportUi(VirtualMachine* VM) noexcept
			{
#ifdef VI_BINDINGS
				VI_ASSERT(VM != nullptr, "manager should be set");

				auto VFontWeight = VM->SetEnum("ui_font_weight");
				VFontWeight->SetValue("automatic", (int)Layer::GUI::FontWeight::Auto);
				VFontWeight->SetValue("bold", (int)Layer::GUI::FontWeight::Bold);
				VFontWeight->SetValue("normal", (int)Layer::GUI::FontWeight::Normal);

				auto VInputType = VM->SetEnum("ui_input_type");
				VInputType->SetValue("keys", (int)Layer::GUI::InputType::Keys);
				VInputType->SetValue("scroll", (int)Layer::GUI::InputType::Scroll);
				VInputType->SetValue("text", (int)Layer::GUI::InputType::Text);
				VInputType->SetValue("cursor", (int)Layer::GUI::InputType::Cursor);
				VInputType->SetValue("any_of", (int)Layer::GUI::InputType::Any);

				auto VContext = VM->SetClass<Layer::GUI::Context>("ui_context", false);
				VContext->SetConstructor<Layer::GUI::Context, const Trigonometry::Vector2&>("ui_context@ f(const vector2&in)");
				VContext->SetConstructor<Layer::GUI::Context, Graphics::GraphicsDevice*>("ui_context@ f(graphics_device@+)");
				VContext->SetMethod("void emit_key(key_code, key_mod, int, int, bool)", &Layer::GUI::Context::EmitKey);
				VContext->SetMethodEx("void emit_input(const string_view&in)", &ContextEmitInput);
				VContext->SetMethod("void emit_wheel(int32, int32, bool, key_mod)", &Layer::GUI::Context::EmitWheel);
				VContext->SetMethod("void emit_resize(int32, int32)", &Layer::GUI::Context::EmitResize);
				VContext->SetMethod("void set_documents_base_tag(const string_view&in)", &Layer::GUI::Context::SetDocumentsBaseTag);
				VContext->SetMethod("void clear_styles()", &Layer::GUI::Context::ClearStyles);
				VContext->SetMethod("void update_events(activity@+)", &Layer::GUI::Context::UpdateEvents);
				VContext->SetMethod("void render_lists(texture_2d@+)", &Layer::GUI::Context::RenderLists);
				VContext->SetMethod("bool clear_documents()", &Layer::GUI::Context::ClearDocuments);
				VContext->SetMethodStatic("bool load_font_face(const string_view&in, bool = false, ui_font_weight = ui_font_weight::automatic)", &VI_SEXPECTIFY_VOID(Layer::GUI::Context::LoadFontFace));
				VContext->SetMethodStatic("string resolve_resource_path(const ui_element&in, const string_view&in)", &Layer::GUI::Context::ResolveResourcePath);
				VContext->SetMethod("bool is_input_focused()", &Layer::GUI::Context::IsInputFocused);
				VContext->SetMethod("bool is_loading()", &Layer::GUI::Context::IsLoading);
				VContext->SetMethod("bool was_input_used(uint32)", &Layer::GUI::Context::WasInputUsed);
				VContext->SetMethod("bool replace_html(const string_view&in, const string_view&in, int = 0)", &Layer::GUI::Context::ReplaceHTML);
				VContext->SetMethod("bool remove_data_model(const string_view&in)", &Layer::GUI::Context::RemoveDataModel);
				VContext->SetMethod("bool remove_data_models()", &Layer::GUI::Context::RemoveDataModels);
				VContext->SetMethod("uint64 get_idle_timeout_ms(uint64) const", &Layer::GUI::Context::GetIdleTimeoutMs);
				VContext->SetMethod("uptr@ get_context()", &Layer::GUI::Context::GetContext);
				VContext->SetMethod("vector2 get_dimensions() const", &Layer::GUI::Context::GetDimensions);
				VContext->SetMethod("string get_documents_base_tag() const", &Layer::GUI::Context::GetDocumentsBaseTag);
				VContext->SetMethod("void set_density_independent_pixel_ratio(float)", &Layer::GUI::Context::GetDensityIndependentPixelRatio);
				VContext->SetMethod("float get_density_independent_pixel_ratio() const", &Layer::GUI::Context::GetDensityIndependentPixelRatio);
				VContext->SetMethod("void enable_mouse_cursor(bool)", &Layer::GUI::Context::EnableMouseCursor);
				VContext->SetMethodEx("ui_document eval_html(const string_view&in, int = 0)", &VI_EXPECTIFY(Layer::GUI::Context::EvalHTML));
				VContext->SetMethodEx("ui_document add_css(const string_view&in, int = 0)", &VI_EXPECTIFY(Layer::GUI::Context::AddCSS));
				VContext->SetMethodEx("ui_document load_css(const string_view&in, int = 0)", &VI_EXPECTIFY(Layer::GUI::Context::LoadCSS));
				VContext->SetMethodEx("ui_document load_document(const string_view&in, bool = false)", &VI_EXPECTIFY(Layer::GUI::Context::LoadDocument));
				VContext->SetMethodEx("ui_document add_document(const string_view&in)", &VI_EXPECTIFY(Layer::GUI::Context::AddDocument));
				VContext->SetMethodEx("ui_document add_document_empty(const string_view&in = \"body\")", &VI_EXPECTIFY(Layer::GUI::Context::AddDocumentEmpty));
				VContext->SetMethod<Layer::GUI::Context, Layer::GUI::IElementDocument, const std::string_view&>("ui_document get_document(const string_view&in)", &Layer::GUI::Context::GetDocument);
				VContext->SetMethod<Layer::GUI::Context, Layer::GUI::IElementDocument, int>("ui_document get_document(int)", &Layer::GUI::Context::GetDocument);
				VContext->SetMethod("int get_num_documents() const", &Layer::GUI::Context::GetNumDocuments);
				VContext->SetMethod("ui_element get_element_by_id(const string_view&in, int = 0)", &Layer::GUI::Context::GetElementById);
				VContext->SetMethod("ui_element get_hover_element()", &Layer::GUI::Context::GetHoverElement);
				VContext->SetMethod("ui_element get_focus_element()", &Layer::GUI::Context::GetFocusElement);
				VContext->SetMethod("ui_element get_root_element()", &Layer::GUI::Context::GetRootElement);
				VContext->SetMethodEx("ui_element get_element_at_point(const vector2 &in)", &ContextGetFocusElement);
				VContext->SetMethod("void pull_document_to_front(const ui_document &in)", &Layer::GUI::Context::PullDocumentToFront);
				VContext->SetMethod("void push_document_to_back(const ui_document &in)", &Layer::GUI::Context::PushDocumentToBack);
				VContext->SetMethod("void unfocus_document(const ui_document &in)", &Layer::GUI::Context::UnfocusDocument);
				VContext->SetMethod("void add_event_listener(const string_view&in, ui_listener@+, bool = false)", &Layer::GUI::Context::AddEventListener);
				VContext->SetMethod("void remove_event_listener(const string_view&in, ui_listener@+, bool = false)", &Layer::GUI::Context::RemoveEventListener);
				VContext->SetMethod("bool is_mouse_interacting()", &Layer::GUI::Context::IsMouseInteracting);
				VContext->SetMethod("ui_model@+ set_model(const string_view&in)", &Layer::GUI::Context::SetDataModel);
				VContext->SetMethod("ui_model@+ get_model(const string_view&in)", &Layer::GUI::Context::GetDataModel);

				return true;
#else
				VI_ASSERT(false, "<ui-context> is not loaded");
				return false;
#endif
			}
			bool HeavyRegistry::ImportUiControl(VirtualMachine* VM) noexcept
			{
#ifdef VI_BINDINGS
				VI_ASSERT(VM != nullptr, "manager should be set");
				VI_TYPEREF(ModelListenerName, "ui_listener");

				auto VEventPhase = VM->SetEnum("ui_event_phase");
				auto VArea = VM->SetEnum("ui_area");
				auto VDisplay = VM->SetEnum("ui_display");
				auto VPosition = VM->SetEnum("ui_position");
				auto VFloat = VM->SetEnum("ui_float");
				auto VTimingFunc = VM->SetEnum("ui_timing_func");
				auto VTimingDir = VM->SetEnum("ui_timing_dir");
				auto VFocusFlag = VM->SetEnum("ui_focus_flag");
				auto VModalFlag = VM->SetEnum("ui_modal_flag");
				auto VNumericUnit = VM->SetEnum("numeric_unit");
				auto VElement = VM->SetStructTrivial<Layer::GUI::IElement>("ui_element");
				auto VDocument = VM->SetStructTrivial<Layer::GUI::IElementDocument>("ui_document");
				auto VEvent = VM->SetStructTrivial<Layer::GUI::IEvent>("ui_event");
				auto VListener = VM->SetClass<ModelListener>("ui_listener", true);
				VM->SetFunctionDef("void model_listener_event(ui_event &in)");

				VModalFlag->SetValue("none", (int)Layer::GUI::ModalFlag::None);
				VModalFlag->SetValue("modal", (int)Layer::GUI::ModalFlag::Modal);
				VModalFlag->SetValue("keep", (int)Layer::GUI::ModalFlag::Keep);

				VFocusFlag->SetValue("none", (int)Layer::GUI::FocusFlag::None);
				VFocusFlag->SetValue("document", (int)Layer::GUI::FocusFlag::Document);
				VFocusFlag->SetValue("keep", (int)Layer::GUI::FocusFlag::Keep);
				VFocusFlag->SetValue("automatic", (int)Layer::GUI::FocusFlag::Auto);

				VEventPhase->SetValue("none", (int)Layer::GUI::EventPhase::None);
				VEventPhase->SetValue("capture", (int)Layer::GUI::EventPhase::Capture);
				VEventPhase->SetValue("target", (int)Layer::GUI::EventPhase::Target);
				VEventPhase->SetValue("bubble", (int)Layer::GUI::EventPhase::Bubble);

				VEvent->SetConstructor<Layer::GUI::IEvent>("void f()");
				VEvent->SetConstructor<Layer::GUI::IEvent, Rml::Event*>("void f(uptr@)");
				VEvent->SetMethod("ui_event_phase get_phase() const", &Layer::GUI::IEvent::GetPhase);
				VEvent->SetMethod("void set_phase(ui_event_phase Phase)", &Layer::GUI::IEvent::SetPhase);
				VEvent->SetMethod("void set_current_element(const ui_element &in)", &Layer::GUI::IEvent::SetCurrentElement);
				VEvent->SetMethod("ui_element get_current_element() const", &Layer::GUI::IEvent::GetCurrentElement);
				VEvent->SetMethod("ui_element get_target_element() const", &Layer::GUI::IEvent::GetTargetElement);
				VEvent->SetMethod("string get_type() const", &Layer::GUI::IEvent::GetType);
				VEvent->SetMethod("void stop_propagation()", &Layer::GUI::IEvent::StopPropagation);
				VEvent->SetMethod("void stop_immediate_propagation()", &Layer::GUI::IEvent::StopImmediatePropagation);
				VEvent->SetMethod("bool is_interruptible() const", &Layer::GUI::IEvent::IsInterruptible);
				VEvent->SetMethod("bool is_propagating() const", &Layer::GUI::IEvent::IsPropagating);
				VEvent->SetMethod("bool is_immediate_propagating() const", &Layer::GUI::IEvent::IsImmediatePropagating);
				VEvent->SetMethod("bool get_boolean(const string_view&in) const", &Layer::GUI::IEvent::GetBoolean);
				VEvent->SetMethod("int64 get_integer(const string_view&in) const", &Layer::GUI::IEvent::GetInteger);
				VEvent->SetMethod("double get_number(const string_view&in) const", &Layer::GUI::IEvent::GetNumber);
				VEvent->SetMethod("string get_string(const string_view&in) const", &Layer::GUI::IEvent::GetString);
				VEvent->SetMethod("vector2 get_vector2(const string_view&in) const", &Layer::GUI::IEvent::GetVector2);
				VEvent->SetMethod("vector3 get_vector3(const string_view&in) const", &Layer::GUI::IEvent::GetVector3);
				VEvent->SetMethod("vector4 get_vector4(const string_view&in) const", &Layer::GUI::IEvent::GetVector4);
				VEvent->SetMethod("uptr@ get_pointer(const string_view&in) const", &Layer::GUI::IEvent::GetPointer);
				VEvent->SetMethod("uptr@ get_event() const", &Layer::GUI::IEvent::GetEvent);
				VEvent->SetMethod("bool is_valid() const", &Layer::GUI::IEvent::IsValid);

				VListener->SetGcConstructor<ModelListener, ModelListenerName, asIScriptFunction*>("ui_listener@ f(model_listener_event@)");
				VListener->SetGcConstructor<ModelListener, ModelListenerName, const std::string_view&>("ui_listener@ f(const string_view&in)");
				VListener->SetEnumRefsEx<ModelListener>([](ModelListener* Base, asIScriptEngine* VM)
				{
					auto& Delegate = Base->GetDelegate();
					FunctionFactory::GCEnumCallback(VM, &Delegate);
				});
				VListener->SetReleaseRefsEx<ModelListener>([](ModelListener* Base, asIScriptEngine*)
				{
					Base->GetDelegate().Release();
					Base->~ModelListener();
				});

				VArea->SetValue("margin", (int)Layer::GUI::Area::Margin);
				VArea->SetValue("border", (int)Layer::GUI::Area::Border);
				VArea->SetValue("padding", (int)Layer::GUI::Area::Padding);
				VArea->SetValue("content", (int)Layer::GUI::Area::Content);

				VDisplay->SetValue("none", (int)Layer::GUI::Display::None);
				VDisplay->SetValue("block", (int)Layer::GUI::Display::Block);
				VDisplay->SetValue("inline", (int)Layer::GUI::Display::Inline);
				VDisplay->SetValue("inline_block", (int)Layer::GUI::Display::InlineBlock);
				VDisplay->SetValue("flex", (int)Layer::GUI::Display::Flex);
				VDisplay->SetValue("table", (int)Layer::GUI::Display::Table);
				VDisplay->SetValue("table_row", (int)Layer::GUI::Display::TableRow);
				VDisplay->SetValue("table_row_group", (int)Layer::GUI::Display::TableRowGroup);
				VDisplay->SetValue("table_column", (int)Layer::GUI::Display::TableColumn);
				VDisplay->SetValue("table_column_group", (int)Layer::GUI::Display::TableColumnGroup);
				VDisplay->SetValue("table_cell", (int)Layer::GUI::Display::TableCell);

				VPosition->SetValue("static", (int)Layer::GUI::Position::Static);
				VPosition->SetValue("relative", (int)Layer::GUI::Position::Relative);
				VPosition->SetValue("absolute", (int)Layer::GUI::Position::Absolute);
				VPosition->SetValue("fixed", (int)Layer::GUI::Position::Fixed);

				VFloat->SetValue("none", (int)Layer::GUI::Float::None);
				VFloat->SetValue("left", (int)Layer::GUI::Float::Left);
				VFloat->SetValue("right", (int)Layer::GUI::Float::Right);

				VTimingFunc->SetValue("none", (int)Layer::GUI::TimingFunc::None);
				VTimingFunc->SetValue("back", (int)Layer::GUI::TimingFunc::Back);
				VTimingFunc->SetValue("bounce", (int)Layer::GUI::TimingFunc::Bounce);
				VTimingFunc->SetValue("circular", (int)Layer::GUI::TimingFunc::Circular);
				VTimingFunc->SetValue("cubic", (int)Layer::GUI::TimingFunc::Cubic);
				VTimingFunc->SetValue("elastic", (int)Layer::GUI::TimingFunc::Elastic);
				VTimingFunc->SetValue("exponential", (int)Layer::GUI::TimingFunc::Exponential);
				VTimingFunc->SetValue("linear", (int)Layer::GUI::TimingFunc::Linear);
				VTimingFunc->SetValue("quadratic", (int)Layer::GUI::TimingFunc::Quadratic);
				VTimingFunc->SetValue("quartic", (int)Layer::GUI::TimingFunc::Quartic);
				VTimingFunc->SetValue("sine", (int)Layer::GUI::TimingFunc::Sine);
				VTimingFunc->SetValue("callback", (int)Layer::GUI::TimingFunc::Callback);

				VTimingDir->SetValue("use_in", (int)Layer::GUI::TimingDir::In);
				VTimingDir->SetValue("use_out", (int)Layer::GUI::TimingDir::Out);
				VTimingDir->SetValue("use_in_out", (int)Layer::GUI::TimingDir::InOut);

				VNumericUnit->SetValue("unknown", (int)Layer::GUI::NumericUnit::UNKNOWN);
				VNumericUnit->SetValue("keyword", (int)Layer::GUI::NumericUnit::KEYWORD);
				VNumericUnit->SetValue("text", (int)Layer::GUI::NumericUnit::STRING);
				VNumericUnit->SetValue("colour", (int)Layer::GUI::NumericUnit::COLOUR);
				VNumericUnit->SetValue("ratio", (int)Layer::GUI::NumericUnit::RATIO);
				VNumericUnit->SetValue("number", (int)Layer::GUI::NumericUnit::NUMBER);
				VNumericUnit->SetValue("percent", (int)Layer::GUI::NumericUnit::PERCENT);
				VNumericUnit->SetValue("px", (int)Layer::GUI::NumericUnit::PX);
				VNumericUnit->SetValue("dp", (int)Layer::GUI::NumericUnit::DP);
				VNumericUnit->SetValue("vw", (int)Layer::GUI::NumericUnit::VW);
				VNumericUnit->SetValue("vh", (int)Layer::GUI::NumericUnit::VH);
				VNumericUnit->SetValue("x", (int)Layer::GUI::NumericUnit::X);
				VNumericUnit->SetValue("em", (int)Layer::GUI::NumericUnit::EM);
				VNumericUnit->SetValue("rem", (int)Layer::GUI::NumericUnit::REM);
				VNumericUnit->SetValue("inch", (int)Layer::GUI::NumericUnit::INCH);
				VNumericUnit->SetValue("cm", (int)Layer::GUI::NumericUnit::CM);
				VNumericUnit->SetValue("mm", (int)Layer::GUI::NumericUnit::MM);
				VNumericUnit->SetValue("pt", (int)Layer::GUI::NumericUnit::PT);
				VNumericUnit->SetValue("pc", (int)Layer::GUI::NumericUnit::PC);
				VNumericUnit->SetValue("ppi_unit", (int)Layer::GUI::NumericUnit::PPI_UNIT);
				VNumericUnit->SetValue("deg", (int)Layer::GUI::NumericUnit::DEG);
				VNumericUnit->SetValue("rad", (int)Layer::GUI::NumericUnit::RAD);
				VNumericUnit->SetValue("transform_unit", (int)Layer::GUI::NumericUnit::TRANSFORM);
				VNumericUnit->SetValue("transition", (int)Layer::GUI::NumericUnit::TRANSITION);
				VNumericUnit->SetValue("animation", (int)Layer::GUI::NumericUnit::ANIMATION);
				VNumericUnit->SetValue("decorator", (int)Layer::GUI::NumericUnit::DECORATOR);
				VNumericUnit->SetValue("font_effect", (int)Layer::GUI::NumericUnit::FONTEFFECT);
				VNumericUnit->SetValue("colour_stop_list", (int)Layer::GUI::NumericUnit::COLORSTOPLIST);
				VNumericUnit->SetValue("shadow_list", (int)Layer::GUI::NumericUnit::SHADOWLIST);
				VNumericUnit->SetValue("length", (int)Layer::GUI::NumericUnit::LENGTH);
				VNumericUnit->SetValue("length_percent", (int)Layer::GUI::NumericUnit::LENGTH_PERCENT);
				VNumericUnit->SetValue("number_length_percent", (int)Layer::GUI::NumericUnit::NUMBER_LENGTH_PERCENT);
				VNumericUnit->SetValue("dp_scalable_length", (int)Layer::GUI::NumericUnit::DP_SCALABLE_LENGTH);
				VNumericUnit->SetValue("angle", (int)Layer::GUI::NumericUnit::ANGLE);
				VNumericUnit->SetValue("numeric", (int)Layer::GUI::NumericUnit::NUMERIC);

				VElement->SetConstructor<Layer::GUI::IElement>("void f()");
				VElement->SetConstructor<Layer::GUI::IElement, Rml::Element*>("void f(uptr@)");
				VElement->SetMethod("ui_element clone() const", &Layer::GUI::IElement::Clone);
				VElement->SetMethod("void set_class(const string_view&in, bool)", &Layer::GUI::IElement::SetClass);
				VElement->SetMethod("bool is_class_set(const string_view&in) const", &Layer::GUI::IElement::IsClassSet);
				VElement->SetMethod("void set_class_names(const string_view&in)", &Layer::GUI::IElement::SetClassNames);
				VElement->SetMethod("string get_class_names() const", &Layer::GUI::IElement::GetClassNames);
				VElement->SetMethod("string get_address(bool = false, bool = true) const", &Layer::GUI::IElement::GetAddress);
				VElement->SetMethod("void set_offset(const vector2 &in, const ui_element &in, bool = false)", &Layer::GUI::IElement::SetOffset);
				VElement->SetMethod("vector2 get_relative_offset(ui_area = ui_area::Content) const", &Layer::GUI::IElement::GetRelativeOffset);
				VElement->SetMethod("vector2 get_absolute_offset(ui_area = ui_area::Content) const", &Layer::GUI::IElement::GetAbsoluteOffset);
				VElement->SetMethod("void set_bontent_box(const vector2 &in)", &Layer::GUI::IElement::SetContentBox);
				VElement->SetMethod("float get_baseline() const", &Layer::GUI::IElement::GetBaseline);
				VElement->SetMethod("bool get_intrinsic_dimensions(vector2 &out, float &out)", &Layer::GUI::IElement::GetIntrinsicDimensions);
				VElement->SetMethod("bool is_point_within_element(const vector2 &in)", &Layer::GUI::IElement::IsPointWithinElement);
				VElement->SetMethod("bool is_visible() const", &Layer::GUI::IElement::IsVisible);
				VElement->SetMethod("float get_zindex() const", &Layer::GUI::IElement::GetZIndex);
				VElement->SetMethod("bool set_property(const string_view&in, const string_view&in)", &Layer::GUI::IElement::SetProperty);
				VElement->SetMethod("void remove_property(const string_view&in)", &Layer::GUI::IElement::RemoveProperty);
				VElement->SetMethod("string get_property(const string_view&in) const", &Layer::GUI::IElement::GetProperty);
				VElement->SetMethod("string get_local_property(const string_view&in) const", &Layer::GUI::IElement::GetLocalProperty);
				VElement->SetMethod("float resolve_numeric_property(const string_view&in) const", &Layer::GUI::IElement::ResolveNumericProperty);
				VElement->SetMethod("vector2 get_containing_block() const", &Layer::GUI::IElement::GetContainingBlock);
				VElement->SetMethod("ui_position get_position() const", &Layer::GUI::IElement::GetPosition);
				VElement->SetMethod("ui_float get_float() const", &Layer::GUI::IElement::GetFloat);
				VElement->SetMethod("ui_display get_display() const", &Layer::GUI::IElement::GetDisplay);
				VElement->SetMethod("float get_line_height() const", &Layer::GUI::IElement::GetLineHeight);
				VElement->SetMethod("bool project(vector2 &out) const", &Layer::GUI::IElement::Project);
				VElement->SetMethod("bool animate(const string_view&in, const string_view&in, float, ui_timing_func, ui_timing_dir, int = -1, bool = true, float = 0)", &Layer::GUI::IElement::Animate);
				VElement->SetMethod("bool add_animation_key(const string_view&in, const string_view&in, float, ui_timing_func, ui_timing_dir)", &Layer::GUI::IElement::AddAnimationKey);
				VElement->SetMethod("void set_pseudo_Class(const string_view&in, bool)", &Layer::GUI::IElement::SetPseudoClass);
				VElement->SetMethod("bool is_pseudo_class_set(const string_view&in) const", &Layer::GUI::IElement::IsPseudoClassSet);
				VElement->SetMethod("void set_attribute(const string_view&in, const string_view&in)", &Layer::GUI::IElement::SetAttribute);
				VElement->SetMethod("string get_attribute(const string_view&in) const", &Layer::GUI::IElement::GetAttribute);
				VElement->SetMethod("bool has_attribute(const string_view&in) const", &Layer::GUI::IElement::HasAttribute);
				VElement->SetMethod("void remove_attribute(const string_view&in)", &Layer::GUI::IElement::RemoveAttribute);
				VElement->SetMethod("ui_element get_focus_leaf_node()", &Layer::GUI::IElement::GetFocusLeafNode);
				VElement->SetMethod("string get_tag_name() const", &Layer::GUI::IElement::GetTagName);
				VElement->SetMethod("string get_id() const", &Layer::GUI::IElement::GetId);
				VElement->SetMethod("float get_absolute_left()", &Layer::GUI::IElement::GetAbsoluteLeft);
				VElement->SetMethod("float get_absolute_top()", &Layer::GUI::IElement::GetAbsoluteTop);
				VElement->SetMethod("float get_client_left()", &Layer::GUI::IElement::GetClientLeft);
				VElement->SetMethod("float get_client_top()", &Layer::GUI::IElement::GetClientTop);
				VElement->SetMethod("float get_client_width()", &Layer::GUI::IElement::GetClientWidth);
				VElement->SetMethod("float get_client_height()", &Layer::GUI::IElement::GetClientHeight);
				VElement->SetMethod("ui_element get_offset_parent()", &Layer::GUI::IElement::GetOffsetParent);
				VElement->SetMethod("float get_offset_left()", &Layer::GUI::IElement::GetOffsetLeft);
				VElement->SetMethod("float get_offset_top()", &Layer::GUI::IElement::GetOffsetTop);
				VElement->SetMethod("float get_offset_width()", &Layer::GUI::IElement::GetOffsetWidth);
				VElement->SetMethod("float get_offset_height()", &Layer::GUI::IElement::GetOffsetHeight);
				VElement->SetMethod("float get_scroll_left()", &Layer::GUI::IElement::GetScrollLeft);
				VElement->SetMethod("void set_scroll_left(float)", &Layer::GUI::IElement::SetScrollLeft);
				VElement->SetMethod("float get_scroll_top()", &Layer::GUI::IElement::GetScrollTop);
				VElement->SetMethod("void set_scroll_top(float)", &Layer::GUI::IElement::SetScrollTop);
				VElement->SetMethod("float get_scroll_width()", &Layer::GUI::IElement::GetScrollWidth);
				VElement->SetMethod("float get_scroll_height()", &Layer::GUI::IElement::GetScrollHeight);
				VElement->SetMethod("ui_document get_owner_document() const", &Layer::GUI::IElement::GetOwnerDocument);
				VElement->SetMethod("ui_element get_parent_node() const", &Layer::GUI::IElement::GetParentNode);
				VElement->SetMethod("ui_element get_next_sibling() const", &Layer::GUI::IElement::GetNextSibling);
				VElement->SetMethod("ui_element get_previous_sibling() const", &Layer::GUI::IElement::GetPreviousSibling);
				VElement->SetMethod("ui_element get_first_child() const", &Layer::GUI::IElement::GetFirstChild);
				VElement->SetMethod("ui_element get_last_child() const", &Layer::GUI::IElement::GetLastChild);
				VElement->SetMethod("ui_element get_child(int) const", &Layer::GUI::IElement::GetChild);
				VElement->SetMethod("int get_num_children(bool = false) const", &Layer::GUI::IElement::GetNumChildren);
				VElement->SetMethod<Layer::GUI::IElement, void, Core::String&>("void get_inner_html(string &out) const", &Layer::GUI::IElement::GetInnerHTML);
				VElement->SetMethod<Layer::GUI::IElement, Core::String>("string get_inner_html() const", &Layer::GUI::IElement::GetInnerHTML);
				VElement->SetMethod("void set_inner_html(const string_view&in)", &Layer::GUI::IElement::SetInnerHTML);
				VElement->SetMethod("bool is_focused()", &Layer::GUI::IElement::IsFocused);
				VElement->SetMethod("bool is_hovered()", &Layer::GUI::IElement::IsHovered);
				VElement->SetMethod("bool is_active()", &Layer::GUI::IElement::IsActive);
				VElement->SetMethod("bool is_checked()", &Layer::GUI::IElement::IsChecked);
				VElement->SetMethod("bool focus()", &Layer::GUI::IElement::Focus);
				VElement->SetMethod("void blur()", &Layer::GUI::IElement::Blur);
				VElement->SetMethod("void click()", &Layer::GUI::IElement::Click);
				VElement->SetMethod("void add_event_listener(const string_view&in, ui_listener@+, bool = false)", &Layer::GUI::IElement::AddEventListener);
				VElement->SetMethod("void remove_event_listener(const string_view&in, ui_listener@+, bool = false)", &Layer::GUI::IElement::RemoveEventListener);
				VElement->SetMethodEx("bool dispatch_event(const string_view&in, schema@+)", &IElementDocumentDispatchEvent);
				VElement->SetMethod("void scroll_into_view(bool = true)", &Layer::GUI::IElement::ScrollIntoView);
				VElement->SetMethod("ui_element append_child(const ui_element &in, bool = true)", &Layer::GUI::IElement::AppendChild);
				VElement->SetMethod("ui_element insert_before(const ui_element &in, const ui_element &in)", &Layer::GUI::IElement::InsertBefore);
				VElement->SetMethod("ui_element replace_child(const ui_element &in, const ui_element &in)", &Layer::GUI::IElement::ReplaceChild);
				VElement->SetMethod("ui_element remove_child(const ui_element &in)", &Layer::GUI::IElement::RemoveChild);
				VElement->SetMethod("bool has_child_nodes() const", &Layer::GUI::IElement::HasChildNodes);
				VElement->SetMethod("ui_element get_element_by_id(const string_view&in)", &Layer::GUI::IElement::GetElementById);
				VElement->SetMethodEx("array<ui_element>@ query_selector_all(const string_view&in)", &IElementDocumentQuerySelectorAll);
				VElement->SetMethod("bool cast_form_color(vector4 &out, bool)", &Layer::GUI::IElement::CastFormColor);
				VElement->SetMethod("bool cast_form_string(string &out)", &Layer::GUI::IElement::CastFormString);
				VElement->SetMethod("bool cast_form_pointer(uptr@ &out)", &Layer::GUI::IElement::CastFormPointer);
				VElement->SetMethod("bool cast_form_int32(int &out)", &Layer::GUI::IElement::CastFormInt32);
				VElement->SetMethod("bool cast_form_uint32(uint &out)", &Layer::GUI::IElement::CastFormUInt32);
				VElement->SetMethod("bool cast_form_flag32(uint &out, uint)", &Layer::GUI::IElement::CastFormFlag32);
				VElement->SetMethod("bool cast_form_int64(int64 &out)", &Layer::GUI::IElement::CastFormInt64);
				VElement->SetMethod("bool cast_form_uint64(uint64 &out)", &Layer::GUI::IElement::CastFormUInt64);
				VElement->SetMethod("bool cast_form_flag64(uint64 &out, uint64)", &Layer::GUI::IElement::CastFormFlag64);
				VElement->SetMethod<Layer::GUI::IElement, bool, float*>("bool cast_form_float(float &out)", &Layer::GUI::IElement::CastFormFloat);
				VElement->SetMethod<Layer::GUI::IElement, bool, float*, float>("bool cast_form_float(float &out, float)", &Layer::GUI::IElement::CastFormFloat);
				VElement->SetMethod("bool cast_form_double(double &out)", &Layer::GUI::IElement::CastFormDouble);
				VElement->SetMethod("bool cast_form_boolean(bool &out)", &Layer::GUI::IElement::CastFormBoolean);
				VElement->SetMethod("string get_form_name() const", &Layer::GUI::IElement::GetFormName);
				VElement->SetMethod("void set_form_name(const string_view&in)", &Layer::GUI::IElement::SetFormName);
				VElement->SetMethod("string get_form_value() const", &Layer::GUI::IElement::GetFormValue);
				VElement->SetMethod("void set_form_value(const string_view&in)", &Layer::GUI::IElement::SetFormValue);
				VElement->SetMethod("bool is_form_disabled() const", &Layer::GUI::IElement::IsFormDisabled);
				VElement->SetMethod("void set_form_disabled(bool)", &Layer::GUI::IElement::SetFormDisabled);
				VElement->SetMethod("uptr@ get_element() const", &Layer::GUI::IElement::GetElement);
				VElement->SetMethod("bool is_valid() const", &Layer::GUI::IElement::IsValid);

				VDocument->SetConstructor<Layer::GUI::IElementDocument>("void f()");
				VDocument->SetConstructor<Layer::GUI::IElementDocument, Rml::ElementDocument*>("void f(uptr@)");
				VDocument->SetMethod("ui_element clone() const", &Layer::GUI::IElementDocument::Clone);
				VDocument->SetMethod("void set_class(const string_view&in, bool)", &Layer::GUI::IElementDocument::SetClass);
				VDocument->SetMethod("bool is_class_set(const string_view&in) const", &Layer::GUI::IElementDocument::IsClassSet);
				VDocument->SetMethod("void set_class_names(const string_view&in)", &Layer::GUI::IElementDocument::SetClassNames);
				VDocument->SetMethod("string get_class_names() const", &Layer::GUI::IElementDocument::GetClassNames);
				VDocument->SetMethod("string get_address(bool = false, bool = true) const", &Layer::GUI::IElementDocument::GetAddress);
				VDocument->SetMethod("void set_offset(const vector2 &in, const ui_element &in, bool = false)", &Layer::GUI::IElementDocument::SetOffset);
				VDocument->SetMethod("vector2 get_relative_offset(ui_area = ui_area::Content) const", &Layer::GUI::IElementDocument::GetRelativeOffset);
				VDocument->SetMethod("vector2 get_absolute_offset(ui_area = ui_area::Content) const", &Layer::GUI::IElementDocument::GetAbsoluteOffset);
				VDocument->SetMethod("void set_bontent_box(const vector2 &in, const vector2 &in)", &Layer::GUI::IElementDocument::SetContentBox);
				VDocument->SetMethod("float get_baseline() const", &Layer::GUI::IElementDocument::GetBaseline);
				VDocument->SetMethod("bool get_intrinsic_dimensions(vector2 &out, float &out)", &Layer::GUI::IElementDocument::GetIntrinsicDimensions);
				VDocument->SetMethod("bool is_point_within_element(const vector2 &in)", &Layer::GUI::IElementDocument::IsPointWithinElement);
				VDocument->SetMethod("bool is_visible() const", &Layer::GUI::IElementDocument::IsVisible);
				VDocument->SetMethod("float get_zindex() const", &Layer::GUI::IElementDocument::GetZIndex);
				VDocument->SetMethod("bool set_property(const string_view&in, const string_view&in)", &Layer::GUI::IElementDocument::SetProperty);
				VDocument->SetMethod("void remove_property(const string_view&in)", &Layer::GUI::IElementDocument::RemoveProperty);
				VDocument->SetMethod("string get_property(const string_view&in) const", &Layer::GUI::IElementDocument::GetProperty);
				VDocument->SetMethod("string get_local_property(const string_view&in) const", &Layer::GUI::IElementDocument::GetLocalProperty);
				VDocument->SetMethod("float resolve_numeric_property(float, numeric_unit, float) const", &Layer::GUI::IElementDocument::ResolveNumericProperty);
				VDocument->SetMethod("vector2 get_containing_block() const", &Layer::GUI::IElementDocument::GetContainingBlock);
				VDocument->SetMethod("ui_position get_position() const", &Layer::GUI::IElementDocument::GetPosition);
				VDocument->SetMethod("ui_float get_float() const", &Layer::GUI::IElementDocument::GetFloat);
				VDocument->SetMethod("ui_display get_display() const", &Layer::GUI::IElementDocument::GetDisplay);
				VDocument->SetMethod("float get_line_height() const", &Layer::GUI::IElementDocument::GetLineHeight);
				VDocument->SetMethod("bool project(vector2 &out) const", &Layer::GUI::IElementDocument::Project);
				VDocument->SetMethod("bool animate(const string_view&in, const string_view&in, float, ui_timing_func, ui_timing_dir, int = -1, bool = true, float = 0)", &Layer::GUI::IElementDocument::Animate);
				VDocument->SetMethod("bool add_animation_key(const string_view&in, const string_view&in, float, ui_timing_func, ui_timing_dir)", &Layer::GUI::IElementDocument::AddAnimationKey);
				VDocument->SetMethod("void set_pseudo_Class(const string_view&in, bool)", &Layer::GUI::IElementDocument::SetPseudoClass);
				VDocument->SetMethod("bool is_pseudo_class_set(const string_view&in) const", &Layer::GUI::IElementDocument::IsPseudoClassSet);
				VDocument->SetMethod("void set_attribute(const string_view&in, const string_view&in)", &Layer::GUI::IElementDocument::SetAttribute);
				VDocument->SetMethod("string get_attribute(const string_view&in) const", &Layer::GUI::IElementDocument::GetAttribute);
				VDocument->SetMethod("bool has_attribute(const string_view&in) const", &Layer::GUI::IElementDocument::HasAttribute);
				VDocument->SetMethod("void remove_attribute(const string_view&in)", &Layer::GUI::IElementDocument::RemoveAttribute);
				VDocument->SetMethod("ui_element get_focus_leaf_node()", &Layer::GUI::IElementDocument::GetFocusLeafNode);
				VDocument->SetMethod("string get_tag_name() const", &Layer::GUI::IElementDocument::GetTagName);
				VDocument->SetMethod("string get_id() const", &Layer::GUI::IElementDocument::GetId);
				VDocument->SetMethod("float get_absolute_left()", &Layer::GUI::IElementDocument::GetAbsoluteLeft);
				VDocument->SetMethod("float get_absolute_top()", &Layer::GUI::IElementDocument::GetAbsoluteTop);
				VDocument->SetMethod("float get_client_left()", &Layer::GUI::IElementDocument::GetClientLeft);
				VDocument->SetMethod("float get_client_top()", &Layer::GUI::IElementDocument::GetClientTop);
				VDocument->SetMethod("float get_client_width()", &Layer::GUI::IElementDocument::GetClientWidth);
				VDocument->SetMethod("float get_client_height()", &Layer::GUI::IElementDocument::GetClientHeight);
				VDocument->SetMethod("ui_element get_offset_parent()", &Layer::GUI::IElementDocument::GetOffsetParent);
				VDocument->SetMethod("float get_offset_left()", &Layer::GUI::IElementDocument::GetOffsetLeft);
				VDocument->SetMethod("float get_offset_top()", &Layer::GUI::IElementDocument::GetOffsetTop);
				VDocument->SetMethod("float get_offset_width()", &Layer::GUI::IElementDocument::GetOffsetWidth);
				VDocument->SetMethod("float get_offset_height()", &Layer::GUI::IElementDocument::GetOffsetHeight);
				VDocument->SetMethod("float get_scroll_left()", &Layer::GUI::IElementDocument::GetScrollLeft);
				VDocument->SetMethod("void set_scroll_left(float)", &Layer::GUI::IElementDocument::SetScrollLeft);
				VDocument->SetMethod("float get_scroll_top()", &Layer::GUI::IElementDocument::GetScrollTop);
				VDocument->SetMethod("void set_scroll_top(float)", &Layer::GUI::IElementDocument::SetScrollTop);
				VDocument->SetMethod("float get_scroll_width()", &Layer::GUI::IElementDocument::GetScrollWidth);
				VDocument->SetMethod("float get_scroll_height()", &Layer::GUI::IElementDocument::GetScrollHeight);
				VDocument->SetMethod("ui_document get_owner_document() const", &Layer::GUI::IElementDocument::GetOwnerDocument);
				VDocument->SetMethod("ui_element get_parent_node() const", &Layer::GUI::IElementDocument::GetParentNode);
				VDocument->SetMethod("ui_element get_next_sibling() const", &Layer::GUI::IElementDocument::GetNextSibling);
				VDocument->SetMethod("ui_element get_previous_sibling() const", &Layer::GUI::IElementDocument::GetPreviousSibling);
				VDocument->SetMethod("ui_element get_first_child() const", &Layer::GUI::IElementDocument::GetFirstChild);
				VDocument->SetMethod("ui_element get_last_child() const", &Layer::GUI::IElementDocument::GetLastChild);
				VDocument->SetMethod("ui_element get_child(int) const", &Layer::GUI::IElementDocument::GetChild);
				VDocument->SetMethod("int get_num_children(bool = false) const", &Layer::GUI::IElementDocument::GetNumChildren);
				VDocument->SetMethod<Layer::GUI::IElement, void, Core::String&>("void get_inner_html(string &out) const", &Layer::GUI::IElementDocument::GetInnerHTML);
				VDocument->SetMethod<Layer::GUI::IElement, Core::String>("string get_inner_html() const", &Layer::GUI::IElementDocument::GetInnerHTML);
				VDocument->SetMethod("void set_inner_html(const string_view&in)", &Layer::GUI::IElementDocument::SetInnerHTML);
				VDocument->SetMethod("bool is_focused()", &Layer::GUI::IElementDocument::IsFocused);
				VDocument->SetMethod("bool is_hovered()", &Layer::GUI::IElementDocument::IsHovered);
				VDocument->SetMethod("bool is_active()", &Layer::GUI::IElementDocument::IsActive);
				VDocument->SetMethod("bool is_checked()", &Layer::GUI::IElementDocument::IsChecked);
				VDocument->SetMethod("bool focus()", &Layer::GUI::IElementDocument::Focus);
				VDocument->SetMethod("void blur()", &Layer::GUI::IElementDocument::Blur);
				VDocument->SetMethod("void click()", &Layer::GUI::IElementDocument::Click);
				VDocument->SetMethod("void add_event_listener(const string_view&in, ui_listener@+, bool = false)", &Layer::GUI::IElementDocument::AddEventListener);
				VDocument->SetMethod("void remove_event_listener(const string_view&in, ui_listener@+, bool = false)", &Layer::GUI::IElementDocument::RemoveEventListener);
				VDocument->SetMethodEx("bool dispatch_event(const string_view&in, schema@+)", &IElementDocumentDispatchEvent);
				VDocument->SetMethod("void scroll_into_view(bool = true)", &Layer::GUI::IElementDocument::ScrollIntoView);
				VDocument->SetMethod("ui_element append_child(const ui_element &in, bool = true)", &Layer::GUI::IElementDocument::AppendChild);
				VDocument->SetMethod("ui_element insert_before(const ui_element &in, const ui_element &in)", &Layer::GUI::IElementDocument::InsertBefore);
				VDocument->SetMethod("ui_element replace_child(const ui_element &in, const ui_element &in)", &Layer::GUI::IElementDocument::ReplaceChild);
				VDocument->SetMethod("ui_element remove_child(const ui_element &in)", &Layer::GUI::IElementDocument::RemoveChild);
				VDocument->SetMethod("bool has_child_nodes() const", &Layer::GUI::IElementDocument::HasChildNodes);
				VDocument->SetMethod("ui_element get_element_by_id(const string_view&in)", &Layer::GUI::IElementDocument::GetElementById);
				VDocument->SetMethodEx("array<ui_element>@ query_selector_all(const string_view&in)", &IElementDocumentQuerySelectorAll);
				VDocument->SetMethod("bool cast_form_color(vector4 &out, bool)", &Layer::GUI::IElementDocument::CastFormColor);
				VDocument->SetMethod("bool cast_form_string(string &out)", &Layer::GUI::IElementDocument::CastFormString);
				VDocument->SetMethod("bool cast_form_pointer(uptr@ &out)", &Layer::GUI::IElementDocument::CastFormPointer);
				VDocument->SetMethod("bool cast_form_int32(int &out)", &Layer::GUI::IElementDocument::CastFormInt32);
				VDocument->SetMethod("bool cast_form_uint32(uint &out)", &Layer::GUI::IElementDocument::CastFormUInt32);
				VDocument->SetMethod("bool cast_form_flag32(uint &out, uint)", &Layer::GUI::IElementDocument::CastFormFlag32);
				VDocument->SetMethod("bool cast_form_int64(int64 &out)", &Layer::GUI::IElementDocument::CastFormInt64);
				VDocument->SetMethod("bool cast_form_uint64(uint64 &out)", &Layer::GUI::IElementDocument::CastFormUInt64);
				VDocument->SetMethod("bool cast_form_flag64(uint64 &out, uint64)", &Layer::GUI::IElementDocument::CastFormFlag64);
				VDocument->SetMethod<Layer::GUI::IElement, bool, float*>("bool cast_form_float(float &out)", &Layer::GUI::IElementDocument::CastFormFloat);
				VDocument->SetMethod<Layer::GUI::IElement, bool, float*, float>("bool cast_form_float(float &out, float)", &Layer::GUI::IElementDocument::CastFormFloat);
				VDocument->SetMethod("bool cast_form_double(double &out)", &Layer::GUI::IElementDocument::CastFormDouble);
				VDocument->SetMethod("bool cast_form_boolean(bool &out)", &Layer::GUI::IElementDocument::CastFormBoolean);
				VDocument->SetMethod("string get_form_name() const", &Layer::GUI::IElementDocument::GetFormName);
				VDocument->SetMethod("void set_form_name(const string_view&in)", &Layer::GUI::IElementDocument::SetFormName);
				VDocument->SetMethod("string get_form_value() const", &Layer::GUI::IElementDocument::GetFormValue);
				VDocument->SetMethod("void set_form_value(const string_view&in)", &Layer::GUI::IElementDocument::SetFormValue);
				VDocument->SetMethod("bool is_form_disabled() const", &Layer::GUI::IElementDocument::IsFormDisabled);
				VDocument->SetMethod("void set_form_disabled(bool)", &Layer::GUI::IElementDocument::SetFormDisabled);
				VDocument->SetMethod("uptr@ get_element() const", &Layer::GUI::IElementDocument::GetElement);
				VDocument->SetMethod("bool is_valid() const", &Layer::GUI::IElementDocument::IsValid);
				VDocument->SetMethod("void set_title(const string_view&in)", &Layer::GUI::IElementDocument::SetTitle);
				VDocument->SetMethod("void pull_to_front()", &Layer::GUI::IElementDocument::PullToFront);
				VDocument->SetMethod("void push_to_back()", &Layer::GUI::IElementDocument::PushToBack);
				VDocument->SetMethod("void show(ui_modal_flag = ui_modal_flag::none, ui_focus_flag = ui_focus_flag::automatic)", &Layer::GUI::IElementDocument::Show);
				VDocument->SetMethod("void hide()", &Layer::GUI::IElementDocument::Hide);
				VDocument->SetMethod("void close()", &Layer::GUI::IElementDocument::Close);
				VDocument->SetMethod("string get_title() const", &Layer::GUI::IElementDocument::GetTitle);
				VDocument->SetMethod("string get_source_url() const", &Layer::GUI::IElementDocument::GetSourceURL);
				VDocument->SetMethod("ui_element create_element(const string_view&in)", &Layer::GUI::IElementDocument::CreateElement);
				VDocument->SetMethod("bool is_modal() const", &Layer::GUI::IElementDocument::IsModal);
				VDocument->SetMethod("uptr@ get_element_document() const", &Layer::GUI::IElementDocument::GetElementDocument);

				return true;
#else
				VI_ASSERT(false, "<ui-control> is not loaded");
				return false;
#endif
			}
			bool HeavyRegistry::ImportUiModel(VirtualMachine* VM) noexcept
			{
#ifdef VI_BINDINGS
				VI_ASSERT(VM != nullptr, "manager should be set");
				VM->SetFunctionDef("void ui_data_event(ui_event &in, array<variant>@+)");

				auto VModel = VM->SetClass<Layer::GUI::DataModel>("ui_model", false);
				VModel->SetMethodEx("bool set(const string_view&in, schema@+)", &DataModelSet);
				VModel->SetMethodEx("bool set_var(const string_view&in, const variant &in)", &DataModelSetVar);
				VModel->SetMethodEx("bool set_string(const string_view&in, const string_view&in)", &DataModelSetString);
				VModel->SetMethodEx("bool set_integer(const string_view&in, int64)", &DataModelSetInteger);
				VModel->SetMethodEx("bool set_float(const string_view&in, float)", &DataModelSetFloat);
				VModel->SetMethodEx("bool set_double(const string_view&in, double)", &DataModelSetDouble);
				VModel->SetMethodEx("bool set_boolean(const string_view&in, bool)", &DataModelSetBoolean);
				VModel->SetMethodEx("bool set_pointer(const string_view&in, uptr@)", &DataModelSetPointer);
				VModel->SetMethodEx("bool set_callback(const string_view&in, ui_data_event@)", &DataModelSetCallback);
				VModel->SetMethodEx("schema@ get(const string_view&in)", &DataModelGet);
				VModel->SetMethod("string get_string(const string_view&in)", &Layer::GUI::DataModel::GetString);
				VModel->SetMethod("int64 get_integer(const string_view&in)", &Layer::GUI::DataModel::GetInteger);
				VModel->SetMethod("float get_float(const string_view&in)", &Layer::GUI::DataModel::GetFloat);
				VModel->SetMethod("double get_double(const string_view&in)", &Layer::GUI::DataModel::GetDouble);
				VModel->SetMethod("bool get_boolean(const string_view&in)", &Layer::GUI::DataModel::GetBoolean);
				VModel->SetMethod("uptr@ get_pointer(const string_view&in)", &Layer::GUI::DataModel::GetPointer);
				VModel->SetMethod("bool has_changed(const string_view&in)", &Layer::GUI::DataModel::HasChanged);
				VModel->SetMethod("void change(const string_view&in)", &Layer::GUI::DataModel::Change);
				VModel->SetMethod("bool isValid() const", &Layer::GUI::DataModel::IsValid);
				VModel->SetMethodStatic("vector4 to_color4(const string_view&in)", &Layer::GUI::IVariant::ToColor4);
				VModel->SetMethodStatic("string from_color4(const vector4 &in, bool)", &Layer::GUI::IVariant::FromColor4);
				VModel->SetMethodStatic("vector4 to_color3(const string_view&in)", &Layer::GUI::IVariant::ToColor3);
				VModel->SetMethodStatic("string from_color3(const vector4 &in, bool)", &Layer::GUI::IVariant::ToColor3);
				VModel->SetMethodStatic("int get_vector_type(const string_view&in)", &Layer::GUI::IVariant::GetVectorType);
				VModel->SetMethodStatic("vector4 to_vector4(const string_view&in)", &Layer::GUI::IVariant::ToVector4);
				VModel->SetMethodStatic("string from_vector4(const vector4 &in)", &Layer::GUI::IVariant::FromVector4);
				VModel->SetMethodStatic("vector3 to_vector3(const string_view&in)", &Layer::GUI::IVariant::ToVector3);
				VModel->SetMethodStatic("string from_vector3(const vector3 &in)", &Layer::GUI::IVariant::FromVector3);
				VModel->SetMethodStatic("vector2 to_vector2(const string_view&in)", &Layer::GUI::IVariant::ToVector2);
				VModel->SetMethodStatic("string from_vector2(const vector2 &in)", &Layer::GUI::IVariant::FromVector2);

				return true;
#else
				VI_ASSERT(false, "<ui-model> is not loaded");
				return false;
#endif
			}
		}
	}
}