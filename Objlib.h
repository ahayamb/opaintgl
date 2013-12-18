#include <GL\gl.h>
#include <GL\glut.h>
#include <stdio.h>
#include <math.h>
#include <vector>
#include "ObjCanvas.h"
using namespace std;

//---------------------------------------------------------------------OBJ-------------------------------------------------------------------//
class Obj
{
protected:
	virtual void InitMod(){ }
	vector<dot> ori, mod;
	float xmax, xmin, ymax, ymin;
	float color[3];
	bool Finished, isRound;
	int ResizePoint;
	float pointSize;

public:
	bool Selected;
	float Side;
	void Rasterize(Canvas *t)
	{
		int cons = round(pointSize / 2.0);
		for (int i = 0; i < mod.size(); i++)
		{
			for (int j = mod[i].X - cons; j <= mod[i].X + cons; j++)
				for (int k = mod[i].Y - cons; k <= mod[i].Y + cons; k++)
					t->setPixelAt(j, k, color);
		}
	}
	virtual void SetInitialPoint(float x, float y, float xt, float yt) {}
	virtual void IncreaseSide() { }
	virtual void DecreaseSide() { }
	void IncreasePoint() { if (pointSize < 6) pointSize += 1.0; }
	void DecreasePoint() { if (pointSize > 1) pointSize -= 1.0; }
	void CheckSelect(float x, float y) //Processed point (x, W - y)
	{
		if (x <= xmax && x >= xmin && y <= ymax && y >= ymin) Selected = true;
		else Selected = false;
	}
	void EditLastPoint(float x, float y) //Processed point (x, W - y)
	{
		if (ori.size() >= 2)
		{
			ori[ori.size() - 2].X = x;
			ori[ori.size() - 2].Y = y;
			InitMod();
		}
	}
	virtual void SelectionDraw()
	{
		if (Selected)
		{
			if (!isRound)
			{
				glColor3f(0, 0, 0);
				glPointSize(5.0);
				glBegin(GL_POINTS);
				for (int i = 0; i < ori.size(); i++)
					glVertex2f(ori[i].X, ori[i].Y);
				glEnd();
			}
			glPushAttrib(GL_ENABLE_BIT);
			glLineStipple(1, 0xAAAA);
			glEnable(GL_LINE_STIPPLE);
			glColor3f(0, 0, 0);
			glLineWidth(0.5);
			glBegin(GL_LINE_LOOP);
			glVertex2f(xmin, ymin);
			glVertex2f(xmax, ymin);
			glVertex2f(xmax, ymax);
			glVertex2f(xmin, ymax);
			glEnd();
			glPopAttrib();
			glColor3f(0, 0, 1);
			glPointSize(10.0);
			glBegin(GL_POINTS);
			glVertex2f(xmin, ymin);
			glVertex2f(xmax, ymin);
			glVertex2f(xmax, ymax);
			glVertex2f(xmin, ymax);
			glEnd();
		}
	}
	virtual void Draw()
	{
		glColor3f(color[0], color[1], color[2]);
		glLineWidth(pointSize + 1);
		if (ori.size() > 1)
		{
			glBegin(GL_LINE_LOOP);
			for (int i = 0; i < ori.size(); i++)
				glVertex2f(ori[i].X, ori[i].Y);
			glEnd();
		}
		SelectionDraw();
	}
	virtual void AddPoint(float x, float y) { } //Processed point (x, W - y)
	virtual void Evaluate() { }
	virtual void EvaluatingFinish()
	{
		if (ori.size() > 1 && ori[0].X <= ori[ori.size() - 1].X + 4 && ori[0].X >= ori[ori.size() - 1].X - 4 && ori[0].Y <= ori[ori.size() - 1].Y + 4 && ori[0].Y >= ori[ori.size() - 1].Y - 4)
		{
			ori[ori.size() - 1].X = ori[0].X;
			ori[ori.size() - 1].Y = ori[0].Y;
			Finished = true;
			Selected = false;
			InitMod();
		}
	}
	int EvaluatingResize(float x, float y) //Processed x, y
	{
		ResizePoint = 0;
		if (xmin - 20 <= x && xmin + 20 >= x && ymin - 20 <= y && ymin + 20 >= y) ResizePoint = 1;
		else if (xmax - 20 <= x && xmax + 20 >= x && ymin - 20 <= y && ymin + 20 >= y) ResizePoint = 2;
		else if (xmax - 20 <= x && xmax + 20 >= x && ymax - 20 <= y && ymax + 20 >= y) ResizePoint = 3;
		else if (xmin - 20 <= x && xmin + 20 >= x && ymax - 20 <= y && ymax + 20 >= y) ResizePoint = 4;
		return ResizePoint;
	}
	void Translate(float dx, float dy) //Processed dx, dy
	{
		for (int i = 0; i < ori.size(); i++)
		{
			ori[i].X += dx;
			ori[i].Y += dy;
		}
		for (int i = 0; i < mod.size(); i++)
		{
			mod[i].X += dx;
			mod[i].Y += dy;
		}
		xmin += dx; xmax += dx;
		ymin += dy; ymax += dy;
	}
	void Rotate(float angle)
	{
		float deg = angle * 3.14159265 / 180.0;

		for (int i = 0; i < ori.size(); i++)
		{
			float tx = ori[i].X, ty = ori[i].Y;
			ori[i].X = (tx - GetCenterX()) * cos(deg) - (ty - GetCenterY()) * sin(deg) + GetCenterX();
			ori[i].Y = (tx - GetCenterX()) * sin(deg) + (ty - GetCenterY()) * cos(deg) + GetCenterY();
		}
		InitMod();
	}
	float GetCenterX() { return (xmin + xmax) / 2; }
	float GetCenterY() { return (ymin + ymax) / 2; }
	void Resize(float x, float y) //Processed x, y
	{
		float cx = 1, cy = 1, Cx, Cy;
		if (ResizePoint == 1)
		{
			cx = (xmax - x) / (xmax - xmin);
			cy = (ymax - y) / (ymax - ymin);
			for (int i = 0; i < ori.size(); i++)
			{
				ori[i].X = xmax - (xmax - ori[i].X) * cx;
				ori[i].Y = ymax - (ymax - ori[i].Y) * cy;
			}
		}
		else if (ResizePoint == 2)
		{
			cx = (xmin - x) / (xmin - xmax);
			cy = (ymax - y) / (ymax - ymin);
			for (int i = 0; i < ori.size(); i++)
			{
				ori[i].X = (ori[i].X - xmin) * cx + xmin;
				ori[i].Y = ymax - (ymax - ori[i].Y) * cy;
			}
		}
		else if (ResizePoint == 3)
		{
			cx = (xmin - x) / (xmin - xmax);
			cy = (ymin - y) / (ymin - ymax);
			for (int i = 0; i < ori.size(); i++)
			{
				ori[i].X = (ori[i].X - xmin) * cx + xmin;
				ori[i].Y = (ori[i].Y - ymin) * cy + ymin;
			}
		}
		else if (ResizePoint == 4)
		{
			cx = (xmax - x) / (xmax - xmin);
			cy = (ymin - y) / (ymin - ymax);
			for (int i = 0; i < ori.size(); i++)
			{
				ori[i].X = xmax - (xmax - ori[i].X) * cx;
				ori[i].Y = (ori[i].Y - ymin) * cy + ymin;
			}
		}
		InitMod();
	}
	bool isFinished() { return Finished; }
	int isResizing() { return ResizePoint; }

};


