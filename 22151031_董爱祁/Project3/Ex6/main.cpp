#include <stdlib.h>
#include<stdio.h>
#include<Windows.h>
#include<string>
#include<math.h>
#include<iostream>
#include<vector>
#include<fstream>
#include "glut.h"
#include"gl/glext.h"
#define PI 3.1415926
#define BITMAP_ID 0x4D42	//��������
#define TEXW 8
#define TEXH 8
#define BUFSIZE 512
#define SIZE 1
using namespace std;

enum MOVESTATE
{
	OBJECT,
	WORLD
};

MOVESTATE mState = OBJECT;

GLuint wHeight = 0;
GLuint wWidth = 0;
GLfloat angle[2] = {0, 90};
GLfloat dis = 8.0f;
GLfloat whRatio = 1;
GLfloat moveSpeed = 0.2;
GLfloat rSpeed = 1.0f;
GLfloat zSpeed = 0.1;

float eye[] = { 0, 0, 8 };
float center[] = { 0, 0, 0 };

GLuint dl = 0;
GLuint texture[SIZE];
vector<vector<GLfloat>>vSets;//��ŵ㣨x,y,z��������
vector<vector<GLfloat>>vtSets;//���ģ�͵�����(x,y)������
vector<vector<GLfloat>>vnSets;//���ģ�͵ķ���(x,y,z)������
vector<vector<vector<GLuint>>>fSets;//�������ĸ� (��������/��������/��������)

//�ƹ��Ƿ��
bool flight = false;

unsigned char *LoadBitmapFile(char *filename, BITMAPINFOHEADER *bitmapInfoHeader)
{
	FILE *filePtr;	// �ļ�ָ��
	BITMAPFILEHEADER bitmapFileHeader;	// bitmap�ļ�ͷ
	unsigned char	*bitmapImage;		// bitmapͼ������
	int	imageIdx = 0;		// ͼ��λ������
	unsigned char	tempRGB;	// ��������

	// �ԡ�������+����ģʽ���ļ�filename 
	filePtr = fopen(filename, "rb");
	if (filePtr == NULL) return NULL;
	// ����bitmap�ļ�ͼ
	fread(&bitmapFileHeader, sizeof(BITMAPFILEHEADER), 1, filePtr);
	// ��֤�Ƿ�Ϊbitmap�ļ�
	if (bitmapFileHeader.bfType != BITMAP_ID) {
		fprintf(stderr, "Error in LoadBitmapFile: the file is not a bitmap file\n");
		return NULL;
	}

	// ����bitmap��Ϣͷ
	fread(bitmapInfoHeader, sizeof(BITMAPINFOHEADER), 1, filePtr);
	// ���ļ�ָ������bitmap����
	fseek(filePtr, bitmapFileHeader.bfOffBits, SEEK_SET);
	// Ϊװ��ͼ�����ݴ����㹻���ڴ�
	(bitmapImage = new unsigned char[bitmapInfoHeader->biSizeImage]);

	// ��֤�ڴ��Ƿ񴴽��ɹ�
	if (!bitmapImage) {
		fprintf(stderr, "Error in LoadBitmapFile: memory error\n");
		return NULL;
	}

	// ����bitmapͼ������
	fread(bitmapImage, 1, bitmapInfoHeader->biSizeImage, filePtr);
	// ȷ�϶���ɹ�
	if (bitmapImage == NULL) {
		fprintf(stderr, "Error in LoadBitmapFile: memory error\n");
		return NULL;
	}

	//����bitmap�б���ĸ�ʽ��BGR�����潻��R��B��ֵ���õ�RGB��ʽ
	for (imageIdx = 0;
		imageIdx < bitmapInfoHeader->biSizeImage; imageIdx += 3) {
		tempRGB = bitmapImage[imageIdx];
		bitmapImage[imageIdx] = bitmapImage[imageIdx + 2];
		bitmapImage[imageIdx + 2] = tempRGB;
	}
	// �ر�bitmapͼ���ļ�
	fclose(filePtr);
	return bitmapImage;
}

//��������ĺ���
void texload(int i, char *filename)
{
	BITMAPINFOHEADER bitmapInfoHeader;                                 // bitmap��Ϣͷ
	unsigned char*   bitmapData;                                       // ��������

	bitmapData = LoadBitmapFile(filename, &bitmapInfoHeader);
	glBindTexture(GL_TEXTURE_2D, texture[i]);
	// ָ����ǰ����ķŴ�/��С���˷�ʽ
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);//�Ŵ�ʱ��������˷�ʽ
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);//��Сʱ��������˷�ʽ

	glTexImage2D(GL_TEXTURE_2D,
		0, 	    //mipmap���(ͨ��Ϊ����ʾ���ϲ�) 
		GL_RGB,	//����ϣ���������к졢�̡�������
		bitmapInfoHeader.biWidth, //��������������n�����б߿�+2 
		bitmapInfoHeader.biHeight, //����߶ȣ�������n�����б߿�+2 
		0, //�߿�(0=�ޱ߿�, 1=�б߿�) 
		GL_RGB,	//bitmap���ݵĸ�ʽ
		GL_UNSIGNED_BYTE, //ÿ����ɫ���ݵ�����
		bitmapData);	//bitmap����ָ��
}
//��������ĺ���
void initTexture()
{
	glGenTextures(SIZE, texture);
	texload(0, "TexturePig.bmp");
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1); //�������ش洢ģʽ��������ȡ��ͼ�����ݵ��ж��뷽ʽ.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);//�Ŵ����
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);//��С����
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);//s�����ظ�
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);//t�����ظ�
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);//t�����ظ�
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
}

