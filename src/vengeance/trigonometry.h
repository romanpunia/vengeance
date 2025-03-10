#ifndef VI_TRIGONOMETRY_H
#define VI_TRIGONOMETRY_H
#include <vitex/compute.h>

namespace vitex
{
	namespace trigonometry
	{
		class transform;

		struct matrix4x4;

		struct quaternion;

		struct vector2;

		struct vector3;

		struct vector4;

		enum class rotator
		{
			xyz = 0,
			xzy,
			yxz,
			yzx,
			zxy,
			zyx
		};

		enum class positioning
		{
			local,
			global
		};

		enum class cube_face
		{
			positive_x,
			negative_x,
			positive_y,
			negative_y,
			positive_z,
			negative_z
		};

		struct vertex
		{
			float position_x;
			float position_y;
			float position_z;
			float texcoord_x;
			float texcoord_y;
			float normal_x;
			float normal_y;
			float normal_z;
			float tangent_x;
			float tangent_y;
			float tangent_z;
			float bitangent_x;
			float bitangent_y;
			float bitangent_z;
		};

		struct skin_vertex
		{
			float position_x;
			float position_y;
			float position_z;
			float texcoord_x;
			float texcoord_y;
			float normal_x;
			float normal_y;
			float normal_z;
			float tangent_x;
			float tangent_y;
			float tangent_z;
			float bitangent_x;
			float bitangent_y;
			float bitangent_z;
			float joint_index0;
			float joint_index1;
			float joint_index2;
			float joint_index3;
			float joint_bias0;
			float joint_bias1;
			float joint_bias2;
			float joint_bias3;
		};

		struct shape_vertex
		{
			float position_x;
			float position_y;
			float position_z;
			float texcoord_x;
			float texcoord_y;
		};

		struct element_vertex
		{
			float position_x;
			float position_y;
			float position_z;
			float scale;
			float velocity_x;
			float velocity_y;
			float velocity_z;
			float rotation;
			float padding1;
			float padding2;
			float padding3;
			float angular;
			float color_x;
			float color_y;
			float color_z;
			float color_w;
		};

		struct vector2
		{
			float x;
			float y;

			vector2() noexcept;
			vector2(float x, float y) noexcept;
			vector2(float xy) noexcept;
			vector2(const vector2& value) noexcept;
			vector2(const vector3& value) noexcept;
			vector2(const vector4& value) noexcept;
			bool is_equals(const vector2& other, float max_displacement = 0.000001f) const;
			float length() const;
			float sum() const;
			float dot(const vector2& b) const;
			float distance(const vector2& target) const;
			float hypotenuse() const;
			float look_at(const vector2& at) const;
			float cross(const vector2& vector1) const;
			vector2 transform(const matrix4x4& v) const;
			vector2 direction(float rotation) const;
			vector2 inv() const;
			vector2 inv_x() const;
			vector2 inv_y() const;
			vector2 normalize() const;
			vector2 snormalize() const;
			vector2 lerp(const vector2& b, float delta_time) const;
			vector2 slerp(const vector2& b, float delta_time) const;
			vector2 alerp(const vector2& b, float delta_time) const;
			vector2 rlerp() const;
			vector2 abs() const;
			vector2 radians() const;
			vector2 degrees() const;
			vector2 xy() const;
			vector3 xyz() const;
			vector4 xyzw() const;
			vector2 mul(float xy) const;
			vector2 mul(float x, float y) const;
			vector2 mul(const vector2& value) const;
			vector2 div(const vector2& value) const;
			vector2 add(const vector2& value) const;
			vector2 set_x(float x) const;
			vector2 set_y(float y) const;
			void set(const vector2& value);
			void get2(float* in) const;
			vector2& operator *=(const vector2& v);
			vector2& operator *=(float v);
			vector2& operator /=(const vector2& v);
			vector2& operator /=(float v);
			vector2& operator +=(const vector2& v);
			vector2& operator +=(float v);
			vector2& operator -=(const vector2& v);
			vector2& operator -=(float v);
			vector2 operator *(const vector2& v) const;
			vector2 operator *(float v) const;
			vector2 operator /(const vector2& v) const;
			vector2 operator /(float v) const;
			vector2 operator +(const vector2& v) const;
			vector2 operator +(float v) const;
			vector2 operator -(const vector2& v) const;
			vector2 operator -(float v) const;
			vector2 operator -() const;
			vector2& operator =(const vector2& v) noexcept;
			bool operator ==(const vector2& v) const;
			bool operator !=(const vector2& v) const;
			bool operator <=(const vector2& v) const;
			bool operator >=(const vector2& v) const;
			bool operator >(const vector2& v) const;
			bool operator <(const vector2& v) const;
			float& operator [](uint32_t axis);
			float operator [](uint32_t axis) const;

