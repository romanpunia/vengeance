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

namespace Vitex
{
	namespace Layer
	{
		Ticker::Ticker() noexcept : Time(0.0f), Delay(16.0f)
		{
		}
		bool Ticker::TickEvent(float ElapsedTime)
		{
			if (ElapsedTime - Time > Delay)
			{
				Time = ElapsedTime;
				return true;
			}

			return false;
		}
		float Ticker::GetTime()
		{
			return Time;
		}

		Event::Event(const std::string_view& NewName) noexcept : Name(NewName)
		{
		}
		Event::Event(const std::string_view& NewName, const Core::VariantArgs& NewArgs) noexcept : Name(NewName), Args(NewArgs)
		{
		}
		Event::Event(const std::string_view& NewName, Core::VariantArgs&& NewArgs) noexcept : Name(NewName), Args(std::move(NewArgs))
		{
		}
		Event::Event(const Event& Other) noexcept : Name(Other.Name), Args(Other.Args)
		{
		}
		Event::Event(Event&& Other) noexcept : Name(std::move(Other.Name)), Args(std::move(Other.Args))
		{
		}
		Event& Event::operator= (const Event& Other) noexcept
		{
			Name = Other.Name;
			Args = Other.Args;
			return *this;
		}
		Event& Event::operator= (Event&& Other) noexcept
		{
			Name = std::move(Other.Name);
			Args = std::move(Other.Args);
			return *this;
		}

		float AnimatorState::GetTimeline(Core::Timer* Timing)const
		{
			return Compute::Mathf::Min(Time + Rate * Timing->GetStep(), GetSecondsDuration());
		}
		float AnimatorState::GetSecondsDuration() const
		{
			return Duration / Rate;
		}
		float AnimatorState::GetProgressTotal() const
		{
			return Time / GetSecondsDuration();
		}
		float AnimatorState::GetProgress() const
		{
			return Compute::Mathf::Min(Time / GetSecondsDuration(), 1.0f);
		}
		bool AnimatorState::IsPlaying() const
		{
			return !Paused && Frame >= 0 && Clip >= 0;
		}

		void Viewer::Set(const Trigonometry::Matrix4x4& _View, const Trigonometry::Matrix4x4& _Projection, const Trigonometry::Vector3& _Position, float _Fov, float _Ratio, float _Near, float _Far, RenderCulling _Type)
		{
			Set(_View, _Projection, _Position, -_View.RotationEuler(), _Fov, _Ratio, _Near, _Far, _Type);
		}
		void Viewer::Set(const Trigonometry::Matrix4x4& _View, const Trigonometry::Matrix4x4& _Projection, const Trigonometry::Vector3& _Position, const Trigonometry::Vector3& _Rotation, float _Fov, float _Ratio, float _Near, float _Far, RenderCulling _Type)
		{
			View = _View;
			Projection = _Projection;
			ViewProjection = _View * _Projection;
			InvViewProjection = ViewProjection.Inv();
			InvPosition = _Position.Inv();
			Position = _Position;
			Rotation = _Rotation;
			FarPlane = (_Far < _Near ? 999999999.0f : _Far);
			NearPlane = _Near;
			Ratio = _Ratio;
			Fov = _Fov;
			Culling = _Type;
			CubicViewProjection[0] = Trigonometry::Matrix4x4::CreateLookAt(Trigonometry::CubeFace::PositiveX, Position) * Projection;
			CubicViewProjection[1] = Trigonometry::Matrix4x4::CreateLookAt(Trigonometry::CubeFace::NegativeX, Position) * Projection;
			CubicViewProjection[2] = Trigonometry::Matrix4x4::CreateLookAt(Trigonometry::CubeFace::PositiveY, Position) * Projection;
			CubicViewProjection[3] = Trigonometry::Matrix4x4::CreateLookAt(Trigonometry::CubeFace::NegativeY, Position) * Projection;
			CubicViewProjection[4] = Trigonometry::Matrix4x4::CreateLookAt(Trigonometry::CubeFace::PositiveZ, Position) * Projection;
			CubicViewProjection[5] = Trigonometry::Matrix4x4::CreateLookAt(Trigonometry::CubeFace::NegativeZ, Position) * Projection;
		}

