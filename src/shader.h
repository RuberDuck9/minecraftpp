#ifndef SHADER_H
#define SHADER_H

#include <string>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Shader {
public:
  // constuctor to open, compile, and link the vertex and fragment shaders
  Shader(const char *vertexPath, const char *fragmentPath);

  // set shader to be used by OpenGL
  void use();

  // set a boolean uniform
  void setBool(const std::string &name, const bool &value) const;
  // set an integer uniform
  void setInt(const std::string &name, const int &value) const;
  // set a float uniform
  void setFloat(const std::string &name, const float &value) const;
  // set a 4x4 matrix uniform
  void setMat4(const std::string &name, const glm::mat4 &value) const;

private:
  // store unique shader ID
  unsigned int ID;
};

#endif
