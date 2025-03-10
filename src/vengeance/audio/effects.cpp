#include "effects.h"
#include "filters.h"
#include "../layer.h"
#define return_error_if do { auto __error = audio_exception(); if (__error.has_error()) return __error; else return core::expectation::met; } while (0)

namespace vitex
{
	namespace audio
	{
		namespace effects
		{
			audio_filter* get_filter_deserialized(core::schema* node)
			{
				core::schema* filter = node->find("filter");
				if (!filter)
					return nullptr;

				uint64_t id;
				if (!layer::series::unpack(filter->find("id"), &id))
					return nullptr;

				audio_filter* target = core::composer::create<audio_filter>(id);
				if (!target)
					return nullptr;

				core::schema* meta = filter->find("metadata");
				if (!meta)
					meta = filter->set("metadata");

				target->deserialize(meta);
				return target;
			}

			reverb::reverb()
			{
				initialize([this]()
				{
					EAX = (audio_context::get_enum_value("AL_EFFECT_EAXREVERB") != 0);
					if (EAX)
						audio_context::set_effect1i(effect, effect_ex::effect_type, (int)effect_ex::effect_eax_reverb);
					else
						audio_context::set_effect1i(effect, effect_ex::effect_type, (int)effect_ex::effect_reverb);
					return true;
				});
			}
			reverb::~reverb()
			{
			}
			expects_audio<void> reverb::synchronize()
			{
				if (EAX)
				{
					float reflections_pan3[3];
					reflections_pan.get3(reflections_pan3);

					float late_reverb_pan3[3];
					late_reverb_pan.get3(late_reverb_pan3);

					audio_context::set_effectvf(effect, effect_ex::eax_reverb_reflections_pan, reflections_pan3);
					audio_context::set_effectvf(effect, effect_ex::eax_reverb_late_reverb_pan, late_reverb_pan3);
					audio_context::set_effect1f(effect, effect_ex::eax_reverb_density, density);
					audio_context::set_effect1f(effect, effect_ex::eax_reverb_diffusion, diffusion);
					audio_context::set_effect1f(effect, effect_ex::eax_reverb_gain, gain);
					audio_context::set_effect1f(effect, effect_ex::eax_reverb_gain_hf, gain_hf);
					audio_context::set_effect1f(effect, effect_ex::eax_reverb_gain_lf, gain_lf);
					audio_context::set_effect1f(effect, effect_ex::eax_reverb_decay_time, decay_time);
					audio_context::set_effect1f(effect, effect_ex::eax_reverb_decay_hf_ratio, decay_hf_ratio);
					audio_context::set_effect1f(effect, effect_ex::eax_reverb_decay_lf_ratio, decay_lf_ratio);
					audio_context::set_effect1f(effect, effect_ex::eax_reverb_reflections_gain, reflections_gain);
					audio_context::set_effect1f(effect, effect_ex::eax_reverb_reflections_delay, reflections_delay);
					audio_context::set_effect1f(effect, effect_ex::eax_reverb_late_reverb_gain, late_reverb_gain);
					audio_context::set_effect1f(effect, effect_ex::eax_reverb_late_reverb_delay, late_reverb_delay);
					audio_context::set_effect1f(effect, effect_ex::eax_reverb_echo_time, echo_time);
					audio_context::set_effect1f(effect, effect_ex::eax_reverb_echo_depth, echo_depth);
					audio_context::set_effect1f(effect, effect_ex::eax_reverb_modulation_time, modulation_time);
					audio_context::set_effect1f(effect, effect_ex::eax_reverb_modulation_depth, modulation_depth);
					audio_context::set_effect1f(effect, effect_ex::eax_reverb_air_absorption_gain_hf, air_absorption_gain_hf);
					audio_context::set_effect1f(effect, effect_ex::eax_reverb_hf_reference, hf_reference);
					audio_context::set_effect1f(effect, effect_ex::eax_reverb_lf_reference, lf_reference);
					audio_context::set_effect1f(effect, effect_ex::eax_reverb_room_rolloff_factor, room_rolloff_factor);
					audio_context::set_effect1i(effect, effect_ex::eax_reverb_decay_hf_limit, is_decay_hf_limited ? 1 : 0);
				}
				else
				{
					audio_context::set_effect1f(effect, effect_ex::reverb_density, density);
					audio_context::set_effect1f(effect, effect_ex::reverb_diffusion, diffusion);
					audio_context::set_effect1f(effect, effect_ex::reverb_gain, gain);
					audio_context::set_effect1f(effect, effect_ex::reverb_gain_hf, gain_hf);
					audio_context::set_effect1f(effect, effect_ex::reverb_decay_time, decay_time);
					audio_context::set_effect1f(effect, effect_ex::reverb_decay_hf_ratio, decay_hf_ratio);
					audio_context::set_effect1f(effect, effect_ex::reverb_reflections_gain, reflections_gain);
					audio_context::set_effect1f(effect, effect_ex::reverb_reflections_delay, reflections_delay);
					audio_context::set_effect1f(effect, effect_ex::reverb_late_reverb_gain, late_reverb_gain);
					audio_context::set_effect1f(effect, effect_ex::reverb_late_reverb_delay, late_reverb_delay);
					audio_context::set_effect1f(effect, effect_ex::reverb_air_absorption_gain_hf, air_absorption_gain_hf);
					audio_context::set_effect1f(effect, effect_ex::reverb_room_rolloff_factor, room_rolloff_factor);
					audio_context::set_effect1i(effect, effect_ex::reverb_decay_hf_limit, is_decay_hf_limited ? 1 : 0);
				}
				return_error_if;
			}
			void reverb::deserialize(core::schema* node)
			{
				VI_ASSERT(node != nullptr, "schema should be set");
				audio_filter* new_filter = get_filter_deserialized(node);
				if (new_filter != nullptr)
					set_filter(&filter);

				layer::heavy_series::unpack(node->find("late-reverb-pan"), &late_reverb_pan);
				layer::heavy_series::unpack(node->find("reflections-pan"), &reflections_pan);
				layer::series::unpack(node->find("density"), &density);
				layer::series::unpack(node->find("diffusion"), &diffusion);
				layer::series::unpack(node->find("gain"), &gain);
				layer::series::unpack(node->find("gain-hf"), &gain_hf);
				layer::series::unpack(node->find("gain-lf"), &gain_lf);
				layer::series::unpack(node->find("decay-time"), &decay_time);
				layer::series::unpack(node->find("decay-hg-ratio"), &decay_hf_ratio);
				layer::series::unpack(node->find("decay-lf-ratio"), &decay_lf_ratio);
				layer::series::unpack(node->find("reflections-gain"), &reflections_gain);
				layer::series::unpack(node->find("reflections-delay"), &reflections_delay);
				layer::series::unpack(node->find("late-reverb-gain"), &late_reverb_gain);
				layer::series::unpack(node->find("late-reverb-delay"), &late_reverb_delay);
				layer::series::unpack(node->find("echo-time"), &echo_time);
				layer::series::unpack(node->find("echo-depth"), &echo_depth);
				layer::series::unpack(node->find("modulation-time"), &modulation_time);
				layer::series::unpack(node->find("modulation-depth"), &modulation_depth);
				layer::series::unpack(node->find("air-absorption-gain-hf"), &air_absorption_gain_hf);
				layer::series::unpack(node->find("hf-reference"), &hf_reference);
				layer::series::unpack(node->find("lf-reference"), &lf_reference);
				layer::series::unpack(node->find("room-rolloff-factor"), &room_rolloff_factor);
				layer::series::unpack(node->find("decay-hf-limited"), &is_decay_hf_limited);
			}
			void reverb::serialize(core::schema* node) const
			{
				VI_ASSERT(node != nullptr, "schema should be set");
				if (filter != nullptr)
					filter->serialize(node->set("filter"));

				layer::heavy_series::pack(node->set("late-reverb-pan"), late_reverb_pan);
				layer::heavy_series::pack(node->set("reflections-pan"), reflections_pan);
				layer::series::pack(node->set("density"), density);
				layer::series::pack(node->set("diffusion"), diffusion);
				layer::series::pack(node->set("gain"), gain);
				layer::series::pack(node->set("gain-hf"), gain_hf);
				layer::series::pack(node->set("gain-lf"), gain_lf);
				layer::series::pack(node->set("decay-time"), decay_time);
				layer::series::pack(node->set("decay-hg-ratio"), decay_hf_ratio);
				layer::series::pack(node->set("decay-lf-ratio"), decay_lf_ratio);
				layer::series::pack(node->set("reflections-gain"), reflections_gain);
				layer::series::pack(node->set("reflections-delay"), reflections_delay);
				layer::series::pack(node->set("late-reverb-gain"), late_reverb_gain);
				layer::series::pack(node->set("late-reverb-delay"), late_reverb_delay);
				layer::series::pack(node->set("echo-time"), echo_time);
				layer::series::pack(node->set("echo-depth"), echo_depth);
				layer::series::pack(node->set("modulation-time"), modulation_time);
				layer::series::pack(node->set("modulation-depth"), modulation_depth);
				layer::series::pack(node->set("air-absorption-gain-hf"), air_absorption_gain_hf);
				layer::series::pack(node->set("hf-reference"), hf_reference);
				layer::series::pack(node->set("lf-reference"), lf_reference);
				layer::series::pack(node->set("room-rolloff-factor"), room_rolloff_factor);
				layer::series::pack(node->set("decay-hf-limited"), is_decay_hf_limited);
			}
			audio_effect* reverb::copy() const
			{
				reverb* target = new reverb();
				target->late_reverb_pan = late_reverb_pan;
				target->reflections_pan = reflections_pan;
				target->density = density;
				target->diffusion = diffusion;
				target->gain = gain;
				target->gain_hf = gain_hf;
				target->gain_lf = gain_lf;
				target->decay_time = decay_time;
				target->decay_hf_ratio = decay_hf_ratio;
				target->decay_lf_ratio = decay_lf_ratio;
				target->reflections_gain = reflections_gain;
				target->reflections_delay = reflections_delay;
				target->late_reverb_gain = late_reverb_gain;
				target->late_reverb_delay = late_reverb_delay;
				target->echo_time = echo_time;
				target->echo_depth = echo_depth;
				target->modulation_time = modulation_time;
				target->modulation_depth = modulation_depth;
				target->air_absorption_gain_hf = air_absorption_gain_hf;
				target->hf_reference = hf_reference;
				target->lf_reference = lf_reference;
				target->room_rolloff_factor = room_rolloff_factor;
				target->is_decay_hf_limited = is_decay_hf_limited;

				return target;
			}

