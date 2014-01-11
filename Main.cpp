#include <windows.h>
#include "Objlib.h"

/*
0 : isDragging
1 : Curve (C)
2 : Select tool & Move tool (s)
3 : Resize (z)
4 : Rotate (r)
5 : Round tool (X)
6 : Square (S)
7 : Polyside (G)
8 : Freehand (F)
9 : Fill (f)
10 : Erase (e)
11 : Brush (b)
*/

vector<dot> Dots;

int W = 800, H = 600, toolSize = 20, tcurrentIdx = -1;
Obj *t, *tcurrent;
vector<Obj*> Objs;
Canvas *workspace;
bool flagTool[20];
int Xinit, Yinit;
float r = 0, g = 0, b = 0;

//--------------------------------------------------Associated Property/Function of Color Picker-------------------------------
void init_ColorPicker(void)
{
	glClearColor( 1, 1, 1, 1);
	glClearDepth( 1.0 );
	gluOrtho2D(0, 290, -20, 260);
}

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

void resetFlag(int idx)
{
	memset(flagTool, false, sizeof(flagTool));
	if (idx >= 0) flagTool[idx] = true;
}
void init(int W, int H)
{
	glClearColor(1, 1, 1, 1);
	gluOrtho2D(0, W, 0, H + 30);
}

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
		puts("\n");
	}

	glFlush();
}

