#ifndef VI_LAYER_GUI_H
#define VI_LAYER_GUI_H
#include "../layer.h"

namespace Rml
{
	class Context;

	class Element;

	class ElementDocument;

	class Decorator;

	class DataModelConstructor;

	class DataModelHandle;

	class Event;

	class EventListener;

	class Variant;
}

namespace vitex
{
	namespace layer
	{
		namespace gui
		{
			class render_subsystem;

			class file_subsystem;

			class main_subsystem;

			class scoped_context;

			class context_instancer;

			class listener_subsystem;

			class listener_instancer;

			class document_subsystem;

			class document_instancer;

			class event_subsystem;

			class ievent;

			class data_model;

			class ielement;

			class ielement_document;

			class data_node;

			class listener;

			class context;

			typedef std::function<core::string(const std::string_view&)> translation_callback;
			typedef std::function<void(void*)> destroy_callback;
			typedef std::function<void(ievent&)> event_callback;
			typedef std::function<void(ievent&, const core::variant_list&)> data_callback;
			typedef std::function<void(core::variant&)> getter_callback;
			typedef std::function<void(const core::variant&)> setter_callback;
			typedef std::function<void(context*)> model_callback;

			enum class modal_flag
			{
				none,
				modal,
				keep
			};

			enum class focus_flag
			{
				none,
				document,
				keep,
				any
			};

			enum class area
			{
				margin = 0,
				border = 1,
				padding = 2,
				content = 3
			};

			enum class display : uint8_t
			{
				none,
				block,
				inline_base,
				inline_block,
				flex,
				table,
				table_row,
				table_row_group,
				table_column,
				table_column_group,
				table_cell
			};

			enum class position : uint8_t
			{
				constant,
				relative,
				absolute,
				fixed
			};

			enum class floatf : uint8_t
			{
				none,
				left,
				right
			};

			enum class timing_func
			{
				none,
				back,
				bounce,
				circular,
				cubic,
				elastic,
				exponential,
				linear,
				quadratic,
				quartic,
				quintic,
				sine,
				callback
			};

			enum class timing_dir
			{
				in = 1,
				out = 2,
				in_out = 3
			};

			enum class event_phase
			{
				none,
				capture = 1,
				target = 2,
				bubble = 4
			};

			enum class input_type
			{
				keys = 1,
				scroll = 2,
				text = 3,
				cursor = 4,
				any = (keys | scroll | text | cursor)
			};

			enum class numeric_unit
			{
				unknown = 0,
				keyword = 1 << 0,
				string = 1 << 1,
				colour = 1 << 2,
				ratio = 1 << 3,
				number = 1 << 4,
				percent = 1 << 5,
				px = 1 << 6,
				dp = 1 << 7,
				vw = 1 << 8,
				vh = 1 << 9,
				x = 1 << 10,
				em = 1 << 11,
				rem = 1 << 12,
				inch = 1 << 13,
				cm = 1 << 14,
				mm = 1 << 15,
				pt = 1 << 16,
				pc = 1 << 17,
				ppi_unit = inch | cm | mm | pt | pc,
				deg = 1 << 18,
				rad = 1 << 19,
				transform = 1 << 20,
				transition = 1 << 21,
				animation = 1 << 22,
				decorator = 1 << 23,
				fonteffect = 1 << 24,
				colorstoplist = 1 << 25,
				shadowlist = 1 << 26,
				length = px | dp | vw | vh | em | rem | ppi_unit,
				length_percent = length | percent,
				number_length_percent = number | length | percent,
				dp_scalable_length = dp | ppi_unit,
				angle = deg | rad,
				numeric = number_length_percent | angle | x
			};

			enum class font_weight : uint16_t
			{
				any = 0,
				normal = 400,
				bold = 700
			};

			inline input_type operator |(input_type a, input_type b)
			{
				return static_cast<input_type>(static_cast<size_t>(a) | static_cast<size_t>(b));
			}

			class gui_exception final : public core::basic_exception
			{
			public:
				gui_exception(core::string&& message);
				const char* type() const noexcept override;
			};

			template <typename v>
			using expects_gui_exception = core::expects<v, gui_exception>;

			struct font_info
			{
				font_weight weight;
				bool fallback;
			};