		void HeavySeries::Pack(Core::Schema* V, const Trigonometry::Vector2& Value)
		{
			VI_ASSERT(V != nullptr, "schema should be set");
			V->SetAttribute("x", Core::Var::Number(Value.X));
			V->SetAttribute("y", Core::Var::Number(Value.Y));
		}
		void HeavySeries::Pack(Core::Schema* V, const Trigonometry::Vector3& Value)
		{
			VI_ASSERT(V != nullptr, "schema should be set");
			V->SetAttribute("x", Core::Var::Number(Value.X));
			V->SetAttribute("y", Core::Var::Number(Value.Y));
			V->SetAttribute("z", Core::Var::Number(Value.Z));
		}
		void HeavySeries::Pack(Core::Schema* V, const Trigonometry::Vector4& Value)
		{
			VI_ASSERT(V != nullptr, "schema should be set");
			V->SetAttribute("x", Core::Var::Number(Value.X));
			V->SetAttribute("y", Core::Var::Number(Value.Y));
			V->SetAttribute("z", Core::Var::Number(Value.Z));
			V->SetAttribute("w", Core::Var::Number(Value.W));
		}
		void HeavySeries::Pack(Core::Schema* V, const Trigonometry::Quaternion& Value)
		{
			VI_ASSERT(V != nullptr, "schema should be set");
			V->SetAttribute("x", Core::Var::Number(Value.X));
			V->SetAttribute("y", Core::Var::Number(Value.Y));
			V->SetAttribute("z", Core::Var::Number(Value.Z));
			V->SetAttribute("w", Core::Var::Number(Value.W));
		}
		void HeavySeries::Pack(Core::Schema* V, const Trigonometry::Matrix4x4& Value)
		{
			VI_ASSERT(V != nullptr, "schema should be set");
			V->SetAttribute("m11", Core::Var::Number(Value.Row[0]));
			V->SetAttribute("m12", Core::Var::Number(Value.Row[1]));
			V->SetAttribute("m13", Core::Var::Number(Value.Row[2]));
			V->SetAttribute("m14", Core::Var::Number(Value.Row[3]));
			V->SetAttribute("m21", Core::Var::Number(Value.Row[4]));
			V->SetAttribute("m22", Core::Var::Number(Value.Row[5]));
			V->SetAttribute("m23", Core::Var::Number(Value.Row[6]));
			V->SetAttribute("m24", Core::Var::Number(Value.Row[7]));
			V->SetAttribute("m31", Core::Var::Number(Value.Row[8]));
			V->SetAttribute("m32", Core::Var::Number(Value.Row[9]));
			V->SetAttribute("m33", Core::Var::Number(Value.Row[10]));
			V->SetAttribute("m34", Core::Var::Number(Value.Row[11]));
			V->SetAttribute("m41", Core::Var::Number(Value.Row[12]));
			V->SetAttribute("m42", Core::Var::Number(Value.Row[13]));
			V->SetAttribute("m43", Core::Var::Number(Value.Row[14]));
			V->SetAttribute("m44", Core::Var::Number(Value.Row[15]));
		}
		void HeavySeries::Pack(Core::Schema* V, const Attenuation& Value)
		{
			VI_ASSERT(V != nullptr, "schema should be set");
			Series::Pack(V->Set("radius"), Value.Radius);
			Series::Pack(V->Set("c1"), Value.C1);
			Series::Pack(V->Set("c2"), Value.C2);
		}
		void HeavySeries::Pack(Core::Schema* V, const AnimatorState& Value)
		{
			VI_ASSERT(V != nullptr, "schema should be set");
			Series::Pack(V->Set("looped"), Value.Looped);
			Series::Pack(V->Set("paused"), Value.Paused);
			Series::Pack(V->Set("blended"), Value.Blended);
			Series::Pack(V->Set("clip"), Value.Clip);
			Series::Pack(V->Set("frame"), Value.Frame);
			Series::Pack(V->Set("rate"), Value.Rate);
			Series::Pack(V->Set("duration"), Value.Duration);
			Series::Pack(V->Set("time"), Value.Time);
		}
		void HeavySeries::Pack(Core::Schema* V, const SpawnerProperties& Value)
		{
			VI_ASSERT(V != nullptr, "schema should be set");
			Series::Pack(V->Set("iterations"), Value.Iterations);

			Core::Schema* Angular = V->Set("angular");
			Series::Pack(Angular->Set("intensity"), Value.Angular.Intensity);
			Series::Pack(Angular->Set("accuracy"), Value.Angular.Accuracy);
			Series::Pack(Angular->Set("min"), Value.Angular.Min);
			Series::Pack(Angular->Set("max"), Value.Angular.Max);

			Core::Schema* Diffusion = V->Set("diffusion");
			Series::Pack(Diffusion->Set("intensity"), Value.Diffusion.Intensity);
			Series::Pack(Diffusion->Set("accuracy"), Value.Diffusion.Accuracy);
			HeavySeries::Pack(Diffusion->Set("min"), Value.Diffusion.Min);
			HeavySeries::Pack(Diffusion->Set("max"), Value.Diffusion.Max);

			Core::Schema* Noise = V->Set("noise");
			Series::Pack(Noise->Set("intensity"), Value.Noise.Intensity);
			Series::Pack(Noise->Set("accuracy"), Value.Noise.Accuracy);
			HeavySeries::Pack(Noise->Set("min"), Value.Noise.Min);
			HeavySeries::Pack(Noise->Set("max"), Value.Noise.Max);

			Core::Schema* Position = V->Set("position");
			Series::Pack(Position->Set("intensity"), Value.Position.Intensity);
			Series::Pack(Position->Set("accuracy"), Value.Position.Accuracy);
			HeavySeries::Pack(Position->Set("min"), Value.Position.Min);
			HeavySeries::Pack(Position->Set("max"), Value.Position.Max);

			Core::Schema* Rotation = V->Set("rotation");
			Series::Pack(Rotation->Set("intensity"), Value.Rotation.Intensity);
			Series::Pack(Rotation->Set("accuracy"), Value.Rotation.Accuracy);
			Series::Pack(Rotation->Set("min"), Value.Rotation.Min);
			Series::Pack(Rotation->Set("max"), Value.Rotation.Max);

			Core::Schema* Scale = V->Set("scale");
			Series::Pack(Scale->Set("intensity"), Value.Scale.Intensity);
			Series::Pack(Scale->Set("accuracy"), Value.Scale.Accuracy);
			Series::Pack(Scale->Set("min"), Value.Scale.Min);
			Series::Pack(Scale->Set("max"), Value.Scale.Max);

			Core::Schema* Velocity = V->Set("velocity");
			Series::Pack(Velocity->Set("intensity"), Value.Velocity.Intensity);
			Series::Pack(Velocity->Set("accuracy"), Value.Velocity.Accuracy);
			HeavySeries::Pack(Velocity->Set("min"), Value.Velocity.Min);
			HeavySeries::Pack(Velocity->Set("max"), Value.Velocity.Max);
		}
		void HeavySeries::Pack(Core::Schema* V, const Trigonometry::KeyAnimatorClip& Value)
		{
			VI_ASSERT(V != nullptr, "schema should be set");
			Series::Pack(V->Set("name"), Value.Name);
			Series::Pack(V->Set("rate"), Value.Rate);
			Series::Pack(V->Set("duration"), Value.Duration);
			HeavySeries::Pack(V->Set("frames"), Value.Keys);
		}
		void HeavySeries::Pack(Core::Schema* V, const Trigonometry::AnimatorKey& Value)
		{
			VI_ASSERT(V != nullptr, "schema should be set");
			HeavySeries::Pack(V->Set("position"), Value.Position);
			HeavySeries::Pack(V->Set("rotation"), Value.Rotation);
			HeavySeries::Pack(V->Set("scale"), Value.Scale);
			Series::Pack(V->Set("time"), Value.Time);
		}
		void HeavySeries::Pack(Core::Schema* V, const Trigonometry::SkinAnimatorKey& Value)
		{
			VI_ASSERT(V != nullptr, "schema should be set");
			HeavySeries::Pack(V->Set("pose"), Value.Pose);
			Series::Pack(V->Set("time"), Value.Time);
		}
		void HeavySeries::Pack(Core::Schema* V, const Trigonometry::ElementVertex& Value)
		{
			VI_ASSERT(V != nullptr, "schema should be set");
			V->SetAttribute("px", Core::Var::Number(Value.PositionX));
			V->SetAttribute("py", Core::Var::Number(Value.PositionY));
			V->SetAttribute("pz", Core::Var::Number(Value.PositionZ));
			V->SetAttribute("vx", Core::Var::Number(Value.VelocityX));
			V->SetAttribute("vy", Core::Var::Number(Value.VelocityY));
			V->SetAttribute("vz", Core::Var::Number(Value.VelocityZ));
			V->SetAttribute("cx", Core::Var::Number(Value.ColorX));
			V->SetAttribute("cy", Core::Var::Number(Value.ColorY));
			V->SetAttribute("cz", Core::Var::Number(Value.ColorZ));
			V->SetAttribute("cw", Core::Var::Number(Value.ColorW));
			V->SetAttribute("a", Core::Var::Number(Value.Angular));
			V->SetAttribute("s", Core::Var::Number(Value.Scale));
			V->SetAttribute("r", Core::Var::Number(Value.Rotation));
		}
		void HeavySeries::Pack(Core::Schema* V, const Trigonometry::Vertex& Value)
		{
			VI_ASSERT(V != nullptr, "schema should be set");
			V->SetAttribute("px", Core::Var::Number(Value.PositionX));
			V->SetAttribute("py", Core::Var::Number(Value.PositionY));
			V->SetAttribute("pz", Core::Var::Number(Value.PositionZ));
			V->SetAttribute("tx", Core::Var::Number(Value.TexCoordX));
			V->SetAttribute("ty", Core::Var::Number(Value.TexCoordY));
			V->SetAttribute("nx", Core::Var::Number(Value.NormalX));
			V->SetAttribute("ny", Core::Var::Number(Value.NormalY));
			V->SetAttribute("nz", Core::Var::Number(Value.NormalZ));
			V->SetAttribute("tnx", Core::Var::Number(Value.TangentX));
			V->SetAttribute("tny", Core::Var::Number(Value.TangentY));
			V->SetAttribute("tnz", Core::Var::Number(Value.TangentZ));
			V->SetAttribute("btx", Core::Var::Number(Value.BitangentX));
			V->SetAttribute("bty", Core::Var::Number(Value.BitangentY));
			V->SetAttribute("btz", Core::Var::Number(Value.BitangentZ));
		}
		void HeavySeries::Pack(Core::Schema* V, const Trigonometry::SkinVertex& Value)
		{
			VI_ASSERT(V != nullptr, "schema should be set");
			V->SetAttribute("px", Core::Var::Number(Value.PositionX));
			V->SetAttribute("py", Core::Var::Number(Value.PositionY));
			V->SetAttribute("pz", Core::Var::Number(Value.PositionZ));
			V->SetAttribute("tx", Core::Var::Number(Value.TexCoordX));
			V->SetAttribute("ty", Core::Var::Number(Value.TexCoordY));
			V->SetAttribute("nx", Core::Var::Number(Value.NormalX));
			V->SetAttribute("ny", Core::Var::Number(Value.NormalY));
			V->SetAttribute("nz", Core::Var::Number(Value.NormalZ));
			V->SetAttribute("tnx", Core::Var::Number(Value.TangentX));
			V->SetAttribute("tny", Core::Var::Number(Value.TangentY));
			V->SetAttribute("tnz", Core::Var::Number(Value.TangentZ));
			V->SetAttribute("btx", Core::Var::Number(Value.BitangentX));
			V->SetAttribute("bty", Core::Var::Number(Value.BitangentY));
			V->SetAttribute("btz", Core::Var::Number(Value.BitangentZ));
			V->SetAttribute("ji0", Core::Var::Number(Value.JointIndex0));
			V->SetAttribute("ji1", Core::Var::Number(Value.JointIndex1));
			V->SetAttribute("ji2", Core::Var::Number(Value.JointIndex2));
			V->SetAttribute("ji3", Core::Var::Number(Value.JointIndex3));
			V->SetAttribute("jb0", Core::Var::Number(Value.JointBias0));
			V->SetAttribute("jb1", Core::Var::Number(Value.JointBias1));
			V->SetAttribute("jb2", Core::Var::Number(Value.JointBias2));
			V->SetAttribute("jb3", Core::Var::Number(Value.JointBias3));
		}
		void HeavySeries::Pack(Core::Schema* V, const Trigonometry::Joint& Value)
		{
			VI_ASSERT(V != nullptr, "schema should be set");
			Series::Pack(V->Set("index"), Value.Index);
			Series::Pack(V->Set("name"), Value.Name);
			HeavySeries::Pack(V->Set("global"), Value.Global);
			HeavySeries::Pack(V->Set("local"), Value.Local);

			Core::Schema* Joints = V->Set("childs", Core::Var::Array());
			for (auto& It : Value.Childs)
				HeavySeries::Pack(Joints->Set("joint"), It);
		}
		void HeavySeries::Pack(Core::Schema* V, const Ticker& Value)
		{
			VI_ASSERT(V != nullptr, "schema should be set");
			V->SetAttribute("delay", Core::Var::Number(Value.Delay));
		}
		void HeavySeries::Pack(Core::Schema* V, const Core::Vector<Trigonometry::Vector2>& Value)
		{
			VI_ASSERT(V != nullptr, "schema should be set");
			Core::StringStream Stream;
			for (auto&& It : Value)
				Stream << It.X << " " << It.Y << " ";

			V->Set("v2-array", Core::Var::String(Stream.str().substr(0, Stream.str().size() - 1)));
			V->Set("size", Core::Var::Integer((int64_t)Value.size()));
		}
		void HeavySeries::Pack(Core::Schema* V, const Core::Vector<Trigonometry::Vector3>& Value)
		{
			VI_ASSERT(V != nullptr, "schema should be set");
			Core::StringStream Stream;
			for (auto&& It : Value)
				Stream << It.X << " " << It.Y << " " << It.Z << " ";

			V->Set("v3-array", Core::Var::String(Stream.str().substr(0, Stream.str().size() - 1)));
			V->Set("size", Core::Var::Integer((int64_t)Value.size()));
		}
		void HeavySeries::Pack(Core::Schema* V, const Core::Vector<Trigonometry::Vector4>& Value)
		{
			VI_ASSERT(V != nullptr, "schema should be set");
			Core::StringStream Stream;
			for (auto&& It : Value)
				Stream << It.X << " " << It.Y << " " << It.Z << " " << It.W << " ";

			V->Set("v4-array", Core::Var::String(Stream.str().substr(0, Stream.str().size() - 1)));
			V->Set("size", Core::Var::Integer((int64_t)Value.size()));
		}
		void HeavySeries::Pack(Core::Schema* V, const Core::Vector<Trigonometry::Matrix4x4>& Value)
		{
			VI_ASSERT(V != nullptr, "schema should be set");
			Core::StringStream Stream;
			for (auto&& It : Value)
			{
				for (float i : It.Row)
					Stream << i << " ";
			}

			V->Set("m4x4-array", Core::Var::String(Stream.str().substr(0, Stream.str().size() - 1)));
			V->Set("size", Core::Var::Integer((int64_t)Value.size()));
		}
		void HeavySeries::Pack(Core::Schema* V, const Core::Vector<AnimatorState>& Value)
		{
			VI_ASSERT(V != nullptr, "schema should be set");
			Core::StringStream Stream;
			for (auto&& It : Value)
			{
				Stream << It.Paused << " ";
				Stream << It.Looped << " ";
				Stream << It.Blended << " ";
				Stream << It.Duration << " ";
				Stream << It.Rate << " ";
				Stream << It.Time << " ";
				Stream << It.Frame << " ";
				Stream << It.Clip << " ";
			}

			V->Set("as-array", Core::Var::String(Stream.str().substr(0, Stream.str().size() - 1)));
			V->Set("size", Core::Var::Integer((int64_t)Value.size()));
		}
		void HeavySeries::Pack(Core::Schema* V, const Core::Vector<SpawnerProperties>& Value)
		{
			VI_ASSERT(V != nullptr, "schema should be set");
			Core::StringStream Stream;
			for (auto&& It : Value)
			{
				Stream << It.Angular.Accuracy << " " << It.Angular.Min << " " << It.Angular.Max << " ";
				Stream << It.Rotation.Accuracy << " " << It.Rotation.Min << " " << It.Rotation.Max << " ";
				Stream << It.Scale.Accuracy << " " << It.Scale.Min << " " << It.Scale.Max << " ";
				Stream << It.Diffusion.Accuracy << " ";
				Stream << It.Diffusion.Min.X << " " << It.Diffusion.Min.Y << " " << It.Diffusion.Min.Z << " " << It.Diffusion.Min.W << " ";
				Stream << It.Diffusion.Max.X << " " << It.Diffusion.Max.Y << " " << It.Diffusion.Max.Z << " " << It.Diffusion.Max.W << " ";
				Stream << It.Noise.Accuracy << " ";
				Stream << It.Noise.Min.X << " " << It.Noise.Min.Y << " " << It.Noise.Min.Z << " ";
				Stream << It.Noise.Max.X << " " << It.Noise.Max.Y << " " << It.Noise.Max.Z << " ";
				Stream << It.Position.Accuracy << " ";
				Stream << It.Position.Min.X << " " << It.Position.Min.Y << " " << It.Position.Min.Z << " ";
				Stream << It.Position.Max.X << " " << It.Position.Max.Y << " " << It.Position.Max.Z << " ";
				Stream << It.Velocity.Accuracy << " ";
				Stream << It.Velocity.Min.X << " " << It.Velocity.Min.Y << " " << It.Velocity.Min.Z << " ";
				Stream << It.Velocity.Max.X << " " << It.Velocity.Max.Y << " " << It.Velocity.Max.Z << " ";
				Stream << It.Iterations << " ";
			}

			V->Set("sp-array", Core::Var::String(Stream.str().substr(0, Stream.str().size() - 1)));
			V->Set("size", Core::Var::Integer((int64_t)Value.size()));
		}
		void HeavySeries::Pack(Core::Schema* V, const Core::Vector<Trigonometry::KeyAnimatorClip>& Value)
		{
			VI_ASSERT(V != nullptr, "schema should be set");
			Core::Schema* Array = V->Set("clips", Core::Var::Array());
			for (auto&& It : Value)
				HeavySeries::Pack(Array->Set("clip"), It);
		}
		void HeavySeries::Pack(Core::Schema* V, const Core::Vector<Trigonometry::AnimatorKey>& Value)
		{
			VI_ASSERT(V != nullptr, "schema should be set");
			Core::StringStream Stream;
			for (auto&& It : Value)
			{
				Stream << It.Position.X << " ";
				Stream << It.Position.Y << " ";
				Stream << It.Position.Z << " ";
				Stream << It.Rotation.X << " ";
				Stream << It.Rotation.Y << " ";
				Stream << It.Rotation.Z << " ";
				Stream << It.Rotation.W << " ";
				Stream << It.Scale.X << " ";
				Stream << It.Scale.Y << " ";
				Stream << It.Scale.Z << " ";
				Stream << It.Time << " ";
			}

			V->Set("ak-array", Core::Var::String(Stream.str().substr(0, Stream.str().size() - 1)));
			V->Set("size", Core::Var::Integer((int64_t)Value.size()));
		}
		void HeavySeries::Pack(Core::Schema* V, const Core::Vector<Trigonometry::ElementVertex>& Value)
		{
			VI_ASSERT(V != nullptr, "schema should be set");
			Core::StringStream Stream;
			for (auto&& It : Value)
			{
				Stream << It.PositionX << " ";
				Stream << It.PositionY << " ";
				Stream << It.PositionZ << " ";
				Stream << It.ColorX << " ";
				Stream << It.ColorY << " ";
				Stream << It.ColorZ << " ";
				Stream << It.ColorW << " ";
				Stream << It.VelocityX << " ";
				Stream << It.VelocityY << " ";
				Stream << It.VelocityZ << " ";
				Stream << It.Angular << " ";
				Stream << It.Rotation << " ";
				Stream << It.Scale << " ";
			}

			V->Set("ev-array", Core::Var::String(Stream.str().substr(0, Stream.str().size() - 1)));
			V->Set("size", Core::Var::Integer((int64_t)Value.size()));
		}
		void HeavySeries::Pack(Core::Schema* V, const Core::Vector<Trigonometry::Joint>& Value)
		{
			VI_ASSERT(V != nullptr, "schema should be set");
			for (auto&& It : Value)
				HeavySeries::Pack(V->Set("joint"), It);
		}
		void HeavySeries::Pack(Core::Schema* V, const Core::Vector<Trigonometry::Vertex>& Value)
		{
			VI_ASSERT(V != nullptr, "schema should be set");
			Core::StringStream Stream;
			for (auto&& It : Value)
			{
				Stream << It.PositionX << " ";
				Stream << It.PositionY << " ";
				Stream << It.PositionZ << " ";
				Stream << It.TexCoordX << " ";
				Stream << It.TexCoordY << " ";
				Stream << It.NormalX << " ";
				Stream << It.NormalY << " ";
				Stream << It.NormalZ << " ";
				Stream << It.TangentX << " ";
				Stream << It.TangentY << " ";
				Stream << It.TangentZ << " ";
				Stream << It.BitangentX << " ";
				Stream << It.BitangentY << " ";
				Stream << It.BitangentZ << " ";
				Stream << "-1 -1 -1 -1 0 0 0 0 ";
			}

			V->Set("iv-array", Core::Var::String(Stream.str().substr(0, Stream.str().size() - 1)));
			V->Set("size", Core::Var::Integer((int64_t)Value.size()));
		}
		void HeavySeries::Pack(Core::Schema* V, const Core::Vector<Trigonometry::SkinVertex>& Value)
		{
			VI_ASSERT(V != nullptr, "schema should be set");
			Core::StringStream Stream;
			for (auto&& It : Value)
			{
				Stream << It.PositionX << " ";
				Stream << It.PositionY << " ";
				Stream << It.PositionZ << " ";
				Stream << It.TexCoordX << " ";
				Stream << It.TexCoordY << " ";
				Stream << It.NormalX << " ";
				Stream << It.NormalY << " ";
				Stream << It.NormalZ << " ";
				Stream << It.TangentX << " ";
				Stream << It.TangentY << " ";
				Stream << It.TangentZ << " ";
				Stream << It.BitangentX << " ";
				Stream << It.BitangentY << " ";
				Stream << It.BitangentZ << " ";
				Stream << It.JointIndex0 << " ";
				Stream << It.JointIndex1 << " ";
				Stream << It.JointIndex2 << " ";
				Stream << It.JointIndex3 << " ";
				Stream << It.JointBias0 << " ";
				Stream << It.JointBias1 << " ";
				Stream << It.JointBias2 << " ";
				Stream << It.JointBias3 << " ";
			}

			V->Set("iv-array", Core::Var::String(Stream.str().substr(0, Stream.str().size() - 1)));
			V->Set("size", Core::Var::Integer((int64_t)Value.size()));
		}
		void HeavySeries::Pack(Core::Schema* V, const Core::Vector<Ticker>& Value)
		{
			VI_ASSERT(V != nullptr, "schema should be set");
			Core::StringStream Stream;
			for (auto&& It : Value)
				Stream << It.Delay << " ";

			V->Set("tt-array", Core::Var::String(Stream.str().substr(0, Stream.str().size() - 1)));
			V->Set("size", Core::Var::Integer((int64_t)Value.size()));
		}
		bool HeavySeries::Unpack(Core::Schema* V, Trigonometry::Vector2* O)
		{
			VI_ASSERT(O != nullptr, "output should be set");
			if (!V)
				return false;

			O->X = (float)V->GetAttributeVar("x").GetNumber();
			O->Y = (float)V->GetAttributeVar("y").GetNumber();
			return true;
		}
		bool HeavySeries::Unpack(Core::Schema* V, Trigonometry::Vector3* O)
		{
			VI_ASSERT(O != nullptr, "output should be set");
			if (!V)
				return false;

			O->X = (float)V->GetAttributeVar("x").GetNumber();
			O->Y = (float)V->GetAttributeVar("y").GetNumber();
			O->Z = (float)V->GetAttributeVar("z").GetNumber();
			return true;
		}
		bool HeavySeries::Unpack(Core::Schema* V, Trigonometry::Vector4* O)
		{
			VI_ASSERT(O != nullptr, "output should be set");
			if (!V)
				return false;

			O->X = (float)V->GetAttributeVar("x").GetNumber();
			O->Y = (float)V->GetAttributeVar("y").GetNumber();
			O->Z = (float)V->GetAttributeVar("z").GetNumber();
			O->W = (float)V->GetAttributeVar("w").GetNumber();
			return true;
		}
		bool HeavySeries::Unpack(Core::Schema* V, Trigonometry::Quaternion* O)
		{
			VI_ASSERT(O != nullptr, "output should be set");
			if (!V)
				return false;

			O->X = (float)V->GetAttributeVar("x").GetNumber();
			O->Y = (float)V->GetAttributeVar("y").GetNumber();
			O->Z = (float)V->GetAttributeVar("z").GetNumber();
			O->W = (float)V->GetAttributeVar("w").GetNumber();
			return true;
		}
		bool HeavySeries::Unpack(Core::Schema* V, Trigonometry::Matrix4x4* O)
		{
			VI_ASSERT(O != nullptr, "output should be set");
			if (!V)
				return false;

			O->Row[0] = (float)V->GetAttributeVar("m11").GetNumber();
			O->Row[1] = (float)V->GetAttributeVar("m12").GetNumber();
			O->Row[2] = (float)V->GetAttributeVar("m13").GetNumber();
			O->Row[3] = (float)V->GetAttributeVar("m14").GetNumber();
			O->Row[4] = (float)V->GetAttributeVar("m21").GetNumber();
			O->Row[5] = (float)V->GetAttributeVar("m22").GetNumber();
			O->Row[6] = (float)V->GetAttributeVar("m23").GetNumber();
			O->Row[7] = (float)V->GetAttributeVar("m24").GetNumber();
			O->Row[8] = (float)V->GetAttributeVar("m31").GetNumber();
			O->Row[9] = (float)V->GetAttributeVar("m32").GetNumber();
			O->Row[10] = (float)V->GetAttributeVar("m33").GetNumber();
			O->Row[11] = (float)V->GetAttributeVar("m34").GetNumber();
			O->Row[12] = (float)V->GetAttributeVar("m41").GetNumber();
			O->Row[13] = (float)V->GetAttributeVar("m42").GetNumber();
			O->Row[14] = (float)V->GetAttributeVar("m43").GetNumber();
			O->Row[15] = (float)V->GetAttributeVar("m44").GetNumber();
			return true;
		}
		bool HeavySeries::Unpack(Core::Schema* V, Attenuation* O)
		{
			VI_ASSERT(O != nullptr, "output should be set");
			if (!V)
				return false;

			Series::Unpack(V->Get("radius"), &O->Radius);
			Series::Unpack(V->Get("c1"), &O->C1);
			Series::Unpack(V->Get("c2"), &O->C2);
			return true;
		}
		bool HeavySeries::Unpack(Core::Schema* V, AnimatorState* O)
		{
			VI_ASSERT(O != nullptr, "output should be set");
			if (!V)
				return false;

			Series::Unpack(V->Get("looped"), &O->Looped);
			Series::Unpack(V->Get("paused"), &O->Paused);
			Series::Unpack(V->Get("blended"), &O->Blended);
			Series::Unpack(V->Get("clip"), &O->Clip);
			Series::Unpack(V->Get("frame"), &O->Frame);
			Series::Unpack(V->Get("rate"), &O->Rate);
			Series::Unpack(V->Get("duration"), &O->Duration);
			Series::Unpack(V->Get("time"), &O->Time);
			return true;
		}
		bool HeavySeries::Unpack(Core::Schema* V, SpawnerProperties* O)
		{
			VI_ASSERT(O != nullptr, "output should be set");
			if (!V)
				return false;

			Series::Unpack(V->Get("iterations"), &O->Iterations);

			Core::Schema* Angular = V->Get("angular");
			Series::Unpack(Angular->Get("intensity"), &O->Angular.Intensity);
			Series::Unpack(Angular->Get("accuracy"), &O->Angular.Accuracy);
			Series::Unpack(Angular->Get("min"), &O->Angular.Min);
			Series::Unpack(Angular->Get("max"), &O->Angular.Max);

			Core::Schema* Diffusion = V->Get("diffusion");
			Series::Unpack(Diffusion->Get("intensity"), &O->Diffusion.Intensity);
			Series::Unpack(Diffusion->Get("accuracy"), &O->Diffusion.Accuracy);
			HeavySeries::Unpack(Diffusion->Get("min"), &O->Diffusion.Min);
			HeavySeries::Unpack(Diffusion->Get("max"), &O->Diffusion.Max);

			Core::Schema* Noise = V->Get("noise");
			Series::Unpack(Noise->Get("intensity"), &O->Noise.Intensity);
			Series::Unpack(Noise->Get("accuracy"), &O->Noise.Accuracy);
			HeavySeries::Unpack(Noise->Get("min"), &O->Noise.Min);
			HeavySeries::Unpack(Noise->Get("max"), &O->Noise.Max);

			Core::Schema* Position = V->Get("position");
			Series::Unpack(Position->Get("intensity"), &O->Position.Intensity);
			Series::Unpack(Position->Get("accuracy"), &O->Position.Accuracy);
			HeavySeries::Unpack(Position->Get("min"), &O->Position.Min);
			HeavySeries::Unpack(Position->Get("max"), &O->Position.Max);

			Core::Schema* Rotation = V->Get("rotation");
			Series::Unpack(Rotation->Get("intensity"), &O->Rotation.Intensity);
			Series::Unpack(Rotation->Get("accuracy"), &O->Rotation.Accuracy);
			Series::Unpack(Rotation->Get("min"), &O->Rotation.Min);
			Series::Unpack(Rotation->Get("max"), &O->Rotation.Max);

			Core::Schema* Scale = V->Get("scale");
			Series::Unpack(Scale->Get("intensity"), &O->Scale.Intensity);
			Series::Unpack(Scale->Get("accuracy"), &O->Scale.Accuracy);
			Series::Unpack(Scale->Get("min"), &O->Scale.Min);
			Series::Unpack(Scale->Get("max"), &O->Scale.Max);

			Core::Schema* Velocity = V->Get("velocity");
			Series::Unpack(Velocity->Get("intensity"), &O->Velocity.Intensity);
			Series::Unpack(Velocity->Get("accuracy"), &O->Velocity.Accuracy);
			HeavySeries::Unpack(Velocity->Get("min"), &O->Velocity.Min);
			HeavySeries::Unpack(Velocity->Get("max"), &O->Velocity.Max);

			return true;
		}
		bool HeavySeries::Unpack(Core::Schema* V, Trigonometry::KeyAnimatorClip* O)
		{
			VI_ASSERT(O != nullptr, "output should be set");
			if (!V)
				return false;

			Series::Unpack(V->Get("name"), &O->Name);
			Series::Unpack(V->Get("duration"), &O->Duration);
			Series::Unpack(V->Get("rate"), &O->Rate);
			HeavySeries::Unpack(V->Get("frames"), &O->Keys);
			return true;
		}
		bool HeavySeries::Unpack(Core::Schema* V, Trigonometry::AnimatorKey* O)
		{
			VI_ASSERT(O != nullptr, "output should be set");
			if (!V)
				return false;

			HeavySeries::Unpack(V->Get("position"), &O->Position);
			HeavySeries::Unpack(V->Get("rotation"), &O->Rotation);
			HeavySeries::Unpack(V->Get("scale"), &O->Scale);
			Series::Unpack(V->Get("time"), &O->Time);
			return true;
		}
		bool HeavySeries::Unpack(Core::Schema* V, Trigonometry::SkinAnimatorKey* O)
		{
			VI_ASSERT(O != nullptr, "output should be set");
			if (!V)
				return false;

			HeavySeries::Unpack(V->Get("pose"), &O->Pose);
			Series::Unpack(V->Get("time"), &O->Time);

			return true;
		}
		bool HeavySeries::Unpack(Core::Schema* V, Trigonometry::Joint* O)
		{
			VI_ASSERT(O != nullptr, "output should be set");
			if (!V)
				return false;

			Series::Unpack(V->Get("index"), &O->Index);
			Series::Unpack(V->Get("name"), &O->Name);
			HeavySeries::Unpack(V->Get("global"), &O->Global);
			HeavySeries::Unpack(V->Get("local"), &O->Local);

			Core::Vector<Core::Schema*> Joints = V->FetchCollection("childs.joint", false);
			for (auto& It : Joints)
			{
				O->Childs.emplace_back();
				HeavySeries::Unpack(It, &O->Childs.back());
			}

			return true;
		}
		bool HeavySeries::Unpack(Core::Schema* V, Trigonometry::ElementVertex* O)
		{
			VI_ASSERT(O != nullptr, "output should be set");
			if (!V)
				return false;

			O->PositionX = (float)V->GetAttributeVar("px").GetNumber();
			O->PositionY = (float)V->GetAttributeVar("py").GetNumber();
			O->PositionZ = (float)V->GetAttributeVar("pz").GetNumber();
			O->VelocityX = (float)V->GetAttributeVar("vx").GetNumber();
			O->VelocityY = (float)V->GetAttributeVar("vy").GetNumber();
			O->VelocityZ = (float)V->GetAttributeVar("vz").GetNumber();
			O->ColorX = (float)V->GetAttributeVar("cx").GetNumber();
			O->ColorY = (float)V->GetAttributeVar("cy").GetNumber();
			O->ColorZ = (float)V->GetAttributeVar("cz").GetNumber();
			O->ColorW = (float)V->GetAttributeVar("cw").GetNumber();
			O->Angular = (float)V->GetAttributeVar("a").GetNumber();
			O->Scale = (float)V->GetAttributeVar("s").GetNumber();
			O->Rotation = (float)V->GetAttributeVar("r").GetNumber();
			return true;
		}
		bool HeavySeries::Unpack(Core::Schema* V, Trigonometry::Vertex* O)
		{
			VI_ASSERT(O != nullptr, "output should be set");
			if (!V)
				return false;

			O->PositionX = (float)V->GetAttributeVar("px").GetNumber();
			O->PositionY = (float)V->GetAttributeVar("py").GetNumber();
			O->PositionZ = (float)V->GetAttributeVar("pz").GetNumber();
			O->TexCoordX = (float)V->GetAttributeVar("tx").GetNumber();
			O->TexCoordY = (float)V->GetAttributeVar("ty").GetNumber();
			O->NormalX = (float)V->GetAttributeVar("nx").GetNumber();
			O->NormalY = (float)V->GetAttributeVar("ny").GetNumber();
			O->NormalZ = (float)V->GetAttributeVar("nz").GetNumber();
			O->TangentX = (float)V->GetAttributeVar("tnx").GetNumber();
			O->TangentY = (float)V->GetAttributeVar("tny").GetNumber();
			O->TangentZ = (float)V->GetAttributeVar("tnz").GetNumber();
			O->BitangentX = (float)V->GetAttributeVar("btx").GetNumber();
			O->BitangentY = (float)V->GetAttributeVar("bty").GetNumber();
			O->BitangentZ = (float)V->GetAttributeVar("btz").GetNumber();
			return true;
		}
		bool HeavySeries::Unpack(Core::Schema* V, Trigonometry::SkinVertex* O)
		{
			VI_ASSERT(O != nullptr, "output should be set");
			if (!V)
				return false;

			O->PositionX = (float)V->GetAttributeVar("px").GetNumber();
			O->PositionY = (float)V->GetAttributeVar("py").GetNumber();
			O->PositionZ = (float)V->GetAttributeVar("pz").GetNumber();
			O->TexCoordX = (float)V->GetAttributeVar("tx").GetNumber();
			O->TexCoordY = (float)V->GetAttributeVar("ty").GetNumber();
			O->NormalX = (float)V->GetAttributeVar("nx").GetNumber();
			O->NormalY = (float)V->GetAttributeVar("ny").GetNumber();
			O->NormalZ = (float)V->GetAttributeVar("nz").GetNumber();
			O->TangentX = (float)V->GetAttributeVar("tnx").GetNumber();
			O->TangentY = (float)V->GetAttributeVar("tny").GetNumber();
			O->TangentZ = (float)V->GetAttributeVar("tnz").GetNumber();
			O->BitangentX = (float)V->GetAttributeVar("btx").GetNumber();
			O->BitangentY = (float)V->GetAttributeVar("bty").GetNumber();
			O->BitangentZ = (float)V->GetAttributeVar("btz").GetNumber();
			O->JointIndex0 = (float)V->GetAttributeVar("ji0").GetNumber();
			O->JointIndex1 = (float)V->GetAttributeVar("ji1").GetNumber();
			O->JointIndex2 = (float)V->GetAttributeVar("ji2").GetNumber();
			O->JointIndex3 = (float)V->GetAttributeVar("ji3").GetNumber();
			O->JointBias0 = (float)V->GetAttributeVar("jb0").GetNumber();
			O->JointBias1 = (float)V->GetAttributeVar("jb1").GetNumber();
			O->JointBias2 = (float)V->GetAttributeVar("jb2").GetNumber();
			O->JointBias3 = (float)V->GetAttributeVar("jb3").GetNumber();
			return true;
		}
		bool HeavySeries::Unpack(Core::Schema* V, Ticker* O)
		{
			VI_ASSERT(O != nullptr, "output should be set");
			if (!V)
				return false;

			O->Delay = (float)V->GetAttributeVar("delay").GetNumber();
			return true;
		}
		bool HeavySeries::Unpack(Core::Schema* V, Core::Vector<Trigonometry::Vector2>* O)
		{
			VI_ASSERT(O != nullptr, "output should be set");
			if (!V)
				return false;

			Core::String Array(V->GetVar("v2-array").GetBlob());
			int64_t Size = V->GetVar("size").GetInteger();
			if (Array.empty() || !Size)
				return false;

			Core::StringStream Stream(Array);
			O->resize((size_t)Size);

			for (auto& It : *O)
				Stream >> It.X >> It.Y;

			return true;
		}
		bool HeavySeries::Unpack(Core::Schema* V, Core::Vector<Trigonometry::Vector3>* O)
		{
			VI_ASSERT(O != nullptr, "output should be set");
			if (!V)
				return false;

			Core::String Array(V->GetVar("v3-array").GetBlob());
			int64_t Size = V->GetVar("size").GetInteger();
			if (Array.empty() || !Size)
				return false;

			Core::StringStream Stream(Array);
			O->resize((size_t)Size);

			for (auto& It : *O)
				Stream >> It.X >> It.Y >> It.Z;

			return true;
		}
		bool HeavySeries::Unpack(Core::Schema* V, Core::Vector<Trigonometry::Vector4>* O)
		{
			VI_ASSERT(O != nullptr, "output should be set");
			if (!V)
				return false;

			Core::String Array(V->GetVar("v4-array").GetBlob());
			int64_t Size = V->GetVar("size").GetInteger();
			if (Array.empty() || !Size)
				return false;

			Core::StringStream Stream(Array);
			O->resize((size_t)Size);

			for (auto& It : *O)
				Stream >> It.X >> It.Y >> It.Z >> It.W;

			return true;
		}
		bool HeavySeries::Unpack(Core::Schema* V, Core::Vector<Trigonometry::Matrix4x4>* O)
		{
			VI_ASSERT(O != nullptr, "output should be set");
			if (!V)
				return false;

			Core::String Array(V->GetVar("m4x4-array").GetBlob());
			int64_t Size = V->GetVar("size").GetInteger();
			if (Array.empty() || !Size)
				return false;

			Core::StringStream Stream(Array);
			O->resize((size_t)Size);

			for (auto& It : *O)
			{
				for (int64_t i = 0; i < 16; i++)
					Stream >> It.Row[i];
			}

			return true;
		}
		bool HeavySeries::Unpack(Core::Schema* V, Core::Vector<AnimatorState>* O)
		{
			VI_ASSERT(O != nullptr, "output should be set");
			if (!V)
				return false;

			Core::String Array(V->GetVar("as-array").GetBlob());
			int64_t Size = V->GetVar("size").GetInteger();
			if (Array.empty() || !Size)
				return false;

			Core::StringStream Stream(Array);
			O->resize((size_t)Size);

			for (auto& It : *O)
			{
				Stream >> It.Paused;
				Stream >> It.Looped;
				Stream >> It.Blended;
				Stream >> It.Duration;
				Stream >> It.Rate;
				Stream >> It.Time;
				Stream >> It.Frame;
				Stream >> It.Clip;
			}

			return true;
		}
		bool HeavySeries::Unpack(Core::Schema* V, Core::Vector<SpawnerProperties>* O)
		{
			VI_ASSERT(O != nullptr, "output should be set");
			if (!V)
				return false;

			Core::String Array(V->GetVar("sp-array").GetBlob());
			int64_t Size = V->GetVar("size").GetInteger();
			if (Array.empty() || !Size)
				return false;

			Core::StringStream Stream(Array);
			O->resize((size_t)Size);

			for (auto& It : *O)
			{
				Stream >> It.Angular.Accuracy >> It.Angular.Min >> It.Angular.Max;
				Stream >> It.Rotation.Accuracy >> It.Rotation.Min >> It.Rotation.Max;
				Stream >> It.Scale.Accuracy >> It.Scale.Min >> It.Scale.Max;
				Stream >> It.Diffusion.Accuracy;
				Stream >> It.Diffusion.Min.X >> It.Diffusion.Min.Y >> It.Diffusion.Min.Z >> It.Diffusion.Min.W;
				Stream >> It.Diffusion.Max.X >> It.Diffusion.Max.Y >> It.Diffusion.Max.Z >> It.Diffusion.Max.W;
				Stream >> It.Noise.Accuracy;
				Stream >> It.Noise.Min.X >> It.Noise.Min.Y >> It.Noise.Min.Z;
				Stream >> It.Noise.Max.X >> It.Noise.Max.Y >> It.Noise.Max.Z;
				Stream >> It.Position.Accuracy;
				Stream >> It.Position.Min.X >> It.Position.Min.Y >> It.Position.Min.Z;
				Stream >> It.Position.Max.X >> It.Position.Max.Y >> It.Position.Max.Z;
				Stream >> It.Velocity.Accuracy;
				Stream >> It.Velocity.Min.X >> It.Velocity.Min.Y >> It.Velocity.Min.Z;
				Stream >> It.Velocity.Max.X >> It.Velocity.Max.Y >> It.Velocity.Max.Z;
				Stream >> It.Iterations;
			}

			return true;
		}
		bool HeavySeries::Unpack(Core::Schema* V, Core::Vector<Trigonometry::KeyAnimatorClip>* O)
		{
			VI_ASSERT(O != nullptr, "output should be set");
			if (!V)
				return false;

			Core::Vector<Core::Schema*> Frames = V->FetchCollection("clips.clip", false);
			for (auto&& It : Frames)
			{
				O->push_back(Trigonometry::KeyAnimatorClip());
				HeavySeries::Unpack(It, &O->back());
			}

			return true;
		}
		bool HeavySeries::Unpack(Core::Schema* V, Core::Vector<Trigonometry::AnimatorKey>* O)
		{
			VI_ASSERT(O != nullptr, "output should be set");
			if (!V)
				return false;

			Core::String Array(V->GetVar("ak-array").GetBlob());
			int64_t Size = V->GetVar("size").GetInteger();
			if (Array.empty() || !Size)
				return false;

			Core::StringStream Stream(Array);
			O->resize((size_t)Size);

			for (auto& It : *O)
			{
				Stream >> It.Position.X >> It.Position.Y >> It.Position.Z >> It.Rotation.X >> It.Rotation.Y >> It.Rotation.Z >> It.Rotation.W;
				Stream >> It.Scale.X >> It.Scale.Y >> It.Scale.Z >> It.Time;
			}

			return true;
		}
		bool HeavySeries::Unpack(Core::Schema* V, Core::Vector<Trigonometry::ElementVertex>* O)
		{
			VI_ASSERT(O != nullptr, "output should be set");
			if (!V)
				return false;

			Core::String Array(V->GetVar("ev-array").GetBlob());
			int64_t Size = V->GetVar("size").GetInteger();
			if (Array.empty() || !Size)
				return false;

			Core::StringStream Stream(Array);
			O->resize((size_t)Size);

			for (auto& It : *O)
			{
				Stream >> It.PositionX;
				Stream >> It.PositionY;
				Stream >> It.PositionZ;
				Stream >> It.ColorX;
				Stream >> It.ColorY;
				Stream >> It.ColorZ;
				Stream >> It.ColorW;
				Stream >> It.VelocityX;
				Stream >> It.VelocityY;
				Stream >> It.VelocityZ;
				Stream >> It.Angular;
				Stream >> It.Rotation;
				Stream >> It.Scale;
			}

			return true;
		}
		bool HeavySeries::Unpack(Core::Schema* V, Core::Vector<Trigonometry::Joint>* O)
		{
			VI_ASSERT(O != nullptr, "output should be set");
			if (!V)
				return false;

			O->reserve(V->Size());
			for (auto&& It : V->GetChilds())
			{
				O->push_back(Trigonometry::Joint());
				HeavySeries::Unpack(It, &O->back());
			}

			return true;
		}
		bool HeavySeries::Unpack(Core::Schema* V, Core::Vector<Trigonometry::Vertex>* O)
		{
			if (!V || !O)
				return false;

			Core::String Array(V->GetVar("iv-array").GetBlob());
			int64_t Size = V->GetVar("size").GetInteger();
			if (Array.empty() || !Size)
				return false;

			Core::StringStream Stream(Array);
			O->resize((size_t)Size);

			float Dummy;
			for (auto& It : *O)
			{
				Stream >> It.PositionX;
				Stream >> It.PositionY;
				Stream >> It.PositionZ;
				Stream >> It.TexCoordX;
				Stream >> It.TexCoordY;
				Stream >> It.NormalX;
				Stream >> It.NormalY;
				Stream >> It.NormalZ;
				Stream >> It.TangentX;
				Stream >> It.TangentY;
				Stream >> It.TangentZ;
				Stream >> It.BitangentX;
				Stream >> It.BitangentY;
				Stream >> It.BitangentZ;
				Stream >> Dummy;
				Stream >> Dummy;
				Stream >> Dummy;
				Stream >> Dummy;
				Stream >> Dummy;
				Stream >> Dummy;
				Stream >> Dummy;
				Stream >> Dummy;
			}

			return true;
		}
		bool HeavySeries::Unpack(Core::Schema* V, Core::Vector<Trigonometry::SkinVertex>* O)
		{
			VI_ASSERT(O != nullptr, "output should be set");
			if (!V)
				return false;

			Core::String Array(V->GetVar("iv-array").GetBlob());
			int64_t Size = V->GetVar("size").GetInteger();
			if (Array.empty() || !Size)
				return false;

			Core::StringStream Stream(Array);
			O->resize((size_t)Size);

			for (auto& It : *O)
			{
				Stream >> It.PositionX;
				Stream >> It.PositionY;
				Stream >> It.PositionZ;
				Stream >> It.TexCoordX;
				Stream >> It.TexCoordY;
				Stream >> It.NormalX;
				Stream >> It.NormalY;
				Stream >> It.NormalZ;
				Stream >> It.TangentX;
				Stream >> It.TangentY;
				Stream >> It.TangentZ;
				Stream >> It.BitangentX;
				Stream >> It.BitangentY;
				Stream >> It.BitangentZ;
				Stream >> It.JointIndex0;
				Stream >> It.JointIndex1;
				Stream >> It.JointIndex2;
				Stream >> It.JointIndex3;
				Stream >> It.JointBias0;
				Stream >> It.JointBias1;
				Stream >> It.JointBias2;
				Stream >> It.JointBias3;
			}

			return true;
		}
		bool HeavySeries::Unpack(Core::Schema* V, Core::Vector<Ticker>* O)
		{
			VI_ASSERT(O != nullptr, "output should be set");
			if (!V)
				return false;

			Core::String Array(V->GetVar("tt-array").GetBlob());
			int64_t Size = V->GetVar("size").GetInteger();
			if (Array.empty() || !Size)
				return false;

			Core::StringStream Stream(Array);
			O->resize((size_t)Size);

			for (auto& It : *O)
				Stream >> It.Delay;

			return true;
		}

