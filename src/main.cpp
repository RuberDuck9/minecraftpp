// OpenGL related includes
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// custom class includes
#include "shader.h"

// cpp related includes
#include <iostream>

// global variables
const unsigned short windowWidth = 1920;
const unsigned short windowHeight = 1080;

// function prototypes
void framebuffer_size_callback(GLFWwindow*, int width, int height);
void processInput(GLFWwindow* window);

uint8_t vertices[] = {
	 0, 0, 0,
	 200, 0, 0,
	 0, 200, 0,
	 200, 200, 0,
};

uint8_t indices[] = {
	0, 1, 2,
	2, 3, 1
};

int main(void) {

	// initialize glfw itself and set the version and core profile
	glfwInit(); 
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// create the glfw window and check for failure
	GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, "minecraftpp", NULL, NULL);
	if (window == NULL) {
		std::cerr << "ERROR:GLFW:UNABLE_TO_CREATE_WINDOW" << std::endl;
		glfwTerminate();
		return -1;
	}

	// set the newly created window to be used
	glfwMakeContextCurrent(window);

	// initialize glad and check for failure
	if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cerr << "ERROR:GLAD:UNABLE_TO_INITIALIZE_GLAD" << std::endl;
		return -1;
	}
	
	// set the viewport to be the entire screen size
	glViewport(0, 0, windowWidth, windowHeight);

	// tell glfw to call the correct function upon window resizing
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	// generate each buffer object
	unsigned int VAO, VBO, EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	// bind each buffer object
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

	// buffer the vertex and index data
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// tell opengl how to interpret the buffered data
	glVertexAttribPointer(0, 3, GL_UNSIGNED_BYTE, GL_TRUE, 3 * sizeof(uint8_t), (void*)0);

	// enable the configuered vertex attrib array (in this case 0 for pos)
	glEnableVertexAttribArray(0);

	// create and enable the shader object
	Shader customShader("./shaders/vertex.glsl", "./shaders/fragment.glsl");
	customShader.use();

	while(!glfwWindowShouldClose(window)) {
		// poll for user input each frame
		processInput(window);

		// set the background color
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// draw the vertices
		glDrawElements(GL_TRIANGLES, sizeof(indices), GL_UNSIGNED_BYTE, 0);

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
