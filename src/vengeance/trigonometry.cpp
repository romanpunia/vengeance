#include "trigonometry.h"
#ifdef VI_VECTORCLASS
#include "internal/vectorclass.hpp"
#endif
#define MAKE_ADJ_TRI(x) ((x) & 0x3fffffff)
#define IS_BOUNDARY(x) ((x) == 0xff)
#define RH_TO_LH (matrix4x4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, -1, 0, 0, 0, 0, 1))
#define NULL_NODE ((size_t)-1)

namespace
{
	size_t offset_of64(const char* source, char dest)
	{
		VI_ASSERT(source != nullptr, "source should be set");
		for (size_t i = 0; i < 64; i++)
		{
			if (source[i] == dest)
				return i;
		}

		return 63;
	}
	vitex::core::string escape_text(const vitex::core::string& data)
	{
		vitex::core::string result = "\"";
		result.append(data).append("\"");
		return result;
	}
}

namespace vitex
{
	namespace trigonometry
	{
		int64_t rectangle::get_x() const
		{
			return left;
		}
		int64_t rectangle::get_y() const
		{
			return top;
		}
		int64_t rectangle::get_width() const
		{
			return right - left;
		}
		int64_t rectangle::get_height() const
		{
			return bottom - top;
		}

		vector2::vector2() noexcept : x(0.0f), y(0.0f)
		{
		}
		vector2::vector2(float x, float y) noexcept : x(x), y(y)
		{
		}
		vector2::vector2(float xy) noexcept : x(xy), y(xy)
		{
		}
		vector2::vector2(const vector2& value) noexcept : x(value.x), y(value.y)
		{
		}
		vector2::vector2(const vector3& value) noexcept : x(value.x), y(value.y)
		{
		}
		vector2::vector2(const vector4& value) noexcept : x(value.x), y(value.y)
		{
		}
		bool vector2::is_equals(const vector2& other, float max_displacement) const
		{
			return fabs(x - other.x) <= max_displacement && fabs(y - other.y) <= max_displacement;
		}
		float vector2::length() const
		{
#ifdef VI_VECTORCLASS
			LOAD_FV2(_r1);
			return std::sqrt(horizontal_add(square(_r1)));
#else
			return std::sqrt(x * x + y * y);
#endif
		}
		float vector2::sum() const
		{
#ifdef VI_VECTORCLASS
			LOAD_FV2(_r1);
			return horizontal_add(_r1);
#else
			return x + y;
#endif
		}
		float vector2::dot(const vector2& b) const
		{
#ifdef VI_VECTORCLASS
			LOAD_FV2(_r1); LOAD_V2(_r2, b);
			return horizontal_add(_r1 * _r2);
#else
			return x * b.x + y * b.y;
#endif
		}
		float vector2::distance(const vector2& point) const
		{
#ifdef VI_VECTORCLASS
			LOAD_FV2(_r1); LOAD_V2(_r2, point);
			return geometric::fast_sqrt(horizontal_add(square(_r1 - _r2)));
#else
			float X1 = x - point.x, Y1 = y - point.y;
			return geometric::fast_sqrt(X1 * X1 + Y1 * Y1);
#endif
		}
		float vector2::hypotenuse() const
		{
			return length();
		}
		float vector2::look_at(const vector2& at) const
		{
#ifdef VI_VECTORCLASS
			LOAD_FV2(_r1); LOAD_V2(_r2, at); _r1 = _r2 - _r1;
			return atan2f(_r1.extract(0), _r1.extract(1));
#else
			return atan2f(at.x - x, at.y - y);
#endif
		}
		float vector2::cross(const vector2& b) const
		{
#ifdef VI_VECTORCLASS
			LOAD_FV2(_r1); LOAD_V2(_r2, b); _r1 = _r1 * _r2;
			return _r1.extract(0) - _r1.extract(1);
#else
			return x * b.y - y * b.x;
#endif
		}
		vector2 vector2::transform(const matrix4x4& matrix) const
		{
#ifdef VI_VECTORCLASS
			LOAD_VAL(_r1, x);
			LOAD_VAL(_r2, y);
			LOAD_VAR(_r3, matrix.row);
			LOAD_VAR(_r4, matrix.row + 4);

			_r1 = _r1 * _r3 + _r2 * _r4;
			return vector2(_r1.extract(0), _r1.extract(1));
#else
			return vector2(
				x * matrix.row[0] + y * matrix.row[4],
				x * matrix.row[1] + y * matrix.row[5]);
#endif
		}
		vector2 vector2::direction(float rotation) const
		{
			return vector2(cos(-rotation), sin(-rotation));
		}
		vector2 vector2::inv() const
		{
			return vector2(-x, -y);
		}
		vector2 vector2::inv_x() const
		{
			return vector2(-x, y);
		}
		vector2 vector2::inv_y() const
		{
			return vector2(x, -y);
		}
		vector2 vector2::normalize() const
		{
#ifdef VI_VECTORCLASS
			LOAD_FV2(_r1);
			_r1 = _r1 * geometric::fast_inv_sqrt(horizontal_add(square(_r1)));
			return vector2(_r1.extract(0), _r1.extract(1));
#else
			float f = length();
			return vector2(x / f, y / f);
#endif
		}
		vector2 vector2::snormalize() const
		{
#ifdef VI_VECTORCLASS
			LOAD_FV2(_r1);
			float f = geometric::fast_sqrt(horizontal_add(square(_r1)));
			if (f == 0.0f)
				return vector2();

			_r1 = _r1 / f;
			return vector2(_r1.extract(0), _r1.extract(1));
#else
			float f = length();
			if (f == 0.0f)
				return vector2();

			return vector2(x / f, y / f);
#endif
		}
		vector2 vector2::lerp(const vector2& b, float delta_time) const
		{
#ifdef VI_VECTORCLASS
			LOAD_FV2(_r1); LOAD_V2(_r2, b);
			_r1 = _r1 + (_r2 - _r1) * delta_time;
			return vector2(_r1.extract(0), _r1.extract(1));
#else
			return *this + (b - *this) * delta_time;
#endif
		}
		vector2 vector2::slerp(const vector2& b, float delta_time) const
		{
			return quaternion(vector3()).slerp(b.xyz(), delta_time).get_euler().xy();
		}
		vector2 vector2::alerp(const vector2& b, float delta_time) const
		{
			float ax = geometric::angluar_lerp(x, b.x, delta_time);
			float ay = geometric::angluar_lerp(y, b.y, delta_time);
			return vector2(ax, ay);
		}
		vector2 vector2::rlerp() const
		{
			float ax = compute::mathf::saturate_angle(x);
			float ay = compute::mathf::saturate_angle(y);
			return vector2(ax, ay);
		}
		vector2 vector2::abs() const
		{
#ifdef VI_VECTORCLASS
			LOAD_FV2(_r1); _r1 = ::abs(_r1);
			return vector2(_r1.extract(0), _r1.extract(1));
#else
			return vector2(x < 0 ? -x : x, y < 0 ? -y : y);
#endif
		}
		vector2 vector2::radians() const
		{
			return (*this) * compute::mathf::deg2rad();
		}
		vector2 vector2::degrees() const
		{
			return (*this) * compute::mathf::rad2deg();
		}
		vector2 vector2::xy() const
		{
			return *this;
		}
		vector3 vector2::xyz() const
		{
			return vector3(x, y, 0);
		}
		vector4 vector2::xyzw() const
		{
			return vector4(x, y, 0, 0);
		}
		vector2 vector2::mul(float xy) const
		{
#ifdef VI_VECTORCLASS
			LOAD_FV2(_r1); _r1 = _r1 * xy;
			return vector2(_r1.extract(0), _r1.extract(1));
#else
			return vector2(x * xy, y * xy);
#endif
		}
		vector2 vector2::mul(float x, float y) const
		{
#ifdef VI_VECTORCLASS
			LOAD_FV2(_r1); LOAD_AV2(_r2, x, y); _r1 = _r1 * _r2;
			return vector2(_r1.extract(0), _r1.extract(1));
#else
			return vector2(x * x, y * y);
#endif
		}
		vector2 vector2::mul(const vector2& value) const
		{
			return mul(value.x, value.y);
		}
		vector2 vector2::div(const vector2& value) const
		{
#ifdef VI_VECTORCLASS
			LOAD_FV2(_r1); LOAD_V2(_r2, value); _r1 = _r1 / _r2;
			return vector2(_r1.extract(0), _r1.extract(1));
#else
			return vector2(x / value.x, y / value.y);
#endif
		}
		vector2 vector2::add(const vector2& value) const
		{
#ifdef VI_VECTORCLASS
			LOAD_FV2(_r1); LOAD_V2(_r2, value); _r1 = _r1 + _r2;
			return vector2(_r1.extract(0), _r1.extract(1));
#else
			return vector2(x + value.x, y + value.y);
#endif
		}
		vector2 vector2::set_x(float xf) const
		{
			return vector2(xf, y);
		}
		vector2 vector2::set_y(float yf) const
		{
			return vector2(x, yf);
		}
		void vector2::set(const vector2& value)
		{
			x = value.x;
			y = value.y;
		}
		void vector2::get2(float* in) const
		{
			VI_ASSERT(in != nullptr, "in of size 2 should be set");
			in[0] = x;
			in[1] = y;
		}
		vector2& vector2::operator *=(const vector2& v)
		{
#ifdef VI_VECTORCLASS
			LOAD_FV2(_r1); LOAD_V2(_r2, v);
			_r1 = _r1 * _r2;
			_r1.store_partial(2, (float*)this);
#else
			x *= v.x;
			y *= v.y;
#endif
			return *this;
		}
		vector2& vector2::operator *=(float v)
		{
#ifdef VI_VECTORCLASS
			LOAD_FV2(_r1);
			_r1 = _r1 * v;
			_r1.store_partial(2, (float*)this);
#else
			x *= v;
			y *= v;
#endif
			return *this;
		}
		vector2& vector2::operator /=(const vector2& v)
		{
#ifdef VI_VECTORCLASS
			LOAD_FV2(_r1); LOAD_V2(_r2, v);
			_r1 = _r1 / _r2;
			_r1.store_partial(2, (float*)this);
#else
			x /= v.x;
			y /= v.y;
#endif
			return *this;
		}
		vector2& vector2::operator /=(float v)
		{
#ifdef VI_VECTORCLASS
			LOAD_FV2(_r1);
			_r1 = _r1 / v;
			_r1.store_partial(2, (float*)this);
#else
			x /= v;
			y /= v;
#endif
			return *this;
		}
		vector2& vector2::operator +=(const vector2& v)
		{
#ifdef VI_VECTORCLASS
			LOAD_FV2(_r1); LOAD_V2(_r2, v);
			_r1 = _r1 + _r2;
			_r1.store_partial(2, (float*)this);
#else
			x += v.x;
			y += v.y;
#endif
			return *this;
		}
		vector2& vector2::operator +=(float v)
		{
#ifdef VI_VECTORCLASS
			LOAD_FV2(_r1);
			_r1 = _r1 + v;
			_r1.store_partial(2, (float*)this);
#else
			x += v;
			y += v;
#endif
			return *this;
		}
		vector2& vector2::operator -=(const vector2& v)
		{
#ifdef VI_VECTORCLASS
			LOAD_FV2(_r1); LOAD_V2(_r2, v);
			_r1 = _r1 - _r2;
			_r1.store_partial(2, (float*)this);
#else
			x -= v.x;
			y -= v.y;
#endif
			return *this;
		}
		vector2& vector2::operator -=(float v)
		{
#ifdef VI_VECTORCLASS
			LOAD_FV2(_r1);
			_r1 = _r1 - v;
			_r1.store_partial(2, (float*)this);
#else
			x -= v;
			y -= v;
#endif
			return *this;
		}
		vector2 vector2::operator *(const vector2& v) const
		{
			return mul(v);
		}
		vector2 vector2::operator *(float v) const
		{
			return mul(v);
		}
		vector2 vector2::operator /(const vector2& v) const
		{
			return div(v);
		}
		vector2 vector2::operator /(float v) const
		{
			return div(v);
		}
		vector2 vector2::operator +(const vector2& v) const
		{
			return add(v);
		}
		vector2 vector2::operator +(float v) const
		{
			return add(v);
		}
		vector2 vector2::operator -(const vector2& v) const
		{
			return add(-v);
		}
		vector2 vector2::operator -(float v) const
		{
			return add(-v);
		}
		vector2 vector2::operator -() const
		{
			return inv();
		}
		vector2& vector2::operator =(const vector2& v) noexcept
		{
			x = v.x;
			y = v.y;
			return *this;
		}
		bool vector2::operator ==(const vector2& r) const
		{
			return x == r.x && y == r.y;
		}
		bool vector2::operator !=(const vector2& r) const
		{
			return !(x == r.x && y == r.y);
		}
		bool vector2::operator <=(const vector2& r) const
		{
			return x <= r.x && y <= r.y;
		}
		bool vector2::operator >=(const vector2& r) const
		{
			return x >= r.x && y >= r.y;
		}
		bool vector2::operator <(const vector2& r) const
		{
			return x < r.x && y < r.y;
		}
		bool vector2::operator >(const vector2& r) const
		{
			return x > r.x && y > r.y;
		}
		float& vector2::operator [](uint32_t axis)
		{
			VI_ASSERT(axis >= 0 && axis <= 1, "index out of range");
			if (axis == 0)
				return x;

			return y;
		}
		float vector2::operator [](uint32_t axis) const
		{
			VI_ASSERT(axis >= 0 && axis <= 1, "index out of range");
			if (axis == 0)
				return x;

			return y;
		}
		vector2 vector2::random()
		{
			return vector2(compute::mathf::random_mag(), compute::mathf::random_mag());
		}
		vector2 vector2::random_abs()
		{
			return vector2(compute::mathf::random(), compute::mathf::random());
		}

		vector3::vector3() noexcept : x(0.0f), y(0.0f), z(0.0f)
		{
		}
		vector3::vector3(float x, float y) noexcept : x(x), y(y), z(0.0f)
		{
		}
		vector3::vector3(float x, float y, float z) noexcept : x(x), y(y), z(z)
		{
		}
		vector3::vector3(float xyzw) noexcept : x(xyzw), y(xyzw), z(xyzw)
		{
		}
		vector3::vector3(const vector2& value) noexcept : x(value.x), y(value.y), z(0.0f)
		{
		}
		vector3::vector3(const vector3& value) noexcept : x(value.x), y(value.y), z(value.z)
		{
		}
		vector3::vector3(const vector4& value) noexcept : x(value.x), y(value.y), z(value.z)
		{
		}
		bool vector3::is_equals(const vector3& other, float max_displacement) const
		{
			return fabs(x - other.x) <= max_displacement && fabs(y - other.y) <= max_displacement && fabs(z - other.z) <= max_displacement;
		}
		float vector3::length() const
		{
#ifdef VI_VECTORCLASS
			LOAD_FV3(_r1);
			return sqrt(horizontal_add(square(_r1)));
#else
			return std::sqrt(x * x + y * y + z * z);
#endif
		}
		float vector3::sum() const
		{
#ifdef VI_VECTORCLASS
			LOAD_FV3(_r1);
			return horizontal_add(_r1);
#else
			return x + y + z;
#endif
		}
		float vector3::dot(const vector3& b) const
		{
#ifdef VI_VECTORCLASS
			LOAD_FV3(_r1); LOAD_V3(_r2, b);
			return horizontal_add(_r1 * _r2);
#else
			return x * b.x + y * b.y + z * b.z;
#endif
		}
		float vector3::distance(const vector3& point) const
		{
#ifdef VI_VECTORCLASS
			LOAD_FV3(_r1); LOAD_V3(_r2, point);
			return geometric::fast_sqrt(horizontal_add(square(_r1 - _r2)));
#else
			float X1 = x - point.x, Y1 = y - point.y, Z1 = z - point.z;
			return geometric::fast_sqrt(X1 * X1 + Y1 * Y1 + Z1 * Z1);
#endif
		}
		float vector3::hypotenuse() const
		{
#ifdef VI_VECTORCLASS
			LOAD_AV2(_r1, x, z);
			float r = geometric::fast_sqrt(horizontal_add(square(_r1)));

			LOAD_AV2(_r2, r, y);
			return geometric::fast_sqrt(horizontal_add(square(_r2)));
#else
			float r = geometric::fast_sqrt(x * x + z * z);
			return geometric::fast_sqrt(r * r + y * y);
#endif
		}
		vector3 vector3::look_at(const vector3& b) const
		{
			vector2 H1(x, z), H2(b.x, b.z);
			return vector3(0.0f, -H1.look_at(H2), 0.0f);
		}
		vector3 vector3::cross(const vector3& b) const
		{
#ifdef VI_VECTORCLASS
			LOAD_AV3(_r1, y, z, x);
			LOAD_AV3(_r2, z, x, y);
			LOAD_AV3(_r3, b.z, b.x, b.y);
			LOAD_AV3(_r4, b.y, b.z, b.x);

			_r1 = _r1 * _r3 - _r2 * _r4;
			return vector3(_r1.extract(0), _r1.extract(1), _r1.extract(2));
#else
			return vector3(y * b.z - z * b.y, z * b.x - x * b.z, x * b.y - y * b.x);
#endif
		}
		vector3 vector3::transform(const matrix4x4& matrix) const
		{
#ifdef VI_VECTORCLASS
			LOAD_VAL(_r1, x);
			LOAD_VAL(_r2, y);
			LOAD_VAL(_r3, z);
			LOAD_VAR(_r4, matrix.row);
			LOAD_VAR(_r5, matrix.row + 4);
			LOAD_VAR(_r6, matrix.row + 8);

			_r1 = _r1 * _r4 + _r2 * _r5 + _r3 * _r6;
			return vector3(_r1.extract(0), _r1.extract(1), _r1.extract(2));
#else
			return vector3(
				x * matrix.row[0] + y * matrix.row[4] + z * matrix.row[8],
				x * matrix.row[1] + y * matrix.row[5] + z * matrix.row[9],
				x * matrix.row[2] + y * matrix.row[6] + z * matrix.row[10]);
#endif
		}
		vector3 vector3::ddirection() const
		{
			float cos_x = cos(x);
			return vector3(sin(y) * cos_x, -sin(x), cos(y) * cos_x);
		}
		vector3 vector3::hdirection() const
		{
			return vector3(-cos(y), 0, sin(y));
		}
		vector3 vector3::direction() const
		{
			return matrix4x4::create_look_at(0, *this, vector3::up()).rotation_euler();
		}
		vector3 vector3::inv() const
		{
			return vector3(-x, -y, -z);
		}
		vector3 vector3::inv_x() const
		{
			return vector3(-x, y, z);
		}
		vector3 vector3::inv_y() const
		{
			return vector3(x, -y, z);
		}
		vector3 vector3::inv_z() const
		{
			return vector3(x, y, -z);
		}
		vector3 vector3::normalize() const
		{
#ifdef VI_VECTORCLASS
			LOAD_FV3(_r1);
			_r1 = _r1 * geometric::fast_inv_sqrt(horizontal_add(square(_r1)));
			return vector3(_r1.extract(0), _r1.extract(1), _r1.extract(2));
#else
			float f = length();
			return vector3(x / f, y / f, z / f);
#endif
		}
		vector3 vector3::snormalize() const
		{
#ifdef VI_VECTORCLASS
			LOAD_FV3(_r1);
			float f = geometric::fast_sqrt(horizontal_add(square(_r1)));
			if (f == 0.0f)
				return vector3();

			_r1 = _r1 / f;
			return vector3(_r1.extract(0), _r1.extract(1), _r1.extract(2));
#else
			float f = length();
			if (f == 0.0f)
				return vector3();

			return vector3(x / f, y / f, z / f);
#endif
		}
		vector3 vector3::lerp(const vector3& b, float delta_time) const
		{
#ifdef VI_VECTORCLASS
			LOAD_FV3(_r1); LOAD_V3(_r2, b);
			_r1 = _r1 + (_r2 - _r1) * delta_time;
			return vector3(_r1.extract(0), _r1.extract(1), _r1.extract(2));
#else
			return *this + (b - *this) * delta_time;
#endif
		}
		vector3 vector3::slerp(const vector3& b, float delta_time) const
		{
			return quaternion(*this).slerp(b, delta_time).get_euler();
		}
		vector3 vector3::alerp(const vector3& b, float delta_time) const
		{
			float ax = geometric::angluar_lerp(x, b.x, delta_time);
			float ay = geometric::angluar_lerp(y, b.y, delta_time);
			float az = geometric::angluar_lerp(z, b.z, delta_time);

			return vector3(ax, ay, az);
		}
		vector3 vector3::rlerp() const
		{
			float ax = compute::mathf::saturate_angle(x);
			float ay = compute::mathf::saturate_angle(y);
			float az = compute::mathf::saturate_angle(z);

			return vector3(ax, ay, az);
		}
		vector3 vector3::abs() const
		{
#ifdef VI_VECTORCLASS
			LOAD_FV3(_r1); _r1 = ::abs(_r1);
			return vector3(_r1.extract(0), _r1.extract(1), _r1.extract(2));
#else
			return vector3(x < 0 ? -x : x, y < 0 ? -y : y, z < 0 ? -z : z);
#endif
		}
		vector3 vector3::radians() const
		{
			return (*this) * compute::mathf::deg2rad();
		}
		vector3 vector3::degrees() const
		{
			return (*this) * compute::mathf::rad2deg();
		}
		vector3 vector3::view_space() const
		{
			return inv_z();
		}
		vector2 vector3::xy() const
		{
			return vector2(x, y);
		}
		vector3 vector3::xyz() const
		{
			return *this;
		}
		vector4 vector3::xyzw() const
		{
			return vector4(x, y, z, 0);
		}
		vector3 vector3::mul(float xyz) const
		{
#ifdef VI_VECTORCLASS
			LOAD_FV3(_r1); _r1 = _r1 * xyz;
			return vector3(_r1.extract(0), _r1.extract(1), _r1.extract(2));
#else
			return vector3(x * xyz, y * xyz, z * xyz);
#endif
		}
		vector3 vector3::mul(const vector2& xy, float z) const
		{
#ifdef VI_VECTORCLASS
			LOAD_FV3(_r1); LOAD_AV3(_r2, xy.x, xy.y, z); _r1 = _r1 * _r2;
			return vector3(_r1.extract(0), _r1.extract(1), _r1.extract(2));
#else
			return vector3(x * xy.x, y * xy.y, z * z);
#endif
		}
		vector3 vector3::mul(const vector3& value) const
		{
#ifdef VI_VECTORCLASS
			LOAD_FV3(_r1); LOAD_V3(_r2, value); _r1 = _r1 * _r2;
			return vector3(_r1.extract(0), _r1.extract(1), _r1.extract(2));
#else
			return vector3(x * value.x, y * value.y, z * value.z);
#endif
		}
		vector3 vector3::div(const vector3& value) const
		{
#ifdef VI_VECTORCLASS
			LOAD_FV3(_r1); LOAD_V3(_r2, value); _r1 = _r1 / _r2;
			return vector3(_r1.extract(0), _r1.extract(1), _r1.extract(2));
#else
			return vector3(x / value.x, y / value.y, z / value.z);
#endif
		}
		vector3 vector3::add(const vector3& value) const
		{
#ifdef VI_VECTORCLASS
			LOAD_FV3(_r1); LOAD_V3(_r2, value); _r1 = _r1 + _r2;
			return vector3(_r1.extract(0), _r1.extract(1), _r1.extract(2));
#else
			return vector3(x + value.x, y + value.y, z + value.z);
#endif
		}
		vector3 vector3::set_x(float xf) const
		{
			return vector3(xf, y, z);
		}
		vector3 vector3::set_y(float yf) const
		{
			return vector3(x, yf, z);
		}
		vector3 vector3::set_z(float zf) const
		{
			return vector3(x, y, zf);
		}
		vector3 vector3::rotate(const vector3& origin, const vector3& rotation)
		{
			return transform(matrix4x4::create(origin, rotation));
		}
		void vector3::set(const vector3& value)
		{
			x = value.x;
			y = value.y;
			z = value.z;
		}
		void vector3::get2(float* in) const
		{
			VI_ASSERT(in != nullptr, "in of size 2 should be set");
			in[0] = x;
			in[1] = y;
		}
		void vector3::get3(float* in) const
		{
			VI_ASSERT(in != nullptr, "in of size 3 should be set");
			in[0] = x;
			in[1] = y;
			in[2] = z;
		}
		vector3& vector3::operator *=(const vector3& v)
		{
#ifdef VI_VECTORCLASS
			LOAD_FV3(_r1); LOAD_V3(_r2, v);
			_r1 = _r1 * _r2;
			_r1.store_partial(3, (float*)this);
#else
			x *= v.x;
			y *= v.y;
			z *= v.z;
#endif
			return *this;
		}
		vector3& vector3::operator *=(float v)
		{
#ifdef VI_VECTORCLASS
			LOAD_FV3(_r1); LOAD_VAL(_r2, v);
			_r1 = _r1 * _r2;
			_r1.store_partial(3, (float*)this);
#else
			x *= v;
			y *= v;
			z *= v;
#endif
			return *this;
		}
		vector3& vector3::operator /=(const vector3& v)
		{
#ifdef VI_VECTORCLASS
			LOAD_FV3(_r1); LOAD_V3(_r2, v);
			_r1 = _r1 / _r2;
			_r1.store_partial(3, (float*)this);
#else
			x /= v.x;
			y /= v.y;
			z /= v.z;
#endif
			return *this;
		}
		vector3& vector3::operator /=(float v)
		{
#ifdef VI_VECTORCLASS
			LOAD_FV3(_r1); LOAD_VAL(_r2, v);
			_r1 = _r1 / _r2;
			_r1.store_partial(3, (float*)this);
#else
			x *= v;
			y *= v;
			z *= v;
#endif
			return *this;
		}
		vector3& vector3::operator +=(const vector3& v)
		{
#ifdef VI_VECTORCLASS
			LOAD_FV3(_r1); LOAD_V3(_r2, v);
			_r1 = _r1 + _r2;
			_r1.store_partial(3, (float*)this);
#else
			x += v.x;
			y += v.y;
			z += v.z;
#endif
			return *this;
		}
		vector3& vector3::operator +=(float v)
		{
#ifdef VI_VECTORCLASS
			LOAD_FV3(_r1); LOAD_VAL(_r2, v);
			_r1 = _r1 + _r2;
			_r1.store_partial(3, (float*)this);
#else
			x += v;
			y += v;
			z += v;
#endif
			return *this;
		}
		vector3& vector3::operator -=(const vector3& v)
		{
#ifdef VI_VECTORCLASS
			LOAD_FV3(_r1); LOAD_V3(_r2, v);
			_r1 = _r1 - _r2;
			_r1.store_partial(3, (float*)this);
#else
			x -= v.x;
			y -= v.y;
			z -= v.z;
#endif
			return *this;
		}
		vector3& vector3::operator -=(float v)
		{
#ifdef VI_VECTORCLASS
			LOAD_FV3(_r1); LOAD_VAL(_r2, v);
			_r1 = _r1 - _r2;
			_r1.store_partial(3, (float*)this);
#else
			x -= v;
			y -= v;
			z -= v;
#endif
			return *this;
		}
		vector3 vector3::operator *(const vector3& v) const
		{
			return mul(v);
		}
		vector3 vector3::operator *(float v) const
		{
			return mul(v);
		}
		vector3 vector3::operator /(const vector3& v) const
		{
			return div(v);
		}
		vector3 vector3::operator /(float v) const
		{
			return div(v);
		}
		vector3 vector3::operator +(const vector3& v) const
		{
			return add(v);
		}
		vector3 vector3::operator +(float v) const
		{
			return add(v);
		}
		vector3 vector3::operator -(const vector3& v) const
		{
			return add(-v);
		}
		vector3 vector3::operator -(float v) const
		{
			return add(-v);
		}
		vector3 vector3::operator -() const
		{
			return inv();
		}
		vector3& vector3::operator =(const vector3& v) noexcept
		{
			x = v.x;
			y = v.y;
			z = v.z;
			return *this;
		}
		bool vector3::operator ==(const vector3& r) const
		{
			return x == r.x && y == r.y && z == r.z;
		}
		bool vector3::operator !=(const vector3& r) const
		{
			return !(x == r.x && y == r.y && z == r.z);
		}
		bool vector3::operator <=(const vector3& r) const
		{
			return x <= r.x && y <= r.y && z <= r.z;
		}
		bool vector3::operator >=(const vector3& r) const
		{
			return x >= r.x && y >= r.y && z >= r.z;
		}
		bool vector3::operator <(const vector3& r) const
		{
			return x < r.x && y < r.y && z < r.z;
		}
		bool vector3::operator >(const vector3& r) const
		{
			return x > r.x && y > r.y && z > r.z;
		}
		float& vector3::operator [](uint32_t axis)
		{
			VI_ASSERT(axis >= 0 && axis <= 2, "index out of range");
			if (axis == 0)
				return x;
			else if (axis == 1)
				return y;

			return z;
		}
		float vector3::operator [](uint32_t axis) const
		{
			VI_ASSERT(axis >= 0 && axis <= 2, "index out of range");
			if (axis == 0)
				return x;
			else if (axis == 1)
				return y;

			return z;
		}
		vector3 vector3::random()
		{
			return vector3(compute::mathf::random_mag(), compute::mathf::random_mag(), compute::mathf::random_mag());
		}
		vector3 vector3::random_abs()
		{
			return vector3(compute::mathf::random(), compute::mathf::random(), compute::mathf::random());
		}