			static vector2 random();
			static vector2 random_abs();
			static vector2 one()
			{
				return vector2(1, 1);
			}
			static vector2 zero()
			{
				return vector2(0, 0);
			}
			static vector2 up()
			{
				return vector2(0, 1);
			}
			static vector2 down()
			{
				return vector2(0, -1);
			}
			static vector2 left()
			{
				return vector2(-1, 0);
			}
			static vector2 right()
			{
				return vector2(1, 0);
			}
		};

		struct vector3
		{
			float x;
			float y;
			float z;

			vector3() noexcept;
			vector3(const vector2& value) noexcept;
			vector3(const vector3& value) noexcept;
			vector3(const vector4& value) noexcept;
			vector3(float x, float y) noexcept;
			vector3(float x, float y, float z) noexcept;
			vector3(float xyz) noexcept;
			bool is_equals(const vector3& other, float max_displacement = 0.000001f) const;
			float length() const;
			float sum() const;
			float dot(const vector3& b) const;
			float distance(const vector3& target) const;
			float hypotenuse() const;
			vector3 look_at(const vector3& vector) const;
			vector3 cross(const vector3& vector) const;
			vector3 transform(const matrix4x4& v) const;
			vector3 hdirection() const;
			vector3 ddirection() const;
			vector3 direction() const;
			vector3 inv() const;
			vector3 inv_x() const;
			vector3 inv_y() const;
			vector3 inv_z() const;
			vector3 normalize() const;
			vector3 snormalize() const;
			vector3 lerp(const vector3& b, float delta_time) const;
			vector3 slerp(const vector3& b, float delta_time) const;
			vector3 alerp(const vector3& b, float delta_time) const;
			vector3 rlerp() const;
			vector3 abs() const;
			vector3 radians() const;
			vector3 degrees() const;
			vector3 view_space() const;
			vector2 xy() const;
			vector3 xyz() const;
			vector4 xyzw() const;
			vector3 mul(float xyz) const;
			vector3 mul(const vector2& XY, float z) const;
			vector3 mul(const vector3& value) const;
			vector3 div(const vector3& value) const;
			vector3 add(const vector3& value) const;
			vector3 set_x(float x) const;
			vector3 set_y(float y) const;
			vector3 set_z(float z) const;
			vector3 rotate(const vector3& origin, const vector3& rotation);
			void set(const vector3& value);
			void get2(float* in) const;
			void get3(float* in) const;
			vector3& operator *=(const vector3& v);
			vector3& operator *=(float v);
			vector3& operator /=(const vector3& v);
			vector3& operator /=(float v);
			vector3& operator +=(const vector3& v);
			vector3& operator +=(float v);
			vector3& operator -=(const vector3& v);
			vector3& operator -=(float v);
			vector3 operator *(const vector3& v) const;
			vector3 operator *(float v) const;
			vector3 operator /(const vector3& v) const;
			vector3 operator /(float v) const;
			vector3 operator +(const vector3& v) const;
			vector3 operator +(float v) const;
			vector3 operator -(const vector3& v) const;
			vector3 operator -(float v) const;
			vector3 operator -() const;
			vector3& operator =(const vector3& v) noexcept;
			bool operator ==(const vector3& v) const;
			bool operator !=(const vector3& v) const;
			bool operator <=(const vector3& v) const;
			bool operator >=(const vector3& v) const;
			bool operator >(const vector3& v) const;
			bool operator <(const vector3& v) const;
			float& operator [](uint32_t axis);
			float operator [](uint32_t axis) const;
			static vector3 random();
			static vector3 random_abs();
			static vector3 one()
			{
				return vector3(1, 1, 1);
			}
			static vector3 zero()
			{
				return vector3(0, 0, 0);
			}
			static vector3 up()
			{
				return vector3(0, 1, 0);
			}
			static vector3 down()
			{
				return vector3(0, -1, 0);
			}
			static vector3 left()
			{
				return vector3(-1, 0, 0);
			}
			static vector3 right()
			{
				return vector3(1, 0, 0);
			}
			static vector3 forward()
			{
				return vector3(0, 0, 1);
			}
			static vector3 backward()
			{
				return vector3(0, 0, -1);
			}
		};