			chorus::chorus()
			{
				initialize([this]()
				{
					audio_context::set_effect1i(effect, effect_ex::effect_type, (int)effect_ex::effect_chorus);
					return true;
				});
			}
			chorus::~chorus()
			{

			}
			expects_audio<void> chorus::synchronize()
			{
				audio_context::set_effect1f(effect, effect_ex::chorus_rate, rate);
				audio_context::set_effect1f(effect, effect_ex::chorus_depth, depth);
				audio_context::set_effect1f(effect, effect_ex::chorus_feedback, feedback);
				audio_context::set_effect1f(effect, effect_ex::chorus_delay, delay);
				audio_context::set_effect1i(effect, effect_ex::chorus_waveform, waveform);
				audio_context::set_effect1i(effect, effect_ex::chorus_phase, phase);
				return_error_if;
			}
			void chorus::deserialize(core::schema* node)
			{
				VI_ASSERT(node != nullptr, "schema should be set");
				audio_filter* new_filter = get_filter_deserialized(node);
				if (new_filter != nullptr)
					set_filter(&filter);

				layer::series::unpack(node->find("rate"), &rate);
				layer::series::unpack(node->find("depth"), &depth);
				layer::series::unpack(node->find("feedback"), &feedback);
				layer::series::unpack(node->find("delay"), &delay);
				layer::series::unpack(node->find("waveform"), &waveform);
				layer::series::unpack(node->find("phase"), &phase);
			}
			void chorus::serialize(core::schema* node) const
			{
				VI_ASSERT(node != nullptr, "schema should be set");
				if (filter != nullptr)
					filter->serialize(node->set("filter"));

				layer::series::pack(node->set("rate"), rate);
				layer::series::pack(node->set("depth"), depth);
				layer::series::pack(node->set("feedback"), feedback);
				layer::series::pack(node->set("delay"), delay);
				layer::series::pack(node->set("waveform"), waveform);
				layer::series::pack(node->set("phase"), phase);
			}
			audio_effect* chorus::copy() const
			{
				chorus* target = new chorus();
				target->rate = 1.1f;
				target->depth = 0.1f;
				target->feedback = 0.25f;
				target->delay = 0.016f;
				target->waveform = 1;
				target->phase = 90;

				return target;
			}

