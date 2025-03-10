#include "layer.h"
#include "layer/components.h"
#include "layer/processors.h"
#include "layer/renderers.h"
#include "audio/effects.h"
#include "audio/filters.h"
#include "vengeance.h"
#include <vitex/network/http.h>
#include <sstream>
#define CONTENT_BLOCKED_WAIT_MS 50

namespace vitex
{
	namespace layer
	{
		ticker::ticker() noexcept : time(0.0f), delay(16.0f)
		{
		}
		bool ticker::tick_event(float elapsed_time)
		{
			if (elapsed_time - time > delay)
			{
				time = elapsed_time;
				return true;
			}

			return false;
		}
		float ticker::get_time()
		{
			return time;
		}

		event::event(const std::string_view& new_name) noexcept : name(new_name)
		{
		}
		event::event(const std::string_view& new_name, const core::variant_args& new_args) noexcept : name(new_name), args(new_args)
		{
		}
		event::event(const std::string_view& new_name, core::variant_args&& new_args) noexcept : name(new_name), args(std::move(new_args))
		{
		}
		event::event(const event& other) noexcept : name(other.name), args(other.args)
		{
		}
		event::event(event&& other) noexcept : name(std::move(other.name)), args(std::move(other.args))
		{
		}
		event& event::operator= (const event& other) noexcept
		{
			name = other.name;
			args = other.args;
			return *this;
		}
		event& event::operator= (event&& other) noexcept
		{
			name = std::move(other.name);
			args = std::move(other.args);
			return *this;
		}

		float animator_state::get_timeline(core::timer* timing)const
		{
			return compute::mathf::min(time + rate * timing->get_step(), get_seconds_duration());
		}
		float animator_state::get_seconds_duration() const
		{
			return duration / rate;
		}
		float animator_state::get_progress_total() const
		{
			return time / get_seconds_duration();
		}
		float animator_state::get_progress() const
		{
			return compute::mathf::min(time / get_seconds_duration(), 1.0f);
		}
		bool animator_state::is_playing() const
		{
			return !paused && frame >= 0 && clip >= 0;
		}

		void viewer::set(const trigonometry::matrix4x4& _View, const trigonometry::matrix4x4& _Projection, const trigonometry::vector3& _Position, float _Fov, float _Ratio, float _Near, float _Far, render_culling _Type)
		{
			set(_View, _Projection, _Position, -_View.rotation_euler(), _Fov, _Ratio, _Near, _Far, _Type);
		}
		void viewer::set(const trigonometry::matrix4x4& _View, const trigonometry::matrix4x4& _Projection, const trigonometry::vector3& _Position, const trigonometry::vector3& _Rotation, float _Fov, float _Ratio, float _Near, float _Far, render_culling _Type)
		{
			view = _View;
			projection = _Projection;
			view_projection = _View * _Projection;
			inv_view_projection = view_projection.inv();
			inv_position = _Position.inv();
			position = _Position;
			rotation = _Rotation;
			far_plane = (_Far < _Near ? 999999999.0f : _Far);
			near_plane = _Near;
			ratio = _Ratio;
			fov = _Fov;
			culling = _Type;
			cube_view_projection[0] = trigonometry::matrix4x4::create_look_at(trigonometry::cube_face::positive_x, position) * projection;
			cube_view_projection[1] = trigonometry::matrix4x4::create_look_at(trigonometry::cube_face::negative_x, position) * projection;
			cube_view_projection[2] = trigonometry::matrix4x4::create_look_at(trigonometry::cube_face::positive_y, position) * projection;
			cube_view_projection[3] = trigonometry::matrix4x4::create_look_at(trigonometry::cube_face::negative_y, position) * projection;
			cube_view_projection[4] = trigonometry::matrix4x4::create_look_at(trigonometry::cube_face::positive_z, position) * projection;
			cube_view_projection[5] = trigonometry::matrix4x4::create_look_at(trigonometry::cube_face::negative_z, position) * projection;
		}

