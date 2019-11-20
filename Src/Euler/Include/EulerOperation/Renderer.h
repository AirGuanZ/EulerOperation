#pragma once

#include <EulerOperation/Solid.h>

EULER_OPR_BEGIN

class Renderer : public agz::misc::uncopyable_t
{
public:

    Renderer() noexcept;

    void SetSolid(const Solid *solid);

    void RenderWireframe(const Mat4 &WVP) const;

    void RenderFace(const Mat4 &WVP) const;

private:

    void ComputeTriangles();

    void TriangulateFace(const Face *face);

    struct Triangle
    {
        Vec3 vertices[3];
        Vec3 color;
    };

    const Solid *solid_;
    std::vector<Triangle> triangles_;

    D3D::Immediate3D imm3d_;
};

EULER_OPR_END
