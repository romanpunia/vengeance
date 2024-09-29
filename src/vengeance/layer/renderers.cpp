#include "renderers.h"
#include "components.h"

namespace Vitex
{
	namespace Layer
	{
		namespace Renderers
		{
			SoftBody::SoftBody(Layer::RenderSystem* Lab) : GeometryRenderer(Lab), VertexBuffer(nullptr), IndexBuffer(nullptr)
			{
				VI_ASSERT(System != nullptr, "render system should be set");
				VI_ASSERT(System->GetDevice() != nullptr, "graphics device should be set");

				Graphics::GraphicsDevice* Device = System->GetDevice();
				DepthStencil = Device->GetDepthStencilState("drw_srw_lt");
				Rasterizer = Device->GetRasterizerState("so_co");
				Blend = Device->GetBlendState("bo_wrgba_one");
				Sampler = Device->GetSamplerState("a16_fa_wrap");
				Layout = Device->GetInputLayout("vx_base");

				Shaders.Geometry = *System->CompileShader("materials/material_model_geometry");
				Shaders.Voxelizer = *System->CompileShader("materials/material_model_voxelizer", sizeof(Lighting::IVoxelBuffer));
				Shaders.Depth.Culling = *System->CompileShader("materials/material_model_depth_culling");
				Shaders.Depth.Linear = *System->CompileShader("materials/material_model_depth_linear");
				Shaders.Depth.Cubic = *System->CompileShader("materials/material_model_depth_cubic", sizeof(Trigonometry::Matrix4x4) * 6);

				Graphics::ElementBuffer* Buffers[2];
				if (Lab->CompileBuffers(Buffers, "soft-body", sizeof(Trigonometry::Vertex), 16384))
				{
					IndexBuffer = Buffers[(size_t)BufferType::Index];
					VertexBuffer = Buffers[(size_t)BufferType::Vertex];
				}
			}
			SoftBody::~SoftBody()
			{
				Graphics::ElementBuffer* Buffers[2];
				Buffers[(size_t)BufferType::Index] = IndexBuffer;
				Buffers[(size_t)BufferType::Vertex] = VertexBuffer;

				System->FreeBuffers(Buffers);
				System->FreeShader(Shaders.Geometry);
				System->FreeShader(Shaders.Voxelizer);
				System->FreeShader(Shaders.Depth.Culling);
				System->FreeShader(Shaders.Depth.Linear);
				System->FreeShader(Shaders.Depth.Cubic);
			}
			size_t SoftBody::CullGeometry(const Viewer& View, const GeometryRenderer::Objects& Chunk)
			{
				VI_ASSERT(System->GetPrimitives() != nullptr, "primitives cache should be set");

				Graphics::ElementBuffer* Box[2];
				System->GetPrimitives()->GetBoxBuffers(Box);

				Graphics::GraphicsDevice* Device = System->GetDevice();
				Device->SetRasterizerState(Rasterizer);
				Device->SetInputLayout(Layout);
				Device->SetShader(nullptr, VI_PS);
				Device->SetShader(Shaders.Depth.Culling, VI_VS);

				size_t Count = 0;
				if (System->PreciseCulling)
				{
					for (auto* Base : Chunk)
					{
						if (Base->GetIndices().empty() || !CullingBegin(Base))
							continue;

						Base->Fill(Device, IndexBuffer, VertexBuffer);
						System->Constants->Render.World.Identify();
						System->Constants->Render.Transform = View.ViewProjection;
						System->UpdateConstantBuffer(RenderBufferType::Render);
						Device->SetVertexBuffer(VertexBuffer);
						Device->SetIndexBuffer(IndexBuffer, Graphics::Format::R32_Uint);
						Device->DrawIndexed((uint32_t)Base->GetIndices().size(), 0, 0);
						CullingEnd();
						Count++;
					}
				}
				else
				{
					Device->SetVertexBuffer(Box[(size_t)BufferType::Vertex]);
					Device->SetIndexBuffer(Box[(size_t)BufferType::Index], Graphics::Format::R32_Uint);

					for (auto* Base : Chunk)
					{
						if (Base->GetIndices().empty() || !CullingBegin(Base))
							continue;

						System->Constants->Render.World = Base->GetEntity()->GetBox();
						System->Constants->Render.Transform = System->Constants->Render.World * View.ViewProjection;
						System->UpdateConstantBuffer(RenderBufferType::Render);
						Device->DrawIndexed((uint32_t)Box[(size_t)BufferType::Index]->GetElements(), 0, 0);
						CullingEnd();

						Count++;
					}
				}

				return Count;
			}
			size_t SoftBody::RenderGeometric(Core::Timer* Time, const GeometryRenderer::Objects& Chunk)
			{
				VI_ASSERT(System->GetScene() != nullptr, "scene should be set");
				Graphics::GraphicsDevice* Device = System->GetDevice();
				bool Static = System->State.IsSet(RenderOpt::Static);

				Device->SetDepthStencilState(DepthStencil);
				Device->SetBlendState(Blend);
				Device->SetRasterizerState(Rasterizer);
				Device->SetInputLayout(Layout);
				Device->SetSamplerState(Sampler, 1, 7, VI_PS);
				Device->SetShader(Shaders.Geometry, VI_VS | VI_PS);

				size_t Count = 0;
				for (auto* Base : Chunk)
				{
					if ((Static && !Base->Static) || Base->GetIndices().empty())
						continue;

					if (!System->TryGeometry(Base->GetMaterial(), true))
						continue;

					Base->Fill(Device, IndexBuffer, VertexBuffer);
					System->Constants->Render.World.Identify();
					System->Constants->Render.Transform = System->View.ViewProjection;
					System->Constants->Render.TexCoord = Base->TexCoord;
					Device->SetVertexBuffer(VertexBuffer);
					Device->SetIndexBuffer(IndexBuffer, Graphics::Format::R32_Uint);
					System->UpdateConstantBuffer(RenderBufferType::Render);
					Device->DrawIndexed((uint32_t)Base->GetIndices().size(), 0, 0);
					Count++;
				}

				return Count;
			}
			size_t SoftBody::RenderVoxelization(Core::Timer* Time, const GeometryRenderer::Objects& Chunk)
			{
				VI_ASSERT(System->GetScene() != nullptr, "scene should be set");
				Graphics::GraphicsDevice* Device = System->GetDevice();
				Device->SetInputLayout(Layout);
				Device->SetSamplerState(Sampler, 4, 6, VI_PS);
				Device->SetShader(Shaders.Voxelizer, VI_VS | VI_PS | VI_GS);
				Lighting::SetVoxelBuffer(System, Shaders.Voxelizer, 3);

				size_t Count = 0;
				for (auto* Base : Chunk)
				{
					if (!Base->Static || Base->GetIndices().empty())
						continue;

					if (!System->TryGeometry(Base->GetMaterial(), true))
						continue;

					Base->Fill(Device, IndexBuffer, VertexBuffer);
					System->Constants->Render.World.Identify();
					System->Constants->Render.Transform.Identify();
					System->Constants->Render.TexCoord = Base->TexCoord;
					Device->SetVertexBuffer(VertexBuffer);
					Device->SetIndexBuffer(IndexBuffer, Graphics::Format::R32_Uint);
					System->UpdateConstantBuffer(RenderBufferType::Render);
					Device->DrawIndexed((uint32_t)Base->GetIndices().size(), 0, 0);

					Count++;
				}

				Device->SetShader(nullptr, VI_GS);
				return Count;
			}
			size_t SoftBody::RenderLinearization(Core::Timer* Time, const GeometryRenderer::Objects& Chunk)
			{
				VI_ASSERT(System->GetScene() != nullptr, "scene should be set");
				Graphics::GraphicsDevice* Device = System->GetDevice();
				Device->SetDepthStencilState(DepthStencil);
				Device->SetBlendState(Blend);
				Device->SetRasterizerState(Rasterizer);
				Device->SetInputLayout(Layout);
				Device->SetSamplerState(Sampler, 1, 1, VI_PS);
				Device->SetShader(Shaders.Depth.Linear, VI_VS | VI_PS);

				size_t Count = 0;
				for (auto* Base : Chunk)
				{
					if (Base->GetIndices().empty())
						continue;

					if (!System->TryGeometry(Base->GetMaterial(), true))
						continue;

					Base->Fill(Device, IndexBuffer, VertexBuffer);
					System->Constants->Render.World.Identify();
					System->Constants->Render.Transform = System->View.ViewProjection;
					System->Constants->Render.TexCoord = Base->TexCoord;
					System->UpdateConstantBuffer(RenderBufferType::Render);
					Device->SetVertexBuffer(VertexBuffer);
					Device->SetIndexBuffer(IndexBuffer, Graphics::Format::R32_Uint);
					Device->DrawIndexed((uint32_t)Base->GetIndices().size(), 0, 0);

					Count++;
				}

				Device->SetTexture2D(nullptr, 1, VI_PS);
				return Count;
			}
			size_t SoftBody::RenderCubic(Core::Timer* Time, const GeometryRenderer::Objects& Chunk, Trigonometry::Matrix4x4* ViewProjection)
			{
				VI_ASSERT(System->GetScene() != nullptr, "scene should be set");
				Graphics::GraphicsDevice* Device = System->GetDevice();
				Device->SetDepthStencilState(DepthStencil);
				Device->SetBlendState(Blend);
				Device->SetRasterizerState(Rasterizer);
				Device->SetInputLayout(Layout);
				Device->SetSamplerState(Sampler, 1, 1, VI_PS);
				Device->SetShader(Shaders.Depth.Linear, VI_VS | VI_PS | VI_GS);
				Device->SetBuffer(Shaders.Depth.Cubic, 3, VI_VS | VI_PS | VI_GS);
				Device->UpdateBuffer(Shaders.Depth.Cubic, ViewProjection);

				size_t Count = 0;
				for (auto* Base : Chunk)
				{
					if (!Base->GetBody())
						continue;

					if (!System->TryGeometry(Base->GetMaterial(), true))
						continue;

					Base->Fill(Device, IndexBuffer, VertexBuffer);
					System->Constants->Render.World.Identify();
					System->Constants->Render.TexCoord = Base->TexCoord;
					System->UpdateConstantBuffer(RenderBufferType::Render);
					Device->SetVertexBuffer(VertexBuffer);
					Device->SetIndexBuffer(IndexBuffer, Graphics::Format::R32_Uint);
					Device->DrawIndexed((uint32_t)Base->GetIndices().size(), 0, 0);

					Count++;
				}

				Device->SetShader(nullptr, VI_GS);
				Device->SetTexture2D(nullptr, 1, VI_PS);
				return Count;
			}

			Model::Model(Layer::RenderSystem* Lab) : GeometryRenderer(Lab)
			{
				VI_ASSERT(System != nullptr, "render system should be set");
				VI_ASSERT(System->GetDevice() != nullptr, "graphics device should be set");

				Graphics::GraphicsDevice* Device = System->GetDevice();
				DepthStencil = Device->GetDepthStencilState("drw_srw_lt");
				BackRasterizer = Device->GetRasterizerState("so_cback");
				FrontRasterizer = Device->GetRasterizerState("so_cfront");
				Blend = Device->GetBlendState("bo_wrgba_one");
				Sampler = Device->GetSamplerState("a16_fa_wrap");
				Layout[0] = Device->GetInputLayout("vx_base");
				Layout[1] = Device->GetInputLayout("vxi_base");

				Shaders.Geometry = *System->CompileShader("materials/material_model_geometry");
				Shaders.Voxelizer = *System->CompileShader("materials/material_model_voxelizer", sizeof(Lighting::IVoxelBuffer));
				Shaders.Depth.Culling = *System->CompileShader("materials/material_model_depth_culling");
				Shaders.Depth.Linear = *System->CompileShader("materials/material_model_depth_linear");
				Shaders.Depth.Cubic = *System->CompileShader("materials/material_model_depth_cubic", sizeof(Trigonometry::Matrix4x4) * 6);
			}
			Model::~Model()
			{
				System->FreeShader(Shaders.Geometry);
				System->FreeShader(Shaders.Voxelizer);
				System->FreeShader(Shaders.Depth.Culling);
				System->FreeShader(Shaders.Depth.Linear);
				System->FreeShader(Shaders.Depth.Cubic);
			}
			void Model::BatchGeometry(Components::Model* Base, GeometryRenderer::Batching& Batch, size_t Chunk)
			{
				auto* Drawable = GetDrawable(Base);
				if (!Base->Static && !System->State.IsSet(RenderOpt::Static))
					return;

				RenderBuffer::Instance Data;
				Data.TexCoord = Base->TexCoord;

				auto& World = Base->GetEntity()->GetBox();
				for (auto* Mesh : Drawable->Meshes)
				{
					Material* Source = Base->GetMaterial(Mesh);
					if (System->TryInstance(Source, Data))
					{
						Data.World = Mesh->Transform * World;
						Data.Transform = Data.World * System->View.ViewProjection;
						Batch.Emplace(Mesh, Source, Data, Chunk);
					}
				}
			}
			size_t Model::CullGeometry(const Viewer& View, const GeometryRenderer::Objects& Chunk)
			{
				VI_ASSERT(System->GetPrimitives() != nullptr, "primitive cache should be set");

				Graphics::ElementBuffer* Box[2];
				System->GetPrimitives()->GetBoxBuffers(Box);

				Graphics::GraphicsDevice* Device = System->GetDevice();
				Device->SetRasterizerState(BackRasterizer);
				Device->SetInputLayout(Layout[0]);
				Device->SetShader(nullptr, VI_PS);
				Device->SetShader(Shaders.Depth.Culling, VI_VS);

				size_t Count = 0;
				if (System->PreciseCulling)
				{
					for (auto* Base : Chunk)
					{
						if (!CullingBegin(Base))
							continue;

						auto* Drawable = GetDrawable(Base);
						auto& World = Base->GetEntity()->GetBox();
						for (auto* Mesh : Drawable->Meshes)
						{
							System->Constants->Render.World = Mesh->Transform * World;
							System->Constants->Render.Transform = System->Constants->Render.World * View.ViewProjection;
							System->UpdateConstantBuffer(RenderBufferType::Render);
							Device->DrawIndexed(Mesh);
						}

						CullingEnd();
						Count++;
					}
				}
				else
				{
					Device->SetVertexBuffer(Box[(size_t)BufferType::Vertex]);
					Device->SetIndexBuffer(Box[(size_t)BufferType::Index], Graphics::Format::R32_Uint);
					for (auto* Base : Chunk)
					{
						if (!CullingBegin(Base))
							continue;

						System->Constants->Render.World = Base->GetEntity()->GetBox();
						System->Constants->Render.Transform = System->Constants->Render.World * View.ViewProjection;
						System->UpdateConstantBuffer(RenderBufferType::Render);
						Device->DrawIndexed((uint32_t)Box[(size_t)BufferType::Index]->GetElements(), 0, 0);
						CullingEnd();
						Count++;
					}
				}

				return Count;
			}
			size_t Model::RenderGeometricBatched(Core::Timer* Time, const GeometryRenderer::Groups& Chunk)
			{
				VI_ASSERT(System->GetScene() != nullptr, "scene should be set");
				Graphics::GraphicsDevice* Device = System->GetDevice();
				Device->SetDepthStencilState(DepthStencil);
				Device->SetBlendState(Blend);
				Device->SetRasterizerState(BackRasterizer);
				Device->SetInputLayout(Layout[1]);
				Device->SetSamplerState(Sampler, 1, 7, VI_PS);
				Device->SetShader(Shaders.Geometry, VI_VS | VI_PS);

				for (auto& Group : Chunk)
				{
					System->TryGeometry(Group->MaterialData, true);
					Device->DrawIndexedInstanced(Group->DataBuffer, Group->GeometryBuffer, (uint32_t)Group->Instances.size());
				}

				static Graphics::ElementBuffer* VertexBuffers[2] = { nullptr, nullptr };
				Device->SetVertexBuffers(VertexBuffers, 2);
				return Chunk.size();
			}
			size_t Model::RenderVoxelizationBatched(Core::Timer* Time, const GeometryRenderer::Groups& Chunk)
			{
				VI_ASSERT(System->GetScene() != nullptr, "scene should be set");
				Graphics::GraphicsDevice* Device = System->GetDevice();
				Device->SetInputLayout(Layout[1]);
				Device->SetSamplerState(Sampler, 4, 6, VI_PS);
				Device->SetShader(Shaders.Voxelizer, VI_VS | VI_PS | VI_GS);
				Lighting::SetVoxelBuffer(System, Shaders.Voxelizer, 3);

				for (auto& Group : Chunk)
				{
					System->TryGeometry(Group->MaterialData, true);
					Device->DrawIndexedInstanced(Group->DataBuffer, Group->GeometryBuffer, (uint32_t)Group->Instances.size());
				}

				static Graphics::ElementBuffer* VertexBuffers[2] = { nullptr, nullptr };
				Device->SetVertexBuffers(VertexBuffers, 2);
				Device->SetShader(nullptr, VI_GS);
				return Chunk.size();
			}
			size_t Model::RenderLinearizationBatched(Core::Timer* Time, const GeometryRenderer::Groups& Chunk)
			{
				VI_ASSERT(System->GetScene() != nullptr, "scene should be set");
				Graphics::GraphicsDevice* Device = System->GetDevice();
				Device->SetDepthStencilState(DepthStencil);
				Device->SetBlendState(Blend);
				Device->SetRasterizerState(System->State.IsSet(RenderOpt::Backfaces) ? FrontRasterizer : BackRasterizer);
				Device->SetInputLayout(Layout[1]);
				Device->SetSamplerState(Sampler, 1, 1, VI_PS);
				Device->SetShader(Shaders.Depth.Linear, VI_VS | VI_PS);

				for (auto& Group : Chunk)
				{
					System->TryGeometry(Group->MaterialData, true);
					Device->DrawIndexedInstanced(Group->DataBuffer, Group->GeometryBuffer, (uint32_t)Group->Instances.size());
				}

				static Graphics::ElementBuffer* VertexBuffers[2] = { nullptr, nullptr };
				Device->SetVertexBuffers(VertexBuffers, 2);
				Device->SetTexture2D(nullptr, 1, VI_PS);
				return Chunk.size();
			}
			size_t Model::RenderCubicBatched(Core::Timer* Time, const GeometryRenderer::Groups& Chunk, Trigonometry::Matrix4x4* ViewProjection)
			{
				VI_ASSERT(System->GetScene() != nullptr, "scene should be set");
				Graphics::GraphicsDevice* Device = System->GetDevice();
				Device->SetDepthStencilState(DepthStencil);
				Device->SetBlendState(Blend);
				Device->SetRasterizerState(System->State.IsSet(RenderOpt::Backfaces) ? FrontRasterizer : BackRasterizer);
				Device->SetInputLayout(Layout[1]);
				Device->SetSamplerState(Sampler, 1, 1, VI_PS);
				Device->SetShader(Shaders.Depth.Cubic, VI_VS | VI_PS | VI_GS);
				Device->SetBuffer(Shaders.Depth.Cubic, 3, VI_VS | VI_PS | VI_GS);
				Device->UpdateBuffer(Shaders.Depth.Cubic, ViewProjection);

				for (auto& Group : Chunk)
				{
					System->TryGeometry(Group->MaterialData, true);
					Device->DrawIndexedInstanced(Group->DataBuffer, Group->GeometryBuffer, (uint32_t)Group->Instances.size());
				}

				static Graphics::ElementBuffer* VertexBuffers[2] = { nullptr, nullptr };
				Device->SetVertexBuffers(VertexBuffers, 2);
				Device->SetTexture2D(nullptr, 1, VI_PS);
				Device->SetShader(nullptr, VI_GS);
				return Chunk.size();
			}
			Layer::Model* Model::GetDrawable(Components::Model* Base)
			{
				auto* Drawable = Base->GetDrawable();
				if (!Drawable)
					Drawable = System->GetPrimitives()->GetBoxModel();

				return Drawable;
			}

