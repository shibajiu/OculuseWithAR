#ifndef _OVR_LM_H_
#define _OVR_LM_H_
#pragma once

#include <Leap.h>
#include <mutex>
#include <glm/gtx/quaternion.hpp>
#include <glm/glm.hpp>

class SampleListener;
class LP_HandResult {
private:
	Leap::Vector result[50];
	Leap::Vector wrist;
	Leap::Vector bones[40];
	Leap::Vector factor;
	Leap::Matrix transformation;
	friend class SampleListener;
public:
	LP_HandResult();

	void SetFactor(Leap::Vector v);
	void SetTransfomationMatrix(Leap::Matrix m);
	void SetWrist(Leap::Vector wrist);
	void SetBone(int finger, int bone, Leap::Vector* boneV);
	void SetBone(int finger, int bone, Leap::Vector boneS, Leap::Vector boneE);
	Leap::Vector* toVectors();
};

class SampleListener : public Leap::Listener {
private:
	LP_HandResult _lr;
	bool isLog = false;
	std::mutex result_lock;
	bool _previous_fist_state = false;
	bool _current_fist_state = false;
	bool isFist = false;
	Leap::Vector palm;
	Leap::Vector factor = Leap::Vector(1, 1, 1);
	Leap::Matrix transformation = Leap::Matrix();
	void setFistState(bool);
public:
	virtual void onInit(const Leap::Controller&);
	virtual void onConnect(const Leap::Controller&);
	virtual void onDisconnect(const Leap::Controller&);
	virtual void onExit(const Leap::Controller&);
	virtual void onFrame(const Leap::Controller&);
	virtual void onFocusGained(const Leap::Controller&);
	virtual void onFocusLost(const Leap::Controller&);
	virtual void onDeviceChange(const Leap::Controller&);
	virtual void onServiceConnect(const Leap::Controller&);
	virtual void onServiceDisconnect(const Leap::Controller&);
	virtual void onServiceChange(const Leap::Controller&);
	virtual void onDeviceFailure(const Leap::Controller&);
	virtual void onLogMessage(const Leap::Controller&, Leap::MessageSeverity severity, int64_t timestamp, const char* msg);
	Leap::Vector* GetHandResult();
	Leap::Vector GetPalmPosition();
	void SetTransfomationMatrix(Leap::Matrix m);
	void SetLogState(bool _l);
	bool GetFistState() const;
	void Lock();
	void Unlock();
	void TurnOffFist();
	void SetFactor(Leap::Vector v);
	glm::vec3 get_trackball_pos(float x, float y);
	glm::quat get_trackball_quat(glm::vec3 _s, glm::vec3 _d);
};

#endif // !_OVR_LM_H_
