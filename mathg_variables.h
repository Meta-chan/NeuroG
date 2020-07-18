#ifndef MATHG_VARIABLES
#define MATHG_VARIABLES

bool MathG::_ok = false;
int MathG::_window = 0;
bool *MathG::_permanent = nullptr;
GLuint *MathG::_temporary = nullptr;
unsigned int MathG::_npermanent = 0;
unsigned int MathG::_ntemporary = 0;

GLuint MathG::Object::_vao1d = GL_ERR;
GLuint MathG::Object::_vbo1d = GL_ERR;
GLuint MathG::Object::_vao2d = GL_ERR;
GLuint MathG::Object::_vbo2d = GL_ERR;

GLuint MathG::Shader::_distribute1d = GL_ERR;
GLuint MathG::Shader::_distribute2d = GL_ERR;

GLuint MathG::Program::Add::_program = GL_ERR;
GLint MathG::Program::Add::_a = -1;
GLint MathG::Program::Add::_b = -1;


#endif	//#ifndef MATHG_VARIABLES