		struct vector4
		{
			float x;
			float y;
			float z;
			float w;

			vector4() noexcept;
			vector4(const vector2& value) noexcept;
			vector4(const vector3& value) noexcept;
			vector4(const vector4& value) noexcept;
			vector4(float x, float y) noexcept;
			vector4(float x, float y, float z) noexcept;
			vector4(float x, float y, float z, float w) noexcept;
			vector4(float xyzw) noexcept;
			bool is_equals(const vector4& other, float max_displacement = 0.000001f) const;
			float length() const;
			float sum() const;
			float dot(const vector4& b) const;
			float distance(const vector4& target) const;
			vector4 cross(const vector4& vector1) const;
			vector4 transform(const matrix4x4& matrix) const;
			vector4 inv() const;
			vector4 inv_x() const;
			vector4 inv_y() const;
			vector4 inv_z() const;
			vector4 inv_w() const;
			vector4 normalize() const;
			vector4 snormalize() const;
			vector4 lerp(const vector4& b, float delta_time) const;
			vector4 slerp(const vector4& b, float delta_time) const;
			vector4 alerp(const vector4& b, float delta_time) const;
			vector4 rlerp() const;
			vector4 abs() const;
			vector4 radians() const;
			vector4 degrees() const;
			vector4 view_space() const;
			vector2 xy() const;
			vector3 xyz() const;
			vector4 xyzw() const;
			vector4 mul(float xyzw) const;
			vector4 mul(const vector2& XY, float z, float w) const;
			vector4 mul(const vector3& XYZ, float w) const;
			vector4 mul(const vector4& value) const;
			vector4 div(const vector4& value) const;
			vector4 add(const vector4& value) const;
			vector4 set_x(float x) const;
			vector4 set_y(float y) const;
			vector4 set_z(float z) const;
			vector4 set_w(float w) const;
			void set(const vector4& value);
			void get2(float* in) const;
			void get3(float* in) const;
			void get4(float* in) const;
			vector4& operator *=(const matrix4x4& v);
			vector4& operator *=(const vector4& v);
			vector4& operator *=(float v);
			vector4& operator /=(const vector4& v);
			vector4& operator /=(float v);
			vector4& operator +=(const vector4& v);
			vector4& operator +=(float v);
			vector4& operator -=(const vector4& v);
			vector4& operator -=(float v);
			vector4 operator *(const matrix4x4& v) const;
			vector4 operator *(const vector4& v) const;
			vector4 operator *(float v) const;
			vector4 operator /(const vector4& v) const;
			vector4 operator /(float v) const;
			vector4 operator +(const vector4& v) const;
			vector4 operator +(float v) const;
			vector4 operator -(const vector4& v) const;
			vector4 operator -(float v) const;
			vector4 operator -() const;
			vector4& operator =(const vector4& v) noexcept;
			bool operator ==(const vector4& v) const;
			bool operator !=(const vector4& v) const;
			bool operator <=(const vector4& v) const;
			bool operator >=(const vector4& v) const;
			bool operator >(const vector4& v) const;
			bool operator <(const vector4& v) const;
			float& operator [](uint32_t axis);
			float operator [](uint32_t axis) const;

