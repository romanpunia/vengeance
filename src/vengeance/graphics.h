#ifndef VI_GRAPHICS_H
#define VI_GRAPHICS_H
#include "trigonometry.h"
#include <iostream>
#include <functional>
#include <limits>
#define VI_VS (uint32_t)vitex::graphics::shader_type::vertex
#define VI_PS (uint32_t)vitex::graphics::shader_type::pixel
#define VI_GS (uint32_t)vitex::graphics::shader_type::geometry
#define VI_CS (uint32_t)vitex::graphics::shader_type::compute
#define VI_HS (uint32_t)vitex::graphics::shader_type::hull
#define VI_DS (uint32_t)vitex::graphics::shader_type::domain

struct SDL_SysWMinfo;
struct SDL_Cursor;
struct SDL_Window;
struct SDL_Surface;

namespace vitex
{
	namespace graphics
	{
		enum
		{
			window_size = 128,
			joints_size = 96,
			units_size = 32
		};

		enum class app_state
		{
			close,
			terminating,
			low_memory,
			enter_background_start,
			enter_background_end,
			enter_foreground_start,
			enter_foreground_end
		};

		enum class window_state
		{
			show,
			hide,
			expose,
			move,
			resize,
			size_change,
			minimize,
			maximize,
			restore,
			enter,
			leave,
			focus,
			blur,
			close
		};

		enum class key_code
		{
			a = 4,
			b = 5,
			c = 6,
			d = 7,
			e = 8,
			f = 9,
			g = 10,
			h = 11,
			i = 12,
			j = 13,
			k = 14,
			l = 15,
			m = 16,
			n = 17,
			o = 18,
			p = 19,
			q = 20,
			r = 21,
			s = 22,
			t = 23,
			u = 24,
			v = 25,
			w = 26,
			x = 27,
			y = 28,
			z = 29,
			d1 = 30,
			d2 = 31,
			d3 = 32,
			d4 = 33,
			d5 = 34,
			d6 = 35,
			d7 = 36,
			d8 = 37,
			d9 = 38,
			d0 = 39,
			defer = 40,
			escape = 41,
			backspace = 42,
			tab = 43,
			space = 44,
			minus = 45,
			equals = 46,
			left_bracket = 47,
			right_bracket = 48,
			backslash = 49,
			non_us_hash = 50,
			semicolon = 51,
			apostrophe = 52,
			grave = 53,
			comma = 54,
			period = 55,
			slash = 56,
			capslock = 57,
			f1 = 58,
			f2 = 59,
			f3 = 60,
			f4 = 61,
			f5 = 62,
			f6 = 63,
			f7 = 64,
			f8 = 65,
			f9 = 66,
			f10 = 67,
			f11 = 68,
			f12 = 69,
			print_screen = 70,
			scroll_lock = 71,
			pause = 72,
			insert = 73,
			home = 74,
			page_up = 75,
			deinit = 76,
			end = 77,
			page_down = 78,
			right = 79,
			left = 80,
			down = 81,
			up = 82,
			num_lock_clear = 83,
			kp_divide = 84,
			kp_multiply = 85,
			kp_minus = 86,
			kp_plus = 87,
			kp_enter = 88,
			kp1 = 89,
			kp2 = 90,
			kp3 = 91,
			kp4 = 92,
			kp5 = 93,
			kp6 = 94,
			kp7 = 95,
			kp8 = 96,
			kp9 = 97,
			kp0 = 98,
			kp_period = 99,
			non_us_backslash = 100,
			app0 = 101,
			power = 102,
			kp_equals = 103,
			f13 = 104,
			f14 = 105,
			f15 = 106,
			f16 = 107,
			f17 = 108,
			f18 = 109,
			f19 = 110,
			f20 = 111,
			f21 = 112,
			f22 = 113,
			f23 = 114,
			f24 = 115,
			execute = 116,
			help = 117,
			menu = 118,
			select = 119,
			stop = 120,
			again = 121,
			undo = 122,
			cut = 123,
			copy = 124,
			paste = 125,
			find = 126,
			mute = 127,
			volume_up = 128,
			volume_down = 129,
			kp_comma = 133,
			kp_equals_as400 = 134,
			international1 = 135,
			international2 = 136,
			international3 = 137,
			international4 = 138,
			international5 = 139,
			international6 = 140,
			international7 = 141,
			international8 = 142,
			international9 = 143,
			lang1 = 144,
			lang2 = 145,
			lang3 = 146,
			lang4 = 147,
			lang5 = 148,
			lang6 = 149,
			lang7 = 150,
			lang8 = 151,
			lang9 = 152,
			alterase = 153,
			sys_req = 154,
			cancel = 155,
			clear = 156,
			prior = 157,
			return2 = 158,
			separator = 159,
			output = 160,
			operation = 161,
			clear_again = 162,
			cr_select = 163,
			ex_select = 164,
			kp00 = 176,
			kp000 = 177,
			thousands_separator = 178,
			decimals_separator = 179,
			currency_unit = 180,
			currency_subunit = 181,
			kp_left_paren = 182,
			kp_right_paren = 183,
			kp_left_brace = 184,
			kp_right_brace = 185,
			kp_tab = 186,
			kp_backspace = 187,
			kp_a = 188,
			kp_b = 189,
			kp_c = 190,
			kp_d = 191,
			kp_e = 192,
			kp_f = 193,
			kp_xor = 194,
			kp_power = 195,
			kp_percent = 196,
			kp_less = 197,
			kp_greater = 198,
			kp_ampersand = 199,
			kp_dbl_ampersand = 200,
			kp_vertical_bar = 201,
			kp_dbl_vertical_bar = 202,
			kp_colon = 203,
			kp_hash = 204,
			kp_space = 205,
			kp_at = 206,
			kp_exclaim = 207,
			kp_mem_store = 208,
			kp_mem_recall = 209,
			kp_mem_clear = 210,
			kp_mem_add = 211,
			kp_mem_subtract = 212,
			kp_mem_multiply = 213,
			kp_mem_divide = 214,
			kp_plus_minus = 215,
			kp_clear = 216,
			kp_clear_entry = 217,
			kp_binary = 218,
			kp_octal = 219,
			kp_decimal = 220,
			kp_hexadecimal = 221,
			left_control = 224,
			left_shift = 225,
			left_alt = 226,
			left_gui = 227,
			right_control = 228,
			right_shift = 229,
			right_alt = 230,
			right_gui = 231,
			mode = 257,
			audio_next = 258,
			audio_prev = 259,
			audio_stop = 260,
			audio_play = 261,
			audio_mute = 262,
			media_select = 263,
			www = 264,
			mail = 265,
			calculator = 266,
			computer = 267,
			ac_search = 268,
			ac_home = 269,
			ac_back = 270,
			ac_forward = 271,
			ac_stop = 272,
			ac_refresh = 273,
			ac_bookmarks = 274,
			brightness_down = 275,
			brightness_up = 276,
			display_switch = 277,
			kb_illum_toggle = 278,
			kb_illum_down = 279,
			kb_illum_up = 280,
			eject = 281,
			sleep = 282,
			app1 = 283,
			app2 = 284,
			audio_rewind = 285,
			audio_fast_forward = 286,
			cursor_left = 287,
			cursor_middle = 288,
			cursor_right = 289,
			cursor_x1 = 290,
			cursor_x2 = 291,
			none = 292
		};

		enum class key_mod
		{
			none = 0x0000,
			left_shift = 0x0001,
			right_shift = 0x0002,
			left_control = 0x0040,
			right_control = 0x0080,
			left_alt = 0x0100,
			right_alt = 0x0200,
			left_gui = 0x0400,
			right_gui = 0x0800,
			num = 0x1000,
			caps = 0x2000,
			mode = 0x4000,
			reserved = 0x8000,
			shift = left_shift | right_shift,
			control = left_control | right_control,
			alt = left_alt | right_alt,
			gui = left_gui | right_gui
		};

		enum class alert_type
		{
			none = 0,
			error = 0x00000010,
			warning = 0x00000020,
			info = 0x00000040
		};

		enum class alert_confirm
		{
			none = 0,
			defer = 0x00000002,
			escape = 0x00000001
		};

		enum class joy_stick_hat
		{
			center = 0x00,
			up = 0x01,
			right = 0x02,
			down = 0x04,
			left = 0x08,
			right_up = 0x02 | 0x01,
			right_down = 0x02 | 0x04,
			left_up = 0x08 | 0x01,
			left_down = 0x08 | 0x04
		};

