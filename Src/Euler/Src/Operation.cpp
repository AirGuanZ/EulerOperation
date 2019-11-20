#include <set>

#include <EulerOperation/Operation.h>

EULER_OPR_BEGIN

namespace
{
    void AddEdge(Solid *solid, Edge *edge)
    {
        if(!solid->edge)
        {
            solid->edge = edge;
        }
        else
        {
            Edge *endEdge = solid->edge;
            while(endEdge->succ)
            {
                endEdge = endEdge->succ;
            }
            endEdge->succ = edge;
            edge->prev    = endEdge;
        }
    }

    void RemoveEdge(Solid *solid, Edge *edge)
    {
        if(solid->edge == edge)
        {
            solid->edge = edge->succ;
        }
        else
        {
            if(edge->prev)
            {
                edge->prev->succ = edge->succ;
            }

            if(edge->succ)
            {
                edge->succ->prev = edge->prev;
            }
        }
    }

    void AddFace(Solid *solid, Face *face)
    {
        Face *endFace = solid->face;
        while(endFace->succ)
        {
            endFace = endFace->succ;
        }
        endFace->succ = face;
        face->prev = endFace;
    }

    void RemoveFace(Solid *solid, Face *face)
    {
        if(solid->face == face)
        {
            solid->face = face->succ;
        }
        else
        {
            if(face->prev)
            {
                face->prev->succ = face->succ;
            }

            if(face->succ)
            {
                face->succ->prev = face->prev;
            }
        }
    }

    void AddLoop(Face *face, Loop *loop)
    {
        assert(face->loop);
        Loop *endLoop = face->loop;
        while(endLoop->succ)
        {
            endLoop = endLoop->succ;
        }
        endLoop->succ = loop;
        loop->prev = endLoop;
    }

    HalfEdge *GetAdjHalfEdge(HalfEdge *halfEdge)
    {
        Edge *edge = halfEdge->edge;
        return halfEdge == edge->hedge0 ? edge->hedge1 : edge->hedge0;
    }
}

MVFSResult MVFS(const Point &point)
{
    Solid  *newSolid  = new Solid;
    Face   *newFace   = new Face;
    Loop   *newLoop   = new Loop;
    Vertex *newVertex = new Vertex;

    newSolid->face   = newFace;
    newFace->solid   = newSolid;
    newFace->loop    = newLoop;
    newLoop->face    = newFace;
    newVertex->point = point;

    return { newSolid, newVertex };
}

HalfEdge *MEV(Vertex *vertex, Loop *loop, const Point &point)
{
    HalfEdge *newHalfEdge0 = new HalfEdge;
    HalfEdge *newHalfEdge1 = new HalfEdge;
    Edge     *newEdge      = new Edge;
    Vertex   *newVertex    = new Vertex;

    newVertex->point = point;

    newHalfEdge0->loop = loop;
    newHalfEdge1->loop = loop;
    newHalfEdge0->edge = newEdge;
    newHalfEdge1->edge = newEdge;

    newEdge->hedge0 = newHalfEdge0;
    newEdge->hedge1 = newHalfEdge1;

    newHalfEdge0->succ = newHalfEdge1;
    newHalfEdge1->prev = newHalfEdge0;

    newHalfEdge0->start = vertex;
    newHalfEdge0->end   = newVertex;
    newHalfEdge1->start = newVertex;
    newHalfEdge1->end   = vertex;

    AddEdge(loop->face->solid, newEdge);

    if(!loop->hedge)
    {
        // loop还是空的，从简处理

        loop->hedge = newHalfEdge0;
        newHalfEdge0->prev = newHalfEdge1;
        newHalfEdge1->succ = newHalfEdge0;
    }
    else
    {
        // 找到以vertex为end的half edge

        HalfEdge *endAtVertex = loop->hedge;
        while(endAtVertex->end != vertex)
        {
            endAtVertex = endAtVertex->succ;
        }

        HalfEdge *startAtVertex = endAtVertex->succ;

        endAtVertex->succ  = newHalfEdge0;
        newHalfEdge0->prev = endAtVertex;

        startAtVertex->prev = newHalfEdge1;
        newHalfEdge1->succ  = startAtVertex;
    }

    return newHalfEdge0;
}

