#ifndef VI_AUDIO_H
#define VI_AUDIO_H
#include "trigonometry.h"

namespace vitex
{
	namespace audio
	{
		class audio_source;

		class audio_effect;

		enum class sound_distance_model
		{
			invalid = 0,
			invert = 0xD001,
			invert_clamp = 0xD002,
			linear = 0xD003,
			linear_clamp = 0xD004,
			exponent = 0xD005,
			exponent_clamp = 0xD006,
		};

		enum class sound_ex
		{
			source_relative = 0x202,
			cone_inner_angle = 0x1001,
			cone_outer_angle = 0x1002,
			pitch = 0x1003,
			position = 0x1004,
			direction = 0x1005,
			velocity = 0x1006,
			looping = 0x1007,
			buffer = 0x1009,
			gain = 0x100A,
			min_gain = 0x100D,
			max_gain = 0x100E,
			orientation = 0x100F,
			channel_mask = 0x3000,
			source_state = 0x1010,
			initial = 0x1011,
			playing = 0x1012,
			paused = 0x1013,
			stopped = 0x1014,
			buffers_queued = 0x1015,
			buffers_processed = 0x1016,
			seconds_offset = 0x1024,
			sample_offset = 0x1025,
			byte_offset = 0x1026,
			source_type = 0x1027,
			constant = 0x1028,
			streaming = 0x1029,
			undetermined = 0x1030,
			format_mono8 = 0x1100,
			format_mono16 = 0x1101,
			format_stereo8 = 0x1102,
			format_stereo16 = 0x1103,
			reference_distance = 0x1020,
			rolloff_factor = 0x1021,
			cone_outer_gain = 0x1022,
			max_distance = 0x1023,
			frequency = 0x2001,
			bits = 0x2002,
			channels = 0x2003,
			size = 0x2004,
			unused = 0x2010,
			pending = 0x2011,
			processed = 0x2012,
			invalid_name = 0xA001,
			illegal_enum = 0xA002,
			invalid_enum = 0xA002,
			invalid_value = 0xA003,
			illegal_command = 0xA004,
			invalid_operation = 0xA004,
			out_of_memory = 0xA005,
			vendor = 0xB001,
			version = 0xB002,
			renderer = 0xB003,
			extentions = 0xB004,
			doppler_factor = 0xC000,
			doppler_velocity = 0xC001,
			speed_of_sound = 0xC003
		};

		enum class filter_ex
		{
			lowpass_gain = 0x0001,
			lowpass_gain_hf = 0x0002,
			highpass_gain = 0x0001,
			highpass_gain_lf = 0x0002,
			bandpass_gain = 0x0001,
			bandpass_gain_lf = 0x0002,
			bandpass_gain_hf = 0x0003,
			filter_type = 0x8001,
			filter_null = 0x0000,
			filter_lowpass = 0x0001,
			filter_highpass = 0x0002,
			filter_bandpass = 0x0003
		};

