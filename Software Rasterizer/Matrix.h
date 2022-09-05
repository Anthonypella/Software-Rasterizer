#ifndef MATRIX_H
#define MATRIX_H

#include <stdio.h>
#include <fstream> // for file access
#include <iostream>
#include <stdlib.h>
#include <sstream>
#include <string>
#include <vector>
#include "Avector.h"
#include <tuple>
#include <cmath>

using std::vector;
using std::tuple;

class Matrix {
private:
	float _m[16];

public:	
	//constructors
	Matrix();
	Matrix(float);
	//Matrix(const Matrix &);
	Matrix(Avector, Avector, Avector, Avector);
	Matrix(bool columnFirst, Avector, Avector, Avector, Avector);

	//Avector getColumn(int index);
	//Operator overloads
	//Matrix operator+(Matrix&);
	//Matrix operator-(Matrix&);
	Matrix operator*(const Matrix&);
	Avector operator*(Avector&);
	float operator[](int);
	Matrix transpose();
	void transposeInPlace();

	
	//Operator Scalars
	//Matrix operator+(double);
	//Matrix operator-(double);
	//Matrix operator*(double);
	//Matrix operator/(double);

	static Matrix Translation(float x, float y, float z);
	static Matrix Scale(float x, float y, float z);
	static Matrix RotationX(float a) {
	
		Avector col1(1, 0, 0, 0);
		Avector col2(0, cos(a), -sin(a), 0);
		Avector col3(0, sin(a), cos(a), 0);
		Avector col4(0, 0, 0, 1);
		return Matrix(col1, col2, col3, col4);
	}
	
	static Matrix RotationY(float a) {
		
		Avector col1(cos(a), 0, sin(a), 0);
		Avector col2(0, 1, 0, 0);
		Avector col3(-sin(a), 0, cos(a), 0);
		Avector col4(0, 0, 0, 1);
		return Matrix(col1, col2, col3, col4);
	}
	static Matrix RotationZ(float a) {
		
		Avector col1(cos(a), sin(a), 0, 0);
		Avector col2(-sin(a), cos(a), 0,0);
		Avector col3(0,0, 1, 0);
		Avector col4(0, 0, 0, 1);
		return Matrix(col1, col2, col3, col4);
	}

	static Matrix identity() {
		Matrix m;
		m._m[0] = 1;
		m._m[5] = 1;
		m._m[10] = 1;
		m._m[15] = 1;
		return m;
	}


};





#endif