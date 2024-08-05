#ifndef VI_TRIGONOMETRY_H
#define VI_TRIGONOMETRY_H
#include <vitex/compute.h>

namespace Vitex
{
	namespace Trigonometry
	{
		class Transform;

		struct Matrix4x4;

		struct Quaternion;

		struct Vector2;

		struct Vector3;

		struct Vector4;

		enum class Rotator
		{
			XYZ = 0,
			XZY,
			YXZ,
			YZX,
			ZXY,
			ZYX
		};

		enum class Positioning
		{
			Local,
			Global
		};

		enum class CubeFace
		{
			PositiveX,
			NegativeX,
			PositiveY,
			NegativeY,
			PositiveZ,
			NegativeZ
		};

		struct VI_OUT Vertex
		{
			float PositionX;
			float PositionY;
			float PositionZ;
			float TexCoordX;
			float TexCoordY;
			float NormalX;
			float NormalY;
			float NormalZ;
			float TangentX;
			float TangentY;
			float TangentZ;
			float BitangentX;
			float BitangentY;
			float BitangentZ;
		};

		struct VI_OUT SkinVertex
		{
			float PositionX;
			float PositionY;
			float PositionZ;
			float TexCoordX;
			float TexCoordY;
			float NormalX;
			float NormalY;
			float NormalZ;
			float TangentX;
			float TangentY;
			float TangentZ;
			float BitangentX;
			float BitangentY;
			float BitangentZ;
			float JointIndex0;
			float JointIndex1;
			float JointIndex2;
			float JointIndex3;
			float JointBias0;
			float JointBias1;
			float JointBias2;
			float JointBias3;
		};

		struct VI_OUT ShapeVertex
		{
			float PositionX;
			float PositionY;
			float PositionZ;
			float TexCoordX;
			float TexCoordY;
		};

		struct VI_OUT ElementVertex
		{
			float PositionX;
			float PositionY;
			float PositionZ;
			float Scale;
			float VelocityX;
			float VelocityY;
			float VelocityZ;
			float Rotation;
			float Padding1;
			float Padding2;
			float Padding3;
			float Angular;
			float ColorX;
			float ColorY;
			float ColorZ;
			float ColorW;
		};

		struct VI_OUT Vector2
		{
			float X;
			float Y;

			Vector2() noexcept;
			Vector2(float x, float y) noexcept;
			Vector2(float xy) noexcept;
			Vector2(const Vector2& Value) noexcept;
			Vector2(const Vector3& Value) noexcept;
			Vector2(const Vector4& Value) noexcept;
			bool IsEquals(const Vector2& Other, float MaxDisplacement = 0.000001f) const;
			float Length() const;
			float Sum() const;
			float Dot(const Vector2& B) const;
			float Distance(const Vector2& Target) const;
			float Hypotenuse() const;
			float LookAt(const Vector2& At) const;
			float Cross(const Vector2& Vector1) const;
			Vector2 Transform(const Matrix4x4& V) const;
			Vector2 Direction(float Rotation) const;
			Vector2 Inv() const;
			Vector2 InvX() const;
			Vector2 InvY() const;
			Vector2 Normalize() const;
			Vector2 sNormalize() const;
			Vector2 Lerp(const Vector2& B, float DeltaTime) const;
			Vector2 sLerp(const Vector2& B, float DeltaTime) const;
			Vector2 aLerp(const Vector2& B, float DeltaTime) const;
			Vector2 rLerp() const;
			Vector2 Abs() const;
			Vector2 Radians() const;
			Vector2 Degrees() const;
			Vector2 XY() const;
			Vector3 XYZ() const;
			Vector4 XYZW() const;
			Vector2 Mul(float xy) const;
			Vector2 Mul(float X, float Y) const;
			Vector2 Mul(const Vector2& Value) const;
			Vector2 Div(const Vector2& Value) const;
			Vector2 Add(const Vector2& Value) const;
			Vector2 SetX(float X) const;
			Vector2 SetY(float Y) const;
			void Set(const Vector2& Value);
			void Get2(float* In) const;
			Vector2& operator *=(const Vector2& V);
			Vector2& operator *=(float V);
			Vector2& operator /=(const Vector2& V);
			Vector2& operator /=(float V);
			Vector2& operator +=(const Vector2& V);
			Vector2& operator +=(float V);
			Vector2& operator -=(const Vector2& V);
			Vector2& operator -=(float V);
			Vector2 operator *(const Vector2& V) const;
			Vector2 operator *(float V) const;
			Vector2 operator /(const Vector2& V) const;
			Vector2 operator /(float V) const;
			Vector2 operator +(const Vector2& V) const;
			Vector2 operator +(float V) const;
			Vector2 operator -(const Vector2& V) const;
			Vector2 operator -(float V) const;
			Vector2 operator -() const;
			Vector2& operator =(const Vector2& V) noexcept;
			bool operator ==(const Vector2& V) const;
			bool operator !=(const Vector2& V) const;
			bool operator <=(const Vector2& V) const;
			bool operator >=(const Vector2& V) const;
			bool operator >(const Vector2& V) const;
			bool operator <(const Vector2& V) const;
			float& operator [](uint32_t Axis);
			float operator [](uint32_t Axis) const;