			class ivariant
			{
			public:
				static void convert(Rml::Variant* from, core::variant* to);
				static void revert(core::variant* from, Rml::Variant* to);
				static trigonometry::vector4 to_color4(const std::string_view& value);
				static core::string from_color4(const trigonometry::vector4& base, bool hex);
				static trigonometry::vector4 to_color3(const std::string_view& value);
				static core::string from_color3(const trigonometry::vector4& base, bool hex);
				static int get_vector_type(const std::string_view& value);
				static trigonometry::vector4 to_vector4(const std::string_view& base);
				static core::string from_vector4(const trigonometry::vector4& base);
				static trigonometry::vector3 to_vector3(const std::string_view& base);
				static core::string from_vector3(const trigonometry::vector3& base);
				static trigonometry::vector2 to_vector2(const std::string_view& base);
				static core::string from_vector2(const trigonometry::vector2& base);
			};

			class ievent
			{
				friend event_subsystem;
				friend data_model;

			private:
				Rml::Event* base;
				bool owned;

			public:
				ievent();
				ievent(Rml::Event* ref);
				ievent copy();
				event_phase get_phase() const;
				void release();
				void set_phase(event_phase phase);
				void set_current_element(const ielement& element);
				ielement get_current_element() const;
				ielement get_target_element() const;
				core::string get_type() const;
				void stop_propagation();
				void stop_immediate_propagation();
				bool is_interruptible() const;
				bool is_propagating() const;
				bool is_immediate_propagating() const;
				bool get_boolean(const std::string_view& key) const;
				int64_t get_integer(const std::string_view& key) const;
				double get_number(const std::string_view& key) const;
				core::string get_string(const std::string_view& key) const;
				trigonometry::vector2 get_vector2(const std::string_view& key) const;
				trigonometry::vector3 get_vector3(const std::string_view& key) const;
				trigonometry::vector4 get_vector4(const std::string_view& key) const;
				void* get_pointer(const std::string_view& key) const;
				Rml::Event* get_event() const;
				bool is_valid() const;
			};

			class ielement
			{
			protected:
				Rml::Element* base;

			public:
				ielement();
				ielement(Rml::Element* ref);
				virtual ~ielement() = default;
				virtual void release();
				ielement clone() const;
				void set_class(const std::string_view& class_name, bool activate);
				bool is_class_set(const std::string_view& class_name) const;
				void set_class_names(const std::string_view& class_names);
				core::string get_class_names() const;
				core::string get_address(bool include_pseudo_classes = false, bool include_parents = true) const;
				void set_offset(const trigonometry::vector2& offset, const ielement& offset_parent, bool offset_fixed = false);
				trigonometry::vector2 get_relative_offset(area type = area::content);
				trigonometry::vector2 get_absolute_offset(area type = area::content);
				void set_content_box(const trigonometry::vector2& content_box);
				float get_baseline() const;
				bool get_intrinsic_dimensions(trigonometry::vector2& dimensions, float& ratio);
				bool is_point_within_element(const trigonometry::vector2& point);
				bool is_visible() const;
				float get_zindex() const;
				bool set_property(const std::string_view& name, const std::string_view& value);
				void remove_property(const std::string_view& name);
				core::string get_property(const std::string_view& name);
				core::string get_local_property(const std::string_view& name);
				float resolve_numeric_property(float value, numeric_unit unit, float base_value);
				trigonometry::vector2 get_containing_block();
				position get_position();
				floatf get_float();
				display get_display();
				float get_line_height();
				bool project(trigonometry::vector2& point) const noexcept;
				bool animate(const std::string_view& property_name, const std::string_view& target_value, float duration, timing_func func, timing_dir dir, int num_iterations = 1, bool alternate_direction = true, float delay = 0.0f);
				bool add_animation_key(const std::string_view& property_name, const std::string_view& target_value, float duration, timing_func func, timing_dir dir);
				void set_pseudo_class(const std::string_view& pseudo_class, bool activate);
				bool is_pseudo_class_set(const std::string_view& pseudo_class) const;
				void set_attribute(const std::string_view& name, const std::string_view& value);
				core::string get_attribute(const std::string_view& name);
				bool has_attribute(const std::string_view& name) const;
				void remove_attribute(const std::string_view& name);
				ielement get_focus_leaf_node();
				core::string get_tag_name() const;
				core::string get_id() const;
				void set_id(const std::string_view& id);
				float get_absolute_left();
				float get_absolute_top();
				float get_client_left();
				float get_client_top();
				float get_client_width();
				float get_client_height();
				ielement get_offset_parent();
				float get_offset_left();
				float get_offset_top();
				float get_offset_width();
				float get_offset_height();
				float get_scroll_left();
				void set_scroll_left(float scroll_left);
				float get_scroll_top();
				void set_scroll_top(float scroll_top);
				float get_scroll_width();
				float get_scroll_height();
				ielement_document get_owner_document() const;
				ielement get_parent_node() const;
				ielement get_next_sibling() const;
				ielement get_previous_sibling() const;
				ielement get_first_child() const;
				ielement get_last_child() const;
				ielement get_child(int index) const;
				int get_num_children(bool include_non_dom_elements = false) const;
				void get_inner_html(core::string& content) const;
				core::string get_inner_html() const;
				void set_inner_html(const std::string_view& html);
				bool is_focused();
				bool is_hovered();
				bool is_active();
				bool is_checked();
				bool focus();
				void blur();
				void click();
				void add_event_listener(const std::string_view& event, listener* source, bool in_capture_phase = false);
				void remove_event_listener(const std::string_view& event, listener* source, bool in_capture_phase = false);
				bool dispatch_event(const std::string_view& type, const core::variant_args& args);
				void scroll_into_view(bool align_with_top = true);
				ielement append_child(const ielement& element, bool dom_element = true);
				ielement insert_before(const ielement& element, const ielement& adjacent_element);
				ielement replace_child(const ielement& inserted_element, const ielement& replaced_element);
				ielement remove_child(const ielement& element);
				bool has_child_nodes() const;
				ielement get_element_by_id(const std::string_view& id);
				ielement query_selector(const std::string_view& selector);
				core::vector<ielement> query_selector_all(const std::string_view& selectors);
				bool cast_form_color(trigonometry::vector4* ptr, bool alpha);
				bool cast_form_string(core::string* ptr);
				bool cast_form_pointer(void** ptr);
				bool cast_form_int32(int32_t* ptr);
				bool cast_form_uint32(uint32_t* ptr);
				bool cast_form_flag32(uint32_t* ptr, uint32_t mask);
				bool cast_form_int64(int64_t* ptr);
				bool cast_form_uint64(uint64_t* ptr);
				bool cast_form_size(size_t* ptr);
				bool cast_form_flag64(uint64_t* ptr, uint64_t mask);
				bool cast_form_float(float* ptr);
				bool cast_form_float(float* ptr, float mult);
				bool cast_form_double(double* ptr);
				bool cast_form_boolean(bool* ptr);
				core::string get_form_name() const;
				void set_form_name(const std::string_view& name);
				core::string get_form_value() const;
				void set_form_value(const std::string_view& value);
				bool is_form_disabled() const;
				void set_form_disabled(bool disable);
				Rml::Element* get_element() const;
				bool is_valid() const;