		void PoseBuffer::Fill(SkinModel* Model)
		{
			VI_ASSERT(Model != nullptr, "model should be set");
			Offsets.clear();
			Matrices.clear();

			Fill(Model->Skeleton);
			for (auto& Mesh : Model->Meshes)
				Matrices.insert(std::make_pair(Mesh, PoseMatrices()));
		}
		void PoseBuffer::Fill(Trigonometry::Joint& Next)
		{
			auto& Data = Offsets[Next.Index];
			Data.Default.Position = Next.Global.Position();
			Data.Default.Rotation = Next.Global.RotationQuaternion();
			Data.Default.Scale = Next.Global.Scale();
			Data.Offset = Data.Frame = Data.Default;

			for (auto& Child : Next.Childs)
				Fill(Child);
		}

		Model::Model() noexcept
		{
		}
		Model::~Model() noexcept
		{
			Cleanup();
		}
		void Model::Cleanup()
		{
			for (auto* Item : Meshes)
				Core::Memory::Release(Item);
			Meshes.clear();
		}
		Graphics::MeshBuffer* Model::FindMesh(const std::string_view& Name)
		{
			for (auto&& It : Meshes)
			{
				if (It->Name == Name)
					return It;
			}

			return nullptr;
		}

		SkinModel::SkinModel() noexcept
		{
		}
		SkinModel::~SkinModel() noexcept
		{
			Cleanup();
		}
		bool SkinModel::FindJoint(const std::string_view& Name, Trigonometry::Joint* Base)
		{
			if (!Base)
				Base = &Skeleton;

			if (Base->Name == Name)
				return Base;

			for (auto&& Child : Base->Childs)
			{
				if (Child.Name == Name)
				{
					Base = &Child;
					return true;
				}

				Trigonometry::Joint* Result = &Child;
				if (FindJoint(Name, Result))
					return true;
			}

			return false;
		}
		bool SkinModel::FindJoint(size_t Index, Trigonometry::Joint* Base)
		{
			if (!Base)
				Base = &Skeleton;

			if (Base->Index == Index)
				return true;

			for (auto&& Child : Base->Childs)
			{
				if (Child.Index == Index)
				{
					Base = &Child;
					return true;
				}

				Trigonometry::Joint* Result = &Child;
				if (FindJoint(Index, Result))
					return true;
			}

			return false;
		}
		void SkinModel::Synchronize(PoseBuffer* Map)
		{
			VI_ASSERT(Map != nullptr, "pose buffer should be set");
			VI_MEASURE(Core::Timings::Atomic);

			for (auto& Mesh : Meshes)
				Map->Matrices[Mesh];

			Synchronize(Map, Skeleton, Transform);
		}
		void SkinModel::Synchronize(PoseBuffer* Map, Trigonometry::Joint& Next, const Trigonometry::Matrix4x4& ParentOffset)
		{
			auto& Node = Map->Offsets[Next.Index].Offset;
			auto LocalOffset = Trigonometry::Matrix4x4::CreateScale(Node.Scale) * Node.Rotation.GetMatrix() * Trigonometry::Matrix4x4::CreateTranslation(Node.Position);
			auto GlobalOffset = LocalOffset * ParentOffset;
			auto FinalOffset = Next.Local * GlobalOffset * InvTransform;

			for (auto& Matrices : Map->Matrices)
			{
				auto Index = Matrices.first->Joints.find(Next.Index);
				if (Index != Matrices.first->Joints.end() && Index->second <= Graphics::JOINTS_SIZE)
					Matrices.second.Data[Index->second] = FinalOffset;
			}

			for (auto& Child : Next.Childs)
				Synchronize(Map, Child, GlobalOffset);
		}
		void SkinModel::Cleanup()
		{
			for (auto* Item : Meshes)
				Core::Memory::Release(Item);
			Meshes.clear();
		}
		Graphics::SkinMeshBuffer* SkinModel::FindMesh(const std::string_view& Name)
		{
			for (auto&& It : Meshes)
			{
				if (It->Name == Name)
					return It;
			}

			return nullptr;
		}

		SkinAnimation::SkinAnimation(Core::Vector<Trigonometry::SkinAnimatorClip>&& Data) noexcept : Clips(std::move(Data))
		{
		}
		const Core::Vector<Trigonometry::SkinAnimatorClip>& SkinAnimation::GetClips()
		{
			return Clips;
		}
		bool SkinAnimation::IsValid()
		{
			return !Clips.empty();
		}

		Material::Material(SceneGraph* NewScene) noexcept : DiffuseMap(nullptr), NormalMap(nullptr), MetallicMap(nullptr), RoughnessMap(nullptr), HeightMap(nullptr), OcclusionMap(nullptr), EmissionMap(nullptr), Scene(NewScene), Slot(0)
		{
		}
		Material::Material(const Material& Other) noexcept : Material(Other.Scene)
		{
			memcpy(&Surface, &Other.Surface, sizeof(Subsurface));
			if (Other.DiffuseMap != nullptr)
			{
				DiffuseMap = Other.DiffuseMap;
				DiffuseMap->AddRef();
			}

			if (Other.NormalMap != nullptr)
			{
				NormalMap = Other.NormalMap;
				NormalMap->AddRef();
			}

			if (Other.MetallicMap != nullptr)
			{
				MetallicMap = Other.MetallicMap;
				MetallicMap->AddRef();
			}

			if (Other.RoughnessMap != nullptr)
			{
				RoughnessMap = Other.RoughnessMap;
				RoughnessMap->AddRef();
			}

			if (Other.HeightMap != nullptr)
			{
				HeightMap = Other.HeightMap;
				HeightMap->AddRef();
			}

			if (Other.OcclusionMap != nullptr)
			{
				OcclusionMap = Other.OcclusionMap;
				OcclusionMap->AddRef();
			}

			if (Other.EmissionMap != nullptr)
			{
				EmissionMap = Other.EmissionMap;
				EmissionMap->AddRef();
			}
		}
		Material::~Material() noexcept
		{
			Core::Memory::Release(DiffuseMap);
			Core::Memory::Release(NormalMap);
			Core::Memory::Release(MetallicMap);
			Core::Memory::Release(RoughnessMap);
			Core::Memory::Release(HeightMap);
			Core::Memory::Release(OcclusionMap);
			Core::Memory::Release(EmissionMap);
		}
		void Material::SetName(const std::string_view& Value)
		{
			Name = Value;
			if (Scene != nullptr)
				Scene->Mutate(this, "set");
		}
		const Core::String& Material::GetName() const
		{
			return Name;
		}
		void Material::SetDiffuseMap(Graphics::Texture2D* New)
		{
			VI_TRACE("[layer] material %s apply diffuse 0x%" PRIXPTR, Name.c_str(), (void*)New);
			Core::Memory::Release(DiffuseMap);
			DiffuseMap = New;
			if (DiffuseMap != nullptr)
				DiffuseMap->AddRef();
		}
		Graphics::Texture2D* Material::GetDiffuseMap() const
		{
			return DiffuseMap;
		}
		void Material::SetNormalMap(Graphics::Texture2D* New)
		{
			VI_TRACE("[layer] material %s apply normal 0x%" PRIXPTR, Name.c_str(), (void*)New);
			Core::Memory::Release(NormalMap);
			NormalMap = New;
			if (NormalMap != nullptr)
				NormalMap->AddRef();
		}
		Graphics::Texture2D* Material::GetNormalMap() const
		{
			return NormalMap;
		}
		void Material::SetMetallicMap(Graphics::Texture2D* New)
		{
			VI_TRACE("[layer] material %s apply metallic 0x%" PRIXPTR, Name.c_str(), (void*)New);
			Core::Memory::Release(MetallicMap);
			MetallicMap = New;
			if (MetallicMap != nullptr)
				MetallicMap->AddRef();
		}
		Graphics::Texture2D* Material::GetMetallicMap() const
		{
			return MetallicMap;
		}
		void Material::SetRoughnessMap(Graphics::Texture2D* New)
		{
			VI_TRACE("[layer] material %s apply roughness 0x%" PRIXPTR, Name.c_str(), (void*)New);
			Core::Memory::Release(RoughnessMap);
			RoughnessMap = New;
			if (RoughnessMap != nullptr)
				RoughnessMap->AddRef();
		}
		Graphics::Texture2D* Material::GetRoughnessMap() const
		{
			return RoughnessMap;
		}
		void Material::SetHeightMap(Graphics::Texture2D* New)
		{
			VI_TRACE("[layer] material %s apply height 0x%" PRIXPTR, Name.c_str(), (void*)New);
			Core::Memory::Release(HeightMap);
			HeightMap = New;
			if (HeightMap != nullptr)
				HeightMap->AddRef();
		}
		Graphics::Texture2D* Material::GetHeightMap() const
		{
			return HeightMap;
		}
		void Material::SetOcclusionMap(Graphics::Texture2D* New)
		{
			VI_TRACE("[layer] material %s apply occlusion 0x%" PRIXPTR, Name.c_str(), (void*)New);
			Core::Memory::Release(OcclusionMap);
			OcclusionMap = New;
			if (OcclusionMap != nullptr)
				OcclusionMap->AddRef();
		}
		Graphics::Texture2D* Material::GetOcclusionMap() const
		{
			return OcclusionMap;
		}
		void Material::SetEmissionMap(Graphics::Texture2D* New)
		{
			VI_TRACE("[layer] material %s apply emission 0x%" PRIXPTR, Name.c_str(), (void*)New);
			Core::Memory::Release(EmissionMap);
			EmissionMap = New;
			if (EmissionMap != nullptr)
				EmissionMap->AddRef();
		}
		Graphics::Texture2D* Material::GetEmissionMap() const
		{
			return EmissionMap;
		}
		SceneGraph* Material::GetScene() const
		{
			return Scene;
		}

		Component::Component(Entity* Reference, ActorSet Rule) noexcept : Parent(Reference), Set((size_t)Rule), Indexed(false), Active(true)
		{
			VI_ASSERT(Reference != nullptr, "entity should be set");
		}
		Component::~Component() noexcept
		{
		}
		void Component::Deserialize(Core::Schema* Node)
		{
		}
		void Component::Serialize(Core::Schema* Node)
		{
		}
		void Component::Activate(Component* New)
		{
		}
		void Component::Deactivate()
		{
		}
		void Component::Animate(Core::Timer* Time)
		{
		}
		void Component::Synchronize(Core::Timer* Time)
		{
		}
		void Component::Update(Core::Timer* Time)
		{
		}
		void Component::Message(const std::string_view& Name, Core::VariantArgs& Args)
		{
		}
		void Component::Movement()
		{
		}
		float Component::GetVisibility(const Viewer& View, float Distance) const
		{
			float Visibility = 1.0f - Distance / View.FarPlane;
			if (Visibility <= 0.0f)
				return 0.0f;

			const Trigonometry::Matrix4x4& Box = Parent->GetBox();
			return Trigonometry::Geometric::IsCubeInFrustum(Box * View.ViewProjection, 1.65f) ? Visibility : 0.0f;
		}
		size_t Component::GetUnitBounds(Trigonometry::Vector3& Min, Trigonometry::Vector3& Max) const
		{
			Min = -1.0f;
			Max = 1.0f;
			return 0;
		}
		void Component::SetActive(bool Status)
		{
			auto* Scene = Parent->GetScene();
			if (Active == Status)
				return;

			Active = Status;
			if (Parent->IsActive())
			{
				if (Active)
					Scene->RegisterComponent(this, false);
				else
					Scene->UnregisterComponent(this);
			}

			Scene->NotifyCosmos(this);
		}
		bool Component::IsDrawable() const
		{
			return Set & (uint64_t)ActorSet::Drawable;
		}
		bool Component::IsCullable() const
		{
			return Set & (uint64_t)ActorSet::Cullable;
		}
		bool Component::IsActive() const
		{
			return Active;
		}
		Entity* Component::GetEntity() const
		{
			return Parent;
		}

		Entity::Entity(SceneGraph* NewScene) noexcept : Transform(new Trigonometry::Transform(this)), Scene(NewScene), Active(false)
		{
			VI_ASSERT(Scene != nullptr, "entity should be created within a scene");
		}
		Entity::~Entity() noexcept
		{
			for (auto& Component : Type.Components)
			{
				if (Component.second != nullptr)
				{
					Component.second->SetActive(false);
					Scene->UnloadComponentAll(Component.second);
					Scene->ClearCosmos(Component.second);
					Core::Memory::Release(Component.second);
				}
			}

			Core::Memory::Release(Transform);
		}
		void Entity::SetName(const std::string_view& Value)
		{
			Type.Name = Value;
			Scene->Mutate(this, "set");
		}
		void Entity::SetRoot(Entity* Parent)
		{
			auto* Old = Transform->GetRoot();
			if (!Parent)
			{
				Transform->SetRoot(nullptr);
				if (Old != nullptr && Scene != nullptr)
					Scene->Mutate((Entity*)Old->UserData, this, "pop");
			}
			else
			{
				Transform->SetRoot(Parent->Transform);
				if (Old != Parent->Transform && Scene != nullptr)
					Scene->Mutate(Parent, this, "push");
			}
		}
		void Entity::UpdateBounds()
		{
			size_t Index = 0;
			for (auto& Item : Type.Components)
			{
				Trigonometry::Vector3 Min, Max;
				size_t Offset = Item.second->GetUnitBounds(Min, Max);
				Item.second->Movement();

				if (Offset > Index)
				{
					Index = Offset;
					Snapshot.Min = Min;
					Snapshot.Max = Max;
				}
			}

			if (!Index)
			{
				Snapshot.Min = -1.0f;
				Snapshot.Max = 1.0f;
			}

			Transform->GetBounds(Snapshot.Box, Snapshot.Min, Snapshot.Max);
		}
		void Entity::RemoveComponent(uint64_t Id)
		{
			Core::UnorderedMap<uint64_t, Component*>::iterator It = Type.Components.find(Id);
			if (It == Type.Components.end())
				return;

			Component* Base = It->second;
			Base->SetActive(false);
			Transform->MakeDirty();
			Type.Components.erase(It);
			if (Scene->Camera == Base)
				Scene->Camera = nullptr;

			auto* Top = Scene;
			Scene->Transaction([Top, Base]()
			{
				Top->ClearCosmos(Base);
				Base->Release();
			});
		}
		void Entity::RemoveChilds()
		{
			Core::Vector<Trigonometry::Transform*>& Childs = Transform->GetChilds();
			for (size_t i = 0; i < Childs.size(); i++)
			{
				Entity* Next = (Entity*)Transform->GetChild(i)->UserData;
				if (Next != nullptr && Next != this)
				{
					Scene->DeleteEntity(Next);
					i--;
				}
			}
		}
		Component* Entity::AddComponent(Component* In)
		{
			VI_ASSERT(In != nullptr, "component should be set");
			if (In == GetComponent(In->GetId()))
				return In;

			RemoveComponent(In->GetId());
			In->Active = false;
			In->Parent = this;

			Type.Components.insert({ In->GetId(), In });
			Scene->Transaction([this, In]()
			{
				for (auto& Component : Type.Components)
					Component.second->Activate(In == Component.second ? nullptr : In);
			});

			In->SetActive(true);
			Transform->MakeDirty();

			return In;
		}
		Component* Entity::GetComponent(uint64_t Id)
		{
			Core::UnorderedMap<uint64_t, Component*>::iterator It = Type.Components.find(Id);
			if (It != Type.Components.end())
				return It->second;

			return nullptr;
		}
		size_t Entity::GetComponentsCount() const
		{
			return Type.Components.size();
		}
		SceneGraph* Entity::GetScene() const
		{
			return Scene;
		}
		Entity* Entity::GetParent() const
		{
			auto* Root = Transform->GetRoot();
			if (!Root)
				return nullptr;

			return (Entity*)Root->UserData;
		}
		Entity* Entity::GetChild(size_t Index) const
		{
			auto* Child = Transform->GetChild(Index);
			if (!Child)
				return nullptr;

			return (Entity*)Child->UserData;
		}
		Trigonometry::Transform* Entity::GetTransform() const
		{
			return Transform;
		}
		const Core::String& Entity::GetName() const
		{
			return Type.Name;
		}
		float Entity::GetVisibility(const Viewer& Base) const
		{
			float Distance = Transform->GetPosition().Distance(Base.Position);
			return 1.0f - Distance / Base.FarPlane;
		}
		const Trigonometry::Matrix4x4& Entity::GetBox() const
		{
			return Snapshot.Box;
		}
		const Trigonometry::Vector3& Entity::GetMin() const
		{
			return Snapshot.Min;
		}
		const Trigonometry::Vector3& Entity::GetMax() const
		{
			return Snapshot.Max;
		}
		size_t Entity::GetChildsCount() const
		{
			return Transform->GetChildsCount();
		}
		bool Entity::IsActive() const
		{
			return Active;
		}
		Trigonometry::Vector3 Entity::GetRadius3() const
		{
			Trigonometry::Vector3 Diameter3 = Snapshot.Max - Snapshot.Min;
			return Diameter3.Abs().Mul(0.5f);
		}
		float Entity::GetRadius() const
		{
			const Trigonometry::Vector3& Radius = GetRadius3();
			float Max = (Radius.X > Radius.Y ? Radius.X : Radius.Y);
			return (Max > Radius.Z ? Radius.Z : Max);
		}

		Drawable::Drawable(Entity* Ref, ActorSet Rule, uint64_t Hash) noexcept : Component(Ref, Rule | ActorSet::Cullable | ActorSet::Drawable | ActorSet::Message), Category(GeoCategory::Opaque), Source(Hash), Overlapping(1.0f), Static(true)
		{
		}
		Drawable::~Drawable() noexcept
		{
		}
		void Drawable::Message(const std::string_view& Name, Core::VariantArgs& Args)
		{
			if (Name != "mutation")
				return;

			Material* Target = (Material*)Args["material"].GetPointer();
			if (!Target || !Args["type"].IsString("push"))
				return;

			for (auto&& Surface : Materials)
			{
				if (Surface.second == Target)
					Surface.second = nullptr;
			}
		}
		void Drawable::Movement()
		{
			Overlapping = 1.0f;
		}
		void Drawable::ClearMaterials()
		{
			Materials.clear();
		}
		bool Drawable::SetCategory(GeoCategory NewCategory)
		{
			Category = NewCategory;
			return true;
		}
		bool Drawable::SetMaterial(void* Instance, Material* Value)
		{
			auto It = Materials.find(Instance);
			if (It == Materials.end())
				Materials[Instance] = Value;
			else
				It->second = Value;

			return true;
		}
		bool Drawable::SetMaterial(Material* Value)
		{
			if (Materials.empty())
				return SetMaterial(nullptr, Value);

			for (auto& Item : Materials)
				Item.second = Value;

			return true;
		}
		GeoCategory Drawable::GetCategory() const
		{
			return Category;
		}
		int64_t Drawable::GetSlot(void* Surface)
		{
			Material* Base = GetMaterial(Surface);
			return Base ? (int64_t)Base->Slot : -1;
		}
		int64_t Drawable::GetSlot()
		{
			Material* Base = GetMaterial();
			return Base ? (int64_t)Base->Slot : -1;
		}
		Material* Drawable::GetMaterial(void* Instance)
		{
			if (Materials.size() == 1)
				return Materials.begin()->second;

			auto It = Materials.find(Instance);
			if (It == Materials.end())
				return nullptr;

			return It->second;
		}
		Material* Drawable::GetMaterial()
		{
			if (Materials.empty())
				return nullptr;

			return Materials.begin()->second;
		}
		const Core::UnorderedMap<void*, Material*>& Drawable::GetMaterials()
		{
			return Materials;
		}

		Renderer::Renderer(RenderSystem* Lab) noexcept : System(Lab), Active(true)
		{
			VI_ASSERT(Lab != nullptr, "render system should be set");
		}
		Renderer::~Renderer() noexcept
		{
		}
		void Renderer::SetRenderer(RenderSystem* NewSystem)
		{
			VI_ASSERT(NewSystem != nullptr, "render system should be set");
			System = NewSystem;
		}
		void Renderer::Deserialize(Core::Schema* Node)
		{
		}
		void Renderer::Serialize(Core::Schema* Node)
		{
		}
		void Renderer::ClearCulling()
		{
		}
		void Renderer::ResizeBuffers()
		{
		}
		void Renderer::Activate()
		{
		}
		void Renderer::Deactivate()
		{
		}
		void Renderer::BeginPass(Core::Timer* Time)
		{
		}
		void Renderer::EndPass()
		{
		}
		bool Renderer::HasCategory(GeoCategory Category)
		{
			return false;
		}
		size_t Renderer::RenderPrepass(Core::Timer* Time)
		{
			return 0;
		}
		size_t Renderer::RenderPass(Core::Timer* TimeStep)
		{
			return 0;
		}
		RenderSystem* Renderer::GetRenderer() const
		{
			return System;
		}

		RenderConstants::RenderConstants(Graphics::GraphicsDevice* NewDevice) noexcept : Device(NewDevice)
		{
			VI_ASSERT(Device != nullptr, "graphics device should be set");
			Graphics::Shader::Desc F = Graphics::Shader::Desc();
			if (Device->GetSectionData("materials/material_basic_geometry", &F))
				Binding.BasicEffect = *Device->CreateShader(F);

			Graphics::ElementBuffer::Desc Desc;
			Desc.BindFlags = Graphics::ResourceBind::Constant_Buffer;
			Desc.ElementCount = 1;
			Desc.ElementWidth = sizeof(Animation);
			Binding.Buffers[(size_t)RenderBufferType::Animation] = *Device->CreateElementBuffer(Desc);
			Binding.Pointers[(size_t)RenderBufferType::Animation] = &Animation;
			Binding.Sizes[(size_t)RenderBufferType::Animation] = sizeof(Animation);

			Desc.ElementWidth = sizeof(Render);
			Binding.Buffers[(size_t)RenderBufferType::Render] = *Device->CreateElementBuffer(Desc);
			Binding.Pointers[(size_t)RenderBufferType::Render] = &Render;
			Binding.Sizes[(size_t)RenderBufferType::Render] = sizeof(Render);

			Desc.ElementWidth = sizeof(View);
			Binding.Buffers[(size_t)RenderBufferType::View] = *Device->CreateElementBuffer(Desc);
			Binding.Pointers[(size_t)RenderBufferType::View] = &View;
			Binding.Sizes[(size_t)RenderBufferType::View] = sizeof(View);

			SetConstantBuffers();
		}
		RenderConstants::~RenderConstants() noexcept
		{
			for (size_t i = 0; i < 3; i++)
				Core::Memory::Release(Binding.Buffers[i]);
			Core::Memory::Release(Binding.BasicEffect);
		}
		void RenderConstants::SetConstantBuffers()
		{
			for (size_t i = 0; i < 3; i++)
				Device->SetConstantBuffer(Binding.Buffers[i], (uint32_t)i, VI_VS | VI_PS | VI_GS | VI_CS | VI_HS | VI_DS);
		}
		void RenderConstants::UpdateConstantBuffer(RenderBufferType Buffer)
		{
			Device->UpdateConstantBuffer(Binding.Buffers[(size_t)Buffer], Binding.Pointers[(size_t)Buffer], Binding.Sizes[(size_t)Buffer]);
		}
		Graphics::Shader* RenderConstants::GetBasicEffect() const
		{
			return Binding.BasicEffect;
		}
		Graphics::GraphicsDevice* RenderConstants::GetDevice() const
		{
			return Device;
		}
		Graphics::ElementBuffer* RenderConstants::GetConstantBuffer(RenderBufferType Buffer) const
		{
			return Binding.Buffers[(size_t)Buffer];
		}

