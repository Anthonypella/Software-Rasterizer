#ifndef CAMERA_H
#define CAMERA_H
#include <iostream>
#include "Avector.h"
#include "Matrix.h"
#include "globals.h"
#include <cmath>
using namespace std;
class Camera {

public:
	float lookSensitivity = .01f;
	float speed = .1f;
	float pitch = 0.0f;
	float yaw = 0.0f;
	Avector pos;
	Avector forward;
	Avector center;
	Avector up;
	Avector right;
	Avector rotation;
	Avector originDirection;
	int screen_width = window_width;
	int screen_height = window_height;
	float nearPlane = .05f;
	float farPlane = 100.05f;
	float vertFov = 25.0f;
	float FovX = (vertFov * (float)screen_width) / (float)screen_height;

	Camera();
	Camera(Avector , Avector , Avector , Avector );
	Camera(float* , float* , float* , float* );
	void updateParams(float* pos1, float* lookat1, float* center1, float* up1);
	void setTransform(float* position, float rx, float ry, float rz);
	void calculateRotation();
	void increaseFOV(float amt) {
		vertFov += amt;
		FovX = (vertFov * (float)screen_width) / (float)screen_height;
	}
	void mouseInput(float x, float y);
	void moveForward(float xMove, float yMove);
	Matrix toScreenSpaceMatrix(float s);
	Matrix getViewMatrix();
	Matrix getProjectionMatrix();
	Matrix toScreenSpaceMatrix();
	Matrix getModelViewMatrix();
	Matrix getRotationMatrix();
	Matrix getTranslationMatrix();
	float clampf(float min, float max, float backGroundColor) {
		if (backGroundColor <= min) {
			return min;
		}
		if (backGroundColor >= max) {
			return max;
		}
		return backGroundColor;
	}
	

};










#endif // !CAMERA_H

