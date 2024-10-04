#ifndef VI_GRAPHICS_H
#define VI_GRAPHICS_H
#include "trigonometry.h"
#include <iostream>
#include <functional>
#include <limits>
#define VI_VS (uint32_t)Vitex::Graphics::ShaderType::Vertex
#define VI_PS (uint32_t)Vitex::Graphics::ShaderType::Pixel
#define VI_GS (uint32_t)Vitex::Graphics::ShaderType::Geometry
#define VI_CS (uint32_t)Vitex::Graphics::ShaderType::Compute
#define VI_HS (uint32_t)Vitex::Graphics::ShaderType::Hull
#define VI_DS (uint32_t)Vitex::Graphics::ShaderType::Domain

struct SDL_SysWMinfo;
struct SDL_Cursor;
struct SDL_Window;
struct SDL_Surface;

namespace Vitex
{
	namespace Graphics
	{
		enum
		{
			WINDOW_SIZE = 128,
			JOINTS_SIZE = 96,
			UNITS_SIZE = 32
		};

		enum class AppState
		{
			Close,
			Terminating,
			Low_Memory,
			Enter_Background_Start,
			Enter_Background_End,
			Enter_Foreground_Start,
			Enter_Foreground_End
		};

		enum class WindowState
		{
			Show,
			Hide,
			Expose,
			Move,
			Resize,
			Size_Change,
			Minimize,
			Maximize,
			Restore,
			Enter,
			Leave,
			Focus,
			Blur,
			Close
		};

		enum class KeyCode
		{
			A = 4,
			B = 5,
			C = 6,
			D = 7,
			E = 8,
			F = 9,
			G = 10,
			H = 11,
			I = 12,
			J = 13,
			K = 14,
			L = 15,
			M = 16,
			N = 17,
			O = 18,
			P = 19,
			Q = 20,
			R = 21,
			S = 22,
			T = 23,
			U = 24,
			V = 25,
			W = 26,
			X = 27,
			Y = 28,
			Z = 29,
			D1 = 30,
			D2 = 31,
			D3 = 32,
			D4 = 33,
			D5 = 34,
			D6 = 35,
			D7 = 36,
			D8 = 37,
			D9 = 38,
			D0 = 39,
			Return = 40,
			Escape = 41,
			Backspace = 42,
			Tab = 43,
			Space = 44,
			Minus = 45,
			Equals = 46,
			LeftBracket = 47,
			RightBracket = 48,
			Backslash = 49,
			NonUsHash = 50,
			Semicolon = 51,
			Apostrophe = 52,
			Grave = 53,
			Comma = 54,
			Period = 55,
			Slash = 56,
			Capslock = 57,
			F1 = 58,
			F2 = 59,
			F3 = 60,
			F4 = 61,
			F5 = 62,
			F6 = 63,
			F7 = 64,
			F8 = 65,
			F9 = 66,
			F10 = 67,
			F11 = 68,
			F12 = 69,
			PrintScreen = 70,
			ScrollLock = 71,
			Pause = 72,
			Insert = 73,
			Home = 74,
			PageUp = 75,
			Delete = 76,
			End = 77,
			PageDown = 78,
			Right = 79,
			Left = 80,
			Down = 81,
			Up = 82,
			NumLockClear = 83,
			KpDivide = 84,
			KpMultiply = 85,
			KpMinus = 86,
			KpPlus = 87,
			KpEnter = 88,
			Kp1 = 89,
			Kp2 = 90,
			Kp3 = 91,
			Kp4 = 92,
			Kp5 = 93,
			Kp6 = 94,
			Kp7 = 95,
			Kp8 = 96,
			Kp9 = 97,
			Kp0 = 98,
			KpPeriod = 99,
			NonUsBackslash = 100,
			App0 = 101,
			Power = 102,
			KpEquals = 103,
			F13 = 104,
			F14 = 105,
			F15 = 106,
			F16 = 107,
			F17 = 108,
			F18 = 109,
			F19 = 110,
			F20 = 111,
			F21 = 112,
			F22 = 113,
			F23 = 114,
			F24 = 115,
			Execute = 116,
			Help = 117,
			Menu = 118,
			Select = 119,
			Stop = 120,
			Again = 121,
			Undo = 122,
			Cut = 123,
			Copy = 124,
			Paste = 125,
			Find = 126,
			Mute = 127,
			VolumeUp = 128,
			VolumeDown = 129,
			KpComma = 133,
			KpEqualsAs400 = 134,
			International1 = 135,
			International2 = 136,
			International3 = 137,
			International4 = 138,
			International5 = 139,
			International6 = 140,
			International7 = 141,
			International8 = 142,
			International9 = 143,
			Lang1 = 144,
			Lang2 = 145,
			Lang3 = 146,
			Lang4 = 147,
			Lang5 = 148,
			Lang6 = 149,
			Lang7 = 150,
			Lang8 = 151,
			Lang9 = 152,
			Alterase = 153,
			SysReq = 154,
			Cancel = 155,
			Clear = 156,
			Prior = 157,
			Return2 = 158,
			Separator = 159,
			Output = 160,
			Operation = 161,
			ClearAgain = 162,
			CrSelect = 163,
			ExSelect = 164,
			Kp00 = 176,
			Kp000 = 177,
			ThousandsSeparator = 178,
			DecimalsSeparator = 179,
			CurrencyUnit = 180,
			CurrencySubunit = 181,
			KpLeftParen = 182,
			KpRightParen = 183,
			KpLeftBrace = 184,
			KpRightBrace = 185,
			KpTab = 186,
			KpBackspace = 187,
			KpA = 188,
			KpB = 189,
			KpC = 190,
			KpD = 191,
			KpE = 192,
			KpF = 193,
			KpXOR = 194,
			KpPower = 195,
			KpPercent = 196,
			KpLess = 197,
			KpGreater = 198,
			KpAmpersand = 199,
			KpDBLAmpersand = 200,
			KpVerticalBar = 201,
			KpDBLVerticalBar = 202,
			KpColon = 203,
			KpHash = 204,
			KpSpace = 205,
			KpAt = 206,
			KpExclaim = 207,
			KpMemStore = 208,
			KpMemRecall = 209,
			KpMemClear = 210,
			KpMemAdd = 211,
			KpMemSubtract = 212,
			KpMemMultiply = 213,
			KpMemDivide = 214,
			KpPlusMinus = 215,
			KpClear = 216,
			KpClearEntry = 217,
			KpBinary = 218,
			KpOctal = 219,
			KpDecimal = 220,
			KpHexadecimal = 221,
			LeftControl = 224,
			LeftShift = 225,
			LeftAlt = 226,
			LeftGUI = 227,
			RightControl = 228,
			RightShift = 229,
			RightAlt = 230,
			RightGUI = 231,
			Mode = 257,
			AudioNext = 258,
			AudioPrev = 259,
			AudioStop = 260,
			AudioPlay = 261,
			AudioMute = 262,
			MediaSelect = 263,
			WWW = 264,
			Mail = 265,
			Calculator = 266,
			Computer = 267,
			AcSearch = 268,
			AcHome = 269,
			AcBack = 270,
			AcForward = 271,
			AcStop = 272,
			AcRefresh = 273,
			AcBookmarks = 274,
			BrightnessDown = 275,
			BrightnessUp = 276,
			DisplaySwitch = 277,
			KbIllumToggle = 278,
			KbIllumDown = 279,
			KbIllumUp = 280,
			Eject = 281,
			Sleep = 282,
			App1 = 283,
			App2 = 284,
			AudioRewind = 285,
			AudioFastForward = 286,
			CursorLeft = 287,
			CursorMiddle = 288,
			CursorRight = 289,
			CursorX1 = 290,
			CursorX2 = 291,
			None = 292
		};

		enum class KeyMod
		{
			None = 0x0000,
			LeftShift = 0x0001,
			RightShift = 0x0002,
			LeftControl = 0x0040,
			RightControl = 0x0080,
			LeftAlt = 0x0100,
			RightAlt = 0x0200,
			LeftGUI = 0x0400,
			RightGUI = 0x0800,
			Num = 0x1000,
			Caps = 0x2000,
			Mode = 0x4000,
			Reserved = 0x8000,
			Shift = LeftShift | RightShift,
			Control = LeftControl | RightControl,
			Alt = LeftAlt | RightAlt,
			GUI = LeftGUI | RightGUI
		};

		enum class AlertType
		{
			None = 0,
			Error = 0x00000010,
			Warning = 0x00000020,
			Info = 0x00000040
		};

		enum class AlertConfirm
		{
			None = 0,
			Return = 0x00000002,
			Escape = 0x00000001
		};

		enum class JoyStickHat
		{
			Center = 0x00,
			Up = 0x01,
			Right = 0x02,
			Down = 0x04,
			Left = 0x08,
			Right_Up = 0x02 | 0x01,
			Right_Down = 0x02 | 0x04,
			Left_Up = 0x08 | 0x01,
			Left_Down = 0x08 | 0x04
		};

		enum class RenderBackend
		{
			None,
			Automatic,
			D3D11,
			OGL
		};

		enum class VSync
		{
			Off,
			Frequency_X1,
			Frequency_X2,
			Frequency_X3,
			Frequency_X4,
			On = Frequency_X1
		};

		enum class SurfaceTarget
		{
			T0 = 1,
			T1 = 2,
			T2 = 3,
			T3 = 4,
			T4 = 5,
			T5 = 6,
			T6 = 7,
			T7 = 8
		};

