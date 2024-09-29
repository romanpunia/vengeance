#include "trigonometry.h"
#ifdef VI_VECTORCLASS
#include "internal/vectorclass.hpp"
#endif
#define MAKE_ADJ_TRI(x) ((x) & 0x3fffffff)
#define IS_BOUNDARY(x) ((x) == 0xff)
#define RH_TO_LH (Matrix4x4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, -1, 0, 0, 0, 0, 1))
#define NULL_NODE ((size_t)-1)

namespace
{
	size_t OffsetOf64(const char* Source, char Dest)
	{
		VI_ASSERT(Source != nullptr, "source should be set");
		for (size_t i = 0; i < 64; i++)
		{
			if (Source[i] == Dest)
				return i;
		}

		return 63;
	}
	Vitex::Core::String EscapeText(const Vitex::Core::String& Data)
	{
		Vitex::Core::String Result = "\"";
		Result.append(Data).append("\"");
		return Result;
	}
}

namespace Vitex
{
	namespace Trigonometry
	{
		int64_t Rectangle::GetX() const
		{
			return Left;
		}
		int64_t Rectangle::GetY() const
		{
			return Top;
		}
		int64_t Rectangle::GetWidth() const
		{
			return Right - Left;
		}
		int64_t Rectangle::GetHeight() const
		{
			return Bottom - Top;
		}

		Vector2::Vector2() noexcept : X(0.0f), Y(0.0f)
		{
		}
		Vector2::Vector2(float x, float y) noexcept : X(x), Y(y)
		{
		}
		Vector2::Vector2(float xy) noexcept : X(xy), Y(xy)
		{
		}
		Vector2::Vector2(const Vector2& Value) noexcept : X(Value.X), Y(Value.Y)
		{
		}
		Vector2::Vector2(const Vector3& Value) noexcept : X(Value.X), Y(Value.Y)
		{
		}
		Vector2::Vector2(const Vector4& Value) noexcept : X(Value.X), Y(Value.Y)
		{
		}
		bool Vector2::IsEquals(const Vector2& Other, float MaxDisplacement) const
		{
			return abs(X - Other.X) <= MaxDisplacement && abs(Y - Other.Y) <= MaxDisplacement;
		}
		float Vector2::Length() const
		{
#ifdef VI_VECTORCLASS
			LOAD_FV2(_r1);
			return std::sqrt(horizontal_add(square(_r1)));
#else
			return std::sqrt(X * X + Y * Y);
#endif
		}
		float Vector2::Sum() const
		{
#ifdef VI_VECTORCLASS
			LOAD_FV2(_r1);
			return horizontal_add(_r1);
#else
			return X + Y;
#endif
		}
		float Vector2::Dot(const Vector2& B) const
		{
#ifdef VI_VECTORCLASS
			LOAD_FV2(_r1); LOAD_V2(_r2, B);
			return horizontal_add(_r1 * _r2);
#else
			return X * B.X + Y * B.Y;
#endif
		}
		float Vector2::Distance(const Vector2& Point) const
		{
#ifdef VI_VECTORCLASS
			LOAD_FV2(_r1); LOAD_V2(_r2, Point);
			return Geometric::FastSqrt(horizontal_add(square(_r1 - _r2)));
#else
			float X1 = X - Point.X, Y1 = Y - Point.Y;
			return Geometric::FastSqrt(X1 * X1 + Y1 * Y1);
#endif
		}
		float Vector2::Hypotenuse() const
		{
			return Length();
		}
		float Vector2::LookAt(const Vector2& At) const
		{
#ifdef VI_VECTORCLASS
			LOAD_FV2(_r1); LOAD_V2(_r2, At); _r1 = _r2 - _r1;
			return atan2f(_r1.extract(0), _r1.extract(1));
#else
			return atan2f(At.X - X, At.Y - Y);
#endif
		}
		float Vector2::Cross(const Vector2& B) const
		{
#ifdef VI_VECTORCLASS
			LOAD_FV2(_r1); LOAD_V2(_r2, B); _r1 = _r1 * _r2;
			return _r1.extract(0) - _r1.extract(1);
#else
			return X * B.Y - Y * B.X;
#endif
		}
		Vector2 Vector2::Transform(const Matrix4x4& Matrix) const
		{
#ifdef VI_VECTORCLASS
			LOAD_VAL(_r1, X);
			LOAD_VAL(_r2, Y);
			LOAD_VAR(_r3, Matrix.Row);
			LOAD_VAR(_r4, Matrix.Row + 4);

			_r1 = _r1 * _r3 + _r2 * _r4;
			return Vector2(_r1.extract(0), _r1.extract(1));
#else
			return Vector2(
				X * Matrix.Row[0] + Y * Matrix.Row[4],
				X * Matrix.Row[1] + Y * Matrix.Row[5]);
#endif
		}
		Vector2 Vector2::Direction(float Rotation) const
		{
			return Vector2(cos(-Rotation), sin(-Rotation));
		}
		Vector2 Vector2::Inv() const
		{
			return Vector2(-X, -Y);
		}
		Vector2 Vector2::InvX() const
		{
			return Vector2(-X, Y);
		}
		Vector2 Vector2::InvY() const
		{
			return Vector2(X, -Y);
		}
		Vector2 Vector2::Normalize() const
		{
#ifdef VI_VECTORCLASS
			LOAD_FV2(_r1);
			_r1 = _r1 * Geometric::FastInvSqrt(horizontal_add(square(_r1)));
			return Vector2(_r1.extract(0), _r1.extract(1));
#else
			float F = Length();
			return Vector2(X / F, Y / F);
#endif
		}
		Vector2 Vector2::sNormalize() const
		{
#ifdef VI_VECTORCLASS
			LOAD_FV2(_r1);
			float F = Geometric::FastSqrt(horizontal_add(square(_r1)));
			if (F == 0.0f)
				return Vector2();

			_r1 = _r1 / F;
			return Vector2(_r1.extract(0), _r1.extract(1));
#else
			float F = Length();
			if (F == 0.0f)
				return Vector2();

			return Vector2(X / F, Y / F);
#endif
		}
		Vector2 Vector2::Lerp(const Vector2& B, float DeltaTime) const
		{
#ifdef VI_VECTORCLASS
			LOAD_FV2(_r1); LOAD_V2(_r2, B);
			_r1 = _r1 + (_r2 - _r1) * DeltaTime;
			return Vector2(_r1.extract(0), _r1.extract(1));
#else
			return *this + (B - *this) * DeltaTime;
#endif
		}
		Vector2 Vector2::sLerp(const Vector2& B, float DeltaTime) const
		{
			return Quaternion(Vector3()).sLerp(B.XYZ(), DeltaTime).GetEuler().XY();
		}
		Vector2 Vector2::aLerp(const Vector2& B, float DeltaTime) const
		{
			float Ax = Geometric::AngluarLerp(X, B.X, DeltaTime);
			float Ay = Geometric::AngluarLerp(Y, B.Y, DeltaTime);
			return Vector2(Ax, Ay);
		}
		Vector2 Vector2::rLerp() const
		{
			float Ax = Compute::Mathf::SaturateAngle(X);
			float Ay = Compute::Mathf::SaturateAngle(Y);
			return Vector2(Ax, Ay);
		}
		Vector2 Vector2::Abs() const
		{
#ifdef VI_VECTORCLASS
			LOAD_FV2(_r1); _r1 = abs(_r1);
			return Vector2(_r1.extract(0), _r1.extract(1));
#else
			return Vector2(X < 0 ? -X : X, Y < 0 ? -Y : Y);
#endif
		}
		Vector2 Vector2::Radians() const
		{
			return (*this) * Compute::Mathf::Deg2Rad();
		}
		Vector2 Vector2::Degrees() const
		{
			return (*this) * Compute::Mathf::Rad2Deg();
		}
		Vector2 Vector2::XY() const
		{
			return *this;
		}
		Vector3 Vector2::XYZ() const
		{
			return Vector3(X, Y, 0);
		}
		Vector4 Vector2::XYZW() const
		{
			return Vector4(X, Y, 0, 0);
		}
		Vector2 Vector2::Mul(float xy) const
		{
#ifdef VI_VECTORCLASS
			LOAD_FV2(_r1); _r1 = _r1 * xy;
			return Vector2(_r1.extract(0), _r1.extract(1));
#else
			return Vector2(X * xy, Y * xy);
#endif
		}
		Vector2 Vector2::Mul(float x, float y) const
		{
#ifdef VI_VECTORCLASS
			LOAD_FV2(_r1); LOAD_AV2(_r2, x, y); _r1 = _r1 * _r2;
			return Vector2(_r1.extract(0), _r1.extract(1));
#else
			return Vector2(X * x, Y * y);
#endif
		}
		Vector2 Vector2::Mul(const Vector2& Value) const
		{
			return Mul(Value.X, Value.Y);
		}
		Vector2 Vector2::Div(const Vector2& Value) const
		{
#ifdef VI_VECTORCLASS
			LOAD_FV2(_r1); LOAD_V2(_r2, Value); _r1 = _r1 / _r2;
			return Vector2(_r1.extract(0), _r1.extract(1));
#else
			return Vector2(X / Value.X, Y / Value.Y);
#endif
		}
		Vector2 Vector2::Add(const Vector2& Value) const
		{
#ifdef VI_VECTORCLASS
			LOAD_FV2(_r1); LOAD_V2(_r2, Value); _r1 = _r1 + _r2;
			return Vector2(_r1.extract(0), _r1.extract(1));
#else
			return Vector2(X + Value.X, Y + Value.Y);
#endif
		}
		Vector2 Vector2::SetX(float Xf) const
		{
			return Vector2(Xf, Y);
		}
		Vector2 Vector2::SetY(float Yf) const
		{
			return Vector2(X, Yf);
		}
		void Vector2::Set(const Vector2& Value)
		{
			X = Value.X;
			Y = Value.Y;
		}
		void Vector2::Get2(float* In) const
		{
			VI_ASSERT(In != nullptr, "in of size 2 should be set");
			In[0] = X;
			In[1] = Y;
		}
		Vector2& Vector2::operator *=(const Vector2& V)
		{
#ifdef VI_VECTORCLASS
			LOAD_FV2(_r1); LOAD_V2(_r2, V);
			_r1 = _r1 * _r2;
			_r1.store_partial(2, (float*)this);
#else
			X *= V.X;
			Y *= V.Y;
#endif
			return *this;
		}
		Vector2& Vector2::operator *=(float V)
		{
#ifdef VI_VECTORCLASS
			LOAD_FV2(_r1);
			_r1 = _r1 * V;
			_r1.store_partial(2, (float*)this);
#else
			X *= V;
			Y *= V;
#endif
			return *this;
		}
		Vector2& Vector2::operator /=(const Vector2& V)
		{
#ifdef VI_VECTORCLASS
			LOAD_FV2(_r1); LOAD_V2(_r2, V);
			_r1 = _r1 / _r2;
			_r1.store_partial(2, (float*)this);
#else
			X /= V.X;
			Y /= V.Y;
#endif
			return *this;
		}
		Vector2& Vector2::operator /=(float V)
		{
#ifdef VI_VECTORCLASS
			LOAD_FV2(_r1);
			_r1 = _r1 / V;
			_r1.store_partial(2, (float*)this);
#else
			X /= V;
			Y /= V;
#endif
			return *this;
		}
		Vector2& Vector2::operator +=(const Vector2& V)
		{
#ifdef VI_VECTORCLASS
			LOAD_FV2(_r1); LOAD_V2(_r2, V);
			_r1 = _r1 + _r2;
			_r1.store_partial(2, (float*)this);
#else
			X += V.X;
			Y += V.Y;
#endif
			return *this;
		}
		Vector2& Vector2::operator +=(float V)
		{
#ifdef VI_VECTORCLASS
			LOAD_FV2(_r1);
			_r1 = _r1 + V;
			_r1.store_partial(2, (float*)this);
#else
			X += V;
			Y += V;
#endif
			return *this;
		}
		Vector2& Vector2::operator -=(const Vector2& V)
		{
#ifdef VI_VECTORCLASS
			LOAD_FV2(_r1); LOAD_V2(_r2, V);
			_r1 = _r1 - _r2;
			_r1.store_partial(2, (float*)this);
#else
			X -= V.X;
			Y -= V.Y;
#endif
			return *this;
		}
		Vector2& Vector2::operator -=(float V)
		{
#ifdef VI_VECTORCLASS
			LOAD_FV2(_r1);
			_r1 = _r1 - V;
			_r1.store_partial(2, (float*)this);
#else
			X -= V;
			Y -= V;
#endif
			return *this;
		}
		Vector2 Vector2::operator *(const Vector2& V) const
		{
			return Mul(V);
		}
		Vector2 Vector2::operator *(float V) const
		{
			return Mul(V);
		}
		Vector2 Vector2::operator /(const Vector2& V) const
		{
			return Div(V);
		}
		Vector2 Vector2::operator /(float V) const
		{
			return Div(V);
		}
		Vector2 Vector2::operator +(const Vector2& V) const
		{
			return Add(V);
		}
		Vector2 Vector2::operator +(float V) const
		{
			return Add(V);
		}
		Vector2 Vector2::operator -(const Vector2& V) const
		{
			return Add(-V);
		}
		Vector2 Vector2::operator -(float V) const
		{
			return Add(-V);
		}
		Vector2 Vector2::operator -() const
		{
			return Inv();
		}
		Vector2& Vector2::operator =(const Vector2& V) noexcept
		{
			X = V.X;
			Y = V.Y;
			return *this;
		}
		bool Vector2::operator ==(const Vector2& R) const
		{
			return X == R.X && Y == R.Y;
		}
		bool Vector2::operator !=(const Vector2& R) const
		{
			return !(X == R.X && Y == R.Y);
		}
		bool Vector2::operator <=(const Vector2& R) const
		{
			return X <= R.X && Y <= R.Y;
		}
		bool Vector2::operator >=(const Vector2& R) const
		{
			return X >= R.X && Y >= R.Y;
		}
		bool Vector2::operator <(const Vector2& R) const
		{
			return X < R.X&& Y < R.Y;
		}
		bool Vector2::operator >(const Vector2& R) const
		{
			return X > R.X && Y > R.Y;
		}
		float& Vector2::operator [](uint32_t Axis)
		{
			VI_ASSERT(Axis >= 0 && Axis <= 1, "index out of range");
			if (Axis == 0)
				return X;

			return Y;
		}
		float Vector2::operator [](uint32_t Axis) const
		{
			VI_ASSERT(Axis >= 0 && Axis <= 1, "index out of range");
			if (Axis == 0)
				return X;

			return Y;
		}
		Vector2 Vector2::Random()
		{
			return Vector2(Compute::Mathf::RandomMag(), Compute::Mathf::RandomMag());
		}
		Vector2 Vector2::RandomAbs()
		{
			return Vector2(Compute::Mathf::Random(), Compute::Mathf::Random());
		}

		Vector3::Vector3() noexcept : X(0.0f), Y(0.0f), Z(0.0f)
		{
		}
		Vector3::Vector3(float x, float y) noexcept : X(x), Y(y), Z(0.0f)
		{
		}
		Vector3::Vector3(float x, float y, float z) noexcept : X(x), Y(y), Z(z)
		{
		}
		Vector3::Vector3(float xyzw) noexcept : X(xyzw), Y(xyzw), Z(xyzw)
		{
		}
		Vector3::Vector3(const Vector2& Value) noexcept : X(Value.X), Y(Value.Y), Z(0.0f)
		{
		}
		Vector3::Vector3(const Vector3& Value) noexcept : X(Value.X), Y(Value.Y), Z(Value.Z)
		{
		}
		Vector3::Vector3(const Vector4& Value) noexcept : X(Value.X), Y(Value.Y), Z(Value.Z)
		{
		}
		bool Vector3::IsEquals(const Vector3& Other, float MaxDisplacement) const
		{
			return abs(X - Other.X) <= MaxDisplacement && abs(Y - Other.Y) <= MaxDisplacement && abs(Z - Other.Z) <= MaxDisplacement;
		}
		float Vector3::Length() const
		{
#ifdef VI_VECTORCLASS
			LOAD_FV3(_r1);
			return sqrt(horizontal_add(square(_r1)));
#else
			return std::sqrt(X * X + Y * Y + Z * Z);
#endif
		}
		float Vector3::Sum() const
		{
#ifdef VI_VECTORCLASS
			LOAD_FV3(_r1);
			return horizontal_add(_r1);
#else
			return X + Y + Z;
#endif
		}
		float Vector3::Dot(const Vector3& B) const
		{
#ifdef VI_VECTORCLASS
			LOAD_FV3(_r1); LOAD_V3(_r2, B);
			return horizontal_add(_r1 * _r2);
#else
			return X * B.X + Y * B.Y + Z * B.Z;
#endif
		}
		float Vector3::Distance(const Vector3& Point) const
		{
#ifdef VI_VECTORCLASS
			LOAD_FV3(_r1); LOAD_V3(_r2, Point);
			return Geometric::FastSqrt(horizontal_add(square(_r1 - _r2)));
#else
			float X1 = X - Point.X, Y1 = Y - Point.Y, Z1 = Z - Point.Z;
			return Geometric::FastSqrt(X1 * X1 + Y1 * Y1 + Z1 * Z1);
#endif
		}
		float Vector3::Hypotenuse() const
		{
#ifdef VI_VECTORCLASS
			LOAD_AV2(_r1, X, Z);
			float R = Geometric::FastSqrt(horizontal_add(square(_r1)));

			LOAD_AV2(_r2, R, Y);
			return Geometric::FastSqrt(horizontal_add(square(_r2)));
#else
			float R = Geometric::FastSqrt(X * X + Z * Z);
			return Geometric::FastSqrt(R * R + Y * Y);
#endif
		}
		Vector3 Vector3::LookAt(const Vector3& B) const
		{
			Vector2 H1(X, Z), H2(B.X, B.Z);
			return Vector3(0.0f, -H1.LookAt(H2), 0.0f);
		}
		Vector3 Vector3::Cross(const Vector3& B) const
		{
#ifdef VI_VECTORCLASS
			LOAD_AV3(_r1, Y, Z, X);
			LOAD_AV3(_r2, Z, X, Y);
			LOAD_AV3(_r3, B.Z, B.X, B.Y);
			LOAD_AV3(_r4, B.Y, B.Z, B.X);

			_r1 = _r1 * _r3 - _r2 * _r4;
			return Vector3(_r1.extract(0), _r1.extract(1), _r1.extract(2));
#else
			return Vector3(Y * B.Z - Z * B.Y, Z * B.X - X * B.Z, X * B.Y - Y * B.X);
#endif
		}
		Vector3 Vector3::Transform(const Matrix4x4& Matrix) const
		{
#ifdef VI_VECTORCLASS
			LOAD_VAL(_r1, X);
			LOAD_VAL(_r2, Y);
			LOAD_VAL(_r3, Z);
			LOAD_VAR(_r4, Matrix.Row);
			LOAD_VAR(_r5, Matrix.Row + 4);
			LOAD_VAR(_r6, Matrix.Row + 8);

			_r1 = _r1 * _r4 + _r2 * _r5 + _r3 * _r6;
			return Vector3(_r1.extract(0), _r1.extract(1), _r1.extract(2));
#else
			return Vector3(
				X * Matrix.Row[0] + Y * Matrix.Row[4] + Z * Matrix.Row[8],
				X * Matrix.Row[1] + Y * Matrix.Row[5] + Z * Matrix.Row[9],
				X * Matrix.Row[2] + Y * Matrix.Row[6] + Z * Matrix.Row[10]);
#endif
		}
		Vector3 Vector3::dDirection() const
		{
			float CosX = cos(X);
			return Vector3(sin(Y) * CosX, -sin(X), cos(Y) * CosX);
		}
		Vector3 Vector3::hDirection() const
		{
			return Vector3(-cos(Y), 0, sin(Y));
		}
		Vector3 Vector3::Direction() const
		{
			return Matrix4x4::CreateLookAt(0, *this, Vector3::Up()).RotationEuler();
		}
		Vector3 Vector3::Inv() const
		{
			return Vector3(-X, -Y, -Z);
		}
		Vector3 Vector3::InvX() const
		{
			return Vector3(-X, Y, Z);
		}
		Vector3 Vector3::InvY() const
		{
			return Vector3(X, -Y, Z);
		}
		Vector3 Vector3::InvZ() const
		{
			return Vector3(X, Y, -Z);
		}
		Vector3 Vector3::Normalize() const
		{
#ifdef VI_VECTORCLASS
			LOAD_FV3(_r1);
			_r1 = _r1 * Geometric::FastInvSqrt(horizontal_add(square(_r1)));
			return Vector3(_r1.extract(0), _r1.extract(1), _r1.extract(2));
#else
			float F = Length();
			return Vector3(X / F, Y / F, Z / F);
#endif
		}
		Vector3 Vector3::sNormalize() const
		{
#ifdef VI_VECTORCLASS
			LOAD_FV3(_r1);
			float F = Geometric::FastSqrt(horizontal_add(square(_r1)));
			if (F == 0.0f)
				return Vector3();

			_r1 = _r1 / F;
			return Vector3(_r1.extract(0), _r1.extract(1), _r1.extract(2));
#else
			float F = Length();
			if (F == 0.0f)
				return Vector3();

			return Vector3(X / F, Y / F, Z / F);
#endif
		}
		Vector3 Vector3::Lerp(const Vector3& B, float DeltaTime) const
		{
#ifdef VI_VECTORCLASS
			LOAD_FV3(_r1); LOAD_V3(_r2, B);
			_r1 = _r1 + (_r2 - _r1) * DeltaTime;
			return Vector3(_r1.extract(0), _r1.extract(1), _r1.extract(2));
#else
			return *this + (B - *this) * DeltaTime;
#endif
		}
		Vector3 Vector3::sLerp(const Vector3& B, float DeltaTime) const
		{
			return Quaternion(*this).sLerp(B, DeltaTime).GetEuler();
		}
		Vector3 Vector3::aLerp(const Vector3& B, float DeltaTime) const
		{
			float Ax = Geometric::AngluarLerp(X, B.X, DeltaTime);
			float Ay = Geometric::AngluarLerp(Y, B.Y, DeltaTime);
			float Az = Geometric::AngluarLerp(Z, B.Z, DeltaTime);

			return Vector3(Ax, Ay, Az);
		}
		Vector3 Vector3::rLerp() const
		{
			float Ax = Compute::Mathf::SaturateAngle(X);
			float Ay = Compute::Mathf::SaturateAngle(Y);
			float Az = Compute::Mathf::SaturateAngle(Z);

			return Vector3(Ax, Ay, Az);
		}
		Vector3 Vector3::Abs() const
		{
#ifdef VI_VECTORCLASS
			LOAD_FV3(_r1); _r1 = abs(_r1);
			return Vector3(_r1.extract(0), _r1.extract(1), _r1.extract(2));
#else
			return Vector3(X < 0 ? -X : X, Y < 0 ? -Y : Y, Z < 0 ? -Z : Z);
#endif
		}
		Vector3 Vector3::Radians() const
		{
			return (*this) * Compute::Mathf::Deg2Rad();
		}
		Vector3 Vector3::Degrees() const
		{
			return (*this) * Compute::Mathf::Rad2Deg();
		}
		Vector3 Vector3::ViewSpace() const
		{
			return InvZ();
		}
		Vector2 Vector3::XY() const
		{
			return Vector2(X, Y);
		}
		Vector3 Vector3::XYZ() const
		{
			return *this;
		}
		Vector4 Vector3::XYZW() const
		{
			return Vector4(X, Y, Z, 0);
		}
		Vector3 Vector3::Mul(float xyz) const
		{
#ifdef VI_VECTORCLASS
			LOAD_FV3(_r1); _r1 = _r1 * xyz;
			return Vector3(_r1.extract(0), _r1.extract(1), _r1.extract(2));
#else
			return Vector3(X * xyz, Y * xyz, Z * xyz);
#endif
		}
		Vector3 Vector3::Mul(const Vector2& XY, float z) const
		{
#ifdef VI_VECTORCLASS
			LOAD_FV3(_r1); LOAD_AV3(_r2, XY.X, XY.Y, z); _r1 = _r1 * _r2;
			return Vector3(_r1.extract(0), _r1.extract(1), _r1.extract(2));
#else
			return Vector3(X * XY.X, Y * XY.Y, Z * z);
#endif
		}
		Vector3 Vector3::Mul(const Vector3& Value) const
		{
#ifdef VI_VECTORCLASS
			LOAD_FV3(_r1); LOAD_V3(_r2, Value); _r1 = _r1 * _r2;
			return Vector3(_r1.extract(0), _r1.extract(1), _r1.extract(2));
#else
			return Vector3(X * Value.X, Y * Value.Y, Z * Value.Z);
#endif
		}
		Vector3 Vector3::Div(const Vector3& Value) const
		{
#ifdef VI_VECTORCLASS
			LOAD_FV3(_r1); LOAD_V3(_r2, Value); _r1 = _r1 / _r2;
			return Vector3(_r1.extract(0), _r1.extract(1), _r1.extract(2));
#else
			return Vector3(X / Value.X, Y / Value.Y, Z / Value.Z);
#endif
		}
		Vector3 Vector3::Add(const Vector3& Value) const
		{
#ifdef VI_VECTORCLASS
			LOAD_FV3(_r1); LOAD_V3(_r2, Value); _r1 = _r1 + _r2;
			return Vector3(_r1.extract(0), _r1.extract(1), _r1.extract(2));
#else
			return Vector3(X + Value.X, Y + Value.Y, Z + Value.Z);
#endif
		}
		Vector3 Vector3::SetX(float Xf) const
		{
			return Vector3(Xf, Y, Z);
		}
		Vector3 Vector3::SetY(float Yf) const
		{
			return Vector3(X, Yf, Z);
		}
		Vector3 Vector3::SetZ(float Zf) const
		{
			return Vector3(X, Y, Zf);
		}
		Vector3 Vector3::Rotate(const Vector3& Origin, const Vector3& Rotation)
		{
			return Transform(Matrix4x4::Create(Origin, Rotation));
		}
		void Vector3::Set(const Vector3& Value)
		{
			X = Value.X;
			Y = Value.Y;
			Z = Value.Z;
		}
		void Vector3::Get2(float* In) const
		{
			VI_ASSERT(In != nullptr, "in of size 2 should be set");
			In[0] = X;
			In[1] = Y;
		}
		void Vector3::Get3(float* In) const
		{
			VI_ASSERT(In != nullptr, "in of size 3 should be set");
			In[0] = X;
			In[1] = Y;
			In[2] = Z;
		}
		Vector3& Vector3::operator *=(const Vector3& V)
		{
#ifdef VI_VECTORCLASS
			LOAD_FV3(_r1); LOAD_V3(_r2, V);
			_r1 = _r1 * _r2;
			_r1.store_partial(3, (float*)this);
#else
			X *= V.X;
			Y *= V.Y;
			Z *= V.Z;
#endif
			return *this;
		}
		Vector3& Vector3::operator *=(float V)
		{
#ifdef VI_VECTORCLASS
			LOAD_FV3(_r1); LOAD_VAL(_r2, V);
			_r1 = _r1 * _r2;
			_r1.store_partial(3, (float*)this);
#else
			X *= V;
			Y *= V;
			Z *= V;
#endif
			return *this;
		}
		Vector3& Vector3::operator /=(const Vector3& V)
		{
#ifdef VI_VECTORCLASS
			LOAD_FV3(_r1); LOAD_V3(_r2, V);
			_r1 = _r1 / _r2;
			_r1.store_partial(3, (float*)this);
#else
			X /= V.X;
			Y /= V.Y;
			Z /= V.Z;
#endif
			return *this;
		}
		Vector3& Vector3::operator /=(float V)
		{
#ifdef VI_VECTORCLASS
			LOAD_FV3(_r1); LOAD_VAL(_r2, V);
			_r1 = _r1 / _r2;
			_r1.store_partial(3, (float*)this);
#else
			X *= V;
			Y *= V;
			Z *= V;
#endif
			return *this;
		}
		Vector3& Vector3::operator +=(const Vector3& V)
		{
#ifdef VI_VECTORCLASS
			LOAD_FV3(_r1); LOAD_V3(_r2, V);
			_r1 = _r1 + _r2;
			_r1.store_partial(3, (float*)this);
#else
			X += V.X;
			Y += V.Y;
			Z += V.Z;
#endif
			return *this;
		}
		Vector3& Vector3::operator +=(float V)
		{
#ifdef VI_VECTORCLASS
			LOAD_FV3(_r1); LOAD_VAL(_r2, V);
			_r1 = _r1 + _r2;
			_r1.store_partial(3, (float*)this);
#else
			X += V;
			Y += V;
			Z += V;
#endif
			return *this;
		}
		Vector3& Vector3::operator -=(const Vector3& V)
		{
#ifdef VI_VECTORCLASS
			LOAD_FV3(_r1); LOAD_V3(_r2, V);
			_r1 = _r1 - _r2;
			_r1.store_partial(3, (float*)this);
#else
			X -= V.X;
			Y -= V.Y;
			Z -= V.Z;
#endif
			return *this;
		}
		Vector3& Vector3::operator -=(float V)
		{
#ifdef VI_VECTORCLASS
			LOAD_FV3(_r1); LOAD_VAL(_r2, V);
			_r1 = _r1 - _r2;
			_r1.store_partial(3, (float*)this);
#else
			X -= V;
			Y -= V;
			Z -= V;
#endif
			return *this;
		}
		Vector3 Vector3::operator *(const Vector3& V) const
		{
			return Mul(V);
		}
		Vector3 Vector3::operator *(float V) const
		{
			return Mul(V);
		}
		Vector3 Vector3::operator /(const Vector3& V) const
		{
			return Div(V);
		}
		Vector3 Vector3::operator /(float V) const
		{
			return Div(V);
		}
		Vector3 Vector3::operator +(const Vector3& V) const
		{
			return Add(V);
		}
		Vector3 Vector3::operator +(float V) const
		{
			return Add(V);
		}
		Vector3 Vector3::operator -(const Vector3& V) const
		{
			return Add(-V);
		}
		Vector3 Vector3::operator -(float V) const
		{
			return Add(-V);
		}
		Vector3 Vector3::operator -() const
		{
			return Inv();
		}
		Vector3& Vector3::operator =(const Vector3& V) noexcept
		{
			X = V.X;
			Y = V.Y;
			Z = V.Z;
			return *this;
		}
		bool Vector3::operator ==(const Vector3& R) const
		{
			return X == R.X && Y == R.Y && Z == R.Z;
		}
		bool Vector3::operator !=(const Vector3& R) const
		{
			return !(X == R.X && Y == R.Y && Z == R.Z);
		}
		bool Vector3::operator <=(const Vector3& R) const
		{
			return X <= R.X && Y <= R.Y && Z <= R.Z;
		}
		bool Vector3::operator >=(const Vector3& R) const
		{
			return X >= R.X && Y >= R.Y && Z >= R.Z;
		}
		bool Vector3::operator <(const Vector3& R) const
		{
			return X < R.X&& Y < R.Y&& Z < R.Z;
		}
		bool Vector3::operator >(const Vector3& R) const
		{
			return X > R.X && Y > R.Y && Z > R.Z;
		}
		float& Vector3::operator [](uint32_t Axis)
		{
			VI_ASSERT(Axis >= 0 && Axis <= 2, "index out of range");
			if (Axis == 0)
				return X;
			else if (Axis == 1)
				return Y;

			return Z;
		}
		float Vector3::operator [](uint32_t Axis) const
		{
			VI_ASSERT(Axis >= 0 && Axis <= 2, "index out of range");
			if (Axis == 0)
				return X;
			else if (Axis == 1)
				return Y;

			return Z;
		}
		Vector3 Vector3::Random()
		{
			return Vector3(Compute::Mathf::RandomMag(), Compute::Mathf::RandomMag(), Compute::Mathf::RandomMag());
		}
		Vector3 Vector3::RandomAbs()
		{
			return Vector3(Compute::Mathf::Random(), Compute::Mathf::Random(), Compute::Mathf::Random());
		}

