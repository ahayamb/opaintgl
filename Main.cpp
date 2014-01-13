#include <windows.h>
#include "Objlib.h"

/*
Shortcut pemilihan toolbox
0 : isDragging
1 : Curve (Shift + C)
2 : Select tool & Move tool (S)
3 : Resize (Z)
4 : Rotate (R)
5 : Round tool (Shift + X)
6 : Square (Shift + S)
7 : Polyside (Shift + G)
8 : Freehand (Shift + F)
9 : Fill (F)
10 : Erase (E)
11 : Brush (B)
*/

int W = 800, H = 600, toolSize = 20, tcurrentIdx = -1;
Obj *t, *tcurrent;
vector<Obj*> Objs;
Obj *tp;
Canvas *workspace;
bool flagTool[20];
int Xinit, Yinit;
float r = 0, g = 0, b = 0;

//--------------------------------------------------Associated Property/Function of Color Picker-------------------------------
void init_toolbox(void)
{
	glClearColor( 1, 1, 1, 1);
	gluOrtho2D(0, 100, 0, 300);
}

/*
	fungsi yang mengeset flag tool yang sedang aktif, jika idx = -1, maka semua tool akan deactivate
*/
void resetFlag(int idx)
{
	memset(flagTool, false, sizeof(flagTool));
	if (idx >= 0) flagTool[idx] = true;
}

// Mengatur window color picker
void init_ColorPicker(void)
{
	glClearColor( 1, 1, 1, 1);
	glClearDepth( 1.0 );
	gluOrtho2D(0, 290, -20, 260);
}
// Menampilkan status bar tentang tool apa yang sekarang aktif
void text()
{
	char text[32];
	glColor3f(0,0,0);
	int R = (int)(r * 255);
	int G = (int)(g * 255);
	int B = (int)(b * 255);
	sprintf(text, "R : %d G : %d B : %d", R, G, B);
	glRasterPos2f(0, -13);
	for(int i = 0; text[i] != '\0'; i++)
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, text[i]);
}
/*
	Fungsi display untuk jendela color picker
	elemen r, g, dan b dianalogikan sebagai kubus
	Sumbu x menyatakan intensitas warna biru, sumbu y menyatakan warna hijau, elemen r dinyatakan dengan slide bar di sebelah kanan color picker
*/
void ColorPicker()
{
	glClear(GL_COLOR_BUFFER_BIT);
	glPointSize(1.0);
	for (float i = 0; i < 256; i += 1.0)
		for (float j = 0; j < 256; j += 0.1)
		{
			glColor3f(r, i / 255, j / 255);
			glBegin(GL_POINTS);
			glVertex2f(i, j);
			glEnd();
		}
	glBegin(GL_POLYGON);
	glColor3f(0, 0, 0);
	glVertex2f(285, 0);
	glVertex2f(261, 0);
	glColor3f(1, 0, 0);
	glVertex2f(261, 255);
	glVertex2f(285, 255);
	glEnd();
	glPointSize(10.0);
	glColor3f(r, g, b);
	glBegin(GL_POINTS);
	glVertex2f(250, -9);
	glEnd();
	text();
	glFlush();
}

