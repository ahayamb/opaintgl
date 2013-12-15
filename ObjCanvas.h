#include <stdio.h>
#include <math.h>
#include <vector>
#include <stdlib.h>
#include <string.h>
#include <GL\gl.h>
#include <GL\glut.h>
#include <queue>
using namespace std;

#define RGB(r, g, b) ((((r)&0xFF)<<16) + (((g)&0xFF)<<8) + ((b)&0xFF))
#define CEIL4(x) ((((x)+3)/4)*4)

typedef unsigned short WORD;
typedef unsigned long  DWORD;

typedef struct tagBmpHeader
{
	DWORD sizeFile;
	DWORD reserved;
	DWORD offbits;
	DWORD sizeStruct;
	DWORD width, height;
	WORD  planes;
	WORD  bitCount;
	DWORD compression;
	DWORD sizeImage;
	DWORD xPelsPerMeter;
	DWORD yPelsPerMeter;
	DWORD colorUsed;
	DWORD colorImportant;
} BmpHeader;


typedef struct tagBitmap
{
	size_t width;
	size_t height;
	size_t size;
	unsigned char *data;
	bool misopen;
} Bitmap;

typedef struct t
{
	float X, Y;
}dot;

class Pixel
{
public:
	Pixel(float clr[3])
	{
		Clr[0] = clr[0];
		Clr[1] = clr[1];
		Clr[2] = clr[2];
	}
	Pixel()
	{
		Clr[0] = 0;
		Clr[1] = 0;
		Clr[2] = 0;
	}
	void Reset()
	{
		Clr[0] = 1;
		Clr[1] = 1;
		Clr[2] = 1;
	}
	void SetEl(float clr[3])
	{
		Clr[0] = clr[0];
		Clr[1] = clr[1];
		Clr[2] = clr[2];
	}
	float *GetEl() { return Clr; }
	float GetR() { return Clr[0]; }
	float GetG() { return Clr[1]; }
	float GetB() { return Clr[2]; }
	bool operator == (Pixel *t)
	{
		if (Clr[0] == t->Clr[0] && Clr[1] == t->Clr[1] && Clr[2] == t->Clr[2]) return true;
		else return false;
	}
	void toString() { printf("%f %f %f\n", Clr[0], Clr[1], Clr[2]); }

private:
	float Clr[3];
};

class Canvas
{
public:
	Canvas(int W, int H) { init(W, H); }
	Canvas() { init(800, 600); }
	int getWidth() { return Width; }
	int getHeight() { return Height; }
	bool Modified;
	void DrawToScreen()
	{
		glPointSize(1.0);
		float *ret;
		for (int i = 0; i < Height; i++)
			for (int j = 0; j < Width; j++)
			{
				glBegin(GL_POINTS);
				ret = Workspace[i][j].GetEl();
				glColor3f(ret[0], ret[1], ret[2]);
				glVertex2f(i, j);
				glEnd();
			}
		Modified = false;
	}
	float *getPixelAt(int i, int j) { return Workspace[i][j].GetEl(); }
	void setPixelAt(int i, int j, float clr[3])
	{
		float in[3];
		in[0] = clr[0];
		in[1] = clr[1];
		in[2] = clr[2];
		if (i >= 0 && i < Height && j >= 0 && j < Width) Workspace[i][j].SetEl(in);
	}
	void resetAt(int i, int j) { Workspace[i][j].Reset(); }
	void eraseAt(int x, int y, int erSize)
	{
		for (int i = x - erSize / 2; i >= 0 && i < Height && i <= x + erSize / 2; i++)
			for (int j = y - erSize / 2; j >= 0 && j <= y + erSize / 2; j++)
				Workspace[i][j].Reset();
	}
	void fillAt(int x, int y, float clr[3])
	{
		y = Width - y;
		float *t = Workspace[x][y].GetEl();
		Pixel a, b;
		a.SetEl(t);
		b.SetEl(clr);
		if (a == &b) return;
		floodFill(x, y, clr, &a);
	}

