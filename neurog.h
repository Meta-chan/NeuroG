#ifndef NEUROG
#define NEUROG

#define FREEGLUT_STATIC
#include <GL/freeglut.h>
#define GL_ERR ((GLuint)-1)
#include <stdio.h>
#include <random>

#include <ir_syschar.h>

#include "assert_pointer.h"

class NeuroG
{
private:

	struct FileHeader
	{
		char signature[3]		= { 'I', 'N', 'R' };
		unsigned char version	= 0;
	};

	struct TF
	{
		GLuint texture			= GL_ERR;
		GLuint framebuffer		= GL_ERR;
	};

	struct Program
	{
		GLuint program			= GL_ERR;
	};

	struct ForwardProgram : Program
	{
		GLuint weights			= GL_ERR;
		GLuint prevvector		= GL_ERR;
		GLuint prevlength		= GL_ERR;
	};

	struct LastBackwardProgram : Program
	{
		GLuint lestvector		= GL_ERR;
		GLuint goal				= GL_ERR;
	};

	struct BackwardProgram : Program
	{
		GLuint weights			= GL_ERR;
		GLuint nexterror		= GL_ERR;
		GLuint prevvector		= GL_ERR;
		GLuint nextlength		= GL_ERR;
	};

	struct CorrigateProgram : Program
	{
		GLuint weights			= GL_ERR;
		GLuint nexterror		= GL_ERR;
		GLuint prevvector		= GL_ERR;
		GLuint coefficient		= GL_ERR;
	};

	struct Bitwise1dProgram : Program
	{
		GLuint vector			= GL_ERR;
	};

	struct Bitwise2dProgram : Program
	{
		GLuint weights			= GL_ERR;
	};
	
	bool _ok					= false;
	unsigned int _nlayers		= 0;
	unsigned int *_layers		= nullptr;
	unsigned int _switch		= 0;		//switch between textures
	float _coefficient			= 0.0f;
	float *_useroutput			= nullptr;
	bool _hardware_direct_store	= false;
	
	//Extended vectors/framebuffers are needed to
	//perform bitwise store operation
	GLuint _goal_texture		= GL_ERR;	//BITWISE! It is supposed to be so! At least yet
	AssertPointer<TF> _weights[2];
	AssertPointer<TF> _vectors;				//_vectors[0].framebuffer is extended
	AssertPointer<TF> _errors;
	
	GLuint _bitinput_texture	= GL_ERR;	//extended
	
	GLuint _distribute1dvao		= GL_ERR;
	GLuint _distribute1dvbo		= GL_ERR;
	GLuint _distribute2dvao		= GL_ERR;
	GLuint _distribute2dvbo		= GL_ERR;

	ForwardProgram _forwardprog;
	LastBackwardProgram _lastbackwardprog;
	BackwardProgram _backwardprog;
	CorrigateProgram _corrigateprog;
	Bitwise1dProgram _bitwise1dprog;
	Bitwise2dProgram _bitwise2dprog;

	bool _init_glut();
	bool _compile_shader(const char *name, const char *source, bool vertex, GLuint *shader);
	bool _link_program(const char *programname, GLuint vertex, GLuint fragment, GLuint *program);
	bool _init_programs();
	bool _init_objects();
	bool _create_texture(GLuint *texture, unsigned int width, unsigned int height, bool bitwise);
	bool _create_framebuffer(GLuint *framebuffer, GLuint texture, unsigned int width, unsigned int height);
	bool _fill_column(unsigned int height,
		float *data, FILE *file, float amplitude, std::default_random_engine* generator);
	bool _store(GLuint texture, unsigned int width, unsigned int height,
		float *data, FILE *file, float amplitude, std::default_random_engine* generator, GLuint framebuffer, GLuint bittexture);
	bool _init_test();
	bool _init_vectors(unsigned int nlayers, const unsigned int *layers);
	bool _init_textures(float amplitude, FILE *file);
	bool _init(unsigned int nlayers, const unsigned int *layers, float amplitude, FILE *file);
	bool _init_from_file(const ir::syschar *filepath);

public:
	NeuroG(unsigned int nlayers, const unsigned int *layers, float amplitude, bool *ok);
	NeuroG(const ir::syschar *filepath, bool *ok);
	bool set_input(const float *input);
	bool set_goal(const float *goal);
	bool set_coefficient(float coefficient);
	bool set_output_pointer(float *output);
	bool get_output();
	bool forward();
	bool backward();
	bool save(const ir::syschar *filepath);
	~NeuroG();
};

#endif