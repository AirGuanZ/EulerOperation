#include <random>

#include <poly2tri/poly2tri.h>

#include <EulerOperation/Renderer.h>

EULER_OPR_BEGIN

namespace
{
    struct Frame2D
    {
        Vec3 o;
        Vec3 x;
        Vec3 y;

        Vec2 WorldToLocal(const Vec3 &p) const noexcept
        {
            Vec3 d = p - o;
            return { dot(d, x), dot(d, y) };
        }

        Vec3 LocalToWorld(const Vec2 &p) const noexcept
        {
            return o + p.x * x + p.y * y;
        }
    };

    // 找到face上的三个不共线的点
    std::tuple<Vec3, Vec3, Vec3> FindTriangleOn(const Face *face)
    {
        const Loop *loop = face->loop;
        const HalfEdge *he = loop->hedge;

        Vec3 a = he->start->point.pos; he = he->succ;
        Vec3 b = he->start->point.pos; he = he->succ;

        do
        {
            Vec3 c = he->start->point.pos;
            float cosValue = cos(b - a, c - b);
            if(-0.99f < cosValue && cosValue < 0.99f)
            {
                return { a, b, c };
            }
            he = he->succ;
        } while(he);

        return { a, b, b };
    }

    Frame2D ConstructFrame2DForPlane(const Face *face)
    {
        auto [a, b, c] = FindTriangleOn(face);
        Vec3 z = cross(b - a, c - b).normalize();
        Vec3 x = (b - a).normalize();
        Vec3 y = cross(z, x);
        return { a, x, y };
    }

    Vec3 ComputeColor(const Face *face)
    {
        using RNG = std::default_random_engine;
        size_t pface = reinterpret_cast<size_t>(face);
        RNG rng(static_cast<RNG::result_type>(pface));
        std::uniform_real_distribution<float> dis(0, 1);

        float r = dis(rng), g = dis(rng), b = dis(rng);
        return { r, g, b };
    }
}

Renderer::Renderer() noexcept
    : solid_(nullptr)
{
    
}

void Renderer::SetSolid(const Solid *solid)
{
    solid_ = solid;

    ComputeTriangles();
}

void Renderer::RenderWireframe(const Mat4 &WVP) const
{
    assert(solid_);

    const Edge *e = solid_->edge;
    do
    {
        Vec3 a = e->hedge0->start->point.pos;
        Vec3 b = e->hedge0->end->point.pos;
        imm3d_.DrawLine(a, b, { 1, 1, 1, 1 }, WVP);
        e = e->succ;

    } while(e && e != solid_->edge);
}

void Renderer::RenderFace(const Mat4 &WVP) const
{
    assert(solid_);

    for(auto &tri : triangles_)
    {
        imm3d_.DrawTriangle(
            tri.vertices[0], tri.vertices[1], tri.vertices[2],
            Vec4(tri.color.x, tri.color.y, tri.color.z, 1), WVP);
    }
}

void Renderer::ComputeTriangles()
{
    triangles_.clear();
    for(const Face *face = solid_->face; face; face = face->succ)
    {
        TriangulateFace(face);
    }
}

void Renderer::TriangulateFace(const Face *face)
{
    Frame2D frame = ConstructFrame2DForPlane(face);
    std::vector<p2t::Point *> allPoints;

    // 找到outer loop

    Loop *outerLoop = face->loop;
    while(outerLoop->inner)
    {
        outerLoop = outerLoop->succ;
    }

    // 讲outer loop上的点投影到frame上

    std::vector<p2t::Point*> outerPoints;
    const HalfEdge *outerHalfEdge = outerLoop->hedge;
    do
    {
        Vec2 p = frame.WorldToLocal(outerHalfEdge->start->point.pos);
        p2t::Point *point = new p2t::Point{ p.x, p.y };

        outerPoints.push_back(point);
        allPoints.push_back(point);

        outerHalfEdge = outerHalfEdge->succ;

    } while(outerHalfEdge != outerLoop->hedge);

    p2t::CDT cdt(std::move(outerPoints));

    // 处理inner loop

    for(Loop *loop = face->loop; loop; loop = loop->succ)
    {
        if(!loop->inner)
        {
            continue;
        }

        std::vector<p2t::Point*> points;
        const HalfEdge *halfEdge = loop->hedge;
        do
        {
            Vec2 p = frame.WorldToLocal(halfEdge->start->point.pos);
            p2t::Point *point = new p2t::Point{ p.x, p.y };

            points.push_back(point);
            allPoints.push_back(point);

            halfEdge = halfEdge->succ;

        } while(halfEdge != loop->hedge);

        cdt.AddHole(std::move(points));
    }

    // 三角化

    cdt.Triangulate();

    // 将三角化后的结果转回三维空间

    Vec3 color = ComputeColor(face);

    auto toWorld = [&](const p2t::Point *p)
    {
        return frame.LocalToWorld({
            static_cast<float>(p->x),
            static_cast<float>(p->y)
        });
    };

    for(auto tri : cdt.GetTriangles())
    {
        Triangle triangle;

        triangle.vertices[0] = toWorld(tri->GetPoint(0));
        triangle.vertices[1] = toWorld(tri->GetPoint(1));
        triangle.vertices[2] = toWorld(tri->GetPoint(2));

        triangle.color = color;

        triangles_.push_back(triangle);
    }

    for(auto p : allPoints)
    {
        delete p;
    }
}

EULER_OPR_END
