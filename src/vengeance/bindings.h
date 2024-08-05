#ifndef VI_BINDINGS_H_EXTENSION
#define VI_BINDINGS_H_EXTENSION
#include <vitex/bindings.h>
#include "layer/gui.h"

namespace Vitex
{
	namespace Scripting
	{
		namespace Bindings
		{
#ifdef VI_BINDINGS
			class VI_OUT ModelListener : public Core::Reference<ModelListener>
			{
			private:
				FunctionDelegate Delegate;
				Layer::GUI::Listener* Base;

			public:
				ModelListener(asIScriptFunction* NewCallback) noexcept;
				ModelListener(const std::string_view& FunctionName) noexcept;
				~ModelListener() noexcept;
				FunctionDelegate& GetDelegate();

			private:
				Layer::GUI::EventCallback Bind(asIScriptFunction* Callback);
			};

			class VI_OUT HeavyApplication final : public Layer::HeavyApplication
			{
			public:
				FunctionDelegate OnKeyEvent;
				FunctionDelegate OnInputEvent;
				FunctionDelegate OnWheelEvent;
				FunctionDelegate OnWindowEvent;
				FunctionDelegate OnDispatch;
				FunctionDelegate OnPublish;
				FunctionDelegate OnComposition;
				FunctionDelegate OnScriptHook;
				FunctionDelegate OnInitialize;
				FunctionDelegate OnStartup;
				FunctionDelegate OnShutdown;

			private:
				size_t ProcessedEvents;
				asITypeInfo* InitiatorType;
				void* InitiatorObject;

			public:
				HeavyApplication(Desc& I, void* Object, int TypeId) noexcept;
				virtual ~HeavyApplication() noexcept override;
				void SetOnKeyEvent(asIScriptFunction* Callback);
				void SetOnInputEvent(asIScriptFunction* Callback);
				void SetOnWheelEvent(asIScriptFunction* Callback);
				void SetOnWindowEvent(asIScriptFunction* Callback);
				void SetOnDispatch(asIScriptFunction* Callback);
				void SetOnPublish(asIScriptFunction* Callback);
				void SetOnComposition(asIScriptFunction* Callback);
				void SetOnScriptHook(asIScriptFunction* Callback);
				void SetOnInitialize(asIScriptFunction* Callback);
				void SetOnStartup(asIScriptFunction* Callback);
				void SetOnShutdown(asIScriptFunction* Callback);
				void KeyEvent(Graphics::KeyCode Key, Graphics::KeyMod Mod, int Virtual, int Repeat, bool Pressed) override;
				void InputEvent(char* Buffer, size_t Length) override;
				void WheelEvent(int X, int Y, bool Normal) override;
				void WindowEvent(Graphics::WindowState NewState, int X, int Y) override;
				void Dispatch(Core::Timer* Time) override;
				void Publish(Core::Timer* Time) override;
				void Composition() override;
				void ScriptHook() override;
				void Initialize() override;
				Core::Promise<void> Startup() override;
				Core::Promise<void> Shutdown() override;
				size_t GetProcessedEvents() const;
				bool HasProcessedEvents() const;
				bool RetrieveInitiatorObject(void* RefPointer, int RefTypeId) const;
				void* GetInitiatorObject() const;

			public:
				static bool WantsRestart(int ExitCode);
			};
#endif
			class VI_OUT_TS HeavyRegistry final : public Registry
			{
			public:
				HeavyRegistry() = default;
				bool BindAddons(VirtualMachine* VM) noexcept override;
				static bool ImportTrigonometry(VirtualMachine* VM) noexcept;
				static bool ImportActivity(VirtualMachine* VM) noexcept;
				static bool ImportPhysics(VirtualMachine* VM) noexcept;
				static bool ImportGraphics(VirtualMachine* VM) noexcept;
				static bool ImportAudio(VirtualMachine* VM) noexcept;
				static bool ImportAudioEffects(VirtualMachine* VM) noexcept;
				static bool ImportAudioFilters(VirtualMachine* VM) noexcept;
				static bool ImportEngine(VirtualMachine* VM) noexcept;
				static bool ImportEngineComponents(VirtualMachine* VM) noexcept;
				static bool ImportEngineRenderers(VirtualMachine* VM) noexcept;
				static bool ImportUi(VirtualMachine* VM) noexcept;
				static bool ImportUiModel(VirtualMachine* VM) noexcept;
				static bool ImportUiControl(VirtualMachine* VM) noexcept;
			};
		}
	}
}
#endif