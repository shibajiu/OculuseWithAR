// onlyoculus.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include "SDLGL.h"
#include "OvrGLItems.h"
#include "loadwinthAssimp.h"
#include "KalmanFilter.h"
#include "ovrrs.h"

//cv part
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include <pxcsensemanager.h>

#if defined(_WIN32)
#include <dxgi.h> // for GetDefaultAdapterLuid
#pragma comment(lib, "dxgi.lib")
#endif

using namespace std;

SDLGL *gl = nullptr;
cv::VideoCapture capture;
PXCSenseManager *sm;
PXCImage::ImageData rsdata;
int offset = 200;
glm::vec3 handpos;
ovrrs_fh *rsf;
KalmanFilter<glm::vec3> *kal;

GLfloat skyboxVertices[] = {
	// Positions          
	-1.0f,  1.0f, -1.0f,
	-1.0f, -1.0f, -1.0f,
	1.0f, -1.0f, -1.0f,
	1.0f, -1.0f, -1.0f,
	1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,

	1.0f, -1.0f, -1.0f,
	1.0f, -1.0f,  1.0f,
	1.0f,  1.0f,  1.0f,
	1.0f,  1.0f,  1.0f,
	1.0f,  1.0f, -1.0f,
	1.0f, -1.0f, -1.0f,

	-1.0f, -1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	1.0f,  1.0f,  1.0f,
	1.0f,  1.0f,  1.0f,
	1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,

	-1.0f,  1.0f, -1.0f,
	1.0f,  1.0f, -1.0f,
	1.0f,  1.0f,  1.0f,
	1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
	1.0f, -1.0f, -1.0f,
	1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
	1.0f, -1.0f,  1.0f
};

GLfloat ARVertices[] = {
	1.0,1.0,1.0,-1.0,
	-1.0,1.0,-1.0,-1.0,
	-1.0,-1.0,-1.0,1.0,
	-1.0,-1.0,-1.0,1.0,
	1.0,-1.0,1.0,1.0,
	1.0,1.0,1.0,-1.0
};

GLfloat mmVertices[] = {
	1.0,1.0,1.0,-1.0,
	-1.0,1.0,-1.0,-1.0,
	-1.0,-1.0,-1.0,1.0,
	-1.0,-1.0,-1.0,1.0,
	1.0,-1.0,1.0,1.0,
	1.0,1.0,1.0,-1.0
};

GLfloat mmVertices1[] = {
	1.0,0.5625,1.0,-1.0,
	-1.0,0.5625,-1.0,-1.0,
	-1.0,-0.5625,-1.0,1.0,
	-1.0,-0.5625,-1.0,1.0,
	1.0,-0.5625,1.0,1.0,
	1.0,0.5625,1.0,-1.0
};


void VALIDATE(bool x, const char *msg) {
	if (!x) {
		cout << msg;
		exit(-1);
	}
}

static ovrGraphicsLuid GetDefaultAdapterLuid() {
	ovrGraphicsLuid luid = ovrGraphicsLuid();

#if defined(_WIN32)
	IDXGIFactory* factory = nullptr;

	if (SUCCEEDED(CreateDXGIFactory(IID_PPV_ARGS(&factory)))) {
		IDXGIAdapter* adapter = nullptr;

		if (SUCCEEDED(factory->EnumAdapters(0, &adapter))) {
			DXGI_ADAPTER_DESC desc;

			adapter->GetDesc(&desc);
			memcpy(&luid, &desc.AdapterLuid, sizeof(luid));
			adapter->Release();
		}

		factory->Release();
	}
#endif
	return luid;
}


static int Compare(const ovrGraphicsLuid& lhs, const ovrGraphicsLuid& rhs) {
	return memcmp(&lhs, &rhs, sizeof(ovrGraphicsLuid));
}

void Display() {
	
}