		Vector4::Vector4() noexcept : X(0.0f), Y(0.0f), Z(0.0f), W(0.0f)
		{
		}
		Vector4::Vector4(float x, float y) noexcept : X(x), Y(y), Z(0.0f), W(0.0f)
		{
		}
		Vector4::Vector4(float x, float y, float z) noexcept : X(x), Y(y), Z(z), W(0.0f)
		{
		}
		Vector4::Vector4(float x, float y, float z, float w) noexcept : X(x), Y(y), Z(z), W(w)
		{
		}
		Vector4::Vector4(float xyzw) noexcept : X(xyzw), Y(xyzw), Z(xyzw), W(xyzw)
		{
		}
		Vector4::Vector4(const Vector2& Value) noexcept : X(Value.X), Y(Value.Y), Z(0.0f), W(0.0f)
		{
		}
		Vector4::Vector4(const Vector3& Value) noexcept : X(Value.X), Y(Value.Y), Z(Value.Z), W(0.0f)
		{
		}
		Vector4::Vector4(const Vector4& Value) noexcept : X(Value.X), Y(Value.Y), Z(Value.Z), W(Value.W)
		{
		}
		bool Vector4::IsEquals(const Vector4& Other, float MaxDisplacement) const
		{
			return abs(X - Other.X) <= MaxDisplacement && abs(Y - Other.Y) <= MaxDisplacement && abs(Z - Other.Z) <= MaxDisplacement && abs(W - Other.W) <= MaxDisplacement;
		}
		float Vector4::Length() const
		{
#ifdef VI_VECTORCLASS
			LOAD_FV4(_r1);
			return std::sqrt(horizontal_add(square(_r1)));
#else
			return std::sqrt(X * X + Y * Y + Z * Z + W * W);
#endif
		}
		float Vector4::Sum() const
		{
#ifdef VI_VECTORCLASS
			LOAD_FV4(_r1);
			return horizontal_add(_r1);
#else
			return X + Y + Z + W;
#endif
		}
		float Vector4::Dot(const Vector4& B) const
		{
#ifdef VI_VECTORCLASS
			LOAD_FV4(_r1); LOAD_V4(_r2, B);
			return horizontal_add(_r1 * _r2);
#else
			return X * B.X + Y * B.Y + Z * B.Z + W * B.W;
#endif
		}
		float Vector4::Distance(const Vector4& Point) const
		{
#ifdef VI_VECTORCLASS
			LOAD_FV4(_r1); LOAD_V4(_r2, Point);
			return Geometric::FastSqrt(horizontal_add(square(_r1 - _r2)));
#else
			float X1 = X - Point.X, Y1 = Y - Point.Y, Z1 = Z - Point.Z, W1 = W - Point.W;
			return Geometric::FastSqrt(X1 * X1 + Y1 * Y1 + Z1 * Z1 + W1 * W1);
#endif
		}
		Vector4 Vector4::Cross(const Vector4& B) const
		{
#ifdef VI_VECTORCLASS
			LOAD_AV3(_r1, Y, Z, X);
			LOAD_AV3(_r2, Z, X, Y);
			LOAD_AV3(_r3, B.Z, B.X, B.Y);
			LOAD_AV3(_r4, B.Y, B.Z, B.X);

			_r1 = _r1 * _r3 - _r2 * _r4;
			return Vector3(_r1.extract(0), _r1.extract(1), _r1.extract(2));
#else
			return Vector4(Y * B.Z - Z * B.Y, Z * B.X - X * B.Z, X * B.Y - Y * B.X);
#endif
		}
		Vector4 Vector4::Transform(const Matrix4x4& Matrix) const
		{
#ifdef VI_VECTORCLASS
			LOAD_VAL(_r1, X);
			LOAD_VAL(_r2, Y);
			LOAD_VAL(_r3, Z);
			LOAD_VAL(_r4, W);
			LOAD_VAR(_r5, Matrix.Row);
			LOAD_VAR(_r6, Matrix.Row + 4);
			LOAD_VAR(_r7, Matrix.Row + 8);
			LOAD_VAR(_r8, Matrix.Row + 12);

			_r1 = _r1 * _r5 + _r2 * _r6 + _r3 * _r7 + _r4 * _r8;
			return Vector4(_r1.extract(0), _r1.extract(1), _r1.extract(2), _r1.extract(3));
#else
			return Vector4(
				X * Matrix.Row[0] + Y * Matrix.Row[4] + Z * Matrix.Row[8] + W * Matrix.Row[12],
				X * Matrix.Row[1] + Y * Matrix.Row[5] + Z * Matrix.Row[9] + W * Matrix.Row[13],
				X * Matrix.Row[2] + Y * Matrix.Row[6] + Z * Matrix.Row[10] + W * Matrix.Row[14],
				X * Matrix.Row[3] + Y * Matrix.Row[7] + Z * Matrix.Row[11] + W * Matrix.Row[15]);
#endif
		}
		Vector4 Vector4::Inv() const
		{
			return Vector4(-X, -Y, -Z, -W);
		}
		Vector4 Vector4::InvX() const
		{
			return Vector4(-X, Y, Z, W);
		}
		Vector4 Vector4::InvY() const
		{
			return Vector4(X, -Y, Z, W);
		}
		Vector4 Vector4::InvZ() const
		{
			return Vector4(X, Y, -Z, W);
		}
		Vector4 Vector4::InvW() const
		{
			return Vector4(X, Y, Z, -W);
		}
		Vector4 Vector4::Normalize() const
		{
#ifdef VI_VECTORCLASS
			LOAD_FV4(_r1);
			_r1 = _r1 * Geometric::FastInvSqrt(horizontal_add(square(_r1)));
			return Vector4(_r1.extract(0), _r1.extract(1), _r1.extract(2), _r1.extract(3));
#else
			float F = Length();
			return Vector4(X / F, Y / F, Z / F, W / F);
#endif
		}
		Vector4 Vector4::sNormalize() const
		{
#ifdef VI_VECTORCLASS
			LOAD_FV4(_r1);
			float F = Geometric::FastSqrt(horizontal_add(square(_r1)));
			if (F == 0.0f)
				return Vector4();

			_r1 = _r1 / F;
			return Vector4(_r1.extract(0), _r1.extract(1), _r1.extract(2), _r1.extract(3));
#else
			float F = Length();
			if (F == 0.0f)
				return Vector4();

			return Vector4(X / F, Y / F, Z / F, W / F);
#endif
		}
		Vector4 Vector4::Lerp(const Vector4& B, float DeltaTime) const
		{
#ifdef VI_VECTORCLASS
			LOAD_FV4(_r1); LOAD_V4(_r2, B);
			_r1 = _r1 + (_r2 - _r1) * DeltaTime;
			return Vector3(_r1.extract(0), _r1.extract(1), _r1.extract(2));
#else
			return *this + (B - *this) * DeltaTime;
#endif
		}
		Vector4 Vector4::sLerp(const Vector4& B, float DeltaTime) const
		{
			Vector3 Lerp = Quaternion(Vector3()).sLerp(B.XYZ(), DeltaTime).GetEuler();
			return Vector4(Lerp.X, Lerp.Y, Lerp.Z, W + (B.W - W) * DeltaTime);
		}
		Vector4 Vector4::aLerp(const Vector4& B, float DeltaTime) const
		{
			float Ax = Geometric::AngluarLerp(X, B.X, DeltaTime);
			float Ay = Geometric::AngluarLerp(Y, B.Y, DeltaTime);
			float Az = Geometric::AngluarLerp(Z, B.Z, DeltaTime);
			float Aw = Geometric::AngluarLerp(W, B.W, DeltaTime);

			return Vector4(Ax, Ay, Az, Aw);
		}
		Vector4 Vector4::rLerp() const
		{
			float Ax = Compute::Mathf::SaturateAngle(X);
			float Ay = Compute::Mathf::SaturateAngle(Y);
			float Az = Compute::Mathf::SaturateAngle(Z);
			float Aw = Compute::Mathf::SaturateAngle(W);

			return Vector4(Ax, Ay, Az, Aw);
		}
		Vector4 Vector4::Abs() const
		{
#ifdef VI_VECTORCLASS
			LOAD_FV4(_r1); _r1 = abs(_r1);
			return Vector4(_r1.extract(0), _r1.extract(1), _r1.extract(2), _r1.extract(3));
#else
			return Vector4(X < 0 ? -X : X, Y < 0 ? -Y : Y, Z < 0 ? -Z : Z, W < 0 ? -W : W);
#endif
		}
		Vector4 Vector4::Radians() const
		{
			return (*this) * Compute::Mathf::Deg2Rad();
		}
		Vector4 Vector4::Degrees() const
		{
			return (*this) * Compute::Mathf::Rad2Deg();
		}
		Vector4 Vector4::ViewSpace() const
		{
			return InvZ();
		}
		Vector2 Vector4::XY() const
		{
			return Vector2(X, Y);
		}
		Vector3 Vector4::XYZ() const
		{
			return Vector3(X, Y, Z);
		}
		Vector4 Vector4::XYZW() const
		{
			return *this;
		}
		Vector4 Vector4::SetX(float TexCoordX) const
		{
			return Vector4(TexCoordX, Y, Z, W);
		}
		Vector4 Vector4::SetY(float TexCoordY) const
		{
			return Vector4(X, TexCoordY, Z, W);
		}
		Vector4 Vector4::SetZ(float TZ) const
		{
			return Vector4(X, Y, TZ, W);
		}
		Vector4 Vector4::SetW(float TW) const
		{
			return Vector4(X, Y, Z, TW);
		}
		Vector4 Vector4::Mul(float xyzw) const
		{
#ifdef VI_VECTORCLASS
			LOAD_FV4(_r1); _r1 = _r1 * xyzw;
			return Vector4(_r1.extract(0), _r1.extract(1), _r1.extract(2), _r1.extract(3));
#else
			return Vector4(X * xyzw, Y * xyzw, Z * xyzw, W * xyzw);
#endif
		}
		Vector4 Vector4::Mul(const Vector2& XY, float z, float w) const
		{
#ifdef VI_VECTORCLASS
			LOAD_FV4(_r1); LOAD_AV4(_r2, XY.X, XY.Y, z, w); _r1 = _r1 * _r2;
			return Vector4(_r1.extract(0), _r1.extract(1), _r1.extract(2), _r1.extract(3));
#else
			return Vector4(X * XY.X, Y * XY.Y, Z * z, W * w);
#endif
		}
		Vector4 Vector4::Mul(const Vector3& XYZ, float w) const
		{
#ifdef VI_VECTORCLASS
			LOAD_FV4(_r1); LOAD_AV4(_r2, XYZ.X, XYZ.Y, XYZ.Z, w); _r1 = _r1 * _r2;
			return Vector4(_r1.extract(0), _r1.extract(1), _r1.extract(2), _r1.extract(3));
#else
			return Vector4(X * XYZ.X, Y * XYZ.Y, Z * XYZ.Z, W * w);
#endif
		}
		Vector4 Vector4::Mul(const Vector4& Value) const
		{
#ifdef VI_VECTORCLASS
			LOAD_FV4(_r1); LOAD_V4(_r2, Value); _r1 = _r1 * _r2;
			return Vector4(_r1.extract(0), _r1.extract(1), _r1.extract(2), _r1.extract(3));
#else
			return Vector4(X * Value.X, Y * Value.Y, Z * Value.Z, W * Value.W);
#endif
		}
		Vector4 Vector4::Div(const Vector4& Value) const
		{
#ifdef VI_VECTORCLASS
			LOAD_FV4(_r1); LOAD_V4(_r2, Value); _r1 = _r1 / _r2;
			return Vector4(_r1.extract(0), _r1.extract(1), _r1.extract(2), _r1.extract(3));
#else
			return Vector4(X / Value.X, Y / Value.Y, Z / Value.Z, W / Value.W);
#endif
		}
		Vector4 Vector4::Add(const Vector4& Value) const
		{
#ifdef VI_VECTORCLASS
			LOAD_FV4(_r1); LOAD_V4(_r2, Value); _r1 = _r1 + _r2;
			return Vector4(_r1.extract(0), _r1.extract(1), _r1.extract(2), _r1.extract(3));
#else
			return Vector4(X + Value.X, Y + Value.Y, Z + Value.Z, W + Value.W);
#endif
		}
		void Vector4::Set(const Vector4& Value)
		{
			X = Value.X;
			Y = Value.Y;
			Z = Value.Z;
			W = Value.W;
		}
		void Vector4::Get2(float* In) const
		{
			VI_ASSERT(In != nullptr, "in of size 2 should be set");
			In[0] = X;
			In[1] = Y;
		}
		void Vector4::Get3(float* In) const
		{
			VI_ASSERT(In != nullptr, "in of size 3 should be set");
			In[0] = X;
			In[1] = Y;
			In[2] = Z;
		}
		void Vector4::Get4(float* In) const
		{
			VI_ASSERT(In != nullptr, "in of size 4 should be set");
			In[0] = X;
			In[1] = Y;
			In[2] = Z;
			In[3] = W;
		}
		Vector4& Vector4::operator *=(const Matrix4x4& V)
		{
			Set(Transform(V));
			return *this;
		}
		Vector4& Vector4::operator *=(const Vector4& V)
		{
#ifdef VI_VECTORCLASS
			LOAD_FV4(_r1); LOAD_V4(_r2, V);
			_r1 = _r1 * _r2;
			_r1.store((float*)this);
#else
			X *= V.X;
			Y *= V.Y;
			Z *= V.Z;
			W *= V.W;
#endif
			return *this;
		}
		Vector4& Vector4::operator *=(float V)
		{
#ifdef VI_VECTORCLASS
			LOAD_FV4(_r1); LOAD_VAL(_r2, V);
			_r1 = _r1 * _r2;
			_r1.store((float*)this);
#else
			X *= V;
			Y *= V;
			Z *= V;
			W *= V;
#endif
			return *this;
		}
		Vector4& Vector4::operator /=(const Vector4& V)
		{
#ifdef VI_VECTORCLASS
			LOAD_FV4(_r1); LOAD_V4(_r2, V);
			_r1 = _r1 / _r2;
			_r1.store((float*)this);
#else
			X /= V.X;
			Y /= V.Y;
			Z /= V.Z;
			W /= V.W;
#endif
			return *this;
		}
		Vector4& Vector4::operator /=(float V)
		{
#ifdef VI_VECTORCLASS
			LOAD_FV4(_r1); LOAD_VAL(_r2, V);
			_r1 = _r1 / _r2;
			_r1.store((float*)this);
#else
			X /= V;
			Y /= V;
			Z /= V;
			W /= V;
#endif
			return *this;
		}
		Vector4& Vector4::operator +=(const Vector4& V)
		{
#ifdef VI_VECTORCLASS
			LOAD_FV4(_r1); LOAD_V4(_r2, V);
			_r1 = _r1 + _r2;
			_r1.store((float*)this);
#else
			X += V.X;
			Y += V.Y;
			Z += V.Z;
			W += V.W;
#endif
			return *this;
		}
		Vector4& Vector4::operator +=(float V)
		{
#ifdef VI_VECTORCLASS
			LOAD_FV4(_r1); LOAD_VAL(_r2, V);
			_r1 = _r1 + _r2;
			_r1.store((float*)this);
#else
			X += V;
			Y += V;
			Z += V;
			W += V;
#endif
			return *this;
		}
		Vector4& Vector4::operator -=(const Vector4& V)
		{
#ifdef VI_VECTORCLASS
			LOAD_FV4(_r1); LOAD_V4(_r2, V);
			_r1 = _r1 - _r2;
			_r1.store((float*)this);
#else
			X -= V.X;
			Y -= V.Y;
			Z -= V.Z;
			W -= V.W;
#endif
			return *this;
		}
		Vector4& Vector4::operator -=(float V)
		{
#ifdef VI_VECTORCLASS
			LOAD_FV4(_r1); LOAD_VAL(_r2, V);
			_r1 = _r1 - _r2;
			_r1.store((float*)this);
#else
			X -= V;
			Y -= V;
			Z -= V;
			W -= V;
#endif
			return *this;
		}
		Vector4 Vector4::operator *(const Matrix4x4& V) const
		{
			return Transform(V);
		}
		Vector4 Vector4::operator *(const Vector4& V) const
		{
			return Mul(V);
		}
		Vector4 Vector4::operator *(float V) const
		{
			return Mul(V);
		}
		Vector4 Vector4::operator /(const Vector4& V) const
		{
			return Div(V);
		}
		Vector4 Vector4::operator /(float V) const
		{
			return Div(V);
		}
		Vector4 Vector4::operator +(const Vector4& V) const
		{
			return Add(V);
		}
		Vector4 Vector4::operator +(float V) const
		{
			return Add(V);
		}
		Vector4 Vector4::operator -(const Vector4& V) const
		{
			return Add(-V);
		}
		Vector4 Vector4::operator -(float V) const
		{
			return Add(-V);
		}
		Vector4 Vector4::operator -() const
		{
			return Inv();
		}
		Vector4& Vector4::operator =(const Vector4& V) noexcept
		{
			X = V.X;
			Y = V.Y;
			Z = V.Z;
			W = V.W;
			return *this;
		}
		bool Vector4::operator ==(const Vector4& R) const
		{
			return X == R.X && Y == R.Y && Z == R.Z && W == R.W;
		}
		bool Vector4::operator !=(const Vector4& R) const
		{
			return !(X == R.X && Y == R.Y && Z == R.Z && W == R.W);
		}
		bool Vector4::operator <=(const Vector4& R) const
		{
			return X <= R.X && Y <= R.Y && Z <= R.Z && W <= R.W;
		}
		bool Vector4::operator >=(const Vector4& R) const
		{
			return X >= R.X && Y >= R.Y && Z >= R.Z && W >= R.W;
		}
		bool Vector4::operator <(const Vector4& R) const
		{
			return X < R.X&& Y < R.Y&& Z < R.Z&& W < R.W;
		}
		bool Vector4::operator >(const Vector4& R) const
		{
			return X > R.X && Y > R.Y && Z > R.Z && W > R.W;
		}
		float& Vector4::operator [](uint32_t Axis)
		{
			VI_ASSERT(Axis >= 0 && Axis <= 3, "index outside of range");
			if (Axis == 0)
				return X;
			else if (Axis == 1)
				return Y;
			else if (Axis == 2)
				return Z;

			return W;
		}
		float Vector4::operator [](uint32_t Axis) const
		{
			VI_ASSERT(Axis >= 0 && Axis <= 3, "index outside of range");
			if (Axis == 0)
				return X;
			else if (Axis == 1)
				return Y;
			else if (Axis == 2)
				return Z;

			return W;
		}
		Vector4 Vector4::Random()
		{
			return Vector4(Compute::Mathf::RandomMag(), Compute::Mathf::RandomMag(), Compute::Mathf::RandomMag(), Compute::Mathf::RandomMag());
		}
		Vector4 Vector4::RandomAbs()
		{
			return Vector4(Compute::Mathf::Random(), Compute::Mathf::Random(), Compute::Mathf::Random(), Compute::Mathf::Random());
		}
		