			distortion::distortion()
			{
				initialize([this]()
				{
					audio_context::set_effect1i(effect, effect_ex::effect_type, (int)effect_ex::effect_distortion);
					return true;
				});
			}
			distortion::~distortion()
			{

			}
			expects_audio<void> distortion::synchronize()
			{
				audio_context::set_effect1f(effect, effect_ex::distortion_edge, edge);
				audio_context::set_effect1f(effect, effect_ex::distortion_gain, gain);
				audio_context::set_effect1f(effect, effect_ex::distortion_lowpass_cutoff, lowpass_cut_off);
				audio_context::set_effect1f(effect, effect_ex::distortion_eq_center, eq_center);
				audio_context::set_effect1f(effect, effect_ex::distortion_eq_bandwidth, eq_bandwidth);
				return_error_if;
			}
			void distortion::deserialize(core::schema* node)
			{
				VI_ASSERT(node != nullptr, "schema should be set");
				audio_filter* new_filter = get_filter_deserialized(node);
				if (new_filter != nullptr)
					set_filter(&filter);

				layer::series::unpack(node->find("edge"), &edge);
				layer::series::unpack(node->find("gain"), &gain);
				layer::series::unpack(node->find("lowpass-cut-off"), &lowpass_cut_off);
				layer::series::unpack(node->find("eq-center"), &eq_center);
				layer::series::unpack(node->find("eq-bandwidth"), &eq_bandwidth);
			}
			void distortion::serialize(core::schema* node) const
			{
				VI_ASSERT(node != nullptr, "schema should be set");
				if (filter != nullptr)
					filter->serialize(node->set("filter"));

				layer::series::pack(node->set("edge"), edge);
				layer::series::pack(node->set("gain"), gain);
				layer::series::pack(node->set("lowpass-cut-off"), lowpass_cut_off);
				layer::series::pack(node->set("eq-center"), eq_center);
				layer::series::pack(node->set("eq-bandwidth"), eq_bandwidth);
			}
			audio_effect* distortion::copy() const
			{
				distortion* target = new distortion();
				target->edge = 0.2f;
				target->gain = 0.05f;
				target->lowpass_cut_off = 8000.0f;
				target->eq_center = 3600.0f;
				target->eq_bandwidth = 3600.0f;

				return target;
			}

