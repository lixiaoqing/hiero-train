#include "tree_str_pair.h"

TreeStrPair::TreeStrPair(string &line_tree,string &line_str,string &line_align)
{
	if (line_tree.size() > 3)
	{
		build_tree_from_str(line_tree,root_src,src_words);
		tgt_words = Split(line_str);

		src_sen_len = src_words.size();
		tgt_sen_len = tgt_words.size();

		src_idx_to_tgt_idx.resize(src_sen_len);
		src_span_to_tgt_span.resize(src_sen_len);
		src_span_to_alignment_agreement_flag.resize(src_sen_len);
		src_span_to_node_flag.resize(src_sen_len);
		src_span_to_rules.resize(src_sen_len);
		for (int beg=0;beg<src_sen_len;beg++)
		{
			src_span_to_tgt_span.at(beg).resize(src_sen_len-beg,make_pair(-1,-1));
			src_span_to_alignment_agreement_flag.at(beg).resize(src_sen_len-beg,false);
			src_span_to_node_flag.at(beg).resize(src_sen_len-beg,false);
			src_span_to_rules.at(beg).resize(src_sen_len-beg);
		}

		tgt_idx_to_src_idx.resize(tgt_sen_len);
		tgt_span_to_src_span.resize(tgt_sen_len);
		for (int beg=0;beg<tgt_sen_len;beg++)
		{
			tgt_span_to_src_span.at(beg).resize(tgt_sen_len-beg,make_pair(-1,-1));
		}
		flag = load_alignment(line_align);
		if (flag == false)
			return;
		cal_proj_span();
		check_alignment_agreement();
		cal_span_for_each_node(root_src,src_span_to_node_flag);
	}
	else
	{
		flag = false;
	}
}

/**************************************************************************************
 1. 函数功能: 将字符串解析成句法树
 2. 入口参数: 一句话的句法分析结果，Berkeley Parser格式
 3. 出口参数: 无
 4. 算法简介: 见注释
************************************************************************************* */
void TreeStrPair::build_tree_from_str(const string &line_tree,SyntaxNode* &root,vector<string> &words)
{
	vector<string> toks = Split(line_tree);
	SyntaxNode* cur_node;
	SyntaxNode* pre_node;
	int word_index = 0;
	for(size_t i=0;i<toks.size();i++)
	{
		//左括号情形，且去除"("作为终结符的特例(做终结符时，后继为“）”)
		if(toks[i]=="(" && i+1<toks.size() && toks[i+1]!=")")
		{
			string test=toks[i];
			if(i == 0)
			{
				root     = new SyntaxNode;
				pre_node = root;
				cur_node = root;
			}
			else
			{
				cur_node = new SyntaxNode;
				cur_node->father = pre_node;
				pre_node->children.push_back(cur_node);
				pre_node = cur_node;
			}
		}
		//右括号情形，去除右括号“）”做终结符的特例（做终结符时，前驱的前驱为“（,而且前驱不是")"
		else if(toks[i]==")" && !(i-2>=0 && toks[i-2] =="(" && toks[i-1] != ")"))
		{
			pre_node = pre_node->father;
			cur_node = pre_node;
		}
		//处理形如 （ VV 需要 ）其中VV节点这样的情形
		else if((i-1>=0 && toks[i-1]=="(") && (i+2<toks.size() && toks[i+2]==")"))
		{
			cur_node->label  = toks[i];
			cur_node         = new SyntaxNode;
			cur_node->father = pre_node;
			pre_node->children.push_back(cur_node);
		}
		//处理形如 VP （ VV 需要 ） VP这样的节点 或 需要 这样的节点
		else
		{
			cur_node->label = toks[i];
			//如果是“需要”的情形，则记录中文词的序号
			if(toks[i+1]==")")
			{
				cur_node->span = make_pair(word_index,0);
				words.push_back(toks[i]);
				word_index++;
			}
		}
	}
}

/**************************************************************************************
 1. 函数功能: 加载词对齐
 2. 入口参数: 一句话的词对齐结果
 3. 出口参数: 无
 4. 算法简介: 根据每一对对齐的单词，更新每个源端单词对应的目标端span，以及每个目标端
 			  单词对应的源端span
************************************************************************************* */
bool TreeStrPair::load_alignment(const string &line_align)
{
	vector<string> alignments = Split(line_align);
	for (auto align : alignments)
	{
		vector<string> idx_pair = Split(align,"-");
		int src_idx = stoi(idx_pair.at(0));
		int tgt_idx = stoi(idx_pair.at(1));
		if (src_idx >= src_sen_len || tgt_idx >= tgt_sen_len)
			return false;
		src_span_to_tgt_span[src_idx][0] = merge_span(src_span_to_tgt_span[src_idx][0],make_pair(tgt_idx,0));
		tgt_span_to_src_span[tgt_idx][0] = merge_span(tgt_span_to_src_span[tgt_idx][0],make_pair(src_idx,0));
		src_idx_to_tgt_idx[src_idx].push_back(tgt_idx);
		tgt_idx_to_src_idx[tgt_idx].push_back(src_idx);
	}
	return true;
}

