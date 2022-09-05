#include <stdio.h>
#include <windows.h>
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iterator>
#include <algorithm>
#include <atlimage.h>
#include <timeapi.h>
#include <winternl.h>
#include "Renderer.h"
#include "Camera.h"
#include "triangles.h"
/* include gl and glut files */
#include "gl.h"
#include "glu.h"
#include "glut.h"
#include "globals.h"
#include <cmath>



using namespace std;
static Camera _softwareCam;
static triangles _triangles;
static Renderer _renderer;
 enum RENDERTYPE
{
	OpenGL = 0,
	Software = 1
};
static RENDERTYPE renderType = RENDERTYPE::Software;
static bool switched = false;

float mouseX = 0;
float mouseY = 0;
float deltaX = 0;
float deltaY = 0;
int intFrames = 150*30;
//Math stuff
float clampf(float min, float max, float backGroundColor) {
	if (backGroundColor <= min) {
		return min;
	}
	if (backGroundColor >= max) {
		return max;
	}
	return backGroundColor;
}
void normalize(float* toNorm,int length) {
	float mag = 0;
	for (int i = 0; i < length; i++)
	{
		mag += (toNorm[i] * toNorm[i]);
	}
	if (mag != 0) {
		mag = sqrt(mag);
	}
	
	for (int i = 0; i < length; i++)
	{
		toNorm[i] /= mag;
	}
}
float mag(float* toMag, int length) {
	float mag = 0;
	for (int i = 0; i < length; i++)
	{
		mag += (toMag[i] * toMag[i]);
	}
	if (mag != 0) {
		mag = sqrt(mag);
	}
	return mag;
}
void LoadJPGToRGB(string filename, vector<RGBTRIPLE>& rgbPixels, int& w, int& h)
{
	rgbPixels.clear();

	CImage image;
	HRESULT result = image.Load(filename.c_str());
	if ((result == E_FAIL))
	{
		return;
	}
	int width = image.GetWidth();
	int height = image.GetHeight();
	w = width;
	h = height;
	int bpp = image.GetBPP() >> 3;

	IStream* pStream = NULL;
	ULARGE_INTEGER liSize;
	vector<uint8_t> bytes;
	if (CreateStreamOnHGlobal(NULL, TRUE, &pStream) == S_OK)
	{
		HRESULT hr = image.Save(pStream, Gdiplus::ImageFormatBMP);

		IStream_Size(pStream, &liSize);
		DWORD dwLength = liSize.LowPart;
		IStream_Reset(pStream);
		bytes.resize(dwLength);
		IStream_Read(pStream, bytes.data(), dwLength);
		pStream->Release();
	}
	image.Destroy();

	rgbPixels.resize(width * height);
	int destIndex = 0;
	int srcOffset = sizeof(BITMAPINFOHEADER) + sizeof(BITMAPFILEHEADER);
	for (int i = 0; i < width * height * bpp; i += bpp)
	{
		rgbPixels[destIndex].rgbtBlue = bytes[srcOffset + i];
		rgbPixels[destIndex].rgbtGreen = bytes[srcOffset + i + 1];
		rgbPixels[destIndex].rgbtRed = bytes[srcOffset + i + 2];
		destIndex++;
	}
	byte temp;
	for (int i = 0; i < (int)rgbPixels.size(); i++)
	{
		temp = rgbPixels[i].rgbtBlue;
		rgbPixels[i].rgbtBlue = rgbPixels[i].rgbtRed;
		rgbPixels[i].rgbtRed = temp;
	}
}
static string I2S(int num)
{
	string str;

	std::ostringstream sx;
	sx << num;
	return sx.str();
}

//end of math stuff
struct wall {
	bool vertical = true;
	float normal[2];
	float p1[2];
	float p2[2];
	float tangent[2];
	float wallLength;