			echo::echo()
			{
				initialize([this]()
				{
					audio_context::set_effect1i(effect, effect_ex::effect_type, (int)effect_ex::effect_echo);
					return true;
				});
			}
			echo::~echo()
			{

			}
			expects_audio<void> echo::synchronize()
			{
				audio_context::set_effect1f(effect, effect_ex::echo_delay, delay);
				audio_context::set_effect1f(effect, effect_ex::echo_lr_delay, lr_delay);
				audio_context::set_effect1f(effect, effect_ex::echo_damping, damping);
				audio_context::set_effect1f(effect, effect_ex::echo_feedback, feedback);
				audio_context::set_effect1f(effect, effect_ex::echo_spread, spread);
				return_error_if;
			}
			void echo::deserialize(core::schema* node)
			{
				VI_ASSERT(node != nullptr, "schema should be set");
				audio_filter* new_filter = get_filter_deserialized(node);
				if (new_filter != nullptr)
					set_filter(&filter);

				layer::series::unpack(node->find("delay"), &delay);
				layer::series::unpack(node->find("lr-delay"), &lr_delay);
				layer::series::unpack(node->find("damping"), &damping);
				layer::series::unpack(node->find("feedback"), &feedback);
				layer::series::unpack(node->find("spread"), &spread);
			}
			void echo::serialize(core::schema* node) const
			{
				VI_ASSERT(node != nullptr, "schema should be set");
				if (filter != nullptr)
					filter->serialize(node->set("filter"));

				layer::series::pack(node->set("delay"), delay);
				layer::series::pack(node->set("lr-delay"), lr_delay);
				layer::series::pack(node->set("damping"), damping);
				layer::series::pack(node->set("feedback"), feedback);
				layer::series::pack(node->set("spread"), spread);
			}
			audio_effect* echo::copy() const
			{
				echo* target = new echo();
				target->delay = 0.1f;
				target->lr_delay = 0.1f;
				target->damping = 0.5f;
				target->feedback = 0.5f;
				target->spread = -1.0f;

				return target;
			}

