#ifndef MATHG_IMPLEMENTATION
#define MATHG_IMPLEMENTATION

#define IR_RESOURCE_IMPLEMENT
#define IR_SHADER_RESOURCE_IMPLEMENT
#include "gl3patch.h"
#include <shellapi.h>
#include <stdio.h>
#include <ir_resource/ir_shader_resource.h>
#include "mathg_source.h"
#include "mathg_variables.h"
#include "mathg_vector.h"
#include "mathg_matrix.h"

bool MathG::_get_status(const char *name, bool link)
{
	GLchar infoLog[512];
	GLint success;
	glGetShaderiv(Shader::_distribute1d, link ? GL_LINK_STATUS : GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(Shader::_distribute1d, 512, nullptr, infoLog);
		printf("Error %s %s %s\n%s",
			link ? "linking" : "compiling",
			link ? "program" : "shader",
			name, infoLog);
		return false;
	}
	return true;
};

bool MathG::_compile_distribute1d()
{
	if (Shader::_distribute1d != GL_ERR) return true;
	 Shader::_distribute1d = glCreateShader(GL_VERTEX_SHADER);
	if (Shader::_distribute1d == GL_ERR) return false;
	glShaderSource(Shader::_distribute1d, 1, &Source::_distribute1d, nullptr);
	glCompileShader(Shader::_distribute1d);
	return _get_status("distribute1d", false);
};

bool MathG::_compile_add()
{
	if (Program::Add::_program != GL_ERR) return true;
	if (!_compile_distribute1d()) return false;
	ir::ShaderResource addshader = glCreateShader(GL_FRAGMENT_SHADER);
	if (addshader == GL_ERR) return false;
	glShaderSource(addshader, 1, &Source::_add, nullptr);
	glCompileShader(addshader);
	if (!_get_status("add", false)) return false;
	Program::Add::_program = glCreateProgram();
	if (Program::Add::_program == GL_ERR) return false;
	glAttachShader(Program::Add::_program, Shader::_distribute1d);
	glAttachShader(Program::Add::_program, addshader);
	glLinkProgram(Program::Add::_program);
	if (!_get_status("add", true)) return false;
	Program::Add::_a = glGetUniformLocation(Program::Add::_program, "a");
	if (Program::Add::_a == -1) return false;
	Program::Add::_b = glGetUniformLocation(Program::Add::_program, "b");
	if (Program::Add::_b == -1) return false;
	return true;
};

void MathG::_bind_vector(unsigned int count, VectorG **vectors, unsigned int *positions)
{
	_bind_matrix(count, (MatrixG**)vectors, positions);
};

void MathG::_unbind_vector(VectorG *vector)
{
	_unbind_matrix((MatrixG*)vector);
};

void MathG::_bind_matrix(unsigned int count, MatrixG **matrixes, unsigned int *positions)
{
	for (unsigned int i = 0; i < count; i++) positions[i] = 0xFFFFFFFF;
	bool temporary_used[4] = { false, false, false, false };

	//Finding already binded textures
	for (unsigned int i = 0; i < count; i++)
	{
		//Find permanent textures
		if (matrixes[i]->_permanent != 0xFFFFFFFF)
		{
			positions[i] = matrixes[i]->_permanent;
		}
		//Find temporary textures
		else
		{
			for (unsigned int j = 0; j < _ntemporary; j++)
			{
				if (matrixes[i]->_texture == _temporary[j])
				{
					temporary_used[j] = true;
					positions[i] = _npermanent + j;
					break;
				}
			}
		}
	}

	//Assign position to new textures
	for (unsigned int i = 0; i < count; i++)
	{
		//Try to find place in permanent textures
		if (positions[i] == 0xFFFFFFFF)
		{
			for (unsigned int j = 0; j < _npermanent; j++)
			{
				if (!_permanent[j])
				{
					matrixes[i]->_permanent = j;
					_permanent[j] = true;
					positions[i] = j;
					glActiveTexture(GL_TEXTURE0 + j);
					glBindTexture(GL_TEXTURE_2D, matrixes[i]->_texture);
					break;
				}
			}
		}

		//Find place in temporary storage
		if (positions[i] == 0xFFFFFFFF)
		{
			for (unsigned int j = 0; j < _ntemporary; j++)
			{
				if (!temporary_used[j])
				{
					temporary_used[j] = true;
					_temporary[j] = matrixes[i]->_texture;
					positions[i] = _npermanent + j;
					glActiveTexture(GL_TEXTURE0 + _npermanent + j);
					glBindTexture(GL_TEXTURE_2D, matrixes[i]->_texture);
					break;
				}
			}
		}
	}
};

void MathG::_unbind_matrix(MatrixG *matrix)
{
	if (matrix->_permanent != 0xFFFFFFFF)
	{
		_permanent[matrix->_permanent] = false;
	}
	else
	{
		for (unsigned int i = 0; i < _ntemporary; i++)
		{
			if (matrix->_texture == _temporary[i])
			{
				_temporary[i] = GL_ERR;
				return;
			}
		}
	}
};

