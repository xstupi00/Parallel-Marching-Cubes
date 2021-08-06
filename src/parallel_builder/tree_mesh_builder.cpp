/**
 * @file    tree_mesh_builder.cpp
 *
 * @author  Simon Stupinsky <xstupi0000@stud.fit.vutbr.cz>
 *
 * @brief   Parallel Marching Cubes implementation using OpenMP tasks + octree early elimination
 *
 * @date    15.12.2019 10:00
 **/

#include <iostream>
#include <math.h>
#include <limits>
//#include <omp.h>

#include "tree_mesh_builder.h"

#define SQRT_DIV_2 sqrtf(3.f) / 2.f

TreeMeshBuilder::TreeMeshBuilder(unsigned gridEdgeSize)
        : BaseMeshBuilder(gridEdgeSize, "Octree") {

}

unsigned TreeMeshBuilder::octree(const ParametricScalarField &field, unsigned gridSize, const Vec3_t<float> &pos) {
    float halfGridSize = gridSize / 2.f;
    Vec3_t<float> center_pos(
            (pos.x + halfGridSize) * mGridResolution,
            (pos.y + halfGridSize) * mGridResolution,
            (pos.z + halfGridSize) * mGridResolution
    );
    if (evaluateFieldAt(center_pos, field) > mIsoLevel + (SQRT_DIV_2 * gridSize * mGridResolution)) {
        return 0.f;
    } else {
        if (gridSize > 1) {
            unsigned sum = 0;
            for (size_t i = 0; i < 8; i++) {
#pragma omp task shared(sum)
                {
                    Vec3_t<float> vertexNormPos = sc_vertexNormPos[i];
                    unsigned res_sum = octree(
                            field, halfGridSize,
                            Vec3_t<float>(pos.x + vertexNormPos.x * halfGridSize,
                                          pos.y + vertexNormPos.y * halfGridSize,
                                          pos.z + vertexNormPos.z * halfGridSize));
#pragma omp atomic update
                    sum += res_sum;
                }
            }
#pragma omp taskwait
            return sum;
        } else {
            return buildCube(pos, field);
        }
    }
}

unsigned TreeMeshBuilder::marchCubes(const ParametricScalarField &field) {
    // Suggested approach to tackle this problem is to add new method to
    // this class. This method will call itself to process the children.
    // It is also strongly suggested to first implement Octree as sequential
    // code and only when that works add OpenMP tasks to achieve parallelism.
    unsigned totalTriangles = 0;
#pragma omp parallel
    {
#pragma omp single nowait
        {
            totalTriangles = octree(field, mGridSize, Vec3_t<float>(0.0f, 0.0f, 0.0f));
        }
    }
    return totalTriangles;
}

float TreeMeshBuilder::evaluateFieldAt(const Vec3_t<float> &pos, const ParametricScalarField &field) {
    // NOTE: This method is called from "buildCube(...)"!

    // 1. Store pointer to and number of 3D points in the field
    //    (to avoid "data()" and "size()" call in the loop).
    const Vec3_t<float> *pPoints = field.getPoints().data();
    const auto count = unsigned(field.getPoints().size());

    float value = std::numeric_limits<float>::max();

    // 2. Find minimum square distance from points "pos" to any point in the
    //    field.
#pragma omp simd reduction(min:value) linear(pPoints) simdlen(16)
    for (unsigned i = 0; i < count; ++i) {
        float distanceSquared = (pos.x - pPoints[i].x) * (pos.x - pPoints[i].x);
        distanceSquared += (pos.y - pPoints[i].y) * (pos.y - pPoints[i].y);
        distanceSquared += (pos.z - pPoints[i].z) * (pos.z - pPoints[i].z);

        // Comparing squares instead of real distance to avoid unnecessary
        // "sqrt"s in the loop.
        value = (value < distanceSquared) ? value : distanceSquared;
    }

    // 3. Finally take square root of the minimal square distance to get the real distance
    return sqrt(value);
}

void TreeMeshBuilder::emitTriangle(const BaseMeshBuilder::Triangle_t &triangle) {
#pragma omp critical(emitTriangle)
    mTriangles.push_back(triangle);
//    mTriangles[omp_get_thread_num()].push_back(triangle);
}
