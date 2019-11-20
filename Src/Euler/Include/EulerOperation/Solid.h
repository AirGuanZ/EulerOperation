#pragma once

#include <EulerOperation/Common.h>

EULER_OPR_BEGIN

struct Point;

struct Solid;
struct Face;
struct Loop;
struct Edge;
struct HalfEdge;
struct Vertex;

struct Point
{
    Vec3 pos;
};

struct Solid
{
    Face *face = nullptr;
    Edge *edge = nullptr;
};

struct Face
{
    Solid *solid = nullptr;

    Face *prev = nullptr;
    Face *succ = nullptr;

    Loop *loop = nullptr;
};

struct Loop
{
    Face *face = nullptr;

    bool inner = false;

    Loop *prev = nullptr;
    Loop *succ = nullptr;

    HalfEdge *hedge = nullptr;
};

struct Edge
{
    Edge *prev = nullptr;
    Edge *succ = nullptr;

    HalfEdge *hedge0 = nullptr;
    HalfEdge *hedge1 = nullptr;
};

struct HalfEdge
{
    Loop *loop = nullptr;
    Edge *edge = nullptr;

    HalfEdge *prev = nullptr;
    HalfEdge *succ = nullptr;

    Vertex *start = nullptr;
    Vertex *end   = nullptr;
};

struct Vertex
{
    Point point;
};

EULER_OPR_END