int posTool = 0;
int Xtool = -1, Ytool = -1;
// Interface untuk operasi klik pada window toolbox (pengaktifan tool)
void toolBoxMouse(int button, int state, int x, int y)
{
	y = 300 - y;
	if (state == GLUT_DOWN)
	{
		Ytool = y / 50;
		Xtool = x / 50;
		posTool = Ytool + Xtool * 6;
		glutPostRedisplay();

		if (posTool == 3)
		{
			resetFlag(1);
			if (t != NULL)
			{
				t->Selected = false;
				if (Objs.size() == 0 || Objs[Objs.size() - 1] != t)
				Objs.push_back(t);
			}
			t = new Curve(r, g, b);
		}
		else if (posTool == 4)
		{
			resetFlag(6);
			if (t != NULL)
			{
				t->Selected = false;
				Objs.push_back(t);
			}
			t = new Square(r, g, b);
		}
		else if (posTool == 5)
		{
			resetFlag(2);
			if (t != NULL)
			{
				t->Selected = false;
				if (Objs.size() == 0 || Objs[Objs.size() - 1] != t)
				Objs.push_back(t);
			}
		}
		else if (posTool == 0)
		{
			resetFlag(3);
			if (t != NULL) Objs.push_back(t);
			if(tcurrent != NULL) tcurrent->Selected = true;
		}
		else if (posTool == 7)
		{
			resetFlag(4);
			if (t != NULL) Objs.push_back(t);
			if(tcurrent != NULL) tcurrent->Selected = true;
		}
		else if (posTool == 11)
		{
			resetFlag(5);
			if (t != NULL)
			{
				t->Selected = false;
				Objs.push_back(t);
			}
			t = new Round(r, g, b);
		}
		else if (posTool == 4)
		{
			resetFlag(6);
			if (t != NULL)
			{
				t->Selected = false;
				Objs.push_back(t);
			}
			t = new Square(r, g, b);
		}
		else if (posTool == 2)
		{
			resetFlag(7);
			if (t != NULL)
			{
				t->Selected = false;
				Objs.push_back(t);
			}
			t = new Polyside(r, g, b);
		}
		else if (posTool == 10)
		{
			resetFlag(8);
			if (t != NULL)
			{
				t->Selected = false;
				Objs.push_back(t);
			}
			t = new Freehand(r, g, b);
		}
		else if (posTool == 8)
		{
			resetFlag(9);
			if (t != NULL) Objs.push_back(t);
			if(tcurrent != NULL) tcurrent->Selected = true;
		}
		else if (posTool == 9)
		{
			resetFlag(11);
			if (t != NULL) Objs.push_back(t);
			if(tcurrent != NULL) tcurrent->Selected = true;
		}
		else if (posTool == 6)
		{
			if(tcurrent != NULL)
			{
				tcurrent->Rasterize(workspace);
			}
			else if (t != NULL)
			{
				t->Rasterize(workspace);
				t = NULL;
			}
		}
	}
}

void drawToolBox()
{
	glClear(GL_COLOR_BUFFER_BIT);
	tp->Draw();
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4f(1, 1, 1, 0.5);
	glBegin(GL_POLYGON);
	printf("%d %d\n", Xtool, Ytool);
	if (Xtool != -1 && Ytool != -1)
	{
		glVertex2f( (Xtool) * 50 + 5, (Ytool) * 50 + 5 );
		glVertex2f( (Xtool + 1) * 50 - 5, (Ytool) * 50 + 5 );
		glVertex2f( (Xtool + 1) * 50 - 5, (Ytool + 1) * 50 - 5 );
		glVertex2f( (Xtool) * 50 + 5, (Ytool + 1) * 50 - 5 );
	}
	glEnd();
	glFlush();
}