	wall(float x1, float y1, float x2, float y2, bool vert) {
		p1[0] = x1;
		p1[1] = y1;
		p2[0] = x2;
		p2[1] = y2;
		vertical = vert;
		tangent[0] = p2[0] - p1[0];
		tangent[1] = p2[1] - p1[1];
		wallLength = tangent[0] + tangent[1];
		normalize(tangent, 2);
		if (vertical) {
			normal[0] = (p1[0] + p2[0]) * .5 + 1;
			normal[1] = (p1[1] + p2[1]) * .5;
		}
		if (vertical) {
			normal[0] = (p1[0] + p2[0]) * .5;
			normal[1] = (p1[1] + p2[1]) * .5 + 1;
		}
	}
	bool GetIntersection(float p0_x, float p0_y, float p1_x, float p1_y, float p2_x, float p2_y, float p3_x, float p3_y, float* i_x, float* i_y)
	{
		float s1_x, s1_y, s2_x, s2_y;
		s1_x = p1_x - p0_x;     s1_y = p1_y - p0_y;
		s2_x = p3_x - p2_x;     s2_y = p3_y - p2_y;

		float s, t;
		s = (-s1_y * (p0_x - p2_x) + s1_x * (p0_y - p2_y)) / (-s2_x * s1_y + s1_x * s2_y);
		t = (s2_x * (p0_y - p2_y) - s2_y * (p0_x - p2_x)) / (-s2_x * s1_y + s1_x * s2_y);

		if (s >= 0 && s <= 1 && t >= 0 && t <= 1)
		{
			// Collision detected
			if (i_x != NULL)
			{
				*i_x = p0_x + (t * s1_x);
			}
			if (i_y != NULL)
			{
				*i_y = p0_y + (t * s1_y);
			}
			return true;
		}

		return false; // No collision
	}
	float* lineIntersectSeg(float* p, float* q, float* A, float* B)
	{
		float px = p[0];
		float py = p[1];
		float qx = q[0];
		float qy = q[1];
		//p and q are the line segemnt, A and B are the actual Line
		float a = p2[1] - p1[1];
		float b = p1[0] - p2[0];
		float c = p2[0] * p1[1] - p1[0] * p2[1];
		float u = fabs(a * p[0] + b * p[1] + c);
		float v = fabs(a * q[0] + b * q[1] + c);
		float intersect[2] = {(p[0] * v + q[0] * u) / (u + v), (p[1] * v + q[1] * u) / (u + v)};
		return intersect;
	}
	bool checkLine(float* forwardPoint, float* camCenter, float radius = 0.15f) {
		float sectX;
		float sectY;
		bool intersects = GetIntersection(camCenter[0], camCenter[1], forwardPoint[0], forwardPoint[1], p1[0], p1[1], p2[0], p2[1], &sectX, &sectY);
		//float* sect = lineIntersectSeg(camCenter, forwardPoint, p1, p2);
		float sectToCam[2] = { sectX - camCenter[0],sectY - camCenter[1] };
		float sectMag = mag(sectToCam, 2);
		float camToForward[2] = { forwardPoint[0] - camCenter[0],forwardPoint[1] - camCenter[1] };
		float camToForwardMag = mag(camToForward, 2);
		if (camToForwardMag < sectMag) {
			return false;
		}
		return true;
		//Collision logic
		/*float ray[2] = {forwardPoint[0] - camCenter[0],forwardPoint[1] - camCenter[1]};
		float dotP = dot(ray, tangent);

		if (dotP < 0) {
			//dont intersect
			return false;
		}
		if (dotP > wallLength) {
			return false;
		}
		//forward is desired point
		float colPoint[2] = { tangent[0] * dotP,tangent[1] * dotP };
		colPoint[0] -= ray[0];
		colPoint[1] -= ray[1];
		float colDist = mag(colPoint, 2);
			//other side of plane - need to check if its between the bounds
		return (colDist <= radius);
		*/

	}
	
