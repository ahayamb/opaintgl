#include <windows.h>
#include <string.h>
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
//Komen baru
// baru lagi

vector<dot> Dots;

int W = 800, H = 600, toolSize = 20;
Obj *t, *tcurrent;
vector<Obj*> Objs;
Canvas *workspace;
bool flagTool[20];
int Xinit, Yinit;

void resetFlag(int idx)
{
	memset(flagTool, false, sizeof(flagTool));
	if (idx >= 0) flagTool[idx] = true;
}
void init(int W, int H)
{
	glClearColor(1, 1, 1, 1);
	gluOrtho2D(0, W, 0, H);
}

void DisplayFunc()
{
	glClear(GL_COLOR_BUFFER_BIT);

	if (workspace != NULL) workspace->DrawToScreen();

	for (int i = 0; i < Objs.size(); i++)
		if (Objs[i] != NULL) Objs[i]->Draw();
	if (t != NULL) t->Draw();

	glFlush();
}

void MouseFunc(int button, int state, int x, int y)
{
	if (state == GLUT_DOWN && button == GLUT_LEFT_BUTTON)
	{
		flagTool[0] = true;
		Xinit = x;
		Yinit = (600 - y);
		if (flagTool[1])
		{
			if (t->isFinished())
			{
				Objs.push_back(t);
				t = new Curve();
			}
			t->AddPoint((float)x, (float)(600 - y));
			t->AddPoint((float)Xinit, (float)Yinit);
			t->Evaluate();
		}
		else if (flagTool[2])
		{
			for (int i = 0; i < Objs.size(); i++)
			{
				Objs[i]->CheckSelect((float)x, (float)(600 - y));
				if (Objs[i]->Selected)
				{
					tcurrent = Objs[i];
					for (int j = i + 1; j < Objs.size(); j++) Objs[j]->Selected = false;
					break;
				}
				tcurrent = NULL;
			}
			if (tcurrent != NULL) tcurrent->Selected = true;
		}
		else if (flagTool[3])
		{
			if(tcurrent != NULL) tcurrent->EvaluatingResize((float)x, (float)(600 - y));
		}
		else if (flagTool[4])
		{
			if(tcurrent != NULL) tcurrent->EvaluatingResize((float)x, (float)(600 - y));
		}
		else if (flagTool[5])
		{
			if (t != NULL)
			{
				t->Selected = false;
				Objs.push_back(t);
				t = new Round();
			}
		}
		else if (flagTool[6])
		{
			if (t != NULL)
			{
				t->Selected = false;
				Objs.push_back(t);
				t = new Square();
			}
		}
		else if (flagTool[7])
		{
			if (t != NULL)
			{
				t->Selected = false;
				Objs.push_back(t);
				t = new Polyside();
			}
		}
		else if (flagTool[8])
		{
			if (t != NULL)
			{
				t->Selected = false;
				Objs.push_back(t);
				t = new Freehand();
			}
		}
		else if (flagTool[9])
		{
			float t[3] = {0.92,0.12,0.12};
			workspace->fillAt(x, y, t);
		}
		else if (flagTool[10]) workspace->eraseAt(x, 600 - y, toolSize);
		else if (flagTool[11])
		{
			float t[3] = {1, 1, 0};
			for (int i = 0; i < 2 * toolSize; i++)
				workspace->setPixelAt(rand() % toolSize + x - toolSize / 2, rand() % toolSize + 600 - y - toolSize / 2, t);
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
			t->EditLastPoint(x, 600 - y);
			glutPostRedisplay();
		}
		else if (flagTool[2])
		{
			if (tcurrent != NULL)
			{
				tcurrent->Translate(x - Xinit, 600 - y - Yinit);
				Xinit = x;
				Yinit = 600 - y;
				glutPostRedisplay();
			}
		}
		else if (flagTool[3])
		{
			if (tcurrent != NULL && tcurrent->isResizing())
			{
				tcurrent->Resize((float)x, (float)(600 - y));
				glutPostRedisplay();
			}
		}
		else if (flagTool[4])
		{
			if (tcurrent != NULL)
			{
				if (tcurrent->isResizing())
				{
					y = 600 - y;
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
			if (Xinit != x && 600 - y != Yinit)
				t->SetInitialPoint(Xinit, Yinit, x, 600 - y);
			glutPostRedisplay();
		}
		else if (flagTool[6])
		{
			if (Xinit != x && 600 - y != Yinit)
				t->SetInitialPoint(Xinit, Yinit, x, 600 - y);
			glutPostRedisplay();
		}
		else if (flagTool[7])
		{
			if (Xinit != x && 600 - y != Yinit)
				t->SetInitialPoint(Xinit, Yinit, x, 600 - y);
			glutPostRedisplay();
		}
		else if (flagTool[8])
		{
			t->AddPoint(x, 600 - y);
			glutPostRedisplay();
		}
		else if (flagTool[10])
		{
			workspace->eraseAt(x, 600 - y, toolSize);
			glutPostRedisplay();
		}
		else if (flagTool[11])
		{
			float t[3] = {1, 1, 0};
			for (int i = 0; i < 2 * toolSize; i++)
				workspace->setPixelAt(rand() % toolSize + x - toolSize / 2, rand() % toolSize + 600 - y - toolSize / 2, t);
			glutPostRedisplay();
		}
	}
}

void KeyboardFunc(unsigned char key, int x, int y)
{
	if (key == 'C')
	{
		resetFlag(1);
		if (t != NULL)
		{
			t->Selected = false;
			Objs.push_back(t);
		}
		t = new Curve();
	}
	else if (key == 's')
	{
		resetFlag(2);
		if (t != NULL)
		{
			t->Selected = false;
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
		t = new Round();
	}
	else if (key == 'S')
	{
		resetFlag(6);
		if (t != NULL)
		{
			t->Selected = false;
			Objs.push_back(t);
		}
		t = new Square();
	}
	else if (key == 'G')
	{
		resetFlag(7);
		if (t != NULL)
		{
			t->Selected = false;
			Objs.push_back(t);
		}
		t = new Polyside();
	}
	else if (key == 'F')
	{
		resetFlag(8);
		if (t != NULL)
		{
			t->Selected = false;
			Objs.push_back(t);
		}
		t = new Freehand();
	}
	else if (key == 'f')
	{
		resetFlag(9);
		if (t != NULL) Objs.push_back(t);
		if(tcurrent != NULL) tcurrent->Selected = true;
	}
	else if (key == 'e')
	{
		resetFlag(10);
		if (t != NULL) Objs.push_back(t);
		if(tcurrent != NULL) tcurrent->Selected = true;
	}
	else if (key == 'b')
	{
		resetFlag(11);
		if (t != NULL) Objs.push_back(t);
		if(tcurrent != NULL) tcurrent->Selected = true;
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
}

int main(int argc, char **argv)
{
	resetFlag(-1);
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA);
	glutInitWindowPosition(80, 80);
	glutInitWindowSize(W, H);
	float t[3] = {1,1,1};
	workspace = new Canvas(H, W);
	for (int i = 0; i < W; i++)
		for (int j = 0; j < H; j++)
			workspace->setPixelAt(i, j, t);
	workspace->Modified = true;
	glutCreateWindow("OpaintGL::Ongisnade 1.0");
	init(W, H);
	glutDisplayFunc(DisplayFunc);
	glutMouseFunc(MouseFunc);
	glutMotionFunc(MouseMoveFunc);
	glutKeyboardFunc(KeyboardFunc);
	glutMainLoop();
}

