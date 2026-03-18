
#include "Scene.h"
#include "QuadModel.h"
#include "cubemodel.h"
#include "OBJModel.h"
#include "Skybox.h"

Scene::Scene(
	ID3D11Device* dxdevice,
	ID3D11DeviceContext* dxdevice_context,
	int window_width,
	int window_height) :
	m_dxdevice(dxdevice),
	m_dxdevice_context(dxdevice_context),
	m_window_width(window_width),
	m_window_height(window_height)
{ }

void Scene::OnWindowResized(
	int new_width,
	int new_height)
{
	m_window_width = new_width;
	m_window_height = new_height;
}

OurTestScene::OurTestScene(
	ID3D11Device* dxdevice,
	ID3D11DeviceContext* dxdevice_context,
	int window_width,
	int window_height) :
	Scene(dxdevice, dxdevice_context, window_width, window_height)
{ 
	InitTransformationBuffer();
	InitLightCameraBuffer();
	InitSampler();
	// + init other CBuffers
}

//
// Called once at initialization
//
void OurTestScene::Init()
{
	m_camera = new Camera(
		45.0f * fTO_RAD,		// field-of-view (radians)
		(float)m_window_width / m_window_height,	// aspect ratio
		0.01f,					// z-near plane (everything closer will be clipped/removed)
		500.0f);				// z-far plane (everything further will be clipped/removed)

	// Move camera to (0,0,5)
	m_camera->MoveTo({ 0, 0, 5 });

	// Create objects
	m_quad = new QuadModel(m_dxdevice, m_dxdevice_context);
	m_cube_sun = new CubeModel(m_dxdevice, m_dxdevice_context, 0.8f);
	m_cube_earth = new CubeModel(m_dxdevice, m_dxdevice_context, 0.5f);
	m_cube_moon = new CubeModel(m_dxdevice, m_dxdevice_context, 0.2f);
	m_sponza = new OBJModel("assets/crytek-sponza/sponza.obj", m_dxdevice, m_dxdevice_context);

	skyBox = new Skybox(m_dxdevice, m_dxdevice_context, 450);

	/*const char* cubeFilenames[6] =
	{
		"assets/cubemaps/debug_cubemap/debug_posx.png",
		"assets/cubemaps/debug_cubemap/debug_negx.png",
		"assets/cubemaps/debug_cubemap/debug_posy.png",
		"assets/cubemaps/debug_cubemap/debug_negy.png",
		"assets/cubemaps/debug_cubemap/debug_posz.png",
		"assets/cubemaps/debug_cubemap/debug_negz.png",
	};*/
	const char* cubeFilenames[6] =
	{
		"assets/cubemaps/brightday/posx.png",
		"assets/cubemaps/brightday/negx.png",
		"assets/cubemaps/brightday/posy.png",
		"assets/cubemaps/brightday/negy.png",
		"assets/cubemaps/brightday/posz.png",
		"assets/cubemaps/brightday/negz.png",
	};

	HRESULT hr = LoadCubeTextureFromFile(
		m_dxdevice,
		cubeFilenames,
		&cubeTex);

	if (SUCCEEDED(hr)) std::cout << "Cubemap OK" << std::endl;
	else std::cout << "Cubemap failed to load" << std::endl;

	m_dxdevice_context->PSSetShaderResources(2, 1, &cubeTex.TextureView);
}