	float dot(float* p1, float* p2) {
		return (p1[0] * p2[0]) + (p1[1] * p2[1]);
	}
	bool between(float backGroundColor, float min, float max) {
		if (min > max) {
			float temp = max;
			max = min;
			min = temp;
		}
		return (backGroundColor >= min && backGroundColor <= max);
	}



};
struct maze
{
	int texCount = 1;
	int dimX = 5;
	int dimY = 5;
	int cellSize = 10;
	int height = 10;
	vector<string> textureNames;
	vector<GLuint> textures;
	vector<vector<int>> floorPlan;
	vector<vector<string>> textFile;
	vector<string> content;
	vector<float> vertexList;
	vector<float> textCoordsList;
	vector<wall> walls;
	bool isNumber(const string& str)
	{
		for (char const& c : str) {
			if (std::isdigit(c) == 0) return false;
		}
		return true;
	}
	void parse(string fileName) {
		fstream fout;
		fout.open(fileName);
		string line;
		while (fout) {
			getline(fout, line);
			int index = 0;
			line = line.substr(0, line.find('#'));
			stringstream sstream(line);
			string word;
			while (getline(sstream, word, ' ')) {
				//word.erase(std::remove_if(word.begin(), word.end()), word.end());
				content.push_back(word);
			}
		}
		for (int i = 0; i < (int)content.size(); i++)
		{
			if (content[i] == "" || content[i] == " ") {
				content.erase(content.begin() + i);
				i--;
			}
		}
		//content now has all the words the are 'active'
		for (int i = 0; i < (int)content.size(); i++)
		{
			if (content[i] == "TEXTURES") {
				texCount = stoi(content[i + 1]);
				
				//loop through the texture list
				for (int j = i + 2; j < i + 2 * texCount + 2; j++) {
					if (isNumber(content[j])) { // add the next string to the textures list at this index
						textureNames.push_back( content[j + 1]);
					}
				}
				

			}
			if (content[i] == "CELL") {
				cellSize = stoi(content[i + 1]);
			}
			if (content[i] == "HEIGHT") {
				height = stoi(content[i + 1]);
			}
			if (content[i] == "DIMENSIONS") {
				dimX = stoi(content[i + 1]);
				dimY = stoi(content[i + 1]);
				floorPlan.resize(2 * dimY);
			}
			if (content[i] == "FLOORPLAN") {
				//double vertical 
				for (int j = 0; j < dimY * 2; j++) {
					//for every row
					floorPlan[j].resize(dimX);
					for (int k = 0; k < dimX; k++) {
						floorPlan[j][k] = stoi(content[i + 1 + j * dimY + k]);
					}
				}

			}

		}


	}
	void getCenter(int& x, int& y) {
		x = dimX / 2 * cellSize;
		y = dimY / 2 * cellSize;
	}
	void setUpColliders(int xCoord, int yCoord, bool vertical) {
		if (vertical) {
			walls.push_back(wall((float)xCoord * cellSize, (float)yCoord * cellSize, (float)xCoord * cellSize, (float)(yCoord + 1) * cellSize, true));
		}
		else {
			walls.push_back(wall((float)xCoord * cellSize, (float)yCoord * cellSize, (float)(1 + xCoord) * cellSize, (float)(yCoord)*cellSize, false));
		}
	}
	void drawQuad(int xCoord, int yCoord, bool vertical,int textIndex) {
		glEnable(GL_TEXTURE_2D);
		//textIndex = min((int)textures.size() - 1, textIndex);
		glBindTexture(GL_TEXTURE_2D, textures[textIndex]);
		glBegin(GL_TRIANGLES);

		if (vertical) {
			//triangle 1 - bottom
			//TODO
			glTexCoord2f(0.0f, 0.0f);
			glVertex3f(xCoord * cellSize , yCoord * cellSize, 0);
			glTexCoord2f(0.0f, 1.0f);
			glVertex3f(xCoord * cellSize , (yCoord)*cellSize, height);
			glTexCoord2f(1.0f, 0.0f);
			glVertex3f(xCoord * cellSize, (yCoord + 1) * cellSize, 0);
			//triangle 2 top
			glTexCoord2f(0.0f, 1.0f);
			glVertex3f(xCoord * cellSize , yCoord * cellSize, height);
			glTexCoord2f(1.0f, 0.0f);
			glVertex3f(xCoord * cellSize, (yCoord + 1) * cellSize, 0);
			glTexCoord2f(1.0f, 1.0f);
			glVertex3f(xCoord * cellSize , (yCoord + 1) * cellSize, height);			
		}
		else {
			glTexCoord2f(0.0f, 0.0f);
			glVertex3f(xCoord * cellSize, yCoord * cellSize, 0);
			glTexCoord2f(0.0f, 1.0f);
			glVertex3f(xCoord * cellSize, (yCoord)*cellSize, height);
			glTexCoord2f(1.0f, 0.0f);
			glVertex3f((xCoord+1) * cellSize, yCoord  * cellSize, 0);
			//triangle 2 top
			glTexCoord2f(0.0f, 1.0f);
			glVertex3f(xCoord * cellSize, yCoord * cellSize, height);
			glTexCoord2f(1.0f, 0.0f);
			glVertex3f((xCoord+1) * cellSize, yCoord * cellSize, 0);
			glTexCoord2f(1.0f, 1.0f);
			glVertex3f((xCoord+1) * cellSize , yCoord * cellSize, height);		
		}
		
		glEnd();
		glDisable(GL_TEXTURE_2D);

	}
	void drawFloor(int xCoord, int yCoord) {
		glEnable(GL_TEXTURE_2D);
		//textIndex = min((int)textures.size() - 1, textIndex);
		glBindTexture(GL_TEXTURE_2D, textures[textures.size()-1]);
		glBegin(GL_TRIANGLES);
		glTexCoord2f(0.0f, 0.0f);
		glVertex3f(xCoord * cellSize, yCoord * cellSize, 0);
		glTexCoord2f(1.0f, 0.0f);
		glVertex3f((xCoord+1) * cellSize, (yCoord)*cellSize, 0);
		glTexCoord2f(0.0f, 1.0f);
		glVertex3f(xCoord * cellSize, (yCoord + 1) * cellSize, 0);
		//triangle 2 top
		glTexCoord2f(1.0f, 0.0f);
		glVertex3f((xCoord + 1) * cellSize, (yCoord)*cellSize, 0);
		glTexCoord2f(0.0f, 1.0f);
		glVertex3f(xCoord * cellSize, (yCoord + 1) * cellSize, 0);
		glTexCoord2f(1.0f, 1.0f);
		glVertex3f((xCoord+1) * cellSize, (yCoord + 1) * cellSize, 0);
		
		glEnd();
		glDisable(GL_TEXTURE_2D);

	}
	void drawCeiling(int xCoord, int yCoord) {
		glEnable(GL_TEXTURE_2D);
		//textIndex = min((int)textures.size() - 1, textIndex);
		glBindTexture(GL_TEXTURE_2D, textures[textures.size() - 1]);
		glBegin(GL_TRIANGLES);
		glTexCoord2f(0.0f, 0.0f);
		glVertex3f(xCoord * cellSize, yCoord * cellSize, height);
		glTexCoord2f(1.0f, 0.0f);
		glVertex3f((xCoord + 1) * cellSize, (yCoord)*cellSize, height);
		glTexCoord2f(0.0f, 1.0f);
		glVertex3f(xCoord * cellSize, (yCoord + 1) * cellSize, height);
		//triangle 2 top
		glTexCoord2f(1.0f, 0.0f);
		glVertex3f((xCoord + 1) * cellSize, (yCoord)*cellSize, height);
		glTexCoord2f(0.0f, 1.0f);
		glVertex3f(xCoord * cellSize, (yCoord + 1) * cellSize, height);
		glTexCoord2f(1.0f, 1.0f);
		glVertex3f((xCoord + 1) * cellSize, (yCoord + 1) * cellSize, height);
		glEnd();
		glDisable(GL_TEXTURE_2D);

	}
	void bufferQuad(int xCoord, int yCoord, bool vertical, int textIndex) {	
		if (vertical) {
			//bottom triangle
			//TODO
			_triangles.addTriangle(
				Avector(xCoord * cellSize, yCoord * cellSize, 0),
				Avector(xCoord * cellSize, (yCoord)*cellSize, height),
				Avector(xCoord * cellSize, (yCoord + 1) * cellSize, 0),
				make_tuple(0.0f,0.0f), make_tuple(0.0f, 1.0f), make_tuple(1.0f, 0.0f),textIndex);
			//top triangle
			_triangles.addTriangle(
				Avector(xCoord * cellSize, yCoord * cellSize, height),
				Avector(xCoord * cellSize, (yCoord + 1) * cellSize, 0),
				Avector(xCoord * cellSize, (yCoord + 1) * cellSize, height),
				make_tuple(0.0f, 1.0f), make_tuple(1.0f, 0.0f), make_tuple(1.0f, 1.0f), textIndex);
		}
		else {
			//bottom
			_triangles.addTriangle(
				Avector(xCoord * cellSize, yCoord * cellSize, 0),
				Avector(xCoord * cellSize, (yCoord)*cellSize, height),
				Avector((xCoord + 1) * cellSize, yCoord * cellSize, 0),
				make_tuple(0.0f, 0.0f), make_tuple(0.0f, 1.0f), make_tuple(1.0f, 0.0f), textIndex);
			//top triangle
			_triangles.addTriangle(
				Avector(xCoord * cellSize, yCoord * cellSize, height),
				Avector((xCoord + 1) * cellSize, yCoord * cellSize, height),
				Avector((xCoord + 1) * cellSize, yCoord * cellSize, 0),
				make_tuple(0.0f, 1.0f), make_tuple(1.0f, 1.0f), make_tuple(1.0f, 0.0f), textIndex);
		}
		
	}
	void bufferCeiling(int xCoord, int yCoord) {

		_triangles.addTriangle(
			Avector(xCoord * cellSize, yCoord * cellSize, height),
			Avector((xCoord + 1) * cellSize, (yCoord)*cellSize, height),
			Avector(xCoord * cellSize, (yCoord + 1) * cellSize, height),
			make_tuple(0.0f, 0.0f), make_tuple(0.0f, 1.0f), make_tuple(1.0f, 0.0f), 0);
		_triangles.addTriangle(
			Avector((xCoord + 1) * cellSize, (yCoord)*cellSize, height),
			Avector(xCoord * cellSize, (yCoord + 1) * cellSize, height),
			Avector((xCoord + 1) * cellSize, (yCoord + 1) * cellSize, height),
			make_tuple(0.0f, 0.0f), make_tuple(0.0f, 1.0f), make_tuple(1.0f, 0.0f), 0);
	}
	void bufferFloor(int xCoord, int yCoord) {

		_triangles.addTriangle(
			Avector(xCoord * cellSize, yCoord * cellSize, 0),
			Avector((xCoord + 1) * cellSize, (yCoord)*cellSize, 0),
			Avector(xCoord * cellSize, (yCoord + 1) * cellSize, 0),
			make_tuple(0.0f, 1.0f), make_tuple(1.0f, 0.0f), make_tuple(1.0f, 1.0f), 0);
		_triangles.addTriangle(
			Avector((xCoord + 1) * cellSize, (yCoord)*cellSize, 0),
			Avector(xCoord * cellSize, (yCoord + 1) * cellSize, 0),
			Avector((xCoord + 1) * cellSize, (yCoord + 1) * cellSize, 0),
			make_tuple(0.0f, 1.0f), make_tuple(1.0f, 0.0f), make_tuple(1.0f, 1.0f), 0);
	}
	void drawTestTriangle() {
		_triangles.addTriangle(
			Avector(0, 2.0, 0),
			Avector(-1.0, 2.0, 1.0),
			Avector(1.0, 2.0, -1.0),
			make_tuple(0.0f, 1.0f), make_tuple(1.0f, 0.0f), make_tuple(1.0f, 1.0f), 0);
		_triangles.addTriangle(
			Avector(0.0, 2.0, -1.0),
			Avector(0.0, 0.0, 0.0),
			Avector(0.0, 1.0, 1.0),
			make_tuple(0.0f, 1.0f), make_tuple(1.0f, 0.0f), make_tuple(1.0f, 1.0f), 0);
	}
	void GLTestTriangle() {
		glBegin(GL_TRIANGLES);
		//glColor3b(0, 255, 0);
		glVertex3f(0, 2.0, 0);
		//glColor3b(255, 0, 0);
		glVertex3f(-1.0, 2.0, 1.0);
		//glColor3b(0, 0, 255);
		glVertex3f(1.0, 2.0, -1.0);
		//triangle 2 top
	
		glVertex3f(0.0, 2.0, -1.0);
	
		glVertex3f(0.0, 0.0, 0.0);
	
		glVertex3f(0.0, 1.0, 1.0);
		glEnd();
	}

