#ifndef IR_NEUROG_IMPLEMENTATION
#define IR_NEUROG_IMPLEMENTATION

#include <time.h>
#include <share.h>
#include <assert.h>
#include <ir_resource/ir_memresource.h>
#include <ir_resource/ir_file_resource.h>

unsigned int NeuroG::_forward_index			= MG_UNINITED_INDEX;
unsigned int NeuroG::_lastbackward_index	= MG_UNINITED_INDEX;
unsigned int NeuroG::_backward_index		= MG_UNINITED_INDEX;
unsigned int NeuroG::_corrigate_index		= MG_UNINITED_INDEX;

bool NeuroG::_init(unsigned int nlayers, const unsigned int *layers, float amplitude, FILE *file)
{
	if (!MathG::init(true)) return false;

	//Check layers
	_nlayers = nlayers;
	if (nlayers < 2) return false;	//quit if less than 2 layers
	for (unsigned int i = 0; i < nlayers; i++)
	{
		if (layers[i] == 0) return false;	//quit if zero neyrons in layer
	}

	//Initializing _weights (n - 1)
	_weights = (MatrixG2*)malloc((nlayers - 1) * sizeof(MatrixG2));
	if (_weights == nullptr) return false;
	for (unsigned int i = 0; i < (nlayers - 1); i++)
	{
		bool ok;
		new(&_weights[i].m[0])MatrixG(layers[i] + 1, layers[i + 1], &ok);
		if (!ok) return false;
		new(&_weights[i].m[1])MatrixG(layers[i] + 1, layers[i + 1], &ok);
		if (!ok) return false;
	}

	//Initializing _vectors (n)
	_vectors = (VectorG*)malloc(nlayers * sizeof(VectorG));
	if (_vectors == nullptr) return false;
	for (unsigned int i = 0; i < nlayers; i++)
	{
		bool ok;
		new(&_vectors[i]) VectorG(layers[i], &ok);
		if (!ok) return false;
	}

	//Initializing _errors (n - 1)
	_errors = (VectorG*)malloc((nlayers - 1) * sizeof(VectorG));
	if (_errors == nullptr) return false;
	for (unsigned int i = 0; i < (nlayers - 1); i++)
	{
		bool ok;
		new(&_errors[i]) VectorG(layers[i], &ok);
		if (!ok) return false;
	}

	//Initializing _goal
	_goal = (VectorG*)malloc(sizeof(VectorG));
	if (_goal == nullptr) return false;
	bool ok;
	new(_goal) VectorG(layers[_nlayers - 1], &ok);
	if (!ok) return false;

	return true;
};

bool NeuroG::_init_from_file(const ir::syschar *filepath)
{
	#ifdef _WIN32
		ir::FileResource file = _wfsopen(filepath, L"rb", _SH_DENYNO);
	#else
		ir::FileResource file = fopen(filepath, "rb");
	#endif
	if (file == nullptr) return false;

	FileHeader header, sample;
	if (fread(&header, sizeof(FileHeader), 1, file) == 0	||
		memcmp(header.signature, sample.signature, 3) != 0				||
		header.version !=sample.version) return false;

	unsigned int nlayers;
	if (fread(&nlayers, sizeof(unsigned int), 1, file) == 0) return false;

	ir::MemResource<unsigned int> layers = (unsigned int*)malloc(nlayers * sizeof(unsigned int));
	if (fread(layers, sizeof(unsigned int), nlayers, file) < nlayers) return false;

	return _init(nlayers, layers, 1.0f, file);
};

NeuroG::NeuroG(unsigned int nlayers, const unsigned int *layers, float amplitude, bool *ok)
{
	bool r = _init(nlayers, layers, amplitude, nullptr);
	if (ok != nullptr) *ok = r;
};

NeuroG::NeuroG(const ir::syschar *filepath, bool *ok)
{
	bool r = _init_from_file(filepath);
	if (ok != nullptr) *ok = r;
};

bool NeuroG::ok()
{
	return _ok;
};

bool NeuroG::set_input(const float *input)
{
	return _ok && _vectors[0].store(input);
};

bool NeuroG::set_goal(const float *goal)
{
	return _ok && _goal->store(goal);
};

bool NeuroG::set_coefficient(float coefficient)
{
	if (!_ok) return false;
	if (coefficient <= 0.0f) return false;
	_coefficient = coefficient;
	return true;
};

bool NeuroG::set_output_pointer(float *output)
{
	if (!_ok) return false;
	if (output == nullptr) return false;
	_useroutput = output;
	return true;
};

bool NeuroG::get_output()
{
	return _ok && _useroutput != nullptr && _vectors[_nlayers - 1].load(_useroutput);
	return true;
};