//---------------------------------------------------------------------SQUARE-------------------------------------------------------------------//
class Square : public Obj
{
public:
	Square(float r, float g, float b)
	{
		color[0] = r;
		color[1] = g;
		color[2] = b;
		ori.clear();
		mod.clear();
		Selected = true;
		Finished = false;
		ResizePoint = 0;
		pointSize = 1.0;
		isRound = false;
		dot pa;
		pa.X = 0; pa.Y = 0; ori.push_back(pa);
		pa.X = 1; pa.Y = 0; ori.push_back(pa);
		pa.X = 1; pa.Y = 1; ori.push_back(pa);
		pa.X = 0; pa.Y = 1; ori.push_back(pa);
		pa.X = 0; pa.Y = 0; ori.push_back(pa);
	}
	void SetInitialPoint(float x, float y, float xt, float yt)
	{
		dot t;
		float xs = x > xt ? x : xt;
		float ys = y > yt ? y : yt;
		float xr = x > xt ? x - xt : xt - x;
		float yr = y > yt ? y - yt : yt - y;
		ori[0].X = xs; ori[0].Y = ys;
		ori[1].X = xs + xr; ori[1].Y = ys;
		ori[2].X = xs + xr; ori[2].Y = ys + yr;
		ori[3].X = xs; ori[3].Y = ys + yr;
		ori[4].X = xs; ori[4].Y = ys;
		InitMod();
	}
private:
	void InitMod()
	{
		mod.clear();
		float t = 0;
		dot pa;
		xmax = ymax = -1;
		xmin = ymin = 1000;
//		if (ori.size() == 4) ori.push_back(ori[0]);
		while (t <= 1.0)
		{
			for (int i = 0; i < ori.size() - 1; i += 1)
			{
				pa.X = (1.0 - t) * ori[i].X + t * ori[i + 1].X;
				pa.Y = (1.0 - t) * ori[i].Y + t * ori[i + 1].Y;

				xmax = xmax < pa.X ? pa.X : xmax;
				ymax = ymax < pa.Y ? pa.Y : ymax;
				xmin = xmin > pa.X ? pa.X : xmin;
				ymin = ymin > pa.Y ? pa.Y : ymin;

				mod.push_back(pa);
			}
			t += 0.001;
		}
	}
};

