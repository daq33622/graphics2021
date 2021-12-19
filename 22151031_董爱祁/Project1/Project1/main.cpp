#include<GL/glut.h>
#include<cmath>
using namespace std;
#define PI 3.1415926

float eye[] = { 0, 0, 8 };								//�۲��λ��
float center[] = { 0, 0, 0 };							//�ӵ㷽��
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
	glutSolidTeapot(0.8f);							//���Ʋ��
	glPopMatrix();
}
#pragma endregion

void updateView(int width, int height)
{
	glViewport(0, 0, width, height);						// �����Ӵ���С�����øú����趨�˽�ȡ��ͼ�ν��������ı�����ʾ��Ĭ��ԭ���ڴ�С

	glMatrixMode(GL_PROJECTION);						// ���þ���ģʽΪͶӰ
	glLoadIdentity();									// ��ʼ������Ϊ��λ����

	float whRatio = (GLfloat)width / (GLfloat)height;	//������ʾ����
	gluPerspective(45, whRatio, 1, 100);	//͸��ͶӰ���ӽǣ���߱ȣ�������Զ����)
	glMatrixMode(GL_MODELVIEW);							// ���þ���ģʽΪģ��
}

void reshape(int width, int height)
{
	if (height == 0)										// Prevent A Divide By Zero By
	{
		height = 1;										// Making Height Equal One
	}
	height = width = (height<width)?height:width;				//���ű��ֱ���һ��

	updateView(height, width);						//�����ӽ�
}

void idle()
{
	glutPostRedisplay();								//���õ�ǰ���ƺ���
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
	eye[0] = dis * sin(angle*PI/180.0f);
	eye[2] = dis * cos(angle*PI/180.0f);
	
}


void redraw()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();									// Reset The Current Modelview Matrix

	gluLookAt(eye[0], eye[1], eye[2],
		center[0], center[1], center[2],
		0, 1, 0);				// ������0��0��0�����ӵ����� (0,5,50)��Y������(ÿһ�д������λ�ã��ӵ�λ�ã����Ϸ���)

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);	//���ö���λ���ģʽ�������棬���

	glEnable(GL_DEPTH_TEST);	//������Ȳ���
	glEnable(GL_LIGHTING);		//��������ģʽ
	GLfloat white[] = { 1.0, 1.0, 1.0, 1.0 };	//������ɫ
	GLfloat light_pos[] = { 5,5,5,1 };			//�����Դλ�ã����һ������һ����0��1����0ʱ�����Դ���볡��Ϊƽ�й⣬��1ʱΪ���Դ

	glLightfv(GL_LIGHT0, GL_POSITION, light_pos);//����0�Ź�Դ����λ��
	glLightfv(GL_LIGHT0, GL_AMBIENT, white);	//����0�Ź�Դ������ɫ
	glEnable(GL_LIGHT0);						//����0�Ź�Դ

	drawScene();// Draw Scene

	glutSwapBuffers();						//����������
}

int main(int argc, char *argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
	glutInitWindowSize(480, 480);
	int windowHandle = glutCreateWindow("Program 1");

	glutDisplayFunc(redraw);
	glutReshapeFunc(reshape);		//ע���ػ�ص��������ڴ��ڴ�С�ı��Լ���ʼ������ʱ����
	glutKeyboardFunc(key);			//ע�ᰴ���ص��������������ڳ�������ʱִ�ж��ǵȴ��û������ĳЩ��������ͨ������ָ����ú���
	glutIdleFunc(idle);

	glutMainLoop();
	return 0;
}
