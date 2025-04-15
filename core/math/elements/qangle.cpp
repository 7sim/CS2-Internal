#include "qangle.h"

// used: [d3d] xmscalarsincos
#include <directxmath.h>

#include "matrix.h"

// used: m_deg2rad
#include "..\math.h" 


Vector_t QAngle_t::ToVectors(Vector_t* side /*= nullptr*/, Vector_t* up /*= nullptr*/) {
	float  rad_pitch = M_DEG2RAD(this->x);
	float  rad_yaw = M_DEG2RAD(this->y);
	float sin_pitch;
	float sin_yaw;
	float sin_roll;
	float cos_pitch;
	float cos_yaw;
	float cos_roll;

	DirectX::XMScalarSinCos(&sin_pitch, &cos_pitch, rad_pitch);
	DirectX::XMScalarSinCos(&sin_yaw, &cos_yaw, rad_yaw);

	if (side || up)
		DirectX::XMScalarSinCos(&sin_roll, &cos_roll, M_DEG2RAD(this->z));

	if (side) {
		side->x = -1.0f * sin_roll * sin_pitch * cos_yaw + -1.0f * cos_roll * -sin_yaw;
		side->y = -1.0f * sin_roll * sin_pitch * sin_yaw + -1.0f * cos_roll * cos_yaw;
		side->z = -1.0f * sin_roll * cos_pitch;
	}

	if (up) {
		up->x = cos_roll * sin_pitch * cos_yaw + -sin_roll * -sin_yaw;
		up->y = cos_roll * sin_pitch * sin_yaw + -sin_roll * cos_yaw;
		up->z = cos_roll * cos_pitch;
	}

	return { cos_pitch * cos_yaw, cos_pitch * sin_yaw, -sin_pitch };
}


void QAngle_t::ToDirections(Vector_t* pvecForward, Vector_t* pvecRight, Vector_t* pvecUp) const
{
	float flPitchSin, flPitchCos, flYawSin, flYawCos, flRollSin, flRollCos;
	DirectX::XMScalarSinCos(&flPitchSin, &flPitchCos, M_DEG2RAD(this->x));
	DirectX::XMScalarSinCos(&flYawSin, &flYawCos, M_DEG2RAD(this->y));
	DirectX::XMScalarSinCos(&flRollSin, &flRollCos, M_DEG2RAD(this->z));

	if (pvecForward != nullptr)
	{
		pvecForward->x = flPitchCos * flYawCos;
		pvecForward->y = flPitchCos * flYawSin;
		pvecForward->z = -flPitchSin;
	}

	if (pvecRight != nullptr)
	{
		pvecRight->x = (-flRollSin * flPitchSin * flYawCos) + (-flRollCos * -flYawSin);
		pvecRight->y = (-flRollSin * flPitchSin * flYawSin) + (-flRollCos * flYawCos);
		pvecRight->z = (-flRollSin * flPitchCos);
	}

	if (pvecUp != nullptr)
	{
		pvecUp->x = (flRollCos * flPitchSin * flYawCos) + (-flRollSin * -flYawSin);
		pvecUp->y = (flRollCos * flPitchSin * flYawSin) + (-flRollSin * flYawCos);
		pvecUp->z = (flRollCos * flPitchCos);
	}
}

Matrix3x4_t QAngle_t::ToMatrix(const Vector_t& vecOrigin) const
{
	float flPitchSin, flPitchCos, flYawSin, flYawCos, flRollSin, flRollCos;
	DirectX::XMScalarSinCos(&flPitchSin, &flPitchCos, M_DEG2RAD(this->x));
	DirectX::XMScalarSinCos(&flYawSin, &flYawCos, M_DEG2RAD(this->y));
	DirectX::XMScalarSinCos(&flRollSin, &flRollCos, M_DEG2RAD(this->z));

	return {
		(flPitchCos * flYawCos), (flRollSin * flPitchSin * flYawCos + flRollCos * -flYawSin), (flRollCos * flPitchSin * flYawCos + -flRollSin * -flYawSin), vecOrigin.x,
		(flPitchCos * flYawSin), (flRollSin * flPitchSin * flYawSin + flRollCos * flYawCos), (flRollCos * flPitchSin * flYawSin + -flRollSin * flYawCos), vecOrigin.y,
		(-flPitchSin), (flRollSin * flPitchCos), (flRollCos * flPitchCos), vecOrigin.z
	};
}