		RenderSystem::RenderSystem(SceneGraph* NewScene, Component* NewComponent) noexcept : Device(nullptr), BaseMaterial(nullptr), Scene(NewScene), Owner(NewComponent), MaxQueries(16384), SortingFrequency(2), OcclusionSkips(2), OccluderSkips(8), OccludeeSkips(3), OccludeeScaling(1.0f), OverflowVisibility(0.0f), Threshold(0.1f), OcclusionCulling(false), PreciseCulling(true), AllowInputLag(false)
		{
			VI_ASSERT(NewScene != nullptr, "scene should be set");
			VI_ASSERT(NewScene->GetDevice() != nullptr, "graphics device should be set");
			VI_ASSERT(NewScene->GetConstants() != nullptr, "render constants should be set");
			Device = NewScene->GetDevice();
			Constants = NewScene->GetConstants();
		}
		RenderSystem::~RenderSystem() noexcept
		{
			RemoveRenderers();
		}
		void RenderSystem::SetView(const Trigonometry::Matrix4x4& _View, const Trigonometry::Matrix4x4& _Projection, const Trigonometry::Vector3& _Position, float _Fov, float _Ratio, float _Near, float _Far, RenderCulling _Type)
		{
			View.Set(_View, _Projection, _Position, _Fov, _Ratio, _Near, _Far, _Type);
			RestoreViewBuffer(&View);
		}
		void RenderSystem::ClearCulling()
		{
			for (auto& Next : Renderers)
				Next->ClearCulling();
			Scene->ClearCulling();
		}
		void RenderSystem::RemoveRenderers()
		{
			for (auto& Next : Renderers)
			{
				Next->Deactivate();
				Core::Memory::Release(Next);
			}
			Renderers.clear();
		}
		void RenderSystem::RestoreViewBuffer(Viewer* Buffer)
		{
			VI_ASSERT(Device != nullptr, "graphics device should be set");
			if (&View != Buffer)
			{
				if (Buffer == nullptr)
				{
					auto* Viewer = (Components::Camera*)Scene->Camera.load();
					if (Viewer != nullptr)
						Viewer->GetViewer(&View);
				}
				else
					View = *Buffer;
			}

			if (View.Culling == RenderCulling::Linear)
				Indexing.Frustum = Trigonometry::Frustum6P(View.ViewProjection);
			else if (View.Culling == RenderCulling::Cubic)
				Indexing.Bounds = Trigonometry::Bounding(View.Position - View.FarPlane, View.Position + View.FarPlane);

			Constants->View.InvViewProj = View.InvViewProjection;
			Constants->View.ViewProj = View.ViewProjection;
			Constants->View.Proj = View.Projection;
			Constants->View.View = View.View;
			Constants->View.Position = View.Position;
			Constants->View.Direction = View.Rotation.dDirection();
			Constants->View.Far = View.FarPlane;
			Constants->View.Near = View.NearPlane;
			Constants->UpdateConstantBuffer(RenderBufferType::View);
		}
		void RenderSystem::Remount(Renderer* Target)
		{
			VI_ASSERT(Target != nullptr, "renderer should be set");
			Target->Deactivate();
			Target->SetRenderer(this);
			Target->Activate();
			Target->ResizeBuffers();
		}
		void RenderSystem::Remount()
		{
			ClearCulling();
			for (auto& Next : Renderers)
				Remount(Next);
		}
		void RenderSystem::Mount()
		{
			ClearCulling();
			for (auto& Next : Renderers)
				Next->Activate();
		}
		void RenderSystem::Unmount()
		{
			for (auto& Next : Renderers)
				Next->Deactivate();
		}
		void RenderSystem::MoveRenderer(uint64_t Id, size_t Offset)
		{
			if (Offset == 0)
				return;

			for (size_t i = 0; i < Renderers.size(); i++)
			{
				if (Renderers[i]->GetId() != Id)
					continue;

				if (i + Offset < 0 || i + Offset >= Renderers.size())
					return;

				Renderer* Swap = Renderers[i + Offset];
				Renderers[i + Offset] = Renderers[i];
				Renderers[i] = Swap;
				return;
			}
		}
		void RenderSystem::RemoveRenderer(uint64_t Id)
		{
			for (auto It = Renderers.begin(); It != Renderers.end(); ++It)
			{
				if (*It && (*It)->GetId() == Id)
				{
					(*It)->Deactivate();
					Core::Memory::Release(*It);
					Renderers.erase(It);
					break;
				}
			}
		}
		void RenderSystem::RestoreOutput()
		{
			Scene->SetMRT(TargetType::Main, false);
		}
		void RenderSystem::FreeShader(const std::string_view& Name, Graphics::Shader* Shader)
		{
			ShaderCache* Cache = Scene->GetShaders();
			if (Cache != nullptr)
			{
				if (Cache->Has(Name))
					return;
			}

			Core::Memory::Release(Shader);
		}
		void RenderSystem::FreeShader(Graphics::Shader* Shader)
		{
			ShaderCache* Cache = Scene->GetShaders();
			if (Cache != nullptr)
				return FreeShader(Cache->Find(Shader), Shader);

			Core::Memory::Release(Shader);
		}
		void RenderSystem::FreeBuffers(const std::string_view& Name, Graphics::ElementBuffer** Buffers)
		{
			if (!Buffers)
				return;

			PrimitiveCache* Cache = Scene->GetPrimitives();
			if (Cache != nullptr)
			{
				if (Cache->Has(Name))
					return;
			}

			Core::Memory::Release(Buffers[0]);
			Core::Memory::Release(Buffers[1]);
		}
		void RenderSystem::FreeBuffers(Graphics::ElementBuffer** Buffers)
		{
			if (!Buffers)
				return;

			PrimitiveCache* Cache = Scene->GetPrimitives();
			if (Cache != nullptr)
				return FreeBuffers(Cache->Find(Buffers), Buffers);

			Core::Memory::Release(Buffers[0]);
			Core::Memory::Release(Buffers[1]);
		}
		void RenderSystem::UpdateConstantBuffer(RenderBufferType Buffer)
		{
			Constants->UpdateConstantBuffer(Buffer);
		}
		Graphics::Shader* RenderSystem::GetBasicEffect() const
		{
			return Constants->GetBasicEffect();
		}
		void RenderSystem::ClearMaterials()
		{
			BaseMaterial = nullptr;
		}
		void RenderSystem::FetchVisibility(Component* Base, VisibilityQuery& Data)
		{
			auto& Snapshot = Base->Parent->Snapshot;
			Snapshot.Distance = Base->Parent->Transform->GetPosition().Distance(View.Position);
			Snapshot.Visibility = std::max<float>(0.0f, 1.0f - Snapshot.Distance / (View.FarPlane + Base->Parent->GetRadius()));
			if (OcclusionCulling && Snapshot.Visibility >= Threshold && State.IsTop() && Base->IsDrawable())
			{
				auto* Varying = (Drawable*)Base;
				Snapshot.Visibility = Varying->Overlapping;
				Data.Category = Varying->GetCategory();
				Data.QueryPixels = (Data.Category == GeoCategory::Opaque);
			}
			else
			{
				Data.Category = GeoCategory::Opaque;
				Data.QueryPixels = false;
			}
			Data.BoundaryVisible = Snapshot.Visibility >= Threshold;
		}
		size_t RenderSystem::Render(Core::Timer* Time, RenderState Stage, RenderOpt Options)
		{
			VI_ASSERT(Time != nullptr, "timer should be set");

			RenderOpt LastOptions = State.Options;
			RenderState LastTarget = State.Target;
			size_t Count = 0;

			State.Top++;
			State.Options = Options;
			State.Target = Stage;

			for (auto& Next : Renderers)
			{
				if (Next->Active)
					Next->BeginPass(Time);
			}

			for (auto& Next : Renderers)
			{
				if (Next->Active)
					Count += Next->RenderPrepass(Time);
			}

			for (auto& Next : Renderers)
			{
				if (Next->Active)
					Count += Next->RenderPass(Time);
			}

			for (auto& Next : Renderers)
			{
				if (Next->Active)
					Next->EndPass();
			}

			State.Target = LastTarget;
			State.Options = LastOptions;
			State.Top--;

			return Count;
		}
		bool RenderSystem::TryInstance(Material* Next, RenderBuffer::Instance& Target)
		{
			if (!Next)
				return false;

			Target.Diffuse = (float)(Next->DiffuseMap != nullptr);
			Target.Normal = (float)(Next->NormalMap != nullptr);
			Target.Height = (float)(Next->HeightMap != nullptr);
			Target.MaterialId = (float)Next->Slot;

			return true;
		}
		bool RenderSystem::TryGeometry(Material* Next, bool WithTextures)
		{
			if (!Next)
				return false;

			if (Next == BaseMaterial)
				return true;

			BaseMaterial = Next;
			Constants->Render.Diffuse = (float)(Next->DiffuseMap != nullptr);
			Constants->Render.Normal = (float)(Next->NormalMap != nullptr);
			Constants->Render.Height = (float)(Next->HeightMap != nullptr);
			Constants->Render.MaterialId = (float)Next->Slot;

			if (WithTextures)
			{
				if (Next->DiffuseMap != nullptr)
					Device->SetTexture2D(Next->DiffuseMap, 1, VI_PS);

				if (Next->NormalMap != nullptr)
					Device->SetTexture2D(Next->NormalMap, 2, VI_PS);

				if (Next->MetallicMap != nullptr)
					Device->SetTexture2D(Next->MetallicMap, 3, VI_PS);

				if (Next->RoughnessMap != nullptr)
					Device->SetTexture2D(Next->RoughnessMap, 4, VI_PS);

				if (Next->HeightMap != nullptr)
					Device->SetTexture2D(Next->HeightMap, 5, VI_PS);

				if (Next->OcclusionMap != nullptr)
					Device->SetTexture2D(Next->OcclusionMap, 6, VI_PS);

				if (Next->EmissionMap != nullptr)
					Device->SetTexture2D(Next->EmissionMap, 7, VI_PS);
			}

			return true;
		}
		bool RenderSystem::HasCategory(GeoCategory Category)
		{
			for (auto* Next : Renderers)
			{
				if (Next->Active && Next->HasCategory(Category))
					return true;
			}

			return false;
		}
		Graphics::ExpectsGraphics<void> RenderSystem::CompileBuffers(Graphics::ElementBuffer** Result, const std::string_view& Name, size_t ElementSize, size_t ElementsCount)
		{
			VI_ASSERT(Result != nullptr, "result should be set");
			VI_ASSERT(!Name.empty(), "buffers must have a name");

			PrimitiveCache* Cache = Scene->GetPrimitives();
			if (Cache != nullptr)
				return Cache->Compile(Result, Name, ElementSize, ElementsCount);

			Graphics::ElementBuffer::Desc F = Graphics::ElementBuffer::Desc();
			F.AccessFlags = Graphics::CPUAccess::Write;
			F.Usage = Graphics::ResourceUsage::Dynamic;
			F.BindFlags = Graphics::ResourceBind::Vertex_Buffer;
			F.ElementWidth = (uint32_t)ElementSize;
			F.ElementCount = (uint32_t)ElementsCount;

			auto VertexBuffer = Device->CreateElementBuffer(F);
			if (!VertexBuffer)
				return VertexBuffer.Error();

			F = Graphics::ElementBuffer::Desc();
			F.AccessFlags = Graphics::CPUAccess::Write;
			F.Usage = Graphics::ResourceUsage::Dynamic;
			F.BindFlags = Graphics::ResourceBind::Index_Buffer;
			F.ElementWidth = (uint32_t)sizeof(int);
			F.ElementCount = (uint32_t)ElementsCount * 3;

			auto IndexBuffer = Device->CreateElementBuffer(F);
			if (!IndexBuffer)
			{
				Core::Memory::Release(*VertexBuffer);
				return IndexBuffer.Error();
			}

			Result[(size_t)BufferType::Index] = *IndexBuffer;
			Result[(size_t)BufferType::Vertex] = *VertexBuffer;
			return Core::Expectation::Met;
		}
		Graphics::ExpectsGraphics<Graphics::Shader*> RenderSystem::CompileShader(Graphics::Shader::Desc& Desc, size_t BufferSize)
		{
			VI_ASSERT(!Desc.Filename.empty(), "shader must have a name");
			ShaderCache* Cache = Scene->GetShaders();
			if (Cache != nullptr)
				return Cache->Compile(Desc.Filename, Desc, BufferSize);

			auto Shader = Device->CreateShader(Desc);
			if (!Shader)
				return Shader;
			else if (BufferSize > 0)
				Device->UpdateBufferSize(*Shader, BufferSize);

			return Shader;
		}
		Graphics::ExpectsGraphics<Graphics::Shader*> RenderSystem::CompileShader(const std::string_view& SectionName, size_t BufferSize)
		{
			Graphics::Shader::Desc I = Graphics::Shader::Desc();
			if (!Device->GetSectionData(SectionName, &I))
				return nullptr;

			return CompileShader(I, BufferSize);
		}
		Renderer* RenderSystem::AddRenderer(Renderer* In)
		{
			VI_ASSERT(In != nullptr, "renderer should be set");
			for (auto It = Renderers.begin(); It != Renderers.end(); ++It)
			{
				if (*It && (*It)->GetId() == In->GetId())
				{
					if (*It == In)
						return In;

					(*It)->Deactivate();
					Core::Memory::Release(*It);
					Renderers.erase(It);
					break;
				}
			}

			In->SetRenderer(this);
			In->Activate();
			In->ResizeBuffers();
			Renderers.push_back(In);

			return In;
		}
		Renderer* RenderSystem::GetRenderer(uint64_t Id)
		{
			for (auto& Next : Renderers)
			{
				if (Next->GetId() == Id)
					return Next;
			}

			return nullptr;
		}
		bool RenderSystem::GetOffset(uint64_t Id, size_t& Offset) const
		{
			for (size_t i = 0; i < Renderers.size(); i++)
			{
				if (Renderers[i]->GetId() == Id)
				{
					Offset = i;
					return true;
				}
			}

			return false;
		}
		Core::Vector<Renderer*>& RenderSystem::GetRenderers()
		{
			return Renderers;
		}
		Graphics::MultiRenderTarget2D* RenderSystem::GetMRT(TargetType Type) const
		{
			return Scene->GetMRT(Type);
		}
		Graphics::RenderTarget2D* RenderSystem::GetRT(TargetType Type) const
		{
			return Scene->GetRT(Type);
		}
		Graphics::GraphicsDevice* RenderSystem::GetDevice() const
		{
			return Device;
		}
		Graphics::Texture2D** RenderSystem::GetMerger()
		{
			return Scene->GetMerger();
		}
		RenderConstants* RenderSystem::GetConstants() const
		{
			return Constants;
		}
		PrimitiveCache* RenderSystem::GetPrimitives() const
		{
			return Scene->GetPrimitives();
		}
		SceneGraph* RenderSystem::GetScene() const
		{
			return Scene;
		}
		Component* RenderSystem::GetComponent() const
		{
			return Owner;
		}
		SparseIndex& RenderSystem::GetStorageWrapper(uint64_t Section)
		{
			return Scene->GetStorage(Section);
		}
		void RenderSystem::Watch(Core::Vector<Core::Promise<void>>&& Tasks)
		{
			Scene->Watch(TaskType::Rendering, std::move(Tasks));
		}

		ShaderCache::ShaderCache(Graphics::GraphicsDevice* NewDevice) noexcept : Device(NewDevice)
		{
		}
		ShaderCache::~ShaderCache() noexcept
		{
			ClearCache();
		}
		Graphics::ExpectsGraphics<Graphics::Shader*> ShaderCache::Compile(const std::string_view& Name, const Graphics::Shader::Desc& Desc, size_t BufferSize)
		{
			Graphics::Shader* Shader = Get(Name);
			if (Shader != nullptr)
				return Shader;

			auto Resource = Device->CreateShader(Desc);
			if (!Resource || !Resource->IsValid())
			{
				Core::Memory::Release(*Resource);
				if (!Resource)
					return Resource.Error();

				return nullptr;
			}
			else if (BufferSize > 0)
				Device->UpdateBufferSize(*Resource, BufferSize);

			Core::UMutex<std::mutex> Unique(Exclusive);
			SCache& Result = Cache[Core::String(Name)];
			Result.Shader = *Resource;
			Result.Count = 1;
			return *Resource;
		}
		Graphics::Shader* ShaderCache::Get(const std::string_view& Name)
		{
			Core::UMutex<std::mutex> Unique(Exclusive);
			auto It = Cache.find(Core::KeyLookupCast(Name));
			if (It != Cache.end())
			{
				It->second.Count++;
				return It->second.Shader;
			}

			return nullptr;
		}
		Core::String ShaderCache::Find(Graphics::Shader* Shader)
		{
			VI_ASSERT(Shader != nullptr, "shader should be set");
			Core::UMutex<std::mutex> Unique(Exclusive);
			for (auto& Item : Cache)
			{
				if (Item.second.Shader == Shader)
					return Item.first;
			}

			return Core::String();
		}
		const Core::UnorderedMap<Core::String, ShaderCache::SCache>& ShaderCache::GetCaches() const
		{
			return Cache;
		}
		bool ShaderCache::Has(const std::string_view& Name)
		{
			Core::UMutex<std::mutex> Unique(Exclusive);
			auto It = Cache.find(Core::KeyLookupCast(Name));
			return It != Cache.end();
		}
		bool ShaderCache::Free(const std::string_view& Name, Graphics::Shader* Shader)
		{
			Core::UMutex<std::mutex> Unique(Exclusive);
			auto It = Cache.find(Core::KeyLookupCast(Name));
			if (It == Cache.end())
				return false;

			if (Shader != nullptr && Shader != It->second.Shader)
				return false;

			It->second.Count--;
			if (It->second.Count > 0)
				return true;

			Core::Memory::Release(It->second.Shader);
			Cache.erase(It);
			return true;
		}
		void ShaderCache::ClearCache()
		{
			Core::UMutex<std::mutex> Unique(Exclusive);
			for (auto It = Cache.begin(); It != Cache.end(); ++It)
				Core::Memory::Release(It->second.Shader);
			Cache.clear();
		}

		PrimitiveCache::PrimitiveCache(Graphics::GraphicsDevice* Ref) noexcept : Device(Ref), Quad(nullptr), BoxModel(nullptr), SkinBoxModel(nullptr)
		{
			Sphere[0] = Sphere[1] = nullptr;
			Cube[0] = Cube[1] = nullptr;
			Box[0] = Box[1] = nullptr;
			SkinBox[0] = SkinBox[1] = nullptr;
		}
		PrimitiveCache::~PrimitiveCache() noexcept
		{
			ClearCache();
		}
		Graphics::ExpectsGraphics<void> PrimitiveCache::Compile(Graphics::ElementBuffer** Results, const std::string_view& Name, size_t ElementSize, size_t ElementsCount)
		{
			VI_ASSERT(Results != nullptr, "results should be set");
			if (Get(Results, Name))
				return Core::Expectation::Met;

			Graphics::ElementBuffer::Desc F = Graphics::ElementBuffer::Desc();
			F.AccessFlags = Graphics::CPUAccess::Write;
			F.Usage = Graphics::ResourceUsage::Dynamic;
			F.BindFlags = Graphics::ResourceBind::Vertex_Buffer;
			F.ElementWidth = (uint32_t)ElementSize;
			F.ElementCount = (uint32_t)ElementsCount;

			auto VertexBuffer = Device->CreateElementBuffer(F);
			if (!VertexBuffer)
				return VertexBuffer.Error();

			F = Graphics::ElementBuffer::Desc();
			F.AccessFlags = Graphics::CPUAccess::Write;
			F.Usage = Graphics::ResourceUsage::Dynamic;
			F.BindFlags = Graphics::ResourceBind::Index_Buffer;
			F.ElementWidth = (uint32_t)sizeof(int);
			F.ElementCount = (uint32_t)ElementsCount * 3;

			auto IndexBuffer = Device->CreateElementBuffer(F);
			if (!IndexBuffer)
			{
				Core::Memory::Release(*VertexBuffer);
				return IndexBuffer.Error();
			}

			Core::UMutex<std::recursive_mutex> Unique(Exclusive);
			SCache& Result = Cache[Core::String(Name)];
			Result.Buffers[(size_t)BufferType::Index] = Results[(size_t)BufferType::Index] = *IndexBuffer;
			Result.Buffers[(size_t)BufferType::Vertex] = Results[(size_t)BufferType::Vertex] = *VertexBuffer;
			Result.Count = 1;
			return Core::Expectation::Met;
		}
		bool PrimitiveCache::Get(Graphics::ElementBuffer** Results, const std::string_view& Name)
		{
			VI_ASSERT(Results != nullptr, "results should be set");
			Core::UMutex<std::recursive_mutex> Unique(Exclusive);
			auto It = Cache.find(Core::KeyLookupCast(Name));
			if (It == Cache.end())
				return false;

			It->second.Count++;
			Results[(size_t)BufferType::Index] = It->second.Buffers[(size_t)BufferType::Index];
			Results[(size_t)BufferType::Vertex] = It->second.Buffers[(size_t)BufferType::Vertex];
			return true;
		}
		bool PrimitiveCache::Has(const std::string_view& Name)
		{
			Core::UMutex<std::recursive_mutex> Unique(Exclusive);
			auto It = Cache.find(Core::KeyLookupCast(Name));
			return It != Cache.end();
		}
		bool PrimitiveCache::Free(const std::string_view& Name, Graphics::ElementBuffer** Buffers)
		{
			Core::UMutex<std::recursive_mutex> Unique(Exclusive);
			auto It = Cache.find(Core::KeyLookupCast(Name));
			if (It == Cache.end())
				return false;

			if (Buffers != nullptr)
			{
				if ((Buffers[0] != nullptr && Buffers[0] != It->second.Buffers[0]) || (Buffers[1] != nullptr && Buffers[1] != It->second.Buffers[1]))
					return false;
			}

			It->second.Count--;
			if (It->second.Count > 0)
				return true;

			Core::Memory::Release(It->second.Buffers[0]);
			Core::Memory::Release(It->second.Buffers[1]);
			Cache.erase(It);
			return true;
		}
		Core::String PrimitiveCache::Find(Graphics::ElementBuffer** Buffers)
		{
			VI_ASSERT(Buffers != nullptr, "buffers should be set");
			Core::UMutex<std::recursive_mutex> Unique(Exclusive);
			for (auto& Item : Cache)
			{
				if (Item.second.Buffers[0] == Buffers[0] && Item.second.Buffers[1] == Buffers[1])
					return Item.first;
			}

			return Core::String();
		}
		Model* PrimitiveCache::GetBoxModel()
		{
			if (BoxModel != nullptr)
				return BoxModel;

			Core::UMutex<std::recursive_mutex> Unique(Exclusive);
			if (BoxModel != nullptr)
				return BoxModel;

			auto* VertexBuffer = GetBox(BufferType::Vertex);
			if (VertexBuffer != nullptr)
				VertexBuffer->AddRef();

			auto* IndexBuffer = GetBox(BufferType::Index);
			if (IndexBuffer != nullptr)
				IndexBuffer->AddRef();

			BoxModel = new Model();
			BoxModel->Meshes.push_back(*Device->CreateMeshBuffer(VertexBuffer, IndexBuffer));
			return BoxModel;
		}
		SkinModel* PrimitiveCache::GetSkinBoxModel()
		{
			if (SkinBoxModel != nullptr)
				return SkinBoxModel;

			Core::UMutex<std::recursive_mutex> Unique(Exclusive);
			if (SkinBoxModel != nullptr)
				return SkinBoxModel;

			auto* VertexBuffer = GetSkinBox(BufferType::Vertex);
			if (VertexBuffer != nullptr)
				VertexBuffer->AddRef();

			auto* IndexBuffer = GetSkinBox(BufferType::Index);
			if (IndexBuffer != nullptr)
				IndexBuffer->AddRef();

			SkinBoxModel = new SkinModel();
			SkinBoxModel->Meshes.push_back(*Device->CreateSkinMeshBuffer(VertexBuffer, IndexBuffer));
			return SkinBoxModel;
		}
		Graphics::ElementBuffer* PrimitiveCache::GetQuad()
		{
			VI_ASSERT(Device != nullptr, "graphics device should be set");
			if (Quad != nullptr)
				return Quad;

			Core::Vector<Trigonometry::ShapeVertex> Elements =
			{
				{ -1.0f, -1.0f, 0, -1, 0 },
				{ -1.0f, 1.0f, 0, -1, -1 },
				{ 1.0f, 1.0f, 0, 0, -1 },
				{ -1.0f, -1.0f, 0, -1, 0 },
				{ 1.0f, 1.0f, 0, 0, -1 },
				{ 1.0f, -1.0f, 0, 0, 0 }
			};
			Trigonometry::Geometric::TexCoordRhToLh(Elements);

			Graphics::ElementBuffer::Desc F = Graphics::ElementBuffer::Desc();
			F.AccessFlags = Graphics::CPUAccess::None;
			F.Usage = Graphics::ResourceUsage::Default;
			F.BindFlags = Graphics::ResourceBind::Vertex_Buffer;
			F.ElementCount = 6;
			F.ElementWidth = sizeof(Trigonometry::ShapeVertex);
			F.Elements = &Elements[0];

			Core::UMutex<std::recursive_mutex> Unique(Exclusive);
			if (Quad != nullptr)
				return Quad;

			Quad = *Device->CreateElementBuffer(F);
			return Quad;
		}
		Graphics::ElementBuffer* PrimitiveCache::GetSphere(BufferType Type)
		{
			VI_ASSERT(Device != nullptr, "graphics device should be set");
			if (Sphere[(size_t)Type] != nullptr)
				return Sphere[(size_t)Type];

			if (Type == BufferType::Index)
			{
				Core::Vector<int> Indices =
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

				Graphics::ElementBuffer::Desc F = Graphics::ElementBuffer::Desc();
				F.AccessFlags = Graphics::CPUAccess::None;
				F.Usage = Graphics::ResourceUsage::Default;
				F.BindFlags = Graphics::ResourceBind::Index_Buffer;
				F.ElementCount = (uint32_t)Indices.size();
				F.ElementWidth = sizeof(int);
				F.Elements = &Indices[0];

				Core::UMutex<std::recursive_mutex> Unique(Exclusive);
				if (!Sphere[(size_t)BufferType::Index])
					Sphere[(size_t)BufferType::Index] = *Device->CreateElementBuffer(F);

				return Sphere[(size_t)BufferType::Index];
			}
			else if (Type == BufferType::Vertex)
			{
				const float X = 0.525731112119133606f;
				const float Z = 0.850650808352039932f;
				const float N = 0.0f;

				Core::Vector<Trigonometry::ShapeVertex> Elements =
				{
					{ -X, N, Z },
					{ X, N, Z },
					{ -X, N, -Z },
					{ X, N, -Z },
					{ N, Z, X },
					{ N, Z, -X },
					{ N, -Z, X },
					{ N, -Z, -X },
					{ Z, X, N },
					{ -Z, X, N },
					{ Z, -X, N },
					{ -Z, -X, N }
				};
				Trigonometry::Geometric::TexCoordRhToLh(Elements);

				Graphics::ElementBuffer::Desc F = Graphics::ElementBuffer::Desc();
				F.AccessFlags = Graphics::CPUAccess::None;
				F.Usage = Graphics::ResourceUsage::Default;
				F.BindFlags = Graphics::ResourceBind::Vertex_Buffer;
				F.ElementCount = (uint32_t)Elements.size();
				F.ElementWidth = sizeof(Trigonometry::ShapeVertex);
				F.Elements = &Elements[0];

				Core::UMutex<std::recursive_mutex> Unique(Exclusive);
				if (!Sphere[(size_t)BufferType::Vertex])
					Sphere[(size_t)BufferType::Vertex] = *Device->CreateElementBuffer(F);

				return Sphere[(size_t)BufferType::Vertex];
			}

			return nullptr;
		}
		Graphics::ElementBuffer* PrimitiveCache::GetCube(BufferType Type)
		{
			VI_ASSERT(Device != nullptr, "graphics device should be set");
			if (Cube[(size_t)Type] != nullptr)
				return Cube[(size_t)Type];

			if (Type == BufferType::Index)
			{
				Core::Vector<int> Indices =
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

				Graphics::ElementBuffer::Desc F = Graphics::ElementBuffer::Desc();
				F.AccessFlags = Graphics::CPUAccess::None;
				F.Usage = Graphics::ResourceUsage::Default;
				F.BindFlags = Graphics::ResourceBind::Index_Buffer;
				F.ElementCount = (uint32_t)Indices.size();
				F.ElementWidth = sizeof(int);
				F.Elements = &Indices[0];

				Core::UMutex<std::recursive_mutex> Unique(Exclusive);
				if (!Cube[(size_t)BufferType::Index])
					Cube[(size_t)BufferType::Index] = *Device->CreateElementBuffer(F);

				return Cube[(size_t)BufferType::Index];
			}
			else if (Type == BufferType::Vertex)
			{
				Core::Vector<Trigonometry::ShapeVertex> Elements
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
				Trigonometry::Geometric::TexCoordRhToLh(Elements);

				Graphics::ElementBuffer::Desc F = Graphics::ElementBuffer::Desc();
				F.AccessFlags = Graphics::CPUAccess::None;
				F.Usage = Graphics::ResourceUsage::Default;
				F.BindFlags = Graphics::ResourceBind::Vertex_Buffer;
				F.ElementCount = (uint32_t)Elements.size();
				F.ElementWidth = sizeof(Trigonometry::ShapeVertex);
				F.Elements = &Elements[0];

				Core::UMutex<std::recursive_mutex> Unique(Exclusive);
				if (!Cube[(size_t)BufferType::Vertex])
					Cube[(size_t)BufferType::Vertex] = *Device->CreateElementBuffer(F);

				return Cube[(size_t)BufferType::Vertex];
			}

			return nullptr;
		}
		Graphics::ElementBuffer* PrimitiveCache::GetBox(BufferType Type)
		{
			VI_ASSERT(Device != nullptr, "graphics device should be set");
			if (Box[(size_t)Type] != nullptr)
				return Box[(size_t)Type];

			if (Type == BufferType::Index)
			{
				Core::Vector<int> Indices =
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

				Graphics::ElementBuffer::Desc F = Graphics::ElementBuffer::Desc();
				F.AccessFlags = Graphics::CPUAccess::None;
				F.Usage = Graphics::ResourceUsage::Default;
				F.BindFlags = Graphics::ResourceBind::Index_Buffer;
				F.ElementCount = (uint32_t)Indices.size();
				F.ElementWidth = sizeof(int);
				F.Elements = &Indices[0];

				Core::UMutex<std::recursive_mutex> Unique(Exclusive);
				if (!Box[(size_t)BufferType::Index])
					Box[(size_t)BufferType::Index] = *Device->CreateElementBuffer(F);

				return Box[(size_t)BufferType::Index];
			}
			else if (Type == BufferType::Vertex)
			{
				Core::Vector<Trigonometry::Vertex> Elements =
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
				Trigonometry::Geometric::TexCoordRhToLh(Elements);

				Graphics::ElementBuffer::Desc F = Graphics::ElementBuffer::Desc();
				F.AccessFlags = Graphics::CPUAccess::None;
				F.Usage = Graphics::ResourceUsage::Default;
				F.BindFlags = Graphics::ResourceBind::Vertex_Buffer;
				F.ElementCount = (uint32_t)Elements.size();
				F.ElementWidth = sizeof(Trigonometry::Vertex);
				F.Elements = &Elements[0];

				Core::UMutex<std::recursive_mutex> Unique(Exclusive);
				if (!Box[(size_t)BufferType::Vertex])
					Box[(size_t)BufferType::Vertex] = *Device->CreateElementBuffer(F);

				return Box[(size_t)BufferType::Vertex];
			}

			return nullptr;
		}
		Graphics::ElementBuffer* PrimitiveCache::GetSkinBox(BufferType Type)
		{
			VI_ASSERT(Device != nullptr, "graphics device should be set");
			if (SkinBox[(size_t)Type] != nullptr)
				return SkinBox[(size_t)Type];

			if (Type == BufferType::Index)
			{
				Core::Vector<int> Indices =
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

				Graphics::ElementBuffer::Desc F = Graphics::ElementBuffer::Desc();
				F.AccessFlags = Graphics::CPUAccess::None;
				F.Usage = Graphics::ResourceUsage::Default;
				F.BindFlags = Graphics::ResourceBind::Index_Buffer;
				F.ElementCount = (uint32_t)Indices.size();
				F.ElementWidth = sizeof(int);
				F.Elements = &Indices[0];

				Core::UMutex<std::recursive_mutex> Unique(Exclusive);
				if (!SkinBox[(size_t)BufferType::Index])
					SkinBox[(size_t)BufferType::Index] = *Device->CreateElementBuffer(F);

				return SkinBox[(size_t)BufferType::Index];
			}
			else if (Type == BufferType::Vertex)
			{
				Core::Vector<Trigonometry::SkinVertex> Elements =
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
				Trigonometry::Geometric::TexCoordRhToLh(Elements);

				Graphics::ElementBuffer::Desc F = Graphics::ElementBuffer::Desc();
				F.AccessFlags = Graphics::CPUAccess::None;
				F.Usage = Graphics::ResourceUsage::Default;
				F.BindFlags = Graphics::ResourceBind::Vertex_Buffer;
				F.ElementCount = (uint32_t)Elements.size();
				F.ElementWidth = sizeof(Trigonometry::SkinVertex);
				F.Elements = &Elements[0];

				Core::UMutex<std::recursive_mutex> Unique(Exclusive);
				if (!SkinBox[(size_t)BufferType::Vertex])
					SkinBox[(size_t)BufferType::Vertex] = *Device->CreateElementBuffer(F);

				return SkinBox[(size_t)BufferType::Vertex];
			}

			return nullptr;
		}
		const Core::UnorderedMap<Core::String, PrimitiveCache::SCache>& PrimitiveCache::GetCaches() const
		{
			return Cache;
		}
		void PrimitiveCache::GetSphereBuffers(Graphics::ElementBuffer** Result)
		{
			VI_ASSERT(Result != nullptr, "result should be set");
			Result[(size_t)BufferType::Index] = GetSphere(BufferType::Index);
			Result[(size_t)BufferType::Vertex] = GetSphere(BufferType::Vertex);
		}
		void PrimitiveCache::GetCubeBuffers(Graphics::ElementBuffer** Result)
		{
			VI_ASSERT(Result != nullptr, "result should be set");
			Result[(size_t)BufferType::Index] = GetCube(BufferType::Index);
			Result[(size_t)BufferType::Vertex] = GetCube(BufferType::Vertex);
		}
		void PrimitiveCache::GetBoxBuffers(Graphics::ElementBuffer** Result)
		{
			VI_ASSERT(Result != nullptr, "result should be set");
			Result[(size_t)BufferType::Index] = GetBox(BufferType::Index);
			Result[(size_t)BufferType::Vertex] = GetBox(BufferType::Vertex);
		}
		void PrimitiveCache::GetSkinBoxBuffers(Graphics::ElementBuffer** Result)
		{
			VI_ASSERT(Result != nullptr, "result should be set");
			Result[(size_t)BufferType::Index] = GetSkinBox(BufferType::Index);
			Result[(size_t)BufferType::Vertex] = GetSkinBox(BufferType::Vertex);
		}
		void PrimitiveCache::ClearCache()
		{
			Core::UMutex<std::recursive_mutex> Unique(Exclusive);
			for (auto It = Cache.begin(); It != Cache.end(); ++It)
			{
				Core::Memory::Release(It->second.Buffers[0]);
				Core::Memory::Release(It->second.Buffers[1]);
			}

			Cache.clear();
			Core::Memory::Release(BoxModel);
			Core::Memory::Release(SkinBoxModel);
			Core::Memory::Release(Sphere[(size_t)BufferType::Index]);
			Core::Memory::Release(Sphere[(size_t)BufferType::Vertex]);
			Core::Memory::Release(Cube[(size_t)BufferType::Index]);
			Core::Memory::Release(Cube[(size_t)BufferType::Vertex]);
			Core::Memory::Release(Box[(size_t)BufferType::Index]);
			Core::Memory::Release(Box[(size_t)BufferType::Vertex]);
			Core::Memory::Release(SkinBox[(size_t)BufferType::Index]);
			Core::Memory::Release(SkinBox[(size_t)BufferType::Vertex]);
			Core::Memory::Release(Quad);
		}