// Fungsi mouse untuk memilih warna pada jendela color picker, warna yang dipilih akan menjadi warna default untuk penggambaran bangun dan region filling
void MouseFunc_ColorPicker(int button, int state, int x, int y)
{
	if (state == GLUT_DOWN)
	{
		if (x > 256 && 256 - y >= 0)
		{
			r = (float)(256 - y) / 255;
			glutPostRedisplay();
		}
		else if (x >= 0 && 256 - y >= 0)
		{
			g = (float)(x) / 255;
			b = (float)(256 - y) / 255;
			glutPostRedisplay();
		}
	}
}
//-----------------------------------------------------------------------------------------------------------------------------
// Fungsi untuk menampilkan active tool toolbar
void ToolInformationBar()
{
	glColor3f(0.1, 1, 1);
	glBegin(GL_POLYGON);
	glVertex2i(0, 600);
	glVertex2i(800, 600);
	glVertex2i(800, 630);
	glVertex2i(0, 630);
	glEnd();
	glColor3f(0,0,0);
	char text[200] = "IDLE";
	if (flagTool[1]) sprintf(text, "CURVE Selected | Curve(C), Select & Move (s), Resize(z), Rotate(r), Round(X), Square(S), Polyside(G), Freehand(F), Fill(f), Erase(e), Brush(b), Rasterize(q)");
	else if (flagTool[2]) sprintf(text, "SELECT Selected | Curve(C), Select & Move (s), Resize(z), Rotate(r), Round(X), Square(S), Polyside(G), Freehand(F), Fill(f), Erase(e), Brush(b), Rasterize(q)");
	else if (flagTool[3]) sprintf(text, "RESIZE Selected | Curve(C), Select & Move (s), Resize(z), Rotate(r), Round(X), Square(S), Polyside(G), Freehand(F), Fill(f), Erase(e), Brush(b), Rasterize(q)");
	else if (flagTool[4]) sprintf(text, "ROTATE Selected | Curve(C), Select & Move (s), Resize(z), Rotate(r), Round(X), Square(S), Polyside(G), Freehand(F), Fill(f), Erase(e), Brush(b), Rasterize(q)");
	else if (flagTool[5]) sprintf(text, "ROUND Selected | Curve(C), Select & Move (s), Resize(z), Rotate(r), Round(X), Square(S), Polyside(G), Freehand(F), Fill(f), Erase(e), Brush(b), Rasterize(q)");
	else if (flagTool[6]) sprintf(text, "SQUARE Selected | Curve(C), Select & Move (s), Resize(z), Rotate(r), Round(X), Square(S), Polyside(G), Freehand(F), Fill(f), Erase(e), Brush(b), Rasterize(q)");
	else if (flagTool[7]) sprintf(text, "POLYSIDE Selected | Curve(C), Select & Move (s), Resize(z), Rotate(r), Round(X), Square(S), Polyside(G), Freehand(F), Fill(f), Erase(e), Brush(b), Rasterize(q)");
	else if (flagTool[8]) sprintf(text, "FREEHAND Selected | Curve(C), Select & Move (s), Resize(z), Rotate(r), Round(X), Square(S), Polyside(G), Freehand(F), Fill(f), Erase(e), Brush(b), Rasterize(q)");
	else if (flagTool[9]) sprintf(text, "FILL Selected | Curve(C), Select & Move (s), Resize(z), Rotate(r), Round(X), Square(S), Polyside(G), Freehand(F), Fill(f), Erase(e), Brush(b), Rasterize(q)");
	else if (flagTool[10]) sprintf(text, "ERASE Selected | Curve(C), Select & Move (s), Resize(z), Rotate(r), Round(X), Square(S), Polyside(G), Freehand(F), Fill(f), Erase(e), Brush(b), Rasterize(q)");
	else if (flagTool[11]) sprintf(text, "BRUSH Selected | Curve(C), Select & Move (s), Resize(z), Rotate(r), Round(X), Square(S), Polyside(G), Freehand(F), Fill(f), Erase(e), Brush(b), Rasterize(q)");

	glColor3f(0,0,0);
	glRasterPos2f(0, 605);
	for(int i = 0; text[i] != '\0'; i++)
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, text[i]);
}
/*
	fungsi initialisasi window utama
*/
void init(int W, int H)
{
	glClearColor(1, 1, 1, 1);
	gluOrtho2D(0, W, 0, H + 30);
}
/*
	fungsi gambar untuk window utama
	vector Objs adalah vector yang berisi daftar objek
	workspace adalah background dengan konsep piksel. Region filling hanya bisa diterapkan pada gambar yang berada pada domain piksel.
	Untuk mewarnai objek, maka objek harus dirasterisasi terlebih dahulu, setelah dirasterisasi, objek akan berada pada domain piksel.
*/
void DisplayFunc()
{
	glClear(GL_COLOR_BUFFER_BIT);

	if (workspace != NULL) workspace->DrawToScreen();

	for (int i = 0; i < Objs.size(); i++)
		if (Objs[i] != NULL) Objs[i]->Draw();
	if (t != NULL) t->Draw();

	ToolInformationBar();

	for (int i = 0; i < Objs.size(); i++)
		if (Objs[i] != NULL) //printf("%d : %d\n", i, Objs[i]->Selected);
	if (Objs.size() > 0)
	{
		//printf("objs num %d\n", tcurrentIdx);
		//puts("\n");
	}

	glFlush();
}

