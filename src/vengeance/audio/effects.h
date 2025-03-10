#ifndef VI_AUDIO_EFFECTS_H
#define VI_AUDIO_EFFECTS_H
#include "../audio.h"

namespace vitex
{
	namespace audio
	{
		namespace effects
		{
			class reverb final : public audio_effect
			{
			private:
				bool EAX = false;

			public:
				trigonometry::vector3 late_reverb_pan;
				trigonometry::vector3 reflections_pan;
				float density = 1.0f;
				float diffusion = 1.0f;
				float gain = 0.32f;
				float gain_hf = 0.89f;
				float gain_lf = 1.0f;
				float decay_time = 1.49f;
				float decay_hf_ratio = 0.83f;
				float decay_lf_ratio = 1.0f;
				float reflections_gain = 0.05f;
				float reflections_delay = 0.007f;
				float late_reverb_gain = 1.26f;
				float late_reverb_delay = 0.011f;
				float echo_time = 0.25f;
				float echo_depth = 0.0f;
				float modulation_time = 0.25f;
				float modulation_depth = 0.0f;
				float air_absorption_gain_hf = 0.994f;
				float hf_reference = 5000.0f;
				float lf_reference = 250.0f;
				float room_rolloff_factor = 0.0f;
				bool is_decay_hf_limited = true;

			public:
				reverb();
				~reverb() override;
				expects_audio<void> synchronize() override;
				void deserialize(core::schema* node) override;
				void serialize(core::schema* node) const override;
				core::unique<audio_effect> copy() const override;

			public:
				VI_COMPONENT("reverb_effect");
			};

			class chorus final : public audio_effect
			{
			public:
				float rate = 1.1f;
				float depth = 0.1f;
				float feedback = 0.25f;
				float delay = 0.016f;
				int waveform = 1;
				int phase = 90;

			public:
				chorus();
				~chorus() override;
				expects_audio<void> synchronize() override;
				void deserialize(core::schema* node) override;
				void serialize(core::schema* node) const override;
				core::unique<audio_effect> copy() const override;

			public:
				VI_COMPONENT("chorus_effect");
			};

			class distortion final : public audio_effect
			{
			public:
				float edge = 0.2f;
				float gain = 0.05f;
				float lowpass_cut_off = 8000.0f;
				float eq_center = 3600.0f;
				float eq_bandwidth = 3600.0f;

			public:
				distortion();
				~distortion() override;
				expects_audio<void> synchronize() override;
				void deserialize(core::schema* node) override;
				void serialize(core::schema* node) const override;
				core::unique<audio_effect> copy() const override;

			public:
				VI_COMPONENT("distortion_effect");
			};

			class echo final : public audio_effect
			{
			public:
				float delay = 0.1f;
				float lr_delay = 0.1f;
				float damping = 0.5f;
				float feedback = 0.5f;
				float spread = -1.0f;

			public:
				echo();
				~echo() override;
				expects_audio<void> synchronize() override;
				void deserialize(core::schema* node) override;
				void serialize(core::schema* node) const override;
				core::unique<audio_effect> copy() const override;

			public:
				VI_COMPONENT("echo_effect");
			};

			class flanger final : public audio_effect
			{
			public:
				float rate = 0.27f;
				float depth = 1.0f;
				float feedback = -0.5f;
				float delay = 0.002f;
				int waveform = 1;
				int phase = 0;

			public:
				flanger();
				~flanger() override;
				expects_audio<void> synchronize() override;
				void deserialize(core::schema* node) override;
				void serialize(core::schema* node) const override;
				core::unique<audio_effect> copy() const override;

			public:
				VI_COMPONENT("flanger_effect");
			};

			class frequency_shifter final : public audio_effect
			{
			public:
				float frequency = 0.0f;
				int left_direction = 0;
				int right_direction = 0;

			public:
				frequency_shifter();
				~frequency_shifter() override;
				expects_audio<void> synchronize() override;
				void deserialize(core::schema* node) override;
				void serialize(core::schema* node) const override;
				core::unique<audio_effect> copy() const override;

			public:
				VI_COMPONENT("frequency_shifter_effect");
			};

			class vocal_morpher final : public audio_effect
			{
			public:
				float rate = 1.41f;
				int phonemea = 0;
				int phonemea_coarse_tuning = 0;
				int phonemeb = 10;
				int phonemeb_coarse_tuning = 0;
				int waveform = 0;

			public:
				vocal_morpher();
				~vocal_morpher() override;
				expects_audio<void> synchronize() override;
				void deserialize(core::schema* node) override;
				void serialize(core::schema* node) const override;
				core::unique<audio_effect> copy() const override;

			public:
				VI_COMPONENT("vocal_morpher_effect");
			};

			class pitch_shifter final : public audio_effect
			{
			public:
				int coarse_tune = 12;
				int fine_tune = 0;

			public:
				pitch_shifter();
				~pitch_shifter() override;
				expects_audio<void> synchronize() override;
				void deserialize(core::schema* node) override;
				void serialize(core::schema* node) const override;
				core::unique<audio_effect> copy() const override;

			public:
				VI_COMPONENT("pitch_shifter_effect");
			};

			class ring_modulator final : public audio_effect
			{
			public:
				float frequency = 440.0f;
				float highpass_cut_off = 800.0f;
				int waveform = 0;

			public:
				ring_modulator();
				~ring_modulator() override;
				expects_audio<void> synchronize() override;
				void deserialize(core::schema* node) override;
				void serialize(core::schema* node) const override;
				core::unique<audio_effect> copy() const override;

			public:
				VI_COMPONENT("ring_modulator_effect");
			};

			class autowah final : public audio_effect
			{
			public:
				float attack_time = 0.06f;
				float release_time = 0.06f;
				float resonance = 1000.0f;
				float peak_gain = 11.22f;

			public:
				autowah();
				~autowah() override;
				expects_audio<void> synchronize() override;
				void deserialize(core::schema* node) override;
				void serialize(core::schema* node) const override;
				core::unique<audio_effect> copy() const override;

			public:
				VI_COMPONENT("autowah_effect");
			};

			class compressor final : public audio_effect
			{
			public:
				compressor();
				~compressor() override;
				expects_audio<void> synchronize() override;
				void deserialize(core::schema* node) override;
				void serialize(core::schema* node) const override;
				core::unique<audio_effect> copy() const override;

			public:
				VI_COMPONENT("compressor_effect");
			};

			class equalizer final : public audio_effect
			{
			public:
				float low_gain = 1.0f;
				float low_cut_off = 200.0f;
				float mid1_gain = 1.0f;
				float mid1_center = 500.0f;
				float mid1_width = 1.0f;
				float mid2_gain = 1.0f;
				float mid2_center = 3000.0f;
				float mid2_width = 1.0f;
				float high_gain = 1.0f;
				float high_cut_off = 6000.0f;

			public:
				equalizer();
				~equalizer() override;
				expects_audio<void> synchronize() override;
				void deserialize(core::schema* node) override;
				void serialize(core::schema* node) const override;
				core::unique<audio_effect> copy() const override;

			public:
				VI_COMPONENT("equalizer_effect");
			};
		}
	}
}
#endif