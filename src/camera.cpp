#include "Camera.h"

using namespace linalg;

void Camera::MoveTo(const vec3f& position) noexcept
{
	m_position = position;
}

void Camera::Move(const vec3f& inputs, float& speed, float& dt) noexcept
{
	mat4f rotationMatrix = mat4f::rotation(0, m_rotation.x, m_rotation.y);

	vec3f forward = vec3f(rotationMatrix.m13, rotationMatrix.m23, rotationMatrix.m33);
	vec3f right = vec3f(rotationMatrix.m11, rotationMatrix.m21, rotationMatrix.m31);
	
	forward.y = 0;
	right.y = 0;
	forward = forward.normalize();
	right = right.normalize();

	vec3f upVec = vec3f(0, 1, 0);

	vec3f moveDirection = vec3f((right * inputs.x) + upVec * inputs.y + (forward * inputs.z));
	m_position += moveDirection * speed * dt;
}

void Camera::Rotate(const long& deltaX, const long& deltaY) noexcept
{
	float sensitivity = 0.001f;
	
	m_rotation.x -= deltaX * sensitivity;

	m_rotation.y -= deltaY * sensitivity;
	m_rotation.y = clamp(m_rotation.y, -1.5f, 1.5f);
}

mat4f Camera::WorldToViewMatrix() const noexcept
{
	// Assuming a camera's position and rotation is defined by matrices T(p) and R,
	// the View-to-World transform is T(p)*R (for a first-person style camera).
	//
	// World-to-View then is the inverse of T(p)*R;
	//		inverse(T(p)*R) = inverse(R)*inverse(T(p)) = transpose(R)*T(-p)
	// Since now there is no rotation, this matrix is simply T(-p)

	mat4f rotationMatrix = mat4f::rotation(0, m_rotation.x, m_rotation.y);
	mat4f translationMatrix = mat4f::translation(m_position);

	return rotationMatrix.inverse() * translationMatrix.inverse();

	return mat4f::translation(-m_position);
}

mat4f Camera::ProjectionMatrix() const noexcept
{
	return mat4f::projection(m_vertical_fov, m_aspect_ratio, m_near_plane, m_far_plane);
}