		enum class render_backend
		{
			none,
			automatic,
			d3d11,
			ogl
		};

		enum class vsync
		{
			off,
			frequency_x1,
			frequency_x2,
			frequency_x3,
			frequency_x4,
			on = frequency_x1
		};

		enum class surface_target
		{
			t0 = 1,
			t1 = 2,
			t2 = 3,
			t3 = 4,
			t4 = 5,
			t5 = 6,
			t6 = 7,
			t7 = 8
		};

		enum class primitive_topology
		{
			invalid,
			point_list,
			line_list,
			line_strip,
			triangle_list,
			triangle_strip,
			line_list_adj,
			line_strip_adj,
			triangle_list_adj,
			triangle_strip_adj
		};

		enum class format
		{
			unknown = 0,
			a8_unorm = 65,
			d16_unorm = 55,
			d24_unorm_s8_uint = 45,
			d32_float = 40,
			r10g10b10a2_uint = 25,
			r10g10b10a2_unorm = 24,
			r11g11b10_float = 26,
			r16g16b16a16_float = 10,
			r16g16b16a16_sint = 14,
			r16g16b16a16_snorm = 13,
			r16g16b16a16_uint = 12,
			r16g16b16a16_unorm = 11,
			r16g16_float = 34,
			r16g16_sint = 38,
			r16g16_snorm = 37,
			r16g16_uint = 36,
			r16g16_unorm = 35,
			r16_float = 54,
			r16_sint = 59,
			r16_snorm = 58,
			r16_uint = 57,
			r16_unorm = 56,
			r1_unorm = 66,
			r32g32b32a32_float = 2,
			r32g32b32a32_sint = 4,
			r32g32b32a32_uint = 3,
			r32g32b32_float = 6,
			r32g32b32_sint = 8,
			r32g32b32_uint = 7,
			r32g32_float = 16,
			r32g32_sint = 18,
			r32g32_uint = 17,
			r32_float = 41,
			r32_sint = 43,
			r32_uint = 42,
			r8g8b8a8_sint = 32,
			r8g8b8a8_snorm = 31,
			r8g8b8a8_uint = 30,
			r8g8b8a8_unorm = 28,
			r8g8b8a8_unorm_srgb = 29,
			r8g8b8g8_unorm = 68,
			r8g8_sint = 52,
			r8g8_snorm = 51,
			r8g8_uint = 50,
			r8g8_unorm = 49,
			r8_sint = 64,
			r8_snorm = 63,
			r8_uint = 62,
			r8_unorm = 61,
			r9g9b9e5_share_dexp = 67
		};

		enum class resource_map
		{
			read = 1,
			write = 2,
			read_write = 3,
			write_discard = 4,
			write_no_overwrite = 5
		};

		enum class resource_usage
		{
			defaults = 0,
			immutable = 1,
			dynamic = 2,
			staging = 3
		};

		enum class shader_model
		{
			invalid,
			any,
			hlsl_1_0 = 100,
			hlsl_2_0 = 200,
			hlsl_3_0 = 300,
			hlsl_4_0 = 400,
			hlsl_4_1 = 410,
			hlsl_5_0 = 500,
			glsl_1_1_0 = 110,
			glsl_1_2_0 = 120,
			glsl_1_3_0 = 130,
			glsl_1_4_0 = 140,
			glsl_1_5_0 = 150,
			glsl_3_3_0 = 330,
			glsl_4_0_0 = 400,
			glsl_4_1_0 = 410,
			glsl_4_2_0 = 420,
			glsl_4_3_0 = 430,
			glsl_4_4_0 = 440,
			glsl_4_5_0 = 450,
			glsl_4_6_0 = 460
		};

		enum class resource_bind
		{
			vertex_buffer = 0x1L,
			index_buffer = 0x2L,
			constant_buffer = 0x4L,
			shader_input = 0x8L,
			stream_output = 0x10L,
			render_target = 0x20L,
			depth_stencil = 0x40L,
			unordered_access = 0x80L
		};

		enum class cpu_access
		{
			none = 0,
			write = 0x10000L,
			read = 0x20000L
		};

		enum class depth_write
		{
			zero,
			all
		};

		enum class comparison
		{
			never = 1,
			less = 2,
			equal = 3,
			less_equal = 4,
			greater = 5,
			not_equal = 6,
			greater_equal = 7,
			always = 8
		};

		enum class stencil_operation
		{
			keep = 1,
			zero = 2,
			replace = 3,
			sat_add = 4,
			sat_subtract = 5,
			invert = 6,
			add = 7,
			subtract = 8
		};

		enum class blend
		{
			zero = 1,
			one = 2,
			source_color = 3,
			source_color_invert = 4,
			source_alpha = 5,
			source_alpha_invert = 6,
			destination_alpha = 7,
			destination_alpha_invert = 8,
			destination_color = 9,
			destination_color_invert = 10,
			source_alpha_sat = 11,
			blend_factor = 14,
			blend_factor_invert = 15,
			source1_color = 16,
			source1_color_invert = 17,
			source1_alpha = 18,
			source1_alpha_invert = 19
		};

		enum class surface_fill
		{
			wireframe = 2,
			solid = 3
		};

		enum class pixel_filter
		{
			min_mag_mip_point = 0,
			min_mag_point_mip_linear = 0x1,
			min_point_mag_linear_mip_point = 0x4,
			min_point_mag_mip_linear = 0x5,
			min_linear_mag_mip_point = 0x10,
			min_linear_mag_point_mip_linear = 0x11,
			min_mag_linear_mip_point = 0x14,
			min_mag_mip_linear = 0x15,
			anistropic = 0x55,
			compare_min_mag_mip_point = 0x80,
			compare_min_mag_point_mip_linear = 0x81,
			compare_min_point_mag_linear_mip_point = 0x84,
			compare_min_point_mag_mip_linear = 0x85,
			compare_min_linear_mag_mip_point = 0x90,
			compare_min_linear_mag_point_mip_linear = 0x91,
			compare_min_mag_linear_mip_point = 0x94,
			compare_min_mag_mip_linear = 0x95,
			compare_anistropic = 0xd5
		};

		enum class texture_address
		{
			wrap = 1,
			mirror = 2,
			clamp = 3,
			border = 4,
			mirror_once = 5
		};

		enum class color_write_enable
		{
			red = 1,
			green = 2,
			blue = 4,
			alpha = 8,
			all = (((red | green) | blue) | alpha)
		};

		enum class blend_operation
		{
			add = 1,
			subtract = 2,
			subtract_reverse = 3,
			min = 4,
			max = 5
		};

		enum class vertex_cull
		{
			none = 1,
			front = 2,
			back = 3
		};

		enum class shader_compile
		{
			debug = 1ll << 0,
			skip_validation = 1ll << 1,
			skip_optimization = 1ll << 2,
			matrix_row_major = 1ll << 3,
			matrix_column_major = 1ll << 4,
			partial_precision = 1ll << 5,
			foevs_no_opt = 1ll << 6,
			foeps_no_opt = 1ll << 7,
			no_preshader = 1ll << 8,
			avoid_flow_control = 1ll << 9,
			prefer_flow_control = 1ll << 10,
			enable_strictness = 1ll << 11,
			enable_backwards_compatibility = 1ll << 12,
			ieee_strictness = 1ll << 13,
			optimization_level0 = 1ll << 14,
			optimization_level1 = 0,
			optimization_level2 = (1ll << 14) | (1ll << 15),
			optimization_level3 = 1ll << 15,
			reseed_x16 = 1ll << 16,
			reseed_x17 = 1ll << 17,
			picky = 1ll << 18
		};

		enum class resource_misc
		{
			none = 0,
			generate_mips = 0x1L,
			shared = 0x2L,
			texture_cube = 0x4L,
			draw_indirect_args = 0x10L,
			buffer_allow_raw_views = 0x20L,
			buffer_structured = 0x40L,
			clamp = 0x80L,
			shared_keyed_mutex = 0x100L,
			gdi_compatible = 0x200L,
			shared_nt_handle = 0x800L,
			restricted_content = 0x1000L,
			restrict_shared = 0x2000L,
			restrict_shared_driver = 0x4000L,
			guarded = 0x8000L,
			tile_pool = 0x20000L,
			tiled = 0x40000L
		};

		enum class display_cursor
		{
			none = -1,
			arrow = 0,
			text_input,
			resize_all,
			resize_ns,
			resize_ew,
			resize_nesw,
			resize_nwse,
			hand,
			crosshair,
			wait,
			progress,
			no,
			count
		};