			flanger::flanger()
			{
				initialize([this]()
				{
					audio_context::set_effect1i(effect, effect_ex::effect_type, (int)effect_ex::effect_flanger);
					return true;
				});
			}
			flanger::~flanger()
			{

			}
			expects_audio<void> flanger::synchronize()
			{
				audio_context::set_effect1f(effect, effect_ex::flanger_rate, rate);
				audio_context::set_effect1f(effect, effect_ex::flanger_depth, depth);
				audio_context::set_effect1f(effect, effect_ex::flanger_feedback, feedback);
				audio_context::set_effect1f(effect, effect_ex::flanger_delay, delay);
				audio_context::set_effect1i(effect, effect_ex::flanger_waveform, waveform);
				audio_context::set_effect1i(effect, effect_ex::flanger_phase, phase);
				return_error_if;
			}
			void flanger::deserialize(core::schema* node)
			{
				VI_ASSERT(node != nullptr, "schema should be set");
				audio_filter* new_filter = get_filter_deserialized(node);
				if (new_filter != nullptr)
					set_filter(&filter);

				layer::series::unpack(node->find("rate"), &rate);
				layer::series::unpack(node->find("depth"), &depth);
				layer::series::unpack(node->find("feedback"), &feedback);
				layer::series::unpack(node->find("delay"), &delay);
				layer::series::unpack(node->find("waveform"), &waveform);
				layer::series::unpack(node->find("phase"), &phase);
			}
			void flanger::serialize(core::schema* node) const
			{
				VI_ASSERT(node != nullptr, "schema should be set");
				if (filter != nullptr)
					filter->serialize(node->set("filter"));

				layer::series::pack(node->set("rate"), rate);
				layer::series::pack(node->set("depth"), depth);
				layer::series::pack(node->set("feedback"), feedback);
				layer::series::pack(node->set("delay"), delay);
				layer::series::pack(node->set("waveform"), waveform);
				layer::series::pack(node->set("phase"), phase);
			}
			audio_effect* flanger::copy() const
			{
				flanger* target = new flanger();
				target->rate = 0.27f;
				target->depth = 1.0f;
				target->feedback = -0.5f;
				target->delay = 0.002f;
				target->waveform = 1;
				target->phase = 0;

				return target;
			}

			frequency_shifter::frequency_shifter()
			{
				initialize([this]()
				{
					audio_context::set_effect1i(effect, effect_ex::effect_type, (int)effect_ex::effect_frequency_shifter);
					return true;
				});
			}
			frequency_shifter::~frequency_shifter()
			{

			}
			expects_audio<void> frequency_shifter::synchronize()
			{
				audio_context::set_effect1f(effect, effect_ex::frequency_shifter_frequency, frequency);
				audio_context::set_effect1i(effect, effect_ex::frequency_shifter_left_direction, left_direction);
				audio_context::set_effect1i(effect, effect_ex::frequency_shifter_right_direction, right_direction);
				return_error_if;
			}
			void frequency_shifter::deserialize(core::schema* node)
			{
				VI_ASSERT(node != nullptr, "schema should be set");
				audio_filter* new_filter = get_filter_deserialized(node);
				if (new_filter != nullptr)
					set_filter(&filter);

				layer::series::unpack(node->find("frequency"), &frequency);
				layer::series::unpack(node->find("left-direction"), &left_direction);
				layer::series::unpack(node->find("right-direction"), &right_direction);
			}
			void frequency_shifter::serialize(core::schema* node) const
			{
				VI_ASSERT(node != nullptr, "schema should be set");
				if (filter != nullptr)
					filter->serialize(node->set("filter"));

				layer::series::pack(node->set("frequency"), frequency);
				layer::series::pack(node->set("left-direction"), left_direction);
				layer::series::pack(node->set("right-direction"), right_direction);
			}
			audio_effect* frequency_shifter::copy() const
			{
				frequency_shifter* target = new frequency_shifter();
				target->frequency = 0.0f;
				target->left_direction = 0;
				target->right_direction = 0;

				return target;
			}

