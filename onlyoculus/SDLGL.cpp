#include "stdafx.h"
#include "SDLGL.h"

SDLGL::SDLGL(int _w, int _h, Uint32 _f) :
	sdl_width(_w),
	sdl_height(_h),
	sdl_init_flags(_f),
	sdl_title("SDLGL"),
	sdl_ispressed(false) {
	Init_sdl();
}

SDLGL::SDLGL(SDLGL &&) {}

const SDLGL & SDLGL::operator=(SDLGL && s) {
	sdl_window = s.sdl_window;
	s.sdl_window = nullptr;
	this->sdl_context = s.sdl_context;
	s.sdl_context = nullptr;
	return *this;
}

SDLGL::~SDLGL() {
	//delete sdl_window;
	//delete sdl_title;
	//delete sdl_program;
}

void SDLGL::SetWindowSize_sdl(int _h, int _w) {
	sdl_height = _h;
	sdl_width = _w;
}

void SDLGL::SetTitle_sdl(char *_t) {
	sdl_title = _t;
}

int SDLGL::Init_sdl() {
	if (SDL_Init(sdl_init_flags) < 0) {
		fprintf(stderr, "Video initialization failed: %s\n",
			SDL_GetError());
		return -1;
	}
	sdl_window = SDL_CreateWindow(sdl_title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, sdl_width, sdl_height, SDL_WINDOW_OPENGL);
	if (!sdl_window) {
		fprintf(stderr, "Create Window failed: %s\n",
			SDL_GetError());
		return -1;
	}
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	sdl_context = SDL_GL_CreateContext(sdl_window);
	if (!sdl_context) {
		fprintf(stderr, "Create Context failed: %s\n",
			SDL_GetError());
		return -1;
	}
	//disable VSync
	SDL_GL_SetSwapInterval(0);
	glewExperimental = true;
	var err = glewInit();
	if (err != GLEW_OK) {
		fprintf(stderr, "GLEW initialization failed: %s\n",
			glewGetErrorString(err));
		return -1;
	}
	return 0;
}

GLuint SDLGL::CreateProgram_sdl_s(const char *_v, const char *_f) {
	GLuint _vshader, _fshader, _program;
	_vshader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(_vshader, 1, &_v, NULL);
	glCompileShader(_vshader);
	PrintShaderLog_sdl(_vshader);

	_fshader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(_fshader, 1, &_f, NULL);
	glCompileShader(_fshader);
	PrintShaderLog_sdl(_fshader);

	_program = glCreateProgram();
	glAttachShader(_program, _vshader);
	glAttachShader(_program, _fshader);
	glLinkProgram(_program);
	PrintProgramLog_sdl(_program);
	glDetachShader(_program, _vshader);
	glDetachShader(_program, _fshader);
	return GLuint(_program);
}

GLuint SDLGL::CreateProgram_sdl_s(const char *_v, const char *_g, const char *_f) {
	GLuint _vshader, _gshader, _fshader, _program;
	_vshader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(_vshader, 1, &_v, NULL);
	glCompileShader(_vshader);
	PrintShaderLog_sdl(_vshader);

	_gshader = glCreateShader(GL_GEOMETRY_SHADER);
	glShaderSource(_gshader, 1, &_g, NULL);
	glCompileShader(_gshader);
	PrintShaderLog_sdl(_gshader);

	_fshader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(_fshader, 1, &_f, NULL);
	glCompileShader(_fshader);
	PrintShaderLog_sdl(_fshader);

	_program = glCreateProgram();
	glAttachShader(_program, _vshader);
	glAttachShader(_program, _gshader);
	glAttachShader(_program, _fshader);
	glLinkProgram(_program);
	PrintProgramLog_sdl(_program);
	glDetachShader(_program, _vshader);
	glDetachShader(_program, _gshader);
	glDetachShader(_program, _fshader);
	return GLuint(_program);
}