			public:
				static core::string from_pointer(void* ptr);
				static void* to_pointer(const std::string_view& value);
			};

			class ielement_document : public ielement
			{
			public:
				ielement_document();
				ielement_document(Rml::ElementDocument* ref);
				void release() override;
				void set_title(const std::string_view& title);
				void pull_to_front();
				void push_to_back();
				void show(modal_flag modal = modal_flag::none, focus_flag focus = focus_flag::any);
				void hide();
				void close();
				core::string get_title() const;
				core::string get_source_url() const;
				ielement create_element(const std::string_view& name);
				bool is_modal() const;
				Rml::ElementDocument* get_element_document() const;
			};

			class utils
			{
			public:
				static trigonometry::matrix4x4 to_matrix(const void* matrix) noexcept;
				static core::string escape_html(const std::string_view& text) noexcept;
			};

			class subsystem final : public core::singleton<subsystem>
			{
				friend render_subsystem;
				friend document_subsystem;
				friend listener_subsystem;
				friend context;

			private:
				struct
				{
					scripting::virtual_machine* vm = nullptr;
					graphics::activity* activity = nullptr;
					render_constants* constants = nullptr;
					heavy_content_manager* content = nullptr;
					core::timer* time = nullptr;

					void add_ref()
					{
						if (vm != nullptr)
							vm->add_ref();
						if (activity != nullptr)
							activity->add_ref();
						if (constants != nullptr)
							constants->add_ref();
						if (content != nullptr)
							content->add_ref();
						if (time != nullptr)
							time->add_ref();
					}
					void release()
					{
						core::memory::release(vm);
						core::memory::release(activity);
						core::memory::release(constants);
						core::memory::release(content);
						core::memory::release(time);
					}
				} shared;

			private:
				context_instancer* context_factory;
				document_instancer* document_factory;
				listener_instancer* listener_factory;
				render_subsystem* render_interface;
				file_subsystem* file_interface;
				main_subsystem* system_interface;
				uint64_t id;