		template <typename T>
		static void UpgradeBufferByRate(Core::Pool<T>& Storage, float Grow)
		{
			double Size = (double)Storage.Capacity();
			Size *= 1.0 + Grow;

			VI_TRACE("[scene] apply buffer 0x%" PRIXPTR " +%" PRIu64 " bytes", (void*)Storage.Get(), (uint64_t)(sizeof(T) * (Size - Storage.Capacity())));
			Storage.Reserve((size_t)Size);
		}
		template <typename T>
		static void UpgradeBufferBySize(Core::Pool<T>& Storage, size_t Size)
		{
			VI_TRACE("[scene] apply buffer 0x%" PRIXPTR " +%" PRIu64 " bytes", (void*)Storage.Get(), (uint64_t)(sizeof(T) * (Size - Storage.Capacity())));
			Storage.Reserve(Size);
		}

		void SceneGraph::Desc::AddRef()
		{
			if (Shared.Constants != nullptr)
				Shared.Constants->AddRef();

			if (Shared.Shaders != nullptr)
				Shared.Shaders->AddRef();

			if (Shared.Primitives != nullptr)
				Shared.Primitives->AddRef();

			if (Shared.Content != nullptr)
				Shared.Content->AddRef();

			if (Shared.Device != nullptr)
				Shared.Device->AddRef();

			if (Shared.Activity != nullptr)
				Shared.Activity->AddRef();

			if (Shared.VM != nullptr)
				Shared.VM->AddRef();
		}
		void SceneGraph::Desc::Release()
		{
			if (Shared.Constants != nullptr)
			{
				if (Shared.Constants->GetRefCount() == 1)
					Core::Memory::Release(Shared.Constants);
				else
					Shared.Constants->Release();
			}

			if (Shared.Shaders != nullptr)
			{
				if (Shared.Shaders->GetRefCount() == 1)
					Core::Memory::Release(Shared.Shaders);
				else
					Shared.Shaders->Release();
			}

			if (Shared.Primitives != nullptr)
			{
				if (Shared.Primitives->GetRefCount() == 1)
					Core::Memory::Release(Shared.Primitives);
				else
					Shared.Primitives->Release();
			}

			if (Shared.Content != nullptr)
			{
				if (Shared.Content->GetRefCount() == 1)
					Core::Memory::Release(Shared.Content);
				else
					Shared.Content->Release();
			}

			if (Shared.Device != nullptr)
			{
				if (Shared.Device->GetRefCount() == 1)
					Core::Memory::Release(Shared.Device);
				else
					Shared.Device->Release();
			}

			if (Shared.Activity != nullptr)
			{
				if (Shared.Activity->GetRefCount() == 1)
					Core::Memory::Release(Shared.Activity);
				else
					Shared.Activity->Release();
			}

			if (Shared.VM != nullptr)
			{
				if (Shared.VM->GetRefCount() == 1)
					Core::Memory::Release(Shared.VM);
				else
					Shared.VM->Release();
			}
		}
		SceneGraph::Desc SceneGraph::Desc::Get(HeavyApplication* Base)
		{
			SceneGraph::Desc I;
			if (!Base)
				return I;

			I.Shared.Shaders = Base->Cache.Shaders;
			I.Shared.Primitives = Base->Cache.Primitives;
			I.Shared.Constants = Base->Constants;
			I.Shared.Content = Base->Content;
			I.Shared.Device = Base->Renderer;
			I.Shared.Activity = Base->Activity;
			I.Shared.VM = Base->VM;
			return I;
		}

