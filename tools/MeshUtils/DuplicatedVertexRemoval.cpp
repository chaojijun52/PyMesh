#include "DuplicatedVertexRemoval.h"
#include <cassert>
#include <sstream>

#include <Core/Exception.h>
#include <Misc/HashGrid.h>

DuplicatedVertexRemoval::DuplicatedVertexRemoval(const MatrixFr& vertices, const MatrixIr& faces):
    m_vertices(vertices), m_faces(faces) {}

size_t DuplicatedVertexRemoval::run(Float tol) {
    const size_t dim = m_vertices.cols();
    HashGrid::Ptr grid = HashGrid::create(tol, dim);
    const size_t num_vertices = m_vertices.rows();
    const size_t num_faces = m_faces.rows();
    const size_t vertex_per_face = m_faces.cols();
    m_index_map.resize(num_vertices);

    size_t count = 0;
    size_t num_duplications = 0;
    for (size_t i=0; i<num_vertices; i++) {
        const VectorF& v = m_vertices.row(i);
        VectorI candidates = grid->get_items_near_point(v);
        if (candidates.size() == 0) {
            grid->insert(count, v);
            m_index_map[i] = count;
            count++;
        } else {
            if (candidates.size() > 1) {
                std::stringstream err_msg;
                err_msg << candidates.size()
                    << " vertices occupy a single cell" << std::endl;
                for (size_t j=0; j<candidates.size(); j++) {
                    err_msg << "index " << candidates[j] << ":\t<"
                        << m_vertices.row(candidates[j]) << " >" << std::endl;
                }
                throw RuntimeError(err_msg.str());
            }
            assert(candidates.size() == 1);
            m_index_map[i] = candidates[0];
            num_duplications++;
        }
    }

    MatrixFr vertices(count, dim);
    for (size_t i=0; i<num_vertices; i++) {
        vertices.row(m_index_map[i]) = m_vertices.row(i);
    }
    m_vertices = vertices;

    for (size_t i=0; i<num_faces; i++) {
        for (size_t j=0; j<vertex_per_face; j++) {
            size_t v_index = m_faces(i,j);
            m_faces(i,j) = m_index_map[v_index];
        }
    }
    return num_duplications;
}