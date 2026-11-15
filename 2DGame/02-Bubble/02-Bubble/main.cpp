#include "GLIncludes.h"
#include <chrono>
#include <thread>
#include <cstdlib>
#include <ctime>
#include "Game.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

#define TARGET_FRAMERATE 60.0f

#ifdef __EMSCRIPTEN__
static GLFWwindow *window = NULL;
static double timePreviousFrame = 0.0;

static int keyFromDomCode(unsigned int code)
{
	if ((code >= 48 && code <= 57) || (code >= 65 && code <= 90))
		return int(code);
	if (code >= 112 && code <= 123)
		return GLFW_KEY_F1 + int(code - 112);
	if (code >= 96 && code <= 105)
		return GLFW_KEY_KP_0 + int(code - 96);

	switch (code)
	{
	case 8: return GLFW_KEY_BACKSPACE;
	case 9: return GLFW_KEY_TAB;
	case 13: return GLFW_KEY_ENTER;
	case 16: return GLFW_KEY_LEFT_SHIFT;
	case 17: return GLFW_KEY_LEFT_CONTROL;
	case 18: return GLFW_KEY_LEFT_ALT;
	case 19: return GLFW_KEY_PAUSE;
	case 20: return GLFW_KEY_CAPS_LOCK;
	case 27: return GLFW_KEY_ESCAPE;
	case 32: return GLFW_KEY_SPACE;
	case 33: return GLFW_KEY_PAGE_UP;
	case 34: return GLFW_KEY_PAGE_DOWN;
	case 35: return GLFW_KEY_END;
	case 36: return GLFW_KEY_HOME;
	case 37: return GLFW_KEY_LEFT;
	case 38: return GLFW_KEY_UP;
	case 39: return GLFW_KEY_RIGHT;
	case 40: return GLFW_KEY_DOWN;
	case 44: return GLFW_KEY_PRINT_SCREEN;
	case 45: return GLFW_KEY_INSERT;
	case 46: return GLFW_KEY_DELETE;
	case 106: return GLFW_KEY_KP_MULTIPLY;
	case 107: return GLFW_KEY_KP_ADD;
	case 109: return GLFW_KEY_KP_SUBTRACT;
	case 110: return GLFW_KEY_KP_DECIMAL;
	case 111: return GLFW_KEY_KP_DIVIDE;
	case 144: return GLFW_KEY_NUM_LOCK;
	case 145: return GLFW_KEY_SCROLL_LOCK;
	case 186: return GLFW_KEY_SEMICOLON;
	case 187: return GLFW_KEY_EQUAL;
	case 188: return GLFW_KEY_COMMA;
	case 189: return GLFW_KEY_MINUS;
	case 190: return GLFW_KEY_PERIOD;
	case 191: return GLFW_KEY_SLASH;
	case 192: return GLFW_KEY_GRAVE_ACCENT;
	case 219: return GLFW_KEY_LEFT_BRACKET;
	case 220: return GLFW_KEY_BACKSLASH;
	case 221: return GLFW_KEY_RIGHT_BRACKET;
	case 222: return GLFW_KEY_APOSTROPHE;
	default: return GLFW_KEY_UNKNOWN;
	}
}

static EM_BOOL webKeyCallback(int eventType, const EmscriptenKeyboardEvent *event, void *)
{
	int key = keyFromDomCode(event->keyCode);
	if (key == GLFW_KEY_UNKNOWN)
		return EM_FALSE;
	if (eventType == EMSCRIPTEN_EVENT_KEYDOWN)
		Game::instance().keyPressed(key);
	else
		Game::instance().keyReleased(key);
	return EM_TRUE;
}

static void mainLoop()
{
	double currentTime = glfwGetTime();
	double deltaTime = currentTime - timePreviousFrame;
	timePreviousFrame = currentTime;
	if (!Game::instance().update(int(1000.0 * deltaTime)))
	{
		emscripten_cancel_main_loop();
		glfwTerminate();
		return;
	}
	Game::instance().render();
	glfwSwapBuffers(window);
	glfwPollEvents();
}
#endif


void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	(void)window;
	(void)scancode;
	(void)mods;
	if (action == GLFW_PRESS)
		Game::instance().keyPressed(key);
	else if (action == GLFW_RELEASE)
		Game::instance().keyReleased(key);
}

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
	(void)window;
	Game::instance().mouseMove(int(xpos), int(ypos));
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	(void)window;
	(void)mods;
	if (action == GLFW_PRESS)
		Game::instance().mousePress(button);
	else if (action == GLFW_RELEASE)
		Game::instance().mouseRelease(button);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	(void)window;
	glViewport(0, 0, width, height);
}


int main(void)
{
#ifndef __EMSCRIPTEN__
	GLFWwindow* window;
	double timePerFrame = 1.f / TARGET_FRAMERATE, timePreviousFrame, currentTime;
#endif

	/* Initialize the library */
	if (!glfwInit())
		return -1;

#ifdef __EMSCRIPTEN__
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
#endif

	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Bomberman", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	/* Set window initial position */
	glfwSetWindowPos(window, 100, 100);
	/* Make the window's context current */
	glfwMakeContextCurrent(window);
	/* Enable VSync to eliminate CPU spinning and screen tearing */
	glfwSwapInterval(1);

	/* Set callbacks */
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
#ifdef __EMSCRIPTEN__
	emscripten_set_keydown_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, NULL, EM_TRUE, webKeyCallback);
	emscripten_set_keyup_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, NULL, EM_TRUE, webKeyCallback);
#endif

	/* Init GLEW to have access to GL extensions */
#ifndef __EMSCRIPTEN__
	glewExperimental = GL_TRUE;
	glewInit();
#endif

	/* Match viewport to framebuffer (handles HiDPI / initial size) */
	int fbWidth, fbHeight;
	glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
	glViewport(0, 0, fbWidth, fbHeight);

	/* Init random seed */
	srand((unsigned int)time(NULL));

	/* Init step of the game loop */
	Game::instance().init();
	timePreviousFrame = glfwGetTime();

#ifdef __EMSCRIPTEN__
	emscripten_set_main_loop(mainLoop, 0, 1);
	return 0;
#else
	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
		currentTime = glfwGetTime();
		if (currentTime - timePreviousFrame >= timePerFrame)
		{
			/* Update & render steps of the game loop */
			if(!Game::instance().update(int(1000.0f * (currentTime - timePreviousFrame))))
				glfwSetWindowShouldClose(window, GLFW_TRUE);
			Game::instance().render();
			timePreviousFrame = currentTime;

			/* Swap front and back buffers */
			glfwSwapBuffers(window);
		}
		else
		{
			// Yield/sleep slightly if there is surplus time before the next frame
			double remainingTime = timePerFrame - (currentTime - timePreviousFrame);
			if (remainingTime > 0.002)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
			}
		}

		/* Poll for and process events */
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
#endif
}
