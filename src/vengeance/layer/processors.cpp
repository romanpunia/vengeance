#include "processors.h"
#include "components.h"
#include "renderers.h"
#include <vitex/network/http.h>
#ifdef VI_OPENAL
#ifdef VI_AL_AT_OPENAL
#include <OpenAL/al.h>
#else
#include <AL/al.h>
#endif
#endif
#ifdef VI_SDL2
#include "../internal/sdl2.hpp"
#endif
#ifdef VI_ASSIMP
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/matrix4x4.h>
#include <assimp/cimport.h>
#include <assimp/postprocess.h> 
#endif
#ifdef VI_STB
extern "C"
{
#define STB_VORBIS_HEADER_ONLY
#include <stb_image.h>
#include <stb_vorbis.c>
}
#endif

namespace Vitex
{
	namespace Layer
	{
		namespace Processors
		{
#ifdef VI_ASSIMP
			Trigonometry::Matrix4x4 FromAssimpMatrix(const aiMatrix4x4& Root)
			{
				return Trigonometry::Matrix4x4(
					Root.a1, Root.a2, Root.a3, Root.a4,
					Root.b1, Root.b2, Root.b3, Root.b4,
					Root.c1, Root.c2, Root.c3, Root.c4,
					Root.d1, Root.d2, Root.d3, Root.d4).Transpose();
			}
			Core::String GetMeshName(const std::string_view& Name, ModelInfo* Info)
			{
				if (Name.empty())
				{
					auto Random = Compute::Crypto::RandomBytes(8);
					if (!Random)
						return "NULL";

					auto Hash = Compute::Crypto::HashHex(Compute::Digests::MD5(), *Random);
					if (!Hash)
						return "NULL";

					return Hash->substr(0, 8);
				}

				Core::String Result = Core::String(Name);
				for (auto&& Data : Info->Meshes)
				{
					if (Data.Name == Result)
						Result += '_';
				}

				return Result;
			}
			Core::String GetJointName(const std::string_view& BaseName, ModelInfo* Info, MeshBlob* Blob)
			{
				if (!BaseName.empty())
					return Core::String(BaseName);

				Core::String Name = Core::String(BaseName) + '?';
				while (Info->JointOffsets.find(Name) != Info->JointOffsets.end())
					Name += '?';

				return Name;
			}
			bool GetKeyFromTime(Core::Vector<Trigonometry::AnimatorKey>& Keys, float Time, Trigonometry::AnimatorKey& Result)
			{
				for (auto& Key : Keys)
				{
					if (Key.Time == Time)
					{
						Result = Key;
						return true;
					}
					
					if (Key.Time < Time)
						Result = Key;
				}

				return false;
			}
			void UpdateSceneBounds(ModelInfo* Info)
			{
				if (Info->Min.X < Info->Low)
					Info->Low = Info->Min.X;

				if (Info->Min.Y < Info->Low)
					Info->Low = Info->Min.Y;

				if (Info->Min.Z < Info->Low)
					Info->Low = Info->Min.Z;

				if (Info->Max.X > Info->High)
					Info->High = Info->Max.X;

				if (Info->Max.Y > Info->High)
					Info->High = Info->Max.Y;

				if (Info->Max.Z > Info->High)
					Info->High = Info->Max.Z;
			}
			void UpdateSceneBounds(ModelInfo* Info, const Trigonometry::SkinVertex& Element)
			{
				if (Element.PositionX > Info->Max.X)
					Info->Max.X = Element.PositionX;
				else if (Element.PositionX < Info->Min.X)
					Info->Min.X = Element.PositionX;

				if (Element.PositionY > Info->Max.Y)
					Info->Max.Y = Element.PositionY;
				else if (Element.PositionY < Info->Min.Y)
					Info->Min.Y = Element.PositionY;

				if (Element.PositionZ > Info->Max.Z)
					Info->Max.Z = Element.PositionZ;
				else if (Element.PositionZ < Info->Min.Z)
					Info->Min.Z = Element.PositionZ;
			}
			bool FillSceneSkeleton(ModelInfo* Info, aiNode* Node, Trigonometry::Joint* Top)
			{
				Core::String Name = Node->mName.C_Str();
				auto It = Info->JointOffsets.find(Name);
				if (It == Info->JointOffsets.end())
				{
					if (Top != nullptr)
					{
						auto& Global = Info->JointOffsets[Name];
						Global.Index = Info->GlobalIndex++;
						Global.Linking = true;

						It = Info->JointOffsets.find(Name);
						goto AddLinkingJoint;
					}

					for (uint32_t i = 0; i < Node->mNumChildren; i++)
					{
						auto& Next = Node->mChildren[i];
						if (FillSceneSkeleton(Info, Next, Top))
							return true;
					}

					return false;
				}

				if (Top != nullptr)
				{
				AddLinkingJoint:
					Top->Childs.emplace_back();
					auto& Next = Top->Childs.back();
					Top = &Next;
				}
				else
					Top = &Info->Skeleton;

				Top->Global = FromAssimpMatrix(Node->mTransformation);
				Top->Local = It->second.Local;
				Top->Index = It->second.Index;
				Top->Name = Name;

				for (uint32_t i = 0; i < Node->mNumChildren; i++)
				{
					auto& Next = Node->mChildren[i];
					FillSceneSkeleton(Info, Next, Top);
				}

				return true;
			}
			void FillSceneGeometry(ModelInfo* Info, MeshBlob* Blob, aiMesh* Mesh)
			{
				Blob->Vertices.reserve((size_t)Mesh->mNumVertices);
				for (uint32_t v = 0; v < Mesh->mNumVertices; v++)
				{
					auto& Vertex = Mesh->mVertices[v];
					Trigonometry::SkinVertex Next = { Vertex.x, Vertex.y, Vertex.z, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -1, -1, -1, -1, 0, 0, 0, 0 };
					UpdateSceneBounds(Info, Next);

					if (Mesh->HasNormals())
					{
						auto& Normal = Mesh->mNormals[v];
						Next.NormalX = Normal.x;
						Next.NormalY = Normal.y;
						Next.NormalZ = Normal.z;
					}

					if (Mesh->HasTextureCoords(0))
					{
						auto& TexCoord = Mesh->mTextureCoords[0][v];
						Next.TexCoordX = TexCoord.x;
						Next.TexCoordY = -TexCoord.y;
					}

					if (Mesh->HasTangentsAndBitangents())
					{
						auto& Tangent = Mesh->mTangents[v];
						Next.TangentX = Tangent.x;
						Next.TangentY = Tangent.y;
						Next.TangentZ = Tangent.z;

						auto& Bitangent = Mesh->mBitangents[v];
						Next.BitangentX = Bitangent.x;
						Next.BitangentY = Bitangent.y;
						Next.BitangentZ = Bitangent.z;
					}

					Blob->Vertices.push_back(Next);
				}

				for (uint32_t f = 0; f < Mesh->mNumFaces; f++)
				{
					auto* Face = &Mesh->mFaces[f];
					Blob->Indices.reserve(Blob->Indices.size() + (size_t)Face->mNumIndices);
					for (uint32_t i = 0; i < Face->mNumIndices; i++)
						Blob->Indices.push_back(Face->mIndices[i]);
				}
			}
			void FillSceneJoints(ModelInfo* Info, MeshBlob* Blob, aiMesh* Mesh)
			{
				for (uint32_t i = 0; i < Mesh->mNumBones; i++)
				{
					auto& Bone = Mesh->mBones[i];
					auto Name = GetJointName(Bone->mName.C_Str(), Info, Blob);

					auto Global = Info->JointOffsets.find(Name);
					if (Global == Info->JointOffsets.end())
					{
						auto& Next = Info->JointOffsets[Name];
						Next.Local = FromAssimpMatrix(Bone->mOffsetMatrix);
						Next.Index = Info->GlobalIndex++;
						Next.Linking = false;
						Global = Info->JointOffsets.find(Name);
					}

					auto& Local = Blob->JointIndices[Global->second.Index];
					Local = Blob->LocalIndex++;

					for (uint32_t j = 0; j < Bone->mNumWeights; j++)
					{
						auto& Weight = Bone->mWeights[j];
						auto& Vertex = Blob->Vertices[Weight.mVertexId];
						if (Vertex.JointIndex0 == -1.0f)
						{
							Vertex.JointIndex0 = (float)Local;
							Vertex.JointBias0 = Weight.mWeight;
						}
						else if (Vertex.JointIndex1 == -1.0f)
						{
							Vertex.JointIndex1 = (float)Local;
							Vertex.JointBias1 = Weight.mWeight;
						}
						else if (Vertex.JointIndex2 == -1.0f)
						{
							Vertex.JointIndex2 = (float)Local;
							Vertex.JointBias2 = Weight.mWeight;
						}
						else if (Vertex.JointIndex3 == -1.0f)
						{
							Vertex.JointIndex3 = (float)Local;
							Vertex.JointBias3 = Weight.mWeight;
						}
					}
				}
			}
			void FillSceneGeometries(ModelInfo* Info, const aiScene* Scene, aiNode* Node, const aiMatrix4x4& ParentTransform)
			{
				Info->Meshes.reserve(Info->Meshes.size() + (size_t)Node->mNumMeshes);
				if (Node == Scene->mRootNode)
					Info->Transform = FromAssimpMatrix(Scene->mRootNode->mTransformation).Inv();

				for (uint32_t n = 0; n < Node->mNumMeshes; n++)
				{
					Info->Meshes.emplace_back();
					MeshBlob& Blob = Info->Meshes.back();
					auto& Geometry = Scene->mMeshes[Node->mMeshes[n]];
					Blob.Name = GetMeshName(Geometry->mName.C_Str(), Info);
					Blob.Transform = FromAssimpMatrix(ParentTransform);

					FillSceneGeometry(Info, &Blob, Geometry);
					FillSceneJoints(Info, &Blob, Geometry);
					UpdateSceneBounds(Info);
				}

				for (uint32_t n = 0; n < Node->mNumChildren; n++)
				{
					auto& Next = Node->mChildren[n];
					FillSceneGeometries(Info, Scene, Next, ParentTransform);
				}
			}
			void FillSceneSkeletons(ModelInfo* Info, const aiScene* Scene)
			{
				FillSceneSkeleton(Info, Scene->mRootNode, nullptr);
				for (auto& Blob : Info->Meshes)
				{
					for (auto& Vertex : Blob.Vertices)
					{
						float Weight = 0.0f;
						if (Vertex.JointBias0 > 0.0f)
							Weight += Vertex.JointBias0;
						if (Vertex.JointBias1 > 0.0f)
							Weight += Vertex.JointBias1;
						if (Vertex.JointBias2 > 0.0f)
							Weight += Vertex.JointBias2;
						if (Vertex.JointBias3 > 0.0f)
							Weight += Vertex.JointBias3;

						if (!Weight)
							continue;

						if (Vertex.JointBias0 > 0.0f)
							Vertex.JointBias0 /= Weight;
						if (Vertex.JointBias1 > 0.0f)
							Vertex.JointBias1 /= Weight;
						if (Vertex.JointBias2 > 0.0f)
							Vertex.JointBias2 /= Weight;
						if (Vertex.JointBias3 > 0.0f)
							Vertex.JointBias3 /= Weight;
					}
				}
			}
			void FillSceneChannel(aiNodeAnim* Channel, ModelChannel& Target)
			{
				Target.Positions.reserve((size_t)Channel->mNumPositionKeys);
				for (uint32_t k = 0; k < Channel->mNumPositionKeys; k++)
				{
					aiVectorKey& Key = Channel->mPositionKeys[k];
					Target.Positions[Key.mTime] = Trigonometry::Vector3(Key.mValue.x, Key.mValue.y, Key.mValue.z);
				}

				Target.Scales.reserve((size_t)Channel->mNumScalingKeys);
				for (uint32_t k = 0; k < Channel->mNumScalingKeys; k++)
				{
					aiVectorKey& Key = Channel->mScalingKeys[k];
					Target.Scales[Key.mTime] = Trigonometry::Vector3(Key.mValue.x, Key.mValue.y, Key.mValue.z);
				}

				Target.Rotations.reserve((size_t)Channel->mNumRotationKeys);
				for (uint32_t k = 0; k < Channel->mNumRotationKeys; k++)
				{
					aiQuatKey& Key = Channel->mRotationKeys[k];
					Target.Rotations[Key.mTime] = Trigonometry::Quaternion(Key.mValue.x, Key.mValue.y, Key.mValue.z, Key.mValue.w);
				}
			}
			void FillSceneTimeline(const Core::UnorderedSet<float>& Timings, Core::Vector<float>& Timeline)
			{
				Timeline.reserve(Timings.size());
				for (auto& Time : Timings)
					Timeline.push_back(Time);

				VI_SORT(Timeline.begin(), Timeline.end(), [](float A, float B)
				{
					return A < B;
				});
			}
			void FillSceneKeys(ModelChannel& Info, Core::Vector<Trigonometry::AnimatorKey>& Keys)
			{
				Core::UnorderedSet<float> Timings;
				Timings.reserve(Keys.size());

				float FirstPosition = std::numeric_limits<float>::max();
				for (auto& Item : Info.Positions)
				{
					Timings.insert(Item.first);
					if (Item.first < FirstPosition)
						FirstPosition = Item.first;
				}

				float FirstScale = std::numeric_limits<float>::max();
				for (auto& Item : Info.Scales)
				{
					Timings.insert(Item.first);
					if (Item.first < FirstScale)
						FirstScale = Item.first;
				}

				float FirstRotation = std::numeric_limits<float>::max();
				for (auto& Item : Info.Rotations)
				{
					Timings.insert(Item.first);
					if (Item.first < FirstRotation)
						FirstRotation = Item.first;
				}

				Core::Vector<float> Timeline;
				Trigonometry::Vector3 LastPosition = (Info.Positions.empty() ? Trigonometry::Vector3::Zero() : Info.Positions[FirstPosition]);
				Trigonometry::Vector3 LastScale = (Info.Scales.empty() ? Trigonometry::Vector3::One() : Info.Scales[FirstScale]);
				Trigonometry::Quaternion LastRotation = (Info.Rotations.empty() ? Trigonometry::Quaternion() : Info.Rotations[FirstRotation]);
				FillSceneTimeline(Timings, Timeline);
				Keys.resize(Timings.size());

				size_t Index = 0;
				for (auto& Time : Timeline)
				{
					auto& Target = Keys[Index++];
					Target.Position = LastPosition;
					Target.Scale = LastScale;
					Target.Rotation = LastRotation;
					Target.Time = Time;

					auto Position = Info.Positions.find(Time);
					if (Position != Info.Positions.end())
					{
						Target.Position = Position->second;
						LastPosition = Target.Position;
					}

					auto Scale = Info.Scales.find(Time);
					if (Scale != Info.Scales.end())
					{
						Target.Scale = Scale->second;
						LastScale = Target.Scale;
					}

					auto Rotation = Info.Rotations.find(Time);
					if (Rotation != Info.Rotations.end())
					{
						Target.Rotation = Rotation->second;
						LastRotation = Target.Rotation;
					}
				}
			}
			void FillSceneClip(Trigonometry::SkinAnimatorClip& Clip, Core::UnorderedMap<Core::String, MeshBone>& Indices, Core::UnorderedMap<Core::String, Core::Vector<Trigonometry::AnimatorKey>>& Channels)
			{
				Core::UnorderedSet<float> Timings;
				for (auto& Channel : Channels)
				{
					Timings.reserve(Channel.second.size());
					for (auto& Key : Channel.second)
						Timings.insert(Key.Time);
				}

				Core::Vector<float> Timeline;
				FillSceneTimeline(Timings, Timeline);

				for (auto& Time : Timeline)
				{
					Clip.Keys.emplace_back();
					auto& Key = Clip.Keys.back();
					Key.Pose.resize(Indices.size());
					Key.Time = Time;

					for (auto& Index : Indices)
					{
						auto& Pose = Key.Pose[Index.second.Index];
						Pose.Position = Index.second.Default.Position;
						Pose.Scale = Index.second.Default.Scale;
						Pose.Rotation = Index.second.Default.Rotation;
						Pose.Time = Time;
					}

					for (auto& Channel : Channels)
					{
						auto Index = Indices.find(Channel.first);
						if (Index == Indices.end())
							continue;

						auto& Next = Key.Pose[Index->second.Index];
						if (GetKeyFromTime(Channel.second, Time, Next))
							Next.Time = Time;
					}
				}
			}
			void FillSceneJointIndices(const aiScene* Scene, aiNode* Node, Core::UnorderedMap<Core::String, MeshBone>& Indices, size_t& Index)
			{
				for (uint32_t n = 0; n < Node->mNumMeshes; n++)
				{
					auto& Mesh = Scene->mMeshes[Node->mMeshes[n]];
					for (uint32_t i = 0; i < Mesh->mNumBones; i++)
					{
						auto& Bone = Mesh->mBones[i];
						auto Joint = Indices.find(Bone->mName.C_Str());
						if (Joint == Indices.end())
							Indices[Bone->mName.C_Str()].Index = Index++;
					}
				}

				for (uint32_t n = 0; n < Node->mNumChildren; n++)
				{
					auto& Next = Node->mChildren[n];
					FillSceneJointIndices(Scene, Next, Indices, Index);
				}
			}
			bool FillSceneJointDefaults(aiNode* Node, Core::UnorderedMap<Core::String, MeshBone>& Indices, size_t& Index, bool InSkeleton)
			{
				Core::String Name = Node->mName.C_Str();
				auto It = Indices.find(Name);
				if (It == Indices.end())
				{
					if (InSkeleton)
					{
						auto& Joint = Indices[Name];
						Joint.Index = Index++;
						It = Indices.find(Name);
						goto AddLinkingJoint;
					}

					for (uint32_t i = 0; i < Node->mNumChildren; i++)
					{
						auto& Next = Node->mChildren[i];
						if (FillSceneJointDefaults(Next, Indices, Index, InSkeleton))
							return true;
					}

					return false;
				}

			AddLinkingJoint:
				auto Offset = FromAssimpMatrix(Node->mTransformation);
				It->second.Default.Position = Offset.Position();
				It->second.Default.Scale = Offset.Scale();
				It->second.Default.Rotation = Offset.RotationQuaternion();

				for (uint32_t i = 0; i < Node->mNumChildren; i++)
				{
					auto& Next = Node->mChildren[i];
					FillSceneJointDefaults(Next, Indices, Index, true);
				}

				return true;
			}
			void FillSceneAnimations(Core::Vector<Trigonometry::SkinAnimatorClip>* Info, const aiScene* Scene)
			{
				Core::UnorderedMap<Core::String, MeshBone> Indices; size_t Index = 0;
				FillSceneJointIndices(Scene, Scene->mRootNode, Indices, Index);
				FillSceneJointDefaults(Scene->mRootNode, Indices, Index, false);

				Info->reserve((size_t)Scene->mNumAnimations);
				for (uint32_t i = 0; i < Scene->mNumAnimations; i++)
				{
					aiAnimation* Animation = Scene->mAnimations[i];
					Info->emplace_back();

					auto& Clip = Info->back();
					Clip.Name = Animation->mName.C_Str();
					Clip.Duration = (float)Animation->mDuration;
					Clip.Rate = Compute::Mathf::Max(0.01f, (float)Animation->mTicksPerSecond);

					Core::UnorderedMap<Core::String, Core::Vector<Trigonometry::AnimatorKey>> Channels;
					for (uint32_t j = 0; j < Animation->mNumChannels; j++)
					{
						auto& Channel = Animation->mChannels[j];
						auto& Frames = Channels[Channel->mNodeName.C_Str()];

						ModelChannel Target;
						FillSceneChannel(Channel, Target);
						FillSceneKeys(Target, Frames);
					}
					FillSceneClip(Clip, Indices, Channels);
				}
			}
#endif
			Core::Vector<Trigonometry::Vertex> SkinVerticesToVertices(const Core::Vector<Trigonometry::SkinVertex>& Data)
			{
				Core::Vector<Trigonometry::Vertex> Result;
				Result.resize(Data.size());

				for (size_t i = 0; i < Data.size(); i++)
				{
					auto& From = Data[i];
					auto& To = Result[i];
					To.PositionX = From.PositionX;
					To.PositionY = From.PositionY;
					To.PositionZ = From.PositionZ;
					To.TexCoordX = From.TexCoordX;
					To.TexCoordY = From.TexCoordY;
					To.NormalX = From.NormalX;
					To.NormalY = From.NormalY;
					To.NormalZ = From.NormalZ;
					To.TangentX = From.TangentX;
					To.TangentY = From.TangentY;
					To.TangentZ = From.TangentZ;
					To.BitangentX = From.BitangentX;
					To.BitangentY = From.BitangentY;
					To.BitangentZ = From.BitangentZ;
				}

				return Result;
			}
			template <typename T>
			T ProcessRendererJob(Graphics::GraphicsDevice* Device, std::function<T(Graphics::GraphicsDevice*)>&& Callback)
			{
				Core::Promise<T> Future;
				Graphics::RenderThreadCallback Job = [Future, Callback = std::move(Callback)](Graphics::GraphicsDevice* Device) mutable
				{
					Future.Set(Callback(Device));
				};

				auto* App = HeavyApplication::HasInstance() ? HeavyApplication::Get() : nullptr;
				if (!App || App->GetState() != ApplicationState::Active || Device != App->Renderer)
					Device->Lockup(std::move(Job));
				else
					Device->Enqueue(std::move(Job));

				return Future.Get();
			}