			Skin::Skin(Layer::RenderSystem* Lab) : GeometryRenderer(Lab)
			{
				VI_ASSERT(System != nullptr, "render system should be set");
				VI_ASSERT(System->GetDevice() != nullptr, "graphics device should be set");

				Graphics::GraphicsDevice* Device = System->GetDevice();
				DepthStencil = Device->GetDepthStencilState("drw_srw_lt");
				BackRasterizer = Device->GetRasterizerState("so_cback");
				FrontRasterizer = Device->GetRasterizerState("so_cfront");
				Blend = Device->GetBlendState("bo_wrgba_one");
				Sampler = Device->GetSamplerState("a16_fa_wrap");
				Layout = Device->GetInputLayout("vx_skin");

				Shaders.Geometry = *System->CompileShader("materials/material_skin_geometry");
				Shaders.Voxelizer = *System->CompileShader("materials/material_skin_voxelizer", sizeof(Lighting::IVoxelBuffer));
				Shaders.Depth.Culling = *System->CompileShader("materials/material_skin_depth_culling");
				Shaders.Depth.Linear = *System->CompileShader("materials/material_skin_depth_linear");
				Shaders.Depth.Cubic = *System->CompileShader("materials/material_skin_depth_cubic", sizeof(Trigonometry::Matrix4x4) * 6);
			}
			Skin::~Skin()
			{
				System->FreeShader(Shaders.Geometry);
				System->FreeShader(Shaders.Voxelizer);
				System->FreeShader(Shaders.Depth.Culling);
				System->FreeShader(Shaders.Depth.Linear);
				System->FreeShader(Shaders.Depth.Cubic);
			}
			size_t Skin::CullGeometry(const Viewer& View, const GeometryRenderer::Objects& Chunk)
			{
				VI_ASSERT(System->GetPrimitives() != nullptr, "primitive cache should be set");

				Graphics::ElementBuffer* Box[2];
				System->GetPrimitives()->GetSkinBoxBuffers(Box);

				Graphics::GraphicsDevice* Device = System->GetDevice();
				Device->SetRasterizerState(BackRasterizer);
				Device->SetInputLayout(Layout);
				Device->SetShader(nullptr, VI_PS);
				Device->SetShader(Shaders.Depth.Culling, VI_VS);

				size_t Count = 0;
				if (System->PreciseCulling)
				{
					for (auto* Base : Chunk)
					{
						if (!CullingBegin(Base))
							continue;

						auto* Drawable = GetDrawable(Base);
						auto& World = Base->GetEntity()->GetBox();
						System->Constants->Animation.Animated = (float)!Drawable->Skeleton.Childs.empty();

						for (auto* Mesh : Drawable->Meshes)
						{
							auto& Matrices = Base->Skeleton.Matrices[Mesh];
							memcpy(System->Constants->Animation.Offsets, Matrices.Data, sizeof(Matrices.Data));
							System->Constants->Render.World = Mesh->Transform * World;
							System->Constants->Render.Transform = System->Constants->Render.World * View.ViewProjection;
							System->UpdateConstantBuffer(RenderBufferType::Animation);
							System->UpdateConstantBuffer(RenderBufferType::Render);
							Device->DrawIndexed(Mesh);
						}

						CullingEnd();
						Count++;
					}
				}
				else
				{
					Device->SetVertexBuffer(Box[(size_t)BufferType::Vertex]);
					Device->SetIndexBuffer(Box[(size_t)BufferType::Index], Graphics::Format::R32_Uint);
					for (auto* Base : Chunk)
					{
						if (!CullingBegin(Base))
							continue;

						System->Constants->Animation.Animated = (float)false;
						System->Constants->Render.World = Base->GetEntity()->GetBox();
						System->Constants->Render.Transform = System->Constants->Render.World * View.ViewProjection;
						System->UpdateConstantBuffer(RenderBufferType::Animation);
						System->UpdateConstantBuffer(RenderBufferType::Render);
						Device->DrawIndexed((uint32_t)Box[(size_t)BufferType::Index]->GetElements(), 0, 0);
						CullingEnd();
						Count++;
					}
				}

				return Count;
			}
			size_t Skin::RenderGeometric(Core::Timer* Time, const GeometryRenderer::Objects& Chunk)
			{
				VI_ASSERT(System->GetScene() != nullptr, "scene should be set");
				Graphics::GraphicsDevice* Device = System->GetDevice();
				bool Static = System->State.IsSet(RenderOpt::Static);

				Device->SetDepthStencilState(DepthStencil);
				Device->SetBlendState(Blend);
				Device->SetRasterizerState(BackRasterizer);
				Device->SetInputLayout(Layout);
				Device->SetSamplerState(Sampler, 1, 7, VI_PS);
				Device->SetShader(Shaders.Geometry, VI_VS | VI_PS);

				size_t Count = 0;
				for (auto* Base : Chunk)
				{
					if (Static && !Base->Static)
						continue;

					auto* Drawable = GetDrawable(Base);
					auto& World = Base->GetEntity()->GetBox();
					System->Constants->Animation.Animated = (float)!Drawable->Skeleton.Childs.empty();
					System->Constants->Render.TexCoord = Base->TexCoord;

					for (auto* Mesh : Drawable->Meshes)
					{
						if (!System->TryGeometry(Base->GetMaterial(Mesh), true))
							continue;

						auto& Matrices = Base->Skeleton.Matrices[Mesh];
						memcpy(System->Constants->Animation.Offsets, Matrices.Data, sizeof(Matrices.Data));
						System->Constants->Render.World = Mesh->Transform * World;
						System->Constants->Render.Transform = System->Constants->Render.World * System->View.ViewProjection;
						System->UpdateConstantBuffer(RenderBufferType::Animation);
						System->UpdateConstantBuffer(RenderBufferType::Render);
						Device->DrawIndexed(Mesh);
					}
					Count++;
				}

				return Count;
			}
			size_t Skin::RenderVoxelization(Core::Timer* Time, const GeometryRenderer::Objects& Chunk)
			{
				VI_ASSERT(System->GetScene() != nullptr, "scene should be set");
				Graphics::GraphicsDevice* Device = System->GetDevice();
				Device->SetInputLayout(Layout);
				Device->SetSamplerState(Sampler, 4, 6, VI_PS);
				Device->SetShader(Shaders.Voxelizer, VI_VS | VI_PS | VI_GS);
				Lighting::SetVoxelBuffer(System, Shaders.Voxelizer, 3);

				size_t Count = 0;
				for (auto* Base : Chunk)
				{
					if (!Base->Static)
						continue;

					auto* Drawable = GetDrawable(Base);
					auto& World = Base->GetEntity()->GetBox();
					System->Constants->Animation.Animated = (float)!Drawable->Skeleton.Childs.empty();

					for (auto* Mesh : Drawable->Meshes)
					{
						if (!System->TryGeometry(Base->GetMaterial(Mesh), true))
							continue;

						auto& Matrices = Base->Skeleton.Matrices[Mesh];
						memcpy(System->Constants->Animation.Offsets, Matrices.Data, sizeof(Matrices.Data));
						System->Constants->Render.Transform = System->Constants->Render.World = Mesh->Transform * World;
						System->UpdateConstantBuffer(RenderBufferType::Animation);
						System->UpdateConstantBuffer(RenderBufferType::Render);
						Device->DrawIndexed(Mesh);
					}
					Count++;
				}

				Device->SetShader(nullptr, VI_GS);
				return Count;
			}
			size_t Skin::RenderLinearization(Core::Timer* Time, const GeometryRenderer::Objects& Chunk)
			{
				VI_ASSERT(System->GetScene() != nullptr, "scene should be set");
				Graphics::GraphicsDevice* Device = System->GetDevice();
				Device->SetDepthStencilState(DepthStencil);
				Device->SetBlendState(Blend);
				Device->SetRasterizerState(System->State.IsSet(RenderOpt::Backfaces) ? FrontRasterizer : BackRasterizer);
				Device->SetInputLayout(Layout);
				Device->SetSamplerState(Sampler, 1, 1, VI_PS);
				Device->SetShader(Shaders.Depth.Linear, VI_VS | VI_PS);

				size_t Count = 0;
				for (auto* Base : Chunk)
				{
					auto* Drawable = GetDrawable(Base);
					auto& World = Base->GetEntity()->GetBox();
					System->Constants->Animation.Animated = (float)!Drawable->Skeleton.Childs.empty();
					System->Constants->Render.TexCoord = Base->TexCoord;

					for (auto* Mesh : Drawable->Meshes)
					{
						if (!System->TryGeometry(Base->GetMaterial(Mesh), true))
							continue;

						auto& Matrices = Base->Skeleton.Matrices[Mesh];
						memcpy(System->Constants->Animation.Offsets, Matrices.Data, sizeof(Matrices.Data));
						System->Constants->Render.World = Mesh->Transform * World;
						System->Constants->Render.Transform = System->Constants->Render.World * System->View.ViewProjection;
						System->UpdateConstantBuffer(RenderBufferType::Animation);
						System->UpdateConstantBuffer(RenderBufferType::Render);
						Device->DrawIndexed(Mesh);
					}

					Count++;
				}

				Device->SetTexture2D(nullptr, 1, VI_PS);
				return Count;
			}
			size_t Skin::RenderCubic(Core::Timer* Time, const GeometryRenderer::Objects& Chunk, Trigonometry::Matrix4x4* ViewProjection)
			{
				VI_ASSERT(System->GetScene() != nullptr, "scene should be set");
				Graphics::GraphicsDevice* Device = System->GetDevice();
				Device->SetDepthStencilState(DepthStencil);
				Device->SetBlendState(Blend);
				Device->SetRasterizerState(System->State.IsSet(RenderOpt::Backfaces) ? FrontRasterizer : BackRasterizer);
				Device->SetInputLayout(Layout);
				Device->SetSamplerState(Sampler, 1, 1, VI_PS);
				Device->SetShader(Shaders.Depth.Linear, VI_VS | VI_PS | VI_GS);
				Device->SetBuffer(Shaders.Depth.Cubic, 3, VI_VS | VI_PS | VI_GS);
				Device->UpdateBuffer(Shaders.Depth.Cubic, ViewProjection);

				size_t Count = 0;
				for (auto* Base : Chunk)
				{
					auto* Drawable = GetDrawable(Base);
					auto& World = Base->GetEntity()->GetBox();
					System->Constants->Animation.Animated = (float)!Drawable->Skeleton.Childs.empty();
					System->Constants->Render.TexCoord = Base->TexCoord;

					for (auto* Mesh : Drawable->Meshes)
					{
						if (!System->TryGeometry(Base->GetMaterial(Mesh), true))
							continue;

						auto& Matrices = Base->Skeleton.Matrices[Mesh];
						memcpy(System->Constants->Animation.Offsets, Matrices.Data, sizeof(Matrices.Data));
						System->Constants->Render.World = Mesh->Transform * World;
						System->UpdateConstantBuffer(RenderBufferType::Animation);
						System->UpdateConstantBuffer(RenderBufferType::Render);
						Device->DrawIndexed(Mesh);
					}

					Count++;
				}

				Device->SetTexture2D(nullptr, 1, VI_PS);
				Device->SetShader(nullptr, VI_GS);
				return Count;
			}
			Layer::SkinModel* Skin::GetDrawable(Components::Skin* Base)
			{
				auto* Drawable = Base->GetDrawable();
				if (!Drawable)
					Drawable = System->GetPrimitives()->GetSkinBoxModel();

				return Drawable;
			}