			static Vector2 Random();
			static Vector2 RandomAbs();
			static Vector2 One()
			{
				return Vector2(1, 1);
			}
			static Vector2 Zero()
			{
				return Vector2(0, 0);
			}
			static Vector2 Up()
			{
				return Vector2(0, 1);
			}
			static Vector2 Down()
			{
				return Vector2(0, -1);
			}
			static Vector2 Left()
			{
				return Vector2(-1, 0);
			}
			static Vector2 Right()
			{
				return Vector2(1, 0);
			}
		};

		struct VI_OUT Vector3
		{
			float X;
			float Y;
			float Z;

			Vector3() noexcept;
			Vector3(const Vector2& Value) noexcept;
			Vector3(const Vector3& Value) noexcept;
			Vector3(const Vector4& Value) noexcept;
			Vector3(float x, float y) noexcept;
			Vector3(float x, float y, float z) noexcept;
			Vector3(float xyz) noexcept;
			bool IsEquals(const Vector3& Other, float MaxDisplacement = 0.000001f) const;
			float Length() const;
			float Sum() const;
			float Dot(const Vector3& B) const;
			float Distance(const Vector3& Target) const;
			float Hypotenuse() const;
			Vector3 LookAt(const Vector3& Vector) const;
			Vector3 Cross(const Vector3& Vector) const;
			Vector3 Transform(const Matrix4x4& V) const;
			Vector3 hDirection() const;
			Vector3 dDirection() const;
			Vector3 Direction() const;
			Vector3 Inv() const;
			Vector3 InvX() const;
			Vector3 InvY() const;
			Vector3 InvZ() const;
			Vector3 Normalize() const;
			Vector3 sNormalize() const;
			Vector3 Lerp(const Vector3& B, float DeltaTime) const;
			Vector3 sLerp(const Vector3& B, float DeltaTime) const;
			Vector3 aLerp(const Vector3& B, float DeltaTime) const;
			Vector3 rLerp() const;
			Vector3 Abs() const;
			Vector3 Radians() const;
			Vector3 Degrees() const;
			Vector3 ViewSpace() const;
			Vector2 XY() const;
			Vector3 XYZ() const;
			Vector4 XYZW() const;
			Vector3 Mul(float xyz) const;
			Vector3 Mul(const Vector2& XY, float Z) const;
			Vector3 Mul(const Vector3& Value) const;
			Vector3 Div(const Vector3& Value) const;
			Vector3 Add(const Vector3& Value) const;
			Vector3 SetX(float X) const;
			Vector3 SetY(float Y) const;
			Vector3 SetZ(float Z) const;
			Vector3 Rotate(const Vector3& Origin, const Vector3& Rotation);
			void Set(const Vector3& Value);
			void Get2(float* In) const;
			void Get3(float* In) const;
			Vector3& operator *=(const Vector3& V);
			Vector3& operator *=(float V);
			Vector3& operator /=(const Vector3& V);
			Vector3& operator /=(float V);
			Vector3& operator +=(const Vector3& V);
			Vector3& operator +=(float V);
			Vector3& operator -=(const Vector3& V);
			Vector3& operator -=(float V);
			Vector3 operator *(const Vector3& V) const;
			Vector3 operator *(float V) const;
			Vector3 operator /(const Vector3& V) const;
			Vector3 operator /(float V) const;
			Vector3 operator +(const Vector3& V) const;
			Vector3 operator +(float V) const;
			Vector3 operator -(const Vector3& V) const;
			Vector3 operator -(float V) const;
			Vector3 operator -() const;
			Vector3& operator =(const Vector3& V) noexcept;
			bool operator ==(const Vector3& V) const;
			bool operator !=(const Vector3& V) const;
			bool operator <=(const Vector3& V) const;
			bool operator >=(const Vector3& V) const;
			bool operator >(const Vector3& V) const;
			bool operator <(const Vector3& V) const;
			float& operator [](uint32_t Axis);
			float operator [](uint32_t Axis) const;
			static Vector3 Random();
			static Vector3 RandomAbs();
			static Vector3 One()
			{
				return Vector3(1, 1, 1);
			}
			static Vector3 Zero()
			{
				return Vector3(0, 0, 0);
			}
			static Vector3 Up()
			{
				return Vector3(0, 1, 0);
			}
			static Vector3 Down()
			{
				return Vector3(0, -1, 0);
			}
			static Vector3 Left()
			{
				return Vector3(-1, 0, 0);
			}
			static Vector3 Right()
			{
				return Vector3(1, 0, 0);
			}
			static Vector3 Forward()
			{
				return Vector3(0, 0, 1);
			}
			static Vector3 Backward()
			{
				return Vector3(0, 0, -1);
			}
		};