bool MainLoop() {
	TextureBuffer *eyeRenderTexture[2] = { nullptr,nullptr };
	DepthBuffer * eyeDepthTexture[2] = { nullptr,nullptr };
	ovrMirrorTexture mirrorTexture = nullptr;
	GLuint mirrorFBO = 0, mirrortextid = 0;
	long long frameIndex = 0;
	SDL_Event event;

	ovrSession session;
	ovrGraphicsLuid luid;
	var _r = ovr_Create(&session, &luid);
	VALIDATE(OVR_SUCCESS(_r), "Failed to create ovrSession");
	// If luid that the Rift is on is not the default adapter LUID...
	if (Compare(luid, GetDefaultAdapterLuid())) {
		VALIDATE(false, "OpenGL supports only the default graphics adapter.");
	}
	var hmdDesc = ovr_GetHmdDesc(session);

	ovrSizei windowSize = { hmdDesc.Resolution.w*0.5, hmdDesc.Resolution.h*0.5 };
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 5);
	gl = new SDLGL(windowSize.w, windowSize.h);

#ifdef CVCAP
	cv::Mat frame;
	capture >> frame;
	GLenum inputColourFormat = GL_BGR;
	if (frame.channels() == 1) {
		inputColourFormat = GL_LUMINANCE;
	}
	GLuint artextid;
	GLint textwidth(frame.cols), textheight(frame.rows);
	glGenTextures(1, &artextid);
	glBindTexture(GL_TEXTURE_2D, artextid);
	glTexImage2D(
		GL_TEXTURE_2D,
		0, GL_RGB,
		textwidth,
		textheight,
		0,
		inputColourFormat,
		GL_UNSIGNED_BYTE,
		nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);
	glEnable(GL_TEXTURE_2D);
	var PRG_AR = SDLGL::LoadShader_sdl_s("ar.vert", "ar.frag");

#endif // CVCAP

#ifndef CVCAP
#ifdef AR_TEST
	int width = 1182;
	int height = 1464;
	GLuint artextid;
	GLint textwidth(1920), textheight(1080);
	glGenTextures(1, &artextid);
	glBindTexture(GL_TEXTURE_2D, artextid);
	glTexImage2D(
		GL_TEXTURE_2D,
		0, GL_RGB,
		textwidth,
		textheight,
		0,
		GL_RGB,
		GL_UNSIGNED_BYTE,
		nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);
	glEnable(GL_TEXTURE_2D);
	var PRG_ar = SDLGL::LoadShader_sdl_s("ar.vert", "ar.frag");
	var windowscale = (float)width / height;
	var texturescale = (float)textwidth / textheight;
	if (windowscale>texturescale) {
		for (int i = 0; i < 6; ++i) {
			ARVertices[4 * i] *= (texturescale / windowscale);
		}
	}
	else {
		for (int i = 0; i < 6; ++i) {
			ARVertices[4 * i + 1] *= (windowscale / texturescale);
		}
	}
#endif // AR_TEST
#endif // !CVCAP

	A_model CastleModel("d:/castle.obj");
	var PRG_castle = SDLGL::LoadShader_sdl_s("cast.vert", "cast.frag");
	var PRG_skybox = SDLGL::LoadShader_sdl_s("skybox.vert", "skybox.frag");
	var PRG_hand = SDLGL::LoadShader_sdl_s("hv.glsl", "hg.glsl", "hf.glsl");
	const char* _p[] = {
		"mp_orbital/orbital-element_ft.tga",
		"mp_orbital/orbital-element_bk.tga",
		"mp_orbital/orbital-element_up.tga",
		"mp_orbital/orbital-element_dn.tga",
		"mp_orbital/orbital-element_rt.tga",
		"mp_orbital/orbital-element_lf.tga"
	};
	GLuint VAO_skybox,VBO_skybox;
	glGenVertexArrays(1, &VAO_skybox);
	glBindVertexArray(VAO_skybox);
	glGenBuffers(1, &VBO_skybox);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_skybox);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);
	//glEnableVertexAttribArray(0);
	glEnableVertexArrayAttrib(VAO_skybox, 0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glBindVertexArray(0);

	var _ve = rsf->GetJointPoints();
	GLuint VBO_hand, VAO_hand;
	//var p = s.GetHandPos_sdl();
	glGenVertexArrays(1, &VAO_hand);
	glBindVertexArray(VAO_hand);
	glGenBuffers(1, &VBO_hand);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_hand);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)*MAX_NUMBER_OF_JOINTS, &_ve[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(glm::vec3), 0);
	glBindVertexArray(0);