			Emitter::Emitter(RenderSystem* Lab) : GeometryRenderer(Lab)
			{
				VI_ASSERT(System != nullptr, "render system should be set");
				VI_ASSERT(System->GetDevice() != nullptr, "graphics device should be set");

				Graphics::GraphicsDevice* Device = System->GetDevice();
				DepthStencilOpaque = Device->GetDepthStencilState("drw_srw_lt");
				DepthStencilAdditive = Device->GetDepthStencilState("dro_srw_lt");
				Rasterizer = Device->GetRasterizerState("so_cback");
				AdditiveBlend = Device->GetBlendState("bw_wrgba_alpha");
				OverwriteBlend = Device->GetBlendState("bo_wrgba_one");
				Sampler = Device->GetSamplerState("a16_fa_wrap");

				Shaders.Geometry.Opaque = *System->CompileShader("materials/material_emitter_geometry_opaque");
				Shaders.Geometry.Transparent = *System->CompileShader("materials/material_emitter_geometry_transparent");
				Shaders.Depth.Linear = *System->CompileShader("materials/material_emitter_depth_linear");
				Shaders.Depth.Point = *System->CompileShader("materials/material_emitter_depth_point");
				Shaders.Depth.Quad = *System->CompileShader("materials/material_emitter_depth_quad", sizeof(Depth));
			}
			Emitter::~Emitter()
			{
				System->FreeShader(Shaders.Geometry.Opaque);
				System->FreeShader(Shaders.Geometry.Transparent);
				System->FreeShader(Shaders.Depth.Linear);
				System->FreeShader(Shaders.Depth.Point);
				System->FreeShader(Shaders.Depth.Quad);
			}
			size_t Emitter::RenderGeometric(Core::Timer* Time, const GeometryRenderer::Objects& Chunk)
			{
				VI_ASSERT(System->GetScene() != nullptr, "scene should be set");
				Graphics::GraphicsDevice* Device = System->GetDevice();
				Graphics::Shader* BaseShader = nullptr;
				Graphics::PrimitiveTopology T = Device->GetPrimitiveTopology();
				Viewer& View = System->View;
				bool Static = System->State.IsSet(RenderOpt::Static);

				if (System->State.IsSet(RenderOpt::Additive))
				{
					BaseShader = Shaders.Geometry.Transparent;
					Device->SetDepthStencilState(DepthStencilAdditive);
					Device->SetBlendState(AdditiveBlend);
				}
				else
				{
					BaseShader = Shaders.Geometry.Opaque;
					Device->SetDepthStencilState(DepthStencilOpaque);
					Device->SetBlendState(OverwriteBlend);
				}

				Device->SetPrimitiveTopology(Graphics::PrimitiveTopology::Point_List);
				Device->SetRasterizerState(Rasterizer);
				Device->SetInputLayout(Layout);
				Device->SetSamplerState(Sampler, 1, 7, VI_PS);
				Device->SetShader(BaseShader, VI_VS | VI_PS);
				Device->SetVertexBuffer(nullptr);

				size_t Count = 0;
				for (auto* Base : Chunk)
				{
					if ((Static && !Base->Static) || !Base->GetBuffer())
						continue;

					if (!System->TryGeometry(Base->GetMaterial(), true))
						continue;

					System->Constants->Render.World = View.Projection;
					System->Constants->Render.Transform = (Base->QuadBased ? View.View : View.ViewProjection);
					System->Constants->Render.TexCoord = Base->GetEntity()->GetTransform()->Forward();
					if (Base->Connected)
						System->Constants->Render.Transform = Base->GetEntity()->GetBox() * System->Constants->Render.Transform;

					Device->SetBuffer(Base->GetBuffer(), 8, VI_VS | VI_PS);
					Device->SetShader(Base->QuadBased ? BaseShader : nullptr, VI_GS);
					Device->UpdateBuffer(Base->GetBuffer());
					System->UpdateConstantBuffer(RenderBufferType::Render);
					Device->Draw((uint32_t)Base->GetBuffer()->GetArray().size(), 0);

					Count++;
				}

				Device->SetBuffer((Graphics::InstanceBuffer*)nullptr, 8, VI_VS | VI_PS);
				Device->SetShader(nullptr, VI_GS);
				Device->SetPrimitiveTopology(T);
				return Count;
			}
			size_t Emitter::RenderLinearization(Core::Timer* Time, const GeometryRenderer::Objects& Chunk)
			{
				VI_ASSERT(System->GetScene() != nullptr, "scene should be set");
				Graphics::GraphicsDevice* Device = System->GetDevice();
				Graphics::PrimitiveTopology T = Device->GetPrimitiveTopology();
				Viewer& View = System->View;

				Device->SetPrimitiveTopology(Graphics::PrimitiveTopology::Point_List);
				Device->SetDepthStencilState(DepthStencilOpaque);
				Device->SetBlendState(OverwriteBlend);
				Device->SetRasterizerState(Rasterizer);
				Device->SetInputLayout(Layout);
				Device->SetSamplerState(Sampler, 1, 1, VI_PS);
				Device->SetShader(Shaders.Depth.Linear, VI_VS | VI_PS);
				Device->SetVertexBuffer(nullptr);

				size_t Count = 0;
				for (auto* Base : Chunk)
				{
					if (!Base->GetBuffer() || !System->TryGeometry(Base->GetMaterial(), true))
						continue;

					System->Constants->Render.World = View.Projection;
					System->Constants->Render.Transform = (Base->QuadBased ? View.View : View.ViewProjection);
					if (Base->Connected)
						System->Constants->Render.Transform = Base->GetEntity()->GetBox() * System->Constants->Render.Transform;

					Device->SetBuffer(Base->GetBuffer(), 8, VI_VS | VI_PS);
					Device->SetShader(Base->QuadBased ? Shaders.Depth.Linear : nullptr, VI_GS);
					System->UpdateConstantBuffer(RenderBufferType::Render);
					Device->Draw((uint32_t)Base->GetBuffer()->GetArray().size(), 0);

					Count++;
				}

				Device->SetTexture2D(nullptr, 1, VI_PS);
				Device->SetBuffer((Graphics::InstanceBuffer*)nullptr, 8, VI_VS | VI_PS);
				Device->SetShader(nullptr, VI_GS);
				Device->SetPrimitiveTopology(T);
				return Count;
			}
			size_t Emitter::RenderCubic(Core::Timer* Time, const GeometryRenderer::Objects& Chunk, Trigonometry::Matrix4x4* ViewProjection)
			{
				VI_ASSERT(System->GetScene() != nullptr, "scene should be set");
				auto& Source = System->View;
				Depth.FaceView[0] = Trigonometry::Matrix4x4::CreateLookAt(Trigonometry::CubeFace::PositiveX, Source.Position);
				Depth.FaceView[1] = Trigonometry::Matrix4x4::CreateLookAt(Trigonometry::CubeFace::NegativeX, Source.Position);
				Depth.FaceView[2] = Trigonometry::Matrix4x4::CreateLookAt(Trigonometry::CubeFace::PositiveY, Source.Position);
				Depth.FaceView[3] = Trigonometry::Matrix4x4::CreateLookAt(Trigonometry::CubeFace::NegativeY, Source.Position);
				Depth.FaceView[4] = Trigonometry::Matrix4x4::CreateLookAt(Trigonometry::CubeFace::PositiveZ, Source.Position);
				Depth.FaceView[5] = Trigonometry::Matrix4x4::CreateLookAt(Trigonometry::CubeFace::NegativeZ, Source.Position);

				Graphics::GraphicsDevice* Device = System->GetDevice();
				Graphics::PrimitiveTopology T = Device->GetPrimitiveTopology();
				Device->SetPrimitiveTopology(Graphics::PrimitiveTopology::Point_List);
				Device->SetDepthStencilState(DepthStencilOpaque);
				Device->SetBlendState(OverwriteBlend);
				Device->SetRasterizerState(Rasterizer);
				Device->SetInputLayout(Layout);
				Device->SetVertexBuffer(nullptr);
				Device->SetSamplerState(Sampler, 1, 1, VI_PS);
				Device->SetBuffer(Shaders.Depth.Quad, 3, VI_VS | VI_PS | VI_GS);
				Device->UpdateBuffer(Shaders.Depth.Quad, &Depth);

				size_t Count = 0;
				for (auto* Base : Chunk)
				{
					if (!Base->GetBuffer() || !System->TryGeometry(Base->GetMaterial(), true))
						continue;

					System->Constants->Render.World = (Base->Connected ? Base->GetEntity()->GetBox() : Trigonometry::Matrix4x4::Identity());
					Device->SetBuffer(Base->GetBuffer(), 8, VI_VS | VI_PS);
					Device->SetShader(Base->QuadBased ? Shaders.Depth.Quad : Shaders.Depth.Point, VI_VS | VI_PS | VI_GS);
					System->UpdateConstantBuffer(RenderBufferType::Render);
					Device->Draw((uint32_t)Base->GetBuffer()->GetArray().size(), 0);

					Count++;
				}

				Device->SetTexture2D(nullptr, 1, VI_PS);
				Device->SetBuffer((Graphics::InstanceBuffer*)nullptr, 8, VI_VS | VI_PS);
				Device->SetShader(nullptr, VI_GS);
				Device->SetPrimitiveTopology(T);
				return Count;
			}

			Decal::Decal(RenderSystem* Lab) : GeometryRenderer(Lab)
			{
				VI_ASSERT(System != nullptr, "render system should be set");
				VI_ASSERT(System->GetDevice() != nullptr, "graphics device should be set");

				Graphics::GraphicsDevice* Device = System->GetDevice();
				DepthStencil = Device->GetDepthStencilState("doo_soo_lt");
				Rasterizer = Device->GetRasterizerState("so_cback");
				Blend = Device->GetBlendState("bw_wrgba_one");
				Sampler = Device->GetSamplerState("a16_fa_wrap");
				Layout = Device->GetInputLayout("vx_shape");

				Shader = *System->CompileShader("materials/material_decal_geometry");
			}
			Decal::~Decal()
			{
				System->FreeShader(Shader);
			}
			size_t Decal::RenderGeometric(Core::Timer* Time, const GeometryRenderer::Objects& Chunk)
			{
				VI_ASSERT(System->GetScene() != nullptr, "scene should be set");

				Graphics::MultiRenderTarget2D* MRT = System->GetMRT(TargetType::Main);
				Graphics::GraphicsDevice* Device = System->GetDevice();
				bool Static = System->State.IsSet(RenderOpt::Static);

				Graphics::ElementBuffer* Box[2];
				System->GetPrimitives()->GetBoxBuffers(Box);

				Device->SetDepthStencilState(DepthStencil);
				Device->SetBlendState(Blend);
				Device->SetRasterizerState(Rasterizer);
				Device->SetInputLayout(Layout);
				Device->SetTarget(MRT, 0);
				Device->SetSamplerState(Sampler, 1, 8, VI_PS);
				Device->SetShader(Shader, VI_VS | VI_PS);
				Device->SetTexture2D(MRT->GetTarget(2), 8, VI_PS);
				Device->SetVertexBuffer(Box[(size_t)BufferType::Vertex]);
				Device->SetIndexBuffer(Box[(size_t)BufferType::Index], Graphics::Format::R32_Uint);

				size_t Count = 0;
				for (auto* Base : Chunk)
				{
					if ((Static && !Base->Static) || !System->TryGeometry(Base->GetMaterial(), true))
						continue;

					System->Constants->Render.Transform = Base->GetEntity()->GetBox() * System->View.ViewProjection;
					System->Constants->Render.World = System->Constants->Render.Transform.Inv();
					System->Constants->Render.TexCoord = Base->TexCoord;
					System->UpdateConstantBuffer(RenderBufferType::Render);
					Device->DrawIndexed((uint32_t)Box[(size_t)BufferType::Index]->GetElements(), 0, 0);
					Count++;
				}

				Device->SetTexture2D(nullptr, 8, VI_PS);
				System->RestoreOutput();
				return Count;
			}

