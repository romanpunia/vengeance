#include "processors.h"
#include "components.h"
#include "renderers.h"
#include <vitex/network/http.h>
#ifdef VI_OPENAL
#ifdef VI_AL_AT_OPENAL
#include <open_al/al.h>
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

namespace vitex
{
	namespace layer
	{
		namespace processors
		{
#ifdef VI_ASSIMP
			trigonometry::matrix4x4 from_assimp_matrix(const aiMatrix4x4& root)
			{
				return trigonometry::matrix4x4(
					root.a1, root.a2, root.a3, root.a4,
					root.b1, root.b2, root.b3, root.b4,
					root.c1, root.c2, root.c3, root.c4,
					root.d1, root.d2, root.d3, root.d4).transpose();
			}
			core::string get_mesh_name(const std::string_view& name, model_info* info)
			{
				if (name.empty())
				{
					auto random = compute::crypto::random_bytes(8);
					if (!random)
						return "NULL";

					auto hash = compute::crypto::hash_hex(compute::digests::MD5(), *random);
					if (!hash)
						return "NULL";

					return hash->substr(0, 8);
				}

				core::string result = core::string(name);
				for (auto&& data : info->meshes)
				{
					if (data.name == result)
						result += '_';
				}

				return result;
			}
			core::string get_joint_name(const std::string_view& base_name, model_info* info, mesh_blob* blob)
			{
				if (!base_name.empty())
					return core::string(base_name);

				core::string name = core::string(base_name) + '?';
				while (info->joint_offsets.find(name) != info->joint_offsets.end())
					name += '?';

				return name;
			}
			bool get_key_from_time(core::vector<trigonometry::animator_key>& keys, float time, trigonometry::animator_key& result)
			{
				for (auto& key : keys)
				{
					if (key.time == time)
					{
						result = key;
						return true;
					}

					if (key.time < time)
						result = key;
				}

				return false;
			}
			void update_scene_bounds(model_info* info)
			{
				if (info->min.x < info->low)
					info->low = info->min.x;

				if (info->min.y < info->low)
					info->low = info->min.y;

				if (info->min.z < info->low)
					info->low = info->min.z;

				if (info->max.x > info->high)
					info->high = info->max.x;

				if (info->max.y > info->high)
					info->high = info->max.y;

				if (info->max.z > info->high)
					info->high = info->max.z;
			}
			void update_scene_bounds(model_info* info, const trigonometry::skin_vertex& element)
			{
				if (element.position_x > info->max.x)
					info->max.x = element.position_x;
				else if (element.position_x < info->min.x)
					info->min.x = element.position_x;

				if (element.position_y > info->max.y)
					info->max.y = element.position_y;
				else if (element.position_y < info->min.y)
					info->min.y = element.position_y;

				if (element.position_z > info->max.z)
					info->max.z = element.position_z;
				else if (element.position_z < info->min.z)
					info->min.z = element.position_z;
			}
			bool fill_scene_skeleton(model_info* info, aiNode* node, trigonometry::joint* top)
			{
				core::string name = node->mName.C_Str();
				auto it = info->joint_offsets.find(name);
				if (it == info->joint_offsets.end())
				{
					if (top != nullptr)
					{
						auto& global = info->joint_offsets[name];
						global.index = info->global_index++;
						global.linking = true;

						it = info->joint_offsets.find(name);
						goto add_linking_joint;
					}

					for (uint32_t i = 0; i < node->mNumChildren; i++)
					{
						auto& next = node->mChildren[i];
						if (fill_scene_skeleton(info, next, top))
							return true;
					}

					return false;
				}

				if (top != nullptr)
				{
				add_linking_joint:
					top->childs.emplace_back();
					auto& next = top->childs.back();
					top = &next;
				}
				else
					top = &info->skeleton;

				top->global = from_assimp_matrix(node->mTransformation);
				top->local = it->second.local;
				top->index = it->second.index;
				top->name = name;

				for (uint32_t i = 0; i < node->mNumChildren; i++)
				{
					auto& next = node->mChildren[i];
					fill_scene_skeleton(info, next, top);
				}

				return true;
			}
			void fill_scene_geometry(model_info* info, mesh_blob* blob, aiMesh* mesh)
			{
				blob->vertices.reserve((size_t)mesh->mNumVertices);
				for (uint32_t v = 0; v < mesh->mNumVertices; v++)
				{
					auto& vertex = mesh->mVertices[v];
					trigonometry::skin_vertex next = { vertex.x, vertex.y, vertex.z, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -1, -1, -1, -1, 0, 0, 0, 0 };
					update_scene_bounds(info, next);

					if (mesh->HasNormals())
					{
						auto& normal = mesh->mNormals[v];
						next.normal_x = normal.x;
						next.normal_y = normal.y;
						next.normal_z = normal.z;
					}

					if (mesh->HasTextureCoords(0))
					{
						auto& texcoord = mesh->mTextureCoords[0][v];
						next.texcoord_x = texcoord.x;
						next.texcoord_y = -texcoord.y;
					}

					if (mesh->HasTangentsAndBitangents())
					{
						auto& tangent = mesh->mTangents[v];
						next.tangent_x = tangent.x;
						next.tangent_y = tangent.y;
						next.tangent_z = tangent.z;

						auto& bitangent = mesh->mBitangents[v];
						next.bitangent_x = bitangent.x;
						next.bitangent_y = bitangent.y;
						next.bitangent_z = bitangent.z;
					}

					blob->vertices.push_back(next);
				}

				for (uint32_t f = 0; f < mesh->mNumFaces; f++)
				{
					auto* face = &mesh->mFaces[f];
					blob->indices.reserve(blob->indices.size() + (size_t)face->mNumIndices);
					for (uint32_t i = 0; i < face->mNumIndices; i++)
						blob->indices.push_back(face->mIndices[i]);
				}
			}
			void fill_scene_joints(model_info* info, mesh_blob* blob, aiMesh* mesh)
			{
				for (uint32_t i = 0; i < mesh->mNumBones; i++)
				{
					auto& bone = mesh->mBones[i];
					auto name = get_joint_name(bone->mName.C_Str(), info, blob);

					auto global = info->joint_offsets.find(name);
					if (global == info->joint_offsets.end())
					{
						auto& next = info->joint_offsets[name];
						next.local = from_assimp_matrix(bone->mOffsetMatrix);
						next.index = info->global_index++;
						next.linking = false;
						global = info->joint_offsets.find(name);
					}

					auto& local = blob->joint_indices[global->second.index];
					local = blob->local_index++;

					for (uint32_t j = 0; j < bone->mNumWeights; j++)
					{
						auto& weight = bone->mWeights[j];
						auto& vertex = blob->vertices[weight.mVertexId];
						if (vertex.joint_index0 == -1.0f)
						{
							vertex.joint_index0 = (float)local;
							vertex.joint_bias0 = weight.mWeight;
						}
						else if (vertex.joint_index1 == -1.0f)
						{
							vertex.joint_index1 = (float)local;
							vertex.joint_bias1 = weight.mWeight;
						}
						else if (vertex.joint_index2 == -1.0f)
						{
							vertex.joint_index2 = (float)local;
							vertex.joint_bias2 = weight.mWeight;
						}
						else if (vertex.joint_index3 == -1.0f)
						{
							vertex.joint_index3 = (float)local;
							vertex.joint_bias3 = weight.mWeight;
						}
					}
				}
			}
			void fill_scene_geometries(model_info* info, const aiScene* scene, aiNode* node, const aiMatrix4x4& parent_transform)
			{
				info->meshes.reserve(info->meshes.size() + (size_t)node->mNumMeshes);
				if (node == scene->mRootNode)
					info->transform = from_assimp_matrix(scene->mRootNode->mTransformation).inv();

				for (uint32_t n = 0; n < node->mNumMeshes; n++)
				{
					info->meshes.emplace_back();
					mesh_blob& blob = info->meshes.back();
					auto& geometry = scene->mMeshes[node->mMeshes[n]];
					blob.name = get_mesh_name(geometry->mName.C_Str(), info);
					blob.transform = from_assimp_matrix(parent_transform);

					fill_scene_geometry(info, &blob, geometry);
					fill_scene_joints(info, &blob, geometry);
					update_scene_bounds(info);
				}

				for (uint32_t n = 0; n < node->mNumChildren; n++)
				{
					auto& next = node->mChildren[n];
					fill_scene_geometries(info, scene, next, parent_transform);
				}
			}
			void fill_scene_skeletons(model_info* info, const aiScene* scene)
			{
				fill_scene_skeleton(info, scene->mRootNode, nullptr);
				for (auto& blob : info->meshes)
				{
					for (auto& vertex : blob.vertices)
					{
						float weight = 0.0f;
						if (vertex.joint_bias0 > 0.0f)
							weight += vertex.joint_bias0;
						if (vertex.joint_bias1 > 0.0f)
							weight += vertex.joint_bias1;
						if (vertex.joint_bias2 > 0.0f)
							weight += vertex.joint_bias2;
						if (vertex.joint_bias3 > 0.0f)
							weight += vertex.joint_bias3;

						if (!weight)
							continue;

						if (vertex.joint_bias0 > 0.0f)
							vertex.joint_bias0 /= weight;
						if (vertex.joint_bias1 > 0.0f)
							vertex.joint_bias1 /= weight;
						if (vertex.joint_bias2 > 0.0f)
							vertex.joint_bias2 /= weight;
						if (vertex.joint_bias3 > 0.0f)
							vertex.joint_bias3 /= weight;
					}
				}
			}
			void fill_scene_channel(aiNodeAnim* channel, model_channel& target)
			{
				target.positions.reserve((size_t)channel->mNumPositionKeys);
				for (uint32_t k = 0; k < channel->mNumPositionKeys; k++)
				{
					aiVectorKey& key = channel->mPositionKeys[k];
					target.positions[key.mTime] = trigonometry::vector3(key.mValue.x, key.mValue.y, key.mValue.z);
				}

				target.scales.reserve((size_t)channel->mNumScalingKeys);
				for (uint32_t k = 0; k < channel->mNumScalingKeys; k++)
				{
					aiVectorKey& key = channel->mScalingKeys[k];
					target.scales[key.mTime] = trigonometry::vector3(key.mValue.x, key.mValue.y, key.mValue.z);
				}

				target.rotations.reserve((size_t)channel->mNumRotationKeys);
				for (uint32_t k = 0; k < channel->mNumRotationKeys; k++)
				{
					aiQuatKey& key = channel->mRotationKeys[k];
					target.rotations[key.mTime] = trigonometry::quaternion(key.mValue.x, key.mValue.y, key.mValue.z, key.mValue.w);
				}
			}
			void fill_scene_timeline(const core::unordered_set<float>& timings, core::vector<float>& timeline)
			{
				timeline.reserve(timings.size());
				for (auto& time : timings)
					timeline.push_back(time);

				VI_SORT(timeline.begin(), timeline.end(), [](float a, float b)
				{
					return a < b;
				});
			}
			void fill_scene_keys(model_channel& info, core::vector<trigonometry::animator_key>& keys)
			{
				core::unordered_set<float> timings;
				timings.reserve(keys.size());

				float first_position = std::numeric_limits<float>::max();
				for (auto& item : info.positions)
				{
					timings.insert(item.first);
					if (item.first < first_position)
						first_position = item.first;
				}

				float first_scale = std::numeric_limits<float>::max();
				for (auto& item : info.scales)
				{
					timings.insert(item.first);
					if (item.first < first_scale)
						first_scale = item.first;
				}

				float first_rotation = std::numeric_limits<float>::max();
				for (auto& item : info.rotations)
				{
					timings.insert(item.first);
					if (item.first < first_rotation)
						first_rotation = item.first;
				}

				core::vector<float> timeline;
				trigonometry::vector3 last_position = (info.positions.empty() ? trigonometry::vector3::zero() : info.positions[first_position]);
				trigonometry::vector3 last_scale = (info.scales.empty() ? trigonometry::vector3::one() : info.scales[first_scale]);
				trigonometry::quaternion last_rotation = (info.rotations.empty() ? trigonometry::quaternion() : info.rotations[first_rotation]);
				fill_scene_timeline(timings, timeline);
				keys.resize(timings.size());

				size_t index = 0;
				for (auto& time : timeline)
				{
					auto& target = keys[index++];
					target.position = last_position;
					target.scale = last_scale;
					target.rotation = last_rotation;
					target.time = time;

					auto position = info.positions.find(time);
					if (position != info.positions.end())
					{
						target.position = position->second;
						last_position = target.position;
					}

					auto scale = info.scales.find(time);
					if (scale != info.scales.end())
					{
						target.scale = scale->second;
						last_scale = target.scale;
					}

					auto rotation = info.rotations.find(time);
					if (rotation != info.rotations.end())
					{
						target.rotation = rotation->second;
						last_rotation = target.rotation;
					}
				}
			}
			void fill_scene_clip(trigonometry::skin_animator_clip& clip, core::unordered_map<core::string, mesh_bone>& indices, core::unordered_map<core::string, core::vector<trigonometry::animator_key>>& channels)
			{
				core::unordered_set<float> timings;
				for (auto& channel : channels)
				{
					timings.reserve(channel.second.size());
					for (auto& key : channel.second)
						timings.insert(key.time);
				}

				core::vector<float> timeline;
				fill_scene_timeline(timings, timeline);

				for (auto& time : timeline)
				{
					clip.keys.emplace_back();
					auto& key = clip.keys.back();
					key.pose.resize(indices.size());
					key.time = time;

					for (auto& index : indices)
					{
						auto& pose = key.pose[index.second.index];
						pose.position = index.second.defaults.position;
						pose.scale = index.second.defaults.scale;
						pose.rotation = index.second.defaults.rotation;
						pose.time = time;
					}

					for (auto& channel : channels)
					{
						auto index = indices.find(channel.first);
						if (index == indices.end())
							continue;

						auto& next = key.pose[index->second.index];
						if (get_key_from_time(channel.second, time, next))
							next.time = time;
					}
				}
			}
			void fill_scene_joint_indices(const aiScene* scene, aiNode* node, core::unordered_map<core::string, mesh_bone>& indices, size_t& index)
			{
				for (uint32_t n = 0; n < node->mNumMeshes; n++)
				{
					auto& mesh = scene->mMeshes[node->mMeshes[n]];
					for (uint32_t i = 0; i < mesh->mNumBones; i++)
					{
						auto& bone = mesh->mBones[i];
						auto joint = indices.find(bone->mName.C_Str());
						if (joint == indices.end())
							indices[bone->mName.C_Str()].index = index++;
					}
				}

				for (uint32_t n = 0; n < node->mNumChildren; n++)
				{
					auto& next = node->mChildren[n];
					fill_scene_joint_indices(scene, next, indices, index);
				}
			}
			bool fill_scene_joint_defaults(aiNode* node, core::unordered_map<core::string, mesh_bone>& indices, size_t& index, bool in_skeleton)
			{
				core::string name = node->mName.C_Str();
				auto it = indices.find(name);
				if (it == indices.end())
				{
					if (in_skeleton)
					{
						auto& joint = indices[name];
						joint.index = index++;
						it = indices.find(name);
						goto add_linking_joint;
					}

					for (uint32_t i = 0; i < node->mNumChildren; i++)
					{
						auto& next = node->mChildren[i];
						if (fill_scene_joint_defaults(next, indices, index, in_skeleton))
							return true;
					}

					return false;
				}

			add_linking_joint:
				auto offset = from_assimp_matrix(node->mTransformation);
				it->second.defaults.position = offset.position();
				it->second.defaults.scale = offset.scale();
				it->second.defaults.rotation = offset.rotation_quaternion();

				for (uint32_t i = 0; i < node->mNumChildren; i++)
				{
					auto& next = node->mChildren[i];
					fill_scene_joint_defaults(next, indices, index, true);
				}

				return true;
			}
			void fill_scene_animations(core::vector<trigonometry::skin_animator_clip>* info, const aiScene* scene)
			{
				core::unordered_map<core::string, mesh_bone> indices; size_t index = 0;
				fill_scene_joint_indices(scene, scene->mRootNode, indices, index);
				fill_scene_joint_defaults(scene->mRootNode, indices, index, false);

				info->reserve((size_t)scene->mNumAnimations);
				for (uint32_t i = 0; i < scene->mNumAnimations; i++)
				{
					aiAnimation* animation = scene->mAnimations[i];
					info->emplace_back();

					auto& clip = info->back();
					clip.name = animation->mName.C_Str();
					clip.duration = (float)animation->mDuration;
					clip.rate = compute::mathf::max(0.01f, (float)animation->mTicksPerSecond);

					core::unordered_map<core::string, core::vector<trigonometry::animator_key>> channels;
					for (uint32_t j = 0; j < animation->mNumChannels; j++)
					{
						auto& channel = animation->mChannels[j];
						auto& frames = channels[channel->mNodeName.C_Str()];

						model_channel target;
						fill_scene_channel(channel, target);
						fill_scene_keys(target, frames);
					}
					fill_scene_clip(clip, indices, channels);
				}
			}
#endif
			core::vector<trigonometry::vertex> skin_vertices_to_vertices(const core::vector<trigonometry::skin_vertex>& data)
			{
				core::vector<trigonometry::vertex> result;
				result.resize(data.size());

				for (size_t i = 0; i < data.size(); i++)
				{
					auto& from = data[i];
					auto& to = result[i];
					to.position_x = from.position_x;
					to.position_y = from.position_y;
					to.position_z = from.position_z;
					to.texcoord_x = from.texcoord_x;
					to.texcoord_y = from.texcoord_y;
					to.normal_x = from.normal_x;
					to.normal_y = from.normal_y;
					to.normal_z = from.normal_z;
					to.tangent_x = from.tangent_x;
					to.tangent_y = from.tangent_y;
					to.tangent_z = from.tangent_z;
					to.bitangent_x = from.bitangent_x;
					to.bitangent_y = from.bitangent_y;
					to.bitangent_z = from.bitangent_z;
				}

				return result;
			}
			template <typename t>
			t process_renderer_job(graphics::graphics_device* device, std::function<t(graphics::graphics_device*)>&& callback)
			{
				core::promise<t> future;
				graphics::render_thread_callback job = [future, callback = std::move(callback)](graphics::graphics_device* device) mutable
				{
					future.set(callback(device));
				};

				auto* app = heavy_application::has_instance() ? heavy_application::get() : nullptr;
				if (!app || app->get_state() != application_state::active || device != app->renderer)
					device->lockup(std::move(job));
				else
					device->enqueue(std::move(job));

				return future.get();
			}

