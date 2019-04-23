#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include "Ngram.h"
#include "Vocab.h"
#include "VocabMap.h"

#define PRUNING_THRESHOLD 150

using namespace std;

void usage(){
	printf("[usage]:./mydisambig -text <text_file> -map <mapping_file> -lm <language_model> -order <order>\n");
	exit(-1);
}

void getCandidate(VocabMap& map, Vocab& key, Vocab& value,const char* ch, vector<string>& v);
Prob getBigramProb(Ngram& lm, Vocab& voc, const char* c1,const char* c2);
Prob getTrigramProb(Ngram& lm, Vocab& voc, const char* c1, const char* c2, const char* c3);
void splitLine(string& s, vector<string>& v);
void findBestLineBi(Ngram& lm, VocabMap& map, Vocab& voc, Vocab& key, Vocab& value, vector<string>& line, vector<string>& bestLine);
void findBestLineTri(Ngram& lm, VocabMap& map, Vocab& voc, Vocab& key, Vocab& value, vector<string>& line, vector<string>& bestLine);

int main(int argc, char* argv[]){
	if(argc != 9) usage();
	int ngram_order = atoi(argv[8]);
	Vocab voc, key, value;
	VocabMap map(key, value);
	Ngram lm(voc, ngram_order);

	File mapFile(argv[4], "r");
	map.read(mapFile);
	mapFile.close();
	File lmFile(argv[6], "r");
	lm.read(lmFile);
	lmFile.close();
	ifstream textFile(argv[2]);
	string line;
	while(getline(textFile, line)){
		vector<string> v, bst;
		splitLine(line, v);
		if (ngram_order == 2) findBestLineBi(lm, map, voc, key, value, v, bst);
		else if(ngram_order == 3) findBestLineTri(lm, map, voc, key, value, v, bst);
		else {cout << "[Error]: Only support order of 2 and 3!!!" << endl; exit(-1);}
		cout << "<s>";
		for(vector<string>::iterator w = bst.begin(); w != bst.end(); w++) cout << " " << *w;
		cout << " </s>" << endl;
	}
	textFile.close();
}

void splitLine(string& s, vector<string>& v){
	istringstream iss(s);
	string S;
	while(getline(iss, S, ' ')){
		if(S.length() > 0) v.push_back(S);
	}
}

void getCandidate(VocabMap& map, Vocab& key, Vocab& value, const char* ch, vector<string>& v){
	VocabIndex ch_idx = key.getIndex(ch), value_idx;
	if(ch_idx == Vocab_None) return;
	VocabMapIter it(map, ch_idx); it.init();
	Prob p;
	v.clear();
	while(it.next(value_idx, p)) v.push_back(value.getWord(value_idx));
}

Prob getBigramProb(Ngram& lm, Vocab& voc, const char* c1, const char* c2){
	VocabIndex c1_idx = voc.getIndex(c1), c2_idx = voc.getIndex(c2);
	if(strlen(c1) == 0){ //get unigram
		c2_idx = (c2_idx == Vocab_None)? voc.getIndex(Vocab_Unknown) : c2_idx;
		VocabIndex context[] = {Vocab_None};
		return lm.wordProb(c2_idx, context);
	}
	else{ // get bigram
		c1_idx = (c1_idx == Vocab_None)? voc.getIndex(Vocab_Unknown) : c1_idx;
		c2_idx = (c2_idx == Vocab_None)? voc.getIndex(Vocab_Unknown) : c2_idx;
		VocabIndex context[] = {c1_idx, Vocab_None};
		return lm.wordProb(c2_idx, context);
	}
}