			Lighting::Lighting(RenderSystem* Lab) : Renderer(Lab), EnableGI(true)
			{
				VI_ASSERT(System != nullptr, "render system should be set");
				VI_ASSERT(System->GetDevice() != nullptr, "graphics device should be set");

				Graphics::GraphicsDevice* Device = System->GetDevice();
				DepthStencilNone = Device->GetDepthStencilState("doo_soo_lt");
				DepthStencilGreater = Device->GetDepthStencilState("dro_sro_gte");
				DepthStencilLess = Device->GetDepthStencilState("dro_sro_lt");
				FrontRasterizer = Device->GetRasterizerState("so_cfront");
				BackRasterizer = Device->GetRasterizerState("so_cback");
				NoneRasterizer = Device->GetRasterizerState("so_co");
				BlendAdditive = Device->GetBlendState("bw_wrgbo_one");
				BlendOverwrite = Device->GetBlendState("bo_woooo_one");
				BlendOverload = Device->GetBlendState("bo_wrgba_one");
				DepthSampler = Device->GetSamplerState("a1_fl_clamp");
				DepthLessSampler = Device->GetSamplerState("a1_fl_clamp_cmp_lt");
				DepthGreaterSampler = Device->GetSamplerState("a1_fl_clamp_cmp_gte");
				WrapSampler = Device->GetSamplerState("a16_fa_wrap");
				Layout = Device->GetInputLayout("vx_shape");

				Shaders.Ambient[0] = *System->CompileShader("shading/lighting_ambient_direct", sizeof(IAmbientLight));
				Shaders.Ambient[1] = *System->CompileShader("shading/lighting_ambient_indirect", sizeof(IVoxelBuffer));
				Shaders.Point[0] = *System->CompileShader("shading/lighting_point_performant", sizeof(IPointLight));
				Shaders.Point[1] = *System->CompileShader("shading/lighting_point_qualitative");
				Shaders.Spot[0] = *System->CompileShader("shading/lighting_spot_performant", sizeof(ISpotLight));
				Shaders.Spot[1] = *System->CompileShader("shading/lighting_spot_qualitative");
				Shaders.Line[0] = *System->CompileShader("shading/lighting_line_performant", sizeof(ILineLight));
				Shaders.Line[1] = *System->CompileShader("shading/lighting_line_qualitative");
				Shaders.Voxelizer = *System->CompileShader("shading/lighting_voxelizer", sizeof(IVoxelBuffer));
				Shaders.Surface = *System->CompileShader("shading/lighting_surface", sizeof(ISurfaceLight));

				Shadows.Tick.Delay = 5;
			}
			Lighting::~Lighting()
			{
				System->FreeShader(Shaders.Voxelizer);
				System->FreeShader(Shaders.Surface);

				for (uint32_t i = 0; i < 2; i++)
				{
					System->FreeShader(Shaders.Line[i]);
					System->FreeShader(Shaders.Spot[i]);
					System->FreeShader(Shaders.Point[i]);
					System->FreeShader(Shaders.Ambient[i]);
				}

				Core::Memory::Release(Voxels.PBuffer);
				Core::Memory::Release(Voxels.SBuffer);
				Core::Memory::Release(Voxels.LBuffer);
				Core::Memory::Release(Surfaces.Subresource);
				Core::Memory::Release(Surfaces.Input);
				Core::Memory::Release(Surfaces.Output);
				Core::Memory::Release(Surfaces.Merger);
				Core::Memory::Release(SkyBase);
				Core::Memory::Release(SkyMap);
				Core::Memory::Release(LightingMap);
			}
			void Lighting::Deserialize(Core::Schema* Node)
			{
				VI_ASSERT(Node != nullptr, "schema should be set");

				Core::String Path;
				if (Series::Unpack(Node->Find("sky-map"), &Path) && !Path.empty())
					System->GetScene()->LoadResource<Graphics::Texture2D>(System->GetComponent(), Path, [this](ExpectsContent<Graphics::Texture2D*>&& Texture) { this->SetSkyMap(Texture.Or(nullptr)); });

				HeavySeries::Unpack(Node->Find("high-emission"), &AmbientLight.HighEmission);
				HeavySeries::Unpack(Node->Find("low-emission"), &AmbientLight.LowEmission);
				Series::Unpack(Node->Find("sky-emission"), &AmbientLight.SkyEmission);
				Series::Unpack(Node->Find("light-emission"), &AmbientLight.LightEmission);
				HeavySeries::Unpack(Node->Find("sky-color"), &AmbientLight.SkyColor);
				HeavySeries::Unpack(Node->Find("fog-color"), &AmbientLight.FogColor);
				Series::Unpack(Node->Find("fog-amount"), &AmbientLight.FogAmount);
				Series::Unpack(Node->Find("fog-far-off"), &AmbientLight.FogFarOff);
				HeavySeries::Unpack(Node->Find("fog-far"), &AmbientLight.FogFar);
				Series::Unpack(Node->Find("fog-near-off"), &AmbientLight.FogNearOff);
				HeavySeries::Unpack(Node->Find("fog-near"), &AmbientLight.FogNear);
				Series::Unpack(Node->Find("recursive"), &AmbientLight.Recursive);
				Series::Unpack(Node->Find("shadow-distance"), &Shadows.Distance);
				Series::UnpackA(Node->Find("sf-size"), &Surfaces.Size);
				Series::Unpack(Node->Find("gi"), &EnableGI);
			}
			void Lighting::Serialize(Core::Schema* Node)
			{
				VI_ASSERT(Node != nullptr, "schema should be set");

				Series::Pack(Node->Set("sky-map"), System->GetScene()->FindResourceId<Graphics::Texture2D>(SkyBase));
				HeavySeries::Pack(Node->Set("high-emission"), AmbientLight.HighEmission);
				HeavySeries::Pack(Node->Set("low-emission"), AmbientLight.LowEmission);
				Series::Pack(Node->Set("sky-emission"), AmbientLight.SkyEmission);
				Series::Pack(Node->Set("light-emission"), AmbientLight.LightEmission);
				HeavySeries::Pack(Node->Set("sky-color"), AmbientLight.SkyColor);
				HeavySeries::Pack(Node->Set("fog-color"), AmbientLight.FogColor);
				Series::Pack(Node->Set("fog-amount"), AmbientLight.FogAmount);
				Series::Pack(Node->Set("fog-far-off"), AmbientLight.FogFarOff);
				HeavySeries::Pack(Node->Set("fog-far"), AmbientLight.FogFar);
				Series::Pack(Node->Set("fog-near-off"), AmbientLight.FogNearOff);
				HeavySeries::Pack(Node->Set("fog-near"), AmbientLight.FogNear);
				Series::Pack(Node->Set("recursive"), AmbientLight.Recursive);
				Series::Pack(Node->Set("shadow-distance"), Shadows.Distance);
				Series::Pack(Node->Set("sf-size"), (uint64_t)Surfaces.Size);
				Series::Pack(Node->Set("gi"), EnableGI);
			}
			void Lighting::ResizeBuffers()
			{
				Core::Memory::Release(LightingMap);
			}
			void Lighting::BeginPass(Core::Timer* Time)
			{
				if (System->State.Is(RenderState::Linearization) || System->State.Is(RenderState::Cubic))
					return;

				auto& Lines = System->GetScene()->GetComponents<Components::LineLight>();
				Lights.Illuminators.Push(Time, System);
				Lights.Surfaces.Push(Time, System);
				Lights.Points.Push(Time, System);
				Lights.Spots.Push(Time, System);
				Lights.Lines = &Lines;
			}
			void Lighting::EndPass()
			{
				if (System->State.Is(RenderState::Linearization) || System->State.Is(RenderState::Cubic))
					return;

				Lights.Spots.Pop();
				Lights.Points.Pop();
				Lights.Surfaces.Pop();
				Lights.Illuminators.Pop();
			}
			void Lighting::RenderResultBuffers()
			{
				Graphics::MultiRenderTarget2D* MRT = System->GetMRT(TargetType::Main);
				Graphics::RenderTarget2D* RT = (System->State.IsSubpass() ? System->GetRT(TargetType::Secondary) : System->GetRT(TargetType::Main));
				State.Backcull = true;

				Graphics::ElementBuffer* Cube[2];
				System->GetPrimitives()->GetCubeBuffers(Cube);

				AmbientLight.SkyOffset = System->View.Projection.Inv() * Trigonometry::Matrix4x4::CreateRotation(System->View.Rotation);
				State.Device->SetDepthStencilState(DepthStencilLess);
				State.Device->SetBlendState(BlendAdditive);
				State.Device->SetRasterizerState(BackRasterizer);
				State.Device->SetInputLayout(Layout);
				State.Device->CopyTarget(MRT, 0, RT, 0);
				State.Device->SetTarget(MRT, 0, 0, 0, 0);
				State.Device->SetSamplerState(WrapSampler, 1, 4, VI_PS);
				State.Device->SetSamplerState(DepthSampler, 5, 1, VI_PS);
				State.Device->SetSamplerState(DepthLessSampler, 6, 1, VI_PS);
				State.Device->SetSamplerState(DepthGreaterSampler, 7, 1, VI_PS);
				State.Device->SetTexture2D(RT->GetTarget(), 1, VI_PS);
				State.Device->SetTexture2D(MRT->GetTarget(1), 2, VI_PS);
				State.Device->SetTexture2D(MRT->GetTarget(2), 3, VI_PS);
				State.Device->SetTexture2D(MRT->GetTarget(3), 4, VI_PS);
				State.Device->SetVertexBuffer(Cube[(size_t)BufferType::Vertex]);
				State.Device->SetIndexBuffer(Cube[(size_t)BufferType::Index], Graphics::Format::R32_Uint);

				RenderSurfaceLights();
				RenderPointLights();
				RenderSpotLights();
				RenderLineLights();
				RenderIllumination();
				RenderAmbient();

				State.Device->FlushTexture(1, 11, VI_PS);
			}
			void Lighting::RenderVoxelMap(Core::Timer* Time)
			{
				if (!EnableGI || Lights.Illuminators.Top().empty())
					return;

				Graphics::Texture3D* In[3], *Out[3];
				if (!State.Scene->GetVoxelBuffer(In, Out))
					return;

				auto& Buffers = State.Scene->GetVoxelsMapping();
				auto& Top = Lights.Illuminators.Top();
				size_t Counter = 0;

				for (auto It = Top.begin(); It != Top.end(); ++It)
				{
					if (Counter >= Buffers.size())
						break;

					auto& Buffer = Buffers[Counter++];
					auto* Last = (Components::Illuminator*)Buffer.second;
					auto* Light = *It;

					if (!Light->Regenerate)
						Light->Regenerate = (Last != Light);

					if (Last != nullptr && Light->Regenerate)
						Last->Regenerate = true;

					Light->VoxelMap = Buffer.first;
					Buffer.second = Light;

					if (!GetIlluminator(&VoxelBuffer, Light))
						continue;

					bool Inside = Trigonometry::Geometric::HasPointIntersectedCube(VoxelBuffer.Center, VoxelBuffer.Scale, System->View.Position);
					auto& Delay = (Inside ? Light->Inside : Light->Outside);
					if (!Light->Regenerate && !Delay.TickEvent(Time->GetElapsedMills()))
						continue;

					Voxels.LightBuffer = Light->VoxelMap;
					Light->Regenerate = false;

                    uint32_t Size = (uint32_t)State.Scene->GetConf().VoxelsSize;
					State.Device->ClearWritable(In[(size_t)VoxelType::Diffuse]);
					State.Device->ClearWritable(In[(size_t)VoxelType::Normal]);
					State.Device->ClearWritable(In[(size_t)VoxelType::Surface]);
					State.Device->SetTargetRect(Size, Size);
					State.Device->SetDepthStencilState(DepthStencilNone);
					State.Device->SetBlendState(BlendOverwrite);
					State.Device->SetRasterizerState(NoneRasterizer);
					State.Device->SetWriteable(In, 1, 3, false);

					Trigonometry::Matrix4x4 Offset = Trigonometry::Matrix4x4::CreateTranslatedScale(VoxelBuffer.Center, VoxelBuffer.Scale);
					System->SetView(Offset, System->View.Projection, VoxelBuffer.Center, 90.0f, 1.0f, 0.1f, GetDominant(VoxelBuffer.Scale) * 2.0f, RenderCulling::Cubic);
					State.Scene->Statistics.DrawCalls += System->Render(Time, RenderState::Voxelization, RenderOpt::None);
					System->RestoreViewBuffer(nullptr);

					State.Device->SetWriteable(Out, 1, 3, false);
					State.Device->GenerateMips(Voxels.LightBuffer);
				}
			}
			void Lighting::RenderSurfaceMaps(Core::Timer* Time)
			{
				auto& Data = Lights.Surfaces.Top();
				if (Data.empty())
					return;
				
				if (!Surfaces.Merger || !Surfaces.Subresource || !Surfaces.Input || !Surfaces.Output)
					SetSurfaceBufferSize(Surfaces.Size);

				State.Scene->SwapMRT(TargetType::Main, Surfaces.Merger);
				State.Scene->SetMRT(TargetType::Main, false);

				float ElapsedTime = Time->GetElapsedMills();
				for (auto* Light : Data)
				{
					if (Light->IsImageBased())
						continue;

					Graphics::TextureCube* Cache = Light->GetProbeCache();
					if (!Cache)
					{
						Cache = *State.Device->CreateTextureCube();
						Light->SetProbeCache(Cache);
					}
					else if (!Light->Tick.TickEvent(ElapsedTime) || Light->Tick.Delay <= 0.0)
						continue;

					State.Device->CubemapPush(Surfaces.Subresource, Cache);
					Light->Locked = true;

					Trigonometry::Vector3 Position = Light->GetEntity()->GetTransform()->GetPosition() * Light->Offset;
					for (uint32_t j = 0; j < 6; j++)
					{
						Trigonometry::CubeFace Face = (Trigonometry::CubeFace)j;
						State.Scene->ClearMRT(TargetType::Main, true, true);
						System->SetView(Light->View[j] = Trigonometry::Matrix4x4::CreateLookAt(Face, Position), Light->Projection, Position, 90.0f, 1.0f, 0.1f, Light->GetSize().Radius, RenderCulling::Linear);
						State.Scene->Statistics.DrawCalls += System->Render(Time, RenderState::Geometric, Light->StaticMask ? RenderOpt::Static : RenderOpt::None);
						State.Device->CubemapFace(Surfaces.Subresource, Face);
					}

					Light->Locked = false;
					State.Device->CubemapPop(Surfaces.Subresource);
				}

				State.Scene->SwapMRT(TargetType::Main, nullptr);
				System->RestoreViewBuffer(nullptr);
			}
			void Lighting::RenderPointShadowMaps(Core::Timer* Time)
			{
				auto& Buffers = State.Scene->GetPointsMapping(); size_t Counter = 0;
				for (auto* Light : Lights.Points.Top())
				{
					if (Counter >= Buffers.size())
						break;

					Light->DepthMap = nullptr;
					if (!Light->Shadow.Enabled)
						continue;

					CubicDepthMap* Target = Buffers[Counter++];
					Light->GenerateOrigin();
					Light->DepthMap = Target;

					State.Device->SetTarget(Target);
					State.Device->ClearDepth(Target);
					System->SetView(Trigonometry::Matrix4x4::Identity(), Light->Projection, Light->GetEntity()->GetTransform()->GetPosition(), 90.0f, 1.0f, 0.1f, Light->Shadow.Distance, RenderCulling::Cubic);
					State.Scene->Statistics.DrawCalls += System->Render(Time, RenderState::Cubic, RenderOpt::None);
				}
			}
			void Lighting::RenderSpotShadowMaps(Core::Timer* Time)
			{
				auto& Buffers = State.Scene->GetSpotsMapping(); size_t Counter = 0;
				for (auto* Light : Lights.Spots.Top())
				{
					if (Counter >= Buffers.size())
						break;

					Light->DepthMap = nullptr;
					if (!Light->Shadow.Enabled)
						continue;

					LinearDepthMap* Target = Buffers[Counter++];
					Light->GenerateOrigin();
					Light->DepthMap = Target;

					State.Device->SetTarget(Target);
					State.Device->ClearDepth(Target);
					System->SetView(Light->View, Light->Projection, Light->GetEntity()->GetTransform()->GetPosition(), Light->Cutoff, 1.0f, 0.1f, Light->Shadow.Distance, RenderCulling::Linear);
					State.Scene->Statistics.DrawCalls += System->Render(Time, RenderState::Linearization, RenderOpt::Backfaces);
				}
			}
			void Lighting::RenderLineShadowMaps(Core::Timer* Time)
			{
				auto& Buffers = State.Scene->GetLinesMapping(); size_t Counter = 0;			
				for (auto It = Lights.Lines->Begin(); It != Lights.Lines->End(); ++It)
				{
					auto* Light = (Components::LineLight*)*It;
					if (Counter >= Buffers.size())
						break;

					Light->DepthMap = nullptr;
					if (!Light->Shadow.Enabled || Light->Shadow.Cascades < 1 || Light->Shadow.Cascades > 6)
						continue;

					CascadedDepthMap*& Target = Buffers[Counter++];
					if (!Target || Target->size() < Light->Shadow.Cascades)
						State.Scene->GenerateDepthCascades(&Target, Light->Shadow.Cascades);

					Light->GenerateOrigin();
					Light->DepthMap = Target;

					for (size_t i = 0; i < Target->size(); i++)
					{
						LinearDepthMap* Cascade = (*Target)[i];
						State.Device->SetTarget(Cascade);
						State.Device->ClearDepth(Cascade);

						System->SetView(Light->View[i], Light->Projection[i], 0.0f, 90.0f, 1.0f, -System->View.FarPlane, System->View.FarPlane, RenderCulling::Disable);
						State.Scene->Statistics.DrawCalls += System->Render(Time, RenderState::Linearization, RenderOpt::None);
					}
				}

				System->RestoreViewBuffer(nullptr);
			}
			void Lighting::RenderSurfaceLights()
			{
				Graphics::ElementBuffer* Cube[2];
				System->GetPrimitives()->GetCubeBuffers(Cube);

				if (!System->State.IsSubpass())
				{
					State.Device->SetShader(Shaders.Surface, VI_VS | VI_PS);
					State.Device->SetBuffer(Shaders.Surface, 3, VI_VS | VI_PS);

					Trigonometry::Vector3 Position, Scale;
					for (auto* Light : Lights.Surfaces.Top())
					{
						if (!Light->GetProbeCache())
							continue;

						Entity* Base = Light->GetEntity();
						GetLightCulling(Light, Base->GetRadius(), &Position, &Scale);
						GetSurfaceLight(&SurfaceLight, Light, Position, Scale);

						SurfaceLight.Lighting *= Base->GetVisibility(System->View);
						State.Device->SetTextureCube(Light->GetProbeCache(), 5, VI_PS);
						State.Device->UpdateBuffer(Shaders.Surface, &SurfaceLight);
						State.Device->DrawIndexed((uint32_t)Cube[(size_t)BufferType::Index]->GetElements(), 0, 0);
					}
				}
				else if (AmbientLight.Recursive > 0.0f)
				{
					State.Device->SetShader(Shaders.Surface, VI_VS | VI_PS);
					State.Device->SetBuffer(Shaders.Surface, 3, VI_VS | VI_PS);

					Trigonometry::Vector3 Position, Scale;
					for (auto* Light : Lights.Surfaces.Top())
					{
						if (Light->Locked || !Light->GetProbeCache())
							continue;

						Entity* Base = Light->GetEntity();
						GetLightCulling(Light, Base->GetRadius(), &Position, &Scale);
						GetSurfaceLight(&SurfaceLight, Light, Position, Scale);

						SurfaceLight.Lighting *= Base->GetVisibility(System->View);
						State.Device->SetTextureCube(Light->GetProbeCache(), 5, VI_PS);
						State.Device->UpdateBuffer(Shaders.Surface, &SurfaceLight);
						State.Device->DrawIndexed((uint32_t)Cube[(size_t)BufferType::Index]->GetElements(), 0, 0);
					}
				}
			}
			void Lighting::RenderPointLights()
			{
				Graphics::ElementBuffer* Cube[2];
				System->GetPrimitives()->GetCubeBuffers(Cube);

				Graphics::Shader* BaseShader = nullptr;
				State.Device->SetBuffer(Shaders.Point[0], 3, VI_VS | VI_PS);

				Trigonometry::Vector3 Position, Scale;
				for (auto* Light : Lights.Points.Top())
				{
					Entity* Base = Light->GetEntity();
					GetLightCulling(Light, Base->GetRadius(), &Position, &Scale);

					if (GetPointLight(&PointLight, Light, Position, Scale, false))
					{
						Graphics::TextureCube* DepthMap = Light->DepthMap->GetTarget();
						State.Device->SetTextureCube(DepthMap, 5, VI_PS);
						BaseShader = Shaders.Point[1];
					}
					else
						BaseShader = Shaders.Point[0];

					PointLight.Lighting *= Base->GetVisibility(System->View);
					State.Device->SetShader(BaseShader, VI_VS | VI_PS);
					State.Device->UpdateBuffer(Shaders.Point[0], &PointLight);
					State.Device->DrawIndexed((uint32_t)Cube[(size_t)BufferType::Index]->GetElements(), 0, 0);
				}
			}
			void Lighting::RenderSpotLights()
			{
				Graphics::ElementBuffer* Cube[2];
				System->GetPrimitives()->GetCubeBuffers(Cube);

				Graphics::Shader* BaseShader = nullptr;
				State.Device->SetBuffer(Shaders.Spot[0], 3, VI_VS | VI_PS);

				Trigonometry::Vector3 Position, Scale;
				for (auto* Light : Lights.Spots.Top())
				{
					Entity* Base = Light->GetEntity();
					GetLightCulling(Light, Base->GetRadius(), &Position, &Scale);

					if (GetSpotLight(&SpotLight, Light, Position, Scale, false))
					{
						Graphics::Texture2D* DepthMap = Light->DepthMap->GetTarget();
						State.Device->SetTexture2D(DepthMap, 5, VI_PS);
						BaseShader = Shaders.Spot[1];
					}
					else
						BaseShader = Shaders.Spot[0];

					SpotLight.Lighting *= Base->GetVisibility(System->View);
					State.Device->SetShader(BaseShader, VI_VS | VI_PS);
					State.Device->UpdateBuffer(Shaders.Spot[0], &SpotLight);
					State.Device->DrawIndexed((uint32_t)Cube[(size_t)BufferType::Index]->GetElements(), 0, 0);
				}
			}
			void Lighting::RenderLineLights()
			{
				Graphics::Shader* BaseShader = nullptr;
				if (!State.Backcull)
				{
					State.Device->SetRasterizerState(BackRasterizer);
					State.Backcull = true;
				}

				State.Device->SetDepthStencilState(DepthStencilNone);
				State.Device->SetBuffer(Shaders.Line[0], 3, VI_VS | VI_PS);
				State.Device->SetVertexBuffer(System->GetPrimitives()->GetQuad());

				for (auto It = Lights.Lines->Begin(); It != Lights.Lines->End(); ++It)
				{
					auto* Light = (Components::LineLight*)*It;
					if (GetLineLight(&LineLight, Light))
					{
						uint32_t Size = (uint32_t)LineLight.Cascades;
						for (uint32_t i = 0; i < Size; i++)
							State.Device->SetTexture2D((*Light->DepthMap)[i]->GetTarget(), 5 + i, VI_PS);
						for (uint32_t i = Size; i < 6; i++)
							State.Device->SetTexture2D((*Light->DepthMap)[Size - 1]->GetTarget(), 5 + i, VI_PS);
						BaseShader = Shaders.Line[1];
					}
					else
						BaseShader = Shaders.Line[0];

					State.Device->SetShader(BaseShader, VI_VS | VI_PS);
					State.Device->UpdateBuffer(Shaders.Line[0], &LineLight);
					State.Device->Draw(6, 0);
				}

				State.Device->SetBlendState(BlendOverload);
			}
			void Lighting::RenderLuminance()
			{
				Graphics::Texture3D* In[3], *Out[3];
				if (!State.Scene->GetVoxelBuffer(In, Out))
					return;

				uint32_t X = (uint32_t)(VoxelBuffer.Size.X / 8.0f);
				uint32_t Y = (uint32_t)(VoxelBuffer.Size.Y / 8.0f);
				uint32_t Z = (uint32_t)(VoxelBuffer.Size.Z / 8.0f);

				State.Device->ClearWritable(Voxels.LightBuffer);
				State.Device->SetSamplerState(nullptr, 1, 6, VI_CS);
				State.Device->SetWriteable(Out, 1, 3, false);
				State.Device->SetWriteable(&Voxels.LightBuffer, 1, 1, true);
				State.Device->SetTexture3D(In[(size_t)VoxelType::Diffuse], 2, VI_CS);
				State.Device->SetTexture3D(In[(size_t)VoxelType::Normal], 3, VI_CS);
				State.Device->SetTexture3D(In[(size_t)VoxelType::Surface], 4, VI_CS);

				size_t PointLightsCount = GeneratePointLights();
				if (PointLightsCount > 0)
				{
					State.Device->UpdateBuffer(Voxels.PBuffer, Voxels.PArray.data(), PointLightsCount * sizeof(IPointLight));
					State.Device->SetStructureBuffer(Voxels.PBuffer, 5, VI_CS);
				}

				size_t SpotLightsCount = GenerateSpotLights();
				if (SpotLightsCount > 0)
				{
					State.Device->UpdateBuffer(Voxels.SBuffer, Voxels.SArray.data(), SpotLightsCount * sizeof(ISpotLight));
					State.Device->SetStructureBuffer(Voxels.SBuffer, 6, VI_CS);
				}

				size_t LineLightsCount = GenerateLineLights();
				if (LineLightsCount > 0)
				{
					State.Device->UpdateBuffer(Voxels.LBuffer, Voxels.LArray.data(), LineLightsCount * sizeof(ILineLight));
					State.Device->SetStructureBuffer(Voxels.LBuffer, 7, VI_CS);
				}

				State.Device->UpdateBuffer(Shaders.Voxelizer, &VoxelBuffer);
				State.Device->SetBuffer(Shaders.Voxelizer, 3, VI_CS);
				State.Device->SetShader(Shaders.Voxelizer, VI_VS | VI_PS | VI_CS);
				State.Device->Dispatch(X, Y, Z);
				State.Device->FlushTexture(2, 3, VI_CS);
				State.Device->SetShader(nullptr, VI_VS | VI_PS | VI_CS);
				State.Device->SetStructureBuffer(nullptr, 5, VI_CS);
				State.Device->SetStructureBuffer(nullptr, 6, VI_CS);
				State.Device->SetStructureBuffer(nullptr, 7, VI_CS);
				State.Device->SetWriteable(Out, 1, 1, true);
				State.Device->SetWriteable(In, 1, 3, false);
			}
			void Lighting::RenderIllumination()
			{
				if (!EnableGI || System->State.IsSubpass() || Lights.Illuminators.Top().empty())
					return;

				Graphics::ElementBuffer* Cube[2];
				System->GetPrimitives()->GetCubeBuffers(Cube);
				State.Backcull = true;

				Graphics::MultiRenderTarget2D* MRT = System->GetMRT(TargetType::Main);
				Graphics::RenderTarget2D* RT = System->GetRT(TargetType::Secondary);
				State.Device->CopyTarget(MRT, 0, RT, 0);
				State.Device->SetDepthStencilState(DepthStencilLess);
				State.Device->SetRasterizerState(BackRasterizer);
				State.Device->SetSamplerState(WrapSampler, 1, 6, VI_PS);
				State.Device->SetTexture2D(RT->GetTarget(), 1, VI_PS);
				State.Device->SetShader(Shaders.Ambient[1], VI_VS | VI_PS);
				State.Device->SetBuffer(Shaders.Ambient[1], 3, VI_VS | VI_PS);
				State.Device->SetVertexBuffer(Cube[(size_t)BufferType::Vertex]);
				State.Device->SetIndexBuffer(Cube[(size_t)BufferType::Index], Graphics::Format::R32_Uint);

				Trigonometry::Vector3 Position, Scale;
				for (auto* Light : Lights.Illuminators.Top())
				{
					if (!GetIlluminator(&VoxelBuffer, Light))
						continue;

					GetLightCulling(Light, 0.0f, &Position, &Scale);
					VoxelBuffer.Transform = Trigonometry::Matrix4x4::CreateTranslatedScale(Position, Scale) * System->View.ViewProjection;

					State.Device->SetTexture3D(Light->VoxelMap, 5, VI_PS);
					State.Device->UpdateBuffer(Shaders.Ambient[1], &VoxelBuffer);
					State.Device->DrawIndexed((uint32_t)Cube[(size_t)BufferType::Index]->GetElements(), 0, 0);
				}

				State.Device->SetTexture2D(System->GetRT(TargetType::Main)->GetTarget(), 1, VI_PS);
				State.Device->SetTexture3D(nullptr, 5, VI_PS);
				State.Device->SetVertexBuffer(System->GetPrimitives()->GetQuad());

				if (!State.Backcull)
					State.Device->SetRasterizerState(BackRasterizer);
			}
			void Lighting::RenderAmbient()
			{
				Graphics::MultiRenderTarget2D* MRT = System->GetMRT(TargetType::Main);
				Graphics::RenderTarget2D* RT = (System->State.IsSubpass() ? System->GetRT(TargetType::Main) : System->GetRT(TargetType::Secondary));
				State.Device->CopyTarget(MRT, 0, RT, 0);
				State.Device->CopyTexture2D(RT, 0, &LightingMap);
				State.Device->SetSamplerState(WrapSampler, 1, 6, VI_PS);
				State.Device->SetTexture2D(RT->GetTarget(), 5, VI_PS);
				State.Device->SetTextureCube(SkyMap, 6, VI_PS);
				State.Device->SetShader(Shaders.Ambient[0], VI_VS | VI_PS);
				State.Device->SetBuffer(Shaders.Ambient[0], 3, VI_VS | VI_PS);
				State.Device->UpdateBuffer(Shaders.Ambient[0], &AmbientLight);
				State.Device->Draw(6, 0);
			}
			void Lighting::SetSkyMap(Graphics::Texture2D* Cubemap)
			{
				Core::Memory::Release(SkyMap);
				Core::Memory::Release(SkyBase);

				SkyBase = Cubemap;
				if (SkyBase != nullptr)
				{
					SkyMap = *System->GetDevice()->CreateTextureCube(SkyBase);
					SkyBase->AddRef();
				}
			}
			void Lighting::SetSurfaceBufferSize(size_t NewSize)
			{
				VI_ASSERT(System->GetScene() != nullptr, "scene should be set");

				SceneGraph* Scene = System->GetScene();
				Graphics::GraphicsDevice* Device = System->GetDevice();
				Graphics::MultiRenderTarget2D::Desc F1 = Scene->GetDescMRT();
				F1.MipLevels = Device->GetMipLevel((uint32_t)Surfaces.Size, (uint32_t)Surfaces.Size);
				F1.Width = (uint32_t)Surfaces.Size;
				F1.Height = (uint32_t)Surfaces.Size;
				SurfaceLight.Mips = (float)F1.MipLevels;
				Surfaces.Size = NewSize;

				Core::Memory::Release(Surfaces.Merger);
				Surfaces.Merger = *Device->CreateMultiRenderTarget2D(F1);

				Graphics::Cubemap::Desc I;
				I.Source = Surfaces.Merger;
				I.MipLevels = F1.MipLevels;
				I.Size = (uint32_t)Surfaces.Size;

				Core::Memory::Release(Surfaces.Subresource);
				Surfaces.Subresource = *Device->CreateCubemap(I);

				Graphics::RenderTarget2D::Desc F2 = Scene->GetDescRT();
				F2.MipLevels = F1.MipLevels;
				F2.Width = F1.Width;
				F2.Height = F1.Height;

				Core::Memory::Release(Surfaces.Output);
				Surfaces.Output = *Device->CreateRenderTarget2D(F2);

				Core::Memory::Release(Surfaces.Input);
				Surfaces.Input = *Device->CreateRenderTarget2D(F2);
			}
			void Lighting::SetVoxelBuffer(RenderSystem* System, Graphics::Shader* Src, uint32_t Slot)
			{
				VI_ASSERT(System != nullptr, "system should be set");
				VI_ASSERT(System->GetScene() != nullptr, "scene should be set");
				VI_ASSERT(Src != nullptr, "src should be set");

				Lighting* Renderer = System->GetRenderer<Lighting>();
				if (!Renderer)
					return;

				Graphics::GraphicsDevice* Device = System->GetDevice();
				Device->SetBuffer(Src, Slot, VI_VS | VI_PS | VI_GS);
				Device->UpdateBuffer(Src, &Renderer->VoxelBuffer);
			}
			size_t Lighting::GeneratePointLights()
			{
				if (!Voxels.PBuffer)
					GenerateLightBuffers();

				Trigonometry::Vector3 Offset = 1.0f;
				Offset.X = (VoxelBuffer.Center.X > 0.0f ? 1.0f : -1.0f);
				Offset.Y = (VoxelBuffer.Center.Y > 0.0f ? 1.0f : -1.0f);
				Offset.Z = (VoxelBuffer.Center.Z > 0.0f ? 1.0f : -1.0f);

				size_t Count = 0;
				for (auto* Light : Lights.Points.Top())
				{
					if (Count >= Voxels.MaxLights)
						break;

					auto* Base = Light->GetEntity();
					Trigonometry::Vector3 Position(Base->GetTransform()->GetPosition());
					Trigonometry::Vector3 Scale(Base->GetRadius());
					Position *= Offset;

					GetPointLight(&Voxels.PArray[Count++], Light, Position, Scale, true);
				}

				VoxelBuffer.Lights.X = (float)Count;
				return Count;
			}
			size_t Lighting::GenerateSpotLights()
			{
				if (!Voxels.SBuffer)
					GenerateLightBuffers();

				Trigonometry::Vector3 Offset = 1.0f;
				Offset.X = (VoxelBuffer.Center.X > 0.0f ? 1.0f : -1.0f);
				Offset.Y = (VoxelBuffer.Center.Y > 0.0f ? 1.0f : -1.0f);
				Offset.Z = (VoxelBuffer.Center.Z > 0.0f ? 1.0f : -1.0f);

				size_t Count = 0;
				for (auto* Light : Lights.Spots.Top())
				{
					if (Count >= Voxels.MaxLights)
						break;

					auto* Base = Light->GetEntity();
					Trigonometry::Vector3 Position(Base->GetTransform()->GetPosition());
					Trigonometry::Vector3 Scale(Base->GetRadius());
					Position *= Offset;

					GetSpotLight(&Voxels.SArray[Count++], Light, Position, Scale, true);
				}

				VoxelBuffer.Lights.Y = (float)Count;
				return Count;
			}
			size_t Lighting::GenerateLineLights()
			{
				if (!Voxels.LBuffer)
					GenerateLightBuffers();

				size_t Count = 0;
				for (auto It = Lights.Lines->Begin(); It != Lights.Lines->End(); ++It)
				{
					auto* Light = (Components::LineLight*)*It;
					if (Count >= Voxels.MaxLights)
						break;

					GetLineLight(&Voxels.LArray[Count++], Light);
				}

				VoxelBuffer.Lights.Z = (float)Count;
				return Count;
			}
			size_t Lighting::RenderPass(Core::Timer* Time)
			{
				VI_ASSERT(System->GetScene() != nullptr, "scene should be set");
				if (System->State.IsSet(RenderOpt::Additive))
					return 0;

				State.Device = System->GetDevice();
				State.Scene = System->GetScene();

				if (System->State.Is(RenderState::Geometric))
				{
					if (!System->State.IsSubpass() && !System->State.IsSet(RenderOpt::Transparent))
					{
						if (Shadows.Tick.TickEvent(Time->GetElapsedMills()))
						{
							RenderPointShadowMaps(Time);
							RenderSpotShadowMaps(Time);
							RenderLineShadowMaps(Time);
							System->RestoreViewBuffer(nullptr);
						}

						RenderSurfaceMaps(Time);
						RenderVoxelMap(Time);
					}

					RenderResultBuffers();
					System->RestoreOutput();
				}
				else if (System->State.Is(RenderState::Voxelization))
					RenderLuminance();

				return 1;
			}
			float Lighting::GetDominant(const Trigonometry::Vector3& Axis)
			{
				float Max = Axis.X;
				if (Axis.Y > Max)
					Max = Axis.Y;

				if (Axis.Z > Max)
					Max = Axis.Z;

				return Max;
			}
			bool Lighting::GetSurfaceLight(ISurfaceLight* Dest, Component* Src, Trigonometry::Vector3& Position, Trigonometry::Vector3& Scale)
			{
				Components::SurfaceLight* Light = (Components::SurfaceLight*)Src;
				auto* Entity = Light->GetEntity();
				auto* Transform = Entity->GetTransform();
				auto& Size = Light->GetSize();

				Dest->Transform = Trigonometry::Matrix4x4::CreateTranslatedScale(Position, Scale) * System->View.ViewProjection;
				Dest->Position = Transform->GetPosition();
				Dest->Lighting = Light->Diffuse.Mul(Light->Emission);
				Dest->Scale = Transform->GetScale();
				Dest->Parallax = (Light->Parallax ? 1.0f : 0.0f);
				Dest->Infinity = Light->Infinity;
				Dest->Attenuation.X = Size.C1;
				Dest->Attenuation.Y = Size.C2;
				Dest->Range = Size.Radius;

				return true;
			}
			bool Lighting::GetPointLight(IPointLight* Dest, Component* Src, Trigonometry::Vector3& Position, Trigonometry::Vector3& Scale, bool Reposition)
			{
				Components::PointLight* Light = (Components::PointLight*)Src;
				auto* Entity = Light->GetEntity();
				auto* Transform = Entity->GetTransform();
				auto& Size = Light->GetSize();

				Dest->Transform = Trigonometry::Matrix4x4::CreateTranslatedScale(Position, Scale) * System->View.ViewProjection;
				Dest->Position = (Reposition ? Position : Transform->GetPosition());
				Dest->Lighting = Light->Diffuse.Mul(Light->Emission);
				Dest->Attenuation.X = Size.C1;
				Dest->Attenuation.Y = Size.C2;
				Dest->Range = Size.Radius;

				if (!Light->Shadow.Enabled || !Light->DepthMap)
				{
					Dest->Softness = 0.0f;
					return false;
				}

				Dest->Softness = Light->Shadow.Softness <= 0 ? 0 : (float)State.Scene->GetConf().PointsSize / Light->Shadow.Softness;
				Dest->Bias = Light->Shadow.Bias;
				Dest->Distance = Light->Shadow.Distance;
				Dest->Iterations = (float)Light->Shadow.Iterations;
				Dest->Umbra = Light->Disperse;
				return true;
			}
			bool Lighting::GetSpotLight(ISpotLight* Dest, Component* Src, Trigonometry::Vector3& Position, Trigonometry::Vector3& Scale, bool Reposition)
			{
				Components::SpotLight* Light = (Components::SpotLight*)Src;
				auto* Entity = Light->GetEntity();
				auto* Transform = Entity->GetTransform();
				auto& Size = Light->GetSize();

				Dest->Transform = Trigonometry::Matrix4x4::CreateTranslatedScale(Position, Scale) * System->View.ViewProjection;
				Dest->ViewProjection = Light->View * Light->Projection;
				Dest->Direction = Transform->GetRotation().dDirection();
				Dest->Position = (Reposition ? Position : Transform->GetPosition());
				Dest->Lighting = Light->Diffuse.Mul(Light->Emission);
				Dest->Cutoff = Compute::Mathf::Cos(Compute::Mathf::Deg2Rad() * Light->Cutoff * 0.5f);
				Dest->Attenuation.X = Size.C1;
				Dest->Attenuation.Y = Size.C2;
				Dest->Range = Size.Radius;

				if (!Light->Shadow.Enabled || !Light->DepthMap)
				{
					Dest->Softness = 0.0f;
					return false;
				}

				Dest->Softness = Light->Shadow.Softness <= 0 ? 0 : (float)State.Scene->GetConf().SpotsSize / Light->Shadow.Softness;
				Dest->Bias = Light->Shadow.Bias;
				Dest->Iterations = (float)Light->Shadow.Iterations;
				Dest->Umbra = Light->Disperse;
				return true;
			}
			bool Lighting::GetLineLight(ILineLight* Dest, Component* Src)
			{
				Components::LineLight* Light = (Components::LineLight*)Src;
				Dest->Position = Light->GetEntity()->GetTransform()->GetPosition().sNormalize();
				Dest->Lighting = Light->Diffuse.Mul(Light->Emission);
				Dest->RlhEmission = Light->Sky.RlhEmission;
				Dest->RlhHeight = Light->Sky.RlhHeight;
				Dest->MieEmission = Light->Sky.MieEmission;
				Dest->MieHeight = Light->Sky.MieHeight;
				Dest->ScatterIntensity = Light->Sky.Intensity;
				Dest->PlanetRadius = Light->Sky.InnerRadius;
				Dest->AtmosphereRadius = Light->Sky.OuterRadius;
				Dest->MieDirection = Light->Sky.MieDirection;
				Dest->SkyOffset = AmbientLight.SkyOffset;

				if (!Light->Shadow.Enabled || !Light->DepthMap)
				{
					Dest->Softness = 0.0f;
					return false;
				}

				Dest->Softness = Light->Shadow.Softness <= 0 ? 0 : (float)State.Scene->GetConf().LinesSize / Light->Shadow.Softness;
				Dest->Iterations = (float)Light->Shadow.Iterations;
				Dest->Umbra = Light->Disperse;
				Dest->Bias = Light->Shadow.Bias;
				Dest->Cascades = (float)std::min(Light->Shadow.Cascades, (uint32_t)Light->DepthMap->size());

				size_t Size = (size_t)Dest->Cascades;
				for (size_t i = 0; i < Size; i++)
					Dest->ViewProjection[i] = Light->View[i] * Light->Projection[i];

				return Size > 0;
			}
			bool Lighting::GetIlluminator(IVoxelBuffer* Dest, Component* Src)
			{
				auto* Light = (Components::Illuminator*)Src;
				if (!Light->VoxelMap)
					return false;

				auto& Conf = State.Scene->GetConf();
				auto* Transform = Light->GetEntity()->GetTransform();
				VoxelBuffer.Center = Transform->GetPosition();
				VoxelBuffer.Scale = Transform->GetScale();
				VoxelBuffer.Mips = (float)Conf.VoxelsMips;
				VoxelBuffer.Size = (float)Conf.VoxelsSize;
				VoxelBuffer.RayStep = Light->RayStep;
				VoxelBuffer.MaxSteps = Light->MaxSteps;
				VoxelBuffer.Distance = Light->Distance;
				VoxelBuffer.Radiance = Light->Radiance;
				VoxelBuffer.Length = Light->Length;
				VoxelBuffer.Margin = Light->Margin;
				VoxelBuffer.Offset = Light->Offset;
				VoxelBuffer.Angle = Light->Angle;
				VoxelBuffer.Occlusion = Light->Occlusion;
				VoxelBuffer.Specular = Light->Specular;
				VoxelBuffer.Bleeding = Light->Bleeding;

				return true;
			}
			void Lighting::GetLightCulling(Component* Src, float Range, Trigonometry::Vector3* Position, Trigonometry::Vector3* Scale)
			{
				auto* Transform = Src->GetEntity()->GetTransform();
				*Position = Transform->GetPosition();
				*Scale = (Range > 0.0f ? Range : Transform->GetScale());

				bool Front = Trigonometry::Geometric::HasPointIntersectedCube(*Position, Scale->Mul(1.01f), System->View.Position);
				if (!(Front && State.Backcull) && !(!Front && !State.Backcull))
					return;

				State.Device->SetRasterizerState(Front ? FrontRasterizer : BackRasterizer);
				State.Device->SetDepthStencilState(Front ? DepthStencilGreater : DepthStencilLess);
				State.Backcull = !State.Backcull;
			}
			void Lighting::GenerateLightBuffers()
			{
				Graphics::ElementBuffer::Desc F = Graphics::ElementBuffer::Desc();
				F.AccessFlags = Graphics::CPUAccess::Write;
				F.MiscFlags = Graphics::ResourceMisc::Buffer_Structured;
				F.Usage = Graphics::ResourceUsage::Dynamic;
				F.BindFlags = Graphics::ResourceBind::Shader_Input;
				F.ElementCount = (uint32_t)Voxels.MaxLights;
				F.ElementWidth = sizeof(IPointLight);
				F.StructureByteStride = F.ElementWidth;

				Core::Memory::Release(Voxels.PBuffer);
				Voxels.PBuffer = *State.Device->CreateElementBuffer(F);
				Voxels.PArray.resize(Voxels.MaxLights);

				F.ElementWidth = sizeof(ISpotLight);
				F.StructureByteStride = F.ElementWidth;
				Core::Memory::Release(Voxels.SBuffer);
				Voxels.SBuffer = *State.Device->CreateElementBuffer(F);
				Voxels.SArray.resize(Voxels.MaxLights);

				F.ElementWidth = sizeof(ILineLight);
				F.StructureByteStride = F.ElementWidth;
				Core::Memory::Release(Voxels.LBuffer);
				Voxels.LBuffer = *State.Device->CreateElementBuffer(F);
				Voxels.LArray.resize(Voxels.MaxLights);
			}
			Graphics::TextureCube* Lighting::GetSkyMap()
			{
				if (!SkyBase)
					return nullptr;

				return SkyMap;
			}
			Graphics::Texture2D* Lighting::GetSkyBase()
			{
				if (!SkyMap)
					return nullptr;

				return SkyBase;
			}

