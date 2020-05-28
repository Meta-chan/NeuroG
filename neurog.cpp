#include "neurog.h"
#include "gl3patch.h"
#include <shellapi.h>
#include <time.h>
#include <share.h>
#include <assert.h>
#define IR_IMPLEMENT
#include <ir_resource/ir_memresource.h>
#include <ir_resource/ir_file_resource.h>
#include <ir_resource/ir_shader_resource.h>

bool NeuroG::_init_glut()
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
	glutCreateWindow("");
	glutDisplayFunc([](){});
	glDisable(GLUT_MULTISAMPLE);

	return gl3patch();
};

bool NeuroG::_compile_shader(const char *filename, bool vertex, GLuint *shader)
{
	//Opening file
	char relpath[128];
	strcpy_s(relpath, "shaders/");
	strcat_s(relpath, filename);

	ir::FileResource file = _fsopen(relpath, "rb", _SH_DENYNO);
	if (file == nullptr) return false;
	fseek(file, 0, SEEK_END);
	unsigned int filesize = ftell(file);
	fseek(file, 0, SEEK_SET);

	//Reading file
	ir::MemResource<char> source = (char*)malloc(filesize + 1);
	if (source == nullptr) return false;
	unsigned int read = fread(source, 1, filesize, file);
	if (read != filesize) return false;
	source[filesize] = '\0';

	//Compiling shader
	*shader = glCreateShader(vertex ? GL_VERTEX_SHADER : GL_FRAGMENT_SHADER);
	if (*shader == GL_ERR) return false;
	const char *constsource = source;
	glShaderSource(*shader, 1, &constsource, NULL);
	glCompileShader(*shader);

	//Writing log if error
	GLchar infoLog[512];
	GLint success;
	glGetShaderiv(*shader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(*shader, 512, NULL, infoLog);
		printf("OpenGL shader compilation error in file %s\n%s", filename, infoLog);
		return false;
	}

	return true;
};

bool NeuroG::_link_program(const char *programname, GLuint vertex, GLuint fragment, GLuint *program)
{
	*program = glCreateProgram();
	if (*program == GL_ERR) return false;
	glAttachShader(*program, vertex);
	glAttachShader(*program, fragment);
	glLinkProgram(*program);

	GLchar infoLog[512];
	GLint success;
	glGetProgramiv(*program, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(*program, 512, NULL, infoLog);
		printf("OpenGL %s program linking error\n%s\n", programname, infoLog);
		return false;
	}

	return true;
};

