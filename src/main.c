#include <stdio.h>
#include <psp2/kernel/modulemgr.h>
#include <psp2/sysmodule.h>
#include <taihen.h>
#include <stdlib.h>
#include <stdbool.h>
#define EGL_EGL_PROTOTYPES 0
#include <EGL/egl.h>
#define GL_GLES_PROTOTYPES 0
#include <GLES2/gl2.h>

PFNEGLGETERRORPROC eglGetError;
PFNEGLGETDISPLAYPROC eglGetDisplay;
PFNEGLINITIALIZEPROC eglInitialize;
PFNEGLBINDAPIPROC eglBindAPI;
PFNEGLCHOOSECONFIGPROC eglChooseConfig;
PFNEGLGETCONFIGSPROC eglGetConfigs;
PFNEGLCREATEWINDOWSURFACEPROC eglCreateWindowSurface;
PFNEGLCREATECONTEXTPROC eglCreateContext;
PFNEGLMAKECURRENTPROC eglMakeCurrent;
PFNEGLSWAPBUFFERSPROC eglSwapBuffers;
PFNEGLGETPROCADDRESSPROC eglGetProcAddress;

PFNGLGETSTRINGPROC glGetString;
PFNGLCLEARCOLORPROC glClearColor;
PFNGLCLEARPROC glClear;
PFNGLGETERRORPROC glGetError;

PFNGLPIXELSTOREIPROC glPixelStorei; //
PFNGLGENTEXTURESPROC glGenTextures;
PFNGLBINDTEXTUREPROC glBindTexture;
PFNGLTEXIMAGE2DPROC glTexImage2D;
PFNGLTEXPARAMETERIPROC glTexParameteri;

PFNGLCREATESHADERPROC glCreateShader;
PFNGLSHADERSOURCEPROC glShaderSource;
PFNGLCOMPILESHADERPROC glCompileShader;
PFNGLGETSHADERIVPROC glGetShaderiv;
PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
PFNGLDELETESHADERPROC glDeleteShader;

PFNGLCREATEPROGRAMPROC glCreateProgram;
PFNGLATTACHSHADERPROC glAttachShader;
PFNGLLINKPROGRAMPROC glLinkProgram;
PFNGLGETPROGRAMIVPROC glGetProgramiv;
PFNGLGETATTRIBLOCATIONPROC glGetAttribLocation;
PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;

PFNGLUSEPROGRAMPROC glUseProgram;
PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;
PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
PFNGLACTIVETEXTUREPROC glActiveTexture;
PFNGLDRAWELEMENTSPROC glDrawElements;

static EGLDisplay s_display = EGL_NO_DISPLAY;
static EGLSurface s_surface = EGL_NO_SURFACE;
static EGLContext s_context = EGL_NO_CONTEXT;

static GLuint s_texture_id = 0;
static GLuint s_program_id = 0;

static GLint s_xyz_loc;
static GLint s_uv_loc;
static GLint s_sampler_loc;

static const GLfloat s_obj_vertices[] = {
	-0.5f, 0.5f, 0.0f,	/* XYZ #0 */
	0.0f, 0.0f,			/* UV  #0 */
	-0.5f, -0.5f, 0.0f, /* XYZ #1 */
	0.0f, 1.0f,			/* UV  #1 */
	0.5f, -0.5f, 0.0f,	/* XYZ #2 */
	1.0f, 1.0f,			/* UV  #2 */
	0.5f, 0.5f, 0.0f,	/* XYZ #3 */
	1.0f, 0.0f			/* UV  #3 */
};
static const GLushort s_obj_indices[] = {
	0,
	1,
	2,
	0,
	2,
	3,
};

static const GLubyte s_texture_data[4 * 3] = {
	255, 0, 0,	 /* red */
	0, 255, 0,	 /* green */
	0, 0, 255,	 /* blue */
	255, 255, 0, /* yellow */
};

static const GLchar s_vertex_shader_code[] =
	"float4 main(float4 a_xyz, float2 a_uv, out float2 v_uv : TEXCOORD0) : POSITION {\n"
	"	v_uv = a_uv;\n"
	"	return a_xyz;\n"
	"}\n";

static const GLchar s_fragment_shader_code[] =
	"float4 main(float2 v_uv : TEXCOORD0, uniform sampler2D s_texture) :COLOR {\n"
	"	return tex2D(s_texture, v_uv);\n"
	"}\n";