			Transparency::Transparency(RenderSystem* Lab) : Renderer(Lab)
			{
				VI_ASSERT(System != nullptr, "render system should be set");
				VI_ASSERT(System->GetDevice() != nullptr, "graphics device should be set");

				Graphics::GraphicsDevice* Device = System->GetDevice();
				DepthStencil = Device->GetDepthStencilState("doo_soo_lt");
				Rasterizer = Device->GetRasterizerState("so_cback");
				Blend = Device->GetBlendState("bo_wrgba_one");
				Sampler = Device->GetSamplerState("a16_fa_wrap");
				Layout = Device->GetInputLayout("vx_shape");

				Shader = *System->CompileShader("postprocessing/transparency", sizeof(RenderData));
			}
			Transparency::~Transparency()
			{
				System->FreeShader(Shader);
				Core::Memory::Release(Merger);
				Core::Memory::Release(Input);
			}
			void Transparency::ResizeBuffers()
			{
				VI_ASSERT(System->GetScene() != nullptr, "scene should be set");

				SceneGraph* Scene = System->GetScene();
				Graphics::MultiRenderTarget2D::Desc F1 = Scene->GetDescMRT();
				Graphics::RenderTarget2D::Desc F2 = Scene->GetDescRT();
				Graphics::GraphicsDevice* Device = System->GetDevice();
				MipLevels[(size_t)TargetType::Main] = (float)F1.MipLevels;

				auto* Renderer = System->GetRenderer<Lighting>();
				if (Renderer != nullptr)
				{
					MipLevels[(size_t)TargetType::Secondary] = (float)Device->GetMipLevel((uint32_t)Renderer->Surfaces.Size, (uint32_t)Renderer->Surfaces.Size);
					F1.MipLevels = (uint32_t)MipLevels[(size_t)TargetType::Secondary];
					F1.Width = (uint32_t)Renderer->Surfaces.Size;
					F1.Height = (uint32_t)Renderer->Surfaces.Size;
					F2.MipLevels = F1.MipLevels;
					F2.Width = F1.Width;
					F2.Height = F1.Height;
				}

				Core::Memory::Release(Merger);
				Merger = *Device->CreateMultiRenderTarget2D(F1);

				Core::Memory::Release(Input);
				Input = *Device->CreateRenderTarget2D(F2);
			}
			size_t Transparency::RenderPass(Core::Timer* Time)
			{
				VI_ASSERT(System->GetScene() != nullptr, "scene should be set");
				if (!System->State.Is(RenderState::Geometric) || System->State.IsSet(RenderOpt::Transparent) || System->State.IsSet(RenderOpt::Additive))
					return 0;

				SceneGraph* Scene = System->GetScene();
				if (System->HasCategory(GeoCategory::Additive))
					Scene->Statistics.DrawCalls += System->Render(Time, RenderState::Geometric, System->State.GetOpts() | RenderOpt::Additive);

				if (!System->HasCategory(GeoCategory::Transparent))
					return 0;

				Graphics::MultiRenderTarget2D* MainMRT = System->GetMRT(TargetType::Main);
				Graphics::MultiRenderTarget2D* MRT = (System->State.IsSubpass() ? Merger : System->GetMRT(TargetType::Secondary));
				Graphics::RenderTarget2D* RT = (System->State.IsSubpass() ? Input : System->GetRT(TargetType::Main));
				Graphics::GraphicsDevice* Device = System->GetDevice();
				RenderData.Mips = (System->State.IsSubpass() ? MipLevels[(size_t)TargetType::Secondary] : MipLevels[(size_t)TargetType::Main]);

				Scene->SwapMRT(TargetType::Main, MRT);
				Scene->SetMRT(TargetType::Main, true);
				Scene->Statistics.DrawCalls += System->Render(Time, RenderState::Geometric, System->State.GetOpts() | RenderOpt::Transparent);
				Scene->SwapMRT(TargetType::Main, nullptr);

				Device->CopyTarget(MainMRT, 0, RT, 0);
				Device->GenerateMips(RT->GetTarget());
				Device->SetTarget(MainMRT, 0);
				Device->Clear(MainMRT, 0, 0, 0, 0);
				Device->UpdateBuffer(Shader, &RenderData);
				Device->SetDepthStencilState(DepthStencil);
				Device->SetBlendState(Blend);
				Device->SetRasterizerState(Rasterizer);
				Device->SetInputLayout(Layout);
				Device->SetSamplerState(Sampler, 1, 8, VI_PS);
				Device->SetTexture2D(RT->GetTarget(), 1, VI_PS);
				Device->SetTexture2D(MainMRT->GetTarget(1), 2, VI_PS);
				Device->SetTexture2D(MainMRT->GetTarget(2), 3, VI_PS);
				Device->SetTexture2D(MainMRT->GetTarget(3), 4, VI_PS);
				Device->SetTexture2D(MRT->GetTarget(0), 5, VI_PS);
				Device->SetTexture2D(MRT->GetTarget(1), 6, VI_PS);
				Device->SetTexture2D(MRT->GetTarget(2), 7, VI_PS);
				Device->SetTexture2D(MRT->GetTarget(3), 8, VI_PS);
				Device->SetShader(Shader, VI_VS | VI_PS);
				Device->SetBuffer(Shader, 3, VI_VS | VI_PS);
				Device->SetVertexBuffer(System->GetPrimitives()->GetQuad());
				System->UpdateConstantBuffer(RenderBufferType::Render);
				Device->Draw(6, 0);
				Device->FlushTexture(1, 8, VI_PS);
				System->RestoreOutput();
				return 1;
			}

