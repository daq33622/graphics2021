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
#define BITMAP_ID 0x4D42	//读纹理函数
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
vector<vector<GLfloat>>vSets;//存放点（x,y,z）的坐标
vector<vector<GLfloat>>vtSets;//存放模型的纹理(x,y)的坐标
vector<vector<GLfloat>>vnSets;//存放模型的法线(x,y,z)的坐标
vector<vector<vector<GLuint>>>fSets;//存放面的四个 (顶点索引/纹理索引/法线索引)

//灯光是否打开
bool flight = false;

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
	glGenTextures(SIZE, texture);
	texload(0, "TexturePig.bmp");
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1); //设置像素存储模式控制所读取的图像数据的行对齐方式.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);//放大过滤
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);//缩小过滤
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);//s方向重复
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);//t方向重复
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);//t方向重复
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
		}//顶点纹理坐标
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
		}//顶点向量
		else if (strText.substr(0, 1) == "v")//顶点坐标
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
			glNormal3f(vnSets[fSets[i][2][j]][0], vnSets[fSets[i][2][j]][1], vnSets[fSets[i][2][j]][2]);//绘制法向量
			glVertex3f(vSets[fSets[i][0][j]][0], vSets[fSets[i][0][j]][1], vSets[fSets[i][0][j]][2]);//绘制顶点
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
	case 'q': {exit(0); break; }						//退出
	case 'd':
		if (mState == WORLD)
		{
			eye[0] += moveSpeed;
			center[0] += moveSpeed;
		}
		else
			angle[0] += rSpeed;
		break;
		//向左转动
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
		0, 1, 0);				// 场景（0，0，0）的视点中心 (0,5,50)，Y轴向上

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

	//设置光照模式：缺省的环境照明
	//glLightModelfv(GL_LIGHT_MODEL_AMBIENT, sourceLight);
	//设置环境光
	glLightfv(GL_LIGHT0, GL_AMBIENT, sourceLight);
	//设置散射光
	glLightfv(GL_LIGHT0, GL_DIFFUSE, sourceLight);
	//设置镜面光
	glLightfv(GL_LIGHT0, GL_SPECULAR, white);
	glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
	glEnable(GL_LIGHT0);

	//设置聚光灯
	glLightfv(GL_LIGHT1, GL_DIFFUSE, spotLight);
	glLightfv(GL_LIGHT1, GL_SPECULAR, spotLight);
	glLightfv(GL_LIGHT1, GL_POSITION, spot_pos);
	glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, 60.0f);
	//glEnable(GL_LIGHT1);

	//启用颜色追踪
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

	glutDisplayFunc(redraw);//注册绘制回调函数
	glutReshapeFunc(reshape);//注册重绘回调函数	
	glutKeyboardFunc(key);//注册按键回调函数
	glutIdleFunc(idle);//注册全局回调函数，空闲时调用
	initTexture();
	initObj();
	SetupRC();
	glutMainLoop();
	return 0;
}


