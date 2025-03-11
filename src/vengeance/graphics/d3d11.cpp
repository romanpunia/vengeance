#include "d3d11.h"
#ifdef VI_MICROSOFT
#include <comdef.h>
#include <d3d11sdklayers.h>
#define SHADER_VERTEX ".asm.vertex.gz"
#define SHADER_PIXEL ".asm.pixel.gz"
#define SHADER_GEOMETRY ".asm.geometry.gz"
#define SHADER_COMPUTE ".asm.compute.gz"
#define SHADER_HULL ".asm.hull.gz"
#define SHADER_DOMAIN ".asm.domain.gz"
#define REG_EXCHANGE(name, value) { if (regs.name == value) return; regs.name = value; }
#define REG_EXCHANGE_T2(name, value1, value2) { if (std::get<0>(regs.name) == value1 && std::get<1>(regs.name) == value2) return; regs.name = std::make_tuple(value1, value2); }
#define REG_EXCHANGE_T3(name, value1, value2, value3) { if (std::get<0>(regs.name) == value1 && std::get<1>(regs.name) == value2 && std::get<2>(regs.name) == value3) return; regs.name = std::make_tuple(value1, value2, value3); }
#define REG_EXCHANGE_RS(name, value1, value2, value3) { auto& __vregrs = regs.name[value2]; if (__vregrs.first == value1 && __vregrs.second == value3) return; __vregrs = std::make_pair(value1, value3); }
#define D3D_INLINE(code) #code

namespace
{
	static void d3d11_release(IUnknown* value)
	{
		if (value != nullptr)
			value->Release();
	}
	static DXGI_FORMAT get_non_depth_format(vitex::graphics::format format)
	{
		switch (format)
		{
			case vitex::graphics::format::d32_float:
				return DXGI_FORMAT_R32_FLOAT;
			case vitex::graphics::format::d16_unorm:
				return DXGI_FORMAT_R16_UNORM;
			case vitex::graphics::format::d24_unorm_s8_uint:
				return DXGI_FORMAT_R24G8_TYPELESS;
			default:
				return (DXGI_FORMAT)format;
		}
	}
	static DXGI_FORMAT get_base_depth_format(vitex::graphics::format format)
	{
		switch (format)
		{
			case vitex::graphics::format::r32_float:
			case vitex::graphics::format::d32_float:
				return DXGI_FORMAT_R32_TYPELESS;
			case vitex::graphics::format::r16_float:
			case vitex::graphics::format::d16_unorm:
				return DXGI_FORMAT_R16_TYPELESS;
			case vitex::graphics::format::d24_unorm_s8_uint:
				return DXGI_FORMAT_R24G8_TYPELESS;
			default:
				return (DXGI_FORMAT)format;
		}
	}
	static DXGI_FORMAT get_internal_depth_format(vitex::graphics::format format)
	{
		switch (format)
		{
			case vitex::graphics::format::r32_float:
			case vitex::graphics::format::d32_float:
				return DXGI_FORMAT_D32_FLOAT;
			case vitex::graphics::format::r16_float:
			case vitex::graphics::format::d16_unorm:
				return DXGI_FORMAT_D16_UNORM;
			case vitex::graphics::format::d24_unorm_s8_uint:
				return DXGI_FORMAT_D24_UNORM_S8_UINT;
			default:
				return (DXGI_FORMAT)format;
		}
	}
	static DXGI_FORMAT get_depth_format(vitex::graphics::format format)
	{
		switch (format)
		{
			case vitex::graphics::format::r32_float:
			case vitex::graphics::format::d32_float:
				return DXGI_FORMAT_R32_FLOAT;
			case vitex::graphics::format::r16_float:
				return DXGI_FORMAT_R16_FLOAT;
			case vitex::graphics::format::d16_unorm:
				return DXGI_FORMAT_R16_UNORM;
			case vitex::graphics::format::d24_unorm_s8_uint:
				return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
			default:
				return (DXGI_FORMAT)format;
		}
	}
	static vitex::graphics::graphics_exception get_exception(HRESULT result_code, const std::string_view& scope_text)
	{
		try
		{
			if (result_code != S_OK)
			{
				_com_error error(result_code);
				auto text = vitex::core::string(scope_text);
				text += " CAUSING ";
				text += error.ErrorMessage();
				return vitex::graphics::graphics_exception((int)result_code, std::move(text));
			}
			else
			{
				auto text = vitex::core::string(scope_text);
				text += " CAUSING internal graphics error";
				return vitex::graphics::graphics_exception((int)result_code, std::move(text));
			}
		}
		catch (...)
		{
			auto text = vitex::core::string(scope_text);
			text += " CAUSING internal graphics error";
			return vitex::graphics::graphics_exception((int)result_code, std::move(text));
		}
	}
	static void debug_message(D3D11_MESSAGE* message)
	{
		const char* _Source;
		switch (message->Category)
		{
			case D3D11_MESSAGE_CATEGORY_APPLICATION_DEFINED:
				_Source = "APPLICATION DEFINED";
				break;
			case D3D11_MESSAGE_CATEGORY_MISCELLANEOUS:
				_Source = "MISCELLANEOUS";
				break;
			case D3D11_MESSAGE_CATEGORY_INITIALIZATION:
				_Source = "INITIALIZATION";
				break;
			case D3D11_MESSAGE_CATEGORY_CLEANUP:
				_Source = "CLEANUP";
				break;
			case D3D11_MESSAGE_CATEGORY_COMPILATION:
				_Source = "COMPILATION";
				break;
			case D3D11_MESSAGE_CATEGORY_STATE_CREATION:
				_Source = "STATE CREATION";
				break;
			case D3D11_MESSAGE_CATEGORY_STATE_SETTING:
				_Source = "STATE STORE";
				break;
			case D3D11_MESSAGE_CATEGORY_STATE_GETTING:
				_Source = "STATE FETCH";
				break;
			case D3D11_MESSAGE_CATEGORY_RESOURCE_MANIPULATION:
				_Source = "RESOURCE MANIPULATION";
				break;
			case D3D11_MESSAGE_CATEGORY_EXECUTION:
				_Source = "EXECUTION";
				break;
			case D3D11_MESSAGE_CATEGORY_SHADER:
				_Source = "SHADER";
				break;
			default:
				_Source = "GENERAL";
				break;
		}

		switch (message->Severity)
		{
			case D3D11_MESSAGE_SEVERITY_ERROR:
			case D3D11_MESSAGE_SEVERITY_CORRUPTION:
				VI_ERR("[d3d11] %s (%d): %.*s", _Source, (int)message->ID, (int)message->DescriptionByteLength, message->pDescription);
				break;
			case D3D11_MESSAGE_SEVERITY_WARNING:
				VI_WARN("[d3d11] %s (%d): %.*s", _Source, (int)message->ID, (int)message->DescriptionByteLength, message->pDescription);
				break;
			case D3D11_MESSAGE_SEVERITY_INFO:
				VI_DEBUG("[d3d11] %s (%d): %.*s", _Source, (int)message->ID, (int)message->DescriptionByteLength, message->pDescription);
				break;
			case D3D11_MESSAGE_SEVERITY_MESSAGE:
				VI_TRACE("[d3d11] %s (%d): %.*s", _Source, (int)message->ID, (int)message->DescriptionByteLength, message->pDescription);
				break;
		}

		(void)_Source;
	}
}

namespace vitex
{
	namespace graphics
	{
		namespace d3d11
		{
			d3d11_depth_stencil_state::d3d11_depth_stencil_state(const desc& i) : depth_stencil_state(i), resource(nullptr)
			{
			}
			d3d11_depth_stencil_state::~d3d11_depth_stencil_state()
			{
				d3d11_release(resource);
			}
			void* d3d11_depth_stencil_state::get_resource() const
			{
				return resource;
			}

			d3d11_rasterizer_state::d3d11_rasterizer_state(const desc& i) : rasterizer_state(i), resource(nullptr)
			{
			}
			d3d11_rasterizer_state::~d3d11_rasterizer_state()
			{
				d3d11_release(resource);
			}
			void* d3d11_rasterizer_state::get_resource() const
			{
				return resource;
			}

			d3d11_blend_state::d3d11_blend_state(const desc& i) : blend_state(i), resource(nullptr)
			{
			}
			d3d11_blend_state::~d3d11_blend_state()
			{
				d3d11_release(resource);
			}
			void* d3d11_blend_state::get_resource() const
			{
				return resource;
			}

			d3d11_sampler_state::d3d11_sampler_state(const desc& i) : sampler_state(i), resource(nullptr)
			{
			}
			d3d11_sampler_state::~d3d11_sampler_state()
			{
				d3d11_release(resource);
			}
			void* d3d11_sampler_state::get_resource() const
			{
				return resource;
			}

			d3d11_input_layout::d3d11_input_layout(const desc& i) : input_layout(i)
			{
			}
			d3d11_input_layout::~d3d11_input_layout()
			{
			}
			void* d3d11_input_layout::get_resource() const
			{
				return (void*)this;
			}

			d3d11_shader::d3d11_shader(const desc& i) : shader(i), compiled(false)
			{
				vertex_shader = nullptr;
				pixel_shader = nullptr;
				geometry_shader = nullptr;
				hull_shader = nullptr;
				domain_shader = nullptr;
				compute_shader = nullptr;
				constant_buffer = nullptr;
				vertex_layout = nullptr;
				signature = nullptr;
			}
			d3d11_shader::~d3d11_shader()
			{
				d3d11_release(constant_buffer);
				d3d11_release(vertex_shader);
				d3d11_release(pixel_shader);
				d3d11_release(geometry_shader);
				d3d11_release(domain_shader);
				d3d11_release(hull_shader);
				d3d11_release(compute_shader);
				d3d11_release(vertex_layout);
				d3d11_release(signature);
			}
			bool d3d11_shader::is_valid() const
			{
				return compiled;
			}

			d3d11_element_buffer::d3d11_element_buffer(const desc& i) : element_buffer(i)
			{
				resource = nullptr;
				element = nullptr;
				access = nullptr;
			}
			d3d11_element_buffer::~d3d11_element_buffer()
			{
				d3d11_release(resource);
				d3d11_release(element);
				d3d11_release(access);
			}
			void* d3d11_element_buffer::get_resource() const
			{
				return (void*)element;
			}

			d3d11_mesh_buffer::d3d11_mesh_buffer(const desc& i) : mesh_buffer(i)
			{
			}
			trigonometry::vertex* d3d11_mesh_buffer::get_elements(graphics_device* device) const
			{
				VI_ASSERT(device != nullptr, "graphics device should be set");

				mapped_subresource resource;
				device->map(vertex_buffer, resource_map::write, &resource);

				trigonometry::vertex* vertices = core::memory::allocate<trigonometry::vertex>(sizeof(trigonometry::vertex) * (uint32_t)vertex_buffer->get_elements());
				memcpy(vertices, resource.pointer, (size_t)vertex_buffer->get_elements() * sizeof(trigonometry::vertex));

				device->unmap(vertex_buffer, &resource);
				return vertices;
			}

			d3d11_skin_mesh_buffer::d3d11_skin_mesh_buffer(const desc& i) : skin_mesh_buffer(i)
			{
			}
			trigonometry::skin_vertex* d3d11_skin_mesh_buffer::get_elements(graphics_device* device) const
			{
				VI_ASSERT(device != nullptr, "graphics device should be set");

				mapped_subresource resource;
				device->map(vertex_buffer, resource_map::write, &resource);

				trigonometry::skin_vertex* vertices = core::memory::allocate<trigonometry::skin_vertex>(sizeof(trigonometry::skin_vertex) * (uint32_t)vertex_buffer->get_elements());
				memcpy(vertices, resource.pointer, (size_t)vertex_buffer->get_elements() * sizeof(trigonometry::skin_vertex));

				device->unmap(vertex_buffer, &resource);
				return vertices;
			}

			d3d11_instance_buffer::d3d11_instance_buffer(const desc& i) : instance_buffer(i), resource(nullptr)
			{
			}
			d3d11_instance_buffer::~d3d11_instance_buffer()
			{
				if (device != nullptr && sync)
					device->clear_buffer(this);

				d3d11_release(resource);
			}

			d3d11_texture_2d::d3d11_texture_2d() : texture_2d(), access(nullptr), resource(nullptr), view(nullptr)
			{
			}
			d3d11_texture_2d::d3d11_texture_2d(const desc& i) : texture_2d(i), access(nullptr), resource(nullptr), view(nullptr)
			{
			}
			d3d11_texture_2d::~d3d11_texture_2d()
			{
				d3d11_release(view);
				d3d11_release(resource);
				d3d11_release(access);
			}
			void* d3d11_texture_2d::get_resource() const
			{
				return (void*)resource;
			}

			d3d11_texture_3d::d3d11_texture_3d() : texture_3d(), access(nullptr), resource(nullptr), view(nullptr)
			{
			}
			d3d11_texture_3d::~d3d11_texture_3d()
			{
				d3d11_release(view);
				d3d11_release(resource);
				d3d11_release(access);
			}
			void* d3d11_texture_3d::get_resource()
			{
				return (void*)resource;
			}

			d3d11_texture_cube::d3d11_texture_cube() : texture_cube(), access(nullptr), resource(nullptr), view(nullptr)
			{
			}
			d3d11_texture_cube::d3d11_texture_cube(const desc& i) : texture_cube(i), access(nullptr), resource(nullptr), view(nullptr)
			{
			}
			d3d11_texture_cube::~d3d11_texture_cube()
			{
				d3d11_release(view);
				d3d11_release(resource);
				d3d11_release(access);
			}
			void* d3d11_texture_cube::get_resource() const
			{
				return (void*)resource;
			}

			d3d11_depth_target_2d::d3d11_depth_target_2d(const desc& i) : depth_target_2d(i)
			{
				depth_stencil_view = nullptr;
			}
			d3d11_depth_target_2d::~d3d11_depth_target_2d()
			{
				d3d11_release(depth_stencil_view);
			}
			void* d3d11_depth_target_2d::get_resource() const
			{
				return depth_stencil_view;
			}
			uint32_t d3d11_depth_target_2d::get_width() const
			{
				return (uint32_t)viewarea.width;
			}
			uint32_t d3d11_depth_target_2d::get_height() const
			{
				return (uint32_t)viewarea.height;
			}

			d3d11_depth_target_cube::d3d11_depth_target_cube(const desc& i) : depth_target_cube(i)
			{
				depth_stencil_view = nullptr;
			}
			d3d11_depth_target_cube::~d3d11_depth_target_cube()
			{
				d3d11_release(depth_stencil_view);
			}
			void* d3d11_depth_target_cube::get_resource() const
			{
				return depth_stencil_view;
			}
			uint32_t d3d11_depth_target_cube::get_width() const
			{
				return (uint32_t)viewarea.width;
			}
			uint32_t d3d11_depth_target_cube::get_height() const
			{
				return (uint32_t)viewarea.height;
			}

			d3d11_render_target_2d::d3d11_render_target_2d(const desc& i) : render_target_2d(i)
			{
				render_target_view = nullptr;
				depth_stencil_view = nullptr;
				texture = nullptr;
			}
			d3d11_render_target_2d::~d3d11_render_target_2d()
			{
				d3d11_release(texture);
				d3d11_release(depth_stencil_view);
				d3d11_release(render_target_view);
			}
			void* d3d11_render_target_2d::get_target_buffer() const
			{
				return (void*)&render_target_view;
			}
			void* d3d11_render_target_2d::get_depth_buffer() const
			{
				return (void*)depth_stencil_view;
			}
			uint32_t d3d11_render_target_2d::get_width() const
			{
				return (uint32_t)viewarea.width;
			}
			uint32_t d3d11_render_target_2d::get_height() const
			{
				return (uint32_t)viewarea.height;
			}

			d3d11_multi_render_target_2d::d3d11_multi_render_target_2d(const desc& i) : multi_render_target_2d(i), depth_stencil_view(nullptr)
			{
				ZeroMemory(&information, sizeof(information));
				for (uint32_t i = 0; i < 8; i++)
				{
					render_target_view[i] = nullptr;
					texture[i] = nullptr;
				}
			}
			d3d11_multi_render_target_2d::~d3d11_multi_render_target_2d()
			{
				d3d11_release(depth_stencil_view);
				for (uint32_t i = 0; i < 8; i++)
				{
					d3d11_release(texture[i]);
					d3d11_release(render_target_view[i]);
				}
			}
			void* d3d11_multi_render_target_2d::get_target_buffer() const
			{
				return (void*)render_target_view;
			}
			void* d3d11_multi_render_target_2d::get_depth_buffer() const
			{
				return (void*)depth_stencil_view;
			}
			uint32_t d3d11_multi_render_target_2d::get_width() const
			{
				return (uint32_t)viewarea.width;
			}
			uint32_t d3d11_multi_render_target_2d::get_height() const
			{
				return (uint32_t)viewarea.height;
			}

			d3d11_render_target_cube::d3d11_render_target_cube(const desc& i) : render_target_cube(i)
			{
				depth_stencil_view = nullptr;
				render_target_view = nullptr;
				texture = nullptr;
			}
			d3d11_render_target_cube::~d3d11_render_target_cube()
			{
				d3d11_release(depth_stencil_view);
				d3d11_release(render_target_view);
				d3d11_release(texture);
			}
			void* d3d11_render_target_cube::get_target_buffer() const
			{
				return (void*)&render_target_view;
			}
			void* d3d11_render_target_cube::get_depth_buffer() const
			{
				return (void*)depth_stencil_view;
			}
			uint32_t d3d11_render_target_cube::get_width() const
			{
				return (uint32_t)viewarea.width;
			}
			uint32_t d3d11_render_target_cube::get_height() const
			{
				return (uint32_t)viewarea.height;
			}

			d3d11_multi_render_target_cube::d3d11_multi_render_target_cube(const desc& i) : multi_render_target_cube(i), depth_stencil_view(nullptr)
			{
				for (uint32_t i = 0; i < 8; i++)
				{
					render_target_view[i] = nullptr;
					texture[i] = nullptr;
					resource[i] = nullptr;
				}
			}
			d3d11_multi_render_target_cube::~d3d11_multi_render_target_cube()
			{
				VI_ASSERT((uint32_t)target <= 8, "targets count should be less than 9");
				for (uint32_t i = 0; i < (uint32_t)target; i++)
				{
					d3d11_release(render_target_view[i]);
					d3d11_release(texture[i]);
				}
				d3d11_release(depth_stencil_view);
			}
			void* d3d11_multi_render_target_cube::get_target_buffer() const
			{
				return (void*)render_target_view;
			}
			void* d3d11_multi_render_target_cube::get_depth_buffer() const
			{
				return (void*)depth_stencil_view;
			}
			uint32_t d3d11_multi_render_target_cube::get_width() const
			{
				return (uint32_t)viewarea.width;
			}
			uint32_t d3d11_multi_render_target_cube::get_height() const
			{
				return (uint32_t)viewarea.height;
			}

			d3d11_cubemap::d3d11_cubemap(const desc& i) : cubemap(i), merger(nullptr), source(nullptr)
			{
				VI_ASSERT(i.source != nullptr, "source should be set");
				VI_ASSERT(i.target < i.source->get_target_count(), "targets count should be less than %i", (int)i.source->get_target_count());

				d3d11_texture_2d* target = (d3d11_texture_2d*)i.source->get_target_2d(i.target);
				VI_ASSERT(target != nullptr && target->view != nullptr, "render target should be valid");

				source = target->view;
				source->GetDesc(&options.texture);
				source->AddRef();

				D3D11_TEXTURE2D_DESC& texture = options.texture;
				texture.MipLevels = i.mip_levels;
				texture.ArraySize = 6;
				texture.Usage = D3D11_USAGE_DEFAULT;
				texture.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
				texture.CPUAccessFlags = 0;
				texture.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE | D3D11_RESOURCE_MISC_GENERATE_MIPS;

				D3D11_SHADER_RESOURCE_VIEW_DESC& resource = options.resource;
				resource.Format = texture.Format;
				resource.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
				resource.TextureCube.MostDetailedMip = 0;
				resource.TextureCube.MipLevels = i.mip_levels;

				D3D11_BOX& region = options.region;
				region = { 0, 0, 0, (uint32_t)i.size, (uint32_t)i.size, 1 };
			}
			d3d11_cubemap::~d3d11_cubemap()
			{
				d3d11_release(source);
				d3d11_release(merger);
			}

			d3d11_query::d3d11_query() : query(), async(nullptr)
			{
			}
			d3d11_query::~d3d11_query()
			{
				d3d11_release(async);
			}
			void* d3d11_query::get_resource() const
			{
				return (void*)async;
			}

			d3d11_device::d3d11_device(const desc& i) : graphics_device(i), immediate_context(nullptr), context(nullptr), swap_chain(nullptr), feature_level(D3D_FEATURE_LEVEL_11_0), driver_type(D3D_DRIVER_TYPE_HARDWARE), window(i.window)
			{
				if (!window)
				{
					VI_ASSERT(virtual_window != nullptr, "cannot initialize virtual activity for device");
					window = virtual_window;
				}
				else
				{
					core::memory::release(virtual_window);
					virtual_window = window;
					virtual_window->add_ref();
				}

				if (!window->get_handle())
				{
					window->apply_configuration(backend);
					if (!window->get_handle())
						return;
				}

				uint32_t creation_flags = i.creation_flags | D3D11_CREATE_DEVICE_DISABLE_GPU_TIMEOUT;
				if (i.debug)
					creation_flags |= D3D11_CREATE_DEVICE_DEBUG;

				D3D_FEATURE_LEVEL feature_levels[] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0, D3D_FEATURE_LEVEL_9_3, D3D_FEATURE_LEVEL_9_2, D3D_FEATURE_LEVEL_9_1, };
				ZeroMemory(&swap_chain_resource, sizeof(swap_chain_resource));
				swap_chain_resource.BufferCount = 2;
				swap_chain_resource.BufferDesc.Width = i.buffer_width;
				swap_chain_resource.BufferDesc.Height = i.buffer_height;
				swap_chain_resource.BufferDesc.Format = (DXGI_FORMAT)i.buffer_format;
				swap_chain_resource.BufferDesc.RefreshRate.Numerator = 60;
				swap_chain_resource.BufferDesc.RefreshRate.Denominator = 1;
				swap_chain_resource.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
				swap_chain_resource.BufferDesc.Scaling = DXGI_MODE_SCALING_CENTERED;
				swap_chain_resource.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
				swap_chain_resource.SampleDesc.Count = 1;
				swap_chain_resource.SampleDesc.Quality = 0;
				swap_chain_resource.Windowed = i.is_windowed;

				if (i.blit_rendering)
				{
					swap_chain_resource.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
					swap_chain_resource.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
				}
				else
					swap_chain_resource.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

				if (window != nullptr)
					swap_chain_resource.OutputWindow = (HWND)video::windows::get_hwnd(window);

