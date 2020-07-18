#ifndef MATHG
#define MATHG

#define FREEGLUT_STATIC
#include <GL/freeglut.h>

#define GL_ERR ((GLuint)-1)

class MathG;

class ObjectG
{
	friend MathG;

protected:
	unsigned int _permanent = 0xFFFFFFFF;
	GLuint _texture			= GL_ERR;
	GLuint _framebuffer		= GL_ERR;
	bool _ok				= false;
};

class VectorG : public ObjectG
{
	friend MathG;

private:
	unsigned int _height	= 0;
	VectorG(VectorG &vector);
	
public:
	VectorG(unsigned int height, bool *ok);
	unsigned int height();
	bool load(float *data);
	bool store(const float *data);
	~VectorG();
};

class MatrixG : public ObjectG
{
	friend MathG;

private:
	unsigned int _width		= 0;
	unsigned int _height	= 0;
	MatrixG(MatrixG &matrix);

public:
	MatrixG(unsigned int width, unsigned int height, bool *ok);
	unsigned int width();
	unsigned int height();
	bool load(float *data);
	//bool load_line(unsigned int line, float *data);
	//bool load_column(unsigned int column, float *data);
	bool store(const float *data);
	bool store_line(unsigned int line, const float *data);
	bool store_column(unsigned int column, const float *data);
	~MatrixG();
};

class MathG
{
	friend ObjectG;
	friend VectorG;
	friend MatrixG;

private:
	static bool _ok;
	static int _window;
	static bool *_permanent;
	static GLuint *_temporary;
	static unsigned int _npermanent;
	static unsigned int _ntemporary;

	struct Object
	{
		static GLuint _vao1d;
		static GLuint _vbo1d;
		static GLuint _vao2d;
		static GLuint _vbo2d;
	};

	struct Shader
	{
		static GLuint _distribute1d;
		static GLuint _distribute2d;
	};

	struct Program
	{
		struct Add { static GLuint _program; static GLint _a; static GLint _b; };
	};

	struct Source
	{
		static const char *_distribute1d;
		static const char *_distribute2d;
		static const char *_add;
		static const char *_substract;
		static const char *_multiply;
	};

	static bool _get_status(const char *name, bool link);
	static bool _compile_distribute1d();
	static bool _compile_add();

	static void _bind_vector(unsigned int count, VectorG **vectors, unsigned int *positions);
	static void _unbind_vector(VectorG *vector);
	static void _bind_matrix(unsigned int count, MatrixG **matrixes, unsigned int *positions);
	static void _unbind_matrix(MatrixG *matrix);

public:
	static bool init(bool print);

	//Math functions
	static bool add(VectorG *a, VectorG *b, VectorG *output);
	static bool substract(VectorG *a, VectorG *b, VectorG *output);
	static bool multiply(MatrixG *a, VectorG *b, VectorG *output);

	//NeuroG-specific functions
	static bool neuro_forawrd_tanh(MatrixG *matrix, VectorG *input, VectorG *output);
	static bool neuro_forawrd_tanh_adjust(MatrixG *matrix, VectorG *input, VectorG *output);
	static bool neuro_backward_tanh(MatrixG *matrix, VectorG *input, VectorG *output);
	static bool neuro_backward_tanh_adjust(MatrixG *matrix, VectorG *input, VectorG *output);
	static bool neuro_corrigate(MatrixG *matrix, VectorG *input, VectorG *output);
	static bool neuro_corrigate_adjust(MatrixG *matrix, VectorG *input, VectorG *output);

	static bool free();
};

#include "mathg_implementation.h"

#endif	//#ifndef MATHG