		SceneGraph::SceneGraph(const Desc& I) noexcept : Simulator(new Physics::Simulator(I.Simulator)), Camera(nullptr), Active(true), Conf(I), Snapshot(nullptr)
		{
			for (size_t i = 0; i < (size_t)TargetType::Count * 2; i++)
			{
				Display.MRT[i] = nullptr;
				Display.RT[i] = nullptr;
			}

			auto Components = Core::Composer::Fetch((uint64_t)ComposerTag::Component);
			for (uint64_t Section : Components)
			{
				Registry[Section] = Core::Memory::New<SparseIndex>();
				Changes[Section].clear();
			}

			Display.VoxelBuffers[(size_t)VoxelType::Diffuse] = nullptr;
			Display.VoxelBuffers[(size_t)VoxelType::Normal] = nullptr;
			Display.VoxelBuffers[(size_t)VoxelType::Surface] = nullptr;
			Display.MaterialBuffer = nullptr;
			Display.Merger = nullptr;
			Display.DepthStencil = nullptr;
			Display.Rasterizer = nullptr;
			Display.Blend = nullptr;
			Display.Sampler = nullptr;
			Display.Layout = nullptr;
			Loading.Default = nullptr;

			Conf.AddRef();
			Configure(Conf);
			ScriptHook();
		}
		SceneGraph::~SceneGraph() noexcept
		{
			VI_MEASURE(Core::Timings::Intensive);
			StepTransactions();

			for (auto& Item : Listeners)
			{
				for (auto* Listener : Item.second)
					Core::Memory::Delete(Listener);
				Item.second.clear();
			}
			Listeners.clear();

			for (auto& Item : Entities)
				Core::Memory::Release(Item);
			Entities.Clear();

			for (auto& Item : Materials)
				Core::Memory::Release(Item);
			Materials.Clear();

			for (auto& Sparse : Registry)
				Core::Memory::Delete(Sparse.second);
			Registry.clear();

			Core::Memory::Release(Display.VoxelBuffers[(size_t)VoxelType::Diffuse]);
			Core::Memory::Release(Display.VoxelBuffers[(size_t)VoxelType::Normal]);
			Core::Memory::Release(Display.VoxelBuffers[(size_t)VoxelType::Surface]);
			Core::Memory::Release(Display.Merger);

			for (auto& Item : Display.Voxels)
				Core::Memory::Release(Item.first);
			Display.Voxels.clear();

			for (auto* Item : Display.Points)
				Core::Memory::Release(Item);
			Display.Points.clear();

			for (auto* Item : Display.Spots)
				Core::Memory::Release(Item);
			Display.Spots.clear();

			for (auto& Item : Display.Lines)
			{
				if (Item != nullptr)
				{
					for (auto* Target : *Item)
						Core::Memory::Release(Target);
					Core::Memory::Delete(Item);
				}
			}
			Display.Lines.clear();

			for (size_t i = 0; i < (size_t)TargetType::Count; i++)
			{
				Core::Memory::Release(Display.MRT[i]);
				Core::Memory::Release(Display.RT[i]);
			}

			Core::Memory::Release(Display.MaterialBuffer);
			Core::Memory::Release(Simulator);
			Conf.Release();
		}
		void SceneGraph::Configure(const Desc& NewConf)
		{
			VI_ASSERT(NewConf.Shared.Device != nullptr, "graphics device should be set");
			Transaction([this, NewConf]()
			{
				VI_TRACE("[scene] configure 0x%" PRIXPTR, (void*)this);
				auto* Device = NewConf.Shared.Device;
				Display.DepthStencil = Device->GetDepthStencilState("doo_soo_lt");
				Display.Rasterizer = Device->GetRasterizerState("so_cback");
				Display.Blend = Device->GetBlendState("bo_wrgba_one");
				Display.Sampler = Device->GetSamplerState("a16_fa_wrap");
				Display.Layout = Device->GetInputLayout("vx_shape");

				Conf.Release();
				Conf = NewConf;
				Conf.AddRef();

				Materials.Reserve(Conf.StartMaterials);
				Entities.Reserve(Conf.StartEntities);
				Dirty.Reserve(Conf.StartEntities);

				for (auto& Sparse : Registry)
					Sparse.second->Data.Reserve(Conf.StartComponents);

				for (size_t i = 0; i < (size_t)ActorType::Count; i++)
					Actors[i].Reserve(Conf.StartComponents);

				GenerateMaterialBuffer();
				GenerateVoxelBuffers();
				GenerateDepthBuffers();
				ResizeBuffers();
				Transaction([this]()
				{
					auto* Base = Camera.load();
					if (Base != nullptr)
						Base->Activate(Base);
				});

				auto& Cameras = GetComponents<Components::Camera>();
				for (auto It = Cameras.Begin(); It != Cameras.End(); ++It)
				{
					auto* Base = (Components::Camera*)*It;
					Base->GetRenderer()->Remount();
				}
			});
		}
		void SceneGraph::Actualize()
		{
			VI_TRACE("[scene] actualize 0x%" PRIXPTR, (void*)this);
			StepTransactions();
		}
		void SceneGraph::ResizeBuffers()
		{
			Transaction([this]()
			{
				VI_TRACE("[scene] resize buffers 0x%" PRIXPTR, (void*)this);
				ResizeRenderBuffers();
				if (!Camera.load())
					return;

				auto& Array = GetComponents<Components::Camera>();
				for (auto It = Array.Begin(); It != Array.End(); ++It)
					((Components::Camera*)*It)->ResizeBuffers();
			});
		}
		void SceneGraph::ResizeRenderBuffers()
		{
			Transaction([this]()
			{
				auto* Device = Conf.Shared.Device;
				VI_ASSERT(Device != nullptr, "graphics device should be set");
				Graphics::MultiRenderTarget2D::Desc MRT = GetDescMRT();
				Graphics::RenderTarget2D::Desc RT = GetDescRT();
				Core::Memory::Release(Display.Merger);

				for (size_t i = 0; i < (size_t)TargetType::Count; i++)
				{
					Core::Memory::Release(Display.MRT[i]);
					Display.MRT[i] = *Device->CreateMultiRenderTarget2D(MRT);

					Core::Memory::Release(Display.RT[i]);
					Display.RT[i] = *Device->CreateRenderTarget2D(RT);
				}
			});
		}
		void SceneGraph::FillMaterialBuffers()
		{
			auto* Device = Conf.Shared.Device;
			VI_ASSERT(Device != nullptr, "graphics device should be set");
			Graphics::MappedSubresource Stream;
			if (!Device->Map(Display.MaterialBuffer, Graphics::ResourceMap::Write_Discard, &Stream))
				return;

			size_t Size = 0;
			Subsurface* Array = (Subsurface*)Stream.Pointer;
			auto Begin = Materials.Begin(), End = Materials.End();
			for (auto It = Begin; It != End; ++It)
			{
				Subsurface& Next = Array[Size];
				(*It)->Slot = Size++;
				Next = (*It)->Surface;
			}

			Device->Unmap(Display.MaterialBuffer, &Stream);
			Device->SetStructureBuffer(Display.MaterialBuffer, 0, VI_PS | VI_CS);
		}
		void SceneGraph::Submit()
		{
			auto* Device = Conf.Shared.Device;
			VI_ASSERT(Device != nullptr, "graphics device should be set");
			VI_ASSERT(Conf.Shared.Primitives != nullptr, "primitive cache should be set");
			Conf.Shared.Constants->Render.TexCoord = 1.0f;
			Conf.Shared.Constants->Render.Transform.Identify();
			Conf.Shared.Constants->UpdateConstantBuffer(RenderBufferType::Render);

			Device->SetTarget();
			Device->SetDepthStencilState(Display.DepthStencil);
			Device->SetBlendState(Display.Blend);
			Device->SetRasterizerState(Display.Rasterizer);
			Device->SetInputLayout(Display.Layout);
			Device->SetSamplerState(Display.Sampler, 1, 1, VI_PS);
			Device->SetTexture2D(Display.MRT[(size_t)TargetType::Main]->GetTarget(0), 1, VI_PS);
			Device->SetShader(Conf.Shared.Constants->GetBasicEffect(), VI_VS | VI_PS);
			Device->SetVertexBuffer(Conf.Shared.Primitives->GetQuad());
			Device->Draw(6, 0);
			Device->SetTexture2D(nullptr, 1, VI_PS);
			Statistics.DrawCalls++;
		}
		void SceneGraph::Dispatch(Core::Timer* Time)
		{
			VI_ASSERT(Time != nullptr, "time should be set");
			VI_MEASURE(Core::Timings::Pass);

			StepEvents();
			StepTransactions();
			StepGameplay(Time);
			StepSimulate(Time);
			StepAnimate(Time);
			StepSynchronize(Time);
			StepIndexing();
			StepFinalize();
		}
		void SceneGraph::Publish(Core::Timer* Time)
		{
			VI_ASSERT(Time != nullptr, "timer should be set");
			VI_MEASURE((uint64_t)Core::Timings::Frame * 2);

			auto* Base = (Components::Camera*)Camera.load();
			auto* Renderer = (Base ? Base->GetRenderer() : nullptr);
			Statistics.Batching = 0;
			Statistics.Sorting = 0;
			Statistics.Instances = 0;
			Statistics.DrawCalls = 0;

			if (!Renderer)
				return;

			FillMaterialBuffers();
			SetMRT(TargetType::Main, true);
			Renderer->RestoreViewBuffer(nullptr);
			Statistics.DrawCalls += Renderer->Render(Time, RenderState::Geometric, RenderOpt::None);
		}
		void SceneGraph::PublishAndSubmit(Core::Timer* Time, float R, float G, float B, bool IsParallel)
		{
			VI_ASSERT(Conf.Shared.Device != nullptr, "graphics device should be set");
			if (IsParallel && false)
			{
				Core::UMutex<std::mutex> Unique(Tasking.Update[(size_t)TaskType::Rendering]);
				if (Tasking.IsRendering)
					return;

				Tasking.IsRendering = true;
				Core::Cospawn([this, Time, R, G, B]()
				{
					PublishAndSubmit(Time, R, G, B, false);
					Core::UMutex<std::mutex> Unique(Tasking.Update[(size_t)TaskType::Rendering]);
					Tasking.IsRendering = false;
				});
			}
			else
			{
				auto* Device = Conf.Shared.Device;
				Device->Clear(R, G, B);
				Device->ClearDepth();
				Publish(Time);
				Submit();
				Device->Submit();
			}
		}
		void SceneGraph::StepSimulate(Core::Timer* Time)
		{
			VI_ASSERT(Time != nullptr, "timer should be set");
			VI_ASSERT(Simulator != nullptr, "simulator should be set");
			VI_MEASURE(Core::Timings::Frame);

			if (!Active)
				return;

			Watch(TaskType::Processing, Parallel::Enqueue([this, Time]()
			{
				Simulator->SimulateStep(Time->GetElapsed());
			}));
		}
		void SceneGraph::StepSynchronize(Core::Timer* Time)
		{
			VI_ASSERT(Time != nullptr, "timer should be set");
			VI_MEASURE(Core::Timings::Frame);

			auto& Storage = Actors[(size_t)ActorType::Synchronize];
			if (!Storage.Empty())
			{
				Watch(TaskType::Processing, Parallel::ForEach(Storage.Begin(), Storage.End(), [Time](Component* Next)
				{
					Next->Synchronize(Time);
				}));
			}
		}
		void SceneGraph::StepAnimate(Core::Timer* Time)
		{
			VI_ASSERT(Time != nullptr, "timer should be set");
			VI_MEASURE(Core::Timings::Frame);

			auto& Storage = Actors[(size_t)ActorType::Animate];
			if (Active && !Storage.Empty())
			{
				Watch(TaskType::Processing, Parallel::ForEach(Storage.Begin(), Storage.End(), [Time](Component* Next)
				{
					Next->Animate(Time);
				}));
			}

			if (!Loading.Default)
				return;

			Material* Base = Loading.Default;
			if (Base->GetRefCount() <= 1)
				return;

			Trigonometry::Vector3 Diffuse = Base->Surface.Diffuse / Loading.Progress;
			Loading.Progress = sin(Time->GetElapsed()) * 0.2f + 0.8f;
			Base->Surface.Diffuse = Diffuse * Loading.Progress;
		}
		void SceneGraph::StepGameplay(Core::Timer* Time)
		{
			VI_ASSERT(Time != nullptr, "timer should be set");
			VI_MEASURE(Core::Timings::Pass);

			auto& Storage = Actors[(size_t)ActorType::Update];
			if (Active && !Storage.Empty())
			{
				std::for_each(Storage.Begin(), Storage.End(), [Time](Component* Next)
				{
					Next->Update(Time);
				});
			}
		}
		void SceneGraph::StepTransactions()
		{
			VI_MEASURE(Core::Timings::Pass);
			if (Transactions.empty())
				return;

			VI_TRACE("[scene] process %" PRIu64 " transactions on 0x%" PRIXPTR, (uint64_t)Transactions.size(), (void*)this);
			Await(TaskType::Rendering);

			while (!Transactions.empty())
			{
				Transactions.front()();
				Transactions.pop();
			}
		}
		void SceneGraph::StepEvents()
		{
			VI_MEASURE(Core::Timings::Pass);
			if (!Events.empty())
				VI_TRACE("[scene] resolve %" PRIu64 " events on 0x%" PRIXPTR, (uint64_t)Events.size(), (void*)this);

			while (!Events.empty())
			{
				auto& Source = Events.front();
				ResolveEvent(Source);
				Events.pop();
			}
		}
		void SceneGraph::StepIndexing()
		{
			VI_MEASURE(Core::Timings::Frame);
			if (!Camera.load() || Dirty.Empty())
				return;

			auto Begin = Dirty.Begin();
			auto End = Dirty.End();
			Dirty.Clear();

			Watch(TaskType::Processing, Parallel::ForEach(Begin, End, [this](Entity* Next)
			{
				Next->Transform->Synchronize();
				Next->UpdateBounds();

				if (Next->Type.Components.empty())
					return;

				Core::UMutex<std::mutex> Unique(Exclusive);
				for (auto& Item : *Next)
				{
					if (Item.second->IsCullable())
						Changes[Item.second->GetId()].insert(Item.second);
				}
			}));
		}
		void SceneGraph::StepFinalize()
		{
			VI_MEASURE(Core::Timings::Frame);

			Await(TaskType::Processing);
			if (!Camera.load())
				return;

			for (auto& Item : Changes)
			{
				if (Item.second.empty())
					continue;

				auto& Storage = GetStorage(Item.first);
				Storage.Index.Reserve(Item.second.size());

				size_t Count = 0;
				for (auto It = Item.second.begin(); It != Item.second.end(); ++It)
				{
					UpdateCosmos(Storage, *It);
					if (++Count > Conf.MaxUpdates)
					{
						Item.second.erase(Item.second.begin(), It);
						break;
					}
				}

				if (Count == Item.second.size())
					Item.second.clear();
			}
		}
		void SceneGraph::SetCamera(Entity* NewCamera)
		{
			VI_TRACE("[scene] apply camera 0x%" PRIXPTR " on 0x%" PRIXPTR, (void*)NewCamera, (void*)this);
			if (!NewCamera)
			{
				Camera = nullptr;
				return;
			}

			Components::Camera* Target = NewCamera->GetComponent<Components::Camera>();
			if (!Target || !Target->Active)
			{
				Camera = nullptr;
				return;
			}

			Camera = Target;
			Transaction([Target]()
			{
				Target->Activate(Target);
			});
		}
		void SceneGraph::RemoveEntity(Entity* Entity)
		{
			VI_ASSERT(Entity != nullptr, "entity should be set");
			UnregisterEntity(Entity);
		}
		void SceneGraph::DeleteEntity(Entity* Entity)
		{
			VI_ASSERT(Entity != nullptr, "entity should be set");
			if (!UnregisterEntity(Entity))
				return;

			Entity->RemoveChilds();
			Transaction([Entity]() { Entity->Release(); });
		}
		void SceneGraph::DeleteMaterial(Material* Value)
		{
			VI_ASSERT(Value != nullptr, "entity should be set");
			VI_TRACE("[scene] delete material %s on 0x%" PRIXPTR, (void*)Value->Name.c_str(), (void*)this);
			Mutate(Value, "pop");

			auto Begin = Materials.Begin(), End = Materials.End();
			for (auto It = Begin; It != End; ++It)
			{
				if (*It == Value)
				{
					Materials.RemoveAt(It);
					break;
				}
			}

			Transaction([Value]() { Value->Release(); });
		}
		void SceneGraph::RegisterEntity(Entity* Target)
		{
			VI_ASSERT(Target != nullptr, "entity should be set");
			VI_ASSERT(Target->Scene == this, "entity be created within this scene");
			VI_TRACE("[scene] register entity 0x%" PRIXPTR " on 0x%" PRIXPTR, (void*)Target, (void*)this);

			Target->Active = true;
			for (auto& Base : Target->Type.Components)
				RegisterComponent(Base.second, Target->Active);

			WatchMovement(Target);
			Mutate(Target, "push");
		}
		bool SceneGraph::UnregisterEntity(Entity* Target)
		{
			VI_ASSERT(Target != nullptr, "entity should be set");
			VI_ASSERT(Target->GetScene() == this, "entity should be attached to current scene");
			VI_TRACE("[scene] unregister entity 0x%" PRIXPTR " on 0x%" PRIXPTR, (void*)Target, (void*)this);

			Component* Viewer = Camera.load();
			if (Viewer != nullptr && Target == Viewer->Parent)
				Camera = nullptr;

			for (auto& Base : Target->Type.Components)
				UnregisterComponent(Base.second);

			Target->Active = false;
			Entities.Remove(Target);
			UnwatchMovement(Target);
			Mutate(Target, "pop");

			return true;
		}
		void SceneGraph::RegisterComponent(Component* Base, bool Verify)
		{
			VI_TRACE("[scene] register component 0x%" PRIXPTR " on 0x%" PRIXPTR "%s", (void*)Base, (void*)this, Verify ? " (with checks)" : "");
			auto& Storage = GetComponents(Base->GetId());
			if (!Base->Active)
			{
				Transaction([Base]()
				{
					Base->Activate(nullptr);
				});
			}

			if (Verify)
			{
				Storage.AddIfNotExists(Base);
				if (Base->Set & (size_t)ActorSet::Update)
					GetActors(ActorType::Update).AddIfNotExists(Base);
				if (Base->Set & (size_t)ActorSet::Synchronize)
					GetActors(ActorType::Synchronize).AddIfNotExists(Base);
				if (Base->Set & (size_t)ActorSet::Animate)
					GetActors(ActorType::Animate).AddIfNotExists(Base);
				if (Base->Set & (size_t)ActorSet::Message)
					GetActors(ActorType::Message).AddIfNotExists(Base);
			}
			else
			{
				Storage.Add(Base);
				if (Base->Set & (size_t)ActorSet::Update)
					GetActors(ActorType::Update).Add(Base);
				if (Base->Set & (size_t)ActorSet::Synchronize)
					GetActors(ActorType::Synchronize).Add(Base);
				if (Base->Set & (size_t)ActorSet::Animate)
					GetActors(ActorType::Animate).Add(Base);
				if (Base->Set & (size_t)ActorSet::Message)
					GetActors(ActorType::Message).Add(Base);
			}
		}
		void SceneGraph::UnregisterComponent(Component* Base)
		{
			VI_TRACE("[scene] unregister component 0x%" PRIXPTR " on 0x%" PRIXPTR, (void*)Base, (void*)this);
			auto& Storage = GetComponents(Base->GetId());
			if (Base->Active)
				Base->Deactivate();

			Storage.Remove(Base);
			if (Base->Set & (size_t)ActorSet::Update)
				GetActors(ActorType::Update).Remove(Base);
			if (Base->Set & (size_t)ActorSet::Synchronize)
				GetActors(ActorType::Synchronize).Remove(Base);
			if (Base->Set & (size_t)ActorSet::Animate)
				GetActors(ActorType::Animate).Remove(Base);
			if (Base->Set & (size_t)ActorSet::Message)
				GetActors(ActorType::Message).Remove(Base);
		}
		void SceneGraph::LoadComponent(Component* Base)
		{
			VI_ASSERT(Base != nullptr, "component should be set");
			VI_ASSERT(Base->Parent != nullptr && Base->Parent->Scene == this, "component should be tied to this scene");
			VI_TRACE("[scene] await component 0x%" PRIXPTR " on 0x%" PRIXPTR, (void*)Base, (void*)this);
			Core::UMutex<std::mutex> Unique(Exclusive);
			++Incomplete[Base];
		}
		void SceneGraph::UnloadComponentAll(Component* Base)
		{
			VI_TRACE("[scene] resolve component 0x%" PRIXPTR " on 0x%" PRIXPTR " fully", (void*)Base, (void*)this);
			Core::UMutex<std::mutex> Unique(Exclusive);
			auto It = Incomplete.find(Base);
			if (It != Incomplete.end())
				Incomplete.erase(It);
		}
		bool SceneGraph::UnloadComponent(Component* Base)
		{
			Core::UMutex<std::mutex> Unique(Exclusive);
			auto It = Incomplete.find(Base);
			if (It == Incomplete.end())
				return false;
			
			if (!--It->second)
			{
				VI_TRACE("[scene] resolve component 0x%" PRIXPTR " on 0x%" PRIXPTR, (void*)Base, (void*)this);
				Incomplete.erase(It);
			}

			return true;
		}
		void SceneGraph::CloneEntities(Entity* Instance, Core::Vector<Entity*>* Array)
		{
			VI_ASSERT(Instance != nullptr, "entity should be set");
			VI_ASSERT(Array != nullptr, "array should be set");
			VI_TRACE("[scene] clone entity 0x%" PRIXPTR " on 0x%" PRIXPTR, (void*)Instance, (void*)this);

			Entity* Clone = CloneEntityInstance(Instance);
			Array->push_back(Clone);

			Trigonometry::Transform* Root = Clone->Transform->GetRoot();
			if (Root != nullptr)
				Root->GetChilds().push_back(Clone->Transform);

			if (!Instance->Transform->GetChildsCount())
				return;

			Core::Vector<Trigonometry::Transform*>& Childs = Instance->Transform->GetChilds();
			Clone->Transform->GetChilds().clear();

			for (auto& Child : Childs)
			{
				size_t Offset = Array->size();
				CloneEntities((Entity*)Child->UserData, Array);
				for (size_t j = Offset; j < Array->size(); j++)
				{
					Entity* Next = (*Array)[j];
					if (Next->Transform->GetRoot() == Instance->Transform)
						Next->Transform->SetRoot(Clone->Transform);
				}
			}
		}
		void SceneGraph::RayTest(uint64_t Section, const Trigonometry::Ray& Origin, const RayCallback& Callback)
		{
			VI_ASSERT(Callback, "callback should not be empty");
			VI_MEASURE(Core::Timings::Pass);

			auto& Array = GetComponents(Section);
			Trigonometry::Ray Base = Origin;
			Trigonometry::Vector3 Hit;

			for (auto& Next : Array)
			{
				if (Trigonometry::Geometric::CursorRayTest(Base, Next->Parent->Snapshot.Box, &Hit) && !Callback(Next, Hit))
					break;
			}
		}
		void SceneGraph::ScriptHook(const std::string_view& Name)
		{
			Transaction([this, Name]()
			{
				VI_TRACE("[scene] script hook call: %.*s() on 0x%" PRIXPTR, (int)Name.size(), Name.data(), (void*)this);
				auto& Array = GetComponents<Components::Scriptable>();
				for (auto It = Array.Begin(); It != Array.End(); ++It)
				{
					Components::Scriptable* Base = (Components::Scriptable*)*It;
					Base->CallEntry(Name);
				}
			});
		}
		void SceneGraph::SetActive(bool Enabled)
		{
			Active = Enabled;
			if (!Active)
				return;

			Transaction([this]()
			{
				VI_TRACE("[scene] perform activation on 0x%" PRIXPTR, (void*)this);
				auto Begin = Entities.Begin(), End = Entities.End();
				for (auto It = Begin; It != End; ++It)
				{
					Entity* Next = *It;
					for (auto& Base : Next->Type.Components)
					{
						if (Base.second->Active)
							Base.second->Activate(nullptr);
					}
				}
			});
		}
		void SceneGraph::SetMRT(TargetType Type, bool Clear)
		{
			auto* Device = Conf.Shared.Device;
			VI_ASSERT(Device != nullptr, "graphics device should be set");
			Graphics::MultiRenderTarget2D* Target = Display.MRT[(size_t)Type];
			Device->SetTarget(Target);

			if (!Clear)
				return;

			Device->Clear(Target, 0, 0, 0, 0);
			Device->Clear(Target, 1, 0, 0, 0);
			Device->Clear(Target, 2, 1, 0, 0);
			Device->Clear(Target, 3, 0, 0, 0);
			Device->ClearDepth(Target);
		}
		void SceneGraph::SetRT(TargetType Type, bool Clear)
		{
			auto* Device = Conf.Shared.Device;
			VI_ASSERT(Device != nullptr, "graphics device should be set");
			Graphics::RenderTarget2D* Target = Display.RT[(size_t)Type];
			Device->SetTarget(Target);

			if (!Clear)
				return;

			Device->Clear(Target, 0, 0, 0, 0);
			Device->ClearDepth(Target);
		}
		void SceneGraph::SwapMRT(TargetType Type, Graphics::MultiRenderTarget2D* New)
		{
			size_t Index = (size_t)Type;
			if (Display.MRT[Index] == New)
				return;

			Graphics::MultiRenderTarget2D* Cache = Display.MRT[Index + (size_t)TargetType::Count];
			if (New != nullptr)
			{
				Graphics::MultiRenderTarget2D* Base = Display.MRT[Index];
				Display.MRT[Index] = New;

				if (!Cache)
					Display.MRT[Index + (size_t)TargetType::Count] = Base;
			}
			else if (Cache != nullptr)
			{
				Display.MRT[Index] = Cache;
				Display.MRT[Index + (size_t)TargetType::Count] = nullptr;
			}
		}
		void SceneGraph::SwapRT(TargetType Type, Graphics::RenderTarget2D* New)
		{
			size_t Index = (size_t)Type;
			Graphics::RenderTarget2D* Cache = Display.RT[Index + (size_t)TargetType::Count];
			if (New != nullptr)
			{
				Graphics::RenderTarget2D* Base = Display.RT[Index];
				Display.RT[Index] = New;

				if (!Cache)
					Display.RT[Index + (size_t)TargetType::Count] = Base;
			}
			else if (Cache != nullptr)
			{
				Display.RT[Index] = Cache;
				Display.RT[Index + (size_t)TargetType::Count] = nullptr;
			}
		}
		void SceneGraph::ClearMRT(TargetType Type, bool Color, bool Depth)
		{
			auto* Device = Conf.Shared.Device;
			VI_ASSERT(Device != nullptr, "graphics device should be set");
			Graphics::MultiRenderTarget2D* Target = Display.MRT[(size_t)Type];
			if (Color)
			{
				Device->Clear(Target, 0, 0, 0, 0);
				Device->Clear(Target, 1, 0, 0, 0);
				Device->Clear(Target, 2, 1, 0, 0);
				Device->Clear(Target, 3, 0, 0, 0);
			}

			if (Depth)
				Device->ClearDepth(Target);
		}
		void SceneGraph::ClearRT(TargetType Type, bool Color, bool Depth)
		{
			auto* Device = Conf.Shared.Device;
			VI_ASSERT(Device != nullptr, "graphics device should be set");
			Graphics::RenderTarget2D* Target = Display.RT[(size_t)Type];
			if (Color)
				Device->Clear(Target, 0, 0, 0, 0);

			if (Depth)
				Device->ClearDepth(Target);
		}
		bool SceneGraph::GetVoxelBuffer(Graphics::Texture3D** In, Graphics::Texture3D** Out)
		{
			VI_ASSERT(In && Out, "input and output should be set");
			VI_ASSERT(Display.VoxelBuffers[0] != nullptr, "first voxel buffer should be set");
			VI_ASSERT(Display.VoxelBuffers[1] != nullptr, "second voxel buffer should be set");
			VI_ASSERT(Display.VoxelBuffers[2] != nullptr, "third voxel buffer should be set");

			for (uint32_t i = 0; i < 3; i++)
			{
				In[i] = Display.VoxelBuffers[i];
				Out[i] = nullptr;
			}

			return true;
		}
		MessageCallback* SceneGraph::SetListener(const std::string_view& EventName, MessageCallback&& Callback)
		{
			VI_ASSERT(Callback != nullptr, "callback should be set");
			VI_TRACE("[scene] attach listener %.*s on 0x%" PRIXPTR, (int)EventName.size(), EventName.data(), (void*)this);
			MessageCallback* Id = Core::Memory::New<MessageCallback>(std::move(Callback));
			Core::UMutex<std::mutex> Unique(Exclusive);
			Listeners[Core::String(EventName)].insert(Id);

			return Id;
		}
		bool SceneGraph::ClearListener(const std::string_view& EventName, MessageCallback* Id)
		{
			VI_ASSERT(!EventName.empty(), "event name should not be empty");
			VI_ASSERT(Id != nullptr, "callback id should be set");
			VI_TRACE("[scene] detach listener %.*s on 0x%" PRIXPTR, (int)EventName.size(), EventName.data(), (void*)this);

			Core::UMutex<std::mutex> Unique(Exclusive);
			auto& Source = Listeners[Core::String(EventName)];
			auto It = Source.find(Id);
			if (It == Source.end())
				return false;

			Source.erase(It);
			Core::Memory::Delete(Id);
			return true;
		}
		bool SceneGraph::PushEvent(const std::string_view& EventName, Core::VariantArgs&& Args, bool Propagate)
		{
			VI_TRACE("[scene] push %.*s event %s on 0x%" PRIXPTR, Propagate ? "scene" : "listener", (int)EventName.size(), EventName.data(), (void*)this);
			Event Next(EventName, std::move(Args));
			Next.Args["__vb"] = Core::Var::Integer((int64_t)(Propagate ? EventTarget::Scene : EventTarget::Listener));
			Next.Args["__vt"] = Core::Var::Pointer((void*)this);

			Core::UMutex<std::mutex> Unique(Exclusive);
			Events.push(std::move(Next));
			return true;
		}
		bool SceneGraph::PushEvent(const std::string_view& EventName, Core::VariantArgs&& Args, Component* Target)
		{
			VI_TRACE("[scene] push component event %.*s on 0x%" PRIXPTR " for 0x%" PRIXPTR, (int)EventName.size(), EventName.data(), (void*)this, (void*)Target);
			VI_ASSERT(Target != nullptr, "target should be set");
			Event Next(EventName, std::move(Args));
			Next.Args["__vb"] = Core::Var::Integer((int64_t)EventTarget::Component);
			Next.Args["__vt"] = Core::Var::Pointer((void*)Target);

			Core::UMutex<std::mutex> Unique(Exclusive);
			Events.push(std::move(Next));
			return true;
		}
		bool SceneGraph::PushEvent(const std::string_view& EventName, Core::VariantArgs&& Args, Entity* Target)
		{
			VI_TRACE("[scene] push entity event %.*s on 0x%" PRIXPTR " for 0x%" PRIXPTR, (int)EventName.size(), EventName.data(), (void*)this, (void*)Target);
			VI_ASSERT(Target != nullptr, "target should be set");
			Event Next(EventName, std::move(Args));
			Next.Args["__vb"] = Core::Var::Integer((int64_t)EventTarget::Entity);
			Next.Args["__vt"] = Core::Var::Pointer((void*)Target);

			Core::UMutex<std::mutex> Unique(Exclusive);
			Events.push(std::move(Next));
			return true;
		}
		void SceneGraph::LoadResource(uint64_t Id, Component* Context, const std::string_view& Path, const Core::VariantArgs& Keys, std::function<void(ExpectsContent<void*>&&)>&& Callback)
		{
			VI_ASSERT(Conf.Shared.Content != nullptr, "content manager should be set");
			VI_ASSERT(Context != nullptr, "component calling this function should be set");
			VI_ASSERT(Callback != nullptr, "callback should be set");

			LoadComponent(Context);
			Conf.Shared.Content->LoadDeferred(Conf.Shared.Content->GetProcessor(Id), Path, Keys).When([this, Context, Callback = std::move(Callback)](ExpectsContent<void*>&& Result)
			{
				if (!UnloadComponent(Context))
					return;

				Transaction([Context, Callback = std::move(Callback), Result = std::move(Result)]() mutable
				{
					Result.Report("scene resource loading error");
					Callback(std::move(Result));
					Context->Parent->Transform->MakeDirty();
				});
			});
		}
		Core::String SceneGraph::FindResourceId(uint64_t Id, void* Resource)
		{
			AssetCache* Cache = Conf.Shared.Content->FindCache(Conf.Shared.Content->GetProcessor(Id), Resource);
			return Cache != nullptr ? AsResourcePath(Cache->Path) : Core::String();
		}
		bool SceneGraph::IsLeftHanded() const
		{
			return Conf.Shared.Device->IsLeftHanded();
		}
		bool SceneGraph::IsIndexed() const
		{
			for (auto& Item : Changes)
			{
				if (!Item.second.empty())
					return false;
			}

			return true;
		}
		bool SceneGraph::IsBusy(TaskType Type)
		{
			if (Type != TaskType::Rendering)
				return !Tasking.Queue[(size_t)Type].empty();

			Core::UMutex<std::mutex> Unique(Tasking.Update[(size_t)Type]);
			return !Tasking.Queue[(size_t)Type].empty();
		}
		void SceneGraph::Mutate(Entity* Parent, Entity* Child, const std::string_view& Type)
		{
			VI_ASSERT(Parent != nullptr, "parent should be set");
			VI_ASSERT(Child != nullptr, "child should be set");
			if (!Conf.Mutations)
				return;

			Core::VariantArgs Args;
			Args["parent"] = Core::Var::Pointer((void*)Parent);
			Args["child"] = Core::Var::Pointer((void*)Child);
			Args["type"] = Core::Var::String(Type);
			PushEvent("mutation", std::move(Args), false);
		}
		void SceneGraph::Mutate(Entity* Target, const std::string_view& Type)
		{
			VI_ASSERT(Target != nullptr, "target should be set");
			if (!Conf.Mutations)
				return;

			Core::VariantArgs Args;
			Args["entity"] = Core::Var::Pointer((void*)Target);
			Args["type"] = Core::Var::String(Type);
			PushEvent("mutation", std::move(Args), false);
		}
		void SceneGraph::Mutate(Component* Target, const std::string_view& Type)
		{
			VI_ASSERT(Target != nullptr, "target should be set");
			NotifyCosmos(Target);

			if (!Conf.Mutations)
				return;

			Core::VariantArgs Args;
			Args["component"] = Core::Var::Pointer((void*)Target);
			Args["type"] = Core::Var::String(Type);
			PushEvent("mutation", std::move(Args), false);
		}
		void SceneGraph::Mutate(Material* Target, const std::string_view& Type)
		{
			VI_ASSERT(Target != nullptr, "target should be set");
			if (!Conf.Mutations)
				return;

			Core::VariantArgs Args;
			Args["material"] = Core::Var::Pointer((void*)Target);
			Args["type"] = Core::Var::String(Type);
			PushEvent("mutation", std::move(Args), false);
		}
		void SceneGraph::MakeSnapshot(IdxSnapshot* Result)
		{
			VI_ASSERT(Result != nullptr, "shapshot result should be set");
			VI_TRACE("[scene] generate snapshot on 0x%" PRIXPTR, (void*)this);
			Result->To.clear();
			Result->From.clear();

			size_t Index = 0;
			auto Begin = Entities.Begin(), End = Entities.End();
			for (auto It = Begin; It != End; ++It)
			{
				Result->To[*It] = Index;
				Result->From[Index] = *It;
				Index++;
			}
		}
		void SceneGraph::Transaction(Core::TaskCallback&& Callback)
		{
			VI_ASSERT(Callback != nullptr, "callback should be set");
			bool ExecuteNow = false;
			{
				Core::UMutex<std::mutex> Unique(Exclusive);
				if (!Events.empty() || IsBusy(TaskType::Processing) || IsBusy(TaskType::Rendering))
					Transactions.emplace(std::move(Callback));
				else
					ExecuteNow = true;
			}
			if (ExecuteNow)
				Callback();
		}
		void SceneGraph::Watch(TaskType Type, Core::Promise<void>&& Awaitable)
		{
			if (!Awaitable.IsPending())
				return;

			if (Type == TaskType::Rendering)
			{
				Core::UMutex<std::mutex> Unique(Tasking.Update[(size_t)Type]);
				Tasking.Queue[(size_t)Type].emplace(std::move(Awaitable));
			}
			else
				Tasking.Queue[(size_t)Type].emplace(std::move(Awaitable));
		}
		void SceneGraph::Watch(TaskType Type, Core::Vector<Core::Promise<void>>&& Awaitables)
		{
			if (Type == TaskType::Rendering)
			{
				Core::UMutex<std::mutex> Unique(Tasking.Update[(size_t)Type]);
				for (auto& Awaitable : Awaitables)
				{
					if (Awaitable.IsPending())
						Tasking.Queue[(size_t)Type].emplace(std::move(Awaitable));
				}
			}
			else
			{
				for (auto& Awaitable : Awaitables)
				{
					if (Awaitable.IsPending())
						Tasking.Queue[(size_t)Type].emplace(std::move(Awaitable));
				}
			}
		}
		void SceneGraph::Await(TaskType Type)
		{
			VI_MEASURE(Core::Timings::Frame);
			if (Type == TaskType::Rendering)
			{
				Core::UMutex<std::mutex> Unique(Tasking.Update[(size_t)Type]);
				auto& Queue = Tasking.Queue[(size_t)Type];
				while (!Queue.empty())
				{
					Queue.front().Wait();
					Queue.pop();
				}
			}
			else
			{
				auto& Queue = Tasking.Queue[(size_t)Type];
				while (!Queue.empty())
				{
					Queue.front().Wait();
					Queue.pop();
				}
			}
		}
		void SceneGraph::ClearCulling()
		{
			for (auto& Sparse : Registry)
			{
				auto Begin = Sparse.second->Data.Begin();
				auto End = Sparse.second->Data.End();
				for (auto* It = Begin; It != End; ++It)
				{
					Component* Target = *It;
					if (Target->IsDrawable())
					{
						Drawable* Base = (Drawable*)Target;
						Base->Overlapping = 1.0f;
					}
				}
			}
		}
		void SceneGraph::ReserveMaterials(size_t Size)
		{
			Transaction([this, Size]()
			{
				UpgradeBufferBySize(Materials, Size);
				GenerateMaterialBuffer();
			});
		}
		void SceneGraph::ReserveEntities(size_t Size)
		{
			Transaction([this, Size]()
			{
				UpgradeBufferBySize(Dirty, Size);
				UpgradeBufferBySize(Entities, Size);
			});
		}
		void SceneGraph::ReserveComponents(uint64_t Section, size_t Size)
		{
			Transaction([this, Section, Size]()
			{
				auto& Storage = GetStorage(Section);
				UpgradeBufferBySize(Storage.Data, Size);
			});
		}
		void SceneGraph::GenerateMaterialBuffer()
		{
			VI_TRACE("[scene] generate material buffer %" PRIu64 "m on 0x%" PRIXPTR, (uint64_t)Materials.Capacity(), (void*)this);
			Graphics::ElementBuffer::Desc F = Graphics::ElementBuffer::Desc();
			F.AccessFlags = Graphics::CPUAccess::Write;
			F.MiscFlags = Graphics::ResourceMisc::Buffer_Structured;
			F.Usage = Graphics::ResourceUsage::Dynamic;
			F.BindFlags = Graphics::ResourceBind::Shader_Input;
			F.ElementCount = (uint32_t)Materials.Capacity();
			F.ElementWidth = sizeof(Subsurface);
			F.StructureByteStride = F.ElementWidth;

			Core::Memory::Release(Display.MaterialBuffer);
			Display.MaterialBuffer = *Conf.Shared.Device->CreateElementBuffer(F);
		}
		void SceneGraph::GenerateVoxelBuffers()
		{
			auto* Device = Conf.Shared.Device;
			VI_ASSERT(Device != nullptr, "graphics device should be set");
			VI_TRACE("[scene] generate voxel buffers %" PRIu64 "v on 0x%" PRIXPTR, (uint64_t)(Conf.VoxelsSize * Conf.VoxelsSize * Conf.VoxelsSize * 3), (void*)this);

			Conf.VoxelsSize = Conf.VoxelsSize - Conf.VoxelsSize % 8;
			Conf.VoxelsMips = Device->GetMipLevel((uint32_t)Conf.VoxelsSize, (uint32_t)Conf.VoxelsSize);

			Graphics::Texture3D::Desc I;
			I.Width = I.Height = I.Depth = (uint32_t)Conf.VoxelsSize;
			I.MipLevels = 0;
			I.Writable = true;

			Core::Memory::Release(Display.VoxelBuffers[(size_t)VoxelType::Diffuse]);
			Display.VoxelBuffers[(size_t)VoxelType::Diffuse] = *Device->CreateTexture3D(I);

			I.FormatMode = Graphics::Format::R16G16B16A16_Float;
			Core::Memory::Release(Display.VoxelBuffers[(size_t)VoxelType::Normal]);
			Display.VoxelBuffers[(size_t)VoxelType::Normal] = *Device->CreateTexture3D(I);

			I.FormatMode = Graphics::Format::R8G8B8A8_Unorm;
			Core::Memory::Release(Display.VoxelBuffers[(size_t)VoxelType::Surface]);
			Display.VoxelBuffers[(size_t)VoxelType::Surface] = *Device->CreateTexture3D(I);

			I.MipLevels = (uint32_t)Conf.VoxelsMips;
			Display.Voxels.resize(Conf.VoxelsMax);
			for (auto& Item : Display.Voxels)
			{
				Core::Memory::Release(Item.first);
				Item.first = *Device->CreateTexture3D(I);
				Item.second = nullptr;
			}

			if (Entities.Empty())
				return;

			Core::VariantArgs Args;
			PushEvent("voxel-flush", std::move(Args), true);
		}
		void SceneGraph::GenerateDepthBuffers()
		{
			auto* Device = Conf.Shared.Device;
			VI_ASSERT(Device != nullptr, "graphics device should be set");
			VI_TRACE("[scene] generate depth buffers %" PRIu64 "t on 0x%" PRIXPTR, (uint64_t)(Conf.PointsMax + Conf.SpotsMax + Conf.LinesMax), (void*)this);

			for (auto& Item : Display.Points)
				Core::Memory::Release(Item);

			for (auto& Item : Display.Spots)
				Core::Memory::Release(Item);

			for (auto& Item : Display.Lines)
			{
				if (Item != nullptr)
				{
					for (auto* Target : *Item)
						Core::Memory::Release(Target);
					Core::Memory::Delete(Item);
					Item = nullptr;
				}
			}

			Display.Points.resize(Conf.PointsMax);
			for (auto& Item : Display.Points)
			{
				Graphics::DepthTargetCube::Desc F = Graphics::DepthTargetCube::Desc();
				F.Size = (uint32_t)Conf.PointsSize;
				F.FormatMode = Graphics::Format::D32_Float;
				Item = *Device->CreateDepthTargetCube(F);
			}

			Display.Spots.resize(Conf.SpotsMax);
			for (auto& Item : Display.Spots)
			{
				Graphics::DepthTarget2D::Desc F = Graphics::DepthTarget2D::Desc();
				F.Width = F.Height = (uint32_t)Conf.SpotsSize;
				F.FormatMode = Graphics::Format::D32_Float;
				Item = *Device->CreateDepthTarget2D(F);
			}

			Display.Lines.resize(Conf.LinesMax);
			for (auto& Item : Display.Lines)
				Item = nullptr;

			if (Entities.Empty())
				return;

			Core::VariantArgs Args;
			PushEvent("depth-flush", std::move(Args), true);
		}
		void SceneGraph::GenerateDepthCascades(CascadedDepthMap** Result, uint32_t Size) const
		{
			VI_TRACE("[scene] generate depth cascades %is on 0x%" PRIXPTR, (int)(Size * Size), (void*)this);
			CascadedDepthMap* Target = (*Result ? *Result : Core::Memory::New<CascadedDepthMap>());
			for (auto& Item : *Target)
				Core::Memory::Release(Item);

			Target->resize(Size);
			for (auto& Item : *Target)
			{
				Graphics::DepthTarget2D::Desc F = Graphics::DepthTarget2D::Desc();
				F.Width = F.Height = (uint32_t)Conf.LinesSize;
				F.FormatMode = Graphics::Format::D32_Float;
				Item = *Conf.Shared.Device->CreateDepthTarget2D(F);
			}

			*Result = Target;
		}
		void SceneGraph::NotifyCosmos(Component* Base)
		{
			if (!Base->IsCullable())
				return;

			Core::UMutex<std::mutex> Unique(Exclusive);
			Changes[Base->GetId()].insert(Base);
		}
		void SceneGraph::ClearCosmos(Component* Base)
		{
			if (!Base->IsCullable())
				return;

			uint64_t Id = Base->GetId();
			Core::UMutex<std::mutex> Unique(Exclusive);
			Changes[Id].erase(Base);

			if (Base->Indexed)
			{
				auto& Storage = GetStorage(Id);
				Storage.Index.RemoveItem((void*)Base);
			}
		}
		void SceneGraph::UpdateCosmos(SparseIndex& Storage, Component* Base)
		{
			if (Base->Active)
			{
				auto& Bounds = Base->Parent->Snapshot;
				if (!Base->Indexed)
				{
					Storage.Index.InsertItem((void*)Base, Bounds.Min, Bounds.Max);
					Base->Indexed = true;
				}
				else
					Storage.Index.UpdateItem((void*)Base, Bounds.Min, Bounds.Max);
			}
			else
			{
				Storage.Index.RemoveItem((void*)Base);
				Base->Indexed = false;
			}
		}
		void SceneGraph::WatchMovement(Entity* Base)
		{
			Base->Transform->WhenDirty([this, Base]()
			{
				Transaction([this, Base]()
				{
					if (Dirty.Size() + Conf.GrowMargin > Dirty.Capacity())
						UpgradeBufferByRate(Dirty, (float)Conf.GrowRate);
					Dirty.Add(Base);
				});
			});
		}
		void SceneGraph::UnwatchMovement(Entity* Base)
		{
			Base->Transform->WhenDirty(nullptr);
			Dirty.Remove(Base);
		}
		bool SceneGraph::ResolveEvent(Event& Source)
		{
			auto _Bubble = Source.Args.find("__vb"), _Target = Source.Args.find("__vt");
			if (_Bubble == Source.Args.end() || _Target == Source.Args.end())
				return false;

			EventTarget Bubble = (EventTarget)_Bubble->second.GetInteger();
			void* Target = _Target->second.GetPointer();

			if (Bubble == EventTarget::Scene)
			{
				auto Begin = Actors[(size_t)ActorType::Message].Begin();
				auto End = Actors[(size_t)ActorType::Message].End();
				for (auto It = Begin; It != End; ++It)
					(*It)->Message(Source.Name, Source.Args);
			}
			else if (Bubble == EventTarget::Entity)
			{
				Entity* Base = (Entity*)Target;
				for (auto& Item : Base->Type.Components)
					Item.second->Message(Source.Name, Source.Args);
			}
			else if (Bubble == EventTarget::Component)
			{
				Component* Base = (Component*)Target;
				Base->Message(Source.Name, Source.Args);
			}

			auto It = Listeners.find(Source.Name);
			if (It == Listeners.end() || It->second.empty())
				return false;

			auto Copy = It->second;
			for (auto* Callback : Copy)
				(*Callback)(Source.Name, Source.Args);

			return true;
		}
		bool SceneGraph::AddMaterial(Material* Base)
		{
			VI_ASSERT(Base != nullptr, "base should be set");
			if (Materials.Size() + Conf.GrowMargin > Materials.Capacity())
			{
				Transaction([this, Base]()
				{
					if (Materials.Size() + Conf.GrowMargin > Materials.Capacity())
					{
						UpgradeBufferByRate(Materials, (float)Conf.GrowRate);
						GenerateMaterialBuffer();
					}
					AddMaterial(Base);
				});
			}
			else
			{
				VI_TRACE("[scene] add material %s on 0x%" PRIXPTR, Base->Name.c_str(), (void*)this);
				Base->Scene = this;
				Materials.AddIfNotExists(Base);
				Mutate(Base, "push");
			}

			return true;
		}
		Material* SceneGraph::GetInvalidMaterial()
		{
			if (!Loading.Default.load())
				Loading.Default = AddMaterial();

			return Loading.Default;
		}
		Material* SceneGraph::AddMaterial()
		{
			Material* Result = new Material(this);
			AddMaterial(Result);
			return Result;
		}
		Material* SceneGraph::CloneMaterial(Material* Base)
		{
			VI_ASSERT(Base != nullptr, "material should be set");
			Material* Result = new Material(*Base);
			AddMaterial(Result);
			return Result;
		}
		Component* SceneGraph::GetCamera()
		{
			Component* Base = Camera.load();
			if (Base != nullptr)
				return Base;

			Base = GetComponent(Components::Camera::GetTypeId(), 0);
			if (!Base || !Base->Active)
			{
				Entity* Next = new Entity(this);
				Base = Next->AddComponent<Components::Camera>();
				AddEntity(Next);
				SetCamera(Next);
			}
			else
			{
				Transaction([this, Base]()
				{
					Base->Activate(Base);
					Camera = Base;
				});
			}

			return Base;
		}
		Component* SceneGraph::GetComponent(uint64_t Section, size_t Component)
		{
			auto& Array = GetComponents(Section);
			if (Component >= Array.Size())
				return nullptr;

			return *Array.At(Component);
		}
		RenderSystem* SceneGraph::GetRenderer()
		{
			auto* Viewer = (Components::Camera*)Camera.load();
			if (!Viewer)
				return nullptr;

			return Viewer->GetRenderer();
		}
		Viewer SceneGraph::GetCameraViewer() const
		{
			auto* Result = (Components::Camera*)Camera.load();
			if (!Result)
				return Viewer();

			return Result->GetViewer();
		}
		SparseIndex& SceneGraph::GetStorage(uint64_t Section)
		{
			SparseIndex* Storage = Registry[Section];
			VI_ASSERT(Storage != nullptr, "component should be registered by composer");
			if (Storage->Data.Size() + Conf.GrowMargin > Storage->Data.Capacity())
			{
				Transaction([this, Section]()
				{
					SparseIndex* Storage = Registry[Section];
					if (Storage->Data.Size() + Conf.GrowMargin > Storage->Data.Capacity())
						UpgradeBufferByRate(Storage->Data, (float)Conf.GrowRate);
				});
			}

			return *Storage;
		}
		Material* SceneGraph::GetMaterial(const std::string_view& Name)
		{
			auto Begin = Materials.Begin(), End = Materials.End();
			for (auto It = Begin; It != End; ++It)
			{
				if ((*It)->Name == Name)
					return *It;
			}

			return nullptr;
		}
		Material* SceneGraph::GetMaterial(size_t Material)
		{
			VI_ASSERT(Material < Materials.Size(), "index outside of range");
			return Materials[Material];
		}
		Entity* SceneGraph::GetEntity(size_t Entity)
		{
			VI_ASSERT(Entity < Entities.Size(), "index outside of range");
			return Entities[Entity];
		}
		Entity* SceneGraph::GetLastEntity()
		{
			if (Entities.Empty())
				return nullptr;

			return Entities.Back();
		}
		Entity* SceneGraph::GetCameraEntity()
		{
			Component* Data = GetCamera();
			return Data ? Data->Parent : nullptr;
		}
		Entity* SceneGraph::CloneEntityInstance(Entity* Entity)
		{
			VI_ASSERT(Entity != nullptr, "entity should be set");
			VI_MEASURE(Core::Timings::Pass);

			Layer::Entity* Instance = new Layer::Entity(this);
			Instance->Transform->Copy(Entity->Transform);
			Instance->Transform->UserData = Instance;
			Instance->Type.Name = Entity->Type.Name;
			Instance->Type.Components = Entity->Type.Components;

			for (auto& It : Instance->Type.Components)
			{
				Component* Source = It.second;
				It.second = Source->Copy(Instance);
				It.second->Parent = Instance;
				It.second->Active = Source->Active;
			}

			AddEntity(Instance);
			return Instance;
		}
		Entity* SceneGraph::CloneEntity(Entity* Value)
		{
			auto Array = CloneEntityAsArray(Value);
			return Array.empty() ? nullptr : Array.front();
		}
		Core::Pool<Component*>& SceneGraph::GetComponents(uint64_t Section)
		{
			SparseIndex& Storage = GetStorage(Section);
			return Storage.Data;
		}
		Core::Pool<Component*>& SceneGraph::GetActors(ActorType Type)
		{
			auto& Storage = Actors[(size_t)Type];
			if (Storage.Size() + Conf.GrowMargin > Storage.Capacity())
			{
				Transaction([this, Type]()
				{
					auto& Storage = Actors[(size_t)Type];
					if (Storage.Size() + Conf.GrowMargin > Storage.Capacity())
						UpgradeBufferByRate(Storage, (float)Conf.GrowRate);
				});
			}

			return Storage;
		}
		Graphics::RenderTarget2D::Desc SceneGraph::GetDescRT() const
		{
			auto* Device = Conf.Shared.Device;
			VI_ASSERT(Device != nullptr, "graphics device should be set");
			Graphics::RenderTarget2D* Target = Device->GetRenderTarget();

			VI_ASSERT(Target != nullptr, "render target should be set");
			Graphics::RenderTarget2D::Desc Desc;
			Desc.MiscFlags = Graphics::ResourceMisc::Generate_Mips;
			Desc.Width = (uint32_t)(Target->GetWidth() * Conf.RenderQuality);
			Desc.Height = (uint32_t)(Target->GetHeight() * Conf.RenderQuality);
			Desc.MipLevels = Device->GetMipLevel(Desc.Width, Desc.Height);
			Desc.FormatMode = GetFormatMRT(0);

			return Desc;
		}
		Graphics::MultiRenderTarget2D::Desc SceneGraph::GetDescMRT() const
		{
			auto* Device = Conf.Shared.Device;
			VI_ASSERT(Device != nullptr, "graphics device should be set");
			Graphics::RenderTarget2D* Target = Device->GetRenderTarget();

			VI_ASSERT(Target != nullptr, "render target should be set");
			Graphics::MultiRenderTarget2D::Desc Desc;
			Desc.MiscFlags = Graphics::ResourceMisc::Generate_Mips;
			Desc.Width = (uint32_t)(Target->GetWidth() * Conf.RenderQuality);
			Desc.Height = (uint32_t)(Target->GetHeight() * Conf.RenderQuality);
			Desc.MipLevels = Device->GetMipLevel(Desc.Width, Desc.Height);
			Desc.Target = Graphics::SurfaceTarget::T3;
			Desc.FormatMode[0] = GetFormatMRT(0);
			Desc.FormatMode[1] = GetFormatMRT(1);
			Desc.FormatMode[2] = GetFormatMRT(2);
			Desc.FormatMode[3] = GetFormatMRT(3);

			return Desc;
		}
		Graphics::Format SceneGraph::GetFormatMRT(uint32_t Target) const
		{
			if (Target == 0)
				return Conf.EnableHDR ? Graphics::Format::R16G16B16A16_Unorm : Graphics::Format::R8G8B8A8_Unorm;

			if (Target == 1)
				return Graphics::Format::R16G16B16A16_Float;

			if (Target == 2)
				return Graphics::Format::R32_Float;

			if (Target == 3)
				return Graphics::Format::R8G8B8A8_Unorm;

			return Graphics::Format::Unknown;
		}
		Core::Vector<Entity*> SceneGraph::CloneEntityAsArray(Entity* Value)
		{
			Core::Vector<Entity*> Array;
			VI_ASSERT(Value != nullptr, "entity should be set");
			CloneEntities(Value, &Array);
			return Array;
		}
		Core::Vector<Entity*> SceneGraph::QueryByParent(Entity* Entity) const
		{
			Core::Vector<Layer::Entity*> Array;
			VI_ASSERT(Entity != nullptr, "entity should be set");

			auto Begin = Entities.Begin(), End = Entities.End();
			for (auto It = Begin; It != End; ++It)
			{
				if (*It != Entity && !(*It)->Transform->HasRoot(Entity->Transform))
					Array.push_back(*It);
			}

			return Array;
		}
		Core::Vector<Entity*> SceneGraph::QueryByName(const std::string_view& Name) const
		{
			Core::Vector<Entity*> Array;
			auto Begin = Entities.Begin(), End = Entities.End();
			for (auto It = Begin; It != End; ++It)
			{
				if ((*It)->Type.Name == Name)
					Array.push_back(*It);
			}

			return Array;
		}
		Core::Vector<Component*> SceneGraph::QueryByPosition(uint64_t Section, const Trigonometry::Vector3& Position, float Radius)
		{
			return QueryByArea(Section, Position - Radius * 0.5f, Position + Radius * 0.5f);
		}
		Core::Vector<Component*> SceneGraph::QueryByArea(uint64_t Section, const Trigonometry::Vector3& Min, const Trigonometry::Vector3& Max)
		{
			Core::Vector<Component*> Result;
			Trigonometry::Bounding Target(Min, Max);
			Trigonometry::Cosmos::Iterator Context;
			auto& Storage = GetStorage(Section);
			Storage.Index.QueryIndex<Component>(Context, [&Target](const Trigonometry::Bounding& Bounds)
			{
				return Target.Overlaps(Bounds);
			}, [&Result](Component* Item)
			{
				Result.push_back(Item);
			});

			return Result;
		}
		Core::Vector<Component*> SceneGraph::QueryByMatch(uint64_t Section, std::function<bool(const Trigonometry::Bounding&)>&& MatchCallback)
		{
			Core::Vector<Component*> Result;
			Trigonometry::Cosmos::Iterator Context;
			auto& Storage = GetStorage(Section);
			auto Enqueue = [&Result](Component* Item) { Result.push_back(Item); };
			Storage.Index.QueryIndex<Component>(Context, std::move(MatchCallback), std::move(Enqueue));

			return Result;
		}
		Core::Vector<std::pair<Component*, Trigonometry::Vector3>> SceneGraph::QueryByRay(uint64_t Section, const Trigonometry::Ray& Origin)
		{
			typedef std::pair<Component*, Trigonometry::Vector3> RayHit;
			Core::Vector<RayHit> Result;
			Trigonometry::Ray Target = Origin;
			Trigonometry::Cosmos::Iterator Context;
			auto& Storage = GetStorage(Section);
			Storage.Index.QueryIndex<Component>(Context, [&Target](const Trigonometry::Bounding& Bounds)
			{
				return Target.IntersectsAABBAt(Bounds.Lower, Bounds.Upper, nullptr);
			}, [&Result](Component* Item)
			{
				Result.emplace_back(Item, Trigonometry::Vector3::Zero());
			});

			for (auto It = Result.begin(); It != Result.end();)
			{
				if (!Trigonometry::Geometric::CursorRayTest(Target, It->first->Parent->Snapshot.Box, &It->second))
					It = Result.erase(It);
				else
					++It;
			}

			VI_SORT(Result.begin(), Result.end(), [&Target](RayHit& A, RayHit& B)
			{
				float D1 = Target.Origin.Distance(A.first->Parent->Transform->GetPosition());
				float D2 = Target.Origin.Distance(B.first->Parent->Transform->GetPosition());
				return D1 < D2;
			});

			return Result;
		}
		Core::Vector<CubicDepthMap*>& SceneGraph::GetPointsMapping()
		{
			return Display.Points;
		}
		Core::Vector<LinearDepthMap*>& SceneGraph::GetSpotsMapping()
		{
			return Display.Spots;
		}
		Core::Vector<CascadedDepthMap*>& SceneGraph::GetLinesMapping()
		{
			return Display.Lines;
		}
		Core::Vector<VoxelMapping>& SceneGraph::GetVoxelsMapping()
		{
			return Display.Voxels;
		}
		const Core::UnorderedMap<uint64_t, SparseIndex*>& SceneGraph::GetRegistry() const
		{
			return Registry;
		}
		Core::String SceneGraph::AsResourcePath(const std::string_view& Path)
		{
			VI_ASSERT(Conf.Shared.Content != nullptr, "content manager should be set");
			Core::String RelPath = Core::String(Path);
			Core::Stringify::Replace(RelPath, Conf.Shared.Content->GetEnvironment(), "./");
			Core::Stringify::Replace(RelPath, '\\', '/');
			return RelPath;
		}
		Entity* SceneGraph::AddEntity()
		{
			Entity* Next = new Entity(this);
			AddEntity(Next);
			return Next;
		}
		bool SceneGraph::AddEntity(Entity* Entity)
		{
			VI_ASSERT(Entity != nullptr, "entity should be set");
			VI_ASSERT(Entity->Scene == this, "entity should be created for this scene");

			if (Entities.Size() + Conf.GrowMargin <= Entities.Capacity())
			{
				Entities.Add(Entity);
				RegisterEntity(Entity);
				return true;
			}
			
			Transaction([this, Entity]()
			{
				if (Entities.Size() + Conf.GrowMargin > Entities.Capacity())
					UpgradeBufferByRate(Entities, (float)Conf.GrowRate);
				AddEntity(Entity);
			});

			return true;
		}
		bool SceneGraph::HasEntity(Entity* Entity) const
		{
			VI_ASSERT(Entity != nullptr, "entity should be set");
			for (size_t i = 0; i < Entities.Size(); i++)
			{
				if (Entities[i] == Entity)
					return true;
			}

			return false;
		}
		bool SceneGraph::HasEntity(size_t Entity) const
		{
			return Entity < Entities.Size() ? Entity : -1;
		}
		bool SceneGraph::IsActive() const
		{
			return Active;
		}
		size_t SceneGraph::GetEntitiesCount() const
		{
			return Entities.Size();
		}
		size_t SceneGraph::GetComponentsCount(uint64_t Section)
		{
			return GetComponents(Section).Size();
		}
		size_t SceneGraph::GetMaterialsCount() const
		{
			return Materials.Size();
		}
		Graphics::MultiRenderTarget2D* SceneGraph::GetMRT(TargetType Type) const
		{
			return Display.MRT[(size_t)Type];
		}
		Graphics::RenderTarget2D* SceneGraph::GetRT(TargetType Type) const
		{
			return Display.RT[(size_t)Type];
		}
		Graphics::Texture2D** SceneGraph::GetMerger()
		{
			return &Display.Merger;
		}
		Graphics::ElementBuffer* SceneGraph::GetStructure() const
		{
			return Display.MaterialBuffer;
		}
		Graphics::GraphicsDevice* SceneGraph::GetDevice() const
		{
			return Conf.Shared.Device;
		}
		Graphics::Activity* SceneGraph::GetActivity() const
		{
			return Conf.Shared.Activity;
		}
		RenderConstants* SceneGraph::GetConstants() const
		{
			return Conf.Shared.Constants;
		}
		ShaderCache* SceneGraph::GetShaders() const
		{
			return Conf.Shared.Shaders;
		}
		PrimitiveCache* SceneGraph::GetPrimitives() const
		{
			return Conf.Shared.Primitives;
		}
		Physics::Simulator* SceneGraph::GetSimulator() const
		{
			return Simulator;
		}
		SceneGraph::Desc& SceneGraph::GetConf()
		{
			return Conf;
		}

