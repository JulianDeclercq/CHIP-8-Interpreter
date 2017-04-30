// GLAD
#include <glad/glad.h>
// GLFW
#include <GLFW/glfw3.h>

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <time.h> //srand
#include <map>

#include "Interpreter.h"

//Forward declaration
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
GLuint LoadShaderFromFile(const std::string & filePath, GLenum shaderType);
unsigned int RgbaToU32(unsigned char r, unsigned char g, unsigned char b, unsigned char a);
void InitialiseKeyMapping(std::map<int, unsigned short>& keyMap);
void SetInput(GLFWwindow* window);

// Window dimensions
const GLuint WIDTH = 1024, HEIGHT = 512;

GLFWwindow* OpenGLInit(const std::string& windowName)
{
	// Init GLFW
	glfwInit();

	// Set all the required options for GLFW
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	// Create a GLFWwindow object that we can use for GLFW's functions
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, windowName.c_str(), NULL, NULL);
	glfwMakeContextCurrent(window);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return nullptr;
	}

	// Set the required callback functions
	glfwSetKeyCallback(window, key_callback);

	if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
	{
		std::cout << "Failed to initialize OpenGL context" << std::endl;
		return nullptr;
	}

	// Define the viewport dimensions
	glViewport(0, 0, WIDTH, HEIGHT);

	//Create vertex array object
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	//vertex buffer obj
	GLuint vbo = 0;
	glGenBuffers(1, &vbo);

	GLfloat points[] =
	{
		-1, 1, 0.0f, 0.0f, // lt
		1, 1, 1.0f, 0.0f, // rt
		1, -1, 1.0f, 1.0f, // rb
		-1, -1, 0.0f, 1.0f  // lb
	};

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_STATIC_DRAW);

	//element array
	GLuint ebo;
	glGenBuffers(1, &ebo);

	GLuint elements[] =
	{
		0, 1, 2, 2, 3, 0
	};

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);

	//Create and compile shaders
	GLuint vs = LoadShaderFromFile("./basic_vertex.glsl", GL_VERTEX_SHADER);
	GLuint fs = LoadShaderFromFile("./basic_fragment.glsl", GL_FRAGMENT_SHADER);

	//Combine compiled shaders into 1 GPU program
	GLuint shader_programme = glCreateProgram();
	glAttachShader(shader_programme, fs);
	glAttachShader(shader_programme, vs);
	glBindFragDataLocation(shader_programme, 0, "outColor");
	glLinkProgram(shader_programme);
	glUseProgram(shader_programme);

	// Specify the layout of the vertex data
	GLint posAttrib = glGetAttribLocation(shader_programme, "position");
	glEnableVertexAttribArray(posAttrib);
	glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);

	GLint texCoordAttrib = glGetAttribLocation(shader_programme, "texCoord");
	glEnableVertexAttribArray(texCoordAttrib);
	glVertexAttribPointer(texCoordAttrib, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));

	glfwSwapInterval(1);

	//Texture parameters
	GLuint texture_id;
	glGenTextures(1, &texture_id);
	glBindTexture(GL_TEXTURE_2D, texture_id);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);  //Always set the base and max mipmap levels of a texture.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	return window;
}

void Draw(GLFWwindow* window, Interpreter& interpreter)
{
	//Get the texture from the interpreter
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 64, 32, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, reinterpret_cast<GLvoid*>(interpreter.GetScreen()));

	// Clear the screen to white
	glClearColor(1, 1, 1, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	// Draw the rectangle for the texture
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	// Swap the screen buffers
	glfwSwapBuffers(window);
}

