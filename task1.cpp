#include<GL/glew.h>
#include<GLFW/glfw3.h>
#include<iostream>

using namespace std;

void init(GLFWwindow*& window){}

void display(GLFWwindow* window,double currentTime) 
{
	glClearColor(1, 0, 0, 1);//设置背景色为红色
	glClear(GL_COLOR_BUFFER_BIT);//清除颜色缓存
}

int main()
{
	if(!glfwInit())
	{
		cerr<<"GLFW init failed"<<endl;
		return -1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	GLFWwindow* window = glfwCreateWindow(800, 600, "task1", NULL, NULL);
	glfwMakeContextCurrent(window);

	if(glewInit()!=GLEW_OK)
	{
		cerr<<"GLEW init failed"<<endl;
		return -1;
	}
	glfwSwapInterval(1);//开启垂直同步
	init(window);

	while(!glfwWindowShouldClose(window))
	{
		double currentTime = glfwGetTime();
		display(window,currentTime);
		glfwSwapBuffers(window);//交换前后缓存
		glfwPollEvents();
	}
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}