		enum class PrimitiveTopology
		{
			Invalid,
			Point_List,
			Line_List,
			Line_Strip,
			Triangle_List,
			Triangle_Strip,
			Line_List_Adj,
			Line_Strip_Adj,
			Triangle_List_Adj,
			Triangle_Strip_Adj
		};

		enum class Format
		{
			Unknown = 0,
			A8_Unorm = 65,
			D16_Unorm = 55,
			D24_Unorm_S8_Uint = 45,
			D32_Float = 40,
			R10G10B10A2_Uint = 25,
			R10G10B10A2_Unorm = 24,
			R11G11B10_Float = 26,
			R16G16B16A16_Float = 10,
			R16G16B16A16_Sint = 14,
			R16G16B16A16_Snorm = 13,
			R16G16B16A16_Uint = 12,
			R16G16B16A16_Unorm = 11,
			R16G16_Float = 34,
			R16G16_Sint = 38,
			R16G16_Snorm = 37,
			R16G16_Uint = 36,
			R16G16_Unorm = 35,
			R16_Float = 54,
			R16_Sint = 59,
			R16_Snorm = 58,
			R16_Uint = 57,
			R16_Unorm = 56,
			R1_Unorm = 66,
			R32G32B32A32_Float = 2,
			R32G32B32A32_Sint = 4,
			R32G32B32A32_Uint = 3,
			R32G32B32_Float = 6,
			R32G32B32_Sint = 8,
			R32G32B32_Uint = 7,
			R32G32_Float = 16,
			R32G32_Sint = 18,
			R32G32_Uint = 17,
			R32_Float = 41,
			R32_Sint = 43,
			R32_Uint = 42,
			R8G8B8A8_Sint = 32,
			R8G8B8A8_Snorm = 31,
			R8G8B8A8_Uint = 30,
			R8G8B8A8_Unorm = 28,
			R8G8B8A8_Unorm_SRGB = 29,
			R8G8_B8G8_Unorm = 68,
			R8G8_Sint = 52,
			R8G8_Snorm = 51,
			R8G8_Uint = 50,
			R8G8_Unorm = 49,
			R8_Sint = 64,
			R8_Snorm = 63,
			R8_Uint = 62,
			R8_Unorm = 61,
			R9G9B9E5_Share_Dexp = 67
		};

		enum class ResourceMap
		{
			Read = 1,
			Write = 2,
			Read_Write = 3,
			Write_Discard = 4,
			Write_No_Overwrite = 5
		};

		enum class ResourceUsage
		{
			Default = 0,
			Immutable = 1,
			Dynamic = 2,
			Staging = 3
		};

		enum class ShaderModel
		{
			Invalid,
			Auto,
			HLSL_1_0 = 100,
			HLSL_2_0 = 200,
			HLSL_3_0 = 300,
			HLSL_4_0 = 400,
			HLSL_4_1 = 410,
			HLSL_5_0 = 500,
			GLSL_1_1_0 = 110,
			GLSL_1_2_0 = 120,
			GLSL_1_3_0 = 130,
			GLSL_1_4_0 = 140,
			GLSL_1_5_0 = 150,
			GLSL_3_3_0 = 330,
			GLSL_4_0_0 = 400,
			GLSL_4_1_0 = 410,
			GLSL_4_2_0 = 420,
			GLSL_4_3_0 = 430,
			GLSL_4_4_0 = 440,
			GLSL_4_5_0 = 450,
			GLSL_4_6_0 = 460
		};

		enum class ResourceBind
		{
			Vertex_Buffer = 0x1L,
			Index_Buffer = 0x2L,
			Constant_Buffer = 0x4L,
			Shader_Input = 0x8L,
			Stream_Output = 0x10L,
			Render_Target = 0x20L,
			Depth_Stencil = 0x40L,
			Unordered_Access = 0x80L
		};

		enum class CPUAccess
		{
			None = 0,
			Write = 0x10000L,
			Read = 0x20000L
		};

		enum class DepthWrite
		{
			Zero,
			All
		};

		enum class Comparison
		{
			Never = 1,
			Less = 2,
			Equal = 3,
			Less_Equal = 4,
			Greater = 5,
			Not_Equal = 6,
			Greater_Equal = 7,
			Always = 8
		};

		enum class StencilOperation
		{
			Keep = 1,
			Zero = 2,
			Replace = 3,
			SAT_Add = 4,
			SAT_Subtract = 5,
			Invert = 6,
			Add = 7,
			Subtract = 8
		};

		enum class Blend
		{
			Zero = 1,
			One = 2,
			Source_Color = 3,
			Source_Color_Invert = 4,
			Source_Alpha = 5,
			Source_Alpha_Invert = 6,
			Destination_Alpha = 7,
			Destination_Alpha_Invert = 8,
			Destination_Color = 9,
			Destination_Color_Invert = 10,
			Source_Alpha_SAT = 11,
			Blend_Factor = 14,
			Blend_Factor_Invert = 15,
			Source1_Color = 16,
			Source1_Color_Invert = 17,
			Source1_Alpha = 18,
			Source1_Alpha_Invert = 19
		};

		enum class SurfaceFill
		{
			Wireframe = 2,
			Solid = 3
		};

		enum class PixelFilter
		{
			Min_Mag_Mip_Point = 0,
			Min_Mag_Point_Mip_Linear = 0x1,
			Min_Point_Mag_Linear_Mip_Point = 0x4,
			Min_Point_Mag_Mip_Linear = 0x5,
			Min_Linear_Mag_Mip_Point = 0x10,
			Min_Linear_Mag_Point_Mip_Linear = 0x11,
			Min_Mag_Linear_Mip_Point = 0x14,
			Min_Mag_Mip_Linear = 0x15,
			Anistropic = 0x55,
			Compare_Min_Mag_Mip_Point = 0x80,
			Compare_Min_Mag_Point_Mip_Linear = 0x81,
			Compare_Min_Point_Mag_Linear_Mip_Point = 0x84,
			Compare_Min_Point_Mag_Mip_Linear = 0x85,
			Compare_Min_Linear_Mag_Mip_Point = 0x90,
			Compare_Min_Linear_Mag_Point_Mip_Linear = 0x91,
			Compare_Min_Mag_Linear_Mip_Point = 0x94,
			Compare_Min_Mag_Mip_Linear = 0x95,
			Compare_Anistropic = 0xd5
		};

		enum class TextureAddress
		{
			Wrap = 1,
			Mirror = 2,
			Clamp = 3,
			Border = 4,
			Mirror_Once = 5
		};

		enum class ColorWriteEnable
		{
			Red = 1,
			Green = 2,
			Blue = 4,
			Alpha = 8,
			All = (((Red | Green) | Blue) | Alpha)
		};

		enum class BlendOperation
		{
			Add = 1,
			Subtract = 2,
			Subtract_Reverse = 3,
			Min = 4,
			Max = 5
		};

		enum class VertexCull
		{
			None = 1,
			Front = 2,
			Back = 3
		};

		enum class ShaderCompile
		{
			Debug = 1ll << 0,
			Skip_Validation = 1ll << 1,
			Skip_Optimization = 1ll << 2,
			Matrix_Row_Major = 1ll << 3,
			Matrix_Column_Major = 1ll << 4,
			Partial_Precision = 1ll << 5,
			FOE_VS_No_OPT = 1ll << 6,
			FOE_PS_No_OPT = 1ll << 7,
			No_Preshader = 1ll << 8,
			Avoid_Flow_Control = 1ll << 9,
			Prefer_Flow_Control = 1ll << 10,
			Enable_Strictness = 1ll << 11,
			Enable_Backwards_Compatibility = 1ll << 12,
			IEEE_Strictness = 1ll << 13,
			Optimization_Level0 = 1ll << 14,
			Optimization_Level1 = 0,
			Optimization_Level2 = (1ll << 14) | (1ll << 15),
			Optimization_Level3 = 1ll << 15,
			Reseed_X16 = 1ll << 16,
			Reseed_X17 = 1ll << 17,
			Picky = 1ll << 18
		};

		enum class ResourceMisc
		{
			None = 0,
			Generate_Mips = 0x1L,
			Shared = 0x2L,
			Texture_Cube = 0x4L,
			Draw_Indirect_Args = 0x10L,
			Buffer_Allow_Raw_Views = 0x20L,
			Buffer_Structured = 0x40L,
			Clamp = 0x80L,
			Shared_Keyed_Mutex = 0x100L,
			GDI_Compatible = 0x200L,
			Shared_NT_Handle = 0x800L,
			Restricted_Content = 0x1000L,
			Restrict_Shared = 0x2000L,
			Restrict_Shared_Driver = 0x4000L,
			Guarded = 0x8000L,
			Tile_Pool = 0x20000L,
			Tiled = 0x40000L
		};

		enum class DisplayCursor
		{
			None = -1,
			Arrow = 0,
			TextInput,
			ResizeAll,
			ResizeNS,
			ResizeEW,
			ResizeNESW,
			ResizeNWSE,
			Hand,
			Crosshair,
			Wait,
			Progress,
			No,
			Count
		};

		enum class ShaderType
		{
			Vertex = 1,
			Pixel = 2,
			Geometry = 4,
			Hull = 8,
			Domain = 16,
			Compute = 32,
			All = Vertex | Pixel | Geometry | Hull | Domain | Compute
		};

		enum class ShaderLang
		{
			None,
			SPV,
			MSL,
			HLSL,
			GLSL,
			GLSL_ES
		};

		enum class AttributeType
		{
			Byte,
			Ubyte,
			Half,
			Float,
			Int,
			Uint,
			Matrix
		};

		enum class OrientationType
		{
			Unknown,
			Landscape,
			LandscapeFlipped,
			Portrait,
			PortraitFlipped
		};

