#include "render/Mesh.h"

void Mesh::Create(const std::vector<Vertex> &vertices)
{
    count = static_cast<GLsizei>(vertices.size());
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(vertices.size() * sizeof(Vertex)),
                 vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void *>(offsetof(Vertex, position)));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void *>(offsetof(Vertex, normal)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void *>(offsetof(Vertex, uv)));
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);
}

void Mesh::Draw() const
{
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, count);
}

void Mesh::Destroy()
{
    if (vbo) { glDeleteBuffers(1, &vbo); }
    if (vao) { glDeleteVertexArrays(1, &vao); }
    vao = vbo = 0;
    count = 0;
}

std::vector<Vertex> MakeCubeVertices()
{
    // 면마다 (법선 n, 접선 u, 접선 v). u×v = n 이므로 (p00,p10,p11)이 바깥에서 볼 때 CCW.
    struct Face { glm::vec3 n, u, v; };
    const Face faces[]{
        {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}},  {{-1, 0, 0}, {0, 0, 1}, {0, 1, 0}},
        {{0, 1, 0}, {0, 0, 1}, {1, 0, 0}},  {{0, -1, 0}, {1, 0, 0}, {0, 0, 1}},
        {{0, 0, 1}, {1, 0, 0}, {0, 1, 0}},  {{0, 0, -1}, {0, 1, 0}, {1, 0, 0}}};
    std::vector<Vertex> vertices;
    for (const Face &f : faces)
    {
        glm::vec3 center = f.n * 0.5f;
        Vertex p00{center - f.u * 0.5f - f.v * 0.5f, f.n, {0, 0}};
        Vertex p10{center + f.u * 0.5f - f.v * 0.5f, f.n, {1, 0}};
        Vertex p11{center + f.u * 0.5f + f.v * 0.5f, f.n, {1, 1}};
        Vertex p01{center - f.u * 0.5f + f.v * 0.5f, f.n, {0, 1}};
        for (const Vertex &v : {p00, p10, p11, p00, p11, p01}) { vertices.push_back(v); }
    }
    return vertices;
}

std::vector<Vertex> MakeFloorVertices()
{
    const glm::vec3 up(0, 1, 0);
    Vertex v0{{-1, 0, 0}, up, {0, 0}}, v1{{-1, 0, 1}, up, {0, 1}};
    Vertex v2{{1, 0, 1}, up, {1, 1}}, v3{{1, 0, 0}, up, {1, 0}};
    return {v0, v1, v2, v0, v2, v3};
}

std::vector<Vertex> MakeQuadVertices()
{
    const glm::vec3 n(0, 0, 1);
    Vertex a{{0, 0, 0}, n, {0, 0}}, b{{1, 0, 0}, n, {1, 0}};
    Vertex c{{1, 1, 0}, n, {1, 1}}, d{{0, 1, 0}, n, {0, 1}};
    return {a, b, c, a, c, d};
}
