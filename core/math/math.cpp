#include "math.h"

#include <algorithm>

#include "../utils/utils.h"
//used: getexportaddr
#include "memory.h"
#include "..\sdk\sdk.h"
#include "..\sdk\addresses\entity.h"

#include "..\math\elements\matrix.h"
#include "..\..\deps\imgui\imgui_internal.h"
bool math::Init()
{
	bool success = true;

	// get tier0 module base
	const HMODULE tier0_base = GetModuleHandle(xorstr_("tier0.dll"));
	if (tier0_base == nullptr)
		return false;

	// grab math functions from tier0 library
	RandomSeed = reinterpret_cast<decltype(RandomSeed)>(GetProcAddress(tier0_base, xorstr_("RandomSeed")));
	success &= (RandomSeed != nullptr);

	RandomFloat = reinterpret_cast<decltype(RandomFloat)>(GetProcAddress(tier0_base, xorstr_("RandomFloat")));
	success &= (RandomFloat != nullptr);

	RandomFloatExp = reinterpret_cast<decltype(RandomFloatExp)>(GetProcAddress(tier0_base, xorstr_("RandomFloatExp")));
	success &= (RandomFloatExp != nullptr);

	RandomInt = reinterpret_cast<decltype(RandomInt)>(GetProcAddress(tier0_base, xorstr_("RandomInt")));
	success &= (RandomInt != nullptr);

	RandomGaussianFloat = reinterpret_cast<decltype(RandomGaussianFloat)>(GetProcAddress(tier0_base, xorstr_("RandomGaussianFloat")));
	success &= (RandomGaussianFloat != nullptr);

	// return status
	return success;
}

// distance between to line segments
float math::segment_dist(Vector_t start1, Vector_t end1, Vector_t start2, Vector_t end2) noexcept {
	Vector_t u = end1 - start1;
	Vector_t v = end2 - start2;
	Vector_t w = start1 - start2;
	float a = u.DotProduct(u);
	float b = u.DotProduct(v);
	float c = v.DotProduct(v);
	float d = u.DotProduct(w);
	float e = v.DotProduct(w);
	float D = a * c - b * b;
	float sc, sN, sD = D;
	float tc, tN, tD = D;

	if (D < 0.001f) {
		sN = 0.0f;
		sD = 1.0f;
		tN = e;
		tD = c;
	}
	else {
		sN = (b * e - c * d);
		tN = (a * e - b * d);
		if (sN < 0.0f) {
			sN = 0.0f;
			tN = e;
			tD = c;
		}
		else if (sN > sD) {
			sN = sD;
			tN = e + b;
			tD = c;
		}
	}

	if (tN < 0.0f) {
		tN = 0.0f;

		if (-d < 0.0f) {
			sN = 0.0f;
		}
		else if (-d > a) {
			sN = sD;
		}
		else {
			sN = -d;
			sD = a;
		}
	}
	else if (tN > tD) {
		tN = tD;

		if ((-d + b) < 0.0f) {
			sN = 0;
		}
		else if ((-d + b) > a) {
			sN = sD;
		}
		else {
			sN = (-d + b);
			sD = a;
		}
	}

	sc = (std::abs(sN) < 0.001f ? 0.0f : sN / sD);
	tc = (std::abs(tN) < 0.001f ? 0.0f : tN / tD);

	Vector_t dP = w + (u * sc) - (v * tc);
	return dP.Length();
}

 void math::TransformAABB(const Matrix3x4_t& transform, const Vector_t& minsIn, const Vector_t& maxsIn, Vector_t& minsOut, Vector_t& maxsOut)
{
	const Vector_t localCenter = (minsIn + maxsIn) * 0.5f;
	const Vector_t localExtent = maxsIn - localCenter;

	const auto& mat = transform.arrData;
	const Vector_t worldAxisX{ mat[0][0], mat[0][1], mat[0][2] };
	const Vector_t worldAxisY{ mat[1][0], mat[1][1], mat[1][2] };
	const Vector_t worldAxisZ{ mat[2][0], mat[2][1], mat[2][2] };

	const Vector_t worldCenter = localCenter.Transform(transform);
	const Vector_t worldExtent{
		localExtent.dot_absolute(worldAxisX),
		localExtent.dot_absolute(worldAxisY),
		localExtent.dot_absolute(worldAxisZ),
	};

	minsOut = worldCenter - worldExtent;
	maxsOut = worldCenter + worldExtent;
}
#include <numbers>

void SinCos(float a, float* s, float* c) {
	*s = sin(a);
	*c = cos(a);
}
void math::AngleVectors(const QAngle_t& angles, Vector_t* forward) {
	float	sp, sy, cp, cy;

	SinCos(M_DEG2RAD(angles[1]), &sy, &cy);
	SinCos(M_DEG2RAD(angles[0]), &sp, &cp);

	forward[0].x = cp * cy;
	forward[1].y = cp * sy;
	forward[2].z = -sp;
}

