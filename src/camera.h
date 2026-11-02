#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

enum Camera_Movement {
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT,
	UP, 
	DOWN
};

const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 10.0f;
const float SENSITIVITY = 0.1f;
const float FOV = 100.0f;

class Camera {
	public:
		// Store camera position
		glm::vec3 cameraPos;
		glm::vec3 cameraChunkPos = glm::vec3(0.0f, 0.0f, 0.0f);
		glm::vec3 cameraFront;
		glm::vec3 worldUp;
		glm::vec3 cameraUp;
		glm::vec3 cameraRight;

		// Store camera angle
		float yaw;
		float pitch;

		// Paramters to configure how fast the camera moves, pans, and the fov of the persective matrix
		float movementSpeed;
		float mouseSensitivity;
		float fov;

		// Constructor
		Camera(glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH) : cameraFront(glm::vec3(0.0f, 0.0f, -1.0f)), movementSpeed(SPEED), mouseSensitivity(SENSITIVITY), fov(FOV) {
			this->cameraPos =  cameraPos;
			this->worldUp = worldUp;
			this->yaw = yaw;
			this->pitch = pitch;
			updateCameraVectors();
		}
		
		// Get the view matrix for the camera based off its position
		glm::mat4 getViewMatrix();

		// update camera position based on keyboard inputs
		void processKeyboard(Camera_Movement direction, float deltaTime);
		
		// update camera angles variables based on mouse input and call function to recalculate camera position matrices
		void processMouse(float xoffset, float yoffset);

	private: 
		// Recalculate camera position matrices based on angle variables
		void updateCameraVectors();
};

#endif