		inline ColorWriteEnable operator |(ColorWriteEnable A, ColorWriteEnable B)
		{
			return static_cast<ColorWriteEnable>(static_cast<size_t>(A) | static_cast<size_t>(B));
		}
		inline ResourceMap operator |(ResourceMap A, ResourceMap B)
		{
			return static_cast<ResourceMap>(static_cast<size_t>(A) | static_cast<size_t>(B));
		}
		inline ShaderCompile operator |(ShaderCompile A, ShaderCompile B)
		{
			return static_cast<ShaderCompile>(static_cast<size_t>(A) | static_cast<size_t>(B));
		}
		inline ResourceMisc operator |(ResourceMisc A, ResourceMisc B)
		{
			return static_cast<ResourceMisc>(static_cast<size_t>(A) | static_cast<size_t>(B));
		}
		inline ResourceBind operator |(ResourceBind A, ResourceBind B)
		{
			return static_cast<ResourceBind>(static_cast<size_t>(A) | static_cast<size_t>(B));
		}

		class Shader;

		class Texture2D;

		class SkinMeshBuffer;

		class GraphicsDevice;

		class Activity;

		typedef std::function<void(AppState)> AppStateChangeCallback;
		typedef std::function<void(WindowState, int, int)> WindowStateChangeCallback;
		typedef std::function<void(KeyCode, KeyMod, int, int, bool)> KeyStateCallback;
		typedef std::function<void(char*, int, int)> InputEditCallback;
		typedef std::function<void(char*, int)> InputCallback;
		typedef std::function<void(int, int, int, int)> CursorMoveCallback;
		typedef std::function<void(int, int, bool)> CursorWheelStateCallback;
		typedef std::function<void(int, int, int)> JoyStickAxisMoveCallback;
		typedef std::function<void(int, int, int, int)> JoyStickBallMoveCallback;
		typedef std::function<void(enum JoyStickHat, int, int)> JoyStickHatMoveCallback;
		typedef std::function<void(int, int, bool)> JoyStickKeyStateCallback;
		typedef std::function<void(int, bool)> JoyStickStateCallback;
		typedef std::function<void(int, int, int)> ControllerAxisMoveCallback;
		typedef std::function<void(int, int, bool)> ControllerKeyStateCallback;
		typedef std::function<void(int, int)> ControllerStateCallback;
		typedef std::function<void(int, int, float, float, float, float, float)> TouchMoveCallback;
		typedef std::function<void(int, int, float, float, float, float, float, bool)> TouchStateCallback;
		typedef std::function<void(int, int, int, float, float, float, bool)> GestureStateCallback;
		typedef std::function<void(int, int, float, float, float, float)> MultiGestureStateCallback;
		typedef std::function<void(const std::string_view&)> DropFileCallback;
		typedef std::function<void(const std::string_view&)> DropTextCallback;
		typedef std::function<void(GraphicsDevice*)> RenderThreadCallback;

		struct VI_OUT Alert
		{
			friend Activity;

		private:
			struct Element
			{
				Core::String Name;
				int Id = -1, Flags = 0;
			};

		private:
			Core::String Name;
			Core::String Data;
			Core::Vector<Element> Buttons;
			std::function<void(int)> Done;
			AlertType View;
			Activity* Base;
			bool Waiting;

		public:
			Alert(Activity* From) noexcept;
			void Setup(AlertType Type, const std::string_view& Title, const std::string_view& Text);
			void Button(AlertConfirm Confirm, const std::string_view& Text, int Id);
			void Result(std::function<void(int)>&& Callback);

		private:
			void Dispatch();
		};

		struct VI_OUT DisplayInfo
		{
			Core::String Name;
			OrientationType Orientation;
			float DiagonalDPI = 1.0f;
			float HorizontalDPI = 1.0f;
			float VerticalDPI = 1.0f;
			uint32_t PixelFormat = 0;
			uint32_t PhysicalWidth = 0;
			uint32_t PhysicalHeight = 0;
			uint32_t RefreshRate = 0;
			int32_t Width = 0;
			int32_t Height = 0;
			int32_t X = 0;
			int32_t Y = 0;
		};

		struct VI_OUT EventConsumers
		{
			friend Activity;

		private:
			Core::UnorderedMap<uint32_t, Activity*> Consumers;

		public:
			void Push(Activity* Value);
			void Pop(Activity* Value);
			Activity* Find(uint32_t Id) const;
		};

		struct VI_OUT KeyMap
		{
			KeyCode Key;
			KeyMod Mod;
			bool Normal;

			KeyMap() noexcept;
			KeyMap(const KeyCode& Value) noexcept;
			KeyMap(const KeyMod& Value) noexcept;
			KeyMap(const KeyCode& Value, const KeyMod& Control) noexcept;
		};

		struct VI_OUT MappedSubresource
		{
			void* Pointer = nullptr;
			uint32_t RowPitch = 0;
			uint32_t DepthPitch = 0;
		};

		struct VI_OUT Viewport
		{
			float TopLeftX;
			float TopLeftY;
			float Width;
			float Height;
			float MinDepth;
			float MaxDepth;
		};

		struct VI_OUT RenderTargetBlendState
		{
			Blend SrcBlend;
			Blend DestBlend;
			BlendOperation BlendOperationMode;
			Blend SrcBlendAlpha;
			Blend DestBlendAlpha;
			BlendOperation BlendOperationAlpha;
			uint8_t RenderTargetWriteMask;
			bool BlendEnable;
		};

		struct VI_OUT BasicEffectBuffer
		{

		};

		class GraphicsException final : public Core::BasicException
		{
		private:
			int ErrorCode;

		public:
			VI_OUT GraphicsException(Core::String&& Message);
			VI_OUT GraphicsException(int ErrorCode, Core::String&& Message);
			VI_OUT const char* type() const noexcept override;
			VI_OUT int error_code() const noexcept;
		};

		class VideoException final : public Core::BasicException
		{
		public:
			VI_OUT VideoException();
			VI_OUT VideoException(GraphicsException&& Other);
			VI_OUT const char* type() const noexcept override;
		};

		template <typename V>
		using ExpectsGraphics = Core::Expects<V, GraphicsException>;

		template <typename V>
		using ExpectsVideo = Core::Expects<V, VideoException>;

		class VI_OUT Surface : public Core::Reference<Surface>
		{
		private:
			SDL_Surface* Handle;

		public:
			Surface() noexcept;
			Surface(SDL_Surface* From) noexcept;
			~Surface() noexcept;
			void SetHandle(SDL_Surface* From);
			void Lock();
			void Unlock();
			int GetWidth() const;
			int GetHeight() const;
			int GetPitch() const;
			void* GetPixels() const;
			void* GetResource() const;
		};

		class VI_OUT DepthStencilState : public Core::Reference<DepthStencilState>
		{
		public:
			struct Desc
			{
				StencilOperation FrontFaceStencilFailOperation;
				StencilOperation FrontFaceStencilDepthFailOperation;
				StencilOperation FrontFaceStencilPassOperation;
				Comparison FrontFaceStencilFunction;
				StencilOperation BackFaceStencilFailOperation;
				StencilOperation BackFaceStencilDepthFailOperation;
				StencilOperation BackFaceStencilPassOperation;
				Comparison BackFaceStencilFunction;
				DepthWrite DepthWriteMask;
				Comparison DepthFunction;
				uint8_t StencilReadMask;
				uint8_t StencilWriteMask;
				bool DepthEnable;
				bool StencilEnable;
			};

		protected:
			Desc State;

		protected:
			DepthStencilState(const Desc& I) noexcept;

		public:
			virtual ~DepthStencilState() noexcept;
			virtual void* GetResource() const = 0;
			Desc GetState() const;
		};

		class VI_OUT RasterizerState : public Core::Reference<RasterizerState>
		{
		public:
			struct Desc
			{
				SurfaceFill FillMode;
				VertexCull CullMode;
				float DepthBiasClamp;
				float SlopeScaledDepthBias;
				int DepthBias;
				bool FrontCounterClockwise;
				bool DepthClipEnable;
				bool ScissorEnable;
				bool MultisampleEnable;
				bool AntialiasedLineEnable;
			};

		protected:
			Desc State;

		protected:
			RasterizerState(const Desc& I) noexcept;

		public:
			virtual ~RasterizerState() noexcept;
			virtual void* GetResource() const = 0;
			Desc GetState() const;
		};

		class VI_OUT BlendState : public Core::Reference<BlendState>
		{
		public:
			struct Desc
			{
				RenderTargetBlendState RenderTarget[8];
				bool AlphaToCoverageEnable;
				bool IndependentBlendEnable;
			};

		protected:
			Desc State;

		protected:
			BlendState(const Desc& I) noexcept;

		public:
			virtual ~BlendState() noexcept;
			virtual void* GetResource() const = 0;
			Desc GetState() const;
		};

		class VI_OUT SamplerState : public Core::Reference<SamplerState>
		{
		public:
			struct Desc
			{
				Comparison ComparisonFunction;
				TextureAddress AddressU;
				TextureAddress AddressV;
				TextureAddress AddressW;
				PixelFilter Filter;
				float MipLODBias;
				uint32_t MaxAnisotropy;
				float BorderColor[4];
				float MinLOD;
				float MaxLOD;
			};

		protected:
			Desc State;

		protected:
			SamplerState(const Desc& I) noexcept;

		public:
			virtual ~SamplerState() noexcept;
			virtual void* GetResource() const = 0;
			Desc GetState() const;
		};