		Bounding::Bounding() noexcept
		{
		}
		Bounding::Bounding(const Vector3& LowerBound, const Vector3& UpperBound) noexcept : Lower(LowerBound), Upper(UpperBound)
		{
			VI_ASSERT(Lower <= Upper, "lower should be smaller than upper");
			Volume = Geometric::AabbVolume(Lower, Upper);
			Radius = ((Upper - Lower) * 0.5f).Sum();
			Center = (Lower + Upper) * 0.5f;
		}
		void Bounding::Merge(const Bounding& A, const Bounding& B)
		{
			Lower.X = std::min(A.Lower.X, B.Lower.X);
			Lower.Y = std::min(A.Lower.Y, B.Lower.Y);
			Lower.Z = std::min(A.Lower.Z, B.Lower.Z);
			Upper.X = std::max(A.Upper.X, B.Upper.X);
			Upper.Y = std::max(A.Upper.Y, B.Upper.Y);
			Upper.Z = std::max(A.Upper.Z, B.Upper.Z);
			Volume = Geometric::AabbVolume(Lower, Upper);
			Radius = ((Upper - Lower) * 0.5f).Sum();
			Center = (Lower + Upper) * 0.5f;
		}
		bool Bounding::Contains(const Bounding& Bounds) const
		{
			return Bounds.Lower >= Lower && Bounds.Upper <= Upper;
		}
		bool Bounding::Overlaps(const Bounding& Bounds) const
		{
			return Bounds.Upper >= Lower && Bounds.Lower <= Upper;
		}

		Frustum8C::Frustum8C() noexcept
		{
			Geometric::CreateFrustum8C(Corners, 90.0f, 1.0f, 0.1f, 1.0f);
		}
		Frustum8C::Frustum8C(float FieldOfView, float Aspect, float NearZ, float FarZ) noexcept
		{
			Geometric::CreateFrustum8CRad(Corners, FieldOfView, Aspect, NearZ, FarZ);
		}
		void Frustum8C::Transform(const Matrix4x4& Value)
		{
			for (uint32_t i = 0; i < 8; i++)
			{
				Vector4& Corner = Corners[i];
				Corner = Corner * Value;
			}
		}
		void Frustum8C::GetBoundingBox(Vector2* X, Vector2* Y, Vector2* Z)
		{
			VI_ASSERT(X || Y || Z, "at least one vector of x, y, z should be set");
			float MinX = std::numeric_limits<float>::max();
			float MaxX = std::numeric_limits<float>::min();
			float MinY = std::numeric_limits<float>::max();
			float MaxY = std::numeric_limits<float>::min();
			float MinZ = std::numeric_limits<float>::max();
			float MaxZ = std::numeric_limits<float>::min();

			for (uint32_t i = 0; i < 8; i++)
			{
				Vector4& Corner = Corners[i];
				MinX = std::min(MinX, Corner.X);
				MaxX = std::max(MaxX, Corner.X);
				MinY = std::min(MinY, Corner.Y);
				MaxY = std::max(MaxY, Corner.Y);
				MinZ = std::min(MinZ, Corner.Z);
				MaxZ = std::max(MaxZ, Corner.Z);
			}

			if (X != nullptr)
				*X = Vector2(MinX, MaxX);

			if (Y != nullptr)
				*Y = Vector2(MinY, MaxY);

			if (Z != nullptr)
				*Z = Vector2(MinZ, MaxZ);
		}

		Frustum6P::Frustum6P() noexcept
		{
		}
		Frustum6P::Frustum6P(const Matrix4x4& Clip) noexcept
		{
			Planes[(size_t)Side::RIGHT].X = Clip[3] - Clip[0];
			Planes[(size_t)Side::RIGHT].Y = Clip[7] - Clip[4];
			Planes[(size_t)Side::RIGHT].Z = Clip[11] - Clip[8];
			Planes[(size_t)Side::RIGHT].W = Clip[15] - Clip[12];
			NormalizePlane(Planes[(size_t)Side::RIGHT]);

			Planes[(size_t)Side::LEFT].X = Clip[3] + Clip[0];
			Planes[(size_t)Side::LEFT].Y = Clip[7] + Clip[4];
			Planes[(size_t)Side::LEFT].Z = Clip[11] + Clip[8];
			Planes[(size_t)Side::LEFT].W = Clip[15] + Clip[12];
			NormalizePlane(Planes[(size_t)Side::LEFT]);

			Planes[(size_t)Side::BOTTOM].X = Clip[3] + Clip[1];
			Planes[(size_t)Side::BOTTOM].Y = Clip[7] + Clip[5];
			Planes[(size_t)Side::BOTTOM].Z = Clip[11] + Clip[9];
			Planes[(size_t)Side::BOTTOM].W = Clip[15] + Clip[13];
			NormalizePlane(Planes[(size_t)Side::BOTTOM]);

			Planes[(size_t)Side::TOP].X = Clip[3] - Clip[1];
			Planes[(size_t)Side::TOP].Y = Clip[7] - Clip[5];
			Planes[(size_t)Side::TOP].Z = Clip[11] - Clip[9];
			Planes[(size_t)Side::TOP].W = Clip[15] - Clip[13];
			NormalizePlane(Planes[(size_t)Side::TOP]);

			Planes[(size_t)Side::BACK].X = Clip[3] - Clip[2];
			Planes[(size_t)Side::BACK].Y = Clip[7] - Clip[6];
			Planes[(size_t)Side::BACK].Z = Clip[11] - Clip[10];
			Planes[(size_t)Side::BACK].W = Clip[15] - Clip[14];
			NormalizePlane(Planes[(size_t)Side::BACK]);

			Planes[(size_t)Side::FRONT].X = Clip[3] + Clip[2];
			Planes[(size_t)Side::FRONT].Y = Clip[7] + Clip[6];
			Planes[(size_t)Side::FRONT].Z = Clip[11] + Clip[10];
			Planes[(size_t)Side::FRONT].W = Clip[15] + Clip[14];
			NormalizePlane(Planes[(size_t)Side::FRONT]);
		}
		void Frustum6P::NormalizePlane(Vector4& Plane)
		{
			float Magnitude = (float)sqrt(Plane.X * Plane.X + Plane.Y * Plane.Y + Plane.Z * Plane.Z);
			Plane.X /= Magnitude;
			Plane.Y /= Magnitude;
			Plane.Z /= Magnitude;
			Plane.W /= Magnitude;
		}
		bool Frustum6P::OverlapsAABB(const Bounding& Bounds) const
		{
			const Vector3& Mid = Bounds.Center;
			const Vector3& Min = Bounds.Lower;
			const Vector3& Max = Bounds.Upper;
			float Distance = -Bounds.Radius;
#ifdef VI_VECTORCLASS
			LOAD_AV4(_rc, Mid.X, Mid.Y, Mid.Z, 1.0f);
			LOAD_AV4(_m1, Min.X, Min.Y, Min.Z, 1.0f);
			LOAD_AV4(_m2, Max.X, Min.Y, Min.Z, 1.0f);
			LOAD_AV4(_m3, Min.X, Max.Y, Min.Z, 1.0f);
			LOAD_AV4(_m4, Max.X, Max.Y, Min.Z, 1.0f);
			LOAD_AV4(_m5, Min.X, Min.Y, Max.Z, 1.0f);
			LOAD_AV4(_m6, Max.X, Min.Y, Max.Z, 1.0f);
			LOAD_AV4(_m7, Min.X, Max.Y, Max.Z, 1.0f);
			LOAD_AV4(_m8, Max.X, Max.Y, Max.Z, 1.0f);
#else
			Vector4 RC(Mid.X, Mid.Y, Mid.Z, 1.0f);
			Vector4 M1(Min.X, Min.Y, Min.Z, 1.0f);
			Vector4 M2(Max.X, Min.Y, Min.Z, 1.0f);
			Vector4 M3(Min.X, Max.Y, Min.Z, 1.0f);
			Vector4 M4(Max.X, Max.Y, Min.Z, 1.0f);
			Vector4 M5(Min.X, Min.Y, Max.Z, 1.0f);
			Vector4 M6(Max.X, Min.Y, Max.Z, 1.0f);
			Vector4 M7(Min.X, Max.Y, Max.Z, 1.0f);
			Vector4 M8(Max.X, Max.Y, Max.Z, 1.0f);
#endif
			for (size_t i = 0; i < 6; i++)
			{
#ifdef VI_VECTORCLASS
				LOAD_V4(_rp, Planes[i]);
				if (horizontal_add(_rc * _rp) < Distance)
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
				auto& Plane = Planes[i];
				if (Plane.Dot(RC) < Distance)
					return false;

				if (Plane.Dot(M1) > 0)
					continue;

				if (Plane.Dot(M2) > 0)
					continue;

				if (Plane.Dot(M3) > 0)
					continue;

				if (Plane.Dot(M4) > 0)
					continue;

				if (Plane.Dot(M5) > 0)
					continue;

				if (Plane.Dot(M6) > 0)
					continue;

				if (Plane.Dot(M7) > 0)
					continue;

				if (Plane.Dot(M8) > 0)
					continue;
#endif
				return false;
			}

			return true;
		}
		bool Frustum6P::OverlapsSphere(const Vector3& Center, float Radius) const
		{
			Vector4 Position(Center.X, Center.Y, Center.Z, 1.0f);
			float Distance = -Radius;

			for (size_t i = 0; i < 6; i++)
			{
				if (Position.Dot(Planes[i]) < Distance)
					return false;
			}

			return true;
		}

		Ray::Ray() noexcept : Direction(0, 0, 1)
		{
		}
		Ray::Ray(const Vector3& _Origin, const Vector3& _Direction) noexcept : Origin(_Origin), Direction(_Direction)
		{
		}
		Vector3 Ray::GetPoint(float T) const
		{
			return Origin + (Direction * T);
		}
		Vector3 Ray::operator *(float T) const
		{
			return GetPoint(T);
		}
		bool Ray::IntersectsPlane(const Vector3& Normal, float Diameter) const
		{
			float D = Normal.Dot(Direction);
			if (Compute::Mathf::Abs(D) < std::numeric_limits<float>::epsilon())
				return false;

			float N = Normal.Dot(Origin) + Diameter;
			float T = -(N / D);
			return T >= 0;
		}
		bool Ray::IntersectsSphere(const Vector3& Position, float Radius, bool DiscardInside) const
		{
			Vector3 R = Origin - Position;
			float L = R.Length();

			if (L * L <= Radius * Radius && DiscardInside)
				return true;

			float A = Direction.Dot(Direction);
			float B = 2 * R.Dot(Direction);
			float C = R.Dot(R) - Radius * Radius;
			float D = (B * B) - (4 * A * C);

			return D >= 0.0f;
		}
		bool Ray::IntersectsAABBAt(const Vector3& Min, const Vector3& Max, Vector3* Hit) const
		{
			Vector3 HitPoint; float T;
			if (Origin > Min && Origin < Max)
				return true;

			if (Origin.X <= Min.X && Direction.X > 0)
			{
				T = (Min.X - Origin.X) / Direction.X;
				HitPoint = Origin + Direction * T;

				if (HitPoint.Y >= Min.Y && HitPoint.Y <= Max.Y && HitPoint.Z >= Min.Z && HitPoint.Z <= Max.Z)
				{
					if (Hit != nullptr)
						*Hit = HitPoint;

					return true;
				}
			}

			if (Origin.X >= Max.X && Direction.X < 0)
			{
				T = (Max.X - Origin.X) / Direction.X;
				HitPoint = Origin + Direction * T;

				if (HitPoint.Y >= Min.Y && HitPoint.Y <= Max.Y && HitPoint.Z >= Min.Z && HitPoint.Z <= Max.Z)
				{
					if (Hit != nullptr)
						*Hit = HitPoint;

					return true;
				}
			}

			if (Origin.Y <= Min.Y && Direction.Y > 0)
			{
				T = (Min.Y - Origin.Y) / Direction.Y;
				HitPoint = Origin + Direction * T;

				if (HitPoint.X >= Min.X && HitPoint.X <= Max.X && HitPoint.Z >= Min.Z && HitPoint.Z <= Max.Z)
				{
					if (Hit != nullptr)
						*Hit = HitPoint;

					return true;
				}
			}

			if (Origin.Y >= Max.Y && Direction.Y < 0)
			{
				T = (Max.Y - Origin.Y) / Direction.Y;
				HitPoint = Origin + Direction * T;

				if (HitPoint.X >= Min.X && HitPoint.X <= Max.X && HitPoint.Z >= Min.Z && HitPoint.Z <= Max.Z)
				{
					if (Hit != nullptr)
						*Hit = HitPoint;

					return true;
				}
			}

			if (Origin.Z <= Min.Z && Direction.Z > 0)
			{
				T = (Min.Z - Origin.Z) / Direction.Z;
				HitPoint = Origin + Direction * T;

				if (HitPoint.X >= Min.X && HitPoint.X <= Max.X && HitPoint.Y >= Min.Y && HitPoint.Y <= Max.Y)
				{
					if (Hit != nullptr)
						*Hit = HitPoint;

					return true;
				}
			}

			if (Origin.Z >= Max.Z && Direction.Z < 0)
			{
				T = (Max.Z - Origin.Z) / Direction.Z;
				HitPoint = Origin + Direction * T;

				if (HitPoint.X >= Min.X && HitPoint.X <= Max.X && HitPoint.Y >= Min.Y && HitPoint.Y <= Max.Y)
				{
					if (Hit != nullptr)
						*Hit = HitPoint;

					return true;
				}
			}

			return false;
		}
		bool Ray::IntersectsAABB(const Vector3& Position, const Vector3& Scale, Vector3* Hit) const
		{
			Vector3 Min = Position - Scale;
			Vector3 Max = Position + Scale;

			return IntersectsAABBAt(Min, Max, Hit);
		}
		bool Ray::IntersectsOBB(const Matrix4x4& World, Vector3* Hit) const
		{
			Matrix4x4 Offset = World.Inv();
			Vector3 Min = -1.0f, Max = 1.0f;
			Vector3 O = (Vector4(Origin.X, Origin.Y, Origin.Z, 1.0f) * Offset).XYZ();
			if (O > Min && O < Max)
				return true;

			Vector3 D = (Direction.XYZW() * Offset).sNormalize().XYZ();
			Vector3 HitPoint; float T;

			if (O.X <= Min.X && D.X > 0)
			{
				T = (Min.X - O.X) / D.X;
				HitPoint = O + D * T;

				if (HitPoint.Y >= Min.Y && HitPoint.Y <= Max.Y && HitPoint.Z >= Min.Z && HitPoint.Z <= Max.Z)
				{
					if (Hit != nullptr)
						*Hit = HitPoint.Transform(World);

					return true;
				}
			}

			if (O.X >= Max.X && D.X < 0)
			{
				T = (Max.X - O.X) / D.X;
				HitPoint = O + D * T;

				if (HitPoint.Y >= Min.Y && HitPoint.Y <= Max.Y && HitPoint.Z >= Min.Z && HitPoint.Z <= Max.Z)
				{
					if (Hit != nullptr)
						*Hit = HitPoint.Transform(World);

					return true;
				}
			}

			if (O.Y <= Min.Y && D.Y > 0)
			{
				T = (Min.Y - O.Y) / D.Y;
				HitPoint = O + D * T;

				if (HitPoint.X >= Min.X && HitPoint.X <= Max.X && HitPoint.Z >= Min.Z && HitPoint.Z <= Max.Z)
				{
					if (Hit != nullptr)
						*Hit = HitPoint.Transform(World);

					return true;
				}
			}

			if (O.Y >= Max.Y && D.Y < 0)
			{
				T = (Max.Y - O.Y) / D.Y;
				HitPoint = O + D * T;

				if (HitPoint.X >= Min.X && HitPoint.X <= Max.X && HitPoint.Z >= Min.Z && HitPoint.Z <= Max.Z)
				{
					if (Hit != nullptr)
						*Hit = HitPoint.Transform(World);

					return true;
				}
			}

			if (O.Z <= Min.Z && D.Z > 0)
			{
				T = (Min.Z - O.Z) / D.Z;
				HitPoint = O + D * T;

				if (HitPoint.X >= Min.X && HitPoint.X <= Max.X && HitPoint.Y >= Min.Y && HitPoint.Y <= Max.Y)
				{
					if (Hit != nullptr)
						*Hit = HitPoint.Transform(World);

					return true;
				}
			}

			if (O.Z >= Max.Z && D.Z < 0)
			{
				T = (Max.Z - O.Z) / D.Z;
				HitPoint = O + D * T;

				if (HitPoint.X >= Min.X && HitPoint.X <= Max.X && HitPoint.Y >= Min.Y && HitPoint.Y <= Max.Y)
				{
					if (Hit != nullptr)
						*Hit = HitPoint.Transform(World);

					return true;
				}
			}

			return false;
		}

