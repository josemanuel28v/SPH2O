#include "GLSLShader.h"
#include <iostream>
#include <fstream>
#include <sstream>

GLSLShader::GLSLShader()
{
	programId = glCreateProgram();
}

GLint GLSLShader::checkShaderError(GLuint shId)
{
	GLint success = 1;
	char* infoLog = new char[1024];
	infoLog[0] = '\0';
	
	glGetShaderiv(shId, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(shId, 1024, nullptr, infoLog);
		std::cout << "Error en shader\n" << infoLog << "\n";
		// exit(-1);
	}
	errorMSGs += "\n" + std::string(infoLog);
	
	return success;
}

GLint GLSLShader::checkProgramError()
{
	GLint program_linked;
	glGetProgramiv(programId, GL_LINK_STATUS, &program_linked);
	if (program_linked != GL_TRUE)
	{
		GLsizei log_length = 0;
		GLchar message[1024];
		glGetProgramInfoLog(programId, 1024, &log_length, message);
		std::cout << "Error en linkado de programa: \n" << message << "\n\n";
	}

	return program_linked;
}

std::string GLSLShader::readFile(const std::string fileName) 
{
	std::ifstream inFile(fileName);

	if (inFile.is_open())
	{
		std::stringstream strStream;
		strStream << inFile.rdbuf(); 
		std::string str = strStream.str(); 
		inFile.close();

		return str;
	}
	else
	{
		std::cout << "Error leyendo " << fileName << std::endl;
		return "";
	}
}

void GLSLShader::setupShaderVarList() {
	
	int count = 0;
	int bufSize = 100;
	char* name = new char[bufSize];
	GLenum type;
	int size = 0;
	int length = 0;

	glGetProgramiv(programId, GL_ACTIVE_ATTRIBUTES, &count);
	for (int i = 0; i < count; i++)
	{
		glGetActiveAttrib(programId, (GLuint)i, bufSize, &length, &size, &type,name);
		varList[std::string(name)] = glGetAttribLocation(programId,name);
		//std::cout << "Attribute: " << std::string(name) << std::endl;
	}

	glGetProgramiv(programId, GL_ACTIVE_UNIFORMS, &count);
	//std::cout << "Uniforms: " << count << std::endl;
	for (int i = 0; i < count; i++)
	{
		glGetActiveUniform(programId, (GLuint)i, bufSize, &length, &size,&type, name);
		varList[std::string(name)] = glGetUniformLocation(programId,name);
		//std::cout << "Uniform: " << std::string(name) << std::endl;
	}
	std::cout << std::endl;

}

void GLSLShader::linkPrograms()
{
	use();
	for (auto& sh : shaders) 
	{
		glAttachShader(programId, sh.second);
	}
	glLinkProgram(programId);
	checkProgramError();
	setupShaderVarList();
}

std::string GLSLShader::getErrorMsg(GLuint shaderID)
{
	GLint success = 1;
	char* infoLog = new char[1024];
	infoLog[0] = '\0';
	glGetShaderiv(shaderID, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(shaderID, 1024, nullptr, infoLog);
		std::cout << "Error en shader\n" << infoLog << "\n";
		// exit(-1);
	}
	errorMSGs += "\n" + std::string(infoLog);
	return errorMSGs;
}

void GLSLShader::use()
{
	glUseProgram(programId);
}

void GLSLShader::setProgram(std::string fileName, renderTypes_e type)
{
	std::string prgSource = readFile(fileName);
	
	if (type == renderTypes_e::vertex)
	{
		this->shaders[type] = glCreateShader(GL_VERTEX_SHADER);
	}
	else if (type == renderTypes_e::fragment)
	{
		this->shaders[type] =  glCreateShader(GL_FRAGMENT_SHADER);
	}

	const char* source = prgSource.c_str();
	GLuint shId = this->shaders[type];
	glShaderSource(shId, 1, &source, nullptr);
	glCompileShader(shId);

	checkShaderError(shId);
}

void GLSLShader::setInt(std::string loc, int val) 
{
	glUniform1i(varList[loc], val);
}

void GLSLShader::setFloat(std::string loc, float val) 
{
	glUniform1f(varList[loc], val);
}

void GLSLShader::setVec3(std::string loc, const glm::vec3& vec) 
{
	glUniform3fv(varList[loc], 1, glm::value_ptr(vec));
}

void GLSLShader::setVec4(std::string loc, const glm::vec4& vec) 
{
	glUniform4fv(varList[loc], 1, glm::value_ptr(vec));
}

void GLSLShader::setMatrix(std::string loc, const glm::mat4& matrix) 
{
	glUniformMatrix4fv(varList[loc], 1, GL_FALSE, glm::value_ptr(matrix));
}

void GLSLShader::setTexture2D(std::string loc, uint textUnit)
{
	glUniform1i(varList[loc], textUnit);
}


