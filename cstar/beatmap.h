#pragma once
#include "pch.h"

using namespace std;

enum CurveType {
	Bezier,
	Central,
	Linear,
	Perfect
};

struct TimingPoint {
public:
	int time;
	double beatLength;
	int meter, sampleSet, sampleIndex, volume;
	bool uninherited;
	int effects;

	TimingPoint(string format);
	TimingPoint();
};

struct Extra {
public:
	CurveType curveType;
	vector<pair<int, int> > curvePoints;
	int slides;
	double length;
	vector<int> edgeSounds;
	vector<pair<int, int> > edgeSets;
};

struct HitObject {
public:
	int x, y, time, type, hitSound;
	Extra extra;
	vector<int> hitSamples;

	HitObject(string format);
};

class Beatmap {

public:
	DWORD Base;
	string Name; // Offset = 94
	int BeatmapID; // Offset = CC

	Beatmap() {
		this->Base = 0;
		this->Name = "None";
		this->BeatmapID = -1;
		this->hitObjects = vector<HitObject>();
		this->timingPoints = vector<TimingPoint>();
	}
	Beatmap(DWORD base) {

		this->Base = base;
		DWORD nameStartAddr = *(DWORD*)(base + 0x94) + 8; // String Start Address = 0x8
		string buf = "";
		int i = 0;
		while(*(char*)(nameStartAddr + i) != NULL) {
			char x = *(char*)(nameStartAddr + i); // UTF-16
			buf += x;
			i += 2;
		}
		this->Name = buf;
		this->BeatmapID = *(int*)(base + 0xCC);
		this->hitObjects = vector<HitObject>();
		this->timingPoints = vector<TimingPoint>();
	}

	vector<TimingPoint> timingPoints;
	vector<HitObject> hitObjects;
	double sliderMultiplier;

	void Parse(map<string, string> &name_path);

};