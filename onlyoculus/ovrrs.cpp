#include "stdafx.h"
#include "ovrrs.h"

using namespace std;

ovrrs_tc::ovrrs_tc() {
	ps = nullptr;
	psm = nullptr;
	ptc = nullptr;
	handlerAlert = new ovrAlertHandler();
	handlerUXEvent = new ovrUXEventHandler();
	handlerUXEvent->rs = this;
	handlerAlert->rs = this;
	isStop = false;
	roll = glm::mat4(1.f);
	pos = oldpos = glm::vec3(0.5f);
	moved = glm::vec3(0.f);
	alert = ovrAttp::Alert_NoAlerts;
}

ovrrs_tc::~ovrrs_tc() {
	ptc->UnsubscribeAlert(handlerAlert);
	ptc->UnsubscribeEvent(handlerUXEvent);
	delete handlerAlert;
	delete handlerUXEvent;
	//ptc->Release();//managed by psm
	psm->Close();
	psm->Release();
}

void ovrrs_tc::Start() {
	ps = PXCSession::CreateInstance();
	if (!ps) {
		cerr << "PXCSession::CreateInstance::FAILED" << endl;
		return;
	}
	psm = PXCSenseManager::CreateInstance();
	if (!psm) {
		cerr << "PXCSenseManager::CreateInstance::FAILED" << endl;
		return;
	}
	if (psm->EnableTouchlessController() < pxcStatus::PXC_STATUS_NO_ERROR) {
		cerr << "PXCSenseManager::EnableTouchlessController::FAILED" << endl;
		return;
	}
	psm->Init();
	ptc = psm->QueryTouchlessController();
	IFCERR(ptc == NULL, "PXCSenseManager::QueryTouchlessController::FAILED");
	ptc->SubscribeEvent(handlerUXEvent);
	ptc->SubscribeAlert(handlerAlert);

	while (psm->AcquireFrame(0) >= pxcStatus::PXC_STATUS_NO_ERROR) {
		if (isStop) break;
		psm->ReleaseFrame();
	}
}

void ovrrs_tc::SetConfiguration() {
	PXCTouchlessController::ProfileInfo _ptc_info;
	IFCERR(ptc->QueryProfile(&_ptc_info) < pxcStatus::PXC_STATUS_NO_ERROR, "PXCTouchlessController::QueryProfile::FAILED");
	_ptc_info.config |= PXCTouchlessController::ProfileInfo::Configuration_Allow_Zoom |
		PXCTouchlessController::ProfileInfo::Configuration_Scroll_Horizontally |
		PXCTouchlessController::ProfileInfo::Configuration_Scroll_Vertically;
	IFCERR(ptc->SetProfile(&_ptc_info) < pxcStatus::PXC_STATUS_NO_ERROR, "PXCTouchlessController::SetProfile::FAILED");
	ptc->SetScrollSensitivity(1.f);
	ptc->SetPointerSensitivity(PXCTouchlessController::PointerSensitivity_Smoothed);
}

void ovrrs_tc::SetAlert(ovrAttp _a) {
	alert = _a;
}

glm::vec3 ovrrs_tc::getHandMove() {
	//return glm::vec3(0.2, 0, 0);//for test
	return moved;
}

glm::vec3 ovrrs_tc::getHandPos() {
	return pos;
}

void ovrrs_tc::Stop() {
	isStop = true;
}

ovrAttp ovrrs_tc::GetAlert() {
	return alert;
}

void ovrUXEventHandler::OnFiredUXEvent(const PXCTouchlessController::UXEventData * _data) {
	switch (_data->type) {
	case PXCTouchlessController::UXEventData::UXEvent_ScrollDown:

	case PXCTouchlessController::UXEventData::UXEvent_ScrollLeft:

	case PXCTouchlessController::UXEventData::UXEvent_ScrollRight:

	case PXCTouchlessController::UXEventData::UXEvent_ScrollUp:

		break;
	default:
		break;
	}

	if (rs->oldpos == glm::vec3(0.5f)) {
		rs->oldpos = glm::vec3(_data->position.x, _data->position.y, _data->position.z);
		cout << "here" << endl;
	}
	rs->pos = PXCPoint3_to_vec3(_data->position);
	rs->moved = getPosition_from_UXEventData(const_cast<PXCTouchlessController::UXEventData*>(_data)) - rs->oldpos;
}

glm::vec3 ovrUXEventHandler::PXCPoint3_to_vec3(const PXCPoint3DF32 &_position) {
	return glm::vec3(_position.x, _position.y, _position.z);
}

glm::vec3 ovrUXEventHandler::getPosition_from_UXEventData(PXCTouchlessController::UXEventData * _data) {
	return PXCPoint3_to_vec3(_data->position);
}

void ovrAlertHandler::OnFiredAlert(const PXCTouchlessController::AlertData *_data) {
	switch (_data->type) {
	case PXCTouchlessController::AlertData::Alert_TooClose:

	case PXCTouchlessController::AlertData::Alert_TooFar:

	case PXCTouchlessController::AlertData::Alert_NoAlerts:
		rs->SetAlert(_data->type);
		break;
	}
}