		Matrix4x4::Matrix4x4() noexcept : Row { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 }
		{
		}
		Matrix4x4::Matrix4x4(float Array[16]) noexcept
		{
			memcpy(Row, Array, sizeof(float) * 16);
		}
		Matrix4x4::Matrix4x4(const Vector4& row0, const Vector4& row1, const Vector4& row2, const Vector4& row3) noexcept
		{
			memcpy(Row + 0, &row0, sizeof(Vector4));
			memcpy(Row + 4, &row1, sizeof(Vector4));
			memcpy(Row + 8, &row2, sizeof(Vector4));
			memcpy(Row + 12, &row3, sizeof(Vector4));
		}
		Matrix4x4::Matrix4x4(float row00, float row01, float row02, float row03, float row10, float row11, float row12, float row13, float row20, float row21, float row22, float row23, float row30, float row31, float row32, float row33) noexcept :
			Row { row00, row01, row02, row03, row10, row11, row12, row13, row20, row21, row22, row23, row30, row31, row32, row33 }
		{
		}
		Matrix4x4::Matrix4x4(bool) noexcept
		{
		}
		Matrix4x4::Matrix4x4(const Matrix4x4& V) noexcept
		{
			memcpy(Row, V.Row, sizeof(float) * 16);
		}
		float& Matrix4x4::operator [](uint32_t Index)
		{
			return Row[Index];
		}
		float Matrix4x4::operator [](uint32_t Index) const
		{
			return Row[Index];
		}
		bool Matrix4x4::operator ==(const Matrix4x4& Equal) const
		{
			return memcmp(Row, Equal.Row, sizeof(float) * 16) == 0;
		}
		bool Matrix4x4::operator !=(const Matrix4x4& Equal) const
		{
			return memcmp(Row, Equal.Row, sizeof(float) * 16) != 0;
		}
		Matrix4x4 Matrix4x4::operator *(const Matrix4x4& V) const
		{
			return this->Mul(V);
		}
		Vector4 Matrix4x4::operator *(const Vector4& V) const
		{
			Matrix4x4 Result = this->Mul(V);
			return Vector4(Result.Row[0], Result.Row[4], Result.Row[8], Result.Row[12]);
		}
		Matrix4x4& Matrix4x4::operator =(const Matrix4x4& V) noexcept
		{
			memcpy(Row, V.Row, sizeof(float) * 16);
			return *this;
		}
		Vector4 Matrix4x4::Row11() const
		{
			return Vector4(Row[0], Row[1], Row[2], Row[3]);
		}
		Vector4 Matrix4x4::Row22() const
		{
			return Vector4(Row[4], Row[5], Row[6], Row[7]);
		}
		Vector4 Matrix4x4::Row33() const
		{
			return Vector4(Row[8], Row[9], Row[10], Row[11]);
		}
		Vector4 Matrix4x4::Row44() const
		{
			return Vector4(Row[12], Row[13], Row[14], Row[15]);
		}
		Vector3 Matrix4x4::Up() const
		{
			return Vector3(-Row[4], Row[5], Row[6]);
		}
		Vector3 Matrix4x4::Right() const
		{
			return Vector3(-Row[0], Row[1], Row[2]);
		}
		Vector3 Matrix4x4::Forward() const
		{
			return Vector3(-Row[8], Row[9], Row[10]);
		}
		Matrix4x4 Matrix4x4::Inv() const
		{
			Matrix4x4 Result(true);
			float A2323 = Row[10] * Row[15] - Row[11] * Row[14];
			float A1323 = Row[9] * Row[15] - Row[11] * Row[13];
			float A1223 = Row[9] * Row[14] - Row[10] * Row[13];
			float A0323 = Row[8] * Row[15] - Row[11] * Row[12];
			float A0223 = Row[8] * Row[14] - Row[10] * Row[12];
			float A0123 = Row[8] * Row[13] - Row[9] * Row[12];
			float A2313 = Row[6] * Row[15] - Row[7] * Row[14];
			float A1313 = Row[5] * Row[15] - Row[7] * Row[13];
			float A1213 = Row[5] * Row[14] - Row[6] * Row[13];
			float A2312 = Row[6] * Row[11] - Row[7] * Row[10];
			float A1312 = Row[5] * Row[11] - Row[7] * Row[9];
			float A1212 = Row[5] * Row[10] - Row[6] * Row[9];
			float A0313 = Row[4] * Row[15] - Row[7] * Row[12];
			float A0213 = Row[4] * Row[14] - Row[6] * Row[12];
			float A0312 = Row[4] * Row[11] - Row[7] * Row[8];
			float A0212 = Row[4] * Row[10] - Row[6] * Row[8];
			float A0113 = Row[4] * Row[13] - Row[5] * Row[12];
			float A0112 = Row[4] * Row[9] - Row[5] * Row[8];
#ifdef VI_VECTORCLASS
			LOAD_AV4(_r1, Row[5], Row[4], Row[4], Row[4]);
			LOAD_AV4(_r2, A2323, A2323, A1323, A1223);
			LOAD_AV4(_r3, Row[6], Row[6], Row[5], Row[5]);
			LOAD_AV4(_r4, A1323, A0323, A0323, A0223);
			LOAD_AV4(_r5, Row[7], Row[7], Row[7], Row[6]);
			LOAD_AV4(_r6, A1223, A0223, A0123, A0123);
			LOAD_AV4(_r7, Row[0], -Row[1], Row[2], -Row[3]);
			LOAD_AV4(_r8, 1.0f, -1.0f, 1.0f, -1.0f);
			_r7 *= _r1 * _r2 - _r3 * _r4 + _r5 * _r6;
			float F = horizontal_add(_r7);
			F = 1.0f / (F != 0.0f ? F : 1.0f);
			_r1 = Vec4f(Row[5], Row[1], Row[1], Row[1]);
			_r2 = Vec4f(A2323, A2323, A2313, A2312);
			_r3 = Vec4f(Row[6], Row[2], Row[2], Row[2]);
			_r4 = Vec4f(A1323, A1323, A1313, A1312);
			_r5 = Vec4f(Row[7], Row[3], Row[3], Row[3]);
			_r6 = Vec4f(A1223, A1223, A1213, A1212);
			_r7 = (_r1 * _r2 - _r3 * _r4 + _r5 * _r6) * _r8 * F;
			_r7.store(Result.Row + 0);
			_r1 = Vec4f(Row[4], Row[0], Row[0], Row[0]);
			_r4 = Vec4f(A0323, A0323, A0313, A0312);
			_r6 = Vec4f(A0223, A0223, A0213, A0212);
			_r7 = (_r1 * _r2 - _r3 * _r4 + _r5 * _r6) * -_r8 * F;
			_r7.store(Result.Row + 4);
			_r2 = Vec4f(A1323, A1323, A1313, A1312);
			_r3 = Vec4f(Row[5], Row[1], Row[1], Row[1]);
			_r6 = Vec4f(A0123, A0123, A0113, A0112);
			_r7 = (_r1 * _r2 - _r3 * _r4 + _r5 * _r6) * _r8 * F;
			_r7.store(Result.Row + 8);
			_r2 = Vec4f(A1223, A1223, A1213, A1212);
			_r4 = Vec4f(A0223, A0223, A0213, A0212);
			_r5 = Vec4f(Row[6], Row[2], Row[2], Row[2]);
			_r7 = (_r1 * _r2 - _r3 * _r4 + _r5 * _r6) * -_r8 * F;
			_r7.store(Result.Row + 12);
#else
			float F =
				Row[0] * (Row[5] * A2323 - Row[6] * A1323 + Row[7] * A1223)
				- Row[1] * (Row[4] * A2323 - Row[6] * A0323 + Row[7] * A0223)
				+ Row[2] * (Row[4] * A1323 - Row[5] * A0323 + Row[7] * A0123)
				- Row[3] * (Row[4] * A1223 - Row[5] * A0223 + Row[6] * A0123);
			F = 1.0f / (F != 0.0f ? F : 1.0f);

			Result.Row[0] = F * (Row[5] * A2323 - Row[6] * A1323 + Row[7] * A1223);
			Result.Row[1] = F * -(Row[1] * A2323 - Row[2] * A1323 + Row[3] * A1223);
			Result.Row[2] = F * (Row[1] * A2313 - Row[2] * A1313 + Row[3] * A1213);
			Result.Row[3] = F * -(Row[1] * A2312 - Row[2] * A1312 + Row[3] * A1212);
			Result.Row[4] = F * -(Row[4] * A2323 - Row[6] * A0323 + Row[7] * A0223);
			Result.Row[5] = F * (Row[0] * A2323 - Row[2] * A0323 + Row[3] * A0223);
			Result.Row[6] = F * -(Row[0] * A2313 - Row[2] * A0313 + Row[3] * A0213);
			Result.Row[7] = F * (Row[0] * A2312 - Row[2] * A0312 + Row[3] * A0212);
			Result.Row[8] = F * (Row[4] * A1323 - Row[5] * A0323 + Row[7] * A0123);
			Result.Row[9] = F * -(Row[0] * A1323 - Row[1] * A0323 + Row[3] * A0123);
			Result.Row[10] = F * (Row[0] * A1313 - Row[1] * A0313 + Row[3] * A0113);
			Result.Row[11] = F * -(Row[0] * A1312 - Row[1] * A0312 + Row[3] * A0112);
			Result.Row[12] = F * -(Row[4] * A1223 - Row[5] * A0223 + Row[6] * A0123);
			Result.Row[13] = F * (Row[0] * A1223 - Row[1] * A0223 + Row[2] * A0123);
			Result.Row[14] = F * -(Row[0] * A1213 - Row[1] * A0213 + Row[2] * A0113);
			Result.Row[15] = F * (Row[0] * A1212 - Row[1] * A0212 + Row[2] * A0112);
#endif
			return Result;
		}
		Matrix4x4 Matrix4x4::Transpose() const
		{
#ifdef VI_VECTORCLASS
			LOAD_FV16(_r1);
			_r1 = permute16f<0, 4, 8, 12, 1, 5, 9, 13, 2, 6, 10, 14, 3, 7, 11, 15>(_r1);

			Matrix4x4 Result(true);
			_r1.store(Result.Row);

			return Result;
#else
			return Matrix4x4(
				Vector4(Row[0], Row[4], Row[8], Row[12]),
				Vector4(Row[1], Row[5], Row[9], Row[13]),
				Vector4(Row[2], Row[6], Row[10], Row[14]),
				Vector4(Row[3], Row[7], Row[11], Row[15]));
#endif
		}
		Quaternion Matrix4x4::RotationQuaternion() const
		{
			Vector3 Scaling[3] =
			{
				Vector3(Row[0], Row[1], Row[2]),
				Vector3(Row[4], Row[5], Row[6]),
				Vector3(Row[8], Row[9], Row[10])
			};

			Vector3 Scale = { Scaling[0].Length(), Scaling[1].Length(), Scaling[2].Length() };
			if (Determinant() < 0)
				Scale = -Scale;

			if (Scale.X)
				Scaling[0] /= Scale.X;

			if (Scale.Y)
				Scaling[1] /= Scale.Y;

			if (Scale.Z)
				Scaling[2] /= Scale.Z;

			Matrix4x4 Rotated =
			{
				Scaling[0].X, Scaling[1].X, Scaling[2].X, 0.0f,
				Scaling[0].Y, Scaling[1].Y, Scaling[2].Y, 0.0f,
				Scaling[0].Z, Scaling[1].Z, Scaling[2].Z, 0.0f,
				0.0f, 0.0f, 0.0f, 1.0f
			};

			return Quaternion(Rotated);
		}
		Vector3 Matrix4x4::RotationEuler() const
		{
			return RotationQuaternion().GetEuler();
		}
		Vector3 Matrix4x4::Position() const
		{
			return Vector3(Row[12], Row[13], Row[14]);
		}
		Vector3 Matrix4x4::Scale() const
		{
			Vector3 Scale =
			{
				Vector3(Row[0], Row[1], Row[2]).Length(),
				Vector3(Row[4], Row[5], Row[6]).Length(),
				Vector3(Row[8], Row[9], Row[10]).Length()
			};
			
			if (Determinant() < 0)
				Scale = -Scale;

			return Scale;
		}
		Matrix4x4 Matrix4x4::SetScale(const Vector3& Value) const
		{
			Matrix4x4 Local = *this;
			Local.Row[0] = Value.X;
			Local.Row[5] = Value.Y;
			Local.Row[10] = Value.Z;
			return Local;
		}
		Matrix4x4 Matrix4x4::Mul(const Matrix4x4& V) const
		{
			Matrix4x4 Result;
#ifdef VI_VECTORCLASS
			LOAD_VAR(_r1, V.Row + 0);
			LOAD_VAR(_r2, V.Row + 4);
			LOAD_VAR(_r3, V.Row + 8);
			LOAD_VAR(_r4, V.Row + 12);
			LOAD_VAL(_r5, 0.0f);

			_r5 += _r1 * Row[0];
			_r5 += _r2 * Row[1];
			_r5 += _r3 * Row[2];
			_r5 += _r4 * Row[3];
			_r5.store(Result.Row + 0);
			_r5 = Vec4f(0.0f);
			_r5 += _r1 * Row[4];
			_r5 += _r2 * Row[5];
			_r5 += _r3 * Row[6];
			_r5 += _r4 * Row[7];
			_r5.store(Result.Row + 4);
			_r5 = Vec4f(0.0f);
			_r5 += _r1 * Row[8];
			_r5 += _r2 * Row[9];
			_r5 += _r3 * Row[10];
			_r5 += _r4 * Row[11];
			_r5.store(Result.Row + 8);
			_r5 = Vec4f(0.0f);
			_r5 += _r1 * Row[12];
			_r5 += _r2 * Row[13];
			_r5 += _r3 * Row[14];
			_r5 += _r4 * Row[15];
			_r5.store(Result.Row + 12);
#else
			Result.Row[0] = (Row[0] * V.Row[0]) + (Row[1] * V.Row[4]) + (Row[2] * V.Row[8]) + (Row[3] * V.Row[12]);
			Result.Row[1] = (Row[0] * V.Row[1]) + (Row[1] * V.Row[5]) + (Row[2] * V.Row[9]) + (Row[3] * V.Row[13]);
			Result.Row[2] = (Row[0] * V.Row[2]) + (Row[1] * V.Row[6]) + (Row[2] * V.Row[10]) + (Row[3] * V.Row[14]);
			Result.Row[3] = (Row[0] * V.Row[3]) + (Row[1] * V.Row[7]) + (Row[2] * V.Row[11]) + (Row[3] * V.Row[15]);
			Result.Row[4] = (Row[4] * V.Row[0]) + (Row[5] * V.Row[4]) + (Row[6] * V.Row[8]) + (Row[7] * V.Row[12]);
			Result.Row[5] = (Row[4] * V.Row[1]) + (Row[5] * V.Row[5]) + (Row[6] * V.Row[9]) + (Row[7] * V.Row[13]);
			Result.Row[6] = (Row[4] * V.Row[2]) + (Row[5] * V.Row[6]) + (Row[6] * V.Row[10]) + (Row[7] * V.Row[14]);
			Result.Row[7] = (Row[4] * V.Row[3]) + (Row[5] * V.Row[7]) + (Row[6] * V.Row[11]) + (Row[7] * V.Row[15]);
			Result.Row[8] = (Row[8] * V.Row[0]) + (Row[9] * V.Row[4]) + (Row[10] * V.Row[8]) + (Row[11] * V.Row[12]);
			Result.Row[9] = (Row[8] * V.Row[1]) + (Row[9] * V.Row[5]) + (Row[10] * V.Row[9]) + (Row[11] * V.Row[13]);
			Result.Row[10] = (Row[8] * V.Row[2]) + (Row[9] * V.Row[6]) + (Row[10] * V.Row[10]) + (Row[11] * V.Row[14]);
			Result.Row[11] = (Row[8] * V.Row[3]) + (Row[9] * V.Row[7]) + (Row[10] * V.Row[11]) + (Row[11] * V.Row[15]);
			Result.Row[12] = (Row[12] * V.Row[0]) + (Row[13] * V.Row[4]) + (Row[14] * V.Row[8]) + (Row[15] * V.Row[12]);
			Result.Row[13] = (Row[12] * V.Row[1]) + (Row[13] * V.Row[5]) + (Row[14] * V.Row[9]) + (Row[15] * V.Row[13]);
			Result.Row[14] = (Row[12] * V.Row[2]) + (Row[13] * V.Row[6]) + (Row[14] * V.Row[10]) + (Row[15] * V.Row[14]);
			Result.Row[15] = (Row[12] * V.Row[3]) + (Row[13] * V.Row[7]) + (Row[14] * V.Row[11]) + (Row[15] * V.Row[15]);
#endif
			return Result;
		}
		Matrix4x4 Matrix4x4::Mul(const Vector4& V) const
		{
			Matrix4x4 Result;
#ifdef VI_VECTORCLASS
			LOAD_V4(_r1, V);
			LOAD_VAR(_r2, Row + 0);
			LOAD_VAR(_r3, Row + 4);
			LOAD_VAR(_r4, Row + 8);
			LOAD_VAR(_r5, Row + 12);
			LOAD_VAL(_r6, 0.0f);

			_r6 = horizontal_add(_r1 * _r2);
			_r6.store(Result.Row + 0);
			_r6 = horizontal_add(_r1 * _r3);
			_r6.store(Result.Row + 4);
			_r6 = horizontal_add(_r1 * _r4);
			_r6.store(Result.Row + 8);
			_r6 = horizontal_add(_r1 * _r5);
			_r6.store(Result.Row + 12);
#else
			float X = (Row[0] * V.X) + (Row[1] * V.Y) + (Row[2] * V.Z) + (Row[3] * V.W);
			Result.Row[0] = Result.Row[1] = Result.Row[2] = Result.Row[3] = X;

			float Y = (Row[4] * V.X) + (Row[5] * V.Y) + (Row[6] * V.Z) + (Row[7] * V.W);
			Result.Row[4] = Result.Row[5] = Result.Row[6] = Result.Row[7] = Y;

			float Z = (Row[8] * V.X) + (Row[9] * V.Y) + (Row[10] * V.Z) + (Row[11] * V.W);
			Result.Row[8] = Result.Row[9] = Result.Row[10] = Result.Row[11] = Z;

			float W = (Row[12] * V.X) + (Row[13] * V.Y) + (Row[14] * V.Z) + (Row[15] * V.W);
			Result.Row[12] = Result.Row[13] = Result.Row[14] = Result.Row[15] = W;
#endif
			return Result;
		}
		Vector2 Matrix4x4::XY() const
		{
#ifdef VI_VECTORCLASS
			LOAD_AV4(_r1, Row[0], Row[4], Row[8], Row[12]);
			LOAD_AV4(_r2, Row[1], Row[5], Row[9], Row[13]);
			return Vector2(horizontal_add(_r1), horizontal_add(_r2));
#else
			return Vector2(
				Row[0] + Row[4] + Row[8] + Row[12],
				Row[1] + Row[5] + Row[9] + Row[13]);
#endif
		}
		Vector3 Matrix4x4::XYZ() const
		{
#ifdef VI_VECTORCLASS
			LOAD_AV4(_r1, Row[0], Row[4], Row[8], Row[12]);
			LOAD_AV4(_r2, Row[1], Row[5], Row[9], Row[13]);
			LOAD_AV4(_r3, Row[2], Row[6], Row[10], Row[14]);
			return Vector3(horizontal_add(_r1), horizontal_add(_r2), horizontal_add(_r3));
#else
			return Vector3(
				Row[0] + Row[4] + Row[8] + Row[12],
				Row[1] + Row[5] + Row[9] + Row[13],
				Row[2] + Row[6] + Row[10] + Row[14]);
#endif
		}
		Vector4 Matrix4x4::XYZW() const
		{
#ifdef VI_VECTORCLASS
			LOAD_AV4(_r1, Row[0], Row[4], Row[8], Row[12]);
			LOAD_AV4(_r2, Row[1], Row[5], Row[9], Row[13]);
			LOAD_AV4(_r3, Row[2], Row[6], Row[10], Row[14]);
			LOAD_AV4(_r4, Row[3], Row[7], Row[11], Row[15]);
			return Vector4(horizontal_add(_r1), horizontal_add(_r2), horizontal_add(_r3), horizontal_add(_r4));
#else
			return Vector4(
				Row[0] + Row[4] + Row[8] + Row[12],
				Row[1] + Row[5] + Row[9] + Row[13],
				Row[2] + Row[6] + Row[10] + Row[14],
				Row[3] + Row[7] + Row[11] + Row[15]);
#endif
		}
		float Matrix4x4::Determinant() const
		{
			return Row[0] * Row[5] * Row[10] * Row[15] - Row[0] * Row[5] * Row[11] * Row[14] + Row[0] * Row[6] * Row[11] * Row[13] - Row[0] * Row[6] * Row[9] * Row[15]
				+ Row[0] * Row[7] * Row[9] * Row[14] - Row[0] * Row[7] * Row[10] * Row[13] - Row[1] * Row[6] * Row[11] * Row[12] + Row[1] * Row[6] * Row[8] * Row[15]
				- Row[1] * Row[7] * Row[8] * Row[14] + Row[1] * Row[7] * Row[10] * Row[12] - Row[1] * Row[4] * Row[10] * Row[15] + Row[1] * Row[4] * Row[11] * Row[14]
				+ Row[2] * Row[7] * Row[8] * Row[13] - Row[2] * Row[7] * Row[9] * Row[12] + Row[2] * Row[4] * Row[9] * Row[15] - Row[2] * Row[4] * Row[11] * Row[13]
				+ Row[2] * Row[5] * Row[11] * Row[12] - Row[2] * Row[5] * Row[8] * Row[15] - Row[3] * Row[4] * Row[9] * Row[14] + Row[3] * Row[4] * Row[10] * Row[13]
				- Row[3] * Row[5] * Row[10] * Row[12] + Row[3] * Row[5] * Row[8] * Row[14] - Row[3] * Row[6] * Row[8] * Row[13] + Row[3] * Row[6] * Row[9] * Row[12];
		}
		void Matrix4x4::Identify()
		{
			static float Base[16] = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
			memcpy(Row, Base, sizeof(float) * 16);
		}
		void Matrix4x4::Set(const Matrix4x4& Value)
		{
			memcpy(Row, Value.Row, sizeof(float) * 16);
		}
		Matrix4x4 Matrix4x4::CreateTranslatedScale(const Vector3& Position, const Vector3& Scale)
		{
			Matrix4x4 Value;
			Value.Row[12] = Position.X;
			Value.Row[13] = Position.Y;
			Value.Row[14] = Position.Z;
			Value.Row[0] = Scale.X;
			Value.Row[5] = Scale.Y;
			Value.Row[10] = Scale.Z;

			return Value;
		}
		Matrix4x4 Matrix4x4::CreateRotationX(float Rotation)
		{
			float Cos = cos(Rotation);
			float Sin = sin(Rotation);
			Matrix4x4 X;
			X.Row[5] = Cos;
			X.Row[6] = Sin;
			X.Row[9] = -Sin;
			X.Row[10] = Cos;

			return X;
		}
		Matrix4x4 Matrix4x4::CreateRotationY(float Rotation)
		{
			float Cos = cos(Rotation);
			float Sin = sin(Rotation);
			Matrix4x4 Y;
			Y.Row[0] = Cos;
			Y.Row[2] = -Sin;
			Y.Row[8] = Sin;
			Y.Row[10] = Cos;

			return Y;
		}
		Matrix4x4 Matrix4x4::CreateRotationZ(float Rotation)
		{
			float Cos = cos(Rotation);
			float Sin = sin(Rotation);
			Matrix4x4 Z;
			Z.Row[0] = Cos;
			Z.Row[1] = Sin;
			Z.Row[4] = -Sin;
			Z.Row[5] = Cos;

			return Z;
		}
		Matrix4x4 Matrix4x4::CreateRotation(const Vector3& Rotation)
		{
			return Matrix4x4::CreateRotationX(Rotation.X) * Matrix4x4::CreateRotationY(Rotation.Y) * Matrix4x4::CreateRotationZ(Rotation.Z);
		}
		Matrix4x4 Matrix4x4::CreateScale(const Vector3& Scale)
		{
			Matrix4x4 Result;
			Result.Row[0] = Scale.X;
			Result.Row[5] = Scale.Y;
			Result.Row[10] = Scale.Z;

			return Result;
		}
		Matrix4x4 Matrix4x4::CreateTranslation(const Vector3& Position)
		{
			Matrix4x4 Result;
			Result.Row[12] = Position.X;
			Result.Row[13] = Position.Y;
			Result.Row[14] = Position.Z;

			return Result;
		}
		Matrix4x4 Matrix4x4::CreatePerspectiveRad(float FieldOfView, float AspectRatio, float NearZ, float FarZ)
		{
			float Height = 1.0f / std::tan(0.5f * FieldOfView);
			float Width = Height / AspectRatio;
			float Depth = 1.0f / (FarZ - NearZ);

			return Matrix4x4(
				Vector4(Width, 0, 0, 0),
				Vector4(0, Height, 0, 0),
				Vector4(0, 0, FarZ * Depth, 1),
				Vector4(0, 0, -NearZ * FarZ * Depth, 0));
		}
		Matrix4x4 Matrix4x4::CreatePerspective(float FieldOfView, float AspectRatio, float NearZ, float FarZ)
		{
			return CreatePerspectiveRad(Compute::Mathf::Deg2Rad() * FieldOfView, AspectRatio, NearZ, FarZ);
		}
		Matrix4x4 Matrix4x4::CreateOrthographic(float Width, float Height, float NearZ, float FarZ)
		{
			if (Geometric::IsLeftHanded())
			{
				float Depth = 1.0f / (FarZ - NearZ);
				return Matrix4x4(
					Vector4(2 / Width, 0, 0, 0),
					Vector4(0, 2 / Height, 0, 0),
					Vector4(0, 0, Depth, 0),
					Vector4(0, 0, -Depth * NearZ, 1));
			}
			else
			{
				float Depth = 1.0f / (NearZ - FarZ);
				return Matrix4x4(
					Vector4(2 / Width, 0, 0, 0),
					Vector4(0, 2 / Height, 0, 0),
					Vector4(0, 0, Depth, 0),
					Vector4(0, 0, Depth * NearZ, 1));
			}
		}
		Matrix4x4 Matrix4x4::CreateOrthographicOffCenter(float Left, float Right, float Bottom, float Top, float NearZ, float FarZ)
		{
			float Width = 1.0f / (Right - Left);
			float Height = 1.0f / (Top - Bottom);
			float Depth = 1.0f / (FarZ - NearZ);

			return Matrix4x4(
				Vector4(2 * Width, 0, 0, 0),
				Vector4(0, 2 * Height, 0, 0),
				Vector4(0, 0, Depth, 0),
				Vector4(-(Left + Right) * Width, -(Top + Bottom) * Height, -Depth * NearZ, 1));
		}
		Matrix4x4 Matrix4x4::Create(const Vector3& Position, const Vector3& Scale, const Vector3& Rotation)
		{
			return Matrix4x4::CreateScale(Scale) * Matrix4x4::Create(Position, Rotation);
		}
		Matrix4x4 Matrix4x4::Create(const Vector3& Position, const Vector3& Rotation)
		{
			return Matrix4x4::CreateRotation(Rotation) * Matrix4x4::CreateTranslation(Position);
		}
		Matrix4x4 Matrix4x4::CreateView(const Vector3& Position, const Vector3& Rotation)
		{
			return
				Matrix4x4::CreateTranslation(-Position) *
				Matrix4x4::CreateRotationY(Rotation.Y) *
				Matrix4x4::CreateRotationX(-Rotation.X) *
				Matrix4x4::CreateScale(Vector3(-1.0f, 1.0f, 1.0f)) *
				Matrix4x4::CreateRotationZ(Rotation.Z);
		}
		Matrix4x4 Matrix4x4::CreateLookAt(const Vector3& Position, const Vector3& Target, const Vector3& Up)
		{
			Vector3 Z = (Target - Position).Normalize();
			Vector3 X = Up.Cross(Z).Normalize();
			Vector3 Y = Z.Cross(X);

			Matrix4x4 Result(true);
			Result.Row[0] = X.X;
			Result.Row[1] = Y.X;
			Result.Row[2] = Z.X;
			Result.Row[3] = 0;
			Result.Row[4] = X.Y;
			Result.Row[5] = Y.Y;
			Result.Row[6] = Z.Y;
			Result.Row[7] = 0;
			Result.Row[8] = X.Z;
			Result.Row[9] = Y.Z;
			Result.Row[10] = Z.Z;
			Result.Row[11] = 0;
			Result.Row[12] = -X.Dot(Position);
			Result.Row[13] = -Y.Dot(Position);
			Result.Row[14] = -Z.Dot(Position);
			Result.Row[15] = 1;

			return Result;
		}
		Matrix4x4 Matrix4x4::CreateRotation(const Vector3& Forward, const Vector3& Up, const Vector3& Right)
		{
			Matrix4x4 Rotation(true);
			Rotation.Row[0] = Right.X;
			Rotation.Row[1] = Right.Y;
			Rotation.Row[2] = Right.Z;
			Rotation.Row[3] = 0;
			Rotation.Row[4] = Up.X;
			Rotation.Row[5] = Up.Y;
			Rotation.Row[6] = Up.Z;
			Rotation.Row[7] = 0;
			Rotation.Row[8] = Forward.X;
			Rotation.Row[9] = Forward.Y;
			Rotation.Row[10] = Forward.Z;
			Rotation.Row[11] = 0;
			Rotation.Row[12] = 0;
			Rotation.Row[13] = 0;
			Rotation.Row[14] = 0;
			Rotation.Row[15] = 1;

			return Rotation;
		}
		Matrix4x4 Matrix4x4::CreateLookAt(CubeFace Face, const Vector3& Position)
		{
			switch (Face)
			{
				case CubeFace::PositiveX:
					return Matrix4x4::CreateLookAt(Position, Position + Vector3(1, 0, 0), Vector3::Up());
				case CubeFace::NegativeX:
					return Matrix4x4::CreateLookAt(Position, Position - Vector3(1, 0, 0), Vector3::Up());
				case CubeFace::PositiveY:
					if (Geometric::IsLeftHanded())
						return Matrix4x4::CreateLookAt(Position, Position + Vector3(0, 1, 0), Vector3::Backward());
					else
						return Matrix4x4::CreateLookAt(Position, Position - Vector3(0, 1, 0), Vector3::Forward());
				case CubeFace::NegativeY:
					if (Geometric::IsLeftHanded())
						return Matrix4x4::CreateLookAt(Position, Position - Vector3(0, 1, 0), Vector3::Forward());
					else
						return Matrix4x4::CreateLookAt(Position, Position + Vector3(0, 1, 0), Vector3::Backward());
				case CubeFace::PositiveZ:
					return Matrix4x4::CreateLookAt(Position, Position + Vector3(0, 0, 1), Vector3::Up());
				case CubeFace::NegativeZ:
					return Matrix4x4::CreateLookAt(Position, Position - Vector3(0, 0, 1), Vector3::Up());
				default:
					return Matrix4x4::Identity();
			}
		}

