#include <GL\gl.h>
#include <GL\glut.h>
#include <stdio.h>
#include <math.h>
#include <vector>
#include "ObjCanvas.h"
using namespace std;

//---------------------------------------------------------------------OBJ-------------------------------------------------------------------//
//Kelas dasar yang menjadi interface bagi fungsi utama
class Obj
{
public:
	bool Selected;	//variabel penanda apakah objek yang
	float Side;
	virtual void Rasterize(Canvas *t) //method yang digunakan untuk rasterisasi (mengubah ke domain piksel) objek
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
	virtual void IncreaseSide() { } //method yang digunakan untuk menambah side (dioverride di kelas polyside)
	virtual void DecreaseSide() { } //method yang digunakan untuk mengurangi side (dioverride di kelas polyside)
	virtual void IncreasePoint() { if (pointSize < 6) pointSize += 1.0; } //nonvirtual, digunakan untuk menambah ketebalan garis objek
	virtual void DecreasePoint() { if (pointSize > 1) pointSize -= 1.0; } //nonvirtual, digunakan untuk mengurangi ketebalan garis objek
	virtual void CheckSelect(float x, float y) //Processed point (x, W - y) //nonvirtual, digunakan untuk mengecek apakah objek akan terseleksi atau tidak
	{
		if (x <= xmax && x >= xmin && y <= ymax && y >= ymin) Selected = true;
		else Selected = false;
	}
	virtual void EditLastPoint(float x, float y) //Processed point (x, W - y), nonvirtual, method untuk mengedit titik terakhir dari objek. Digunakan sebagai akomodasi vektor yang bertipe private
	{
		if (ori.size() >= 2)
		{
			ori[ori.size() - 2].X = x;
			ori[ori.size() - 2].Y = y;
			InitMod();
		}
	}
	virtual void SelectionDraw() //digunakan untuk menggambar daerah seleksi objek
	{
		if (Selected && ori.size() > 0)
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
	virtual void Draw()	//digunakan untuk menggambar objek
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
	virtual void EvaluatingFinish() //digunakan untuk menyatukan titik awal dan titik akhir
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
	virtual int EvaluatingResize(float x, float y) //Processed x, y nonvirtual. Digunakan untuk memeriksa dari titik mana objek diseleksi
	{
		ResizePoint = 0;
		if (xmin - 20 <= x && xmin + 20 >= x && ymin - 20 <= y && ymin + 20 >= y) ResizePoint = 1;
		else if (xmax - 20 <= x && xmax + 20 >= x && ymin - 20 <= y && ymin + 20 >= y) ResizePoint = 2;
		else if (xmax - 20 <= x && xmax + 20 >= x && ymax - 20 <= y && ymax + 20 >= y) ResizePoint = 3;
		else if (xmin - 20 <= x && xmin + 20 >= x && ymax - 20 <= y && ymax + 20 >= y) ResizePoint = 4;
		return ResizePoint;
	}
	virtual void Translate(float dx, float dy) //Processed dx, dy nonvirtual. Method untuk memindah objek
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
	virtual void Rotate(float angle) //nonvirtual. Method untuk merotasi objek
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
	virtual void Resize(float x, float y) //Processed x, y nonvirtual. Method untuk merubah ukuran objek
	{
		float cx = 1, cy = 1, Cx, Cy;
		if (ResizePoint == 1)	//jika ukuran diubah dari titik sebelah kiri bawah
		{
			cx = (xmax - x) / (xmax - xmin);
			cy = (ymax - y) / (ymax - ymin);
			for (int i = 0; i < ori.size(); i++)
			{
				ori[i].X = xmax - (xmax - ori[i].X) * cx;
				ori[i].Y = ymax - (ymax - ori[i].Y) * cy;
			}
		}
		else if (ResizePoint == 2)	//jika ukuran diubah dari titik sebelah kanan bawah
		{
			cx = (xmin - x) / (xmin - xmax);
			cy = (ymax - y) / (ymax - ymin);
			for (int i = 0; i < ori.size(); i++)
			{
				ori[i].X = (ori[i].X - xmin) * cx + xmin;
				ori[i].Y = ymax - (ymax - ori[i].Y) * cy;
			}
		}
		else if (ResizePoint == 3) //jika ukuran diubah dari titik sebelah kanan atas
		{
			cx = (xmin - x) / (xmin - xmax);
			cy = (ymin - y) / (ymin - ymax);
			for (int i = 0; i < ori.size(); i++)
			{
				ori[i].X = (ori[i].X - xmin) * cx + xmin;
				ori[i].Y = (ori[i].Y - ymin) * cy + ymin;
			}
		}
		else if (ResizePoint == 4) //jika ukuran diubah dari titik sebelah kiri atas
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
	virtual bool isFinished() { return Finished; } //nonvirtual
	virtual int isResizing() { return ResizePoint; } //nonvirtual
protected:
	Bitmap bmImg;
	unsigned char *getpixel(int x, int y, Bitmap *mbmp){}
	void closebmp(Bitmap *mbmp) {}
	int openbmp(char *fname, Bitmap *mbmp){}
	virtual void InitMod(){ }
	vector<dot> ori, mod;
	float xmax, xmin, ymax, ymin;
	float color[3];
	bool Finished, isRound;
	int ResizePoint;
	float pointSize;
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
	void SetInitialPoint(float x, float y, float xt, float yt)	//untuk kelas square, titik awal adalah 5 titik. Karena jika 4 titik, titik terakhir tidak tergambar
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
	void SetInitialPoint(float x, float y, float xt, float yt)	//Untuk kelas Round, titik awal adalah 360 titik yang didapatkan
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
		 	for (float i = 0; i <= 360; i += (360.0 / Side))	//banyak sisi
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
		//printf("%d\n", mod.size());
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

		for (int i = 0; i < ori.size() - 1; i++)
		{
			float d = sqrtf(pow((ori[i].X - ori[i + 1].X), 2) + pow((ori[i].Y - ori[i + 1].Y), 2));
			while(t <= 1.0)
			{
				pa.X = (1.0 - t) * ori[i].X + t * ori[i + 1].X;
				pa.Y = (1.0 - t) * ori[i].Y + t * ori[i + 1].Y;

				xmax = xmax < pa.X ? pa.X : xmax;
				ymax = ymax < pa.Y ? pa.Y : ymax;
				xmin = xmin > pa.X ? pa.X : xmin;
				ymin = ymin > pa.Y ? pa.Y : ymin;

				mod.push_back(pa);

				if (d < 150) t += 0.01;
				else t += 0.001;
			}
			t = 0;
		}

		//printf("free %d\n", mod.size());
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

class BitmapImg : public Obj
{
public:
	BitmapImg(char *fname)
	{
		pointSize = 1;
		openbmp(fname, &bmImg);
		xmin = 0;
		xmax = xmin + bmImg.width * pointSize;
		ymin = 0;
		ymax = ymin + bmImg.height  * pointSize;
		Finished = true;
	}
	bool Selected;
	float Side;
	void Rasterize(Canvas *t)
	{
		//printf("Rasterize");
		for (int i = 0; i < bmImg.width; i++)
			for (int j = 0; j < bmImg.height; j++)
			{
				for (int x = pointSize * i + xmin; x <= pointSize * (i + 1) + xmin; x++)
					for (int y = pointSize * j + ymin; y <= pointSize * (j + 1) + ymin; y++)
					{
						float c[3];
						unsigned char *clr = getpixel(i, j, &bmImg);
						c[0] = (float)clr[0] / 255.0;
						c[1] = (float)clr[1] / 255.0;
						c[2] = (float)clr[2] / 255.0;
						t->setPixelAt(x, y, c);
					}
			}
	}
	void SetInitialPoint(float x, float y, float xt, float yt) {}
	void IncreaseSide() { }
	void DecreaseSide() { }
	void IncreasePoint() { } //nonvirtual
	void DecreasePoint() { } //nonvirtual
	void EditLastPoint(float x, float y) //Processed point (x, W - y) //nonvirtual
	{
	}
	void SelectionDraw()
	{
		if (Selected)
		{
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
	void Draw()
	{
		for (int x = 0; x < bmImg.width; x++)
			for (int y = 0; y < bmImg.height; y++)
			{
				unsigned char *clr = getpixel(x, y, &bmImg);
				glColor3f((float)clr[0] / 255.0, (float)clr[1] / 255.0, (float)clr[2] / 255.0);
				glBegin(GL_QUADS);
					glVertex2f(pointSize * x + xmin, pointSize * y + ymin);
					glVertex2f(pointSize * (x + 1) + xmin, pointSize * y + ymin);
					glVertex2f(pointSize * (x + 1) + xmin, pointSize * (y + 1) + ymin);
					glVertex2f(pointSize * x + xmin, pointSize * (y + 1) + ymin);
				glEnd();
			}
		SelectionDraw();
	}
	void AddPoint(float x, float y) { } //Processed point (x, W - y)
	void Evaluate() { }
	void EvaluatingFinish() { }
	int EvaluatingResize(float x, float y) //Processed x, y //nonvirtual
	{
		ResizePoint = 0;
		if (xmin - 20 <= x && xmin + 20 >= x && ymin - 20 <= y && ymin + 20 >= y) ResizePoint = 1;
		else if (xmax - 20 <= x && xmax + 20 >= x && ymin - 20 <= y && ymin + 20 >= y) ResizePoint = 2;
		else if (xmax - 20 <= x && xmax + 20 >= x && ymax - 20 <= y && ymax + 20 >= y) ResizePoint = 3;
		else if (xmin - 20 <= x && xmin + 20 >= x && ymax - 20 <= y && ymax + 20 >= y) ResizePoint = 4;
		return ResizePoint;
	}
	void Rotate(float angle) //nonvirtual
	{

	}
	float GetCenterX() { return (xmin + xmax) / 2; }
	float GetCenterY() { return (ymin + ymax) / 2; }
	void Resize(float x, float y) //Processed x, y //nonvirtual
	{
		if (ResizePoint == 1)
		{
			pointSize = (xmax - x) / bmImg.width < (ymax - y) / bmImg.height ? (xmax - x) / bmImg.width : (ymax - y) / bmImg.height;
			xmin = x;
			ymin = y;
			xmax = xmin + (float)bmImg.width * pointSize;
			ymax = ymin + (float)bmImg.height * pointSize;
		}
		else if (ResizePoint == 2)
		{

		}
		else if (ResizePoint == 3)
		{
		}
		else if (ResizePoint == 4)
		{
		}
	}
	bool isFinished() { return Finished; } //nonvirtual
	int isResizing() { return ResizePoint; } //nonvirtual
private:
	Bitmap bmImg;
	void closebmp(Bitmap *mbmp)
	{
		if (mbmp->misopen)
		{
			mbmp->width = mbmp->height = 0;
			if (mbmp->data != NULL)
			{
				free(mbmp->data);
				mbmp->data = NULL;
			}
		}
	}
	int openbmp(char *fname, Bitmap *mbmp)
	{
		FILE *fp;
		BmpHeader head = {0};

		if ((fp = fopen(fname, "rb")) == NULL)
		{
			printf("Error: failed to open \"%s\".\n", fname);
			return 1;
		}

		if (fgetc(fp) != 'B' || fgetc(fp) != 'M')
		{
			printf("Error: bmp file \"%s\" format error.", fname);
			goto ERR_EXIT;
		}

		fread(&head, sizeof head, 1, fp);
		if (head.sizeStruct != 40 || head.reserved != 0)
		{
			printf("Error: bmp file \"%s\" format error.", fname);
			goto ERR_EXIT;
		}

		if (head.bitCount != 24)
		{
			printf("Sorry: bmp file \"%s\" bit count != 24", fname);
			goto ERR_EXIT;
		}

		if (mbmp->misopen)
			closebmp(mbmp);

		mbmp->width = head.width;
		mbmp->height = head.height;
		mbmp->size = head.sizeImage;
		if (mbmp->size != CEIL4(mbmp->width * 3) * mbmp->height)
		{
			printf("Error: bmp file \"%s\" size do not match!\n", fname);
			goto ERR_EXIT;
		}
		if ((mbmp->data = (unsigned char*)realloc(mbmp->data, head.sizeImage)) == NULL)
		{
			printf("Error: alloc fail!");
			goto ERR_EXIT;
		}
		if (fread(mbmp->data, 1, mbmp->size, fp) != mbmp->size)
		{
			printf("Error: read data fail!");
			goto ERR_EXIT;
		}

		mbmp->misopen = 1;
		fclose(fp);
		return 0;

	ERR_EXIT:
		mbmp->misopen = 0;
		fclose(fp);
		return 1;
	}

	unsigned char *getpixel(int x, int y, Bitmap *mbmp)
	{
		unsigned char *p;

		if (x < mbmp->width && y < mbmp->height)
		{
			p = mbmp->data + CEIL4(mbmp->width * 3) * y + x * 3;
			unsigned char ret[3] = {p[2], p[0], p[1]};
			return ret;
		}
		return 0;
	}
};