			vocal_morpher::vocal_morpher()
			{
				initialize([this]()
				{
					audio_context::set_effect1i(effect, effect_ex::effect_type, (int)effect_ex::effect_vocmorpher);
					return true;
				});
			}
			vocal_morpher::~vocal_morpher()
			{

			}
			expects_audio<void> vocal_morpher::synchronize()
			{
				audio_context::set_effect1f(effect, effect_ex::vocmorpher_rate, rate);
				audio_context::set_effect1i(effect, effect_ex::vocmorpher_phoneme_a, phonemea);
				audio_context::set_effect1i(effect, effect_ex::vocmorpher_phoneme_acoarse_tuning, phonemea_coarse_tuning);
				audio_context::set_effect1i(effect, effect_ex::vocmorpher_phoneme_b, phonemeb);
				audio_context::set_effect1i(effect, effect_ex::vocmorpher_phoneme_bcoarse_tuning, phonemeb_coarse_tuning);
				audio_context::set_effect1i(effect, effect_ex::vocmorpher_waveform, waveform);
				return_error_if;
			}
			void vocal_morpher::deserialize(core::schema* node)
			{
				VI_ASSERT(node != nullptr, "schema should be set");
				audio_filter* new_filter = get_filter_deserialized(node);
				if (new_filter != nullptr)
					set_filter(&filter);

				layer::series::unpack(node->find("rate"), &rate);
				layer::series::unpack(node->find("phonemea"), &phonemea);
				layer::series::unpack(node->find("phonemea-coarse-tuning"), &phonemea_coarse_tuning);
				layer::series::unpack(node->find("phonemeb"), &phonemeb);
				layer::series::unpack(node->find("phonemeb-coarse-tuning"), &phonemeb_coarse_tuning);
				layer::series::unpack(node->find("waveform"), &waveform);
			}
			void vocal_morpher::serialize(core::schema* node) const
			{
				VI_ASSERT(node != nullptr, "schema should be set");
				if (filter != nullptr)
					filter->serialize(node->set("filter"));

				layer::series::pack(node->set("rate"), rate);
				layer::series::pack(node->set("phonemea"), phonemea);
				layer::series::pack(node->set("phonemea-coarse-tuning"), phonemea_coarse_tuning);
				layer::series::pack(node->set("phonemeb"), phonemeb);
				layer::series::pack(node->set("phonemeb-coarse-tuning"), phonemeb_coarse_tuning);
				layer::series::pack(node->set("waveform"), waveform);
			}
			audio_effect* vocal_morpher::copy() const
			{
				vocal_morpher* target = new vocal_morpher();
				target->rate = 1.41f;
				target->phonemea = 0;
				target->phonemea_coarse_tuning = 0;
				target->phonemeb = 10;
				target->phonemeb_coarse_tuning = 0;
				target->waveform = 0;

				return target;
			}

			pitch_shifter::pitch_shifter()
			{
				initialize([this]()
				{
					audio_context::set_effect1i(effect, effect_ex::effect_type, (int)effect_ex::effect_pitch_shifter);
					return true;
				});
			}
			pitch_shifter::~pitch_shifter()
			{

			}
			expects_audio<void> pitch_shifter::synchronize()
			{
				audio_context::set_effect1i(effect, effect_ex::pitch_shifter_coarse_tune, coarse_tune);
				audio_context::set_effect1i(effect, effect_ex::pitch_shifter_fine_tune, fine_tune);
				return_error_if;
			}
			void pitch_shifter::deserialize(core::schema* node)
			{
				VI_ASSERT(node != nullptr, "schema should be set");
				audio_filter* new_filter = get_filter_deserialized(node);
				if (new_filter != nullptr)
					set_filter(&filter);

				layer::series::unpack(node->find("coarse-tune"), &coarse_tune);
				layer::series::unpack(node->find("fine-tune"), &fine_tune);
			}
			void pitch_shifter::serialize(core::schema* node) const
			{
				VI_ASSERT(node != nullptr, "schema should be set");
				if (filter != nullptr)
					filter->serialize(node->set("filter"));

				layer::series::pack(node->set("coarse-tune"), coarse_tune);
				layer::series::pack(node->set("fine-tune"), fine_tune);
			}
			audio_effect* pitch_shifter::copy() const
			{
				pitch_shifter* target = new pitch_shifter();
				target->coarse_tune = 12;
				target->fine_tune = 0;

				return target;
			}