std::map<int, unsigned short> m_KeyMap = std::map<int, unsigned short>();
Interpreter* m_Interpreter = nullptr;
int main()
{
	srand(static_cast<unsigned int>(time(0))); //seed rand
	rand(); rand(); rand();

	GLFWwindow* window = OpenGLInit("CHIP8_Interpreter by Julian Declercq");

	m_Interpreter = new Interpreter();
	m_Interpreter->Initialize();
	m_Interpreter->LoadRom("./Resources/15PUZZLE");

	InitialiseKeyMapping(m_KeyMap);

	// Game loop
	while (!glfwWindowShouldClose(window))
	{
		// Check if any events have been activated (key pressed, mouse moved etc.) and call corresponding response functions
		glfwPollEvents();

		// Every cycle you should check the key input state and store it in the interpreters keypad.
		SetInput(window);

		// Cycle the interpreter
		if (!m_Interpreter->Cycle())
			break;

		if (m_Interpreter->m_DrawFlag)
			Draw(window, *m_Interpreter);

		// First clear the previous cycle's key information
		m_Interpreter->m_Keypad = 0;
	}

	// Terminates GLFW, clearing any resources allocated by GLFW.
	glfwTerminate();

	return 0;
}

// Is called whenever a key is pressed/released via GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	UNREFERENCED_PARAMETER(mode);
	UNREFERENCED_PARAMETER(scancode);

	std::cout << "Key pressed: " << key << std::endl;
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
}

void InitialiseKeyMapping(std::map<int, unsigned short>& keyMap)
{
	/* Original Keypad		will map to
	+-+-+-+-+				+-+-+-+-+
	|1|2|3|C|				|1|2|3|4|
	+-+-+-+-+				+-+-+-+-+
	|4|5|6|D|				|Q|W|E|R|
	+-+-+-+-+       =>		+-+-+-+-+
	|7|8|9|E|				|A|S|D|F|
	+-+-+-+-+				+-+-+-+-+
	|A|0|B|F|				|Z|X|C|V|
	+-+-+-+-+				+-+-+-+-+	*/

	keyMap =
	{
		{ GLFW_KEY_1, 1 }, { GLFW_KEY_2, 2 }, { GLFW_KEY_3, 3 }, { GLFW_KEY_4, 0xC },
		{ GLFW_KEY_Q, 4 }, { GLFW_KEY_W, 5 }, { GLFW_KEY_E, 6 }, { GLFW_KEY_R, 0xD },
		{ GLFW_KEY_A, 7 }, { GLFW_KEY_S, 8 }, { GLFW_KEY_D, 9 }, { GLFW_KEY_F, 0xE },
		{ GLFW_KEY_Z, 0xA }, { GLFW_KEY_X, 0 }, { GLFW_KEY_C, 0xB }, { GLFW_KEY_V, 0xF }
	};
}

void SetInput(GLFWwindow* window)
{
	// Check state for all keybinds and update the interpreter
	for (const std::pair<int, unsigned short>& keybind : m_KeyMap)
	{
		if (glfwGetKey(window, keybind.first) == GLFW_PRESS)
			m_Interpreter->m_Keypad |= 1 << keybind.second;
	}
}

GLuint LoadShaderFromFile(const std::string& filePath, GLenum shaderType) {
	GLuint shaderID = 0;
	std::string shaderString;
	std::ifstream sourceFile(filePath.c_str());

	if (sourceFile)
	{
		//Get shader source
		shaderString.assign((std::istreambuf_iterator< char >(sourceFile)), std::istreambuf_iterator< char >());

		//Create shader ID
		shaderID = glCreateShader(shaderType);

		//Set shader source
		const GLchar* shaderSource = shaderString.c_str();
		glShaderSource(shaderID, 1, (const GLchar**)&shaderSource, NULL);

		//Compile shader source
		glCompileShader(shaderID);

		//Check shader for errors
		GLint shaderCompiled = GL_FALSE;
		glGetShaderiv(shaderID, GL_COMPILE_STATUS, &shaderCompiled);
		if (shaderCompiled != GL_TRUE)
		{
			std::clog << "Unable to compile shader " << shaderCompiled << "!\n\nSource:\n" << shaderSource << std::endl;
			glDeleteShader(shaderID);
			shaderID = 0;
		}
	}
	else
	{
		printf("Unable to open file %s\n", filePath.c_str());
	}

	return shaderID;
}