			material_processor::material_processor(content_manager* manager) : processor(manager)
			{
			}
			expects_content<void*> material_processor::duplicate(asset_cache* asset, const core::variant_args& args)
			{
				VI_ASSERT(asset != nullptr, "asset should be set");
				VI_ASSERT(asset->resource != nullptr, "instance should be set");

				((layer::material*)asset->resource)->add_ref();
				return asset->resource;
			}
			expects_content<void*> material_processor::deserialize(core::stream* stream, size_t offset, const core::variant_args& args)
			{
				VI_ASSERT(stream != nullptr, "stream should be set");
				auto data_status = content->load<core::schema>(stream->virtual_name());
				if (!data_status)
					return data_status.error();

				core::string path;
				core::uptr<core::schema> data = *data_status;
				core::uptr<layer::material> object = new layer::material(nullptr);
				if (series::unpack(data->get("diffuse-map"), &path) && !path.empty())
				{
					auto new_texture = content->load<graphics::texture_2d>(path);
					if (!new_texture)
						return new_texture.error();

					object->set_diffuse_map(*new_texture);
					core::memory::release(*new_texture);
				}

				if (series::unpack(data->get("normal-map"), &path) && !path.empty())
				{
					auto new_texture = content->load<graphics::texture_2d>(path);
					if (!new_texture)
						return new_texture.error();

					object->set_normal_map(*new_texture);
					core::memory::release(*new_texture);
				}

				if (series::unpack(data->get("metallic-map"), &path) && !path.empty())
				{
					auto new_texture = content->load<graphics::texture_2d>(path);
					if (!new_texture)
						return new_texture.error();

					object->set_metallic_map(*new_texture);
					core::memory::release(*new_texture);
				}

				if (series::unpack(data->get("roughness-map"), &path) && !path.empty())
				{
					auto new_texture = content->load<graphics::texture_2d>(path);
					if (!new_texture)
						return new_texture.error();

					object->set_roughness_map(*new_texture);
					core::memory::release(*new_texture);
				}

				if (series::unpack(data->get("height-map"), &path) && !path.empty())
				{
					auto new_texture = content->load<graphics::texture_2d>(path);
					if (!new_texture)
						return new_texture.error();

					object->set_height_map(*new_texture);
					core::memory::release(*new_texture);
				}

				if (series::unpack(data->get("occlusion-map"), &path) && !path.empty())
				{
					auto new_texture = content->load<graphics::texture_2d>(path);
					if (!new_texture)
						return new_texture.error();

					object->set_occlusion_map(*new_texture);
					core::memory::release(*new_texture);
				}

				if (series::unpack(data->get("emission-map"), &path) && !path.empty())
				{
					auto new_texture = content->load<graphics::texture_2d>(path);
					if (!new_texture)
						return new_texture.error();

					object->set_emission_map(*new_texture);
					core::memory::release(*new_texture);
				}

				core::string name;
				heavy_series::unpack(data->get("emission"), &object->surface.emission);
				heavy_series::unpack(data->get("metallic"), &object->surface.metallic);
				heavy_series::unpack(data->get("penetration"), &object->surface.penetration);
				heavy_series::unpack(data->get("diffuse"), &object->surface.diffuse);
				heavy_series::unpack(data->get("scattering"), &object->surface.scattering);
				heavy_series::unpack(data->get("roughness"), &object->surface.roughness);
				heavy_series::unpack(data->get("occlusion"), &object->surface.occlusion);
				series::unpack(data->get("fresnel"), &object->surface.fresnel);
				series::unpack(data->get("refraction"), &object->surface.refraction);
				series::unpack(data->get("transparency"), &object->surface.transparency);
				series::unpack(data->get("environment"), &object->surface.environment);
				series::unpack(data->get("radius"), &object->surface.radius);
				series::unpack(data->get("height"), &object->surface.height);
				series::unpack(data->get("bias"), &object->surface.bias);
				series::unpack(data->get("name"), &name);
				object->set_name(name);

				auto* existing = (layer::material*)content->try_to_cache(this, stream->virtual_name(), *object);
				if (existing != nullptr)
					object = existing;

				object->add_ref();
				return object.reset();
			}
			expects_content<void> material_processor::serialize(core::stream* stream, void* instance, const core::variant_args& args)
			{
				VI_ASSERT(stream != nullptr, "stream should be set");
				VI_ASSERT(instance != nullptr, "instance should be set");

				layer::material* object = (layer::material*)instance;
				core::uptr<core::schema> data = core::var::set::object();
				data->key = "material";

				asset_cache* asset = content->find_cache<graphics::texture_2d>(object->get_diffuse_map());
				if (asset != nullptr)
					series::pack(data->set("diffuse-map"), asset->path);

				asset = content->find_cache<graphics::texture_2d>(object->get_normal_map());
				if (asset != nullptr)
					series::pack(data->set("normal-map"), asset->path);

				asset = content->find_cache<graphics::texture_2d>(object->get_metallic_map());
				if (asset != nullptr)
					series::pack(data->set("metallic-map"), asset->path);

				asset = content->find_cache<graphics::texture_2d>(object->get_roughness_map());
				if (asset != nullptr)
					series::pack(data->set("roughness-map"), asset->path);

				asset = content->find_cache<graphics::texture_2d>(object->get_height_map());
				if (asset != nullptr)
					series::pack(data->set("height-map"), asset->path);

				asset = content->find_cache<graphics::texture_2d>(object->get_occlusion_map());
				if (asset != nullptr)
					series::pack(data->set("occlusion-map"), asset->path);

				asset = content->find_cache<graphics::texture_2d>(object->get_emission_map());
				if (asset != nullptr)
					series::pack(data->set("emission-map"), asset->path);

				heavy_series::pack(data->set("emission"), object->surface.emission);
				heavy_series::pack(data->set("metallic"), object->surface.metallic);
				heavy_series::pack(data->set("penetration"), object->surface.penetration);
				heavy_series::pack(data->set("diffuse"), object->surface.diffuse);
				heavy_series::pack(data->set("scattering"), object->surface.scattering);
				heavy_series::pack(data->set("roughness"), object->surface.roughness);
				heavy_series::pack(data->set("occlusion"), object->surface.occlusion);
				series::pack(data->set("fresnel"), object->surface.fresnel);
				series::pack(data->set("refraction"), object->surface.refraction);
				series::pack(data->set("transparency"), object->surface.transparency);
				series::pack(data->set("environment"), object->surface.environment);
				series::pack(data->set("radius"), object->surface.radius);
				series::pack(data->set("height"), object->surface.height);
				series::pack(data->set("bias"), object->surface.bias);
				series::pack(data->set("name"), object->get_name());
				return content->save<core::schema>(stream->virtual_name(), *data, args);
			}
			void material_processor::free(asset_cache* asset)
			{
				VI_ASSERT(asset != nullptr, "asset should be set");
				auto* value = (layer::material*)asset->resource;
				asset->resource = nullptr;
				core::memory::release(value);
			}