		vector4::vector4() noexcept : x(0.0f), y(0.0f), z(0.0f), w(0.0f)
		{
		}
		vector4::vector4(float x, float y) noexcept : x(x), y(y), z(0.0f), w(0.0f)
		{
		}
		vector4::vector4(float x, float y, float z) noexcept : x(x), y(y), z(z), w(0.0f)
		{
		}
		vector4::vector4(float x, float y, float z, float w) noexcept : x(x), y(y), z(z), w(w)
		{
		}
		vector4::vector4(float xyzw) noexcept : x(xyzw), y(xyzw), z(xyzw), w(xyzw)
		{
		}
		vector4::vector4(const vector2& value) noexcept : x(value.x), y(value.y), z(0.0f), w(0.0f)
		{
		}
		vector4::vector4(const vector3& value) noexcept : x(value.x), y(value.y), z(value.z), w(0.0f)
		{
		}
		vector4::vector4(const vector4& value) noexcept : x(value.x), y(value.y), z(value.z), w(value.w)
		{
		}
		bool vector4::is_equals(const vector4& other, float max_displacement) const
		{
			return fabs(x - other.x) <= max_displacement && fabs(y - other.y) <= max_displacement && fabs(z - other.z) <= max_displacement && fabs(w - other.w) <= max_displacement;
		}
		float vector4::length() const
		{
#ifdef VI_VECTORCLASS
			LOAD_FV4(_r1);
			return std::sqrt(horizontal_add(square(_r1)));
#else
			return std::sqrt(x * x + y * y + z * z + w * w);
#endif
		}
		float vector4::sum() const
		{
#ifdef VI_VECTORCLASS
			LOAD_FV4(_r1);
			return horizontal_add(_r1);
#else
			return x + y + z + w;
#endif
		}
		float vector4::dot(const vector4& b) const
		{
#ifdef VI_VECTORCLASS
			LOAD_FV4(_r1); LOAD_V4(_r2, b);
			return horizontal_add(_r1 * _r2);
#else
			return x * b.x + y * b.y + z * b.z + w * b.w;
#endif
		}
		float vector4::distance(const vector4& point) const
		{
#ifdef VI_VECTORCLASS
			LOAD_FV4(_r1); LOAD_V4(_r2, point);
			return geometric::fast_sqrt(horizontal_add(square(_r1 - _r2)));
#else
			float X1 = x - point.x, Y1 = y - point.y, Z1 = z - point.z, W1 = w - point.w;
			return geometric::fast_sqrt(X1 * X1 + Y1 * Y1 + Z1 * Z1 + W1 * W1);
#endif
		}
		vector4 vector4::cross(const vector4& b) const
		{
#ifdef VI_VECTORCLASS
			LOAD_AV3(_r1, y, z, x);
			LOAD_AV3(_r2, z, x, y);
			LOAD_AV3(_r3, b.z, b.x, b.y);
			LOAD_AV3(_r4, b.y, b.z, b.x);

			_r1 = _r1 * _r3 - _r2 * _r4;
			return vector3(_r1.extract(0), _r1.extract(1), _r1.extract(2));
#else
			return vector4(y * b.z - z * b.y, z * b.x - x * b.z, x * b.y - y * b.x);
#endif
		}
		vector4 vector4::transform(const matrix4x4& matrix) const
		{
#ifdef VI_VECTORCLASS
			LOAD_VAL(_r1, x);
			LOAD_VAL(_r2, y);
			LOAD_VAL(_r3, z);
			LOAD_VAL(_r4, w);
			LOAD_VAR(_r5, matrix.row);
			LOAD_VAR(_r6, matrix.row + 4);
			LOAD_VAR(_r7, matrix.row + 8);
			LOAD_VAR(_r8, matrix.row + 12);

			_r1 = _r1 * _r5 + _r2 * _r6 + _r3 * _r7 + _r4 * _r8;
			return vector4(_r1.extract(0), _r1.extract(1), _r1.extract(2), _r1.extract(3));
#else
			return vector4(
				x * matrix.row[0] + y * matrix.row[4] + z * matrix.row[8] + w * matrix.row[12],
				x * matrix.row[1] + y * matrix.row[5] + z * matrix.row[9] + w * matrix.row[13],
				x * matrix.row[2] + y * matrix.row[6] + z * matrix.row[10] + w * matrix.row[14],
				x * matrix.row[3] + y * matrix.row[7] + z * matrix.row[11] + w * matrix.row[15]);
#endif
		}
		vector4 vector4::inv() const
		{
			return vector4(-x, -y, -z, -w);
		}
		vector4 vector4::inv_x() const
		{
			return vector4(-x, y, z, w);
		}
		vector4 vector4::inv_y() const
		{
			return vector4(x, -y, z, w);
		}
		vector4 vector4::inv_z() const
		{
			return vector4(x, y, -z, w);
		}
		vector4 vector4::inv_w() const
		{
			return vector4(x, y, z, -w);
		}
		vector4 vector4::normalize() const
		{
#ifdef VI_VECTORCLASS
			LOAD_FV4(_r1);
			_r1 = _r1 * geometric::fast_inv_sqrt(horizontal_add(square(_r1)));
			return vector4(_r1.extract(0), _r1.extract(1), _r1.extract(2), _r1.extract(3));
#else
			float f = length();
			return vector4(x / f, y / f, z / f, w / f);
#endif
		}
		vector4 vector4::snormalize() const
		{
#ifdef VI_VECTORCLASS
			LOAD_FV4(_r1);
			float f = geometric::fast_sqrt(horizontal_add(square(_r1)));
			if (f == 0.0f)
				return vector4();

			_r1 = _r1 / f;
			return vector4(_r1.extract(0), _r1.extract(1), _r1.extract(2), _r1.extract(3));
#else
			float f = length();
			if (f == 0.0f)
				return vector4();

			return vector4(x / f, y / f, z / f, w / f);
#endif
		}
		vector4 vector4::lerp(const vector4& b, float delta_time) const
		{
#ifdef VI_VECTORCLASS
			LOAD_FV4(_r1); LOAD_V4(_r2, b);
			_r1 = _r1 + (_r2 - _r1) * delta_time;
			return vector3(_r1.extract(0), _r1.extract(1), _r1.extract(2));
#else
			return *this + (b - *this) * delta_time;
#endif
		}
		vector4 vector4::slerp(const vector4& b, float delta_time) const
		{
			vector3 lerp = quaternion(vector3()).slerp(b.xyz(), delta_time).get_euler();
			return vector4(lerp.x, lerp.y, lerp.z, w + (b.w - w) * delta_time);
		}
		vector4 vector4::alerp(const vector4& b, float delta_time) const
		{
			float ax = geometric::angluar_lerp(x, b.x, delta_time);
			float ay = geometric::angluar_lerp(y, b.y, delta_time);
			float az = geometric::angluar_lerp(z, b.z, delta_time);
			float aw = geometric::angluar_lerp(w, b.w, delta_time);

			return vector4(ax, ay, az, aw);
		}
		vector4 vector4::rlerp() const
		{
			float ax = compute::mathf::saturate_angle(x);
			float ay = compute::mathf::saturate_angle(y);
			float az = compute::mathf::saturate_angle(z);
			float aw = compute::mathf::saturate_angle(w);

			return vector4(ax, ay, az, aw);
		}
		vector4 vector4::abs() const
		{
#ifdef VI_VECTORCLASS
			LOAD_FV4(_r1); _r1 = ::abs(_r1);
			return vector4(_r1.extract(0), _r1.extract(1), _r1.extract(2), _r1.extract(3));
#else
			return vector4(x < 0 ? -x : x, y < 0 ? -y : y, z < 0 ? -z : z, w < 0 ? -w : w);
#endif
		}
		vector4 vector4::radians() const
		{
			return (*this) * compute::mathf::deg2rad();
		}
		vector4 vector4::degrees() const
		{
			return (*this) * compute::mathf::rad2deg();
		}
		vector4 vector4::view_space() const
		{
			return inv_z();
		}
		vector2 vector4::xy() const
		{
			return vector2(x, y);
		}
		vector3 vector4::xyz() const
		{
			return vector3(x, y, z);
		}
		vector4 vector4::xyzw() const
		{
			return *this;
		}
		vector4 vector4::set_x(float texcoord_x) const
		{
			return vector4(texcoord_x, y, z, w);
		}
		vector4 vector4::set_y(float texcoord_y) const
		{
			return vector4(x, texcoord_y, z, w);
		}
		vector4 vector4::set_z(float TZ) const
		{
			return vector4(x, y, TZ, w);
		}
		vector4 vector4::set_w(float TW) const
		{
			return vector4(x, y, z, TW);
		}
		vector4 vector4::mul(float xyzw) const
		{
#ifdef VI_VECTORCLASS
			LOAD_FV4(_r1); _r1 = _r1 * xyzw;
			return vector4(_r1.extract(0), _r1.extract(1), _r1.extract(2), _r1.extract(3));
#else
			return vector4(x * xyzw, y * xyzw, z * xyzw, w * xyzw);
#endif
		}
		vector4 vector4::mul(const vector2& xy, float z, float w) const
		{
#ifdef VI_VECTORCLASS
			LOAD_FV4(_r1); LOAD_AV4(_r2, xy.x, xy.y, z, w); _r1 = _r1 * _r2;
			return vector4(_r1.extract(0), _r1.extract(1), _r1.extract(2), _r1.extract(3));
#else
			return vector4(x * xy.x, y * xy.y, z * z, w * w);
#endif
		}
		vector4 vector4::mul(const vector3& xyz, float w) const
		{
#ifdef VI_VECTORCLASS
			LOAD_FV4(_r1); LOAD_AV4(_r2, xyz.x, xyz.y, xyz.z, w); _r1 = _r1 * _r2;
			return vector4(_r1.extract(0), _r1.extract(1), _r1.extract(2), _r1.extract(3));
#else
			return vector4(x * xyz.x, y * xyz.y, z * xyz.z, w * w);
#endif
		}
		vector4 vector4::mul(const vector4& value) const
		{
#ifdef VI_VECTORCLASS
			LOAD_FV4(_r1); LOAD_V4(_r2, value); _r1 = _r1 * _r2;
			return vector4(_r1.extract(0), _r1.extract(1), _r1.extract(2), _r1.extract(3));
#else
			return vector4(x * value.x, y * value.y, z * value.z, w * value.w);
#endif
		}
		vector4 vector4::div(const vector4& value) const
		{
#ifdef VI_VECTORCLASS
			LOAD_FV4(_r1); LOAD_V4(_r2, value); _r1 = _r1 / _r2;
			return vector4(_r1.extract(0), _r1.extract(1), _r1.extract(2), _r1.extract(3));
#else
			return vector4(x / value.x, y / value.y, z / value.z, w / value.w);
#endif
		}
		vector4 vector4::add(const vector4& value) const
		{
#ifdef VI_VECTORCLASS
			LOAD_FV4(_r1); LOAD_V4(_r2, value); _r1 = _r1 + _r2;
			return vector4(_r1.extract(0), _r1.extract(1), _r1.extract(2), _r1.extract(3));
#else
			return vector4(x + value.x, y + value.y, z + value.z, w + value.w);
#endif
		}
		void vector4::set(const vector4& value)
		{
			x = value.x;
			y = value.y;
			z = value.z;
			w = value.w;
		}
		void vector4::get2(float* in) const
		{
			VI_ASSERT(in != nullptr, "in of size 2 should be set");
			in[0] = x;
			in[1] = y;
		}
		void vector4::get3(float* in) const
		{
			VI_ASSERT(in != nullptr, "in of size 3 should be set");
			in[0] = x;
			in[1] = y;
			in[2] = z;
		}
		void vector4::get4(float* in) const
		{
			VI_ASSERT(in != nullptr, "in of size 4 should be set");
			in[0] = x;
			in[1] = y;
			in[2] = z;
			in[3] = w;
		}
		vector4& vector4::operator *=(const matrix4x4& v)
		{
			set(transform(v));
			return *this;
		}
		vector4& vector4::operator *=(const vector4& v)
		{
#ifdef VI_VECTORCLASS
			LOAD_FV4(_r1); LOAD_V4(_r2, v);
			_r1 = _r1 * _r2;
			_r1.store((float*)this);
#else
			x *= v.x;
			y *= v.y;
			z *= v.z;
			w *= v.w;
#endif
			return *this;
		}
		vector4& vector4::operator *=(float v)
		{
#ifdef VI_VECTORCLASS
			LOAD_FV4(_r1); LOAD_VAL(_r2, v);
			_r1 = _r1 * _r2;
			_r1.store((float*)this);
#else
			x *= v;
			y *= v;
			z *= v;
			w *= v;
#endif
			return *this;
		}
		vector4& vector4::operator /=(const vector4& v)
		{
#ifdef VI_VECTORCLASS
			LOAD_FV4(_r1); LOAD_V4(_r2, v);
			_r1 = _r1 / _r2;
			_r1.store((float*)this);
#else
			x /= v.x;
			y /= v.y;
			z /= v.z;
			w /= v.w;
#endif
			return *this;
		}
		vector4& vector4::operator /=(float v)
		{
#ifdef VI_VECTORCLASS
			LOAD_FV4(_r1); LOAD_VAL(_r2, v);
			_r1 = _r1 / _r2;
			_r1.store((float*)this);
#else
			x /= v;
			y /= v;
			z /= v;
			w /= v;
#endif
			return *this;
		}
		vector4& vector4::operator +=(const vector4& v)
		{
#ifdef VI_VECTORCLASS
			LOAD_FV4(_r1); LOAD_V4(_r2, v);
			_r1 = _r1 + _r2;
			_r1.store((float*)this);
#else
			x += v.x;
			y += v.y;
			z += v.z;
			w += v.w;
#endif
			return *this;
		}
		vector4& vector4::operator +=(float v)
		{
#ifdef VI_VECTORCLASS
			LOAD_FV4(_r1); LOAD_VAL(_r2, v);
			_r1 = _r1 + _r2;
			_r1.store((float*)this);
#else
			x += v;
			y += v;
			z += v;
			w += v;
#endif
			return *this;
		}
		vector4& vector4::operator -=(const vector4& v)
		{
#ifdef VI_VECTORCLASS
			LOAD_FV4(_r1); LOAD_V4(_r2, v);
			_r1 = _r1 - _r2;
			_r1.store((float*)this);
#else
			x -= v.x;
			y -= v.y;
			z -= v.z;
			w -= v.w;
#endif
			return *this;
		}
		vector4& vector4::operator -=(float v)
		{
#ifdef VI_VECTORCLASS
			LOAD_FV4(_r1); LOAD_VAL(_r2, v);
			_r1 = _r1 - _r2;
			_r1.store((float*)this);
#else
			x -= v;
			y -= v;
			z -= v;
			w -= v;
#endif
			return *this;
		}
		vector4 vector4::operator *(const matrix4x4& v) const
		{
			return transform(v);
		}
		vector4 vector4::operator *(const vector4& v) const
		{
			return mul(v);
		}
		vector4 vector4::operator *(float v) const
		{
			return mul(v);
		}
		vector4 vector4::operator /(const vector4& v) const
		{
			return div(v);
		}
		vector4 vector4::operator /(float v) const
		{
			return div(v);
		}
		vector4 vector4::operator +(const vector4& v) const
		{
			return add(v);
		}
		vector4 vector4::operator +(float v) const
		{
			return add(v);
		}
		vector4 vector4::operator -(const vector4& v) const
		{
			return add(-v);
		}
		vector4 vector4::operator -(float v) const
		{
			return add(-v);
		}
		vector4 vector4::operator -() const
		{
			return inv();
		}
		vector4& vector4::operator =(const vector4& v) noexcept
		{
			x = v.x;
			y = v.y;
			z = v.z;
			w = v.w;
			return *this;
		}
		bool vector4::operator ==(const vector4& r) const
		{
			return x == r.x && y == r.y && z == r.z && w == r.w;
		}
		bool vector4::operator !=(const vector4& r) const
		{
			return !(x == r.x && y == r.y && z == r.z && w == r.w);
		}
		bool vector4::operator <=(const vector4& r) const
		{
			return x <= r.x && y <= r.y && z <= r.z && w <= r.w;
		}
		bool vector4::operator >=(const vector4& r) const
		{
			return x >= r.x && y >= r.y && z >= r.z && w >= r.w;
		}
		bool vector4::operator <(const vector4& r) const
		{
			return x < r.x && y < r.y && z < r.z && w < r.w;
		}
		bool vector4::operator >(const vector4& r) const
		{
			return x > r.x && y > r.y && z > r.z && w > r.w;
		}
		float& vector4::operator [](uint32_t axis)
		{
			VI_ASSERT(axis >= 0 && axis <= 3, "index outside of range");
			if (axis == 0)
				return x;
			else if (axis == 1)
				return y;
			else if (axis == 2)
				return z;

			return w;
		}
		float vector4::operator [](uint32_t axis) const
		{
			VI_ASSERT(axis >= 0 && axis <= 3, "index outside of range");
			if (axis == 0)
				return x;
			else if (axis == 1)
				return y;
			else if (axis == 2)
				return z;

			return w;
		}
		vector4 vector4::random()
		{
			return vector4(compute::mathf::random_mag(), compute::mathf::random_mag(), compute::mathf::random_mag(), compute::mathf::random_mag());
		}
		vector4 vector4::random_abs()
		{
			return vector4(compute::mathf::random(), compute::mathf::random(), compute::mathf::random(), compute::mathf::random());
		}