		enum class effect_ex
		{
			reverb_density = 0x0001,
			reverb_diffusion = 0x0002,
			reverb_gain = 0x0003,
			reverb_gain_hf = 0x0004,
			reverb_decay_time = 0x0005,
			reverb_decay_hf_ratio = 0x0006,
			reverb_reflections_gain = 0x0007,
			reverb_reflections_delay = 0x0008,
			reverb_late_reverb_gain = 0x0009,
			reverb_late_reverb_delay = 0x000A,
			reverb_air_absorption_gain_hf = 0x000B,
			reverb_room_rolloff_factor = 0x000C,
			reverb_decay_hf_limit = 0x000D,
			eax_reverb_density = 0x0001,
			eax_reverb_diffusion = 0x0002,
			eax_reverb_gain = 0x0003,
			eax_reverb_gain_hf = 0x0004,
			eax_reverb_gain_lf = 0x0005,
			eax_reverb_decay_time = 0x0006,
			eax_reverb_decay_hf_ratio = 0x0007,
			eax_reverb_decay_lf_ratio = 0x0008,
			eax_reverb_reflections_gain = 0x0009,
			eax_reverb_reflections_delay = 0x000A,
			eax_reverb_reflections_pan = 0x000B,
			eax_reverb_late_reverb_gain = 0x000C,
			eax_reverb_late_reverb_delay = 0x000D,
			eax_reverb_late_reverb_pan = 0x000E,
			eax_reverb_echo_time = 0x000F,
			eax_reverb_echo_depth = 0x0010,
			eax_reverb_modulation_time = 0x0011,
			eax_reverb_modulation_depth = 0x0012,
			eax_reverb_air_absorption_gain_hf = 0x0013,
			eax_reverb_hf_reference = 0x0014,
			eax_reverb_lf_reference = 0x0015,
			eax_reverb_room_rolloff_factor = 0x0016,
			eax_reverb_decay_hf_limit = 0x0017,
			chorus_waveform = 0x0001,
			chorus_phase = 0x0002,
			chorus_rate = 0x0003,
			chorus_depth = 0x0004,
			chorus_feedback = 0x0005,
			chorus_delay = 0x0006,
			distortion_edge = 0x0001,
			distortion_gain = 0x0002,
			distortion_lowpass_cutoff = 0x0003,
			distortion_eq_center = 0x0004,
			distortion_eq_bandwidth = 0x0005,
			echo_delay = 0x0001,
			echo_lr_delay = 0x0002,
			echo_damping = 0x0003,
			echo_feedback = 0x0004,
			echo_spread = 0x0005,
			flanger_waveform = 0x0001,
			flanger_phase = 0x0002,
			flanger_rate = 0x0003,
			flanger_depth = 0x0004,
			flanger_feedback = 0x0005,
			flanger_delay = 0x0006,
			frequency_shifter_frequency = 0x0001,
			frequency_shifter_left_direction = 0x0002,
			frequency_shifter_right_direction = 0x0003,
			vocmorpher_phoneme_a = 0x0001,
			vocmorpher_phoneme_acoarse_tuning = 0x0002,
			vocmorpher_phoneme_b = 0x0003,
			vocmorpher_phoneme_bcoarse_tuning = 0x0004,
			vocmorpher_waveform = 0x0005,
			vocmorpher_rate = 0x0006,
			pitch_shifter_coarse_tune = 0x0001,
			pitch_shifter_fine_tune = 0x0002,
			ring_modulator_frequency = 0x0001,
			ring_modulator_highpass_cutoff = 0x0002,
			ring_modulator_waveform = 0x0003,
			autowah_attack_time = 0x0001,
			autowah_release_time = 0x0002,
			autowah_resonance = 0x0003,
			autowah_peak_gain = 0x0004,
			compressor_onoff = 0x0001,
			equalizer_low_gain = 0x0001,
			equalizer_low_cutoff = 0x0002,
			equalizer_mid1_gain = 0x0003,
			equalizer_mid1_center = 0x0004,
			equalizer_mid1_width = 0x0005,
			equalizer_mid2_gain = 0x0006,
			equalizer_mid2_center = 0x0007,
			equalizer_mid2_width = 0x0008,
			equalizer_high_gain = 0x0009,
			equalizer_high_cutoff = 0x000A,
			effect_first_parameter = 0x0000,
			effect_last_parameter = 0x8000,
			effect_type = 0x8001,
			effect_null = 0x0000,
			effect_reverb = 0x0001,
			effect_chorus = 0x0002,
			effect_distortion = 0x0003,
			effect_echo = 0x0004,
			effect_flanger = 0x0005,
			effect_frequency_shifter = 0x0006,
			effect_vocmorpher = 0x0007,
			effect_pitch_shifter = 0x0008,
			effect_ring_modulator = 0x0009,
			effect_autowah = 0x000A,
			effect_compressor = 0x000B,
			effect_equalizer = 0x000C,
			effect_eax_reverb = 0x8000,
			effect_slot_effect = 0x0001,
			effect_slot_gain = 0x0002,
			effect_slot_auxiliary_send_auto = 0x0003,
			effect_slot_null = 0x0000
		};

		struct audio_sync
		{
			trigonometry::vector3 direction;
			trigonometry::vector3 velocity;
			float cone_inner_angle = 360.0f;
			float cone_outer_angle = 360.0f;
			float cone_outer_gain = 0.0f;
			float pitch = 1.0f;
			float gain = 1.0f;
			float ref_distance = 0.0f;
			float distance = 10.0f;
			float rolloff = 1.0f;
			float position = 0.0f;
			float air_absorption = 0.0f;
			float room_roll_off = 0.0f;
			bool is_relative = false;
			bool is_looped = false;
		};

