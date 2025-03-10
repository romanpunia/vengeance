#include "audio.h"
#include "audio/effects.h"
#include "audio/filters.h"
#ifdef VI_OPENAL
#ifdef VI_AL_AT_OPENAL
#include <open_al/al.h>
#include <open_al/alc.h>
#else
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/efx.h>
#include <AL/efx-presets.h>
#define HAS_EFX
#endif
#endif
#define LOAD_PROC(t, x) ((x) = (t)alGetProcAddress(#x))
#if defined(VI_OPENAL) && defined(HAS_EFX)
namespace
{
	LPALGENFILTERS alGenFilters = nullptr;
	LPALDELETEFILTERS alDeleteFilters = nullptr;
	LPALISFILTER alIsFilter = nullptr;
	LPALFILTERI alFilteri = nullptr;
	LPALFILTERIV alFilteriv = nullptr;
	LPALFILTERF alFilterf = nullptr;
	LPALFILTERFV alFilterfv = nullptr;
	LPALGETFILTERI alGetFilteri = nullptr;
	LPALGETFILTERIV alGetFilteriv = nullptr;
	LPALGETFILTERF alGetFilterf = nullptr;
	LPALGETFILTERFV alGetFilterfv = nullptr;
	LPALGENEFFECTS alGenEffects = nullptr;
	LPALDELETEEFFECTS alDeleteEffects = nullptr;
	LPALISEFFECT alIsEffect = nullptr;
	LPALEFFECTI alEffecti = nullptr;
	LPALEFFECTIV alEffectiv = nullptr;
	LPALEFFECTF alEffectf = nullptr;
	LPALEFFECTFV alEffectfv = nullptr;
	LPALGETEFFECTI alGetEffecti = nullptr;
	LPALGETEFFECTIV alGetEffectiv = nullptr;
	LPALGETEFFECTF alGetEffectf = nullptr;
	LPALGETEFFECTFV alGetEffectfv = nullptr;
	LPALGENAUXILIARYEFFECTSLOTS alGenAuxiliaryEffectSlots = nullptr;
	LPALDELETEAUXILIARYEFFECTSLOTS alDeleteAuxiliaryEffectSlots = nullptr;
	LPALISAUXILIARYEFFECTSLOT alIsAuxiliaryEffectSlot = nullptr;
	LPALAUXILIARYEFFECTSLOTI alAuxiliaryEffectSloti = nullptr;
	LPALAUXILIARYEFFECTSLOTIV alAuxiliaryEffectSlotiv = nullptr;
	LPALAUXILIARYEFFECTSLOTF alAuxiliaryEffectSlotf = nullptr;
	LPALAUXILIARYEFFECTSLOTFV alAuxiliaryEffectSlotfv = nullptr;
	LPALGETAUXILIARYEFFECTSLOTI alGetAuxiliaryEffectSloti = nullptr;
	LPALGETAUXILIARYEFFECTSLOTIV alGetAuxiliaryEffectSlotiv = nullptr;
	LPALGETAUXILIARYEFFECTSLOTF alGetAuxiliaryEffectSlotf = nullptr;
	LPALGETAUXILIARYEFFECTSLOTFV alGetAuxiliaryEffectSlotfv = nullptr;
}
#endif
#define return_error_if do { auto __error = audio_exception(); if (__error.has_error()) return __error; else return core::expectation::met; } while (0)
#define return_error_if_dev(device) do { auto __error = audio_exception(device); if (__error.has_error()) return __error; else return core::expectation::met; } while (0)

namespace vitex
{
	namespace audio
	{
		audio_exception::audio_exception(void* device) : al_error_code(0), alc_error_code(0)
		{
#ifdef VI_OPENAL
			al_error_code = alGetError();
			if (al_error_code != AL_NO_ERROR)
			{
				auto* text = alGetString(al_error_code);
				error_message += "AL:" + core::to_string(al_error_code);
				if (text != nullptr)
				{
					error_message += " on ";
					error_message += text;
				}
			}

			if (device != nullptr)
			{
				alc_error_code = alcGetError((ALCdevice*)device);
				if (alc_error_code != ALC_NO_ERROR)
				{
					auto* text = alcGetString((ALCdevice*)device, alc_error_code);
					error_message += core::stringify::text("ALC:%i:0x%" PRIXPTR, alc_error_code, device);
					if (text != nullptr)
					{
						error_message += " on ";
						error_message += text;
					}
				}
			}
#else
			error_message = "audio is not supported";
#endif
		}
		const char* audio_exception::type() const noexcept
		{
			return "audio_error";
		}
		int audio_exception::al_code() const noexcept
		{
			return al_error_code;
		}
		int audio_exception::alc_code() const noexcept
		{
			return alc_error_code;
		}
		bool audio_exception::has_error() const noexcept
		{
#ifdef VI_OPENAL
			return al_error_code != AL_NO_ERROR || alc_error_code != ALC_NO_ERROR;
#else
			return true;
#endif
		}

