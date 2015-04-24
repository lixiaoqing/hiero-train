#include "stdafx.h"
#include "myutils.h"
#include "rule_extractor.h"

int main(int argc, char* argv[])
{
	ifstream fs(argv[1]);
	ifstream ft(argv[2]);
	ifstream fa(argv[3]);
	string line_tree,line_str,line_align;
	int num = 0;
	ofstream fs2t("rules.s2t");
	ofstream ft2s("rules.t2s");
	if (!fs2t.is_open() || !ft2s.is_open())
		return 0;
	while(getline(fs,line_str))
	{
		getline(ft,line_tree);
		getline(fa,line_align);
		RuleExtractor rule_extractor(line_str,line_tree,line_align);
		if (rule_extractor.flag == false)
			continue;
		rule_extractor.extract_rules();
		rule_extractor.dump_rules(fs2t,ft2s);
		num++;
		if (num%1000 == 0)
			cerr<<num<<endl;
	}
}
