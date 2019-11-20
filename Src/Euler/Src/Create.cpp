#include <EulerOperation/Operation.h>
#include <EulerOperation/Create.h>

EULER_OPR_BEGIN

Solid *CreateFromPolygon(const Polygon2D &polygon, float height)
{
    auto toTopPoint = [y = 0.5f * height](const Vec2 &p)
    {
        return Point{ Vec3(p.x, y, p.y) };
    };

    auto &outerPolygon = polygon.outerLoop;
    assert(outerPolygon.size() >= 3);

    auto [solid, startVertex] = MVFS(toTopPoint(outerPolygon[0]));
    Loop *loop = solid->face->loop;

    Vertex *lastEnd = startVertex;
    for(size_t i = 1; i < outerPolygon.size(); ++i)
    {
        HalfEdge *halfEdge = MEV(lastEnd, loop, toTopPoint(outerPolygon[i]));
        lastEnd = halfEdge->end;
    }

    auto [sameDir, revDir] = MEF(lastEnd, startVertex, loop);

    for(const auto &innerPolygon : polygon.innerLoops)
    {
        assert(innerPolygon.size() >= 3);

        Vertex *bridgeStart   = revDir->hedge->start;
        HalfEdge *bridge      = MEV(bridgeStart, revDir, toTopPoint(innerPolygon[0]));

        startVertex = bridge->end;
        lastEnd     = startVertex;
        for(size_t i = 1; i < innerPolygon.size(); ++i)
        {
            HalfEdge *halfEdge = MEV(lastEnd, revDir, toTopPoint(innerPolygon[i]));
            lastEnd = halfEdge->end;
        }

        auto [outer, inner] = KEMR(bridgeStart, startVertex, revDir);
        auto [sam, rev] = MEF(lastEnd, startVertex, inner);

        sam->inner = true;
    }

    SweepAlongSegment(sameDir->face, Vec3(0, -height, 0));

    return solid;
}

EULER_OPR_END
