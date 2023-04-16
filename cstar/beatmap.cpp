#include "pch.h"
#include "beatmap.h"

bool hcmp(HitObject left, HitObject right) {
	return left.time < right.time;
}

bool tcmp(TimingPoint left, TimingPoint right) {
	return left.time < right.time;
}

vector<string> ___split(string input, char delimiter) {
	vector<string> answer;
	stringstream ss(input);
	string temp;

	while (getline(ss, temp, delimiter)) {
		answer.push_back(temp);
	}

	return answer;
}

TimingPoint::TimingPoint(string format) {
	vector<string> inputs = ___split(format, ',');
	this->time = atoi(inputs[0].c_str());
	this->beatLength = atof(inputs[1].c_str());
	this->meter = atoi(inputs[2].c_str());
	this->sampleSet = atoi(inputs[3].c_str());
	this->sampleIndex = atoi(inputs[4].c_str());
	this->volume = atoi(inputs[5].c_str());
	this->uninherited = atoi(inputs[6].c_str());
	this->effects = atoi(inputs[7].c_str());
}

TimingPoint::TimingPoint() {
	this->time = 999999999;
	this->beatLength = 0.0;
	this->meter = 0;
	this->sampleSet = 0;
	this->sampleIndex = 0;
	this->volume = 0;
	this->uninherited = 0;
	this->effects = 0;
}

HitObject::HitObject(string format) {

	vector<string> ins = ___split(format, ',');
	this->x = atoi(ins[0].c_str());
	this->y = atoi(ins[1].c_str());
	this->time = atoi(ins[2].c_str());
	this->type = atoi(ins[3].c_str());
	this->hitSound = atoi(ins[4].c_str());

	int start = 5;
	if (this->type & (1 << 1)) { // Slider Object

		vector<string> curves = ___split(ins[start++], '|');
		if (curves[0] == "B") this->extra.curveType = CurveType::Bezier;
		else if (curves[0] == "C") this->extra.curveType = CurveType::Central;
		else if (curves[0] == "L") this->extra.curveType = CurveType::Linear;
		else this->extra.curveType = CurveType::Perfect;

		for (size_t i = 1; i < curves.size(); i++) {
			vector<string> form = ___split(curves[i], ':');
			pair<int, int> _new = { atoi(form[0].c_str()), atoi(form[1].c_str()) };
			this->extra.curvePoints.push_back(_new);
		}

		this->extra.slides = atoi(ins[start++].c_str());
		this->extra.length = atoi(ins[start++].c_str());

		if (ins.size() == start) { // osu file format v14
			return;
		}

		vector<string> es = ___split(ins[start++], '|');
		for (size_t i = 0; i < es.size(); i++) {
			this->extra.edgeSounds.push_back(atoi(es[i].c_str()));
		}

		vector<string> edges = ___split(ins[start++], '|');
		for (size_t i = 0; i < edges.size(); i++) {
			vector<string> form = ___split(edges[i], ':');
			pair<int, int> _new = { atoi(form[0].c_str()), atoi(form[1].c_str()) };
			this->extra.edgeSets.push_back(_new);
		}
	}
	
	vector<string> hs = ___split(ins[start], ':');
	for (size_t i = 0; i < hs.size(); i++) {
		if (!hs[i].empty()) this->hitSamples.push_back(atoi(hs[i].c_str()));
	}

}

void Beatmap::Parse(map<string, string>& name_path) {

	auto midx = name_path.find(this->Name);
	if (midx == name_path.end()) {
		cout << "Beatmap not found!" << '\n';
		return;
	}
	string path = midx->second;

	ifstream osuFile(path);
	if (osuFile.is_open()) {
		string form;
		bool writingTP = false, writingHO = false;
		while (getline(osuFile, form)) {
			cout << (form.empty() ? "None" : form) << '\n';
			if (form == "[TimingPoints]") writingTP = true;
			else if (form == "[HitObjects]") writingHO = true;
			else if (form.empty()) { writingHO = false; writingTP = false; }
			else if (writingHO) { HitObject toWrite(form); this->hitObjects.push_back(toWrite); }
			else if (writingTP) { TimingPoint toWrite(form); this->timingPoints.push_back(toWrite); }
			else if (___split(form, ':')[0] == "SliderMultiplier") { this->sliderMultiplier = atof(___split(form, ':')[1].c_str()); }
		}
		cout << 1 << '\n';
	}
	else {
		cout << "Unable to open file" << '\n';
	}
	return;

}