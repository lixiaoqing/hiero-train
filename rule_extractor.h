#ifndef RULE_EXTRACTOR_H
#define RULE_EXTRACTOR_H
#include "stdafx.h"
#include "myutils.h"
#include "tree_pair.h"

class RuleExtractor
{
	public:
		RuleExtractor(string &line_tree_src,string &line_tree_tgt,string &line_align);
		~RuleExtractor()
		{
			delete tree_pair;
		}
		void extract_rules();
		void dump_rules(ofstream &fs2t,ofstream &ft2s);

	private:
		void fill_span2rules_with_AX_XA_XAX_rule();
		void fill_span2rules_with_AXX_XXA_rule();
		void fill_span2rules_with_AXB_AXBX_XAXB_rule();
		void fill_span2rules_with_AXBXC_rule();
		string get_words_according_to_spans(Span span,Span span_X1,Span span_X2,vector<string> &words);
		string get_alignment_inside_rule(Span span,Span span_X1,Span span_X2,Span tgt_span,Span tgt_span_X1,Span tgt_span_X2);
		void generate_rule_according_to_src_spans(Span span,Span span_X1,Span span_X2);
		bool check_alignment_constraint(Span span,Span span_X1,Span span_X2);
		bool check_node_constraint(Span span,Span span_X1,Span span_X2,vector<vector<bool> > &span_to_node_flag);
		bool check_boundary_constraint(Span span_X1,Span span_X2);

	public:
		bool flag;				//记录初始化是否成功
	private:
		TreePair *tree_pair;
		map<string,int> rule_table;
};

#endif

