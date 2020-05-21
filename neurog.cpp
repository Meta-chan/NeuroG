#include "neurog.h"
#include "gl3patch.h"
#include <shellapi.h>
#include <time.h>
#include <random>
#include <share.h>
#define IR_IMPLEMENT
#include <ir_resource/ir_memresource.h>
#include <ir_resource/ir_file_resource.h>
#include <ir_resource/ir_shader_resource.h>

bool NeuroG::_initglut()
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

bool NeuroG::_compileshader(const char *filename, bool vertex, GLuint *shader)
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

bool NeuroG::_linkprogram(const char *programname, GLuint vertex, GLuint fragment, GLuint *program)
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

bool NeuroG::_initprograms()
{
	ir::ShaderResource distribute1d, distribute2d, forward, lastbackward, backward, corrigate, bitwise1d, bitwise2d;

	//Compiling shaders
	if (!_compileshader("vertex_distribute_1d.c", true, &distribute1d.value())					||
		!_compileshader("vertex_distribute_2d.c", true, &distribute2d.value())					||
		!_compileshader("fragment_forward.c", false, &forward.value())							||
		!_compileshader("fragment_lastbackward.c", false, &lastbackward.value())				||
		!_compileshader("fragment_backward.c", false, &backward.value())						||
		!_compileshader("fragment_corrigate.c", false, &corrigate.value())						||
		!_compileshader("fragment_bitwise_1d.c", false, &bitwise1d.value())						||
		!_compileshader("fragment_bitwise_2d.c", false, &bitwise2d.value())) return false;
	
	//Linking programs
	if (!_linkprogram("forward", distribute1d, forward, &_forwardprog.program)					||
		!_linkprogram("lastbackward", distribute1d, lastbackward, &_lastbackwardprog.program)	||
		!_linkprogram("backward", distribute1d, backward, &_backwardprog.program)				||
		!_linkprogram("corrigate", distribute2d, corrigate, &_corrigateprog.program)			||
		!_linkprogram("bitwise_1d", distribute1d, bitwise1d, &_bitwise1dprog.program)			||
		!_linkprogram("bitwise_2d", distribute2d, bitwise2d, &_bitwise2dprog.program)) return false;
	
	//getting addresses of uniforms
	if ((_forwardprog.nextlength = glGetUniformLocation(_forwardprog.program, "nextlength"))			== GL_ERR ||
		(_forwardprog.prevlength = glGetUniformLocation(_forwardprog.program, "prevlength"))			== GL_ERR ||
		(_forwardprog.prevvector = glGetUniformLocation(_forwardprog.program, "prevvector"))			== GL_ERR ||
		(_forwardprog.weights = glGetUniformLocation(_forwardprog.program, "weights"))					== GL_ERR ||

		(_lastbackwardprog.goal = glGetUniformLocation(_lastbackwardprog.program, "goal"))				== GL_ERR ||
		(_lastbackwardprog.lastlength = glGetUniformLocation(_lastbackwardprog.program, "lastlength"))	== GL_ERR ||
		(_lastbackwardprog.lestvector = glGetUniformLocation(_lastbackwardprog.program, "lastvector"))	== GL_ERR ||

		(_backwardprog.nextlength = glGetUniformLocation(_backwardprog.program, "nextlength"))			== GL_ERR ||
		(_backwardprog.nexterror = glGetUniformLocation(_backwardprog.program, "nexterror"))			== GL_ERR ||
		(_backwardprog.prevlength = glGetUniformLocation(_backwardprog.program, "prevlength"))			== GL_ERR ||
		(_backwardprog.prevvector = glGetUniformLocation(_backwardprog.program, "prevvector"))			== GL_ERR ||
		(_backwardprog.weights = glGetUniformLocation(_backwardprog.program, "weights"))				== GL_ERR ||

		(_corrigateprog.koef = glGetUniformLocation(_corrigateprog.program, "koef"))					== GL_ERR ||
		(_corrigateprog.nextlength = glGetUniformLocation(_corrigateprog.program, "nextlength"))		== GL_ERR ||
		(_corrigateprog.nexterror = glGetUniformLocation(_corrigateprog.program, "nexterror"))			== GL_ERR ||
		(_corrigateprog.prevlength = glGetUniformLocation(_corrigateprog.program, "prevlength"))		== GL_ERR ||
		(_corrigateprog.prevvector = glGetUniformLocation(_corrigateprog.program, "prevvector"))		== GL_ERR ||
		(_corrigateprog.weights = glGetUniformLocation(_corrigateprog.program, "weights"))				== GL_ERR ||
		
		(_bitwise1dprog.length = glGetUniformLocation(_bitwise1dprog.program, "length"))				== GL_ERR ||
		(_bitwise1dprog.vector = glGetUniformLocation(_bitwise1dprog.program, "vector"))				== GL_ERR ||
		
		(_bitwise2dprog.height = glGetUniformLocation(_bitwise2dprog.program, "height"))				== GL_ERR ||
		(_bitwise2dprog.weights = glGetUniformLocation(_bitwise2dprog.program, "weights"))				== GL_ERR ||
		(_bitwise2dprog.width = glGetUniformLocation(_bitwise2dprog.program, "width"))					== GL_ERR)
		return false;
	
	return true;
};