MEFResult MEF(Vertex *vtx0, Vertex *vtx1, Loop *loop)
{
    Edge     *newEdge      = new Edge;
    HalfEdge *newHalfEdge0 = new HalfEdge;
    HalfEdge *newHalfEdge1 = new HalfEdge;
    Loop     *newLoop      = new Loop;
    Face     *newFace      = new Face;

    newEdge->hedge0 = newHalfEdge0;
    newEdge->hedge1 = newHalfEdge1;
    AddEdge(loop->face->solid, newEdge);

    newHalfEdge0->edge = newEdge;
    newHalfEdge1->edge = newEdge;

    newHalfEdge0->start = vtx0;
    newHalfEdge0->end   = vtx1;
    newHalfEdge1->start = vtx1;
    newHalfEdge1->end   = vtx0;

    HalfEdge *endAtVtx0 = loop->hedge;
    while(endAtVtx0->end != vtx0)
    {
        endAtVtx0 = endAtVtx0->succ;
    }
    HalfEdge *startAtVtx0 = endAtVtx0->succ;

    HalfEdge *endAtVtx1 = loop->hedge;
    while(endAtVtx1->end != vtx1)
    {
        endAtVtx1 = endAtVtx1->succ;
    }
    HalfEdge *startAtVtx1 = endAtVtx1->succ;

    endAtVtx0->succ    = newHalfEdge0;
    newHalfEdge0->prev = endAtVtx0;

    startAtVtx1->prev  = newHalfEdge0;
    newHalfEdge0->succ = startAtVtx1;

    endAtVtx1->succ    = newHalfEdge1;
    newHalfEdge1->prev = endAtVtx1;

    startAtVtx0->prev  = newHalfEdge1;
    newHalfEdge1->succ = startAtVtx0;

    AddFace(loop->face->solid, newFace);
    newFace->solid = loop->face->solid;
    newFace->loop = newLoop;
    newLoop->face = newFace;

    Loop *loop0 = loop;
    Loop *loop1 = newLoop;

    loop0->hedge = newHalfEdge0;
    loop1->hedge = newHalfEdge1;
    newHalfEdge0->loop = loop0;
    newHalfEdge1->loop = loop1;

    for(HalfEdge *l = newHalfEdge0->succ; l != newHalfEdge0; l = l->succ)
    {
        l->loop = loop0;
    }

    for(HalfEdge *l = newHalfEdge1->succ; l != newHalfEdge1; l = l->succ)
    {
        l->loop = loop1;
    }

    return { loop0, loop1 };
}

KEMRResult KEMR(Vertex *vtx0, Vertex *vtx1, Loop *loop)
{
    Loop *newLoop = new Loop;

    // 从half edge链表中删除vtx0和vtx1间的半边

    HalfEdge *vtx0ToVtx1 = loop->hedge;
    while(vtx0ToVtx1->start != vtx0 || vtx0ToVtx1->end != vtx1)
    {
        vtx0ToVtx1 = vtx0ToVtx1->succ;
    }
    HalfEdge *vtx1ToVtx0 = GetAdjHalfEdge(vtx0ToVtx1);

    vtx0ToVtx1->prev->succ = vtx1ToVtx0->succ;
    vtx1ToVtx0->succ->prev = vtx0ToVtx1->prev;

    vtx0ToVtx1->succ->prev = vtx1ToVtx0->prev;
    vtx1ToVtx0->prev->succ = vtx0ToVtx1->succ;

    // 从边表中删除vtx0和vtx1间的边

    RemoveEdge(loop->face->solid, vtx0ToVtx1->edge);

    // 填充newLoop

    newLoop->face = loop->face;
    AddLoop(loop->face, newLoop);

    loop->hedge    = vtx0ToVtx1->prev;
    newLoop->hedge = vtx0ToVtx1->succ;

    // 修改newLoop的hedge的loop

    HalfEdge *lInNewLoop = newLoop->hedge;
    do
    {
        lInNewLoop->loop = newLoop;
        lInNewLoop = lInNewLoop->succ;

    } while(lInNewLoop != newLoop->hedge);

    delete vtx0ToVtx1->edge;
    delete vtx0ToVtx1;
    delete vtx1ToVtx0;

    return { loop, newLoop };
}