		class VI_OUT InputLayout : public Core::Reference<InputLayout>
		{
		public:
			struct Attribute
			{
				Core::String SemanticName;
				uint32_t SemanticIndex;
				AttributeType Format;
				uint32_t Components;
				uint32_t AlignedByteOffset;
				uint32_t Slot = 0;
				bool PerVertex = true;
			};

		public:
			struct Desc
			{
				Core::Vector<Attribute> Attributes;
				Shader* Source = nullptr;
			};

		protected:
			Core::Vector<Attribute> Layout;

		protected:
			InputLayout(const Desc& I) noexcept;

		public:
			virtual ~InputLayout() noexcept;
			virtual void* GetResource() const = 0;
			const Core::Vector<Attribute>& GetAttributes() const;
		};

		class VI_OUT Shader : public Core::Reference<Shader>
		{
		public:
			struct Desc
			{
				Compute::ProcIncludeCallback Include = nullptr;
				Compute::Preprocessor::Desc Features;
				Core::Vector<Core::String> Defines;
				Core::String Filename;
				Core::String Data;
				ShaderType Stage = ShaderType::All;
			};

		protected:
			Shader(const Desc& I) noexcept;

		public:
			virtual ~Shader() noexcept = default;
			virtual bool IsValid() const = 0;
		};

		class VI_OUT ElementBuffer : public Core::Reference<ElementBuffer>
		{
		public:
			struct Desc
			{
				void* Elements = nullptr;
				uint32_t StructureByteStride = 0;
				uint32_t ElementWidth = 0;
				uint32_t ElementCount = 0;
				CPUAccess AccessFlags = CPUAccess::None;
				ResourceUsage Usage = ResourceUsage::Default;
				ResourceBind BindFlags = ResourceBind::Vertex_Buffer;
				ResourceMisc MiscFlags = ResourceMisc::None;
				bool Writable = false;
			};

		protected:
			size_t Elements;
			size_t Stride;

		protected:
			ElementBuffer(const Desc& I) noexcept;

		public:
			virtual ~ElementBuffer() noexcept = default;
			virtual void* GetResource() const = 0;
			size_t GetElements() const;
			size_t GetStride() const;
		};

		class VI_OUT MeshBuffer : public Core::Reference<MeshBuffer>
		{
		public:
			struct Desc
			{
				Core::Vector<Trigonometry::Vertex> Elements;
				Core::Vector<int> Indices;
				CPUAccess AccessFlags = CPUAccess::None;
				ResourceUsage Usage = ResourceUsage::Default;
			};

		protected:
			ElementBuffer* VertexBuffer;
			ElementBuffer* IndexBuffer;

		public:
			Trigonometry::Matrix4x4 Transform;
			Core::String Name;

		protected:
			MeshBuffer(const Desc& I) noexcept;

		public:
			virtual ~MeshBuffer() noexcept;
			virtual Core::Unique<Trigonometry::Vertex> GetElements(GraphicsDevice* Device) const = 0;
			ElementBuffer* GetVertexBuffer() const;
			ElementBuffer* GetIndexBuffer() const;
		};

		class VI_OUT SkinMeshBuffer : public Core::Reference<SkinMeshBuffer>
		{
		public:
			struct Desc
			{
				Core::Vector<Trigonometry::SkinVertex> Elements;
				Core::Vector<int> Indices;
				CPUAccess AccessFlags = CPUAccess::None;
				ResourceUsage Usage = ResourceUsage::Default;
			};

		protected:
			ElementBuffer* VertexBuffer;
			ElementBuffer* IndexBuffer;

		public:
			Trigonometry::Matrix4x4 Transform;
			Core::UnorderedMap<size_t, size_t> Joints;
			Core::String Name;

		protected:
			SkinMeshBuffer(const Desc& I) noexcept;

		public:
			virtual ~SkinMeshBuffer() noexcept;
			virtual Core::Unique<Trigonometry::SkinVertex> GetElements(GraphicsDevice* Device) const = 0;
			ElementBuffer* GetVertexBuffer() const;
			ElementBuffer* GetIndexBuffer() const;
		};

		class VI_OUT InstanceBuffer : public Core::Reference<InstanceBuffer>
		{
		public:
			struct Desc
			{
				GraphicsDevice* Device = nullptr;
				uint32_t ElementWidth = sizeof(Trigonometry::ElementVertex);
				uint32_t ElementLimit = 100;
			};

		protected:
			Core::Vector<Trigonometry::ElementVertex> Array;
			ElementBuffer* Elements;
			GraphicsDevice* Device;
			size_t ElementLimit;
			size_t ElementWidth;
			bool Sync;

		protected:
			InstanceBuffer(const Desc& I) noexcept;

		public:
			virtual ~InstanceBuffer() noexcept;
			Core::Vector<Trigonometry::ElementVertex>& GetArray();
			ElementBuffer* GetElements() const;
			GraphicsDevice* GetDevice() const;
			size_t GetElementLimit() const;
		};

		class VI_OUT Texture2D : public Core::Reference<Texture2D>
		{
		public:
			struct Desc
			{
				CPUAccess AccessFlags = CPUAccess::None;
				Format FormatMode = Format::R8G8B8A8_Unorm;
				ResourceUsage Usage = ResourceUsage::Default;
				ResourceBind BindFlags = ResourceBind::Shader_Input;
				ResourceMisc MiscFlags = ResourceMisc::None;
				void* Data = nullptr;
				uint32_t RowPitch = 0;
				uint32_t DepthPitch = 0;
				uint32_t Width = WINDOW_SIZE;
				uint32_t Height = WINDOW_SIZE;
				int MipLevels = 1;
				bool Writable = false;
			};

		protected:
			CPUAccess AccessFlags;
			Format FormatMode;
			ResourceUsage Usage;
			ResourceBind Binding;
			uint32_t Width, Height;
			uint32_t MipLevels;

		protected:
			Texture2D() noexcept;
			Texture2D(const Desc& I) noexcept;

		public:
			virtual ~Texture2D() = default;
			virtual void* GetResource() const = 0;
			CPUAccess GetAccessFlags() const;
			Format GetFormatMode() const;
			ResourceUsage GetUsage() const;
			ResourceBind GetBinding() const;
			uint32_t GetWidth() const;
			uint32_t GetHeight() const;
			uint32_t GetMipLevels() const;
		};

		class VI_OUT Texture3D : public Core::Reference<Texture3D>
		{
		public:
			struct Desc
			{
				CPUAccess AccessFlags = CPUAccess::None;
				Format FormatMode = Format::R8G8B8A8_Unorm;
				ResourceUsage Usage = ResourceUsage::Default;
				ResourceBind BindFlags = ResourceBind::Shader_Input;
				ResourceMisc MiscFlags = ResourceMisc::None;
				uint32_t Width = WINDOW_SIZE;
				uint32_t Height = WINDOW_SIZE;
				uint32_t Depth = 1;
				int MipLevels = 1;
				bool Writable = false;
			};

		protected:
			CPUAccess AccessFlags;
			Format FormatMode;
			ResourceUsage Usage;
			ResourceBind Binding;
			uint32_t Width, Height;
			uint32_t MipLevels;
			uint32_t Depth;

		protected:
			Texture3D();

		public:
			virtual ~Texture3D() = default;
			virtual void* GetResource() = 0;
			CPUAccess GetAccessFlags() const;
			Format GetFormatMode() const;
			ResourceUsage GetUsage() const;
			ResourceBind GetBinding() const;
			uint32_t GetWidth() const;
			uint32_t GetHeight() const;
			uint32_t GetDepth() const;
			uint32_t GetMipLevels() const;
		};

		class VI_OUT TextureCube : public Core::Reference<TextureCube>
		{
		public:
			struct Desc
			{
				CPUAccess AccessFlags = CPUAccess::None;
				Format FormatMode = Format::R8G8B8A8_Unorm;
				ResourceUsage Usage = ResourceUsage::Default;
				ResourceBind BindFlags = ResourceBind::Shader_Input;
				ResourceMisc MiscFlags = ResourceMisc::Texture_Cube;
				uint32_t Width = WINDOW_SIZE;
				uint32_t Height = WINDOW_SIZE;
				int MipLevels = 1;
				bool Writable = false;
			};

		protected:
			CPUAccess AccessFlags;
			Format FormatMode;
			ResourceUsage Usage;
			ResourceBind Binding;
			uint32_t Width, Height;
			uint32_t MipLevels;

		protected:
			TextureCube() noexcept;
			TextureCube(const Desc& I) noexcept;

		public:
			virtual ~TextureCube() = default;
			virtual void* GetResource() const = 0;
			CPUAccess GetAccessFlags() const;
			Format GetFormatMode() const;
			ResourceUsage GetUsage() const;
			ResourceBind GetBinding() const;
			uint32_t GetWidth() const;
			uint32_t GetHeight() const;
			uint32_t GetMipLevels() const;
		};

		class VI_OUT DepthTarget2D : public Core::Reference<DepthTarget2D>
		{
		public:
			struct Desc
			{
				CPUAccess AccessFlags = CPUAccess::None;
				ResourceUsage Usage = ResourceUsage::Default;
				Format FormatMode = Format::D24_Unorm_S8_Uint;
				uint32_t Width = WINDOW_SIZE;
				uint32_t Height = WINDOW_SIZE;
			};

		protected:
			Texture2D* Resource;
			Viewport Viewarea;

		protected:
			DepthTarget2D(const Desc& I) noexcept;

		public:
			virtual ~DepthTarget2D() noexcept;
			virtual void* GetResource() const = 0;
			virtual uint32_t GetWidth() const = 0;
			virtual uint32_t GetHeight() const = 0;
			Texture2D* GetTarget();
			const Graphics::Viewport& GetViewport() const;
		};