			static vector4 random();
			static vector4 random_abs();
			static vector4 one()
			{
				return vector4(1, 1, 1, 1);
			};
			static vector4 zero()
			{
				return vector4(0, 0, 0, 0);
			};
			static vector4 up()
			{
				return vector4(0, 1, 0, 0);
			};
			static vector4 down()
			{
				return vector4(0, -1, 0, 0);
			};
			static vector4 left()
			{
				return vector4(-1, 0, 0, 0);
			};
			static vector4 right()
			{
				return vector4(1, 0, 0, 0);
			};
			static vector4 forward()
			{
				return vector4(0, 0, 1, 0);
			};
			static vector4 backward()
			{
				return vector4(0, 0, -1, 0);
			};
			static vector4 unit_w()
			{
				return vector4(0, 0, 0, 1);
			};
		};

		struct matrix4x4
		{
		public:
			float row[16];

		public:
			matrix4x4() noexcept;
			matrix4x4(float array[16]) noexcept;
			matrix4x4(const vector4& row0, const vector4& row1, const vector4& row2, const vector4& row3) noexcept;
			matrix4x4(float row00, float row01, float row02, float row03, float row10, float row11, float row12, float row13, float row20, float row21, float row22, float row23, float row30, float row31, float row32, float row33) noexcept;
			matrix4x4(const matrix4x4& other) noexcept;
			float& operator [](uint32_t index);
			float operator [](uint32_t index) const;
			bool operator ==(const matrix4x4& index) const;
			bool operator !=(const matrix4x4& index) const;
			matrix4x4 operator *(const matrix4x4& v) const;
			vector4 operator *(const vector4& v) const;
			matrix4x4& operator =(const matrix4x4& v) noexcept;
			matrix4x4 mul(const matrix4x4& right) const;
			matrix4x4 mul(const vector4& right) const;
			matrix4x4 inv() const;
			matrix4x4 transpose() const;
			matrix4x4 set_scale(const vector3& value) const;
			vector4 row11() const;
			vector4 row22() const;
			vector4 row33() const;
			vector4 row44() const;
			vector3 up() const;
			vector3 right() const;
			vector3 forward() const;
			quaternion rotation_quaternion() const;
			vector3 rotation_euler() const;
			vector3 position() const;
			vector3 scale() const;
			vector2 xy() const;
			vector3 xyz() const;
			vector4 xyzw() const;
			float determinant() const;
			void identify();
			void set(const matrix4x4& value);

		private:
			matrix4x4(bool) noexcept;

		public:
			static matrix4x4 create_look_at(const vector3& position, const vector3& target, const vector3& up);
			static matrix4x4 create_rotation_x(float rotation);
			static matrix4x4 create_rotation_y(float rotation);
			static matrix4x4 create_rotation_z(float rotation);
			static matrix4x4 create_scale(const vector3& scale);
			static matrix4x4 create_translated_scale(const vector3& position, const vector3& scale);
			static matrix4x4 create_translation(const vector3& position);
			static matrix4x4 create_perspective(float field_of_view, float aspect_ratio, float near_z, float far_z);
			static matrix4x4 create_perspective_rad(float field_of_view, float aspect_ratio, float near_z, float far_z);
			static matrix4x4 create_orthographic(float width, float height, float near_z, float far_z);
			static matrix4x4 create_orthographic_off_center(float left, float right, float bottom, float top, float near_z, float far_z);
			static matrix4x4 create(const vector3& position, const vector3& scale, const vector3& rotation);
			static matrix4x4 create(const vector3& position, const vector3& rotation);
			static matrix4x4 create_rotation(const vector3& rotation);
			static matrix4x4 create_view(const vector3& position, const vector3& rotation);
			static matrix4x4 create_look_at(cube_face face, const vector3& position);
			static matrix4x4 create_rotation(const vector3& forward, const vector3& up, const vector3& right);
			static matrix4x4 identity()
			{
				return matrix4x4(
					1, 0, 0, 0,
					0, 1, 0, 0,
					0, 0, 1, 0,
					0, 0, 0, 1);
			};
		};