bool NeuroG::_init_programs()
{
	ir::ShaderResource distribute1d, distribute2d, forward, lastbackward, backward, corrigate, bitwise1d, bitwise2d;

	//Compiling shaders
	if (!_compile_shader("vertex_distribute_1d.c", true, &distribute1d.value()) ||
		!_compile_shader("vertex_distribute_2d.c", true, &distribute2d.value()) ||
		!_compile_shader("fragment_forward.c", false, &forward.value()) ||
		!_compile_shader("fragment_lastbackward.c", false, &lastbackward.value()) ||
		!_compile_shader("fragment_backward.c", false, &backward.value()) ||
		!_compile_shader("fragment_corrigate.c", false, &corrigate.value()) ||
		!_compile_shader("fragment_bitwise_1d.c", false, &bitwise1d.value()) ||
		!_compile_shader("fragment_bitwise_2d.c", false, &bitwise2d.value())) return false;

	//Linking programs
	if (!_link_program("forward", distribute1d, forward, &_forwardprog.program) ||
		!_link_program("lastbackward", distribute1d, lastbackward, &_lastbackwardprog.program) ||
		!_link_program("backward", distribute1d, backward, &_backwardprog.program) ||
		!_link_program("corrigate", distribute2d, corrigate, &_corrigateprog.program) ||
		!_link_program("bitwise_1d", distribute1d, bitwise1d, &_bitwise1dprog.program) ||
		!_link_program("bitwise_2d", distribute2d, bitwise2d, &_bitwise2dprog.program)) return false;

	//getting addresses of uniforms
	if ((_forwardprog.prevlength = glGetUniformLocation(_forwardprog.program, "prevlength")) == GL_ERR ||
		(_forwardprog.prevvector = glGetUniformLocation(_forwardprog.program, "prevvector")) == GL_ERR ||
		(_forwardprog.weights = glGetUniformLocation(_forwardprog.program, "weights")) == GL_ERR ||

		(_lastbackwardprog.goal = glGetUniformLocation(_lastbackwardprog.program, "goal")) == GL_ERR ||
		(_lastbackwardprog.lestvector = glGetUniformLocation(_lastbackwardprog.program, "lastvector")) == GL_ERR ||

		(_backwardprog.nextlength = glGetUniformLocation(_backwardprog.program, "nextlength")) == GL_ERR ||
		(_backwardprog.nexterror = glGetUniformLocation(_backwardprog.program, "nexterror")) == GL_ERR ||
		(_backwardprog.prevvector = glGetUniformLocation(_backwardprog.program, "prevvector")) == GL_ERR ||
		(_backwardprog.weights = glGetUniformLocation(_backwardprog.program, "weights")) == GL_ERR ||

		(_corrigateprog.koef = glGetUniformLocation(_corrigateprog.program, "koef")) == GL_ERR ||
		(_corrigateprog.nexterror = glGetUniformLocation(_corrigateprog.program, "nexterror")) == GL_ERR ||
		(_corrigateprog.prevvector = glGetUniformLocation(_corrigateprog.program, "prevvector")) == GL_ERR ||
		(_corrigateprog.weights = glGetUniformLocation(_corrigateprog.program, "weights")) == GL_ERR ||

		(_bitwise1dprog.vector = glGetUniformLocation(_bitwise1dprog.program, "vector")) == GL_ERR ||

		(_bitwise2dprog.weights = glGetUniformLocation(_bitwise2dprog.program, "weights")) == GL_ERR)
		return false;

	return true;
};

bool NeuroG::_init_objects()
{
	//Initializing d1 distribution
	GLfloat d1[] = { -1.0f, 1.0f };

	glGenVertexArrays(1, &_distribute1dvao);
	glBindVertexArray(_distribute1dvao);
	glGenBuffers(1, &_distribute1dvbo);
	glBindBuffer(GL_ARRAY_BUFFER, _distribute1dvbo);
	glBufferData(GL_ARRAY_BUFFER, 2 * sizeof(GLfloat), d1, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	//Initializing d2 distribution
	GLfloat d2[] = { 1.0f,	1.0f,
						-1.0f,	1.0f,
						-1.0f,	-1.0f,

						-1.0f,	-1.0f,
						1.0f,	-1.0f,
						1.0f,	1.0f,
	};


	glGenVertexArrays(1, &_distribute2dvao);
	glBindVertexArray(_distribute2dvao);
	glGenBuffers(1, &_distribute2dvbo);
	glBindBuffer(GL_ARRAY_BUFFER, _distribute2dvbo);
	glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(GLfloat), d2, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (GLvoid*)sizeof(GLfloat));
	glEnableVertexAttribArray(1);

	return true;
};

bool NeuroG::_create_texture(GLuint *texture, unsigned int width, unsigned int height, bool bitwise)
{
	glGenTextures(1, texture);
	if (*texture == GL_ERR) return false;
	glBindTexture(GL_TEXTURE_2D, *texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	if (bitwise) glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	else glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, width, height, 0, GL_RED, GL_FLOAT, nullptr);
	return true;
};

