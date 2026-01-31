#pragma once
#include "Model.h"

class CubeModel : public Model
{
	unsigned m_number_of_indices = 0;

public:
	
	CubeModel(ID3D11Device* dxdevice, ID3D11DeviceContext* dxdevice_context, float size);

	virtual void CreateCube(float, std::vector<Vertex>&, std::vector<unsigned>&) const;

	virtual void CreateCubeFace(float, std::vector<Vertex>&, std::vector<unsigned>&, const vec3<float>&) const;
	
	virtual void Render() const;

	
	~CubeModel() {}
};