		Quaternion::Quaternion() noexcept : X(0.0f), Y(0.0f), Z(0.0f), W(0.0f)
		{
		}
		Quaternion::Quaternion(float x, float y, float z, float w) noexcept : X(x), Y(y), Z(z), W(w)
		{
		}
		Quaternion::Quaternion(const Quaternion& In) noexcept : X(In.X), Y(In.Y), Z(In.Z), W(In.W)
		{
		}
		Quaternion::Quaternion(const Vector3& Axis, float Angle) noexcept
		{
			SetAxis(Axis, Angle);
		}
		Quaternion::Quaternion(const Vector3& Euler) noexcept
		{
			SetEuler(Euler);
		}
		Quaternion::Quaternion(const Matrix4x4& Value) noexcept
		{
			SetMatrix(Value);
		}
		void Quaternion::SetAxis(const Vector3& Axis, float Angle)
		{
#ifdef VI_VECTORCLASS
			LOAD_V3(_r1, Axis);
			_r1 *= std::sin(Angle / 2);
			_r1.insert(3, std::cos(Angle / 2));
			_r1.store((float*)this);
#else
			float Sin = std::sin(Angle / 2);
			X = Axis.X * Sin;
			Y = Axis.Y * Sin;
			Z = Axis.Z * Sin;
			W = std::cos(Angle / 2);
#endif
		}
		void Quaternion::SetEuler(const Vector3& V)
		{
#ifdef VI_VECTORCLASS
			float _sx[4], _cx[4];
			LOAD_V3(_r1, V);
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
			float SinX = std::sin(V.X / 2);
			float CosX = std::cos(V.X / 2);
			float SinY = std::sin(V.Y / 2);
			float CosY = std::cos(V.Y / 2);
			float SinZ = std::sin(V.Z / 2);
			float CosZ = std::cos(V.Z / 2);
			X = SinX * CosY;
			Y = CosX * SinY;
			Z = SinX * SinY;
			W = CosX * CosY;

			float fX = X * CosZ + Y * SinZ;
			float fY = Y * CosZ - X * SinZ;
			float fZ = Z * CosZ + W * SinZ;
			float fW = W * CosZ - Z * SinZ;
			X = fX;
			Y = fY;
			Z = fZ;
			W = fW;
#endif
		}
		void Quaternion::SetMatrix(const Matrix4x4& Value)
		{
			float T = Value.Row[0] + Value.Row[5] + Value.Row[10];
			if (T > 0.0f)
			{
				float S = std::sqrt(1 + T) * 2.0f;
				X = (Value.Row[9] - Value.Row[6]) / S;
				Y = (Value.Row[2] - Value.Row[8]) / S;
				Z = (Value.Row[4] - Value.Row[1]) / S;
				W = 0.25f * S;
			}
			else if (Value.Row[0] > Value.Row[5] && Value.Row[0] > Value.Row[10])
			{
				float S = std::sqrt(1.0f + Value.Row[0] - Value.Row[5] - Value.Row[10]) * 2.0f;
				X = 0.25f * S;
				Y = (Value.Row[4] + Value.Row[1]) / S;
				Z = (Value.Row[2] + Value.Row[8]) / S;
				W = (Value.Row[9] - Value.Row[6]) / S;
			}
			else if (Value.Row[5] > Value.Row[10])
			{
				float S = std::sqrt(1.0f + Value.Row[5] - Value.Row[0] - Value.Row[10]) * 2.0f;
				X = (Value.Row[4] + Value.Row[1]) / S;
				Y = 0.25f * S;
				Z = (Value.Row[9] + Value.Row[6]) / S;
				W = (Value.Row[2] - Value.Row[8]) / S;
			}
			else
			{
				float S = std::sqrt(1.0f + Value.Row[10] - Value.Row[0] - Value.Row[5]) * 2.0f;
				X = (Value.Row[2] + Value.Row[8]) / S;
				Y = (Value.Row[9] + Value.Row[6]) / S;
				Z = 0.25f * S;
				W = (Value.Row[4] - Value.Row[1]) / S;
			}
		}
		void Quaternion::Set(const Quaternion& Value)
		{
			X = Value.X;
			Y = Value.Y;
			Z = Value.Z;
			W = Value.W;
		}
		Quaternion Quaternion::operator *(float R) const
		{
			return Mul(R);
		}
		Vector3 Quaternion::operator *(const Vector3& R) const
		{
			return Mul(R);
		}
		Quaternion Quaternion::operator *(const Quaternion& R) const
		{
			return Mul(R);
		}
		Quaternion Quaternion::operator -(const Quaternion& R) const
		{
			return Sub(R);
		}
		Quaternion Quaternion::operator +(const Quaternion& R) const
		{
			return Add(R);
		}
		Quaternion& Quaternion::operator =(const Quaternion& R) noexcept
		{
			this->X = R.X;
			this->Y = R.Y;
			this->Z = R.Z;
			this->W = R.W;
			return *this;
		}
		Quaternion Quaternion::Normalize() const
		{
#ifdef VI_VECTORCLASS
			LOAD_FV4(_r1);
			_r1 /= Geometric::FastSqrt(horizontal_add(square(_r1)));

			Quaternion Result;
			_r1.store((float*)&Result);
			return Result;
#else
			float F = Length();
			return Quaternion(X / F, Y / F, Z / F, W / F);
#endif
		}
		Quaternion Quaternion::sNormalize() const
		{
#ifdef VI_VECTORCLASS
			LOAD_FV4(_r1);
			float F = Geometric::FastSqrt(horizontal_add(square(_r1)));
			if (F == 0.0f)
				return Quaternion();

			Quaternion Result;
			_r1 /= F;
			_r1.store((float*)&Result);
			return Result;
#else
			float F = Length();
			if (F == 0.0f)
				return Quaternion();

			return Quaternion(X / F, Y / F, Z / F, W / F);
#endif
		}
		Quaternion Quaternion::Conjugate() const
		{
			return Quaternion(-X, -Y, -Z, W);
		}
		Quaternion Quaternion::Mul(float R) const
		{
#ifdef VI_VECTORCLASS
			LOAD_FV4(_r1);
			_r1 *= R;

			Quaternion Result;
			_r1.store((float*)&Result);
			return Result;
#else
			return Quaternion(X * R, Y * R, Z * R, W * R);
#endif
		}
		Quaternion Quaternion::Mul(const Quaternion& R) const
		{
#ifdef VI_VECTORCLASS
			LOAD_AV4(_r1, W, -X, -Y, -Z);
			LOAD_AV4(_r2, X, W, Y, -Z);
			LOAD_AV4(_r3, Y, W, Z, -X);
			LOAD_AV4(_r4, Z, W, X, -Y);
			LOAD_AV4(_r5, R.W, R.X, R.Y, R.Z);
			LOAD_AV4(_r6, R.W, R.X, R.Z, R.Y);
			LOAD_AV4(_r7, R.W, R.Y, R.X, R.Z);
			LOAD_AV4(_r8, R.W, R.Z, R.Y, R.X);
			float W1 = horizontal_add(_r1 * _r5);
			float X1 = horizontal_add(_r2 * _r6);
			float Y1 = horizontal_add(_r3 * _r7);
			float Z1 = horizontal_add(_r4 * _r8);
#else
			float W1 = W * R.W - X * R.X - Y * R.Y - Z * R.Z;
			float X1 = X * R.W + W * R.X + Y * R.Z - Z * R.Y;
			float Y1 = Y * R.W + W * R.Y + Z * R.X - X * R.Z;
			float Z1 = Z * R.W + W * R.Z + X * R.Y - Y * R.X;
#endif
			return Quaternion(X1, Y1, Z1, W1);
		}
		Vector3 Quaternion::Mul(const Vector3& R) const
		{
			Vector3 UV0(X, Y, Z), UV1, UV2;
			UV1 = UV0.Cross(R);
			UV2 = UV0.Cross(UV1);
			UV1 *= (2.0f * W);
			UV2 *= 2.0f;

			return R + UV1 + UV2;
		}
		Quaternion Quaternion::Sub(const Quaternion& R) const
		{
#ifdef VI_VECTORCLASS
			LOAD_FV4(_r1);
			LOAD_V4(_r2, R);
			_r1 -= _r2;

			Quaternion Result;
			_r1.store((float*)&Result);
			return Result;
#else
			return Quaternion(X - R.X, Y - R.Y, Z - R.Z, W - R.W);
#endif
		}
		Quaternion Quaternion::Add(const Quaternion& R) const
		{
#ifdef VI_VECTORCLASS
			LOAD_FV4(_r1);
			LOAD_V4(_r2, R);
			_r1 += _r2;

			Quaternion Result;
			_r1.store((float*)&Result);
			return Result;
#else
			return Quaternion(X + R.X, Y + R.Y, Z + R.Z, W + R.W);
#endif
		}
		Quaternion Quaternion::Lerp(const Quaternion& B, float DeltaTime) const
		{
			Quaternion Correction = B;
			if (Dot(B) < 0.0f)
				Correction = Quaternion(-B.X, -B.Y, -B.Z, -B.W);

			return (Correction - *this) * DeltaTime + Normalize();
		}
		Quaternion Quaternion::sLerp(const Quaternion& B, float DeltaTime) const
		{
			Quaternion Correction = B;
			float Cos = Dot(B);

			if (Cos < 0.0f)
			{
				Correction = Quaternion(-B.X, -B.Y, -B.Z, -B.W);
				Cos = -Cos;
			}

			if (std::abs(Cos) >= 1.0f - 1e3f)
				return Lerp(Correction, DeltaTime);

			float Sin = Geometric::FastSqrt(1.0f - Cos * Cos);
			float Angle = std::atan2(Sin, Cos);
			float InvedSin = 1.0f / Sin;
			float Source = std::sin(Angle - DeltaTime * Angle) * InvedSin;
			float Destination = std::sin(DeltaTime * Angle) * InvedSin;

			return Mul(Source).Add(Correction.Mul(Destination));
		}
		Quaternion Quaternion::CreateEulerRotation(const Vector3& V)
		{
			Quaternion Result;
			Result.SetEuler(V);
			return Result;
		}
		Quaternion Quaternion::CreateRotation(const Matrix4x4& V)
		{
			Quaternion Result;
			Result.SetMatrix(V);
			return Result;
		}
		Vector3 Quaternion::Forward() const
		{
#ifdef VI_VECTORCLASS
			LOAD_AV4(_r1, X, -W, Y, W);
			LOAD_AV4(_r2, Z, Y, Z, X);
			LOAD_AV2(_r3, X, Y);
			_r1 *= _r2;
			_r2 = permute4f<-1, -1, 2, 3>(_r1);
			_r1 = permute4f<0, 1, -1, -1>(_r1);
			_r3 = square(_r3);

			Vector3 Result(horizontal_add(_r1), horizontal_add(_r2), horizontal_add(_r3));
			Result *= 2.0f;
			Result.Z = 1.0f - Result.Z;
			return Result;
#else
			return Vector3(
				2.0f * (X * Z - W * Y),
				2.0f * (Y * Z + W * X),
				1.0f - 2.0f * (X * X + Y * Y));
#endif
		}
		Vector3 Quaternion::Up() const
		{
#ifdef VI_VECTORCLASS
			LOAD_AV4(_r1, X, W, Y, -W);
			LOAD_AV4(_r2, Y, Z, Z, X);
			LOAD_AV2(_r3, X, Z);
			_r1 *= _r2;
			_r2 = permute4f<-1, -1, 2, 3>(_r1);
			_r1 = permute4f<0, 1, -1, -1>(_r1);
			_r3 = square(_r3);

			Vector3 Result(horizontal_add(_r1), horizontal_add(_r3), horizontal_add(_r2));
			Result *= 2.0f;
			Result.Y = 1.0f - Result.Y;
			return Result;
#else
			return Vector3(
				2.0f * (X * Y + W * Z),
				1.0f - 2.0f * (X * X + Z * Z),
				2.0f * (Y * Z - W * X));
#endif
		}
		Vector3 Quaternion::Right() const
		{
#ifdef VI_VECTORCLASS
			LOAD_AV4(_r1, X, -W, X, W);
			LOAD_AV4(_r2, Y, Z, Z, Y);
			LOAD_AV2(_r3, Y, Z);
			_r1 *= _r2;
			_r2 = permute4f<-1, -1, 2, 3>(_r1);
			_r1 = permute4f<0, 1, -1, -1>(_r1);
			_r3 = square(_r3);

			Vector3 Result(horizontal_add(_r3), horizontal_add(_r1), horizontal_add(_r2));
			Result *= 2.0f;
			Result.X = 1.0f - Result.X;
			return Result;
#else
			return Vector3(
				1.0f - 2.0f * (Y * Y + Z * Z),
				2.0f * (X * Y - W * Z),
				2.0f * (X * Z + W * Y));
#endif
		}
		Matrix4x4 Quaternion::GetMatrix() const
		{
			Matrix4x4 Result =
			{
				1.0f - 2.0f * (Y * Y + Z * Z), 2.0f * (X * Y + Z * W), 2.0f * (X * Z - Y * W), 0.0f,
				2.0f * (X * Y - Z * W), 1.0f - 2.0f * (X * X + Z * Z), 2.0f * (Y * Z + X * W), 0.0f,
				2.0f * (X * Z + Y * W), 2.0f * (Y * Z - X * W), 1.0f - 2.0f * (X * X + Y * Y), 0.0f,
				0.0f, 0.0f, 0.0f, 1.0f
			};

			return Result;
		}
		Vector3 Quaternion::GetEuler() const
		{
#ifdef VI_VECTORCLASS
			LOAD_AV4(_r1, W, Y, W, -Z);
			LOAD_AV4(_r2, X, Z, Y, X);
			LOAD_FV3(_r3);
			LOAD_AV2(_r4, W, Z);
			LOAD_AV2(_r5, X, Y);
			float XYZW[4];
			_r1 *= _r2;
			_r4 *= _r5;
			_r2 = permute4f<-1, -1, 2, 3>(_r1);
			_r1 = permute4f<0, 1, -1, -1>(_r1);
			_r3 = square(_r3);
			_r3.store(XYZW);

			float T0 = +2.0f * horizontal_add(_r1);
			float T1 = +1.0f - 2.0f * (XYZW[0] + XYZW[1]);
			float Roll = Compute::Mathf::Atan2(T0, T1);

			float T2 = +2.0f * horizontal_add(_r2);
			T2 = ((T2 > 1.0f) ? 1.0f : T2);
			T2 = ((T2 < -1.0f) ? -1.0f : T2);
			float Pitch = Compute::Mathf::Asin(T2);

			float T3 = +2.0f * horizontal_add(_r4);
			float T4 = +1.0f - 2.0f * (XYZW[1] + XYZW[2]);
			float Yaw = Compute::Mathf::Atan2(T3, T4);

			return Vector3(Roll, Pitch, Yaw);
#else
			float Y2 = Y * Y;
			float T0 = +2.0f * (W * X + Y * Z);
			float T1 = +1.0f - 2.0f * (X * X + Y2);
			float Roll = Compute::Mathf::Atan2(T0, T1);

			float T2 = +2.0f * (W * Y - Z * X);
			T2 = ((T2 > 1.0f) ? 1.0f : T2);
			T2 = ((T2 < -1.0f) ? -1.0f : T2);
			float Pitch = Compute::Mathf::Asin(T2);

			float T3 = +2.0f * (W * Z + X * Y);
			float T4 = +1.0f - 2.0f * (Y2 + Z * Z);
			float Yaw = Compute::Mathf::Atan2(T3, T4);

			return Vector3(Roll, Pitch, Yaw);
#endif
		}
		float Quaternion::Dot(const Quaternion& R) const
		{
#ifdef VI_VECTORCLASS
			LOAD_FV4(_r1);
			LOAD_V4(_r2, R);
			_r1 *= _r2;
			return horizontal_add(_r1);
#else
			return X * R.X + Y * R.Y + Z * R.Z + W * R.W;
#endif
		}
		float Quaternion::Length() const
		{
#ifdef VI_VECTORCLASS
			LOAD_FV4(_r1);
			return std::sqrt(horizontal_add(square(_r1)));
#else
			return std::sqrt(X * X + Y * Y + Z * Z + W * W);
#endif
		}
		bool Quaternion::operator ==(const Quaternion& V) const
		{
			return X == V.X && Y == V.Y && Z == V.Z && W == V.W;
		}
		bool Quaternion::operator !=(const Quaternion& V) const
		{
			return !(*this == V);
		}

		RandomVector2::RandomVector2() noexcept : Min(0), Max(1), Intensity(false), Accuracy(1)
		{
		}
		RandomVector2::RandomVector2(const Vector2& MinV, const Vector2& MaxV, bool IntensityV, float AccuracyV) noexcept : Min(MinV), Max(MaxV), Intensity(IntensityV), Accuracy(AccuracyV)
		{
		}
		Vector2 RandomVector2::Generate()
		{
			Vector2 fMin = Min * Accuracy;
			Vector2 fMax = Max * Accuracy;
			float InvAccuracy = 1.0f / Accuracy;
			if (Intensity)
				InvAccuracy *= Compute::Mathf::Random();

			return Vector2(
				Compute::Mathf::Random(fMin.X, fMax.X),
				Compute::Mathf::Random(fMin.Y, fMax.Y)) * InvAccuracy;
		}

		RandomVector3::RandomVector3() noexcept : Min(0), Max(1), Intensity(false), Accuracy(1)
		{
		}
		RandomVector3::RandomVector3(const Vector3& MinV, const Vector3& MaxV, bool IntensityV, float AccuracyV) noexcept : Min(MinV), Max(MaxV), Intensity(IntensityV), Accuracy(AccuracyV)
		{
		}
		Vector3 RandomVector3::Generate()
		{
			Vector3 fMin = Min * Accuracy;
			Vector3 fMax = Max * Accuracy;
			float InvAccuracy = 1.0f / Accuracy;
			if (Intensity)
				InvAccuracy *= Compute::Mathf::Random();

			return Vector3(
				Compute::Mathf::Random(fMin.X, fMax.X),
				Compute::Mathf::Random(fMin.Y, fMax.Y),
				Compute::Mathf::Random(fMin.Z, fMax.Z)) * InvAccuracy;
		}

		RandomVector4::RandomVector4() noexcept : Min(0), Max(1), Intensity(false), Accuracy(1)
		{
		}
		RandomVector4::RandomVector4(const Vector4& MinV, const Vector4& MaxV, bool IntensityV, float AccuracyV) noexcept : Min(MinV), Max(MaxV), Intensity(IntensityV), Accuracy(AccuracyV)
		{
		}
		Vector4 RandomVector4::Generate()
		{
			Vector4 fMin = Min * Accuracy;
			Vector4 fMax = Max * Accuracy;
			float InvAccuracy = 1.0f / Accuracy;
			if (Intensity)
				InvAccuracy *= Compute::Mathf::Random();

			return Vector4(
				Compute::Mathf::Random(fMin.X, fMax.X),
				Compute::Mathf::Random(fMin.Y, fMax.Y),
				Compute::Mathf::Random(fMin.Z, fMax.Z),
				Compute::Mathf::Random(fMin.W, fMax.W)) * InvAccuracy;
		}

		RandomFloat::RandomFloat() noexcept : Min(0), Max(1), Intensity(false), Accuracy(1)
		{
		}
		RandomFloat::RandomFloat(float MinV, float MaxV, bool IntensityV, float AccuracyV) noexcept : Min(MinV), Max(MaxV), Intensity(IntensityV), Accuracy(AccuracyV)
		{
		}
		float RandomFloat::Generate()
		{
			return (Compute::Mathf::Random(Min * Accuracy, Max * Accuracy) / Accuracy) * (Intensity ? Compute::Mathf::Random() : 1);
		}

		uint8_t AdjTriangle::FindEdge(uint32_t vref0, uint32_t vref1)
		{
			uint8_t EdgeNb = 0xff;
			if (VRef[0] == vref0 && VRef[1] == vref1)
				EdgeNb = 0;
			else if (VRef[0] == vref1 && VRef[1] == vref0)
				EdgeNb = 0;
			else if (VRef[0] == vref0 && VRef[2] == vref1)
				EdgeNb = 1;
			else if (VRef[0] == vref1 && VRef[2] == vref0)
				EdgeNb = 1;
			else if (VRef[1] == vref0 && VRef[2] == vref1)
				EdgeNb = 2;
			else if (VRef[1] == vref1 && VRef[2] == vref0)
				EdgeNb = 2;

			return EdgeNb;
		}
		uint32_t AdjTriangle::OppositeVertex(uint32_t vref0, uint32_t vref1)
		{
			uint32_t Ref = 0xffffffff;
			if (VRef[0] == vref0 && VRef[1] == vref1)
				Ref = VRef[2];
			else if (VRef[0] == vref1 && VRef[1] == vref0)
				Ref = VRef[2];
			else if (VRef[0] == vref0 && VRef[2] == vref1)
				Ref = VRef[1];
			else if (VRef[0] == vref1 && VRef[2] == vref0)
				Ref = VRef[1];
			else if (VRef[1] == vref0 && VRef[2] == vref1)
				Ref = VRef[0];
			else if (VRef[1] == vref1 && VRef[2] == vref0)
				Ref = VRef[0];

			return Ref;
		}