bool NeuroG::_create_framebuffer(GLuint *framebuffer, GLuint texture, unsigned int width, unsigned int height)
{
	glGenFramebuffers(1, framebuffer);
	if (*framebuffer == GL_ERR) return false;
	glBindFramebuffer(GL_FRAMEBUFFER, *framebuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
	glViewport(0, 0, width, height);
	return true;
};

bool NeuroG::_fill_column(unsigned int height,
	float *data, FILE *file, std::default_random_engine* generator)
{
	if (file != nullptr)
	{
		if (fread(data, sizeof(float), height, file) < height) return false;
	}
	else if (generator != nullptr)
	{
		std::uniform_real_distribution<float> distribution(-1.0f, 1.0f);
		for (unsigned int i = 0; i < height; i++)
		{
			#ifdef _DEBUG
				data[i] = (float)i - 1;
			#else
				data[i] = distribution(*generator);
			#endif
		}
	}
	return true;
};

bool NeuroG::_store(GLuint texture, unsigned int width, unsigned int height,
	float *data, FILE *file, std::default_random_engine* generator, GLuint framebuffer, GLuint bittexture)
{
	if (_hardware_direct_store)
	{
		glBindTexture(GL_TEXTURE_2D, texture);
		for (unsigned int column = 0; column < width; column++)
		{
			if (!_fill_column(height, data, file, generator)) return false;
			glTexSubImage2D(GL_TEXTURE_2D, 0, column, 0, 1, height, GL_RED, GL_FLOAT, data);
		}
	}
	else
	{
		bool bufframe = (framebuffer == GL_ERR);
		if (bufframe)
		{
			glGenFramebuffers(1, &framebuffer);
			if (framebuffer == GL_ERR) return false;
		}

		bool buftex = (bittexture == GL_ERR);
		if (buftex)
		{
			if (!_create_texture(&bittexture, width, height, true))
			{
				if (bufframe) glDeleteFramebuffers(1, &framebuffer);
				return false;
			}
		}
		else
		{
			glBindTexture(GL_TEXTURE_2D, bittexture);
		}

		for (unsigned int column = 0; column < width; column++)
		{
			if (!_fill_column(height, data, file, generator)) return false;
			glTexSubImage2D(GL_TEXTURE_2D, 0, column, 0, 1, height, GL_RGBA, GL_UNSIGNED_BYTE, data);
		}

		//program
		if (width == 1)
		{
			glUseProgram(_bitwise1dprog.program);
			glBindVertexArray(_distribute1dvao);
		}
		else
		{
			glUseProgram(_bitwise2dprog.program);
			glBindVertexArray(_distribute2dvao);
		}

		//vector
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, bittexture);
		glUniform1i((width == 1) ? _bitwise1dprog.vector : _bitwise2dprog.weights, 0);

		//framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
		glViewport(0, 0, width, height);

		//calculate
		if (width == 1) glDrawArrays(GL_LINES, 0, 2);
		else glDrawArrays(GL_TRIANGLES, 0, 6);
		glFinish();

		if (bufframe) glDeleteFramebuffers(1, &framebuffer);
		if (buftex) glDeleteTextures(1, &bittexture);
	};

	return true;
};

bool NeuroG::_init_test()
{
	//Direct store test
	GLuint test = GL_ERR;
	float data = - 1.0f / 3.0f;
	if (!_create_texture(&test, 1, 1, false)) return false;
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, 1, GL_RED, GL_FLOAT, &data);
	data = 0;
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, &data);
	_hardware_direct_store = (abs(data + 1.0f / 3.0f) < 0.001);
	_hardware_direct_store = false;

	//Store Test
	data = -1.0f / 7.0f;
	if (!_store(test, 1, 1, &data, nullptr, nullptr, GL_ERR, GL_ERR)) return false;
	data = 0;
	glBindTexture(GL_TEXTURE_2D, test);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, &data);

	glDeleteTextures(1, &test);
	return abs(data + 1.0f / 7.0f) < 0.001;
};

