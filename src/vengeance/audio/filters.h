#ifndef VI_AUDIO_FILTERS_H
#define VI_AUDIO_FILTERS_H
#include "../audio.h"

namespace vitex
{
	namespace audio
	{
		namespace filters
		{
			class lowpass final : public audio_filter
			{
			public:
				float gain_hf = 1.0f;
				float gain = 1.0f;

			public:
				lowpass();
				~lowpass() override;
				expects_audio<void> synchronize() override;
				void deserialize(core::schema* node) override;
				void serialize(core::schema* node) const override;
				audio_filter* copy() const override;

			public:
				VI_COMPONENT("lowpass_filter");
			};

			class highpass final : public audio_filter
			{
			public:
				float gain_lf = 1.0f;
				float gain = 1.0f;

			public:
				highpass();
				~highpass() override;
				expects_audio<void> synchronize() override;
				void deserialize(core::schema* node) override;
				void serialize(core::schema* node) const override;
				audio_filter* copy() const override;

			public:
				VI_COMPONENT("highpass_filter");
			};

			class bandpass final : public audio_filter
			{
			public:
				float gain_hf = 1.0f;
				float gain_lf = 1.0f;
				float gain = 1.0f;

			public:
				bandpass();
				~bandpass() override;
				expects_audio<void> synchronize() override;
				void deserialize(core::schema* node) override;
				void serialize(core::schema* node) const override;
				audio_filter* copy() const override;

			public:
				VI_COMPONENT("bandpass_filter");
			};
		}
	}
}
#endif