			MaterialProcessor::MaterialProcessor(ContentManager* Manager) : Processor(Manager)
			{
			}
			ExpectsContent<void*> MaterialProcessor::Duplicate(AssetCache* Asset, const Core::VariantArgs& Args)
			{
				VI_ASSERT(Asset != nullptr, "asset should be set");
				VI_ASSERT(Asset->Resource != nullptr, "instance should be set");

				((Layer::Material*)Asset->Resource)->AddRef();
				return Asset->Resource;
			}
			ExpectsContent<void*> MaterialProcessor::Deserialize(Core::Stream* Stream, size_t Offset, const Core::VariantArgs& Args)
			{
				VI_ASSERT(Stream != nullptr, "stream should be set");
				auto DataStatus = Content->Load<Core::Schema>(Stream->VirtualName());
				if (!DataStatus)
					return DataStatus.Error();

				Core::String Path;
				Core::UPtr<Core::Schema> Data = *DataStatus;
				Core::UPtr<Layer::Material> Object = new Layer::Material(nullptr);
				if (Series::Unpack(Data->Get("diffuse-map"), &Path) && !Path.empty())
				{
					auto NewTexture = Content->Load<Graphics::Texture2D>(Path);
					if (!NewTexture)
						return NewTexture.Error();

					Object->SetDiffuseMap(*NewTexture);
					Core::Memory::Release(*NewTexture);
				}

				if (Series::Unpack(Data->Get("normal-map"), &Path) && !Path.empty())
				{
					auto NewTexture = Content->Load<Graphics::Texture2D>(Path);
					if (!NewTexture)
						return NewTexture.Error();

					Object->SetNormalMap(*NewTexture);
					Core::Memory::Release(*NewTexture);
				}

				if (Series::Unpack(Data->Get("metallic-map"), &Path) && !Path.empty())
				{
					auto NewTexture = Content->Load<Graphics::Texture2D>(Path);
					if (!NewTexture)
						return NewTexture.Error();

					Object->SetMetallicMap(*NewTexture);
					Core::Memory::Release(*NewTexture);
				}

				if (Series::Unpack(Data->Get("roughness-map"), &Path) && !Path.empty())
				{
					auto NewTexture = Content->Load<Graphics::Texture2D>(Path);
					if (!NewTexture)
						return NewTexture.Error();

					Object->SetRoughnessMap(*NewTexture);
					Core::Memory::Release(*NewTexture);
				}

				if (Series::Unpack(Data->Get("height-map"), &Path) && !Path.empty())
				{
					auto NewTexture = Content->Load<Graphics::Texture2D>(Path);
					if (!NewTexture)
						return NewTexture.Error();

					Object->SetHeightMap(*NewTexture);
					Core::Memory::Release(*NewTexture);
				}

				if (Series::Unpack(Data->Get("occlusion-map"), &Path) && !Path.empty())
				{
					auto NewTexture = Content->Load<Graphics::Texture2D>(Path);
					if (!NewTexture)
						return NewTexture.Error();

					Object->SetOcclusionMap(*NewTexture);
					Core::Memory::Release(*NewTexture);
				}

				if (Series::Unpack(Data->Get("emission-map"), &Path) && !Path.empty())
				{
					auto NewTexture = Content->Load<Graphics::Texture2D>(Path);
					if (!NewTexture)
						return NewTexture.Error();

					Object->SetEmissionMap(*NewTexture);
					Core::Memory::Release(*NewTexture);
				}

				Core::String Name;
				HeavySeries::Unpack(Data->Get("emission"), &Object->Surface.Emission);
				HeavySeries::Unpack(Data->Get("metallic"), &Object->Surface.Metallic);
				HeavySeries::Unpack(Data->Get("penetration"), &Object->Surface.Penetration);
				HeavySeries::Unpack(Data->Get("diffuse"), &Object->Surface.Diffuse);
				HeavySeries::Unpack(Data->Get("scattering"), &Object->Surface.Scattering);
				HeavySeries::Unpack(Data->Get("roughness"), &Object->Surface.Roughness);
				HeavySeries::Unpack(Data->Get("occlusion"), &Object->Surface.Occlusion);
				Series::Unpack(Data->Get("fresnel"), &Object->Surface.Fresnel);
				Series::Unpack(Data->Get("refraction"), &Object->Surface.Refraction);
				Series::Unpack(Data->Get("transparency"), &Object->Surface.Transparency);
				Series::Unpack(Data->Get("environment"), &Object->Surface.Environment);
				Series::Unpack(Data->Get("radius"), &Object->Surface.Radius);
				Series::Unpack(Data->Get("height"), &Object->Surface.Height);
				Series::Unpack(Data->Get("bias"), &Object->Surface.Bias);
				Series::Unpack(Data->Get("name"), &Name);
				Object->SetName(Name);

				auto* Existing = (Layer::Material*)Content->TryToCache(this, Stream->VirtualName(), *Object);
				if (Existing != nullptr)
					Object = Existing;

				Object->AddRef();
				return Object.Reset();
			}
			ExpectsContent<void> MaterialProcessor::Serialize(Core::Stream* Stream, void* Instance, const Core::VariantArgs& Args)
			{
				VI_ASSERT(Stream != nullptr, "stream should be set");
				VI_ASSERT(Instance != nullptr, "instance should be set");

				Layer::Material* Object = (Layer::Material*)Instance;
				Core::UPtr<Core::Schema> Data = Core::Var::Set::Object();
				Data->Key = "material";

				AssetCache* Asset = Content->FindCache<Graphics::Texture2D>(Object->GetDiffuseMap());
				if (Asset != nullptr)
					Series::Pack(Data->Set("diffuse-map"), Asset->Path);

				Asset = Content->FindCache<Graphics::Texture2D>(Object->GetNormalMap());
				if (Asset != nullptr)
					Series::Pack(Data->Set("normal-map"), Asset->Path);

				Asset = Content->FindCache<Graphics::Texture2D>(Object->GetMetallicMap());
				if (Asset != nullptr)
					Series::Pack(Data->Set("metallic-map"), Asset->Path);

				Asset = Content->FindCache<Graphics::Texture2D>(Object->GetRoughnessMap());
				if (Asset != nullptr)
					Series::Pack(Data->Set("roughness-map"), Asset->Path);

				Asset = Content->FindCache<Graphics::Texture2D>(Object->GetHeightMap());
				if (Asset != nullptr)
					Series::Pack(Data->Set("height-map"), Asset->Path);

				Asset = Content->FindCache<Graphics::Texture2D>(Object->GetOcclusionMap());
				if (Asset != nullptr)
					Series::Pack(Data->Set("occlusion-map"), Asset->Path);

				Asset = Content->FindCache<Graphics::Texture2D>(Object->GetEmissionMap());
				if (Asset != nullptr)
					Series::Pack(Data->Set("emission-map"), Asset->Path);

				HeavySeries::Pack(Data->Set("emission"), Object->Surface.Emission);
				HeavySeries::Pack(Data->Set("metallic"), Object->Surface.Metallic);
				HeavySeries::Pack(Data->Set("penetration"), Object->Surface.Penetration);
				HeavySeries::Pack(Data->Set("diffuse"), Object->Surface.Diffuse);
				HeavySeries::Pack(Data->Set("scattering"), Object->Surface.Scattering);
				HeavySeries::Pack(Data->Set("roughness"), Object->Surface.Roughness);
				HeavySeries::Pack(Data->Set("occlusion"), Object->Surface.Occlusion);
				Series::Pack(Data->Set("fresnel"), Object->Surface.Fresnel);
				Series::Pack(Data->Set("refraction"), Object->Surface.Refraction);
				Series::Pack(Data->Set("transparency"), Object->Surface.Transparency);
				Series::Pack(Data->Set("environment"), Object->Surface.Environment);
				Series::Pack(Data->Set("radius"), Object->Surface.Radius);
				Series::Pack(Data->Set("height"), Object->Surface.Height);
				Series::Pack(Data->Set("bias"), Object->Surface.Bias);
				Series::Pack(Data->Set("name"), Object->GetName());
				return Content->Save<Core::Schema>(Stream->VirtualName(), *Data, Args);
			}
			void MaterialProcessor::Free(AssetCache* Asset)
			{
				VI_ASSERT(Asset != nullptr, "asset should be set");
				auto* Value = (Layer::Material*)Asset->Resource;
				Asset->Resource = nullptr;
				Core::Memory::Release(Value);
			}

