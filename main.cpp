#include "stdafx.h"
#include "myutils.h"
#include "rule_extractor.h"

int main(int argc, char* argv[])
{
	ifstream ft(argv[1]);
	ifstream fs(argv[2]);
	ifstream fa(argv[3]);
	string line_tree,line_str,line_align;
	while(getline(ft,line_tree))
	{
		getline(fs,line_str);
		getline(fa,line_align);
		RuleExtractor rule_extractor(line_tree,line_str,line_align);
		rule_extractor.extract_rules();
		rule_extractor.dump_rules();
	}
}