			public:
				subsystem() noexcept;
				virtual ~subsystem() noexcept override;
				void set_shared(scripting::virtual_machine* vm, graphics::activity* activity, render_constants* constants, heavy_content_manager* content, core::timer* time) noexcept;
				void set_translator(const std::string_view& name, translation_callback&& callback) noexcept;
				void cleanup_shared();
				render_subsystem* get_render_interface() noexcept;
				file_subsystem* get_file_interface() noexcept;
				main_subsystem* get_system_interface() noexcept;
				graphics::graphics_device* get_device() noexcept;
				graphics::texture_2d* get_background() noexcept;
				trigonometry::matrix4x4* get_transform() noexcept;
				trigonometry::matrix4x4* get_projection() noexcept;

			private:
				void resize_decorators(int width, int height) noexcept;
				void create_decorators(render_constants* constants) noexcept;
				void release_decorators() noexcept;
				void create_elements() noexcept;
				void release_elements() noexcept;
			};

			class data_model final : public core::reference<data_model>
			{
				friend context;

			private:
				core::unordered_map<core::string, data_node*> props;
				core::vector<std::function<void()>> callbacks;
				Rml::DataModelConstructor* base;
				model_callback on_unmount;

			private:
				data_model(Rml::DataModelConstructor* ref);

			public:
				~data_model() noexcept;
				data_node* set_property(const std::string_view& name, const core::variant& value);
				data_node* set_property(const std::string_view& name, core::variant* reference);
				data_node* set_array(const std::string_view& name);
				data_node* set_string(const std::string_view& name, const std::string_view& value);
				data_node* set_integer(const std::string_view& name, int64_t value);
				data_node* set_float(const std::string_view& name, float value);
				data_node* set_double(const std::string_view& name, double value);
				data_node* set_boolean(const std::string_view& name, bool value);
				data_node* set_pointer(const std::string_view& name, void* value);
				data_node* get_property(const std::string_view& name);
				core::string get_string(const std::string_view& name);
				int64_t get_integer(const std::string_view& name);
				float get_float(const std::string_view& name);
				double get_double(const std::string_view& name);
				bool get_boolean(const std::string_view& name);
				void* get_pointer(const std::string_view& name);
				bool set_callback(const std::string_view& name, data_callback&& callback);
				bool set_unmount_callback(model_callback&& callback);
				bool has_changed(const std::string_view& variable_name) const;
				void set_detach_callback(std::function<void()>&& callback);
				void change(const std::string_view& variable_name);
				void detach();
				bool is_valid() const;
				Rml::DataModelConstructor* get();
			};

			class data_node
			{
				friend context;
				friend data_model;

			private:
				core::vector<data_node> childs;
				core::string name;
				core::variant* ref;
				data_model* handle;
				void* order;
				size_t depth;
				bool safe;

			public:
				data_node(data_model* model, const std::string_view& top_name, const core::variant& initial) noexcept;
				data_node(data_model* model, const std::string_view& top_name, core::variant* reference) noexcept;
				data_node(data_node&& other) noexcept;
				data_node(const data_node& other) noexcept;
				~data_node();
				data_node& insert(size_t where, const core::variant_list& initial, std::pair<void*, size_t>* top = nullptr);
				data_node& insert(size_t where, const core::variant& initial, std::pair<void*, size_t>* top = nullptr);
				data_node& insert(size_t where, core::variant* reference, std::pair<void*, size_t>* top = nullptr);
				data_node& add(const core::variant_list& initial, std::pair<void*, size_t>* top = nullptr);
				data_node& add(const core::variant& initial, std::pair<void*, size_t>* top = nullptr);
				data_node& add(core::variant* reference, std::pair<void*, size_t>* top = nullptr);
				data_node& at(size_t index);
				bool remove(size_t index);
				bool clear();
				void sort_tree();
				void replace(const core::variant_list& data, std::pair<void*, size_t>* top = nullptr);
				void set(const core::variant& new_value);
				void set(core::variant* new_reference);
				void set_top(void* seq_id, size_t depth);
				void set_string(const std::string_view& value);
				void set_vector2(const trigonometry::vector2& value);
				void set_vector3(const trigonometry::vector3& value);
				void set_vector4(const trigonometry::vector4& value);
				void set_integer(int64_t value);
				void set_float(float value);
				void set_double(double value);
				void set_boolean(bool value);
				void set_pointer(void* value);
				size_t size() const;
				void* get_seq_id() const;
				size_t get_depth() const;
				const core::variant& get();
				core::string get_string();
				trigonometry::vector2 get_vector2();
				trigonometry::vector3 get_vector3();
				trigonometry::vector4 get_vector4();
				int64_t get_integer();
				float get_float();
				double get_double();
				bool get_boolean();
				void* get_pointer();
				data_node& operator= (const data_node& other) noexcept;
				data_node& operator= (data_node&& other) noexcept;

