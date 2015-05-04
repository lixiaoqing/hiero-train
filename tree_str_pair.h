#ifndef STR_TREE_PAIR_H
#define STR_TREE_PAIR_H
#include "stdafx.h"
#include "myutils.h"

typedef pair<int,int> Span;			//由起始位置和span长度表示（span长度为实际长度减1）

// 句法树节点
struct SyntaxNode
{
	string label;                      //该节点的句法标签或者词
	SyntaxNode* father;
	vector<SyntaxNode*> children;
	Span span;                //该节点的span
	
	SyntaxNode ()
	{
		father   = NULL;
		span = make_pair(-1,-1);
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
		TreeStrPair(string &line_tree,string &line_str,string &line_align);
		~TreeStrPair()
		{
			delete root_src;
		}

	private:
		void build_tree_from_str(const string &line_tree,SyntaxNode* &root,vector<string> &words);
		bool load_alignment(const string &align_line);
		void cal_proj_span();
		Span merge_span(Span span1,Span span2);
		void check_alignment_agreement();
		void cal_span_for_each_node(SyntaxNode* node,vector<vector<bool> > &span_to_node_flag);

	public:
		bool flag;														//记录初始化是否成功
		SyntaxNode* root_src;
		vector<vector<Span> > src_span_to_tgt_span;						//记录每个源端span投射到目标端的span
		vector<vector<Span> > tgt_span_to_src_span;						//记录每个目标端span投射到源端的span
		vector<vector<int> > src_idx_to_tgt_idx;						//记录每个源端位置对应的目标端位置
		vector<vector<int> > tgt_idx_to_src_idx;						//记录每个目标端位置对应的源端位置
		vector<vector<bool> > src_span_to_node_flag;					//记录每个目标端span是否有对应的句法节点
		vector<vector<bool> > src_span_to_alignment_agreement_flag;		//记录每个源端span是否满足对齐一致性
		vector<vector<vector<string> > > src_span_to_rules;				//记录每个源端span能抽取的所有规则
		vector<string> src_words;
		vector<string> tgt_words;
		int src_sen_len;
		int tgt_sen_len;
};

#endif
