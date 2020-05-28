# Welcome to NeuroG!
 NeuroG is ultra-light GPU-accelerated neuronal network.

### Dependencies
 At the moment NeuroG depends on [FreeGlut](http://freeglut.sourceforge.net) and [Ironic Library](https://github.com/Meta-chan/ironic_lib). FreeGlut can be easily replaced with any equivalent library.

### Platforms
 Currently Windows x86 only, but in perspective can be ported on every platform that provides OpenGL API. In nearest future I plan to port it on Linux x86.

### Architecture
 Simplest of simplest, multi-layer perceptron with tanh as activating function.

### Usage
```c++
class NeuroG
{
	//Build the network based on number of neurons in every layer, given as array
	//Get network status if ok != nullptr
	NeuroG(unsigned int nlayers, const unsigned int *layers, bool *ok);
	//Build the network based on the file
	//Get network status if ok != nullptr
	NeuroG(const wchar_t *filepath, bool *ok);
	//Set input, size of array must be layers[0]
	bool set_input(const float *input);
	//Set goal i.e. wished result, size of array must be layers[nlayers - 1]
	bool set_goal(const float *goal);
	//Set learning coefficient
	bool set_coefficient(float coefficient);
	//Get output, size of array must be layers[nlayers - 1]
	bool get_output(float *output);
	//Perform forward calculation i.e. calculate output based on input and state
	bool forward();
	//Perform backward calculation i.e. train the network based on
	//input, output, goal, coefficient and state
	//Must be called after forward calculation
	bool backward();
	//Save network to file
	bool save(const wchar_t *filepath);
	//Destructor
	~NeuroG();
};
```

###### P.S. My code is not dirty, it is alternatively clean.