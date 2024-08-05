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

namespace Vitex
{
	HeavyRuntime::HeavyRuntime(size_t Modules, Core::GlobalAllocator* Allocator) noexcept : Runtime(Modules, Allocator)
	{
		if (Modes & LOAD_PLATFORM)
			InitializePlatform();

		if (Modes & LOAD_GRAPHICS)
			InitializeGraphics();

		if (Modes & LOAD_AUDIO)
			InitializeAudio();
	}
	HeavyRuntime::~HeavyRuntime() noexcept
	{
		Core::ErrorHandling::SetFlag(Core::LogOption::Async, false);
		CleanupInstances();
		if (Modes & LOAD_PLATFORM)
			CleanupPlatform();
		CleanupImporter();
		CleanupScripting();
	}
	bool HeavyRuntime::InitializePlatform() noexcept
	{
#ifdef VI_SDL2
		SDL_SetMainReady();
		int Code = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER | SDL_INIT_HAPTIC);
		VI_PANIC(Code == 0, "SDL2 initialization failed code:%i", Code);
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

		const char* Platform = SDL_GetPlatform();
		if (!strcmp(Platform, "iOS") || !strcmp(Platform, "Android") || !strcmp(Platform, "Unknown"))
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
		else
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

		VI_TRACE("[lib] initialize sdl2 library");
		return true;
#else
		return false;
#endif
	}
	bool HeavyRuntime::InitializeGraphics() noexcept
	{
#ifdef VI_GLEW
		glewExperimental = true;
		VI_TRACE("[lib] initialize graphics library");
		return true;
#else
		return false;
#endif
	}
	bool HeavyRuntime::InitializeAudio() noexcept
	{
		VI_TRACE("[lib] initialize audio library");
		auto Status = Audio::AudioContext::Initialize();
		Status.Report("audio initialization error");
		return !!Status;
	}
	void HeavyRuntime::CleanupPlatform() noexcept
	{
#ifdef VI_SDL2
		SDL_Quit();
		VI_TRACE("[lib] free sdl2 library");
#endif
	}
	void HeavyRuntime::CleanupImporter() noexcept
	{
#ifdef VI_ASSIMP
		Assimp::DefaultLogger::kill();
		VI_TRACE("[lib] free importer library");
#endif
	}
	void HeavyRuntime::CleanupScripting() noexcept
	{
		Scripting::Bindings::HeavyRegistry().Cleanup();
		VI_TRACE("[lib] free heavy bindings registry");
	}
	bool HeavyRuntime::HasFtShaders() const noexcept
	{
		return HasSoSpirv();
	}
	bool HeavyRuntime::HasSoOpenGL() const noexcept
	{
#ifdef VI_OPENGL
		return true;
#else
		return false;
#endif
	}
	bool HeavyRuntime::HasSoOpenAL() const noexcept
	{
#ifdef VI_OPENAL
		return true;
#else
		return false;
#endif
	}
	bool HeavyRuntime::HasSoSDL2() const noexcept
	{
#ifdef VI_SDL2
		return true;
#else
		return false;
#endif
	}
	bool HeavyRuntime::HasSoGLEW() const noexcept
	{
#ifdef VI_GLEW
		return true;
#else
		return false;
#endif
	}
	bool HeavyRuntime::HasSoSpirv() const noexcept
	{
#ifdef VI_SPIRV
		return true;
#else
		return false;
#endif
	}
	bool HeavyRuntime::HasSoAssimp() const noexcept
	{
#ifdef VI_ASSIMP
		return true;
#else
		return false;
#endif
	}
	bool HeavyRuntime::HasSoFreeType() const noexcept
	{
#ifdef VI_FREETYPE
		return true;
#else
		return false;
#endif
	}
	bool HeavyRuntime::HasMdRmlUI() const noexcept
	{
#ifdef VI_RMLUI
		return true;
#else
		return false;
#endif
	}
	bool HeavyRuntime::HasMdBullet3() const noexcept
	{
#ifdef VI_BULLET3
		return true;
#else
		return false;
#endif
	}
	bool HeavyRuntime::HasMdTinyFileDialogs() const noexcept
	{
#ifdef VI_TINYFILEDIALOGS
		return true;
#else
		return false;
#endif
	}
	bool HeavyRuntime::HasMdStb() const noexcept
	{
#ifdef VI_STB
		return true;
#else
		return false;
#endif
	}
	bool HeavyRuntime::HasMdVectorclass() const noexcept
	{
#ifdef VI_VECTORCLASS
		return true;
#else
		return false;
#endif
	}
	Core::String HeavyRuntime::GetDetails() const noexcept
	{
		Core::Vector<Core::String> Features;
		if (HasSoOpenGL())
			Features.push_back("so:opengl");
		if (HasSoOpenAL())
			Features.push_back("so:openal");
		if (HasSoOpenSSL())
			Features.push_back("so:openssl");
		if (HasSoSDL2())
			Features.push_back("so:sdl2");
		if (HasSoGLEW())
			Features.push_back("so:glew");
		if (HasSoSpirv())
			Features.push_back("so:spirv");
		if (HasSoZLib())
			Features.push_back("so:zlib");
		if (HasSoAssimp())
			Features.push_back("so:assimp");
		if (HasSoMongoc())
			Features.push_back("so:mongoc");
		if (HasSoPostgreSQL())
			Features.push_back("so:pq");
		if (HasSoSQLite())
			Features.push_back("so:sqlite");
		if (HasSoFreeType())
			Features.push_back("so:freetype");
		if (HasMdAngelScript())
			Features.push_back("md:angelscript");
		if (HasMdBackwardCpp())
			Features.push_back("md:backward-cpp");
		if (HasMdRmlUI())
			Features.push_back("md:rmlui");
		if (HasMdBullet3())
			Features.push_back("md:bullet3");
		if (HasMdTinyFileDialogs())
			Features.push_back("md:tinyfiledialogs");
		if (HasMdWepoll())
			Features.push_back("md:wepoll");
		if (HasMdStb())
			Features.push_back("md:stb");
		if (HasMdPugiXml())
			Features.push_back("md:pugixml");
		if (HasMdRapidJson())
			Features.push_back("md:rapidjson");
		if (HasMdVectorclass())
			Features.push_back("md:vectorclass");
		if (HasFtFContext())
			Features.push_back("ft:fcontext");
		if (HasFtAllocator())
			Features.push_back("ft:allocator");
		if (HasFtPessimistic())
			Features.push_back("ft:pessimistic");
		if (HasFtBindings())
			Features.push_back("ft:bindings");
		if (HasFtShaders())
			Features.push_back("ft:shaders");

		Core::StringStream Result;
		Result << "library: " << MAJOR_VERSION << "." << MINOR_VERSION << "." << PATCH_VERSION << "." << BUILD_VERSION << " / " << VERSION << "\n";
		Result << "  platform: " << GetPlatform() << " / " << GetBuild() << "\n";
		Result << "  compiler: " << GetCompiler() << "\n";
        Result << "configuration:" << "\n";
        
		for (size_t i = 0; i < Features.size(); i++)
			Result << "  " << Features[i] << "\n";

		return Result.str();
	}
	HeavyRuntime* HeavyRuntime::Get() noexcept
	{
		return (HeavyRuntime*)Runtime::Get();
	}
}