			SceneGraphProcessor::SceneGraphProcessor(ContentManager* Manager) : Processor(Manager)
			{
			}
			ExpectsContent<void*> SceneGraphProcessor::Deserialize(Core::Stream* Stream, size_t Offset, const Core::VariantArgs& Args)
			{
				Layer::SceneGraph::Desc I = Layer::SceneGraph::Desc::Get(HeavyApplication::HasInstance() ? HeavyApplication::Get() : nullptr);
				VI_ASSERT(Stream != nullptr, "stream should be set");
				VI_ASSERT(I.Shared.Device != nullptr, "graphics device should be set");

				auto BlobStatus = Content->Load<Core::Schema>(Stream->VirtualName());
				if (!BlobStatus)
					return BlobStatus.Error();

				Core::UPtr<Core::Schema> Blob = *BlobStatus;
				Core::Schema* Metadata = Blob->Find("metadata");
				if (Metadata != nullptr)
				{
					Core::Schema* Simulator = Metadata->Find("simulator");
					if (Simulator != nullptr)
					{
						Series::Unpack(Simulator->Find("enable-soft-body"), &I.Simulator.EnableSoftBody);
						Series::Unpack(Simulator->Find("max-displacement"), &I.Simulator.MaxDisplacement);
						Series::Unpack(Simulator->Find("air-density"), &I.Simulator.AirDensity);
						Series::Unpack(Simulator->Find("water-offset"), &I.Simulator.WaterOffset);
						Series::Unpack(Simulator->Find("water-density"), &I.Simulator.WaterDensity);
						HeavySeries::Unpack(Simulator->Find("water-normal"), &I.Simulator.WaterNormal);
						HeavySeries::Unpack(Simulator->Find("gravity"), &I.Simulator.Gravity);
					}

					Series::Unpack(Metadata->Find("materials"), &I.StartMaterials);
					Series::Unpack(Metadata->Find("entities"), &I.StartEntities);
					Series::Unpack(Metadata->Find("components"), &I.StartComponents);
					Series::Unpack(Metadata->Find("render-quality"), &I.RenderQuality);
					Series::Unpack(Metadata->Find("enable-hdr"), &I.EnableHDR);
					Series::Unpack(Metadata->Find("grow-margin"), &I.GrowMargin);
					Series::Unpack(Metadata->Find("grow-rate"), &I.GrowRate);
					Series::Unpack(Metadata->Find("max-updates"), &I.MaxUpdates);
					Series::Unpack(Metadata->Find("voxels-size"), &I.VoxelsSize);
					Series::Unpack(Metadata->Find("voxels-max"), &I.VoxelsMax);
					Series::Unpack(Metadata->Find("points-size"), &I.PointsSize);
					Series::Unpack(Metadata->Find("points-max"), &I.PointsMax);
					Series::Unpack(Metadata->Find("spots-size"), &I.SpotsSize);
					Series::Unpack(Metadata->Find("spots-max"), &I.SpotsMax);
					Series::Unpack(Metadata->Find("line-size"), &I.LinesSize);
					Series::Unpack(Metadata->Find("lines-max"), &I.LinesMax);
				}

				bool IntegrityCheck = false;
				auto EnsureIntegrity = Args.find("integrity");
				if (EnsureIntegrity != Args.end())
					IntegrityCheck = EnsureIntegrity->second.GetBoolean();

				auto HasMutations = Args.find("mutations");
				if (HasMutations != Args.end())
					I.Mutations = HasMutations->second.GetBoolean();

				Layer::SceneGraph* Object = new Layer::SceneGraph(I);
				Layer::IdxSnapshot Snapshot;
				Object->Snapshot = &Snapshot;
				if (SetupCallback)
					SetupCallback(Object);

				auto IsActive = Args.find("active");
				if (IsActive != Args.end())
					Object->SetActive(IsActive->second.GetBoolean());

				Core::Schema* Materials = Blob->Find("materials");
				if (Materials != nullptr)
				{
					Core::Vector<Core::Schema*> Collection = Materials->FindCollection("material");
					for (auto& It : Collection)
					{
						Core::String Path;
						if (!Series::Unpack(It, &Path) || Path.empty())
							continue;

						auto Value = Content->Load<Layer::Material>(Path);
						if (Value)
						{
							Series::Unpack(It, &Value->Slot);
							Object->AddMaterial(*Value);
						}
						else if (IntegrityCheck)
							return Value.Error();
					}
				}

				Core::Schema* Entities = Blob->Find("entities");
				if (Entities != nullptr)
				{
					Core::Vector<Core::Schema*> Collection = Entities->FindCollection("entity");
					for (auto& It : Collection)
					{
						Entity* Entity = Object->AddEntity();
						int64_t Refer = -1;

						if (Series::Unpack(It->Find("refer"), &Refer) && Refer >= 0)
						{
							Snapshot.To[Entity] = (size_t)Refer;
							Snapshot.From[(size_t)Refer] = Entity;
						}
					}

					size_t Next = 0;
					for (auto& It : Collection)
					{
						Entity* Entity = Object->GetEntity(Next++);
						if (!Entity)
							continue;

						Core::String Name;
						Series::Unpack(It->Find("name"), &Name);
						Entity->SetName(Name);

						Core::Schema* Transform = It->Find("transform");
						if (Transform != nullptr)
						{
							Trigonometry::Transform* Offset = Entity->GetTransform();
							Trigonometry::Transform::Spacing& Space = Offset->GetSpacing(Trigonometry::Positioning::Global);
							bool Scaling = Offset->HasScaling();
							HeavySeries::Unpack(Transform->Find("position"), &Space.Position);
							HeavySeries::Unpack(Transform->Find("rotation"), &Space.Rotation);
							HeavySeries::Unpack(Transform->Find("scale"), &Space.Scale);
							Series::Unpack(Transform->Find("scaling"), &Scaling);
							Offset->SetScaling(Scaling);
						}

						Core::Schema* Parent = It->Find("parent");
						if (Parent != nullptr)
						{
							Trigonometry::Transform* Root = nullptr;
							Trigonometry::Transform::Spacing* Space = Core::Memory::New<Trigonometry::Transform::Spacing>();
							HeavySeries::Unpack(Parent->Find("position"), &Space->Position);
							HeavySeries::Unpack(Parent->Find("rotation"), &Space->Rotation);
							HeavySeries::Unpack(Parent->Find("scale"), &Space->Scale);
							HeavySeries::Unpack(Parent->Find("world"), &Space->Offset);

							size_t Where = 0;
							if (Series::Unpack(Parent->Find("where"), &Where))
							{
								auto It = Snapshot.From.find(Where);
								if (It != Snapshot.From.end() && It->second != Entity)
									Root = It->second->GetTransform();
							}

							Trigonometry::Transform* Offset = Entity->GetTransform();
							Offset->SetPivot(Root, Space);
							Offset->MakeDirty();
						}

						Core::Schema* Components = It->Find("components");
						if (Components != nullptr)
						{
							Core::Vector<Core::Schema*> Elements = Components->FindCollection("component");
							for (auto& Element : Elements)
							{
								uint64_t Id;
								if (!Series::Unpack(Element->Find("id"), &Id))
									continue;

								Component* Target = Core::Composer::Create<Component>(Id, Entity);
								if (!Entity->AddComponent(Target))
									continue;

								bool Active = true;
								if (Series::Unpack(Element->Find("active"), &Active))
									Target->SetActive(Active);

								Core::Schema* Meta = Element->Find("metadata");
								if (!Meta)
									Meta = Element->Set("metadata");
								Target->Deserialize(Meta);
							}
						}
					}
				}

				Object->Snapshot = nullptr;
				Object->Actualize();
				return Object;
			}
			ExpectsContent<void> SceneGraphProcessor::Serialize(Core::Stream* Stream, void* Instance, const Core::VariantArgs& Args)
			{
				VI_ASSERT(Stream != nullptr, "stream should be set");
				VI_ASSERT(Instance != nullptr, "instance should be set");

				auto Ext = Core::OS::Path::GetExtension(Stream->VirtualName());
				if (Ext.empty())
				{
					auto Type = Args.find("type");
					if (Type->second == Core::Var::String("XML"))
						Ext = ".xml";
					else if (Type->second == Core::Var::String("JSON"))
						Ext = ".json";
					else if (Type->second == Core::Var::String("JSONB"))
						Ext = ".jsonb";
					else
						Ext = ".xml";
				}

				Layer::SceneGraph* Object = (Layer::SceneGraph*)Instance;
				if (SetupCallback)
					SetupCallback(Object);
				Object->Actualize();

				Layer::IdxSnapshot Snapshot;
				Object->MakeSnapshot(&Snapshot);
				Object->Snapshot = &Snapshot;

				Core::UPtr<Core::Schema> Blob = Core::Var::Set::Object();
				Blob->Key = "scene";

				auto& Conf = Object->GetConf();
				Core::Schema* Metadata = Blob->Set("metadata");
				Series::Pack(Metadata->Set("materials"), Conf.StartMaterials);
				Series::Pack(Metadata->Set("entities"), Conf.StartEntities);
				Series::Pack(Metadata->Set("components"), Conf.StartComponents);
				Series::Pack(Metadata->Set("render-quality"), Conf.RenderQuality);
				Series::Pack(Metadata->Set("enable-hdr"), Conf.EnableHDR);
				Series::Pack(Metadata->Set("grow-margin"), Conf.GrowMargin);
				Series::Pack(Metadata->Set("grow-rate"), Conf.GrowRate);
				Series::Pack(Metadata->Set("max-updates"), Conf.MaxUpdates);
				Series::Pack(Metadata->Set("voxels-size"), Conf.VoxelsSize);
				Series::Pack(Metadata->Set("voxels-max"), Conf.VoxelsMax);
				Series::Pack(Metadata->Set("points-size"), Conf.PointsSize);
				Series::Pack(Metadata->Set("points-max"), Conf.PointsMax);
				Series::Pack(Metadata->Set("spots-size"), Conf.SpotsSize);
				Series::Pack(Metadata->Set("spots-max"), Conf.SpotsMax);
				Series::Pack(Metadata->Set("line-size"), Conf.LinesSize);
				Series::Pack(Metadata->Set("lines-max"), Conf.LinesMax);

				auto* fSimulator = Object->GetSimulator();
				Core::Schema* Simulator = Metadata->Set("simulator");
				Series::Pack(Simulator->Set("enable-soft-body"), fSimulator->HasSoftBodySupport());
				Series::Pack(Simulator->Set("max-displacement"), fSimulator->GetMaxDisplacement());
				Series::Pack(Simulator->Set("air-density"), fSimulator->GetAirDensity());
				Series::Pack(Simulator->Set("water-offset"), fSimulator->GetWaterOffset());
				Series::Pack(Simulator->Set("water-density"), fSimulator->GetWaterDensity());
				HeavySeries::Pack(Simulator->Set("water-normal"), fSimulator->GetWaterNormal());
				HeavySeries::Pack(Simulator->Set("gravity"), fSimulator->GetGravity());

				Core::Schema* Materials = Blob->Set("materials", Core::Var::Array());
				for (size_t i = 0; i < Object->GetMaterialsCount(); i++)
				{
					Layer::Material* Material = Object->GetMaterial(i);
					if (!Material || Material == Object->GetInvalidMaterial())
						continue;

					Core::String Path;
					AssetCache* Asset = Content->FindCache<Layer::Material>(Material);
					if (!Asset)
						Path.assign("./materials/" + Material->GetName() + ".modified");
					else
						Path.assign(Asset->Path);

					if (!Core::Stringify::EndsWith(Path, Ext))
						Path.append(Ext);

					if (Content->Save<Layer::Material>(Path, Material, Args))
					{
						Core::Schema* Where = Materials->Set("material");
						Series::Pack(Where, Material->Slot);
						Series::Pack(Where, Path);
					}
				}

				Core::Schema* Entities = Blob->Set("entities", Core::Var::Array());
				for (size_t i = 0; i < Object->GetEntitiesCount(); i++)
				{
					Entity* Ref = Object->GetEntity(i);
					auto* Offset = Ref->GetTransform();

					Core::Schema* Entity = Entities->Set("entity");
					Series::Pack(Entity->Set("name"), Ref->GetName());
					Series::Pack(Entity->Set("refer"), i);

					Core::Schema* Transform = Entity->Set("transform");
					HeavySeries::Pack(Transform->Set("position"), Offset->GetPosition());
					HeavySeries::Pack(Transform->Set("rotation"), Offset->GetRotation());
					HeavySeries::Pack(Transform->Set("scale"), Offset->GetScale());
					Series::Pack(Transform->Set("scaling"), Offset->HasScaling());

					if (Offset->GetRoot() != nullptr)
					{
						Core::Schema* Parent = Entity->Set("parent");
						if (Offset->GetRoot()->UserData != nullptr)
						{
							auto It = Snapshot.To.find((Layer::Entity*)Offset->GetRoot());
							if (It != Snapshot.To.end())
								Series::Pack(Parent->Set("where"), It->second);
						}

						Trigonometry::Transform::Spacing& Space = Offset->GetSpacing();
						HeavySeries::Pack(Parent->Set("position"), Space.Position);
						HeavySeries::Pack(Parent->Set("rotation"), Space.Rotation);
						HeavySeries::Pack(Parent->Set("scale"), Space.Scale);
						HeavySeries::Pack(Parent->Set("world"), Space.Offset);
					}

					if (!Ref->GetComponentsCount())
						continue;

					Core::Schema* Components = Entity->Set("components", Core::Var::Array());
					for (auto& Item : *Ref)
					{
						Core::Schema* Component = Components->Set("component");
						Series::Pack(Component->Set("id"), Item.second->GetId());
						Series::Pack(Component->Set("active"), Item.second->IsActive());
						Item.second->Serialize(Component->Set("metadata"));
					}
				}

				Object->Snapshot = nullptr;
				return Content->Save<Core::Schema>(Stream->VirtualName(), *Blob, Args);
			}

