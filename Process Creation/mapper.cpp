#include<iostream>
#include<fstream>
#include<string>
#include<cstring>

using namespace std;

void transform(char userId[], char action[], char topic[]) {

	int actionval = 0;
	ofstream outfile;
	outfile.open("Mapper_Output.txt", ios::app);


	if((userId[0] != '\0') && (action[0] != '\0') && (topic[0] != '\0')) {
	
		if(action[0] == 'P') {
			actionval = 50;
		}
		else if(action[0] == 'L') {
			actionval = 20;
		}
		else if(action[0] == 'D') {
			actionval = -10;
		}
		else if(action[0] == 'C') {
			actionval = 30;
		}
		else if(action[0] == 'S') {
			actionval = 40;
		}
		else {
			actionval = 0;
		}		

	outfile <<"("<<userId<<","<<topic<<","<<actionval<<")"<<"\n";
	outfile.close();
	}
}

void extract(char str[]) {

	char* piece = strtok(str, ",");
	char dataset[3][15];
	int counter = 0;

	while(piece != NULL) {

		strcpy(dataset[counter], piece);
		piece = strtok(NULL, ",");
		counter++;

	}

	transform(dataset[0],dataset[1],dataset[2]);

}



int main(int argc, char** argv) {

	string inputFile = argv[1];

	string line;
	char record[23];
	
	ifstream ofile("Mapper_Output.txt");
	if (ofile) {

		remove("Mapper_Output.txt");
	}


	ifstream infile;
	infile.open(inputFile);

	while(getline(infile, line)) {

		for(int i=1; i<=line.size(); i=i+25) {

			strcpy(record, line.substr(i,22).c_str());
			extract(record);

		}


	}

	infile.close();
	return 0;

}