		struct quaternion
		{
			float x, y, z, w;

			quaternion() noexcept;
			quaternion(float x, float y, float z, float w) noexcept;
			quaternion(const quaternion& in) noexcept;
			quaternion(const vector3& axis, float angle) noexcept;
			quaternion(const vector3& euler) noexcept;
			quaternion(const matrix4x4& value) noexcept;
			void set_axis(const vector3& axis, float angle);
			void set_euler(const vector3& euler);
			void set_matrix(const matrix4x4& value);
			void set(const quaternion& value);
			quaternion operator *(float r) const;
			vector3 operator *(const vector3& r) const;
			quaternion operator *(const quaternion& r) const;
			quaternion operator -(const quaternion& r) const;
			quaternion operator +(const quaternion& r) const;
			quaternion& operator =(const quaternion& r) noexcept;
			quaternion normalize() const;
			quaternion snormalize() const;
			quaternion conjugate() const;
			quaternion mul(float r) const;
			quaternion mul(const quaternion& r) const;
			vector3 mul(const vector3& r) const;
			quaternion sub(const quaternion& r) const;
			quaternion add(const quaternion& r) const;
			quaternion lerp(const quaternion& b, float delta_time) const;
			quaternion slerp(const quaternion& b, float delta_time) const;
			vector3 forward() const;
			vector3 up() const;
			vector3 right() const;
			matrix4x4 get_matrix() const;
			vector3 get_euler() const;
			float dot(const quaternion& r) const;
			float length() const;
			bool operator ==(const quaternion& v) const;
			bool operator !=(const quaternion& v) const;

			static quaternion create_euler_rotation(const vector3& euler);
			static quaternion create_rotation(const matrix4x4& transform);
		};

		struct rectangle
		{
			int64_t left;
			int64_t top;
			int64_t right;
			int64_t bottom;

			int64_t get_x() const;
			int64_t get_y() const;
			int64_t get_width() const;
			int64_t get_height() const;
		};

		struct bounding
		{
		public:
			vector3 lower;
			vector3 upper;
			vector3 center;
			float radius;
			float volume;

		public:
			bounding() noexcept;
			bounding(const vector3&, const vector3&) noexcept;
			void merge(const bounding&, const bounding&);
			bool contains(const bounding&) const;
			bool overlaps(const bounding&) const;
		};

		struct ray
		{
			vector3 origin;
			vector3 direction;

			ray() noexcept;
			ray(const vector3& _Origin, const vector3& _Direction) noexcept;
			vector3 get_point(float t) const;
			vector3 operator *(float t) const;
			bool intersects_plane(const vector3& normal, float diameter) const;
			bool intersects_sphere(const vector3& position, float radius, bool discard_inside = true) const;
			bool intersects_aabb_at(const vector3& min, const vector3& max, vector3* hit) const;
			bool intersects_aabb(const vector3& position, const vector3& scale, vector3* hit) const;
			bool intersects_obb(const matrix4x4& world, vector3* hit) const;
		};

		struct frustum8c
		{
			vector4 corners[8];

			frustum8c() noexcept;
			frustum8c(float field_of_view, float aspect, float near_z, float far_z) noexcept;
			void transform(const matrix4x4& value);
			void get_bounding_box(float size, vector2* x, vector2* y, vector2* z);
		};

		struct frustum6p
		{
			enum class side : size_t
			{
				RIGHT = 0,
				LEFT = 1,
				BOTTOM = 2,
				TOP = 3,
				BACK = 4,
				FRONT = 5
			};

			vector4 planes[6];

			frustum6p() noexcept;
			frustum6p(const matrix4x4& view_projection) noexcept;
			bool overlaps_aabb(const bounding& bounds) const;
			bool overlaps_sphere(const vector3& center, float radius) const;