		class VI_OUT DepthTargetCube : public Core::Reference<DepthTargetCube>
		{
		public:
			struct Desc
			{
				CPUAccess AccessFlags = CPUAccess::None;
				ResourceUsage Usage = ResourceUsage::Default;
				Format FormatMode = Format::D24_Unorm_S8_Uint;
				uint32_t Size = 512;
			};

		protected:
			TextureCube* Resource;
			Viewport Viewarea;

		protected:
			DepthTargetCube(const Desc& I) noexcept;

		public:
			virtual ~DepthTargetCube() noexcept;
			virtual void* GetResource() const = 0;
			virtual uint32_t GetWidth() const = 0;
			virtual uint32_t GetHeight() const = 0;
			TextureCube* GetTarget();
			const Graphics::Viewport& GetViewport() const;
		};

		class VI_OUT RenderTarget : public Core::Reference<RenderTarget>
		{
		protected:
			Texture2D* DepthStencil;
			Viewport Viewarea;

		protected:
			RenderTarget() noexcept;

		public:
			virtual ~RenderTarget() noexcept;
			virtual void* GetTargetBuffer() const = 0;
			virtual void* GetDepthBuffer() const = 0;
			virtual uint32_t GetWidth() const = 0;
			virtual uint32_t GetHeight() const = 0;
			virtual uint32_t GetTargetCount() const = 0;
			virtual Texture2D* GetTarget2D(uint32_t Index) = 0;
			virtual TextureCube* GetTargetCube(uint32_t Index) = 0;
			Texture2D* GetDepthStencil();
			const Graphics::Viewport& GetViewport() const;
		};

		class VI_OUT RenderTarget2D : public RenderTarget
		{
		public:
			struct Desc
			{
				CPUAccess AccessFlags = CPUAccess::None;
				Format FormatMode = Format::R8G8B8A8_Unorm;
				ResourceUsage Usage = ResourceUsage::Default;
				ResourceBind BindFlags = (ResourceBind)(ResourceBind::Render_Target | ResourceBind::Shader_Input);
				ResourceMisc MiscFlags = ResourceMisc::None;
				void* RenderSurface = nullptr;
				uint32_t Width = WINDOW_SIZE;
				uint32_t Height = WINDOW_SIZE;
				uint32_t MipLevels = 1;
				bool DepthStencil = true;
			};

		protected:
			Texture2D* Resource;

		protected:
			RenderTarget2D(const Desc& I) noexcept;

		public:
			virtual ~RenderTarget2D() noexcept;
			virtual void* GetTargetBuffer() const = 0;
			virtual void* GetDepthBuffer() const = 0;
			virtual uint32_t GetWidth() const = 0;
			virtual uint32_t GetHeight() const = 0;
			uint32_t GetTargetCount() const;
			Texture2D* GetTarget2D(uint32_t Index);
			TextureCube* GetTargetCube(uint32_t Index);
			Texture2D* GetTarget();
		};

		class VI_OUT MultiRenderTarget2D : public RenderTarget
		{
		public:
			struct Desc
			{
				CPUAccess AccessFlags = CPUAccess::None;
				SurfaceTarget Target = SurfaceTarget::T0;
				Format FormatMode[8] = { Format::R8G8B8A8_Unorm, Format::Unknown, Format::Unknown, Format::Unknown, Format::Unknown, Format::Unknown, Format::Unknown, Format::Unknown };
				ResourceUsage Usage = ResourceUsage::Default;
				ResourceBind BindFlags = (ResourceBind)(ResourceBind::Render_Target | ResourceBind::Shader_Input);
				ResourceMisc MiscFlags = ResourceMisc::None;
				uint32_t Width = WINDOW_SIZE;
				uint32_t Height = WINDOW_SIZE;
				uint32_t MipLevels = 1;
				bool DepthStencil = true;
			};

		protected:
			SurfaceTarget Target;
			Texture2D* Resource[8];

		protected:
			MultiRenderTarget2D(const Desc& I) noexcept;

		public:
			virtual ~MultiRenderTarget2D() noexcept;
			virtual void* GetTargetBuffer() const = 0;
			virtual void* GetDepthBuffer() const = 0;
			virtual uint32_t GetWidth() const = 0;
			virtual uint32_t GetHeight() const = 0;
			uint32_t GetTargetCount() const;
			Texture2D* GetTarget2D(uint32_t Index);
			TextureCube* GetTargetCube(uint32_t Index);
			Texture2D* GetTarget(uint32_t Index);
		};

		class VI_OUT RenderTargetCube : public RenderTarget
		{
		public:
			struct Desc
			{
				CPUAccess AccessFlags = CPUAccess::None;
				Format FormatMode = Format::R8G8B8A8_Unorm;
				ResourceUsage Usage = ResourceUsage::Default;
				ResourceBind BindFlags = (ResourceBind)(ResourceBind::Render_Target | ResourceBind::Shader_Input);
				ResourceMisc MiscFlags = (ResourceMisc)(ResourceMisc::Generate_Mips | ResourceMisc::Texture_Cube);
				uint32_t Size = 512;
				uint32_t MipLevels = 1;
				bool DepthStencil = true;
			};

		protected:
			TextureCube* Resource;

		protected:
			RenderTargetCube(const Desc& I) noexcept;

		public:
			virtual ~RenderTargetCube() noexcept;
			virtual void* GetTargetBuffer() const = 0;
			virtual void* GetDepthBuffer() const = 0;
			virtual uint32_t GetWidth() const = 0;
			virtual uint32_t GetHeight() const = 0;
			uint32_t GetTargetCount() const;
			Texture2D* GetTarget2D(uint32_t Index);
			TextureCube* GetTargetCube(uint32_t Index);
			TextureCube* GetTarget();
		};

		class VI_OUT MultiRenderTargetCube : public RenderTarget
		{
		public:
			struct Desc
			{
				CPUAccess AccessFlags = CPUAccess::None;
				SurfaceTarget Target = SurfaceTarget::T0;
				Format FormatMode[8] = { Format::R8G8B8A8_Unorm, Format::Unknown, Format::Unknown, Format::Unknown, Format::Unknown, Format::Unknown, Format::Unknown, Format::Unknown };
				ResourceUsage Usage = ResourceUsage::Default;
				ResourceBind BindFlags = (ResourceBind)(ResourceBind::Render_Target | ResourceBind::Shader_Input);
				ResourceMisc MiscFlags = (ResourceMisc)(ResourceMisc::Generate_Mips | ResourceMisc::Texture_Cube);
				uint32_t MipLevels = 1;
				uint32_t Size = 512;
				bool DepthStencil = true;
			};

		protected:
			SurfaceTarget Target;
			TextureCube* Resource[8];

		protected:
			MultiRenderTargetCube(const Desc& I) noexcept;

		public:
			virtual ~MultiRenderTargetCube() noexcept;
			virtual void* GetTargetBuffer() const = 0;
			virtual void* GetDepthBuffer() const = 0;
			virtual uint32_t GetWidth() const = 0;
			virtual uint32_t GetHeight() const = 0;
			uint32_t GetTargetCount() const;
			Texture2D* GetTarget2D(uint32_t Index);
			TextureCube* GetTargetCube(uint32_t Index);
			TextureCube* GetTarget(uint32_t Index);
		};

		class VI_OUT Cubemap : public Core::Reference<Cubemap>
		{
		public:
			struct Desc
			{
				RenderTarget* Source = nullptr;
				uint32_t Target = 0;
				uint32_t MipLevels = 0;
				uint32_t Size = 512;
			};

		protected:
			TextureCube* Dest;
			Desc Meta;

		protected:
			Cubemap(const Desc& I) noexcept;

		public:
			virtual ~Cubemap() noexcept = default;
			bool IsValid() const;
		};

		class VI_OUT Query : public Core::Reference<Query>
		{
		public:
			struct Desc
			{
				bool Predicate = true;
				bool AutoPass = false;
			};

		protected:
			Query() noexcept;

		public:
			virtual ~Query() noexcept = default;
			virtual void* GetResource() const = 0;
		};

		class VI_OUT GraphicsDevice : public Core::Reference<GraphicsDevice>
		{
		protected:
			struct DirectBuffer
			{
				Trigonometry::Matrix4x4 Transform;
				Trigonometry::Vector4 Padding;
			};

			struct Vertex
			{
				float PX, PY, PZ;
				float TX, TY;
				float CX, CY, CZ, CW;
			};

		public:
			struct Desc
			{
				RenderBackend Backend = RenderBackend::Automatic;
				ShaderModel ShaderMode = ShaderModel::Auto;
				Format BufferFormat = Format::R8G8B8A8_Unorm;
				VSync VSyncMode = VSync::Frequency_X1;
				Core::String CacheDirectory = "./shaders";
				int IsWindowed = 1;
				bool ShaderCache = true;
				bool Debug = false;
				bool BlitRendering = false;
				uint32_t PresentationFlags = 0;
				uint32_t CompilationFlags = (uint32_t)(ShaderCompile::Enable_Strictness | ShaderCompile::Optimization_Level3 | ShaderCompile::Matrix_Row_Major);
				uint32_t CreationFlags = 0;
				uint32_t BufferWidth = WINDOW_SIZE;
				uint32_t BufferHeight = WINDOW_SIZE;
				Activity* Window = nullptr;
			};

			struct Section
			{
				Core::String Name;
				Core::String Code;
			};

