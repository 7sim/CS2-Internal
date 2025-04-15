#pragma once

#include "../utils/utils.h"
// used: std::is_integral_v
#include <type_traits>
// used: sin, cos, pow, abs, sqrt
#include <corecrt_math.h>// used: MATH::Sin, cos, MATH::Pow, abs, sqrt
#include <corecrt_math.h>
// used: rand, srand
#include <cstdlib>
// used: time
#include <ctime>
#include "..\..\deps\imgui\imgui.h"
#include "elements\matrix.h"
#include "elements\vector.h"
#include "elements\qangle.h"

#undef min
#undef max
// convert angle in degrees to radians
#define M_DEG2RAD(DEGREES) ((DEGREES) * (math::_PI / 180.f))
// convert angle in radians to degrees
#define M_RAD2DEG(RADIANS) ((RADIANS) * (180.f / math::_PI))
/// linearly interpolate the value between @a'X0' and @a'X1' by @a'FACTOR'
#define M_LERP(X0, X1, FACTOR) ((X0) + ((X1) - (X0)) * (FACTOR))
/// trigonometry
#define M_COS(ANGLE) cos(ANGLE)
#define M_SIN(ANGLE) sin(ANGLE)
#define M_TAN(ANGLE) tan(ANGLE)
/// power
#define M_POW(BASE, EXPONENT) pow(BASE, EXPONENT)
/// absolute value
#define M_ABS(VALUE) abs(VALUE)
/// square root
#define M_SQRT(VALUE) sqrt(VALUE)
/// floor
#define M_FLOOR(VALUE) floor(VALUE)
constexpr auto RadPi = 3.14159265358979323846;
constexpr auto DegPi = 180.0;
template<typename T>
T ToDegrees(T radians) {
	return (radians * (static_cast<T>(DegPi) / static_cast<T>(RadPi)));
}
struct bone_data;
/*
 * MATHEMATICS
 * - basic trigonometry, algebraic mathematical functions and constants
 */
namespace math
{
	/* @section: constants */
	// pi value
	inline constexpr float _PI = 3.141592654f;
	// double of pi
	inline constexpr float _2PI = 6.283185307f;
	// half of pi
	inline constexpr float _HPI = 1.570796327f;
	// quarter of pi
	inline constexpr float _QPI = 0.785398163f;
	// reciprocal of double of pi
	inline constexpr float _1DIV2PI = 0.159154943f;
	// golden ratio
	inline constexpr float _PHI = 1.618033988f;
	bool WorldToScreen(const Vector_t& in, ImVec2& out);
	bool world_2s(Vector_t& in, Vector_t& out);

	bool w2s(const Vector_t& in, ImVec2& out);

	// capture game's exports
	bool Init();
	void vector_angles(Vector_t vec_forward, Vector_t& vec_angles);
	Vector_t aim_direction(Vector_t src, Vector_t dst);

	void angle_vectors(Vector_t angles, Vector_t& forward, Vector_t& right, Vector_t& up);
	void vector_transform(Vector_t& in, Matrix3x4_t& matrix, Vector_t& out);

	void TransformAABB(const Matrix3x4a_t& transform, const Vector_t& minsIn, const Vector_t& maxsIn, Vector_t& minsOut, Vector_t& maxsOut);
	void transform_to_matrix(bone_data& in, Matrix3x4_t& out);
	void TransformAABB(const Matrix3x4_t& transform, const Vector_t& minsIn, const Vector_t& maxsIn, Vector_t& minsOut, Vector_t& maxsOut);
	void AngleVectors(const QAngle_t& angles, Vector_t* forward);

	void AngleV2ectors(const QAngle_t& angles, Vector_t& forward);

	void angle_vector(const QAngle_t& angles, Vector_t& forward) noexcept;

	Vector_t angle_vector(const QAngle_t& angles) noexcept;

	float segment_dist(Vector_t start1, Vector_t end1, Vector_t start2, Vector_t end2) noexcept;

	/* @section: algorithm */
	/// alternative of 'std::min'
	/// @returns : minimal value of the given comparable values
	template <typename T>
	inline constexpr const T& Min(const T& left, const T& right) noexcept
	{
		return (right < left) ? right : left;
	}
	
	template <typename T>
	inline T clamp(const T& n, const T& lower, const T& upper)
	{
		return std::max(lower, std::min(n, upper));
	}

	/// alternative of 'std::max'
	/// @returns : maximal value of the given comparable values
	template <typename T>
	[[nodiscard]] inline constexpr const T& Max(const T& left, const T& right) noexcept
	{
		return (right > left) ? right : left;
	}