		bounding::bounding() noexcept
		{
		}
		bounding::bounding(const vector3& lower_bound, const vector3& upper_bound) noexcept : lower(lower_bound), upper(upper_bound)
		{
			VI_ASSERT(lower <= upper, "lower should be smaller than upper");
			volume = geometric::aabb_volume(lower, upper);
			radius = ((upper - lower) * 0.5f).sum();
			center = (lower + upper) * 0.5f;
		}
		void bounding::merge(const bounding& a, const bounding& b)
		{
			lower.x = std::min(a.lower.x, b.lower.x);
			lower.y = std::min(a.lower.y, b.lower.y);
			lower.z = std::min(a.lower.z, b.lower.z);
			upper.x = std::max(a.upper.x, b.upper.x);
			upper.y = std::max(a.upper.y, b.upper.y);
			upper.z = std::max(a.upper.z, b.upper.z);
			volume = geometric::aabb_volume(lower, upper);
			radius = ((upper - lower) * 0.5f).sum();
			center = (lower + upper) * 0.5f;
		}
		bool bounding::contains(const bounding& bounds) const
		{
			return bounds.lower >= lower && bounds.upper <= upper;
		}
		bool bounding::overlaps(const bounding& bounds) const
		{
			return bounds.upper >= lower && bounds.lower <= upper;
		}

		frustum8c::frustum8c() noexcept
		{
			geometric::create_frustum8c(corners, 90.0f, 1.0f, 0.1f, 1.0f);
		}
		frustum8c::frustum8c(float field_of_view, float aspect, float near_z, float far_z) noexcept
		{
			geometric::create_frustum8c_rad(corners, field_of_view, aspect, near_z, far_z);
		}
		void frustum8c::transform(const matrix4x4& value)
		{
			for (uint32_t i = 0; i < 8; i++)
			{
				vector4& corner = corners[i];
				corner = corner * value;
			}
		}
		void frustum8c::get_bounding_box(float size, vector2* x, vector2* y, vector2* z)
		{
			VI_ASSERT(x || y || z, "at least one vector of x, y, z should be set");
			float min_x = std::numeric_limits<float>::max();
			float max_x = std::numeric_limits<float>::min();
			float min_y = std::numeric_limits<float>::max();
			float max_y = std::numeric_limits<float>::min();
			float min_z = std::numeric_limits<float>::max();
			float max_z = std::numeric_limits<float>::min();

			for (uint32_t i = 0; i < 8; i++)
			{
				vector4& corner = corners[i];
				min_x = std::min(min_x, corner.x);
				max_x = std::max(max_x, corner.x);
				min_y = std::min(min_y, corner.y);
				max_y = std::max(max_y, corner.y);
				min_z = std::min(min_z, corner.z);
				max_z = std::max(max_z, corner.z);
			}

			if (x != nullptr)
				*x = vector2(min_x, max_x) * size;

			if (y != nullptr)
				*y = vector2(min_y, max_y) * size;

			if (z != nullptr)
				*z = vector2(min_z, max_z) * size;
		}

		frustum6p::frustum6p() noexcept
		{
		}
		frustum6p::frustum6p(const matrix4x4& clip) noexcept
		{
			planes[(size_t)side::RIGHT].x = clip[3] - clip[0];
			planes[(size_t)side::RIGHT].y = clip[7] - clip[4];
			planes[(size_t)side::RIGHT].z = clip[11] - clip[8];
			planes[(size_t)side::RIGHT].w = clip[15] - clip[12];
			normalize_plane(planes[(size_t)side::RIGHT]);

			planes[(size_t)side::LEFT].x = clip[3] + clip[0];
			planes[(size_t)side::LEFT].y = clip[7] + clip[4];
			planes[(size_t)side::LEFT].z = clip[11] + clip[8];
			planes[(size_t)side::LEFT].w = clip[15] + clip[12];
			normalize_plane(planes[(size_t)side::LEFT]);

			planes[(size_t)side::BOTTOM].x = clip[3] + clip[1];
			planes[(size_t)side::BOTTOM].y = clip[7] + clip[5];
			planes[(size_t)side::BOTTOM].z = clip[11] + clip[9];
			planes[(size_t)side::BOTTOM].w = clip[15] + clip[13];
			normalize_plane(planes[(size_t)side::BOTTOM]);

			planes[(size_t)side::TOP].x = clip[3] - clip[1];
			planes[(size_t)side::TOP].y = clip[7] - clip[5];
			planes[(size_t)side::TOP].z = clip[11] - clip[9];
			planes[(size_t)side::TOP].w = clip[15] - clip[13];
			normalize_plane(planes[(size_t)side::TOP]);

			planes[(size_t)side::BACK].x = clip[3] - clip[2];
			planes[(size_t)side::BACK].y = clip[7] - clip[6];
			planes[(size_t)side::BACK].z = clip[11] - clip[10];
			planes[(size_t)side::BACK].w = clip[15] - clip[14];
			normalize_plane(planes[(size_t)side::BACK]);

			planes[(size_t)side::FRONT].x = clip[3] + clip[2];
			planes[(size_t)side::FRONT].y = clip[7] + clip[6];
			planes[(size_t)side::FRONT].z = clip[11] + clip[10];
			planes[(size_t)side::FRONT].w = clip[15] + clip[14];
			normalize_plane(planes[(size_t)side::FRONT]);
		}
		void frustum6p::normalize_plane(vector4& plane)
		{
			float magnitude = (float)sqrt(plane.x * plane.x + plane.y * plane.y + plane.z * plane.z);
			plane.x /= magnitude;
			plane.y /= magnitude;
			plane.z /= magnitude;
			plane.w /= magnitude;
		}
		bool frustum6p::overlaps_aabb(const bounding& bounds) const
		{
			const vector3& mid = bounds.center;
			const vector3& min = bounds.lower;
			const vector3& max = bounds.upper;
			float distance = -bounds.radius;
#ifdef VI_VECTORCLASS
			LOAD_AV4(_rc, mid.x, mid.y, mid.z, 1.0f);
			LOAD_AV4(_m1, min.x, min.y, min.z, 1.0f);
			LOAD_AV4(_m2, max.x, min.y, min.z, 1.0f);
			LOAD_AV4(_m3, min.x, max.y, min.z, 1.0f);
			LOAD_AV4(_m4, max.x, max.y, min.z, 1.0f);
			LOAD_AV4(_m5, min.x, min.y, max.z, 1.0f);
			LOAD_AV4(_m6, max.x, min.y, max.z, 1.0f);
			LOAD_AV4(_m7, min.x, max.y, max.z, 1.0f);
			LOAD_AV4(_m8, max.x, max.y, max.z, 1.0f);
#else
			vector4 RC(mid.x, mid.y, mid.z, 1.0f);
			vector4 M1(min.x, min.y, min.z, 1.0f);
			vector4 M2(max.x, min.y, min.z, 1.0f);
			vector4 M3(min.x, max.y, min.z, 1.0f);
			vector4 M4(max.x, max.y, min.z, 1.0f);
			vector4 M5(min.x, min.y, max.z, 1.0f);
			vector4 M6(max.x, min.y, max.z, 1.0f);
			vector4 M7(min.x, max.y, max.z, 1.0f);
			vector4 M8(max.x, max.y, max.z, 1.0f);
#endif
			for (size_t i = 0; i < 6; i++)
			{
#ifdef VI_VECTORCLASS
				LOAD_V4(_rp, planes[i]);
				if (horizontal_add(_rc * _rp) < distance)
					return false;

				if (horizontal_add(_rp * _m1) > 0.0f)
					continue;

				if (horizontal_add(_rp * _m2) > 0.0f)
					continue;

				if (horizontal_add(_rp * _m3) > 0.0f)
					continue;

				if (horizontal_add(_rp * _m4) > 0.0f)
					continue;

				if (horizontal_add(_rp * _m5) > 0.0f)
					continue;

				if (horizontal_add(_rp * _m6) > 0.0f)
					continue;

				if (horizontal_add(_rp * _m7) > 0.0f)
					continue;

				if (horizontal_add(_rp * _m8) > 0.0f)
					continue;
#else
				auto& plane = planes[i];
				if (plane.dot(RC) < distance)
					return false;

				if (plane.dot(M1) > 0)
					continue;

				if (plane.dot(M2) > 0)
					continue;

				if (plane.dot(M3) > 0)
					continue;

				if (plane.dot(M4) > 0)
					continue;

				if (plane.dot(M5) > 0)
					continue;

				if (plane.dot(M6) > 0)
					continue;

				if (plane.dot(M7) > 0)
					continue;

				if (plane.dot(M8) > 0)
					continue;
#endif
				return false;
			}

			return true;
		}
		bool frustum6p::overlaps_sphere(const vector3& center, float radius) const
		{
			vector4 position(center.x, center.y, center.z, 1.0f);
			float distance = -radius;

			for (size_t i = 0; i < 6; i++)
			{
				if (position.dot(planes[i]) < distance)
					return false;
			}

			return true;
		}

		ray::ray() noexcept : direction(0, 0, 1)
		{
		}
		ray::ray(const vector3& _Origin, const vector3& _Direction) noexcept : origin(_Origin), direction(_Direction)
		{
		}
		vector3 ray::get_point(float t) const
		{
			return origin + (direction * t);
		}
		vector3 ray::operator *(float t) const
		{
			return get_point(t);
		}
		bool ray::intersects_plane(const vector3& normal, float diameter) const
		{
			float d = normal.dot(direction);
			if (compute::mathf::abs(d) < std::numeric_limits<float>::epsilon())
				return false;

			float n = normal.dot(origin) + diameter;
			float t = -(n / d);
			return t >= 0;
		}
		bool ray::intersects_sphere(const vector3& position, float radius, bool discard_inside) const
		{
			vector3 r = origin - position;
			float l = r.length();

			if (l * l <= radius * radius && discard_inside)
				return true;

			float a = direction.dot(direction);
			float b = 2 * r.dot(direction);
			float c = r.dot(r) - radius * radius;
			float d = (b * b) - (4 * a * c);

			return d >= 0.0f;
		}
		bool ray::intersects_aabb_at(const vector3& min, const vector3& max, vector3* hit) const
		{
			vector3 hit_point; float t;
			if (origin > min && origin < max)
				return true;

			if (origin.x <= min.x && direction.x > 0)
			{
				t = (min.x - origin.x) / direction.x;
				hit_point = origin + direction * t;

				if (hit_point.y >= min.y && hit_point.y <= max.y && hit_point.z >= min.z && hit_point.z <= max.z)
				{
					if (hit != nullptr)
						*hit = hit_point;

					return true;
				}
			}

			if (origin.x >= max.x && direction.x < 0)
			{
				t = (max.x - origin.x) / direction.x;
				hit_point = origin + direction * t;

				if (hit_point.y >= min.y && hit_point.y <= max.y && hit_point.z >= min.z && hit_point.z <= max.z)
				{
					if (hit != nullptr)
						*hit = hit_point;

					return true;
				}
			}

			if (origin.y <= min.y && direction.y > 0)
			{
				t = (min.y - origin.y) / direction.y;
				hit_point = origin + direction * t;

				if (hit_point.x >= min.x && hit_point.x <= max.x && hit_point.z >= min.z && hit_point.z <= max.z)
				{
					if (hit != nullptr)
						*hit = hit_point;

					return true;
				}
			}

			if (origin.y >= max.y && direction.y < 0)
			{
				t = (max.y - origin.y) / direction.y;
				hit_point = origin + direction * t;

				if (hit_point.x >= min.x && hit_point.x <= max.x && hit_point.z >= min.z && hit_point.z <= max.z)
				{
					if (hit != nullptr)
						*hit = hit_point;

					return true;
				}
			}

			if (origin.z <= min.z && direction.z > 0)
			{
				t = (min.z - origin.z) / direction.z;
				hit_point = origin + direction * t;

				if (hit_point.x >= min.x && hit_point.x <= max.x && hit_point.y >= min.y && hit_point.y <= max.y)
				{
					if (hit != nullptr)
						*hit = hit_point;

					return true;
				}
			}

			if (origin.z >= max.z && direction.z < 0)
			{
				t = (max.z - origin.z) / direction.z;
				hit_point = origin + direction * t;

				if (hit_point.x >= min.x && hit_point.x <= max.x && hit_point.y >= min.y && hit_point.y <= max.y)
				{
					if (hit != nullptr)
						*hit = hit_point;

					return true;
				}
			}

			return false;
		}
		bool ray::intersects_aabb(const vector3& position, const vector3& scale, vector3* hit) const
		{
			vector3 min = position - scale;
			vector3 max = position + scale;
			return intersects_aabb_at(min, max, hit);
		}
		bool ray::intersects_obb(const matrix4x4& world, vector3* hit) const
		{
			matrix4x4 offset = world.inv();
			vector3 min = -1.0f, max = 1.0f;
			vector3 o = (vector4(origin.x, origin.y, origin.z, 1.0f) * offset).xyz();
			if (o > min && o < max)
				return true;

			vector3 d = (direction.xyzw() * offset).snormalize().xyz();
			vector3 hit_point; float t;

			if (o.x <= min.x && d.x > 0)
			{
				t = (min.x - o.x) / d.x;
				hit_point = o + d * t;

				if (hit_point.y >= min.y && hit_point.y <= max.y && hit_point.z >= min.z && hit_point.z <= max.z)
				{
					if (hit != nullptr)
						*hit = hit_point.transform(world);

					return true;
				}
			}

			if (o.x >= max.x && d.x < 0)
			{
				t = (max.x - o.x) / d.x;
				hit_point = o + d * t;

				if (hit_point.y >= min.y && hit_point.y <= max.y && hit_point.z >= min.z && hit_point.z <= max.z)
				{
					if (hit != nullptr)
						*hit = hit_point.transform(world);

					return true;
				}
			}

			if (o.y <= min.y && d.y > 0)
			{
				t = (min.y - o.y) / d.y;
				hit_point = o + d * t;

				if (hit_point.x >= min.x && hit_point.x <= max.x && hit_point.z >= min.z && hit_point.z <= max.z)
				{
					if (hit != nullptr)
						*hit = hit_point.transform(world);

					return true;
				}
			}

			if (o.y >= max.y && d.y < 0)
			{
				t = (max.y - o.y) / d.y;
				hit_point = o + d * t;

				if (hit_point.x >= min.x && hit_point.x <= max.x && hit_point.z >= min.z && hit_point.z <= max.z)
				{
					if (hit != nullptr)
						*hit = hit_point.transform(world);

					return true;
				}
			}

			if (o.z <= min.z && d.z > 0)
			{
				t = (min.z - o.z) / d.z;
				hit_point = o + d * t;

				if (hit_point.x >= min.x && hit_point.x <= max.x && hit_point.y >= min.y && hit_point.y <= max.y)
				{
					if (hit != nullptr)
						*hit = hit_point.transform(world);

					return true;
				}
			}

			if (o.z >= max.z && d.z < 0)
			{
				t = (max.z - o.z) / d.z;
				hit_point = o + d * t;

				if (hit_point.x >= min.x && hit_point.x <= max.x && hit_point.y >= min.y && hit_point.y <= max.y)
				{
					if (hit != nullptr)
						*hit = hit_point.transform(world);

					return true;
				}
			}

			return false;
		}

