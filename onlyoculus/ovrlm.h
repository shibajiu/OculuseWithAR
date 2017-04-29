#ifndef _OVR_LM_H_
#define _OVR_LM_H_
#pragma once
#include <Leap.h>
#include <mutex>
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
	LP_HandResult():wrist(), factor(1,1,1),transformation(){
		for (int i = 0; i < 40; ++i)
			bones[i] = Leap::Vector();
	}

	void SetFactor(Leap::Vector v) { factor = v; }

	void SetTransfomationMatrix(Leap::Matrix m) { transformation = m; }

	void SetWrist(Leap::Vector wrist) {
		this->wrist = wrist;
	}

	void SetBone(int finger, int bone, Leap::Vector* boneV) {
		this->bones[2 * (finger * 4 + bone)] = boneV[0];
		this->bones[2 * (finger * 4 + bone) + 1] = boneV[1];
	}

	void SetBone(int finger, int bone, Leap::Vector boneS, Leap::Vector boneE) {
		this->bones[2 * (finger * 4 + bone)] = boneS;
		this->bones[2 * (finger * 4 + bone) + 1] = boneE;
	}

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
	Leap::Vector GetPalmPosition() { return transformation*(palm*factor); }
	void SetTransfomationMatrix(Leap::Matrix m) { transformation = m; _lr.SetTransfomationMatrix(m); }
	void SetLogState(bool _l) { isLog = _l; }
	bool GetFistState() const { return isFist; }
	void Lock() { result_lock.lock(); }
	void Unlock() { result_lock.unlock(); }
	void TurnOffFist() { isFist = false; }
	void SetFactor(Leap::Vector v) { factor = v; _lr.SetFactor(v); }
};

#endif // !_OVR_LM_H_