		private:
			void normalize_plane(vector4& plane);
		};

		struct joint
		{
			core::vector<joint> childs;
			core::string name;
			matrix4x4 global;
			matrix4x4 local;
			size_t index;
		};

		struct animator_key
		{
			vector3 position = 0.0f;
			quaternion rotation;
			vector3 scale = 1.0f;
			float time = 1.0f;
		};

		struct skin_animator_key
		{
			core::vector<animator_key> pose;
			float time;
		};

		struct skin_animator_clip
		{
			core::vector<skin_animator_key> keys;
			core::string name;
			float duration = 1.0f;
			float rate = 1.0f;
		};

		struct key_animator_clip
		{
			core::vector<animator_key> keys;
			core::string name;
			float duration = 1.0f;
			float rate = 1.0f;
		};

		struct random_vector2
		{
			vector2 min, max;
			bool intensity;
			float accuracy;

			random_vector2() noexcept;
			random_vector2(const vector2& min_v, const vector2& max_v, bool intensity_v, float accuracy_v) noexcept;
			vector2 generate();
		};

		struct random_vector3
		{
			vector3 min, max;
			bool intensity;
			float accuracy;

			random_vector3() noexcept;
			random_vector3(const vector3& min_v, const vector3& max_v, bool intensity_v, float accuracy_v) noexcept;
			vector3 generate();
		};

		struct random_vector4
		{
			vector4 min, max;
			bool intensity;
			float accuracy;

			random_vector4() noexcept;
			random_vector4(const vector4& min_v, const vector4& max_v, bool intensity_v, float accuracy_v) noexcept;
			vector4 generate();
		};

		struct random_float
		{
			float min, max;
			bool intensity;
			float accuracy;

			random_float() noexcept;
			random_float(float min_v, float max_v, bool intensity_v, float accuracy_v) noexcept;
			float generate();
		};

		struct adj_triangle
		{
			uint32_t vref[3];
			uint32_t atri[3];

			uint8_t find_edge(uint32_t vref0, uint32_t vref1);
			uint32_t opposite_vertex(uint32_t vref0, uint32_t vref1);
		};

		struct adj_edge
		{
			uint32_t ref0;
			uint32_t ref1;
			uint32_t face_nb;
		};

		class adjacencies
		{
		public:
			struct desc
			{
				uint32_t nb_faces = 0;
				uint32_t* faces = nullptr;
			};

		private:
			uint32_t nb_edges;
			uint32_t current_nb_faces;
			adj_edge* edges;

		public:
			uint32_t nb_faces;
			adj_triangle* faces;

		public:
			adjacencies() noexcept;
			~adjacencies() noexcept;
			bool fill(adjacencies::desc& i);
			bool resolve();

		private:
			bool add_triangle(uint32_t ref0, uint32_t ref1, uint32_t ref2);
			bool add_edge(uint32_t ref0, uint32_t ref1, uint32_t face);
			bool update_link(uint32_t first_tri, uint32_t second_tri, uint32_t ref0, uint32_t ref1);
		};

		class triangle_strip
		{
		public:
			struct desc
			{
				uint32_t* faces = nullptr;
				uint32_t nb_faces = 0;
				bool one_sided = true;
				bool sgi_cipher = true;
				bool connect_all_strips = false;
			};

			struct result
			{
				core::vector<uint32_t> strips;
				core::vector<uint32_t> groups;

				core::vector<int> get_indices(int group = -1);
				core::vector<int> get_inv_indices(int group = -1);
			};

		private:
			core::vector<uint32_t> single_strip;
			core::vector<uint32_t> strip_lengths;
			core::vector<uint32_t> strip_runs;
			adjacencies* adj;
			bool* tags;
			uint32_t nb_strips;
			uint32_t total_length;
			bool one_sided;
			bool sgi_cipher;
			bool connect_all_strips;

