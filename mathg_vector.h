#ifndef MATHG_VECTOR
#define MATHG_VECTOR

VectorG::VectorG(unsigned int height, bool *ok)
{
	glGenTextures(1, &_texture);
	if (_texture != GL_ERR)
	{
		VectorG *vector = this;
		unsigned int position;
		MathG::_bind_vector(1, &vector, &position);
		glActiveTexture(GL_TEXTURE0 + position);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, 1, height, 0, GL_RED, GL_FLOAT, nullptr);
		_height = height;
		_ok = true;
	}
	if (ok != nullptr) *ok = _ok;
};

unsigned int VectorG::height()
{
	if (!_ok) return 0;
	return _height;
};

bool VectorG::load(float *data)
{
	if (!_ok) return false;
	VectorG *vector = this;
	unsigned int position;
	MathG::_bind_vector(1, &vector, &position);
	glActiveTexture(GL_TEXTURE0 + position);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, data);
	return true;
};

bool VectorG::store(const float *data)
{
	if (!_ok) return false;
	VectorG *vector = this;
	unsigned int position;
	MathG::_bind_vector(1, &vector, &position);
	glActiveTexture(GL_TEXTURE0 + position);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, _height, GL_RED, GL_FLOAT, data);
	return true;
};

VectorG::~VectorG()
{
	if (_texture != GL_ERR)
	{
		MathG::_unbind_vector(this);
		glDeleteTextures(1, &_texture);
	}
	if (_framebuffer != GL_ERR) glDeleteFramebuffers(1, &_framebuffer);
	_ok = false;
};

#endif	//#ifndef MATHG_VECTOR