bool NeuroG::_initvectors(unsigned int nlayers, const unsigned int *layers)
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
	_weights = (TF2*)malloc((nlayers - 1) * sizeof(TF2));
	if (_weights == nullptr) return false;
	
	//Initializing _vectors (n)
	_vectors = (TF*)malloc(nlayers * sizeof(TF));
	if (_vectors == nullptr) return false;

	//Initializing _errors (n - 1)
	_errors = (TF*)malloc((nlayers - 1) * sizeof(TF));
	if (_errors == nullptr) return false;
		
	return true;
};

bool NeuroG::_inittextures(unsigned int nlayers, const unsigned int *layers, FILE *file)
{
	//Allocating maximal buffer
	unsigned int maxsize = 0;
	for (unsigned int i = 0; i < (nlayers - 1); i++)
	{
		if ((layers[i] + 1) * layers[i + 1] > maxsize) maxsize = (layers[i] + 1) * layers[i + 1];
	}
	ir::MemResource<float> buffer = (float*)malloc(maxsize * sizeof(float));
	if (buffer == nullptr) return false;
	memset(buffer, 0, maxsize * sizeof(float));

	//Initializing _vectors and _framebuffers
	for (unsigned int i = 0; i < nlayers; i++)
	{
		glGenFramebuffers(1, &_vectors[i].framebuffer);
		if (_vectors[i].framebuffer == GL_ERR) return false;
		glBindFramebuffer(GL_FRAMEBUFFER, _vectors[i].framebuffer);
		
		glGenTextures(1, &_vectors[i].texture);
		if (_vectors[i].texture == GL_ERR) return false;
		glBindTexture(GL_TEXTURE_2D, _vectors[i].texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, 1, layers[i], 0, GL_RED, GL_FLOAT, buffer);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _vectors[i].texture, 0);
		glViewport(0, 0, 1, layers[i]);
	}

	//Initializing _errors
	for (unsigned int i = 0; i < (nlayers - 1); i++)
	{
		glGenFramebuffers(1, &_errors[i].framebuffer);
		if (_errors[i].framebuffer == GL_ERR) return false;
		glBindFramebuffer(GL_FRAMEBUFFER, _errors[i].framebuffer);
		
		glGenTextures(1, &_errors[i].texture);
		if (_errors[i].texture == GL_ERR) return false;
		glBindTexture(GL_TEXTURE_2D, _errors[i].texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, 1, layers[i + 1], 0, GL_RED, GL_FLOAT, buffer);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _errors[i].texture, 0);
		glViewport(0, 0, 1, layers[i + 1]);
	}

	//Initializing _buyteinput
	glGenTextures(1, &_byteinput);
	if (_byteinput == GL_ERR) return false;
	glBindTexture(GL_TEXTURE_2D, _byteinput);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, layers[0], 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);

	//Initializing _bytegoal
	glGenTextures(1, &_bytegoal);
	if (_bytegoal == GL_ERR) return false;
	glBindTexture(GL_TEXTURE_2D, _bytegoal);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, layers[_nlayers - 1], 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);

	//Initializing weights
	std::default_random_engine generator;
	generator.seed((unsigned int)time(nullptr));
	std::uniform_real_distribution<float> distribution(-1.0f, 1.0f);

	glUseProgram(_bitwise2dprog.program);
	glBindVertexArray(_distribute1dvao);
	glActiveTexture(GL_TEXTURE0);
	glUniform1i(_bitwise2dprog.weights, 0);
	for (unsigned int i = 0; i < (nlayers - 1); i++)
	{
		if (file == nullptr)
		{
			for (unsigned int j = 0; j < (layers[i] + 1) * layers[i + 1]; j++)
			{
				buffer[j] = distribution(generator);
			}
		}
		else
		{
			unsigned int matrixsize = (layers[i] + 1) * layers[i + 1];
			if (fread(buffer, sizeof(float), matrixsize, file) < matrixsize) return false;
		}

		for (int a = 1; a > 0; a--)
		{
			glGenFramebuffers(1, &_weights[i].tf[a].framebuffer);
			if (_weights[i].tf[a].framebuffer == GL_ERR) return false;
			glBindFramebuffer(GL_FRAMEBUFFER, _weights[i].tf[a].framebuffer);

			glGenTextures(1, &_weights[i].tf[a].texture);
			if (_weights[i].tf[a].texture == GL_ERR) return false;
			glBindTexture(GL_TEXTURE_2D, _weights[i].tf[a].texture);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			if (a == 0) glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, layers[i] + 1, layers[i + 1], 0, GL_RED, GL_FLOAT, buffer);
			else glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, layers[i] + 1, layers[i + 1], 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);

			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _weights[i].tf[a].texture, 0);
			glViewport(0, 0, layers[i] + 1, layers[i + 1]);
		}

		glBindTexture(GL_TEXTURE_2D, _weights[i].tf[1].texture);
		glUniform1i(_bitwise2dprog.width, layers[i] + 1);
		glUniform1i(_bitwise2dprog.height, layers[i + 1]);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glFinish();

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, layers[i] + 1, layers[i + 1], 0, GL_RED, GL_FLOAT, buffer);
	}

	return true;
};