	void drawMaze() {		
		for (int i = 0; i <= (int)floorPlan.size() - 1; i++)
		{
			for (int j = 0; j < dimX; j++)
			{
				if (floorPlan[i][j] > 0)
				{
					drawQuad(j - (dimX) / 2, (i / 2) - (dimY) / 2, (i % 2) == 1, floorPlan[i][j] - 1);
				}
			}
		}	
	}
	void drawAllMazeTrianglesToBuffer() 
	{
		for (int i = 0; i <= (int)floorPlan.size() - 1; i++)
		{
			for (int j = 0; j < dimX; j++)
			{
				if (floorPlan[i][j] > 0)
				{
					bufferQuad(j - (dimX) / 2, (i / 2) - (dimY) / 2, (i % 2) == 1, floorPlan[i][j] - 1);				
				}
			}
		}
			
		
	}
	void setMaze() {
		for (int i = 0; i <= (int)floorPlan.size() - 1; i++) {
			for (int j = 0; j < dimX; j++) {
				if (floorPlan[i][j] > 0 ) {
					setUpColliders(j - (dimX) / 2, (i / 2) - (dimY) / 2, (i % 2) == 1);
				}
			}
		}
	}

	bool testCollision(float* forward, float* center) {
		for (int i = 0; i < (int)walls.size(); i++)
		{
			if (walls[i].checkLine(forward, center)) {
				return true;
			}
		}
		return false;
	}
	void loadTexture(string fileName) {
		vector<RGBTRIPLE> pixelData;
		int w;
		int h;
		LoadJPGToRGB(fileName, pixelData, w, h);
		RGBTRIPLE t = pixelData[65535];
		if (pixelData.size() == 0) {
			MessageBoxA(NULL, "Texture img cannot be found","load texture error", MB_OK);
			return;
		}

		GLuint texture = 0;
		glGenTextures(1,&texture);
		textures.push_back(texture);
		glBindTexture(GL_TEXTURE_2D, textures[textures.size()-1]);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, pixelData.data());
		gluBuild2DMipmaps(GL_TEXTURE_2D, 3, w, h, GL_RGB, GL_UNSIGNED_BYTE, pixelData.data());
		glBindTexture(GL_TEXTURE_2D, textures[textures.size() - 1]);
	}
	void loadAllTextures() {
		for (int i = 0; i < (int)textureNames.size(); i++) {
			loadTexture("Resources/"+textureNames[i]);
			_renderer.bindTextureToRenderer("Resources/" + textureNames[i]);
		}
	}
	

};

