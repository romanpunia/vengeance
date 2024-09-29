#include "../gui.h"
#ifdef VI_RMLUI
#include <RmlUi/Core.h>
#include <Source/Core/TransformState.h>

namespace Vitex
{
	namespace Layer
	{
		namespace GUI
		{
			static class BoxShadowInstancer* IBoxShadow = nullptr;
			static class BoxBlurInstancer* IBoxBlur = nullptr;

			class DecoratorUtils
			{
			public:
				static void SetWorldViewProjection(RenderConstants* Constants, Rml::Element* Element, const Rml::Vector2f& Position, const Rml::Vector2f& Size, const Trigonometry::Vector2& Mul = 1.0f)
				{
					VI_ASSERT(Constants != nullptr, "graphics device should be set");
					VI_ASSERT(Element != nullptr, "element should be set");

					Trigonometry::Vector3 Scale(Size.x / 2.0f, Size.y / 2.0f);
					Trigonometry::Vector3 Offset(Position.x + Scale.X, Position.y + Scale.Y);
					Trigonometry::Matrix4x4& Ortho = *Subsystem::Get()->GetProjection();

					const Rml::TransformState* State = Element->GetTransformState();
					if (State != nullptr && State->GetTransform() != nullptr)
					{
						Trigonometry::Matrix4x4 View = Utils::ToMatrix(State->GetTransform());
						Constants->Render.Transform = Trigonometry::Matrix4x4::CreateTranslatedScale(Offset, Scale + Mul) * View * Ortho;
						Constants->Render.World = (Trigonometry::Matrix4x4::CreateTranslation(Trigonometry::Vector2(Position.x, Position.y)) * View * Ortho).Inv();
					}
					else
					{
						Constants->Render.Transform = Trigonometry::Matrix4x4::CreateTranslatedScale(Offset, Scale + Mul) * Ortho;
						Constants->Render.World = (Trigonometry::Matrix4x4::CreateTranslation(Trigonometry::Vector2(Position.x, Position.y)) * Ortho).Inv();
					}
				}
			};

			class BoxShadowInstancer final : public Rml::DecoratorInstancer
			{
			public:
				struct RenderConstant
				{
					Trigonometry::Vector4 Color;
					Trigonometry::Vector4 Radius;
					Trigonometry::Vector2 Size;
					Trigonometry::Vector2 Position;
					Trigonometry::Vector2 Offset;
					float Softness = 0.0f;
					float Padding = 0.0f;
				} RenderPass;

			public:
				Rml::PropertyId Color;
				Rml::PropertyId OffsetX;
				Rml::PropertyId OffsetY;
				Rml::PropertyId Softness;

			public:
				Graphics::ElementBuffer* VertexBuffer;
				Graphics::Shader* Shader;
				Graphics::GraphicsDevice* Device;
				RenderConstants* Constants;

			public:
				BoxShadowInstancer(RenderConstants* NewConstants);
				virtual ~BoxShadowInstancer() override;
				Rml::SharedPtr<Rml::Decorator> InstanceDecorator(const Rml::String& Name, const Rml::PropertyDictionary& Props, const Rml::DecoratorInstancerInterface& Interface) override;
			};

			class BoxBlurInstancer final : public Rml::DecoratorInstancer
			{
			public:
				struct RenderConstant
				{
					Trigonometry::Vector4 Color;
					Trigonometry::Vector4 Radius;
					Trigonometry::Vector2 Texel;
					Trigonometry::Vector2 Size;
					Trigonometry::Vector2 Position;
					float Softness = 0.0f;
					float Alpha = 1.0f;
				} RenderPass;

			public:
				Rml::PropertyId Color;
				Rml::PropertyId Softness;

			public:
				Graphics::Texture2D* Background;
				Graphics::ElementBuffer* VertexBuffer;
				Graphics::Shader* Shader;
				Graphics::GraphicsDevice* Device;
				RenderConstants* Constants;

			public:
				BoxBlurInstancer(RenderConstants* NewConstants);
				virtual ~BoxBlurInstancer() override;
				Rml::SharedPtr<Rml::Decorator> InstanceDecorator(const Rml::String& Name, const Rml::PropertyDictionary& Props, const Rml::DecoratorInstancerInterface& Interface) override;
			};

			class BoxShadow final : public Rml::Decorator
			{
			private:
				Trigonometry::Vector4 Color;
				Trigonometry::Vector2 Offset;
				float Softness;

