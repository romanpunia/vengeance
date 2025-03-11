#include "graphics.h"
#include "graphics/d3d11.h"
#include "graphics/ogl.h"
#include "graphics/shaders/bundle.hpp"
#include <sstream>
#ifdef VI_TINYFILEDIALOGS
#include <tinyfiledialogs.h>
#endif
#ifdef VI_SPIRV
#include <glslang/Public/ShaderLang.h>
#include <spirv_cross/spirv_glsl.hpp>
#include <spirv_cross/spirv_hlsl.hpp>
#include <spirv_cross/spirv_msl.hpp>
#include <SPIRV/GlslangToSpv.h>
#endif
#ifdef VI_SDL2
#include "internal/sdl2.hpp"
#endif
#ifdef VI_MICROSOFT
#include <VersionHelpers.h>
#include <dwmapi.h>
#endif

namespace
{
#ifdef VI_SPIRV
	static TBuiltInResource driver_limits = { };

	static void prepare_driver_limits()
	{
		static bool is_ready = false;
		if (is_ready)
			return;

		driver_limits.maxLights = 32;
		driver_limits.maxClipPlanes = 6;
		driver_limits.maxTextureUnits = 32;
		driver_limits.maxTextureCoords = 32;
		driver_limits.maxVertexAttribs = 64;
		driver_limits.maxVertexUniformComponents = 4096;
		driver_limits.maxVaryingFloats = 64;
		driver_limits.maxVertexTextureImageUnits = 32;
		driver_limits.maxCombinedTextureImageUnits = 80;
		driver_limits.maxTextureImageUnits = 32;
		driver_limits.maxFragmentUniformComponents = 4096;
		driver_limits.maxDrawBuffers = 32;
		driver_limits.maxVertexUniformVectors = 128;
		driver_limits.maxVaryingVectors = 8;
		driver_limits.maxFragmentUniformVectors = 16;
		driver_limits.maxVertexOutputVectors = 16;
		driver_limits.maxFragmentInputVectors = 15;
		driver_limits.minProgramTexelOffset = -8;
		driver_limits.maxProgramTexelOffset = 7;
		driver_limits.maxClipDistances = 8;
		driver_limits.maxComputeWorkGroupCountX = 65535;
		driver_limits.maxComputeWorkGroupCountY = 65535;
		driver_limits.maxComputeWorkGroupCountZ = 65535;
		driver_limits.maxComputeWorkGroupSizeX = 1024;
		driver_limits.maxComputeWorkGroupSizeY = 1024;
		driver_limits.maxComputeWorkGroupSizeZ = 64;
		driver_limits.maxComputeUniformComponents = 1024;
		driver_limits.maxComputeTextureImageUnits = 16;
		driver_limits.maxComputeImageUniforms = 8;
		driver_limits.maxComputeAtomicCounters = 8;
		driver_limits.maxComputeAtomicCounterBuffers = 1;
		driver_limits.maxVaryingComponents = 60;
		driver_limits.maxVertexOutputComponents = 64;
		driver_limits.maxGeometryInputComponents = 64;
		driver_limits.maxGeometryOutputComponents = 128;
		driver_limits.maxFragmentInputComponents = 128;
		driver_limits.maxImageUnits = 8;
		driver_limits.maxCombinedImageUnitsAndFragmentOutputs = 8;
		driver_limits.maxCombinedShaderOutputResources = 8;
		driver_limits.maxImageSamples = 0;
		driver_limits.maxVertexImageUniforms = 0;
		driver_limits.maxTessControlImageUniforms = 0;
		driver_limits.maxTessEvaluationImageUniforms = 0;
		driver_limits.maxGeometryImageUniforms = 0;
		driver_limits.maxFragmentImageUniforms = 8;
		driver_limits.maxCombinedImageUniforms = 8;
		driver_limits.maxGeometryTextureImageUnits = 16;
		driver_limits.maxGeometryOutputVertices = 256;
		driver_limits.maxGeometryTotalOutputComponents = 1024;
		driver_limits.maxGeometryUniformComponents = 1024;
		driver_limits.maxGeometryVaryingComponents = 64;
		driver_limits.maxTessControlInputComponents = 128;
		driver_limits.maxTessControlOutputComponents = 128;
		driver_limits.maxTessControlTextureImageUnits = 16;
		driver_limits.maxTessControlUniformComponents = 1024;
		driver_limits.maxTessControlTotalOutputComponents = 4096;
		driver_limits.maxTessEvaluationInputComponents = 128;
		driver_limits.maxTessEvaluationOutputComponents = 128;
		driver_limits.maxTessEvaluationTextureImageUnits = 16;
		driver_limits.maxTessEvaluationUniformComponents = 1024;
		driver_limits.maxTessPatchComponents = 120;
		driver_limits.maxPatchVertices = 32;
		driver_limits.maxTessGenLevel = 64;
		driver_limits.maxViewports = 16;
		driver_limits.maxVertexAtomicCounters = 0;
		driver_limits.maxTessControlAtomicCounters = 0;
		driver_limits.maxTessEvaluationAtomicCounters = 0;
		driver_limits.maxGeometryAtomicCounters = 0;
		driver_limits.maxFragmentAtomicCounters = 8;
		driver_limits.maxCombinedAtomicCounters = 8;
		driver_limits.maxAtomicCounterBindings = 1;
		driver_limits.maxVertexAtomicCounterBuffers = 0;
		driver_limits.maxTessControlAtomicCounterBuffers = 0;
		driver_limits.maxTessEvaluationAtomicCounterBuffers = 0;
		driver_limits.maxGeometryAtomicCounterBuffers = 0;
		driver_limits.maxFragmentAtomicCounterBuffers = 1;
		driver_limits.maxCombinedAtomicCounterBuffers = 1;
		driver_limits.maxAtomicCounterBufferSize = 16384;
		driver_limits.maxTransformFeedbackBuffers = 4;
		driver_limits.maxTransformFeedbackInterleavedComponents = 64;
		driver_limits.maxCullDistances = 8;
		driver_limits.maxCombinedClipAndCullDistances = 8;
		driver_limits.maxSamples = 4;
		driver_limits.maxMeshOutputVerticesNV = 256;
		driver_limits.maxMeshOutputPrimitivesNV = 512;
		driver_limits.maxMeshWorkGroupSizeX_NV = 32;
		driver_limits.maxMeshWorkGroupSizeY_NV = 1;
		driver_limits.maxMeshWorkGroupSizeZ_NV = 1;
		driver_limits.maxTaskWorkGroupSizeX_NV = 32;
		driver_limits.maxTaskWorkGroupSizeY_NV = 1;
		driver_limits.maxTaskWorkGroupSizeZ_NV = 1;
		driver_limits.maxMeshViewCountNV = 4;
		driver_limits.limits.nonInductiveForLoops = 1;
		driver_limits.limits.whileLoops = 1;
		driver_limits.limits.doWhileLoops = 1;
		driver_limits.limits.generalUniformIndexing = 1;
		driver_limits.limits.generalAttributeMatrixVectorIndexing = 1;
		driver_limits.limits.generalVaryingIndexing = 1;
		driver_limits.limits.generalSamplerIndexing = 1;
		driver_limits.limits.generalVariableIndexing = 1;
		driver_limits.limits.generalConstantMatrixVectorIndexing = 1;
		is_ready = true;
	}
	static void prepare_combined_samplers(spirv_cross::Compiler* compiler)
	{
		compiler->build_dummy_sampler_for_combined_images();
		compiler->build_combined_image_samplers();
		for (auto& resource : compiler->get_combined_image_samplers())
		{
			uint32_t binding_id = compiler->get_decoration(resource.image_id, spv::DecorationBinding);
			compiler->set_decoration(resource.combined_id, spv::DecorationBinding, binding_id);
			compiler->set_name(resource.combined_id, compiler->get_name(resource.image_id));
		}
	}
#endif
	static vitex::graphics::render_backend get_supported_backend(vitex::graphics::render_backend type)
	{
		if (type != vitex::graphics::render_backend::automatic)
			return type;
#ifdef VI_MICROSOFT
		return vitex::graphics::render_backend::d3d11;
#endif
#ifdef VI_GL
		return vitex::graphics::render_backend::ogl;
#endif
		return vitex::graphics::render_backend::none;
	}
}

namespace vitex
{
	namespace graphics
	{
		alert::alert(activity* from) noexcept : view(alert_type::none), base(from), waiting(false)
		{
		}
		void alert::setup(alert_type type, const std::string_view& title, const std::string_view& text)
		{
			VI_ASSERT(type != alert_type::none, "alert type should not be none");
			view = type;
			name = title;
			data = text;
			buttons.clear();
		}
		void alert::button(alert_confirm confirm, const std::string_view& text, int id)
		{
			VI_ASSERT(view != alert_type::none, "alert type should not be none");
			VI_ASSERT(buttons.size() < 16, "there must be less than 16 buttons in alert");

			for (auto& item : buttons)
			{
				if (item.id == id)
					return;
			}

			element button;
			button.name = text;
			button.id = id;
			button.flags = (int)confirm;

			buttons.push_back(button);
		}
		void alert::result(std::function<void(int)>&& callback)
		{
			VI_ASSERT(view != alert_type::none, "alert type should not be none");
			done = std::move(callback);
			waiting = true;
		}
		void alert::dispatch()
		{
#ifdef VI_SDL2
			if (view == alert_type::none || !waiting)
				return;

			SDL_MessageBoxButtonData views[16];
			for (size_t i = 0; i < buttons.size(); i++)
			{
				SDL_MessageBoxButtonData* to = views + i;
				auto from = buttons.begin() + i;
				to->text = from->name.c_str();
				to->buttonid = from->id;
				to->flags = from->flags;
			}

			SDL_MessageBoxData alert_data;
			alert_data.title = name.c_str();
			alert_data.message = data.c_str();
			alert_data.flags = (SDL_MessageBoxFlags)view;
			alert_data.numbuttons = (int)buttons.size();
			alert_data.buttons = views;
			alert_data.window = base->get_handle();

			int id = 0;
			view = alert_type::none;
			waiting = false;
			int rd = SDL_ShowMessageBox(&alert_data, &id);

			if (done)
				done(rd >= 0 ? id : -1);
#endif
		}

		void event_consumers::push(activity* value)
		{
			VI_ASSERT(value != nullptr, "activity should be set");
			consumers[value->get_id()] = value;
		}
		void event_consumers::pop(activity* value)
		{
			VI_ASSERT(value != nullptr, "activity should be set");
			auto it = consumers.find(value->get_id());
			if (it != consumers.end())
				consumers.erase(it);
		}
		activity* event_consumers::find(uint32_t id) const
		{
			auto it = consumers.find(id);
			return it != consumers.end() ? it->second : nullptr;
		}

		key_map::key_map() noexcept : key(key_code::none), mod(key_mod::none), normal(false)
		{
		}
		key_map::key_map(const key_code& value) noexcept : key(value), mod(key_mod::none), normal(false)
		{
		}
		key_map::key_map(const key_mod& value) noexcept : key(key_code::none), mod(value), normal(false)
		{
		}
		key_map::key_map(const key_code& value, const key_mod& control) noexcept : key(value), mod(control), normal(false)
		{
		}

		graphics_exception::graphics_exception(core::string&& new_message) : error_code(0)
		{
			error_message = std::move(new_message);
		}
		graphics_exception::graphics_exception(int new_error_code, core::string&& new_message) : error_code(new_error_code)
		{
			error_message = std::move(new_message);
			if (error_code != 0)
				error_message += " (error = " + core::to_string(error_code) + ")";
		}
		const char* graphics_exception::type() const noexcept
		{
			return "graphics_error";
		}
		int graphics_exception::code() const noexcept
		{
			return error_code;
		}

		video_exception::video_exception()
		{
#ifdef VI_SDL2
			const char* error_text = SDL_GetError();
			if (error_text != nullptr)
			{
				error_message = error_text;
				SDL_ClearError();
			}
			else
				error_message = "internal video error occurred";
#else
			error_message = "video systems are not supported";
#endif
		}
		video_exception::video_exception(graphics_exception&& other)
		{
			error_message = std::move(other.message());
		}
		const char* video_exception::type() const noexcept
		{
			return "video_error";
		}

		surface::surface() noexcept : handle(nullptr)
		{
		}
		surface::surface(SDL_Surface* from) noexcept : handle(from)
		{
		}
		surface::~surface() noexcept
		{
#ifdef VI_SDL2
			if (handle != nullptr)
			{
				SDL_FreeSurface(handle);
				handle = nullptr;
			}
#endif
		}
		void surface::set_handle(SDL_Surface* from)
		{
#ifdef VI_SDL2
			if (handle != nullptr)
				SDL_FreeSurface(handle);
#endif
			handle = from;
		}
		void surface::lock()
		{
#ifdef VI_SDL2
			SDL_LockSurface(handle);
#endif
		}
		void surface::unlock()
		{
#ifdef VI_SDL2
			SDL_UnlockSurface(handle);
#endif
		}
		int surface::get_width() const
		{
#ifdef VI_SDL2
			VI_ASSERT(handle != nullptr, "handle should be set");
			return handle->w;
#else
			return -1;
#endif
		}
		int surface::get_height() const
		{
#ifdef VI_SDL2
			VI_ASSERT(handle != nullptr, "handle should be set");
			return handle->h;
#else
			return -1;
#endif
		}
		int surface::get_pitch() const
		{
#ifdef VI_SDL2
			VI_ASSERT(handle != nullptr, "handle should be set");
			return handle->pitch;
#else
			return -1;
#endif
		}
		void* surface::get_pixels() const
		{
#ifdef VI_SDL2
			VI_ASSERT(handle != nullptr, "handle should be set");
			return handle->pixels;
#else
			return nullptr;
#endif
		}
		void* surface::get_resource() const
		{
			return (void*)handle;
		}

		depth_stencil_state::depth_stencil_state(const desc& i) noexcept : state(i)
		{
		}
		depth_stencil_state::~depth_stencil_state() noexcept
		{
		}
		depth_stencil_state::desc depth_stencil_state::get_state() const
		{
			return state;
		}

		rasterizer_state::rasterizer_state(const desc& i) noexcept : state(i)
		{
		}
		rasterizer_state::~rasterizer_state() noexcept
		{
		}
		rasterizer_state::desc rasterizer_state::get_state() const
		{
			return state;
		}

		blend_state::blend_state(const desc& i) noexcept : state(i)
		{
		}
		blend_state::~blend_state() noexcept
		{
		}
		blend_state::desc blend_state::get_state() const
		{
			return state;
		}

		sampler_state::sampler_state(const desc& i) noexcept : state(i)
		{
		}
		sampler_state::~sampler_state() noexcept
		{
		}
		sampler_state::desc sampler_state::get_state() const
		{
			return state;
		}

		input_layout::input_layout(const desc& i) noexcept : layout(i.attributes)
		{
		}
		input_layout::~input_layout() noexcept
		{
		}
		const core::vector<input_layout::attribute>& input_layout::get_attributes() const
		{
			return layout;
		}

		shader::shader(const desc& i) noexcept
		{
		}

		element_buffer::element_buffer(const desc& i) noexcept
		{
			elements = i.element_count;
			stride = i.element_width;
		}
		size_t element_buffer::get_elements() const
		{
			return elements;
		}
		size_t element_buffer::get_stride() const
		{
			return stride;
		}

		mesh_buffer::mesh_buffer(const desc& i) noexcept : vertex_buffer(nullptr), index_buffer(nullptr)
		{
		}
		mesh_buffer::~mesh_buffer() noexcept
		{
			core::memory::release(vertex_buffer);
			core::memory::release(index_buffer);
		}
		element_buffer* mesh_buffer::get_vertex_buffer() const
		{
			return vertex_buffer;
		}
		element_buffer* mesh_buffer::get_index_buffer() const
		{
			return index_buffer;
		}

		skin_mesh_buffer::skin_mesh_buffer(const desc& i) noexcept : vertex_buffer(nullptr), index_buffer(nullptr)
		{
		}
		skin_mesh_buffer::~skin_mesh_buffer() noexcept
		{
			core::memory::release(vertex_buffer);
			core::memory::release(index_buffer);
		}
		element_buffer* skin_mesh_buffer::get_vertex_buffer() const
		{
			return vertex_buffer;
		}
		element_buffer* skin_mesh_buffer::get_index_buffer() const
		{
			return index_buffer;
		}

		instance_buffer::instance_buffer(const desc& i) noexcept : elements(nullptr), device(i.device), sync(false)
		{
			element_limit = i.element_limit;
			element_width = i.element_width;

			if (element_limit < 1)
				element_limit = 1;

			array.reserve(element_limit);
		}
		instance_buffer::~instance_buffer() noexcept
		{
			core::memory::release(elements);
		}
		core::vector<trigonometry::element_vertex>& instance_buffer::get_array()
		{
			return array;
		}
		element_buffer* instance_buffer::get_elements() const
		{
			return elements;
		}
		graphics_device* instance_buffer::get_device() const
		{
			return device;
		}
		size_t instance_buffer::get_element_limit() const
		{
			return element_limit;
		}