			SSR::SSR(RenderSystem* Lab) : EffectRenderer(Lab)
			{
				Shaders.Reflectance = *CompileEffect("postprocessing/reflectance", sizeof(Reflectance));
				Shaders.Gloss[0] = *CompileEffect("postprocessing/gloss_x", sizeof(Gloss));
				Shaders.Gloss[1] = *CompileEffect("postprocessing/gloss_y");
				Shaders.Additive = *CompileEffect("postprocessing/additive");
			}
			void SSR::Deserialize(Core::Schema* Node)
			{
				VI_ASSERT(Node != nullptr, "schema should be set");

				Series::Unpack(Node->Find("samples-1"), &Reflectance.Samples);
				Series::Unpack(Node->Find("samples-2"), &Gloss.Samples);
				Series::Unpack(Node->Find("intensity"), &Reflectance.Intensity);
				Series::Unpack(Node->Find("distance"), &Reflectance.Distance);
				Series::Unpack(Node->Find("cutoff"), &Gloss.Cutoff);
				Series::Unpack(Node->Find("blur"), &Gloss.Blur);
				Series::Unpack(Node->Find("deadzone"), &Gloss.Deadzone);
				Series::Unpack(Node->Find("mips"), &Gloss.Mips);
			}
			void SSR::Serialize(Core::Schema* Node)
			{
				VI_ASSERT(Node != nullptr, "schema should be set");

				Series::Pack(Node->Set("samples-1"), Reflectance.Samples);
				Series::Pack(Node->Set("samples-2"), Gloss.Samples);
				Series::Pack(Node->Set("intensity"), Reflectance.Intensity);
				Series::Pack(Node->Set("distance"), Reflectance.Distance);
				Series::Pack(Node->Set("cutoff"), Gloss.Cutoff);
				Series::Pack(Node->Set("blur"), Gloss.Blur);
				Series::Pack(Node->Set("deadzone"), Gloss.Deadzone);
				Series::Pack(Node->Set("mips"), Gloss.Mips);
			}
			void SSR::RenderEffect(Core::Timer* Time)
			{
				Graphics::MultiRenderTarget2D* MRT = System->GetMRT(TargetType::Main);
				System->GetDevice()->GenerateMips(MRT->GetTarget(0));

				Gloss.Mips = (float)GetMipLevels();
				Gloss.Texel[0] = 1.0f / GetWidth();
				Gloss.Texel[1] = 1.0f / GetHeight();

				RenderMerge(Shaders.Reflectance, &Reflectance);
				SampleClamp();
				RenderMerge(Shaders.Gloss[0], &Gloss, 2);
				RenderMerge(Shaders.Gloss[1], nullptr, 2);
				SampleWrap();
				RenderResult(Shaders.Additive);
			}