//
// Called every frame
// dt (seconds) is time elapsed since the previous frame
//
void OurTestScene::Update(
	float dt,
	const InputHandler& input_handler)
{

	//light animation
	if (m_light_position.z >= 10)
	{
		lightMoveVector = { 0, 0, -5 };
	}
	else if (m_light_position.z <= -10)
	{
		lightMoveVector = { 0, 0, 5 };
	}
	m_light_position += lightMoveVector * dt;


	// Basic camera control
	vec3f inputs = { 0, 0, 0 };

	if (input_handler.IsKeyPressed(Keys::Up) || input_handler.IsKeyPressed(Keys::W))
		inputs.z += -1;
	if (input_handler.IsKeyPressed(Keys::Down) || input_handler.IsKeyPressed(Keys::S))
		inputs.z += 1;
	if (input_handler.IsKeyPressed(Keys::Right) || input_handler.IsKeyPressed(Keys::D))
		inputs.x += 1;
	if (input_handler.IsKeyPressed(Keys::Left) || input_handler.IsKeyPressed(Keys::A))
		inputs.x += -1;
	if (input_handler.IsKeyPressed(Keys::Space))
		inputs.y += 1;
	if (input_handler.IsKeyPressed(Keys::LCtrl))
		inputs.y += -1;
	if(input_handler.IsKeyPressed(Keys::Esc))
		PostQuitMessage(0);

	m_camera->Move(inputs, m_camera_velocity, dt);

	//camera rotation
	m_camera->Rotate(input_handler.GetMouseDeltaX(), input_handler.GetMouseDeltaY());

	if (input_handler.IsKeyPressed(Keys::P))
	{
		UpdateSamplerState(1);
	}
	if (input_handler.IsKeyPressed(Keys::O))
	{
		UpdateSamplerState(2);
	}
	if (input_handler.IsKeyPressed(Keys::I))
	{
		UpdateSamplerState(0);
	}

	skyBox->SetTransform(mat4f::translation(m_camera->Position()) *
						mat4f::rotation(0, 0.0f, 0.0f, 0.0f) *
						mat4f::scaling(1, 1, 1));

	m_cube_sun->SetTransform(mat4f::translation(0, 0, 0) *
							mat4f::rotation(-m_angle, 0.0f, 1.0f, 0.0f) *
							mat4f::scaling(1, 1, 1));
	
	m_cube_earth->SetTransform(mat4f::translation(5, 0, 0) *
							   mat4f::rotation(-m_angle, 0, 1, 0) *
						       mat4f::scaling(1, 1, 1));
	m_cube_earth->SetParent(*m_cube_sun);


	m_cube_moon->SetTransform(mat4f::translation(1, 0, 0) *
							  mat4f::rotation(-m_angle, 0, 1, 0) *
							  mat4f::scaling(1, 1, 1));
	m_cube_moon->SetParent(*m_cube_earth);
	

	m_sponza->SetTransform(mat4f::translation(0, -5, 0) *
						   mat4f::rotation(fPI / 2, 0.0f, 1.0f, 0.0f) *
						   mat4f::scaling(0.05f));

	// Increment the rotation angle.
	m_angle += m_angular_velocity * dt;

	// Print fps
	m_fps_cooldown -= dt;
	if (m_fps_cooldown < 0.0)
	{
		std::cout << "fps " << (int)(1.0f / dt) << std::endl;
//		printf("fps %i\n", (int)(1.0f / dt));
		m_fps_cooldown = 2.0;
	}
}

//
// Called every frame, after update
//
void OurTestScene::Render()
{
	UpdateLightCameraBuffer();

	// Bind transformation_buffer to slot b0 of the VS
	m_dxdevice_context->VSSetConstantBuffers(0, 1, &m_transformation_buffer);
	m_dxdevice_context->PSSetConstantBuffers(2, 1, &m_lightcamera_buffer);

	m_dxdevice_context->PSSetSamplers(0, 1, &sampler);

	// Obtain the matrices needed for rendering from the camera
	m_view_matrix = m_camera->WorldToViewMatrix();
	m_projection_matrix = m_camera->ProjectionMatrix();

	// Load matrices + the Quad's transformation to the device and render it
	/*UpdateTransformationBuffer(m_quad_transform, m_view_matrix, m_projection_matrix);
	m_quad->Render();*/

	UpdateTransformationBuffer(skyBox->GetTransform(), m_view_matrix, m_projection_matrix);
	skyBox->Render();

	//cube
	UpdateTransformationBuffer(m_cube_sun->GetTransform(), m_view_matrix, m_projection_matrix);
	m_cube_sun->Render();

	UpdateTransformationBuffer(m_cube_earth->GetTransform(), m_view_matrix, m_projection_matrix);
	m_cube_earth->Render();

	UpdateTransformationBuffer(m_cube_moon->GetTransform(), m_view_matrix, m_projection_matrix);
	m_cube_moon->Render();

	// Load matrices + Sponza's transformation to the device and render it
	UpdateTransformationBuffer(m_sponza->GetTransform(), m_view_matrix, m_projection_matrix);
	m_sponza->Render();
}

void OurTestScene::Release()
{
	SAFE_DELETE(m_quad);
	SAFE_DELETE(m_cube_sun);
	SAFE_DELETE(m_cube_earth);
	SAFE_DELETE(m_cube_moon);
	SAFE_DELETE(m_sponza);
	SAFE_DELETE(m_camera);

	SAFE_DELETE(skyBox);

	SAFE_RELEASE(m_transformation_buffer);
	SAFE_RELEASE(m_lightcamera_buffer);
	SAFE_RELEASE(samplerAni);
	SAFE_RELEASE(samplerPoint);
	SAFE_RELEASE(samplerLinear);

	SAFE_RELEASE(cubeTex.TextureView);
	// + release other CBuffers
}