		void heavy_series::pack(core::schema* v, const trigonometry::vector2& value)
		{
			VI_ASSERT(v != nullptr, "schema should be set");
			v->set_attribute("x", core::var::number(value.x));
			v->set_attribute("y", core::var::number(value.y));
		}
		void heavy_series::pack(core::schema* v, const trigonometry::vector3& value)
		{
			VI_ASSERT(v != nullptr, "schema should be set");
			v->set_attribute("x", core::var::number(value.x));
			v->set_attribute("y", core::var::number(value.y));
			v->set_attribute("z", core::var::number(value.z));
		}
		void heavy_series::pack(core::schema* v, const trigonometry::vector4& value)
		{
			VI_ASSERT(v != nullptr, "schema should be set");
			v->set_attribute("x", core::var::number(value.x));
			v->set_attribute("y", core::var::number(value.y));
			v->set_attribute("z", core::var::number(value.z));
			v->set_attribute("w", core::var::number(value.w));
		}
		void heavy_series::pack(core::schema* v, const trigonometry::quaternion& value)
		{
			VI_ASSERT(v != nullptr, "schema should be set");
			v->set_attribute("x", core::var::number(value.x));
			v->set_attribute("y", core::var::number(value.y));
			v->set_attribute("z", core::var::number(value.z));
			v->set_attribute("w", core::var::number(value.w));
		}
		void heavy_series::pack(core::schema* v, const trigonometry::matrix4x4& value)
		{
			VI_ASSERT(v != nullptr, "schema should be set");
			v->set_attribute("m11", core::var::number(value.row[0]));
			v->set_attribute("m12", core::var::number(value.row[1]));
			v->set_attribute("m13", core::var::number(value.row[2]));
			v->set_attribute("m14", core::var::number(value.row[3]));
			v->set_attribute("m21", core::var::number(value.row[4]));
			v->set_attribute("m22", core::var::number(value.row[5]));
			v->set_attribute("m23", core::var::number(value.row[6]));
			v->set_attribute("m24", core::var::number(value.row[7]));
			v->set_attribute("m31", core::var::number(value.row[8]));
			v->set_attribute("m32", core::var::number(value.row[9]));
			v->set_attribute("m33", core::var::number(value.row[10]));
			v->set_attribute("m34", core::var::number(value.row[11]));
			v->set_attribute("m41", core::var::number(value.row[12]));
			v->set_attribute("m42", core::var::number(value.row[13]));
			v->set_attribute("m43", core::var::number(value.row[14]));
			v->set_attribute("m44", core::var::number(value.row[15]));
		}
		void heavy_series::pack(core::schema* v, const attenuation& value)
		{
			VI_ASSERT(v != nullptr, "schema should be set");
			series::pack(v->set("radius"), value.radius);
			series::pack(v->set("c1"), value.C1);
			series::pack(v->set("c2"), value.C2);
		}
		void heavy_series::pack(core::schema* v, const animator_state& value)
		{
			VI_ASSERT(v != nullptr, "schema should be set");
			series::pack(v->set("looped"), value.looped);
			series::pack(v->set("paused"), value.paused);
			series::pack(v->set("blended"), value.blended);
			series::pack(v->set("clip"), value.clip);
			series::pack(v->set("frame"), value.frame);
			series::pack(v->set("rate"), value.rate);
			series::pack(v->set("duration"), value.duration);
			series::pack(v->set("time"), value.time);
		}
		void heavy_series::pack(core::schema* v, const spawner_properties& value)
		{
			VI_ASSERT(v != nullptr, "schema should be set");
			series::pack(v->set("iterations"), value.iterations);

			core::schema* angular = v->set("angular");
			series::pack(angular->set("intensity"), value.angular.intensity);
			series::pack(angular->set("accuracy"), value.angular.accuracy);
			series::pack(angular->set("min"), value.angular.min);
			series::pack(angular->set("max"), value.angular.max);

			core::schema* diffusion = v->set("diffusion");
			series::pack(diffusion->set("intensity"), value.diffusion.intensity);
			series::pack(diffusion->set("accuracy"), value.diffusion.accuracy);
			heavy_series::pack(diffusion->set("min"), value.diffusion.min);
			heavy_series::pack(diffusion->set("max"), value.diffusion.max);

			core::schema* noise = v->set("noise");
			series::pack(noise->set("intensity"), value.noise.intensity);
			series::pack(noise->set("accuracy"), value.noise.accuracy);
			heavy_series::pack(noise->set("min"), value.noise.min);
			heavy_series::pack(noise->set("max"), value.noise.max);

			core::schema* position = v->set("position");
			series::pack(position->set("intensity"), value.position.intensity);
			series::pack(position->set("accuracy"), value.position.accuracy);
			heavy_series::pack(position->set("min"), value.position.min);
			heavy_series::pack(position->set("max"), value.position.max);

			core::schema* rotation = v->set("rotation");
			series::pack(rotation->set("intensity"), value.rotation.intensity);
			series::pack(rotation->set("accuracy"), value.rotation.accuracy);
			series::pack(rotation->set("min"), value.rotation.min);
			series::pack(rotation->set("max"), value.rotation.max);

			core::schema* scale = v->set("scale");
			series::pack(scale->set("intensity"), value.scale.intensity);
			series::pack(scale->set("accuracy"), value.scale.accuracy);
			series::pack(scale->set("min"), value.scale.min);
			series::pack(scale->set("max"), value.scale.max);

			core::schema* velocity = v->set("velocity");
			series::pack(velocity->set("intensity"), value.velocity.intensity);
			series::pack(velocity->set("accuracy"), value.velocity.accuracy);
			heavy_series::pack(velocity->set("min"), value.velocity.min);
			heavy_series::pack(velocity->set("max"), value.velocity.max);
		}
		void heavy_series::pack(core::schema* v, const trigonometry::key_animator_clip& value)
		{
			VI_ASSERT(v != nullptr, "schema should be set");
			series::pack(v->set("name"), value.name);
			series::pack(v->set("rate"), value.rate);
			series::pack(v->set("duration"), value.duration);
			heavy_series::pack(v->set("frames"), value.keys);
		}
		void heavy_series::pack(core::schema* v, const trigonometry::animator_key& value)
		{
			VI_ASSERT(v != nullptr, "schema should be set");
			heavy_series::pack(v->set("position"), value.position);
			heavy_series::pack(v->set("rotation"), value.rotation);
			heavy_series::pack(v->set("scale"), value.scale);
			series::pack(v->set("time"), value.time);
		}
		void heavy_series::pack(core::schema* v, const trigonometry::skin_animator_key& value)
		{
			VI_ASSERT(v != nullptr, "schema should be set");
			heavy_series::pack(v->set("pose"), value.pose);
			series::pack(v->set("time"), value.time);
		}
		void heavy_series::pack(core::schema* v, const trigonometry::element_vertex& value)
		{
			VI_ASSERT(v != nullptr, "schema should be set");
			v->set_attribute("px", core::var::number(value.position_x));
			v->set_attribute("py", core::var::number(value.position_y));
			v->set_attribute("pz", core::var::number(value.position_z));
			v->set_attribute("vx", core::var::number(value.velocity_x));
			v->set_attribute("vy", core::var::number(value.velocity_y));
			v->set_attribute("vz", core::var::number(value.velocity_z));
			v->set_attribute("cx", core::var::number(value.color_x));
			v->set_attribute("cy", core::var::number(value.color_y));
			v->set_attribute("cz", core::var::number(value.color_z));
			v->set_attribute("cw", core::var::number(value.color_w));
			v->set_attribute("a", core::var::number(value.angular));
			v->set_attribute("s", core::var::number(value.scale));
			v->set_attribute("r", core::var::number(value.rotation));
		}
		void heavy_series::pack(core::schema* v, const trigonometry::vertex& value)
		{
			VI_ASSERT(v != nullptr, "schema should be set");
			v->set_attribute("px", core::var::number(value.position_x));
			v->set_attribute("py", core::var::number(value.position_y));
			v->set_attribute("pz", core::var::number(value.position_z));
			v->set_attribute("tx", core::var::number(value.texcoord_x));
			v->set_attribute("ty", core::var::number(value.texcoord_y));
			v->set_attribute("nx", core::var::number(value.normal_x));
			v->set_attribute("ny", core::var::number(value.normal_y));
			v->set_attribute("nz", core::var::number(value.normal_z));
			v->set_attribute("tnx", core::var::number(value.tangent_x));
			v->set_attribute("tny", core::var::number(value.tangent_y));
			v->set_attribute("tnz", core::var::number(value.tangent_z));
			v->set_attribute("btx", core::var::number(value.bitangent_x));
			v->set_attribute("bty", core::var::number(value.bitangent_y));
			v->set_attribute("btz", core::var::number(value.bitangent_z));
		}
		void heavy_series::pack(core::schema* v, const trigonometry::skin_vertex& value)
		{
			VI_ASSERT(v != nullptr, "schema should be set");
			v->set_attribute("px", core::var::number(value.position_x));
			v->set_attribute("py", core::var::number(value.position_y));
			v->set_attribute("pz", core::var::number(value.position_z));
			v->set_attribute("tx", core::var::number(value.texcoord_x));
			v->set_attribute("ty", core::var::number(value.texcoord_y));
			v->set_attribute("nx", core::var::number(value.normal_x));
			v->set_attribute("ny", core::var::number(value.normal_y));
			v->set_attribute("nz", core::var::number(value.normal_z));
			v->set_attribute("tnx", core::var::number(value.tangent_x));
			v->set_attribute("tny", core::var::number(value.tangent_y));
			v->set_attribute("tnz", core::var::number(value.tangent_z));
			v->set_attribute("btx", core::var::number(value.bitangent_x));
			v->set_attribute("bty", core::var::number(value.bitangent_y));
			v->set_attribute("btz", core::var::number(value.bitangent_z));
			v->set_attribute("ji0", core::var::number(value.joint_index0));
			v->set_attribute("ji1", core::var::number(value.joint_index1));
			v->set_attribute("ji2", core::var::number(value.joint_index2));
			v->set_attribute("ji3", core::var::number(value.joint_index3));
			v->set_attribute("jb0", core::var::number(value.joint_bias0));
			v->set_attribute("jb1", core::var::number(value.joint_bias1));
			v->set_attribute("jb2", core::var::number(value.joint_bias2));
			v->set_attribute("jb3", core::var::number(value.joint_bias3));
		}
		void heavy_series::pack(core::schema* v, const trigonometry::joint& value)
		{
			VI_ASSERT(v != nullptr, "schema should be set");
			series::pack(v->set("index"), (uint64_t)value.index);
			series::pack(v->set("name"), value.name);
			heavy_series::pack(v->set("global"), value.global);
			heavy_series::pack(v->set("local"), value.local);

			core::schema* joints = v->set("childs", core::var::array());
			for (auto& it : value.childs)
				heavy_series::pack(joints->set("joint"), it);
		}
		void heavy_series::pack(core::schema* v, const ticker& value)
		{
			VI_ASSERT(v != nullptr, "schema should be set");
			v->set_attribute("delay", core::var::number(value.delay));
		}
		void heavy_series::pack(core::schema* v, const core::vector<trigonometry::vector2>& value)
		{
			VI_ASSERT(v != nullptr, "schema should be set");
			core::string_stream stream;
			for (auto&& it : value)
				stream << it.x << " " << it.y << " ";

			v->set("v2-array", core::var::string(stream.str().substr(0, stream.str().size() - 1)));
			v->set("size", core::var::integer((int64_t)value.size()));
		}
		void heavy_series::pack(core::schema* v, const core::vector<trigonometry::vector3>& value)
		{
			VI_ASSERT(v != nullptr, "schema should be set");
			core::string_stream stream;
			for (auto&& it : value)
				stream << it.x << " " << it.y << " " << it.z << " ";

			v->set("v3-array", core::var::string(stream.str().substr(0, stream.str().size() - 1)));
			v->set("size", core::var::integer((int64_t)value.size()));
		}
		void heavy_series::pack(core::schema* v, const core::vector<trigonometry::vector4>& value)
		{
			VI_ASSERT(v != nullptr, "schema should be set");
			core::string_stream stream;
			for (auto&& it : value)
				stream << it.x << " " << it.y << " " << it.z << " " << it.w << " ";

			v->set("v4-array", core::var::string(stream.str().substr(0, stream.str().size() - 1)));
			v->set("size", core::var::integer((int64_t)value.size()));
		}
		void heavy_series::pack(core::schema* v, const core::vector<trigonometry::matrix4x4>& value)
		{
			VI_ASSERT(v != nullptr, "schema should be set");
			core::string_stream stream;
			for (auto&& it : value)
			{
				for (float i : it.row)
					stream << i << " ";
			}

			v->set("m4x4-array", core::var::string(stream.str().substr(0, stream.str().size() - 1)));
			v->set("size", core::var::integer((int64_t)value.size()));
		}
		void heavy_series::pack(core::schema* v, const core::vector<animator_state>& value)
		{
			VI_ASSERT(v != nullptr, "schema should be set");
			core::string_stream stream;
			for (auto&& it : value)
			{
				stream << it.paused << " ";
				stream << it.looped << " ";
				stream << it.blended << " ";
				stream << it.duration << " ";
				stream << it.rate << " ";
				stream << it.time << " ";
				stream << it.frame << " ";
				stream << it.clip << " ";
			}

			v->set("as-array", core::var::string(stream.str().substr(0, stream.str().size() - 1)));
			v->set("size", core::var::integer((int64_t)value.size()));
		}
		void heavy_series::pack(core::schema* v, const core::vector<spawner_properties>& value)
		{
			VI_ASSERT(v != nullptr, "schema should be set");
			core::string_stream stream;
			for (auto&& it : value)
			{
				stream << it.angular.accuracy << " " << it.angular.min << " " << it.angular.max << " ";
				stream << it.rotation.accuracy << " " << it.rotation.min << " " << it.rotation.max << " ";
				stream << it.scale.accuracy << " " << it.scale.min << " " << it.scale.max << " ";
				stream << it.diffusion.accuracy << " ";
				stream << it.diffusion.min.x << " " << it.diffusion.min.y << " " << it.diffusion.min.z << " " << it.diffusion.min.w << " ";
				stream << it.diffusion.max.x << " " << it.diffusion.max.y << " " << it.diffusion.max.z << " " << it.diffusion.max.w << " ";
				stream << it.noise.accuracy << " ";
				stream << it.noise.min.x << " " << it.noise.min.y << " " << it.noise.min.z << " ";
				stream << it.noise.max.x << " " << it.noise.max.y << " " << it.noise.max.z << " ";
				stream << it.position.accuracy << " ";
				stream << it.position.min.x << " " << it.position.min.y << " " << it.position.min.z << " ";
				stream << it.position.max.x << " " << it.position.max.y << " " << it.position.max.z << " ";
				stream << it.velocity.accuracy << " ";
				stream << it.velocity.min.x << " " << it.velocity.min.y << " " << it.velocity.min.z << " ";
				stream << it.velocity.max.x << " " << it.velocity.max.y << " " << it.velocity.max.z << " ";
				stream << it.iterations << " ";
			}

			v->set("sp-array", core::var::string(stream.str().substr(0, stream.str().size() - 1)));
			v->set("size", core::var::integer((int64_t)value.size()));
		}
		void heavy_series::pack(core::schema* v, const core::vector<trigonometry::key_animator_clip>& value)
		{
			VI_ASSERT(v != nullptr, "schema should be set");
			core::schema* array = v->set("clips", core::var::array());
			for (auto&& it : value)
				heavy_series::pack(array->set("clip"), it);
		}
		void heavy_series::pack(core::schema* v, const core::vector<trigonometry::animator_key>& value)
		{
			VI_ASSERT(v != nullptr, "schema should be set");
			core::string_stream stream;
			for (auto&& it : value)
			{
				stream << it.position.x << " ";
				stream << it.position.y << " ";
				stream << it.position.z << " ";
				stream << it.rotation.x << " ";
				stream << it.rotation.y << " ";
				stream << it.rotation.z << " ";
				stream << it.rotation.w << " ";
				stream << it.scale.x << " ";
				stream << it.scale.y << " ";
				stream << it.scale.z << " ";
				stream << it.time << " ";
			}

			v->set("ak-array", core::var::string(stream.str().substr(0, stream.str().size() - 1)));
			v->set("size", core::var::integer((int64_t)value.size()));
		}
		void heavy_series::pack(core::schema* v, const core::vector<trigonometry::element_vertex>& value)
		{
			VI_ASSERT(v != nullptr, "schema should be set");
			core::string_stream stream;
			for (auto&& it : value)
			{
				stream << it.position_x << " ";
				stream << it.position_y << " ";
				stream << it.position_z << " ";
				stream << it.color_x << " ";
				stream << it.color_y << " ";
				stream << it.color_z << " ";
				stream << it.color_w << " ";
				stream << it.velocity_x << " ";
				stream << it.velocity_y << " ";
				stream << it.velocity_z << " ";
				stream << it.angular << " ";
				stream << it.rotation << " ";
				stream << it.scale << " ";
			}

			v->set("ev-array", core::var::string(stream.str().substr(0, stream.str().size() - 1)));
			v->set("size", core::var::integer((int64_t)value.size()));
		}
		void heavy_series::pack(core::schema* v, const core::vector<trigonometry::joint>& value)
		{
			VI_ASSERT(v != nullptr, "schema should be set");
			for (auto&& it : value)
				heavy_series::pack(v->set("joint"), it);
		}
		void heavy_series::pack(core::schema* v, const core::vector<trigonometry::vertex>& value)
		{
			VI_ASSERT(v != nullptr, "schema should be set");
			core::string_stream stream;
			for (auto&& it : value)
			{
				stream << it.position_x << " ";
				stream << it.position_y << " ";
				stream << it.position_z << " ";
				stream << it.texcoord_x << " ";
				stream << it.texcoord_y << " ";
				stream << it.normal_x << " ";
				stream << it.normal_y << " ";
				stream << it.normal_z << " ";
				stream << it.tangent_x << " ";
				stream << it.tangent_y << " ";
				stream << it.tangent_z << " ";
				stream << it.bitangent_x << " ";
				stream << it.bitangent_y << " ";
				stream << it.bitangent_z << " ";
				stream << "-1 -1 -1 -1 0 0 0 0 ";
			}

			v->set("iv-array", core::var::string(stream.str().substr(0, stream.str().size() - 1)));
			v->set("size", core::var::integer((int64_t)value.size()));
		}
		void heavy_series::pack(core::schema* v, const core::vector<trigonometry::skin_vertex>& value)
		{
			VI_ASSERT(v != nullptr, "schema should be set");
			core::string_stream stream;
			for (auto&& it : value)
			{
				stream << it.position_x << " ";
				stream << it.position_y << " ";
				stream << it.position_z << " ";
				stream << it.texcoord_x << " ";
				stream << it.texcoord_y << " ";
				stream << it.normal_x << " ";
				stream << it.normal_y << " ";
				stream << it.normal_z << " ";
				stream << it.tangent_x << " ";
				stream << it.tangent_y << " ";
				stream << it.tangent_z << " ";
				stream << it.bitangent_x << " ";
				stream << it.bitangent_y << " ";
				stream << it.bitangent_z << " ";
				stream << it.joint_index0 << " ";
				stream << it.joint_index1 << " ";
				stream << it.joint_index2 << " ";
				stream << it.joint_index3 << " ";
				stream << it.joint_bias0 << " ";
				stream << it.joint_bias1 << " ";
				stream << it.joint_bias2 << " ";
				stream << it.joint_bias3 << " ";
			}

			v->set("iv-array", core::var::string(stream.str().substr(0, stream.str().size() - 1)));
			v->set("size", core::var::integer((int64_t)value.size()));
		}
		void heavy_series::pack(core::schema* v, const core::vector<ticker>& value)
		{
			VI_ASSERT(v != nullptr, "schema should be set");
			core::string_stream stream;
			for (auto&& it : value)
				stream << it.delay << " ";

			v->set("tt-array", core::var::string(stream.str().substr(0, stream.str().size() - 1)));
			v->set("size", core::var::integer((int64_t)value.size()));
		}
		bool heavy_series::unpack(core::schema* v, trigonometry::vector2* o)
		{
			VI_ASSERT(o != nullptr, "output should be set");
			if (!v)
				return false;

			o->x = (float)v->get_attribute_var("x").get_number();
			o->y = (float)v->get_attribute_var("y").get_number();
			return true;
		}
		bool heavy_series::unpack(core::schema* v, trigonometry::vector3* o)
		{
			VI_ASSERT(o != nullptr, "output should be set");
			if (!v)
				return false;

			o->x = (float)v->get_attribute_var("x").get_number();
			o->y = (float)v->get_attribute_var("y").get_number();
			o->z = (float)v->get_attribute_var("z").get_number();
			return true;
		}
		bool heavy_series::unpack(core::schema* v, trigonometry::vector4* o)
		{
			VI_ASSERT(o != nullptr, "output should be set");
			if (!v)
				return false;

			o->x = (float)v->get_attribute_var("x").get_number();
			o->y = (float)v->get_attribute_var("y").get_number();
			o->z = (float)v->get_attribute_var("z").get_number();
			o->w = (float)v->get_attribute_var("w").get_number();
			return true;
		}
		bool heavy_series::unpack(core::schema* v, trigonometry::quaternion* o)
		{
			VI_ASSERT(o != nullptr, "output should be set");
			if (!v)
				return false;

			o->x = (float)v->get_attribute_var("x").get_number();
			o->y = (float)v->get_attribute_var("y").get_number();
			o->z = (float)v->get_attribute_var("z").get_number();
			o->w = (float)v->get_attribute_var("w").get_number();
			return true;
		}
		bool heavy_series::unpack(core::schema* v, trigonometry::matrix4x4* o)
		{
			VI_ASSERT(o != nullptr, "output should be set");
			if (!v)
				return false;

			o->row[0] = (float)v->get_attribute_var("m11").get_number();
			o->row[1] = (float)v->get_attribute_var("m12").get_number();
			o->row[2] = (float)v->get_attribute_var("m13").get_number();
			o->row[3] = (float)v->get_attribute_var("m14").get_number();
			o->row[4] = (float)v->get_attribute_var("m21").get_number();
			o->row[5] = (float)v->get_attribute_var("m22").get_number();
			o->row[6] = (float)v->get_attribute_var("m23").get_number();
			o->row[7] = (float)v->get_attribute_var("m24").get_number();
			o->row[8] = (float)v->get_attribute_var("m31").get_number();
			o->row[9] = (float)v->get_attribute_var("m32").get_number();
			o->row[10] = (float)v->get_attribute_var("m33").get_number();
			o->row[11] = (float)v->get_attribute_var("m34").get_number();
			o->row[12] = (float)v->get_attribute_var("m41").get_number();
			o->row[13] = (float)v->get_attribute_var("m42").get_number();
			o->row[14] = (float)v->get_attribute_var("m43").get_number();
			o->row[15] = (float)v->get_attribute_var("m44").get_number();
			return true;
		}
		bool heavy_series::unpack(core::schema* v, attenuation* o)
		{
			VI_ASSERT(o != nullptr, "output should be set");
			if (!v)
				return false;

			series::unpack(v->get("radius"), &o->radius);
			series::unpack(v->get("c1"), &o->C1);
			series::unpack(v->get("c2"), &o->C2);
			return true;
		}
		bool heavy_series::unpack(core::schema* v, animator_state* o)
		{
			VI_ASSERT(o != nullptr, "output should be set");
			if (!v)
				return false;

			series::unpack(v->get("looped"), &o->looped);
			series::unpack(v->get("paused"), &o->paused);
			series::unpack(v->get("blended"), &o->blended);
			series::unpack(v->get("clip"), &o->clip);
			series::unpack(v->get("frame"), &o->frame);
			series::unpack(v->get("rate"), &o->rate);
			series::unpack(v->get("duration"), &o->duration);
			series::unpack(v->get("time"), &o->time);
			return true;
		}
		bool heavy_series::unpack(core::schema* v, spawner_properties* o)
		{
			VI_ASSERT(o != nullptr, "output should be set");
			if (!v)
				return false;

			series::unpack(v->get("iterations"), &o->iterations);

			core::schema* angular = v->get("angular");
			series::unpack(angular->get("intensity"), &o->angular.intensity);
			series::unpack(angular->get("accuracy"), &o->angular.accuracy);
			series::unpack(angular->get("min"), &o->angular.min);
			series::unpack(angular->get("max"), &o->angular.max);

			core::schema* diffusion = v->get("diffusion");
			series::unpack(diffusion->get("intensity"), &o->diffusion.intensity);
			series::unpack(diffusion->get("accuracy"), &o->diffusion.accuracy);
			heavy_series::unpack(diffusion->get("min"), &o->diffusion.min);
			heavy_series::unpack(diffusion->get("max"), &o->diffusion.max);

			core::schema* noise = v->get("noise");
			series::unpack(noise->get("intensity"), &o->noise.intensity);
			series::unpack(noise->get("accuracy"), &o->noise.accuracy);
			heavy_series::unpack(noise->get("min"), &o->noise.min);
			heavy_series::unpack(noise->get("max"), &o->noise.max);

			core::schema* position = v->get("position");
			series::unpack(position->get("intensity"), &o->position.intensity);
			series::unpack(position->get("accuracy"), &o->position.accuracy);
			heavy_series::unpack(position->get("min"), &o->position.min);
			heavy_series::unpack(position->get("max"), &o->position.max);

			core::schema* rotation = v->get("rotation");
			series::unpack(rotation->get("intensity"), &o->rotation.intensity);
			series::unpack(rotation->get("accuracy"), &o->rotation.accuracy);
			series::unpack(rotation->get("min"), &o->rotation.min);
			series::unpack(rotation->get("max"), &o->rotation.max);

			core::schema* scale = v->get("scale");
			series::unpack(scale->get("intensity"), &o->scale.intensity);
			series::unpack(scale->get("accuracy"), &o->scale.accuracy);
			series::unpack(scale->get("min"), &o->scale.min);
			series::unpack(scale->get("max"), &o->scale.max);

			core::schema* velocity = v->get("velocity");
			series::unpack(velocity->get("intensity"), &o->velocity.intensity);
			series::unpack(velocity->get("accuracy"), &o->velocity.accuracy);
			heavy_series::unpack(velocity->get("min"), &o->velocity.min);
			heavy_series::unpack(velocity->get("max"), &o->velocity.max);

			return true;
		}
		bool heavy_series::unpack(core::schema* v, trigonometry::key_animator_clip* o)
		{
			VI_ASSERT(o != nullptr, "output should be set");
			if (!v)
				return false;

			series::unpack(v->get("name"), &o->name);
			series::unpack(v->get("duration"), &o->duration);
			series::unpack(v->get("rate"), &o->rate);
			heavy_series::unpack(v->get("frames"), &o->keys);
			return true;
		}
		bool heavy_series::unpack(core::schema* v, trigonometry::animator_key* o)
		{
			VI_ASSERT(o != nullptr, "output should be set");
			if (!v)
				return false;

			heavy_series::unpack(v->get("position"), &o->position);
			heavy_series::unpack(v->get("rotation"), &o->rotation);
			heavy_series::unpack(v->get("scale"), &o->scale);
			series::unpack(v->get("time"), &o->time);
			return true;
		}
		bool heavy_series::unpack(core::schema* v, trigonometry::skin_animator_key* o)
		{
			VI_ASSERT(o != nullptr, "output should be set");
			if (!v)
				return false;

			heavy_series::unpack(v->get("pose"), &o->pose);
			series::unpack(v->get("time"), &o->time);

			return true;
		}
		bool heavy_series::unpack(core::schema* v, trigonometry::joint* o)
		{
			VI_ASSERT(o != nullptr, "output should be set");
			if (!v)
				return false;

			series::unpack_a(v->get("index"), &o->index);
			series::unpack(v->get("name"), &o->name);
			heavy_series::unpack(v->get("global"), &o->global);
			heavy_series::unpack(v->get("local"), &o->local);

			core::vector<core::schema*> joints = v->fetch_collection("childs.joint", false);
			for (auto& it : joints)
			{
				o->childs.emplace_back();
				heavy_series::unpack(it, &o->childs.back());
			}

			return true;
		}
		bool heavy_series::unpack(core::schema* v, trigonometry::element_vertex* o)
		{
			VI_ASSERT(o != nullptr, "output should be set");
			if (!v)
				return false;

			o->position_x = (float)v->get_attribute_var("px").get_number();
			o->position_y = (float)v->get_attribute_var("py").get_number();
			o->position_z = (float)v->get_attribute_var("pz").get_number();
			o->velocity_x = (float)v->get_attribute_var("vx").get_number();
			o->velocity_y = (float)v->get_attribute_var("vy").get_number();
			o->velocity_z = (float)v->get_attribute_var("vz").get_number();
			o->color_x = (float)v->get_attribute_var("cx").get_number();
			o->color_y = (float)v->get_attribute_var("cy").get_number();
			o->color_z = (float)v->get_attribute_var("cz").get_number();
			o->color_w = (float)v->get_attribute_var("cw").get_number();
			o->angular = (float)v->get_attribute_var("a").get_number();
			o->scale = (float)v->get_attribute_var("s").get_number();
			o->rotation = (float)v->get_attribute_var("r").get_number();
			return true;
		}
		bool heavy_series::unpack(core::schema* v, trigonometry::vertex* o)
		{
			VI_ASSERT(o != nullptr, "output should be set");
			if (!v)
				return false;

			o->position_x = (float)v->get_attribute_var("px").get_number();
			o->position_y = (float)v->get_attribute_var("py").get_number();
			o->position_z = (float)v->get_attribute_var("pz").get_number();
			o->texcoord_x = (float)v->get_attribute_var("tx").get_number();
			o->texcoord_y = (float)v->get_attribute_var("ty").get_number();
			o->normal_x = (float)v->get_attribute_var("nx").get_number();
			o->normal_y = (float)v->get_attribute_var("ny").get_number();
			o->normal_z = (float)v->get_attribute_var("nz").get_number();
			o->tangent_x = (float)v->get_attribute_var("tnx").get_number();
			o->tangent_y = (float)v->get_attribute_var("tny").get_number();
			o->tangent_z = (float)v->get_attribute_var("tnz").get_number();
			o->bitangent_x = (float)v->get_attribute_var("btx").get_number();
			o->bitangent_y = (float)v->get_attribute_var("bty").get_number();
			o->bitangent_z = (float)v->get_attribute_var("btz").get_number();
			return true;
		}
		bool heavy_series::unpack(core::schema* v, trigonometry::skin_vertex* o)
		{
			VI_ASSERT(o != nullptr, "output should be set");
			if (!v)
				return false;

			o->position_x = (float)v->get_attribute_var("px").get_number();
			o->position_y = (float)v->get_attribute_var("py").get_number();
			o->position_z = (float)v->get_attribute_var("pz").get_number();
			o->texcoord_x = (float)v->get_attribute_var("tx").get_number();
			o->texcoord_y = (float)v->get_attribute_var("ty").get_number();
			o->normal_x = (float)v->get_attribute_var("nx").get_number();
			o->normal_y = (float)v->get_attribute_var("ny").get_number();
			o->normal_z = (float)v->get_attribute_var("nz").get_number();
			o->tangent_x = (float)v->get_attribute_var("tnx").get_number();
			o->tangent_y = (float)v->get_attribute_var("tny").get_number();
			o->tangent_z = (float)v->get_attribute_var("tnz").get_number();
			o->bitangent_x = (float)v->get_attribute_var("btx").get_number();
			o->bitangent_y = (float)v->get_attribute_var("bty").get_number();
			o->bitangent_z = (float)v->get_attribute_var("btz").get_number();
			o->joint_index0 = (float)v->get_attribute_var("ji0").get_number();
			o->joint_index1 = (float)v->get_attribute_var("ji1").get_number();
			o->joint_index2 = (float)v->get_attribute_var("ji2").get_number();
			o->joint_index3 = (float)v->get_attribute_var("ji3").get_number();
			o->joint_bias0 = (float)v->get_attribute_var("jb0").get_number();
			o->joint_bias1 = (float)v->get_attribute_var("jb1").get_number();
			o->joint_bias2 = (float)v->get_attribute_var("jb2").get_number();
			o->joint_bias3 = (float)v->get_attribute_var("jb3").get_number();
			return true;
		}
		bool heavy_series::unpack(core::schema* v, ticker* o)
		{
			VI_ASSERT(o != nullptr, "output should be set");
			if (!v)
				return false;

			o->delay = (float)v->get_attribute_var("delay").get_number();
			return true;
		}
		bool heavy_series::unpack(core::schema* v, core::vector<trigonometry::vector2>* o)
		{
			VI_ASSERT(o != nullptr, "output should be set");
			if (!v)
				return false;

			core::string array(v->get_var("v2-array").get_blob());
			int64_t size = v->get_var("size").get_integer();
			if (array.empty() || !size)
				return false;

			core::string_stream stream(array);
			o->resize((size_t)size);

			for (auto& it : *o)
				stream >> it.x >> it.y;

			return true;
		}
		bool heavy_series::unpack(core::schema* v, core::vector<trigonometry::vector3>* o)
		{
			VI_ASSERT(o != nullptr, "output should be set");
			if (!v)
				return false;

			core::string array(v->get_var("v3-array").get_blob());
			int64_t size = v->get_var("size").get_integer();
			if (array.empty() || !size)
				return false;

			core::string_stream stream(array);
			o->resize((size_t)size);

			for (auto& it : *o)
				stream >> it.x >> it.y >> it.z;

			return true;
		}
		bool heavy_series::unpack(core::schema* v, core::vector<trigonometry::vector4>* o)
		{
			VI_ASSERT(o != nullptr, "output should be set");
			if (!v)
				return false;

			core::string array(v->get_var("v4-array").get_blob());
			int64_t size = v->get_var("size").get_integer();
			if (array.empty() || !size)
				return false;

			core::string_stream stream(array);
			o->resize((size_t)size);

			for (auto& it : *o)
				stream >> it.x >> it.y >> it.z >> it.w;

			return true;
		}
		bool heavy_series::unpack(core::schema* v, core::vector<trigonometry::matrix4x4>* o)
		{
			VI_ASSERT(o != nullptr, "output should be set");
			if (!v)
				return false;

			core::string array(v->get_var("m4x4-array").get_blob());
			int64_t size = v->get_var("size").get_integer();
			if (array.empty() || !size)
				return false;

			core::string_stream stream(array);
			o->resize((size_t)size);

			for (auto& it : *o)
			{
				for (int64_t i = 0; i < 16; i++)
					stream >> it.row[i];
			}

			return true;
		}
		bool heavy_series::unpack(core::schema* v, core::vector<animator_state>* o)
		{
			VI_ASSERT(o != nullptr, "output should be set");
			if (!v)
				return false;

			core::string array(v->get_var("as-array").get_blob());
			int64_t size = v->get_var("size").get_integer();
			if (array.empty() || !size)
				return false;

			core::string_stream stream(array);
			o->resize((size_t)size);

			for (auto& it : *o)
			{
				stream >> it.paused;
				stream >> it.looped;
				stream >> it.blended;
				stream >> it.duration;
				stream >> it.rate;
				stream >> it.time;
				stream >> it.frame;
				stream >> it.clip;
			}

			return true;
		}
		bool heavy_series::unpack(core::schema* v, core::vector<spawner_properties>* o)
		{
			VI_ASSERT(o != nullptr, "output should be set");
			if (!v)
				return false;

			core::string array(v->get_var("sp-array").get_blob());
			int64_t size = v->get_var("size").get_integer();
			if (array.empty() || !size)
				return false;

			core::string_stream stream(array);
			o->resize((size_t)size);

			for (auto& it : *o)
			{
				stream >> it.angular.accuracy >> it.angular.min >> it.angular.max;
				stream >> it.rotation.accuracy >> it.rotation.min >> it.rotation.max;
				stream >> it.scale.accuracy >> it.scale.min >> it.scale.max;
				stream >> it.diffusion.accuracy;
				stream >> it.diffusion.min.x >> it.diffusion.min.y >> it.diffusion.min.z >> it.diffusion.min.w;
				stream >> it.diffusion.max.x >> it.diffusion.max.y >> it.diffusion.max.z >> it.diffusion.max.w;
				stream >> it.noise.accuracy;
				stream >> it.noise.min.x >> it.noise.min.y >> it.noise.min.z;
				stream >> it.noise.max.x >> it.noise.max.y >> it.noise.max.z;
				stream >> it.position.accuracy;
				stream >> it.position.min.x >> it.position.min.y >> it.position.min.z;
				stream >> it.position.max.x >> it.position.max.y >> it.position.max.z;
				stream >> it.velocity.accuracy;
				stream >> it.velocity.min.x >> it.velocity.min.y >> it.velocity.min.z;
				stream >> it.velocity.max.x >> it.velocity.max.y >> it.velocity.max.z;
				stream >> it.iterations;
			}

			return true;
		}
		bool heavy_series::unpack(core::schema* v, core::vector<trigonometry::key_animator_clip>* o)
		{
			VI_ASSERT(o != nullptr, "output should be set");
			if (!v)
				return false;

			core::vector<core::schema*> frames = v->fetch_collection("clips.clip", false);
			for (auto&& it : frames)
			{
				o->push_back(trigonometry::key_animator_clip());
				heavy_series::unpack(it, &o->back());
			}

			return true;
		}
		bool heavy_series::unpack(core::schema* v, core::vector<trigonometry::animator_key>* o)
		{
			VI_ASSERT(o != nullptr, "output should be set");
			if (!v)
				return false;

			core::string array(v->get_var("ak-array").get_blob());
			int64_t size = v->get_var("size").get_integer();
			if (array.empty() || !size)
				return false;

			core::string_stream stream(array);
			o->resize((size_t)size);

			for (auto& it : *o)
			{
				stream >> it.position.x >> it.position.y >> it.position.z >> it.rotation.x >> it.rotation.y >> it.rotation.z >> it.rotation.w;
				stream >> it.scale.x >> it.scale.y >> it.scale.z >> it.time;
			}

			return true;
		}
		bool heavy_series::unpack(core::schema* v, core::vector<trigonometry::element_vertex>* o)
		{
			VI_ASSERT(o != nullptr, "output should be set");
			if (!v)
				return false;

			core::string array(v->get_var("ev-array").get_blob());
			int64_t size = v->get_var("size").get_integer();
			if (array.empty() || !size)
				return false;

			core::string_stream stream(array);
			o->resize((size_t)size);

			for (auto& it : *o)
			{
				stream >> it.position_x;
				stream >> it.position_y;
				stream >> it.position_z;
				stream >> it.color_x;
				stream >> it.color_y;
				stream >> it.color_z;
				stream >> it.color_w;
				stream >> it.velocity_x;
				stream >> it.velocity_y;
				stream >> it.velocity_z;
				stream >> it.angular;
				stream >> it.rotation;
				stream >> it.scale;
			}

			return true;
		}
		bool heavy_series::unpack(core::schema* v, core::vector<trigonometry::joint>* o)
		{
			VI_ASSERT(o != nullptr, "output should be set");
			if (!v)
				return false;

			o->reserve(v->size());
			for (auto&& it : v->get_childs())
			{
				o->push_back(trigonometry::joint());
				heavy_series::unpack(it, &o->back());
			}

			return true;
		}
		bool heavy_series::unpack(core::schema* v, core::vector<trigonometry::vertex>* o)
		{
			if (!v || !o)
				return false;

			core::string array(v->get_var("iv-array").get_blob());
			int64_t size = v->get_var("size").get_integer();
			if (array.empty() || !size)
				return false;

			core::string_stream stream(array);
			o->resize((size_t)size);

			float dummy;
			for (auto& it : *o)
			{
				stream >> it.position_x;
				stream >> it.position_y;
				stream >> it.position_z;
				stream >> it.texcoord_x;
				stream >> it.texcoord_y;
				stream >> it.normal_x;
				stream >> it.normal_y;
				stream >> it.normal_z;
				stream >> it.tangent_x;
				stream >> it.tangent_y;
				stream >> it.tangent_z;
				stream >> it.bitangent_x;
				stream >> it.bitangent_y;
				stream >> it.bitangent_z;
				stream >> dummy;
				stream >> dummy;
				stream >> dummy;
				stream >> dummy;
				stream >> dummy;
				stream >> dummy;
				stream >> dummy;
				stream >> dummy;
			}

			return true;
		}
		bool heavy_series::unpack(core::schema* v, core::vector<trigonometry::skin_vertex>* o)
		{
			VI_ASSERT(o != nullptr, "output should be set");
			if (!v)
				return false;

			core::string array(v->get_var("iv-array").get_blob());
			int64_t size = v->get_var("size").get_integer();
			if (array.empty() || !size)
				return false;

			core::string_stream stream(array);
			o->resize((size_t)size);

			for (auto& it : *o)
			{
				stream >> it.position_x;
				stream >> it.position_y;
				stream >> it.position_z;
				stream >> it.texcoord_x;
				stream >> it.texcoord_y;
				stream >> it.normal_x;
				stream >> it.normal_y;
				stream >> it.normal_z;
				stream >> it.tangent_x;
				stream >> it.tangent_y;
				stream >> it.tangent_z;
				stream >> it.bitangent_x;
				stream >> it.bitangent_y;
				stream >> it.bitangent_z;
				stream >> it.joint_index0;
				stream >> it.joint_index1;
				stream >> it.joint_index2;
				stream >> it.joint_index3;
				stream >> it.joint_bias0;
				stream >> it.joint_bias1;
				stream >> it.joint_bias2;
				stream >> it.joint_bias3;
			}

			return true;
		}
		bool heavy_series::unpack(core::schema* v, core::vector<ticker>* o)
		{
			VI_ASSERT(o != nullptr, "output should be set");
			if (!v)
				return false;

			core::string array(v->get_var("tt-array").get_blob());
			int64_t size = v->get_var("size").get_integer();
			if (array.empty() || !size)
				return false;

			core::string_stream stream(array);
			o->resize((size_t)size);

			for (auto& it : *o)
				stream >> it.delay;

			return true;
		}

		void pose_buffer::fill(skin_model* model)
		{
			VI_ASSERT(model != nullptr, "model should be set");
			offsets.clear();
			matrices.clear();

			fill(model->skeleton);
			for (auto& mesh : model->meshes)
				matrices.insert(std::make_pair(mesh, pose_matrices()));
		}
		void pose_buffer::fill(trigonometry::joint& next)
		{
			auto& data = offsets[next.index];
			data.defaults.position = next.global.position();
			data.defaults.rotation = next.global.rotation_quaternion();
			data.defaults.scale = next.global.scale();
			data.offset = data.frame = data.defaults;

			for (auto& child : next.childs)
				fill(child);
		}

		model::model() noexcept
		{
		}
		model::~model() noexcept
		{
			cleanup();
		}
		void model::cleanup()
		{
			for (auto* item : meshes)
				core::memory::release(item);
			meshes.clear();
		}
		graphics::mesh_buffer* model::find_mesh(const std::string_view& name)
		{
			for (auto&& it : meshes)
			{
				if (it->name == name)
					return it;
			}

			return nullptr;
		}

		skin_model::skin_model() noexcept
		{
		}
		skin_model::~skin_model() noexcept
		{
			cleanup();
		}
		bool skin_model::find_joint(const std::string_view& name, trigonometry::joint* base)
		{
			if (!base)
				base = &skeleton;

			if (base->name == name)
				return base;

			for (auto&& child : base->childs)
			{
				if (child.name == name)
				{
					base = &child;
					return true;
				}

				trigonometry::joint* result = &child;
				if (find_joint(name, result))
					return true;
			}

			return false;
		}
		bool skin_model::find_joint(size_t index, trigonometry::joint* base)
		{
			if (!base)
				base = &skeleton;

			if (base->index == index)
				return true;

			for (auto&& child : base->childs)
			{
				if (child.index == index)
				{
					base = &child;
					return true;
				}

				trigonometry::joint* result = &child;
				if (find_joint(index, result))
					return true;
			}

			return false;
		}
		void skin_model::synchronize(pose_buffer* map)
		{
			VI_ASSERT(map != nullptr, "pose buffer should be set");
			VI_MEASURE(core::timings::atomic);

			for (auto& mesh : meshes)
				map->matrices[mesh];

			synchronize(map, skeleton, transform);
		}
		void skin_model::synchronize(pose_buffer* map, trigonometry::joint& next, const trigonometry::matrix4x4& parent_offset)
		{
			auto& node = map->offsets[next.index].offset;
			auto local_offset = trigonometry::matrix4x4::create_scale(node.scale) * node.rotation.get_matrix() * trigonometry::matrix4x4::create_translation(node.position);
			auto global_offset = local_offset * parent_offset;
			auto final_offset = next.local * global_offset * inv_transform;

			for (auto& matrices : map->matrices)
			{
				auto index = matrices.first->joints.find(next.index);
				if (index != matrices.first->joints.end() && index->second <= graphics::joints_size)
					matrices.second.data[index->second] = final_offset;
			}

			for (auto& child : next.childs)
				synchronize(map, child, global_offset);
		}
		void skin_model::cleanup()
		{
			for (auto* item : meshes)
				core::memory::release(item);
			meshes.clear();
		}
		graphics::skin_mesh_buffer* skin_model::find_mesh(const std::string_view& name)
		{
			for (auto&& it : meshes)
			{
				if (it->name == name)
					return it;
			}

			return nullptr;
		}

