//------- Ignore this ----------
#include<filesystem>
namespace fs = std::filesystem;
//------------------------------
#define _USE_MATH_DEFINES
#define GL_GLEXT_PROTOTYPES

#include<math.h>
#include"Model.h"
#include<gl/GL.h>
#include<GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


const unsigned int width = 3600;
const unsigned int height = 2000;

float skyboxVertices[] =
{
	-1.0f, -1.0f,  1.0f,
	 1.0f, -1.0f,  1.0f,
	 1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f, -1.0f,
	-1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f
};

unsigned int skyboxIndices[] =
{
	// Prawa
	1, 2, 6,
	6, 5, 1,
	// Lewa
	0, 4, 7,
	7, 3, 0,
	// Górna
	4, 5, 6,
	6, 7, 4,
	// Dolna
	0, 3, 2,
	2, 1, 0,
	// Tylna
	0, 1, 5,
	5, 4, 0,
	// Przednia
	3, 7, 6,
	6, 2, 3
};

int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(width, height, "Rybka wszechswiata", NULL, NULL);

	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	gladLoadGL();
	glViewport(0, 0, width, height);

	Shader shaderProgram("default.vert", "default.frag");
	Shader skyboxShader("skybox.vert", "skybox.frag");
	Shader shaderEarth("earth.vert", "earth.frag");

	glm::vec4 lightColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	glm::vec3 lightPos = glm::vec3(0.0f, 5.0f, 0.0f);

	shaderProgram.Activate();
	glUniform4f(glGetUniformLocation(shaderProgram.ID, "lightColor"), lightColor.x, lightColor.y, lightColor.z, lightColor.w);
	glUniform3f(glGetUniformLocation(shaderProgram.ID, "lightPos"), lightPos.x, lightPos.y, lightPos.z);
	skyboxShader.Activate();
	glUniform1i(glGetUniformLocation(skyboxShader.ID, "skybox"), 0);
	shaderEarth.Activate();
	glUniform4f(glGetUniformLocation(shaderEarth.ID, "lightColor"), lightColor.x, lightColor.y, lightColor.z, lightColor.w);
	glUniform3f(glGetUniformLocation(shaderEarth.ID, "lightPos"), lightPos.x, lightPos.y, lightPos.z);
	

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);
	glFrontFace(GL_CCW);

	std::string parentDir = (fs::current_path().fs::path::parent_path()).string();
	std::string tuiPath = "/Resources/models/tui/scene.gltf";
	std::string earthPath = "/Resources/models/earth/scene.gltf";

	Model tui((parentDir + tuiPath).c_str());
	Model earth((parentDir + earthPath).c_str());

	unsigned int skyboxVAO, skyboxVBO, skyboxEBO;
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glGenBuffers(1, &skyboxEBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, skyboxEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(skyboxIndices), &skyboxIndices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	std::string facesCubemap[6] =
	{
		parentDir + "/Resources/skybox/right.png",
		parentDir + "/Resources/skybox/left.png",
		parentDir + "/Resources/skybox/top.png",
		parentDir + "/Resources/skybox/bottom.png",
		parentDir + "/Resources/skybox/front.png",
		parentDir + "/Resources/skybox/back.png",
	};

	unsigned int cubemapTexture;
	glGenTextures(1, &cubemapTexture);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	for (unsigned int i = 0; i < 6; i++)
	{
		int width, height, nrChannels;
		unsigned char* data = stbi_load(facesCubemap[i].c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			stbi_set_flip_vertically_on_load(false);
			glTexImage2D
			(
				GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0,
				GL_RGB,
				width,
				height,
				0,
				GL_RGB,
				GL_UNSIGNED_BYTE,
				data
			);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Failed to load texture: " << facesCubemap[i] << std::endl;
			stbi_image_free(data);
		}
	}

	float movement = 0.0, radius = 155.0;
	glm::quat rotation_koi;
	glm::quat rotation_earth;

	Camera camera(width, height, glm::vec3(0.0f, 50.0f, 50.0f));

	while (!glfwWindowShouldClose(window))
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		camera.Inputs(window);
		camera.updateMatrix(45.0f, 0.1f, 1000000.0f);

		rotation_koi = glm::angleAxis(movement, glm::vec3(0.0f, 1.0f, 0.0f));
		rotation_earth = glm::angleAxis(movement, glm::vec3(0.0f, -1.0f, 0.0f));

		tui.Draw(shaderProgram, camera, glm::vec3(radius * sin(movement), 0.0f, radius * cos(movement)), glm::vec3(26.0f, 26.0f, 26.0f), rotation_koi);
		
		earth.Draw(shaderEarth, camera, glm::vec3(0.0f, 5.0f, 0.0f), glm::vec3(3.0f, 3.0f, 3.0f), rotation_earth);

		movement += glm::radians(0.5f);

		glDepthFunc(GL_LEQUAL);

		skyboxShader.Activate();
		glm::mat4 view = glm::mat4(1.0f);
		glm::mat4 projection = glm::mat4(1.0f);

		view = glm::mat4(glm::mat3(glm::lookAt(camera.Position, camera.Position + camera.Orientation, camera.Up)));
		projection = glm::perspective(glm::radians(45.0f), (float)width / height, 0.1f, 100.0f);
		glUniformMatrix4fv(glGetUniformLocation(skyboxShader.ID, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(skyboxShader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

		glBindVertexArray(skyboxVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
		glDepthFunc(GL_LESS);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	shaderProgram.Delete();
	skyboxShader.Delete();

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}