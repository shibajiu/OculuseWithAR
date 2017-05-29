#include "stdafx.h"
#include "ovrlm.h"

using namespace Leap;

const std::string fingerNames[] = { "Thumb", "Index", "Middle", "Ring", "Pinky" };
const std::string boneNames[] = { "Metacarpal", "Proximal", "Middle", "Distal" };

void SampleListener::setFistState(bool fist) {
	if (_current_fist_state != fist) {
		_previous_fist_state = _current_fist_state;
		isFist = _current_fist_state = fist;
	}
}

void SampleListener::onInit(const Controller& controller) {
	std::cout << "Initialized" << std::endl;
}

void SampleListener::onConnect(const Controller& controller) {
	std::cout << "Connected" << std::endl;
}

void SampleListener::onDisconnect(const Controller& controller) {
	// Note: not dispatched when running in a debugger.
	std::cout << "Disconnected" << std::endl;
}

void SampleListener::onExit(const Controller& controller) {
	std::cout << "Exited" << std::endl;
}

void SampleListener::onFrame(const Controller& controller) {
	// Get the most recent frame and report some basic information
	const Frame frame = controller.frame();
	if (isLog) {
		std::cout << "Frame id: " << frame.id()
			<< ", timestamp: " << frame.timestamp()
			<< ", hands: " << frame.hands().count()
			<< ", extended fingers: " << frame.fingers().extended().count() << std::endl;
	}

	HandList hands = frame.hands();
	for (HandList::const_iterator hl = hands.begin(); hl != hands.end(); ++hl) {
		// Get the first hand
		const Hand hand = *hl;
		var angle = hand.grabAngle();
		if (angle > 2) {
			setFistState(true);
		}
		else {
			setFistState(false);
		}
		if (isLog) {
			std::string handType = hand.isLeft() ? "Left hand" : "Right hand";
			std::cout << std::string(2, ' ') << handType << ", id: " << hand.id()
				<< ", palm position: " << hand.palmPosition() << std::endl;

			// Get the hand's normal vector and direction
			const Vector normal = hand.palmNormal();
			const Vector direction = hand.direction();

			// Calculate the hand's pitch, roll, and yaw angles
			std::cout << std::string(2, ' ') << "pitch: " << direction.pitch() * RAD_TO_DEG << " degrees, "
				<< "roll: " << normal.roll() * RAD_TO_DEG << " degrees, "
				<< "yaw: " << direction.yaw() * RAD_TO_DEG << " degrees" << std::endl;

			// Get the Arm bone
			Arm arm = hand.arm();
			std::cout << std::string(2, ' ') << "Arm direction: " << arm.direction()
				<< " wrist position: " << arm.wristPosition()
				<< " elbow position: " << arm.elbowPosition() << std::endl;
		}
		this->result_lock.lock();
		this->_lr.SetWrist(hand.arm().wristPosition());
		palm = hand.palmPosition();
		// Get fingers
		const FingerList fingers = hand.fingers();
		int _f(0);
		for (FingerList::const_iterator fl = fingers.begin(); fl != fingers.end(); ++fl) {
			const Finger finger = *fl;
			if (isLog) {
				std::cout << std::string(4, ' ') << fingerNames[finger.type()]
					<< " finger, id: " << finger.id()
					<< ", length: " << finger.length()
					<< "mm, width: " << finger.width() << std::endl;
			}
			
			// Get finger bones
			for (int b = 0; b < 4; ++b) {
				Bone::Type boneType = static_cast<Bone::Type>(b);
				Bone bone = finger.bone(boneType);
				this->_lr.SetBone(_f, b, bone.prevJoint(), bone.nextJoint());
				if (isLog) {
					std::cout << std::string(6, ' ') << boneNames[boneType]
						<< " bone, start: " << bone.prevJoint()
						<< ", end: " << bone.nextJoint()
						<< ", direction: " << bone.direction() << std::endl;
				}
			}
			++_f;
		}
		this->result_lock.unlock();
		//Only one hand
		break;
	}

	/*if (!frame.hands().isEmpty()) {
		std::cout << std::endl;
	}*/

}

void SampleListener::onFocusGained(const Controller& controller) {
	std::cout << "Focus Gained" << std::endl;
}

void SampleListener::onFocusLost(const Controller& controller) {
	std::cout << "Focus Lost" << std::endl;
}

