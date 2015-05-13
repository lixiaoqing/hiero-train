#include "stdafx.h"
#include "myutils.h"
#include "rule_extractor.h"

int main(int argc, char* argv[])
{
	ifstream fs(argv[1]);
	ifstream ft(argv[2]);
	ifstream fa(argv[3]);
	string line_tree,line_str,line_align;
	vector<vector<string> > line_tree_vecs;
	vector<vector<string> > line_str_vecs;
	vector<vector<string> > line_align_vecs;
	int block_size = 1000;
	load_data_into_blocks(line_tree_vecs,fs,block_size);
	load_data_into_blocks(line_str_vecs,ft,block_size);
	load_data_into_blocks(line_align_vecs,fa,block_size);
	ofstream fs2t("rules.s2t");
	ofstream ft2s("rules.t2s");
	if (!fs2t.is_open() || !ft2s.is_open())
		return 0;
	int processed_num = 0;
	int block_num = line_tree_vecs.size();
	for (size_t i=0;i<block_num;i++)
	{
		vector<vector<string> > fw_rules_vec;
		vector<vector<string> > bw_rules_vec;
		block_size = line_tree_vecs.at(i).size();
		fw_rules_vec.resize(block_size);
		bw_rules_vec.resize(block_size);
#pragma omp parallel for num_threads(16)
		for (size_t j=0;j<block_size;j++)
		{
			RuleExtractor rule_extractor(line_tree_vecs.at(i).at(j),line_str_vecs.at(i).at(j),line_align_vecs.at(i).at(j));
			if (rule_extractor.flag == false)
				continue;
			rule_extractor.extract_rules();
			rule_extractor.dump_rules(fw_rules_vec.at(j),bw_rules_vec.at(j));
		}
		processed_num += block_size;
		cerr<<processed_num<<endl;
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
}