#ifdef AR_TEST
	GLuint VAO_ar, VBO_ar;
	glGenVertexArrays(1, &VAO_ar);
	glBindVertexArray(VAO_ar);
	glGenBuffers(1, &VBO_ar);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_ar);
	glBufferData(GL_ARRAY_BUFFER, sizeof(ARVertices), ARVertices, GL_STATIC_DRAW);
	//glEnableVertexAttribArray(0);
	glEnableVertexArrayAttrib(VAO_ar, 0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)0);
	glEnableVertexArrayAttrib(VAO_ar, 1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)(2 * sizeof(GLfloat)));
	glBindVertexArray(0);
#endif // AR_TEST

#ifdef MM_TEST
	SDL_Surface *mm_img;
	mm_img = IMG_Load("mearsurement.png");
	if (!mm_img) {
		printf("SDL_Image load error: %s\n", IMG_GetError());
		system("pause");
		return 0;
	}

	GLuint mmtextid;
	glGenTextures(1, &mmtextid);
	glBindTexture(GL_TEXTURE_2D, mmtextid);
	glTexImage2D(
		GL_TEXTURE_2D,
		0, GL_RGB,
		textwidth,
		textheight,
		0,
		GL_BGR,
		GL_UNSIGNED_BYTE,
		mm_img->pixels);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);


	var windowscale = 1182.f / 1464;
	var texturescale = (float)textwidth / textheight;
	if (windowscale > texturescale) {
		for (int i = 0; i < 6; ++i) {
			mmVertices[4 * i] *= (texturescale / windowscale);
		}
	}
	else {
		for (int i = 0; i < 6; ++i) {
			mmVertices[4 * i + 1] *= (windowscale / texturescale);
		}
	}
	GLuint VAO_mm, VBO_mm;
	glGenVertexArrays(1, &VAO_mm);
	glBindVertexArray(VAO_mm);
	glGenBuffers(1, &VBO_mm);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_mm);
	glBufferData(GL_ARRAY_BUFFER, sizeof(mmVertices), mmVertices, GL_STATIC_DRAW);
	//glEnableVertexAttribArray(0);
	glEnableVertexArrayAttrib(VAO_mm, 0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)0);
	glEnableVertexArrayAttrib(VAO_mm, 1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)(2 * sizeof(GLfloat)));
	glBindVertexArray(0);

	var PRG_mm = SDLGL::LoadShader_sdl_s("mm.vert", "mm.frag");
#endif // MM_TEST

	var TEX_skybox = SDLGL::CreatCubeMap_s(&_p);

	for (int eye = 0; eye < 2; ++eye) {
		ovrSizei idealTexturesize = ovr_GetFovTextureSize(session, ovrEyeType(eye), hmdDesc.DefaultEyeFov[1], 1);
		eyeRenderTexture[eye] = new TextureBuffer(session, true, true, idealTexturesize, 1, nullptr, 1);
		eyeDepthTexture[eye] = new DepthBuffer(idealTexturesize, 1);
		VALIDATE(static_cast<bool>(eyeRenderTexture[eye]->TextureChain), "Failed to create eye textures");
	}

	ovrMirrorTextureDesc mirrordesc;
#ifdef _WIN32
	ZeroMemory(&mirrordesc, sizeof(mirrordesc));
#else
	memset(&mirrordesc, 0, sizeof(mirrordesc));
