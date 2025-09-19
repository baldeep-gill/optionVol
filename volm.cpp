#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <map>
#include <charconv>
#include <cstdlib>

using namespace std;

map<string, double> callData;
map<string, double> putData;
map<string, double> spotPrice;

int safe_stoi(const string& s) {
	int val = 0;
	auto start = s.data();
	auto end = s.data() + s.size();

	auto [ptr, ec] = from_chars(start, end, val);

	if (ec == errc()) {
		return val;
	}

	return 0;
}

double safe_stod(const string& s) {
	char* end;
	const char* start = s.c_str();
	double value = strtod(start, &end);

	if (end == start) {
		return 0.0;
	}

	return value;
}

void get_csv(string filename) {
	ifstream file(filename);
	if (!file.is_open()) {
		cerr << "Error opening file" << endl;
	}

	string line;
	getline(file, line);
	getline(file, line);

	string g_time = "09:30";
	int call_acc = 0, put_acc = 0, call_vol = 0, put_vol = 0;
	double spot = 0;
	
	while (getline(file, line)) {
		stringstream ss(line);
		string timestamp, date, time, expiry, type, temp;
		double strike;
		int cvolume, pvolume;
		
		getline(ss, timestamp, ','); // DATETIME
		getline(ss, temp, ','); // DATE - throwaway
		getline(ss, temp, ','); // TIME - throwaway

		getline(ss, temp, ','); // SPOT
		spot = safe_stod(temp);

		getline(ss, expiry, ','); // EXPIRY

		getline(ss, temp, ','); // CALL VOLUME
		if (temp != "") cvolume = safe_stoi(temp);
		else cvolume = 0;

		getline(ss, temp, ','); // STRIKE
		strike = safe_stod(temp);

		getline(ss, temp, ','); // PUT VOLUME
		if (temp != "") pvolume = safe_stoi(temp);
		else pvolume = 0;
		
		auto pos = timestamp.find(' ');
		if (pos != string::npos) {
			date = timestamp.substr(0, pos);
			time = timestamp.substr(pos + 1);
		}

		if (expiry != date) continue;

		if (time != g_time) {
			if (call_vol > 0) callData[g_time] = call_acc / call_vol;
			if (put_vol > 0) putData[g_time] = put_acc / put_vol;
			spotPrice[g_time] = spot;

			g_time = time;
			call_acc = call_vol = put_acc = put_vol = 0;
		} 

		call_acc += strike * cvolume;
		call_vol += cvolume;

		put_acc += strike * pvolume;
		put_vol += pvolume;
	}

	if (call_vol > 0) callData[g_time] = call_acc / call_vol;
	if (put_vol > 0) putData[g_time] = put_acc / put_vol;
	spotPrice[g_time] = spot;
	
	file.close();
}

int main() {
	get_csv("spy-sample-clean.csv");
	
	for (auto snapshot: spotPrice) {
		cout << "Time: " << snapshot.first << " | " << snapshot.second << endl;
	}

	return 0;
}