struct camera
{
	float pos[3];
	float lookAt[3];
	float right[3];
	float center[3];
	float up[3];
	float collForward[2];
	float collCenter[2];
	float pitch = 0.0;
	float yaw = 0.0;
	float sensitvity = .01;
	float speed = 0.40;
	float verticalFOV = 54.0;

	camera() {
		pos[0] = 0.0;
		pos[1] = -2.0;
		pos[2] = 0.5;
		lookAt[0] = 0.0;
		lookAt[1] = 1.0;
		lookAt[2] = 0.0;
		right[0] = 1.0;
		right[1] = 0.0;
		right[2] = 0.0;
		up[0] = 0;
		up[1] = 0;
		up[2] = 1.0;
		calcCenter();
		//_softwareCam.updateParams(pos, lookAt, center, up);
		//_softwareCam.setTransform(pos, -yaw, -pitch, 0.0);
	}


	void mouseInput(float x, float y)
	{
		_softwareCam.mouseInput(x, y);
		pitch += y * sensitvity;
		pitch = clampf(-myPI / 2, myPI / 2, pitch);
		yaw += x * sensitvity ;
		calcLookAt();
	    calcRight();
		calcCenter();
		//_softwareCam.updateParams(pos, lookAt, center, up);
		//_softwareCam.setTransform(pos, yaw, pitch, 0.0f);
	}
	void movement(float xMove, float yMove, maze& mz) {
		_softwareCam.moveForward(xMove, yMove);
		xMove *= speed;
		yMove *= speed;

		collForward[0] = pos[0] + xMove * right[0] + yMove * lookAt[0];
		collForward[1] = pos[1] + xMove * right[1] + yMove * lookAt[1];

		collCenter[0] = pos[0];
		collCenter[1] = pos[1];

		//if (!mz.testCollision(collForward, collCenter)) {
			pos[0] += xMove * right[0] + yMove * lookAt[0];
			pos[1] += xMove * right[1] + yMove * lookAt[1];
		//}
		
		//pos[2] += xMove * right[2] + yMove * lookAt[2];
		calcCenter();
		//_softwareCam.updateParams(pos, lookAt, center, up);
		
	}
	void checkPossibleMove(float[3]) {
		
	}
	void moveToCenter(float x, float y) {
		pos[0] = x;
		pos[1] = y;
	}
	void debugCenter() {
		cout << endl;
		cout << pos[0];
		cout << " " << pos[1];
	}
	bool isOutside() {
		if (pos[0] >= 22.0f && pos[1] >= 59.0f) {
			MessageBoxA(NULL, "YOU WIN!!", "WINSCREEN", MB_OK);
			exit(0);
		}
		return false;
	}

