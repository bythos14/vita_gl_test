#include <stdio.h>
#include <psp2/kernel/modulemgr.h>
#include <psp2/sysmodule.h>
#include <taihen.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#define EGL_EGL_PROTOTYPES 0
#include <EGL/egl.h>
#define GL_GLES_PROTOTYPES 0
#include <GLES2/gl2.h>
#include <stdarg.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

extern "C" void _SCE_Assert(const char *, int, const char *, const char *);

void __assert_func(const char * p1, int p2, const char * p3, const char * p4) 
{
	_SCE_Assert(p1, p2, p3, p4);
	abort();
}

void log2json(const char *format, ...)
{
	va_list arg;
	va_start(arg, format);
	char msg[512];
	vsprintf(msg, format, arg);
	va_end(arg);
	sprintf(msg, "%s\n", msg);
	FILE *log = fopen("ux0:/data/json.log", "a+");
	if (log != NULL)
	{
		fwrite(msg, 1, strlen(msg), log);
		fclose(log);
	}
}

#define printf log2json

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
PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv;

PFNGLUSEPROGRAMPROC glUseProgram;
PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;
PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
PFNGLACTIVETEXTUREPROC glActiveTexture;
PFNGLDRAWELEMENTSPROC glDrawElements;

PFNGLENABLEPROC glEnable;
PFNGLDEPTHFUNCPROC glDepthFunc;

#include <assert.h>

static EGLDisplay s_display = EGL_NO_DISPLAY;
static EGLSurface s_surface = EGL_NO_SURFACE;
static EGLContext s_context = EGL_NO_CONTEXT;

static GLuint s_texture_id = 0;
static GLuint s_program_id = 0;

static GLint pos_loc;
static GLint color_loc;
static GLint wvp_loc;

float colors[] = {1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 0.0, 1.0, 1.0, 1.0, 1.0}; // Colors for a face

float vertices_front[] = {-0.5f, -0.5f, -0.5f, 0.5f, -0.5f, -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f, -0.5f}; // Front Face
float vertices_back[] = {-0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f}; // Back Face
float vertices_left[] = {-0.5f, -0.5f, -0.5f, -0.5f, 0.5f, -0.5f, -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f}; // Left Face
float vertices_right[] = {0.5f, -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f, 0.5f, 0.5f}; // Right Face
float vertices_top[] = {-0.5f, -0.5f, -0.5f, 0.5f, -0.5f, -0.5f, -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f}; // Top Face
float vertices_bottom[] = {-0.5f, 0.5f, -0.5f, 0.5f, 0.5f, -0.5f, -0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f}; // Bottom Face

uint16_t indices[] = {
	 0, 1, 2, 1, 2, 3, // Front
	 4, 5, 6, 5, 6, 7, // Back
	 8, 9,10, 9,10,11, // Left
	12,13,14,13,14,15, // Right
	16,17,18,17,18,19, // Top
	20,21,22,21,22,23  // Bottom
};

static const GLchar s_vertex_shader_code[] =
	"float4 main(float4 aPos, float3 aColor, uniform float4x4 wvp, out float3 vColor : TEXCOORD0) : POSITION {\n"
	"	vColor = aColor;\n"
	"	return mul(aPos, wvp);\n"
	"}\n";

static const GLchar s_fragment_shader_code[] =
	"float4 main(float3 vColor : TEXCOORD0) :COLOR {\n"
	"	return float4(vColor, 1.0);\n"
	"}\n";

#define getFunc(ptr, addr) ptr = (typeof(ptr))(seg_1 + (addr | 1))
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
	getFunc(glEnable, 0xB2518);
	getFunc(glDepthFunc, 0xB20A4);

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
	getFunc(glUniformMatrix4fv, 0xB6044);

	getFunc(glUseProgram, 0xB611A);
	getFunc(glVertexAttribPointer, 0xB61DE);
	getFunc(glEnableVertexAttribArray, 0xB25EE);
	getFunc(glActiveTexture, 0xB0EF4);
	getFunc(glDrawElements, 0xB23A8);
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

	pos_loc = glGetAttribLocation(program_id, "aPos");
	color_loc = glGetAttribLocation(program_id, "aColor");
	wvp_loc = glGetUniformLocation(program_id, "wvp");

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
extern "C" {
int _start()
{
	glm::mat4 proj = glm::perspective(90.0f, 960.f/544.0f, 0.01f, 100.0f);

	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, 0.0f, -2.0f));

	glm::mat4 wvp;
	
	printf("App start\n");

	init();

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

	if (!create_program())
	{
		printf("Unable to create shader program.\n");
		goto err;
	}

	// Creating colors array
	float color_array[12 * 6];
	int i;
	for (i = 0; i < 12 * 6; i++)
	{
		color_array[i] = colors[i % 12];
	}

	// Creating vertices array
	float vertex_array[12 * 6];
	memcpy(&vertex_array[12 * 0], &vertices_front[0], sizeof(float) * 12);
	memcpy(&vertex_array[12 * 1], &vertices_back[0], sizeof(float) * 12);
	memcpy(&vertex_array[12 * 2], &vertices_left[0], sizeof(float) * 12);
	memcpy(&vertex_array[12 * 3], &vertices_right[0], sizeof(float) * 12);
	memcpy(&vertex_array[12 * 4], &vertices_top[0], sizeof(float) * 12);
	memcpy(&vertex_array[12 * 5], &vertices_bottom[0], sizeof(float) * 12);

	glClearColor(0.0f, 1.0f, 1.0f, 1.0f);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	while (1)
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
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

		glVertexAttribPointer(pos_loc, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), vertex_array);
		ret = glGetError();
		if (ret)
		{
			printf("glVertexAttribPointer failed: 0x%08X\n", ret);
			goto err;
		}
		glVertexAttribPointer(color_loc, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), color_array);
		ret = glGetError();
		if (ret)
		{
			printf("glVertexAttribPointer failed: 0x%08X\n", ret);
			goto err;
		}

		glEnableVertexAttribArray(pos_loc);
		ret = glGetError();
		if (ret)
		{
			printf("glEnableVertexAttribArray failed: 0x%08X\n", ret);
			goto err;
		}
		glEnableVertexAttribArray(color_loc);
		ret = glGetError();
		if (ret)
		{
			printf("glEnableVertexAttribArray failed: 0x%08X\n", ret);
			goto err;
		}

		model = glm::rotate(model, glm::radians(1.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::rotate(model, glm::radians(0.75f), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, glm::radians(0.25f), glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 wvp = proj * model;
		glUniformMatrix4fv(wvp_loc, 1, GL_FALSE, &wvp[0][0]);

		glDrawElements(GL_TRIANGLES, 6 * 6, GL_UNSIGNED_SHORT, indices);
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

err:
	return 0;
}
}