#endif // _WIN32
	mirrordesc.Width = windowSize.w;
	mirrordesc.Height = windowSize.h;
	mirrordesc.Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB;
	_r = ovr_CreateMirrorTextureGL(session, &mirrordesc, &mirrorTexture);
	VALIDATE(OVR_SUCCESS(_r), "Failed to create mirror texture");

	ovr_GetMirrorTextureBufferGL(session, mirrorTexture, &mirrortextid);

	//Use GL_READ_FRAMEBUFFER intend to avoid polution
	glGenFramebuffers(1, &mirrorFBO);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, mirrorFBO);
	glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mirrortextid, 0);
	glFramebufferRenderbuffer(GL_READ_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 0);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

	glEnable(GL_DEPTH_TEST);
	glClearColor(0, 0, 0, 0);
	bool quit = false;
	static glm::mat4 model(1);

	var light_dir = glm::normalize(glm::vec3(-1, 1, -1));
	glProgramUniform3fv(PRG_castle, glGetUniformLocation(PRG_castle, "light_dir"), 1, &light_dir.x);

	while (!quit) {
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_QUIT:
				quit = true;
				break;
			default:
				break;
			}
		}
		ovrSessionStatus sstatus;
		ovr_GetSessionStatus(session, &sstatus);
		if (sstatus.ShouldQuit) {
			break;
		}
		if (sstatus.ShouldRecenter) {
			ovr_RecenterTrackingOrigin(session);
		}
		if (sstatus.IsVisible) {

#ifdef CVCAP
			capture >> frame;
			cv::Mat flipmat;
			cv::flip(frame, flipmat, 0);
#endif // CVCAP

			static float Yaw(3.141592f);
			static Vector3f Pos2(0.0f, 0.0f, -8.0f);
			// Call ovr_GetRenderDesc each frame to get the ovrEyeRenderDesc, as the returned values (e.g. HmdToEyeOffset) may change at runtime.
			ovrEyeRenderDesc eyeRenderDesc[2];
			eyeRenderDesc[0] = ovr_GetRenderDesc(session, ovrEye_Left, hmdDesc.DefaultEyeFov[0]);
			eyeRenderDesc[1] = ovr_GetRenderDesc(session, ovrEye_Right, hmdDesc.DefaultEyeFov[1]);

			// Get eye poses, feeding in correct IPD offset
			ovrPosef                  EyeRenderPose[2];
			ovrVector3f               HmdToEyeOffset[2] = { eyeRenderDesc[0].HmdToEyeOffset,
				eyeRenderDesc[1].HmdToEyeOffset };

			double sensorSampleTime;    // sensorSampleTime is fed into the layer later
			ovr_GetEyePoses(session, frameIndex, ovrTrue, HmdToEyeOffset, EyeRenderPose, &sensorSampleTime);
			for (int eye = 0; eye < 2; ++eye) {
				eyeRenderTexture[eye]->SetAndClearRenderSurface(eyeDepthTexture[eye]);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

				Matrix4f rollPitchYaw = Matrix4f::RotationY(Yaw);
				Matrix4f finalRollPitchYaw = rollPitchYaw * Matrix4f(EyeRenderPose[eye].Orientation);
				//cout << eye << ":\t" << EyeRenderPose[eye].Orientation.x << ends << EyeRenderPose[eye].Orientation.y << ends << EyeRenderPose[eye].Orientation.z << endl;
				Vector3f finalUp = finalRollPitchYaw.Transform(Vector3f(0, 1, 0));
				Vector3f finalForward = finalRollPitchYaw.Transform(Vector3f(0, 0, -1));
				Vector3f shiftedEyePos = Pos2 + rollPitchYaw.Transform(EyeRenderPose[eye].Position);

				Matrix4f view = Matrix4f::LookAtRH(shiftedEyePos, shiftedEyePos + finalForward, finalUp);
				Matrix4f proj = ovrMatrix4f_Projection(hmdDesc.DefaultEyeFov[eye], 0.2f, 1000.0f, ovrProjection_None);

				var mat_model1 = glm::mat4(1);
				var mat_view1 = glm::lookAt(glm::vec3(0, 0, 1), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
				glUseProgram(PRG_hand);
				glUniformMatrix4fv(glGetUniformLocation(PRG_hand, "mat_model"), 1, false, glm::value_ptr(mat_model1));
				glUniformMatrix4fv(glGetUniformLocation(PRG_hand, "mat_view"), 1, false, glm::value_ptr(mat_view1));
				glUniformMatrix4fv(glGetUniformLocation(PRG_hand, "mat_projection"), 1, GL_TRUE, (float*)&proj.M[0]);
				glBindVertexArray(VAO_hand);
				_ve = rsf->GetJointPoints();
				glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GLfloat)*MAX_NUMBER_OF_JOINTS * 3, &_ve[0]);
				glDrawArrays(GL_POINTS, 0, MAX_NUMBER_OF_JOINTS);
				glBindVertexArray(0);
				glUseProgram(0);

				glUseProgram(PRG_castle);
				for (int i = 0; i < CastleModel.nMeshNum; ++i) {
					model = glm::translate(-CastleModel.vCenter[i]);
					glBindVertexArray(CastleModel.mVAOs[i]);
					glUniform3fv(glGetUniformLocation(PRG_castle, "viewPos"), 1, &shiftedEyePos.x);
					glUniformMatrix4fv(glGetUniformLocation(PRG_castle, "mat_model"), 1, GL_FALSE, glm::value_ptr(model));
					//ovrmath use row-major order
					glUniformMatrix4fv(glGetUniformLocation(PRG_castle, "mat_view"), 1, GL_TRUE, (float*)&view.M[0]);
					glUniformMatrix4fv(glGetUniformLocation(PRG_castle, "mat_projection"), 1, GL_TRUE, (float*)&proj.M[0]);
					glDrawElements(GL_TRIANGLES, CastleModel.mElementCount[i], GL_UNSIGNED_INT, 0);
					glBindVertexArray(0);
				}
				glUseProgram(0);

				//Render part
				glDepthFunc(GL_LEQUAL);
				glUseProgram(PRG_skybox);
				static glm::mat4 model_sky(1);
				//glm use column-major order
				glUniformMatrix4fv(glGetUniformLocation(PRG_skybox, "model"), 1, GL_FALSE, glm::value_ptr(model_sky));
				//ovrmath use row-major order
				glUniformMatrix4fv(glGetUniformLocation(PRG_skybox, "view"), 1, GL_TRUE, (float*)&view.M[0]);
				glUniformMatrix4fv(glGetUniformLocation(PRG_skybox, "projection"), 1, GL_TRUE, (float*)&proj.M[0]);
				glActiveTexture(GL_TEXTURE0);
				glUniform1i(glGetUniformLocation(PRG_skybox, "texSkyBox"), 0);
				glBindVertexArray(VAO_skybox);
				glBindTexture(GL_TEXTURE_CUBE_MAP, TEX_skybox);
				glDrawArrays(GL_TRIANGLES, 0, 36);
				glBindVertexArray(0);
				glUseProgram(0);
				glDepthFunc(GL_LESS);

#ifdef CVCAP
				glUseProgram(PRG_ar);
				glBindVertexArray(VAO_ar);

				glActiveTexture(GL_TEXTURE0);
				glUniform1i(glGetUniformLocation(PRG_ar, "artex"), 0);
				glBindTexture(GL_TEXTURE_2D, artextid);
				glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, textwidth, textheight, GL_BGR, GL_UNSIGNED_BYTE, flipmat.ptr());
				glDrawArrays(GL_TRIANGLES, 0, 6);
				glBindTexture(GL_TEXTURE_2D, 0);
				glBindVertexArray(0);
				glUseProgram(0);