		enum class shader_type
		{
			vertex = 1,
			pixel = 2,
			geometry = 4,
			hull = 8,
			domain = 16,
			compute = 32,
			all = vertex | pixel | geometry | hull | domain | compute
		};

		enum class shader_lang
		{
			none,
			spv,
			msl,
			hlsl,
			glsl,
			glsl_es
		};

		enum class attribute_type
		{
			byte,
			ubyte,
			half,
			floatf,
			intf,
			uint,
			matrix
		};

		enum class orientation_type
		{
			unknown,
			landscape,
			landscape_flipped,
			portrait,
			portrait_flipped
		};

		inline color_write_enable operator |(color_write_enable a, color_write_enable b)
		{
			return static_cast<color_write_enable>(static_cast<size_t>(a) | static_cast<size_t>(b));
		}
		inline resource_map operator |(resource_map a, resource_map b)
		{
			return static_cast<resource_map>(static_cast<size_t>(a) | static_cast<size_t>(b));
		}
		inline shader_compile operator |(shader_compile a, shader_compile b)
		{
			return static_cast<shader_compile>(static_cast<size_t>(a) | static_cast<size_t>(b));
		}
		inline resource_misc operator |(resource_misc a, resource_misc b)
		{
			return static_cast<resource_misc>(static_cast<size_t>(a) | static_cast<size_t>(b));
		}
		inline resource_bind operator |(resource_bind a, resource_bind b)
		{
			return static_cast<resource_bind>(static_cast<size_t>(a) | static_cast<size_t>(b));
		}

		class shader;

		class texture_2d;

		class skin_mesh_buffer;

		class graphics_device;

		class activity;

		typedef std::function<void(app_state)> app_state_change_callback;
		typedef std::function<void(window_state, int, int)> window_state_change_callback;
		typedef std::function<void(key_code, key_mod, int, int, bool)> key_state_callback;
		typedef std::function<void(char*, int, int)> input_edit_callback;
		typedef std::function<void(char*, int)> input_callback;
		typedef std::function<void(int, int, int, int)> cursor_move_callback;
		typedef std::function<void(int, int, bool)> cursor_wheel_state_callback;
		typedef std::function<void(int, int, int)> joy_stick_axis_move_callback;
		typedef std::function<void(int, int, int, int)> joy_stick_ball_move_callback;
		typedef std::function<void(enum joy_stick_hat, int, int)> joy_stick_hat_move_callback;
		typedef std::function<void(int, int, bool)> joy_stick_key_state_callback;
		typedef std::function<void(int, bool)> joy_stick_state_callback;
		typedef std::function<void(int, int, int)> controller_axis_move_callback;
		typedef std::function<void(int, int, bool)> controller_key_state_callback;
		typedef std::function<void(int, int)> controller_state_callback;
		typedef std::function<void(int, int, float, float, float, float, float)> touch_move_callback;
		typedef std::function<void(int, int, float, float, float, float, float, bool)> touch_state_callback;
		typedef std::function<void(int, int, int, float, float, float, bool)> gesture_state_callback;
		typedef std::function<void(int, int, float, float, float, float)> multi_gesture_state_callback;
		typedef std::function<void(const std::string_view&)> drop_file_callback;
		typedef std::function<void(const std::string_view&)> drop_text_callback;
		typedef std::function<void(graphics_device*)> render_thread_callback;

		struct alert
		{
			friend activity;

		private:
			struct element
			{
				core::string name;
				int id = -1, flags = 0;
			};

		private:
			core::string name;
			core::string data;
			core::vector<element> buttons;
			std::function<void(int)> done;
			alert_type view;
			activity* base;
			bool waiting;

		public:
			alert(activity* from) noexcept;
			void setup(alert_type type, const std::string_view& title, const std::string_view& text);
			void button(alert_confirm confirm, const std::string_view& text, int id);
			void result(std::function<void(int)>&& callback);

		private:
			void dispatch();
		};

		struct display_info
		{
			core::string name;
			orientation_type orientation;
			float diagonal_dpi = 1.0f;
			float horizontal_dpi = 1.0f;
			float vertical_dpi = 1.0f;
			uint32_t pixel_format = 0;
			uint32_t physical_width = 0;
			uint32_t physical_height = 0;
			uint32_t refresh_rate = 0;
			int32_t width = 0;
			int32_t height = 0;
			int32_t x = 0;
			int32_t y = 0;
		};

		struct event_consumers
		{
			friend activity;

		private:
			core::unordered_map<uint32_t, activity*> consumers;

		public:
			void push(activity* value);
			void pop(activity* value);
			activity* find(uint32_t id) const;
		};

		struct key_map
		{
			key_code key;
			key_mod mod;
			bool normal;

			key_map() noexcept;
			key_map(const key_code& value) noexcept;
			key_map(const key_mod& value) noexcept;
			key_map(const key_code& value, const key_mod& control) noexcept;
		};

		struct mapped_subresource
		{
			void* pointer = nullptr;
			uint32_t row_pitch = 0;
			uint32_t depth_pitch = 0;
		};

		struct viewport
		{
			float top_left_x;
			float top_left_y;
			float width;
			float height;
			float min_depth;
			float max_depth;
		};

		struct render_target_blend_state
		{
			blend src_blend;
			blend dest_blend;
			blend_operation blend_operation_mode;
			blend src_blend_alpha;
			blend dest_blend_alpha;
			blend_operation blend_operation_alpha;
			uint8_t render_target_write_mask;
			bool blend_enable;
		};

		struct basic_effect_buffer
		{

		};

		class graphics_exception final : public core::basic_exception
		{
		private:
			int error_code;

		public:
			graphics_exception(core::string&& message);
			graphics_exception(int error_code, core::string&& message);
			const char* type() const noexcept override;
			int code() const noexcept;
		};

		class video_exception final : public core::basic_exception
		{
		public:
			video_exception();
			video_exception(graphics_exception&& other);
			const char* type() const noexcept override;
		};

		template <typename v>
		using expects_graphics = core::expects<v, graphics_exception>;

		template <typename v>
		using expects_video = core::expects<v, video_exception>;

		class surface : public core::reference<surface>
		{
		private:
			SDL_Surface* handle;

		public:
			surface() noexcept;
			surface(SDL_Surface* from) noexcept;
			~surface() noexcept;
			void set_handle(SDL_Surface* from);
			void lock();
			void unlock();
			int get_width() const;
			int get_height() const;
			int get_pitch() const;
			void* get_pixels() const;
			void* get_resource() const;
		};

		class depth_stencil_state : public core::reference<depth_stencil_state>
		{
		public:
			struct desc
			{
				stencil_operation front_face_stencil_fail_operation;
				stencil_operation front_face_stencil_depth_fail_operation;
				stencil_operation front_face_stencil_pass_operation;
				comparison front_face_stencil_function;
				stencil_operation back_face_stencil_fail_operation;
				stencil_operation back_face_stencil_depth_fail_operation;
				stencil_operation back_face_stencil_pass_operation;
				comparison back_face_stencil_function;
				depth_write depth_write_mask;
				comparison depth_function;
				uint8_t stencil_read_mask;
				uint8_t stencil_write_mask;
				bool depth_enable;
				bool stencil_enable;
			};

		protected:
			desc state;

		protected:
			depth_stencil_state(const desc& i) noexcept;

		public:
			virtual ~depth_stencil_state() noexcept;
			virtual void* get_resource() const = 0;
			desc get_state() const;
		};

		class rasterizer_state : public core::reference<rasterizer_state>
		{
		public:
			struct desc
			{
				surface_fill fill_mode;
				vertex_cull cull_mode;
				float depth_bias_clamp;
				float slope_scaled_depth_bias;
				int depth_bias;
				bool front_counter_clockwise;
				bool depth_clip_enable;
				bool scissor_enable;
				bool multisample_enable;
				bool antialiased_line_enable;
			};

		protected:
			desc state;

		protected:
			rasterizer_state(const desc& i) noexcept;

		public:
			virtual ~rasterizer_state() noexcept;
			virtual void* get_resource() const = 0;
			desc get_state() const;
		};

		class blend_state : public core::reference<blend_state>
		{
		public:
			struct desc
			{
				render_target_blend_state render_target[8];
				bool alpha_to_coverage_enable;
				bool independent_blend_enable;
			};

		protected:
			desc state;

		protected:
			blend_state(const desc& i) noexcept;

		public:
			virtual ~blend_state() noexcept;
			virtual void* get_resource() const = 0;
			desc get_state() const;
		};

