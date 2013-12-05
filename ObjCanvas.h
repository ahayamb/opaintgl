#include <stdio.h>
#include <math.h>
#include <vector>
#include <GL\gl.h>
#include <GL\glut.h>
using namespace std;

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
		int xmin = x - 100 > 0 ? x - 100 : 0;
		int xmax = x + 100 > Height ? Height - 1 : x + 100;
		int ymin = y - 100 > 0 ? y - 100 : 0;
		int ymax = y + 100 > Width ? Width - 1 : y + 100;
		floodFill(x, y, clr, &a, xmin, xmax, ymin, ymax);
	}

private:
	int Width, Height;
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

