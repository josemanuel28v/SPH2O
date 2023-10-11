#include "InputManager.h"
#include "GLFWInputManager.h"
#include "Material.h"
#include "GLSLMaterial.h"
#include "Render.h"
#include "GL1Render.h"
#include "GL4Render.h"
#include "Texture.h"
#include "GLTexture.h"

class FactoryEngine 
{
public:

	enum InputBackend : int 
	{
		GLFW = 0
	};

	enum GraphicsBackend : int
	{
		GL1 = 0,
		GL4 = 1
	};

	static int selectedGraphicsBackend;
	static int selectedInputBackend;

	static void SetSelectedGraphicsBackend(int backend);
	static void SetSelectedInputBackend(int backend);
	static int GetSelectedGraphicsBackend();
	static int GetSelectedInputBackend();

	static Render* getNewRender();
	static InputManager* getNewInputManager();
	static Material* getNewMaterial();
	static Texture* getNewTexture();

	static bool isClosed();
};
 