bool NeuroG::forward()
{
	if (!_ok) return false;

	//Submitting forward
	if (_forward_index == MG_ERROR_INDEX) return false;
	else if (_forward_index == MG_UNINITED_INDEX)
	{
		MathG::Operation op;
		op.name = "neurog_forward";
		op.argument_number = 3;
		const char *names[3] = {
			"weights",
			"prevvector",
			"prevlength"
		};
		op.argument_names = names;
		MathG::ArgumentType types[3] = {
			MathG::ArgumentType::matrix,
			MathG::ArgumentType::vector,
			MathG::ArgumentType::int_
		};
		op.argument_types = types;
		op.result_type = MathG::ArgumentType::vector;
		op.source = R"(
			#version 330 core

			uniform sampler2D weights;
			uniform sampler2D prevvector;
			uniform int prevlength;

			out float nextvectorelem;

			void main()
			{
				int iposition = int(gl_FragCoord.y - 0.5);
				float sum = texelFetch(weights, ivec2(0, iposition), 0).r;
				for (int i = 0; i < prevlength; i++)
				{
					sum += texelFetch(weights, ivec2(i + 1, iposition), 0).r * texelFetch(prevvector, ivec2(0, i), 0).r;
				}

				nextvectorelem = tanh(sum);
				//nextvectorelem = 0.001 * tanh(sum) - 0.42;
			};
		)";
		op.check = [](MathG::Argument* arguments, ObjectG *result) -> bool
		{
			arguments[2].u = arguments[1].v->height();
			return(arguments[0].m->width() == arguments[1].v->height() + 1
				&& arguments[0].m->height() == ((VectorG*)result)->height());
		};

		_forward_index = MathG::submit(op);
		if (_forward_index == MG_ERROR_INDEX) return false;
	}

	for (unsigned int i = 0; i < (_nlayers - 1); i++)
	{
		MathG::Argument arguments[3];
		arguments[0].m = &_weights[i].m[_switch];
		arguments[1].v = &_vectors[i];
		if (!MathG::perform(_forward_index, arguments, &_vectors[i + 1])) return false;
	}

	return true;
}; 

bool NeuroG::backward()
{
	if (!_ok) return false;

	//Submitting lastbackward
	if (_lastbackward_index == MG_ERROR_INDEX) return false;
	else if (_lastbackward_index == MG_UNINITED_INDEX)
	{
		MathG::Operation op;
		op.name = "neurog_lastbackward";
		op.argument_number = 2;
		const char *names[2] = { "lastvector", "goal" };
		op.argument_names = names;
		MathG::ArgumentType types[2] = { MathG::ArgumentType::vector, MathG::ArgumentType::vector };
		op.argument_types = types;
		op.result_type = MathG::ArgumentType::vector;
		op.source = R"(
			#version 330 core

			uniform sampler2D lastvector;
			uniform sampler2D goal;

			out float lasterrorelem;

			void main()
			{
				int iposition = int(gl_FragCoord.y - 0.5);
				float goalelem = texelFetch(goal, ivec2(0, iposition), 0).r;
				float lastvectorelem = texelFetch(lastvector, ivec2(0, iposition), 0).r;
				lasterrorelem = (1 - lastvectorelem * lastvectorelem) * (goalelem - lastvectorelem);
			};
		)";
		op.check = [](MathG::Argument* arguments, ObjectG *result) -> bool
		{
			return(arguments[0].v->height() == arguments[1].v->height()
				&& arguments[0].v->height() == ((VectorG*)result)->height());
		};

		_lastbackward_index = MathG::submit(op);
		if (_lastbackward_index == MG_ERROR_INDEX) return false;
	}

	//Submitting backward
	if (_backward_index == MG_ERROR_INDEX) return false;
	else if (_backward_index == MG_UNINITED_INDEX)
	{
		MathG::Operation op;
		op.name = "neurog_backward";
		op.argument_number = 4;
		const char *names[4] = {
			"weights",
			"nexterror",
			"prevvector",
			"nextlength"
		};
		op.argument_names = names;
		MathG::ArgumentType types[4] = {
			MathG::ArgumentType::matrix,
			MathG::ArgumentType::vector,
			MathG::ArgumentType::vector,
			MathG::ArgumentType::int_
		};
		op.argument_types = types;
		op.result_type = MathG::ArgumentType::vector;
		op.source = R"(
			#version 330 core
			uniform sampler2D weights;
			uniform sampler2D nexterror;
			uniform sampler2D prevvector;
			uniform int nextlength;

			out float preverrorelem;

			void main()
			{
				int iposition = int(gl_FragCoord.y - 0.5);
				float error = 0.0f;
				for (int i = 0; i < nextlength; i++)
				{
					error += texelFetch(weights, ivec2(iposition + 1, i), 0).r * texelFetch(nexterror, ivec2(0, i), 0).r;
				}
				float prevvectorelem = texelFetch(prevvector, ivec2(0, iposition), 0).r;
				preverrorelem = (1 - prevvectorelem * prevvectorelem) * error;
			};
		)";
		op.check = [](MathG::Argument* arguments, ObjectG *result) -> bool
		{
			arguments[3].u = arguments[1].v->height();
			return(arguments[0].m->width() == arguments[1].v->height() + 1
				&& arguments[0].m->height() == arguments[2].v->height()
				&& arguments[0].m->height() == ((VectorG*)result)->height());
		};

		_backward_index = MathG::submit(op);
		if (_backward_index == MG_ERROR_INDEX) return false;
	}

	//Submitting corrigate
	if (_corrigate_index == MG_ERROR_INDEX) return false;
	else if (_corrigate_index == MG_UNINITED_INDEX)
	{
		MathG::Operation op;
		op.name = "neurog_corrigate";
		op.argument_number = 4;
		const char *names[4] = {
			"weights",
			"nexterror",
			"prevvector",
			"coefficient"
		};
		op.argument_names = names;
		MathG::ArgumentType types[4] = {
			MathG::ArgumentType::matrix,
			MathG::ArgumentType::vector,
			MathG::ArgumentType::vector,
			MathG::ArgumentType::float_
		};
		op.argument_types = types;
		op.result_type = MathG::ArgumentType::matrix;
		op.source = R"(
			#version 330 core

			uniform sampler2D weights;
			uniform sampler2D nexterror;
			uniform sampler2D prevvector;
			uniform float coefficient;

			out float newweightselem;

			void main()
			{
				int ihorizontal = int(gl_FragCoord.x - 0.5);
				int ivertical = int(gl_FragCoord.y - 0.5);
	
				float oldweightselem = texelFetch(weights, ivec2(ihorizontal, ivertical), 0).r;
				float nexterrorelem = texelFetch(nexterror, ivec2(0, ivertical), 0).r;
				if (ihorizontal != 0) nexterrorelem *= texelFetch(prevvector, ivec2(0, ihorizontal - 1), 0).r;
				newweightselem = oldweightselem + coefficient * nexterrorelem;
			};
		)";
		op.check = [](MathG::Argument* arguments, ObjectG *result) -> bool
		{
			return(arguments[0].m->width() == arguments[1].v->height() + 1
				&& arguments[0].m->height() == arguments[2].v->height()
				&& arguments[0].m->height() == ((VectorG*)result)->height());
		};

		_corrigate_index = MathG::submit(op);
		if (_corrigate_index == MG_ERROR_INDEX) return false;
	}

	MathG::Argument arguments[4];
	arguments[0].v = &_vectors[_nlayers - 1];
	arguments[1].v = _goal;
	MathG::perform(_lastbackward_index, arguments, &_errors[_nlayers - 2]);

	for (unsigned int i = _nlayers - 2; i > 0; i--)
	{
		arguments[0].m = &_weights[i].m[_switch];
		arguments[1].v = &_errors[i];
		arguments[2].v = &_vectors[i - 1];
		MathG::perform(_backward_index, arguments, &_errors[i - 1]);
	}

	for (unsigned int i = _nlayers - 1; i > 0; i--)
	{
		arguments[0].m = &_weights[i].m[_switch];
		arguments[1].v = &_errors[i];
		arguments[2].v = &_vectors[i - 1];
		arguments[3].f = _coefficient;
		MathG::perform(_backward_index, arguments, &_weights[i].m[_switch ^ 1]);
	}

	_switch ^= 1;
	return true;
};

