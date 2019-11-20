#pragma once

#include <agz/utility/math.h>

#include <D3D/Base.h>

#define EULER_OPR_BEGIN namespace EulerOpr {
#define EULER_OPR_END   }

EULER_OPR_BEGIN

using Vec2 = D3D::Vec2;
using Vec3 = D3D::Vec3;
using Vec4 = D3D::Vec4;

using Mat4   = D3D::Mat4;
using Trans4 = Mat4::right_transform;

EULER_OPR_END