		texture_2d::texture_2d() noexcept
		{
			width = window_size;
			height = window_size;
			mip_levels = 1;
			format_mode = format::unknown;
			usage = resource_usage::defaults;
			access_flags = cpu_access::none;
			binding = resource_bind::shader_input;
		}
		texture_2d::texture_2d(const desc& i) noexcept
		{
			width = i.width;
			height = i.height;
			mip_levels = i.mip_levels;
			format_mode = i.format_mode;
			usage = i.usage;
			access_flags = i.access_flags;
			binding = i.bind_flags;
		}
		cpu_access texture_2d::get_access_flags() const
		{
			return access_flags;
		}
		format texture_2d::get_format_mode() const
		{
			return format_mode;
		}
		resource_usage texture_2d::get_usage() const
		{
			return usage;
		}
		resource_bind texture_2d::get_binding() const
		{
			return binding;
		}
		uint32_t texture_2d::get_width() const
		{
			return width;
		}
		uint32_t texture_2d::get_height() const
		{
			return height;
		}
		uint32_t texture_2d::get_mip_levels() const
		{
			return mip_levels;
		}

		texture_3d::texture_3d()
		{
			width = window_size;
			height = window_size;
			depth = 1;
			mip_levels = 1;
			format_mode = format::unknown;
			usage = resource_usage::defaults;
			access_flags = cpu_access::none;
			binding = resource_bind::shader_input;
		}
		cpu_access texture_3d::get_access_flags() const
		{
			return access_flags;
		}
		format texture_3d::get_format_mode() const
		{
			return format_mode;
		}
		resource_usage texture_3d::get_usage() const
		{
			return usage;
		}
		resource_bind texture_3d::get_binding() const
		{
			return binding;
		}
		uint32_t texture_3d::get_width() const
		{
			return width;
		}
		uint32_t texture_3d::get_height() const
		{
			return height;
		}
		uint32_t texture_3d::get_depth() const
		{
			return depth;
		}
		uint32_t texture_3d::get_mip_levels() const
		{
			return mip_levels;
		}

		texture_cube::texture_cube() noexcept
		{
			width = window_size;
			height = window_size;
			mip_levels = 1;
			format_mode = format::unknown;
			usage = resource_usage::defaults;
			access_flags = cpu_access::none;
			binding = resource_bind::shader_input;
		}
		texture_cube::texture_cube(const desc& i) noexcept
		{
			width = i.width;
			height = i.height;
			mip_levels = i.mip_levels;
			format_mode = i.format_mode;
			usage = i.usage;
			access_flags = i.access_flags;
			binding = i.bind_flags;
		}
		cpu_access texture_cube::get_access_flags() const
		{
			return access_flags;
		}
		format texture_cube::get_format_mode() const
		{
			return format_mode;
		}
		resource_usage texture_cube::get_usage() const
		{
			return usage;
		}
		resource_bind texture_cube::get_binding() const
		{
			return binding;
		}
		uint32_t texture_cube::get_width() const
		{
			return width;
		}
		uint32_t texture_cube::get_height() const
		{
			return height;
		}
		uint32_t texture_cube::get_mip_levels() const
		{
			return mip_levels;
		}

		depth_target_2d::depth_target_2d(const desc& i) noexcept : resource(nullptr), viewarea({ 0, 0, window_size, window_size, 0, 1 })
		{
		}
		depth_target_2d::~depth_target_2d() noexcept
		{
			core::memory::release(resource);
		}
		texture_2d* depth_target_2d::get_target()
		{
			return resource;
		}
		const viewport& depth_target_2d::get_viewport() const
		{
			return viewarea;
		}

		depth_target_cube::depth_target_cube(const desc& i) noexcept : resource(nullptr), viewarea({ 0, 0, window_size, window_size, 0, 1 })
		{
		}
		depth_target_cube::~depth_target_cube() noexcept
		{
			core::memory::release(resource);
		}
		texture_cube* depth_target_cube::get_target()
		{
			return resource;
		}
		const viewport& depth_target_cube::get_viewport() const
		{
			return viewarea;
		}

		render_target::render_target() noexcept : depth_stencil(nullptr), viewarea({ 0, 0, window_size, window_size, 0, 1 })
		{
		}
		render_target::~render_target() noexcept
		{
			core::memory::release(depth_stencil);
		}
		texture_2d* render_target::get_depth_stencil()
		{
			return depth_stencil;
		}
		const viewport& render_target::get_viewport() const
		{
			return viewarea;
		}

		render_target_2d::render_target_2d(const desc& i) noexcept : render_target(), resource(nullptr)
		{
		}
		render_target_2d::~render_target_2d() noexcept
		{
			core::memory::release(resource);
		}
		uint32_t render_target_2d::get_target_count() const
		{
			return 1;
		}
		texture_2d* render_target_2d::get_target_2d(uint32_t index)
		{
			return get_target();
		}
		texture_cube* render_target_2d::get_target_cube(uint32_t index)
		{
			return nullptr;
		}
		texture_2d* render_target_2d::get_target()
		{
			return resource;
		}

		multi_render_target_2d::multi_render_target_2d(const desc& i) noexcept : render_target()
		{
			VI_ASSERT((uint32_t)i.target <= 8, "target should be less than 9");
			target = i.target;

			for (uint32_t i = 0; i < 8; i++)
				resource[i] = nullptr;
		}
		multi_render_target_2d::~multi_render_target_2d() noexcept
		{
			VI_ASSERT((uint32_t)target <= 8, "target should be less than 9");
			for (uint32_t i = 0; i < (uint32_t)target; i++)
				core::memory::release(resource[i]);
		}
		uint32_t multi_render_target_2d::get_target_count() const
		{
			return (uint32_t)target;
		}
		texture_2d* multi_render_target_2d::get_target_2d(uint32_t index)
		{
			return get_target(index);
		}
		texture_cube* multi_render_target_2d::get_target_cube(uint32_t index)
		{
			return nullptr;
		}
		texture_2d* multi_render_target_2d::get_target(uint32_t slot)
		{
			VI_ASSERT(slot < (uint32_t)target, "slot should be less than targets count");
			return resource[slot];
		}

		render_target_cube::render_target_cube(const desc& i) noexcept : render_target(), resource(nullptr)
		{
		}
		render_target_cube::~render_target_cube() noexcept
		{
			core::memory::release(resource);
		}
		uint32_t render_target_cube::get_target_count() const
		{
			return 1;
		}
		texture_2d* render_target_cube::get_target_2d(uint32_t index)
		{
			return nullptr;
		}
		texture_cube* render_target_cube::get_target_cube(uint32_t index)
		{
			return get_target();
		}
		texture_cube* render_target_cube::get_target()
		{
			return resource;
		}

		multi_render_target_cube::multi_render_target_cube(const desc& i) noexcept : render_target()
		{
			VI_ASSERT((uint32_t)i.target <= 8, "target should be less than 9");
			target = i.target;

			for (uint32_t i = 0; i < 8; i++)
				resource[i] = nullptr;
		}
		multi_render_target_cube::~multi_render_target_cube() noexcept
		{
			VI_ASSERT((uint32_t)target <= 8, "target should be less than 9");
			for (uint32_t i = 0; i < (uint32_t)target; i++)
				core::memory::release(resource[i]);
		}
		uint32_t multi_render_target_cube::get_target_count() const
		{
			return (uint32_t)target;
		}
		texture_2d* multi_render_target_cube::get_target_2d(uint32_t index)
		{
			return nullptr;
		}
		texture_cube* multi_render_target_cube::get_target_cube(uint32_t index)
		{
			return get_target(index);
		}
		texture_cube* multi_render_target_cube::get_target(uint32_t slot)
		{
			VI_ASSERT(slot < (uint32_t)target, "slot should be less than targets count");
			return resource[slot];
		}

		cubemap::cubemap(const desc& i) noexcept : dest(nullptr), meta(i)
		{
		}
		bool cubemap::is_valid() const
		{
			return meta.source != nullptr;
		}

		query::query() noexcept
		{
		}