Prob getTrigramProb(Ngram& lm, Vocab& voc, const char* c1, const char* c2, const char* c3){
	VocabIndex c1_idx = voc.getIndex(c1), c2_idx = voc.getIndex(c2), c3_idx = voc.getIndex(c3);
	if(strlen(c1) == 0 && strlen(c2) == 0){ //get unigram
		c3_idx = (c3_idx == Vocab_None)? voc.getIndex(Vocab_Unknown) : c3_idx;
		VocabIndex context[] = {Vocab_None};
		return lm.wordProb(c3_idx, context);
	}else if(strlen(c1) == 0){ // get bigram
		c2_idx = (c2_idx == Vocab_None)? voc.getIndex(Vocab_Unknown) : c2_idx;
		c3_idx = (c3_idx == Vocab_None)? voc.getIndex(Vocab_Unknown) : c3_idx;
		VocabIndex context[] = {c2_idx, Vocab_None};
		return lm.wordProb(c3_idx, context);
	}
	else{ // get trigram
		c1_idx = (c1_idx == Vocab_None)? voc.getIndex(Vocab_Unknown) : c1_idx;
		c2_idx = (c2_idx == Vocab_None)? voc.getIndex(Vocab_Unknown) : c2_idx;
		c3_idx = (c3_idx == Vocab_None)? voc.getIndex(Vocab_Unknown) : c3_idx;
		VocabIndex context[] = {c2_idx, c1_idx, Vocab_None};
		return lm.wordProb(c3_idx, context);
	}
}

struct Compare {
    Compare(Vocab* voc,  Ngram* lm) { this->voc = voc; this->lm = lm;}
    bool operator () (string i, string j) {
    	VocabIndex c1_idx = voc->getIndex(i.c_str()), c2_idx = voc->getIndex(j.c_str());
    	c1_idx = (c1_idx == Vocab_None)? voc->getIndex(Vocab_Unknown): c1_idx;
    	c2_idx = (c2_idx == Vocab_None)? voc->getIndex(Vocab_Unknown): c2_idx;
    	VocabIndex context[] = {Vocab_None};
    	return lm->wordProb(c1_idx, context) > lm->wordProb(c2_idx, context);
    }
    Vocab* voc;
    Ngram* lm;
};


void findBestLineBi(Ngram& lm, VocabMap& map, Vocab& voc, Vocab& key, Vocab& value, vector<string>& line, vector<string>& bestLine){
	/*cout << "line:";
	for (vector<string>::iterator it = line.begin(); it != line.end(); it++){
		cout << "/" << *it;
	}
	cout << endl;*/

	vector<vector<string>> word;

	for(vector<string>::iterator w_it = line.begin(); w_it != line.end(); w_it++){
		vector<string> candidates;
		getCandidate(map, key, value, (*w_it).c_str(), candidates);
		word.push_back(candidates);
	}
	/*for(int w = 0; w < word.size(); w++){
		for(int cd = 0; cd < word[w].size(); cd++){
			cout << word[w][cd] << "/";
		}
		cout << endl;
	}*/

	vector<vector<Prob>> prob;
	vector<vector<int>> bt;

	for(int w = 0; w < word.size(); w++){
		prob.push_back(vector<Prob>());
		bt.push_back(vector<int>());
		for(int cd = 0; cd < word[w].size(); cd++){
			if(w == 0){
				Prob p = getBigramProb(lm, voc, "", word[w][cd].c_str());
				prob[0].push_back(p);
				bt[0].push_back(-1);
				//cout << word[w][cd] << ":" << p <<endl;
			}else{
				Prob maxP = -10000, p;
				int maxW;
				for(int cd_prev = 0; cd_prev < word[w-1].size(); cd_prev++){
					p = getBigramProb(lm, voc, word[w-1][cd_prev].c_str(), word[w][cd].c_str());
					if ((p + prob[w-1][cd_prev]) > maxP) maxP = p + prob[w-1][cd_prev], maxW = cd_prev;
					//cout << word[w-1][cd_prev] << " " << word[w][cd] << ":" << p << endl;
				}
				prob[w].push_back(maxP);
				bt[w].push_back(maxW);
			}
		}
	}

	int maxCd = 0;
	Prob maxP = -100000;
	for (int cd = 0; cd < word[word.size()-1].size(); cd++){
		if(prob[word.size()-1][cd] > maxP) maxP = prob[word.size()-1][cd], maxCd = cd;
	}

	for(int w = word.size()-1; w >= 0; w--){
		bestLine.insert(bestLine.begin(), word[w][maxCd]);
		maxCd = bt[w][maxCd];
	}
}

