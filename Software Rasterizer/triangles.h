#ifndef TRIANGLES_H
#define TRIANGLES_H
#include <Windows.h>
#include "Avector.h"
#include "Matrix.h"
#include "Camera.h"
using namespace std;
struct vertex
{
	Avector position;
	RGBTRIPLE color;
	float u;
	float v;
	int textureIndex;
	float projScale = 1.0;
	vertex() {
		
	}
	vertex(Avector& a) {
		position = a;
		u = 1.0f;
		v = 1.0f;
		textureIndex = 0;
	}
	vertex(Avector& a, char r, char g, char b, float uu, float vv) {
		position = a;
		u = uu;
		v = vv;
		color.rgbtRed = r;
		color.rgbtGreen = g;
		color.rgbtBlue = b;
		textureIndex = 0;
	}
	vertex(Avector& a, int texIndex, float uu, float vv) {
		position = a;
		u = uu;
		v = vv;
		textureIndex = texIndex;
	}
	vertex(Avector& a, vertex& copy) {
		position = a;
		u = copy.u;
		v = copy.v;
		color.rgbtRed = copy.color.rgbtRed;
		color.rgbtGreen = copy.color.rgbtGreen;
		color.rgbtBlue = copy.color.rgbtBlue;
		textureIndex = copy.textureIndex;
	}
	vertex(vertex& p1, vertex& p2, float t) {
		t = 1.0f - t;
		position = (p1.position.hMultiply(t)).hAdd( (p2.position.hMultiply(1.0f-t)));
		u = (p1.u * t) + (p2.u * (1.0f - t));
		v = (p1.v * t) + (p2.v * (1.0f - t));
		projScale = (p1.projScale * t) + (p2.projScale * (1.0f - t));
		textureIndex = p1.textureIndex;
	}
	void Swap(vertex& swap) {
		Avector temp = swap.position;
		swap.position = position;
		position = temp;


		uint8_t tempc = swap.color.rgbtBlue;
		swap.color.rgbtBlue = color.rgbtBlue;
		color.rgbtBlue = tempc;
		tempc = swap.color.rgbtGreen;
		swap.color.rgbtGreen = color.rgbtGreen;
		color.rgbtGreen = tempc;
		tempc = swap.color.rgbtRed;
		swap.color.rgbtRed = color.rgbtRed;
		color.rgbtRed = tempc;

		float ftemp = swap.u;
		swap.u = u;
		u = ftemp;

		ftemp = swap.v;
		swap.v = v;
		v = ftemp;

		ftemp = swap.projScale;
		swap.projScale = projScale;
		projScale = ftemp;
	}

};

class triangles {
public:
	vector<int> indexList;
	vector<vertex> vertexList;
	vector<int> transformedIndexList;
	vector<vertex> transformedVertexList;
	vector<vertex> clippedVertexList;
	vector<int> clippedIndices;
	Avector clippingPlanes[6];
	Avector clippingPlanePoints[6];

	triangles() {
		//vertexList.resize(100);
		//triangleIndexList.resize(300);
		clippingPlanes[0] = Avector(1, 0, 0); //R
		clippingPlanes[1] = Avector(-1.0f, 0, 0); //L
		clippingPlanes[2] = Avector(0, 1, 0); //Near
		clippingPlanes[3] = Avector(0, -1, 0); //Far
		clippingPlanes[4] = Avector(0, 0, 1); //Bottom
		clippingPlanes[5] = Avector(0, 0, -1); //top

		clippingPlanePoints[0] = Avector(-1, 0, 0); //R
		clippingPlanePoints[1] = Avector(.5, 0, 0); //L
		clippingPlanePoints[2] = Avector(0, -1, 0); //Near
		clippingPlanePoints[3] = Avector(0, 1, 0); //Far
		clippingPlanePoints[4] = Avector(0, 0, -1); //Bottom
		clippingPlanePoints[5] = Avector(0, 0, 1); //top
	}
	triangles(int maxSize) {
		indexList.reserve(maxSize);
		vertexList.reserve(maxSize);
		transformedIndexList.reserve(maxSize);
		transformedVertexList.reserve(maxSize);
		clippedVertexList.reserve(maxSize * 2);
		clippedIndices.reserve(maxSize * 2);

		clippingPlanes[0] = Avector(1, 0, 0); //R
		clippingPlanes[1] = Avector(-1, 0, 0); //L
		clippingPlanes[2] = Avector(0, 1, 0); //Near
		clippingPlanes[3] = Avector(0, -1, 0); //Far
		clippingPlanes[4] = Avector(0, 0, 1); //Bottom
		clippingPlanes[5] = Avector(0, 0, -1); //top

		clippingPlanePoints[0] = Avector(-1, 0, 0); //R
		clippingPlanePoints[1] = Avector(1, 0, 0); //L
		clippingPlanePoints[2] = Avector(0, -1, 0); //Near
		clippingPlanePoints[3] = Avector(0,1, 0); //Far
		clippingPlanePoints[4] = Avector(0, 0, -1); //Bottom
		clippingPlanePoints[5] = Avector(0, 0, 1); //top
	}				  