void MouseFunc(int button, int state, int x, int y)
{
	if (state == GLUT_DOWN && button == GLUT_LEFT_BUTTON)
	{
		flagTool[0] = true;
		Xinit = x;
		Yinit = (600 - y + 30);
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
		else if (flagTool[2])
		{
			for (int i = Objs.size() - 1; i >= 0; i--)
			{
				//printf("%d %d\n", i, Objs[i]->Selected);
				Objs[i]->CheckSelect((float)x, (float)(600 - y + 30));
				//printf("%d %d\n", i, Objs[i]->Selected);
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
		else if (flagTool[3])
		{
			if(tcurrent != NULL) tcurrent->EvaluatingResize((float)x, (float)(600 - y + 30));
		}
		else if (flagTool[4])
		{
			if(tcurrent != NULL) tcurrent->EvaluatingResize((float)x, (float)(600 - y + 30));
		}
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
		else if (flagTool[9])
		{
			float t[3];
			t[0] = r; t[1] = g; t[2] = b;
			workspace->fillAt(x, y - 30, t);
		}
		else if (flagTool[10]) workspace->eraseAt(x, 600 - y + 30, toolSize);
		else if (flagTool[11])
		{
			float t[3];
			t[0] = r; t[1] = g; t[2] = b;
			for (int i = 0; i < 2 * toolSize; i++)
				workspace->setPixelAt(rand() % toolSize + x - toolSize / 2, rand() % toolSize + 600 - y + 30 - toolSize / 2, t);
		}
	}
	else if (state == GLUT_UP && button == GLUT_LEFT_BUTTON)
	{
		flagTool[0] = false;
		if (flagTool[1]) { t->EvaluatingFinish(); }
	}
	glutPostRedisplay();
}

void MouseMoveFunc(int x, int y)
{
	if (flagTool[0])
	{
		if (flagTool[1])
		{
			t->EditLastPoint(x, 600 - y + 30);
			glutPostRedisplay();
		}
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
		else if (flagTool[3])
		{
			if (tcurrent != NULL && tcurrent->isResizing())
			{
				tcurrent->Resize((float)x, (float)(600 - y + 30));
				glutPostRedisplay();
			}
		}
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
		else if (flagTool[5])
		{
			if (Xinit != x && 600 - y + 30 != Yinit)
				t->SetInitialPoint(Xinit, Yinit, x, 600 - y + 30);
			glutPostRedisplay();
		}
		else if (flagTool[6])
		{
			if (Xinit != x && 600 - y + 30 != Yinit)
				t->SetInitialPoint(Xinit, Yinit, x, 600 - y + 30);
			glutPostRedisplay();
		}
		else if (flagTool[7])
		{
			if (Xinit != x && 600 - y + 30 != Yinit)
				t->SetInitialPoint(Xinit, Yinit, x, 600 - y + 30);
			glutPostRedisplay();
		}
		else if (flagTool[8])
		{
			t->AddPoint(x, 600 - y + 30);
			glutPostRedisplay();
		}
		else if (flagTool[10])
		{
			workspace->eraseAt(x, 600 - y + 30, toolSize);
			glutPostRedisplay();
		}
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

void KeyboardFunc(unsigned char key, int x, int y)
{
	if (key == 'C')
	{
		resetFlag(1);
		glutPostRedisplay();
		if (t != NULL)
		{
			t->Selected = false;
			if (Objs.size() == 0 || Objs[Objs.size() - 1] != t)
			Objs.push_back(t);
		}
		t = new Curve(r, g, b);
	}
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
	else if (key == 's')
	{
		resetFlag(2);
		if (t != NULL)
		{
			t->Selected = false;
			if (Objs.size() == 0 || Objs[Objs.size() - 1] != t)
			Objs.push_back(t);
		}
		glutPostRedisplay();
	}
	else if (key == 'z') //Resize tool
	{
		resetFlag(3);
		if (t != NULL) Objs.push_back(t);
		if(tcurrent != NULL) tcurrent->Selected = true;
		glutPostRedisplay();
	}
	else if (key == 'r')
	{
		resetFlag(4);
		if (t != NULL) Objs.push_back(t);
		if(tcurrent != NULL) tcurrent->Selected = true;
		glutPostRedisplay();
	}
	else if (key == '+')
	{
		if(tcurrent != NULL) tcurrent->IncreasePoint();
		else if (t != NULL) t->IncreasePoint();
		glutPostRedisplay();
	}
	else if (key == '-')
	{
		if(tcurrent != NULL) tcurrent->DecreasePoint();
		else if (t != NULL) t->DecreasePoint();
		glutPostRedisplay();
	}
	else if (key == 'X')
	{
		resetFlag(5);
		if (t != NULL)
		{
			t->Selected = false;
			Objs.push_back(t);
		}
		t = new Round(r, g, b);
		glutPostRedisplay();
	}
	else if (key == 'S')
	{
		resetFlag(6);
		if (t != NULL)
		{
			t->Selected = false;
			Objs.push_back(t);
		}
		t = new Square(r, g, b);
		glutPostRedisplay();
	}
	else if (key == 'G')
	{
		resetFlag(7);
		if (t != NULL)
		{
			t->Selected = false;
			Objs.push_back(t);
		}
		t = new Polyside(r, g, b);
		glutPostRedisplay();
	}
	else if (key == 'F')
	{
		resetFlag(8);
		if (t != NULL)
		{
			t->Selected = false;
			Objs.push_back(t);
		}
		t = new Freehand(r, g, b);
		glutPostRedisplay();
	}
	else if (key == 'f')
	{
		resetFlag(9);
		if (t != NULL) Objs.push_back(t);
		if(tcurrent != NULL) tcurrent->Selected = true;
		glutPostRedisplay();
	}
	else if (key == 'e')
	{
		resetFlag(10);
		if (t != NULL) Objs.push_back(t);
		if(tcurrent != NULL) tcurrent->Selected = true;
		glutPostRedisplay();
	}
	else if (key == 'b')
	{
		resetFlag(11);
		if (t != NULL) Objs.push_back(t);
		if(tcurrent != NULL) tcurrent->Selected = true;
		glutPostRedisplay();
	}
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
	else if (key == '>')
	{
		if(tcurrent != NULL) tcurrent->IncreaseSide();
		else if (t != NULL) t->IncreaseSide();
		glutPostRedisplay();
	}
	else if (key == '<')
	{
		if(tcurrent != NULL) tcurrent->DecreaseSide();
		else if (t != NULL) t->DecreaseSide();
		glutPostRedisplay();
	}
	else if (key == '[')
	{
		if (toolSize < 80) toolSize += 10;
	}
	else if (key == ']')
	{
		if (toolSize > 10) toolSize -= 10;
	}
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

	//Displaying Color Picker
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