			scene_graph_processor::scene_graph_processor(content_manager* manager) : processor(manager)
			{
			}
			expects_content<void*> scene_graph_processor::deserialize(core::stream* stream, size_t offset, const core::variant_args& args)
			{
				layer::scene_graph::desc i = layer::scene_graph::desc::get(heavy_application::has_instance() ? heavy_application::get() : nullptr);
				VI_ASSERT(stream != nullptr, "stream should be set");
				VI_ASSERT(i.shared.device != nullptr, "graphics device should be set");

				auto blob_status = content->load<core::schema>(stream->virtual_name());
				if (!blob_status)
					return blob_status.error();

				core::uptr<core::schema> blob = *blob_status;
				core::schema* metadata = blob->find("metadata");
				if (metadata != nullptr)
				{
					core::schema* simulator = metadata->find("simulator");
					if (simulator != nullptr)
					{
						series::unpack(simulator->find("enable-soft-body"), &i.simulator.enable_soft_body);
						series::unpack(simulator->find("max-displacement"), &i.simulator.max_displacement);
						series::unpack(simulator->find("air-density"), &i.simulator.air_density);
						series::unpack(simulator->find("water-offset"), &i.simulator.water_offset);
						series::unpack(simulator->find("water-density"), &i.simulator.water_density);
						heavy_series::unpack(simulator->find("water-normal"), &i.simulator.water_normal);
						heavy_series::unpack(simulator->find("gravity"), &i.simulator.gravity);
					}

					series::unpack_a(metadata->find("materials"), &i.start_materials);
					series::unpack_a(metadata->find("entities"), &i.start_entities);
					series::unpack_a(metadata->find("components"), &i.start_components);
					series::unpack(metadata->find("render-quality"), &i.render_quality);
					series::unpack(metadata->find("enable-hdr"), &i.enable_hdr);
					series::unpack_a(metadata->find("grow-margin"), &i.grow_margin);
					series::unpack(metadata->find("grow-rate"), &i.grow_rate);
					series::unpack_a(metadata->find("max-updates"), &i.max_updates);
					series::unpack_a(metadata->find("points-size"), &i.points_size);
					series::unpack_a(metadata->find("points-max"), &i.points_max);
					series::unpack_a(metadata->find("spots-size"), &i.spots_size);
					series::unpack_a(metadata->find("spots-max"), &i.spots_max);
					series::unpack_a(metadata->find("line-size"), &i.lines_size);
					series::unpack_a(metadata->find("lines-max"), &i.lines_max);
				}

				bool integrity_check = false;
				auto ensure_integrity = args.find("integrity");
				if (ensure_integrity != args.end())
					integrity_check = ensure_integrity->second.get_boolean();

				auto has_mutations = args.find("mutations");
				if (has_mutations != args.end())
					i.mutations = has_mutations->second.get_boolean();

				layer::scene_graph* object = new layer::scene_graph(i);
				layer::idx_snapshot snapshot;
				object->snapshot = &snapshot;
				if (setup_callback)
					setup_callback(object);

				auto is_active = args.find("active");
				if (is_active != args.end())
					object->set_active(is_active->second.get_boolean());

				core::schema* materials = blob->find("materials");
				if (materials != nullptr)
				{
					core::vector<core::schema*> collection = materials->find_collection("material");
					for (auto& it : collection)
					{
						core::string path;
						if (!series::unpack(it, &path) || path.empty())
							continue;

						auto value = content->load<layer::material>(path);
						if (value)
						{
							series::unpack_a(it, &value->slot);
							object->add_material(*value);
						}
						else if (integrity_check)
							return value.error();
					}
				}

				core::schema* entities = blob->find("entities");
				if (entities != nullptr)
				{
					core::vector<core::schema*> collection = entities->find_collection("entity");
					for (auto& it : collection)
					{
						entity* entity = object->add_entity();
						int64_t refer = -1;

						if (series::unpack(it->find("refer"), &refer) && refer >= 0)
						{
							snapshot.to[entity] = (size_t)refer;
							snapshot.from[(size_t)refer] = entity;
						}
					}

					size_t next = 0;
					for (auto& it : collection)
					{
						entity* entity = object->get_entity(next++);
						if (!entity)
							continue;

						core::string name;
						series::unpack(it->find("name"), &name);
						entity->set_name(name);

						core::schema* transform = it->find("transform");
						if (transform != nullptr)
						{
							trigonometry::transform* offset = entity->get_transform();
							trigonometry::transform::spacing& space = offset->get_spacing(trigonometry::positioning::global);
							bool scaling = offset->has_scaling();
							heavy_series::unpack(transform->find("position"), &space.position);
							heavy_series::unpack(transform->find("rotation"), &space.rotation);
							heavy_series::unpack(transform->find("scale"), &space.scale);
							series::unpack(transform->find("scaling"), &scaling);
							offset->set_scaling(scaling);
						}

						core::schema* parent = it->find("parent");
						if (parent != nullptr)
						{
							trigonometry::transform* root = nullptr;
							trigonometry::transform::spacing* space = core::memory::init<trigonometry::transform::spacing>();
							heavy_series::unpack(parent->find("position"), &space->position);
							heavy_series::unpack(parent->find("rotation"), &space->rotation);
							heavy_series::unpack(parent->find("scale"), &space->scale);
							heavy_series::unpack(parent->find("world"), &space->offset);

							size_t where = 0;
							if (series::unpack_a(parent->find("where"), &where))
							{
								auto it = snapshot.from.find(where);
								if (it != snapshot.from.end() && it->second != entity)
									root = it->second->get_transform();
							}

							trigonometry::transform* offset = entity->get_transform();
							offset->set_pivot(root, space);
							offset->make_dirty();
						}

						core::schema* components = it->find("components");
						if (components != nullptr)
						{
							core::vector<core::schema*> elements = components->find_collection("component");
							for (auto& element : elements)
							{
								uint64_t id;
								if (!series::unpack(element->find("id"), &id))
									continue;

								component* target = core::composer::create<component>(id, entity);
								if (!entity->add_component(target))
									continue;

								bool active = true;
								if (series::unpack(element->find("active"), &active))
									target->set_active(active);

								core::schema* meta = element->find("metadata");
								if (!meta)
									meta = element->set("metadata");
								target->deserialize(meta);
							}
						}
					}
				}

				object->snapshot = nullptr;
				object->actualize();
				return object;
			}
			expects_content<void> scene_graph_processor::serialize(core::stream* stream, void* instance, const core::variant_args& args)
			{
				VI_ASSERT(stream != nullptr, "stream should be set");
				VI_ASSERT(instance != nullptr, "instance should be set");

				auto ext = core::os::path::get_extension(stream->virtual_name());
				if (ext.empty())
				{
					auto type = args.find("type");
					if (type->second == core::var::string("XML"))
						ext = ".xml";
					else if (type->second == core::var::string("JSON"))
						ext = ".json";
					else if (type->second == core::var::string("JSONB"))
						ext = ".jsonb";
					else
						ext = ".xml";
				}

				layer::scene_graph* object = (layer::scene_graph*)instance;
				if (setup_callback)
					setup_callback(object);
				object->actualize();

				layer::idx_snapshot snapshot;
				object->make_snapshot(&snapshot);
				object->snapshot = &snapshot;

				core::uptr<core::schema> blob = core::var::set::object();
				blob->key = "scene";

				auto& conf = object->get_conf();
				core::schema* metadata = blob->set("metadata");
				series::pack(metadata->set("materials"), (uint64_t)conf.start_materials);
				series::pack(metadata->set("entities"), (uint64_t)conf.start_entities);
				series::pack(metadata->set("components"), (uint64_t)conf.start_components);
				series::pack(metadata->set("render-quality"), conf.render_quality);
				series::pack(metadata->set("enable-hdr"), conf.enable_hdr);
				series::pack(metadata->set("grow-margin"), (uint64_t)conf.grow_margin);
				series::pack(metadata->set("grow-rate"), conf.grow_rate);
				series::pack(metadata->set("max-updates"), (uint64_t)conf.max_updates);
				series::pack(metadata->set("points-size"), (uint64_t)conf.points_size);
				series::pack(metadata->set("points-max"), (uint64_t)conf.points_max);
				series::pack(metadata->set("spots-size"), (uint64_t)conf.spots_size);
				series::pack(metadata->set("spots-max"), (uint64_t)conf.spots_max);
				series::pack(metadata->set("line-size"), (uint64_t)conf.lines_size);
				series::pack(metadata->set("lines-max"), (uint64_t)conf.lines_max);

				auto* fSimulator = object->get_simulator();
				core::schema* simulator = metadata->set("simulator");
				series::pack(simulator->set("enable-soft-body"), fSimulator->has_soft_body_support());
				series::pack(simulator->set("max-displacement"), fSimulator->get_max_displacement());
				series::pack(simulator->set("air-density"), fSimulator->get_air_density());
				series::pack(simulator->set("water-offset"), fSimulator->get_water_offset());
				series::pack(simulator->set("water-density"), fSimulator->get_water_density());
				heavy_series::pack(simulator->set("water-normal"), fSimulator->get_water_normal());
				heavy_series::pack(simulator->set("gravity"), fSimulator->get_gravity());

				core::schema* materials = blob->set("materials", core::var::array());
				for (size_t i = 0; i < object->get_materials_count(); i++)
				{
					layer::material* material = object->get_material(i);
					if (!material || material == object->get_invalid_material())
						continue;

					core::string path;
					asset_cache* asset = content->find_cache<layer::material>(material);
					if (!asset)
						path.assign("./materials/" + material->get_name() + ".modified");
					else
						path.assign(asset->path);

					if (!core::stringify::ends_with(path, ext))
						path.append(ext);

					if (content->save<layer::material>(path, material, args))
					{
						core::schema* where = materials->set("material");
						series::pack(where, (uint64_t)material->slot);
						series::pack(where, path);
					}
				}

				core::schema* entities = blob->set("entities", core::var::array());
				for (size_t i = 0; i < object->get_entities_count(); i++)
				{
					entity* ref = object->get_entity(i);
					auto* offset = ref->get_transform();

					core::schema* entity = entities->set("entity");
					series::pack(entity->set("name"), ref->get_name());
					series::pack(entity->set("refer"), (uint64_t)i);

					core::schema* transform = entity->set("transform");
					heavy_series::pack(transform->set("position"), offset->get_position());
					heavy_series::pack(transform->set("rotation"), offset->get_rotation());
					heavy_series::pack(transform->set("scale"), offset->get_scale());
					series::pack(transform->set("scaling"), offset->has_scaling());

					if (offset->get_root() != nullptr)
					{
						core::schema* parent = entity->set("parent");
						if (offset->get_root()->user_data != nullptr)
						{
							auto it = snapshot.to.find((layer::entity*)offset->get_root());
							if (it != snapshot.to.end())
								series::pack(parent->set("where"), (uint64_t)it->second);
						}

						trigonometry::transform::spacing& space = offset->get_spacing();
						heavy_series::pack(parent->set("position"), space.position);
						heavy_series::pack(parent->set("rotation"), space.rotation);
						heavy_series::pack(parent->set("scale"), space.scale);
						heavy_series::pack(parent->set("world"), space.offset);
					}

					if (!ref->get_components_count())
						continue;

					core::schema* components = entity->set("components", core::var::array());
					for (auto& item : *ref)
					{
						core::schema* component = components->set("component");
						series::pack(component->set("id"), item.second->get_id());
						series::pack(component->set("active"), item.second->is_active());
						item.second->serialize(component->set("metadata"));
					}
				}

				object->snapshot = nullptr;
				return content->save<core::schema>(stream->virtual_name(), *blob, args);
			}

