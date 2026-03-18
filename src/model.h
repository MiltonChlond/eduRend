/**
 * @file model.h
 * @brief Contains model definitions
 * @author Carl Johan Gribel 2016, cjgribel@gmail.com
*/

#pragma once
#ifndef MODEL_H
#define MODEL_H

#include "stdafx.h"
#include <vector>
#include "vec\vec.h"
#include "vec\mat.h"
#include "Drawcall.h"
#include "OBJLoader.h"
#include "Texture.h"

using namespace linalg;

/**
 * @brief Abstract class. Defines the Render method and contains mesh data needed for a model.
*/

struct MaterialBuffer
{
	vec4f ambientColor;
	vec4f diffuseColor;
	vec4f specularColor;
	int IsSkybox;
	float3 padding;
};

class Model
{
protected:
	// Pointers to the current device and device context
	ID3D11Device* const			m_dxdevice; //!< Graphics device, use for creating resources.
	ID3D11DeviceContext* const	m_dxdevice_context; //!< Graphics context, use for binding resources and draw commands.

	// Pointers to the class' vertex & index arrays
	ID3D11Buffer* m_vertex_buffer = nullptr; //!< Pointer to gpu side vertex buffer
	ID3D11Buffer* m_index_buffer = nullptr; //!< Pointer to gpu side index buffer

	ID3D11Buffer* m_material_buffer = nullptr;
	Material m_material;

	mat4f transform;
	Model* parent = nullptr;

public:

	/**
	 * @brief Sets the protected member variables to the input params.
	 * @param dxdevice ID3D11Device to be used in the model.
	 * @param dxdevice_context ID3D11DeviceContext to be used in the model.
	*/
	Model(ID3D11Device* dxdevice, ID3D11DeviceContext* dxdevice_context) 
		:	m_dxdevice(dxdevice), m_dxdevice_context(dxdevice_context) 
	{
		HRESULT hr;
		D3D11_BUFFER_DESC matBuffer = { 0 };
		matBuffer.Usage = D3D11_USAGE_DYNAMIC;
		matBuffer.ByteWidth = sizeof(MaterialBuffer);
		matBuffer.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		matBuffer.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		matBuffer.MiscFlags = 0;
		matBuffer.StructureByteStride = 0;
		ASSERT(hr = m_dxdevice->CreateBuffer(&matBuffer, nullptr, &m_material_buffer));
	}

	void UpdateMatBuffer(int isSkybox)
	{
		D3D11_MAPPED_SUBRESOURCE resource;
		m_dxdevice_context->Map(m_material_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);

		MaterialBuffer* buffer = (MaterialBuffer*)resource.pData;

		buffer->ambientColor.x = m_material.AmbientColour.x;
		buffer->ambientColor.y = m_material.AmbientColour.y;
		buffer->ambientColor.z = m_material.AmbientColour.z;
		buffer->ambientColor.w = 0;

		buffer->diffuseColor.x = m_material.DiffuseColour.x;
		buffer->diffuseColor.y = m_material.DiffuseColour.y;
		buffer->diffuseColor.z = m_material.DiffuseColour.z;
		buffer->diffuseColor.w = 0;

		buffer->specularColor.x = m_material.SpecularColour.x;
		buffer->specularColor.y = m_material.SpecularColour.y;
		buffer->specularColor.z = m_material.SpecularColour.z;
		buffer->specularColor.w = 0.3f;

		buffer->IsSkybox = isSkybox;

		m_dxdevice_context->Unmap(m_material_buffer, 0);
	}

	void Calculate_TB(Vertex& v0, Vertex& v1, Vertex& v2)
	{
		vec3f D = v1.Position - v0.Position;
		vec3f E = v2.Position - v0.Position;
		vec2f F = v1.TexCoord - v0.TexCoord;
		vec2f G = v2.TexCoord - v0.TexCoord;

		float r = 1 / (F.x * G.y - F.y * G.x);

		vec3f T = (D * G.y - E * F.y) * r;
		vec3f B = (E * F.x - D * G.x) * r;

		v0.Tangent = v1.Tangent = v2.Tangent = normalize(T);
		v0.Binormal = v1.Binormal = v2.Binormal = normalize(B);
	}

	void SetParent(Model& par)
	{
		parent = &par;
	}

	void SetTransform(mat4f transformNew)
	{
		transform = transformNew;
	}

	mat4f GetTransform() const
	{
		if (parent != nullptr)
		{
			return parent->GetTransform() * transform;
		}
		return transform;
	}

	/**
	 * @brief Abstract render method: must be implemented by derived classes
	*/
	virtual void Render() = 0;

	/**
	 * @brief Destructor.
	 * @details Releases the vertex and index buffers of the Model.
	*/
	virtual ~Model()
	{ 
		SAFE_RELEASE(m_vertex_buffer);
		SAFE_RELEASE(m_index_buffer);
		SAFE_RELEASE(m_material_buffer);
	}
};

#endif