		skin_animation::skin_animation(core::vector<trigonometry::skin_animator_clip>&& data) noexcept : clips(std::move(data))
		{
		}
		const core::vector<trigonometry::skin_animator_clip>& skin_animation::get_clips()
		{
			return clips;
		}
		bool skin_animation::is_valid()
		{
			return !clips.empty();
		}

		material::material(scene_graph* new_scene) noexcept : diffuse_map(nullptr), normal_map(nullptr), metallic_map(nullptr), roughness_map(nullptr), height_map(nullptr), occlusion_map(nullptr), emission_map(nullptr), scene(new_scene), slot(0)
		{
		}
		material::material(const material& other) noexcept : material(other.scene)
		{
			memcpy(&surface, &other.surface, sizeof(subsurface));
			if (other.diffuse_map != nullptr)
			{
				diffuse_map = other.diffuse_map;
				diffuse_map->add_ref();
			}

			if (other.normal_map != nullptr)
			{
				normal_map = other.normal_map;
				normal_map->add_ref();
			}

			if (other.metallic_map != nullptr)
			{
				metallic_map = other.metallic_map;
				metallic_map->add_ref();
			}

			if (other.roughness_map != nullptr)
			{
				roughness_map = other.roughness_map;
				roughness_map->add_ref();
			}

			if (other.height_map != nullptr)
			{
				height_map = other.height_map;
				height_map->add_ref();
			}

			if (other.occlusion_map != nullptr)
			{
				occlusion_map = other.occlusion_map;
				occlusion_map->add_ref();
			}

			if (other.emission_map != nullptr)
			{
				emission_map = other.emission_map;
				emission_map->add_ref();
			}
		}
		material::~material() noexcept
		{
			core::memory::release(diffuse_map);
			core::memory::release(normal_map);
			core::memory::release(metallic_map);
			core::memory::release(roughness_map);
			core::memory::release(height_map);
			core::memory::release(occlusion_map);
			core::memory::release(emission_map);
		}
		void material::set_name(const std::string_view& value)
		{
			name = value;
			if (scene != nullptr)
				scene->mutate(this, "set");
		}
		const core::string& material::get_name() const
		{
			return name;
		}
		void material::set_diffuse_map(graphics::texture_2d* init)
		{
			VI_TRACE("[layer] material %s apply diffuse 0x%" PRIXPTR, name.c_str(), (void*)init);
			core::memory::release(diffuse_map);
			diffuse_map = init;
			if (diffuse_map != nullptr)
				diffuse_map->add_ref();
		}
		graphics::texture_2d* material::get_diffuse_map() const
		{
			return diffuse_map;
		}
		void material::set_normal_map(graphics::texture_2d* init)
		{
			VI_TRACE("[layer] material %s apply normal 0x%" PRIXPTR, name.c_str(), (void*)init);
			core::memory::release(normal_map);
			normal_map = init;
			if (normal_map != nullptr)
				normal_map->add_ref();
		}
		graphics::texture_2d* material::get_normal_map() const
		{
			return normal_map;
		}
		void material::set_metallic_map(graphics::texture_2d* init)
		{
			VI_TRACE("[layer] material %s apply metallic 0x%" PRIXPTR, name.c_str(), (void*)init);
			core::memory::release(metallic_map);
			metallic_map = init;
			if (metallic_map != nullptr)
				metallic_map->add_ref();
		}
		graphics::texture_2d* material::get_metallic_map() const
		{
			return metallic_map;
		}
		void material::set_roughness_map(graphics::texture_2d* init)
		{
			VI_TRACE("[layer] material %s apply roughness 0x%" PRIXPTR, name.c_str(), (void*)init);
			core::memory::release(roughness_map);
			roughness_map = init;
			if (roughness_map != nullptr)
				roughness_map->add_ref();
		}
		graphics::texture_2d* material::get_roughness_map() const
		{
			return roughness_map;
		}
		void material::set_height_map(graphics::texture_2d* init)
		{
			VI_TRACE("[layer] material %s apply height 0x%" PRIXPTR, name.c_str(), (void*)init);
			core::memory::release(height_map);
			height_map = init;
			if (height_map != nullptr)
				height_map->add_ref();
		}
		graphics::texture_2d* material::get_height_map() const
		{
			return height_map;
		}
		void material::set_occlusion_map(graphics::texture_2d* init)
		{
			VI_TRACE("[layer] material %s apply occlusion 0x%" PRIXPTR, name.c_str(), (void*)init);
			core::memory::release(occlusion_map);
			occlusion_map = init;
			if (occlusion_map != nullptr)
				occlusion_map->add_ref();
		}
		graphics::texture_2d* material::get_occlusion_map() const
		{
			return occlusion_map;
		}
		void material::set_emission_map(graphics::texture_2d* init)
		{
			VI_TRACE("[layer] material %s apply emission 0x%" PRIXPTR, name.c_str(), (void*)init);
			core::memory::release(emission_map);
			emission_map = init;
			if (emission_map != nullptr)
				emission_map->add_ref();
		}
		graphics::texture_2d* material::get_emission_map() const
		{
			return emission_map;
		}
		scene_graph* material::get_scene() const
		{
			return scene;
		}

		component::component(entity* reference, actor_set rule) noexcept : parent(reference), set((size_t)rule), indexed(false), active(true)
		{
			VI_ASSERT(reference != nullptr, "entity should be set");
		}
		component::~component() noexcept
		{
		}
		void component::deserialize(core::schema* node)
		{
		}
		void component::serialize(core::schema* node)
		{
		}
		void component::activate(component* init)
		{
		}
		void component::deactivate()
		{
		}
		void component::animate(core::timer* time)
		{
		}
		void component::synchronize(core::timer* time)
		{
		}
		void component::update(core::timer* time)
		{
		}
		void component::message(const std::string_view& name, core::variant_args& args)
		{
		}
		void component::movement()
		{
		}
		float component::get_visibility(const viewer& view, float distance) const
		{
			float visibility = 1.0f - distance / view.far_plane;
			if (visibility <= 0.0f)
				return 0.0f;

			const trigonometry::matrix4x4& box = parent->get_box();
			return trigonometry::geometric::is_cube_in_frustum(box * view.view_projection, 1.65f) ? visibility : 0.0f;
		}
		size_t component::get_unit_bounds(trigonometry::vector3& min, trigonometry::vector3& max) const
		{
			min = -1.0f;
			max = 1.0f;
			return 0;
		}
		void component::set_active(bool status)
		{
			auto* scene = parent->get_scene();
			if (active == status)
				return;

			active = status;
			if (parent->is_active())
			{
				if (active)
					scene->register_component(this, false);
				else
					scene->unregister_component(this);
			}

			scene->notify_cosmos(this);
		}
		bool component::is_drawable() const
		{
			return set & (uint64_t)actor_set::drawable;
		}
		bool component::is_cullable() const
		{
			return set & (uint64_t)actor_set::cullable;
		}
		bool component::is_active() const
		{
			return active;
		}
		entity* component::get_entity() const
		{
			return parent;
		}

		entity::entity(scene_graph* new_scene) noexcept : transform(new trigonometry::transform(this)), scene(new_scene), active(false)
		{
			VI_ASSERT(scene != nullptr, "entity should be created within a scene");
		}
		entity::~entity() noexcept
		{
			for (auto& component : type.components)
			{
				if (component.second != nullptr)
				{
					component.second->set_active(false);
					scene->unload_component_all(component.second);
					scene->clear_cosmos(component.second);
					core::memory::release(component.second);
				}
			}

			core::memory::release(transform);
		}
		void entity::set_name(const std::string_view& value)
		{
			type.name = value;
			scene->mutate(this, "set");
		}
		void entity::set_root(entity* parent)
		{
			auto* old = transform->get_root();
			if (!parent)
			{
				transform->set_root(nullptr);
				if (old != nullptr && scene != nullptr)
					scene->mutate((entity*)old->user_data, this, "pop");
			}
			else
			{
				transform->set_root(parent->transform);
				if (old != parent->transform && scene != nullptr)
					scene->mutate(parent, this, "push");
			}
		}
		void entity::update_bounds()
		{
			size_t index = 0;
			for (auto& item : type.components)
			{
				trigonometry::vector3 min, max;
				size_t offset = item.second->get_unit_bounds(min, max);
				item.second->movement();

				if (offset > index)
				{
					index = offset;
					snapshot.min = min;
					snapshot.max = max;
				}
			}

			if (!index)
			{
				snapshot.min = -1.0f;
				snapshot.max = 1.0f;
			}

			transform->get_bounds(snapshot.box, snapshot.min, snapshot.max);
		}
		void entity::remove_component(uint64_t id)
		{
			core::unordered_map<uint64_t, component*>::iterator it = type.components.find(id);
			if (it == type.components.end())
				return;

			component* base = it->second;
			base->set_active(false);
			transform->make_dirty();
			type.components.erase(it);
			if (scene->camera == base)
				scene->camera = nullptr;

			auto* top = scene;
			scene->transaction([top, base]()
			{
				top->clear_cosmos(base);
				base->release();
			});
		}
		void entity::remove_childs()
		{
			core::vector<trigonometry::transform*>& childs = transform->get_childs();
			for (size_t i = 0; i < childs.size(); i++)
			{
				entity* next = (entity*)transform->get_child(i)->user_data;
				if (next != nullptr && next != this)
				{
					scene->delete_entity(next);
					i--;
				}
			}
		}
		component* entity::add_component(component* in)
		{
			VI_ASSERT(in != nullptr, "component should be set");
			if (in == get_component(in->get_id()))
				return in;

			remove_component(in->get_id());
			in->active = false;
			in->parent = this;

			type.components.insert({ in->get_id(), in });
			scene->transaction([this, in]()
			{
				for (auto& component : type.components)
					component.second->activate(in == component.second ? nullptr : in);
			});

			in->set_active(true);
			transform->make_dirty();
			return in;
		}
		component* entity::get_component(uint64_t id)
		{
			core::unordered_map<uint64_t, component*>::iterator it = type.components.find(id);
			if (it != type.components.end())
				return it->second;

			return nullptr;
		}
		size_t entity::get_components_count() const
		{
			return type.components.size();
		}
		scene_graph* entity::get_scene() const
		{
			return scene;
		}
		entity* entity::get_parent() const
		{
			auto* root = transform->get_root();
			if (!root)
				return nullptr;

			return (entity*)root->user_data;
		}
		entity* entity::get_child(size_t index) const
		{
			auto* child = transform->get_child(index);
			if (!child)
				return nullptr;

			return (entity*)child->user_data;
		}
		trigonometry::transform* entity::get_transform() const
		{
			return transform;
		}
		const core::string& entity::get_name() const
		{
			return type.name;
		}
		float entity::get_visibility(const viewer& base) const
		{
			float distance = transform->get_position().distance(base.position);
			return 1.0f - distance / base.far_plane;
		}
		const trigonometry::matrix4x4& entity::get_box() const
		{
			return snapshot.box;
		}
		const trigonometry::vector3& entity::get_min() const
		{
			return snapshot.min;
		}
		const trigonometry::vector3& entity::get_max() const
		{
			return snapshot.max;
		}
		size_t entity::get_childs_count() const
		{
			return transform->get_childs_count();
		}
		bool entity::is_active() const
		{
			return active;
		}
		trigonometry::vector3 entity::get_radius3() const
		{
			trigonometry::vector3 diameter3 = snapshot.max - snapshot.min;
			return diameter3.abs().mul(0.5f);
		}
		float entity::get_radius() const
		{
			const trigonometry::vector3& radius = get_radius3();
			float max = (radius.x > radius.y ? radius.x : radius.y);
			return (max > radius.z ? radius.z : max);
		}

		drawable::drawable(entity* ref, actor_set rule, uint64_t hash) noexcept : component(ref, rule | actor_set::cullable | actor_set::drawable | actor_set::message), category(geo_category::opaque), source(hash), overlapping(1.0f), constant(true)
		{
		}
		drawable::~drawable() noexcept
		{
		}
		void drawable::message(const std::string_view& name, core::variant_args& args)
		{
			if (name != "mutation")
				return;

			material* target = (material*)args["material"].get_pointer();
			if (!target || !args["type"].is_string("push"))
				return;

			for (auto&& surface : materials)
			{
				if (surface.second == target)
					surface.second = nullptr;
			}
		}
		void drawable::movement()
		{
			overlapping = 1.0f;
		}
		void drawable::clear_materials()
		{
			materials.clear();
		}
		bool drawable::set_category(geo_category new_category)
		{
			category = new_category;
			return true;
		}
		bool drawable::set_material(void* instance, material* value)
		{
			auto it = materials.find(instance);
			if (it == materials.end())
				materials[instance] = value;
			else
				it->second = value;

			return true;
		}
		bool drawable::set_material(material* value)
		{
			if (materials.empty())
				return set_material(nullptr, value);

			for (auto& item : materials)
				item.second = value;

			return true;
		}
		geo_category drawable::get_category() const
		{
			return category;
		}
		int64_t drawable::get_slot(void* surface)
		{
			material* base = get_material(surface);
			return base ? (int64_t)base->slot : -1;
		}
		int64_t drawable::get_slot()
		{
			material* base = get_material();
			return base ? (int64_t)base->slot : -1;
		}
		material* drawable::get_material(void* instance)
		{
			if (materials.size() == 1)
				return materials.begin()->second;

			auto it = materials.find(instance);
			if (it == materials.end())
				return nullptr;

			return it->second;
		}
		material* drawable::get_material()
		{
			if (materials.empty())
				return nullptr;

			return materials.begin()->second;
		}
		const core::unordered_map<void*, material*>& drawable::get_materials()
		{
			return materials;
		}

		renderer::renderer(render_system* lab) noexcept : system(lab), active(true)
		{
			VI_ASSERT(lab != nullptr, "render system should be set");
		}
		renderer::~renderer() noexcept
		{
		}
		void renderer::set_renderer(render_system* new_system)
		{
			VI_ASSERT(new_system != nullptr, "render system should be set");
			system = new_system;
		}
		void renderer::deserialize(core::schema* node)
		{
		}
		void renderer::serialize(core::schema* node)
		{
		}
		void renderer::clear_culling()
		{
		}
		void renderer::resize_buffers()
		{
		}
		void renderer::activate()
		{
		}
		void renderer::deactivate()
		{
		}
		void renderer::begin_pass(core::timer* time)
		{
		}
		void renderer::end_pass()
		{
		}
		bool renderer::has_category(geo_category category)
		{
			return false;
		}
		size_t renderer::render_prepass(core::timer* time)
		{
			return 0;
		}
		size_t renderer::render_pass(core::timer* time_step)
		{
			return 0;
		}
		render_system* renderer::get_renderer() const
		{
			return system;
		}

		render_constants::render_constants(graphics::graphics_device* new_device) noexcept : device(new_device)
		{
			VI_ASSERT(device != nullptr, "graphics device should be set");
			graphics::shader::desc f = graphics::shader::desc();
			if (device->get_section_data("materials/material_basic_geometry", &f))
				binding.basic_effect = *device->create_shader(f);

			graphics::element_buffer::desc desc;
			desc.bind_flags = graphics::resource_bind::constant_buffer;
			desc.element_count = 1;
			desc.element_width = sizeof(animation);
			binding.buffers[(size_t)render_buffer_type::animation] = *device->create_element_buffer(desc);
			binding.pointers[(size_t)render_buffer_type::animation] = &animation;
			binding.sizes[(size_t)render_buffer_type::animation] = sizeof(animation);

			desc.element_width = sizeof(render);
			binding.buffers[(size_t)render_buffer_type::render] = *device->create_element_buffer(desc);
			binding.pointers[(size_t)render_buffer_type::render] = &render;
			binding.sizes[(size_t)render_buffer_type::render] = sizeof(render);

			desc.element_width = sizeof(view);
			binding.buffers[(size_t)render_buffer_type::view] = *device->create_element_buffer(desc);
			binding.pointers[(size_t)render_buffer_type::view] = &view;
			binding.sizes[(size_t)render_buffer_type::view] = sizeof(view);
		}
		render_constants::~render_constants() noexcept
		{
			for (size_t i = 0; i < 3; i++)
				core::memory::release(binding.buffers[i]);
			core::memory::release(binding.basic_effect);
		}
		void render_constants::set_constant_buffer(render_buffer_type buffer, uint32_t slot, uint32_t type)
		{
			device->set_constant_buffer(binding.buffers[(size_t)buffer], slot, type);
		}
		void render_constants::set_updated_constant_buffer(render_buffer_type buffer, uint32_t slot, uint32_t type)
		{
			device->update_constant_buffer(binding.buffers[(size_t)buffer], binding.pointers[(size_t)buffer], binding.sizes[(size_t)buffer]);
			device->set_constant_buffer(binding.buffers[(size_t)buffer], slot, type);
		}
		void render_constants::update_constant_buffer(render_buffer_type buffer)
		{
			device->update_constant_buffer(binding.buffers[(size_t)buffer], binding.pointers[(size_t)buffer], binding.sizes[(size_t)buffer]);
		}
		graphics::shader* render_constants::get_basic_effect() const
		{
			return binding.basic_effect;
		}
		graphics::graphics_device* render_constants::get_device() const
		{
			return device;
		}
		graphics::element_buffer* render_constants::get_constant_buffer(render_buffer_type buffer) const
		{
			return binding.buffers[(size_t)buffer];
		}

		render_system::render_system(scene_graph* new_scene, component* new_component) noexcept : device(nullptr), base_material(nullptr), scene(new_scene), owner(new_component), max_queries(16384), sorting_frequency(2), occlusion_skips(2), occluder_skips(8), occludee_skips(3), occludee_scaling(1.0f), overflow_visibility(0.0f), threshold(0.1f), occlusion_culling(false), precise_culling(true), allow_input_lag(false)
		{
			VI_ASSERT(new_scene != nullptr, "scene should be set");
			VI_ASSERT(new_scene->get_device() != nullptr, "graphics device should be set");
			VI_ASSERT(new_scene->get_constants() != nullptr, "render constants should be set");
			device = new_scene->get_device();
			constants = new_scene->get_constants();
		}
		render_system::~render_system() noexcept
		{
			remove_renderers();
		}
		void render_system::set_view(const trigonometry::matrix4x4& _View, const trigonometry::matrix4x4& _Projection, const trigonometry::vector3& _Position, float _Fov, float _Ratio, float _Near, float _Far, render_culling _Type)
		{
			view.set(_View, _Projection, _Position, _Fov, _Ratio, _Near, _Far, _Type);
			restore_view_buffer(&view);
		}
		void render_system::clear_culling()
		{
			for (auto& next : renderers)
				next->clear_culling();
			scene->clear_culling();
		}
		void render_system::remove_renderers()
		{
			for (auto& next : renderers)
			{
				next->deactivate();
				core::memory::release(next);
			}
			renderers.clear();
		}
		void render_system::restore_view_buffer(viewer* buffer)
		{
			VI_ASSERT(device != nullptr, "graphics device should be set");
			if (&view != buffer)
			{
				if (buffer == nullptr)
				{
					auto* viewer = (components::camera*)scene->camera.load();
					if (viewer != nullptr)
						viewer->get_viewer(&view);
				}
				else
					view = *buffer;
			}

			if (view.culling == render_culling::depth)
				indexing.frustum = trigonometry::frustum6p(view.view_projection);
			else if (view.culling == render_culling::depth_cube)
				indexing.bounds = trigonometry::bounding(view.position - view.far_plane, view.position + view.far_plane);

			constants->view.inv_view_proj = view.inv_view_projection;
			constants->view.view_proj = view.view_projection;
			constants->view.proj = view.projection;
			constants->view.view = view.view;
			constants->view.position = view.position;
			constants->view.direction = view.rotation.ddirection();
			constants->view.far = view.far_plane;
			constants->view.near = view.near_plane;
			constants->update_constant_buffer(render_buffer_type::view);
		}
		void render_system::remount(renderer* target)
		{
			VI_ASSERT(target != nullptr, "renderer should be set");
			target->deactivate();
			target->set_renderer(this);
			target->activate();
			target->resize_buffers();
		}
		void render_system::remount()
		{
			clear_culling();
			for (auto& next : renderers)
				remount(next);
		}
		void render_system::mount()
		{
			clear_culling();
			for (auto& next : renderers)
				next->activate();
		}
		void render_system::unmount()
		{
			for (auto& next : renderers)
				next->deactivate();
		}
		void render_system::move_renderer(uint64_t id, size_t offset)
		{
			if (offset == 0)
				return;

			for (size_t i = 0; i < renderers.size(); i++)
			{
				if (renderers[i]->get_id() != id)
					continue;

				if (i + offset < 0 || i + offset >= renderers.size())
					return;

				renderer* swap = renderers[i + offset];
				renderers[i + offset] = renderers[i];
				renderers[i] = swap;
				return;
			}
		}
		void render_system::remove_renderer(uint64_t id)
		{
			for (auto it = renderers.begin(); it != renderers.end(); ++it)
			{
				if (*it && (*it)->get_id() == id)
				{
					(*it)->deactivate();
					core::memory::release(*it);
					renderers.erase(it);
					break;
				}
			}
		}
		void render_system::restore_output()
		{
			scene->set_mrt(target_type::main, false);
		}
		void render_system::free_shader(const std::string_view& name, graphics::shader* shader)
		{
			shader_cache* cache = scene->get_shaders();
			if (cache != nullptr)
			{
				if (cache->has(name))
					return;
			}

			core::memory::release(shader);
		}
		void render_system::free_shader(graphics::shader* shader)
		{
			shader_cache* cache = scene->get_shaders();
			if (cache != nullptr)
				return free_shader(cache->find(shader), shader);

			core::memory::release(shader);
		}
		void render_system::free_buffers(const std::string_view& name, graphics::element_buffer** buffers)
		{
			if (!buffers)
				return;

			primitive_cache* cache = scene->get_primitives();
			if (cache != nullptr)
			{
				if (cache->has(name))
					return;
			}

			core::memory::release(buffers[0]);
			core::memory::release(buffers[1]);
		}
		void render_system::free_buffers(graphics::element_buffer** buffers)
		{
			if (!buffers)
				return;

			primitive_cache* cache = scene->get_primitives();
			if (cache != nullptr)
				return free_buffers(cache->find(buffers), buffers);

			core::memory::release(buffers[0]);
			core::memory::release(buffers[1]);
		}
		void render_system::set_constant_buffer(render_buffer_type buffer, uint32_t slot, uint32_t type)
		{
			constants->set_constant_buffer(buffer, slot, type);
		}
		void render_system::set_updated_constant_buffer(render_buffer_type buffer, uint32_t slot, uint32_t type)
		{
			constants->set_updated_constant_buffer(buffer, slot, type);
		}
		void render_system::update_constant_buffer(render_buffer_type buffer)
		{
			constants->update_constant_buffer(buffer);
		}
		graphics::shader* render_system::get_basic_effect() const
		{
			return constants->get_basic_effect();
		}
		void render_system::clear_materials()
		{
			base_material = nullptr;
		}
		void render_system::fetch_visibility(component* base, visibility_query& data)
		{
			auto* varying = (drawable*)base;
			auto& snapshot = base->parent->snapshot;
			snapshot.distance = base->parent->transform->get_position().distance(view.position);
			snapshot.visibility = std::max<float>(0.0f, 1.0f - snapshot.distance / (view.far_plane + base->parent->get_radius()));
			if (occlusion_culling && snapshot.visibility >= threshold && state.is_top() && base->is_drawable())
			{
				snapshot.visibility = varying->overlapping;
				data.category = varying->get_category();
				data.query_pixels = (data.category == geo_category::opaque);
			}
			else
			{
				data.category = varying->get_category();
				data.query_pixels = false;
			}
			data.boundary_visible = snapshot.visibility >= threshold;
		}
		size_t render_system::render(core::timer* time, render_state stage, render_opt options)
		{
			VI_ASSERT(time != nullptr, "timer should be set");

			render_opt last_options = state.options;
			render_state last_target = state.target;
			size_t count = 0;

			state.top++;
			state.options = options;
			state.target = stage;

			for (auto& next : renderers)
			{
				if (next->active)
					next->begin_pass(time);
			}

			for (auto& next : renderers)
			{
				if (next->active)
					count += next->render_prepass(time);
			}

			for (auto& next : renderers)
			{
				if (next->active)
					count += next->render_pass(time);
			}

			for (auto& next : renderers)
			{
				if (next->active)
					next->end_pass();
			}

			state.target = last_target;
			state.options = last_options;
			state.top--;

			return count;
		}
		bool render_system::try_instance(material* next, render_buffer::instance& target)
		{
			if (!next)
				return false;

			target.diffuse = (float)(next->diffuse_map != nullptr);
			target.normal = (float)(next->normal_map != nullptr);
			target.height = (float)(next->height_map != nullptr);
			target.material_id = (float)next->slot;

			return true;
		}
		bool render_system::try_geometry(material* next, material::slots* slotdata)
		{
			if (!next)
				return false;

			if (next == base_material)
				return true;

			base_material = next;
			constants->render.diffuse = (float)(next->diffuse_map != nullptr);
			constants->render.normal = (float)(next->normal_map != nullptr);
			constants->render.height = (float)(next->height_map != nullptr);
			constants->render.material_id = (float)next->slot;

			if (slotdata != nullptr)
			{
				if (slotdata->diffuse_map != (uint32_t)-1)
					device->set_texture_2d(next->diffuse_map, slotdata->diffuse_map, VI_PS);

				if (slotdata->normal_map != (uint32_t)-1)
					device->set_texture_2d(next->normal_map, slotdata->normal_map, VI_PS);

				if (slotdata->metallic_map != (uint32_t)-1)
					device->set_texture_2d(next->metallic_map, slotdata->metallic_map, VI_PS);

				if (slotdata->roughness_map != (uint32_t)-1)
					device->set_texture_2d(next->roughness_map, slotdata->roughness_map, VI_PS);

				if (slotdata->height_map != (uint32_t)-1)
					device->set_texture_2d(next->height_map, slotdata->height_map, VI_PS);

				if (slotdata->occlusion_map != (uint32_t)-1)
					device->set_texture_2d(next->occlusion_map, slotdata->occlusion_map, VI_PS);

				if (slotdata->emission_map != (uint32_t)-1)
					device->set_texture_2d(next->emission_map, slotdata->emission_map, VI_PS);
			}

			return true;
		}
		bool render_system::has_category(geo_category category)
		{
			for (auto* next : renderers)
			{
				if (next->active && next->has_category(category))
					return true;
			}

			return false;
		}
		graphics::expects_graphics<void> render_system::compile_buffers(graphics::element_buffer** result, const std::string_view& name, size_t element_size, size_t elements_count)
		{
			VI_ASSERT(result != nullptr, "result should be set");
			VI_ASSERT(!name.empty(), "buffers must have a name");

			primitive_cache* cache = scene->get_primitives();
			if (cache != nullptr)
				return cache->compile(result, name, element_size, elements_count);

			graphics::element_buffer::desc f = graphics::element_buffer::desc();
			f.access_flags = graphics::cpu_access::write;
			f.usage = graphics::resource_usage::dynamic;
			f.bind_flags = graphics::resource_bind::vertex_buffer;
			f.element_width = (uint32_t)element_size;
			f.element_count = (uint32_t)elements_count;

			auto vertex_buffer = device->create_element_buffer(f);
			if (!vertex_buffer)
				return vertex_buffer.error();

			f = graphics::element_buffer::desc();
			f.access_flags = graphics::cpu_access::write;
			f.usage = graphics::resource_usage::dynamic;
			f.bind_flags = graphics::resource_bind::index_buffer;
			f.element_width = (uint32_t)sizeof(int);
			f.element_count = (uint32_t)elements_count * 3;

			auto index_buffer = device->create_element_buffer(f);
			if (!index_buffer)
			{
				core::memory::release(*vertex_buffer);
				return index_buffer.error();
			}

			result[(size_t)buffer_type::index] = *index_buffer;
			result[(size_t)buffer_type::vertex] = *vertex_buffer;
			return core::expectation::met;
		}
		graphics::expects_graphics<graphics::shader*> render_system::compile_shader(graphics::shader::desc& desc, size_t buffer_size)
		{
			VI_ASSERT(!desc.filename.empty(), "shader must have a name");
			shader_cache* cache = scene->get_shaders();
			if (cache != nullptr)
				return cache->compile(desc.filename, desc, buffer_size);

			auto shader = device->create_shader(desc);
			if (!shader)
				return shader;
			else if (buffer_size > 0)
				device->update_buffer_size(*shader, buffer_size);

			return shader;
		}
		graphics::expects_graphics<graphics::shader*> render_system::compile_shader(const std::string_view& section_name, core::vector<core::string>&& features, size_t buffer_size)
		{
			graphics::shader::desc i = graphics::shader::desc();
			if (!device->get_section_data(section_name, &i))
				return nullptr;

			i.defines.reserve(i.defines.size() + features.size());
			for (auto& feature : features)
				i.defines.push_back(std::move(feature));

			return compile_shader(i, buffer_size);
		}
		renderer* render_system::add_renderer(renderer* in)
		{
			VI_ASSERT(in != nullptr, "renderer should be set");
			for (auto it = renderers.begin(); it != renderers.end(); ++it)
			{
				if (*it && (*it)->get_id() == in->get_id())
				{
					if (*it == in)
						return in;

					(*it)->deactivate();
					core::memory::release(*it);
					renderers.erase(it);
					break;
				}
			}

			in->set_renderer(this);
			in->activate();
			in->resize_buffers();
			renderers.push_back(in);

			return in;
		}
		renderer* render_system::get_renderer(uint64_t id)
		{
			for (auto& next : renderers)
			{
				if (next->get_id() == id)
					return next;
			}

			return nullptr;
		}
		bool render_system::get_offset(uint64_t id, size_t& offset) const
		{
			for (size_t i = 0; i < renderers.size(); i++)
			{
				if (renderers[i]->get_id() == id)
				{
					offset = i;
					return true;
				}
			}

			return false;
		}
		core::vector<renderer*>& render_system::get_renderers()
		{
			return renderers;
		}
		graphics::multi_render_target_2d* render_system::get_mrt(target_type type) const
		{
			return scene->get_mrt(type);
		}
		graphics::render_target_2d* render_system::get_rt(target_type type) const
		{
			return scene->get_rt(type);
		}
		graphics::element_buffer* render_system::get_material_buffer() const
		{
			return scene->get_material_buffer();
		}
		graphics::graphics_device* render_system::get_device() const
		{
			return device;
		}
		graphics::texture_2d** render_system::get_merger()
		{
			return scene->get_merger();
		}
		render_constants* render_system::get_constants() const
		{
			return constants;
		}
		primitive_cache* render_system::get_primitives() const
		{
			return scene->get_primitives();
		}
		scene_graph* render_system::get_scene() const
		{
			return scene;
		}
		component* render_system::get_component() const
		{
			return owner;
		}
		sparse_index& render_system::get_storage_wrapper(uint64_t section)
		{
			return scene->get_storage(section);
		}
		void render_system::watch(core::vector<core::promise<void>>&& tasks)
		{
			scene->watch(task_type::rendering, std::move(tasks));
		}

