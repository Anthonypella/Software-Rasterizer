#ifndef RENDERER_H
#define RENDERER_H
#include <Windows.h>
#include "Avector.h"
#include "Matrix.h"
#include "triangles.h"
#include "Camera.h"
#include "glut.h"
#include "globals.h"
#include <atlimage.h>

typedef unsigned char u08;


struct pt
{
	int x;
	int y;
};
struct fragment {
	union {
		pt pos;
		struct {
			int x;
			int y;
		};
	};
	
	RGBTRIPLE color;
	int depth;
	float u;
	float v;
	float projScale;
	int textureIndex;
	void SetColor(int r, int g, int b)
	{
		color.rgbtBlue = (uint8_t)b;
		color.rgbtGreen = (uint8_t)g;
		color.rgbtRed = (uint8_t)r;
	}
	fragment() {
		pos.x = 0;
		pos.y = 0;
		depth = 0;
		color.rgbtRed = 255;
		color.rgbtGreen = 255;
		color.rgbtBlue = 255;
		u = .5f;
		v = .5f;
		projScale = 1.0f;
		textureIndex = 0;
		
	}
	fragment(pt& point) {
		pos.x = point.x;
		pos.y = point.y;
	}
	fragment(vertex& vert) {
		pos.x = (int)(vert.position.x+.5f);
		pos.y = (int)(vert.position.y+.5f);
		depth = (int)(vert.position.z*1024.0f);
		color = vert.color;
		u = vert.u;
		v = vert.v;
		projScale = vert.projScale;
		textureIndex = vert.textureIndex;
	}
	fragment(pt& point, int r, int g, int b, int d, float texU, float texV,float pScale,int tIndex) {
		pos.x = point.x;
		pos.y = point.y;
		SetColor(r, g, b);
		depth = d;
		u = texU;
		v = texV;
		projScale = pScale;
		textureIndex = tIndex;

	}
	void Swap(fragment& swap) {
		int temp = swap.pos.x;
		swap.pos.x = pos.x;
		pos.x = temp;

		temp = swap.pos.y;
		swap.pos.y = pos.y;
		pos.y = temp;

		temp = swap.textureIndex;
		swap.textureIndex = textureIndex;
		textureIndex = temp;

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

		ftemp = swap.depth;
		swap.depth = depth;
		depth = ftemp;

		ftemp = swap.projScale;
		swap.projScale = projScale;
		projScale = ftemp;
	}
};
struct texture {
	vector<RGBTRIPLE> rgbPixels;
	int w;
	int h;
	texture(string filename)
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


};

class Renderer {
	
public:
	int viewport_Width = window_width;
	int viewport_Height = window_height;
	int bpp = 3;
	float zoom = 100.0f;
	Camera* pCamera = NULL;
	vector<unsigned char> buffer;
	vector<unsigned int> zBuffer;
	vector<fragment> edgeL;
	vector<fragment> edgeR;
	vector<texture> textures;
	RECT clipRect = { 0,1080,1920,0 };
	
	bool NN = false;
	
	
	int offscreenBufferStride = bpp * viewport_Width;

	void setRenderParams(int w, int h,int bpp, Camera* pCam) {
		viewport_Width = w;
		viewport_Height = h;
		this->bpp = bpp;
		pCamera = pCam;
		buffer.resize(viewport_Width * viewport_Height * bpp);
		zBuffer.resize(viewport_Width * viewport_Height);
		edgeL.reserve(viewport_Height);
		edgeR.reserve(viewport_Height);
	}
	void clearBuffer(int r,int g, int b) {
		memset(buffer.data(), 0, buffer.size() * sizeof(char));	
	}
	void clearZbuffer() {
		memset(zBuffer.data(), 255, zBuffer.size() * sizeof(unsigned int));
	}
	void increaseZoom(float amt) {
		zoom += amt;
	}
	
