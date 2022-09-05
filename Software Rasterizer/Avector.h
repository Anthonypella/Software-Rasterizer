#ifndef AVECTOR_H
#define AVECTOR_H

#include <stdio.h>
#include <fstream> // for file access
#include <iostream>
#include <stdlib.h>
#include <sstream>
#include <string>
#include <vector>
#include <tuple>
#include <cmath>

using std::vector;
using std::tuple;

class Avector {

private:
	
public:
	union
	{
		float _v[4];

		struct
		{
			float x;
			float y;
			float z;
			float w;
		};
		 
	};
	
		// x, y, z, w;
	
	
	Avector();
	Avector(float);
	Avector(float, float, float);
	Avector(float*);
	Avector(float, float, float, float);
	Avector(vector<float>&);
	Avector(const Avector &);
	

	float dot(Avector&);
	Avector cross(Avector&);
	//operator overloads
	Avector operator+(Avector&);
	Avector operator-(Avector&);
	void operator=(Avector&);
	void operator=(float*);
	//Avector operator*(Avector&);
	
	//Operator Scalars
	Avector operator+(float);
	Avector operator-(float);
	Avector operator*(float);
	Avector operator/(float);
	float operator[](int);
	float magnitude();
	Avector hMultiply(float d) {
		return Avector(_v[0] * d, _v[1] * d, _v[2] * d, _v[3] * d);
	}
	Avector hAdd(Avector& V) {
		return Avector(_v[0] + V[0], _v[1] + V[1], _v[2] + V[2], _v[3]+V[3]);
	}
	float sqrMag();
	void normalize();
	Avector normalized();
	void homogenize();




};



#endif // AVECTOR_H