void math::AngleV2ectors(const QAngle_t& angles, Vector_t& forward) {
	float	sp, sy, cp, cy;

	SinCos(M_DEG2RAD(angles[1]), &sy, &cy);
	SinCos(M_DEG2RAD(angles[0]), &sp, &cp);

	forward.x = cp * cy;
	forward.y = cp * sy;
	forward.z = -sp;
}
void math::angle_vector(const QAngle_t& angles, Vector_t& forward) noexcept {
	const float x = angles.x * std::numbers::pi_v<float> / 180.f;
	const float y = angles.y * std::numbers::pi_v<float> / 180.f;
	const float sp = std::sin(x);
	const float cp = std::cos(x);
	const float sy = std::sin(y);
	const float cy = std::cos(y);
	forward.x = cp * cy;
	forward.y = cp * sy;
	forward.z = -sp;
}

Vector_t math::angle_vector(const QAngle_t& angles) noexcept {
	Vector_t forward;
	angle_vector(angles, forward);
	return forward;
}

void  math::anglevectors(const QAngle_t& angles, Vector_t* forward, Vector_t* right, Vector_t* up)
{
	float cp = std::cos(M_DEG2RAD(angles.x)), sp = std::sin(M_DEG2RAD(angles.x));
	float cy = std::cos(M_DEG2RAD(angles.y)), sy = std::sin(M_DEG2RAD(angles.y));
	float cr = std::cos(M_DEG2RAD(angles.z)), sr = std::sin(M_DEG2RAD(angles.z));

	if (forward) {
		forward->x = cp * cy;
		forward->y = cp * sy;
		forward->z = -sp;
	}

	if (right) {
		right->x = -1.f * sr * sp * cy + -1.f * cr * -sy;
		right->y = -1.f * sr * sp * sy + -1.f * cr * cy;
		right->z = -1.f * sr * cp;
	}

	if (up) {
		up->x = cr * sp * cy + -sr * -sy;
		up->y = cr * sp * sy + -sr * cy;
		up->z = cr * cp;
	}
}
void math::angle_vectors(const Vector_t& angles, Vector_t* forward, Vector_t* right, Vector_t* up) {
	float sp, sy, sr, cp, cy, cr;

	SinCos(angles.x * 0.017453292f, &sp, &cp);
	SinCos(angles.y * 0.017453292f, &sy, &cy);
	SinCos(angles.z * 0.017453292f, &sr, &cr);

	if (forward)
	{
		forward->x = cp * cy;
		forward->y = cp * sy;
		forward->z = -sp;
	}

	if (right)
	{
		right->x = sr * sp * cy - cr * sy;
		right->y = cr * cy + sr * sp * sy;
		right->z = sr * cp;
	}

	if (up)
	{
		up->x = sr * sy + cr * sp * cy;
		up->y = cr * sp * sy - sr * cy;
		up->z = cr * cp;
	}
}


void math::vector_angles(Vector_t vec_forward, Vector_t& vec_angles)
{
	if (vec_forward.x == 0.f && vec_forward.y == 0.f)
	{
		vec_angles.y = 0.f;
		vec_angles.x = vec_forward.z > 0.f ? 270.f : 90.f;
	}
	else
	{
		vec_angles.y = M_RAD2DEG(std::atan2(vec_forward.y, vec_forward.x));
		if (vec_angles.y < 0.f)
			vec_angles.y += 360.f;

		vec_angles.x = M_RAD2DEG(std::atan2(-vec_forward.z, vec_forward.Length2D()));
		if (vec_angles.x < 0.f)
			vec_angles.x += 360.f;
	}

	vec_angles.x = std::remainder(vec_angles.x, 360.f);
	vec_angles.y = std::remainder(vec_angles.y, 360.f);
	vec_angles.z = std::remainder(vec_angles.z, 360.f);
}

Vector_t math::aim_direction(Vector_t src, Vector_t dst)
{
	Vector_t result;

	Vector_t delta = dst - src;

	vector_angles(delta, result);

	return result;
}

void math::vector_transform(Vector_t& in, Matrix3x4_t& matrix, Vector_t& out) {
	out = {
		in.Dot({ matrix[0][0], matrix[0][1], matrix[0][2] }) + matrix[0][3],
		in.Dot({ matrix[1][0], matrix[1][1], matrix[1][2] }) + matrix[1][3],
		in.Dot({ matrix[2][0], matrix[2][1], matrix[2][2] }) + matrix[2][3]
	};
}
void math::angle_vectors(Vector_t angles, Vector_t& forward, Vector_t& right, Vector_t& up)
{
	float			angle;
	static float	sr, sp, sy, cr, cp, cy;

	angle = angles.y * (math::_2PI / 360);
	sy = sin(angle);
	cy = cos(angle);

	angle = angles.x * (math::_2PI / 360);
	sp = sin(angle);
	cp = cos(angle);

	angle = angles.z * (math::_2PI / 360);
	sr = sin(angle);
	cr = cos(angle);

	forward.x = cp * cy;
	forward.y = cp * sy;
	forward.z = -sp;

	right.x = (-1 * sr * sp * cy + -1 * cr * -sy);
	right.y = (-1 * sr * sp * sy + -1 * cr * cy);
	right.z = -1 * sr * cp;

	up.x = (cr * sp * cy + -sr * -sy);
	up.y = (cr * sp * sy + -sr * cy);
	up.z = cr * cp;
}