			audio_clip_processor::audio_clip_processor(content_manager* manager) : processor(manager)
			{
			}
			audio_clip_processor::~audio_clip_processor()
			{
			}
			expects_content<void*> audio_clip_processor::duplicate(asset_cache* asset, const core::variant_args& args)
			{
				VI_ASSERT(asset != nullptr, "asset should be set");
				VI_ASSERT(asset->resource != nullptr, "asset resource should be set");

				((audio::audio_clip*)asset->resource)->add_ref();
				return asset->resource;
			}
			expects_content<void*> audio_clip_processor::deserialize(core::stream* stream, size_t offset, const core::variant_args& args)
			{
				if (core::stringify::ends_with(stream->virtual_name(), ".wav"))
					return deserialize_wave(stream, offset, args);
				else if (core::stringify::ends_with(stream->virtual_name(), ".ogg"))
					return deserialize_ogg(stream, offset, args);

				return content_exception("deserialize audio unsupported: " + core::string(core::os::path::get_extension(stream->virtual_name())));
			}
			expects_content<void*> audio_clip_processor::deserialize_wave(core::stream* stream, size_t offset, const core::variant_args& args)
			{
				VI_ASSERT(stream != nullptr, "stream should be set");
#ifdef VI_SDL2
				core::vector<char> data;
				stream->read_all([&data](uint8_t* buffer, size_t size)
				{
					data.reserve(data.size() + size);
					for (size_t i = 0; i < size; i++)
						data.push_back(buffer[i]);
				});

				SDL_RWops* wav_data = SDL_RWFromMem(data.data(), (int)data.size());
				SDL_AudioSpec wav_info;
				Uint8* wav_samples;
				Uint32 wav_count;

				if (!SDL_LoadWAV_RW(wav_data, 1, &wav_info, &wav_samples, &wav_count))
				{
					SDL_RWclose(wav_data);
					return content_exception(std::move(graphics::video_exception().message()));
				}

				int format = 0;
#ifdef VI_OPENAL
				switch (wav_info.format)
				{
					case AUDIO_U8:
					case AUDIO_S8:
						format = wav_info.channels == 2 ? AL_FORMAT_STEREO8 : AL_FORMAT_MONO8;
						break;
					case AUDIO_U16:
					case AUDIO_S16:
						format = wav_info.channels == 2 ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16;
						break;
					default:
						SDL_FreeWAV(wav_samples);
						SDL_RWclose(wav_data);
						return content_exception("load wave audio: unsupported audio format");
				}
#endif
				core::uptr<audio::audio_clip> object = new audio::audio_clip(1, format);
				audio::audio_context::set_buffer_data(object->get_buffer(), (int)format, (const void*)wav_samples, (int)wav_count, (int)wav_info.freq);
				SDL_FreeWAV(wav_samples);
				SDL_RWclose(wav_data);

				auto* existing = (audio::audio_clip*)content->try_to_cache(this, stream->virtual_name(), *object);
				if (existing != nullptr)
					object = existing;

				object->add_ref();
				return object.reset();
#else
				return content_exception("load wave audio: unsupported");
#endif
			}
			expects_content<void*> audio_clip_processor::deserialize_ogg(core::stream* stream, size_t offset, const core::variant_args& args)
			{
#ifdef VI_STB
				VI_ASSERT(stream != nullptr, "stream should be set");
				core::vector<char> data;
				stream->read_all([&data](uint8_t* buffer, size_t size)
				{
					data.reserve(data.size() + size);
					for (size_t i = 0; i < size; i++)
						data.push_back(buffer[i]);
				});

				short* buffer;
				int channels, sample_rate;
				int samples = stb_vorbis_decode_memory((const uint8_t*)data.data(), (int)data.size(), &channels, &sample_rate, &buffer);
				if (samples <= 0)
					return content_exception("load ogg audio: invalid file");

				int format = 0;
#ifdef VI_OPENAL
				if (channels == 2)
					format = AL_FORMAT_STEREO16;
				else
					format = AL_FORMAT_MONO16;
#endif
				core::uptr<audio::audio_clip> object = new audio::audio_clip(1, format);
				audio::audio_context::set_buffer_data(object->get_buffer(), (int)format, (const void*)buffer, samples * sizeof(short) * channels, (int)sample_rate);
				core::memory::deallocate(buffer);

				auto* existing = (audio::audio_clip*)content->try_to_cache(this, stream->virtual_name(), *object);
				if (existing != nullptr)
					object = existing;

				object->add_ref();
				return object.reset();
#else
				return content_exception("load ogg audio: unsupported");
#endif
			}
			void audio_clip_processor::free(asset_cache* asset)
			{
				VI_ASSERT(asset != nullptr, "asset should be set");
				auto* value = (audio::audio_clip*)asset->resource;
				core::memory::release(value);
				asset->resource = nullptr;
			}

