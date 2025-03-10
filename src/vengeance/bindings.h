#ifndef VI_BINDINGS_H_EXTENSION
#define VI_BINDINGS_H_EXTENSION
#include <vitex/bindings.h>
#include "layer/gui.h"

namespace vitex
{
	namespace scripting
	{
		namespace bindings
		{
#ifdef VI_BINDINGS
			class model_listener : public core::reference<model_listener>
			{
			private:
				function_delegate delegatef;
				layer::gui::listener* base;

			public:
				model_listener(asIScriptFunction* new_callback) noexcept;
				model_listener(const std::string_view& function_name) noexcept;
				~model_listener() noexcept;
				function_delegate& get_delegate();

			private:
				layer::gui::event_callback bind(asIScriptFunction* callback);
			};

			class heavy_application final : public layer::heavy_application
			{
			public:
				function_delegate on_key_event;
				function_delegate on_input_event;
				function_delegate on_wheel_event;
				function_delegate on_window_event;
				function_delegate on_dispatch;
				function_delegate on_publish;
				function_delegate on_composition;
				function_delegate on_script_hook;
				function_delegate on_initialize;
				function_delegate on_startup;
				function_delegate on_shutdown;

			private:
				size_t processed_events;
				asITypeInfo* initiator_type;
				void* initiator_object;

			public:
				heavy_application(desc& i, void* object, int type_id) noexcept;
				virtual ~heavy_application() noexcept override;
				void set_on_key_event(asIScriptFunction* callback);
				void set_on_input_event(asIScriptFunction* callback);
				void set_on_wheel_event(asIScriptFunction* callback);
				void set_on_window_event(asIScriptFunction* callback);
				void set_on_dispatch(asIScriptFunction* callback);
				void set_on_publish(asIScriptFunction* callback);
				void set_on_composition(asIScriptFunction* callback);
				void set_on_script_hook(asIScriptFunction* callback);
				void set_on_initialize(asIScriptFunction* callback);
				void set_on_startup(asIScriptFunction* callback);
				void set_on_shutdown(asIScriptFunction* callback);
				void key_event(graphics::key_code key, graphics::key_mod mod, int computed, int repeat, bool pressed) override;
				void input_event(char* buffer, size_t length) override;
				void wheel_event(int x, int y, bool normal) override;
				void window_event(graphics::window_state new_state, int x, int y) override;
				void dispatch(core::timer* time) override;
				void publish(core::timer* time) override;
				void composition() override;
				void script_hook() override;
				void initialize() override;
				core::promise<void> startup() override;
				core::promise<void> shutdown() override;
				size_t get_processed_events() const;
				bool has_processed_events() const;
				bool retrieve_initiator_object(void* ref_pointer, int ref_type_id) const;
				void* get_initiator_object() const;

			public:
				static bool wants_restart(int exit_code);
			};
#endif
			class heavy_registry final : public registry
			{
			public:
				heavy_registry() = default;
				bool bind_addons(virtual_machine* vm) noexcept override;
				static bool import_trigonometry(virtual_machine* vm) noexcept;
				static bool import_activity(virtual_machine* vm) noexcept;
				static bool import_physics(virtual_machine* vm) noexcept;
				static bool import_graphics(virtual_machine* vm) noexcept;
				static bool import_audio(virtual_machine* vm) noexcept;
				static bool import_audio_effects(virtual_machine* vm) noexcept;
				static bool import_audio_filters(virtual_machine* vm) noexcept;
				static bool import_engine(virtual_machine* vm) noexcept;
				static bool import_engine_components(virtual_machine* vm) noexcept;
				static bool import_engine_renderers(virtual_machine* vm) noexcept;
				static bool import_ui(virtual_machine* vm) noexcept;
				static bool import_ui_model(virtual_machine* vm) noexcept;
				static bool import_ui_control(virtual_machine* vm) noexcept;
			};
		}
	}
}
#endif