void math::transform_to_matrix(bone_data& in, Matrix3x4_t& out) 
{
	Matrix3x4_t  matrix;

	Vector4D_t rot;
	rot = in.rot;
	Vector_t pos;
	pos = in.pos;

	matrix[0][0] = 1.0f - 2.0f * rot.y * rot.y - 2.0f * rot.z * rot.z;
	matrix[1][0] = 2.0f * rot.x * rot.y + 2.0f * rot.w * rot.z;
	matrix[2][0] = 2.0f * rot.x * rot.z - 2.0f * rot.w * rot.y;

	matrix[0][1] = 2.0f * rot.x * rot.y - 2.0f * rot.w * rot.z;
	matrix[1][1] = 1.0f - 2.0f * rot.x * rot.x - 2.0f * rot.z * rot.z;
	matrix[2][1] = 2.0f * rot.y * rot.z + 2.0f * rot.w * rot.x;

	matrix[0][2] = 2.0f * rot.x * rot.z + 2.0f * rot.w * rot.y;
	matrix[1][2] = 2.0f * rot.y * rot.z - 2.0f * rot.w * rot.x;
	matrix[2][2] = 1.0f - 2.0f * rot.x * rot.x - 2.0f * rot.y * rot.y;

	matrix[0][3] = pos.x;
	matrix[1][3] = pos.y;
	matrix[2][3] = pos.z;

	out = matrix;

}
void math::VectorAngless(const Vector_t& forward, QAngle_t& angles, Vector_t* up) {
	Vector_t  left;
	float   len, up_z, pitch, yaw, roll;

	// get 2d length.
	len = forward.Length2D();

	if (up && len > 0.001f) {
		pitch = M_RAD2DEG(std::atan2(-forward.z, len));
		yaw = M_RAD2DEG(std::atan2(forward.y, forward.x));

		// get left direction vector using cross product.
		left = (*up).CrossProduct(forward).Normalized();

		// calculate up_z.
		up_z = (left.y * forward.x) - (left.x * forward.y);

		// calculate roll.
		roll = M_RAD2DEG(std::atan2(left.z, up_z));
	}

	else {
		if (len > 0.f) {
			// calculate pitch and yaw.
			pitch = M_RAD2DEG(std::atan2(-forward.z, len));
			yaw = M_RAD2DEG(std::atan2(forward.y, forward.x));
			roll = 0.f;
		}

		else {
			pitch = (forward.z > 0.f) ? -90.f : 90.f;
			yaw = 0.f;
			roll = 0.f;
		}
	}

	// set out angles.
	angles = { pitch, yaw, roll };
}
#include <imgui_internal.h>
bool math::WorldToScreen(const Vector_t& in, ImVec2& out) {
	if (!ImGui::GetCurrentContext()) return false;

	auto z = g::m_view_matrix[3][0] * in.x + g::m_view_matrix[3][1] * in.y +
		g::m_view_matrix[3][2] * in.z + g::m_view_matrix[3][3];

	if (z < 0.001f)
		return false;

	out = { (ImGui::GetIO().DisplaySize.x * 0.5f), (ImGui::GetIO().DisplaySize.y * 0.5f) };
	out.x *= 1.0f + (g::m_view_matrix[0][0] * in.x + g::m_view_matrix[0][1] * in.y +
		g::m_view_matrix[0][2] * in.z + g::m_view_matrix[0][3]) /
		z;
	out.y *= 1.0f - (g::m_view_matrix[1][0] * in.x + g::m_view_matrix[1][1] * in.y +
		g::m_view_matrix[1][2] * in.z + g::m_view_matrix[1][3]) /
		z;

	out = ImTrunc(out);
	return true;
}

bool math::world_2s(Vector_t& origin, Vector_t& screen)
{
	ImVec2 out_imvec2 = { };
	if (!math::WorldToScreen(origin, out_imvec2))
		return false;

	screen = Vector_t(out_imvec2.x, out_imvec2.y, 0.f);

	return true;
}
bool math::w2s(const Vector_t& in, ImVec2& out) {
	if (!ImGui::GetCurrentContext()) return false;

	auto z = g::m_view_matrix[3][0] * in.x + g::m_view_matrix[3][1] * in.y +
		g::m_view_matrix[3][2] * in.z + g::m_view_matrix[3][3];

	if (z < 0.001f)
		return false;

	out = { (ImGui::GetIO().DisplaySize.x * 0.5f), (ImGui::GetIO().DisplaySize.y * 0.5f) };
	out.x *= 1.0f + (g::m_view_matrix[0][0] * in.x + g::m_view_matrix[0][1] * in.y +
		g::m_view_matrix[0][2] * in.z + g::m_view_matrix[0][3]) /
		z;
	out.y *= 1.0f - (g::m_view_matrix[1][0] * in.x + g::m_view_matrix[1][1] * in.y +
		g::m_view_matrix[1][2] * in.z + g::m_view_matrix[1][3]) /
		z;

	out = { out.x, out.y };
	return true;
}
