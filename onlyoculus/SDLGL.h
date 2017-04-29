#pragma once
#ifndef _SDLGL_H_
#define _SDLGL_H_

#include <SDL.h>
#include <SDL_image.h>
#include <GL/glew.h>

//GLM part
#include <glm/gtx/transform.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

//using namespace glm;
using namespace std;

class SDLGL {
public:
	typedef const char* const (*CubePaths)[6];

	SDLGL(int _w = 800, int _h = 600, Uint32 _f = SDL_INIT_VIDEO | SDL_INIT_TIMER);
	SDLGL(const SDLGL&) = delete;
	const SDLGL& operator=(const SDLGL&) = delete;
	SDLGL(SDLGL&&);
	const SDLGL& operator=(SDLGL&&);
	~SDLGL();
	//call this before Init
	void SetWindowSize_sdl(int, int);
	//call this before Init
	void SetTitle_sdl(char*);
	//use SDL_GL_SetAttribute before call this
	int Init_sdl();
	//two way to create program
	static GLuint CreateProgram_sdl_s(const char*, const char*);
	static GLuint CreateProgram_sdl_s(const char*, const char*, const char*);
	[[deprecated("Untested")]] void CreateProgram_sdl(const char*, const char*);
	static GLuint LoadShader_sdl_s(const char* _vpath, const char* _fpath);
	static GLuint LoadShader_sdl_s(const char* _vpath, const char* _gpath, const char* _fpath);
	[[deprecated("Untested")]] void LoadShader_sdl(const char* _vpath, const char* _fpath);
	static glm::mat4 GetRotationMatrixFromVec3(glm::vec3, glm::vec3);
	static GLuint CreatCubeMap_s(CubePaths cp);
	
	//handle sdl event
	virtual int ProcessEvent_sdl();

	//sample render test,generator must be called before game loop
	virtual int RenderTestGenerator_sdl();
	virtual int RenderTest_sdl(GLuint);
 	virtual SDL_Window* GetWindow_sdl() { return sdl_window; }

protected:

	SDL_GLContext sdl_context;
	Uint32 sdl_init_flags;
	int sdl_height, sdl_width;
	SDL_Event sdl_event;
	bool sdl_ispressed, sdl_isfirstlosthand = true;
	SDL_Window *sdl_window;
	char* sdl_title;
	GLuint *sdl_program;
	
	static void PrintShaderLog_sdl(GLuint);
	static void PrintProgramLog_sdl(GLuint);
};

#endif // !_SDLGL_H_