		class audio_exception final : public core::basic_exception
		{
		private:
			int al_error_code;
			int alc_error_code;

		public:
			audio_exception(void* device = nullptr);
			const char* type() const noexcept override;
			int al_code() const noexcept;
			int alc_code() const noexcept;
			bool has_error() const noexcept;
		};

		template <typename v>
		using expects_audio = core::expects<v, audio_exception>;

		class audio_context final : public core::singletonish
		{
		public:
			static expects_audio<void> initialize();
			static expects_audio<void> generate_buffers(int count, uint32_t* buffers);
			static expects_audio<void> set_filter1i(uint32_t filter, filter_ex value, int F1);
			static expects_audio<void> set_filter1f(uint32_t filter, filter_ex value, float F1);
			static expects_audio<void> set_effect1i(uint32_t effect, effect_ex value, int F1);
			static expects_audio<void> set_effect1f(uint32_t effect, effect_ex value, float F1);
			static expects_audio<void> set_effectvf(uint32_t effect, effect_ex value, float* FS);
			static expects_audio<void> set_buffer_data(uint32_t buffer, int format, const void* data, int size, int frequency);
			static expects_audio<void> set_source_data3f(uint32_t source, sound_ex value, float F1, float F2, float F3);
			static expects_audio<void> get_source_data3f(uint32_t source, sound_ex value, float* F1, float* F2, float* F3);
			static expects_audio<void> set_source_datavf(uint32_t source, sound_ex value, float* FS);
			static expects_audio<void> get_source_datavf(uint32_t source, sound_ex value, float* FS);
			static expects_audio<void> set_source_data1f(uint32_t source, sound_ex value, float F1);
			static expects_audio<void> get_source_data1f(uint32_t source, sound_ex value, float* F1);
			static expects_audio<void> set_source_data3i(uint32_t source, sound_ex value, int F1, int F2, int F3);
			static expects_audio<void> get_source_data3i(uint32_t source, sound_ex value, int* F1, int* F2, int* F3);
			static expects_audio<void> set_source_datavi(uint32_t source, sound_ex value, int* FS);
			static expects_audio<void> get_source_datavi(uint32_t source, sound_ex value, int* FS);
			static expects_audio<void> set_source_data1i(uint32_t source, sound_ex value, int F1);
			static expects_audio<void> get_source_data1i(uint32_t source, sound_ex value, int* F1);
			static expects_audio<void> set_listener_data3f(sound_ex listener, float F1, float F2, float F3);
			static expects_audio<void> get_listener_data3f(sound_ex listener, float* F1, float* F2, float* F3);
			static expects_audio<void> set_listener_datavf(sound_ex listener, float* FS);
			static expects_audio<void> get_listener_datavf(sound_ex listener, float* FS);
			static expects_audio<void> set_listener_data1f(sound_ex listener, float F1);
			static expects_audio<void> get_listener_data1f(sound_ex listener, float* F1);
			static expects_audio<void> set_listener_data3i(sound_ex listener, int F1, int F2, int F3);
			static expects_audio<void> get_listener_data3i(sound_ex listener, int* F1, int* F2, int* F3);
			static expects_audio<void> set_listener_datavi(sound_ex listener, int* FS);
			static expects_audio<void> get_listener_datavi(sound_ex listener, int* FS);
			static expects_audio<void> set_listener_data1i(sound_ex listener, int F1);
			static expects_audio<void> get_listener_data1i(sound_ex listener, int* F1);
			static uint32_t get_enum_value(const char* name);
		};

		class audio_filter : public core::reference<audio_filter>
		{
			friend audio_effect;
			friend audio_source;

		protected:
			audio_source* source = nullptr;
			uint32_t filter = 0;

		public:
			audio_filter() noexcept;
			virtual ~audio_filter() noexcept;
			virtual expects_audio<void> synchronize() = 0;
			virtual void deserialize(core::schema* node) = 0;
			virtual void serialize(core::schema* node) const = 0;
			virtual core::unique<audio_filter> copy() const = 0;
			audio_source* get_source() const;