		public:
			triangle_strip() noexcept;
			~triangle_strip() noexcept;
			bool fill(const triangle_strip::desc& i);
			bool resolve(triangle_strip::result& result);

		private:
			triangle_strip& free_buffers();
			uint32_t compute_strip(uint32_t face);
			uint32_t track_strip(uint32_t face, uint32_t oldest, uint32_t middle, uint32_t* strip, uint32_t* faces, bool* tags);
			bool connect_strips(triangle_strip::result& result);
		};

		class radix_sorter
		{
		private:
			uint32_t* histogram;
			uint32_t* offset;
			uint32_t current_size;
			uint32_t* indices;
			uint32_t* indices2;

		public:
			radix_sorter() noexcept;
			radix_sorter(const radix_sorter& other) noexcept;
			radix_sorter(radix_sorter&& other) noexcept;
			~radix_sorter() noexcept;
			radix_sorter& operator =(const radix_sorter& v);
			radix_sorter& operator =(radix_sorter&& v) noexcept;
			radix_sorter& sort(uint32_t* input, uint32_t nb, bool signed_values = true);
			radix_sorter& sort(float* input, uint32_t nb);
			radix_sorter& reset_indices();
			uint32_t* get_indices();
		};

		class geometric
		{
		private:
			static bool left_handed;

		public:
			static bool is_cube_in_frustum(const matrix4x4& world_view_projection, float radius);
			static bool is_left_handed();
			static bool has_sphere_intersected(const vector3& position_r0, float radius_r0, const vector3& position_r1, float radius_r1);
			static bool has_line_intersected(float distance_f, float distance_d, const vector3& start, const vector3& end, vector3& hit);
			static bool has_line_intersected_cube(const vector3& min, const vector3& max, const vector3& start, const vector3& end);
			static bool has_point_intersected_cube(const vector3& hit, const vector3& position, const vector3& scale, int axis);
			static bool has_point_intersected_rectangle(const vector3& position, const vector3& scale, const vector3& P0);
			static bool has_point_intersected_cube(const vector3& position, const vector3& scale, const vector3& P0);
			static bool has_sb_intersected(transform* box_r0, transform* box_r1);
			static bool has_obb_intersected(transform* box_r0, transform* box_r1);
			static bool has_aabb_intersected(transform* box_r0, transform* box_r1);
			static void flip_index_winding_order(core::vector<int>& indices);
			static void matrix_rh_to_lh(matrix4x4* matrix);
			static void set_left_handed(bool is_left_handed);
			static core::vector<int> create_triangle_strip(triangle_strip::desc& desc, const core::vector<int>& indices);
			static core::vector<int> create_triangle_list(const core::vector<int>& indices);
			static void create_frustum8c_rad(vector4* result8, float field_of_view, float aspect, float near_z, float far_z);
			static void create_frustum8c(vector4* result8, float field_of_view, float aspect, float near_z, float far_z);
			static ray create_cursor_ray(const vector3& origin, const vector2& cursor, const vector2& screen, const matrix4x4& inv_projection, const matrix4x4& inv_view);
			static bool cursor_ray_test(const ray& cursor, const vector3& position, const vector3& scale, vector3* hit = nullptr);
			static bool cursor_ray_test(const ray& cursor, const matrix4x4& world, vector3* hit = nullptr);
			static float fast_inv_sqrt(float value);
			static float fast_sqrt(float value);
			static float aabb_volume(const vector3& min, const vector3& max);
			static float angluar_lerp(float a, float b, float delta_time);
			static float angle_distance(float a, float b);

		public:
			template <typename t>
			static void texcoord_rh_to_lh(core::vector<t>& vertices, bool always = false)
			{
				if (is_left_handed() || always)
					return;

				for (auto& item : vertices)
					item.texcoord_y = 1.0f - item.texcoord_y;
			}
		};

		class transform final : public core::reference<transform>
		{
			friend geometric;

		public:
			struct spacing
			{
				matrix4x4 offset;
				vector3 position;
				vector3 rotation;
				vector3 scale = 1.0f;
			};

