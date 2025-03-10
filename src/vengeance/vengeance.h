#ifndef VENGEANCE_H
#define VENGEANCE_H
#include <vitex/vitex.h>

namespace vitex
{
	enum
	{
		use_platform = 1 << 4,
		use_audio = 1 << 5,
		use_graphics = 1 << 6
	};

	class heavy_runtime final : public runtime
	{
	public:
		heavy_runtime(size_t modules = use_networking | use_cryptography | use_providers | use_locale | use_platform | use_audio | use_graphics, core::global_allocator* allocator = nullptr) noexcept;
		virtual ~heavy_runtime() noexcept override;
		bool has_ft_shaders() const noexcept;
		bool has_so_opengl() const noexcept;
		bool has_so_openal() const noexcept;
		bool has_so_sdl2() const noexcept;
		bool has_so_glew() const noexcept;
		bool has_so_spirv() const noexcept;
		bool has_so_assimp() const noexcept;
		bool has_so_freetype() const noexcept;
		bool has_md_rmlui() const noexcept;
		bool has_md_bullet3() const noexcept;
		bool has_md_tinyfiledialogs() const noexcept;
		bool has_md_stb() const noexcept;
		bool has_md_vectorclass() const noexcept;
		core::string get_details() const noexcept override;

	public:
		static bool initialize_platform() noexcept;
		static bool initialize_graphics() noexcept;
		static bool initialize_audio() noexcept;
		static void cleanup_instances() noexcept;
		static void cleanup_platform() noexcept;
		static void cleanup_importer() noexcept;
		static void cleanup_scripting() noexcept;
		static heavy_runtime* get() noexcept;
	};
}
#endif
