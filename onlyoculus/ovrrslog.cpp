#include "stdafx.h"
#include "ovrrslog.h"

using namespace std;

void WriteLog(vector<array<glm::vec3,3>> points) {
	SYSTEMTIME time;
	::GetLocalTime(&time);

	char *filepath = "d:\\joint";
	DWORD attr = ::GetFileAttributesA(filepath);
	if (attr == INVALID_FILE_ATTRIBUTES) {
		system("md d:\\joint");
	}

	char file0[256], file1[256], file2[256];
	sprintf_s(file0, "D:\\joint\\%d-%d-%d-%d-%d-%d@joint0",
		time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond);
	sprintf_s(file1, "D:\\joint\\%d-%d-%d-%d-%d-%d@joint1",
		time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond);
	sprintf_s(file2, "D:\\joint\\%d-%d-%d-%d-%d-%d@joint2",
		time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond);

	ofstream f0(file0);
	ofstream f1(file1);
	ofstream f2(file2);

	for (var point:points) {
		if (point[0].x|| point[0].y|| point[0].z) {
			f0 << point[0].x << "\t" << point[0].y << "\t" << point[0].z << endl;
			f1 << point[1].x << "\t" << point[1].y << "\t" << point[1].z << endl;
			f2 << point[2].x << "\t" << point[2].y << "\t" << point[2].z << endl;
		}
	}
	f0.close();
	f1.close();
	f2.close();
	cout << "Log Completed" << endl;
}

void WriteLogWithSpeed(const vector<array<glm::vec3, 3>>position, const vector<array<glm::vec3, 3>>speed) {
	SYSTEMTIME time;
	::GetLocalTime(&time);

	char *filepath = "d:\\joint";
	DWORD attr = ::GetFileAttributesA(filepath);
	if (attr == INVALID_FILE_ATTRIBUTES) {
		system("md d:\\joint");
	}

	char file0[256], file1[256], file2[256];
	sprintf_s(file0, "D:\\joint\\%d-%d-%d-%d-%d-%d@joint0",
		time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond);
	sprintf_s(file1, "D:\\joint\\%d-%d-%d-%d-%d-%d@joint1",
		time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond);
	sprintf_s(file2, "D:\\joint\\%d-%d-%d-%d-%d-%d@joint2",
		time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond);

	ofstream f0(file0);
	ofstream f1(file1);
	ofstream f2(file2);

	for (int _i = 0; _i < position.size();++_i) {
		if (position[_i][0].x || position[_i][0].y || position[_i][0].z) {
			f0 << position[_i][0].x << "\t" 
				<< position[_i][0].y << "\t" 
				<< position[_i][0].z <<"\t"
				<< speed[_i][0].x << "\t"
				<< speed[_i][0].y << "\t"
				<< speed[_i][0].z << "\t"
				<< endl;
			f1 << position[_i][1].x << "\t" 
				<< position[_i][1].y << "\t" 
				<< position[_i][1].z << "\t"
				<< speed[_i][1].x << "\t"
				<< speed[_i][1].y << "\t"
				<< speed[_i][1].z << "\t"
				<< endl;
			f2 << position[_i][2].x << "\t" 
				<< position[_i][2].y << "\t" 
				<< position[_i][2].z << "\t"
				<< speed[_i][2].x << "\t"
				<< speed[_i][2].y << "\t"
				<< speed[_i][2].z << "\t"
				<< endl;
		}
	}
	f0.close();
	f1.close();
	f2.close();
	cout << "Log Completed" << endl;
}