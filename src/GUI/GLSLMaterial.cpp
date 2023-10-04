#include "GLSLMaterial.h"
#include "GLTexture.h"
#include "System.h"

using renderType = RenderProgram::renderTypes_e;

GLSLMaterial::GLSLMaterial()
{
	this->program = new GLSLShader();
	this->colorMap = nullptr;
	this->normalMap = nullptr;
	color = glm::vec4(1.0, 1.0, 1.0, 1.0);
}

void GLSLMaterial::loadPrograms(std::vector<std::string> fileNames)
{
	for (const auto& file : fileNames) 
	{
		if (file.ends_with("vert"))
		{
			this->program->setProgram(file, renderType::vertex);
		}
		else if (file.ends_with("frag"))
		{
			this->program->setProgram(file, renderType::fragment);
		}
	}

	this->program->linkPrograms();
}

void GLSLMaterial::prepare()
{
	program->use();

	if (texturing)
	{
		Texture::TextureType textType = colorMap->getType();

		program->setInt("textType", (int)textType);

		switch (textType)
		{
		case Texture::COLOR2D:
			colorMap->bind(0);
			glDisable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
			program->setTexture2D("colorText", 0);
			program->setTexture2D("cubeText", 1);
			program->setTexture2D("reflectText", 2);
			program->setTexture2D("refractText", 3);
			program->setTexture2D("normalText", 4);
			break;

		case Texture::COLOR3D:
			colorMap->bind(1);
			glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
			program->setTexture2D("colorText", 0);
			program->setTexture2D("cubeText", 1);
			program->setTexture2D("reflectText", 2);
			program->setTexture2D("refractText", 3);
			program->setTexture2D("normalText", 4);
			break;

			/*
				Necesario setear colorText y cubeText en ambos casos
			*/
		}
	}

	if (reflection)
	{
		reflectionMap->bind(2);
		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
		program->setTexture2D("colorText", 0);
		program->setTexture2D("cubeText", 1);
		program->setTexture2D("reflectText", 2);
		program->setTexture2D("refractText", 3);
		program->setTexture2D("normalText", 4);

	}

	if (refraction)
	{
		refractionMap->bind(3);
		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
		program->setTexture2D("colorText", 0);
		program->setTexture2D("cubeText", 1);
		program->setTexture2D("reflectText", 2);
		program->setTexture2D("refractText", 3);
		program->setTexture2D("normalText", 4);
	}

	if (normalMapping)
	{
		normalMap->bind(4);
		program->setTexture2D("colorText", 0);
		program->setTexture2D("cubeText", 1);
		program->setTexture2D("reflectText", 2);
		program->setTexture2D("refractText", 3);
		program->setTexture2D("normalText", 4);
	}

	// Uniforms
	program->setMatrix("model", System::getModelMatrix());
	program->setMatrix("view", System::getCamera()->getView());
	program->setMatrix("proj", System::getCamera()->getProjection());
	program->setVec3("eyePos", System::getCamera()->getPosition());
	program->setVec3("ambient", System::getAmbient());
	program->setInt("texturing", (int)texturing);
	program->setInt("lighting", (int)lighting);
	program->setInt("normalMapping", (int)normalMapping);
	program->setInt("shininess", shininess);
	program->setVec4("baseColor", color);
	program->setInt("numLights", static_cast<int>(System::getLights().size()));
	program->setInt("computeReflect", (int)reflection);
	program->setInt("computeRefract", (int)refraction);
	program->setFloat("refractCoef", refractCoef);

	for (int i = 0; i < System::getLights().size(); i++)
	{
		program->setVec3("lights[" + std::to_string(i) + "].position", System::getLights()[i]->getPosition());
		program->setVec3("lights[" + std::to_string(i) + "].color", System::getLights()[i]->getInfo().color);
		program->setInt("lights[" + std::to_string(i) + "].type", static_cast<int>(System::getLights()[i]->getInfo().type));
		program->setFloat("lights[" + std::to_string(i) + "].intensity", System::getLights()[i]->getInfo().intensity);
		program->setFloat("lights[" + std::to_string(i) + "].linearAtt", System::getLights()[i]->getInfo().linearAtt);
	}

	// Depth test
	depthWrite ? glDepthMask(GL_TRUE) : glDepthMask(GL_FALSE);

	// Culling test
	if (culling) 
	{
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
	}
	else 
	{
		glDisable(GL_CULL_FACE);
	}

	// Modo de mezclado de colores
	switch (blendMode)
	{	
	case SOLID:
		glBlendFunc(GL_ONE, GL_ZERO);
		break;
	case ALPHA: 
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		break;
	
	case MUL: 
		glBlendFunc(GL_DST_COLOR, GL_ZERO);
		break;
	
	case ADD: 
		glBlendFunc(GL_ONE, GL_ONE);
		break;
	}

	if (wireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else		   glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void GLSLMaterial::prepareInstanced()
{
    program->use();

	if (getTexturing())
	{
		colorMap->bind(0);
		program->setTexture2D("colorText", 0);
	}

	program->setInt("texturing", (int)texturing);
	program->setMatrix("model", System::getModelMatrix());
	program->setMatrix("view", System::getCamera()->getView());
	program->setMatrix("proj", System::getCamera()->getProjection());
	program->setFloat("radius", System::getSimulationSystem()->getPrototype(0)->getRadius()); //System::getParticleSystem()->getPrototype()->getRadius()); // to do: cuidado

	// Light
	program->setInt("numLights", static_cast<int>(System::getLights().size()));
	program->setInt("lighting", (int)lighting);
	program->setVec3("eyePos", glm::vec3(System::getCamera()->getPosition()));
	program->setVec3("ambient", System::getAmbient());
	program->setInt("shininess", shininess);

	for (int i = 0; i < System::getLights().size(); i++)
	{
		program->setVec3("lights[" + std::to_string(i) + "].position", glm::vec3(System::getLights()[i]->getPosition()));
		program->setVec3("lights[" + std::to_string(i) + "].color", System::getLights()[i]->getInfo().color);
		program->setInt("lights[" + std::to_string(i) + "].type", static_cast<int>(System::getLights()[i]->getInfo().type));
		program->setFloat("lights[" + std::to_string(i) + "].intensity", System::getLights()[i]->getInfo().intensity);
		program->setFloat("lights[" + std::to_string(i) + "].linearAtt", System::getLights()[i]->getInfo().linearAtt);
	}

	// Depth test
	//depthWrite ? glDepthMask(GL_TRUE) : glDepthMask(GL_FALSE);
    glDepthMask(GL_TRUE);

	// Culling test
	// if (culling)
	// {
	// 	glEnable(GL_CULL_FACE);
	// 	glCullFace(GL_BACK);
	// }
	// else
	// {
	 	glDisable(GL_CULL_FACE);
	// }

	// Modo de mezclado de colores
	// switch (blendMode)
	// {
	// case SOLID:
	 	glBlendFunc(GL_ONE, GL_ZERO);
	// 	break;

	// case ALPHA:
	 	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	// 	break;

	// case MUL:
	// 	glBlendFunc(GL_DST_COLOR, GL_ZERO);
	// 	break;

	// case ADD:
	// 	glBlendFunc(GL_ONE, GL_ONE);
	// 	break;
	// }

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}