void KFMRH(Loop *outerLoop, Loop *innerLoop)
{
    Face *innerFace = innerLoop->face;
    RemoveFace(innerFace->solid, innerFace);
    innerLoop->face = outerLoop->face;
    innerLoop->inner = true;
    AddLoop(outerLoop->face, innerLoop);
    delete innerFace;
}

namespace
{
    /**
     * @brief 扫成单个loop，返回扫成结果中原loop对应的loop
     */
    Loop *SweepLoop(Loop *loop, const Vec3 &sweepVector)
    {
        HalfEdge *halfEdge = loop->hedge;
        Vertex   *start    = halfEdge->start;

        HalfEdge *firstBridge = MEV(start, loop, { start->point.pos + sweepVector });
        Vertex *lastNewVertex = firstBridge->end;

        halfEdge = halfEdge->succ;
        Vertex *vertex = halfEdge->start;

        while(vertex != start)
        {
            HalfEdge *bridge = MEV(vertex, loop, { vertex->point.pos + sweepVector });
            Vertex *newVertex = bridge->end;

            auto [sameDir, revDir] = MEF(lastNewVertex, newVertex, loop);
            loop = sameDir;

            lastNewVertex = newVertex;
            halfEdge      = halfEdge->succ;
            vertex        = halfEdge->start;
        }

        return MEF(lastNewVertex, firstBridge->end, loop).sameDir;
    }
}

void SweepAlongSegment(Face *face, const Vec3 &sweepVector)
{
    // 先对outer loop进行扫成

    Loop *outerLoop = face->loop;
    Face *adjFace = GetAdjHalfEdge(face->loop->hedge)->loop->face;
    while(outerLoop->inner)
    {
        outerLoop = outerLoop->succ;
    }
    outerLoop = SweepLoop(outerLoop, sweepVector);

    // 处理所有的inner loop

    for(Loop *loop = adjFace->loop; loop; loop = loop->succ)
    {
        if(loop->inner)
        {
            Loop *innerLoop = GetAdjHalfEdge(loop->hedge)->loop;
            innerLoop = SweepLoop(innerLoop, sweepVector);
            KFMRH(outerLoop, innerLoop);
        }
    }
}

void DestroySolid(Solid *solid)
{
    std::set<HalfEdge*> halfEdges;
    
    Face *nextFace = nullptr;
    for(Face *face = solid->face; face; face = nextFace)
    {
        Loop *nextLoop = nullptr;
        for(Loop *loop = face->loop; loop; loop = nextLoop)
        {
            halfEdges.insert(loop->hedge);
            for(HalfEdge *halfEdge = loop->hedge->succ; halfEdge != loop->hedge; halfEdge = halfEdge->succ)
            {
                halfEdges.insert(halfEdge);
            }

            nextLoop = loop->succ;
            delete loop;
        }
        nextFace = face->succ;
        delete face;
    }

    std::set<Edge*>   edges;
    std::set<Vertex*> vertices;

    for(HalfEdge *halfEdge : halfEdges)
    {
        edges.insert(halfEdge->edge);
        vertices.insert(halfEdge->start);
        delete halfEdge;
    }

    for(Edge *edge : edges)
    {
        delete edge;
    }

    for(Vertex *vertex : vertices)
    {
        delete vertex;
    }

    delete solid;
}

EULER_OPR_END