		graphics_device::graphics_device(const desc& i) noexcept : primitives(primitive_topology::triangle_list), shader_gen(shader_model::invalid), view_resource(nullptr), present_flags(i.presentation_flags), compile_flags(i.compilation_flags), vsync_mode(i.vsync_mode), max_elements(1), backend(i.backend), shader_cache(i.shader_cache), debug(i.debug)
		{
			render_thread = std::this_thread::get_id();
			if (!i.cache_directory.empty())
			{
				auto directory = core::os::path::resolve_directory(i.cache_directory.c_str());
				if (directory && core::os::directory::is_exists(directory->c_str()))
					caches = *directory;
			}

			if (!i.window)
			{
				activity::desc init;
				init.title = "activity.virtual.hidden";
				init.hidden = true;
				init.borderless = true;
				init.width = 128;
				init.height = 128;

				virtual_window = new activity(init);
			}

			create_sections();
		}
		graphics_device::~graphics_device() noexcept
		{
			release_proxy();
			for (auto it = sections.begin(); it != sections.end(); it++)
				core::memory::deinit(it->second);

			core::memory::release(virtual_window);
			sections.clear();
		}
		void graphics_device::set_vertex_buffer(element_buffer* resource)
		{
			if (resource != nullptr)
				set_vertex_buffers(&resource, 1);
			else
				set_vertex_buffers(nullptr, 0);
		}
		void graphics_device::set_shader_cache(bool enabled)
		{
			shader_cache = enabled;
		}
		void graphics_device::set_vsync_mode(vsync mode)
		{
			vsync_mode = mode;
		}
		void graphics_device::lockup(render_thread_callback&& callback)
		{
			VI_ASSERT(callback != nullptr, "callback should be set");
			core::umutex<std::recursive_mutex> unique(exclusive);
			callback(this);
		}
		void graphics_device::enqueue(render_thread_callback&& callback)
		{
			VI_ASSERT(callback != nullptr, "callback should be set");
			if (render_thread != std::this_thread::get_id())
			{
				core::umutex<std::recursive_mutex> unique(exclusive);
				queue.emplace(std::move(callback));
			}
			else
				callback(this);
		}
		void graphics_device::dispatch_queue()
		{
			render_thread = std::this_thread::get_id();
			if (queue.empty())
				return;

			core::umutex<std::recursive_mutex> unique(exclusive);
			while (!queue.empty())
			{
				queue.front()(this);
				queue.pop();
			}
		}
		void graphics_device::create_states()
		{
			depth_stencil_state::desc depth_stencil;
			depth_stencil.depth_enable = true;
			depth_stencil.depth_write_mask = depth_write::all;
			depth_stencil.depth_function = comparison::less;
			depth_stencil.stencil_enable = true;
			depth_stencil.stencil_read_mask = 0xFF;
			depth_stencil.stencil_write_mask = 0xFF;
			depth_stencil.front_face_stencil_fail_operation = stencil_operation::keep;
			depth_stencil.front_face_stencil_depth_fail_operation = stencil_operation::add;
			depth_stencil.front_face_stencil_pass_operation = stencil_operation::keep;
			depth_stencil.front_face_stencil_function = comparison::always;
			depth_stencil.back_face_stencil_fail_operation = stencil_operation::keep;
			depth_stencil.back_face_stencil_depth_fail_operation = stencil_operation::subtract;
			depth_stencil.back_face_stencil_pass_operation = stencil_operation::keep;
			depth_stencil.back_face_stencil_function = comparison::always;
			depth_stencil_states["drw_srw_lt"] = *create_depth_stencil_state(depth_stencil);

			depth_stencil.depth_write_mask = depth_write::zero;
			depth_stencil.stencil_write_mask = 0x0;
			depth_stencil_states["dro_sro_lt"] = *create_depth_stencil_state(depth_stencil);

			depth_stencil.depth_function = comparison::greater_equal;
			depth_stencil_states["dro_sro_gte"] = *create_depth_stencil_state(depth_stencil);

			depth_stencil.depth_write_mask = depth_write::all;
			depth_stencil.stencil_write_mask = 0xFF;
			depth_stencil_states["dro_srw_gte"] = *create_depth_stencil_state(depth_stencil);

			depth_stencil.depth_enable = false;
			depth_stencil.depth_function = comparison::less;
			depth_stencil.stencil_enable = false;
			depth_stencil_states["doo_soo_lt"] = *create_depth_stencil_state(depth_stencil);

			depth_stencil.depth_enable = true;
			depth_stencil.depth_write_mask = depth_write::zero;
			depth_stencil.stencil_enable = true;
			depth_stencil_states["dro_srw_lt"] = *create_depth_stencil_state(depth_stencil);

			depth_stencil.depth_write_mask = depth_write::all;
			depth_stencil.stencil_enable = false;
			depth_stencil_states["drw_soo_lt"] = *create_depth_stencil_state(depth_stencil);

			depth_stencil.depth_function = comparison::less_equal;
			depth_stencil.depth_write_mask = depth_write::zero;
			depth_stencil.stencil_enable = false;
			depth_stencil_states["dro_soo_lte"] = *create_depth_stencil_state(depth_stencil);

			rasterizer_state::desc rasterizer;
			rasterizer.antialiased_line_enable = false;
			rasterizer.cull_mode = vertex_cull::back;
			rasterizer.depth_bias = 0;
			rasterizer.depth_bias_clamp = 0;
			rasterizer.depth_clip_enable = true;
			rasterizer.fill_mode = surface_fill::solid;
			rasterizer.front_counter_clockwise = false;
			rasterizer.multisample_enable = false;
			rasterizer.scissor_enable = false;
			rasterizer.slope_scaled_depth_bias = 0.0f;
			rasterizer_states["so_cback"] = *create_rasterizer_state(rasterizer);

			rasterizer.cull_mode = vertex_cull::front;
			rasterizer_states["so_cfront"] = *create_rasterizer_state(rasterizer);

			rasterizer.cull_mode = vertex_cull::none;
			rasterizer_states["so_co"] = *create_rasterizer_state(rasterizer);

			rasterizer.scissor_enable = true;
			rasterizer_states["sw_co"] = *create_rasterizer_state(rasterizer);

			rasterizer.cull_mode = vertex_cull::back;
			rasterizer_states["sw_cback"] = *create_rasterizer_state(rasterizer);

			blend_state::desc blend;
			blend.alpha_to_coverage_enable = false;
			blend.independent_blend_enable = false;
			blend.render_target[0].blend_enable = false;
			blend.render_target[0].render_target_write_mask = (uint8_t)color_write_enable::all;
			blend_states["bo_wrgba_one"] = *create_blend_state(blend);

			blend.render_target[0].render_target_write_mask = (uint8_t)(color_write_enable::red | color_write_enable::green | color_write_enable::blue);
			blend_states["bo_wrgbo_one"] = *create_blend_state(blend);

			blend.render_target[0].render_target_write_mask = 0;
			blend_states["bo_woooo_one"] = *create_blend_state(blend);

			blend.render_target[0].blend_enable = true;
			blend.render_target[0].src_blend = blend::one;
			blend.render_target[0].dest_blend = blend::one;
			blend.render_target[0].blend_operation_mode = blend_operation::add;
			blend.render_target[0].src_blend_alpha = blend::one;
			blend.render_target[0].dest_blend_alpha = blend::one;
			blend.render_target[0].blend_operation_alpha = blend_operation::add;
			blend.render_target[0].render_target_write_mask = (uint8_t)color_write_enable::all;
			blend_states["bw_wrgba_one"] = *create_blend_state(blend);

			blend.render_target[0].render_target_write_mask = (uint8_t)(color_write_enable::red | color_write_enable::green | color_write_enable::blue);
			blend_states["bw_wrgbo_one"] = *create_blend_state(blend);

			blend.independent_blend_enable = true;
			for (uint32_t i = 0; i < 8; i++)
			{
				blend.render_target[i].blend_enable = (i != 1 && i != 2);
				blend.render_target[i].src_blend = blend::one;
				blend.render_target[i].dest_blend = blend::one;
				blend.render_target[i].blend_operation_mode = blend_operation::add;
				blend.render_target[i].src_blend_alpha = blend::one;
				blend.render_target[i].dest_blend_alpha = blend::one;
				blend.render_target[i].blend_operation_alpha = blend_operation::add;
				blend.render_target[i].render_target_write_mask = (uint8_t)color_write_enable::all;
			}
			blend_states["bw_wrgba_gbuffer"] = *create_blend_state(blend);

			blend.independent_blend_enable = false;
			blend.render_target[0].blend_enable = true;
			blend.render_target[0].src_blend = blend::source_alpha;
			blend_states["bw_wrgba_alpha"] = *create_blend_state(blend);

			blend.render_target[0].dest_blend = blend::source_alpha_invert;
			blend.render_target[0].src_blend_alpha = blend::source_alpha_invert;
			blend.render_target[0].dest_blend_alpha = blend::zero;
			blend_states["bw_wrgba_source"] = *create_blend_state(blend);

			sampler_state::desc sampler;
			sampler.filter = pixel_filter::anistropic;
			sampler.address_u = texture_address::wrap;
			sampler.address_v = texture_address::wrap;
			sampler.address_w = texture_address::wrap;
			sampler.mip_lod_bias = 0.0f;
			sampler.max_anisotropy = 16;
			sampler.comparison_function = comparison::never;
			sampler.border_color[0] = 0.0f;
			sampler.border_color[1] = 0.0f;
			sampler.border_color[2] = 0.0f;
			sampler.border_color[3] = 0.0f;
			sampler.min_lod = 0.0f;
			sampler.max_lod = std::numeric_limits<float>::max();
			sampler_states["a16_fa_wrap"] = *create_sampler_state(sampler);

			sampler.address_u = texture_address::mirror;
			sampler.address_v = texture_address::mirror;
			sampler.address_w = texture_address::mirror;
			sampler_states["a16_fa_mirror"] = *create_sampler_state(sampler);

			sampler.address_u = texture_address::clamp;
			sampler.address_v = texture_address::clamp;
			sampler.address_w = texture_address::clamp;
			sampler_states["a16_fa_clamp"] = *create_sampler_state(sampler);

			sampler.filter = pixel_filter::min_mag_mip_linear;
			sampler_states["a16_fl_clamp"] = *create_sampler_state(sampler);

			sampler.filter = pixel_filter::min_mag_mip_point;
			sampler.comparison_function = comparison::never;
			sampler_states["a16_fp_clamp"] = *create_sampler_state(sampler);

			sampler.filter = pixel_filter::min_mag_mip_linear;
			sampler.max_anisotropy = 1;
			sampler_states["a1_fl_clamp"] = *create_sampler_state(sampler);

			sampler.filter = pixel_filter::compare_min_mag_mip_linear;
			sampler.comparison_function = comparison::less;
			sampler_states["a1_fl_clamp_cmp_lt"] = *create_sampler_state(sampler);

			sampler.comparison_function = comparison::greater_equal;
			sampler_states["a1_fl_clamp_cmp_gte"] = *create_sampler_state(sampler);

			input_layout::desc layout;
			layout.attributes =
			{
				{ "POSITION", 0, attribute_type::floatf, 3, 0 },
				{ "TEXCOORD", 0, attribute_type::floatf, 2, 3 * sizeof(float) }
			};
			input_layouts["vx_shape"] = *create_input_layout(layout);

			layout.attributes =
			{
				{ "POSITION", 0, attribute_type::floatf, 3, 0 },
				{ "TEXCOORD", 0, attribute_type::floatf, 4, 3 * sizeof(float) },
				{ "TEXCOORD", 1, attribute_type::floatf, 4, 7 * sizeof(float) },
				{ "TEXCOORD", 2, attribute_type::floatf, 3, 11 * sizeof(float) }
			};
			input_layouts["vx_element"] = *create_input_layout(layout);

			layout.attributes =
			{
				{ "POSITION", 0, attribute_type::floatf, 3, 0 },
				{ "TEXCOORD", 0, attribute_type::floatf, 2, 3 * sizeof(float) },
				{ "NORMAL", 0, attribute_type::floatf, 3, 5 * sizeof(float) },
				{ "TANGENT", 0, attribute_type::floatf, 3, 8 * sizeof(float) },
				{ "BINORMAL", 0, attribute_type::floatf, 3, 11 * sizeof(float) }
			};
			input_layouts["vx_base"] = *create_input_layout(layout);

			layout.attributes =
			{
				{ "POSITION", 0, attribute_type::floatf, 3, 0 },
				{ "TEXCOORD", 0, attribute_type::floatf, 2, 3 * sizeof(float) },
				{ "NORMAL", 0, attribute_type::floatf, 3, 5 * sizeof(float) },
				{ "TANGENT", 0, attribute_type::floatf, 3, 8 * sizeof(float) },
				{ "BINORMAL", 0, attribute_type::floatf, 3, 11 * sizeof(float) },
				{ "OB_TRANSFORM", 0, attribute_type::matrix, 16, 0, 1, false },
				{ "OB_WORLD", 0, attribute_type::matrix, 16, sizeof(trigonometry::matrix4x4), 1, false },
				{ "OB_TEXCOORD", 0, attribute_type::floatf, 2, sizeof(trigonometry::matrix4x4) * 2, 1, false },
				{ "OB_MATERIAL", 0, attribute_type::floatf, 4, sizeof(trigonometry::matrix4x4) * 2 + 2 * sizeof(float), 1, false }
			};
			input_layouts["vxi_base"] = *create_input_layout(layout);

			layout.attributes =
			{
				{ "POSITION", 0, attribute_type::floatf, 3, 0 },
				{ "TEXCOORD", 0, attribute_type::floatf, 2, 3 * sizeof(float) },
				{ "NORMAL", 0, attribute_type::floatf, 3, 5 * sizeof(float) },
				{ "TANGENT", 0, attribute_type::floatf, 3, 8 * sizeof(float) },
				{ "BINORMAL", 0, attribute_type::floatf, 3, 11 * sizeof(float) },
				{ "JOINTBIAS", 0, attribute_type::floatf, 4, 14 * sizeof(float) },
				{ "JOINTBIAS", 1, attribute_type::floatf, 4, 18 * sizeof(float) }
			};
			input_layouts["vx_skin"] = *create_input_layout(layout);

			layout.attributes =
			{
				{ "POSITION", 0, attribute_type::floatf, 2, 0 },
				{ "COLOR", 0, attribute_type::ubyte, 4, 2 * sizeof(float) },
				{ "TEXCOORD", 0, attribute_type::floatf, 2, 2 * sizeof(float) + 4 * sizeof(uint8_t) }
			};
			input_layouts["vx_ui"] = *create_input_layout(layout);

			set_depth_stencil_state(get_depth_stencil_state("drw_srw_lt"));
			set_rasterizer_state(get_rasterizer_state("so_cback"));
			set_blend_state(get_blend_state("bo_wrgba_one"));
		}
		void graphics_device::create_sections()
		{
#ifdef HAS_SHADER_BUNDLE
			shader_bundle::foreach(this, [](void* context, const char* name, const uint8_t* buffer, unsigned size)
			{
				graphics_device* base = (graphics_device*)context;
				if (base != nullptr && base->get_backend() != render_backend::none)
					base->add_section(name, core::string((const char*)buffer, size - 1));
			});
#endif
		}
		void graphics_device::release_proxy()
		{
			for (auto it = depth_stencil_states.begin(); it != depth_stencil_states.end(); it++)
				core::memory::release(it->second);
			depth_stencil_states.clear();

			for (auto it = rasterizer_states.begin(); it != rasterizer_states.end(); it++)
				core::memory::release(it->second);
			rasterizer_states.clear();

			for (auto it = blend_states.begin(); it != blend_states.end(); it++)
				core::memory::release(it->second);
			blend_states.clear();

			for (auto it = sampler_states.begin(); it != sampler_states.end(); it++)
				core::memory::release(it->second);
			sampler_states.clear();

			for (auto it = input_layouts.begin(); it != input_layouts.end(); it++)
				core::memory::release(it->second);
			input_layouts.clear();
			core::memory::release(render_target);
		}
		bool graphics_device::add_section(const std::string_view& name, const std::string_view& code)
		{
			core::string language(core::os::path::get_extension(name));
			if (language.empty())
				return false;

			language.erase(0, 1);
			core::stringify::trim(language);
			core::stringify::to_lower(language);
			remove_section(name);

			section* include = core::memory::init<section>();
			include->code = code;
			include->name = name;
			sections[core::string(name)] = include;

			return true;
		}
		bool graphics_device::remove_section(const std::string_view& name)
		{
			auto it = sections.find(core::key_lookup_cast(name));
			if (it == sections.end())
				return false;

			core::memory::deinit(it->second);
			sections.erase(it);
			return true;
		}
		compute::expects_preprocessor<void> graphics_device::preprocess(shader::desc& subresult)
		{
			if (subresult.data.empty())
				return core::expectation::met;

			switch (backend)
			{
				case vitex::graphics::render_backend::d3d11:
					subresult.defines.push_back("TARGET_D3D");
					break;
				case vitex::graphics::render_backend::ogl:
					subresult.defines.push_back("TARGET_OGL");
					break;
				default:
					break;
			}

			compute::include_desc desc = compute::include_desc();
			desc.exts.push_back(".hlsl");
			desc.exts.push_back(".glsl");
			desc.exts.push_back(".msl");
			desc.exts.push_back(".spv");
			subresult.features.pragmas = false;

			auto directory = core::os::directory::get_working();
			if (directory)
				desc.root = *directory;

			expects_graphics<void> internal_status = core::expectation::met;
			compute::preprocessor* processor = new compute::preprocessor();
			processor->set_include_callback([this, &internal_status, &subresult](compute::preprocessor* p, const compute::include_result& file, core::string& output) -> compute::expects_preprocessor<compute::include_type>
			{
				if (subresult.include)
				{
					auto status = subresult.include(p, file, output);
					if (status && *status != compute::include_type::error)
						return status;
				}

				if (file.library.empty() || (!file.is_file && !file.is_abstract))
					return compute::include_type::error;

				if (file.is_abstract && !file.is_file)
				{
					section* result;
					auto section_status = get_section_info(file.library, &result);
					if (!section_status)
					{
						internal_status = std::move(section_status);
						return compute::include_type::error;
					}

					output.assign(result->code);
					return compute::include_type::preprocess;
				}

				auto data = core::os::file::read_as_string(file.library);
				if (!data)
					return compute::include_type::error;

				output.assign(*data);
				return compute::include_type::preprocess;
			});
			processor->set_include_options(desc);
			processor->set_features(subresult.features);

			for (auto& word : subresult.defines)
				processor->define(word);

			auto result = processor->process(subresult.filename, subresult.data);
			core::memory::release(processor);
			if (!internal_status)
				return compute::preprocessor_exception(compute::preprocessor_error::include_not_found, 0, internal_status.error().message());

			return result;
		}
		expects_graphics<void> graphics_device::transpile(core::string* hlsl, shader_type type, shader_lang to)
		{
			if (!hlsl || hlsl->empty())
				return core::expectation::met;
#ifdef VI_SPIRV
			const char* buffer = hlsl->c_str();
			int size = (int)hlsl->size();

			shader_model model = get_shader_model();
			VI_ASSERT(model != shader_model::any && model != shader_model::invalid, "transpilation requests a defined shader model to proceed");

			EShLanguage stage;
			switch (type)
			{
				case shader_type::vertex:
					stage = EShLangVertex;
					break;
				case shader_type::pixel:
					stage = EShLangFragment;
					break;
				case shader_type::geometry:
					stage = EShLangGeometry;
					break;
				case shader_type::hull:
					stage = EShLangTessControl;
					break;
				case shader_type::domain:
					stage = EShLangTessEvaluation;
					break;
				case shader_type::compute:
					stage = EShLangCompute;
					break;
				default:
					VI_ASSERT(false, "transpilation requests a defined shader type to proceed");
					return graphics_exception("shader model is not valid");
			}

			core::string entry = get_shader_main(type);
			std::vector<uint32_t> binary;
			glslang::InitializeProcess();

			glslang::TShader transpiler(stage);
			transpiler.setStringsWithLengths(&buffer, &size, 1);
			transpiler.setAutoMapBindings(false);
			transpiler.setAutoMapLocations(false);
			transpiler.setEnvInput(glslang::EShSourceHlsl, stage, glslang::EShClientVulkan, 100);
			transpiler.setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_4);
			transpiler.setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_1);
			transpiler.setEntryPoint(entry.c_str());

			EShMessages flags = (EShMessages)(EShMsgSpvRules | EShMsgReadHlsl | EShMsgHlslOffsets | EShMsgHlslLegalization | EShMsgKeepUncalled | EShMsgSuppressWarnings);
			prepare_driver_limits();

			if (!transpiler.parse(&driver_limits, 100, true, flags))
			{
				core::string message = transpiler.getInfoLog();
				glslang::FinalizeProcess();
				return graphics_exception(std::move(message));
			}

			try
			{
				glslang::SpvOptions options;
				options.validate = false;
				options.disableOptimizer = false;
				options.optimizeSize = false;

				spv::SpvBuildLogger logger;
				glslang::TIntermediate* context = transpiler.getIntermediate();
				glslang::GlslangToSpv(*context, binary, &logger, &options);
				glslang::FinalizeProcess();
			}
			catch (...)
			{
				glslang::FinalizeProcess();
				return graphics_exception("shader to spv: an internal error occurred");
			}

			try
			{
				if (to == shader_lang::glsl || to == shader_lang::glsl_es)
				{
					spirv_cross::CompilerGLSL::Options options;
					options.version = (uint32_t)model;
					options.es = (to == shader_lang::glsl_es);

					spirv_cross::CompilerGLSL compiler(binary);
					compiler.set_common_options(options);
					prepare_combined_samplers(&compiler);

					*hlsl = core::copy<core::string>(compiler.compile());
					if (!hlsl->empty())
					{
						core::stringify::replace_groups(*hlsl, "layout\\(row_major\\)\\s+", "");
						core::stringify::replace_groups(*hlsl, "invocations\\s+=\\s+\\d+,\\s+", "");
					}

					return core::expectation::met;
				}
				else if (to == shader_lang::hlsl)
				{
#ifdef VI_MICROSOFT
					spirv_cross::CompilerHLSL::Options options;
					options.shader_model = (uint32_t)model;

					spirv_cross::CompilerHLSL compiler(binary);
					compiler.set_hlsl_options(options);

					*hlsl = core::copy<core::string>(compiler.compile());
					return core::expectation::met;
#else
					return graphics_exception("spv to hlsl: not supported");
#endif
				}
				else if (to == shader_lang::msl)
				{
#ifdef VI_APPLE
					spirv_cross::CompilerMSL::Options options;
					spirv_cross::CompilerMSL compiler(binary);
					compiler.set_msl_options(options);
					prepare_combined_samplers(&compiler);

					*hlsl = core::copy<core::string>(compiler.compile());
					return core::expectation::met;
#else
					return graphics_exception("spv to msv: not supported");
#endif
				}
				else if (to == shader_lang::spv)
				{
					core::string_stream stream;
					std::copy(binary.begin(), binary.end(), std::ostream_iterator<uint32_t>(stream, " "));

					hlsl->assign(stream.str());
					return core::expectation::met;
				}
			}
			catch (const spirv_cross::CompilerError& exception)
			{
				return graphics_exception(core::stringify::text("spv to shader: %s", exception.what()));
			}
			catch (...)
			{
				return graphics_exception("spv to shader: an internal error occurred");
			}