	void calcLookAt()
	{
		lookAt[0] = cos(pitch) * sin(yaw); 
		lookAt[1] = cos(pitch) * cos(yaw);
		lookAt[2] = -sin(pitch);

	}
	void calcRight() {
		right[0] = cos(yaw);
		right[1] = -sin(yaw);
		right[2] = 0;	
	}
	
	void calcCenter() 
	{
		center[0] = pos[0] + lookAt[0];
		center[1] = pos[1] + lookAt[1];
		center[2] = pos[2] + lookAt[2];
	}
	void forward() {
		
		//gluLookAt(pos[0], pos[1], pos[2], center[0], center[1], center[2], up[0], up[1], up[2]);
		gluLookAt(_softwareCam.pos.x, _softwareCam.pos.y, _softwareCam.pos.z, 
			_softwareCam.center.x, _softwareCam.center.y, _softwareCam.center.z,
			_softwareCam.up.x, _softwareCam.up.y, _softwareCam.up.z);
		GLfloat mat[16];
		//GLfloat proj[16];
		glGetFloatv(GL_MODELVIEW_MATRIX, mat);
		//glGetFloatv(GL_PROJECTION_MATRIX, proj);
		return;
	}
	void setUpGLUEnv() {
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		forward();
		GLfloat mat[16];
		glGetFloatv(GL_MODELVIEW_MATRIX, mat);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(_softwareCam.vertFov, (float)glutGet(GLUT_WINDOW_WIDTH) / (float)glutGet(GLUT_WINDOW_HEIGHT), 0.01, 1000.0);
		
		GLfloat proj[16];
		
		glGetFloatv(GL_PROJECTION_MATRIX, proj);
	}
/*
void reshape() {
		//glViewport(0, 0, (GLsizei)glutGetWindow, (GLsizei)h);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(verticalFOV, (float)glutGet(GLUT_WINDOW_WIDTH) / (float)glutGet(GLUT_WINDOW_HEIGHT), 0.01, 1000.0);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
	}
*/	
	
	

};

