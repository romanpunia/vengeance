#include "vengeance.h"
#include "audio.h"
#include "bindings.h"
#include "layer/gui.h"
#include <sstream>
#ifdef VI_SDL2
#include "internal/sdl2.hpp"
#endif
#ifdef VI_ASSIMP
#include <assimp/DefaultLogger.hpp>
#endif
#ifdef VI_GLEW
#include <GL/glew.h>
#endif

namespace vitex
{
	heavy_runtime::heavy_runtime(size_t modules, core::global_allocator* allocator) noexcept : runtime(modules, allocator)
	{
		if (modes & use_platform)
			initialize_platform();

		if (modes & use_graphics)
			initialize_graphics();

		if (modes & use_audio)
			initialize_audio();
	}
	heavy_runtime::~heavy_runtime() noexcept
	{
		core::error_handling::set_flag(core::log_option::async, false);
		cleanup_instances();
		if (modes & use_platform)
			cleanup_platform();
		cleanup_importer();
		cleanup_scripting();
	}
	bool heavy_runtime::initialize_platform() noexcept
	{
#ifdef VI_SDL2
		SDL_SetMainReady();
		int code = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER | SDL_INIT_HAPTIC);
		VI_PANIC(code == 0, "SDL2 initialization failed code:%i", code);
		SDL_EventState(SDL_QUIT, SDL_ENABLE);
		SDL_EventState(SDL_APP_TERMINATING, SDL_ENABLE);
		SDL_EventState(SDL_APP_LOWMEMORY, SDL_ENABLE);
		SDL_EventState(SDL_APP_WILLENTERBACKGROUND, SDL_ENABLE);
		SDL_EventState(SDL_APP_DIDENTERBACKGROUND, SDL_ENABLE);
		SDL_EventState(SDL_APP_WILLENTERFOREGROUND, SDL_ENABLE);
		SDL_EventState(SDL_APP_DIDENTERFOREGROUND, SDL_ENABLE);
		SDL_EventState(SDL_APP_DIDENTERFOREGROUND, SDL_ENABLE);
		SDL_EventState(SDL_WINDOWEVENT, SDL_ENABLE);
		SDL_EventState(SDL_SYSWMEVENT, SDL_DISABLE);
		SDL_EventState(SDL_KEYDOWN, SDL_ENABLE);
		SDL_EventState(SDL_KEYUP, SDL_ENABLE);
		SDL_EventState(SDL_TEXTEDITING, SDL_ENABLE);
		SDL_EventState(SDL_TEXTINPUT, SDL_ENABLE);
#if SDL_VERSION_ATLEAST(2, 0, 4)
		SDL_EventState(SDL_KEYMAPCHANGED, SDL_DISABLE);
#endif
		SDL_EventState(SDL_MOUSEMOTION, SDL_ENABLE);
		SDL_EventState(SDL_MOUSEBUTTONDOWN, SDL_ENABLE);
		SDL_EventState(SDL_MOUSEBUTTONUP, SDL_ENABLE);
		SDL_EventState(SDL_MOUSEWHEEL, SDL_ENABLE);
		SDL_EventState(SDL_JOYAXISMOTION, SDL_ENABLE);
		SDL_EventState(SDL_JOYBALLMOTION, SDL_ENABLE);
		SDL_EventState(SDL_JOYHATMOTION, SDL_ENABLE);
		SDL_EventState(SDL_JOYBUTTONDOWN, SDL_ENABLE);
		SDL_EventState(SDL_JOYBUTTONUP, SDL_ENABLE);
		SDL_EventState(SDL_JOYDEVICEADDED, SDL_ENABLE);
		SDL_EventState(SDL_JOYDEVICEREMOVED, SDL_ENABLE);
		SDL_EventState(SDL_CONTROLLERAXISMOTION, SDL_ENABLE);
		SDL_EventState(SDL_CONTROLLERBUTTONDOWN, SDL_ENABLE);
		SDL_EventState(SDL_CONTROLLERBUTTONUP, SDL_ENABLE);
		SDL_EventState(SDL_CONTROLLERDEVICEADDED, SDL_ENABLE);
		SDL_EventState(SDL_CONTROLLERDEVICEREMOVED, SDL_ENABLE);
		SDL_EventState(SDL_CONTROLLERDEVICEREMAPPED, SDL_ENABLE);
		SDL_EventState(SDL_FINGERDOWN, SDL_ENABLE);
		SDL_EventState(SDL_FINGERUP, SDL_ENABLE);
		SDL_EventState(SDL_FINGERMOTION, SDL_ENABLE);
		SDL_EventState(SDL_DOLLARGESTURE, SDL_ENABLE);
		SDL_EventState(SDL_DOLLARRECORD, SDL_ENABLE);
		SDL_EventState(SDL_MULTIGESTURE, SDL_ENABLE);
		SDL_EventState(SDL_CLIPBOARDUPDATE, SDL_DISABLE);
#if SDL_VERSION_ATLEAST(2, 0, 5)
		SDL_EventState(SDL_DROPFILE, SDL_ENABLE);
		SDL_EventState(SDL_DROPTEXT, SDL_ENABLE);
		SDL_EventState(SDL_DROPBEGIN, SDL_ENABLE);
		SDL_EventState(SDL_DROPCOMPLETE, SDL_ENABLE);
#endif
#if SDL_VERSION_ATLEAST(2, 0, 4)
		SDL_EventState(SDL_AUDIODEVICEADDED, SDL_DISABLE);
		SDL_EventState(SDL_AUDIODEVICEREMOVED, SDL_DISABLE);
		SDL_EventState(SDL_RENDER_DEVICE_RESET, SDL_DISABLE);
#endif
		SDL_EventState(SDL_RENDER_TARGETS_RESET, SDL_DISABLE);
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
		SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