void SampleListener::onDeviceChange(const Controller& controller) {
	std::cout << "Device Changed" << std::endl;
	const DeviceList devices = controller.devices();

	for (int i = 0; i < devices.count(); ++i) {
		std::cout << "id: " << devices[i].toString() << std::endl;
		std::cout << "  isStreaming: " << (devices[i].isStreaming() ? "true" : "false") << std::endl;
		std::cout << "  isSmudged:" << (devices[i].isSmudged() ? "true" : "false") << std::endl;
		std::cout << "  isLightingBad:" << (devices[i].isLightingBad() ? "true" : "false") << std::endl;
	}
}

void SampleListener::onServiceConnect(const Controller& controller) {
	std::cout << "Service Connected" << std::endl;
}

void SampleListener::onServiceDisconnect(const Controller& controller) {
	std::cout << "Service Disconnected" << std::endl;
}

void SampleListener::onServiceChange(const Controller& controller) {
	std::cout << "Service Changed" << std::endl;
}

void SampleListener::onDeviceFailure(const Controller& controller) {
	std::cout << "Device Error" << std::endl;
	const Leap::FailedDeviceList devices = controller.failedDevices();

	for (FailedDeviceList::const_iterator dl = devices.begin(); dl != devices.end(); ++dl) {
		const FailedDevice device = *dl;
		std::cout << "  PNP ID:" << device.pnpId();
		std::cout << "    Failure type:" << device.failure();
	}
}

void SampleListener::onLogMessage(const Controller&, MessageSeverity s, int64_t t, const char* msg) {
	switch (s) {
	case Leap::MESSAGE_CRITICAL:
		std::cout << "[Critical]";
		break;
	case Leap::MESSAGE_WARNING:
		std::cout << "[Warning]";
		break;
	case Leap::MESSAGE_INFORMATION:
		std::cout << "[Info]";
		break;
	case Leap::MESSAGE_UNKNOWN:
		std::cout << "[Unknown]";
	}
	std::cout << "[" << t << "] ";
	std::cout << msg << std::endl;
}

Leap::Vector* SampleListener::GetHandResult() { 
	return _lr.toVectors(); 
}

Leap::Vector SampleListener::GetPalmPosition() { 
	return transformation*(palm*factor);
}

void SampleListener::SetTransfomationMatrix(Leap::Matrix m) { 
	transformation = m;
	_lr.SetTransfomationMatrix(m); 
}

void SampleListener::SetLogState(bool _l) { 
	isLog = _l; 
}

bool SampleListener::GetFistState() const {
	return isFist;
}

void SampleListener::Lock() { 
	result_lock.lock(); 
}

void SampleListener::Unlock() {
	result_lock.unlock();
}

void SampleListener::TurnOffFist() {
	isFist = false;
}

void SampleListener::SetFactor(Leap::Vector v) {
	factor = v; 
	_lr.SetFactor(v); 
}

glm::vec3 SampleListener::get_trackball_pos(float x, float y) {
	var _te = x * x + y * y;
	return glm::vec3();
}

glm::quat SampleListener::get_trackball_quat(glm::vec3 _s, glm::vec3 _d) {
	return glm::quat();
}

LP_HandResult::LP_HandResult():wrist(), factor(1, 1, 1), transformation() {
	for (int i = 0; i < 40; ++i)
		bones[i] = Leap::Vector();
}

void LP_HandResult::SetFactor(Leap::Vector v) { factor = v; }

void LP_HandResult::SetTransfomationMatrix(Leap::Matrix m) { transformation = m; }

void LP_HandResult::SetWrist(Leap::Vector wrist) {
	this->wrist = wrist;
}
void LP_HandResult::SetBone(int finger, int bone, Leap::Vector * boneV) {
	this->bones[2 * (finger * 4 + bone)] = boneV[0];
	this->bones[2 * (finger * 4 + bone) + 1] = boneV[1];
}

void LP_HandResult::SetBone(int finger, int bone, Leap::Vector boneS, Leap::Vector boneE) {
	this->bones[2 * (finger * 4 + bone)] = boneS;
	this->bones[2 * (finger * 4 + bone) + 1] = boneE;
}
Leap::Vector * LP_HandResult::toVectors() {
	for (int f = 0; f < 5; ++f) {
		result[f * 10] = transformation * (this->wrist * factor);
		result[f * 10 + 1] = transformation * (this->bones[f * 8] * factor);
		for (int b = 2; b < 10; ++b)
		{
			result[f * 10 + b] = transformation * (this->bones[f * 8 + b - 2] * factor);
		}
	}
	return result;
}