			ring_modulator::ring_modulator()
			{
				initialize([this]()
				{
					audio_context::set_effect1i(effect, effect_ex::effect_type, (int)effect_ex::effect_ring_modulator);
					return true;
				});
			}
			ring_modulator::~ring_modulator()
			{

			}
			expects_audio<void> ring_modulator::synchronize()
			{
				audio_context::set_effect1f(effect, effect_ex::ring_modulator_frequency, frequency);
				audio_context::set_effect1f(effect, effect_ex::ring_modulator_highpass_cutoff, highpass_cut_off);
				audio_context::set_effect1i(effect, effect_ex::ring_modulator_waveform, waveform);
				return_error_if;
			}
			void ring_modulator::deserialize(core::schema* node)
			{
				VI_ASSERT(node != nullptr, "schema should be set");
				audio_filter* new_filter = get_filter_deserialized(node);
				if (new_filter != nullptr)
					set_filter(&filter);

				layer::series::unpack(node->find("frequency"), &frequency);
				layer::series::unpack(node->find("highpass-cut-off"), &highpass_cut_off);
				layer::series::unpack(node->find("waveform"), &waveform);
			}
			void ring_modulator::serialize(core::schema* node) const
			{
				VI_ASSERT(node != nullptr, "schema should be set");
				if (filter != nullptr)
					filter->serialize(node->set("filter"));

				layer::series::pack(node->set("frequency"), frequency);
				layer::series::pack(node->set("highpass-cut-off"), highpass_cut_off);
				layer::series::pack(node->set("waveform"), waveform);
			}
			audio_effect* ring_modulator::copy() const
			{
				ring_modulator* target = new ring_modulator();
				target->frequency = 440.0f;
				target->highpass_cut_off = 800.0f;
				target->waveform = 0;

				return target;
			}

			autowah::autowah()
			{
				initialize([this]()
				{
					audio_context::set_effect1i(effect, effect_ex::effect_type, (int)effect_ex::effect_autowah);
					return true;
				});
			}
			autowah::~autowah()
			{

			}
			expects_audio<void> autowah::synchronize()
			{
				audio_context::set_effect1f(effect, effect_ex::autowah_attack_time, attack_time);
				audio_context::set_effect1f(effect, effect_ex::autowah_release_time, release_time);
				audio_context::set_effect1f(effect, effect_ex::autowah_resonance, resonance);
				audio_context::set_effect1f(effect, effect_ex::autowah_peak_gain, peak_gain);
				return_error_if;
			}
			void autowah::deserialize(core::schema* node)
			{
				VI_ASSERT(node != nullptr, "schema should be set");
				audio_filter* new_filter = get_filter_deserialized(node);
				if (new_filter != nullptr)
					set_filter(&filter);

				layer::series::unpack(node->find("attack-time"), &attack_time);
				layer::series::unpack(node->find("release-time"), &release_time);
				layer::series::unpack(node->find("resonance"), &resonance);
				layer::series::unpack(node->find("peak-gain"), &peak_gain);
			}
			void autowah::serialize(core::schema* node) const
			{
				VI_ASSERT(node != nullptr, "schema should be set");
				if (filter != nullptr)
					filter->serialize(node->set("filter"));

				layer::series::pack(node->set("attack-time"), attack_time);
				layer::series::pack(node->set("release-time"), release_time);
				layer::series::pack(node->set("resonance"), resonance);
				layer::series::pack(node->set("peak-gain"), peak_gain);
			}
			audio_effect* autowah::copy() const
			{
				autowah* target = new autowah();
				target->attack_time = 0.06f;
				target->release_time = 0.06f;
				target->resonance = 1000.0f;
				target->peak_gain = 11.22f;

				return target;
			}