			AudioClipProcessor::AudioClipProcessor(ContentManager* Manager) : Processor(Manager)
			{
			}
			AudioClipProcessor::~AudioClipProcessor()
			{
			}
			ExpectsContent<void*> AudioClipProcessor::Duplicate(AssetCache* Asset, const Core::VariantArgs& Args)
			{
				VI_ASSERT(Asset != nullptr, "asset should be set");
				VI_ASSERT(Asset->Resource != nullptr, "asset resource should be set");

				((Audio::AudioClip*)Asset->Resource)->AddRef();
				return Asset->Resource;
			}
			ExpectsContent<void*> AudioClipProcessor::Deserialize(Core::Stream* Stream, size_t Offset, const Core::VariantArgs& Args)
			{
				if (Core::Stringify::EndsWith(Stream->VirtualName(), ".wav"))
					return DeserializeWAVE(Stream, Offset, Args);
				else if (Core::Stringify::EndsWith(Stream->VirtualName(), ".ogg"))
					return DeserializeOGG(Stream, Offset, Args);

				return ContentException("deserialize audio unsupported: " + Core::String(Core::OS::Path::GetExtension(Stream->VirtualName())));
			}
			ExpectsContent<void*> AudioClipProcessor::DeserializeWAVE(Core::Stream* Stream, size_t Offset, const Core::VariantArgs& Args)
			{
				VI_ASSERT(Stream != nullptr, "stream should be set");
#ifdef VI_SDL2
				Core::Vector<char> Data;
				Stream->ReadAll([&Data](uint8_t* Buffer, size_t Size)
				{
					Data.reserve(Data.size() + Size);
					for (size_t i = 0; i < Size; i++)
						Data.push_back(Buffer[i]);
				});

				SDL_RWops* WavData = SDL_RWFromMem(Data.data(), (int)Data.size());
				SDL_AudioSpec WavInfo;
				Uint8* WavSamples;
				Uint32 WavCount;

				if (!SDL_LoadWAV_RW(WavData, 1, &WavInfo, &WavSamples, &WavCount))
				{
					SDL_RWclose(WavData);
					return ContentException(std::move(Graphics::VideoException().message()));
				}

				int Format = 0;
#ifdef VI_OPENAL
				switch (WavInfo.format)
				{
					case AUDIO_U8:
					case AUDIO_S8:
						Format = WavInfo.channels == 2 ? AL_FORMAT_STEREO8 : AL_FORMAT_MONO8;
						break;
					case AUDIO_U16:
					case AUDIO_S16:
						Format = WavInfo.channels == 2 ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16;
						break;
					default:
						SDL_FreeWAV(WavSamples);
						SDL_RWclose(WavData);
						return ContentException("load wave audio: unsupported audio format");
				}
#endif
				Core::UPtr<Audio::AudioClip> Object = new Audio::AudioClip(1, Format);
				Audio::AudioContext::SetBufferData(Object->GetBuffer(), (int)Format, (const void*)WavSamples, (int)WavCount, (int)WavInfo.freq);
				SDL_FreeWAV(WavSamples);
				SDL_RWclose(WavData);

				auto* Existing = (Audio::AudioClip*)Content->TryToCache(this, Stream->VirtualName(), *Object);
				if (Existing != nullptr)
					Object = Existing;

				Object->AddRef();
				return Object.Reset();
#else
				return ContentException("load wave audio: unsupported");
#endif
			}
			ExpectsContent<void*> AudioClipProcessor::DeserializeOGG(Core::Stream* Stream, size_t Offset, const Core::VariantArgs& Args)
			{
#ifdef VI_STB
				VI_ASSERT(Stream != nullptr, "stream should be set");
				Core::Vector<char> Data;
				Stream->ReadAll([&Data](uint8_t* Buffer, size_t Size)
				{
					Data.reserve(Data.size() + Size);
					for (size_t i = 0; i < Size; i++)
						Data.push_back(Buffer[i]);
				});

				short* Buffer;
				int Channels, SampleRate;
				int Samples = stb_vorbis_decode_memory((const uint8_t*)Data.data(), (int)Data.size(), &Channels, &SampleRate, &Buffer);
				if (Samples <= 0)
					return ContentException("load ogg audio: invalid file");

				int Format = 0;
#ifdef VI_OPENAL
				if (Channels == 2)
					Format = AL_FORMAT_STEREO16;
				else
					Format = AL_FORMAT_MONO16;
#endif
				Core::UPtr<Audio::AudioClip> Object = new Audio::AudioClip(1, Format);
				Audio::AudioContext::SetBufferData(Object->GetBuffer(), (int)Format, (const void*)Buffer, Samples * sizeof(short) * Channels, (int)SampleRate);
				Core::Memory::Deallocate(Buffer);

				auto* Existing = (Audio::AudioClip*)Content->TryToCache(this, Stream->VirtualName(), *Object);
				if (Existing != nullptr)
					Object = Existing;

				Object->AddRef();
				return Object.Reset();
#else
				return ContentException("load ogg audio: unsupported");
#endif
			}
			void AudioClipProcessor::Free(AssetCache* Asset)
			{
				VI_ASSERT(Asset != nullptr, "asset should be set");
				auto* Value = (Audio::AudioClip*)Asset->Resource;
				Core::Memory::Release(Value);
				Asset->Resource = nullptr;
			}