		protected:
			Core::UnorderedMap<Core::String, DepthStencilState*> DepthStencilStates;
			Core::UnorderedMap<Core::String, RasterizerState*> RasterizerStates;
			Core::UnorderedMap<Core::String, BlendState*> BlendStates;
			Core::UnorderedMap<Core::String, SamplerState*> SamplerStates;
			Core::UnorderedMap<Core::String, InputLayout*> InputLayouts;
			Core::UnorderedMap<Core::String, Section*> Sections;
			Core::SingleQueue<RenderThreadCallback> Queue;
			std::thread::id RenderThread;
			PrimitiveTopology Primitives;
			ShaderModel ShaderGen;
			Texture2D* ViewResource = nullptr;
			RenderTarget2D* RenderTarget = nullptr;
			Activity* VirtualWindow = nullptr;
			uint32_t PresentFlags = 0;
			uint32_t CompileFlags = 0;
			VSync VSyncMode = VSync::Frequency_X1;
			Core::Vector<Vertex> Elements;
			Core::String Caches;
			const void* Constants[4];
			size_t MaxElements;
			RenderBackend Backend;
			DirectBuffer Direct;
			std::recursive_mutex Exclusive;
			bool ShaderCache;
			bool Debug;

		protected:
			GraphicsDevice(const Desc& I) noexcept;

