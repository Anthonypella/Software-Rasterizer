#include "Matrix.h"
using namespace std;

Matrix::Matrix() {
	for (int i = 0; i < 16; i++)
	{
		_m[i] = 0.0f;
	}
}
Matrix::Matrix(float f) {
	for (int i = 0; i < 16; i++)
	{
		_m[i] = f;
	}
}

Matrix::Matrix(Avector col1, Avector col2, Avector col3, Avector col4) {
	_m[0] = col1[0];
	_m[1] = col2[0];
	_m[2] = col3[0];
	_m[3] = col4[0];
	_m[4] = col1[1];
	_m[5] = col2[1];
	_m[6] = col3[1];
	_m[7] = col4[1];
	_m[8] = col1[2];
	_m[9] = col2[2];
	_m[10] = col3[2];
	_m[11] = col4[2];
	_m[12] = col1[3];
	_m[13] = col2[3];
	_m[14] = col3[3];
	_m[15] = col4[3];
}
Matrix::Matrix(bool row,Avector col1, Avector col2, Avector col3, Avector col4) {
	_m[0] = col1[0];
	_m[1] = col1[1];
	_m[2] = col1[2];
	_m[3] = col1[3];
	_m[4] = col2[0];
	_m[5] = col2[1];
	_m[6] = col2[2];
	_m[7] = col2[3];
	_m[8] = col3[0];
	_m[9] = col3[1];
	_m[10] = col3[2];
	_m[11] = col3[3];
	_m[12] = col4[0];
	_m[13] = col4[1];
	_m[14] = col4[2];
	_m[15] = col4[3];
}


Matrix Matrix::operator*(const Matrix& src2) {
	Matrix dest;
	dest._m[0] = _m[0] * src2._m[0] + _m[1] * src2._m[4] + _m[2] * src2._m[8] + _m[3] * src2._m[12];
	dest._m[1] = _m[0] * src2._m[1] + _m[1] * src2._m[5] + _m[2] *src2._m[9] + _m[3] * src2._m[13];
	dest._m[2] = _m[0] * src2._m[2] + _m[1] * src2._m[6] + _m[2] *src2._m[10] + _m[3] * src2._m[14];
	dest._m[3] = _m[0] * src2._m[3] + _m[1] * src2._m[7] + _m[2] *src2._m[11] + _m[3] * src2._m[15];
	dest._m[4] = _m[4] * src2._m[0] + _m[5] * src2._m[4] + _m[6] *src2._m[8] + _m[7] * src2._m[12];
	dest._m[5] = _m[4] * src2._m[1] + _m[5] * src2._m[5] + _m[6] *src2._m[9] + _m[7] * src2._m[13];
	dest._m[6] = _m[4] * src2._m[2] + _m[5] * src2._m[6] + _m[6] *src2._m[10] + _m[7] * src2._m[14];
	dest._m[7] = _m[4] * src2._m[3] + _m[5] * src2._m[7] + _m[6] *src2._m[11] + _m[7]* src2._m[15];
	dest._m[8] = _m[8] * src2._m[0] + _m[9] * src2._m[4] + _m[10] * src2._m[8] + _m[11] * src2._m[12];
	dest._m[9] = _m[8] * src2._m[1] + _m[9] * src2._m[5] + _m[10] * src2._m[9] + _m[11]* src2._m[13];
	dest._m[10] = _m[8] *src2._m[2] + _m[9] * src2._m[6] + _m[10] * src2._m[10] + _m[11] * src2._m[14];
	dest._m[11] = _m[8] *src2._m[3] + _m[9] * src2._m[7] + _m[10] * src2._m[11] + _m[11] * src2._m[15];
	dest._m[12] = _m[12] *src2._m[0] + _m[13] * src2._m[4] + _m[14] * src2._m[8] + _m[15] * src2._m[12];
	dest._m[13] = _m[12] * src2._m[1] + _m[13] * src2._m[5] + _m[14] * src2._m[9] + _m[15] * src2._m[13];
	dest._m[14] = _m[12] *src2._m[2] + _m[13] * src2._m[6] + _m[14] * src2._m[10] + _m[15] * src2._m[14];
	dest._m[15] = _m[12] *src2._m[3] + _m[13] * src2._m[7] + _m[14] * src2._m[11] + _m[15] * src2._m[15];
	return dest;
}
Avector Matrix::operator*(Avector& vect) {
	Avector dest;
	dest._v[0] = _m[0] * vect[0] + _m[1] * vect[1] + _m[2] * vect[2] + _m[3] * vect[3];
	dest._v[1] = _m[4] * vect[0] + _m[5] * vect[1] + _m[6] * vect[2] + _m[7] * vect[3];
	dest._v[2] = _m[8] * vect[0] + _m[9] * vect[1] + _m[10] * vect[2] + _m[11] * vect[3];
	dest._v[3] = _m[12] * vect[0] + _m[13] * vect[1] + _m[14] * vect[2] + _m[15] * vect[3];
	return dest;
}
float Matrix::operator[](int index) {
	return _m[index];
}
Matrix Matrix::transpose() {
	Matrix out;
	out._m[0] = _m[0];  // A . . .
	out._m[1] = _m[4];  // A E . .
	out._m[2] = _m[8];  // A E I .
	out._m[3] = _m[12]; // A E I M
	out._m[4] = _m[1];  // B . . .
	out._m[5] = _m[5];  // B F . .
	out._m[6] = _m[9];  // B F J .
	out._m[7] = _m[13]; // B F J N
	out._m[8] = _m[2];  // C . . .
	out._m[9] = _m[6];  // C G . .
	out._m[10] = _m[10]; // C G K .
	out._m[11] = _m[14]; // C G K O
	out._m[12] = _m[3];  // D . . .
	out._m[13] = _m[7];  // D H . .
	out._m[14] = _m[11]; // D H L .
	out._m[15] = _m[15]; // D H L P
	return out;
}
void Matrix::transposeInPlace() {	
	swap(_m[1], _m[4]);
	swap(_m[2], _m[8]);
	swap(_m[3], _m[12]);
	swap(_m[6], _m[9]);
	swap(_m[7], _m[13]);
	swap(_m[11], _m[14]);
}

Matrix Matrix::Translation(float x, float y, float z) 
{
	Matrix m = Matrix::identity();
	m._m[3] = x;
	m._m[7] = y;
	m._m[11] = z;
	return m;
}
Matrix Matrix::Scale(float x, float y, float z) {
	Matrix m = Matrix::identity();
	m._m[0] = x;
	m._m[5] = y;
	m._m[10] = z;
	return m;
}