			public:
				BoxShadow(const Trigonometry::Vector4& NewColor, const Trigonometry::Vector2& NewOffset, float NewSoftness) : Color(NewColor.Div(255.0f)), Offset(NewOffset), Softness(NewSoftness)
				{
				}
				virtual ~BoxShadow() = default;
				Rml::DecoratorDataHandle GenerateElementData(Rml::Element* Element, Rml::BoxArea PaintArea) const override
				{
					return 1;
				}
				void ReleaseElementData(Rml::DecoratorDataHandle ElementData) const override
				{
				}
				void RenderElement(Rml::Element* Element, Rml::DecoratorDataHandle ElementData) const override
				{
					VI_ASSERT(Element != nullptr, "element should be set");
					Rml::Vector2f Position = Element->GetAbsoluteOffset(Rml::BoxArea::Padding).Round();
					Rml::Vector2f Size = Element->GetBox().GetSize(Rml::BoxArea::Padding).Round();
					float Alpha = Element->GetProperty<float>("opacity");
					float Radius = Softness * 0.85f;

					Graphics::GraphicsDevice* Device = IBoxShadow->Device;
					IBoxShadow->RenderPass.Position = Trigonometry::Vector2(Position.x, Position.y);
					IBoxShadow->RenderPass.Size = Trigonometry::Vector2(Size.x, Size.y);
					IBoxShadow->RenderPass.Color = Color;
					IBoxShadow->RenderPass.Color.W *= Alpha;
					IBoxShadow->RenderPass.Offset = Offset;
					IBoxShadow->RenderPass.Softness = Softness;
					IBoxShadow->RenderPass.Radius.X = Element->GetProperty<float>("border-bottom-left-radius");
					IBoxShadow->RenderPass.Radius.Y = Element->GetProperty<float>("border-bottom-right-radius");
					IBoxShadow->RenderPass.Radius.Z = Element->GetProperty<float>("border-top-right-radius");
					IBoxShadow->RenderPass.Radius.W = Element->GetProperty<float>("border-top-left-radius");

					RenderConstants* Constants = IBoxShadow->Constants;
					DecoratorUtils::SetWorldViewProjection(Constants, Element, Position, Size, Offset.Abs() + Radius + 4096.0f);
					Constants->UpdateConstantBuffer(RenderBufferType::Render);

					Device->SetShader(IBoxShadow->Shader, VI_VS | VI_PS);
					Device->SetBuffer(IBoxShadow->Shader, 3, VI_PS);
					Device->SetVertexBuffer(IBoxShadow->VertexBuffer);
					Device->UpdateBuffer(IBoxShadow->Shader, &IBoxShadow->RenderPass);
					Device->Draw((unsigned int)IBoxShadow->VertexBuffer->GetElements(), 0);
				}
			};

			class BoxBlur final : public Rml::Decorator
			{
			private:
				Trigonometry::Vector4 Color;
				float Softness;

			public:
				BoxBlur(const Trigonometry::Vector4& NewColor, float NewSoftness) : Color(NewColor.Div(255)), Softness(NewSoftness)
				{
				}
				virtual ~BoxBlur() = default;
				Rml::DecoratorDataHandle GenerateElementData(Rml::Element* Element, Rml::BoxArea PaintArea) const override
				{
					return 1;
				}
				void ReleaseElementData(Rml::DecoratorDataHandle ElementData) const override
				{
				}
				void RenderElement(Rml::Element* Element, Rml::DecoratorDataHandle ElementData) const override
				{
					VI_ASSERT(Element != nullptr, "element should be set");
					Graphics::Texture2D* Background = Subsystem::Get()->GetBackground();
					if (!Background)
						return;

					Rml::Vector2i Screen = Element->GetContext()->GetDimensions();
					Rml::Vector2f Position = Element->GetAbsoluteOffset(Rml::BoxArea::Padding).Round();
					Rml::Vector2f Size = Element->GetBox().GetSize(Rml::BoxArea::Padding).Round();
					float Alpha = Element->GetProperty<float>("opacity");

					Graphics::GraphicsDevice* Device = IBoxBlur->Device;
					IBoxBlur->RenderPass.Color = Color;
					IBoxBlur->RenderPass.Texel = Trigonometry::Vector2((float)Screen.x, (float)Screen.y);
					IBoxBlur->RenderPass.Position = Trigonometry::Vector2(Position.x, Position.y);
					IBoxBlur->RenderPass.Size = Trigonometry::Vector2(Size.x, Size.y);
					IBoxBlur->RenderPass.Softness = Softness;
					IBoxBlur->RenderPass.Alpha = Alpha;
					IBoxBlur->RenderPass.Radius.X = Element->GetProperty<float>("border-bottom-left-radius");
					IBoxBlur->RenderPass.Radius.Y = Element->GetProperty<float>("border-bottom-right-radius");
					IBoxBlur->RenderPass.Radius.Z = Element->GetProperty<float>("border-top-right-radius");
					IBoxBlur->RenderPass.Radius.W = Element->GetProperty<float>("border-top-left-radius");

					RenderConstants* Constants = IBoxBlur->Constants;
					DecoratorUtils::SetWorldViewProjection(Constants, Element, Position, Size);
					Constants->UpdateConstantBuffer(RenderBufferType::Render);

					Device->CopyTexture2D(Background, &IBoxBlur->Background);
					Device->SetTexture2D(IBoxBlur->Background, 1, VI_PS);
					Device->SetShader(IBoxBlur->Shader, VI_VS | VI_PS);
					Device->SetBuffer(IBoxBlur->Shader, 3, VI_PS);
					Device->SetVertexBuffer(IBoxBlur->VertexBuffer);
					Device->UpdateBuffer(IBoxBlur->Shader, &IBoxBlur->RenderPass);
					Device->Draw((unsigned int)IBoxBlur->VertexBuffer->GetElements(), 0);
				}
			};