/**************************************************************************************
 1. 函数功能: 计算当前子树中的每个节点的span
 2. 入口参数: 当前子树的根节点
 3. 出口参数: 无
 4. 算法简介: 后序遍历当前子树，根据第一个和最后一个孩子节点的span计算当前节点的span
************************************************************************************* */
void TreeStrPair::cal_span_for_each_node(SyntaxNode* node,vector<vector<bool> > &span_to_node_flag)
{
	if (node->children.empty() )                                                                            //单词节点
		return;
	for (const auto child : node->children)
	{
		cal_span_for_each_node(child,span_to_node_flag);
	}
	node->span = make_pair(node->children.front()->span.first,node->children.back()->span.first+
				 node->children.back()->span.second - node->children.front()->span.first);
	span_to_node_flag[node->span.first][node->span.second] = true;											//更新每个span是否有句法节点
}

/**************************************************************************************
 1. 函数功能: 计算每个源端span投射到目标端的span，以及每个目标端span投射到源端的span
 2. 入口参数: 无
 3. 出口参数: 无
 4. 算法简介: 采用动态规划算法自底向上，自左向右地计算每个span的proj_span，计算公式为
 			  proj_span[beg][len] = proj_span[beg][len-1] + proj_span[beg+len][0]
************************************************************************************* */
void TreeStrPair::cal_proj_span()
{
	for (int span_len=1;span_len<src_sen_len;span_len++)
	{
		for (int beg=0;beg<src_sen_len-span_len;beg++)
		{
			src_span_to_tgt_span[beg][span_len] = merge_span(src_span_to_tgt_span[beg][span_len-1],src_span_to_tgt_span[beg+span_len][0]);
		}
	}
	for (int span_len=1;span_len<tgt_sen_len;span_len++)
	{
		for (int beg=0;beg<tgt_sen_len-span_len;beg++)
		{
			tgt_span_to_src_span[beg][span_len] = merge_span(tgt_span_to_src_span[beg][span_len-1],tgt_span_to_src_span[beg+span_len][0]);
		}
	}
}

/**************************************************************************************
 1. 函数功能: 将两个span合并为一个span
 2. 入口参数: 被合并的两个span
 3. 出口参数: 合并后的span
 4. 算法简介: 见注释
************************************************************************************* */
Span TreeStrPair::merge_span(Span span1,Span span2)
{
	if (span2.first == -1)
		return span1;
	if (span1.first == -1)
		return span2;
	Span span;
	span.first = min(span1.first,span2.first);												// 合并后span的左边界
	span.second = max(span1.first+span1.second,span2.first+span2.second) - span.first;		// 合并后span的长度，即右边界减去左边界
	return span;
}

/**************************************************************************************
 1. 函数功能: 检查每个源端span是否满足词对齐一致性
 2. 入口参数: 无
 3. 出口参数: 无
 4. 算法简介: 根据预先计算好的源端span和目标端span的映射表，检查源端span映射到目标端
 			  再映射回来的span是否越过了原来源端span的边界
************************************************************************************* */
void TreeStrPair::check_alignment_agreement()
{
	for (int beg=0;beg<src_sen_len;beg++)
	{
		for (int span_len=0;span_len<src_sen_len-beg;span_len++)
		{
			Span tgt_span = src_span_to_tgt_span[beg][span_len];
			if (tgt_span.first == -1)
			{
				src_span_to_alignment_agreement_flag[beg][span_len] = false;	  //如果目标端span为空，认为不满足对齐一致性
			}
			else
			{
				Span proj_src_span = tgt_span_to_src_span[tgt_span.first][tgt_span.second];
				//如果目标端span投射回来的源端span不超原来的源端span，则满足对齐一致性
				if (proj_src_span.first >= beg && proj_src_span.first+proj_src_span.second <= beg+span_len)
				{
					src_span_to_alignment_agreement_flag[beg][span_len] = true;
				}
			}
		}
	}
}