			return graphics_exception("shader transpiler supports only: glsl, glsl_es, hlsl, msl or SPV");
#else
			return graphics_exception("shader transpiler is not supported");
#endif
		}
		expects_graphics<void> graphics_device::get_section_info(const std::string_view& name, section** result)
		{
			if (name.empty() || sections.empty())
				return graphics_exception("shader section name is empty");

			auto resolve = [this, &result](const std::string_view& src)
			{
				auto it = sections.find(core::key_lookup_cast(src));
				if (it == sections.end())
					return false;

				if (result != nullptr)
					*result = it->second;

				return true;
			};

			if (resolve(name) ||
				resolve(core::string(name) + ".hlsl") ||
				resolve(core::string(name) + ".glsl") ||
				resolve(core::string(name) + ".msl") ||
				resolve(core::string(name) + ".spv"))
				return core::expectation::met;
			if (result != nullptr)
				*result = nullptr;

			return graphics_exception("shader section not found: " + core::string(name));
		}
		expects_graphics<void> graphics_device::get_section_data(const std::string_view& name, shader::desc* result)
		{
			if (name.empty() || !result)
				return graphics_exception("shader section name is empty");

			section* subresult;
			auto section_status = get_section_info(name, &subresult);
			if (!section_status)
				return section_status;

			result->filename.assign(subresult->name);
			result->data.assign(subresult->code);
			return core::expectation::met;
		}
		bool graphics_device::is_debug() const
		{
			return debug;
		}
		bool graphics_device::get_program_cache(const std::string_view& name, core::string* data)
		{
			VI_ASSERT(data != nullptr, "data should be set");
			data->clear();

			if (!shader_cache || caches.empty())
				return false;

			core::string path = caches + core::string(name);
			if (path.empty())
				return false;

			if (!core::os::file::is_exists(path.c_str()))
				return false;

			core::uptr<core::gz_stream> stream = new core::gz_stream();
			if (!stream->open(path.c_str(), core::file_mode::binary_read_only))
				return false;

			uint8_t buffer[core::BLOB_SIZE]; size_t size = 0;
			while ((size = (size_t)stream->read(buffer, sizeof(buffer)).or_else(0)) > 0)
				data->append(std::string_view((char*)buffer, size));

			VI_DEBUG("[graphics] load %.*s program cache", (int)name.size(), name.data());
			return !data->empty();
		}
		bool graphics_device::set_program_cache(const std::string_view& name, const std::string_view& data)
		{
			if (!shader_cache || caches.empty())
				return true;

			core::string path = caches + core::string(name);
			if (path.empty())
				return false;

			core::uptr<core::gz_stream> stream = new core::gz_stream();
			if (!stream->open(path.c_str(), core::file_mode::binary_write_only))
				return false;

			size_t size = data.size();
			bool result = (stream->write((uint8_t*)data.data(), size).or_else(0) == size);
			VI_DEBUG("[graphics] save %.*s program cache", (int)name.size(), name.data());
			return result;
		}
		bool graphics_device::is_left_handed() const
		{
			return backend == render_backend::d3d11;
		}
		uint32_t graphics_device::get_row_pitch(uint32_t width, uint32_t element_size) const
		{
			return width * element_size;
		}
		uint32_t graphics_device::get_depth_pitch(uint32_t row_pitch, uint32_t height) const
		{
			return row_pitch * height;
		}
		uint32_t graphics_device::get_mip_level(uint32_t width, uint32_t height) const
		{
			uint32_t mip_levels = 1;
			while (width > 1 && height > 1)
			{
				width = (uint32_t)compute::mathf::max((float)width / 2.0f, 1.0f);
				height = (uint32_t)compute::mathf::max((float)height / 2.0f, 1.0f);
				mip_levels++;
			}

			return mip_levels;
		}
		uint32_t graphics_device::get_format_size(format mode) const
		{
			switch (mode)
			{
				case format::a8_unorm:
				case format::r1_unorm:
				case format::r8_sint:
				case format::r8_snorm:
				case format::r8_uint:
				case format::r8_unorm:
					return 1;
				case format::d16_unorm:
				case format::r16_float:
				case format::r16_sint:
				case format::r16_snorm:
				case format::r16_uint:
				case format::r16_unorm:
				case format::r8g8_sint:
				case format::r8g8_snorm:
				case format::r8g8_uint:
				case format::r8g8_unorm:
					return 2;
				case format::d24_unorm_s8_uint:
				case format::d32_float:
				case format::r10g10b10a2_uint:
				case format::r10g10b10a2_unorm:
				case format::r11g11b10_float:
				case format::r16g16_float:
				case format::r16g16_sint:
				case format::r16g16_snorm:
				case format::r16g16_uint:
				case format::r16g16_unorm:
				case format::r32_float:
				case format::r32_sint:
				case format::r32_uint:
				case format::r8g8b8a8_sint:
				case format::r8g8b8a8_snorm:
				case format::r8g8b8a8_uint:
				case format::r8g8b8a8_unorm:
				case format::r8g8b8a8_unorm_srgb:
				case format::r8g8b8g8_unorm:
				case format::r9g9b9e5_share_dexp:
					return 4;
				case format::r16g16b16a16_float:
				case format::r16g16b16a16_sint:
				case format::r16g16b16a16_snorm:
				case format::r16g16b16a16_uint:
				case format::r16g16b16a16_unorm:
				case format::r32g32_float:
				case format::r32g32_sint:
				case format::r32g32_uint:
					return 8;
				case format::r32g32b32a32_float:
				case format::r32g32b32a32_sint:
				case format::r32g32b32a32_uint:
					return 16;
				case format::r32g32b32_float:
				case format::r32g32b32_sint:
				case format::r32g32b32_uint:
					return 12;
				default:
					return 0;
			}
		}
		uint32_t graphics_device::get_present_flags() const
		{
			return present_flags;
		}
		uint32_t graphics_device::get_compile_flags() const
		{
			return compile_flags;
		}
		core::option<core::string> graphics_device::get_program_name(const shader::desc& desc)
		{
			core::string result = desc.filename;
			for (auto& item : desc.defines)
				result += '&' + item + "=1";

			if (desc.features.pragmas)
				result += "&pragmas=on";

			if (desc.features.includes)
				result += "&includes=on";

			if (desc.features.defines)
				result += "&defines=on";

			if (desc.features.conditions)
				result += "&conditions=on";

			switch (desc.stage)
			{
				case shader_type::vertex:
					result += "&stage=vertex";
					break;
				case shader_type::pixel:
					result += "&stage=pixel";
					break;
				case shader_type::geometry:
					result += "&stage=geometry";
					break;
				case shader_type::hull:
					result += "&stage=hull";
					break;
				case shader_type::domain:
					result += "&stage=domain";
					break;
				case shader_type::compute:
					result += "&stage=compute";
					break;
				default:
					break;
			}

			auto hash = compute::crypto::hash_hex(compute::digests::md5(), result);
			if (!hash)
				return core::optional::none;

			core::string postfix;
			switch (backend)
			{
				case vitex::graphics::render_backend::d3d11:
					postfix = ".hlsl";
					break;
				case vitex::graphics::render_backend::ogl:
					postfix = ".glsl";
					break;
				default:
					break;
			}

			return *hash + postfix;
		}
		core::string graphics_device::get_shader_main(shader_type type) const
		{
			switch (type)
			{
				case shader_type::vertex:
					return "vs_main";
				case shader_type::pixel:
					return "ps_main";
				case shader_type::geometry:
					return "gs_main";
				case shader_type::hull:
					return "hs_main";
				case shader_type::domain:
					return "ds_main";
				case shader_type::compute:
					return "cs_main";
				default:
					return "main";
			}
		}
		shader_model graphics_device::get_shader_model() const
		{
			return shader_gen;
		}
		const core::unordered_map<core::string, depth_stencil_state*>& graphics_device::get_depth_stencil_states() const
		{
			return depth_stencil_states;
		}
		const core::unordered_map<core::string, rasterizer_state*>& graphics_device::get_rasterizer_states() const
		{
			return rasterizer_states;
		}
		const core::unordered_map<core::string, blend_state*>& graphics_device::get_blend_states() const
		{
			return blend_states;
		}
		const core::unordered_map<core::string, sampler_state*>& graphics_device::get_sampler_states() const
		{
			return sampler_states;
		}
		const core::unordered_map<core::string, input_layout*>& graphics_device::get_input_layouts() const
		{
			return input_layouts;
		}
		expects_video<surface*> graphics_device::create_surface(texture_2d* base)
		{
			VI_ASSERT(base != nullptr, "texture should be set");
#ifdef VI_SDL2
			int width = (int)base->get_width();
			int height = (int)base->get_height();
			int bytes_per_pixel = get_format_size(base->get_format_mode());
			int bits_per_pixel = bytes_per_pixel * 8;

			texture_2d::desc desc;
			desc.access_flags = cpu_access::read;
			desc.usage = resource_usage::staging;
			desc.bind_flags = (resource_bind)0;
			desc.format_mode = base->get_format_mode();
			desc.width = base->get_width();
			desc.height = base->get_height();
			desc.mip_levels = base->get_mip_levels();

			auto copy = create_texture_2d(desc);
			if (!copy)
				return video_exception(std::move(copy.error()));

			texture_2d* copy_texture_address = *copy;
			core::uptr<texture_2d> copy_texture = copy_texture_address;
			auto copy_status = copy_texture_2d(base, &copy_texture_address);
			if (!copy_status)
				return video_exception(std::move(copy_status.error()));

			mapped_subresource data;
			auto map_status = map(copy_texture_address, resource_map::read, &data);
			if (!map_status)
				return video_exception(std::move(map_status.error()));
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
			const Uint32 r = 0xff000000;
			const Uint32 g = 0x00ff0000;
			const Uint32 b = 0x0000ff00;
			const Uint32 a = 0x000000ff;
#else
			const Uint32 r = 0x000000ff;
			const Uint32 g = 0x0000ff00;
			const Uint32 b = 0x00ff0000;
			const Uint32 a = 0xff000000;
#endif
			SDL_Surface* handle = SDL_CreateRGBSurface(0, width, height, bits_per_pixel, r, g, b, a);
			if (handle != nullptr)
			{
				SDL_SetSurfaceBlendMode(handle, SDL_BlendMode::SDL_BLENDMODE_BLEND);
				SDL_LockSurface(handle);
				memcpy(handle->pixels, data.pointer, width * height * bytes_per_pixel);
				SDL_UnlockSurface(handle);
			}

			unmap(copy_texture_address, &data);
			if (!handle)
				return video_exception();

			return new surface(handle);
#else
			return video_exception();
#endif
		}
		depth_stencil_state* graphics_device::get_depth_stencil_state(const std::string_view& name)
		{
			auto it = depth_stencil_states.find(core::key_lookup_cast(name));
			if (it != depth_stencil_states.end())
				return it->second;

			return nullptr;
		}
		blend_state* graphics_device::get_blend_state(const std::string_view& name)
		{
			auto it = blend_states.find(core::key_lookup_cast(name));
			if (it != blend_states.end())
				return it->second;

			return nullptr;
		}
		rasterizer_state* graphics_device::get_rasterizer_state(const std::string_view& name)
		{
			auto it = rasterizer_states.find(core::key_lookup_cast(name));
			if (it != rasterizer_states.end())
				return it->second;

			return nullptr;
		}
		sampler_state* graphics_device::get_sampler_state(const std::string_view& name)
		{
			auto it = sampler_states.find(core::key_lookup_cast(name));
			if (it != sampler_states.end())
				return it->second;

			return nullptr;
		}
		input_layout* graphics_device::get_input_layout(const std::string_view& name)
		{
			auto it = input_layouts.find(core::key_lookup_cast(name));
			if (it != input_layouts.end())
				return it->second;

			return nullptr;
		}
		render_target_2d* graphics_device::get_render_target()
		{
			return render_target;
		}
		render_backend graphics_device::get_backend() const
		{
			return backend;
		}
		vsync graphics_device::get_vsync_mode() const
		{
			return vsync_mode;
		}
		graphics_device* graphics_device::create(desc& i)
		{
			i.backend = get_supported_backend(i.backend);
#ifdef VI_MICROSOFT
			if (i.backend == render_backend::d3d11)
				return new d3d11::d3d11_device(i);
#endif
#ifdef VI_GL
			if (i.backend == render_backend::ogl)
				return new ogl::ogl_device(i);
#endif
			VI_PANIC(false, "renderer backend is not present or is invalid");
			return nullptr;
		}
		expects_graphics<void> graphics_device::compile_builtin_shaders(const core::vector<graphics_device*>& devices, const std::function<bool(graphics_device*, const std::string_view&, const expects_graphics<shader*>&)>& callback)
		{
			for (auto* device : devices)
			{
				if (!device)
					continue;

				device->set_as_current_device();
				for (auto& section : device->sections)
				{
					shader::desc desc;
					if (!device->get_section_data(section.first, &desc))
						continue;

					auto result = device->create_shader(desc);
					if (callback && !callback(device, section.first, result))
						return result ? graphics_exception("compilation stopped") : result.error();
					else if (!result)
						return result.error();
					if (!callback)
						core::memory::release(*result);
				}
			}
			return core::expectation::met;
		}

		activity::activity(const desc& i) noexcept : handle(nullptr), favicon(nullptr), options(i), command(0), cx(0), cy(0), message(this)
		{
#ifdef VI_SDL2
			cursors[(size_t)display_cursor::arrow] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
			cursors[(size_t)display_cursor::text_input] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_IBEAM);
			cursors[(size_t)display_cursor::resize_all] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEALL);
			cursors[(size_t)display_cursor::resize_ns] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENS);
			cursors[(size_t)display_cursor::resize_ew] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEWE);
			cursors[(size_t)display_cursor::resize_nesw] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENESW);
			cursors[(size_t)display_cursor::resize_nwse] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENWSE);
			cursors[(size_t)display_cursor::hand] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);
			cursors[(size_t)display_cursor::crosshair] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_CROSSHAIR);
			cursors[(size_t)display_cursor::wait] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_WAIT);
			cursors[(size_t)display_cursor::progress] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_WAITARROW);
			cursors[(size_t)display_cursor::no] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_NO);