		void HeavyContentManager::SetDevice(Graphics::GraphicsDevice* NewDevice)
		{
			Device = NewDevice;
		}
		Graphics::GraphicsDevice* HeavyContentManager::GetDevice() const
		{
			return Device;
		}

		HeavyApplication::HeavyApplication(Desc* I) noexcept : Control(I ? *I : Desc())
		{
			VI_ASSERT(I != nullptr, "desc should be set");
			Control.Usage |= Control.AdvancedUsage;
			State = ApplicationState::Staging;
		}
		HeavyApplication::~HeavyApplication() noexcept
		{
			if (Renderer != nullptr)
				Renderer->FlushState();

			Core::Memory::Release(InternalUI);
			if (Layer::GUI::Subsystem::HasInstance())
				Layer::GUI::Subsystem::Get()->CleanupShared();

			Core::Memory::Release(Scene);
			Core::Memory::Release(VM);
			Core::Memory::Release(Audio);
			Core::Memory::Release(Cache.Shaders);
			Core::Memory::Release(Cache.Primitives);
			Core::Memory::Release(Content);
			Core::Memory::Release(Constants);
			Core::Memory::Release(Renderer);
			Core::Memory::Release(Activity);
			Core::Memory::Release(InternalClock);

			HeavyApplication::UnlinkInstance(this);
			Vitex::HeavyRuntime::CleanupInstances();
		}
		Core::Promise<void> HeavyApplication::Startup()
		{
			return Core::Promise<void>::Null();
		}
		Core::Promise<void> HeavyApplication::Shutdown()
		{
			return Core::Promise<void>::Null();
		}
		void HeavyApplication::ScriptHook()
		{
		}
		void HeavyApplication::KeyEvent(Graphics::KeyCode Key, Graphics::KeyMod Mod, int Virtual, int Repeat, bool Pressed)
		{
		}
		void HeavyApplication::InputEvent(char* Buffer, size_t Length)
		{
		}
		void HeavyApplication::WheelEvent(int X, int Y, bool Normal)
		{
		}
		void HeavyApplication::WindowEvent(Graphics::WindowState NewState, int X, int Y)
		{
		}
		void HeavyApplication::Composition()
		{
		}
		void HeavyApplication::Dispatch(Core::Timer* Time)
		{
		}
		void HeavyApplication::Publish(Core::Timer* Time)
		{
		}
		void HeavyApplication::Initialize()
		{
		}
		void HeavyApplication::LoopTrigger()
		{
			VI_MEASURE(Core::Timings::Infinite);
			Core::Schedule::Desc& Policy = Control.Scheduler;
			Core::Schedule* Queue = Core::Schedule::Get();
			if (Policy.Parallel)
			{
				if (Activity != nullptr)
				{
					while (State == ApplicationState::Active)
					{
						bool RenderFrame = Activity->Dispatch(0, Control.BlockingDispatch);
						InternalClock->Begin();
						Dispatch(InternalClock);

						InternalClock->Finish();
						if (RenderFrame)
							Publish(InternalClock);
					}
				}
				else
				{
					while (State == ApplicationState::Active)
					{
						InternalClock->Begin();
						Dispatch(InternalClock);

						InternalClock->Finish();
						Publish(InternalClock);
					}
				}

				while (Content && Content->IsBusy())
					std::this_thread::sleep_for(std::chrono::milliseconds(CONTENT_BLOCKED_WAIT_MS));
			}
			else
			{
				if (Activity != nullptr)
				{
					while (State == ApplicationState::Active)
					{
						bool RenderFrame = Activity->Dispatch(0, Control.BlockingDispatch);
						Queue->Dispatch();

						InternalClock->Begin();
						Dispatch(InternalClock);

						InternalClock->Finish();
						if (RenderFrame)
							Publish(InternalClock);
					}
				}
				else
				{
					while (State == ApplicationState::Active)
					{
						Queue->Dispatch();

						InternalClock->Begin();
						Dispatch(InternalClock);

						InternalClock->Finish();
						Publish(InternalClock);
					}
				}
			}
		}
		int HeavyApplication::Start()
		{
			Composition();
			Compose();

			if (Control.Usage & USE_PROCESSING)
			{
				if (!Content)
					Content = new HeavyContentManager();

				Content->AddProcessor<Processors::AssetProcessor, Layer::AssetFile>();
				Content->AddProcessor<Processors::MaterialProcessor, Layer::Material>();
				Content->AddProcessor<Processors::SceneGraphProcessor, Layer::SceneGraph>();
				Content->AddProcessor<Processors::AudioClipProcessor, Audio::AudioClip>();
				Content->AddProcessor<Processors::Texture2DProcessor, Graphics::Texture2D>();
				Content->AddProcessor<Processors::ShaderProcessor, Graphics::Shader>();
				Content->AddProcessor<Processors::ModelProcessor, Model>();
				Content->AddProcessor<Processors::SkinModelProcessor, SkinModel>();
				Content->AddProcessor<Processors::SkinAnimationProcessor, Layer::SkinAnimation>();
				Content->AddProcessor<Processors::SchemaProcessor, Core::Schema>();
				Content->AddProcessor<Processors::ServerProcessor, Network::HTTP::Server>();
				Content->AddProcessor<Processors::HullShapeProcessor, Physics::HullShape>();

				if (Control.Environment.empty())
				{
					auto Directory = Core::OS::Directory::GetWorking();
					if (Directory)
						Content->SetEnvironment(*Directory + Control.Directory);
				}
				else
					Content->SetEnvironment(Control.Environment + Control.Directory);
				
				if (!Control.Preferences.empty() && !Database)
				{
					auto Path = Core::OS::Path::Resolve(Control.Preferences, Content->GetEnvironment(), true);
					Database = new AppData(Content, Path ? *Path : Control.Preferences);
				}
			}

			if (Control.Usage & USE_ACTIVITY)
			{
				if (!Control.Activity.Width || !Control.Activity.Height)
				{
					Graphics::DisplayInfo Info;
					VI_PANIC(Graphics::Video::GetDisplayInfo(0, &Info), "video driver is not initialized");
					Control.Activity.Width = (uint32_t)(Info.PhysicalWidth / 1.1);
					Control.Activity.Height = (uint32_t)(Info.PhysicalHeight / 1.2);
				}
				
				VI_PANIC(Control.Activity.Width > 0 && Control.Activity.Height > 0, "activity width/height is not acceptable");
				bool Maximized = Control.Activity.Maximized;
				Control.Activity.GPUAsRenderer = (Control.Usage & USE_GRAPHICS);
				Control.Activity.Maximized = false;

				if (!Activity)
					Activity = new Graphics::Activity(Control.Activity);

				if (Control.Activity.GPUAsRenderer)
				{
					VI_PANIC(!Renderer, "graphics device is pre-initialized which is not compatible with GPU as renderer mode");
					Control.GraphicsDevice.BufferWidth = Control.Activity.Width;
					Control.GraphicsDevice.BufferHeight = Control.Activity.Height;
					Control.GraphicsDevice.Window = Activity;

					if (Content != nullptr && !Control.GraphicsDevice.CacheDirectory.empty())
					{
						auto Directory = Core::OS::Path::ResolveDirectory(Control.GraphicsDevice.CacheDirectory, Content->GetEnvironment(), false);
						if (Directory)
						{
							Control.GraphicsDevice.CacheDirectory = *Directory;
							Core::OS::Directory::Patch(Control.GraphicsDevice.CacheDirectory);
						}
					}

					Renderer = Graphics::GraphicsDevice::Create(Control.GraphicsDevice);
					VI_PANIC(Renderer && Renderer->IsValid(), "video driver is not initialized");
					Trigonometry::Geometric::SetLeftHanded(Renderer->IsLeftHanded());
					if (Content != nullptr)
						Content->SetDevice(Renderer);

					if (!Cache.Shaders)
						Cache.Shaders = new ShaderCache(Renderer);

					if (!Cache.Primitives)
						Cache.Primitives = new PrimitiveCache(Renderer);

					if (!Constants)
						Constants = new RenderConstants(Renderer);
				}
					
				VI_PANIC(Activity->GetHandle() != nullptr, "activity instance is non-existant");
				Activity->UserPointer = this;
				Activity->SetCursorVisibility(Control.Cursor);
				Activity->Callbacks.KeyState = [this](Graphics::KeyCode Key, Graphics::KeyMod Mod, int Virtual, int Repeat, bool Pressed)
				{
					GUI::Context* UI = TryGetUI();
					if (UI != nullptr)
						UI->EmitKey(Key, Mod, Virtual, Repeat, Pressed);
					KeyEvent(Key, Mod, Virtual, Repeat, Pressed);
				};
				Activity->Callbacks.Input = [this](char* Buffer, int Length)
				{
					if (!Buffer)
						return;

					GUI::Context* UI = TryGetUI();
					if (UI != nullptr)
						UI->EmitInput(Buffer, Length);
					InputEvent(Buffer, Length < 0 ? strlen(Buffer) : (size_t)Length);
				};
				Activity->Callbacks.CursorWheelState = [this](int X, int Y, bool Normal)
				{
					GUI::Context* UI = TryGetUI();
					if (UI != nullptr)
						UI->EmitWheel(X, Y, Normal, Activity->GetKeyModState());
					WheelEvent(X, Y, Normal);
				};
				Activity->Callbacks.WindowStateChange = [this](Graphics::WindowState NewState, int X, int Y)
				{
					if (NewState == Graphics::WindowState::Resize)
					{
						GUI::Context* UI = TryGetUI();
						if (UI != nullptr)
							UI->EmitResize(X, Y);
					}
					WindowEvent(NewState, X, Y);
				};
				Control.Activity.Maximized = Maximized;
			}

			if (Control.Usage & USE_AUDIO && !Audio)
				Audio = new Audio::AudioDevice();

			if (Control.Usage & USE_SCRIPTING && !VM)
				VM = new Scripting::VirtualMachine();

			InternalClock = new Core::Timer();
			InternalClock->SetFixedFrames(Control.Refreshrate.Stable);
			InternalClock->SetMaxFrames(Control.Refreshrate.Limit);

			if (Control.Usage & USE_NETWORKING)
			{
				if (Network::Multiplexer::HasInstance())
					Network::Multiplexer::Get()->Rescale(Control.PollingTimeout, Control.PollingEvents);
				else
					new Network::Multiplexer(Control.PollingTimeout, Control.PollingEvents);
			}

			if (Control.Usage & USE_SCRIPTING)
				ScriptHook();

			Initialize();
			if (State == ApplicationState::Terminated)
				return ExitCode != 0 ? ExitCode : EXIT_JUMP + 6;

			State = ApplicationState::Active;
			if (Activity != nullptr)
			{
				Activity->Show();
				if (Control.Activity.Maximized)
					Activity->Maximize();
			}

			Core::Schedule::Desc& Policy = Control.Scheduler;
			Policy.Initialize = [this](Core::TaskCallback&& Callback) { this->Startup().When(std::move(Callback)); };
			Policy.Ping = Control.Daemon ? std::bind(&HeavyApplication::Status, this) : (Core::ActivityCallback)nullptr;

			if (Control.Threads > 0)
			{
				Core::Schedule::Desc Launch = Core::Schedule::Desc(Control.Threads);
				memcpy(Policy.Threads, Launch.Threads, sizeof(Policy.Threads));
			}

			auto* Queue = Core::Schedule::Get();
			Queue->Start(Policy);
			InternalClock->Reset();
			LoopTrigger();
			Shutdown().Wait();
			Queue->Stop();

			ExitCode = (State == ApplicationState::Restart ? EXIT_RESTART : ExitCode);
			State = ApplicationState::Terminated;
			return ExitCode;
		}
		void HeavyApplication::Stop(int Code)
		{
			Core::Schedule* Queue = Core::Schedule::Get();
			State = ApplicationState::Terminated;
			ExitCode = Code;
			Queue->Wakeup();
		}
		void HeavyApplication::Restart()
		{
			Core::Schedule* Queue = Core::Schedule::Get();
			State = ApplicationState::Restart;
			Queue->Wakeup();
		}
		GUI::Context* HeavyApplication::TryGetUI() const
		{
			return InternalUI;
		}
		GUI::Context* HeavyApplication::FetchUI()
		{
			if (!InternalUI && Activity != nullptr && Renderer != nullptr && Constants != nullptr && Content != nullptr)
			{
				GUI::Subsystem::Get()->SetShared(VM, Activity, Constants, Content, InternalClock);
				InternalUI = new GUI::Context(Renderer);
			}
			return InternalUI;
		}
		ApplicationState HeavyApplication::GetState() const
		{
			return State;
		}
		bool HeavyApplication::Status(HeavyApplication* App)
		{
			return App->State == ApplicationState::Active;
		}
		void HeavyApplication::Compose()
		{
			static bool WasComposed = false;
			if (WasComposed)
				return;
			else
				WasComposed = true;

			uint64_t AsComponent = (uint64_t)ComposerTag::Component;
			Core::Composer::Push<Components::RigidBody, Entity*>(AsComponent);
			Core::Composer::Push<Components::SoftBody, Entity*>(AsComponent);
			Core::Composer::Push<Components::Acceleration, Entity*>(AsComponent);
			Core::Composer::Push<Components::SliderConstraint, Entity*>(AsComponent);
			Core::Composer::Push<Components::Model, Entity*>(AsComponent);
			Core::Composer::Push<Components::Skin, Entity*>(AsComponent);
			Core::Composer::Push<Components::Emitter, Entity*>(AsComponent);
			Core::Composer::Push<Components::Decal, Entity*>(AsComponent);
			Core::Composer::Push<Components::SkinAnimator, Entity*>(AsComponent);
			Core::Composer::Push<Components::KeyAnimator, Entity*>(AsComponent);
			Core::Composer::Push<Components::EmitterAnimator, Entity*>(AsComponent);
			Core::Composer::Push<Components::FreeLook, Entity*>(AsComponent);
			Core::Composer::Push<Components::Fly, Entity*>(AsComponent);
			Core::Composer::Push<Components::AudioSource, Entity*>(AsComponent);
			Core::Composer::Push<Components::AudioListener, Entity*>(AsComponent);
			Core::Composer::Push<Components::PointLight, Entity*>(AsComponent);
			Core::Composer::Push<Components::SpotLight, Entity*>(AsComponent);
			Core::Composer::Push<Components::LineLight, Entity*>(AsComponent);
			Core::Composer::Push<Components::SurfaceLight, Entity*>(AsComponent);
			Core::Composer::Push<Components::Illuminator, Entity*>(AsComponent);
			Core::Composer::Push<Components::Camera, Entity*>(AsComponent);
			Core::Composer::Push<Components::Scriptable, Entity*>(AsComponent);

			uint64_t AsRenderer = (uint64_t)ComposerTag::Renderer;
			Core::Composer::Push<Renderers::SoftBody, RenderSystem*>(AsRenderer);
			Core::Composer::Push<Renderers::Model, RenderSystem*>(AsRenderer);
			Core::Composer::Push<Renderers::Skin, RenderSystem*>(AsRenderer);
			Core::Composer::Push<Renderers::Emitter, RenderSystem*>(AsRenderer);
			Core::Composer::Push<Renderers::Decal, RenderSystem*>(AsRenderer);
			Core::Composer::Push<Renderers::Lighting, RenderSystem*>(AsRenderer);
			Core::Composer::Push<Renderers::Transparency, RenderSystem*>(AsRenderer);
			Core::Composer::Push<Renderers::Glitch, RenderSystem*>(AsRenderer);
			Core::Composer::Push<Renderers::Tone, RenderSystem*>(AsRenderer);
			Core::Composer::Push<Renderers::DoF, RenderSystem*>(AsRenderer);
			Core::Composer::Push<Renderers::Bloom, RenderSystem*>(AsRenderer);
			Core::Composer::Push<Renderers::SSR, RenderSystem*>(AsRenderer);
			Core::Composer::Push<Renderers::SSGI, RenderSystem*>(AsRenderer);
			Core::Composer::Push<Renderers::SSAO, RenderSystem*>(AsRenderer);
			Core::Composer::Push<Renderers::MotionBlur, RenderSystem*>(AsRenderer);
			Core::Composer::Push<Renderers::UserInterface, RenderSystem*>(AsRenderer);

			uint64_t AsEffect = (uint64_t)ComposerTag::Effect;
			Core::Composer::Push<Audio::Effects::Reverb>(AsEffect);
			Core::Composer::Push<Audio::Effects::Chorus>(AsEffect);
			Core::Composer::Push<Audio::Effects::Distortion>(AsEffect);
			Core::Composer::Push<Audio::Effects::Echo>(AsEffect);
			Core::Composer::Push<Audio::Effects::Flanger>(AsEffect);
			Core::Composer::Push<Audio::Effects::FrequencyShifter>(AsEffect);
			Core::Composer::Push<Audio::Effects::VocalMorpher>(AsEffect);
			Core::Composer::Push<Audio::Effects::PitchShifter>(AsEffect);
			Core::Composer::Push<Audio::Effects::RingModulator>(AsEffect);
			Core::Composer::Push<Audio::Effects::Autowah>(AsEffect);
			Core::Composer::Push<Audio::Effects::Compressor>(AsEffect);
			Core::Composer::Push<Audio::Effects::Equalizer>(AsEffect);

			uint64_t AsFilter = (uint64_t)ComposerTag::Filter;
			Core::Composer::Push<Audio::Filters::Lowpass>(AsFilter);
			Core::Composer::Push<Audio::Filters::Bandpass>(AsFilter);
			Core::Composer::Push<Audio::Filters::Highpass>(AsFilter);
		}

