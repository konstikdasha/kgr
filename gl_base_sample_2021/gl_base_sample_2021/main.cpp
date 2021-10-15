//internal includes
#include "common.h"
#include "ShaderProgram.h"
#include "Camera.h"
#include "std_image.h"
#include <stdio.h>
#include <filesystem>


//External dependencies
#define GLFW_DLL
#include <GLFW/glfw3.h>
#include <random>

static const GLsizei WIDTH = 1024, HEIGHT = 1024; //размеры окна
static int filling = 0;
static bool keys[1024]; //массив состояний кнопок - нажата/не нажата
static GLfloat lastX = 400, lastY = 300; //исходное положение мыши
static bool firstMouse = true;
static bool g_captureMouse = true;  // Мышка захвачена нашим приложением или нет?
static bool g_capturedMouseJustNow = false;
static int g_shaderProgram = 0;


GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

Camera camera(float3(0.0f, 0.0f, 5.0f));

//функция для обработки нажатий на кнопки клавиатуры
void OnKeyboardPressed(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	//std::cout << key << std::endl;
	switch (key)
	{
	case GLFW_KEY_ESCAPE: //на Esc выходим из программы
		if (action == GLFW_PRESS)
			glfwSetWindowShouldClose(window, GL_TRUE);
		break;
	case GLFW_KEY_SPACE: //на пробел переключение в каркасный режим и обратно
		if (action == GLFW_PRESS)
		{
			if (filling == 0)
			{
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				filling = 1;
			}
			else
			{
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				filling = 0;
			}
		}
		break;
	case GLFW_KEY_1:
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		break;
	case GLFW_KEY_Q:
		keys[GLFW_KEY_E] = false;
		keys[GLFW_KEY_R] = false;
		keys[key] = true;
		break;
	case GLFW_KEY_E:
		keys[GLFW_KEY_Q] = false;
		keys[GLFW_KEY_R] = false;
		keys[key] = true;
		break;
	case GLFW_KEY_R:
		keys[GLFW_KEY_Q] = false;
		keys[GLFW_KEY_E] = false;
		keys[key] = true;
		break;
	case GLFW_KEY_2:
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		break;
	default:
		if (action == GLFW_PRESS)
			keys[key] = true;
		else if (action == GLFW_RELEASE)
			keys[key] = false;
	}
}

//функция для обработки клавиш мыши
void OnMouseButtonClicked(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
		g_captureMouse = !g_captureMouse;


	if (g_captureMouse)
	{
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		g_capturedMouseJustNow = true;
	}
	else
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

}

//функция для обработки перемещения мыши
void OnMouseMove(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = float(xpos);
		lastY = float(ypos);
		firstMouse = false;
	}

	GLfloat xoffset = float(xpos) - lastX;
	GLfloat yoffset = lastY - float(ypos);

	lastX = float(xpos);
	lastY = float(ypos);

	if (g_captureMouse)
		camera.ProcessMouseMove(xoffset, yoffset);
}


void OnMouseScroll(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(GLfloat(yoffset));
}

void doCameraMovement(Camera& camera, GLfloat deltaTime)
{
	if (keys[GLFW_KEY_W])
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (keys[GLFW_KEY_A])
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (keys[GLFW_KEY_S])
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (keys[GLFW_KEY_D])
		camera.ProcessKeyboard(RIGHT, deltaTime);

	if (camera.pos.y > 0.46f) {
		camera.pos.y = 0.46f;
	};
	if (camera.pos.y < -1.144f) {
		camera.pos.y = -1.144f;
	};
	if (camera.pos.x < -0.3f) {
		camera.pos.x = -0.3f;
	}
	if (camera.pos.x > 7.2f) {
		camera.pos.x = 7.2f;
	}
	if (camera.pos.z > 5.4f) {
		camera.pos.z = 5.4f;
	}
	if (camera.pos.z < 3.5f) {
		camera.pos.z = 3.5f;
	};
}


GLsizei CreateSphere(float radius, int numberSlices, GLuint& vao)
{
	int i, j;

	int numberParallels = numberSlices;
	int numberVertices = (numberParallels + 1) * (numberSlices + 1);
	int numberIndices = numberParallels * numberSlices * 3;

	float angleStep = (2.0f * 3.14159265358979323846f) / ((float)numberSlices);
	//float helpVector[3] = {0.0f, 1.0f, 0.0f};

	std::vector<float> pos(numberVertices * 4, 0.0f);
	std::vector<float> norm(numberVertices * 4, 0.0f);
	std::vector<float> texcoords(numberVertices * 2, 0.0f);

	std::vector<int> indices(numberIndices, -1);

	for (i = 0; i < numberParallels + 1; i++)
	{
		for (j = 0; j < numberSlices + 1; j++)
		{
			int vertexIndex = (i * (numberSlices + 1) + j) * 4;
			int normalIndex = (i * (numberSlices + 1) + j) * 4;
			int texCoordsIndex = (i * (numberSlices + 1) + j) * 2;

			pos.at(vertexIndex + 0) = radius * sinf(angleStep * (float)i) * sinf(angleStep * (float)j);
			pos.at(vertexIndex + 1) = radius * cosf(angleStep * (float)i);
			pos.at(vertexIndex + 2) = radius * sinf(angleStep * (float)i) * cosf(angleStep * (float)j);
			pos.at(vertexIndex + 3) = 1.0f;

			norm.at(normalIndex + 0) = pos.at(vertexIndex + 0) / radius;
			norm.at(normalIndex + 1) = pos.at(vertexIndex + 1) / radius;
			norm.at(normalIndex + 2) = pos.at(vertexIndex + 2) / radius;
			norm.at(normalIndex + 3) = 1.0f;

			texcoords.at(texCoordsIndex + 0) = (float)j / (float)numberSlices;
			texcoords.at(texCoordsIndex + 1) = (1.0f - (float)i) / (float)(numberParallels - 1);
		}
	}

	int* indexBuf = &indices[0];

	for (i = 0; i < numberParallels; i++)
	{
		for (j = 0; j < numberSlices; j++)
		{
			*indexBuf++ = i * (numberSlices + 1) + j;
			*indexBuf++ = (i + 1) * (numberSlices + 1) + j;
			*indexBuf++ = (i + 1) * (numberSlices + 1) + (j + 1);

			*indexBuf++ = i * (numberSlices + 1) + j;
			*indexBuf++ = (i + 1) * (numberSlices + 1) + (j + 1);
			*indexBuf++ = i * (numberSlices + 1) + (j + 1);

			int diff = int(indexBuf - &indices[0]);
			if (diff >= numberIndices)
				break;
		}
		int diff = int(indexBuf - &indices[0]);
		if (diff >= numberIndices)
			break;
	}

	std::vector<float> tangent, bitangent;

	for (int i = 0; i < pos.size() / 4; i += 1) {
		float3 edge1, edge2;
		float2 deltaUV1, deltaUV2;
		if (i < pos.size() - 3) {
			edge1 = float3(pos.at(i + 4), pos.at(i + 5), pos.at(i + 6)) - float3(pos.at(i), pos.at(i + 1), pos.at(i + 2));
			edge2 = float3(pos.at(i + 8), pos.at(i + 9), pos.at(i + 10)) - float3(pos.at(i), pos.at(i + 1), pos.at(i + 2));
			deltaUV1 = float2(texcoords.at(i + 3), texcoords.at(i + 4)) - float2(texcoords.at(i), texcoords.at(i + 1));
			deltaUV2 = float2(texcoords.at(i + 5), texcoords.at(i + 6)) - float2(texcoords.at(i), texcoords.at(i + 1));
		}
		else if (i == pos.size() - 2) {
			edge1 = float3(pos.at(i + 4), pos.at(i + 5), pos.at(i + 6)) - float3(pos.at(i), pos.at(i + 1), pos.at(i + 2));
			edge2 = float3(pos.at(0), pos.at(1), pos.at(2)) - float3(pos.at(i), pos.at(i + 1), pos.at(i + 2));
			deltaUV1 = float2(texcoords.at(i + 3), texcoords.at(i + 4)) - float2(texcoords.at(i), texcoords.at(i + 1));
			deltaUV2 = float2(texcoords.at(0), texcoords.at(1)) - float2(texcoords.at(i), texcoords.at(i + 1));
		}
		else {
			edge1 = float3(pos.at(0), pos.at(1), pos.at(2)) - float3(pos.at(i), pos.at(i + 1), pos.at(i + 2));
			edge2 = float3(pos.at(3), pos.at(4), pos.at(5)) - float3(pos.at(i), pos.at(i + 1), pos.at(i + 2));
			deltaUV1 = float2(texcoords.at(0), texcoords.at(1)) - float2(texcoords.at(i), texcoords.at(i + 1));
			deltaUV2 = float2(texcoords.at(2), texcoords.at(3)) - float2(texcoords.at(i), texcoords.at(i + 1));
		}
		float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
		float3 tangent1, bitangent1;

		tangent1.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
		tangent1.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
		tangent1.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
		tangent1 = normalize(tangent1);

		bitangent1.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
		bitangent1.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
		bitangent1.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
		bitangent1 = normalize(bitangent1);

		tangent.push_back(tangent1.x);
		tangent.push_back(tangent1.y);
		tangent.push_back(tangent1.z);

		bitangent.push_back(bitangent1.x);
		bitangent.push_back(bitangent1.y);
		bitangent.push_back(bitangent1.z);
	}

	GLuint vboVertices, vboIndices, vboNormals, vboTexCoords, vboTangent, vboBitangent;

	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vboIndices);

	glBindVertexArray(vao);

	glGenBuffers(1, &vboVertices);
	glBindBuffer(GL_ARRAY_BUFFER, vboVertices);
	glBufferData(GL_ARRAY_BUFFER, pos.size() * sizeof(GLfloat), &pos[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), nullptr);
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &vboNormals);
	glBindBuffer(GL_ARRAY_BUFFER, vboNormals);
	glBufferData(GL_ARRAY_BUFFER, norm.size() * sizeof(GLfloat), &norm[0], GL_STATIC_DRAW);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), nullptr);
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &vboTexCoords);
	glBindBuffer(GL_ARRAY_BUFFER, vboTexCoords);
	glBufferData(GL_ARRAY_BUFFER, texcoords.size() * sizeof(GLfloat), &texcoords[0], GL_STATIC_DRAW);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), nullptr);
	glEnableVertexAttribArray(2);

	glGenBuffers(1, &vboTangent);
	glBindBuffer(GL_ARRAY_BUFFER, vboTangent);
	glBufferData(GL_ARRAY_BUFFER, tangent.size() * sizeof(GLfloat), &tangent[0], GL_STATIC_DRAW);
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), nullptr);
	glEnableVertexAttribArray(4);

	glGenBuffers(1, &vboBitangent);
	glBindBuffer(GL_ARRAY_BUFFER, vboBitangent);
	glBufferData(GL_ARRAY_BUFFER, bitangent.size() * sizeof(GLfloat), &bitangent[0], GL_STATIC_DRAW);
	glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), nullptr);
	glEnableVertexAttribArray(5);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIndices);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(int), &indices[0], GL_STATIC_DRAW);

	glBindVertexArray(0);

	return indices.size();
}



