// OpenGL related includes
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// cpp related includes
#include <iostream>

// global variables
const unsigned short windowWidth = 1920;
const unsigned short windowHeight = 1080;

// function prototypes
void framebuffer_size_callback(GLFWwindow*, int width, int height);
void processInput(GLFWwindow* window);

int main(void) {

	// initialize glfw itself and set the version and core profile
	glfwInit(); 
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// create the glfw window and check for failure
	GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, "minecraftpp", NULL, NULL);
	if (window == NULL) {
		std::cerr << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	// set the newly created window to be used
	glfwMakeContextCurrent(window);

	// initialize glad and check for failure
	if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cerr << "Failed to initialize GLAD" << std::endl;
		return -1;
	}
	
	// set the viewport to be the entire screen size
	glViewport(0, 0, windowWidth, windowHeight);

	// tell glfw to call the correct function upon window resizing
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	while(!glfwWindowShouldClose(window)) {
		// poll for user input each frame
		processInput(window);

		// set the background color
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// cleanup
	glfwTerminate();
	return 0;
}

// handle window resizing
void framebuffer_size_callback(GLFWwindow*, int width, int height) {
	glViewport(0, 0, width, height);
}

// handle user input
void processInput(GLFWwindow* window) {
	if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, true);
}