/*
	Fungsi mouse untuk window utama
*/
void MouseFunc(int button, int state, int x, int y)
{
	if (state == GLUT_DOWN && button == GLUT_LEFT_BUTTON)
	{
		flagTool[0] = true;
		Xinit = x;
		Yinit = (600 - y + 30);
		/*
			Kondisi dimana curve tool dipilih. Awal untuk menggambar kurva, ditambahkan dua titik yang sama, dan ketika user men-drag titik yang telah dibuat, maka titik yang
			terakhir ditambahkan akan dipindah sesuai dengan posisi mouse dilepaskan oleh user. Untuk menggambar kurva, digunakan de casteljau's algorithm, minimal 2 titik bisa
			digambar dengan menggunakan algoritma ini sehingga garis yang digambarkan bisa membentuk kurva
		*/
		if (flagTool[1])
		{
			glutPostRedisplay();
			if (t->isFinished())
			{
				Objs.push_back(t);
				t = new Curve(r, g, b);
			}
			t->Selected = true;
			t->AddPoint((float)x, (float)(600 - y + 30));
			t->AddPoint((float)Xinit, (float)Yinit);
			t->Evaluate();
		}
		/*
			Kondisi yang mengatur pemilihan objek. Jika input klik yang diberikan user mengenai suatu objek (berada di daerah objek tertentu), maka status selected objek itu akan diaktifkan,
			sedangkan status objek lain di deactivate. Program ini hanya bisa menjalankan single selection, jika ada lebih dari satu objek yang dikenai oleh input mouse user,
			maka objek yang paling akhir dibuat yang akan diseleksi. KOndisi ini juga menangani perpindahan objek
		*/
		else if (flagTool[2])
		{
			for (int i = Objs.size() - 1; i >= 0; i--)
			{
				Objs[i]->CheckSelect((float)x, (float)(600 - y + 30));
				if (Objs[i]->Selected)
				{
					tcurrent = Objs[i];
					tcurrentIdx = i;
					for (int j = i - 1; j >= 0; j--) Objs[j]->Selected = false;
					break;
				}
				tcurrent = NULL;
				tcurrentIdx = -1;
			}
			if (tcurrent != NULL) tcurrent->Selected = true;
		}
		/*
			KOndisi yang digunakan untuk menangani pengubahan ukuran objek. Untuk pengubahan ukuran, harus diketahui, dari titik mana objek akan diubah ukurannya, dari kiri bawah, kanan bawah, kiri atas atau kanan atas.
			Setiap perbesaran dari titik yang berbeda, akan menghasilkan langkah2 pembesaran yang berbeda pula. Evaluatingresize adalah method yang menangani
		*/
		else if (flagTool[3])
		{
			if(tcurrent != NULL) tcurrent->EvaluatingResize((float)x, (float)(600 - y + 30));
		}
		/*
			KOndisi yang digunakan untuk menangani perputaran objek. Untuk perputaran, harus diketahui, dari titik mana objek akan dirotasi, dari kiri bawah, kanan bawah, kiri atas atau kanan atas.
			Setiap perbesaran dari titik yang berbeda, akan menghasilkan langkah2 rotasi yang berbeda pula
		*/
		else if (flagTool[4])
		{
			if(tcurrent != NULL) tcurrent->EvaluatingResize((float)x, (float)(600 - y + 30));
		}
		// Kondisi yang digunakan untuk menggambar lingkaran
		else if (flagTool[5])
		{
			if (t != NULL)
			{
				t->Selected = false;
				if (Objs.size() == 0 || Objs[Objs.size() - 1] != t)
				Objs.push_back(t);
				t = new Round(r, g, b);
			}
		}
		// Kondisi yang digunakan untuk menggambar kotak
		else if (flagTool[6])
		{
			if (t != NULL)
			{
				t->Selected = false;
				if (Objs.size() == 0 || Objs[Objs.size() - 1] != t)
				Objs.push_back(t);
				t = new Square(r, g, b);
			}
		}
		// Kondisi yang digunakan untuk menggambar segi banyak, dengan jumlah segi asli = 3, dan maksimal 10
		else if (flagTool[7])
		{
			if (t != NULL)
			{
				t->Selected = false;
				if (Objs.size() == 0 || Objs[Objs.size() - 1] != t)
				Objs.push_back(t);
				t = new Polyside(r, g, b);
			}
		}
		// Kondisi yang digunakan untuk menggambar garis bebas
		else if (flagTool[8])
		{
			if (t != NULL)
			{
				t->Selected = false;
				if (Objs.size() == 0 || Objs[Objs.size() - 1] != t)
				Objs.push_back(t);
				t = new Freehand(r, g, b);
			}
		}
		// Kondisi yang digunakan untuk melakukan region filling daerah di domain piksel
		else if (flagTool[9])
		{
			float t[3];
			t[0] = r; t[1] = g; t[2] = b;
			workspace->fillAt(x, y - 30, t);
		}
		// Kondisi yang digunakan untuk melakukan penghapusan daerah di domain piksel
		else if (flagTool[10]) workspace->eraseAt(x, 600 - y + 30, toolSize);
		// Kondisi yang digunakan untuk melakukan penggambaran brush di domain piksel
		else if (flagTool[11])
		{
			float t[3];
			t[0] = r; t[1] = g; t[2] = b;
			for (int i = 0; i < 2 * toolSize; i++)
				workspace->setPixelAt(rand() % toolSize + x - toolSize / 2, rand() % toolSize + 600 - y + 30 - toolSize / 2, t);
		}
	}
	// Kondisi ketika klik mouse dilepas
	else if (state == GLUT_UP && button == GLUT_LEFT_BUTTON)
	{
		flagTool[0] = false;
		if (flagTool[1]) { t->EvaluatingFinish(); }
	}
	glutPostRedisplay();
}