		struct VI_OUT Vector4
		{
			float X;
			float Y;
			float Z;
			float W;

			Vector4() noexcept;
			Vector4(const Vector2& Value) noexcept;
			Vector4(const Vector3& Value) noexcept;
			Vector4(const Vector4& Value) noexcept;
			Vector4(float x, float y) noexcept;
			Vector4(float x, float y, float z) noexcept;
			Vector4(float x, float y, float z, float w) noexcept;
			Vector4(float xyzw) noexcept;
			bool IsEquals(const Vector4& Other, float MaxDisplacement = 0.000001f) const;
			float Length() const;
			float Sum() const;
			float Dot(const Vector4& B) const;
			float Distance(const Vector4& Target) const;
			Vector4 Cross(const Vector4& Vector1) const;
			Vector4 Transform(const Matrix4x4& Matrix) const;
			Vector4 Inv() const;
			Vector4 InvX() const;
			Vector4 InvY() const;
			Vector4 InvZ() const;
			Vector4 InvW() const;
			Vector4 Normalize() const;
			Vector4 sNormalize() const;
			Vector4 Lerp(const Vector4& B, float DeltaTime) const;
			Vector4 sLerp(const Vector4& B, float DeltaTime) const;
			Vector4 aLerp(const Vector4& B, float DeltaTime) const;
			Vector4 rLerp() const;
			Vector4 Abs() const;
			Vector4 Radians() const;
			Vector4 Degrees() const;
			Vector4 ViewSpace() const;
			Vector2 XY() const;
			Vector3 XYZ() const;
			Vector4 XYZW() const;
			Vector4 Mul(float xyzw) const;
			Vector4 Mul(const Vector2& XY, float Z, float W) const;
			Vector4 Mul(const Vector3& XYZ, float W) const;
			Vector4 Mul(const Vector4& Value) const;
			Vector4 Div(const Vector4& Value) const;
			Vector4 Add(const Vector4& Value) const;
			Vector4 SetX(float X) const;
			Vector4 SetY(float Y) const;
			Vector4 SetZ(float Z) const;
			Vector4 SetW(float W) const;
			void Set(const Vector4& Value);
			void Get2(float* In) const;
			void Get3(float* In) const;
			void Get4(float* In) const;
			Vector4& operator *=(const Matrix4x4& V);
			Vector4& operator *=(const Vector4& V);
			Vector4& operator *=(float V);
			Vector4& operator /=(const Vector4& V);
			Vector4& operator /=(float V);
			Vector4& operator +=(const Vector4& V);
			Vector4& operator +=(float V);
			Vector4& operator -=(const Vector4& V);
			Vector4& operator -=(float V);
			Vector4 operator *(const Matrix4x4& V) const;
			Vector4 operator *(const Vector4& V) const;
			Vector4 operator *(float V) const;
			Vector4 operator /(const Vector4& V) const;
			Vector4 operator /(float V) const;
			Vector4 operator +(const Vector4& V) const;
			Vector4 operator +(float V) const;
			Vector4 operator -(const Vector4& V) const;
			Vector4 operator -(float V) const;
			Vector4 operator -() const;
			Vector4& operator =(const Vector4& V) noexcept;
			bool operator ==(const Vector4& V) const;
			bool operator !=(const Vector4& V) const;
			bool operator <=(const Vector4& V) const;
			bool operator >=(const Vector4& V) const;
			bool operator >(const Vector4& V) const;
			bool operator <(const Vector4& V) const;
			float& operator [](uint32_t Axis);
			float operator [](uint32_t Axis) const;