bool NeuroG::save(const ir::syschar *filepath)
{
	if (!_ok) return false;

	#ifdef _WIN32
		ir::FileResource file = _wfsopen(filepath, L"wb", _SH_DENYNO);
	#else
		ir::FileResource file = fopen(filepath, "wb");
	#endif

	if (file == nullptr) return false;

	FileHeader header;
	if (fwrite(&header, sizeof(FileHeader), 1, file) == 0) return false;

	if (fwrite(&_nlayers, sizeof(unsigned int), 1, file) == 0) return false;

	unsigned int maxsize = 0;
	for (unsigned int i = 0; i < (_nlayers - 1); i++)
	{
		unsigned int size = _weights[i].m[0].width() * _weights[i].m[0].height();
		if (size > maxsize) maxsize = size;
	}
	ir::MemResource<float> buffer = (float*)malloc(maxsize * sizeof(float));
	if (buffer == nullptr) return false;
	
	for (unsigned int i = 0; i < (_nlayers - 1); i++)
	{
		_weights[i].m[_switch].load(buffer);
		unsigned int size = _weights[i].m[0].width() * _weights[i].m[0].height();
		if (fwrite(buffer, sizeof(float), size, file) < size) return false;
	}

	return true;
};

NeuroG::~NeuroG()
{
	if (_weights != nullptr)
	{
		for (unsigned int i = 0; i < (_nlayers - 1); i++)
		{
			_weights[i].m[0].~MatrixG();
			_weights[i].m[1].~MatrixG();
		}
		free(_weights);
	}

	if (_vectors != nullptr)
	{
		for (unsigned int i = 0; i < _nlayers; i++)
		{
			_vectors[i].~VectorG();
		}
		free(_vectors);
	}

	if (_errors != nullptr)
	{
		for (unsigned int i = 0; i < (_nlayers - 1); i++)
		{
			_errors[i].~VectorG();
		}
		free(_errors);
	}

	if (_goal != nullptr)
	{
		_goal->~VectorG();
		free(_goal);
	}

	MathG::free();
};

#endif	//#ifndef IR_NEUROG_IMPLEMENTATION