/*
	Fungsi yang menangani rendering window utama setiap kali user men-drag mouse
*/
void MouseMoveFunc(int x, int y)
{
	if (flagTool[0])
	{
		/*
			Kondisi yang berasosiasi dengan penggambaran kurva, setiap klik di curve tool, menghasilkan dua titik, dan ketika user
			mendrag mouse, titik terakhir yang ditambahkan akan diubah sesuai dengan posisi dimana user melepas tombol mouse. Titik yang diubah adalah control point dalam
			suatu segmen garis pada de casteljau's algorithm
		*/
		if (flagTool[1])
		{
			t->EditLastPoint(x, 600 - y + 30);
			glutPostRedisplay();
		}
		// Kondisi yang menangani perpindahan objek
		else if (flagTool[2])
		{
			if (tcurrent != NULL)
			{
				tcurrent->Translate(x - Xinit, 600 - y + 30 - Yinit);
				Xinit = x;
				Yinit = 600 - y + 30;
				glutPostRedisplay();
			}
		}
		// Kondisi yang menangani pengubahan ukuran objek
		else if (flagTool[3])
		{
			if (tcurrent != NULL && tcurrent->isResizing())
			{
				tcurrent->Resize((float)x, (float)(600 - y + 30));
				glutPostRedisplay();
			}
		}
		// Kondisi yang menangani perputaran objek
		else if (flagTool[4])
		{
			if (tcurrent != NULL)
			{
				if (tcurrent->isResizing())
				{
					y = 600 - y + 30;
					float direction = 0, angle;
					float dx1 = Xinit - tcurrent->GetCenterX();
					float dx2 = x - tcurrent->GetCenterX();
					float dy1 = Yinit - tcurrent->GetCenterY();
					float dy2 = y - tcurrent->GetCenterY();
					direction = (dx1 * dy2 > dy1 * dx2) ? 1 : -1;
					direction = (dx1 * dy2 == dy1 * dx2) ? 0 : direction;
					float a = (x - tcurrent->GetCenterX()) * (x - tcurrent->GetCenterX()) + (y - tcurrent->GetCenterY()) * (y - tcurrent->GetCenterY());
					float b = (Xinit - tcurrent->GetCenterX()) * (Xinit - tcurrent->GetCenterX()) + (Yinit - tcurrent->GetCenterY()) * (Yinit - tcurrent->GetCenterY());
					float c = (Xinit - x) * (Xinit - x) + (Yinit - y) * (Yinit - y);
					angle = acos((a + b - c) / (2 * sqrt(a * b))) * 180 / 3.14159265;
					tcurrent->Rotate(direction * angle);
					Xinit = x;
					Yinit = y;
					glutPostRedisplay();
				}
			}
		}
		// KOndisi yang berasosiasi dengan penggambaran lingkaran
		else if (flagTool[5])
		{
			if (Xinit != x && 600 - y + 30 != Yinit)
				t->SetInitialPoint(Xinit, Yinit, x, 600 - y + 30);
			glutPostRedisplay();
		}
		// Kondisi yang berasosiasi dengan penggambaran kotak
		else if (flagTool[6])
		{
			if (Xinit != x && 600 - y + 30 != Yinit)
				t->SetInitialPoint(Xinit, Yinit, x, 600 - y + 30);
			glutPostRedisplay();
		}
		// Kondisi yang berasosiasi dengan penggambaran segi banyak
		else if (flagTool[7])
		{
			if (Xinit != x && 600 - y + 30 != Yinit)
				t->SetInitialPoint(Xinit, Yinit, x, 600 - y + 30);
			glutPostRedisplay();
		}
		// Kondisi yang berasosiasi dengan freehand tool, setiap perpindahan mouse akan ditambahkan titik baru pada objek freehand
		else if (flagTool[8])
		{
			t->AddPoint(x, 600 - y + 30);
			glutPostRedisplay();
		}
		// KOndisi yang berasosiasi dengan penghapusan objek pada domain piksel
		else if (flagTool[10])
		{
			workspace->eraseAt(x, 600 - y + 30, toolSize);
			glutPostRedisplay();
		}
		// Kondisi yang berasosiasi dengan penggambaran kuas pada domain piksel
		else if (flagTool[11])
		{
			float t[3];
			t[0] = r; t[1] = g; t[2] = b;
			for (int i = 0; i < 2 * toolSize; i++)
				workspace->setPixelAt(rand() % toolSize + x - toolSize / 2, rand() % toolSize + 600 - y + 30 - toolSize / 2, t);
			glutPostRedisplay();
		}
	}
}

