/**
 * @file    tree_mesh_builder.h
 *
 * @author  Simon Stupinsky <xstupi0000@stud.fit.vutbr.cz>
 *
 * @brief   Parallel Marching Cubes implementation using OpenMP tasks + octree early elimination
 *
 * @date    15.12.2019 10:00
 **/

#ifndef TREE_MESH_BUILDER_H
#define TREE_MESH_BUILDER_H

//#include <map>

#include "base_mesh_builder.h"

class TreeMeshBuilder : public BaseMeshBuilder {
public:
    TreeMeshBuilder(unsigned gridEdgeSize);

protected:
    unsigned octree(const ParametricScalarField &field, unsigned mGridSize, const Vec3_t<float> &pos);

    unsigned marchCubes(const ParametricScalarField &field);

    float evaluateFieldAt(const Vec3_t<float> &pos, const ParametricScalarField &field);

    void emitTriangle(const Triangle_t &triangle);

    const Triangle_t *getTrianglesArray() const { return mTriangles.data(); }

    std::vector <Triangle_t> mTriangles; ///< Temporary array of triangles

//    const Triangle_t *getTrianglesArray() const {
//        unsigned size = 0;
//        for (const auto &myPair : mTriangles) { size += myPair.second.size(); }
//        auto* res = new Triangle_t[size];
//        unsigned offset = 0;
////    #pragma omp parallel default(shared)
//            for (int i = 0; i < mTriangles.size(); i++) {
//                copyVectorToVector(mTriangles.at(i), res + offset);
//                offset += mTriangles.at(i).size();
//            }
//            return res;
//        };
//
//        void static copyVectorToVector(const std::vector<Triangle_t>& src, Triangle_t* dst) {
//            unsigned size = src.size();
//            for (unsigned i = 0; i < size; i++) {
//                dst[i] = src.at(i);
//            }
//        }
//
//        std::map<int, std::vector<Triangle_t> > mTriangles;
};

#endif // TREE_MESH_BUILDER_H