GLsizei CreateTriangle(GLuint& vao)
{
	std::vector<float> positions = { -1.0f, 0.0f, 0.0f, 1.0f,
									  1.0f, 0.0f, 0.0f, 1.0f,
									  0.0f, 2.0f, 0.0f, 1.0f };

	std::vector<float> normals = { 1.0f, 0.0f, 0.0f, 0.1f,
									1.0f, 0.0f, 0.0f, 0.1f,
									1.0f, 0.0f, 0.0f, 0.1f };

	std::vector<float> texCoords = { 0.0f, 0.0f,
									 0.5f, 1.0f,
									 1.0f, 0.0f };

	std::vector<uint32_t> indices = { 0u, 1u, 2u };

	GLuint vboVertices, vboIndices, vboNormals, vboTexCoords;

	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vboIndices);

	glBindVertexArray(vao);

	glGenBuffers(1, &vboVertices);
	glBindBuffer(GL_ARRAY_BUFFER, vboVertices);
	glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(GLfloat), positions.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), nullptr);
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &vboNormals);
	glBindBuffer(GL_ARRAY_BUFFER, vboNormals);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(GLfloat), normals.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), nullptr);
	glEnableVertexAttribArray(1);

	/* glGenBuffers(1, &vboTexCoords);
	 glBindBuffer(GL_ARRAY_BUFFER, vboTexCoords);
	 glBufferData(GL_ARRAY_BUFFER, texCoords.size() * sizeof(GLfloat), texCoords.data(), GL_STATIC_DRAW);
	 glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), nullptr);
	 glEnableVertexAttribArray(2);*/

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIndices);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(int), indices.data(), GL_STATIC_DRAW);

	glBindVertexArray(0);

	return indices.size();
}

GLuint CreateCube(GLuint& vao, float4 a, float4 b, float4 c, float4 d, float4 c1) {
	std::vector<float> positions = {
		a.x, a.y, a.z, a.w,
		b.x, b.y, b.z, b.w,
		c.x, c.y, c.z, c.w,
		d.x, d.y, d.z, d.w,
		a.x, c1.y, a.z, a.w,
		b.x, c1.y, b.z, a.w,
		c1.x, c1.y, c1.z, c1.w,
		d.x, c1.y, d.z, d.w
	};

	std::vector<float> normals = {
		-1.0, 0.0, 0.0,
		 0.0, 1.0, 0.0,
		 1.0, 0.0, 0.0,
		 0.0, -1.0, 0.0,
		 0.0, 0.0, 1.0,
		 0.0, 0.0, -1.0
	};

	std::vector<float> texcoords = {
		a.x, a.y,
		b.x, b.y,
		c.x, c.y,
		d.x, d.y,
		a.x, c1.y,
		b.x, c1.y,
		c1.x, c1.y,
		d.x, c1.y
	};
	std::vector<uint32_t> indices = {
		1u, 4u, 0u,
		1u, 4u, 5u,
		1u, 5u, 2u,
		6u, 5u, 2u,
		6u, 7u, 2u,
		3u, 7u, 2u,
		3u, 7u, 0u,
		4u, 7u, 0u,
		4u, 6u, 5u,
		4u, 6u, 7u,
		0u, 2u, 1u,
		0u, 2u, 3u,
	};

	GLuint vboVertices, vboIndices, vboNormals, vboTexCoords;

	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vboIndices);

	glBindVertexArray(vao);

	glGenBuffers(1, &vboVertices);
	glBindBuffer(GL_ARRAY_BUFFER, vboVertices);
	glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(GLfloat), positions.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), nullptr);
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &vboNormals);
	glBindBuffer(GL_ARRAY_BUFFER, vboNormals);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(GLfloat), normals.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), nullptr);
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &vboTexCoords);
	glBindBuffer(GL_ARRAY_BUFFER, vboTexCoords);
	glBufferData(GL_ARRAY_BUFFER, texcoords.size() * sizeof(GLfloat), &texcoords[0], GL_STATIC_DRAW);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), nullptr);
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIndices);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(int), indices.data(), GL_STATIC_DRAW);

	glBindVertexArray(0);

	return indices.size();
}

GLuint CreateCone(GLuint& vao, float4 center, float4 vertex, float radius, int slices) {

	std::vector<float> norm(4 * (slices + 2), 0.0f);
	std::vector<float> positions(4 * (slices + 2), 0.0f);
	std::vector<float> texcoords(4 * (slices + 2), 0.0f);
	positions.at(0) = (vertex.x);
	positions.at(1) = (vertex.y);
	positions.at(2) = (vertex.z);
	positions.at(3) = (vertex.w);
	positions.at(4) = (center.x);
	positions.at(5) = (center.y);
	positions.at(6) = (center.z);
	positions.at(7) = (center.w);

	float angleStep = (2.0f * 3.14159265358979323846f) / ((float)slices);

	for (int i = 2; i < slices + 2; i++) {
		positions.at(i * 4) = center.x + cosf(angleStep * (i - 2)) * radius;
		positions.at(i * 4 + 1) = center.y + sinf(angleStep * (i - 2)) * radius;
		positions.at(i * 4 + 2) = center.z;
		positions.at(i * 4 + 3) = 1.0f;

		norm.at(i + 0) = positions.at(i + 0) / radius;
		norm.at(i + 1) = positions.at(i + 1) / radius;
		norm.at(i + 2) = positions.at(i + 2) / radius;
		norm.at(i + 3) = 1.0f;
	}

	std::vector<uint32_t>indicies(slices * 3 * 2, 0.0f);

	for (unsigned int i = 2; i < slices + 2; i++) {
		indicies.at((i - 2) * 6 + 0) = 1u;
		indicies.at((i - 2) * 6 + 1) = i;
		indicies.at((i - 2) * 6 + 2) = i >= (slices + 1) ? 2u : i + 1;

		indicies.at((i - 2) * 6 + 3) = 0u;
		indicies.at((i - 2) * 6 + 4) = i;
		indicies.at((i - 2) * 6 + 5) = i >= (slices + 1) ? 2u : i + 1;
	}

	GLuint vboVertices, vboIndices, vboNormals, vboTexCoords;
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vboIndices);

	glBindVertexArray(vao);

	glGenBuffers(1, &vboVertices);
	glBindBuffer(GL_ARRAY_BUFFER, vboVertices);
	glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(GLfloat), positions.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), nullptr);
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &vboNormals);
	glBindBuffer(GL_ARRAY_BUFFER, vboNormals);
	glBufferData(GL_ARRAY_BUFFER, norm.size() * sizeof(GLfloat), norm.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), nullptr);
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &vboTexCoords);
	glBindBuffer(GL_ARRAY_BUFFER, vboTexCoords);
	glBufferData(GL_ARRAY_BUFFER, texcoords.size() * sizeof(GLfloat), &texcoords[0], GL_STATIC_DRAW);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), nullptr);
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIndices);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicies.size() * sizeof(int), indicies.data(), GL_STATIC_DRAW);

	glBindVertexArray(0);

	return indicies.size();
}