		matrix4x4::matrix4x4() noexcept : row { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 }
		{
		}
		matrix4x4::matrix4x4(float array[16]) noexcept
		{
			memcpy(row, array, sizeof(float) * 16);
		}
		matrix4x4::matrix4x4(const vector4& row0, const vector4& row1, const vector4& row2, const vector4& row3) noexcept
		{
			memcpy(row + 0, &row0, sizeof(vector4));
			memcpy(row + 4, &row1, sizeof(vector4));
			memcpy(row + 8, &row2, sizeof(vector4));
			memcpy(row + 12, &row3, sizeof(vector4));
		}
		matrix4x4::matrix4x4(float row00, float row01, float row02, float row03, float row10, float row11, float row12, float row13, float row20, float row21, float row22, float row23, float row30, float row31, float row32, float row33) noexcept :
			row { row00, row01, row02, row03, row10, row11, row12, row13, row20, row21, row22, row23, row30, row31, row32, row33 }
		{
		}
		matrix4x4::matrix4x4(bool) noexcept
		{
		}
		matrix4x4::matrix4x4(const matrix4x4& v) noexcept
		{
			memcpy(row, v.row, sizeof(float) * 16);
		}
		float& matrix4x4::operator [](uint32_t index)
		{
			return row[index];
		}
		float matrix4x4::operator [](uint32_t index) const
		{
			return row[index];
		}
		bool matrix4x4::operator ==(const matrix4x4& equal) const
		{
			return memcmp(row, equal.row, sizeof(float) * 16) == 0;
		}
		bool matrix4x4::operator !=(const matrix4x4& equal) const
		{
			return memcmp(row, equal.row, sizeof(float) * 16) != 0;
		}
		matrix4x4 matrix4x4::operator *(const matrix4x4& v) const
		{
			return this->mul(v);
		}
		vector4 matrix4x4::operator *(const vector4& v) const
		{
			matrix4x4 result = this->mul(v);
			return vector4(result.row[0], result.row[4], result.row[8], result.row[12]);
		}
		matrix4x4& matrix4x4::operator =(const matrix4x4& v) noexcept
		{
			memcpy(row, v.row, sizeof(float) * 16);
			return *this;
		}
		vector4 matrix4x4::row11() const
		{
			return vector4(row[0], row[1], row[2], row[3]);
		}
		vector4 matrix4x4::row22() const
		{
			return vector4(row[4], row[5], row[6], row[7]);
		}
		vector4 matrix4x4::row33() const
		{
			return vector4(row[8], row[9], row[10], row[11]);
		}
		vector4 matrix4x4::row44() const
		{
			return vector4(row[12], row[13], row[14], row[15]);
		}
		vector3 matrix4x4::up() const
		{
			return vector3(-row[4], row[5], row[6]);
		}
		vector3 matrix4x4::right() const
		{
			return vector3(-row[0], row[1], row[2]);
		}
		vector3 matrix4x4::forward() const
		{
			return vector3(-row[8], row[9], row[10]);
		}
		matrix4x4 matrix4x4::inv() const
		{
			matrix4x4 result(true);
			float A2323 = row[10] * row[15] - row[11] * row[14];
			float A1323 = row[9] * row[15] - row[11] * row[13];
			float A1223 = row[9] * row[14] - row[10] * row[13];
			float A0323 = row[8] * row[15] - row[11] * row[12];
			float A0223 = row[8] * row[14] - row[10] * row[12];
			float A0123 = row[8] * row[13] - row[9] * row[12];
			float A2313 = row[6] * row[15] - row[7] * row[14];
			float A1313 = row[5] * row[15] - row[7] * row[13];
			float A1213 = row[5] * row[14] - row[6] * row[13];
			float A2312 = row[6] * row[11] - row[7] * row[10];
			float A1312 = row[5] * row[11] - row[7] * row[9];
			float A1212 = row[5] * row[10] - row[6] * row[9];
			float A0313 = row[4] * row[15] - row[7] * row[12];
			float A0213 = row[4] * row[14] - row[6] * row[12];
			float A0312 = row[4] * row[11] - row[7] * row[8];
			float A0212 = row[4] * row[10] - row[6] * row[8];
			float A0113 = row[4] * row[13] - row[5] * row[12];
			float A0112 = row[4] * row[9] - row[5] * row[8];
#ifdef VI_VECTORCLASS
			LOAD_AV4(_r1, row[5], row[4], row[4], row[4]);
			LOAD_AV4(_r2, A2323, A2323, A1323, A1223);
			LOAD_AV4(_r3, row[6], row[6], row[5], row[5]);
			LOAD_AV4(_r4, A1323, A0323, A0323, A0223);
			LOAD_AV4(_r5, row[7], row[7], row[7], row[6]);
			LOAD_AV4(_r6, A1223, A0223, A0123, A0123);
			LOAD_AV4(_r7, row[0], -row[1], row[2], -row[3]);
			LOAD_AV4(_r8, 1.0f, -1.0f, 1.0f, -1.0f);
			_r7 *= _r1 * _r2 - _r3 * _r4 + _r5 * _r6;
			float f = horizontal_add(_r7);
			f = 1.0f / (f != 0.0f ? f : 1.0f);
			_r1 = Vec4f(row[5], row[1], row[1], row[1]);
			_r2 = Vec4f(A2323, A2323, A2313, A2312);
			_r3 = Vec4f(row[6], row[2], row[2], row[2]);
			_r4 = Vec4f(A1323, A1323, A1313, A1312);
			_r5 = Vec4f(row[7], row[3], row[3], row[3]);
			_r6 = Vec4f(A1223, A1223, A1213, A1212);
			_r7 = (_r1 * _r2 - _r3 * _r4 + _r5 * _r6) * _r8 * f;
			_r7.store(result.row + 0);
			_r1 = Vec4f(row[4], row[0], row[0], row[0]);
			_r4 = Vec4f(A0323, A0323, A0313, A0312);
			_r6 = Vec4f(A0223, A0223, A0213, A0212);
			_r7 = (_r1 * _r2 - _r3 * _r4 + _r5 * _r6) * -_r8 * f;
			_r7.store(result.row + 4);
			_r2 = Vec4f(A1323, A1323, A1313, A1312);
			_r3 = Vec4f(row[5], row[1], row[1], row[1]);
			_r6 = Vec4f(A0123, A0123, A0113, A0112);
			_r7 = (_r1 * _r2 - _r3 * _r4 + _r5 * _r6) * _r8 * f;
			_r7.store(result.row + 8);
			_r2 = Vec4f(A1223, A1223, A1213, A1212);
			_r4 = Vec4f(A0223, A0223, A0213, A0212);
			_r5 = Vec4f(row[6], row[2], row[2], row[2]);
			_r7 = (_r1 * _r2 - _r3 * _r4 + _r5 * _r6) * -_r8 * f;
			_r7.store(result.row + 12);
#else
			float f =
				row[0] * (row[5] * A2323 - row[6] * A1323 + row[7] * A1223)
				- row[1] * (row[4] * A2323 - row[6] * A0323 + row[7] * A0223)
				+ row[2] * (row[4] * A1323 - row[5] * A0323 + row[7] * A0123)
				- row[3] * (row[4] * A1223 - row[5] * A0223 + row[6] * A0123);
			f = 1.0f / (f != 0.0f ? f : 1.0f);

			result.row[0] = f * (row[5] * A2323 - row[6] * A1323 + row[7] * A1223);
			result.row[1] = f * -(row[1] * A2323 - row[2] * A1323 + row[3] * A1223);
			result.row[2] = f * (row[1] * A2313 - row[2] * A1313 + row[3] * A1213);
			result.row[3] = f * -(row[1] * A2312 - row[2] * A1312 + row[3] * A1212);
			result.row[4] = f * -(row[4] * A2323 - row[6] * A0323 + row[7] * A0223);
			result.row[5] = f * (row[0] * A2323 - row[2] * A0323 + row[3] * A0223);
			result.row[6] = f * -(row[0] * A2313 - row[2] * A0313 + row[3] * A0213);
			result.row[7] = f * (row[0] * A2312 - row[2] * A0312 + row[3] * A0212);
			result.row[8] = f * (row[4] * A1323 - row[5] * A0323 + row[7] * A0123);
			result.row[9] = f * -(row[0] * A1323 - row[1] * A0323 + row[3] * A0123);
			result.row[10] = f * (row[0] * A1313 - row[1] * A0313 + row[3] * A0113);
			result.row[11] = f * -(row[0] * A1312 - row[1] * A0312 + row[3] * A0112);
			result.row[12] = f * -(row[4] * A1223 - row[5] * A0223 + row[6] * A0123);
			result.row[13] = f * (row[0] * A1223 - row[1] * A0223 + row[2] * A0123);
			result.row[14] = f * -(row[0] * A1213 - row[1] * A0213 + row[2] * A0113);
			result.row[15] = f * (row[0] * A1212 - row[1] * A0212 + row[2] * A0112);
#endif
			return result;
		}
		matrix4x4 matrix4x4::transpose() const
		{
#ifdef VI_VECTORCLASS
			LOAD_FV16(_r1);
			_r1 = permute16f<0, 4, 8, 12, 1, 5, 9, 13, 2, 6, 10, 14, 3, 7, 11, 15>(_r1);

			matrix4x4 result(true);
			_r1.store(result.row);

			return result;
#else
			return matrix4x4(
				vector4(row[0], row[4], row[8], row[12]),
				vector4(row[1], row[5], row[9], row[13]),
				vector4(row[2], row[6], row[10], row[14]),
				vector4(row[3], row[7], row[11], row[15]));
#endif
		}
		quaternion matrix4x4::rotation_quaternion() const
		{
			vector3 scaling[3] =
			{
				vector3(row[0], row[1], row[2]),
				vector3(row[4], row[5], row[6]),
				vector3(row[8], row[9], row[10])
			};

			vector3 scale = { scaling[0].length(), scaling[1].length(), scaling[2].length() };
			if (determinant() < 0)
				scale = -scale;

			if (scale.x)
				scaling[0] /= scale.x;

			if (scale.y)
				scaling[1] /= scale.y;

			if (scale.z)
				scaling[2] /= scale.z;

			matrix4x4 rotated =
			{
				scaling[0].x, scaling[1].x, scaling[2].x, 0.0f,
				scaling[0].y, scaling[1].y, scaling[2].y, 0.0f,
				scaling[0].z, scaling[1].z, scaling[2].z, 0.0f,
				0.0f, 0.0f, 0.0f, 1.0f
			};

			return quaternion(rotated);
		}
		vector3 matrix4x4::rotation_euler() const
		{
			return rotation_quaternion().get_euler();
		}
		vector3 matrix4x4::position() const
		{
			return vector3(row[12], row[13], row[14]);
		}
		vector3 matrix4x4::scale() const
		{
			vector3 scale =
			{
				vector3(row[0], row[1], row[2]).length(),
				vector3(row[4], row[5], row[6]).length(),
				vector3(row[8], row[9], row[10]).length()
			};

			if (determinant() < 0)
				scale = -scale;

			return scale;
		}
		matrix4x4 matrix4x4::set_scale(const vector3& value) const
		{
			matrix4x4 local = *this;
			local.row[0] = value.x;
			local.row[5] = value.y;
			local.row[10] = value.z;
			return local;
		}
		matrix4x4 matrix4x4::mul(const matrix4x4& v) const
		{
			matrix4x4 result;
#ifdef VI_VECTORCLASS
			LOAD_VAR(_r1, v.row + 0);
			LOAD_VAR(_r2, v.row + 4);
			LOAD_VAR(_r3, v.row + 8);
			LOAD_VAR(_r4, v.row + 12);
			LOAD_VAL(_r5, 0.0f);

			_r5 += _r1 * row[0];
			_r5 += _r2 * row[1];
			_r5 += _r3 * row[2];
			_r5 += _r4 * row[3];
			_r5.store(result.row + 0);
			_r5 = Vec4f(0.0f);
			_r5 += _r1 * row[4];
			_r5 += _r2 * row[5];
			_r5 += _r3 * row[6];
			_r5 += _r4 * row[7];
			_r5.store(result.row + 4);
			_r5 = Vec4f(0.0f);
			_r5 += _r1 * row[8];
			_r5 += _r2 * row[9];
			_r5 += _r3 * row[10];
			_r5 += _r4 * row[11];
			_r5.store(result.row + 8);
			_r5 = Vec4f(0.0f);
			_r5 += _r1 * row[12];
			_r5 += _r2 * row[13];
			_r5 += _r3 * row[14];
			_r5 += _r4 * row[15];
			_r5.store(result.row + 12);
#else
			result.row[0] = (row[0] * v.row[0]) + (row[1] * v.row[4]) + (row[2] * v.row[8]) + (row[3] * v.row[12]);
			result.row[1] = (row[0] * v.row[1]) + (row[1] * v.row[5]) + (row[2] * v.row[9]) + (row[3] * v.row[13]);
			result.row[2] = (row[0] * v.row[2]) + (row[1] * v.row[6]) + (row[2] * v.row[10]) + (row[3] * v.row[14]);
			result.row[3] = (row[0] * v.row[3]) + (row[1] * v.row[7]) + (row[2] * v.row[11]) + (row[3] * v.row[15]);
			result.row[4] = (row[4] * v.row[0]) + (row[5] * v.row[4]) + (row[6] * v.row[8]) + (row[7] * v.row[12]);
			result.row[5] = (row[4] * v.row[1]) + (row[5] * v.row[5]) + (row[6] * v.row[9]) + (row[7] * v.row[13]);
			result.row[6] = (row[4] * v.row[2]) + (row[5] * v.row[6]) + (row[6] * v.row[10]) + (row[7] * v.row[14]);
			result.row[7] = (row[4] * v.row[3]) + (row[5] * v.row[7]) + (row[6] * v.row[11]) + (row[7] * v.row[15]);
			result.row[8] = (row[8] * v.row[0]) + (row[9] * v.row[4]) + (row[10] * v.row[8]) + (row[11] * v.row[12]);
			result.row[9] = (row[8] * v.row[1]) + (row[9] * v.row[5]) + (row[10] * v.row[9]) + (row[11] * v.row[13]);
			result.row[10] = (row[8] * v.row[2]) + (row[9] * v.row[6]) + (row[10] * v.row[10]) + (row[11] * v.row[14]);
			result.row[11] = (row[8] * v.row[3]) + (row[9] * v.row[7]) + (row[10] * v.row[11]) + (row[11] * v.row[15]);
			result.row[12] = (row[12] * v.row[0]) + (row[13] * v.row[4]) + (row[14] * v.row[8]) + (row[15] * v.row[12]);
			result.row[13] = (row[12] * v.row[1]) + (row[13] * v.row[5]) + (row[14] * v.row[9]) + (row[15] * v.row[13]);
			result.row[14] = (row[12] * v.row[2]) + (row[13] * v.row[6]) + (row[14] * v.row[10]) + (row[15] * v.row[14]);
			result.row[15] = (row[12] * v.row[3]) + (row[13] * v.row[7]) + (row[14] * v.row[11]) + (row[15] * v.row[15]);
#endif
			return result;
		}
		matrix4x4 matrix4x4::mul(const vector4& v) const
		{
			matrix4x4 result;
#ifdef VI_VECTORCLASS
			LOAD_V4(_r1, v);
			LOAD_VAR(_r2, row + 0);
			LOAD_VAR(_r3, row + 4);
			LOAD_VAR(_r4, row + 8);
			LOAD_VAR(_r5, row + 12);
			LOAD_VAL(_r6, 0.0f);

			_r6 = horizontal_add(_r1 * _r2);
			_r6.store(result.row + 0);
			_r6 = horizontal_add(_r1 * _r3);
			_r6.store(result.row + 4);
			_r6 = horizontal_add(_r1 * _r4);
			_r6.store(result.row + 8);
			_r6 = horizontal_add(_r1 * _r5);
			_r6.store(result.row + 12);
#else
			float x = (row[0] * v.x) + (row[1] * v.y) + (row[2] * v.z) + (row[3] * v.w);
			result.row[0] = result.row[1] = result.row[2] = result.row[3] = x;

			float y = (row[4] * v.x) + (row[5] * v.y) + (row[6] * v.z) + (row[7] * v.w);
			result.row[4] = result.row[5] = result.row[6] = result.row[7] = y;

			float z = (row[8] * v.x) + (row[9] * v.y) + (row[10] * v.z) + (row[11] * v.w);
			result.row[8] = result.row[9] = result.row[10] = result.row[11] = z;

			float w = (row[12] * v.x) + (row[13] * v.y) + (row[14] * v.z) + (row[15] * v.w);
			result.row[12] = result.row[13] = result.row[14] = result.row[15] = w;
#endif
			return result;
		}
		vector2 matrix4x4::xy() const
		{
#ifdef VI_VECTORCLASS
			LOAD_AV4(_r1, row[0], row[4], row[8], row[12]);
			LOAD_AV4(_r2, row[1], row[5], row[9], row[13]);
			return vector2(horizontal_add(_r1), horizontal_add(_r2));
#else
			return vector2(
				row[0] + row[4] + row[8] + row[12],
				row[1] + row[5] + row[9] + row[13]);
#endif
		}
		vector3 matrix4x4::xyz() const
		{
#ifdef VI_VECTORCLASS
			LOAD_AV4(_r1, row[0], row[4], row[8], row[12]);
			LOAD_AV4(_r2, row[1], row[5], row[9], row[13]);
			LOAD_AV4(_r3, row[2], row[6], row[10], row[14]);
			return vector3(horizontal_add(_r1), horizontal_add(_r2), horizontal_add(_r3));
#else
			return vector3(
				row[0] + row[4] + row[8] + row[12],
				row[1] + row[5] + row[9] + row[13],
				row[2] + row[6] + row[10] + row[14]);
#endif
		}
		vector4 matrix4x4::xyzw() const
		{
#ifdef VI_VECTORCLASS
			LOAD_AV4(_r1, row[0], row[4], row[8], row[12]);
			LOAD_AV4(_r2, row[1], row[5], row[9], row[13]);
			LOAD_AV4(_r3, row[2], row[6], row[10], row[14]);
			LOAD_AV4(_r4, row[3], row[7], row[11], row[15]);
			return vector4(horizontal_add(_r1), horizontal_add(_r2), horizontal_add(_r3), horizontal_add(_r4));
#else
			return vector4(
				row[0] + row[4] + row[8] + row[12],
				row[1] + row[5] + row[9] + row[13],
				row[2] + row[6] + row[10] + row[14],
				row[3] + row[7] + row[11] + row[15]);
#endif
		}
		float matrix4x4::determinant() const
		{
			return row[0] * row[5] * row[10] * row[15] - row[0] * row[5] * row[11] * row[14] + row[0] * row[6] * row[11] * row[13] - row[0] * row[6] * row[9] * row[15]
				+ row[0] * row[7] * row[9] * row[14] - row[0] * row[7] * row[10] * row[13] - row[1] * row[6] * row[11] * row[12] + row[1] * row[6] * row[8] * row[15]
				- row[1] * row[7] * row[8] * row[14] + row[1] * row[7] * row[10] * row[12] - row[1] * row[4] * row[10] * row[15] + row[1] * row[4] * row[11] * row[14]
				+ row[2] * row[7] * row[8] * row[13] - row[2] * row[7] * row[9] * row[12] + row[2] * row[4] * row[9] * row[15] - row[2] * row[4] * row[11] * row[13]
				+ row[2] * row[5] * row[11] * row[12] - row[2] * row[5] * row[8] * row[15] - row[3] * row[4] * row[9] * row[14] + row[3] * row[4] * row[10] * row[13]
				- row[3] * row[5] * row[10] * row[12] + row[3] * row[5] * row[8] * row[14] - row[3] * row[6] * row[8] * row[13] + row[3] * row[6] * row[9] * row[12];
		}
		void matrix4x4::identify()
		{
			static float base[16] = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
			memcpy(row, base, sizeof(float) * 16);
		}
		void matrix4x4::set(const matrix4x4& value)
		{
			memcpy(row, value.row, sizeof(float) * 16);
		}
		matrix4x4 matrix4x4::create_translated_scale(const vector3& position, const vector3& scale)
		{
			matrix4x4 value;
			value.row[12] = position.x;
			value.row[13] = position.y;
			value.row[14] = position.z;
			value.row[0] = scale.x;
			value.row[5] = scale.y;
			value.row[10] = scale.z;

			return value;
		}
		matrix4x4 matrix4x4::create_rotation_x(float rotation)
		{
			float cos = ::cos(rotation);
			float sin = ::sin(rotation);
			matrix4x4 x;
			x.row[5] = cos;
			x.row[6] = sin;
			x.row[9] = -sin;
			x.row[10] = cos;

			return x;
		}
		matrix4x4 matrix4x4::create_rotation_y(float rotation)
		{
			float cos = ::cos(rotation);
			float sin = ::sin(rotation);
			matrix4x4 y;
			y.row[0] = cos;
			y.row[2] = -sin;
			y.row[8] = sin;
			y.row[10] = cos;

			return y;
		}
		matrix4x4 matrix4x4::create_rotation_z(float rotation)
		{
			float cos = ::cos(rotation);
			float sin = ::sin(rotation);
			matrix4x4 z;
			z.row[0] = cos;
			z.row[1] = sin;
			z.row[4] = -sin;
			z.row[5] = cos;

			return z;
		}
		matrix4x4 matrix4x4::create_rotation(const vector3& rotation)
		{
			return matrix4x4::create_rotation_x(rotation.x) * matrix4x4::create_rotation_y(rotation.y) * matrix4x4::create_rotation_z(rotation.z);
		}
		matrix4x4 matrix4x4::create_scale(const vector3& scale)
		{
			matrix4x4 result;
			result.row[0] = scale.x;
			result.row[5] = scale.y;
			result.row[10] = scale.z;

			return result;
		}
		matrix4x4 matrix4x4::create_translation(const vector3& position)
		{
			matrix4x4 result;
			result.row[12] = position.x;
			result.row[13] = position.y;
			result.row[14] = position.z;

			return result;
		}
		matrix4x4 matrix4x4::create_perspective_rad(float field_of_view, float aspect_ratio, float near_z, float far_z)
		{
			float height = 1.0f / std::tan(0.5f * field_of_view);
			float width = height / aspect_ratio;
			float depth = 1.0f / (far_z - near_z);

			return matrix4x4(
				vector4(width, 0, 0, 0),
				vector4(0, height, 0, 0),
				vector4(0, 0, far_z * depth, 1),
				vector4(0, 0, -near_z * far_z * depth, 0));
		}
		matrix4x4 matrix4x4::create_perspective(float field_of_view, float aspect_ratio, float near_z, float far_z)
		{
			return create_perspective_rad(compute::mathf::deg2rad() * field_of_view, aspect_ratio, near_z, far_z);
		}
		matrix4x4 matrix4x4::create_orthographic(float width, float height, float near_z, float far_z)
		{
			if (geometric::is_left_handed())
			{
				float depth = 1.0f / (far_z - near_z);
				return matrix4x4(
					vector4(2 / width, 0, 0, 0),
					vector4(0, 2 / height, 0, 0),
					vector4(0, 0, depth, 0),
					vector4(0, 0, -depth * near_z, 1));
			}
			else
			{
				float depth = 1.0f / (near_z - far_z);
				return matrix4x4(
					vector4(2 / width, 0, 0, 0),
					vector4(0, 2 / height, 0, 0),
					vector4(0, 0, depth, 0),
					vector4(0, 0, depth * near_z, 1));
			}
		}
		matrix4x4 matrix4x4::create_orthographic_off_center(float left, float right, float bottom, float top, float near_z, float far_z)
		{
			float width = 1.0f / (right - left);
			float height = 1.0f / (top - bottom);
			float depth = 1.0f / (far_z - near_z);

			return matrix4x4(
				vector4(2 * width, 0, 0, 0),
				vector4(0, 2 * height, 0, 0),
				vector4(0, 0, depth, 0),
				vector4(-(left + right) * width, -(top + bottom) * height, -depth * near_z, 1));
		}
		matrix4x4 matrix4x4::create(const vector3& position, const vector3& scale, const vector3& rotation)
		{
			return matrix4x4::create_scale(scale) * matrix4x4::create(position, rotation);
		}
		matrix4x4 matrix4x4::create(const vector3& position, const vector3& rotation)
		{
			return matrix4x4::create_rotation(rotation) * matrix4x4::create_translation(position);
		}
		matrix4x4 matrix4x4::create_view(const vector3& position, const vector3& rotation)
		{
			return
				matrix4x4::create_translation(-position) *
				matrix4x4::create_rotation_y(rotation.y) *
				matrix4x4::create_rotation_x(-rotation.x) *
				matrix4x4::create_scale(vector3(-1.0f, 1.0f, 1.0f)) *
				matrix4x4::create_rotation_z(rotation.z);
		}
		matrix4x4 matrix4x4::create_look_at(const vector3& position, const vector3& target, const vector3& up)
		{
			vector3 z = (target - position).normalize();
			vector3 x = up.cross(z).normalize();
			vector3 y = z.cross(x);

			matrix4x4 result(true);
			result.row[0] = x.x;
			result.row[1] = y.x;
			result.row[2] = z.x;
			result.row[3] = 0;
			result.row[4] = x.y;
			result.row[5] = y.y;
			result.row[6] = z.y;
			result.row[7] = 0;
			result.row[8] = x.z;
			result.row[9] = y.z;
			result.row[10] = z.z;
			result.row[11] = 0;
			result.row[12] = -x.dot(position);
			result.row[13] = -y.dot(position);
			result.row[14] = -z.dot(position);
			result.row[15] = 1;

			return result;
		}
		matrix4x4 matrix4x4::create_rotation(const vector3& forward, const vector3& up, const vector3& right)
		{
			matrix4x4 rotation(true);
			rotation.row[0] = right.x;
			rotation.row[1] = right.y;
			rotation.row[2] = right.z;
			rotation.row[3] = 0;
			rotation.row[4] = up.x;
			rotation.row[5] = up.y;
			rotation.row[6] = up.z;
			rotation.row[7] = 0;
			rotation.row[8] = forward.x;
			rotation.row[9] = forward.y;
			rotation.row[10] = forward.z;
			rotation.row[11] = 0;
			rotation.row[12] = 0;
			rotation.row[13] = 0;
			rotation.row[14] = 0;
			rotation.row[15] = 1;

			return rotation;
		}
		matrix4x4 matrix4x4::create_look_at(cube_face face, const vector3& position)
		{
			switch (face)
			{
				case cube_face::positive_x:
					return matrix4x4::create_look_at(position, position + vector3(1, 0, 0), vector3::up());
				case cube_face::negative_x:
					return matrix4x4::create_look_at(position, position - vector3(1, 0, 0), vector3::up());
				case cube_face::positive_y:
					if (geometric::is_left_handed())
						return matrix4x4::create_look_at(position, position + vector3(0, 1, 0), vector3::backward());
					else
						return matrix4x4::create_look_at(position, position - vector3(0, 1, 0), vector3::forward());
				case cube_face::negative_y:
					if (geometric::is_left_handed())
						return matrix4x4::create_look_at(position, position - vector3(0, 1, 0), vector3::forward());
					else
						return matrix4x4::create_look_at(position, position + vector3(0, 1, 0), vector3::backward());
				case cube_face::positive_z:
					return matrix4x4::create_look_at(position, position + vector3(0, 0, 1), vector3::up());
				case cube_face::negative_z:
					return matrix4x4::create_look_at(position, position - vector3(0, 0, 1), vector3::up());
				default:
					return matrix4x4::identity();
			}
		}