		class sampler_state : public core::reference<sampler_state>
		{
		public:
			struct desc
			{
				comparison comparison_function;
				texture_address address_u;
				texture_address address_v;
				texture_address address_w;
				pixel_filter filter;
				float mip_lod_bias;
				uint32_t max_anisotropy;
				float border_color[4];
				float min_lod;
				float max_lod;
			};

		protected:
			desc state;

		protected:
			sampler_state(const desc& i) noexcept;

		public:
			virtual ~sampler_state() noexcept;
			virtual void* get_resource() const = 0;
			desc get_state() const;
		};

		class input_layout : public core::reference<input_layout>
		{
		public:
			struct attribute
			{
				core::string semantic_name;
				uint32_t semantic_index;
				attribute_type format;
				uint32_t components;
				uint32_t aligned_byte_offset;
				uint32_t slot = 0;
				bool per_vertex = true;
			};

		public:
			struct desc
			{
				core::vector<attribute> attributes;
				shader* source = nullptr;
			};

		protected:
			core::vector<attribute> layout;

		protected:
			input_layout(const desc& i) noexcept;

		public:
			virtual ~input_layout() noexcept;
			virtual void* get_resource() const = 0;
			const core::vector<attribute>& get_attributes() const;
		};

		class shader : public core::reference<shader>
		{
		public:
			struct desc
			{
				compute::proc_include_callback include = nullptr;
				compute::preprocessor::desc features;
				core::vector<core::string> defines;
				core::string filename;
				core::string data;
				shader_type stage = shader_type::all;
			};

		protected:
			shader(const desc& i) noexcept;

		public:
			virtual ~shader() noexcept = default;
			virtual bool is_valid() const = 0;
		};

		class element_buffer : public core::reference<element_buffer>
		{
		public:
			struct desc
			{
				void* elements = nullptr;
				uint32_t structure_byte_stride = 0;
				uint32_t element_width = 0;
				uint32_t element_count = 0;
				cpu_access access_flags = cpu_access::none;
				resource_usage usage = resource_usage::defaults;
				resource_bind bind_flags = resource_bind::vertex_buffer;
				resource_misc misc_flags = resource_misc::none;
				bool writable = false;
			};

		protected:
			size_t elements;
			size_t stride;

		protected:
			element_buffer(const desc& i) noexcept;

		public:
			virtual ~element_buffer() noexcept = default;
			virtual void* get_resource() const = 0;
			size_t get_elements() const;
			size_t get_stride() const;
		};

		class mesh_buffer : public core::reference<mesh_buffer>
		{
		public:
			struct desc
			{
				core::vector<trigonometry::vertex> elements;
				core::vector<int> indices;
				cpu_access access_flags = cpu_access::none;
				resource_usage usage = resource_usage::defaults;
			};

		protected:
			element_buffer* vertex_buffer;
			element_buffer* index_buffer;

		public:
			trigonometry::matrix4x4 transform;
			core::string name;

		protected:
			mesh_buffer(const desc& i) noexcept;

		public:
			virtual ~mesh_buffer() noexcept;
			virtual core::unique<trigonometry::vertex> get_elements(graphics_device* device) const = 0;
			element_buffer* get_vertex_buffer() const;
			element_buffer* get_index_buffer() const;
		};

		class skin_mesh_buffer : public core::reference<skin_mesh_buffer>
		{
		public:
			struct desc
			{
				core::vector<trigonometry::skin_vertex> elements;
				core::vector<int> indices;
				cpu_access access_flags = cpu_access::none;
				resource_usage usage = resource_usage::defaults;
			};

		protected:
			element_buffer* vertex_buffer;
			element_buffer* index_buffer;

		public:
			trigonometry::matrix4x4 transform;
			core::unordered_map<size_t, size_t> joints;
			core::string name;

		protected:
			skin_mesh_buffer(const desc& i) noexcept;

		public:
			virtual ~skin_mesh_buffer() noexcept;
			virtual core::unique<trigonometry::skin_vertex> get_elements(graphics_device* device) const = 0;
			element_buffer* get_vertex_buffer() const;
			element_buffer* get_index_buffer() const;
		};

		class instance_buffer : public core::reference<instance_buffer>
		{
		public:
			struct desc
			{
				graphics_device* device = nullptr;
				uint32_t element_width = sizeof(trigonometry::element_vertex);
				uint32_t element_limit = 100;
			};

		protected:
			core::vector<trigonometry::element_vertex> array;
			element_buffer* elements;
			graphics_device* device;
			size_t element_limit;
			size_t element_width;
			bool sync;

		protected:
			instance_buffer(const desc& i) noexcept;

		public:
			virtual ~instance_buffer() noexcept;
			core::vector<trigonometry::element_vertex>& get_array();
			element_buffer* get_elements() const;
			graphics_device* get_device() const;
			size_t get_element_limit() const;
		};

		class texture_2d : public core::reference<texture_2d>
		{
		public:
			struct desc
			{
				cpu_access access_flags = cpu_access::none;
				format format_mode = format::r8g8b8a8_unorm;
				resource_usage usage = resource_usage::defaults;
				resource_bind bind_flags = resource_bind::shader_input;
				resource_misc misc_flags = resource_misc::none;
				void* data = nullptr;
				uint32_t row_pitch = 0;
				uint32_t depth_pitch = 0;
				uint32_t width = window_size;
				uint32_t height = window_size;
				int mip_levels = 1;
				bool writable = false;
			};

		protected:
			cpu_access access_flags;
			format format_mode;
			resource_usage usage;
			resource_bind binding;
			uint32_t width, height;
			uint32_t mip_levels;

		protected:
			texture_2d() noexcept;
			texture_2d(const desc& i) noexcept;

		public:
			virtual ~texture_2d() = default;
			virtual void* get_resource() const = 0;
			cpu_access get_access_flags() const;
			format get_format_mode() const;
			resource_usage get_usage() const;
			resource_bind get_binding() const;
			uint32_t get_width() const;
			uint32_t get_height() const;
			uint32_t get_mip_levels() const;
		};

		class texture_3d : public core::reference<texture_3d>
		{
		public:
			struct desc
			{
				cpu_access access_flags = cpu_access::none;
				format format_mode = format::r8g8b8a8_unorm;
				resource_usage usage = resource_usage::defaults;
				resource_bind bind_flags = resource_bind::shader_input;
				resource_misc misc_flags = resource_misc::none;
				uint32_t width = window_size;
				uint32_t height = window_size;
				uint32_t depth = 1;
				int mip_levels = 1;
				bool writable = false;
			};

		protected:
			cpu_access access_flags;
			format format_mode;
			resource_usage usage;
			resource_bind binding;
			uint32_t width, height;
			uint32_t mip_levels;
			uint32_t depth;

		protected:
			texture_3d();

		public:
			virtual ~texture_3d() = default;
			virtual void* get_resource() = 0;
			cpu_access get_access_flags() const;
			format get_format_mode() const;
			resource_usage get_usage() const;
			resource_bind get_binding() const;
			uint32_t get_width() const;
			uint32_t get_height() const;
			uint32_t get_depth() const;
			uint32_t get_mip_levels() const;
		};

		class texture_cube : public core::reference<texture_cube>
		{
		public:
			struct desc
			{
				cpu_access access_flags = cpu_access::none;
				format format_mode = format::r8g8b8a8_unorm;
				resource_usage usage = resource_usage::defaults;
				resource_bind bind_flags = resource_bind::shader_input;
				resource_misc misc_flags = resource_misc::texture_cube;
				uint32_t width = window_size;
				uint32_t height = window_size;
				int mip_levels = 1;
				bool writable = false;
			};

		protected:
			cpu_access access_flags;
			format format_mode;
			resource_usage usage;
			resource_bind binding;
			uint32_t width, height;
			uint32_t mip_levels;

		protected:
			texture_cube() noexcept;
			texture_cube(const desc& i) noexcept;

		public:
			virtual ~texture_cube() = default;
			virtual void* get_resource() const = 0;
			cpu_access get_access_flags() const;
			format get_format_mode() const;
			resource_usage get_usage() const;
			resource_bind get_binding() const;
			uint32_t get_width() const;
			uint32_t get_height() const;
			uint32_t get_mip_levels() const;
		};

		class depth_target_2d : public core::reference<depth_target_2d>
		{
		public:
			struct desc
			{
				cpu_access access_flags = cpu_access::none;
				resource_usage usage = resource_usage::defaults;
				format format_mode = format::d24_unorm_s8_uint;
				uint32_t width = window_size;
				uint32_t height = window_size;
			};

