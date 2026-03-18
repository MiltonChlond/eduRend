#pragma once
#include "Model.h"

class Skybox : public Model
{
	unsigned m_number_of_indices = 0;

public:

	Skybox(ID3D11Device* dxdevice, ID3D11DeviceContext* dxdevice_context, float size);

	virtual void Render();

	virtual void CreateSkybox(float, std::vector<Vertex>&, std::vector<unsigned>&) const;

	virtual void CreateSkyboxFace(float, std::vector<Vertex>&, std::vector<unsigned>&, const vec3<float>&) const;

	~Skybox();
};