bool NeuroG::_init_vectors(unsigned int nlayers, const unsigned int *layers)
{
	_nlayers = nlayers;

	//Initializing _layers (n)
	if (nlayers < 2) return false;	//quit if less than 2 layers
	for (unsigned int i = 0; i < nlayers; i++)
	{
		if (layers[i] == 0) return false;	//quit if zero neyrons in layer
	}
	_layers = (unsigned int*)malloc(nlayers * sizeof(unsigned int));
	if (_layers == nullptr) return false;
	memcpy(_layers, layers, nlayers * sizeof(unsigned int));

	//Initializing _weights (n - 1)
	for (unsigned int a = 0; a < 2; a++)
	{
		_weights[a] = (TF*)malloc((nlayers - 1) * sizeof(TF));
		if (_weights[a] == nullptr) return false;
		_weights[a].set_size(nlayers - 1);
		for (unsigned int i = 0; i < (nlayers - 1); i++) { _weights[a][i].texture = GL_ERR;  _weights[a][i].framebuffer = GL_ERR; }
	}

	//Initializing _vectors (n)
	_vectors = (TF*)malloc(nlayers * sizeof(TF));
	if (_vectors == nullptr) return false;
	_vectors.set_size(nlayers);
	for (unsigned int i = 0; i < nlayers; i++) { _vectors[i].texture = GL_ERR; _vectors[i].framebuffer = GL_ERR; }

	//Initializing _errors (n - 1)
	_errors = (TF*)malloc((nlayers - 1) * sizeof(TF));
	if (_errors == nullptr) return false;
	_errors.set_size(nlayers - 1);
	for (unsigned int i = 0; i < (nlayers - 1); i++) { _errors[i].texture = GL_ERR; _errors[i].framebuffer = GL_ERR; }

	return true;
};

bool NeuroG::_init_textures(FILE *file)
{
	//Allocating maximal buffer
	unsigned int maxsize = 0;
	for (unsigned int i = 0; i < _nlayers; i++)
	{
		if (_layers[i] + 1 > maxsize) maxsize = _layers[i] + 1;
	}
	ir::MemResource<float> buffer = (float*)malloc(maxsize * sizeof(float));
	if (buffer == nullptr) return false;
	memset(buffer, 0, maxsize * sizeof(float));
	
	//Initializing _vectors and _framebuffers
	if (!_create_texture(&_vectors[0].texture, 1, _layers[0], false)) return false; 
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, _layers[0], GL_RED, GL_FLOAT, buffer);
	if (!_hardware_direct_store)
	{
		if (!_create_framebuffer(&_vectors[0].framebuffer, _vectors[0].texture, 1, _layers[0])) return false;
	}

	for (unsigned int i = 1; i < _nlayers; i++)
	{
		if (!_create_texture(&_vectors[i].texture, 1, _layers[i], false)) return false;
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, _layers[i], GL_RED, GL_FLOAT, buffer);
		if (!_create_framebuffer(&_vectors[i].framebuffer, _vectors[i].texture, 1, _layers[i])) return false;
	}

	//Initializing _errors
	for (unsigned int i = 0; i < (_nlayers - 1); i++)
	{
		if (!_create_texture(&_errors[i].texture, 1, _layers[i + 1], false)) return false;
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, _layers[i + 1], GL_RED, GL_FLOAT, buffer);
		if (!_create_framebuffer(&_errors[i].framebuffer, _errors[i].texture, 1, _layers[i + 1])) return false;
	}

	//Initializing goal (bitwise)
	if (!_create_texture(&_goal_texture, 1, _layers[_nlayers - 1], true)) return false;
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, _layers[_nlayers - 1], GL_RGBA, GL_UNSIGNED_BYTE, buffer);

	//Initializing extra vectors/framebuffers
	if (!_hardware_direct_store)
	{
		if (!_create_texture(&_bitinput_texture, 1, _layers[0], true)) return false;
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, _layers[0], GL_RGBA, GL_UNSIGNED_BYTE, buffer);
	}

	//Initializing weights
	std::default_random_engine generator;
	generator.seed((unsigned int)time(nullptr));

	for (unsigned int i = 0; i < (_nlayers - 1); i++)
	{
		for (unsigned int a = 0; a < 2; a++)
		{
			if (!_create_texture(&_weights[a][i].texture, _layers[i] + 1, _layers[i + 1], false)) return false;
			if (!_create_framebuffer(&_weights[a][i].framebuffer, _weights[a][i].texture, _layers[i] + 1, _layers[i + 1])) return false;
			if (!_store(_weights[a][i].texture, _layers[i] + 1, _layers[i + 1], buffer, file, &generator, _weights[a][i].framebuffer, GL_ERR)) return false;
		}

		#ifdef _DEBUG
			float check[10];
			memset(check, 0, 10 * sizeof(float));
			glBindTexture(GL_TEXTURE_2D, _weights[0][i].texture);
			glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, check);
			int q = 0;
		#endif
	}

	return true;
};