	void SaveWork(char *filename)
	{
		Bitmap saved = {0};
		newbmp(800, 600, &saved);
		savebmp(filename, &saved);
	}

private:
	int savebmp(char *fname, Bitmap *mbmp)
	{
		FILE *fp;
		BmpHeader head = {0, 0, 54, 40, 0, 0, 1, 24, 0, 0}; /* BMP file header */

		if ((fp = fopen(fname, "wb")) == NULL)
		{
			printf("Error: can't save to BMP file \"%s\".\n", fname);
			return 1;
		}

		fputc('B', fp);
		fputc('M', fp); /* write type */
		/* fill BMP file header */
		head.width = mbmp->width;
		head.height = mbmp->height;
		head.sizeImage = mbmp->size;
		head.sizeFile = mbmp->size + head.offbits;
		fwrite(&head, sizeof head, 1, fp); /* write header */
		if (fwrite(mbmp->data, 1, mbmp->size, fp) != mbmp->size)
		{
			fclose(fp);
			return 1; /* write bitmap infomation */
		}

		fclose(fp);
		return 0;
	}
	int putpixel(int x, int y, int color, Bitmap *mbmp) //int putpixel(int x, int y, int color, Bitmap *mbmp)
	{
		unsigned char *p;

		if (x < mbmp->width && y < mbmp->height)
		{
			p = mbmp->data + CEIL4(mbmp->width * 3) * y + x * 3;
			p[0] = (unsigned char)(color & 0xff);
			p[1] = (unsigned char)((color >> 8) & 0xff);
			p[2] = (unsigned char)((color >> 16) & 0xff);
			return 0;
		}
		return 1;
	}
	void closebmp(Bitmap *mbmp)
	{
		if (mbmp->misopen) /* clear bitmap */
		{
			mbmp->width = mbmp->height = 0;
			if (mbmp->data != NULL)
			{
				free(mbmp->data);
				mbmp->data = NULL;
			}
		}
	}
	int newbmp(int width, int height, Bitmap *mbmp)
	{
		if (width <= 0 || height <= 0)
		{
			printf("Width and height should be positve.\n");
			return 1;
		}

		if (mbmp->misopen)
			closebmp(mbmp);
		mbmp->width = (size_t)width;
		mbmp->height = (size_t)height;
		mbmp->size = CEIL4(mbmp->width * 3) * mbmp->height;

		if ((mbmp->data = (unsigned char*)malloc(mbmp->size)) == NULL)
		{
			printf("Error: alloc fail!");
			return 1;
		}

		float *ret;
		for (int i = 0; i < this->Height; i++)
			for (int j = 0; j < this->Width; j++)
			{
				ret = Workspace[i][j].GetEl();
				putpixel(i, j, RGB((int)(ret[0] * 255), (int)(ret[1] * 255), (int)(ret[2] * 255)), mbmp);
			}

		mbmp->misopen = 1;

		return 0;
	}

	int Width, Height;
	void floodFill(int x, int y, float clr[3], Pixel *ex)
	{
		dot tt;
		queue<dot> waitingList;
		tt.X = x; tt.Y = y;
		waitingList.push(tt);
		while(waitingList.size() > 0)
		{
			int xx = waitingList.front().X;
			int yy = waitingList.front().Y;
			if (xx >= 0 && xx < this->Height && yy >= 0 && this->Width && Workspace[xx][yy] == ex)
			{
				Workspace[xx][yy].SetEl(clr);
				tt.X = xx + 1; tt.Y = yy; waitingList.push(tt);
				tt.X = xx - 1; tt.Y = yy; waitingList.push(tt);
				tt.X = xx; tt.Y = yy - 1; waitingList.push(tt);
				tt.X = xx; tt.Y = yy + 1; waitingList.push(tt);
			}
			waitingList.pop();
		}
	}
	void floodFill(int x, int y, float clr[3], Pixel *ex, int xmin, int xmax, int ymin, int ymax)
	{
		if (x >= xmin && x <= xmax && y >= ymin && y <= ymax && Workspace[x][y] == ex)
		{
			Workspace[x][y].SetEl(clr);
			floodFill(x - 1, y, clr, ex, xmin, xmax, ymin, ymax);
			floodFill(x + 1, y, clr, ex, xmin, xmax, ymin, ymax);
			floodFill(x, y - 1, clr, ex, xmin, xmax, ymin, ymax);
			floodFill(x, y + 1, clr, ex, xmin, xmax, ymin, ymax);
		}
	}
	void init(int W, int H)
	{
		Workspace.clear();
		Width = W;
		Height = H;
		Pixel t;
		vector<Pixel> temp;
		for (int i = 0; i < Width; i++) temp.push_back(t);
		for (int i = 0; i < Height; i++) Workspace.push_back(temp);
		temp.clear();
	}
	vector< vector<Pixel> > Workspace;
};