		EffectRenderer::EffectRenderer(RenderSystem* Lab) noexcept : Renderer(Lab), Output(nullptr), Swap(nullptr), MaxSlot(0)
		{
			VI_ASSERT(Lab != nullptr, "render system should be set");
			VI_ASSERT(Lab->GetDevice() != nullptr, "graphics device should be set");

			auto* Device = Lab->GetDevice();
			DepthStencil = Device->GetDepthStencilState("doo_soo_lt");
			Rasterizer = Device->GetRasterizerState("so_cback");
			Blend = Device->GetBlendState("bo_wrgbo_one");
			SamplerWrap = Device->GetSamplerState("a16_fa_wrap");
			SamplerClamp = Device->GetSamplerState("a16_fa_clamp");
			SamplerMirror = Device->GetSamplerState("a16_fa_mirror");
			Layout = Device->GetInputLayout("vx_shape");
		}
		EffectRenderer::~EffectRenderer() noexcept
		{
			for (auto It = Effects.begin(); It != Effects.end(); ++It)
				System->FreeShader(It->first, It->second);
		}
		void EffectRenderer::ResizeBuffers()
		{
			Output = nullptr;
			ResizeEffect();
		}
		void EffectRenderer::ResizeEffect()
		{
		}
		void EffectRenderer::RenderCopyMain(uint32_t Slot, Graphics::Texture2D* Target)
		{
			VI_ASSERT(Target != nullptr, "texture should be set");
			Graphics::GraphicsDevice* Device = System->GetDevice();
			Device->CopyTexture2D(System->GetScene()->GetMRT(TargetType::Main), Slot, &Target);
		}
		void EffectRenderer::RenderCopyLast(Graphics::Texture2D* Target)
		{
			VI_ASSERT(Target != nullptr, "texture should be set");
			Graphics::GraphicsDevice* Device = System->GetDevice();
			Device->CopyTexture2D(Output, 0, &Target);
		}
		void EffectRenderer::RenderOutput(Graphics::RenderTarget2D* Resource)
		{
			VI_ASSERT(System->GetDevice() != nullptr, "graphics device should be set");
			if (Resource != nullptr)
			{
				Output = Resource;
				Swap = Resource;
			}
			else
				Output = System->GetRT(TargetType::Main);

			Graphics::GraphicsDevice* Device = System->GetDevice();
			Device->SetTarget(Output, 0, 0, 0, 0);
		}
		void EffectRenderer::RenderTexture(uint32_t Slot6, Graphics::Texture2D* Resource)
		{
			Graphics::GraphicsDevice* Device = System->GetDevice();
			Device->SetTexture2D(Resource, 6 + Slot6, VI_PS);

			if (Resource != nullptr)
				MaxSlot = std::max(MaxSlot, 6 + Slot6);
		}
		void EffectRenderer::RenderTexture(uint32_t Slot6, Graphics::Texture3D* Resource)
		{
			Graphics::GraphicsDevice* Device = System->GetDevice();
			Device->SetTexture3D(Resource, 6 + Slot6, VI_PS);

			if (Resource != nullptr)
				MaxSlot = std::max(MaxSlot, 6 + Slot6);
		}
		void EffectRenderer::RenderTexture(uint32_t Slot6, Graphics::TextureCube* Resource)
		{
			Graphics::GraphicsDevice* Device = System->GetDevice();
			Device->SetTextureCube(Resource, 6 + Slot6, VI_PS);

			if (Resource != nullptr)
				MaxSlot = std::max(MaxSlot, 6 + Slot6);
		}
		void EffectRenderer::RenderMerge(Graphics::Shader* Effect, void* Buffer, size_t Count)
		{
			VI_ASSERT(Count > 0, "count should be greater than zero");
			if (!Effect)
				Effect = Effects.begin()->second;

			Graphics::GraphicsDevice* Device = System->GetDevice();
			Graphics::Texture2D** Merger = System->GetMerger();

			if (Swap != nullptr && Output != Swap)
			{
				Device->SetTexture2D(nullptr, 5, VI_PS);
				Device->SetTexture2D(Swap->GetTarget(), 5, VI_PS);
			}
			else if (Merger != nullptr)
			{
				Device->SetTexture2D(nullptr, 5, VI_PS);
				Device->SetTexture2D(*Merger, 5, VI_PS);
			}

			Device->SetShader(Effect, VI_VS | VI_PS);
			if (Buffer != nullptr)
			{
				Device->UpdateBuffer(Effect, Buffer);
				Device->SetBuffer(Effect, 3, VI_VS | VI_PS);
			}

			for (size_t i = 0; i < Count; i++)
			{
				Device->Draw(6, 0);
				if (!Swap)
					Device->CopyTexture2D(Output, 0, Merger);
			}

			auto* Scene = System->GetScene();
			Scene->Statistics.DrawCalls += Count;

			if (Swap == Output)
				RenderOutput();
		}
		void EffectRenderer::RenderResult(Graphics::Shader* Effect, void* Buffer)
		{
			if (!Effect)
				Effect = Effects.begin()->second;

			Graphics::GraphicsDevice* Device = System->GetDevice();
			Graphics::Texture2D** Merger = System->GetMerger();

			if (Swap != nullptr && Output != Swap)
			{
				Device->SetTexture2D(nullptr, 5, VI_PS);
				Device->SetTexture2D(Swap->GetTarget(), 5, VI_PS);
			}
			else if (Merger != nullptr)
			{
				Device->SetTexture2D(nullptr, 5, VI_PS);
				Device->SetTexture2D(*Merger, 5, VI_PS);
			}

			Device->SetShader(Effect, VI_VS | VI_PS);
			if (Buffer != nullptr)
			{
				Device->UpdateBuffer(Effect, Buffer);
				Device->SetBuffer(Effect, 3, VI_VS | VI_PS);
			}

			Device->Draw(6, 0);
			Output = System->GetRT(TargetType::Main);

			auto* Scene = System->GetScene();
			Scene->Statistics.DrawCalls++;
		}
		void EffectRenderer::RenderResult()
		{
			Graphics::GraphicsDevice* Device = System->GetDevice();
			Graphics::Texture2D** Merger = System->GetMerger();

			if (Swap != nullptr && Output != Swap)
				Device->SetTexture2D(Swap->GetTarget(), 1, VI_PS);
			else if (Merger != nullptr)
				Device->SetTexture2D(*Merger, 1, VI_PS);

			Device->SetShader(System->GetBasicEffect(), VI_VS | VI_PS);
			Device->Draw(6, 0);
			Output = System->GetRT(TargetType::Main);

			auto* Scene = System->GetScene();
			Scene->Statistics.DrawCalls++;
		}
		void EffectRenderer::RenderEffect(Core::Timer* Time)
		{
		}
		void EffectRenderer::SampleWrap()
		{
			Graphics::GraphicsDevice* Device = System->GetDevice();
			Device->SetSamplerState(SamplerWrap, 1, MaxSlot, VI_PS);
		}
		void EffectRenderer::SampleClamp()
		{
			Graphics::GraphicsDevice* Device = System->GetDevice();
			Device->SetSamplerState(SamplerClamp, 1, MaxSlot, VI_PS);
		}
		void EffectRenderer::SampleMirror()
		{
			Graphics::GraphicsDevice* Device = System->GetDevice();
			Device->SetSamplerState(SamplerMirror, 1, MaxSlot, VI_PS);
		}
		void EffectRenderer::GenerateMips()
		{
			Graphics::GraphicsDevice* Device = System->GetDevice();
			Graphics::Texture2D** Merger = System->GetMerger();

			if (Swap != nullptr && Output != Swap)
				Device->GenerateMips(Swap->GetTarget());
			else if (Merger != nullptr)
				Device->GenerateMips(*Merger);
		}
		size_t EffectRenderer::RenderPass(Core::Timer* Time)
		{
			VI_ASSERT(System->GetPrimitives() != nullptr, "primitive cache should be set");
			VI_ASSERT(System->GetMRT(TargetType::Main) != nullptr, "main render target should be set");
			VI_MEASURE(Core::Timings::Pass);

			if (!System->State.Is(RenderState::Geometric) || System->State.IsSubpass())
				return 0;

			MaxSlot = 5;
			if (Effects.empty())
				return 0;

			Swap = nullptr;
			if (!Output)
				Output = System->GetRT(TargetType::Main);

			Graphics::MultiRenderTarget2D* Input = System->GetMRT(TargetType::Main);
			PrimitiveCache* Cache = System->GetPrimitives();
			Graphics::GraphicsDevice* Device = System->GetDevice();
			Device->SetDepthStencilState(DepthStencil);
			Device->SetBlendState(Blend);
			Device->SetRasterizerState(Rasterizer);
			Device->SetInputLayout(Layout);
			Device->SetTarget(Output, 0, 0, 0, 0);
			Device->SetSamplerState(SamplerWrap, 1, MaxSlot, VI_PS);
			Device->SetTexture2D(Input->GetTarget(0), 1, VI_PS);
			Device->SetTexture2D(Input->GetTarget(1), 2, VI_PS);
			Device->SetTexture2D(Input->GetTarget(2), 3, VI_PS);
			Device->SetTexture2D(Input->GetTarget(3), 4, VI_PS);
			Device->SetVertexBuffer(Cache->GetQuad());

			RenderEffect(Time);

			Device->FlushTexture(1, MaxSlot, VI_PS);
			Device->CopyTarget(Output, 0, Input, 0);
			System->RestoreOutput();
			return 1;
		}
		Graphics::Shader* EffectRenderer::GetEffect(const std::string_view& Name)
		{
			auto It = Effects.find(Core::KeyLookupCast(Name));
			if (It != Effects.end())
				return It->second;

			return nullptr;
		}
		Graphics::ExpectsGraphics<Graphics::Shader*> EffectRenderer::CompileEffect(Graphics::Shader::Desc& Desc, size_t BufferSize)
		{
			VI_ASSERT(!Desc.Filename.empty(), "cannot compile unnamed shader source");
			auto Shader = System->CompileShader(Desc, BufferSize);
			if (!Shader)
				return Shader.Error();

			auto It = Effects.find(Desc.Filename);
			if (It != Effects.end())
			{
				Core::Memory::Release(It->second);
				It->second = *Shader;
			}
			else
				Effects[Desc.Filename] = *Shader;

			return Shader;
		}
		Graphics::ExpectsGraphics<Graphics::Shader*> EffectRenderer::CompileEffect(const std::string_view& SectionName, size_t BufferSize)
		{
			Graphics::Shader::Desc I = Graphics::Shader::Desc();
			auto Status = System->GetDevice()->GetSectionData(SectionName, &I);
			if (!Status)
				return Status.Error();

			return CompileEffect(I, BufferSize);
		}
		uint32_t EffectRenderer::GetMipLevels() const
		{
			VI_ASSERT(System->GetRT(TargetType::Main) != nullptr, "main render target should be set");
			Graphics::RenderTarget2D* RT = System->GetRT(TargetType::Main);
			return System->GetDevice()->GetMipLevel(RT->GetWidth(), RT->GetHeight());
		}
		uint32_t EffectRenderer::GetWidth() const
		{
			VI_ASSERT(System->GetRT(TargetType::Main) != nullptr, "main render target should be set");
			Graphics::RenderTarget2D* RT = System->GetRT(TargetType::Main);
			return RT->GetWidth();
		}
		uint32_t EffectRenderer::GetHeight() const
		{
			VI_ASSERT(System->GetRT(TargetType::Main) != nullptr, "main render target should be set");
			Graphics::RenderTarget2D* RT = System->GetRT(TargetType::Main);
			return RT->GetHeight();
		}
	}
}