	[[nodiscard]] inline float normalize_yaw(float yaw) noexcept
	{
		while (yaw > 180.0f)
			yaw -= 360.0f;

		while (yaw < -180.0f)
			yaw += 360.0f;

		return yaw;
	}
	/// alternative of 'std::clamp'
	/// @returns : value clamped in range ['minimal' .. 'maximal']
	template <typename T>
	[[nodiscard]] inline constexpr const T& Clamp(const T& value, const T& minimal, const T& maximal) noexcept
	{
		return (value < minimal) ? minimal : (value > maximal) ? maximal :
			value;
	}	
	inline void sin_cos(float rad, float* sine, float* cosine) {
		*sine = std::sinf(rad);
		*cosine = std::cosf(rad);
	}


	void  anglevectors(const QAngle_t& angles, Vector_t* forward, Vector_t* right = nullptr, Vector_t* up = nullptr);

	inline void angle_vectors(const Vector_t& angles, Vector_t& forward)
	{
		float	sp, sy, cp, cy;

		sin_cos(M_DEG2RAD(angles[1]), &sy, &cy);
		sin_cos(M_DEG2RAD(angles[0]), &sp, &cp);

		forward.x = cp * cy;
		forward.y = cp * sy;
		forward.z = -sp;
	}

	inline void vector_angles_aim(const Vector_t& vecForward, Vector_t& vecAngles) {
		Vector_t vecView;
		if (vecForward.y == 0 && vecForward.x == 0)
		{
			vecView.x = 0.f;
			vecView.y = 0.f;
		}
		else
		{
			vecView.y = atan2(vecForward.y, vecForward.x) * 180.f / 3.14f;

			if (vecView.y < 0.f)
				vecView.y += 360.f;

			auto tmp = vecForward.Length2D();

			vecView.x = atan2(-vecForward.z, tmp) * 180.f / 3.14f;

			if (vecView.x < 0.f)
				vecView.x += 360.f;
		}

		vecAngles.x = vecView.x;
		vecAngles.y = vecView.y;
		vecAngles.z = 0.f;
	}
	inline void normalize_angles(Vector_t& angles)
	{
		while (angles.x > 89.0f)
			angles.x -= 180.0f;

		while (angles.x < -89.0f)
			angles.x += 180.0f;

		while (angles.y < -180.0f)
			angles.y += 360.0f;

		while (angles.y > 180.0f)
			angles.y -= 360.0f;

		angles.z = 0.0f;
	}


	void VectorAngless(const Vector_t& forward, QAngle_t& angles, Vector_t* up = nullptr);

 void angle_vectors(const Vector_t& angles, Vector_t* forward, Vector_t* right, Vector_t* up);
	[[nodiscard]] inline void vec_angles(Vector_t forward, Vector_t* angles)
	{
		float tmp, yaw, pitch;

		if (forward.y == 0.f && forward.x == 0.f) {
			yaw = 0;
			if (forward.z > 0) {
				pitch = 270;
			}
			else {
				pitch = 90.f;
			}
		}
		else {
			yaw = (float)(atan2(forward.y, forward.x) * 180.f / 3.14159265358979323846f);
			if (yaw < 0) {
				yaw += 360.f;
			}
			tmp = (float)sqrt(forward.x * forward.x + forward.y * forward.y);
			pitch = (float)(atan2(-forward.z, tmp) * 180.f / 3.14159265358979323846f);
			if (pitch < 0) {
				pitch += 360.f;
			}
		}
		angles->x = pitch;
		angles->y = yaw;
		angles->z = 0.f;
	}

	/* @section: exponential */
	/// @returns: true if given number is power of two, false otherwise
	template <typename T> requires (std::is_integral_v<T>)
		[[nodiscard]] inline constexpr bool IsPowerOfTwo(const T value) noexcept
	{
		return value != 0 && (value & (value - 1)) == 0;
	}

	__forceinline float AngleNormalize(float angle)
	{
		angle = fmodf(angle, 360.0f);
		if (angle > 180)
		{
			angle -= 360;
		}
		if (angle < -180)
		{
			angle += 360;
		}
		return angle;
	}

	/* @section: random using game's exports */

	inline int(__cdecl* RandomSeed)(int seed) = nullptr;
	inline float(__cdecl* RandomFloat)(float min, float max) = nullptr;
	inline float(__cdecl* RandomFloatExp)(float min, float max, float exponent) = nullptr;
	inline int(__cdecl* RandomInt)(int min, int max) = nullptr;
	inline float(__cdecl* RandomGaussianFloat)(float mean, float std_dev) = nullptr;

}