	void addTriangle(Avector& vert1, Avector& vert2, Avector& vert3,
		tuple<float, float> uv1, tuple<float, float> uv2, tuple<float, float> uv3, int textureIndex) {
		int index = vertexList.size();
		vertexList.push_back(vertex(vert1, textureIndex,get<0>(uv1), get<1>(uv1)));
		indexList.push_back(index);
		vertexList.push_back(vertex(vert2, textureIndex, get<0>(uv2), get<1>(uv2)));
		indexList.push_back(index + 1);
		vertexList.push_back(vertex(vert3, textureIndex, get<0>(uv3), get<1>(uv3)));
		indexList.push_back(index + 2);
	}


	void transformVerticesByMatrix(Matrix& mult) {
		transformedVertexList.clear();
		transformedIndexList.clear();
		for (int i = 0; i < (int)vertexList.size(); i++)
		{
			transformedVertexList.push_back(vertex(mult * vertexList[i].position,vertexList[i]));
			transformedIndexList.push_back(i);
		}
	};

	void clipTriangles() {
		clippedIndices.clear();
		clippedVertexList.clear();
		int index;
		vertex A;
		vertex B;
		vertex C;
		vertex BPrime;
		vertex CPrime;
		int outOfClippingPlane = 0;
		for (int planeIndex = 0; planeIndex < 6; planeIndex++)
		{
			/*if (planeIndex == 2) {
				continue;
			}*/
			for (int i = 0; i < (int)transformedIndexList.size(); i+=3) {

				index = (int)clippedVertexList.size();
				A = transformedVertexList[transformedIndexList[i]];
			    B = transformedVertexList[transformedIndexList[i+1]];
				C = transformedVertexList[transformedIndexList[i+2]];
				bool Ain = testVertexAgainstPlane(A, clippingPlanes[planeIndex], clippingPlanePoints[planeIndex]);
				bool Bin = testVertexAgainstPlane(B, clippingPlanes[planeIndex], clippingPlanePoints[planeIndex]);
				bool Cin = testVertexAgainstPlane(C, clippingPlanes[planeIndex], clippingPlanePoints[planeIndex]);
				//outOfClippingPlane = 3 - (Ain + Bin + Cin);
				
				outOfClippingPlane = 0;
				outOfClippingPlane += Ain ? 0 : 1;
				outOfClippingPlane += Bin ? 0 : 1;
				outOfClippingPlane += Cin ? 0 : 1;
				switch (outOfClippingPlane)
				{
				case 3:
					//out of frame - discard
					break;
				case 2: //2 are out of the clipping plane
					if (!Ain) { //A is out swap with either B or C, whichever is in
						if (Bin) {
							A.Swap(B);
							swapBools(Ain, Bin);
						}
						else {
							A.Swap(C);
							swapBools(Ain, Cin);
						}
					}
					//Add AB'C' to the list
					//A
					clippedVertexList.push_back(A);
					clippedIndices.push_back(index);
					//add the AB' line
					//B'
					BPrime = LineSegmentAgainstPlane(A, B, clippingPlanes[planeIndex],clippingPlanePoints[planeIndex]);
					clippedVertexList.push_back(BPrime);
					clippedIndices.push_back(index + 1);
					//Add the AC line
					//C'
					CPrime = LineSegmentAgainstPlane(A, C, clippingPlanes[planeIndex], clippingPlanePoints[planeIndex]);
					clippedVertexList.push_back(CPrime);
					clippedIndices.push_back(index + 2);	
					break;
				case 1:
					
					//one vertex is out of frame, we want it to be B
					if (!Ain) {
						A.Swap(B);
						swapBools(Ain, Bin);
					}
					if (!Cin) {
						C.Swap(B);
						swapBools(Cin, Bin);
					}
					BPrime = LineSegmentAgainstPlane(A, B, clippingPlanes[planeIndex], clippingPlanePoints[planeIndex]);
					CPrime = LineSegmentAgainstPlane(C, B, clippingPlanes[planeIndex], clippingPlanePoints[planeIndex]);
					//Add AC'C
					//A
					clippedVertexList.push_back(A);
					clippedIndices.push_back(index );
					//C'
					
					clippedVertexList.push_back(CPrime);
					clippedIndices.push_back(index + 1);
					//C
					clippedVertexList.push_back(C);
					clippedIndices.push_back(index + 2);

					// AB and CB intersect
					//Add 2 triangles to the list AB'C' and AC'C
					//A
					clippedVertexList.push_back(A);
					clippedIndices.push_back(index + 3);
					//add the AB' line
					//B'
					
					clippedVertexList.push_back(BPrime);
					clippedIndices.push_back(index + 4);
					//Add the CB' line
					//C'
					
					clippedVertexList.push_back(CPrime);
					clippedIndices.push_back(index + 5);

					break;
				case 0:
					//all are good
					clippedVertexList.push_back(A);
					clippedIndices.push_back(index);
					clippedVertexList.push_back(B);
					clippedIndices.push_back(index + 1);
					clippedVertexList.push_back(C);
					clippedIndices.push_back(index + 2);
					break;
				default:
					break;
				}
			}

			std::swap(transformedVertexList, clippedVertexList);
			std::swap(transformedIndexList, clippedIndices);
			clippedIndices.clear();
			clippedVertexList.clear();
			

		}
		clippedIndices = transformedIndexList;
		clippedVertexList = transformedVertexList;
		//if (clippedVertexList.size() == 9) {
		//	/*clippedIndices.resize(3);
		//	clippedIndices[0] = 3;
		//	clippedIndices[1] = 4;
		//	clippedIndices[2] = 5;*/
		//	clippedVertexList[4].position.y += .01f;
		//}
		//

	}
	void switchToClippedTris() {
		clippedIndices = transformedIndexList;
		clippedVertexList = transformedVertexList;
	}
	bool testVertexAgainstPlane(vertex& vert1, Avector& planeNormal,Avector& planePoint) {
		return ((vert1.position).dot(planeNormal) <= vert1.position.w);
	}

