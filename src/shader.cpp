#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include "shader.h"

// open, compile, and link the vertex and fragment shaders
Shader::Shader(const char *vertexPath, const char *fragmentPath) {
  // istream to pull shader data off of file
  std::ifstream vShaderFile;
  std::ifstream fShaderFile;

  // string to hold extracted shader file data
  std::string vertexCode;
  std::string fragmentCode;

  vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
  fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

  // open and extract data from shader files
  try {
    vShaderFile.open(vertexPath);
    fShaderFile.open(fragmentPath);
    std::stringstream vShaderStream, fShaderStream;

    vShaderStream << vShaderFile.rdbuf();
    fShaderStream << fShaderFile.rdbuf();

    vShaderFile.close();
    fShaderFile.close();

    vertexCode = vShaderStream.str();
    fragmentCode = fShaderStream.str();
  } catch (const std::ifstream::failure &e) {
    std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ" << std::endl;
  }

  // convert c++ string object to c style char array for opengl's use
  const char *vShaderCode = vertexCode.c_str();
  const char *fShaderCode = fragmentCode.c_str();

  // store ID for vertex and fragment shaders
  unsigned int vertex, fragment;

  int success;
  char infoLog[1024];

  // compile the vertex shader and check for failure
  vertex = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertex, 1, &vShaderCode, NULL);
  glCompileShader(vertex);

  glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(vertex, 1024, NULL, infoLog);
    std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n"
              << infoLog << std::endl;
  }

  // comile the fragment shader and check for failure
  fragment = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragment, 1, &fShaderCode, NULL);
  glCompileShader(fragment);

  glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(fragment, 1024, NULL, infoLog);
    std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n"
              << infoLog << std::endl;
  }

  // create the final shader object by combing the vertex and fragment shader,
  // check for failure
  ID = glCreateProgram();
  glAttachShader(ID, vertex);
  glAttachShader(ID, fragment);
  glLinkProgram(ID);

  glGetProgramiv(ID, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(ID, 1024, NULL, infoLog);
    std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
              << infoLog << std::endl;
  }

  // clear the individual vertex and fragment shaders since they are no longer
  // need
  glDeleteShader(vertex);
  glDeleteShader(fragment);
}

// set shader to be used by OpenGL
void Shader::use() { glUseProgram(ID); }

// set a boolean uniform
void Shader::setBool(const std::string &name, const bool &value) const {
  glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
}

// set an integer uniform
void Shader::setInt(const std::string &name, const int &value) const {
  glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}

// set a float uniform
void Shader::setFloat(const std::string &name, const float &value) const {
  glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}

// set a 4x4 matrix uniform
void Shader::setMat4(const std::string &name, const glm::mat4 &value) const {
  glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &value[0][0]);
}
