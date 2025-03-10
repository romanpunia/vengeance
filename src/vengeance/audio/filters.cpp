#include "filters.h"
#include "../layer.h"
#define return_error_if do { auto __error = audio_exception(); if (__error.has_error()) return __error; else return core::expectation::met; } while (0)

namespace vitex
{
	namespace audio
	{
		namespace filters
		{
			lowpass::lowpass()
			{
				initialize([this]()
				{
					audio_context::set_filter1i(filter, filter_ex::filter_type, (int)filter_ex::filter_lowpass);
					return true;
				});
			}
			lowpass::~lowpass()
			{
			}
			expects_audio<void> lowpass::synchronize()
			{
				audio_context::set_filter1f(filter, filter_ex::lowpass_gain, gain);
				audio_context::set_filter1f(filter, filter_ex::lowpass_gain_hf, gain_hf);
				return_error_if;
			}
			void lowpass::deserialize(core::schema* node)
			{
				VI_ASSERT(node != nullptr, "schema should be set");
				layer::series::unpack(node->find("gain"), &gain);
				layer::series::unpack(node->find("gain-hf"), &gain_hf);
			}
			void lowpass::serialize(core::schema* node) const
			{
				VI_ASSERT(node != nullptr, "schema should be set");
				layer::series::pack(node->set("gain"), gain);
				layer::series::pack(node->set("gain-hf"), gain_hf);
			}
			audio_filter* lowpass::copy() const
			{
				lowpass* target = new lowpass();
				target->gain = gain;
				target->gain_hf = gain_hf;

				return target;
			}

			highpass::highpass()
			{
				initialize([this]()
				{
					audio_context::set_filter1i(filter, filter_ex::filter_type, (int)filter_ex::filter_highpass);
					return true;
				});
			}
			highpass::~highpass()
			{
			}
			expects_audio<void> highpass::synchronize()
			{
				audio_context::set_filter1f(filter, filter_ex::highpass_gain, gain);
				audio_context::set_filter1f(filter, filter_ex::highpass_gain_lf, gain_lf);
				return_error_if;
			}
			void highpass::deserialize(core::schema* node)
			{
				VI_ASSERT(node != nullptr, "schema should be set");
				layer::series::unpack(node->find("gain"), &gain);
				layer::series::unpack(node->find("gain-lf"), &gain_lf);
			}
			void highpass::serialize(core::schema* node) const
			{
				VI_ASSERT(node != nullptr, "schema should be set");
				layer::series::pack(node->set("gain"), gain);
				layer::series::pack(node->set("gain-lf"), gain_lf);
			}
			audio_filter* highpass::copy() const
			{
				highpass* target = new highpass();
				target->gain = gain;
				target->gain_lf = gain_lf;

				return target;
			}

			bandpass::bandpass()
			{
				initialize([this]()
				{
					audio_context::set_filter1i(filter, filter_ex::filter_type, (int)filter_ex::filter_bandpass);
					return true;
				});
			}
			bandpass::~bandpass()
			{
			}
			expects_audio<void> bandpass::synchronize()
			{
				audio_context::set_filter1f(filter, filter_ex::bandpass_gain, gain);
				audio_context::set_filter1f(filter, filter_ex::bandpass_gain_lf, gain_lf);
				audio_context::set_filter1f(filter, filter_ex::bandpass_gain_hf, gain_hf);
				return_error_if;
			}
			void bandpass::deserialize(core::schema* node)
			{
				VI_ASSERT(node != nullptr, "schema should be set");
				layer::series::unpack(node->find("gain"), &gain);
				layer::series::unpack(node->find("gain-lf"), &gain_lf);
				layer::series::unpack(node->find("gain-hf"), &gain_hf);
			}
			void bandpass::serialize(core::schema* node) const
			{
				VI_ASSERT(node != nullptr, "schema should be set");
				layer::series::pack(node->set("gain"), gain);
				layer::series::pack(node->set("gain-lf"), gain_lf);
				layer::series::pack(node->set("gain-hf"), gain_hf);
			}
			audio_filter* bandpass::copy() const
			{
				bandpass* target = new bandpass();
				target->gain = gain;
				target->gain_lf = gain_lf;
				target->gain_hf = gain_hf;

				return target;
			}
		}
	}
}
