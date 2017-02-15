// onlyoculus.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "SDLGL.h"
#include "OvrGLItems.h"
#include "loadwinthAssimp.h"
//cv part
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#if defined(_WIN32)
#include <dxgi.h> // for GetDefaultAdapterLuid
#pragma comment(lib, "dxgi.lib")
#endif

using namespace std;

SDLGL *gl = nullptr;
cv::VideoCapture capture;
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
	1.0,1.0,
	-1.0,1.0,
	-1.0,-1.0,
	-1.0,-1.0,
	1.0,-1.0,
	1.0,1.0
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

	A_model model("d:/castle.obj");
	var PRG_castle = SDLGL::LoadShader_sdl_s("cast.vert", "cast.frag");
	var PRG_skybox = SDLGL::LoadShader_sdl_s("skybox.vert", "skybox.frag");
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
			static float Yaw(3.141592f);
			static Vector3f Pos2(0.0f, 0.0f, -5.0f);
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

				Matrix4f rollPitchYaw = Matrix4f::RotationY(Yaw);
				Matrix4f finalRollPitchYaw = rollPitchYaw * Matrix4f(EyeRenderPose[eye].Orientation);
				//cout << eye << ":\t" << EyeRenderPose[eye].Orientation.x << ends << EyeRenderPose[eye].Orientation.y << ends << EyeRenderPose[eye].Orientation.z << endl;
				Vector3f finalUp = finalRollPitchYaw.Transform(Vector3f(0, 1, 0));
				Vector3f finalForward = finalRollPitchYaw.Transform(Vector3f(0, 0, -1));
				Vector3f shiftedEyePos = Pos2 + rollPitchYaw.Transform(EyeRenderPose[eye].Position);

				Matrix4f view = Matrix4f::LookAtRH(shiftedEyePos, shiftedEyePos + finalForward, finalUp);
				Matrix4f proj = ovrMatrix4f_Projection(hmdDesc.DefaultEyeFov[eye], 0.2f, 1000.0f, ovrProjection_None);
				static glm::mat4 model(1);
				//Render part
				glDepthFunc(GL_LEQUAL);
				glUseProgram(PRG_skybox);
				//glm use column-major order
				glUniformMatrix4fv(glGetUniformLocation(PRG_skybox, "model"), 1, GL_FALSE, glm::value_ptr(model));
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

int main(int argc, char* argv[]) {
	capture.open(0);
	if (!capture.isOpened()) {
		cout << "capture failed" << endl;
		system("pause");
		return -1;
	}

	ovrInitParams initParams = { ovrInit_RequestVersion, OVR_MINOR_VERSION, NULL, 0, 0 };
	var result = ovr_Initialize(&initParams);
	VALIDATE(OVR_SUCCESS(result), "Failed to initialize libOVR.");
	MainLoop();
	IMG_Quit();
	SDL_Quit();
	//system("pause");
	return 0;
}