// Fungsi keyboard yang mengatur tentang jalan pintas perpindahan tool satu ke tool yang lain
void KeyboardFunc(unsigned char key, int x, int y)
{
	// Untuk mengkatifkan curve tool
	if (key == 'C')
	{
		resetFlag(1);
		glutPostRedisplay();
		Ytool = 3;
		Xtool = 0;
		posTool = Ytool + Xtool * 6;
		if (t != NULL)
		{
			t->Selected = false;
			if (Objs.size() == 0 || Objs[Objs.size() - 1] != t)
			Objs.push_back(t);
		}
		t = new Curve(r, g, b);
	}
	// Untuk menambahkan gambar berformat .bmp ke dalam window utama
	else if (key == 'L')
	{
		char fileName[100];
		scanf("%[^\n]", fileName);
		getchar();
		if (t != NULL && (Objs.size() == 0 || Objs[Objs.size() - 1] != t)) Objs.push_back(t);
		t = new BitmapImg(fileName);
		Objs.push_back(t);
		tcurrent = Objs[Objs.size() - 1];
		t = NULL;
		glutPostRedisplay();
	}
	// Untuk mengkatifkan tool select & move
	else if (key == 's')
	{
		resetFlag(2);
		Ytool = 5;
		Xtool = 0;
		posTool = Ytool + Xtool * 6;
		if (t != NULL)
		{
			t->Selected = false;
			if (Objs.size() == 0 || Objs[Objs.size() - 1] != t)
			Objs.push_back(t);
		}
		glutPostRedisplay();
	}
	// Untuk mengaktifkan resize tool
	else if (key == 'z') // Resize tool
	{
		resetFlag(3);
		Ytool = 0;
		Xtool = 0;
		posTool = Ytool + Xtool * 6;
		if (t != NULL) Objs.push_back(t);
		if(tcurrent != NULL) tcurrent->Selected = true;
		glutPostRedisplay();
	}
	// Untuk mengkatifkan rotate tool
	else if (key == 'r')
	{
		resetFlag(4);
		Ytool = 7;
		Xtool = 1;
		posTool = Ytool + Xtool * 6;
		if (t != NULL) Objs.push_back(t);
		if(tcurrent != NULL) tcurrent->Selected = true;
		glutPostRedisplay();
	}
	// Untuk menambah ketebalan garis
	else if (key == '+')
	{
		if(tcurrent != NULL) tcurrent->IncreasePoint();
		else if (t != NULL) t->IncreasePoint();
		glutPostRedisplay();
	}
	// Untuk mengurangi ketebalan garis
	else if (key == '-')
	{
		if(tcurrent != NULL) tcurrent->DecreasePoint();
		else if (t != NULL) t->DecreasePoint();
		glutPostRedisplay();
	}
	// Untuk mengatifkan tool gambar lingkaran
	else if (key == 'X')
	{
		resetFlag(5);
		Ytool = 11;
		Xtool = 0;
		posTool = Ytool + Xtool * 6;
		if (t != NULL)
		{
			t->Selected = false;
			Objs.push_back(t);
		}
		t = new Round(r, g, b);
		glutPostRedisplay();
	}
	// Untuk mengaktifkan tool gambar kotak
	else if (key == 'S')
	{
		resetFlag(6);
		Ytool = 4;
		Xtool = 0;
		posTool = Ytool + Xtool * 6;
		if (t != NULL)
		{
			t->Selected = false;
			Objs.push_back(t);
		}
		t = new Square(r, g, b);
		glutPostRedisplay();
	}
	// Untuk mengaktifkan tool gambar sisi banyak
	else if (key == 'G')
	{
		resetFlag(7);
		Ytool = 2;
		Xtool = 0;
		posTool = Ytool + Xtool * 6;
		if (t != NULL)
		{
			t->Selected = false;
			Objs.push_back(t);
		}
		t = new Polyside(r, g, b);
		glutPostRedisplay();
	}
	// Untuk mengaktifkan tool gambar tangan bebas
	else if (key == 'F')
	{
		resetFlag(8);
		Ytool = 10;
		Xtool = 1;
		posTool = Ytool + Xtool * 6;
		if (t != NULL)
		{
			t->Selected = false;
			Objs.push_back(t);
		}
		t = new Freehand(r, g, b);
		glutPostRedisplay();
	}
	// Untuk mengaktifkan tool region filling
	else if (key == 'f')
	{
		resetFlag(9);
		Ytool = 8;
		Xtool = 1;
		posTool = Ytool + Xtool * 6;
		if (t != NULL) Objs.push_back(t);
		if(tcurrent != NULL) tcurrent->Selected = true;
		glutPostRedisplay();
	}
	// Untuk mengaktifkan tool hapus
	else if (key == 'e')
	{
		resetFlag(10);
		if (t != NULL) Objs.push_back(t);
		if(tcurrent != NULL) tcurrent->Selected = true;
		glutPostRedisplay();
	}
	// Untuk mengaktifkan brush tool
	else if (key == 'b')
	{
		resetFlag(11);
		Ytool = 9;
		Xtool = 1;
		posTool = Ytool + Xtool * 6;
		if (t != NULL) Objs.push_back(t);
		if(tcurrent != NULL) tcurrent->Selected = true;
		glutPostRedisplay();
	}
	// Untuk rasterisasi objek gambar, sehingga objek gambar akan berada di domain piksel
	else if (key == 'q')
	{
		if(tcurrent != NULL)
		{
			tcurrent->Rasterize(workspace);
		}
		else if (t != NULL)
		{
			t->Rasterize(workspace);
			t = NULL;
		}
		glutPostRedisplay();
	}
	// Untuk menambah jumlah sisi dari objek polyside
	else if (key == '>')
	{
		if(tcurrent != NULL) tcurrent->IncreaseSide();
		else if (t != NULL) t->IncreaseSide();
		glutPostRedisplay();
	}
	// Untuk mengurangi jumlah sisi dari objek polyside
	else if (key == '<')
	{
		if(tcurrent != NULL) tcurrent->DecreaseSide();
		else if (t != NULL) t->DecreaseSide();
		glutPostRedisplay();
	}
	// Untuk memperbesar ukuran dari brush dan erase tool
	else if (key == '[')
	{
		if (toolSize < 80) toolSize += 10;
	}
	// Untuk mengecilkan ukuran dari brush dan erase tool
	else if (key == ']')
	{
		if (toolSize > 10) toolSize -= 10;
	}
	// Untuk menghapus objek yang dipilih
	else if (key == ' ')
	{
		if (Objs.size() > 0)
		{
			Objs.erase(Objs.begin() + tcurrentIdx);
			tcurrent = NULL;
			t = NULL;
			tcurrentIdx = -1;
			glutPostRedisplay();
		}
	}
	// Untuk keluar dari program dan menyimpan gambar ke dalam format .bmp
	else if (key == 'W')
	{
		for (int i = 0; i < Objs.size(); i++)
			Objs[i]->Rasterize(workspace);
		workspace->SaveWork("FinalResult.bmp");
		exit(0);
	}
}