			Texture2DProcessor::Texture2DProcessor(ContentManager* Manager) : Processor(Manager)
			{
			}
			Texture2DProcessor::~Texture2DProcessor()
			{
			}
			ExpectsContent<void*> Texture2DProcessor::Duplicate(AssetCache* Asset, const Core::VariantArgs& Args)
			{
				VI_ASSERT(Asset != nullptr, "asset should be set");
				VI_ASSERT(Asset->Resource != nullptr, "instance should be set");

				((Graphics::Texture2D*)Asset->Resource)->AddRef();
				return Asset->Resource;
			}
			ExpectsContent<void*> Texture2DProcessor::Deserialize(Core::Stream* Stream, size_t Offset, const Core::VariantArgs& Args)
			{
#ifdef VI_STB
				VI_ASSERT(Stream != nullptr, "stream should be set");
				Core::Vector<char> Data;
				Stream->ReadAll([&Data](uint8_t* Buffer, size_t Size)
				{
					Data.reserve(Data.size() + Size);
					for (size_t i = 0; i < Size; i++)
						Data.push_back(Buffer[i]);
				});

				int Width, Height, Channels;
				uint8_t* Resource = stbi_load_from_memory((const uint8_t*)Data.data(), (int)Data.size(), &Width, &Height, &Channels, STBI_rgb_alpha);
				if (!Resource)
					return ContentException("load texture 2d: invalid file");

				auto* HeavyContent = (HeavyContentManager*)Content;
				auto* Device = HeavyContent->GetDevice();
				Graphics::Texture2D::Desc I = Graphics::Texture2D::Desc();
				I.Data = (void*)Resource;
				I.Width = (uint32_t)Width;
				I.Height = (uint32_t)Height;
				I.RowPitch = Device->GetRowPitch(I.Width);
				I.DepthPitch = Device->GetDepthPitch(I.RowPitch, I.Height);
				I.MipLevels = Device->GetMipLevel(I.Width, I.Height);

				auto ObjectStatus = ProcessRendererJob<Graphics::ExpectsGraphics<Graphics::Texture2D*>>(Device, [&I](Graphics::GraphicsDevice* Device) { return Device->CreateTexture2D(I); });
				stbi_image_free(Resource);
				if (!ObjectStatus)
					return ContentException(std::move(ObjectStatus.Error().message()));

				Core::UPtr<Graphics::Texture2D> Object = *ObjectStatus;
				auto* Existing = (Graphics::Texture2D*)Content->TryToCache(this, Stream->VirtualName(), *Object);
				if (Existing != nullptr)
					Object = Existing;

				Object->AddRef();
				return Object.Reset();
#else
				return ContentException("load texture 2d: unsupported");
#endif
			}
			void Texture2DProcessor::Free(AssetCache* Asset)
			{
				VI_ASSERT(Asset != nullptr, "asset should be set");
				auto* Value = (Graphics::Texture2D*)Asset->Resource;
				Asset->Resource = nullptr;
				Core::Memory::Release(Value);
			}

