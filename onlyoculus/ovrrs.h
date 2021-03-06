#pragma once
#include <pxcsensemanager.h>
#include <pxctouchlesscontroller.h>
#include <pxchandmodule.h>
#include <pxchandconfiguration.h>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/norm.hpp>
#include "Tree.h"
#include <vector>
#include <array>

using namespace std;

#define IFCERR(b,msg) if(b){cerr<<msg<<endl;return;} 
#define PXCIFERR(b,msg) IFCERR(b<pxcStatus::PXC_STATUS_NO_ERROR,msg)
#define MAX_NUMBER_OF_JOINTS 22
#define MAX_NUMBER_OF_HANDS 2
#define RS_IMAGECOORD_WIDTH 640
#define RS_IMAGECOORD_HEIGHT 480

typedef PXCTouchlessController::AlertData::AlertType ovrAttp;
class ovrUXEventHandler;
class ovrAlertHandler;
class ovrHandAlertHandler;
class HandsModel;
class ovrHandGestureHandler;
struct JointPositionSpeed;

class ovrrs_tc {
private:
	friend class ovrUXEventHandler;
	friend class ovrAlertHandler;
	PXCSession *ps;
	PXCSenseManager *psm;
	PXCTouchlessController *ptc;
	ovrUXEventHandler *handlerUXEvent;
	ovrAlertHandler * handlerAlert;
	glm::mat4 roll;
	glm::vec3 oldpos;
	glm::vec3 pos;
	glm::vec3 moved;
	const ovrrs_tc& operator =(const ovrrs_tc& o) = delete;
	ovrrs_tc(const ovrrs_tc&) = delete;
	const ovrrs_tc& operator =(const ovrrs_tc&& o) = delete;
	ovrrs_tc(const ovrrs_tc&&) = delete;

	bool isStop;
	ovrAttp alert;

	void SetConfiguration();
	void SetAlert(ovrAttp);

public:
	//using GL::setWindowTitle(GLFWwindow*, bool);
	ovrrs_tc();
	~ovrrs_tc();
	void Start();
	glm::vec3 getHandMove();
	glm::vec3 getHandPos();
	void Stop();
	ovrAttp GetAlert();
};

class ovrUXEventHandler :public PXCTouchlessController::UXEventHandler {
private:
	friend class ovrrs_tc;
	ovrrs_tc* rs = nullptr;
	glm::vec3 PXCPoint3_to_vec3(const PXCPoint3DF32 &);
	glm::vec3 getPosition_from_UXEventData(PXCTouchlessController::UXEventData *);

public:
	virtual void PXCAPI OnFiredUXEvent(const PXCTouchlessController::UXEventData *);
};

class ovrAlertHandler :public PXCTouchlessController::AlertHandler {
private:
	friend class ovrrs_tc;
	ovrrs_tc* rs = nullptr;
public:
	virtual void PXCAPI OnFiredAlert(const PXCTouchlessController::AlertData *);
};

class ovrActionHandler :public PXCTouchlessController::ActionHandler {
public:
	virtual void PXCAPI OnFiredAction(const PXCTouchlessController::Action);
};

class ovrrs_fh {
private:
	friend class HandsModel;
	ovrHandAlertHandler *alert;
	glm::vec3 orientation;
	bool isStop = false;
	HandsModel* handsmodel;
	ovrHandGestureHandler* hg_handler;
	PXCPoint3DF32 joint_center_imagecoord;
	static glm::vec3 trkb_center;
	static GLfloat trkb_radius_sqr;

	const ovrrs_fh& operator =(const ovrrs_fh& o) = delete;
	ovrrs_fh(const ovrrs_fh&) = delete;
	const ovrrs_fh& operator =(const ovrrs_fh&& o) = delete;
	ovrrs_fh(const ovrrs_fh&&) = delete;

	void init();
	glm::vec3 PXCPoint3DF32_to_vec3(PXCPoint3DF32 _p) const;
	glm::quat PXCPoint4DF32_to_quat(PXCPoint4DF32 _p) const;

public:
	ovrrs_fh();
	int Start();
	int Stop();
	glm::vec2 GetHandCenterImageCoord();
	const bool GetFistState() const throw();
	void TurnOffFist();
	glm::vec3 GetWristOrientation() const;
	JointPositionSpeed* GetJointPoints() const;
	int GetLogCount() const;
	const vector<array<glm::vec3, 3>>& GetLog_p() const;
	const vector<array<glm::vec3, 3>>& GetLog_s() const;
	void Release();

	static glm::vec3 get_trackball_pos(glm::vec2);
	static glm::vec3 get_trackball_pos(float x, float y);
	static glm::quat get_trackball_quat(glm::vec3 _s, glm::vec3 _d);

	~ovrrs_fh();
};

class ovrHandAlertHandler :public PXCHandConfiguration::AlertHandler {
public:
	virtual void PXCAPI OnFiredAlert(const PXCHandData::AlertData &_d);
};

class ovrHandGestureHandler :public PXCHandConfiguration::GestureHandler {
private:
	friend class ovrrs_fh;
	ovrrs_fh* fh;
	char isFist;

public:
	ovrHandGestureHandler(ovrrs_fh* _f) :fh(_f) , isFist(0){};
	void setFistState(char);
	virtual void PXCAPI OnFiredGesture(const PXCHandData::GestureData &data);
};

struct JointPositionSpeed {
	glm::vec3 position;
	glm::vec3 speed;

	JointPositionSpeed():position(0),speed(0){}
};

class HandsModel {
private:
	friend class ovrrs_fh;
	ovrrs_fh* fh;
	int HandCount;
	vector<array<glm::vec3, 3>> handlog_p;
	vector<array<glm::vec3, 3>> handlog_s;
	Tree<PointData>* skeletontree;
	PXCHandData* handdata;
	bool righthand = false, lefthand = false;
	JointPositionSpeed *jointpoints_t = new JointPositionSpeed[MAX_NUMBER_OF_JOINTS];
	void copyJointToPoint(PointData & dst, const PXCHandData::JointData & src);

public:
	HandsModel(PXCHandData*, ovrrs_fh*);
	~HandsModel();
	void updateskeletonTree();
	glm::vec3 PXCPoint3DF32_to_vec3(PXCPoint3DF32 _p) const;
	JointPositionSpeed* GetPoint();
	void Release();
};