		quaternion::quaternion() noexcept : x(0.0f), y(0.0f), z(0.0f), w(0.0f)
		{
		}
		quaternion::quaternion(float x, float y, float z, float w) noexcept : x(x), y(y), z(z), w(w)
		{
		}
		quaternion::quaternion(const quaternion& in) noexcept : x(in.x), y(in.y), z(in.z), w(in.w)
		{
		}
		quaternion::quaternion(const vector3& axis, float angle) noexcept
		{
			set_axis(axis, angle);
		}
		quaternion::quaternion(const vector3& euler) noexcept
		{
			set_euler(euler);
		}
		quaternion::quaternion(const matrix4x4& value) noexcept
		{
			set_matrix(value);
		}
		void quaternion::set_axis(const vector3& axis, float angle)
		{
#ifdef VI_VECTORCLASS
			LOAD_V3(_r1, axis);
			_r1 *= std::sin(angle / 2);
			_r1.insert(3, std::cos(angle / 2));
			_r1.store((float*)this);
#else
			float sin = std::sin(angle / 2);
			x = axis.x * sin;
			y = axis.y * sin;
			z = axis.z * sin;
			w = std::cos(angle / 2);
#endif
		}
		void quaternion::set_euler(const vector3& v)
		{
#ifdef VI_VECTORCLASS
			float _sx[4], _cx[4];
			LOAD_V3(_r1, v);
			LOAD_VAL(_r2, 0.0f);
			_r1 *= 0.5f;
			_r2 = cos(_r1);
			_r1 = sin(_r1);
			_r1.store(_sx);
			_r2.store(_cx);

			LOAD_AV4(_r3, _sx[0], _cx[0], _sx[0], _cx[0]);
			LOAD_AV4(_r4, _cx[1], _sx[1], _sx[1], _cx[1]);
			LOAD_AV4(_r5, 1.0f, -1.0f, 1.0f, -1.0f);
			_r3 *= _r4;
			_r1 = _r3 * _cx[2];
			_r2 = _r3 * _sx[2];
			_r2 = permute4f<1, 0, 3, 2>(_r2);
			_r1 += _r2 * _r5;
			_r1.store((float*)this);
#else
			float sin_x = std::sin(v.x / 2);
			float cos_x = std::cos(v.x / 2);
			float sin_y = std::sin(v.y / 2);
			float cos_y = std::cos(v.y / 2);
			float sin_z = std::sin(v.z / 2);
			float cos_z = std::cos(v.z / 2);
			x = sin_x * cos_y;
			y = cos_x * sin_y;
			z = sin_x * sin_y;
			w = cos_x * cos_y;

			float fX = x * cos_z + y * sin_z;
			float fY = y * cos_z - x * sin_z;
			float fZ = z * cos_z + w * sin_z;
			float fW = w * cos_z - z * sin_z;
			x = fX;
			y = fY;
			z = fZ;
			w = fW;
#endif
		}
		void quaternion::set_matrix(const matrix4x4& value)
		{
			float t = value.row[0] + value.row[5] + value.row[10];
			if (t > 0.0f)
			{
				float s = std::sqrt(1 + t) * 2.0f;
				x = (value.row[9] - value.row[6]) / s;
				y = (value.row[2] - value.row[8]) / s;
				z = (value.row[4] - value.row[1]) / s;
				w = 0.25f * s;
			}
			else if (value.row[0] > value.row[5] && value.row[0] > value.row[10])
			{
				float s = std::sqrt(1.0f + value.row[0] - value.row[5] - value.row[10]) * 2.0f;
				x = 0.25f * s;
				y = (value.row[4] + value.row[1]) / s;
				z = (value.row[2] + value.row[8]) / s;
				w = (value.row[9] - value.row[6]) / s;
			}
			else if (value.row[5] > value.row[10])
			{
				float s = std::sqrt(1.0f + value.row[5] - value.row[0] - value.row[10]) * 2.0f;
				x = (value.row[4] + value.row[1]) / s;
				y = 0.25f * s;
				z = (value.row[9] + value.row[6]) / s;
				w = (value.row[2] - value.row[8]) / s;
			}
			else
			{
				float s = std::sqrt(1.0f + value.row[10] - value.row[0] - value.row[5]) * 2.0f;
				x = (value.row[2] + value.row[8]) / s;
				y = (value.row[9] + value.row[6]) / s;
				z = 0.25f * s;
				w = (value.row[4] - value.row[1]) / s;
			}
		}
		void quaternion::set(const quaternion& value)
		{
			x = value.x;
			y = value.y;
			z = value.z;
			w = value.w;
		}
		quaternion quaternion::operator *(float r) const
		{
			return mul(r);
		}
		vector3 quaternion::operator *(const vector3& r) const
		{
			return mul(r);
		}
		quaternion quaternion::operator *(const quaternion& r) const
		{
			return mul(r);
		}
		quaternion quaternion::operator -(const quaternion& r) const
		{
			return sub(r);
		}
		quaternion quaternion::operator +(const quaternion& r) const
		{
			return add(r);
		}
		quaternion& quaternion::operator =(const quaternion& r) noexcept
		{
			this->x = r.x;
			this->y = r.y;
			this->z = r.z;
			this->w = r.w;
			return *this;
		}
		quaternion quaternion::normalize() const
		{
#ifdef VI_VECTORCLASS
			LOAD_FV4(_r1);
			_r1 /= geometric::fast_sqrt(horizontal_add(square(_r1)));

			quaternion result;
			_r1.store((float*)&result);
			return result;
#else
			float f = length();
			return quaternion(x / f, y / f, z / f, w / f);
#endif
		}
		quaternion quaternion::snormalize() const
		{
#ifdef VI_VECTORCLASS
			LOAD_FV4(_r1);
			float f = geometric::fast_sqrt(horizontal_add(square(_r1)));
			if (f == 0.0f)
				return quaternion();

			quaternion result;
			_r1 /= f;
			_r1.store((float*)&result);
			return result;
#else
			float f = length();
			if (f == 0.0f)
				return quaternion();

			return quaternion(x / f, y / f, z / f, w / f);
#endif
		}
		quaternion quaternion::conjugate() const
		{
			return quaternion(-x, -y, -z, w);
		}
		quaternion quaternion::mul(float r) const
		{
#ifdef VI_VECTORCLASS
			LOAD_FV4(_r1);
			_r1 *= r;

			quaternion result;
			_r1.store((float*)&result);
			return result;
#else
			return quaternion(x * r, y * r, z * r, w * r);
#endif
		}
		quaternion quaternion::mul(const quaternion& r) const
		{
#ifdef VI_VECTORCLASS
			LOAD_AV4(_r1, w, -x, -y, -z);
			LOAD_AV4(_r2, x, w, y, -z);
			LOAD_AV4(_r3, y, w, z, -x);
			LOAD_AV4(_r4, z, w, x, -y);
			LOAD_AV4(_r5, r.w, r.x, r.y, r.z);
			LOAD_AV4(_r6, r.w, r.x, r.z, r.y);
			LOAD_AV4(_r7, r.w, r.y, r.x, r.z);
			LOAD_AV4(_r8, r.w, r.z, r.y, r.x);
			float W1 = horizontal_add(_r1 * _r5);
			float X1 = horizontal_add(_r2 * _r6);
			float Y1 = horizontal_add(_r3 * _r7);
			float Z1 = horizontal_add(_r4 * _r8);
#else
			float W1 = w * r.w - x * r.x - y * r.y - z * r.z;
			float X1 = x * r.w + w * r.x + y * r.z - z * r.y;
			float Y1 = y * r.w + w * r.y + z * r.x - x * r.z;
			float Z1 = z * r.w + w * r.z + x * r.y - y * r.x;
#endif
			return quaternion(X1, Y1, Z1, W1);
		}
		vector3 quaternion::mul(const vector3& r) const
		{
			vector3 UV0(x, y, z), UV1, UV2;
			UV1 = UV0.cross(r);
			UV2 = UV0.cross(UV1);
			UV1 *= (2.0f * w);
			UV2 *= 2.0f;

			return r + UV1 + UV2;
		}
		quaternion quaternion::sub(const quaternion& r) const
		{
#ifdef VI_VECTORCLASS
			LOAD_FV4(_r1);
			LOAD_V4(_r2, r);
			_r1 -= _r2;

			quaternion result;
			_r1.store((float*)&result);
			return result;
#else
			return quaternion(x - r.x, y - r.y, z - r.z, w - r.w);
#endif
		}
		quaternion quaternion::add(const quaternion& r) const
		{
#ifdef VI_VECTORCLASS
			LOAD_FV4(_r1);
			LOAD_V4(_r2, r);
			_r1 += _r2;

			quaternion result;
			_r1.store((float*)&result);
			return result;
#else
			return quaternion(x + r.x, y + r.y, z + r.z, w + r.w);
#endif
		}
		quaternion quaternion::lerp(const quaternion& b, float delta_time) const
		{
			quaternion correction = b;
			if (dot(b) < 0.0f)
				correction = quaternion(-b.x, -b.y, -b.z, -b.w);

			return (correction - *this) * delta_time + normalize();
		}
		quaternion quaternion::slerp(const quaternion& b, float delta_time) const
		{
			quaternion correction = b;
			float cos = dot(b);

			if (cos < 0.0f)
			{
				correction = quaternion(-b.x, -b.y, -b.z, -b.w);
				cos = -cos;
			}

			if (std::abs(cos) >= 1.0f - 1e3f)
				return lerp(correction, delta_time);

			float sin = geometric::fast_sqrt(1.0f - cos * cos);
			float angle = std::atan2(sin, cos);
			float inved_sin = 1.0f / sin;
			float source = std::sin(angle - delta_time * angle) * inved_sin;
			float destination = std::sin(delta_time * angle) * inved_sin;

			return mul(source).add(correction.mul(destination));
		}
		quaternion quaternion::create_euler_rotation(const vector3& v)
		{
			quaternion result;
			result.set_euler(v);
			return result;
		}
		quaternion quaternion::create_rotation(const matrix4x4& v)
		{
			quaternion result;
			result.set_matrix(v);
			return result;
		}
		vector3 quaternion::forward() const
		{
#ifdef VI_VECTORCLASS
			LOAD_AV4(_r1, x, -w, y, w);
			LOAD_AV4(_r2, z, y, z, x);
			LOAD_AV2(_r3, x, y);
			_r1 *= _r2;
			_r2 = permute4f<-1, -1, 2, 3>(_r1);
			_r1 = permute4f<0, 1, -1, -1>(_r1);
			_r3 = square(_r3);

			vector3 result(horizontal_add(_r1), horizontal_add(_r2), horizontal_add(_r3));
			result *= 2.0f;
			result.z = 1.0f - result.z;
			return result;
#else
			return vector3(
				2.0f * (x * z - w * y),
				2.0f * (y * z + w * x),
				1.0f - 2.0f * (x * x + y * y));
#endif
		}
		vector3 quaternion::up() const
		{
#ifdef VI_VECTORCLASS
			LOAD_AV4(_r1, x, w, y, -w);
			LOAD_AV4(_r2, y, z, z, x);
			LOAD_AV2(_r3, x, z);
			_r1 *= _r2;
			_r2 = permute4f<-1, -1, 2, 3>(_r1);
			_r1 = permute4f<0, 1, -1, -1>(_r1);
			_r3 = square(_r3);

			vector3 result(horizontal_add(_r1), horizontal_add(_r3), horizontal_add(_r2));
			result *= 2.0f;
			result.y = 1.0f - result.y;
			return result;
#else
			return vector3(
				2.0f * (x * y + w * z),
				1.0f - 2.0f * (x * x + z * z),
				2.0f * (y * z - w * x));
#endif
		}
		vector3 quaternion::right() const
		{
#ifdef VI_VECTORCLASS
			LOAD_AV4(_r1, x, -w, x, w);
			LOAD_AV4(_r2, y, z, z, y);
			LOAD_AV2(_r3, y, z);
			_r1 *= _r2;
			_r2 = permute4f<-1, -1, 2, 3>(_r1);
			_r1 = permute4f<0, 1, -1, -1>(_r1);
			_r3 = square(_r3);

			vector3 result(horizontal_add(_r3), horizontal_add(_r1), horizontal_add(_r2));
			result *= 2.0f;
			result.x = 1.0f - result.x;
			return result;
#else
			return vector3(
				1.0f - 2.0f * (y * y + z * z),
				2.0f * (x * y - w * z),
				2.0f * (x * z + w * y));
#endif
		}
		matrix4x4 quaternion::get_matrix() const
		{
			matrix4x4 result =
			{
				1.0f - 2.0f * (y * y + z * z), 2.0f * (x * y + z * w), 2.0f * (x * z - y * w), 0.0f,
				2.0f * (x * y - z * w), 1.0f - 2.0f * (x * x + z * z), 2.0f * (y * z + x * w), 0.0f,
				2.0f * (x * z + y * w), 2.0f * (y * z - x * w), 1.0f - 2.0f * (x * x + y * y), 0.0f,
				0.0f, 0.0f, 0.0f, 1.0f
			};

			return result;
		}
		vector3 quaternion::get_euler() const
		{
#ifdef VI_VECTORCLASS
			LOAD_AV4(_r1, w, y, w, -z);
			LOAD_AV4(_r2, x, z, y, x);
			LOAD_FV3(_r3);
			LOAD_AV2(_r4, w, z);
			LOAD_AV2(_r5, x, y);
			float xyzw[4];
			_r1 *= _r2;
			_r4 *= _r5;
			_r2 = permute4f<-1, -1, 2, 3>(_r1);
			_r1 = permute4f<0, 1, -1, -1>(_r1);
			_r3 = square(_r3);
			_r3.store(xyzw);

			float T0 = +2.0f * horizontal_add(_r1);
			float T1 = +1.0f - 2.0f * (xyzw[0] + xyzw[1]);
			float roll = compute::mathf::atan2(T0, T1);

			float T2 = +2.0f * horizontal_add(_r2);
			T2 = ((T2 > 1.0f) ? 1.0f : T2);
			T2 = ((T2 < -1.0f) ? -1.0f : T2);
			float pitch = compute::mathf::asin(T2);

			float T3 = +2.0f * horizontal_add(_r4);
			float T4 = +1.0f - 2.0f * (xyzw[1] + xyzw[2]);
			float yaw = compute::mathf::atan2(T3, T4);

			return vector3(roll, pitch, yaw);
#else
			float Y2 = y * y;
			float T0 = +2.0f * (w * x + y * z);
			float T1 = +1.0f - 2.0f * (x * x + Y2);
			float roll = compute::mathf::atan2(T0, T1);

			float T2 = +2.0f * (w * y - z * x);
			T2 = ((T2 > 1.0f) ? 1.0f : T2);
			T2 = ((T2 < -1.0f) ? -1.0f : T2);
			float pitch = compute::mathf::asin(T2);

			float T3 = +2.0f * (w * z + x * y);
			float T4 = +1.0f - 2.0f * (Y2 + z * z);
			float yaw = compute::mathf::atan2(T3, T4);

			return vector3(roll, pitch, yaw);
#endif
		}
		float quaternion::dot(const quaternion& r) const
		{
#ifdef VI_VECTORCLASS
			LOAD_FV4(_r1);
			LOAD_V4(_r2, r);
			_r1 *= _r2;
			return horizontal_add(_r1);
#else
			return x * r.x + y * r.y + z * r.z + w * r.w;
#endif
		}
		float quaternion::length() const
		{
#ifdef VI_VECTORCLASS
			LOAD_FV4(_r1);
			return std::sqrt(horizontal_add(square(_r1)));
#else
			return std::sqrt(x * x + y * y + z * z + w * w);
#endif
		}
		bool quaternion::operator ==(const quaternion& v) const
		{
			return x == v.x && y == v.y && z == v.z && w == v.w;
		}
		bool quaternion::operator !=(const quaternion& v) const
		{
			return !(*this == v);
		}

		random_vector2::random_vector2() noexcept : min(0), max(1), intensity(false), accuracy(1)
		{
		}
		random_vector2::random_vector2(const vector2& min_v, const vector2& max_v, bool intensity_v, float accuracy_v) noexcept : min(min_v), max(max_v), intensity(intensity_v), accuracy(accuracy_v)
		{
		}
		vector2 random_vector2::generate()
		{
			vector2 fMin = min * accuracy;
			vector2 fMax = max * accuracy;
			float inv_accuracy = 1.0f / accuracy;
			if (intensity)
				inv_accuracy *= compute::mathf::random();

			return vector2(
				compute::mathf::random(fMin.x, fMax.x),
				compute::mathf::random(fMin.y, fMax.y)) * inv_accuracy;
		}

		random_vector3::random_vector3() noexcept : min(0), max(1), intensity(false), accuracy(1)
		{
		}
		random_vector3::random_vector3(const vector3& min_v, const vector3& max_v, bool intensity_v, float accuracy_v) noexcept : min(min_v), max(max_v), intensity(intensity_v), accuracy(accuracy_v)
		{
		}
		vector3 random_vector3::generate()
		{
			vector3 fMin = min * accuracy;
			vector3 fMax = max * accuracy;
			float inv_accuracy = 1.0f / accuracy;
			if (intensity)
				inv_accuracy *= compute::mathf::random();

			return vector3(
				compute::mathf::random(fMin.x, fMax.x),
				compute::mathf::random(fMin.y, fMax.y),
				compute::mathf::random(fMin.z, fMax.z)) * inv_accuracy;
		}

		random_vector4::random_vector4() noexcept : min(0), max(1), intensity(false), accuracy(1)
		{
		}
		random_vector4::random_vector4(const vector4& min_v, const vector4& max_v, bool intensity_v, float accuracy_v) noexcept : min(min_v), max(max_v), intensity(intensity_v), accuracy(accuracy_v)
		{
		}
		vector4 random_vector4::generate()
		{
			vector4 fMin = min * accuracy;
			vector4 fMax = max * accuracy;
			float inv_accuracy = 1.0f / accuracy;
			if (intensity)
				inv_accuracy *= compute::mathf::random();

			return vector4(
				compute::mathf::random(fMin.x, fMax.x),
				compute::mathf::random(fMin.y, fMax.y),
				compute::mathf::random(fMin.z, fMax.z),
				compute::mathf::random(fMin.w, fMax.w)) * inv_accuracy;
		}

		random_float::random_float() noexcept : min(0), max(1), intensity(false), accuracy(1)
		{
		}
		random_float::random_float(float min_v, float max_v, bool intensity_v, float accuracy_v) noexcept : min(min_v), max(max_v), intensity(intensity_v), accuracy(accuracy_v)
		{
		}
		float random_float::generate()
		{
			return (compute::mathf::random(min * accuracy, max * accuracy) / accuracy) * (intensity ? compute::mathf::random() : 1);
		}

		uint8_t adj_triangle::find_edge(uint32_t vref0, uint32_t vref1)
		{
			uint8_t edge_nb = 0xff;
			if (vref[0] == vref0 && vref[1] == vref1)
				edge_nb = 0;
			else if (vref[0] == vref1 && vref[1] == vref0)
				edge_nb = 0;
			else if (vref[0] == vref0 && vref[2] == vref1)
				edge_nb = 1;
			else if (vref[0] == vref1 && vref[2] == vref0)
				edge_nb = 1;
			else if (vref[1] == vref0 && vref[2] == vref1)
				edge_nb = 2;
			else if (vref[1] == vref1 && vref[2] == vref0)
				edge_nb = 2;

			return edge_nb;
		}
		uint32_t adj_triangle::opposite_vertex(uint32_t vref0, uint32_t vref1)
		{
			uint32_t ref = 0xffffffff;
			if (vref[0] == vref0 && vref[1] == vref1)
				ref = vref[2];
			else if (vref[0] == vref1 && vref[1] == vref0)
				ref = vref[2];
			else if (vref[0] == vref0 && vref[2] == vref1)
				ref = vref[1];
			else if (vref[0] == vref1 && vref[2] == vref0)
				ref = vref[1];
			else if (vref[1] == vref0 && vref[2] == vref1)
				ref = vref[0];
			else if (vref[1] == vref1 && vref[2] == vref0)
				ref = vref[0];

			return ref;
		}