		Adjacencies::Adjacencies() noexcept : NbEdges(0), CurrentNbFaces(0), Edges(nullptr), NbFaces(0), Faces(nullptr)
		{
		}
		Adjacencies::~Adjacencies() noexcept
		{
			Core::Memory::Deallocate(Faces);
			Core::Memory::Deallocate(Edges);
		}
		bool Adjacencies::Fill(Adjacencies::Desc& create)
		{
			NbFaces = create.NbFaces;
			Faces = Core::Memory::Allocate<AdjTriangle>(sizeof(AdjTriangle) * NbFaces);
			Edges = Core::Memory::Allocate<AdjEdge>(sizeof(AdjEdge) * NbFaces * 3);
			for (uint32_t i = 0; i < NbFaces; i++)
			{
				uint32_t Ref0 = create.Faces[i * 3 + 0];
				uint32_t Ref1 = create.Faces[i * 3 + 1];
				uint32_t Ref2 = create.Faces[i * 3 + 2];
				AddTriangle(Ref0, Ref1, Ref2);
			}

			return true;
		}
		bool Adjacencies::Resolve()
		{
			RadixSorter Core;
			uint32_t* FaceNb = Core::Memory::Allocate<uint32_t>(sizeof(uint32_t) * NbEdges);
			uint32_t* VRefs0 = Core::Memory::Allocate<uint32_t>(sizeof(uint32_t) * NbEdges);
			uint32_t* VRefs1 = Core::Memory::Allocate<uint32_t>(sizeof(uint32_t) * NbEdges);
			for (uint32_t i = 0; i < NbEdges; i++)
			{
				FaceNb[i] = Edges[i].FaceNb;
				VRefs0[i] = Edges[i].Ref0;
				VRefs1[i] = Edges[i].Ref1;
			}

			uint32_t* Sorted = Core.Sort(FaceNb, NbEdges).Sort(VRefs0, NbEdges).Sort(VRefs1, NbEdges).GetIndices();
			uint32_t LastRef0 = VRefs0[Sorted[0]];
			uint32_t LastRef1 = VRefs1[Sorted[0]];
			uint32_t Count = 0;
			uint32_t TmpBuffer[3];

			for (uint32_t i = 0; i < NbEdges; i++)
			{
				uint32_t Face = FaceNb[Sorted[i]];
				uint32_t Ref0 = VRefs0[Sorted[i]];
				uint32_t Ref1 = VRefs1[Sorted[i]];

				if (Ref0 == LastRef0 && Ref1 == LastRef1)
				{
					TmpBuffer[Count++] = Face;
					if (Count == 3)
					{
						Core::Memory::Deallocate(FaceNb);
						Core::Memory::Deallocate(VRefs0);
						Core::Memory::Deallocate(VRefs1);
						return false;
					}
				}
				else
				{
					if (Count == 2)
					{
						bool Status = UpdateLink(TmpBuffer[0], TmpBuffer[1], LastRef0, LastRef1);
						if (!Status)
						{
							Core::Memory::Deallocate(FaceNb);
							Core::Memory::Deallocate(VRefs0);
							Core::Memory::Deallocate(VRefs1);
							return Status;
						}
					}

					Count = 0;
					TmpBuffer[Count++] = Face;
					LastRef0 = Ref0;
					LastRef1 = Ref1;
				}
			}

			bool Status = true;
			if (Count == 2)
				Status = UpdateLink(TmpBuffer[0], TmpBuffer[1], LastRef0, LastRef1);

			Core::Memory::Deallocate(FaceNb);
			Core::Memory::Deallocate(VRefs0);
			Core::Memory::Deallocate(VRefs1);
			Core::Memory::Deallocate(Edges);

			return Status;
		}
		bool Adjacencies::AddTriangle(uint32_t ref0, uint32_t ref1, uint32_t ref2)
		{
			Faces[CurrentNbFaces].VRef[0] = ref0;
			Faces[CurrentNbFaces].VRef[1] = ref1;
			Faces[CurrentNbFaces].VRef[2] = ref2;
			Faces[CurrentNbFaces].ATri[0] = -1;
			Faces[CurrentNbFaces].ATri[1] = -1;
			Faces[CurrentNbFaces].ATri[2] = -1;

			if (ref0 < ref1)
				AddEdge(ref0, ref1, CurrentNbFaces);
			else
				AddEdge(ref1, ref0, CurrentNbFaces);

			if (ref0 < ref2)
				AddEdge(ref0, ref2, CurrentNbFaces);
			else
				AddEdge(ref2, ref0, CurrentNbFaces);

			if (ref1 < ref2)
				AddEdge(ref1, ref2, CurrentNbFaces);
			else
				AddEdge(ref2, ref1, CurrentNbFaces);

			CurrentNbFaces++;
			return true;
		}
		bool Adjacencies::AddEdge(uint32_t ref0, uint32_t ref1, uint32_t face)
		{
			Edges[NbEdges].Ref0 = ref0;
			Edges[NbEdges].Ref1 = ref1;
			Edges[NbEdges].FaceNb = face;
			NbEdges++;

			return true;
		}
		bool Adjacencies::UpdateLink(uint32_t firsttri, uint32_t secondtri, uint32_t ref0, uint32_t ref1)
		{
			AdjTriangle* Tri0 = &Faces[firsttri];
			AdjTriangle* Tri1 = &Faces[secondtri];
			uint8_t EdgeNb0 = Tri0->FindEdge(ref0, ref1);
			if (EdgeNb0 == 0xff)
				return false;

			uint8_t EdgeNb1 = Tri1->FindEdge(ref0, ref1);
			if (EdgeNb1 == 0xff)
				return false;

			Tri0->ATri[EdgeNb0] = secondtri | ((uint32_t)EdgeNb1 << 30);
			Tri1->ATri[EdgeNb1] = firsttri | ((uint32_t)EdgeNb0 << 30);

			return true;
		}

		TriangleStrip::TriangleStrip() noexcept : Adj(nullptr), Tags(nullptr), NbStrips(0), TotalLength(0), OneSided(false), SGICipher(false), ConnectAllStrips(false)
		{
		}
		TriangleStrip::~TriangleStrip() noexcept
		{
			FreeBuffers();
		}
		TriangleStrip& TriangleStrip::FreeBuffers()
		{
			Core::Vector<uint32_t>().swap(SingleStrip);
			Core::Vector<uint32_t>().swap(StripRuns);
			Core::Vector<uint32_t>().swap(StripLengths);
			Core::Memory::Deallocate(Tags);
			Tags = nullptr;

			Core::Memory::Delete(Adj);
			return *this;
		}
		bool TriangleStrip::Fill(const TriangleStrip::Desc& create)
		{
			Adjacencies::Desc ac;
			ac.NbFaces = create.NbFaces;
			ac.Faces = create.Faces;
			FreeBuffers();

			Adj = Core::Memory::New<Adjacencies>();
			if (!Adj->Fill(ac))
			{
				Core::Memory::Delete(Adj);
				Adj = nullptr;
				return false;
			}

			if (!Adj->Resolve())
			{
				Core::Memory::Delete(Adj);
				Adj = nullptr;
				return false;
			}

			OneSided = create.OneSided;
			SGICipher = create.SGICipher;
			ConnectAllStrips = create.ConnectAllStrips;

			return true;
		}
		bool TriangleStrip::Resolve(TriangleStrip::Result& result)
		{
			VI_ASSERT(Adj != nullptr, "triangle strip should be initialized");
			Tags = Core::Memory::Allocate<bool>(sizeof(bool) * Adj->NbFaces);
			uint32_t* Connectivity = Core::Memory::Allocate<uint32_t>(sizeof(uint32_t) * Adj->NbFaces);

			memset(Tags, 0, Adj->NbFaces * sizeof(bool));
			memset(Connectivity, 0, Adj->NbFaces * sizeof(uint32_t));

			if (SGICipher)
			{
				for (uint32_t i = 0; i < Adj->NbFaces; i++)
				{
					AdjTriangle* Tri = &Adj->Faces[i];
					if (!IS_BOUNDARY(Tri->ATri[0]))
						Connectivity[i]++;

					if (!IS_BOUNDARY(Tri->ATri[1]))
						Connectivity[i]++;

					if (!IS_BOUNDARY(Tri->ATri[2]))
						Connectivity[i]++;
				}

				RadixSorter RS;
				uint32_t* Sorted = RS.Sort(Connectivity, Adj->NbFaces).GetIndices();
				memcpy(Connectivity, Sorted, Adj->NbFaces * sizeof(uint32_t));
			}
			else
			{
				for (uint32_t i = 0; i < Adj->NbFaces; i++)
					Connectivity[i] = i;
			}

			NbStrips = 0;
			uint32_t TotalNbFaces = 0;
			uint32_t Index = 0;

			while (TotalNbFaces != Adj->NbFaces)
			{
				while (Tags[Connectivity[Index]])
					Index++;

				uint32_t FirstFace = Connectivity[Index];
				TotalNbFaces += ComputeStrip(FirstFace);
				NbStrips++;
			}

			Core::Memory::Deallocate(Connectivity);
			Core::Memory::Deallocate(Tags);
			result.Groups = StripLengths;
			result.Strips = StripRuns;

			if (ConnectAllStrips)
				ConnectStrips(result);

			return true;
		}
		uint32_t TriangleStrip::ComputeStrip(uint32_t face)
		{
			uint32_t* Strip[3];
			uint32_t* Faces[3];
			uint32_t Length[3];
			uint32_t FirstLength[3];
			uint32_t Refs0[3];
			uint32_t Refs1[3];

			Refs0[0] = Adj->Faces[face].VRef[0];
			Refs1[0] = Adj->Faces[face].VRef[1];

			Refs0[1] = Adj->Faces[face].VRef[2];
			Refs1[1] = Adj->Faces[face].VRef[0];

			Refs0[2] = Adj->Faces[face].VRef[1];
			Refs1[2] = Adj->Faces[face].VRef[2];

			for (uint32_t j = 0; j < 3; j++)
			{
				Strip[j] = Core::Memory::Allocate<uint32_t>(sizeof(uint32_t) * (Adj->NbFaces + 2 + 1 + 2));
				Faces[j] = Core::Memory::Allocate<uint32_t>(sizeof(uint32_t) * (Adj->NbFaces + 2));
				memset(Strip[j], 0xff, (Adj->NbFaces + 2 + 1 + 2) * sizeof(uint32_t));
				memset(Faces[j], 0xff, (Adj->NbFaces + 2) * sizeof(uint32_t));

				bool* vTags = Core::Memory::Allocate<bool>(sizeof(bool) * Adj->NbFaces);
				memcpy(vTags, Tags, Adj->NbFaces * sizeof(bool));

				Length[j] = TrackStrip(face, Refs0[j], Refs1[j], &Strip[j][0], &Faces[j][0], vTags);
				FirstLength[j] = Length[j];

				for (uint32_t i = 0; i < Length[j] / 2; i++)
				{
					Strip[j][i] ^= Strip[j][Length[j] - i - 1];
					Strip[j][Length[j] - i - 1] ^= Strip[j][i];
					Strip[j][i] ^= Strip[j][Length[j] - i - 1];
				}

				for (uint32_t i = 0; i < (Length[j] - 2) / 2; i++)
				{
					Faces[j][i] ^= Faces[j][Length[j] - i - 3];
					Faces[j][Length[j] - i - 3] ^= Faces[j][i];
					Faces[j][i] ^= Faces[j][Length[j] - i - 3];
				}

				uint32_t NewRef0 = Strip[j][Length[j] - 3];
				uint32_t NewRef1 = Strip[j][Length[j] - 2];
				uint32_t ExtraLength = TrackStrip(face, NewRef0, NewRef1, &Strip[j][Length[j] - 3], &Faces[j][Length[j] - 3], vTags);
				Length[j] += ExtraLength - 3;
				Core::Memory::Deallocate(vTags);
			}

			uint32_t Longest = Length[0];
			uint32_t Best = 0;
			if (Length[1] > Longest)
			{
				Longest = Length[1];
				Best = 1;
			}

			if (Length[2] > Longest)
			{
				Longest = Length[2];
				Best = 2;
			}

			uint32_t NbFaces = Longest - 2;
			for (uint32_t j = 0; j < Longest - 2; j++)
				Tags[Faces[Best][j]] = true;

			if (OneSided && FirstLength[Best] & 1)
			{
				if (Longest == 3 || Longest == 4)
				{
					Strip[Best][1] ^= Strip[Best][2];
					Strip[Best][2] ^= Strip[Best][1];
					Strip[Best][1] ^= Strip[Best][2];
				}
				else
				{
					for (uint32_t j = 0; j < Longest / 2; j++)
					{
						Strip[Best][j] ^= Strip[Best][Longest - j - 1];
						Strip[Best][Longest - j - 1] ^= Strip[Best][j];
						Strip[Best][j] ^= Strip[Best][Longest - j - 1];
					}

					uint32_t NewPos = Longest - FirstLength[Best];
					if (NewPos & 1)
					{
						for (uint32_t j = 0; j < Longest; j++)
							Strip[Best][Longest - j] = Strip[Best][Longest - j - 1];
						Longest++;
					}
				}
			}

			for (uint32_t j = 0; j < Longest; j++)
			{
				uint32_t Ref = Strip[Best][j];
				StripRuns.push_back(Ref);
			}

			StripLengths.push_back(Longest);
			for (uint32_t j = 0; j < 3; j++)
			{
				Core::Memory::Deallocate(Faces[j]);
				Core::Memory::Deallocate(Strip[j]);
			}

			return NbFaces;
		}
		uint32_t TriangleStrip::TrackStrip(uint32_t face, uint32_t oldest, uint32_t middle, uint32_t* strip, uint32_t* faces, bool* tags)
		{
			uint32_t Length = 2;
			strip[0] = oldest;
			strip[1] = middle;

			bool DoTheStrip = true;
			while (DoTheStrip)
			{
				uint32_t Newest = Adj->Faces[face].OppositeVertex(oldest, middle);
				strip[Length++] = Newest;
				*faces++ = face;
				tags[face] = true;

				uint8_t CurEdge = Adj->Faces[face].FindEdge(middle, Newest);
				if (!IS_BOUNDARY(CurEdge))
				{
					uint32_t Link = Adj->Faces[face].ATri[CurEdge];
					face = MAKE_ADJ_TRI(Link);
					if (tags[face])
						DoTheStrip = false;
				}
				else
					DoTheStrip = false;

				oldest = middle;
				middle = Newest;
			}

			return Length;
		}
		bool TriangleStrip::ConnectStrips(TriangleStrip::Result& result)
		{
			uint32_t* drefs = (uint32_t*)result.Strips.data();
			SingleStrip.clear();
			TotalLength = 0;

			for (uint32_t k = 0; k < result.Groups.size(); k++)
			{
				if (k)
				{
					uint32_t LastRef = drefs[-1];
					uint32_t FirstRef = drefs[0];
					SingleStrip.push_back(LastRef);
					SingleStrip.push_back(FirstRef);
					TotalLength += 2;

					if (OneSided && TotalLength & 1)
					{
						uint32_t SecondRef = drefs[1];
						if (FirstRef != SecondRef)
						{
							SingleStrip.push_back(FirstRef);
							TotalLength++;
						}
						else
						{
							result.Groups[k]--;
							drefs++;
						}
					}
				}

				for (uint32_t j = 0; j < result.Groups[k]; j++)
				{
					uint32_t Ref = drefs[j];
					SingleStrip.push_back(Ref);
				}

				drefs += result.Groups[k];
				TotalLength += result.Groups[k];
			}

			result.Strips = SingleStrip;
			result.Groups = Core::Vector<uint32_t>({ TotalLength });

			return true;
		}

		Core::Vector<int> TriangleStrip::Result::GetIndices(int Group)
		{
			Core::Vector<int> Indices;
			if (Group < 0)
			{
				Indices.reserve(Strips.size());
				for (auto& Index : Strips)
					Indices.push_back(Index);

				return Indices;
			}
			else if (Group < (int)Groups.size())
			{
				size_t Size = Groups[Group];
				Indices.reserve(Size);

				size_t Off = 0, Idx = 0;
				while (Off != Group)
					Idx += Groups[Off++];

				Size += Idx;
				for (size_t i = Idx; i < Size; i++)
					Indices.push_back(Strips[i]);

				return Indices;
			}

			return Indices;
		}
		Core::Vector<int> TriangleStrip::Result::GetInvIndices(int Group)
		{
			Core::Vector<int> Indices = GetIndices(Group);
			std::reverse(Indices.begin(), Indices.end());

			return Indices;
		}

		RadixSorter::RadixSorter() noexcept : CurrentSize(0), Indices(nullptr), Indices2(nullptr)
		{
			Histogram = Core::Memory::Allocate<uint32_t>(sizeof(uint32_t) * 256 * 4);
			Offset = Core::Memory::Allocate<uint32_t>(sizeof(uint32_t) * 256);
			ResetIndices();
		}
		RadixSorter::RadixSorter(const RadixSorter& Other) noexcept : CurrentSize(0), Indices(nullptr), Indices2(nullptr)
		{
			Histogram = Core::Memory::Allocate<uint32_t>(sizeof(uint32_t) * 256 * 4);
			Offset = Core::Memory::Allocate<uint32_t>(sizeof(uint32_t) * 256);
			ResetIndices();
		}
		RadixSorter::RadixSorter(RadixSorter&& Other) noexcept : Histogram(Other.Histogram), Offset(Other.Offset), CurrentSize(Other.CurrentSize), Indices(Other.Indices), Indices2(Other.Indices2)
		{
			Other.Indices = nullptr;
			Other.Indices2 = nullptr;
			Other.CurrentSize = 0;
			Other.Histogram = nullptr;
			Other.Offset = nullptr;
		}
		RadixSorter::~RadixSorter() noexcept
		{
			Core::Memory::Deallocate(Offset);
			Core::Memory::Deallocate(Histogram);
			Core::Memory::Deallocate(Indices2);
			Core::Memory::Deallocate(Indices);
		}
		RadixSorter& RadixSorter::operator =(const RadixSorter& V)
		{
			Core::Memory::Deallocate(Histogram);
			Histogram = Core::Memory::Allocate<uint32_t>(sizeof(uint32_t) * 256 * 4);

			Core::Memory::Deallocate(Offset);
			Offset = Core::Memory::Allocate<uint32_t>(sizeof(uint32_t) * 256);
			ResetIndices();

			return *this;
		}
		RadixSorter& RadixSorter::operator =(RadixSorter&& Other) noexcept
		{
			Core::Memory::Deallocate(Offset);
			Core::Memory::Deallocate(Histogram);
			Core::Memory::Deallocate(Indices2);
			Core::Memory::Deallocate(Indices);
			Indices = Other.Indices;
			Indices2 = Other.Indices2;
			CurrentSize = Other.CurrentSize;
			Histogram = Other.Histogram;
			Offset = Other.Offset;
			Other.Indices = nullptr;
			Other.Indices2 = nullptr;
			Other.CurrentSize = 0;
			Other.Histogram = nullptr;
			Other.Offset = nullptr;

			return *this;
		}
		RadixSorter& RadixSorter::Sort(uint32_t* input, uint32_t nb, bool signedvalues)
		{
			if (nb > CurrentSize)
			{
				Core::Memory::Deallocate(Indices2);
				Core::Memory::Deallocate(Indices);
				Indices = Core::Memory::Allocate<uint32_t>(sizeof(uint32_t) * nb);
				Indices2 = Core::Memory::Allocate<uint32_t>(sizeof(uint32_t) * nb);
				CurrentSize = nb;
				ResetIndices();
			}

			memset(Histogram, 0, 256 * 4 * sizeof(uint32_t));

			bool AlreadySorted = true;
			uint32_t* vIndices = Indices;
			uint8_t* p = (uint8_t*)input;
			uint8_t* pe = &p[nb * 4];
			uint32_t* h0 = &Histogram[0];
			uint32_t* h1 = &Histogram[256];
			uint32_t* h2 = &Histogram[512];
			uint32_t* h3 = &Histogram[768];

			if (!signedvalues)
			{
				uint32_t PrevVal = input[Indices[0]];
				while (p != pe)
				{
					uint32_t Val = input[*vIndices++];
					if (Val < PrevVal)
						AlreadySorted = false;

					PrevVal = Val;
					h0[*p++]++;
					h1[*p++]++;
					h2[*p++]++;
					h3[*p++]++;
				}
			}
			else
			{
				signed int PrevVal = (signed int)input[Indices[0]];
				while (p != pe)
				{
					signed int Val = (signed int)input[*vIndices++];
					if (Val < PrevVal)
						AlreadySorted = false;

					PrevVal = Val;
					h0[*p++]++;
					h1[*p++]++;
					h2[*p++]++;
					h3[*p++]++;
				}
			}

			if (AlreadySorted)
				return *this;

			uint32_t NbNegativeValues = 0;
			if (signedvalues)
			{
				uint32_t* h4 = &Histogram[768];
				for (uint32_t i = 128; i < 256; i++)
					NbNegativeValues += h4[i];
			}

			for (uint32_t j = 0; j < 4; j++)
			{
				uint32_t* CurCount = &Histogram[j << 8];
				bool PerformPass = true;

				for (uint32_t i = 0; i < 256; i++)
				{
					if (CurCount[i] == nb)
					{
						PerformPass = false;
						break;
					}

					if (CurCount[i])
						break;
				}

				if (PerformPass)
				{
					if (j != 3 || !signedvalues)
					{
						Offset[0] = 0;
						for (uint32_t i = 1; i < 256; i++)
							Offset[i] = Offset[i - 1] + CurCount[i - 1];
					}
					else
					{
						Offset[0] = NbNegativeValues;
						for (uint32_t i = 1; i < 128; i++)
							Offset[i] = Offset[i - 1] + CurCount[i - 1];

						Offset[128] = 0;
						for (uint32_t i = 129; i < 256; i++)
							Offset[i] = Offset[i - 1] + CurCount[i - 1];
					}

					uint8_t* InputBytes = (uint8_t*)input;
					uint32_t* IndicesStart = Indices;
					uint32_t* IndicesEnd = &Indices[nb];
					InputBytes += j;

					while (IndicesStart != IndicesEnd)
					{
						uint32_t id = *IndicesStart++;
						Indices2[Offset[InputBytes[id << 2]]++] = id;
					}

					uint32_t* Tmp = Indices;
					Indices = Indices2;
					Indices2 = Tmp;
				}
			}

			return *this;
		}
		RadixSorter& RadixSorter::Sort(float* input2, uint32_t nb)
		{
			uint32_t* input = (uint32_t*)input2;
			if (nb > CurrentSize)
			{
				Core::Memory::Deallocate(Indices2);
				Core::Memory::Deallocate(Indices);
				Indices = Core::Memory::Allocate<uint32_t>(sizeof(uint32_t) * nb);
				Indices2 = Core::Memory::Allocate<uint32_t>(sizeof(uint32_t) * nb);
				CurrentSize = nb;
				ResetIndices();
			}

			memset(Histogram, 0, 256 * 4 * sizeof(uint32_t));
			{
				float PrevVal = input2[Indices[0]];
				bool AlreadySorted = true;
				uint32_t* vIndices = Indices;
				uint8_t* p = (uint8_t*)input;
				uint8_t* pe = &p[nb * 4];
				uint32_t* h0 = &Histogram[0];
				uint32_t* h1 = &Histogram[256];
				uint32_t* h2 = &Histogram[512];
				uint32_t* h3 = &Histogram[768];

				while (p != pe)
				{
					float Val = input2[*vIndices++];
					if (Val < PrevVal)
						AlreadySorted = false;

					PrevVal = Val;
					h0[*p++]++;
					h1[*p++]++;
					h2[*p++]++;
					h3[*p++]++;
				}

				if (AlreadySorted)
					return *this;
			}

			uint32_t NbNegativeValues = 0;
			uint32_t* h3 = &Histogram[768];
			for (uint32_t i = 128; i < 256; i++)
				NbNegativeValues += h3[i];

			for (uint32_t j = 0; j < 4; j++)
			{
				uint32_t* CurCount = &Histogram[j << 8];
				bool PerformPass = true;

				for (uint32_t i = 0; i < 256; i++)
				{
					if (CurCount[i] == nb)
					{
						PerformPass = false;
						break;
					}

					if (CurCount[i])
						break;
				}

				if (PerformPass)
				{
					if (j != 3)
					{
						Offset[0] = 0;
						for (uint32_t i = 1; i < 256; i++)
							Offset[i] = Offset[i - 1] + CurCount[i - 1];

						uint8_t* InputBytes = (uint8_t*)input;
						uint32_t* IndicesStart = Indices;
						uint32_t* IndicesEnd = &Indices[nb];
						InputBytes += j;

						while (IndicesStart != IndicesEnd)
						{
							uint32_t id = *IndicesStart++;
							Indices2[Offset[InputBytes[id << 2]]++] = id;
						}
					}
					else
					{
						Offset[0] = NbNegativeValues;
						for (uint32_t i = 1; i < 128; i++)
							Offset[i] = Offset[i - 1] + CurCount[i - 1];

						Offset[255] = 0;
						for (uint32_t i = 0; i < 127; i++)
							Offset[254 - i] = Offset[255 - i] + CurCount[255 - i];

						for (uint32_t i = 128; i < 256; i++)
							Offset[i] += CurCount[i];

						for (uint32_t i = 0; i < nb; i++)
						{
							uint32_t Radix = input[Indices[i]] >> 24;
							if (Radix < 128)
								Indices2[Offset[Radix]++] = Indices[i];
							else
								Indices2[--Offset[Radix]] = Indices[i];
						}
					}

					uint32_t* Tmp = Indices;
					Indices = Indices2;
					Indices2 = Tmp;
				}
			}

			return *this;
		}
		RadixSorter& RadixSorter::ResetIndices()
		{
			for (uint32_t i = 0; i < CurrentSize; i++)
				Indices[i] = i;

			return *this;
		}
		uint32_t* RadixSorter::GetIndices()
		{
			return Indices;
		}