void findBestLineTri(Ngram& lm, VocabMap& map, Vocab& voc, Vocab& key, Vocab& value, vector<string>& line, vector<string>& bestLine){
	vector<vector<string>> word;

	for(vector<string>::iterator w_it = line.begin(); w_it != line.end(); w_it++){
		vector<string> candidates;
		getCandidate(map, key, value, (*w_it).c_str(), candidates);
		sort(candidates.begin(), candidates.end(), Compare(&voc, &lm));
		int cd_cnt = candidates.size();
		if (cd_cnt >= PRUNING_THRESHOLD) candidates.erase(candidates.begin()+PRUNING_THRESHOLD, candidates.end());
		word.push_back(candidates);
	}

	vector<vector<vector<Prob>>> prob;
	vector<vector<vector<int>>> bt;
	for(int w = 0; w < word.size(); w++){
		prob.push_back(vector<vector<Prob>>());
		bt.push_back(vector<vector<int>>());
		if(w == 0){
			prob[w].push_back(vector<Prob>());
			bt[w].push_back(vector<int>());
			for(int cd = 0; cd < word[w].size(); cd++){
				prob[w][0].push_back(-10000);
				bt[w][0].push_back(-1);
			}
		}
		else{
			for(int cd_prev1 = 0; cd_prev1 < word[w-1].size(); cd_prev1++){
				prob[w].push_back(vector<Prob>());
				bt[w].push_back(vector<int>());
				for(int cd = 0; cd < word[w].size(); cd++){
					prob[w][cd_prev1].push_back(-10000);
					bt[w][cd_prev1].push_back(-1);
				}
			}
		}
	}	

	for(int w = 0; w < word.size(); w++){
		for(int cd = 0; cd < word[w].size(); cd++){
			if(w == 0){
				Prob uni_p = getTrigramProb(lm, voc, "", "", word[w][cd].c_str());
				prob[w][0][cd] = uni_p;
				bt[w][0][cd] = -1;
				//cout << word[w][cd] << ":" << uni_p <<endl;
			}else if(w == 1){
				Prob bi_p;
				for(int cd_prev1 = 0; cd_prev1 < word[w-1].size(); cd_prev1++){
					bi_p = getTrigramProb(lm, voc, "", word[w-1][cd_prev1].c_str(), word[w][cd].c_str());
					prob[w][cd_prev1][cd] = bi_p + prob[w-1][0][cd_prev1];
					bt[w][cd_prev1][cd] = -1;
					//cout << word[w-1][cd_prev1] << " " << word[w][cd] << ":" << bi_p <<endl;
				}
			}else{
				for(int cd_prev1 = 0; cd_prev1 < word[w-1].size(); cd_prev1++){
					Prob maxP = -10000, tri_p, p;
					int maxW;
					for(int cd_prev2 = 0; cd_prev2 < word[w-2].size(); cd_prev2++){
						tri_p = getTrigramProb(lm, voc, word[w-2][cd_prev2].c_str(), word[w-1][cd_prev1].c_str(), word[w][cd].c_str());
						if ((tri_p + prob[w-1][cd_prev2][cd_prev1]) > maxP) maxP = tri_p + prob[w-1][cd_prev2][cd_prev1], maxW = cd_prev2;
						//cout << word[w-2][cd_prev2] << " " << word[w-1][cd_prev1] << " " << word[w][cd] << ":" << tri_p << endl;
					}
					prob[w][cd_prev1][cd] = maxP;
					bt[w][cd_prev1][cd] = maxW;
				}
			}
		}
	}
	int maxCd = 0, maxPrev1 = 0;
	Prob maxP = -100000;
	for (int cd = 0; cd < word[word.size()-1].size(); cd++){
		for (int prev1 = 0; prev1 < word[word.size()-2].size(); prev1++){
			if(prob[word.size()-1][prev1][cd] > maxP) maxP = prob[word.size()-1][prev1][cd], maxCd = cd, maxPrev1 = prev1;
		}
	}
	bestLine.insert(bestLine.begin(), word[word.size()-1][maxCd]);
	bestLine.insert(bestLine.begin(), word[word.size()-2][maxPrev1]);
	int maxPrev1_s = 0;
	for(int w = word.size()-1; w >= 2; w--){
		maxPrev1_s = maxPrev1;
		maxPrev1 = bt[w][maxPrev1][maxCd];
		maxCd = maxPrev1_s;
		bestLine.insert(bestLine.begin(), word[w-2][maxPrev1]);
	}
}