void ovrrs_fh::init() {
	var sm = PXCSenseManager::CreateInstance();
	IFCERR(sm == NULL, "PXCSenseManager::CreateInstance::FAILED");
	IFCERR(sm->EnableHand() < pxcStatus::PXC_STATUS_NO_ERROR, "PXCSenseManager::EnableHand::FAILED");
	var hand = sm->QueryHand();

	//Set up configuration
	var config = hand->CreateActiveConfiguration();
	config->EnableStabilizer(true);
	config->SubscribeAlert(alert);
	//config->EnableJointSpeed(PXCHandData::JointType::JOINT_WRIST, PXCHandData::JointSpeedType::JOINT_SPEED_AVERAGE, 0);
	config->EnableJointSpeed(PXCHandData::JointType::JOINT_CENTER, PXCHandData::JointSpeedType::JOINT_SPEED_AVERAGE, 0);
	config->SetTrackingMode(PXCHandData::TrackingModeType::TRACKING_MODE_FULL_HAND);
	config->EnableSegmentationImage(false);
	config->EnableTrackedJoints(true);
	//Enable config
	config->ApplyChanges();
	config->Release();

	var data = hand->CreateOutput();
	handsmodel = new HandsModel(data);
	//cout << "CreateOutput" << endl;
	sm->Init();
	PXCHandData::GestureData gsdata;

	while (sm->AcquireFrame(0) >= pxcStatus::PXC_STATUS_NO_ERROR) {
		//If break the loop
		if (isStop)
			break;
		//Update date
		IFCERR(data->Update() != pxcStatus::PXC_STATUS_NO_ERROR, "PXCHandData::Update::Failed");
		PXCHandData::IHand *ihand = 0;
		if (data->QueryNumberOfHands() != 0) {
			data->QueryHandData(PXCHandData::ACCESS_ORDER_NEAR_TO_FAR, 0, ihand);
			if (data->IsGestureFired(L"fist", gsdata)) {
				isFist = true;
			}
			else {
				isFist = false;
			}
			PXCHandData::JointData jdata;
			handsmodel->updateskeletonTree();
			ihand->QueryTrackedJoint((PXCHandData::JointType)10, jdata);
			var _pos = jdata.positionWorld;
			ihand->QueryTrackedJoint((PXCHandData::JointType)0, jdata);
			var _pos1 = jdata.positionWorld;
			var _t = PXCPoint3DF32_to_vec3(_pos) - PXCPoint3DF32_to_vec3(_pos1);
			//Use this as hand's orientation
			orientation = _t;
			//orientation.x *= -1;
			//cout << orientation.x << "/" << orientation.y << "/" << orientation.z << "/" << endl;

		}
		sm->ReleaseFrame();
	}
	cout << "ovrrs_fh Main loop break" << endl;
	data->Release();
	sm->Release();
}

glm::vec3 ovrrs_fh::PXCPoint3DF32_to_vec3(PXCPoint3DF32 _p)const {
	return glm::vec3(_p.x, _p.y, _p.z);
}

glm::quat ovrrs_fh::PXCPoint4DF32_to_quat(PXCPoint4DF32 _p) const {
	glm::quat _q;
	_q.w = _p.w;
	_q.x = _p.x;
	_q.y = _p.y;
	_q.z = _p.z;
	return _q;
}

int ovrrs_fh::Start() {
	init();
	return 0;
}

int ovrrs_fh::Stop() {
	isStop = true;
	return 0;
}

glm::vec3 ovrrs_fh::GetWristOrientation() const {
	return orientation;
}

JointPositionSpeed* ovrrs_fh::GetJointPoints() const {
	return handsmodel->GetPoint();
}

const bool ovrrs_fh::GetFistStatus() const {
	return isFist;
}

int ovrrs_fh::GetLogCount() const {
	return handsmodel->HandCount;
}

const vector<array<glm::vec3, 3>>& ovrrs_fh::GetLog_p() const {
	return handsmodel->handlog_p;
}

const vector<array<glm::vec3, 3>>& ovrrs_fh::GetLog_s() const {
	return handsmodel->handlog_s;
}

void ovrrs_fh::Release() {
	delete handsmodel;
}

ovrrs_fh::~ovrrs_fh() {
	Release();
}

HandsModel::HandsModel(PXCHandData * _d) :handdata(_d),HandCount(0) {
	skeletontree = new Tree<PointData>[MAX_NUMBER_OF_HANDS];
	/*jointpoints = new vec3[MAX_NUMBER_OF_JOINTS];
	jointpoints_t = new vec3[MAX_NUMBER_OF_JOINTS];*/
}

HandsModel::~HandsModel() {
	Release();
}

