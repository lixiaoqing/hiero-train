#ifndef RULE_EXTRACTOR_H
#define RULE_EXTRACTOR_H
#include "stdafx.h"
#include "myutils.h"
#include "tree_str_pair.h"
#include "rule_counter.h"

class RuleExtractor
{
	public:
		RuleExtractor(string &line_tree,string &line_str,string &line_align,map<string,double> *lex_s2t,map<string,double> *lex_t2s,RuleCounter *counter);
		~RuleExtractor()
		{
			delete tspair;
		}
		void extract_rules();

	private:
		void fill_span2rules_with_AX_XA_XAX_rule();
		void fill_span2rules_with_AXX_XXA_rule();
		void fill_span2rules_with_AXB_AXBX_XAXB_rule();
		void fill_span2rules_with_AXBXC_rule();
		string get_words_according_to_spans(pair<int,int> span,pair<int,int> span_X1,pair<int,int> span_X2,vector<string> &words);
		void generate_rule_according_to_src_spans(pair<int,int> span,pair<int,int> span_X1,pair<int,int> span_X2);
		bool check_alignment_and_node_constraints(pair<int,int> span,pair<int,int> span_X1,pair<int,int> span_X2);

	private:
		TreeStrPair *tspair;
};

#endif

