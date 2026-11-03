#include <glm/ext/matrix_clip_space.hpp>
#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include "camera.h"
#include "shader.h"
#include "chunk.h"

// global variables
const unsigned short windowWidth = 1920;
const unsigned short windowHeight = 1080;

float deltaTime = 0.0f;
float lastTime = 0.0f;

Camera camera(glm::vec3(0.0f, 0.0f, 0.0f));
float lastX = windowHeight / 2.0f;
float lastY = windowWidth / 2.0f;
bool mouseInitialized = false;

// function prototypes
void framebuffer_size_callback(GLFWwindow*, int width, int height);
void processInput(GLFWwindow *window);
void mouse_callback(GLFWwindow *, double xpos, double ypos);

int main(void) {

  // initialize glfw itself and set the version and core profile
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  // create the glfw window and check for failure
  GLFWwindow *window =
      glfwCreateWindow(windowWidth, windowHeight, "minecraftpp", NULL, NULL);
  if (window == NULL) {
    std::cerr << "ERROR:GLFW:UNABLE_TO_CREATE_WINDOW" << std::endl;
    glfwTerminate();
    return -1;
  }

  // set the newly created window to be used
  glfwMakeContextCurrent(window);

  // initialize glad and check for failure
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cerr << "ERROR:GLAD:UNABLE_TO_INITIALIZE_GLAD" << std::endl;
    return -1;
  }

  // set the viewport to be the entire screen size
  glViewport(0, 0, windowWidth, windowHeight);

  // tell glfw to call the correct function upon window resizing
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  glfwSetCursorPosCallback(window, mouse_callback);

  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  // make sure things render front to back
  glEnable(GL_DEPTH_TEST);

  // create and enable the shader 
  Shader customShader("./shaders/vertex.glsl", "./shaders/fragment.glsl");
  customShader.use();

  // load the atlas map
  int width, height, nrChannels;
  unsigned char *data = stbi_load("./assets/alpha-atlas.png", &width, &height, &nrChannels, 0);

  // declare the texture
  unsigned int texture;
  glGenTextures(1, &texture);

  // check if the map loaded, if it did generate the texture
  if (data) {
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    //glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);
  } else {
    std::cerr << "ERROR:TEXTURE:FAILED_TO_GENERATE_TEXTURE" << std::endl;
  }

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
 
  // bind the texture
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texture);

  Chunk please1({0, 0, 0}, customShader);
  Chunk please2({0, 1, 0}, customShader);
  Chunk please3({0, 2, 0}, customShader);

  please1.initializeChunk();
  please2.initializeChunk();
  please3.initializeChunk();
    
  please1.generateChunkState();
  please2.generateChunkState();
  please3.generateChunkState();

  please1.generateChunkMesh();
  please2.generateChunkMesh();
  please3.generateChunkMesh();

  please1.bufferChunkData();
  please2.bufferChunkData();
  please3.bufferChunkData();

  while (!glfwWindowShouldClose(window)) {
    // poll for user input each frame
    processInput(window);

    float currentTime = glfwGetTime();
    deltaTime = currentTime - lastTime;
    lastTime = currentTime;

    // set the background color
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 view = camera.getViewMatrix();
    customShader.setMat4("view", view);

    glm::mat4 projection = glm::perspective(glm::radians(camera.fov), (float)windowWidth / windowHeight, 0.1f, 1024.0f);
    customShader.setMat4("projection", projection);

    please1.renderChunkMesh();
    please2.renderChunkMesh();
    please3.renderChunkMesh();

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  // cleanup
  glfwTerminate();
  return 0;
}

// handle window resizing
void framebuffer_size_callback(GLFWwindow *, int width, int height) {
  glViewport(0, 0, width, height);
}

// handle user input
void processInput(GLFWwindow *window) {

  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);

  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    camera.processKeyboard(FORWARD, deltaTime);

  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    camera.processKeyboard(BACKWARD, deltaTime);

  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    camera.processKeyboard(LEFT, deltaTime);

  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    camera.processKeyboard(RIGHT, deltaTime);

  if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    camera.processKeyboard(UP, deltaTime);

  if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
    camera.processKeyboard(DOWN, deltaTime);

  if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
    camera.movementSpeed = SPEED * 5;
  } else {
    camera.movementSpeed = SPEED;
  }

  if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) {
    camera.fov = 25.0f;
  } else {
    camera.fov = 100.0f;
  }

  if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS) {
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  }
  if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS) {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  }

}

// check for mouse movement and use the difference between current position to rotation the view matrix
void mouse_callback(GLFWwindow *, double xpos, double ypos) {

  if (!mouseInitialized) {
    lastX = xpos;
    lastY = ypos;
    mouseInitialized = true;
  }

  float xoffset = xpos - lastX;
  float yoffset = lastY - ypos;

  lastX = xpos;
  lastY = ypos;

  camera.processMouse(xoffset, yoffset);

}