			SSGI::SSGI(RenderSystem* Lab) : EffectRenderer(Lab), EmissionMap(nullptr)
			{
				Shaders.Stochastic = *CompileEffect("postprocessing/stochastic", sizeof(Stochastic));
				Shaders.Indirection = *CompileEffect("postprocessing/indirection", sizeof(Indirection));
				Shaders.Denoise[0] = *CompileEffect("postprocessing/denoise_x", sizeof(Denoise));
				Shaders.Denoise[1] = *CompileEffect("postprocessing/denoise_y");
				Shaders.Additive = *CompileEffect("postprocessing/additive");
			}
			SSGI::~SSGI()
			{
				Core::Memory::Release(EmissionMap);
			}
			void SSGI::Deserialize(Core::Schema* Node)
			{
				VI_ASSERT(Node != nullptr, "schema should be set");

				Series::Unpack(Node->Find("samples-1"), &Indirection.Samples);
				Series::Unpack(Node->Find("samples-2"), &Denoise.Samples);
				Series::Unpack(Node->Find("cutoff-1"), &Indirection.Cutoff);
				Series::Unpack(Node->Find("cutoff-2"), &Denoise.Cutoff);
				Series::Unpack(Node->Find("attenuation"), &Indirection.Attenuation);
				Series::Unpack(Node->Find("swing"), &Indirection.Swing);
				Series::Unpack(Node->Find("bias"), &Indirection.Bias);
				Series::Unpack(Node->Find("distance"), &Indirection.Distance);
				Series::Unpack(Node->Find("blur"), &Denoise.Blur);
			}
			void SSGI::Serialize(Core::Schema* Node)
			{
				VI_ASSERT(Node != nullptr, "schema should be set");

				Series::Pack(Node->Set("samples-1"), Indirection.Samples);
				Series::Pack(Node->Set("samples-2"), Denoise.Samples);
				Series::Pack(Node->Set("cutoff-1"), Indirection.Cutoff);
				Series::Pack(Node->Set("cutoff-2"), Denoise.Cutoff);
				Series::Pack(Node->Set("attenuation"), Indirection.Attenuation);
				Series::Pack(Node->Set("swing"), Indirection.Swing);
				Series::Pack(Node->Set("bias"), Indirection.Bias);
				Series::Pack(Node->Set("distance"), Indirection.Distance);
				Series::Pack(Node->Set("blur"), Denoise.Blur);
			}
			void SSGI::RenderEffect(Core::Timer* Time)
			{
				Indirection.Random[0] = Compute::Math<float>::Random();
				Indirection.Random[1] = Compute::Math<float>::Random();
				Denoise.Texel[0] = 1.0f / (float)GetWidth();
				Denoise.Texel[1] = 1.0f / (float)GetHeight();
				Stochastic.Texel[0] = (float)GetWidth();
				Stochastic.Texel[1] = (float)GetHeight();
				Stochastic.FrameId++;

				float Distance = Indirection.Distance;
				float Swing = Indirection.Swing;
				float Bias = Indirection.Bias;
				RenderTexture(0, EmissionMap);
				RenderCopyMain(0, EmissionMap);
				RenderMerge(Shaders.Stochastic, &Stochastic);
				for (uint32_t i = 0; i < Bounces; i++)
				{
					float Bounce = (float)(i + 1);
					Indirection.Distance = Distance * Bounce;
					Indirection.Swing = Swing / Bounce;
					Indirection.Bias = Bias * Bounce;
					Indirection.Initial = i > 0 ? 0.0f : 1.0f;
					RenderMerge(Shaders.Indirection, &Indirection);
					if (i + 1 < Bounces)
						RenderCopyLast(EmissionMap);
				}
				SampleClamp();
				RenderMerge(Shaders.Denoise[0], &Denoise, 3);
				RenderMerge(Shaders.Denoise[1], nullptr, 3);
				SampleWrap();
				RenderResult(Shaders.Additive);
				Indirection.Distance = Distance;
				Indirection.Swing = Swing;
				Indirection.Bias = Bias;
			}
			void SSGI::ResizeEffect()
			{
				VI_ASSERT(System->GetScene() != nullptr, "scene should be set");
				Core::Memory::Release(EmissionMap);

				SceneGraph* Scene = System->GetScene();
				Graphics::GraphicsDevice* Device = System->GetDevice();
				Device->CopyTexture2D(Scene->GetRT(TargetType::Main), 0, &EmissionMap);
			}

			SSAO::SSAO(RenderSystem* Lab) : EffectRenderer(Lab)
			{
				Shaders.Shading = *CompileEffect("postprocessing/shading", sizeof(Shading));
				Shaders.Fibo[0] = *CompileEffect("postprocessing/fibo_x", sizeof(Fibo));
				Shaders.Fibo[1] = *CompileEffect("postprocessing/fibo_y");
				Shaders.Multiply = *CompileEffect("postprocessing/multiply");
			}
			void SSAO::Deserialize(Core::Schema* Node)
			{
				VI_ASSERT(Node != nullptr, "schema should be set");

				Series::Unpack(Node->Find("samples-1"), &Shading.Samples);
				Series::Unpack(Node->Find("scale"), &Shading.Scale);
				Series::Unpack(Node->Find("intensity"), &Shading.Intensity);
				Series::Unpack(Node->Find("bias"), &Shading.Bias);
				Series::Unpack(Node->Find("radius"), &Shading.Radius);
				Series::Unpack(Node->Find("distance"), &Shading.Distance);
				Series::Unpack(Node->Find("fade"), &Shading.Fade);
				Series::Unpack(Node->Find("power"), &Fibo.Power);
				Series::Unpack(Node->Find("samples-2"), &Fibo.Samples);
				Series::Unpack(Node->Find("blur"), &Fibo.Blur);
			}
			void SSAO::Serialize(Core::Schema* Node)
			{
				VI_ASSERT(Node != nullptr, "schema should be set");

				Series::Pack(Node->Set("samples-1"), Shading.Samples);
				Series::Pack(Node->Set("scale"), Shading.Scale);
				Series::Pack(Node->Set("intensity"), Shading.Intensity);
				Series::Pack(Node->Set("bias"), Shading.Bias);
				Series::Pack(Node->Set("radius"), Shading.Radius);
				Series::Pack(Node->Set("distance"), Shading.Distance);
				Series::Pack(Node->Set("fade"), Shading.Fade);
				Series::Pack(Node->Set("power"), Fibo.Power);
				Series::Pack(Node->Set("samples-2"), Fibo.Samples);
				Series::Pack(Node->Set("blur"), Fibo.Blur);
			}
			void SSAO::RenderEffect(Core::Timer* Time)
			{
				Fibo.Texel[0] = 1.0f / GetWidth();
				Fibo.Texel[1] = 1.0f / GetHeight();

				RenderMerge(Shaders.Shading, &Shading);
				SampleClamp();
				RenderMerge(Shaders.Fibo[0], &Fibo, 2);
				RenderMerge(Shaders.Fibo[1], nullptr, 2);
				SampleWrap();
				RenderResult(Shaders.Multiply);
			}

			DoF::DoF(RenderSystem* Lab) : EffectRenderer(Lab)
			{
				*CompileEffect("postprocessing/focus", sizeof(Focus));
			}
			void DoF::Deserialize(Core::Schema* Node)
			{
				VI_ASSERT(Node != nullptr, "schema should be set");

				Series::Unpack(Node->Find("distance"), &Distance);
				Series::Unpack(Node->Find("time"), &Time);
				Series::Unpack(Node->Find("radius"), &Radius);
				Series::Unpack(Node->Find("radius"), &Focus.Radius);
				Series::Unpack(Node->Find("bokeh"), &Focus.Bokeh);
				Series::Unpack(Node->Find("scale"), &Focus.Scale);
				Series::Unpack(Node->Find("near-distance"), &Focus.NearDistance);
				Series::Unpack(Node->Find("near-range"), &Focus.NearRange);
				Series::Unpack(Node->Find("far-distance"), &Focus.FarDistance);
				Series::Unpack(Node->Find("far-range"), &Focus.FarRange);
			}
			void DoF::Serialize(Core::Schema* Node)
			{
				VI_ASSERT(Node != nullptr, "schema should be set");

				Series::Pack(Node->Set("distance"), Distance);
				Series::Pack(Node->Set("time"), Time);
				Series::Pack(Node->Set("radius"), Radius);
				Series::Pack(Node->Set("radius"), Focus.Radius);
				Series::Pack(Node->Set("bokeh"), Focus.Bokeh);
				Series::Pack(Node->Set("scale"), Focus.Scale);
				Series::Pack(Node->Set("near-distance"), Focus.NearDistance);
				Series::Pack(Node->Set("near-range"), Focus.NearRange);
				Series::Pack(Node->Set("far-distance"), Focus.FarDistance);
				Series::Pack(Node->Set("far-range"), Focus.FarRange);
			}
			void DoF::RenderEffect(Core::Timer* fTime)
			{
				VI_ASSERT(fTime != nullptr, "time should be set");
				if (Distance > 0.0f)
					FocusAtNearestTarget(fTime->GetStep());

				Focus.Texel[0] = 1.0f / GetWidth();
				Focus.Texel[1] = 1.0f / GetHeight();
				RenderResult(nullptr, &Focus);
			}
			void DoF::FocusAtNearestTarget(float Step)
			{
				VI_ASSERT(System->GetScene() != nullptr, "scene should be set");

				Trigonometry::Ray Origin;
				Origin.Origin = System->View.Position;
				Origin.Direction = System->View.Rotation.dDirection();

				bool Change = false;
				for (auto& Hit : System->GetScene()->QueryByRay<Components::Model>(Origin))
				{
					float Radius = Hit.first->GetEntity()->GetRadius();
					float Spacing = Origin.Origin.Distance(Hit.second) + Radius / 2.0f;
					if (Spacing <= Focus.NearRange || Spacing + Radius / 2.0f >= Distance)
						continue;

					if (Spacing < State.Distance || State.Distance <= 0.0f)
					{
						State.Distance = Spacing;
						State.Range = Radius;
						Change = true;
					}
				};

				if (Change)
				{
					State.Radius = Focus.Radius;
					State.Factor = 0.0f;
				}

				State.Factor += Time * Step;
				if (State.Factor > 1.0f)
					State.Factor = 1.0f;

				if (State.Distance > 0.0f)
				{
					State.Distance += State.Range / 2.0f + Focus.FarRange;
					Focus.FarDistance = State.Distance;
					Focus.Radius = Compute::Math<float>::Lerp(State.Radius, Radius, State.Factor);
				}
				else
				{
					State.Distance = 0.0f;
					if (State.Factor >= 1.0f)
						Focus.FarDistance = State.Distance;

					Focus.Radius = Compute::Math<float>::Lerp(State.Radius, 0.0f, State.Factor);
				}

				if (Focus.Radius < 0.0f)
					Focus.Radius = 0.0f;
			}

