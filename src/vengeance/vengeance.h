#ifndef VENGEANCE_H
#define VENGEANCE_H
#include <vitex/vitex.h>

namespace Vitex
{
	enum
	{
		LOAD_PLATFORM = 1 << 4,
		LOAD_AUDIO = 1 << 5,
		LOAD_GRAPHICS = 1 << 6
	};

	class VI_OUT_TS HeavyRuntime final : public Runtime
	{
	public:
		HeavyRuntime(size_t Modules = LOAD_NETWORKING | LOAD_CRYPTOGRAPHY | LOAD_PROVIDERS | LOAD_LOCALE | LOAD_PLATFORM | LOAD_AUDIO | LOAD_GRAPHICS, Core::GlobalAllocator* Allocator = nullptr) noexcept;
		virtual ~HeavyRuntime() noexcept override;
		bool HasFtShaders() const noexcept;
		bool HasSoOpenGL() const noexcept;
		bool HasSoOpenAL() const noexcept;
		bool HasSoSDL2() const noexcept;
		bool HasSoGLEW() const noexcept;
		bool HasSoSpirv() const noexcept;
		bool HasSoAssimp() const noexcept;
		bool HasSoFreeType() const noexcept;
		bool HasMdRmlUI() const noexcept;
		bool HasMdBullet3() const noexcept;
		bool HasMdTinyFileDialogs() const noexcept;
		bool HasMdStb() const noexcept;
		bool HasMdVectorclass() const noexcept;
		Core::String GetDetails() const noexcept override;

	public:
		static bool InitializePlatform() noexcept;
		static bool InitializeGraphics() noexcept;
		static bool InitializeAudio() noexcept;
		static void CleanupInstances() noexcept;
		static void CleanupPlatform() noexcept;
		static void CleanupImporter() noexcept;
		static void CleanupScripting() noexcept;
		static HeavyRuntime* Get() noexcept;
	};
}
#endif