int main(int argc, char **argv)
{
	resetFlag(-1);
	glutInit(&argc, argv);
	tp = new BitmapImg("toolbox.bmp");

	// Displaying toolbox
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA);
	glutInitWindowPosition(0, 0);
	glutInitWindowSize(100, 300);
	glutCreateWindow("Toolbox::OpaintGL - Ongisnade 1.1");
	init_toolbox();
	glutDisplayFunc(drawToolBox);
	glutMouseFunc(toolBoxMouse);

	// Displaying Color Picker
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA);
	glutInitWindowPosition(100 + W, 80);
	glutInitWindowSize(290, 276);
	glutCreateWindow("Color Picker::OpaintGL - Ongisnade 1.1");
	init_ColorPicker();
	glutDisplayFunc(ColorPicker);
	glutMouseFunc(MouseFunc_ColorPicker);

	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA);
	glutInitWindowPosition(80, 80);
	glutInitWindowSize(W, H + 30);

	float aa[3] = {1,1,1};
	// mengatur objek kanvas (domain piksel) menjadi berwarna putih
	workspace = new Canvas(H, W);
	for (int i = 0; i < W; i++)
		for (int j = 0; j < H; j++)
			workspace->setPixelAt(i, j, aa);
	workspace->Modified = true;
	glutCreateWindow("Canvas::OpaintGL - Ongisnade 1.1");
	init(W, H);
	glutDisplayFunc(DisplayFunc);
	glutMouseFunc(MouseFunc);
	glutMotionFunc(MouseMoveFunc);
	glutKeyboardFunc(KeyboardFunc);

	glutMainLoop();
}