		expects_audio<void> audio_context::initialize()
		{
			VI_TRACE("[audio] load audio context func addresses");
#if defined(VI_OPENAL) && defined(HAS_EFX)
			LOAD_PROC(LPALGENFILTERS, alGenFilters);
			LOAD_PROC(LPALDELETEFILTERS, alDeleteFilters);
			LOAD_PROC(LPALISFILTER, alIsFilter);
			LOAD_PROC(LPALFILTERI, alFilteri);
			LOAD_PROC(LPALFILTERIV, alFilteriv);
			LOAD_PROC(LPALFILTERF, alFilterf);
			LOAD_PROC(LPALFILTERFV, alFilterfv);
			LOAD_PROC(LPALGETFILTERI, alGetFilteri);
			LOAD_PROC(LPALGETFILTERIV, alGetFilteriv);
			LOAD_PROC(LPALGETFILTERF, alGetFilterf);
			LOAD_PROC(LPALGETFILTERFV, alGetFilterfv);
			LOAD_PROC(LPALGENEFFECTS, alGenEffects);
			LOAD_PROC(LPALDELETEEFFECTS, alDeleteEffects);
			LOAD_PROC(LPALISEFFECT, alIsEffect);
			LOAD_PROC(LPALEFFECTI, alEffecti);
			LOAD_PROC(LPALEFFECTIV, alEffectiv);
			LOAD_PROC(LPALEFFECTF, alEffectf);
			LOAD_PROC(LPALEFFECTFV, alEffectfv);
			LOAD_PROC(LPALGETEFFECTI, alGetEffecti);
			LOAD_PROC(LPALGETEFFECTIV, alGetEffectiv);
			LOAD_PROC(LPALGETEFFECTF, alGetEffectf);
			LOAD_PROC(LPALGETEFFECTFV, alGetEffectfv);
			LOAD_PROC(LPALGENAUXILIARYEFFECTSLOTS, alGenAuxiliaryEffectSlots);
			LOAD_PROC(LPALDELETEAUXILIARYEFFECTSLOTS, alDeleteAuxiliaryEffectSlots);
			LOAD_PROC(LPALISAUXILIARYEFFECTSLOT, alIsAuxiliaryEffectSlot);
			LOAD_PROC(LPALAUXILIARYEFFECTSLOTI, alAuxiliaryEffectSloti);
			LOAD_PROC(LPALAUXILIARYEFFECTSLOTIV, alAuxiliaryEffectSlotiv);
			LOAD_PROC(LPALAUXILIARYEFFECTSLOTF, alAuxiliaryEffectSlotf);
			LOAD_PROC(LPALAUXILIARYEFFECTSLOTFV, alAuxiliaryEffectSlotfv);
			LOAD_PROC(LPALGETAUXILIARYEFFECTSLOTI, alGetAuxiliaryEffectSloti);
			LOAD_PROC(LPALGETAUXILIARYEFFECTSLOTIV, alGetAuxiliaryEffectSlotiv);
			LOAD_PROC(LPALGETAUXILIARYEFFECTSLOTF, alGetAuxiliaryEffectSlotf);
			LOAD_PROC(LPALGETAUXILIARYEFFECTSLOTFV, alGetAuxiliaryEffectSlotfv);
#endif
			return_error_if;
		}
		expects_audio<void> audio_context::generate_buffers(int count, uint32_t* buffers)
		{
			VI_TRACE("[audio] generate %i buffers", count);
#ifdef VI_OPENAL
			alGenBuffers(count, buffers);
#endif
			return_error_if;
		}
		expects_audio<void> audio_context::set_filter1i(uint32_t filter, filter_ex value, int F1)
		{
#if defined(VI_OPENAL) && defined(HAS_EFX)
			alFilteri(filter, (uint32_t)value, F1);
#endif
			return_error_if;
		}
		expects_audio<void> audio_context::set_filter1f(uint32_t filter, filter_ex value, float F1)
		{
#if defined(VI_OPENAL) && defined(HAS_EFX)
			alFilterf(filter, (uint32_t)value, F1);
#endif
			return_error_if;
		}
		expects_audio<void> audio_context::set_effect1i(uint32_t filter, effect_ex value, int F1)
		{
#if defined(VI_OPENAL) && defined(HAS_EFX)
			alEffecti(filter, (uint32_t)value, F1);
#endif
			return_error_if;
		}
		expects_audio<void> audio_context::set_effect1f(uint32_t filter, effect_ex value, float F1)
		{
#if defined(VI_OPENAL) && defined(HAS_EFX)
			alEffectf(filter, (uint32_t)value, F1);
#endif
			return_error_if;
		}
		expects_audio<void> audio_context::set_effectvf(uint32_t filter, effect_ex value, float* FS)
		{
#if defined(VI_OPENAL) && defined(HAS_EFX)
			alEffectfv(filter, (uint32_t)value, FS);
#endif
			return_error_if;
		}
		expects_audio<void> audio_context::set_buffer_data(uint32_t buffer, int format, const void* data, int size, int frequency)
		{
#ifdef VI_OPENAL
			alBufferData(buffer, format, data, size, frequency);
#endif
			return_error_if;
		}
		expects_audio<void> audio_context::set_source_data3f(uint32_t source, sound_ex value, float F1, float F2, float F3)
		{
#ifdef VI_OPENAL
			alSource3f(source, (uint32_t)value, F1, F2, F3);
#endif
			return_error_if;
		}
		expects_audio<void> audio_context::get_source_data3f(uint32_t source, sound_ex value, float* F1, float* F2, float* F3)
		{
#ifdef VI_OPENAL
			alGetSource3f(source, (uint32_t)value, F1, F2, F3);
#endif
			return_error_if;
		}
		expects_audio<void> audio_context::set_source_datavf(uint32_t source, sound_ex value, float* FS)
		{
#ifdef VI_OPENAL
			alSourcefv(source, (uint32_t)value, FS);
#endif
			return_error_if;
		}
		expects_audio<void> audio_context::get_source_datavf(uint32_t source, sound_ex value, float* FS)
		{
#ifdef VI_OPENAL
			alGetSourcefv(source, (uint32_t)value, FS);
#endif
			return_error_if;
		}
		expects_audio<void> audio_context::set_source_data1f(uint32_t source, sound_ex value, float F1)
		{
#ifdef VI_OPENAL
			alSourcef(source, (uint32_t)value, F1);
#endif
			return_error_if;
		}
		expects_audio<void> audio_context::get_source_data1f(uint32_t source, sound_ex value, float* F1)
		{
#ifdef VI_OPENAL
			alGetSourcef(source, (uint32_t)value, F1);
#endif
			return_error_if;
		}
		expects_audio<void> audio_context::set_source_data3i(uint32_t source, sound_ex value, int F1, int F2, int F3)
		{
#ifdef VI_OPENAL
			alSource3i(source, (uint32_t)value, F1, F2, F3);
#endif
			return_error_if;
		}
		expects_audio<void> audio_context::get_source_data3i(uint32_t source, sound_ex value, int* F1, int* F2, int* F3)
		{
#ifdef VI_OPENAL
			alGetSource3i(source, (uint32_t)value, F1, F2, F3);
#endif
			return_error_if;
		}
		expects_audio<void> audio_context::set_source_datavi(uint32_t source, sound_ex value, int* FS)
		{
#ifdef VI_OPENAL
			alSourceiv(source, (uint32_t)value, FS);
#endif
			return_error_if;
		}
		expects_audio<void> audio_context::get_source_datavi(uint32_t source, sound_ex value, int* FS)
		{
#ifdef VI_OPENAL
			alGetSourceiv(source, (uint32_t)value, FS);
#endif
			return_error_if;
		}
		expects_audio<void> audio_context::set_source_data1i(uint32_t source, sound_ex value, int F1)
		{
#ifdef VI_OPENAL
			alSourcei(source, (uint32_t)value, F1);
#endif
			return_error_if;
		}
		expects_audio<void> audio_context::get_source_data1i(uint32_t source, sound_ex value, int* F1)
		{
#ifdef VI_OPENAL
			alGetSourcei(source, (uint32_t)value, F1);
#endif
			return_error_if;
		}
		expects_audio<void> audio_context::set_listener_data3f(sound_ex listener, float F1, float F2, float F3)
		{
#ifdef VI_OPENAL
			alListener3f((uint32_t)listener, F1, F2, F3);
#endif
			return_error_if;
		}
		expects_audio<void> audio_context::get_listener_data3f(sound_ex listener, float* F1, float* F2, float* F3)
		{
#ifdef VI_OPENAL
			alGetListener3f((uint32_t)listener, F1, F2, F3);
#endif
			return_error_if;
		}
		expects_audio<void> audio_context::set_listener_datavf(sound_ex listener, float* FS)
		{
#ifdef VI_OPENAL
			alListenerfv((uint32_t)listener, FS);
#endif
			return_error_if;
		}
		expects_audio<void> audio_context::get_listener_datavf(sound_ex listener, float* FS)
		{
#ifdef VI_OPENAL
			alGetListenerfv((uint32_t)listener, FS);
#endif
			return_error_if;
		}
		expects_audio<void> audio_context::set_listener_data1f(sound_ex listener, float F1)
		{
#ifdef VI_OPENAL
			alListenerf((uint32_t)listener, F1);
#endif
			return_error_if;
		}
		expects_audio<void> audio_context::get_listener_data1f(sound_ex listener, float* F1)
		{
#ifdef VI_OPENAL
			alGetListenerf((uint32_t)listener, F1);
#endif
			return_error_if;
		}
		expects_audio<void> audio_context::set_listener_data3i(sound_ex listener, int F1, int F2, int F3)
		{
#ifdef VI_OPENAL
			alListener3i((uint32_t)listener, F1, F2, F3);
#endif
			return_error_if;
		}
		expects_audio<void> audio_context::get_listener_data3i(sound_ex listener, int* F1, int* F2, int* F3)
		{
#ifdef VI_OPENAL
			alGetListener3i((uint32_t)listener, F1, F2, F3);
#endif
			return_error_if;
		}
		expects_audio<void> audio_context::set_listener_datavi(sound_ex listener, int* FS)
		{
#ifdef VI_OPENAL
			alListeneriv((uint32_t)listener, FS);
#endif
			return_error_if;
		}
		expects_audio<void> audio_context::get_listener_datavi(sound_ex listener, int* FS)
		{
#ifdef VI_OPENAL
			alGetListeneriv((uint32_t)listener, FS);
#endif
			return_error_if;
		}
		expects_audio<void> audio_context::set_listener_data1i(sound_ex listener, int F1)
		{
#ifdef VI_OPENAL
			alListeneri((uint32_t)listener, F1);
#endif
			return_error_if;
		}
		expects_audio<void> audio_context::get_listener_data1i(sound_ex listener, int* F1)
		{
#ifdef VI_OPENAL
			alGetListeneri((uint32_t)listener, F1);
#endif
			return_error_if;
		}
		uint32_t audio_context::get_enum_value(const char* name)
		{
#ifdef VI_OPENAL
			VI_ASSERT(name != nullptr, "name should be set");
			return (uint32_t)alGetEnumValue(name);
#else
			return 0;
#endif
		}