void LoadObj(string filename)
{
	ifstream file(filename);
	string strText, temp;
	int start, end;
	long offset = 0;
	while (!file.eof())
	{
		getline(file, strText);
		if (strText.substr(0, 2) == "vt")
		{
			vector<GLfloat> vTextures;
			GLfloat x, y;
			start = 3;
			end = strText.find(' ', start);
			temp = strText.substr(start, end - start);
			vTextures.push_back(atof(temp.c_str()));
			start = end + 1;
			end = strText.length();
			temp = strText.substr(start, end - start);
			vTextures.push_back(atof(temp.c_str()));
			vtSets.push_back(vTextures);
		}//������������
		else if (strText.substr(0, 2) == "vn") 
		{
			vector<GLfloat> Normal;
			start = 3;
			end = strText.find(' ', start);
			temp = strText.substr(start, end - start);
			Normal.push_back(atof(temp.c_str()));
			start = end + 1;
			end = strText.find(' ', start);
			temp = strText.substr(start, end - start);
			Normal.push_back(atof(temp.c_str()));
			start = end + 1;
			end = strText.length();
			temp = strText.substr(start, end - start);
			Normal.push_back(atof(temp.c_str()));
			vnSets.push_back(Normal);
		}//��������
		else if (strText.substr(0, 1) == "v")//��������
		{
			vector<GLfloat> Point;
			start = 2;
			end = strText.find(' ', start);
			temp = strText.substr(start, end - start);
		    Point.push_back(atof(temp.c_str()));
			start = end + 1;
			end = strText.find(' ', start);
			temp = strText.substr(start, end - start);
			Point.push_back(atof(temp.c_str()));
			start = end + 1;
			end = strText.length();
			temp = strText.substr(start, end - start);
			Point.push_back(atof(temp.c_str()));
			vSets.push_back(Point);
		}
		else if (strText.substr(0, 1) == "f")
		{
			vector<vector<GLuint>> vIndexSets;
			vector<GLuint> vIndex, vtIndex, vnIndex;
			for (start = 2; start < strText.length(); )
			{
				end = strText.find('/', start);
				temp = strText.substr(start, end - start);
				vIndex.push_back(stoi(temp)-1);
				start = end + 1;
				end = strText.find('/', start);
				temp = strText.substr(start, end - start);
				vtIndex.push_back(stoi(temp)-1);
				start = end + 1;
				end = strText.find(' ', start);
				if (end == -1)
					end = strText.length();
				temp = strText.substr(start, end - start);
				vnIndex.push_back(stoi(temp)-1);
				start = end + 1;
			}
			vIndexSets.push_back(vIndex);
			vIndexSets.push_back(vtIndex);
			vIndexSets.push_back(vnIndex);
			fSets.push_back(vIndexSets);
		}
		else if (strText.substr(0, 1) == "#") {}
		else {}
	}
	file.close();
}

void DrawObj() // This function draws a triangle with RGB colors
{
	GLfloat white[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, white);
	glMaterialfv(GL_FRONT, GL_SPECULAR, white);
	glMateriali(GL_FRONT, GL_SHININESS, 128);
	glPushMatrix();
	for (int i = 0; i < fSets.size(); i++)
	{
		glBegin(GL_POLYGON);
		for (int j = 0; j < fSets[i][0].size(); j++)
		{
			glTexCoord2f(vtSets[fSets[i][1][j]][0], vtSets[fSets[i][1][j]][1]);
			glNormal3f(vnSets[fSets[i][2][j]][0], vnSets[fSets[i][2][j]][1], vnSets[fSets[i][2][j]][2]);//���Ʒ�����
			glVertex3f(vSets[fSets[i][0][j]][0], vSets[fSets[i][0][j]][1], vSets[fSets[i][0][j]][2]);//���ƶ���
		}
		glEnd();
	}
	glPopMatrix();
}

void DrawScene()
{
	//glActiveTextureARB(GL_TEXTURE0_ARB);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture[0]);
	DrawObj();
	//glActiveTextureARB(GL_TEXTURE0_ARB);
	glDisable(GL_TEXTURE_2D);
}

void updateView(int width, int height)
{
	glViewport(0,0,width,height);						// Reset The Current Viewport

	glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
	glLoadIdentity();									// Reset The Projection Matrix

	whRatio = (GLfloat)width/(GLfloat)height;
	gluPerspective(45.0f, whRatio,0.1f,100.0f);

	glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
}