#define getFunc(ptr, addr) ptr = seg_1 + (addr | 1)
void init()
{
	sceSysmoduleLoadModule(SCE_SYSMODULE_HTTPS);
	sceSysmoduleLoadModule(SCE_SYSMODULE_SSL);
	sceSysmoduleLoadModule(SCE_SYSMODULE_NP);
	sceSysmoduleLoadModule(SCE_SYSMODULE_CLIPBOARD);
	sceSysmoduleLoadModule(SCE_SYSMODULE_PHOTO_EXPORT);
	sceSysmoduleLoadModule(SCE_SYSMODULE_LOCATION);
	sceSysmoduleLoadModule(SCE_SYSMODULE_SHUTTER_SOUND);

	printf("Loading app0:libpsm.suprx\n");

	int status;
	SceKernelLMOption opt;
	opt.size = sizeof(opt);
	SceUID modid;
	modid = sceKernelLoadStartModule("app0:libmono_bridge.suprx", 0, 0, 0, &opt, &status);
	if (modid < 0)
	{
		printf("Failed to load libmono_bridge.suprx\n");
		exit(0);
	}

	modid = sceKernelLoadStartModule("app0:libpsm.suprx", 0, 0, 0, &opt, &status);
	if (modid < 0)
	{
		printf("Failed to load libpsm.suprx\n");
		exit(0);
	}

	SceKernelModuleInfo info;
	info.size = sizeof(info);
	sceKernelGetModuleInfo(modid, &info);

	uint8_t* seg_1 = (uint8_t*)info.segments[0].vaddr;
	printf("LibPsm Base = 0x%08X\n", seg_1);

	getFunc(eglGetDisplay, 0xB418);
	getFunc(eglInitialize, 0xB456);
	getFunc(eglGetError, 0xB410);
	getFunc(eglBindAPI, 0xBC0E);
	getFunc(eglChooseConfig, 0xB58E);
	getFunc(eglGetConfigs, 0xB52E);
	getFunc(eglCreateWindowSurface, 0xB866);
	getFunc(eglCreateContext, 0xBC3A);
	getFunc(eglMakeCurrent, 0xBD88);
	getFunc(eglSwapBuffers, 0xBE48);
	getFunc(eglGetProcAddress, 0xBEB0);

	getFunc(glGetString, 0xB45CE);
	getFunc(glClearColor, 0xB15DA);
	getFunc(glClear, 0xB152C);
	getFunc(glGetError, 0xB2E6A);

	getFunc(glPixelStorei, 0xB47A6);
	getFunc(glGenTextures, 0xB2988);
	getFunc(glBindTexture, 0xB1156);
	getFunc(glTexImage2D, 0xB4F3E);
	getFunc(glTexParameteri, 0xB52FC);

	getFunc(glCreateShader, 0xB1BF6);
	getFunc(glShaderSource, 0xB4C54);
	getFunc(glCompileShader, 0xB1746);
	getFunc(glGetShaderiv, 0xB44A8);
	getFunc(glGetShaderInfoLog, 0xB4550);
	getFunc(glDeleteShader, 0xB1F8C);

	getFunc(glCreateProgram, 0xB1BB8);
	getFunc(glAttachShader, 0xB0F24);
	getFunc(glLinkProgram, 0xB4772);
	getFunc(glGetProgramiv, 0xB43B6);
	getFunc(glGetAttribLocation, 0xB2C96);
	getFunc(glGetUniformLocation, 0xB4672);

	getFunc(glUseProgram, 0xB611A);
	getFunc(glVertexAttribPointer, 0xB61DE);
	getFunc(glEnableVertexAttribArray, 0xB25EE);
	getFunc(glActiveTexture, 0xB0EF4);
	getFunc(glDrawElements, 0xB23A8);
}