		adjacencies::adjacencies() noexcept : nb_edges(0), current_nb_faces(0), edges(nullptr), nb_faces(0), faces(nullptr)
		{
		}
		adjacencies::~adjacencies() noexcept
		{
			core::memory::deallocate(faces);
			core::memory::deallocate(edges);
		}
		bool adjacencies::fill(adjacencies::desc& create)
		{
			nb_faces = create.nb_faces;
			faces = core::memory::allocate<adj_triangle>(sizeof(adj_triangle) * nb_faces);
			edges = core::memory::allocate<adj_edge>(sizeof(adj_edge) * nb_faces * 3);
			for (uint32_t i = 0; i < nb_faces; i++)
			{
				uint32_t ref0 = create.faces[i * 3 + 0];
				uint32_t ref1 = create.faces[i * 3 + 1];
				uint32_t ref2 = create.faces[i * 3 + 2];
				add_triangle(ref0, ref1, ref2);
			}

			return true;
		}
		bool adjacencies::resolve()
		{
			radix_sorter core;
			uint32_t* face_nb = core::memory::allocate<uint32_t>(sizeof(uint32_t) * nb_edges);
			uint32_t* vrefs0 = core::memory::allocate<uint32_t>(sizeof(uint32_t) * nb_edges);
			uint32_t* vrefs1 = core::memory::allocate<uint32_t>(sizeof(uint32_t) * nb_edges);
			for (uint32_t i = 0; i < nb_edges; i++)
			{
				face_nb[i] = edges[i].face_nb;
				vrefs0[i] = edges[i].ref0;
				vrefs1[i] = edges[i].ref1;
			}

			uint32_t* sorted = core.sort(face_nb, nb_edges).sort(vrefs0, nb_edges).sort(vrefs1, nb_edges).get_indices();
			uint32_t last_ref0 = vrefs0[sorted[0]];
			uint32_t last_ref1 = vrefs1[sorted[0]];
			uint32_t count = 0;
			uint32_t tmp_buffer[3];

			for (uint32_t i = 0; i < nb_edges; i++)
			{
				uint32_t face = face_nb[sorted[i]];
				uint32_t ref0 = vrefs0[sorted[i]];
				uint32_t ref1 = vrefs1[sorted[i]];

				if (ref0 == last_ref0 && ref1 == last_ref1)
				{
					tmp_buffer[count++] = face;
					if (count == 3)
					{
						core::memory::deallocate(face_nb);
						core::memory::deallocate(vrefs0);
						core::memory::deallocate(vrefs1);
						return false;
					}
				}
				else
				{
					if (count == 2)
					{
						bool status = update_link(tmp_buffer[0], tmp_buffer[1], last_ref0, last_ref1);
						if (!status)
						{
							core::memory::deallocate(face_nb);
							core::memory::deallocate(vrefs0);
							core::memory::deallocate(vrefs1);
							return status;
						}
					}

					count = 0;
					tmp_buffer[count++] = face;
					last_ref0 = ref0;
					last_ref1 = ref1;
				}
			}

			bool status = true;
			if (count == 2)
				status = update_link(tmp_buffer[0], tmp_buffer[1], last_ref0, last_ref1);

			core::memory::deallocate(face_nb);
			core::memory::deallocate(vrefs0);
			core::memory::deallocate(vrefs1);
			core::memory::deallocate(edges);

			return status;
		}
		bool adjacencies::add_triangle(uint32_t ref0, uint32_t ref1, uint32_t ref2)
		{
			faces[current_nb_faces].vref[0] = ref0;
			faces[current_nb_faces].vref[1] = ref1;
			faces[current_nb_faces].vref[2] = ref2;
			faces[current_nb_faces].atri[0] = -1;
			faces[current_nb_faces].atri[1] = -1;
			faces[current_nb_faces].atri[2] = -1;

			if (ref0 < ref1)
				add_edge(ref0, ref1, current_nb_faces);
			else
				add_edge(ref1, ref0, current_nb_faces);

			if (ref0 < ref2)
				add_edge(ref0, ref2, current_nb_faces);
			else
				add_edge(ref2, ref0, current_nb_faces);

			if (ref1 < ref2)
				add_edge(ref1, ref2, current_nb_faces);
			else
				add_edge(ref2, ref1, current_nb_faces);

			current_nb_faces++;
			return true;
		}
		bool adjacencies::add_edge(uint32_t ref0, uint32_t ref1, uint32_t face)
		{
			edges[nb_edges].ref0 = ref0;
			edges[nb_edges].ref1 = ref1;
			edges[nb_edges].face_nb = face;
			nb_edges++;

			return true;
		}
		bool adjacencies::update_link(uint32_t firsttri, uint32_t secondtri, uint32_t ref0, uint32_t ref1)
		{
			adj_triangle* tri0 = &faces[firsttri];
			adj_triangle* tri1 = &faces[secondtri];
			uint8_t edge_nb0 = tri0->find_edge(ref0, ref1);
			if (edge_nb0 == 0xff)
				return false;

			uint8_t edge_nb1 = tri1->find_edge(ref0, ref1);
			if (edge_nb1 == 0xff)
				return false;

			tri0->atri[edge_nb0] = secondtri | ((uint32_t)edge_nb1 << 30);
			tri1->atri[edge_nb1] = firsttri | ((uint32_t)edge_nb0 << 30);

			return true;
		}

		triangle_strip::triangle_strip() noexcept : adj(nullptr), tags(nullptr), nb_strips(0), total_length(0), one_sided(false), sgi_cipher(false), connect_all_strips(false)
		{
		}
		triangle_strip::~triangle_strip() noexcept
		{
			free_buffers();
		}
		triangle_strip& triangle_strip::free_buffers()
		{
			core::vector<uint32_t>().swap(single_strip);
			core::vector<uint32_t>().swap(strip_runs);
			core::vector<uint32_t>().swap(strip_lengths);
			core::memory::deallocate(tags);
			tags = nullptr;

			core::memory::deinit(adj);
			return *this;
		}
		bool triangle_strip::fill(const triangle_strip::desc& create)
		{
			adjacencies::desc ac;
			ac.nb_faces = create.nb_faces;
			ac.faces = create.faces;
			free_buffers();

			adj = core::memory::init<adjacencies>();
			if (!adj->fill(ac))
			{
				core::memory::deinit(adj);
				adj = nullptr;
				return false;
			}

			if (!adj->resolve())
			{
				core::memory::deinit(adj);
				adj = nullptr;
				return false;
			}

			one_sided = create.one_sided;
			sgi_cipher = create.sgi_cipher;
			connect_all_strips = create.connect_all_strips;

			return true;
		}
		bool triangle_strip::resolve(triangle_strip::result& result)
		{
			VI_ASSERT(adj != nullptr, "triangle strip should be initialized");
			tags = core::memory::allocate<bool>(sizeof(bool) * adj->nb_faces);
			uint32_t* connectivity = core::memory::allocate<uint32_t>(sizeof(uint32_t) * adj->nb_faces);

			memset(tags, 0, adj->nb_faces * sizeof(bool));
			memset(connectivity, 0, adj->nb_faces * sizeof(uint32_t));

			if (sgi_cipher)
			{
				for (uint32_t i = 0; i < adj->nb_faces; i++)
				{
					adj_triangle* tri = &adj->faces[i];
					if (!IS_BOUNDARY(tri->atri[0]))
						connectivity[i]++;

					if (!IS_BOUNDARY(tri->atri[1]))
						connectivity[i]++;

					if (!IS_BOUNDARY(tri->atri[2]))
						connectivity[i]++;
				}

				radix_sorter RS;
				uint32_t* sorted = RS.sort(connectivity, adj->nb_faces).get_indices();
				memcpy(connectivity, sorted, adj->nb_faces * sizeof(uint32_t));
			}
			else
			{
				for (uint32_t i = 0; i < adj->nb_faces; i++)
					connectivity[i] = i;
			}

			nb_strips = 0;
			uint32_t total_nb_faces = 0;
			uint32_t index = 0;

			while (total_nb_faces != adj->nb_faces)
			{
				while (tags[connectivity[index]])
					index++;

				uint32_t first_face = connectivity[index];
				total_nb_faces += compute_strip(first_face);
				nb_strips++;
			}

			core::memory::deallocate(connectivity);
			core::memory::deallocate(tags);
			result.groups = strip_lengths;
			result.strips = strip_runs;

			if (connect_all_strips)
				connect_strips(result);

			return true;
		}
		uint32_t triangle_strip::compute_strip(uint32_t face)
		{
			uint32_t* strip[3];
			uint32_t* faces[3];
			uint32_t length[3];
			uint32_t first_length[3];
			uint32_t refs0[3];
			uint32_t refs1[3];

			refs0[0] = adj->faces[face].vref[0];
			refs1[0] = adj->faces[face].vref[1];

			refs0[1] = adj->faces[face].vref[2];
			refs1[1] = adj->faces[face].vref[0];

			refs0[2] = adj->faces[face].vref[1];
			refs1[2] = adj->faces[face].vref[2];

			for (uint32_t j = 0; j < 3; j++)
			{
				strip[j] = core::memory::allocate<uint32_t>(sizeof(uint32_t) * (adj->nb_faces + 2 + 1 + 2));
				faces[j] = core::memory::allocate<uint32_t>(sizeof(uint32_t) * (adj->nb_faces + 2));
				memset(strip[j], 0xff, (adj->nb_faces + 2 + 1 + 2) * sizeof(uint32_t));
				memset(faces[j], 0xff, (adj->nb_faces + 2) * sizeof(uint32_t));

				bool* vTags = core::memory::allocate<bool>(sizeof(bool) * adj->nb_faces);
				memcpy(vTags, tags, adj->nb_faces * sizeof(bool));

				length[j] = track_strip(face, refs0[j], refs1[j], &strip[j][0], &faces[j][0], vTags);
				first_length[j] = length[j];

				for (uint32_t i = 0; i < length[j] / 2; i++)
				{
					strip[j][i] ^= strip[j][length[j] - i - 1];
					strip[j][length[j] - i - 1] ^= strip[j][i];
					strip[j][i] ^= strip[j][length[j] - i - 1];
				}

				for (uint32_t i = 0; i < (length[j] - 2) / 2; i++)
				{
					faces[j][i] ^= faces[j][length[j] - i - 3];
					faces[j][length[j] - i - 3] ^= faces[j][i];
					faces[j][i] ^= faces[j][length[j] - i - 3];
				}

				uint32_t new_ref0 = strip[j][length[j] - 3];
				uint32_t new_ref1 = strip[j][length[j] - 2];
				uint32_t extra_length = track_strip(face, new_ref0, new_ref1, &strip[j][length[j] - 3], &faces[j][length[j] - 3], vTags);
				length[j] += extra_length - 3;
				core::memory::deallocate(vTags);
			}

			uint32_t longest = length[0];
			uint32_t best = 0;
			if (length[1] > longest)
			{
				longest = length[1];
				best = 1;
			}

			if (length[2] > longest)
			{
				longest = length[2];
				best = 2;
			}

			uint32_t nb_faces = longest - 2;
			for (uint32_t j = 0; j < longest - 2; j++)
				tags[faces[best][j]] = true;

			if (one_sided && first_length[best] & 1)
			{
				if (longest == 3 || longest == 4)
				{
					strip[best][1] ^= strip[best][2];
					strip[best][2] ^= strip[best][1];
					strip[best][1] ^= strip[best][2];
				}
				else
				{
					for (uint32_t j = 0; j < longest / 2; j++)
					{
						strip[best][j] ^= strip[best][longest - j - 1];
						strip[best][longest - j - 1] ^= strip[best][j];
						strip[best][j] ^= strip[best][longest - j - 1];
					}

					uint32_t new_pos = longest - first_length[best];
					if (new_pos & 1)
					{
						for (uint32_t j = 0; j < longest; j++)
							strip[best][longest - j] = strip[best][longest - j - 1];
						longest++;
					}
				}
			}

			for (uint32_t j = 0; j < longest; j++)
			{
				uint32_t ref = strip[best][j];
				strip_runs.push_back(ref);
			}

			strip_lengths.push_back(longest);
			for (uint32_t j = 0; j < 3; j++)
			{
				core::memory::deallocate(faces[j]);
				core::memory::deallocate(strip[j]);
			}

			return nb_faces;
		}
		uint32_t triangle_strip::track_strip(uint32_t face, uint32_t oldest, uint32_t middle, uint32_t* strip, uint32_t* faces, bool* tags)
		{
			uint32_t length = 2;
			strip[0] = oldest;
			strip[1] = middle;

			bool do_the_strip = true;
			while (do_the_strip)
			{
				uint32_t newest = adj->faces[face].opposite_vertex(oldest, middle);
				strip[length++] = newest;
				*faces++ = face;
				tags[face] = true;

				uint8_t cur_edge = adj->faces[face].find_edge(middle, newest);
				if (!IS_BOUNDARY(cur_edge))
				{
					uint32_t link = adj->faces[face].atri[cur_edge];
					face = MAKE_ADJ_TRI(link);
					if (tags[face])
						do_the_strip = false;
				}
				else
					do_the_strip = false;

				oldest = middle;
				middle = newest;
			}

			return length;
		}
		bool triangle_strip::connect_strips(triangle_strip::result& result)
		{
			uint32_t* drefs = (uint32_t*)result.strips.data();
			single_strip.clear();
			total_length = 0;

			for (uint32_t k = 0; k < result.groups.size(); k++)
			{
				if (k)
				{
					uint32_t last_ref = drefs[-1];
					uint32_t first_ref = drefs[0];
					single_strip.push_back(last_ref);
					single_strip.push_back(first_ref);
					total_length += 2;

					if (one_sided && total_length & 1)
					{
						uint32_t second_ref = drefs[1];
						if (first_ref != second_ref)
						{
							single_strip.push_back(first_ref);
							total_length++;
						}
						else
						{
							result.groups[k]--;
							drefs++;
						}
					}
				}

				for (uint32_t j = 0; j < result.groups[k]; j++)
				{
					uint32_t ref = drefs[j];
					single_strip.push_back(ref);
				}

				drefs += result.groups[k];
				total_length += result.groups[k];
			}

			result.strips = single_strip;
			result.groups = core::vector<uint32_t>({ total_length });

			return true;
		}

		core::vector<int> triangle_strip::result::get_indices(int group)
		{
			core::vector<int> indices;
			if (group < 0)
			{
				indices.reserve(strips.size());
				for (auto& index : strips)
					indices.push_back(index);

				return indices;
			}
			else if (group < (int)groups.size())
			{
				size_t size = groups[group];
				indices.reserve(size);

				size_t off = 0, idx = 0;
				while (off != group)
					idx += groups[off++];

				size += idx;
				for (size_t i = idx; i < size; i++)
					indices.push_back(strips[i]);

				return indices;
			}

			return indices;
		}
		core::vector<int> triangle_strip::result::get_inv_indices(int group)
		{
			core::vector<int> indices = get_indices(group);
			std::reverse(indices.begin(), indices.end());

			return indices;
		}

		radix_sorter::radix_sorter() noexcept : current_size(0), indices(nullptr), indices2(nullptr)
		{
			histogram = core::memory::allocate<uint32_t>(sizeof(uint32_t) * 256 * 4);
			offset = core::memory::allocate<uint32_t>(sizeof(uint32_t) * 256);
			reset_indices();
		}
		radix_sorter::radix_sorter(const radix_sorter& other) noexcept : current_size(0), indices(nullptr), indices2(nullptr)
		{
			histogram = core::memory::allocate<uint32_t>(sizeof(uint32_t) * 256 * 4);
			offset = core::memory::allocate<uint32_t>(sizeof(uint32_t) * 256);
			reset_indices();
		}
		radix_sorter::radix_sorter(radix_sorter&& other) noexcept : histogram(other.histogram), offset(other.offset), current_size(other.current_size), indices(other.indices), indices2(other.indices2)
		{
			other.indices = nullptr;
			other.indices2 = nullptr;
			other.current_size = 0;
			other.histogram = nullptr;
			other.offset = nullptr;
		}
		radix_sorter::~radix_sorter() noexcept
		{
			core::memory::deallocate(offset);
			core::memory::deallocate(histogram);
			core::memory::deallocate(indices2);
			core::memory::deallocate(indices);
		}
		radix_sorter& radix_sorter::operator =(const radix_sorter& v)
		{
			core::memory::deallocate(histogram);
			histogram = core::memory::allocate<uint32_t>(sizeof(uint32_t) * 256 * 4);

			core::memory::deallocate(offset);
			offset = core::memory::allocate<uint32_t>(sizeof(uint32_t) * 256);
			reset_indices();

			return *this;
		}
		radix_sorter& radix_sorter::operator =(radix_sorter&& other) noexcept
		{
			core::memory::deallocate(offset);
			core::memory::deallocate(histogram);
			core::memory::deallocate(indices2);
			core::memory::deallocate(indices);
			indices = other.indices;
			indices2 = other.indices2;
			current_size = other.current_size;
			histogram = other.histogram;
			offset = other.offset;
			other.indices = nullptr;
			other.indices2 = nullptr;
			other.current_size = 0;
			other.histogram = nullptr;
			other.offset = nullptr;

			return *this;
		}
		radix_sorter& radix_sorter::sort(uint32_t* input, uint32_t nb, bool signedvalues)
		{
			if (nb > current_size)
			{
				core::memory::deallocate(indices2);
				core::memory::deallocate(indices);
				indices = core::memory::allocate<uint32_t>(sizeof(uint32_t) * nb);
				indices2 = core::memory::allocate<uint32_t>(sizeof(uint32_t) * nb);
				current_size = nb;
				reset_indices();
			}

			memset(histogram, 0, 256 * 4 * sizeof(uint32_t));

			bool already_sorted = true;
			uint32_t* vIndices = indices;
			uint8_t* p = (uint8_t*)input;
			uint8_t* pe = &p[nb * 4];
			uint32_t* h0 = &histogram[0];
			uint32_t* h1 = &histogram[256];
			uint32_t* h2 = &histogram[512];
			uint32_t* h3 = &histogram[768];

			if (!signedvalues)
			{
				uint32_t prev_val = input[indices[0]];
				while (p != pe)
				{
					uint32_t val = input[*vIndices++];
					if (val < prev_val)
						already_sorted = false;

					prev_val = val;
					h0[*p++]++;
					h1[*p++]++;
					h2[*p++]++;
					h3[*p++]++;
				}
			}
			else
			{
				signed int prev_val = (signed int)input[indices[0]];
				while (p != pe)
				{
					signed int val = (signed int)input[*vIndices++];
					if (val < prev_val)
						already_sorted = false;

					prev_val = val;
					h0[*p++]++;
					h1[*p++]++;
					h2[*p++]++;
					h3[*p++]++;
				}
			}

			if (already_sorted)
				return *this;

			uint32_t nb_negative_values = 0;
			if (signedvalues)
			{
				uint32_t* h4 = &histogram[768];
				for (uint32_t i = 128; i < 256; i++)
					nb_negative_values += h4[i];
			}

			for (uint32_t j = 0; j < 4; j++)
			{
				uint32_t* cur_count = &histogram[j << 8];
				bool perform_pass = true;

				for (uint32_t i = 0; i < 256; i++)
				{
					if (cur_count[i] == nb)
					{
						perform_pass = false;
						break;
					}

					if (cur_count[i])
						break;
				}

				if (perform_pass)
				{
					if (j != 3 || !signedvalues)
					{
						offset[0] = 0;
						for (uint32_t i = 1; i < 256; i++)
							offset[i] = offset[i - 1] + cur_count[i - 1];
					}
					else
					{
						offset[0] = nb_negative_values;
						for (uint32_t i = 1; i < 128; i++)
							offset[i] = offset[i - 1] + cur_count[i - 1];

						offset[128] = 0;
						for (uint32_t i = 129; i < 256; i++)
							offset[i] = offset[i - 1] + cur_count[i - 1];
					}

					uint8_t* input_bytes = (uint8_t*)input;
					uint32_t* indices_start = indices;
					uint32_t* indices_end = &indices[nb];
					input_bytes += j;

					while (indices_start != indices_end)
					{
						uint32_t id = *indices_start++;
						indices2[offset[input_bytes[id << 2]]++] = id;
					}

					uint32_t* tmp = indices;
					indices = indices2;
					indices2 = tmp;
				}
			}

			return *this;
		}
		radix_sorter& radix_sorter::sort(float* input2, uint32_t nb)
		{
			uint32_t* input = (uint32_t*)input2;
			if (nb > current_size)
			{
				core::memory::deallocate(indices2);
				core::memory::deallocate(indices);
				indices = core::memory::allocate<uint32_t>(sizeof(uint32_t) * nb);
				indices2 = core::memory::allocate<uint32_t>(sizeof(uint32_t) * nb);
				current_size = nb;
				reset_indices();
			}

			memset(histogram, 0, 256 * 4 * sizeof(uint32_t));
			{
				float prev_val = input2[indices[0]];
				bool already_sorted = true;
				uint32_t* vIndices = indices;
				uint8_t* p = (uint8_t*)input;
				uint8_t* pe = &p[nb * 4];
				uint32_t* h0 = &histogram[0];
				uint32_t* h1 = &histogram[256];
				uint32_t* h2 = &histogram[512];
				uint32_t* h3 = &histogram[768];

				while (p != pe)
				{
					float val = input2[*vIndices++];
					if (val < prev_val)
						already_sorted = false;

					prev_val = val;
					h0[*p++]++;
					h1[*p++]++;
					h2[*p++]++;
					h3[*p++]++;
				}

				if (already_sorted)
					return *this;
			}

			uint32_t nb_negative_values = 0;
			uint32_t* h3 = &histogram[768];
			for (uint32_t i = 128; i < 256; i++)
				nb_negative_values += h3[i];

			for (uint32_t j = 0; j < 4; j++)
			{
				uint32_t* cur_count = &histogram[j << 8];
				bool perform_pass = true;

				for (uint32_t i = 0; i < 256; i++)
				{
					if (cur_count[i] == nb)
					{
						perform_pass = false;
						break;
					}

					if (cur_count[i])
						break;
				}

				if (perform_pass)
				{
					if (j != 3)
					{
						offset[0] = 0;
						for (uint32_t i = 1; i < 256; i++)
							offset[i] = offset[i - 1] + cur_count[i - 1];

						uint8_t* input_bytes = (uint8_t*)input;
						uint32_t* indices_start = indices;
						uint32_t* indices_end = &indices[nb];
						input_bytes += j;

						while (indices_start != indices_end)
						{
							uint32_t id = *indices_start++;
							indices2[offset[input_bytes[id << 2]]++] = id;
						}
					}
					else
					{
						offset[0] = nb_negative_values;
						for (uint32_t i = 1; i < 128; i++)
							offset[i] = offset[i - 1] + cur_count[i - 1];

						offset[255] = 0;
						for (uint32_t i = 0; i < 127; i++)
							offset[254 - i] = offset[255 - i] + cur_count[255 - i];

						for (uint32_t i = 128; i < 256; i++)
							offset[i] += cur_count[i];

						for (uint32_t i = 0; i < nb; i++)
						{
							uint32_t radix = input[indices[i]] >> 24;
							if (radix < 128)
								indices2[offset[radix]++] = indices[i];
							else
								indices2[--offset[radix]] = indices[i];
						}
					}

					uint32_t* tmp = indices;
					indices = indices2;
					indices2 = tmp;
				}
			}

			return *this;
		}
		radix_sorter& radix_sorter::reset_indices()
		{
			for (uint32_t i = 0; i < current_size; i++)
				indices[i] = i;

			return *this;
		}
		uint32_t* radix_sorter::get_indices()
		{
			return indices;
		}