void reshape(int width, int height)
{
	if (height==0)										// Prevent A Divide By Zero By
	{
		height=1;										// Making Height Equal One
	}

	wHeight = height;
	wWidth = width;

	updateView(wHeight, wWidth);
}

void idle()
{
	glutPostRedisplay();
}

void key(unsigned char k, int x, int y)
{
	switch (k)
	{
	case 27:
	case 'q': {exit(0); break; }						//�˳�
	case 'd':
		if (mState == WORLD)
		{
			eye[0] += moveSpeed;
			center[0] += moveSpeed;
		}
		else
			angle[0] += rSpeed;
		break;
		//����ת��
	case 'a':
		if (mState == WORLD)
		{
			eye[0] -= moveSpeed;
			center[0] -= moveSpeed;
		}
		else 
			angle[0] -= rSpeed;
		break;
	case 's':
		if (mState == WORLD)
		{
			eye[1] -= moveSpeed;
			center[1] -= moveSpeed;
		}
		else
			angle[1] -= rSpeed;
		break;
	case 'w':
		if (mState == WORLD)
		{
			eye[1] += moveSpeed;
			center[1] += moveSpeed;
		}
		else
			angle[1] += rSpeed;
		break;
	case 'z':
		if (mState == WORLD)
		{
			eye[2] -= moveSpeed;
			center[2] -= moveSpeed;
		}
		else dis -= zSpeed;
		break;
	case 'c':
		if (mState == WORLD)
		{
			eye[2] += moveSpeed;
			center[2] += moveSpeed;
		}
		else dis += zSpeed;
		break;
	case '1':
		if (mState == OBJECT)
			mState = WORLD;
		else
			mState = OBJECT;
		break;
	case '2':
		if (!flight)
		{
			glEnable(GL_LIGHT1);
			flight = true;
		}
		else
		{
			flight = false;
			glDisable(GL_LIGHT1);
		}
	case 'F':
		mState = OBJECT;
		for (int i = 0; i < 3; i++) center[i] = 0;
		eye[0] = dis * sin(angle[1] * PI / 180.0f)*sin(angle[0] * PI / 180.0f);
		eye[1] = dis * sin(angle[1] * PI / 180.0f)*cos(angle[0] * PI / 180.0f);
		eye[2] = dis * cos(angle[1]);
		break;
	}
	if (mState == OBJECT)
	{
		eye[0] = center[0] + dis * sin(angle[1] * PI / 180.0f)*sin(angle[0] * PI / 180.0f);
		eye[2] = center[2] + dis * sin(angle[1] * PI / 180.0f)*cos(angle[0] * PI / 180.0f);
		eye[1] = center[1] + dis * cos(angle[1] * PI / 180.0f);
	}
}

void redraw()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();									// Reset The Current Modelview Matrix

	gluLookAt(eye[0], eye[1], eye[2],
		center[0], center[1], center[2],
		0, 1, 0);				// ������0��0��0�����ӵ����� (0,5,50)��Y������

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	DrawScene();						// Draw triangle
	glutSwapBuffers();
}

void initObj()
{
	LoadObj("pig.obj");
}

void SetupRC()
{
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_CCW);
	glEnable(GL_LIGHTING);

	GLfloat white[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat spotLight[] = { 1.0, 0.0, 0.0, 1.0 };
	GLfloat sourceLight[] = { 0.5, 0.5, 0.5, 1 };
	GLfloat light_pos[] = { 5, 5, 5, 1 };
	GLfloat spot_pos[] = { 0,0,4,1 };

	//���ù���ģʽ��ȱʡ�Ļ�������
	//glLightModelfv(GL_LIGHT_MODEL_AMBIENT, sourceLight);
	//���û�����
	glLightfv(GL_LIGHT0, GL_AMBIENT, sourceLight);
	//����ɢ���
	glLightfv(GL_LIGHT0, GL_DIFFUSE, sourceLight);
	//���þ����
	glLightfv(GL_LIGHT0, GL_SPECULAR, white);
	glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
	glEnable(GL_LIGHT0);

	//���þ۹��
	glLightfv(GL_LIGHT1, GL_DIFFUSE, spotLight);
	glLightfv(GL_LIGHT1, GL_SPECULAR, spotLight);
	glLightfv(GL_LIGHT1, GL_POSITION, spot_pos);
	glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, 60.0f);
	//glEnable(GL_LIGHT1);

	//������ɫ׷��
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
	glClearColor(0, 0, 0, 1);
}

int main (int argc,  char *argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
	glutInitWindowSize(480,480);
	int windowHandle = glutCreateWindow("Project3");

	glutDisplayFunc(redraw);//ע����ƻص�����
	glutReshapeFunc(reshape);//ע���ػ�ص�����	
	glutKeyboardFunc(key);//ע�ᰴ���ص�����
	glutIdleFunc(idle);//ע��ȫ�ֻص�����������ʱ����
	initTexture();
	initObj();
	SetupRC();
	glutMainLoop();
	return 0;
}