			static Vector4 Random();
			static Vector4 RandomAbs();
			static Vector4 One()
			{
				return Vector4(1, 1, 1, 1);
			};
			static Vector4 Zero()
			{
				return Vector4(0, 0, 0, 0);
			};
			static Vector4 Up()
			{
				return Vector4(0, 1, 0, 0);
			};
			static Vector4 Down()
			{
				return Vector4(0, -1, 0, 0);
			};
			static Vector4 Left()
			{
				return Vector4(-1, 0, 0, 0);
			};
			static Vector4 Right()
			{
				return Vector4(1, 0, 0, 0);
			};
			static Vector4 Forward()
			{
				return Vector4(0, 0, 1, 0);
			};
			static Vector4 Backward()
			{
				return Vector4(0, 0, -1, 0);
			};
			static Vector4 UnitW()
			{
				return Vector4(0, 0, 0, 1);
			};
		};

		struct VI_OUT Matrix4x4
		{
		public:
			float Row[16];

		public:
			Matrix4x4() noexcept;
			Matrix4x4(float Array[16]) noexcept;
			Matrix4x4(const Vector4& Row0, const Vector4& Row1, const Vector4& Row2, const Vector4& Row3) noexcept;
			Matrix4x4(float Row00, float Row01, float Row02, float Row03, float Row10, float Row11, float Row12, float Row13, float Row20, float Row21, float Row22, float Row23, float Row30, float Row31, float Row32, float Row33) noexcept;
			Matrix4x4(const Matrix4x4& Other) noexcept;
			float& operator [](uint32_t Index);
			float operator [](uint32_t Index) const;
			bool operator ==(const Matrix4x4& Index) const;
			bool operator !=(const Matrix4x4& Index) const;
			Matrix4x4 operator *(const Matrix4x4& V) const;
			Vector4 operator *(const Vector4& V) const;
			Matrix4x4& operator =(const Matrix4x4& V) noexcept;
			Matrix4x4 Mul(const Matrix4x4& Right) const;
			Matrix4x4 Mul(const Vector4& Right) const;
			Matrix4x4 Inv() const;
			Matrix4x4 Transpose() const;
			Matrix4x4 SetScale(const Vector3& Value) const;
			Vector4 Row11() const;
			Vector4 Row22() const;
			Vector4 Row33() const;
			Vector4 Row44() const;
			Vector3 Up() const;
			Vector3 Right() const;
			Vector3 Forward() const;
			Quaternion RotationQuaternion() const;
			Vector3 RotationEuler() const;
			Vector3 Position() const;
			Vector3 Scale() const;
			Vector2 XY() const;
			Vector3 XYZ() const;
			Vector4 XYZW() const;
			float Determinant() const;
			void Identify();
			void Set(const Matrix4x4& Value);

		private:
			Matrix4x4(bool) noexcept;

		public:
			static Matrix4x4 CreateLookAt(const Vector3& Position, const Vector3& Target, const Vector3& Up);
			static Matrix4x4 CreateRotationX(float Rotation);
			static Matrix4x4 CreateRotationY(float Rotation);
			static Matrix4x4 CreateRotationZ(float Rotation);
			static Matrix4x4 CreateScale(const Vector3& Scale);
			static Matrix4x4 CreateTranslatedScale(const Vector3& Position, const Vector3& Scale);
			static Matrix4x4 CreateTranslation(const Vector3& Position);
			static Matrix4x4 CreatePerspective(float FieldOfView, float AspectRatio, float NearZ, float FarZ);
			static Matrix4x4 CreatePerspectiveRad(float FieldOfView, float AspectRatio, float NearZ, float FarZ);
			static Matrix4x4 CreateOrthographic(float Width, float Height, float NearZ, float FarZ);
			static Matrix4x4 CreateOrthographicOffCenter(float Left, float Right, float Bottom, float Top, float NearZ, float FarZ);
			static Matrix4x4 Create(const Vector3& Position, const Vector3& Scale, const Vector3& Rotation);
			static Matrix4x4 Create(const Vector3& Position, const Vector3& Rotation);
			static Matrix4x4 CreateRotation(const Vector3& Rotation);
			static Matrix4x4 CreateView(const Vector3& Position, const Vector3& Rotation);
			static Matrix4x4 CreateLookAt(CubeFace Face, const Vector3& Position);
			static Matrix4x4 CreateRotation(const Vector3& Forward, const Vector3& Up, const Vector3& Right);
			static Matrix4x4 Identity()
			{
				return Matrix4x4(
					1, 0, 0, 0,
					0, 1, 0, 0,
					0, 0, 1, 0,
					0, 0, 0, 1);
			};
		};