	u08 r = 100;
	Matrix mViewProj;
	void Render(triangles& tri) {
		//r = (u08)min(r + 1, 255);
		clearBuffer(0,0,0);
		clearZbuffer();
		Matrix mProj = pCamera->getProjectionMatrix();
		Matrix mScreen = pCamera->toScreenSpaceMatrix();
		Matrix mTestTrans = pCamera->getTranslationMatrix();
		Matrix mTestRot = pCamera->getRotationMatrix();

		tri.transformVerticesByMatrix(mProj*mTestRot*mTestTrans*Matrix::identity());
		
		
		tri.clipTriangles();
		//tri.calcProjectionScale();
		//tri.homogenizeVertices();
		tri.homogenizeClippedVertices();
		tri.multiplyClippedTransformedVerticesByMatrix(mScreen);
		
		
		vertex currentVert;
		vertex currentVert1;
		vertex currentVert2;
		int index = 0;
		//draw vertices
		for (int i = 0; i < (int)tri.clippedIndices.size()-2; i+=3)
		{

				currentVert = tri.clippedVertexList[tri.clippedIndices[i]];
				currentVert1 = tri.clippedVertexList[tri.clippedIndices[i+1]];
				currentVert2 = tri.clippedVertexList[tri.clippedIndices[i+2]];
				Rasterize(currentVert, currentVert1, currentVert2, Avector(255, 105, 180));
		}
		
		
	}
	//assume triangle is screen-space clipped
	void Rasterize(vertex& vert1, vertex& vert2, vertex& vert3,Avector color) {
		fragment a(vert1);
		fragment b(vert2);
		fragment c(vert3);
		int temp;
		bool kneeOnLeft = false;

		//if (a.y == b.y) {
		//	if (a.y < 500) {
		//		a.y += 1;
		//		//b.y += 1;
		//	}
		//	else {
		//		a.y -= 1;
		//		//b.y -= 1;
		//	}
		//}
		//if (a.y == c.y) {
		//	if (a.y < 500) {
		//		a.y += 1;
		//		//c.y += 1;
		//	}
		//	else {
		//		a.y -= 1;
		//		//c.y -= 1;
		//	}
		//}
		if (b.y == c.y) {
			if (b.y < 1080) {
				b.y += 1;
				//c.y += 1;
			}
			else {
				b.y -= 1;
				//c.y -= 1;
			}
		}
		


		if (b.y < a.y && b.y < c.y)
		{
			a.Swap(b);
		}
		if (c.y < a.y && c.y < b.y)
		{
			a.Swap(c);
		}
		//now ay is lowestY
		if (b.y > c.y)
		{
			b.Swap(c);
		}

		temp = ((c.x - a.x) * (b.y - a.y) - (c.y - a.y) * (b.x - a.x));
		kneeOnLeft = temp > 0;
	
		edgeL.clear();
		edgeR.clear();
		if (a.y == b.y && a.y == c.y) //degenerate triangle
		{
			return;
		}
		// edgeL has 2 edges
		if ((a.y == c.y || b.y == c.y) && a.y > b.y)
		{
			RasterizeEdge(b, a, edgeL);
			RasterizeEdge(c, a, edgeR);
		}
		else
		{
			RasterizeEdge(a, b, kneeOnLeft ? edgeL : edgeR);
			RasterizeEdge(b, c, kneeOnLeft ? edgeL : edgeR);
			RasterizeEdge(a, c, kneeOnLeft ? edgeR : edgeL);
		}
		//TODO
		DrawTexturedTriangle();
		//drawEdges(255);
		
		
		//left edge and right edge have same number of lines
		//for each y send in draw span with the left x, right x, and ly+i
			

	}
	void RasterizeEdge(fragment& a, fragment& b, vector<fragment>& edge) {
		
		int edgeHeight = b.y - a.y;
		if (edgeHeight == 0) return;
		int xStep = ((b.x - a.x)<<10) / edgeHeight;
		int xStepTotal = a.x<<10;

		int bstep = ((b.color.rgbtBlue - a.color.rgbtBlue) << 10) / edgeHeight;
		int bstepTotal = ((int)a.color.rgbtBlue) << 10;

		int gstep = ((b.color.rgbtGreen - a.color.rgbtGreen) << 10) / edgeHeight;
		int gstepTotal = ((int)a.color.rgbtGreen) << 10;

		int rstep = ((b.color.rgbtRed - a.color.rgbtRed) << 10) / edgeHeight;
		int rstepTotal = ((int)a.color.rgbtRed) << 10;

		int dstep = ((b.depth - a.depth)<<10) / edgeHeight;
		int dstepTotal = a.depth<<10;

		float ustep = (b.u * b.projScale - a.u * a.projScale) / (float)edgeHeight;
		float utotal = a.u * a.projScale;
		float vstep = (b.v * b.projScale - a.v * a.projScale) / (float)edgeHeight;
		float vtotal = a.v * a.projScale;

		float projStep = (b.projScale - a.projScale) / (float)edgeHeight;
		float ptotal = a.projScale;

		for (int i = a.y; i < a.y + edgeHeight; i++)
		{		
			pt p = { a.x + xStepTotal>>10,i };
			edge.push_back(fragment(p, rstepTotal >> 10, gstepTotal >> 10, bstepTotal >> 10,dstepTotal, utotal, vtotal,ptotal,a.textureIndex));
			xStepTotal += xStep;
			bstepTotal += bstep;
			rstepTotal += rstep;
			gstepTotal += gstep;
			dstepTotal += dstep;
			utotal += ustep;
			vtotal += vstep;
			ptotal += projStep;
			
		}
		

	}
	void drawSpan(int y, int xl, int xr,char color) {
		/*if (xl < xr) {
			memset(&(buffer[(xl + y * window_width) * 3]), color, xr - xl);
		}*/
		int index = 0;
		for (int i = xl; i < xr; i++) {
			drawToBuffer(i, y, color, color, color);
		}
		
	}
	void drawTriangle(char fillColor) {
		if ((int)min(edgeL.size(), edgeR.size()) < 2) return;

		for (int i = 0; i < (int)min(edgeL.size(), edgeR.size()); i++){
			
			drawSpan(edgeL[i].y, edgeL[i].x, edgeR[i].x, fillColor);
		}
	}

