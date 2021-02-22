#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using namespace std;

typedef struct Data
{
	string userId;
	string topic;
	int score;

}DATA;

int main(int argc, char** argv) {

	vector<DATA> tuple;
	ifstream infile;
	infile.open(argv[1]);
	string line;


	while(getline(infile, line)) {

		DATA item;
		item.userId = line.substr(1,4);
		item.topic = line.substr(6,15);

		if(line.size() == 25) {
			item.score = stoi(line.substr(22,2));
		}
		else {
			item.score = stoi(line.substr(22,3));
		}

		tuple.push_back(item);		
	}	

		infile.close();
		ofstream outfile ("output.txt");

		for (vector<DATA>::iterator i = tuple.begin(); i!= tuple.end(); ++i) {


				int scoreSum = (*i).score;
				int rep = scoreSum;

				int flag = 0;

				if (i  != tuple.begin()) {

					vector<DATA>::iterator j = i-1;
					while(j != tuple.begin() || j == tuple.begin()) {

						if ((*i).userId.compare((*j).userId) == 0 &&
							(*i).topic.compare((*j).topic) == 0) {

							flag = 1;
							break;
						}	


						if (j != tuple.begin()) {
							--j;
						}
						else
							break;
					}	
				}

				if(flag)
					continue;

				for (vector<DATA>::iterator k = tuple.begin(); k!= tuple.end(); ++k) {

					if ((*i).userId.compare((*k).userId) == 0 &&
						(*i).topic.compare((*k).topic) == 0) {

							scoreSum += (*k).score; 
					}
				}

				outfile << "(" <<(*i).userId<<","<<(*i).topic<<","<<(scoreSum-rep)<<")\n";	
		}


		return 0;
}

