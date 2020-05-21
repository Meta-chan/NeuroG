#ifndef NEUROG
#define NEUROG

#define FREEGLUT_STATIC
#include <GL/freeglut.h>
#define GL_ERR ((GLuint)-1)
#include <stdio.h>

class NeuroG
{
private:

	struct FileHeader
	{
		char signature[3];
		unsigned char version;
	};

	struct TF	//Texture and Framebuffer
	{
		GLuint texture		= GL_ERR;
		GLuint framebuffer	= GL_ERR;
	};

	struct TF2	//Double Texture and Framebuffer, for weights
	{
		TF tf[2];
	};

	struct Program
	{
		GLuint program		= GL_ERR;
	};

	struct ForwardProgram : Program
	{
		GLuint weights		= GL_ERR;
		GLuint prevvector	= GL_ERR;
		GLuint prevlength	= GL_ERR;
		GLuint nextlength	= GL_ERR;
	};

	struct LastBackwardProgram : Program
	{
		GLuint lestvector	= GL_ERR;
		GLuint goal			= GL_ERR;
		GLuint lastlength	= GL_ERR;
	};

	struct BackwardProgram : Program
	{
		GLuint weights		= GL_ERR;
		GLuint nexterror	= GL_ERR;
		GLuint prevlength	= GL_ERR;
		GLuint prevvector	= GL_ERR;
		GLuint nextlength	= GL_ERR;
	};

	struct CorrigateProgram : Program
	{
		GLuint weights		= GL_ERR;
		GLuint nexterror	= GL_ERR;
		GLuint prevlength	= GL_ERR;
		GLuint prevvector	= GL_ERR;
		GLuint nextlength	= GL_ERR;
		GLuint koef			= GL_ERR;
	};

	struct Bitwise1dProgram : Program
	{
		GLuint length		= GL_ERR;
		GLuint vector		= GL_ERR;
	};

	struct Bitwise2dProgram : Program
	{
		GLuint width		= GL_ERR;
		GLuint height		= GL_ERR;
		GLuint weights		= GL_ERR;
	};

	bool _ok				= false;
	unsigned int _nlayers	= 0;
	unsigned int *_layers	= nullptr;
	unsigned int _switch	= 0;		//switch between tf[0] and tf[1]
	float _koef				= 0.0f;

	TF2 *_weights			= nullptr;	//n - 1
	TF *_vectors			= nullptr;	//n
	TF *_errors				= nullptr;	//n - 1, because no error for layer 0
	GLuint _byteinput		= GL_ERR;
	GLuint _bytegoal		= GL_ERR;

	GLuint _distribute1dvao = GL_ERR;
	GLuint _distribute1dvbo = GL_ERR;
	GLuint _distribute2dvao = GL_ERR;
	GLuint _distribute2dvbo = GL_ERR;

	ForwardProgram _forwardprog;
	LastBackwardProgram _lastbackwardprog;
	BackwardProgram _backwardprog;
	CorrigateProgram _corrigateprog;
	Bitwise1dProgram _bitwise1dprog;
	Bitwise2dProgram _bitwise2dprog;

	bool _initvectors(unsigned int nlayers, const unsigned int *layers);
	bool _initglut();
	bool _compileshader(const char *filename, bool vertex, GLuint *shader);
	bool _linkprogram(const char *programname, GLuint vertex, GLuint fragment, GLuint *program);
	bool _initprograms();
	bool _initobjects();
	bool _inittextures(unsigned int nlayers, const unsigned int *layers, FILE *file);
	bool _init(const wchar_t *filepath);
	bool _init(unsigned int nlayers, const unsigned int *layers, FILE *file);

public:
	NeuroG(unsigned int nlayers, const unsigned int *layers, bool *ok);
	NeuroG(const wchar_t *filepath, bool *ok);
	bool set_input(const float *input);
	bool set_goal(const float *goal);
	bool set_koef(float koef);
	bool get_output(float *output);
	bool forward();
	bool backward();
	bool save(const wchar_t *filepath);
	~NeuroG();
};

#endif