		private:
			core::task_callback on_dirty;
			core::vector<transform*> childs;
			matrix4x4 temporary;
			transform* root;
			spacing* local;
			spacing global;
			bool scaling;
			bool dirty;

		public:
			void* user_data;

		public:
			transform(void* new_user_data) noexcept;
			~transform() noexcept;
			void synchronize();
			void move(const vector3& value);
			void rotate(const vector3& value);
			void rescale(const vector3& value);
			void localize(spacing& where);
			void globalize(spacing& where);
			void specialize(spacing& where);
			void copy(transform* target);
			void add_child(transform* child);
			void remove_child(transform* child);
			void remove_childs();
			void when_dirty(core::task_callback&& callback);
			void make_dirty();
			void set_scaling(bool enabled);
			void set_position(const vector3& value);
			void set_rotation(const vector3& value);
			void set_scale(const vector3& value);
			void set_spacing(positioning space, spacing& where);
			void set_pivot(transform* root, spacing* pivot);
			void set_root(transform* root);
			void get_bounds(matrix4x4& world, vector3& min, vector3& max);
			bool has_root(const transform* target) const;
			bool has_child(transform* target) const;
			bool has_scaling() const;
			bool is_dirty() const;
			const matrix4x4& get_bias() const;
			const matrix4x4& get_bias_unscaled() const;
			const vector3& get_position() const;
			const vector3& get_rotation() const;
			const vector3& get_scale() const;
			vector3 forward() const;
			vector3 right() const;
			vector3 up() const;
			spacing& get_spacing();
			spacing& get_spacing(positioning space);
			transform* get_root() const;
			transform* get_upper_root() const;
			transform* get_child(size_t child) const;
			size_t get_childs_count() const;
			core::vector<transform*>& get_childs();

		protected:
			bool can_root_be_applied(transform* root) const;
		};

		class cosmos
		{
		public:
			typedef core::vector<size_t> iterator;

		public:
			struct node
			{
				bounding bounds;
				size_t parent = 0;
				size_t next = 0;
				size_t left = 0;
				size_t right = 0;
				void* item = nullptr;
				int height = 0;

				bool is_leaf() const;
			};

		private:
			core::unordered_map<void*, size_t> items;
			core::vector<node> nodes;
			size_t root;
			size_t node_count;
			size_t node_capacity;
			size_t free_list;

		public:
			cosmos(size_t default_size = 16) noexcept;
			void reserve(size_t size);
			void clear();
			void remove_item(void* item);
			void insert_item(void* item, const vector3& lower_bound, const vector3& upper_bound);
			bool update_item(void* item, const vector3& lower_bound, const vector3& upper_bound, bool always = false);
			const bounding& get_area(void* item);
			const core::unordered_map<void*, size_t>& get_items() const;
			const core::vector<node>& get_nodes() const;
			size_t get_nodes_count() const;
			size_t get_height() const;
			size_t get_max_balance() const;
			size_t get_root() const;
			const node& get_root_node() const;
			const node& get_node(size_t id) const;
			float get_volume_ratio() const;
			bool is_null(size_t id) const;
			bool empty() const;

		private:
			size_t allocate_node();
			void free_node(size_t);
			void insert_leaf(size_t);
			void remove_leaf(size_t);
			size_t balance(size_t);
			size_t compute_height() const;
			size_t compute_height(size_t) const;

		public:
			template <typename t, typename overlaps_function, typename match_function>
			void query_index(iterator& context, overlaps_function&& overlaps, match_function&& match)
			{
				context.clear();
				if (!items.empty())
					context.push_back(root);

				while (!context.empty())
				{
					auto& next = nodes[context.back()];
					context.pop_back();

					if (overlaps(next.bounds))
					{
						if (!next.is_leaf())
						{
							context.push_back(next.left);
							context.push_back(next.right);
						}
						else if (next.item != nullptr)
							match((t*)next.item);
					}
				}
			}
		};
	}
}
#endif