		audio_filter::audio_filter() noexcept
		{
#if defined(VI_OPENAL) && defined(HAS_EFX)
			filter = AL_FILTER_NULL;
#endif
		}
		audio_filter::~audio_filter() noexcept
		{
#if defined(VI_OPENAL) && defined(HAS_EFX)
			VI_TRACE("[audio] delete %i filter", (int)filter);
			if (alDeleteFilters != nullptr && filter != AL_FILTER_NULL)
				alDeleteFilters(1, &filter);
#endif
		}
		expects_audio<void> audio_filter::initialize(const std::function<bool()>& callback)
		{
#if defined(VI_OPENAL) && defined(HAS_EFX)
			if (alDeleteFilters != nullptr && filter != AL_FILTER_NULL)
				alDeleteFilters(1, &filter);

			if (alGenFilters != nullptr)
				alGenFilters(1, &filter);

			if (callback)
				callback();
			VI_TRACE("[audio] generate %i filter", (int)filter);
#endif
			return_error_if;
		}
		audio_source* audio_filter::get_source() const
		{
			return source;
		}

		audio_effect::audio_effect() noexcept
		{
#if defined(VI_OPENAL) && defined(HAS_EFX)
			effect = AL_EFFECT_NULL;
			slot = AL_EFFECTSLOT_NULL;
#endif
		}
		audio_effect::~audio_effect() noexcept
		{
			unbind();
#if defined(VI_OPENAL) && defined(HAS_EFX)
			VI_TRACE("[audio] delete %i effect", (int)effect);
			if (alDeleteEffects != nullptr && effect != AL_EFFECT_NULL)
				alDeleteEffects(1, &effect);

			if (alDeleteAuxiliaryEffectSlots != nullptr && slot != AL_EFFECTSLOT_NULL)
				alDeleteAuxiliaryEffectSlots(1, &slot);
#endif
			core::memory::release(filter);
		}
		expects_audio<void> audio_effect::initialize(const std::function<bool()>& callback)
		{
#if defined(VI_OPENAL) && defined(HAS_EFX)
			if (alDeleteAuxiliaryEffectSlots != nullptr && slot != AL_EFFECTSLOT_NULL)
				alDeleteAuxiliaryEffectSlots(1, &slot);

			if (alGenAuxiliaryEffectSlots != nullptr)
				alGenAuxiliaryEffectSlots(1, &slot);

			if (alDeleteEffects != nullptr && effect != AL_EFFECT_NULL)
				alDeleteEffects(1, &effect);

			if (alGenEffects != nullptr)
				alGenEffects(1, &effect);

			if (callback)
				callback();

			if (alAuxiliaryEffectSloti != nullptr && effect != AL_EFFECT_NULL)
				alAuxiliaryEffectSloti(slot, AL_EFFECTSLOT_EFFECT, (ALint)effect);
			VI_TRACE("[audio] generate %i effect", (int)effect);
#endif
			return_error_if;
		}
		expects_audio<void> audio_effect::set_filter(audio_filter** new_filter)
		{
			core::memory::release(filter);
			if (!new_filter || !*new_filter)
				return core::expectation::met;

			filter = *new_filter;
			*new_filter = nullptr;
			return bind(source, zone);
		}
		expects_audio<void> audio_effect::bind(audio_source* new_source, int new_zone)
		{
			VI_ASSERT(source != nullptr, "source should not be empty");
			source = new_source;
			zone = new_zone;
#if defined(VI_OPENAL) && defined(HAS_EFX)
			alSource3i(source->get_instance(), AL_AUXILIARY_SEND_FILTER, (ALint)slot, zone, (ALint)(filter ? filter->filter : AL_FILTER_NULL));
#endif
			return_error_if;
		}
		expects_audio<void> audio_effect::unbind()
		{
			VI_ASSERT(source != nullptr, "source should not be empty");
#if defined(VI_OPENAL) && defined(HAS_EFX)
			alSource3i(source->get_instance(), AL_AUXILIARY_SEND_FILTER, AL_EFFECTSLOT_NULL, zone, AL_FILTER_NULL);
#endif
			return_error_if;
		}
		audio_filter* audio_effect::get_filter() const
		{
			return filter;
		}
		audio_source* audio_effect::get_source() const
		{
			return source;
		}

