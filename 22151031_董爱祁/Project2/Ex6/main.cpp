#include <stdlib.h>
#include<stdio.h>
#include<Windows.h>
#include<string>
#include<math.h>
#include<iostream>
#include "glut.h"
#include"gl/glext.h"
#define PI 3.1415926
#define BITMAP_ID 0x4D42	//��������
#define TEXW 8
#define TEXH 8
#define BUFSIZE 512
#define SIZE 4
using namespace std;

GLuint wHeight = 0;
GLuint wWidth = 0;
GLfloat angle = 0.0f;
GLfloat dis = 8.0f;
GLfloat whRatio = 1;
GLuint texture[SIZE];
GLuint selectBuf[BUFSIZE];

float eye[] = { 0, 0, 8 };
float center[] = { 0, 0, 0 };
GLint selectIndex = -1;
char* names[] = { "Teaport1", "Teaport2", "Teaport3", "Teaport4" };

GLuint dl = 0;

// �����ʾ�����飬������������ı�ʾ��
// ����: ͨ��ָ�룬����filename ָ����bitmap�ļ������ݡ�
// ͬʱҲ����bitmap��Ϣͷ.����֧��-bitλͼ��
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
	glShadeModel(GL_SMOOTH);
	glGenTextures(SIZE, texture);
	for (int i = 0; i < SIZE; i++)
		texload(i, (char*)(to_string(i+1)+".bmp").data());

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1); //�������ش洢ģʽ��������ȡ��ͼ�����ݵ��ж��뷽ʽ.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);//�Ŵ����
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);//��С����
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);//s�����ظ�
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);//t�����ظ�
}

void drawstring(const char* str) {

	static int isfirstcall = 1;
	static GLuint lists;
	if (isfirstcall) {
		isfirstcall = 0;
		lists = glGenLists(128);
		wglUseFontBitmaps(wglGetCurrentDC(), 0, 128, lists);
	}
	for (; *str != '\0'; ++str) {
		glCallList(lists + *str);
	}
}

void DrawScene() // This function draws a triangle with RGB colors
{
	glInitNames();
	glPushName(0);
	for (int i = 0; i < SIZE; i++)
	{
		glLoadName(i);
		glActiveTextureARB(GL_TEXTURE0_ARB);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, texture[i]);
		glPushMatrix();
		glTranslatef(-2+i*1.4f, 0, 0);
		glScalef(0.4f, 0.4f, 0.4f);
		glutSolidTeapot(1);
		glPopMatrix();
		glActiveTextureARB(GL_TEXTURE0_ARB);
		glDisable(GL_TEXTURE_2D);
	}
	glPopName();
	glColor3f(0, 1, 0);
	glRasterPos2f(-0.5f, 2);
	char* txt = "";
	if (selectIndex >= 0) txt = names[selectIndex];
	drawstring(txt);
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
	//����ת�����ӵ�λ�ú��ӵ㷽������ת��
	case 'd':
		angle += 1.0f;
		break;
		//����ת��
	case 'a':
		angle -= 1.0f;
		break;
	case 's':
		center[1] -= 0.1f;
		eye[1] -= 0.1f;
		if (eye[1] <= -1.8f)
			eye[1] = -1.8f;
		if (center[1] <= -1.8f)
			center[1] = -1.8f;
		break;
	case 'w':
		center[1] += 0.1f;
		eye[1] += 0.1f;
		if (eye[1] >= 3.0f)
			eye[1] = 3.0f;
		if (center[1] >= 3.0f)
			center[1] = 3.0f;
		break;
	case 'z':
		dis -= 0.1f;
		break;
	case 'c':
		dis += 0.1f;
		break;
	}
	eye[0] = dis * sin(angle*PI / 180.0f);
	eye[2] = dis * cos(angle*PI / 180.0f);
}


void redraw()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();									// Reset The Current Modelview Matrix

	gluLookAt(eye[0], eye[1], eye[2],
		center[0], center[1], center[2],
		0, 1, 0);				// ������0��0��0�����ӵ����� (0,5,50)��Y������

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);

	GLfloat white[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat light_pos[] = {5, 5, 5, 1};

	glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
	glLightfv(GL_LIGHT0, GL_AMBIENT, white);
	glEnable(GL_LIGHT0);
	
	DrawScene();						// Draw triangle
	glutSwapBuffers();
}

void mouse(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON)//���������
	{
		if (state == GLUT_DOWN)//���ڰ���״̬
		{
			GLint eyepoint[4];//���ڴ洢�ӽ�
			glSelectBuffer(BUFSIZE, selectBuf);//����ѡ�񻺳�����hit record�ᱻ�洢������
			glRenderMode(GL_SELECT);//��ʽ��ʼ����ѡ��

			glMatrixMode(GL_PROJECTION);//����ͶӰ����
			glPushMatrix();//������ջ
			glLoadIdentity();//��ʼ��Ϊ��λ����

			glGetIntegerv(GL_VIEWPORT, eyepoint);//����ӽ�
			//����ѡ��Χ��С(1*1)����Ϊ��������ϵ������������ϵ������Ҫת������������
			gluPickMatrix((GLdouble)x, (GLdouble)(eyepoint[3] - y), 3, 3, eyepoint);

			gluPerspective(45.0f, whRatio, 0.1f, 100.0f);//͸��ͶӰ
			glMatrixMode(GL_MODELVIEW);
			redraw();

			int hits;
			glMatrixMode(GL_PROJECTION);
			glPopMatrix();
			glMatrixMode(GL_MODELVIEW);
			glFlush();

			hits = glRenderMode(GL_RENDER);//������Ⱦģʽ��ѡ������ĸ���
			if (hits >= 1)//�������ĸ������ڵ���1��Ҫ������Ȳ��ԣ�ѡ�������
			{
				int minchoose = selectBuf[3];//���������id
				int mindepth = selectBuf[1];//������������
				for (int i = 0; i < hits; i++)//�������е�ѡ������
				{
					if (selectBuf[i * 4 + 1] < (GLuint)mindepth)
					{
						minchoose = selectBuf[i * 4 + 3];
						mindepth = selectBuf[i * 4 + 1];
					}
				}
				selectIndex = minchoose;
				//cout << "hits��" << hits << " choose:" << selectIndex<<endl;
			}
		}
	}
}

int main (int argc,  char *argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
	glutInitWindowSize(480,480);
	int windowHandle = glutCreateWindow("Project2");

	glutDisplayFunc(redraw);//ע����ƻص�����
	glutReshapeFunc(reshape);//ע���ػ�ص�����	
	glutKeyboardFunc(key);//ע�ᰴ���ص�����
	glutMouseFunc(mouse);
	glutIdleFunc(idle);//ע��ȫ�ֻص�����������ʱ����
	initTexture();
	glutMainLoop();
	return 0;
}