		bool geometric::is_cube_in_frustum(const matrix4x4& WVP, float radius)
		{
			radius = -radius;
#ifdef VI_VECTORCLASS
			LOAD_AV4(_r1, WVP.row[3], WVP.row[7], WVP.row[11], WVP.row[15]);
			LOAD_AV4(_r2, WVP.row[0], WVP.row[4], WVP.row[8], WVP.row[12]);
			LOAD_VAL(_r3, _r1 + _r2);
			float f = _r3.extract(3); _r3.cutoff(3);
			f /= geometric::fast_sqrt(horizontal_add(square(_r3)));
			if (f <= radius)
				return false;

			_r3 = _r1 - _r2;
			f = _r3.extract(3); _r3.cutoff(3);
			f /= geometric::fast_sqrt(horizontal_add(square(_r3)));
			if (f <= radius)
				return false;

			_r2 = Vec4f(WVP.row[1], WVP.row[5], WVP.row[9], WVP.row[13]);
			_r3 = _r1 + _r2;
			f = _r3.extract(3); _r3.cutoff(3);
			f /= geometric::fast_sqrt(horizontal_add(square(_r3)));
			if (f <= radius)
				return false;

			_r3 = _r1 - _r2;
			f = _r3.extract(3); _r3.cutoff(3);
			f /= geometric::fast_sqrt(horizontal_add(square(_r3)));
			if (f <= radius)
				return false;

			_r2 = Vec4f(WVP.row[2], WVP.row[6], WVP.row[10], WVP.row[14]);
			_r3 = _r1 + _r2;
			f = _r3.extract(3); _r3.cutoff(3);
			f /= geometric::fast_sqrt(horizontal_add(square(_r3)));
			if (f <= radius)
				return false;

			_r2 = Vec4f(WVP.row[2], WVP.row[6], WVP.row[10], WVP.row[14]);
			_r3 = _r1 - _r2;
			f = _r3.extract(3); _r3.cutoff(3);
			f /= geometric::fast_sqrt(horizontal_add(square(_r3)));
			if (f <= radius)
				return false;
#else
			float plane[4];
			plane[0] = WVP.row[3] + WVP.row[0];
			plane[1] = WVP.row[7] + WVP.row[4];
			plane[2] = WVP.row[11] + WVP.row[8];
			plane[3] = WVP.row[15] + WVP.row[12];

			plane[3] /= geometric::fast_sqrt(plane[0] * plane[0] + plane[1] * plane[1] + plane[2] * plane[2]);
			if (plane[3] <= radius)
				return false;

			plane[0] = WVP.row[3] - WVP.row[0];
			plane[1] = WVP.row[7] - WVP.row[4];
			plane[2] = WVP.row[11] - WVP.row[8];
			plane[3] = WVP.row[15] - WVP.row[12];

			plane[3] /= geometric::fast_sqrt(plane[0] * plane[0] + plane[1] * plane[1] + plane[2] * plane[2]);
			if (plane[3] <= radius)
				return false;

			plane[0] = WVP.row[3] + WVP.row[1];
			plane[1] = WVP.row[7] + WVP.row[5];
			plane[2] = WVP.row[11] + WVP.row[9];
			plane[3] = WVP.row[15] + WVP.row[13];

			plane[3] /= geometric::fast_sqrt(plane[0] * plane[0] + plane[1] * plane[1] + plane[2] * plane[2]);
			if (plane[3] <= radius)
				return false;

			plane[0] = WVP.row[3] - WVP.row[1];
			plane[1] = WVP.row[7] - WVP.row[5];
			plane[2] = WVP.row[11] - WVP.row[9];
			plane[3] = WVP.row[15] - WVP.row[13];

			plane[3] /= geometric::fast_sqrt(plane[0] * plane[0] + plane[1] * plane[1] + plane[2] * plane[2]);
			if (plane[3] <= radius)
				return false;

			plane[0] = WVP.row[3] + WVP.row[2];
			plane[1] = WVP.row[7] + WVP.row[6];
			plane[2] = WVP.row[11] + WVP.row[10];
			plane[3] = WVP.row[15] + WVP.row[14];

			plane[3] /= geometric::fast_sqrt(plane[0] * plane[0] + plane[1] * plane[1] + plane[2] * plane[2]);
			if (plane[3] <= radius)
				return false;

			plane[0] = WVP.row[3] - WVP.row[2];
			plane[1] = WVP.row[7] - WVP.row[6];
			plane[2] = WVP.row[11] - WVP.row[10];
			plane[3] = WVP.row[15] - WVP.row[14];

			plane[3] /= geometric::fast_sqrt(plane[0] * plane[0] + plane[1] * plane[1] + plane[2] * plane[2]);
			if (plane[3] <= radius)
				return false;
#endif
			return true;
		}
		bool geometric::is_left_handed()
		{
			return left_handed;
		}
		bool geometric::has_sphere_intersected(const vector3& position_r0, float radius_r0, const vector3& position_r1, float radius_r1)
		{
			if (position_r0.distance(position_r1) < radius_r0 + radius_r1)
				return true;

			return false;
		}
		bool geometric::has_line_intersected(float distance0, float distance1, const vector3& point0, const vector3& point1, vector3& hit)
		{
			if ((distance0 * distance1) >= 0)
				return false;

			if (distance0 == distance1)
				return false;

			hit = point0 + (point1 - point0) * (-distance0 / (distance1 - distance0));
			return true;
		}
		bool geometric::has_line_intersected_cube(const vector3& min, const vector3& max, const vector3& start, const vector3& end)
		{
			if (end.x < min.x && start.x < min.x)
				return false;

			if (end.x > max.x && start.x > max.x)
				return false;

			if (end.y < min.y && start.y < min.y)
				return false;

			if (end.y > max.y && start.y > max.y)
				return false;

			if (end.z < min.z && start.z < min.z)
				return false;

			if (end.z > max.z && start.z > max.z)
				return false;

			if (start.x > min.x && start.x < max.x && start.y > min.y && start.y < max.y && start.z > min.z && start.z < max.z)
				return true;

			vector3 last_hit;
			if ((has_line_intersected(start.x - min.x, end.x - min.x, start, end, last_hit) && has_point_intersected_cube(last_hit, min, max, 1)) || (has_line_intersected(start.y - min.y, end.y - min.y, start, end, last_hit) && has_point_intersected_cube(last_hit, min, max, 2)) || (has_line_intersected(start.z - min.z, end.z - min.z, start, end, last_hit) && has_point_intersected_cube(last_hit, min, max, 3)) || (has_line_intersected(start.x - max.x, end.x - max.x, start, end, last_hit) && has_point_intersected_cube(last_hit, min, max, 1)) || (has_line_intersected(start.y - max.y, end.y - max.y, start, end, last_hit) && has_point_intersected_cube(last_hit, min, max, 2)) || (has_line_intersected(start.z - max.z, end.z - max.z, start, end, last_hit) && has_point_intersected_cube(last_hit, min, max, 3)))
				return true;

			return false;
		}
		bool geometric::has_point_intersected_cube(const vector3& last_hit, const vector3& min, const vector3& max, int axis)
		{
			if (axis == 1 && last_hit.z > min.z && last_hit.z < max.z && last_hit.y > min.y && last_hit.y < max.y)
				return true;

			if (axis == 2 && last_hit.z > min.z && last_hit.z < max.z && last_hit.x > min.x && last_hit.x < max.x)
				return true;

			if (axis == 3 && last_hit.x > min.x && last_hit.x < max.x && last_hit.y > min.y && last_hit.y < max.y)
				return true;

			return false;
		}
		bool geometric::has_point_intersected_cube(const vector3& position, const vector3& scale, const vector3& P0)
		{
			return (P0.x) <= (position.x + scale.x) && (position.x - scale.x) <= (P0.x) && (P0.y) <= (position.y + scale.y) && (position.y - scale.y) <= (P0.y) && (P0.z) <= (position.z + scale.z) && (position.z - scale.z) <= (P0.z);
		}
		bool geometric::has_point_intersected_rectangle(const vector3& position, const vector3& scale, const vector3& P0)
		{
			return P0.x >= position.x - scale.x && P0.x < position.x + scale.x && P0.y >= position.y - scale.y && P0.y < position.y + scale.y;
		}
		bool geometric::has_sb_intersected(transform* R0, transform* R1)
		{
			if (!has_sphere_intersected(R0->get_position(), R0->get_scale().hypotenuse(), R1->get_position(), R1->get_scale().hypotenuse()))
				return false;

			return true;
		}
		bool geometric::has_obb_intersected(transform* R0, transform* R1)
		{
			const vector3& rotation0 = R0->get_rotation();
			const vector3& rotation1 = R1->get_rotation();
			if (rotation0 == 0.0f && rotation1 == 0.0f)
				return has_aabb_intersected(R0, R1);

			const vector3& position0 = R0->get_position();
			const vector3& position1 = R1->get_position();
			const vector3& scale0 = R0->get_scale();
			const vector3& scale1 = R1->get_scale();
			matrix4x4 temp0 = matrix4x4::create(position0 - position1, scale0, rotation0) * matrix4x4::create_rotation(rotation1).inv();
			if (has_line_intersected_cube(-scale1, scale1, vector4(1, 1, 1, 1).transform(temp0.row).xyz(), vector4(-1, -1, -1, 1).transform(temp0.row).xyz()))
				return true;

			if (has_line_intersected_cube(-scale1, scale1, vector4(1, -1, 1, 1).transform(temp0.row).xyz(), vector4(-1, 1, -1, 1).transform(temp0.row).xyz()))
				return true;

			if (has_line_intersected_cube(-scale1, scale1, vector4(-1, -1, 1, 1).transform(temp0.row).xyz(), vector4(1, 1, -1, 1).transform(temp0.row).xyz()))
				return true;

			if (has_line_intersected_cube(-scale1, scale1, vector4(-1, 1, 1, 1).transform(temp0.row).xyz(), vector4(1, -1, -1, 1).transform(temp0.row).xyz()))
				return true;

			if (has_line_intersected_cube(-scale1, scale1, vector4(0, 1, 0, 1).transform(temp0.row).xyz(), vector4(0, -1, 0, 1).transform(temp0.row).xyz()))
				return true;

			if (has_line_intersected_cube(-scale1, scale1, vector4(1, 0, 0, 1).transform(temp0.row).xyz(), vector4(-1, 0, 0, 1).transform(temp0.row).xyz()))
				return true;

			if (has_line_intersected_cube(-scale1, scale1, vector4(0, 0, 1, 1).transform(temp0.row).xyz(), vector4(0, 0, -1, 1).transform(temp0.row).xyz()))
				return true;

			temp0 = matrix4x4::create(position1 - position0, scale1, rotation1) * matrix4x4::create_rotation(rotation0).inv();
			if (has_line_intersected_cube(-scale0, scale0, vector4(1, 1, 1, 1).transform(temp0.row).xyz(), vector4(-1, -1, -1, 1).transform(temp0.row).xyz()))
				return true;

			if (has_line_intersected_cube(-scale0, scale0, vector4(1, -1, 1, 1).transform(temp0.row).xyz(), vector4(-1, 1, -1, 1).transform(temp0.row).xyz()))
				return true;

			if (has_line_intersected_cube(-scale0, scale0, vector4(-1, -1, 1, 1).transform(temp0.row).xyz(), vector4(1, 1, -1, 1).transform(temp0.row).xyz()))
				return true;

			if (has_line_intersected_cube(-scale0, scale0, vector4(-1, 1, 1, 1).transform(temp0.row).xyz(), vector4(1, -1, -1, 1).transform(temp0.row).xyz()))
				return true;

			if (has_line_intersected_cube(-scale0, scale0, vector4(0, 1, 0, 1).transform(temp0.row).xyz(), vector4(0, -1, 0, 1).transform(temp0.row).xyz()))
				return true;

			if (has_line_intersected_cube(-scale0, scale0, vector4(1, 0, 0, 1).transform(temp0.row).xyz(), vector4(-1, 0, 0, 1).transform(temp0.row).xyz()))
				return true;

			if (has_line_intersected_cube(-scale0, scale0, vector4(0, 0, 1, 1).transform(temp0.row).xyz(), vector4(0, 0, -1, 1).transform(temp0.row).xyz()))
				return true;

			return false;
		}
		bool geometric::has_aabb_intersected(transform* R0, transform* R1)
		{
			VI_ASSERT(R0 != nullptr && R1 != nullptr, "transforms should be set");
			const vector3& position0 = R0->get_position();
			const vector3& position1 = R1->get_position();
			const vector3& scale0 = R0->get_scale();
			const vector3& scale1 = R1->get_scale();
			return
				(position0.x - scale0.x) <= (position1.x + scale1.x) &&
				(position1.x - scale1.x) <= (position0.x + scale0.x) &&
				(position0.y - scale0.y) <= (position1.y + scale1.y) &&
				(position1.y - scale1.y) <= (position0.y + scale0.y) &&
				(position0.z - scale0.z) <= (position1.z + scale1.z) &&
				(position1.z - scale1.z) <= (position0.z + scale0.z);
		}
		void geometric::flip_index_winding_order(core::vector<int>& indices)
		{
			std::reverse(indices.begin(), indices.end());
		}
		void geometric::matrix_rh_to_lh(trigonometry::matrix4x4* matrix)
		{
			VI_ASSERT(matrix != nullptr, "matrix should be set");
			*matrix = *matrix * RH_TO_LH;
		}
		void geometric::set_left_handed(bool is_left_handed)
		{
			VI_TRACE("[geometric] apply left-handed: %s", is_left_handed ? "on" : "off");
			left_handed = is_left_handed;
		}
		core::vector<int> geometric::create_triangle_strip(triangle_strip::desc& desc, const core::vector<int>& indices)
		{
			core::vector<uint32_t> src;
			src.reserve(indices.size());

			for (auto& index : indices)
				src.push_back((uint32_t)index);

			desc.faces = src.data();
			desc.nb_faces = (uint32_t)src.size() / 3;

			triangle_strip strip;
			if (!strip.fill(desc))
			{
				core::vector<int> empty;
				return empty;
			}

			triangle_strip::result result;
			if (!strip.resolve(result))
			{
				core::vector<int> empty;
				return empty;
			}

			return result.get_indices();
		}
		core::vector<int> geometric::create_triangle_list(const core::vector<int>& indices)
		{
			size_t size = indices.size() - 2;
			core::vector<int> result;
			result.reserve(size * 3);

			for (size_t i = 0; i < size; i++)
			{
				if (i % 2 == 0)
				{
					result.push_back(indices[i + 0]);
					result.push_back(indices[i + 1]);
					result.push_back(indices[i + 2]);
				}
				else
				{
					result.push_back(indices[i + 2]);
					result.push_back(indices[i + 1]);
					result.push_back(indices[i + 0]);
				}
			}

			return result;
		}
		void geometric::create_frustum8c_rad(vector4* result8, float field_of_view, float aspect, float near_z, float far_z)
		{
			VI_ASSERT(result8 != nullptr, "8 sized array should be set");
			float half_hfov = std::tan(field_of_view * 0.5f) * aspect;
			float half_vfov = std::tan(field_of_view * 0.5f);
			float XN = near_z * half_hfov;
			float XF = far_z * half_hfov;
			float YN = near_z * half_vfov;
			float YF = far_z * half_vfov;

			result8[0] = vector4(XN, YN, near_z, 1.0);
			result8[1] = vector4(-XN, YN, near_z, 1.0);
			result8[2] = vector4(XN, -YN, near_z, 1.0);
			result8[3] = vector4(-XN, -YN, near_z, 1.0);
			result8[4] = vector4(XF, YF, far_z, 1.0);
			result8[5] = vector4(-XF, YF, far_z, 1.0);
			result8[6] = vector4(XF, -YF, far_z, 1.0);
			result8[7] = vector4(-XF, -YF, far_z, 1.0);
		}
		void geometric::create_frustum8c(vector4* result8, float field_of_view, float aspect, float near_z, float far_z)
		{
			return create_frustum8c_rad(result8, compute::mathf::deg2rad() * field_of_view, aspect, near_z, far_z);
		}
		ray geometric::create_cursor_ray(const vector3& origin, const vector2& cursor, const vector2& screen, const matrix4x4& inv_projection, const matrix4x4& inv_view)
		{
			vector2 tmp = cursor * 2.0f;
			tmp /= screen;

			vector4 eye = vector4(tmp.x - 1.0f, 1.0f - tmp.y, 1.0f, 1.0f) * inv_projection;
			eye = (vector4(eye.x, eye.y, 1.0f, 0.0f) * inv_view).snormalize();
			return ray(origin, vector3(eye.x, eye.y, eye.z));
		}
		bool geometric::cursor_ray_test(const ray& cursor, const vector3& position, const vector3& scale, vector3* hit)
		{
			return cursor.intersects_aabb(position, scale, hit);
		}
		bool geometric::cursor_ray_test(const ray& cursor, const matrix4x4& world, vector3* hit)
		{
			return cursor.intersects_obb(world, hit);
		}
		float geometric::fast_inv_sqrt(float value)
		{
			float f = value;
			long i = *(long*)&f;
			i = 0x5f3759df - (i >> 1);
			f = *(float*)&i;
			f = f * (1.5f - ((value * 0.5f) * f * f));

			return f;
		}
		float geometric::fast_sqrt(float value)
		{
			return 1.0f / fast_inv_sqrt(value);
		}
		float geometric::aabb_volume(const vector3& min, const vector3& max)
		{
			float volume =
				(max[1] - min[1]) * (max[2] - min[2]) +
				(max[0] - min[0]) * (max[2] - min[2]) +
				(max[0] - min[0]) * (max[1] - min[1]);
			return volume * 2.0f;
		}
		float geometric::angluar_lerp(float a, float b, float delta_time)
		{
			if (a == b)
				return a;

			vector2 acircle = vector2(cosf(a), sinf(a));
			vector2 bcircle = vector2(cosf(b), sinf(b));
			vector2 floatnterpolation = acircle.lerp(bcircle, (float)delta_time);
			return std::atan2(floatnterpolation.y, floatnterpolation.x);
		}
		float geometric::angle_distance(float a, float b)
		{
			return vector2(cosf(a), sinf(a)).distance(vector2(cosf(b), sinf(b)));
		}
		bool geometric::left_handed = true;