	vertex LineSegmentAgainstPlane(vertex& vert1, vertex& vert2, Avector& planeNormal,Avector& PP) {
		float dot1 = vert1.position.dot(planeNormal);
		float dot2 = vert2.position.dot(planeNormal);
		float diff = dot1 - vert1.position.w;
		float t = diff / (diff -(dot2 - vert2.position.w));
		return vertex(vert1, vert2, t);
		/*dot1 = d_dot_product_vec3_int(clip_tri.vertices[A].pos, ndc_planes[p]);
		dot2 = d_dot_product_vec3_int(clip_tri.vertices[B].pos, ndc_planes[p]);
		diff = dot1 - clip_tri.vertices[A].pos[3];
		t = diff / (diff - (dot2 - clip_tri.vertices[B].pos[3]));
		interpolate_vertex(t, clip_tri.vertices[A], clip_tri.vertices[B], clip_tri.vertices[B], false);*/
		//change ABC to AB'C
	}
	/*vertex LineSegmentAgainstPlane(vertex& ptA, vertex& ptB, Avector& Pn, Avector& Pp)
	{
		Avector ABray = ptB.position - ptA.position;
		Avector diff = ptA.position - Pp;
		float    mag1 = diff.dot(Pn);
		float    mag2 = ABray.dot(Pn);
		float    magRatio = mag1 / mag2;
		return   vertex(ptA,ptB,magRatio);
	}*/

	void swapBools(bool& b1, bool& b2) {
		bool temp = b2;
		b2 = b1;
		b1 = temp;
	}
	void calcProjectionScale() {
		for (int i = 0; i < (int)clippedIndices.size(); i++)
		{
			clippedVertexList[i].projScale = (1.0f / clippedVertexList[i].position.w);
		}
	}
	
	void zeroOut() {
		for (int i = 0; i < (int)transformedVertexList.size(); i++)
		{
			transformedVertexList[i].position = Avector(0, 0, 0);
		}
	}
	void multiplyTransformedVerticesByMatrix(Matrix& mult) {
		
		for (int i = 0; i < (int)transformedVertexList.size(); i++)
		{
			transformedVertexList[i].position = mult * transformedVertexList[i].position;
		}
	}
	void multiplyClippedTransformedVerticesByMatrix(Matrix& mult) {

		for (int i = 0; i < (int)clippedVertexList.size(); i++)
		{
			clippedVertexList[i].position = mult * clippedVertexList[i].position;

		}
	}
	void homogenizeVertices() {
		for (int i = 0; i < (int)transformedVertexList.size(); i++)
		{	
			transformedVertexList[i].position.homogenize();
		}
	}
	void homogenizeClippedVertices() {
		for (int i = 0; i < (int)clippedVertexList.size(); i++)
		{
			clippedVertexList[i].projScale = 1.0 / clippedVertexList[i].position.w;
			clippedVertexList[i].position.homogenize();
		}
	}
	void swapYZ() {
		for (int i = 0; i < (int)vertexList.size(); i++)
		{
			transformedVertexList[i].position.y = transformedVertexList[i].position.z;
		}
	}
	void clearTransformedVertices() {
		transformedVertexList.clear();
		transformedIndexList.clear();
	}
	void clipAllTriangles(Camera* pCam) {
		clippedIndices.resize(indexList.size());
		memset(clippedIndices.data(), 0, clippedIndices.size() * sizeof(int));
		Avector camPoint(pCam->pos);
		Avector camLook(pCam->forward);
		for (int i = 0; i < (int)indexList.size() - 2; i += 3) {
			Avector a(transformedVertexList[indexList[i]].position);
			Avector b(transformedVertexList[indexList[i+1]].position);
			Avector c(transformedVertexList[indexList[i+2]].position);
			if (!clip(a) && !clip(b) && !clip(c)) {
				clippedIndices[i] = 1;
				clippedIndices[i+1] = 1;
				clippedIndices[i+2] = 1;
			}
		}

	}
	bool clip(Avector& vertex) {
		return (vertex.x >= -vertex.w && vertex.x < vertex.w
			&& vertex.y >= -vertex.w && vertex.y < vertex.w
			&& vertex.z >= -vertex.w && vertex.z < vertex.w);
	}

};


#endif