		public:
			virtual ~GraphicsDevice() noexcept;
			virtual void SetAsCurrentDevice() = 0;
			virtual void SetShaderModel(ShaderModel Model) = 0;
			virtual void SetBlendState(BlendState* State) = 0;
			virtual void SetRasterizerState(RasterizerState* State) = 0;
			virtual void SetDepthStencilState(DepthStencilState* State) = 0;
			virtual void SetInputLayout(InputLayout* State) = 0;
			virtual ExpectsGraphics<void> SetShader(Shader* Resource, uint32_t Type) = 0;
			virtual void SetSamplerState(SamplerState* State, uint32_t Slot, uint32_t Count, uint32_t Type) = 0;
			virtual void SetBuffer(Shader* Resource, uint32_t Slot, uint32_t Type) = 0;
			virtual void SetBuffer(InstanceBuffer* Resource, uint32_t Slot, uint32_t Type) = 0;
			virtual void SetConstantBuffer(ElementBuffer* Resource, uint32_t Slot, uint32_t Type) = 0;
			virtual void SetStructureBuffer(ElementBuffer* Resource, uint32_t Slot, uint32_t Type) = 0;
			virtual void SetTexture2D(Texture2D* Resource, uint32_t Slot, uint32_t Type) = 0;
			virtual void SetTexture3D(Texture3D* Resource, uint32_t Slot, uint32_t Type) = 0;
			virtual void SetTextureCube(TextureCube* Resource, uint32_t Slot, uint32_t Type) = 0;
			virtual void SetIndexBuffer(ElementBuffer* Resource, Format FormatMode) = 0;
			virtual void SetVertexBuffers(ElementBuffer** Resources, uint32_t Count, bool DynamicLinkage = false) = 0;
			virtual void SetWriteable(ElementBuffer** Resource, uint32_t Slot, uint32_t Count, bool Computable) = 0;
			virtual void SetWriteable(Texture2D** Resource, uint32_t Slot, uint32_t Count, bool Computable) = 0;
			virtual void SetWriteable(Texture3D** Resource, uint32_t Slot, uint32_t Count, bool Computable) = 0;
			virtual void SetWriteable(TextureCube** Resource, uint32_t Slot, uint32_t Count, bool Computable) = 0;
			virtual void SetTarget(float R, float G, float B) = 0;
			virtual void SetTarget() = 0;
			virtual void SetTarget(DepthTarget2D* Resource) = 0;
			virtual void SetTarget(DepthTargetCube* Resource) = 0;
			virtual void SetTarget(Graphics::RenderTarget* Resource, uint32_t Target, float R, float G, float B) = 0;
			virtual void SetTarget(Graphics::RenderTarget* Resource, uint32_t Target) = 0;
			virtual void SetTarget(Graphics::RenderTarget* Resource, float R, float G, float B) = 0;
			virtual void SetTarget(Graphics::RenderTarget* Resource) = 0;
			virtual void SetTargetMap(Graphics::RenderTarget* Resource, bool Enabled[8]) = 0;
			virtual void SetTargetRect(uint32_t Width, uint32_t Height) = 0;
			virtual void SetViewports(uint32_t Count, Viewport* Viewports) = 0;
			virtual void SetScissorRects(uint32_t Count, Trigonometry::Rectangle* Value) = 0;
			virtual void SetPrimitiveTopology(PrimitiveTopology Topology) = 0;
			virtual void FlushTexture(uint32_t Slot, uint32_t Count, uint32_t Type) = 0;
			virtual void FlushState() = 0;
			virtual void ClearBuffer(InstanceBuffer* Resource) = 0;
			virtual void ClearWritable(Texture2D* Resource) = 0;
			virtual void ClearWritable(Texture2D* Resource, float R, float G, float B) = 0;
			virtual void ClearWritable(Texture3D* Resource) = 0;
			virtual void ClearWritable(Texture3D* Resource, float R, float G, float B) = 0;
			virtual void ClearWritable(TextureCube* Resource) = 0;
			virtual void ClearWritable(TextureCube* Resource, float R, float G, float B) = 0;
			virtual void Clear(float R, float G, float B) = 0;
			virtual void Clear(Graphics::RenderTarget* Resource, uint32_t Target, float R, float G, float B) = 0;
			virtual void ClearDepth() = 0;
			virtual void ClearDepth(DepthTarget2D* Resource) = 0;
			virtual void ClearDepth(DepthTargetCube* Resource) = 0;
			virtual void ClearDepth(Graphics::RenderTarget* Resource) = 0;
			virtual void DrawIndexed(uint32_t Count, uint32_t IndexLocation, uint32_t BaseLocation) = 0;
			virtual void DrawIndexed(MeshBuffer* Resource) = 0;
			virtual void DrawIndexed(SkinMeshBuffer* Resource) = 0;
			virtual void DrawIndexedInstanced(uint32_t IndexCountPerInstance, uint32_t InstanceCount, uint32_t IndexLocation, uint32_t VertexLocation, uint32_t InstanceLocation) = 0;
			virtual void DrawIndexedInstanced(ElementBuffer* Instances, MeshBuffer* Resource, uint32_t InstanceCount) = 0;
			virtual void DrawIndexedInstanced(ElementBuffer* Instances, SkinMeshBuffer* Resource, uint32_t InstanceCount) = 0;
			virtual void Draw(uint32_t Count, uint32_t Location) = 0;
			virtual void DrawInstanced(uint32_t VertexCountPerInstance, uint32_t InstanceCount, uint32_t VertexLocation, uint32_t InstanceLocation) = 0;
			virtual void Dispatch(uint32_t GroupX, uint32_t GroupY, uint32_t GroupZ) = 0;
			virtual void GetViewports(uint32_t* Count, Viewport* Out) = 0;
			virtual void GetScissorRects(uint32_t* Count, Trigonometry::Rectangle* Out) = 0;
			virtual void QueryBegin(Query* Resource) = 0;
			virtual void QueryEnd(Query* Resource) = 0;
			virtual void GenerateMips(Texture2D* Resource) = 0;
			virtual void GenerateMips(Texture3D* Resource) = 0;
			virtual void GenerateMips(TextureCube* Resource) = 0;
			virtual bool ImBegin() = 0;
			virtual void ImTransform(const Trigonometry::Matrix4x4& Transform) = 0;
			virtual void ImTopology(PrimitiveTopology Topology) = 0;
			virtual void ImEmit() = 0;
			virtual void ImTexture(Texture2D* In) = 0;
			virtual void ImColor(float X, float Y, float Z, float W) = 0;
			virtual void ImIntensity(float Intensity) = 0;
			virtual void ImTexCoord(float X, float Y) = 0;
			virtual void ImTexCoordOffset(float X, float Y) = 0;
			virtual void ImPosition(float X, float Y, float Z) = 0;
			virtual bool ImEnd() = 0;
			virtual ExpectsGraphics<void> Submit() = 0;
			virtual ExpectsGraphics<void> Map(ElementBuffer* Resource, ResourceMap Mode, MappedSubresource* Map) = 0;
			virtual ExpectsGraphics<void> Map(Texture2D* Resource, ResourceMap Mode, MappedSubresource* Map) = 0;
			virtual ExpectsGraphics<void> Map(Texture3D* Resource, ResourceMap Mode, MappedSubresource* Map) = 0;
			virtual ExpectsGraphics<void> Map(TextureCube* Resource, ResourceMap Mode, MappedSubresource* Map) = 0;
			virtual ExpectsGraphics<void> Unmap(Texture2D* Resource, MappedSubresource* Map) = 0;
			virtual ExpectsGraphics<void> Unmap(Texture3D* Resource, MappedSubresource* Map) = 0;
			virtual ExpectsGraphics<void> Unmap(TextureCube* Resource, MappedSubresource* Map) = 0;
			virtual ExpectsGraphics<void> Unmap(ElementBuffer* Resource, MappedSubresource* Map) = 0;
			virtual ExpectsGraphics<void> UpdateConstantBuffer(ElementBuffer* Resource, void* Data, size_t Size) = 0;
			virtual ExpectsGraphics<void> UpdateBuffer(ElementBuffer* Resource, void* Data, size_t Size) = 0;
			virtual ExpectsGraphics<void> UpdateBuffer(Shader* Resource, const void* Data) = 0;
			virtual ExpectsGraphics<void> UpdateBuffer(MeshBuffer* Resource, Trigonometry::Vertex* Data) = 0;
			virtual ExpectsGraphics<void> UpdateBuffer(SkinMeshBuffer* Resource, Trigonometry::SkinVertex* Data) = 0;
			virtual ExpectsGraphics<void> UpdateBuffer(InstanceBuffer* Resource) = 0;
			virtual ExpectsGraphics<void> UpdateBufferSize(Shader* Resource, size_t Size) = 0;
			virtual ExpectsGraphics<void> UpdateBufferSize(InstanceBuffer* Resource, size_t Size) = 0;
			virtual ExpectsGraphics<void> CopyTexture2D(Texture2D* Resource, Core::Unique<Texture2D>* Result) = 0;
			virtual ExpectsGraphics<void> CopyTexture2D(Graphics::RenderTarget* Resource, uint32_t Target, Core::Unique<Texture2D>* Result) = 0;
			virtual ExpectsGraphics<void> CopyTexture2D(RenderTargetCube* Resource, Trigonometry::CubeFace Face, Core::Unique<Texture2D>* Result) = 0;
			virtual ExpectsGraphics<void> CopyTexture2D(MultiRenderTargetCube* Resource, uint32_t Cube, Trigonometry::CubeFace Face, Core::Unique<Texture2D>* Result) = 0;
			virtual ExpectsGraphics<void> CopyTextureCube(RenderTargetCube* Resource, Core::Unique<TextureCube>* Result) = 0;
			virtual ExpectsGraphics<void> CopyTextureCube(MultiRenderTargetCube* Resource, uint32_t Cube, Core::Unique<TextureCube>* Result) = 0;
			virtual ExpectsGraphics<void> CopyTarget(Graphics::RenderTarget* From, uint32_t FromTarget, Graphics::RenderTarget* To, uint32_t ToTarget) = 0;
			virtual ExpectsGraphics<void> CopyBackBuffer(Core::Unique<Texture2D>* Result) = 0;
			virtual ExpectsGraphics<void> CubemapPush(Cubemap* Resource, TextureCube* Result) = 0;
			virtual ExpectsGraphics<void> CubemapFace(Cubemap* Resource, Trigonometry::CubeFace Face) = 0;
			virtual ExpectsGraphics<void> CubemapPop(Cubemap* Resource) = 0;
			virtual ExpectsGraphics<void> RescaleBuffers(uint32_t Width, uint32_t Height) = 0;
			virtual ExpectsGraphics<void> ResizeBuffers(uint32_t Width, uint32_t Height) = 0;
			virtual ExpectsGraphics<void> GenerateTexture(Texture2D* Resource) = 0;
			virtual ExpectsGraphics<void> GenerateTexture(Texture3D* Resource) = 0;
			virtual ExpectsGraphics<void> GenerateTexture(TextureCube* Resource) = 0;
			virtual ExpectsGraphics<void> GetQueryData(Query* Resource, size_t* Result, bool Flush = true) = 0;
			virtual ExpectsGraphics<void> GetQueryData(Query* Resource, bool* Result, bool Flush = true) = 0;
			virtual ExpectsGraphics<Core::Unique<DepthStencilState>> CreateDepthStencilState(const DepthStencilState::Desc& I) = 0;
			virtual ExpectsGraphics<Core::Unique<BlendState>> CreateBlendState(const BlendState::Desc& I) = 0;
			virtual ExpectsGraphics<Core::Unique<RasterizerState>> CreateRasterizerState(const RasterizerState::Desc& I) = 0;
			virtual ExpectsGraphics<Core::Unique<SamplerState>> CreateSamplerState(const SamplerState::Desc& I) = 0;
			virtual ExpectsGraphics<Core::Unique<InputLayout>> CreateInputLayout(const InputLayout::Desc& I) = 0;
			virtual ExpectsGraphics<Core::Unique<Shader>> CreateShader(const Shader::Desc& I) = 0;
			virtual ExpectsGraphics<Core::Unique<ElementBuffer>> CreateElementBuffer(const ElementBuffer::Desc& I) = 0;
			virtual ExpectsGraphics<Core::Unique<MeshBuffer>> CreateMeshBuffer(const MeshBuffer::Desc& I) = 0;
			virtual ExpectsGraphics<Core::Unique<MeshBuffer>> CreateMeshBuffer(ElementBuffer* VertexBuffer, ElementBuffer* IndexBuffer) = 0;
			virtual ExpectsGraphics<Core::Unique<SkinMeshBuffer>> CreateSkinMeshBuffer(const SkinMeshBuffer::Desc& I) = 0;
			virtual ExpectsGraphics<Core::Unique<SkinMeshBuffer>> CreateSkinMeshBuffer(ElementBuffer* VertexBuffer, ElementBuffer* IndexBuffer) = 0;
			virtual ExpectsGraphics<Core::Unique<InstanceBuffer>> CreateInstanceBuffer(const InstanceBuffer::Desc& I) = 0;
			virtual ExpectsGraphics<Core::Unique<Texture2D>> CreateTexture2D() = 0;
			virtual ExpectsGraphics<Core::Unique<Texture2D>> CreateTexture2D(const Texture2D::Desc& I) = 0;
			virtual ExpectsGraphics<Core::Unique<Texture3D>> CreateTexture3D() = 0;
			virtual ExpectsGraphics<Core::Unique<Texture3D>> CreateTexture3D(const Texture3D::Desc& I) = 0;
			virtual ExpectsGraphics<Core::Unique<TextureCube>> CreateTextureCube() = 0;
			virtual ExpectsGraphics<Core::Unique<TextureCube>> CreateTextureCube(const TextureCube::Desc& I) = 0;
			virtual ExpectsGraphics<Core::Unique<TextureCube>> CreateTextureCube(Texture2D* Resource[6]) = 0;
			virtual ExpectsGraphics<Core::Unique<TextureCube>> CreateTextureCube(Texture2D* Resource) = 0;
			virtual ExpectsGraphics<Core::Unique<DepthTarget2D>> CreateDepthTarget2D(const DepthTarget2D::Desc& I) = 0;
			virtual ExpectsGraphics<Core::Unique<DepthTargetCube>> CreateDepthTargetCube(const DepthTargetCube::Desc& I) = 0;
			virtual ExpectsGraphics<Core::Unique<RenderTarget2D>> CreateRenderTarget2D(const RenderTarget2D::Desc& I) = 0;
			virtual ExpectsGraphics<Core::Unique<MultiRenderTarget2D>> CreateMultiRenderTarget2D(const MultiRenderTarget2D::Desc& I) = 0;
			virtual ExpectsGraphics<Core::Unique<RenderTargetCube>> CreateRenderTargetCube(const RenderTargetCube::Desc& I) = 0;
			virtual ExpectsGraphics<Core::Unique<MultiRenderTargetCube>> CreateMultiRenderTargetCube(const MultiRenderTargetCube::Desc& I) = 0;
			virtual ExpectsGraphics<Core::Unique<Cubemap>> CreateCubemap(const Cubemap::Desc& I) = 0;
			virtual ExpectsGraphics<Core::Unique<Query>> CreateQuery(const Query::Desc& I) = 0;
			virtual PrimitiveTopology GetPrimitiveTopology() const = 0;
			virtual ShaderModel GetSupportedShaderModel()  const = 0;
			virtual void* GetDevice() const = 0;
			virtual void* GetContext() const = 0;
			virtual bool IsValid() const = 0;
			virtual void SetVSyncMode(VSync Mode);
			void SetVertexBuffer(ElementBuffer* Resource);
			void SetShaderCache(bool Enabled);
			void Lockup(RenderThreadCallback&& Callback);
			void Enqueue(RenderThreadCallback&& Callback);
			Compute::ExpectsPreprocessor<void> Preprocess(Shader::Desc& Subresult);
			ExpectsGraphics<void> Transpile(Core::String* HLSL, ShaderType Stage, ShaderLang To);
			ExpectsGraphics<void> GetSectionInfo(const std::string_view& Name, Section** Result);
			ExpectsGraphics<void> GetSectionData(const std::string_view& Name, Shader::Desc* Result);
			bool AddSection(const std::string_view& Name, const std::string_view& Code);
			bool RemoveSection(const std::string_view& Name);
			bool IsLeftHanded() const;
			Core::String GetShaderMain(ShaderType Type) const;
			const Core::UnorderedMap<Core::String, DepthStencilState*>& GetDepthStencilStates() const;
			const Core::UnorderedMap<Core::String, RasterizerState*>& GetRasterizerStates() const;
			const Core::UnorderedMap<Core::String, BlendState*>& GetBlendStates() const;
			const Core::UnorderedMap<Core::String, SamplerState*>& GetSamplerStates() const;
			const Core::UnorderedMap<Core::String, InputLayout*>& GetInputLayouts() const;
			ExpectsVideo<Core::Unique<Surface>> CreateSurface(Texture2D* Base);
			DepthStencilState* GetDepthStencilState(const std::string_view& Name);
			BlendState* GetBlendState(const std::string_view& Name);
			RasterizerState* GetRasterizerState(const std::string_view& Name);
			SamplerState* GetSamplerState(const std::string_view& Name);
			InputLayout* GetInputLayout(const std::string_view& Name);
			ShaderModel GetShaderModel() const;
			RenderTarget2D* GetRenderTarget();
			RenderBackend GetBackend() const;
			uint32_t GetFormatSize(Format Mode) const;
			uint32_t GetPresentFlags() const;
			uint32_t GetCompileFlags() const;
			uint32_t GetRowPitch(uint32_t Width, uint32_t ElementSize = sizeof(uint8_t) * 4) const;
			uint32_t GetDepthPitch(uint32_t RowPitch, uint32_t Height) const;
			uint32_t GetMipLevel(uint32_t Width, uint32_t Height) const;
			VSync GetVSyncMode() const;
			bool IsDebug() const;