			BoxShadowInstancer::BoxShadowInstancer(RenderConstants* NewConstants) : Shader(nullptr), Device(nullptr), Constants(NewConstants)
			{
				VI_ASSERT(Constants != nullptr, "render constants should be set");
				VI_ASSERT(Constants->GetDevice() != nullptr, "graphics device should be set");
				Device = Constants->GetDevice();

				Graphics::Shader::Desc I = Graphics::Shader::Desc();
				if (Device->GetSectionData("materials/material_ui_box_shadow", &I))
				{
					Shader = *Device->CreateShader(I);
					Device->UpdateBufferSize(Shader, sizeof(RenderPass));
				}

				Rml::Vertex Elements[6];
				Elements[0] = { Rml::Vector2f(-1.0f, -1.0f), Rml::ColourbPremultiplied(0, 0, 0, 0), Rml::Vector2f(-1, 0) };
				Elements[1] = { Rml::Vector2f(-1.0f, 1.0f), Rml::ColourbPremultiplied(0, 0, 0, 0), Rml::Vector2f(-1, 1) };
				Elements[2] = { Rml::Vector2f(1.0f, 1.0f), Rml::ColourbPremultiplied(0, 0, 0, 0), Rml::Vector2f(0, 1) };
				Elements[3] = { Rml::Vector2f(-1.0f, -1.0f), Rml::ColourbPremultiplied(0, 0, 0, 0), Rml::Vector2f(-1, 0) };
				Elements[4] = { Rml::Vector2f(1.0f, 1.0f), Rml::ColourbPremultiplied(0, 0, 0, 0), Rml::Vector2f(0, 1) };
				Elements[5] = { Rml::Vector2f(1.0f, -1.0f), Rml::ColourbPremultiplied(0, 0, 0, 0), Rml::Vector2f(0, 0) };

				Graphics::ElementBuffer::Desc F = Graphics::ElementBuffer::Desc();
				F.AccessFlags = Graphics::CPUAccess::None;
				F.Usage = Graphics::ResourceUsage::Default;
				F.BindFlags = Graphics::ResourceBind::Vertex_Buffer;
				F.ElementCount = 6;
				F.ElementWidth = sizeof(Rml::Vertex);
				F.Elements = &Elements[0];

				VertexBuffer = *Device->CreateElementBuffer(F);
				Color = RegisterProperty("color", "#000").AddParser("color").GetId();
				Softness = RegisterProperty("softness", "60").AddParser("number").GetId();
				OffsetX = RegisterProperty("x", "0").AddParser("number").GetId();
				OffsetY = RegisterProperty("y", "10").AddParser("number").GetId();
				RegisterShorthand("decorator", "x, y, softness, color", Rml::ShorthandType::FallThrough);
			}
			BoxShadowInstancer::~BoxShadowInstancer()
			{
				Core::Memory::Release(Shader);
				Core::Memory::Release(VertexBuffer);
			}
			Rml::SharedPtr<Rml::Decorator> BoxShadowInstancer::InstanceDecorator(const Rml::String& Name, const Rml::PropertyDictionary& Props, const Rml::DecoratorInstancerInterface& Interface)
			{
				const Rml::Property* SColor = Props.GetProperty(Color);
				const Rml::Property* SSoftness = Props.GetProperty(Softness);
				const Rml::Property* SOffsetX = Props.GetProperty(OffsetX);
				const Rml::Property* SOffsetY = Props.GetProperty(OffsetY);

				Rml::Colourb IColor = SColor->Get<Rml::Colourb>();
				float ISoftness = SSoftness->Get<float>();
				float IOffsetX = SOffsetX->Get<float>();
				float IOffsetY = SOffsetY->Get<float>();

				return Rml::MakeShared<BoxShadow>(
					Trigonometry::Vector4(IColor.red, IColor.green, IColor.blue, IColor.alpha),
					Trigonometry::Vector2(IOffsetX, IOffsetY), ISoftness);
			}