GLuint createCylinder(GLuint& vao, float4 bottonCenter, float4 topCenter, float radius, uint32_t slices) {
	std::vector<float> norm(2 * 4 * (slices)+8, 0.0f);
	std::vector<float> positions(2 * 4 * (slices)+8, 0.0f);
	std::vector<float> texcoords(2 * 2 * (slices)+8, 0.5f);
	positions.at(0) = (bottonCenter.x);
	positions.at(1) = (bottonCenter.y);
	positions.at(2) = (bottonCenter.z);
	positions.at(3) = (bottonCenter.w);
	positions.at((slices + 1) * 4) = (topCenter.x);
	positions.at((slices + 1) * 4 + 1) = (topCenter.y);
	positions.at((slices + 1) * 4 + 2) = (topCenter.z);
	positions.at((slices + 1) * 4 + 3) = (topCenter.w);
	norm.at(0) = -1.0f;
	norm.at(1) = 1.0f;
	norm.at(2) = -1.0f;
	norm.at(3) = 1.0f;


	float angleStep = (2.0f * 3.14159265358979323846f) / ((float)slices);

	for (int i = 1; i < (slices + 1) * 2; i++) {
		if (i < (slices + 1)) {
			positions.at(i * 4) = bottonCenter.x + cosf(angleStep * (i - 2)) * radius;
			positions.at(i * 4 + 1) = bottonCenter.y;
			positions.at(i * 4 + 2) = bottonCenter.z + sinf(angleStep * (i - 2)) * radius;
			positions.at(i * 4 + 3) = 1.0f;
		}
		else  if (i != slices + 1) {
			positions.at(i * 4) = topCenter.x + cosf(angleStep * (i - 2)) * radius;
			positions.at(i * 4 + 1) = topCenter.y;
			positions.at(i * 4 + 2) = topCenter.z + sinf(angleStep * (i - 2)) * radius;
			positions.at(i * 4 + 3) = 1.0f;
		}
	}

	for (int i = 0; i < 2 * slices + 2; i++) {
		norm.at(i * 4 + 0) = 0 - i / (slices);
		norm.at(i * 4 + 1) = -1.0f;
		norm.at(i * 4 + 2) = 0.5f + i / (slices);
		norm.at(i * 4 + 3) = 1.0f;
	}
	//for (int i = 0; i < 2 * slices + 2; i++) {
	//    std::cout << i <<": "<< positions.at(4*i)<< " " << positions.at(4*i+1)<<std::endl;
	//}
	std::vector<uint32_t>indicies(slices * 12, 0.0f);

	for (unsigned int i = 1; i < slices + 1; i++) {
		indicies.at((i - 1) * 12 + 0) = 0u;
		indicies.at((i - 1) * 12 + 1) = i;
		indicies.at((i - 1) * 12 + 2) = i >= (slices) ? 1u : i + 1;

		indicies.at((i - 1) * 12 + 3) = slices + 1;
		indicies.at((i - 1) * 12 + 4) = i + slices + 1;
		indicies.at((i - 1) * 12 + 5) = i + slices + 1 >= (2 * (slices)+1) ? slices + 2 : slices + i + 2;

		indicies.at((i - 1) * 12 + 6) = i;
		indicies.at((i - 1) * 12 + 7) = slices + i + 1;
		indicies.at((i - 1) * 12 + 8) = i >= (slices) ? 1u : i + 1;

		indicies.at((i - 1) * 12 + 9) = i + slices + 1 >= (2 * (slices)+1) ? slices + 2 : slices + i + 2;
		indicies.at((i - 1) * 12 + 10) = slices + i + 1;
		indicies.at((i - 1) * 12 + 11) = i >= (slices) ? 1u : i + 1;
	}
	//for (int i = 0; i < slices * 4; i++) {
	//    std::cout << indicies.at(i*3)<<" "<<indicies.at(i*3+1)<<" "<<indicies.at(i*3+2)<<std::endl;
	//}
	GLuint vboVertices, vboIndices, vboNormals, vboTexCoords;
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vboIndices);

	glBindVertexArray(vao);

	glGenBuffers(1, &vboVertices);
	glBindBuffer(GL_ARRAY_BUFFER, vboVertices);
	glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(GLfloat), positions.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), nullptr);
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &vboNormals);
	glBindBuffer(GL_ARRAY_BUFFER, vboNormals);
	glBufferData(GL_ARRAY_BUFFER, norm.size() * sizeof(GLfloat), norm.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), nullptr);
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &vboTexCoords);
	glBindBuffer(GL_ARRAY_BUFFER, vboTexCoords);
	glBufferData(GL_ARRAY_BUFFER, texcoords.size() * sizeof(GLfloat), &texcoords[0], GL_STATIC_DRAW);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), nullptr);
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIndices);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicies.size() * sizeof(int), indicies.data(), GL_STATIC_DRAW);

	glBindVertexArray(0);

	return indicies.size();
}

GLuint createPlane(GLuint& vao, float4 position1, float4 position2, float4 position3) {
	std::vector<float> positions = {
		position1.x, position1.y, position1.z, position1.w,
		position2.x, position2.y, position2.z, position2.w,
		position3.x, position3.y, position3.z, position3.w,
	};
	positions.push_back(position1.x + position3.x - position2.x);
	positions.push_back(position1.y + position3.y - position2.y);
	positions.push_back(position1.z + position3.z - position2.z);
	positions.push_back(1.0f);
	std::vector<float> norms = {
		1.0f, 0.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 0.0f, 1.0f,
	};
	std::vector<uint32_t>indicies = {
		0u, 1u, 2u,
		2u, 3u, 0u
	};
	GLuint vboVertices, vboIndices, vboNormals;
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vboIndices);

	glBindVertexArray(vao);

	glGenBuffers(1, &vboVertices);
	glBindBuffer(GL_ARRAY_BUFFER, vboVertices);
	glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(GLfloat), positions.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), nullptr);
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &vboNormals);
	glBindBuffer(GL_ARRAY_BUFFER, vboNormals);
	glBufferData(GL_ARRAY_BUFFER, norms.size() * sizeof(GLfloat), norms.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), nullptr);
	glEnableVertexAttribArray(1);


	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIndices);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicies.size() * sizeof(int), indicies.data(), GL_STATIC_DRAW);

	glBindVertexArray(0);

	return indicies.size();
}