		audio_clip::audio_clip(int buffer_count, int new_format) noexcept : format(new_format)
		{
			if (buffer_count > 0)
				audio_context::generate_buffers(buffer_count, &buffer);
		}
		audio_clip::~audio_clip() noexcept
		{
#ifdef VI_OPENAL
			VI_TRACE("[audio] delete %i buffer", (int)buffer);
			alDeleteBuffers(1, &buffer);
			buffer = 0;
#endif
		}
		float audio_clip::length() const
		{
#ifdef VI_OPENAL
			int byte_size = 0, channel_count = 0, bits = 0, frequency = 0;
			alGetBufferi(buffer, AL_SIZE, &byte_size);
			alGetBufferi(buffer, AL_CHANNELS, &channel_count);
			alGetBufferi(buffer, AL_BITS, &bits);
			alGetBufferi(buffer, AL_FREQUENCY, &frequency);

			if (byte_size == 0 || channel_count == 0 || bits == 0 || frequency == 0)
				return 0;

			return (float)(byte_size * 8 / (channel_count * bits)) / (float)frequency;
#else
			return 0.0f;
#endif
		}
		bool audio_clip::is_mono() const
		{
#ifdef VI_OPENAL
			if (format == AL_FORMAT_MONO8 || format == AL_FORMAT_MONO16)
				return true;
#endif
			return false;
		}
		uint32_t audio_clip::get_buffer() const
		{
			return buffer;
		}
		int audio_clip::get_format() const
		{
			return format;
		}