		protected:
			texture_2d* resource;
			viewport viewarea;

		protected:
			depth_target_2d(const desc& i) noexcept;

		public:
			virtual ~depth_target_2d() noexcept;
			virtual void* get_resource() const = 0;
			virtual uint32_t get_width() const = 0;
			virtual uint32_t get_height() const = 0;
			texture_2d* get_target();
			const graphics::viewport& get_viewport() const;
		};

		class depth_target_cube : public core::reference<depth_target_cube>
		{
		public:
			struct desc
			{
				cpu_access access_flags = cpu_access::none;
				resource_usage usage = resource_usage::defaults;
				format format_mode = format::d24_unorm_s8_uint;
				uint32_t size = 512;
			};

		protected:
			texture_cube* resource;
			viewport viewarea;

		protected:
			depth_target_cube(const desc& i) noexcept;

		public:
			virtual ~depth_target_cube() noexcept;
			virtual void* get_resource() const = 0;
			virtual uint32_t get_width() const = 0;
			virtual uint32_t get_height() const = 0;
			texture_cube* get_target();
			const graphics::viewport& get_viewport() const;
		};

		class render_target : public core::reference<render_target>
		{
		protected:
			texture_2d* depth_stencil;
			viewport viewarea;

		protected:
			render_target() noexcept;

		public:
			virtual ~render_target() noexcept;
			virtual void* get_target_buffer() const = 0;
			virtual void* get_depth_buffer() const = 0;
			virtual uint32_t get_width() const = 0;
			virtual uint32_t get_height() const = 0;
			virtual uint32_t get_target_count() const = 0;
			virtual texture_2d* get_target_2d(uint32_t index) = 0;
			virtual texture_cube* get_target_cube(uint32_t index) = 0;
			texture_2d* get_depth_stencil();
			const graphics::viewport& get_viewport() const;
		};

		class render_target_2d : public render_target
		{
		public:
			struct desc
			{
				cpu_access access_flags = cpu_access::none;
				format format_mode = format::r8g8b8a8_unorm;
				resource_usage usage = resource_usage::defaults;
				resource_bind bind_flags = (resource_bind)(resource_bind::render_target | resource_bind::shader_input);
				resource_misc misc_flags = resource_misc::none;
				void* render_surface = nullptr;
				uint32_t width = window_size;
				uint32_t height = window_size;
				uint32_t mip_levels = 1;
				bool depth_stencil = true;
			};

		protected:
			texture_2d* resource;

		protected:
			render_target_2d(const desc& i) noexcept;

		public:
			virtual ~render_target_2d() noexcept;
			virtual void* get_target_buffer() const = 0;
			virtual void* get_depth_buffer() const = 0;
			virtual uint32_t get_width() const = 0;
			virtual uint32_t get_height() const = 0;
			uint32_t get_target_count() const;
			texture_2d* get_target_2d(uint32_t index);
			texture_cube* get_target_cube(uint32_t index);
			texture_2d* get_target();
		};

		class multi_render_target_2d : public render_target
		{
		public:
			struct desc
			{
				cpu_access access_flags = cpu_access::none;
				surface_target target = surface_target::t0;
				format format_mode[8] = { format::r8g8b8a8_unorm, format::unknown, format::unknown, format::unknown, format::unknown, format::unknown, format::unknown, format::unknown };
				resource_usage usage = resource_usage::defaults;
				resource_bind bind_flags = (resource_bind)(resource_bind::render_target | resource_bind::shader_input);
				resource_misc misc_flags = resource_misc::none;
				uint32_t width = window_size;
				uint32_t height = window_size;
				uint32_t mip_levels = 1;
				bool depth_stencil = true;
			};

		protected:
			surface_target target;
			texture_2d* resource[8];

		protected:
			multi_render_target_2d(const desc& i) noexcept;

		public:
			virtual ~multi_render_target_2d() noexcept;
			virtual void* get_target_buffer() const = 0;
			virtual void* get_depth_buffer() const = 0;
			virtual uint32_t get_width() const = 0;
			virtual uint32_t get_height() const = 0;
			uint32_t get_target_count() const;
			texture_2d* get_target_2d(uint32_t index);
			texture_cube* get_target_cube(uint32_t index);
			texture_2d* get_target(uint32_t index);
		};

		class render_target_cube : public render_target
		{
		public:
			struct desc
			{
				cpu_access access_flags = cpu_access::none;
				format format_mode = format::r8g8b8a8_unorm;
				resource_usage usage = resource_usage::defaults;
				resource_bind bind_flags = (resource_bind)(resource_bind::render_target | resource_bind::shader_input);
				resource_misc misc_flags = (resource_misc)(resource_misc::generate_mips | resource_misc::texture_cube);
				uint32_t size = 512;
				uint32_t mip_levels = 1;
				bool depth_stencil = true;
			};

		protected:
			texture_cube* resource;

		protected:
			render_target_cube(const desc& i) noexcept;

		public:
			virtual ~render_target_cube() noexcept;
			virtual void* get_target_buffer() const = 0;
			virtual void* get_depth_buffer() const = 0;
			virtual uint32_t get_width() const = 0;
			virtual uint32_t get_height() const = 0;
			uint32_t get_target_count() const;
			texture_2d* get_target_2d(uint32_t index);
			texture_cube* get_target_cube(uint32_t index);
			texture_cube* get_target();
		};

		class multi_render_target_cube : public render_target
		{
		public:
			struct desc
			{
				cpu_access access_flags = cpu_access::none;
				surface_target target = surface_target::t0;
				format format_mode[8] = { format::r8g8b8a8_unorm, format::unknown, format::unknown, format::unknown, format::unknown, format::unknown, format::unknown, format::unknown };
				resource_usage usage = resource_usage::defaults;
				resource_bind bind_flags = (resource_bind)(resource_bind::render_target | resource_bind::shader_input);
				resource_misc misc_flags = (resource_misc)(resource_misc::generate_mips | resource_misc::texture_cube);
				uint32_t mip_levels = 1;
				uint32_t size = 512;
				bool depth_stencil = true;
			};

		protected:
			surface_target target;
			texture_cube* resource[8];

		protected:
			multi_render_target_cube(const desc& i) noexcept;

		public:
			virtual ~multi_render_target_cube() noexcept;
			virtual void* get_target_buffer() const = 0;
			virtual void* get_depth_buffer() const = 0;
			virtual uint32_t get_width() const = 0;
			virtual uint32_t get_height() const = 0;
			uint32_t get_target_count() const;
			texture_2d* get_target_2d(uint32_t index);
			texture_cube* get_target_cube(uint32_t index);
			texture_cube* get_target(uint32_t index);
		};

		class cubemap : public core::reference<cubemap>
		{
		public:
			struct desc
			{
				render_target* source = nullptr;
				uint32_t target = 0;
				uint32_t mip_levels = 0;
				uint32_t size = 512;
			};

		protected:
			texture_cube* dest;
			desc meta;

		protected:
			cubemap(const desc& i) noexcept;

		public:
			virtual ~cubemap() noexcept = default;
			bool is_valid() const;
		};

		class query : public core::reference<query>
		{
		public:
			struct desc
			{
				bool predicate = true;
				bool auto_pass = false;
			};

		protected:
			query() noexcept;

		public:
			virtual ~query() noexcept = default;
			virtual void* get_resource() const = 0;
		};

		class graphics_device : public core::reference<graphics_device>
		{
		protected:
			struct direct_buffer
			{
				trigonometry::matrix4x4 transform;
				trigonometry::vector4 padding;
			};

			struct vertex
			{
				float px, py, pz;
				float tx, ty;
				float cx, cy, cz, cw;
			};

		public:
			struct desc
			{
				render_backend backend = render_backend::automatic;
				shader_model shader_mode = shader_model::any;
				format buffer_format = format::r8g8b8a8_unorm;
				vsync vsync_mode = vsync::frequency_x1;
				core::string cache_directory = "./shaders";
				int is_windowed = 1;
				bool shader_cache = true;
				bool debug = false;
				bool blit_rendering = false;
				uint32_t presentation_flags = 0;
				uint32_t compilation_flags = (uint32_t)(shader_compile::enable_strictness | shader_compile::optimization_level3 | shader_compile::matrix_row_major);
				uint32_t creation_flags = 0;
				uint32_t buffer_width = window_size;
				uint32_t buffer_height = window_size;
				activity* window = nullptr;
			};

			struct section
			{
				core::string name;
				core::string code;
			};