			texture_2d_processor::texture_2d_processor(content_manager* manager) : processor(manager)
			{
			}
			texture_2d_processor::~texture_2d_processor()
			{
			}
			expects_content<void*> texture_2d_processor::duplicate(asset_cache* asset, const core::variant_args& args)
			{
				VI_ASSERT(asset != nullptr, "asset should be set");
				VI_ASSERT(asset->resource != nullptr, "instance should be set");

				((graphics::texture_2d*)asset->resource)->add_ref();
				return asset->resource;
			}
			expects_content<void*> texture_2d_processor::deserialize(core::stream* stream, size_t offset, const core::variant_args& args)
			{
#ifdef VI_STB
				VI_ASSERT(stream != nullptr, "stream should be set");
				core::vector<char> data;
				stream->read_all([&data](uint8_t* buffer, size_t size)
				{
					data.reserve(data.size() + size);
					for (size_t i = 0; i < size; i++)
						data.push_back(buffer[i]);
				});

				int width, height, channels;
				uint8_t* resource = stbi_load_from_memory((const uint8_t*)data.data(), (int)data.size(), &width, &height, &channels, STBI_rgb_alpha);
				if (!resource)
					return content_exception("load texture 2d: invalid file");

				auto* heavy_content = (heavy_content_manager*)content;
				auto* device = heavy_content->get_device();
				graphics::texture_2d::desc i = graphics::texture_2d::desc();
				i.data = (void*)resource;
				i.width = (uint32_t)width;
				i.height = (uint32_t)height;
				i.row_pitch = device->get_row_pitch(i.width);
				i.depth_pitch = device->get_depth_pitch(i.row_pitch, i.height);
				i.mip_levels = device->get_mip_level(i.width, i.height);

				auto object_status = process_renderer_job<graphics::expects_graphics<graphics::texture_2d*>>(device, [&i](graphics::graphics_device* device) { return device->create_texture_2d(i); });
				stbi_image_free(resource);
				if (!object_status)
					return content_exception(std::move(object_status.error().message()));

				core::uptr<graphics::texture_2d> object = *object_status;
				auto* existing = (graphics::texture_2d*)content->try_to_cache(this, stream->virtual_name(), *object);
				if (existing != nullptr)
					object = existing;

				object->add_ref();
				return object.reset();
#else
				return content_exception("load texture 2d: unsupported");
#endif
			}
			void texture_2d_processor::free(asset_cache* asset)
			{
				VI_ASSERT(asset != nullptr, "asset should be set");
				auto* value = (graphics::texture_2d*)asset->resource;
				asset->resource = nullptr;
				core::memory::release(value);
			}

