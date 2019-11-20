#pragma once

#include <EulerOperation/Solid.h>

EULER_OPR_BEGIN

struct MVFSResult
{
    Solid  *solid  = nullptr;
    Vertex *vertex = nullptr;
};

/**
 * @brief 从零开始构造一个含有一个面、一个环以及一个顶点的实体
 *
 * 返回实体和顶点
 */
MVFSResult MVFS(const Point &point);

/**
 * @brief 构造一个位于point处的顶点，并连接vertex和此顶点得到一条边
 *
 * 此边的两条半边被加入loop中，返回以新顶点为end的半边
 */
HalfEdge *MEV(Vertex *vertex, Loop *loop, const Point &point);

struct MEFResult
{
    Loop *sameDir    = nullptr;
    Loop *reverseDir = nullptr;
};

/**
 * @brief 在vtx0和vtx1间添加一条边，同时新增一个面（及一个环）
 *
 * 此操作将新增一个环，返回的sameDir为vtx0 -> vtx1d的那个环，reverseDir为另一个环
 */
MEFResult MEF(Vertex *vtx0, Vertex *vtx1, Loop *loop);

struct KEMRResult
{
    Loop *startSide = nullptr;
    Loop *endSide   = nullptr;
};

/**
 * @brief 删除vtx0和vtx1间的边，产生一个新环
 *
 * 返回的startSide为vtx0那边的环，endSide为vtx1那边的环
 */
KEMRResult KEMR(Vertex *vtx0, Vertex *vtx1, Loop *loop);

/**
 * @brief 删除一个面，将该面的环变为另一个面的内环
 */
void KFMRH(Loop *outerLoop, Loop *innerLoop);

/**
 * @brief 将face沿sweepVector进行扫成
 *
 * inner loop会形成通孔
 */
void SweepAlongSegment(Face *face, const Vec3 &sweepVector);

/**
 * @brief 释放一个实体的所有空间
 */
void DestroySolid(Solid *solid);

EULER_OPR_END