		struct VI_OUT Quaternion
		{
			float X, Y, Z, W;

			Quaternion() noexcept;
			Quaternion(float x, float y, float z, float w) noexcept;
			Quaternion(const Quaternion& In) noexcept;
			Quaternion(const Vector3& Axis, float Angle) noexcept;
			Quaternion(const Vector3& Euler) noexcept;
			Quaternion(const Matrix4x4& Value) noexcept;
			void SetAxis(const Vector3& Axis, float Angle);
			void SetEuler(const Vector3& Euler);
			void SetMatrix(const Matrix4x4& Value);
			void Set(const Quaternion& Value);
			Quaternion operator *(float r) const;
			Vector3 operator *(const Vector3& r) const;
			Quaternion operator *(const Quaternion& r) const;
			Quaternion operator -(const Quaternion& r) const;
			Quaternion operator +(const Quaternion& r) const;
			Quaternion& operator =(const Quaternion& r) noexcept;
			Quaternion Normalize() const;
			Quaternion sNormalize() const;
			Quaternion Conjugate() const;
			Quaternion Mul(float r) const;
			Quaternion Mul(const Quaternion& r) const;
			Vector3 Mul(const Vector3& r) const;
			Quaternion Sub(const Quaternion& r) const;
			Quaternion Add(const Quaternion& r) const;
			Quaternion Lerp(const Quaternion& B, float DeltaTime) const;
			Quaternion sLerp(const Quaternion& B, float DeltaTime) const;
			Vector3 Forward() const;
			Vector3 Up() const;
			Vector3 Right() const;
			Matrix4x4 GetMatrix() const;
			Vector3 GetEuler() const;
			float Dot(const Quaternion& r) const;
			float Length() const;
			bool operator ==(const Quaternion& V) const;
			bool operator !=(const Quaternion& V) const;

			static Quaternion CreateEulerRotation(const Vector3& Euler);
			static Quaternion CreateRotation(const Matrix4x4& Transform);
		};

		struct VI_OUT Rectangle
		{
			int64_t Left;
			int64_t Top;
			int64_t Right;
			int64_t Bottom;

			int64_t GetX() const;
			int64_t GetY() const;
			int64_t GetWidth() const;
			int64_t GetHeight() const;
		};

		struct VI_OUT Bounding
		{
		public:
			Vector3 Lower;
			Vector3 Upper;
			Vector3 Center;
			float Radius;
			float Volume;

		public:
			Bounding() noexcept;
			Bounding(const Vector3&, const Vector3&) noexcept;
			void Merge(const Bounding&, const Bounding&);
			bool Contains(const Bounding&) const;
			bool Overlaps(const Bounding&) const;
		};

		struct VI_OUT Ray
		{
			Vector3 Origin;
			Vector3 Direction;

			Ray() noexcept;
			Ray(const Vector3& _Origin, const Vector3& _Direction) noexcept;
			Vector3 GetPoint(float T) const;
			Vector3 operator *(float T) const;
			bool IntersectsPlane(const Vector3& Normal, float Diameter) const;
			bool IntersectsSphere(const Vector3& Position, float Radius, bool DiscardInside = true) const;
			bool IntersectsAABBAt(const Vector3& Min, const Vector3& Max, Vector3* Hit) const;
			bool IntersectsAABB(const Vector3& Position, const Vector3& Scale, Vector3* Hit) const;
			bool IntersectsOBB(const Matrix4x4& World, Vector3* Hit) const;
		};

		struct VI_OUT Frustum8C
		{
			Vector4 Corners[8];

			Frustum8C() noexcept;
			Frustum8C(float FieldOfView, float Aspect, float NearZ, float FarZ) noexcept;
			void Transform(const Matrix4x4& Value);
			void GetBoundingBox(Vector2* X, Vector2* Y, Vector2* Z);
		};

		struct VI_OUT Frustum6P
		{
			enum class Side : size_t
			{
				RIGHT = 0,
				LEFT = 1,
				BOTTOM = 2,
				TOP = 3,
				BACK = 4,
				FRONT = 5
			};

			Vector4 Planes[6];

			Frustum6P() noexcept;
			Frustum6P(const Matrix4x4& ViewProjection) noexcept;
			bool OverlapsAABB(const Bounding& Bounds) const;
			bool OverlapsSphere(const Vector3& Center, float Radius) const;

		private:
			void NormalizePlane(Vector4& Plane);
		};