		bool Geometric::IsCubeInFrustum(const Matrix4x4& WVP, float Radius)
		{
			Radius = -Radius;
#ifdef VI_VECTORCLASS
			LOAD_AV4(_r1, WVP.Row[3], WVP.Row[7], WVP.Row[11], WVP.Row[15]);
			LOAD_AV4(_r2, WVP.Row[0], WVP.Row[4], WVP.Row[8], WVP.Row[12]);
			LOAD_VAL(_r3, _r1 + _r2);
			float F = _r3.extract(3); _r3.cutoff(3);
			F /= Geometric::FastSqrt(horizontal_add(square(_r3)));
			if (F <= Radius)
				return false;

			_r3 = _r1 - _r2;
			F = _r3.extract(3); _r3.cutoff(3);
			F /= Geometric::FastSqrt(horizontal_add(square(_r3)));
			if (F <= Radius)
				return false;

			_r2 = Vec4f(WVP.Row[1], WVP.Row[5], WVP.Row[9], WVP.Row[13]);
			_r3 = _r1 + _r2;
			F = _r3.extract(3); _r3.cutoff(3);
			F /= Geometric::FastSqrt(horizontal_add(square(_r3)));
			if (F <= Radius)
				return false;

			_r3 = _r1 - _r2;
			F = _r3.extract(3); _r3.cutoff(3);
			F /= Geometric::FastSqrt(horizontal_add(square(_r3)));
			if (F <= Radius)
				return false;

			_r2 = Vec4f(WVP.Row[2], WVP.Row[6], WVP.Row[10], WVP.Row[14]);
			_r3 = _r1 + _r2;
			F = _r3.extract(3); _r3.cutoff(3);
			F /= Geometric::FastSqrt(horizontal_add(square(_r3)));
			if (F <= Radius)
				return false;

			_r2 = Vec4f(WVP.Row[2], WVP.Row[6], WVP.Row[10], WVP.Row[14]);
			_r3 = _r1 - _r2;
			F = _r3.extract(3); _r3.cutoff(3);
			F /= Geometric::FastSqrt(horizontal_add(square(_r3)));
			if (F <= Radius)
				return false;
#else
			float Plane[4];
			Plane[0] = WVP.Row[3] + WVP.Row[0];
			Plane[1] = WVP.Row[7] + WVP.Row[4];
			Plane[2] = WVP.Row[11] + WVP.Row[8];
			Plane[3] = WVP.Row[15] + WVP.Row[12];

			Plane[3] /= Geometric::FastSqrt(Plane[0] * Plane[0] + Plane[1] * Plane[1] + Plane[2] * Plane[2]);
			if (Plane[3] <= Radius)
				return false;

			Plane[0] = WVP.Row[3] - WVP.Row[0];
			Plane[1] = WVP.Row[7] - WVP.Row[4];
			Plane[2] = WVP.Row[11] - WVP.Row[8];
			Plane[3] = WVP.Row[15] - WVP.Row[12];

			Plane[3] /= Geometric::FastSqrt(Plane[0] * Plane[0] + Plane[1] * Plane[1] + Plane[2] * Plane[2]);
			if (Plane[3] <= Radius)
				return false;

			Plane[0] = WVP.Row[3] + WVP.Row[1];
			Plane[1] = WVP.Row[7] + WVP.Row[5];
			Plane[2] = WVP.Row[11] + WVP.Row[9];
			Plane[3] = WVP.Row[15] + WVP.Row[13];

			Plane[3] /= Geometric::FastSqrt(Plane[0] * Plane[0] + Plane[1] * Plane[1] + Plane[2] * Plane[2]);
			if (Plane[3] <= Radius)
				return false;

			Plane[0] = WVP.Row[3] - WVP.Row[1];
			Plane[1] = WVP.Row[7] - WVP.Row[5];
			Plane[2] = WVP.Row[11] - WVP.Row[9];
			Plane[3] = WVP.Row[15] - WVP.Row[13];

			Plane[3] /= Geometric::FastSqrt(Plane[0] * Plane[0] + Plane[1] * Plane[1] + Plane[2] * Plane[2]);
			if (Plane[3] <= Radius)
				return false;

			Plane[0] = WVP.Row[3] + WVP.Row[2];
			Plane[1] = WVP.Row[7] + WVP.Row[6];
			Plane[2] = WVP.Row[11] + WVP.Row[10];
			Plane[3] = WVP.Row[15] + WVP.Row[14];

			Plane[3] /= Geometric::FastSqrt(Plane[0] * Plane[0] + Plane[1] * Plane[1] + Plane[2] * Plane[2]);
			if (Plane[3] <= Radius)
				return false;

			Plane[0] = WVP.Row[3] - WVP.Row[2];
			Plane[1] = WVP.Row[7] - WVP.Row[6];
			Plane[2] = WVP.Row[11] - WVP.Row[10];
			Plane[3] = WVP.Row[15] - WVP.Row[14];

			Plane[3] /= Geometric::FastSqrt(Plane[0] * Plane[0] + Plane[1] * Plane[1] + Plane[2] * Plane[2]);
			if (Plane[3] <= Radius)
				return false;
#endif
			return true;
		}
		bool Geometric::IsLeftHanded()
		{
			return LeftHanded;
		}
		bool Geometric::HasSphereIntersected(const Vector3& PositionR0, float RadiusR0, const Vector3& PositionR1, float RadiusR1)
		{
			if (PositionR0.Distance(PositionR1) < RadiusR0 + RadiusR1)
				return true;

			return false;
		}
		bool Geometric::HasLineIntersected(float Distance0, float Distance1, const Vector3& Point0, const Vector3& Point1, Vector3& Hit)
		{
			if ((Distance0 * Distance1) >= 0)
				return false;

			if (Distance0 == Distance1)
				return false;

			Hit = Point0 + (Point1 - Point0) * (-Distance0 / (Distance1 - Distance0));
			return true;
		}
		bool Geometric::HasLineIntersectedCube(const Vector3& Min, const Vector3& Max, const Vector3& Start, const Vector3& End)
		{
			if (End.X < Min.X && Start.X < Min.X)
				return false;

			if (End.X > Max.X && Start.X > Max.X)
				return false;

			if (End.Y < Min.Y && Start.Y < Min.Y)
				return false;

			if (End.Y > Max.Y && Start.Y > Max.Y)
				return false;

			if (End.Z < Min.Z && Start.Z < Min.Z)
				return false;

			if (End.Z > Max.Z && Start.Z > Max.Z)
				return false;

			if (Start.X > Min.X && Start.X < Max.X && Start.Y > Min.Y && Start.Y < Max.Y && Start.Z > Min.Z && Start.Z < Max.Z)
				return true;

			Vector3 LastHit;
			if ((HasLineIntersected(Start.X - Min.X, End.X - Min.X, Start, End, LastHit) && HasPointIntersectedCube(LastHit, Min, Max, 1)) || (HasLineIntersected(Start.Y - Min.Y, End.Y - Min.Y, Start, End, LastHit) && HasPointIntersectedCube(LastHit, Min, Max, 2)) || (HasLineIntersected(Start.Z - Min.Z, End.Z - Min.Z, Start, End, LastHit) && HasPointIntersectedCube(LastHit, Min, Max, 3)) || (HasLineIntersected(Start.X - Max.X, End.X - Max.X, Start, End, LastHit) && HasPointIntersectedCube(LastHit, Min, Max, 1)) || (HasLineIntersected(Start.Y - Max.Y, End.Y - Max.Y, Start, End, LastHit) && HasPointIntersectedCube(LastHit, Min, Max, 2)) || (HasLineIntersected(Start.Z - Max.Z, End.Z - Max.Z, Start, End, LastHit) && HasPointIntersectedCube(LastHit, Min, Max, 3)))
				return true;

			return false;
		}
		bool Geometric::HasPointIntersectedCube(const Vector3& LastHit, const Vector3& Min, const Vector3& Max, int Axis)
		{
			if (Axis == 1 && LastHit.Z > Min.Z && LastHit.Z < Max.Z && LastHit.Y > Min.Y && LastHit.Y < Max.Y)
				return true;

			if (Axis == 2 && LastHit.Z > Min.Z && LastHit.Z < Max.Z && LastHit.X > Min.X && LastHit.X < Max.X)
				return true;

			if (Axis == 3 && LastHit.X > Min.X && LastHit.X < Max.X && LastHit.Y > Min.Y && LastHit.Y < Max.Y)
				return true;

			return false;
		}
		bool Geometric::HasPointIntersectedCube(const Vector3& Position, const Vector3& Scale, const Vector3& P0)
		{
			return (P0.X) <= (Position.X + Scale.X) && (Position.X - Scale.X) <= (P0.X) && (P0.Y) <= (Position.Y + Scale.Y) && (Position.Y - Scale.Y) <= (P0.Y) && (P0.Z) <= (Position.Z + Scale.Z) && (Position.Z - Scale.Z) <= (P0.Z);
		}
		bool Geometric::HasPointIntersectedRectangle(const Vector3& Position, const Vector3& Scale, const Vector3& P0)
		{
			return P0.X >= Position.X - Scale.X && P0.X < Position.X + Scale.X && P0.Y >= Position.Y - Scale.Y && P0.Y < Position.Y + Scale.Y;
		}
		bool Geometric::HasSBIntersected(Transform* R0, Transform* R1)
		{
			if (!HasSphereIntersected(R0->GetPosition(), R0->GetScale().Hypotenuse(), R1->GetPosition(), R1->GetScale().Hypotenuse()))
				return false;

			return true;
		}
		bool Geometric::HasOBBIntersected(Transform* R0, Transform* R1)
		{
			const Vector3& Rotation0 = R0->GetRotation();
			const Vector3& Rotation1 = R1->GetRotation();
			if (Rotation0 == 0.0f && Rotation1 == 0.0f)
				return HasAABBIntersected(R0, R1);

			const Vector3& Position0 = R0->GetPosition();
			const Vector3& Position1 = R1->GetPosition();
			const Vector3& Scale0 = R0->GetScale();
			const Vector3& Scale1 = R1->GetScale();
			Matrix4x4 Temp0 = Matrix4x4::Create(Position0 - Position1, Scale0, Rotation0) * Matrix4x4::CreateRotation(Rotation1).Inv();
			if (HasLineIntersectedCube(-Scale1, Scale1, Vector4(1, 1, 1, 1).Transform(Temp0.Row).XYZ(), Vector4(-1, -1, -1, 1).Transform(Temp0.Row).XYZ()))
				return true;

			if (HasLineIntersectedCube(-Scale1, Scale1, Vector4(1, -1, 1, 1).Transform(Temp0.Row).XYZ(), Vector4(-1, 1, -1, 1).Transform(Temp0.Row).XYZ()))
				return true;

			if (HasLineIntersectedCube(-Scale1, Scale1, Vector4(-1, -1, 1, 1).Transform(Temp0.Row).XYZ(), Vector4(1, 1, -1, 1).Transform(Temp0.Row).XYZ()))
				return true;

			if (HasLineIntersectedCube(-Scale1, Scale1, Vector4(-1, 1, 1, 1).Transform(Temp0.Row).XYZ(), Vector4(1, -1, -1, 1).Transform(Temp0.Row).XYZ()))
				return true;

			if (HasLineIntersectedCube(-Scale1, Scale1, Vector4(0, 1, 0, 1).Transform(Temp0.Row).XYZ(), Vector4(0, -1, 0, 1).Transform(Temp0.Row).XYZ()))
				return true;

			if (HasLineIntersectedCube(-Scale1, Scale1, Vector4(1, 0, 0, 1).Transform(Temp0.Row).XYZ(), Vector4(-1, 0, 0, 1).Transform(Temp0.Row).XYZ()))
				return true;

			if (HasLineIntersectedCube(-Scale1, Scale1, Vector4(0, 0, 1, 1).Transform(Temp0.Row).XYZ(), Vector4(0, 0, -1, 1).Transform(Temp0.Row).XYZ()))
				return true;

			Temp0 = Matrix4x4::Create(Position1 - Position0, Scale1, Rotation1) * Matrix4x4::CreateRotation(Rotation0).Inv();
			if (HasLineIntersectedCube(-Scale0, Scale0, Vector4(1, 1, 1, 1).Transform(Temp0.Row).XYZ(), Vector4(-1, -1, -1, 1).Transform(Temp0.Row).XYZ()))
				return true;

			if (HasLineIntersectedCube(-Scale0, Scale0, Vector4(1, -1, 1, 1).Transform(Temp0.Row).XYZ(), Vector4(-1, 1, -1, 1).Transform(Temp0.Row).XYZ()))
				return true;

			if (HasLineIntersectedCube(-Scale0, Scale0, Vector4(-1, -1, 1, 1).Transform(Temp0.Row).XYZ(), Vector4(1, 1, -1, 1).Transform(Temp0.Row).XYZ()))
				return true;

			if (HasLineIntersectedCube(-Scale0, Scale0, Vector4(-1, 1, 1, 1).Transform(Temp0.Row).XYZ(), Vector4(1, -1, -1, 1).Transform(Temp0.Row).XYZ()))
				return true;

			if (HasLineIntersectedCube(-Scale0, Scale0, Vector4(0, 1, 0, 1).Transform(Temp0.Row).XYZ(), Vector4(0, -1, 0, 1).Transform(Temp0.Row).XYZ()))
				return true;

			if (HasLineIntersectedCube(-Scale0, Scale0, Vector4(1, 0, 0, 1).Transform(Temp0.Row).XYZ(), Vector4(-1, 0, 0, 1).Transform(Temp0.Row).XYZ()))
				return true;

			if (HasLineIntersectedCube(-Scale0, Scale0, Vector4(0, 0, 1, 1).Transform(Temp0.Row).XYZ(), Vector4(0, 0, -1, 1).Transform(Temp0.Row).XYZ()))
				return true;

			return false;
		}
		bool Geometric::HasAABBIntersected(Transform* R0, Transform* R1)
		{
			VI_ASSERT(R0 != nullptr && R1 != nullptr, "transforms should be set");
			const Vector3& Position0 = R0->GetPosition();
			const Vector3& Position1 = R1->GetPosition();
			const Vector3& Scale0 = R0->GetScale();
			const Vector3& Scale1 = R1->GetScale();
			return
				(Position0.X - Scale0.X) <= (Position1.X + Scale1.X) &&
				(Position1.X - Scale1.X) <= (Position0.X + Scale0.X) &&
				(Position0.Y - Scale0.Y) <= (Position1.Y + Scale1.Y) &&
				(Position1.Y - Scale1.Y) <= (Position0.Y + Scale0.Y) &&
				(Position0.Z - Scale0.Z) <= (Position1.Z + Scale1.Z) &&
				(Position1.Z - Scale1.Z) <= (Position0.Z + Scale0.Z);
		}
		void Geometric::FlipIndexWindingOrder(Core::Vector<int>& Indices)
		{
			std::reverse(Indices.begin(), Indices.end());
		}
		void Geometric::MatrixRhToLh(Trigonometry::Matrix4x4* Matrix)
		{
			VI_ASSERT(Matrix != nullptr, "matrix should be set");
			*Matrix = *Matrix * RH_TO_LH;
		}
		void Geometric::SetLeftHanded(bool IsLeftHanded)
		{
			VI_TRACE("[geometric] apply left-handed: %s", IsLeftHanded ? "on" : "off");
			LeftHanded = IsLeftHanded;
		}
        Core::Vector<int> Geometric::CreateTriangleStrip(TriangleStrip::Desc& Desc, const Core::Vector<int>& Indices)
		{
			Core::Vector<uint32_t> Src;
			Src.reserve(Indices.size());

			for (auto& Index : Indices)
				Src.push_back((uint32_t)Index);

			Desc.Faces = Src.data();
			Desc.NbFaces = (uint32_t)Src.size() / 3;

			TriangleStrip Strip;
			if (!Strip.Fill(Desc))
			{
				Core::Vector<int> Empty;
				return Empty;
			}

			TriangleStrip::Result Result;
			if (!Strip.Resolve(Result))
			{
				Core::Vector<int> Empty;
				return Empty;
			}

			return Result.GetIndices();
		}
		Core::Vector<int> Geometric::CreateTriangleList(const Core::Vector<int>& Indices)
		{
			size_t Size = Indices.size() - 2;
			Core::Vector<int> Result;
			Result.reserve(Size * 3);

			for (size_t i = 0; i < Size; i++)
			{
				if (i % 2 == 0)
				{
					Result.push_back(Indices[i + 0]);
					Result.push_back(Indices[i + 1]);
					Result.push_back(Indices[i + 2]);
				}
				else
				{
					Result.push_back(Indices[i + 2]);
					Result.push_back(Indices[i + 1]);
					Result.push_back(Indices[i + 0]);
				}
			}

			return Result;
		}
		void Geometric::CreateFrustum8CRad(Vector4* Result8, float FieldOfView, float Aspect, float NearZ, float FarZ)
		{
			VI_ASSERT(Result8 != nullptr, "8 sized array should be set");
			float HalfHFov = std::tan(FieldOfView * 0.5f) * Aspect;
			float HalfVFov = std::tan(FieldOfView * 0.5f);
			float XN = NearZ * HalfHFov;
			float XF = FarZ * HalfHFov;
			float YN = NearZ * HalfVFov;
			float YF = FarZ * HalfVFov;

			Result8[0] = Vector4(XN, YN, NearZ, 1.0);
			Result8[1] = Vector4(-XN, YN, NearZ, 1.0);
			Result8[2] = Vector4(XN, -YN, NearZ, 1.0);
			Result8[3] = Vector4(-XN, -YN, NearZ, 1.0);
			Result8[4] = Vector4(XF, YF, FarZ, 1.0);
			Result8[5] = Vector4(-XF, YF, FarZ, 1.0);
			Result8[6] = Vector4(XF, -YF, FarZ, 1.0);
			Result8[7] = Vector4(-XF, -YF, FarZ, 1.0);
		}
		void Geometric::CreateFrustum8C(Vector4* Result8, float FieldOfView, float Aspect, float NearZ, float FarZ)
		{
			return CreateFrustum8CRad(Result8, Compute::Mathf::Deg2Rad() * FieldOfView, Aspect, NearZ, FarZ);
		}
		Ray Geometric::CreateCursorRay(const Vector3& Origin, const Vector2& Cursor, const Vector2& Screen, const Matrix4x4& InvProjection, const Matrix4x4& InvView)
		{
			Vector2 Tmp = Cursor * 2.0f;
			Tmp /= Screen;

			Vector4 Eye = Vector4(Tmp.X - 1.0f, 1.0f - Tmp.Y, 1.0f, 1.0f) * InvProjection;
			Eye = (Vector4(Eye.X, Eye.Y, 1.0f, 0.0f) * InvView).sNormalize();
			return Ray(Origin, Vector3(Eye.X, Eye.Y, Eye.Z));
		}
		bool Geometric::CursorRayTest(const Ray& Cursor, const Vector3& Position, const Vector3& Scale, Vector3* Hit)
		{
			return Cursor.IntersectsAABB(Position, Scale, Hit);
		}
		bool Geometric::CursorRayTest(const Ray& Cursor, const Matrix4x4& World, Vector3* Hit)
		{
			return Cursor.IntersectsOBB(World, Hit);
		}
		float Geometric::FastInvSqrt(float Value)
		{
			float F = Value;
			long I = *(long*)&F;
			I = 0x5f3759df - (I >> 1);
			F = *(float*)&I;
			F = F * (1.5f - ((Value * 0.5f) * F * F));

			return F;
		}
		float Geometric::FastSqrt(float Value)
		{
			return 1.0f / FastInvSqrt(Value);
		}
		float Geometric::AabbVolume(const Vector3& Min, const Vector3& Max)
		{
			float Volume =
				(Max[1] - Min[1]) * (Max[2] - Min[2]) +
				(Max[0] - Min[0]) * (Max[2] - Min[2]) +
				(Max[0] - Min[0]) * (Max[1] - Min[1]);
			return Volume * 2.0f;
		}
		float Geometric::AngluarLerp(float A, float B, float DeltaTime)
		{
			if (A == B)
				return A;

			Vector2 ACircle = Vector2(cosf(A), sinf(A));
			Vector2 BCircle = Vector2(cosf(B), sinf(B));
			Vector2 floatnterpolation = ACircle.Lerp(BCircle, (float)DeltaTime);
			return std::atan2(floatnterpolation.Y, floatnterpolation.X);
		}
		float Geometric::AngleDistance(float A, float B)
		{
			return Vector2(cosf(A), sinf(A)).Distance(Vector2(cosf(B), sinf(B)));
		}
		bool Geometric::LeftHanded = true;

