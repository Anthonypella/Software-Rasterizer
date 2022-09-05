#include "Camera.h"

Camera::Camera() {
	pos = Avector(0.0f, -1.0f, 0.5f);
	forward = Avector(0, 1.0f, 0);
	center = Avector(0, 0, 0);
	up = Avector(0, 0, 1.0);
	right = Avector(1.0, 0.0, 0.0);
	rotation = Avector(0, 0, 0);
	originDirection = Avector(0, -1, 0);
}
Camera::Camera(Avector pos1, Avector lookat1, Avector center1, Avector up1) {
	pos = pos1;
	forward = lookat1;
	center = center1;
	up = up1;
}
Camera::Camera(float* pos1, float* lookat1, float* center1, float* up1) {
	pos = pos1;
	forward = lookat1;
	center = center1;
	up = up1;
	forward.normalize();
	up.normalize();
}
void Camera::mouseInput(float x, float y) {
	pitch += y * lookSensitivity;
	pitch = clampf(-myPI / 8, myPI / 8, pitch);
	yaw += x * lookSensitivity;
	forward.x = cos(pitch) * sin(yaw);
	forward.y = cos(pitch)* cos(yaw);
	forward.z = -sin(pitch);
	forward.normalize();
	right.x = cos(yaw);
	right.y = -sin(yaw);
	right.z = 0;
	right.normalize();
	center.x = pos.x + forward.x;
	center.y = pos.y + forward.y;
	center.z = pos.z + forward.z;

}
void Camera::moveForward(float xMove, float yMove) {
	xMove *= speed;
	yMove *= speed;
	pos.x += xMove * right.x + yMove * forward.x;
	pos.y += xMove * right.y + yMove * forward.y;
	center.x = pos.x + forward.x;
	center.y = pos.y + forward.y;
	center.z = pos.z + forward.z;
}






Matrix Camera::getViewMatrix() {
	Avector L = center - pos;
	L.normalize();
	L._v[3] = 1;
	Avector S = L.cross(up);
	S.normalize();
	S._v[3] = 1;
	Avector Uprime = S.cross(L);
	Avector NegE = pos * -1;
	NegE._v[3] = 1;
	return Matrix(S, Uprime, L * -1, NegE);
}
void Camera::updateParams(float* pos1, float* lookat1, float* center1, float* up1) {
	if (false) {
		pos = Avector(pos1[0], -pos1[2], pos1[1]);
		forward = Avector(lookat1[0], lookat1[2], lookat1[1]);
		up = Avector(up[0], up[2], up[1]);
	}
	else {
		pos = Avector(-pos1[1], pos1[0], pos1[2]);
		forward = Avector(lookat1[1], lookat1[0], lookat1[2]);
		up = Avector(up[1], up[0], up[2]);
	}
	
	center = center1;
	
	forward.normalize();
	up.normalize();
}
void Camera::setTransform(float* position, float rx, float ry, float rz) {
	pos = position;
	rotation.x = rx;
	rotation.y = ry;
	rotation.z = rz;
}

Matrix Camera::getProjectionMatrix() {
	Matrix proj = Matrix::identity();
	Avector col1(1.0 / tan(FovX * DEG2RAD / 2.0f), 0, 0, 0);
	Avector col2(0, 1.0 / tan(vertFov*DEG2RAD / 2.0f), 0, 0);
	Avector col3(0, 0, -1.0f * ((nearPlane + farPlane) / (farPlane - nearPlane)), -1);
	Avector col4(0, 0, -2.0f * (farPlane * nearPlane) / (farPlane - nearPlane), 0);
	return Matrix(col1, col2, col3, col4);
	
}
Matrix Camera::toScreenSpaceMatrix() {
	Matrix Scale = Matrix::Scale((float)window_width / 2.0, (float)window_height / 2.0, 1.0f);
	Matrix Translation = Matrix::Translation((float)window_width / 2.0, (float)window_height / 2.0, 0);
	return Translation * Scale;
}
Matrix Camera::toScreenSpaceMatrix(float s) {
	Matrix Scale = Matrix::Scale(s, s, s);
	Matrix Translation = Matrix::Translation((float)window_width / 2.0, (float)window_height / 2.0, 0);
	return Translation*Scale;
}

Matrix Camera::getRotationMatrix() {
	calculateRotation();
	return (Matrix::RotationX(-(pitch-myPI/2.0f)) * Matrix::RotationY(0) * Matrix::RotationZ(yaw));
}
Matrix Camera::getTranslationMatrix() {
	return Matrix::Translation(-pos.x, -pos.y, -pos.z);
}
void Camera::calculateRotation() {
	Avector worldForward(0.0, 1.0, 0.0);
	float d = worldForward.dot(forward);
	Avector ground(forward);
	ground.z = 0;
	ground.normalize();
	float d2 = ground.dot(forward);
	rotation.x = asin(d);
	rotation.z = asin(d2);
	rotation.y = 0.0f;
}

Matrix Camera::getModelViewMatrix() {
	Avector Lookat = forward;
	Lookat.normalize();
	Avector S = Lookat.cross(up);
	S.normalize();
	Avector U = Lookat.cross(S);
	Avector lookatNegated = Lookat * -1.0;
	Avector lastRow(0.0, 0.0, 0.0, 1.0);
	S.w = S.dot(pos * -1.0f);
	U.w = U.dot(pos * -1.0f);
	lookatNegated.w = lookatNegated.dot(pos*-1.0f);
	Matrix R(true,S, U, lookatNegated, lastRow);
	
	return R;
}