		struct VI_OUT Joint
		{
			Core::Vector<Joint> Childs;
			Core::String Name;
			Matrix4x4 Global;
			Matrix4x4 Local;
			size_t Index;
		};

		struct VI_OUT AnimatorKey
		{
			Vector3 Position = 0.0f;
			Quaternion Rotation;
			Vector3 Scale = 1.0f;
			float Time = 1.0f;
		};

		struct VI_OUT SkinAnimatorKey
		{
			Core::Vector<AnimatorKey> Pose;
			float Time;
		};

		struct VI_OUT SkinAnimatorClip
		{
			Core::Vector<SkinAnimatorKey> Keys;
			Core::String Name;
			float Duration = 1.0f;
			float Rate = 1.0f;
		};

		struct VI_OUT KeyAnimatorClip
		{
			Core::Vector<AnimatorKey> Keys;
			Core::String Name;
			float Duration = 1.0f;
			float Rate = 1.0f;
		};

		struct VI_OUT RandomVector2
		{
			Vector2 Min, Max;
			bool Intensity;
			float Accuracy;

			RandomVector2() noexcept;
			RandomVector2(const Vector2& MinV, const Vector2& MaxV, bool IntensityV, float AccuracyV) noexcept;
			Vector2 Generate();
		};

		struct VI_OUT RandomVector3
		{
			Vector3 Min, Max;
			bool Intensity;
			float Accuracy;

			RandomVector3() noexcept;
			RandomVector3(const Vector3& MinV, const Vector3& MaxV, bool IntensityV, float AccuracyV) noexcept;
			Vector3 Generate();
		};

		struct VI_OUT RandomVector4
		{
			Vector4 Min, Max;
			bool Intensity;
			float Accuracy;

			RandomVector4() noexcept;
			RandomVector4(const Vector4& MinV, const Vector4& MaxV, bool IntensityV, float AccuracyV) noexcept;
			Vector4 Generate();
		};

		struct VI_OUT RandomFloat
		{
			float Min, Max;
			bool Intensity;
			float Accuracy;

			RandomFloat() noexcept;
			RandomFloat(float MinV, float MaxV, bool IntensityV, float AccuracyV) noexcept;
			float Generate();
		};

		struct VI_OUT AdjTriangle
		{
			uint32_t VRef[3];
			uint32_t ATri[3];

			uint8_t FindEdge(uint32_t VRef0, uint32_t VRef1);
			uint32_t OppositeVertex(uint32_t VRef0, uint32_t VRef1);
		};

		struct VI_OUT AdjEdge
		{
			uint32_t Ref0;
			uint32_t Ref1;
			uint32_t FaceNb;
		};

		class VI_OUT Adjacencies
		{
		public:
			struct Desc
			{
				uint32_t NbFaces = 0;
				uint32_t* Faces = nullptr;
			};

		private:
			uint32_t NbEdges;
			uint32_t CurrentNbFaces;
			AdjEdge* Edges;

		public:
			uint32_t NbFaces;
			AdjTriangle* Faces;

		public:
			Adjacencies() noexcept;
			~Adjacencies() noexcept;
			bool Fill(Adjacencies::Desc& I);
			bool Resolve();

		private:
			bool AddTriangle(uint32_t Ref0, uint32_t Ref1, uint32_t Ref2);
			bool AddEdge(uint32_t Ref0, uint32_t Ref1, uint32_t Face);
			bool UpdateLink(uint32_t FirstTri, uint32_t SecondTri, uint32_t Ref0, uint32_t Ref1);
		};

		class VI_OUT TriangleStrip
		{
		public:
			struct Desc
			{
				uint32_t* Faces = nullptr;
				uint32_t NbFaces = 0;
				bool OneSided = true;
				bool SGICipher = true;
				bool ConnectAllStrips = false;
			};

			struct Result
			{
				Core::Vector<uint32_t> Strips;
				Core::Vector<uint32_t> Groups;

				Core::Vector<int> GetIndices(int Group = -1);
				Core::Vector<int> GetInvIndices(int Group = -1);
			};

		private:
			Core::Vector<uint32_t> SingleStrip;
			Core::Vector<uint32_t> StripLengths;
			Core::Vector<uint32_t> StripRuns;
			Adjacencies* Adj;
			bool* Tags;
			uint32_t NbStrips;
			uint32_t TotalLength;
			bool OneSided;
			bool SGICipher;
			bool ConnectAllStrips;