//---------------------------------------------------------------------ROUND-------------------------------------------------------------------//
class Round : public Obj
{
public:
	Round(float r, float g, float b)
	{
		color[0] = r;
		color[1] = g;
		color[2] = b;
		ori.clear();
		mod.clear();
		Selected = true;
		Finished = false;
		ResizePoint = 0;
		pointSize = 1.0;
		isRound = true;
	}
	void SetInitialPoint(float x, float y, float xt, float yt)
	{
		ori.clear();
		dot t;
		float xs = x > xt ? x : xt;
		float ys = y > yt ? y : yt;
		float xr = x > xt ? x - xt : xt - x;
		float yr = y > yt ? y - yt : yt - y;
		for (float i = 0; i <= 360; i += 1)
		{
			t.X = xr / 2 * cos(i * 3.14 / 180.0) + xs;
			t.Y = yr / 2 * sin(i * 3.14 / 180.0) + ys;
			ori.push_back(t);
		}
		InitMod();
	}
private:
	void InitMod()
	{
		mod.clear();
		float t = 0;
		dot pa;
		xmax = ymax = -1;
		xmin = ymin = 1000;
		while (t <= 1.0)
		{
			for (int i = 0; i < ori.size() - 1; i += 1)
			{
				pa.X = (1.0 - t) * ori[i].X + t * ori[i + 1].X;
				pa.Y = (1.0 - t) * ori[i].Y + t * ori[i + 1].Y;

				xmax = xmax < pa.X ? pa.X : xmax;
				ymax = ymax < pa.Y ? pa.Y : ymax;
				xmin = xmin > pa.X ? pa.X : xmin;
				ymin = ymin > pa.Y ? pa.Y : ymin;

				mod.push_back(pa);
			}
			t += 0.1;
		}
	}
};

//---------------------------------------------------------------------POLYSIDE-------------------------------------------------------------------//
class Polyside : public Obj
{
public:
	Polyside(float r, float g, float b)
	{
		color[0] = r;
		color[1] = g;
		color[2] = b;
		ori.clear();
		mod.clear();
		Selected = true;
		Finished = false;
		ResizePoint = 0;
		pointSize = 1.0;
		isRound = false;
		Side = 3;
	}
	void IncreaseSide()
	{
		dot t;
		if (Side < 10)
		{
			Side += 1.0;
			ori.clear();
		 	for (float i = 0; i <= 360; i += (360.0 / Side))
			{
				t.X = (xmax - xmin) / 2 * cos(i * 3.14 / 180.0) + GetCenterX();
				t.Y = (ymax - ymin) / 2 * sin(i * 3.14 / 180.0) + GetCenterY();
				ori.push_back(t);
			}
			InitMod();
		 }
	}
	void DecreaseSide()
	{
		dot t;
		if (Side > 3)
		{
			Side -= 1.0;
			ori.clear();
			for (float i = 0; i <= 360; i += (360.0 / Side))
			{
				t.X = (xmax - xmin) / 2 * cos(i * 3.14 / 180.0) + GetCenterX();
				t.Y = (ymax - ymin) / 2 * sin(i * 3.14 / 180.0) + GetCenterY();
				ori.push_back(t);
			}
			InitMod();
		}
	}
	void SetInitialPoint(float x, float y, float xt, float yt)
	{
		ori.clear();
		dot t;
		float xs = x > xt ? x : xt;
		float ys = y > yt ? y : yt;
		float xr = x > xt ? x - xt : xt - x;
		float yr = y > yt ? y - yt : yt - y;
		for (float i = 0; i <= 360; i += (360.0 / Side))
		{
			t.X = xr * cos(i * 3.14 / 180.0) + xs;
			t.Y = yr * sin(i * 3.14 / 180.0) + ys;
			ori.push_back(t);
		}
		InitMod();
	}
private:
	void InitMod()
	{
		mod.clear();
		float t = 0;
		dot pa;
		xmax = ymax = -1;
		xmin = ymin = 1000;
		while (t <= 1.0)
		{
			for (int i = 0; i < ori.size() - 1; i += 1)
			{
				pa.X = (1.0 - t) * ori[i].X + t * ori[i + 1].X;
				pa.Y = (1.0 - t) * ori[i].Y + t * ori[i + 1].Y;

				xmax = xmax < pa.X ? pa.X : xmax;
				ymax = ymax < pa.Y ? pa.Y : ymax;
				xmin = xmin > pa.X ? pa.X : xmin;
				ymin = ymin > pa.Y ? pa.Y : ymin;

				mod.push_back(pa);
			}
			t += 0.01;
		}
	}
};


