#include "stdafx.h"
#include "myutils.h"
#include "rule_extractor.h"

int main(int argc, char* argv[])
{
	ifstream ft(argv[1]);
	ifstream fs(argv[2]);
	ifstream fa(argv[3]);
	string line_tree,line_str,line_align;
	int num = 0;
	ofstream fc2e("rules.c2e");
	ofstream fe2c("rules.e2c");
	if (!fc2e.is_open() || !fe2c.is_open())
		return 0;
	while(getline(ft,line_tree))
	{
		getline(fs,line_str);
		getline(fa,line_align);
		RuleExtractor rule_extractor(line_tree,line_str,line_align);
		rule_extractor.extract_rules();
		rule_extractor.dump_rules(fc2e,fe2c);
		num++;
		if (num%1000 == 0)
			cerr<<num<<endl;
	}
}