		protected:
			expects_audio<void> initialize(const std::function<bool()>& callback);

		public:
			VI_COMPONENT_ROOT("base_audio_filter");
		};

		class audio_effect : public core::reference<audio_effect>
		{
			friend audio_source;

		private:
			int zone = -1;

		protected:
			audio_source* source = nullptr;
			audio_filter* filter = nullptr;
			uint32_t effect = 0;
			uint32_t slot = 0;

		public:
			audio_effect() noexcept;
			virtual ~audio_effect() noexcept;
			virtual expects_audio<void> synchronize() = 0;
			virtual void deserialize(core::schema* node) = 0;
			virtual void serialize(core::schema* node) const = 0;
			virtual core::unique<audio_effect> copy() const = 0;
			expects_audio<void> set_filter(audio_filter** filter);
			audio_filter* get_filter() const;
			audio_source* get_source() const;

		protected:
			expects_audio<void> initialize(const std::function<bool()>& callback);

		private:
			expects_audio<void> bind(audio_source* new_source, int new_zone);
			expects_audio<void> unbind();

		public:
			VI_COMPONENT_ROOT("base_audio_effect");
		};

		class audio_clip final : public core::reference<audio_clip>
		{
		private:
			uint32_t buffer = 0;
			int format = 0;

		public:
			audio_clip(int buffer_count, int new_format) noexcept;
			~audio_clip() noexcept;
			float length() const;
			bool is_mono() const;
			uint32_t get_buffer() const;
			int get_format() const;
		};

		class audio_source final : public core::reference<audio_source>
		{
			friend class audio_device;

		private:
			core::vector<audio_effect*> effects;
			audio_clip* clip = nullptr;
			uint32_t instance = 0;

		public:
			audio_source() noexcept;
			~audio_source() noexcept;
			int64_t add_effect(audio_effect* effect);
			expects_audio<void> remove_effect(size_t effect_id);
			expects_audio<void> remove_effect_by_id(uint64_t effect_id);
			expects_audio<void> remove_effects();
			expects_audio<void> set_clip(audio_clip* clip);
			expects_audio<void> synchronize(audio_sync* sync, const trigonometry::vector3& position);
			expects_audio<void> reset();
			expects_audio<void> pause();
			expects_audio<void> play();
			expects_audio<void> stop();
			bool is_playing() const;
			size_t get_effects_count() const;
			audio_clip* get_clip() const;
			audio_effect* get_effect(uint64_t section) const;
			uint32_t get_instance() const;
			const core::vector<audio_effect*>& get_effects() const;

		public:
			template <typename t>
			t* get_effect()
			{
				return (t*)get_effect(t::get_type_id());
			}
		};

		class audio_device final : public core::reference<audio_device>
		{
		public:
			void* context = nullptr;
			void* device = nullptr;

		public:
			audio_device() noexcept;
			~audio_device() noexcept;
			expects_audio<void> offset(audio_source* source, float& seconds, bool get);
			expects_audio<void> velocity(audio_source* source, trigonometry::vector3& velocity, bool get);
			expects_audio<void> position(audio_source* source, trigonometry::vector3& position, bool get);
			expects_audio<void> direction(audio_source* source, trigonometry::vector3& direction, bool get);
			expects_audio<void> relative(audio_source* source, int& value, bool get);
			expects_audio<void> pitch(audio_source* source, float& value, bool get);
			expects_audio<void> gain(audio_source* source, float& value, bool get);
			expects_audio<void> loop(audio_source* source, int& is_loop, bool get);
			expects_audio<void> cone_inner_angle(audio_source* source, float& value, bool get);
			expects_audio<void> cone_outer_angle(audio_source* source, float& value, bool get);
			expects_audio<void> cone_outer_gain(audio_source* source, float& value, bool get);
			expects_audio<void> distance(audio_source* source, float& value, bool get);
			expects_audio<void> ref_distance(audio_source* source, float& value, bool get);
			expects_audio<void> set_distance_model(sound_distance_model model);
			void display_audio_log() const;
			bool is_valid() const;
		};
	}
}
#endif