	void DrawColoredTriangle() {
		if ((int)min(edgeL.size(), edgeR.size()) < 2) return;
		if ((int)min(edgeL.size(), edgeR.size()) < 2) return;

		for (int i = 0; i < (int)min(edgeL.size(), edgeR.size()); i++) {

			//drawSpan(edgeL[i].y, edgeL[i].x, edgeR[i].x, 255);
		}
		int index;
		int spanLen;
		int trimL, trimR;
		int rstep, gstep, bstep;
		int rStepTotal, gStepTotal, bStepTotal;
		int yoffset = edgeL[0].y * window_width;
		for (int spanOn = 0; spanOn < (int)edgeL.size(); spanOn+=1)
		{
			index = (edgeL[spanOn].x + yoffset) * 3;
			spanLen = (edgeR[spanOn].x - edgeL[spanOn].x)*3;
			if (spanLen && edgeL[spanOn].y >= clipRect.top && edgeL[spanOn].y < clipRect.bottom)
			{
				trimL = (edgeL[spanOn].x < clipRect.left ? clipRect.left - edgeL[spanOn].x : 0);
				trimR = edgeR[spanOn].x >= clipRect.right ? clipRect.right - edgeR[spanOn].x : 0;

				rstep = (((int)edgeR[spanOn].color.rgbtRed - (int)edgeL[spanOn].color.rgbtRed) << 10) / spanLen;
				rStepTotal = ((int)edgeL[spanOn].color.rgbtRed) << 10;

				gstep = (((int)edgeR[spanOn].color.rgbtGreen - (int)edgeL[spanOn].color.rgbtGreen) << 10) / spanLen;
				gStepTotal = ((int)edgeL[spanOn].color.rgbtGreen) << 10;

				bstep = (((int)edgeR[spanOn].color.rgbtBlue - (int)edgeL[spanOn].color.rgbtBlue) << 10) / spanLen;
				bStepTotal = ((int)edgeL[spanOn].color.rgbtBlue) << 10;

				//drawSpan(edgeL[spanOn].y, edgeL[spanOn].x, edgeR[spanOn].x, 255);
				for (int p = index; p < index + spanLen; p+=3)
				{
					if (p >= index + trimL && p < index + spanLen + trimR)
					{
						buffer[p] = (char)(rStepTotal >> 10);
						buffer[p + 1] = (char)(gStepTotal >> 10);
						buffer[p + 2] = (char)(bStepTotal >> 10);
					}
					rStepTotal += rstep;
					gStepTotal += gstep;
					bStepTotal += bstep;

				}
			}
			yoffset += window_width;


		}





	}
	void DrawTexturedTriangle() {
		if ((int)min(edgeL.size(), edgeR.size()) < 2) return;
		if ((int)min(edgeL.size(), edgeR.size()) < 2) return;

		//for (int i = 0; i < (int)min(edgeL.size(), edgeR.size()); i++) {

		//	//drawSpan(edgeL[i].y, edgeL[i].x, edgeR[i].x, 255);
		//}
		texture* currentTex = &textures[edgeL[0].textureIndex];
		int index;
		int spanLen, texSpanLen;
		int trimL, trimR;
		int dstep, dstepTotal;
		float pstep, pstepTotal;
		float ustep, usteptotal, vstep, vsteptotal;
		int yoffset = edgeL[0].y * window_width;
		float fu, fv;
		int iindex;
		int zVal = 0;
		RGBTRIPLE textureLookup;
		for (int spanOn = 0; spanOn < (int)edgeL.size(); spanOn += 1)
		{
			index = (edgeL[spanOn].x + yoffset)*3;
			spanLen = (edgeR[spanOn].x - edgeL[spanOn].x)*3;
			texSpanLen = (edgeR[spanOn].x - edgeL[spanOn].x);
			if (texSpanLen == 0) {
				texSpanLen++;
			}

			
				trimL = (edgeL[spanOn].x < clipRect.left ? clipRect.left - edgeL[spanOn].x : 0);
				trimR = edgeR[spanOn].x >= clipRect.right ? clipRect.right - edgeR[spanOn].x : 0;

				pstep = ((edgeR[spanOn].projScale - edgeL[spanOn].projScale) / (float)texSpanLen);
				pstepTotal = (edgeL[spanOn].projScale);

				/*pstep = 1.0f;
				pstepTotal = 1.0f;*/

				//ustep = ((edgeR[spanOn].u *edgeR[spanOn].projScale) - (edgeL[spanOn].u * edgeL[spanOn].projScale)) / (float)texSpanLen;
				//usteptotal = (edgeL[spanOn].u * pstepTotal); // u / w

				//vstep = ((edgeR[spanOn].v *edgeR[spanOn].projScale) - (edgeL[spanOn].v * edgeL[spanOn].projScale)) / (float)texSpanLen;
				//vsteptotal =(edgeL[spanOn].v * pstepTotal); // v/w

				ustep = ((edgeR[spanOn].u) - (edgeL[spanOn].u )) / texSpanLen;
				usteptotal = (edgeL[spanOn].u); // u / w

				vstep = ((edgeR[spanOn].v ) - (edgeL[spanOn].v )) / texSpanLen;
				vsteptotal = (edgeL[spanOn].v ); // v/w


			    dstep = (edgeR[spanOn].depth - edgeL[spanOn].depth) / (float)texSpanLen;
				dstepTotal = edgeL[spanOn].depth;

				int zIndex = index / 3;
				//drawSpan(edgeL[spanOn].y, edgeL[spanOn].x, edgeR[spanOn].x, 255);
				for (int p = index; p < index + spanLen +1; p += 3)
				{
					
					//zVal++;
					if (p >= index + trimL && p < index + spanLen +1 + trimR)
					{
						if (p < zBuffer.size()*3) {
							if (zBuffer[zIndex] > dstepTotal) {
								zBuffer[zIndex] = dstepTotal;
								fu = usteptotal / pstepTotal;
								fv = vsteptotal / pstepTotal;

								while (fu < 0)
								{
									fu += 1.0f;
								}
								while (fu > 1.0f)
								{
									fu -= 1.0f;
								}
								while (fv < 0)
								{
									fv += 1.0f;
								}
								while (fv > 1.0f)
								{
									fv -= 1.0f;
								}
								fu *= (currentTex->w - 1);
								fv *= (currentTex->h - 1);
								iindex = (int)fu + (int)fv * currentTex->w;

								getFilteredTextureColor(fu, fv, *currentTex, textureLookup);

								//add fog here
								buffer[p] = textureLookup.rgbtBlue;
								buffer[p + 1] = textureLookup.rgbtGreen;
								buffer[p + 2] = textureLookup.rgbtRed;


							}

						}
							//z buffer
							
							usteptotal += ustep;
							vsteptotal += vstep;
							dstepTotal += dstep;
							pstepTotal += pstep;
							zIndex++;
					}	
					
				}
				yoffset += window_width;
			
		}
	}
	void bindTextureToRenderer(string fname) {
		textures.push_back(texture(fname));
	}