void OurTestScene::OnWindowResized(
	int new_width,
	int new_height)
{
	if (m_camera)
		m_camera->SetAspect(float(new_width) / new_height);

	Scene::OnWindowResized(new_width, new_height);
}

void OurTestScene::InitSampler()
{
	//sampler
	D3D11_SAMPLER_DESC sampDescAni{};
	sampDescAni.Filter = D3D11_FILTER_ANISOTROPIC;
	sampDescAni.MaxAnisotropy = 16;
	sampDescAni.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDescAni.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDescAni.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDescAni.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDescAni.MinLOD = 0;
	sampDescAni.MaxLOD = D3D11_FLOAT32_MAX;
	m_dxdevice->CreateSamplerState(&sampDescAni, &samplerAni);

	D3D11_SAMPLER_DESC sampDescP{};
	sampDescP.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	sampDescP.MaxAnisotropy = 16;
	sampDescP.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDescP.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDescP.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDescP.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDescP.MinLOD = 0;
	sampDescP.MaxLOD = D3D11_FLOAT32_MAX;
	m_dxdevice->CreateSamplerState(&sampDescP, &samplerPoint);

	D3D11_SAMPLER_DESC sampDescL{};
	sampDescL.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDescL.MaxAnisotropy = 16;
	sampDescL.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDescL.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDescL.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDescL.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDescL.MinLOD = 0;
	sampDescL.MaxLOD = D3D11_FLOAT32_MAX;
	m_dxdevice->CreateSamplerState(&sampDescL, &samplerLinear);

	sampler = samplerAni;
}

void OurTestScene::UpdateSamplerState(int i)
{
	if (i == 0)
	{
		sampler = samplerAni;
	}
	if (i == 1)
	{
		sampler = samplerPoint;
	}
	if (i == 2)
	{
		sampler = samplerLinear;
	}
}

void OurTestScene::InitTransformationBuffer()
{
	HRESULT hr;
	D3D11_BUFFER_DESC matrixBufferDesc = { 0 };
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(TransformationBuffer);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;
	ASSERT(hr = m_dxdevice->CreateBuffer(&matrixBufferDesc, nullptr, &m_transformation_buffer));
}

void OurTestScene::InitLightCameraBuffer()
{
	HRESULT hr;
	D3D11_BUFFER_DESC LCBufferDesc = { 0 };
	LCBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	LCBufferDesc.ByteWidth = sizeof(LightCameraBuffer);
	LCBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	LCBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	LCBufferDesc.MiscFlags = 0;
	LCBufferDesc.StructureByteStride = 0;
	ASSERT(hr = m_dxdevice->CreateBuffer(&LCBufferDesc, nullptr, &m_lightcamera_buffer));
}

void OurTestScene::UpdateTransformationBuffer(
	mat4f ModelToWorldMatrix,
	mat4f WorldToViewMatrix,
	mat4f ProjectionMatrix)
{
	// Map the resource buffer, obtain a pointer and then write our matrices to it
	D3D11_MAPPED_SUBRESOURCE resource;
	m_dxdevice_context->Map(m_transformation_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
	TransformationBuffer* matrixBuffer = (TransformationBuffer*)resource.pData;
	matrixBuffer->ModelToWorldMatrix = ModelToWorldMatrix;
	matrixBuffer->WorldToViewMatrix = WorldToViewMatrix;
	matrixBuffer->ProjectionMatrix = ProjectionMatrix;
	m_dxdevice_context->Unmap(m_transformation_buffer, 0);
}

void OurTestScene::UpdateLightCameraBuffer()
{
	D3D11_MAPPED_SUBRESOURCE resource;
	m_dxdevice_context->Map(m_lightcamera_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);

	LightCameraBuffer* buffer = (LightCameraBuffer*)resource.pData;
	buffer->lightPos = vec4f(m_light_position.x, m_light_position.y, m_light_position.z, 1.0f);

	vec3f camPos = m_camera->Position();
	buffer->camPos = vec4f(camPos.x, camPos.y, camPos.z, 1.0f);

	m_dxdevice_context->Unmap(m_lightcamera_buffer, 0);
}