bool NeuroG::_init(unsigned int nlayers, const unsigned int *layers, FILE *file)
{
	if (!_init_glut()							||
		!_init_programs()						||
		!_init_objects()						||
		!_init_test()							||
		!_init_vectors(nlayers, layers)			||
		!_init_textures(file)) return false;
	
	_ok = true;
	return true;
};

bool NeuroG::_init_from_file(const wchar_t *filepath)
{
	ir::FileResource file = _wfsopen(filepath, L"rb", _SH_DENYNO);
	if (file == nullptr) return false;

	FileHeader header, sample;
	if (fread(&header, sizeof(FileHeader), 1, file) == 0	||
		memcmp(header.signature, sample.signature, 3) != 0				||
		header.version !=sample.version) return false;

	unsigned int nlayers;
	if (fread(&nlayers, sizeof(unsigned int), 1, file) == 0) return false;

	ir::MemResource<unsigned int> layers = (unsigned int*)malloc(nlayers * sizeof(unsigned int));
	if (fread(layers, sizeof(unsigned int), nlayers, file) < nlayers) return false;

	return _init(nlayers, layers, file);
};

NeuroG::NeuroG(unsigned int nlayers, const unsigned int *layers, bool *ok)
{
	bool r = _init(nlayers, layers, nullptr);
	if (ok != nullptr) *ok = r;
};

NeuroG::NeuroG(const wchar_t *filepath, bool *ok)
{
	bool r = _init_from_file(filepath);
	if (ok != nullptr) *ok = r;
};

bool NeuroG::set_input(const float *input)
{
	if (!_ok) return false;

	//Very very dirty. Fix!
	bool r = _store(_vectors[0].texture, 1, _layers[0], (float*)input, nullptr, nullptr, _vectors[0].framebuffer, _bitinput_texture);

	#ifdef _DEBUG
		float check[10];
		memset(check, 0, 10 * sizeof(float));
		glBindTexture(GL_TEXTURE_2D, _vectors[0].texture);
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, check);
		int q = 0;
	#endif
	
	return r;
};

bool NeuroG::set_goal(const float *goal)
{
	if (!_ok) return false;

	glBindTexture(GL_TEXTURE_2D, _goal_texture);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, _layers[_nlayers - 1], GL_RGBA, GL_UNSIGNED_BYTE, goal);

	#ifdef _DEBUG
		float check[10];
		memset(check, 0, 10 * sizeof(float));
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, check);
		int q = 0;
	#endif

	return true;
};

bool NeuroG::set_koef(float koef)
{
	if (!_ok) return false;
	_koef = koef;
	return true;
};

bool NeuroG::get_output(float *output)
{
	if (!_ok) return false;
	glBindTexture(GL_TEXTURE_2D, _vectors[_nlayers - 1].texture);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, output);
	return true;
};

bool NeuroG::forward()
{
	if (!_ok) return false;

	glUseProgram(_forwardprog.program);
	glBindVertexArray(_distribute1dvao);

	for (unsigned int i = 0; i < (_nlayers - 1); i++)
	{
		glUniform1i(_forwardprog.prevlength, _layers[i]);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, _vectors[i].texture);
		glUniform1i(_forwardprog.prevvector, 0);
		
		glActiveTexture(GL_TEXTURE0 + 1);
		glBindTexture(GL_TEXTURE_2D, _weights[_switch][i].texture);
		glUniform1i(_forwardprog.weights, 1);
		
		glBindFramebuffer(GL_FRAMEBUFFER, _vectors[i + 1].framebuffer);
		//WTF? Do I need to initialize framebuffers?
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _vectors[i + 1].texture, 0);
		glViewport(0, 0, 1, _layers[i + 1]);
		glDrawArrays(GL_LINES, 0, 2);
		glFinish();

		#ifdef _DEBUG
			float check[10];
			memset(check, 0, 10 * sizeof(float));
			glBindTexture(GL_TEXTURE_2D, _vectors[i + 1].texture);
			glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, check);
			int q = 0;
		#endif
	}

	return true;
}; 