		protected:
			core::unordered_map<core::string, depth_stencil_state*> depth_stencil_states;
			core::unordered_map<core::string, rasterizer_state*> rasterizer_states;
			core::unordered_map<core::string, blend_state*> blend_states;
			core::unordered_map<core::string, sampler_state*> sampler_states;
			core::unordered_map<core::string, input_layout*> input_layouts;
			core::unordered_map<core::string, section*> sections;
			core::single_queue<render_thread_callback> queue;
			std::thread::id render_thread;
			primitive_topology primitives;
			shader_model shader_gen;
			texture_2d* view_resource = nullptr;
			render_target_2d* render_target = nullptr;
			activity* virtual_window = nullptr;
			uint32_t present_flags = 0;
			uint32_t compile_flags = 0;
			vsync vsync_mode = vsync::frequency_x1;
			core::vector<vertex> elements;
			core::string caches;
			size_t max_elements;
			render_backend backend;
			direct_buffer direct;
			std::recursive_mutex exclusive;
			bool shader_cache;
			bool debug;

		protected:
			graphics_device(const desc& i) noexcept;

		public:
			virtual ~graphics_device() noexcept;
			virtual void set_as_current_device() = 0;
			virtual void set_shader_model(shader_model model) = 0;
			virtual void set_blend_state(blend_state* state) = 0;
			virtual void set_rasterizer_state(rasterizer_state* state) = 0;
			virtual void set_depth_stencil_state(depth_stencil_state* state) = 0;
			virtual void set_input_layout(input_layout* state) = 0;
			virtual expects_graphics<void> set_shader(shader* resource, uint32_t type) = 0;
			virtual void set_sampler_state(sampler_state* state, uint32_t slot, uint32_t count, uint32_t type) = 0;
			virtual void set_buffer(shader* resource, uint32_t slot, uint32_t type) = 0;
			virtual void set_buffer(instance_buffer* resource, uint32_t slot, uint32_t type) = 0;
			virtual void set_constant_buffer(element_buffer* resource, uint32_t slot, uint32_t type) = 0;
			virtual void set_structure_buffer(element_buffer* resource, uint32_t slot, uint32_t type) = 0;
			virtual void set_texture_2d(texture_2d* resource, uint32_t slot, uint32_t type) = 0;
			virtual void set_texture_3d(texture_3d* resource, uint32_t slot, uint32_t type) = 0;
			virtual void set_texture_cube(texture_cube* resource, uint32_t slot, uint32_t type) = 0;
			virtual void set_index_buffer(element_buffer* resource, format format_mode) = 0;
			virtual void set_vertex_buffers(element_buffer** resources, uint32_t count, bool dynamic_linkage = false) = 0;
			virtual void set_writeable(element_buffer** resource, uint32_t slot, uint32_t count, bool computable) = 0;
			virtual void set_writeable(texture_2d** resource, uint32_t slot, uint32_t count, bool computable) = 0;
			virtual void set_writeable(texture_3d** resource, uint32_t slot, uint32_t count, bool computable) = 0;
			virtual void set_writeable(texture_cube** resource, uint32_t slot, uint32_t count, bool computable) = 0;
			virtual void set_target(float r, float g, float b) = 0;
			virtual void set_target() = 0;
			virtual void set_target(depth_target_2d* resource) = 0;
			virtual void set_target(depth_target_cube* resource) = 0;
			virtual void set_target(graphics::render_target* resource, uint32_t target, float r, float g, float b) = 0;
			virtual void set_target(graphics::render_target* resource, uint32_t target) = 0;
			virtual void set_target(graphics::render_target* resource, float r, float g, float b) = 0;
			virtual void set_target(graphics::render_target* resource) = 0;
			virtual void set_target_map(graphics::render_target* resource, bool enabled[8]) = 0;
			virtual void set_target_rect(uint32_t width, uint32_t height) = 0;
			virtual void set_viewports(uint32_t count, viewport* viewports) = 0;
			virtual void set_scissor_rects(uint32_t count, trigonometry::rectangle* value) = 0;
			virtual void set_primitive_topology(primitive_topology topology) = 0;
			virtual void flush_texture(uint32_t slot, uint32_t count, uint32_t type) = 0;
			virtual void flush_state() = 0;
			virtual void clear_buffer(instance_buffer* resource) = 0;
			virtual void clear_writable(texture_2d* resource) = 0;
			virtual void clear_writable(texture_2d* resource, float r, float g, float b) = 0;
			virtual void clear_writable(texture_3d* resource) = 0;
			virtual void clear_writable(texture_3d* resource, float r, float g, float b) = 0;
			virtual void clear_writable(texture_cube* resource) = 0;
			virtual void clear_writable(texture_cube* resource, float r, float g, float b) = 0;
			virtual void clear(float r, float g, float b) = 0;
			virtual void clear(graphics::render_target* resource, uint32_t target, float r, float g, float b) = 0;
			virtual void clear_depth() = 0;
			virtual void clear_depth(depth_target_2d* resource) = 0;
			virtual void clear_depth(depth_target_cube* resource) = 0;
			virtual void clear_depth(graphics::render_target* resource) = 0;
			virtual void draw_indexed(uint32_t count, uint32_t index_location, uint32_t base_location) = 0;
			virtual void draw_indexed(mesh_buffer* resource) = 0;
			virtual void draw_indexed(skin_mesh_buffer* resource) = 0;
			virtual void draw_indexed_instanced(uint32_t index_count_per_instance, uint32_t instance_count, uint32_t index_location, uint32_t vertex_location, uint32_t instance_location) = 0;
			virtual void draw_indexed_instanced(element_buffer* instances, mesh_buffer* resource, uint32_t instance_count) = 0;
			virtual void draw_indexed_instanced(element_buffer* instances, skin_mesh_buffer* resource, uint32_t instance_count) = 0;
			virtual void draw(uint32_t count, uint32_t location) = 0;
			virtual void draw_instanced(uint32_t vertex_count_per_instance, uint32_t instance_count, uint32_t vertex_location, uint32_t instance_location) = 0;
			virtual void dispatch(uint32_t group_x, uint32_t group_y, uint32_t group_z) = 0;
			virtual void get_viewports(uint32_t* count, viewport* out) = 0;
			virtual void get_scissor_rects(uint32_t* count, trigonometry::rectangle* out) = 0;
			virtual void query_begin(query* resource) = 0;
			virtual void query_end(query* resource) = 0;
			virtual void generate_mips(texture_2d* resource) = 0;
			virtual void generate_mips(texture_3d* resource) = 0;
			virtual void generate_mips(texture_cube* resource) = 0;
			virtual bool im_begin() = 0;
			virtual void im_transform(const trigonometry::matrix4x4& transform) = 0;
			virtual void im_topology(primitive_topology topology) = 0;
			virtual void im_emit() = 0;
			virtual void im_texture(texture_2d* in) = 0;
			virtual void im_color(float x, float y, float z, float w) = 0;
			virtual void im_intensity(float intensity) = 0;
			virtual void im_texcoord(float x, float y) = 0;
			virtual void im_texcoord_offset(float x, float y) = 0;
			virtual void im_position(float x, float y, float z) = 0;
			virtual bool im_end() = 0;
			virtual bool has_explicit_slots() const = 0;
			virtual expects_graphics<uint32_t> get_shader_slot(shader* resource, const std::string_view& name) const = 0;
			virtual expects_graphics<uint32_t> get_shader_sampler_slot(shader* resource, const std::string_view& resource_name, const std::string_view& sampler_name) const = 0;
			virtual expects_graphics<void> submit() = 0;
			virtual expects_graphics<void> map(element_buffer* resource, resource_map mode, mapped_subresource* map) = 0;
			virtual expects_graphics<void> map(texture_2d* resource, resource_map mode, mapped_subresource* map) = 0;
			virtual expects_graphics<void> map(texture_3d* resource, resource_map mode, mapped_subresource* map) = 0;
			virtual expects_graphics<void> map(texture_cube* resource, resource_map mode, mapped_subresource* map) = 0;
			virtual expects_graphics<void> unmap(texture_2d* resource, mapped_subresource* map) = 0;
			virtual expects_graphics<void> unmap(texture_3d* resource, mapped_subresource* map) = 0;
			virtual expects_graphics<void> unmap(texture_cube* resource, mapped_subresource* map) = 0;
			virtual expects_graphics<void> unmap(element_buffer* resource, mapped_subresource* map) = 0;
			virtual expects_graphics<void> update_constant_buffer(element_buffer* resource, void* data, size_t size) = 0;
			virtual expects_graphics<void> update_buffer(element_buffer* resource, void* data, size_t size) = 0;
			virtual expects_graphics<void> update_buffer(shader* resource, const void* data) = 0;
			virtual expects_graphics<void> update_buffer(mesh_buffer* resource, trigonometry::vertex* data) = 0;
			virtual expects_graphics<void> update_buffer(skin_mesh_buffer* resource, trigonometry::skin_vertex* data) = 0;
			virtual expects_graphics<void> update_buffer(instance_buffer* resource) = 0;
			virtual expects_graphics<void> update_buffer_size(shader* resource, size_t size) = 0;
			virtual expects_graphics<void> update_buffer_size(instance_buffer* resource, size_t size) = 0;
			virtual expects_graphics<void> copy_texture_2d(texture_2d* resource, core::unique<texture_2d>* result) = 0;
			virtual expects_graphics<void> copy_texture_2d(graphics::render_target* resource, uint32_t target, core::unique<texture_2d>* result) = 0;
			virtual expects_graphics<void> copy_texture_2d(render_target_cube* resource, trigonometry::cube_face face, core::unique<texture_2d>* result) = 0;
			virtual expects_graphics<void> copy_texture_2d(multi_render_target_cube* resource, uint32_t cube, trigonometry::cube_face face, core::unique<texture_2d>* result) = 0;
			virtual expects_graphics<void> copy_texture_cube(render_target_cube* resource, core::unique<texture_cube>* result) = 0;
			virtual expects_graphics<void> copy_texture_cube(multi_render_target_cube* resource, uint32_t cube, core::unique<texture_cube>* result) = 0;
			virtual expects_graphics<void> copy_target(graphics::render_target* from, uint32_t from_target, graphics::render_target* to, uint32_t to_target) = 0;
			virtual expects_graphics<void> copy_back_buffer(core::unique<texture_2d>* result) = 0;
			virtual expects_graphics<void> cubemap_push(cubemap* resource, texture_cube* result) = 0;
			virtual expects_graphics<void> cubemap_face(cubemap* resource, trigonometry::cube_face face) = 0;
			virtual expects_graphics<void> cubemap_pop(cubemap* resource) = 0;
			virtual expects_graphics<void> rescale_buffers(uint32_t width, uint32_t height) = 0;
			virtual expects_graphics<void> resize_buffers(uint32_t width, uint32_t height) = 0;
			virtual expects_graphics<void> generate_texture(texture_2d* resource) = 0;
			virtual expects_graphics<void> generate_texture(texture_3d* resource) = 0;
			virtual expects_graphics<void> generate_texture(texture_cube* resource) = 0;
			virtual expects_graphics<void> get_query_data(query* resource, size_t* result, bool flush = true) = 0;
			virtual expects_graphics<void> get_query_data(query* resource, bool* result, bool flush = true) = 0;
			virtual expects_graphics<core::unique<depth_stencil_state>> create_depth_stencil_state(const depth_stencil_state::desc& i) = 0;
			virtual expects_graphics<core::unique<blend_state>> create_blend_state(const blend_state::desc& i) = 0;
			virtual expects_graphics<core::unique<rasterizer_state>> create_rasterizer_state(const rasterizer_state::desc& i) = 0;
			virtual expects_graphics<core::unique<sampler_state>> create_sampler_state(const sampler_state::desc& i) = 0;
			virtual expects_graphics<core::unique<input_layout>> create_input_layout(const input_layout::desc& i) = 0;
			virtual expects_graphics<core::unique<shader>> create_shader(const shader::desc& i) = 0;
			virtual expects_graphics<core::unique<element_buffer>> create_element_buffer(const element_buffer::desc& i) = 0;
			virtual expects_graphics<core::unique<mesh_buffer>> create_mesh_buffer(const mesh_buffer::desc& i) = 0;
			virtual expects_graphics<core::unique<mesh_buffer>> create_mesh_buffer(element_buffer* vertex_buffer, element_buffer* index_buffer) = 0;
			virtual expects_graphics<core::unique<skin_mesh_buffer>> create_skin_mesh_buffer(const skin_mesh_buffer::desc& i) = 0;
			virtual expects_graphics<core::unique<skin_mesh_buffer>> create_skin_mesh_buffer(element_buffer* vertex_buffer, element_buffer* index_buffer) = 0;
			virtual expects_graphics<core::unique<instance_buffer>> create_instance_buffer(const instance_buffer::desc& i) = 0;
			virtual expects_graphics<core::unique<texture_2d>> create_texture_2d() = 0;
			virtual expects_graphics<core::unique<texture_2d>> create_texture_2d(const texture_2d::desc& i) = 0;
			virtual expects_graphics<core::unique<texture_3d>> create_texture_3d() = 0;
			virtual expects_graphics<core::unique<texture_3d>> create_texture_3d(const texture_3d::desc& i) = 0;
			virtual expects_graphics<core::unique<texture_cube>> create_texture_cube() = 0;
			virtual expects_graphics<core::unique<texture_cube>> create_texture_cube(const texture_cube::desc& i) = 0;
			virtual expects_graphics<core::unique<texture_cube>> create_texture_cube(texture_2d* resource[6]) = 0;
			virtual expects_graphics<core::unique<texture_cube>> create_texture_cube(texture_2d* resource) = 0;
			virtual expects_graphics<core::unique<depth_target_2d>> create_depth_target_2d(const depth_target_2d::desc& i) = 0;
			virtual expects_graphics<core::unique<depth_target_cube>> create_depth_target_cube(const depth_target_cube::desc& i) = 0;
			virtual expects_graphics<core::unique<render_target_2d>> create_render_target_2d(const render_target_2d::desc& i) = 0;
			virtual expects_graphics<core::unique<multi_render_target_2d>> create_multi_render_target_2d(const multi_render_target_2d::desc& i) = 0;
			virtual expects_graphics<core::unique<render_target_cube>> create_render_target_cube(const render_target_cube::desc& i) = 0;
			virtual expects_graphics<core::unique<multi_render_target_cube>> create_multi_render_target_cube(const multi_render_target_cube::desc& i) = 0;
			virtual expects_graphics<core::unique<cubemap>> create_cubemap(const cubemap::desc& i) = 0;
			virtual expects_graphics<core::unique<query>> create_query(const query::desc& i) = 0;
			virtual primitive_topology get_primitive_topology() const = 0;
			virtual shader_model get_supported_shader_model()  const = 0;
			virtual void* get_device() const = 0;
			virtual void* get_context() const = 0;
			virtual bool is_valid() const = 0;
			virtual void set_vsync_mode(vsync mode);
			void set_vertex_buffer(element_buffer* resource);
			void set_shader_cache(bool enabled);
			void lockup(render_thread_callback&& callback);
			void enqueue(render_thread_callback&& callback);
			compute::expects_preprocessor<void> preprocess(shader::desc& subresult);
			expects_graphics<void> transpile(core::string* hlsl, shader_type stage, shader_lang to);
			expects_graphics<void> get_section_info(const std::string_view& name, section** result);
			expects_graphics<void> get_section_data(const std::string_view& name, shader::desc* result);
			bool add_section(const std::string_view& name, const std::string_view& code);
			bool remove_section(const std::string_view& name);
			bool is_left_handed() const;
			core::string get_shader_main(shader_type type) const;
			const core::unordered_map<core::string, depth_stencil_state*>& get_depth_stencil_states() const;
			const core::unordered_map<core::string, rasterizer_state*>& get_rasterizer_states() const;
			const core::unordered_map<core::string, blend_state*>& get_blend_states() const;
			const core::unordered_map<core::string, sampler_state*>& get_sampler_states() const;
			const core::unordered_map<core::string, input_layout*>& get_input_layouts() const;
			expects_video<core::unique<surface>> create_surface(texture_2d* base);
			depth_stencil_state* get_depth_stencil_state(const std::string_view& name);
			blend_state* get_blend_state(const std::string_view& name);
			rasterizer_state* get_rasterizer_state(const std::string_view& name);
			sampler_state* get_sampler_state(const std::string_view& name);
			input_layout* get_input_layout(const std::string_view& name);
			shader_model get_shader_model() const;
			render_target_2d* get_render_target();
			render_backend get_backend() const;
			uint32_t get_format_size(format mode) const;
			uint32_t get_present_flags() const;
			uint32_t get_compile_flags() const;
			uint32_t get_row_pitch(uint32_t width, uint32_t element_size = sizeof(uint8_t) * 4) const;
			uint32_t get_depth_pitch(uint32_t row_pitch, uint32_t height) const;
			uint32_t get_mip_level(uint32_t width, uint32_t height) const;
			core::option<core::string> get_program_name(const shader::desc& desc);
			vsync get_vsync_mode() const;
			bool is_debug() const;