		const char* platform = SDL_GetPlatform();
		if (!strcmp(platform, "iOS") || !strcmp(platform, "Android") || !strcmp(platform, "Unknown"))
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
		else
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

		VI_TRACE("[lib] initialize sdl2 library");
		return true;
#else
		return false;
#endif
	}
	bool heavy_runtime::initialize_graphics() noexcept
	{
#ifdef VI_GLEW
		glewExperimental = true;
		VI_TRACE("[lib] initialize graphics library");
		return true;
#else
		return false;
#endif
	}
	bool heavy_runtime::initialize_audio() noexcept
	{
		VI_TRACE("[lib] initialize audio library");
		auto status = audio::audio_context::initialize();
		status.report("audio initialization error");
		return !!status;
	}
	void heavy_runtime::cleanup_instances() noexcept
	{
		runtime::cleanup_instances();
		layer::gui::subsystem::cleanup_instance();
	}
	void heavy_runtime::cleanup_platform() noexcept
	{
#ifdef VI_SDL2
		SDL_Quit();
		VI_TRACE("[lib] free sdl2 library");
#endif
	}
	void heavy_runtime::cleanup_importer() noexcept
	{
#ifdef VI_ASSIMP
		Assimp::DefaultLogger::kill();
		VI_TRACE("[lib] free importer library");
#endif
	}
	void heavy_runtime::cleanup_scripting() noexcept
	{
		scripting::bindings::heavy_registry().cleanup();
		VI_TRACE("[lib] free heavy bindings registry");
	}
	bool heavy_runtime::has_ft_shaders() const noexcept
	{
		return has_so_spirv();
	}
	bool heavy_runtime::has_so_opengl() const noexcept
	{
#ifdef VI_OPENGL
		return true;
#else
		return false;
#endif
	}
	bool heavy_runtime::has_so_openal() const noexcept
	{
#ifdef VI_OPENAL
		return true;
#else
		return false;
#endif
	}
	bool heavy_runtime::has_so_sdl2() const noexcept
	{
#ifdef VI_SDL2
		return true;
#else
		return false;
#endif
	}
	bool heavy_runtime::has_so_glew() const noexcept
	{
#ifdef VI_GLEW
		return true;
#else
		return false;
#endif
	}
	bool heavy_runtime::has_so_spirv() const noexcept
	{
#ifdef VI_SPIRV
		return true;
#else
		return false;
#endif
	}
	bool heavy_runtime::has_so_assimp() const noexcept
	{
#ifdef VI_ASSIMP
		return true;
#else
		return false;
#endif
	}
	bool heavy_runtime::has_so_freetype() const noexcept
	{
#ifdef VI_FREETYPE
		return true;
#else
		return false;
#endif
	}
	bool heavy_runtime::has_md_rmlui() const noexcept
	{
#ifdef VI_RMLUI
		return true;
#else
		return false;
#endif
	}
	bool heavy_runtime::has_md_bullet3() const noexcept
	{
#ifdef VI_BULLET3
		return true;
#else
		return false;
#endif
	}
	bool heavy_runtime::has_md_tinyfiledialogs() const noexcept
	{
#ifdef VI_TINYFILEDIALOGS
		return true;
#else
		return false;
#endif
	}
	bool heavy_runtime::has_md_stb() const noexcept
	{
#ifdef VI_STB
		return true;
#else
		return false;
#endif
	}
	bool heavy_runtime::has_md_vectorclass() const noexcept
	{
#ifdef VI_VECTORCLASS
		return true;
#else
		return false;
#endif
	}
	core::string heavy_runtime::get_details() const noexcept
	{
		core::vector<core::string> features;
		if (has_so_opengl())
			features.push_back("so:opengl");
		if (has_so_openal())
			features.push_back("so:openal");
		if (has_so_openssl())
			features.push_back("so:openssl");
		if (has_so_sdl2())
			features.push_back("so:sdl2");
		if (has_so_glew())
			features.push_back("so:glew");
		if (has_so_spirv())
			features.push_back("so:spirv");
		if (has_so_zlib())
			features.push_back("so:zlib");
		if (has_so_assimp())
			features.push_back("so:assimp");
		if (has_so_mongoc())
			features.push_back("so:mongoc");
		if (has_so_postgresql())
			features.push_back("so:pq");
		if (has_so_sqlite())
			features.push_back("so:sqlite");
		if (has_so_freetype())
			features.push_back("so:freetype");
		if (has_md_angelscript())
			features.push_back("md:angelscript");
		if (has_md_backwardcpp())
			features.push_back("md:backward-cpp");
		if (has_md_rmlui())
			features.push_back("md:rmlui");
		if (has_md_bullet3())
			features.push_back("md:bullet3");
		if (has_md_tinyfiledialogs())
			features.push_back("md:tinyfiledialogs");
		if (has_md_wepoll())
			features.push_back("md:wepoll");
		if (has_md_stb())
			features.push_back("md:stb");
		if (has_md_pugixml())
			features.push_back("md:pugixml");
		if (has_md_rapidjson())
			features.push_back("md:rapidjson");
		if (has_md_vectorclass())
			features.push_back("md:vectorclass");
		if (has_ft_fcontext())
			features.push_back("ft:fcontext");
		if (has_ft_allocator())
			features.push_back("ft:allocator");
		if (has_ft_pessimistic())
			features.push_back("ft:pessimistic");
		if (has_ft_bindings())
			features.push_back("ft:bindings");
		if (has_ft_shaders())
			features.push_back("ft:shaders");

		core::string_stream result;
		result << "library: " << major_version << "." << minor_version << "." << patch_version << "." << build_version << " / " << version << "\n";
		result << "  platform: " << get_platform() << " / " << get_build() << "\n";
		result << "  compiler: " << get_compiler() << "\n";
		result << "configuration:" << "\n";

		for (size_t i = 0; i < features.size(); i++)
			result << "  " << features[i] << "\n";

		return result.str();
	}
	heavy_runtime* heavy_runtime::get() noexcept
	{
		return (heavy_runtime*)runtime::get();
	}
}