		shader_cache::shader_cache(graphics::graphics_device* new_device) noexcept : device(new_device)
		{
		}
		shader_cache::~shader_cache() noexcept
		{
			clear_cache();
		}
		graphics::expects_graphics<graphics::shader*> shader_cache::compile(const std::string_view& name, const graphics::shader::desc& desc, size_t buffer_size)
		{
			auto program = device->get_program_name(desc).or_else(core::string(name));
			graphics::shader* shader = get(program);
			if (shader != nullptr)
				return shader;

			auto resource = device->create_shader(desc);
			if (!resource || !resource->is_valid())
			{
				core::memory::release(*resource);
				if (!resource)
					return resource.error();

				return nullptr;
			}
			else if (buffer_size > 0)
				device->update_buffer_size(*resource, buffer_size);

			core::umutex<std::mutex> unique(exclusive);
			scache& result = cache[program];
			result.shader = *resource;
			result.count = 1;
			return *resource;
		}
		graphics::shader* shader_cache::get(const std::string_view& name)
		{
			core::umutex<std::mutex> unique(exclusive);
			auto it = cache.find(core::key_lookup_cast(name));
			if (it != cache.end())
			{
				it->second.count++;
				return it->second.shader;
			}

			return nullptr;
		}
		core::string shader_cache::find(graphics::shader* shader)
		{
			VI_ASSERT(shader != nullptr, "shader should be set");
			core::umutex<std::mutex> unique(exclusive);
			for (auto& item : cache)
			{
				if (item.second.shader == shader)
					return item.first;
			}

			return core::string();
		}
		const core::unordered_map<core::string, shader_cache::scache>& shader_cache::get_caches() const
		{
			return cache;
		}
		bool shader_cache::has(const std::string_view& name)
		{
			core::umutex<std::mutex> unique(exclusive);
			auto it = cache.find(core::key_lookup_cast(name));
			return it != cache.end();
		}
		bool shader_cache::free(const std::string_view& name, graphics::shader* shader)
		{
			core::umutex<std::mutex> unique(exclusive);
			auto it = cache.find(core::key_lookup_cast(name));
			if (it == cache.end())
				return false;

			if (shader != nullptr && shader != it->second.shader)
				return false;

			it->second.count--;
			if (it->second.count > 0)
				return true;

			core::memory::release(it->second.shader);
			cache.erase(it);
			return true;
		}
		void shader_cache::clear_cache()
		{
			core::umutex<std::mutex> unique(exclusive);
			for (auto it = cache.begin(); it != cache.end(); ++it)
				core::memory::release(it->second.shader);
			cache.clear();
		}

		primitive_cache::primitive_cache(graphics::graphics_device* ref) noexcept : device(ref), quad(nullptr), box_model(nullptr), skin_box_model(nullptr)
		{
			sphere[0] = sphere[1] = nullptr;
			cube[0] = cube[1] = nullptr;
			box[0] = box[1] = nullptr;
			skin_box[0] = skin_box[1] = nullptr;
		}
		primitive_cache::~primitive_cache() noexcept
		{
			clear_cache();
		}
		graphics::expects_graphics<void> primitive_cache::compile(graphics::element_buffer** results, const std::string_view& name, size_t element_size, size_t elements_count)
		{
			VI_ASSERT(results != nullptr, "results should be set");
			if (get(results, name))
				return core::expectation::met;

			graphics::element_buffer::desc f = graphics::element_buffer::desc();
			f.access_flags = graphics::cpu_access::write;
			f.usage = graphics::resource_usage::dynamic;
			f.bind_flags = graphics::resource_bind::vertex_buffer;
			f.element_width = (uint32_t)element_size;
			f.element_count = (uint32_t)elements_count;

			auto vertex_buffer = device->create_element_buffer(f);
			if (!vertex_buffer)
				return vertex_buffer.error();

			f = graphics::element_buffer::desc();
			f.access_flags = graphics::cpu_access::write;
			f.usage = graphics::resource_usage::dynamic;
			f.bind_flags = graphics::resource_bind::index_buffer;
			f.element_width = (uint32_t)sizeof(int);
			f.element_count = (uint32_t)elements_count * 3;

			auto index_buffer = device->create_element_buffer(f);
			if (!index_buffer)
			{
				core::memory::release(*vertex_buffer);
				return index_buffer.error();
			}

			core::umutex<std::recursive_mutex> unique(exclusive);
			scache& result = cache[core::string(name)];
			result.buffers[(size_t)buffer_type::index] = results[(size_t)buffer_type::index] = *index_buffer;
			result.buffers[(size_t)buffer_type::vertex] = results[(size_t)buffer_type::vertex] = *vertex_buffer;
			result.count = 1;
			return core::expectation::met;
		}
		bool primitive_cache::get(graphics::element_buffer** results, const std::string_view& name)
		{
			VI_ASSERT(results != nullptr, "results should be set");
			core::umutex<std::recursive_mutex> unique(exclusive);
			auto it = cache.find(core::key_lookup_cast(name));
			if (it == cache.end())
				return false;

			it->second.count++;
			results[(size_t)buffer_type::index] = it->second.buffers[(size_t)buffer_type::index];
			results[(size_t)buffer_type::vertex] = it->second.buffers[(size_t)buffer_type::vertex];
			return true;
		}
		bool primitive_cache::has(const std::string_view& name)
		{
			core::umutex<std::recursive_mutex> unique(exclusive);
			auto it = cache.find(core::key_lookup_cast(name));
			return it != cache.end();
		}
		bool primitive_cache::free(const std::string_view& name, graphics::element_buffer** buffers)
		{
			core::umutex<std::recursive_mutex> unique(exclusive);
			auto it = cache.find(core::key_lookup_cast(name));
			if (it == cache.end())
				return false;

			if (buffers != nullptr)
			{
				if ((buffers[0] != nullptr && buffers[0] != it->second.buffers[0]) || (buffers[1] != nullptr && buffers[1] != it->second.buffers[1]))
					return false;
			}

			it->second.count--;
			if (it->second.count > 0)
				return true;

			core::memory::release(it->second.buffers[0]);
			core::memory::release(it->second.buffers[1]);
			cache.erase(it);
			return true;
		}
		core::string primitive_cache::find(graphics::element_buffer** buffers)
		{
			VI_ASSERT(buffers != nullptr, "buffers should be set");
			core::umutex<std::recursive_mutex> unique(exclusive);
			for (auto& item : cache)
			{
				if (item.second.buffers[0] == buffers[0] && item.second.buffers[1] == buffers[1])
					return item.first;
			}

			return core::string();
		}
		model* primitive_cache::get_box_model()
		{
			if (box_model != nullptr)
				return box_model;

			core::umutex<std::recursive_mutex> unique(exclusive);
			if (box_model != nullptr)
				return box_model;

			auto* vertex_buffer = get_box(buffer_type::vertex);
			if (vertex_buffer != nullptr)
				vertex_buffer->add_ref();

			auto* index_buffer = get_box(buffer_type::index);
			if (index_buffer != nullptr)
				index_buffer->add_ref();

			box_model = new model();
			box_model->meshes.push_back(*device->create_mesh_buffer(vertex_buffer, index_buffer));
			return box_model;
		}
		skin_model* primitive_cache::get_skin_box_model()
		{
			if (skin_box_model != nullptr)
				return skin_box_model;

			core::umutex<std::recursive_mutex> unique(exclusive);
			if (skin_box_model != nullptr)
				return skin_box_model;

			auto* vertex_buffer = get_skin_box(buffer_type::vertex);
			if (vertex_buffer != nullptr)
				vertex_buffer->add_ref();

			auto* index_buffer = get_skin_box(buffer_type::index);
			if (index_buffer != nullptr)
				index_buffer->add_ref();

			skin_box_model = new skin_model();
			skin_box_model->meshes.push_back(*device->create_skin_mesh_buffer(vertex_buffer, index_buffer));
			return skin_box_model;
		}
		graphics::element_buffer* primitive_cache::get_quad()
		{
			VI_ASSERT(device != nullptr, "graphics device should be set");
			if (quad != nullptr)
				return quad;

			core::vector<trigonometry::shape_vertex> elements =
			{
				{ -1.0f, -1.0f, 0, -1, 0 },
				{ -1.0f, 1.0f, 0, -1, -1 },
				{ 1.0f, 1.0f, 0, 0, -1 },
				{ -1.0f, -1.0f, 0, -1, 0 },
				{ 1.0f, 1.0f, 0, 0, -1 },
				{ 1.0f, -1.0f, 0, 0, 0 }
			};
			trigonometry::geometric::texcoord_rh_to_lh(elements);

			graphics::element_buffer::desc f = graphics::element_buffer::desc();
			f.access_flags = graphics::cpu_access::none;
			f.usage = graphics::resource_usage::defaults;
			f.bind_flags = graphics::resource_bind::vertex_buffer;
			f.element_count = 6;
			f.element_width = sizeof(trigonometry::shape_vertex);
			f.elements = &elements[0];

			core::umutex<std::recursive_mutex> unique(exclusive);
			if (quad != nullptr)
				return quad;

			quad = *device->create_element_buffer(f);
			return quad;
		}
		graphics::element_buffer* primitive_cache::get_sphere(buffer_type type)
		{
			VI_ASSERT(device != nullptr, "graphics device should be set");
			if (sphere[(size_t)type] != nullptr)
				return sphere[(size_t)type];

			if (type == buffer_type::index)
			{
				core::vector<int> indices =
				{
					0, 4, 1, 0,
					9, 4, 9, 5,
					4, 4, 5, 8,
					4, 8, 1, 8,
					10, 1, 8, 3,
					10, 5, 3, 8,
					5, 2, 3, 2,
					7, 3, 7, 10,
					3, 7, 6, 10,
					7, 11, 6, 11,
					0, 6, 0, 1,
					6, 6, 1, 10,
					9, 0, 11, 9,
					11, 2, 9, 2,
					5, 7, 2, 11
				};

				graphics::element_buffer::desc f = graphics::element_buffer::desc();
				f.access_flags = graphics::cpu_access::none;
				f.usage = graphics::resource_usage::defaults;
				f.bind_flags = graphics::resource_bind::index_buffer;
				f.element_count = (uint32_t)indices.size();
				f.element_width = sizeof(int);
				f.elements = &indices[0];

				core::umutex<std::recursive_mutex> unique(exclusive);
				if (!sphere[(size_t)buffer_type::index])
					sphere[(size_t)buffer_type::index] = *device->create_element_buffer(f);

				return sphere[(size_t)buffer_type::index];
			}
			else if (type == buffer_type::vertex)
			{
				const float x = 0.525731112119133606f;
				const float z = 0.850650808352039932f;
				const float n = 0.0f;

				core::vector<trigonometry::shape_vertex> elements =
				{
					{ -x, n, z },
					{ x, n, z },
					{ -x, n, -z },
					{ x, n, -z },
					{ n, z, x },
					{ n, z, -x },
					{ n, -z, x },
					{ n, -z, -x },
					{ z, x, n },
					{ -z, x, n },
					{ z, -x, n },
					{ -z, -x, n }
				};
				trigonometry::geometric::texcoord_rh_to_lh(elements);

				graphics::element_buffer::desc f = graphics::element_buffer::desc();
				f.access_flags = graphics::cpu_access::none;
				f.usage = graphics::resource_usage::defaults;
				f.bind_flags = graphics::resource_bind::vertex_buffer;
				f.element_count = (uint32_t)elements.size();
				f.element_width = sizeof(trigonometry::shape_vertex);
				f.elements = &elements[0];

				core::umutex<std::recursive_mutex> unique(exclusive);
				if (!sphere[(size_t)buffer_type::vertex])
					sphere[(size_t)buffer_type::vertex] = *device->create_element_buffer(f);

				return sphere[(size_t)buffer_type::vertex];
			}

			return nullptr;
		}
		graphics::element_buffer* primitive_cache::get_cube(buffer_type type)
		{
			VI_ASSERT(device != nullptr, "graphics device should be set");
			if (cube[(size_t)type] != nullptr)
				return cube[(size_t)type];

			if (type == buffer_type::index)
			{
				core::vector<int> indices =
				{
					0, 1, 2, 0,
					18, 1, 3, 4,
					5, 3, 19, 4,
					6, 7, 8, 6,
					20, 7, 9, 10,
					11, 9, 21, 10,
					12, 13, 14, 12,
					22, 13, 15, 16,
					17, 15, 23, 16
				};

				graphics::element_buffer::desc f = graphics::element_buffer::desc();
				f.access_flags = graphics::cpu_access::none;
				f.usage = graphics::resource_usage::defaults;
				f.bind_flags = graphics::resource_bind::index_buffer;
				f.element_count = (uint32_t)indices.size();
				f.element_width = sizeof(int);
				f.elements = &indices[0];

				core::umutex<std::recursive_mutex> unique(exclusive);
				if (!cube[(size_t)buffer_type::index])
					cube[(size_t)buffer_type::index] = *device->create_element_buffer(f);

				return cube[(size_t)buffer_type::index];
			}
			else if (type == buffer_type::vertex)
			{
				core::vector<trigonometry::shape_vertex> elements
				{
					{ 1, 1, 1, 0.875, -0.5 },
					{ -1, -1, 1, 0.625, -0.75 },
					{ -1, 1, 1, 0.625, -0.5 },
					{ -1, -1, 1, 0.625, -0.75 },
					{ 1, -1, -1, 0.375, -1 },
					{ -1, -1, -1, 0.375, -0.75 },
					{ 1, -1, 1, 0.625, -0 },
					{ 1, 1, -1, 0.375, -0.25 },
					{ 1, -1, -1, 0.375, -0 },
					{ -1, 1, -1, 0.375, -0.5 },
					{ 1, -1, -1, 0.125, -0.75 },
					{ 1, 1, -1, 0.125, -0.5 },
					{ -1, 1, 1, 0.625, -0.5 },
					{ -1, -1, -1, 0.375, -0.75 },
					{ -1, 1, -1, 0.375, -0.5 },
					{ 1, 1, 1, 0.625, -0.25 },
					{ -1, 1, -1, 0.375, -0.5 },
					{ 1, 1, -1, 0.375, -0.25 },
					{ 1, -1, 1, 0.875, -0.75 },
					{ 1, -1, 1, 0.625, -1 },
					{ 1, 1, 1, 0.625, -0.25 },
					{ -1, -1, -1, 0.375, -0.75 },
					{ -1, -1, 1, 0.625, -0.75 },
					{ -1, 1, 1, 0.625, -0.5 }
				};
				trigonometry::geometric::texcoord_rh_to_lh(elements);

				graphics::element_buffer::desc f = graphics::element_buffer::desc();
				f.access_flags = graphics::cpu_access::none;
				f.usage = graphics::resource_usage::defaults;
				f.bind_flags = graphics::resource_bind::vertex_buffer;
				f.element_count = (uint32_t)elements.size();
				f.element_width = sizeof(trigonometry::shape_vertex);
				f.elements = &elements[0];

				core::umutex<std::recursive_mutex> unique(exclusive);
				if (!cube[(size_t)buffer_type::vertex])
					cube[(size_t)buffer_type::vertex] = *device->create_element_buffer(f);

				return cube[(size_t)buffer_type::vertex];
			}

			return nullptr;
		}
		graphics::element_buffer* primitive_cache::get_box(buffer_type type)
		{
			VI_ASSERT(device != nullptr, "graphics device should be set");
			if (box[(size_t)type] != nullptr)
				return box[(size_t)type];

			if (type == buffer_type::index)
			{
				core::vector<int> indices =
				{
					16, 23, 15, 17,
					16, 15, 13, 22,
					12, 14, 13, 12,
					10, 21, 9, 11,
					10, 9, 7, 20,
					6, 8, 7, 6,
					4, 19, 3, 5,
					4, 3, 1, 18,
					0, 2, 1, 0
				};

				graphics::element_buffer::desc f = graphics::element_buffer::desc();
				f.access_flags = graphics::cpu_access::none;
				f.usage = graphics::resource_usage::defaults;
				f.bind_flags = graphics::resource_bind::index_buffer;
				f.element_count = (uint32_t)indices.size();
				f.element_width = sizeof(int);
				f.elements = &indices[0];

				core::umutex<std::recursive_mutex> unique(exclusive);
				if (!box[(size_t)buffer_type::index])
					box[(size_t)buffer_type::index] = *device->create_element_buffer(f);

				return box[(size_t)buffer_type::index];
			}
			else if (type == buffer_type::vertex)
			{
				core::vector<trigonometry::vertex> elements =
				{
					{ -1, 1, 1, 0.875, -0.5, 0, 0, 1, -1, 0, 0, 0, 1, 0 },
					{ 1, -1, 1, 0.625, -0.75, 0, 0, 1, -1, 0, 0, 0, 1, 0 },
					{ 1, 1, 1, 0.625, -0.5, 0, 0, 1, -1, 0, 0, 0, 1, 0 },
					{ 1, -1, 1, 0.625, -0.75, 0, -1, 0, 0, 0, 1, 1, 0, 0 },
					{ -1, -1, -1, 0.375, -1, 0, -1, 0, 0, 0, 1, 1, 0, 0 },
					{ 1, -1, -1, 0.375, -0.75, 0, -1, 0, 0, 0, 1, 1, 0, 0 },
					{ -1, -1, 1, 0.625, -0, -1, 0, 0, 0, 0, 1, 0, -1, 0 },
					{ -1, 1, -1, 0.375, -0.25, -1, 0, 0, 0, 0, 1, 0, -1, 0 },
					{ -1, -1, -1, 0.375, -0, -1, 0, 0, 0, 0, 1, 0, -1, 0 },
					{ 1, 1, -1, 0.375, -0.5, 0, 0, -1, 1, 0, 0, 0, 1, 0 },
					{ -1, -1, -1, 0.125, -0.75, 0, 0, -1, 1, 0, 0, 0, 1, 0 },
					{ -1, 1, -1, 0.125, -0.5, 0, 0, -1, 1, 0, 0, 0, 1, 0 },
					{ 1, 1, 1, 0.625, -0.5, 1, 0, 0, 0, 0, 1, 0, 1, 0 },
					{ 1, -1, -1, 0.375, -0.75, 1, 0, 0, 0, 0, 1, 0, 1, 0 },
					{ 1, 1, -1, 0.375, -0.5, 1, 0, 0, 0, 0, 1, 0, 1, 0 },
					{ -1, 1, 1, 0.625, -0.25, 0, 1, 0, 0, 0, 1, -1, 0, 0 },
					{ 1, 1, -1, 0.375, -0.5, 0, 1, 0, 0, 0, 1, -1, 0, 0 },
					{ -1, 1, -1, 0.375, -0.25, 0, 1, 0, 0, 0, 1, -1, 0, 0 },
					{ -1, -1, 1, 0.875, -0.75, 0, 0, 1, -1, 0, 0, 0, 1, 0 },
					{ -1, -1, 1, 0.625, -1, 0, -1, 0, 0, 0, 1, 1, 0, 0 },
					{ -1, 1, 1, 0.625, -0.25, -1, 0, 0, 0, 0, 1, 0, -1, 0 },
					{ 1, -1, -1, 0.375, -0.75, 0, 0, -1, 1, 0, 0, 0, 1, 0 },
					{ 1, -1, 1, 0.625, -0.75, 1, 0, 0, 0, 0, 1, 0, 1, 0 },
					{ 1, 1, 1, 0.625, -0.5, 0, 1, 0, 0, 0, 1, -1, 0, 0 }
				};
				trigonometry::geometric::texcoord_rh_to_lh(elements);

				graphics::element_buffer::desc f = graphics::element_buffer::desc();
				f.access_flags = graphics::cpu_access::none;
				f.usage = graphics::resource_usage::defaults;
				f.bind_flags = graphics::resource_bind::vertex_buffer;
				f.element_count = (uint32_t)elements.size();
				f.element_width = sizeof(trigonometry::vertex);
				f.elements = &elements[0];

				core::umutex<std::recursive_mutex> unique(exclusive);
				if (!box[(size_t)buffer_type::vertex])
					box[(size_t)buffer_type::vertex] = *device->create_element_buffer(f);

				return box[(size_t)buffer_type::vertex];
			}

			return nullptr;
		}
		graphics::element_buffer* primitive_cache::get_skin_box(buffer_type type)
		{
			VI_ASSERT(device != nullptr, "graphics device should be set");
			if (skin_box[(size_t)type] != nullptr)
				return skin_box[(size_t)type];

			if (type == buffer_type::index)
			{
				core::vector<int> indices =
				{
					16, 23, 15, 17,
					16, 15, 13, 22,
					12, 14, 13, 12,
					10, 21, 9, 11,
					10, 9, 7, 20,
					6, 8, 7, 6,
					4, 19, 3, 5,
					4, 3, 1, 18,
					0, 2, 1, 0
				};

				graphics::element_buffer::desc f = graphics::element_buffer::desc();
				f.access_flags = graphics::cpu_access::none;
				f.usage = graphics::resource_usage::defaults;
				f.bind_flags = graphics::resource_bind::index_buffer;
				f.element_count = (uint32_t)indices.size();
				f.element_width = sizeof(int);
				f.elements = &indices[0];

				core::umutex<std::recursive_mutex> unique(exclusive);
				if (!skin_box[(size_t)buffer_type::index])
					skin_box[(size_t)buffer_type::index] = *device->create_element_buffer(f);

				return skin_box[(size_t)buffer_type::index];
			}
			else if (type == buffer_type::vertex)
			{
				core::vector<trigonometry::skin_vertex> elements =
				{
					{ -1, 1, 1, 0.875, -0.5, 0, 0, 1, -1, 0, 0, 0, 1, 0, -1, -1, -1, -1, 0, 0, 0, 0 },
					{ 1, -1, 1, 0.625, -0.75, 0, 0, 1, -1, 0, 0, 0, 1, 0, -1, -1, -1, -1, 0, 0, 0, 0 },
					{ 1, 1, 1, 0.625, -0.5, 0, 0, 1, -1, 0, 0, 0, 1, 0, -1, -1, -1, -1, 0, 0, 0, 0 },
					{ 1, -1, 1, 0.625, -0.75, 0, -1, 0, 0, 0, 1, 1, 0, 0, -1, -1, -1, -1, 0, 0, 0, 0 },
					{ -1, -1, -1, 0.375, -1, 0, -1, 0, 0, 0, 1, 1, 0, 0, -1, -1, -1, -1, 0, 0, 0, 0 },
					{ 1, -1, -1, 0.375, -0.75, 0, -1, 0, 0, 0, 1, 1, 0, 0, -1, -1, -1, -1, 0, 0, 0, 0 },
					{ -1, -1, 1, 0.625, -0, -1, 0, 0, 0, 0, 1, 0, -1, 0, -1, -1, -1, -1, 0, 0, 0, 0 },
					{ -1, 1, -1, 0.375, -0.25, -1, 0, 0, 0, 0, 1, 0, -1, 0, -1, -1, -1, -1, 0, 0, 0, 0 },
					{ -1, -1, -1, 0.375, -0, -1, 0, 0, 0, 0, 1, 0, -1, 0, -1, -1, -1, -1, 0, 0, 0, 0 },
					{ 1, 1, -1, 0.375, -0.5, 0, 0, -1, 1, 0, 0, 0, 1, 0, -1, -1, -1, -1, 0, 0, 0, 0 },
					{ -1, -1, -1, 0.125, -0.75, 0, 0, -1, 1, 0, 0, 0, 1, 0, -1, -1, -1, -1, 0, 0, 0, 0 },
					{ -1, 1, -1, 0.125, -0.5, 0, 0, -1, 1, 0, 0, 0, 1, 0, -1, -1, -1, -1, 0, 0, 0, 0 },
					{ 1, 1, 1, 0.625, -0.5, 1, 0, 0, 0, 0, 1, 0, 1, 0, -1, -1, -1, -1, 0, 0, 0, 0 },
					{ 1, -1, -1, 0.375, -0.75, 1, 0, 0, 0, 0, 1, 0, 1, 0, -1, -1, -1, -1, 0, 0, 0, 0 },
					{ 1, 1, -1, 0.375, -0.5, 1, 0, 0, 0, 0, 1, 0, 1, 0, -1, -1, -1, -1, 0, 0, 0, 0 },
					{ -1, 1, 1, 0.625, -0.25, 0, 1, 0, 0, 0, 1, -1, 0, 0, -1, -1, -1, -1, 0, 0, 0, 0 },
					{ 1, 1, -1, 0.375, -0.5, 0, 1, 0, 0, 0, 1, -1, 0, 0, -1, -1, -1, -1, 0, 0, 0, 0 },
					{ -1, 1, -1, 0.375, -0.25, 0, 1, 0, 0, 0, 1, -1, 0, 0, -1, -1, -1, -1, 0, 0, 0, 0 },
					{ -1, -1, 1, 0.875, -0.75, 0, 0, 1, -1, 0, 0, 0, 1, 0, -1, -1, -1, -1, 0, 0, 0, 0 },
					{ -1, -1, 1, 0.625, -1, 0, -1, 0, 0, 0, 1, 1, 0, 0, -1, -1, -1, -1, 0, 0, 0, 0 },
					{ -1, 1, 1, 0.625, -0.25, -1, 0, 0, 0, 0, 1, 0, -1, 0, -1, -1, -1, -1, 0, 0, 0, 0 },
					{ 1, -1, -1, 0.375, -0.75, 0, 0, -1, 1, 0, 0, 0, 1, 0, -1, -1, -1, -1, 0, 0, 0, 0 },
					{ 1, -1, 1, 0.625, -0.75, 1, 0, 0, 0, 0, 1, 0, 1, 0, -1, -1, -1, -1, 0, 0, 0, 0 },
					{ 1, 1, 1, 0.625, -0.5, 0, 1, 0, 0, 0, 1, -1, 0, 0, -1, -1, -1, -1, 0, 0, 0, 0 }
				};
				trigonometry::geometric::texcoord_rh_to_lh(elements);

				graphics::element_buffer::desc f = graphics::element_buffer::desc();
				f.access_flags = graphics::cpu_access::none;
				f.usage = graphics::resource_usage::defaults;
				f.bind_flags = graphics::resource_bind::vertex_buffer;
				f.element_count = (uint32_t)elements.size();
				f.element_width = sizeof(trigonometry::skin_vertex);
				f.elements = &elements[0];

				core::umutex<std::recursive_mutex> unique(exclusive);
				if (!skin_box[(size_t)buffer_type::vertex])
					skin_box[(size_t)buffer_type::vertex] = *device->create_element_buffer(f);

				return skin_box[(size_t)buffer_type::vertex];
			}

			return nullptr;
		}
		const core::unordered_map<core::string, primitive_cache::scache>& primitive_cache::get_caches() const
		{
			return cache;
		}
		void primitive_cache::get_sphere_buffers(graphics::element_buffer** result)
		{
			VI_ASSERT(result != nullptr, "result should be set");
			result[(size_t)buffer_type::index] = get_sphere(buffer_type::index);
			result[(size_t)buffer_type::vertex] = get_sphere(buffer_type::vertex);
		}
		void primitive_cache::get_cube_buffers(graphics::element_buffer** result)
		{
			VI_ASSERT(result != nullptr, "result should be set");
			result[(size_t)buffer_type::index] = get_cube(buffer_type::index);
			result[(size_t)buffer_type::vertex] = get_cube(buffer_type::vertex);
		}
		void primitive_cache::get_box_buffers(graphics::element_buffer** result)
		{
			VI_ASSERT(result != nullptr, "result should be set");
			result[(size_t)buffer_type::index] = get_box(buffer_type::index);
			result[(size_t)buffer_type::vertex] = get_box(buffer_type::vertex);
		}
		void primitive_cache::get_skin_box_buffers(graphics::element_buffer** result)
		{
			VI_ASSERT(result != nullptr, "result should be set");
			result[(size_t)buffer_type::index] = get_skin_box(buffer_type::index);
			result[(size_t)buffer_type::vertex] = get_skin_box(buffer_type::vertex);
		}
		void primitive_cache::clear_cache()
		{
			core::umutex<std::recursive_mutex> unique(exclusive);
			for (auto it = cache.begin(); it != cache.end(); ++it)
			{
				core::memory::release(it->second.buffers[0]);
				core::memory::release(it->second.buffers[1]);
			}

			cache.clear();
			core::memory::release(box_model);
			core::memory::release(skin_box_model);
			core::memory::release(sphere[(size_t)buffer_type::index]);
			core::memory::release(sphere[(size_t)buffer_type::vertex]);
			core::memory::release(cube[(size_t)buffer_type::index]);
			core::memory::release(cube[(size_t)buffer_type::vertex]);
			core::memory::release(box[(size_t)buffer_type::index]);
			core::memory::release(box[(size_t)buffer_type::vertex]);
			core::memory::release(skin_box[(size_t)buffer_type::index]);
			core::memory::release(skin_box[(size_t)buffer_type::vertex]);
			core::memory::release(quad);
		}