#endif
			memset(keys[0], 0, sizeof(keys[0]));
			memset(keys[1], 0, sizeof(keys[1]));
			if (!i.gpu_as_renderer)
				apply_configuration(render_backend::none);
		}
		activity::~activity() noexcept
		{
#ifdef VI_SDL2
			for (size_t i = 0; i < (size_t)display_cursor::count; i++)
				SDL_FreeCursor(cursors[i]);

			if (handle != nullptr)
			{
				SDL_DestroyWindow(handle);
				handle = nullptr;
			}
#endif
		}
		bool activity::apply_system_theme()
		{
#ifdef VI_SDL2
#ifdef VI_MICROSOFT
			SDL_SysWMinfo info;
			SDL_VERSION(&info.version);
			SDL_GetWindowWMInfo(handle, &info);
			HWND window_handle = info.info.win.window;
			if (window_handle == INVALID_HANDLE_VALUE)
				return false;

			HMODULE library = LoadLibraryA("dwmapi.dll");
			if (!library)
				return false;

			typedef HRESULT(*DwmSetWindowAttributePtr1)(HWND, DWORD, LPCVOID, DWORD);
			DwmSetWindowAttributePtr1 DWM_SetWindowAttribute = (DwmSetWindowAttributePtr1)GetProcAddress(library, "DwmSetWindowAttribute");
			if (!DWM_SetWindowAttribute)
				return false;

			HKEY personalize;
			DWORD is_light_theme = 0, is_light_theme_size = sizeof(DWORD), type = REG_DWORD;
			if (RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", 0, KEY_QUERY_VALUE, &personalize) == ERROR_SUCCESS)
				RegQueryValueEx(personalize, "SystemUsesLightTheme", NULL, &type, (LPBYTE)&is_light_theme, &is_light_theme_size);
			RegCloseKey(personalize);

			BOOL dark_mode = is_light_theme ? 0 : 1;
			if (DWM_SetWindowAttribute(window_handle, DWMWA_USE_IMMERSIVE_DARK_MODE, &dark_mode, sizeof(dark_mode)) != S_OK)
			{
				const DWORD DWMWA_USE_IMMERSIVE_DARK_MODE_DEPRECATED = 19;
				DWM_SetWindowAttribute(window_handle, DWMWA_USE_IMMERSIVE_DARK_MODE_DEPRECATED, &dark_mode, sizeof(dark_mode));
			}
#if 0
			DWM_SYSTEMBACKDROP_TYPE backdrop_type = DWMSBT_MAINWINDOW;
			if (DWM_SetWindowAttribute(window_handle, DWMWA_SYSTEMBACKDROP_TYPE, &backdrop_type, sizeof(backdrop_type)) != S_OK)
			{
				BOOL mica_effect = true;
				const DWORD DWMWA_MICA_EFFECT_DEPRECATED = 1029;
				DWM_SetWindowAttribute(window_handle, DWMWA_MICA_EFFECT_DEPRECATED, &mica_effect, sizeof(mica_effect));
			}
#endif
			return true;
#else
			return false;
#endif
#else
			return false;
#endif
		}
		void activity::set_clipboard_text(const std::string_view& text)
		{
#ifdef VI_SDL2
			VI_ASSERT(handle != nullptr, "activity should be initialized");
			VI_ASSERT(core::stringify::is_cstring(text), "text should be set");
			SDL_SetClipboardText(text.data());
#endif
		}
		void activity::set_cursor_position(const trigonometry::vector2& position)
		{
#ifdef VI_SDL2
#if SDL_VERSION_ATLEAST(2, 0, 4)
			VI_ASSERT(handle != nullptr, "activity should be initialized");
			SDL_WarpMouseInWindow(handle, (int)position.x, (int)position.y);
#endif
#endif
		}
		void activity::set_cursor_position(float x, float y)
		{
#ifdef VI_SDL2
#if SDL_VERSION_ATLEAST(2, 0, 4)
			VI_ASSERT(handle != nullptr, "activity should be initialized");
			SDL_WarpMouseInWindow(handle, (int)x, (int)y);
#endif
#endif
		}
		void activity::set_global_cursor_position(const trigonometry::vector2& position)
		{
#ifdef VI_SDL2
			VI_ASSERT(handle != nullptr, "activity should be initialized");
#if SDL_VERSION_ATLEAST(2, 0, 4)
			SDL_WarpMouseGlobal((int)position.x, (int)position.y);
#endif
#endif
		}
		void activity::set_global_cursor_position(float x, float y)
		{
#ifdef VI_SDL2
			VI_ASSERT(handle != nullptr, "activity should be initialized");
#if SDL_VERSION_ATLEAST(2, 0, 4)
			SDL_WarpMouseGlobal((int)x, (int)y);
#endif
#endif
		}
		void activity::set_key(key_code key, bool value)
		{
			keys[0][(size_t)key] = value;
		}
		void activity::set_cursor(display_cursor style)
		{
#ifdef VI_SDL2
			VI_ASSERT((size_t)style <= (size_t)display_cursor::count, "style should be less than %i", (int)display_cursor::count);
			if (style != display_cursor::none)
			{
				SDL_ShowCursor(1);
				SDL_SetCursor(cursors[(size_t)style]);
			}
			else
			{
				SDL_ShowCursor(0);
				SDL_SetCursor(cursors[(size_t)display_cursor::arrow]);
			}
#endif
		}
		void activity::set_cursor_visibility(bool enabled)
		{
#ifdef VI_SDL2
			VI_ASSERT(handle != nullptr, "activity should be initialized");
			SDL_ShowCursor(enabled);
#endif
		}
		void activity::set_grabbing(bool enabled)
		{
#ifdef VI_SDL2
			VI_ASSERT(handle != nullptr, "activity should be initialized");
			SDL_SetWindowGrab(handle, enabled ? SDL_TRUE : SDL_FALSE);
#endif
		}
		void activity::set_fullscreen(bool enabled)
		{
#ifdef VI_SDL2
			VI_ASSERT(handle != nullptr, "activity should be initialized");
			SDL_SetWindowFullscreen(handle, enabled ? SDL_WINDOW_FULLSCREEN : 0);
#endif
		}
		void activity::set_borderless(bool enabled)
		{
#ifdef VI_SDL2
			VI_ASSERT(handle != nullptr, "activity should be initialized");
			SDL_SetWindowBordered(handle, enabled ? SDL_TRUE : SDL_FALSE);
#endif
		}
		void activity::set_screen_keyboard(bool enabled)
		{
#ifdef VI_SDL2
			VI_ASSERT(handle != nullptr, "activity should be initialized");
			if (!SDL_HasScreenKeyboardSupport())
				return;

			if (enabled)
				SDL_StartTextInput();
			else
				SDL_StopTextInput();
#endif
		}
		void activity::apply_configuration(render_backend backend)
		{
#ifdef VI_SDL2
			if (handle != nullptr)
			{
				event_source.pop(this);
				SDL_DestroyWindow(handle);
				handle = nullptr;
			}

			Uint32 flags = 0;
			if (options.fullscreen)
				flags |= SDL_WINDOW_FULLSCREEN;

			if (options.hidden)
				flags |= SDL_WINDOW_HIDDEN;

			if (options.borderless)
				flags |= SDL_WINDOW_BORDERLESS;

			if (options.resizable)
				flags |= SDL_WINDOW_RESIZABLE;

			if (options.minimized)
				flags |= SDL_WINDOW_MINIMIZED;

			if (options.maximized)
				flags |= SDL_WINDOW_MAXIMIZED;

			if (options.focused)
				flags |= SDL_WINDOW_INPUT_GRABBED;

			if (options.high_dpi)
				flags |= SDL_WINDOW_ALLOW_HIGHDPI;

			if (options.centered)
			{
				options.x = SDL_WINDOWPOS_CENTERED;
				options.y = SDL_WINDOWPOS_CENTERED;
			}
			else if (options.free_position)
			{
				options.x = SDL_WINDOWPOS_UNDEFINED;
				options.y = SDL_WINDOWPOS_UNDEFINED;
			}

			switch (backend)
			{
				case vitex::graphics::render_backend::ogl:
					flags |= SDL_WINDOW_OPENGL;
					break;
				default:
					break;
			}

			handle = SDL_CreateWindow(options.title.c_str(), options.x, options.y, options.width, options.height, flags);
			if (handle != nullptr)
			{
				event_source.push(this);
				apply_system_theme();
			}
#endif
		}
		void activity::wakeup()
		{
#ifdef VI_SDL2
			SDL_Event event;
			event.type = SDL_USEREVENT;
			event.user.timestamp = SDL_GetTicks();
			event.user.windowID = SDL_GetWindowID(handle);
			event.user.code = 200;
			event.user.data1 = nullptr;
			event.user.data2 = nullptr;
			SDL_PushEvent(&event);
#endif
		}
		void activity::hide()
		{
#ifdef VI_SDL2
			VI_ASSERT(handle != nullptr, "activity should be initialized");
			SDL_HideWindow(handle);
#endif
		}
		void activity::show()
		{
#ifdef VI_SDL2
			VI_ASSERT(handle != nullptr, "activity should be initialized");
			SDL_ShowWindow(handle);
#endif
		}
		void activity::maximize()
		{
#ifdef VI_SDL2
			VI_ASSERT(handle != nullptr, "activity should be initialized");
			SDL_MaximizeWindow(handle);
#endif
		}
		void activity::minimize()
		{
#ifdef VI_SDL2
			VI_ASSERT(handle != nullptr, "activity should be initialized");
			SDL_MinimizeWindow(handle);
#endif
		}
		void activity::focus()
		{
#ifdef VI_SDL2
#if SDL_VERSION_ATLEAST(2, 0, 5)
			VI_ASSERT(handle != nullptr, "activity should be initialized");
			SDL_SetWindowInputFocus(handle);
#endif
#endif
		}
		void activity::move(int x, int y)
		{
#ifdef VI_SDL2
			VI_ASSERT(handle != nullptr, "activity should be initialized");
			SDL_SetWindowPosition(handle, x, y);
#endif
		}
		void activity::resize(int w, int h)
		{
#ifdef VI_SDL2
			VI_ASSERT(handle != nullptr, "activity should be initialized");
			SDL_SetWindowSize(handle, w, h);
#endif
		}
		void activity::set_title(const std::string_view& value)
		{
#ifdef VI_SDL2
			VI_ASSERT(handle != nullptr, "activity should be initialized");
			VI_ASSERT(core::stringify::is_cstring(value), "value should be set");
			SDL_SetWindowTitle(handle, value.data());
#endif
		}
		void activity::set_icon(surface* icon)
		{
#ifdef VI_SDL2
			VI_ASSERT(handle != nullptr, "activity should be initialized");
			if (!favicon)
				favicon = SDL_GetWindowSurface(handle);

			SDL_SetWindowIcon(handle, icon ? (SDL_Surface*)icon->get_resource() : favicon);
#endif
		}
		void activity::load(SDL_SysWMinfo* base)
		{
#ifdef VI_SDL2
			VI_ASSERT(handle != nullptr, "activity should be initialized");
			VI_ASSERT(base != nullptr, "base should be set");
			SDL_VERSION(&base->version);
			SDL_GetWindowWMInfo(handle, base);
#endif
		}
		bool activity::dispatch(uint64_t timeout_ms, bool dispatch_all)
		{
			return multi_dispatch(event_source, timeout_ms, dispatch_all);
		}
		bool activity::multi_dispatch(const event_consumers& sources, uint64_t timeout_ms, bool dispatch_all)
		{
			VI_MEASURE(core::timings::mixed);
#ifdef VI_SDL2
			if (sources.consumers.empty())
				return false;

			for (auto& item : sources.consumers)
			{
				auto* target = item.second;
				VI_ASSERT(target->handle != nullptr, "activity should be initialized");
				memcpy((void*)target->keys[1], (void*)target->keys[0], sizeof(target->keys[0]));
				target->command = (int)SDL_GetModState();
				target->get_input_state();
				target->message.dispatch();
			}

			SDL_Event event;
			size_t incoming_events = 0;
			int has_events = timeout_ms > 0 ? SDL_WaitEventTimeout(&event, (int)timeout_ms) : SDL_PollEvent(&event);
			while (has_events)
			{
				bool is_common_event = true;
				switch (event.type)
				{
					case SDL_QUIT:
						for (auto& item : sources.consumers)
						{
							if (item.second->callbacks.window_state_change)
								item.second->callbacks.window_state_change(window_state::close, 0, 0);
							if (item.second->callbacks.app_state_change)
								item.second->callbacks.app_state_change(app_state::close);
						}
						break;
					case SDL_APP_TERMINATING:
						for (auto& item : sources.consumers)
						{
							if (item.second->callbacks.app_state_change)
								item.second->callbacks.app_state_change(app_state::terminating);
						}
						break;
					case SDL_APP_LOWMEMORY:
						for (auto& item : sources.consumers)
						{
							if (item.second->callbacks.app_state_change)
								item.second->callbacks.app_state_change(app_state::low_memory);
						}
						break;
					case SDL_APP_WILLENTERBACKGROUND:
						for (auto& item : sources.consumers)
						{
							if (item.second->callbacks.app_state_change)
								item.second->callbacks.app_state_change(app_state::enter_background_start);
						}
						break;
					case SDL_APP_DIDENTERBACKGROUND:
						for (auto& item : sources.consumers)
						{
							if (item.second->callbacks.app_state_change)
								item.second->callbacks.app_state_change(app_state::enter_background_end);
						}
						break;
					case SDL_APP_WILLENTERFOREGROUND:
						for (auto& item : sources.consumers)
						{
							if (item.second->callbacks.app_state_change)
								item.second->callbacks.app_state_change(app_state::enter_foreground_start);
						}
						break;
					case SDL_APP_DIDENTERFOREGROUND:
						for (auto& item : sources.consumers)
						{
							if (item.second->callbacks.app_state_change)
								item.second->callbacks.app_state_change(app_state::enter_foreground_end);
						}
						break;
					default:
						is_common_event = false;
						break;
				}

				auto* target = is_common_event ? nullptr : sources.find(event.window.windowID);
				if (target != nullptr)
				{
					switch (event.type)
					{
						case SDL_WINDOWEVENT:
							switch (event.window.event)
							{
								case SDL_WINDOWEVENT_SHOWN:
									if (target->callbacks.window_state_change)
										target->callbacks.window_state_change(window_state::show, 0, 0);
									break;
								case SDL_WINDOWEVENT_HIDDEN:
									if (target->callbacks.window_state_change)
										target->callbacks.window_state_change(window_state::hide, 0, 0);
									break;
								case SDL_WINDOWEVENT_EXPOSED:
									if (target->callbacks.window_state_change)
										target->callbacks.window_state_change(window_state::expose, 0, 0);
									break;
								case SDL_WINDOWEVENT_MOVED:
									if (target->callbacks.window_state_change)
										target->callbacks.window_state_change(window_state::move, event.window.data1, event.window.data2);
									break;
								case SDL_WINDOWEVENT_RESIZED:
									if (target->callbacks.window_state_change)
										target->callbacks.window_state_change(window_state::resize, event.window.data1, event.window.data2);
									break;
								case SDL_WINDOWEVENT_SIZE_CHANGED:
									if (target->callbacks.window_state_change)
										target->callbacks.window_state_change(window_state::size_change, event.window.data1, event.window.data2);
									break;
								case SDL_WINDOWEVENT_MINIMIZED:
									if (target->callbacks.window_state_change)
										target->callbacks.window_state_change(window_state::minimize, 0, 0);
									break;
								case SDL_WINDOWEVENT_MAXIMIZED:
									if (target->callbacks.window_state_change)
										target->callbacks.window_state_change(window_state::maximize, 0, 0);
									break;
								case SDL_WINDOWEVENT_RESTORED:
									if (target->callbacks.window_state_change)
										target->callbacks.window_state_change(window_state::restore, 0, 0);
									break;
								case SDL_WINDOWEVENT_ENTER:
									if (target->callbacks.window_state_change)
										target->callbacks.window_state_change(window_state::enter, 0, 0);
									break;
								case SDL_WINDOWEVENT_LEAVE:
									if (target->callbacks.window_state_change)
										target->callbacks.window_state_change(window_state::leave, 0, 0);
									break;
#if SDL_VERSION_ATLEAST(2, 0, 5)
								case SDL_WINDOWEVENT_TAKE_FOCUS:
#endif
								case SDL_WINDOWEVENT_FOCUS_GAINED:
									if (target->callbacks.window_state_change)
										target->callbacks.window_state_change(window_state::focus, 0, 0);
									break;
								case SDL_WINDOWEVENT_FOCUS_LOST:
									if (target->callbacks.window_state_change)
										target->callbacks.window_state_change(window_state::blur, 0, 0);
									break;
								case SDL_WINDOWEVENT_CLOSE:
									if (target->callbacks.window_state_change)
										target->callbacks.window_state_change(window_state::close, 0, 0);
									break;
							}
							break;
						case SDL_KEYDOWN:
							if (target->callbacks.key_state)
								target->callbacks.key_state((key_code)event.key.keysym.scancode, (key_mod)event.key.keysym.mod, (int)event.key.keysym.sym, (int)(event.key.repeat != 0), true);

							if (target->mapping.enabled && !target->mapping.mapped)
							{
								target->mapping.key.key = (key_code)event.key.keysym.scancode;
								target->mapping.mapped = true;
								target->mapping.captured = false;
							}
							break;
						case SDL_KEYUP:
							if (target->callbacks.key_state)
								target->callbacks.key_state((key_code)event.key.keysym.scancode, (key_mod)event.key.keysym.mod, (int)event.key.keysym.sym, (int)(event.key.repeat != 0), false);

							if (target->mapping.enabled && target->mapping.mapped && target->mapping.key.key == (key_code)event.key.keysym.scancode)
							{
								target->mapping.key.mod = (key_mod)SDL_GetModState();
								target->mapping.captured = true;
							}
							break;
						case SDL_TEXTINPUT:
							if (target->callbacks.input)
								target->callbacks.input((char*)event.text.text, (int)strlen(event.text.text));
							break;
						case SDL_TEXTEDITING:
							if (target->callbacks.input_edit)
								target->callbacks.input_edit((char*)event.edit.text, (int)event.edit.start, (int)event.edit.length);
							break;
						case SDL_MOUSEMOTION:
							target->cx = event.motion.x;
							target->cy = event.motion.y;
							if (target->callbacks.cursor_move)
								target->callbacks.cursor_move(target->cx, target->cy, (int)event.motion.xrel, (int)event.motion.yrel);
							break;
						case SDL_MOUSEBUTTONDOWN:
							switch (event.button.button)
							{
								case SDL_BUTTON_LEFT:
									if (target->callbacks.key_state)
										target->callbacks.key_state(key_code::cursor_left, (key_mod)SDL_GetModState(), (int)key_code::cursor_left, (int)event.button.clicks, true);

									if (target->mapping.enabled && !target->mapping.mapped)
									{
										target->mapping.key.key = key_code::cursor_left;
										target->mapping.mapped = true;
										target->mapping.captured = false;
									}
									break;
								case SDL_BUTTON_MIDDLE:
									if (target->callbacks.key_state)
										target->callbacks.key_state(key_code::cursor_middle, (key_mod)SDL_GetModState(), (int)key_code::cursor_middle, (int)event.button.clicks, true);

									if (target->mapping.enabled && !target->mapping.mapped)
									{
										target->mapping.key.key = key_code::cursor_middle;
										target->mapping.mapped = true;
										target->mapping.captured = false;
									}
									break;
								case SDL_BUTTON_RIGHT:
									if (target->callbacks.key_state)
										target->callbacks.key_state(key_code::cursor_right, (key_mod)SDL_GetModState(), (int)key_code::cursor_right, (int)event.button.clicks, true);

									if (target->mapping.enabled && !target->mapping.mapped)
									{
										target->mapping.key.key = key_code::cursor_right;
										target->mapping.mapped = true;
										target->mapping.captured = false;
									}
									break;
								case SDL_BUTTON_X1:
									if (target->callbacks.key_state)
										target->callbacks.key_state(key_code::cursor_x1, (key_mod)SDL_GetModState(), (int)key_code::cursor_x1, (int)event.button.clicks, true);

									if (target->mapping.enabled && !target->mapping.mapped)
									{
										target->mapping.key.key = key_code::cursor_x1;
										target->mapping.mapped = true;
										target->mapping.captured = false;
									}
									break;
								case SDL_BUTTON_X2:
									if (target->callbacks.key_state)
										target->callbacks.key_state(key_code::cursor_x2, (key_mod)SDL_GetModState(), (int)key_code::cursor_x2, (int)event.button.clicks, true);

									if (target->mapping.enabled && !target->mapping.mapped)
									{
										target->mapping.key.key = key_code::cursor_x2;
										target->mapping.mapped = true;
										target->mapping.captured = false;
									}
									break;
							}
							break;
						case SDL_MOUSEBUTTONUP:
							switch (event.button.button)
							{
								case SDL_BUTTON_LEFT:
									if (target->callbacks.key_state)
										target->callbacks.key_state(key_code::cursor_left, (key_mod)SDL_GetModState(), (int)key_code::cursor_left, (int)event.button.clicks, false);

									if (target->mapping.enabled && target->mapping.mapped && target->mapping.key.key == key_code::cursor_left)
									{
										target->mapping.key.mod = (key_mod)SDL_GetModState();
										target->mapping.captured = true;
									}
									break;
								case SDL_BUTTON_MIDDLE:
									if (target->callbacks.key_state)
										target->callbacks.key_state(key_code::cursor_middle, (key_mod)SDL_GetModState(), (int)key_code::cursor_middle, (int)event.button.clicks, false);

									if (target->mapping.enabled && target->mapping.mapped && target->mapping.key.key == key_code::cursor_middle)
									{
										target->mapping.key.mod = (key_mod)SDL_GetModState();
										target->mapping.captured = true;
									}
									break;
								case SDL_BUTTON_RIGHT:
									if (target->callbacks.key_state)
										target->callbacks.key_state(key_code::cursor_right, (key_mod)SDL_GetModState(), (int)key_code::cursor_right, (int)event.button.clicks, false);

									if (target->mapping.enabled && target->mapping.mapped && target->mapping.key.key == key_code::cursor_right)
									{
										target->mapping.key.mod = (key_mod)SDL_GetModState();
										target->mapping.captured = true;
									}
									break;
								case SDL_BUTTON_X1:
									if (target->callbacks.key_state)
										target->callbacks.key_state(key_code::cursor_x1, (key_mod)SDL_GetModState(), (int)key_code::cursor_x1, (int)event.button.clicks, false);

									if (target->mapping.enabled && target->mapping.mapped && target->mapping.key.key == key_code::cursor_x1)
									{
										target->mapping.key.mod = (key_mod)SDL_GetModState();
										target->mapping.captured = true;
									}
									break;
								case SDL_BUTTON_X2:
									if (target->callbacks.key_state)
										target->callbacks.key_state(key_code::cursor_x2, (key_mod)SDL_GetModState(), (int)key_code::cursor_x2, (int)event.button.clicks, false);

									if (target->mapping.enabled && target->mapping.mapped && target->mapping.key.key == key_code::cursor_x2)
									{
										target->mapping.key.mod = (key_mod)SDL_GetModState();
										target->mapping.captured = true;
									}
									break;
							}
							break;
						case SDL_MOUSEWHEEL:
#if SDL_VERSION_ATLEAST(2, 0, 4)
							if (target->callbacks.cursor_wheel_state)
								target->callbacks.cursor_wheel_state((int)event.wheel.x, (int)event.wheel.y, event.wheel.direction == SDL_MOUSEWHEEL_NORMAL);
#else
							if (target->callbacks.cursor_wheel_state)
								target->callbacks.cursor_wheel_state((int)event.wheel.x, (int)event.wheel.y, 1);
#endif
							break;
						case SDL_JOYAXISMOTION:
							if (target->callbacks.joy_stick_axis_move)
								target->callbacks.joy_stick_axis_move((int)event.jaxis.which, (int)event.jaxis.axis, (int)event.jaxis.value);
							break;
						case SDL_JOYBALLMOTION:
							if (target->callbacks.joy_stick_ball_move)
								target->callbacks.joy_stick_ball_move((int)event.jball.which, (int)event.jball.ball, (int)event.jball.xrel, (int)event.jball.yrel);
							break;
						case SDL_JOYHATMOTION:
							if (target->callbacks.joy_stick_hat_move)
							{
								switch (event.jhat.value)
								{
									case SDL_HAT_CENTERED:
										target->callbacks.joy_stick_hat_move(joy_stick_hat::center, (int)event.jhat.which, (int)event.jhat.hat);
										break;
									case SDL_HAT_UP:
										target->callbacks.joy_stick_hat_move(joy_stick_hat::up, (int)event.jhat.which, (int)event.jhat.hat);
										break;
									case SDL_HAT_DOWN:
										target->callbacks.joy_stick_hat_move(joy_stick_hat::down, (int)event.jhat.which, (int)event.jhat.hat);
										break;
									case SDL_HAT_LEFT:
										target->callbacks.joy_stick_hat_move(joy_stick_hat::left, (int)event.jhat.which, (int)event.jhat.hat);
										break;
									case SDL_HAT_LEFTUP:
										target->callbacks.joy_stick_hat_move(joy_stick_hat::left_up, (int)event.jhat.which, (int)event.jhat.hat);
										break;
									case SDL_HAT_LEFTDOWN:
										target->callbacks.joy_stick_hat_move(joy_stick_hat::left_down, (int)event.jhat.which, (int)event.jhat.hat);
										break;
									case SDL_HAT_RIGHT:
										target->callbacks.joy_stick_hat_move(joy_stick_hat::right, (int)event.jhat.which, (int)event.jhat.hat);
										break;
									case SDL_HAT_RIGHTUP:
										target->callbacks.joy_stick_hat_move(joy_stick_hat::right_up, (int)event.jhat.which, (int)event.jhat.hat);
										break;
									case SDL_HAT_RIGHTDOWN:
										target->callbacks.joy_stick_hat_move(joy_stick_hat::right_down, (int)event.jhat.which, (int)event.jhat.hat);
										break;
								}
							}
							break;
						case SDL_JOYBUTTONDOWN:
							if (target->callbacks.joy_stick_key_state)
								target->callbacks.joy_stick_key_state((int)event.jbutton.which, (int)event.jbutton.button, true);
							break;
						case SDL_JOYBUTTONUP:
							if (target->callbacks.joy_stick_key_state)
								target->callbacks.joy_stick_key_state((int)event.jbutton.which, (int)event.jbutton.button, false);
							break;
						case SDL_JOYDEVICEADDED:
							if (target->callbacks.joy_stick_state)
								target->callbacks.joy_stick_state((int)event.jdevice.which, true);
							break;
						case SDL_JOYDEVICEREMOVED:
							if (target->callbacks.joy_stick_state)
								target->callbacks.joy_stick_state((int)event.jdevice.which, false);
							break;
						case SDL_CONTROLLERAXISMOTION:
							if (target->callbacks.controller_axis_move)
								target->callbacks.controller_axis_move((int)event.caxis.which, (int)event.caxis.axis, (int)event.caxis.value);
							break;
						case SDL_CONTROLLERBUTTONDOWN:
							if (target->callbacks.controller_key_state)
								target->callbacks.controller_key_state((int)event.cbutton.which, (int)event.cbutton.button, true);
							break;
						case SDL_CONTROLLERBUTTONUP:
							if (target->callbacks.controller_key_state)
								target->callbacks.controller_key_state((int)event.cbutton.which, (int)event.cbutton.button, false);
							break;
						case SDL_CONTROLLERDEVICEADDED:
							if (target->callbacks.controller_state)
								target->callbacks.controller_state((int)event.cdevice.which, 1);
							break;
						case SDL_CONTROLLERDEVICEREMOVED:
							if (target->callbacks.controller_state)
								target->callbacks.controller_state((int)event.cdevice.which, -1);
							break;
						case SDL_CONTROLLERDEVICEREMAPPED:
							if (target->callbacks.controller_state)
								target->callbacks.controller_state((int)event.cdevice.which, 0);
							break;
						case SDL_FINGERMOTION:
							if (target->callbacks.touch_move)
								target->callbacks.touch_move((int)event.tfinger.touchId, (int)event.tfinger.fingerId, event.tfinger.x, event.tfinger.y, event.tfinger.dx, event.tfinger.dy, event.tfinger.pressure);
							break;
						case SDL_FINGERDOWN:
							if (target->callbacks.touch_state)
								target->callbacks.touch_state((int)event.tfinger.touchId, (int)event.tfinger.fingerId, event.tfinger.x, event.tfinger.y, event.tfinger.dx, event.tfinger.dy, event.tfinger.pressure, true);
							break;
						case SDL_FINGERUP:
							if (target->callbacks.touch_state)
								target->callbacks.touch_state((int)event.tfinger.touchId, (int)event.tfinger.fingerId, event.tfinger.x, event.tfinger.y, event.tfinger.dx, event.tfinger.dy, event.tfinger.pressure, false);
							break;
						case SDL_DOLLARGESTURE:
							if (target->callbacks.gesture_state)
								target->callbacks.gesture_state((int)event.dgesture.touchId, (int)event.dgesture.gestureId, (int)event.dgesture.numFingers, event.dgesture.x, event.dgesture.y, event.dgesture.error, false);
							break;
						case SDL_DOLLARRECORD:
							if (target->callbacks.gesture_state)
								target->callbacks.gesture_state((int)event.dgesture.touchId, (int)event.dgesture.gestureId, (int)event.dgesture.numFingers, event.dgesture.x, event.dgesture.y, event.dgesture.error, true);
							break;
						case SDL_MULTIGESTURE:
							if (target->callbacks.multi_gesture_state)
								target->callbacks.multi_gesture_state((int)event.mgesture.touchId, (int)event.mgesture.numFingers, event.mgesture.x, event.mgesture.y, event.mgesture.dDist, event.mgesture.dTheta);
							break;
#if SDL_VERSION_ATLEAST(2, 0, 5)
						case SDL_DROPFILE:
							if (target->callbacks.drop_file)
								target->callbacks.drop_file(event.drop.file);

							SDL_free(event.drop.file);
							break;
						case SDL_DROPTEXT:
							if (target->callbacks.drop_text)
								target->callbacks.drop_text(event.drop.file);

							SDL_free(event.drop.file);
							break;
#endif
						default:
							break;
					}
				}

				has_events = dispatch_all ? SDL_PollEvent(&event) : 0;
				++incoming_events;
			}

			if (timeout_ms > 0 || !dispatch_all)
				return incoming_events > 0;

			uint32_t timeout = sources.consumers.begin()->second->options.inactive_sleep_ms;
			for (auto& item : sources.consumers)
			{
				auto* target = item.second;
				if (target->options.render_even_if_inactive)
					return true;

				Uint32 flags = SDL_GetWindowFlags(target->handle);
				if (flags & SDL_WINDOW_MAXIMIZED || flags & SDL_WINDOW_INPUT_FOCUS || flags & SDL_WINDOW_MOUSE_FOCUS)
					return true;

				if (timeout > target->options.inactive_sleep_ms)
					timeout = target->options.inactive_sleep_ms;
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(timeout));
			return false;
#else
			return false;
#endif
		}
		bool activity::capture_key_map(key_map* value)
		{
			if (!value)
			{
				mapping.mapped = false;
				mapping.captured = false;
				mapping.enabled = false;
				return false;
			}

			if (!mapping.enabled)
			{
				mapping.mapped = false;
				mapping.captured = false;
				mapping.enabled = true;
				return false;
			}

			if (!mapping.mapped || !mapping.captured)
				return false;

			mapping.enabled = mapping.mapped = mapping.captured = false;
			memcpy(value, &mapping.key, sizeof(key_map));

			return true;
		}
		bool activity::is_fullscreen() const
		{
#ifdef VI_SDL2
			VI_ASSERT(handle != nullptr, "activity should be initialized");
			Uint32 flags = SDL_GetWindowFlags(handle);
			return flags & SDL_WINDOW_FULLSCREEN || flags & SDL_WINDOW_FULLSCREEN_DESKTOP;
#else
			return false;
#endif
		}
		bool activity::is_any_key_down() const
		{
			for (int i = 0; i < sizeof(keys[0]) / sizeof(bool); i++)
			{
				if (keys[0][i])
					return true;
			}

			return false;
		}
		bool activity::is_key_down(const key_map& key) const
		{
#ifdef VI_SDL2
			if (key.mod == key_mod::none)
				return keys[0][(size_t)key.key];

			if (key.key == key_code::none)
				return command & (int)key.mod;

			return command & (int)key.mod && keys[0][(size_t)key.key];
#else
			return keys[0][(size_t)key.key];
#endif
		}
		bool activity::is_key_up(const key_map& key) const
		{
			return !is_key_down(key);
		}
		bool activity::is_key_down_hit(const key_map& key) const
		{
#ifdef VI_SDL2
			if (key.mod == key_mod::none)
				return keys[0][(size_t)key.key] && !keys[1][(size_t)key.key];

			if (key.key == key_code::none)
				return command & (int)key.mod;

			return (command & (int)key.mod) && keys[0][(size_t)key.key] && !keys[1][(size_t)key.key];
#else
			return keys[0][(size_t)key.key] && !keys[1][(size_t)key.key];
#endif
		}
		bool activity::is_key_up_hit(const key_map& key) const
		{
#ifdef VI_SDL2
			if (key.mod == key_mod::none)
				return !keys[0][(size_t)key.key] && keys[1][(size_t)key.key];

			if (key.key == key_code::none)
				return !(command & (int)key.mod);

			return !(command & (int)key.mod) && !keys[0][(size_t)key.key] && keys[1][(size_t)key.key];
#else
			return !keys[0][(size_t)key.key] && keys[1][(size_t)key.key];
#endif
		}
		bool activity::is_screen_keyboard_enabled() const
		{
#ifdef VI_SDL2
			VI_ASSERT(handle != nullptr, "activity should be initialized");
			return SDL_IsScreenKeyboardShown(handle);
#else
			return false;
#endif
		}
		uint32_t activity::get_x() const
		{
#ifdef VI_SDL2
			VI_ASSERT(handle != nullptr, "activity should be initialized");

			int x, y;
			SDL_GetWindowPosition(handle, &x, &y);
			return x;
#else
			return 0;
#endif
		}
		uint32_t activity::get_y() const
		{
#ifdef VI_SDL2
			VI_ASSERT(handle != nullptr, "activity should be initialized");

			int x, y;
			SDL_GetWindowPosition(handle, &x, &y);
			return y;
#else
			return 0;
#endif
		}
		uint32_t activity::get_width() const
		{
#ifdef VI_SDL2
			VI_ASSERT(handle != nullptr, "activity should be initialized");

			int w, h;
			SDL_GetWindowSize(handle, &w, &h);
			return w;
#else
			return 0;
#endif
		}
		uint32_t activity::get_height() const
		{
#ifdef VI_SDL2
			VI_ASSERT(handle != nullptr, "activity should be initialized");

			int w, h;
			SDL_GetWindowSize(handle, &w, &h);
			return h;
#else
			return 0;
#endif
		}
		uint32_t activity::get_id() const
		{
#ifdef VI_SDL2
			VI_ASSERT(handle != nullptr, "activity should be initialized");
			return (uint32_t)SDL_GetWindowID(handle);
#else
			return 0;
#endif
		}
		float activity::get_aspect_ratio() const
		{
#ifdef VI_SDL2
			VI_ASSERT(handle != nullptr, "activity should be initialized");

			int w, h;
			SDL_GetWindowSize(handle, &w, &h);
			return (h > 0 ? (float)w / (float)h : 0.0f);
#else
			return 0.0f;
#endif
		}
		key_mod activity::get_key_mod_state() const
		{
#ifdef VI_SDL2
			VI_ASSERT(handle != nullptr, "activity should be initialized");
			return (key_mod)SDL_GetModState();
#else
			return key_mod::none;
#endif
		}
		viewport activity::get_viewport() const
		{
#ifdef VI_SDL2
			VI_ASSERT(handle != nullptr, "activity should be initialized");

			int w, h;
			SDL_GetWindowSize(handle, &w, &h);

			viewport id;
			id.width = (float)w;
			id.height = (float)h;
			id.min_depth = 0.0f;
			id.max_depth = 1.0f;
			id.top_left_x = 0.0f;
			id.top_left_y = 0.0f;
			return id;
#else
			return viewport();
#endif
		}
		trigonometry::vector2 activity::get_size() const
		{
#ifdef VI_SDL2
			VI_ASSERT(handle != nullptr, "activity should be initialized");

			int w, h;
			SDL_GL_GetDrawableSize(handle, &w, &h);
			return trigonometry::vector2((float)w, (float)h);
#else
			return trigonometry::vector2();
#endif
		}
		trigonometry::vector2 activity::get_client_size() const
		{
#ifdef VI_SDL2
			VI_ASSERT(handle != nullptr, "activity should be initialized");

			int w, h;
			SDL_GetWindowSize(handle, &w, &h);
			return trigonometry::vector2((float)w, (float)h);
#else
			return trigonometry::vector2();
#endif
		}
		trigonometry::vector2 activity::get_drawable_size(uint32_t width, uint32_t height) const
		{
#ifdef VI_SDL2
			VI_ASSERT(handle != nullptr, "activity should be initialized");

			int w = -1, h = -1;
			SDL_GL_GetDrawableSize(handle, &w, &h);
			return trigonometry::vector2((float)(w < 0 ? width : w), (float)(h < 0 ? height : h));
#else
			return trigonometry::vector2(width, height);
#endif
		}
		trigonometry::vector2 activity::get_offset() const
		{
#ifdef VI_SDL2
			VI_ASSERT(handle != nullptr, "activity should be initialized");

			SDL_DisplayMode display;
			SDL_GetCurrentDisplayMode(0, &display);
			trigonometry::vector2 size = get_size();
			return trigonometry::vector2((float)display.w / size.x, (float)display.h / size.y);
#else
			return trigonometry::vector2();
#endif
		}
		trigonometry::vector2 activity::get_global_cursor_position() const
		{
#ifdef VI_SDL2
			VI_ASSERT(handle != nullptr, "activity should be initialized");
#if SDL_VERSION_ATLEAST(2, 0, 4)
			int x, y;
			SDL_GetGlobalMouseState(&x, &y);
			return trigonometry::vector2((float)x, (float)y);
#else
			return trigonometry::vector2();
#endif
#else
			return trigonometry::vector2();
#endif
		}
		trigonometry::vector2 activity::get_cursor_position() const
		{
#ifdef VI_SDL2
			VI_ASSERT(handle != nullptr, "activity should be initialized");
#if SDL_VERSION_ATLEAST(2, 0, 4)
			int x, y;
			SDL_GetMouseState(&x, &y);

			return trigonometry::vector2((float)x, (float)y);
#else
			return trigonometry::vector2();
#endif
#else
			return trigonometry::vector2();
#endif
		}
		trigonometry::vector2 activity::get_cursor_position(float screen_width, float screen_height) const
		{
#ifdef VI_SDL2
			trigonometry::vector2 size = get_size();
			return get_cursor_position() * trigonometry::vector2(screen_width, screen_height) / size;
#else
			return trigonometry::vector2();
#endif
		}
		trigonometry::vector2 activity::get_cursor_position(const trigonometry::vector2& screen_dimensions) const
		{
#ifdef VI_SDL2
			trigonometry::vector2 size = get_size();
			return get_cursor_position() * screen_dimensions / size;
#else
			return trigonometry::vector2();
#endif
		}
		core::string activity::get_clipboard_text() const
		{
#ifdef VI_SDL2
			VI_ASSERT(handle != nullptr, "activity should be initialized");
			char* text = SDL_GetClipboardText();
			core::string result = (text ? text : "");

			if (text != nullptr)
				SDL_free(text);

			return result;
#else
			return core::string();
#endif
		}
		SDL_Window* activity::get_handle() const
		{
			return handle;
		}
		core::string activity::get_error() const
		{
#ifdef VI_SDL2
			VI_ASSERT(handle != nullptr, "activity should be initialized");
			const char* error = SDL_GetError();
			if (!error)
				return "";

			return error;
#else
			return "";
#endif
		}
		activity::desc& activity::get_options()
		{
			return options;
		}
		bool* activity::get_input_state()
		{
#ifdef VI_SDL2
			int count;
			auto* map = SDL_GetKeyboardState(&count);
			if (count > sizeof(keys[0]) / sizeof(bool))
				count = sizeof(keys[0]) / sizeof(bool);

			for (int i = 0; i < count; i++)
				keys[0][i] = map[i] > 0;

			Uint32 state = SDL_GetMouseState(nullptr, nullptr);
			keys[0][(size_t)key_code::cursor_left] = (state & SDL_BUTTON(SDL_BUTTON_LEFT));
			keys[0][(size_t)key_code::cursor_middle] = (state & SDL_BUTTON(SDL_BUTTON_MIDDLE));
			keys[0][(size_t)key_code::cursor_right] = (state & SDL_BUTTON(SDL_BUTTON_RIGHT));
			keys[0][(size_t)key_code::cursor_x1] = (state & SDL_BUTTON(SDL_BUTTON_X1));
			keys[0][(size_t)key_code::cursor_x2] = (state & SDL_BUTTON(SDL_BUTTON_X2));
#endif
			return keys[0];
		}

		bool alerts::text(const std::string_view& title, const std::string_view& message, const std::string_view& default_input, core::string* result)
		{
#ifdef VI_TINYFILEDIALOGS
			VI_ASSERT(core::stringify::is_cstring(title) && core::stringify::is_cstring(message), "title and message should be set");
			VI_TRACE("[tfd] open input { title: %.*s, message: %.*s }", (int)title.size(), title.data(), (int)message.size(), message.data());
			const char* data = tinyfd_inputBox(title.data(), message.data(), default_input.data());
			if (!data)
				return false;

			VI_TRACE("[tfd] close input: %s", data ? data : "NULL");
			if (result != nullptr)
				*result = data;

			return true;
#else
			return false;
#endif
		}
		bool alerts::password(const std::string_view& title, const std::string_view& message, core::string* result)
		{
#ifdef VI_TINYFILEDIALOGS
			VI_ASSERT(core::stringify::is_cstring(title) && core::stringify::is_cstring(message), "title and message should be set");
			VI_TRACE("[tfd] open password { title: %.*s, message: %.*s }", (int)title.size(), title.data(), (int)message.size(), message.data());
			const char* data = tinyfd_inputBox(title.data(), message.data(), nullptr);
			if (!data)
				return false;

			VI_TRACE("[tfd] close password: %s", data ? data : "NULL");
			if (result != nullptr)
				*result = data;

			return true;
#else
			return false;
#endif
		}
		bool alerts::save(const std::string_view& title, const std::string_view& default_path, const std::string_view& filter, const std::string_view& filter_description, core::string* result)
		{
#ifdef VI_TINYFILEDIALOGS
			VI_ASSERT(core::stringify::is_cstring(title) && core::stringify::is_cstring(default_path) && core::stringify::is_cstring(filter) && core::stringify::is_cstring(filter_description), "title, default and filter should be set");
			VI_ASSERT(filter_description.empty() || core::stringify::is_cstring(filter_description), "desc should be set");
			core::vector<core::string> sources = core::stringify::split(filter, ',');
			core::vector<char*> patterns;
			for (auto& it : sources)
				patterns.push_back((char*)it.c_str());

			VI_TRACE("[tfd] open save { title: %.*s, filter: %.*s }", (int)title.size(), title.data(), (int)filter.size(), filter.data());
			const char* data = tinyfd_saveFileDialog(title.data(), default_path.data(), (int)patterns.size(),
				patterns.empty() ? nullptr : patterns.data(), filter_description.empty() ? nullptr : filter_description.data());

			if (!data)
				return false;

			VI_TRACE("[tfd] close save: %s", data ? data : "NULL");
			if (result != nullptr)
				*result = data;

			return true;
#else
			return false;
#endif
		}
		bool alerts::open(const std::string_view& title, const std::string_view& default_path, const std::string_view& filter, const std::string_view& filter_description, bool multiple, core::string* result)
		{
#ifdef VI_TINYFILEDIALOGS
			VI_ASSERT(core::stringify::is_cstring(title) && core::stringify::is_cstring(default_path) && core::stringify::is_cstring(filter) && core::stringify::is_cstring(filter_description), "title, default and filter should be set");
			VI_ASSERT(filter_description.empty() || core::stringify::is_cstring(filter_description), "desc should be set");
			core::vector<core::string> sources = core::stringify::split(filter, ',');
			core::vector<char*> patterns;
			for (auto& it : sources)
				patterns.push_back((char*)it.c_str());

			VI_TRACE("[tfd] open load { title: %.*s, filter: %.*s }", (int)title.size(), title.data(), (int)filter.size(), filter.data());
			const char* data = tinyfd_openFileDialog(title.data(), default_path.data(), (int)patterns.size(),
				patterns.empty() ? nullptr : patterns.data(), filter_description.empty() ? nullptr : filter_description.data(), multiple);

			if (!data)
				return false;

			VI_TRACE("[tfd] close load: %s", data ? data : "NULL");
			if (result != nullptr)
				*result = data;

			return true;
#else
			return false;
#endif
		}
		bool alerts::folder(const std::string_view& title, const std::string_view& default_path, core::string* result)
		{
#ifdef VI_TINYFILEDIALOGS
			VI_ASSERT(core::stringify::is_cstring(title) && core::stringify::is_cstring(default_path), "title and default should be set");
			VI_TRACE("[tfd] open folder { title: %.*s }", (int)title.size(), title.data());
			const char* data = tinyfd_selectFolderDialog(title.data(), default_path.data());
			if (!data)
				return false;

			VI_TRACE("[tfd] close folder: %s", data ? data : "NULL");
			if (result != nullptr)
				*result = data;

			return true;
#else
			return false;
#endif
		}
		bool alerts::color(const std::string_view& title, const std::string_view& default_hex_rgb, core::string* result)
		{
#ifdef VI_TINYFILEDIALOGS
			VI_ASSERT(core::stringify::is_cstring(title) && core::stringify::is_cstring(default_hex_rgb), "title and default should be set");
			VI_TRACE("[tfd] open color { title: %.*s }", (int)title.size(), title.data());
			uint8_t RGB[3] = { 0, 0, 0 };
			const char* data = tinyfd_colorChooser(title.data(), default_hex_rgb.data(), RGB, RGB);
			if (!data)
				return false;

			VI_TRACE("[tfd] close color: %s", data ? data : "NULL");
			if (result != nullptr)
				*result = data;

			return true;
#else
			return false;
#endif
		}

		void* video::windows::get_hdc(activity* target)
		{
			VI_ASSERT(target != nullptr, "activity should be set");
#ifdef SDL_VIDEO_DRIVER_WINDOWS
			SDL_SysWMinfo info;
			target->load(&info);
			return info.info.win.hdc;
#else
			return nullptr;
#endif
		}
		void* video::windows::get_hinstance(activity* target)
		{
			VI_ASSERT(target != nullptr, "activity should be set");
#ifdef SDL_VIDEO_DRIVER_WINDOWS
			SDL_SysWMinfo info;
			target->load(&info);
			return info.info.win.hinstance;
#else
			return nullptr;
#endif
		}
		void* video::windows::get_hwnd(activity* target)
		{
			VI_ASSERT(target != nullptr, "activity should be set");
#ifdef SDL_VIDEO_DRIVER_WINDOWS
			SDL_SysWMinfo info;
			target->load(&info);
			return info.info.win.window;
#else
			return nullptr;
#endif
		}
		void* video::win_rt::get_iinspectable(activity* target)
		{
			VI_ASSERT(target != nullptr, "activity should be set");
#ifdef SDL_VIDEO_DRIVER_WINRT
			SDL_SysWMinfo info;
			target->load(&info);
			return info.info.winrt.window;
#else
			return nullptr;
#endif
		}
		void* video::x11::get_display(activity* target)
		{
			VI_ASSERT(target != nullptr, "activity should be set");
#ifdef SDL_VIDEO_DRIVER_X11
			SDL_SysWMinfo info;
			target->load(&info);
			return info.info.x11.display;
#else
			return nullptr;
#endif
		}
		size_t video::x11::get_window(activity* target)
		{
			VI_ASSERT(target != nullptr, "activity should be set");
#ifdef SDL_VIDEO_DRIVER_X11
			SDL_SysWMinfo info;
			target->load(&info);
			return (size_t)info.info.x11.window;
#else
			return 0;
#endif
		}
		void* video::direct_fb::get_idirect_fb(activity* target)
		{
			VI_ASSERT(target != nullptr, "activity should be set");
#ifdef SDL_VIDEO_DRIVER_DIRECTFB
			SDL_SysWMinfo info;
			target->load(&info);
			return (size_t)info.info.dfb.dfb;
#else
			return nullptr;
#endif
		}
		void* video::direct_fb::get_idirect_fb_window(activity* target)
		{
			VI_ASSERT(target != nullptr, "activity should be set");
#ifdef SDL_VIDEO_DRIVER_DIRECTFB
			SDL_SysWMinfo info;
			target->load(&info);
			return (size_t)info.info.dfb.window;
#else
			return nullptr;
#endif
		}
		void* video::direct_fb::get_idirect_fb_surface(activity* target)
		{
			VI_ASSERT(target != nullptr, "activity should be set");
#ifdef SDL_VIDEO_DRIVER_DIRECTFB
			SDL_SysWMinfo info;
			target->load(&info);
			return (size_t)info.info.dfb.surface;
#else
			return nullptr;
#endif
		}
		void* video::cocoa::get_ns_window(activity* target)
		{
			VI_ASSERT(target != nullptr, "activity should be set");
#ifdef SDL_VIDEO_DRIVER_COCOA
			SDL_SysWMinfo info;
			target->load(&info);
			return info.info.cocoa.window;
#else
			return nullptr;
#endif
		}
		void* video::ui_kit::get_ui_window(activity* target)
		{
			VI_ASSERT(target != nullptr, "activity should be set");
#ifdef SDL_VIDEO_DRIVER_UIKIT
			SDL_SysWMinfo info;
			target->load(&info);
			return (size_t)info.info.uikit.window;
#else
			return nullptr;
#endif
		}
		void* video::wayland::get_wl_display(activity* target)
		{
			VI_ASSERT(target != nullptr, "activity should be set");
#ifdef SDL_VIDEO_DRIVER_UIKIT
			SDL_SysWMinfo info;
			target->load(&info);
			return (size_t)info.info.wl.display;
#else
			return nullptr;
#endif
		}
		void* video::wayland::get_wl_surface(activity* target)
		{
			VI_ASSERT(target != nullptr, "activity should be set");
#ifdef SDL_VIDEO_DRIVER_UIKIT
			SDL_SysWMinfo info;
			target->load(&info);
			return (size_t)info.info.wl.surface;
#else
			return nullptr;
#endif
		}
		void* video::wayland::get_wl_egl_window(activity* target)
		{
			VI_ASSERT(target != nullptr, "activity should be set");
#ifdef SDL_VIDEO_DRIVER_UIKIT
			SDL_SysWMinfo info;
			target->load(&info);
			return (size_t)info.info.wl.egl_window;
#else
			return nullptr;
#endif
		}
		void* video::wayland::get_xdg_surface(activity* target)
		{
			VI_ASSERT(target != nullptr, "activity should be set");
#ifdef SDL_VIDEO_DRIVER_UIKIT
			SDL_SysWMinfo info;
			target->load(&info);
			return (size_t)info.info.wl.xdg_surface;
#else
			return nullptr;
#endif
		}
		void* video::wayland::get_xdg_top_level(activity* target)
		{
			VI_ASSERT(target != nullptr, "activity should be set");
#ifdef SDL_VIDEO_DRIVER_UIKIT
			SDL_SysWMinfo info;
			target->load(&info);
			return (size_t)info.info.wl.xdg_toplevel;
#else
			return nullptr;
#endif
		}
		void* video::wayland::get_xdg_popup(activity* target)
		{
			VI_ASSERT(target != nullptr, "activity should be set");
#ifdef SDL_VIDEO_DRIVER_UIKIT
			SDL_SysWMinfo info;
			target->load(&info);
			return (size_t)info.info.wl.xdg_popup;
#else
			return nullptr;
#endif
		}
		void* video::wayland::get_xdg_positioner(activity* target)
		{
			VI_ASSERT(target != nullptr, "activity should be set");
#ifdef SDL_VIDEO_DRIVER_UIKIT
			SDL_SysWMinfo info;
			target->load(&info);
			return (size_t)info.info.wl.xdg_positioner;
#else
			return nullptr;
#endif
		}
		void* video::android::get_anative_window(activity* target)
		{
			VI_ASSERT(target != nullptr, "activity should be set");
#ifdef SDL_VIDEO_DRIVER_ANDROID
			SDL_SysWMinfo info;
			target->load(&info);
			return (size_t)info.info.android.window;
#else
			return nullptr;
#endif
		}
		void* video::os2::get_hwnd(activity* target)
		{
			VI_ASSERT(target != nullptr, "activity should be set");
#ifdef SDL_VIDEO_DRIVER_OS2
			SDL_SysWMinfo info;
			target->load(&info);
			return (size_t)info.info.os2.hwnd;
#else
			return nullptr;
#endif
		}
		void* video::os2::get_hwnd_frame(activity* target)
		{
			VI_ASSERT(target != nullptr, "activity should be set");
#ifdef SDL_VIDEO_DRIVER_OS2
			SDL_SysWMinfo info;
			target->load(&info);
			return (size_t)info.info.os2.hwndFrame;
#else
			return nullptr;
#endif
		}
		bool video::glew::set_swap_interval(int32_t interval)
		{
#ifdef VI_SDL2
			return SDL_GL_SetSwapInterval(interval) == 0;
#else
			return false;
#endif
		}
		bool video::glew::set_swap_parameters(int32_t r, int32_t g, int32_t b, int32_t a, bool debugging)
		{
#ifdef VI_SDL2
			SDL_GL_SetAttribute(SDL_GL_RED_SIZE, r);
			SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, g);
			SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, b);
			SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, a);
			SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
			SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_NO_ERROR, debugging ? 0 : 1);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, debugging ? SDL_GL_CONTEXT_DEBUG_FLAG : 0);
			return true;
