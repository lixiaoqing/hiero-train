#include "stdafx.h"
#include "myutils.h"
#include "rule_extractor.h"

int main(int argc, char* argv[])
{
	ifstream fs(argv[1]);
	ifstream ft(argv[2]);
	ifstream fa(argv[3]);
	string line_tree,line_str,line_align;
	vector<string> line_tree_vec;
	vector<string> line_str_vec;
	vector<string> line_align_vec;
	while(getline(fs,line_str))
	{
		getline(ft,line_tree);
		getline(fa,line_align);
		line_tree_vec.push_back(line_tree);
		line_str_vec.push_back(line_str);
		line_align_vec.push_back(line_align);
	}
	int sen_num = line_tree_vec.size();
	vector<vector<string> > fw_rules_vec;
	vector<vector<string> > bw_rules_vec;
	fw_rules_vec.resize(sen_num);
	bw_rules_vec.resize(sen_num);
#pragma omp parallel for num_threads(16)
	for (size_t i=0;i<sen_num;i++)
	{
		RuleExtractor rule_extractor(line_str_vec.at(i),line_tree_vec.at(i),line_align_vec.at(i));
		if (rule_extractor.flag == false)
			continue;
		rule_extractor.extract_rules();
		rule_extractor.dump_rules(fw_rules_vec.at(i),bw_rules_vec.at(i));
		if (i%1000 == 0)
			cerr<<'*';
	}
	cerr<<endl;
	ofstream fs2t("rules.s2t");
	ofstream ft2s("rules.t2s");
	if (!fs2t.is_open() || !ft2s.is_open())
		return 0;
	for (auto &rules : fw_rules_vec)
	{
		for (auto &rule : rules)
		{
			fs2t<<rule<<endl;
		}
	}
	for (auto &rules : bw_rules_vec)
	{
		for (auto &rule : rules)
		{
			ft2s<<rule<<endl;
		}
	}
}
