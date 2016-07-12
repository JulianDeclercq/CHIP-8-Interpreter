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
GLuint LoadShaders(const char * vertex_file_path, const char * fragment_file_path);
unsigned int RgbaToU32(unsigned char r, unsigned char g, unsigned char b, unsigned char a);
std::map<int, unsigned char>  InitialiseKeyMapping();

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

int main()
{
	srand(static_cast<unsigned int>(time(0))); //seed rand
	rand(); rand(); rand();

	GLFWwindow* window = OpenGLInit("CHIP8_Interpreter by Julian Declercq");

	Interpreter interpreter = Interpreter(InitialiseKeyMapping());
	interpreter.LoadRom("./Resources/PONG");

	// Game loop
	bool interpreterRunning = true;
	while (!glfwWindowShouldClose(window))
	{
		if (interpreterRunning)
		{
			for (int i = 0; i < 5; i++)
			{
				//interpreterRunning = interpreter.Cycle();
				if (!interpreterRunning)
					break; //out of for loop
			}
		}

		Draw(window, interpreter);
		// Check if any events have been activated (key pressed, mouse moved etc.) and call corresponding response functions
		glfwPollEvents();

		//interpreter.SetKeys();
	}

	// Terminates GLFW, clearing any resources allocated by GLFW.
	glfwTerminate();

	std::cin.get();
	return 0;
}

// Is called whenever a key is pressed/released via GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	UNREFERENCED_PARAMETER(mode);
	UNREFERENCED_PARAMETER(scancode);

	std::cout << key << std::endl;
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
}

std::map<int, unsigned char>  InitialiseKeyMapping()
{
	/* Original Keypad		will map to			OpenGl keycodes
	+-+-+-+-+				+-+-+-+-+			+-+-+-+-+
	|1|2|3|C|				|1|2|3|4|			|49|50|51|52|
	+-+-+-+-+				+-+-+-+-+			+-+-+-+-+
	|4|5|6|D|				|Q|W|E|R|			|81|87|69|82|
	+-+-+-+-+       =>		+-+-+-+-+	  =>	+-+-+-+-+
	|7|8|9|E|				|A|S|D|F|			|65|83|68|70|
	+-+-+-+-+				+-+-+-+-+			+-+-+-+-+
	|A|0|B|F|				|Z|X|C|V|			|90|88|67|86|
	+-+-+-+-+				+-+-+-+-+			+-+-+-+-+		*/

	std::map<int, unsigned char> keypad;
	keypad.insert(std::make_pair(49, 1));
	keypad.insert(std::make_pair(50, 2));
	keypad.insert(std::make_pair(51, 3));
	keypad.insert(std::make_pair(52, 0xC));

	keypad.insert(std::make_pair(81, 4));
	keypad.insert(std::make_pair(87, 5));
	keypad.insert(std::make_pair(69, 6));
	keypad.insert(std::make_pair(82, 0xD));

	keypad.insert(std::make_pair(65, 7));
	keypad.insert(std::make_pair(83, 8));
	keypad.insert(std::make_pair(68, 9));
	keypad.insert(std::make_pair(70, 0xE));

	keypad.insert(std::make_pair(90, 0xA));
	keypad.insert(std::make_pair(88, 0));
	keypad.insert(std::make_pair(67, 0xB));
	keypad.insert(std::make_pair(86, 0xF));

	return keypad;
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