bool NeuroG::_initobjects()
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
	GLfloat d2[] =	{	1.0f,	1.0f,
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

bool NeuroG::_init(unsigned int nlayers, const unsigned int *layers, FILE *file)
{
	if (!_initglut()							||
		!_initprograms()						||
		!_initvectors(nlayers, layers)			||
		!_inittextures(nlayers, layers, file)	||
		!_initobjects()) return false;
	
	_ok = true;
	return true;
};

bool NeuroG::_init(const wchar_t *filepath)
{
	ir::FileResource file = _wfsopen(filepath, L"rb", _SH_DENYNO);
	if (file == nullptr) return false;

	FileHeader header;
	if (fread(&header, sizeof(FileHeader), 1, file) == 0	||
		memcmp(header.signature, "INR", 3) != 0				||
		header.version != 0) return false;

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
	bool r = _init(filepath);
	if (ok != nullptr) *ok = r;
};

bool NeuroG::set_input(const float *input)
{
	if (!_ok) return false;
	
	//program
	glUseProgram(_bitwise1dprog.program);
	glBindVertexArray(_distribute1dvao);
	glUniform1i(_bitwise1dprog.length, _layers[0]);
	
	//vector
	glBindTexture(GL_TEXTURE_2D, _byteinput);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, _layers[0], 0, GL_RGBA, GL_UNSIGNED_BYTE, input);
	glActiveTexture(GL_TEXTURE0);
	glUniform1i(_bitwise1dprog.vector, 0);

	//framebuffer
	glBindFramebuffer(GL_TEXTURE_2D, _vectors[0].framebuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _vectors[0].texture, 0);
	glViewport(0, 0, 1, _layers[0]);

	//calculate
	glDrawArrays(GL_LINES, 0, 2);
	glFinish();
	
	return true;
};

bool NeuroG::set_goal(const float *goal)
{
	if (!_ok) return false;
	glBindTexture(GL_TEXTURE_2D, _bytegoal);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, _layers[_nlayers - 1], 0, GL_RGBA, GL_UNSIGNED_BYTE, goal);
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
		glUniform1i(_forwardprog.nextlength, _layers[i + 1]);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, _vectors[i].texture);
		glUniform1i(_forwardprog.prevvector, 0);
		
		glActiveTexture(GL_TEXTURE0 + 1);
		glBindTexture(GL_TEXTURE_2D, _weights[i].tf[_switch].texture);
		glUniform1i(_forwardprog.weights, 1);
		
		glBindFramebuffer(GL_TEXTURE_2D, _vectors[i + 1].framebuffer);
		//WTF? Do I need to initialize framebuffers?
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _vectors[i + 1].texture, 0);
		glViewport(0, 0, 1, _layers[i + 1]);
		glDrawArrays(GL_LINES, 0, 2);
		glFinish();
	}

	return true;
}; 

