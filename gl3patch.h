#ifndef GL3PATHCH
#define GL3PATCH

#define FREEGLUT_STATIC
#include <GL/freeglut.h>

typedef char GLchar; 
typedef size_t GLsizeiptr;

typedef void (__stdcall GlUseProgram) (GLuint program);
typedef void (__stdcall GlBindVertexArray)(GLuint array);
typedef void (__stdcall GlBindBuffer)(GLenum target, GLuint buffer);
typedef GLuint  (__stdcall GlCreateShader)(GLenum shaderType);
typedef void (__stdcall GlShaderSource)(GLuint shader, GLsizei count, const GLchar **string, const GLint *length);
typedef void (__stdcall GlCompileShader)(GLuint shader);
typedef void (__stdcall GlGetShaderiv)(GLuint shader, GLenum pname, GLint *params);
typedef void (__stdcall GlGetShaderInfoLog)(GLuint shader, GLsizei maxLength, GLsizei *length, GLchar *infoLog);
typedef GLuint  (__stdcall GlCreateProgram)(void);
typedef void (__stdcall GlAttachShader)(GLuint program, GLuint shader);
typedef void (__stdcall GlLinkProgram)(GLuint program);
typedef void(__stdcall GlDeleteProgram)(GLuint program);
typedef void (__stdcall GlGetProgramiv)(GLuint program, GLenum pname, GLint *params);
typedef void (__stdcall GlGetProgramInfoLog)(GLuint program, GLsizei maxLength, GLsizei *length, GLchar *infoLog);
typedef void (__stdcall GlDeleteShader)(GLuint shader);
typedef void (__stdcall GlGenVertexArrays)(GLsizei n, GLuint *arrays);
typedef void(__stdcall GlDeleteVertexArrays)(GLsizei n, const GLuint *arrays);
typedef void (__stdcall GlGenBuffers)(GLsizei n, GLuint * buffers);
typedef void(__stdcall GlDeleteBuffers)(GLsizei n, const GLuint * buffers);
typedef void (__stdcall GlBufferData)(GLenum target, GLsizeiptr size, const void * data, GLenum usage);
typedef void (__stdcall GlVertexAttribPointer)(GLuint index, GLint size, GLenum type, GLboolean normalized,	GLsizei stride,	const void * pointer);
typedef void (__stdcall GlEnableVertexAttribArray)(GLuint index);
typedef GLint (__stdcall GlGetUniformLocation)(GLuint program, const GLchar *name);
typedef void (__stdcall GlUniform1fv)(GLint location, GLsizei count, const GLfloat *value);
typedef void (__stdcall GlUniform1i)(GLint location, GLint v0);
typedef void (__stdcall GlGetUniformfv)(GLuint program,	GLint location,	GLfloat *params);
typedef void (__stdcall GlGenFramebuffers)(GLsizei n, GLuint *ids);
typedef void (__stdcall GlBindFramebuffer)(GLenum target, GLuint framebuffer);
typedef void (__stdcall GlFramebufferTexture2D)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
typedef void (__stdcall GlDeleteFramebuffers)(GLsizei n, const GLuint * framebuffers);
typedef void (__stdcall GlActiveTexture)(GLenum texture);

extern GlUseProgram *glUseProgram;
extern GlBindVertexArray *glBindVertexArray;
extern GlBindBuffer *glBindBuffer;
extern GlCreateShader *glCreateShader;
extern GlShaderSource *glShaderSource;
extern GlCompileShader *glCompileShader;
extern GlGetShaderiv *glGetShaderiv;
extern GlGetShaderInfoLog *glGetShaderInfoLog;
extern GlCreateProgram *glCreateProgram;
extern GlAttachShader *glAttachShader;
extern GlLinkProgram *glLinkProgram;
extern GlDeleteProgram *glDeleteProgram;
extern GlGetProgramiv *glGetProgramiv;
extern GlGetProgramInfoLog *glGetProgramInfoLog;
extern GlDeleteShader *glDeleteShader;
extern GlGenVertexArrays *glGenVertexArrays;
extern GlDeleteVertexArrays *glDeleteVertexArrays;
extern GlGenBuffers *glGenBuffers;
extern GlDeleteBuffers *glDeleteBuffers;
extern GlBufferData *glBufferData;
extern GlVertexAttribPointer *glVertexAttribPointer;
extern GlEnableVertexAttribArray *glEnableVertexAttribArray;
extern GlGetUniformLocation *glGetUniformLocation;
extern GlUniform1i *glUniform1i;
extern GlUniform1fv *glUniform1fv;
extern GlGetUniformfv *glGetUniformfv;
extern GlGenFramebuffers *glGenFramebuffers;
extern GlBindFramebuffer *glBindFramebuffer;
extern GlFramebufferTexture2D *glFramebufferTexture2D;
extern GlDeleteFramebuffers *glDeleteFramebuffers;
extern GlActiveTexture *glActiveTexture;

#define GL_ELEMENT_ARRAY_BUFFER	34963
#define GL_VERTEX_SHADER		35633
#define GL_FRAGMENT_SHADER		35632
#define GL_COMPILE_STATUS		35713
#define GL_LINK_STATUS			35714
#define GL_ARRAY_BUFFER			34962
#define GL_ELEMENT_ARRAY_BUFFER	34963
#define GL_STATIC_DRAW			35044
#define GL_FRAMEBUFFER			36160
#define GL_COLOR_ATTACHMENT0	36064
#define GL_DRAW_FRAMEBUFFER		36009
#define GL_READ_FRAMEBUFFER		36008
#define GL_BGRA					0x80E1
#define GL_RGBA32F				34836
#define GL_R32F					33326
#define GL_CLAMP_TO_BORDER		0x812D
#define GL_TEXTURE0				33984

bool gl3patch();

#endif