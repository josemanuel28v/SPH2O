#include "GUI/GL4Render.h"
#include "GUI/System.h"

void GL4Render::setupObject(Object* obj)
{
	for (const auto& mesh : obj->getMeshes())
	{
		// Si no existe el id de la mesh en el mapa bufferObjects se añade
		if (bufferObjects.find(mesh->getMeshID()) == bufferObjects.end())
		{
			VAO_t vbo;
			std::vector<vertex_t>* vertices = mesh->getVertices();
			std::vector<glm::uint32>* indices = mesh->getIndices();
			Material* mat = mesh->getMaterial();
			RenderProgram* program = mat->getProgram();

			glGenVertexArrays(1, &vbo.id);
			glBindVertexArray(vbo.id);

			// Crear los buffers para los vertices y los indices
			glGenBuffers(1, &vbo.v_id);
			glGenBuffers(1, &vbo.i_id);

			// Vertices
			glBindBuffer(GL_ARRAY_BUFFER, vbo.v_id);
			glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_t) * vertices->size(), vertices->data(), GL_STATIC_DRAW);

			// Indices 
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo.i_id);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint) * indices->size(), indices->data(), GL_STATIC_DRAW);

			glEnableVertexAttribArray(program->varList["vpos"]);
			glVertexAttribPointer(program->varList["vpos"], 4, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void*)offsetof(vertex_t, position));

			// Aunque las normales vengan de un mapa de normales se necesitan las normales por vértice para calcular la TBN
			glEnableVertexAttribArray(program->varList["vnorm"]);
			glVertexAttribPointer(program->varList["vnorm"], 4, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void*)offsetof(vertex_t, normal));
			
			if (mat->getNormalMapping())
			{
				glEnableVertexAttribArray(program->varList["vtan"]);
				glVertexAttribPointer(program->varList["vtan"], 4, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void*)offsetof(vertex_t, tangent));
			}

			if (mat->getTexturing())
			{
				glEnableVertexAttribArray(program->varList["vtextcoord"]);
				glVertexAttribPointer(program->varList["vtextcoord"], 2, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void*)offsetof(vertex_t, textCoord));
			}

			glBindVertexArray(0);

			bufferObjects[mesh->getMeshID()] = vbo;
		}
	}
}

void GL4Render::setupObject(Object* obj, uint numInstances)
{
	for (const auto& mesh : obj->getMeshes())
	{
		// Si no existe el id de la mesh en el mapa bufferObjects se añade
		if (bufferObjects.find(mesh->getMeshID()) == bufferObjects.end())
		{
			VAO_t vbo;
			std::vector<vertex_t>* vertices = mesh->getVertices();
			std::vector<glm::uint32>* indices = mesh->getIndices();
			Material* mat = mesh->getMaterial();
			RenderProgram* program = mat->getProgram();

			glGenVertexArrays(1, &vbo.id);
			glBindVertexArray(vbo.id);

			// Crear los buffers para los vertices y los indices
			glGenBuffers(1, &vbo.v_id);
			glGenBuffers(1, &vbo.i_id);
			glGenBuffers(1, &vbo.mvp_id);
			glGenBuffers(1, &vbo.color_id);

			// Vertices
			glBindBuffer(GL_ARRAY_BUFFER, vbo.v_id);
			glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_t) * vertices->size(), vertices->data(), GL_STATIC_DRAW);

			// Indices 
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo.i_id);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint) * indices->size(), indices->data(), GL_STATIC_DRAW);
			
			glEnableVertexAttribArray(program->varList["vpos"]);
			glVertexAttribPointer(program->varList["vpos"], 4, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void*)offsetof(vertex_t, position));

			if (mat->getTexturing())
			{
				glEnableVertexAttribArray(program->varList["vtextcoord"]);
				glVertexAttribPointer(program->varList["vtextcoord"], 2, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void*)offsetof(vertex_t, textCoord));
			}

			// Positions (instancing)
			glBindBuffer(GL_ARRAY_BUFFER, vbo.mvp_id);
        	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * numInstances, nullptr, GL_DYNAMIC_DRAW); 

        	glEnableVertexAttribArray(program->varList["ppos"]);
        	glVertexAttribPointer(program->varList["ppos"], 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (GLvoid*)0x00);
        	glVertexAttribDivisor(program->varList["ppos"], 1);

			// Color (instancing)
			glBindBuffer(GL_ARRAY_BUFFER, vbo.color_id);
			glBufferData(GL_ARRAY_BUFFER, sizeof(float) * numInstances, nullptr, GL_DYNAMIC_DRAW);

			glEnableVertexAttribArray(program->varList["pscalar"]);
			glVertexAttribPointer(program->varList["pscalar"], 1, GL_FLOAT, GL_FALSE, sizeof(float), (GLvoid*)0x00);
			glVertexAttribDivisor(program->varList["pscalar"], 1);

			glBindVertexArray(0);

			bufferObjects[mesh->getMeshID()] = vbo;
		}
	}
}

