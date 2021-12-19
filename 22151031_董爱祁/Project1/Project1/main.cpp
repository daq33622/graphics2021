#include<GL/glut.h>
#include<cmath>
using namespace std;
#define PI 3.1415926

float eye[] = { 0, 0, 8 };								//观察点位置
float center[] = { 0, 0, 0 };							//视点方向
float angle = 0;
float dis = 8.0f;

#pragma region drawScene

void drawScene()
{
	glPushMatrix();
	glTranslatef(-2.0f, 0.0f, 0.0f);
	glutSolidCube(1.0f);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0.0f, 0.0f, 0.0f);
	glutSolidSphere(0.8f, 36, 12);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(2.0f, 0.0f, 0.0f);
	glutSolidTeapot(0.8f);							//绘制茶壶
	glPopMatrix();
}
#pragma endregion

void updateView(int width, int height)
{
	glViewport(0, 0, width, height);						// 设置视窗大小，调用该函数设定了截取的图形将以怎样的比例显示，默认原窗口大小

	glMatrixMode(GL_PROJECTION);						// 设置矩阵模式为投影
	glLoadIdentity();									// 初始化矩阵为单位矩阵

	float whRatio = (GLfloat)width / (GLfloat)height;	//设置显示比例
	gluPerspective(45, whRatio, 1, 100);	//透视投影（视角，宽高比，近处，远处）)
	glMatrixMode(GL_MODELVIEW);							// 设置矩阵模式为模型
}

void reshape(int width, int height)
{
	if (height == 0)										// Prevent A Divide By Zero By
	{
		height = 1;										// Making Height Equal One
	}
	height = width = (height<width)?height:width;				//缩放保持比例一致

	updateView(height, width);						//更新视角
}

void idle()
{
	glutPostRedisplay();								//调用当前绘制函数
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
	eye[0] = dis * sin(angle*PI/180.0f);
	eye[2] = dis * cos(angle*PI/180.0f);
	
}


void redraw()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();									// Reset The Current Modelview Matrix

	gluLookAt(eye[0], eye[1], eye[2],
		center[0], center[1], center[2],
		0, 1, 0);				// 场景（0，0，0）的视点中心 (0,5,50)，Y轴向上(每一行代表相机位置，视点位置，向上方向)

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);	//设置多边形绘制模式：正反面，填充

	glEnable(GL_DEPTH_TEST);	//开启深度测试
	glEnable(GL_LIGHTING);		//开启光照模式
	GLfloat white[] = { 1.0, 1.0, 1.0, 1.0 };	//定义颜色
	GLfloat light_pos[] = { 5,5,5,1 };			//定义光源位置，最后一个参数一般是0或1，是0时代表光源距离场景为平行光，是1时为点光源

	glLightfv(GL_LIGHT0, GL_POSITION, light_pos);//设置0号光源光照位置
	glLightfv(GL_LIGHT0, GL_AMBIENT, white);	//设置0号光源光照颜色
	glEnable(GL_LIGHT0);						//开启0号光源

	drawScene();// Draw Scene

	glutSwapBuffers();						//交换缓冲区
}

int main(int argc, char *argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
	glutInitWindowSize(480, 480);
	int windowHandle = glutCreateWindow("Program 1");

	glutDisplayFunc(redraw);
	glutReshapeFunc(reshape);		//注册重绘回调函数，在窗口大小改变以及初始化窗口时调用
	glutKeyboardFunc(key);			//注册按键回调函数，它不是在程序启动时执行而是等待用户完成了某些操作后，再通过函数指针调用函数
	glutIdleFunc(idle);

	glutMainLoop();
	return 0;
}