bool NeuroG::backward()
{
	if (!_ok) return false;

	glUseProgram(_lastbackwardprog.program);
	glBindVertexArray(_distribute1dvao);
	//last vector
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _vectors[_nlayers - 1].texture);
	glUniform1i(_lastbackwardprog.lestvector, 0);
	//goal
	glActiveTexture(GL_TEXTURE0 + 1);
	glBindTexture(GL_TEXTURE_2D, _goal_texture);
	glUniform1i(_lastbackwardprog.goal, 1);
	//last error
	glBindFramebuffer(GL_FRAMEBUFFER, _errors[_nlayers - 2].framebuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _errors[_nlayers - 2].texture, 0);
	glViewport(0, 0, 1, _layers[_nlayers - 1]);
	//calculating
	glDrawArrays(GL_LINES, 0, 2);
	glFinish();

	#ifdef _DEBUG
		float check[10];
		memset(check, 0, 10 * sizeof(float));
		glBindTexture(GL_TEXTURE_2D, _errors[_nlayers - 2].texture);
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, check);
		int q = 0;
	#endif

	if (_nlayers > 2) glUseProgram(_backwardprog.program);
	for (unsigned int i = _nlayers - 2; i > 0; i--)
	{
		glUniform1i(_backwardprog.nextlength, _layers[i]);

		//next error
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, _errors[i].texture);
		glUniform1i(_backwardprog.nexterror, 0);
		//previous vector
		glActiveTexture(GL_TEXTURE0 + 1);
		glBindTexture(GL_TEXTURE_2D, _vectors[i].texture);
		glUniform1i(_backwardprog.prevvector, 1);
		//weights
		glActiveTexture(GL_TEXTURE0 + 2);
		glBindTexture(GL_TEXTURE_2D, _weights[_switch][i].texture);
		glUniform1i(_backwardprog.weights, 2);
		//previous error
		glBindFramebuffer(GL_FRAMEBUFFER, _errors[i - 1].framebuffer);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _errors[i - 1].texture, 0);
		glViewport(0, 0, 1, _layers[i]);
		//calculating
		glDrawArrays(GL_LINES, 0, 2);
		glFinish();

		#ifdef _DEBUG
			float check[10];
			memset(check, 0, 10 * sizeof(float));
			glBindTexture(GL_TEXTURE_2D, _errors[i - 1].texture);
			glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, check);
			int q = 0;
		#endif
	}

	glUseProgram(_corrigateprog.program);
	glBindVertexArray(_distribute2dvao);
	glUniform1fv(_corrigateprog.koef, 1, &_koef);
	for (unsigned int i = _nlayers - 1; i > 0; i--)
	{
		//next error
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, _errors[i - 1].texture);
		glUniform1i(_corrigateprog.nexterror, 0);
		//previous vector
		glActiveTexture(GL_TEXTURE0 + 1);
		glBindTexture(GL_TEXTURE_2D, _vectors[i - 1].texture);
		glUniform1i(_corrigateprog.prevvector, 1);
		//weights
		glActiveTexture(GL_TEXTURE0 + 2);
		glBindTexture(GL_TEXTURE_2D, _weights[_switch][i - 1].texture);
		glUniform1i(_corrigateprog.weights, 2);
		//new weights
		glBindFramebuffer(GL_FRAMEBUFFER, _weights[_switch ^ 1][i - 1].framebuffer);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _weights[_switch ^ 1][i - 1].texture, 0);
		glViewport(0, 0, _layers[i - 1] + 1, _layers[i]);
		//calculating
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glFinish();

		#ifdef _DEBUG
			float check[10];
			memset(check, 0, 10 * sizeof(float));
			glBindTexture(GL_TEXTURE_2D, _weights[_switch ^ 1][i - 1].texture);
			glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, check);
			int q = 0;
		#endif
	}

	_switch ^= 1;
	return true;
};