void GL4Render::removeObject(Object* obj, bool instancing)
{
	if (!instancing)
	{
		for (const auto& mesh : obj->getMeshes())
		{
			uint meshID = mesh->getMeshID();

			// Si el vbo asociado al meshID existe en bufferObjects se libera la memoria de sus buffers
			if (bufferObjects.find(meshID) != bufferObjects.end())
			{
				const VAO_t& vbo = bufferObjects[meshID];
				glDeleteVertexArrays(1, &vbo.id);
				glDeleteBuffers(1, &vbo.v_id);
				glDeleteBuffers(1, &vbo.i_id);
			}
		}
	}
	else
	{
		for (const auto& mesh : obj->getMeshes())
		{
			uint meshID = mesh->getMeshID();

			// Si el vbo asociado al meshID existe en bufferObjects se libera la memoria de sus buffers
			if (bufferObjects.find(meshID) != bufferObjects.end())
			{
				const VAO_t& vbo = bufferObjects[meshID];
				glDeleteVertexArrays(1, &vbo.id);
				glDeleteBuffers(1, &vbo.v_id);
				glDeleteBuffers(1, &vbo.i_id);
				glDeleteBuffers(1, &vbo.mvp_id);
				glDeleteBuffers(1, &vbo.color_id);
			}
		}
	}
}

void GL4Render::drawObject(Object* obj)
{
	obj->computeModelMatrix();
	System::setModelMatrix(obj->getModelMt());

	for (const auto& mesh : obj->getMeshes())
	{
		Material* mat = mesh->getMaterial();

		// Activar buffers antes de usar el programa
		VAO_t buffer = bufferObjects[mesh->getMeshID()];

		// Attributes
		mat->prepare();

		// Dibujado
		glBindVertexArray(buffer.id);
		glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(mesh->getIndices()->size()), GL_UNSIGNED_INT, nullptr);
		glBindVertexArray(0);
	}
}

void GL4Render::drawObject(Object* obj, unsigned numInstances, glm::vec4* positions, Real* scalarField)
{
	for (const auto& mesh : obj->getMeshes())
	{
		Material* mat = mesh->getMaterial();

		// Activar buffers antes de usar el programa
		VAO_t buffer = bufferObjects[mesh->getMeshID()];

		// Attributes
		mat->prepareInstanced();

		// Dibujado
		glBindVertexArray(buffer.id);
		glBindBuffer(GL_ARRAY_BUFFER, buffer.mvp_id);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * numInstances, &positions[0], GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, buffer.color_id);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * numInstances, &scalarField[0], GL_DYNAMIC_DRAW);
		glDrawElementsInstanced(GL_TRIANGLES, static_cast<GLsizei>(mesh->getIndices()->size()), GL_UNSIGNED_INT, nullptr, numInstances); 
		glBindVertexArray(0);
	}
}

void GL4Render::drawObjects(const std::vector<Object*>& objs) 
{
	for (const auto obj : objs) 
	{
		drawObject(obj);
	}
}