			shader_processor::shader_processor(content_manager* manager) : processor(manager)
			{
			}
			shader_processor::~shader_processor()
			{
			}
			expects_content<void*> shader_processor::duplicate(asset_cache* asset, const core::variant_args& args)
			{
				VI_ASSERT(asset != nullptr, "asset should be set");
				VI_ASSERT(asset->resource != nullptr, "instance should be set");

				((graphics::shader*)asset->resource)->add_ref();
				return asset->resource;
			}
			expects_content<void*> shader_processor::deserialize(core::stream* stream, size_t offset, const core::variant_args& args)
			{
				VI_ASSERT(stream != nullptr, "stream should be set");
				core::string data;
				stream->read_all([&data](uint8_t* buffer, size_t size) { data.append((char*)buffer, size); });

				graphics::shader::desc i = graphics::shader::desc();
				i.filename = stream->virtual_name();
				i.data = data;

				auto* heavy_content = (heavy_content_manager*)content;
				auto* device = heavy_content->get_device();
				auto object_status = process_renderer_job<graphics::expects_graphics<graphics::shader*>>(device, [&i](graphics::graphics_device* device) { return device->create_shader(i); });
				if (!object_status)
					return content_exception(std::move(object_status.error().message()));

				core::uptr<graphics::shader> object = *object_status;
				auto* existing = (graphics::shader*)content->try_to_cache(this, stream->virtual_name(), *object);
				if (existing != nullptr)
					object = existing;

				object->add_ref();
				return object.reset();
			}
			void shader_processor::free(asset_cache* asset)
			{
				VI_ASSERT(asset != nullptr, "asset should be set");
				auto* value = (graphics::shader*)asset->resource;
				asset->resource = nullptr;
				core::memory::release(value);
			}

