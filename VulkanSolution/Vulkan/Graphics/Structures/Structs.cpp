#include "Structs.h"

Mesh::Mesh()
{
}

Mesh::Mesh(const std::string& name, const std::vector<uint32_t>& indices, const std::vector<Vertex>& vertices)
	: meshName(name), indices(indices), vertices(vertices)
{
}

Mesh::Mesh(const Mesh& m)
	:meshName(m.meshName), indices(m.indices), vertices(m.vertices)
{
}

Mesh::Mesh(Mesh&& m)
	: meshName(m.meshName), indices(m.indices), vertices(m.vertices)
{
}

Mesh& Mesh::operator=(const Mesh& m)
{
	meshName = m.meshName;
	indices = m.indices;
	vertices = m.vertices;
	return *this;
}

Mesh& Mesh::operator=(Mesh&& m)
{
	meshName = m.meshName;
	indices = m.indices;
	vertices = m.vertices;
	return *this;
}