		public:
			TriangleStrip() noexcept;
			~TriangleStrip() noexcept;
			bool Fill(const TriangleStrip::Desc& I);
			bool Resolve(TriangleStrip::Result& Result);

		private:
			TriangleStrip& FreeBuffers();
			uint32_t ComputeStrip(uint32_t Face);
			uint32_t TrackStrip(uint32_t Face, uint32_t Oldest, uint32_t Middle, uint32_t* Strip, uint32_t* Faces, bool* Tags);
			bool ConnectStrips(TriangleStrip::Result& Result);
		};

		class VI_OUT RadixSorter
		{
		private:
			uint32_t* Histogram;
			uint32_t* Offset;
			uint32_t CurrentSize;
			uint32_t* Indices;
			uint32_t* Indices2;

		public:
			RadixSorter() noexcept;
			RadixSorter(const RadixSorter& Other) noexcept;
			RadixSorter(RadixSorter&& Other) noexcept;
			~RadixSorter() noexcept;
			RadixSorter& operator =(const RadixSorter& V);
			RadixSorter& operator =(RadixSorter&& V) noexcept;
			RadixSorter& Sort(uint32_t* Input, uint32_t Nb, bool SignedValues = true);
			RadixSorter& Sort(float* Input, uint32_t Nb);
			RadixSorter& ResetIndices();
			uint32_t* GetIndices();
		};

		class VI_OUT_TS Geometric
		{
		private:
			static bool LeftHanded;

		public:
			static bool IsCubeInFrustum(const Matrix4x4& WorldViewProjection, float Radius);
			static bool IsLeftHanded();
			static bool HasSphereIntersected(const Vector3& PositionR0, float RadiusR0, const Vector3& PositionR1, float RadiusR1);
			static bool HasLineIntersected(float DistanceF, float DistanceD, const Vector3& Start, const Vector3& End, Vector3& Hit);
			static bool HasLineIntersectedCube(const Vector3& Min, const Vector3& Max, const Vector3& Start, const Vector3& End);
			static bool HasPointIntersectedCube(const Vector3& Hit, const Vector3& Position, const Vector3& Scale, int Axis);
			static bool HasPointIntersectedRectangle(const Vector3& Position, const Vector3& Scale, const Vector3& P0);
			static bool HasPointIntersectedCube(const Vector3& Position, const Vector3& Scale, const Vector3& P0);
			static bool HasSBIntersected(Transform* BoxR0, Transform* BoxR1);
			static bool HasOBBIntersected(Transform* BoxR0, Transform* BoxR1);
			static bool HasAABBIntersected(Transform* BoxR0, Transform* BoxR1);
			static void FlipIndexWindingOrder(Core::Vector<int>& Indices);
			static void MatrixRhToLh(Matrix4x4* Matrix);
			static void SetLeftHanded(bool IsLeftHanded);
			static Core::Vector<int> CreateTriangleStrip(TriangleStrip::Desc& Desc, const Core::Vector<int>& Indices);
			static Core::Vector<int> CreateTriangleList(const Core::Vector<int>& Indices);
			static void CreateFrustum8CRad(Vector4* Result8, float FieldOfView, float Aspect, float NearZ, float FarZ);
			static void CreateFrustum8C(Vector4* Result8, float FieldOfView, float Aspect, float NearZ, float FarZ);
			static Ray CreateCursorRay(const Vector3& Origin, const Vector2& Cursor, const Vector2& Screen, const Matrix4x4& InvProjection, const Matrix4x4& InvView);
			static bool CursorRayTest(const Ray& Cursor, const Vector3& Position, const Vector3& Scale, Vector3* Hit = nullptr);
			static bool CursorRayTest(const Ray& Cursor, const Matrix4x4& World, Vector3* Hit = nullptr);
			static float FastInvSqrt(float Value);
			static float FastSqrt(float Value);
			static float AabbVolume(const Vector3& Min, const Vector3& Max);

		public:
			template <typename T>
			static void TexCoordRhToLh(Core::Vector<T>& Vertices, bool Always = false)
			{
				if (IsLeftHanded() || Always)
					return;

				for (auto& Item : Vertices)
					Item.TexCoordY = 1.0f - Item.TexCoordY;
			}
		};

		class VI_OUT Transform final : public Core::Reference<Transform>
		{
			friend Geometric;

		public:
			struct Spacing
			{
				Matrix4x4 Offset;
				Vector3 Position;
				Vector3 Rotation;
				Vector3 Scale = 1.0f;
			};

