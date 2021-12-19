#include <stdlib.h>
#include<stdio.h>
#include<Windows.h>
#include<string>
#include<math.h>
#include<iostream>
#include "glut.h"
#include"gl/glext.h"
#define PI 3.1415926
#define BITMAP_ID 0x4D42	//读纹理函数
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

// 纹理标示符数组，保存两个纹理的标示符
// 描述: 通过指针，返回filename 指定的bitmap文件中数据。
// 同时也返回bitmap信息头.（不支持-bit位图）
unsigned char *LoadBitmapFile(char *filename, BITMAPINFOHEADER *bitmapInfoHeader)
{
	FILE *filePtr;	// 文件指针
	BITMAPFILEHEADER bitmapFileHeader;	// bitmap文件头
	unsigned char	*bitmapImage;		// bitmap图像数据
	int	imageIdx = 0;		// 图像位置索引
	unsigned char	tempRGB;	// 交换变量

	// 以“二进制+读”模式打开文件filename 
	filePtr = fopen(filename, "rb");
	if (filePtr == NULL) return NULL;
	// 读入bitmap文件图
	fread(&bitmapFileHeader, sizeof(BITMAPFILEHEADER), 1, filePtr);
	// 验证是否为bitmap文件
	if (bitmapFileHeader.bfType != BITMAP_ID) {
		fprintf(stderr, "Error in LoadBitmapFile: the file is not a bitmap file\n");
		return NULL;
	}

	// 读入bitmap信息头
	fread(bitmapInfoHeader, sizeof(BITMAPINFOHEADER), 1, filePtr);
	// 将文件指针移至bitmap数据
	fseek(filePtr, bitmapFileHeader.bfOffBits, SEEK_SET);
	// 为装载图像数据创建足够的内存
	(bitmapImage = new unsigned char[bitmapInfoHeader->biSizeImage]);

	// 验证内存是否创建成功
	if (!bitmapImage) {
		fprintf(stderr, "Error in LoadBitmapFile: memory error\n");
		return NULL;
	}

	// 读入bitmap图像数据
	fread(bitmapImage, 1, bitmapInfoHeader->biSizeImage, filePtr);
	// 确认读入成功
	if (bitmapImage == NULL) {
		fprintf(stderr, "Error in LoadBitmapFile: memory error\n");
		return NULL;
	}

	//由于bitmap中保存的格式是BGR，下面交换R和B的值，得到RGB格式
	for (imageIdx = 0;
		imageIdx < bitmapInfoHeader->biSizeImage; imageIdx += 3) {
		tempRGB = bitmapImage[imageIdx];
		bitmapImage[imageIdx] = bitmapImage[imageIdx + 2];
		bitmapImage[imageIdx + 2] = tempRGB;
	}
	// 关闭bitmap图像文件
	fclose(filePtr);
	return bitmapImage;
}

//加载纹理的函数
void texload(int i, char *filename)
{

	BITMAPINFOHEADER bitmapInfoHeader;                                 // bitmap信息头
	unsigned char*   bitmapData;                                       // 纹理数据

	bitmapData = LoadBitmapFile(filename, &bitmapInfoHeader);
	glBindTexture(GL_TEXTURE_2D, texture[i]);
	// 指定当前纹理的放大/缩小过滤方式
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);//放大时的纹理过滤方式
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);//缩小时的纹理过滤方式

	glTexImage2D(GL_TEXTURE_2D,
		0, 	    //mipmap层次(通常为，表示最上层) 
		GL_RGB,	//我们希望该纹理有红、绿、蓝数据
		bitmapInfoHeader.biWidth, //纹理宽带，必须是n，若有边框+2 
		bitmapInfoHeader.biHeight, //纹理高度，必须是n，若有边框+2 
		0, //边框(0=无边框, 1=有边框) 
		GL_RGB,	//bitmap数据的格式
		GL_UNSIGNED_BYTE, //每个颜色数据的类型
		bitmapData);	//bitmap数据指针
}
//定义纹理的函数
void initTexture()
{
	glShadeModel(GL_SMOOTH);
	glGenTextures(SIZE, texture);
	for (int i = 0; i < SIZE; i++)
		texload(i, (char*)(to_string(i+1)+".bmp").data());

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1); //设置像素存储模式控制所读取的图像数据的行对齐方式.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);//放大过滤
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);//缩小过滤
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);//s方向重复
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);//t方向重复
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
	case 'q': {exit(0); break; }						//退出
	//向右转动，视点位置和视点方向向右转动
	case 'd':
		angle += 1.0f;
		break;
		//向左转动
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
		0, 1, 0);				// 场景（0，0，0）的视点中心 (0,5,50)，Y轴向上

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
	if (button == GLUT_LEFT_BUTTON)//鼠标左键点击
	{
		if (state == GLUT_DOWN)//处于按下状态
		{
			GLint eyepoint[4];//用于存储视角
			glSelectBuffer(BUFSIZE, selectBuf);//设置选择缓冲区，hit record会被存储在里面
			glRenderMode(GL_SELECT);//正式开始进行选择

			glMatrixMode(GL_PROJECTION);//保存投影矩阵
			glPushMatrix();//矩阵入栈
			glLoadIdentity();//初始化为单位矩阵

			glGetIntegerv(GL_VIEWPORT, eyepoint);//获得视角
			//设置选择范围大小(1*1)，因为鼠标的坐标系不是世界坐标系，所以要转化成世界坐标
			gluPickMatrix((GLdouble)x, (GLdouble)(eyepoint[3] - y), 3, 3, eyepoint);

			gluPerspective(45.0f, whRatio, 0.1f, 100.0f);//透视投影
			glMatrixMode(GL_MODELVIEW);
			redraw();

			int hits;
			glMatrixMode(GL_PROJECTION);
			glPopMatrix();
			glMatrixMode(GL_MODELVIEW);
			glFlush();

			hits = glRenderMode(GL_RENDER);//进入渲染模式，选中物体的个数
			if (hits >= 1)//如果物体的个数大于等于1，要进行深度测试，选择最近的
			{
				int minchoose = selectBuf[3];//放最近物体id
				int mindepth = selectBuf[1];//放最近物体距离
				for (int i = 0; i < hits; i++)//遍历所有的选中物体
				{
					if (selectBuf[i * 4 + 1] < (GLuint)mindepth)
					{
						minchoose = selectBuf[i * 4 + 3];
						mindepth = selectBuf[i * 4 + 1];
					}
				}
				selectIndex = minchoose;
				//cout << "hits：" << hits << " choose:" << selectIndex<<endl;
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

	glutDisplayFunc(redraw);//注册绘制回调函数
	glutReshapeFunc(reshape);//注册重绘回调函数	
	glutKeyboardFunc(key);//注册按键回调函数
	glutMouseFunc(mouse);
	glutIdleFunc(idle);//注册全局回调函数，空闲时调用
	initTexture();
	glutMainLoop();
	return 0;
}