			private:
				void get_value(Rml::Variant& result);
				void set_value(const Rml::Variant& result);
				void set_value_str(const core::string& value);
				void set_value_num(double value);
				void set_value_int(int64_t value);
				int64_t get_value_size();
			};

			class listener final : public core::reference<listener>
			{
				friend ielement;
				friend context;

			private:
				Rml::EventListener* base;

			public:
				listener(event_callback&& new_callback);
				listener(const std::string_view& function_name);
				~listener() noexcept;
			};

			class context final : public core::reference<context>
			{
				friend document_subsystem;
				friend listener_subsystem;

			private:
				struct
				{
					bool keys = false;
					bool scroll = false;
					bool text = false;
					bool cursor = false;
				} inputs;

			private:
				core::unordered_map<int, core::unordered_map<core::string, Rml::Element*>> elements;
				core::unordered_map<core::string, data_model*> models;
				scripting::compiler* compiler;
				trigonometry::vector2 cursor;
				model_callback on_mount;
				scoped_context* base;
				subsystem* system;
				uint32_t skips;
				uint32_t busy;

			public:
				context(const trigonometry::vector2& size);
				context(graphics::graphics_device* device);
				~context() noexcept;
				void emit_key(graphics::key_code key, graphics::key_mod mod, int computed, int repeat, bool pressed);
				void emit_input(const char* buffer, int length);
				void emit_wheel(int x, int y, bool normal, graphics::key_mod mod);
				void emit_resize(int width, int height);
				void update_events(graphics::activity* activity);
				void render_lists(graphics::texture_2d* target);
				void enable_mouse_cursor(bool enable);
				void clear_styles();
				bool clear_documents();
				bool is_loading();
				bool is_input_focused();
				bool was_input_used(uint32_t input_type_mask);
				uint64_t calculate_idle_timeout_ms(uint64_t max_timeout);
				int get_num_documents() const;
				void set_density_independent_pixel_ratio(float density_independent_pixel_ratio);
				float get_density_independent_pixel_ratio() const;
				bool replace_html(const std::string_view& selector, const std::string_view& html, int index = 0);
				expects_gui_exception<ielement_document> eval_html(const std::string_view& html, int index = 0);
				expects_gui_exception<ielement_document> add_css(const std::string_view& css, int index = 0);
				expects_gui_exception<ielement_document> load_css(const std::string_view& path, int index = 0);
				expects_gui_exception<ielement_document> load_document(const std::string_view& path, bool allow_preprocessing = false);
				expects_gui_exception<ielement_document> add_document(const std::string_view& html);
				expects_gui_exception<ielement_document> add_document_empty(const std::string_view& instancer_name = "body");
				ielement_document get_document(const std::string_view& id);
				ielement_document get_document(int index);
				ielement get_element_by_id(const std::string_view& id, int index = 0);
				ielement get_hover_element();
				ielement get_focus_element();
				ielement get_root_element();
				ielement get_element_at_point(const trigonometry::vector2& point, const ielement& ignore_element = nullptr, const ielement& element = nullptr) const;
				void pull_document_to_front(const ielement_document& schema);
				void push_document_to_back(const ielement_document& schema);
				void unfocus_document(const ielement_document& schema);
				void add_event_listener(const std::string_view& event, listener* listener, bool in_capture_phase = false);
				void remove_event_listener(const std::string_view& event, listener* listener, bool in_capture_phase = false);
				bool is_mouse_interacting() const;
				bool remove_data_model(const std::string_view& name);
				bool remove_data_models();
				void set_documents_base_tag(const std::string_view& tag);
				void set_mount_callback(model_callback&& callback);
				core::string get_documents_base_tag();
				core::unordered_map<core::string, core::vector<font_info>>* get_font_faces();
				trigonometry::vector2 get_dimensions() const;
				data_model* set_data_model(const std::string_view& name);
				data_model* get_data_model(const std::string_view& name);
				Rml::Context* get_context();

			public:
				static expects_gui_exception<void> load_font_face(const std::string_view& path, bool use_as_fallback = false, font_weight weight = font_weight::any);
				static core::string resolve_resource_path(const ielement& element, const std::string_view& path);

			private:
				expects_gui_exception<void> preprocess(const std::string_view& path, core::string& buffer);
				void decompose(core::string& buffer);
				void clear_scope();

			private:
				static int get_key_code(graphics::key_code key);
				static int get_key_mod(graphics::key_mod mod);
			};
		}
	}
}
#endif
