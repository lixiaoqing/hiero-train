#ifndef TREE_STR_PAIR_H
#define TREE_STR_PAIR_H
#include "stdafx.h"
#include "myutils.h"
#include "rule_counter.h"

struct Rule
{
	string rule_src;						//规则源端部分
	string rule_tgt;						//规则目标端部分
	int type;								//规则类型，0和1表示包含0个或1个非终结符，2和3表示包含两个非终结符的正序或逆序规则
};

// 源端句法树节点
struct SyntaxNode
{
	string label;                           //该节点的句法标签或者词
	SyntaxNode* father;
	vector<SyntaxNode*> children;
	pair<int,int> src_span;                 //该节点对应的源端span,用首位两个单词的位置表示
	pair<int,int> tgt_span;                 //该节点对应的目标端span
	int type;                               //节点类型，0：单词节点，1：边界节点，2：非边界节点
	
	SyntaxNode ()
	{
		father   = NULL;
		src_span = make_pair(-1,-1);
		tgt_span = make_pair(-1,-1);
		type     = -1;
	}
	~SyntaxNode ()
	{
		for (auto node : children)
		{
			delete node;
		}
	}
};

class TreeStrPair
{
	public:
		TreeStrPair(string &line_tree,string &line_str,string &line_align,map<string,double> *plex_s2t,map<string,double> *plex_t2s,RuleCounter *counter);
		~TreeStrPair()
		{
			delete root;
		}

	private:
		void build_tree_from_str(const string &line_of_tree);
		void load_alignment(const string &align_line);
		void cal_proj_span();
		pair<int,int> merge_span(pair<int,int> span1,pair<int,int> span2);
		void check_alignment_agreement();
		void check_frontier_for_nodes_in_subtree(SyntaxNode* node);

	public:
        RuleCounter *rule_counter;
		SyntaxNode* root;
		vector<vector<pair<int,int> > > src_span_to_tgt_span;			//记录每个源端span投射到目标端的span，span用起始位置和跨度长度来表示
		vector<vector<pair<int,int> > > tgt_span_to_src_span;			//记录每个目标端端span投射到源的span
		vector<vector<bool> > src_span_to_node_flag;					//记录每个源端span是否有对应的句法节点
		vector<vector<bool> > src_span_to_alignment_agreement_flag;		//记录每个源端span是否满足对齐一致性
		vector<vector<vector<Rule> > > src_span_to_rules;				//记录每个源端span能抽取的所有规则
		vector<string> src_words;
		vector<string> tgt_words;
		int src_sen_len;
		int tgt_sen_len;
        map<string,double> *lex_s2t;
        map<string,double> *lex_t2s;
};

#endif