		Transform::Transform(void* NewUserData) noexcept : Root(nullptr), Local(nullptr), Scaling(false), Dirty(true), UserData(NewUserData)
		{
		}
		Transform::~Transform() noexcept
		{
			SetRoot(nullptr);
			RemoveChilds();
			Core::Memory::Delete(Local);
		}
		void Transform::Synchronize()
		{
			VI_ASSERT(!Root || Local != nullptr, "corrupted root transform");
			if (Root != nullptr)
			{
				Local->Offset = Matrix4x4::Create(Local->Position, Local->Rotation) * Root->GetBiasUnscaled();
				Global.Position = Local->Offset.Position();
				Global.Rotation = Local->Offset.RotationEuler();
				Global.Scale = (Scaling ? Local->Scale : Local->Scale * Root->Global.Scale);
				Temporary = Matrix4x4::CreateScale(Global.Scale) * Local->Offset;
			}
			else
			{
				Global.Offset = Matrix4x4::Create(Global.Position, Global.Scale, Global.Rotation);
				Temporary = Matrix4x4::CreateRotation(Global.Rotation) * Matrix4x4::CreateTranslation(Global.Position);
			}
			Dirty = false;
		}
		void Transform::Move(const Vector3& Value)
		{
			VI_ASSERT(!Root || Local != nullptr, "corrupted root transform");
			if (Root != nullptr)
				Local->Position += Value;
			else
				Global.Position += Value;
			MakeDirty();
		}
		void Transform::Rotate(const Vector3& Value)
		{
			VI_ASSERT(!Root || Local != nullptr, "corrupted root transform");
			if (Root != nullptr)
				Local->Rotation += Value;
			else
				Global.Rotation += Value;
			MakeDirty();
		}
		void Transform::Rescale(const Vector3& Value)
		{
			VI_ASSERT(!Root || Local != nullptr, "corrupted root transform");
			if (Root != nullptr)
				Local->Scale += Value;
			else
				Global.Scale += Value;
			MakeDirty();
		}
		void Transform::Localize(Spacing& Where)
		{
			if (Root != nullptr)
			{
				Where.Offset = Matrix4x4::Create(Where.Position, Where.Rotation) * Root->GetBiasUnscaled().Inv();
				Where.Position = Where.Offset.Position();
				Where.Rotation = Where.Offset.RotationEuler();
				Where.Scale = (Scaling ? Where.Scale : Where.Scale / Root->Global.Scale);
			}
		}
		void Transform::Globalize(Spacing& Where)
		{
			if (Root != nullptr)
			{
				Where.Offset = Matrix4x4::Create(Where.Position, Where.Rotation) * Root->GetBiasUnscaled();
				Where.Position = Where.Offset.Position();
				Where.Rotation = Where.Offset.RotationEuler();
				Where.Scale = (Scaling ? Where.Scale : Where.Scale * Root->Global.Scale);
			}
		}
		void Transform::Specialize(Spacing& Where)
		{
			VI_ASSERT(!Root || Local != nullptr, "corrupted root transform");
			if (Root != nullptr)
			{
				Where.Offset = Matrix4x4::Create(Local->Position, Local->Rotation) * Root->GetBiasUnscaled();
				Where.Position = Where.Offset.Position();
				Where.Rotation = Where.Offset.RotationEuler();
				Where.Scale = (Scaling ? Local->Scale : Local->Scale * Root->Global.Scale);
			}
			else if (&Where != &Global)
				Where = Global;
		}
		void Transform::Copy(Transform* Target)
		{
			VI_ASSERT(Target != nullptr && Target != this, "target should be set");
			Core::Memory::Delete(Local);
			if (Target->Root != nullptr)
				Local = Core::Memory::New<Spacing>(*Target->Local);
			else
				Local = nullptr;

			UserData = Target->UserData;
			Childs = Target->Childs;
			Global = Target->Global;
			Scaling = Target->Scaling;
			Root = Target->Root;
			Dirty = true;
		}
		void Transform::AddChild(Transform* Child)
		{
			VI_ASSERT(Child != nullptr && Child != this, "child should be set");
			Childs.push_back(Child);
			Child->MakeDirty();
		}
		void Transform::RemoveChild(Transform* Child)
		{
			VI_ASSERT(Child != nullptr && Child != this, "child should be set");
			if (Child->Root == this)
				Child->SetRoot(nullptr);
		}
		void Transform::RemoveChilds()
		{
			Core::Vector<Transform*> Array;
			Array.swap(Childs);

			for (auto& Child : Array)
			{
				if (Child->Root == this)
					Child->SetRoot(nullptr);
			}
		}
		void Transform::WhenDirty(Core::TaskCallback&& Callback)
		{
			OnDirty = std::move(Callback);
			if (Dirty && OnDirty)
				OnDirty();
		}
		void Transform::MakeDirty()
		{
			if (Dirty)
				return;

			Dirty = true;
			if (OnDirty)
				OnDirty();

			for (auto& Child : Childs)
				Child->MakeDirty();
		}
		void Transform::SetScaling(bool Enabled)
		{
			Scaling = Enabled;
			MakeDirty();
		}
		void Transform::SetPosition(const Vector3& Value)
		{
			VI_ASSERT(!Root || Local != nullptr, "corrupted root transform");
			bool Updated = !(Root ? Local->Position.IsEquals(Value) : Global.Position.IsEquals(Value));
			if (Root != nullptr)
				Local->Position = Value;
			else
				Global.Position = Value;

			if (Updated)
				MakeDirty();
		}
		void Transform::SetRotation(const Vector3& Value)
		{
			VI_ASSERT(!Root || Local != nullptr, "corrupted root transform");
			bool Updated = !(Root ? Local->Rotation.IsEquals(Value) : Global.Rotation.IsEquals(Value));
			if (Root != nullptr)
				Local->Rotation = Value;
			else
				Global.Rotation = Value;

			if (Updated)
				MakeDirty();
		}
		void Transform::SetScale(const Vector3& Value)
		{
			VI_ASSERT(!Root || Local != nullptr, "corrupted root transform");
			bool Updated = !(Root ? Local->Scale.IsEquals(Value) : Global.Scale.IsEquals(Value));
			if (Root != nullptr)
				Local->Scale = Value;
			else
				Global.Scale = Value;

			if (Updated)
				MakeDirty();
		}
		void Transform::SetSpacing(Positioning Space, Spacing& Where)
		{
			if (Space == Positioning::Global)
				Localize(Where);

			GetSpacing() = Where;
			MakeDirty();
		}
		void Transform::SetPivot(Transform* Parent, Spacing* Pivot)
		{
			Core::Memory::Delete(Local);
			Root = Parent;
			Local = Pivot;

			if (Parent != nullptr)
				Parent->AddChild(this);
		}
		void Transform::SetRoot(Transform* Parent)
		{
			if (!CanRootBeApplied(Parent))
				return;

			if (Root != nullptr)
			{
				Specialize(Global);
				if (Parent != nullptr)
				{
					Core::Memory::Delete(Local);
					Local = nullptr;
				}

				for (auto It = Root->Childs.begin(); It != Root->Childs.end(); ++It)
				{
					if ((*It) == this)
					{
						Root->Childs.erase(It);
						break;
					}
				}
			}

			Root = Parent;
			if (Root != nullptr)
			{
				Root->AddChild(this);
				if (!Local)
					Local = Core::Memory::New<Spacing>();

				*Local = Global;
				Localize(*Local);
			}
		}
		void Transform::GetBounds(Matrix4x4& World, Vector3& Min, Vector3& Max)
		{
			Transform::Spacing& Space = GetSpacing();
			Vector3 Scale = (Max - Min).Abs();
			Vector3 Radius = Scale * Space.Scale * 0.5f;
			Vector3 Top = Radius.Rotate(0, Space.Rotation);
			Vector3 Left = Radius.InvX().Rotate(0, Space.Rotation);
			Vector3 Right = Radius.InvY().Rotate(0, Space.Rotation);
			Vector3 Bottom = Radius.InvZ().Rotate(0, Space.Rotation);
			Vector3 Points[8] =
			{
				-Top, Top, -Left, Left,
				-Right, Right, -Bottom, Bottom
			};
			Vector3 Lower = Points[0];
			Vector3 Upper = Points[1];

			for (size_t i = 0; i < 8; ++i)
			{
				auto& Point = Points[i];
				if (Point.X > Upper.X)
					Upper.X = Point.X;
				if (Point.Y > Upper.Y)
					Upper.Y = Point.Y;
				if (Point.Z > Upper.Z)
					Upper.Z = Point.Z;

				if (Point.X < Lower.X)
					Lower.X = Point.X;
				if (Point.Y < Lower.Y)
					Lower.Y = Point.Y;
				if (Point.Z < Lower.Z)
					Lower.Z = Point.Z;
			}

			Min = Space.Position + Lower;
			Max = Space.Position + Upper;
			World = GetBias();
		}
		bool Transform::HasRoot(const Transform* Target) const
		{
			VI_ASSERT(Target != nullptr, "target should be set");
			Trigonometry::Transform* UpperRoot = Root;
			while (UpperRoot != nullptr)
			{
				if (UpperRoot == Target)
					return true;

				UpperRoot = UpperRoot->GetRoot();
			}

			return false;
		}
		bool Transform::HasChild(Transform* Target) const
		{
			VI_ASSERT(Target != nullptr, "target should be set");
			for (auto& Child : Childs)
			{
				if (Child == Target)
					return true;

				if (Child->HasChild(Target))
					return true;
			}

			return false;
		}
		bool Transform::HasScaling() const
		{
			return Scaling;
		}
		bool Transform::CanRootBeApplied(Transform* Parent) const
		{
			if ((!Root && !Parent) || Root == Parent)
				return false;

			if (Parent == this)
				return false;

			if (Parent && Parent->HasRoot(this))
				return false;

			return true;
		}
		bool Transform::IsDirty() const
		{
			return Dirty;
		}
		const Matrix4x4& Transform::GetBias() const
		{
			VI_ASSERT(!Root || Local != nullptr, "corrupted root transform");
			if (Root != nullptr)
				return Temporary;

			return Global.Offset;
		}
		const Matrix4x4& Transform::GetBiasUnscaled() const
		{
			VI_ASSERT(!Root || Local != nullptr, "corrupted root transform");
			if (Root != nullptr)
				return Local->Offset;

			return Temporary;
		}
		const Vector3& Transform::GetPosition() const
		{
			return Global.Position;
		}
		const Vector3& Transform::GetRotation() const
		{
			return Global.Rotation;
		}
		const Vector3& Transform::GetScale() const
		{
			return Global.Scale;
		}
		Vector3 Transform::Forward() const
		{
			VI_ASSERT(!Root || Local != nullptr, "corrupted root transform");
			if (Root != nullptr)
				return Local->Offset.Forward();

			return Global.Offset.Forward();
		}
		Vector3 Transform::Right() const
		{
			VI_ASSERT(!Root || Local != nullptr, "corrupted root transform");
			if (Root != nullptr)
				return Local->Offset.Right();

			return Global.Offset.Right();
		}
		Vector3 Transform::Up() const
		{
			VI_ASSERT(!Root || Local != nullptr, "corrupted root transform");
			if (Root != nullptr)
				return Local->Offset.Up();

			return Global.Offset.Up();
		}
		Transform::Spacing& Transform::GetSpacing()
		{
			VI_ASSERT(!Root || Local != nullptr, "corrupted root transform");
			if (Root != nullptr)
				return *Local;

			return Global;
		}
		Transform::Spacing& Transform::GetSpacing(Positioning Space)
		{
			VI_ASSERT(!Root || Local != nullptr, "corrupted root transform");
			if (Space == Positioning::Local)
				return *Local;

			return Global;
		}
		Transform* Transform::GetRoot() const
		{
			return Root;
		}
		Transform* Transform::GetUpperRoot() const
		{
			Trigonometry::Transform* UpperRoot = Root;
			while (UpperRoot != nullptr)
				UpperRoot = UpperRoot->GetRoot();

			return nullptr;
		}
		Transform* Transform::GetChild(size_t Child) const
		{
			VI_ASSERT(Child < Childs.size(), "index outside of range");
			return Childs[Child];
		}
		size_t Transform::GetChildsCount() const
		{
			return Childs.size();
		}
		Core::Vector<Transform*>& Transform::GetChilds()
		{
			return Childs;
		}

		bool Cosmos::Node::IsLeaf() const
		{
			return (Left == NULL_NODE);
		}

		Cosmos::Cosmos(size_t DefaultSize) noexcept
		{
			Root = NULL_NODE;
			NodeCount = 0;
			NodeCapacity = DefaultSize;
			Nodes.resize(NodeCapacity);

			for (size_t i = 0; i < NodeCapacity - 1; i++)
			{
				auto& Node = Nodes[i];
				Node.Next = i + 1;
				Node.Height = -1;
			}

			auto& Node = Nodes[NodeCapacity - 1];
			Node.Next = NULL_NODE;
			Node.Height = -1;
			FreeList = 0;
		}
		void Cosmos::FreeNode(size_t NodeIndex)
		{
			VI_ASSERT(NodeIndex < NodeCapacity, "outside of borders");
			VI_ASSERT(NodeCount > 0, "there must be at least one node");

			auto& Node = Nodes[NodeIndex];
			Node.Next = FreeList;
			Node.Height = -1;
			FreeList = NodeIndex;
			NodeCount--;
		}
		void Cosmos::InsertItem(void* Item, const Vector3& Lower, const Vector3& Upper)
		{
			size_t NodeIndex = AllocateNode();
			auto& Node = Nodes[NodeIndex];
			Node.Bounds = Bounding(Lower, Upper);
			Node.Height = 0;
			Node.Item = Item;
			InsertLeaf(NodeIndex);

			Items.insert(Core::UnorderedMap<void*, size_t>::value_type(Item, NodeIndex));
		}
		void Cosmos::RemoveItem(void* Item)
		{
			auto It = Items.find(Item);
			if (It == Items.end())
				return;

			size_t NodeIndex = It->second;
			Items.erase(It);

			VI_ASSERT(NodeIndex < NodeCapacity, "outside of borders");
			VI_ASSERT(Nodes[NodeIndex].IsLeaf(), "cannot remove root node");

			RemoveLeaf(NodeIndex);
			FreeNode(NodeIndex);
		}
		void Cosmos::InsertLeaf(size_t LeafIndex)
		{
			if (Root == NULL_NODE)
			{
				Root = LeafIndex;
				Nodes[Root].Parent = NULL_NODE;
				return;
			}

			size_t NextIndex = Root;
			Bounding LeafBounds = Nodes[LeafIndex].Bounds;
			Bounding NextBounds;

			while (!Nodes[NextIndex].IsLeaf())
			{
				auto& Next = Nodes[NextIndex];
				auto& Left = Nodes[Next.Left];
				auto& Right = Nodes[Next.Right];

				NextBounds.Merge(LeafBounds, Next.Bounds);
				float BaseCost = 2.0f * (float)NextBounds.Volume;
				float ParentCost = 2.0f * (float)(NextBounds.Volume - Next.Bounds.Volume);

				NextBounds.Merge(LeafBounds, Left.Bounds);
				float LeftCost = (NextBounds.Volume - (Left.IsLeaf() ? 0.0f : Left.Bounds.Volume)) + ParentCost;

				NextBounds.Merge(LeafBounds, Right.Bounds);
				float RightCost = (NextBounds.Volume - (Right.IsLeaf() ? 0.0f : Right.Bounds.Volume)) + ParentCost;

				if ((BaseCost < LeftCost) && (BaseCost < RightCost))
					break;

				if (LeftCost < RightCost)
					NextIndex = Next.Left;
				else
					NextIndex = Next.Right;
			}

			size_t NewParentIndex = AllocateNode();
			auto& Leaf = Nodes[LeafIndex];
			auto& Sibling = Nodes[NextIndex];
			auto& NewParent = Nodes[NewParentIndex];

			size_t OldParentIndex = Sibling.Parent;
			NewParent.Parent = OldParentIndex;
			NewParent.Bounds.Merge(LeafBounds, Sibling.Bounds);
			NewParent.Height = Sibling.Height + 1;

			if (OldParentIndex != NULL_NODE)
			{
				auto& OldParent = Nodes[OldParentIndex];
				if (OldParent.Left == NextIndex)
					OldParent.Left = NewParentIndex;
				else
					OldParent.Right = NewParentIndex;

				NewParent.Left = NextIndex;
				NewParent.Right = LeafIndex;
				Sibling.Parent = NewParentIndex;
				Leaf.Parent = NewParentIndex;
			}
			else
			{
				NewParent.Left = NextIndex;
				NewParent.Right = LeafIndex;
				Sibling.Parent = NewParentIndex;
				Leaf.Parent = NewParentIndex;
				Root = NewParentIndex;
			}

			NextIndex = Leaf.Parent;
			while (NextIndex != NULL_NODE)
			{
				NextIndex = Balance(NextIndex);
				auto& Next = Nodes[NextIndex];
				auto& Left = Nodes[Next.Left];
				auto& Right = Nodes[Next.Right];

				Next.Height = 1 + std::max(Left.Height, Right.Height);
				Next.Bounds.Merge(Left.Bounds, Right.Bounds);
				NextIndex = Next.Parent;
			}
		}
		void Cosmos::RemoveLeaf(size_t LeafIndex)
		{
			if (LeafIndex == Root)
			{
				Root = NULL_NODE;
				return;
			}

			size_t ParentIndex = Nodes[LeafIndex].Parent;
			auto& Parent = Nodes[ParentIndex];

			size_t SiblingIndex = (Parent.Left == LeafIndex ? Parent.Right : Parent.Left);
			auto& Sibling = Nodes[SiblingIndex];

			if (Parent.Parent != NULL_NODE)
			{
				auto& UpperParent = Nodes[Parent.Parent];
				if (UpperParent.Left == ParentIndex)
					UpperParent.Left = SiblingIndex;
				else
					UpperParent.Right = SiblingIndex;

				Sibling.Parent = Parent.Parent;
				FreeNode(ParentIndex);

				size_t NextIndex = Parent.Parent;
				while (NextIndex != NULL_NODE)
				{
					NextIndex = Balance(NextIndex);
					auto& Next = Nodes[NextIndex];
					auto& Left = Nodes[Next.Left];
					auto& Right = Nodes[Next.Right];

					Next.Bounds.Merge(Left.Bounds, Right.Bounds);
					Next.Height = 1 + std::max(Left.Height, Right.Height);
					NextIndex = Next.Parent;
				}
			}
			else
			{
				Root = SiblingIndex;
				Sibling.Parent = NULL_NODE;
				FreeNode(ParentIndex);
			}
		}
		void Cosmos::Reserve(size_t Size)
		{
			if (Size < Nodes.capacity())
				return;

			Items.reserve(Size);
			Nodes.reserve(Size);
		}
		void Cosmos::Clear()
		{
			auto It = Items.begin();
			while (It != Items.end())
			{
				size_t NodeIndex = It->second;
				VI_ASSERT(NodeIndex < NodeCapacity, "outside of borders");
				VI_ASSERT(Nodes[NodeIndex].IsLeaf(), "cannot remove root node");

				RemoveLeaf(NodeIndex);
				FreeNode(NodeIndex);
				It++;
			}
			Items.clear();
		}
		bool Cosmos::UpdateItem(void* Item, const Vector3& Lower, const Vector3& Upper, bool Always)
		{
			auto It = Items.find(Item);
			if (It == Items.end())
				return false;

			auto& Next = Nodes[It->second];
			if (!Always)
			{
				Bounding Bounds(Lower, Upper);
				if (Next.Bounds.Contains(Bounds))
					return true;

				RemoveLeaf(It->second);
				Next.Bounds = Bounds;
			}
			else
			{
				RemoveLeaf(It->second);
				Next.Bounds = Bounding(Lower, Upper);
			}

			InsertLeaf(It->second);
			return true;
		}
		const Bounding& Cosmos::GetArea(void* Item)
		{
			auto It = Items.find(Item);
			if (It != Items.end())
				return Nodes[It->second].Bounds;

			return Nodes[Root].Bounds;
		}
		size_t Cosmos::AllocateNode()
		{
			if (FreeList == NULL_NODE)
			{
				VI_ASSERT(NodeCount == NodeCapacity, "invalid capacity");

				NodeCapacity *= 2;
				Nodes.resize(NodeCapacity);

				for (size_t i = NodeCount; i < NodeCapacity - 1; i++)
				{
					auto& Next = Nodes[i];
					Next.Next = i + 1;
					Next.Height = -1;
				}

				Nodes[NodeCapacity - 1].Next = NULL_NODE;
				Nodes[NodeCapacity - 1].Height = -1;
				FreeList = NodeCount;
			}

			size_t NodeIndex = FreeList;
			auto& Node = Nodes[NodeIndex];
			FreeList = Node.Next;
			Node.Parent = NULL_NODE;
			Node.Left = NULL_NODE;
			Node.Right = NULL_NODE;
			Node.Height = 0;
			NodeCount++;

			return NodeIndex;
		}
		size_t Cosmos::Balance(size_t NodeIndex)
		{
			VI_ASSERT(NodeIndex != NULL_NODE, "node should not be null");

			auto& Next = Nodes[NodeIndex];
			if (Next.IsLeaf() || (Next.Height < 2))
				return NodeIndex;

			VI_ASSERT(Next.Left < NodeCapacity, "left outside of borders");
			VI_ASSERT(Next.Right < NodeCapacity, "right outside of borders");

			size_t LeftIndex = Next.Left;
			size_t RightIndex = Next.Right;
			auto& Left = Nodes[LeftIndex];
			auto& Right = Nodes[RightIndex];

			int Balance = Right.Height - Left.Height;
			if (Balance > 1)
			{
				VI_ASSERT(Right.Left < NodeCapacity, "subleft outside of borders");
				VI_ASSERT(Right.Right < NodeCapacity, "subright outside of borders");

				size_t RightLeftIndex = Right.Left;
				size_t RightRightIndex = Right.Right;
				auto& RightLeft = Nodes[RightLeftIndex];
				auto& RightRight = Nodes[RightRightIndex];

				Right.Left = NodeIndex;
				Right.Parent = Next.Parent;
				Next.Parent = RightIndex;

				if (Right.Parent != NULL_NODE)
				{
					if (Nodes[Right.Parent].Left != NodeIndex)
					{
						VI_ASSERT(Nodes[Right.Parent].Right == NodeIndex, "invalid right spacing");
						Nodes[Right.Parent].Right = RightIndex;
					}
					else
						Nodes[Right.Parent].Left = RightIndex;
				}
				else
					Root = RightIndex;

				if (RightLeft.Height > RightRight.Height)
				{
					Right.Right = RightLeftIndex;
					Next.Right = RightRightIndex;
					RightRight.Parent = NodeIndex;
					Next.Bounds.Merge(Left.Bounds, RightRight.Bounds);
					Right.Bounds.Merge(Next.Bounds, RightLeft.Bounds);

					Next.Height = 1 + std::max(Left.Height, RightRight.Height);
					Right.Height = 1 + std::max(Next.Height, RightLeft.Height);
				}
				else
				{
					Right.Right = RightRightIndex;
					Next.Right = RightLeftIndex;
					RightLeft.Parent = NodeIndex;
					Next.Bounds.Merge(Left.Bounds, RightLeft.Bounds);
					Right.Bounds.Merge(Next.Bounds, RightRight.Bounds);

					Next.Height = 1 + std::max(Left.Height, RightLeft.Height);
					Right.Height = 1 + std::max(Next.Height, RightRight.Height);
				}

				return RightIndex;
			}
			else if (Balance < -1)
			{
				VI_ASSERT(Left.Left < NodeCapacity, "subleft outside of borders");
				VI_ASSERT(Left.Right < NodeCapacity, "subright outside of borders");

				size_t LeftLeftIndex = Left.Left;
				size_t LeftRightIndex = Left.Right;
				auto& LeftLeft = Nodes[LeftLeftIndex];
				auto& LeftRight = Nodes[LeftRightIndex];

				Left.Left = NodeIndex;
				Left.Parent = Next.Parent;
				Next.Parent = LeftIndex;

				if (Left.Parent != NULL_NODE)
				{
					if (Nodes[Left.Parent].Left != NodeIndex)
					{
						VI_ASSERT(Nodes[Left.Parent].Right == NodeIndex, "invalid left spacing");
						Nodes[Left.Parent].Right = LeftIndex;
					}
					else
						Nodes[Left.Parent].Left = LeftIndex;
				}
				else
					Root = LeftIndex;

				if (LeftLeft.Height > LeftRight.Height)
				{
					Left.Right = LeftLeftIndex;
					Next.Left = LeftRightIndex;
					LeftRight.Parent = NodeIndex;
					Next.Bounds.Merge(Right.Bounds, LeftRight.Bounds);
					Left.Bounds.Merge(Next.Bounds, LeftLeft.Bounds);
					Next.Height = 1 + std::max(Right.Height, LeftRight.Height);
					Left.Height = 1 + std::max(Next.Height, LeftLeft.Height);
				}
				else
				{
					Left.Right = LeftRightIndex;
					Next.Left = LeftLeftIndex;
					LeftLeft.Parent = NodeIndex;
					Next.Bounds.Merge(Right.Bounds, LeftLeft.Bounds);
					Left.Bounds.Merge(Next.Bounds, LeftRight.Bounds);
					Next.Height = 1 + std::max(Right.Height, LeftLeft.Height);
					Left.Height = 1 + std::max(Next.Height, LeftRight.Height);
				}

				return LeftIndex;
			}

			return NodeIndex;
		}
		size_t Cosmos::ComputeHeight() const
		{
			return ComputeHeight(Root);
		}
		size_t Cosmos::ComputeHeight(size_t NodeIndex) const
		{
			VI_ASSERT(NodeIndex < NodeCapacity, "outside of borders");

			auto& Next = Nodes[NodeIndex];
			if (Next.IsLeaf())
				return 0;

			size_t Height1 = ComputeHeight(Next.Left);
			size_t Height2 = ComputeHeight(Next.Right);
			return 1 + std::max(Height1, Height2);
		}
		size_t Cosmos::GetNodesCount() const
		{
			return Nodes.size();
		}
		size_t Cosmos::GetHeight() const
		{
			return Root == NULL_NODE ? 0 : Nodes[Root].Height;
		}
		size_t Cosmos::GetMaxBalance() const
		{
			size_t MaxBalance = 0;
			for (size_t i = 0; i < NodeCapacity; i++)
			{
				auto& Next = Nodes[i];
				if (Next.Height <= 1)
					continue;

				VI_ASSERT(!Next.IsLeaf(), "node should be leaf");
				size_t Balance = std::abs(Nodes[Next.Left].Height - Nodes[Next.Right].Height);
				MaxBalance = std::max(MaxBalance, Balance);
			}

			return MaxBalance;
		}
		size_t Cosmos::GetRoot() const
		{
			return Root;
		}
		const Core::UnorderedMap<void*, size_t>& Cosmos::GetItems() const
		{
			return Items;
		}
		const Core::Vector<Cosmos::Node>& Cosmos::GetNodes() const
		{
			return Nodes;
		}
		const Cosmos::Node& Cosmos::GetRootNode() const
		{
			VI_ASSERT(Root < Nodes.size(), "index out of range");
			return Nodes[Root];
		}
		const Cosmos::Node& Cosmos::GetNode(size_t Id) const
		{
			VI_ASSERT(Id < Nodes.size(), "index out of range");
			return Nodes[Id];
		}
		float Cosmos::GetVolumeRatio() const
		{
			if (Root == NULL_NODE)
				return 0.0;

			float RootArea = Nodes[Root].Bounds.Volume;
			float TotalArea = 0.0;

			for (size_t i = 0; i < NodeCapacity; i++)
			{
				auto& Next = Nodes[i];
				if (Next.Height >= 0)
					TotalArea += Next.Bounds.Volume;
			}

			return TotalArea / RootArea;
		}
		bool Cosmos::IsNull(size_t Id) const
		{
			return Id == NULL_NODE;
		}
		bool Cosmos::Empty() const
		{
			return Items.empty();
		}
	}
}