		audio_source::audio_source() noexcept
		{
#ifdef VI_OPENAL
			if (alIsSource(instance))
			{
				alSourceStop(instance);
				alDeleteSources(1, &instance);
			}

			alGenSources(1, &instance);
			alSource3f(instance, AL_DIRECTION, 0, 0, 0);
			alSourcei(instance, AL_SOURCE_RELATIVE, 0);
			alSourcei(instance, AL_LOOPING, 0);
			alSourcef(instance, AL_PITCH, 1);
			alSourcef(instance, AL_GAIN, 1);
			alSourcef(instance, AL_MAX_DISTANCE, 100);
			alSourcef(instance, AL_REFERENCE_DISTANCE, 0.25f);
			alSourcef(instance, AL_ROLLOFF_FACTOR, 1);
			alSourcef(instance, AL_CONE_INNER_ANGLE, 360);
			alSourcef(instance, AL_CONE_OUTER_ANGLE, 360);
			alSourcef(instance, AL_CONE_OUTER_GAIN, 0);
			alSource3f(instance, AL_POSITION, 0, 0, 0);
			alSourcei(instance, AL_SEC_OFFSET, 0);
			VI_TRACE("[audio] generate %i source", (int)instance);
#endif
		}
		audio_source::~audio_source() noexcept
		{
			remove_effects();
			core::memory::release(clip);
#ifdef VI_OPENAL
			VI_TRACE("[audio] delete %i source", (int)instance);
			alSourceStop(instance);
			alSourcei(instance, AL_BUFFER, 0);
			alDeleteSources(1, &instance);
#endif
		}
		int64_t audio_source::add_effect(audio_effect* effect)
		{
			VI_ASSERT(effect != nullptr, "effect should be set");
			effect->bind(this, (int)effects.size());
			effects.push_back(effect);
			return effects.size() - 1;
		}
		expects_audio<void> audio_source::remove_effect(size_t effect_id)
		{
			VI_ASSERT(effect_id < effects.size(), "index outside of range");
			auto it = effects.begin() + effect_id;
			core::memory::release(*it);
			effects.erase(it);

			for (size_t i = effect_id; i < effects.size(); i++)
			{
				audio_effect* effect = effects[i];
				effect->unbind();
				effect->bind(this, (int)i);
			}

			return_error_if;
		}
		expects_audio<void> audio_source::remove_effect_by_id(uint64_t effect_id)
		{
			for (size_t i = 0; i < effects.size(); i++)
			{
				if (effects[i]->get_id() == effect_id)
					return remove_effect(i);
			}

			return_error_if;
		}
		expects_audio<void> audio_source::remove_effects()
		{
			for (auto* effect : effects)
				core::memory::release(effect);

			effects.clear();
			return_error_if;
		}
		expects_audio<void> audio_source::set_clip(audio_clip* new_clip)
		{
#ifdef VI_OPENAL
			VI_TRACE("[audio] apply clip %i on %i source", new_clip ? (int)new_clip->get_buffer() : 0, (int)instance);
			alSourceStop(instance);

			core::memory::release(clip);
			clip = new_clip;
			if (clip != nullptr)
			{
				alSourcei(instance, AL_BUFFER, clip->get_buffer());
				clip->add_ref();
			}
#endif
			return_error_if;
		}
		expects_audio<void> audio_source::synchronize(audio_sync* sync, const trigonometry::vector3& position)
		{
			VI_ASSERT(sync != nullptr, "sync should be set");
			for (auto* effect : effects)
			{
				if (!effect)
					continue;

				effect->synchronize();
				if (effect->filter != nullptr)
					effect->filter->synchronize();
#if defined(VI_OPENAL) && defined(HAS_EFX)
				if (alAuxiliaryEffectSloti != nullptr && effect->effect != AL_EFFECT_NULL && effect->slot != AL_EFFECTSLOT_NULL)
					alAuxiliaryEffectSloti(effect->slot, AL_EFFECTSLOT_EFFECT, (ALint)effect->effect);
#endif
			}
#ifdef VI_OPENAL
			if (!sync->is_relative)
				alSource3f(instance, AL_POSITION, 0, 0, 0);
			else
				alSource3f(instance, AL_POSITION, -position.x, -position.y, position.z);
#ifdef AL_ROOM_ROLLOFF_FACTOR
			alSourcef(instance, AL_ROOM_ROLLOFF_FACTOR, sync->room_roll_off);
#endif
#ifdef AL_AIR_ABSORPTION_FACTOR
			alSourcef(instance, AL_AIR_ABSORPTION_FACTOR, sync->air_absorption);
#endif
			alSource3f(instance, AL_VELOCITY, sync->velocity.x, sync->velocity.y, sync->velocity.z);
			alSource3f(instance, AL_DIRECTION, sync->direction.x, sync->direction.y, sync->direction.z);
			alSourcei(instance, AL_SOURCE_RELATIVE, sync->is_relative ? 0 : 1);
			alSourcei(instance, AL_LOOPING, sync->is_looped ? 1 : 0);
			alSourcef(instance, AL_PITCH, sync->pitch);
			alSourcef(instance, AL_GAIN, sync->gain);
			alSourcef(instance, AL_MAX_DISTANCE, sync->distance);
			alSourcef(instance, AL_REFERENCE_DISTANCE, sync->ref_distance);
			alSourcef(instance, AL_ROLLOFF_FACTOR, sync->rolloff);
			alSourcef(instance, AL_CONE_INNER_ANGLE, sync->cone_inner_angle);
			alSourcef(instance, AL_CONE_OUTER_ANGLE, sync->cone_outer_angle);
			alSourcef(instance, AL_CONE_OUTER_GAIN, sync->cone_outer_gain);
			alGetSourcef(instance, AL_SEC_OFFSET, &sync->position);
#endif
			return_error_if;
		}
		expects_audio<void> audio_source::reset()
		{
#ifdef VI_OPENAL
			VI_TRACE("[audio] reset on %i source", (int)instance);
			alSource3f(instance, AL_DIRECTION, 0, 0, 0);
			alSourcei(instance, AL_SOURCE_RELATIVE, 0);
			alSourcei(instance, AL_LOOPING, 0);
			alSourcef(instance, AL_PITCH, 1);
			alSourcef(instance, AL_GAIN, 1);
			alSourcef(instance, AL_MAX_DISTANCE, 100);
			alSourcef(instance, AL_REFERENCE_DISTANCE, 0.25f);
			alSourcef(instance, AL_ROLLOFF_FACTOR, 1);
			alSourcef(instance, AL_CONE_INNER_ANGLE, 360);
			alSourcef(instance, AL_CONE_OUTER_ANGLE, 360);
			alSourcef(instance, AL_CONE_OUTER_GAIN, 0);
			alSource3f(instance, AL_POSITION, 0, 0, 0);
			alSourcei(instance, AL_SEC_OFFSET, 0);
#endif
			return_error_if;
		}
		expects_audio<void> audio_source::pause()
		{
#ifdef VI_OPENAL
			VI_TRACE("[audio] pause on %i source", (int)instance);
			alSourcePause(instance);
#endif
			return_error_if;
		}
		expects_audio<void> audio_source::play()
		{
#ifdef VI_OPENAL
			VI_TRACE("[audio] play on %i source", (int)instance);
			alSourcePlay(instance);
#endif
			return_error_if;
		}
		expects_audio<void> audio_source::stop()
		{
#ifdef VI_OPENAL
			VI_TRACE("[audio] stop on %i source", (int)instance);
			alSourceStop(instance);
#endif
			return_error_if;
		}
		bool audio_source::is_playing() const
		{
#ifdef VI_OPENAL
			int state = 0;
			alGetSourcei(instance, AL_SOURCE_STATE, &state);
			return state == AL_PLAYING;
#else
			return false;
#endif
		}
		size_t audio_source::get_effects_count() const
		{
			return effects.size();
		}
		audio_clip* audio_source::get_clip() const
		{
			return clip;
		}
		audio_effect* audio_source::get_effect(uint64_t section) const
		{
			for (auto* effect : effects)
			{
				if (effect && effect->get_id() == section)
					return effect;
			}

			return nullptr;
		}
		uint32_t audio_source::get_instance() const
		{
			return instance;
		}
		const core::vector<audio_effect*>& audio_source::get_effects() const
		{
			return effects;
		}