#else
			return false;
#endif
		}
		bool video::glew::set_context(activity* target, void* context)
		{
			VI_ASSERT(target != nullptr, "activity should be set");
#ifdef VI_SDL2
			return SDL_GL_MakeCurrent(target->get_handle(), context) == 0;
#else
			return false;
#endif
		}
		bool video::glew::perform_swap(activity* target)
		{
			VI_ASSERT(target != nullptr, "activity should be set");
#ifdef VI_SDL2
			SDL_GL_SwapWindow(target->get_handle());
			return true;
#else
			return false;
#endif
		}
		void* video::glew::create_context(activity* target)
		{
			VI_ASSERT(target != nullptr, "activity should be set");
#ifdef VI_SDL2
			return SDL_GL_CreateContext(target->get_handle());
#else
			return nullptr;
#endif
		}
		void video::glew::destroy_context(void* context)
		{
#ifdef VI_SDL2
			if (context != nullptr)
				SDL_GL_DeleteContext(context);
#endif
		}
		uint32_t video::get_display_count()
		{
#ifdef VI_SDL2
			int displays = SDL_GetNumVideoDisplays();
			return displays >= 1 ? (uint32_t)(displays - 1) : 0;
#else
			return 0;
#endif
		}
		bool video::get_display_info(uint32_t display_index, display_info* info)
		{
#ifdef VI_SDL2
			SDL_DisplayMode display;
			if (SDL_GetCurrentDisplayMode(display_index, &display) != 0)
				return false;
			else if (!info)
				return true;

			const char* name = SDL_GetDisplayName(display_index);
			if (name != nullptr)
				info->name = name;

			SDL_Rect bounds;
			if (!SDL_GetDisplayUsableBounds(display_index, &bounds))
			{
				info->x = bounds.x;
				info->y = bounds.y;
				info->width = bounds.w;
				info->height = bounds.h;
			}

			SDL_GetDisplayDPI(display_index, &info->diagonal_dpi, &info->horizontal_dpi, &info->vertical_dpi);
			info->orientation = (orientation_type)SDL_GetDisplayOrientation(display_index);
			info->pixel_format = (uint32_t)display.format;
			info->physical_width = (uint32_t)display.w;
			info->physical_height = (uint32_t)display.h;
			info->refresh_rate = (uint32_t)display.refresh_rate;
			return true;
#else
			return false;
#endif
		}
		std::string_view video::get_key_code_as_literal(key_code code)
		{
			const char* name;
			switch (code)
			{
				case key_code::g:
					name = "G";
					break;
				case key_code::h:
					name = "H";
					break;
				case key_code::i:
					name = "I";
					break;
				case key_code::j:
					name = "J";
					break;
				case key_code::k:
					name = "K";
					break;
				case key_code::l:
					name = "L";
					break;
				case key_code::m:
					name = "M";
					break;
				case key_code::n:
					name = "N";
					break;
				case key_code::o:
					name = "O";
					break;
				case key_code::p:
					name = "P";
					break;
				case key_code::q:
					name = "Q";
					break;
				case key_code::r:
					name = "R";
					break;
				case key_code::s:
					name = "S";
					break;
				case key_code::t:
					name = "T";
					break;
				case key_code::u:
					name = "U";
					break;
				case key_code::v:
					name = "V";
					break;
				case key_code::w:
					name = "W";
					break;
				case key_code::x:
					name = "X";
					break;
				case key_code::y:
					name = "Y";
					break;
				case key_code::z:
					name = "Z";
					break;
				case key_code::defer:
					name = "Return";
					break;
				case key_code::escape:
					name = "Escape";
					break;
				case key_code::left_bracket:
					name = "Left Bracket";
					break;
				case key_code::right_bracket:
					name = "Right Bracket";
					break;
				case key_code::backslash:
					name = "Backslash";
					break;
				case key_code::non_us_hash:
					name = "Non-US Hash";
					break;
				case key_code::semicolon:
					name = "Semicolon";
					break;
				case key_code::apostrophe:
					name = "Apostrophe";
					break;
				case key_code::grave:
					name = "Grave";
					break;
				case key_code::slash:
					name = "Slash";
					break;
				case key_code::capslock:
					name = "Caps Lock";
					break;
				case key_code::f1:
					name = "F1";
					break;
				case key_code::f2:
					name = "F2";
					break;
				case key_code::f3:
					name = "F3";
					break;
				case key_code::f4:
					name = "F4";
					break;
				case key_code::f5:
					name = "F5";
					break;
				case key_code::f6:
					name = "F6";
					break;
				case key_code::f7:
					name = "F7";
					break;
				case key_code::f8:
					name = "F8";
					break;
				case key_code::f9:
					name = "F9";
					break;
				case key_code::f10:
					name = "F10";
					break;
				case key_code::f11:
					name = "F11";
					break;
				case key_code::f12:
					name = "F12";
					break;
				case key_code::print_screen:
					name = "Print Screen";
					break;
				case key_code::scroll_lock:
					name = "Scroll Lock";
					break;
				case key_code::pause:
					name = "Pause";
					break;
				case key_code::insert:
					name = "Insert";
					break;
				case key_code::home:
					name = "Home";
					break;
				case key_code::page_up:
					name = "Page Up";
					break;
				case key_code::deinit:
					name = "Delete";
					break;
				case key_code::end:
					name = "End";
					break;
				case key_code::page_down:
					name = "Page Down";
					break;
				case key_code::right:
					name = "Right";
					break;
				case key_code::left:
					name = "Left";
					break;
				case key_code::down:
					name = "Down";
					break;
				case key_code::up:
					name = "Up";
					break;
				case key_code::num_lock_clear:
					name = "Numlock Clear";
					break;
				case key_code::kp_divide:
					name = "Divide";
					break;
				case key_code::kp_multiply:
					name = "Multiply";
					break;
				case key_code::minus:
				case key_code::kp_minus:
					name = "Minus";
					break;
				case key_code::kp_plus:
					name = "Plus";
					break;
				case key_code::kp_enter:
					name = "Enter";
					break;
				case key_code::d1:
				case key_code::kp1:
					name = "1";
					break;
				case key_code::d2:
				case key_code::kp2:
					name = "2";
					break;
				case key_code::d3:
				case key_code::kp3:
					name = "3";
					break;
				case key_code::d4:
				case key_code::kp4:
					name = "4";
					break;
				case key_code::d5:
				case key_code::kp5:
					name = "5";
					break;
				case key_code::d6:
				case key_code::kp6:
					name = "6";
					break;
				case key_code::d7:
				case key_code::kp7:
					name = "7";
					break;
				case key_code::d8:
				case key_code::kp8:
					name = "8";
					break;
				case key_code::d9:
				case key_code::kp9:
					name = "9";
					break;
				case key_code::d0:
				case key_code::kp0:
					name = "0";
					break;
				case key_code::period:
				case key_code::kp_period:
					name = "Period";
					break;
				case key_code::non_us_backslash:
					name = "Non-US Backslash";
					break;
				case key_code::app0:
					name = "Application";
					break;
				case key_code::equals:
				case key_code::kp_equals:
					name = "Equals";
					break;
				case key_code::f13:
					name = "F13";
					break;
				case key_code::f14:
					name = "F14";
					break;
				case key_code::f15:
					name = "F15";
					break;
				case key_code::f16:
					name = "F16";
					break;
				case key_code::f17:
					name = "F17";
					break;
				case key_code::f18:
					name = "F18";
					break;
				case key_code::f19:
					name = "F19";
					break;
				case key_code::f20:
					name = "F20";
					break;
				case key_code::f21:
					name = "F21";
					break;
				case key_code::f22:
					name = "F22";
					break;
				case key_code::f23:
					name = "F23";
					break;
				case key_code::f24:
					name = "F24";
					break;
				case key_code::execute:
					name = "Execute";
					break;
				case key_code::help:
					name = "Help";
					break;
				case key_code::menu:
					name = "Menu";
					break;
				case key_code::select:
					name = "Select";
					break;
				case key_code::stop:
					name = "Stop";
					break;
				case key_code::again:
					name = "Again";
					break;
				case key_code::undo:
					name = "Undo";
					break;
				case key_code::cut:
					name = "Cut";
					break;
				case key_code::copy:
					name = "Copy";
					break;
				case key_code::paste:
					name = "Paste";
					break;
				case key_code::find:
					name = "Find";
					break;
				case key_code::mute:
					name = "Mute";
					break;
				case key_code::volume_up:
					name = "Volume Up";
					break;
				case key_code::volume_down:
					name = "Volume Down";
					break;
				case key_code::comma:
				case key_code::kp_comma:
					name = "Comma";
					break;
				case key_code::kp_equals_as400:
					name = "Equals as 400";
					break;
				case key_code::international1:
					name = "International 1";
					break;
				case key_code::international2:
					name = "International 2";
					break;
				case key_code::international3:
					name = "International 3";
					break;
				case key_code::international4:
					name = "International 4";
					break;
				case key_code::international5:
					name = "International 5";
					break;
				case key_code::international6:
					name = "International 6";
					break;
				case key_code::international7:
					name = "International 7";
					break;
				case key_code::international8:
					name = "International 8";
					break;
				case key_code::international9:
					name = "International 9";
					break;
				case key_code::lang1:
					name = "Lang 1";
					break;
				case key_code::lang2:
					name = "Lang 2";
					break;
				case key_code::lang3:
					name = "Lang 3";
					break;
				case key_code::lang4:
					name = "Lang 4";
					break;
				case key_code::lang5:
					name = "Lang 5";
					break;
				case key_code::lang6:
					name = "Lang 6";
					break;
				case key_code::lang7:
					name = "Lang 7";
					break;
				case key_code::lang8:
					name = "Lang 8";
					break;
				case key_code::lang9:
					name = "Lang 9";
					break;
				case key_code::alterase:
					name = "Alterase";
					break;
				case key_code::sys_req:
					name = "System Request";
					break;
				case key_code::cancel:
					name = "Cancel";
					break;
				case key_code::prior:
					name = "Prior";
					break;
				case key_code::return2:
					name = "Return 2";
					break;
				case key_code::separator:
					name = "Separator";
					break;
				case key_code::output:
					name = "Output";
					break;
				case key_code::operation:
					name = "Operation";
					break;
				case key_code::clear_again:
					name = "Clear Again";
					break;
				case key_code::cr_select:
					name = "CR Select";
					break;
				case key_code::ex_select:
					name = "EX Select";
					break;
				case key_code::kp00:
					name = "00";
					break;
				case key_code::kp000:
					name = "000";
					break;
				case key_code::thousands_separator:
					name = "Thousands Separator";
					break;
				case key_code::decimals_separator:
					name = "Decimal Separator";
					break;
				case key_code::currency_unit:
					name = "Currency Unit";
					break;
				case key_code::currency_subunit:
					name = "Currency Subunit";
					break;
				case key_code::kp_left_paren:
					name = "Left Parent";
					break;
				case key_code::kp_right_paren:
					name = "Right Parent";
					break;
				case key_code::kp_left_brace:
					name = "Left Brace";
					break;
				case key_code::kp_right_brace:
					name = "Right Brace";
					break;
				case key_code::tab:
				case key_code::kp_tab:
					name = "Tab";
					break;
				case key_code::backspace:
				case key_code::kp_backspace:
					name = "Backspace";
					break;
				case key_code::a:
				case key_code::kp_a:
					name = "A";
					break;
				case key_code::b:
				case key_code::kp_b:
					name = "B";
					break;
				case key_code::c:
				case key_code::kp_c:
					name = "C";
					break;
				case key_code::d:
				case key_code::kp_d:
					name = "D";
					break;
				case key_code::e:
				case key_code::kp_e:
					name = "E";
					break;
				case key_code::f:
				case key_code::kp_f:
					name = "F";
					break;
				case key_code::kp_xor:
					name = "Xor";
					break;
				case key_code::power:
				case key_code::kp_power:
					name = "Power";
					break;
				case key_code::kp_percent:
					name = "Percent";
					break;
				case key_code::kp_less:
					name = "Less";
					break;
				case key_code::kp_greater:
					name = "Greater";
					break;
				case key_code::kp_ampersand:
					name = "Ampersand";
					break;
				case key_code::kp_dbl_ampersand:
					name = "DBL Ampersand";
					break;
				case key_code::kp_vertical_bar:
					name = "Vertical Bar";
					break;
				case key_code::kp_dbl_vertical_bar:
					name = "OBL vertical Bar";
					break;
				case key_code::kp_colon:
					name = "Colon";
					break;
				case key_code::kp_hash:
					name = "Hash";
					break;
				case key_code::space:
				case key_code::kp_space:
					name = "Space";
					break;
				case key_code::kp_at:
					name = "At";
					break;
				case key_code::kp_exclaim:
					name = "Exclam";
					break;
				case key_code::kp_mem_store:
					name = "Mem Store";
					break;
				case key_code::kp_mem_recall:
					name = "Mem Recall";
					break;
				case key_code::kp_mem_clear:
					name = "Mem Clear";
					break;
				case key_code::kp_mem_add:
					name = "Mem Add";
					break;
				case key_code::kp_mem_subtract:
					name = "Mem Subtract";
					break;
				case key_code::kp_mem_multiply:
					name = "Mem Multiply";
					break;
				case key_code::kp_mem_divide:
					name = "Mem Divide";
					break;
				case key_code::kp_plus_minus:
					name = "Plus-Minus";
					break;
				case key_code::clear:
				case key_code::kp_clear:
					name = "Clear";
					break;
				case key_code::kp_clear_entry:
					name = "Clear Entry";
					break;
				case key_code::kp_binary:
					name = "Binary";
					break;
				case key_code::kp_octal:
					name = "Octal";
					break;
				case key_code::kp_decimal:
					name = "Decimal";
					break;
				case key_code::kp_hexadecimal:
					name = "Hexadecimal";
					break;
				case key_code::left_control:
					name = "Left CTRL";
					break;
				case key_code::left_shift:
					name = "Left Shift";
					break;
				case key_code::left_alt:
					name = "Left Alt";
					break;
				case key_code::left_gui:
					name = "Left GUI";
					break;
				case key_code::right_control:
					name = "Right CTRL";
					break;
				case key_code::right_shift:
					name = "Right Shift";
					break;
				case key_code::right_alt:
					name = "Right Alt";
					break;
				case key_code::right_gui:
					name = "Right GUI";
					break;
				case key_code::mode:
					name = "Mode";
					break;
				case key_code::audio_next:
					name = "Audio Next";
					break;
				case key_code::audio_prev:
					name = "Audio Prev";
					break;
				case key_code::audio_stop:
					name = "Audio Stop";
					break;
				case key_code::audio_play:
					name = "Audio Play";
					break;
				case key_code::audio_mute:
					name = "Audio Mute";
					break;
				case key_code::media_select:
					name = "Media Select";
					break;
				case key_code::www:
					name = "WWW";
					break;
				case key_code::mail:
					name = "Mail";
					break;
				case key_code::calculator:
					name = "Calculator";
					break;
				case key_code::computer:
					name = "Computer";
					break;
				case key_code::ac_search:
					name = "AC Search";
					break;
				case key_code::ac_home:
					name = "AC Home";
					break;
				case key_code::ac_back:
					name = "AC Back";
					break;
				case key_code::ac_forward:
					name = "AC Forward";
					break;
				case key_code::ac_stop:
					name = "AC Stop";
					break;
				case key_code::ac_refresh:
					name = "AC Refresh";
					break;
				case key_code::ac_bookmarks:
					name = "AC Bookmarks";
					break;
				case key_code::brightness_down:
					name = "Brigthness Down";
					break;
				case key_code::brightness_up:
					name = "Brigthness Up";
					break;
				case key_code::display_switch:
					name = "Display Switch";
					break;
				case key_code::kb_illum_toggle:
					name = "Dillum Toggle";
					break;
				case key_code::kb_illum_down:
					name = "Dillum Down";
					break;
				case key_code::kb_illum_up:
					name = "Dillum Up";
					break;
				case key_code::eject:
					name = "Eject";
					break;
				case key_code::sleep:
					name = "Sleep";
					break;
				case key_code::app1:
					name = "App 1";
					break;
				case key_code::app2:
					name = "App 2";
					break;
				case key_code::audio_rewind:
					name = "Audio Rewind";
					break;
				case key_code::audio_fast_forward:
					name = "Audio fast Forward";
					break;
				case key_code::cursor_left:
					name = "Cursor Left";
					break;
				case key_code::cursor_middle:
					name = "Cursor Middle";
					break;
				case key_code::cursor_right:
					name = "Cursor Right";
					break;
				case key_code::cursor_x1:
					name = "Cursor X1";
					break;
				case key_code::cursor_x2:
					name = "Cursor X2";
					break;
				default:
					name = "";
					break;
			}

			return name;
		}
		std::string_view video::get_key_mod_as_literal(key_mod code)
		{
			const char* name;
			switch (code)
			{
				case key_mod::left_shift:
					name = "Left Shift";
					break;
				case key_mod::right_shift:
					name = "Right Shift";
					break;
				case key_mod::left_control:
					name = "Left Ctrl";
					break;
				case key_mod::right_control:
					name = "Right Ctrl";
					break;
				case key_mod::left_alt:
					name = "Left Alt";
					break;
				case key_mod::right_alt:
					name = "Right Alt";
					break;
				case key_mod::left_gui:
					name = "Left Gui";
					break;
				case key_mod::right_gui:
					name = "Right Gui";
					break;
				case key_mod::num:
					name = "Num-lock";
					break;
				case key_mod::caps:
					name = "Caps-lock";
					break;
				case key_mod::mode:
					name = "Mode";
					break;
				case key_mod::shift:
					name = "Shift";
					break;
				case key_mod::control:
					name = "Ctrl";
					break;
				case key_mod::alt:
					name = "Alt";
					break;
				case key_mod::gui:
					name = "Gui";
					break;
				default:
					name = "";
					break;
			}

			return name;
		}
		core::string video::get_key_code_as_string(key_code code)
		{
			return core::string(get_key_code_as_literal(code));
		}
		core::string video::get_key_mod_as_string(key_mod code)
		{
			return core::string(get_key_mod_as_literal(code));
		}
	}
}
