#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>

using namespace std;

struct OptionData {
	string timestamp;
	double strike;
	string type;
	int volume;
};

vector<OptionData> options;

int main() {
	ifstream file("spy-sample-clean.csv");
	if (!file.is_open()) {
		cerr << "Error opening file" << endl;
		return 1;
	}

	string line;

	while(getline(file, line)) {
		
	}
 
	return 0;
}