			MotionBlur::MotionBlur(RenderSystem* Lab) : EffectRenderer(Lab)
			{
				Shaders.Velocity = *CompileEffect("postprocessing/velocity", sizeof(Velocity));
				Shaders.Motion = *CompileEffect("postprocessing/motion", sizeof(Motion));
			}
			void MotionBlur::Deserialize(Core::Schema* Node)
			{
				VI_ASSERT(Node != nullptr, "schema should be set");

				Series::Unpack(Node->Find("samples"), &Motion.Samples);
				Series::Unpack(Node->Find("blur"), &Motion.Blur);
				Series::Unpack(Node->Find("motion"), &Motion.Motion);
			}
			void MotionBlur::Serialize(Core::Schema* Node)
			{
				VI_ASSERT(Node != nullptr, "schema should be set");

				Series::Pack(Node->Set("samples"), Motion.Samples);
				Series::Pack(Node->Set("blur"), Motion.Blur);
				Series::Pack(Node->Set("motion"), Motion.Motion);
			}
			void MotionBlur::RenderEffect(Core::Timer* Time)
			{
				VI_ASSERT(System->GetScene() != nullptr, "scene should be set");
				RenderMerge(Shaders.Velocity, &Velocity);
				RenderResult(Shaders.Motion, &Motion);
				Velocity.LastViewProjection = System->View.ViewProjection;
			}

			Bloom::Bloom(RenderSystem* Lab) : EffectRenderer(Lab)
			{
				Shaders.Bloom = *CompileEffect("postprocessing/bloom", sizeof(Extraction));
				Shaders.Fibo[0] = *CompileEffect("postprocessing/fibo_x", sizeof(Fibo));
				Shaders.Fibo[1] = *CompileEffect("postprocessing/fibo_y");
				Shaders.Additive = *CompileEffect("postprocessing/additive");
			}
			void Bloom::Deserialize(Core::Schema* Node)
			{
				Series::Unpack(Node->Find("intensity"), &Extraction.Intensity);
				Series::Unpack(Node->Find("threshold"), &Extraction.Threshold);
				Series::Unpack(Node->Find("power"), &Fibo.Power);
				Series::Unpack(Node->Find("samples"), &Fibo.Samples);
				Series::Unpack(Node->Find("blur"), &Fibo.Blur);
			}
			void Bloom::Serialize(Core::Schema* Node)
			{
				Series::Pack(Node->Set("intensity"), Extraction.Intensity);
				Series::Pack(Node->Set("threshold"), Extraction.Threshold);
				Series::Pack(Node->Set("power"), Fibo.Power);
				Series::Pack(Node->Set("samples"), Fibo.Samples);
				Series::Pack(Node->Set("blur"), Fibo.Blur);
			}
			void Bloom::RenderEffect(Core::Timer* Time)
			{
				Fibo.Texel[0] = 1.0f / GetWidth();
				Fibo.Texel[1] = 1.0f / GetHeight();

				RenderMerge(Shaders.Bloom, &Extraction);
				SampleClamp();
				RenderMerge(Shaders.Fibo[0], &Fibo, 3);
				RenderMerge(Shaders.Fibo[1], nullptr, 3);
				SampleWrap();
				RenderResult(Shaders.Additive);
			}

			Tone::Tone(RenderSystem* Lab) : EffectRenderer(Lab)
			{
				Shaders.Luminance = *CompileEffect("postprocessing/luminance", sizeof(Luminance));
				Shaders.Tone = *CompileEffect("postprocessing/tone", sizeof(Mapping));
			}
			Tone::~Tone()
			{
				Core::Memory::Release(LutTarget);
				Core::Memory::Release(LutMap);
			}
			void Tone::Deserialize(Core::Schema* Node)
			{
				VI_ASSERT(Node != nullptr, "schema should be set");

				Series::Unpack(Node->Find("grayscale"), &Mapping.Grayscale);
				Series::Unpack(Node->Find("aces"), &Mapping.ACES);
				Series::Unpack(Node->Find("filmic"), &Mapping.Filmic);
				Series::Unpack(Node->Find("lottes"), &Mapping.Lottes);
				Series::Unpack(Node->Find("reinhard"), &Mapping.Reinhard);
				Series::Unpack(Node->Find("reinhard2"), &Mapping.Reinhard2);
				Series::Unpack(Node->Find("unreal"), &Mapping.Unreal);
				Series::Unpack(Node->Find("uchimura"), &Mapping.Uchimura);
				Series::Unpack(Node->Find("ubrightness"), &Mapping.UBrightness);
				Series::Unpack(Node->Find("usontrast"), &Mapping.UContrast);
				Series::Unpack(Node->Find("ustart"), &Mapping.UStart);
				Series::Unpack(Node->Find("ulength"), &Mapping.ULength);
				Series::Unpack(Node->Find("ublack"), &Mapping.UBlack);
				Series::Unpack(Node->Find("upedestal"), &Mapping.UPedestal);
				Series::Unpack(Node->Find("exposure"), &Mapping.Exposure);
				Series::Unpack(Node->Find("eintensity"), &Mapping.EIntensity);
				Series::Unpack(Node->Find("egamma"), &Mapping.EGamma);
				Series::Unpack(Node->Find("adaptation"), &Mapping.Adaptation);
				Series::Unpack(Node->Find("agray"), &Mapping.AGray);
				Series::Unpack(Node->Find("awhite"), &Mapping.AWhite);
				Series::Unpack(Node->Find("ablack"), &Mapping.ABlack);
				Series::Unpack(Node->Find("aspeed"), &Mapping.ASpeed);
			}
			void Tone::Serialize(Core::Schema* Node)
			{
				VI_ASSERT(Node != nullptr, "schema should be set");

				Series::Pack(Node->Set("grayscale"), Mapping.Grayscale);
				Series::Pack(Node->Set("aces"), Mapping.ACES);
				Series::Pack(Node->Set("filmic"), Mapping.Filmic);
				Series::Pack(Node->Set("lottes"), Mapping.Lottes);
				Series::Pack(Node->Set("reinhard"), Mapping.Reinhard);
				Series::Pack(Node->Set("reinhard2"), Mapping.Reinhard2);
				Series::Pack(Node->Set("unreal"), Mapping.Unreal);
				Series::Pack(Node->Set("uchimura"), Mapping.Uchimura);
				Series::Pack(Node->Set("ubrightness"), Mapping.UBrightness);
				Series::Pack(Node->Set("usontrast"), Mapping.UContrast);
				Series::Pack(Node->Set("ustart"), Mapping.UStart);
				Series::Pack(Node->Set("ulength"), Mapping.ULength);
				Series::Pack(Node->Set("ublack"), Mapping.UBlack);
				Series::Pack(Node->Set("upedestal"), Mapping.UPedestal);
				Series::Pack(Node->Set("exposure"), Mapping.Exposure);
				Series::Pack(Node->Set("eintensity"), Mapping.EIntensity);
				Series::Pack(Node->Set("egamma"), Mapping.EGamma);
				Series::Pack(Node->Set("adaptation"), Mapping.Adaptation);
				Series::Pack(Node->Set("agray"), Mapping.AGray);
				Series::Pack(Node->Set("awhite"), Mapping.AWhite);
				Series::Pack(Node->Set("ablack"), Mapping.ABlack);
				Series::Pack(Node->Set("aspeed"), Mapping.ASpeed);
			}
			void Tone::RenderEffect(Core::Timer* Time)
			{
				if (Mapping.Adaptation > 0.0f)
					RenderLUT(Time);

				RenderResult(Shaders.Tone, &Mapping);
			}
			void Tone::RenderLUT(Core::Timer* Time)
			{
				VI_ASSERT(Time != nullptr, "time should be set");
				if (!LutMap || !LutTarget)
					SetLUTSize(1);

				Graphics::MultiRenderTarget2D* MRT = System->GetMRT(TargetType::Main);
				Graphics::GraphicsDevice* Device = System->GetDevice();
				Device->GenerateMips(MRT->GetTarget(0));
				Device->CopyTexture2D(LutTarget, 0, &LutMap);

				Luminance.Texel[0] = 1.0f / (float)GetWidth();
				Luminance.Texel[1] = 1.0f / (float)GetHeight();
				Luminance.Mips = (float)GetMipLevels();
				Luminance.Time = Time->GetStep() * Mapping.ASpeed;

				RenderTexture(0, LutMap);
				RenderOutput(LutTarget);
				RenderMerge(Shaders.Luminance, &Luminance);
			}
			void Tone::SetLUTSize(size_t Size)
			{
				VI_ASSERT(System->GetScene() != nullptr, "scene should be set");
				Core::Memory::Release(LutTarget);
				Core::Memory::Release(LutMap);

				SceneGraph* Scene = System->GetScene();
				Graphics::GraphicsDevice* Device = System->GetDevice();
				Graphics::RenderTarget2D::Desc RT = Scene->GetDescRT();
				RT.MipLevels = Device->GetMipLevel((uint32_t)Size, (uint32_t)Size);
				RT.FormatMode = Graphics::Format::R16_Float;
				RT.Width = (uint32_t)Size;
				RT.Height = (uint32_t)Size;

				LutTarget = *Device->CreateRenderTarget2D(RT);
				Device->CopyTexture2D(LutTarget, 0, &LutMap);
			}

			Glitch::Glitch(RenderSystem* Lab) : EffectRenderer(Lab), ScanLineJitter(0), VerticalJump(0), HorizontalShake(0), ColorDrift(0)
			{
				*CompileEffect("postprocessing/glitch", sizeof(Distortion));
			}
			void Glitch::Deserialize(Core::Schema* Node)
			{
				VI_ASSERT(Node != nullptr, "schema should be set");

				Series::Unpack(Node->Find("scanline-jitter"), &ScanLineJitter);
				Series::Unpack(Node->Find("vertical-jump"), &VerticalJump);
				Series::Unpack(Node->Find("horizontal-shake"), &HorizontalShake);
				Series::Unpack(Node->Find("color-drift"), &ColorDrift);
				Series::Unpack(Node->Find("horizontal-shake"), &HorizontalShake);
				Series::Unpack(Node->Find("elapsed-time"), &Distortion.ElapsedTime);
				Series::Unpack(Node->Find("scanline-jitter-displacement"), &Distortion.ScanLineJitterDisplacement);
				Series::Unpack(Node->Find("scanline-jitter-threshold"), &Distortion.ScanLineJitterThreshold);
				Series::Unpack(Node->Find("vertical-jump-amount"), &Distortion.VerticalJumpAmount);
				Series::Unpack(Node->Find("vertical-jump-time"), &Distortion.VerticalJumpTime);
				Series::Unpack(Node->Find("color-drift-amount"), &Distortion.ColorDriftAmount);
				Series::Unpack(Node->Find("color-drift-time"), &Distortion.ColorDriftTime);
			}
			void Glitch::Serialize(Core::Schema* Node)
			{
				VI_ASSERT(Node != nullptr, "schema should be set");

				Series::Pack(Node->Set("scanline-jitter"), ScanLineJitter);
				Series::Pack(Node->Set("vertical-jump"), VerticalJump);
				Series::Pack(Node->Set("horizontal-shake"), HorizontalShake);
				Series::Pack(Node->Set("color-drift"), ColorDrift);
				Series::Pack(Node->Set("horizontal-shake"), HorizontalShake);
				Series::Pack(Node->Set("elapsed-time"), Distortion.ElapsedTime);
				Series::Pack(Node->Set("scanline-jitter-displacement"), Distortion.ScanLineJitterDisplacement);
				Series::Pack(Node->Set("scanline-jitter-threshold"), Distortion.ScanLineJitterThreshold);
				Series::Pack(Node->Set("vertical-jump-amount"), Distortion.VerticalJumpAmount);
				Series::Pack(Node->Set("vertical-jump-time"), Distortion.VerticalJumpTime);
				Series::Pack(Node->Set("color-drift-amount"), Distortion.ColorDriftAmount);
				Series::Pack(Node->Set("color-drift-time"), Distortion.ColorDriftTime);
			}
			void Glitch::RenderEffect(Core::Timer* Time)
			{
				if (Distortion.ElapsedTime >= 32000.0f)
					Distortion.ElapsedTime = 0.0f;

				float Step = Time->GetStep() * 10.0f;
				Distortion.ElapsedTime += Step * 10.0f;
				Distortion.VerticalJumpAmount = VerticalJump;
				Distortion.VerticalJumpTime += Step * VerticalJump * 11.3f;
				Distortion.ScanLineJitterThreshold = Compute::Mathf::Saturate(1.0f - ScanLineJitter * 1.2f);
				Distortion.ScanLineJitterDisplacement = 0.002f + Compute::Mathf::Pow(ScanLineJitter, 3) * 0.05f;
				Distortion.HorizontalShake = HorizontalShake * 0.2f;
				Distortion.ColorDriftAmount = ColorDrift * 0.04f;
				Distortion.ColorDriftTime = Distortion.ElapsedTime * 606.11f;
				RenderResult(nullptr, &Distortion);
			}

			UserInterface::UserInterface(RenderSystem* Lab) : UserInterface(Lab, HeavyApplication::HasInstance() ? HeavyApplication::Get()->FetchUI() : nullptr, HeavyApplication::HasInstance() ? HeavyApplication::Get()->Activity : nullptr)
			{
			}
			UserInterface::UserInterface(RenderSystem* Lab, GUI::Context* NewContext, Graphics::Activity* NewActivity) : Renderer(Lab), Activity(NewActivity), Context(NewContext)
			{
				VI_ASSERT(System != nullptr, "render system should be set");
				VI_ASSERT(System->GetDevice() != nullptr, "graphics device should be set");
				VI_ASSERT(Activity != nullptr, "activity should be set");
				VI_ASSERT(Context != nullptr, "context should be set");
			}
			size_t UserInterface::RenderPass(Core::Timer* Timer)
			{
				VI_ASSERT(Context != nullptr, "context should be set");
				if (!System->State.Is(RenderState::Geometric) || System->State.IsSubpass() || System->State.IsSet(RenderOpt::Transparent) || System->State.IsSet(RenderOpt::Additive))
					return 0;

				Context->UpdateEvents(Activity);
				Context->RenderLists(System->GetMRT(TargetType::Main)->GetTarget(0));
				System->RestoreOutput();

				return 1;
			}
			GUI::Context* UserInterface::GetContext()
			{
				return Context;
			}
		}
	}
}