bool NeuroG::backward()
{
	if (!_ok) return false;

	glUseProgram(_lastbackwardprog.program);
	glBindVertexArray(_distribute1dvao);
	glUniform1i(_lastbackwardprog.lastlength, _layers[_nlayers - 1]);
	//last vector
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _vectors[_nlayers - 1].texture);
	glUniform1i(_lastbackwardprog.lestvector, 0);
	//goal
	glActiveTexture(GL_TEXTURE0 + 1);
	glBindTexture(GL_TEXTURE_2D, _bytegoal);
	glUniform1i(_lastbackwardprog.goal, 1);
	//last error
	glBindFramebuffer(GL_TEXTURE_2D, _errors[_nlayers - 2].framebuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _errors[_nlayers - 2].texture, 0);
	glViewport(0, 0, 1, _layers[_nlayers - 1]);
	//calculating
	glDrawArrays(GL_LINES, 0, 2);
	glFinish();

	if (_nlayers > 2) glUseProgram(_backwardprog.program);
	for (unsigned int i = _nlayers - 2; i > 0; i--)
	{
		glUniform1i(_backwardprog.nextlength, _layers[i]);
		glUniform1i(_backwardprog.prevlength, _layers[i - 1]);

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
		glBindTexture(GL_TEXTURE_2D, _weights[i].tf[_switch].texture);
		glUniform1i(_backwardprog.weights, 2);
		//previous error
		glBindFramebuffer(GL_TEXTURE_2D, _errors[i - 1].framebuffer);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _errors[i - 1].texture, 0);
		glViewport(0, 0, 1, _layers[i]);
		//calculating
		glDrawArrays(GL_LINES, 0, 2);
		glFinish();
	}

	glUseProgram(_corrigateprog.program);
	glBindVertexArray(_distribute2dvao);
	glUniform1fv(_corrigateprog.koef, 1, &_koef);
	for (unsigned int i = _nlayers - 1; i > 0; i--)
	{
		glUniform1i(_corrigateprog.nextlength, _layers[i]);
		glUniform1i(_corrigateprog.prevlength, _layers[i - 1]);

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
		glBindTexture(GL_TEXTURE_2D, _weights[i - 1].tf[_switch].texture);
		glUniform1i(_corrigateprog.weights, 2);
		//new weights
		glBindFramebuffer(GL_TEXTURE_2D, _weights[i - 1].tf[_switch ^ 1].framebuffer);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _weights[i - 1].tf[_switch ^ 1].texture, 0);
		glViewport(0, 0, _layers[i - 1] + 1, _layers[i]);
		//calculating
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glFinish();
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
	memcpy(header.signature, "INR", 3);
	header.version = 0;
	if (fwrite(&header, sizeof(FileHeader), 1, file) == 0) return false;

	if (fwrite(&_nlayers, sizeof(unsigned int), 1, file) == 0) return false;

	unsigned int maxsize = 0;
	for (unsigned int i = 0; i < (_nlayers - 1); i++)
	{
		if ((_layers[i] + 1) * _layers[i + 1] > maxsize) maxsize = (_layers[i] + 1) * _layers[i + 1];
	}
	ir::MemResource<float> buffer = (float*)malloc(maxsize * sizeof(float));
	if (buffer == nullptr) return false;
	memset(buffer, 0, maxsize * sizeof(float));

	for (unsigned int i = 0; i < (_nlayers - 1); i++)
	{
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, buffer);
		unsigned int matrixsize = (_layers[i] + 1) * _layers[i + 1];
		if (fwrite(buffer, sizeof(float), matrixsize, file) < matrixsize) return false;
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

	if (_bytegoal != GL_ERR) glDeleteTextures(1, &_bytegoal);

	if (_vectors != nullptr)
	{
		for (unsigned int i = 0; i < _nlayers; i++)
		{
			if (_vectors[i].texture != GL_ERR) glDeleteTextures(1, &_vectors[i].texture);
			if (_vectors[i].framebuffer != GL_ERR) glDeleteFramebuffers(1, &_vectors[i].framebuffer);
		}
		free(_vectors);
	}

	if (_errors != nullptr)
	{
		for (unsigned int i = 0; i < (_nlayers - 1); i++)
		{
			if (_errors[i].texture != GL_ERR) glDeleteTextures(1, &_errors[i].texture);
			if (_errors[i].framebuffer != GL_ERR) glDeleteFramebuffers(1, &_errors[i].framebuffer);
		}
		free(_errors);
	}

	if (_weights != nullptr)
	{
		for (unsigned int i = 0; i < (_nlayers - 1); i++)
		{
			for (unsigned int a = 0; i < 1; a++)
			{
				if (_weights[i].tf[a].texture != GL_ERR) glDeleteTextures(1, &_weights[i].tf[a].texture);
				if (_weights[i].tf[a].framebuffer != GL_ERR) glDeleteFramebuffers(1, &_weights[i].tf[a].framebuffer);
			}
		}
		free(_weights);
	}

	if (_layers != nullptr) free(_layers);

	glutExit();
};