			ShaderProcessor::ShaderProcessor(ContentManager* Manager) : Processor(Manager)
			{
			}
			ShaderProcessor::~ShaderProcessor()
			{
			}
			ExpectsContent<void*> ShaderProcessor::Duplicate(AssetCache* Asset, const Core::VariantArgs& Args)
			{
				VI_ASSERT(Asset != nullptr, "asset should be set");
				VI_ASSERT(Asset->Resource != nullptr, "instance should be set");

				((Graphics::Shader*)Asset->Resource)->AddRef();
				return Asset->Resource;
			}
			ExpectsContent<void*> ShaderProcessor::Deserialize(Core::Stream* Stream, size_t Offset, const Core::VariantArgs& Args)
			{
				VI_ASSERT(Stream != nullptr, "stream should be set");
				Core::String Data;
				Stream->ReadAll([&Data](uint8_t* Buffer, size_t Size) { Data.append((char*)Buffer, Size); });

				Graphics::Shader::Desc I = Graphics::Shader::Desc();
				I.Filename = Stream->VirtualName();
				I.Data = Data;

				auto* HeavyContent = (HeavyContentManager*)Content;
				auto* Device = HeavyContent->GetDevice();
				auto ObjectStatus = ProcessRendererJob<Graphics::ExpectsGraphics<Graphics::Shader*>>(Device, [&I](Graphics::GraphicsDevice* Device) { return Device->CreateShader(I); });
				if (!ObjectStatus)
					return ContentException(std::move(ObjectStatus.Error().message()));

				Core::UPtr<Graphics::Shader> Object = *ObjectStatus;
				auto* Existing = (Graphics::Shader*)Content->TryToCache(this, Stream->VirtualName(), *Object);
				if (Existing != nullptr)
					Object = Existing;

				Object->AddRef();
				return Object.Reset();
			}
			void ShaderProcessor::Free(AssetCache* Asset)
			{
				VI_ASSERT(Asset != nullptr, "asset should be set");
				auto* Value = (Graphics::Shader*)Asset->Resource;
				Asset->Resource = nullptr;
				Core::Memory::Release(Value);
			}

