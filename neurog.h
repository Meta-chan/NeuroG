/*
	Part of the NeuroG Project. Redistributed under MIT License, which means:
		- Do whatever you want
		- Please keep this notice and include the license file to your project
		- I provide no warranty
	To get help with installation, visit README
	Created by @meta-chan, k.sovailo@gmail.com
	Reinventing bicycles since 2020
*/

#ifndef NEUROG
#define NEUROG

#include <mathg.h>
#include <stdio.h>
#include <random>
#include <ir_syschar.h>

///Ultra-lite GPU-accelerated neuronal network
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
	///Creates the network based on number of neurons in each layer
	///@param nlayers Number of layers
	///@param layers Number of neyrons in each layer, where 0 corresponds input layer and <tt>nlayers - 1</tt> corresponds output layer.
	///@param amplitude Amplitude of values of weights. Weights are initialized with random values from <tt>-amplitude</tt> to <tt>amplitude</tt>.
	///@param ok Pointer to receive if network is ok, may be @c nullptr
	NeuroG(unsigned int nlayers, const unsigned int *layers, float amplitude, bool *ok);
	///Loads the network from file.
	///@param filepath Path to file to load the network from
	///@param ok Pointer to receive if network is ok, may be @c nullptr
	NeuroG(const ir::syschar *filepath, bool *ok);
	///Returns if the network is ok.
	bool ok();
	///Sets the input of the net.
	///@param input Pointer to buffer that contains input of the network. Must be at least <tt>layers[0]</tt> floats size.
	bool set_input(const float *input);
	///Sets the goal i.e. wished result of the forward function.
	///@param goal Pointer to buffer that contains goal of the network. Must be at least <tt>layers[nlayers - 1]</tt> floats size.
	bool set_goal(const float *goal);
	///Sets the learning coefficient.
	///@param coefficient Coefficient to be set
	bool set_coefficient(float coefficient);
	///Sets a pointer to output array.
	///@param output Pointer to buffer that will receive output of the network. Must be at least <tt>layers[nlayers - 1]</tt> size.
	bool set_output_pointer(float *output);
	///Forces the network to put output value of forward into array specified by set_output_pointer.
	bool get_output();
	///Performs forward calculation.
	bool forward();
	///Performs backward learning. Needs to be called after forward.
	bool backward();
	///Saves the network to file
	///@param filepath Path to file to save the network to
	bool save(const ir::syschar *filepath);
	///Destroys the network
	~NeuroG();
};

/**@mainpage Welcome to NeuroG!
*NeuroG is ultra-light GPU-accelerated neuronal network. The architecture is simplest of simplest — a multi-layer perceptron with tanh as activating function.
*
*### Installation
*The installation process is integrated with [Ironic Library](https://github.com/Meta-chan/ironic_library), follow the link detailed explanation. But in a nutshell, you can do just this:
*@code{.cpp}
*#define IR_IMPLEMENT
*#include <neurog.h>
*@endcode
*
*### Dependencies
*NeuroG is fully based on [MathG](https://github.com/Meta-chan/MathG), so please follow the link to know more about installation, dependencies and platforms. Potentially NeuroG can work with minor changes on every device that supports OpenGL API \> 3.0.
*
*### Documentation
*The code is pretty self-documented. But more importantly, I provide [Doxygen](https://www.doxygen.nl/manual/starting.html) documentation! It does not look too pretty since I am not an expert, but it is still quite informative. And of course, feel free to contact me!
*
*###### P.S. My code is not dirty, it is alternatively clean.*/

#if (defined(IR_IMPLEMENT) || defined(IR_NEUROG_IMPLEMENT)) && !defined(IR_NEUROG_NOT_IMPLEMENT)
	#include "neurog_implementation.h"
#endif

#endif