GLuint loadFile(const char* p, GLuint& vao) {
	struct normStuct {
		uint32_t index;
		float kd[3];
	};
	std::vector<float> vertices;
	std::vector<float> norm;
	std::vector<float> norms;
	std::vector<float> tex;
	std::vector<float> texs;
	std::vector<uint32_t> indicies;
	std::vector<normStuct> normIndicies;
	std::vector<uint32_t> uvIndices;

	std::string path = (std::string)p;
	path.append("obj");

	int fileType = 0; // Вручную пишем вначале файла его тип (11: поверхность состоит из 4 вершин, указаны индексы нормалей и текстурных коор)
	// 21: Поверхность состоит из 4 вершин, однако без указания текст.
	// 22: Тоже самое но из 3
	// 31: Поверхность состоит их 4 вершин, однако без указания нормалей
	// 32: Тоже самое но из 3

	FILE* file = fopen(path.c_str(), "rb");
	float kd[3] = { 1, 1, 1 };
	while (true) {
		char lineHeader[128];

		int res = fscanf(file, "%s", lineHeader);
		if (res == EOF)
			break;

		if (fileType == 0) {
			fileType = atoi(lineHeader);
			std::cout << fileType;
		}

		if (strcmp(lineHeader, "v") == 0) {
			float x, y, z;
			fscanf(file, "%f %f %f\n", &x, &y, &z);
			vertices.push_back(x);
			vertices.push_back(y);
			vertices.push_back(z);
			vertices.push_back(1.0f);
		}
		else if (strcmp(lineHeader, "vt") == 0) {
			float x, y;
			fscanf(file, "%f %f\n", &x, &y);
			tex.push_back(x);
			tex.push_back(y);
		}
		else if (strcmp(lineHeader, "vn") == 0) {
			float x, y, z;
			fscanf(file, "%f %f %f\n", &x, &y, &z);
			norm.push_back(x);
			norm.push_back(y);
			norm.push_back(z);
			norm.push_back(1.0f);
		}
		else if (strcmp(lineHeader, "f") == 0) {
			unsigned int vertexIndex[4], uvIndex[4], normalIndex[4];
			char c[256];
			int matches;
			//int matches = fscanf(file, "%d/%d %d/%d %d/%d\n", &vertexIndex[0], &normalIndex[0], &vertexIndex[1], &normalIndex[1], &vertexIndex[2], &normalIndex[2]);
			switch (fileType) {
			case 11:
				matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d %d/%d/%d", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2], &vertexIndex[3], &uvIndex[3], &normalIndex[3]);

				indicies.push_back(vertexIndex[0] - 1);
				indicies.push_back(vertexIndex[1] - 1);
				indicies.push_back(vertexIndex[2] - 1);
				uvIndices.push_back(uvIndex[0] - 1);
				uvIndices.push_back(uvIndex[1] - 1);
				uvIndices.push_back(uvIndex[2] - 1);
				normIndicies.push_back({ normalIndex[0] - 1, {kd[0], kd[1],kd[2]} });
				normIndicies.push_back({ normalIndex[1] - 1, {kd[0], kd[1],kd[2]} });
				normIndicies.push_back({ normalIndex[2] - 1, {kd[0], kd[1],kd[2]} });

				indicies.push_back(vertexIndex[3] - 1);
				uvIndices.push_back(uvIndex[3] - 1);
				normIndicies.push_back({ normalIndex[3] - 1, {kd[0], kd[1],kd[2]} });
				indicies.push_back(vertexIndex[0] - 1);
				indicies.push_back(vertexIndex[2] - 1);
				uvIndices.push_back(uvIndex[0] - 1);
				uvIndices.push_back(uvIndex[2] - 1);
				normIndicies.push_back({ normalIndex[0] - 1, {kd[0], kd[1],kd[2]} });
				normIndicies.push_back({ normalIndex[2] - 1, {kd[0], kd[1],kd[2]} });
			case 12: {
				matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2]);

				indicies.push_back(vertexIndex[0] - 1);
				indicies.push_back(vertexIndex[1] - 1);
				uvIndices.push_back(uvIndex[0] - 1);
				uvIndices.push_back(uvIndex[1] - 1);
				normIndicies.push_back({ normalIndex[0] - 1, {kd[0], kd[1],kd[2]} });
				normIndicies.push_back({ normalIndex[1] - 1, {kd[0], kd[1],kd[2]} });
				uvIndices.push_back(uvIndex[2] - 1);
				indicies.push_back(vertexIndex[2] - 1);
				normIndicies.push_back({ normalIndex[2] - 1, {kd[0], kd[1],kd[2]} });

				/*int z = 0;
				int polN = 0;
				int j = 1;
				for (int i = 1; i < 256; i++) {
					if (c[i] == '/' || c[i] == ' ' || (c[i] == '\r' && (int)c[i - 1] > 20) || (c[i] == '\n' && (int)c[i - 1] > 20)) {
						std::string vert;
						while (j != i) {
							if (int(c[j]) > 20) {
								vert += c[j];
							}
							j++;
						}
						j++;
						if (z%3 == 0) {
							if (polN == 4) {
								std::cout << 123;
								indicies.push_back(vertexIndex[2] - 1);
								indicies.push_back(vertexIndex[0] - 1);
							}
							vertexIndex[polN] = uint32_t(stoi(vert));
							indicies.push_back(vertexIndex[polN] - 1);
						}
						else if (z%3 == 1) {
							if (polN == 4) {
								std::cout << 123;
								uvIndices.push_back(uvIndex[0] - 1);
								uvIndices.push_back(uvIndex[2] - 1);
							}
							uvIndex[polN] = uint32_t(stoi(vert));
							uvIndices.push_back(uvIndex[polN] - 1);
						}
						else if (z%3== 2) {
							if (polN == 4) {
								normIndicies.push_back({ normalIndex[0] - 1, {kd[0], kd[1],kd[2]} });
								normIndicies.push_back({ normalIndex[2] - 1, {kd[0], kd[1],kd[2]} });
							}
							normalIndex[polN] = uint32_t(stoi(vert));
							normIndicies.push_back({ normalIndex[polN] - 1, { kd[0], kd[1],kd[2] } });
							polN++;
						}
						z++;
					}
					if ((int)c[i] == -52) {
						break;
					}
				}*/
				break;
			}
			case 21:
				matches = fscanf(file, "%d//%d %d//%d %d//%d %d//%d\n", &vertexIndex[0], &normalIndex[0], &vertexIndex[1], &normalIndex[1], &vertexIndex[2], &normalIndex[2], &vertexIndex[3], &normalIndex[3]);
				indicies.push_back(vertexIndex[0] - 1);
				indicies.push_back(vertexIndex[1] - 1);
				indicies.push_back(vertexIndex[2] - 1);
				indicies.push_back(vertexIndex[3] - 1);
				normIndicies.push_back({ normalIndex[0] - 1, {kd[0], kd[1],kd[2]} });
				normIndicies.push_back({ normalIndex[1] - 1, {kd[0], kd[1],kd[2]} });
				normIndicies.push_back({ normalIndex[2] - 1, {kd[0], kd[1],kd[2]} });
				normIndicies.push_back({ normalIndex[3] - 1, {kd[0], kd[1],kd[2]} });
				break;
			case 22:
				matches = fscanf(file, "%d//%d %d//%d %d//%d\n", &vertexIndex[0], &normalIndex[0], &vertexIndex[1], &normalIndex[1], &vertexIndex[2], &normalIndex[2]);
				indicies.push_back(vertexIndex[0] - 1);
				indicies.push_back(vertexIndex[1] - 1);
				indicies.push_back(vertexIndex[2] - 1);
				normIndicies.push_back({ normalIndex[0] - 1, {kd[0], kd[1],kd[2]} });
				normIndicies.push_back({ normalIndex[1] - 1, {kd[0], kd[1],kd[2]} });
				normIndicies.push_back({ normalIndex[2] - 1, {kd[0], kd[1],kd[2]} });
				break;
			case 31:
				matches = fscanf(file, "%d/%d %d/%d %d/%d %d/%d\n", &vertexIndex[0], &uvIndex[0], &vertexIndex[1], &uvIndex[1], &vertexIndex[2], &uvIndex[2], &vertexIndex[3], &uvIndex[3]);
				indicies.push_back(vertexIndex[0] - 1);
				indicies.push_back(vertexIndex[1] - 1);
				indicies.push_back(vertexIndex[2] - 1);
				indicies.push_back(vertexIndex[3] - 1);
				uvIndices.push_back(uvIndex[0] - 1);
				uvIndices.push_back(uvIndex[1] - 1);
				uvIndices.push_back(uvIndex[2] - 1);
				uvIndices.push_back(uvIndex[3] - 1);
				break;
			case 32:
				matches = fscanf(file, "%d/%d %d/%d %d/%d\n", &vertexIndex[0], &uvIndex[0], &vertexIndex[1], &uvIndex[1], &vertexIndex[2], &uvIndex[2]);
				indicies.push_back(vertexIndex[0] - 1);
				indicies.push_back(vertexIndex[1] - 1);
				indicies.push_back(vertexIndex[2] - 1);
				uvIndices.push_back(uvIndex[0] - 1);
				uvIndices.push_back(uvIndex[1] - 1);
				uvIndices.push_back(uvIndex[2] - 1);
				break;
			}
		}
		else if (strcmp(lineHeader, "usemtl") == 0) {
			char mtlLink[128];
			fscanf(file, "%s\n", &mtlLink);
			FILE* file1 = fopen((std::string(p).append("mtl")).c_str(), "rb");
			while (true) {
				char lineHeader[128];
				char mtlName[64];

				int res = fscanf(file1, "%s %s\n", mtlName, lineHeader);
				if (res == EOF)
					break;

				if (strcmp(lineHeader, mtlLink) == 0) {
					char k[16];
					float t1, t2, t3;
					//fscanf(file1, "\n");
					fscanf(file1, "%s\n", k);
					fscanf(file1, "%s %d %d %d\n", k, &kd[0], &kd[1], &kd[2]);
					fscanf(file1, "%s %d %d %d\n", k, &t1, &t2, &t3);
				}
			}
		}

	};


	if (fileType != 31 && fileType != 32) {
		for (int i = 0; i < vertices.size() / 4; i++) {
			for (int j = 0; j < normIndicies.size(); j++) {
				if (indicies.at(j) == i) {
					norms.push_back(norm.at(normIndicies.at(j).index * 4) * normIndicies.at(j).kd[0]);
					norms.push_back(norm.at(normIndicies.at(j).index * 4 + 1) * normIndicies.at(j).kd[1]);
					norms.push_back(norm.at(normIndicies.at(j).index * 4 + 2) * normIndicies.at(j).kd[2]);
					norms.push_back(1.0f);
					break;
				}
			}
		}
	}
	else {
		norms = norm;
	}

	if (fileType != 21 && fileType != 22) {
		for (int i = 0; i < vertices.size() / 4; i++) {
			for (int j = 0; j < uvIndices.size(); j++) {
				if (indicies.at(j) == i) {
					texs.push_back(tex.at(uvIndices.at(j) * 2));
					texs.push_back(tex.at(uvIndices.at(j) * 2 + 1));
					break;
				}
			}
		}
	}
	else {
		texs = tex;
	}

	std::vector<float> tangent, bitangent;

	for (int i = 0; i < vertices.size() / 4; i += 1) {
		float3 edge1, edge2;
		float2 deltaUV1, deltaUV2;
		if (i < vertices.size() - 3) {
			edge1 = float3(vertices.at(i + 4), vertices.at(i + 5), vertices.at(i + 6)) - float3(vertices.at(i), vertices.at(i + 1), vertices.at(i + 2));
			edge2 = float3(vertices.at(i + 8), vertices.at(i + 9), vertices.at(i + 10)) - float3(vertices.at(i), vertices.at(i + 1), vertices.at(i + 2));
			deltaUV1 = float2(texs.at(i + 3), texs.at(i + 4)) - float2(texs.at(i), texs.at(i + 1));
			deltaUV2 = float2(texs.at(i + 5), texs.at(i + 6)) - float2(texs.at(i), texs.at(i + 1));
		}
		else if (i == vertices.size() - 2) {
			edge1 = float3(vertices.at(i + 4), vertices.at(i + 5), vertices.at(i + 6)) - float3(vertices.at(i), vertices.at(i + 1), vertices.at(i + 2));
			edge2 = float3(vertices.at(0), vertices.at(1), vertices.at(2)) - float3(vertices.at(i), vertices.at(i + 1), vertices.at(i + 2));
			deltaUV1 = float2(texs.at(i + 3), texs.at(i + 4)) - float2(texs.at(i), texs.at(i + 1));
			deltaUV2 = float2(texs.at(0), texs.at(1)) - float2(texs.at(i), texs.at(i + 1));
		}
		else {
			edge1 = float3(vertices.at(0), vertices.at(1), vertices.at(2)) - float3(vertices.at(i), vertices.at(i + 1), vertices.at(i + 2));
			edge2 = float3(vertices.at(3), vertices.at(4), vertices.at(5)) - float3(vertices.at(i), vertices.at(i + 1), vertices.at(i + 2));
			deltaUV1 = float2(texs.at(0), texs.at(1)) - float2(texs.at(i), texs.at(i + 1));
			deltaUV2 = float2(texs.at(2), texs.at(3)) - float2(texs.at(i), texs.at(i + 1));
		}
		float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
		float3 tangent1, bitangent1;

		tangent1.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
		tangent1.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
		tangent1.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
		tangent1 = normalize(tangent1);

		bitangent1.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
		bitangent1.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
		bitangent1.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
		bitangent1 = normalize(bitangent1);

		tangent.push_back(tangent1.x);
		tangent.push_back(tangent1.y);
		tangent.push_back(tangent1.z);

		bitangent.push_back(bitangent1.x);
		bitangent.push_back(bitangent1.y);
		bitangent.push_back(bitangent1.z);
	}

	GLuint vboVertices, vboIndices, vboNormals, vboTexCoords, vboTangent, vboBitangent;
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vboIndices);

	glBindVertexArray(vao);

	glGenBuffers(1, &vboVertices);
	glBindBuffer(GL_ARRAY_BUFFER, vboVertices);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), nullptr);
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &vboNormals);
	glBindBuffer(GL_ARRAY_BUFFER, vboNormals);
	glBufferData(GL_ARRAY_BUFFER, norms.size() * sizeof(GLfloat), norms.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), nullptr);
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &vboTexCoords);
	glBindBuffer(GL_ARRAY_BUFFER, vboTexCoords);
	glBufferData(GL_ARRAY_BUFFER, texs.size() * sizeof(GLfloat), &texs[0], GL_STATIC_DRAW);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), nullptr);
	glEnableVertexAttribArray(2);

	glGenBuffers(1, &vboTangent);
	glBindBuffer(GL_ARRAY_BUFFER, vboTangent);
	glBufferData(GL_ARRAY_BUFFER, tangent.size() * sizeof(GLfloat), &tangent[0], GL_STATIC_DRAW);
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), nullptr);
	glEnableVertexAttribArray(4);

	glGenBuffers(1, &vboBitangent);
	glBindBuffer(GL_ARRAY_BUFFER, vboBitangent);
	glBufferData(GL_ARRAY_BUFFER, bitangent.size() * sizeof(GLfloat), &bitangent[0], GL_STATIC_DRAW);
	glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), nullptr);
	glEnableVertexAttribArray(5);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIndices);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicies.size() * sizeof(int), indicies.data(), GL_STATIC_DRAW);

	glBindVertexArray(0);

	return indicies.size();
}

