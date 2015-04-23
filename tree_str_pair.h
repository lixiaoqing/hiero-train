#ifndef TREE_STR_PAIR_H
#define TREE_STR_PAIR_H
#include "stdafx.h"
#include "myutils.h"

// 源端句法树节点
struct SyntaxNode
{
	string label;                           //该节点的句法标签或者词
	SyntaxNode* father;
	vector<SyntaxNode*> children;
	pair<int,int> src_span;                //该节点的源端span
	pair<int,int> tgt_span;                //该节点的目标端span
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
		TreeStrPair(string &line_str,string &line_tree,string &line_align);
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
		SyntaxNode* root;
		vector<vector<pair<int,int> > > src_span_to_tgt_span;			//记录每个源端span投射到目标端的span，span用起始位置和跨度长度来表示
		vector<vector<pair<int,int> > > tgt_span_to_src_span;			//记录每个目标端span投射到源的span
		vector<vector<int> > src_idx_to_tgt_idx;						//记录每个源端位置对应的目标端位置
		vector<vector<int> > tgt_idx_to_src_idx;						//记录每个目标端位置对应的源端位置
		vector<vector<bool> > tgt_span_to_node_flag;					//记录每个目标端span是否有对应的句法节点
		vector<vector<bool> > src_span_to_alignment_agreement_flag;		//记录每个源端span是否满足对齐一致性
		vector<vector<bool> > tgt_span_to_alignment_agreement_flag;		//记录每个目标端span是否满足对齐一致性
		vector<vector<vector<string> > > src_span_to_rules;				//记录每个源端span能抽取的所有规则
		vector<string> src_words;
		vector<string> tgt_words;
		int src_sen_len;
		int tgt_sen_len;
};

#endif