void SDLGL::CreateProgram_sdl(const char *_v, const char *_f) {
	CreateProgram_sdl_s(_v, _f);
}

GLuint SDLGL::LoadShader_sdl_s(const char * _vpath, const char * _gpath, const char * _fpath) {
	string _vshader_f_source, _gshader_f_source, _fshader_f_source, temp;
	auto shaderfstream = ifstream(_vpath, ios::in);
	if (shaderfstream.is_open()) {
		while (getline(shaderfstream, temp)) {
			_vshader_f_source += temp + "\n";
		}
	}
	shaderfstream.close();

	shaderfstream = ifstream(_gpath);
	if (shaderfstream.is_open()) {
		while (getline(shaderfstream, temp)) {
			_gshader_f_source += temp + "\n";
		}
	}
	shaderfstream.close();

	shaderfstream = ifstream(_fpath);
	if (shaderfstream.is_open()) {
		while (getline(shaderfstream, temp)) {
			_fshader_f_source += temp + "\n";
		}
	}
	shaderfstream.close();

	return CreateProgram_sdl_s(_vshader_f_source.c_str(), _gshader_f_source.c_str(), _fshader_f_source.c_str());
}

GLuint SDLGL::LoadShader_sdl_s(const char * _vpath, const char * _fpath) {
	string _vshader_f_source, _fshader_f_source, temp;
	auto shaderfstream = ifstream(_vpath, ios::in);
	if (shaderfstream.is_open()) {
		while (getline(shaderfstream, temp)) {
			_vshader_f_source += temp + "\n";
		}
	}
	shaderfstream.close();
	shaderfstream = ifstream(_fpath);
	if (shaderfstream.is_open()) {
		while (getline(shaderfstream, temp)) {
			_fshader_f_source += temp + "\n";
		}
	}
	shaderfstream.close();
	return CreateProgram_sdl_s(_vshader_f_source.c_str(), _fshader_f_source.c_str());
}

void SDLGL::LoadShader_sdl(const char * _vpath, const char * _fpath) {

}

glm::mat4 SDLGL::GetRotationMatrixFromVec3(glm::vec3 src, glm::vec3 dest) {
	glm::mat4 _r(1);
	if ((src.x == 0 &&
		src.y == 0 &&
		src.z == 0) ||
		(dest.x == 0 &&
			dest.y == 0 &&
			dest.z == 0))
		return _r;
	src = glm::normalize(src);
	dest = glm::normalize(dest);
	var _iv = glm::dot(src, dest);
	if (glm::length(src + dest) > 0.02) {
		if (_iv > 0.98) {
			cerr << "Same Direction" << endl;
		}
		else {
			var _axis = glm::normalize(glm::cross(src, dest));
			var _rad = glm::acos(_iv);
			cout << "radiant:" << _rad << endl;
			var _s = glm::cos(0.5*_rad);
			var _v = ((float)glm::sin(0.5*_rad))*_axis;
			var _quat = glm::quat(_s, _v);
			_r = std::move(glm::toMat4(_quat));
		}
	}
	else {
		var _axis = glm::normalize(glm::cross(src, glm::vec3(0,0,1)));
		if (glm::length(_axis) < 0.02) {
			_axis = glm::cross(src, glm::vec3(0, 1, 0));
		}
		var _s = glm::cos(glm::radians(90.f));
		cout << "almost parallel" << endl;
		var _v = ((float)glm::sin(glm::radians(90.f)))*_axis;
		var _quat = glm::quat(_s, _v);
		_r = std::move(glm::toMat4(_quat));
	}
	return _r;
}