		template <typename t>
		static void upgrade_buffer_by_rate(core::pool<t>& storage, float grow)
		{
			double size = (double)storage.capacity();
			size *= 1.0 + grow;

			VI_TRACE("[scene] apply buffer 0x%" PRIXPTR " +%" PRIu64 " bytes", (void*)storage.get(), (uint64_t)(sizeof(t) * (size - storage.capacity())));
			storage.reserve((size_t)size);
		}
		template <typename t>
		static void upgrade_buffer_by_size(core::pool<t>& storage, size_t size)
		{
			VI_TRACE("[scene] apply buffer 0x%" PRIXPTR " +%" PRIu64 " bytes", (void*)storage.get(), (uint64_t)(sizeof(t) * (size - storage.capacity())));
			storage.reserve(size);
		}

		void scene_graph::desc::add_ref()
		{
			if (shared.constants != nullptr)
				shared.constants->add_ref();

			if (shared.shaders != nullptr)
				shared.shaders->add_ref();

			if (shared.primitives != nullptr)
				shared.primitives->add_ref();

			if (shared.content != nullptr)
				shared.content->add_ref();

			if (shared.device != nullptr)
				shared.device->add_ref();

			if (shared.activity != nullptr)
				shared.activity->add_ref();

			if (shared.vm != nullptr)
				shared.vm->add_ref();
		}
		void scene_graph::desc::release()
		{
			if (shared.constants != nullptr)
			{
				if (shared.constants->get_ref_count() == 1)
					core::memory::release(shared.constants);
				else
					shared.constants->release();
			}

			if (shared.shaders != nullptr)
			{
				if (shared.shaders->get_ref_count() == 1)
					core::memory::release(shared.shaders);
				else
					shared.shaders->release();
			}

			if (shared.primitives != nullptr)
			{
				if (shared.primitives->get_ref_count() == 1)
					core::memory::release(shared.primitives);
				else
					shared.primitives->release();
			}

			if (shared.content != nullptr)
			{
				if (shared.content->get_ref_count() == 1)
					core::memory::release(shared.content);
				else
					shared.content->release();
			}

			if (shared.device != nullptr)
			{
				if (shared.device->get_ref_count() == 1)
					core::memory::release(shared.device);
				else
					shared.device->release();
			}

			if (shared.activity != nullptr)
			{
				if (shared.activity->get_ref_count() == 1)
					core::memory::release(shared.activity);
				else
					shared.activity->release();
			}

			if (shared.vm != nullptr)
			{
				if (shared.vm->get_ref_count() == 1)
					core::memory::release(shared.vm);
				else
					shared.vm->release();
			}
		}
		scene_graph::desc scene_graph::desc::get(heavy_application* base)
		{
			scene_graph::desc i;
			if (!base)
				return i;

			i.shared.shaders = base->cache.shaders;
			i.shared.primitives = base->cache.primitives;
			i.shared.constants = base->constants;
			i.shared.content = base->content;
			i.shared.device = base->renderer;
			i.shared.activity = base->activity;
			i.shared.vm = base->vm;
			return i;
		}