		private:
			Core::TaskCallback OnDirty;
			Core::Vector<Transform*> Childs;
			Matrix4x4 Temporary;
			Transform* Root;
			Spacing* Local;
			Spacing Global;
			bool Scaling;
			bool Dirty;

		public:
			void* UserData;

		public:
			Transform(void* NewUserData) noexcept;
			~Transform() noexcept;
			void Synchronize();
			void Move(const Vector3& Value);
			void Rotate(const Vector3& Value);
			void Rescale(const Vector3& Value);
			void Localize(Spacing& Where);
			void Globalize(Spacing& Where);
			void Specialize(Spacing& Where);
			void Copy(Transform* Target);
			void AddChild(Transform* Child);
			void RemoveChild(Transform* Child);
			void RemoveChilds();
			void WhenDirty(Core::TaskCallback&& Callback);
			void MakeDirty();
			void SetScaling(bool Enabled);
			void SetPosition(const Vector3& Value);
			void SetRotation(const Vector3& Value);
			void SetScale(const Vector3& Value);
			void SetSpacing(Positioning Space, Spacing& Where);
			void SetPivot(Transform* Root, Spacing* Pivot);
			void SetRoot(Transform* Root);
			void GetBounds(Matrix4x4& World, Vector3& Min, Vector3& Max);
			bool HasRoot(const Transform* Target) const;
			bool HasChild(Transform* Target) const;
			bool HasScaling() const;
			bool IsDirty() const;
			const Matrix4x4& GetBias() const;
			const Matrix4x4& GetBiasUnscaled() const;
			const Vector3& GetPosition() const;
			const Vector3& GetRotation() const;
			const Vector3& GetScale() const;
			Vector3 Forward() const;
			Vector3 Right() const;
			Vector3 Up() const;
			Spacing& GetSpacing();
			Spacing& GetSpacing(Positioning Space);
			Transform* GetRoot() const;
			Transform* GetUpperRoot() const;
			Transform* GetChild(size_t Child) const;
			size_t GetChildsCount() const;
			Core::Vector<Transform*>& GetChilds();

		protected:
			bool CanRootBeApplied(Transform* Root) const;
		};

		class VI_OUT Cosmos
		{
		public:
			typedef Core::Vector<size_t> Iterator;

		public:
			struct VI_OUT Node
			{
				Bounding Bounds;
				size_t Parent = 0;
				size_t Next = 0;
				size_t Left = 0;
				size_t Right = 0;
				void* Item = nullptr;
				int Height = 0;

				bool IsLeaf() const;
			};

		private:
			Core::UnorderedMap<void*, size_t> Items;
			Core::Vector<Node> Nodes;
			size_t Root;
			size_t NodeCount;
			size_t NodeCapacity;
			size_t FreeList;

		public:
			Cosmos(size_t DefaultSize = 16) noexcept;
			void Reserve(size_t Size);
			void Clear();
			void RemoveItem(void* Item);
			void InsertItem(void* Item, const Vector3& LowerBound, const Vector3& UpperBound);
			bool UpdateItem(void* Item, const Vector3& LowerBound, const Vector3& UpperBound, bool Always = false);
			const Bounding& GetArea(void* Item);
			const Core::UnorderedMap<void*, size_t>& GetItems() const;
			const Core::Vector<Node>& GetNodes() const;
			size_t GetNodesCount() const;
			size_t GetHeight() const;
			size_t GetMaxBalance() const;
			size_t GetRoot() const;
			const Node& GetRootNode() const;
			const Node& GetNode(size_t Id) const;
			float GetVolumeRatio() const;
			bool IsNull(size_t Id) const;
			bool Empty() const;

		private:
			size_t AllocateNode();
			void FreeNode(size_t);
			void InsertLeaf(size_t);
			void RemoveLeaf(size_t);
			size_t Balance(size_t);
			size_t ComputeHeight() const;
			size_t ComputeHeight(size_t) const;

		public:
			template <typename T, typename OverlapsFunction, typename MatchFunction>
			void QueryIndex(Iterator& Context, OverlapsFunction&& Overlaps, MatchFunction&& Match)
			{
				Context.clear();
				if (!Items.empty())
					Context.push_back(Root);

				while (!Context.empty())
				{
					auto& Next = Nodes[Context.back()];
					Context.pop_back();

					if (Overlaps(Next.Bounds))
					{
						if (!Next.IsLeaf())
						{
							Context.push_back(Next.Left);
							Context.push_back(Next.Right);
						}
						else if (Next.Item != nullptr)
							Match((T*)Next.Item);
					}
				}
			}
		};
	}
}
#endif