		audio_device::audio_device() noexcept
		{
#ifdef VI_OPENAL
			device = (void*)alcOpenDevice(nullptr);
			VI_TRACE("[audio] open alc device: 0x%" PRIXPTR, (void*)device);
			VI_PANIC(device != nullptr, "audio device cannot be created [ %s ]", alGetString(alGetError()));

			context = (void*)alcCreateContext((ALCdevice*)device, nullptr);
			VI_TRACE("[audio] create alc context: 0x%" PRIXPTR, (void*)context);
			VI_PANIC(context != nullptr, "audio context cannot be created [ %s ]", alcGetString((ALCdevice*)device, alcGetError((ALCdevice*)device)));

			alcMakeContextCurrent((ALCcontext*)context);
			alDistanceModel(AL_LINEAR_DISTANCE);
			alListenerf(AL_GAIN, 0.0f);
#endif
		}
		audio_device::~audio_device() noexcept
		{
#ifdef VI_OPENAL
			if (context != nullptr)
			{
				VI_TRACE("[audio] delete alc context: 0x%" PRIXPTR, (void*)context);
				alcMakeContextCurrent(nullptr);
				alcDestroyContext((ALCcontext*)context);
				context = nullptr;
			}

			if (device != nullptr)
			{
				VI_TRACE("[audio] close alc context: 0x%" PRIXPTR, (void*)device);
				alcCloseDevice((ALCdevice*)device);
				device = nullptr;
			}
#endif
		}
		expects_audio<void> audio_device::offset(audio_source* source, float& seconds, bool get)
		{
			VI_ASSERT(source != nullptr, "souce should be set");
#ifdef VI_OPENAL
			if (!get)
				alSourcef(source->instance, AL_SEC_OFFSET, seconds);
			else
				alGetSourcef(source->instance, AL_SEC_OFFSET, &seconds);
#endif
			return_error_if_dev(device);
		}
		expects_audio<void> audio_device::relative(audio_source* source, int& value, bool get)
		{
			VI_ASSERT(source != nullptr, "souce should be set");
#ifdef VI_OPENAL
			if (!get)
				alSourcei(source->instance, AL_SOURCE_RELATIVE, value);
			else
				alGetSourcei(source->instance, AL_SOURCE_RELATIVE, &value);
#endif
			return_error_if_dev(device);
		}
		expects_audio<void> audio_device::position(audio_source* source, trigonometry::vector3& position, bool get)
		{
			VI_ASSERT(source != nullptr, "souce should be set");
#ifdef VI_OPENAL
			if (!get)
			{
				alGetSource3f(source->instance, AL_POSITION, &position.x, &position.y, &position.z);
				position.x = -position.x;
			}
			else
				alSource3f(source->instance, AL_POSITION, -position.x, position.y, position.z);
#endif
			return_error_if_dev(device);
		}
		expects_audio<void> audio_device::direction(audio_source* source, trigonometry::vector3& direction, bool get)
		{
			VI_ASSERT(source != nullptr, "souce should be set");
#ifdef VI_OPENAL
			if (!get)
				alSource3f(source->instance, AL_DIRECTION, direction.x, direction.y, direction.z);
			else
				alGetSource3f(source->instance, AL_DIRECTION, &direction.x, &direction.y, &direction.z);
#endif
			return_error_if_dev(device);
		}
		expects_audio<void> audio_device::velocity(audio_source* source, trigonometry::vector3& velocity, bool get)
		{
			VI_ASSERT(source != nullptr, "souce should be set");
#ifdef VI_OPENAL
			if (!get)
				alSource3f(source->instance, AL_VELOCITY, velocity.x, velocity.y, velocity.z);
			else
				alGetSource3f(source->instance, AL_VELOCITY, &velocity.x, &velocity.y, &velocity.z);
#endif
			return_error_if_dev(device);
		}
		expects_audio<void> audio_device::pitch(audio_source* source, float& value, bool get)
		{
			VI_ASSERT(source != nullptr, "souce should be set");
#ifdef VI_OPENAL
			if (!get)
				alSourcef(source->instance, AL_PITCH, value);
			else
				alGetSourcef(source->instance, AL_PITCH, &value);
#endif
			return_error_if_dev(device);
		}
		expects_audio<void> audio_device::gain(audio_source* source, float& value, bool get)
		{
			VI_ASSERT(source != nullptr, "souce should be set");
#ifdef VI_OPENAL
			if (!get)
				alSourcef(source->instance, AL_GAIN, value);
			else
				alGetSourcef(source->instance, AL_GAIN, &value);
#endif
			return_error_if_dev(device);
		}
		expects_audio<void> audio_device::cone_inner_angle(audio_source* source, float& value, bool get)
		{
			VI_ASSERT(source != nullptr, "souce should be set");
#ifdef VI_OPENAL
			if (!get)
				alSourcef(source->instance, AL_CONE_INNER_ANGLE, value);
			else
				alGetSourcef(source->instance, AL_CONE_INNER_ANGLE, &value);
#endif
			return_error_if_dev(device);
		}
		expects_audio<void> audio_device::cone_outer_angle(audio_source* source, float& value, bool get)
		{
			VI_ASSERT(source != nullptr, "souce should be set");
#ifdef VI_OPENAL
			if (!get)
				alSourcef(source->instance, AL_CONE_OUTER_ANGLE, value);
			else
				alGetSourcef(source->instance, AL_CONE_OUTER_ANGLE, &value);
#endif
			return_error_if_dev(device);
		}
		expects_audio<void> audio_device::cone_outer_gain(audio_source* source, float& value, bool get)
		{
			VI_ASSERT(source != nullptr, "souce should be set");
#ifdef VI_OPENAL
			if (!get)
				alSourcef(source->instance, AL_CONE_OUTER_GAIN, value);
			else
				alGetSourcef(source->instance, AL_CONE_OUTER_GAIN, &value);
#endif
			return_error_if_dev(device);
		}
		expects_audio<void> audio_device::distance(audio_source* source, float& value, bool get)
		{
			VI_ASSERT(source != nullptr, "souce should be set");
#ifdef VI_OPENAL
			if (!get)
				alSourcef(source->instance, AL_MAX_DISTANCE, value);
			else
				alGetSourcef(source->instance, AL_MAX_DISTANCE, &value);
#endif
			return_error_if_dev(device);
		}
		expects_audio<void> audio_device::ref_distance(audio_source* source, float& value, bool get)
		{
			VI_ASSERT(source != nullptr, "souce should be set");
#ifdef VI_OPENAL
			if (!get)
				alSourcef(source->instance, AL_REFERENCE_DISTANCE, value);
			else
				alGetSourcef(source->instance, AL_REFERENCE_DISTANCE, &value);
#endif
			return_error_if_dev(device);
		}
		expects_audio<void> audio_device::loop(audio_source* source, int& is_loop, bool get)
		{
			VI_ASSERT(source != nullptr, "souce should be set");
#ifdef VI_OPENAL
			if (!get)
				alSourcei(source->instance, AL_LOOPING, is_loop);
			else
				alGetSourcei(source->instance, AL_LOOPING, &is_loop);
#endif
			return_error_if_dev(device);
		}
		expects_audio<void> audio_device::set_distance_model(sound_distance_model model)
		{
#ifdef VI_OPENAL
			alDistanceModel((int)model);
#endif
			return_error_if_dev(device);
		}
		void audio_device::display_audio_log() const
		{
#ifdef VI_OPENAL
			int ALCCode = ALC_NO_ERROR;
			if ((ALCCode = alcGetError((ALCdevice*)device)) != ALC_NO_ERROR)
				VI_ERR("[openalc] %s", alcGetString((ALCdevice*)device, ALCCode));

			int ALCode = AL_NO_ERROR;
			if ((ALCode = alGetError()) != AL_NO_ERROR)
				VI_ERR("[openal] %s", alGetString(ALCode));
#endif
		}
		bool audio_device::is_valid() const
		{
			return context && device;
		}
	}
}