		protected:
			virtual expects_graphics<texture_cube*> create_texture_cube_internal(void* resource[6]) = 0;
			bool get_program_cache(const std::string_view& name, core::string* data);
			bool set_program_cache(const std::string_view& name, const std::string_view& data);
			void create_states();
			void create_sections();
			void release_proxy();
			void dispatch_queue();

		public:
			static graphics_device* create(desc& i);
			static expects_graphics<void> compile_builtin_shaders(const core::vector<graphics_device*>& devices, const std::function<bool(graphics_device*, const std::string_view&, const expects_graphics<shader*>&)>& callback);
		};

		class activity final : public core::reference<activity>
		{
		public:
			struct desc
			{
				core::string title = "Activity";
				uint32_t inactive_sleep_ms = 66;
				uint32_t width = 0;
				uint32_t height = 0;
				int32_t x = 0, y = 0;
				bool fullscreen = false;
				bool hidden = true;
				bool borderless = false;
				bool resizable = true;
				bool minimized = false;
				bool maximized = false;
				bool centered = false;
				bool free_position = false;
				bool focused = false;
				bool render_even_if_inactive = false;
				bool gpu_as_renderer = true;
				bool high_dpi = true;
			};

			struct
			{
				app_state_change_callback app_state_change = nullptr;
				window_state_change_callback window_state_change = nullptr;
				key_state_callback key_state = nullptr;
				input_edit_callback input_edit = nullptr;
				input_callback input = nullptr;
				cursor_move_callback cursor_move = nullptr;
				cursor_wheel_state_callback cursor_wheel_state = nullptr;
				joy_stick_axis_move_callback joy_stick_axis_move = nullptr;
				joy_stick_ball_move_callback joy_stick_ball_move = nullptr;
				joy_stick_hat_move_callback joy_stick_hat_move = nullptr;
				joy_stick_key_state_callback joy_stick_key_state = nullptr;
				joy_stick_state_callback joy_stick_state = nullptr;
				controller_axis_move_callback controller_axis_move = nullptr;
				controller_key_state_callback controller_key_state = nullptr;
				controller_state_callback controller_state = nullptr;
				touch_move_callback touch_move = nullptr;
				touch_state_callback touch_state = nullptr;
				gesture_state_callback gesture_state = nullptr;
				multi_gesture_state_callback multi_gesture_state = nullptr;
				drop_file_callback drop_file = nullptr;
				drop_text_callback drop_text = nullptr;
			} callbacks;

