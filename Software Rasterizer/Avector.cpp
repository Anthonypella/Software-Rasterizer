#include "Avector.h"
using namespace std;
Avector::Avector() {
	for each (float var in _v)
	{
		var = 0;
	}
	_v[3] = 1;
}
Avector::Avector(float f) {
	_v[0] = f;
	_v[1] = f;
	_v[2] = f;
	_v[3] = f;
}
Avector::Avector(float x,float y,float z) {
	_v[0] = x;
	_v[1] = y;
	_v[2] = z;
	_v[3] = 1;
}
Avector::Avector(float* arr) {
	_v[0] = arr[0];
	_v[1] = arr[1];
	_v[2] = arr[2];
}
Avector::Avector(float x, float y, float z,float w) {
	_v[0] = x;
	_v[1] = y;
	_v[2] = z;
	_v[3] = w;
}
Avector::Avector(vector<float>& copyVect) {
	for (int i = 0; i < 4; i++)
	{
		_v[i] = copyVect[i];
	}
}
Avector::Avector(const Avector& vect) {
	for (int i = 0; i < 4; i++)
	{
		_v[i] = vect._v[i];
	}
}

float Avector::dot(Avector& otherVect) {
	float dot = 0;
	for (size_t i = 0; i < 3; i++)
	{
		dot += _v[i] * otherVect._v[i];
	}
	return dot;
}
Avector Avector::cross(Avector& V) {
	float x = _v[1] * V[2] - _v[2] * V[1];
	float y = _v[2] * V[0] - _v[0] * V[2];
	float z = _v[0] * V[1] - _v[1] * V[0];
	return Avector(x, y, x);
}
Avector Avector::operator+(Avector& V) {
	return Avector(_v[0] + V[0], _v[1] + V[1], _v[2] + V[2], 1);
}
Avector Avector::operator-(Avector& V) {
	return Avector(_v[0] - V.x, _v[1] - V.y, _v[2] - V.z, 1);
}
void Avector::operator=(Avector& V) {
	for (int i = 0; i < 4; i++)
	{
		_v[i] = V[i];
	}
}
void Avector::operator=(float* V) {
	for (int i = 0; i < 3; i++)
	{
		_v[i] = V[i];
	}
}
Avector Avector::operator+(float d) {
	return Avector(_v[0] +d, _v[1] + d, _v[2] + d, 1);
}
Avector Avector::operator-(float d) {
	return Avector(_v[0] - d, _v[1] - d, _v[2] - d, 1);
}
Avector Avector::operator*(float d) {
	return Avector(_v[0] *d, _v[1] *d, _v[2] *d, 1);
}
Avector Avector::operator/(float d) {
	float recip = 1.0f / d;
	return Avector(_v[0] * recip, _v[1] * recip, _v[2] * recip, 1);
}

float Avector::operator[](int index) {
	return _v[index];
}



float Avector::magnitude() {
	float mag = 0.0;
	mag += _v[0] * _v[0] + _v[1] * _v[1] + _v[2] * _v[2];
	return sqrt(mag);
}
float Avector::sqrMag() {
	float mag = 0.0;
	mag += _v[0] * _v[0] + _v[1] * _v[1] + _v[2] * _v[2];
	return mag;
}
void Avector::normalize() {
	float mag = magnitude();
	x /= mag;
	y /= mag;
	z /= mag;
}
Avector Avector::normalized() {
	float r = 1.0f / magnitude();
	return Avector(_v[0] * r, _v[1] * r, _v[2] * r);
}
void Avector::homogenize() {
	x /= w;
	y /= w;
	z /= w;
	w = 1.0f;
}