maze myMaze;

struct camera cam = {};


struct Image {
	unsigned long sizeX;
	unsigned long sizeY;
	char* data;
};


//
//void addVector(float* vect1, float* vect2, float* out) {
//	out[0] = vect1[0] + vect2[0];
//	out[1] = vect1[1] + vect2[1];
//	out[2] = vect1[2] + vect2[2];
//
//}

/* this function checks for GL errors that might have cropped up and 
   breaks the program if they have */
void checkGLErrors(char *prefix) {
	GLenum error;
	if ((error = glGetError()) != GL_NO_ERROR) {
		fprintf(stderr,"%s: found gl error: %s\n",prefix, gluErrorString(error));
		//exit(-1);
	}
}


void display(void) {
	
	intFrames--;
	string title = "Maze walker " + I2S(intFrames / 150);
	/*if (intFrames < 0) {
		MessageBoxA(NULL, "YOU LOSE :(((", "LOSE SCREEN", MB_OK);
		exit(0);
	}*/
	glutSetWindowTitle(I2S(intFrames).c_str());
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	checkGLErrors("Errors in display()!\n");
	if (renderType == RENDERTYPE::OpenGL) 
	{		
		cam.setUpGLUEnv();
		myMaze.drawMaze();
		
	}
	else if (renderType == RENDERTYPE::Software) 
	{
		_renderer.Render(_triangles);
		glDrawPixels(window_width, window_height, GL_RGB, GL_UNSIGNED_BYTE, _renderer.buffer.data());
	}
	glutSwapBuffers();

}