			model_processor::model_processor(content_manager* manager) : processor(manager)
			{
			}
			model_processor::~model_processor()
			{
			}
			expects_content<void*> model_processor::duplicate(asset_cache* asset, const core::variant_args& args)
			{
				VI_ASSERT(asset != nullptr, "asset should be set");
				VI_ASSERT(asset->resource != nullptr, "instance should be set");

				((model*)asset->resource)->add_ref();
				return asset->resource;
			}
			expects_content<void*> model_processor::deserialize(core::stream* stream, size_t offset, const core::variant_args& args)
			{
				VI_ASSERT(stream != nullptr, "stream should be set");
				core::uptr<model> object = new model();
				auto path = stream->virtual_name();
				if (core::stringify::ends_with(path, ".xml") || core::stringify::ends_with(path, ".json") || core::stringify::ends_with(path, ".jsonb") || core::stringify::ends_with(path, ".xml.gz") || core::stringify::ends_with(path, ".json.gz") || core::stringify::ends_with(path, ".jsonb.gz"))
				{
					auto data_status = content->load<core::schema>(path);
					if (!data_status)
						return data_status.error();

					core::uptr<core::schema> data = *data_status;
					heavy_series::unpack(data->get("min"), &object->min);
					heavy_series::unpack(data->get("max"), &object->max);

					auto* meshes = data->get("meshes");
					if (meshes != nullptr)
					{
						object->meshes.reserve(meshes->size());
						for (auto& mesh : meshes->get_childs())
						{
							graphics::mesh_buffer::desc i;
							i.access_flags = options.access_flags;
							i.usage = options.usage;

							if (!series::unpack(mesh->get("indices"), &i.indices))
								return content_exception("import model: invalid indices");

							if (!heavy_series::unpack(mesh->get("vertices"), &i.elements))
								return content_exception("import model: invalid vertices");

							auto* heavy_content = (heavy_content_manager*)content;
							auto* device = heavy_content->get_device();
							auto new_buffer = process_renderer_job<graphics::expects_graphics<graphics::mesh_buffer*>>(device, [&i](graphics::graphics_device* device) { return device->create_mesh_buffer(i); });
							if (!new_buffer)
								return content_exception(std::move(new_buffer.error().message()));

							object->meshes.emplace_back(*new_buffer);
							series::unpack(mesh->get("name"), &new_buffer->name);
							heavy_series::unpack(mesh->get("transform"), &new_buffer->transform);
						}
					}
				}
				else
				{
					auto data = import_for_immediate_use(stream);
					if (!data)
						return data.error();

					object->meshes.reserve(data->meshes.size());
					object->min = data->min;
					object->max = data->max;
					for (auto& mesh : data->meshes)
					{
						graphics::mesh_buffer::desc i;
						i.access_flags = options.access_flags;
						i.usage = options.usage;
						i.indices = std::move(mesh.indices);
						i.elements = skin_vertices_to_vertices(mesh.vertices);

						auto* heavy_content = (heavy_content_manager*)content;
						auto* device = heavy_content->get_device();
						auto new_buffer = process_renderer_job<graphics::expects_graphics<graphics::mesh_buffer*>>(device, [&i](graphics::graphics_device* device) { return device->create_mesh_buffer(i); });
						if (!new_buffer)
							return content_exception(std::move(new_buffer.error().message()));

						object->meshes.emplace_back(*new_buffer);
						new_buffer->name = mesh.name;
						new_buffer->transform = mesh.transform;
					}
				}

				auto* existing = (model*)content->try_to_cache(this, stream->virtual_name(), *object);
				if (existing != nullptr)
					object = existing;

				object->add_ref();
				return object.reset();
			}
			expects_content<core::schema*> model_processor::import(core::stream * stream, uint64_t opts)
			{
				auto info = import_for_immediate_use(stream, opts);
				if (!info || (info->meshes.empty() && info->joint_offsets.empty()))
				{
					if (!info)
						return info.error();

					return content_exception("import model: no mesh data");
				}

				auto* blob = core::var::set::object();
				blob->key = "model";

				series::pack(blob->set("options"), opts);
				heavy_series::pack(blob->set("inv-transform"), info->transform);
				heavy_series::pack(blob->set("min"), info->min.xyzw().set_w(info->low));
				heavy_series::pack(blob->set("max"), info->max.xyzw().set_w(info->high));
				heavy_series::pack(blob->set("skeleton"), info->skeleton);

				core::schema* meshes = blob->set("meshes", core::var::array());
				for (auto&& it : info->meshes)
				{
					core::schema* mesh = meshes->set("mesh");
					series::pack(mesh->set("name"), it.name);
					heavy_series::pack(mesh->set("transform"), it.transform);
					heavy_series::pack(mesh->set("vertices"), it.vertices);
					series::pack(mesh->set("indices"), it.indices);
					series::pack(mesh->set("joints"), it.joint_indices);
				}

				return blob;
			}
			expects_content<model_info> model_processor::import_for_immediate_use(core::stream* stream, uint64_t opts)
			{
#ifdef VI_ASSIMP
				core::vector<char> data;
				stream->read_all([&data](uint8_t* buffer, size_t size)
				{
					data.reserve(data.size() + size);
					for (size_t i = 0; i < size; i++)
						data.push_back(buffer[i]);
				});

				Assimp::Importer importer;
				auto* scene = importer.ReadFileFromMemory(data.data(), data.size(), (uint32_t)opts, core::os::path::get_extension(stream->virtual_name()).data());
				if (!scene)
					return content_exception(core::stringify::text("import model: %s", importer.GetErrorString()));

				model_info info;
				fill_scene_geometries(&info, scene, scene->mRootNode, scene->mRootNode->mTransformation);
				fill_scene_skeletons(&info, scene);
				return info;
#else
				return content_exception("import model: unsupported");
#endif
			}
			void model_processor::free(asset_cache* asset)
			{
				VI_ASSERT(asset != nullptr, "asset should be set");
				auto* value = (model*)asset->resource;
				asset->resource = nullptr;
				core::memory::release(value);
			}

			skin_model_processor::skin_model_processor(content_manager* manager) : processor(manager)
			{
			}
			skin_model_processor::~skin_model_processor()
			{
			}
			expects_content<void*> skin_model_processor::duplicate(asset_cache* asset, const core::variant_args& args)
			{
				VI_ASSERT(asset != nullptr, "asset should be set");
				VI_ASSERT(asset->resource != nullptr, "instance should be set");

				((skin_model*)asset->resource)->add_ref();
				return asset->resource;
			}
			expects_content<void*> skin_model_processor::deserialize(core::stream* stream, size_t offset, const core::variant_args& args)
			{
				VI_ASSERT(stream != nullptr, "stream should be set");
				core::uptr<skin_model> object = new skin_model();
				auto path = stream->virtual_name();
				if (core::stringify::ends_with(path, ".xml") || core::stringify::ends_with(path, ".json") || core::stringify::ends_with(path, ".jsonb") || core::stringify::ends_with(path, ".xml.gz") || core::stringify::ends_with(path, ".json.gz") || core::stringify::ends_with(path, ".jsonb.gz"))
				{
					auto data_status = content->load<core::schema>(path);
					if (!data_status)
						return data_status.error();

					core::uptr<core::schema> data = *data_status;
					heavy_series::unpack(data->get("inv-transform"), &object->inv_transform);
					heavy_series::unpack(data->get("min"), &object->min);
					heavy_series::unpack(data->get("max"), &object->max);
					heavy_series::unpack(data->get("skeleton"), &object->skeleton);
					object->transform = object->inv_transform.inv();

					auto* meshes = data->get("meshes");
					if (meshes != nullptr)
					{
						object->meshes.reserve(meshes->size());
						for (auto& mesh : meshes->get_childs())
						{
							graphics::skin_mesh_buffer::desc i;
							i.access_flags = options.access_flags;
							i.usage = options.usage;

							if (!series::unpack(mesh->get("indices"), &i.indices))
								return content_exception("import model: invalid indices");

							if (!heavy_series::unpack(mesh->get("vertices"), &i.elements))
								return content_exception("import model: invalid vertices");

							auto* heavy_content = (heavy_content_manager*)content;
							auto* device = heavy_content->get_device();
							auto new_buffer = process_renderer_job<graphics::expects_graphics<graphics::skin_mesh_buffer*>>(device, [&i](graphics::graphics_device* device) { return device->create_skin_mesh_buffer(i); });
							if (!new_buffer)
								return content_exception(std::move(new_buffer.error().message()));

							object->meshes.emplace_back(*new_buffer);
							series::unpack(mesh->get("name"), &new_buffer->name);
							heavy_series::unpack(mesh->get("transform"), &new_buffer->transform);
							series::unpack(mesh->get("joints"), &new_buffer->joints);
						}
					}
				}
				else
				{
					auto data = model_processor::import_for_immediate_use(stream);
					if (!data)
						return data.error();

					object = new skin_model();
					object->meshes.reserve(data->meshes.size());
					object->inv_transform = data->transform;
					object->min = data->min;
					object->max = data->max;
					object->skeleton = std::move(data->skeleton);

					for (auto& mesh : data->meshes)
					{
						graphics::skin_mesh_buffer::desc i;
						i.access_flags = options.access_flags;
						i.usage = options.usage;
						i.indices = std::move(mesh.indices);
						i.elements = std::move(mesh.vertices);

						auto* heavy_content = (heavy_content_manager*)content;
						auto* device = heavy_content->get_device();
						auto new_buffer = process_renderer_job<graphics::expects_graphics<graphics::skin_mesh_buffer*>>(device, [&i](graphics::graphics_device* device) { return device->create_skin_mesh_buffer(i); });
						if (!new_buffer)
							return content_exception(std::move(new_buffer.error().message()));

						object->meshes.emplace_back(*new_buffer);
						new_buffer->name = mesh.name;
						new_buffer->transform = mesh.transform;
						new_buffer->joints = std::move(mesh.joint_indices);
					}
				}

				auto* existing = (skin_model*)content->try_to_cache(this, stream->virtual_name(), *object);
				if (existing != nullptr)
					object = existing;

				object->add_ref();
				return object.reset();
			}
			void skin_model_processor::free(asset_cache* asset)
			{
				VI_ASSERT(asset != nullptr, "asset should be set");
				auto* value = (skin_model*)asset->resource;
				asset->resource = nullptr;
				core::memory::release(value);
			}