void HandsModel::updateskeletonTree() {
	// Iterate over hands
	int numOfHands = handdata->QueryNumberOfHands();//@
	for (int index = 0; index < numOfHands; ++index) {
		// Get hand by access order of entering time
		PXCHandData::IHand* handOutput = NULL;
		if (handdata->QueryHandData(PXCHandData::ACCESS_ORDER_BY_TIME, index, handOutput) == PXC_STATUS_NO_ERROR) {
			// Get hand body side (left, right, unknown)
			int side = 0;
			if (handOutput->QueryBodySide() == PXCHandData::BodySideType::BODY_SIDE_RIGHT) {
				righthand = true;
				side = 0;
			}
			else if (handOutput->QueryBodySide() == PXCHandData::BodySideType::BODY_SIDE_LEFT) {
				lefthand = true;
				side = 1;
			}
			PXCHandData::JointData jointData;
			handOutput->QueryTrackedJoint(PXCHandData::JointType::JOINT_WRIST, jointData);
			PointData pointData;
			copyJointToPoint(pointData, jointData);
			jointpoints_t[0].position = PXCPoint3DF32_to_vec3(pointData.positionWorld);
			jointpoints_t[0].speed = PXCPoint3DF32_to_vec3(pointData.speed);

			Node<PointData> rootDataNode(pointData);

			handOutput->QueryTrackedJoint(PXCHandData::JointType::JOINT_CENTER, jointData);
			copyJointToPoint(pointData, jointData);
			jointpoints_t[1].position = PXCPoint3DF32_to_vec3(pointData.positionWorld);
			jointpoints_t[1].speed = PXCPoint3DF32_to_vec3(pointData.speed);

			// Iterate over hand joints
			for (int i = 2; i < MAX_NUMBER_OF_JOINTS - 3; i += 4) {
				handOutput->QueryTrackedJoint((PXCHandData::JointType)(i + 3), jointData);
				copyJointToPoint(pointData, jointData);
				jointpoints_t[(i + 3)].position = PXCPoint3DF32_to_vec3(pointData.positionWorld);
				jointpoints_t[(i + 3)].speed = PXCPoint3DF32_to_vec3(pointData.speed);

				Node<PointData> dataNode(pointData);
				handOutput->QueryTrackedJoint((PXCHandData::JointType)(i + 2), jointData);
				copyJointToPoint(pointData, jointData);
				jointpoints_t[i + 2].position = PXCPoint3DF32_to_vec3(pointData.positionWorld);
				jointpoints_t[(i + 2)].speed = PXCPoint3DF32_to_vec3(pointData.speed);
				Node<PointData> dataNode1(pointData);
				handOutput->QueryTrackedJoint((PXCHandData::JointType)(i + 1), jointData);
				copyJointToPoint(pointData, jointData);
				jointpoints_t[i + 1].position = PXCPoint3DF32_to_vec3(pointData.positionWorld);
				jointpoints_t[(i + 1)].speed = PXCPoint3DF32_to_vec3(pointData.speed);

				Node<PointData> dataNode2(pointData);
				handOutput->QueryTrackedJoint((PXCHandData::JointType)(i), jointData);
				copyJointToPoint(pointData, jointData);
				jointpoints_t[i].position = PXCPoint3DF32_to_vec3(pointData.positionWorld);
				jointpoints_t[i].speed = PXCPoint3DF32_to_vec3(pointData.speed);

				Node<PointData> dataNode3(pointData);

				dataNode1.add(dataNode);
				dataNode2.add(dataNode1);
				dataNode3.add(dataNode2);
				rootDataNode.add(dataNode3);
			}

			skeletontree[side].setRoot(rootDataNode);
			var muti = glm::vec3(100, 100, 50);
			bool isLog = true;
			if (isLog) {
				if (HandCount < 1500) {
				
					std::array<glm::vec3, 3> _tlog_p = { jointpoints_t[0].position,jointpoints_t[1].position,jointpoints_t[2].position };
					std::array<glm::vec3, 3> _tlog_s = { jointpoints_t[0].speed,jointpoints_t[1].speed,jointpoints_t[2].speed };
					handlog_p.push_back(_tlog_p);
					handlog_s.push_back(_tlog_s);
					cout << "Recording:" << HandCount
						<< "\t" << jointpoints_t[0].position.x
						<< "\t" << jointpoints_t[0].position.y
						<< "\t" << jointpoints_t[0].position.z
						<< endl;
					HandCount += 1;
				}
				
			}
			/*for (int j = 0; j < MAX_NUMBER_OF_JOINTS; ++j) {
			jointpoints[j] = muti*jointpoints_t[j];
			}*/
		}
	}
}

glm::vec3 HandsModel::PXCPoint3DF32_to_vec3(PXCPoint3DF32 _p)const {
	return glm::vec3(_p.x, _p.y, _p.z);
}

JointPositionSpeed* HandsModel::GetPoint() {
	return jointpoints_t;
}

void HandsModel::Release() {
	delete[] jointpoints_t;
	delete[] skeletontree;
}


void HandsModel::copyJointToPoint(PointData & dst, const PXCHandData::JointData & src) {
	dst.confidence = src.confidence;
	dst.globalOrientation = src.globalOrientation;
	dst.localRotation = src.localRotation;
	dst.positionImage = src.positionImage;
	dst.positionWorld = src.positionWorld;
	dst.speed = src.speed;
}

void PXCAPI ovrHandAlertHandler::OnFiredAlert(const PXCHandData::AlertData & _d) {
	return;
}