		private:
			struct
			{
				bool captured = false;
				bool mapped = false;
				bool enabled = false;
				key_map key;
			} mapping;

		private:
			event_consumers event_source;
			SDL_Cursor* cursors[(size_t)display_cursor::count];
			SDL_Window* handle;
			SDL_Surface* favicon;
			desc options;
			bool keys[2][1024];
			int command, cx, cy;

		public:
			void* user_pointer = nullptr;
			alert message;

		public:
			activity(const desc& i) noexcept;
			virtual ~activity() noexcept;
			void set_clipboard_text(const std::string_view& text);
			void set_cursor_position(const trigonometry::vector2& position);
			void set_cursor_position(float x, float y);
			void set_global_cursor_position(const trigonometry::vector2& position);
			void set_global_cursor_position(float x, float y);
			void set_key(key_code key_code, bool value);
			void set_cursor(display_cursor style);
			void set_cursor_visibility(bool enabled);
			void set_grabbing(bool enabled);
			void set_fullscreen(bool enabled);
			void set_borderless(bool enabled);
			void set_icon(surface* icon);
			void set_title(const std::string_view& value);
			void set_screen_keyboard(bool enabled);
			void apply_configuration(render_backend backend);
			void wakeup();
			void hide();
			void show();
			void maximize();
			void minimize();
			void focus();
			void move(int x, int y);
			void resize(int width, int height);
			void load(SDL_SysWMinfo* base);
			bool capture_key_map(key_map* value);
			bool dispatch(uint64_t timeout_ms = 0, bool dispatch_all = true);
			bool is_fullscreen() const;
			bool is_any_key_down() const;
			bool is_key_down(const key_map& key) const;
			bool is_key_up(const key_map& key) const;
			bool is_key_down_hit(const key_map& key) const;
			bool is_key_up_hit(const key_map& key) const;
			bool is_screen_keyboard_enabled() const;
			uint32_t get_x() const;
			uint32_t get_y() const;
			uint32_t get_width() const;
			uint32_t get_height() const;
			uint32_t get_id() const;
			float get_aspect_ratio() const;
			key_mod get_key_mod_state() const;
			graphics::viewport get_viewport() const;
			trigonometry::vector2 get_offset() const;
			trigonometry::vector2 get_size() const;
			trigonometry::vector2 get_client_size() const;
			trigonometry::vector2 get_drawable_size(uint32_t width, uint32_t height) const;
			trigonometry::vector2 get_global_cursor_position() const;
			trigonometry::vector2 get_cursor_position() const;
			trigonometry::vector2 get_cursor_position(float screen_width, float screen_height) const;
			trigonometry::vector2 get_cursor_position(const trigonometry::vector2& screen_dimensions) const;
			core::string get_clipboard_text() const;
			SDL_Window* get_handle() const;
			core::string get_error() const;
			desc& get_options();

		public:
			static bool multi_dispatch(const event_consumers& sources, uint64_t timeout_ms = 0, bool dispatch_all = true);

		private:
			bool apply_system_theme();
			bool* get_input_state();
		};

		class alerts
		{
		public:
			static bool text(const std::string_view& title, const std::string_view& message, const std::string_view& default_input, core::string* result);
			static bool password(const std::string_view& title, const std::string_view& message, core::string* result);
			static bool save(const std::string_view& title, const std::string_view& default_path, const std::string_view& filter, const std::string_view& filter_description, core::string* result);
			static bool open(const std::string_view& title, const std::string_view& default_path, const std::string_view& filter, const std::string_view& filter_description, bool multiple, core::string* result);
			static bool folder(const std::string_view& title, const std::string_view& default_path, core::string* result);
			static bool color(const std::string_view& title, const std::string_view& default_hex_rgb, core::string* result);
		};

		class video : public core::singletonish
		{
		public:
			class windows
			{
			public:
				static void* get_hdc(activity* target);
				static void* get_hinstance(activity* target);
				static void* get_hwnd(activity* target);
			};

			class win_rt
			{
			public:
				static void* get_iinspectable(activity* target);
			};

			class x11
			{
			public:
				static void* get_display(activity* target);
				static size_t get_window(activity* target);
			};

			class direct_fb
			{
			public:
				static void* get_idirect_fb(activity* target);
				static void* get_idirect_fb_window(activity* target);
				static void* get_idirect_fb_surface(activity* target);
			};

			class cocoa
			{
			public:
				static void* get_ns_window(activity* target);
			};

			class ui_kit
			{
			public:
				static void* get_ui_window(activity* target);
			};

			class wayland
			{
			public:
				static void* get_wl_display(activity* target);
				static void* get_wl_surface(activity* target);
				static void* get_wl_egl_window(activity* target);
				static void* get_xdg_surface(activity* target);
				static void* get_xdg_top_level(activity* target);
				static void* get_xdg_popup(activity* target);
				static void* get_xdg_positioner(activity* target);
			};

			class android
			{
			public:
				static void* get_anative_window(activity* target);
			};

			class os2
			{
			public:
				static void* get_hwnd(activity* target);
				static void* get_hwnd_frame(activity* target);
			};

			class glew
			{
			public:
				static bool set_swap_interval(int32_t interval);
				static bool set_swap_parameters(int32_t r, int32_t g, int32_t b, int32_t a, bool debugging);
				static bool set_context(activity* target, void* context);
				static bool perform_swap(activity* target);
				static void* create_context(activity* target);
				static void destroy_context(void* context);
			};

		public:
			static uint32_t get_display_count();
			static bool get_display_info(uint32_t display_index, display_info* info);
			static std::string_view get_key_code_as_literal(key_code code);
			static std::string_view get_key_mod_as_literal(key_mod code);
			static core::string get_key_code_as_string(key_code code);
			static core::string get_key_mod_as_string(key_mod code);
		};
	}
}
#endif
