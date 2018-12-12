#include "OSWindow.h"
#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "imgui.h"
#include "imgui_impl_glfw_gl3.h"
#include "Importer.h"
#include "SceneManager.h"

#include <iostream>
#include <thread>

namespace OSWindow
{
	static GLFWwindow* handle = nullptr;
	static glm::ivec2 size{1920, 900};
	static bool mouseDrag = false;
	static double lastFrame = 0;
	static double frameDelta = 0;
	namespace callback
	{
		static void framebufferResized(GLFWwindow* window, int width, int height);
		static void cursorMoved(GLFWwindow* window, double xpos, double ypos);
		static void mouseButtonPressed(GLFWwindow* window, int button, int mode, int modifier);
		static void keyPressed(GLFWwindow* window, int key, int keycode, int mode, int modifier);
		static void filesDropped(GLFWwindow* window, int count, const char** paths);
		static void APIENTRY debug(GLenum source, GLenum type, GLuint id, GLenum severity,
			GLsizei length, const GLchar *message, const void* userParam);
	}
	static void processInput();
}

void OSWindow::init()
{
	if(!glfwInit())
	{
		std::cerr << "Failed to initialize GLFW\n";
		std::terminate();
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifndef NDEBUG
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#endif
	handle = glfwCreateWindow(size.x, size.y, "LPCRenderer", nullptr, nullptr);
	if(handle == nullptr)
	{
		std::cerr << "Failed to create GLFW window\n";
		std::terminate();
	}
	glfwMakeContextCurrent(handle);

	if(!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
	{
		std::cerr << "Failed to initialize GLAD\n";
		std::terminate();
	}
#ifndef NDEBUG
	GLint flags;
	glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
	if(flags & GL_CONTEXT_FLAG_DEBUG_BIT)
	{
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(callback::debug, nullptr);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
	}
#endif

	//glEnable(GL_FRAMEBUFFER_SRGB);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glfwSetFramebufferSizeCallback(handle, callback::framebufferResized);
	glfwSetCursorPosCallback(handle, callback::cursorMoved);
	glfwSetMouseButtonCallback(handle, callback::mouseButtonPressed);
	glfwSetKeyCallback(handle, callback::keyPressed);
	glfwSetScrollCallback(handle, ImGui_ImplGlfw_ScrollCallback);
	glfwSetCharCallback(handle, ImGui_ImplGlfw_CharCallback);
	glfwSetDropCallback(handle, callback::filesDropped);

	auto guiContext = ImGui::CreateContext();
	if(!ImGui_ImplGlfwGL3_Init(handle, false) || !guiContext)
	{
		std::cerr << "Failed to initialize ImGui context\n";
		std::terminate();
	}

	ImGui::StyleColorsDark();
	ImGui::GetStyle().WindowRounding = 0.0f;
	ImGui::GetStyle().WindowBorderSize = 0.0f;
	ImGui::GetStyle().PopupRounding = 0.0f;
	ImGui::GetStyle().ScrollbarRounding = 0.0f;
}

glm::ivec2 OSWindow::getSize()
{
	return size;
}

float OSWindow::getAspectRatio()
{
	return float(size.x) / size.y;
}

void OSWindow::resize(glm::ivec2 newSize)
{
	size = newSize;
	glViewport(0, 0, size.x, size.y);
}

void OSWindow::beginFrame()
{
	while(glfwPollEvents(),
		//!glfwGetWindowAttrib(handle, GLFW_FOCUSED)
		 size.x * size.y == 0)
	{
		std::this_thread::yield();
	}
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	processInput();
}

void OSWindow::endFrame()
{
	glfwSwapBuffers(handle);
	double currentFrame = glfwGetTime();
	frameDelta = currentFrame - lastFrame;
	lastFrame = currentFrame;
}

bool OSWindow::shouldClose()
{
	return glfwWindowShouldClose(handle);
}

void OSWindow::destroy()
{
	ImGui_ImplGlfwGL3_Shutdown();
	glfwTerminate();
}

void OSWindow::processInput()
{
	if(!SceneManager::getActive())
		return;
	float distance = 2.5f * frameDelta;
	glm::vec3 direction{0.0f};
	if(glfwGetKey(handle, GLFW_KEY_W) == GLFW_PRESS)
		direction.z += 1.0f;
	if(glfwGetKey(handle, GLFW_KEY_S) == GLFW_PRESS)
		direction.z -= 1.0f;
	if(glfwGetKey(handle, GLFW_KEY_D) == GLFW_PRESS)
		direction.x += 1.0f;
	if(glfwGetKey(handle, GLFW_KEY_A) == GLFW_PRESS)
		direction.x -= 1.0f;
	if(glfwGetKey(handle, GLFW_KEY_E) == GLFW_PRESS)
		direction.y += 1.0f;
	if(glfwGetKey(handle, GLFW_KEY_Q) == GLFW_PRESS)
		direction.y -= 1.0f;
	if(direction != glm::vec3{0.0f})
		SceneManager::getActive()->getCamera().move(direction * distance);
}

void OSWindow::callback::framebufferResized(GLFWwindow* window, int width, int height)
{
	OSWindow::resize({width, height});
}

void OSWindow::callback::cursorMoved(GLFWwindow* window, double xpos, double ypos)
{
	static glm::dvec2 lastMouse = size / 2;
	static bool firstMouse = true;
	double xoffset = xpos - lastMouse.x;
	double yoffset = ypos - lastMouse.y;
	lastMouse.x = xpos;
	lastMouse.y = ypos;
	if(!mouseDrag || firstMouse)
	{
		firstMouse = false;
		return;
	}
	float sensitivity = 0.05f;
	xoffset *= -sensitivity;
	yoffset *= -sensitivity;
	if(SceneManager::getActive())
	{
		SceneManager::getActive()->getCamera().rotate({yoffset, xoffset});
	}
}

void OSWindow::callback::mouseButtonPressed(GLFWwindow* window, int button, int mode, int modifier)
{
	if(ImGuiIO& io = ImGui::GetIO(); io.WantCaptureMouse)
		return ImGui_ImplGlfw_MouseButtonCallback(window, button, mode, modifier);

	if(button == GLFW_MOUSE_BUTTON_1)
	{
		if(mode == GLFW_PRESS)
		{
			mouseDrag = true;
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		}
		else
		{
			mouseDrag = false;
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}
	}
}

void OSWindow::callback::keyPressed(GLFWwindow* window, int key, int keycode, int mode, int modifier)
{
	if(key == GLFW_KEY_ESCAPE && mode == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, true);
		return;
	}
	return ImGui_ImplGlfw_KeyCallback(window, key, keycode, mode, modifier);
}

void OSWindow::callback::filesDropped(GLFWwindow* window, int count, const char** paths)
{
	std::vector<std::filesystem::path> filenames;
	for(int i = 0; i < count; i++)
		 filenames.emplace_back(paths[i]);
	Importer::import(filenames);
}

void APIENTRY OSWindow::callback::debug(GLenum source, GLenum type, GLuint id, GLenum severity,
	GLsizei length, const GLchar *message, const void* userParam)
{
	if(type == GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR || severity == GL_DEBUG_SEVERITY_LOW)
		return;
	std::cout << "-----------------------------------\n"
		<< "OpenGL Debug Message (" << id << "): \n" << message << '\n';
	std::cout << "Source: ";
	switch(source)
	{
		case GL_DEBUG_SOURCE_API:
			std::cout << "API";
			break;
		case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
			std::cout << "Window System";
			break;
		case GL_DEBUG_SOURCE_SHADER_COMPILER:
			std::cout << "Shader Compiler";
			break;
		case GL_DEBUG_SOURCE_THIRD_PARTY:
			std::cout << "Third Party";
			break;
		case GL_DEBUG_SOURCE_APPLICATION:
			std::cout << "Application";
			break;
		default:
			std::cout << "Other";
			break;
	}
	std::cout << "\nType: ";
	switch(type)
	{
		case GL_DEBUG_TYPE_ERROR:
			std::cout << "Error";
			break;
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
			std::cout << "Deprecated Behaviour";
			break;
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
			std::cout << "Undefined Behaviour";
			break;
		case GL_DEBUG_TYPE_PORTABILITY:
			std::cout << "Portability";
			break;
		case GL_DEBUG_TYPE_PERFORMANCE:
			std::cout << "Performance";
			break;
		case GL_DEBUG_TYPE_MARKER:
			std::cout << "Marker";
			break;
		case GL_DEBUG_TYPE_PUSH_GROUP:
			std::cout << "Push Group";
			break;
		case GL_DEBUG_TYPE_POP_GROUP:
			std::cout << "Pop Group";
			break;
		default:
			std::cout << "Other";
			break;
	}
	std::cout << "\nSeverity: ";
	switch(severity)
	{
		case GL_DEBUG_SEVERITY_HIGH:
			std::cout << "High";
			break;
		case GL_DEBUG_SEVERITY_MEDIUM:
			std::cout << "Medium";
			break;
		case GL_DEBUG_SEVERITY_LOW:
			std::cout << "Low";
			break;
		case GL_DEBUG_SEVERITY_NOTIFICATION:
			std::cout << "Notification";
			break;
	}
	std::cout << '\n';
}