GLuint SDLGL::CreatCubeMap_s(CubePaths cp) {
	GLuint _tid;
	glGenTextures(1, &_tid);
	glBindTexture(GL_TEXTURE_CUBE_MAP, _tid);
	int _width, _height;
	SDL_Surface* _image;

	for (int i = 0; i < 6; ++i) {
		_image = IMG_Load((*cp)[i]);
		if (!_image) {
			cerr << "IMG_Load failed" << endl;
			return 0;
		}
		SDL_PIXELFORMAT_UNKNOWN;
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, _image->w, _image->h, 0, GL_BGR, GL_UNSIGNED_BYTE, _image->pixels);
		SDL_FreeSurface(_image);
	}
	//glTexParameteri(GL_TEXTURE_CUBE_MAP,GL_TEXTURE_MAG_FILTER,)
	glTextureParameteri(_tid, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTextureParameteri(_tid, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTextureParameteri(_tid, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(_tid, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteri(_tid, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	cout << "CubeMap Created!" << endl;
	return _tid;
}

int SDLGL::ProcessEvent_sdl() {
	while (SDL_PollEvent(&sdl_event)) {
		switch (sdl_event.type) {
		case SDL_MOUSEBUTTONDOWN:
			//cout << "click" << endl;
			sdl_ispressed = true;
			break;
		case SDL_MOUSEBUTTONUP:
			//cout << "r" << endl;
			sdl_ispressed = false;
			break;
		case SDL_MOUSEMOTION:
			//cout << sdl_event.motion.x << "\t" << sdl_event.motion.y+ << endl;
			if (sdl_ispressed)
			break;
		case SDL_QUIT:
			return 0;
			break;
		default:
			break;
		}
	}
	return 1;
}

int SDLGL::RenderTestGenerator_sdl() {
	GLfloat _t[] = { 0.0, 0.5,0.0, 1.0,0.0,0.3,
		-0.5,-0.5,0.5,0.3,0.7,0.1,
		0.5,-0.5,0.5, 0.4,0.1,0.9,
		0.0,-0.5,-0.5,0.0,0.1,0.3 };
	GLushort _ti[] = { 0,1,2,
		0,3,1,
		0,2,3,
		1,3,2 };
	GLuint _vao, _vbo, _ebo;
	glGenVertexArrays(1, &_vao);
	glGenBuffers(1, &_vbo);
	glGenBuffers(1, &_ebo);
	glBindVertexArray(_vao);
	glBindBuffer(GL_ARRAY_BUFFER, _vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(_t), _t, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, false, 6 * sizeof(GLfloat), nullptr);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, false, 6 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(_ti), _ti, GL_STATIC_DRAW);
	glBindVertexArray(0);
	return _vao;
}

int SDLGL::RenderTest_sdl(GLuint _v) {
	glBindVertexArray(_v);
	glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_SHORT, 0);
	glBindVertexArray(0);
	return 0;
}

void SDLGL::PrintShaderLog_sdl(GLuint _s) {
	//Make sure name is shader
	if (glIsShader(_s)) {
		//Shader log length
		int infoLogLength = 0;
		int maxLength = infoLogLength;
		//Get info string length
		glGetShaderiv(_s, GL_INFO_LOG_LENGTH, &maxLength);
		//Allocate string
		char* infoLog = new char[maxLength];
		//Get info log
		glGetShaderInfoLog(_s, maxLength, &infoLogLength, infoLog);
		if (infoLogLength > 0) {
			//Print Log
			printf("%s\n", infoLog);
		}
		//Deallocate string
		delete[] infoLog;
	}
	else {
		printf("Name %d is not a shader\n", _s);
	}
}

void SDLGL::PrintProgramLog_sdl(GLuint _p) {
	if (glIsProgram(_p)) {
		//Program log length
		int infoLogLength = 0;
		int maxLength = infoLogLength;
		//Get info string length
		glGetProgramiv(_p, GL_INFO_LOG_LENGTH, &maxLength);
		//Allocate string
		char* infoLog = new char[maxLength];
		//Get info log
		glGetProgramInfoLog(_p, maxLength, &infoLogLength, infoLog);
		if (infoLogLength > 0) {
			//Print Log
			printf("%s\n", infoLog);
		}
		//Deallocate string
		delete[] infoLog;
	}
	else {
		printf("Name %d is not a program\n", _p);
	}
}
