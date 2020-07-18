GlUseProgram *glUseProgram;
GlBindVertexArray *glBindVertexArray;
GlBindBuffer *glBindBuffer;
GlCreateShader *glCreateShader;
GlShaderSource *glShaderSource;
GlCompileShader *glCompileShader;
GlGetShaderiv *glGetShaderiv;
GlGetShaderInfoLog *glGetShaderInfoLog;
GlCreateProgram *glCreateProgram;
GlAttachShader *glAttachShader;
GlLinkProgram *glLinkProgram;
GlDeleteProgram *glDeleteProgram;
GlGetProgramiv *glGetProgramiv;
GlGetProgramInfoLog *glGetProgramInfoLog;
GlDeleteShader *glDeleteShader;
GlGenVertexArrays *glGenVertexArrays;
GlDeleteVertexArrays *glDeleteVertexArrays;
GlGenBuffers *glGenBuffers;
GlDeleteBuffers *glDeleteBuffers;
GlBufferData *glBufferData;
GlVertexAttribPointer *glVertexAttribPointer;
GlEnableVertexAttribArray *glEnableVertexAttribArray;
GlGetUniformLocation *glGetUniformLocation;
GlUniform1i *glUniform1i;
GlUniform1fv *glUniform1fv;
GlGetUniformfv *glGetUniformfv;
GlGenFramebuffers *glGenFramebuffers;
GlBindFramebuffer *glBindFramebuffer;
GlFramebufferTexture2D *glFramebufferTexture2D;
GlDeleteFramebuffers *glDeleteFramebuffers;
GlActiveTexture *glActiveTexture;

bool gl3patch()
{
return (((glGetUniformLocation = (GlGetUniformLocation*)wglGetProcAddress("glGetUniformLocation")) != nullptr)
	&& ((glUniform1i = (GlUniform1i*)wglGetProcAddress("glUniform1i")) != nullptr)
	&& ((glUniform1fv = (GlUniform1fv*)wglGetProcAddress("glUniform1fv")) != nullptr)
	&& ((glGetUniformfv = (GlGetUniformfv*)wglGetProcAddress("glGetUniformfv")) != nullptr)
	&& ((glUseProgram = (GlUseProgram*)wglGetProcAddress("glUseProgram")) != nullptr)
	&& ((glBindVertexArray = (GlBindVertexArray*)wglGetProcAddress("glBindVertexArray")) != nullptr)
	&& ((glBindBuffer = (GlBindBuffer*)wglGetProcAddress("glBindBuffer")) != nullptr)
	&& ((glCreateShader = (GlCreateShader*)wglGetProcAddress("glCreateShader")) != nullptr)
	&& ((glShaderSource = (GlShaderSource*)wglGetProcAddress("glShaderSource")) != nullptr)
	&& ((glCompileShader = (GlCompileShader*)wglGetProcAddress("glCompileShader")) != nullptr)
	&& ((glGetShaderiv = (GlGetShaderiv*)wglGetProcAddress("glGetShaderiv")) != nullptr)
	&& ((glGetShaderInfoLog = (GlGetProgramInfoLog*)wglGetProcAddress("glGetShaderInfoLog")) != nullptr)
	&& ((glCreateProgram = (GlCreateProgram*)wglGetProcAddress("glCreateProgram")) != nullptr)
	&& ((glAttachShader = (GlAttachShader*)wglGetProcAddress("glAttachShader")) != nullptr)
	&& ((glLinkProgram = (GlLinkProgram*)wglGetProcAddress("glLinkProgram")) != nullptr)
	&& ((glDeleteProgram = (GlDeleteProgram*)wglGetProcAddress("glDeleteProgram")) != nullptr)
	&& ((glGetProgramiv = (GlGetProgramiv*)wglGetProcAddress("glGetProgramiv")) != nullptr)
	&& ((glGetProgramInfoLog = (GlGetProgramInfoLog*)wglGetProcAddress("glGetProgramInfoLog")) != nullptr)
	&& ((glDeleteShader = (GlDeleteShader*)wglGetProcAddress("glDeleteShader")) != nullptr)
	&& ((glGenVertexArrays = (GlGenVertexArrays*)wglGetProcAddress("glGenVertexArrays")) != nullptr)
	&& ((glDeleteVertexArrays = (GlDeleteVertexArrays*)wglGetProcAddress("glDeleteVertexArrays")) != nullptr)
	&& ((glGenBuffers = (GlGenBuffers*)wglGetProcAddress("glGenBuffers")) != nullptr)
	&& ((glDeleteBuffers = (GlDeleteBuffers*)wglGetProcAddress("glDeleteBuffers")) != nullptr)
	&& ((glBufferData = (GlBufferData*)wglGetProcAddress("glBufferData")) != nullptr)
	&& ((glVertexAttribPointer = (GlVertexAttribPointer*)wglGetProcAddress("glVertexAttribPointer")) != nullptr)
	&& ((glEnableVertexAttribArray = (GlEnableVertexAttribArray*)wglGetProcAddress("glEnableVertexAttribArray")) != nullptr)
	&& ((glGenFramebuffers = (GlGenFramebuffers*)wglGetProcAddress("glGenFramebuffers")) != nullptr)
	&& ((glBindFramebuffer = (GlBindFramebuffer*)wglGetProcAddress("glBindFramebuffer")) != nullptr)
	&& ((glFramebufferTexture2D = (GlFramebufferTexture2D*)wglGetProcAddress("glFramebufferTexture2D")) != nullptr)
	&& ((glDeleteFramebuffers = (GlDeleteFramebuffers*)wglGetProcAddress("glDeleteFramebuffers")) != nullptr)
	&& ((glActiveTexture = (GlActiveTexture*)wglGetProcAddress("glActiveTexture")) != nullptr));
}