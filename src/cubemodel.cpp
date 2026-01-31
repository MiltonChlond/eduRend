#include "CubeModel.h"

CubeModel::CubeModel(
	ID3D11Device* dxdevice,
	ID3D11DeviceContext* dxdevice_context,
	float size)
	: Model(dxdevice, dxdevice_context)
{
	
	std::vector<Vertex> vertices;
	std::vector<unsigned> indices;

	CreateCube(size, vertices, indices);

	// Vertex array descriptor
	D3D11_BUFFER_DESC vertexbufferDesc{ 0 };
	vertexbufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexbufferDesc.CPUAccessFlags = 0;
	vertexbufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexbufferDesc.MiscFlags = 0;
	vertexbufferDesc.ByteWidth = (UINT)(vertices.size() * sizeof(Vertex));
	// Data resource
	D3D11_SUBRESOURCE_DATA vertexData = { 0 };
	vertexData.pSysMem = &vertices[0];
	// Create vertex buffer on device using descriptor & data
	dxdevice->CreateBuffer(&vertexbufferDesc, &vertexData, &m_vertex_buffer);
	SETNAME(m_vertex_buffer, "VertexBuffer");

	//  Index array descriptor
	D3D11_BUFFER_DESC indexbufferDesc = { 0 };
	indexbufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexbufferDesc.CPUAccessFlags = 0;
	indexbufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexbufferDesc.MiscFlags = 0;
	indexbufferDesc.ByteWidth = (UINT)(indices.size() * sizeof(unsigned));
	// Data resource
	D3D11_SUBRESOURCE_DATA indexData{ 0 };
	indexData.pSysMem = &indices[0];
	// Create index buffer on device using descriptor & data
	dxdevice->CreateBuffer(&indexbufferDesc, &indexData, &m_index_buffer);
	SETNAME(m_index_buffer, "IndexBuffer");

	m_number_of_indices = (unsigned int)indices.size();
}

void CubeModel::CreateCube(float sideLength, std::vector<Vertex>& vertices, std::vector<unsigned>& indices) const
{
	float halfLength = sideLength / 2;
	CreateCubeFace(halfLength, vertices, indices, vec3<float>( 0, 0, 1 ));
	CreateCubeFace(halfLength, vertices, indices, vec3<float>( 0, 0, -1 ));
	CreateCubeFace(halfLength, vertices, indices, vec3<float>( 0, 1, 0 ));
	CreateCubeFace(halfLength, vertices, indices, vec3<float>( 0, -1, 0 ));
	CreateCubeFace(halfLength, vertices, indices, vec3<float>( 1, 0, 0 ));
	CreateCubeFace(halfLength, vertices, indices, vec3<float>( -1, 0, 0 ));
}

//creates 4 vertices to create a face of the cube, halfLength = half of the sideLength to keep the center of cube at origin (0,0,0)
void CubeModel::CreateCubeFace(float halfLength, std::vector<Vertex>& vertices, std::vector<unsigned>& indices, const vec3<float>& normal) const
{
	Vertex v1, v2, v3, v4;
	int indexStart = vertices.size();

	if (normal.z != 0)
	{
		float z = normal.z * halfLength; // if normal.z is negative the z of position will also be negative and vice versa for positive values
		v1.Position = { -halfLength, -halfLength, z };
		v2.Position = { halfLength, -halfLength, z };
		v3.Position = { halfLength, halfLength, z };
		v4.Position = { -halfLength, halfLength, z };
	}
	else if (normal.y != 0)
	{
		float y = normal.y * halfLength; 
		v1.Position = { -halfLength, y, -halfLength };
		v2.Position = { -halfLength, y, halfLength };
		v3.Position = { halfLength, y, halfLength };
		v4.Position = { halfLength, y, -halfLength };
	}
	else if (normal.x != 0)
	{
		float x = normal.x * halfLength;
		v1.Position = { x, -halfLength, -halfLength };
		v2.Position = { x, halfLength, -halfLength };
		v3.Position = { x, halfLength, halfLength };
		v4.Position = { x, -halfLength, halfLength };
	}

	v1.Normal = normal;
	v2.Normal = normal;
	v3.Normal = normal;
	v4.Normal = normal;

	v1.TexCoord = { 0,0 };
	v2.TexCoord = { 0,1 };
	v3.TexCoord = { 1,1 };
	v4.TexCoord = { 1,0 };

	vertices.push_back(v1);
	vertices.push_back(v2);
	vertices.push_back(v3);
	vertices.push_back(v4);

	if (normal.x > 0 || normal.y > 0 || normal.z > 0)
	{
		indices.push_back(indexStart + 0);
		indices.push_back(indexStart + 1);
		indices.push_back(indexStart + 3);

		indices.push_back(indexStart + 1);
		indices.push_back(indexStart + 2);
		indices.push_back(indexStart + 3);
	}
	else
	{
		indices.push_back(indexStart + 3);
		indices.push_back(indexStart + 1);
		indices.push_back(indexStart + 0);

		indices.push_back(indexStart + 3);
		indices.push_back(indexStart + 2);
		indices.push_back(indexStart + 1);
	}
}

void CubeModel::Render() const
{
	// Bind our vertex buffer
	const UINT32 stride = sizeof(Vertex); //  sizeof(float) * 8;
	const UINT32 offset = 0;
	m_dxdevice_context->IASetVertexBuffers(0, 1, &m_vertex_buffer, &stride, &offset);

	// Bind our index buffer
	m_dxdevice_context->IASetIndexBuffer(m_index_buffer, DXGI_FORMAT_R32_UINT, 0);

	// Make the drawcall
	m_dxdevice_context->DrawIndexed(m_number_of_indices, 0, 0);
}