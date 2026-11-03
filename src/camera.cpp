#include "camera.h"

// return the view matrix given the camera position vectors
glm::mat4 Camera::getViewMatrix() {
  return glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
}

// update camera position based on keyboard inputs
void Camera::processKeyboard(CameraMovement direction, float deltaTime) {
  float velocity = movementSpeed * deltaTime;

  if (direction == FORWARD)
    cameraPos += cameraFront * velocity;

  if (direction == BACKWARD)
    cameraPos -= cameraFront * velocity;

  if (direction == LEFT)
    cameraPos -= cameraRight * velocity;

  if (direction == RIGHT)
    cameraPos += cameraRight * velocity;

  if (direction == UP)
    cameraPos += cameraUp * velocity;

  if (direction == DOWN)
    cameraPos -= cameraUp * velocity;

  cameraChunkPos.x = (int)(cameraPos.x / 16);
  cameraChunkPos.z = (int)(cameraPos.z / 16);
}

// update camera angles variables based on mouse input and call function to recalculate camera position matrices
void Camera::processMouse(float xoffset, float yoffset) {
  xoffset *= mouseSensitivity;
  yoffset *= mouseSensitivity;

  yaw += xoffset;
  pitch += yoffset;

  if (pitch > 89.0f)
    pitch = 89.0f;
  if (pitch < -89.0f)
    pitch = -89.0f;

  updateCameraVectors();
}

// recalculate camera position matrices based on angle variables
void Camera::updateCameraVectors() {
  glm::vec3 newFront;
  newFront.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
  newFront.y = sin(glm::radians(pitch));
  newFront.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
  cameraFront = glm::normalize(newFront);

  cameraRight = glm::normalize(glm::cross(cameraFront, worldUp));
  cameraUp = glm::normalize(glm::cross(cameraRight, cameraFront));
}