static bool create_texture(void)
{
	int ret;

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	ret = glGetError();
	if (ret)
	{
		printf("glPixelStorei failed: 0x%08X\n", ret);
		goto err;
	}

	glGenTextures(1, &s_texture_id);
	ret = glGetError();
	if (ret)
	{
		printf("glGenTextures failed: 0x%08X\n", ret);
		goto err;
	}

	glBindTexture(GL_TEXTURE_2D, s_texture_id);
	ret = glGetError();
	if (ret)
	{
		printf("glBindTexture failed: 0x%08X\n", ret);
		goto err;
	}

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 2, 2, 0, GL_RGB, GL_UNSIGNED_BYTE, s_texture_data);
	ret = glGetError();
	if (ret)
	{
		printf("glTexImage2D failed: 0x%08X\n", ret);
		goto err;
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	ret = glGetError();
	if (ret)
	{
		printf("glTexParameteri failed: 0x%08X\n", ret);
		goto err;
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	ret = glGetError();
	if (ret)
	{
		printf("glTexParameteri failed: 0x%08X\n", ret);
		goto err;
	}

	return true;

err:
	return false;
}

static bool compile_shader(GLenum type, const char *source, size_t length, GLuint *pShader)
{
	GLuint shader;
	GLint tmp_length = (int)length;
	GLint success;
	char log_buf[256];
	int ret;

	shader = glCreateShader(type);
	if (!shader)
	{
		ret = glGetError();
		printf("glCreateShader failed: 0x%08X\n", ret);
		goto err;
	}

	glShaderSource(shader, 1, &source, &tmp_length);
	ret = glGetError();
	if (ret)
	{
		printf("glShaderSource failed: 0x%08X\n", ret);
		goto err;
	}

	glCompileShader(shader);
	ret = glGetError();
	if (ret)
	{
		printf("glCompileShader failed: 0x%08X\n", ret);
		goto err;
	}

	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	ret = glGetError();
	if (ret)
	{
		printf("glGetShaderiv failed: 0x%08X\n", ret);
		goto err;
	}
	if (!success)
	{
		glGetShaderInfoLog(shader, sizeof(log_buf), NULL, log_buf);
		ret = glGetError();
		if (ret)
		{
			printf("glGetShaderInfoLog failed: 0x%08X\n", ret);
			goto err;
		}
		if (strlen(log_buf) > 1)
		{
			printf("shader compilation failed with log:\n%s\n", log_buf);
		}
		else
		{
			printf("shader compilation failed\n");
		}
		goto err;
	}

	if (pShader)
	{
		*pShader = shader;
	}

	return true;

err:
	return false;
}

static bool create_program(void)
{
	GLuint vertex_shader_id = 0;
	GLuint fragment_shader_id = 0;
	GLuint program_id = 0;
	GLint success;
	char log[256];
	int ret;

	if (!compile_shader(GL_VERTEX_SHADER, s_vertex_shader_code, strlen(s_vertex_shader_code), &vertex_shader_id))
	{
		printf("Unable to compile vertex shader.\n");
		goto err;
	}

	if (!compile_shader(GL_FRAGMENT_SHADER, s_fragment_shader_code, strlen(s_fragment_shader_code), &fragment_shader_id))
	{
		printf("Unable to compile fragment shader.\n");
		goto err;
	}

	program_id = glCreateProgram();
	if (!program_id)
	{
		ret = glGetError();
		printf("glCreateProgram failed: 0x%08X\n", ret);
		goto err;
	}

	glAttachShader(program_id, vertex_shader_id);
	ret = glGetError();
	if (ret)
	{
		printf("glAttachShader(vertex_shader) failed: 0x%08X\n", ret);
		goto err;
	}

	glAttachShader(program_id, fragment_shader_id);
	ret = glGetError();
	if (ret)
	{
		printf("glAttachShader(fragment_shader) failed: 0x%08X\n", ret);
		goto err;
	}

	glLinkProgram(program_id);
	ret = glGetError();
	if (ret)
	{
		printf("glLinkProgram() failed: 0x%08X\n", ret);
		goto err;
	}

	glGetProgramiv(program_id, GL_LINK_STATUS, &success);
	ret = glGetError();
	if (ret)
	{
		printf("glGetProgramiv() failed: 0x%08X\n", ret);
		goto err;
	}
	if (!success)
	{
		printf("Unable to link shader program.\n");
		goto err;
	}

	glDeleteShader(fragment_shader_id);
	ret = glGetError();
	if (ret)
	{
		printf("glDeleteShader(fragment_shader) failed: 0x%08X\n", ret);
		goto err;
	}

	glDeleteShader(vertex_shader_id);
	ret = glGetError();
	if (ret)
	{
		printf("glDeleteShader(vertex_shader) failed: 0x%08X\n", ret);
		goto err;
	}

	s_xyz_loc = glGetAttribLocation(program_id, "a_xyz");
	s_uv_loc = glGetAttribLocation(program_id, "a_uv");
	s_sampler_loc = glGetUniformLocation(program_id, "s_texture");

	s_program_id = program_id;

	return true;

err:
	if (fragment_shader_id > 0)
	{
		glDeleteShader(fragment_shader_id);
		ret = glGetError();
		if (ret)
		{
			printf("glDeleteShader(fragment_shader) failed: 0x%08X\n", ret);
			goto err;
		}
		fragment_shader_id = 0;
	}

	if (vertex_shader_id > 0)
	{
		glDeleteShader(vertex_shader_id);
		ret = glGetError();
		if (ret)
		{
			printf("glDeleteShader(vertex_shader) failed: 0x%08X\n", ret);
			goto err;
		}
		vertex_shader_id = 0;
	}

	return false;
}

int _start()
{
	printf("App start\n");

	init();

	int render_window[3] = {0, 960, 544};
	EGLConfig config = NULL;
	EGLint num_configs;

	EGLint attribs[] = {
		EGL_RED_SIZE,
		8,
		EGL_GREEN_SIZE,
		8,
		EGL_BLUE_SIZE,
		8,
		EGL_ALPHA_SIZE,
		8,
		EGL_DEPTH_SIZE,
		32,
		EGL_RENDERABLE_TYPE,
		EGL_OPENGL_ES2_BIT,
		EGL_NONE,
	};

	EGLint ctx_attribs[] = {
		EGL_CONTEXT_CLIENT_VERSION,
		2,
		EGL_NONE,
	};

	EGLint window_attribs[] = {
		EGL_RENDER_BUFFER,
		EGL_BACK_BUFFER,
		EGL_NONE,
	};

	int major, minor;
	int ret;

	s_display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	if (s_display == EGL_NO_DISPLAY)
	{
		printf("eglGetDisplay failed.\n");
	}

	if (!eglInitialize(s_display, &major, &minor))
	{
		ret = eglGetError();
		printf("eglInitialize failed: 0x%08X\n", ret);
	}
	printf("EGL version major:%d, minor:%d\n", major, minor);

	if (!eglBindAPI(EGL_OPENGL_ES_API))
	{
		ret = eglGetError();
		printf("eglBindAPI failed: 0x%08X\n", ret);
	}

	if (!eglChooseConfig(s_display, attribs, &config, 1, &num_configs))
	{
		ret = eglGetError();
		printf("eglChooseConfig failed: 0x%08X\n", ret);
	}
	if (num_configs < 1)
	{
		printf("No available configuration found.\n");
		goto err;
	}
	printf("Configs %d\n", num_configs);

	s_surface = eglCreateWindowSurface(s_display, config, 1, 0);
	if (s_surface == EGL_NO_SURFACE)
	{
		ret = eglGetError();
		printf("eglCreateWindowSurface failed: 0x%08X\n", ret);
		goto err;
	}

	s_context = eglCreateContext(s_display, config, EGL_NO_CONTEXT, ctx_attribs);
	if (s_context == EGL_NO_CONTEXT)
	{
		ret = eglGetError();
		printf("eglCreateContext failed: 0x%08X\n", ret);
		goto err;
	}

	if (!eglMakeCurrent(s_display, s_surface, s_surface, s_context))
	{
		ret = eglGetError();
		printf("eglMakeCurrent failed: 0x%08X\n", ret);
		goto err;
	}

	printf("GL_RENDERER: %s\n", glGetString(GL_RENDERER));
	printf("GL_VENDOR: %s\n", glGetString(GL_VENDOR));
	printf("GL_VERSION: %s\n", glGetString(GL_VERSION));
	printf("GL_EXTENSIONS: %s\n", glGetString(GL_EXTENSIONS));

	if (!create_texture())
	{
		printf("Unable to create texture.\n");
		goto err;
	}
	if (!create_program())
	{
		printf("Unable to create shader program.\n");
		goto err;
	}

	glClearColor(0.0f, 1.0f, 1.0f, 1.0f);

	{
		while (1)
		{
			glClear(GL_COLOR_BUFFER_BIT);
			ret = glGetError();
			if (ret)
			{
				printf("glClear failed: 0x%08X\n", ret);
				goto err;
			}

			glUseProgram(s_program_id);
			ret = glGetError();
			if (ret)
			{
				printf("glUseProgram failed: 0x%08X\n", ret);
				goto err;
			}

			glVertexAttribPointer(s_xyz_loc, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), s_obj_vertices);
			ret = glGetError();
			if (ret)
			{
				printf("glVertexAttribPointer failed: 0x%08X\n", ret);
				goto err;
			}
			glVertexAttribPointer(s_uv_loc, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), &s_obj_vertices[3]);
			ret = glGetError();
			if (ret)
			{
				printf("glVertexAttribPointer failed: 0x%08X\n", ret);
				goto err;
			}

			glEnableVertexAttribArray(s_xyz_loc);
			ret = glGetError();
			if (ret)
			{
				printf("glEnableVertexAttribArray failed: 0x%08X\n", ret);
				goto err;
			}
			glEnableVertexAttribArray(s_uv_loc);
			ret = glGetError();
			if (ret)
			{
				printf("glEnableVertexAttribArray failed: 0x%08X\n", ret);
				goto err;
			}

			glActiveTexture(GL_TEXTURE0);
			ret = glGetError();
			if (ret)
			{
				printf("glActiveTexture failed: 0x%08X\n", ret);
				goto err;
			}
			glBindTexture(GL_TEXTURE_2D, s_texture_id);
			ret = glGetError();
			if (ret)
			{
				printf("glBindTexture failed: 0x%08X\n", ret);
				goto err;
			}

			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, s_obj_indices);
			ret = glGetError();
			if (ret)
			{
				printf("glDrawElements failed: 0x%08X\n", ret);
				goto err;
			}

			if (!eglSwapBuffers(s_display, s_surface))
			{
				ret = eglGetError();
				printf("eglSwapBuffers failed: 0x%08X\n", ret);
				goto err;
			}
		}
	}

err:
	return 0;
}