		protected:
			virtual ExpectsGraphics<TextureCube*> CreateTextureCubeInternal(void* Resource[6]) = 0;
			Core::Option<Core::String> GetProgramName(const Shader::Desc& Desc);
			bool GetProgramCache(const std::string_view& Name, Core::String* Data);
			bool SetProgramCache(const std::string_view& Name, const std::string_view& Data);
			void CreateStates();
			void CreateSections();
			void ReleaseProxy();
			void DispatchQueue();

		public:
			static GraphicsDevice* Create(Desc& I);
			static ExpectsGraphics<void> CompileBuiltinShaders(const Core::Vector<GraphicsDevice*>& Devices, const std::function<bool(GraphicsDevice*, const std::string_view&, const ExpectsGraphics<Shader*>&)>& Callback);
		};

		class VI_OUT Activity final : public Core::Reference<Activity>
		{
		public:
			struct Desc
			{
				Core::String Title = "Activity";
				uint32_t InactiveSleepMs = 66;
				uint32_t Width = 0;
				uint32_t Height = 0;
				int32_t X = 0, Y = 0;
				bool Fullscreen = false;
				bool Hidden = true;
				bool Borderless = false;
				bool Resizable = true;
				bool Minimized = false;
				bool Maximized = false;
				bool Centered = false;
				bool FreePosition = false;
				bool Focused = false;
				bool RenderEvenIfInactive = false;
				bool GPUAsRenderer = true;
				bool HighDPI = true;
			};

			struct
			{
				AppStateChangeCallback AppStateChange = nullptr;
				WindowStateChangeCallback WindowStateChange = nullptr;
				KeyStateCallback KeyState = nullptr;
				InputEditCallback InputEdit = nullptr;
				InputCallback Input = nullptr;
				CursorMoveCallback CursorMove = nullptr;
				CursorWheelStateCallback CursorWheelState = nullptr;
				JoyStickAxisMoveCallback JoyStickAxisMove = nullptr;
				JoyStickBallMoveCallback JoyStickBallMove = nullptr;
				JoyStickHatMoveCallback JoyStickHatMove = nullptr;
				JoyStickKeyStateCallback JoyStickKeyState = nullptr;
				JoyStickStateCallback JoyStickState = nullptr;
				ControllerAxisMoveCallback ControllerAxisMove = nullptr;
				ControllerKeyStateCallback ControllerKeyState = nullptr;
				ControllerStateCallback ControllerState = nullptr;
				TouchMoveCallback TouchMove = nullptr;
				TouchStateCallback TouchState = nullptr;
				GestureStateCallback GestureState = nullptr;
				MultiGestureStateCallback MultiGestureState = nullptr;
				DropFileCallback DropFile = nullptr;
				DropTextCallback DropText = nullptr;
			} Callbacks;

		private:
			struct
			{
				bool Captured = false;
				bool Mapped = false;
				bool Enabled = false;
				KeyMap Key;
			} Mapping;

		private:
			EventConsumers EventSource;
			SDL_Cursor* Cursors[(size_t)DisplayCursor::Count];
			SDL_Window* Handle;
			SDL_Surface* Favicon;
			Desc Options;
			bool Keys[2][1024];
			int Command, CX, CY;

		public:
			void* UserPointer = nullptr;
			Alert Message;

		public:
			Activity(const Desc& I) noexcept;
			virtual ~Activity() noexcept;
			void SetClipboardText(const std::string_view& Text);
			void SetCursorPosition(const Trigonometry::Vector2& Position);
			void SetCursorPosition(float X, float Y);
			void SetGlobalCursorPosition(const Trigonometry::Vector2& Position);
			void SetGlobalCursorPosition(float X, float Y);
			void SetKey(KeyCode KeyCode, bool Value);
			void SetCursor(DisplayCursor Style);
			void SetCursorVisibility(bool Enabled);
			void SetGrabbing(bool Enabled);
			void SetFullscreen(bool Enabled);
			void SetBorderless(bool Enabled);
			void SetIcon(Surface* Icon);
			void SetTitle(const std::string_view& Value);
			void SetScreenKeyboard(bool Enabled);
			void ApplyConfiguration(RenderBackend Backend);
			void Wakeup();
			void Hide();
			void Show();
			void Maximize();
			void Minimize();
			void Focus();
			void Move(int X, int Y);
			void Resize(int Width, int Height);
			void Load(SDL_SysWMinfo* Base);
			bool CaptureKeyMap(KeyMap* Value);
			bool Dispatch(uint64_t TimeoutMs = 0, bool DispatchAll = true);
			bool IsFullscreen() const;
			bool IsAnyKeyDown() const;
			bool IsKeyDown(const KeyMap& Key) const;
			bool IsKeyUp(const KeyMap& Key) const;
			bool IsKeyDownHit(const KeyMap& Key) const;
			bool IsKeyUpHit(const KeyMap& Key) const;
			bool IsScreenKeyboardEnabled() const;
			uint32_t GetX() const;
			uint32_t GetY() const;
			uint32_t GetWidth() const;
			uint32_t GetHeight() const;
			uint32_t GetId() const;
			float GetAspectRatio() const;
			KeyMod GetKeyModState() const;
			Graphics::Viewport GetViewport() const;
			Trigonometry::Vector2 GetOffset() const;
			Trigonometry::Vector2 GetSize() const;
			Trigonometry::Vector2 GetClientSize() const;
			Trigonometry::Vector2 GetDrawableSize(uint32_t Width, uint32_t Height) const;
			Trigonometry::Vector2 GetGlobalCursorPosition() const;
			Trigonometry::Vector2 GetCursorPosition() const;
			Trigonometry::Vector2 GetCursorPosition(float ScreenWidth, float ScreenHeight) const;
			Trigonometry::Vector2 GetCursorPosition(const Trigonometry::Vector2& ScreenDimensions) const;
			Core::String GetClipboardText() const;
			SDL_Window* GetHandle() const;
			Core::String GetError() const;
			Desc& GetOptions();

		public:
			static bool MultiDispatch(const EventConsumers& Sources, uint64_t TimeoutMs = 0, bool DispatchAll = true);

		private:
			bool ApplySystemTheme();
			bool* GetInputState();
		};

		class VI_OUT Alerts
		{
		public:
			static bool Text(const std::string_view& Title, const std::string_view& Message, const std::string_view& DefaultInput, Core::String* Result);
			static bool Password(const std::string_view& Title, const std::string_view& Message, Core::String* Result);
			static bool Save(const std::string_view& Title, const std::string_view& DefaultPath, const std::string_view& Filter, const std::string_view& FilterDescription, Core::String* Result);
			static bool Open(const std::string_view& Title, const std::string_view& DefaultPath, const std::string_view& Filter, const std::string_view& FilterDescription, bool Multiple, Core::String* Result);
			static bool Folder(const std::string_view& Title, const std::string_view& DefaultPath, Core::String* Result);
			static bool Color(const std::string_view& Title, const std::string_view& DefaultHexRGB, Core::String* Result);
		};

		class VI_OUT_TS Video : public Core::Singletonish
		{
		public:
			class VI_OUT Windows
			{
			public:
				static void* GetHDC(Activity* Target);
				static void* GetHINSTANCE(Activity* Target);
				static void* GetHWND(Activity* Target);
			};

			class VI_OUT WinRT
			{
			public:
				static void* GetIInspectable(Activity* Target);
			};

			class VI_OUT X11
			{
			public:
				static void* GetDisplay(Activity* Target);
				static size_t GetWindow(Activity* Target);
			};

			class VI_OUT DirectFB
			{
			public:
				static void* GetIDirectFB(Activity* Target);
				static void* GetIDirectFBWindow(Activity* Target);
				static void* GetIDirectFBSurface(Activity* Target);
			};

			class VI_OUT Cocoa
			{
			public:
				static void* GetNSWindow(Activity* Target);
			};

			class VI_OUT UIKit
			{
			public:
				static void* GetUIWindow(Activity* Target);
			};

			class VI_OUT Wayland
			{
			public:
				static void* GetWlDisplay(Activity* Target);
				static void* GetWlSurface(Activity* Target);
				static void* GetWlEglWindow(Activity* Target);
				static void* GetXdgSurface(Activity* Target);
				static void* GetXdgTopLevel(Activity* Target);
				static void* GetXdgPopup(Activity* Target);
				static void* GetXdgPositioner(Activity* Target);
			};

			class VI_OUT Android
			{
			public:
				static void* GetANativeWindow(Activity* Target);
			};

			class VI_OUT OS2
			{
			public:
				static void* GetHWND(Activity* Target);
				static void* GetHWNDFrame(Activity* Target);
			};

			class VI_OUT GLEW
			{
			public:
				static bool SetSwapInterval(int32_t Interval);
				static bool SetSwapParameters(int32_t R, int32_t G, int32_t B, int32_t A, bool Debugging);
				static bool SetContext(Activity* Target, void* Context);
				static bool PerformSwap(Activity* Target);
				static void* CreateContext(Activity* Target);
				static void DestroyContext(void* Context);
			};

		public:
			static uint32_t GetDisplayCount();
			static bool GetDisplayInfo(uint32_t DisplayIndex, DisplayInfo* Info);
			static std::string_view GetKeyCodeAsLiteral(KeyCode Code);
			static std::string_view GetKeyModAsLiteral(KeyMod Code);
			static Core::String GetKeyCodeAsString(KeyCode Code);
			static Core::String GetKeyModAsString(KeyMod Code);
		};
	}
}
#endif