			skin_animation_processor::skin_animation_processor(content_manager* manager) : processor(manager)
			{
			}
			skin_animation_processor::~skin_animation_processor()
			{
			}
			expects_content<void*> skin_animation_processor::duplicate(asset_cache* asset, const core::variant_args& args)
			{
				VI_ASSERT(asset != nullptr, "asset should be set");
				VI_ASSERT(asset->resource != nullptr, "instance should be set");

				((layer::skin_animation*)asset->resource)->add_ref();
				return asset->resource;
			}
			expects_content<void*> skin_animation_processor::deserialize(core::stream* stream, size_t offset, const core::variant_args& args)
			{
				VI_ASSERT(stream != nullptr, "stream should be set");
				core::vector<trigonometry::skin_animator_clip> clips;
				auto path = stream->virtual_name();
				if (core::stringify::ends_with(path, ".xml") || core::stringify::ends_with(path, ".json") || core::stringify::ends_with(path, ".jsonb") || core::stringify::ends_with(path, ".xml.gz") || core::stringify::ends_with(path, ".json.gz") || core::stringify::ends_with(path, ".jsonb.gz"))
				{
					auto data_status = content->load<core::schema>(path);
					if (!data_status)
						return data_status.error();

					core::uptr<core::schema> data = *data_status;
					clips.reserve(data->size());
					for (auto& item : data->get_childs())
					{
						clips.emplace_back();
						auto& clip = clips.back();
						series::unpack(item->get("name"), &clip.name);
						series::unpack(item->get("duration"), &clip.duration);
						series::unpack(item->get("rate"), &clip.rate);

						auto* keys = item->get("keys");
						if (keys != nullptr)
						{
							clip.keys.reserve(keys->size());
							for (auto& key : keys->get_childs())
							{
								clip.keys.emplace_back();
								auto& pose = clip.keys.back();
								series::unpack(key, &pose.time);

								size_t array_offset = 0;
								for (auto& orientation : key->get_childs())
								{
									if (!array_offset++)
										continue;

									pose.pose.emplace_back();
									auto& value = pose.pose.back();
									heavy_series::unpack(orientation->get("position"), &value.position);
									heavy_series::unpack(orientation->get("scale"), &value.scale);
									heavy_series::unpack(orientation->get("rotation"), &value.rotation);
									series::unpack(orientation->get("time"), &value.time);
								}
							}
						}
					}
				}
				else
				{
					auto new_clips = import_for_immediate_use(stream);
					if (!new_clips)
						return new_clips.error();

					clips = std::move(*new_clips);
				}

				if (clips.empty())
					return content_exception("load animation: no clips");

				core::uptr<layer::skin_animation> object = new layer::skin_animation(std::move(clips));
				auto* existing = (layer::skin_animation*)content->try_to_cache(this, stream->virtual_name(), *object);
				if (existing != nullptr)
					object = existing;

				object->add_ref();
				return object.reset();
			}
			expects_content<core::schema*> skin_animation_processor::import(core::stream * stream, uint64_t opts)
			{
				auto info = import_for_immediate_use(stream, opts);
				if (!info)
					return info.error();

				auto* blob = core::var::set::array();
				blob->key = "animation";

				for (auto& clip : *info)
				{
					auto* item = blob->push(core::var::set::object());
					series::pack(item->set("name"), clip.name);
					series::pack(item->set("duration"), clip.duration);
					series::pack(item->set("rate"), clip.rate);

					auto* keys = item->set("keys", core::var::set::array());
					keys->reserve(clip.keys.size());

					for (auto& key : clip.keys)
					{
						auto* pose = keys->set("pose", core::var::set::array());
						pose->reserve(key.pose.size() + 1);
						series::pack(pose, key.time);

						for (auto& orientation : key.pose)
						{
							auto* value = pose->set("key", core::var::set::object());
							heavy_series::pack(value->set("position"), orientation.position);
							heavy_series::pack(value->set("scale"), orientation.scale);
							heavy_series::pack(value->set("rotation"), orientation.rotation);
							series::pack(value->set("time"), orientation.time);
						}
					}
				}

				return blob;
			}
			expects_content<core::vector<trigonometry::skin_animator_clip>> skin_animation_processor::import_for_immediate_use(core::stream* stream, uint64_t opts)
			{
#ifdef VI_ASSIMP
				core::vector<char> data;
				stream->read_all([&data](uint8_t* buffer, size_t size)
				{
					data.reserve(data.size() + size);
					for (size_t i = 0; i < size; i++)
						data.push_back(buffer[i]);
				});

				Assimp::Importer importer;
				auto* scene = importer.ReadFileFromMemory(data.data(), data.size(), (uint32_t)opts, core::os::path::get_extension(stream->virtual_name()).data());
				if (!scene)
					return content_exception(core::stringify::text("import animation: %s", importer.GetErrorString()));

				core::vector<trigonometry::skin_animator_clip> info;
				fill_scene_animations(&info, scene);
				return info;
#else
				return content_exception("import animation: unsupported");
#endif
			}
			void skin_animation_processor::free(asset_cache* asset)
			{
				VI_ASSERT(asset != nullptr, "asset should be set");
				auto* value = (layer::skin_animation*)asset->resource;
				asset->resource = nullptr;
				core::memory::release(value);
			}

			hull_shape_processor::hull_shape_processor(content_manager* manager) : processor(manager)
			{
			}
			hull_shape_processor::~hull_shape_processor()
			{
			}
			expects_content<void*> hull_shape_processor::duplicate(asset_cache* asset, const core::variant_args& args)
			{
				VI_ASSERT(asset != nullptr, "asset should be set");
				VI_ASSERT(asset->resource != nullptr, "instance should be set");

				((physics::hull_shape*)asset->resource)->add_ref();
				return asset->resource;
			}
			expects_content<void*> hull_shape_processor::deserialize(core::stream* stream, size_t offset, const core::variant_args& args)
			{
				VI_ASSERT(stream != nullptr, "stream should be set");
				auto data_status = content->load<core::schema>(stream->virtual_name());
				if (!data_status)
					return data_status.error();

				core::uptr<core::schema> data = *data_status;
				core::vector<core::schema*> meshes = data->fetch_collection("meshes.mesh");
				core::vector<trigonometry::vertex> vertices;
				core::vector<int> indices;

				for (auto&& mesh : meshes)
				{
					if (!series::unpack(mesh->find("indices"), &indices))
						return content_exception("import shape: invalid indices");

					if (!heavy_series::unpack(mesh->find("vertices"), &vertices))
						return content_exception("import shape: invalid vertices");
				}

				core::uptr<physics::hull_shape> object = new physics::hull_shape(std::move(vertices), std::move(indices));
				if (!object->get_shape())
					return content_exception("import shape: invalid shape");

				auto* existing = (physics::hull_shape*)content->try_to_cache(this, stream->virtual_name(), *object);
				if (existing != nullptr)
					object = existing;

				object->add_ref();
				return object.reset();
			}
			void hull_shape_processor::free(asset_cache* asset)
			{
				VI_ASSERT(asset != nullptr, "asset should be set");
				auto* value = (physics::hull_shape*)asset->resource;
				asset->resource = nullptr;
				core::memory::release(value);
			}
		}
	}
}