void createSkybox(GLuint& vao) {
	float skyboxVertices[] = {
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f
	};

	GLuint vbo;
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
}

GLuint cave(GLuint& vao) {
	return loadFile("../models/wall2.", vao);
}

GLuint dangeonRoof(GLuint& vao) {
	return loadFile("../models/texture.", vao);
}

GLuint conteiner(GLuint& vao) {
	return loadFile("../models/Crate.", vao);
}

GLuint gate(GLuint& vao) {
	return loadFile("../models/chest.", vao);
}

GLuint createQuad(GLuint& quadVAO) {
	float quadVertices[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
	  // positions   // texCoords
	  -1.0f,  1.0f,  0.0f, 1.0f,
	  -1.0f, -1.0f,  0.0f, 0.0f,
	   1.0f, -1.0f,  1.0f, 0.0f,

	  -1.0f,  1.0f,  0.0f, 1.0f,
	   1.0f, -1.0f,  1.0f, 0.0f,
	   1.0f,  1.0f,  1.0f, 1.0f
	};
	// screen quad VAO
	unsigned int quadVBO;
	glGenVertexArrays(1, &quadVAO);
	glGenBuffers(1, &quadVBO);
	glBindVertexArray(quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

	return 6;
}

GLuint intsScene(GLuint& vao, float4 a, float4 b, float4 c, float4 d, float4 c1) {
	std::vector<float> positions = {
	   a.x, a.y, a.z, a.w,
	   b.x, b.y, b.z, b.w,
	   c.x, c.y, c.z, c.w,
	   d.x, d.y, d.z, d.w,
	   a.x, c1.y, a.z, a.w,
	   b.x, c1.y, b.z, a.w,
	   c1.x, c1.y, c1.z, c1.w,
	   d.x, c1.y, d.z, d.w
	};

	std::vector<float> normals = {
		-1.0f, -1.0f,  1.0f, 1.0f,
		 1.0f, -1.0f, -1.0f, 1.0f,
		-1.0f,  1.0f, -1.0f, 1.0f,
		 1.0f, -1.0f, -1.0f, 1.0f,
		-1.0f,  1.0f, -1.0f, 1.0f,
		 1.0f, -1.0f,  1.0f, 1.0f,
		-1.0f,  1.0f, -1.0f, 1.0f,
		-1.0f,  1.0f, -1.0f, 1.0f };

	std::vector<float> texcoords = {
		a.x, a.y,
		b.x, b.y,
		c.x, c.y,
		d.x, d.y,
		a.x, c1.y,
		b.x, c1.y,
		c1.x, c1.y,
		d.x, c1.y
	};
	std::vector<uint32_t> indices = {
		1u, 4u, 0u,
		1u, 4u, 5u,
		1u, 5u, 2u,
		6u, 5u, 2u,
		6u, 7u, 2u,
		3u, 7u, 2u,
		3u, 7u, 0u,
		4u, 7u, 0u,
		4u, 6u, 5u,
		4u, 6u, 7u,
		0u, 2u, 1u,
		0u, 2u, 3u,
	};

	std::vector<float> translations;
	int index = 0;
	float offset = 1.1f;
	for (int y = -4000; y < 4000; y += 8)
	{
		for (int x = -4000; x < 4000; x += 8)
		{
			translations.push_back((float)x / 1000.0f + offset);
			translations.push_back((float)y / 1000.0f + offset);
		}
	}

	// store instance data in an array buffer
	// --------------------------------------
	unsigned int instanceVBO;
	glGenBuffers(1, &instanceVBO);
	glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
	glBufferData(GL_ARRAY_BUFFER, translations.size() * sizeof(GLfloat), translations.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	GLuint vboVertices, vboIndices, vboNormals, vboTexCoords;
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vboIndices);

	glBindVertexArray(vao);

	glGenBuffers(1, &vboVertices);
	glBindBuffer(GL_ARRAY_BUFFER, vboVertices);
	glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(GLfloat), positions.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), nullptr);
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &vboNormals);
	glBindBuffer(GL_ARRAY_BUFFER, vboNormals);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(GLfloat), normals.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), nullptr);
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &vboTexCoords);
	glBindBuffer(GL_ARRAY_BUFFER, vboTexCoords);
	glBufferData(GL_ARRAY_BUFFER, texcoords.size() * sizeof(GLfloat), &texcoords[0], GL_STATIC_DRAW);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), nullptr);
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIndices);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(int), indices.data(), GL_STATIC_DRAW);


	glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), nullptr);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glEnableVertexAttribArray(3);
	glVertexAttribDivisor(3, 1);

	glBindVertexArray(0);

	return indices.size();
}