	void drawEdges(char val) {
		for (int i = 0; i < (int)edgeL.size(); i++) {
			drawToBuffer(edgeL[i].x, edgeL[i].y, val, val, val);
		}
		for (int i = 0; i < (int)edgeR.size(); i++) {
			drawToBuffer(edgeR[i].x, edgeR[i].y, val, val, val);
		}
	}
	void getFilteredTextureColor(float fu,float fv, texture& tex, RGBTRIPLE& out) {
		RGBTRIPLE c;
		
		RGBTRIPLE d;
		
		RGBTRIPLE r;
		int iu = (int)fu;
		int iv = (int)fv;
		int index = iu + iv * tex.w;
		fu -= (float)iu;
		fv -= (float)iv;
		
		if (iu > 0 && iu < tex.w - 1 && iv > 0 && iv < tex.h -1 && !NN) 
		{
			// i means rgb at index pixel
			//final rgb = (i)*(1-fu) + (i+1)*fu + i*(1-fv)+(i+w)*fv / 2
			c.rgbtBlue = tex.rgbPixels[index].rgbtBlue;
			c.rgbtGreen = tex.rgbPixels[index].rgbtGreen;
			c.rgbtRed = tex.rgbPixels[index].rgbtRed;

			r.rgbtBlue = tex.rgbPixels[index+1].rgbtBlue;
			r.rgbtGreen = tex.rgbPixels[index+1].rgbtGreen;
			r.rgbtRed = tex.rgbPixels[index+1].rgbtRed;

			d.rgbtBlue = tex.rgbPixels[index + tex.w].rgbtBlue;
			d.rgbtGreen = tex.rgbPixels[index + tex.w].rgbtGreen;
			d.rgbtRed = tex.rgbPixels[index + tex.w].rgbtRed;

			out.rgbtBlue = (char)(((float)c.rgbtBlue * (1.0f - fu) + (float)r.rgbtBlue * (fu)+
				(float)c.rgbtBlue * (1.0f - fv) + (float)d.rgbtBlue * (fv)) * .5f);
			out.rgbtGreen = (char)(((float)c.rgbtGreen * (1.0f - fu) + (float)r.rgbtGreen * (fu)+
				(float)c.rgbtGreen * (1.0f - fv) + (float)d.rgbtGreen * (fv)) * .5f);
			out.rgbtRed = (char)(((float)c.rgbtRed * (1.0f - fu) + (float)r.rgbtRed * (fu)+
				(float)c.rgbtRed * (1.0f - fv) + (float)d.rgbtRed * (fv)) * .5f);
			
		}
		else 
		{
			out.rgbtBlue = tex.rgbPixels[index].rgbtBlue;
			out.rgbtGreen = tex.rgbPixels[index].rgbtGreen;
			out.rgbtRed = tex.rgbPixels[index].rgbtRed;
			
		}

		
		

	}


	
	void drawToBuffer(int x, int y, char r, char g, char b) {
		//if (x < 0 || x >= viewport_Width || y < 0 || y >= viewport_Height) return;
		int i = x + y * window_width;
		i *= 3;
		//if (i > (int)buffer.size() - 2 || i < 0) return;
		buffer[i] = r;
		buffer[i + 1] = g;
		buffer[i + 2] = b;
	}
	bool clip(Avector& vertex) {
		return (vertex.x >= -vertex.w && vertex.x < vertex.w
			&& vertex.y >= -vertex.w && vertex.y < vertex.w
			&& vertex.z >= -vertex.w && vertex.z < vertex.w);
	}
	bool inScreenBounds(Avector& vertex) {
		return vertex.x >= 0 && vertex.x < viewport_Width
			&& vertex.y >= 0 && vertex.y < viewport_Height
			&& (vertex.x + vertex.y * window_width)*3 < (int)buffer.size();
	}
	

	









};







#endif // !RENDERER_H