#endif // CVCAP

#ifndef CVCAP

#ifdef AR_TEST
				//This function blocks until a color sample is ready
				if (sm->AcquireFrame(true) < PXC_STATUS_NO_ERROR) {
					cout << "AcquireFrame::ERR" << endl;
					break;
				}
				// retrieve the sample
				PXCCapture::Sample *sample = sm->QuerySample();
				// work on the image sample->color
				PXCImage* color_img = sample->color;
				static bool info = false;
				if (!info) {
					PXCImage::ImageInfo rgb_info;
					rgb_info = color_img->QueryInfo();
					cout << rgb_info.width << "*" << rgb_info.height << endl;
					info = true;
				}
				glDepthFunc(GL_LEQUAL);
				color_img->AcquireAccess(PXCImage::ACCESS_READ, PXCImage::PIXEL_FORMAT_RGB24, &rsdata);
				glUseProgram(PRG_ar);
				glBindVertexArray(VAO_ar);

				glActiveTexture(GL_TEXTURE0);
				glUniform1i(glGetUniformLocation(PRG_ar, "artex"), 0);
				glBindTexture(GL_TEXTURE_2D, artextid);
				glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, textwidth, textheight, GL_BGR, GL_UNSIGNED_BYTE, rsdata.planes[0]);
				glDrawArrays(GL_TRIANGLES, 0, 6);
				glBindTexture(GL_TEXTURE_2D, 0);
				glBindVertexArray(0);
				glUseProgram(0);
				glDepthFunc(GL_LESS);
				color_img->ReleaseAccess(&rsdata);
				// go fetching the next sample
				sm->ReleaseFrame();
#endif // AR_TEST

#ifdef MM_TEST
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				glUseProgram(PRG_mm);
				glBindVertexArray(VAO_mm);

				glActiveTexture(GL_TEXTURE0);
				glUniform1i(glGetUniformLocation(PRG_mm, "artex"), 0);
				glBindTexture(GL_TEXTURE_2D, mmtextid);
				glDrawArrays(GL_TRIANGLES, 0, 6);
				glBindTexture(GL_TEXTURE_2D, 0);
				glBindVertexArray(0);
				glUseProgram(0);
