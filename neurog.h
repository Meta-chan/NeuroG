#ifndef NEUROG
#define NEUROG

#include <mathg.h>
#include <stdio.h>
#include <random>
#include <ir_syschar.h>

class NeuroG
{
private:

	struct FileHeader
	{
		char signature[3]		= { 'I', 'N', 'R' };
		unsigned char version	= 0;
	};

	struct MatrixG2
	{
		MatrixG m[2];
	};

	static unsigned int _forward_index;
	static unsigned int _lastbackward_index;
	static unsigned int _backward_index;
	static unsigned int _corrigate_index;
	
	bool _ok					= false;
	unsigned int _nlayers		= 0;
	unsigned int _switch		= 0;
	float _coefficient			= 0.0f;
	float *_useroutput			= nullptr;
	
	MatrixG2 *_weights			= nullptr;
	VectorG *_vectors			= nullptr;
	VectorG *_errors			= nullptr;
	VectorG *_goal				= nullptr;

	typedef bool ReadFunction(void *user, unsigned int count, float *buffer);
	bool _init(unsigned int nlayers, const unsigned int *layers, void *user, ReadFunction *function);
	bool _init_from_file(const ir::syschar *filepath);

public:
	NeuroG(unsigned int nlayers, const unsigned int *layers, float amplitude, bool *ok);
	NeuroG(const ir::syschar *filepath, bool *ok);
	bool ok();
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

#if (defined(IR_IMPLEMENT) || defined(IR_NEUROG_IMPLEMENT)) && !defined(IR_NEUROG_NOT_IMPLEMENT)
	#include "neurog_implementation.h"
#endif

#endif