int initGL()
{
	int res = 0;

	//грузим функции opengl через glad
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize OpenGL context" << std::endl;
		return -1;
	}

	//выводим в консоль некоторую информацию о драйвере и контексте opengl
	std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;
	std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
	std::cout << "Version: " << glGetString(GL_VERSION) << std::endl;
	std::cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

	std::cout << "Controls: " << std::endl;
	std::cout << "press right mouse button to capture/release mouse cursor  " << std::endl;
	std::cout << "press spacebar to alternate between shaded wireframe and fill display modes" << std::endl;
	std::cout << "press ESC to exit" << std::endl;

	return 0;
}

unsigned int loadTexture(char const* path)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		//glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_FLOAT, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}

unsigned int loadCubemap(std::vector<std::string> faces)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrChannels;
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
			);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}

int main(int argc, char** argv)
{
	if (!glfwInit())
		return -1;

	//запрашиваем контекст opengl версии 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);


	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "OpenGL basic sample", nullptr, nullptr);
	if (window == nullptr)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);

	//регистрируем коллбеки для обработки сообщений от пользователя - клавиатура, мышь..
	glfwSetKeyCallback(window, OnKeyboardPressed);
	glfwSetCursorPosCallback(window, OnMouseMove);
	glfwSetMouseButtonCallback(window, OnMouseButtonClicked);
	glfwSetScrollCallback(window, OnMouseScroll);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	if (initGL() != 0)
		return -1;

	//Reset any OpenGL errors which could be present for some reason
	GLenum gl_error = glGetError();
	while (gl_error != GL_NO_ERROR)
		gl_error = glGetError();

	//создание шейдерной программы из двух файлов с исходниками шейдеров
	//используется класс-обертка ShaderProgram
	std::unordered_map<GLenum, std::string> shaders;
	shaders[GL_VERTEX_SHADER] = "../shaders/vertex.glsl";
	shaders[GL_FRAGMENT_SHADER] = "../shaders/lambert.frag";
	ShaderProgram lambert(shaders); GL_CHECK_ERRORS;

	shaders[GL_VERTEX_SHADER] = "../shaders/tunnel.glsl";
	shaders[GL_FRAGMENT_SHADER] = "../shaders/tunnel.frag";
	ShaderProgram tunnel(shaders); GL_CHECK_ERRORS;

	GLuint vaoSphere;
	float radius = 1.0f;
	GLsizei sphereIndices = CreateSphere(radius, 118, vaoSphere);

	GLuint vaoCone;
	float4 center = float4(0.5f, 0.5f, 0.0f, 1.0f);
	float4 vertex = float4(0.5f, 0.5f, 3.0f, 1.0f);
	radius = 1.0f;
	int slices = 118;
	GLsizei coneIndices = CreateCone(vaoCone, center, vertex, radius, slices);

	GLuint vaoCylinder;
	float4 topCenter = float4(0.0f, -1.0f, 0.0f, 1.0f);
	float4 bottonCenter = float4(0.0f, -5.0f, 0.0f, 1.0f);
	radius = 0.2f;
	slices = 116;
	GLsizei cylinderIndices = createCylinder(vaoCylinder, bottonCenter, topCenter, radius, slices);

	GLuint vaoCube;
	GLsizei cubeIndices = CreateCube(vaoCube, float4(0.1f, -5.0f, 0.0f, 1.0f), float4(0.5f, -5.0f, 0.0f, 1.0f), float4(0.5f, -5.0f, 0.3f, 1.0f), float4(0.1f, -5.0f, 0.3f, 1.0f), float4(0.5f, -4.9f, 0.3f, 1.0f));

	GLuint vaoPlane;
	GLsizei planeIndices = createPlane(vaoPlane, float4(-11.0f, -10.0f, -11.0f, 1.0f), float4(-11.0f, -10.0f, 11.0f, 1.0f), float4(11.0f, -10.0f, -11.0f, 1.0f));

	GLuint vaoPlaneWings;
	GLsizei planeWings = CreateCube(vaoPlaneWings, float4(-0.5f, -0.5f, -0.5f, 1.0f), float4(0.5f, -0.5f, -0.5f, 1.0f), float4(0.5f, -0.5f, 0.5f, 1.0f), float4(-0.5f, -0.5f, 0.5f, 1.0f), float4(0.5f, 0.5f, 0.5f, 1.0f));

	GLuint vaoCave;
	GLsizei Cave = cave(vaoCave);

	GLuint vaoInts;
	GLsizei ints = intsScene(vaoInts, float4(0.1f, -5.0f, 0.0f, 1.0f), float4(0.5f, -5.0f, 0.0f, 1.0f), float4(0.5f, -5.0f, 0.3f, 1.0f), float4(0.1f, -5.0f, 0.3f, 1.0f), float4(0.5f, -4.9f, 0.3f, 1.0f));

	GLuint vaoSkybox;
	createSkybox(vaoSkybox);

	GLuint quadVao;
	GLsizei quad = createQuad(quadVao);

	GLuint dungeonRoofVao;
	GLsizei dungeonRoofIn = dangeonRoof(dungeonRoofVao);

	GLuint conteinerVao;
	GLsizei conteinerIn = dangeonRoof(conteinerVao);

	GLuint chestVao;
	GLsizei chestIn = gate(chestVao);

	glViewport(0, 0, WIDTH, HEIGHT);  GL_CHECK_ERRORS;
	glEnable(GL_DEPTH_TEST);  GL_CHECK_ERRORS;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	float move = 0.0f;
	float triggerVar = 0.0f;
	unsigned int diffuseMap = loadTexture("../models/wall.jpg");
	unsigned int dungeonRoofTex = loadTexture("../models/tarmac.png");
	unsigned int specularMap = loadTexture("../models/container2_specular.png");
	unsigned int normalMap = loadTexture("../models/wood-normal.jpg");
	unsigned int groundTex = loadTexture("../models/grass.png");
	unsigned int conteinerTex = loadTexture("../models/container2.png");
	unsigned int chestTex = loadTexture("../models/chest.jpg");
	std::vector<std::string> faces
	{
		"../models/skybox/right.png",
		"../models/skybox/left.png",
		"../models/skybox/top.png",
		"../models/skybox/bottom.png",
		"../models/skybox/front.png",
		"../models/skybox/back.png"
	};
	unsigned int cubemapTexture = loadCubemap(faces);

	tunnel.StartUseShader();
	tunnel.SetUniform("screenTexture", 15);

	unsigned int hdrFBO;
	glGenFramebuffers(1, &hdrFBO);
	// create floating point color buffer
	unsigned int colorBuffer;
	glGenTextures(1, &colorBuffer);
	glBindTexture(GL_TEXTURE_2D, colorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, WIDTH, HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// create depth buffer (renderbuffer)

	unsigned int rboDepth;
	glGenRenderbuffers(1, &rboDepth);
	glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, WIDTH, HEIGHT);
	// attach buffers

	glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorBuffer, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Framebuffer not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//цикл обработки сообщений и отрисовки сцены каждый кадр
	while (!glfwWindowShouldClose(window))
	{
		//считаем сколько времени прошло за кадр
		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		glfwPollEvents();
		doCameraMovement(camera, deltaTime);



		glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
		glEnable(GL_DEPTH_TEST); // тест глубины
				//очищаем экран каждый кадр
		glClearColor(0.9f, 0.95f, 0.97f, 1.0f); GL_CHECK_ERRORS;
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); GL_CHECK_ERRORS;

		lambert.StartUseShader(); GL_CHECK_ERRORS;
		lambert.SetUniform("move", move);
		lambert.SetUniform("material.diffuse", 6);
		lambert.SetUniform("material.specular", 1);
		lambert.SetUniform("normalMap", 5);
		lambert.SetUniform("useNormalMap", true);
		float4x4 view = camera.GetViewMatrix();
		float4x4 projection = projectionMatrixTransposed(camera.zoom, float(WIDTH) / float(HEIGHT), 0.1f, 1000.0f);
		float4x4 model;

		lambert.SetUniform("projection", projection); GL_CHECK_ERRORS;
		// SKkybox
		lambert.SetUniform("type", 1);
		lambert.SetUniform("material.light", 2);
		lambert.SetUniform("skybox", 2);

		glDepthFunc(GL_LEQUAL);
		float4x4 view1;
		view1.row[0] = camera.GetViewMatrix().row[0];
		view1.row[1] = camera.GetViewMatrix().row[1];
		view1.row[2] = camera.GetViewMatrix().row[2];
		lambert.SetUniform("view", view1);
		glBindVertexArray(vaoSkybox);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
		glDepthFunc(GL_LESS);

		lambert.SetUniform("view", view);
		lambert.SetUniform("type", 0);

		glActiveTexture(GL_TEXTURE6);
		glBindTexture(GL_TEXTURE_2D, diffuseMap);
		// bind specular map
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, specularMap);

		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_2D, normalMap);

		lambert.SetUniform("dirLight[0].direction", float3(-0.0f, -5.0f, -0.0f));
		lambert.SetUniform("dirLight[0].ambient", float3(0.05f, 0.05f, 0.05f));
		lambert.SetUniform("dirLight[0].diffuse", float3(0.4f, 0.4f, 0.4f));
		lambert.SetUniform("dirLight[0].specular", float3(0.5f, 0.5f, 0.5f));
		lambert.SetUniform("camPos", camera.pos);
		lambert.SetUniform("material.light", 1);
		lambert.SetUniform("material.shininess", 32.0f);

		lambert.SetUniform("spotLight.position", camera.pos);
		lambert.SetUniform("spotLight.direction", camera.front);
		lambert.SetUniform("spotLight.ambient", float3(0.0f, 0.0f, 0.0f));
		lambert.SetUniform("spotLight.diffuse", float3(1.0f, 1.0f, 1.0f));
		lambert.SetUniform("spotLight.specular", float3(1.0f, 1.0f, 1.0f));
		lambert.SetUniform("spotLight.constant", 1.0f);
		lambert.SetUniform("spotLight.linear", 0.09f);
		lambert.SetUniform("spotLight.quadratic", 0.032f);
		lambert.SetUniform("spotLight.cutOff", cos(12.5f * LiteMath::DEG_TO_RAD));
		lambert.SetUniform("spotLight.outerCutOff", cos(15.0f * LiteMath::DEG_TO_RAD));

		lambert.SetUniform("camPos", camera.pos);

		if (keys[GLFW_KEY_Q]) {
			//if ((int)move % 2 == 0) {
			glBindVertexArray(vaoCylinder); GL_CHECK_ERRORS;
			{
				lambert.SetUniform("model", model); GL_CHECK_ERRORS;
				lambert.SetUniform("pointLights[0].position", float3(0.0f, -2.0f, 0.0f)); GL_CHECK_ERRORS;
				lambert.SetUniform("pointLights[0].ambient", float3(0.55f, 0.55f, 0.55f)); GL_CHECK_ERRORS;
				lambert.SetUniform("pointLights[0].diffuse", float3(0.8f, 0.8f, 0.8f)); GL_CHECK_ERRORS;
				lambert.SetUniform("pointLights[0].specular", float3(1.0f, 1.0f, 1.0f)); GL_CHECK_ERRORS;
				lambert.SetUniform("pointLights[0].constant", 1.0f); GL_CHECK_ERRORS;
				lambert.SetUniform("pointLights[0].linear", 0.09f); GL_CHECK_ERRORS;
				lambert.SetUniform("pointLights[0].quadratic", 0.632f); GL_CHECK_ERRORS;

				glDrawElements(GL_TRIANGLES, cylinderIndices, GL_UNSIGNED_INT, nullptr); GL_CHECK_ERRORS;
				// }
			}
			lambert.SetUniform("material.light", 0);
			glBindVertexArray(vaoSphere); GL_CHECK_ERRORS;
			{
				float4x4 model1 = model;
				model1 = transpose(translate4x4(float3(0.0f, 3.0f, 0.0f)));
				lambert.SetUniform("model", model1); GL_CHECK_ERRORS;

				glDrawElements(GL_TRIANGLE_STRIP, sphereIndices, GL_UNSIGNED_INT, nullptr); GL_CHECK_ERRORS;
			}
			glBindVertexArray(vaoCube); GL_CHECK_ERRORS;
			{
				float4x4 model1 = model;
				std::vector<float4> stairs;

				for (int i = 0; i < 30; i++) {

					if (camera.pos.x * camera.pos.x + camera.pos.y * camera.pos.y + camera.pos.z * camera.pos.z < 5) {
						triggerVar += 0.0015;
					}

					model1 = transpose(mul(translate4x4(float3(0.0f, i * 0.14f, 0.0f)), rotate_Y_4x4(0.5f * i + triggerVar)));
					lambert.SetUniform("model", model1); GL_CHECK_ERRORS;

					glDrawElements(GL_TRIANGLE_STRIP, cubeIndices, GL_UNSIGNED_INT, nullptr); GL_CHECK_ERRORS;
				}
			}

			float radius = 3;
			float angle = 0.5f;

			float moveX = cosf(angle * move) * radius;
			float moveZ = sinf(angle * move) * radius;

			glBindVertexArray(vaoPlaneWings); GL_CHECK_ERRORS;
			// рисуем пропеллер
			{
				float4x4 model1;
				float4x4 timeModel = mul(translate4x4(float3(3.0f, -2.0f, 0.15f)), rotate_Z_4x4(move * 30));
				timeModel = mul(rotate_Y_4x4(-angle * move), timeModel);
				model1 = transpose(mul(timeModel, scale4x4(float3(0.09, 0.25, 0.02f))));
				lambert.SetUniform("model", model1); GL_CHECK_ERRORS;

				glDrawElements(GL_TRIANGLES, planeWings, GL_UNSIGNED_INT, nullptr); GL_CHECK_ERRORS;
			}
			// рисуем пропеллер
			{
				float4x4 model1;
				float4x4 timeModel = mul(translate4x4(float3(4.0f, -2.0f, 0.15f)), rotate_Z_4x4(move * 30));
				timeModel = mul(rotate_Y_4x4(-angle * move), timeModel);
				model1 = transpose(mul(timeModel, scale4x4(float3(0.09, 0.25, 0.02f))));
				lambert.SetUniform("model", model1); GL_CHECK_ERRORS;
				glDrawElements(GL_TRIANGLE_STRIP, planeWings, GL_UNSIGNED_INT, nullptr); GL_CHECK_ERRORS;
			}
			// Рисуем крылья
			{
				float4x4 model1;
				float4x4 timeModel = mul(rotate_Y_4x4(-angle * move), translate4x4(float3(3.5f, -2.0f, 0.0f)));
				model1 = transpose(mul(timeModel, scale4x4(float3(1.5f, 0.05f, 0.3f))));
				lambert.SetUniform("model", model1); GL_CHECK_ERRORS;

				glDrawElements(GL_TRIANGLES, planeWings, GL_UNSIGNED_INT, nullptr); GL_CHECK_ERRORS;
			}
			// Рисуем тело
			{
				float4x4 model1;
				float4x4 timeModel = mul(rotate_Y_4x4(-angle * move), translate4x4(float3(3.5f, -2.0f, 0.0f)));
				model1 = transpose(mul(timeModel, scale4x4(float3(0.3, 0.2, 1.2f))));
				lambert.SetUniform("model", model1); GL_CHECK_ERRORS;

				glDrawElements(GL_TRIANGLE_STRIP, planeWings, GL_UNSIGNED_INT, nullptr); GL_CHECK_ERRORS;
			}
		}

		if (keys[GLFW_KEY_E]) {
			glBindVertexArray(vaoInts); GL_CHECK_ERRORS;
			{
				lambert.SetUniform("material.light", 0);
				lambert.SetUniform("type", 2);
				float4x4 model1 = transpose(mul(model, scale4x4(float3(0.01f, 0.01f, 0.01f))));
				lambert.SetUniform("model", model1); GL_CHECK_ERRORS;
				glDrawElementsInstanced(GL_TRIANGLES, ints, GL_UNSIGNED_INT, 0, abs(int(sin(move) * 1000000))); GL_CHECK_ERRORS;

			}

		}

		if (keys[GLFW_KEY_R]) {
			glBindVertexArray(vaoCave); GL_CHECK_ERRORS;
			{
				lambert.SetUniform("useNormalMap", false);
				lambert.SetUniform("material.light", 0);
				for (int i = 0; i < 16; i++) {
					float4x4 model1;
					if (i < 8) {
						model1 = mul(model, translate4x4(float3(i, -1.5f, 3.0f)));
						model1 = mul(model1, rotate_X_4x4(-90 * LiteMath::DEG_TO_RAD));
						model1 = transpose(mul(model1, scale4x4(float3(0.1f, 0.1f, 0.1f))));
					}
					else {
						model1 = mul(model, translate4x4(float3(i - 8, -1.5f, 5.6f)));
						model1 = mul(model1, rotate_Z_4x4(-180 * LiteMath::DEG_TO_RAD));
						model1 = mul(model1, rotate_X_4x4(90 * LiteMath::DEG_TO_RAD));
						model1 = transpose(mul(model1, scale4x4(float3(0.1f, 0.1f, 0.1f))));
					}
					lambert.SetUniform("model", model1); GL_CHECK_ERRORS;
					glDrawElements(GL_TRIANGLES, Cave, GL_UNSIGNED_INT, nullptr); GL_CHECK_ERRORS;
				}


				float4x4 model1 = mul(model, translate4x4(float3(8, 0.8f, 4.3f)));
				model1 = mul(model1, rotate_Z_4x4(-90 * LiteMath::DEG_TO_RAD));
				model1 = mul(model1, rotate_Y_4x4(90 * LiteMath::DEG_TO_RAD));
				model1 = transpose(mul(model1, scale4x4(float3(0.12f, 0.1f, 0.1f))));
				lambert.SetUniform("model", model1); GL_CHECK_ERRORS;
				glDrawElements(GL_TRIANGLES, Cave, GL_UNSIGNED_INT, nullptr); GL_CHECK_ERRORS;
			}

			glActiveTexture(GL_TEXTURE6);
			glBindTexture(GL_TEXTURE_2D, dungeonRoofTex);
			lambert.SetUniform("useNormalMap", true);

			glBindVertexArray(dungeonRoofVao); GL_CHECK_ERRORS;
			{
				float4x4 model1;
				for (int i = 0; i < 8; i++) {
					if (i == 4) {
						glActiveTexture(GL_TEXTURE6);
						glBindTexture(GL_TEXTURE_2D, groundTex);
					}
					if (i < 4) {
						model1 = mul(model, translate4x4(float3(i * 2, 0.0f, 0.0f)));
						model1 = mul(model1, translate4x4(float3(0.63f + i, 0.56f, 2.8f)));
					}
					else {
						model1 = mul(model, translate4x4(float3((i - 4) * 2, 0.0f, 0.0f)));
						model1 = mul(model1, translate4x4(float3(0.63f + i - 4, -1.344f, 2.8f)));
					}
					model1 = mul(model1, rotate_Z_4x4(90 * LiteMath::DEG_TO_RAD));
					model1 = transpose(mul(model1, scale4x4(float3(0.004f, 0.06f, 0.06f))));
					lambert.SetUniform("model", model1); GL_CHECK_ERRORS;
					glDrawElements(GL_TRIANGLES, dungeonRoofIn, GL_UNSIGNED_INT, nullptr); GL_CHECK_ERRORS;
				}
			}

			glActiveTexture(GL_TEXTURE6);
			glBindTexture(GL_TEXTURE_2D, conteinerTex);

			glBindVertexArray(conteinerVao); GL_CHECK_ERRORS;
			{
				float4x4 model1;
				for (int i = 0; i < 4; i++) {
					if (i % 2 == 0) {
						model1 = mul(model, translate4x4(float3(i, -1.0f, 3.0f)));
					}
					else {
						model1 = mul(model, translate4x4(float3(i + 1, -1.0f, 5.1f)));
					}
					model1 = transpose(mul(model1, scale4x4(float3(0.01f, 0.01f, 0.01f))));
					lambert.SetUniform("model", model1); GL_CHECK_ERRORS;
					glDrawElements(GL_TRIANGLES, conteinerIn, GL_UNSIGNED_INT, nullptr); GL_CHECK_ERRORS;
				}

				model1 = mul(model, translate4x4(float3(5, -1.0f, 5.1f)));
				model1 = transpose(mul(model1, scale4x4(float3(0.01f, 0.01f, 0.01f))));
				lambert.SetUniform("model", model1); GL_CHECK_ERRORS;
				glDrawElements(GL_TRIANGLES, conteinerIn, GL_UNSIGNED_INT, nullptr); GL_CHECK_ERRORS;

				model1 = mul(model, translate4x4(float3(7.65f, -1.0f, 3.0f)));
				model1 = transpose(mul(model1, scale4x4(float3(0.01f, 0.01f, 0.01f))));
				lambert.SetUniform("model", model1); GL_CHECK_ERRORS;
				glDrawElements(GL_TRIANGLES, conteinerIn, GL_UNSIGNED_INT, nullptr); GL_CHECK_ERRORS;

				model1 = mul(model, translate4x4(float3(7.65f, -1.0f, 4.0f)));
				model1 = transpose(mul(model1, scale4x4(float3(0.01f, 0.01f, 0.01f))));
				lambert.SetUniform("model", model1); GL_CHECK_ERRORS;
				glDrawElements(GL_TRIANGLES, conteinerIn, GL_UNSIGNED_INT, nullptr); GL_CHECK_ERRORS;
			}

			//glActiveTexture(GL_TEXTURE6);
			//glBindTexture(GL_TEXTURE_2D, chestTex);
			lambert.SetUniform("useNormalMap", false);
			glBindVertexArray(chestVao); GL_CHECK_ERRORS;
			{
				float4x4 model1;
				model1 = mul(model, translate4x4(float3(5.2f, -1.2f, 3.3f)));
				model1 = mul(model1, rotate_Y_4x4(-90 * LiteMath::DEG_TO_RAD));
				model1 = transpose(mul(model1, scale4x4(float3(0.005f, 0.005f, 0.005f))));
				lambert.SetUniform("model", model1); GL_CHECK_ERRORS;
				glDrawElements(GL_TRIANGLES, chestIn, GL_UNSIGNED_INT, nullptr); GL_CHECK_ERRORS;
			}

			glBindVertexArray(chestVao); GL_CHECK_ERRORS;
			{
				float4x4 model1;
				model1 = mul(model, translate4x4(float3(3.2f, -1.2f, 3.3f)));
				model1 = mul(model1, rotate_Y_4x4(-90 * LiteMath::DEG_TO_RAD));
				model1 = transpose(mul(model1, scale4x4(float3(0.005f, 0.005f, 0.005f))));
				lambert.SetUniform("model", model1); GL_CHECK_ERRORS;
				glDrawElements(GL_TRIANGLES, chestIn, GL_UNSIGNED_INT, nullptr); GL_CHECK_ERRORS;
			}

			float pos = 0;
			for (int i = 0; i < 100; i++) {
				if (abs(camera.pos.x - pos - 2.5) > abs(camera.pos.x - pos) && abs(camera.pos.x - pos) < abs(camera.pos.x - pos + 2.5)) {
					break;
				}
				pos += 2.5;
			}

			for (int i = 0; i < 6; i++) {
				std::string index = std::to_string(i);
				lambert.SetUniform("pointLights[" + index + "].position", float3(pos, -0.0f, i % 2 == 0 ? 3.2f : 5.4f)); GL_CHECK_ERRORS;
				lambert.SetUniform("pointLights[" + index + "].ambient", float3(0.55f, 0.55f, 0.55f)); GL_CHECK_ERRORS;
				lambert.SetUniform("pointLights[" + index + "].diffuse", float3(0.8f, 0.8f, 0.8f)); GL_CHECK_ERRORS;
				lambert.SetUniform("pointLights[" + index + "].specular", float3(1.0f, 1.0f, 1.0f)); GL_CHECK_ERRORS;
				lambert.SetUniform("pointLights[" + index + "].constant", 1.0f); GL_CHECK_ERRORS;
				lambert.SetUniform("pointLights[" + index + "].linear", 0.09f); GL_CHECK_ERRORS;
				lambert.SetUniform("pointLights[" + index + "].quadratic", 0.632f); GL_CHECK_ERRORS;
			}
		}

		lambert.StopUseShader(); GL_CHECK_ERRORS;


		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT);
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // set clear color to white (not really necessery actually, since we won't be able to see behind the quad anyways)
		glDisable(GL_DEPTH_TEST);


		tunnel.StartUseShader();
		glBindVertexArray(quadVao);
		glActiveTexture(GL_TEXTURE15);
		glBindTexture(GL_TEXTURE_2D, colorBuffer);    // use the color attachment texture as the texture of the quad plane

		tunnel.SetUniform("hdr", 0);
		tunnel.SetUniform("exposure", 1.0f);
		tunnel.SetUniform("gamma", 1.0f);

		glDrawArrays(GL_TRIANGLES, 0, quad); GL_CHECK_ERRORS;

		tunnel.StopUseShader(); GL_CHECK_ERRORS;

		glfwSwapBuffers(window);

		move += 0.03f;
	}

	glfwTerminate();
	return 0;
}