			ModelProcessor::ModelProcessor(ContentManager* Manager) : Processor(Manager)
			{
			}
			ModelProcessor::~ModelProcessor()
			{
			}
			ExpectsContent<void*> ModelProcessor::Duplicate(AssetCache* Asset, const Core::VariantArgs& Args)
			{
				VI_ASSERT(Asset != nullptr, "asset should be set");
				VI_ASSERT(Asset->Resource != nullptr, "instance should be set");

				((Model*)Asset->Resource)->AddRef();
				return Asset->Resource;
			}
			ExpectsContent<void*> ModelProcessor::Deserialize(Core::Stream* Stream, size_t Offset, const Core::VariantArgs& Args)
			{
				VI_ASSERT(Stream != nullptr, "stream should be set");
				Core::UPtr<Model> Object = new Model();
				auto Path = Stream->VirtualName();
				if (Core::Stringify::EndsWith(Path, ".xml") || Core::Stringify::EndsWith(Path, ".json") || Core::Stringify::EndsWith(Path, ".jsonb") || Core::Stringify::EndsWith(Path, ".xml.gz") || Core::Stringify::EndsWith(Path, ".json.gz") || Core::Stringify::EndsWith(Path, ".jsonb.gz"))
				{
					auto DataStatus = Content->Load<Core::Schema>(Path);
					if (!DataStatus)
						return DataStatus.Error();

					Core::UPtr<Core::Schema> Data = *DataStatus;
					HeavySeries::Unpack(Data->Get("min"), &Object->Min);
					HeavySeries::Unpack(Data->Get("max"), &Object->Max);

					auto* Meshes = Data->Get("meshes");
					if (Meshes != nullptr)
					{
						Object->Meshes.reserve(Meshes->Size());
						for (auto& Mesh : Meshes->GetChilds())
						{
							Graphics::MeshBuffer::Desc I;
							I.AccessFlags = Options.AccessFlags;
							I.Usage = Options.Usage;

							if (!Series::Unpack(Mesh->Get("indices"), &I.Indices))
								return ContentException("import model: invalid indices");

							if (!HeavySeries::Unpack(Mesh->Get("vertices"), &I.Elements))
								return ContentException("import model: invalid vertices");

							auto* HeavyContent = (HeavyContentManager*)Content;
							auto* Device = HeavyContent->GetDevice();
							auto NewBuffer = ProcessRendererJob<Graphics::ExpectsGraphics<Graphics::MeshBuffer*>>(Device, [&I](Graphics::GraphicsDevice* Device) { return Device->CreateMeshBuffer(I); });
							if (!NewBuffer)
								return ContentException(std::move(NewBuffer.Error().message()));

							Object->Meshes.emplace_back(*NewBuffer);
							Series::Unpack(Mesh->Get("name"), &NewBuffer->Name);
							HeavySeries::Unpack(Mesh->Get("transform"), &NewBuffer->Transform);
						}
					}
				}
				else
				{
					auto Data = ImportForImmediateUse(Stream);
					if (!Data)
						return Data.Error();

					Object->Meshes.reserve(Data->Meshes.size());
					Object->Min = Data->Min;
					Object->Max = Data->Max;
					for (auto& Mesh : Data->Meshes)
					{
						Graphics::MeshBuffer::Desc I;
						I.AccessFlags = Options.AccessFlags;
						I.Usage = Options.Usage;
						I.Indices = std::move(Mesh.Indices);
						I.Elements = SkinVerticesToVertices(Mesh.Vertices);

						auto* HeavyContent = (HeavyContentManager*)Content;
						auto* Device = HeavyContent->GetDevice();
						auto NewBuffer = ProcessRendererJob<Graphics::ExpectsGraphics<Graphics::MeshBuffer*>>(Device, [&I](Graphics::GraphicsDevice* Device) { return Device->CreateMeshBuffer(I); });
						if (!NewBuffer)
							return ContentException(std::move(NewBuffer.Error().message()));

						Object->Meshes.emplace_back(*NewBuffer);
						NewBuffer->Name = Mesh.Name;
						NewBuffer->Transform = Mesh.Transform;
					}
				}

				auto* Existing = (Model*)Content->TryToCache(this, Stream->VirtualName(), *Object);
				if (Existing != nullptr)
					Object = Existing;

				Object->AddRef();
				return Object.Reset();
			}
			ExpectsContent<Core::Schema*> ModelProcessor::Import(Core::Stream* Stream, uint64_t Opts)
			{
				auto Info = ImportForImmediateUse(Stream, Opts);
				if (!Info || (Info->Meshes.empty() && Info->JointOffsets.empty()))
				{
					if (!Info)
						return Info.Error();

					return ContentException("import model: no mesh data");
				}

				auto* Blob = Core::Var::Set::Object();
				Blob->Key = "model";

				Series::Pack(Blob->Set("options"), Opts);
				HeavySeries::Pack(Blob->Set("inv-transform"), Info->Transform);
				HeavySeries::Pack(Blob->Set("min"), Info->Min.XYZW().SetW(Info->Low));
				HeavySeries::Pack(Blob->Set("max"), Info->Max.XYZW().SetW(Info->High));
				HeavySeries::Pack(Blob->Set("skeleton"), Info->Skeleton);

				Core::Schema* Meshes = Blob->Set("meshes", Core::Var::Array());
				for (auto&& It : Info->Meshes)
				{
					Core::Schema* Mesh = Meshes->Set("mesh");
					Series::Pack(Mesh->Set("name"), It.Name);
					HeavySeries::Pack(Mesh->Set("transform"), It.Transform);
					HeavySeries::Pack(Mesh->Set("vertices"), It.Vertices);
					Series::Pack(Mesh->Set("indices"), It.Indices);
					Series::Pack(Mesh->Set("joints"), It.JointIndices);
				}

				return Blob;
			}
			ExpectsContent<ModelInfo> ModelProcessor::ImportForImmediateUse(Core::Stream* Stream, uint64_t Opts)
			{
#ifdef VI_ASSIMP
				Core::Vector<char> Data;
				Stream->ReadAll([&Data](uint8_t* Buffer, size_t Size)
				{
					Data.reserve(Data.size() + Size);
					for (size_t i = 0; i < Size; i++)
						Data.push_back(Buffer[i]);
				});

				Assimp::Importer Importer;
				auto* Scene = Importer.ReadFileFromMemory(Data.data(), Data.size(), (uint32_t)Opts, Core::OS::Path::GetExtension(Stream->VirtualName()).data());
				if (!Scene)
					return ContentException(Core::Stringify::Text("import model: %s", Importer.GetErrorString()));

				ModelInfo Info;
				FillSceneGeometries(&Info, Scene, Scene->mRootNode, Scene->mRootNode->mTransformation);
				FillSceneSkeletons(&Info, Scene);
				return Info;
#else
				return ContentException("import model: unsupported");
#endif
			}
			void ModelProcessor::Free(AssetCache* Asset)
			{
				VI_ASSERT(Asset != nullptr, "asset should be set");
				auto* Value = (Model*)Asset->Resource;
				Asset->Resource = nullptr;
				Core::Memory::Release(Value);
			}
			
			SkinModelProcessor::SkinModelProcessor(ContentManager* Manager) : Processor(Manager)
			{
			}
			SkinModelProcessor::~SkinModelProcessor()
			{
			}
			ExpectsContent<void*> SkinModelProcessor::Duplicate(AssetCache* Asset, const Core::VariantArgs& Args)
			{
				VI_ASSERT(Asset != nullptr, "asset should be set");
				VI_ASSERT(Asset->Resource != nullptr, "instance should be set");

				((SkinModel*)Asset->Resource)->AddRef();
				return Asset->Resource;
			}
			ExpectsContent<void*> SkinModelProcessor::Deserialize(Core::Stream* Stream, size_t Offset, const Core::VariantArgs& Args)
			{
				VI_ASSERT(Stream != nullptr, "stream should be set");
				Core::UPtr<SkinModel> Object = new SkinModel();
				auto Path = Stream->VirtualName();
				if (Core::Stringify::EndsWith(Path, ".xml") || Core::Stringify::EndsWith(Path, ".json") || Core::Stringify::EndsWith(Path, ".jsonb") || Core::Stringify::EndsWith(Path, ".xml.gz") || Core::Stringify::EndsWith(Path, ".json.gz") || Core::Stringify::EndsWith(Path, ".jsonb.gz"))
				{
					auto DataStatus = Content->Load<Core::Schema>(Path);
					if (!DataStatus)
						return DataStatus.Error();

					Core::UPtr<Core::Schema> Data = *DataStatus;
					HeavySeries::Unpack(Data->Get("inv-transform"), &Object->InvTransform);
					HeavySeries::Unpack(Data->Get("min"), &Object->Min);
					HeavySeries::Unpack(Data->Get("max"), &Object->Max);
					HeavySeries::Unpack(Data->Get("skeleton"), &Object->Skeleton);
					Object->Transform = Object->InvTransform.Inv();

					auto* Meshes = Data->Get("meshes");
					if (Meshes != nullptr)
					{
						Object->Meshes.reserve(Meshes->Size());
						for (auto& Mesh : Meshes->GetChilds())
						{
							Graphics::SkinMeshBuffer::Desc I;
							I.AccessFlags = Options.AccessFlags;
							I.Usage = Options.Usage;

							if (!Series::Unpack(Mesh->Get("indices"), &I.Indices))
								return ContentException("import model: invalid indices");

							if (!HeavySeries::Unpack(Mesh->Get("vertices"), &I.Elements))
								return ContentException("import model: invalid vertices");

							auto* HeavyContent = (HeavyContentManager*)Content;
							auto* Device = HeavyContent->GetDevice();
							auto NewBuffer = ProcessRendererJob<Graphics::ExpectsGraphics<Graphics::SkinMeshBuffer*>>(Device, [&I](Graphics::GraphicsDevice* Device) { return Device->CreateSkinMeshBuffer(I); });
							if (!NewBuffer)
								return ContentException(std::move(NewBuffer.Error().message()));

							Object->Meshes.emplace_back(*NewBuffer);
							Series::Unpack(Mesh->Get("name"), &NewBuffer->Name);
							HeavySeries::Unpack(Mesh->Get("transform"), &NewBuffer->Transform);
							Series::Unpack(Mesh->Get("joints"), &NewBuffer->Joints);
						}
					}
				}
				else
				{
					auto Data = ModelProcessor::ImportForImmediateUse(Stream);
					if (!Data)
						return Data.Error();

					Object = new SkinModel();
					Object->Meshes.reserve(Data->Meshes.size());
					Object->InvTransform = Data->Transform;
					Object->Min = Data->Min;
					Object->Max = Data->Max;
					Object->Skeleton = std::move(Data->Skeleton);

					for (auto& Mesh : Data->Meshes)
					{
						Graphics::SkinMeshBuffer::Desc I;
						I.AccessFlags = Options.AccessFlags;
						I.Usage = Options.Usage;
						I.Indices = std::move(Mesh.Indices);
						I.Elements = std::move(Mesh.Vertices);

						auto* HeavyContent = (HeavyContentManager*)Content;
						auto* Device = HeavyContent->GetDevice();
						auto NewBuffer = ProcessRendererJob<Graphics::ExpectsGraphics<Graphics::SkinMeshBuffer*>>(Device, [&I](Graphics::GraphicsDevice* Device) { return Device->CreateSkinMeshBuffer(I); });
						if (!NewBuffer)
							return ContentException(std::move(NewBuffer.Error().message()));

						Object->Meshes.emplace_back(*NewBuffer);
						NewBuffer->Name = Mesh.Name;
						NewBuffer->Transform = Mesh.Transform;
						NewBuffer->Joints = std::move(Mesh.JointIndices);
					}
				}

				auto* Existing = (SkinModel*)Content->TryToCache(this, Stream->VirtualName(), *Object);
				if (Existing != nullptr)
					Object = Existing;

				Object->AddRef();
				return Object.Reset();
			}
			void SkinModelProcessor::Free(AssetCache* Asset)
			{
				VI_ASSERT(Asset != nullptr, "asset should be set");
				auto* Value = (SkinModel*)Asset->Resource;
				Asset->Resource = nullptr;
				Core::Memory::Release(Value);
			}

