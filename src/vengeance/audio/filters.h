#ifndef VI_AUDIO_FILTERS_H
#define VI_AUDIO_FILTERS_H
#include "../audio.h"

namespace Vitex
{
	namespace Audio
	{
		namespace Filters
		{
			class VI_OUT Lowpass final : public AudioFilter
			{
			public:
				float GainHF = 1.0f;
				float Gain = 1.0f;

			public:
				Lowpass();
				~Lowpass() override;
				ExpectsAudio<void> Synchronize() override;
				void Deserialize(Core::Schema* Node) override;
				void Serialize(Core::Schema* Node) const override;
				Core::Unique<AudioFilter> Copy() const override;

			public:
				VI_COMPONENT("lowpass_filter");
			};

			class VI_OUT Highpass final : public AudioFilter
			{
			public:
				float GainLF = 1.0f;
				float Gain = 1.0f;

			public:
				Highpass();
				~Highpass() override;
				ExpectsAudio<void> Synchronize() override;
				void Deserialize(Core::Schema* Node) override;
				void Serialize(Core::Schema* Node) const override;
				Core::Unique<AudioFilter> Copy() const override;

			public:
				VI_COMPONENT("highpass_filter");
			};

			class VI_OUT Bandpass final : public AudioFilter
			{
			public:
				float GainHF = 1.0f;
				float GainLF = 1.0f;
				float Gain = 1.0f;

			public:
				Bandpass();
				~Bandpass() override;
				ExpectsAudio<void> Synchronize() override;
				void Deserialize(Core::Schema* Node) override;
				void Serialize(Core::Schema* Node) const override;
				Core::Unique<AudioFilter> Copy() const override;

			public:
				VI_COMPONENT("bandpass_filter");
			};
		}
	}
}
#endif