#endif // MM_TEST

#endif // !CVCAP

				// Avoids an error when calling SetAndClearRenderSurface during next iteration.
				// Without this, during the next while loop iteration SetAndClearRenderSurface
				// would bind a framebuffer with an invalid COLOR_ATTACHMENT0 because the texture ID
				// associated with COLOR_ATTACHMENT0 had been unlocked by calling wglDXUnlockObjectsNV.
				eyeRenderTexture[eye]->UnsetRenderSurface();

				// Commit changes to the textures so they get picked up frame
				eyeRenderTexture[eye]->Commit();
			}
			// Do distortion rendering, Present and flush/sync
			
			ovrLayerEyeFov ld;
			ld.Header.Type = ovrLayerType_EyeFov;
			ld.Header.Flags = ovrLayerFlag_TextureOriginAtBottomLeft;   // Because OpenGL.
			ld.SensorSampleTime = sensorSampleTime;
			for (int eye = 0; eye < 2; ++eye) {
				ld.ColorTexture[eye] = eyeRenderTexture[eye]->TextureChain;
				ld.Viewport[eye] = Recti(eyeRenderTexture[eye]->GetSize());
				ld.Fov[eye] = hmdDesc.DefaultEyeFov[eye];
				ld.RenderPose[eye] = EyeRenderPose[eye];
			}

			ovrLayerHeader* layers = &ld.Header;
			_r = ovr_SubmitFrame(session, frameIndex, nullptr, &layers, 1);
			// exit the rendering loop if submit returns an error, will retry on ovrError_DisplayLost
			if (!OVR_SUCCESS(_r)) {
				cout << "Submit frame failed" << endl;
				break;
			}
			frameIndex++;
		}
		// Blit mirror texture to back buffer
		glBindFramebuffer(GL_READ_FRAMEBUFFER, mirrorFBO);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		GLint w = windowSize.w;
		GLint h = windowSize.h;
		glBlitFramebuffer(0, h, w, 0,
			0, 0, w, h,
			GL_COLOR_BUFFER_BIT, GL_NEAREST);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
		SDL_GL_SwapWindow(gl->GetWindow_sdl());
	}
	
	if (mirrorFBO) {
		glDeleteFramebuffers(1, &mirrorFBO);
	}
	if (mirrorTexture) {
		ovr_DestroyMirrorTexture(session, mirrorTexture);
	}
	for (int eye = 0; eye < 2; ++eye) {
		delete eyeRenderTexture[eye];
		delete eyeDepthTexture[eye];
	}
	ovr_Destroy(session);
	return true;
}

void s(ovrrs_tc *_r) {
	_r->Start();
}

void s_fh(ovrrs_fh* _f) {
	_f->Start();
}

int main(int argc, char* argv[]) {
	rsf = new ovrrs_fh();
	thread rsthrd = thread(s_fh, rsf);
	kal = new KalmanFilter<glm::vec3>(glm::vec3(1.f, 1.f, 1.f), glm::vec3(1.f, 1.f, 1.f), glm::vec3(1.f, 1.f, 1.f));
	//Use this to wait for init a glm*, DANGEROUS!!!!@
	Sleep(100);
#ifdef CVCAP
	capture.open(0);
	if (!capture.isOpened()) {
		cout << "capture failed" << endl;
		system("pause");
		return -1;
	}
#endif // CVCAP

#ifndef CVCAP
	//// Create a PXCSenseManager instance
	//sm = PXCSenseManager::CreateInstance();
	//// Select the color stream
	//sm->EnableStream(PXCCapture::STREAM_TYPE_COLOR, 1920, 1080);
	//// Initialize and Stream Samples
	//sm->Init();
#endif // !CVCAP

	ovrInitParams initParams = { ovrInit_RequestVersion, OVR_MINOR_VERSION, NULL, 0, 0 };
	var result = ovr_Initialize(&initParams);
	VALIDATE(OVR_SUCCESS(result), "Failed to initialize libOVR.");
	MainLoop();
	rsf->Stop();
	rsthrd.join();
	IMG_Quit();
	SDL_Quit();
	//system("pause");
	return 0;
}