void redisplay(void) {
    glutPostRedisplay();
	return;
}
void reshape(void) {
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	cam.forward();
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(cam.verticalFOV, (float)glutGet(GLUT_WINDOW_WIDTH) / (float)glutGet(GLUT_WINDOW_HEIGHT), 0.01, 1000.0);
}

void keyboard(unsigned char key, int x, int y) {
	switch (key) {
		case 'w':
			cam.movement(0, 1.0,myMaze);
			break;
		case 's':
			cam.movement(0, -1.0,myMaze);
			break;
		case 'a':
			cam.movement(-1.0, 0,myMaze);
			break;
		case 'd':
			cam.movement(1.0, 0,myMaze);
			break;
		case 'p':
			cam.debugCenter();
			break;

		case 27:
			break;
		case 'q':
			exit(0);
			break;
		case 'Q':
			/* quit the program */
			exit(0);
			break;
		case 32:
			if (renderType == RENDERTYPE::OpenGL) {
				renderType = RENDERTYPE::Software;
				switched = true;
			}
			else {
				renderType = RENDERTYPE::OpenGL;
			}
			break;
		case 'k':
			//_renderer.increaseZoom(1.0);
			_softwareCam.increaseFOV(0.2);
			break;
		case 'c':
			_renderer.increaseZoom(1.0);
			break;
		case'n':
			_renderer.NN = !_renderer.NN;
			break;
		default:
			break;
	}

	return;
}


/* handle mouse interaction */
void mouseInput(int button, int state, int x, int y) {
    switch(button) {
		case GLUT_LEFT_BUTTON:
			if (state == GLUT_DOWN) {
				mouseX = (float)x;
				mouseY = (float)y;
				/* this is called when the button is first pushed down */
				//fprintf(stderr,"%d\t%d\n", x, y);
				glutPostRedisplay();
			}
			else {
				/* this is called when the button is released */
				//fprintf(stderr,"%d\t%d\n", x, y);
				glutPostRedisplay();
			}
			break;
		case GLUT_MIDDLE_BUTTON:
			break;
		case GLUT_RIGHT_BUTTON:
			//fprintf(stderr, "%d\t%d\n", x, y);
			break;
	}

	return;
}

void mouseMotion(int x, int y) {
	deltaX = (float)x - mouseX;
	deltaY = (float)y - mouseY;
	mouseX = (float)x;
	mouseY = (float)y;
	cam.mouseInput(deltaX, deltaY);
	//cout << deltaX << endl;
	//cam.lookat();
	//fprintf(stderr, "%f\t%d\n", deltaY, deltaX);
	
    glutPostRedisplay();
}
void drawPolygon(void) {
	glBegin(GL_POLYGON);
	glVertex2f(0.0, 0.0);
	glVertex2f(0.0, 3.0);
	glVertex2f(4.0, 2.0);
	glVertex2f(-1.0, 1.0);
	glEnd();
}



int main(int argc, char **argv) {
	
    /* Initialize GLUT */
    glutInit(&argc, argv);
	glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH );
    glutInitWindowSize(window_width, window_height);
	glutCreateWindow("window_name");
    glutInitWindowPosition(100,50);

	glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutReshapeFunc(NULL);
	
	/* set an idle function */
	glutIdleFunc(redisplay);

	/* init the mouse glut callback functions */
    glutMouseFunc(mouseInput);
    glutMotionFunc(mouseMotion);
   /// glutPassiveMotionFunc(mouseMotion);
	if(argc > 1)
		myMaze.parse(argv[2]);
	else {
		myMaze.parse("Resources/maze3.txt");
	}
	myMaze.loadAllTextures();
	
	myMaze.setMaze();
	int x;
	int y;
	myMaze.getCenter(x, y);
	myMaze.drawAllMazeTrianglesToBuffer();
	_renderer.setRenderParams(window_width, window_height, 3, &_softwareCam);
	
	
	
	cam.setUpGLUEnv();
	glEnable(GL_DEPTH_TEST);
	//cam.moveToCenter(float(x) + 1, float(y));
	//glEnable(GL_CULL_FACE);
	//glCullFace(GL_BACK);
    /* Enter main loop */
	
	glutMainLoop();
	

	return 1;
}