		scene_graph::scene_graph(const desc& i) noexcept : simulator(new physics::simulator(i.simulator)), camera(nullptr), active(true), conf(i), snapshot(nullptr)
		{
			for (size_t i = 0; i < (size_t)target_type::count * 2; i++)
			{
				display.mrt[i] = nullptr;
				display.rt[i] = nullptr;
			}

			auto components = core::composer::fetch((uint64_t)composer_tag::component);
			for (uint64_t section : components)
			{
				registry[section] = core::memory::init<sparse_index>();
				changes[section].clear();
			}

			display.material_buffer = nullptr;
			display.merger = nullptr;
			display.depth_stencil = nullptr;
			display.rasterizer = nullptr;
			display.blend = nullptr;
			display.sampler = nullptr;
			display.layout = nullptr;
			loading.defaults = nullptr;

			conf.add_ref();
			configure(conf);
			script_hook();
		}
		scene_graph::~scene_graph() noexcept
		{
			VI_MEASURE(core::timings::intensive);
			step_transactions();

			for (auto& item : listeners)
			{
				for (auto* listener : item.second)
					core::memory::deinit(listener);
				item.second.clear();
			}
			listeners.clear();

			for (auto& item : entities)
				core::memory::release(item);
			entities.clear();

			for (auto& item : materials)
				core::memory::release(item);
			materials.clear();

			for (auto& sparse : registry)
				core::memory::deinit(sparse.second);
			registry.clear();

			core::memory::release(display.merger);
			for (auto* item : display.points)
				core::memory::release(item);
			display.points.clear();

			for (auto* item : display.spots)
				core::memory::release(item);
			display.spots.clear();

			for (auto& item : display.lines)
			{
				if (item != nullptr)
				{
					for (auto* target : *item)
						core::memory::release(target);
					core::memory::deinit(item);
				}
			}
			display.lines.clear();

			for (size_t i = 0; i < (size_t)target_type::count; i++)
			{
				core::memory::release(display.mrt[i]);
				core::memory::release(display.rt[i]);
			}

			core::memory::release(display.material_buffer);
			core::memory::release(simulator);
			conf.release();
		}
		void scene_graph::configure(const desc& new_conf)
		{
			VI_ASSERT(new_conf.shared.device != nullptr, "graphics device should be set");
			transaction([this, new_conf]()
			{
				VI_TRACE("[scene] configure 0x%" PRIXPTR, (void*)this);
				auto* device = new_conf.shared.device;
				display.depth_stencil = device->get_depth_stencil_state("doo_soo_lt");
				display.rasterizer = device->get_rasterizer_state("so_cback");
				display.blend = device->get_blend_state("bo_wrgba_one");
				display.sampler = device->get_sampler_state("a16_fa_wrap");
				display.layout = device->get_input_layout("vx_shape");

				conf.release();
				conf = new_conf;
				conf.add_ref();

				slots.diffuse_map = *device->get_shader_slot(conf.shared.constants->get_basic_effect(), "DiffuseMap");
				slots.sampler = *device->get_shader_sampler_slot(conf.shared.constants->get_basic_effect(), "DiffuseMap", "Sampler");
				slots.object = *device->get_shader_slot(conf.shared.constants->get_basic_effect(), "ObjectBuffer");

				materials.reserve(conf.start_materials);
				entities.reserve(conf.start_entities);
				dirty.reserve(conf.start_entities);

				for (auto& sparse : registry)
					sparse.second->data.reserve(conf.start_components);

				for (size_t i = 0; i < (size_t)actor_type::count; i++)
					actors[i].reserve(conf.start_components);

				generate_material_buffer();
				generate_depth_buffers();
				resize_buffers();
				transaction([this]()
				{
					auto* base = camera.load();
					if (base != nullptr)
						base->activate(base);
				});

				auto& cameras = get_components<components::camera>();
				for (auto it = cameras.begin(); it != cameras.end(); ++it)
				{
					auto* base = (components::camera*)*it;
					base->get_renderer()->remount();
				}
			});
		}
		void scene_graph::actualize()
		{
			VI_TRACE("[scene] actualize 0x%" PRIXPTR, (void*)this);
			step_events();
			step_transactions();
		}
		void scene_graph::resize_buffers()
		{
			transaction([this]()
			{
				VI_TRACE("[scene] resize buffers 0x%" PRIXPTR, (void*)this);
				resize_render_buffers();
				if (!camera.load())
					return;

				auto& array = get_components<components::camera>();
				for (auto it = array.begin(); it != array.end(); ++it)
					((components::camera*)*it)->resize_buffers();
			});
		}
		void scene_graph::resize_render_buffers()
		{
			transaction([this]()
			{
				auto* device = conf.shared.device;
				VI_ASSERT(device != nullptr, "graphics device should be set");
				graphics::multi_render_target_2d::desc mrt = get_desc_mrt();
				graphics::render_target_2d::desc rt = get_desc_rt();
				core::memory::release(display.merger);

				for (size_t i = 0; i < (size_t)target_type::count; i++)
				{
					core::memory::release(display.mrt[i]);
					display.mrt[i] = *device->create_multi_render_target_2d(mrt);

					core::memory::release(display.rt[i]);
					display.rt[i] = *device->create_render_target_2d(rt);
				}
			});
		}
		void scene_graph::fill_material_buffers()
		{
			auto* device = conf.shared.device;
			VI_ASSERT(device != nullptr, "graphics device should be set");
			graphics::mapped_subresource stream;
			if (!device->map(display.material_buffer, graphics::resource_map::write_discard, &stream))
				return;

			size_t size = 0;
			subsurface* array = (subsurface*)stream.pointer;
			auto begin = materials.begin(), end = materials.end();
			for (auto it = begin; it != end; ++it)
			{
				subsurface& next = array[size];
				(*it)->slot = size++;
				next = (*it)->surface;
			}

			device->unmap(display.material_buffer, &stream);
		}
		void scene_graph::submit()
		{
			auto* device = conf.shared.device;
			VI_ASSERT(device != nullptr, "graphics device should be set");
			VI_ASSERT(conf.shared.primitives != nullptr, "primitive cache should be set");
			conf.shared.constants->render.texcoord = 1.0f;
			conf.shared.constants->render.transform.identify();
			conf.shared.constants->set_updated_constant_buffer(render_buffer_type::render, slots.object, VI_VS | VI_PS);
			device->set_target();
			device->set_depth_stencil_state(display.depth_stencil);
			device->set_blend_state(display.blend);
			device->set_rasterizer_state(display.rasterizer);
			device->set_input_layout(display.layout);
			device->set_sampler_state(display.sampler, slots.sampler, 1, VI_PS);
			device->set_texture_2d(display.mrt[(size_t)target_type::main]->get_target(0), slots.diffuse_map, VI_PS);
			device->set_shader(conf.shared.constants->get_basic_effect(), VI_VS | VI_PS);
			device->set_vertex_buffer(conf.shared.primitives->get_quad());
			device->draw(6, 0);
			device->set_texture_2d(nullptr, slots.diffuse_map, VI_PS);
			statistics.draw_calls++;
		}
		void scene_graph::dispatch(core::timer* time)
		{
			VI_ASSERT(time != nullptr, "time should be set");
			VI_MEASURE(core::timings::pass);

			step_events();
			step_transactions();
			step_gameplay(time);
			step_simulate(time);
			step_animate(time);
			step_synchronize(time);
			step_indexing();
			step_finalize();
		}
		void scene_graph::publish(core::timer* time)
		{
			VI_ASSERT(time != nullptr, "timer should be set");
			VI_MEASURE((uint64_t)core::timings::frame * 2);

			auto* base = (components::camera*)camera.load();
			auto* renderer = (base ? base->get_renderer() : nullptr);
			statistics.batching = 0;
			statistics.sorting = 0;
			statistics.instances = 0;
			statistics.draw_calls = 0;

			if (!renderer)
				return;

			fill_material_buffers();
			set_mrt(target_type::main, true);
			renderer->restore_view_buffer(nullptr);
			statistics.draw_calls += renderer->render(time, render_state::geometry, render_opt::none);
		}
		void scene_graph::publish_and_submit(core::timer* time, float r, float g, float b, bool is_parallel)
		{
			VI_ASSERT(conf.shared.device != nullptr, "graphics device should be set");
			if (is_parallel && false)
			{
				core::umutex<std::mutex> unique(tasking.update[(size_t)task_type::rendering]);
				if (tasking.is_rendering)
					return;

				tasking.is_rendering = true;
				core::cospawn([this, time, r, g, b]()
				{
					publish_and_submit(time, r, g, b, false);
					core::umutex<std::mutex> unique(tasking.update[(size_t)task_type::rendering]);
					tasking.is_rendering = false;
				});
			}
			else
			{
				auto* device = conf.shared.device;
				device->clear(r, g, b);
				device->clear_depth();
				publish(time);
				submit();
				device->submit();
			}
		}
		void scene_graph::step_simulate(core::timer* time)
		{
			VI_ASSERT(time != nullptr, "timer should be set");
			VI_ASSERT(simulator != nullptr, "simulator should be set");
			VI_MEASURE(core::timings::frame);

			if (!active)
				return;

			watch(task_type::processing, parallel::enqueue([this, time]()
			{
				simulator->simulate_step(time->get_elapsed());
			}));
		}
		void scene_graph::step_synchronize(core::timer* time)
		{
			VI_ASSERT(time != nullptr, "timer should be set");
			VI_MEASURE(core::timings::frame);

			auto& storage = actors[(size_t)actor_type::synchronize];
			if (!storage.empty())
			{
				watch(task_type::processing, parallel::for_each(storage.begin(), storage.end(), THRESHOLD_PER_ELEMENT, [time](component* next)
				{
					next->synchronize(time);
				}));
			}
		}
		void scene_graph::step_animate(core::timer* time)
		{
			VI_ASSERT(time != nullptr, "timer should be set");
			VI_MEASURE(core::timings::frame);

			auto& storage = actors[(size_t)actor_type::animate];
			if (active && !storage.empty())
			{
				watch(task_type::processing, parallel::for_each(storage.begin(), storage.end(), THRESHOLD_PER_ELEMENT, [time](component* next)
				{
					next->animate(time);
				}));
			}

			if (!loading.defaults)
				return;

			material* base = loading.defaults;
			if (base->get_ref_count() <= 1)
				return;

			trigonometry::vector3 diffuse = base->surface.diffuse / loading.progress;
			loading.progress = sin(time->get_elapsed()) * 0.2f + 0.8f;
			base->surface.diffuse = diffuse * loading.progress;
		}
		void scene_graph::step_gameplay(core::timer* time)
		{
			VI_ASSERT(time != nullptr, "timer should be set");
			VI_MEASURE(core::timings::pass);

			auto& storage = actors[(size_t)actor_type::update];
			if (active && !storage.empty())
			{
				std::for_each(storage.begin(), storage.end(), [time](component* next)
				{
					next->update(time);
				});
			}
		}
		void scene_graph::step_transactions()
		{
			VI_MEASURE(core::timings::pass);
			if (transactions.empty())
				return;

			VI_TRACE("[scene] process %" PRIu64 " transactions on 0x%" PRIXPTR, (uint64_t)transactions.size(), (void*)this);
			await(task_type::rendering);

			while (!transactions.empty())
			{
				transactions.front()();
				transactions.pop();
			}
		}
		void scene_graph::step_events()
		{
			VI_MEASURE(core::timings::pass);
			if (!events.empty())
				VI_TRACE("[scene] resolve %" PRIu64 " events on 0x%" PRIXPTR, (uint64_t)events.size(), (void*)this);

			while (!events.empty())
			{
				auto& source = events.front();
				resolve_event(source);
				events.pop();
			}
		}
		void scene_graph::step_indexing()
		{
			VI_MEASURE(core::timings::frame);
			if (!camera.load() || dirty.empty())
				return;

			auto begin = dirty.begin();
			auto end = dirty.end();
			dirty.clear();

			watch(task_type::processing, parallel::for_each(begin, end, THRESHOLD_PER_ELEMENT, [this](entity* next)
			{
				next->transform->synchronize();
				next->update_bounds();

				if (next->type.components.empty())
					return;

				core::umutex<std::mutex> unique(exclusive);
				for (auto& item : *next)
				{
					if (item.second->is_cullable())
						changes[item.second->get_id()].insert(item.second);
				}
			}));
		}
		void scene_graph::step_finalize()
		{
			VI_MEASURE(core::timings::frame);

			await(task_type::processing);
			if (!camera.load())
				return;

			for (auto& item : changes)
			{
				if (item.second.empty())
					continue;

				auto& storage = get_storage(item.first);
				storage.index.reserve(item.second.size());

				size_t count = 0;
				for (auto it = item.second.begin(); it != item.second.end(); ++it)
				{
					update_cosmos(storage, *it);
					if (++count > conf.max_updates)
					{
						item.second.erase(item.second.begin(), it);
						break;
					}
				}

				if (count == item.second.size())
					item.second.clear();
			}
		}
		void scene_graph::set_camera(entity* new_camera)
		{
			VI_TRACE("[scene] apply camera 0x%" PRIXPTR " on 0x%" PRIXPTR, (void*)new_camera, (void*)this);
			if (!new_camera)
			{
				camera = nullptr;
				return;
			}

			components::camera* target = new_camera->get_component<components::camera>();
			if (!target || !target->active)
			{
				camera = nullptr;
				return;
			}

			camera = target;
			transaction([target]()
			{
				target->activate(target);
			});
		}
		void scene_graph::remove_entity(entity* entity)
		{
			VI_ASSERT(entity != nullptr, "entity should be set");
			unregister_entity(entity);
		}
		void scene_graph::delete_entity(entity* entity)
		{
			VI_ASSERT(entity != nullptr, "entity should be set");
			if (!unregister_entity(entity))
				return;

			entity->remove_childs();
			transaction([entity]() { entity->release(); });
		}
		void scene_graph::delete_material(material* value)
		{
			VI_ASSERT(value != nullptr, "entity should be set");
			VI_TRACE("[scene] delete material %s on 0x%" PRIXPTR, (void*)value->name.c_str(), (void*)this);
			mutate(value, "pop");

			auto begin = materials.begin(), end = materials.end();
			for (auto it = begin; it != end; ++it)
			{
				if (*it == value)
				{
					materials.remove_at(it);
					break;
				}
			}

			transaction([value]() { value->release(); });
		}
		void scene_graph::register_entity(entity* target)
		{
			VI_ASSERT(target != nullptr, "entity should be set");
			VI_ASSERT(target->scene == this, "entity be created within this scene");
			VI_TRACE("[scene] register entity 0x%" PRIXPTR " on 0x%" PRIXPTR, (void*)target, (void*)this);

			target->active = true;
			for (auto& base : target->type.components)
				register_component(base.second, target->active);

			watch_movement(target);
			mutate(target, "push");
		}
		bool scene_graph::unregister_entity(entity* target)
		{
			VI_ASSERT(target != nullptr, "entity should be set");
			VI_ASSERT(target->get_scene() == this, "entity should be attached to current scene");
			VI_TRACE("[scene] unregister entity 0x%" PRIXPTR " on 0x%" PRIXPTR, (void*)target, (void*)this);

			component* viewer = camera.load();
			if (viewer != nullptr && target == viewer->parent)
				camera = nullptr;

			for (auto& base : target->type.components)
				unregister_component(base.second);

			target->active = false;
			entities.remove(target);
			unwatch_movement(target);
			mutate(target, "pop");

			return true;
		}
		void scene_graph::register_component(component* base, bool verify)
		{
			VI_TRACE("[scene] register component 0x%" PRIXPTR " on 0x%" PRIXPTR "%s", (void*)base, (void*)this, verify ? " (with checks)" : "");
			auto& storage = get_components(base->get_id());
			if (!base->active)
			{
				transaction([base]()
				{
					base->activate(nullptr);
				});
			}

			if (verify)
			{
				storage.add_if_not_exists(base);
				if (base->set & (size_t)actor_set::update)
					get_actors(actor_type::update).add_if_not_exists(base);
				if (base->set & (size_t)actor_set::synchronize)
					get_actors(actor_type::synchronize).add_if_not_exists(base);
				if (base->set & (size_t)actor_set::animate)
					get_actors(actor_type::animate).add_if_not_exists(base);
				if (base->set & (size_t)actor_set::message)
					get_actors(actor_type::message).add_if_not_exists(base);
			}
			else
			{
				storage.add(base);
				if (base->set & (size_t)actor_set::update)
					get_actors(actor_type::update).add(base);
				if (base->set & (size_t)actor_set::synchronize)
					get_actors(actor_type::synchronize).add(base);
				if (base->set & (size_t)actor_set::animate)
					get_actors(actor_type::animate).add(base);
				if (base->set & (size_t)actor_set::message)
					get_actors(actor_type::message).add(base);
			}
			mutate(base, "push");
		}
		void scene_graph::unregister_component(component* base)
		{
			VI_TRACE("[scene] unregister component 0x%" PRIXPTR " on 0x%" PRIXPTR, (void*)base, (void*)this);
			auto& storage = get_components(base->get_id());
			if (base->active)
				base->deactivate();

			storage.remove(base);
			if (base->set & (size_t)actor_set::update)
				get_actors(actor_type::update).remove(base);
			if (base->set & (size_t)actor_set::synchronize)
				get_actors(actor_type::synchronize).remove(base);
			if (base->set & (size_t)actor_set::animate)
				get_actors(actor_type::animate).remove(base);
			if (base->set & (size_t)actor_set::message)
				get_actors(actor_type::message).remove(base);
			mutate(base, "pop");
		}
		void scene_graph::load_component(component* base)
		{
			VI_ASSERT(base != nullptr, "component should be set");
			VI_ASSERT(base->parent != nullptr && base->parent->scene == this, "component should be tied to this scene");
			VI_TRACE("[scene] await component 0x%" PRIXPTR " on 0x%" PRIXPTR, (void*)base, (void*)this);
			core::umutex<std::mutex> unique(exclusive);
			++incomplete[base];
		}
		void scene_graph::unload_component_all(component* base)
		{
			VI_TRACE("[scene] resolve component 0x%" PRIXPTR " on 0x%" PRIXPTR " fully", (void*)base, (void*)this);
			core::umutex<std::mutex> unique(exclusive);
			auto it = incomplete.find(base);
			if (it != incomplete.end())
				incomplete.erase(it);
		}
		bool scene_graph::unload_component(component* base)
		{
			core::umutex<std::mutex> unique(exclusive);
			auto it = incomplete.find(base);
			if (it == incomplete.end())
				return false;

			if (!--it->second)
			{
				VI_TRACE("[scene] resolve component 0x%" PRIXPTR " on 0x%" PRIXPTR, (void*)base, (void*)this);
				incomplete.erase(it);
			}

			return true;
		}
		void scene_graph::clone_entities(entity* instance, core::vector<entity*>* array)
		{
			VI_ASSERT(instance != nullptr, "entity should be set");
			VI_ASSERT(array != nullptr, "array should be set");
			VI_TRACE("[scene] clone entity 0x%" PRIXPTR " on 0x%" PRIXPTR, (void*)instance, (void*)this);

			entity* clone = clone_entity_instance(instance);
			array->push_back(clone);

			trigonometry::transform* root = clone->transform->get_root();
			if (root != nullptr)
				root->get_childs().push_back(clone->transform);

			if (!instance->transform->get_childs_count())
				return;

			core::vector<trigonometry::transform*>& childs = instance->transform->get_childs();
			clone->transform->get_childs().clear();

			for (auto& child : childs)
			{
				size_t offset = array->size();
				clone_entities((entity*)child->user_data, array);
				for (size_t j = offset; j < array->size(); j++)
				{
					entity* next = (*array)[j];
					if (next->transform->get_root() == instance->transform)
						next->transform->set_root(clone->transform);
				}
			}
		}
		void scene_graph::ray_test(uint64_t section, const trigonometry::ray& origin, const ray_callback& callback)
		{
			VI_ASSERT(callback, "callback should not be empty");
			VI_MEASURE(core::timings::pass);

			auto& array = get_components(section);
			trigonometry::ray base = origin;
			trigonometry::vector3 hit;

			for (auto& next : array)
			{
				if (trigonometry::geometric::cursor_ray_test(base, next->parent->snapshot.box, &hit) && !callback(next, hit))
					break;
			}
		}
		void scene_graph::script_hook(const std::string_view& name)
		{
			transaction([this, name]()
			{
				VI_TRACE("[scene] script hook call: %.*s() on 0x%" PRIXPTR, (int)name.size(), name.data(), (void*)this);
				auto& array = get_components<components::scriptable>();
				for (auto it = array.begin(); it != array.end(); ++it)
				{
					components::scriptable* base = (components::scriptable*)*it;
					base->call_entry(name);
				}
			});
		}
		void scene_graph::set_active(bool enabled)
		{
			active = enabled;
			if (!active)
				return;

			transaction([this]()
			{
				VI_TRACE("[scene] perform activation on 0x%" PRIXPTR, (void*)this);
				auto begin = entities.begin(), end = entities.end();
				for (auto it = begin; it != end; ++it)
				{
					entity* next = *it;
					for (auto& base : next->type.components)
					{
						if (base.second->active)
							base.second->activate(nullptr);
					}
				}
			});
		}
		void scene_graph::set_mrt(target_type type, bool clear)
		{
			auto* device = conf.shared.device;
			VI_ASSERT(device != nullptr, "graphics device should be set");
			graphics::multi_render_target_2d* target = display.mrt[(size_t)type];
			device->set_target(target);

			if (!clear)
				return;

			device->clear(target, 0, 0, 0, 0);
			device->clear(target, 1, 0, 0, 0);
			device->clear(target, 2, 1, 0, 0);
			device->clear(target, 3, 0, 0, 0);
			device->clear_depth(target);
		}
		void scene_graph::set_rt(target_type type, bool clear)
		{
			auto* device = conf.shared.device;
			VI_ASSERT(device != nullptr, "graphics device should be set");
			graphics::render_target_2d* target = display.rt[(size_t)type];
			device->set_target(target);

			if (!clear)
				return;

			device->clear(target, 0, 0, 0, 0);
			device->clear_depth(target);
		}
		void scene_graph::swap_mrt(target_type type, graphics::multi_render_target_2d* init)
		{
			size_t index = (size_t)type;
			if (display.mrt[index] == init)
				return;

			graphics::multi_render_target_2d* cache = display.mrt[index + (size_t)target_type::count];
			if (init != nullptr)
			{
				graphics::multi_render_target_2d* base = display.mrt[index];
				display.mrt[index] = init;

				if (!cache)
					display.mrt[index + (size_t)target_type::count] = base;
			}
			else if (cache != nullptr)
			{
				display.mrt[index] = cache;
				display.mrt[index + (size_t)target_type::count] = nullptr;
			}
		}
		void scene_graph::swap_rt(target_type type, graphics::render_target_2d* init)
		{
			size_t index = (size_t)type;
			graphics::render_target_2d* cache = display.rt[index + (size_t)target_type::count];
			if (init != nullptr)
			{
				graphics::render_target_2d* base = display.rt[index];
				display.rt[index] = init;

				if (!cache)
					display.rt[index + (size_t)target_type::count] = base;
			}
			else if (cache != nullptr)
			{
				display.rt[index] = cache;
				display.rt[index + (size_t)target_type::count] = nullptr;
			}
		}
		void scene_graph::clear_mrt(target_type type, bool color, bool depth)
		{
			auto* device = conf.shared.device;
			VI_ASSERT(device != nullptr, "graphics device should be set");
			graphics::multi_render_target_2d* target = display.mrt[(size_t)type];
			if (color)
			{
				device->clear(target, 0, 0, 0, 0);
				device->clear(target, 1, 0, 0, 0);
				device->clear(target, 2, 1, 0, 0);
				device->clear(target, 3, 0, 0, 0);
			}

			if (depth)
				device->clear_depth(target);
		}
		void scene_graph::clear_rt(target_type type, bool color, bool depth)
		{
			auto* device = conf.shared.device;
			VI_ASSERT(device != nullptr, "graphics device should be set");
			graphics::render_target_2d* target = display.rt[(size_t)type];
			if (color)
				device->clear(target, 0, 0, 0, 0);

			if (depth)
				device->clear_depth(target);
		}
		message_callback* scene_graph::set_listener(const std::string_view& event_name, message_callback&& callback)
		{
			VI_ASSERT(callback != nullptr, "callback should be set");
			VI_TRACE("[scene] attach listener %.*s on 0x%" PRIXPTR, (int)event_name.size(), event_name.data(), (void*)this);
			message_callback* id = core::memory::init<message_callback>(std::move(callback));
			core::umutex<std::mutex> unique(exclusive);
			listeners[core::string(event_name)].insert(id);

			return id;
		}
		bool scene_graph::clear_listener(const std::string_view& event_name, message_callback* id)
		{
			VI_ASSERT(!event_name.empty(), "event name should not be empty");
			VI_TRACE("[scene] detach listener %.*s on 0x%" PRIXPTR, (int)event_name.size(), event_name.data(), (void*)this);

			core::umutex<std::mutex> unique(exclusive);
			auto& source = listeners[core::string(event_name)];
			if (id != nullptr)
			{
				auto it = source.find(id);
				if (it == source.end())
					return false;

				source.erase(it);
				core::memory::deinit(id);
				return true;
			}

			bool updates = !source.empty();
			for (auto& listener : source)
				core::memory::deinit(listener);
			source.clear();
			return updates;
		}
		bool scene_graph::push_event(const std::string_view& event_name, core::variant_args&& args, bool propagate)
		{
			VI_TRACE("[scene] push %.*s event %s on 0x%" PRIXPTR, propagate ? "scene" : "listener", (int)event_name.size(), event_name.data(), (void*)this);
			event next(event_name, std::move(args));
			next.args["__vb"] = core::var::integer((int64_t)(propagate ? event_target::scene : event_target::listener));
			next.args["__vt"] = core::var::pointer((void*)this);

			core::umutex<std::mutex> unique(exclusive);
			events.push(std::move(next));
			return true;
		}
		bool scene_graph::push_event(const std::string_view& event_name, core::variant_args&& args, component* target)
		{
			VI_TRACE("[scene] push component event %.*s on 0x%" PRIXPTR " for 0x%" PRIXPTR, (int)event_name.size(), event_name.data(), (void*)this, (void*)target);
			VI_ASSERT(target != nullptr, "target should be set");
			event next(event_name, std::move(args));
			next.args["__vb"] = core::var::integer((int64_t)event_target::component);
			next.args["__vt"] = core::var::pointer((void*)target);

			core::umutex<std::mutex> unique(exclusive);
			events.push(std::move(next));
			return true;
		}
		bool scene_graph::push_event(const std::string_view& event_name, core::variant_args&& args, entity* target)
		{
			VI_TRACE("[scene] push entity event %.*s on 0x%" PRIXPTR " for 0x%" PRIXPTR, (int)event_name.size(), event_name.data(), (void*)this, (void*)target);
			VI_ASSERT(target != nullptr, "target should be set");
			event next(event_name, std::move(args));
			next.args["__vb"] = core::var::integer((int64_t)event_target::entity);
			next.args["__vt"] = core::var::pointer((void*)target);

			core::umutex<std::mutex> unique(exclusive);
			events.push(std::move(next));
			return true;
		}
		void scene_graph::load_resource(uint64_t id, component* context, const std::string_view& path, const core::variant_args& keys, std::function<void(expects_content<void*>&&)>&& callback)
		{
			VI_ASSERT(conf.shared.content != nullptr, "content manager should be set");
			VI_ASSERT(context != nullptr, "component calling this function should be set");
			VI_ASSERT(callback != nullptr, "callback should be set");

			load_component(context);
			conf.shared.content->load_deferred(conf.shared.content->get_processor(id), path, keys).when([this, context, callback = std::move(callback)](expects_content<void*>&& result)
			{
				if (!unload_component(context))
					return;

				transaction([context, callback = std::move(callback), result = std::move(result)]() mutable
				{
					result.report("scene resource loading error");
					callback(std::move(result));
					context->parent->transform->make_dirty();
				});
			});
		}
		core::string scene_graph::find_resource_id(uint64_t id, void* resource)
		{
			asset_cache* cache = conf.shared.content->find_cache(conf.shared.content->get_processor(id), resource);
			return cache != nullptr ? as_resource_path(cache->path) : core::string();
		}
		bool scene_graph::is_left_handed() const
		{
			return conf.shared.device->is_left_handed();
		}
		bool scene_graph::is_indexed() const
		{
			for (auto& item : changes)
			{
				if (!item.second.empty())
					return false;
			}

			return true;
		}
		bool scene_graph::is_busy(task_type type)
		{
			if (type != task_type::rendering)
				return !tasking.queue[(size_t)type].empty();

			core::umutex<std::mutex> unique(tasking.update[(size_t)type]);
			return !tasking.queue[(size_t)type].empty();
		}
		void scene_graph::mutate(entity* parent, entity* child, const std::string_view& type)
		{
			VI_ASSERT(parent != nullptr, "parent should be set");
			VI_ASSERT(child != nullptr, "child should be set");
			if (!conf.mutations)
				return;

			core::variant_args args;
			args["parent"] = core::var::pointer((void*)parent);
			args["child"] = core::var::pointer((void*)child);
			args["type"] = core::var::string(type);
			push_event("mutation", std::move(args), false);
		}
		void scene_graph::mutate(entity* target, const std::string_view& type)
		{
			VI_ASSERT(target != nullptr, "target should be set");
			if (!conf.mutations)
				return;

			core::variant_args args;
			args["entity"] = core::var::pointer((void*)target);
			args["type"] = core::var::string(type);
			push_event("mutation", std::move(args), false);
		}
		void scene_graph::mutate(component* target, const std::string_view& type)
		{
			VI_ASSERT(target != nullptr, "target should be set");
			notify_cosmos(target);

			if (!conf.mutations)
				return;

			core::variant_args args;
			args["component"] = core::var::pointer((void*)target);
			args["type"] = core::var::string(type);
			push_event("mutation", std::move(args), false);
		}
		void scene_graph::mutate(material* target, const std::string_view& type)
		{
			VI_ASSERT(target != nullptr, "target should be set");
			if (!conf.mutations)
				return;

			core::variant_args args;
			args["material"] = core::var::pointer((void*)target);
			args["type"] = core::var::string(type);
			push_event("mutation", std::move(args), false);
		}
		void scene_graph::make_snapshot(idx_snapshot* result)
		{
			VI_ASSERT(result != nullptr, "shapshot result should be set");
			VI_TRACE("[scene] generate snapshot on 0x%" PRIXPTR, (void*)this);
			result->to.clear();
			result->from.clear();

			size_t index = 0;
			auto begin = entities.begin(), end = entities.end();
			for (auto it = begin; it != end; ++it)
			{
				result->to[*it] = index;
				result->from[index] = *it;
				index++;
			}
		}
		void scene_graph::transaction(core::task_callback&& callback)
		{
			VI_ASSERT(callback != nullptr, "callback should be set");
			bool execute_now = false;
			{
				core::umutex<std::mutex> unique(exclusive);
				if (!events.empty() || is_busy(task_type::processing) || is_busy(task_type::rendering))
					transactions.emplace(std::move(callback));
				else
					execute_now = true;
			}
			if (execute_now)
				callback();
		}
		void scene_graph::watch(task_type type, core::promise<void>&& awaitable)
		{
			if (!awaitable.is_pending())
				return;

			if (type == task_type::rendering)
			{
				core::umutex<std::mutex> unique(tasking.update[(size_t)type]);
				tasking.queue[(size_t)type].emplace(std::move(awaitable));
			}
			else
				tasking.queue[(size_t)type].emplace(std::move(awaitable));
		}
		void scene_graph::watch(task_type type, core::vector<core::promise<void>>&& awaitables)
		{
			if (type == task_type::rendering)
			{
				core::umutex<std::mutex> unique(tasking.update[(size_t)type]);
				for (auto& awaitable : awaitables)
				{
					if (awaitable.is_pending())
						tasking.queue[(size_t)type].emplace(std::move(awaitable));
				}
			}
			else
			{
				for (auto& awaitable : awaitables)
				{
					if (awaitable.is_pending())
						tasking.queue[(size_t)type].emplace(std::move(awaitable));
				}
			}
		}
		void scene_graph::await(task_type type)
		{
			VI_MEASURE(core::timings::frame);
			if (type == task_type::rendering)
			{
				core::umutex<std::mutex> unique(tasking.update[(size_t)type]);
				auto& queue = tasking.queue[(size_t)type];
				while (!queue.empty())
				{
					queue.front().wait();
					queue.pop();
				}
			}
			else
			{
				auto& queue = tasking.queue[(size_t)type];
				while (!queue.empty())
				{
					queue.front().wait();
					queue.pop();
				}
			}
		}
		void scene_graph::clear_culling()
		{
			for (auto& sparse : registry)
			{
				auto begin = sparse.second->data.begin();
				auto end = sparse.second->data.end();
				for (auto* it = begin; it != end; ++it)
				{
					component* target = *it;
					if (target->is_drawable())
					{
						drawable* base = (drawable*)target;
						base->overlapping = 1.0f;
					}
				}
			}
		}
		void scene_graph::reserve_materials(size_t size)
		{
			transaction([this, size]()
			{
				upgrade_buffer_by_size(materials, size);
				generate_material_buffer();
			});
		}
		void scene_graph::reserve_entities(size_t size)
		{
			transaction([this, size]()
			{
				upgrade_buffer_by_size(dirty, size);
				upgrade_buffer_by_size(entities, size);
			});
		}
		void scene_graph::reserve_components(uint64_t section, size_t size)
		{
			transaction([this, section, size]()
			{
				auto& storage = get_storage(section);
				upgrade_buffer_by_size(storage.data, size);
			});
		}
		void scene_graph::generate_material_buffer()
		{
			VI_TRACE("[scene] generate material buffer %" PRIu64 "m on 0x%" PRIXPTR, (uint64_t)materials.capacity(), (void*)this);
			graphics::element_buffer::desc f = graphics::element_buffer::desc();
			f.access_flags = graphics::cpu_access::write;
			f.misc_flags = graphics::resource_misc::buffer_structured;
			f.usage = graphics::resource_usage::dynamic;
			f.bind_flags = graphics::resource_bind::shader_input;
			f.element_count = (uint32_t)materials.capacity();
			f.element_width = sizeof(subsurface);
			f.structure_byte_stride = f.element_width;

			core::memory::release(display.material_buffer);
			display.material_buffer = *conf.shared.device->create_element_buffer(f);
		}
		void scene_graph::generate_depth_buffers()
		{
			auto* device = conf.shared.device;
			VI_ASSERT(device != nullptr, "graphics device should be set");
			VI_TRACE("[scene] generate depth buffers %" PRIu64 "t on 0x%" PRIXPTR, (uint64_t)(conf.points_max + conf.spots_max + conf.lines_max), (void*)this);

			for (auto& item : display.points)
				core::memory::release(item);

			for (auto& item : display.spots)
				core::memory::release(item);

			for (auto& item : display.lines)
			{
				if (item != nullptr)
				{
					for (auto* target : *item)
						core::memory::release(target);
					core::memory::deinit(item);
					item = nullptr;
				}
			}

			display.points.resize(conf.points_max);
			for (auto& item : display.points)
			{
				graphics::depth_target_cube::desc f = graphics::depth_target_cube::desc();
				f.size = (uint32_t)conf.points_size;
				f.format_mode = graphics::format::d32_float;
				item = *device->create_depth_target_cube(f);
			}

			display.spots.resize(conf.spots_max);
			for (auto& item : display.spots)
			{
				graphics::depth_target_2d::desc f = graphics::depth_target_2d::desc();
				f.width = f.height = (uint32_t)conf.spots_size;
				f.format_mode = graphics::format::d32_float;
				item = *device->create_depth_target_2d(f);
			}

			display.lines.resize(conf.lines_max);
			for (auto& item : display.lines)
				item = nullptr;

			if (entities.empty())
				return;

			core::variant_args args;
			push_event("depth-flush", std::move(args), true);
		}
		void scene_graph::generate_depth_cascades(depth_cascade_map** result, uint32_t size) const
		{
			VI_TRACE("[scene] generate depth cascades %is on 0x%" PRIXPTR, (int)(size * size), (void*)this);
			depth_cascade_map* target = (*result ? *result : core::memory::init<depth_cascade_map>());
			for (auto& item : *target)
				core::memory::release(item);

			target->resize(size);
			for (auto& item : *target)
			{
				graphics::depth_target_2d::desc f = graphics::depth_target_2d::desc();
				f.width = f.height = (uint32_t)conf.lines_size;
				f.format_mode = graphics::format::d32_float;
				item = *conf.shared.device->create_depth_target_2d(f);
			}

			*result = target;
		}
		void scene_graph::notify_cosmos(component* base)
		{
			if (!base->is_cullable())
				return;

			core::umutex<std::mutex> unique(exclusive);
			changes[base->get_id()].insert(base);
		}
		void scene_graph::clear_cosmos(component* base)
		{
			if (!base->is_cullable())
				return;

			uint64_t id = base->get_id();
			core::umutex<std::mutex> unique(exclusive);
			changes[id].erase(base);

			if (base->indexed)
			{
				auto& storage = get_storage(id);
				storage.index.remove_item((void*)base);
			}
		}
		void scene_graph::update_cosmos(sparse_index& storage, component* base)
		{
			if (base->active)
			{
				auto& bounds = base->parent->snapshot;
				if (!base->indexed)
				{
					storage.index.insert_item((void*)base, bounds.min, bounds.max);
					base->indexed = true;
				}
				else
					storage.index.update_item((void*)base, bounds.min, bounds.max);
			}
			else
			{
				storage.index.remove_item((void*)base);
				base->indexed = false;
			}
		}
		void scene_graph::watch_movement(entity* base)
		{
			base->transform->when_dirty([this, base]()
			{
				transaction([this, base]()
				{
					if (dirty.size() + conf.grow_margin > dirty.capacity())
						upgrade_buffer_by_rate(dirty, (float)conf.grow_rate);
					dirty.add(base);
				});
			});
		}
		void scene_graph::unwatch_movement(entity* base)
		{
			base->transform->when_dirty(nullptr);
			dirty.remove(base);
		}
		bool scene_graph::resolve_event(event& source)
		{
			auto _Bubble = source.args.find("__vb"), _Target = source.args.find("__vt");
			if (_Bubble == source.args.end() || _Target == source.args.end())
				return false;

			event_target bubble = (event_target)_Bubble->second.get_integer();
			void* target = _Target->second.get_pointer();

			if (bubble == event_target::scene)
			{
				auto begin = actors[(size_t)actor_type::message].begin();
				auto end = actors[(size_t)actor_type::message].end();
				for (auto it = begin; it != end; ++it)
					(*it)->message(source.name, source.args);
			}
			else if (bubble == event_target::entity)
			{
				entity* base = (entity*)target;
				for (auto& item : base->type.components)
					item.second->message(source.name, source.args);
			}
			else if (bubble == event_target::component)
			{
				component* base = (component*)target;
				base->message(source.name, source.args);
			}

			auto it = listeners.find(source.name);
			if (it == listeners.end() || it->second.empty())
				return false;

			auto copy = it->second;
			for (auto* callback : copy)
				(*callback)(source.name, source.args);

			return true;
		}
		bool scene_graph::add_material(material* base)
		{
			VI_ASSERT(base != nullptr, "base should be set");
			if (materials.size() + conf.grow_margin > materials.capacity())
			{
				transaction([this, base]()
				{
					if (materials.size() + conf.grow_margin > materials.capacity())
					{
						upgrade_buffer_by_rate(materials, (float)conf.grow_rate);
						generate_material_buffer();
					}
					add_material(base);
				});
			}
			else
			{
				VI_TRACE("[scene] add material %s on 0x%" PRIXPTR, base->name.c_str(), (void*)this);
				base->scene = this;
				materials.add_if_not_exists(base);
				mutate(base, "push");
			}

			return true;
		}
		material* scene_graph::get_invalid_material()
		{
			if (!loading.defaults.load())
				loading.defaults = add_material();

			return loading.defaults;
		}
		material* scene_graph::add_material()
		{
			material* result = new material(this);
			add_material(result);
			return result;
		}
		material* scene_graph::clone_material(material* base)
		{
			VI_ASSERT(base != nullptr, "material should be set");
			material* result = new material(*base);
			add_material(result);
			return result;
		}
		component* scene_graph::get_camera()
		{
			component* base = camera.load();
			if (base != nullptr)
				return base;

			base = get_component(components::camera::get_type_id(), 0);
			if (!base || !base->active)
			{
				entity* next = new entity(this);
				base = next->add_component<components::camera>();
				add_entity(next);
				set_camera(next);
			}
			else
			{
				transaction([this, base]()
				{
					base->activate(base);
					camera = base;
				});
			}

			return base;
		}
		component* scene_graph::get_component(uint64_t section, size_t component)
		{
			auto& array = get_components(section);
			if (component >= array.size())
				return nullptr;

			return *array.at(component);
		}
		render_system* scene_graph::get_renderer()
		{
			auto* viewer = (components::camera*)camera.load();
			if (!viewer)
				return nullptr;

			return viewer->get_renderer();
		}
		viewer scene_graph::get_camera_viewer() const
		{
			auto* result = (components::camera*)camera.load();
			if (!result)
				return viewer();

			return result->get_viewer();
		}
		sparse_index& scene_graph::get_storage(uint64_t section)
		{
			sparse_index* storage = registry[section];
			VI_ASSERT(storage != nullptr, "component should be registered by composer");
			if (storage->data.size() + conf.grow_margin > storage->data.capacity())
			{
				transaction([this, section]()
				{
					sparse_index* storage = registry[section];
					if (storage->data.size() + conf.grow_margin > storage->data.capacity())
						upgrade_buffer_by_rate(storage->data, (float)conf.grow_rate);
				});
			}

			return *storage;
		}
		material* scene_graph::get_material(const std::string_view& name)
		{
			auto begin = materials.begin(), end = materials.end();
			for (auto it = begin; it != end; ++it)
			{
				if ((*it)->name == name)
					return *it;
			}

			return nullptr;
		}
		material* scene_graph::get_material(size_t material)
		{
			VI_ASSERT(material < materials.size(), "index outside of range");
			return materials[material];
		}
		entity* scene_graph::get_entity(size_t entity)
		{
			VI_ASSERT(entity < entities.size(), "index outside of range");
			return entities[entity];
		}
		entity* scene_graph::get_last_entity()
		{
			if (entities.empty())
				return nullptr;

			return entities.back();
		}
		entity* scene_graph::get_camera_entity()
		{
			component* data = get_camera();
			return data ? data->parent : nullptr;
		}
		entity* scene_graph::clone_entity_instance(entity* entity)
		{
			VI_ASSERT(entity != nullptr, "entity should be set");
			VI_MEASURE(core::timings::pass);

			layer::entity* instance = new layer::entity(this);
			instance->transform->copy(entity->transform);
			instance->transform->user_data = instance;
			instance->type.name = entity->type.name;
			instance->type.components = entity->type.components;

			for (auto& it : instance->type.components)
			{
				component* source = it.second;
				it.second = source->copy(instance);
				it.second->parent = instance;
				it.second->active = source->active;
			}

			add_entity(instance);
			return instance;
		}
		entity* scene_graph::clone_entity(entity* value)
		{
			auto array = clone_entity_as_array(value);
			return array.empty() ? nullptr : array.front();
		}
		core::pool<component*>& scene_graph::get_components(uint64_t section)
		{
			sparse_index& storage = get_storage(section);
			return storage.data;
		}
		core::pool<component*>& scene_graph::get_actors(actor_type type)
		{
			auto& storage = actors[(size_t)type];
			if (storage.size() + conf.grow_margin > storage.capacity())
			{
				transaction([this, type]()
				{
					auto& storage = actors[(size_t)type];
					if (storage.size() + conf.grow_margin > storage.capacity())
						upgrade_buffer_by_rate(storage, (float)conf.grow_rate);
				});
			}

			return storage;
		}
		graphics::render_target_2d::desc scene_graph::get_desc_rt() const
		{
			auto* device = conf.shared.device;
			VI_ASSERT(device != nullptr, "graphics device should be set");
			graphics::render_target_2d* target = device->get_render_target();

			VI_ASSERT(target != nullptr, "render target should be set");
			graphics::render_target_2d::desc desc;
			desc.misc_flags = graphics::resource_misc::generate_mips;
			desc.width = (uint32_t)(target->get_width() * conf.render_quality);
			desc.height = (uint32_t)(target->get_height() * conf.render_quality);
			desc.mip_levels = device->get_mip_level(desc.width, desc.height);
			desc.format_mode = get_format_mrt(0);

			return desc;
		}
		graphics::multi_render_target_2d::desc scene_graph::get_desc_mrt() const
		{
			auto* device = conf.shared.device;
			VI_ASSERT(device != nullptr, "graphics device should be set");
			graphics::render_target_2d* target = device->get_render_target();

			VI_ASSERT(target != nullptr, "render target should be set");
			graphics::multi_render_target_2d::desc desc;
			desc.misc_flags = graphics::resource_misc::generate_mips;
			desc.width = (uint32_t)(target->get_width() * conf.render_quality);
			desc.height = (uint32_t)(target->get_height() * conf.render_quality);
			desc.mip_levels = device->get_mip_level(desc.width, desc.height);
			desc.target = graphics::surface_target::t3;
			desc.format_mode[0] = get_format_mrt(0);
			desc.format_mode[1] = get_format_mrt(1);
			desc.format_mode[2] = get_format_mrt(2);
			desc.format_mode[3] = get_format_mrt(3);

			return desc;
		}
		graphics::format scene_graph::get_format_mrt(uint32_t target) const
		{
			if (target == 0)
				return conf.enable_hdr ? graphics::format::r16g16b16a16_unorm : graphics::format::r8g8b8a8_unorm;

			if (target == 1)
				return graphics::format::r16g16b16a16_float;

			if (target == 2)
				return graphics::format::r32_float;

			if (target == 3)
				return graphics::format::r8g8b8a8_unorm;

			return graphics::format::unknown;
		}
		core::vector<entity*> scene_graph::clone_entity_as_array(entity* value)
		{
			core::vector<entity*> array;
			VI_ASSERT(value != nullptr, "entity should be set");
			clone_entities(value, &array);
			return array;
		}
		core::vector<entity*> scene_graph::query_by_parent(entity* entity) const
		{
			core::vector<layer::entity*> array;
			VI_ASSERT(entity != nullptr, "entity should be set");

			auto begin = entities.begin(), end = entities.end();
			for (auto it = begin; it != end; ++it)
			{
				if (*it != entity && !(*it)->transform->has_root(entity->transform))
					array.push_back(*it);
			}

			return array;
		}
		core::vector<entity*> scene_graph::query_by_name(const std::string_view& name) const
		{
			core::vector<entity*> array;
			auto begin = entities.begin(), end = entities.end();
			for (auto it = begin; it != end; ++it)
			{
				if ((*it)->type.name == name)
					array.push_back(*it);
			}

			return array;
		}
		core::vector<component*> scene_graph::query_by_position(uint64_t section, const trigonometry::vector3& position, float radius)
		{
			return query_by_area(section, position - radius * 0.5f, position + radius * 0.5f);
		}
		core::vector<component*> scene_graph::query_by_area(uint64_t section, const trigonometry::vector3& min, const trigonometry::vector3& max)
		{
			core::vector<component*> result;
			trigonometry::bounding target(min, max);
			trigonometry::cosmos::iterator context;
			auto& storage = get_storage(section);
			storage.index.query_index<component>(context, [&target](const trigonometry::bounding& bounds)
			{
				return target.overlaps(bounds);
			}, [&result](component* item)
			{
				result.push_back(item);
			});

			return result;
		}
		core::vector<component*> scene_graph::query_by_match(uint64_t section, std::function<bool(const trigonometry::bounding&)>&& match_callback)
		{
			core::vector<component*> result;
			trigonometry::cosmos::iterator context;
			auto& storage = get_storage(section);
			auto enqueue = [&result](component* item) { result.push_back(item); };
			storage.index.query_index<component>(context, std::move(match_callback), std::move(enqueue));

			return result;
		}
		core::vector<std::pair<component*, trigonometry::vector3>> scene_graph::query_by_ray(uint64_t section, const trigonometry::ray& origin)
		{
			typedef std::pair<component*, trigonometry::vector3> ray_hit;
			core::vector<ray_hit> result;
			trigonometry::ray target = origin;
			trigonometry::cosmos::iterator context;
			auto& storage = get_storage(section);
			storage.index.query_index<component>(context, [&target](const trigonometry::bounding& bounds)
			{
				return target.intersects_aabb_at(bounds.lower, bounds.upper, nullptr);
			}, [&result](component* item)
			{
				result.emplace_back(item, trigonometry::vector3::zero());
			});

			for (auto it = result.begin(); it != result.end();)
			{
				if (!trigonometry::geometric::cursor_ray_test(target, it->first->parent->snapshot.box, &it->second))
					it = result.erase(it);
				else
					++it;
			}

			VI_SORT(result.begin(), result.end(), [&target](ray_hit& a, ray_hit& b)
			{
				float D1 = target.origin.distance(a.first->parent->transform->get_position());
				float D2 = target.origin.distance(b.first->parent->transform->get_position());
				return D1 < D2;
			});

			return result;
		}
		core::vector<depth_cube_map*>& scene_graph::get_points_mapping()
		{
			return display.points;
		}
		core::vector<depth_map*>& scene_graph::get_spots_mapping()
		{
			return display.spots;
		}
		core::vector<depth_cascade_map*>& scene_graph::get_lines_mapping()
		{
			return display.lines;
		}
		const core::unordered_map<uint64_t, sparse_index*>& scene_graph::get_registry() const
		{
			return registry;
		}
		core::string scene_graph::as_resource_path(const std::string_view& path)
		{
			VI_ASSERT(conf.shared.content != nullptr, "content manager should be set");
			core::string rel_path = core::string(path);
			core::stringify::replace(rel_path, conf.shared.content->get_environment(), "./");
			core::stringify::replace(rel_path, '\\', '/');
			return rel_path;
		}
		entity* scene_graph::add_entity()
		{
			entity* next = new entity(this);
			add_entity(next);
			return next;
		}
		bool scene_graph::add_entity(entity* entity)
		{
			VI_ASSERT(entity != nullptr, "entity should be set");
			VI_ASSERT(entity->scene == this, "entity should be created for this scene");

			if (entities.size() + conf.grow_margin <= entities.capacity())
			{
				entities.add(entity);
				register_entity(entity);
				return true;
			}

			transaction([this, entity]()
			{
				if (entities.size() + conf.grow_margin > entities.capacity())
					upgrade_buffer_by_rate(entities, (float)conf.grow_rate);
				add_entity(entity);
			});

			return true;
		}
		bool scene_graph::has_entity(entity* entity) const
		{
			VI_ASSERT(entity != nullptr, "entity should be set");
			for (size_t i = 0; i < entities.size(); i++)
			{
				if (entities[i] == entity)
					return true;
			}

			return false;
		}
		bool scene_graph::has_entity(size_t entity) const
		{
			return entity < entities.size() ? entity : -1;
		}
		bool scene_graph::is_active() const
		{
			return active;
		}
		size_t scene_graph::get_entities_count() const
		{
			return entities.size();
		}
		size_t scene_graph::get_components_count(uint64_t section)
		{
			return get_components(section).size();
		}
		size_t scene_graph::get_materials_count() const
		{
			return materials.size();
		}
		graphics::multi_render_target_2d* scene_graph::get_mrt(target_type type) const
		{
			return display.mrt[(size_t)type];
		}
		graphics::render_target_2d* scene_graph::get_rt(target_type type) const
		{
			return display.rt[(size_t)type];
		}
		graphics::texture_2d** scene_graph::get_merger()
		{
			return &display.merger;
		}
		graphics::element_buffer* scene_graph::get_material_buffer() const
		{
			return display.material_buffer;
		}
		graphics::graphics_device* scene_graph::get_device() const
		{
			return conf.shared.device;
		}
		graphics::activity* scene_graph::get_activity() const
		{
			return conf.shared.activity;
		}
		render_constants* scene_graph::get_constants() const
		{
			return conf.shared.constants;
		}
		shader_cache* scene_graph::get_shaders() const
		{
			return conf.shared.shaders;
		}
		primitive_cache* scene_graph::get_primitives() const
		{
			return conf.shared.primitives;
		}
		physics::simulator* scene_graph::get_simulator() const
		{
			return simulator;
		}
		scene_graph::desc& scene_graph::get_conf()
		{
			return conf;
		}