			SkinAnimationProcessor::SkinAnimationProcessor(ContentManager* Manager) : Processor(Manager)
			{
			}
			SkinAnimationProcessor::~SkinAnimationProcessor()
			{
			}
			ExpectsContent<void*> SkinAnimationProcessor::Duplicate(AssetCache* Asset, const Core::VariantArgs& Args)
			{
				VI_ASSERT(Asset != nullptr, "asset should be set");
				VI_ASSERT(Asset->Resource != nullptr, "instance should be set");

				((Layer::SkinAnimation*)Asset->Resource)->AddRef();
				return Asset->Resource;
			}
			ExpectsContent<void*> SkinAnimationProcessor::Deserialize(Core::Stream* Stream, size_t Offset, const Core::VariantArgs& Args)
			{
				VI_ASSERT(Stream != nullptr, "stream should be set");
				Core::Vector<Trigonometry::SkinAnimatorClip> Clips;
				auto Path = Stream->VirtualName();
				if (Core::Stringify::EndsWith(Path, ".xml") || Core::Stringify::EndsWith(Path, ".json") || Core::Stringify::EndsWith(Path, ".jsonb") || Core::Stringify::EndsWith(Path, ".xml.gz") || Core::Stringify::EndsWith(Path, ".json.gz") || Core::Stringify::EndsWith(Path, ".jsonb.gz"))
				{
					auto DataStatus = Content->Load<Core::Schema>(Path);
					if (!DataStatus)
						return DataStatus.Error();

					Core::UPtr<Core::Schema> Data = *DataStatus;
					Clips.reserve(Data->Size());
					for (auto& Item : Data->GetChilds())
					{
						Clips.emplace_back();
						auto& Clip = Clips.back();
						Series::Unpack(Item->Get("name"), &Clip.Name);
						Series::Unpack(Item->Get("duration"), &Clip.Duration);
						Series::Unpack(Item->Get("rate"), &Clip.Rate);

						auto* Keys = Item->Get("keys");
						if (Keys != nullptr)
						{
							Clip.Keys.reserve(Keys->Size());
							for (auto& Key : Keys->GetChilds())
							{
								Clip.Keys.emplace_back();
								auto& Pose = Clip.Keys.back();
								Series::Unpack(Key, &Pose.Time);

								size_t ArrayOffset = 0;
								for (auto& Orientation : Key->GetChilds())
								{
									if (!ArrayOffset++)
										continue;

									Pose.Pose.emplace_back();
									auto& Value = Pose.Pose.back();
									HeavySeries::Unpack(Orientation->Get("position"), &Value.Position);
									HeavySeries::Unpack(Orientation->Get("scale"), &Value.Scale);
									HeavySeries::Unpack(Orientation->Get("rotation"), &Value.Rotation);
									Series::Unpack(Orientation->Get("time"), &Value.Time);
								}
							}
						}
					}
				}
				else
				{
					auto NewClips = ImportForImmediateUse(Stream);
					if (!NewClips)
						return NewClips.Error();

					Clips = std::move(*NewClips);
				}

				if (Clips.empty())
					return ContentException("load animation: no clips");

				Core::UPtr<Layer::SkinAnimation> Object = new Layer::SkinAnimation(std::move(Clips));
				auto* Existing = (Layer::SkinAnimation*)Content->TryToCache(this, Stream->VirtualName(), *Object);
				if (Existing != nullptr)
					Object = Existing;

				Object->AddRef();
				return Object.Reset();
			}
			ExpectsContent<Core::Schema*> SkinAnimationProcessor::Import(Core::Stream* Stream, uint64_t Opts)
			{
				auto Info = ImportForImmediateUse(Stream, Opts);
				if (!Info)
					return Info.Error();

				auto* Blob = Core::Var::Set::Array();
				Blob->Key = "animation";

				for (auto& Clip : *Info)
				{
					auto* Item = Blob->Push(Core::Var::Set::Object());
					Series::Pack(Item->Set("name"), Clip.Name);
					Series::Pack(Item->Set("duration"), Clip.Duration);
					Series::Pack(Item->Set("rate"), Clip.Rate);

					auto* Keys = Item->Set("keys", Core::Var::Set::Array());
					Keys->Reserve(Clip.Keys.size());

					for (auto& Key : Clip.Keys)
					{
						auto* Pose = Keys->Set("pose", Core::Var::Set::Array());
						Pose->Reserve(Key.Pose.size() + 1);
						Series::Pack(Pose, Key.Time);

						for (auto& Orientation : Key.Pose)
						{
							auto* Value = Pose->Set("key", Core::Var::Set::Object());
							HeavySeries::Pack(Value->Set("position"), Orientation.Position);
							HeavySeries::Pack(Value->Set("scale"), Orientation.Scale);
							HeavySeries::Pack(Value->Set("rotation"), Orientation.Rotation);
							Series::Pack(Value->Set("time"), Orientation.Time);
						}
					}
				}

				return Blob;
			}
			ExpectsContent<Core::Vector<Trigonometry::SkinAnimatorClip>> SkinAnimationProcessor::ImportForImmediateUse(Core::Stream* Stream, uint64_t Opts)
			{
#ifdef VI_ASSIMP
				Core::Vector<char> Data;
				Stream->ReadAll([&Data](uint8_t* Buffer, size_t Size)
				{
					Data.reserve(Data.size() + Size);
					for (size_t i = 0; i < Size; i++)
						Data.push_back(Buffer[i]);
				});

				Assimp::Importer Importer;
				auto* Scene = Importer.ReadFileFromMemory(Data.data(), Data.size(), (uint32_t)Opts, Core::OS::Path::GetExtension(Stream->VirtualName()).data());
				if (!Scene)
					return ContentException(Core::Stringify::Text("import animation: %s", Importer.GetErrorString()));

				Core::Vector<Trigonometry::SkinAnimatorClip> Info;
				FillSceneAnimations(&Info, Scene);
				return Info;
#else
				return ContentException("import animation: unsupported");
#endif
			}
			void SkinAnimationProcessor::Free(AssetCache* Asset)
			{
				VI_ASSERT(Asset != nullptr, "asset should be set");
				auto* Value = (Layer::SkinAnimation*)Asset->Resource;
				Asset->Resource = nullptr;
				Core::Memory::Release(Value);
			}

			HullShapeProcessor::HullShapeProcessor(ContentManager* Manager) : Processor(Manager)
			{
			}
			HullShapeProcessor::~HullShapeProcessor()
			{
			}
			ExpectsContent<void*> HullShapeProcessor::Duplicate(AssetCache* Asset, const Core::VariantArgs& Args)
			{
				VI_ASSERT(Asset != nullptr, "asset should be set");
				VI_ASSERT(Asset->Resource != nullptr, "instance should be set");

				((Physics::HullShape*)Asset->Resource)->AddRef();
				return Asset->Resource;
			}
			ExpectsContent<void*> HullShapeProcessor::Deserialize(Core::Stream* Stream, size_t Offset, const Core::VariantArgs& Args)
			{
				VI_ASSERT(Stream != nullptr, "stream should be set");
				auto DataStatus = Content->Load<Core::Schema>(Stream->VirtualName());
				if (!DataStatus)
					return DataStatus.Error();

				Core::UPtr<Core::Schema> Data = *DataStatus;
				Core::Vector<Core::Schema*> Meshes = Data->FetchCollection("meshes.mesh");
				Core::Vector<Trigonometry::Vertex> Vertices;
				Core::Vector<int> Indices;

				for (auto&& Mesh : Meshes)
				{
					if (!Series::Unpack(Mesh->Find("indices"), &Indices))
						return ContentException("import shape: invalid indices");

					if (!HeavySeries::Unpack(Mesh->Find("vertices"), &Vertices))
						return ContentException("import shape: invalid vertices");
				}

				Core::UPtr<Physics::HullShape> Object = new Physics::HullShape(std::move(Vertices), std::move(Indices));
				if (!Object->GetShape())
					return ContentException("import shape: invalid shape");

				auto* Existing = (Physics::HullShape*)Content->TryToCache(this, Stream->VirtualName(), *Object);
				if (Existing != nullptr)
					Object = Existing;

				Object->AddRef();
				return Object.Reset();
			}
			void HullShapeProcessor::Free(AssetCache* Asset)
			{
				VI_ASSERT(Asset != nullptr, "asset should be set");
				auto* Value = (Physics::HullShape*)Asset->Resource;
				Asset->Resource = nullptr;
				Core::Memory::Release(Value);
			}
		}
	}
}