bool NeuroG::save(const wchar_t *filepath)
{
	if (!_ok) return false;

	ir::FileResource file = _wfsopen(filepath, L"wb", _SH_DENYNO);
	if (file == nullptr) return false;

	FileHeader header;
	if (fwrite(&header, sizeof(FileHeader), 1, file) == 0) return false;

	if (fwrite(&_nlayers, sizeof(unsigned int), 1, file) == 0) return false;

	unsigned int maxsize = 0;
	for (unsigned int i = 0; i < _nlayers; i++)
	{
		if (_layers[i] + 1 > maxsize) maxsize = _layers[i] + 1;
	}
	ir::MemResource<float> buffer = (float*)malloc(maxsize * sizeof(float));
	if (buffer == nullptr) return false;
	memset(buffer, 0, maxsize * sizeof(float));

	for (unsigned int i = 0; i < (_nlayers - 1); i++)
	{
		glBindTexture(GL_TEXTURE_2D, _weights[_switch][i].texture);
		for (unsigned int column = 0; column < (_layers[i] + 1); column++)
		{
			glReadPixels(column, 0, 1, _layers[i + 1], GL_RED, GL_FLOAT, buffer);
			if (fwrite(buffer, sizeof(float), _layers[i + 1], file) < _layers[i + 1]) return false;
		}
	}

	return true;
};

NeuroG::~NeuroG()
{
	if (_forwardprog.program != GL_ERR) glDeleteProgram(_forwardprog.program);
	if (_lastbackwardprog.program != GL_ERR) glDeleteProgram(_lastbackwardprog.program);
	if (_backwardprog.program != GL_ERR) glDeleteProgram(_backwardprog.program);
	if (_corrigateprog.program != GL_ERR) glDeleteProgram(_corrigateprog.program);

	if (_distribute1dvbo != GL_ERR) glDeleteBuffers(1, &_distribute1dvbo);
	if (_distribute1dvao != GL_ERR) glDeleteVertexArrays(1, &_distribute1dvao);
	if (_distribute2dvbo != GL_ERR) glDeleteBuffers(1, &_distribute2dvbo);
	if (_distribute2dvao != GL_ERR) glDeleteVertexArrays(1, &_distribute2dvao);

	if (_goal_texture != GL_ERR) glDeleteTextures(1, &_goal_texture);
	if (_bitinput_texture != GL_ERR) glDeleteTextures(1, &_bitinput_texture);
	
	if (_vectors.data() != nullptr)
	{
		for (unsigned int i = 0; i < _nlayers; i++)
		{
			if (_vectors[i].texture != GL_ERR) glDeleteTextures(1, &_vectors[i].texture);
			if (_vectors[i].framebuffer != GL_ERR) glDeleteFramebuffers(1, &_vectors[i].framebuffer);
		}
		free(_vectors.data());
	}

	if (_errors.data() != nullptr)
	{
		for (unsigned int i = 0; i < (_nlayers - 1); i++)
		{
			if (_errors[i].texture != GL_ERR) glDeleteTextures(1, &_errors[i].texture);
			if (_errors[i].framebuffer != GL_ERR) glDeleteFramebuffers(1, &_errors[i].framebuffer);
		}
		free(_errors.data());
	}

	for (unsigned int a = 0; a < 2; a++)
	{
		if (_weights[a].data() != nullptr)
		{
			for (unsigned int i = 0; i < (_nlayers - 1); i++)
			{
				if (_weights[a][i].texture != GL_ERR) glDeleteTextures(1, &_weights[a][i].texture);
				if (_weights[a][i].framebuffer != GL_ERR) glDeleteFramebuffers(1, &_weights[a][i].framebuffer);
			}
		}
		free(_weights[a].data());
	}

	if (_layers != nullptr) free(_layers);

	glutExit();
};

