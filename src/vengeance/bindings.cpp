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

namespace vitex
{
	namespace scripting
	{
		namespace bindings
		{
			void handle_to_handle_cast(void* from, void** to, int type_id);
#ifdef VI_BINDINGS
			template <typename t>
			core::string get_component_name(t* base)
			{
				return base->get_name();
			}
			template <typename t>
			void populate_component(ref_class& data_class)
			{
				data_class.set_method_ex("string get_name() const", &get_component_name<t>);
				data_class.set_method("uint64 get_id() const", &t::get_id);
			}
			core::variant_args to_variant_keys(core::schema* args);

			heavy_application::heavy_application(desc& i, void* new_object, int new_type_id) noexcept : layer::heavy_application(&i), processed_events(0), initiator_type(nullptr), initiator_object(new_object)
			{
				virtual_machine* current_vm = virtual_machine::get();
				if (current_vm != nullptr && initiator_object != nullptr && ((new_type_id & (int)type_id::objhandle) || (new_type_id & (int)type_id::mask_object)))
				{
					initiator_type = current_vm->get_type_info_by_id(new_type_id).get_type_info();
					if (new_type_id & (int)type_id::objhandle)
						initiator_object = *(void**)initiator_object;
					current_vm->add_ref_object(initiator_object, initiator_type);
				}
				else if (current_vm != nullptr && initiator_object != nullptr)
				{
					if (new_type_id != (int)type_id::voidf)
						exception::throw_ptr(exception::pointer(EXCEPTION_INVALIDINITIATOR));
					initiator_object = nullptr;
				}

				if (i.usage & layer::USE_SCRIPTING)
					vm = current_vm;

				if (i.usage & layer::USE_PROCESSING)
				{
					content = new layer::heavy_content_manager();
					content->add_processor(new layer::processors::asset_processor(content), VI_HASH(TYPENAME_ASSETFILE));
					content->add_processor(new layer::processors::material_processor(content), VI_HASH(TYPENAME_MATERIAL));
					content->add_processor(new layer::processors::scene_graph_processor(content), VI_HASH(TYPENAME_SCENEGRAPH));
					content->add_processor(new layer::processors::audio_clip_processor(content), VI_HASH(TYPENAME_AUDIOCLIP));
					content->add_processor(new layer::processors::texture_2d_processor(content), VI_HASH(TYPENAME_TEXTURE2D));
					content->add_processor(new layer::processors::shader_processor(content), VI_HASH(TYPENAME_SHADER));
					content->add_processor(new layer::processors::model_processor(content), VI_HASH(TYPENAME_MODEL));
					content->add_processor(new layer::processors::skin_model_processor(content), VI_HASH(TYPENAME_SKINMODEL));
					content->add_processor(new layer::processors::skin_animation_processor(content), VI_HASH(TYPENAME_SKINANIMATION));
					content->add_processor(new layer::processors::schema_processor(content), VI_HASH(TYPENAME_SCHEMA));
					content->add_processor(new layer::processors::server_processor(content), VI_HASH(TYPENAME_HTTPSERVER));
					content->add_processor(new layer::processors::hull_shape_processor(content), VI_HASH(TYPENAME_PHYSICSHULLSHAPE));
				}
			}
			heavy_application::~heavy_application() noexcept
			{
				virtual_machine* current_vm = vm ? vm : virtual_machine::get();
				if (current_vm != nullptr && initiator_object != nullptr && initiator_type != nullptr)
					current_vm->release_object(initiator_object, initiator_type);

				on_key_event.release();
				on_input_event.release();
				on_wheel_event.release();
				on_window_event.release();
				on_dispatch.release();
				on_publish.release();
				on_composition.release();
				on_script_hook.release();
				on_initialize.release();
				on_startup.release();
				on_shutdown.release();
				initiator_object = nullptr;
				initiator_type = nullptr;
				vm = nullptr;
			}
			void heavy_application::set_on_key_event(asIScriptFunction* callback)
			{
				on_key_event = function_delegate(callback);
			}
			void heavy_application::set_on_input_event(asIScriptFunction* callback)
			{
				on_input_event = function_delegate(callback);
			}
			void heavy_application::set_on_wheel_event(asIScriptFunction* callback)
			{
				on_wheel_event = function_delegate(callback);
			}
			void heavy_application::set_on_window_event(asIScriptFunction* callback)
			{
				on_window_event = function_delegate(callback);
			}
			void heavy_application::set_on_dispatch(asIScriptFunction* callback)
			{
				on_dispatch = function_delegate(callback);
			}
			void heavy_application::set_on_publish(asIScriptFunction* callback)
			{
				on_publish = function_delegate(callback);
			}
			void heavy_application::set_on_composition(asIScriptFunction* callback)
			{
				on_composition = function_delegate(callback);
			}
			void heavy_application::set_on_script_hook(asIScriptFunction* callback)
			{
				on_script_hook = function_delegate(callback);
			}
			void heavy_application::set_on_initialize(asIScriptFunction* callback)
			{
				on_initialize = function_delegate(callback);
			}
			void heavy_application::set_on_startup(asIScriptFunction* callback)
			{
				on_startup = function_delegate(callback);
			}
			void heavy_application::set_on_shutdown(asIScriptFunction* callback)
			{
				on_shutdown = function_delegate(callback);
			}
			void heavy_application::script_hook()
			{
				if (!on_script_hook.is_valid())
					return;

				auto* context = immediate_context::get();
				VI_ASSERT(context != nullptr, "application method cannot be called outside of script context");
				context->execute_subcall(on_script_hook.callable(), nullptr);
			}
			void heavy_application::key_event(graphics::key_code key, graphics::key_mod mod, int computed, int repeat, bool pressed)
			{
				if (!on_key_event.is_valid())
					return;

				auto* context = immediate_context::get();
				VI_ASSERT(context != nullptr, "application method cannot be called outside of script context");
				context->execute_subcall(on_key_event.callable(), [key, mod, computed, repeat, pressed](immediate_context* context)
				{
					context->set_arg32(0, (int)key);
					context->set_arg32(1, (int)mod);
					context->set_arg32(2, computed);
					context->set_arg32(3, repeat);
					context->set_arg8(4, (unsigned char)pressed);
				});
			}
			void heavy_application::input_event(char* buffer, size_t length)
			{
				if (!on_input_event.is_valid())
					return;

				std::string_view text = std::string_view(buffer, length);
				auto* context = immediate_context::get();
				VI_ASSERT(context != nullptr, "application method cannot be called outside of script context");
				context->execute_subcall(on_input_event.callable(), [&text](immediate_context* context)
				{
					context->set_arg_object(0, (void*)&text);
				});
			}
			void heavy_application::wheel_event(int x, int y, bool normal)
			{
				if (!on_wheel_event.is_valid())
					return;

				auto* context = immediate_context::get();
				VI_ASSERT(context != nullptr, "application method cannot be called outside of script context");
				context->execute_subcall(on_wheel_event.callable(), [x, y, normal](immediate_context* context)
				{
					context->set_arg32(0, x);
					context->set_arg32(1, y);
					context->set_arg8(2, (unsigned char)normal);
				});
			}
			void heavy_application::window_event(graphics::window_state new_state, int x, int y)
			{
				if (!on_window_event.is_valid())
					return;

				auto* context = immediate_context::get();
				VI_ASSERT(context != nullptr, "application method cannot be called outside of script context");
				context->execute_subcall(on_window_event.callable(), [new_state, x, y](immediate_context* context)
				{
					context->set_arg32(0, (int)new_state);
					context->set_arg32(1, x);
					context->set_arg32(2, y);
				});
			}
			void heavy_application::composition()
			{
				if (!on_composition.is_valid())
					return;

				auto* context = immediate_context::get();
				VI_ASSERT(context != nullptr, "application method cannot be called outside of script context");
				context->execute_subcall(on_composition.callable(), nullptr);
			}
			void heavy_application::dispatch(core::timer* time)
			{
				auto* loop = event_loop::get();
				if (loop != nullptr)
					processed_events = loop->dequeue(vm);
				else
					processed_events = 0;

				if (on_dispatch.is_valid())
				{
					auto* context = immediate_context::get();
					VI_ASSERT(context != nullptr, "application method cannot be called outside of script context");
					context->execute_subcall(on_dispatch.callable(), [time](immediate_context* context)
					{
						context->set_arg_object(0, (void*)time);
					});
				}
			}
			void heavy_application::publish(core::timer* time)
			{
				if (!on_publish.is_valid())
					return;

				auto* context = immediate_context::get();
				VI_ASSERT(context != nullptr, "application method cannot be called outside of script context");
				context->execute_subcall(on_publish.callable(), [time](immediate_context* context)
				{
					context->set_arg_object(0, (void*)time);
				});
			}
			void heavy_application::initialize()
			{
				if (!on_initialize.is_valid())
					return;

				auto* context = immediate_context::get();
				VI_ASSERT(context != nullptr, "application method cannot be called outside of script context");
				context->execute_subcall(on_initialize.callable(), nullptr);
			}
			core::promise<void> heavy_application::startup()
			{
				if (!on_startup.is_valid())
					return core::promise<void>::null();

				VI_ASSERT(immediate_context::get() != nullptr, "application method cannot be called outside of script context");
				auto future = on_startup(nullptr);
				if (!future.is_pending())
					return core::promise<void>::null();

				return future.then(std::function<void(expects_vm<execution>&&)>(nullptr));
			}
			core::promise<void> heavy_application::shutdown()
			{
				if (!on_shutdown.is_valid())
					return core::promise<void>::null();

				VI_ASSERT(immediate_context::get() != nullptr, "application method cannot be called outside of script context");
				auto future = on_shutdown(nullptr);
				if (!future.is_pending())
					return core::promise<void>::null();

				return future.then(std::function<void(expects_vm<execution>&&)>(nullptr));
			}
			size_t heavy_application::get_processed_events() const
			{
				return processed_events;
			}
			bool heavy_application::has_processed_events() const
			{
				return processed_events > 0;
			}
			bool heavy_application::retrieve_initiator_object(void* ref_pointer, int ref_type_id) const
			{
				virtual_machine* current_vm = vm ? vm : virtual_machine::get();
				if (!initiator_object || !initiator_type || !ref_pointer || !current_vm)
					return false;

				if (ref_type_id & (size_t)type_id::objhandle)
				{
					current_vm->ref_cast_object(initiator_object, initiator_type, current_vm->get_type_info_by_id(ref_type_id), reinterpret_cast<void**>(ref_pointer));
#ifdef VI_ANGELSCRIPT
					if (*(asPWORD*)ref_pointer == 0)
						return false;
#endif
					return true;
				}
				else if (ref_type_id & (size_t)type_id::mask_object)
				{
					auto ref_type_info = current_vm->get_type_info_by_id(ref_type_id);
					if (initiator_type == ref_type_info.get_type_info())
					{
						current_vm->assign_object(ref_pointer, initiator_object, initiator_type);
						return true;
					}
				}

				return false;
			}
			void* heavy_application::get_initiator_object() const
			{
				return initiator_object;
			}
			bool heavy_application::wants_restart(int exit_code)
			{
				return exit_code == layer::EXIT_RESTART;
			}

			trigonometry::vector2& vector2_mul_eq1(trigonometry::vector2& a, const trigonometry::vector2& b)
			{
				return a *= b;
			}
			trigonometry::vector2& vector2_mul_eq2(trigonometry::vector2& a, float b)
			{
				return a *= b;
			}
			trigonometry::vector2& vector_2div_eq1(trigonometry::vector2& a, const trigonometry::vector2& b)
			{
				return a /= b;
			}
			trigonometry::vector2& vector_2div_eq2(trigonometry::vector2& a, float b)
			{
				return a /= b;
			}
			trigonometry::vector2& vector2_add_eq1(trigonometry::vector2& a, const trigonometry::vector2& b)
			{
				return a += b;
			}
			trigonometry::vector2& vector2_add_eq2(trigonometry::vector2& a, float b)
			{
				return a += b;
			}
			trigonometry::vector2& vector2_sub_eq1(trigonometry::vector2& a, const trigonometry::vector2& b)
			{
				return a -= b;
			}
			trigonometry::vector2& vector2_sub_eq2(trigonometry::vector2& a, float b)
			{
				return a -= b;
			}
			trigonometry::vector2 vector2_mul1(trigonometry::vector2& a, const trigonometry::vector2& b)
			{
				return a * b;
			}
			trigonometry::vector2 vector2_mul2(trigonometry::vector2& a, float b)
			{
				return a * b;
			}
			trigonometry::vector2 vector_2div1(trigonometry::vector2& a, const trigonometry::vector2& b)
			{
				return a / b;
			}
			trigonometry::vector2 vector_2div2(trigonometry::vector2& a, float b)
			{
				return a / b;
			}
			trigonometry::vector2 vector2_add1(trigonometry::vector2& a, const trigonometry::vector2& b)
			{
				return a + b;
			}
			trigonometry::vector2 vector2_add2(trigonometry::vector2& a, float b)
			{
				return a + b;
			}
			trigonometry::vector2 vector2_sub1(trigonometry::vector2& a, const trigonometry::vector2& b)
			{
				return a - b;
			}
			trigonometry::vector2 vector2_sub2(trigonometry::vector2& a, float b)
			{
				return a - b;
			}
			trigonometry::vector2 vector2_neg(trigonometry::vector2& a)
			{
				return -a;
			}
			bool vector2_eq(trigonometry::vector2& a, const trigonometry::vector2& b)
			{
				return a == b;
			}
			int vector2_cmp(trigonometry::vector2& a, const trigonometry::vector2& b)
			{
				if (a == b)
					return 0;

				return a > b ? 1 : -1;
			}

			trigonometry::vector3& vector3_mul_eq1(trigonometry::vector3& a, const trigonometry::vector3& b)
			{
				return a *= b;
			}
			trigonometry::vector3& vector3_mul_eq2(trigonometry::vector3& a, float b)
			{
				return a *= b;
			}
			trigonometry::vector3& vector_3div_eq1(trigonometry::vector3& a, const trigonometry::vector3& b)
			{
				return a /= b;
			}
			trigonometry::vector3& vector_3div_eq2(trigonometry::vector3& a, float b)
			{
				return a /= b;
			}
			trigonometry::vector3& vector3_add_eq1(trigonometry::vector3& a, const trigonometry::vector3& b)
			{
				return a += b;
			}
			trigonometry::vector3& vector3_add_eq2(trigonometry::vector3& a, float b)
			{
				return a += b;
			}
			trigonometry::vector3& vector3_sub_eq1(trigonometry::vector3& a, const trigonometry::vector3& b)
			{
				return a -= b;
			}
			trigonometry::vector3& vector3_sub_eq2(trigonometry::vector3& a, float b)
			{
				return a -= b;
			}
			trigonometry::vector3 vector3_mul1(trigonometry::vector3& a, const trigonometry::vector3& b)
			{
				return a * b;
			}
			trigonometry::vector3 vector3_mul2(trigonometry::vector3& a, float b)
			{
				return a * b;
			}
			trigonometry::vector3 vector_3div1(trigonometry::vector3& a, const trigonometry::vector3& b)
			{
				return a / b;
			}
			trigonometry::vector3 vector_3div2(trigonometry::vector3& a, float b)
			{
				return a / b;
			}
			trigonometry::vector3 vector3_add1(trigonometry::vector3& a, const trigonometry::vector3& b)
			{
				return a + b;
			}
			trigonometry::vector3 vector3_add2(trigonometry::vector3& a, float b)
			{
				return a + b;
			}
			trigonometry::vector3 vector3_sub1(trigonometry::vector3& a, const trigonometry::vector3& b)
			{
				return a - b;
			}
			trigonometry::vector3 vector3_sub2(trigonometry::vector3& a, float b)
			{
				return a - b;
			}
			trigonometry::vector3 vector3_neg(trigonometry::vector3& a)
			{
				return -a;
			}
			bool vector3_eq(trigonometry::vector3& a, const trigonometry::vector3& b)
			{
				return a == b;
			}
			int vector3_cmp(trigonometry::vector3& a, const trigonometry::vector3& b)
			{
				if (a == b)
					return 0;

				return a > b ? 1 : -1;
			}

			trigonometry::vector4& vector4_mul_eq1(trigonometry::vector4& a, const trigonometry::vector4& b)
			{
				return a *= b;
			}
			trigonometry::vector4& vector4_mul_eq2(trigonometry::vector4& a, float b)
			{
				return a *= b;
			}
			trigonometry::vector4& vector4_div_eq1(trigonometry::vector4& a, const trigonometry::vector4& b)
			{
				return a /= b;
			}
			trigonometry::vector4& vector4_div_eq2(trigonometry::vector4& a, float b)
			{
				return a /= b;
			}
			trigonometry::vector4& vector4_add_eq1(trigonometry::vector4& a, const trigonometry::vector4& b)
			{
				return a += b;
			}
			trigonometry::vector4& vector4_add_eq2(trigonometry::vector4& a, float b)
			{
				return a += b;
			}
			trigonometry::vector4& vector4_sub_eq1(trigonometry::vector4& a, const trigonometry::vector4& b)
			{
				return a -= b;
			}
			trigonometry::vector4& vector4_sub_eq2(trigonometry::vector4& a, float b)
			{
				return a -= b;
			}
			trigonometry::vector4 vector4_mul1(trigonometry::vector4& a, const trigonometry::vector4& b)
			{
				return a * b;
			}
			trigonometry::vector4 vector4_mul2(trigonometry::vector4& a, float b)
			{
				return a * b;
			}
			trigonometry::vector4 vector4_div1(trigonometry::vector4& a, const trigonometry::vector4& b)
			{
				return a / b;
			}
			trigonometry::vector4 vector4_div2(trigonometry::vector4& a, float b)
			{
				return a / b;
			}
			trigonometry::vector4 vector4_add1(trigonometry::vector4& a, const trigonometry::vector4& b)
			{
				return a + b;
			}
			trigonometry::vector4 vector4_add2(trigonometry::vector4& a, float b)
			{
				return a + b;
			}
			trigonometry::vector4 vector4_sub1(trigonometry::vector4& a, const trigonometry::vector4& b)
			{
				return a - b;
			}
			trigonometry::vector4 vector4_sub2(trigonometry::vector4& a, float b)
			{
				return a - b;
			}
			trigonometry::vector4 vector4_neg(trigonometry::vector4& a)
			{
				return -a;
			}
			bool vector4_eq(trigonometry::vector4& a, const trigonometry::vector4& b)
			{
				return a == b;
			}
			int vector4_cmp(trigonometry::vector4& a, const trigonometry::vector4& b)
			{
				if (a == b)
					return 0;

				return a > b ? 1 : -1;
			}

			trigonometry::matrix4x4 matrix4x4_mul1(trigonometry::matrix4x4& a, const trigonometry::matrix4x4& b)
			{
				return a * b;
			}
			trigonometry::vector4 matrix4x4_mul2(trigonometry::matrix4x4& a, const trigonometry::vector4& b)
			{
				return a * b;
			}
			bool matrix4x4_eq(trigonometry::matrix4x4& a, const trigonometry::matrix4x4& b)
			{
				return a == b;
			}
			float& matrix4x4_get_row(trigonometry::matrix4x4& base, size_t index)
			{
				return base.row[index % 16];
			}

			trigonometry::vector3 quaternion_mul1(trigonometry::quaternion& a, const trigonometry::vector3& b)
			{
				return a * b;
			}
			trigonometry::quaternion quaternion_mul2(trigonometry::quaternion& a, const trigonometry::quaternion& b)
			{
				return a * b;
			}
			trigonometry::quaternion quaternion_mul3(trigonometry::quaternion& a, float b)
			{
				return a * b;
			}
			trigonometry::quaternion quaternion_add(trigonometry::quaternion& a, const trigonometry::quaternion& b)
			{
				return a + b;
			}
			trigonometry::quaternion quaternion_sub(trigonometry::quaternion& a, const trigonometry::quaternion& b)
			{
				return a - b;
			}

			trigonometry::vector4& frustum8cget_corners(trigonometry::frustum8c& base, size_t index)
			{
				return base.corners[index % 8];
			}

			trigonometry::vector4& frustum6pget_corners(trigonometry::frustum6p& base, size_t index)
			{
				return base.planes[index % 6];
			}

			size_t joint_size(trigonometry::joint& base)
			{
				return base.childs.size();
			}
			trigonometry::joint& joint_get_childs(trigonometry::joint& base, size_t index)
			{
				if (base.childs.empty())
					return base;

				return base.childs[index % base.childs.size()];
			}

			size_t skin_animator_key_size(trigonometry::skin_animator_key& base)
			{
				return base.pose.size();
			}
			trigonometry::animator_key& skin_animator_key_get_pose(trigonometry::skin_animator_key& base, size_t index)
			{
				if (base.pose.empty())
				{
					base.pose.push_back(trigonometry::animator_key());
					return base.pose.front();
				}

				return base.pose[index % base.pose.size()];
			}

			size_t skin_animator_clip_size(trigonometry::skin_animator_clip& base)
			{
				return base.keys.size();
			}
			trigonometry::skin_animator_key& skin_animator_clip_get_keys(trigonometry::skin_animator_clip& base, size_t index)
			{
				if (base.keys.empty())
				{
					base.keys.push_back(trigonometry::skin_animator_key());
					return base.keys.front();
				}

				return base.keys[index % base.keys.size()];
			}

			size_t key_animator_clip_size(trigonometry::key_animator_clip& base)
			{
				return base.keys.size();
			}
			trigonometry::animator_key& key_animator_clip_get_keys(trigonometry::key_animator_clip& base, size_t index)
			{
				if (base.keys.empty())
				{
					base.keys.push_back(trigonometry::animator_key());
					return base.keys.front();
				}

				return base.keys[index % base.keys.size()];
			}

			void cosmos_query_index(trigonometry::cosmos* base, asIScriptFunction* overlaps, asIScriptFunction* match)
			{
				immediate_context* context = immediate_context::get();
				function_delegate overlaps_callback(overlaps);
				function_delegate match_callback(match);
				if (!context || !overlaps_callback.is_valid() || !match_callback.is_valid())
					return;

				trigonometry::cosmos::iterator iterator;
				base->query_index<void>(iterator, [context, &overlaps_callback](const trigonometry::bounding& box)
				{
					bool result = false;
					context->execute_subcall(overlaps_callback.callable(), [&box](immediate_context* context)
					{
						context->set_arg_object(0, (void*)&box);
					}, [&result](immediate_context* context)
					{
						result = (bool)context->get_return_byte();
					});
					return result;
				}, [context, &match_callback](void* item)
				{
					context->execute_subcall(match_callback.callable(), [item](immediate_context* context) { context->set_arg_address(0, item); });
				});
			}

			array* hull_shape_get_vertices(physics::hull_shape* base)
			{
				typeinfo type = virtual_machine::get()->get_type_info_by_decl(TYPENAME_ARRAY "<" TYPENAME_VERTEX ">@");
				return array::compose(type.get_type_info(), base->get_vertices());
			}
			array* hull_shape_get_indices(physics::hull_shape* base)
			{
				typeinfo type = virtual_machine::get()->get_type_info_by_decl(TYPENAME_ARRAY "<int>@");
				return array::compose(type.get_type_info(), base->get_indices());
			}

			physics::soft_body::desc::cv::sconvex& soft_body_sconvex_copy(physics::soft_body::desc::cv::sconvex& base, physics::soft_body::desc::cv::sconvex& other)
			{
				if (&base == &other)
					return base;

				core::memory::release(base.hull);
				base = other;
				if (base.hull != nullptr)
					base.hull->add_ref();

				return base;
			}
			void soft_body_sconvex_destructor(physics::soft_body::desc::cv::sconvex& base)
			{
				core::memory::release(base.hull);
			}

			array* soft_body_get_indices(physics::soft_body* base)
			{
				core::vector<int> result;
				base->get_indices(&result);

				typeinfo type = virtual_machine::get()->get_type_info_by_decl(TYPENAME_ARRAY "<int>@");
				return array::compose(type.get_type_info(), result);
			}
			array* soft_body_get_vertices(physics::soft_body* base)
			{
				core::vector<trigonometry::vertex> result;
				base->get_vertices(&result);

				typeinfo type = virtual_machine::get()->get_type_info_by_decl(TYPENAME_ARRAY "<" TYPENAME_VERTEX ">@");
				return array::compose(type.get_type_info(), result);
			}

			btCollisionShape* simulator_create_convex_hull_skin_vertex(physics::simulator* base, array* data)
			{
				auto value = array::decompose<trigonometry::skin_vertex>(data);
				return base->create_convex_hull(value);
			}
			btCollisionShape* simulator_create_convex_hull_vertex(physics::simulator* base, array* data)
			{
				auto value = array::decompose<trigonometry::vertex>(data);
				return base->create_convex_hull(value);
			}
			btCollisionShape* simulator_create_convex_hull_vector2(physics::simulator* base, array* data)
			{
				auto value = array::decompose<trigonometry::vector2>(data);
				return base->create_convex_hull(value);
			}
			btCollisionShape* simulator_create_convex_hull_vector3(physics::simulator* base, array* data)
			{
				auto value = array::decompose<trigonometry::vector3>(data);
				return base->create_convex_hull(value);
			}
			btCollisionShape* simulator_create_convex_hull_vector4(physics::simulator* base, array* data)
			{
				auto value = array::decompose<trigonometry::vector4>(data);
				return base->create_convex_hull(value);
			}
			array* simulator_get_shape_vertices(physics::simulator* base, btCollisionShape* shape)
			{
				typeinfo type = virtual_machine::get()->get_type_info_by_decl(TYPENAME_ARRAY "<" TYPENAME_VECTOR3 ">@");
				return array::compose(type.get_type_info(), base->get_shape_vertices(shape));
			}

			void audio_effect_set_filter(audio::audio_effect* base, audio::audio_filter* init)
			{
				audio::audio_filter* copy = init;
				expects_wrapper::unwrap_void(base->set_filter(&copy));
			}

			void alert_result(graphics::alert& base, asIScriptFunction* callback)
			{
				immediate_context* context = immediate_context::get();
				function_delegate delegatef(callback);
				if (!context || !delegatef.is_valid())
					return base.result(nullptr);

				base.result([context, delegatef](int id)
				{
					context->execute_subcall(delegatef.callable(), [id](immediate_context* context)
					{
						context->set_arg32(0, id);
					});
				});
			}

			void activity_set_app_state_change(graphics::activity* base, asIScriptFunction* callback)
			{
				immediate_context* context = immediate_context::get();
				if (context != nullptr && callback != nullptr)
				{
					function_delegate delegatef(callback);
					base->callbacks.app_state_change = [context, delegatef](graphics::app_state type)
					{
						context->execute_subcall(delegatef.callable(), [type](immediate_context* context)
						{
							context->set_arg32(0, (int)type);
						});
					};
				}
				else
					base->callbacks.app_state_change = nullptr;
			}
			void activity_set_window_state_change(graphics::activity* base, asIScriptFunction* callback)
			{
				immediate_context* context = immediate_context::get();
				if (context != nullptr && callback != nullptr)
				{
					function_delegate delegatef(callback);
					base->callbacks.window_state_change = [context, delegatef](graphics::window_state state, int x, int y)
					{
						context->execute_subcall(delegatef.callable(), [state, x, y](immediate_context* context)
						{
							context->set_arg32(0, (int)state);
							context->set_arg32(1, x);
							context->set_arg32(2, y);
						});
					};
				}
				else
					base->callbacks.window_state_change = nullptr;
			}
			void activity_set_key_state(graphics::activity* base, asIScriptFunction* callback)
			{
				immediate_context* context = immediate_context::get();
				if (context != nullptr && callback != nullptr)
				{
					function_delegate delegatef(callback);
					base->callbacks.key_state = [context, delegatef](graphics::key_code code, graphics::key_mod mod, int x, int y, bool value)
					{
						context->execute_subcall(delegatef.callable(), [code, mod, x, y, value](immediate_context* context)
						{
							context->set_arg32(0, (int)code);
							context->set_arg32(1, (int)mod);
							context->set_arg32(2, x);
							context->set_arg32(3, y);
							context->set_arg8(4, value);
						});
					};
				}
				else
					base->callbacks.key_state = nullptr;
			}
			void activity_set_input_edit(graphics::activity* base, asIScriptFunction* callback)
			{
				immediate_context* context = immediate_context::get();
				if (context != nullptr && callback != nullptr)
				{
					function_delegate delegatef(callback);
					base->callbacks.input_edit = [context, delegatef](char* buffer, int x, int y)
					{
						std::string_view text = (buffer ? buffer : std::string_view());
						context->execute_subcall(delegatef.callable(), [text, x, y](immediate_context* context)
						{
							context->set_arg_object(0, (void*)&text);
							context->set_arg32(1, x);
							context->set_arg32(2, y);
						});
					};
				}
				else
					base->callbacks.input_edit = nullptr;
			}
			void activity_set_input(graphics::activity* base, asIScriptFunction* callback)
			{
				immediate_context* context = immediate_context::get();
				if (context != nullptr && callback != nullptr)
				{
					function_delegate delegatef(callback);
					base->callbacks.input = [context, delegatef](char* buffer, int x)
					{
						std::string_view text = (buffer ? buffer : std::string_view());
						context->execute_subcall(delegatef.callable(), [text, x](immediate_context* context)
						{
							context->set_arg_object(0, (void*)&text);
							context->set_arg32(1, x);
						});
					};
				}
				else
					base->callbacks.input = nullptr;
			}
			void activity_set_cursor_move(graphics::activity* base, asIScriptFunction* callback)
			{
				immediate_context* context = immediate_context::get();
				if (context != nullptr && callback != nullptr)
				{
					function_delegate delegatef(callback);
					base->callbacks.cursor_move = [context, delegatef](int x, int y, int z, int w)
					{
						context->execute_subcall(delegatef.callable(), [x, y, z, w](immediate_context* context)
						{
							context->set_arg32(0, x);
							context->set_arg32(1, y);
							context->set_arg32(2, z);
							context->set_arg32(3, w);
						});
					};
				}
				else
					base->callbacks.cursor_move = nullptr;
			}
			void activity_set_cursor_wheel_state(graphics::activity* base, asIScriptFunction* callback)
			{
				immediate_context* context = immediate_context::get();
				if (context != nullptr && callback != nullptr)
				{
					function_delegate delegatef(callback);
					base->callbacks.cursor_wheel_state = [context, delegatef](int x, int y, bool z)
					{
						context->execute_subcall(delegatef.callable(), [x, y, z](immediate_context* context)
						{
							context->set_arg32(0, x);
							context->set_arg32(1, y);
							context->set_arg8(2, z);
						});
					};
				}
				else
					base->callbacks.cursor_wheel_state = nullptr;
			}
			void activity_set_joy_stick_axis_move(graphics::activity* base, asIScriptFunction* callback)
			{
				immediate_context* context = immediate_context::get();
				if (context != nullptr && callback != nullptr)
				{
					function_delegate delegatef(callback);
					base->callbacks.joy_stick_axis_move = [context, delegatef](int x, int y, int z)
					{
						context->execute_subcall(delegatef.callable(), [x, y, z](immediate_context* context)
						{
							context->set_arg32(0, x);
							context->set_arg32(1, y);
							context->set_arg32(2, z);
						});
					};
				}
				else
					base->callbacks.joy_stick_axis_move = nullptr;
			}
			void activity_set_joy_stick_ball_move(graphics::activity* base, asIScriptFunction* callback)
			{
				immediate_context* context = immediate_context::get();
				if (context != nullptr && callback != nullptr)
				{
					function_delegate delegatef(callback);
					base->callbacks.joy_stick_ball_move = [context, delegatef](int x, int y, int z, int w)
					{
						context->execute_subcall(delegatef.callable(), [x, y, z, w](immediate_context* context)
						{
							context->set_arg32(0, x);
							context->set_arg32(1, y);
							context->set_arg32(2, z);
							context->set_arg32(3, w);
						});
					};
				}
				else
					base->callbacks.joy_stick_ball_move = nullptr;
			}
			void activity_set_joy_stick_hat_move(graphics::activity* base, asIScriptFunction* callback)
			{
				immediate_context* context = immediate_context::get();
				if (context != nullptr && callback != nullptr)
				{
					function_delegate delegatef(callback);
					base->callbacks.joy_stick_hat_move = [context, delegatef](graphics::joy_stick_hat type, int x, int y)
					{
						context->execute_subcall(delegatef.callable(), [type, x, y](immediate_context* context)
						{
							context->set_arg32(0, (int)type);
							context->set_arg32(1, x);
							context->set_arg32(2, y);
						});
					};
				}
				else
					base->callbacks.joy_stick_hat_move = nullptr;
			}
			void activity_set_joy_stick_key_state(graphics::activity* base, asIScriptFunction* callback)
			{
				immediate_context* context = immediate_context::get();
				if (context != nullptr && callback != nullptr)
				{
					function_delegate delegatef(callback);
					base->callbacks.joy_stick_key_state = [context, delegatef](int x, int y, bool z)
					{
						context->execute_subcall(delegatef.callable(), [x, y, z](immediate_context* context)
						{
							context->set_arg32(0, x);
							context->set_arg32(1, y);
							context->set_arg8(2, z);
						});
					};
				}
				else
					base->callbacks.joy_stick_key_state = nullptr;
			}
			void activity_set_joy_stick_state(graphics::activity* base, asIScriptFunction* callback)
			{
				immediate_context* context = immediate_context::get();
				if (context != nullptr && callback != nullptr)
				{
					function_delegate delegatef(callback);
					base->callbacks.joy_stick_state = [context, delegatef](int x, bool y)
					{
						context->execute_subcall(delegatef.callable(), [x, y](immediate_context* context)
						{
							context->set_arg32(0, x);
							context->set_arg8(1, y);
						});
					};
				}
				else
					base->callbacks.joy_stick_state = nullptr;
			}
			void activity_set_controller_axis_move(graphics::activity* base, asIScriptFunction* callback)
			{
				immediate_context* context = immediate_context::get();
				if (context != nullptr && callback != nullptr)
				{
					function_delegate delegatef(callback);
					base->callbacks.controller_axis_move = [context, delegatef](int x, int y, int z)
					{
						context->execute_subcall(delegatef.callable(), [x, y, z](immediate_context* context)
						{
							context->set_arg32(0, x);
							context->set_arg32(1, y);
							context->set_arg32(2, z);
						});
					};
				}
				else
					base->callbacks.controller_axis_move = nullptr;
			}
			void activity_set_controller_key_state(graphics::activity* base, asIScriptFunction* callback)
			{
				immediate_context* context = immediate_context::get();
				if (context != nullptr && callback != nullptr)
				{
					function_delegate delegatef(callback);
					base->callbacks.controller_key_state = [context, delegatef](int x, int y, bool z)
					{
						context->execute_subcall(delegatef.callable(), [x, y, z](immediate_context* context)
						{
							context->set_arg32(0, x);
							context->set_arg32(1, y);
							context->set_arg8(2, z);
						});
					};
				}
				else
					base->callbacks.controller_key_state = nullptr;
			}
			void activity_set_controller_state(graphics::activity* base, asIScriptFunction* callback)
			{
				immediate_context* context = immediate_context::get();
				if (context != nullptr && callback != nullptr)
				{
					function_delegate delegatef(callback);
					base->callbacks.controller_state = [context, delegatef](int x, int y)
					{
						context->execute_subcall(delegatef.callable(), [x, y](immediate_context* context)
						{
							context->set_arg32(0, x);
							context->set_arg32(1, y);
						});
					};
				}
				else
					base->callbacks.controller_state = nullptr;
			}
			void activity_set_touch_move(graphics::activity* base, asIScriptFunction* callback)
			{
				immediate_context* context = immediate_context::get();
				if (context != nullptr && callback != nullptr)
				{
					function_delegate delegatef(callback);
					base->callbacks.touch_move = [context, delegatef](int x, int y, float z, float w, float X1, float Y1, float Z1)
					{
						context->execute_subcall(delegatef.callable(), [x, y, z, w, X1, Y1, Z1](immediate_context* context)
						{
							context->set_arg32(0, x);
							context->set_arg32(1, y);
							context->set_arg_float(2, z);
							context->set_arg_float(3, w);
							context->set_arg_float(4, X1);
							context->set_arg_float(5, Y1);
							context->set_arg_float(6, Z1);
						});
					};
				}
				else
					base->callbacks.touch_move = nullptr;
			}
			void activity_set_touch_state(graphics::activity* base, asIScriptFunction* callback)
			{
				immediate_context* context = immediate_context::get();
				if (context != nullptr && callback != nullptr)
				{
					function_delegate delegatef(callback);
					base->callbacks.touch_state = [context, delegatef](int x, int y, float z, float w, float X1, float Y1, float Z1, bool W1)
					{
						context->execute_subcall(delegatef.callable(), [x, y, z, w, X1, Y1, Z1, W1](immediate_context* context)
						{
							context->set_arg32(0, x);
							context->set_arg32(1, y);
							context->set_arg_float(2, z);
							context->set_arg_float(3, w);
							context->set_arg_float(4, X1);
							context->set_arg_float(5, Y1);
							context->set_arg_float(6, Z1);
							context->set_arg8(7, W1);
						});
					};
				}
				else
					base->callbacks.touch_state = nullptr;
			}
			void activity_set_gesture_state(graphics::activity* base, asIScriptFunction* callback)
			{
				immediate_context* context = immediate_context::get();
				if (context != nullptr && callback != nullptr)
				{
					function_delegate delegatef(callback);
					base->callbacks.gesture_state = [context, delegatef](int x, int y, int z, float w, float X1, float Y1, bool Z1)
					{
						context->execute_subcall(delegatef.callable(), [x, y, z, w, X1, Y1, Z1](immediate_context* context)
						{
							context->set_arg32(0, x);
							context->set_arg32(1, y);
							context->set_arg32(2, z);
							context->set_arg_float(3, w);
							context->set_arg_float(4, X1);
							context->set_arg_float(5, Y1);
							context->set_arg8(6, Z1);
						});
					};
				}
				else
					base->callbacks.gesture_state = nullptr;
			}
			void activity_set_multi_gesture_state(graphics::activity* base, asIScriptFunction* callback)
			{
				immediate_context* context = immediate_context::get();
				if (context != nullptr && callback != nullptr)
				{
					function_delegate delegatef(callback);
					base->callbacks.multi_gesture_state = [context, delegatef](int x, int y, float z, float w, float X1, float Y1)
					{
						context->execute_subcall(delegatef.callable(), [x, y, z, w, X1, Y1](immediate_context* context)
						{
							context->set_arg32(0, x);
							context->set_arg32(1, y);
							context->set_arg_float(2, z);
							context->set_arg_float(3, w);
							context->set_arg_float(4, X1);
							context->set_arg_float(5, Y1);
						});
					};
				}
				else
					base->callbacks.multi_gesture_state = nullptr;
			}
			void activity_set_drop_file(graphics::activity* base, asIScriptFunction* callback)
			{
				immediate_context* context = immediate_context::get();
				if (context != nullptr && callback != nullptr)
				{
					function_delegate delegatef(callback);
					base->callbacks.drop_file = [context, delegatef](const std::string_view& text)
					{
						context->execute_subcall(delegatef.callable(), [text](immediate_context* context)
						{
							context->set_arg_object(0, (void*)&text);
						});
					};
				}
				else
					base->callbacks.drop_file = nullptr;
			}
			void activity_set_drop_text(graphics::activity* base, asIScriptFunction* callback)
			{
				immediate_context* context = immediate_context::get();
				if (context != nullptr && callback != nullptr)
				{
					function_delegate delegatef(callback);
					base->callbacks.drop_text = [context, delegatef](const std::string_view& text)
					{
						context->execute_subcall(delegatef.callable(), [text](immediate_context* context)
						{
							context->set_arg_object(0, (void*)&text);
						});
					};
				}
				else
					base->callbacks.drop_text = nullptr;
			}

			graphics::render_target_blend_state& blend_state_desc_get_render_target(graphics::blend_state::desc& base, size_t index)
			{
				return base.render_target[index % 8];
			}

			float& sampler_state_desc_get_border_color(graphics::sampler_state::desc& base, size_t index)
			{
				return base.border_color[index % 4];
			}

			graphics::input_layout::desc& input_layout_desc_copy(graphics::input_layout::desc& base, graphics::input_layout::desc& other)
			{
				if (&base == &other)
					return base;

				core::memory::release(base.source);
				base = other;
				if (base.source != nullptr)
					base.source->add_ref();

				return base;
			}
			void input_layout_desc_destructor(graphics::input_layout::desc& base)
			{
				core::memory::release(base.source);
			}
			void input_layout_desc_set_attributes(graphics::input_layout::desc& base, array* data)
			{
				base.attributes = array::decompose<graphics::input_layout::attribute>(data);
			}

			array* input_layout_get_attributes(graphics::input_layout* base)
			{
				typeinfo type = virtual_machine::get()->get_type_info_by_decl(TYPENAME_ARRAY "<" TYPENAME_INPUTLAYOUTATTRIBUTE ">@");
				return array::compose(type.get_type_info(), base->get_attributes());
			}

			void shader_desc_set_defines(graphics::shader::desc& base, array* data)
			{
				base.defines = array::decompose<core::string>(data);
			}

			void mesh_buffer_desc_set_elements(graphics::mesh_buffer::desc& base, array* data)
			{
				base.elements = array::decompose<trigonometry::vertex>(data);
			}
			void mesh_buffer_desc_set_indices(graphics::mesh_buffer::desc& base, array* data)
			{
				base.indices = array::decompose<int>(data);
			}

			void skin_mesh_buffer_desc_set_elements(graphics::skin_mesh_buffer::desc& base, array* data)
			{
				base.elements = array::decompose<trigonometry::skin_vertex>(data);
			}
			void skin_mesh_buffer_desc_set_indices(graphics::skin_mesh_buffer::desc& base, array* data)
			{
				base.indices = array::decompose<int>(data);
			}

			graphics::instance_buffer::desc& instance_buffer_desc_copy(graphics::instance_buffer::desc& base, graphics::instance_buffer::desc& other)
			{
				if (&base == &other)
					return base;

				core::memory::release(base.device);
				base = other;
				if (base.device != nullptr)
					base.device->add_ref();

				return base;
			}
			void instance_buffer_desc_destructor(graphics::instance_buffer::desc& base)
			{
				core::memory::release(base.device);
			}

			void instance_buffer_set_array(graphics::instance_buffer* base, array* data)
			{
				auto& source = base->get_array();
				source = array::decompose<trigonometry::element_vertex>(data);
			}
			array* instance_buffer_get_array(graphics::instance_buffer* base)
			{
				typeinfo type = virtual_machine::get()->get_type_info_by_decl(TYPENAME_ARRAY "<" TYPENAME_ELEMENTVERTEX ">@");
				return array::compose(type.get_type_info(), base->get_array());
			}

			void multi_render_target_2ddesc_set_format_mode(graphics::multi_render_target_2d::desc& base, size_t index, graphics::format mode)
			{
				base.format_mode[index % 8] = mode;
			}

			void multi_render_target_cube_desc_set_format_mode(graphics::multi_render_target_cube::desc& base, size_t index, graphics::format mode)
			{
				base.format_mode[index % 8] = mode;
			}

			graphics::cubemap::desc& cubemap_desc_copy(graphics::cubemap::desc& base, graphics::cubemap::desc& other)
			{
				if (&base == &other)
					return base;

				core::memory::release(base.source);
				base = other;
				if (base.source != nullptr)
					base.source->add_ref();

				return base;
			}
			void cubemap_desc_destructor(graphics::cubemap::desc& base)
			{
				core::memory::release(base.source);
			}

			graphics::graphics_device::desc& graphics_device_desc_copy(graphics::graphics_device::desc& base, graphics::graphics_device::desc& other)
			{
				if (&base == &other)
					return base;

				core::memory::release(base.window);
				base = other;
				if (base.window != nullptr)
					base.window->add_ref();

				return base;
			}
			void graphics_device_desc_destructor(graphics::graphics_device::desc& base)
			{
				core::memory::release(base.window);
			}

			void graphics_device_set_vertex_buffers(graphics::graphics_device* base, array* data, bool value)
			{
				core::vector<graphics::element_buffer*> buffer = array::decompose<graphics::element_buffer*>(data);
				base->set_vertex_buffers(buffer.data(), (uint32_t)buffer.size(), value);
			}
			void graphics_device_set_writeable1(graphics::graphics_device* base, array* data, uint32_t slot, bool value)
			{
				core::vector<graphics::element_buffer*> buffer = array::decompose<graphics::element_buffer*>(data);
				base->set_writeable(buffer.data(), slot, (uint32_t)buffer.size(), value);
			}
			void graphics_device_set_writeable2(graphics::graphics_device* base, array* data, uint32_t slot, bool value)
			{
				core::vector<graphics::texture_2d*> buffer = array::decompose<graphics::texture_2d*>(data);
				base->set_writeable(buffer.data(), slot, (uint32_t)buffer.size(), value);
			}
			void graphics_device_set_writeable3(graphics::graphics_device* base, array* data, uint32_t slot, bool value)
			{
				core::vector<graphics::texture_3d*> buffer = array::decompose<graphics::texture_3d*>(data);
				base->set_writeable(buffer.data(), slot, (uint32_t)buffer.size(), value);
			}
			void graphics_device_set_writeable4(graphics::graphics_device* base, array* data, uint32_t slot, bool value)
			{
				core::vector<graphics::texture_cube*> buffer = array::decompose<graphics::texture_cube*>(data);
				base->set_writeable(buffer.data(), slot, (uint32_t)buffer.size(), value);
			}
			void graphics_device_set_target_map(graphics::graphics_device* base, graphics::render_target* target, array* data)
			{
				core::vector<bool> buffer = array::decompose<bool>(data);
				while (buffer.size() < 8)
					buffer.push_back(false);

				bool map[8];
				for (size_t i = 0; i < 8; i++)
					map[i] = buffer[i];

				base->set_target_map(target, map);
			}
			void graphics_device_set_viewports(graphics::graphics_device* base, array* data)
			{
				core::vector<graphics::viewport> buffer = array::decompose<graphics::viewport>(data);
				base->set_viewports((uint32_t)buffer.size(), buffer.data());
			}
			void graphics_device_set_scissor_rects(graphics::graphics_device* base, array* data)
			{
				core::vector<trigonometry::rectangle> buffer = array::decompose<trigonometry::rectangle>(data);
				base->set_scissor_rects((uint32_t)buffer.size(), buffer.data());
			}
			bool graphics_device_map1(graphics::graphics_device* base, graphics::element_buffer* resource, graphics::resource_map type, graphics::mapped_subresource* result)
			{
				return expects_wrapper::unwrap_void(base->map(resource, type, result));
			}
			bool graphics_device_map2(graphics::graphics_device* base, graphics::texture_2d* resource, graphics::resource_map type, graphics::mapped_subresource* result)
			{
				return expects_wrapper::unwrap_void(base->map(resource, type, result));
			}
			bool graphics_device_map3(graphics::graphics_device* base, graphics::texture_cube* resource, graphics::resource_map type, graphics::mapped_subresource* result)
			{
				return expects_wrapper::unwrap_void(base->map(resource, type, result));
			}
			bool graphics_device_map4(graphics::graphics_device* base, graphics::texture_3d* resource, graphics::resource_map type, graphics::mapped_subresource* result)
			{
				return expects_wrapper::unwrap_void(base->map(resource, type, result));
			}
			bool graphics_device_unmap1(graphics::graphics_device* base, graphics::element_buffer* resource, graphics::mapped_subresource* result)
			{
				return expects_wrapper::unwrap_void(base->unmap(resource, result));
			}
			bool graphics_device_unmap2(graphics::graphics_device* base, graphics::texture_2d* resource, graphics::mapped_subresource* result)
			{
				return expects_wrapper::unwrap_void(base->unmap(resource, result));
			}
			bool graphics_device_unmap3(graphics::graphics_device* base, graphics::texture_cube* resource, graphics::mapped_subresource* result)
			{
				return expects_wrapper::unwrap_void(base->unmap(resource, result));
			}
			bool graphics_device_unmap4(graphics::graphics_device* base, graphics::texture_3d* resource, graphics::mapped_subresource* result)
			{
				return expects_wrapper::unwrap_void(base->unmap(resource, result));
			}
			bool graphics_device_update_constant_buffer(graphics::graphics_device* base, graphics::element_buffer* resource, void* data, size_t size)
			{
				return expects_wrapper::unwrap_void(base->update_constant_buffer(resource, data, size));
			}
			bool graphics_device_update_buffer1(graphics::graphics_device* base, graphics::element_buffer* resource, void* data, size_t size)
			{
				return expects_wrapper::unwrap_void(base->update_buffer(resource, data, size));
			}
			bool graphics_device_update_buffer2(graphics::graphics_device* base, graphics::shader* resource, const void* data)
			{
				return expects_wrapper::unwrap_void(base->update_buffer(resource, data));
			}
			bool graphics_device_update_buffer3(graphics::graphics_device* base, graphics::mesh_buffer* resource, trigonometry::vertex* data)
			{
				return expects_wrapper::unwrap_void(base->update_buffer(resource, data));
			}
			bool graphics_device_update_buffer4(graphics::graphics_device* base, graphics::skin_mesh_buffer* resource, trigonometry::skin_vertex* data)
			{
				return expects_wrapper::unwrap_void(base->update_buffer(resource, data));
			}
			bool graphics_device_update_buffer5(graphics::graphics_device* base, graphics::instance_buffer* resource)
			{
				return expects_wrapper::unwrap_void(base->update_buffer(resource));
			}
			bool graphics_device_update_buffer_size1(graphics::graphics_device* base, graphics::shader* resource, size_t size)
			{
				return expects_wrapper::unwrap_void(base->update_buffer_size(resource, size));
			}
			bool graphics_device_update_buffer_size2(graphics::graphics_device* base, graphics::instance_buffer* resource, size_t size)
			{
				return expects_wrapper::unwrap_void(base->update_buffer_size(resource, size));
			}
			array* graphics_device_get_viewports(graphics::graphics_device* base)
			{
				core::vector<graphics::viewport> viewports;
				viewports.resize(32);

				uint32_t count = 0;
				base->get_viewports(&count, viewports.data());
				viewports.resize((size_t)count);

				typeinfo type = virtual_machine::get()->get_type_info_by_decl(TYPENAME_ARRAY "<" TYPENAME_VIEWPORT ">@");
				return array::compose(type.get_type_info(), viewports);
			}
			array* graphics_device_get_scissor_rects(graphics::graphics_device* base)
			{
				core::vector<trigonometry::rectangle> rects;
				rects.resize(32);

				uint32_t count = 0;
				base->get_scissor_rects(&count, rects.data());
				rects.resize((size_t)count);

				typeinfo type = virtual_machine::get()->get_type_info_by_decl(TYPENAME_ARRAY "<" TYPENAME_RECTANGLE ">@");
				return array::compose(type.get_type_info(), rects);
			}
			bool graphics_device_generate_texture1(graphics::graphics_device* base, graphics::texture_2d* resource)
			{
				return expects_wrapper::unwrap_void(base->generate_texture(resource));
			}
			bool graphics_device_generate_texture2(graphics::graphics_device* base, graphics::texture_3d* resource)
			{
				return expects_wrapper::unwrap_void(base->generate_texture(resource));
			}
			bool graphics_device_generate_texture3(graphics::graphics_device* base, graphics::texture_cube* resource)
			{
				return expects_wrapper::unwrap_void(base->generate_texture(resource));
			}
			bool graphics_device_get_query_data1(graphics::graphics_device* base, graphics::query* resource, size_t& result, bool flush)
			{
				return expects_wrapper::unwrap_void(base->get_query_data(resource, &result, flush));
			}
			bool graphics_device_get_query_data2(graphics::graphics_device* base, graphics::query* resource, bool& result, bool flush)
			{
				return expects_wrapper::unwrap_void(base->get_query_data(resource, &result, flush));
			}
			graphics::mesh_buffer* graphics_device_create_mesh_buffer1(graphics::graphics_device* base, const graphics::mesh_buffer::desc& desc)
			{
				auto* result = expects_wrapper::unwrap(base->create_mesh_buffer(desc), (graphics::mesh_buffer*)nullptr);
				if (result != nullptr)
					function_factory::atomic_notify_gc(TYPENAME_MESHBUFFER, result);
				return result;
			}
			graphics::mesh_buffer* graphics_device_create_mesh_buffer2(graphics::graphics_device* base, graphics::element_buffer* vertex_buffer, graphics::element_buffer* index_buffer)
			{
				auto* result = expects_wrapper::unwrap(base->create_mesh_buffer(vertex_buffer, index_buffer), (graphics::mesh_buffer*)nullptr);
				if (!result)
					return nullptr;

				if (vertex_buffer != nullptr)
					vertex_buffer->add_ref();

				if (index_buffer != nullptr)
					index_buffer->add_ref();

				function_factory::atomic_notify_gc(TYPENAME_MESHBUFFER, result);
				return result;
			}
			graphics::skin_mesh_buffer* graphics_device_create_skin_mesh_buffer1(graphics::graphics_device* base, const graphics::skin_mesh_buffer::desc& desc)
			{
				auto* result = expects_wrapper::unwrap(base->create_skin_mesh_buffer(desc), (graphics::skin_mesh_buffer*)nullptr);
				if (result != nullptr)
					function_factory::atomic_notify_gc(TYPENAME_SKINMESHBUFFER, result);
				return result;
			}
			graphics::skin_mesh_buffer* graphics_device_create_skin_mesh_buffer2(graphics::graphics_device* base, graphics::element_buffer* vertex_buffer, graphics::element_buffer* index_buffer)
			{
				auto* result = expects_wrapper::unwrap(base->create_skin_mesh_buffer(vertex_buffer, index_buffer), (graphics::skin_mesh_buffer*)nullptr);
				if (!result)
					return nullptr;

				if (vertex_buffer != nullptr)
					vertex_buffer->add_ref();

				if (index_buffer != nullptr)
					index_buffer->add_ref();

				function_factory::atomic_notify_gc(TYPENAME_SKINMESHBUFFER, result);
				return result;
			}
			graphics::instance_buffer* graphics_device_create_instance_buffer(graphics::graphics_device* base, const graphics::instance_buffer::desc& desc)
			{
				auto* result = expects_wrapper::unwrap(base->create_instance_buffer(desc), (graphics::instance_buffer*)nullptr);
				if (result != nullptr)
					function_factory::atomic_notify_gc(TYPENAME_INSTANCEBUFFER, result);
				return result;
			}
			graphics::texture_2d* graphics_device_create_texture_2d1(graphics::graphics_device* base)
			{
				return expects_wrapper::unwrap(base->create_texture_2d(), (graphics::texture_2d*)nullptr);
			}
			graphics::texture_2d* graphics_device_create_texture_2d2(graphics::graphics_device* base, const graphics::texture_2d::desc& desc)
			{
				return expects_wrapper::unwrap(base->create_texture_2d(desc), (graphics::texture_2d*)nullptr);
			}
			graphics::texture_3d* graphics_device_create_texture_3d1(graphics::graphics_device* base)
			{
				return expects_wrapper::unwrap(base->create_texture_3d(), (graphics::texture_3d*)nullptr);
			}
			graphics::texture_3d* graphics_device_create_texture_3d2(graphics::graphics_device* base, const graphics::texture_3d::desc& desc)
			{
				return expects_wrapper::unwrap(base->create_texture_3d(desc), (graphics::texture_3d*)nullptr);
			}
			graphics::texture_cube* graphics_device_create_texture_cube1(graphics::graphics_device* base)
			{
				return expects_wrapper::unwrap(base->create_texture_cube(), (graphics::texture_cube*)nullptr);
			}
			graphics::texture_cube* graphics_device_create_texture_cube2(graphics::graphics_device* base, const graphics::texture_cube::desc& desc)
			{
				return expects_wrapper::unwrap(base->create_texture_cube(desc), (graphics::texture_cube*)nullptr);
			}
			graphics::texture_cube* graphics_device_create_texture_cube3(graphics::graphics_device* base, array* data)
			{
				core::vector<graphics::texture_2d*> buffer = array::decompose<graphics::texture_2d*>(data);
				while (buffer.size() < 6)
					buffer.push_back(nullptr);

				graphics::texture_2d* map[6];
				for (size_t i = 0; i < 6; i++)
					map[i] = buffer[i];

				return expects_wrapper::unwrap(base->create_texture_cube(map), (graphics::texture_cube*)nullptr);
			}
			graphics::texture_cube* graphics_device_create_texture_cube4(graphics::graphics_device* base, graphics::texture_2d* data)
			{
				return expects_wrapper::unwrap(base->create_texture_cube(data), (graphics::texture_cube*)nullptr);
			}
			graphics::texture_2d* graphics_device_copy_texture_2d1(graphics::graphics_device* base, graphics::texture_2d* source)
			{
				graphics::texture_2d* result = nullptr;
				expects_wrapper::unwrap_void(base->copy_texture_2d(source, &result));
				return result;
			}
			graphics::texture_2d* graphics_device_copy_texture_2d2(graphics::graphics_device* base, graphics::render_target* source, uint32_t index)
			{
				graphics::texture_2d* result = nullptr;
				expects_wrapper::unwrap_void(base->copy_texture_2d(source, index, &result));
				return result;
			}
			graphics::texture_2d* graphics_device_copy_texture_2d3(graphics::graphics_device* base, graphics::render_target_cube* source, trigonometry::cube_face face)
			{
				graphics::texture_2d* result = nullptr;
				expects_wrapper::unwrap_void(base->copy_texture_2d(source, face, &result));
				return result;
			}
			graphics::texture_2d* graphics_device_copy_texture_2d4(graphics::graphics_device* base, graphics::multi_render_target_cube* source, uint32_t index, trigonometry::cube_face face)
			{
				graphics::texture_2d* result = nullptr;
				expects_wrapper::unwrap_void(base->copy_texture_2d(source, index, face, &result));
				return result;
			}
			graphics::texture_cube* graphics_device_copy_texture_cube1(graphics::graphics_device* base, graphics::render_target_cube* source)
			{
				graphics::texture_cube* result = nullptr;
				expects_wrapper::unwrap_void(base->copy_texture_cube(source, &result));
				return result;
			}
			graphics::texture_cube* graphics_device_copy_texture_cube2(graphics::graphics_device* base, graphics::multi_render_target_cube* source, uint32_t index)
			{
				graphics::texture_cube* result = nullptr;
				expects_wrapper::unwrap_void(base->copy_texture_cube(source, index, &result));
				return result;
			}
			graphics::texture_2d* graphics_device_copy_back_buffer(graphics::graphics_device* base)
			{
				graphics::texture_2d* result = nullptr;
				expects_wrapper::unwrap_void(base->copy_back_buffer(&result));
				return result;
			}
			graphics::graphics_device* graphics_device_create(graphics::graphics_device::desc& desc)
			{
				auto* result = graphics::graphics_device::create(desc);
				function_factory::atomic_notify_gc(TYPENAME_GRAPHICSDEVICE, result);
				return result;
			}
			void graphics_device_compile_builtin_shaders(array* devices)
			{
				expects_wrapper::unwrap_void(graphics::graphics_device::compile_builtin_shaders(array::decompose<graphics::graphics_device*>(devices), nullptr));
			}

			trigonometry::matrix4x4& animation_buffer_get_offsets(layer::animation_buffer& base, size_t index)
			{
				return base.offsets[index % graphics::joints_size];
			}

			void pose_buffer_set_offset(layer::pose_buffer& base, int64_t index, const layer::pose_data& data)
			{
				base.offsets[index] = data;
			}
			void pose_buffer_set_matrix(layer::pose_buffer& base, graphics::skin_mesh_buffer* mesh, size_t index, const trigonometry::matrix4x4& data)
			{
				base.matrices[mesh].data[index % graphics::joints_size] = data;
			}
			layer::pose_data& pose_buffer_get_offset(layer::pose_buffer& base, int64_t index)
			{
				return base.offsets[index];
			}
			trigonometry::matrix4x4& pose_buffer_get_matrix(layer::pose_buffer& base, graphics::skin_mesh_buffer* mesh, size_t index)
			{
				return base.matrices[mesh].data[index % graphics::joints_size];
			}
			size_t pose_buffer_get_offsets_size(layer::pose_buffer& base)
			{
				return base.offsets.size();
			}
			size_t pose_buffer_get_matrices_size(layer::pose_buffer& base, graphics::skin_mesh_buffer* mesh)
			{
				return graphics::joints_size;
			}

			array* model_get_meshes(layer::model* base)
			{
				typeinfo type = virtual_machine::get()->get_type_info_by_decl(TYPENAME_ARRAY "<" TYPENAME_MESHBUFFER "@>@");
				return array::compose(type.get_type_info(), base->meshes);
			}
			void model_set_meshes(layer::model* base, array* data)
			{
				base->meshes = array::decompose<graphics::mesh_buffer*>(data);
			}
			array* skin_model_get_meshes(layer::skin_model* base)
			{
				typeinfo type = virtual_machine::get()->get_type_info_by_decl(TYPENAME_ARRAY "<" TYPENAME_SKINMESHBUFFER "@>@");
				return array::compose(type.get_type_info(), base->meshes);
			}
			void skin_model_set_meshes(layer::skin_model* base, array* data)
			{
				base->meshes = array::decompose<graphics::skin_mesh_buffer*>(data);
			}

			template <typename t>
			void populate_audio_filter_base(ref_class& data_class, bool base_cast = true)
			{
				if (base_cast)
					data_class.set_operator_ex(operators::cast, 0, "void", "?&out", &handle_to_handle_cast);

				data_class.set_method_ex("bool synchronize()", &VI_EXPECTIFY_VOID(t::synchronize));
				data_class.set_method("void deserialize(schema@+)", &t::deserialize);
				data_class.set_method("void serialize(schema@+)", &t::serialize);
				data_class.set_method("base_audio_filter@ copy()", &t::copy);
				data_class.set_method("audio_source@+ get_source()", &t::get_source);
				populate_component<t>(data_class);
			}
			template <typename t, typename... args>
			void populate_audio_filter_interface(ref_class& data_class, const char* Constructor)
			{
				data_class.set_constructor<t, args...>(Constructor);
				data_class.set_dynamic_cast<t, audio::audio_filter>("base_audio_filter@+", true);
				populate_audio_filter_base<t>(data_class, false);
			}

			template <typename t>
			void populate_audio_effect_base(ref_class& data_class, bool base_cast = true)
			{
				if (base_cast)
					data_class.set_operator_ex(operators::cast, 0, "void", "?&out", &handle_to_handle_cast);

				data_class.set_method_ex("bool synchronize()", &VI_EXPECTIFY_VOID(t::synchronize));
				data_class.set_method("void deserialize(schema@+)", &t::deserialize);
				data_class.set_method("void serialize(schema@+)", &t::serialize);
				data_class.set_method_ex("void set_filter(base_audio_filter@+)", &audio_effect_set_filter);
				data_class.set_method("base_audio_effect@ copy()", &t::copy);
				data_class.set_method("audio_source@+ get_filter()", &t::get_filter);
				data_class.set_method("audio_source@+ get_source()", &t::get_source);
				populate_component<t>(data_class);
			}
			template <typename t, typename... args>
			void populate_audio_effect_interface(ref_class& data_class, const char* Constructor)
			{
				data_class.set_constructor<t, args...>(Constructor);
				data_class.set_dynamic_cast<t, audio::audio_effect>("base_audio_effect@+", true);
				populate_audio_effect_base<t>(data_class, false);
			}

			void event_set_args(layer::event& base, dictionary* data)
			{
				typeinfo type = virtual_machine::get()->get_type_info_by_decl(TYPENAME_DICTIONARY "@");
				base.args = dictionary::decompose<core::variant>(type.get_type_id(), data);
			}
			dictionary* event_get_args(layer::event& base)
			{
				typeinfo type = virtual_machine::get()->get_type_info_by_decl(TYPENAME_DICTIONARY "@");
				return dictionary::compose<core::variant>(type.get_type_id(), base.args);
			}

			layer::viewer& viewer_copy(layer::viewer& base, layer::viewer& other)
			{
				if (&base == &other)
					return base;

				core::memory::release(base.renderer);
				base = other;
				if (base.renderer != nullptr)
					base.renderer->add_ref();

				return base;
			}
			void viewer_destructor(layer::viewer& base)
			{
				core::memory::release(base.renderer);
			}

			array* skin_animation_get_clips(layer::skin_animation* base)
			{
				typeinfo type = virtual_machine::get()->get_type_info_by_decl(TYPENAME_ARRAY "<" TYPENAME_SKINANIMATORCLIP ">@");
				return array::compose<trigonometry::skin_animator_clip>(type.get_type_info(), base->get_clips());
			}

			size_t sparse_index_get_size(layer::sparse_index& base)
			{
				return base.data.size();
			}
			layer::component* sparse_index_get_data(layer::sparse_index& base, size_t index)
			{
				if (index >= base.data.size())
					return nullptr;

				return base.data[index];
			}

			template <typename t>
			void component_message(t* base, const std::string_view& name, core::schema* args)
			{
				auto data = to_variant_keys(args);
				return base->message(name, data);
			}
			template <typename t>
			void populate_component_base(ref_class& data_class, bool base_cast = true)
			{
				if (base_cast)
					data_class.set_operator_ex(operators::cast, 0, "void", "?&out", &handle_to_handle_cast);

				data_class.set_method("void serialize(schema@+)", &layer::component::serialize);
				data_class.set_method("void deserialize(schema@+)", &layer::component::deserialize);
				data_class.set_method("void activate(base_component@+)", &layer::component::activate);
				data_class.set_method("void deactivate()", &layer::component::deactivate);
				data_class.set_method("void synchronize(clock_timer@+)", &layer::component::synchronize);
				data_class.set_method("void animate(clock_timer@+)", &layer::component::animate);
				data_class.set_method("void update(clock_timer@+)", &layer::component::update);
				data_class.set_method_ex("void message(const string_view&in, schema@+)", &component_message<t>);
				data_class.set_method("void movement()", &layer::component::movement);
				data_class.set_method("usize get_unit_bounds(vector3 &out, vector3 &out) const", &layer::component::get_unit_bounds);
				data_class.set_method("float get_visibility(const viewer_t &in, float) const", &layer::component::get_visibility);
				data_class.set_method("base_component@+ copy(scene_entity@+) const", &layer::component::copy);
				data_class.set_method("scene_entity@+ get_entity() const", &layer::component::get_entity);
				data_class.set_method("void set_active(bool)", &layer::component::set_active);
				data_class.set_method("bool is_drawable() const", &layer::component::is_drawable);
				data_class.set_method("bool is_cullable() const", &layer::component::is_cullable);
				data_class.set_method("bool is_active() const", &layer::component::is_active);
				populate_component<t>(data_class);
			};
			template <typename t, typename... args>
			void populate_component_interface(ref_class& data_class, const char* Constructor)
			{
				data_class.set_constructor<t, args...>(Constructor);
				data_class.set_dynamic_cast<t, layer::component>("base_component@+", true);
				populate_component_base<t>(data_class, false);
			};

			void entity_remove_component(layer::entity* base, layer::component* source)
			{
				if (source != nullptr)
					base->remove_component(source->get_id());
			}
			void entity_remove_component_by_id(layer::entity* base, uint64_t id)
			{
				base->remove_component(id);
			}
			layer::component* entity_add_component(layer::entity* base, layer::component* source)
			{
				if (!source)
					return nullptr;

				return base->add_component(source);
			}
			layer::component* entity_get_component_by_id(layer::entity* base, uint64_t id)
			{
				return base->get_component(id);
			}
			array* entity_get_components(layer::entity* base)
			{
				core::vector<layer::component*> components;
				components.reserve(base->get_components_count());

				for (auto& item : *base)
					components.push_back(item.second);

				typeinfo type = virtual_machine::get()->get_type_info_by_decl(TYPENAME_ARRAY "<" TYPENAME_COMPONENT "@>@");
				return array::compose<layer::component*>(type.get_type_info(), components);
			}

			template <typename t>
			void populate_drawable_base(ref_class& data_class)
			{
				data_class.set_property<layer::drawable>("float overlapping", &layer::drawable::overlapping);
				data_class.set_property<layer::drawable>("bool static", &layer::drawable::constant);
				data_class.set_method("void clear_materials()", &layer::drawable::clear_materials);
				data_class.set_method("bool set_category(geo_category)", &layer::drawable::set_category);
				data_class.set_method<layer::drawable, bool, void*, layer::material*>("bool set_material(uptr@, material@+)", &layer::drawable::set_material);
				data_class.set_method<layer::drawable, bool, layer::material*>("bool set_material(material@+)", &layer::drawable::set_material);
				data_class.set_method("geo_category get_category() const", &layer::drawable::get_category);
				data_class.set_method<layer::drawable, int64_t, void*>("int64 get_slot(uptr@)", &layer::drawable::get_slot);
				data_class.set_method<layer::drawable, int64_t>("int64 get_slot()", &layer::drawable::get_slot);
				data_class.set_method<layer::drawable, layer::material*, void*>("material@+ get_material(uptr@)", &layer::drawable::get_material);
				data_class.set_method<layer::drawable, layer::material*>("material@+ get_material()", &layer::drawable::get_material);
				populate_component_base<t>(data_class, false);
			}
			template <typename t, typename... args>
			void populate_drawable_interface(ref_class& data_class, const char* Constructor)
			{
				data_class.set_constructor<t, args...>(Constructor);
				data_class.set_dynamic_cast<t, layer::component>("base_component@+", true);
				populate_drawable_base<t>(data_class);
			}

			template <typename t>
			void populate_renderer_base(ref_class& data_class, bool base_cast = true)
			{
				if (base_cast)
					data_class.set_operator_ex(operators::cast, 0, "void", "?&out", &handle_to_handle_cast);

				data_class.set_property<layer::renderer>("bool active", &layer::renderer::active);
				data_class.set_method("void serialize(schema@+)", &layer::renderer::serialize);
				data_class.set_method("void deserialize(schema@+)", &layer::renderer::deserialize);
				data_class.set_method("void clear_culling()", &layer::renderer::clear_culling);
				data_class.set_method("void resize_buffers()", &layer::renderer::resize_buffers);
				data_class.set_method("void activate()", &layer::renderer::activate);
				data_class.set_method("void deactivate()", &layer::renderer::deactivate);
				data_class.set_method("void begin_pass()", &layer::renderer::begin_pass);
				data_class.set_method("void end_pass()", &layer::renderer::end_pass);
				data_class.set_method("bool has_category(geo_category)", &layer::renderer::has_category);
				data_class.set_method("usize render_pass(clock_timer@+)", &layer::renderer::render_pass);
				data_class.set_method("void set_renderer(render_system@+)", &layer::renderer::set_renderer);
				data_class.set_method("render_system@+ get_renderer() const", &layer::renderer::get_renderer);
				populate_component<t>(data_class);
			}
			template <typename t, typename... args>
			void populate_renderer_interface(ref_class& data_class, const char* Constructor)
			{
				data_class.set_constructor<t, args...>(Constructor);
				data_class.set_dynamic_cast<t, layer::renderer>("base_renderer@+", true);
				populate_renderer_base<t>(data_class, false);
			}

			void render_system_restore_view_buffer(layer::render_system* base)
			{
				base->restore_view_buffer(nullptr);
			}
			void render_system_move_renderer(layer::render_system* base, layer::renderer* source, size_t offset)
			{
				if (source != nullptr)
					base->move_renderer(source->get_id(), offset);
			}
			void render_system_remove_renderer(layer::render_system* base, layer::renderer* source)
			{
				if (source != nullptr)
					base->remove_renderer(source->get_id());
			}
			void render_system_free_buffers1(layer::render_system* base, const std::string_view& name, graphics::element_buffer* buffer1, graphics::element_buffer* buffer2)
			{
				graphics::element_buffer* buffers[2];
				buffers[0] = buffer1;
				buffers[1] = buffer2;
				base->free_buffers(name, buffers);
			}
			void render_system_free_buffers2(layer::render_system* base, const std::string_view& name, graphics::element_buffer* buffer1, graphics::element_buffer* buffer2)
			{
				graphics::element_buffer* buffers[2];
				buffers[0] = buffer1;
				buffers[1] = buffer2;
				base->free_buffers(buffers);
			}
			graphics::shader* render_system_compile_shader1(layer::render_system* base, graphics::shader::desc& desc, size_t buffer_size)
			{
				return expects_wrapper::unwrap(base->compile_shader(desc, buffer_size), (graphics::shader*)nullptr);
			}
			graphics::shader* render_system_compile_shader2(layer::render_system* base, const std::string_view& section_name, array* features, size_t buffer_size)
			{
				return expects_wrapper::unwrap(base->compile_shader(section_name, array::decompose<core::string>(features), buffer_size), (graphics::shader*)nullptr);
			}
			array* render_system_compile_buffers(layer::render_system* base, const std::string_view& name, size_t element_size, size_t elements_count)
			{
				core::vector<graphics::element_buffer*> buffers;
				buffers.push_back(nullptr);
				buffers.push_back(nullptr);

				if (!expects_wrapper::unwrap_void(base->compile_buffers(buffers.data(), name, element_size, elements_count)))
					return nullptr;

				typeinfo type = virtual_machine::get()->get_type_info_by_decl(TYPENAME_ARRAY "<" TYPENAME_ELEMENTBUFFER "@>@");
				return array::compose(type.get_type_info(), buffers);
			}
			bool render_system_add_renderer(layer::render_system* base, layer::renderer* source)
			{
				if (!source)
					return false;

				return base->add_renderer(source) != nullptr;
			}
			layer::renderer* render_system_get_renderer(layer::render_system* base, uint64_t id)
			{
				return base->get_renderer(id);
			}
			layer::renderer* render_system_get_renderer_by_index(layer::render_system* base, size_t index)
			{
				auto& data = base->get_renderers();
				if (index >= data.size())
					return nullptr;

				return data[index];
			}
			size_t render_system_get_renderers_count(layer::render_system* base)
			{
				return base->get_renderers().size();
			}
			void render_system_query_group(layer::render_system* base, uint64_t id, asIScriptFunction* callback)
			{
				immediate_context* context = immediate_context::get();
				function_delegate delegatef(callback);
				if (!context || !delegatef.is_valid())
					return;

				base->query_group(id, [context, delegatef](layer::component* item)
				{
					context->execute_subcall(delegatef.callable(), [item](immediate_context* context)
					{
						context->set_arg_object(0, (void*)item);
					});
				});
			}

			array* primitive_cache_compile(layer::primitive_cache* base, const std::string_view& name, size_t element_size, size_t elements_count)
			{
				core::vector<graphics::element_buffer*> buffers;
				buffers.push_back(nullptr);
				buffers.push_back(nullptr);

				if (!expects_wrapper::unwrap_void(base->compile(buffers.data(), name, element_size, elements_count)))
					return nullptr;

				typeinfo type = virtual_machine::get()->get_type_info_by_decl(TYPENAME_ARRAY "<" TYPENAME_ELEMENTBUFFER "@>@");
				return array::compose(type.get_type_info(), buffers);
			}
			array* primitive_cache_get(layer::primitive_cache* base, const std::string_view& name)
			{
				core::vector<graphics::element_buffer*> buffers;
				buffers.push_back(nullptr);
				buffers.push_back(nullptr);

				if (!base->get(buffers.data(), name))
					return nullptr;

				typeinfo type = virtual_machine::get()->get_type_info_by_decl(TYPENAME_ARRAY "<" TYPENAME_ELEMENTBUFFER "@>@");
				return array::compose(type.get_type_info(), buffers);
			}
			bool primitive_cache_free(layer::primitive_cache* base, const std::string_view& name, graphics::element_buffer* buffer1, graphics::element_buffer* buffer2)
			{
				graphics::element_buffer* buffers[2];
				buffers[0] = buffer1;
				buffers[1] = buffer2;
				return base->free(name, buffers);
			}
			core::string primitive_cache_find(layer::primitive_cache* base, graphics::element_buffer* buffer1, graphics::element_buffer* buffer2)
			{
				graphics::element_buffer* buffers[2];
				buffers[0] = buffer1;
				buffers[1] = buffer2;
				return base->find(buffers);
			}
			array* primitive_cache_get_sphere_buffers(layer::primitive_cache* base)
			{
				core::vector<graphics::element_buffer*> buffers;
				buffers.push_back(nullptr);
				buffers.push_back(nullptr);
				base->get_sphere_buffers(buffers.data());

				typeinfo type = virtual_machine::get()->get_type_info_by_decl(TYPENAME_ARRAY "<" TYPENAME_ELEMENTBUFFER "@>@");
				return array::compose(type.get_type_info(), buffers);
			}
			array* primitive_cache_get_cube_buffers(layer::primitive_cache* base)
			{
				core::vector<graphics::element_buffer*> buffers;
				buffers.push_back(nullptr);
				buffers.push_back(nullptr);
				base->get_cube_buffers(buffers.data());

				typeinfo type = virtual_machine::get()->get_type_info_by_decl(TYPENAME_ARRAY "<" TYPENAME_ELEMENTBUFFER "@>@");
				return array::compose(type.get_type_info(), buffers);
			}
			array* primitive_cache_get_box_buffers(layer::primitive_cache* base)
			{
				core::vector<graphics::element_buffer*> buffers;
				buffers.push_back(nullptr);
				buffers.push_back(nullptr);
				base->get_box_buffers(buffers.data());

				typeinfo type = virtual_machine::get()->get_type_info_by_decl(TYPENAME_ARRAY "<" TYPENAME_ELEMENTBUFFER "@>@");
				return array::compose(type.get_type_info(), buffers);
			}
			array* primitive_cache_get_skin_box_buffers(layer::primitive_cache* base)
			{
				core::vector<graphics::element_buffer*> buffers;
				buffers.push_back(nullptr);
				buffers.push_back(nullptr);
				base->get_skin_box_buffers(buffers.data());

				typeinfo type = virtual_machine::get()->get_type_info_by_decl(TYPENAME_ARRAY "<" TYPENAME_ELEMENTBUFFER "@>@");
				return array::compose(type.get_type_info(), buffers);
			}

			layer::scene_graph::desc::dependencies& scene_graph_shared_desc_copy(layer::scene_graph::desc::dependencies& base, layer::scene_graph::desc::dependencies& other)
			{
				if (&base == &other)
					return base;

				core::memory::release(base.device);
				core::memory::release(base.activity);
				core::memory::release(base.vm);
				core::memory::release(base.content);
				core::memory::release(base.primitives);
				core::memory::release(base.shaders);
				base = other;
				if (base.device != nullptr)
					base.device->add_ref();
				if (base.activity != nullptr)
					base.activity->add_ref();
				if (base.vm != nullptr)
					base.vm->add_ref();
				if (base.content != nullptr)
					base.content->add_ref();
				if (base.primitives != nullptr)
					base.primitives->add_ref();
				if (base.shaders != nullptr)
					base.shaders->add_ref();

				return base;
			}
			void scene_graph_shared_desc_destructor(layer::scene_graph::desc::dependencies& base)
			{
				core::memory::release(base.device);
				core::memory::release(base.activity);
				core::memory::release(base.vm);
				core::memory::release(base.content);
				core::memory::release(base.primitives);
				core::memory::release(base.shaders);
			}

			void scene_graph_ray_test(layer::scene_graph* base, uint64_t id, const trigonometry::ray& ray, asIScriptFunction* callback)
			{
				immediate_context* context = immediate_context::get();
				function_delegate delegatef(callback);
				if (!context || !delegatef.is_valid())
					return;

				base->ray_test(id, ray, [context, delegatef](layer::component* source, const trigonometry::vector3& where)
				{
					bool result = false;
					context->execute_subcall(delegatef.callable(), [&source, &where](immediate_context* context)
					{
						context->set_arg_object(0, (void*)source);
						context->set_arg_object(1, (void*)&where);
					}, [&result](immediate_context* context)
					{
						result = (bool)context->get_return_byte();
					});
					return result;
				});
			}
			void scene_graph_mutate1(layer::scene_graph* base, layer::entity* source, layer::entity* child, const std::string_view& name)
			{
				base->mutate(source, child, name);
			}
			void scene_graph_mutate2(layer::scene_graph* base, layer::entity* source, const std::string_view& name)
			{
				base->mutate(source, name);
			}
			void scene_graph_mutate3(layer::scene_graph* base, layer::component* source, const std::string_view& name)
			{
				base->mutate(source, name);
			}
			void scene_graph_mutate4(layer::scene_graph* base, layer::material* source, const std::string_view& name)
			{
				base->mutate(source, name);
			}
			void scene_graph_transaction(layer::scene_graph* base, asIScriptFunction* callback)
			{
				function_delegate delegatef(callback);
				if (!delegatef.is_valid())
					return;

				base->transaction([delegatef]() mutable
				{
					delegatef(nullptr).wait();
				});
			}
			void scene_graph_push_event1(layer::scene_graph* base, const std::string_view& name, core::schema* args, bool propagate)
			{
				base->push_event(name, to_variant_keys(args), propagate);
			}
			void scene_graph_push_event2(layer::scene_graph* base, const std::string_view& name, core::schema* args, layer::component* source)
			{
				base->push_event(name, to_variant_keys(args), source);
			}
			void scene_graph_push_event3(layer::scene_graph* base, const std::string_view& name, core::schema* args, layer::entity* source)
			{
				base->push_event(name, to_variant_keys(args), source);
			}
			void* scene_graph_set_listener(layer::scene_graph* base, const std::string_view& name, asIScriptFunction* callback)
			{
				function_delegate delegatef(callback);
				if (!delegatef.is_valid())
					return nullptr;

				return base->set_listener(name, [delegatef](const std::string_view& name, core::variant_args& base_args) mutable
				{
					core::schema* args = core::var::set::object();
					args->reserve(base_args.size());
					for (auto& item : base_args)
						args->set(item.first, item.second);

					core::string copy = core::string(name);
					delegatef([copy, args](immediate_context* context)
					{
						context->set_arg_object(0, (void*)&copy);
						context->set_arg_object(1, (void*)args);
					}).when([args](expects_vm<execution>&&) { args->release(); });
				});
			}
			void scene_graph_load_resource(layer::scene_graph* base, uint64_t id, layer::component* source, const std::string_view& path, core::schema* args, asIScriptFunction* callback)
			{
				function_delegate delegatef(callback);
				if (!delegatef.is_valid())
					return;

				base->load_resource(id, source, path, to_variant_keys(args), [delegatef](layer::expects_content<void*>&& resource) mutable
				{
					void* address = resource.or_else(nullptr);
					delegatef([address](immediate_context* context)
					{
						context->set_arg_address(0, address);
					});
				});
			}
			array* scene_graph_get_components(layer::scene_graph* base, uint64_t id)
			{
				auto& data = base->get_components(id);
				core::vector<layer::component*> output;
				output.reserve(data.size());

				for (auto* next : data)
					output.push_back(next);

				typeinfo type = virtual_machine::get()->get_type_info_by_decl(TYPENAME_ARRAY "<" TYPENAME_COMPONENT "@>@");
				return array::compose(type.get_type_info(), output);
			}
			array* scene_graph_get_actors(layer::scene_graph* base, layer::actor_type source)
			{
				auto& data = base->get_actors(source);
				core::vector<layer::component*> output;
				output.reserve(data.size());

				for (auto* next : data)
					output.push_back(next);

				typeinfo type = virtual_machine::get()->get_type_info_by_decl(TYPENAME_ARRAY "<" TYPENAME_COMPONENT "@>@");
				return array::compose(type.get_type_info(), output);
			}
			array* scene_graph_clone_entity_as_array(layer::scene_graph* base, layer::entity* source)
			{
				typeinfo type = virtual_machine::get()->get_type_info_by_decl(TYPENAME_ARRAY "<" TYPENAME_ENTITY "@>@");
				return array::compose(type.get_type_info(), base->clone_entity_as_array(source));
			}
			array* scene_graph_query_by_parent(layer::scene_graph* base, layer::entity* source)
			{
				typeinfo type = virtual_machine::get()->get_type_info_by_decl(TYPENAME_ARRAY "<" TYPENAME_ENTITY "@>@");
				return array::compose(type.get_type_info(), base->query_by_parent(source));
			}
			array* scene_graph_query_by_name(layer::scene_graph* base, const std::string_view& source)
			{
				typeinfo type = virtual_machine::get()->get_type_info_by_decl(TYPENAME_ARRAY "<" TYPENAME_ENTITY "@>@");
				return array::compose(type.get_type_info(), base->query_by_name(source));
			}
			array* scene_graph_query_by_position(layer::scene_graph* base, uint64_t id, const trigonometry::vector3& position, float radius)
			{
				typeinfo type = virtual_machine::get()->get_type_info_by_decl(TYPENAME_ARRAY "<" TYPENAME_COMPONENT "@>@");
				return array::compose(type.get_type_info(), base->query_by_position(id, position, radius));
			}
			array* scene_graph_query_by_area(layer::scene_graph* base, uint64_t id, const trigonometry::vector3& min, const trigonometry::vector3& max)
			{
				typeinfo type = virtual_machine::get()->get_type_info_by_decl(TYPENAME_ARRAY "<" TYPENAME_COMPONENT "@>@");
				return array::compose(type.get_type_info(), base->query_by_area(id, min, max));
			}
			array* scene_graph_query_by_ray(layer::scene_graph* base, uint64_t id, const trigonometry::ray& ray)
			{
				typeinfo type = virtual_machine::get()->get_type_info_by_decl(TYPENAME_ARRAY "<" TYPENAME_COMPONENT "@>@");
				return array::compose(type.get_type_info(), base->query_by_ray(id, ray));
			}
			array* scene_graph_query_by_match(layer::scene_graph* base, uint64_t id, asIScriptFunction* callback)
			{
				immediate_context* context = immediate_context::get();
				function_delegate delegatef(callback);
				if (!context || !delegatef.is_valid())
					return nullptr;

				typeinfo type = virtual_machine::get()->get_type_info_by_decl(TYPENAME_ARRAY "<" TYPENAME_COMPONENT "@>@");
				return array::compose(type.get_type_info(), base->query_by_match(id, [context, delegatef](const trigonometry::bounding& source)
				{
					bool result = false;
					context->execute_subcall(delegatef.callable(), [&source](immediate_context* context)
					{
						context->set_arg_object(0, (void*)&source);
					}, [&result](immediate_context* context)
					{
						result = (bool)context->get_return_byte();
					});
					return result;
				}));
			}

			heavy_application::cache_info& application_cache_info_copy(heavy_application::cache_info& base, heavy_application::cache_info& other)
			{
				if (&base == &other)
					return base;

				core::memory::release(base.primitives);
				core::memory::release(base.shaders);
				base = other;
				if (base.primitives != nullptr)
					base.primitives->add_ref();
				if (base.shaders != nullptr)
					base.shaders->add_ref();

				return base;
			}
			void application_cache_info_destructor(heavy_application::cache_info& base)
			{
				core::memory::release(base.primitives);
				core::memory::release(base.shaders);
			}

			bool ielement_dispatch_event(layer::gui::ielement& base, const std::string_view& name, core::schema* args)
			{
				core::variant_args data;
				if (args != nullptr)
				{
					data.reserve(args->size());
					for (auto item : args->get_childs())
						data[item->key] = item->value;
				}

				return base.dispatch_event(name, data);
			}
			array* ielement_query_selector_all(layer::gui::ielement& base, const std::string_view& value)
			{
				typeinfo type = virtual_machine::get()->get_type_info_by_decl(TYPENAME_ARRAY "<" TYPENAME_ELEMENTNODE ">@");
				return array::compose(type.get_type_info(), base.query_selector_all(value));
			}

			bool ielement_document_dispatch_event(layer::gui::ielement_document& base, const std::string_view& name, core::schema* args)
			{
				core::variant_args data;
				if (args != nullptr)
				{
					data.reserve(args->size());
					for (auto item : args->get_childs())
						data[item->key] = item->value;
				}

				return base.dispatch_event(name, data);
			}
			array* ielement_document_query_selector_all(layer::gui::ielement_document& base, const std::string_view& value)
			{
				typeinfo type = virtual_machine::get()->get_type_info_by_decl(TYPENAME_ARRAY "<" TYPENAME_ELEMENTNODE ">@");
				return array::compose(type.get_type_info(), base.query_selector_all(value));
			}

			bool data_model_set_recursive(layer::gui::data_node* node, core::schema* data, size_t depth)
			{
				size_t index = 0;
				for (auto& item : data->get_childs())
				{
					auto& child = node->add(item->value);
					child.set_top((void*)(uintptr_t)index++, depth);
					if (item->value.is_object())
						data_model_set_recursive(&child, item, depth + 1);
				}

				node->sort_tree();
				return true;
			}
			bool data_model_get_recursive(layer::gui::data_node* node, core::schema* data)
			{
				size_t size = node->size();
				for (size_t i = 0; i < size; i++)
				{
					auto& child = node->at(i);
					data_model_get_recursive(&child, data->push(child.get()));
				}

				return true;
			}
			bool data_model_set(layer::gui::data_model* base, const std::string_view& name, core::schema* data)
			{
				if (!data->value.is_object())
					return base->set_property(name, data->value) != nullptr;

				layer::gui::data_node* node = base->set_array(name);
				if (!node)
					return false;

				return data_model_set_recursive(node, data, 0);
			}
			bool data_model_set_var(layer::gui::data_model* base, const std::string_view& name, const core::variant& data)
			{
				return base->set_property(name, data) != nullptr;
			}
			bool data_model_set_string(layer::gui::data_model* base, const std::string_view& name, const std::string_view& value)
			{
				return base->set_string(name, value) != nullptr;
			}
			bool data_model_set_integer(layer::gui::data_model* base, const std::string_view& name, int64_t value)
			{
				return base->set_integer(name, value) != nullptr;
			}
			bool data_model_set_float(layer::gui::data_model* base, const std::string_view& name, float value)
			{
				return base->set_float(name, value) != nullptr;
			}
			bool data_model_set_double(layer::gui::data_model* base, const std::string_view& name, double value)
			{
				return base->set_double(name, value) != nullptr;
			}
			bool data_model_set_boolean(layer::gui::data_model* base, const std::string_view& name, bool value)
			{
				return base->set_boolean(name, value) != nullptr;
			}
			bool data_model_set_pointer(layer::gui::data_model* base, const std::string_view& name, void* value)
			{
				return base->set_pointer(name, value) != nullptr;
			}
			bool data_model_set_callback(layer::gui::data_model* base, const std::string_view& name, asIScriptFunction* callback)
			{
				function_delegate delegatef(callback);
				if (!delegatef.is_valid())
					return false;

				typeinfo type = virtual_machine::get()->get_type_info_by_decl(TYPENAME_ARRAY "<" TYPENAME_VARIANT ">@");
				return base->set_callback(name, [type, delegatef](layer::gui::ievent& wrapper, const core::variant_list& args) mutable
				{
					layer::gui::ievent event = wrapper.copy();
					array* data = array::compose(type.get_type_info(), args);
					delegatef([event, &data](immediate_context* context) mutable
					{
						context->set_arg_object(0, &event);
						context->set_arg_object(1, &data);
					}).when([event](expects_vm<execution>&&) mutable
					{
						event.release();
					});
				});
			}
			core::schema* data_model_get(layer::gui::data_model* base, const std::string_view& name)
			{
				layer::gui::data_node* node = base->get_property(name);
				if (!node)
					return nullptr;

				core::schema* result = new core::schema(node->get());
				if (result->value.is_object())
					data_model_get_recursive(node, result);

				return result;
			}

			layer::gui::ielement context_get_focus_element(layer::gui::context* base, const trigonometry::vector2& value)
			{
				return base->get_element_at_point(value);
			}
			void context_emit_input(layer::gui::context* base, const std::string_view& data)
			{
				base->emit_input(data.data(), (int)data.size());
			}

			model_listener::model_listener(asIScriptFunction* new_callback) noexcept : delegatef(), base(new layer::gui::listener(bind(new_callback)))
			{
			}
			model_listener::model_listener(const std::string_view& function_name) noexcept : delegatef(), base(new layer::gui::listener(function_name))
			{
			}
			model_listener::~model_listener() noexcept
			{
				core::memory::release(base);
			}
			function_delegate& model_listener::get_delegate()
			{
				return delegatef;
			}
			layer::gui::event_callback model_listener::bind(asIScriptFunction* callback)
			{
				delegatef = function_delegate(callback);
				if (!delegatef.is_valid())
					return nullptr;

				model_listener* listener = this;
				return [listener](layer::gui::ievent& wrapper)
				{
					layer::gui::ievent event = wrapper.copy();
					listener->delegatef([event](immediate_context* context) mutable
					{
						context->set_arg_object(0, &event);
					}).when([event](expects_vm<execution>&&) mutable
					{
						event.release();
					});
				};
			}

			void components_soft_body_load(layer::components::soft_body* base, const std::string_view& path, float ant, asIScriptFunction* callback)
			{
				function_delegate delegatef(callback);
				if (!delegatef.is_valid())
					return base->load(path, ant);

				base->load(path, ant, [delegatef]() mutable
				{
					delegatef(nullptr);
				});
			}

			void components_rigid_body_load(layer::components::rigid_body* base, const std::string_view& path, float mass, float ant, asIScriptFunction* callback)
			{
				function_delegate delegatef(callback);
				if (!delegatef.is_valid())
					return base->load(path, mass, ant);

				base->load(path, mass, ant, [delegatef]() mutable
				{
					delegatef(nullptr);
				});
			}

			void components_key_animator_load_animation(layer::components::key_animator* base, const std::string_view& path, asIScriptFunction* callback)
			{
				function_delegate delegatef(callback);
				if (!delegatef.is_valid())
					return base->load_animation(path);

				base->load_animation(path, [delegatef](bool) mutable
				{
					delegatef(nullptr);
				});
			}
#endif
			bool heavy_registry::bind_addons(virtual_machine* vm) noexcept
			{
				if (!registry::bind_addons(vm))
					return false;

				vm->add_system_addon("trigonometry", { "string" }, &import_trigonometry);
				vm->add_system_addon("physics", { "trigonometry" }, &import_physics);
				vm->add_system_addon("activity", { "string", "trigonometry" }, &import_activity);
				vm->add_system_addon("graphics", { "activity", "trigonometry" }, &import_graphics);
				vm->add_system_addon("audio", { "string", "trigonometry", "schema" }, &import_audio);
				vm->add_system_addon("audio-effects", { "audio" }, &import_audio_effects);
				vm->add_system_addon("audio-filters", { "audio" }, &import_audio_filters);
				vm->add_system_addon("engine", { "layer", "graphics", "audio", "physics", "ui" }, &import_engine);
				vm->add_system_addon("engine-components", { "engine" }, &import_engine_components);
				vm->add_system_addon("engine-renderers", { "engine" }, &import_engine_renderers);
				vm->add_system_addon("ui-control", { "trigonometry", "schema", "array" }, &import_ui_control);
				vm->add_system_addon("ui-model", { "ui-control", }, &import_ui_model);
				vm->add_system_addon("ui", { "ui-model" }, &import_ui);
				return true;
			}
			bool heavy_registry::import_trigonometry(virtual_machine* vm) noexcept
			{
				VI_ASSERT(vm != nullptr && vm->get_engine() != nullptr, "manager should be set");
#ifdef VI_BINDINGS
				auto vvertex = vm->set_pod<trigonometry::vertex>("vertex");
				vvertex->set_property<trigonometry::vertex>("float position_x", &trigonometry::vertex::position_x);
				vvertex->set_property<trigonometry::vertex>("float position_y", &trigonometry::vertex::position_y);
				vvertex->set_property<trigonometry::vertex>("float position_z", &trigonometry::vertex::position_z);
				vvertex->set_property<trigonometry::vertex>("float texcoord_x", &trigonometry::vertex::texcoord_x);
				vvertex->set_property<trigonometry::vertex>("float texcoord_y", &trigonometry::vertex::texcoord_y);
				vvertex->set_property<trigonometry::vertex>("float normal_x", &trigonometry::vertex::normal_x);
				vvertex->set_property<trigonometry::vertex>("float normal_y", &trigonometry::vertex::normal_y);
				vvertex->set_property<trigonometry::vertex>("float normal_z", &trigonometry::vertex::normal_z);
				vvertex->set_property<trigonometry::vertex>("float tangent_x", &trigonometry::vertex::tangent_x);
				vvertex->set_property<trigonometry::vertex>("float tangent_y", &trigonometry::vertex::tangent_y);
				vvertex->set_property<trigonometry::vertex>("float tangent_z", &trigonometry::vertex::tangent_z);
				vvertex->set_property<trigonometry::vertex>("float bitangent_x", &trigonometry::vertex::bitangent_x);
				vvertex->set_property<trigonometry::vertex>("float bitangent_y", &trigonometry::vertex::bitangent_y);
				vvertex->set_property<trigonometry::vertex>("float bitangent_z", &trigonometry::vertex::bitangent_z);
				vvertex->set_constructor<trigonometry::vertex>("void f()");

				auto vskin_vertex = vm->set_pod<trigonometry::skin_vertex>("skin_vertex");
				vskin_vertex->set_property<trigonometry::skin_vertex>("float position_x", &trigonometry::skin_vertex::position_x);
				vskin_vertex->set_property<trigonometry::skin_vertex>("float position_y", &trigonometry::skin_vertex::position_y);
				vskin_vertex->set_property<trigonometry::skin_vertex>("float position_z", &trigonometry::skin_vertex::position_z);
				vskin_vertex->set_property<trigonometry::skin_vertex>("float texcoord_x", &trigonometry::skin_vertex::texcoord_x);
				vskin_vertex->set_property<trigonometry::skin_vertex>("float texcoord_y", &trigonometry::skin_vertex::texcoord_y);
				vskin_vertex->set_property<trigonometry::skin_vertex>("float normal_x", &trigonometry::skin_vertex::normal_x);
				vskin_vertex->set_property<trigonometry::skin_vertex>("float normal_y", &trigonometry::skin_vertex::normal_y);
				vskin_vertex->set_property<trigonometry::skin_vertex>("float normal_z", &trigonometry::skin_vertex::normal_z);
				vskin_vertex->set_property<trigonometry::skin_vertex>("float tangent_x", &trigonometry::skin_vertex::tangent_x);
				vskin_vertex->set_property<trigonometry::skin_vertex>("float tangent_y", &trigonometry::skin_vertex::tangent_y);
				vskin_vertex->set_property<trigonometry::skin_vertex>("float tangent_z", &trigonometry::skin_vertex::tangent_z);
				vskin_vertex->set_property<trigonometry::skin_vertex>("float bitangent_x", &trigonometry::skin_vertex::bitangent_x);
				vskin_vertex->set_property<trigonometry::skin_vertex>("float bitangent_y", &trigonometry::skin_vertex::bitangent_y);
				vskin_vertex->set_property<trigonometry::skin_vertex>("float bitangent_z", &trigonometry::skin_vertex::bitangent_z);
				vskin_vertex->set_property<trigonometry::skin_vertex>("float joint_index0", &trigonometry::skin_vertex::joint_index0);
				vskin_vertex->set_property<trigonometry::skin_vertex>("float joint_index1", &trigonometry::skin_vertex::joint_index1);
				vskin_vertex->set_property<trigonometry::skin_vertex>("float joint_index2", &trigonometry::skin_vertex::joint_index2);
				vskin_vertex->set_property<trigonometry::skin_vertex>("float joint_index3", &trigonometry::skin_vertex::joint_index3);
				vskin_vertex->set_property<trigonometry::skin_vertex>("float joint_bias0", &trigonometry::skin_vertex::joint_bias0);
				vskin_vertex->set_property<trigonometry::skin_vertex>("float joint_bias1", &trigonometry::skin_vertex::joint_bias1);
				vskin_vertex->set_property<trigonometry::skin_vertex>("float joint_bias2", &trigonometry::skin_vertex::joint_bias2);
				vskin_vertex->set_property<trigonometry::skin_vertex>("float joint_bias3", &trigonometry::skin_vertex::joint_bias3);
				vskin_vertex->set_constructor<trigonometry::skin_vertex>("void f()");

				auto vshape_vertex = vm->set_pod<trigonometry::shape_vertex>("shape_vertex");
				vshape_vertex->set_property<trigonometry::shape_vertex>("float position_x", &trigonometry::shape_vertex::position_x);
				vshape_vertex->set_property<trigonometry::shape_vertex>("float position_y", &trigonometry::shape_vertex::position_y);
				vshape_vertex->set_property<trigonometry::shape_vertex>("float position_z", &trigonometry::shape_vertex::position_z);
				vshape_vertex->set_property<trigonometry::shape_vertex>("float texcoord_x", &trigonometry::shape_vertex::texcoord_x);
				vshape_vertex->set_property<trigonometry::shape_vertex>("float texcoord_y", &trigonometry::shape_vertex::texcoord_y);
				vshape_vertex->set_constructor<trigonometry::shape_vertex>("void f()");

				auto velement_vertex = vm->set_pod<trigonometry::element_vertex>("element_vertex");
				velement_vertex->set_property<trigonometry::element_vertex>("float position_x", &trigonometry::element_vertex::position_x);
				velement_vertex->set_property<trigonometry::element_vertex>("float position_y", &trigonometry::element_vertex::position_y);
				velement_vertex->set_property<trigonometry::element_vertex>("float position_z", &trigonometry::element_vertex::position_z);
				velement_vertex->set_property<trigonometry::element_vertex>("float velocity_x", &trigonometry::element_vertex::velocity_x);
				velement_vertex->set_property<trigonometry::element_vertex>("float velocity_y", &trigonometry::element_vertex::velocity_y);
				velement_vertex->set_property<trigonometry::element_vertex>("float velocity_z", &trigonometry::element_vertex::velocity_z);
				velement_vertex->set_property<trigonometry::element_vertex>("float color_x", &trigonometry::element_vertex::color_x);
				velement_vertex->set_property<trigonometry::element_vertex>("float color_y", &trigonometry::element_vertex::color_y);
				velement_vertex->set_property<trigonometry::element_vertex>("float color_z", &trigonometry::element_vertex::color_z);
				velement_vertex->set_property<trigonometry::element_vertex>("float color_w", &trigonometry::element_vertex::color_w);
				velement_vertex->set_property<trigonometry::element_vertex>("float scale", &trigonometry::element_vertex::scale);
				velement_vertex->set_property<trigonometry::element_vertex>("float rotation", &trigonometry::element_vertex::rotation);
				velement_vertex->set_property<trigonometry::element_vertex>("float angular", &trigonometry::element_vertex::angular);
				velement_vertex->set_constructor<trigonometry::element_vertex>("void f()");

				auto vcube_face = vm->set_enum("cube_face");
				vcube_face->set_value("positive_x", (int)trigonometry::cube_face::positive_x);
				vcube_face->set_value("negative_x", (int)trigonometry::cube_face::negative_x);
				vcube_face->set_value("positive_y", (int)trigonometry::cube_face::positive_y);
				vcube_face->set_value("negative_y", (int)trigonometry::cube_face::negative_y);
				vcube_face->set_value("positive_z", (int)trigonometry::cube_face::positive_z);
				vcube_face->set_value("negative_z", (int)trigonometry::cube_face::negative_z);

				auto vvector2 = vm->set_pod<trigonometry::vector2>("vector2");
				vvector2->set_constructor<trigonometry::vector2>("void f()");
				vvector2->set_constructor<trigonometry::vector2, float, float>("void f(float, float)");
				vvector2->set_constructor<trigonometry::vector2, float>("void f(float)");
				vvector2->set_property<trigonometry::vector2>("float x", &trigonometry::vector2::x);
				vvector2->set_property<trigonometry::vector2>("float y", &trigonometry::vector2::y);
				vvector2->set_method("float length() const", &trigonometry::vector2::length);
				vvector2->set_method("float sum() const", &trigonometry::vector2::sum);
				vvector2->set_method("float dot(const vector2 &in) const", &trigonometry::vector2::dot);
				vvector2->set_method("float distance(const vector2 &in) const", &trigonometry::vector2::distance);
				vvector2->set_method("float hypotenuse() const", &trigonometry::vector2::hypotenuse);
				vvector2->set_method("float look_at(const vector2 &in) const", &trigonometry::vector2::look_at);
				vvector2->set_method("float cross(const vector2 &in) const", &trigonometry::vector2::cross);
				vvector2->set_method("vector2 direction(float) const", &trigonometry::vector2::direction);
				vvector2->set_method("vector2 inv() const", &trigonometry::vector2::inv);
				vvector2->set_method("vector2 inv_x() const", &trigonometry::vector2::inv_x);
				vvector2->set_method("vector2 inv_y() const", &trigonometry::vector2::inv_y);
				vvector2->set_method("vector2 normalize() const", &trigonometry::vector2::normalize);
				vvector2->set_method("vector2 snormalize() const", &trigonometry::vector2::snormalize);
				vvector2->set_method("vector2 lerp(const vector2 &in, float) const", &trigonometry::vector2::lerp);
				vvector2->set_method("vector2 slerp(const vector2 &in, float) const", &trigonometry::vector2::slerp);
				vvector2->set_method("vector2 alerp(const vector2 &in, float) const", &trigonometry::vector2::alerp);
				vvector2->set_method("vector2 rlerp() const", &trigonometry::vector2::rlerp);
				vvector2->set_method("vector2 abs() const", &trigonometry::vector2::abs);
				vvector2->set_method("vector2 radians() const", &trigonometry::vector2::radians);
				vvector2->set_method("vector2 degrees() const", &trigonometry::vector2::degrees);
				vvector2->set_method("vector2 xy() const", &trigonometry::vector2::xy);
				vvector2->set_method<trigonometry::vector2, trigonometry::vector2, float>("vector2 mul(float) const", &trigonometry::vector2::mul);
				vvector2->set_method<trigonometry::vector2, trigonometry::vector2, float, float>("vector2 mul(float, float) const", &trigonometry::vector2::mul);
				vvector2->set_method<trigonometry::vector2, trigonometry::vector2, const trigonometry::vector2&>("vector2 mul(const vector2 &in) const", &trigonometry::vector2::mul);
				vvector2->set_method("vector2 div(const vector2 &in) const", &trigonometry::vector2::div);
				vvector2->set_method("vector2 set_x(float) const", &trigonometry::vector2::set_x);
				vvector2->set_method("vector2 set_y(float) const", &trigonometry::vector2::set_y);
				vvector2->set_method("void set(const vector2 &in) const", &trigonometry::vector2::set);
				vvector2->set_operator_ex(operators::mul_assign, (uint32_t)position::left, "vector2&", "const vector2 &in", &vector2_mul_eq1);
				vvector2->set_operator_ex(operators::mul_assign, (uint32_t)position::left, "vector2&", "float", &vector2_mul_eq2);
				vvector2->set_operator_ex(operators::div_assign, (uint32_t)position::left, "vector2&", "const vector2 &in", &vector_2div_eq1);
				vvector2->set_operator_ex(operators::div_assign, (uint32_t)position::left, "vector2&", "float", &vector_2div_eq2);
				vvector2->set_operator_ex(operators::add_assign, (uint32_t)position::left, "vector2&", "const vector2 &in", &vector2_add_eq1);
				vvector2->set_operator_ex(operators::add_assign, (uint32_t)position::left, "vector2&", "float", &vector2_add_eq2);
				vvector2->set_operator_ex(operators::sub_assign, (uint32_t)position::left, "vector2&", "const vector2 &in", &vector2_sub_eq1);
				vvector2->set_operator_ex(operators::sub_assign, (uint32_t)position::left, "vector2&", "float", &vector2_sub_eq2);
				vvector2->set_operator_ex(operators::mul, (uint32_t)position::constant, "vector2", "const vector2 &in", &vector2_mul1);
				vvector2->set_operator_ex(operators::mul, (uint32_t)position::constant, "vector2", "float", &vector2_mul2);
				vvector2->set_operator_ex(operators::div, (uint32_t)position::constant, "vector2", "const vector2 &in", &vector_2div1);
				vvector2->set_operator_ex(operators::div, (uint32_t)position::constant, "vector2", "float", &vector_2div2);
				vvector2->set_operator_ex(operators::add, (uint32_t)position::constant, "vector2", "const vector2 &in", &vector2_add1);
				vvector2->set_operator_ex(operators::add, (uint32_t)position::constant, "vector2", "float", &vector2_add2);
				vvector2->set_operator_ex(operators::sub, (uint32_t)position::constant, "vector2", "const vector2 &in", &vector2_sub1);
				vvector2->set_operator_ex(operators::sub, (uint32_t)position::constant, "vector2", "float", &vector2_sub2);
				vvector2->set_operator_ex(operators::neg, (uint32_t)position::constant, "vector2", "", &vector2_neg);
				vvector2->set_operator_ex(operators::equals, (uint32_t)position::constant, "bool", "const vector2 &in", &vector2_eq);
				vvector2->set_operator_ex(operators::cmp, (uint32_t)position::constant, "int", "const vector2 &in", &vector2_cmp);

				vm->begin_namespace("vector2");
				vm->set_function("vector2 random()", &trigonometry::vector2::random);
				vm->set_function("vector2 random_abs()", &trigonometry::vector2::random_abs);
				vm->set_function("vector2 one()", &trigonometry::vector2::one);
				vm->set_function("vector2 zero()", &trigonometry::vector2::zero);
				vm->set_function("vector2 up()", &trigonometry::vector2::up);
				vm->set_function("vector2 down()", &trigonometry::vector2::down);
				vm->set_function("vector2 left()", &trigonometry::vector2::left);
				vm->set_function("vector2 right()", &trigonometry::vector2::right);
				vm->end_namespace();

				auto vvector3 = vm->set_pod<trigonometry::vector3>("vector3");
				vvector3->set_constructor<trigonometry::vector3>("void f()");
				vvector3->set_constructor<trigonometry::vector3, float, float>("void f(float, float)");
				vvector3->set_constructor<trigonometry::vector3, float, float, float>("void f(float, float, float)");
				vvector3->set_constructor<trigonometry::vector3, float>("void f(float)");
				vvector3->set_constructor<trigonometry::vector3, const trigonometry::vector2&>("void f(const vector2 &in)");
				vvector3->set_property<trigonometry::vector3>("float x", &trigonometry::vector3::x);
				vvector3->set_property<trigonometry::vector3>("float y", &trigonometry::vector3::y);
				vvector3->set_property<trigonometry::vector3>("float z", &trigonometry::vector3::z);
				vvector3->set_method("float length() const", &trigonometry::vector3::length);
				vvector3->set_method("float sum() const", &trigonometry::vector3::sum);
				vvector3->set_method("float dot(const vector3 &in) const", &trigonometry::vector3::dot);
				vvector3->set_method("float distance(const vector3 &in) const", &trigonometry::vector3::distance);
				vvector3->set_method("float hypotenuse() const", &trigonometry::vector3::hypotenuse);
				vvector3->set_method("vector3 look_at(const vector3 &in) const", &trigonometry::vector3::look_at);
				vvector3->set_method("vector3 cross(const vector3 &in) const", &trigonometry::vector3::cross);
				vvector3->set_method("vector3 direction_horizontal() const", &trigonometry::vector3::hdirection);
				vvector3->set_method("vector3 direction_depth() const", &trigonometry::vector3::ddirection);
				vvector3->set_method("vector3 inv() const", &trigonometry::vector3::inv);
				vvector3->set_method("vector3 inv_x() const", &trigonometry::vector3::inv_x);
				vvector3->set_method("vector3 inv_y() const", &trigonometry::vector3::inv_y);
				vvector3->set_method("vector3 inv_z() const", &trigonometry::vector3::inv_z);
				vvector3->set_method("vector3 normalize() const", &trigonometry::vector3::normalize);
				vvector3->set_method("vector3 snormalize() const", &trigonometry::vector3::snormalize);
				vvector3->set_method("vector3 lerp(const vector3 &in, float) const", &trigonometry::vector3::lerp);
				vvector3->set_method("vector3 slerp(const vector3 &in, float) const", &trigonometry::vector3::slerp);
				vvector3->set_method("vector3 alerp(const vector3 &in, float) const", &trigonometry::vector3::alerp);
				vvector3->set_method("vector3 rlerp() const", &trigonometry::vector3::rlerp);
				vvector3->set_method("vector3 abs() const", &trigonometry::vector3::abs);
				vvector3->set_method("vector3 radians() const", &trigonometry::vector3::radians);
				vvector3->set_method("vector3 degrees() const", &trigonometry::vector3::degrees);
				vvector3->set_method("vector3 view_space() const", &trigonometry::vector3::view_space);
				vvector3->set_method("vector2 xy() const", &trigonometry::vector3::xy);
				vvector3->set_method("vector3 xyz() const", &trigonometry::vector3::xyz);
				vvector3->set_method<trigonometry::vector3, trigonometry::vector3, float>("vector3 mul(float) const", &trigonometry::vector3::mul);
				vvector3->set_method<trigonometry::vector3, trigonometry::vector3, const trigonometry::vector2&, float>("vector3 mul(const vector2 &in, float) const", &trigonometry::vector3::mul);
				vvector3->set_method<trigonometry::vector3, trigonometry::vector3, const trigonometry::vector3&>("vector3 mul(const vector3 &in) const", &trigonometry::vector3::mul);
				vvector3->set_method("vector3 div(const vector3 &in) const", &trigonometry::vector3::div);
				vvector3->set_method("vector3 set_x(float) const", &trigonometry::vector3::set_x);
				vvector3->set_method("vector3 set_y(float) const", &trigonometry::vector3::set_y);
				vvector3->set_method("vector3 set_z(float) const", &trigonometry::vector3::set_z);
				vvector3->set_method("void set(const vector3 &in) const", &trigonometry::vector3::set);
				vvector3->set_operator_ex(operators::mul_assign, (uint32_t)position::left, "vector3&", "const vector3 &in", &vector3_mul_eq1);
				vvector3->set_operator_ex(operators::mul_assign, (uint32_t)position::left, "vector3&", "float", &vector3_mul_eq2);
				vvector3->set_operator_ex(operators::div_assign, (uint32_t)position::left, "vector3&", "const vector3 &in", &vector_3div_eq1);
				vvector3->set_operator_ex(operators::div_assign, (uint32_t)position::left, "vector3&", "float", &vector_3div_eq2);
				vvector3->set_operator_ex(operators::add_assign, (uint32_t)position::left, "vector3&", "const vector3 &in", &vector3_add_eq1);
				vvector3->set_operator_ex(operators::add_assign, (uint32_t)position::left, "vector3&", "float", &vector3_add_eq2);
				vvector3->set_operator_ex(operators::sub_assign, (uint32_t)position::left, "vector3&", "const vector3 &in", &vector3_sub_eq1);
				vvector3->set_operator_ex(operators::sub_assign, (uint32_t)position::left, "vector3&", "float", &vector3_sub_eq2);
				vvector3->set_operator_ex(operators::mul, (uint32_t)position::constant, "vector3", "const vector3 &in", &vector3_mul1);
				vvector3->set_operator_ex(operators::mul, (uint32_t)position::constant, "vector3", "float", &vector3_mul2);
				vvector3->set_operator_ex(operators::div, (uint32_t)position::constant, "vector3", "const vector3 &in", &vector_3div1);
				vvector3->set_operator_ex(operators::div, (uint32_t)position::constant, "vector3", "float", &vector_3div2);
				vvector3->set_operator_ex(operators::add, (uint32_t)position::constant, "vector3", "const vector3 &in", &vector3_add1);
				vvector3->set_operator_ex(operators::add, (uint32_t)position::constant, "vector3", "float", &vector3_add2);
				vvector3->set_operator_ex(operators::sub, (uint32_t)position::constant, "vector3", "const vector3 &in", &vector3_sub1);
				vvector3->set_operator_ex(operators::sub, (uint32_t)position::constant, "vector3", "float", &vector3_sub2);
				vvector3->set_operator_ex(operators::neg, (uint32_t)position::constant, "vector3", "", &vector3_neg);
				vvector3->set_operator_ex(operators::equals, (uint32_t)position::constant, "bool", "const vector3 &in", &vector3_eq);
				vvector3->set_operator_ex(operators::cmp, (uint32_t)position::constant, "int", "const vector3 &in", &vector3_cmp);

				vm->begin_namespace("vector3");
				vm->set_function("vector3 random()", &trigonometry::vector3::random);
				vm->set_function("vector3 random_abs()", &trigonometry::vector3::random_abs);
				vm->set_function("vector3 one()", &trigonometry::vector3::one);
				vm->set_function("vector3 zero()", &trigonometry::vector3::zero);
				vm->set_function("vector3 up()", &trigonometry::vector3::up);
				vm->set_function("vector3 down()", &trigonometry::vector3::down);
				vm->set_function("vector3 left()", &trigonometry::vector3::left);
				vm->set_function("vector3 right()", &trigonometry::vector3::right);
				vm->set_function("vector3 forward()", &trigonometry::vector3::forward);
				vm->set_function("vector3 backward()", &trigonometry::vector3::backward);
				vm->end_namespace();

				auto vvector4 = vm->set_pod<trigonometry::vector4>("vector4");
				vvector4->set_constructor<trigonometry::vector4>("void f()");
				vvector4->set_constructor<trigonometry::vector4, float, float>("void f(float, float)");
				vvector4->set_constructor<trigonometry::vector4, float, float, float>("void f(float, float, float)");
				vvector4->set_constructor<trigonometry::vector4, float, float, float, float>("void f(float, float, float, float)");
				vvector4->set_constructor<trigonometry::vector4, float>("void f(float)");
				vvector4->set_constructor<trigonometry::vector4, const trigonometry::vector2&>("void f(const vector2 &in)");
				vvector4->set_constructor<trigonometry::vector4, const trigonometry::vector3&>("void f(const vector3 &in)");
				vvector4->set_property<trigonometry::vector4>("float x", &trigonometry::vector4::x);
				vvector4->set_property<trigonometry::vector4>("float y", &trigonometry::vector4::y);
				vvector4->set_property<trigonometry::vector4>("float z", &trigonometry::vector4::z);
				vvector4->set_property<trigonometry::vector4>("float w", &trigonometry::vector4::w);
				vvector4->set_method("float length() const", &trigonometry::vector4::length);
				vvector4->set_method("float sum() const", &trigonometry::vector4::sum);
				vvector4->set_method("float dot(const vector4 &in) const", &trigonometry::vector4::dot);
				vvector4->set_method("float distance(const vector4 &in) const", &trigonometry::vector4::distance);
				vvector4->set_method("float cross(const vector4 &in) const", &trigonometry::vector4::cross);
				vvector4->set_method("vector4 inv() const", &trigonometry::vector4::inv);
				vvector4->set_method("vector4 inv_x() const", &trigonometry::vector4::inv_x);
				vvector4->set_method("vector4 inv_y() const", &trigonometry::vector4::inv_y);
				vvector4->set_method("vector4 inv_z() const", &trigonometry::vector4::inv_z);
				vvector4->set_method("vector4 inv_w() const", &trigonometry::vector4::inv_w);
				vvector4->set_method("vector4 normalize() const", &trigonometry::vector4::normalize);
				vvector4->set_method("vector4 snormalize() const", &trigonometry::vector4::snormalize);
				vvector4->set_method("vector4 lerp(const vector4 &in, float) const", &trigonometry::vector4::lerp);
				vvector4->set_method("vector4 slerp(const vector4 &in, float) const", &trigonometry::vector4::slerp);
				vvector4->set_method("vector4 alerp(const vector4 &in, float) const", &trigonometry::vector4::alerp);
				vvector4->set_method("vector4 rlerp() const", &trigonometry::vector4::rlerp);
				vvector4->set_method("vector4 abs() const", &trigonometry::vector4::abs);
				vvector4->set_method("vector4 radians() const", &trigonometry::vector4::radians);
				vvector4->set_method("vector4 degrees() const", &trigonometry::vector4::degrees);
				vvector4->set_method("vector4 view_space() const", &trigonometry::vector4::view_space);
				vvector4->set_method("vector2 xy() const", &trigonometry::vector4::xy);
				vvector4->set_method("vector3 xyz() const", &trigonometry::vector4::xyz);
				vvector4->set_method("vector4 xyzw() const", &trigonometry::vector4::xyzw);
				vvector4->set_method<trigonometry::vector4, trigonometry::vector4, float>("vector4 mul(float) const", &trigonometry::vector4::mul);
				vvector4->set_method<trigonometry::vector4, trigonometry::vector4, const trigonometry::vector2&, float, float>("vector4 mul(const vector2 &in, float, float) const", &trigonometry::vector4::mul);
				vvector4->set_method<trigonometry::vector4, trigonometry::vector4, const trigonometry::vector3&, float>("vector4 mul(const vector3 &in, float) const", &trigonometry::vector4::mul);
				vvector4->set_method<trigonometry::vector4, trigonometry::vector4, const trigonometry::vector4&>("vector4 mul(const vector4 &in) const", &trigonometry::vector4::mul);
				vvector4->set_method("vector4 div(const vector4 &in) const", &trigonometry::vector4::div);
				vvector4->set_method("vector4 set_x(float) const", &trigonometry::vector4::set_x);
				vvector4->set_method("vector4 set_y(float) const", &trigonometry::vector4::set_y);
				vvector4->set_method("vector4 set_z(float) const", &trigonometry::vector4::set_z);
				vvector4->set_method("vector4 set_w(float) const", &trigonometry::vector4::set_w);
				vvector4->set_method("void set(const vector4 &in) const", &trigonometry::vector4::set);
				vvector4->set_operator_ex(operators::mul_assign, (uint32_t)position::left, "vector4&", "const vector4 &in", &vector4_mul_eq1);
				vvector4->set_operator_ex(operators::mul_assign, (uint32_t)position::left, "vector4&", "float", &vector4_mul_eq2);
				vvector4->set_operator_ex(operators::div_assign, (uint32_t)position::left, "vector4&", "const vector4 &in", &vector4_div_eq1);
				vvector4->set_operator_ex(operators::div_assign, (uint32_t)position::left, "vector4&", "float", &vector4_div_eq2);
				vvector4->set_operator_ex(operators::add_assign, (uint32_t)position::left, "vector4&", "const vector4 &in", &vector4_add_eq1);
				vvector4->set_operator_ex(operators::add_assign, (uint32_t)position::left, "vector4&", "float", &vector4_add_eq2);
				vvector4->set_operator_ex(operators::sub_assign, (uint32_t)position::left, "vector4&", "const vector4 &in", &vector4_sub_eq1);
				vvector4->set_operator_ex(operators::sub_assign, (uint32_t)position::left, "vector4&", "float", &vector4_sub_eq2);
				vvector4->set_operator_ex(operators::mul, (uint32_t)position::constant, "vector4", "const vector4 &in", &vector4_mul1);
				vvector4->set_operator_ex(operators::mul, (uint32_t)position::constant, "vector4", "float", &vector4_mul2);
				vvector4->set_operator_ex(operators::div, (uint32_t)position::constant, "vector4", "const vector4 &in", &vector4_div1);
				vvector4->set_operator_ex(operators::div, (uint32_t)position::constant, "vector4", "float", &vector4_div2);
				vvector4->set_operator_ex(operators::add, (uint32_t)position::constant, "vector4", "const vector4 &in", &vector4_add1);
				vvector4->set_operator_ex(operators::add, (uint32_t)position::constant, "vector4", "float", &vector4_add2);
				vvector4->set_operator_ex(operators::sub, (uint32_t)position::constant, "vector4", "const vector4 &in", &vector4_sub1);
				vvector4->set_operator_ex(operators::sub, (uint32_t)position::constant, "vector4", "float", &vector4_sub2);
				vvector4->set_operator_ex(operators::neg, (uint32_t)position::constant, "vector4", "", &vector4_neg);
				vvector4->set_operator_ex(operators::equals, (uint32_t)position::constant, "bool", "const vector4 &in", &vector4_eq);
				vvector4->set_operator_ex(operators::cmp, (uint32_t)position::constant, "int", "const vector4 &in", &vector4_cmp);

				vm->begin_namespace("vector4");
				vm->set_function("vector4 random()", &trigonometry::vector4::random);
				vm->set_function("vector4 random_abs()", &trigonometry::vector4::random_abs);
				vm->set_function("vector4 one()", &trigonometry::vector4::one);
				vm->set_function("vector4 zero()", &trigonometry::vector4::zero);
				vm->set_function("vector4 up()", &trigonometry::vector4::up);
				vm->set_function("vector4 down()", &trigonometry::vector4::down);
				vm->set_function("vector4 left()", &trigonometry::vector4::left);
				vm->set_function("vector4 right()", &trigonometry::vector4::right);
				vm->set_function("vector4 forward()", &trigonometry::vector4::forward);
				vm->set_function("vector4 backward()", &trigonometry::vector4::backward);
				vm->set_function("vector4 unitW()", &trigonometry::vector4::unit_w);
				vm->end_namespace();

				auto vmatrix4x4 = vm->set_pod<trigonometry::matrix4x4>("matrix4x4");
				vmatrix4x4->set_constructor<trigonometry::matrix4x4>("void f()");
				vmatrix4x4->set_constructor<trigonometry::matrix4x4, const trigonometry::vector4&, const trigonometry::vector4&, const trigonometry::vector4&, const trigonometry::vector4&>("void f(const vector4 &in, const vector4 &in, const vector4 &in, const vector4 &in)");
				vmatrix4x4->set_constructor<trigonometry::matrix4x4, float, float, float, float, float, float, float, float, float, float, float, float, float, float, float, float>("void f(float, float, float, float, float, float, float, float, float, float, float, float, float, float, float, float)");
				vmatrix4x4->set_method("matrix4x4 inv() const", &trigonometry::matrix4x4::inv);
				vmatrix4x4->set_method("matrix4x4 transpose() const", &trigonometry::matrix4x4::transpose);
				vmatrix4x4->set_method("matrix4x4 set_scale(const vector3 &in) const", &trigonometry::matrix4x4::set_scale);
				vmatrix4x4->set_method("vector4 row11() const", &trigonometry::matrix4x4::row11);
				vmatrix4x4->set_method("vector4 row22() const", &trigonometry::matrix4x4::row22);
				vmatrix4x4->set_method("vector4 row33() const", &trigonometry::matrix4x4::row33);
				vmatrix4x4->set_method("vector4 row44() const", &trigonometry::matrix4x4::row44);
				vmatrix4x4->set_method("vector3 up() const", &trigonometry::matrix4x4::up);
				vmatrix4x4->set_method("vector3 right() const", &trigonometry::matrix4x4::right);
				vmatrix4x4->set_method("vector3 forward() const", &trigonometry::matrix4x4::forward);
				vmatrix4x4->set_method("vector3 rotation_quaternion() const", &trigonometry::matrix4x4::rotation_quaternion);
				vmatrix4x4->set_method("vector3 rotation_euler() const", &trigonometry::matrix4x4::rotation_euler);
				vmatrix4x4->set_method("vector3 position() const", &trigonometry::matrix4x4::position);
				vmatrix4x4->set_method("vector3 scale() const", &trigonometry::matrix4x4::scale);
				vmatrix4x4->set_method("vector3 xy() const", &trigonometry::matrix4x4::xy);
				vmatrix4x4->set_method("vector3 xyz() const", &trigonometry::matrix4x4::xyz);
				vmatrix4x4->set_method("vector3 xyzw() const", &trigonometry::matrix4x4::xyzw);
				vmatrix4x4->set_method("void identify()", &trigonometry::matrix4x4::identify);
				vmatrix4x4->set_method("void set(const matrix4x4 &in)", &trigonometry::matrix4x4::set);
				vmatrix4x4->set_method<trigonometry::matrix4x4, trigonometry::matrix4x4, const trigonometry::matrix4x4&>("matrix4x4 mul(const matrix4x4 &in) const", &trigonometry::matrix4x4::mul);
				vmatrix4x4->set_method<trigonometry::matrix4x4, trigonometry::matrix4x4, const trigonometry::vector4&>("matrix4x4 mul(const vector4 &in) const", &trigonometry::matrix4x4::mul);
				vmatrix4x4->set_operator_ex(operators::index, (uint32_t)position::left, "float&", "usize", &matrix4x4_get_row);
				vmatrix4x4->set_operator_ex(operators::index, (uint32_t)position::constant, "const float&", "usize", &matrix4x4_get_row);
				vmatrix4x4->set_operator_ex(operators::equals, (uint32_t)position::constant, "bool", "const matrix4x4 &in", &matrix4x4_eq);
				vmatrix4x4->set_operator_ex(operators::mul, (uint32_t)position::constant, "matrix4x4", "const matrix4x4 &in", &matrix4x4_mul1);
				vmatrix4x4->set_operator_ex(operators::mul, (uint32_t)position::constant, "vector4", "const vector4 &in", &matrix4x4_mul2);

				vm->begin_namespace("matrix4x4");
				vm->set_function("matrix4x4 identity()", &trigonometry::matrix4x4::identity);
				vm->set_function("matrix4x4 create_rotation_x(float)", &trigonometry::matrix4x4::create_rotation_x);
				vm->set_function("matrix4x4 create_rotation_y(float)", &trigonometry::matrix4x4::create_rotation_y);
				vm->set_function("matrix4x4 create_rotation_z(float)", &trigonometry::matrix4x4::create_rotation_z);
				vm->set_function("matrix4x4 create_view(const vector3 &in, const vector3 &in)", &trigonometry::matrix4x4::create_view);
				vm->set_function("matrix4x4 create_scale(const vector3 &in)", &trigonometry::matrix4x4::create_scale);
				vm->set_function("matrix4x4 create_translated_scale(const vector3& in, const vector3 &in)", &trigonometry::matrix4x4::create_translated_scale);
				vm->set_function("matrix4x4 create_translation(const vector3& in)", &trigonometry::matrix4x4::create_translation);
				vm->set_function("matrix4x4 create_perspective(float, float, float, float)", &trigonometry::matrix4x4::create_perspective);
				vm->set_function("matrix4x4 create_perspective_rad(float, float, float, float)", &trigonometry::matrix4x4::create_perspective_rad);
				vm->set_function("matrix4x4 create_orthographic(float, float, float, float)", &trigonometry::matrix4x4::create_orthographic);
				vm->set_function("matrix4x4 create_orthographic_off_center(float, float, float, float, float, float)", &trigonometry::matrix4x4::create_orthographic_off_center);
				vm->set_function<trigonometry::matrix4x4(const trigonometry::vector3&)>("matrix4x4 createRotation(const vector3 &in)", &trigonometry::matrix4x4::create_rotation);
				vm->set_function<trigonometry::matrix4x4(const trigonometry::vector3&, const trigonometry::vector3&, const trigonometry::vector3&)>("matrix4x4 create_rotation(const vector3 &in, const vector3 &in, const vector3 &in)", &trigonometry::matrix4x4::create_rotation);
				vm->set_function<trigonometry::matrix4x4(const trigonometry::vector3&, const trigonometry::vector3&, const trigonometry::vector3&)>("matrix4x4 create_look_at(const vector3 &in, const vector3 &in, const vector3 &in)", &trigonometry::matrix4x4::create_look_at);
				vm->set_function<trigonometry::matrix4x4(trigonometry::cube_face, const trigonometry::vector3&)>("matrix4x4 create_look_at(cube_face, const vector3 &in)", &trigonometry::matrix4x4::create_look_at);
				vm->set_function<trigonometry::matrix4x4(const trigonometry::vector3&, const trigonometry::vector3&, const trigonometry::vector3&)>("matrix4x4 create(const vector3 &in, const vector3 &in, const vector3 &in)", &trigonometry::matrix4x4::create);
				vm->set_function<trigonometry::matrix4x4(const trigonometry::vector3&, const trigonometry::vector3&)>("matrix4x4 create(const vector3 &in, const vector3 &in)", &trigonometry::matrix4x4::create);
				vm->end_namespace();

				auto vquaternion = vm->set_pod<trigonometry::vector4>("quaternion");
				vquaternion->set_constructor<trigonometry::quaternion>("void f()");
				vquaternion->set_constructor<trigonometry::quaternion, float, float, float, float>("void f(float, float, float, float)");
				vquaternion->set_constructor<trigonometry::quaternion, const trigonometry::vector3&, float>("void f(const vector3 &in, float)");
				vquaternion->set_constructor<trigonometry::quaternion, const trigonometry::vector3&>("void f(const vector3 &in)");
				vquaternion->set_constructor<trigonometry::quaternion, const trigonometry::matrix4x4&>("void f(const matrix4x4 &in)");
				vquaternion->set_property<trigonometry::quaternion>("float x", &trigonometry::quaternion::x);
				vquaternion->set_property<trigonometry::quaternion>("float y", &trigonometry::quaternion::y);
				vquaternion->set_property<trigonometry::quaternion>("float z", &trigonometry::quaternion::z);
				vquaternion->set_property<trigonometry::quaternion>("float w", &trigonometry::quaternion::w);
				vquaternion->set_method("void set_axis(const vector3 &in, float)", &trigonometry::quaternion::set_axis);
				vquaternion->set_method("void set_euler(const vector3 &in)", &trigonometry::quaternion::set_euler);
				vquaternion->set_method("void set_matrix(const matrix4x4 &in)", &trigonometry::quaternion::set_matrix);
				vquaternion->set_method("void set(const quaternion &in)", &trigonometry::quaternion::set);
				vquaternion->set_method("quaternion normalize() const", &trigonometry::quaternion::normalize);
				vquaternion->set_method("quaternion snormalize() const", &trigonometry::quaternion::snormalize);
				vquaternion->set_method("quaternion conjugate() const", &trigonometry::quaternion::conjugate);
				vquaternion->set_method("quaternion sub(const quaternion &in) const", &trigonometry::quaternion::sub);
				vquaternion->set_method("quaternion add(const quaternion &in) const", &trigonometry::quaternion::add);
				vquaternion->set_method("quaternion lerp(const quaternion &in, float) const", &trigonometry::quaternion::lerp);
				vquaternion->set_method("quaternion slerp(const quaternion &in, float) const", &trigonometry::quaternion::slerp);
				vquaternion->set_method("vector3 forward() const", &trigonometry::quaternion::forward);
				vquaternion->set_method("vector3 up() const", &trigonometry::quaternion::up);
				vquaternion->set_method("vector3 right() const", &trigonometry::quaternion::right);
				vquaternion->set_method("matrix4x4 get_matrix() const", &trigonometry::quaternion::get_matrix);
				vquaternion->set_method("vector3 get_euler() const", &trigonometry::quaternion::get_euler);
				vquaternion->set_method("float dot(const quaternion &in) const", &trigonometry::quaternion::dot);
				vquaternion->set_method("float length() const", &trigonometry::quaternion::length);
				vquaternion->set_method<trigonometry::quaternion, trigonometry::quaternion, float>("quaternion mul(float) const", &trigonometry::quaternion::mul);
				vquaternion->set_method<trigonometry::quaternion, trigonometry::quaternion, const trigonometry::quaternion&>("quaternion mul(const quaternion &in) const", &trigonometry::quaternion::mul);
				vquaternion->set_method<trigonometry::quaternion, trigonometry::vector3, const trigonometry::vector3&>("vector3 mul(const vector3 &in) const", &trigonometry::quaternion::mul);
				vquaternion->set_operator_ex(operators::mul, (uint32_t)position::constant, "vector3", "const vector3 &in", &quaternion_mul1);
				vquaternion->set_operator_ex(operators::mul, (uint32_t)position::constant, "quaternion", "const quaternion &in", &quaternion_mul2);
				vquaternion->set_operator_ex(operators::mul, (uint32_t)position::constant, "quaternion", "float", &quaternion_mul3);
				vquaternion->set_operator_ex(operators::add, (uint32_t)position::constant, "quaternion", "const quaternion &in", &quaternion_add);
				vquaternion->set_operator_ex(operators::sub, (uint32_t)position::constant, "quaternion", "const quaternion &in", &quaternion_sub);

				vm->begin_namespace("quaternion");
				vm->set_function("quaternion create_euler_rotation(const vector3 &in)", &trigonometry::quaternion::create_euler_rotation);
				vm->set_function("quaternion create_rotation(const matrix4x4 &in)", &trigonometry::quaternion::create_rotation);
				vm->end_namespace();

				auto vrectangle = vm->set_pod<trigonometry::rectangle>("rectangle");
				vrectangle->set_property<trigonometry::rectangle>("int64 left", &trigonometry::rectangle::left);
				vrectangle->set_property<trigonometry::rectangle>("int64 top", &trigonometry::rectangle::top);
				vrectangle->set_property<trigonometry::rectangle>("int64 right", &trigonometry::rectangle::right);
				vrectangle->set_property<trigonometry::rectangle>("int64 bottom", &trigonometry::rectangle::bottom);
				vrectangle->set_constructor<trigonometry::rectangle>("void f()");

				auto vbounding = vm->set_pod<trigonometry::bounding>("bounding");
				vbounding->set_property<trigonometry::bounding>("vector3 lower", &trigonometry::bounding::lower);
				vbounding->set_property<trigonometry::bounding>("vector3 upper", &trigonometry::bounding::upper);
				vbounding->set_property<trigonometry::bounding>("vector3 center", &trigonometry::bounding::center);
				vbounding->set_property<trigonometry::bounding>("float radius", &trigonometry::bounding::radius);
				vbounding->set_property<trigonometry::bounding>("float volume", &trigonometry::bounding::volume);
				vbounding->set_constructor<trigonometry::bounding>("void f()");
				vbounding->set_constructor<trigonometry::bounding, const trigonometry::vector3&, const trigonometry::vector3&>("void f(const vector3 &in, const vector3 &in)");
				vbounding->set_method("void merge(const bounding &in, const bounding &in)", &trigonometry::bounding::merge);
				vbounding->set_method("bool contains(const bounding &in) const", &trigonometry::bounding::contains);
				vbounding->set_method("bool overlaps(const bounding &in) const", &trigonometry::bounding::overlaps);

				auto vray = vm->set_pod<trigonometry::ray>("ray");
				vray->set_property<trigonometry::ray>("vector3 origin", &trigonometry::ray::origin);
				vray->set_property<trigonometry::ray>("vector3 direction", &trigonometry::ray::direction);
				vray->set_constructor<trigonometry::ray>("void f()");
				vray->set_constructor<trigonometry::ray, const trigonometry::vector3&, const trigonometry::vector3&>("void f(const vector3 &in, const vector3 &in)");
				vray->set_method("vector3 get_point(float) const", &trigonometry::ray::get_point);
				vray->set_method("bool intersects_plane(const vector3 &in, float) const", &trigonometry::ray::intersects_plane);
				vray->set_method("bool intersects_sphere(const vector3 &in, float, bool = true) const", &trigonometry::ray::intersects_sphere);
				vray->set_method("bool intersects_aabb_at(const vector3 &in, const vector3 &in, vector3 &out) const", &trigonometry::ray::intersects_aabb_at);
				vray->set_method("bool intersects_aabb(const vector3 &in, const vector3 &in, vector3 &out) const", &trigonometry::ray::intersects_aabb);
				vray->set_method("bool intersects_obb(const matrix4x4 &in, vector3 &out) const", &trigonometry::ray::intersects_obb);

				auto vfrustum8c = vm->set_pod<trigonometry::frustum8c>("frustum_8c");
				vfrustum8c->set_constructor<trigonometry::frustum8c>("void f()");
				vfrustum8c->set_constructor<trigonometry::frustum8c, float, float, float, float>("void f(float, float, float, float)");
				vfrustum8c->set_method("void transform(const matrix4x4 &in) const", &trigonometry::frustum8c::transform);
				vfrustum8c->set_method("void get_bounding_box(vector2 &out, vector2 &out, vector2 &out) const", &trigonometry::frustum8c::get_bounding_box);
				vfrustum8c->set_operator_ex(operators::index, (uint32_t)position::left, "vector4&", "usize", &frustum8cget_corners);
				vfrustum8c->set_operator_ex(operators::index, (uint32_t)position::constant, "const vector4&", "usize", &frustum8cget_corners);

				auto vfrustum6p = vm->set_pod<trigonometry::frustum6p>("frustum_6p");
				vfrustum6p->set_constructor<trigonometry::frustum6p>("void f()");
				vfrustum6p->set_constructor<trigonometry::frustum6p, const trigonometry::matrix4x4&>("void f(const matrix4x4 &in)");
				vfrustum6p->set_method("bool overlaps_aabb(const bounding &in) const", &trigonometry::frustum6p::overlaps_aabb);
				vfrustum6p->set_method("bool overlaps_sphere(const vector3 &in, float) const", &trigonometry::frustum6p::overlaps_sphere);
				vfrustum6p->set_operator_ex(operators::index, (uint32_t)position::left, "vector4&", "usize", &frustum6pget_corners);
				vfrustum6p->set_operator_ex(operators::index, (uint32_t)position::constant, "const vector4&", "usize", &frustum6pget_corners);

				auto vjoint = vm->set_struct_trivial<trigonometry::joint>("joint");
				vjoint->set_property<trigonometry::joint>("string name", &trigonometry::joint::name);
				vjoint->set_property<trigonometry::joint>("matrix4x4 global", &trigonometry::joint::global);
				vjoint->set_property<trigonometry::joint>("matrix4x4 local", &trigonometry::joint::local);
				vjoint->set_property<trigonometry::joint>("usize index", &trigonometry::joint::index);
				vjoint->set_constructor<trigonometry::joint>("void f()");
				vjoint->set_method_ex("usize size() const", &joint_size);
				vjoint->set_operator_ex(operators::index, (uint32_t)position::left, "joint&", "usize", &joint_get_childs);
				vjoint->set_operator_ex(operators::index, (uint32_t)position::constant, "const joint&", "usize", &joint_get_childs);

				auto vanimator_key = vm->set_pod<trigonometry::animator_key>("animator_key");
				vanimator_key->set_property<trigonometry::animator_key>("vector3 position", &trigonometry::animator_key::position);
				vanimator_key->set_property<trigonometry::animator_key>("quaternion rotation", &trigonometry::animator_key::rotation);
				vanimator_key->set_property<trigonometry::animator_key>("vector3 scale", &trigonometry::animator_key::scale);
				vanimator_key->set_property<trigonometry::animator_key>("float time", &trigonometry::animator_key::time);
				vanimator_key->set_constructor<trigonometry::animator_key>("void f()");

				auto vskin_animator_key = vm->set_struct_trivial<trigonometry::skin_animator_key>("skin_animator_key");
				vskin_animator_key->set_property<trigonometry::skin_animator_key>("float time", &trigonometry::skin_animator_key::time);
				vskin_animator_key->set_constructor<trigonometry::animator_key>("void f()");
				vskin_animator_key->set_method_ex("usize size() const", &skin_animator_key_size);
				vskin_animator_key->set_operator_ex(operators::index, (uint32_t)position::left, "animator_key&", "usize", &skin_animator_key_get_pose);
				vskin_animator_key->set_operator_ex(operators::index, (uint32_t)position::constant, "const animator_key&", "usize", &skin_animator_key_get_pose);

				auto vskin_animator_clip = vm->set_struct_trivial<trigonometry::skin_animator_clip>("skin_animator_clip");
				vskin_animator_clip->set_property<trigonometry::skin_animator_clip>("string name", &trigonometry::skin_animator_clip::name);
				vskin_animator_clip->set_property<trigonometry::skin_animator_clip>("float duration", &trigonometry::skin_animator_clip::duration);
				vskin_animator_clip->set_property<trigonometry::skin_animator_clip>("float rate", &trigonometry::skin_animator_clip::rate);
				vskin_animator_clip->set_constructor<trigonometry::skin_animator_clip>("void f()");
				vskin_animator_clip->set_method_ex("usize size() const", &skin_animator_clip_size);
				vskin_animator_clip->set_operator_ex(operators::index, (uint32_t)position::left, "skin_animator_key&", "usize", &skin_animator_clip_get_keys);
				vskin_animator_clip->set_operator_ex(operators::index, (uint32_t)position::constant, "const skin_animator_key&", "usize", &skin_animator_clip_get_keys);

				auto vkey_animator_clip = vm->set_struct_trivial<trigonometry::key_animator_clip>("key_animator_clip");
				vkey_animator_clip->set_property<trigonometry::key_animator_clip>("string name", &trigonometry::key_animator_clip::name);
				vkey_animator_clip->set_property<trigonometry::key_animator_clip>("float duration", &trigonometry::key_animator_clip::duration);
				vkey_animator_clip->set_property<trigonometry::key_animator_clip>("float rate", &trigonometry::key_animator_clip::rate);
				vkey_animator_clip->set_constructor<trigonometry::key_animator_clip>("void f()");
				vkey_animator_clip->set_method_ex("usize size() const", &key_animator_clip_size);
				vkey_animator_clip->set_operator_ex(operators::index, (uint32_t)position::left, "animator_key&", "usize", &key_animator_clip_get_keys);
				vkey_animator_clip->set_operator_ex(operators::index, (uint32_t)position::constant, "const animator_key&", "usize", &key_animator_clip_get_keys);

				auto vrandom_vector2 = vm->set_pod<trigonometry::random_vector2>("random_vector2");
				vrandom_vector2->set_property<trigonometry::random_vector2>("vector2 min", &trigonometry::random_vector2::min);
				vrandom_vector2->set_property<trigonometry::random_vector2>("vector2 max", &trigonometry::random_vector2::max);
				vrandom_vector2->set_property<trigonometry::random_vector2>("bool intensity", &trigonometry::random_vector2::intensity);
				vrandom_vector2->set_property<trigonometry::random_vector2>("float accuracy", &trigonometry::random_vector2::accuracy);
				vrandom_vector2->set_constructor<trigonometry::random_vector2>("void f()");
				vrandom_vector2->set_constructor<trigonometry::random_vector2, const trigonometry::vector2&, const trigonometry::vector2&, bool, float>("void f(const vector2 &in, const vector2 &in, bool, float)");
				vrandom_vector2->set_method("vector2 generate()", &trigonometry::random_vector2::generate);

				auto vrandom_vector3 = vm->set_pod<trigonometry::random_vector3>("random_vector3");
				vrandom_vector3->set_property<trigonometry::random_vector3>("vector3 min", &trigonometry::random_vector3::min);
				vrandom_vector3->set_property<trigonometry::random_vector3>("vector3 max", &trigonometry::random_vector3::max);
				vrandom_vector3->set_property<trigonometry::random_vector3>("bool intensity", &trigonometry::random_vector3::intensity);
				vrandom_vector3->set_property<trigonometry::random_vector3>("float accuracy", &trigonometry::random_vector3::accuracy);
				vrandom_vector3->set_constructor<trigonometry::random_vector3>("void f()");
				vrandom_vector3->set_constructor<trigonometry::random_vector3, const trigonometry::vector3&, const trigonometry::vector3&, bool, float>("void f(const vector3 &in, const vector3 &in, bool, float)");
				vrandom_vector3->set_method("vector3 generate()", &trigonometry::random_vector3::generate);

				auto vrandom_vector4 = vm->set_pod<trigonometry::random_vector4>("random_vector4");
				vrandom_vector4->set_property<trigonometry::random_vector4>("vector4 min", &trigonometry::random_vector4::min);
				vrandom_vector4->set_property<trigonometry::random_vector4>("vector4 max", &trigonometry::random_vector4::max);
				vrandom_vector4->set_property<trigonometry::random_vector4>("bool intensity", &trigonometry::random_vector4::intensity);
				vrandom_vector4->set_property<trigonometry::random_vector4>("float accuracy", &trigonometry::random_vector4::accuracy);
				vrandom_vector4->set_constructor<trigonometry::random_vector4>("void f()");
				vrandom_vector4->set_constructor<trigonometry::random_vector4, const trigonometry::vector4&, const trigonometry::vector4&, bool, float>("void f(const vector4 &in, const vector4 &in, bool, float)");
				vrandom_vector4->set_method("vector4 generate()", &trigonometry::random_vector4::generate);

				auto vrandom_float = vm->set_pod<trigonometry::random_float>("random_float");
				vrandom_float->set_property<trigonometry::random_float>("float min", &trigonometry::random_float::min);
				vrandom_float->set_property<trigonometry::random_float>("float max", &trigonometry::random_float::max);
				vrandom_float->set_property<trigonometry::random_float>("bool intensity", &trigonometry::random_float::intensity);
				vrandom_float->set_property<trigonometry::random_float>("float accuracy", &trigonometry::random_float::accuracy);
				vrandom_float->set_constructor<trigonometry::random_float>("void f()");
				vrandom_float->set_constructor<trigonometry::random_float, float, float, bool, float>("void f(float, float, bool, float)");
				vrandom_float->set_method("float generate()", &trigonometry::random_float::generate);

				auto vpositioning = vm->set_enum("positioning");
				vpositioning->set_value("local", (int)trigonometry::positioning::local);
				vpositioning->set_value("global", (int)trigonometry::positioning::global);

				auto vspacing = vm->set_pod<trigonometry::transform::spacing>("transform_spacing");
				vspacing->set_property<trigonometry::transform::spacing>("matrix4x4 offset", &trigonometry::transform::spacing::offset);
				vspacing->set_property<trigonometry::transform::spacing>("vector3 position", &trigonometry::transform::spacing::position);
				vspacing->set_property<trigonometry::transform::spacing>("vector3 rotation", &trigonometry::transform::spacing::rotation);
				vspacing->set_property<trigonometry::transform::spacing>("vector3 scale", &trigonometry::transform::spacing::scale);
				vspacing->set_constructor<trigonometry::transform::spacing>("void f()");

				auto vtransform = vm->set_class<trigonometry::transform>("transform", false);
				vtransform->set_property<trigonometry::transform>("uptr@ user_data", &trigonometry::transform::user_data);
				vtransform->set_constructor<trigonometry::transform, void*>("transform@ f(uptr@)");
				vtransform->set_method("void synchronize()", &trigonometry::transform::synchronize);
				vtransform->set_method("void move(const vector3 &in)", &trigonometry::transform::move);
				vtransform->set_method("void rotate(const vector3 &in)", &trigonometry::transform::rotate);
				vtransform->set_method("void rescale(const vector3 &in)", &trigonometry::transform::rescale);
				vtransform->set_method("void localize(transform_spacing &in)", &trigonometry::transform::localize);
				vtransform->set_method("void globalize(transform_spacing &in)", &trigonometry::transform::globalize);
				vtransform->set_method("void specialize(transform_spacing &in)", &trigonometry::transform::specialize);
				vtransform->set_method("void copy(transform@+) const", &trigonometry::transform::copy);
				vtransform->set_method("void add_child(transform@+)", &trigonometry::transform::add_child);
				vtransform->set_method("void remove_child(transform@+)", &trigonometry::transform::remove_child);
				vtransform->set_method("void remove_childs()", &trigonometry::transform::remove_childs);
				vtransform->set_method("void make_dirty()", &trigonometry::transform::make_dirty);
				vtransform->set_method("void set_scaling(bool)", &trigonometry::transform::set_scaling);
				vtransform->set_method("void set_position(const vector3 &in)", &trigonometry::transform::set_position);
				vtransform->set_method("void set_rotation(const vector3 &in)", &trigonometry::transform::set_rotation);
				vtransform->set_method("void set_scale(const vector3 &in)", &trigonometry::transform::set_scale);
				vtransform->set_method("void set_spacing(positioning, transform_spacing &in)", &trigonometry::transform::set_spacing);
				vtransform->set_method("void set_pivot(transform@+, transform_spacing &in)", &trigonometry::transform::set_pivot);
				vtransform->set_method("void set_root(transform@+)", &trigonometry::transform::set_root);
				vtransform->set_method("void get_bounds(matrix4x4 &in, vector3 &in, vector3 &in)", &trigonometry::transform::get_bounds);
				vtransform->set_method("bool has_root(transform@+) const", &trigonometry::transform::has_root);
				vtransform->set_method("bool has_child(transform@+) const", &trigonometry::transform::has_child);
				vtransform->set_method("bool has_scaling() const", &trigonometry::transform::has_scaling);
				vtransform->set_method("bool is_dirty() const", &trigonometry::transform::is_dirty);
				vtransform->set_method("const matrix4x4& get_bias() const", &trigonometry::transform::get_bias);
				vtransform->set_method("const matrix4x4& get_bias_unscaled() const", &trigonometry::transform::get_bias_unscaled);
				vtransform->set_method("const vector3& get_position() const", &trigonometry::transform::get_position);
				vtransform->set_method("const vector3& get_rotation() const", &trigonometry::transform::get_rotation);
				vtransform->set_method("const vector3& get_scale() const", &trigonometry::transform::get_scale);
				vtransform->set_method("vector3 forward() const", &trigonometry::transform::forward);
				vtransform->set_method("vector3 right() const", &trigonometry::transform::right);
				vtransform->set_method("vector3 up() const", &trigonometry::transform::up);
				vtransform->set_method<trigonometry::transform, trigonometry::transform::spacing&>("transform_spacing& get_spacing()", &trigonometry::transform::get_spacing);
				vtransform->set_method<trigonometry::transform, trigonometry::transform::spacing&, trigonometry::positioning>("transform_spacing& get_spacing(positioning)", &trigonometry::transform::get_spacing);
				vtransform->set_method("transform@+ get_root() const", &trigonometry::transform::get_root);
				vtransform->set_method("transform@+ get_upper_root() const", &trigonometry::transform::get_upper_root);
				vtransform->set_method("transform@+ get_child(usize) const", &trigonometry::transform::get_child);
				vtransform->set_method("usize get_childs_count() const", &trigonometry::transform::get_childs_count);

				auto vnode = vm->set_pod<trigonometry::cosmos::node>("cosmos_node");
				vnode->set_property<trigonometry::cosmos::node>("bounding bounds", &trigonometry::cosmos::node::bounds);
				vnode->set_property<trigonometry::cosmos::node>("usize parent", &trigonometry::cosmos::node::parent);
				vnode->set_property<trigonometry::cosmos::node>("usize next", &trigonometry::cosmos::node::next);
				vnode->set_property<trigonometry::cosmos::node>("usize left", &trigonometry::cosmos::node::left);
				vnode->set_property<trigonometry::cosmos::node>("usize right", &trigonometry::cosmos::node::right);
				vnode->set_property<trigonometry::cosmos::node>("uptr@ item", &trigonometry::cosmos::node::item);
				vnode->set_property<trigonometry::cosmos::node>("int32 height", &trigonometry::cosmos::node::height);
				vnode->set_constructor<trigonometry::cosmos::node>("void f()");
				vnode->set_method("bool is_leaf() const", &trigonometry::cosmos::node::is_leaf);

				auto vcosmos = vm->set_struct_trivial<trigonometry::cosmos>("cosmos");
				vcosmos->set_function_def("bool cosmos_query_overlaps_sync(const bounding &in)");
				vcosmos->set_function_def("void cosmos_query_match_sync(uptr@)");
				vcosmos->set_constructor<trigonometry::cosmos>("void f(usize = 16)");
				vcosmos->set_method("void reserve(usize)", &trigonometry::cosmos::reserve);
				vcosmos->set_method("void clear()", &trigonometry::cosmos::clear);
				vcosmos->set_method("void remove_item(uptr@)", &trigonometry::cosmos::remove_item);
				vcosmos->set_method("void insert_item(uptr@, const vector3 &in, const vector3 &in)", &trigonometry::cosmos::insert_item);
				vcosmos->set_method("void update_item(uptr@, const vector3 &in, const vector3 &in, bool = false)", &trigonometry::cosmos::update_item);
				vcosmos->set_method("const bounding& get_area(uptr@)", &trigonometry::cosmos::get_area);
				vcosmos->set_method("usize get_nodes_count() const", &trigonometry::cosmos::get_nodes_count);
				vcosmos->set_method("usize get_height() const", &trigonometry::cosmos::get_height);
				vcosmos->set_method("usize get_max_balance() const", &trigonometry::cosmos::get_max_balance);
				vcosmos->set_method("usize get_root() const", &trigonometry::cosmos::get_root);
				vcosmos->set_method("const cosmos_node& get_root_node() const", &trigonometry::cosmos::get_root_node);
				vcosmos->set_method("const cosmos_node& get_node(usize) const", &trigonometry::cosmos::get_node);
				vcosmos->set_method("float get_volume_ratio() const", &trigonometry::cosmos::get_volume_ratio);
				vcosmos->set_method("bool is_null(usize) const", &trigonometry::cosmos::is_null);
				vcosmos->set_method("bool is_empty() const", &trigonometry::cosmos::empty);
				vcosmos->set_method_ex("void query_index(cosmos_query_overlaps_sync@, cosmos_query_match_sync@)", &cosmos_query_index);

				vm->begin_namespace("geometric");
				vm->set_function("bool is_cube_in_frustum(const matrix4x4 &in, float)", &trigonometry::geometric::is_cube_in_frustum);
				vm->set_function("bool is_left_handed()", &trigonometry::geometric::is_left_handed);
				vm->set_function("bool has_sphere_intersected(const vector3 &in, float, const vector3 &in, float)", &trigonometry::geometric::has_sphere_intersected);
				vm->set_function("bool has_line_intersected(float, float, const vector3 &in, const vector3 &in, vector3 &out)", &trigonometry::geometric::has_line_intersected);
				vm->set_function("bool has_line_intersected_cube(const vector3 &in, const vector3 &in, const vector3 &in, const vector3 &in)", &trigonometry::geometric::has_line_intersected_cube);
				vm->set_function<bool(const trigonometry::vector3&, const trigonometry::vector3&, const trigonometry::vector3&, int)>("bool has_point_intersected_cube(const vector3 &in, const vector3 &in, const vector3 &in, int32)", &trigonometry::geometric::has_point_intersected_cube);
				vm->set_function("bool has_point_intersected_rectangle(const vector3 &in, const vector3 &in, const vector3 &in)", &trigonometry::geometric::has_point_intersected_rectangle);
				vm->set_function<bool(const trigonometry::vector3&, const trigonometry::vector3&, const trigonometry::vector3&)>("bool has_point_intersected_cube(const vector3 &in, const vector3 &in, const vector3 &in)", &trigonometry::geometric::has_point_intersected_cube);
				vm->set_function("bool has_sb_intersected(transform@+, transform@+)", &trigonometry::geometric::has_sb_intersected);
				vm->set_function("bool has_obb_intersected(transform@+, transform@+)", &trigonometry::geometric::has_obb_intersected);
				vm->set_function("bool has_aabb_intersected(transform@+, transform@+)", &trigonometry::geometric::has_aabb_intersected);
				vm->set_function("void matrix_rh_to_lh(matrix4x4 &out)", &trigonometry::geometric::matrix_rh_to_lh);
				vm->set_function("void set_left_handed(bool)", &trigonometry::geometric::set_left_handed);
				vm->set_function("ray create_cursor_ray(const vector3 &in, const vector2 &in, const vector2 &in, const matrix4x4 &in, const matrix4x4 &in)", &trigonometry::geometric::create_cursor_ray);
				vm->set_function<bool(const trigonometry::ray&, const trigonometry::vector3&, const trigonometry::vector3&, trigonometry::vector3*)>("bool cursor_ray_test(const ray &in, const vector3 &in, const vector3 &in, vector3 &out)", &trigonometry::geometric::cursor_ray_test);
				vm->set_function<bool(const trigonometry::ray&, const trigonometry::matrix4x4&, trigonometry::vector3*)>("bool cursor_ray_test(const ray &in, const matrix4x4 &in, vector3 &out)", &trigonometry::geometric::cursor_ray_test);
				vm->set_function("float fast_inv_sqrt(float)", &trigonometry::geometric::fast_inv_sqrt);
				vm->set_function("float fast_sqrt(float)", &trigonometry::geometric::fast_sqrt);
				vm->set_function("float aabb_volume(const vector3 &in, const vector3 &in)", &trigonometry::geometric::aabb_volume);
				vm->set_function("float angle_distance(float, float)", &trigonometry::geometric::angle_distance);
				vm->set_function("float angle_lerp(float, float, float)", &trigonometry::geometric::angluar_lerp);
				vm->end_namespace();

				return true;
#else
				VI_ASSERT(false, "<trigonometry> is not loaded");
				return false;
#endif
			}
			bool heavy_registry::import_activity(virtual_machine* vm) noexcept
			{
#ifdef VI_BINDINGS
				VI_ASSERT(vm != nullptr, "manager should be set");
				auto vapp_state = vm->set_enum("app_state");
				vapp_state->set_value("close", (int)graphics::app_state::close);
				vapp_state->set_value("terminating", (int)graphics::app_state::terminating);
				vapp_state->set_value("low_memory", (int)graphics::app_state::low_memory);
				vapp_state->set_value("enter_background_start", (int)graphics::app_state::enter_background_start);
				vapp_state->set_value("enter_background_end", (int)graphics::app_state::enter_background_end);
				vapp_state->set_value("enter_foreground_start", (int)graphics::app_state::enter_foreground_start);
				vapp_state->set_value("enter_foreground_end", (int)graphics::app_state::enter_foreground_end);

				auto vwindow_state = vm->set_enum("window_state");
				vwindow_state->set_value("show", (int)graphics::window_state::show);
				vwindow_state->set_value("hide", (int)graphics::window_state::hide);
				vwindow_state->set_value("expose", (int)graphics::window_state::expose);
				vwindow_state->set_value("move", (int)graphics::window_state::move);
				vwindow_state->set_value("resize", (int)graphics::window_state::resize);
				vwindow_state->set_value("size_change", (int)graphics::window_state::size_change);
				vwindow_state->set_value("minimize", (int)graphics::window_state::minimize);
				vwindow_state->set_value("maximize", (int)graphics::window_state::maximize);
				vwindow_state->set_value("restore", (int)graphics::window_state::restore);
				vwindow_state->set_value("enter", (int)graphics::window_state::enter);
				vwindow_state->set_value("leave", (int)graphics::window_state::leave);
				vwindow_state->set_value("focus", (int)graphics::window_state::focus);
				vwindow_state->set_value("blur", (int)graphics::window_state::blur);
				vwindow_state->set_value("close", (int)graphics::window_state::close);

				auto vkey_code = vm->set_enum("key_code");
				vkey_code->set_value("a", (int)graphics::key_code::a);
				vkey_code->set_value("b", (int)graphics::key_code::b);
				vkey_code->set_value("c", (int)graphics::key_code::c);
				vkey_code->set_value("d", (int)graphics::key_code::d);
				vkey_code->set_value("e", (int)graphics::key_code::e);
				vkey_code->set_value("f", (int)graphics::key_code::f);
				vkey_code->set_value("g", (int)graphics::key_code::g);
				vkey_code->set_value("h", (int)graphics::key_code::h);
				vkey_code->set_value("i", (int)graphics::key_code::i);
				vkey_code->set_value("j", (int)graphics::key_code::j);
				vkey_code->set_value("k", (int)graphics::key_code::k);
				vkey_code->set_value("l", (int)graphics::key_code::l);
				vkey_code->set_value("m", (int)graphics::key_code::m);
				vkey_code->set_value("n", (int)graphics::key_code::n);
				vkey_code->set_value("o", (int)graphics::key_code::o);
				vkey_code->set_value("p", (int)graphics::key_code::p);
				vkey_code->set_value("q", (int)graphics::key_code::q);
				vkey_code->set_value("r", (int)graphics::key_code::r);
				vkey_code->set_value("s", (int)graphics::key_code::s);
				vkey_code->set_value("t", (int)graphics::key_code::t);
				vkey_code->set_value("u", (int)graphics::key_code::u);
				vkey_code->set_value("v", (int)graphics::key_code::v);
				vkey_code->set_value("w", (int)graphics::key_code::w);
				vkey_code->set_value("x", (int)graphics::key_code::x);
				vkey_code->set_value("y", (int)graphics::key_code::y);
				vkey_code->set_value("z", (int)graphics::key_code::z);
				vkey_code->set_value("d1", (int)graphics::key_code::d1);
				vkey_code->set_value("d2", (int)graphics::key_code::d2);
				vkey_code->set_value("d3", (int)graphics::key_code::d3);
				vkey_code->set_value("d4", (int)graphics::key_code::d4);
				vkey_code->set_value("d5", (int)graphics::key_code::d5);
				vkey_code->set_value("d6", (int)graphics::key_code::d6);
				vkey_code->set_value("d7", (int)graphics::key_code::d7);
				vkey_code->set_value("d8", (int)graphics::key_code::d8);
				vkey_code->set_value("d9", (int)graphics::key_code::d9);
				vkey_code->set_value("d0", (int)graphics::key_code::d0);
				vkey_code->set_value("returns", (int)graphics::key_code::defer);
				vkey_code->set_value("escape", (int)graphics::key_code::escape);
				vkey_code->set_value("backspace", (int)graphics::key_code::backspace);
				vkey_code->set_value("tab", (int)graphics::key_code::tab);
				vkey_code->set_value("space", (int)graphics::key_code::space);
				vkey_code->set_value("minus", (int)graphics::key_code::minus);
				vkey_code->set_value("equals", (int)graphics::key_code::equals);
				vkey_code->set_value("left_bracket", (int)graphics::key_code::left_bracket);
				vkey_code->set_value("right_bracket", (int)graphics::key_code::right_bracket);
				vkey_code->set_value("backslash", (int)graphics::key_code::backslash);
				vkey_code->set_value("non_us_hash", (int)graphics::key_code::non_us_hash);
				vkey_code->set_value("semicolon", (int)graphics::key_code::semicolon);
				vkey_code->set_value("apostrophe", (int)graphics::key_code::apostrophe);
				vkey_code->set_value("grave", (int)graphics::key_code::grave);
				vkey_code->set_value("comma", (int)graphics::key_code::comma);
				vkey_code->set_value("period", (int)graphics::key_code::period);
				vkey_code->set_value("slash", (int)graphics::key_code::slash);
				vkey_code->set_value("capslock", (int)graphics::key_code::capslock);
				vkey_code->set_value("f1", (int)graphics::key_code::f1);
				vkey_code->set_value("f2", (int)graphics::key_code::f2);
				vkey_code->set_value("f3", (int)graphics::key_code::f3);
				vkey_code->set_value("f4", (int)graphics::key_code::f4);
				vkey_code->set_value("f5", (int)graphics::key_code::f5);
				vkey_code->set_value("f6", (int)graphics::key_code::f6);
				vkey_code->set_value("f7", (int)graphics::key_code::f7);
				vkey_code->set_value("f8", (int)graphics::key_code::f8);
				vkey_code->set_value("f9", (int)graphics::key_code::f9);
				vkey_code->set_value("f10", (int)graphics::key_code::f10);
				vkey_code->set_value("f11", (int)graphics::key_code::f11);
				vkey_code->set_value("f12", (int)graphics::key_code::f12);
				vkey_code->set_value("print_screen", (int)graphics::key_code::print_screen);
				vkey_code->set_value("scroll_lock", (int)graphics::key_code::scroll_lock);
				vkey_code->set_value("pause", (int)graphics::key_code::pause);
				vkey_code->set_value("insert", (int)graphics::key_code::insert);
				vkey_code->set_value("home", (int)graphics::key_code::home);
				vkey_code->set_value("page_up", (int)graphics::key_code::page_up);
				vkey_code->set_value("delete", (int)graphics::key_code::deinit);
				vkey_code->set_value("end", (int)graphics::key_code::end);
				vkey_code->set_value("page_down", (int)graphics::key_code::page_down);
				vkey_code->set_value("right", (int)graphics::key_code::right);
				vkey_code->set_value("left", (int)graphics::key_code::left);
				vkey_code->set_value("down", (int)graphics::key_code::down);
				vkey_code->set_value("up", (int)graphics::key_code::up);
				vkey_code->set_value("num_lock_clear", (int)graphics::key_code::num_lock_clear);
				vkey_code->set_value("kp_divide", (int)graphics::key_code::kp_divide);
				vkey_code->set_value("kp_multiply", (int)graphics::key_code::kp_multiply);
				vkey_code->set_value("kp_minus", (int)graphics::key_code::kp_minus);
				vkey_code->set_value("kp_plus", (int)graphics::key_code::kp_plus);
				vkey_code->set_value("kp_enter", (int)graphics::key_code::kp_enter);
				vkey_code->set_value("kp_1", (int)graphics::key_code::kp1);
				vkey_code->set_value("kp_2", (int)graphics::key_code::kp2);
				vkey_code->set_value("kp_3", (int)graphics::key_code::kp3);
				vkey_code->set_value("kp_4", (int)graphics::key_code::kp4);
				vkey_code->set_value("kp_5", (int)graphics::key_code::kp5);
				vkey_code->set_value("kp_6", (int)graphics::key_code::kp6);
				vkey_code->set_value("kp_7", (int)graphics::key_code::kp7);
				vkey_code->set_value("kp_8", (int)graphics::key_code::kp8);
				vkey_code->set_value("kp_9", (int)graphics::key_code::kp9);
				vkey_code->set_value("kp_0", (int)graphics::key_code::kp0);
				vkey_code->set_value("kp_period", (int)graphics::key_code::kp_period);
				vkey_code->set_value("non_us_backslash", (int)graphics::key_code::non_us_backslash);
				vkey_code->set_value("app0", (int)graphics::key_code::app0);
				vkey_code->set_value("power", (int)graphics::key_code::power);
				vkey_code->set_value("kp_equals", (int)graphics::key_code::kp_equals);
				vkey_code->set_value("f13", (int)graphics::key_code::f13);
				vkey_code->set_value("f14", (int)graphics::key_code::f14);
				vkey_code->set_value("f15", (int)graphics::key_code::f15);
				vkey_code->set_value("f16", (int)graphics::key_code::f16);
				vkey_code->set_value("f17", (int)graphics::key_code::f17);
				vkey_code->set_value("f18", (int)graphics::key_code::f18);
				vkey_code->set_value("f19", (int)graphics::key_code::f19);
				vkey_code->set_value("f20", (int)graphics::key_code::f20);
				vkey_code->set_value("f21", (int)graphics::key_code::f21);
				vkey_code->set_value("f22", (int)graphics::key_code::f22);
				vkey_code->set_value("f23", (int)graphics::key_code::f23);
				vkey_code->set_value("f24", (int)graphics::key_code::f24);
				vkey_code->set_value("execute", (int)graphics::key_code::execute);
				vkey_code->set_value("help", (int)graphics::key_code::help);
				vkey_code->set_value("menu", (int)graphics::key_code::menu);
				vkey_code->set_value("select", (int)graphics::key_code::select);
				vkey_code->set_value("stop", (int)graphics::key_code::stop);
				vkey_code->set_value("again", (int)graphics::key_code::again);
				vkey_code->set_value("undo", (int)graphics::key_code::undo);
				vkey_code->set_value("cut", (int)graphics::key_code::cut);
				vkey_code->set_value("copy", (int)graphics::key_code::copy);
				vkey_code->set_value("paste", (int)graphics::key_code::paste);
				vkey_code->set_value("find", (int)graphics::key_code::find);
				vkey_code->set_value("mute", (int)graphics::key_code::mute);
				vkey_code->set_value("volume_up", (int)graphics::key_code::volume_up);
				vkey_code->set_value("volume_down", (int)graphics::key_code::volume_down);
				vkey_code->set_value("kp_comma", (int)graphics::key_code::kp_comma);
				vkey_code->set_value("kp_equals_as_400", (int)graphics::key_code::kp_equals_as400);
				vkey_code->set_value("international1", (int)graphics::key_code::international1);
				vkey_code->set_value("international2", (int)graphics::key_code::international2);
				vkey_code->set_value("international3", (int)graphics::key_code::international3);
				vkey_code->set_value("international4", (int)graphics::key_code::international4);
				vkey_code->set_value("international5", (int)graphics::key_code::international5);
				vkey_code->set_value("international6", (int)graphics::key_code::international6);
				vkey_code->set_value("international7", (int)graphics::key_code::international7);
				vkey_code->set_value("international8", (int)graphics::key_code::international8);
				vkey_code->set_value("international9", (int)graphics::key_code::international9);
				vkey_code->set_value("lang1", (int)graphics::key_code::lang1);
				vkey_code->set_value("lang2", (int)graphics::key_code::lang2);
				vkey_code->set_value("lang3", (int)graphics::key_code::lang3);
				vkey_code->set_value("lang4", (int)graphics::key_code::lang4);
				vkey_code->set_value("lang5", (int)graphics::key_code::lang5);
				vkey_code->set_value("lang6", (int)graphics::key_code::lang6);
				vkey_code->set_value("lang7", (int)graphics::key_code::lang7);
				vkey_code->set_value("lang8", (int)graphics::key_code::lang8);
				vkey_code->set_value("lang9", (int)graphics::key_code::lang9);
				vkey_code->set_value("alterase", (int)graphics::key_code::alterase);
				vkey_code->set_value("sys_req", (int)graphics::key_code::sys_req);
				vkey_code->set_value("cancel", (int)graphics::key_code::cancel);
				vkey_code->set_value("clear", (int)graphics::key_code::clear);
				vkey_code->set_value("prior", (int)graphics::key_code::prior);
				vkey_code->set_value("return2", (int)graphics::key_code::return2);
				vkey_code->set_value("separator", (int)graphics::key_code::separator);
				vkey_code->set_value("output", (int)graphics::key_code::output);
				vkey_code->set_value("operation", (int)graphics::key_code::operation);
				vkey_code->set_value("clear_again", (int)graphics::key_code::clear_again);
				vkey_code->set_value("cr_select", (int)graphics::key_code::cr_select);
				vkey_code->set_value("ex_select", (int)graphics::key_code::ex_select);
				vkey_code->set_value("kp_00", (int)graphics::key_code::kp00);
				vkey_code->set_value("kp_000", (int)graphics::key_code::kp000);
				vkey_code->set_value("thousands_separator", (int)graphics::key_code::thousands_separator);
				vkey_code->set_value("decimals_separator", (int)graphics::key_code::decimals_separator);
				vkey_code->set_value("currency_unit", (int)graphics::key_code::currency_unit);
				vkey_code->set_value("currency_subunit", (int)graphics::key_code::currency_subunit);
				vkey_code->set_value("kp_left_paren", (int)graphics::key_code::kp_left_paren);
				vkey_code->set_value("kp_right_paren", (int)graphics::key_code::kp_right_paren);
				vkey_code->set_value("kp_left_brace", (int)graphics::key_code::kp_left_brace);
				vkey_code->set_value("kp_right_brace", (int)graphics::key_code::kp_right_brace);
				vkey_code->set_value("kp_tab", (int)graphics::key_code::kp_tab);
				vkey_code->set_value("kp_backspace", (int)graphics::key_code::kp_backspace);
				vkey_code->set_value("kp_a", (int)graphics::key_code::kp_a);
				vkey_code->set_value("kp_b", (int)graphics::key_code::kp_b);
				vkey_code->set_value("kp_c", (int)graphics::key_code::kp_c);
				vkey_code->set_value("kp_d", (int)graphics::key_code::kp_d);
				vkey_code->set_value("kp_e", (int)graphics::key_code::kp_e);
				vkey_code->set_value("kp_f", (int)graphics::key_code::kp_f);
				vkey_code->set_value("kp_xor", (int)graphics::key_code::kp_xor);
				vkey_code->set_value("kp_power", (int)graphics::key_code::kp_power);
				vkey_code->set_value("kp_percent", (int)graphics::key_code::kp_percent);
				vkey_code->set_value("kp_less", (int)graphics::key_code::kp_less);
				vkey_code->set_value("kp_greater", (int)graphics::key_code::kp_greater);
				vkey_code->set_value("kp_ampersand", (int)graphics::key_code::kp_ampersand);
				vkey_code->set_value("kp_dbl_ampersand", (int)graphics::key_code::kp_dbl_ampersand);
				vkey_code->set_value("kp_vertical_bar", (int)graphics::key_code::kp_vertical_bar);
				vkey_code->set_value("kp_dbl_vertical_bar", (int)graphics::key_code::kp_dbl_vertical_bar);
				vkey_code->set_value("kp_colon", (int)graphics::key_code::kp_colon);
				vkey_code->set_value("kp_hash", (int)graphics::key_code::kp_hash);
				vkey_code->set_value("kp_space", (int)graphics::key_code::kp_space);
				vkey_code->set_value("kp_at", (int)graphics::key_code::kp_at);
				vkey_code->set_value("kp_exclaim", (int)graphics::key_code::kp_exclaim);
				vkey_code->set_value("kp_mem_store", (int)graphics::key_code::kp_mem_store);
				vkey_code->set_value("kp_mem_recall", (int)graphics::key_code::kp_mem_recall);
				vkey_code->set_value("kp_mem_clear", (int)graphics::key_code::kp_mem_clear);
				vkey_code->set_value("kp_mem_add", (int)graphics::key_code::kp_mem_add);
				vkey_code->set_value("kp_mem_subtract", (int)graphics::key_code::kp_mem_subtract);
				vkey_code->set_value("kp_mem_multiply", (int)graphics::key_code::kp_mem_multiply);
				vkey_code->set_value("kp_mem_divide", (int)graphics::key_code::kp_mem_divide);
				vkey_code->set_value("kp_plus_minus", (int)graphics::key_code::kp_plus_minus);
				vkey_code->set_value("kp_clear", (int)graphics::key_code::kp_clear);
				vkey_code->set_value("kp_clear_entry", (int)graphics::key_code::kp_clear_entry);
				vkey_code->set_value("kp_binary", (int)graphics::key_code::kp_binary);
				vkey_code->set_value("kp_octal", (int)graphics::key_code::kp_octal);
				vkey_code->set_value("kp_decimal", (int)graphics::key_code::kp_decimal);
				vkey_code->set_value("kp_hexadecimal", (int)graphics::key_code::kp_hexadecimal);
				vkey_code->set_value("left_control", (int)graphics::key_code::left_control);
				vkey_code->set_value("left_shift", (int)graphics::key_code::left_shift);
				vkey_code->set_value("left_alt", (int)graphics::key_code::left_alt);
				vkey_code->set_value("left_gui", (int)graphics::key_code::left_gui);
				vkey_code->set_value("right_control", (int)graphics::key_code::right_control);
				vkey_code->set_value("right_shift", (int)graphics::key_code::right_shift);
				vkey_code->set_value("right_alt", (int)graphics::key_code::right_alt);
				vkey_code->set_value("right_gui", (int)graphics::key_code::right_gui);
				vkey_code->set_value("mode", (int)graphics::key_code::mode);
				vkey_code->set_value("audio_next", (int)graphics::key_code::audio_next);
				vkey_code->set_value("audio_prev", (int)graphics::key_code::audio_prev);
				vkey_code->set_value("audio_stop", (int)graphics::key_code::audio_stop);
				vkey_code->set_value("audio_play", (int)graphics::key_code::audio_play);
				vkey_code->set_value("audio_mute", (int)graphics::key_code::audio_mute);
				vkey_code->set_value("media_select", (int)graphics::key_code::media_select);
				vkey_code->set_value("www", (int)graphics::key_code::www);
				vkey_code->set_value("mail", (int)graphics::key_code::mail);
				vkey_code->set_value("calculator", (int)graphics::key_code::calculator);
				vkey_code->set_value("computer", (int)graphics::key_code::computer);
				vkey_code->set_value("ac_search", (int)graphics::key_code::ac_search);
				vkey_code->set_value("ac_home", (int)graphics::key_code::ac_home);
				vkey_code->set_value("ac_back", (int)graphics::key_code::ac_back);
				vkey_code->set_value("ac_forward", (int)graphics::key_code::ac_forward);
				vkey_code->set_value("ac_stop", (int)graphics::key_code::ac_stop);
				vkey_code->set_value("ac_refresh", (int)graphics::key_code::ac_refresh);
				vkey_code->set_value("ac_bookmarks", (int)graphics::key_code::ac_bookmarks);
				vkey_code->set_value("brightness_down", (int)graphics::key_code::brightness_down);
				vkey_code->set_value("brightness_up", (int)graphics::key_code::brightness_up);
				vkey_code->set_value("display_switch", (int)graphics::key_code::display_switch);
				vkey_code->set_value("kb_illum_toggle", (int)graphics::key_code::kb_illum_toggle);
				vkey_code->set_value("kb_illum_down", (int)graphics::key_code::kb_illum_down);
				vkey_code->set_value("kb_illum_up", (int)graphics::key_code::kb_illum_up);
				vkey_code->set_value("eject", (int)graphics::key_code::eject);
				vkey_code->set_value("sleep", (int)graphics::key_code::sleep);
				vkey_code->set_value("app1", (int)graphics::key_code::app1);
				vkey_code->set_value("app2", (int)graphics::key_code::app2);
				vkey_code->set_value("audio_rewind", (int)graphics::key_code::audio_rewind);
				vkey_code->set_value("audio_fast_forward", (int)graphics::key_code::audio_fast_forward);
				vkey_code->set_value("cursor_left", (int)graphics::key_code::cursor_left);
				vkey_code->set_value("cursor_middle", (int)graphics::key_code::cursor_middle);
				vkey_code->set_value("cursor_right", (int)graphics::key_code::cursor_right);
				vkey_code->set_value("cursor_x1", (int)graphics::key_code::cursor_x1);
				vkey_code->set_value("cursor_x2", (int)graphics::key_code::cursor_x2);
				vkey_code->set_value("none", (int)graphics::key_code::none);

				auto vkey_mod = vm->set_enum("key_mod");
				vkey_mod->set_value("none", (int)graphics::key_mod::none);
				vkey_mod->set_value("left_shift", (int)graphics::key_mod::left_shift);
				vkey_mod->set_value("right_shift", (int)graphics::key_mod::right_shift);
				vkey_mod->set_value("left_control", (int)graphics::key_mod::left_control);
				vkey_mod->set_value("right_control", (int)graphics::key_mod::right_control);
				vkey_mod->set_value("left_alt", (int)graphics::key_mod::left_alt);
				vkey_mod->set_value("right_alt", (int)graphics::key_mod::right_alt);
				vkey_mod->set_value("left_gui", (int)graphics::key_mod::left_gui);
				vkey_mod->set_value("right_gui", (int)graphics::key_mod::right_gui);
				vkey_mod->set_value("num", (int)graphics::key_mod::num);
				vkey_mod->set_value("caps", (int)graphics::key_mod::caps);
				vkey_mod->set_value("mode", (int)graphics::key_mod::mode);
				vkey_mod->set_value("reserved", (int)graphics::key_mod::reserved);
				vkey_mod->set_value("shift", (int)graphics::key_mod::shift);
				vkey_mod->set_value("control", (int)graphics::key_mod::control);
				vkey_mod->set_value("alt", (int)graphics::key_mod::alt);
				vkey_mod->set_value("gui", (int)graphics::key_mod::gui);

				auto valert_type = vm->set_enum("alert_type");
				valert_type->set_value("none", (int)graphics::alert_type::none);
				valert_type->set_value("error", (int)graphics::alert_type::error);
				valert_type->set_value("warning", (int)graphics::alert_type::warning);
				valert_type->set_value("info", (int)graphics::alert_type::info);

				auto valert_confirm = vm->set_enum("alert_confirm");
				valert_confirm->set_value("none", (int)graphics::alert_confirm::none);
				valert_confirm->set_value("returns", (int)graphics::alert_confirm::defer);
				valert_confirm->set_value("escape", (int)graphics::alert_confirm::escape);

				auto vjoy_stick_hat = vm->set_enum("joy_stick_hat");
				vjoy_stick_hat->set_value("center", (int)graphics::joy_stick_hat::center);
				vjoy_stick_hat->set_value("up", (int)graphics::joy_stick_hat::up);
				vjoy_stick_hat->set_value("right", (int)graphics::joy_stick_hat::right);
				vjoy_stick_hat->set_value("down", (int)graphics::joy_stick_hat::down);
				vjoy_stick_hat->set_value("left", (int)graphics::joy_stick_hat::left);
				vjoy_stick_hat->set_value("right_up", (int)graphics::joy_stick_hat::right_up);
				vjoy_stick_hat->set_value("right_down", (int)graphics::joy_stick_hat::right_down);
				vjoy_stick_hat->set_value("left_up", (int)graphics::joy_stick_hat::left_up);
				vjoy_stick_hat->set_value("left_down", (int)graphics::joy_stick_hat::left_down);

				auto vrender_backend = vm->set_enum("render_backend");
				vrender_backend->set_value("none", (int)graphics::render_backend::none);
				vrender_backend->set_value("automatic", (int)graphics::render_backend::automatic);
				vrender_backend->set_value("d3d11", (int)graphics::render_backend::d3d11);
				vrender_backend->set_value("ogl", (int)graphics::render_backend::ogl);

				auto vdisplay_cursor = vm->set_enum("display_cursor");
				vdisplay_cursor->set_value("none", (int)graphics::display_cursor::none);
				vdisplay_cursor->set_value("arrow", (int)graphics::display_cursor::arrow);
				vdisplay_cursor->set_value("text_input", (int)graphics::display_cursor::text_input);
				vdisplay_cursor->set_value("resize_all", (int)graphics::display_cursor::resize_all);
				vdisplay_cursor->set_value("resize_ns", (int)graphics::display_cursor::resize_ns);
				vdisplay_cursor->set_value("resize_ew", (int)graphics::display_cursor::resize_ew);
				vdisplay_cursor->set_value("resize_nesw", (int)graphics::display_cursor::resize_nesw);
				vdisplay_cursor->set_value("resize_nwse", (int)graphics::display_cursor::resize_nwse);
				vdisplay_cursor->set_value("hand", (int)graphics::display_cursor::hand);
				vdisplay_cursor->set_value("crosshair", (int)graphics::display_cursor::crosshair);
				vdisplay_cursor->set_value("wait", (int)graphics::display_cursor::wait);
				vdisplay_cursor->set_value("progress", (int)graphics::display_cursor::progress);
				vdisplay_cursor->set_value("no", (int)graphics::display_cursor::no);

				auto vorientation_type = vm->set_enum("orientation_type");
				vorientation_type->set_value("unknown", (int)graphics::orientation_type::unknown);
				vorientation_type->set_value("landscape", (int)graphics::orientation_type::landscape);
				vorientation_type->set_value("landscape_flipped", (int)graphics::orientation_type::landscape_flipped);
				vorientation_type->set_value("portrait", (int)graphics::orientation_type::portrait);
				vorientation_type->set_value("portrait_flipped", (int)graphics::orientation_type::portrait_flipped);

				auto vkey_map = vm->set_pod<graphics::key_map>("key_map");
				vkey_map->set_property<graphics::key_map>("key_code key", &graphics::key_map::key);
				vkey_map->set_property<graphics::key_map>("key_mod mod", &graphics::key_map::mod);
				vkey_map->set_property<graphics::key_map>("bool normal", &graphics::key_map::normal);
				vkey_map->set_constructor<graphics::key_map>("void f()");
				vkey_map->set_constructor<graphics::key_map, const graphics::key_code&>("void f(const key_code &in)");
				vkey_map->set_constructor<graphics::key_map, const graphics::key_mod&>("void f(const key_mod &in)");
				vkey_map->set_constructor<graphics::key_map, const graphics::key_code&, const graphics::key_mod&>("void f(const key_code &in, const key_mod &in)");

				auto vviewport = vm->set_pod<graphics::viewport>("viewport");
				vviewport->set_property<graphics::viewport>("float top_left_x", &graphics::viewport::top_left_x);
				vviewport->set_property<graphics::viewport>("float top_left_y", &graphics::viewport::top_left_y);
				vviewport->set_property<graphics::viewport>("float width", &graphics::viewport::width);
				vviewport->set_property<graphics::viewport>("float height", &graphics::viewport::height);
				vviewport->set_property<graphics::viewport>("float min_depth", &graphics::viewport::min_depth);
				vviewport->set_property<graphics::viewport>("float max_depth", &graphics::viewport::max_depth);
				vviewport->set_constructor<graphics::viewport>("void f()");

				auto vdisplay_info = vm->set_pod<graphics::display_info>("display_info");
				vdisplay_info->set_property<graphics::display_info>("string name", &graphics::display_info::name);
				vdisplay_info->set_property<graphics::display_info>("orientation_type orientation", &graphics::display_info::orientation);
				vdisplay_info->set_property<graphics::display_info>("float diagonal_dpi", &graphics::display_info::diagonal_dpi);
				vdisplay_info->set_property<graphics::display_info>("float horizontal_dpi", &graphics::display_info::horizontal_dpi);
				vdisplay_info->set_property<graphics::display_info>("float vertical_dpi", &graphics::display_info::vertical_dpi);
				vdisplay_info->set_property<graphics::display_info>("uint32 pixel_format", &graphics::display_info::pixel_format);
				vdisplay_info->set_property<graphics::display_info>("uint32 physical_width", &graphics::display_info::physical_width);
				vdisplay_info->set_property<graphics::display_info>("uint32 physical_height", &graphics::display_info::physical_height);
				vdisplay_info->set_property<graphics::display_info>("uint32 refresh_rate", &graphics::display_info::refresh_rate);
				vdisplay_info->set_property<graphics::display_info>("int32 width", &graphics::display_info::width);
				vdisplay_info->set_property<graphics::display_info>("int32 height", &graphics::display_info::height);
				vdisplay_info->set_property<graphics::display_info>("int32 x", &graphics::display_info::x);
				vdisplay_info->set_property<graphics::display_info>("int32 y", &graphics::display_info::y);
				vdisplay_info->set_constructor<graphics::display_info>("void f()");

				auto vactivity = vm->set_class<graphics::activity>("activity", false);
				auto valert = vm->set_struct_trivial<graphics::alert>("activity_alert");
				valert->set_function_def("void alert_sync(int)");
				valert->set_constructor<graphics::alert, graphics::activity*>("void f(activity@+)");
				valert->set_method("void setup(alert_type, const string_view&in, const string_view&in)", &graphics::alert::setup);
				valert->set_method("void button(alert_confirm, const string_view&in, int32)", &graphics::alert::button);
				valert->set_method_ex("void result(alert_sync@)", &alert_result);

				auto vevent_consumers = vm->set_struct_trivial<graphics::event_consumers>("activity_event_consumers");
				vevent_consumers->set_constructor<graphics::event_consumers>("void f()");
				vevent_consumers->set_method("void push(activity@+)", &graphics::event_consumers::push);
				vevent_consumers->set_method("void pop(activity@+)", &graphics::event_consumers::pop);
				vevent_consumers->set_method("activity@+ find(uint32) const", &graphics::event_consumers::find);

				auto vsurface = vm->set_class<graphics::surface>("activity_surface", false);
				vsurface->set_constructor<graphics::surface>("activity_surface@ f()");
				vsurface->set_constructor<graphics::surface, SDL_Surface*>("activity_surface@ f(uptr@)");
				vsurface->set_method("void set_handle(uptr@)", &graphics::surface::set_handle);
				vsurface->set_method("void lock()", &graphics::surface::lock);
				vsurface->set_method("void unlock()", &graphics::surface::unlock);
				vsurface->set_method("int get_width()", &graphics::surface::get_width);
				vsurface->set_method("int get_height()", &graphics::surface::get_height);
				vsurface->set_method("int get_pitch()", &graphics::surface::get_pitch);
				vsurface->set_method("uptr@ get_pixels()", &graphics::surface::get_pixels);
				vsurface->set_method("uptr@ get_resource()", &graphics::surface::get_resource);

				auto vactivity_desc = vm->set_struct_trivial<graphics::activity::desc>("activity_desc");
				vactivity_desc->set_property<graphics::activity::desc>("string title", &graphics::activity::desc::title);
				vactivity_desc->set_property<graphics::activity::desc>("uint32 inactive_sleep_ms", &graphics::activity::desc::inactive_sleep_ms);
				vactivity_desc->set_property<graphics::activity::desc>("uint32 width", &graphics::activity::desc::width);
				vactivity_desc->set_property<graphics::activity::desc>("uint32 height", &graphics::activity::desc::height);
				vactivity_desc->set_property<graphics::activity::desc>("int32 x", &graphics::activity::desc::x);
				vactivity_desc->set_property<graphics::activity::desc>("int32 y", &graphics::activity::desc::y);
				vactivity_desc->set_property<graphics::activity::desc>("bool fullscreen", &graphics::activity::desc::fullscreen);
				vactivity_desc->set_property<graphics::activity::desc>("bool hidden", &graphics::activity::desc::hidden);
				vactivity_desc->set_property<graphics::activity::desc>("bool borderless", &graphics::activity::desc::borderless);
				vactivity_desc->set_property<graphics::activity::desc>("bool resizable", &graphics::activity::desc::resizable);
				vactivity_desc->set_property<graphics::activity::desc>("bool minimized", &graphics::activity::desc::minimized);
				vactivity_desc->set_property<graphics::activity::desc>("bool maximized", &graphics::activity::desc::maximized);
				vactivity_desc->set_property<graphics::activity::desc>("bool centered", &graphics::activity::desc::centered);
				vactivity_desc->set_property<graphics::activity::desc>("bool free_position", &graphics::activity::desc::free_position);
				vactivity_desc->set_property<graphics::activity::desc>("bool focused", &graphics::activity::desc::focused);
				vactivity_desc->set_property<graphics::activity::desc>("bool render_even_if_inactive", &graphics::activity::desc::render_even_if_inactive);
				vactivity_desc->set_property<graphics::activity::desc>("bool gpu_as_renderer", &graphics::activity::desc::gpu_as_renderer);
				vactivity_desc->set_property<graphics::activity::desc>("bool high_dpi", &graphics::activity::desc::high_dpi);
				vactivity_desc->set_constructor<graphics::activity::desc>("void f()");

				vactivity->set_property<graphics::activity>("activity_alert message", &graphics::activity::message);
				vactivity->set_constructor<graphics::activity, const graphics::activity::desc&>("activity@ f(const activity_desc &in)");
				vactivity->set_function_def("void app_state_change_sync(app_state)");
				vactivity->set_function_def("void window_state_change_sync(window_state, int, int)");
				vactivity->set_function_def("void key_state_sync(key_code, key_mod, int, int, bool)");
				vactivity->set_function_def("void input_edit_sync(const string_view&in, int, int)");
				vactivity->set_function_def("void input_sync(const string_view&in, int)");
				vactivity->set_function_def("void cursor_move_sync(int, int, int, int)");
				vactivity->set_function_def("void cursor_wheel_state_sync(int, int, bool)");
				vactivity->set_function_def("void joy_stick_axis_move_sync(int, int, int)");
				vactivity->set_function_def("void joy_stick_ball_move_sync(int, int, int, int)");
				vactivity->set_function_def("void joy_stick_hat_move_sync(joy_stick_hat, int, int)");
				vactivity->set_function_def("void joy_stick_key_state_sync(int, int, bool)");
				vactivity->set_function_def("void joy_stick_state_sync(int, bool)");
				vactivity->set_function_def("void controller_axis_move_sync(int, int, int)");
				vactivity->set_function_def("void controller_key_state_sync(int, int, bool)");
				vactivity->set_function_def("void controller_state_sync(int, int)");
				vactivity->set_function_def("void touch_move_sync(int, int, float, float, float, float, float)");
				vactivity->set_function_def("void touch_state_sync(int, int, float, float, float, float, float, bool)");
				vactivity->set_function_def("void gesture_state_sync(int, int, int, float, float, float, bool)");
				vactivity->set_function_def("void multi_gesture_state_sync(int, int, float, float, float, float)");
				vactivity->set_function_def("void drop_file_sync(const string_view&in)");
				vactivity->set_function_def("void drop_text_sync(const string_view&in)");
				vactivity->set_method_ex("void set_app_state_change(app_state_change_sync@)", &activity_set_app_state_change);
				vactivity->set_method_ex("void set_window_state_change(window_state_change_sync@)", &activity_set_window_state_change);
				vactivity->set_method_ex("void set_key_state(key_state_sync@)", &activity_set_key_state);
				vactivity->set_method_ex("void set_input_edit(input_edit_sync@)", &activity_set_input_edit);
				vactivity->set_method_ex("void set_input(input_sync@)", &activity_set_input);
				vactivity->set_method_ex("void set_cursor_move(cursor_move_sync@)", &activity_set_cursor_move);
				vactivity->set_method_ex("void set_cursor_wheel_state(cursor_wheel_state_sync@)", &activity_set_cursor_wheel_state);
				vactivity->set_method_ex("void set_joy_stick_axis_move(joy_stick_axis_move_sync@)", &activity_set_joy_stick_axis_move);
				vactivity->set_method_ex("void set_joy_stick_ball_move(joy_stick_ball_move_sync@)", &activity_set_joy_stick_ball_move);
				vactivity->set_method_ex("void set_joy_stick_hat_move(joy_stick_hat_move_sync@)", &activity_set_joy_stick_hat_move);
				vactivity->set_method_ex("void set_joy_stickKeyState(joy_stick_key_state_sync@)", &activity_set_joy_stick_key_state);
				vactivity->set_method_ex("void set_joy_stickState(joy_stick_state_sync@)", &activity_set_joy_stick_state);
				vactivity->set_method_ex("void set_controller_axis_move(controller_axis_move_sync@)", &activity_set_controller_axis_move);
				vactivity->set_method_ex("void set_controller_key_state(controller_key_state_sync@)", &activity_set_controller_key_state);
				vactivity->set_method_ex("void set_controller_state(controller_state_sync@)", &activity_set_controller_state);
				vactivity->set_method_ex("void set_touch_move(touch_move_sync@)", &activity_set_touch_move);
				vactivity->set_method_ex("void set_touch_state(touch_state_sync@)", &activity_set_touch_state);
				vactivity->set_method_ex("void set_gesture_state(gesture_state_sync@)", &activity_set_gesture_state);
				vactivity->set_method_ex("void set_multi_gesture_state(multi_gesture_state_sync@)", &activity_set_multi_gesture_state);
				vactivity->set_method_ex("void set_drop_file(drop_file_sync@)", &activity_set_drop_file);
				vactivity->set_method_ex("void set_drop_text(drop_text_sync@)", &activity_set_drop_text);
				vactivity->set_method("void set_clipboard_text(const string_view&in)", &graphics::activity::set_clipboard_text);
				vactivity->set_method<graphics::activity, void, const trigonometry::vector2&>("void set_cursor_position(const vector2 &in)", &graphics::activity::set_cursor_position);
				vactivity->set_method<graphics::activity, void, float, float>("void set_cursor_position(float, float)", &graphics::activity::set_cursor_position);
				vactivity->set_method<graphics::activity, void, const trigonometry::vector2&>("void set_global_cursor_position(const vector2 &in)", &graphics::activity::set_global_cursor_position);
				vactivity->set_method<graphics::activity, void, float, float>("void set_global_cursor_position(float, float)", &graphics::activity::set_global_cursor_position);
				vactivity->set_method("void set_key(key_code, bool)", &graphics::activity::set_key);
				vactivity->set_method("void set_cursor(display_cursor)", &graphics::activity::set_cursor);
				vactivity->set_method("void set_cursor_visibility(bool)", &graphics::activity::set_cursor_visibility);
				vactivity->set_method("void set_grabbing(bool)", &graphics::activity::set_grabbing);
				vactivity->set_method("void set_fullscreen(bool)", &graphics::activity::set_fullscreen);
				vactivity->set_method("void set_borderless(bool)", &graphics::activity::set_borderless);
				vactivity->set_method("void set_icon(activity_surface@+)", &graphics::activity::set_icon);
				vactivity->set_method("void set_title(const string_view&in)", &graphics::activity::set_title);
				vactivity->set_method("void set_screen_keyboard(bool)", &graphics::activity::set_screen_keyboard);
				vactivity->set_method("void apply_configuration(render_backend)", &graphics::activity::apply_configuration);
				vactivity->set_method("void hide()", &graphics::activity::hide);
				vactivity->set_method("void show()", &graphics::activity::show);
				vactivity->set_method("void maximize()", &graphics::activity::maximize);
				vactivity->set_method("void minimize()", &graphics::activity::minimize);
				vactivity->set_method("void focus()", &graphics::activity::focus);
				vactivity->set_method("void move(int, int)", &graphics::activity::move);
				vactivity->set_method("void resize(int, int)", &graphics::activity::resize);
				vactivity->set_method("bool capture_key_map(key_map &out)", &graphics::activity::capture_key_map);
				vactivity->set_method("bool dispatch(uint64 = 0, bool = true)", &graphics::activity::dispatch);
				vactivity->set_method("bool is_fullscreen() const", &graphics::activity::is_fullscreen);
				vactivity->set_method("bool is_any_key_down() const", &graphics::activity::is_any_key_down);
				vactivity->set_method("bool is_key_down(const key_map &in) const", &graphics::activity::is_key_down);
				vactivity->set_method("bool is_key_up(const key_map &in) const", &graphics::activity::is_key_up);
				vactivity->set_method("bool is_key_down_hit(const key_map &in) const", &graphics::activity::is_key_down_hit);
				vactivity->set_method("bool is_key_up_hit(const key_map &in) const", &graphics::activity::is_key_up_hit);
				vactivity->set_method("bool is_screen_keyboard_enabled() const", &graphics::activity::is_screen_keyboard_enabled);
				vactivity->set_method("uint32 get_x() const", &graphics::activity::get_x);
				vactivity->set_method("uint32 get_y() const", &graphics::activity::get_y);
				vactivity->set_method("uint32 get_width() const", &graphics::activity::get_width);
				vactivity->set_method("uint32 get_height() const", &graphics::activity::get_height);
				vactivity->set_method("uint32 get_id() const", &graphics::activity::get_id);
				vactivity->set_method("float get_aspect_ratio() const", &graphics::activity::get_aspect_ratio);
				vactivity->set_method("key_mod get_key_mod_state() const", &graphics::activity::get_key_mod_state);
				vactivity->set_method("viewport get_viewport() const", &graphics::activity::get_viewport);
				vactivity->set_method("vector2 get_offset() const", &graphics::activity::get_offset);
				vactivity->set_method("vector2 get_size() const", &graphics::activity::get_size);
				vactivity->set_method("vector2 get_client_size() const", &graphics::activity::get_client_size);
				vactivity->set_method("vector2 get_drawable_size(uint32, uint32) const", &graphics::activity::get_drawable_size);
				vactivity->set_method("vector2 get_global_cursor_position() const", &graphics::activity::get_global_cursor_position);
				vactivity->set_method<graphics::activity, trigonometry::vector2>("vector2 get_cursor_position() const", &graphics::activity::get_cursor_position);
				vactivity->set_method<graphics::activity, trigonometry::vector2, float, float>("vector2 get_cursor_position(float, float) const", &graphics::activity::get_cursor_position);
				vactivity->set_method<graphics::activity, trigonometry::vector2, const trigonometry::vector2&>("vector2 get_cursor_position(const vector2 &in) const", &graphics::activity::get_cursor_position);
				vactivity->set_method("string get_clipboard_text() const", &graphics::activity::get_clipboard_text);
				vactivity->set_method("string get_error() const", &graphics::activity::get_error);
				vactivity->set_method("activity_desc& get_options()", &graphics::activity::get_options);
				vactivity->set_method_static("bool multi_dispatch(const activity_event_consumers&in, uint64 = 0, bool = true)", &graphics::activity::multi_dispatch);

				vm->begin_namespace("alerts");
				vm->set_function("bool text(const string_view&in, const string_view&in, const string_view&in, string &out)", &graphics::alerts::text);
				vm->set_function("bool password(const string_view&in, const string_view&in, string &out)", &graphics::alerts::password);
				vm->set_function("bool save(const string_view&in, const string_view&in, const string_view&in, const string_view&in, string &out)", &graphics::alerts::save);
				vm->set_function("bool open(const string_view&in, const string_view&in, const string_view&in, const string_view&in, bool, string &out)", &graphics::alerts::open);
				vm->set_function("bool folder(const string_view&in, const string_view&in, string &out)", &graphics::alerts::folder);
				vm->set_function("bool color(const string_view&in, const string_view&in, string &out)", &graphics::alerts::color);
				vm->end_namespace();

				vm->begin_namespace("video");
				vm->set_function("uint32 get_display_count()", &graphics::video::get_display_count);
				vm->set_function("bool get_display_info(uint32, display_info&out)", &graphics::video::get_display_info);
				vm->set_function("string get_key_code_as_string(key_code)", &graphics::video::get_key_code_as_string);
				vm->set_function("string get_key_mod_as_string(key_mod)", &graphics::video::get_key_mod_as_string);
				vm->end_namespace();

				return true;
#else
				VI_ASSERT(false, "<activity> is not loaded");
				return false;
#endif
			}
			bool heavy_registry::import_physics(virtual_machine* vm) noexcept
			{
#ifdef VI_BINDINGS
				VI_ASSERT(vm != nullptr, "manager should be set");
				auto vsimulator = vm->set_class<physics::simulator>("physics_simulator", false);

				auto vshape = vm->set_enum("physics_shape");
				vshape->set_value("box", (int)physics::shape::box);
				vshape->set_value("triangle", (int)physics::shape::triangle);
				vshape->set_value("tetrahedral", (int)physics::shape::tetrahedral);
				vshape->set_value("convex_triangle_mesh", (int)physics::shape::convex_triangle_mesh);
				vshape->set_value("convex_hull", (int)physics::shape::convex_hull);
				vshape->set_value("convex_point_cloud", (int)physics::shape::convex_point_cloud);
				vshape->set_value("convex_polyhedral", (int)physics::shape::convex_polyhedral);
				vshape->set_value("implicit_convex_start", (int)physics::shape::implicit_convex_start);
				vshape->set_value("sphere", (int)physics::shape::sphere);
				vshape->set_value("multi_sphere", (int)physics::shape::multi_sphere);
				vshape->set_value("capsule", (int)physics::shape::capsule);
				vshape->set_value("cone", (int)physics::shape::cone);
				vshape->set_value("convex", (int)physics::shape::convex);
				vshape->set_value("cylinder", (int)physics::shape::cylinder);
				vshape->set_value("uniform_scaling", (int)physics::shape::uniform_scaling);
				vshape->set_value("minkowski_sum", (int)physics::shape::minkowski_sum);
				vshape->set_value("minkowski_difference", (int)physics::shape::minkowski_difference);
				vshape->set_value("box_2d", (int)physics::shape::box_2d);
				vshape->set_value("convex_2d", (int)physics::shape::convex_2d);
				vshape->set_value("custom_convex", (int)physics::shape::custom_convex);
				vshape->set_value("concaves_start", (int)physics::shape::concaves_start);
				vshape->set_value("triangle_mesh", (int)physics::shape::triangle_mesh);
				vshape->set_value("triangle_mesh_scaled", (int)physics::shape::triangle_mesh_scaled);
				vshape->set_value("fast_concave_mesh", (int)physics::shape::fast_concave_mesh);
				vshape->set_value("terrain", (int)physics::shape::terrain);
				vshape->set_value("triangle_mesh_multimaterial", (int)physics::shape::triangle_mesh_multimaterial);
				vshape->set_value("empty", (int)physics::shape::empty);
				vshape->set_value("static_plane", (int)physics::shape::static_plane);
				vshape->set_value("custom_concave", (int)physics::shape::custom_concave);
				vshape->set_value("concaves_end", (int)physics::shape::concaves_end);
				vshape->set_value("compound", (int)physics::shape::compound);
				vshape->set_value("softbody", (int)physics::shape::softbody);
				vshape->set_value("hf_fluid", (int)physics::shape::hf_fluid);
				vshape->set_value("hf_fluid_bouyant_convex", (int)physics::shape::hf_fluid_bouyant_convex);
				vshape->set_value("invalid", (int)physics::shape::invalid);

				auto vmotion_state = vm->set_enum("physics_motion_state");
				vmotion_state->set_value("active", (int)physics::motion_state::active);
				vmotion_state->set_value("island_sleeping", (int)physics::motion_state::island_sleeping);
				vmotion_state->set_value("deactivation_needed", (int)physics::motion_state::deactivation_needed);
				vmotion_state->set_value("disable_deactivation", (int)physics::motion_state::disable_deactivation);
				vmotion_state->set_value("disable_simulation", (int)physics::motion_state::disable_simulation);

				auto vsoft_feature = vm->set_enum("physics_soft_feature");
				vsoft_feature->set_value("none", (int)physics::soft_feature::none);
				vsoft_feature->set_value("node", (int)physics::soft_feature::node);
				vsoft_feature->set_value("link", (int)physics::soft_feature::link);
				vsoft_feature->set_value("face", (int)physics::soft_feature::face);
				vsoft_feature->set_value("tetra", (int)physics::soft_feature::tetra);

				auto vsoft_aero_model = vm->set_enum("physics_soft_aero_model");
				vsoft_aero_model->set_value("vpoint", (int)physics::soft_aero_model::vpoint);
				vsoft_aero_model->set_value("vtwo_sided", (int)physics::soft_aero_model::vtwo_sided);
				vsoft_aero_model->set_value("vtwo_sided_lift_drag", (int)physics::soft_aero_model::vtwo_sided_lift_drag);
				vsoft_aero_model->set_value("vone_sided", (int)physics::soft_aero_model::vone_sided);
				vsoft_aero_model->set_value("ftwo_sided", (int)physics::soft_aero_model::ftwo_sided);
				vsoft_aero_model->set_value("ftwo_sided_lift_drag", (int)physics::soft_aero_model::ftwo_sided_lift_drag);
				vsoft_aero_model->set_value("fone_sided", (int)physics::soft_aero_model::fone_sided);

				auto vsoft_collision = vm->set_enum("physics_soft_collision");
				vsoft_collision->set_value("rvs_mask", (int)physics::soft_collision::rvs_mask);
				vsoft_collision->set_value("sdf_rs", (int)physics::soft_collision::sdf_rs);
				vsoft_collision->set_value("cl_rs", (int)physics::soft_collision::cl_rs);
				vsoft_collision->set_value("sdf_rd", (int)physics::soft_collision::sdf_rd);
				vsoft_collision->set_value("sdf_rdf", (int)physics::soft_collision::sdf_rdf);
				vsoft_collision->set_value("svs_mask", (int)physics::soft_collision::svs_mask);
				vsoft_collision->set_value("vf_ss", (int)physics::soft_collision::vf_ss);
				vsoft_collision->set_value("cl_ss", (int)physics::soft_collision::cl_ss);
				vsoft_collision->set_value("cl_self", (int)physics::soft_collision::cl_self);
				vsoft_collision->set_value("vf_dd", (int)physics::soft_collision::vf_dd);
				vsoft_collision->set_value("default_t", (int)physics::soft_collision::defaults);

				auto vrotator = vm->set_enum("physics_rotator");
				vrotator->set_value("xyz", (int)trigonometry::rotator::xyz);
				vrotator->set_value("xzy", (int)trigonometry::rotator::xzy);
				vrotator->set_value("yxz", (int)trigonometry::rotator::yxz);
				vrotator->set_value("yzx", (int)trigonometry::rotator::yzx);
				vrotator->set_value("zxy", (int)trigonometry::rotator::zxy);
				vrotator->set_value("zyx", (int)trigonometry::rotator::zyx);

				auto vhull_shape = vm->set_class<physics::hull_shape>("physics_hull_shape", false);
				vhull_shape->set_method("uptr@ get_shape()", &physics::hull_shape::get_shape);
				vhull_shape->set_method_ex("array<vertex>@ get_vertices()", &hull_shape_get_vertices);
				vhull_shape->set_method_ex("array<int>@ get_indices()", &hull_shape_get_indices);

				auto vrigid_body_desc = vm->set_pod<physics::rigid_body::desc>("physics_rigidbody_desc");
				vrigid_body_desc->set_property<physics::rigid_body::desc>("uptr@ shape", &physics::rigid_body::desc::shape);
				vrigid_body_desc->set_property<physics::rigid_body::desc>("float anticipation", &physics::rigid_body::desc::anticipation);
				vrigid_body_desc->set_property<physics::rigid_body::desc>("float mass", &physics::rigid_body::desc::mass);
				vrigid_body_desc->set_property<physics::rigid_body::desc>("vector3 position", &physics::rigid_body::desc::position);
				vrigid_body_desc->set_property<physics::rigid_body::desc>("vector3 rotation", &physics::rigid_body::desc::rotation);
				vrigid_body_desc->set_property<physics::rigid_body::desc>("vector3 scale", &physics::rigid_body::desc::scale);
				vrigid_body_desc->set_constructor<physics::rigid_body::desc>("void f()");

				auto vrigid_body = vm->set_class<physics::rigid_body>("physics_rigidbody", false);
				vrigid_body->set_method("physics_rigidbody@ copy()", &physics::rigid_body::copy);
				vrigid_body->set_method<physics::rigid_body, void, const trigonometry::vector3&>("void push(const vector3 &in)", &physics::rigid_body::push);
				vrigid_body->set_method<physics::rigid_body, void, const trigonometry::vector3&, const trigonometry::vector3&>("void push(const vector3 &in, const vector3 &in)", &physics::rigid_body::push);
				vrigid_body->set_method<physics::rigid_body, void, const trigonometry::vector3&, const trigonometry::vector3&, const trigonometry::vector3&>("void push(const vector3 &in, const vector3 &in, const vector3 &in)", &physics::rigid_body::push);
				vrigid_body->set_method<physics::rigid_body, void, const trigonometry::vector3&>("void push_kinematic(const vector3 &in)", &physics::rigid_body::push_kinematic);
				vrigid_body->set_method<physics::rigid_body, void, const trigonometry::vector3&, const trigonometry::vector3&>("void push_kinematic(const vector3 &in, const vector3 &in)", &physics::rigid_body::push_kinematic);
				vrigid_body->set_method("void synchronize(transform@+, bool)", &physics::rigid_body::synchronize);
				vrigid_body->set_method("void set_collision_flags(usize)", &physics::rigid_body::set_collision_flags);
				vrigid_body->set_method("void set_activity(bool)", &physics::rigid_body::set_activity);
				vrigid_body->set_method("void set_as_ghost()", &physics::rigid_body::set_as_ghost);
				vrigid_body->set_method("void set_as_normal()", &physics::rigid_body::set_as_normal);
				vrigid_body->set_method("void set_self_pointer()", &physics::rigid_body::set_self_pointer);
				vrigid_body->set_method("void set_world_transform(uptr@)", &physics::rigid_body::set_world_transform);
				vrigid_body->set_method("void set_collision_shape(uptr@, transform@+)", &physics::rigid_body::set_collision_shape);
				vrigid_body->set_method("void set_mass(float)", &physics::rigid_body::set_mass);
				vrigid_body->set_method("void set_activation_state(physics_motion_state)", &physics::rigid_body::set_activation_state);
				vrigid_body->set_method("void set_angular_damping(float)", &physics::rigid_body::set_angular_damping);
				vrigid_body->set_method("void set_angular_sleeping_threshold(float)", &physics::rigid_body::set_angular_sleeping_threshold);
				vrigid_body->set_method("void set_spinning_friction(float)", &physics::rigid_body::set_spinning_friction);
				vrigid_body->set_method("void set_contact_stiffness(float)", &physics::rigid_body::set_contact_stiffness);
				vrigid_body->set_method("void set_contact_damping(float)", &physics::rigid_body::set_contact_damping);
				vrigid_body->set_method("void set_friction(float)", &physics::rigid_body::set_friction);
				vrigid_body->set_method("void set_restitution(float)", &physics::rigid_body::set_restitution);
				vrigid_body->set_method("void set_hit_fraction(float)", &physics::rigid_body::set_hit_fraction);
				vrigid_body->set_method("void set_linear_damping(float)", &physics::rigid_body::set_linear_damping);
				vrigid_body->set_method("void set_linear_sleeping_threshold(float)", &physics::rigid_body::set_linear_sleeping_threshold);
				vrigid_body->set_method("void set_ccd_motion_threshold(float)", &physics::rigid_body::set_ccd_motion_threshold);
				vrigid_body->set_method("void set_ccd_swept_sphere_radius(float)", &physics::rigid_body::set_ccd_swept_sphere_radius);
				vrigid_body->set_method("void set_contact_processing_threshold(float)", &physics::rigid_body::set_contact_processing_threshold);
				vrigid_body->set_method("void set_deactivation_time(float)", &physics::rigid_body::set_deactivation_time);
				vrigid_body->set_method("void set_rolling_friction(float)", &physics::rigid_body::set_rolling_friction);
				vrigid_body->set_method("void set_angular_factor(const vector3 &in)", &physics::rigid_body::set_angular_factor);
				vrigid_body->set_method("void set_anisotropic_friction(const vector3 &in)", &physics::rigid_body::set_anisotropic_friction);
				vrigid_body->set_method("void set_gravity(const vector3 &in)", &physics::rigid_body::set_gravity);
				vrigid_body->set_method("void set_linear_factor(const vector3 &in)", &physics::rigid_body::set_linear_factor);
				vrigid_body->set_method("void set_linear_velocity(const vector3 &in)", &physics::rigid_body::set_linear_velocity);
				vrigid_body->set_method("void set_angular_velocity(const vector3 &in)", &physics::rigid_body::set_angular_velocity);
				vrigid_body->set_method("physics_motion_state get_activation_state() const", &physics::rigid_body::get_activation_state);
				vrigid_body->set_method("physics_shape get_collision_shape_type() const", &physics::rigid_body::get_collision_shape_type);
				vrigid_body->set_method("vector3 get_angular_factor() const", &physics::rigid_body::get_angular_factor);
				vrigid_body->set_method("vector3 get_anisotropic_friction() const", &physics::rigid_body::get_anisotropic_friction);
				vrigid_body->set_method("vector3 get_Gravity() const", &physics::rigid_body::get_gravity);
				vrigid_body->set_method("vector3 get_linear_factor() const", &physics::rigid_body::get_linear_factor);
				vrigid_body->set_method("vector3 get_linear_velocity() const", &physics::rigid_body::get_linear_velocity);
				vrigid_body->set_method("vector3 get_angular_velocity() const", &physics::rigid_body::get_angular_velocity);
				vrigid_body->set_method("vector3 get_scale() const", &physics::rigid_body::get_scale);
				vrigid_body->set_method("vector3 get_position() const", &physics::rigid_body::get_position);
				vrigid_body->set_method("vector3 get_rotation() const", &physics::rigid_body::get_rotation);
				vrigid_body->set_method("uptr@ get_world_transform() const", &physics::rigid_body::get_world_transform);
				vrigid_body->set_method("uptr@ get_collision_shape() const", &physics::rigid_body::get_collision_shape);
				vrigid_body->set_method("uptr@ get() const", &physics::rigid_body::get);
				vrigid_body->set_method("bool is_active() const", &physics::rigid_body::is_active);
				vrigid_body->set_method("bool is_static() const", &physics::rigid_body::is_static);
				vrigid_body->set_method("bool is_ghost() const", &physics::rigid_body::is_ghost);
				vrigid_body->set_method("bool iscolliding() const", &physics::rigid_body::is_colliding);
				vrigid_body->set_method("float get_spinning_friction() const", &physics::rigid_body::get_spinning_friction);
				vrigid_body->set_method("float get_contact_stiffness() const", &physics::rigid_body::get_contact_stiffness);
				vrigid_body->set_method("float get_contact_damping() const", &physics::rigid_body::get_contact_damping);
				vrigid_body->set_method("float get_angular_damping() const", &physics::rigid_body::get_angular_damping);
				vrigid_body->set_method("float get_angular_sleeping_threshold() const", &physics::rigid_body::get_angular_sleeping_threshold);
				vrigid_body->set_method("float get_friction() const", &physics::rigid_body::get_friction);
				vrigid_body->set_method("float get_restitution() const", &physics::rigid_body::get_restitution);
				vrigid_body->set_method("float get_hit_fraction() const", &physics::rigid_body::get_hit_fraction);
				vrigid_body->set_method("float get_linear_damping() const", &physics::rigid_body::get_linear_damping);
				vrigid_body->set_method("float get_linear_sleeping_threshold() const", &physics::rigid_body::get_linear_sleeping_threshold);
				vrigid_body->set_method("float get_ccd_motion_threshold() const", &physics::rigid_body::get_ccd_motion_threshold);
				vrigid_body->set_method("float get_ccd_swept_sphere_radius() const", &physics::rigid_body::get_ccd_swept_sphere_radius);
				vrigid_body->set_method("float get_contact_processing_threshold() const", &physics::rigid_body::get_contact_processing_threshold);
				vrigid_body->set_method("float get_deactivation_time() const", &physics::rigid_body::get_deactivation_time);
				vrigid_body->set_method("float get_rolling_friction() const", &physics::rigid_body::get_rolling_friction);
				vrigid_body->set_method("float get_mass() const", &physics::rigid_body::get_mass);
				vrigid_body->set_method("usize get_collision_flags() const", &physics::rigid_body::get_collision_flags);
				vrigid_body->set_method("physics_rigidbody_desc& get_initial_state()", &physics::rigid_body::get_initial_state);
				vrigid_body->set_method("physics_simulator@+ get_simulator() const", &physics::rigid_body::get_simulator);

				auto vsoft_body_sconvex = vm->set_struct<physics::soft_body::desc::cv::sconvex>("physics_softbody_desc_cv_sconvex");
				vsoft_body_sconvex->set_property<physics::soft_body::desc::cv::sconvex>("physics_hull_shape@ hull", &physics::soft_body::desc::cv::sconvex::hull);
				vsoft_body_sconvex->set_property<physics::soft_body::desc::cv::sconvex>("bool enabled", &physics::soft_body::desc::cv::sconvex::enabled);
				vsoft_body_sconvex->set_constructor<physics::soft_body::desc::cv::sconvex>("void f()");
				vsoft_body_sconvex->set_operator_copy_static(&soft_body_sconvex_copy);
				vsoft_body_sconvex->set_destructor_ex("void f()", &soft_body_sconvex_destructor);

				auto vsoft_body_srope = vm->set_pod<physics::soft_body::desc::cv::srope>("physics_softbody_desc_cv_srope");
				vsoft_body_srope->set_property<physics::soft_body::desc::cv::srope>("bool start_fixed", &physics::soft_body::desc::cv::srope::start_fixed);
				vsoft_body_srope->set_property<physics::soft_body::desc::cv::srope>("bool end_fixed", &physics::soft_body::desc::cv::srope::end_fixed);
				vsoft_body_srope->set_property<physics::soft_body::desc::cv::srope>("bool enabled", &physics::soft_body::desc::cv::srope::enabled);
				vsoft_body_srope->set_property<physics::soft_body::desc::cv::srope>("int count", &physics::soft_body::desc::cv::srope::count);
				vsoft_body_srope->set_property<physics::soft_body::desc::cv::srope>("vector3 start", &physics::soft_body::desc::cv::srope::start);
				vsoft_body_srope->set_property<physics::soft_body::desc::cv::srope>("vector3 end", &physics::soft_body::desc::cv::srope::end);
				vsoft_body_srope->set_constructor<physics::soft_body::desc::cv::srope>("void f()");

				auto vsoft_body_spatch = vm->set_pod<physics::soft_body::desc::cv::spatch>("physics_softbody_desc_cv_spatch");
				vsoft_body_spatch->set_property<physics::soft_body::desc::cv::spatch>("bool generate_diagonals", &physics::soft_body::desc::cv::spatch::generate_diagonals);
				vsoft_body_spatch->set_property<physics::soft_body::desc::cv::spatch>("bool corner00_fixed", &physics::soft_body::desc::cv::spatch::corner00_fixed);
				vsoft_body_spatch->set_property<physics::soft_body::desc::cv::spatch>("bool corner10_fixed", &physics::soft_body::desc::cv::spatch::corner10_fixed);
				vsoft_body_spatch->set_property<physics::soft_body::desc::cv::spatch>("bool corner01_fixed", &physics::soft_body::desc::cv::spatch::corner01_fixed);
				vsoft_body_spatch->set_property<physics::soft_body::desc::cv::spatch>("bool corner11_fixed", &physics::soft_body::desc::cv::spatch::corner11_fixed);
				vsoft_body_spatch->set_property<physics::soft_body::desc::cv::spatch>("bool enabled", &physics::soft_body::desc::cv::spatch::enabled);
				vsoft_body_spatch->set_property<physics::soft_body::desc::cv::spatch>("int count_x", &physics::soft_body::desc::cv::spatch::count_x);
				vsoft_body_spatch->set_property<physics::soft_body::desc::cv::spatch>("int count_y", &physics::soft_body::desc::cv::spatch::count_y);
				vsoft_body_spatch->set_property<physics::soft_body::desc::cv::spatch>("vector3 corner00", &physics::soft_body::desc::cv::spatch::corner00);
				vsoft_body_spatch->set_property<physics::soft_body::desc::cv::spatch>("vector3 corner10", &physics::soft_body::desc::cv::spatch::corner10);
				vsoft_body_spatch->set_property<physics::soft_body::desc::cv::spatch>("vector3 corner01", &physics::soft_body::desc::cv::spatch::corner01);
				vsoft_body_spatch->set_property<physics::soft_body::desc::cv::spatch>("vector3 corner11", &physics::soft_body::desc::cv::spatch::corner11);
				vsoft_body_spatch->set_constructor<physics::soft_body::desc::cv::spatch>("void f()");

				auto vsoft_body_sellipsoid = vm->set_pod<physics::soft_body::desc::cv::sellipsoid>("physics_softbody_desc_cv_sellipsoid");
				vsoft_body_sellipsoid->set_property<physics::soft_body::desc::cv::sellipsoid>("vector3 center", &physics::soft_body::desc::cv::sellipsoid::center);
				vsoft_body_sellipsoid->set_property<physics::soft_body::desc::cv::sellipsoid>("vector3 radius", &physics::soft_body::desc::cv::sellipsoid::radius);
				vsoft_body_sellipsoid->set_property<physics::soft_body::desc::cv::sellipsoid>("int count", &physics::soft_body::desc::cv::sellipsoid::count);
				vsoft_body_sellipsoid->set_property<physics::soft_body::desc::cv::sellipsoid>("bool enabled", &physics::soft_body::desc::cv::sellipsoid::enabled);
				vsoft_body_sellipsoid->set_constructor<physics::soft_body::desc::cv::sellipsoid>("void f()");

				auto vsoft_body_cv = vm->set_pod<physics::soft_body::desc::cv>("physics_softbody_desc_cv");
				vsoft_body_cv->set_property<physics::soft_body::desc::cv>("physics_softbody_desc_cv_sconvex convex", &physics::soft_body::desc::cv::convex);
				vsoft_body_cv->set_property<physics::soft_body::desc::cv>("physics_softbody_desc_cv_srope rope", &physics::soft_body::desc::cv::rope);
				vsoft_body_cv->set_property<physics::soft_body::desc::cv>("physics_softbody_desc_cv_spatch patch", &physics::soft_body::desc::cv::patch);
				vsoft_body_cv->set_property<physics::soft_body::desc::cv>("physics_softbody_desc_cv_sellipsoid ellipsoid", &physics::soft_body::desc::cv::ellipsoid);
				vsoft_body_cv->set_constructor<physics::soft_body::desc::cv>("void f()");

				auto vsoft_body_sconfig = vm->set_pod<physics::soft_body::desc::sconfig>("physics_softbody_desc_config");
				vsoft_body_sconfig->set_property<physics::soft_body::desc::sconfig>("physics_soft_aero_model aero_model", &physics::soft_body::desc::sconfig::aero_model);
				vsoft_body_sconfig->set_property<physics::soft_body::desc::sconfig>("float vcf", &physics::soft_body::desc::sconfig::vcf);
				vsoft_body_sconfig->set_property<physics::soft_body::desc::sconfig>("float dp", &physics::soft_body::desc::sconfig::dp);
				vsoft_body_sconfig->set_property<physics::soft_body::desc::sconfig>("float dg", &physics::soft_body::desc::sconfig::dg);
				vsoft_body_sconfig->set_property<physics::soft_body::desc::sconfig>("float lf", &physics::soft_body::desc::sconfig::lf);
				vsoft_body_sconfig->set_property<physics::soft_body::desc::sconfig>("float pr", &physics::soft_body::desc::sconfig::pr);
				vsoft_body_sconfig->set_property<physics::soft_body::desc::sconfig>("float vc", &physics::soft_body::desc::sconfig::vc);
				vsoft_body_sconfig->set_property<physics::soft_body::desc::sconfig>("float df", &physics::soft_body::desc::sconfig::df);
				vsoft_body_sconfig->set_property<physics::soft_body::desc::sconfig>("float mt", &physics::soft_body::desc::sconfig::mt);
				vsoft_body_sconfig->set_property<physics::soft_body::desc::sconfig>("float chr", &physics::soft_body::desc::sconfig::chr);
				vsoft_body_sconfig->set_property<physics::soft_body::desc::sconfig>("float khr", &physics::soft_body::desc::sconfig::khr);
				vsoft_body_sconfig->set_property<physics::soft_body::desc::sconfig>("float shr", &physics::soft_body::desc::sconfig::shr);
				vsoft_body_sconfig->set_property<physics::soft_body::desc::sconfig>("float ahr", &physics::soft_body::desc::sconfig::ahr);
				vsoft_body_sconfig->set_property<physics::soft_body::desc::sconfig>("float srhr_cl", &physics::soft_body::desc::sconfig::srhr_cl);
				vsoft_body_sconfig->set_property<physics::soft_body::desc::sconfig>("float skhr_cl", &physics::soft_body::desc::sconfig::skhr_cl);
				vsoft_body_sconfig->set_property<physics::soft_body::desc::sconfig>("float sshr_cl", &physics::soft_body::desc::sconfig::sshr_cl);
				vsoft_body_sconfig->set_property<physics::soft_body::desc::sconfig>("float sr_splt_cl", &physics::soft_body::desc::sconfig::sr_splt_cl);
				vsoft_body_sconfig->set_property<physics::soft_body::desc::sconfig>("float sk_splt_cl", &physics::soft_body::desc::sconfig::sk_splt_cl);
				vsoft_body_sconfig->set_property<physics::soft_body::desc::sconfig>("float ss_splt_cl", &physics::soft_body::desc::sconfig::ss_splt_cl);
				vsoft_body_sconfig->set_property<physics::soft_body::desc::sconfig>("float max_volume", &physics::soft_body::desc::sconfig::max_volume);
				vsoft_body_sconfig->set_property<physics::soft_body::desc::sconfig>("float time_scale", &physics::soft_body::desc::sconfig::time_scale);
				vsoft_body_sconfig->set_property<physics::soft_body::desc::sconfig>("float drag", &physics::soft_body::desc::sconfig::drag);
				vsoft_body_sconfig->set_property<physics::soft_body::desc::sconfig>("float max_stress", &physics::soft_body::desc::sconfig::max_stress);
				vsoft_body_sconfig->set_property<physics::soft_body::desc::sconfig>("int clusters", &physics::soft_body::desc::sconfig::clusters);
				vsoft_body_sconfig->set_property<physics::soft_body::desc::sconfig>("int constraints", &physics::soft_body::desc::sconfig::constraints);
				vsoft_body_sconfig->set_property<physics::soft_body::desc::sconfig>("int viterations", &physics::soft_body::desc::sconfig::viterations);
				vsoft_body_sconfig->set_property<physics::soft_body::desc::sconfig>("int Piterations", &physics::soft_body::desc::sconfig::piterations);
				vsoft_body_sconfig->set_property<physics::soft_body::desc::sconfig>("int diterations", &physics::soft_body::desc::sconfig::diterations);
				vsoft_body_sconfig->set_property<physics::soft_body::desc::sconfig>("int citerations", &physics::soft_body::desc::sconfig::citerations);
				vsoft_body_sconfig->set_property<physics::soft_body::desc::sconfig>("int collisions", &physics::soft_body::desc::sconfig::collisions);
				vsoft_body_sconfig->set_constructor<physics::soft_body::desc::sconfig>("void f()");

				auto vsoft_body_desc = vm->set_pod<physics::soft_body::desc>("physics_softbody_desc");
				vsoft_body_desc->set_property<physics::soft_body::desc>("physics_softbody_desc_cv shape", &physics::soft_body::desc::shape);
				vsoft_body_desc->set_property<physics::soft_body::desc>("physics_softbody_desc_config feature", &physics::soft_body::desc::config);
				vsoft_body_desc->set_property<physics::soft_body::desc>("float anticipation", &physics::soft_body::soft_body::desc::anticipation);
				vsoft_body_desc->set_property<physics::soft_body::desc>("vector3 position", &physics::soft_body::soft_body::desc::position);
				vsoft_body_desc->set_property<physics::soft_body::desc>("vector3 rotation", &physics::soft_body::soft_body::desc::rotation);
				vsoft_body_desc->set_property<physics::soft_body::desc>("vector3 scale", &physics::soft_body::soft_body::desc::scale);
				vsoft_body_desc->set_constructor<physics::soft_body::desc>("void f()");

				auto vsoft_body = vm->set_class<physics::soft_body>("physics_softbody", false);
				auto vsoft_body_ray_cast = vm->set_pod<physics::soft_body::ray_cast>("physics_softbody_raycast");
				vsoft_body_ray_cast->set_property<physics::soft_body::ray_cast>("physics_softbody@ body", &physics::soft_body::ray_cast::body);
				vsoft_body_ray_cast->set_property<physics::soft_body::ray_cast>("physics_soft_feature feature", &physics::soft_body::ray_cast::feature);
				vsoft_body_ray_cast->set_property<physics::soft_body::ray_cast>("float fraction", &physics::soft_body::ray_cast::fraction);
				vsoft_body_ray_cast->set_property<physics::soft_body::ray_cast>("int32 index", &physics::soft_body::ray_cast::index);
				vsoft_body_ray_cast->set_constructor<physics::soft_body::ray_cast>("void f()");

				vsoft_body->set_method("physics_softbody@ copy()", &physics::soft_body::copy);
				vsoft_body->set_method("void activate(bool)", &physics::soft_body::activate);
				vsoft_body->set_method("void synchronize(transform@+, bool)", &physics::soft_body::synchronize);
				vsoft_body->set_method_ex("array<int32>@ get_indices() const", &soft_body_get_indices);
				vsoft_body->set_method_ex("array<vertex>@ get_vertices() const", &soft_body_get_vertices);
				vsoft_body->set_method("void get_bounding_box(vector3 &out, vector3 &out) const", &physics::soft_body::get_bounding_box);
				vsoft_body->set_method("void set_contact_stiffness_and_damping(float, float)", &physics::soft_body::set_contact_stiffness_and_damping);
				vsoft_body->set_method<physics::soft_body, void, int, physics::rigid_body*, bool, float>("void add_anchor(int32, physics_rigidbody@+, bool = false, float = 1)", &physics::soft_body::add_anchor);
				vsoft_body->set_method<physics::soft_body, void, int, physics::rigid_body*, const trigonometry::vector3&, bool, float>("void add_anchor(int32, physics_rigidbody@+, const vector3 &in, bool = false, float = 1)", &physics::soft_body::add_anchor);
				vsoft_body->set_method<physics::soft_body, void, const trigonometry::vector3&>("void add_force(const vector3 &in)", &physics::soft_body::add_force);
				vsoft_body->set_method<physics::soft_body, void, const trigonometry::vector3&, int>("void add_force(const vector3 &in, int)", &physics::soft_body::add_force);
				vsoft_body->set_method("void add_aero_force_to_node(const vector3 &in, int)", &physics::soft_body::add_aero_force_to_node);
				vsoft_body->set_method("void add_aero_force_to_face(const vector3 &in, int)", &physics::soft_body::add_aero_force_to_face);
				vsoft_body->set_method<physics::soft_body, void, const trigonometry::vector3&>("void add_velocity(const vector3 &in)", &physics::soft_body::add_velocity);
				vsoft_body->set_method<physics::soft_body, void, const trigonometry::vector3&, int>("void add_velocity(const vector3 &in, int)", &physics::soft_body::add_velocity);
				vsoft_body->set_method("void set_selocity(const vector3 &in)", &physics::soft_body::set_velocity);
				vsoft_body->set_method("void set_mass(int, float)", &physics::soft_body::set_mass);
				vsoft_body->set_method("void set_total_mass(float, bool = false)", &physics::soft_body::set_total_mass);
				vsoft_body->set_method("void set_total_density(float)", &physics::soft_body::set_total_density);
				vsoft_body->set_method("void set_volume_mass(float)", &physics::soft_body::set_volume_mass);
				vsoft_body->set_method("void set_volume_density(float)", &physics::soft_body::set_volume_density);
				vsoft_body->set_method("void translate(const vector3 &in)", &physics::soft_body::translate);
				vsoft_body->set_method("void rotate(const vector3 &in)", &physics::soft_body::rotate);
				vsoft_body->set_method("void scale(const vector3 &in)", &physics::soft_body::scale);
				vsoft_body->set_method("void set_rest_length_scale(float)", &physics::soft_body::set_rest_length_scale);
				vsoft_body->set_method("void set_pose(bool, bool)", &physics::soft_body::set_pose);
				vsoft_body->set_method("float get_mass(int) const", &physics::soft_body::get_mass);
				vsoft_body->set_method("float get_total_mass() const", &physics::soft_body::get_total_mass);
				vsoft_body->set_method("float get_rest_length_scale() const", &physics::soft_body::get_rest_length_scale);
				vsoft_body->set_method("float get_volume() const", &physics::soft_body::get_volume);
				vsoft_body->set_method("int generate_bending_constraints(int)", &physics::soft_body::generate_bending_constraints);
				vsoft_body->set_method("void randomize_constraints()", &physics::soft_body::randomize_constraints);
				vsoft_body->set_method("bool cut_link(int, int, float)", &physics::soft_body::cut_link);
				vsoft_body->set_method("bool ray_test(const vector3 &in, const vector3 &in, physics_softbody_raycast &out)", &physics::soft_body::ray_test);
				vsoft_body->set_method("void set_wind_velocity(const vector3 &in)", &physics::soft_body::set_wind_velocity);
				vsoft_body->set_method("vector3 get_wind_velocity() const", &physics::soft_body::get_wind_velocity);
				vsoft_body->set_method("void get_aabb(vector3 &out, vector3 &out) const", &physics::soft_body::get_aabb);
				vsoft_body->set_method("void set_spinning_friction(float)", &physics::soft_body::set_spinning_friction);
				vsoft_body->set_method("vector3 get_linear_velocity() const", &physics::soft_body::get_linear_velocity);
				vsoft_body->set_method("vector3 get_angular_velocity() const", &physics::soft_body::get_angular_velocity);
				vsoft_body->set_method("vector3 get_center_position() const", &physics::soft_body::get_center_position);
				vsoft_body->set_method("void set_activity(bool)", &physics::soft_body::set_activity);
				vsoft_body->set_method("void set_as_ghost()", &physics::soft_body::set_as_ghost);
				vsoft_body->set_method("void set_as_normal()", &physics::soft_body::set_as_normal);
				vsoft_body->set_method("void set_self_pointer()", &physics::soft_body::set_self_pointer);
				vsoft_body->set_method("void set_world_transform(uptr@)", &physics::soft_body::set_world_transform);
				vsoft_body->set_method("void set_activation_state(physics_motion_state)", &physics::soft_body::set_activation_state);
				vsoft_body->set_method("void set_contact_stiffness(float)", &physics::soft_body::set_contact_stiffness);
				vsoft_body->set_method("void set_contact_damping(float)", &physics::soft_body::set_contact_damping);
				vsoft_body->set_method("void set_friction(float)", &physics::soft_body::set_friction);
				vsoft_body->set_method("void set_restitution(float)", &physics::soft_body::set_restitution);
				vsoft_body->set_method("void set_hit_fraction(float)", &physics::soft_body::set_hit_fraction);
				vsoft_body->set_method("void set_ccd_motion_threshold(float)", &physics::soft_body::set_ccd_motion_threshold);
				vsoft_body->set_method("void set_ccd_swept_sphere_radius(float)", &physics::soft_body::set_ccd_swept_sphere_radius);
				vsoft_body->set_method("void set_contact_processing_threshold(float)", &physics::soft_body::set_contact_processing_threshold);
				vsoft_body->set_method("void set_reactivation_time(float)", &physics::soft_body::set_deactivation_time);
				vsoft_body->set_method("void set_rolling_friction(float)", &physics::soft_body::set_rolling_friction);
				vsoft_body->set_method("void set_anisotropic_friction(const vector3 &in)", &physics::soft_body::set_anisotropic_friction);
				vsoft_body->set_method("void set_config(const physics_softbody_desc_config &in)", &physics::soft_body::set_config);
				vsoft_body->set_method("physics_shape get_collision_shape_type() const", &physics::soft_body::get_collision_shape_type);
				vsoft_body->set_method("physics_motion_state get_activation_state() const", &physics::soft_body::get_activation_state);
				vsoft_body->set_method("vector3 get_anisotropic_friction() const", &physics::soft_body::get_anisotropic_friction);
				vsoft_body->set_method("vector3 get_scale() const", &physics::soft_body::get_scale);
				vsoft_body->set_method("vector3 get_position() const", &physics::soft_body::get_position);
				vsoft_body->set_method("vector3 get_rotation() const", &physics::soft_body::get_rotation);
				vsoft_body->set_method("uptr@ get_world_transform() const", &physics::soft_body::get_world_transform);
				vsoft_body->set_method("uptr@ get() const", &physics::soft_body::get);
				vsoft_body->set_method("bool is_active() const", &physics::soft_body::is_active);
				vsoft_body->set_method("bool is_static() const", &physics::soft_body::is_static);
				vsoft_body->set_method("bool is_ghost() const", &physics::soft_body::is_ghost);
				vsoft_body->set_method("bool is_colliding() const", &physics::soft_body::is_colliding);
				vsoft_body->set_method("float get_spinning_friction() const", &physics::soft_body::get_spinning_friction);
				vsoft_body->set_method("float get_contact_stiffness() const", &physics::soft_body::get_contact_stiffness);
				vsoft_body->set_method("float get_contact_damping() const", &physics::soft_body::get_contact_damping);
				vsoft_body->set_method("float get_friction() const", &physics::soft_body::get_friction);
				vsoft_body->set_method("float get_restitution() const", &physics::soft_body::get_restitution);
				vsoft_body->set_method("float get_hit_fraction() const", &physics::soft_body::get_hit_fraction);
				vsoft_body->set_method("float get_ccd_motion_threshold() const", &physics::soft_body::get_ccd_motion_threshold);
				vsoft_body->set_method("float get_ccd_swept_sphere_radius() const", &physics::soft_body::get_ccd_swept_sphere_radius);
				vsoft_body->set_method("float get_contact_processing_threshold() const", &physics::soft_body::get_contact_processing_threshold);
				vsoft_body->set_method("float get_deactivation_time() const", &physics::soft_body::get_deactivation_time);
				vsoft_body->set_method("float get_rolling_friction() const", &physics::soft_body::get_rolling_friction);
				vsoft_body->set_method("usize get_collision_flags() const", &physics::soft_body::get_collision_flags);
				vsoft_body->set_method("usize get_vertices_count() const", &physics::soft_body::get_vertices_count);
				vsoft_body->set_method("physics_softbody_desc& get_initial_state()", &physics::soft_body::get_initial_state);
				vsoft_body->set_method("physics_simulator@+ get_simulator() const", &physics::soft_body::get_simulator);

				auto vconstraint = vm->set_class<physics::constraint>("physics_constraint", false);
				vconstraint->set_method("physics_constraint@ copy() const", &physics::constraint::copy);
				vconstraint->set_method("physics_simulator@+ get_simulator() const", &physics::constraint::get_simulator);
				vconstraint->set_method("uptr@ get() const", &physics::constraint::get);
				vconstraint->set_method("uptr@ get_first() const", &physics::constraint::get_first);
				vconstraint->set_method("uptr@ get_second() const", &physics::constraint::get_second);
				vconstraint->set_method("bool has_collisions() const", &physics::constraint::has_collisions);
				vconstraint->set_method("bool is_enabled() const", &physics::constraint::is_enabled);
				vconstraint->set_method("bool is_active() const", &physics::constraint::is_active);
				vconstraint->set_method("void set_breaking_impulse_threshold(float)", &physics::constraint::set_breaking_impulse_threshold);
				vconstraint->set_method("void set_enabled(bool)", &physics::constraint::set_enabled);
				vconstraint->set_method("float get_breaking_impulse_threshold() const", &physics::constraint::get_breaking_impulse_threshold);

				auto vp_constraint_desc = vm->set_pod<physics::pconstraint::desc>("physics_pconstraint_desc");
				vp_constraint_desc->set_property<physics::pconstraint::desc>("physics_rigidbody@ target_a", &physics::pconstraint::desc::target_a);
				vp_constraint_desc->set_property<physics::pconstraint::desc>("physics_rigidbody@ target_b", &physics::pconstraint::desc::target_b);
				vp_constraint_desc->set_property<physics::pconstraint::desc>("vector3 pivot_a", &physics::pconstraint::desc::pivot_a);
				vp_constraint_desc->set_property<physics::pconstraint::desc>("vector3 pivot_b", &physics::pconstraint::desc::pivot_b);
				vp_constraint_desc->set_property<physics::pconstraint::desc>("bool collisions", &physics::pconstraint::desc::collisions);
				vp_constraint_desc->set_constructor<physics::pconstraint::desc>("void f()");

				auto vp_constraint = vm->set_class<physics::pconstraint>("physics_pconstraint", false);
				vp_constraint->set_method("physics_pconstraint@ copy() const", &physics::pconstraint::copy);
				vp_constraint->set_method("physics_simulator@+ get_simulator() const", &physics::pconstraint::get_simulator);
				vp_constraint->set_method("uptr@ get() const", &physics::pconstraint::get);
				vp_constraint->set_method("uptr@ get_first() const", &physics::pconstraint::get_first);
				vp_constraint->set_method("uptr@ get_second() const", &physics::pconstraint::get_second);
				vp_constraint->set_method("bool has_collisions() const", &physics::pconstraint::has_collisions);
				vp_constraint->set_method("bool is_enabled() const", &physics::pconstraint::is_enabled);
				vp_constraint->set_method("bool is_active() const", &physics::pconstraint::is_active);
				vp_constraint->set_method("void set_breaking_impulse_threshold(float)", &physics::pconstraint::set_breaking_impulse_threshold);
				vp_constraint->set_method("void set_enabled(bool)", &physics::pconstraint::set_enabled);
				vp_constraint->set_method("float get_breaking_impulse_threshold() const", &physics::pconstraint::get_breaking_impulse_threshold);
				vp_constraint->set_method("void set_pivot_a(const vector3 &in)", &physics::pconstraint::set_pivot_a);
				vp_constraint->set_method("void set_pivot_b(const vector3 &in)", &physics::pconstraint::set_pivot_b);
				vp_constraint->set_method("vector3 get_pivot_a() const", &physics::pconstraint::get_pivot_a);
				vp_constraint->set_method("vector3 get_pivot_b() const", &physics::pconstraint::get_pivot_b);
				vp_constraint->set_method("physics_pconstraint_desc get_state() const", &physics::pconstraint::get_state);

				auto vh_constraint_desc = vm->set_pod<physics::hconstraint::desc>("physics_hconstraint_desc");
				vh_constraint_desc->set_property<physics::hconstraint::desc>("physics_rigidbody@ target_a", &physics::hconstraint::desc::target_a);
				vh_constraint_desc->set_property<physics::hconstraint::desc>("physics_rigidbody@ target_b", &physics::hconstraint::desc::target_b);
				vh_constraint_desc->set_property<physics::hconstraint::desc>("bool references", &physics::hconstraint::desc::references);
				vh_constraint_desc->set_property<physics::hconstraint::desc>("bool collisions", &physics::hconstraint::desc::collisions);
				vp_constraint_desc->set_constructor<physics::hconstraint::desc>("void f()");

				auto vh_constraint = vm->set_class<physics::hconstraint>("physics_hconstraint", false);
				vh_constraint->set_method("physics_hconstraint@ copy() const", &physics::hconstraint::copy);
				vh_constraint->set_method("physics_simulator@+ get_simulator() const", &physics::hconstraint::get_simulator);
				vh_constraint->set_method("uptr@ get() const", &physics::hconstraint::get);
				vh_constraint->set_method("uptr@ get_first() const", &physics::hconstraint::get_first);
				vh_constraint->set_method("uptr@ get_second() const", &physics::hconstraint::get_second);
				vh_constraint->set_method("bool has_collisions() const", &physics::hconstraint::has_collisions);
				vh_constraint->set_method("bool is_enabled() const", &physics::hconstraint::is_enabled);
				vh_constraint->set_method("bool is_active() const", &physics::hconstraint::is_active);
				vh_constraint->set_method("void set_breaking_impulse_threshold(float)", &physics::hconstraint::set_breaking_impulse_threshold);
				vh_constraint->set_method("void set_enabled(bool)", &physics::hconstraint::set_enabled);
				vh_constraint->set_method("float get_breaking_impulse_threshold() const", &physics::hconstraint::get_breaking_impulse_threshold);
				vh_constraint->set_method("void enable_angular_motor(bool, float, float)", &physics::hconstraint::enable_angular_motor);
				vh_constraint->set_method("void enable_motor(bool)", &physics::hconstraint::enable_motor);
				vh_constraint->set_method("void test_limit(const matrix4x4 &in, const matrix4x4 &in)", &physics::hconstraint::test_limit);
				vh_constraint->set_method("void set_frames(const matrix4x4 &in, const matrix4x4 &in)", &physics::hconstraint::set_frames);
				vh_constraint->set_method("void set_angular_only(bool)", &physics::hconstraint::set_angular_only);
				vh_constraint->set_method("void set_max_motor_impulse(float)", &physics::hconstraint::set_max_motor_impulse);
				vh_constraint->set_method("void set_motor_target_velocity(float)", &physics::hconstraint::set_motor_target_velocity);
				vh_constraint->set_method("void set_motor_target(float, float)", &physics::hconstraint::set_motor_target);
				vh_constraint->set_method("void set_limit(float low, float high, float softness = 0.9f, float bias_factor = 0.3f, float relaxation_factor = 1.0f)", &physics::hconstraint::set_limit);
				vh_constraint->set_method("void set_offset(bool value)", &physics::hconstraint::set_offset);
				vh_constraint->set_method("void set_reference_to_a(bool value)", &physics::hconstraint::set_reference_to_a);
				vh_constraint->set_method("void set_axis(const vector3 &in)", &physics::hconstraint::set_axis);
				vh_constraint->set_method("int get_solve_limit() const", &physics::hconstraint::get_solve_limit);
				vh_constraint->set_method("float get_motor_target_velocity() const", &physics::hconstraint::get_motor_target_velocity);
				vh_constraint->set_method("float get_max_motor_impulse() const", &physics::hconstraint::get_max_motor_impulse);
				vh_constraint->set_method("float get_limit_sign() const", &physics::hconstraint::get_limit_sign);
				vh_constraint->set_method<physics::hconstraint, float>("float get_hinge_angle() const", &physics::hconstraint::get_hinge_angle);
				vh_constraint->set_method<physics::hconstraint, float, const trigonometry::matrix4x4&, const trigonometry::matrix4x4&>("float get_hinge_angle(const matrix4x4 &in, const matrix4x4 &in) const", &physics::hconstraint::get_hinge_angle);
				vh_constraint->set_method("float get_lower_limit() const", &physics::hconstraint::get_lower_limit);
				vh_constraint->set_method("float get_upper_limit() const", &physics::hconstraint::get_upper_limit);
				vh_constraint->set_method("float get_limit_softness() const", &physics::hconstraint::get_limit_softness);
				vh_constraint->set_method("float get_limit_bias_factor() const", &physics::hconstraint::get_limit_bias_factor);
				vh_constraint->set_method("float get_limit_relaxation_factor() const", &physics::hconstraint::get_limit_relaxation_factor);
				vh_constraint->set_method("bool has_limit() const", &physics::hconstraint::has_limit);
				vh_constraint->set_method("bool is_offset() const", &physics::hconstraint::is_offset);
				vh_constraint->set_method("bool is_reference_to_a() const", &physics::hconstraint::is_reference_to_a);
				vh_constraint->set_method("bool is_angular_only() const", &physics::hconstraint::is_angular_only);
				vh_constraint->set_method("bool is_angular_motor_enabled() const", &physics::hconstraint::is_angular_motor_enabled);
				vh_constraint->set_method("physics_hconstraint_desc& get_state()", &physics::hconstraint::get_state);

				auto vs_constraint_desc = vm->set_pod<physics::sconstraint::desc>("physics_sconstraint_desc");
				vs_constraint_desc->set_property<physics::sconstraint::desc>("physics_rigidbody@ target_a", &physics::sconstraint::desc::target_a);
				vs_constraint_desc->set_property<physics::sconstraint::desc>("physics_rigidbody@ target_b", &physics::sconstraint::desc::target_b);
				vs_constraint_desc->set_property<physics::sconstraint::desc>("bool linear", &physics::sconstraint::desc::linear);
				vs_constraint_desc->set_property<physics::sconstraint::desc>("bool collisions", &physics::sconstraint::desc::collisions);
				vs_constraint_desc->set_constructor<physics::sconstraint::desc>("void f()");

				auto vs_constraint = vm->set_class<physics::sconstraint>("physics_sconstraint", false);
				vs_constraint->set_method("physics_sconstraint@ copy() const", &physics::sconstraint::copy);
				vs_constraint->set_method("physics_simulator@+ get_simulator() const", &physics::sconstraint::get_simulator);
				vs_constraint->set_method("uptr@ get() const", &physics::sconstraint::get);
				vs_constraint->set_method("uptr@ get_first() const", &physics::sconstraint::get_first);
				vs_constraint->set_method("uptr@ get_second() const", &physics::sconstraint::get_second);
				vs_constraint->set_method("bool has_collisions() const", &physics::sconstraint::has_collisions);
				vs_constraint->set_method("bool is_enabled() const", &physics::sconstraint::is_enabled);
				vs_constraint->set_method("bool is_active() const", &physics::sconstraint::is_active);
				vs_constraint->set_method("void set_breaking_impulse_threshold(float)", &physics::sconstraint::set_breaking_impulse_threshold);
				vs_constraint->set_method("void set_enabled(bool)", &physics::sconstraint::set_enabled);
				vs_constraint->set_method("float get_breaking_impulse_threshold() const", &physics::sconstraint::get_breaking_impulse_threshold);
				vs_constraint->set_method("void set_angular_motor_velocity(float)", &physics::sconstraint::set_angular_motor_velocity);
				vs_constraint->set_method("void set_linear_motor_velocity(float)", &physics::sconstraint::set_linear_motor_velocity);
				vs_constraint->set_method("void set_upper_linear_limit(float)", &physics::sconstraint::set_upper_linear_limit);
				vs_constraint->set_method("void set_lower_linear_limit(float)", &physics::sconstraint::set_lower_linear_limit);
				vs_constraint->set_method("void set_angular_damping_direction(float)", &physics::sconstraint::set_angular_damping_direction);
				vs_constraint->set_method("void set_linear_damping_direction(float)", &physics::sconstraint::set_linear_damping_direction);
				vs_constraint->set_method("void set_angular_damping_limit(float)", &physics::sconstraint::set_angular_damping_limit);
				vs_constraint->set_method("void set_linear_damping_limit(float)", &physics::sconstraint::set_linear_damping_limit);
				vs_constraint->set_method("void set_angular_damping_ortho(float)", &physics::sconstraint::set_angular_damping_ortho);
				vs_constraint->set_method("void set_linear_damping_ortho(float)", &physics::sconstraint::set_linear_damping_ortho);
				vs_constraint->set_method("void set_upper_angular_limit(float)", &physics::sconstraint::set_upper_angular_limit);
				vs_constraint->set_method("void set_lower_angular_limit(float)", &physics::sconstraint::set_lower_angular_limit);
				vs_constraint->set_method("void set_max_angular_motor_force(float)", &physics::sconstraint::set_max_angular_motor_force);
				vs_constraint->set_method("void set_max_linear_motor_force(float)", &physics::sconstraint::set_max_linear_motor_force);
				vs_constraint->set_method("void set_angular_restitution_direction(float)", &physics::sconstraint::set_angular_restitution_direction);
				vs_constraint->set_method("void set_linear_restitution_direction(float)", &physics::sconstraint::set_linear_restitution_direction);
				vs_constraint->set_method("void set_angular_restitution_limit(float)", &physics::sconstraint::set_angular_restitution_limit);
				vs_constraint->set_method("void set_linear_restitution_limit(float)", &physics::sconstraint::set_linear_restitution_limit);
				vs_constraint->set_method("void set_angular_restitution_ortho(float)", &physics::sconstraint::set_angular_restitution_ortho);
				vs_constraint->set_method("void set_linear_restitution_ortho(float)", &physics::sconstraint::set_linear_restitution_ortho);
				vs_constraint->set_method("void set_angular_softness_direction(float)", &physics::sconstraint::set_angular_softness_direction);
				vs_constraint->set_method("void set_linear_softnessdirection(float)", &physics::sconstraint::set_linear_softness_direction);
				vs_constraint->set_method("void setangularsoftnesslimit(float)", &physics::sconstraint::set_angular_softness_limit);
				vs_constraint->set_method("void setlinearsoftnesslimit(float)", &physics::sconstraint::set_linear_softness_limit);
				vs_constraint->set_method("void setangularsoftnessortho(float)", &physics::sconstraint::set_angular_softness_ortho);
				vs_constraint->set_method("void set_linear_softness_ortho(float)", &physics::sconstraint::set_linear_softness_ortho);
				vs_constraint->set_method("void set_powered_angular_motor(bool)", &physics::sconstraint::set_powered_angular_motor);
				vs_constraint->set_method("void set_powered_linear_motor(bool)", &physics::sconstraint::set_powered_linear_motor);
				vs_constraint->set_method("float getAngularMotorVelocity() const", &physics::sconstraint::get_angular_motor_velocity);
				vs_constraint->set_method("float get_linear_motor_velocity() const", &physics::sconstraint::get_linear_motor_velocity);
				vs_constraint->set_method("float get_upper_linear_limit() const", &physics::sconstraint::get_upper_linear_limit);
				vs_constraint->set_method("float get_lower_linear_limit() const", &physics::sconstraint::get_lower_linear_limit);
				vs_constraint->set_method("float get_angular_damping_direction() const", &physics::sconstraint::get_angular_damping_direction);
				vs_constraint->set_method("float get_linear_damping_direction() const", &physics::sconstraint::get_linear_damping_direction);
				vs_constraint->set_method("float get_angular_damping_limit() const", &physics::sconstraint::get_angular_damping_limit);
				vs_constraint->set_method("float get_linear_damping_limit() const", &physics::sconstraint::get_linear_damping_limit);
				vs_constraint->set_method("float get_angular_damping_ortho() const", &physics::sconstraint::get_angular_damping_ortho);
				vs_constraint->set_method("float get_linear_damping_ortho() const", &physics::sconstraint::get_linear_damping_ortho);
				vs_constraint->set_method("float get_upper_angular_limit() const", &physics::sconstraint::get_upper_angular_limit);
				vs_constraint->set_method("float get_lower_angular_limit() const", &physics::sconstraint::get_lower_angular_limit);
				vs_constraint->set_method("float get_max_angular_motor_force() const", &physics::sconstraint::get_max_angular_motor_force);
				vs_constraint->set_method("float get_max_linear_motor_force() const", &physics::sconstraint::get_max_linear_motor_force);
				vs_constraint->set_method("float get_angular_restitution_direction() const", &physics::sconstraint::get_angular_restitution_direction);
				vs_constraint->set_method("float get_linear_restitution_direction() const", &physics::sconstraint::get_linear_restitution_direction);
				vs_constraint->set_method("float get_angular_restitution_limit() const", &physics::sconstraint::get_angular_restitution_limit);
				vs_constraint->set_method("float get_linear_restitution_limit() const", &physics::sconstraint::get_linear_restitution_limit);
				vs_constraint->set_method("float get_angular_restitution_ortho() const", &physics::sconstraint::get_angular_restitution_ortho);
				vs_constraint->set_method("float get_linearRestitution_ortho() const", &physics::sconstraint::get_linear_restitution_ortho);
				vs_constraint->set_method("float get_angular_softness_direction() const", &physics::sconstraint::get_angular_softness_direction);
				vs_constraint->set_method("float get_linear_softness_direction() const", &physics::sconstraint::get_linear_softness_direction);
				vs_constraint->set_method("float get_angular_softness_limit() const", &physics::sconstraint::get_angular_softness_limit);
				vs_constraint->set_method("float get_linear_softness_limit() const", &physics::sconstraint::get_linear_softness_limit);
				vs_constraint->set_method("float get_angular_softness_ortho() const", &physics::sconstraint::get_angular_softness_ortho);
				vs_constraint->set_method("float get_linear_softness_ortho() const", &physics::sconstraint::get_linear_softness_ortho);
				vs_constraint->set_method("bool get_powered_angular_motor() const", &physics::sconstraint::get_powered_angular_motor);
				vs_constraint->set_method("bool get_powered_linear_motor() const", &physics::sconstraint::get_powered_linear_motor);
				vs_constraint->set_method("physics_sconstraint_desc& get_state()", &physics::sconstraint::get_state);

				auto vct_constraint_desc = vm->set_pod<physics::ct_constraint::desc>("physics_ctconstraint_desc");
				vct_constraint_desc->set_property<physics::ct_constraint::desc>("physics_rigidbody@ target_a", &physics::ct_constraint::desc::target_a);
				vct_constraint_desc->set_property<physics::ct_constraint::desc>("physics_rigidbody@ target_b", &physics::ct_constraint::desc::target_b);
				vct_constraint_desc->set_property<physics::ct_constraint::desc>("bool collisions", &physics::ct_constraint::desc::collisions);
				vct_constraint_desc->set_constructor<physics::ct_constraint::desc>("void f()");

				auto vct_constraint = vm->set_class<physics::ct_constraint>("physics_ctconstraint", false);
				vct_constraint->set_method("physics_ctconstraint@ copy() const", &physics::ct_constraint::copy);
				vct_constraint->set_method("physics_simulator@+ get_simulator() const", &physics::ct_constraint::get_simulator);
				vct_constraint->set_method("uptr@ get() const", &physics::ct_constraint::get);
				vct_constraint->set_method("uptr@ get_first() const", &physics::ct_constraint::get_first);
				vct_constraint->set_method("uptr@ get_second() const", &physics::ct_constraint::get_second);
				vct_constraint->set_method("bool has_collisions() const", &physics::ct_constraint::has_collisions);
				vct_constraint->set_method("bool is_enabled() const", &physics::ct_constraint::is_enabled);
				vct_constraint->set_method("bool is_active() const", &physics::ct_constraint::is_active);
				vct_constraint->set_method("void set_breaking_impulse_threshold(float)", &physics::ct_constraint::set_breaking_impulse_threshold);
				vct_constraint->set_method("void set_enabled(bool)", &physics::ct_constraint::set_enabled);
				vct_constraint->set_method("float get_breaking_impulse_threshold() const", &physics::ct_constraint::get_breaking_impulse_threshold);
				vct_constraint->set_method("void enable_motor(bool)", &physics::ct_constraint::enable_motor);
				vct_constraint->set_method("void set_frames(const matrix4x4 &in, const matrix4x4 &in)", &physics::ct_constraint::set_frames);
				vct_constraint->set_method("void set_angular_only(bool)", &physics::ct_constraint::set_angular_only);
				vct_constraint->set_method<physics::ct_constraint, void, int, float>("void set_limit(int, float)", &physics::ct_constraint::set_limit);
				vct_constraint->set_method<physics::ct_constraint, void, float, float, float, float, float, float>("void set_limit(float, float, float, float = 1.f, float = 0.3f, float = 1.0f)", &physics::ct_constraint::set_limit);
				vct_constraint->set_method("void set_damping(float)", &physics::ct_constraint::set_damping);
				vct_constraint->set_method("void set_max_motor_impulse(float)", &physics::ct_constraint::set_max_motor_impulse);
				vct_constraint->set_method("void set_max_motor_impulse_normalized(float)", &physics::ct_constraint::set_max_motor_impulse_normalized);
				vct_constraint->set_method("void set_fix_thresh(float)", &physics::ct_constraint::set_fix_thresh);
				vct_constraint->set_method("void set_motor_target(const quaternion &in)", &physics::ct_constraint::set_motor_target);
				vct_constraint->set_method("void set_motor_target_in_constraint_space(const quaternion &in)", &physics::ct_constraint::set_motor_target_in_constraint_space);
				vct_constraint->set_method("vector3 get_point_for_angle(float, float) const", &physics::ct_constraint::get_point_for_angle);
				vct_constraint->set_method("quaternion get_motor_target() const", &physics::ct_constraint::get_motor_target);
				vct_constraint->set_method("int get_solve_twist_limit() const", &physics::ct_constraint::get_solve_twist_limit);
				vct_constraint->set_method("int get_solve_swing_limit() const", &physics::ct_constraint::get_solve_swing_limit);
				vct_constraint->set_method("float get_twist_limit_sign() const", &physics::ct_constraint::get_twist_limit_sign);
				vct_constraint->set_method("float get_swing_span1() const", &physics::ct_constraint::get_swing_span1);
				vct_constraint->set_method("float get_swing_span2() const", &physics::ct_constraint::get_swing_span2);
				vct_constraint->set_method("float get_twist_span() const", &physics::ct_constraint::get_twist_span);
				vct_constraint->set_method("float get_limit_softness() const", &physics::ct_constraint::get_limit_softness);
				vct_constraint->set_method("float get_bias_factor() const", &physics::ct_constraint::get_bias_factor);
				vct_constraint->set_method("float get_relaxation_factor() const", &physics::ct_constraint::get_relaxation_factor);
				vct_constraint->set_method("float get_twist_angle() const", &physics::ct_constraint::get_twist_angle);
				vct_constraint->set_method("float get_limit(int) const", &physics::ct_constraint::get_limit);
				vct_constraint->set_method("float get_damping() const", &physics::ct_constraint::get_damping);
				vct_constraint->set_method("float get_max_motor_impulse() const", &physics::ct_constraint::get_max_motor_impulse);
				vct_constraint->set_method("float get_fix_thresh() const", &physics::ct_constraint::get_fix_thresh);
				vct_constraint->set_method("bool is_motor_enabled() const", &physics::ct_constraint::is_motor_enabled);
				vct_constraint->set_method("bool is_max_motor_impulse_normalized() const", &physics::ct_constraint::is_max_motor_impulse_normalized);
				vct_constraint->set_method("bool is_past_swing_limit() const", &physics::ct_constraint::is_past_swing_limit);
				vct_constraint->set_method("bool is_angular_only() const", &physics::ct_constraint::is_angular_only);
				vct_constraint->set_method("physics_ctconstraint_desc& get_state()", &physics::ct_constraint::get_state);

				auto vdf6_constraint_desc = vm->set_pod<physics::df6_constraint::desc>("physics_df6constraint_desc");
				vdf6_constraint_desc->set_property<physics::df6_constraint::desc>("physics_rigidbody@ target_a", &physics::df6_constraint::desc::target_a);
				vdf6_constraint_desc->set_property<physics::df6_constraint::desc>("physics_rigidbody@ target_b", &physics::df6_constraint::desc::target_b);
				vdf6_constraint_desc->set_property<physics::df6_constraint::desc>("bool collisions", &physics::df6_constraint::desc::collisions);
				vdf6_constraint_desc->set_constructor<physics::df6_constraint::desc>("void f()");

				auto vdf6_constraint = vm->set_class<physics::df6_constraint>("physics_df6constraint", false);
				vdf6_constraint->set_method("physics_df6constraint@ copy() const", &physics::df6_constraint::copy);
				vdf6_constraint->set_method("physics_simulator@+ get_simulator() const", &physics::df6_constraint::get_simulator);
				vdf6_constraint->set_method("uptr@ get() const", &physics::df6_constraint::get);
				vdf6_constraint->set_method("uptr@ get_first() const", &physics::df6_constraint::get_first);
				vdf6_constraint->set_method("uptr@ get_second() const", &physics::df6_constraint::get_second);
				vdf6_constraint->set_method("bool has_collisions() const", &physics::df6_constraint::has_collisions);
				vdf6_constraint->set_method("bool is_enabled() const", &physics::df6_constraint::is_enabled);
				vdf6_constraint->set_method("bool is_active() const", &physics::df6_constraint::is_active);
				vdf6_constraint->set_method("void set_breaking_impulse_threshold(float)", &physics::df6_constraint::set_breaking_impulse_threshold);
				vdf6_constraint->set_method("void set_enabled(bool)", &physics::df6_constraint::set_enabled);
				vdf6_constraint->set_method("float get_breaking_impulse_threshold() const", &physics::df6_constraint::get_breaking_impulse_threshold);
				vdf6_constraint->set_method("void enable_motor(int, bool)", &physics::df6_constraint::enable_motor);
				vdf6_constraint->set_method("void enable_spring(int, bool)", &physics::df6_constraint::enable_spring);
				vdf6_constraint->set_method("void set_frames(const matrix4x4 &in, const matrix4x4 &in)", &physics::df6_constraint::set_frames);
				vdf6_constraint->set_method("void set_linear_lower_limit(const vector3 &in)", &physics::df6_constraint::set_linear_lower_limit);
				vdf6_constraint->set_method("void set_linear_upper_limit(const vector3 &in)", &physics::df6_constraint::set_linear_upper_limit);
				vdf6_constraint->set_method("void set_angular_lower_limit(const vector3 &in)", &physics::df6_constraint::set_angular_lower_limit);
				vdf6_constraint->set_method("void set_angular_lower_limit_reversed(const vector3 &in)", &physics::df6_constraint::set_angular_lower_limit_reversed);
				vdf6_constraint->set_method("void set_angular_upper_limit(const vector3 &in)", &physics::df6_constraint::set_angular_upper_limit);
				vdf6_constraint->set_method("void set_angular_upper_limit_reversed(const vector3 &in)", &physics::df6_constraint::set_angular_upper_limit_reversed);
				vdf6_constraint->set_method("void set_limit(int, float, float)", &physics::df6_constraint::set_limit);
				vdf6_constraint->set_method("void set_limit_reversed(int, float, float)", &physics::df6_constraint::set_limit_reversed);
				vdf6_constraint->set_method("void set_rotation_order(physics_rotator)", &physics::df6_constraint::set_rotation_order);
				vdf6_constraint->set_method("void set_axis(const vector3 &in, const vector3 &in)", &physics::df6_constraint::set_axis);
				vdf6_constraint->set_method("void set_bounce(int, float)", &physics::df6_constraint::set_bounce);
				vdf6_constraint->set_method("void set_servo(int, bool)", &physics::df6_constraint::set_servo);
				vdf6_constraint->set_method("void set_target_velocity(int, float)", &physics::df6_constraint::set_target_velocity);
				vdf6_constraint->set_method("void set_servo_target(int, float)", &physics::df6_constraint::set_servo_target);
				vdf6_constraint->set_method("void set_max_motor_force(int, float)", &physics::df6_constraint::set_max_motor_force);
				vdf6_constraint->set_method("void set_stiffness(int, float, bool = true)", &physics::df6_constraint::set_stiffness);
				vdf6_constraint->set_method<physics::df6_constraint, void>("void set_equilibrium_point()", &physics::df6_constraint::set_equilibrium_point);
				vdf6_constraint->set_method<physics::df6_constraint, void, int>("void set_equilibrium_point(int)", &physics::df6_constraint::set_equilibrium_point);
				vdf6_constraint->set_method<physics::df6_constraint, void, int, float>("void set_equilibrium_point(int, float)", &physics::df6_constraint::set_equilibrium_point);
				vdf6_constraint->set_method("vector3 get_angular_upper_limit() const", &physics::df6_constraint::get_angular_upper_limit);
				vdf6_constraint->set_method("vector3 get_angular_upper_limit_reversed() const", &physics::df6_constraint::get_angular_upper_limit_reversed);
				vdf6_constraint->set_method("vector3 get_angular_lower_limit() const", &physics::df6_constraint::get_angular_lower_limit);
				vdf6_constraint->set_method("vector3 get_angular_lower_limit_reversed() const", &physics::df6_constraint::get_angular_lower_limit_reversed);
				vdf6_constraint->set_method("vector3 get_linear_upper_limit() const", &physics::df6_constraint::get_linear_upper_limit);
				vdf6_constraint->set_method("vector3 get_linear_lower_limit() const", &physics::df6_constraint::get_linear_lower_limit);
				vdf6_constraint->set_method("vector3 get_axis(int) const", &physics::df6_constraint::get_axis);
				vdf6_constraint->set_method("physics_rotator get_rotation_order() const", &physics::df6_constraint::get_rotation_order);
				vdf6_constraint->set_method("float get_angle(int) const", &physics::df6_constraint::get_angle);
				vdf6_constraint->set_method("float get_relative_pivot_position(int) const", &physics::df6_constraint::get_relative_pivot_position);
				vdf6_constraint->set_method("bool is_limited(int) const", &physics::df6_constraint::is_limited);
				vdf6_constraint->set_method("physics_df6constraint_desc& get_state()", &physics::df6_constraint::get_state);

				auto vsimulator_desc = vm->set_pod<physics::simulator::desc>("physics_simulator_desc");
				vsimulator_desc->set_property<physics::simulator::desc>("vector3 water_normal", &physics::simulator::desc::water_normal);
				vsimulator_desc->set_property<physics::simulator::desc>("vector3 gravity", &physics::simulator::desc::gravity);
				vsimulator_desc->set_property<physics::simulator::desc>("float air_density", &physics::simulator::desc::air_density);
				vsimulator_desc->set_property<physics::simulator::desc>("float water_density", &physics::simulator::desc::water_density);
				vsimulator_desc->set_property<physics::simulator::desc>("float water_offset", &physics::simulator::desc::water_offset);
				vsimulator_desc->set_property<physics::simulator::desc>("float max_displacement", &physics::simulator::desc::max_displacement);
				vsimulator_desc->set_property<physics::simulator::desc>("bool enable_soft_body", &physics::simulator::desc::enable_soft_body);
				vsimulator_desc->set_constructor<physics::simulator::desc>("void f()");

				vsimulator->set_property<physics::simulator>("float speedup", &physics::simulator::speedup);
				vsimulator->set_property<physics::simulator>("bool active", &physics::simulator::active);
				vsimulator->set_constructor<physics::simulator, const physics::simulator::desc&>("physics_simulator@ f(const physics_simulator_desc &in)");
				vsimulator->set_method("void set_gravity(const vector3 &in)", &physics::simulator::set_gravity);
				vsimulator->set_method<physics::simulator, void, const trigonometry::vector3&, bool>("void set_linear_impulse(const vector3 &in, bool = false)", &physics::simulator::set_linear_impulse);
				vsimulator->set_method<physics::simulator, void, const trigonometry::vector3&, int, int, bool>("void set_linear_impulse(const vector3 &in, int, int, bool = false)", &physics::simulator::set_linear_impulse);
				vsimulator->set_method<physics::simulator, void, const trigonometry::vector3&, bool>("void set_angular_impulse(const vector3 &in, bool = false)", &physics::simulator::set_angular_impulse);
				vsimulator->set_method<physics::simulator, void, const trigonometry::vector3&, int, int, bool>("void set_angular_impulse(const vector3 &in, int, int, bool = false)", &physics::simulator::set_angular_impulse);
				vsimulator->set_method<physics::simulator, void, const trigonometry::vector3&, bool>("void create_linear_impulse(const vector3 &in, bool = false)", &physics::simulator::create_linear_impulse);
				vsimulator->set_method<physics::simulator, void, const trigonometry::vector3&, int, int, bool>("void create_linear_impulse(const vector3 &in, int, int, bool = false)", &physics::simulator::create_linear_impulse);
				vsimulator->set_method<physics::simulator, void, const trigonometry::vector3&, bool>("void create_angular_impulse(const vector3 &in, bool = false)", &physics::simulator::create_angular_impulse);
				vsimulator->set_method<physics::simulator, void, const trigonometry::vector3&, int, int, bool>("void create_angular_impulse(const vector3 &in, int, int, bool = false)", &physics::simulator::create_angular_impulse);
				vsimulator->set_method("void add_softbody(physics_softbody@+)", &physics::simulator::add_soft_body);
				vsimulator->set_method("void remove_softbody(physics_softbody@+)", &physics::simulator::remove_soft_body);
				vsimulator->set_method("void add_rigidbody(physics_rigidbody@+)", &physics::simulator::add_rigid_body);
				vsimulator->set_method("void remove_rigidbody(physics_rigidbody@+)", &physics::simulator::remove_rigid_body);
				vsimulator->set_method("void add_constraint(physics_constraint@+)", &physics::simulator::add_constraint);
				vsimulator->set_method("void remove_constraint(physics_constraint@+)", &physics::simulator::remove_constraint);
				vsimulator->set_method("void remove_all()", &physics::simulator::remove_all);
				vsimulator->set_method("void simulate_step(float)", &physics::simulator::simulate_step);
				vsimulator->set_method<physics::simulator, physics::rigid_body*, const physics::rigid_body::desc&>("physics_rigidbody@ create_rigidbody(const physics_rigidbody_desc &in)", &physics::simulator::create_rigid_body);
				vsimulator->set_method<physics::simulator, physics::rigid_body*, const physics::rigid_body::desc&, trigonometry::transform*>("physics_rigidbody@ create_rigidbody(const physics_rigidbody_desc &in, transform@+)", &physics::simulator::create_rigid_body);
				vsimulator->set_method<physics::simulator, physics::soft_body*, const physics::soft_body::desc&>("physics_softbody@ create_softbody(const physics_softbody_desc &in)", &physics::simulator::create_soft_body);
				vsimulator->set_method<physics::simulator, physics::soft_body*, const physics::soft_body::desc&, trigonometry::transform*>("physics_softbody@ create_softbody(const physics_softbody_desc &in, transform@+)", &physics::simulator::create_soft_body);
				vsimulator->set_method("physics_pconstraint@ create_pconstraint(const physics_pconstraint_desc &in)", &physics::simulator::create_point2_point_constraint);
				vsimulator->set_method("physics_hconstraint@ create_hconstraint(const physics_hconstraint_desc &in)", &physics::simulator::create_hinge_constraint);
				vsimulator->set_method("physics_sconstraint@ create_sconstraint(const physics_sconstraint_desc &in)", &physics::simulator::create_slider_constraint);
				vsimulator->set_method("physics_ctconstraint@ create_ctconstraint(const physics_ctconstraint_desc &in)", &physics::simulator::create_cone_twist_constraint);
				vsimulator->set_method("physics_df6constraint@ create_6dof_constraint(const physics_df6constraint_desc &in)", &physics::simulator::create_6dof_constraint);
				vsimulator->set_method("uptr@ create_shape(physics_shape)", &physics::simulator::create_shape);
				vsimulator->set_method("uptr@ create_cube(const vector3 &in = vector3(1, 1, 1))", &physics::simulator::create_cube);
				vsimulator->set_method("uptr@ create_sphere(float = 1)", &physics::simulator::create_sphere);
				vsimulator->set_method("uptr@ create_capsule(float = 1, float = 1)", &physics::simulator::create_capsule);
				vsimulator->set_method("uptr@ create_cone(float = 1, float = 1)", &physics::simulator::create_cone);
				vsimulator->set_method("uptr@ create_cylinder(const vector3 &in = vector3(1, 1, 1))", &physics::simulator::create_cylinder);
				vsimulator->set_method_ex("uptr@ create_convex_hull(array<skin_vertex>@+)", &simulator_create_convex_hull_skin_vertex);
				vsimulator->set_method_ex("uptr@ create_convex_hull(array<vertex>@+)", &simulator_create_convex_hull_vertex);
				vsimulator->set_method_ex("uptr@ create_convex_hull(array<vector2>@+)", &simulator_create_convex_hull_vector2);
				vsimulator->set_method_ex("uptr@ create_convex_hull(array<vector3>@+)", &simulator_create_convex_hull_vector3);
				vsimulator->set_method_ex("uptr@ create_convex_hull(array<vector4>@+)", &simulator_create_convex_hull_vector4);
				vsimulator->set_method<physics::simulator, btCollisionShape*, btCollisionShape*>("uptr@ create_convex_hull(uptr@)", &physics::simulator::create_convex_hull);
				vsimulator->set_method("uptr@ try_clone_shape(uptr@)", &physics::simulator::try_clone_shape);
				vsimulator->set_method("uptr@ reuse_shape(uptr@)", &physics::simulator::reuse_shape);
				vsimulator->set_method("void free_shape(uptr@)", &physics::simulator::free_shape);
				vsimulator->set_method_ex("array<vector3>@ get_shape_vertices(uptr@) const", &simulator_get_shape_vertices);
				vsimulator->set_method("usize get_shape_vertices_count(uptr@) const", &physics::simulator::get_shape_vertices_count);
				vsimulator->set_method("float get_max_displacement() const", &physics::simulator::get_max_displacement);
				vsimulator->set_method("float get_air_density() const", &physics::simulator::get_air_density);
				vsimulator->set_method("float get_water_offset() const", &physics::simulator::get_water_offset);
				vsimulator->set_method("float get_water_density() const", &physics::simulator::get_water_density);
				vsimulator->set_method("vector3 get_water_normal() const", &physics::simulator::get_water_normal);
				vsimulator->set_method("vector3 get_gravity() const", &physics::simulator::get_gravity);
				vsimulator->set_method("bool has_softbody_support() const", &physics::simulator::has_soft_body_support);
				vsimulator->set_method("int get_contact_manifold_count() const", &physics::simulator::get_contact_manifold_count);

				vconstraint->set_dynamic_cast<physics::constraint, physics::pconstraint>("physics_pconstraint@+");
				vconstraint->set_dynamic_cast<physics::constraint, physics::hconstraint>("physics_hconstraint@+");
				vconstraint->set_dynamic_cast<physics::constraint, physics::sconstraint>("physics_sconstraint@+");
				vconstraint->set_dynamic_cast<physics::constraint, physics::ct_constraint>("physics_ctconstraint@+");
				vconstraint->set_dynamic_cast<physics::constraint, physics::df6_constraint>("physics_df6constraint@+");
				vp_constraint->set_dynamic_cast<physics::pconstraint, physics::constraint>("physics_constraint@+", true);
				vh_constraint->set_dynamic_cast<physics::hconstraint, physics::constraint>("physics_constraint@+", true);
				vs_constraint->set_dynamic_cast<physics::sconstraint, physics::constraint>("physics_constraint@+", true);
				vct_constraint->set_dynamic_cast<physics::ct_constraint, physics::constraint>("physics_constraint@+", true);
				vdf6_constraint->set_dynamic_cast<physics::df6_constraint, physics::constraint>("physics_constraint@+", true);

				return true;
#else
				VI_ASSERT(false, "<physics> is not loaded");
				return false;
#endif
			}
			bool heavy_registry::import_graphics(virtual_machine* vm) noexcept
			{
#ifdef VI_BINDINGS
				VI_ASSERT(vm != nullptr, "manager should be set");
				auto vv_sync = vm->set_enum("vsync");
				vv_sync->set_value("off", (int)graphics::vsync::off);
				vv_sync->set_value("frequency_x1", (int)graphics::vsync::frequency_x1);
				vv_sync->set_value("frequency_x2", (int)graphics::vsync::frequency_x2);
				vv_sync->set_value("frequency_x3", (int)graphics::vsync::frequency_x3);
				vv_sync->set_value("frequency_x4", (int)graphics::vsync::frequency_x4);
				vv_sync->set_value("on", (int)graphics::vsync::on);

				auto vsurface_target = vm->set_enum("surface_target");
				vsurface_target->set_value("t0", (int)graphics::surface_target::t0);
				vsurface_target->set_value("t1", (int)graphics::surface_target::t1);
				vsurface_target->set_value("t2", (int)graphics::surface_target::t2);
				vsurface_target->set_value("t3", (int)graphics::surface_target::t3);
				vsurface_target->set_value("t4", (int)graphics::surface_target::t4);
				vsurface_target->set_value("t5", (int)graphics::surface_target::t5);
				vsurface_target->set_value("t6", (int)graphics::surface_target::t6);
				vsurface_target->set_value("t7", (int)graphics::surface_target::t7);

				auto vprimitive_topology = vm->set_enum("primitive_topology");
				vprimitive_topology->set_value("invalid", (int)graphics::primitive_topology::invalid);
				vprimitive_topology->set_value("point_list", (int)graphics::primitive_topology::point_list);
				vprimitive_topology->set_value("line_list", (int)graphics::primitive_topology::line_list);
				vprimitive_topology->set_value("line_strip", (int)graphics::primitive_topology::line_strip);
				vprimitive_topology->set_value("triangle_list", (int)graphics::primitive_topology::triangle_list);
				vprimitive_topology->set_value("triangle_strip", (int)graphics::primitive_topology::triangle_strip);
				vprimitive_topology->set_value("line_list_adj", (int)graphics::primitive_topology::line_list_adj);
				vprimitive_topology->set_value("line_strip_adj", (int)graphics::primitive_topology::line_strip_adj);
				vprimitive_topology->set_value("triangle_list_adj", (int)graphics::primitive_topology::triangle_list_adj);
				vprimitive_topology->set_value("triangle_strip_adj", (int)graphics::primitive_topology::triangle_strip_adj);

				auto vformat = vm->set_enum("surface_format");
				vformat->set_value("unknown", (int)graphics::format::unknown);
				vformat->set_value("A8_unorm", (int)graphics::format::a8_unorm);
				vformat->set_value("D16_unorm", (int)graphics::format::d16_unorm);
				vformat->set_value("D24_unorm_S8_uint", (int)graphics::format::d24_unorm_s8_uint);
				vformat->set_value("D32_float", (int)graphics::format::d32_float);
				vformat->set_value("R10G10B10A2_uint", (int)graphics::format::r10g10b10a2_uint);
				vformat->set_value("R10G10B10A2_unorm", (int)graphics::format::r10g10b10a2_unorm);
				vformat->set_value("R11G11B10_float", (int)graphics::format::r11g11b10_float);
				vformat->set_value("R16G16B16A16_float", (int)graphics::format::r16g16b16a16_float);
				vformat->set_value("R16G16B16A16_sint", (int)graphics::format::r16g16b16a16_sint);
				vformat->set_value("R16G16B16A16_snorm", (int)graphics::format::r16g16b16a16_snorm);
				vformat->set_value("R16G16B16A16_uint", (int)graphics::format::r16g16b16a16_uint);
				vformat->set_value("R16G16B16A16_unorm", (int)graphics::format::r16g16b16a16_unorm);
				vformat->set_value("R16G16_float", (int)graphics::format::r16g16_float);
				vformat->set_value("R16G16_sint", (int)graphics::format::r16g16_sint);
				vformat->set_value("R16G16_snorm", (int)graphics::format::r16g16_snorm);
				vformat->set_value("R16G16_uint", (int)graphics::format::r16g16_uint);
				vformat->set_value("R16G16_unorm", (int)graphics::format::r16g16_unorm);
				vformat->set_value("R16_float", (int)graphics::format::r16_float);
				vformat->set_value("R16_sint", (int)graphics::format::r16_sint);
				vformat->set_value("R16_snorm", (int)graphics::format::r16_snorm);
				vformat->set_value("R16_uint", (int)graphics::format::r16_uint);
				vformat->set_value("R16_unorm", (int)graphics::format::r16_unorm);
				vformat->set_value("R1_unorm", (int)graphics::format::r1_unorm);
				vformat->set_value("R32G32B32A32_float", (int)graphics::format::r32g32b32a32_float);
				vformat->set_value("R32G32B32A32_sint", (int)graphics::format::r32g32b32a32_sint);
				vformat->set_value("R32G32B32A32_uint", (int)graphics::format::r32g32b32a32_uint);
				vformat->set_value("R32G32B32_float", (int)graphics::format::r32g32b32_float);
				vformat->set_value("R32G32B32_sint", (int)graphics::format::r32g32b32_sint);
				vformat->set_value("R32G32B32_uint", (int)graphics::format::r32g32b32_uint);
				vformat->set_value("R32G32_float", (int)graphics::format::r32g32_float);
				vformat->set_value("R32G32_sint", (int)graphics::format::r32g32_sint);
				vformat->set_value("R32G32_uint", (int)graphics::format::r32g32_uint);
				vformat->set_value("R32_float", (int)graphics::format::r32_float);
				vformat->set_value("R32_sint", (int)graphics::format::r32_sint);
				vformat->set_value("R32_uint", (int)graphics::format::r32_uint);
				vformat->set_value("R8G8B8A8_sint", (int)graphics::format::r8g8b8a8_sint);
				vformat->set_value("R8G8B8A8_snorm", (int)graphics::format::r8g8b8a8_snorm);
				vformat->set_value("R8G8B8A8_uint", (int)graphics::format::r8g8b8a8_uint);
				vformat->set_value("R8G8B8A8_unorm", (int)graphics::format::r8g8b8a8_unorm);
				vformat->set_value("R8G8B8A8_unorm_SRGB", (int)graphics::format::r8g8b8a8_unorm_srgb);
				vformat->set_value("R8G8_B8G8_unorm", (int)graphics::format::r8g8b8g8_unorm);
				vformat->set_value("R8G8_sint", (int)graphics::format::r8g8_sint);
				vformat->set_value("R8G8_snorm", (int)graphics::format::r8g8_snorm);
				vformat->set_value("R8G8_uint", (int)graphics::format::r8g8_uint);
				vformat->set_value("R8G8_unorm", (int)graphics::format::r8g8_unorm);
				vformat->set_value("R8_sint", (int)graphics::format::r8_sint);
				vformat->set_value("R8_snorm", (int)graphics::format::r8_snorm);
				vformat->set_value("R8_uint", (int)graphics::format::r8_uint);
				vformat->set_value("R8_unorm", (int)graphics::format::r8_unorm);
				vformat->set_value("R9G9B9E5_share_dexp", (int)graphics::format::r9g9b9e5_share_dexp);

				auto vresource_map = vm->set_enum("resource_map");
				vresource_map->set_value("read", (int)graphics::resource_map::read);
				vresource_map->set_value("write", (int)graphics::resource_map::write);
				vresource_map->set_value("read_write", (int)graphics::resource_map::read_write);
				vresource_map->set_value("write_discard", (int)graphics::resource_map::write_discard);
				vresource_map->set_value("write_no_overwrite", (int)graphics::resource_map::write_no_overwrite);

				auto vresource_usage = vm->set_enum("resource_usage");
				vresource_usage->set_value("default_t", (int)graphics::resource_usage::defaults);
				vresource_usage->set_value("immutable", (int)graphics::resource_usage::immutable);
				vresource_usage->set_value("dynamic", (int)graphics::resource_usage::dynamic);
				vresource_usage->set_value("staging", (int)graphics::resource_usage::staging);

				auto vshader_model = vm->set_enum("shader_model");
				vshader_model->set_value("invalid", (int)graphics::shader_model::invalid);
				vshader_model->set_value("auto_t", (int)graphics::shader_model::any);
				vshader_model->set_value("hlsl_1_0", (int)graphics::shader_model::hlsl_1_0);
				vshader_model->set_value("hlsl_2_0", (int)graphics::shader_model::hlsl_2_0);
				vshader_model->set_value("hlsl_3_0", (int)graphics::shader_model::hlsl_3_0);
				vshader_model->set_value("hlsl_4_0", (int)graphics::shader_model::hlsl_4_0);
				vshader_model->set_value("hlsl_4_1", (int)graphics::shader_model::hlsl_4_1);
				vshader_model->set_value("hlsl_5_0", (int)graphics::shader_model::hlsl_5_0);
				vshader_model->set_value("glsl_1_1_0", (int)graphics::shader_model::glsl_1_1_0);
				vshader_model->set_value("glsl_1_2_0", (int)graphics::shader_model::glsl_1_2_0);
				vshader_model->set_value("glsl_1_3_0", (int)graphics::shader_model::glsl_1_3_0);
				vshader_model->set_value("glsl_1_4_0", (int)graphics::shader_model::glsl_1_4_0);
				vshader_model->set_value("glsl_1_5_0", (int)graphics::shader_model::glsl_1_5_0);
				vshader_model->set_value("glsl_3_3_0", (int)graphics::shader_model::glsl_3_3_0);
				vshader_model->set_value("glsl_4_0_0", (int)graphics::shader_model::glsl_4_0_0);
				vshader_model->set_value("glsl_4_1_0", (int)graphics::shader_model::glsl_4_1_0);
				vshader_model->set_value("glsl_4_2_0", (int)graphics::shader_model::glsl_4_2_0);
				vshader_model->set_value("glsl_4_3_0", (int)graphics::shader_model::glsl_4_3_0);
				vshader_model->set_value("glsl_4_4_0", (int)graphics::shader_model::glsl_4_4_0);
				vshader_model->set_value("glsl_4_5_0", (int)graphics::shader_model::glsl_4_5_0);
				vshader_model->set_value("glsl_4_6_0", (int)graphics::shader_model::glsl_4_6_0);

				auto vresource_bind = vm->set_enum("resource_bind");
				vresource_bind->set_value("vertex_buffer", (int)graphics::resource_bind::vertex_buffer);
				vresource_bind->set_value("index_buffer", (int)graphics::resource_bind::index_buffer);
				vresource_bind->set_value("constant_buffer", (int)graphics::resource_bind::constant_buffer);
				vresource_bind->set_value("shader_input", (int)graphics::resource_bind::shader_input);
				vresource_bind->set_value("stream_output", (int)graphics::resource_bind::stream_output);
				vresource_bind->set_value("render_target", (int)graphics::resource_bind::render_target);
				vresource_bind->set_value("depth_stencil", (int)graphics::resource_bind::depth_stencil);
				vresource_bind->set_value("unordered_access", (int)graphics::resource_bind::unordered_access);

				auto vcpu_access = vm->set_enum("cpu_access");
				vcpu_access->set_value("none", (int)graphics::cpu_access::none);
				vcpu_access->set_value("write", (int)graphics::cpu_access::write);
				vcpu_access->set_value("read", (int)graphics::cpu_access::read);

				auto vdepth_write = vm->set_enum("depth_write");
				vdepth_write->set_value("zero", (int)graphics::depth_write::zero);
				vdepth_write->set_value("all", (int)graphics::depth_write::all);

				auto vcomparison = vm->set_enum("comparison");
				vcomparison->set_value("never", (int)graphics::comparison::never);
				vcomparison->set_value("less", (int)graphics::comparison::less);
				vcomparison->set_value("equal", (int)graphics::comparison::equal);
				vcomparison->set_value("less_equal", (int)graphics::comparison::less_equal);
				vcomparison->set_value("greater", (int)graphics::comparison::greater);
				vcomparison->set_value("not_equal", (int)graphics::comparison::not_equal);
				vcomparison->set_value("greater_equal", (int)graphics::comparison::greater_equal);
				vcomparison->set_value("always", (int)graphics::comparison::always);

				auto vstencil_operation = vm->set_enum("stencil_operation");
				vstencil_operation->set_value("keep", (int)graphics::stencil_operation::keep);
				vstencil_operation->set_value("zero", (int)graphics::stencil_operation::zero);
				vstencil_operation->set_value("replace", (int)graphics::stencil_operation::replace);
				vstencil_operation->set_value("sat_add", (int)graphics::stencil_operation::sat_add);
				vstencil_operation->set_value("sat_subtract", (int)graphics::stencil_operation::sat_subtract);
				vstencil_operation->set_value("invert", (int)graphics::stencil_operation::invert);
				vstencil_operation->set_value("add", (int)graphics::stencil_operation::add);
				vstencil_operation->set_value("subtract", (int)graphics::stencil_operation::subtract);

				auto vblend = vm->set_enum("blend_t");
				vblend->set_value("zero", (int)graphics::blend::zero);
				vblend->set_value("one", (int)graphics::blend::one);
				vblend->set_value("source_color", (int)graphics::blend::source_color);
				vblend->set_value("source_color_invert", (int)graphics::blend::source_color_invert);
				vblend->set_value("source_alpha", (int)graphics::blend::source_alpha);
				vblend->set_value("source_alpha_invert", (int)graphics::blend::source_alpha_invert);
				vblend->set_value("destination_alpha", (int)graphics::blend::destination_alpha);
				vblend->set_value("destination_alpha_invert", (int)graphics::blend::destination_alpha_invert);
				vblend->set_value("destination_color", (int)graphics::blend::destination_color);
				vblend->set_value("destination_color_invert", (int)graphics::blend::destination_color_invert);
				vblend->set_value("source_alpha_sat", (int)graphics::blend::source_alpha_sat);
				vblend->set_value("blend_factor", (int)graphics::blend::blend_factor);
				vblend->set_value("blend_factor_invert", (int)graphics::blend::blend_factor_invert);
				vblend->set_value("source1_color", (int)graphics::blend::source1_color);
				vblend->set_value("source1_color_invert", (int)graphics::blend::source1_color_invert);
				vblend->set_value("source1_alpha", (int)graphics::blend::source1_alpha);
				vblend->set_value("source1_alpha_invert", (int)graphics::blend::source1_alpha_invert);

				auto vsurface_fill = vm->set_enum("surface_fill");
				vsurface_fill->set_value("wireframe", (int)graphics::surface_fill::wireframe);
				vsurface_fill->set_value("solid", (int)graphics::surface_fill::solid);

				auto vpixel_filter = vm->set_enum("pixel_filter");
				vpixel_filter->set_value("min_mag_mip_point", (int)graphics::pixel_filter::min_mag_mip_point);
				vpixel_filter->set_value("min_mag_point_mip_linear", (int)graphics::pixel_filter::min_mag_point_mip_linear);
				vpixel_filter->set_value("min_point_mag_linear_mip_point", (int)graphics::pixel_filter::min_point_mag_linear_mip_point);
				vpixel_filter->set_value("min_point_mag_mip_linear", (int)graphics::pixel_filter::min_point_mag_mip_linear);
				vpixel_filter->set_value("min_linear_mag_mip_point", (int)graphics::pixel_filter::min_linear_mag_mip_point);
				vpixel_filter->set_value("min_linear_mag_point_mip_linear", (int)graphics::pixel_filter::min_linear_mag_point_mip_linear);
				vpixel_filter->set_value("min_mag_linear_mip_point", (int)graphics::pixel_filter::min_mag_linear_mip_point);
				vpixel_filter->set_value("min_mag_mip_minear", (int)graphics::pixel_filter::min_mag_mip_linear);
				vpixel_filter->set_value("anistropic", (int)graphics::pixel_filter::anistropic);
				vpixel_filter->set_value("compare_min_mag_mip_point", (int)graphics::pixel_filter::compare_min_mag_mip_point);
				vpixel_filter->set_value("compare_min_mag_point_mip_linear", (int)graphics::pixel_filter::compare_min_mag_point_mip_linear);
				vpixel_filter->set_value("compare_min_point_mag_linear_mip_point", (int)graphics::pixel_filter::compare_min_point_mag_linear_mip_point);
				vpixel_filter->set_value("compare_min_point_mag_mip_linear", (int)graphics::pixel_filter::compare_min_point_mag_mip_linear);
				vpixel_filter->set_value("compare_min_linear_mag_mip_point", (int)graphics::pixel_filter::compare_min_linear_mag_mip_point);
				vpixel_filter->set_value("compare_min_linear_mag_point_mip_linear", (int)graphics::pixel_filter::compare_min_linear_mag_point_mip_linear);
				vpixel_filter->set_value("compare_min_mag_linear_mip_point", (int)graphics::pixel_filter::compare_min_mag_linear_mip_point);
				vpixel_filter->set_value("compare_min_mag_mip_linear", (int)graphics::pixel_filter::compare_min_mag_mip_linear);
				vpixel_filter->set_value("compare_anistropic", (int)graphics::pixel_filter::compare_anistropic);

				auto vtexture_address = vm->set_enum("texture_address");
				vtexture_address->set_value("wrap", (int)graphics::texture_address::wrap);
				vtexture_address->set_value("mirror", (int)graphics::texture_address::mirror);
				vtexture_address->set_value("clamp", (int)graphics::texture_address::clamp);
				vtexture_address->set_value("border", (int)graphics::texture_address::border);
				vtexture_address->set_value("mirror_once", (int)graphics::texture_address::mirror_once);

				auto vcolor_write_enable = vm->set_enum("color_write_enable");
				vcolor_write_enable->set_value("red", (int)graphics::color_write_enable::red);
				vcolor_write_enable->set_value("green", (int)graphics::color_write_enable::green);
				vcolor_write_enable->set_value("blue", (int)graphics::color_write_enable::blue);
				vcolor_write_enable->set_value("alpha", (int)graphics::color_write_enable::alpha);
				vcolor_write_enable->set_value("all", (int)graphics::color_write_enable::all);

				auto vblend_operation = vm->set_enum("blend_operation");
				vblend_operation->set_value("add", (int)graphics::blend_operation::add);
				vblend_operation->set_value("subtract", (int)graphics::blend_operation::subtract);
				vblend_operation->set_value("subtract_reverse", (int)graphics::blend_operation::subtract_reverse);
				vblend_operation->set_value("min", (int)graphics::blend_operation::min);
				vblend_operation->set_value("max", (int)graphics::blend_operation::max);

				auto vvertex_cull = vm->set_enum("vertex_cull");
				vvertex_cull->set_value("none", (int)graphics::vertex_cull::none);
				vvertex_cull->set_value("front", (int)graphics::vertex_cull::front);
				vvertex_cull->set_value("back", (int)graphics::vertex_cull::back);

				auto vshader_compile = vm->set_enum("shader_compile");
				vshader_compile->set_value("debug", (int)graphics::shader_compile::debug);
				vshader_compile->set_value("skip_validation", (int)graphics::shader_compile::skip_validation);
				vshader_compile->set_value("skip_optimization", (int)graphics::shader_compile::skip_optimization);
				vshader_compile->set_value("matrix_row_major", (int)graphics::shader_compile::matrix_row_major);
				vshader_compile->set_value("matrix_column_major", (int)graphics::shader_compile::matrix_column_major);
				vshader_compile->set_value("partial_precision", (int)graphics::shader_compile::partial_precision);
				vshader_compile->set_value("foe_vs_no_opt", (int)graphics::shader_compile::foevs_no_opt);
				vshader_compile->set_value("foe_ps_no_opt", (int)graphics::shader_compile::foeps_no_opt);
				vshader_compile->set_value("no_preshader", (int)graphics::shader_compile::no_preshader);
				vshader_compile->set_value("avoid_flow_control", (int)graphics::shader_compile::avoid_flow_control);
				vshader_compile->set_value("prefer_flow_control", (int)graphics::shader_compile::prefer_flow_control);
				vshader_compile->set_value("enable_strictness", (int)graphics::shader_compile::enable_strictness);
				vshader_compile->set_value("enable_backwards_compatibility", (int)graphics::shader_compile::enable_backwards_compatibility);
				vshader_compile->set_value("ieee_strictness", (int)graphics::shader_compile::ieee_strictness);
				vshader_compile->set_value("optimization_level0", (int)graphics::shader_compile::optimization_level0);
				vshader_compile->set_value("optimization_level1", (int)graphics::shader_compile::optimization_level1);
				vshader_compile->set_value("optimization_level2", (int)graphics::shader_compile::optimization_level2);
				vshader_compile->set_value("optimization_level3", (int)graphics::shader_compile::optimization_level3);
				vshader_compile->set_value("reseed_x16", (int)graphics::shader_compile::reseed_x16);
				vshader_compile->set_value("reseed_x17", (int)graphics::shader_compile::reseed_x17);
				vshader_compile->set_value("picky", (int)graphics::shader_compile::picky);

				auto vresource_misc = vm->set_enum("resource_misc");
				vresource_misc->set_value("none", (int)graphics::resource_misc::none);
				vresource_misc->set_value("generate_mips", (int)graphics::resource_misc::generate_mips);
				vresource_misc->set_value("shared", (int)graphics::resource_misc::shared);
				vresource_misc->set_value("texture_cube", (int)graphics::resource_misc::texture_cube);
				vresource_misc->set_value("draw_indirect_args", (int)graphics::resource_misc::draw_indirect_args);
				vresource_misc->set_value("buffer_allow_raw_views", (int)graphics::resource_misc::buffer_allow_raw_views);
				vresource_misc->set_value("buffer_structured", (int)graphics::resource_misc::buffer_structured);
				vresource_misc->set_value("clamp", (int)graphics::resource_misc::clamp);
				vresource_misc->set_value("shared_keyed_mutex", (int)graphics::resource_misc::shared_keyed_mutex);
				vresource_misc->set_value("gdi_compatible", (int)graphics::resource_misc::gdi_compatible);
				vresource_misc->set_value("shared_nt_handle", (int)graphics::resource_misc::shared_nt_handle);
				vresource_misc->set_value("restricted_content", (int)graphics::resource_misc::restricted_content);
				vresource_misc->set_value("restrict_shared", (int)graphics::resource_misc::restrict_shared);
				vresource_misc->set_value("restrict_shared_driver", (int)graphics::resource_misc::restrict_shared_driver);
				vresource_misc->set_value("guarded", (int)graphics::resource_misc::guarded);
				vresource_misc->set_value("tile_pool", (int)graphics::resource_misc::tile_pool);
				vresource_misc->set_value("tiled", (int)graphics::resource_misc::tiled);

				auto vshader_type = vm->set_enum("shader_type");
				vshader_type->set_value("vertex", (int)graphics::shader_type::vertex);
				vshader_type->set_value("pixel", (int)graphics::shader_type::pixel);
				vshader_type->set_value("geometry", (int)graphics::shader_type::geometry);
				vshader_type->set_value("hull", (int)graphics::shader_type::hull);
				vshader_type->set_value("domain", (int)graphics::shader_type::domain);
				vshader_type->set_value("compute", (int)graphics::shader_type::compute);
				vshader_type->set_value("all", (int)graphics::shader_type::all);

				auto vshader_lang = vm->set_enum("shader_lang");
				vshader_lang->set_value("none", (int)graphics::shader_lang::none);
				vshader_lang->set_value("spv", (int)graphics::shader_lang::spv);
				vshader_lang->set_value("msl", (int)graphics::shader_lang::msl);
				vshader_lang->set_value("hlsl", (int)graphics::shader_lang::hlsl);
				vshader_lang->set_value("glsl", (int)graphics::shader_lang::glsl);
				vshader_lang->set_value("glsl_es", (int)graphics::shader_lang::glsl_es);

				auto vattribute_type = vm->set_enum("attribute_type");
				vattribute_type->set_value("byte_t", (int)graphics::attribute_type::byte);
				vattribute_type->set_value("ubyte_t", (int)graphics::attribute_type::ubyte);
				vattribute_type->set_value("half_t", (int)graphics::attribute_type::half);
				vattribute_type->set_value("float_t", (int)graphics::attribute_type::floatf);
				vattribute_type->set_value("int_t", (int)graphics::attribute_type::intf);
				vattribute_type->set_value("uint_t", (int)graphics::attribute_type::uint);
				vattribute_type->set_value("matrix_t", (int)graphics::attribute_type::matrix);

				auto vmapped_subresource = vm->set_pod<graphics::mapped_subresource>("mapped_subresource");
				vmapped_subresource->set_property<graphics::mapped_subresource>("uptr@ pointer", &graphics::mapped_subresource::pointer);
				vmapped_subresource->set_property<graphics::mapped_subresource>("uint32 row_pitch", &graphics::mapped_subresource::row_pitch);
				vmapped_subresource->set_property<graphics::mapped_subresource>("uint32 depth_pitch", &graphics::mapped_subresource::depth_pitch);
				vmapped_subresource->set_constructor<graphics::mapped_subresource>("void f()");

				auto vrender_target_blend_state = vm->set_pod<graphics::render_target_blend_state>("render_target_blend_state");
				vrender_target_blend_state->set_property<graphics::render_target_blend_state>("blend_t src_blend", &graphics::render_target_blend_state::src_blend);
				vrender_target_blend_state->set_property<graphics::render_target_blend_state>("blend_t dest_blend", &graphics::render_target_blend_state::dest_blend);
				vrender_target_blend_state->set_property<graphics::render_target_blend_state>("blend_operation blend_operation_mode", &graphics::render_target_blend_state::blend_operation_mode);
				vrender_target_blend_state->set_property<graphics::render_target_blend_state>("blend_t src_blend_alpha", &graphics::render_target_blend_state::src_blend_alpha);
				vrender_target_blend_state->set_property<graphics::render_target_blend_state>("blend_t dest_blend_alpha", &graphics::render_target_blend_state::dest_blend_alpha);
				vrender_target_blend_state->set_property<graphics::render_target_blend_state>("blend_operation blend_operation_alpha", &graphics::render_target_blend_state::blend_operation_alpha);
				vrender_target_blend_state->set_property<graphics::render_target_blend_state>("uint8 render_target_write_mask", &graphics::render_target_blend_state::render_target_write_mask);
				vrender_target_blend_state->set_property<graphics::render_target_blend_state>("bool blend_enable", &graphics::render_target_blend_state::blend_enable);
				vrender_target_blend_state->set_constructor<graphics::render_target_blend_state>("void f()");

				auto vsurface = vm->set_class<graphics::surface>("surface_handle", false);
				vsurface->set_constructor<graphics::surface>("surface_handle@ f()");
				vsurface->set_constructor<graphics::surface, SDL_Surface*>("surface_handle@ f(uptr@)");
				vsurface->set_method("void set_handle(uptr@)", &graphics::surface::set_handle);
				vsurface->set_method("void lock()", &graphics::surface::lock);
				vsurface->set_method("void unlock()", &graphics::surface::unlock);
				vsurface->set_method("int get_width() const", &graphics::surface::get_width);
				vsurface->set_method("int get_height() const", &graphics::surface::get_height);
				vsurface->set_method("int get_pitch() const", &graphics::surface::get_pitch);
				vsurface->set_method("uptr@ get_pixels() const", &graphics::surface::get_pixels);
				vsurface->set_method("uptr@ get_resource() const", &graphics::surface::get_resource);

				auto vdepth_stencil_state_desc = vm->set_pod<graphics::depth_stencil_state::desc>("depth_stencil_state_desc");
				vdepth_stencil_state_desc->set_property<graphics::depth_stencil_state::desc>("stencil_operation front_face_stencil_fail_operation", &graphics::depth_stencil_state::desc::front_face_stencil_fail_operation);
				vdepth_stencil_state_desc->set_property<graphics::depth_stencil_state::desc>("stencil_operation front_face_stencil_depth_fail_operation", &graphics::depth_stencil_state::desc::front_face_stencil_depth_fail_operation);
				vdepth_stencil_state_desc->set_property<graphics::depth_stencil_state::desc>("stencil_operation front_face_stencil_pass_operation", &graphics::depth_stencil_state::desc::front_face_stencil_pass_operation);
				vdepth_stencil_state_desc->set_property<graphics::depth_stencil_state::desc>("comparison front_face_stencil_function", &graphics::depth_stencil_state::desc::front_face_stencil_function);
				vdepth_stencil_state_desc->set_property<graphics::depth_stencil_state::desc>("stencil_operation back_face_stencil_fail_operation", &graphics::depth_stencil_state::desc::back_face_stencil_fail_operation);
				vdepth_stencil_state_desc->set_property<graphics::depth_stencil_state::desc>("stencil_operation back_face_stencil_depth_fail_operation", &graphics::depth_stencil_state::desc::back_face_stencil_depth_fail_operation);
				vdepth_stencil_state_desc->set_property<graphics::depth_stencil_state::desc>("stencil_operation back_face_stencil_pass_operation", &graphics::depth_stencil_state::desc::back_face_stencil_pass_operation);
				vdepth_stencil_state_desc->set_property<graphics::depth_stencil_state::desc>("comparison back_face_stencil_function", &graphics::depth_stencil_state::desc::back_face_stencil_function);
				vdepth_stencil_state_desc->set_property<graphics::depth_stencil_state::desc>("depth_write depth_write_mask", &graphics::depth_stencil_state::desc::depth_write_mask);
				vdepth_stencil_state_desc->set_property<graphics::depth_stencil_state::desc>("comparison depth_function", &graphics::depth_stencil_state::desc::depth_function);
				vdepth_stencil_state_desc->set_property<graphics::depth_stencil_state::desc>("uint8 stencil_read_mask", &graphics::depth_stencil_state::desc::stencil_read_mask);
				vdepth_stencil_state_desc->set_property<graphics::depth_stencil_state::desc>("uint8 stencil_write_mask", &graphics::depth_stencil_state::desc::stencil_write_mask);
				vdepth_stencil_state_desc->set_property<graphics::depth_stencil_state::desc>("bool depth_enable", &graphics::depth_stencil_state::desc::depth_enable);
				vdepth_stencil_state_desc->set_property<graphics::depth_stencil_state::desc>("bool stencil_enable", &graphics::depth_stencil_state::desc::stencil_enable);
				vdepth_stencil_state_desc->set_constructor<graphics::depth_stencil_state::desc>("void f()");

				auto vdepth_stencil_state = vm->set_class<graphics::depth_stencil_state>("depth_stencil_state", false);
				vdepth_stencil_state->set_method("uptr@ get_resource() const", &graphics::depth_stencil_state::get_resource);
				vdepth_stencil_state->set_method("depth_stencil_state_desc get_state() const", &graphics::depth_stencil_state::get_state);

				auto vrasterizer_state_desc = vm->set_pod<graphics::rasterizer_state::desc>("rasterizer_state_desc");
				vrasterizer_state_desc->set_property<graphics::rasterizer_state::desc>("surface_fill fill_mode", &graphics::rasterizer_state::desc::fill_mode);
				vrasterizer_state_desc->set_property<graphics::rasterizer_state::desc>("vertex_cull cull_mode", &graphics::rasterizer_state::desc::cull_mode);
				vrasterizer_state_desc->set_property<graphics::rasterizer_state::desc>("float depth_bias_clamp", &graphics::rasterizer_state::desc::depth_bias_clamp);
				vrasterizer_state_desc->set_property<graphics::rasterizer_state::desc>("float slope_scaled_depth_bias", &graphics::rasterizer_state::desc::slope_scaled_depth_bias);
				vrasterizer_state_desc->set_property<graphics::rasterizer_state::desc>("int32 depth_bias", &graphics::rasterizer_state::desc::depth_bias);
				vrasterizer_state_desc->set_property<graphics::rasterizer_state::desc>("bool front_counter_clockwise", &graphics::rasterizer_state::desc::front_counter_clockwise);
				vrasterizer_state_desc->set_property<graphics::rasterizer_state::desc>("bool depth_clip_enable", &graphics::rasterizer_state::desc::depth_clip_enable);
				vrasterizer_state_desc->set_property<graphics::rasterizer_state::desc>("bool scissor_enable", &graphics::rasterizer_state::desc::scissor_enable);
				vrasterizer_state_desc->set_property<graphics::rasterizer_state::desc>("bool multisample_enable", &graphics::rasterizer_state::desc::multisample_enable);
				vrasterizer_state_desc->set_property<graphics::rasterizer_state::desc>("bool antialiased_line_enable", &graphics::rasterizer_state::desc::antialiased_line_enable);
				vrasterizer_state_desc->set_constructor<graphics::rasterizer_state::desc>("void f()");

				auto vrasterizer_state = vm->set_class<graphics::rasterizer_state>("rasterizer_state", false);
				vrasterizer_state->set_method("uptr@ get_resource() const", &graphics::rasterizer_state::get_resource);
				vrasterizer_state->set_method("rasterizer_state_desc get_state() const", &graphics::rasterizer_state::get_state);

				auto vblend_state_desc = vm->set_pod<graphics::blend_state::desc>("blend_state_desc");
				vblend_state_desc->set_property<graphics::blend_state::desc>("bool alpha_to_coverage_enable", &graphics::blend_state::desc::alpha_to_coverage_enable);
				vblend_state_desc->set_property<graphics::blend_state::desc>("bool independent_blend_enable", &graphics::blend_state::desc::independent_blend_enable);
				vblend_state_desc->set_constructor<graphics::blend_state::desc>("void f()");
				vblend_state_desc->set_operator_ex(operators::index, (uint32_t)position::left, "render_target_blend_state&", "usize", &blend_state_desc_get_render_target);
				vblend_state_desc->set_operator_ex(operators::index, (uint32_t)position::constant, "const render_target_blend_state&", "usize", &blend_state_desc_get_render_target);

				auto vblend_state = vm->set_class<graphics::blend_state>("blend_state", false);
				vblend_state->set_method("uptr@ get_resource() const", &graphics::blend_state::get_resource);
				vblend_state->set_method("blend_state_desc get_state() const", &graphics::blend_state::get_state);

				auto vsampler_state_desc = vm->set_pod<graphics::sampler_state::desc>("sampler_state_desc");
				vsampler_state_desc->set_property<graphics::sampler_state::desc>("comparison comparison_function", &graphics::sampler_state::desc::comparison_function);
				vsampler_state_desc->set_property<graphics::sampler_state::desc>("texture_address address_u", &graphics::sampler_state::desc::address_u);
				vsampler_state_desc->set_property<graphics::sampler_state::desc>("texture_address address_v", &graphics::sampler_state::desc::address_v);
				vsampler_state_desc->set_property<graphics::sampler_state::desc>("texture_address address_w", &graphics::sampler_state::desc::address_w);
				vsampler_state_desc->set_property<graphics::sampler_state::desc>("pixel_filter filter", &graphics::sampler_state::desc::filter);
				vsampler_state_desc->set_property<graphics::sampler_state::desc>("float mip_lod_bias", &graphics::sampler_state::desc::mip_lod_bias);
				vsampler_state_desc->set_property<graphics::sampler_state::desc>("uint32 max_anisotropy", &graphics::sampler_state::desc::max_anisotropy);
				vsampler_state_desc->set_property<graphics::sampler_state::desc>("float min_lod", &graphics::sampler_state::desc::min_lod);
				vsampler_state_desc->set_property<graphics::sampler_state::desc>("float max_lod", &graphics::sampler_state::desc::max_lod);
				vsampler_state_desc->set_constructor<graphics::sampler_state::desc>("void f()");
				vsampler_state_desc->set_operator_ex(operators::index, (uint32_t)position::left, "float&", "usize", &sampler_state_desc_get_border_color);
				vsampler_state_desc->set_operator_ex(operators::index, (uint32_t)position::constant, "const float&", "usize", &sampler_state_desc_get_border_color);

				auto vsampler_state = vm->set_class<graphics::sampler_state>("sampler_state", false);
				vsampler_state->set_method("uptr@ get_resource() const", &graphics::sampler_state::get_resource);
				vsampler_state->set_method("sampler_state_desc get_state() const", &graphics::sampler_state::get_state);

				auto vinput_layout_attribute = vm->set_struct_trivial<graphics::input_layout::attribute>("input_layout_attribute");
				vinput_layout_attribute->set_property<graphics::input_layout::attribute>("string semantic_name", &graphics::input_layout::attribute::semantic_name);
				vinput_layout_attribute->set_property<graphics::input_layout::attribute>("uint32 semantic_index", &graphics::input_layout::attribute::semantic_index);
				vinput_layout_attribute->set_property<graphics::input_layout::attribute>("attribute_type format", &graphics::input_layout::attribute::format);
				vinput_layout_attribute->set_property<graphics::input_layout::attribute>("uint32 components", &graphics::input_layout::attribute::components);
				vinput_layout_attribute->set_property<graphics::input_layout::attribute>("uint32 aligned_byte_offset", &graphics::input_layout::attribute::aligned_byte_offset);
				vinput_layout_attribute->set_property<graphics::input_layout::attribute>("uint32 slot", &graphics::input_layout::attribute::slot);
				vinput_layout_attribute->set_property<graphics::input_layout::attribute>("bool per_vertex", &graphics::input_layout::attribute::per_vertex);
				vinput_layout_attribute->set_constructor<graphics::input_layout::attribute>("void f()");

				auto vshader = vm->set_class<graphics::shader>("shader", false);
				vshader->set_method("bool is_valid() const", &graphics::shader::is_valid);

				auto vinput_layout_desc = vm->set_struct<graphics::input_layout::desc>("input_layout_desc");
				vinput_layout_desc->set_property<graphics::input_layout::desc>("shader@ source", &graphics::input_layout::desc::source);
				vinput_layout_desc->set_constructor<graphics::input_layout::desc>("void f()");
				vinput_layout_desc->set_operator_copy_static(&input_layout_desc_copy);
				vinput_layout_desc->set_destructor_ex("void f()", &input_layout_desc_destructor);
				vinput_layout_desc->set_method_ex("void set_attributes(array<input_layout_attribute>@+)", &input_layout_desc_set_attributes);

				auto vinput_layout = vm->set_class<graphics::input_layout>("input_layout", false);
				vinput_layout->set_method("uptr@ get_resource() const", &graphics::input_layout::get_resource);
				vinput_layout->set_method_ex("array<input_layout_attribute>@ get_attributes() const", &input_layout_get_attributes);

				auto vshader_desc = vm->set_struct_trivial<graphics::shader::desc>("shader_desc");
				vshader_desc->set_property<graphics::shader::desc>("string filename", &graphics::shader::desc::filename);
				vshader_desc->set_property<graphics::shader::desc>("string data", &graphics::shader::desc::data);
				vshader_desc->set_property<graphics::shader::desc>("shader_type stage", &graphics::shader::desc::stage);
				vshader_desc->set_constructor<graphics::shader::desc>("void f()");
				vshader_desc->set_method_ex("void set_defines(array<input_layout_attribute>@+)", &shader_desc_set_defines);

				auto velement_buffer_desc = vm->set_struct_trivial<graphics::element_buffer::desc>("element_buffer_desc");
				velement_buffer_desc->set_property<graphics::element_buffer::desc>("uptr@ elements", &graphics::element_buffer::desc::elements);
				velement_buffer_desc->set_property<graphics::element_buffer::desc>("uint32 structure_byte_stride", &graphics::element_buffer::desc::structure_byte_stride);
				velement_buffer_desc->set_property<graphics::element_buffer::desc>("uint32 element_width", &graphics::element_buffer::desc::element_width);
				velement_buffer_desc->set_property<graphics::element_buffer::desc>("uint32 element_count", &graphics::element_buffer::desc::element_count);
				velement_buffer_desc->set_property<graphics::element_buffer::desc>("cpu_access access_flags", &graphics::element_buffer::desc::access_flags);
				velement_buffer_desc->set_property<graphics::element_buffer::desc>("resource_usage usage", &graphics::element_buffer::desc::usage);
				velement_buffer_desc->set_property<graphics::element_buffer::desc>("resource_bind bind_flags", &graphics::element_buffer::desc::bind_flags);
				velement_buffer_desc->set_property<graphics::element_buffer::desc>("resource_misc misc_flags", &graphics::element_buffer::desc::misc_flags);
				velement_buffer_desc->set_property<graphics::element_buffer::desc>("bool writable", &graphics::element_buffer::desc::writable);
				velement_buffer_desc->set_constructor<graphics::element_buffer::desc>("void f()");

				auto velement_buffer = vm->set_class<graphics::element_buffer>("element_buffer", false);
				velement_buffer->set_method("uptr@ get_resource() const", &graphics::element_buffer::get_resource);
				velement_buffer->set_method("usize get_elements() const", &graphics::element_buffer::get_elements);
				velement_buffer->set_method("usize get_stride() const", &graphics::element_buffer::get_stride);

				auto vmesh_buffer_desc = vm->set_struct_trivial<graphics::mesh_buffer::desc>("mesh_buffer_desc");
				vmesh_buffer_desc->set_property<graphics::mesh_buffer::desc>("cpu_access access_flags", &graphics::mesh_buffer::desc::access_flags);
				vmesh_buffer_desc->set_property<graphics::mesh_buffer::desc>("resource_usage usage", &graphics::mesh_buffer::desc::usage);
				vmesh_buffer_desc->set_constructor<graphics::mesh_buffer::desc>("void f()");
				vmesh_buffer_desc->set_method_ex("void set_elements(array<vertex>@+)", &mesh_buffer_desc_set_elements);
				vmesh_buffer_desc->set_method_ex("void set_indices(array<int>@+)", &mesh_buffer_desc_set_indices);

				auto vmesh_buffer = vm->set_class<graphics::mesh_buffer>("mesh_buffer", true);
				vmesh_buffer->set_property<graphics::mesh_buffer>("matrix4x4 transform", &graphics::mesh_buffer::transform);
				vmesh_buffer->set_property<graphics::mesh_buffer>("string name", &graphics::mesh_buffer::name);
				vmesh_buffer->set_method("element_buffer@+ get_vertex_buffer() const", &graphics::mesh_buffer::get_vertex_buffer);
				vmesh_buffer->set_method("element_buffer@+ get_index_buffer() const", &graphics::mesh_buffer::get_index_buffer);
				vmesh_buffer->set_enum_refs_ex<graphics::mesh_buffer>([](graphics::mesh_buffer* base, asIScriptEngine* vm) { });
				vmesh_buffer->set_release_refs_ex<graphics::mesh_buffer>([](graphics::mesh_buffer* base, asIScriptEngine*) { });

				auto vskin_mesh_buffer_desc = vm->set_struct_trivial<graphics::skin_mesh_buffer::desc>("skin_mesh_buffer_desc");
				vskin_mesh_buffer_desc->set_property<graphics::skin_mesh_buffer::desc>("cpu_access access_flags", &graphics::skin_mesh_buffer::desc::access_flags);
				vskin_mesh_buffer_desc->set_property<graphics::skin_mesh_buffer::desc>("resource_usage usage", &graphics::skin_mesh_buffer::desc::usage);
				vskin_mesh_buffer_desc->set_constructor<graphics::skin_mesh_buffer::desc>("void f()");
				vskin_mesh_buffer_desc->set_method_ex("void set_elements(array<vertex>@+)", &skin_mesh_buffer_desc_set_elements);
				vskin_mesh_buffer_desc->set_method_ex("void set_indices(array<int>@+)", &skin_mesh_buffer_desc_set_indices);

				auto vskin_mesh_buffer = vm->set_class<graphics::skin_mesh_buffer>("skin_mesh_buffer", true);
				vskin_mesh_buffer->set_property<graphics::skin_mesh_buffer>("matrix4x4 transform", &graphics::skin_mesh_buffer::transform);
				vskin_mesh_buffer->set_property<graphics::skin_mesh_buffer>("string name", &graphics::skin_mesh_buffer::name);
				vskin_mesh_buffer->set_method("element_buffer@+ get_vertex_buffer() const", &graphics::skin_mesh_buffer::get_vertex_buffer);
				vskin_mesh_buffer->set_method("element_buffer@+ get_index_buffer() const", &graphics::skin_mesh_buffer::get_index_buffer);
				vskin_mesh_buffer->set_enum_refs_ex<graphics::skin_mesh_buffer>([](graphics::skin_mesh_buffer* base, asIScriptEngine* vm) { });
				vskin_mesh_buffer->set_release_refs_ex<graphics::skin_mesh_buffer>([](graphics::skin_mesh_buffer* base, asIScriptEngine*) { });

				auto vgraphics_device = vm->set_class<graphics::graphics_device>("graphics_device", true);
				auto vinstance_buffer_desc = vm->set_struct<graphics::instance_buffer::desc>("instance_buffer_desc");
				vinstance_buffer_desc->set_property<graphics::instance_buffer::desc>("graphics_device@ device", &graphics::instance_buffer::desc::device);
				vinstance_buffer_desc->set_property<graphics::instance_buffer::desc>("uint32 element_width", &graphics::instance_buffer::desc::element_width);
				vinstance_buffer_desc->set_property<graphics::instance_buffer::desc>("uint32 element_limit", &graphics::instance_buffer::desc::element_limit);
				vinstance_buffer_desc->set_constructor<graphics::instance_buffer::desc>("void f()");
				vinstance_buffer_desc->set_operator_copy_static(&instance_buffer_desc_copy);
				vinstance_buffer_desc->set_destructor_ex("void f()", &instance_buffer_desc_destructor);

				auto vinstance_buffer = vm->set_class<graphics::instance_buffer>("instance_buffer", true);
				vinstance_buffer->set_method_ex("void set_array(array<element_vertex>@+)", &instance_buffer_set_array);
				vinstance_buffer->set_method_ex("array<element_vertex>@ get_array() const", &instance_buffer_get_array);
				vinstance_buffer->set_method("element_buffer@+ get_elements() const", &graphics::instance_buffer::get_elements);
				vinstance_buffer->set_method("graphics_device@+ get_device() const", &graphics::instance_buffer::get_device);
				vinstance_buffer->set_method("usize get_element_limit() const", &graphics::instance_buffer::get_element_limit);
				vinstance_buffer->set_enum_refs_ex<graphics::instance_buffer>([](graphics::instance_buffer* base, asIScriptEngine* vm) { });
				vinstance_buffer->set_release_refs_ex<graphics::instance_buffer>([](graphics::instance_buffer* base, asIScriptEngine*) { });

				auto vtexture_2ddesc = vm->set_pod<graphics::texture_2d::desc>("texture_2d_desc");
				vtexture_2ddesc->set_property<graphics::texture_2d::desc>("cpu_access access_flags", &graphics::texture_2d::desc::access_flags);
				vtexture_2ddesc->set_property<graphics::texture_2d::desc>("surface_format format_mode", &graphics::texture_2d::desc::format_mode);
				vtexture_2ddesc->set_property<graphics::texture_2d::desc>("resource_usage usage", &graphics::texture_2d::desc::usage);
				vtexture_2ddesc->set_property<graphics::texture_2d::desc>("resource_bind bind_flags", &graphics::texture_2d::desc::bind_flags);
				vtexture_2ddesc->set_property<graphics::texture_2d::desc>("resource_misc misc_flags", &graphics::texture_2d::desc::misc_flags);
				vtexture_2ddesc->set_property<graphics::texture_2d::desc>("uptr@ data", &graphics::texture_2d::desc::data);
				vtexture_2ddesc->set_property<graphics::texture_2d::desc>("uint32 row_pitch", &graphics::texture_2d::desc::row_pitch);
				vtexture_2ddesc->set_property<graphics::texture_2d::desc>("uint32 depth_pitch", &graphics::texture_2d::desc::depth_pitch);
				vtexture_2ddesc->set_property<graphics::texture_2d::desc>("uint32 width", &graphics::texture_2d::desc::width);
				vtexture_2ddesc->set_property<graphics::texture_2d::desc>("uint32 height", &graphics::texture_2d::desc::height);
				vtexture_2ddesc->set_property<graphics::texture_2d::desc>("int32 mip_levels", &graphics::texture_2d::desc::mip_levels);
				vtexture_2ddesc->set_property<graphics::texture_2d::desc>("bool writable", &graphics::texture_2d::desc::writable);
				vtexture_2ddesc->set_constructor<graphics::texture_2d::desc>("void f()");

				auto vtexture_2d = vm->set_class<graphics::texture_2d>("texture_2d", false);
				vtexture_2d->set_method("uptr@ get_resource() const", &graphics::texture_2d::get_resource);
				vtexture_2d->set_method("cpu_access get_access_flags() const", &graphics::texture_2d::get_access_flags);
				vtexture_2d->set_method("surface_format get_format_mode() const", &graphics::texture_2d::get_format_mode);
				vtexture_2d->set_method("resource_usage get_usage() const", &graphics::texture_2d::get_usage);
				vtexture_2d->set_method("resource_bind get_binding() const", &graphics::texture_2d::get_binding);
				vtexture_2d->set_method("uint32 get_width() const", &graphics::texture_2d::get_width);
				vtexture_2d->set_method("uint32 get_height() const", &graphics::texture_2d::get_height);
				vtexture_2d->set_method("uint32 get_mip_levels() const", &graphics::texture_2d::get_mip_levels);

				auto vtexture_3ddesc = vm->set_pod<graphics::texture_3d::desc>("texture_3d_desc");
				vtexture_3ddesc->set_property<graphics::texture_3d::desc>("cpu_access access_flags", &graphics::texture_3d::desc::access_flags);
				vtexture_3ddesc->set_property<graphics::texture_3d::desc>("surface_format format_mode", &graphics::texture_3d::desc::format_mode);
				vtexture_3ddesc->set_property<graphics::texture_3d::desc>("resource_usage usage", &graphics::texture_3d::desc::usage);
				vtexture_3ddesc->set_property<graphics::texture_3d::desc>("resource_bind bind_flags", &graphics::texture_3d::desc::bind_flags);
				vtexture_3ddesc->set_property<graphics::texture_3d::desc>("resource_misc misc_flags", &graphics::texture_3d::desc::misc_flags);
				vtexture_3ddesc->set_property<graphics::texture_3d::desc>("uint32 width", &graphics::texture_3d::desc::width);
				vtexture_3ddesc->set_property<graphics::texture_3d::desc>("uint32 height", &graphics::texture_3d::desc::height);
				vtexture_3ddesc->set_property<graphics::texture_3d::desc>("uint32 depth", &graphics::texture_3d::desc::depth);
				vtexture_3ddesc->set_property<graphics::texture_3d::desc>("int32 mip_levels", &graphics::texture_3d::desc::mip_levels);
				vtexture_3ddesc->set_property<graphics::texture_3d::desc>("bool writable", &graphics::texture_3d::desc::writable);
				vtexture_3ddesc->set_constructor<graphics::texture_3d::desc>("void f()");

				auto vtexture_3d = vm->set_class<graphics::texture_3d>("texture_3d", false);
				vtexture_3d->set_method("uptr@ get_resource() const", &graphics::texture_3d::get_resource);
				vtexture_3d->set_method("cpu_access get_access_flags() const", &graphics::texture_3d::get_access_flags);
				vtexture_3d->set_method("surface_format get_format_mode() const", &graphics::texture_3d::get_format_mode);
				vtexture_3d->set_method("resource_usage get_usage() const", &graphics::texture_3d::get_usage);
				vtexture_3d->set_method("resource_bind get_binding() const", &graphics::texture_3d::get_binding);
				vtexture_3d->set_method("uint32 get_width() const", &graphics::texture_3d::get_width);
				vtexture_3d->set_method("uint32 get_height() const", &graphics::texture_3d::get_height);
				vtexture_3d->set_method("uint32 get_depth() const", &graphics::texture_3d::get_depth);
				vtexture_3d->set_method("uint32 get_mip_levels() const", &graphics::texture_3d::get_mip_levels);

				auto vtexture_cube_desc = vm->set_pod<graphics::texture_cube::desc>("texture_cube_desc");
				vtexture_cube_desc->set_property<graphics::texture_cube::desc>("cpu_access access_flags", &graphics::texture_cube::desc::access_flags);
				vtexture_cube_desc->set_property<graphics::texture_cube::desc>("surface_format format_mode", &graphics::texture_cube::desc::format_mode);
				vtexture_cube_desc->set_property<graphics::texture_cube::desc>("resource_usage usage", &graphics::texture_cube::desc::usage);
				vtexture_cube_desc->set_property<graphics::texture_cube::desc>("resource_bind bind_flags", &graphics::texture_cube::desc::bind_flags);
				vtexture_cube_desc->set_property<graphics::texture_cube::desc>("resource_misc misc_flags", &graphics::texture_cube::desc::misc_flags);
				vtexture_cube_desc->set_property<graphics::texture_cube::desc>("uint32 width", &graphics::texture_cube::desc::width);
				vtexture_cube_desc->set_property<graphics::texture_cube::desc>("uint32 height", &graphics::texture_cube::desc::height);
				vtexture_cube_desc->set_property<graphics::texture_cube::desc>("int32 mip_levels", &graphics::texture_cube::desc::mip_levels);
				vtexture_cube_desc->set_property<graphics::texture_cube::desc>("bool writable", &graphics::texture_cube::desc::writable);
				vtexture_cube_desc->set_constructor<graphics::texture_cube::desc>("void f()");

				auto vtexture_cube = vm->set_class<graphics::texture_cube>("texture_cube", false);
				vtexture_cube->set_method("uptr@ get_resource() const", &graphics::texture_cube::get_resource);
				vtexture_cube->set_method("cpu_access get_access_flags() const", &graphics::texture_cube::get_access_flags);
				vtexture_cube->set_method("surface_format get_format_mode() const", &graphics::texture_cube::get_format_mode);
				vtexture_cube->set_method("resource_usage get_usage() const", &graphics::texture_cube::get_usage);
				vtexture_cube->set_method("resource_bind get_binding() const", &graphics::texture_cube::get_binding);
				vtexture_cube->set_method("uint32 get_width() const", &graphics::texture_cube::get_width);
				vtexture_cube->set_method("uint32 get_height() const", &graphics::texture_cube::get_height);
				vtexture_cube->set_method("uint32 get_mip_levels() const", &graphics::texture_cube::get_mip_levels);

				auto vdepth_target_2ddesc = vm->set_pod<graphics::depth_target_2d::desc>("depth_target_2d_desc");
				vdepth_target_2ddesc->set_property<graphics::depth_target_2d::desc>("cpu_access access_flags", &graphics::depth_target_2d::desc::access_flags);
				vdepth_target_2ddesc->set_property<graphics::depth_target_2d::desc>("resource_usage usage", &graphics::depth_target_2d::desc::usage);
				vdepth_target_2ddesc->set_property<graphics::depth_target_2d::desc>("surface_format format_mode", &graphics::depth_target_2d::desc::format_mode);
				vdepth_target_2ddesc->set_property<graphics::depth_target_2d::desc>("uint32 width", &graphics::depth_target_2d::desc::width);
				vdepth_target_2ddesc->set_property<graphics::depth_target_2d::desc>("uint32 height", &graphics::depth_target_2d::desc::height);
				vdepth_target_2ddesc->set_constructor<graphics::depth_target_2d::desc>("void f()");

				auto vdepth_target_2d = vm->set_class<graphics::depth_target_2d>("depth_target_2d", false);
				vdepth_target_2d->set_method("uptr@ get_resource() const", &graphics::depth_target_2d::get_resource);
				vdepth_target_2d->set_method("uint32 get_width() const", &graphics::depth_target_2d::get_width);
				vdepth_target_2d->set_method("uint32 get_height() const", &graphics::depth_target_2d::get_height);
				vdepth_target_2d->set_method("texture_2d@+ get_target() const", &graphics::depth_target_2d::get_target);
				vdepth_target_2d->set_method("const viewport& get_viewport() const", &graphics::depth_target_2d::get_viewport);

				auto vdepth_target_cube_desc = vm->set_pod<graphics::depth_target_cube::desc>("depth_target_cube_desc");
				vdepth_target_cube_desc->set_property<graphics::depth_target_cube::desc>("cpu_access access_flags", &graphics::depth_target_cube::desc::access_flags);
				vdepth_target_cube_desc->set_property<graphics::depth_target_cube::desc>("resource_usage usage", &graphics::depth_target_cube::desc::usage);
				vdepth_target_cube_desc->set_property<graphics::depth_target_cube::desc>("surface_format format_mode", &graphics::depth_target_cube::desc::format_mode);
				vdepth_target_cube_desc->set_property<graphics::depth_target_cube::desc>("uint32 size", &graphics::depth_target_cube::desc::size);
				vdepth_target_cube_desc->set_constructor<graphics::depth_target_cube::desc>("void f()");

				auto vdepth_target_cube = vm->set_class<graphics::depth_target_cube>("depth_target_cube", false);
				vdepth_target_cube->set_method("uptr@ get_resource() const", &graphics::depth_target_cube::get_resource);
				vdepth_target_cube->set_method("uint32 get_width() const", &graphics::depth_target_cube::get_width);
				vdepth_target_cube->set_method("uint32 get_height() const", &graphics::depth_target_cube::get_height);
				vdepth_target_cube->set_method("texture_2d@+ get_target() const", &graphics::depth_target_cube::get_target);
				vdepth_target_cube->set_method("const viewport& get_viewport() const", &graphics::depth_target_cube::get_viewport);

				auto vrender_target = vm->set_class<graphics::render_target>("render_target", false);
				vrender_target->set_method("uptr@ get_target_buffer() const", &graphics::render_target::get_target_buffer);
				vrender_target->set_method("uptr@ get_depth_buffer() const", &graphics::render_target::get_depth_buffer);
				vrender_target->set_method("uint32 get_width() const", &graphics::render_target::get_width);
				vrender_target->set_method("uint32 get_height() const", &graphics::render_target::get_height);
				vrender_target->set_method("uint32 get_target_count() const", &graphics::render_target::get_target_count);
				vrender_target->set_method("texture_2d@+ get_target_2d(uint32) const", &graphics::render_target::get_target_2d);
				vrender_target->set_method("texture_cube@+ get_target_cube(uint32) const", &graphics::render_target::get_target_cube);
				vrender_target->set_method("texture_2d@+ get_depth_stencil() const", &graphics::render_target::get_depth_stencil);
				vrender_target->set_method("const viewport& get_viewport() const", &graphics::render_target::get_viewport);

				auto vrender_target_2ddesc = vm->set_pod<graphics::render_target_2d::desc>("render_target_2d_desc");
				vrender_target_2ddesc->set_property<graphics::render_target_2d::desc>("cpu_access access_flags", &graphics::render_target_2d::desc::access_flags);
				vrender_target_2ddesc->set_property<graphics::render_target_2d::desc>("surface_format format_mode", &graphics::render_target_2d::desc::format_mode);
				vrender_target_2ddesc->set_property<graphics::render_target_2d::desc>("resource_usage usage", &graphics::render_target_2d::desc::usage);
				vrender_target_2ddesc->set_property<graphics::render_target_2d::desc>("resource_bind bind_flags", &graphics::render_target_2d::desc::bind_flags);
				vrender_target_2ddesc->set_property<graphics::render_target_2d::desc>("resource_misc misc_flags", &graphics::render_target_2d::desc::misc_flags);
				vrender_target_2ddesc->set_property<graphics::render_target_2d::desc>("uptr@ render_surface", &graphics::render_target_2d::desc::render_surface);
				vrender_target_2ddesc->set_property<graphics::render_target_2d::desc>("uint32 width", &graphics::render_target_2d::desc::width);
				vrender_target_2ddesc->set_property<graphics::render_target_2d::desc>("uint32 height", &graphics::render_target_2d::desc::height);
				vrender_target_2ddesc->set_property<graphics::render_target_2d::desc>("uint32 mip_levels", &graphics::render_target_2d::desc::mip_levels);
				vrender_target_2ddesc->set_property<graphics::render_target_2d::desc>("bool depth_stencil", &graphics::render_target_2d::desc::depth_stencil);
				vrender_target_2ddesc->set_constructor<graphics::render_target_2d::desc>("void f()");

				auto vrender_target_2d = vm->set_class<graphics::render_target_2d>("render_target_2d", false);
				vrender_target_2d->set_method("uptr@ get_target_buffer() const", &graphics::render_target_2d::get_target_buffer);
				vrender_target_2d->set_method("uptr@ get_depth_buffer() const", &graphics::render_target_2d::get_depth_buffer);
				vrender_target_2d->set_method("uint32 get_width() const", &graphics::render_target_2d::get_width);
				vrender_target_2d->set_method("uint32 get_height() const", &graphics::render_target_2d::get_height);
				vrender_target_2d->set_method("uint32 get_target_count() const", &graphics::render_target_2d::get_target_count);
				vrender_target_2d->set_method("texture_2d@+ get_target_2d(uint32) const", &graphics::render_target_2d::get_target_2d);
				vrender_target_2d->set_method("texture_cube@+ get_target_cube(uint32) const", &graphics::render_target_2d::get_target_cube);
				vrender_target_2d->set_method("texture_2d@+ get_depth_stencil() const", &graphics::render_target_2d::get_depth_stencil);
				vrender_target_2d->set_method("const viewport& get_viewport() const", &graphics::render_target_2d::get_viewport);
				vrender_target_2d->set_method("texture_2d@+ get_target() const", &graphics::render_target_2d::get_target);

				auto vmulti_render_target_2ddesc = vm->set_pod<graphics::multi_render_target_2d::desc>("multi_render_target_2d_desc");
				vmulti_render_target_2ddesc->set_property<graphics::multi_render_target_2d::desc>("cpu_access access_flags", &graphics::multi_render_target_2d::desc::access_flags);
				vmulti_render_target_2ddesc->set_property<graphics::multi_render_target_2d::desc>("surface_target target", &graphics::multi_render_target_2d::desc::target);
				vmulti_render_target_2ddesc->set_property<graphics::multi_render_target_2d::desc>("resource_usage usage", &graphics::multi_render_target_2d::desc::usage);
				vmulti_render_target_2ddesc->set_property<graphics::multi_render_target_2d::desc>("resource_bind bind_flags", &graphics::multi_render_target_2d::desc::bind_flags);
				vmulti_render_target_2ddesc->set_property<graphics::multi_render_target_2d::desc>("resource_misc misc_flags", &graphics::multi_render_target_2d::desc::misc_flags);
				vmulti_render_target_2ddesc->set_property<graphics::multi_render_target_2d::desc>("uint32 width", &graphics::multi_render_target_2d::desc::width);
				vmulti_render_target_2ddesc->set_property<graphics::multi_render_target_2d::desc>("uint32 height", &graphics::multi_render_target_2d::desc::height);
				vmulti_render_target_2ddesc->set_property<graphics::multi_render_target_2d::desc>("uint32 mip_levels", &graphics::multi_render_target_2d::desc::mip_levels);
				vmulti_render_target_2ddesc->set_property<graphics::multi_render_target_2d::desc>("bool depth_stencil", &graphics::multi_render_target_2d::desc::depth_stencil);
				vmulti_render_target_2ddesc->set_constructor<graphics::multi_render_target_2d::desc>("void f()");
				vmulti_render_target_2ddesc->set_method_ex("void set_format_mode(usize, surface_format)", &multi_render_target_2ddesc_set_format_mode);

				auto vmulti_render_target_2d = vm->set_class<graphics::multi_render_target_2d>("multi_render_target_2d", false);
				vmulti_render_target_2d->set_method("uptr@ get_target_buffer() const", &graphics::multi_render_target_2d::get_target_buffer);
				vmulti_render_target_2d->set_method("uptr@ get_depth_buffer() const", &graphics::multi_render_target_2d::get_depth_buffer);
				vmulti_render_target_2d->set_method("uint32 get_width() const", &graphics::multi_render_target_2d::get_width);
				vmulti_render_target_2d->set_method("uint32 get_height() const", &graphics::multi_render_target_2d::get_height);
				vmulti_render_target_2d->set_method("uint32 get_target_count() const", &graphics::multi_render_target_2d::get_target_count);
				vmulti_render_target_2d->set_method("texture_2d@+ get_target_2d(uint32) const", &graphics::multi_render_target_2d::get_target_2d);
				vmulti_render_target_2d->set_method("texture_cube@+ get_target_cube(uint32) const", &graphics::multi_render_target_2d::get_target_cube);
				vmulti_render_target_2d->set_method("texture_2d@+ get_depth_stencil() const", &graphics::multi_render_target_2d::get_depth_stencil);
				vmulti_render_target_2d->set_method("const viewport& get_viewport() const", &graphics::multi_render_target_2d::get_viewport);
				vmulti_render_target_2d->set_method("texture_2d@+ get_target(uint32) const", &graphics::multi_render_target_2d::get_target);

				auto vrender_target_cube_desc = vm->set_pod<graphics::render_target_cube::desc>("render_target_cube_desc");
				vrender_target_cube_desc->set_property<graphics::render_target_cube::desc>("cpu_access access_flags", &graphics::render_target_cube::desc::access_flags);
				vrender_target_cube_desc->set_property<graphics::render_target_cube::desc>("surface_format format_mode", &graphics::render_target_cube::desc::format_mode);
				vrender_target_cube_desc->set_property<graphics::render_target_cube::desc>("resource_usage usage", &graphics::render_target_cube::desc::usage);
				vrender_target_cube_desc->set_property<graphics::render_target_cube::desc>("resource_bind bind_flags", &graphics::render_target_cube::desc::bind_flags);
				vrender_target_cube_desc->set_property<graphics::render_target_cube::desc>("resource_misc misc_flags", &graphics::render_target_cube::desc::misc_flags);
				vrender_target_cube_desc->set_property<graphics::render_target_cube::desc>("uint32 size", &graphics::render_target_cube::desc::size);
				vrender_target_cube_desc->set_property<graphics::render_target_cube::desc>("uint32 mip_levels", &graphics::render_target_cube::desc::mip_levels);
				vrender_target_cube_desc->set_property<graphics::render_target_cube::desc>("bool depth_stencil", &graphics::render_target_cube::desc::depth_stencil);
				vrender_target_cube_desc->set_constructor<graphics::render_target_cube::desc>("void f()");

				auto vrender_target_cube = vm->set_class<graphics::render_target_cube>("render_target_cube", false);
				vrender_target_cube->set_method("uptr@ get_target_buffer() const", &graphics::render_target_cube::get_target_buffer);
				vrender_target_cube->set_method("uptr@ get_depth_buffer() const", &graphics::render_target_cube::get_depth_buffer);
				vrender_target_cube->set_method("uint32 get_width() const", &graphics::render_target_cube::get_width);
				vrender_target_cube->set_method("uint32 get_height() const", &graphics::render_target_cube::get_height);
				vrender_target_cube->set_method("uint32 get_target_count() const", &graphics::render_target_cube::get_target_count);
				vrender_target_cube->set_method("texture_2d@+ get_target_2d(uint32) const", &graphics::render_target_cube::get_target_2d);
				vrender_target_cube->set_method("texture_cube@+ get_target_cube(uint32) const", &graphics::render_target_cube::get_target_cube);
				vrender_target_cube->set_method("texture_2d@+ get_depth_stencil() const", &graphics::render_target_cube::get_depth_stencil);
				vrender_target_cube->set_method("const viewport& get_viewport() const", &graphics::render_target_cube::get_viewport);
				vrender_target_cube->set_method("texture_cube@+ get_target() const", &graphics::render_target_cube::get_target);

				auto vmulti_render_target_cube_desc = vm->set_pod<graphics::multi_render_target_cube::desc>("multi_render_target_cube_desc");
				vmulti_render_target_cube_desc->set_property<graphics::multi_render_target_cube::desc>("cpu_access access_flags", &graphics::multi_render_target_cube::desc::access_flags);
				vmulti_render_target_cube_desc->set_property<graphics::multi_render_target_cube::desc>("surface_target target", &graphics::multi_render_target_cube::desc::target);
				vmulti_render_target_cube_desc->set_property<graphics::multi_render_target_cube::desc>("resource_usage usage", &graphics::multi_render_target_cube::desc::usage);
				vmulti_render_target_cube_desc->set_property<graphics::multi_render_target_cube::desc>("resource_bind bind_flags", &graphics::multi_render_target_cube::desc::bind_flags);
				vmulti_render_target_cube_desc->set_property<graphics::multi_render_target_cube::desc>("resource_misc misc_flags", &graphics::multi_render_target_cube::desc::misc_flags);
				vmulti_render_target_cube_desc->set_property<graphics::multi_render_target_cube::desc>("uint32 size", &graphics::multi_render_target_cube::desc::size);
				vmulti_render_target_cube_desc->set_property<graphics::multi_render_target_cube::desc>("uint32 mip_levels", &graphics::multi_render_target_cube::desc::mip_levels);
				vmulti_render_target_cube_desc->set_property<graphics::multi_render_target_cube::desc>("bool depth_stencil", &graphics::multi_render_target_cube::desc::depth_stencil);
				vmulti_render_target_cube_desc->set_constructor<graphics::multi_render_target_cube::desc>("void f()");
				vmulti_render_target_cube_desc->set_method_ex("void set_format_mode(usize, surface_format)", &multi_render_target_cube_desc_set_format_mode);

				auto vmulti_render_target_cube = vm->set_class<graphics::multi_render_target_cube>("multi_render_target_cube", false);
				vmulti_render_target_cube->set_method("uptr@ get_target_buffer() const", &graphics::multi_render_target_cube::get_target_buffer);
				vmulti_render_target_cube->set_method("uptr@ get_depth_buffer() const", &graphics::multi_render_target_cube::get_depth_buffer);
				vmulti_render_target_cube->set_method("uint32 get_width() const", &graphics::multi_render_target_cube::get_width);
				vmulti_render_target_cube->set_method("uint32 get_height() const", &graphics::multi_render_target_cube::get_height);
				vmulti_render_target_cube->set_method("uint32 get_target_count() const", &graphics::multi_render_target_cube::get_target_count);
				vmulti_render_target_cube->set_method("texture_2d@+ get_target_2d(uint32) const", &graphics::multi_render_target_cube::get_target_2d);
				vmulti_render_target_cube->set_method("texture_cube@+ get_target_cube(uint32) const", &graphics::multi_render_target_cube::get_target_cube);
				vmulti_render_target_cube->set_method("texture_2d@+ get_depth_stencil() const", &graphics::multi_render_target_cube::get_depth_stencil);
				vmulti_render_target_cube->set_method("const viewport& get_viewport() const", &graphics::multi_render_target_cube::get_viewport);
				vmulti_render_target_cube->set_method("texture_cube@+ get_target(uint32) const", &graphics::multi_render_target_cube::get_target);

				auto vcubemap_desc = vm->set_struct<graphics::cubemap::desc>("cubemap_desc");
				vcubemap_desc->set_property<graphics::cubemap::desc>("render_target@ source", &graphics::cubemap::desc::source);
				vcubemap_desc->set_property<graphics::cubemap::desc>("uint32 target", &graphics::cubemap::desc::target);
				vcubemap_desc->set_property<graphics::cubemap::desc>("uint32 size", &graphics::cubemap::desc::size);
				vcubemap_desc->set_property<graphics::cubemap::desc>("uint32 mip_levels", &graphics::cubemap::desc::mip_levels);
				vcubemap_desc->set_constructor<graphics::cubemap::desc>("void f()");
				vcubemap_desc->set_operator_copy_static(&cubemap_desc_copy);
				vcubemap_desc->set_destructor_ex("void f()", &cubemap_desc_destructor);

				auto vcubemap = vm->set_class<graphics::cubemap>("cubemap", false);
				vcubemap->set_method("bool is_valid() const", &graphics::cubemap::is_valid);

				auto vquery_desc = vm->set_pod<graphics::query::desc>("visibility_query_desc");
				vquery_desc->set_property<graphics::query::desc>("bool predicate", &graphics::query::desc::predicate);
				vquery_desc->set_property<graphics::query::desc>("bool auto_pass", &graphics::query::desc::auto_pass);
				vquery_desc->set_constructor<graphics::query::desc>("void f()");

				auto vquery = vm->set_class<graphics::query>("visibility_query", false);
				vquery->set_method("uptr@ get_resource() const", &graphics::query::get_resource);

				auto vgraphics_device_desc = vm->set_struct<graphics::graphics_device::desc>("graphics_device_desc");
				vgraphics_device_desc->set_property<graphics::graphics_device::desc>("render_backend backend", &graphics::graphics_device::desc::backend);
				vgraphics_device_desc->set_property<graphics::graphics_device::desc>("shader_model shader_mode", &graphics::graphics_device::desc::shader_mode);
				vgraphics_device_desc->set_property<graphics::graphics_device::desc>("surface_format buffer_format", &graphics::graphics_device::desc::buffer_format);
				vgraphics_device_desc->set_property<graphics::graphics_device::desc>("vsync vsync_mode", &graphics::graphics_device::desc::vsync_mode);
				vgraphics_device_desc->set_property<graphics::graphics_device::desc>("string cache_directory", &graphics::graphics_device::desc::cache_directory);
				vgraphics_device_desc->set_property<graphics::graphics_device::desc>("int32 is_windowed", &graphics::graphics_device::desc::is_windowed);
				vgraphics_device_desc->set_property<graphics::graphics_device::desc>("bool shader_cache", &graphics::graphics_device::desc::shader_cache);
				vgraphics_device_desc->set_property<graphics::graphics_device::desc>("bool debug", &graphics::graphics_device::desc::debug);
				vgraphics_device_desc->set_property<graphics::graphics_device::desc>("bool blit_rendering", &graphics::graphics_device::desc::blit_rendering);
				vgraphics_device_desc->set_property<graphics::graphics_device::desc>("uint32 presentation_flags", &graphics::graphics_device::desc::presentation_flags);
				vgraphics_device_desc->set_property<graphics::graphics_device::desc>("uint32 compilation_flags", &graphics::graphics_device::desc::compilation_flags);
				vgraphics_device_desc->set_property<graphics::graphics_device::desc>("uint32 creation_flags", &graphics::graphics_device::desc::creation_flags);
				vgraphics_device_desc->set_property<graphics::graphics_device::desc>("uint32 buffer_width", &graphics::graphics_device::desc::buffer_width);
				vgraphics_device_desc->set_property<graphics::graphics_device::desc>("uint32 buffer_height", &graphics::graphics_device::desc::buffer_height);
				vgraphics_device_desc->set_property<graphics::graphics_device::desc>("activity@ window", &graphics::graphics_device::desc::window);
				vgraphics_device_desc->set_constructor<graphics::graphics_device::desc>("void f()");
				vgraphics_device_desc->set_operator_copy_static(&graphics_device_desc_copy);
				vgraphics_device_desc->set_destructor_ex("void f()", &graphics_device_desc_destructor);

				vgraphics_device->set_method("void set_as_current_device()", &graphics::graphics_device::set_as_current_device);
				vgraphics_device->set_method("void set_shader_model(shader_model)", &graphics::graphics_device::set_shader_model);
				vgraphics_device->set_method("void set_blend_state(blend_state@+)", &graphics::graphics_device::set_blend_state);
				vgraphics_device->set_method("void set_rasterizer_state(rasterizer_state@+)", &graphics::graphics_device::set_rasterizer_state);
				vgraphics_device->set_method("void set_depth_stencil_state(depth_stencil_state@+)", &graphics::graphics_device::set_depth_stencil_state);
				vgraphics_device->set_method("void set_input_layout(input_layout@+)", &graphics::graphics_device::set_input_layout);
				vgraphics_device->set_method_ex("bool set_shader(shader@+, uint32)", &VI_EXPECTIFY_VOID(graphics::graphics_device::set_shader));
				vgraphics_device->set_method("void set_sampler_state(sampler_state@+, uint32, uint32, uint32)", &graphics::graphics_device::set_sampler_state);
				vgraphics_device->set_method<graphics::graphics_device, void, graphics::shader*, uint32_t, uint32_t>("void set_buffer(shader@+, uint32, uint32)", &graphics::graphics_device::set_buffer);
				vgraphics_device->set_method<graphics::graphics_device, void, graphics::instance_buffer*, uint32_t, uint32_t>("void set_buffer(instance_buffer@+, uint32, uint32)", &graphics::graphics_device::set_buffer);
				vgraphics_device->set_method("void set_constant_buffer(element_buffer@+, uint32, uint32)", &graphics::graphics_device::set_constant_buffer);
				vgraphics_device->set_method("void set_structure_buffer(element_buffer@+, uint32, uint32)", &graphics::graphics_device::set_structure_buffer);
				vgraphics_device->set_method("void set_texture_2d(texture_2d@+, uint32, uint32)", &graphics::graphics_device::set_texture_2d);
				vgraphics_device->set_method("void set_texture_3d(texture_3d@+, uint32, uint32)", &graphics::graphics_device::set_texture_3d);
				vgraphics_device->set_method("void set_texture_cube(texture_cube@+, uint32, uint32)", &graphics::graphics_device::set_texture_cube);
				vgraphics_device->set_method("void set_index_buffer(element_buffer@+, surface_format)", &graphics::graphics_device::set_index_buffer);
				vgraphics_device->set_method_ex("void set_vertex_buffers(array<element_buffer@>@+, bool = false)", &graphics_device_set_vertex_buffers);
				vgraphics_device->set_method_ex("void set_writeable(array<element_buffer@>@+, uint32, uint32, bool)", &graphics_device_set_writeable1);
				vgraphics_device->set_method_ex("void set_writeable(array<texture_2d@>@+, uint32, uint32, bool)", &graphics_device_set_writeable2);
				vgraphics_device->set_method_ex("void set_writeable(array<texture_3d@>@+, uint32, uint32, bool)", &graphics_device_set_writeable3);
				vgraphics_device->set_method_ex("void set_writeable(array<texture_cube@>@+, uint32, uint32, bool)", &graphics_device_set_writeable4);
				vgraphics_device->set_method<graphics::graphics_device, void, float, float, float>("void set_target(float, float, float)", &graphics::graphics_device::set_target);
				vgraphics_device->set_method<graphics::graphics_device, void>("void set_target()", &graphics::graphics_device::set_target);
				vgraphics_device->set_method<graphics::graphics_device, void, graphics::depth_target_2d*>("void set_target(depth_target_2d@+)", &graphics::graphics_device::set_target);
				vgraphics_device->set_method<graphics::graphics_device, void, graphics::depth_target_cube*>("void set_target(depth_target_cube@+)", &graphics::graphics_device::set_target);
				vgraphics_device->set_method<graphics::graphics_device, void, graphics::render_target*, uint32_t, float, float, float>("void set_target(render_target@+, uint32, float, float, float)", &graphics::graphics_device::set_target);
				vgraphics_device->set_method<graphics::graphics_device, void, graphics::render_target*, uint32_t>("void set_target(render_target@+, uint32)", &graphics::graphics_device::set_target);
				vgraphics_device->set_method<graphics::graphics_device, void, graphics::render_target*, float, float, float>("void set_target(render_target@+, float, float, float)", &graphics::graphics_device::set_target);
				vgraphics_device->set_method<graphics::graphics_device, void, graphics::render_target*>("void set_target(render_target@+)", &graphics::graphics_device::set_target);
				vgraphics_device->set_method_ex("void set_target_map(render_target@+, array<bool>@+)", &graphics_device_set_target_map);
				vgraphics_device->set_method("void set_target_rect(uint32, uint32)", &graphics::graphics_device::set_target_rect);
				vgraphics_device->set_method_ex("void set_viewports(array<viewport>@+)", &graphics_device_set_viewports);
				vgraphics_device->set_method_ex("void set_scissor_rects(array<rectangle>@+)", &graphics_device_set_scissor_rects);
				vgraphics_device->set_method("void set_primitive_topology(primitive_topology)", &graphics::graphics_device::set_primitive_topology);
				vgraphics_device->set_method("void flush_texture(uint32, uint32, uint32)", &graphics::graphics_device::flush_texture);
				vgraphics_device->set_method("void flush_state()", &graphics::graphics_device::flush_state);
				vgraphics_device->set_method_ex("bool map(element_buffer@+, resource_map, mapped_subresource &out)", &graphics_device_map1);
				vgraphics_device->set_method_ex("bool map(texture_2d@+, resource_map, mapped_subresource &out)", &graphics_device_map2);
				vgraphics_device->set_method_ex("bool map(texture_3d@+, resource_map, mapped_subresource &out)", &graphics_device_map3);
				vgraphics_device->set_method_ex("bool map(texture_cube@+, resource_map, mapped_subresource &out)", &graphics_device_map4);
				vgraphics_device->set_method_ex("bool unmap(texture_2d@+, mapped_subresource &in)", &graphics_device_unmap1);
				vgraphics_device->set_method_ex("bool unmap(texture_3d@+, mapped_subresource &in)", &graphics_device_unmap2);
				vgraphics_device->set_method_ex("bool unmap(texture_cube@+, mapped_subresource &in)", &graphics_device_unmap3);
				vgraphics_device->set_method_ex("bool unmap(element_buffer@+, mapped_subresource &in)", &graphics_device_unmap4);
				vgraphics_device->set_method_ex("bool update_constant_buffer(element_buffer@+, uptr@, usize)", &graphics_device_update_constant_buffer);
				vgraphics_device->set_method_ex("bool update_buffer(element_buffer@+, uptr@, usize)", &graphics_device_update_buffer1);
				vgraphics_device->set_method_ex("bool update_buffer(shader@+, uptr@)", &graphics_device_update_buffer2);
				vgraphics_device->set_method_ex("bool update_buffer(mesh_buffer@+, uptr@)", &graphics_device_update_buffer3);
				vgraphics_device->set_method_ex("bool update_buffer(skin_mesh_buffer@+, uptr@)", &graphics_device_update_buffer4);
				vgraphics_device->set_method_ex("bool update_buffer(instance_buffer@+)", &graphics_device_update_buffer5);
				vgraphics_device->set_method_ex("bool update_buffer_size(shader@+, usize)", &graphics_device_update_buffer_size1);
				vgraphics_device->set_method_ex("bool update_buffer_size(instance_buffer@+, usize)", &graphics_device_update_buffer_size2);
				vgraphics_device->set_method("void clear_buffer(instance_buffer@+)", &graphics::graphics_device::clear_buffer);
				vgraphics_device->set_method<graphics::graphics_device, void, graphics::texture_2d*>("void clear_writable(texture_2d@+)", &graphics::graphics_device::clear_writable);
				vgraphics_device->set_method<graphics::graphics_device, void, graphics::texture_2d*, float, float, float>("void clear_writable(texture_2d@+, float, float, float)", &graphics::graphics_device::clear_writable);
				vgraphics_device->set_method<graphics::graphics_device, void, graphics::texture_3d*>("void clear_writable(texture_3d@+)", &graphics::graphics_device::clear_writable);
				vgraphics_device->set_method<graphics::graphics_device, void, graphics::texture_3d*, float, float, float>("void clear_writable(texture_3d@+, float, float, float)", &graphics::graphics_device::clear_writable);
				vgraphics_device->set_method<graphics::graphics_device, void, graphics::texture_cube*>("void clear_writable(texture_cube@+)", &graphics::graphics_device::clear_writable);
				vgraphics_device->set_method<graphics::graphics_device, void, graphics::texture_cube*, float, float, float>("void clear_writable(texture_cube@+, float, float, float)", &graphics::graphics_device::clear_writable);
				vgraphics_device->set_method<graphics::graphics_device, void, float, float, float>("void clear(float, float, float)", &graphics::graphics_device::clear);
				vgraphics_device->set_method<graphics::graphics_device, void, graphics::render_target*, uint32_t, float, float, float>("void clear(render_target@+, uint32, float, float, float)", &graphics::graphics_device::clear);
				vgraphics_device->set_method<graphics::graphics_device, void>("void clear_depth()", &graphics::graphics_device::clear_depth);
				vgraphics_device->set_method<graphics::graphics_device, void, graphics::depth_target_2d*>("void clear_depth(depth_target_2d@+)", &graphics::graphics_device::clear_depth);
				vgraphics_device->set_method<graphics::graphics_device, void, graphics::depth_target_cube*>("void clear_depth(depth_target_cube@+)", &graphics::graphics_device::clear_depth);
				vgraphics_device->set_method<graphics::graphics_device, void, graphics::render_target*>("void clear_depth(render_target@+)", &graphics::graphics_device::clear_depth);
				vgraphics_device->set_method<graphics::graphics_device, void, uint32_t, uint32_t, uint32_t>("void draw_indexed(uint32, uint32, uint32)", &graphics::graphics_device::draw_indexed);
				vgraphics_device->set_method<graphics::graphics_device, void, graphics::mesh_buffer*>("void draw_indexed(mesh_buffer@+)", &graphics::graphics_device::draw_indexed);
				vgraphics_device->set_method<graphics::graphics_device, void, graphics::skin_mesh_buffer*>("void draw_indexed(skin_mesh_buffer@+)", &graphics::graphics_device::draw_indexed);
				vgraphics_device->set_method<graphics::graphics_device, void, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t>("void draw_indexed_instanced(uint32, uint32, uint32, uint32, uint32)", &graphics::graphics_device::draw_indexed_instanced);
				vgraphics_device->set_method<graphics::graphics_device, void, graphics::element_buffer*, graphics::mesh_buffer*, uint32_t>("void draw_indexed_instanced(element_buffer@+, mesh_buffer@+, uint32)", &graphics::graphics_device::draw_indexed_instanced);
				vgraphics_device->set_method<graphics::graphics_device, void, graphics::element_buffer*, graphics::skin_mesh_buffer*, uint32_t>("void draw_indexed_instanced(element_buffer@+, skin_mesh_buffer@+, uint32)", &graphics::graphics_device::draw_indexed_instanced);
				vgraphics_device->set_method("void draw(uint32, uint32)", &graphics::graphics_device::draw);
				vgraphics_device->set_method("void draw_instanced(uint32, uint32, uint32, uint32)", &graphics::graphics_device::draw_instanced);
				vgraphics_device->set_method("void dispatch(uint32, uint32, uint32)", &graphics::graphics_device::dispatch);
				vgraphics_device->set_method_ex("texture_2d@ copy_texture_2d(texture_2d@+)", &graphics_device_copy_texture_2d1);
				vgraphics_device->set_method_ex("texture_2d@ copy_texture_2d(render_target@+, uint32)", &graphics_device_copy_texture_2d2);
				vgraphics_device->set_method_ex("texture_2d@ copy_texture_2d(render_target_cube@+, cube_face)", &graphics_device_copy_texture_2d3);
				vgraphics_device->set_method_ex("texture_2d@ copy_texture_2d(multi_render_target_cube@+, uint32, cube_face)", &graphics_device_copy_texture_2d4);
				vgraphics_device->set_method_ex("texture_cube@ copy_texture_cube(render_target_cube@+)", &graphics_device_copy_texture_cube1);
				vgraphics_device->set_method_ex("texture_cube@ copy_texture_cube(multi_render_target_cube@+, uint32)", &graphics_device_copy_texture_cube2);
				vgraphics_device->set_method_ex("bool copy_target(render_target@+, uint32, render_target@+, uint32)", &VI_EXPECTIFY_VOID(graphics::graphics_device::copy_target));
				vgraphics_device->set_method_ex("texture_2d@ copy_back_buffer()", &graphics_device_copy_back_buffer);
				vgraphics_device->set_method_ex("bool cubemap_push(cubemap@+, texture_cube@+)", &VI_EXPECTIFY_VOID(graphics::graphics_device::cubemap_push));
				vgraphics_device->set_method_ex("bool cubemap_face(cubemap@+, cube_face)", &VI_EXPECTIFY_VOID(graphics::graphics_device::cubemap_face));
				vgraphics_device->set_method_ex("bool cubemap_pop(cubemap@+)", &VI_EXPECTIFY_VOID(graphics::graphics_device::cubemap_pop));
				vgraphics_device->set_method_ex("array<viewport>@ get_viewports()", &graphics_device_get_viewports);
				vgraphics_device->set_method_ex("array<rectangle>@ get_scissor_rects()", &graphics_device_get_scissor_rects);
				vgraphics_device->set_method_ex("bool rescale_buffers(uint32, uint32)", &VI_EXPECTIFY_VOID(graphics::graphics_device::rescale_buffers));
				vgraphics_device->set_method_ex("bool resize_buffers(uint32, uint32)", &VI_EXPECTIFY_VOID(graphics::graphics_device::resize_buffers));
				vgraphics_device->set_method_ex("bool generate_texture(texture_2d@+)", &graphics_device_generate_texture1);
				vgraphics_device->set_method_ex("bool generate_texture(texture_3d@+)", &graphics_device_generate_texture2);
				vgraphics_device->set_method_ex("bool generate_texture(texture_cube@+)", &graphics_device_generate_texture3);
				vgraphics_device->set_method_ex("bool get_query_data(visibility_query@+, usize &out, bool = true)", &graphics_device_get_query_data1);
				vgraphics_device->set_method_ex("bool get_query_data(visibility_query@+, bool &out, bool = true)", &graphics_device_get_query_data2);
				vgraphics_device->set_method("void get_shader_slot(shader@+, const string_view&in)", &graphics::graphics_device::get_shader_slot);
				vgraphics_device->set_method("void get_shader_sampler_slot(shader@+, const string_view&in, const string_view&in)", &graphics::graphics_device::get_shader_sampler_slot);
				vgraphics_device->set_method("void query_begin(visibility_query@+)", &graphics::graphics_device::query_begin);
				vgraphics_device->set_method("void query_end(visibility_query@+)", &graphics::graphics_device::query_end);
				vgraphics_device->set_method<graphics::graphics_device, void, graphics::texture_2d*>("void generate_mips(texture_2d@+)", &graphics::graphics_device::generate_mips);
				vgraphics_device->set_method<graphics::graphics_device, void, graphics::texture_3d*>("void generate_mips(texture_3d@+)", &graphics::graphics_device::generate_mips);
				vgraphics_device->set_method<graphics::graphics_device, void, graphics::texture_cube*>("void generate_mips(texture_cube@+)", &graphics::graphics_device::generate_mips);
				vgraphics_device->set_method("bool im_begin()", &graphics::graphics_device::im_begin);
				vgraphics_device->set_method("void im_transform(const matrix4x4 &in)", &graphics::graphics_device::im_transform);
				vgraphics_device->set_method("void im_topology(primitive_topology)", &graphics::graphics_device::im_topology);
				vgraphics_device->set_method("void im_emit()", &graphics::graphics_device::im_emit);
				vgraphics_device->set_method("void im_texture(texture_2d@+)", &graphics::graphics_device::im_texture);
				vgraphics_device->set_method("void im_color(float, float, float, float)", &graphics::graphics_device::im_color);
				vgraphics_device->set_method("void im_intensity(float)", &graphics::graphics_device::im_intensity);
				vgraphics_device->set_method("void im_texcoord(float, float)", &graphics::graphics_device::im_texcoord);
				vgraphics_device->set_method("void im_texcoord_offset(float, float)", &graphics::graphics_device::im_texcoord_offset);
				vgraphics_device->set_method("void im_position(float, float, float)", &graphics::graphics_device::im_position);
				vgraphics_device->set_method("bool im_end()", &graphics::graphics_device::im_end);
				vgraphics_device->set_method_ex("bool submit()", &VI_EXPECTIFY_VOID(graphics::graphics_device::submit));
				vgraphics_device->set_method_ex("depth_stencil_state@ create_depth_stencil_state(const depth_stencil_state_desc &in)", &VI_EXPECTIFY(graphics::graphics_device::create_depth_stencil_state));
				vgraphics_device->set_method_ex("blend_state@ create_blend_state(const blend_state_desc &in)", &VI_EXPECTIFY(graphics::graphics_device::create_blend_state));
				vgraphics_device->set_method_ex("rasterizer_state@ create_rasterizer_state(const rasterizer_state_desc &in)", &VI_EXPECTIFY(graphics::graphics_device::create_rasterizer_state));
				vgraphics_device->set_method_ex("sampler_state@ create_sampler_state(const sampler_state_desc &in)", &VI_EXPECTIFY(graphics::graphics_device::create_sampler_state));
				vgraphics_device->set_method_ex("input_layout@ create_input_layout(const input_layout_desc &in)", &VI_EXPECTIFY(graphics::graphics_device::create_input_layout));
				vgraphics_device->set_method_ex("shader@ create_shader(const shader_desc &in)", &VI_EXPECTIFY(graphics::graphics_device::create_shader));
				vgraphics_device->set_method_ex("element_buffer@ create_element_buffer(const element_buffer_desc &in)", &VI_EXPECTIFY(graphics::graphics_device::create_element_buffer));
				vgraphics_device->set_method_ex("mesh_buffer@ create_mesh_buffer(const mesh_buffer_desc &in)", &graphics_device_create_mesh_buffer1);
				vgraphics_device->set_method_ex("mesh_buffer@ create_mesh_buffer(element_buffer@+, element_buffer@+)", &graphics_device_create_mesh_buffer2);
				vgraphics_device->set_method_ex("skin_mesh_buffer@ create_skin_mesh_buffer(const skin_mesh_buffer_desc &in)", &graphics_device_create_skin_mesh_buffer1);
				vgraphics_device->set_method_ex("skin_mesh_buffer@ create_skin_mesh_buffer(element_buffer@+, element_buffer@+)", &graphics_device_create_skin_mesh_buffer2);
				vgraphics_device->set_method_ex("instance_buffer@ create_instance_buffer(const instance_buffer_desc &in)", &graphics_device_create_instance_buffer);
				vgraphics_device->set_method_ex("texture_2d@ create_texture_2d()", &graphics_device_create_texture_2d1);
				vgraphics_device->set_method_ex("texture_2d@ create_texture_2d(const texture_2d_desc &in)", &graphics_device_create_texture_2d2);
				vgraphics_device->set_method_ex("texture_3d@ create_texture_3d()", &graphics_device_create_texture_3d1);
				vgraphics_device->set_method_ex("texture_3d@ create_texture_3d(const texture_3d_desc &in)", &graphics_device_create_texture_3d2);
				vgraphics_device->set_method_ex("texture_cube@ create_texture_cube()", &graphics_device_create_texture_cube1);
				vgraphics_device->set_method_ex("texture_cube@ create_texture_cube(const texture_cube_desc &in)", &graphics_device_create_texture_cube2);
				vgraphics_device->set_method_ex("texture_cube@ create_texture_cube(array<texture_2d@>@+)", &graphics_device_create_texture_cube3);
				vgraphics_device->set_method_ex("texture_cube@ create_texture_cube(texture_2d@+)", &graphics_device_create_texture_cube4);
				vgraphics_device->set_method_ex("depth_target_2d@ create_depth_target_2d(const depth_target_2d_desc &in)", &VI_EXPECTIFY(graphics::graphics_device::create_depth_target_2d));
				vgraphics_device->set_method_ex("depth_target_cube@ create_depth_target_cube(const depth_target_cube_desc &in)", &VI_EXPECTIFY(graphics::graphics_device::create_depth_target_cube));
				vgraphics_device->set_method_ex("render_target_2d@ create_render_target_2d(const render_target_2d_desc &in)", &VI_EXPECTIFY(graphics::graphics_device::create_render_target_2d));
				vgraphics_device->set_method_ex("multi_render_target_2d@ create_multi_render_target_2d(const multi_render_target_2d_desc &in)", &VI_EXPECTIFY(graphics::graphics_device::create_multi_render_target_2d));
				vgraphics_device->set_method_ex("render_target_cube@ create_render_target_cube(const render_target_cube_desc &in)", &VI_EXPECTIFY(graphics::graphics_device::create_render_target_cube));
				vgraphics_device->set_method_ex("multi_render_target_cube@ create_multi_render_target_cube(const multi_render_target_cube_desc &in)", &VI_EXPECTIFY(graphics::graphics_device::create_multi_render_target_cube));
				vgraphics_device->set_method_ex("cubemap@ create_cubemap(const cubemap_desc &in)", &VI_EXPECTIFY(graphics::graphics_device::create_cubemap));
				vgraphics_device->set_method_ex("visibility_query@ create_query(const visibility_query_desc &in)", &VI_EXPECTIFY(graphics::graphics_device::create_query));
				vgraphics_device->set_method_ex("activity_surface@ create_surface(texture_2d@+)", &VI_EXPECTIFY(graphics::graphics_device::create_surface));
				vgraphics_device->set_method("primitive_topology get_primitive_topology() const", &graphics::graphics_device::get_primitive_topology);
				vgraphics_device->set_method("shader_model get_supported_shader_model()  const", &graphics::graphics_device::get_supported_shader_model);
				vgraphics_device->set_method("uptr@ get_device() const", &graphics::graphics_device::get_device);
				vgraphics_device->set_method("uptr@ get_context() const", &graphics::graphics_device::get_context);
				vgraphics_device->set_method("bool is_valid() const", &graphics::graphics_device::is_valid);
				vgraphics_device->set_method("void set_vertex_buffer(element_buffer@+)", &graphics::graphics_device::set_vertex_buffer);
				vgraphics_device->set_method("void set_shader_cache(bool)", &graphics::graphics_device::set_shader_cache);
				vgraphics_device->set_method("void set_vsync_mode(vsync)", &graphics::graphics_device::set_vsync_mode);
				vgraphics_device->set_method("bool preprocess(shader_desc &in)", &graphics::graphics_device::preprocess);
				vgraphics_device->set_method("bool transpile(string &out, shader_type, shader_lang)", &graphics::graphics_device::transpile);
				vgraphics_device->set_method("bool add_section(const string_view&in, const string_view&in)", &graphics::graphics_device::add_section);
				vgraphics_device->set_method("bool remove_section(const string_view&in)", &graphics::graphics_device::remove_section);
				vgraphics_device->set_method("bool get_section_data(const string_view&in, shader_desc &out)", &graphics::graphics_device::get_section_data);
				vgraphics_device->set_method("bool is_left_handed() const", &graphics::graphics_device::is_left_handed);
				vgraphics_device->set_method("string get_shader_main(shader_type) const", &graphics::graphics_device::get_shader_main);
				vgraphics_device->set_method("depth_stencil_state@+ get_depth_stencil_state(const string_view&in)", &graphics::graphics_device::get_depth_stencil_state);
				vgraphics_device->set_method("blend_state@+ get_blend_state(const string_view&in)", &graphics::graphics_device::get_blend_state);
				vgraphics_device->set_method("rasterizer_state@+ get_rasterizer_state(const string_view&in)", &graphics::graphics_device::get_rasterizer_state);
				vgraphics_device->set_method("sampler_state@+ get_sampler_state(const string_view&in)", &graphics::graphics_device::get_sampler_state);
				vgraphics_device->set_method("input_layout@+ get_input_layout(const string_view&in)", &graphics::graphics_device::get_input_layout);
				vgraphics_device->set_method("shader_model get_shader_model() const", &graphics::graphics_device::get_shader_model);
				vgraphics_device->set_method("render_target_2d@+ get_render_target()", &graphics::graphics_device::get_render_target);
				vgraphics_device->set_method("render_backend get_backend() const", &graphics::graphics_device::get_backend);
				vgraphics_device->set_method("uint32 get_format_size(surface_format) const", &graphics::graphics_device::get_format_size);
				vgraphics_device->set_method("uint32 get_present_flags() const", &graphics::graphics_device::get_present_flags);
				vgraphics_device->set_method("uint32 get_compile_flags() const", &graphics::graphics_device::get_compile_flags);
				vgraphics_device->set_method("uint32 get_mip_level(uint32, uint32) const", &graphics::graphics_device::get_mip_level);
				vgraphics_device->set_method_ex("string get_program_name(const shader_desc &in) const", &VI_OPTIONIFY(graphics::graphics_device::get_program_name));
				vgraphics_device->set_method("vsync get_vsync_mode() const", &graphics::graphics_device::get_vsync_mode);
				vgraphics_device->set_method("bool is_debug() const", &graphics::graphics_device::is_debug);
				vgraphics_device->set_method_static("graphics_device@ create(graphics_device_desc &in)", &graphics_device_create);
				vgraphics_device->set_method_static("void compile_buildin_shaders(array<graphics_device@>@+)", &graphics_device_compile_builtin_shaders);
				vgraphics_device->set_enum_refs_ex<graphics::graphics_device>([](graphics::graphics_device* base, asIScriptEngine* vm) { });
				vgraphics_device->set_release_refs_ex<graphics::graphics_device>([](graphics::graphics_device* base, asIScriptEngine*) { });

				vrender_target->set_dynamic_cast<graphics::render_target, graphics::render_target_2d>("render_target_2d@+");
				vrender_target->set_dynamic_cast<graphics::render_target, graphics::render_target_cube>("render_target_cube@+");
				vrender_target->set_dynamic_cast<graphics::render_target, graphics::multi_render_target_2d>("multi_render_target_2d@+");
				vrender_target->set_dynamic_cast<graphics::render_target, graphics::multi_render_target_cube>("multi_render_target_cube@+");
				vrender_target_2d->set_dynamic_cast<graphics::render_target_2d, graphics::render_target>("render_target@+", true);
				vmulti_render_target_2d->set_dynamic_cast<graphics::multi_render_target_2d, graphics::render_target>("render_target@+", true);
				vrender_target_cube->set_dynamic_cast<graphics::render_target_cube, graphics::render_target>("render_target@+", true);
				vmulti_render_target_cube->set_dynamic_cast<graphics::multi_render_target_cube, graphics::render_target>("render_target@+", true);

				return true;
#else
				VI_ASSERT(false, "<graphics> is not loaded");
				return false;
#endif
			}
			bool heavy_registry::import_audio(virtual_machine* vm) noexcept
			{
#ifdef VI_BINDINGS
				VI_ASSERT(vm != nullptr, "manager should be set");
				VI_TYPEREF(audio_source, "audio_source");

				auto vsound_distance_model = vm->set_enum("sound_distance_model");
				vsound_distance_model->set_value("invalid", (int)audio::sound_distance_model::invalid);
				vsound_distance_model->set_value("invert", (int)audio::sound_distance_model::invert);
				vsound_distance_model->set_value("invert_clamp", (int)audio::sound_distance_model::invert_clamp);
				vsound_distance_model->set_value("linear", (int)audio::sound_distance_model::linear);
				vsound_distance_model->set_value("linear_clamp", (int)audio::sound_distance_model::linear_clamp);
				vsound_distance_model->set_value("exponent", (int)audio::sound_distance_model::exponent);
				vsound_distance_model->set_value("exponent_clamp", (int)audio::sound_distance_model::exponent_clamp);

				auto vsound_ex = vm->set_enum("sound_ex");
				vsound_ex->set_value("source_relative", (int)audio::sound_ex::source_relative);
				vsound_ex->set_value("cone_inner_angle", (int)audio::sound_ex::cone_inner_angle);
				vsound_ex->set_value("cone_outer_angle", (int)audio::sound_ex::cone_outer_angle);
				vsound_ex->set_value("pitch", (int)audio::sound_ex::pitch);
				vsound_ex->set_value("position", (int)audio::sound_ex::position);
				vsound_ex->set_value("direction", (int)audio::sound_ex::direction);
				vsound_ex->set_value("velocity", (int)audio::sound_ex::velocity);
				vsound_ex->set_value("looping", (int)audio::sound_ex::looping);
				vsound_ex->set_value("buffer", (int)audio::sound_ex::buffer);
				vsound_ex->set_value("gain", (int)audio::sound_ex::gain);
				vsound_ex->set_value("min_gain", (int)audio::sound_ex::min_gain);
				vsound_ex->set_value("max_gain", (int)audio::sound_ex::max_gain);
				vsound_ex->set_value("orientation", (int)audio::sound_ex::orientation);
				vsound_ex->set_value("channel_mask", (int)audio::sound_ex::channel_mask);
				vsound_ex->set_value("source_state", (int)audio::sound_ex::source_state);
				vsound_ex->set_value("initial", (int)audio::sound_ex::initial);
				vsound_ex->set_value("playing", (int)audio::sound_ex::playing);
				vsound_ex->set_value("paused", (int)audio::sound_ex::paused);
				vsound_ex->set_value("stopped", (int)audio::sound_ex::stopped);
				vsound_ex->set_value("buffers_queued", (int)audio::sound_ex::buffers_queued);
				vsound_ex->set_value("buffers_processed", (int)audio::sound_ex::buffers_processed);
				vsound_ex->set_value("seconds_offset", (int)audio::sound_ex::seconds_offset);
				vsound_ex->set_value("sample_offset", (int)audio::sound_ex::sample_offset);
				vsound_ex->set_value("byte_offset", (int)audio::sound_ex::byte_offset);
				vsound_ex->set_value("source_type", (int)audio::sound_ex::source_type);
				vsound_ex->set_value("static", (int)audio::sound_ex::constant);
				vsound_ex->set_value("streaming", (int)audio::sound_ex::streaming);
				vsound_ex->set_value("undetermined", (int)audio::sound_ex::undetermined);
				vsound_ex->set_value("format_mono8", (int)audio::sound_ex::format_mono8);
				vsound_ex->set_value("format_mono16", (int)audio::sound_ex::format_mono16);
				vsound_ex->set_value("format_stereo8", (int)audio::sound_ex::format_stereo8);
				vsound_ex->set_value("format_stereo16", (int)audio::sound_ex::format_stereo16);
				vsound_ex->set_value("reference_distance", (int)audio::sound_ex::reference_distance);
				vsound_ex->set_value("rolloff_gactor", (int)audio::sound_ex::rolloff_factor);
				vsound_ex->set_value("cone_outer_gain", (int)audio::sound_ex::cone_outer_gain);
				vsound_ex->set_value("max_distance", (int)audio::sound_ex::max_distance);
				vsound_ex->set_value("frequency", (int)audio::sound_ex::frequency);
				vsound_ex->set_value("bits", (int)audio::sound_ex::bits);
				vsound_ex->set_value("channels", (int)audio::sound_ex::channels);
				vsound_ex->set_value("size", (int)audio::sound_ex::size);
				vsound_ex->set_value("unused", (int)audio::sound_ex::unused);
				vsound_ex->set_value("pending", (int)audio::sound_ex::pending);
				vsound_ex->set_value("processed", (int)audio::sound_ex::processed);
				vsound_ex->set_value("invalid_name", (int)audio::sound_ex::invalid_name);
				vsound_ex->set_value("illegal_enum", (int)audio::sound_ex::illegal_enum);
				vsound_ex->set_value("invalid_enum", (int)audio::sound_ex::invalid_enum);
				vsound_ex->set_value("invalid_value", (int)audio::sound_ex::invalid_value);
				vsound_ex->set_value("illegal_command", (int)audio::sound_ex::illegal_command);
				vsound_ex->set_value("invalid_operation", (int)audio::sound_ex::invalid_operation);
				vsound_ex->set_value("out_of_memory", (int)audio::sound_ex::out_of_memory);
				vsound_ex->set_value("vendor", (int)audio::sound_ex::vendor);
				vsound_ex->set_value("version", (int)audio::sound_ex::version);
				vsound_ex->set_value("renderer", (int)audio::sound_ex::renderer);
				vsound_ex->set_value("extentions", (int)audio::sound_ex::extentions);
				vsound_ex->set_value("doppler_factor", (int)audio::sound_ex::doppler_factor);
				vsound_ex->set_value("doppler_velocity", (int)audio::sound_ex::doppler_velocity);
				vsound_ex->set_value("speed_of_sound", (int)audio::sound_ex::speed_of_sound);

				auto vaudio_sync = vm->set_pod<audio::audio_sync>("audio_sync");
				vaudio_sync->set_property<audio::audio_sync>("vector3 direction", &audio::audio_sync::direction);
				vaudio_sync->set_property<audio::audio_sync>("vector3 velocity", &audio::audio_sync::velocity);
				vaudio_sync->set_property<audio::audio_sync>("float cone_inner_angle", &audio::audio_sync::cone_inner_angle);
				vaudio_sync->set_property<audio::audio_sync>("float cone_outer_angle", &audio::audio_sync::cone_outer_angle);
				vaudio_sync->set_property<audio::audio_sync>("float cone_outer_gain", &audio::audio_sync::cone_outer_gain);
				vaudio_sync->set_property<audio::audio_sync>("float pitch", &audio::audio_sync::pitch);
				vaudio_sync->set_property<audio::audio_sync>("float gain", &audio::audio_sync::gain);
				vaudio_sync->set_property<audio::audio_sync>("float ref_distance", &audio::audio_sync::ref_distance);
				vaudio_sync->set_property<audio::audio_sync>("float distance", &audio::audio_sync::distance);
				vaudio_sync->set_property<audio::audio_sync>("float rolloff", &audio::audio_sync::rolloff);
				vaudio_sync->set_property<audio::audio_sync>("float position", &audio::audio_sync::position);
				vaudio_sync->set_property<audio::audio_sync>("float air_absorption", &audio::audio_sync::air_absorption);
				vaudio_sync->set_property<audio::audio_sync>("float room_roll_off", &audio::audio_sync::room_roll_off);
				vaudio_sync->set_property<audio::audio_sync>("bool is_relative", &audio::audio_sync::is_relative);
				vaudio_sync->set_property<audio::audio_sync>("bool is_looped", &audio::audio_sync::is_looped);
				vaudio_sync->set_constructor<audio::audio_sync>("void f()");

				vm->begin_namespace("audio_context");
				vm->set_function("bool initialize()", &VI_SEXPECTIFY_VOID(audio::audio_context::initialize));
				vm->set_function("bool generate_buffers(int32, uint32 &out)", &VI_SEXPECTIFY_VOID(audio::audio_context::generate_buffers));
				vm->set_function("bool set_source_data_3f(uint32, sound_ex, float, float, float)", &VI_SEXPECTIFY_VOID(audio::audio_context::set_source_data3f));
				vm->set_function("bool get_source_data_3f(uint32, sound_ex, float &out, float &out, float &out)", &VI_SEXPECTIFY_VOID(audio::audio_context::get_source_data3f));
				vm->set_function("bool set_source_data_1f(uint32, sound_ex, float)", &VI_SEXPECTIFY_VOID(audio::audio_context::set_source_data1f));
				vm->set_function("bool get_source_data_1f(uint32, sound_ex, float &out)", &VI_SEXPECTIFY_VOID(audio::audio_context::get_source_data1f));
				vm->set_function("bool set_source_data_3i(uint32, sound_ex, int32, int32, int32)", &VI_SEXPECTIFY_VOID(audio::audio_context::set_source_data3i));
				vm->set_function("bool get_source_data_3i(uint32, sound_ex, int32 &out, int32 &out, int32 &out)", &VI_SEXPECTIFY_VOID(audio::audio_context::get_source_data3i));
				vm->set_function("bool set_source_data_1i(uint32, sound_ex, int32)", &VI_SEXPECTIFY_VOID(audio::audio_context::set_source_data1i));
				vm->set_function("bool get_source_data_1i(uint32, sound_ex, int32 &out)", &VI_SEXPECTIFY_VOID(audio::audio_context::get_source_data1i));
				vm->set_function("bool set_listener_data_3f(sound_ex, float, float, float)", &VI_SEXPECTIFY_VOID(audio::audio_context::set_listener_data3f));
				vm->set_function("bool get_listener_data_3f(sound_ex, float &out, float &out, float &out)", &VI_SEXPECTIFY_VOID(audio::audio_context::get_listener_data3f));
				vm->set_function("bool set_listener_data_1f(sound_ex, float)", &VI_SEXPECTIFY_VOID(audio::audio_context::set_listener_data1f));
				vm->set_function("bool get_listener_data_1f(sound_ex, float &out)", &VI_SEXPECTIFY_VOID(audio::audio_context::get_listener_data1f));
				vm->set_function("bool set_listener_data_3i(sound_ex, int32, int32, int32)", &VI_SEXPECTIFY_VOID(audio::audio_context::set_listener_data3i));
				vm->set_function("bool get_listener_data_3i(sound_ex, int32 &out, int32 &out, int32 &out)", &VI_SEXPECTIFY_VOID(audio::audio_context::get_listener_data3i));
				vm->set_function("bool set_listener_data_1i(sound_ex, int32)", &VI_SEXPECTIFY_VOID(audio::audio_context::set_listener_data1i));
				vm->set_function("bool get_listener_data_1i(sound_ex, int32 &out)", &VI_SEXPECTIFY_VOID(audio::audio_context::get_listener_data1i));
				vm->end_namespace();

				auto vaudio_source = vm->set_class<audio::audio_source>("audio_source", true);
				auto vaudio_filter = vm->set_class<audio::audio_filter>("base_audio_filter", false);
				populate_audio_filter_base<audio::audio_filter>(*vaudio_filter);

				auto vaudio_effect = vm->set_class<audio::audio_effect>("base_audio_effect", false);
				populate_audio_effect_base<audio::audio_effect>(*vaudio_effect);

				auto vaudio_clip = vm->set_class<audio::audio_clip>("audio_clip", false);
				vaudio_clip->set_constructor<audio::audio_clip, int, int>("audio_clip@ f(int, int)");
				vaudio_clip->set_method("float length() const", &audio::audio_clip::length);
				vaudio_clip->set_method("bool is_mono() const", &audio::audio_clip::is_mono);
				vaudio_clip->set_method("uint32 get_buffer() const", &audio::audio_clip::get_buffer);
				vaudio_clip->set_method("int32 get_format() const", &audio::audio_clip::get_format);

				vaudio_source->set_gc_constructor<audio::audio_source, audio_source>("audio_source@ f()");
				vaudio_source->set_method("int64 add_effect(base_audio_effect@+)", &audio::audio_source::add_effect);
				vaudio_source->set_method_ex("bool remove_effect(usize)", &VI_EXPECTIFY_VOID(audio::audio_source::remove_effect));
				vaudio_source->set_method_ex("bool remove_effect_by_id(uint64)", &VI_EXPECTIFY_VOID(audio::audio_source::remove_effect_by_id));
				vaudio_source->set_method_ex("bool set_clip(audio_clip@+)", &VI_EXPECTIFY_VOID(audio::audio_source::set_clip));
				vaudio_source->set_method_ex("bool synchronize(audio_sync &in, const vector3 &in)", &VI_EXPECTIFY_VOID(audio::audio_source::synchronize));
				vaudio_source->set_method_ex("bool reset()", &VI_EXPECTIFY_VOID(audio::audio_source::reset));
				vaudio_source->set_method_ex("bool pause()", &VI_EXPECTIFY_VOID(audio::audio_source::pause));
				vaudio_source->set_method_ex("bool play()", &VI_EXPECTIFY_VOID(audio::audio_source::play));
				vaudio_source->set_method_ex("bool stop()", &VI_EXPECTIFY_VOID(audio::audio_source::stop));
				vaudio_source->set_method("bool is_playing() const", &audio::audio_source::is_playing);
				vaudio_source->set_method("usize get_effects_count() const", &audio::audio_source::get_effects_count);
				vaudio_source->set_method("uint32 get_instance() const", &audio::audio_source::get_instance);
				vaudio_source->set_method("audio_clip@+ get_clip() const", &audio::audio_source::get_clip);
				vaudio_source->set_method<audio::audio_source, audio::audio_effect*, uint64_t>("base_audio_effect@+ get_effect(uint64) const", &audio::audio_source::get_effect);
				vaudio_source->set_enum_refs_ex<audio::audio_source>([](audio::audio_source* base, asIScriptEngine* vm)
				{
					for (auto* item : base->get_effects())
						function_factory::gc_enum_callback(vm, item);
				});
				vaudio_source->set_release_refs_ex<audio::audio_source>([](audio::audio_source* base, asIScriptEngine*)
				{
					base->remove_effects();
				});

				auto vaudio_device = vm->set_class<audio::audio_device>("audio_device", false);
				vaudio_device->set_constructor<audio::audio_device>("audio_device@ f()");
				vaudio_device->set_method_ex("bool offset(audio_source@+, float &out, bool)", &VI_EXPECTIFY_VOID(audio::audio_device::offset));
				vaudio_device->set_method_ex("bool velocity(audio_source@+, vector3 &out, bool)", &VI_EXPECTIFY_VOID(audio::audio_device::velocity));
				vaudio_device->set_method_ex("bool position(audio_source@+, vector3 &out, bool)", &VI_EXPECTIFY_VOID(audio::audio_device::position));
				vaudio_device->set_method_ex("bool direction(audio_source@+, vector3 &out, bool)", &VI_EXPECTIFY_VOID(audio::audio_device::direction));
				vaudio_device->set_method_ex("bool relative(audio_source@+, int &out, bool)", &VI_EXPECTIFY_VOID(audio::audio_device::relative));
				vaudio_device->set_method_ex("bool pitch(audio_source@+, float &out, bool)", &VI_EXPECTIFY_VOID(audio::audio_device::pitch));
				vaudio_device->set_method_ex("bool gain(audio_source@+, float &out, bool)", &VI_EXPECTIFY_VOID(audio::audio_device::gain));
				vaudio_device->set_method_ex("bool loop(audio_source@+, int &out, bool)", &VI_EXPECTIFY_VOID(audio::audio_device::loop));
				vaudio_device->set_method_ex("bool cone_inner_angle(audio_source@+, float &out, bool)", &VI_EXPECTIFY_VOID(audio::audio_device::cone_inner_angle));
				vaudio_device->set_method_ex("bool cone_outer_angle(audio_source@+, float &out, bool)", &VI_EXPECTIFY_VOID(audio::audio_device::cone_outer_angle));
				vaudio_device->set_method_ex("bool cone_outer_gain(audio_source@+, float &out, bool)", &VI_EXPECTIFY_VOID(audio::audio_device::cone_outer_gain));
				vaudio_device->set_method_ex("bool distance(audio_source@+, float &out, bool)", &VI_EXPECTIFY_VOID(audio::audio_device::distance));
				vaudio_device->set_method_ex("bool ref_distance(audio_source@+, float &out, bool)", &VI_EXPECTIFY_VOID(audio::audio_device::ref_distance));
				vaudio_device->set_method_ex("bool set_distance_model(sound_distance_model)", &VI_EXPECTIFY_VOID(audio::audio_device::set_distance_model));
				vaudio_device->set_method("void display_audio_log() const", &audio::audio_device::display_audio_log);
				vaudio_device->set_method("bool is_valid() const", &audio::audio_device::is_valid);

				return true;
#else
				VI_ASSERT(false, "<audio> is not loaded");
				return false;
#endif
			}
			bool heavy_registry::import_audio_effects(virtual_machine* vm) noexcept
			{
#ifdef VI_BINDINGS
				VI_ASSERT(vm != nullptr, "manager should be set");

				auto vreverb = vm->set_class<audio::effects::reverb>("reverb_effect", false);
				vreverb->set_property<audio::effects::reverb>("vector3 late_reverb_pan", &audio::effects::reverb::late_reverb_pan);
				vreverb->set_property<audio::effects::reverb>("vector3 reflections_pan", &audio::effects::reverb::reflections_pan);
				vreverb->set_property<audio::effects::reverb>("float density", &audio::effects::reverb::density);
				vreverb->set_property<audio::effects::reverb>("float diffusion", &audio::effects::reverb::diffusion);
				vreverb->set_property<audio::effects::reverb>("float gain", &audio::effects::reverb::gain);
				vreverb->set_property<audio::effects::reverb>("float gain_hf", &audio::effects::reverb::gain_hf);
				vreverb->set_property<audio::effects::reverb>("float gain_lf", &audio::effects::reverb::gain_lf);
				vreverb->set_property<audio::effects::reverb>("float decay_time", &audio::effects::reverb::decay_time);
				vreverb->set_property<audio::effects::reverb>("float decay_hf_ratio", &audio::effects::reverb::decay_hf_ratio);
				vreverb->set_property<audio::effects::reverb>("float decay_lf_ratio", &audio::effects::reverb::decay_lf_ratio);
				vreverb->set_property<audio::effects::reverb>("float reflections_gain", &audio::effects::reverb::reflections_gain);
				vreverb->set_property<audio::effects::reverb>("float reflections_delay", &audio::effects::reverb::reflections_delay);
				vreverb->set_property<audio::effects::reverb>("float late_reverb_gain", &audio::effects::reverb::late_reverb_gain);
				vreverb->set_property<audio::effects::reverb>("float late_reverb_delay", &audio::effects::reverb::late_reverb_delay);
				vreverb->set_property<audio::effects::reverb>("float echo_time", &audio::effects::reverb::echo_time);
				vreverb->set_property<audio::effects::reverb>("float echo_depth", &audio::effects::reverb::echo_depth);
				vreverb->set_property<audio::effects::reverb>("float modulation_time", &audio::effects::reverb::modulation_time);
				vreverb->set_property<audio::effects::reverb>("float modulation_depth", &audio::effects::reverb::modulation_depth);
				vreverb->set_property<audio::effects::reverb>("float air_absorption_gain_hf", &audio::effects::reverb::air_absorption_gain_hf);
				vreverb->set_property<audio::effects::reverb>("float hf_reference", &audio::effects::reverb::hf_reference);
				vreverb->set_property<audio::effects::reverb>("float lf_reference", &audio::effects::reverb::lf_reference);
				vreverb->set_property<audio::effects::reverb>("float room_rolloff_factor", &audio::effects::reverb::room_rolloff_factor);
				vreverb->set_property<audio::effects::reverb>("bool is_decay_hf_limited", &audio::effects::reverb::is_decay_hf_limited);
				populate_audio_effect_interface<audio::effects::reverb>(*vreverb, "reverb_effect@+ f()");

				auto vchorus = vm->set_class<audio::effects::chorus>("chorus_effect", false);
				vchorus->set_property<audio::effects::chorus>("float rate", &audio::effects::chorus::rate);
				vchorus->set_property<audio::effects::chorus>("float depth", &audio::effects::chorus::depth);
				vchorus->set_property<audio::effects::chorus>("float feedback", &audio::effects::chorus::feedback);
				vchorus->set_property<audio::effects::chorus>("float delay", &audio::effects::chorus::delay);
				vchorus->set_property<audio::effects::chorus>("int32 waveform", &audio::effects::chorus::waveform);
				vchorus->set_property<audio::effects::chorus>("int32 phase", &audio::effects::chorus::phase);
				populate_audio_effect_interface<audio::effects::chorus>(*vchorus, "chorus_effect@+ f()");

				auto vdistortion = vm->set_class<audio::effects::distortion>("distortion_effect", false);
				vdistortion->set_property<audio::effects::distortion>("float edge", &audio::effects::distortion::edge);
				vdistortion->set_property<audio::effects::distortion>("float gain", &audio::effects::distortion::gain);
				vdistortion->set_property<audio::effects::distortion>("float lowpass_cut_off", &audio::effects::distortion::lowpass_cut_off);
				vdistortion->set_property<audio::effects::distortion>("float eq_center", &audio::effects::distortion::eq_center);
				vdistortion->set_property<audio::effects::distortion>("float eq_bandwidth", &audio::effects::distortion::eq_bandwidth);
				populate_audio_effect_interface<audio::effects::distortion>(*vdistortion, "distortion_effect@+ f()");

				auto vecho = vm->set_class<audio::effects::echo>("echo_effect", false);
				vecho->set_property<audio::effects::echo>("float delay", &audio::effects::echo::delay);
				vecho->set_property<audio::effects::echo>("float lr_delay", &audio::effects::echo::lr_delay);
				vecho->set_property<audio::effects::echo>("float damping", &audio::effects::echo::damping);
				vecho->set_property<audio::effects::echo>("float feedback", &audio::effects::echo::feedback);
				vecho->set_property<audio::effects::echo>("float spread", &audio::effects::echo::spread);
				populate_audio_effect_interface<audio::effects::echo>(*vecho, "echo_effect@+ f()");

				auto vflanger = vm->set_class<audio::effects::flanger>("flanger_effect", false);
				vflanger->set_property<audio::effects::flanger>("float rate", &audio::effects::flanger::rate);
				vflanger->set_property<audio::effects::flanger>("float depth", &audio::effects::flanger::depth);
				vflanger->set_property<audio::effects::flanger>("float feedback", &audio::effects::flanger::feedback);
				vflanger->set_property<audio::effects::flanger>("float delay", &audio::effects::flanger::delay);
				vflanger->set_property<audio::effects::flanger>("int32 waveform", &audio::effects::flanger::waveform);
				vflanger->set_property<audio::effects::flanger>("int32 phase", &audio::effects::flanger::phase);
				populate_audio_effect_interface<audio::effects::flanger>(*vflanger, "flanger_effect@+ f()");

				auto vfrequency_shifter = vm->set_class<audio::effects::frequency_shifter>("frequency_shifter_effect", false);
				vfrequency_shifter->set_property<audio::effects::frequency_shifter>("float frequency", &audio::effects::frequency_shifter::frequency);
				vfrequency_shifter->set_property<audio::effects::frequency_shifter>("int32 left_direction", &audio::effects::frequency_shifter::left_direction);
				vfrequency_shifter->set_property<audio::effects::frequency_shifter>("int32 right_direction", &audio::effects::frequency_shifter::right_direction);
				populate_audio_effect_interface<audio::effects::frequency_shifter>(*vfrequency_shifter, "frequency_shifter_effect@+ f()");

				auto vvocal_morpher = vm->set_class<audio::effects::vocal_morpher>("vocal_morpher_effect", false);
				vvocal_morpher->set_property<audio::effects::vocal_morpher>("float rate", &audio::effects::vocal_morpher::rate);
				vvocal_morpher->set_property<audio::effects::vocal_morpher>("int32 phonemea", &audio::effects::vocal_morpher::phonemea);
				vvocal_morpher->set_property<audio::effects::vocal_morpher>("int32 phonemea_coarse_tuning", &audio::effects::vocal_morpher::phonemea_coarse_tuning);
				vvocal_morpher->set_property<audio::effects::vocal_morpher>("int32 phonemeb", &audio::effects::vocal_morpher::phonemeb);
				vvocal_morpher->set_property<audio::effects::vocal_morpher>("int32 phonemeb_coarse_tuning", &audio::effects::vocal_morpher::phonemeb_coarse_tuning);
				vvocal_morpher->set_property<audio::effects::vocal_morpher>("int32 waveform", &audio::effects::vocal_morpher::waveform);
				populate_audio_effect_interface<audio::effects::vocal_morpher>(*vvocal_morpher, "vocal_morpher_effect@+ f()");

				auto vpitch_shifter = vm->set_class<audio::effects::pitch_shifter>("pitch_shifter_effect", false);
				vpitch_shifter->set_property<audio::effects::pitch_shifter>("int32 coarse_tune", &audio::effects::pitch_shifter::coarse_tune);
				vpitch_shifter->set_property<audio::effects::pitch_shifter>("int32 fine_tune", &audio::effects::pitch_shifter::fine_tune);
				populate_audio_effect_interface<audio::effects::pitch_shifter>(*vpitch_shifter, "pitch_shifter_effect@+ f()");

				auto vring_modulator = vm->set_class<audio::effects::ring_modulator>("ring_modulator_effect", false);
				vring_modulator->set_property<audio::effects::ring_modulator>("float frequency", &audio::effects::ring_modulator::frequency);
				vring_modulator->set_property<audio::effects::ring_modulator>("float highpass_cut_off", &audio::effects::ring_modulator::highpass_cut_off);
				vring_modulator->set_property<audio::effects::ring_modulator>("int32 waveform", &audio::effects::ring_modulator::waveform);
				populate_audio_effect_interface<audio::effects::ring_modulator>(*vring_modulator, "ring_modulator_effect@+ f()");

				auto vautowah = vm->set_class<audio::effects::autowah>("autowah_effect", false);
				vautowah->set_property<audio::effects::autowah>("float attack_time", &audio::effects::autowah::attack_time);
				vautowah->set_property<audio::effects::autowah>("float release_time", &audio::effects::autowah::release_time);
				vautowah->set_property<audio::effects::autowah>("float resonance", &audio::effects::autowah::resonance);
				vautowah->set_property<audio::effects::autowah>("float peak_gain", &audio::effects::autowah::peak_gain);
				populate_audio_effect_interface<audio::effects::autowah>(*vautowah, "autowah_effect@+ f()");

				auto vcompressor = vm->set_class<audio::effects::compressor>("compressor_effect", false);
				populate_audio_effect_interface<audio::effects::compressor>(*vcompressor, "compressor_effect@+ f()");

				auto vequalizer = vm->set_class<audio::effects::equalizer>("equalizer_effect", false);
				vequalizer->set_property<audio::effects::equalizer>("float low_gain", &audio::effects::equalizer::low_gain);
				vequalizer->set_property<audio::effects::equalizer>("float low_cut_off", &audio::effects::equalizer::low_cut_off);
				vequalizer->set_property<audio::effects::equalizer>("float mid1_gain", &audio::effects::equalizer::mid1_gain);
				vequalizer->set_property<audio::effects::equalizer>("float mid1_center", &audio::effects::equalizer::mid1_center);
				vequalizer->set_property<audio::effects::equalizer>("float mid1_width", &audio::effects::equalizer::mid1_width);
				vequalizer->set_property<audio::effects::equalizer>("float mid2_gain", &audio::effects::equalizer::mid2_gain);
				vequalizer->set_property<audio::effects::equalizer>("float mid2_center", &audio::effects::equalizer::mid2_center);
				vequalizer->set_property<audio::effects::equalizer>("float mid2_width", &audio::effects::equalizer::mid2_width);
				vequalizer->set_property<audio::effects::equalizer>("float high_gain", &audio::effects::equalizer::high_gain);
				vequalizer->set_property<audio::effects::equalizer>("float high_cut_off", &audio::effects::equalizer::high_cut_off);
				populate_audio_effect_interface<audio::effects::equalizer>(*vequalizer, "equalizer_effect@+ f()");

				return true;
#else
				VI_ASSERT(false, "<audio/effects> is not loaded");
				return false;
#endif
			}
			bool heavy_registry::import_audio_filters(virtual_machine* vm) noexcept
			{
#ifdef VI_BINDINGS
				VI_ASSERT(vm != nullptr, "manager should be set");

				auto vlowpass = vm->set_class<audio::filters::lowpass>("lowpass_filter", false);
				vlowpass->set_property<audio::filters::lowpass>("float gain_hf", &audio::filters::lowpass::gain_hf);
				vlowpass->set_property<audio::filters::lowpass>("float gain", &audio::filters::lowpass::gain);
				populate_audio_filter_interface<audio::filters::lowpass>(*vlowpass, "lowpass_filter@+ f()");

				auto vhighpass = vm->set_class<audio::filters::highpass>("highpass_filter", false);
				vhighpass->set_property<audio::filters::highpass>("float gain_lf", &audio::filters::highpass::gain_lf);
				vhighpass->set_property<audio::filters::highpass>("float gain", &audio::filters::highpass::gain);
				populate_audio_filter_interface<audio::filters::highpass>(*vhighpass, "highpass_filter@+ f()");

				auto vbandpass = vm->set_class<audio::filters::bandpass>("bandpass_filter", false);
				vbandpass->set_property<audio::filters::bandpass>("float gain_hf", &audio::filters::bandpass::gain_hf);
				vbandpass->set_property<audio::filters::bandpass>("float gain_lf", &audio::filters::bandpass::gain_lf);
				vbandpass->set_property<audio::filters::bandpass>("float gain", &audio::filters::bandpass::gain);
				populate_audio_filter_interface<audio::filters::bandpass>(*vbandpass, "bandpass_filter@+ f()");

				return true;
#else
				VI_ASSERT(false, "<audio/filters> is not loaded");
				return false;
#endif
			}
			bool heavy_registry::import_engine(virtual_machine* vm) noexcept
			{
#ifdef VI_BINDINGS
				VI_ASSERT(vm != nullptr, "manager should be set");
				VI_TYPEREF(material, "material");
				VI_TYPEREF(model, "model");
				VI_TYPEREF(skin_model, "skin_model");
				VI_TYPEREF(render_system, "render_system");
				VI_TYPEREF(shader_cache, "shader_cache");
				VI_TYPEREF(primitive_cache, "primitive_cache");
				VI_TYPEREF(scene_graph, "scene_graph");
				VI_TYPEREF(heavy_application_name, "heavy_application");

				auto vheavy_application_use = vm->set_enum("heavy_application_use");
				vheavy_application_use->set_value("scripting", (int)layer::USE_SCRIPTING);
				vheavy_application_use->set_value("processing", (int)layer::USE_PROCESSING);
				vheavy_application_use->set_value("networking", (int)layer::USE_NETWORKING);
				vheavy_application_use->set_value("graphics", (int)layer::USE_GRAPHICS);
				vheavy_application_use->set_value("activity", (int)layer::USE_ACTIVITY);
				vheavy_application_use->set_value("audio", (int)layer::USE_AUDIO);

				auto vrender_opt = vm->set_enum("render_opt");
				vrender_opt->set_value("none_t", (int)layer::render_opt::none);
				vrender_opt->set_value("transparent_t", (int)layer::render_opt::transparent);
				vrender_opt->set_value("static_t", (int)layer::render_opt::constant);
				vrender_opt->set_value("additive_t", (int)layer::render_opt::additive);
				vrender_opt->set_value("backfaces_t", (int)layer::render_opt::backfaces);

				auto vrender_culling = vm->set_enum("render_culling");
				vrender_culling->set_value("depth_t", (int)layer::render_culling::depth);
				vrender_culling->set_value("depth_cube_t", (int)layer::render_culling::depth_cube);
				vrender_culling->set_value("disable_t", (int)layer::render_culling::disable);

				auto vrender_state = vm->set_enum("render_state");
				vrender_state->set_value("geometry_t", (int)layer::render_state::geometry);
				vrender_state->set_value("depth_t", (int)layer::render_state::depth);
				vrender_state->set_value("depth_cube_t", (int)layer::render_state::depth_cube);

				auto vgeo_category = vm->set_enum("geo_category");
				vgeo_category->set_value("opaque_t", (int)layer::geo_category::opaque);
				vgeo_category->set_value("transparent_t", (int)layer::geo_category::transparent);
				vgeo_category->set_value("additive_t", (int)layer::geo_category::additive);
				vgeo_category->set_value("count_t", (int)layer::geo_category::count);

				auto vbuffer_type = vm->set_enum("buffer_type");
				vbuffer_type->set_value("index_t", (int)layer::buffer_type::index);
				vbuffer_type->set_value("vertex_t", (int)layer::buffer_type::vertex);

				auto vtarget_type = vm->set_enum("target_type");
				vtarget_type->set_value("main_t", (int)layer::target_type::main);
				vtarget_type->set_value("secondary_t", (int)layer::target_type::secondary);
				vtarget_type->set_value("count_t", (int)layer::target_type::count);

				auto vevent_target = vm->set_enum("event_target");
				vevent_target->set_value("scene_target", (int)layer::event_target::scene);
				vevent_target->set_value("entity_target", (int)layer::event_target::entity);
				vevent_target->set_value("component_target", (int)layer::event_target::component);
				vevent_target->set_value("listener_target", (int)layer::event_target::listener);

				auto vactor_set = vm->set_enum("actor_set");
				vactor_set->set_value("none_t", (int)layer::actor_set::none);
				vactor_set->set_value("update_t", (int)layer::actor_set::update);
				vactor_set->set_value("synchronize_t", (int)layer::actor_set::synchronize);
				vactor_set->set_value("animate_t", (int)layer::actor_set::animate);
				vactor_set->set_value("message_t", (int)layer::actor_set::message);
				vactor_set->set_value("cullable_t", (int)layer::actor_set::cullable);
				vactor_set->set_value("drawable_t", (int)layer::actor_set::drawable);

				auto vactor_type = vm->set_enum("actor_type");
				vactor_type->set_value("update_t", (int)layer::actor_type::update);
				vactor_type->set_value("synchronize_t", (int)layer::actor_type::synchronize);
				vactor_type->set_value("animate_t", (int)layer::actor_type::animate);
				vactor_type->set_value("message_t", (int)layer::actor_type::message);
				vactor_type->set_value("count_t", (int)layer::actor_type::count);

				auto vcomposer_tag = vm->set_enum("composer_tag");
				vcomposer_tag->set_value("component_t", (int)layer::composer_tag::component);
				vcomposer_tag->set_value("renderer_t", (int)layer::composer_tag::renderer);
				vcomposer_tag->set_value("effect_t", (int)layer::composer_tag::effect);
				vcomposer_tag->set_value("filter_t", (int)layer::composer_tag::filter);

				auto vrender_buffer_type = vm->set_enum("render_buffer_type");
				vrender_buffer_type->set_value("Animation", (int)layer::render_buffer_type::animation);
				vrender_buffer_type->set_value("Render", (int)layer::render_buffer_type::render);
				vrender_buffer_type->set_value("View", (int)layer::render_buffer_type::view);

				auto vpose_node = vm->set_pod<layer::pose_node>("pose_node");
				vpose_node->set_property<layer::pose_node>("vector3 position", &layer::pose_node::position);
				vpose_node->set_property<layer::pose_node>("vector3 scale", &layer::pose_node::scale);
				vpose_node->set_property<layer::pose_node>("quaternion rotation", &layer::pose_node::rotation);
				vpose_node->set_constructor<layer::pose_node>("void f()");

				auto vpose_data = vm->set_pod<layer::pose_data>("pose_data");
				vpose_data->set_property<layer::pose_data>("pose_node frame_pose", &layer::pose_data::frame);
				vpose_data->set_property<layer::pose_data>("pose_node offset_pose", &layer::pose_data::offset);
				vpose_data->set_property<layer::pose_data>("pose_node default_pose", &layer::pose_data::defaults);
				vpose_data->set_constructor<layer::pose_data>("void f()");

				auto vanimation_buffer = vm->set_pod<layer::animation_buffer>("animation_buffer");
				vanimation_buffer->set_property<layer::animation_buffer>("vector3 padding", &layer::animation_buffer::padding);
				vanimation_buffer->set_property<layer::animation_buffer>("float animated", &layer::animation_buffer::animated);
				vanimation_buffer->set_constructor<layer::animation_buffer>("void f()");
				vanimation_buffer->set_operator_ex(operators::index, (uint32_t)position::left, "matrix4x4&", "usize", &animation_buffer_get_offsets);
				vanimation_buffer->set_operator_ex(operators::index, (uint32_t)position::constant, "const matrix4x4&", "usize", &animation_buffer_get_offsets);

				auto vrender_buffer_instance = vm->set_pod<layer::render_buffer::instance>("render_buffer_instance");
				vrender_buffer_instance->set_property<layer::render_buffer::instance>("matrix4x4 transform", &layer::render_buffer::instance::transform);
				vrender_buffer_instance->set_property<layer::render_buffer::instance>("matrix4x4 world", &layer::render_buffer::instance::world);
				vrender_buffer_instance->set_property<layer::render_buffer::instance>("vector2 texcoord", &layer::render_buffer::instance::texcoord);
				vrender_buffer_instance->set_property<layer::render_buffer::instance>("float diffuse", &layer::render_buffer::instance::diffuse);
				vrender_buffer_instance->set_property<layer::render_buffer::instance>("float normal", &layer::render_buffer::instance::normal);
				vrender_buffer_instance->set_property<layer::render_buffer::instance>("float height", &layer::render_buffer::instance::height);
				vrender_buffer_instance->set_property<layer::render_buffer::instance>("float material_id", &layer::render_buffer::instance::material_id);
				vrender_buffer_instance->set_constructor<layer::render_buffer::instance>("void f()");

				auto vrender_buffer = vm->set_pod<layer::render_buffer>("render_buffer");
				vrender_buffer->set_property<layer::render_buffer>("matrix4x4 transform", &layer::render_buffer::transform);
				vrender_buffer->set_property<layer::render_buffer>("matrix4x4 world", &layer::render_buffer::world);
				vrender_buffer->set_property<layer::render_buffer>("vector4 texcoord", &layer::render_buffer::texcoord);
				vrender_buffer->set_property<layer::render_buffer>("float diffuse", &layer::render_buffer::diffuse);
				vrender_buffer->set_property<layer::render_buffer>("float normal", &layer::render_buffer::normal);
				vrender_buffer->set_property<layer::render_buffer>("float height", &layer::render_buffer::height);
				vrender_buffer->set_property<layer::render_buffer>("float material_id", &layer::render_buffer::material_id);
				vrender_buffer->set_constructor<layer::render_buffer>("void f()");

				auto vview_buffer = vm->set_pod<layer::view_buffer>("view_buffer");
				vview_buffer->set_property<layer::view_buffer>("matrix4x4 inv_view_proj", &layer::view_buffer::inv_view_proj);
				vview_buffer->set_property<layer::view_buffer>("matrix4x4 view_proj", &layer::view_buffer::view_proj);
				vview_buffer->set_property<layer::view_buffer>("matrix4x4 proj", &layer::view_buffer::proj);
				vview_buffer->set_property<layer::view_buffer>("matrix4x4 view", &layer::view_buffer::view);
				vview_buffer->set_property<layer::view_buffer>("vector3 position", &layer::view_buffer::position);
				vview_buffer->set_property<layer::view_buffer>("float far", &layer::view_buffer::far);
				vview_buffer->set_property<layer::view_buffer>("vector3 direction", &layer::view_buffer::direction);
				vview_buffer->set_property<layer::view_buffer>("float near", &layer::view_buffer::near);
				vview_buffer->set_constructor<layer::view_buffer>("void f()");

				auto vskin_model = vm->set_class<layer::skin_model>("skin_model", true);
				auto vpose_buffer = vm->set_struct_trivial<layer::pose_buffer>("pose_buffer");
				vpose_buffer->set_method_ex("void set_offset(int64, const pose_data &in)", &pose_buffer_set_offset);
				vpose_buffer->set_method_ex("void set_matrix(skin_mesh_buffer@+, usize, const matrix4x4 &in)", &pose_buffer_set_matrix);
				vpose_buffer->set_method_ex("pose_data& get_offset(int64)", &pose_buffer_get_offset);
				vpose_buffer->set_method_ex("matrix4x4& get_matrix(skin_mesh_buffer@+, usize)", &pose_buffer_get_matrix);
				vpose_buffer->set_method_ex("usize get_offsets_size()", &pose_buffer_get_offsets_size);
				vpose_buffer->set_method_ex("usize get_matrices_size(skin_mesh_buffer@+)", &pose_buffer_get_matrices_size);
				vpose_buffer->set_constructor<layer::pose_buffer>("void f()");

				auto vticker = vm->set_struct_trivial<layer::ticker>("clock_ticker");
				vticker->set_property<layer::ticker>("float delay", &layer::ticker::delay);
				vticker->set_constructor<layer::ticker>("void f()");
				vticker->set_method("bool tick_event(float)", &layer::ticker::tick_event);
				vticker->set_method("float get_time() const", &layer::ticker::get_time);

				auto vevent = vm->set_struct_trivial<layer::event>("scene_event");
				vevent->set_property<layer::event>("string name", &layer::event::name);
				vevent->set_constructor<layer::event, const std::string_view&>("void f(const string_view&in)");
				vevent->set_method_ex("void set_args(dictionary@+)", &event_set_args);
				vevent->set_method_ex("dictionary@ get_args() const", &event_get_args);

				auto vmaterial = vm->set_class<layer::material>("material", true);
				auto vbatch_data = vm->set_struct_trivial<layer::batch_data>("batch_data");
				vbatch_data->set_property<layer::batch_data>("element_buffer@ instances_buffer", &layer::batch_data::instance_buffer);
				vbatch_data->set_property<layer::batch_data>("uptr@ geometry_buffer", &layer::batch_data::geometry_buffer);
				vbatch_data->set_property<layer::batch_data>("material@ batch_material", &layer::batch_data::batch_material);
				vbatch_data->set_property<layer::batch_data>("usize instances_count", &layer::batch_data::instances_count);
				vbatch_data->set_constructor<layer::batch_data>("void f()");

				auto vvisibility_query = vm->set_pod<layer::visibility_query>("scene_visibility_query");
				vvisibility_query->set_property<layer::visibility_query>("geo_category category", &layer::visibility_query::category);
				vvisibility_query->set_property<layer::visibility_query>("bool boundary_visible", &layer::visibility_query::boundary_visible);
				vvisibility_query->set_property<layer::visibility_query>("bool query_pixels", &layer::visibility_query::query_pixels);
				vvisibility_query->set_constructor<layer::visibility_query>("void f()");

				auto vanimator_state = vm->set_pod<layer::animator_state>("animator_state");
				vanimator_state->set_property<layer::animator_state>("bool paused", &layer::animator_state::paused);
				vanimator_state->set_property<layer::animator_state>("bool looped", &layer::animator_state::looped);
				vanimator_state->set_property<layer::animator_state>("bool blended", &layer::animator_state::blended);
				vanimator_state->set_property<layer::animator_state>("float duration", &layer::animator_state::duration);
				vanimator_state->set_property<layer::animator_state>("float rate", &layer::animator_state::rate);
				vanimator_state->set_property<layer::animator_state>("float time", &layer::animator_state::time);
				vanimator_state->set_property<layer::animator_state>("int64 frame", &layer::animator_state::frame);
				vanimator_state->set_property<layer::animator_state>("int64 clip", &layer::animator_state::clip);
				vanimator_state->set_constructor<layer::animator_state>("void f()");
				vanimator_state->set_method("float get_timeline(clock_timer@+) const", &layer::animator_state::get_timeline);
				vanimator_state->set_method("float get_seconds_duration() const", &layer::animator_state::get_seconds_duration);
				vanimator_state->set_method("float get_progress() const", &layer::animator_state::get_progress);
				vanimator_state->set_method("bool is_playing() const", &layer::animator_state::is_playing);

				auto vspawner_properties = vm->set_pod<layer::spawner_properties>("spawner_properties");
				vspawner_properties->set_property<layer::spawner_properties>("random_vector4 diffusion", &layer::spawner_properties::diffusion);
				vspawner_properties->set_property<layer::spawner_properties>("random_vector3 position", &layer::spawner_properties::position);
				vspawner_properties->set_property<layer::spawner_properties>("random_vector3 velocity", &layer::spawner_properties::velocity);
				vspawner_properties->set_property<layer::spawner_properties>("random_vector3 noise", &layer::spawner_properties::noise);
				vspawner_properties->set_property<layer::spawner_properties>("random_float rotation", &layer::spawner_properties::rotation);
				vspawner_properties->set_property<layer::spawner_properties>("random_float scale", &layer::spawner_properties::scale);
				vspawner_properties->set_property<layer::spawner_properties>("random_float angular", &layer::spawner_properties::angular);
				vspawner_properties->set_property<layer::spawner_properties>("int32 iterations", &layer::spawner_properties::iterations);
				vspawner_properties->set_constructor<layer::spawner_properties>("void f()");

				auto vrender_constants = vm->set_class<layer::render_constants>("render_constants", false);
				vrender_constants->set_property<layer::render_constants>("animation_buffer animation", &layer::render_constants::animation);
				vrender_constants->set_property<layer::render_constants>("render_buffer render", &layer::render_constants::render);
				vrender_constants->set_property<layer::render_constants>("view_buffer view", &layer::render_constants::view);
				vrender_constants->set_constructor<layer::render_constants, graphics::graphics_device*>("render_constants@ f()");
				vrender_constants->set_method("void set_constant_buffer(render_buffer_type, uint32, uint32)", &layer::render_constants::set_constant_buffer);
				vrender_constants->set_method("void set_updated_constant_buffer(render_buffer_type, uint32, uint32)", &layer::render_constants::set_updated_constant_buffer);
				vrender_constants->set_method("void update_constant_buffer(render_buffer_type)", &layer::render_constants::update_constant_buffer);
				vrender_constants->set_method("shader@+ get_basic_effect() const", &layer::render_constants::get_basic_effect);
				vrender_constants->set_method("graphics_device@+ get_device() const", &layer::render_constants::get_device);
				vrender_constants->set_method("element_buffer@+ get_constant_buffer(render_buffer_type) const", &layer::render_constants::get_constant_buffer);

				auto vrender_system = vm->set_class<layer::render_system>("render_system", true);
				auto vviewer = vm->set_struct<layer::viewer>("viewer_t");
				vviewer->set_property<layer::viewer>("render_system@ renderer", &layer::viewer::renderer);
				vviewer->set_property<layer::viewer>("render_culling culling", &layer::viewer::culling);
				vviewer->set_property<layer::viewer>("matrix4x4 inv_view_projection", &layer::viewer::inv_view_projection);
				vviewer->set_property<layer::viewer>("matrix4x4 view_projection", &layer::viewer::view_projection);
				vviewer->set_property<layer::viewer>("matrix4x4 projection", &layer::viewer::projection);
				vviewer->set_property<layer::viewer>("matrix4x4 view", &layer::viewer::view);
				vviewer->set_property<layer::viewer>("vector3 inv_position", &layer::viewer::inv_position);
				vviewer->set_property<layer::viewer>("vector3 position", &layer::viewer::position);
				vviewer->set_property<layer::viewer>("vector3 rotation", &layer::viewer::rotation);
				vviewer->set_property<layer::viewer>("float far_plane", &layer::viewer::far_plane);
				vviewer->set_property<layer::viewer>("float near_plane", &layer::viewer::near_plane);
				vviewer->set_property<layer::viewer>("float ratio", &layer::viewer::ratio);
				vviewer->set_property<layer::viewer>("float fov", &layer::viewer::fov);
				vviewer->set_constructor<layer::viewer>("void f()");
				vviewer->set_operator_copy_static(&viewer_copy);
				vviewer->set_destructor_ex("void f()", &viewer_destructor);
				vviewer->set_method<layer::viewer, void, const trigonometry::matrix4x4&, const trigonometry::matrix4x4&, const trigonometry::vector3&, float, float, float, float, layer::render_culling>("void set(const matrix4x4 &in, const matrix4x4 &in, const vector3 &in, float, float, float, float, render_culling)", &layer::viewer::set);
				vviewer->set_method<layer::viewer, void, const trigonometry::matrix4x4&, const trigonometry::matrix4x4&, const trigonometry::vector3&, const trigonometry::vector3&, float, float, float, float, layer::render_culling>("void set(const matrix4x4 &in, const matrix4x4 &in, const vector3 &in, const vector3 &in, float, float, float, float, render_culling)", &layer::viewer::set);

				auto vattenuation = vm->set_pod<layer::attenuation>("attenuation");
				vattenuation->set_property<layer::attenuation>("float radius", &layer::attenuation::radius);
				vattenuation->set_property<layer::attenuation>("float c1", &layer::attenuation::C1);
				vattenuation->set_property<layer::attenuation>("float c2", &layer::attenuation::C2);
				vattenuation->set_constructor<layer::attenuation>("void f()");

				auto vsubsurface = vm->set_pod<layer::subsurface>("subsurface");
				vsubsurface->set_property<layer::subsurface>("vector4 emission", &layer::subsurface::emission);
				vsubsurface->set_property<layer::subsurface>("vector4 metallic", &layer::subsurface::metallic);
				vsubsurface->set_property<layer::subsurface>("vector4 penetration", &layer::subsurface::penetration);
				vsubsurface->set_property<layer::subsurface>("vector3 diffuse", &layer::subsurface::diffuse);
				vsubsurface->set_property<layer::subsurface>("float fresnel", &layer::subsurface::fresnel);
				vsubsurface->set_property<layer::subsurface>("vector3 scattering", &layer::subsurface::scattering);
				vsubsurface->set_property<layer::subsurface>("float transparency", &layer::subsurface::transparency);
				vsubsurface->set_property<layer::subsurface>("vector3 padding", &layer::subsurface::padding);
				vsubsurface->set_property<layer::subsurface>("float bias", &layer::subsurface::bias);
				vsubsurface->set_property<layer::subsurface>("vector2 roughness", &layer::subsurface::roughness);
				vsubsurface->set_property<layer::subsurface>("float refraction", &layer::subsurface::refraction);
				vsubsurface->set_property<layer::subsurface>("float environment", &layer::subsurface::environment);
				vsubsurface->set_property<layer::subsurface>("vector2 occlusion", &layer::subsurface::occlusion);
				vsubsurface->set_property<layer::subsurface>("float radius", &layer::subsurface::radius);
				vsubsurface->set_property<layer::subsurface>("float height", &layer::subsurface::height);
				vsubsurface->set_constructor<layer::subsurface>("void f()");

				auto vskin_animation = vm->set_class<layer::skin_animation>("skin_animation", false);
				vskin_animation->set_method_ex("array<skin_animator_clip>@+ get_clips() const", &skin_animation_get_clips);
				vskin_animation->set_method("bool is_valid() const", &layer::skin_animation::is_valid);

				auto vscene_graph = vm->set_class<layer::scene_graph>("scene_graph", true);
				vmaterial->set_property<layer::material>("subsurface surface", &layer::material::surface);
				vmaterial->set_property<layer::material>("usize slot", &layer::material::slot);
				vmaterial->set_gc_constructor<layer::material, material, layer::scene_graph*>("material@ f(scene_graph@+)");
				vmaterial->set_method("void set_name(const string_view&in)", &layer::material::set_name);
				vmaterial->set_method("const string& get_name(const string_view&in)", &layer::material::get_name);
				vmaterial->set_method("void set_diffuse_map(texture_2d@+)", &layer::material::set_diffuse_map);
				vmaterial->set_method("texture_2d@+ get_diffuse_map() const", &layer::material::get_diffuse_map);
				vmaterial->set_method("void set_normal_map(texture_2d@+)", &layer::material::set_normal_map);
				vmaterial->set_method("texture_2d@+ get_normal_map() const", &layer::material::get_normal_map);
				vmaterial->set_method("void set_metallic_map(texture_2d@+)", &layer::material::set_metallic_map);
				vmaterial->set_method("texture_2d@+ get_metallic_map() const", &layer::material::get_metallic_map);
				vmaterial->set_method("void set_roughness_map(texture_2d@+)", &layer::material::set_roughness_map);
				vmaterial->set_method("texture_2d@+ get_roughness_map() const", &layer::material::get_roughness_map);
				vmaterial->set_method("void set_height_map(texture_2d@+)", &layer::material::set_height_map);
				vmaterial->set_method("texture_2d@+ get_height_map() const", &layer::material::get_height_map);
				vmaterial->set_method("void set_occlusion_map(texture_2d@+)", &layer::material::set_occlusion_map);
				vmaterial->set_method("texture_2d@+ get_occlusion_map() const", &layer::material::get_occlusion_map);
				vmaterial->set_method("void set_emission_map(texture_2d@+)", &layer::material::set_emission_map);
				vmaterial->set_method("texture_2d@+ get_emission_map() const", &layer::material::get_emission_map);
				vmaterial->set_method("scene_graph@+ get_scene() const", &layer::material::get_scene);
				vmaterial->set_enum_refs_ex<layer::material>([](layer::material* base, asIScriptEngine* vm)
				{
					function_factory::gc_enum_callback(vm, base->get_diffuse_map());
					function_factory::gc_enum_callback(vm, base->get_normal_map());
					function_factory::gc_enum_callback(vm, base->get_metallic_map());
					function_factory::gc_enum_callback(vm, base->get_roughness_map());
					function_factory::gc_enum_callback(vm, base->get_height_map());
					function_factory::gc_enum_callback(vm, base->get_emission_map());
				});
				vmaterial->set_release_refs_ex<layer::material>([](layer::material* base, asIScriptEngine*)
				{
					base->~material();
				});

				auto vcomponent = vm->set_class<layer::component>("base_component", false);
				auto vsparse_index = vm->set_struct_trivial<layer::sparse_index>("sparse_index");
				vsparse_index->set_property<layer::sparse_index>("cosmos index", &layer::sparse_index::index);
				vsparse_index->set_method_ex("usize size() const", &sparse_index_get_size);
				vsparse_index->set_operator_ex(operators::index, (uint32_t)position::left, "base_component@+", "usize", &sparse_index_get_data);
				vsparse_index->set_constructor<layer::sparse_index>("void f()");

				vm->begin_namespace("content_heavy_series");
				vm->set_function<void(core::schema*, const trigonometry::vector2&)>("void pack(schema@+, const vector2 &in)", &layer::heavy_series::pack);
				vm->set_function<void(core::schema*, const trigonometry::vector3&)>("void pack(schema@+, const vector3 &in)", &layer::heavy_series::pack);
				vm->set_function<void(core::schema*, const trigonometry::vector4&)>("void pack(schema@+, const vector4 &in)", &layer::heavy_series::pack);
				vm->set_function<void(core::schema*, const trigonometry::vector4&)>("void pack(schema@+, const quaternion &in)", &layer::heavy_series::pack);
				vm->set_function<void(core::schema*, const trigonometry::matrix4x4&)>("void pack(schema@+, const matrix4x4 &in)", &layer::heavy_series::pack);
				vm->set_function<void(core::schema*, const layer::attenuation&)>("void pack(schema@+, const attenuation &in)", &layer::heavy_series::pack);
				vm->set_function<void(core::schema*, const layer::animator_state&)>("void pack(schema@+, const animator_state &in)", &layer::heavy_series::pack);
				vm->set_function<void(core::schema*, const layer::spawner_properties&)>("void pack(schema@+, const spawner_properties &in)", &layer::heavy_series::pack);
				vm->set_function<void(core::schema*, const trigonometry::skin_animator_key&)>("void pack(schema@+, const skin_animator_key &in)", &layer::heavy_series::pack);
				vm->set_function<void(core::schema*, const trigonometry::key_animator_clip&)>("void pack(schema@+, const key_animator_clip &in)", &layer::heavy_series::pack);
				vm->set_function<void(core::schema*, const trigonometry::animator_key&)>("void pack(schema@+, const animator_key &in)", &layer::heavy_series::pack);
				vm->set_function<void(core::schema*, const trigonometry::element_vertex&)>("void pack(schema@+, const element_vertex &in)", &layer::heavy_series::pack);
				vm->set_function<void(core::schema*, const trigonometry::joint&)>("void pack(schema@+, const joint &in)", &layer::heavy_series::pack);
				vm->set_function<void(core::schema*, const trigonometry::vertex&)>("void pack(schema@+, const vertex &in)", &layer::heavy_series::pack);
				vm->set_function<void(core::schema*, const trigonometry::skin_vertex&)>("void pack(schema@+, const skin_vertex &in)", &layer::heavy_series::pack);
				vm->set_function<void(core::schema*, const layer::ticker&)>("void pack(schema@+, const clock_ticker &in)", &layer::heavy_series::pack);
				vm->set_function<bool(core::schema*, trigonometry::vector2*)>("bool unpack(schema@+, vector2 &out)", &layer::heavy_series::unpack);
				vm->set_function<bool(core::schema*, trigonometry::vector3*)>("bool unpack(schema@+, vector3 &out)", &layer::heavy_series::unpack);
				vm->set_function<bool(core::schema*, trigonometry::vector4*)>("bool unpack(schema@+, vector4 &out)", &layer::heavy_series::unpack);
				vm->set_function<bool(core::schema*, trigonometry::vector4*)>("bool unpack(schema@+, quaternion &out)", &layer::heavy_series::unpack);
				vm->set_function<bool(core::schema*, trigonometry::matrix4x4*)>("bool unpack(schema@+, matrix4x4 &out)", &layer::heavy_series::unpack);
				vm->set_function<bool(core::schema*, layer::attenuation*)>("bool unpack(schema@+, attenuation &out)", &layer::heavy_series::unpack);
				vm->set_function<bool(core::schema*, layer::animator_state*)>("bool unpack(schema@+, animator_state &out)", &layer::heavy_series::unpack);
				vm->set_function<bool(core::schema*, layer::spawner_properties*)>("bool unpack(schema@+, spawner_properties &out)", &layer::heavy_series::unpack);
				vm->set_function<bool(core::schema*, trigonometry::skin_animator_key*)>("bool unpack(schema@+, skin_animator_key &out)", &layer::heavy_series::unpack);
				vm->set_function<bool(core::schema*, trigonometry::key_animator_clip*)>("bool unpack(schema@+, key_animator_clip &out)", &layer::heavy_series::unpack);
				vm->set_function<bool(core::schema*, trigonometry::animator_key*)>("bool unpack(schema@+, animator_key &out)", &layer::heavy_series::unpack);
				vm->set_function<bool(core::schema*, trigonometry::element_vertex*)>("bool unpack(schema@+, element_vertex &out)", &layer::heavy_series::unpack);
				vm->set_function<bool(core::schema*, trigonometry::joint*)>("bool unpack(schema@+, joint &out)", &layer::heavy_series::unpack);
				vm->set_function<bool(core::schema*, trigonometry::vertex*)>("bool unpack(schema@+, vertex &out)", &layer::heavy_series::unpack);
				vm->set_function<bool(core::schema*, trigonometry::skin_vertex*)>("bool unpack(schema@+, skin_vertex &out)", &layer::heavy_series::unpack);
				vm->set_function<bool(core::schema*, layer::ticker*)>("bool unpack(schema@+, clock_ticker &out)", &layer::heavy_series::unpack);
				vm->end_namespace();

				auto vmodel = vm->set_class<layer::model>("model", true);
				vmodel->set_property<layer::model>("vector4 max", &layer::model::max);
				vmodel->set_property<layer::model>("vector4 min", &layer::model::min);
				vmodel->set_gc_constructor<layer::model, model>("model@ f()");
				vmodel->set_method("mesh_buffer@+ find_mesh(const string_view&in) const", &layer::model::find_mesh);
				vmodel->set_method_ex("array<mesh_buffer@>@ get_meshes() const", &model_get_meshes);
				vmodel->set_method_ex("void set_meshes(array<mesh_buffer@>@+)", &model_set_meshes);
				vmodel->set_enum_refs_ex<layer::model>([](layer::model* base, asIScriptEngine* vm)
				{
					for (auto* item : base->meshes)
						function_factory::gc_enum_callback(vm, item);
				});
				vmodel->set_release_refs_ex<layer::model>([](layer::model* base, asIScriptEngine*)
				{
					base->cleanup();
				});

				vskin_model->set_property<layer::skin_model>("joint skeleton", &layer::skin_model::skeleton);
				vskin_model->set_property<layer::skin_model>("matrix4x4 inv_transform", &layer::skin_model::inv_transform);
				vskin_model->set_property<layer::skin_model>("matrix4x4 base_transform", &layer::skin_model::transform);
				vskin_model->set_property<layer::skin_model>("vector4 max", &layer::skin_model::max);
				vskin_model->set_property<layer::skin_model>("vector4 min", &layer::skin_model::min);
				vskin_model->set_gc_constructor<layer::skin_model, skin_model>("skin_model@ f()");
				vskin_model->set_method<layer::skin_model, bool, const std::string_view&, trigonometry::joint*>("bool find_joint(const string_view&in, joint &out) const", &layer::skin_model::find_joint);
				vskin_model->set_method<layer::skin_model, bool, size_t, trigonometry::joint*>("bool find_joint(usize, joint &out) const", &layer::skin_model::find_joint);
				vskin_model->set_method("skin_mesh_buffer@+ find_mesh(const string_view&in) const", &layer::skin_model::find_mesh);
				vskin_model->set_method_ex("array<skin_mesh_buffer@>@ get_meshes() const", &skin_model_get_meshes);
				vskin_model->set_method_ex("void set_meshes(array<skin_mesh_buffer@>@+)", &skin_model_set_meshes);
				vskin_model->set_enum_refs_ex<layer::skin_model>([](layer::skin_model* base, asIScriptEngine* vm)
				{
					for (auto* item : base->meshes)
						function_factory::gc_enum_callback(vm, item);
				});
				vskin_model->set_release_refs_ex<layer::skin_model>([](layer::skin_model* base, asIScriptEngine*)
				{
					base->cleanup();
				});

				auto ventity = vm->set_class<layer::entity>("scene_entity", true);
				populate_component_base<layer::component>(*vcomponent);

				ventity->set_method("void set_name(const string_view&in)", &layer::entity::set_name);
				ventity->set_method("void set_root(scene_entity@+)", &layer::entity::set_root);
				ventity->set_method("void update_bounds()", &layer::entity::update_bounds);
				ventity->set_method("void remove_childs()", &layer::entity::remove_childs);
				ventity->set_method_ex("void remove_component(base_component@+)", &entity_remove_component);
				ventity->set_method_ex("void remove_component(uint64)", &entity_remove_component_by_id);
				ventity->set_method_ex("base_component@+ add_component(base_component@+)", &entity_add_component);
				ventity->set_method_ex("base_component@+ get_component(uint64) const", &entity_get_component_by_id);
				ventity->set_method_ex("array<base_component@>@ get_components() const", &entity_get_components);
				ventity->set_method("scene_graph@+ get_scene() const", &layer::entity::get_scene);
				ventity->set_method("scene_entity@+ get_parent() const", &layer::entity::get_parent);
				ventity->set_method("scene_entity@+ get_child(usize) const", &layer::entity::get_child);
				ventity->set_method("transform@+ get_transform() const", &layer::entity::get_transform);
				ventity->set_method("const matrix4x4& get_box() const", &layer::entity::get_box);
				ventity->set_method("const vector3& get_min() const", &layer::entity::get_min);
				ventity->set_method("const vector3& get_max() const", &layer::entity::get_max);
				ventity->set_method("const string& get_name() const", &layer::entity::get_name);
				ventity->set_method("usize get_components_count() const", &layer::entity::get_components_count);
				ventity->set_method("usize get_childs_count() const", &layer::entity::get_components_count);
				ventity->set_method("float get_visibility(const viewer_t &in) const", &layer::entity::get_visibility);
				ventity->set_method("float is_active() const", &layer::entity::is_active);
				ventity->set_method("vector3 get_radius3() const", &layer::entity::get_radius3);
				ventity->set_method("float get_radius() const", &layer::entity::get_radius);
				ventity->set_enum_refs_ex<layer::entity>([](layer::entity* base, asIScriptEngine* vm)
				{
					for (auto& item : *base)
						function_factory::gc_enum_callback(vm, item.second);
				});
				ventity->set_release_refs_ex<layer::entity>([](layer::entity* base, asIScriptEngine*) { });

				auto vdrawable = vm->set_class<layer::drawable>("drawable_component", false);
				populate_drawable_base<layer::drawable>(*vdrawable);

				auto vrenderer = vm->set_class<layer::renderer>("base_renderer", false);
				populate_renderer_base<layer::renderer>(*vrenderer);

				auto vrs_state = vm->set_pod<layer::render_system::rs_state>("rs_state");
				vrs_state->set_method("bool is_state(render_state) const", &layer::render_system::rs_state::is);
				vrs_state->set_method("bool is_set(render_opt) const", &layer::render_system::rs_state::is_set);
				vrs_state->set_method("bool is_top() const", &layer::render_system::rs_state::is_top);
				vrs_state->set_method("bool is_subpass() const", &layer::render_system::rs_state::is_subpass);
				vrs_state->set_method("render_opt get_opts() const", &layer::render_system::rs_state::get_opts);
				vrs_state->set_method("render_state get_state() const", &layer::render_system::rs_state::get);

				auto vprimitive_cache = vm->set_class<layer::renderer>("primitive_cache", true);
				vrender_system->set_function_def("void overlapping_result_sync(base_component@+)");
				vrender_system->set_property<layer::render_system>("rs_state state", &layer::render_system::state);
				vrender_system->set_property<layer::render_system>("viewer_t view", &layer::render_system::view);
				vrender_system->set_property<layer::render_system>("usize max_queries", &layer::render_system::max_queries);
				vrender_system->set_property<layer::render_system>("usize sorting_frequency", &layer::render_system::sorting_frequency);
				vrender_system->set_property<layer::render_system>("usize occlusion_skips", &layer::render_system::occlusion_skips);
				vrender_system->set_property<layer::render_system>("usize occluder_skips", &layer::render_system::occluder_skips);
				vrender_system->set_property<layer::render_system>("usize occludee_skips", &layer::render_system::occludee_skips);
				vrender_system->set_property<layer::render_system>("float overflow_visibility", &layer::render_system::overflow_visibility);
				vrender_system->set_property<layer::render_system>("float threshold", &layer::render_system::threshold);
				vrender_system->set_property<layer::render_system>("bool occlusion_culling", &layer::render_system::occlusion_culling);
				vrender_system->set_property<layer::render_system>("bool precise_culling", &layer::render_system::precise_culling);
				vrender_system->set_property<layer::render_system>("bool allow_input_lag", &layer::render_system::allow_input_lag);
				vrender_system->set_gc_constructor<layer::render_system, render_system, layer::scene_graph*, layer::component*>("render_system@ f(scene_graph@+, base_component@+)");
				vrender_system->set_method("void set_view(const matrix4x4 &in, const matrix4x4 &in, const vector3 &in, float, float, float, float, render_culling)", &layer::render_system::set_view);
				vrender_system->set_method("void clear_culling()", &layer::render_system::clear_culling);
				vrender_system->set_method_ex("void restore_view_buffer()", &render_system_restore_view_buffer);
				vrender_system->set_method("void restore_view_buffer(viewer_t &out)", &layer::render_system::restore_view_buffer);
				vrender_system->set_method<layer::render_system, void, layer::renderer*>("void remount(base_renderer@+)", &layer::render_system::remount);
				vrender_system->set_method<layer::render_system, void>("void remount()", &layer::render_system::remount);
				vrender_system->set_method("void mount()", &layer::render_system::mount);
				vrender_system->set_method("void unmount()", &layer::render_system::unmount);
				vrender_system->set_method("void move_renderer(uint64, usize)", &layer::render_system::move_renderer);
				vrender_system->set_method<layer::render_system, void, uint64_t>("void remove_renderer(uint64)", &layer::render_system::remove_renderer);
				vrender_system->set_method_ex("void move_renderer(base_renderer@+, usize)", &render_system_move_renderer);
				vrender_system->set_method_ex("void remove_renderer(base_renderer@+, usize)", &render_system_remove_renderer);
				vrender_system->set_method("void restore_output()", &layer::render_system::restore_output);
				vrender_system->set_method<layer::render_system, void, const std::string_view&, graphics::shader*>("void free_shader(const string_view&in, shader@+)", &layer::render_system::free_shader);
				vrender_system->set_method<layer::render_system, void, graphics::shader*>("void free_shader(shader@+)", &layer::render_system::free_shader);
				vrender_system->set_method_ex("void free_buffers(const string_view&in, element_buffer@+, element_buffer@+)", &render_system_free_buffers1);
				vrender_system->set_method_ex("void free_buffers(element_buffer@+, element_buffer@+)", &render_system_free_buffers2);
				vrender_system->set_method("void update_constant_buffer(render_buffer_type)", &layer::render_system::update_constant_buffer);
				vrender_system->set_method("void clear_materials()", &layer::render_system::clear_materials);
				vrender_system->set_method("void fetch_visibility(base_component@+, scene_visibility_query &out)", &layer::render_system::fetch_visibility);
				vrender_system->set_method("usize render(clock_timer@+, render_state, render_opt)", &layer::render_system::render);
				vrender_system->set_method("bool try_instance(material@+, render_buffer_instance &out)", &layer::render_system::try_instance);
				vrender_system->set_method("bool try_geometry(material@+, bool)", &layer::render_system::try_geometry);
				vrender_system->set_method("bool has_category(geo_category)", &layer::render_system::has_category);
				vrender_system->set_method_ex("shader@+ compile_shader(shader_desc &in, usize = 0)", &render_system_compile_shader1);
				vrender_system->set_method_ex("shader@+ compile_shader(const string_view&in, array<string>@+, usize = 0)", &render_system_compile_shader2);
				vrender_system->set_method_ex("array<element_buffer@>@ compile_buffers(const string_view&in, usize, usize)", &render_system_compile_buffers);
				vrender_system->set_method_ex("bool add_renderer(base_renderer@+)", &render_system_add_renderer);
				vrender_system->set_method_ex("base_renderer@+ get_renderer(uint64) const", &render_system_get_renderer);
				vrender_system->set_method_ex("base_renderer@+ get_renderer_by_index(usize) const", &render_system_get_renderer_by_index);
				vrender_system->set_method_ex("usize get_renderers_count() const", &render_system_get_renderers_count);
				vrender_system->set_method<layer::render_system, bool, uint64_t, size_t&>("bool get_offset(uint64, usize &out) const", &layer::render_system::get_offset);
				vrender_system->set_method("multi_render_target_2d@+ get_mrt(target_type)", &layer::render_system::get_mrt);
				vrender_system->set_method("render_target_2d@+ get_rt(target_type)", &layer::render_system::get_rt);
				vrender_system->set_method("graphics_device@+ get_device()", &layer::render_system::get_device);
				vrender_system->set_method("primitive_cache@+ get_primitives()", &layer::render_system::get_primitives);
				vrender_system->set_method("render_constants@+ get_constants()", &layer::render_system::get_constants);
				vrender_system->set_method("shader@+ get_basic_effect()", &layer::render_system::get_basic_effect);
				vrender_system->set_method("scene_graph@+ get_scene()", &layer::render_system::get_scene);
				vrender_system->set_method("base_component@+ get_component()", &layer::render_system::get_component);
				vrender_system->set_method_ex("void query_group(uint64, overlapping_result_sync@)", &render_system_query_group);
				vrender_system->set_enum_refs_ex<layer::render_system>([](layer::render_system* base, asIScriptEngine* vm)
				{
					for (auto* item : base->get_renderers())
						function_factory::gc_enum_callback(vm, item);
				});
				vrender_system->set_release_refs_ex<layer::render_system>([](layer::render_system* base, asIScriptEngine*)
				{
					base->remove_renderers();
				});

				auto vshader_cache = vm->set_class<layer::shader_cache>("shader_cache", true);
				vshader_cache->set_gc_constructor<layer::shader_cache, shader_cache, graphics::graphics_device*>("shader_cache@ f()");
				vshader_cache->set_method_ex("shader@+ compile(const string_view&in, const shader_desc &in, usize = 0)", &VI_EXPECTIFY(layer::shader_cache::compile));
				vshader_cache->set_method("shader@+ get(const string_view&in)", &layer::shader_cache::get);
				vshader_cache->set_method("string find(shader@+)", &layer::shader_cache::find);
				vshader_cache->set_method("bool has(const string_view&in)", &layer::shader_cache::has);
				vshader_cache->set_method("bool free(const string_view&in, shader@+ = null)", &layer::shader_cache::free);
				vshader_cache->set_method("void clear_cache()", &layer::shader_cache::clear_cache);
				vshader_cache->set_enum_refs_ex<layer::shader_cache>([](layer::shader_cache* base, asIScriptEngine* vm)
				{
					for (auto& item : base->get_caches())
						function_factory::gc_enum_callback(vm, item.second.shader);
				});
				vshader_cache->set_release_refs_ex<layer::shader_cache>([](layer::shader_cache* base, asIScriptEngine*)
				{
					base->clear_cache();
				});

				vprimitive_cache->set_gc_constructor<layer::primitive_cache, primitive_cache, graphics::graphics_device*>("primitive_cache@ f()");
				vprimitive_cache->set_method_ex("array<element_buffer@>@ compile(const string_view&in, usize, usize)", &primitive_cache_compile);
				vprimitive_cache->set_method_ex("array<element_buffer@>@ get(const string_view&in) const", &primitive_cache_get);
				vprimitive_cache->set_method("bool has(const string_view&in) const", &layer::primitive_cache::has);
				vprimitive_cache->set_method_ex("bool free(const string_view&in, element_buffer@+, element_buffer@+)", &primitive_cache_free);
				vprimitive_cache->set_method_ex("string find(element_buffer@+, element_buffer@+) const", &primitive_cache_find);
				vprimitive_cache->set_method("model@+ get_box_model() const", &layer::primitive_cache::get_box_model);
				vprimitive_cache->set_method("skin_model@+ get_skin_box_model() const", &layer::primitive_cache::get_skin_box_model);
				vprimitive_cache->set_method("element_buffer@+ get_quad() const", &layer::primitive_cache::get_quad);
				vprimitive_cache->set_method("element_buffer@+ get_sphere(buffer_type) const", &layer::primitive_cache::get_sphere);
				vprimitive_cache->set_method("element_buffer@+ get_cube(buffer_type) const", &layer::primitive_cache::get_cube);
				vprimitive_cache->set_method("element_buffer@+ get_box(buffer_type) const", &layer::primitive_cache::get_box);
				vprimitive_cache->set_method("element_buffer@+ get_skin_box(buffer_type) const", &layer::primitive_cache::get_skin_box);
				vprimitive_cache->set_method_ex("array<element_buffer@>@ get_sphere_buffers() const", &primitive_cache_get_sphere_buffers);
				vprimitive_cache->set_method_ex("array<element_buffer@>@ get_cube_buffers() const", &primitive_cache_get_cube_buffers);
				vprimitive_cache->set_method_ex("array<element_buffer@>@ get_box_buffers() const", &primitive_cache_get_box_buffers);
				vprimitive_cache->set_method_ex("array<element_buffer@>@ get_skin_box_buffers() const", &primitive_cache_get_skin_box_buffers);
				vprimitive_cache->set_method("void clear_cache()", &layer::primitive_cache::clear_cache);
				vprimitive_cache->set_enum_refs_ex<layer::primitive_cache>([](layer::primitive_cache* base, asIScriptEngine* vm)
				{
					function_factory::gc_enum_callback(vm, base->get_sphere(layer::buffer_type::vertex));
					function_factory::gc_enum_callback(vm, base->get_sphere(layer::buffer_type::index));
					function_factory::gc_enum_callback(vm, base->get_cube(layer::buffer_type::vertex));
					function_factory::gc_enum_callback(vm, base->get_cube(layer::buffer_type::index));
					function_factory::gc_enum_callback(vm, base->get_box(layer::buffer_type::vertex));
					function_factory::gc_enum_callback(vm, base->get_box(layer::buffer_type::index));
					function_factory::gc_enum_callback(vm, base->get_skin_box(layer::buffer_type::vertex));
					function_factory::gc_enum_callback(vm, base->get_skin_box(layer::buffer_type::index));
					function_factory::gc_enum_callback(vm, base->get_quad());

					for (auto& item : base->get_caches())
					{
						function_factory::gc_enum_callback(vm, item.second.buffers[0]);
						function_factory::gc_enum_callback(vm, item.second.buffers[1]);
					}
				});
				vprimitive_cache->set_release_refs_ex<layer::primitive_cache>([](layer::primitive_cache* base, asIScriptEngine*)
				{
					base->clear_cache();
				});

				auto vscene_graph_shared_desc = vm->set_struct<layer::scene_graph::desc>("scene_graph_shared_desc");
				vscene_graph_shared_desc->set_property<layer::scene_graph::desc::dependencies>("graphics_device@ device", &layer::scene_graph::desc::dependencies::device);
				vscene_graph_shared_desc->set_property<layer::scene_graph::desc::dependencies>("activity@ window", &layer::scene_graph::desc::dependencies::activity);
				vscene_graph_shared_desc->set_property<layer::scene_graph::desc::dependencies>("content_manager@ content", &layer::scene_graph::desc::dependencies::content);
				vscene_graph_shared_desc->set_property<layer::scene_graph::desc::dependencies>("primitive_cache@ primitives", &layer::scene_graph::desc::dependencies::primitives);
				vscene_graph_shared_desc->set_property<layer::scene_graph::desc::dependencies>("shader_cache@ shaders", &layer::scene_graph::desc::dependencies::shaders);
				vscene_graph_shared_desc->set_constructor<layer::scene_graph::desc::dependencies>("void f()");
				vscene_graph_shared_desc->set_operator_copy_static(&scene_graph_shared_desc_copy);
				vscene_graph_shared_desc->set_destructor_ex("void f()", &scene_graph_shared_desc_destructor);

				auto vheavy_application = vm->set_class<application>("heavy_application", true);
				auto vscene_graph_desc = vm->set_struct_trivial<layer::scene_graph::desc>("scene_graph_desc");
				vscene_graph_desc->set_property<layer::scene_graph::desc>("scene_graph_shared_desc shared", &layer::scene_graph::desc::shared);
				vscene_graph_desc->set_property<layer::scene_graph::desc>("physics_simulator_desc simulator", &layer::scene_graph::desc::simulator);
				vscene_graph_desc->set_property<layer::scene_graph::desc>("usize start_materials", &layer::scene_graph::desc::start_materials);
				vscene_graph_desc->set_property<layer::scene_graph::desc>("usize start_entities", &layer::scene_graph::desc::start_entities);
				vscene_graph_desc->set_property<layer::scene_graph::desc>("usize start_components", &layer::scene_graph::desc::start_components);
				vscene_graph_desc->set_property<layer::scene_graph::desc>("usize grow_margin", &layer::scene_graph::desc::grow_margin);
				vscene_graph_desc->set_property<layer::scene_graph::desc>("usize max_updates", &layer::scene_graph::desc::max_updates);
				vscene_graph_desc->set_property<layer::scene_graph::desc>("usize points_size", &layer::scene_graph::desc::points_size);
				vscene_graph_desc->set_property<layer::scene_graph::desc>("usize points_max", &layer::scene_graph::desc::points_max);
				vscene_graph_desc->set_property<layer::scene_graph::desc>("usize spots_size", &layer::scene_graph::desc::spots_size);
				vscene_graph_desc->set_property<layer::scene_graph::desc>("usize spots_max", &layer::scene_graph::desc::spots_max);
				vscene_graph_desc->set_property<layer::scene_graph::desc>("usize lines_size", &layer::scene_graph::desc::lines_size);
				vscene_graph_desc->set_property<layer::scene_graph::desc>("usize lines_max", &layer::scene_graph::desc::lines_max);
				vscene_graph_desc->set_property<layer::scene_graph::desc>("double grow_rate", &layer::scene_graph::desc::grow_rate);
				vscene_graph_desc->set_property<layer::scene_graph::desc>("float render_quality", &layer::scene_graph::desc::render_quality);
				vscene_graph_desc->set_property<layer::scene_graph::desc>("bool enable_hdr", &layer::scene_graph::desc::enable_hdr);
				vscene_graph_desc->set_property<layer::scene_graph::desc>("bool mutations", &layer::scene_graph::desc::mutations);
				vscene_graph_desc->set_constructor<layer::scene_graph::desc>("void f()");
				vscene_graph_desc->set_method_static("scene_graph_desc get(heavy_application@+)", &layer::scene_graph::desc::get);

				auto vscene_graph_statistics = vm->set_pod<layer::scene_graph::sg_statistics>("scene_graph_statistics");
				vscene_graph_statistics->set_property<layer::scene_graph::sg_statistics>("usize batching", &layer::scene_graph::sg_statistics::batching);
				vscene_graph_statistics->set_property<layer::scene_graph::sg_statistics>("usize sorting", &layer::scene_graph::sg_statistics::sorting);
				vscene_graph_statistics->set_property<layer::scene_graph::sg_statistics>("usize instances", &layer::scene_graph::sg_statistics::instances);
				vscene_graph_statistics->set_property<layer::scene_graph::sg_statistics>("usize draw_calls", &layer::scene_graph::sg_statistics::draw_calls);

				vscene_graph->set_gc_constructor<layer::scene_graph, scene_graph, const layer::scene_graph::desc&>("scene_graph@ f(const scene_graph_desc &in)");
				vscene_graph->set_function_def("bool ray_test_sync(base_component@+, const vector3 &in)");
				vscene_graph->set_function_def("void transaction_sync()");
				vscene_graph->set_function_def("void event_async(const string&in, schema@+)");
				vscene_graph->set_function_def("void match_sync(const bounding &in)");
				vscene_graph->set_function_def("void resource_async(uptr@)");
				vscene_graph->set_property("scene_graph_statistics statistics", &layer::scene_graph::statistics);
				vscene_graph->set_method("void configure(const scene_graph_desc &in)", &layer::scene_graph::configure);
				vscene_graph->set_method("void actualize()", &layer::scene_graph::actualize);
				vscene_graph->set_method("void resize_buffers()", &layer::scene_graph::resize_buffers);
				vscene_graph->set_method("void submit()", &layer::scene_graph::submit);
				vscene_graph->set_method("void dispatch(clock_timer@+)", &layer::scene_graph::dispatch);
				vscene_graph->set_method("void publish(clock_timer@+)", &layer::scene_graph::publish);
				vscene_graph->set_method("void publish_and_submit(clock_timer@+, float, float, float, bool)", &layer::scene_graph::publish_and_submit);
				vscene_graph->set_method("void delete_material(material@+)", &layer::scene_graph::delete_material);
				vscene_graph->set_method("void remove_entity(scene_entity@+)", &layer::scene_graph::remove_entity);
				vscene_graph->set_method("void delete_entity(scene_entity@+)", &layer::scene_graph::delete_entity);
				vscene_graph->set_method("void set_camera(scene_entity@+)", &layer::scene_graph::set_camera);
				vscene_graph->set_method_ex("void ray_test(uint64, const ray &in, ray_test_sync@)", &scene_graph_ray_test);
				vscene_graph->set_method("void script_hook(const string_view&in = \"main\")", &layer::scene_graph::script_hook);
				vscene_graph->set_method("void set_active(bool)", &layer::scene_graph::set_active);
				vscene_graph->set_method("void set_mrt(target_type, bool)", &layer::scene_graph::set_mrt);
				vscene_graph->set_method("void set_rt(target_type, bool)", &layer::scene_graph::set_rt);
				vscene_graph->set_method("void swap_mrt(target_type, multi_render_target_2d@+)", &layer::scene_graph::swap_mrt);
				vscene_graph->set_method("void swap_rt(target_type, render_target_2d@+)", &layer::scene_graph::swap_rt);
				vscene_graph->set_method("void clear_mrt(target_type, bool, bool)", &layer::scene_graph::clear_mrt);
				vscene_graph->set_method("void clear_rt(target_type, bool, bool)", &layer::scene_graph::clear_rt);
				vscene_graph->set_method_ex("void mutate(scene_entity@+, scene_entity@+, const string_view&in)", &scene_graph_mutate1);
				vscene_graph->set_method_ex("void mutate(scene_entity@+, const string_view&in)", &scene_graph_mutate2);
				vscene_graph->set_method_ex("void mutate(base_component@+, const string_view&in)", &scene_graph_mutate3);
				vscene_graph->set_method_ex("void mutate(material@+, const string_view&in)", &scene_graph_mutate4);
				vscene_graph->set_method_ex("void transaction(transaction_sync@)", &scene_graph_transaction);
				vscene_graph->set_method("void clear_culling()", &layer::scene_graph::clear_culling);
				vscene_graph->set_method("void reserve_materials(usize)", &layer::scene_graph::reserve_materials);
				vscene_graph->set_method("void reserve_entities(usize)", &layer::scene_graph::reserve_entities);
				vscene_graph->set_method("void reserve_components(uint64, usize)", &layer::scene_graph::reserve_components);
				vscene_graph->set_method_ex("bool push_event(const string_view&in, schema@+, bool)", &scene_graph_push_event1);
				vscene_graph->set_method_ex("bool push_event(const string_view&in, schema@+, base_component@+)", &scene_graph_push_event2);
				vscene_graph->set_method_ex("bool push_event(const string_view&in, schema@+, scene_entity@+)", &scene_graph_push_event3);
				vscene_graph->set_method_ex("uptr@ set_listener(const string_view&in, event_async@)", &scene_graph_set_listener);
				vscene_graph->set_method("bool clear_listener(const string_view&in, uptr@)", &layer::scene_graph::clear_listener);
				vscene_graph->set_method("material@+ get_invalid_material()", &layer::scene_graph::get_invalid_material);
				vscene_graph->set_method<layer::scene_graph, bool, layer::material*>("bool add_material(material@+)", &layer::scene_graph::add_material);
				vscene_graph->set_method<layer::scene_graph, layer::material*>("material@+ add_material()", &layer::scene_graph::add_material);
				vscene_graph->set_method_ex("void load_resource(uint64, base_component@+, const string_view&in, schema@+, resource_async@)", &scene_graph_load_resource);
				vscene_graph->set_method<layer::scene_graph, core::string, uint64_t, void*>("string find_resource_id(uint64, uptr@)", &layer::scene_graph::find_resource_id);
				vscene_graph->set_method("material@+ clone_material(material@+)", &layer::scene_graph::clone_material);
				vscene_graph->set_method("scene_entity@+ get_entity(usize) const", &layer::scene_graph::get_entity);
				vscene_graph->set_method("scene_entity@+ get_last_entity() const", &layer::scene_graph::get_last_entity);
				vscene_graph->set_method("scene_entity@+ get_camera_entity() const", &layer::scene_graph::get_camera_entity);
				vscene_graph->set_method("base_component@+ get_component(uint64, usize) const", &layer::scene_graph::get_component);
				vscene_graph->set_method("base_component@+ get_camera() const", &layer::scene_graph::get_camera);
				vscene_graph->set_method("render_system@+ get_renderer() const", &layer::scene_graph::get_renderer);
				vscene_graph->set_method("viewer_t get_camera_viewer() const", &layer::scene_graph::get_camera_viewer);
				vscene_graph->set_method<layer::scene_graph, layer::material*, const std::string_view&>("material@+ get_material(const string_view&in) const", &layer::scene_graph::get_material);
				vscene_graph->set_method<layer::scene_graph, layer::material*, size_t>("material@+ get_material(usize) const", &layer::scene_graph::get_material);
				vscene_graph->set_method<layer::scene_graph, layer::sparse_index&, uint64_t>("sparse_index& get_storage(uint64) const", &layer::scene_graph::get_storage);
				vscene_graph->set_method_ex("array<base_component@>@ get_components(uint64) const", &scene_graph_get_components);
				vscene_graph->set_method_ex("array<base_component@>@ get_actors(actor_type) const", &scene_graph_get_actors);
				vscene_graph->set_method("render_target_2d_desc get_desc_rt() const", &layer::scene_graph::get_desc_rt);
				vscene_graph->set_method("multi_render_target_2d_desc get_desc_mrt() const", &layer::scene_graph::get_desc_mrt);
				vscene_graph->set_method("surface_format get_format_mrt(uint32) const", &layer::scene_graph::get_format_mrt);
				vscene_graph->set_method_ex("array<scene_entity@>@ clone_entity_as_array(scene_entity@+)", &scene_graph_clone_entity_as_array);
				vscene_graph->set_method_ex("array<scene_entity@>@ query_by_parent(scene_entity@+) const", &scene_graph_query_by_parent);
				vscene_graph->set_method_ex("array<scene_entity@>@ query_by_name(const string_view&in) const", &scene_graph_query_by_name);
				vscene_graph->set_method_ex("array<base_component@>@ query_by_position(uint64, const vector3 &in, float) const", &scene_graph_query_by_position);
				vscene_graph->set_method_ex("array<base_component@>@ query_by_area(uint64, const vector3 &in, const vector3 &in) const", &scene_graph_query_by_area);
				vscene_graph->set_method_ex("array<base_component@>@ query_by_ray(uint64, const ray &in) const", &scene_graph_query_by_ray);
				vscene_graph->set_method_ex("array<base_component@>@ query_by_match(uint64, match_sync@) const", &scene_graph_query_by_match);
				vscene_graph->set_method("string as_resource_path(const string_view&in) const", &layer::scene_graph::as_resource_path);
				vscene_graph->set_method<layer::scene_graph, layer::entity*>("scene_entity@+ add_entity()", &layer::scene_graph::add_entity);
				vscene_graph->set_method("scene_entity@+ clone_entity(scene_entity@+)", &layer::scene_graph::clone_entity);
				vscene_graph->set_method<layer::scene_graph, bool, layer::entity*>("bool add_entity(scene_entity@+)", &layer::scene_graph::add_entity);
				vscene_graph->set_method("bool is_active() const", &layer::scene_graph::is_active);
				vscene_graph->set_method("bool is_left_handed() const", &layer::scene_graph::is_left_handed);
				vscene_graph->set_method("bool is_indexed() const", &layer::scene_graph::is_indexed);
				vscene_graph->set_method("bool is_busy() const", &layer::scene_graph::is_busy);
				vscene_graph->set_method("usize get_materials_count() const", &layer::scene_graph::get_materials_count);
				vscene_graph->set_method("usize get_entities_count() const", &layer::scene_graph::get_entities_count);
				vscene_graph->set_method("usize get_components_count(uint64) const", &layer::scene_graph::get_components_count);
				vscene_graph->set_method<layer::scene_graph, bool, layer::entity*>("bool has_entity(scene_entity@+) const", &layer::scene_graph::has_entity);
				vscene_graph->set_method<layer::scene_graph, bool, size_t>("bool has_entity(usize) const", &layer::scene_graph::has_entity);
				vscene_graph->set_method("multi_render_target_2d@+ get_mrt() const", &layer::scene_graph::get_mrt);
				vscene_graph->set_method("render_target_2d@+ get_rt() const", &layer::scene_graph::get_rt);
				vscene_graph->set_method("graphics_device@+ get_device() const", &layer::scene_graph::get_device);
				vscene_graph->set_method("physics_simulator@+ get_simulator() const", &layer::scene_graph::get_simulator);
				vscene_graph->set_method("activity@+ get_activity() const", &layer::scene_graph::get_activity);
				vscene_graph->set_method("render_constants@+ get_constants() const", &layer::scene_graph::get_constants);
				vscene_graph->set_method("shader_cache@+ get_shaders() const", &layer::scene_graph::get_shaders);
				vscene_graph->set_method("primitive_cache@+ get_primitives() const", &layer::scene_graph::get_primitives);
				vscene_graph->set_method("scene_graph_desc& get_conf()", &layer::scene_graph::get_conf);
				vscene_graph->set_enum_refs_ex<layer::scene_graph>([](layer::scene_graph* base, asIScriptEngine* vm)
				{
					auto& conf = base->get_conf();
					function_factory::gc_enum_callback(vm, conf.shared.shaders);
					function_factory::gc_enum_callback(vm, conf.shared.primitives);
					function_factory::gc_enum_callback(vm, conf.shared.content);
					function_factory::gc_enum_callback(vm, conf.shared.device);
					function_factory::gc_enum_callback(vm, conf.shared.activity);
					function_factory::gc_enum_callback(vm, conf.shared.vm);

					size_t materials = base->get_materials_count();
					for (size_t i = 0; i < materials; i++)
						function_factory::gc_enum_callback(vm, base->get_material(i));

					size_t entities = base->get_entities_count();
					for (size_t i = 0; i < entities; i++)
						function_factory::gc_enum_callback(vm, base->get_entity(i));

					for (auto& item : base->get_registry())
					{
						for (auto* next : item.second->data)
							function_factory::gc_enum_callback(vm, next);
					}
				});
				vscene_graph->set_release_refs_ex<layer::scene_graph>([](layer::scene_graph* base, asIScriptEngine*) { });

				auto vheavy_application_cache_info = vm->set_struct<heavy_application::cache_info>("heavy_application_cache_info");
				vheavy_application_cache_info->set_property<heavy_application::cache_info>("shader_cache@ shaders", &heavy_application::cache_info::shaders);
				vheavy_application_cache_info->set_property<heavy_application::cache_info>("primitive_cache@ primitives", &heavy_application::cache_info::primitives);
				vheavy_application_cache_info->set_constructor<heavy_application::cache_info>("void f()");
				vheavy_application_cache_info->set_operator_copy_static(&application_cache_info_copy);
				vheavy_application_cache_info->set_destructor_ex("void f()", &application_cache_info_destructor);

				auto vheavy_application_desc = vm->set_struct_trivial<heavy_application::desc>("heavy_application_desc");
				vheavy_application_desc->set_property<application::desc>("application_frame_info refreshrate", &application::desc::refreshrate);
				vheavy_application_desc->set_property<heavy_application::desc>("graphics_device_desc graphics", &heavy_application::desc::graphics_device);
				vheavy_application_desc->set_property<heavy_application::desc>("activity_desc window", &heavy_application::desc::activity);
				vheavy_application_desc->set_property<application::desc>("schedule_policy scheduler", &application::desc::scheduler);
				vheavy_application_desc->set_property<application::desc>("string preferences", &application::desc::preferences);
				vheavy_application_desc->set_property<application::desc>("string environment", &application::desc::environment);
				vheavy_application_desc->set_property<application::desc>("string directory", &application::desc::directory);
				vheavy_application_desc->set_property<application::desc>("usize polling_timeout", &application::desc::polling_timeout);
				vheavy_application_desc->set_property<application::desc>("usize polling_events", &application::desc::polling_events);
				vheavy_application_desc->set_property<application::desc>("usize threads", &application::desc::threads);
				vheavy_application_desc->set_property<application::desc>("usize usage", &application::desc::usage);
				vheavy_application_desc->set_property<heavy_application::desc>("usize advanced_usage", &heavy_application::desc::advanced_usage);
				vheavy_application_desc->set_property<heavy_application::desc>("bool blocking_dispatch", &heavy_application::desc::blocking_dispatch);
				vheavy_application_desc->set_property<application::desc>("bool daemon", &application::desc::daemon);
				vheavy_application_desc->set_property<heavy_application::desc>("bool cursor", &heavy_application::desc::cursor);
				vheavy_application_desc->set_constructor<heavy_application::desc>("void f()");

				vheavy_application->set_function_def("void key_event_sync(key_code, key_mod, int32, int32, bool)");
				vheavy_application->set_function_def("void input_event_sync(const string_view&in)");
				vheavy_application->set_function_def("void wheel_event_sync(int, int, bool)");
				vheavy_application->set_function_def("void window_event_sync(window_state, int, int)");
				vheavy_application->set_property<layer::heavy_application>("heavy_application_cache_info cache", &layer::heavy_application::cache);
				vheavy_application->set_property<layer::heavy_application>("audio_device@ audio", &layer::heavy_application::audio);
				vheavy_application->set_property<layer::heavy_application>("graphics_device@ renderer", &layer::heavy_application::renderer);
				vheavy_application->set_property<layer::heavy_application>("activity@ window", &layer::heavy_application::activity);
				vheavy_application->set_property<layer::heavy_application>("render_constants@ constants", &layer::heavy_application::constants);
				vheavy_application->set_property<layer::heavy_application>("content_manager@ content", &layer::heavy_application::content);
				vheavy_application->set_property<layer::heavy_application>("app_data@ database", &layer::heavy_application::database);
				vheavy_application->set_property<layer::heavy_application>("scene_graph@ scene", &layer::heavy_application::scene);
				vheavy_application->set_property<layer::heavy_application>("heavy_application_desc control", &layer::heavy_application::control);
				vheavy_application->set_gc_constructor<heavy_application, heavy_application_name, heavy_application::desc&, void*, int>("heavy_application@ f(heavy_application_desc &in, ?&in)");
				vheavy_application->set_method("void set_on_key_event(key_event_sync@)", &heavy_application::set_on_key_event);
				vheavy_application->set_method("void set_on_input_event(input_event_sync@)", &heavy_application::set_on_input_event);
				vheavy_application->set_method("void set_on_wheel_event(wheel_event_sync@)", &heavy_application::set_on_wheel_event);
				vheavy_application->set_method("void set_on_window_event(window_event_sync@)", &heavy_application::set_on_window_event);
				vheavy_application->set_method("void set_on_dispatch(dispatch_sync@)", &heavy_application::set_on_dispatch);
				vheavy_application->set_method("void set_on_publish(publish_sync@)", &heavy_application::set_on_publish);
				vheavy_application->set_method("void set_on_composition(composition_sync@)", &heavy_application::set_on_composition);
				vheavy_application->set_method("void set_on_script_hook(script_hook_sync@)", &heavy_application::set_on_script_hook);
				vheavy_application->set_method("void set_on_initialize(initialize_sync@)", &heavy_application::set_on_initialize);
				vheavy_application->set_method("void set_on_startup(startup_sync@)", &heavy_application::set_on_startup);
				vheavy_application->set_method("void set_on_shutdown(shutdown_sync@)", &heavy_application::set_on_shutdown);
				vheavy_application->set_method("int start()", &layer::heavy_application::start);
				vheavy_application->set_method("int restart()", &layer::heavy_application::restart);
				vheavy_application->set_method("void stop(int = 0)", &layer::heavy_application::stop);
				vheavy_application->set_method("application_state get_state() const", &layer::heavy_application::get_state);
				vheavy_application->set_method("uptr@ get_initiator() const", &heavy_application::get_initiator_object);
				vheavy_application->set_method("usize get_processed_events() const", &heavy_application::get_processed_events);
				vheavy_application->set_method("bool has_processed_events() const", &heavy_application::has_processed_events);
				vheavy_application->set_method("bool retrieve_initiator(?&out) const", &heavy_application::retrieve_initiator_object);
				vheavy_application->set_method("ui_context@+ try_get_ui() const", &heavy_application::try_get_ui);
				vheavy_application->set_method("ui_context@+ fetch_ui()", &heavy_application::fetch_ui);
				vheavy_application->set_method_static("heavy_application@+ get()", &layer::heavy_application::get);
				vheavy_application->set_method_static("bool wants_restart(int)", &heavy_application::wants_restart);
				vheavy_application->set_enum_refs_ex<heavy_application>([](heavy_application* base, asIScriptEngine* vm)
				{
					function_factory::gc_enum_callback(vm, base->audio);
					function_factory::gc_enum_callback(vm, base->renderer);
					function_factory::gc_enum_callback(vm, base->activity);
					function_factory::gc_enum_callback(vm, base->vm);
					function_factory::gc_enum_callback(vm, base->content);
					function_factory::gc_enum_callback(vm, base->database);
					function_factory::gc_enum_callback(vm, base->scene);
					function_factory::gc_enum_callback(vm, base->get_initiator_object());
					function_factory::gc_enum_callback(vm, &base->on_key_event);
					function_factory::gc_enum_callback(vm, &base->on_input_event);
					function_factory::gc_enum_callback(vm, &base->on_wheel_event);
					function_factory::gc_enum_callback(vm, &base->on_window_event);
					function_factory::gc_enum_callback(vm, &base->on_dispatch);
					function_factory::gc_enum_callback(vm, &base->on_publish);
					function_factory::gc_enum_callback(vm, &base->on_composition);
					function_factory::gc_enum_callback(vm, &base->on_script_hook);
					function_factory::gc_enum_callback(vm, &base->on_initialize);
					function_factory::gc_enum_callback(vm, &base->on_startup);
					function_factory::gc_enum_callback(vm, &base->on_shutdown);
				});
				vheavy_application->set_release_refs_ex<heavy_application>([](heavy_application* base, asIScriptEngine*)
				{
					base->~heavy_application();
				});

				return true;
#else
				VI_ASSERT(false, "<engine> is not loaded");
				return false;
#endif
			}
			bool heavy_registry::import_engine_components(virtual_machine* vm) noexcept
			{
#ifdef VI_BINDINGS
				VI_ASSERT(vm != nullptr, "manager should be set");
				vm->set_function_def("void component_resource_event()");

				auto vsoft_body = vm->set_class<layer::components::soft_body>("soft_body_component", false);
				vsoft_body->set_property<layer::components::soft_body>("vector2 texcoord", &layer::components::soft_body::texcoord);
				vsoft_body->set_property<layer::components::soft_body>("bool kinematic", &layer::components::soft_body::kinematic);
				vsoft_body->set_property<layer::components::soft_body>("bool manage", &layer::components::soft_body::manage);
				vsoft_body->set_method_ex("void load(const string_view&in, float = 0.0f, component_resource_event@ = null)", &components_soft_body_load);
				vsoft_body->set_method<layer::components::soft_body, void, physics::hull_shape*, float>("void load(physics_hull_shape@+, float = 0.0f)", &layer::components::soft_body::load);
				vsoft_body->set_method("void load_ellipsoid(const physics_softbody_desc_cv_sconvex &in, float)", &layer::components::soft_body::load_ellipsoid);
				vsoft_body->set_method("void load_patch(const physics_softbody_desc_cv_spatch &in, float)", &layer::components::soft_body::load_patch);
				vsoft_body->set_method("void load_rope(const physics_softbody_desc_cv_srope &in, float)", &layer::components::soft_body::load_rope);
				vsoft_body->set_method("void fill(graphics_device@+, element_buffer@+, element_buffer@+)", &layer::components::soft_body::fill);
				vsoft_body->set_method("void regenerate()", &layer::components::soft_body::regenerate);
				vsoft_body->set_method("void clear()", &layer::components::soft_body::clear);
				vsoft_body->set_method<layer::components::soft_body, void, const trigonometry::vector3&, const trigonometry::vector3&, const trigonometry::vector3&>("void set_transform(const vector3 &in, const vector3 &in, const vector3 &in)", &layer::components::soft_body::set_transform);
				vsoft_body->set_method<layer::components::soft_body, void, bool>("void set_transform(bool)", &layer::components::soft_body::set_transform);
				vsoft_body->set_method("physics_softbody@+ get_body() const", &layer::components::soft_body::get_body);
				populate_drawable_interface<layer::components::soft_body, layer::entity*>(*vsoft_body, "soft_body_component@+ f(scene_entity@+)");

				auto vrigid_body = vm->set_class<layer::components::rigid_body>("rigid_body_component", false);
				vrigid_body->set_property<layer::components::rigid_body>("bool kinematic", &layer::components::rigid_body::kinematic);
				vrigid_body->set_property<layer::components::rigid_body>("bool manage", &layer::components::rigid_body::manage);
				vrigid_body->set_method_ex("void load(const string_view&in, float, float = 0.0f, component_resource_event@ = null)", &components_rigid_body_load);
				vrigid_body->set_method<layer::components::rigid_body, void, btCollisionShape*, float, float>("void load(uptr@, float, float = 0.0f)", &layer::components::rigid_body::load);
				vrigid_body->set_method("void clear()", &layer::components::rigid_body::clear);
				vrigid_body->set_method("void set_mass(float)", &layer::components::rigid_body::set_mass);
				vrigid_body->set_method<layer::components::rigid_body, void, const trigonometry::vector3&, const trigonometry::vector3&, const trigonometry::vector3&>("void set_transform(const vector3 &in, const vector3 &in, const vector3 &in)", &layer::components::rigid_body::set_transform);
				vrigid_body->set_method<layer::components::rigid_body, void, bool>("void set_transform(bool)", &layer::components::rigid_body::set_transform);
				vrigid_body->set_method("physics_rigidbody@+ get_body() const", &layer::components::rigid_body::get_body);
				populate_component_interface<layer::components::rigid_body, layer::entity*>(*vrigid_body, "rigid_body_component@+ f(scene_entity@+)");

				auto vslider_constraint = vm->set_class<layer::components::slider_constraint>("slider_constraint_component", false);
				vslider_constraint->set_method("void load(scene_entity@+, bool, bool)", &layer::components::slider_constraint::load);
				vslider_constraint->set_method("void clear()", &layer::components::slider_constraint::clear);
				vslider_constraint->set_method("physics_rigidbody@+ get_constraint() const", &layer::components::slider_constraint::get_constraint);
				vslider_constraint->set_method("scene_entity@+ get_connection() const", &layer::components::slider_constraint::get_connection);
				populate_component_interface<layer::components::slider_constraint, layer::entity*>(*vslider_constraint, "slider_constraint_component@+ f(scene_entity@+)");

				auto vacceleration = vm->set_class<layer::components::acceleration>("acceleration_component", false);
				vacceleration->set_property<layer::components::acceleration>("vector3 amplitude_velocity", &layer::components::acceleration::amplitude_velocity);
				vacceleration->set_property<layer::components::acceleration>("vector3 amplitude_torque", &layer::components::acceleration::amplitude_torque);
				vacceleration->set_property<layer::components::acceleration>("vector3 constant_velocity", &layer::components::acceleration::constant_velocity);
				vacceleration->set_property<layer::components::acceleration>("vector3 constant_torque", &layer::components::acceleration::constant_torque);
				vacceleration->set_property<layer::components::acceleration>("vector3 constant_center", &layer::components::acceleration::constant_center);
				vacceleration->set_property<layer::components::acceleration>("bool kinematic", &layer::components::acceleration::kinematic);
				vacceleration->set_method("rigid_body_component@+ get_body() const", &layer::components::acceleration::get_body);
				populate_component_interface<layer::components::acceleration, layer::entity*>(*vacceleration, "acceleration_component@+ f(scene_entity@+)");

				auto vmodel = vm->set_class<layer::components::model>("model_component", false);
				vmodel->set_property<layer::components::model>("vector2 texcoord", &layer::components::model::texcoord);
				vmodel->set_method("void set_drawable(model@+)", &layer::components::model::set_drawable);
				vmodel->set_method("void set_material_for(const string_view&in, material@+)", &layer::components::model::set_material_for);
				vmodel->set_method("model@+ get_drawable() const", &layer::components::model::get_drawable);
				vmodel->set_method("material@+ get_material_for(const string_view&in)", &layer::components::model::get_material_for);
				populate_drawable_interface<layer::components::model, layer::entity*>(*vmodel, "model_component@+ f(scene_entity@+)");

				auto vskin = vm->set_class<layer::components::skin>("skin_component", false);
				vskin->set_property<layer::components::skin>("vector2 texcoord", &layer::components::skin::texcoord);
				vskin->set_property<layer::components::skin>("pose_buffer skeleton", &layer::components::skin::skeleton);
				vskin->set_method("void set_drawable(skin_model@+)", &layer::components::skin::set_drawable);
				vskin->set_method("void set_material_for(const string_view&in, material@+)", &layer::components::skin::set_material_for);
				vskin->set_method("skin_model@+ get_drawable() const", &layer::components::skin::get_drawable);
				vskin->set_method("material@+ get_material_for(const string_view&in)", &layer::components::skin::get_material_for);
				populate_drawable_interface<layer::components::skin, layer::entity*>(*vskin, "skin_component@+ f(scene_entity@+)");

				auto vemitter = vm->set_class<layer::components::emitter>("emitter_component", false);
				vemitter->set_property<layer::components::emitter>("vector3 min", &layer::components::emitter::min);
				vemitter->set_property<layer::components::emitter>("vector3 max", &layer::components::emitter::max);
				vemitter->set_property<layer::components::emitter>("bool connected", &layer::components::emitter::connected);
				vemitter->set_property<layer::components::emitter>("bool quad_based", &layer::components::emitter::quad_based);
				vemitter->set_method("instance_buffer@+ get_buffer() const", &layer::components::emitter::get_buffer);
				populate_drawable_interface<layer::components::emitter, layer::entity*>(*vemitter, "emitter_component@+ f(scene_entity@+)");

				auto vdecal = vm->set_class<layer::components::decal>("decal_component", false);
				vdecal->set_property<layer::components::decal>("vector2 texcoord", &layer::components::decal::texcoord);
				populate_drawable_interface<layer::components::decal, layer::entity*>(*vdecal, "decal_component@+ f(scene_entity@+)");

				auto vskin_animator = vm->set_class<layer::components::skin_animator>("skin_animator_component", false);
				vskin_animator->set_property<layer::components::skin_animator>("animator_state state", &layer::components::skin_animator::state);
				vskin_animator->set_method("void set_animation(skin_animation@+)", &layer::components::skin_animator::set_animation);
				vskin_animator->set_method("void play(int64 = -1, int64 = -1)", &layer::components::skin_animator::play);
				vskin_animator->set_method("void pause()", &layer::components::skin_animator::pause);
				vskin_animator->set_method("void stop()", &layer::components::skin_animator::stop);
				vskin_animator->set_method<layer::components::skin_animator, bool, int64_t>("bool is_exists(int64) const", &layer::components::skin_animator::is_exists);
				vskin_animator->set_method<layer::components::skin_animator, bool, int64_t, int64_t>("bool is_exists(int64, int64) const", &layer::components::skin_animator::is_exists);
				vskin_animator->set_method("skin_animator_key& get_frame(int64, int64) const", &layer::components::skin_animator::get_frame);
				vskin_animator->set_method("skin_component@+ get_skin() const", &layer::components::skin_animator::get_skin);
				vskin_animator->set_method("skin_animation@+ get_animation() const", &layer::components::skin_animator::get_animation);
				vskin_animator->set_method("string get_path() const", &layer::components::skin_animator::get_path);
				vskin_animator->set_method("int64 get_clip_by_name(const string_view&in) const", &layer::components::skin_animator::get_clip_by_name);
				vskin_animator->set_method("usize get_clips_count() const", &layer::components::skin_animator::get_clips_count);
				populate_component_interface<layer::components::skin_animator, layer::entity*>(*vskin_animator, "skin_animator_component@+ f(scene_entity@+)");

				auto vkey_animator = vm->set_class<layer::components::key_animator>("key_animator_component", false);
				vkey_animator->set_property<layer::components::key_animator>("animator_key offset_pose", &layer::components::key_animator::offset);
				vkey_animator->set_property<layer::components::key_animator>("animator_key default_pose", &layer::components::key_animator::defaults);
				vkey_animator->set_property<layer::components::key_animator>("animator_state state", &layer::components::key_animator::state);
				vkey_animator->set_method_ex("void load_animation(const string_view&in, component_resource_event@ = null)", &components_key_animator_load_animation);
				vkey_animator->set_method("void clear_animation()", &layer::components::key_animator::clear_animation);
				vkey_animator->set_method("void play(int64 = -1, int64 = -1)", &layer::components::key_animator::play);
				vkey_animator->set_method("void pause()", &layer::components::key_animator::pause);
				vkey_animator->set_method("void stop()", &layer::components::key_animator::stop);
				vkey_animator->set_method<layer::components::key_animator, bool, int64_t>("bool is_exists(int64) const", &layer::components::key_animator::is_exists);
				vkey_animator->set_method<layer::components::key_animator, bool, int64_t, int64_t>("bool is_exists(int64, int64) const", &layer::components::key_animator::is_exists);
				vkey_animator->set_method("animator_key& get_frame(int64, int64) const", &layer::components::key_animator::get_frame);
				vkey_animator->set_method("string get_path() const", &layer::components::key_animator::get_path);
				populate_component_interface<layer::components::key_animator, layer::entity*>(*vkey_animator, "key_animator_component@+ f(scene_entity@+)");

				auto vemitter_animator = vm->set_class<layer::components::emitter_animator>("emitter_animator_component", false);
				vemitter_animator->set_property<layer::components::emitter_animator>("vector4 diffuse", &layer::components::emitter_animator::diffuse);
				vemitter_animator->set_property<layer::components::emitter_animator>("vector3 position", &layer::components::emitter_animator::position);
				vemitter_animator->set_property<layer::components::emitter_animator>("vector3 velocity", &layer::components::emitter_animator::velocity);
				vemitter_animator->set_property<layer::components::emitter_animator>("spawner_properties spawner", &layer::components::emitter_animator::spawner);
				vemitter_animator->set_property<layer::components::emitter_animator>("float rotation_speed", &layer::components::emitter_animator::rotation_speed);
				vemitter_animator->set_property<layer::components::emitter_animator>("float scale_speed", &layer::components::emitter_animator::scale_speed);
				vemitter_animator->set_property<layer::components::emitter_animator>("float noise", &layer::components::emitter_animator::noise);
				vemitter_animator->set_property<layer::components::emitter_animator>("bool simulate", &layer::components::emitter_animator::simulate);
				vemitter_animator->set_method("emitter_component@+ get_emitter() const", &layer::components::emitter_animator::get_emitter);
				populate_component_interface<layer::components::emitter_animator, layer::entity*>(*vemitter_animator, "emitter_animator_component@+ f(scene_entity@+)");

				auto vfree_look = vm->set_class<layer::components::free_look>("free_look_component", false);
				vfree_look->set_property<layer::components::free_look>("vector2 direction", &layer::components::free_look::direction);
				vfree_look->set_property<layer::components::free_look>("key_map rotate", &layer::components::free_look::rotate);
				vfree_look->set_property<layer::components::free_look>("float sensivity", &layer::components::free_look::sensivity);
				populate_component_interface<layer::components::free_look, layer::entity*>(*vfree_look, "free_look_component@+ f(scene_entity@+)");

				auto vfly_move_info = vm->set_pod<layer::components::fly::move_info>("fly_move_info");
				vfly_move_info->set_property<layer::components::fly::move_info>("vector3 axis", &layer::components::fly::move_info::axis);
				vfly_move_info->set_property<layer::components::fly::move_info>("float faster", &layer::components::fly::move_info::faster);
				vfly_move_info->set_property<layer::components::fly::move_info>("float normal", &layer::components::fly::move_info::normal);
				vfly_move_info->set_property<layer::components::fly::move_info>("float slower", &layer::components::fly::move_info::slower);
				vfly_move_info->set_property<layer::components::fly::move_info>("float fading", &layer::components::fly::move_info::fading);

				auto vfly = vm->set_class<layer::components::fly>("fly_component", false);
				vfly->set_property<layer::components::fly>("fly_move_info moving", &layer::components::fly::moving);
				vfly->set_property<layer::components::fly>("key_map forward", &layer::components::fly::forward);
				vfly->set_property<layer::components::fly>("key_map backward", &layer::components::fly::backward);
				vfly->set_property<layer::components::fly>("key_map right", &layer::components::fly::right);
				vfly->set_property<layer::components::fly>("key_map left", &layer::components::fly::left);
				vfly->set_property<layer::components::fly>("key_map up", &layer::components::fly::up);
				vfly->set_property<layer::components::fly>("key_map down", &layer::components::fly::down);
				vfly->set_property<layer::components::fly>("key_map fast", &layer::components::fly::fast);
				vfly->set_property<layer::components::fly>("key_map slow", &layer::components::fly::slow);
				populate_component_interface<layer::components::fly, layer::entity*>(*vfly, "fly_component@+ f(scene_entity@+)");

				auto vaudio_source = vm->set_class<layer::components::audio_source>("audio_source_component", false);
				vaudio_source->set_method("void apply_playing_position()", &layer::components::audio_source::apply_playing_position);
				vaudio_source->set_method("audio_source@+ get_source() const", &layer::components::audio_source::get_source);
				vaudio_source->set_method("audio_sync& get_sync()", &layer::components::audio_source::get_sync);
				populate_component_interface<layer::components::audio_source, layer::entity*>(*vaudio_source, "audio_source_component@+ f(scene_entity@+)");

				auto vaudio_listener = vm->set_class<layer::components::audio_listener>("audio_listener_component", false);
				vaudio_listener->set_property<layer::components::audio_listener>("float gain", &layer::components::audio_listener::gain);
				populate_component_interface<layer::components::audio_listener, layer::entity*>(*vaudio_listener, "audio_listener_component@+ f(scene_entity@+)");

				auto vpoint_light_shadow_info = vm->set_pod<layer::components::point_light::shadow_info>("point_light_shadow_info");
				vpoint_light_shadow_info->set_property<layer::components::point_light::shadow_info>("float softness", &layer::components::point_light::shadow_info::softness);
				vpoint_light_shadow_info->set_property<layer::components::point_light::shadow_info>("float distance", &layer::components::point_light::shadow_info::distance);
				vpoint_light_shadow_info->set_property<layer::components::point_light::shadow_info>("float bias", &layer::components::point_light::shadow_info::bias);
				vpoint_light_shadow_info->set_property<layer::components::point_light::shadow_info>("uint32 iterations", &layer::components::point_light::shadow_info::iterations);
				vpoint_light_shadow_info->set_property<layer::components::point_light::shadow_info>("bool enabled", &layer::components::point_light::shadow_info::enabled);

				auto vpoint_light = vm->set_class<layer::components::point_light>("point_light_component", false);
				vpoint_light->set_property<layer::components::point_light>("point_light_shadow_info shadow", &layer::components::point_light::shadow);
				vpoint_light->set_property<layer::components::point_light>("matrix4x4 view", &layer::components::point_light::view);
				vpoint_light->set_property<layer::components::point_light>("matrix4x4 projection", &layer::components::point_light::projection);
				vpoint_light->set_property<layer::components::point_light>("vector3 diffuse", &layer::components::point_light::diffuse);
				vpoint_light->set_property<layer::components::point_light>("float emission", &layer::components::point_light::emission);
				vpoint_light->set_property<layer::components::point_light>("float disperse", &layer::components::point_light::disperse);
				vpoint_light->set_method("void generate_origin()", &layer::components::point_light::generate_origin);
				vpoint_light->set_method("void set_size(const attenuation &in)", &layer::components::point_light::set_size);
				vpoint_light->set_method("const attenuation& get_size() const", &layer::components::point_light::set_size);
				populate_component_interface<layer::components::point_light, layer::entity*>(*vpoint_light, "point_light_component@+ f(scene_entity@+)");

				auto vspot_light_shadow_info = vm->set_pod<layer::components::spot_light::shadow_info>("spot_light_shadow_info");
				vspot_light_shadow_info->set_property<layer::components::spot_light::shadow_info>("float softness", &layer::components::spot_light::shadow_info::softness);
				vspot_light_shadow_info->set_property<layer::components::spot_light::shadow_info>("float distance", &layer::components::spot_light::shadow_info::distance);
				vspot_light_shadow_info->set_property<layer::components::spot_light::shadow_info>("float bias", &layer::components::spot_light::shadow_info::bias);
				vspot_light_shadow_info->set_property<layer::components::spot_light::shadow_info>("uint32 iterations", &layer::components::spot_light::shadow_info::iterations);
				vspot_light_shadow_info->set_property<layer::components::spot_light::shadow_info>("bool enabled", &layer::components::spot_light::shadow_info::enabled);

				auto vspot_light = vm->set_class<layer::components::spot_light>("spot_light_component", false);
				vspot_light->set_property<layer::components::spot_light>("spot_light_shadow_info shadow", &layer::components::spot_light::shadow);
				vspot_light->set_property<layer::components::spot_light>("matrix4x4 view", &layer::components::spot_light::view);
				vspot_light->set_property<layer::components::spot_light>("matrix4x4 projection", &layer::components::spot_light::projection);
				vspot_light->set_property<layer::components::spot_light>("vector3 diffuse", &layer::components::spot_light::diffuse);
				vspot_light->set_property<layer::components::spot_light>("float emission", &layer::components::spot_light::emission);
				vspot_light->set_property<layer::components::spot_light>("float cutoff", &layer::components::spot_light::cutoff);
				vspot_light->set_property<layer::components::spot_light>("float disperse", &layer::components::spot_light::disperse);
				vspot_light->set_method("void generate_origin()", &layer::components::spot_light::generate_origin);
				vspot_light->set_method("void set_size(const attenuation &in)", &layer::components::spot_light::set_size);
				vspot_light->set_method("const attenuation& get_size() const", &layer::components::spot_light::set_size);
				populate_component_interface<layer::components::spot_light, layer::entity*>(*vspot_light, "spot_light_component@+ f(scene_entity@+)");

				auto vline_light_sky_info = vm->set_pod<layer::components::line_light::sky_info>("line_light_sky_info");
				vline_light_sky_info->set_property<layer::components::line_light::sky_info>("vector3 elh_emission", &layer::components::line_light::sky_info::rlh_emission);
				vline_light_sky_info->set_property<layer::components::line_light::sky_info>("vector3 mie_emission", &layer::components::line_light::sky_info::mie_emission);
				vline_light_sky_info->set_property<layer::components::line_light::sky_info>("float rlh_height", &layer::components::line_light::sky_info::rlh_height);
				vline_light_sky_info->set_property<layer::components::line_light::sky_info>("float mie_height", &layer::components::line_light::sky_info::mie_height);
				vline_light_sky_info->set_property<layer::components::line_light::sky_info>("float mie_direction", &layer::components::line_light::sky_info::mie_direction);
				vline_light_sky_info->set_property<layer::components::line_light::sky_info>("float inner_radius", &layer::components::line_light::sky_info::inner_radius);
				vline_light_sky_info->set_property<layer::components::line_light::sky_info>("float outer_radius", &layer::components::line_light::sky_info::outer_radius);
				vline_light_sky_info->set_property<layer::components::line_light::sky_info>("float intensity", &layer::components::line_light::sky_info::intensity);

				auto vline_light_shadow_info = vm->set_pod<layer::components::line_light::shadow_info>("line_light_shadow_info");
				vline_light_shadow_info->set_property_array<layer::components::line_light::shadow_info>("float distance", &layer::components::line_light::shadow_info::distance, 6);
				vline_light_shadow_info->set_property<layer::components::line_light::shadow_info>("float softness", &layer::components::line_light::shadow_info::softness);
				vline_light_shadow_info->set_property<layer::components::line_light::shadow_info>("float bias", &layer::components::line_light::shadow_info::bias);
				vline_light_shadow_info->set_property<layer::components::line_light::shadow_info>("float near", &layer::components::line_light::shadow_info::near);
				vline_light_shadow_info->set_property<layer::components::line_light::shadow_info>("float far", &layer::components::line_light::shadow_info::far);
				vline_light_shadow_info->set_property<layer::components::line_light::shadow_info>("uint32 iterations", &layer::components::line_light::shadow_info::iterations);
				vline_light_shadow_info->set_property<layer::components::line_light::shadow_info>("uint32 cascades", &layer::components::line_light::shadow_info::cascades);
				vline_light_shadow_info->set_property<layer::components::line_light::shadow_info>("bool enabled", &layer::components::line_light::shadow_info::enabled);

				auto vline_light = vm->set_class<layer::components::line_light>("line_light_component", false);
				vline_light->set_property<layer::components::line_light>("line_light_sky_info sky", &layer::components::line_light::sky);
				vline_light->set_property<layer::components::line_light>("line_light_shadow_info shadow", &layer::components::line_light::shadow);
				vline_light->set_property_array<layer::components::line_light>("matrix4x4 projection", &layer::components::line_light::projection, 6);
				vline_light->set_property_array<layer::components::line_light>("matrix4x4 view", &layer::components::line_light::view, 6);
				vline_light->set_property<layer::components::line_light>("vector3 diffuse", &layer::components::line_light::diffuse);
				vline_light->set_property<layer::components::line_light>("float emission", &layer::components::line_light::emission);
				vline_light->set_method("void generate_origin()", &layer::components::line_light::generate_origin);
				populate_component_interface<layer::components::line_light, layer::entity*>(*vline_light, "line_light_component@+ f(scene_entity@+)");

				auto vsurface_light = vm->set_class<layer::components::surface_light>("surface_light_component", false);
				vsurface_light->set_property_array<layer::components::surface_light>("matrix4x4 view", &layer::components::surface_light::view, 6);
				vsurface_light->set_property<layer::components::surface_light>("matrix4x4 projection", &layer::components::surface_light::projection);
				vsurface_light->set_property<layer::components::surface_light>("vector3 offset", &layer::components::surface_light::offset);
				vsurface_light->set_property<layer::components::surface_light>("vector3 diffuse", &layer::components::surface_light::diffuse);
				vsurface_light->set_property<layer::components::surface_light>("clock_ticker tick", &layer::components::surface_light::tick);
				vsurface_light->set_property<layer::components::surface_light>("float emission", &layer::components::surface_light::emission);
				vsurface_light->set_property<layer::components::surface_light>("float infinity", &layer::components::surface_light::infinity);
				vsurface_light->set_property<layer::components::surface_light>("bool parallax", &layer::components::surface_light::parallax);
				vsurface_light->set_property<layer::components::surface_light>("bool locked", &layer::components::surface_light::locked);
				vsurface_light->set_property<layer::components::surface_light>("bool static_mask", &layer::components::surface_light::static_mask);
				vsurface_light->set_method("void set_probe_cache(texture_cube@+)", &layer::components::surface_light::set_probe_cache);
				vsurface_light->set_method("void set_size(const attenuation &in)", &layer::components::surface_light::set_size);
				vsurface_light->set_method<layer::components::surface_light, bool, graphics::texture_2d*>("void set_diffuse_map(texture_2d@+)", &layer::components::surface_light::set_diffuse_map);
				vsurface_light->set_method("bool is_image_based() const", &layer::components::surface_light::is_image_based);
				vsurface_light->set_method("const attenuation& get_size() const", &layer::components::surface_light::get_size);
				vsurface_light->set_method("texture_cube@+ get_probe_cache() const", &layer::components::surface_light::get_probe_cache);
				vsurface_light->set_method("texture_2d@+ get_diffuse_map_xp() const", &layer::components::surface_light::get_diffuse_map_xp);
				vsurface_light->set_method("texture_2d@+ get_diffuse_map_xn() const", &layer::components::surface_light::get_diffuse_map_xn);
				vsurface_light->set_method("texture_2d@+ get_diffuse_map_yp() const", &layer::components::surface_light::get_diffuse_map_yp);
				vsurface_light->set_method("texture_2d@+ get_diffuse_map_yn() const", &layer::components::surface_light::get_diffuse_map_yn);
				vsurface_light->set_method("texture_2d@+ get_diffuse_map_zp() const", &layer::components::surface_light::get_diffuse_map_zp);
				vsurface_light->set_method("texture_2d@+ get_diffuse_map_zn() const", &layer::components::surface_light::get_diffuse_map_zn);
				vsurface_light->set_method("texture_2d@+ get_diffuse_map() const", &layer::components::surface_light::get_diffuse_map);
				populate_component_interface<layer::components::surface_light, layer::entity*>(*vsurface_light, "surface_light_component@+ f(scene_entity@+)");

				auto villuminator = vm->set_class<layer::components::illuminator>("illuminator_component", false);
				villuminator->set_property<layer::components::illuminator>("clock_ticker inside", &layer::components::illuminator::inside);
				villuminator->set_property<layer::components::illuminator>("clock_ticker outside", &layer::components::illuminator::outside);
				villuminator->set_property<layer::components::illuminator>("float ray_step", &layer::components::illuminator::ray_step);
				villuminator->set_property<layer::components::illuminator>("float max_steps", &layer::components::illuminator::max_steps);
				villuminator->set_property<layer::components::illuminator>("float distance", &layer::components::illuminator::distance);
				villuminator->set_property<layer::components::illuminator>("float radiance", &layer::components::illuminator::radiance);
				villuminator->set_property<layer::components::illuminator>("float length", &layer::components::illuminator::length);
				villuminator->set_property<layer::components::illuminator>("float margin", &layer::components::illuminator::margin);
				villuminator->set_property<layer::components::illuminator>("float offset", &layer::components::illuminator::offset);
				villuminator->set_property<layer::components::illuminator>("float angle", &layer::components::illuminator::angle);
				villuminator->set_property<layer::components::illuminator>("float occlusion", &layer::components::illuminator::occlusion);
				villuminator->set_property<layer::components::illuminator>("float specular", &layer::components::illuminator::specular);
				villuminator->set_property<layer::components::illuminator>("float bleeding", &layer::components::illuminator::bleeding);
				villuminator->set_property<layer::components::illuminator>("bool regenerate", &layer::components::illuminator::regenerate);
				populate_component_interface<layer::components::illuminator, layer::entity*>(*villuminator, "illuminator_component@+ f(scene_entity@+)");

				auto vcamera_projection = vm->set_enum("camera_projection");
				vcamera_projection->set_value("perspective", (int)layer::components::camera::projection_mode::perspective);
				vcamera_projection->set_value("orthographic", (int)layer::components::camera::projection_mode::orthographic);

				auto vcamera = vm->set_class<layer::components::camera>("camera_component", false);
				vcamera->set_property<layer::components::camera>("camera_projection mode", &layer::components::camera::mode);
				vcamera->set_property<layer::components::camera>("float near_plane", &layer::components::camera::near_plane);
				vcamera->set_property<layer::components::camera>("float far_plane", &layer::components::camera::far_plane);
				vcamera->set_property<layer::components::camera>("float width", &layer::components::camera::width);
				vcamera->set_property<layer::components::camera>("float height", &layer::components::camera::height);
				vcamera->set_property<layer::components::camera>("float field_of_view", &layer::components::camera::field_of_view);
				vcamera->set_method<layer::components::camera, void, layer::viewer*>("void get_viewer(viewer_t &out)", &layer::components::camera::get_viewer);
				vcamera->set_method("void resize_buffers()", &layer::components::camera::resize_buffers);
				vcamera->set_method<layer::components::camera, layer::viewer&>("viewer_t& get_viewer()", &layer::components::camera::get_viewer);
				vcamera->set_method("render_system@+ get_renderer() const", &layer::components::camera::get_renderer);
				vcamera->set_method("matrix4x4 get_projection() const", &layer::components::camera::get_projection);
				vcamera->set_method("matrix4x4 get_view_projection() const", &layer::components::camera::get_view_projection);
				vcamera->set_method("matrix4x4 get_view() const", &layer::components::camera::get_view);
				vcamera->set_method("vector3 get_view_position() const", &layer::components::camera::get_view_position);
				vcamera->set_method("frustum_8c get_frustum_8c() const", &layer::components::camera::get_frustum8c);
				vcamera->set_method("frustum_6p get_frustum_6p() const", &layer::components::camera::get_frustum6p);
				vcamera->set_method("ray get_screen_ray(const vector2 &in) const", &layer::components::camera::get_screen_ray);
				vcamera->set_method("ray get_cursor_ray() const", &layer::components::camera::get_cursor_ray);
				vcamera->set_method("float get_distance(scene_entity@+) const", &layer::components::camera::get_distance);
				vcamera->set_method("float get_width() const", &layer::components::camera::get_width);
				vcamera->set_method("float get_height() const", &layer::components::camera::get_height);
				vcamera->set_method("float get_aspect() const", &layer::components::camera::get_aspect);
				vcamera->set_method<layer::components::camera, bool, const trigonometry::ray&, layer::entity*, trigonometry::vector3*>("bool ray_test(const ray &in, scene_entity@+, vector3 &out)", &layer::components::camera::ray_test);
				vcamera->set_method<layer::components::camera, bool, const trigonometry::ray&, const trigonometry::matrix4x4&, trigonometry::vector3*>("bool ray_test(const ray &in, const matrix4x4 &in, vector3 &out)", &layer::components::camera::ray_test);
				populate_component_interface<layer::components::camera, layer::entity*>(*vcamera, "camera_component@+ f(scene_entity@+)");

				auto vscriptable = vm->set_class<layer::components::scriptable>("scriptable_component", false);
				populate_component_interface<layer::components::scriptable, layer::entity*>(*vscriptable, "scriptable_component@+ f(scene_entity@+)");

				return true;
#else
				VI_ASSERT(false, "<components> is not loaded");
				return false;
#endif
			}
			bool heavy_registry::import_engine_renderers(virtual_machine* vm) noexcept
			{
#ifdef VI_BINDINGS
				VI_ASSERT(vm != nullptr, "manager should be set");

				auto vsoft_body = vm->set_class<layer::renderers::soft_body>("soft_body_renderer", false);
				populate_renderer_interface<layer::renderers::soft_body, layer::render_system*>(*vsoft_body, "soft_body_renderer@+ f(render_system@+)");

				auto vmodel = vm->set_class<layer::renderers::model>("model_renderer", false);
				populate_renderer_interface<layer::renderers::model, layer::render_system*>(*vmodel, "model_renderer@+ f(render_system@+)");

				auto vskin = vm->set_class<layer::renderers::skin>("skin_renderer", false);
				populate_renderer_interface<layer::renderers::skin, layer::render_system*>(*vskin, "skin_renderer@+ f(render_system@+)");

				auto vemitter = vm->set_class<layer::renderers::emitter>("emitter_renderer", false);
				populate_renderer_interface<layer::renderers::emitter, layer::render_system*>(*vemitter, "emitter_renderer@+ f(render_system@+)");

				auto vdecal = vm->set_class<layer::renderers::decal>("decal_renderer", false);
				populate_renderer_interface<layer::renderers::decal, layer::render_system*>(*vdecal, "decal_renderer@+ f(render_system@+)");

				auto vlighting_isurface_buffer = vm->set_pod<layer::renderers::lighting::isurface_buffer>("lighting_surface_light");
				vlighting_isurface_buffer->set_property<layer::renderers::lighting::isurface_buffer>("matrix4x4 world", &layer::renderers::lighting::isurface_buffer::transform);
				vlighting_isurface_buffer->set_property<layer::renderers::lighting::isurface_buffer>("vector3 position", &layer::renderers::lighting::isurface_buffer::position);
				vlighting_isurface_buffer->set_property<layer::renderers::lighting::isurface_buffer>("float range", &layer::renderers::lighting::isurface_buffer::range);
				vlighting_isurface_buffer->set_property<layer::renderers::lighting::isurface_buffer>("vector3 lighting", &layer::renderers::lighting::isurface_buffer::lighting);
				vlighting_isurface_buffer->set_property<layer::renderers::lighting::isurface_buffer>("float mips", &layer::renderers::lighting::isurface_buffer::mips);
				vlighting_isurface_buffer->set_property<layer::renderers::lighting::isurface_buffer>("vector3 scale", &layer::renderers::lighting::isurface_buffer::scale);
				vlighting_isurface_buffer->set_property<layer::renderers::lighting::isurface_buffer>("float parallax", &layer::renderers::lighting::isurface_buffer::parallax);
				vlighting_isurface_buffer->set_property<layer::renderers::lighting::isurface_buffer>("vector3 attenuations", &layer::renderers::lighting::isurface_buffer::attenuation);
				vlighting_isurface_buffer->set_property<layer::renderers::lighting::isurface_buffer>("float infinity", &layer::renderers::lighting::isurface_buffer::infinity);

				auto vlighting_ipoint_buffer = vm->set_pod<layer::renderers::lighting::ipoint_buffer>("lighting_point_light");
				vlighting_ipoint_buffer->set_property<layer::renderers::lighting::ipoint_buffer>("matrix4x4 world", &layer::renderers::lighting::ipoint_buffer::transform);
				vlighting_ipoint_buffer->set_property<layer::renderers::lighting::ipoint_buffer>("vector4 attenuations", &layer::renderers::lighting::ipoint_buffer::attenuation);
				vlighting_ipoint_buffer->set_property<layer::renderers::lighting::ipoint_buffer>("vector3 position", &layer::renderers::lighting::ipoint_buffer::position);
				vlighting_ipoint_buffer->set_property<layer::renderers::lighting::ipoint_buffer>("float range", &layer::renderers::lighting::ipoint_buffer::range);
				vlighting_ipoint_buffer->set_property<layer::renderers::lighting::ipoint_buffer>("vector3 lighting", &layer::renderers::lighting::ipoint_buffer::lighting);
				vlighting_ipoint_buffer->set_property<layer::renderers::lighting::ipoint_buffer>("float distance", &layer::renderers::lighting::ipoint_buffer::distance);
				vlighting_ipoint_buffer->set_property<layer::renderers::lighting::ipoint_buffer>("float umbra", &layer::renderers::lighting::ipoint_buffer::umbra);
				vlighting_ipoint_buffer->set_property<layer::renderers::lighting::ipoint_buffer>("float softness", &layer::renderers::lighting::ipoint_buffer::softness);
				vlighting_ipoint_buffer->set_property<layer::renderers::lighting::ipoint_buffer>("float bias", &layer::renderers::lighting::ipoint_buffer::bias);
				vlighting_ipoint_buffer->set_property<layer::renderers::lighting::ipoint_buffer>("float iterations", &layer::renderers::lighting::ipoint_buffer::iterations);

				auto vlighting_ispot_buffer = vm->set_pod<layer::renderers::lighting::ispot_buffer>("lighting_spot_light");
				vlighting_ispot_buffer->set_property<layer::renderers::lighting::ispot_buffer>("matrix4x4 world", &layer::renderers::lighting::ispot_buffer::transform);
				vlighting_ispot_buffer->set_property<layer::renderers::lighting::ispot_buffer>("matrix4x4 view_projection", &layer::renderers::lighting::ispot_buffer::view_projection);
				vlighting_ispot_buffer->set_property<layer::renderers::lighting::ispot_buffer>("vector4 attenuations", &layer::renderers::lighting::ispot_buffer::attenuation);
				vlighting_ispot_buffer->set_property<layer::renderers::lighting::ispot_buffer>("vector3 direction", &layer::renderers::lighting::ispot_buffer::direction);
				vlighting_ispot_buffer->set_property<layer::renderers::lighting::ispot_buffer>("float cutoff", &layer::renderers::lighting::ispot_buffer::cutoff);
				vlighting_ispot_buffer->set_property<layer::renderers::lighting::ispot_buffer>("vector3 position", &layer::renderers::lighting::ispot_buffer::position);
				vlighting_ispot_buffer->set_property<layer::renderers::lighting::ispot_buffer>("float range", &layer::renderers::lighting::ispot_buffer::range);
				vlighting_ispot_buffer->set_property<layer::renderers::lighting::ispot_buffer>("vector3 lighting", &layer::renderers::lighting::ispot_buffer::lighting);
				vlighting_ispot_buffer->set_property<layer::renderers::lighting::ispot_buffer>("float softness", &layer::renderers::lighting::ispot_buffer::softness);
				vlighting_ispot_buffer->set_property<layer::renderers::lighting::ispot_buffer>("float bias", &layer::renderers::lighting::ispot_buffer::bias);
				vlighting_ispot_buffer->set_property<layer::renderers::lighting::ispot_buffer>("float iterations", &layer::renderers::lighting::ispot_buffer::iterations);
				vlighting_ispot_buffer->set_property<layer::renderers::lighting::ispot_buffer>("float umbra", &layer::renderers::lighting::ispot_buffer::umbra);
				vlighting_ispot_buffer->set_property<layer::renderers::lighting::ispot_buffer>("float padding", &layer::renderers::lighting::ispot_buffer::padding);

				auto vlighting_iline_buffer = vm->set_pod<layer::renderers::lighting::iline_buffer>("lighting_line_light");
				vlighting_iline_buffer->set_property_array<layer::renderers::lighting::iline_buffer>("matrix4x4 view_projection", &layer::renderers::lighting::iline_buffer::view_projection, 6);
				vlighting_iline_buffer->set_property<layer::renderers::lighting::iline_buffer>("matrix4x4 sky_offset", &layer::renderers::lighting::iline_buffer::sky_offset);
				vlighting_iline_buffer->set_property<layer::renderers::lighting::iline_buffer>("vector3 rlh_emission", &layer::renderers::lighting::iline_buffer::rlh_emission);
				vlighting_iline_buffer->set_property<layer::renderers::lighting::iline_buffer>("float rlh_height", &layer::renderers::lighting::iline_buffer::rlh_height);
				vlighting_iline_buffer->set_property<layer::renderers::lighting::iline_buffer>("vector3 mie_emission", &layer::renderers::lighting::iline_buffer::mie_emission);
				vlighting_iline_buffer->set_property<layer::renderers::lighting::iline_buffer>("float mie_height", &layer::renderers::lighting::iline_buffer::mie_height);
				vlighting_iline_buffer->set_property<layer::renderers::lighting::iline_buffer>("vector3 lighting", &layer::renderers::lighting::iline_buffer::lighting);
				vlighting_iline_buffer->set_property<layer::renderers::lighting::iline_buffer>("float softness", &layer::renderers::lighting::iline_buffer::softness);
				vlighting_iline_buffer->set_property<layer::renderers::lighting::iline_buffer>("vector3 position", &layer::renderers::lighting::iline_buffer::position);
				vlighting_iline_buffer->set_property<layer::renderers::lighting::iline_buffer>("float cascades", &layer::renderers::lighting::iline_buffer::cascades);
				vlighting_iline_buffer->set_property<layer::renderers::lighting::iline_buffer>("vector2 padding", &layer::renderers::lighting::iline_buffer::padding);
				vlighting_iline_buffer->set_property<layer::renderers::lighting::iline_buffer>("float bias", &layer::renderers::lighting::iline_buffer::bias);
				vlighting_iline_buffer->set_property<layer::renderers::lighting::iline_buffer>("float iterations", &layer::renderers::lighting::iline_buffer::iterations);
				vlighting_iline_buffer->set_property<layer::renderers::lighting::iline_buffer>("float scatter_intensity", &layer::renderers::lighting::iline_buffer::scatter_intensity);
				vlighting_iline_buffer->set_property<layer::renderers::lighting::iline_buffer>("float planet_radius", &layer::renderers::lighting::iline_buffer::planet_radius);
				vlighting_iline_buffer->set_property<layer::renderers::lighting::iline_buffer>("float atmosphere_radius", &layer::renderers::lighting::iline_buffer::atmosphere_radius);
				vlighting_iline_buffer->set_property<layer::renderers::lighting::iline_buffer>("float mie_direction", &layer::renderers::lighting::iline_buffer::mie_direction);

				auto vlighting_iambient_buffer = vm->set_pod<layer::renderers::lighting::iambient_buffer>("lighting_ambient_light");
				vlighting_iambient_buffer->set_property<layer::renderers::lighting::iambient_buffer>("matrix4x4 sky_offset", &layer::renderers::lighting::iambient_buffer::sky_offset);
				vlighting_iambient_buffer->set_property<layer::renderers::lighting::iambient_buffer>("vector3 high_emission", &layer::renderers::lighting::iambient_buffer::high_emission);
				vlighting_iambient_buffer->set_property<layer::renderers::lighting::iambient_buffer>("float sky_emission", &layer::renderers::lighting::iambient_buffer::sky_emission);
				vlighting_iambient_buffer->set_property<layer::renderers::lighting::iambient_buffer>("vector3 low_emission", &layer::renderers::lighting::iambient_buffer::low_emission);
				vlighting_iambient_buffer->set_property<layer::renderers::lighting::iambient_buffer>("float light_emission", &layer::renderers::lighting::iambient_buffer::light_emission);
				vlighting_iambient_buffer->set_property<layer::renderers::lighting::iambient_buffer>("vector3 sky_color", &layer::renderers::lighting::iambient_buffer::sky_color);
				vlighting_iambient_buffer->set_property<layer::renderers::lighting::iambient_buffer>("float fog_far_off", &layer::renderers::lighting::iambient_buffer::fog_far_off);
				vlighting_iambient_buffer->set_property<layer::renderers::lighting::iambient_buffer>("vector3 fog_color", &layer::renderers::lighting::iambient_buffer::fog_color);
				vlighting_iambient_buffer->set_property<layer::renderers::lighting::iambient_buffer>("float fog_near_off", &layer::renderers::lighting::iambient_buffer::fog_near_off);
				vlighting_iambient_buffer->set_property<layer::renderers::lighting::iambient_buffer>("vector3 fog_far", &layer::renderers::lighting::iambient_buffer::fog_far);
				vlighting_iambient_buffer->set_property<layer::renderers::lighting::iambient_buffer>("float fog_amount", &layer::renderers::lighting::iambient_buffer::fog_amount);
				vlighting_iambient_buffer->set_property<layer::renderers::lighting::iambient_buffer>("vector3 fog_near", &layer::renderers::lighting::iambient_buffer::fog_near);
				vlighting_iambient_buffer->set_property<layer::renderers::lighting::iambient_buffer>("float recursive", &layer::renderers::lighting::iambient_buffer::recursive);

				auto vlighting = vm->set_class<layer::renderers::lighting>("lighting_renderer", false);
				vlighting->set_property<layer::renderers::lighting>("lighting_ambient_light ambient_light", &layer::renderers::lighting::ambient_buffer);
				vlighting->set_method("void set_sky_map(texture_2d@+)", &layer::renderers::lighting::set_sky_map);
				vlighting->set_method("void set_surface_buffer_size(usize)", &layer::renderers::lighting::set_surface_buffer_size);
				vlighting->set_method("texture_cube@+ get_sky_map() const", &layer::renderers::lighting::get_sky_map);
				vlighting->set_method("texture_2d@+ get_sky_base() const", &layer::renderers::lighting::get_sky_base);
				populate_renderer_interface<layer::renderers::lighting, layer::render_system*>(*vlighting, "lighting_renderer@+ f(render_system@+)");

				auto vtransparency_render_constant = vm->set_pod<layer::renderers::transparency::render_constant>("transparency_render_constant");
				vtransparency_render_constant->set_property<layer::renderers::transparency::render_constant>("vector3 padding", &layer::renderers::transparency::render_constant::padding);
				vtransparency_render_constant->set_property<layer::renderers::transparency::render_constant>("float mips", &layer::renderers::transparency::render_constant::mips);

				auto vtransparency = vm->set_class<layer::renderers::transparency>("transparency_renderer", false);
				vtransparency->set_property<layer::renderers::transparency>("transparency_render_constant render_data", &layer::renderers::transparency::render_data);
				populate_renderer_interface<layer::renderers::transparency, layer::render_system*>(*vtransparency, "transparency_renderer@+ f(render_system@+)");

				auto vlocal_reflections_reflectance_buffer = vm->set_pod<layer::renderers::local_reflections::reflectance_buffer>("local_reflections_reflectance_buffer");
				vlocal_reflections_reflectance_buffer->set_property<layer::renderers::local_reflections::reflectance_buffer>("float samples", &layer::renderers::local_reflections::reflectance_buffer::samples);
				vlocal_reflections_reflectance_buffer->set_property<layer::renderers::local_reflections::reflectance_buffer>("float padding", &layer::renderers::local_reflections::reflectance_buffer::padding);
				vlocal_reflections_reflectance_buffer->set_property<layer::renderers::local_reflections::reflectance_buffer>("float intensity", &layer::renderers::local_reflections::reflectance_buffer::intensity);
				vlocal_reflections_reflectance_buffer->set_property<layer::renderers::local_reflections::reflectance_buffer>("float distance", &layer::renderers::local_reflections::reflectance_buffer::distance);

				auto vlocal_reflections_gloss_buffer = vm->set_pod<layer::renderers::local_reflections::gloss_buffer>("local_reflections_gloss_buffer");
				vlocal_reflections_gloss_buffer->set_property<layer::renderers::local_reflections::gloss_buffer>("float padding", &layer::renderers::local_reflections::gloss_buffer::padding);
				vlocal_reflections_gloss_buffer->set_property<layer::renderers::local_reflections::gloss_buffer>("float deadzone", &layer::renderers::local_reflections::gloss_buffer::deadzone);
				vlocal_reflections_gloss_buffer->set_property<layer::renderers::local_reflections::gloss_buffer>("float mips", &layer::renderers::local_reflections::gloss_buffer::mips);
				vlocal_reflections_gloss_buffer->set_property<layer::renderers::local_reflections::gloss_buffer>("float cutoff", &layer::renderers::local_reflections::gloss_buffer::cutoff);
				vlocal_reflections_gloss_buffer->set_property_array<layer::renderers::local_reflections::gloss_buffer>("float texel", &layer::renderers::local_reflections::gloss_buffer::texel, 2);
				vlocal_reflections_gloss_buffer->set_property<layer::renderers::local_reflections::gloss_buffer>("float samples", &layer::renderers::local_reflections::gloss_buffer::samples);
				vlocal_reflections_gloss_buffer->set_property<layer::renderers::local_reflections::gloss_buffer>("float blur", &layer::renderers::local_reflections::gloss_buffer::blur);

				auto VSSR = vm->set_class<layer::renderers::local_reflections>("local_reflections_renderer", false);
				VSSR->set_property<layer::renderers::local_reflections>("local_reflections_reflectance_buffer reflectance", &layer::renderers::local_reflections::reflectance);
				VSSR->set_property<layer::renderers::local_reflections>("local_reflections_gloss_buffer gloss", &layer::renderers::local_reflections::gloss);
				populate_renderer_interface<layer::renderers::local_reflections, layer::render_system*>(*VSSR, "local_reflections_renderer@+ f(render_system@+)");

				auto vlocal_illumination_stochastic_buffer = vm->set_pod<layer::renderers::local_illumination::stochastic_buffer>("local_illumination_stochastic_buffer");
				vlocal_illumination_stochastic_buffer->set_property_array<layer::renderers::local_illumination::stochastic_buffer>("float texel", &layer::renderers::local_illumination::stochastic_buffer::texel, 2);
				vlocal_illumination_stochastic_buffer->set_property<layer::renderers::local_illumination::stochastic_buffer>("float frame_id", &layer::renderers::local_illumination::stochastic_buffer::frame_id);
				vlocal_illumination_stochastic_buffer->set_property<layer::renderers::local_illumination::stochastic_buffer>("float padding", &layer::renderers::local_illumination::stochastic_buffer::padding);

				auto vlocal_illumination_indirection_buffer = vm->set_pod<layer::renderers::local_illumination::indirection_buffer>("local_illumination_indirection_buffer");
				vlocal_illumination_indirection_buffer->set_property_array<layer::renderers::local_illumination::indirection_buffer>("float random", &layer::renderers::local_illumination::indirection_buffer::random, 2);
				vlocal_illumination_indirection_buffer->set_property<layer::renderers::local_illumination::indirection_buffer>("float samples", &layer::renderers::local_illumination::indirection_buffer::samples);
				vlocal_illumination_indirection_buffer->set_property<layer::renderers::local_illumination::indirection_buffer>("float distance", &layer::renderers::local_illumination::indirection_buffer::distance);
				vlocal_illumination_indirection_buffer->set_property<layer::renderers::local_illumination::indirection_buffer>("float initial", &layer::renderers::local_illumination::indirection_buffer::initial);
				vlocal_illumination_indirection_buffer->set_property<layer::renderers::local_illumination::indirection_buffer>("float cutoff", &layer::renderers::local_illumination::indirection_buffer::cutoff);
				vlocal_illumination_indirection_buffer->set_property<layer::renderers::local_illumination::indirection_buffer>("float attenuations", &layer::renderers::local_illumination::indirection_buffer::attenuation);
				vlocal_illumination_indirection_buffer->set_property<layer::renderers::local_illumination::indirection_buffer>("float swing", &layer::renderers::local_illumination::indirection_buffer::swing);
				vlocal_illumination_indirection_buffer->set_property_array<layer::renderers::local_illumination::indirection_buffer>("float padding", &layer::renderers::local_illumination::indirection_buffer::padding, 3);
				vlocal_illumination_indirection_buffer->set_property<layer::renderers::local_illumination::indirection_buffer>("float bias", &layer::renderers::local_illumination::indirection_buffer::bias);

				auto vlocal_illumination_denoise_buffer = vm->set_pod<layer::renderers::local_illumination::denoise_buffer>("local_illumination_denoise_buffer");
				vlocal_illumination_denoise_buffer->set_property_array<layer::renderers::local_illumination::denoise_buffer>("float padding", &layer::renderers::local_illumination::denoise_buffer::padding, 3);
				vlocal_illumination_denoise_buffer->set_property<layer::renderers::local_illumination::denoise_buffer>("float cutoff", &layer::renderers::local_illumination::denoise_buffer::cutoff);
				vlocal_illumination_denoise_buffer->set_property_array<layer::renderers::local_illumination::denoise_buffer>("float texel", &layer::renderers::local_illumination::denoise_buffer::texel, 2);
				vlocal_illumination_denoise_buffer->set_property<layer::renderers::local_illumination::denoise_buffer>("float samples", &layer::renderers::local_illumination::denoise_buffer::samples);
				vlocal_illumination_denoise_buffer->set_property<layer::renderers::local_illumination::denoise_buffer>("float blur", &layer::renderers::local_illumination::denoise_buffer::blur);

				auto Vlocal_illumination = vm->set_class<layer::renderers::local_illumination>("local_illumination_renderer", false);
				Vlocal_illumination->set_property<layer::renderers::local_illumination>("local_illumination_stochastic_buffer stochastic", &layer::renderers::local_illumination::stochastic);
				Vlocal_illumination->set_property<layer::renderers::local_illumination>("local_illumination_indirection_buffer indirection", &layer::renderers::local_illumination::indirection);
				Vlocal_illumination->set_property<layer::renderers::local_illumination>("local_illumination_denoise_buffer denoise", &layer::renderers::local_illumination::indirection);
				Vlocal_illumination->set_property<layer::renderers::local_illumination>("uint32 bounces", &layer::renderers::local_illumination::bounces);
				populate_renderer_interface<layer::renderers::local_illumination, layer::render_system*>(*Vlocal_illumination, "local_illumination_renderer@+ f(render_system@+)");

				auto vlocal_ambient_shading_buffer = vm->set_pod<layer::renderers::local_ambient::shading_buffer>("local_ambient_shading_buffer");
				vlocal_ambient_shading_buffer->set_property<layer::renderers::local_ambient::shading_buffer>("float samples", &layer::renderers::local_ambient::shading_buffer::samples);
				vlocal_ambient_shading_buffer->set_property<layer::renderers::local_ambient::shading_buffer>("float intensity", &layer::renderers::local_ambient::shading_buffer::intensity);
				vlocal_ambient_shading_buffer->set_property<layer::renderers::local_ambient::shading_buffer>("float scale", &layer::renderers::local_ambient::shading_buffer::scale);
				vlocal_ambient_shading_buffer->set_property<layer::renderers::local_ambient::shading_buffer>("float bias", &layer::renderers::local_ambient::shading_buffer::bias);
				vlocal_ambient_shading_buffer->set_property<layer::renderers::local_ambient::shading_buffer>("float radius", &layer::renderers::local_ambient::shading_buffer::radius);
				vlocal_ambient_shading_buffer->set_property<layer::renderers::local_ambient::shading_buffer>("float distance", &layer::renderers::local_ambient::shading_buffer::distance);
				vlocal_ambient_shading_buffer->set_property<layer::renderers::local_ambient::shading_buffer>("float fade", &layer::renderers::local_ambient::shading_buffer::fade);
				vlocal_ambient_shading_buffer->set_property<layer::renderers::local_ambient::shading_buffer>("float padding", &layer::renderers::local_ambient::shading_buffer::padding);

				auto vlocal_ambient_fibo_buffer = vm->set_pod<layer::renderers::local_ambient::fibo_buffer>("local_ambient_fibo_buffer");
				vlocal_ambient_fibo_buffer->set_property_array<layer::renderers::local_ambient::fibo_buffer>("float padding", &layer::renderers::local_ambient::fibo_buffer::padding, 3);
				vlocal_ambient_fibo_buffer->set_property<layer::renderers::local_ambient::fibo_buffer>("float power", &layer::renderers::local_ambient::fibo_buffer::power);
				vlocal_ambient_fibo_buffer->set_property_array<layer::renderers::local_ambient::fibo_buffer>("float texel", &layer::renderers::local_ambient::fibo_buffer::texel, 2);
				vlocal_ambient_fibo_buffer->set_property<layer::renderers::local_ambient::fibo_buffer>("float samples", &layer::renderers::local_ambient::fibo_buffer::samples);
				vlocal_ambient_fibo_buffer->set_property<layer::renderers::local_ambient::fibo_buffer>("float blur", &layer::renderers::local_ambient::fibo_buffer::blur);

				auto Vlocal_ambient = vm->set_class<layer::renderers::local_ambient>("local_ambient_renderer", false);
				Vlocal_ambient->set_property<layer::renderers::local_ambient>("local_ambient_shading_buffer shading", &layer::renderers::local_ambient::shading);
				Vlocal_ambient->set_property<layer::renderers::local_ambient>("local_ambient_fibo_buffer fibo", &layer::renderers::local_ambient::fibo);
				populate_renderer_interface<layer::renderers::local_ambient, layer::render_system*>(*Vlocal_ambient, "local_ambient_renderer@+ f(render_system@+)");

				auto vdepth_of_fieldfocus_buffer = vm->set_pod<layer::renderers::depth_of_field::focus_buffer>("depth_of_field_focus_buffer");
				vdepth_of_fieldfocus_buffer->set_property_array<layer::renderers::depth_of_field::focus_buffer>("float texel", &layer::renderers::depth_of_field::focus_buffer::texel, 2);
				vdepth_of_fieldfocus_buffer->set_property<layer::renderers::depth_of_field::focus_buffer>("float radius", &layer::renderers::depth_of_field::focus_buffer::radius);
				vdepth_of_fieldfocus_buffer->set_property<layer::renderers::depth_of_field::focus_buffer>("float bokeh", &layer::renderers::depth_of_field::focus_buffer::bokeh);
				vdepth_of_fieldfocus_buffer->set_property_array<layer::renderers::depth_of_field::focus_buffer>("float padding", &layer::renderers::depth_of_field::focus_buffer::padding, 3);
				vdepth_of_fieldfocus_buffer->set_property<layer::renderers::depth_of_field::focus_buffer>("float scale", &layer::renderers::depth_of_field::focus_buffer::scale);
				vdepth_of_fieldfocus_buffer->set_property<layer::renderers::depth_of_field::focus_buffer>("float near_distance", &layer::renderers::depth_of_field::focus_buffer::near_distance);
				vdepth_of_fieldfocus_buffer->set_property<layer::renderers::depth_of_field::focus_buffer>("float near_range", &layer::renderers::depth_of_field::focus_buffer::near_range);
				vdepth_of_fieldfocus_buffer->set_property<layer::renderers::depth_of_field::focus_buffer>("float far_distance", &layer::renderers::depth_of_field::focus_buffer::far_distance);
				vdepth_of_fieldfocus_buffer->set_property<layer::renderers::depth_of_field::focus_buffer>("float far_range", &layer::renderers::depth_of_field::focus_buffer::far_range);

				auto vdepth_of_field = vm->set_class<layer::renderers::depth_of_field>("depth_of_field_renderer", false);
				vdepth_of_field->set_property<layer::renderers::depth_of_field>("depth_of_field_focus_buffer focus", &layer::renderers::depth_of_field::focus);
				vdepth_of_field->set_property<layer::renderers::depth_of_field>("float distance", &layer::renderers::depth_of_field::distance);
				vdepth_of_field->set_property<layer::renderers::depth_of_field>("float radius", &layer::renderers::depth_of_field::radius);
				vdepth_of_field->set_property<layer::renderers::depth_of_field>("float time", &layer::renderers::depth_of_field::time);
				vdepth_of_field->set_method("void focus_at_nearest_target(float)", &layer::renderers::depth_of_field::focus_at_nearest_target);
				populate_renderer_interface<layer::renderers::depth_of_field, layer::render_system*>(*vdepth_of_field, "depth_of_field_renderer@+ f(render_system@+)");

				auto vmotion_blur_velocity_buffer = vm->set_pod<layer::renderers::motion_blur::velocity_buffer>("motion_blur_velocity_buffer");
				vmotion_blur_velocity_buffer->set_property_array<layer::renderers::motion_blur::velocity_buffer>("float padding", &layer::renderers::motion_blur::velocity_buffer::padding, 2);
				vmotion_blur_velocity_buffer->set_property<layer::renderers::motion_blur::velocity_buffer>("float power", &layer::renderers::motion_blur::velocity_buffer::power);
				vmotion_blur_velocity_buffer->set_property<layer::renderers::motion_blur::velocity_buffer>("float threshold", &layer::renderers::motion_blur::velocity_buffer::threshold);

				auto vmotion_blur_motion_buffer = vm->set_pod<layer::renderers::motion_blur::motion_buffer>("motion_blur_motion_buffer");
				vmotion_blur_motion_buffer->set_property_array<layer::renderers::motion_blur::motion_buffer>("float texel", &layer::renderers::motion_blur::motion_buffer::texel, 2);
				vmotion_blur_motion_buffer->set_property<layer::renderers::motion_blur::motion_buffer>("float samples", &layer::renderers::motion_blur::motion_buffer::samples);
				vmotion_blur_motion_buffer->set_property<layer::renderers::motion_blur::motion_buffer>("float motion", &layer::renderers::motion_blur::motion_buffer::motion);

				auto vmotion_blur = vm->set_class<layer::renderers::motion_blur>("motion_blur_renderer", false);
				vmotion_blur->set_property<layer::renderers::motion_blur>("motion_blur_velocity_buffer velocity", &layer::renderers::motion_blur::velocity);
				vmotion_blur->set_property<layer::renderers::motion_blur>("motion_blur_motion_buffer motion", &layer::renderers::motion_blur::motion);
				populate_renderer_interface<layer::renderers::motion_blur, layer::render_system*>(*vmotion_blur, "motion_blur_renderer@+ f(render_system@+)");

				auto vbloom_extraction_buffer = vm->set_pod<layer::renderers::bloom::extraction_buffer>("bloom_extraction_buffer");
				vbloom_extraction_buffer->set_property_array<layer::renderers::bloom::extraction_buffer>("float padding", &layer::renderers::bloom::extraction_buffer::padding, 2);
				vbloom_extraction_buffer->set_property<layer::renderers::bloom::extraction_buffer>("float intensity", &layer::renderers::bloom::extraction_buffer::intensity);
				vbloom_extraction_buffer->set_property<layer::renderers::bloom::extraction_buffer>("float threshold", &layer::renderers::bloom::extraction_buffer::threshold);

				auto vbloom_fibo_buffer = vm->set_pod<layer::renderers::bloom::fibo_buffer>("bloom_fibo_buffer");
				vbloom_fibo_buffer->set_property_array<layer::renderers::bloom::fibo_buffer>("float padding", &layer::renderers::bloom::fibo_buffer::padding, 3);
				vbloom_fibo_buffer->set_property<layer::renderers::bloom::fibo_buffer>("float power", &layer::renderers::bloom::fibo_buffer::power);
				vbloom_fibo_buffer->set_property_array<layer::renderers::bloom::fibo_buffer>("float texel", &layer::renderers::bloom::fibo_buffer::texel, 2);
				vbloom_fibo_buffer->set_property<layer::renderers::bloom::fibo_buffer>("float samples", &layer::renderers::bloom::fibo_buffer::samples);
				vbloom_fibo_buffer->set_property<layer::renderers::bloom::fibo_buffer>("float blur", &layer::renderers::bloom::fibo_buffer::blur);

				auto vbloom = vm->set_class<layer::renderers::bloom>("bloom_renderer", false);
				vbloom->set_property<layer::renderers::bloom>("bloom_extraction_buffer extraction", &layer::renderers::bloom::extraction);
				vbloom->set_property<layer::renderers::bloom>("bloom_fibo_buffer fibo", &layer::renderers::bloom::fibo);
				populate_renderer_interface<layer::renderers::bloom, layer::render_system*>(*vbloom, "bloom_renderer@+ f(render_system@+)");

				auto vtone_luminance_buffer = vm->set_pod<layer::renderers::tone::luminance_buffer>("tone_luminance_buffer");
				vtone_luminance_buffer->set_property_array<layer::renderers::tone::luminance_buffer>("float texel", &layer::renderers::tone::luminance_buffer::texel, 2);
				vtone_luminance_buffer->set_property<layer::renderers::tone::luminance_buffer>("float mips", &layer::renderers::tone::luminance_buffer::mips);
				vtone_luminance_buffer->set_property<layer::renderers::tone::luminance_buffer>("float time", &layer::renderers::tone::luminance_buffer::time);

				auto vtone_mapping_buffer = vm->set_pod<layer::renderers::tone::mapping_buffer>("tone_mapping_buffer");
				vtone_mapping_buffer->set_property_array<layer::renderers::tone::mapping_buffer>("float padding", &layer::renderers::tone::mapping_buffer::padding, 2);
				vtone_mapping_buffer->set_property<layer::renderers::tone::mapping_buffer>("float grayscale", &layer::renderers::tone::mapping_buffer::grayscale);
				vtone_mapping_buffer->set_property<layer::renderers::tone::mapping_buffer>("float aces", &layer::renderers::tone::mapping_buffer::aces);
				vtone_mapping_buffer->set_property<layer::renderers::tone::mapping_buffer>("float filmic", &layer::renderers::tone::mapping_buffer::filmic);
				vtone_mapping_buffer->set_property<layer::renderers::tone::mapping_buffer>("float lottes", &layer::renderers::tone::mapping_buffer::lottes);
				vtone_mapping_buffer->set_property<layer::renderers::tone::mapping_buffer>("float reinhard", &layer::renderers::tone::mapping_buffer::reinhard);
				vtone_mapping_buffer->set_property<layer::renderers::tone::mapping_buffer>("float reinhard2", &layer::renderers::tone::mapping_buffer::reinhard2);
				vtone_mapping_buffer->set_property<layer::renderers::tone::mapping_buffer>("float unreal", &layer::renderers::tone::mapping_buffer::unreal);
				vtone_mapping_buffer->set_property<layer::renderers::tone::mapping_buffer>("float uchimura", &layer::renderers::tone::mapping_buffer::uchimura);
				vtone_mapping_buffer->set_property<layer::renderers::tone::mapping_buffer>("float ubrightness", &layer::renderers::tone::mapping_buffer::ubrightness);
				vtone_mapping_buffer->set_property<layer::renderers::tone::mapping_buffer>("float ucontrast", &layer::renderers::tone::mapping_buffer::ucontrast);
				vtone_mapping_buffer->set_property<layer::renderers::tone::mapping_buffer>("float ustart", &layer::renderers::tone::mapping_buffer::ustart);
				vtone_mapping_buffer->set_property<layer::renderers::tone::mapping_buffer>("float ulength", &layer::renderers::tone::mapping_buffer::ulength);
				vtone_mapping_buffer->set_property<layer::renderers::tone::mapping_buffer>("float ublack", &layer::renderers::tone::mapping_buffer::ublack);
				vtone_mapping_buffer->set_property<layer::renderers::tone::mapping_buffer>("float upedestal", &layer::renderers::tone::mapping_buffer::upedestal);
				vtone_mapping_buffer->set_property<layer::renderers::tone::mapping_buffer>("float exposure", &layer::renderers::tone::mapping_buffer::exposure);
				vtone_mapping_buffer->set_property<layer::renderers::tone::mapping_buffer>("float eintensity", &layer::renderers::tone::mapping_buffer::eintensity);
				vtone_mapping_buffer->set_property<layer::renderers::tone::mapping_buffer>("float egamma", &layer::renderers::tone::mapping_buffer::egamma);
				vtone_mapping_buffer->set_property<layer::renderers::tone::mapping_buffer>("float adaptation", &layer::renderers::tone::mapping_buffer::adaptation);
				vtone_mapping_buffer->set_property<layer::renderers::tone::mapping_buffer>("float ahray", &layer::renderers::tone::mapping_buffer::agray);
				vtone_mapping_buffer->set_property<layer::renderers::tone::mapping_buffer>("float awhite", &layer::renderers::tone::mapping_buffer::awhite);
				vtone_mapping_buffer->set_property<layer::renderers::tone::mapping_buffer>("float ablack", &layer::renderers::tone::mapping_buffer::ablack);
				vtone_mapping_buffer->set_property<layer::renderers::tone::mapping_buffer>("float aspeed", &layer::renderers::tone::mapping_buffer::aspeed);

				auto vtone = vm->set_class<layer::renderers::tone>("tone_renderer", false);
				vtone->set_property<layer::renderers::tone>("tone_luminance_buffer luminance", &layer::renderers::tone::luminance);
				vtone->set_property<layer::renderers::tone>("tone_mapping_buffer mapping", &layer::renderers::tone::mapping);
				populate_renderer_interface<layer::renderers::tone, layer::render_system*>(*vtone, "tone_renderer@+ f(render_system@+)");

				auto vglitch_distortion_buffer = vm->set_pod<layer::renderers::glitch::distortion_buffer>("glitch_distortion_buffer");
				vglitch_distortion_buffer->set_property<layer::renderers::glitch::distortion_buffer>("float scan_line_jitter_displacement", &layer::renderers::glitch::distortion_buffer::scan_line_jitter_displacement);
				vglitch_distortion_buffer->set_property<layer::renderers::glitch::distortion_buffer>("float scan_line_jitter_threshold", &layer::renderers::glitch::distortion_buffer::scan_line_jitter_threshold);
				vglitch_distortion_buffer->set_property<layer::renderers::glitch::distortion_buffer>("float vertical_jump_amount", &layer::renderers::glitch::distortion_buffer::vertical_jump_amount);
				vglitch_distortion_buffer->set_property<layer::renderers::glitch::distortion_buffer>("float vertical_jump_time", &layer::renderers::glitch::distortion_buffer::vertical_jump_time);
				vglitch_distortion_buffer->set_property<layer::renderers::glitch::distortion_buffer>("float color_drift_amount", &layer::renderers::glitch::distortion_buffer::color_drift_amount);
				vglitch_distortion_buffer->set_property<layer::renderers::glitch::distortion_buffer>("float color_drift_time", &layer::renderers::glitch::distortion_buffer::color_drift_time);
				vglitch_distortion_buffer->set_property<layer::renderers::glitch::distortion_buffer>("float horizontal_shake", &layer::renderers::glitch::distortion_buffer::horizontal_shake);
				vglitch_distortion_buffer->set_property<layer::renderers::glitch::distortion_buffer>("float elapsed_time", &layer::renderers::glitch::distortion_buffer::elapsed_time);

				auto vglitch = vm->set_class<layer::renderers::glitch>("glitch_renderer", false);
				vglitch->set_property<layer::renderers::glitch>("glitch_distortion_buffer distortion", &layer::renderers::glitch::distortion);
				vglitch->set_property<layer::renderers::glitch>("float scan_line_jitter", &layer::renderers::glitch::scan_line_jitter);
				vglitch->set_property<layer::renderers::glitch>("float vertical_jump", &layer::renderers::glitch::vertical_jump);
				vglitch->set_property<layer::renderers::glitch>("float horizontal_shake", &layer::renderers::glitch::horizontal_shake);
				vglitch->set_property<layer::renderers::glitch>("float color_drift", &layer::renderers::glitch::color_drift);
				populate_renderer_interface<layer::renderers::glitch, layer::render_system*>(*vglitch, "glitch_renderer@+ f(render_system@+)");

				auto vuser_interface = vm->set_class<layer::renderers::user_interface>("ui_renderer", false);
				vuser_interface->set_method("ui_context@+ get_context() const", &layer::renderers::user_interface::get_context);
				populate_renderer_interface<layer::renderers::user_interface, layer::render_system*>(*vuser_interface, "ui_renderer@+ f(render_system@+)");

				return true;
#else
				VI_ASSERT(false, "<renderers> is not loaded");
				return false;
#endif
			}
			bool heavy_registry::import_ui(virtual_machine* vm) noexcept
			{
#ifdef VI_BINDINGS
				VI_ASSERT(vm != nullptr, "manager should be set");

				auto vfont_weight = vm->set_enum("ui_font_weight");
				vfont_weight->set_value("automatic", (int)layer::gui::font_weight::any);
				vfont_weight->set_value("bold", (int)layer::gui::font_weight::bold);
				vfont_weight->set_value("normal", (int)layer::gui::font_weight::normal);

				auto vinput_type = vm->set_enum("ui_input_type");
				vinput_type->set_value("keys", (int)layer::gui::input_type::keys);
				vinput_type->set_value("scroll", (int)layer::gui::input_type::scroll);
				vinput_type->set_value("text", (int)layer::gui::input_type::text);
				vinput_type->set_value("cursor", (int)layer::gui::input_type::cursor);
				vinput_type->set_value("any_of", (int)layer::gui::input_type::any);

				auto vcontext = vm->set_class<layer::gui::context>("ui_context", false);
				vcontext->set_constructor<layer::gui::context, const trigonometry::vector2&>("ui_context@ f(const vector2&in)");
				vcontext->set_constructor<layer::gui::context, graphics::graphics_device*>("ui_context@ f(graphics_device@+)");
				vcontext->set_method("void emit_key(key_code, key_mod, int, int, bool)", &layer::gui::context::emit_key);
				vcontext->set_method_ex("void emit_input(const string_view&in)", &context_emit_input);
				vcontext->set_method("void emit_wheel(int32, int32, bool, key_mod)", &layer::gui::context::emit_wheel);
				vcontext->set_method("void emit_resize(int32, int32)", &layer::gui::context::emit_resize);
				vcontext->set_method("void set_documents_base_tag(const string_view&in)", &layer::gui::context::set_documents_base_tag);
				vcontext->set_method("void clear_styles()", &layer::gui::context::clear_styles);
				vcontext->set_method("void update_events(activity@+)", &layer::gui::context::update_events);
				vcontext->set_method("void render_lists(texture_2d@+)", &layer::gui::context::render_lists);
				vcontext->set_method("bool clear_documents()", &layer::gui::context::clear_documents);
				vcontext->set_method_static("bool load_font_face(const string_view&in, bool = false, ui_font_weight = ui_font_weight::automatic)", &VI_SEXPECTIFY_VOID(layer::gui::context::load_font_face));
				vcontext->set_method_static("string resolve_resource_path(const ui_element&in, const string_view&in)", &layer::gui::context::resolve_resource_path);
				vcontext->set_method("bool is_input_focused()", &layer::gui::context::is_input_focused);
				vcontext->set_method("bool is_loading()", &layer::gui::context::is_loading);
				vcontext->set_method("bool was_input_used(uint32)", &layer::gui::context::was_input_used);
				vcontext->set_method("bool replace_html(const string_view&in, const string_view&in, int = 0)", &layer::gui::context::replace_html);
				vcontext->set_method("bool remove_data_model(const string_view&in)", &layer::gui::context::remove_data_model);
				vcontext->set_method("bool remove_data_models()", &layer::gui::context::remove_data_models);
				vcontext->set_method("uint64 calculate_idle_timeout_ms(uint64)", &layer::gui::context::calculate_idle_timeout_ms);
				vcontext->set_method("uptr@ get_context()", &layer::gui::context::get_context);
				vcontext->set_method("vector2 get_dimensions() const", &layer::gui::context::get_dimensions);
				vcontext->set_method("string get_documents_base_tag() const", &layer::gui::context::get_documents_base_tag);
				vcontext->set_method("void set_density_independent_pixel_ratio(float)", &layer::gui::context::get_density_independent_pixel_ratio);
				vcontext->set_method("float get_density_independent_pixel_ratio() const", &layer::gui::context::get_density_independent_pixel_ratio);
				vcontext->set_method("void enable_mouse_cursor(bool)", &layer::gui::context::enable_mouse_cursor);
				vcontext->set_method_ex("ui_document eval_html(const string_view&in, int = 0)", &VI_EXPECTIFY(layer::gui::context::eval_html));
				vcontext->set_method_ex("ui_document add_css(const string_view&in, int = 0)", &VI_EXPECTIFY(layer::gui::context::add_css));
				vcontext->set_method_ex("ui_document load_css(const string_view&in, int = 0)", &VI_EXPECTIFY(layer::gui::context::load_css));
				vcontext->set_method_ex("ui_document load_document(const string_view&in, bool = false)", &VI_EXPECTIFY(layer::gui::context::load_document));
				vcontext->set_method_ex("ui_document add_document(const string_view&in)", &VI_EXPECTIFY(layer::gui::context::add_document));
				vcontext->set_method_ex("ui_document add_document_empty(const string_view&in = \"body\")", &VI_EXPECTIFY(layer::gui::context::add_document_empty));
				vcontext->set_method<layer::gui::context, layer::gui::ielement_document, const std::string_view&>("ui_document get_document(const string_view&in)", &layer::gui::context::get_document);
				vcontext->set_method<layer::gui::context, layer::gui::ielement_document, int>("ui_document get_document(int)", &layer::gui::context::get_document);
				vcontext->set_method("int get_num_documents() const", &layer::gui::context::get_num_documents);
				vcontext->set_method("ui_element get_element_by_id(const string_view&in, int = 0)", &layer::gui::context::get_element_by_id);
				vcontext->set_method("ui_element get_hover_element()", &layer::gui::context::get_hover_element);
				vcontext->set_method("ui_element get_focus_element()", &layer::gui::context::get_focus_element);
				vcontext->set_method("ui_element get_root_element()", &layer::gui::context::get_root_element);
				vcontext->set_method_ex("ui_element get_element_at_point(const vector2 &in)", &context_get_focus_element);
				vcontext->set_method("void pull_document_to_front(const ui_document &in)", &layer::gui::context::pull_document_to_front);
				vcontext->set_method("void push_document_to_back(const ui_document &in)", &layer::gui::context::push_document_to_back);
				vcontext->set_method("void unfocus_document(const ui_document &in)", &layer::gui::context::unfocus_document);
				vcontext->set_method("void add_event_listener(const string_view&in, ui_listener@+, bool = false)", &layer::gui::context::add_event_listener);
				vcontext->set_method("void remove_event_listener(const string_view&in, ui_listener@+, bool = false)", &layer::gui::context::remove_event_listener);
				vcontext->set_method("bool is_mouse_interacting()", &layer::gui::context::is_mouse_interacting);
				vcontext->set_method("ui_model@+ set_model(const string_view&in)", &layer::gui::context::set_data_model);
				vcontext->set_method("ui_model@+ get_model(const string_view&in)", &layer::gui::context::get_data_model);

				return true;
#else
				VI_ASSERT(false, "<ui-context> is not loaded");
				return false;
#endif
			}
			bool heavy_registry::import_ui_control(virtual_machine* vm) noexcept
			{
#ifdef VI_BINDINGS
				VI_ASSERT(vm != nullptr, "manager should be set");
				VI_TYPEREF(model_listener_name, "ui_listener");

				auto vevent_phase = vm->set_enum("ui_event_phase");
				auto varea = vm->set_enum("ui_area");
				auto vdisplay = vm->set_enum("ui_display");
				auto vposition = vm->set_enum("ui_position");
				auto vfloat = vm->set_enum("ui_float");
				auto vtiming_func = vm->set_enum("ui_timing_func");
				auto vtiming_dir = vm->set_enum("ui_timing_dir");
				auto vfocus_flag = vm->set_enum("ui_focus_flag");
				auto vmodal_flag = vm->set_enum("ui_modal_flag");
				auto vnumeric_unit = vm->set_enum("numeric_unit");
				auto velement = vm->set_struct_trivial<layer::gui::ielement>("ui_element");
				auto vdocument = vm->set_struct_trivial<layer::gui::ielement_document>("ui_document");
				auto vevent = vm->set_struct_trivial<layer::gui::ievent>("ui_event");
				auto vlistener = vm->set_class<model_listener>("ui_listener", true);
				vm->set_function_def("void model_listener_event(ui_event &in)");

				vmodal_flag->set_value("none", (int)layer::gui::modal_flag::none);
				vmodal_flag->set_value("modal", (int)layer::gui::modal_flag::modal);
				vmodal_flag->set_value("keep", (int)layer::gui::modal_flag::keep);

				vfocus_flag->set_value("none", (int)layer::gui::focus_flag::none);
				vfocus_flag->set_value("document", (int)layer::gui::focus_flag::document);
				vfocus_flag->set_value("keep", (int)layer::gui::focus_flag::keep);
				vfocus_flag->set_value("automatic", (int)layer::gui::focus_flag::any);

				vevent_phase->set_value("none", (int)layer::gui::event_phase::none);
				vevent_phase->set_value("capture", (int)layer::gui::event_phase::capture);
				vevent_phase->set_value("target", (int)layer::gui::event_phase::target);
				vevent_phase->set_value("bubble", (int)layer::gui::event_phase::bubble);

				vevent->set_constructor<layer::gui::ievent>("void f()");
				vevent->set_constructor<layer::gui::ievent, Rml::Event*>("void f(uptr@)");
				vevent->set_method("ui_event_phase get_phase() const", &layer::gui::ievent::get_phase);
				vevent->set_method("void set_phase(ui_event_phase phase)", &layer::gui::ievent::set_phase);
				vevent->set_method("void set_current_element(const ui_element &in)", &layer::gui::ievent::set_current_element);
				vevent->set_method("ui_element get_current_element() const", &layer::gui::ievent::get_current_element);
				vevent->set_method("ui_element get_target_element() const", &layer::gui::ievent::get_target_element);
				vevent->set_method("string get_type() const", &layer::gui::ievent::get_type);
				vevent->set_method("void stop_propagation()", &layer::gui::ievent::stop_propagation);
				vevent->set_method("void stop_immediate_propagation()", &layer::gui::ievent::stop_immediate_propagation);
				vevent->set_method("bool is_interruptible() const", &layer::gui::ievent::is_interruptible);
				vevent->set_method("bool is_propagating() const", &layer::gui::ievent::is_propagating);
				vevent->set_method("bool is_immediate_propagating() const", &layer::gui::ievent::is_immediate_propagating);
				vevent->set_method("bool get_boolean(const string_view&in) const", &layer::gui::ievent::get_boolean);
				vevent->set_method("int64 get_integer(const string_view&in) const", &layer::gui::ievent::get_integer);
				vevent->set_method("double get_number(const string_view&in) const", &layer::gui::ievent::get_number);
				vevent->set_method("string get_string(const string_view&in) const", &layer::gui::ievent::get_string);
				vevent->set_method("vector2 get_vector2(const string_view&in) const", &layer::gui::ievent::get_vector2);
				vevent->set_method("vector3 get_vector3(const string_view&in) const", &layer::gui::ievent::get_vector3);
				vevent->set_method("vector4 get_vector4(const string_view&in) const", &layer::gui::ievent::get_vector4);
				vevent->set_method("uptr@ get_pointer(const string_view&in) const", &layer::gui::ievent::get_pointer);
				vevent->set_method("uptr@ get_event() const", &layer::gui::ievent::get_event);
				vevent->set_method("bool is_valid() const", &layer::gui::ievent::is_valid);

				vlistener->set_gc_constructor<model_listener, model_listener_name, asIScriptFunction*>("ui_listener@ f(model_listener_event@)");
				vlistener->set_gc_constructor<model_listener, model_listener_name, const std::string_view&>("ui_listener@ f(const string_view&in)");
				vlistener->set_enum_refs_ex<model_listener>([](model_listener* base, asIScriptEngine* vm)
				{
					auto& delegatef = base->get_delegate();
					function_factory::gc_enum_callback(vm, &delegatef);
				});
				vlistener->set_release_refs_ex<model_listener>([](model_listener* base, asIScriptEngine*)
				{
					base->get_delegate().release();
					base->~model_listener();
				});

				varea->set_value("margin", (int)layer::gui::area::margin);
				varea->set_value("border", (int)layer::gui::area::border);
				varea->set_value("padding", (int)layer::gui::area::padding);
				varea->set_value("content", (int)layer::gui::area::content);

				vdisplay->set_value("none", (int)layer::gui::display::none);
				vdisplay->set_value("block", (int)layer::gui::display::block);
				vdisplay->set_value("inline", (int)layer::gui::display::inline_base);
				vdisplay->set_value("inline_block", (int)layer::gui::display::inline_block);
				vdisplay->set_value("flex", (int)layer::gui::display::flex);
				vdisplay->set_value("table", (int)layer::gui::display::table);
				vdisplay->set_value("table_row", (int)layer::gui::display::table_row);
				vdisplay->set_value("table_row_group", (int)layer::gui::display::table_row_group);
				vdisplay->set_value("table_column", (int)layer::gui::display::table_column);
				vdisplay->set_value("table_column_group", (int)layer::gui::display::table_column_group);
				vdisplay->set_value("table_cell", (int)layer::gui::display::table_cell);

				vposition->set_value("static", (int)layer::gui::position::constant);
				vposition->set_value("relative", (int)layer::gui::position::relative);
				vposition->set_value("absolute", (int)layer::gui::position::absolute);
				vposition->set_value("fixed", (int)layer::gui::position::fixed);

				vfloat->set_value("none", (int)layer::gui::floatf::none);
				vfloat->set_value("left", (int)layer::gui::floatf::left);
				vfloat->set_value("right", (int)layer::gui::floatf::right);

				vtiming_func->set_value("none", (int)layer::gui::timing_func::none);
				vtiming_func->set_value("back", (int)layer::gui::timing_func::back);
				vtiming_func->set_value("bounce", (int)layer::gui::timing_func::bounce);
				vtiming_func->set_value("circular", (int)layer::gui::timing_func::circular);
				vtiming_func->set_value("cubic", (int)layer::gui::timing_func::cubic);
				vtiming_func->set_value("elastic", (int)layer::gui::timing_func::elastic);
				vtiming_func->set_value("exponential", (int)layer::gui::timing_func::exponential);
				vtiming_func->set_value("linear", (int)layer::gui::timing_func::linear);
				vtiming_func->set_value("quadratic", (int)layer::gui::timing_func::quadratic);
				vtiming_func->set_value("quartic", (int)layer::gui::timing_func::quartic);
				vtiming_func->set_value("sine", (int)layer::gui::timing_func::sine);
				vtiming_func->set_value("callback", (int)layer::gui::timing_func::callback);

				vtiming_dir->set_value("use_in", (int)layer::gui::timing_dir::in);
				vtiming_dir->set_value("use_out", (int)layer::gui::timing_dir::out);
				vtiming_dir->set_value("use_in_out", (int)layer::gui::timing_dir::in_out);

				vnumeric_unit->set_value("unknown", (int)layer::gui::numeric_unit::unknown);
				vnumeric_unit->set_value("keyword", (int)layer::gui::numeric_unit::keyword);
				vnumeric_unit->set_value("text", (int)layer::gui::numeric_unit::string);
				vnumeric_unit->set_value("colour", (int)layer::gui::numeric_unit::colour);
				vnumeric_unit->set_value("ratio", (int)layer::gui::numeric_unit::ratio);
				vnumeric_unit->set_value("number", (int)layer::gui::numeric_unit::number);
				vnumeric_unit->set_value("percent", (int)layer::gui::numeric_unit::percent);
				vnumeric_unit->set_value("px", (int)layer::gui::numeric_unit::px);
				vnumeric_unit->set_value("dp", (int)layer::gui::numeric_unit::dp);
				vnumeric_unit->set_value("vw", (int)layer::gui::numeric_unit::vw);
				vnumeric_unit->set_value("vh", (int)layer::gui::numeric_unit::vh);
				vnumeric_unit->set_value("x", (int)layer::gui::numeric_unit::x);
				vnumeric_unit->set_value("em", (int)layer::gui::numeric_unit::em);
				vnumeric_unit->set_value("rem", (int)layer::gui::numeric_unit::rem);
				vnumeric_unit->set_value("inch", (int)layer::gui::numeric_unit::inch);
				vnumeric_unit->set_value("cm", (int)layer::gui::numeric_unit::cm);
				vnumeric_unit->set_value("mm", (int)layer::gui::numeric_unit::mm);
				vnumeric_unit->set_value("pt", (int)layer::gui::numeric_unit::pt);
				vnumeric_unit->set_value("pc", (int)layer::gui::numeric_unit::pc);
				vnumeric_unit->set_value("ppi_unit", (int)layer::gui::numeric_unit::ppi_unit);
				vnumeric_unit->set_value("deg", (int)layer::gui::numeric_unit::deg);
				vnumeric_unit->set_value("rad", (int)layer::gui::numeric_unit::rad);
				vnumeric_unit->set_value("transform_unit", (int)layer::gui::numeric_unit::transform);
				vnumeric_unit->set_value("transition", (int)layer::gui::numeric_unit::transition);
				vnumeric_unit->set_value("animation", (int)layer::gui::numeric_unit::animation);
				vnumeric_unit->set_value("decorator", (int)layer::gui::numeric_unit::decorator);
				vnumeric_unit->set_value("font_effect", (int)layer::gui::numeric_unit::fonteffect);
				vnumeric_unit->set_value("colour_stop_list", (int)layer::gui::numeric_unit::colorstoplist);
				vnumeric_unit->set_value("shadow_list", (int)layer::gui::numeric_unit::shadowlist);
				vnumeric_unit->set_value("length", (int)layer::gui::numeric_unit::length);
				vnumeric_unit->set_value("length_percent", (int)layer::gui::numeric_unit::length_percent);
				vnumeric_unit->set_value("number_length_percent", (int)layer::gui::numeric_unit::number_length_percent);
				vnumeric_unit->set_value("dp_scalable_length", (int)layer::gui::numeric_unit::dp_scalable_length);
				vnumeric_unit->set_value("angle", (int)layer::gui::numeric_unit::angle);
				vnumeric_unit->set_value("numeric", (int)layer::gui::numeric_unit::numeric);

				velement->set_constructor<layer::gui::ielement>("void f()");
				velement->set_constructor<layer::gui::ielement, Rml::Element*>("void f(uptr@)");
				velement->set_method("ui_element clone() const", &layer::gui::ielement::clone);
				velement->set_method("void set_class(const string_view&in, bool)", &layer::gui::ielement::set_class);
				velement->set_method("bool is_class_set(const string_view&in) const", &layer::gui::ielement::is_class_set);
				velement->set_method("void set_class_names(const string_view&in)", &layer::gui::ielement::set_class_names);
				velement->set_method("string get_class_names() const", &layer::gui::ielement::get_class_names);
				velement->set_method("string get_address(bool = false, bool = true) const", &layer::gui::ielement::get_address);
				velement->set_method("void set_offset(const vector2 &in, const ui_element &in, bool = false)", &layer::gui::ielement::set_offset);
				velement->set_method("vector2 get_relative_offset(ui_area = ui_area::content) const", &layer::gui::ielement::get_relative_offset);
				velement->set_method("vector2 get_absolute_offset(ui_area = ui_area::content) const", &layer::gui::ielement::get_absolute_offset);
				velement->set_method("void set_bontent_box(const vector2 &in)", &layer::gui::ielement::set_content_box);
				velement->set_method("float get_baseline() const", &layer::gui::ielement::get_baseline);
				velement->set_method("bool get_intrinsic_dimensions(vector2 &out, float &out)", &layer::gui::ielement::get_intrinsic_dimensions);
				velement->set_method("bool is_point_within_element(const vector2 &in)", &layer::gui::ielement::is_point_within_element);
				velement->set_method("bool is_visible() const", &layer::gui::ielement::is_visible);
				velement->set_method("float get_zindex() const", &layer::gui::ielement::get_zindex);
				velement->set_method("bool set_property(const string_view&in, const string_view&in)", &layer::gui::ielement::set_property);
				velement->set_method("void remove_property(const string_view&in)", &layer::gui::ielement::remove_property);
				velement->set_method("string get_property(const string_view&in) const", &layer::gui::ielement::get_property);
				velement->set_method("string get_local_property(const string_view&in) const", &layer::gui::ielement::get_local_property);
				velement->set_method("float resolve_numeric_property(const string_view&in) const", &layer::gui::ielement::resolve_numeric_property);
				velement->set_method("vector2 get_containing_block() const", &layer::gui::ielement::get_containing_block);
				velement->set_method("ui_position get_position() const", &layer::gui::ielement::get_position);
				velement->set_method("ui_float get_float() const", &layer::gui::ielement::get_float);
				velement->set_method("ui_display get_display() const", &layer::gui::ielement::get_display);
				velement->set_method("float get_line_height() const", &layer::gui::ielement::get_line_height);
				velement->set_method("bool project(vector2 &out) const", &layer::gui::ielement::project);
				velement->set_method("bool animate(const string_view&in, const string_view&in, float, ui_timing_func, ui_timing_dir, int = -1, bool = true, float = 0)", &layer::gui::ielement::animate);
				velement->set_method("bool add_animation_key(const string_view&in, const string_view&in, float, ui_timing_func, ui_timing_dir)", &layer::gui::ielement::add_animation_key);
				velement->set_method("void set_pseudo_Class(const string_view&in, bool)", &layer::gui::ielement::set_pseudo_class);
				velement->set_method("bool is_pseudo_class_set(const string_view&in) const", &layer::gui::ielement::is_pseudo_class_set);
				velement->set_method("void set_attribute(const string_view&in, const string_view&in)", &layer::gui::ielement::set_attribute);
				velement->set_method("string get_attribute(const string_view&in) const", &layer::gui::ielement::get_attribute);
				velement->set_method("bool has_attribute(const string_view&in) const", &layer::gui::ielement::has_attribute);
				velement->set_method("void remove_attribute(const string_view&in)", &layer::gui::ielement::remove_attribute);
				velement->set_method("ui_element get_focus_leaf_node()", &layer::gui::ielement::get_focus_leaf_node);
				velement->set_method("string get_tag_name() const", &layer::gui::ielement::get_tag_name);
				velement->set_method("string get_id() const", &layer::gui::ielement::get_id);
				velement->set_method("float get_absolute_left()", &layer::gui::ielement::get_absolute_left);
				velement->set_method("float get_absolute_top()", &layer::gui::ielement::get_absolute_top);
				velement->set_method("float get_client_left()", &layer::gui::ielement::get_client_left);
				velement->set_method("float get_client_top()", &layer::gui::ielement::get_client_top);
				velement->set_method("float get_client_width()", &layer::gui::ielement::get_client_width);
				velement->set_method("float get_client_height()", &layer::gui::ielement::get_client_height);
				velement->set_method("ui_element get_offset_parent()", &layer::gui::ielement::get_offset_parent);
				velement->set_method("float get_offset_left()", &layer::gui::ielement::get_offset_left);
				velement->set_method("float get_offset_top()", &layer::gui::ielement::get_offset_top);
				velement->set_method("float get_offset_width()", &layer::gui::ielement::get_offset_width);
				velement->set_method("float get_offset_height()", &layer::gui::ielement::get_offset_height);
				velement->set_method("float get_scroll_left()", &layer::gui::ielement::get_scroll_left);
				velement->set_method("void set_scroll_left(float)", &layer::gui::ielement::set_scroll_left);
				velement->set_method("float get_scroll_top()", &layer::gui::ielement::get_scroll_top);
				velement->set_method("void set_scroll_top(float)", &layer::gui::ielement::set_scroll_top);
				velement->set_method("float get_scroll_width()", &layer::gui::ielement::get_scroll_width);
				velement->set_method("float get_scroll_height()", &layer::gui::ielement::get_scroll_height);
				velement->set_method("ui_document get_owner_document() const", &layer::gui::ielement::get_owner_document);
				velement->set_method("ui_element get_parent_node() const", &layer::gui::ielement::get_parent_node);
				velement->set_method("ui_element get_next_sibling() const", &layer::gui::ielement::get_next_sibling);
				velement->set_method("ui_element get_previous_sibling() const", &layer::gui::ielement::get_previous_sibling);
				velement->set_method("ui_element get_first_child() const", &layer::gui::ielement::get_first_child);
				velement->set_method("ui_element get_last_child() const", &layer::gui::ielement::get_last_child);
				velement->set_method("ui_element get_child(int) const", &layer::gui::ielement::get_child);
				velement->set_method("int get_num_children(bool = false) const", &layer::gui::ielement::get_num_children);
				velement->set_method<layer::gui::ielement, void, core::string&>("void get_inner_html(string &out) const", &layer::gui::ielement::get_inner_html);
				velement->set_method<layer::gui::ielement, core::string>("string get_inner_html() const", &layer::gui::ielement::get_inner_html);
				velement->set_method("void set_inner_html(const string_view&in)", &layer::gui::ielement::set_inner_html);
				velement->set_method("bool is_focused()", &layer::gui::ielement::is_focused);
				velement->set_method("bool is_hovered()", &layer::gui::ielement::is_hovered);
				velement->set_method("bool is_active()", &layer::gui::ielement::is_active);
				velement->set_method("bool is_checked()", &layer::gui::ielement::is_checked);
				velement->set_method("bool focus()", &layer::gui::ielement::focus);
				velement->set_method("void blur()", &layer::gui::ielement::blur);
				velement->set_method("void click()", &layer::gui::ielement::click);
				velement->set_method("void add_event_listener(const string_view&in, ui_listener@+, bool = false)", &layer::gui::ielement::add_event_listener);
				velement->set_method("void remove_event_listener(const string_view&in, ui_listener@+, bool = false)", &layer::gui::ielement::remove_event_listener);
				velement->set_method_ex("bool dispatch_event(const string_view&in, schema@+)", &ielement_document_dispatch_event);
				velement->set_method("void scroll_into_view(bool = true)", &layer::gui::ielement::scroll_into_view);
				velement->set_method("ui_element append_child(const ui_element &in, bool = true)", &layer::gui::ielement::append_child);
				velement->set_method("ui_element insert_before(const ui_element &in, const ui_element &in)", &layer::gui::ielement::insert_before);
				velement->set_method("ui_element replace_child(const ui_element &in, const ui_element &in)", &layer::gui::ielement::replace_child);
				velement->set_method("ui_element remove_child(const ui_element &in)", &layer::gui::ielement::remove_child);
				velement->set_method("bool has_child_nodes() const", &layer::gui::ielement::has_child_nodes);
				velement->set_method("ui_element get_element_by_id(const string_view&in)", &layer::gui::ielement::get_element_by_id);
				velement->set_method_ex("array<ui_element>@ query_selector_all(const string_view&in)", &ielement_document_query_selector_all);
				velement->set_method("bool cast_form_color(vector4 &out, bool)", &layer::gui::ielement::cast_form_color);
				velement->set_method("bool cast_form_string(string &out)", &layer::gui::ielement::cast_form_string);
				velement->set_method("bool cast_form_pointer(uptr@ &out)", &layer::gui::ielement::cast_form_pointer);
				velement->set_method("bool cast_form_int32(int &out)", &layer::gui::ielement::cast_form_int32);
				velement->set_method("bool cast_form_uint32(uint &out)", &layer::gui::ielement::cast_form_uint32);
				velement->set_method("bool cast_form_flag32(uint &out, uint)", &layer::gui::ielement::cast_form_flag32);
				velement->set_method("bool cast_form_int64(int64 &out)", &layer::gui::ielement::cast_form_int64);
				velement->set_method("bool cast_form_uint64(uint64 &out)", &layer::gui::ielement::cast_form_uint64);
				velement->set_method("bool cast_form_flag64(uint64 &out, uint64)", &layer::gui::ielement::cast_form_flag64);
				velement->set_method<layer::gui::ielement, bool, float*>("bool cast_form_float(float &out)", &layer::gui::ielement::cast_form_float);
				velement->set_method<layer::gui::ielement, bool, float*, float>("bool cast_form_float(float &out, float)", &layer::gui::ielement::cast_form_float);
				velement->set_method("bool cast_form_double(double &out)", &layer::gui::ielement::cast_form_double);
				velement->set_method("bool cast_form_boolean(bool &out)", &layer::gui::ielement::cast_form_boolean);
				velement->set_method("string get_form_name() const", &layer::gui::ielement::get_form_name);
				velement->set_method("void set_form_name(const string_view&in)", &layer::gui::ielement::set_form_name);
				velement->set_method("string get_form_value() const", &layer::gui::ielement::get_form_value);
				velement->set_method("void set_form_value(const string_view&in)", &layer::gui::ielement::set_form_value);
				velement->set_method("bool is_form_disabled() const", &layer::gui::ielement::is_form_disabled);
				velement->set_method("void set_form_disabled(bool)", &layer::gui::ielement::set_form_disabled);
				velement->set_method("uptr@ get_element() const", &layer::gui::ielement::get_element);
				velement->set_method("bool is_valid() const", &layer::gui::ielement::is_valid);

				vdocument->set_constructor<layer::gui::ielement_document>("void f()");
				vdocument->set_constructor<layer::gui::ielement_document, Rml::ElementDocument*>("void f(uptr@)");
				vdocument->set_method("ui_element clone() const", &layer::gui::ielement_document::clone);
				vdocument->set_method("void set_class(const string_view&in, bool)", &layer::gui::ielement_document::set_class);
				vdocument->set_method("bool is_class_set(const string_view&in) const", &layer::gui::ielement_document::is_class_set);
				vdocument->set_method("void set_class_names(const string_view&in)", &layer::gui::ielement_document::set_class_names);
				vdocument->set_method("string get_class_names() const", &layer::gui::ielement_document::get_class_names);
				vdocument->set_method("string get_address(bool = false, bool = true) const", &layer::gui::ielement_document::get_address);
				vdocument->set_method("void set_offset(const vector2 &in, const ui_element &in, bool = false)", &layer::gui::ielement_document::set_offset);
				vdocument->set_method("vector2 get_relative_offset(ui_area = ui_area::content) const", &layer::gui::ielement_document::get_relative_offset);
				vdocument->set_method("vector2 get_absolute_offset(ui_area = ui_area::content) const", &layer::gui::ielement_document::get_absolute_offset);
				vdocument->set_method("void set_bontent_box(const vector2 &in, const vector2 &in)", &layer::gui::ielement_document::set_content_box);
				vdocument->set_method("float get_baseline() const", &layer::gui::ielement_document::get_baseline);
				vdocument->set_method("bool get_intrinsic_dimensions(vector2 &out, float &out)", &layer::gui::ielement_document::get_intrinsic_dimensions);
				vdocument->set_method("bool is_point_within_element(const vector2 &in)", &layer::gui::ielement_document::is_point_within_element);
				vdocument->set_method("bool is_visible() const", &layer::gui::ielement_document::is_visible);
				vdocument->set_method("float get_zindex() const", &layer::gui::ielement_document::get_zindex);
				vdocument->set_method("bool set_property(const string_view&in, const string_view&in)", &layer::gui::ielement_document::set_property);
				vdocument->set_method("void remove_property(const string_view&in)", &layer::gui::ielement_document::remove_property);
				vdocument->set_method("string get_property(const string_view&in) const", &layer::gui::ielement_document::get_property);
				vdocument->set_method("string get_local_property(const string_view&in) const", &layer::gui::ielement_document::get_local_property);
				vdocument->set_method("float resolve_numeric_property(float, numeric_unit, float) const", &layer::gui::ielement_document::resolve_numeric_property);
				vdocument->set_method("vector2 get_containing_block() const", &layer::gui::ielement_document::get_containing_block);
				vdocument->set_method("ui_position get_position() const", &layer::gui::ielement_document::get_position);
				vdocument->set_method("ui_float get_float() const", &layer::gui::ielement_document::get_float);
				vdocument->set_method("ui_display get_display() const", &layer::gui::ielement_document::get_display);
				vdocument->set_method("float get_line_height() const", &layer::gui::ielement_document::get_line_height);
				vdocument->set_method("bool project(vector2 &out) const", &layer::gui::ielement_document::project);
				vdocument->set_method("bool animate(const string_view&in, const string_view&in, float, ui_timing_func, ui_timing_dir, int = -1, bool = true, float = 0)", &layer::gui::ielement_document::animate);
				vdocument->set_method("bool add_animation_key(const string_view&in, const string_view&in, float, ui_timing_func, ui_timing_dir)", &layer::gui::ielement_document::add_animation_key);
				vdocument->set_method("void set_pseudo_Class(const string_view&in, bool)", &layer::gui::ielement_document::set_pseudo_class);
				vdocument->set_method("bool is_pseudo_class_set(const string_view&in) const", &layer::gui::ielement_document::is_pseudo_class_set);
				vdocument->set_method("void set_attribute(const string_view&in, const string_view&in)", &layer::gui::ielement_document::set_attribute);
				vdocument->set_method("string get_attribute(const string_view&in) const", &layer::gui::ielement_document::get_attribute);
				vdocument->set_method("bool has_attribute(const string_view&in) const", &layer::gui::ielement_document::has_attribute);
				vdocument->set_method("void remove_attribute(const string_view&in)", &layer::gui::ielement_document::remove_attribute);
				vdocument->set_method("ui_element get_focus_leaf_node()", &layer::gui::ielement_document::get_focus_leaf_node);
				vdocument->set_method("string get_tag_name() const", &layer::gui::ielement_document::get_tag_name);
				vdocument->set_method("string get_id() const", &layer::gui::ielement_document::get_id);
				vdocument->set_method("float get_absolute_left()", &layer::gui::ielement_document::get_absolute_left);
				vdocument->set_method("float get_absolute_top()", &layer::gui::ielement_document::get_absolute_top);
				vdocument->set_method("float get_client_left()", &layer::gui::ielement_document::get_client_left);
				vdocument->set_method("float get_client_top()", &layer::gui::ielement_document::get_client_top);
				vdocument->set_method("float get_client_width()", &layer::gui::ielement_document::get_client_width);
				vdocument->set_method("float get_client_height()", &layer::gui::ielement_document::get_client_height);
				vdocument->set_method("ui_element get_offset_parent()", &layer::gui::ielement_document::get_offset_parent);
				vdocument->set_method("float get_offset_left()", &layer::gui::ielement_document::get_offset_left);
				vdocument->set_method("float get_offset_top()", &layer::gui::ielement_document::get_offset_top);
				vdocument->set_method("float get_offset_width()", &layer::gui::ielement_document::get_offset_width);
				vdocument->set_method("float get_offset_height()", &layer::gui::ielement_document::get_offset_height);
				vdocument->set_method("float get_scroll_left()", &layer::gui::ielement_document::get_scroll_left);
				vdocument->set_method("void set_scroll_left(float)", &layer::gui::ielement_document::set_scroll_left);
				vdocument->set_method("float get_scroll_top()", &layer::gui::ielement_document::get_scroll_top);
				vdocument->set_method("void set_scroll_top(float)", &layer::gui::ielement_document::set_scroll_top);
				vdocument->set_method("float get_scroll_width()", &layer::gui::ielement_document::get_scroll_width);
				vdocument->set_method("float get_scroll_height()", &layer::gui::ielement_document::get_scroll_height);
				vdocument->set_method("ui_document get_owner_document() const", &layer::gui::ielement_document::get_owner_document);
				vdocument->set_method("ui_element get_parent_node() const", &layer::gui::ielement_document::get_parent_node);
				vdocument->set_method("ui_element get_next_sibling() const", &layer::gui::ielement_document::get_next_sibling);
				vdocument->set_method("ui_element get_previous_sibling() const", &layer::gui::ielement_document::get_previous_sibling);
				vdocument->set_method("ui_element get_first_child() const", &layer::gui::ielement_document::get_first_child);
				vdocument->set_method("ui_element get_last_child() const", &layer::gui::ielement_document::get_last_child);
				vdocument->set_method("ui_element get_child(int) const", &layer::gui::ielement_document::get_child);
				vdocument->set_method("int get_num_children(bool = false) const", &layer::gui::ielement_document::get_num_children);
				vdocument->set_method<layer::gui::ielement, void, core::string&>("void get_inner_html(string &out) const", &layer::gui::ielement_document::get_inner_html);
				vdocument->set_method<layer::gui::ielement, core::string>("string get_inner_html() const", &layer::gui::ielement_document::get_inner_html);
				vdocument->set_method("void set_inner_html(const string_view&in)", &layer::gui::ielement_document::set_inner_html);
				vdocument->set_method("bool is_focused()", &layer::gui::ielement_document::is_focused);
				vdocument->set_method("bool is_hovered()", &layer::gui::ielement_document::is_hovered);
				vdocument->set_method("bool is_active()", &layer::gui::ielement_document::is_active);
				vdocument->set_method("bool is_checked()", &layer::gui::ielement_document::is_checked);
				vdocument->set_method("bool focus()", &layer::gui::ielement_document::focus);
				vdocument->set_method("void blur()", &layer::gui::ielement_document::blur);
				vdocument->set_method("void click()", &layer::gui::ielement_document::click);
				vdocument->set_method("void add_event_listener(const string_view&in, ui_listener@+, bool = false)", &layer::gui::ielement_document::add_event_listener);
				vdocument->set_method("void remove_event_listener(const string_view&in, ui_listener@+, bool = false)", &layer::gui::ielement_document::remove_event_listener);
				vdocument->set_method_ex("bool dispatch_event(const string_view&in, schema@+)", &ielement_document_dispatch_event);
				vdocument->set_method("void scroll_into_view(bool = true)", &layer::gui::ielement_document::scroll_into_view);
				vdocument->set_method("ui_element append_child(const ui_element &in, bool = true)", &layer::gui::ielement_document::append_child);
				vdocument->set_method("ui_element insert_before(const ui_element &in, const ui_element &in)", &layer::gui::ielement_document::insert_before);
				vdocument->set_method("ui_element replace_child(const ui_element &in, const ui_element &in)", &layer::gui::ielement_document::replace_child);
				vdocument->set_method("ui_element remove_child(const ui_element &in)", &layer::gui::ielement_document::remove_child);
				vdocument->set_method("bool has_child_nodes() const", &layer::gui::ielement_document::has_child_nodes);
				vdocument->set_method("ui_element get_element_by_id(const string_view&in)", &layer::gui::ielement_document::get_element_by_id);
				vdocument->set_method_ex("array<ui_element>@ query_selector_all(const string_view&in)", &ielement_document_query_selector_all);
				vdocument->set_method("bool cast_form_color(vector4 &out, bool)", &layer::gui::ielement_document::cast_form_color);
				vdocument->set_method("bool cast_form_string(string &out)", &layer::gui::ielement_document::cast_form_string);
				vdocument->set_method("bool cast_form_pointer(uptr@ &out)", &layer::gui::ielement_document::cast_form_pointer);
				vdocument->set_method("bool cast_form_int32(int &out)", &layer::gui::ielement_document::cast_form_int32);
				vdocument->set_method("bool cast_form_uint32(uint &out)", &layer::gui::ielement_document::cast_form_uint32);
				vdocument->set_method("bool cast_form_flag32(uint &out, uint)", &layer::gui::ielement_document::cast_form_flag32);
				vdocument->set_method("bool cast_form_int64(int64 &out)", &layer::gui::ielement_document::cast_form_int64);
				vdocument->set_method("bool cast_form_uint64(uint64 &out)", &layer::gui::ielement_document::cast_form_uint64);
				vdocument->set_method("bool cast_form_flag64(uint64 &out, uint64)", &layer::gui::ielement_document::cast_form_flag64);
				vdocument->set_method<layer::gui::ielement, bool, float*>("bool cast_form_float(float &out)", &layer::gui::ielement_document::cast_form_float);
				vdocument->set_method<layer::gui::ielement, bool, float*, float>("bool cast_form_float(float &out, float)", &layer::gui::ielement_document::cast_form_float);
				vdocument->set_method("bool cast_form_double(double &out)", &layer::gui::ielement_document::cast_form_double);
				vdocument->set_method("bool cast_form_boolean(bool &out)", &layer::gui::ielement_document::cast_form_boolean);
				vdocument->set_method("string get_form_name() const", &layer::gui::ielement_document::get_form_name);
				vdocument->set_method("void set_form_name(const string_view&in)", &layer::gui::ielement_document::set_form_name);
				vdocument->set_method("string get_form_value() const", &layer::gui::ielement_document::get_form_value);
				vdocument->set_method("void set_form_value(const string_view&in)", &layer::gui::ielement_document::set_form_value);
				vdocument->set_method("bool is_form_disabled() const", &layer::gui::ielement_document::is_form_disabled);
				vdocument->set_method("void set_form_disabled(bool)", &layer::gui::ielement_document::set_form_disabled);
				vdocument->set_method("uptr@ get_element() const", &layer::gui::ielement_document::get_element);
				vdocument->set_method("bool is_valid() const", &layer::gui::ielement_document::is_valid);
				vdocument->set_method("void set_title(const string_view&in)", &layer::gui::ielement_document::set_title);
				vdocument->set_method("void pull_to_front()", &layer::gui::ielement_document::pull_to_front);
				vdocument->set_method("void push_to_back()", &layer::gui::ielement_document::push_to_back);
				vdocument->set_method("void show(ui_modal_flag = ui_modal_flag::none, ui_focus_flag = ui_focus_flag::automatic)", &layer::gui::ielement_document::show);
				vdocument->set_method("void hide()", &layer::gui::ielement_document::hide);
				vdocument->set_method("void close()", &layer::gui::ielement_document::close);
				vdocument->set_method("string get_title() const", &layer::gui::ielement_document::get_title);
				vdocument->set_method("string get_source_url() const", &layer::gui::ielement_document::get_source_url);
				vdocument->set_method("ui_element create_element(const string_view&in)", &layer::gui::ielement_document::create_element);
				vdocument->set_method("bool is_modal() const", &layer::gui::ielement_document::is_modal);
				vdocument->set_method("uptr@ get_element_document() const", &layer::gui::ielement_document::get_element_document);

				return true;
#else
				VI_ASSERT(false, "<ui-control> is not loaded");
				return false;
#endif
			}
			bool heavy_registry::import_ui_model(virtual_machine* vm) noexcept
			{
#ifdef VI_BINDINGS
				VI_ASSERT(vm != nullptr, "manager should be set");
				vm->set_function_def("void ui_data_event(ui_event &in, array<variant>@+)");

				auto vmodel = vm->set_class<layer::gui::data_model>("ui_model", false);
				vmodel->set_method_ex("bool set(const string_view&in, schema@+)", &data_model_set);
				vmodel->set_method_ex("bool set_var(const string_view&in, const variant &in)", &data_model_set_var);
				vmodel->set_method_ex("bool set_string(const string_view&in, const string_view&in)", &data_model_set_string);
				vmodel->set_method_ex("bool set_integer(const string_view&in, int64)", &data_model_set_integer);
				vmodel->set_method_ex("bool set_float(const string_view&in, float)", &data_model_set_float);
				vmodel->set_method_ex("bool set_double(const string_view&in, double)", &data_model_set_double);
				vmodel->set_method_ex("bool set_boolean(const string_view&in, bool)", &data_model_set_boolean);
				vmodel->set_method_ex("bool set_pointer(const string_view&in, uptr@)", &data_model_set_pointer);
				vmodel->set_method_ex("bool set_callback(const string_view&in, ui_data_event@)", &data_model_set_callback);
				vmodel->set_method_ex("schema@ get(const string_view&in)", &data_model_get);
				vmodel->set_method("string get_string(const string_view&in)", &layer::gui::data_model::get_string);
				vmodel->set_method("int64 get_integer(const string_view&in)", &layer::gui::data_model::get_integer);
				vmodel->set_method("float get_float(const string_view&in)", &layer::gui::data_model::get_float);
				vmodel->set_method("double get_double(const string_view&in)", &layer::gui::data_model::get_double);
				vmodel->set_method("bool get_boolean(const string_view&in)", &layer::gui::data_model::get_boolean);
				vmodel->set_method("uptr@ get_pointer(const string_view&in)", &layer::gui::data_model::get_pointer);
				vmodel->set_method("bool has_changed(const string_view&in)", &layer::gui::data_model::has_changed);
				vmodel->set_method("void change(const string_view&in)", &layer::gui::data_model::change);
				vmodel->set_method("bool isValid() const", &layer::gui::data_model::is_valid);
				vmodel->set_method_static("vector4 to_color4(const string_view&in)", &layer::gui::ivariant::to_color4);
				vmodel->set_method_static("string from_color4(const vector4 &in, bool)", &layer::gui::ivariant::from_color4);
				vmodel->set_method_static("vector4 to_color3(const string_view&in)", &layer::gui::ivariant::to_color3);
				vmodel->set_method_static("string from_color3(const vector4 &in, bool)", &layer::gui::ivariant::to_color3);
				vmodel->set_method_static("int get_vector_type(const string_view&in)", &layer::gui::ivariant::get_vector_type);
				vmodel->set_method_static("vector4 to_vector4(const string_view&in)", &layer::gui::ivariant::to_vector4);
				vmodel->set_method_static("string from_vector4(const vector4 &in)", &layer::gui::ivariant::from_vector4);
				vmodel->set_method_static("vector3 to_vector3(const string_view&in)", &layer::gui::ivariant::to_vector3);
				vmodel->set_method_static("string from_vector3(const vector3 &in)", &layer::gui::ivariant::from_vector3);
				vmodel->set_method_static("vector2 to_vector2(const string_view&in)", &layer::gui::ivariant::to_vector2);
				vmodel->set_method_static("string from_vector2(const vector2 &in)", &layer::gui::ivariant::from_vector2);

				return true;
#else
				VI_ASSERT(false, "<ui-model> is not loaded");
				return false;
#endif
			}
		}
	}
}