		transform::transform(void* new_user_data) noexcept : root(nullptr), local(nullptr), scaling(false), dirty(true), user_data(new_user_data)
		{
		}
		transform::~transform() noexcept
		{
			set_root(nullptr);
			remove_childs();
			core::memory::deinit(local);
		}
		void transform::synchronize()
		{
			VI_ASSERT(!root || local != nullptr, "corrupted root transform");
			if (root != nullptr)
			{
				local->offset = matrix4x4::create(local->position, local->rotation) * root->get_bias_unscaled();
				global.position = local->offset.position();
				global.rotation = local->offset.rotation_euler();
				global.scale = (scaling ? local->scale : local->scale * root->global.scale);
				temporary = matrix4x4::create_scale(global.scale) * local->offset;
			}
			else
			{
				global.offset = matrix4x4::create(global.position, global.scale, global.rotation);
				temporary = matrix4x4::create_rotation(global.rotation) * matrix4x4::create_translation(global.position);
			}
			dirty = false;
		}
		void transform::move(const vector3& value)
		{
			VI_ASSERT(!root || local != nullptr, "corrupted root transform");
			if (root != nullptr)
				local->position += value;
			else
				global.position += value;
			make_dirty();
		}
		void transform::rotate(const vector3& value)
		{
			VI_ASSERT(!root || local != nullptr, "corrupted root transform");
			if (root != nullptr)
				local->rotation += value;
			else
				global.rotation += value;
			make_dirty();
		}
		void transform::rescale(const vector3& value)
		{
			VI_ASSERT(!root || local != nullptr, "corrupted root transform");
			if (root != nullptr)
				local->scale += value;
			else
				global.scale += value;
			make_dirty();
		}
		void transform::localize(spacing& where)
		{
			if (root != nullptr)
			{
				where.offset = matrix4x4::create(where.position, where.rotation) * root->get_bias_unscaled().inv();
				where.position = where.offset.position();
				where.rotation = where.offset.rotation_euler();
				where.scale = (scaling ? where.scale : where.scale / root->global.scale);
			}
		}
		void transform::globalize(spacing& where)
		{
			if (root != nullptr)
			{
				where.offset = matrix4x4::create(where.position, where.rotation) * root->get_bias_unscaled();
				where.position = where.offset.position();
				where.rotation = where.offset.rotation_euler();
				where.scale = (scaling ? where.scale : where.scale * root->global.scale);
			}
		}
		void transform::specialize(spacing& where)
		{
			VI_ASSERT(!root || local != nullptr, "corrupted root transform");
			if (root != nullptr)
			{
				where.offset = matrix4x4::create(local->position, local->rotation) * root->get_bias_unscaled();
				where.position = where.offset.position();
				where.rotation = where.offset.rotation_euler();
				where.scale = (scaling ? local->scale : local->scale * root->global.scale);
			}
			else if (&where != &global)
				where = global;
		}
		void transform::copy(transform* target)
		{
			VI_ASSERT(target != nullptr && target != this, "target should be set");
			core::memory::deinit(local);
			if (target->root != nullptr)
				local = core::memory::init<spacing>(*target->local);
			else
				local = nullptr;

			user_data = target->user_data;
			childs = target->childs;
			global = target->global;
			scaling = target->scaling;
			root = target->root;
			dirty = true;
		}
		void transform::add_child(transform* child)
		{
			VI_ASSERT(child != nullptr && child != this, "child should be set");
			childs.push_back(child);
			child->make_dirty();
		}
		void transform::remove_child(transform* child)
		{
			VI_ASSERT(child != nullptr && child != this, "child should be set");
			if (child->root == this)
				child->set_root(nullptr);
		}
		void transform::remove_childs()
		{
			core::vector<transform*> array;
			array.swap(childs);

			for (auto& child : array)
			{
				if (child->root == this)
					child->set_root(nullptr);
			}
		}
		void transform::when_dirty(core::task_callback&& callback)
		{
			on_dirty = std::move(callback);
			if (dirty && on_dirty)
				on_dirty();
		}
		void transform::make_dirty()
		{
			if (dirty)
				return;

			dirty = true;
			if (on_dirty)
				on_dirty();

			for (auto& child : childs)
				child->make_dirty();
		}
		void transform::set_scaling(bool enabled)
		{
			scaling = enabled;
			make_dirty();
		}
		void transform::set_position(const vector3& value)
		{
			VI_ASSERT(!root || local != nullptr, "corrupted root transform");
			bool updated = !(root ? local->position.is_equals(value) : global.position.is_equals(value));
			if (root != nullptr)
				local->position = value;
			else
				global.position = value;

			if (updated)
				make_dirty();
		}
		void transform::set_rotation(const vector3& value)
		{
			VI_ASSERT(!root || local != nullptr, "corrupted root transform");
			bool updated = !(root ? local->rotation.is_equals(value) : global.rotation.is_equals(value));
			if (root != nullptr)
				local->rotation = value;
			else
				global.rotation = value;

			if (updated)
				make_dirty();
		}
		void transform::set_scale(const vector3& value)
		{
			VI_ASSERT(!root || local != nullptr, "corrupted root transform");
			bool updated = !(root ? local->scale.is_equals(value) : global.scale.is_equals(value));
			if (root != nullptr)
				local->scale = value;
			else
				global.scale = value;

			if (updated)
				make_dirty();
		}
		void transform::set_spacing(positioning space, spacing& where)
		{
			if (space == positioning::global)
				localize(where);

			get_spacing() = where;
			make_dirty();
		}
		void transform::set_pivot(transform* parent, spacing* pivot)
		{
			core::memory::deinit(local);
			root = parent;
			local = pivot;

			if (parent != nullptr)
				parent->add_child(this);
		}
		void transform::set_root(transform* parent)
		{
			if (!can_root_be_applied(parent))
				return;

			if (root != nullptr)
			{
				specialize(global);
				if (parent != nullptr)
				{
					core::memory::deinit(local);
					local = nullptr;
				}

				for (auto it = root->childs.begin(); it != root->childs.end(); ++it)
				{
					if ((*it) == this)
					{
						root->childs.erase(it);
						break;
					}
				}
			}

			root = parent;
			if (root != nullptr)
			{
				root->add_child(this);
				if (!local)
					local = core::memory::init<spacing>();

				*local = global;
				localize(*local);
			}
		}
		void transform::get_bounds(matrix4x4& world, vector3& min, vector3& max)
		{
			transform::spacing& space = get_spacing();
			vector3 scale = (max - min).abs();
			vector3 radius = scale * space.scale * 0.5f;
			vector3 top = radius.rotate(0, space.rotation);
			vector3 left = radius.inv_x().rotate(0, space.rotation);
			vector3 right = radius.inv_y().rotate(0, space.rotation);
			vector3 bottom = radius.inv_z().rotate(0, space.rotation);
			vector3 points[8] =
			{
				-top, top, -left, left,
				-right, right, -bottom, bottom
			};
			vector3 lower = points[0];
			vector3 upper = points[1];

			for (size_t i = 0; i < 8; ++i)
			{
				auto& point = points[i];
				if (point.x > upper.x)
					upper.x = point.x;
				if (point.y > upper.y)
					upper.y = point.y;
				if (point.z > upper.z)
					upper.z = point.z;

				if (point.x < lower.x)
					lower.x = point.x;
				if (point.y < lower.y)
					lower.y = point.y;
				if (point.z < lower.z)
					lower.z = point.z;
			}

			min = space.position + lower;
			max = space.position + upper;
			world = get_bias();
		}
		bool transform::has_root(const transform* target) const
		{
			VI_ASSERT(target != nullptr, "target should be set");
			trigonometry::transform* upper_root = root;
			while (upper_root != nullptr)
			{
				if (upper_root == target)
					return true;

				upper_root = upper_root->get_root();
			}

			return false;
		}
		bool transform::has_child(transform* target) const
		{
			VI_ASSERT(target != nullptr, "target should be set");
			for (auto& child : childs)
			{
				if (child == target)
					return true;

				if (child->has_child(target))
					return true;
			}

			return false;
		}
		bool transform::has_scaling() const
		{
			return scaling;
		}
		bool transform::can_root_be_applied(transform* parent) const
		{
			if ((!root && !parent) || root == parent)
				return false;

			if (parent == this)
				return false;

			if (parent && parent->has_root(this))
				return false;

			return true;
		}
		bool transform::is_dirty() const
		{
			return dirty;
		}
		const matrix4x4& transform::get_bias() const
		{
			VI_ASSERT(!root || local != nullptr, "corrupted root transform");
			if (root != nullptr)
				return temporary;

			return global.offset;
		}
		const matrix4x4& transform::get_bias_unscaled() const
		{
			VI_ASSERT(!root || local != nullptr, "corrupted root transform");
			if (root != nullptr)
				return local->offset;

			return temporary;
		}
		const vector3& transform::get_position() const
		{
			return global.position;
		}
		const vector3& transform::get_rotation() const
		{
			return global.rotation;
		}
		const vector3& transform::get_scale() const
		{
			return global.scale;
		}
		vector3 transform::forward() const
		{
			VI_ASSERT(!root || local != nullptr, "corrupted root transform");
			if (root != nullptr)
				return local->offset.forward();

			return global.offset.forward();
		}
		vector3 transform::right() const
		{
			VI_ASSERT(!root || local != nullptr, "corrupted root transform");
			if (root != nullptr)
				return local->offset.right();

			return global.offset.right();
		}
		vector3 transform::up() const
		{
			VI_ASSERT(!root || local != nullptr, "corrupted root transform");
			if (root != nullptr)
				return local->offset.up();

			return global.offset.up();
		}
		transform::spacing& transform::get_spacing()
		{
			VI_ASSERT(!root || local != nullptr, "corrupted root transform");
			if (root != nullptr)
				return *local;

			return global;
		}
		transform::spacing& transform::get_spacing(positioning space)
		{
			VI_ASSERT(!root || local != nullptr, "corrupted root transform");
			if (space == positioning::local)
				return *local;

			return global;
		}
		transform* transform::get_root() const
		{
			return root;
		}
		transform* transform::get_upper_root() const
		{
			trigonometry::transform* upper_root = root;
			while (upper_root != nullptr)
				upper_root = upper_root->get_root();

			return nullptr;
		}
		transform* transform::get_child(size_t child) const
		{
			VI_ASSERT(child < childs.size(), "index outside of range");
			return childs[child];
		}
		size_t transform::get_childs_count() const
		{
			return childs.size();
		}
		core::vector<transform*>& transform::get_childs()
		{
			return childs;
		}

		bool cosmos::node::is_leaf() const
		{
			return (left == NULL_NODE);
		}

		cosmos::cosmos(size_t default_size) noexcept
		{
			root = NULL_NODE;
			node_count = 0;
			node_capacity = default_size;
			nodes.resize(node_capacity);

			for (size_t i = 0; i < node_capacity - 1; i++)
			{
				auto& node = nodes[i];
				node.next = i + 1;
				node.height = -1;
			}

			auto& node = nodes[node_capacity - 1];
			node.next = NULL_NODE;
			node.height = -1;
			free_list = 0;
		}
		void cosmos::free_node(size_t node_index)
		{
			VI_ASSERT(node_index < node_capacity, "outside of borders");
			VI_ASSERT(node_count > 0, "there must be at least one node");

			auto& node = nodes[node_index];
			node.next = free_list;
			node.height = -1;
			free_list = node_index;
			node_count--;
		}
		void cosmos::insert_item(void* item, const vector3& lower, const vector3& upper)
		{
			size_t node_index = allocate_node();
			auto& node = nodes[node_index];
			node.bounds = bounding(lower, upper);
			node.height = 0;
			node.item = item;
			insert_leaf(node_index);

			items.insert(core::unordered_map<void*, size_t>::value_type(item, node_index));
		}
		void cosmos::remove_item(void* item)
		{
			auto it = items.find(item);
			if (it == items.end())
				return;

			size_t node_index = it->second;
			items.erase(it);

			VI_ASSERT(node_index < node_capacity, "outside of borders");
			VI_ASSERT(nodes[node_index].is_leaf(), "cannot remove root node");

			remove_leaf(node_index);
			free_node(node_index);
		}
		void cosmos::insert_leaf(size_t leaf_index)
		{
			if (root == NULL_NODE)
			{
				root = leaf_index;
				nodes[root].parent = NULL_NODE;
				return;
			}

			size_t next_index = root;
			bounding leaf_bounds = nodes[leaf_index].bounds;
			bounding next_bounds;

			while (!nodes[next_index].is_leaf())
			{
				auto& next = nodes[next_index];
				auto& left = nodes[next.left];
				auto& right = nodes[next.right];

				next_bounds.merge(leaf_bounds, next.bounds);
				float base_cost = 2.0f * (float)next_bounds.volume;
				float parent_cost = 2.0f * (float)(next_bounds.volume - next.bounds.volume);

				next_bounds.merge(leaf_bounds, left.bounds);
				float left_cost = (next_bounds.volume - (left.is_leaf() ? 0.0f : left.bounds.volume)) + parent_cost;

				next_bounds.merge(leaf_bounds, right.bounds);
				float right_cost = (next_bounds.volume - (right.is_leaf() ? 0.0f : right.bounds.volume)) + parent_cost;

				if ((base_cost < left_cost) && (base_cost < right_cost))
					break;

				if (left_cost < right_cost)
					next_index = next.left;
				else
					next_index = next.right;
			}

			size_t new_parent_index = allocate_node();
			auto& leaf = nodes[leaf_index];
			auto& sibling = nodes[next_index];
			auto& new_parent = nodes[new_parent_index];

			size_t old_parent_index = sibling.parent;
			new_parent.parent = old_parent_index;
			new_parent.bounds.merge(leaf_bounds, sibling.bounds);
			new_parent.height = sibling.height + 1;

			if (old_parent_index != NULL_NODE)
			{
				auto& old_parent = nodes[old_parent_index];
				if (old_parent.left == next_index)
					old_parent.left = new_parent_index;
				else
					old_parent.right = new_parent_index;

				new_parent.left = next_index;
				new_parent.right = leaf_index;
				sibling.parent = new_parent_index;
				leaf.parent = new_parent_index;
			}
			else
			{
				new_parent.left = next_index;
				new_parent.right = leaf_index;
				sibling.parent = new_parent_index;
				leaf.parent = new_parent_index;
				root = new_parent_index;
			}

			next_index = leaf.parent;
			while (next_index != NULL_NODE)
			{
				next_index = balance(next_index);
				auto& next = nodes[next_index];
				auto& left = nodes[next.left];
				auto& right = nodes[next.right];

				next.height = 1 + std::max(left.height, right.height);
				next.bounds.merge(left.bounds, right.bounds);
				next_index = next.parent;
			}
		}
		void cosmos::remove_leaf(size_t leaf_index)
		{
			if (leaf_index == root)
			{
				root = NULL_NODE;
				return;
			}

			size_t parent_index = nodes[leaf_index].parent;
			auto& parent = nodes[parent_index];

			size_t sibling_index = (parent.left == leaf_index ? parent.right : parent.left);
			auto& sibling = nodes[sibling_index];

			if (parent.parent != NULL_NODE)
			{
				auto& upper_parent = nodes[parent.parent];
				if (upper_parent.left == parent_index)
					upper_parent.left = sibling_index;
				else
					upper_parent.right = sibling_index;

				sibling.parent = parent.parent;
				free_node(parent_index);

				size_t next_index = parent.parent;
				while (next_index != NULL_NODE)
				{
					next_index = balance(next_index);
					auto& next = nodes[next_index];
					auto& left = nodes[next.left];
					auto& right = nodes[next.right];

					next.bounds.merge(left.bounds, right.bounds);
					next.height = 1 + std::max(left.height, right.height);
					next_index = next.parent;
				}
			}
			else
			{
				root = sibling_index;
				sibling.parent = NULL_NODE;
				free_node(parent_index);
			}
		}
		void cosmos::reserve(size_t size)
		{
			if (size < nodes.capacity())
				return;

			items.reserve(size);
			nodes.reserve(size);
		}
		void cosmos::clear()
		{
			auto it = items.begin();
			while (it != items.end())
			{
				size_t node_index = it->second;
				VI_ASSERT(node_index < node_capacity, "outside of borders");
				VI_ASSERT(nodes[node_index].is_leaf(), "cannot remove root node");

				remove_leaf(node_index);
				free_node(node_index);
				it++;
			}
			items.clear();
		}
		bool cosmos::update_item(void* item, const vector3& lower, const vector3& upper, bool always)
		{
			auto it = items.find(item);
			if (it == items.end())
				return false;

			auto& next = nodes[it->second];
			if (!always)
			{
				bounding bounds(lower, upper);
				if (next.bounds.contains(bounds))
					return true;

				remove_leaf(it->second);
				next.bounds = bounds;
			}
			else
			{
				remove_leaf(it->second);
				next.bounds = bounding(lower, upper);
			}

			insert_leaf(it->second);
			return true;
		}
		const bounding& cosmos::get_area(void* item)
		{
			auto it = items.find(item);
			if (it != items.end())
				return nodes[it->second].bounds;

			return nodes[root].bounds;
		}
		size_t cosmos::allocate_node()
		{
			if (free_list == NULL_NODE)
			{
				VI_ASSERT(node_count == node_capacity, "invalid capacity");

				node_capacity *= 2;
				nodes.resize(node_capacity);

				for (size_t i = node_count; i < node_capacity - 1; i++)
				{
					auto& next = nodes[i];
					next.next = i + 1;
					next.height = -1;
				}

				nodes[node_capacity - 1].next = NULL_NODE;
				nodes[node_capacity - 1].height = -1;
				free_list = node_count;
			}

			size_t node_index = free_list;
			auto& node = nodes[node_index];
			free_list = node.next;
			node.parent = NULL_NODE;
			node.left = NULL_NODE;
			node.right = NULL_NODE;
			node.height = 0;
			node_count++;

			return node_index;
		}
		size_t cosmos::balance(size_t node_index)
		{
			VI_ASSERT(node_index != NULL_NODE, "node should not be null");

			auto& next = nodes[node_index];
			if (next.is_leaf() || (next.height < 2))
				return node_index;

			VI_ASSERT(next.left < node_capacity, "left outside of borders");
			VI_ASSERT(next.right < node_capacity, "right outside of borders");

			size_t left_index = next.left;
			size_t right_index = next.right;
			auto& left = nodes[left_index];
			auto& right = nodes[right_index];

			int balance = right.height - left.height;
			if (balance > 1)
			{
				VI_ASSERT(right.left < node_capacity, "subleft outside of borders");
				VI_ASSERT(right.right < node_capacity, "subright outside of borders");

				size_t right_left_index = right.left;
				size_t right_right_index = right.right;
				auto& right_left = nodes[right_left_index];
				auto& right_right = nodes[right_right_index];

				right.left = node_index;
				right.parent = next.parent;
				next.parent = right_index;

				if (right.parent != NULL_NODE)
				{
					if (nodes[right.parent].left != node_index)
					{
						VI_ASSERT(nodes[right.parent].right == node_index, "invalid right spacing");
						nodes[right.parent].right = right_index;
					}
					else
						nodes[right.parent].left = right_index;
				}
				else
					root = right_index;

				if (right_left.height > right_right.height)
				{
					right.right = right_left_index;
					next.right = right_right_index;
					right_right.parent = node_index;
					next.bounds.merge(left.bounds, right_right.bounds);
					right.bounds.merge(next.bounds, right_left.bounds);

					next.height = 1 + std::max(left.height, right_right.height);
					right.height = 1 + std::max(next.height, right_left.height);
				}
				else
				{
					right.right = right_right_index;
					next.right = right_left_index;
					right_left.parent = node_index;
					next.bounds.merge(left.bounds, right_left.bounds);
					right.bounds.merge(next.bounds, right_right.bounds);

					next.height = 1 + std::max(left.height, right_left.height);
					right.height = 1 + std::max(next.height, right_right.height);
				}

				return right_index;
			}
			else if (balance < -1)
			{
				VI_ASSERT(left.left < node_capacity, "subleft outside of borders");
				VI_ASSERT(left.right < node_capacity, "subright outside of borders");

				size_t left_left_index = left.left;
				size_t left_right_index = left.right;
				auto& left_left = nodes[left_left_index];
				auto& left_right = nodes[left_right_index];

				left.left = node_index;
				left.parent = next.parent;
				next.parent = left_index;

				if (left.parent != NULL_NODE)
				{
					if (nodes[left.parent].left != node_index)
					{
						VI_ASSERT(nodes[left.parent].right == node_index, "invalid left spacing");
						nodes[left.parent].right = left_index;
					}
					else
						nodes[left.parent].left = left_index;
				}
				else
					root = left_index;

				if (left_left.height > left_right.height)
				{
					left.right = left_left_index;
					next.left = left_right_index;
					left_right.parent = node_index;
					next.bounds.merge(right.bounds, left_right.bounds);
					left.bounds.merge(next.bounds, left_left.bounds);
					next.height = 1 + std::max(right.height, left_right.height);
					left.height = 1 + std::max(next.height, left_left.height);
				}
				else
				{
					left.right = left_right_index;
					next.left = left_left_index;
					left_left.parent = node_index;
					next.bounds.merge(right.bounds, left_left.bounds);
					left.bounds.merge(next.bounds, left_right.bounds);
					next.height = 1 + std::max(right.height, left_left.height);
					left.height = 1 + std::max(next.height, left_right.height);
				}

				return left_index;
			}

			return node_index;
		}
		size_t cosmos::compute_height() const
		{
			return compute_height(root);
		}
		size_t cosmos::compute_height(size_t node_index) const
		{
			VI_ASSERT(node_index < node_capacity, "outside of borders");

			auto& next = nodes[node_index];
			if (next.is_leaf())
				return 0;

			size_t height1 = compute_height(next.left);
			size_t height2 = compute_height(next.right);
			return 1 + std::max(height1, height2);
		}
		size_t cosmos::get_nodes_count() const
		{
			return nodes.size();
		}
		size_t cosmos::get_height() const
		{
			return root == NULL_NODE ? 0 : nodes[root].height;
		}
		size_t cosmos::get_max_balance() const
		{
			size_t max_balance = 0;
			for (size_t i = 0; i < node_capacity; i++)
			{
				auto& next = nodes[i];
				if (next.height <= 1)
					continue;

				VI_ASSERT(!next.is_leaf(), "node should be leaf");
				size_t balance = std::abs(nodes[next.left].height - nodes[next.right].height);
				max_balance = std::max(max_balance, balance);
			}

			return max_balance;
		}
		size_t cosmos::get_root() const
		{
			return root;
		}
		const core::unordered_map<void*, size_t>& cosmos::get_items() const
		{
			return items;
		}
		const core::vector<cosmos::node>& cosmos::get_nodes() const
		{
			return nodes;
		}
		const cosmos::node& cosmos::get_root_node() const
		{
			VI_ASSERT(root < nodes.size(), "index out of range");
			return nodes[root];
		}
		const cosmos::node& cosmos::get_node(size_t id) const
		{
			VI_ASSERT(id < nodes.size(), "index out of range");
			return nodes[id];
		}
		float cosmos::get_volume_ratio() const
		{
			if (root == NULL_NODE)
				return 0.0;

			float root_area = nodes[root].bounds.volume;
			float total_area = 0.0;

			for (size_t i = 0; i < node_capacity; i++)
			{
				auto& next = nodes[i];
				if (next.height >= 0)
					total_area += next.bounds.volume;
			}

			return total_area / root_area;
		}
		bool cosmos::is_null(size_t id) const
		{
			return id == NULL_NODE;
		}
		bool cosmos::empty() const
		{
			return items.empty();
		}
	}
}