			compressor::compressor()
			{
				initialize([this]()
				{
					audio_context::set_effect1i(effect, effect_ex::effect_type, (int)effect_ex::effect_compressor);
					audio_context::set_effect1i(effect, effect_ex::compressor_onoff, 1);
					return true;
				});
			}
			compressor::~compressor()
			{
			}
			expects_audio<void> compressor::synchronize()
			{
				return core::expectation::met;
			}
			void compressor::deserialize(core::schema* node)
			{
				VI_ASSERT(node != nullptr, "schema should be set");
				audio_filter* new_filter = get_filter_deserialized(node);
				if (new_filter != nullptr)
					set_filter(&filter);
			}
			void compressor::serialize(core::schema* node) const
			{
				VI_ASSERT(node != nullptr, "schema should be set");
				if (filter != nullptr)
					filter->serialize(node->set("filter"));
			}
			audio_effect* compressor::copy() const
			{
				return new compressor();
			}

			equalizer::equalizer()
			{
				initialize([this]()
				{
					audio_context::set_effect1i(effect, effect_ex::effect_type, (int)effect_ex::effect_equalizer);
					return true;
				});
			}
			equalizer::~equalizer()
			{

			}
			expects_audio<void> equalizer::synchronize()
			{
				audio_context::set_effect1f(effect, effect_ex::equalizer_low_gain, low_gain);
				audio_context::set_effect1f(effect, effect_ex::equalizer_low_cutoff, low_cut_off);
				audio_context::set_effect1f(effect, effect_ex::equalizer_mid1_center, mid1_center);
				audio_context::set_effect1f(effect, effect_ex::equalizer_mid1_width, mid1_width);
				audio_context::set_effect1f(effect, effect_ex::equalizer_mid2_gain, mid2_gain);
				audio_context::set_effect1f(effect, effect_ex::equalizer_mid2_center, mid2_center);
				audio_context::set_effect1f(effect, effect_ex::equalizer_mid2_width, mid2_width);
				audio_context::set_effect1f(effect, effect_ex::equalizer_high_gain, high_gain);
				audio_context::set_effect1f(effect, effect_ex::equalizer_high_cutoff, high_cut_off);
				return_error_if;
			}
			void equalizer::deserialize(core::schema* node)
			{
				VI_ASSERT(node != nullptr, "schema should be set");
				audio_filter* new_filter = get_filter_deserialized(node);
				if (new_filter != nullptr)
					set_filter(&filter);

				layer::series::unpack(node->find("low-gain"), &low_gain);
				layer::series::unpack(node->find("low-cut-off"), &low_cut_off);
				layer::series::unpack(node->find("mid1-gain"), &mid1_gain);
				layer::series::unpack(node->find("mid1-center"), &mid1_center);
				layer::series::unpack(node->find("mid1-width"), &mid1_width);
				layer::series::unpack(node->find("mid2-gain"), &mid2_gain);
				layer::series::unpack(node->find("mid2-center"), &mid2_center);
				layer::series::unpack(node->find("mid2-width"), &mid2_width);
				layer::series::unpack(node->find("high-gain"), &high_gain);
				layer::series::unpack(node->find("high-cut-off"), &high_cut_off);
			}
			void equalizer::serialize(core::schema* node) const
			{
				VI_ASSERT(node != nullptr, "schema should be set");
				if (filter != nullptr)
					filter->serialize(node->set("filter"));

				layer::series::pack(node->find("low-gain"), low_gain);
				layer::series::pack(node->find("low-cut-off"), low_cut_off);
				layer::series::pack(node->find("mid1-gain"), mid1_gain);
				layer::series::pack(node->find("mid1-center"), mid1_center);
				layer::series::pack(node->find("mid1-width"), mid1_width);
				layer::series::pack(node->find("mid2-gain"), mid2_gain);
				layer::series::pack(node->find("mid2-center"), mid2_center);
				layer::series::pack(node->find("mid2-width"), mid2_width);
				layer::series::pack(node->find("high-gain"), high_gain);
				layer::series::pack(node->find("high-cut-off"), high_cut_off);
			}
			audio_effect* equalizer::copy() const
			{
				equalizer* target = new equalizer();
				target->low_gain = 1.0f;
				target->low_cut_off = 200.0f;
				target->mid1_gain = 1.0f;
				target->mid1_center = 500.0f;
				target->mid1_width = 1.0f;
				target->mid2_gain = 1.0f;
				target->mid2_center = 3000.0f;
				target->mid2_width = 1.0f;
				target->high_gain = 1.0f;
				target->high_cut_off = 6000.0f;

				return target;
			}
		}
	}
}