		void heavy_content_manager::set_device(graphics::graphics_device* new_device)
		{
			device = new_device;
		}
		graphics::graphics_device* heavy_content_manager::get_device() const
		{
			return device;
		}

		heavy_application::heavy_application(desc* i) noexcept : control(i ? *i : desc())
		{
			VI_ASSERT(i != nullptr, "desc should be set");
			control.usage |= control.advanced_usage;
			state = application_state::staging;
		}
		heavy_application::~heavy_application() noexcept
		{
			if (renderer != nullptr)
				renderer->flush_state();

			core::memory::release(internal_ui);
			if (layer::gui::subsystem::has_instance())
				layer::gui::subsystem::get()->cleanup_shared();

			core::memory::release(scene);
			core::memory::release(vm);
			core::memory::release(audio);
			core::memory::release(cache.shaders);
			core::memory::release(cache.primitives);
			core::memory::release(content);
			core::memory::release(constants);
			core::memory::release(renderer);
			core::memory::release(activity);
			core::memory::release(internal_clock);

			heavy_application::unlink_instance(this);
			vitex::heavy_runtime::cleanup_instances();
		}
		core::promise<void> heavy_application::startup()
		{
			return core::promise<void>::null();
		}
		core::promise<void> heavy_application::shutdown()
		{
			return core::promise<void>::null();
		}
		void heavy_application::script_hook()
		{
		}
		void heavy_application::key_event(graphics::key_code key, graphics::key_mod mod, int computed, int repeat, bool pressed)
		{
		}
		void heavy_application::input_event(char* buffer, size_t length)
		{
		}
		void heavy_application::wheel_event(int x, int y, bool normal)
		{
		}
		void heavy_application::window_event(graphics::window_state new_state, int x, int y)
		{
		}
		void heavy_application::composition()
		{
		}
		void heavy_application::dispatch(core::timer* time)
		{
		}
		void heavy_application::publish(core::timer* time)
		{
		}
		void heavy_application::initialize()
		{
		}
		void heavy_application::loop_trigger()
		{
			VI_MEASURE(core::timings::infinite);
			core::schedule::desc& policy = control.scheduler;
			core::schedule* queue = core::schedule::get();
			if (policy.parallel)
			{
				if (activity != nullptr)
				{
					while (state == application_state::active)
					{
						bool render_frame = activity->dispatch(0, control.blocking_dispatch);
						internal_clock->begin();
						dispatch(internal_clock);

						internal_clock->finish();
						if (render_frame)
							publish(internal_clock);
					}
				}
				else
				{
					while (state == application_state::active)
					{
						internal_clock->begin();
						dispatch(internal_clock);

						internal_clock->finish();
						publish(internal_clock);
					}
				}

				while (content && content->is_busy())
					std::this_thread::sleep_for(std::chrono::milliseconds(CONTENT_BLOCKED_WAIT_MS));
			}
			else
			{
				if (activity != nullptr)
				{
					while (state == application_state::active)
					{
						bool render_frame = activity->dispatch(0, control.blocking_dispatch);
						queue->dispatch();

						internal_clock->begin();
						dispatch(internal_clock);

						internal_clock->finish();
						if (render_frame)
							publish(internal_clock);
					}
				}
				else
				{
					while (state == application_state::active)
					{
						queue->dispatch();

						internal_clock->begin();
						dispatch(internal_clock);

						internal_clock->finish();
						publish(internal_clock);
					}
				}
			}
		}
		int heavy_application::start()
		{
			composition();
			compose();

			if (control.usage & USE_PROCESSING)
			{
				if (!content)
					content = new heavy_content_manager();

				content->add_processor<processors::asset_processor, layer::asset_file>();
				content->add_processor<processors::material_processor, layer::material>();
				content->add_processor<processors::scene_graph_processor, layer::scene_graph>();
				content->add_processor<processors::audio_clip_processor, audio::audio_clip>();
				content->add_processor<processors::texture_2d_processor, graphics::texture_2d>();
				content->add_processor<processors::shader_processor, graphics::shader>();
				content->add_processor<processors::model_processor, model>();
				content->add_processor<processors::skin_model_processor, skin_model>();
				content->add_processor<processors::skin_animation_processor, layer::skin_animation>();
				content->add_processor<processors::schema_processor, core::schema>();
				content->add_processor<processors::server_processor, network::http::server>();
				content->add_processor<processors::hull_shape_processor, physics::hull_shape>();

				if (control.environment.empty())
				{
					auto directory = core::os::directory::get_working();
					if (directory)
						content->set_environment(*directory + control.directory);
				}
				else
					content->set_environment(control.environment + control.directory);

				if (!control.preferences.empty() && !database)
				{
					auto path = core::os::path::resolve(control.preferences, content->get_environment(), true);
					database = new app_data(content, path ? *path : control.preferences);
				}
			}

			if (control.usage & USE_ACTIVITY)
			{
				if (!control.activity.width || !control.activity.height)
				{
					graphics::display_info info;
					VI_PANIC(graphics::video::get_display_info(0, &info), "video driver is not initialized");
					control.activity.width = (uint32_t)(info.physical_width / 1.1);
					control.activity.height = (uint32_t)(info.physical_height / 1.2);
				}

				VI_PANIC(control.activity.width > 0 && control.activity.height > 0, "activity width/height is not acceptable");
				bool maximized = control.activity.maximized;
				control.activity.gpu_as_renderer = (control.usage & USE_GRAPHICS);
				control.activity.maximized = false;

				if (!activity)
					activity = new graphics::activity(control.activity);

				if (control.activity.gpu_as_renderer)
				{
					VI_PANIC(!renderer, "graphics device is pre-initialized which is not compatible with GPU as renderer mode");
					control.graphics_device.buffer_width = control.activity.width;
					control.graphics_device.buffer_height = control.activity.height;
					control.graphics_device.window = activity;

					if (content != nullptr && !control.graphics_device.cache_directory.empty())
					{
						auto directory = core::os::path::resolve_directory(control.graphics_device.cache_directory, content->get_environment(), false);
						if (directory)
						{
							control.graphics_device.cache_directory = *directory;
							core::os::directory::patch(control.graphics_device.cache_directory);
						}
					}

					renderer = graphics::graphics_device::create(control.graphics_device);
					VI_PANIC(renderer && renderer->is_valid(), "video driver is not initialized");
					trigonometry::geometric::set_left_handed(renderer->is_left_handed());
					if (content != nullptr)
						content->set_device(renderer);

					if (!cache.shaders)
						cache.shaders = new shader_cache(renderer);

					if (!cache.primitives)
						cache.primitives = new primitive_cache(renderer);

					if (!constants)
						constants = new render_constants(renderer);
				}

				VI_PANIC(activity->get_handle() != nullptr, "activity instance is non-existant");
				activity->user_pointer = this;
				activity->set_cursor_visibility(control.cursor);
				activity->callbacks.key_state = [this](graphics::key_code key, graphics::key_mod mod, int computed, int repeat, bool pressed)
				{
					gui::context* UI = try_get_ui();
					if (UI != nullptr)
						UI->emit_key(key, mod, computed, repeat, pressed);
					key_event(key, mod, computed, repeat, pressed);
				};
				activity->callbacks.input = [this](char* buffer, int length)
				{
					if (!buffer)
						return;

					gui::context* UI = try_get_ui();
					if (UI != nullptr)
						UI->emit_input(buffer, length);
					input_event(buffer, length < 0 ? strlen(buffer) : (size_t)length);
				};
				activity->callbacks.cursor_wheel_state = [this](int x, int y, bool normal)
				{
					gui::context* UI = try_get_ui();
					if (UI != nullptr)
						UI->emit_wheel(x, y, normal, activity->get_key_mod_state());
					wheel_event(x, y, normal);
				};
				activity->callbacks.window_state_change = [this](graphics::window_state new_state, int x, int y)
				{
					if (new_state == graphics::window_state::resize)
					{
						gui::context* UI = try_get_ui();
						if (UI != nullptr)
							UI->emit_resize(x, y);
					}
					window_event(new_state, x, y);
				};
				control.activity.maximized = maximized;
			}

			if (control.usage & USE_AUDIO && !audio)
				audio = new audio::audio_device();

			if (control.usage & USE_SCRIPTING && !vm)
				vm = new scripting::virtual_machine();

			internal_clock = new core::timer();
			internal_clock->set_fixed_frames(control.refreshrate.stable);
			internal_clock->set_max_frames(control.refreshrate.limit);

			if (control.usage & USE_NETWORKING)
			{
				if (network::multiplexer::has_instance())
					network::multiplexer::get()->rescale(control.polling_timeout, control.polling_events);
				else
					new network::multiplexer(control.polling_timeout, control.polling_events);
			}

			if (control.usage & USE_SCRIPTING)
				script_hook();

			initialize();
			if (state == application_state::terminated)
				return exit_code != 0 ? exit_code : EXIT_JUMP + 6;

			state = application_state::active;
			if (activity != nullptr)
			{
				activity->show();
				if (control.activity.maximized)
					activity->maximize();
			}

			core::schedule::desc& policy = control.scheduler;
			policy.initialize = [this](core::task_callback&& callback) { this->startup().when(std::move(callback)); };
			policy.ping = control.daemon ? std::bind(&heavy_application::status, this) : (core::activity_callback)nullptr;

			if (control.threads > 0)
			{
				core::schedule::desc launch = core::schedule::desc(control.threads);
				memcpy(policy.threads, launch.threads, sizeof(policy.threads));
			}

			auto* queue = core::schedule::get();
			queue->start(policy);
			internal_clock->reset();
			loop_trigger();
			shutdown().wait();
			queue->stop();

			exit_code = (state == application_state::restart ? EXIT_RESTART : exit_code);
			state = application_state::terminated;
			return exit_code;
		}
		void heavy_application::stop(int code)
		{
			core::schedule* queue = core::schedule::get();
			state = application_state::terminated;
			exit_code = code;
			queue->wakeup();
		}
		void heavy_application::restart()
		{
			core::schedule* queue = core::schedule::get();
			state = application_state::restart;
			queue->wakeup();
		}
		gui::context* heavy_application::try_get_ui() const
		{
			return internal_ui;
		}
		gui::context* heavy_application::fetch_ui()
		{
			if (!internal_ui && activity != nullptr && renderer != nullptr && constants != nullptr && content != nullptr)
			{
				gui::subsystem::get()->set_shared(vm, activity, constants, content, internal_clock);
				internal_ui = new gui::context(renderer);
			}
			return internal_ui;
		}
		application_state heavy_application::get_state() const
		{
			return state;
		}
		bool heavy_application::status(heavy_application* app)
		{
			return app->state == application_state::active;
		}
		void heavy_application::compose()
		{
			static bool was_composed = false;
			if (was_composed)
				return;
			else
				was_composed = true;

			uint64_t as_component = (uint64_t)composer_tag::component;
			core::composer::push<components::rigid_body, entity*>(as_component);
			core::composer::push<components::soft_body, entity*>(as_component);
			core::composer::push<components::acceleration, entity*>(as_component);
			core::composer::push<components::slider_constraint, entity*>(as_component);
			core::composer::push<components::model, entity*>(as_component);
			core::composer::push<components::skin, entity*>(as_component);
			core::composer::push<components::emitter, entity*>(as_component);
			core::composer::push<components::decal, entity*>(as_component);
			core::composer::push<components::skin_animator, entity*>(as_component);
			core::composer::push<components::key_animator, entity*>(as_component);
			core::composer::push<components::emitter_animator, entity*>(as_component);
			core::composer::push<components::free_look, entity*>(as_component);
			core::composer::push<components::fly, entity*>(as_component);
			core::composer::push<components::audio_source, entity*>(as_component);
			core::composer::push<components::audio_listener, entity*>(as_component);
			core::composer::push<components::point_light, entity*>(as_component);
			core::composer::push<components::spot_light, entity*>(as_component);
			core::composer::push<components::line_light, entity*>(as_component);
			core::composer::push<components::surface_light, entity*>(as_component);
			core::composer::push<components::illuminator, entity*>(as_component);
			core::composer::push<components::camera, entity*>(as_component);
			core::composer::push<components::scriptable, entity*>(as_component);

			uint64_t as_renderer = (uint64_t)composer_tag::renderer;
			core::composer::push<renderers::soft_body, render_system*>(as_renderer);
			core::composer::push<renderers::model, render_system*>(as_renderer);
			core::composer::push<renderers::skin, render_system*>(as_renderer);
			core::composer::push<renderers::emitter, render_system*>(as_renderer);
			core::composer::push<renderers::decal, render_system*>(as_renderer);
			core::composer::push<renderers::lighting, render_system*>(as_renderer);
			core::composer::push<renderers::transparency, render_system*>(as_renderer);
			core::composer::push<renderers::glitch, render_system*>(as_renderer);
			core::composer::push<renderers::tone, render_system*>(as_renderer);
			core::composer::push<renderers::depth_of_field, render_system*>(as_renderer);
			core::composer::push<renderers::bloom, render_system*>(as_renderer);
			core::composer::push<renderers::local_reflections, render_system*>(as_renderer);
			core::composer::push<renderers::local_illumination, render_system*>(as_renderer);
			core::composer::push<renderers::local_ambient, render_system*>(as_renderer);
			core::composer::push<renderers::motion_blur, render_system*>(as_renderer);
			core::composer::push<renderers::user_interface, render_system*>(as_renderer);

			uint64_t as_effect = (uint64_t)composer_tag::effect;
			core::composer::push<audio::effects::reverb>(as_effect);
			core::composer::push<audio::effects::chorus>(as_effect);
			core::composer::push<audio::effects::distortion>(as_effect);
			core::composer::push<audio::effects::echo>(as_effect);
			core::composer::push<audio::effects::flanger>(as_effect);
			core::composer::push<audio::effects::frequency_shifter>(as_effect);
			core::composer::push<audio::effects::vocal_morpher>(as_effect);
			core::composer::push<audio::effects::pitch_shifter>(as_effect);
			core::composer::push<audio::effects::ring_modulator>(as_effect);
			core::composer::push<audio::effects::autowah>(as_effect);
			core::composer::push<audio::effects::compressor>(as_effect);
			core::composer::push<audio::effects::equalizer>(as_effect);

			uint64_t as_filter = (uint64_t)composer_tag::filter;
			core::composer::push<audio::filters::lowpass>(as_filter);
			core::composer::push<audio::filters::bandpass>(as_filter);
			core::composer::push<audio::filters::highpass>(as_filter);
		}