//---------------------------------------------------------------------FREEHAND-------------------------------------------------------------------//
class Freehand : public Obj
{
public:
	Freehand(float r, float g, float b)
	{
		color[0] = r;
		color[1] = g;
		color[2] = b;
		ori.clear();
		mod.clear();
		Selected = true;
		Finished = false;
		ResizePoint = 0;
		pointSize = 1.0;
		isRound = false;
	}
	void AddPoint(float x, float y) //Processed point (x, W - y)
	{
		dot ta;
		ta.X = x; ta.Y = y;
		ori.push_back(ta);
		InitMod();
	}
	void Draw()
	{
		glColor3f(color[0], color[1], color[2]);
		glPointSize(pointSize);
		if (mod.size() > 1)
		{
			glBegin(GL_POINTS);
			for (int i = 0; i < mod.size(); i++)
				glVertex2f(mod[i].X, mod[i].Y);
			glEnd();
		}
		SelectionDraw();
	}
private:
	void InitMod()
	{
		mod.clear();
		float t = 0;
		dot pa;
		xmax = ymax = -1;
		xmin = ymin = 1000;
		while (t <= 1.0)
		{
			for (int i = 0; i < ori.size() - 1; i += 1)
			{
				pa.X = (1.0 - t) * ori[i].X + t * ori[i + 1].X;
				pa.Y = (1.0 - t) * ori[i].Y + t * ori[i + 1].Y;

				xmax = xmax < pa.X ? pa.X : xmax;
				ymax = ymax < pa.Y ? pa.Y : ymax;
				xmin = xmin > pa.X ? pa.X : xmin;
				ymin = ymin > pa.Y ? pa.Y : ymin;

				mod.push_back(pa);
			}
			t += 0.01;
		}
	}
};


//---------------------------------------------------------------------CURVE-------------------------------------------------------------------//
class Curve : public Obj
{
public:
	bool Selected;
	Curve(float r, float g, float b)
	{
		color[0] = r;
		color[1] = g;
		color[2] = b;
		ori.clear();
		mod.clear();
		Selected = true;
		Finished = false;
		ResizePoint = 0;
		pointSize = 1.0;
		isRound = false;
	}
	void Draw()
	{
		glColor3f(color[0], color[1], color[2]);
		glPointSize(pointSize);
		if (mod.size() > 1)
		{
			glBegin(GL_POINTS);
			for (int i = 0; i < mod.size(); i++)
				glVertex2f(mod[i].X, mod[i].Y);
			glEnd();
		}
		SelectionDraw();
	}
	void AddPoint(float x, float y) //Processed point (x, W - y)
	{
		dot pa;
		pa.X = x; pa.Y = y;
		ori.push_back(pa);
		InitMod();
	}
	void Evaluate() { if (ori.size() == 2) ori.pop_back(); }
private:
	void InitMod()
	{
		mod.clear();
		float t = 0;
		dot pa;
		xmax = ymax = -1;
		xmin = ymin = 1000;
		while (t <= 1.0)
		{
			for (int i = 0; i + 2 < ori.size(); i += 2)
			{
				pa.X = (1.0 - t) * ((1.0 - t) * ori[i].X + t * ori[i + 1].X) + t * ((1.0 - t) * ori[i + 1].X + t * ori[i + 2].X);
				pa.Y = (1.0 - t) * ((1.0 - t) * ori[i].Y + t * ori[i + 1].Y) + t * ((1.0 - t) * ori[i + 1].Y + t * ori[i + 2].Y);

				xmax = xmax < pa.X ? pa.X : xmax;
				ymax = ymax < pa.Y ? pa.Y : ymax;
				xmin = xmin > pa.X ? pa.X : xmin;
				ymin = ymin > pa.Y ? pa.Y : ymin;

				mod.push_back(pa);
			}
			t += 0.001;
		}
	}
};