bool MathG::init(bool print)
{
	//Making argv
	int argc;
	LPWSTR* wargv = CommandLineToArgvW(GetCommandLineW(), &argc);
	if (wargv == nullptr) return false;
	char **argv = (char**)calloc(argc, sizeof(char*));
	if (argv == nullptr) return false;
	char undefchar = '?';
	for (int i = 0; i < argc; i++)
	{
		int len = WideCharToMultiByte(CP_ACP, 0, wargv[i], -1, NULL, 0, &undefchar, NULL);
		argv[i] = (char*)calloc(len, sizeof(char));
		if (argv[i] == nullptr) return false;
		WideCharToMultiByte(CP_ACP, 0, wargv[i], -1, argv[i], len, &undefchar, NULL);
	}
	LocalFree(wargv);

	//Initializing Freeglut
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitContextVersion(3, 3);
	glutInitContextFlags(GLUT_FORWARD_COMPATIBLE | GLUT_DEBUG);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutInitWindowSize(100, 100);
	_window = glutCreateWindow("MathG");
	if (_window < 1) return false;
	glutHideWindow();
	glutDisplayFunc([](){});
	glDisable(GLUT_MULTISAMPLE);

	//Applying patch
	if (!gl3patch()) return false;

	//Initializing tables
	GLint ntextures;
	glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &ntextures);
	if (ntextures < 3) return false;
	_ntemporary = 3;
	_temporary = (GLuint*)malloc(_ntemporary * sizeof(GLuint));
	if (_temporary == nullptr) return false;
	for (unsigned int i = 0; i < _ntemporary; i++) _temporary[i] = GL_ERR;
	_npermanent = ntextures - 3;
	_permanent = (bool*)malloc(_npermanent * sizeof(bool));
	if (_permanent == nullptr) return false;
	for (unsigned int i = 0; i < _npermanent; i++) _permanent[i] = false;

	//Initializing 1D distribution
	GLfloat d1[] = { -1.0f, 1.0f };
	glGenVertexArrays(1, &Object::_vao1d);
	if (Object::_vao1d == GL_ERR) return false;
	glBindVertexArray(Object::_vao1d);
	glGenBuffers(1, &Object::_vbo1d);
	if (Object::_vbo1d == GL_ERR) return false;
	glBindBuffer(GL_ARRAY_BUFFER, Object::_vbo1d);
	glBufferData(GL_ARRAY_BUFFER, 2 * sizeof(GLfloat), d1, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	//Initializing 2D distribution
	GLfloat d2[] = { 1.0f,	1.0f,
					-1.0f,	1.0f,
					-1.0f,	-1.0f,

					-1.0f,	-1.0f,
					1.0f,	-1.0f,
					1.0f,	1.0f,
	};
	glGenVertexArrays(1, &Object::_vao2d);
	if (Object::_vao2d == GL_ERR) return false;
	glBindVertexArray(Object::_vao2d);
	glGenBuffers(1, &Object::_vbo2d);
	if (Object::_vbo2d == GL_ERR) return false;
	glBindBuffer(GL_ARRAY_BUFFER, Object::_vbo2d);
	glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(GLfloat), d2, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (GLvoid*)sizeof(GLfloat));
	glEnableVertexAttribArray(1);

	_ok = true;
	return true;
};

bool MathG::add(VectorG *a, VectorG *b, VectorG *output)
{
	if (!_ok) return false;
	if (a == b || a == output || b == output) return false;
	if (a->_height != b->_height || a->_height != output->_height) return false;
	if (!_compile_add()) return false;
	glUseProgram(Program::Add::_program);
	glBindVertexArray(MathG::Object::_vao1d);
	VectorG *vectors[2] = { a, b };
	unsigned int positions[2];
	_bind_vector(2, vectors, positions);
	glUniform1i(Program::Add::_a, positions[0]);
	glUniform1i(Program::Add::_b, positions[1]);
	if (output->_framebuffer == GL_ERR)
	{
		glGenFramebuffers(1, &output->_framebuffer);
		if (output->_framebuffer == GL_ERR) return false;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, output->_framebuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, output->_texture, 0);
	glViewport(0, 0, 1, output->_height);
	glDrawArrays(GL_LINES, 0, 2);
	glFinish();
	return false;
};

bool MathG::substract(VectorG *a, VectorG *b, VectorG *output)
{
	return false;
};

bool MathG::multiply(MatrixG *a, VectorG *b, VectorG *output)
{
	return false;
};

bool MathG::free()
{
	if (Object::_vbo1d != GL_ERR) glDeleteBuffers(1, &Object::_vbo1d);
	if (Object::_vao1d != GL_ERR) glDeleteVertexArrays(1, &Object::_vao1d);
	if (Object::_vbo2d != GL_ERR) glDeleteBuffers(1, &Object::_vbo2d);
	if (Object::_vao2d != GL_ERR) glDeleteVertexArrays(1, &Object::_vao2d);
	if (_window) glutDestroyWindow(_window);
	return true;
};

#endif	//#ifndef MATHG_IMPLEMENTATION