		effect_renderer::effect_renderer(render_system* lab) noexcept : renderer(lab), output(nullptr), swap(nullptr)
		{
			VI_ASSERT(lab != nullptr, "render system should be set");
			VI_ASSERT(lab->get_device() != nullptr, "graphics device should be set");

			auto* device = lab->get_device();
			depth_stencil = device->get_depth_stencil_state("doo_soo_lt");
			rasterizer = device->get_rasterizer_state("so_cback");
			blend = device->get_blend_state("bo_wrgbo_one");
			sampler_wrap = device->get_sampler_state("a16_fa_wrap");
			sampler_clamp = device->get_sampler_state("a16_fa_clamp");
			sampler_mirror = device->get_sampler_state("a16_fa_mirror");
			layout = device->get_input_layout("vx_shape");
			slots.diffuse_map = *device->get_shader_slot(lab->get_basic_effect(), "DiffuseMap");
			slots.sampler = *device->get_shader_sampler_slot(lab->get_basic_effect(), "DiffuseMap", "Sampler");
			slots.object = *device->get_shader_slot(lab->get_basic_effect(), "ObjectBuffer");
		}
		effect_renderer::~effect_renderer() noexcept
		{
			for (auto it = effects.begin(); it != effects.end(); ++it)
				system->free_shader(it->second.filename, it->second.effect);
		}
		void effect_renderer::resize_buffers()
		{
			output = nullptr;
			resize_effect();
		}
		void effect_renderer::resize_effect()
		{
		}
		void effect_renderer::render_copy_from_main(uint32_t slot, graphics::texture_2d* dest)
		{
			VI_ASSERT(dest != nullptr, "texture should be set");
			graphics::graphics_device* device = system->get_device();
			device->copy_texture_2d(system->get_scene()->get_mrt(target_type::main), slot, &dest);
		}
		void effect_renderer::render_copy_to_main(uint32_t slot, graphics::texture_2d* src)
		{
			VI_ASSERT(src != nullptr, "texture should be set");
			graphics::graphics_device* device = system->get_device();
			auto* dest = system->get_scene()->get_mrt(target_type::main)->get_target_2d(slot);
			device->copy_texture_2d(src, &dest);
		}
		void effect_renderer::render_copy_from_last(graphics::texture_2d* dest)
		{
			VI_ASSERT(dest != nullptr, "texture should be set");
			graphics::graphics_device* device = system->get_device();
			device->copy_texture_2d(output, 0, &dest);
		}
		void effect_renderer::render_copy_to_last(graphics::texture_2d* src)
		{
			VI_ASSERT(src != nullptr, "texture should be set");
			graphics::texture_2d** merger = system->get_merger();
			graphics::graphics_device* device = system->get_device();
			graphics::texture_2d* dest = swap != nullptr && output != swap ? swap->get_target() : (merger ? *merger : nullptr);
			if (dest != nullptr)
				device->copy_texture_2d(src, &dest);
		}
		void effect_renderer::render_output(graphics::render_target_2d* resource)
		{
			VI_ASSERT(system->get_device() != nullptr, "graphics device should be set");
			if (resource != nullptr)
			{
				output = resource;
				swap = resource;
			}
			else
				output = system->get_rt(target_type::main);

			graphics::graphics_device* device = system->get_device();
			device->set_target(output, 0, 0, 0, 0);
		}
		void effect_renderer::render_texture(graphics::shader* effect, const std::string_view& name, graphics::texture_2d* resource)
		{
			auto source = effect ? effects.find(effect) : effects.begin();
			VI_ASSERT(source != effects.end(), "effect not registered");
			graphics::graphics_device* device = system->get_device();
			auto target_slot = source->second.offsets.find(core::key_lookup_cast(name));
			if (target_slot == source->second.offsets.end())
			{
				source->second.offsets[core::string(name)] = *device->get_shader_slot(source->first, name);
				target_slot = source->second.offsets.find(core::key_lookup_cast(name));
			}
			device->set_texture_2d(resource, target_slot->second, VI_PS);
			if (resource != nullptr)
				source->second.regs.insert(target_slot->second);
		}
		void effect_renderer::render_texture(graphics::shader* effect, const std::string_view& name, graphics::texture_3d* resource)
		{
			auto source = effect ? effects.find(effect) : effects.begin();
			VI_ASSERT(source != effects.end(), "effect not registered");
			graphics::graphics_device* device = system->get_device();
			auto target_slot = source->second.offsets.find(core::key_lookup_cast(name));
			if (target_slot == source->second.offsets.end())
			{
				source->second.offsets[core::string(name)] = *device->get_shader_slot(source->first, name);
				target_slot = source->second.offsets.find(core::key_lookup_cast(name));
			}
			device->set_texture_3d(resource, target_slot->second, VI_PS);
			if (resource != nullptr)
				source->second.regs.insert(target_slot->second);
		}
		void effect_renderer::render_texture(graphics::shader* effect, const std::string_view& name, graphics::texture_cube* resource)
		{
			auto source = effect ? effects.find(effect) : effects.begin();
			VI_ASSERT(source != effects.end(), "effect not registered");
			graphics::graphics_device* device = system->get_device();
			auto target_slot = source->second.offsets.find(core::key_lookup_cast(name));
			if (target_slot == source->second.offsets.end())
			{
				source->second.offsets[core::string(name)] = *device->get_shader_slot(source->first, name);
				target_slot = source->second.offsets.find(core::key_lookup_cast(name));
			}
			device->set_texture_cube(resource, target_slot->second, VI_PS);
			if (resource != nullptr)
				source->second.regs.insert(target_slot->second);
		}
		void effect_renderer::render_merge(graphics::shader* effect, graphics::sampler_state* sampler, void* buffer, size_t count)
		{
			auto source = effect ? effects.find(effect) : effects.begin();
			VI_ASSERT(source != effects.end(), "effect not registered");
			VI_ASSERT(count > 0, "count should be greater than zero");

			graphics::graphics_device* device = system->get_device();
			graphics::multi_render_target_2d* input = system->get_mrt(target_type::main);
			if (source->second.slots.sampler != (uint32_t)-1)
				device->set_sampler_state(sampler, source->second.slots.sampler, texture_count_of(source->second), VI_PS);
			if (source->second.slots.diffuse_buffer != (uint32_t)-1)
				device->set_texture_2d(input->get_target(0), source->second.slots.diffuse_buffer, VI_PS);
			if (source->second.slots.normal_buffer != (uint32_t)-1)
				device->set_texture_2d(input->get_target(1), source->second.slots.normal_buffer, VI_PS);
			if (source->second.slots.depth_buffer != (uint32_t)-1)
				device->set_texture_2d(input->get_target(2), source->second.slots.depth_buffer, VI_PS);
			if (source->second.slots.surface_buffer != (uint32_t)-1)
				device->set_texture_2d(input->get_target(3), source->second.slots.surface_buffer, VI_PS);

			graphics::texture_2d** merger = system->get_merger();
			if (source->second.slots.image_buffer != (uint32_t)-1)
			{
				if (swap != nullptr && output != swap)
				{
					device->set_texture_2d(nullptr, source->second.slots.image_buffer, VI_PS);
					device->set_texture_2d(swap->get_target(), source->second.slots.image_buffer, VI_PS);
				}
				else if (merger != nullptr)
				{
					device->set_texture_2d(nullptr, source->second.slots.image_buffer, VI_PS);
					device->set_texture_2d(*merger, source->second.slots.image_buffer, VI_PS);
				}
			}

			device->set_shader(source->first, VI_VS | VI_PS);
			if (buffer != nullptr && source->second.slots.render_buffer != (uint32_t)-1)
			{
				device->update_buffer(source->first, buffer);
				device->set_buffer(source->first, source->second.slots.render_buffer, VI_VS | VI_PS);
			}

			for (size_t i = 0; i < count; i++)
			{
				device->draw(6, 0);
				if (!swap)
					device->copy_texture_2d(output, 0, merger);
			}

			auto* scene = system->get_scene();
			scene->statistics.draw_calls += count;
			if (swap == output)
				render_output();
		}
		void effect_renderer::render_result(graphics::shader* effect, graphics::sampler_state* sampler, void* buffer)
		{
			auto source = effect ? effects.find(effect) : effects.begin();
			VI_ASSERT(source != effects.end(), "effect not registered");

			graphics::graphics_device* device = system->get_device();
			graphics::multi_render_target_2d* input = system->get_mrt(target_type::main);
			if (source->second.slots.sampler != (uint32_t)-1)
				device->set_sampler_state(sampler, source->second.slots.sampler, texture_count_of(source->second), VI_PS);
			if (source->second.slots.diffuse_buffer != (uint32_t)-1)
				device->set_texture_2d(input->get_target(0), source->second.slots.diffuse_buffer, VI_PS);
			if (source->second.slots.normal_buffer != (uint32_t)-1)
				device->set_texture_2d(input->get_target(1), source->second.slots.normal_buffer, VI_PS);
			if (source->second.slots.depth_buffer != (uint32_t)-1)
				device->set_texture_2d(input->get_target(2), source->second.slots.depth_buffer, VI_PS);
			if (source->second.slots.surface_buffer != (uint32_t)-1)
				device->set_texture_2d(input->get_target(3), source->second.slots.surface_buffer, VI_PS);

			graphics::texture_2d** merger = system->get_merger();
			if (source->second.slots.image_buffer != (uint32_t)-1)
			{
				if (swap != nullptr && output != swap)
				{
					device->set_texture_2d(nullptr, source->second.slots.image_buffer, VI_PS);
					device->set_texture_2d(swap->get_target(), source->second.slots.image_buffer, VI_PS);
				}
				else if (merger != nullptr)
				{
					device->set_texture_2d(nullptr, source->second.slots.image_buffer, VI_PS);
					device->set_texture_2d(*merger, source->second.slots.image_buffer, VI_PS);
				}
			}

			device->set_shader(source->first, VI_VS | VI_PS);
			if (buffer != nullptr && source->second.slots.render_buffer != (uint32_t)-1)
			{
				device->update_buffer(source->first, buffer);
				device->set_buffer(source->first, source->second.slots.render_buffer, VI_VS | VI_PS);
			}

			device->draw(6, 0);
			output = system->get_rt(target_type::main);

			auto* scene = system->get_scene();
			scene->statistics.draw_calls++;
		}
		void effect_renderer::render_result(graphics::sampler_state* sampler)
		{
			graphics::graphics_device* device = system->get_device();
			graphics::texture_2d** merger = system->get_merger();
			if (swap != nullptr && output != swap)
				device->set_texture_2d(swap->get_target(), slots.diffuse_map, VI_PS);
			else if (merger != nullptr)
				device->set_texture_2d(*merger, slots.diffuse_map, VI_PS);

			device->set_sampler_state(sampler, slots.sampler, 1, VI_PS);
			device->set_shader(system->get_basic_effect(), VI_VS | VI_PS);
			device->draw(6, 0);
			output = system->get_rt(target_type::main);

			auto* scene = system->get_scene();
			scene->statistics.draw_calls++;
		}
		void effect_renderer::render_effect(core::timer* time)
		{
		}
		void effect_renderer::generate_mips()
		{
			graphics::graphics_device* device = system->get_device();
			graphics::texture_2d** merger = system->get_merger();
			if (swap != nullptr && output != swap)
				device->generate_mips(swap->get_target());
			else if (merger != nullptr)
				device->generate_mips(*merger);
		}
		uint32_t effect_renderer::texture_count_of(shader_data& data)
		{
			if (data.slots.diffuse_buffer != (uint32_t)-1)
				data.regs.insert(data.slots.diffuse_buffer);
			if (data.slots.normal_buffer != (uint32_t)-1)
				data.regs.insert(data.slots.normal_buffer);
			if (data.slots.depth_buffer != (uint32_t)-1)
				data.regs.insert(data.slots.depth_buffer);
			if (data.slots.surface_buffer != (uint32_t)-1)
				data.regs.insert(data.slots.surface_buffer);
			if (data.slots.image_buffer != (uint32_t)-1)
				data.regs.insert(data.slots.image_buffer);
			return data.regs.size();
		}
		size_t effect_renderer::render_pass(core::timer* time)
		{
			VI_ASSERT(system->get_primitives() != nullptr, "primitive cache should be set");
			VI_ASSERT(system->get_mrt(target_type::main) != nullptr, "main render target should be set");
			VI_MEASURE(core::timings::pass);

			if (!system->state.is(render_state::geometry) || system->state.is_subpass() || effects.empty())
				return 0;

			swap = nullptr;
			if (!output)
				output = system->get_rt(target_type::main);

			graphics::multi_render_target_2d* input = system->get_mrt(target_type::main);
			primitive_cache* cache = system->get_primitives();
			graphics::graphics_device* device = system->get_device();
			device->set_depth_stencil_state(depth_stencil);
			device->set_blend_state(blend);
			device->set_rasterizer_state(rasterizer);
			device->set_input_layout(layout);
			device->set_target(output, 0, 0, 0, 0);
			device->set_vertex_buffer(cache->get_quad());

			render_effect(time);

			uint32_t max_slot = 0;
			for (auto& effect : effects)
			{
				for (auto& slot : effect.second.regs)
					max_slot = std::max(max_slot, slot);
			}

			device->flush_texture(0, max_slot + 1, VI_PS);
			device->copy_target(output, 0, input, 0);
			system->restore_output();
			return 1;
		}
		effect_renderer::shader_data* effect_renderer::get_effect_by_filename(const std::string_view& name)
		{
			for (auto& effect : effects)
			{
				if (effect.second.filename == name)
					return &effect.second;
			}

			return nullptr;
		}
		effect_renderer::shader_data* effect_renderer::get_effect_by_shader(graphics::shader* shader)
		{
			auto it = effects.find(shader);
			if (it != effects.end())
				return &it->second;

			return nullptr;
		}
		graphics::expects_graphics<graphics::shader*> effect_renderer::compile_effect(graphics::shader::desc& desc, size_t buffer_size)
		{
			VI_ASSERT(!desc.filename.empty(), "cannot compile unnamed shader source");
			auto shader = system->compile_shader(desc, buffer_size);
			if (!shader)
				return shader.error();

			shader_data data;
			data.effect = *shader;
			data.filename = desc.filename;
			data.slots.diffuse_buffer = system->get_device()->get_shader_slot(data.effect, "DiffuseBuffer").or_else((uint32_t)-1);
			data.slots.normal_buffer = system->get_device()->get_shader_slot(data.effect, "NormalBuffer").or_else((uint32_t)-1);
			data.slots.depth_buffer = system->get_device()->get_shader_slot(data.effect, "DepthBuffer").or_else((uint32_t)-1);
			data.slots.surface_buffer = system->get_device()->get_shader_slot(data.effect, "SurfaceBuffer").or_else((uint32_t)-1);
			data.slots.image_buffer = system->get_device()->get_shader_slot(data.effect, "ImageBuffer").or_else((uint32_t)-1);
			data.slots.render_buffer = system->get_device()->get_shader_slot(data.effect, "RenderBuffer").or_else((uint32_t)-1);

			if (data.slots.diffuse_buffer != (uint32_t)-1)
				data.slots.sampler = system->get_device()->get_shader_sampler_slot(data.effect, "DiffuseBuffer", "Sampler").or_else((uint32_t)-1);
			else if (data.slots.normal_buffer != (uint32_t)-1)
				data.slots.sampler = system->get_device()->get_shader_sampler_slot(data.effect, "NormalBuffer", "Sampler").or_else((uint32_t)-1);
			else if (data.slots.depth_buffer != (uint32_t)-1)
				data.slots.sampler = system->get_device()->get_shader_sampler_slot(data.effect, "DepthBuffer", "Sampler").or_else((uint32_t)-1);
			else if (data.slots.surface_buffer != (uint32_t)-1)
				data.slots.sampler = system->get_device()->get_shader_sampler_slot(data.effect, "SurfaceBuffer", "Sampler").or_else((uint32_t)-1);
			else if (data.slots.image_buffer != (uint32_t)-1)
				data.slots.sampler = system->get_device()->get_shader_sampler_slot(data.effect, "ImageBuffer", "Sampler").or_else((uint32_t)-1);

			auto* effect = get_effect_by_filename(desc.filename);
			if (effect != nullptr)
			{
				auto* target = effect->effect;
				core::memory::release(target);
				effects.erase(target);
			}

			effects[*shader] = std::move(data);
			return shader;
		}
		graphics::expects_graphics<graphics::shader*> effect_renderer::compile_effect(const std::string_view& section_name, core::vector<core::string>&& features, size_t buffer_size)
		{
			graphics::shader::desc i = graphics::shader::desc();
			auto status = system->get_device()->get_section_data(section_name, &i);
			if (!status)
				return status.error();

			i.defines.reserve(i.defines.size() + features.size());
			for (auto& feature : features)
				i.defines.push_back(std::move(feature));

			return compile_effect(i, buffer_size);
		}
		uint32_t effect_renderer::get_mip_levels() const
		{
			VI_ASSERT(system->get_rt(target_type::main) != nullptr, "main render target should be set");
			graphics::render_target_2d* rt = system->get_rt(target_type::main);
			return system->get_device()->get_mip_level(rt->get_width(), rt->get_height());
		}
		uint32_t effect_renderer::get_width() const
		{
			VI_ASSERT(system->get_rt(target_type::main) != nullptr, "main render target should be set");
			graphics::render_target_2d* rt = system->get_rt(target_type::main);
			return rt->get_width();
		}
		uint32_t effect_renderer::get_height() const
		{
			VI_ASSERT(system->get_rt(target_type::main) != nullptr, "main render target should be set");
			graphics::render_target_2d* rt = system->get_rt(target_type::main);
			return rt->get_height();
		}
	}
}