			BoxBlurInstancer::BoxBlurInstancer(RenderConstants* NewConstants) : Background(nullptr), Shader(nullptr), Device(nullptr), Constants(NewConstants)
			{
				VI_ASSERT(Constants != nullptr, "render constants should be set");
				VI_ASSERT(Constants->GetDevice() != nullptr, "graphics device should be set");
				Device = Constants->GetDevice();

				Graphics::Shader::Desc I = Graphics::Shader::Desc();
				if (Device->GetSectionData("materials/material_ui_box_blur", &I))
				{
					Shader = *Device->CreateShader(I);
					Device->UpdateBufferSize(Shader, sizeof(RenderPass));
				}

				Rml::Vertex Elements[6];
				Elements[0] = { Rml::Vector2f(-1.0f, -1.0f), Rml::ColourbPremultiplied(0, 0, 0, 0), Rml::Vector2f(-1, 0) };
				Elements[1] = { Rml::Vector2f(-1.0f, 1.0f), Rml::ColourbPremultiplied(0, 0, 0, 0), Rml::Vector2f(-1, 1) };
				Elements[2] = { Rml::Vector2f(1.0f, 1.0f), Rml::ColourbPremultiplied(0, 0, 0, 0), Rml::Vector2f(0, 1) };
				Elements[3] = { Rml::Vector2f(-1.0f, -1.0f), Rml::ColourbPremultiplied(0, 0, 0, 0), Rml::Vector2f(-1, 0) };
				Elements[4] = { Rml::Vector2f(1.0f, 1.0f), Rml::ColourbPremultiplied(0, 0, 0, 0), Rml::Vector2f(0, 1) };
				Elements[5] = { Rml::Vector2f(1.0f, -1.0f), Rml::ColourbPremultiplied(0, 0, 0, 0), Rml::Vector2f(0, 0) };

				Graphics::ElementBuffer::Desc F = Graphics::ElementBuffer::Desc();
				F.AccessFlags = Graphics::CPUAccess::None;
				F.Usage = Graphics::ResourceUsage::Default;
				F.BindFlags = Graphics::ResourceBind::Vertex_Buffer;
				F.ElementCount = 6;
				F.ElementWidth = sizeof(Rml::Vertex);
				F.Elements = &Elements[0];

				VertexBuffer = *Device->CreateElementBuffer(F);
				Color = RegisterProperty("color", "#fff").AddParser("color").GetId();
				Softness = RegisterProperty("softness", "8").AddParser("number").GetId();
				RegisterShorthand("decorator", "softness, color", Rml::ShorthandType::FallThrough);
			}
			BoxBlurInstancer::~BoxBlurInstancer()
			{
				Core::Memory::Release(Shader);
				Core::Memory::Release(VertexBuffer);
				Core::Memory::Release(Background);
			}
			Rml::SharedPtr<Rml::Decorator> BoxBlurInstancer::InstanceDecorator(const Rml::String& Name, const Rml::PropertyDictionary& Props, const Rml::DecoratorInstancerInterface& Interface)
			{
				const Rml::Property* SColor = Props.GetProperty(Color);
				const Rml::Property* SSoftness = Props.GetProperty(Softness);

				Rml::Colourb IColor = SColor->Get<Rml::Colourb>();
				float ISoftness = SSoftness->Get<float>();

				return Rml::MakeShared<BoxBlur>(Trigonometry::Vector4(IColor.red, IColor.green, IColor.blue, IColor.alpha), ISoftness);
			}

			void Subsystem::ResizeDecorators(int Width, int Height) noexcept
			{
				if (IBoxBlur != nullptr)
					Core::Memory::Release(IBoxBlur->Background);
			}
			void Subsystem::CreateDecorators(RenderConstants* Constants) noexcept
			{
				VI_ASSERT(Constants != nullptr, "render constants should be set");
				ReleaseDecorators();

				IBoxShadow = Core::Memory::New<BoxShadowInstancer>(Constants);
				Rml::Factory::RegisterDecoratorInstancer("box-shadow", IBoxShadow);

				IBoxBlur = Core::Memory::New<BoxBlurInstancer>(Constants);
				Rml::Factory::RegisterDecoratorInstancer("box-blur", IBoxBlur);
			}
			void Subsystem::ReleaseDecorators() noexcept
			{
				Core::Memory::Delete(IBoxShadow);
				IBoxShadow = nullptr;

				Core::Memory::Delete(IBoxBlur);
				IBoxBlur = nullptr;
			}
		}
	}
}
#endif