				try
				{
					HRESULT code = D3D11CreateDeviceAndSwapChain(nullptr, driver_type, nullptr, creation_flags, feature_levels, ARRAYSIZE(feature_levels), D3D11_SDK_VERSION, &swap_chain_resource, &swap_chain, &context, &feature_level, &immediate_context);
					VI_PANIC(code == S_OK && context != nullptr && immediate_context != nullptr && swap_chain != nullptr, "d3d11 graphics device creation failed");
				}
				catch (...)
				{
					VI_PANIC(false, "d3d11 device creation request has thrown an exception");
				}

				set_shader_model(i.shader_mode == shader_model::any ? get_supported_shader_model() : i.shader_mode);
				set_primitive_topology(primitive_topology::triangle_list);
				resize_buffers(i.buffer_width, i.buffer_height);
				create_states();
			}
			d3d11_device::~d3d11_device()
			{
				release_proxy();
				d3d11_release(immediate.vertex_shader);
				d3d11_release(immediate.vertex_layout);
				d3d11_release(immediate.constant_buffer);
				d3d11_release(immediate.pixel_shader);
				d3d11_release(immediate.vertex_buffer);
				d3d11_release(immediate_context);
				d3d11_release(swap_chain);

				if (debug)
				{
					ID3D11Debug* debugger = nullptr;
					context->QueryInterface(__uuidof(ID3D11Debug), reinterpret_cast<void**>(&debugger));
					if (debugger != nullptr)
					{
						D3D11_RLDO_FLAGS flags = (D3D11_RLDO_FLAGS)(D3D11_RLDO_DETAIL | 0x4); // D3D11_RLDO_IGNORE_INTERNAL
						debugger->ReportLiveDeviceObjects(flags);
						d3d11_release(debugger);
					}
				}

				d3d11_release(context);
			}
			void d3d11_device::set_as_current_device()
			{
			}
			void d3d11_device::set_shader_model(shader_model model)
			{
				shader_gen = model;
				if (shader_gen == shader_model::hlsl_1_0)
				{
					models.vertex = "vs_1_0";
					models.pixel = "ps_1_0";
					models.geometry = "gs_1_0";
					models.compute = "cs_1_0";
					models.domain = "ds_1_0";
					models.hull = "hs_1_0";
				}
				else if (shader_gen == shader_model::hlsl_2_0)
				{
					models.vertex = "vs_2_0";
					models.pixel = "ps_2_0";
					models.geometry = "gs_2_0";
					models.compute = "cs_2_0";
					models.domain = "ds_2_0";
					models.hull = "hs_2_0";
				}
				else if (shader_gen == shader_model::hlsl_3_0)
				{
					models.vertex = "vs_3_0";
					models.pixel = "ps_3_0";
					models.geometry = "gs_3_0";
					models.compute = "cs_3_0";
					models.domain = "ds_3_0";
					models.hull = "hs_3_0";
				}
				else if (shader_gen == shader_model::hlsl_4_0)
				{
					models.vertex = "vs_4_0";
					models.pixel = "ps_4_0";
					models.geometry = "gs_4_0";
					models.compute = "cs_4_0";
					models.domain = "ds_4_0";
					models.hull = "hs_4_0";
				}
				else if (shader_gen == shader_model::hlsl_4_1)
				{
					models.vertex = "vs_4_1";
					models.pixel = "ps_4_1";
					models.geometry = "gs_4_1";
					models.compute = "cs_4_1";
					models.domain = "ds_4_1";
					models.hull = "hs_4_1";
				}
				else if (shader_gen == shader_model::hlsl_5_0)
				{
					models.vertex = "vs_5_0";
					models.pixel = "ps_5_0";
					models.geometry = "gs_5_0";
					models.compute = "cs_5_0";
					models.domain = "ds_5_0";
					models.hull = "hs_5_0";
				}
				else
					set_shader_model(shader_model::hlsl_4_0);
			}
			void d3d11_device::set_blend_state(blend_state* state)
			{
				ID3D11BlendState* new_state = (ID3D11BlendState*)(state ? state->get_resource() : nullptr);
				REG_EXCHANGE(blend, new_state);
				immediate_context->OMSetBlendState(new_state, 0, 0xffffffff);
			}
			void d3d11_device::set_rasterizer_state(rasterizer_state* state)
			{
				ID3D11RasterizerState* new_state = (ID3D11RasterizerState*)(state ? state->get_resource() : nullptr);
				REG_EXCHANGE(rasterizer, new_state);
				immediate_context->RSSetState(new_state);
			}
			void d3d11_device::set_depth_stencil_state(depth_stencil_state* state)
			{
				ID3D11DepthStencilState* new_state = (ID3D11DepthStencilState*)(state ? state->get_resource() : nullptr);
				REG_EXCHANGE(depth_stencil, new_state);
				immediate_context->OMSetDepthStencilState(new_state, 1);
			}
			void d3d11_device::set_input_layout(input_layout* resource)
			{
				regs.layout = (d3d11_input_layout*)resource;
			}
			expects_graphics<void> d3d11_device::set_shader(shader* resource, uint32_t type)
			{
				d3d11_shader* iresource = (d3d11_shader*)resource;
				bool flush = (!iresource), update = false;

				if (type & (uint32_t)shader_type::vertex)
				{
					auto& item = regs.shaders[0];
					if (item != iresource)
					{
						immediate_context->VSSetShader(flush ? nullptr : iresource->vertex_shader, nullptr, 0);
						item = iresource;
						update = true;
					}
				}

				if (type & (uint32_t)shader_type::pixel)
				{
					auto& item = regs.shaders[1];
					if (item != iresource)
					{
						immediate_context->PSSetShader(flush ? nullptr : iresource->pixel_shader, nullptr, 0);
						item = iresource;
						update = true;
					}
				}

				if (type & (uint32_t)shader_type::geometry)
				{
					auto& item = regs.shaders[2];
					if (item != iresource)
					{
						immediate_context->GSSetShader(flush ? nullptr : iresource->geometry_shader, nullptr, 0);
						item = iresource;
						update = true;
					}
				}

				if (type & (uint32_t)shader_type::hull)
				{
					auto& item = regs.shaders[3];
					if (item != iresource)
					{
						immediate_context->HSSetShader(flush ? nullptr : iresource->hull_shader, nullptr, 0);
						item = iresource;
						update = true;
					}
				}

				if (type & (uint32_t)shader_type::domain)
				{
					auto& item = regs.shaders[4];
					if (item != iresource)
					{
						immediate_context->DSSetShader(flush ? nullptr : iresource->domain_shader, nullptr, 0);
						item = iresource;
						update = true;
					}
				}

				if (type & (uint32_t)shader_type::compute)
				{
					auto& item = regs.shaders[5];
					if (item != iresource)
					{
						immediate_context->CSSetShader(flush ? nullptr : iresource->compute_shader, nullptr, 0);
						item = iresource;
						update = true;
					}
				}

				if (!update)
					return core::expectation::met;

				if (flush)
				{
					immediate_context->IASetInputLayout(nullptr);
					return core::expectation::met;
				}

				auto new_layout = generate_input_layout(iresource);
				immediate_context->IASetInputLayout(new_layout ? *new_layout : nullptr);
				if (!new_layout)
					return new_layout.error();

				return core::expectation::met;
			}
			void d3d11_device::set_sampler_state(sampler_state* state, uint32_t slot, uint32_t count, uint32_t type)
			{
				VI_ASSERT(slot < units_size, "slot should be less than %i", (int)units_size);
				VI_ASSERT(count <= units_size && slot + count <= units_size, "count should be less than or equal %i", (int)units_size);
				if (slot == (uint32_t)-1)
					return;

				ID3D11SamplerState* new_state = (ID3D11SamplerState*)(state ? state->get_resource() : nullptr);
				REG_EXCHANGE_T3(sampler, new_state, slot, type);

				if (type & (uint32_t)shader_type::vertex)
					immediate_context->VSSetSamplers(slot, 1, &new_state);

				if (type & (uint32_t)shader_type::pixel)
					immediate_context->PSSetSamplers(slot, 1, &new_state);

				if (type & (uint32_t)shader_type::geometry)
					immediate_context->GSSetSamplers(slot, 1, &new_state);

				if (type & (uint32_t)shader_type::hull)
					immediate_context->HSSetSamplers(slot, 1, &new_state);

				if (type & (uint32_t)shader_type::domain)
					immediate_context->DSSetSamplers(slot, 1, &new_state);

				if (type & (uint32_t)shader_type::compute)
					immediate_context->CSSetSamplers(slot, 1, &new_state);
			}
			void d3d11_device::set_buffer(shader* resource, uint32_t slot, uint32_t type)
			{
				VI_ASSERT(slot < units_size, "slot should be less than %i", (int)units_size);

				ID3D11Buffer* ibuffer = (resource ? ((d3d11_shader*)resource)->constant_buffer : nullptr);
				if (type & (uint32_t)shader_type::vertex)
					immediate_context->VSSetConstantBuffers(slot, 1, &ibuffer);

				if (type & (uint32_t)shader_type::pixel)
					immediate_context->PSSetConstantBuffers(slot, 1, &ibuffer);

				if (type & (uint32_t)shader_type::geometry)
					immediate_context->GSSetConstantBuffers(slot, 1, &ibuffer);

				if (type & (uint32_t)shader_type::hull)
					immediate_context->HSSetConstantBuffers(slot, 1, &ibuffer);

				if (type & (uint32_t)shader_type::domain)
					immediate_context->DSSetConstantBuffers(slot, 1, &ibuffer);

				if (type & (uint32_t)shader_type::compute)
					immediate_context->CSSetConstantBuffers(slot, 1, &ibuffer);
			}
			void d3d11_device::set_buffer(instance_buffer* resource, uint32_t slot, uint32_t type)
			{
				VI_ASSERT(slot < units_size, "slot should be less than %i", (int)units_size);

				ID3D11ShaderResourceView* new_state = (resource ? ((d3d11_instance_buffer*)resource)->resource : nullptr);
				REG_EXCHANGE_RS(resources, new_state, slot, type);

				if (type & (uint32_t)shader_type::vertex)
					immediate_context->VSSetShaderResources(slot, 1, &new_state);

				if (type & (uint32_t)shader_type::pixel)
					immediate_context->PSSetShaderResources(slot, 1, &new_state);

				if (type & (uint32_t)shader_type::geometry)
					immediate_context->GSSetShaderResources(slot, 1, &new_state);

				if (type & (uint32_t)shader_type::hull)
					immediate_context->HSSetShaderResources(slot, 1, &new_state);

				if (type & (uint32_t)shader_type::domain)
					immediate_context->DSSetShaderResources(slot, 1, &new_state);

				if (type & (uint32_t)shader_type::compute)
					immediate_context->CSSetShaderResources(slot, 1, &new_state);
			}
			void d3d11_device::set_constant_buffer(element_buffer* resource, uint32_t slot, uint32_t type)
			{
				VI_ASSERT(slot < units_size, "slot should be less than %i", (int)units_size);

				ID3D11Buffer* ibuffer = (resource ? ((d3d11_element_buffer*)resource)->element : nullptr);
				if (type & (uint32_t)shader_type::vertex)
					immediate_context->VSSetConstantBuffers(slot, 1, &ibuffer);

				if (type & (uint32_t)shader_type::pixel)
					immediate_context->PSSetConstantBuffers(slot, 1, &ibuffer);

				if (type & (uint32_t)shader_type::geometry)
					immediate_context->GSSetConstantBuffers(slot, 1, &ibuffer);

				if (type & (uint32_t)shader_type::hull)
					immediate_context->HSSetConstantBuffers(slot, 1, &ibuffer);

				if (type & (uint32_t)shader_type::domain)
					immediate_context->DSSetConstantBuffers(slot, 1, &ibuffer);

				if (type & (uint32_t)shader_type::compute)
					immediate_context->CSSetConstantBuffers(slot, 1, &ibuffer);
			}
			void d3d11_device::set_structure_buffer(element_buffer* resource, uint32_t slot, uint32_t type)
			{
				VI_ASSERT(slot < units_size, "slot should be less than %i", (int)units_size);

				ID3D11ShaderResourceView* new_state = (resource ? ((d3d11_element_buffer*)resource)->resource : nullptr);
				REG_EXCHANGE_RS(resources, new_state, slot, type);

				if (type & (uint32_t)shader_type::vertex)
					immediate_context->VSSetShaderResources(slot, 1, &new_state);

				if (type & (uint32_t)shader_type::pixel)
					immediate_context->PSSetShaderResources(slot, 1, &new_state);

				if (type & (uint32_t)shader_type::geometry)
					immediate_context->GSSetShaderResources(slot, 1, &new_state);

				if (type & (uint32_t)shader_type::hull)
					immediate_context->HSSetShaderResources(slot, 1, &new_state);

				if (type & (uint32_t)shader_type::domain)
					immediate_context->DSSetShaderResources(slot, 1, &new_state);

				if (type & (uint32_t)shader_type::compute)
					immediate_context->CSSetShaderResources(slot, 1, &new_state);
			}
			void d3d11_device::set_texture_2d(texture_2d* resource, uint32_t slot, uint32_t type)
			{
				VI_ASSERT(slot < units_size, "slot should be less than %i", (int)units_size);

				ID3D11ShaderResourceView* new_state = (resource ? ((d3d11_texture_2d*)resource)->resource : nullptr);
				REG_EXCHANGE_RS(resources, new_state, slot, type);

				if (type & (uint32_t)shader_type::vertex)
					immediate_context->VSSetShaderResources(slot, 1, &new_state);

				if (type & (uint32_t)shader_type::pixel)
					immediate_context->PSSetShaderResources(slot, 1, &new_state);

				if (type & (uint32_t)shader_type::geometry)
					immediate_context->GSSetShaderResources(slot, 1, &new_state);

				if (type & (uint32_t)shader_type::hull)
					immediate_context->HSSetShaderResources(slot, 1, &new_state);

				if (type & (uint32_t)shader_type::domain)
					immediate_context->DSSetShaderResources(slot, 1, &new_state);

				if (type & (uint32_t)shader_type::compute)
					immediate_context->CSSetShaderResources(slot, 1, &new_state);
			}
			void d3d11_device::set_texture_3d(texture_3d* resource, uint32_t slot, uint32_t type)
			{
				VI_ASSERT(slot < units_size, "slot should be less than %i", (int)units_size);

				ID3D11ShaderResourceView* new_state = (resource ? ((d3d11_texture_3d*)resource)->resource : nullptr);
				REG_EXCHANGE_RS(resources, new_state, slot, type);

				if (type & (uint32_t)shader_type::vertex)
					immediate_context->VSSetShaderResources(slot, 1, &new_state);

				if (type & (uint32_t)shader_type::pixel)
					immediate_context->PSSetShaderResources(slot, 1, &new_state);

				if (type & (uint32_t)shader_type::geometry)
					immediate_context->GSSetShaderResources(slot, 1, &new_state);

				if (type & (uint32_t)shader_type::hull)
					immediate_context->HSSetShaderResources(slot, 1, &new_state);

				if (type & (uint32_t)shader_type::domain)
					immediate_context->DSSetShaderResources(slot, 1, &new_state);

				if (type & (uint32_t)shader_type::compute)
					immediate_context->CSSetShaderResources(slot, 1, &new_state);
			}
			void d3d11_device::set_texture_cube(texture_cube* resource, uint32_t slot, uint32_t type)
			{
				VI_ASSERT(slot < units_size, "slot should be less than %i", (int)units_size);

				ID3D11ShaderResourceView* new_state = (resource ? ((d3d11_texture_cube*)resource)->resource : nullptr);
				REG_EXCHANGE_RS(resources, new_state, slot, type);

				if (type & (uint32_t)shader_type::vertex)
					immediate_context->VSSetShaderResources(slot, 1, &new_state);

				if (type & (uint32_t)shader_type::pixel)
					immediate_context->PSSetShaderResources(slot, 1, &new_state);

				if (type & (uint32_t)shader_type::geometry)
					immediate_context->GSSetShaderResources(slot, 1, &new_state);

				if (type & (uint32_t)shader_type::hull)
					immediate_context->HSSetShaderResources(slot, 1, &new_state);

				if (type & (uint32_t)shader_type::domain)
					immediate_context->DSSetShaderResources(slot, 1, &new_state);

				if (type & (uint32_t)shader_type::compute)
					immediate_context->CSSetShaderResources(slot, 1, &new_state);
			}
			void d3d11_device::set_index_buffer(element_buffer* resource, format format_mode)
			{
				d3d11_element_buffer* iresource = (d3d11_element_buffer*)resource;
				REG_EXCHANGE_T2(index_buffer, iresource, format_mode);
				immediate_context->IASetIndexBuffer(iresource ? iresource->element : nullptr, (DXGI_FORMAT)format_mode, 0);
			}
			void d3d11_device::set_vertex_buffers(element_buffer** resources, uint32_t count, bool)
			{
				VI_ASSERT(resources != nullptr || !count, "invalid vertex buffer array pointer");
				VI_ASSERT(count <= units_size, "slot should be less than or equal to %i", (int)units_size);

				static ID3D11Buffer* ibuffers[units_size] = { nullptr };
				static uint32_t strides[units_size] = { };
				static uint32_t offsets[units_size] = { };

				for (uint32_t i = 0; i < count; i++)
				{
					d3d11_element_buffer* iresource = (d3d11_element_buffer*)resources[i];
					ibuffers[i] = (iresource ? iresource->element : nullptr);
					strides[i] = (uint32_t)(iresource ? iresource->stride : 0);
					REG_EXCHANGE_RS(vertex_buffers, iresource, i, i);
				}

				immediate_context->IASetVertexBuffers(0, count, ibuffers, strides, offsets);
			}
			void d3d11_device::set_writeable(element_buffer** resource, uint32_t slot, uint32_t count, bool computable)
			{
				VI_ASSERT(slot < 8, "slot should be less than 8");
				VI_ASSERT(count <= 8 && slot + count <= 8, "count should be less than or equal 8");
				VI_ASSERT(resource != nullptr, "buffers ptr should be set");

				ID3D11UnorderedAccessView* array[8] = { nullptr };
				for (uint32_t i = 0; i < count; i++)
					array[i] = (resource[i] ? ((d3d11_element_buffer*)(resource[i]))->access : nullptr);

				UINT offset = 0;
				if (!computable)
					immediate_context->OMSetRenderTargetsAndUnorderedAccessViews(D3D11_KEEP_RENDER_TARGETS_AND_DEPTH_STENCIL, nullptr, nullptr, slot, count, array, &offset);
				else
					immediate_context->CSSetUnorderedAccessViews(slot, count, array, &offset);
			}
			void d3d11_device::set_writeable(texture_2d** resource, uint32_t slot, uint32_t count, bool computable)
			{
				VI_ASSERT(slot < 8, "slot should be less than 8");
				VI_ASSERT(count <= 8 && slot + count <= 8, "count should be less than or equal 8");
				VI_ASSERT(resource != nullptr, "buffers ptr should be set");

				ID3D11UnorderedAccessView* array[8] = { nullptr };
				for (uint32_t i = 0; i < count; i++)
					array[i] = (resource[i] ? ((d3d11_texture_2d*)(resource[i]))->access : nullptr);

				UINT offset = 0;
				if (!computable)
					immediate_context->OMSetRenderTargetsAndUnorderedAccessViews(D3D11_KEEP_RENDER_TARGETS_AND_DEPTH_STENCIL, nullptr, nullptr, slot, count, array, &offset);
				else
					immediate_context->CSSetUnorderedAccessViews(slot, count, array, &offset);
			}
			void d3d11_device::set_writeable(texture_3d** resource, uint32_t slot, uint32_t count, bool computable)
			{
				VI_ASSERT(slot < 8, "slot should be less than 8");
				VI_ASSERT(count <= 8 && slot + count <= 8, "count should be less than or equal 8");
				VI_ASSERT(resource != nullptr, "buffers ptr should be set");

				ID3D11UnorderedAccessView* array[8] = { nullptr };
				for (uint32_t i = 0; i < count; i++)
					array[i] = (resource[i] ? ((d3d11_texture_3d*)(resource[i]))->access : nullptr);

				UINT offset = 0;
				if (!computable)
					immediate_context->OMSetRenderTargetsAndUnorderedAccessViews(D3D11_KEEP_RENDER_TARGETS_AND_DEPTH_STENCIL, nullptr, nullptr, slot, count, array, &offset);
				else
					immediate_context->CSSetUnorderedAccessViews(slot, count, array, &offset);
			}
			void d3d11_device::set_writeable(texture_cube** resource, uint32_t slot, uint32_t count, bool computable)
			{
				VI_ASSERT(slot < 8, "slot should be less than 8");
				VI_ASSERT(count <= 8 && slot + count <= 8, "count should be less than or equal 8");
				VI_ASSERT(resource != nullptr, "buffers ptr should be set");

				ID3D11UnorderedAccessView* array[8] = { nullptr };
				for (uint32_t i = 0; i < count; i++)
					array[i] = (resource[i] ? ((d3d11_texture_cube*)(resource[i]))->access : nullptr);

				UINT offset = 0;
				if (!computable)
					immediate_context->OMSetRenderTargetsAndUnorderedAccessViews(D3D11_KEEP_RENDER_TARGETS_AND_DEPTH_STENCIL, nullptr, nullptr, slot, count, array, &offset);
				else
					immediate_context->CSSetUnorderedAccessViews(slot, count, array, &offset);
			}
			void d3d11_device::set_target(float r, float g, float b)
			{
				set_target(render_target, 0, r, g, b);
			}
			void d3d11_device::set_target()
			{
				set_target(render_target, 0);
			}
			void d3d11_device::set_target(depth_target_2d* resource)
			{
				VI_ASSERT(resource != nullptr, "depth target should be set");
				d3d11_depth_target_2d* iresource = (d3d11_depth_target_2d*)resource;
				const viewport& viewarea = resource->get_viewport();
				D3D11_VIEWPORT viewport = { viewarea.top_left_x, viewarea.top_left_y, viewarea.width, viewarea.height, viewarea.min_depth, viewarea.max_depth };

				immediate_context->OMSetRenderTargets(0, nullptr, iresource->depth_stencil_view);
				immediate_context->RSSetViewports(1, &viewport);
			}
			void d3d11_device::set_target(depth_target_cube* resource)
			{
				VI_ASSERT(resource != nullptr, "depth target should be set");
				d3d11_depth_target_cube* iresource = (d3d11_depth_target_cube*)resource;
				const viewport& viewarea = resource->get_viewport();
				D3D11_VIEWPORT viewport = { viewarea.top_left_x, viewarea.top_left_y, viewarea.width, viewarea.height, viewarea.min_depth, viewarea.max_depth };

				immediate_context->OMSetRenderTargets(0, nullptr, iresource->depth_stencil_view);
				immediate_context->RSSetViewports(1, &viewport);
			}
			void d3d11_device::set_target(graphics::render_target* resource, uint32_t target, float r, float g, float b)
			{
				VI_ASSERT(resource != nullptr, "render target should be set");
				VI_ASSERT(target < resource->get_target_count(), "targets count should be less than %i", (int)resource->get_target_count());

				const viewport& viewarea = resource->get_viewport();
				ID3D11RenderTargetView** target_buffer = (ID3D11RenderTargetView**)resource->get_target_buffer();
				ID3D11DepthStencilView* depth_buffer = (ID3D11DepthStencilView*)resource->get_depth_buffer();
				D3D11_VIEWPORT viewport = { viewarea.top_left_x, viewarea.top_left_y, viewarea.width, viewarea.height, viewarea.min_depth, viewarea.max_depth };
				float clear_color[4] = { r, g, b, 0.0f };

				immediate_context->RSSetViewports(1, &viewport);
				immediate_context->OMSetRenderTargets(1, &target_buffer[target], depth_buffer);
				immediate_context->ClearRenderTargetView(target_buffer[target], clear_color);
			}
			void d3d11_device::set_target(graphics::render_target* resource, uint32_t target)
			{
				VI_ASSERT(resource != nullptr, "render target should be set");
				VI_ASSERT(target < resource->get_target_count(), "targets count should be less than %i", (int)resource->get_target_count());

				const viewport& viewarea = resource->get_viewport();
				ID3D11RenderTargetView** target_buffer = (ID3D11RenderTargetView**)resource->get_target_buffer();
				ID3D11DepthStencilView* depth_buffer = (ID3D11DepthStencilView*)resource->get_depth_buffer();
				D3D11_VIEWPORT viewport = { viewarea.top_left_x, viewarea.top_left_y, viewarea.width, viewarea.height, viewarea.min_depth, viewarea.max_depth };

				immediate_context->RSSetViewports(1, &viewport);
				immediate_context->OMSetRenderTargets(1, &target_buffer[target], depth_buffer);
			}
			void d3d11_device::set_target(graphics::render_target* resource, float r, float g, float b)
			{
				VI_ASSERT(resource != nullptr, "render target should be set");

				const viewport& viewarea = resource->get_viewport();
				ID3D11RenderTargetView** target_buffer = (ID3D11RenderTargetView**)resource->get_target_buffer();
				ID3D11DepthStencilView* depth_buffer = (ID3D11DepthStencilView*)resource->get_depth_buffer();
				D3D11_VIEWPORT viewport = { viewarea.top_left_x, viewarea.top_left_y, viewarea.width, viewarea.height, viewarea.min_depth, viewarea.max_depth };
				uint32_t count = resource->get_target_count();
				float clear_color[4] = { r, g, b, 0.0f };

				immediate_context->RSSetViewports(1, &viewport);
				immediate_context->OMSetRenderTargets(count, target_buffer, depth_buffer);

				for (uint32_t i = 0; i < count; i++)
					immediate_context->ClearRenderTargetView(target_buffer[i], clear_color);
			}
			void d3d11_device::set_target(graphics::render_target* resource)
			{
				VI_ASSERT(resource != nullptr, "render target should be set");

				const viewport& viewarea = resource->get_viewport();
				ID3D11RenderTargetView** target_buffer = (ID3D11RenderTargetView**)resource->get_target_buffer();
				ID3D11DepthStencilView* depth_buffer = (ID3D11DepthStencilView*)resource->get_depth_buffer();
				D3D11_VIEWPORT viewport = { viewarea.top_left_x, viewarea.top_left_y, viewarea.width, viewarea.height, viewarea.min_depth, viewarea.max_depth };

				immediate_context->RSSetViewports(1, &viewport);
				immediate_context->OMSetRenderTargets(resource->get_target_count(), target_buffer, depth_buffer);
			}
			void d3d11_device::set_target_map(graphics::render_target* resource, bool enabled[8])
			{
				VI_ASSERT(resource != nullptr, "render target should be set");
				VI_ASSERT(resource->get_target_count() > 1, "render target should have more than one targets");

				const viewport& viewarea = resource->get_viewport();
				ID3D11RenderTargetView** target_buffers = (ID3D11RenderTargetView**)resource->get_target_buffer();
				ID3D11DepthStencilView* depth_buffer = (ID3D11DepthStencilView*)resource->get_depth_buffer();
				D3D11_VIEWPORT viewport = { viewarea.top_left_x, viewarea.top_left_y, viewarea.width, viewarea.height, viewarea.min_depth, viewarea.max_depth };
				uint32_t count = resource->get_target_count();

				ID3D11RenderTargetView* targets[8] = { };
				for (uint32_t i = 0; i < count; i++)
					targets[i] = (enabled[i] ? target_buffers[i] : nullptr);

				immediate_context->RSSetViewports(1, &viewport);
				immediate_context->OMSetRenderTargets(count, targets, depth_buffer);
			}
			void d3d11_device::set_target_rect(uint32_t width, uint32_t height)
			{
				VI_ASSERT(width > 0 && height > 0, "width and height should be greater than zero");

				D3D11_VIEWPORT viewport = { 0, 0, (FLOAT)width, (FLOAT)height, 0, 1 };
				immediate_context->RSSetViewports(1, &viewport);
				immediate_context->OMSetRenderTargets(0, nullptr, nullptr);
			}
			void d3d11_device::set_viewports(uint32_t count, viewport* value)
			{
				VI_ASSERT(value != nullptr, "value should be set");
				VI_ASSERT(count > 0, "count should be greater than zero");

				D3D11_VIEWPORT viewports[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
				for (uint32_t i = 0; i < count; i++)
					memcpy(&viewports[i], &value[i], sizeof(viewport));

				immediate_context->RSSetViewports(count, viewports);
			}
			void d3d11_device::set_scissor_rects(uint32_t count, trigonometry::rectangle* value)
			{
				VI_ASSERT(value != nullptr, "value should be set");
				VI_ASSERT(count > 0, "count should be greater than zero");

				D3D11_RECT rects[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
				for (uint32_t i = 0; i < count; i++)
				{
					trigonometry::rectangle& from = value[i];
					D3D11_RECT& to = rects[i];
					to.left = (LONG)from.left;
					to.right = (LONG)from.right;
					to.bottom = (LONG)from.bottom;
					to.top = (LONG)from.top;
				}

				immediate_context->RSSetScissorRects(count, rects);
			}
			void d3d11_device::set_primitive_topology(primitive_topology topology)
			{
				immediate_context->IASetPrimitiveTopology((D3D11_PRIMITIVE_TOPOLOGY)topology);
			}
			void d3d11_device::flush_texture(uint32_t slot, uint32_t count, uint32_t type)
			{
				VI_ASSERT(slot < units_size, "slot should be less than %i", (int)units_size);
				VI_ASSERT(count <= units_size && slot + count <= units_size, "count should be less than or equal %i", (int)units_size);

				static ID3D11ShaderResourceView* array[units_size] = { nullptr };
				if (type & (uint32_t)shader_type::vertex)
					immediate_context->VSSetShaderResources(slot, count, array);

				if (type & (uint32_t)shader_type::pixel)
					immediate_context->PSSetShaderResources(slot, count, array);

				if (type & (uint32_t)shader_type::geometry)
					immediate_context->GSSetShaderResources(slot, count, array);

				if (type & (uint32_t)shader_type::hull)
					immediate_context->HSSetShaderResources(slot, count, array);

				if (type & (uint32_t)shader_type::domain)
					immediate_context->DSSetShaderResources(slot, count, array);

				if (type & (uint32_t)shader_type::compute)
					immediate_context->CSSetShaderResources(slot, count, array);

				size_t offset = slot + count;
				for (size_t i = slot; i < offset; i++)
					regs.resources[i] = std::make_pair<ID3D11ShaderResourceView*, uint32_t>(nullptr, 0);
			}
			void d3d11_device::flush_state()
			{
				if (immediate_context != nullptr)
					immediate_context->ClearState();
			}
			void d3d11_device::clear_buffer(instance_buffer* resource)
			{
				VI_ASSERT(resource != nullptr, "resource should be set");
				d3d11_instance_buffer* iresource = (d3d11_instance_buffer*)resource;
				if (!iresource->sync)
					return;

				d3d11_element_buffer* element = (d3d11_element_buffer*)iresource->elements;
				iresource->sync = false;

				D3D11_MAPPED_SUBRESOURCE mapped_resource;
				immediate_context->Map(element->element, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_resource);
				mapped_resource.pData = nullptr;
				immediate_context->Unmap(element->element, 0);
			}
			void d3d11_device::clear_writable(texture_2d* resource)
			{
				VI_ASSERT(resource != nullptr, "resource should be set");
				d3d11_texture_2d* iresource = (d3d11_texture_2d*)resource;

				VI_ASSERT(iresource->access != nullptr, "resource should be valid");
				UINT clear_color[4] = { 0, 0, 0, 0 };
				immediate_context->ClearUnorderedAccessViewUint(iresource->access, clear_color);
			}
			void d3d11_device::clear_writable(texture_2d* resource, float r, float g, float b)
			{
				VI_ASSERT(resource != nullptr, "resource should be set");
				d3d11_texture_2d* iresource = (d3d11_texture_2d*)resource;

				VI_ASSERT(iresource->access != nullptr, "resource should be valid");
				float clear_color[4] = { r, g, b, 0.0f };
				immediate_context->ClearUnorderedAccessViewFloat(iresource->access, clear_color);
			}
			void d3d11_device::clear_writable(texture_3d* resource)
			{
				VI_ASSERT(resource != nullptr, "resource should be set");
				d3d11_texture_3d* iresource = (d3d11_texture_3d*)resource;

				VI_ASSERT(iresource->access != nullptr, "resource should be valid");
				UINT clear_color[4] = { 0, 0, 0, 0 };
				immediate_context->ClearUnorderedAccessViewUint(iresource->access, clear_color);
			}
			void d3d11_device::clear_writable(texture_3d* resource, float r, float g, float b)
			{
				VI_ASSERT(resource != nullptr, "resource should be set");
				d3d11_texture_3d* iresource = (d3d11_texture_3d*)resource;

				VI_ASSERT(iresource->access != nullptr, "resource should be valid");
				float clear_color[4] = { r, g, b, 0.0f };
				immediate_context->ClearUnorderedAccessViewFloat(iresource->access, clear_color);
			}
			void d3d11_device::clear_writable(texture_cube* resource)
			{
				VI_ASSERT(resource != nullptr, "resource should be set");
				d3d11_texture_cube* iresource = (d3d11_texture_cube*)resource;

				VI_ASSERT(iresource->access != nullptr, "resource should be valid");
				UINT clear_color[4] = { 0, 0, 0, 0 };
				immediate_context->ClearUnorderedAccessViewUint(iresource->access, clear_color);
			}
			void d3d11_device::clear_writable(texture_cube* resource, float r, float g, float b)
			{
				VI_ASSERT(resource != nullptr, "resource should be set");
				d3d11_texture_cube* iresource = (d3d11_texture_cube*)resource;

				VI_ASSERT(iresource->access != nullptr, "resource should be valid");
				float clear_color[4] = { r, g, b, 0.0f };
				immediate_context->ClearUnorderedAccessViewFloat(iresource->access, clear_color);
			}
			void d3d11_device::clear(float r, float g, float b)
			{
				clear(render_target, 0, r, g, b);
			}
			void d3d11_device::clear(graphics::render_target* resource, uint32_t target, float r, float g, float b)
			{
				VI_ASSERT(resource != nullptr, "resource should be set");
				VI_ASSERT(target < resource->get_target_count(), "targets count should be less than %i", (int)resource->get_target_count());

				ID3D11RenderTargetView** target_buffer = (ID3D11RenderTargetView**)resource->get_target_buffer();
				float clear_color[4] = { r, g, b, 0.0f };

				immediate_context->ClearRenderTargetView(target_buffer[target], clear_color);
			}
			void d3d11_device::clear_depth()
			{
				clear_depth(render_target);
			}
			void d3d11_device::clear_depth(depth_target_2d* resource)
			{
				VI_ASSERT(resource != nullptr, "resource should be set");
				d3d11_depth_target_2d* iresource = (d3d11_depth_target_2d*)resource;
				immediate_context->ClearDepthStencilView(iresource->depth_stencil_view, D3D11_CLEAR_DEPTH, 1.0f, 0);
			}
			void d3d11_device::clear_depth(depth_target_cube* resource)
			{
				VI_ASSERT(resource != nullptr, "resource should be set");
				d3d11_depth_target_cube* iresource = (d3d11_depth_target_cube*)resource;
				immediate_context->ClearDepthStencilView(iresource->depth_stencil_view, D3D11_CLEAR_DEPTH, 1.0f, 0);
			}
			void d3d11_device::clear_depth(graphics::render_target* resource)
			{
				VI_ASSERT(resource != nullptr, "resource should be set");
				ID3D11DepthStencilView* depth_buffer = (ID3D11DepthStencilView*)resource->get_depth_buffer();
				immediate_context->ClearDepthStencilView(depth_buffer, D3D11_CLEAR_DEPTH, 1.0f, 0);
			}
			void d3d11_device::draw_indexed(uint32_t count, uint32_t index_location, uint32_t base_location)
			{
				immediate_context->DrawIndexed(count, index_location, base_location);
			}
			void d3d11_device::draw_indexed(mesh_buffer* resource)
			{
				VI_ASSERT(resource != nullptr, "resource should be set");
				d3d11_element_buffer* vertex_buffer = (d3d11_element_buffer*)resource->get_vertex_buffer();
				d3d11_element_buffer* index_buffer = (d3d11_element_buffer*)resource->get_index_buffer();
				uint32_t stride = (uint32_t)vertex_buffer->stride, offset = 0;

				if (regs.vertex_buffers[0].first != vertex_buffer)
				{
					regs.vertex_buffers[0] = std::make_pair(vertex_buffer, 0);
					immediate_context->IASetVertexBuffers(0, 1, &vertex_buffer->element, &stride, &offset);
				}

				if (std::get<0>(regs.index_buffer) != index_buffer || std::get<1>(regs.index_buffer) != format::r32_uint)
				{
					regs.index_buffer = std::make_tuple(index_buffer, format::r32_uint);
					immediate_context->IASetIndexBuffer(index_buffer->element, DXGI_FORMAT_R32_UINT, 0);
				}

				immediate_context->DrawIndexed((uint32_t)index_buffer->get_elements(), 0, 0);
			}
			void d3d11_device::draw_indexed(skin_mesh_buffer* resource)
			{
				VI_ASSERT(resource != nullptr, "resource should be set");
				d3d11_element_buffer* vertex_buffer = (d3d11_element_buffer*)resource->get_vertex_buffer();
				d3d11_element_buffer* index_buffer = (d3d11_element_buffer*)resource->get_index_buffer();
				uint32_t stride = (uint32_t)vertex_buffer->stride, offset = 0;

				if (regs.vertex_buffers[0].first != vertex_buffer)
				{
					regs.vertex_buffers[0] = std::make_pair(vertex_buffer, 0);
					immediate_context->IASetVertexBuffers(0, 1, &vertex_buffer->element, &stride, &offset);
				}

				if (std::get<0>(regs.index_buffer) != index_buffer || std::get<1>(regs.index_buffer) != format::r32_uint)
				{
					regs.index_buffer = std::make_tuple(index_buffer, format::r32_uint);
					immediate_context->IASetIndexBuffer(index_buffer->element, DXGI_FORMAT_R32_UINT, 0);
				}

				immediate_context->DrawIndexed((uint32_t)index_buffer->get_elements(), 0, 0);
			}
			void d3d11_device::draw_indexed_instanced(uint32_t index_count_per_instance, uint32_t instance_count, uint32_t index_location, uint32_t vertex_location, uint32_t instance_location)
			{
				immediate_context->DrawIndexedInstanced(index_count_per_instance, instance_count, index_location, vertex_location, instance_location);
			}
			void d3d11_device::draw_indexed_instanced(element_buffer* instances, mesh_buffer* resource, uint32_t instance_count)
			{
				VI_ASSERT(instances != nullptr, "instances should be set");
				VI_ASSERT(resource != nullptr, "resource should be set");

				d3d11_element_buffer* instance_buffer = (d3d11_element_buffer*)instances;
				d3d11_element_buffer* vertex_buffer = (d3d11_element_buffer*)resource->get_vertex_buffer();
				d3d11_element_buffer* index_buffer = (d3d11_element_buffer*)resource->get_index_buffer();
				uint32_t stride = (uint32_t)vertex_buffer->stride, offset = 0;

				if (regs.vertex_buffers[0].first != vertex_buffer)
				{
					regs.vertex_buffers[0] = std::make_pair(vertex_buffer, 0);
					immediate_context->IASetVertexBuffers(0, 1, &vertex_buffer->element, &stride, &offset);
				}

				if (std::get<0>(regs.index_buffer) != index_buffer || std::get<1>(regs.index_buffer) != format::r32_uint)
				{
					regs.index_buffer = std::make_tuple(index_buffer, format::r32_uint);
					immediate_context->IASetIndexBuffer(index_buffer->element, DXGI_FORMAT_R32_UINT, 0);
				}

				stride = (uint32_t)instance_buffer->stride;
				immediate_context->IASetVertexBuffers(1, 1, &instance_buffer->element, &stride, &offset);
				immediate_context->DrawIndexedInstanced((uint32_t)index_buffer->get_elements(), instance_count, 0, 0, 0);
			}
			void d3d11_device::draw_indexed_instanced(element_buffer* instances, skin_mesh_buffer* resource, uint32_t instance_count)
			{
				VI_ASSERT(instances != nullptr, "instances should be set");
				VI_ASSERT(resource != nullptr, "resource should be set");

				d3d11_element_buffer* instance_buffer = (d3d11_element_buffer*)instances;
				d3d11_element_buffer* vertex_buffer = (d3d11_element_buffer*)resource->get_vertex_buffer();
				d3d11_element_buffer* index_buffer = (d3d11_element_buffer*)resource->get_index_buffer();
				uint32_t stride = (uint32_t)vertex_buffer->stride, offset = 0;

				if (regs.vertex_buffers[0].first != vertex_buffer)
				{
					regs.vertex_buffers[0] = std::make_pair(vertex_buffer, 0);
					immediate_context->IASetVertexBuffers(0, 1, &vertex_buffer->element, &stride, &offset);
				}

				if (std::get<0>(regs.index_buffer) != index_buffer || std::get<1>(regs.index_buffer) != format::r32_uint)
				{
					regs.index_buffer = std::make_tuple(index_buffer, format::r32_uint);
					immediate_context->IASetIndexBuffer(index_buffer->element, DXGI_FORMAT_R32_UINT, 0);
				}

				stride = (uint32_t)instance_buffer->stride;
				immediate_context->IASetVertexBuffers(1, 1, &instance_buffer->element, &stride, &offset);
				immediate_context->DrawIndexedInstanced((uint32_t)index_buffer->get_elements(), instance_count, 0, 0, 0);
			}
			void d3d11_device::draw(uint32_t count, uint32_t location)
			{
				immediate_context->Draw(count, location);
			}
			void d3d11_device::draw_instanced(uint32_t vertex_count_per_instance, uint32_t instance_count, uint32_t vertex_location, uint32_t instance_location)
			{
				immediate_context->DrawInstanced(vertex_count_per_instance, instance_count, vertex_location, instance_location);
			}
			void d3d11_device::dispatch(uint32_t group_x, uint32_t group_y, uint32_t group_z)
			{
				immediate_context->Dispatch(group_x, group_y, group_z);
			}
			void d3d11_device::get_viewports(uint32_t* count, viewport* out)
			{
				D3D11_VIEWPORT viewports[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
				UINT view_count = (count ? *count : 1);

				immediate_context->RSGetViewports(&view_count, viewports);
				if (!view_count || !out)
					return;

				for (UINT i = 0; i < view_count; i++)
					memcpy(&out[i], &viewports[i], sizeof(D3D11_VIEWPORT));
			}
			void d3d11_device::get_scissor_rects(uint32_t* count, trigonometry::rectangle* out)
			{
				D3D11_RECT rects[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
				UINT rect_count = (count ? *count : 1);

				immediate_context->RSGetScissorRects(&rect_count, rects);
				if (!out)
					return;

				if (count != nullptr)
					*count = rect_count;

				for (UINT i = 0; i < rect_count; i++)
					memcpy(&out[i], &rects[i], sizeof(D3D11_RECT));
			}
			void d3d11_device::query_begin(query* resource)
			{
				VI_ASSERT(resource != nullptr, "resource should be set");
				d3d11_query* iresource = (d3d11_query*)resource;

				VI_ASSERT(iresource->async != nullptr, "resource should be valid");
				immediate_context->Begin(iresource->async);
			}
			void d3d11_device::query_end(query* resource)
			{
				VI_ASSERT(resource != nullptr, "resource should be set");
				d3d11_query* iresource = (d3d11_query*)resource;

				VI_ASSERT(iresource->async != nullptr, "resource should be valid");
				immediate_context->End(iresource->async);
			}
			void d3d11_device::generate_mips(texture_2d* resource)
			{
				VI_ASSERT(resource != nullptr, "resource should be set");
				d3d11_texture_2d* iresource = (d3d11_texture_2d*)resource;

				VI_ASSERT(iresource->resource != nullptr, "resource should be valid");
				immediate_context->GenerateMips(iresource->resource);
			}
			void d3d11_device::generate_mips(texture_3d* resource)
			{
				VI_ASSERT(resource != nullptr, "resource should be set");
				d3d11_texture_3d* iresource = (d3d11_texture_3d*)resource;

				VI_ASSERT(iresource->resource != nullptr, "resource should be valid");
				immediate_context->GenerateMips(iresource->resource);
			}
			void d3d11_device::generate_mips(texture_cube* resource)
			{
				VI_ASSERT(resource != nullptr, "resource should be set");
				d3d11_texture_cube* iresource = (d3d11_texture_cube*)resource;

				VI_ASSERT(iresource->resource != nullptr, "resource should be valid");
				immediate_context->GenerateMips(iresource->resource);
			}
			bool d3d11_device::im_begin()
			{
				if (!immediate.vertex_buffer && !create_direct_buffer(0))
					return false;

				primitives = primitive_topology::triangle_list;
				direct.transform = trigonometry::matrix4x4::identity();
				direct.padding = { 0, 0, 0, 1 };
				view_resource = nullptr;

				elements.clear();
				return true;
			}
			void d3d11_device::im_transform(const trigonometry::matrix4x4& transform)
			{
				direct.transform = direct.transform * transform;
			}
			void d3d11_device::im_topology(primitive_topology topology)
			{
				primitives = topology;
			}
			void d3d11_device::im_emit()
			{
				elements.push_back({ 0, 0, 0, 0, 0, 1, 1, 1, 1 });
			}
			void d3d11_device::im_texture(texture_2d* in)
			{
				view_resource = in;
				direct.padding.z = (in != nullptr);
			}
			void d3d11_device::im_color(float x, float y, float z, float w)
			{
				VI_ASSERT(!elements.empty(), "vertex should already be emitted");
				auto& element = elements.back();
				element.cx = x;
				element.cy = y;
				element.cz = z;
				element.cw = w;
			}
			void d3d11_device::im_intensity(float intensity)
			{
				direct.padding.w = intensity;
			}
			void d3d11_device::im_texcoord(float x, float y)
			{
				VI_ASSERT(!elements.empty(), "vertex should already be emitted");
				auto& element = elements.back();
				element.tx = x;
				element.ty = y;
			}
			void d3d11_device::im_texcoord_offset(float x, float y)
			{
				direct.padding.x = x;
				direct.padding.y = y;
			}
			void d3d11_device::im_position(float x, float y, float z)
			{
				VI_ASSERT(!elements.empty(), "vertex should already be emitted");
				auto& element = elements.back();
				element.px = x;
				element.py = y;
				element.pz = z;
			}
			bool d3d11_device::im_end()
			{
				if (elements.size() > max_elements && !create_direct_buffer(elements.size()))
					return false;

				uint32_t stride = sizeof(vertex), offset = 0;
				uint32_t last_stride = 0, last_offset = 0;
				D3D11_MAPPED_SUBRESOURCE mapped_resource;
				immediate_context->Map(immediate.vertex_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_resource);
				memcpy(mapped_resource.pData, elements.data(), (size_t)elements.size() * sizeof(vertex));
				immediate_context->Unmap(immediate.vertex_buffer, 0);

				D3D11_PRIMITIVE_TOPOLOGY last_topology;
				immediate_context->IAGetPrimitiveTopology(&last_topology);
				immediate_context->IASetPrimitiveTopology((D3D11_PRIMITIVE_TOPOLOGY)primitives);

				ID3D11InputLayout* last_layout;
				immediate_context->IAGetInputLayout(&last_layout);
				immediate_context->IASetInputLayout(immediate.vertex_layout);

				ID3D11VertexShader* last_vertex_shader;
				immediate_context->VSGetShader(&last_vertex_shader, nullptr, nullptr);
				immediate_context->VSSetShader(immediate.vertex_shader, nullptr, 0);

				ID3D11PixelShader* last_pixel_shader;
				immediate_context->PSGetShader(&last_pixel_shader, nullptr, nullptr);
				immediate_context->PSSetShader(immediate.pixel_shader, nullptr, 0);

				ID3D11Buffer* last_vertex_buffer;
				immediate_context->IAGetVertexBuffers(0, 1, &last_vertex_buffer, &last_stride, &last_offset);
				immediate_context->IASetVertexBuffers(0, 1, &immediate.vertex_buffer, &stride, &offset);

				ID3D11Buffer* last_buffer1;
				ID3D11Buffer* last_buffer2;
				immediate_context->VSGetConstantBuffers(0, 1, &last_buffer1);
				immediate_context->VSSetConstantBuffers(0, 1, &immediate.constant_buffer);
				immediate_context->PSGetConstantBuffers(0, 1, &last_buffer2);
				immediate_context->PSSetConstantBuffers(0, 1, &immediate.constant_buffer);

				ID3D11SamplerState* last_sampler;
				immediate_context->PSGetSamplers(1, 1, &last_sampler);
				immediate_context->PSSetSamplers(1, 1, &immediate.sampler);

				ID3D11ShaderResourceView* null_texture = nullptr;
				ID3D11ShaderResourceView* last_texture;
				immediate_context->PSGetShaderResources(1, 1, &last_texture);
				immediate_context->PSSetShaderResources(1, 1, view_resource ? &((d3d11_texture_2d*)view_resource)->resource : &null_texture);

				immediate_context->UpdateSubresource(immediate.constant_buffer, 0, nullptr, &direct, 0, 0);
				immediate_context->Draw((uint32_t)elements.size(), 0);
				immediate_context->IASetPrimitiveTopology(last_topology);
				immediate_context->IASetInputLayout(last_layout);
				immediate_context->VSSetShader(last_vertex_shader, nullptr, 0);
				immediate_context->VSSetConstantBuffers(0, 1, &last_buffer1);
				immediate_context->PSSetShader(last_pixel_shader, nullptr, 0);
				immediate_context->PSSetConstantBuffers(0, 1, &last_buffer2);
				immediate_context->PSSetSamplers(1, 1, &last_sampler);
				immediate_context->PSSetShaderResources(1, 1, &last_texture);
				immediate_context->IASetVertexBuffers(0, 1, &last_vertex_buffer, &last_stride, &last_offset);
				d3d11_release(last_texture);
				d3d11_release(last_sampler);
				d3d11_release(last_buffer1);
				d3d11_release(last_buffer2);
				d3d11_release(last_vertex_buffer);
				d3d11_release(last_pixel_shader);
				d3d11_release(last_vertex_shader);
				d3d11_release(last_layout);
				return true;
			}
			bool d3d11_device::has_explicit_slots() const
			{
				return true;
			}
			expects_graphics<uint32_t> d3d11_device::get_shader_slot(shader* resource, const std::string_view& name) const
			{
				VI_ASSERT(core::stringify::is_cstring(name), "name should be set");
				VI_ASSERT(resource != nullptr, "resource should be set");
				d3d11_shader* iresource = (d3d11_shader*)resource;
				static auto resolve_slot = [](ID3D11ShaderReflection* reflection, const std::string_view& name) -> core::option<uint32_t>
				{
					if (!reflection || name.empty())
						return core::optional::none;

					D3D11_SHADER_INPUT_BIND_DESC input_desc;
					if (reflection->GetResourceBindingDescByName(name.data(), &input_desc) == S_OK)
					{
						auto slot = (uint32_t)input_desc.BindPoint;
						if (slot != std::numeric_limits<uint32_t>::max())
							return slot;
					}

					return core::optional::none;
				};

				auto result = resolve_slot(iresource->reflection.vertex_shader, name);
				if (result)
					return *result;

				result = resolve_slot(iresource->reflection.pixel_shader, name);
				if (result)
					return *result;

				result = resolve_slot(iresource->reflection.geometry_shader, name);
				if (result)
					return *result;

				result = resolve_slot(iresource->reflection.domain_shader, name);
				if (result)
					return *result;

				result = resolve_slot(iresource->reflection.hull_shader, name);
				if (result)
					return *result;

				result = resolve_slot(iresource->reflection.compute_shader, name);
				if (result)
					return *result;

				return graphics_exception(-1, core::stringify::text("shader slot for variable %s not found", name.data()));
			}
			expects_graphics<uint32_t> d3d11_device::get_shader_sampler_slot(shader* resource, const std::string_view& resource_name, const std::string_view& sampler_name) const
			{
				return get_shader_slot(resource, sampler_name);
			}
			expects_graphics<void> d3d11_device::submit()
			{
				HRESULT result_code = swap_chain->Present((uint32_t)vsync_mode, present_flags);
				dispatch_queue();
				if (debug)
				{
					ID3D11InfoQueue* debugger = nullptr;
					context->QueryInterface(__uuidof(ID3D11InfoQueue), reinterpret_cast<void**>(&debugger));
					if (debugger != nullptr)
					{
						core::uptr<D3D11_MESSAGE> message;
						SIZE_T current_message_size = 0;
						UINT64 messages = debugger->GetNumStoredMessages();
						for (UINT64 i = 0; i < messages; i++)
						{
							SIZE_T next_message_size = 0;
							if (debugger->GetMessage(i, nullptr, &next_message_size) != S_OK)
								continue;

							if (current_message_size < next_message_size)
							{
								current_message_size = next_message_size;
								message = core::memory::allocate<D3D11_MESSAGE>((size_t)current_message_size);
							}

							if (debugger->GetMessage(i, *message, &next_message_size) == S_OK)
								debug_message(*message);
						}

						debugger->ClearStoredMessages();
						debugger->Release();
					}
				}
				if (result_code != S_OK)
					return get_exception(result_code, "swap chain present");

				return core::expectation::met;
			}
			expects_graphics<void> d3d11_device::map(element_buffer* resource, resource_map mode, mapped_subresource* map)
			{
				VI_ASSERT(resource != nullptr, "resource should be set");
				VI_ASSERT(map != nullptr, "map should be set");

				d3d11_element_buffer* iresource = (d3d11_element_buffer*)resource;
				D3D11_MAPPED_SUBRESOURCE mapped_resource;
				HRESULT result_code = immediate_context->Map(iresource->element, 0, (D3D11_MAP)mode, 0, &mapped_resource);
				if (result_code != S_OK)
					return get_exception(result_code, "map element buffer");

				map->pointer = mapped_resource.pData;
				map->row_pitch = mapped_resource.RowPitch;
				map->depth_pitch = mapped_resource.DepthPitch;
				return core::expectation::met;
			}
			expects_graphics<void> d3d11_device::map(texture_2d* resource, resource_map mode, mapped_subresource* map)
			{
				VI_ASSERT(resource != nullptr, "resource should be set");
				VI_ASSERT(map != nullptr, "map should be set");

				d3d11_texture_2d* iresource = (d3d11_texture_2d*)resource;
				D3D11_MAPPED_SUBRESOURCE mapped_resource;
				HRESULT result_code = immediate_context->Map(iresource->view, 0, (D3D11_MAP)mode, 0, &mapped_resource);
				if (result_code != S_OK)
					return get_exception(result_code, "map texture 2d");

				map->pointer = mapped_resource.pData;
				map->row_pitch = mapped_resource.RowPitch;
				map->depth_pitch = mapped_resource.DepthPitch;
				return core::expectation::met;
			}
			expects_graphics<void> d3d11_device::map(texture_3d* resource, resource_map mode, mapped_subresource* map)
			{
				VI_ASSERT(resource != nullptr, "resource should be set");
				VI_ASSERT(map != nullptr, "map should be set");

				d3d11_texture_3d* iresource = (d3d11_texture_3d*)resource;
				D3D11_MAPPED_SUBRESOURCE mapped_resource;
				HRESULT result_code = immediate_context->Map(iresource->view, 0, (D3D11_MAP)mode, 0, &mapped_resource);
				if (result_code != S_OK)
					return get_exception(result_code, "map texture 3d");

				map->pointer = mapped_resource.pData;
				map->row_pitch = mapped_resource.RowPitch;
				map->depth_pitch = mapped_resource.DepthPitch;
				return core::expectation::met;
			}
			expects_graphics<void> d3d11_device::map(texture_cube* resource, resource_map mode, mapped_subresource* map)
			{
				VI_ASSERT(resource != nullptr, "resource should be set");
				VI_ASSERT(map != nullptr, "map should be set");

				d3d11_texture_cube* iresource = (d3d11_texture_cube*)resource;
				D3D11_MAPPED_SUBRESOURCE mapped_resource;
				HRESULT result_code = immediate_context->Map(iresource->view, 0, (D3D11_MAP)mode, 0, &mapped_resource);
				if (result_code != S_OK)
					return get_exception(result_code, "map texture cube");

				map->pointer = mapped_resource.pData;
				map->row_pitch = mapped_resource.RowPitch;
				map->depth_pitch = mapped_resource.DepthPitch;
				return core::expectation::met;
			}
			expects_graphics<void> d3d11_device::unmap(texture_2d* resource, mapped_subresource* map)
			{
				VI_ASSERT(resource != nullptr, "resource should be set");
				VI_ASSERT(map != nullptr, "map should be set");

				d3d11_texture_2d* iresource = (d3d11_texture_2d*)resource;
				immediate_context->Unmap(iresource->view, 0);
				return core::expectation::met;
			}
			expects_graphics<void> d3d11_device::unmap(texture_3d* resource, mapped_subresource* map)
			{
				VI_ASSERT(resource != nullptr, "resource should be set");
				VI_ASSERT(map != nullptr, "map should be set");

				d3d11_texture_3d* iresource = (d3d11_texture_3d*)resource;
				immediate_context->Unmap(iresource->view, 0);
				return core::expectation::met;
			}
			expects_graphics<void> d3d11_device::unmap(texture_cube* resource, mapped_subresource* map)
			{
				VI_ASSERT(resource != nullptr, "resource should be set");
				VI_ASSERT(map != nullptr, "map should be set");

				d3d11_texture_cube* iresource = (d3d11_texture_cube*)resource;
				immediate_context->Unmap(iresource->view, 0);
				return core::expectation::met;
			}
			expects_graphics<void> d3d11_device::unmap(element_buffer* resource, mapped_subresource* map)
			{
				VI_ASSERT(resource != nullptr, "resource should be set");
				VI_ASSERT(map != nullptr, "map should be set");

				d3d11_element_buffer* iresource = (d3d11_element_buffer*)resource;
				immediate_context->Unmap(iresource->element, 0);
				return core::expectation::met;
			}
			expects_graphics<void> d3d11_device::update_constant_buffer(element_buffer* resource, void* data, size_t size)
			{
				VI_ASSERT(resource != nullptr, "resource should be set");
				VI_ASSERT(data != nullptr, "data should be set");

				d3d11_element_buffer* iresource = (d3d11_element_buffer*)resource;
				immediate_context->UpdateSubresource(iresource->element, 0, nullptr, data, 0, 0);
				return core::expectation::met;
			}
			expects_graphics<void> d3d11_device::update_buffer(element_buffer* resource, void* data, size_t size)
			{
				VI_ASSERT(resource != nullptr, "resource should be set");
				VI_ASSERT(data != nullptr, "data should be set");

				d3d11_element_buffer* iresource = (d3d11_element_buffer*)resource;
				D3D11_MAPPED_SUBRESOURCE mapped_resource;
				HRESULT result_code = immediate_context->Map(iresource->element, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_resource);
				if (result_code != S_OK)
					return get_exception(result_code, "map element buffer");

				memcpy(mapped_resource.pData, data, size);
				immediate_context->Unmap(iresource->element, 0);
				return core::expectation::met;
			}
			expects_graphics<void> d3d11_device::update_buffer(shader* resource, const void* data)
			{
				VI_ASSERT(resource != nullptr, "resource should be set");
				VI_ASSERT(data != nullptr, "data should be set");

				d3d11_shader* iresource = (d3d11_shader*)resource;
				immediate_context->UpdateSubresource(iresource->constant_buffer, 0, nullptr, data, 0, 0);
				return core::expectation::met;
			}
			expects_graphics<void> d3d11_device::update_buffer(mesh_buffer* resource, trigonometry::vertex* data)
			{
				VI_ASSERT(resource != nullptr, "resource should be set");
				VI_ASSERT(data != nullptr, "data should be set");

				d3d11_mesh_buffer* iresource = (d3d11_mesh_buffer*)resource;
				mapped_subresource mapped_resource;
				auto map_status = map(iresource->vertex_buffer, resource_map::write, &mapped_resource);
				if (!map_status)
					return map_status;

				memcpy(mapped_resource.pointer, data, (size_t)iresource->vertex_buffer->get_elements() * sizeof(trigonometry::vertex));
				return unmap(iresource->vertex_buffer, &mapped_resource);
			}
			expects_graphics<void> d3d11_device::update_buffer(skin_mesh_buffer* resource, trigonometry::skin_vertex* data)
			{
				VI_ASSERT(resource != nullptr, "resource should be set");
				VI_ASSERT(data != nullptr, "data should be set");

				d3d11_skin_mesh_buffer* iresource = (d3d11_skin_mesh_buffer*)resource;
				mapped_subresource mapped_resource;
				auto map_status = map(iresource->vertex_buffer, resource_map::write, &mapped_resource);
				if (!map_status)
					return map_status;

				memcpy(mapped_resource.pointer, data, (size_t)iresource->vertex_buffer->get_elements() * sizeof(trigonometry::skin_vertex));
				return unmap(iresource->vertex_buffer, &mapped_resource);
			}
			expects_graphics<void> d3d11_device::update_buffer(instance_buffer* resource)
			{
				VI_ASSERT(resource != nullptr, "resource should be set");
				d3d11_instance_buffer* iresource = (d3d11_instance_buffer*)resource;
				if (iresource->array.empty() || iresource->array.size() > iresource->element_limit)
					return graphics_exception("instance buffer mapping error: invalid array size");

				d3d11_element_buffer* element = (d3d11_element_buffer*)iresource->elements;
				iresource->sync = true;

				D3D11_MAPPED_SUBRESOURCE mapped_resource;
				HRESULT result_code = immediate_context->Map(element->element, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_resource);
				if (result_code != S_OK)
					return get_exception(result_code, "map instance buffer");

				memcpy(mapped_resource.pData, iresource->array.data(), (size_t)iresource->array.size() * iresource->element_width);
				immediate_context->Unmap(element->element, 0);
				return core::expectation::met;
			}
			expects_graphics<void> d3d11_device::update_buffer_size(shader* resource, size_t size)
			{
				VI_ASSERT(resource != nullptr, "resource should be set");
				VI_ASSERT(size > 0, "size should be greater than zero");

				d3d11_shader* iresource = (d3d11_shader*)resource;
				d3d11_release(iresource->constant_buffer);
				HRESULT result_code = create_constant_buffer(&iresource->constant_buffer, size);
				if (result_code != S_OK)
					return get_exception(result_code, "map shader buffer");

				return core::expectation::met;
			}
			expects_graphics<void> d3d11_device::update_buffer_size(instance_buffer* resource, size_t size)
			{
				VI_ASSERT(resource != nullptr, "resource should be set");
				VI_ASSERT(size > 0, "size should be greater than zero");

				d3d11_instance_buffer* iresource = (d3d11_instance_buffer*)resource;
				clear_buffer(iresource);
				core::memory::release(iresource->elements);
				d3d11_release(iresource->resource);
				iresource->element_limit = size;
				iresource->array.clear();
				iresource->array.reserve(iresource->element_limit);

				element_buffer::desc f = element_buffer::desc();
				f.access_flags = cpu_access::write;
				f.misc_flags = resource_misc::buffer_structured;
				f.usage = resource_usage::dynamic;
				f.bind_flags = resource_bind::shader_input;
				f.element_count = (uint32_t)iresource->element_limit;
				f.element_width = (uint32_t)iresource->element_width;
				f.structure_byte_stride = f.element_width;

				auto buffer = create_element_buffer(f);
				if (!buffer)
					return buffer.error();

				iresource->elements = *buffer;

				D3D11_SHADER_RESOURCE_VIEW_DESC SRV;
				ZeroMemory(&SRV, sizeof(SRV));
				SRV.Format = DXGI_FORMAT_UNKNOWN;
				SRV.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
				SRV.Buffer.ElementWidth = (uint32_t)iresource->element_limit;

				HRESULT result_code = context->CreateShaderResourceView(((d3d11_element_buffer*)iresource->elements)->element, &SRV, &iresource->resource);
				if (result_code != S_OK)
					return get_exception(result_code, "create shader resource view for instance buffer");

				return core::expectation::met;
			}
			expects_graphics<void> d3d11_device::copy_texture_2d(texture_2d* resource, texture_2d** result)
			{
				VI_ASSERT(resource != nullptr, "resource should be set");
				VI_ASSERT(result != nullptr, "result should be set");
				d3d11_texture_2d* iresource = (d3d11_texture_2d*)resource;

				VI_ASSERT(iresource->view != nullptr, "resource should be valid");
				D3D11_TEXTURE2D_DESC information;
				iresource->view->GetDesc(&information);
				information.BindFlags |= D3D11_BIND_SHADER_RESOURCE;

				d3d11_texture_2d* texture = (d3d11_texture_2d*)*result;
				if (!texture)
				{
					auto new_texture = create_texture_2d();
					if (!new_texture)
						return new_texture.error();

					texture = (d3d11_texture_2d*)*new_texture;
					HRESULT result_code = context->CreateTexture2D(&information, nullptr, &texture->view);
					if (result_code != S_OK)
						return get_exception(result_code, "create texture 2d for copy");
				}

				*result = texture;
				immediate_context->CopyResource(texture->view, iresource->view);
				return generate_texture(texture);
			}
			expects_graphics<void> d3d11_device::copy_texture_2d(graphics::render_target* resource, uint32_t target, texture_2d** result)
			{
				VI_ASSERT(resource != nullptr, "resource should be set");
				VI_ASSERT(result != nullptr, "result should be set");

				d3d11_texture_2d* source = (d3d11_texture_2d*)resource->get_target_2d(target);
				VI_ASSERT(source != nullptr, "source should be set");
				VI_ASSERT(source->view != nullptr, "source should be valid");

				D3D11_TEXTURE2D_DESC information;
				source->view->GetDesc(&information);
				information.BindFlags |= D3D11_BIND_SHADER_RESOURCE;

				d3d11_texture_2d* texture = (d3d11_texture_2d*)*result;
				if (!texture)
				{
					auto new_texture = create_texture_2d();
					if (!new_texture)
						return new_texture.error();

					texture = (d3d11_texture_2d*)*new_texture;
					HRESULT result_code = context->CreateTexture2D(&information, nullptr, &texture->view);
					if (result_code != S_OK)
						return get_exception(result_code, "create texture 2d for copy");
				}

				*result = texture;
				immediate_context->CopyResource(texture->view, source->view);
				return generate_texture(texture);
			}
			expects_graphics<void> d3d11_device::copy_texture_2d(render_target_cube* resource, trigonometry::cube_face face, texture_2d** result)
			{
				VI_ASSERT(resource != nullptr, "resource should be set");
				VI_ASSERT(result != nullptr, "result should be set");

				d3d11_render_target_cube* iresource = (d3d11_render_target_cube*)resource;

				VI_ASSERT(iresource->texture != nullptr, "resource should be valid");
				D3D11_TEXTURE2D_DESC information;
				iresource->texture->GetDesc(&information);
				information.BindFlags |= D3D11_BIND_SHADER_RESOURCE;

				d3d11_texture_2d* texture = (d3d11_texture_2d*)*result;
				if (!texture)
				{
					auto new_texture = create_texture_2d();
					if (!new_texture)
						return new_texture.error();

					texture = (d3d11_texture_2d*)*new_texture;
					HRESULT result_code = context->CreateTexture2D(&information, nullptr, &texture->view);
					if (result_code != S_OK)
						return get_exception(result_code, "create texture 2d for copy");
				}

				*result = texture;
				immediate_context->CopySubresourceRegion(texture->view, (uint32_t)face * information.MipLevels, 0, 0, 0, iresource->texture, 0, 0);
				return generate_texture(texture);
			}
			expects_graphics<void> d3d11_device::copy_texture_2d(multi_render_target_cube* resource, uint32_t cube, trigonometry::cube_face face, texture_2d** result)
			{
				VI_ASSERT(resource != nullptr, "resource should be set");
				VI_ASSERT(result != nullptr, "result should be set");

				d3d11_multi_render_target_cube* iresource = (d3d11_multi_render_target_cube*)resource;
				VI_ASSERT(iresource->texture[cube] != nullptr, "source should be set");
				VI_ASSERT(cube < (uint32_t)iresource->target, "cube index should be less than %i", (int)iresource->target);

				D3D11_TEXTURE2D_DESC information;
				iresource->texture[cube]->GetDesc(&information);
				information.BindFlags |= D3D11_BIND_SHADER_RESOURCE;

				d3d11_texture_2d* texture = (d3d11_texture_2d*)*result;
				if (!texture)
				{
					auto new_texture = create_texture_2d();
					if (!new_texture)
						return new_texture.error();

					texture = (d3d11_texture_2d*)*new_texture;
					HRESULT result_code = context->CreateTexture2D(&information, nullptr, &texture->view);
					if (result_code != S_OK)
						return get_exception(result_code, "create texture 2d for copy");
				}

				*result = texture;
				immediate_context->CopySubresourceRegion(texture->view, (uint32_t)face * information.MipLevels, 0, 0, 0, iresource->texture[cube], 0, 0);
				return generate_texture(texture);
			}
			expects_graphics<void> d3d11_device::copy_texture_cube(render_target_cube* resource, texture_cube** result)
			{
				VI_ASSERT(resource != nullptr, "resource should be set");
				VI_ASSERT(result != nullptr, "result should be set");

				d3d11_render_target_cube* iresource = (d3d11_render_target_cube*)resource;
				VI_ASSERT(iresource->texture != nullptr, "resource should be valid");
				D3D11_TEXTURE2D_DESC information;
				iresource->texture->GetDesc(&information);
				information.BindFlags |= D3D11_BIND_SHADER_RESOURCE;

				void* resources[6] = { nullptr };
				for (uint32_t i = 0; i < 6; i++)
				{
					ID3D11Texture2D* subresource;
					HRESULT result_code = context->CreateTexture2D(&information, nullptr, &subresource);
					if (result_code != S_OK)
					{
						for (uint32_t j = 0; j < 6; j++)
						{
							ID3D11Texture2D* src = (ID3D11Texture2D*)resources[j];
							d3d11_release(src);
						}

						return get_exception(result_code, "create texture 2d for copy");
					}
					else
					{
						immediate_context->CopySubresourceRegion(subresource, i, 0, 0, 0, iresource->texture, 0, 0);
						resources[i] = subresource;
					}
				}

				auto new_texture = create_texture_cube_internal(resources);
				if (!new_texture)
				{
					core::memory::release(*result);
					return new_texture.error();
				}

				core::memory::release(*result);
				*result = *new_texture;
				return core::expectation::met;
			}
			expects_graphics<void> d3d11_device::copy_texture_cube(multi_render_target_cube* resource, uint32_t cube, texture_cube** result)
			{
				VI_ASSERT(resource != nullptr, "resource should be set");
				VI_ASSERT(result != nullptr, "result should be set");

				d3d11_multi_render_target_cube* iresource = (d3d11_multi_render_target_cube*)resource;
				VI_ASSERT(iresource->texture[cube] != nullptr, "source should be set");
				VI_ASSERT(cube < (uint32_t)iresource->target, "cube index should be less than %i", (int)iresource->target);

				D3D11_TEXTURE2D_DESC information;
				iresource->texture[cube]->GetDesc(&information);
				information.BindFlags |= D3D11_BIND_SHADER_RESOURCE;

				void* resources[6] = { nullptr };
				for (uint32_t i = 0; i < 6; i++)
				{
					ID3D11Texture2D* subresource;
					HRESULT result_code = context->CreateTexture2D(&information, nullptr, &subresource);
					if (result_code != S_OK)
					{
						for (uint32_t j = 0; j < 6; j++)
						{
							ID3D11Texture2D* src = (ID3D11Texture2D*)resources[j];
							d3d11_release(src);
						}

						return get_exception(result_code, "create texture 2d for copy");
					}
					else
					{
						immediate_context->CopySubresourceRegion(subresource, i, 0, 0, 0, iresource->texture[cube], 0, 0);
						resources[i] = subresource;
					}
				}

				auto new_texture = create_texture_cube_internal(resources);
				core::memory::release(*result);
				if (!new_texture)
					return new_texture.error();

				*result = *new_texture;
				return core::expectation::met;
			}
			expects_graphics<void> d3d11_device::copy_target(graphics::render_target* from, uint32_t from_target, graphics::render_target* to, uint32_t to_target)
			{
				VI_ASSERT(from != nullptr, "from should be set");
				VI_ASSERT(to != nullptr, "to should be set");

				d3d11_texture_2d* source_2d = (d3d11_texture_2d*)from->get_target_2d(from_target);
				d3d11_texture_cube* source_cube = (d3d11_texture_cube*)from->get_target_cube(from_target);
				d3d11_texture_2d* dest_2d = (d3d11_texture_2d*)to->get_target_2d(to_target);
				d3d11_texture_cube* dest_cube = (d3d11_texture_cube*)to->get_target_cube(to_target);
				ID3D11Texture2D* source = (source_2d ? source_2d->view : (source_cube ? source_cube->view : nullptr));
				ID3D11Texture2D* dest = (dest_2d ? dest_2d->view : (dest_cube ? dest_cube->view : nullptr));

				VI_ASSERT(source != nullptr, "from should be set");
				VI_ASSERT(dest != nullptr, "to should be set");

				immediate_context->CopyResource(dest, source);
				return core::expectation::met;
			}
			expects_graphics<void> d3d11_device::copy_back_buffer(texture_2d** result)
			{
				VI_ASSERT(result != nullptr, "result should be set");

				ID3D11Texture2D* back_buffer = nullptr;
				HRESULT result_code = swap_chain->GetBuffer(0, __uuidof(back_buffer), reinterpret_cast<void**>(&back_buffer));
				if (result_code != S_OK)
					return get_exception(result_code, "fetch swap chain for copy");

				D3D11_TEXTURE2D_DESC information;
				back_buffer->GetDesc(&information);
				information.BindFlags = D3D11_BIND_SHADER_RESOURCE;

				d3d11_texture_2d* texture = (d3d11_texture_2d*)*result;
				if (!texture)
				{
					auto new_texture = create_texture_2d();
					if (!new_texture)
						return new_texture.error();

					texture = (d3d11_texture_2d*)*new_texture;
					if (swap_chain_resource.SampleDesc.Count > 1)
					{
						information.SampleDesc.Count = 1;
						information.SampleDesc.Quality = 0;
					}

					HRESULT result_code = context->CreateTexture2D(&information, nullptr, &texture->view);
					if (result_code != S_OK)
						return get_exception(result_code, "create texture 2d for copy");
				}

				if (swap_chain_resource.SampleDesc.Count > 1)
					immediate_context->ResolveSubresource(texture->view, 0, back_buffer, 0, DXGI_FORMAT_R8G8B8A8_UNORM);
				else
					immediate_context->CopyResource(texture->view, back_buffer);

				d3d11_release(back_buffer);
				return generate_texture(texture);
			}
			expects_graphics<void> d3d11_device::cubemap_push(cubemap* resource, texture_cube* result)
			{
				VI_ASSERT(resource != nullptr, "resource should be set");
				VI_ASSERT(resource->is_valid(), "resource should be valid");
				VI_ASSERT(result != nullptr, "result should be set");

				d3d11_cubemap* iresource = (d3d11_cubemap*)resource;
				d3d11_texture_cube* dest = (d3d11_texture_cube*)result;
				iresource->dest = dest;

				if (dest->view != nullptr && dest->resource != nullptr)
					return core::expectation::met;

				d3d11_release(dest->view);
				HRESULT result_code = context->CreateTexture2D(&iresource->options.texture, nullptr, &dest->view);
				if (result_code != S_OK)
					return get_exception(result_code, "create texture cube for cubemap");

				d3d11_release(dest->resource);
				result_code = context->CreateShaderResourceView(dest->view, &iresource->options.resource, &dest->resource);
				if (result_code != S_OK)
					return get_exception(result_code, "create texture resource for cubemap");

				return generate_texture(dest);
			}
			expects_graphics<void> d3d11_device::cubemap_face(cubemap* resource, trigonometry::cube_face face)
			{
				VI_ASSERT(resource != nullptr, "resource should be set");
				VI_ASSERT(resource->is_valid(), "resource should be valid");

				d3d11_cubemap* iresource = (d3d11_cubemap*)resource;
				d3d11_texture_cube* dest = (d3d11_texture_cube*)iresource->dest;

				VI_ASSERT(iresource->dest != nullptr, "result should be set");
				immediate_context->CopyResource(iresource->merger, iresource->source);
				immediate_context->CopySubresourceRegion(dest->view, (uint32_t)face * iresource->meta.mip_levels, 0, 0, 0, iresource->merger, 0, &iresource->options.region);
				return core::expectation::met;
			}
			expects_graphics<void> d3d11_device::cubemap_pop(cubemap* resource)
			{
				VI_ASSERT(resource != nullptr, "resource should be set");
				VI_ASSERT(resource->is_valid(), "resource should be valid");

				d3d11_cubemap* iresource = (d3d11_cubemap*)resource;
				d3d11_texture_cube* dest = (d3d11_texture_cube*)iresource->dest;

				VI_ASSERT(iresource->dest != nullptr, "result should be set");
				if (iresource->meta.mip_levels > 0)
					immediate_context->GenerateMips(dest->resource);

				return core::expectation::met;
			}
			expects_graphics<void> d3d11_device::rescale_buffers(uint32_t width, uint32_t height)
			{
				VI_ASSERT(virtual_window != nullptr, "window should be set");
				auto size = virtual_window->get_drawable_size(width, height);
				return resize_buffers((uint32_t)size.x, (uint32_t)size.y);
			}
			expects_graphics<void> d3d11_device::resize_buffers(uint32_t width, uint32_t height)
			{
				if (render_target != nullptr)
				{
					immediate_context->OMSetRenderTargets(0, 0, 0);
					core::memory::release(render_target);

					DXGI_SWAP_CHAIN_DESC info;
					HRESULT result_code = swap_chain->GetDesc(&info);
					if (result_code != S_OK)
						return get_exception(result_code, "resize buffers");

					result_code = swap_chain->ResizeBuffers(2, width, height, info.BufferDesc.Format, info.Flags);
					if (result_code != S_OK)
						return get_exception(result_code, "resize buffers");
				}

				ID3D11Texture2D* back_buffer = nullptr;
				HRESULT result_code = swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&back_buffer);
				if (result_code != S_OK)
					return get_exception(result_code, "resize buffers");

				render_target_2d::desc f = render_target_2d::desc();
				f.width = width;
				f.height = height;
				f.mip_levels = 1;
				f.misc_flags = resource_misc::none;
				f.format_mode = format::r8g8b8a8_unorm;
				f.usage = resource_usage::defaults;
				f.access_flags = cpu_access::none;
				f.bind_flags = resource_bind::render_target | resource_bind::shader_input;
				f.render_surface = (void*)back_buffer;

				auto new_target = create_render_target_2d(f);
				if (!new_target)
					return new_target.error();

				render_target = *new_target;
				set_target(render_target, 0);
				d3d11_release(back_buffer);
				return core::expectation::met;
			}
			expects_graphics<void> d3d11_device::generate_texture(texture_2d* resource)
			{
				return create_texture_2d(resource, DXGI_FORMAT_UNKNOWN);
			}
			expects_graphics<void> d3d11_device::generate_texture(texture_3d* resource)
			{
				VI_ASSERT(resource != nullptr, "resource should be set");
				d3d11_texture_3d* iresource = (d3d11_texture_3d*)resource;

				VI_ASSERT(iresource->view != nullptr, "resource should be valid");
				D3D11_TEXTURE3D_DESC description;
				iresource->view->GetDesc(&description);
				iresource->format_mode = (format)description.Format;
				iresource->usage = (resource_usage)description.Usage;
				iresource->width = description.Width;
				iresource->height = description.Height;
				iresource->mip_levels = description.MipLevels;
				iresource->access_flags = (cpu_access)description.CPUAccessFlags;
				iresource->binding = (resource_bind)description.BindFlags;

				if (!((uint32_t)iresource->binding & (uint32_t)resource_bind::shader_input))
					return core::expectation::met;

				D3D11_SHADER_RESOURCE_VIEW_DESC SRV;
				ZeroMemory(&SRV, sizeof(SRV));
				SRV.Format = description.Format;
				SRV.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
				SRV.Texture3D.MostDetailedMip = 0;
				SRV.Texture3D.MipLevels = description.MipLevels;

				d3d11_release(iresource->resource);
				HRESULT result_code = context->CreateShaderResourceView(iresource->view, &SRV, &iresource->resource);
				if (result_code != S_OK)
					return get_exception(result_code, "generate texture");

				return core::expectation::met;
			}
			expects_graphics<void> d3d11_device::generate_texture(texture_cube* resource)
			{
				VI_ASSERT(resource != nullptr, "resource should be set");
				d3d11_texture_cube* iresource = (d3d11_texture_cube*)resource;

				VI_ASSERT(iresource->view != nullptr, "resource should be valid");
				D3D11_TEXTURE2D_DESC description;
				iresource->view->GetDesc(&description);
				iresource->format_mode = (format)description.Format;
				iresource->usage = (resource_usage)description.Usage;
				iresource->width = description.Width;
				iresource->height = description.Height;
				iresource->mip_levels = description.MipLevels;
				iresource->access_flags = (cpu_access)description.CPUAccessFlags;
				iresource->binding = (resource_bind)description.BindFlags;

				if (!((uint32_t)iresource->binding & (uint32_t)resource_bind::shader_input))
					return core::expectation::met;

				D3D11_SHADER_RESOURCE_VIEW_DESC SRV;
				ZeroMemory(&SRV, sizeof(SRV));
				SRV.Format = description.Format;
				SRV.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
				SRV.TextureCube.MostDetailedMip = 0;
				SRV.TextureCube.MipLevels = description.MipLevels;

				d3d11_release(iresource->resource);
				HRESULT result_code = context->CreateShaderResourceView(iresource->view, &SRV, &iresource->resource);
				if (result_code != S_OK)
					return get_exception(result_code, "generate texture");

				return core::expectation::met;
			}
			expects_graphics<void> d3d11_device::get_query_data(query* resource, size_t* result, bool flush)
			{
				VI_ASSERT(resource != nullptr, "resource should be set");
				VI_ASSERT(result != nullptr, "result should be set");

				d3d11_query* iresource = (d3d11_query*)resource;
				uint64_t passing = 0;

				VI_ASSERT(iresource->async != nullptr, "resource should be valid");
				HRESULT result_code = immediate_context->GetData(iresource->async, &passing, sizeof(uint64_t), !flush ? D3D11_ASYNC_GETDATA_DONOTFLUSH : 0);
				*result = (size_t)passing;
				if (result_code != S_OK)
					return get_exception(result_code, "query data");

				return core::expectation::met;
			}
			expects_graphics<void> d3d11_device::get_query_data(query* resource, bool* result, bool flush)
			{
				VI_ASSERT(resource != nullptr, "resource should be set");
				VI_ASSERT(result != nullptr, "result should be set");

				d3d11_query* iresource = (d3d11_query*)resource;
				VI_ASSERT(iresource->async != nullptr, "resource should be valid");
				HRESULT result_code = immediate_context->GetData(iresource->async, result, sizeof(bool), !flush ? D3D11_ASYNC_GETDATA_DONOTFLUSH : 0);
				if (result_code != S_OK)
					return get_exception(result_code, "query data");

				return core::expectation::met;
			}
			expects_graphics<depth_stencil_state*> d3d11_device::create_depth_stencil_state(const depth_stencil_state::desc& i)
			{
				D3D11_DEPTH_STENCIL_DESC state;
				state.BackFace.StencilDepthFailOp = (D3D11_STENCIL_OP)i.back_face_stencil_depth_fail_operation;
				state.BackFace.StencilFailOp = (D3D11_STENCIL_OP)i.back_face_stencil_fail_operation;
				state.BackFace.StencilFunc = (D3D11_COMPARISON_FUNC)i.back_face_stencil_function;
				state.BackFace.StencilPassOp = (D3D11_STENCIL_OP)i.back_face_stencil_pass_operation;
				state.FrontFace.StencilDepthFailOp = (D3D11_STENCIL_OP)i.front_face_stencil_depth_fail_operation;
				state.FrontFace.StencilFailOp = (D3D11_STENCIL_OP)i.front_face_stencil_fail_operation;
				state.FrontFace.StencilFunc = (D3D11_COMPARISON_FUNC)i.front_face_stencil_function;
				state.FrontFace.StencilPassOp = (D3D11_STENCIL_OP)i.front_face_stencil_pass_operation;
				state.DepthEnable = i.depth_enable;
				state.DepthFunc = (D3D11_COMPARISON_FUNC)i.depth_function;
				state.DepthWriteMask = (D3D11_DEPTH_WRITE_MASK)i.depth_write_mask;
				state.StencilEnable = i.stencil_enable;
				state.StencilReadMask = i.stencil_read_mask;
				state.StencilWriteMask = i.stencil_write_mask;

				core::uptr<ID3D11DepthStencilState> device_state;
				HRESULT result_code = context->CreateDepthStencilState(&state, device_state.out());
				if (result_code != S_OK)
					return get_exception(result_code, "create depth stencil state");

				d3d11_depth_stencil_state* result = new d3d11_depth_stencil_state(i);
				result->resource = device_state.reset();
				return result;
			}
			expects_graphics<blend_state*> d3d11_device::create_blend_state(const blend_state::desc& i)
			{
				D3D11_BLEND_DESC state;
				state.AlphaToCoverageEnable = i.alpha_to_coverage_enable;
				state.IndependentBlendEnable = i.independent_blend_enable;
				for (uint32_t j = 0; j < 8; j++)
				{
					state.RenderTarget[j].BlendEnable = i.render_target[j].blend_enable;
					state.RenderTarget[j].BlendOp = (D3D11_BLEND_OP)i.render_target[j].blend_operation_mode;
					state.RenderTarget[j].BlendOpAlpha = (D3D11_BLEND_OP)i.render_target[j].blend_operation_alpha;
					state.RenderTarget[j].DestBlend = (D3D11_BLEND)i.render_target[j].dest_blend;
					state.RenderTarget[j].DestBlendAlpha = (D3D11_BLEND)i.render_target[j].dest_blend_alpha;
					state.RenderTarget[j].RenderTargetWriteMask = i.render_target[j].render_target_write_mask;
					state.RenderTarget[j].SrcBlend = (D3D11_BLEND)i.render_target[j].src_blend;
					state.RenderTarget[j].SrcBlendAlpha = (D3D11_BLEND)i.render_target[j].src_blend_alpha;
				}

				core::uptr<ID3D11BlendState> device_state;
				HRESULT result_code = context->CreateBlendState(&state, device_state.out());
				if (result_code != S_OK)
					return get_exception(result_code, "create blend state");

				d3d11_blend_state* result = new d3d11_blend_state(i);
				result->resource = device_state.reset();
				return result;
			}
			expects_graphics<rasterizer_state*> d3d11_device::create_rasterizer_state(const rasterizer_state::desc& i)
			{
				D3D11_RASTERIZER_DESC state;
				state.AntialiasedLineEnable = i.antialiased_line_enable;
				state.CullMode = (D3D11_CULL_MODE)i.cull_mode;
				state.DepthBias = i.depth_bias;
				state.DepthBiasClamp = i.depth_bias_clamp;
				state.DepthClipEnable = i.depth_clip_enable;
				state.FillMode = (D3D11_FILL_MODE)i.fill_mode;
				state.FrontCounterClockwise = i.front_counter_clockwise;
				state.MultisampleEnable = i.multisample_enable;
				state.ScissorEnable = i.scissor_enable;
				state.SlopeScaledDepthBias = i.slope_scaled_depth_bias;

				core::uptr<ID3D11RasterizerState> device_state;
				HRESULT result_code = context->CreateRasterizerState(&state, device_state.out());
				if (result_code != S_OK)
					return get_exception(result_code, "create rasterizer state");

				d3d11_rasterizer_state* result = new d3d11_rasterizer_state(i);
				result->resource = device_state.reset();
				return result;
			}
			expects_graphics<sampler_state*> d3d11_device::create_sampler_state(const sampler_state::desc& i)
			{
				D3D11_SAMPLER_DESC state;
				state.AddressU = (D3D11_TEXTURE_ADDRESS_MODE)i.address_u;
				state.AddressV = (D3D11_TEXTURE_ADDRESS_MODE)i.address_v;
				state.AddressW = (D3D11_TEXTURE_ADDRESS_MODE)i.address_w;
				state.ComparisonFunc = (D3D11_COMPARISON_FUNC)i.comparison_function;
				state.Filter = (D3D11_FILTER)i.filter;
				state.MaxAnisotropy = i.max_anisotropy;
				state.MinLOD = i.min_lod;
				state.MaxLOD = i.max_lod;
				state.MipLODBias = i.mip_lod_bias;
				state.BorderColor[0] = i.border_color[0];
				state.BorderColor[1] = i.border_color[1];
				state.BorderColor[2] = i.border_color[2];
				state.BorderColor[3] = i.border_color[3];

				core::uptr<ID3D11SamplerState> device_state;
				HRESULT result_code = context->CreateSamplerState(&state, device_state.out());
				if (result_code != S_OK)
					return get_exception(result_code, "create sampler state");

				d3d11_sampler_state* result = new d3d11_sampler_state(i);
				result->resource = device_state.reset();
				return result;
			}
			expects_graphics<input_layout*> d3d11_device::create_input_layout(const input_layout::desc& i)
			{
				return new d3d11_input_layout(i);
			}
			expects_graphics<shader*> d3d11_device::create_shader(const shader::desc& i)
			{
				shader::desc f(i);
				auto preprocess_status = preprocess(f);
				if (!preprocess_status)
					return graphics_exception(std::move(preprocess_status.error().message()));

				auto name = get_program_name(f);
				if (!name)
					return graphics_exception("shader name is not defined");

				core::uptr<d3d11_shader> result = new d3d11_shader(i);
				core::string program_name = std::move(*name);
				ID3DBlob* shader_blob = nullptr;
				ID3DBlob* error_blob = nullptr;
				void* data = nullptr;
				size_t size = 0;

				core::string vertex_entry = get_shader_main(shader_type::vertex);
				if (f.data.find(vertex_entry) != core::string::npos)
				{
					core::string stage = program_name + SHADER_VERTEX, bytecode;
					if (!get_program_cache(stage, &bytecode))
					{
						VI_DEBUG("[d3d11] compile %s vertex shader source", stage.c_str());
						HRESULT result_code = D3DCompile(f.data.c_str(), f.data.size() * sizeof(char), f.filename.empty() ? nullptr : f.filename.c_str(), nullptr, nullptr, vertex_entry.c_str(), get_vs_profile().data(), compile_flags, 0, &result->signature, &error_blob);
						if (result_code != S_OK || !get_compile_state(error_blob).empty())
						{
							auto message = get_compile_state(error_blob);
							d3d11_release(error_blob);
							return get_exception(result_code, message);
						}

						data = (void*)result->signature->GetBufferPointer();
						size = (size_t)result->signature->GetBufferSize();
						set_program_cache(stage, core::string((char*)data, size));
					}
					else
					{
						data = (void*)bytecode.c_str(); size = bytecode.size();
						HRESULT result_code = D3DCreateBlob(size, &result->signature);
						if (result_code != S_OK)
							return get_exception(result_code, "compile vertex shader");

						void* buffer = result->signature->GetBufferPointer();
						memcpy(buffer, data, size);
					}

					VI_DEBUG("[d3d11] load %s vertex shader bytecode", stage.c_str());
					HRESULT result_code = context->CreateVertexShader(data, size, nullptr, &result->vertex_shader);
					if (result_code != S_OK)
						return get_exception(result_code, "compile vertex shader");

					result_code = D3DReflect(data, size, IID_ID3D11ShaderReflection, (void**)&result->reflection.vertex_shader);
					if (result_code != S_OK)
						return get_exception(result_code, "disassemble vertex shader");
				}

				core::string pixel_entry = get_shader_main(shader_type::pixel);
				if (f.data.find(pixel_entry) != core::string::npos)
				{
					core::string stage = program_name + SHADER_PIXEL, bytecode;
					if (!get_program_cache(stage, &bytecode))
					{
						VI_DEBUG("[d3d11] compile %s pixel shader source", stage.c_str());
						HRESULT result_code = D3DCompile(f.data.c_str(), f.data.size() * sizeof(char), f.filename.empty() ? nullptr : f.filename.c_str(), nullptr, nullptr, pixel_entry.c_str(), get_ps_profile().data(), compile_flags, 0, &shader_blob, &error_blob);
						if (result_code != S_OK || !get_compile_state(error_blob).empty())
						{
							auto message = get_compile_state(error_blob);
							d3d11_release(error_blob);
							return get_exception(result_code, message);
						}

						data = (void*)shader_blob->GetBufferPointer();
						size = (size_t)shader_blob->GetBufferSize();
						set_program_cache(stage, core::string((char*)data, size));
					}
					else
					{
						data = (void*)bytecode.c_str();
						size = bytecode.size();
					}

					VI_DEBUG("[d3d11] load %s pixel shader bytecode", stage.c_str());
					HRESULT result_code = context->CreatePixelShader(data, size, nullptr, &result->pixel_shader);
					if (result_code != S_OK)
					{
						d3d11_release(shader_blob);
						return get_exception(result_code, "compile pixel shader");
					}

					result_code = D3DReflect(data, size, IID_ID3D11ShaderReflection, (void**)&result->reflection.pixel_shader);
					d3d11_release(shader_blob);
					if (result_code != S_OK)
						return get_exception(result_code, "disassemble pixel shader");
				}

				core::string geometry_entry = get_shader_main(shader_type::geometry);
				if (f.data.find(geometry_entry) != core::string::npos)
				{
					core::string stage = program_name + SHADER_GEOMETRY, bytecode;
					if (!get_program_cache(stage, &bytecode))
					{
						VI_DEBUG("[d3d11] compile %s geometry shader source", stage.c_str());
						HRESULT result_code = D3DCompile(f.data.c_str(), f.data.size() * sizeof(char), f.filename.empty() ? nullptr : f.filename.c_str(), nullptr, nullptr, geometry_entry.c_str(), get_gs_profile().data(), get_compile_flags(), 0, &shader_blob, &error_blob);
						if (result_code != S_OK || !get_compile_state(error_blob).empty())
						{
							auto message = get_compile_state(error_blob);
							d3d11_release(error_blob);
							return get_exception(result_code, message);
						}

						data = (void*)shader_blob->GetBufferPointer();
						size = (size_t)shader_blob->GetBufferSize();
						set_program_cache(stage, core::string((char*)data, size));
					}
					else
					{
						data = (void*)bytecode.c_str();
						size = bytecode.size();
					}

					VI_DEBUG("[d3d11] load %s geometry shader bytecode", stage.c_str());
					HRESULT result_code = context->CreateGeometryShader(data, size, nullptr, &result->geometry_shader);
					if (result_code != S_OK)
					{
						d3d11_release(shader_blob);
						return get_exception(result_code, "compile geometry shader");
					}

					result_code = D3DReflect(data, size, IID_ID3D11ShaderReflection, (void**)&result->reflection.geometry_shader);
					d3d11_release(shader_blob);
					if (result_code != S_OK)
						return get_exception(result_code, "disassemble geometry shader");
				}

				core::string compute_entry = get_shader_main(shader_type::compute);
				if (f.data.find(compute_entry) != core::string::npos)
				{
					core::string stage = program_name + SHADER_COMPUTE, bytecode;
					if (!get_program_cache(stage, &bytecode))
					{
						VI_DEBUG("[d3d11] compile %s compute shader source", stage.c_str());
						HRESULT result_code = D3DCompile(f.data.c_str(), f.data.size() * sizeof(char), f.filename.empty() ? nullptr : f.filename.c_str(), nullptr, nullptr, compute_entry.c_str(), get_cs_profile().data(), get_compile_flags(), 0, &shader_blob, &error_blob);
						if (result_code != S_OK || !get_compile_state(error_blob).empty())
						{
							auto message = get_compile_state(error_blob);
							d3d11_release(error_blob);
							return get_exception(result_code, message);
						}

						data = (void*)shader_blob->GetBufferPointer();
						size = (size_t)shader_blob->GetBufferSize();
						set_program_cache(stage, core::string((char*)data, size));
					}
					else
					{
						data = (void*)bytecode.c_str();
						size = bytecode.size();
					}

					VI_DEBUG("[d3d11] load %s compute shader bytecode", stage.c_str());
					HRESULT result_code = context->CreateComputeShader(data, size, nullptr, &result->compute_shader);
					if (result_code != S_OK)
					{
						d3d11_release(shader_blob);
						return get_exception(result_code, "compile compute shader");
					}

					result_code = D3DReflect(data, size, IID_ID3D11ShaderReflection, (void**)&result->reflection.compute_shader);
					d3d11_release(shader_blob);
					if (result_code != S_OK)
						return get_exception(result_code, "disassemble compute shader");
				}

				core::string hull_entry = get_shader_main(shader_type::hull);
				if (f.data.find(hull_entry) != core::string::npos)
				{
					core::string stage = program_name + SHADER_HULL, bytecode;
					if (!get_program_cache(stage, &bytecode))
					{
						VI_DEBUG("[d3d11] compile %s hull shader source", stage.c_str());
						HRESULT result_code = D3DCompile(f.data.c_str(), f.data.size() * sizeof(char), f.filename.empty() ? nullptr : f.filename.c_str(), nullptr, nullptr, hull_entry.c_str(), get_hs_profile().data(), get_compile_flags(), 0, &shader_blob, &error_blob);
						if (result_code != S_OK || !get_compile_state(error_blob).empty())
						{
							auto message = get_compile_state(error_blob);
							d3d11_release(error_blob);
							return get_exception(result_code, message);
						}

						data = (void*)shader_blob->GetBufferPointer();
						size = (size_t)shader_blob->GetBufferSize();
						set_program_cache(stage, core::string((char*)data, size));
					}
					else
					{
						data = (void*)bytecode.c_str();
						size = bytecode.size();
					}

					VI_DEBUG("[d3d11] load %s hull shader bytecode", stage.c_str());
					HRESULT result_code = context->CreateHullShader(data, size, nullptr, &result->hull_shader);
					if (result_code != S_OK)
					{
						d3d11_release(shader_blob);
						return get_exception(result_code, "compile hull shader");
					}

					result_code = D3DReflect(data, size, IID_ID3D11ShaderReflection, (void**)&result->reflection.hull_shader);
					d3d11_release(shader_blob);
					if (result_code != S_OK)
						return get_exception(result_code, "disassemble hull shader");
				}

				core::string domain_entry = get_shader_main(shader_type::domain);
				if (f.data.find(domain_entry) != core::string::npos)
				{
					core::string stage = program_name + SHADER_DOMAIN, bytecode;
					if (!get_program_cache(stage, &bytecode))
					{
						VI_DEBUG("[d3d11] compile %s domain shader source", stage.c_str());
						HRESULT result_code = D3DCompile(f.data.c_str(), f.data.size() * sizeof(char), f.filename.empty() ? nullptr : f.filename.c_str(), nullptr, nullptr, domain_entry.c_str(), get_ds_profile().data(), get_compile_flags(), 0, &shader_blob, &error_blob);
						if (result_code != S_OK || !get_compile_state(error_blob).empty())
						{
							auto message = get_compile_state(error_blob);
							d3d11_release(error_blob);
							return get_exception(result_code, message);
						}

						data = (void*)shader_blob->GetBufferPointer();
						size = (size_t)shader_blob->GetBufferSize();
						set_program_cache(stage, core::string((char*)data, size));
					}
					else
					{
						data = (void*)bytecode.c_str();
						size = bytecode.size();
					}

					VI_DEBUG("[d3d11] load %s domain shader bytecode", stage.c_str());
					HRESULT result_code = context->CreateDomainShader(data, size, nullptr, &result->domain_shader);
					if (result_code != S_OK)
					{
						d3d11_release(shader_blob);
						return get_exception(result_code, "compile domain shader");
					}

					result_code = D3DReflect(data, size, IID_ID3D11ShaderReflection, (void**)&result->reflection.domain_shader);
					d3d11_release(shader_blob);
					if (result_code != S_OK)
						return get_exception(result_code, "disassemble domain shader");
				}

				result->compiled = true;
				return result.reset();
			}
			expects_graphics<element_buffer*> d3d11_device::create_element_buffer(const element_buffer::desc& i)
			{
				D3D11_BUFFER_DESC buffer_desc;
				ZeroMemory(&buffer_desc, sizeof(buffer_desc));
				buffer_desc.Usage = (D3D11_USAGE)i.usage;
				buffer_desc.ByteWidth = (uint32_t)i.element_count * i.element_width;
				buffer_desc.BindFlags = (uint32_t)i.bind_flags;
				buffer_desc.CPUAccessFlags = (uint32_t)i.access_flags;
				buffer_desc.MiscFlags = (uint32_t)i.misc_flags;
				buffer_desc.StructureByteStride = i.structure_byte_stride;

				core::uptr<d3d11_element_buffer> result = new d3d11_element_buffer(i);
				HRESULT result_code;
				if (i.elements != nullptr)
				{
					D3D11_SUBRESOURCE_DATA subresource;
					ZeroMemory(&subresource, sizeof(subresource));
					subresource.pSysMem = i.elements;

					result_code = context->CreateBuffer(&buffer_desc, &subresource, &result->element);
				}
				else
					result_code = context->CreateBuffer(&buffer_desc, nullptr, &result->element);

				if (result_code != S_OK)
					return get_exception(result_code, "create element buffer");

				if (i.writable)
				{
					D3D11_UNORDERED_ACCESS_VIEW_DESC access_desc;
					ZeroMemory(&access_desc, sizeof(access_desc));
					access_desc.Format = DXGI_FORMAT_R32_FLOAT;
					access_desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
					access_desc.Buffer.Flags = 0;
					access_desc.Buffer.FirstElement = 0;
					access_desc.Buffer.NumElements = i.element_count;

					result_code = context->CreateUnorderedAccessView(result->element, &access_desc, &result->access);
					if (result_code != S_OK)
						return get_exception(result_code, "create element buffer");
				}

				if (!((size_t)i.misc_flags & (size_t)resource_misc::buffer_structured))
					return result.reset();

				D3D11_SHADER_RESOURCE_VIEW_DESC SRV;
				ZeroMemory(&SRV, sizeof(SRV));
				SRV.Format = DXGI_FORMAT_UNKNOWN;
				SRV.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
				SRV.Buffer.ElementWidth = i.element_count;

				result_code = context->CreateShaderResourceView(result->element, &SRV, &result->resource);
				if (result_code != S_OK)
					return get_exception(result_code, "create element buffer");

				return result.reset();
			}
			expects_graphics<mesh_buffer*> d3d11_device::create_mesh_buffer(const mesh_buffer::desc& i)
			{
				element_buffer::desc f = element_buffer::desc();
				f.access_flags = i.access_flags;
				f.usage = i.usage;
				f.bind_flags = resource_bind::vertex_buffer;
				f.element_count = (uint32_t)i.elements.size();
				f.elements = (void*)i.elements.data();
				f.element_width = sizeof(trigonometry::vertex);

				auto new_vertex_buffer = create_element_buffer(f);
				if (!new_vertex_buffer)
					return new_vertex_buffer.error();

				f = element_buffer::desc();
				f.access_flags = i.access_flags;
				f.usage = i.usage;
				f.bind_flags = resource_bind::index_buffer;
				f.element_count = (uint32_t)i.indices.size();
				f.element_width = sizeof(int);
				f.elements = (void*)i.indices.data();

				auto new_index_buffer = create_element_buffer(f);
				if (!new_index_buffer)
				{
					core::memory::release(*new_vertex_buffer);
					return new_index_buffer.error();
				}

				d3d11_mesh_buffer* result = new d3d11_mesh_buffer(i);
				result->vertex_buffer = *new_vertex_buffer;
				result->index_buffer = *new_index_buffer;
				return result;
			}
			expects_graphics<mesh_buffer*> d3d11_device::create_mesh_buffer(element_buffer* vertex_buffer, element_buffer* index_buffer)
			{
				VI_ASSERT(vertex_buffer != nullptr, "vertex buffer should be set");
				VI_ASSERT(index_buffer != nullptr, "index buffer should be set");
				d3d11_mesh_buffer* result = new d3d11_mesh_buffer(d3d11_mesh_buffer::desc());
				result->vertex_buffer = vertex_buffer;
				result->index_buffer = index_buffer;
				return result;
			}
			expects_graphics<skin_mesh_buffer*> d3d11_device::create_skin_mesh_buffer(const skin_mesh_buffer::desc& i)
			{
				element_buffer::desc f = element_buffer::desc();
				f.access_flags = i.access_flags;
				f.usage = i.usage;
				f.bind_flags = resource_bind::vertex_buffer;
				f.element_count = (uint32_t)i.elements.size();
				f.elements = (void*)i.elements.data();
				f.element_width = sizeof(trigonometry::skin_vertex);

				auto new_vertex_buffer = create_element_buffer(f);
				if (!new_vertex_buffer)
					return new_vertex_buffer.error();

				f = element_buffer::desc();
				f.access_flags = i.access_flags;
				f.usage = i.usage;
				f.bind_flags = resource_bind::index_buffer;
				f.element_count = (uint32_t)i.indices.size();
				f.element_width = sizeof(int);
				f.elements = (void*)i.indices.data();

				auto new_index_buffer = create_element_buffer(f);
				if (!new_index_buffer)
				{
					core::memory::release(*new_vertex_buffer);
					return new_index_buffer.error();
				}

				d3d11_skin_mesh_buffer* result = new d3d11_skin_mesh_buffer(i);
				result->vertex_buffer = *new_vertex_buffer;
				result->index_buffer = *new_index_buffer;
				return result;
			}
			expects_graphics<skin_mesh_buffer*> d3d11_device::create_skin_mesh_buffer(element_buffer* vertex_buffer, element_buffer* index_buffer)
			{
				VI_ASSERT(vertex_buffer != nullptr, "vertex buffer should be set");
				VI_ASSERT(index_buffer != nullptr, "index buffer should be set");
				d3d11_skin_mesh_buffer* result = new d3d11_skin_mesh_buffer(d3d11_skin_mesh_buffer::desc());
				result->vertex_buffer = vertex_buffer;
				result->index_buffer = index_buffer;
				return result;
			}
			expects_graphics<instance_buffer*> d3d11_device::create_instance_buffer(const instance_buffer::desc& i)
			{
				element_buffer::desc f = element_buffer::desc();
				f.access_flags = cpu_access::write;
				f.misc_flags = resource_misc::buffer_structured;
				f.usage = resource_usage::dynamic;
				f.bind_flags = resource_bind::shader_input;
				f.element_count = i.element_limit;
				f.element_width = i.element_width;
				f.structure_byte_stride = f.element_width;

				auto new_elements = create_element_buffer(f);
				if (!new_elements)
					return new_elements.error();

				core::uptr<d3d11_instance_buffer> result = new d3d11_instance_buffer(i);
				result->elements = *new_elements;

				D3D11_SHADER_RESOURCE_VIEW_DESC SRV;
				ZeroMemory(&SRV, sizeof(SRV));
				SRV.Format = DXGI_FORMAT_UNKNOWN;
				SRV.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
				SRV.Buffer.ElementWidth = i.element_limit;

				HRESULT result_code = context->CreateShaderResourceView(((d3d11_element_buffer*)result->elements)->element, &SRV, &result->resource);
				if (result_code != S_OK)
					return get_exception(result_code, "create instance buffer");

				return result.reset();
			}
			expects_graphics<texture_2d*> d3d11_device::create_texture_2d()
			{
				return new d3d11_texture_2d();
			}
			expects_graphics<texture_2d*> d3d11_device::create_texture_2d(const texture_2d::desc& i)
			{
				D3D11_TEXTURE2D_DESC description;
				ZeroMemory(&description, sizeof(description));
				description.Width = i.width;
				description.Height = i.height;
				description.MipLevels = i.mip_levels;
				description.ArraySize = 1;
				description.Format = (DXGI_FORMAT)i.format_mode;
				description.SampleDesc.Count = 1;
				description.SampleDesc.Quality = 0;
				description.Usage = (D3D11_USAGE)i.usage;
				description.BindFlags = (uint32_t)i.bind_flags;
				description.CPUAccessFlags = (uint32_t)i.access_flags;
				description.MiscFlags = (uint32_t)i.misc_flags;

				if (i.data != nullptr && i.mip_levels > 0)
				{
					description.BindFlags |= D3D11_BIND_RENDER_TARGET;
					description.MiscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
				}

				if (i.writable)
					description.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;

				D3D11_SUBRESOURCE_DATA data;
				data.pSysMem = i.data;
				data.SysMemPitch = i.row_pitch;
				data.SysMemSlicePitch = i.depth_pitch;

				core::uptr<d3d11_texture_2d> result = new d3d11_texture_2d();
				HRESULT result_code = context->CreateTexture2D(&description, i.data && i.mip_levels <= 0 ? &data : nullptr, &result->view);
				if (result_code != S_OK)
					return get_exception(result_code, "create texture 2d");

				auto generate_status = generate_texture(*result);
				if (!generate_status)
					return generate_status.error();

				if (i.data != nullptr && i.mip_levels > 0)
				{
					immediate_context->UpdateSubresource(result->view, 0, nullptr, i.data, i.row_pitch, i.depth_pitch);
					immediate_context->GenerateMips(result->resource);
				}

				if (!i.writable)
					return result.reset();

				D3D11_UNORDERED_ACCESS_VIEW_DESC access_desc;
				ZeroMemory(&access_desc, sizeof(access_desc));
				access_desc.Format = (DXGI_FORMAT)i.format_mode;
				access_desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
				access_desc.Texture2D.MipSlice = 0;

				result_code = context->CreateUnorderedAccessView(result->view, &access_desc, &result->access);
				if (result_code != S_OK)
					return get_exception(result_code, "create texture 2d");

				return result.reset();
			}
			expects_graphics<texture_3d*> d3d11_device::create_texture_3d()
			{
				return new d3d11_texture_3d();
			}
			expects_graphics<texture_3d*> d3d11_device::create_texture_3d(const texture_3d::desc& i)
			{
				D3D11_TEXTURE3D_DESC description;
				ZeroMemory(&description, sizeof(description));
				description.Width = i.width;
				description.Height = i.height;
				description.Depth = i.depth;
				description.MipLevels = i.mip_levels;
				description.Format = (DXGI_FORMAT)i.format_mode;
				description.Usage = (D3D11_USAGE)i.usage;
				description.BindFlags = (uint32_t)i.bind_flags;
				description.CPUAccessFlags = (uint32_t)i.access_flags;
				description.MiscFlags = (uint32_t)i.misc_flags;

				if (i.mip_levels > 0)
				{
					description.BindFlags |= D3D11_BIND_RENDER_TARGET;
					description.MiscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
				}

				if (i.writable)
					description.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;

				core::uptr<d3d11_texture_3d> result = new d3d11_texture_3d();
				HRESULT result_code = context->CreateTexture3D(&description, nullptr, &result->view);
				if (result_code != S_OK)
					return get_exception(result_code, "create texture 3d");

				auto generate_status = generate_texture(*result);
				if (!generate_status)
					return generate_status.error();

				if (!i.writable)
					return result.reset();

				D3D11_UNORDERED_ACCESS_VIEW_DESC access_desc;
				ZeroMemory(&access_desc, sizeof(access_desc));
				access_desc.Format = (DXGI_FORMAT)i.format_mode;
				access_desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE3D;
				access_desc.Texture3D.MipSlice = 0;
				access_desc.Texture3D.FirstWSlice = 0;
				access_desc.Texture3D.WSize = i.depth;

				result_code = context->CreateUnorderedAccessView(result->view, &access_desc, &result->access);
				if (result_code != S_OK)
					return get_exception(result_code, "create texture 3d");

				return result.reset();
			}
			expects_graphics<texture_cube*> d3d11_device::create_texture_cube()
			{
				return new d3d11_texture_cube();
			}
			expects_graphics<texture_cube*> d3d11_device::create_texture_cube(const texture_cube::desc& i)
			{
				D3D11_TEXTURE2D_DESC description;
				ZeroMemory(&description, sizeof(description));
				description.Width = i.width;
				description.Height = i.height;
				description.MipLevels = i.mip_levels;
				description.ArraySize = 6;
				description.Format = (DXGI_FORMAT)i.format_mode;
				description.SampleDesc.Count = 1;
				description.SampleDesc.Quality = 0;
				description.Usage = (D3D11_USAGE)i.usage;
				description.BindFlags = (uint32_t)i.bind_flags;
				description.CPUAccessFlags = (uint32_t)i.access_flags;
				description.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE | (uint32_t)i.misc_flags;

				if (i.mip_levels > 0)
				{
					description.BindFlags |= D3D11_BIND_RENDER_TARGET;
					description.MiscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
				}

				if (i.writable)
					description.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;

				core::uptr<d3d11_texture_cube> result = new d3d11_texture_cube();
				HRESULT result_code = context->CreateTexture2D(&description, 0, &result->view);
				if (result_code != S_OK)
					return get_exception(result_code, "create texture cube");

				D3D11_BOX region;
				region.left = 0;
				region.right = description.Width;
				region.top = 0;
				region.bottom = description.Height;
				region.front = 0;
				region.back = 1;

				auto generate_status = generate_texture(*result);
				if (!generate_status)
					return generate_status.error();

				if (!i.writable)
					return result.reset();

				D3D11_UNORDERED_ACCESS_VIEW_DESC access_desc;
				ZeroMemory(&access_desc, sizeof(access_desc));
				access_desc.Format = (DXGI_FORMAT)i.format_mode;
				access_desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2DARRAY;
				access_desc.Texture2DArray.MipSlice = 0;
				access_desc.Texture2DArray.FirstArraySlice = 0;
				access_desc.Texture2DArray.ArraySize = 6;

				result_code = context->CreateUnorderedAccessView(result->view, &access_desc, &result->access);
				if (result_code != S_OK)
					return get_exception(result_code, "create texture cube");

				return result.reset();
			}
			expects_graphics<texture_cube*> d3d11_device::create_texture_cube(graphics::texture_2d* resource[6])
			{
				VI_ASSERT(resource[0] && resource[1] && resource[2] && resource[3] && resource[4] && resource[5], "all 6 faces should be set");
				void* resources[6];
				for (uint32_t i = 0; i < 6; i++)
					resources[i] = (void*)((d3d11_texture_2d*)resource[i])->view;

				return create_texture_cube_internal(resources);
			}
			expects_graphics<texture_cube*> d3d11_device::create_texture_cube(graphics::texture_2d* resource)
			{
				VI_ASSERT(resource != nullptr, "resource should be set");
				ID3D11Texture2D* src = ((d3d11_texture_2d*)resource)->view;

				VI_ASSERT(src != nullptr, "src should be set");
				D3D11_TEXTURE2D_DESC description;
				src->GetDesc(&description);
				description.ArraySize = 6;
				description.Usage = D3D11_USAGE_DEFAULT;
				description.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
				description.CPUAccessFlags = 0;
				description.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE | D3D11_RESOURCE_MISC_GENERATE_MIPS;

				uint32_t width = description.Width;
				description.Width = description.Width / 4;

				uint32_t height = description.Height;
				description.Height = description.Width;

				D3D11_BOX region;
				region.front = 0;
				region.back = 1;

				description.MipLevels = get_mip_level(description.Width, description.Height);
				if (width % 4 != 0 || height % 3 != 0)
					return graphics_exception("create texture cube: width / height is invalid");

				core::uptr<d3d11_texture_cube> result = new d3d11_texture_cube();
				HRESULT result_code = context->CreateTexture2D(&description, 0, &result->view);
				if (result_code != S_OK)
					return get_exception(result_code, "create texture cube");

				region.left = description.Width * 2 - 1;
				region.top = description.Height - 1;
				region.right = region.left + description.Width;
				region.bottom = region.top + description.Height;
				immediate_context->CopySubresourceRegion(result->view, D3D11CalcSubresource(0, 0, description.MipLevels), 0, 0, 0, src, 0, &region);

				region.left = 0;
				region.top = description.Height;
				region.right = region.left + description.Width;
				region.bottom = region.top + description.Height;
				immediate_context->CopySubresourceRegion(result->view, D3D11CalcSubresource(0, 1, description.MipLevels), 0, 0, 0, src, 0, &region);

				region.left = description.Width;
				region.top = 0;
				region.right = region.left + description.Width;
				region.bottom = region.top + description.Height;
				immediate_context->CopySubresourceRegion(result->view, D3D11CalcSubresource(0, 2, description.MipLevels), 0, 0, 0, src, 0, &region);

				region.top = description.Height * 2;
				region.right = region.left + description.Width;
				region.bottom = region.top + description.Height;
				immediate_context->CopySubresourceRegion(result->view, D3D11CalcSubresource(0, 3, description.MipLevels), 0, 0, 0, src, 0, &region);

				region.top = description.Height;
				region.right = region.left + description.Width;
				region.bottom = region.top + description.Height;
				immediate_context->CopySubresourceRegion(result->view, D3D11CalcSubresource(0, 4, description.MipLevels), 0, 0, 0, src, 0, &region);

				region.left = description.Width * 3;
				region.right = region.left + description.Width;
				region.bottom = region.top + description.Height;
				immediate_context->CopySubresourceRegion(result->view, D3D11CalcSubresource(0, 5, description.MipLevels), 0, 0, 0, src, 0, &region);

				auto generate_status = generate_texture(*result);
				if (!generate_status)
					return generate_status.error();

				if (description.MipLevels > 0)
					immediate_context->GenerateMips(result->resource);

				return result.reset();
			}
			expects_graphics<texture_cube*> d3d11_device::create_texture_cube_internal(void* resource[6])
			{
				VI_ASSERT(resource[0] && resource[1] && resource[2] && resource[3] && resource[4] && resource[5], "all 6 faces should be set");

				D3D11_TEXTURE2D_DESC description;
				((ID3D11Texture2D*)resource[0])->GetDesc(&description);
				description.MipLevels = 1;
				description.ArraySize = 6;
				description.Usage = D3D11_USAGE_DEFAULT;
				description.BindFlags = D3D11_BIND_SHADER_RESOURCE;
				description.CPUAccessFlags = 0;
				description.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

				core::uptr<d3d11_texture_cube> result = new d3d11_texture_cube();
				HRESULT result_code = context->CreateTexture2D(&description, 0, &result->view);
				if (result_code != S_OK)
					return get_exception(result_code, "create texture cube internal");

				D3D11_BOX region;
				region.left = 0;
				region.right = description.Width;
				region.top = 0;
				region.bottom = description.Height;
				region.front = 0;
				region.back = 1;

				for (uint32_t j = 0; j < 6; j++)
					immediate_context->CopySubresourceRegion(result->view, j, 0, 0, 0, (ID3D11Texture2D*)resource[j], 0, &region);

				auto generate_status = generate_texture(*result);
				if (!generate_status)
					return generate_status.error();

				return result.reset();
			}
			expects_graphics<depth_target_2d*> d3d11_device::create_depth_target_2d(const depth_target_2d::desc& i)
			{
				D3D11_TEXTURE2D_DESC depth_buffer;
				ZeroMemory(&depth_buffer, sizeof(depth_buffer));
				depth_buffer.Width = i.width;
				depth_buffer.Height = i.height;
				depth_buffer.MipLevels = 1;
				depth_buffer.ArraySize = 1;
				depth_buffer.Format = get_base_depth_format(i.format_mode);
				depth_buffer.SampleDesc.Count = 1;
				depth_buffer.SampleDesc.Quality = 0;
				depth_buffer.Usage = (D3D11_USAGE)i.usage;
				depth_buffer.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
				depth_buffer.CPUAccessFlags = (uint32_t)i.access_flags;
				depth_buffer.MiscFlags = 0;

				ID3D11Texture2D* depth_texture_address = nullptr;
				HRESULT result_code = context->CreateTexture2D(&depth_buffer, nullptr, &depth_texture_address);
				if (result_code != S_OK)
					return get_exception(result_code, "create depth target 2d");

				D3D11_DEPTH_STENCIL_VIEW_DESC DSV;
				ZeroMemory(&DSV, sizeof(DSV));
				DSV.Format = get_internal_depth_format(i.format_mode);
				DSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
				DSV.Texture2D.MipSlice = 0;

				core::uptr<ID3D11Texture2D> depth_texture = depth_texture_address;
				core::uptr<d3d11_depth_target_2d> result = new d3d11_depth_target_2d(i);
				result_code = context->CreateDepthStencilView(*depth_texture, &DSV, &result->depth_stencil_view);
				if (result_code != S_OK)
					return get_exception(result_code, "create depth target 2d");

				auto new_texture = create_texture_2d();
				if (!new_texture)
					return new_texture.error();

				result->resource = *new_texture;
				((d3d11_texture_2d*)result->resource)->view = depth_texture.reset();

				auto new_resource = create_texture_2d(result->resource, get_depth_format(i.format_mode));
				if (!new_resource)
					return new_resource.error();

				result->viewarea.width = (FLOAT)i.width;
				result->viewarea.height = (FLOAT)i.height;
				result->viewarea.min_depth = 0.0f;
				result->viewarea.max_depth = 1.0f;
				result->viewarea.top_left_x = 0.0f;
				result->viewarea.top_left_y = 0.0f;
				return result.reset();
			}
			expects_graphics<depth_target_cube*> d3d11_device::create_depth_target_cube(const depth_target_cube::desc& i)
			{
				D3D11_TEXTURE2D_DESC depth_buffer;
				ZeroMemory(&depth_buffer, sizeof(depth_buffer));
				depth_buffer.Width = i.size;
				depth_buffer.Height = i.size;
				depth_buffer.MipLevels = 1;
				depth_buffer.ArraySize = 6;
				depth_buffer.Format = get_base_depth_format(i.format_mode);
				depth_buffer.SampleDesc.Count = 1;
				depth_buffer.SampleDesc.Quality = 0;
				depth_buffer.Usage = (D3D11_USAGE)i.usage;
				depth_buffer.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
				depth_buffer.CPUAccessFlags = (uint32_t)i.access_flags;
				depth_buffer.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

				ID3D11Texture2D* depth_texture_address = nullptr;
				HRESULT result_code = context->CreateTexture2D(&depth_buffer, nullptr, &depth_texture_address);
				if (result_code != S_OK)
					return get_exception(result_code, "create depth target 2d");

				D3D11_DEPTH_STENCIL_VIEW_DESC DSV;
				ZeroMemory(&DSV, sizeof(DSV));
				DSV.Format = get_internal_depth_format(i.format_mode);
				DSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
				DSV.Texture2DArray.FirstArraySlice = 0;
				DSV.Texture2DArray.ArraySize = 6;
				DSV.Texture2DArray.MipSlice = 0;

				core::uptr<ID3D11Texture2D> depth_texture = depth_texture_address;
				core::uptr<d3d11_depth_target_cube> result = new d3d11_depth_target_cube(i);
				result_code = context->CreateDepthStencilView(*depth_texture, &DSV, &result->depth_stencil_view);
				if (result_code != S_OK)
					return get_exception(result_code, "create depth target cube");

				auto new_texture = create_texture_cube();
				if (!new_texture)
					return new_texture.error();

				result->resource = *new_texture;
				((d3d11_texture_cube*)result->resource)->view = depth_texture.reset();

				auto new_resource = create_texture_cube(result->resource, get_depth_format(i.format_mode));
				if (!new_resource)
					return new_resource.error();

				result->viewarea.width = (FLOAT)i.size;
				result->viewarea.height = (FLOAT)i.size;
				result->viewarea.min_depth = 0.0f;
				result->viewarea.max_depth = 1.0f;
				result->viewarea.top_left_x = 0.0f;
				result->viewarea.top_left_y = 0.0f;
				return result.reset();
			}
			expects_graphics<render_target_2d*> d3d11_device::create_render_target_2d(const render_target_2d::desc& i)
			{
				D3D11_TEXTURE2D_DESC depth_buffer;
				ZeroMemory(&depth_buffer, sizeof(depth_buffer));
				depth_buffer.Width = i.width;
				depth_buffer.Height = i.height;
				depth_buffer.MipLevels = 1;
				depth_buffer.ArraySize = 1;
				depth_buffer.Format = DXGI_FORMAT_R24G8_TYPELESS;
				depth_buffer.SampleDesc.Count = 1;
				depth_buffer.SampleDesc.Quality = 0;
				depth_buffer.Usage = (D3D11_USAGE)i.usage;
				depth_buffer.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
				depth_buffer.CPUAccessFlags = (uint32_t)i.access_flags;
				depth_buffer.MiscFlags = 0;

				ID3D11Texture2D* depth_texture_address = nullptr;
				HRESULT result_code = context->CreateTexture2D(&depth_buffer, nullptr, &depth_texture_address);
				if (result_code != S_OK)
					return get_exception(result_code, "create render target 2d");

				D3D11_DEPTH_STENCIL_VIEW_DESC DSV;
				ZeroMemory(&DSV, sizeof(DSV));
				DSV.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
				DSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
				DSV.Texture2D.MipSlice = 0;

				core::uptr<ID3D11Texture2D> depth_texture = depth_texture_address;
				core::uptr<d3d11_render_target_2d> result = new d3d11_render_target_2d(i);
				result_code = context->CreateDepthStencilView(*depth_texture, &DSV, &result->depth_stencil_view);
				if (result_code != S_OK)
					return get_exception(result_code, "create render target 2d");

				auto new_texture = create_texture_2d();
				if (!new_texture)
					return new_texture.error();

				result->depth_stencil = *new_texture;
				((d3d11_texture_2d*)result->depth_stencil)->view = depth_texture.reset();

				auto new_resource = create_texture_2d(result->depth_stencil, DXGI_FORMAT_R24_UNORM_X8_TYPELESS);
				if (!new_resource)
					return new_resource.error();

				if (i.render_surface == nullptr)
				{
					D3D11_TEXTURE2D_DESC description;
					ZeroMemory(&description, sizeof(description));
					description.Width = i.width;
					description.Height = i.height;
					description.MipLevels = (i.mip_levels < 1 ? 1 : i.mip_levels);
					description.ArraySize = 1;
					description.Format = get_non_depth_format(i.format_mode);
					description.SampleDesc.Count = 1;
					description.SampleDesc.Quality = 0;
					description.Usage = (D3D11_USAGE)i.usage;
					description.BindFlags = (uint32_t)i.bind_flags;
					description.CPUAccessFlags = (uint32_t)i.access_flags;
					description.MiscFlags = (uint32_t)i.misc_flags;

					result_code = context->CreateTexture2D(&description, nullptr, &result->texture);
					if (result_code != S_OK)
						return get_exception(result_code, "create render target 2d");

					auto new_subtexture = create_texture_2d();
					if (!new_subtexture)
						return new_subtexture.error();

					result->resource = *new_subtexture;
					((d3d11_texture_2d*)result->resource)->view = result->texture;
					result->texture->AddRef();

					auto generate_status = generate_texture(result->resource);
					if (!generate_status)
						return generate_status.error();
				}

				D3D11_RENDER_TARGET_VIEW_DESC RTV;
				RTV.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
				RTV.Texture2DArray.MipSlice = 0;
				RTV.Texture2DArray.ArraySize = 1;
				RTV.Format = get_non_depth_format(i.format_mode);

				result_code = context->CreateRenderTargetView(i.render_surface ? (ID3D11Texture2D*)i.render_surface : result->texture, &RTV, &result->render_target_view);
				if (result_code != S_OK)
					return get_exception(result_code, "create render target 2d");

				result->viewarea.width = (float)i.width;
				result->viewarea.height = (float)i.height;
				result->viewarea.min_depth = 0.0f;
				result->viewarea.max_depth = 1.0f;
				result->viewarea.top_left_x = 0.0f;
				result->viewarea.top_left_y = 0.0f;

				if (!i.render_surface)
					return result.reset();

				result_code = swap_chain->GetBuffer(0, __uuidof(result->texture), reinterpret_cast<void**>(&result->texture));
				if (result_code != S_OK)
					return get_exception(result_code, "create render target 2d");

				auto new_target = create_texture_2d();
				if (!new_target)
					return new_target.error();

				d3d11_texture_2d* target = (d3d11_texture_2d*)*new_target;
				target->view = result->texture;
				result->resource = target;
				result->texture->AddRef();

				D3D11_TEXTURE2D_DESC description;
				target->view->GetDesc(&description);
				target->format_mode = (format)description.Format;
				target->usage = (resource_usage)description.Usage;
				target->width = description.Width;
				target->height = description.Height;
				target->mip_levels = description.MipLevels;
				target->access_flags = (cpu_access)description.CPUAccessFlags;
				target->binding = (resource_bind)description.BindFlags;
				return result.reset();
			}
			expects_graphics<multi_render_target_2d*> d3d11_device::create_multi_render_target_2d(const multi_render_target_2d::desc& i)
			{
				D3D11_TEXTURE2D_DESC depth_buffer;
				ZeroMemory(&depth_buffer, sizeof(depth_buffer));
				depth_buffer.Width = i.width;
				depth_buffer.Height = i.height;
				depth_buffer.MipLevels = 1;
				depth_buffer.ArraySize = 1;
				depth_buffer.Format = DXGI_FORMAT_R24G8_TYPELESS;
				depth_buffer.SampleDesc.Count = 1;
				depth_buffer.SampleDesc.Quality = 0;
				depth_buffer.Usage = (D3D11_USAGE)i.usage;
				depth_buffer.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
				depth_buffer.CPUAccessFlags = (uint32_t)i.access_flags;
				depth_buffer.MiscFlags = 0;

				ID3D11Texture2D* depth_texture_address = nullptr;
				HRESULT result_code = context->CreateTexture2D(&depth_buffer, nullptr, &depth_texture_address);
				if (result_code != S_OK)
					return get_exception(result_code, "create multi render target 2d");

				D3D11_DEPTH_STENCIL_VIEW_DESC DSV;
				ZeroMemory(&DSV, sizeof(DSV));
				DSV.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
				DSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
				DSV.Texture2D.MipSlice = 0;

				core::uptr<ID3D11Texture2D> depth_texture = depth_texture_address;
				core::uptr<d3d11_multi_render_target_2d> result = new d3d11_multi_render_target_2d(i);
				result_code = context->CreateDepthStencilView(*depth_texture, &DSV, &result->depth_stencil_view);
				if (result_code != S_OK)
					return get_exception(result_code, "create multi render target 2d");

				auto new_texture = create_texture_2d();
				if (!new_texture)
					return get_exception(result_code, "create multi render target 2d");

				result->depth_stencil = *new_texture;
				((d3d11_texture_2d*)result->depth_stencil)->view = depth_texture.reset();

				auto new_resource = create_texture_2d(result->depth_stencil, DXGI_FORMAT_R24_UNORM_X8_TYPELESS);
				if (!new_resource)
					return get_exception(result_code, "create multi render target 2d");

				uint32_t mip_levels = (i.mip_levels < 1 ? 1 : i.mip_levels);
				ZeroMemory(&result->information, sizeof(result->information));
				result->information.Width = i.width;
				result->information.Height = i.height;
				result->information.MipLevels = mip_levels;
				result->information.ArraySize = 1;
				result->information.SampleDesc.Count = 1;
				result->information.SampleDesc.Quality = 0;
				result->information.Usage = (D3D11_USAGE)i.usage;
				result->information.BindFlags = (uint32_t)i.bind_flags;
				result->information.CPUAccessFlags = (uint32_t)i.access_flags;
				result->information.MiscFlags = (uint32_t)i.misc_flags;

				D3D11_RENDER_TARGET_VIEW_DESC RTV;
				RTV.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
				RTV.Texture2DArray.MipSlice = 0;
				RTV.Texture2DArray.ArraySize = 1;

				for (uint32_t j = 0; j < (uint32_t)result->target; j++)
				{
					DXGI_FORMAT format = get_non_depth_format(i.format_mode[j]);
					result->information.Format = format;
					result_code = context->CreateTexture2D(&result->information, nullptr, &result->texture[j]);
					if (result_code != S_OK)
						return get_exception(result_code, "create multi render target 2d surface");

					RTV.Format = format;
					result_code = context->CreateRenderTargetView(result->texture[j], &RTV, &result->render_target_view[j]);
					if (result_code != S_OK)
						return get_exception(result_code, "create multi render target 2d surface");

					auto new_subtexture = create_texture_2d();
					if (!new_subtexture)
						return new_subtexture.error();

					result->resource[j] = *new_subtexture;
					((d3d11_texture_2d*)result->resource[j])->view = result->texture[j];
					result->texture[j]->AddRef();

					auto generate_status = generate_texture(result->resource[j]);
					if (!generate_status)
						return generate_status.error();
				}

				result->viewarea.width = (float)i.width;
				result->viewarea.height = (float)i.height;
				result->viewarea.min_depth = 0.0f;
				result->viewarea.max_depth = 1.0f;
				result->viewarea.top_left_x = 0.0f;
				result->viewarea.top_left_y = 0.0f;
				return result.reset();
			}
			expects_graphics<render_target_cube*> d3d11_device::create_render_target_cube(const render_target_cube::desc& i)
			{
				D3D11_TEXTURE2D_DESC depth_buffer;
				ZeroMemory(&depth_buffer, sizeof(depth_buffer));
				depth_buffer.Width = i.size;
				depth_buffer.Height = i.size;
				depth_buffer.MipLevels = 1;
				depth_buffer.ArraySize = 6;
				depth_buffer.Format = DXGI_FORMAT_R24G8_TYPELESS;
				depth_buffer.SampleDesc.Count = 1;
				depth_buffer.SampleDesc.Quality = 0;
				depth_buffer.Usage = (D3D11_USAGE)i.usage;
				depth_buffer.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
				depth_buffer.CPUAccessFlags = (uint32_t)i.access_flags;
				depth_buffer.MiscFlags = 0;

				ID3D11Texture2D* depth_texture_address = nullptr;
				HRESULT result_code = context->CreateTexture2D(&depth_buffer, nullptr, &depth_texture_address);
				if (result_code != S_OK)
					return get_exception(result_code, "create render target cube");

				D3D11_DEPTH_STENCIL_VIEW_DESC DSV;
				ZeroMemory(&DSV, sizeof(DSV));
				DSV.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
				DSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
				DSV.Texture2DArray.FirstArraySlice = 0;
				DSV.Texture2DArray.ArraySize = 6;
				DSV.Texture2DArray.MipSlice = 0;

				core::uptr<ID3D11Texture2D> depth_texture = depth_texture_address;
				core::uptr<d3d11_render_target_cube> result = new d3d11_render_target_cube(i);
				result_code = context->CreateDepthStencilView(*depth_texture, &DSV, &result->depth_stencil_view);
				if (result_code != S_OK)
					return get_exception(result_code, "create render target cube");

				auto new_texture = create_texture_2d();
				if (!new_texture)
					return new_texture.error();

				result->depth_stencil = *new_texture;
				((d3d11_texture_2d*)result->depth_stencil)->view = depth_texture.reset();

				auto new_resource = create_texture_2d(result->depth_stencil, DXGI_FORMAT_R24_UNORM_X8_TYPELESS);
				if (!new_resource)
					return new_resource.error();

				uint32_t mip_levels = (i.mip_levels < 1 ? 1 : i.mip_levels);
				D3D11_TEXTURE2D_DESC description;
				ZeroMemory(&description, sizeof(description));
				description.Width = i.size;
				description.Height = i.size;
				description.MipLevels = mip_levels;
				description.ArraySize = 6;
				description.SampleDesc.Count = 1;
				description.SampleDesc.Quality = 0;
				description.Format = get_non_depth_format(i.format_mode);
				description.Usage = (D3D11_USAGE)i.usage;
				description.BindFlags = (uint32_t)i.bind_flags;
				description.CPUAccessFlags = (uint32_t)i.access_flags;
				description.MiscFlags = (uint32_t)i.misc_flags;

				result_code = context->CreateTexture2D(&description, nullptr, &result->texture);
				if (result_code != S_OK)
					return get_exception(result_code, "create render target cube");

				D3D11_RENDER_TARGET_VIEW_DESC RTV;
				ZeroMemory(&RTV, sizeof(RTV));
				RTV.Format = description.Format;
				RTV.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
				RTV.Texture2DArray.FirstArraySlice = 0;
				RTV.Texture2DArray.ArraySize = 6;
				RTV.Texture2DArray.MipSlice = 0;

				result_code = context->CreateRenderTargetView(result->texture, &RTV, &result->render_target_view);
				if (result_code != S_OK)
					return get_exception(result_code, "create render target cube");

				auto new_subtexture = create_texture_cube();
				if (!new_subtexture)
					return new_subtexture.error();

				result->resource = *new_subtexture;
				((d3d11_texture_cube*)result->resource)->view = result->texture;
				result->texture->AddRef();

				auto generate_status = generate_texture(result->resource);
				if (!generate_status)
					return generate_status.error();

				result->viewarea.width = (float)i.size;
				result->viewarea.height = (float)i.size;
				result->viewarea.min_depth = 0.0f;
				result->viewarea.max_depth = 1.0f;
				result->viewarea.top_left_x = 0.0f;
				result->viewarea.top_left_y = 0.0f;
				return result.reset();
			}
			expects_graphics<multi_render_target_cube*> d3d11_device::create_multi_render_target_cube(const multi_render_target_cube::desc& i)
			{
				D3D11_TEXTURE2D_DESC depth_buffer;
				ZeroMemory(&depth_buffer, sizeof(depth_buffer));
				depth_buffer.Width = i.size;
				depth_buffer.Height = i.size;
				depth_buffer.MipLevels = 1;
				depth_buffer.ArraySize = 6;
				depth_buffer.Format = DXGI_FORMAT_R24G8_TYPELESS;
				depth_buffer.SampleDesc.Count = 1;
				depth_buffer.SampleDesc.Quality = 0;
				depth_buffer.Usage = (D3D11_USAGE)i.usage;
				depth_buffer.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
				depth_buffer.CPUAccessFlags = (uint32_t)i.access_flags;
				depth_buffer.MiscFlags = 0;

				ID3D11Texture2D* depth_texture_address = nullptr;
				HRESULT result_code = context->CreateTexture2D(&depth_buffer, nullptr, &depth_texture_address);
				if (result_code != S_OK)
					return get_exception(result_code, "create multi render target cube");

				D3D11_DEPTH_STENCIL_VIEW_DESC DSV;
				ZeroMemory(&DSV, sizeof(DSV));
				DSV.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
				DSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
				DSV.Texture2DArray.FirstArraySlice = 0;
				DSV.Texture2DArray.ArraySize = 6;
				DSV.Texture2DArray.MipSlice = 0;

				core::uptr<ID3D11Texture2D> depth_texture = depth_texture_address;
				core::uptr<d3d11_multi_render_target_cube> result = new d3d11_multi_render_target_cube(i);
				result_code = context->CreateDepthStencilView(*depth_texture, &DSV, &result->depth_stencil_view);
				if (result_code != S_OK)
					return get_exception(result_code, "create multi render target cube");

				auto new_texture = create_texture_2d();
				if (!new_texture)
					return new_texture.error();

				result->depth_stencil = *new_texture;
				((d3d11_texture_2d*)result->depth_stencil)->view = depth_texture.reset();

				auto new_resource = create_texture_2d(result->depth_stencil, DXGI_FORMAT_R24_UNORM_X8_TYPELESS);
				if (!new_resource)
					return new_resource.error();

				uint32_t mip_levels = (i.mip_levels < 1 ? 1 : i.mip_levels);
				D3D11_TEXTURE2D_DESC description;
				ZeroMemory(&description, sizeof(description));
				description.Width = i.size;
				description.Height = i.size;
				description.ArraySize = 6;
				description.SampleDesc.Count = 1;
				description.SampleDesc.Quality = 0;
				description.Usage = (D3D11_USAGE)i.usage;
				description.CPUAccessFlags = (uint32_t)i.access_flags;
				description.MiscFlags = (uint32_t)i.misc_flags;
				description.BindFlags = (uint32_t)i.bind_flags;
				description.MipLevels = mip_levels;

				D3D11_RENDER_TARGET_VIEW_DESC RTV;
				ZeroMemory(&RTV, sizeof(RTV));
				RTV.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
				RTV.Texture2DArray.FirstArraySlice = 0;
				RTV.Texture2DArray.ArraySize = 6;
				RTV.Texture2DArray.MipSlice = 0;

				for (uint32_t j = 0; j < (uint32_t)result->target; j++)
				{
					DXGI_FORMAT format = get_non_depth_format(i.format_mode[j]);
					description.Format = format;
					result_code = context->CreateTexture2D(&description, nullptr, &result->texture[j]);
					if (result_code != S_OK)
						return get_exception(result_code, "create multi render target cube surface");

					RTV.Format = format;
					result_code = context->CreateRenderTargetView(result->texture[j], &RTV, &result->render_target_view[j]);
					if (result_code != S_OK)
						return get_exception(result_code, "create multi render target cube surface");

					auto new_subtexture = create_texture_cube();
					if (!new_subtexture)
						return new_subtexture.error();

					result->resource[j] = *new_subtexture;
					((d3d11_texture_cube*)result->resource[j])->view = result->texture[j];
					result->texture[j]->AddRef();

					auto generate_status = generate_texture(result->resource[j]);
					if (!generate_status)
						return generate_status.error();
				}

				result->viewarea.width = (float)i.size;
				result->viewarea.height = (float)i.size;
				result->viewarea.min_depth = 0.0f;
				result->viewarea.max_depth = 1.0f;
				result->viewarea.top_left_x = 0.0f;
				result->viewarea.top_left_y = 0.0f;
				return result.reset();
			}
			expects_graphics<cubemap*> d3d11_device::create_cubemap(const cubemap::desc& i)
			{
				core::uptr<d3d11_cubemap> result = new d3d11_cubemap(i);
				D3D11_TEXTURE2D_DESC texture;
				result->source->GetDesc(&texture);
				texture.ArraySize = 1;
				texture.CPUAccessFlags = 0;
				texture.MiscFlags = 0;
				texture.MipLevels = i.mip_levels;

				HRESULT result_code = context->CreateTexture2D(&texture, nullptr, &result->merger);
				if (result_code != S_OK)
					return get_exception(result_code, "create cubemap");

				return result.reset();
			}
			expects_graphics<query*> d3d11_device::create_query(const query::desc& i)
			{
				D3D11_QUERY_DESC desc;
				desc.Query = (i.predicate ? D3D11_QUERY_OCCLUSION_PREDICATE : D3D11_QUERY_OCCLUSION);
				desc.MiscFlags = (i.auto_pass ? D3D11_QUERY_MISC_PREDICATEHINT : 0);

				core::uptr<d3d11_query> result = new d3d11_query();
				HRESULT result_code = context->CreateQuery(&desc, &result->async);
				if (result_code != S_OK)
					return get_exception(result_code, "create query");

				return result.reset();
			}
			primitive_topology d3d11_device::get_primitive_topology() const
			{
				D3D11_PRIMITIVE_TOPOLOGY topology;
				immediate_context->IAGetPrimitiveTopology(&topology);
				return (primitive_topology)topology;
			}
			shader_model d3d11_device::get_supported_shader_model() const
			{
				if (feature_level == D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_11_0)
					return shader_model::hlsl_5_0;

				if (feature_level == D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_10_1)
					return shader_model::hlsl_4_1;

				if (feature_level == D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_10_0)
					return shader_model::hlsl_4_0;

				if (feature_level == D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_9_3)
					return shader_model::hlsl_3_0;

				if (feature_level == D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_9_2)
					return shader_model::hlsl_2_0;

				if (feature_level == D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_9_1)
					return shader_model::hlsl_1_0;

				return shader_model::invalid;
			}
			void* d3d11_device::get_device() const
			{
				return (void*)context;
			}
			void* d3d11_device::get_context() const
			{
				return (void*)immediate_context;
			}
			bool d3d11_device::is_valid() const
			{
				return context != nullptr && immediate_context != nullptr;
			}
			expects_graphics<void> d3d11_device::create_direct_buffer(size_t size)
			{
				max_elements = size + 1;
				d3d11_release(immediate.vertex_buffer);

				D3D11_BUFFER_DESC buffer_desc;
				ZeroMemory(&buffer_desc, sizeof(buffer_desc));
				buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
				buffer_desc.ByteWidth = (uint32_t)max_elements * sizeof(vertex);
				buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
				buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

				HRESULT result_code = context->CreateBuffer(&buffer_desc, nullptr, &immediate.vertex_buffer);
				if (result_code != S_OK)
					return get_exception(result_code, "create direct vertex buffer");

				if (!immediate.sampler)
				{
					d3d11_sampler_state* state = (d3d11_sampler_state*)get_sampler_state("a16_fa_wrap");
					if (state != nullptr)
						immediate.sampler = state->resource;
				}

				if (!immediate.vertex_shader)
				{
					static const char* vertex_shader_code = D3D_INLINE(
					cbuffer vertex_buffer : register(b0)
					{
						matrix transform;
						float4 padding;
					};

					struct vs_input
					{
						float4 position : POSITION0;
						float2 texcoord : TEXCOORD0;
						float4 color : COLOR0;
					};

					struct vs_output
					{
						float4 position : SV_POSITION;
						float2 texcoord : TEXCOORD0;
						float4 color : COLOR0;
					};

					vs_output vs_main(vs_input input)
					{
						vs_output output;
						output.position = mul(transform, float4(input.position.xyz, 1));
						output.color = input.color;
						output.texcoord = input.texcoord;

						return output;
					};);

					ID3DBlob* blob = nullptr, * error = nullptr;
					result_code = D3DCompile(vertex_shader_code, strlen(vertex_shader_code), nullptr, nullptr, nullptr, "vs_main", get_vs_profile().data(), 0, 0, &blob, &error);
					if (result_code != S_OK || !get_compile_state(error).empty())
					{
						auto message = get_compile_state(error);
						d3d11_release(error);
						return get_exception(result_code, message);
					}

					result_code = context->CreateVertexShader((DWORD*)blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &immediate.vertex_shader);
					if (result_code != S_OK)
					{
						d3d11_release(blob);
						return get_exception(result_code, "compile direct vertex shader");
					}

					D3D11_INPUT_ELEMENT_DESC layout_desc[] =
					{
						{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
						{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 3 * sizeof(float), D3D11_INPUT_PER_VERTEX_DATA, 0 },
						{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 5 * sizeof(float), D3D11_INPUT_PER_VERTEX_DATA, 0 }
					};
					result_code = context->CreateInputLayout(layout_desc, 3, blob->GetBufferPointer(), blob->GetBufferSize(), &immediate.vertex_layout);
					d3d11_release(blob);
					if (result_code != S_OK)
						return get_exception(result_code, "create direct input layout");
				}

				if (!immediate.pixel_shader)
				{
					static const char* pixel_shader_code = D3D_INLINE(
					cbuffer vertex_buffer : register(b0)
					{
						matrix transform;
						float4 padding;
					};

					struct ps_input
					{
						float4 position : SV_POSITION;
						float2 texcoord : TEXCOORD0;
						float4 color : COLOR0;
					};

					Texture2D diffuse : register(t1);
					SamplerState state : register(s1);

					float4 ps_main(ps_input input) : SV_TARGET0
					{
						if (padding.z > 0)
							return input.color * diffuse.Sample(state, input.texcoord + padding.xy) * padding.w;

						return input.color * padding.w;
					};);

					ID3DBlob* blob = nullptr, * error = nullptr;
					result_code = D3DCompile(pixel_shader_code, strlen(pixel_shader_code), nullptr, nullptr, nullptr, "ps_main", get_ps_profile().data(), 0, 0, &blob, &error);
					if (result_code != S_OK || !get_compile_state(error).empty())
					{
						auto message = get_compile_state(error);
						d3d11_release(error);
						return get_exception(result_code, message);
					}

					result_code = context->CreatePixelShader((DWORD*)blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &immediate.pixel_shader);
					d3d11_release(blob);
					if (result_code != S_OK)
						return get_exception(result_code, "compile direct pixel shader");
				}

				if (!immediate.constant_buffer)
				{
					result_code = create_constant_buffer(&immediate.constant_buffer, sizeof(direct));
					if (result_code != S_OK)
						return get_exception(result_code, "compile direct constant buffer");
				}

				return core::expectation::met;
			}
			expects_graphics<void> d3d11_device::create_texture_2d(texture_2d* resource, DXGI_FORMAT internal_format)
			{
				VI_ASSERT(resource != nullptr, "resource should be set");
				d3d11_texture_2d* iresource = (d3d11_texture_2d*)resource;
				VI_ASSERT(iresource->view != nullptr, "resource should be set");

				D3D11_TEXTURE2D_DESC description;
				iresource->view->GetDesc(&description);
				iresource->format_mode = (format)description.Format;
				iresource->usage = (resource_usage)description.Usage;
				iresource->width = description.Width;
				iresource->height = description.Height;
				iresource->mip_levels = description.MipLevels;
				iresource->access_flags = (cpu_access)description.CPUAccessFlags;
				iresource->binding = (resource_bind)description.BindFlags;

				if (iresource->resource != nullptr || !((uint32_t)iresource->binding & (uint32_t)resource_bind::shader_input))
					return core::expectation::met;

				D3D11_SHADER_RESOURCE_VIEW_DESC SRV;
				ZeroMemory(&SRV, sizeof(SRV));

				if (internal_format == DXGI_FORMAT_UNKNOWN)
					SRV.Format = description.Format;
				else
					SRV.Format = internal_format;

				if (description.ArraySize > 1)
				{
					if (description.MiscFlags & D3D11_RESOURCE_MISC_TEXTURECUBE)
					{
						SRV.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
						SRV.TextureCube.MostDetailedMip = 0;
						SRV.TextureCube.MipLevels = description.MipLevels;
					}
					else if (description.SampleDesc.Count <= 1)
					{
						SRV.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
						SRV.Texture2DArray.MostDetailedMip = 0;
						SRV.Texture2DArray.MipLevels = description.MipLevels;
						SRV.Texture2DArray.FirstArraySlice = 0;
						SRV.Texture2DArray.ArraySize = description.ArraySize;
					}
					else
					{
						SRV.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMSARRAY;
						SRV.Texture2DMSArray.FirstArraySlice = 0;
						SRV.Texture2DMSArray.ArraySize = description.ArraySize;
					}
				}
				else if (description.SampleDesc.Count <= 1)
				{
					SRV.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
					SRV.Texture2D.MostDetailedMip = 0;
					SRV.Texture2D.MipLevels = description.MipLevels;
				}
				else
					SRV.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMS;

				d3d11_release(iresource->resource);
				HRESULT result_code = context->CreateShaderResourceView(iresource->view, &SRV, &iresource->resource);
				if (result_code != S_OK)
					return get_exception(result_code, "create texture 2d internal");

				return core::expectation::met;
			}
			expects_graphics<void> d3d11_device::create_texture_cube(texture_cube* resource, DXGI_FORMAT internal_format)
			{
				VI_ASSERT(resource != nullptr, "resource should be set");
				d3d11_texture_cube* iresource = (d3d11_texture_cube*)resource;
				VI_ASSERT(iresource->view != nullptr, "resource should be set");

				D3D11_TEXTURE2D_DESC description;
				iresource->view->GetDesc(&description);
				iresource->format_mode = (format)description.Format;
				iresource->usage = (resource_usage)description.Usage;
				iresource->width = description.Width;
				iresource->height = description.Height;
				iresource->mip_levels = description.MipLevels;
				iresource->access_flags = (cpu_access)description.CPUAccessFlags;
				iresource->binding = (resource_bind)description.BindFlags;

				if (iresource->resource != nullptr || !((uint32_t)iresource->binding & (uint32_t)resource_bind::shader_input))
					return core::expectation::met;

				D3D11_SHADER_RESOURCE_VIEW_DESC SRV;
				ZeroMemory(&SRV, sizeof(SRV));

				if (internal_format == DXGI_FORMAT_UNKNOWN)
					SRV.Format = description.Format;
				else
					SRV.Format = internal_format;

				if (description.ArraySize > 1)
				{
					if (description.MiscFlags & D3D11_RESOURCE_MISC_TEXTURECUBE)
					{
						SRV.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
						SRV.TextureCube.MostDetailedMip = 0;
						SRV.TextureCube.MipLevels = description.MipLevels;
					}
					else if (description.SampleDesc.Count <= 1)
					{
						SRV.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
						SRV.Texture2DArray.MostDetailedMip = 0;
						SRV.Texture2DArray.MipLevels = description.MipLevels;
						SRV.Texture2DArray.FirstArraySlice = 0;
						SRV.Texture2DArray.ArraySize = description.ArraySize;
					}
					else
					{
						SRV.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMSARRAY;
						SRV.Texture2DMSArray.FirstArraySlice = 0;
						SRV.Texture2DMSArray.ArraySize = description.ArraySize;
					}
				}
				else if (description.SampleDesc.Count <= 1)
				{
					SRV.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
					SRV.Texture2D.MostDetailedMip = 0;
					SRV.Texture2D.MipLevels = description.MipLevels;
				}
				else
					SRV.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMS;

				d3d11_release(iresource->resource);
				HRESULT result_code = context->CreateShaderResourceView(iresource->view, &SRV, &iresource->resource);
				if (result_code != S_OK)
					return get_exception(result_code, "create texture 2d internal");

				return core::expectation::met;
			}
			expects_graphics<ID3D11InputLayout*> d3d11_device::generate_input_layout(d3d11_shader* shader)
			{
				VI_ASSERT(shader != nullptr, "shader should be set");
				if (shader->vertex_layout != nullptr)
					return shader->vertex_layout;

				if (!shader->signature || !regs.layout || regs.layout->layout.empty())
					return graphics_exception("generate input layout: invalid argument");

				core::vector<D3D11_INPUT_ELEMENT_DESC> result;
				for (size_t i = 0; i < regs.layout->layout.size(); i++)
				{
					const input_layout::attribute& it = regs.layout->layout[i];
					D3D11_INPUT_CLASSIFICATION data_class = it.per_vertex ? D3D11_INPUT_PER_VERTEX_DATA : D3D11_INPUT_PER_INSTANCE_DATA;
					UINT step = it.per_vertex ? 0 : 1;

					if (it.format == attribute_type::matrix)
					{
						DXGI_FORMAT format = DXGI_FORMAT_R32G32B32A32_FLOAT;
						result.push_back({ it.semantic_name.c_str(), it.semantic_index + 0, format, it.slot, it.aligned_byte_offset + 0, data_class, step });
						result.push_back({ it.semantic_name.c_str(), it.semantic_index + 1, format, it.slot, it.aligned_byte_offset + 16, data_class, step });
						result.push_back({ it.semantic_name.c_str(), it.semantic_index + 2, format, it.slot, it.aligned_byte_offset + 32, data_class, step });
						result.push_back({ it.semantic_name.c_str(), it.semantic_index + 3, format, it.slot, it.aligned_byte_offset + 48, data_class, step });
						continue;
					}

					D3D11_INPUT_ELEMENT_DESC at;
					at.SemanticName = it.semantic_name.c_str();
					at.AlignedByteOffset = it.aligned_byte_offset;
					at.Format = DXGI_FORMAT_R32_FLOAT;
					at.SemanticIndex = it.semantic_index;
					at.InputSlot = it.slot;
					at.InstanceDataStepRate = step;
					at.InputSlotClass = data_class;

					switch (it.format)
					{
						case vitex::graphics::attribute_type::byte:
							if (it.components == 3)
								return graphics_exception("generate input layout: no 24bit support format for this type");
							else if (it.components == 1)
								at.Format = DXGI_FORMAT_R8_SNORM;
							else if (it.components == 2)
								at.Format = DXGI_FORMAT_R8G8_SNORM;
							else if (it.components == 4)
								at.Format = DXGI_FORMAT_R8G8B8A8_SNORM;
							break;
						case vitex::graphics::attribute_type::ubyte:
							if (it.components == 3)
								return graphics_exception("generate input layout: no 24bit support format for this type");
							else if (it.components == 1)
								at.Format = DXGI_FORMAT_R8_UNORM;
							else if (it.components == 2)
								at.Format = DXGI_FORMAT_R8G8_UNORM;
							else if (it.components == 4)
								at.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
							break;
						case vitex::graphics::attribute_type::half:
							if (it.components == 1)
								at.Format = DXGI_FORMAT_R16_FLOAT;
							else if (it.components == 2)
								at.Format = DXGI_FORMAT_R16G16_FLOAT;
							else if (it.components == 3)
								at.Format = DXGI_FORMAT_R11G11B10_FLOAT;
							else if (it.components == 4)
								at.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
							break;
						case vitex::graphics::attribute_type::floatf:
							if (it.components == 1)
								at.Format = DXGI_FORMAT_R32_FLOAT;
							else if (it.components == 2)
								at.Format = DXGI_FORMAT_R32G32_FLOAT;
							else if (it.components == 3)
								at.Format = DXGI_FORMAT_R32G32B32_FLOAT;
							else if (it.components == 4)
								at.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
							break;
						case vitex::graphics::attribute_type::intf:
							if (it.components == 1)
								at.Format = DXGI_FORMAT_R32_SINT;
							else if (it.components == 2)
								at.Format = DXGI_FORMAT_R32G32_SINT;
							else if (it.components == 3)
								at.Format = DXGI_FORMAT_R32G32B32_SINT;
							else if (it.components == 4)
								at.Format = DXGI_FORMAT_R32G32B32A32_SINT;
							break;
						case vitex::graphics::attribute_type::uint:
							if (it.components == 1)
								at.Format = DXGI_FORMAT_R32_UINT;
							else if (it.components == 2)
								at.Format = DXGI_FORMAT_R32G32_UINT;
							else if (it.components == 3)
								at.Format = DXGI_FORMAT_R32G32B32_UINT;
							else if (it.components == 4)
								at.Format = DXGI_FORMAT_R32G32B32A32_UINT;
							break;
						default:
							break;
					}

					result.push_back(std::move(at));
				}

				HRESULT result_code = context->CreateInputLayout(result.data(), (uint32_t)result.size(), shader->signature->GetBufferPointer(), shader->signature->GetBufferSize(), &shader->vertex_layout);
				if (result_code != S_OK)
					return get_exception(result_code, "generate input layout");

				return shader->vertex_layout;
			}
			HRESULT d3d11_device::create_constant_buffer(ID3D11Buffer** ibuffer, size_t size)
			{
				VI_ASSERT(ibuffer != nullptr, "buffers ptr should be set");

				D3D11_BUFFER_DESC description;
				ZeroMemory(&description, sizeof(description));
				description.Usage = D3D11_USAGE_DEFAULT;
				description.ByteWidth = (uint32_t)size;
				description.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
				description.CPUAccessFlags = 0;

				return context->CreateBuffer(&description, nullptr, ibuffer);
			}
			std::string_view d3d11_device::get_compile_state(ID3DBlob* error)
			{
				if (!error)
					return "";

				char* ptr = (char*)error->GetBufferPointer();
				return ptr ? ptr : "";
			}
			const std::string_view& d3d11_device::get_vs_profile()
			{
				return models.vertex;
			}
			const std::string_view& d3d11_device::get_ps_profile()
			{
				return models.pixel;
			}
			const std::string_view& d3d11_device::get_gs_profile()
			{
				return models.geometry;
			}
			const std::string_view& d3d11_device::get_hs_profile()
			{
				return models.hull;
			}
			const std::string_view& d3d11_device::get_cs_profile()
			{
				return models.compute;
			}
			const std::string_view& d3d11_device::get_ds_profile()
			{
				return models.domain;
			}
		}
	}
}
#endif
