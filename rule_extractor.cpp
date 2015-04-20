#include "rule_extractor.h"

RuleExtractor::RuleExtractor(string &line_tree,string &line_str,string &line_align,map<string,double> *lex_s2t,map<string,double> *lex_t2s,RuleCounter *counter)
{
	tspair = new TreeStrPair(line_tree,line_str,line_align,lex_s2t,lex_t2s,counter);
}

/**************************************************************************************
 1. 函数功能: 检查规则源端的pattern是否满足词对齐和句法节点约束
 2. 入口参数: 规则源端的跨度，以及其中第一个和第二个变量的跨度
 3. 出口参数: 是否满足约束
 4. 算法简介: 如果规则源端跨度或其中的变量的跨度不对应句法节点或不满足词对齐约束，则返回
 			  false，否则返回true
************************************************************************************* */
bool RuleExtractor::check_alignment_and_node_constraints(pair<int,int> span,pair<int,int> span_X1,pair<int,int> span_X2)
{
	if (tspair->src_span_to_alignment_agreement_flag[span.first][span.second] == false)
		return false;
	if (tspair->src_span_to_node_flag[span.first][span.second] == false)
		return false;
	if (tspair->src_span_to_alignment_agreement_flag[span_X1.first][span_X1.second] == false)
		return false;
	if (tspair->src_span_to_node_flag[span_X1.first][span_X1.second] == false)
		return false;
	if (span_X2.first == -1)
		return true;
	if (tspair->src_span_to_alignment_agreement_flag[span_X2.first][span_X2.second] == false)
		return false;
	if (tspair->src_span_to_node_flag[span_X2.first][span_X2.second] == false)
		return false;
	return true;
}

/**************************************************************************************
 1. 函数功能: 根据规则源端以及其中变量的跨度生成规则的最终形式
 2. 入口参数: 规则源端以及其中变量的跨度
 3. 出口参数: 无
 4. 算法简介: 见注释
************************************************************************************* */
void RuleExtractor::generate_rule_according_to_src_spans(pair<int,int> span,pair<int,int> span_X1,pair<int,int> span_X2)
{
	//cout<<"valid src span: "<<span.first<<' '<<span.second<<' '<<span_X1.first<<' '<<span_X1.second<<' '<<span_X2.first<<' '<<span_X2.second<<endl;
	string rule_src = get_words_according_to_spans(span,span_X1,span_X2,tspair->src_words);				// 生成规则源端的字符串表示
	pair<int,int> tgt_span = tspair->src_span_to_tgt_span[span.first][span.second];						// 获取规则目标端以及其中变量的跨度
	pair<int,int> tgt_span_X1 = tspair->src_span_to_tgt_span[span_X1.first][span_X1.second];
	pair<int,int> tgt_span_X2 = make_pair(-1,-1);
	if (span_X2.first != -1)
	{
		tgt_span_X2 = tspair->src_span_to_tgt_span[span_X2.first][span_X2.second];
	}
	string rule_tgt = get_words_according_to_spans(tgt_span,tgt_span_X1,tgt_span_X2,tspair->tgt_words); // 生成规则目标端的字符串表示
	int type;
	if (span_X2.first == -1)
	{
		type = 1;
	}
	else
	{
		type = tgt_span_X1.first<tgt_span_X2.first?2:3;													// 判断规则类型
	}
	Rule rule = {rule_src,rule_tgt,type};
	cout<<rule.rule_src<<" ||| "<<rule.rule_tgt<<" ||| "<<rule.type<<endl;			//4debug
	tspair->src_span_to_rules[span.first][span.second].push_back(rule);
}

/**************************************************************************************
 1. 函数功能: 根据规则(源端或目标端)以及其中变量的跨度生成规则的字符串表示
 2. 入口参数: 规则(源端或目标端)以及其中变量的跨度
 3. 出口参数: 规则(源端或目标端)的字符串表示
 4. 算法简介: 见注释
************************************************************************************* */
string RuleExtractor::get_words_according_to_spans(pair<int,int> span,pair<int,int> span_X1,pair<int,int> span_X2,vector<string> &words)
{
	string rule_str;
	int i = span.first;
	while (i <= span.first+span.second)
	{
		if (i>=span_X1.first && i<=span_X1.first+span_X1.second)								// 遇到第一个变量的起始位置
		{
			rule_str += "[X][X] ";
			while(i>=span_X1.first && i<=span_X1.first+span_X1.second)							// 跳过第一个变量的剩余位置
			{
				i++;
			}
		}
		else if (span_X2.first != -1 && i>=span_X2.first && i<=span_X2.first+span_X2.second)	// 遇到第二个变量的起始位置
		{
			rule_str += "[X][X] ";
			while(i>=span_X2.first && i<=span_X2.first+span_X2.second)							// 跳过第二个变量的剩余位置
			{
				i++;
			}
		}
		else
		{
			rule_str += words.at(i) + " ";														// 非变量部分
			i++;
		}
	}
	TrimLine(rule_str);
	return rule_str;
}

void RuleExtractor::extract_rules()
{
	fill_span2rules_with_AX_XA_XAX_rule();                            //形如AX,XA和XAX的规则
	fill_span2rules_with_AXX_XXA_rule();                              //形如AXX和XXA的规则
	fill_span2rules_with_AXB_AXBX_XAXB_rule();                        //形如AXB,AXBX和XAXB的规则
	fill_span2rules_with_AXBXC_rule();                                //形如AXBXC的规则
}

/**************************************************************************************
 1. 函数功能: 抽取形如AX,XA和XAX的规则
 2. 入口参数: 无
 3. 出口参数: 无
 4. 算法简介: 遍历终结符序列（即A）所有可能的span，对每种可能的pattern，先计算规则源端
 			  以及其中变量的span，然后检查这三个span是否满足句法节点和词对齐约束，满足
			  的话就抽取相应的规则
************************************************************************************* */
void RuleExtractor::fill_span2rules_with_AX_XA_XAX_rule()
{
	int src_sen_len = tspair->src_sen_len;
	for (int beg_A=0;beg_A<src_sen_len;beg_A++)
	{
		for (int len_A=0;len_A<src_sen_len-beg_A && len_A<MAX_SPAN_LEN;len_A++)
		{
			//抽取形如XA的规则
			if (beg_A != 0)
			{
				for (int len_X=0;len_X<beg_A && len_X<MAX_SPAN_LEN-len_A-1;len_X++)   //TODO 注意边界取值
				{
					int beg_X = beg_A - len_X - 1;
					pair<int,int> span = make_pair(beg_X,len_X+len_A+1);
					pair<int,int> span_X1 = make_pair(beg_X,len_X);
					pair<int,int> span_X2 = make_pair(-1,-1);
					//如果XA或X不对应句法节点或不满足对齐一致性
					if (check_alignment_and_node_constraints(span,span_X1,span_X2)==false)
						continue;
					generate_rule_according_to_src_spans(span,span_X1,span_X2);
				}
			}
			//抽取形如AX的规则
			if (beg_A+len_A != src_sen_len - 1)
			{
				for (int len_X=0;len_X<src_sen_len-beg_A-len_A-1 && len_X<MAX_SPAN_LEN-len_A-1;len_X++)   //TODO 注意边界取值
				{
					int beg_X = beg_A + len_A + 1;
					pair<int,int> span = make_pair(beg_A,len_A+len_X+1);
					pair<int,int> span_X1 = make_pair(beg_X,len_X);
					pair<int,int> span_X2 = make_pair(-1,-1);
					if (check_alignment_and_node_constraints(span,span_X1,span_X2)==false)
						continue;
					generate_rule_according_to_src_spans(span,span_X1,span_X2);
				}
			}
			//抽取形如XAX的规则
			if (beg_A != 0 && beg_A+len_A != src_sen_len - 1)
			{
				for (int len_X1=0;len_X1<beg_A && len_X1<MAX_SPAN_LEN-len_A-2;len_X1++)   //TODO 注意边界取值
				{
					for (int len_X2=0;len_X2<src_sen_len-beg_A-len_A-1 && len_X2<MAX_SPAN_LEN-len_A-len_X1-1;len_X2++)   //TODO 注意边界取值
					{
						int beg_X1 = beg_A - len_X1 - 1;
						int beg_X2 = beg_A + len_A + 1;
						pair<int,int> span = make_pair(beg_X1,len_A+len_X1+len_X2+2);
						pair<int,int> span_X1 = make_pair(beg_X1,len_X1);
						pair<int,int> span_X2 = make_pair(beg_X2,len_X2);
						if (check_alignment_and_node_constraints(span,span_X1,span_X2)==false)
							continue;
						generate_rule_according_to_src_spans(span,span_X1,span_X2);
					}
				}
			}
		}
	}
}

/**************************************************************************************
 1. 函数功能: 抽取形如AXX和XXA的规则
 2. 入口参数: 无
 3. 出口参数: 无
 4. 算法简介: 遍历终结符序列（即A）所有可能的span，对每种可能的pattern，先计算规则源端
 			  以及其中变量的span，然后检查这三个span是否满足句法节点和词对齐约束，满足
			  的话就抽取相应的规则
************************************************************************************* */
void RuleExtractor::fill_span2rules_with_AXX_XXA_rule()
{
	int src_sen_len = tspair->src_sen_len;
	for (int beg_A=0;beg_A<src_sen_len;beg_A++)
	{
		for (int len_A=0;len_A<src_sen_len-beg_A && len_A<MAX_SPAN_LEN;len_A++)
		{
			/*
			//抽取形如XXA的规则
			if (beg_A > 1)
			{
				for (int len_X1X2=1;len_X1X2<beg_A && len_X1X2<MAX_SPAN_LEN-len_A-1;len_X1X2++)   //TODO 注意边界取值
				{
					for (int len_X2=0;len_X2<len_X1X2;len_X2++)   //TODO 注意边界取值
					{
						int beg_X1 = beg_A - len_X1X2 - 1;
						int beg_X2 = beg_A - len_X2 - 1;
						pair<int,int> span = make_pair(beg_X1,len_X1X2+len_A+1);
						pair<int,int> span_X1 = make_pair(beg_X1,len_X1X2-len_X2-1);
						pair<int,int> span_X2 = make_pair(beg_X2,len_X2);
						//如果XA或X不对应句法节点或不满足对齐一致性
						if (check_alignment_and_node_constraints(span,span_X1,span_X2)==false)
							continue;
						generate_rule_according_to_src_spans(span,span_X1,span_X2);
					}
				}
			}
			*/
			//抽取形如AXX的规则
			if (beg_A+len_A < src_sen_len - 2)
			{
				for (int len_X1X2=1;len_X1X2<src_sen_len-beg_A-len_A-1 && len_X1X2<MAX_SPAN_LEN-len_A-1;len_X1X2++)   //TODO 注意边界取值
				{
					for (int len_X1=0;len_X1<len_X1X2;len_X1++)   //TODO 注意边界取值
					{
						int beg_X1 = beg_A + len_A + 1;
						int beg_X2 = beg_A + len_A + 1 + len_X1 + 1;
						pair<int,int> span = make_pair(beg_A,len_A+len_X1X2+1);
						pair<int,int> span_X1 = make_pair(beg_X1,len_X1);
						pair<int,int> span_X2 = make_pair(beg_X2,len_X1X2-len_X1-1);
						if (check_alignment_and_node_constraints(span,span_X1,span_X2)==false)
							continue;
						generate_rule_according_to_src_spans(span,span_X1,span_X2);
					}
				}
			}
		}
	}
}

/**************************************************************************************
 1. 函数功能: 抽取形如AXB,AXBX和XAXB的规则
 2. 入口参数: 无
 3. 出口参数: 无
 4. 算法简介: 遍历AXB所有可能的span，对每种可能的pattern，先计算规则源端以及其中变量的
 			  span，然后检查这三个span是否满足句法节点和词对齐约束，满足的话就抽取相应
			  的规则
************************************************************************************* */
void RuleExtractor::fill_span2rules_with_AXB_AXBX_XAXB_rule()
{
	int src_sen_len = tspair->src_sen_len;
	for (int beg_AXB=0;beg_AXB<src_sen_len;beg_AXB++)
	{
		for (int len_AXB=0;len_AXB<src_sen_len-beg_AXB && len_AXB<MAX_SPAN_LEN;len_AXB++)
		{
			for (int beg_X=beg_AXB+1;beg_X<beg_AXB+len_AXB-1;beg_X++)
			{
				for (int len_X=0;len_X<len_AXB-(beg_X-beg_AXB);len_X++)
				{
					//抽取形如XAXB的pattern
					if (beg_AXB != 0)
					{
						for (int len_X1=0;len_X1<beg_AXB && len_X1<MAX_SPAN_LEN-len_AXB-1;len_X1++)   //TODO 注意边界取值
						{
							int beg_X1 = beg_AXB - len_X1 - 1;
							pair<int,int> span = make_pair(beg_X1,len_X1+len_AXB+1);
							pair<int,int> span_X1 = make_pair(beg_X1,len_X1);
							pair<int,int> span_X2 = make_pair(beg_X,len_X);
							if (check_alignment_and_node_constraints(span,span_X1,span_X2)==false)
								continue;
							generate_rule_according_to_src_spans(span,span_X1,span_X2);
						}
					}
					//抽取形如AXBX的pattern
					if (beg_AXB+len_AXB != src_sen_len - 1)
					{
						for (int len_X2=0;len_X2<src_sen_len-beg_AXB-len_AXB-1 && len_X2<MAX_SPAN_LEN-len_AXB-1;len_X2++)   //TODO 注意边界取值
						{
							int beg_X2 = beg_AXB + len_AXB + 1;
							pair<int,int> span = make_pair(beg_AXB,len_AXB+len_X2+1);
							pair<int,int> span_X1 = make_pair(beg_X,len_X);
							pair<int,int> span_X2 = make_pair(beg_X2,len_X2);
							if (check_alignment_and_node_constraints(span,span_X1,span_X2)==false)
								continue;
							generate_rule_according_to_src_spans(span,span_X1,span_X2);
						}
					}
					//抽取形如AXB的pattern
					pair<int,int> span = make_pair(beg_AXB,len_AXB);
					pair<int,int> span_X1 = make_pair(beg_X,len_X);
					pair<int,int> span_X2 = make_pair(-1,-1);
					if (check_alignment_and_node_constraints(span,span_X1,span_X2)==false)
						continue;
					generate_rule_according_to_src_spans(span,span_X1,span_X2);
				}
			}
		}
	}
}

/**************************************************************************************
 1. 函数功能: 抽取形如AXBXC的规则
 2. 入口参数: 无
 3. 出口参数: 无
 4. 算法简介: 遍历AXBXC所有可能的span，对每种可能的pattern，先计算规则源端以及其中变量的
 			  span，然后检查这三个span是否满足句法节点和词对齐约束，满足的话就抽取相应的
			  规则
************************************************************************************* */
void RuleExtractor::fill_span2rules_with_AXBXC_rule()
{
	int src_sen_len = tspair->src_sen_len;
	for (int beg_AXBXC=0;beg_AXBXC<src_sen_len;beg_AXBXC++)
	{
		for (int len_AXBXC=0;len_AXBXC<src_sen_len-beg_AXBXC && len_AXBXC<MAX_SPAN_LEN;len_AXBXC++)
		{
			for (int beg_XBX=beg_AXBXC+1;beg_XBX<beg_AXBXC+len_AXBXC-1;beg_XBX++)
			{
				for (int len_XBX=0;len_XBX<len_AXBXC-(beg_XBX-beg_AXBXC);len_XBX++)
				{
					for (int beg_B=beg_XBX+1;beg_B<beg_XBX+len_XBX;beg_B++)
					{
						for (int len_B=0;len_B<len_XBX-(beg_B-beg_XBX);len_B++)
						{
							pair<int,int> span = make_pair(beg_AXBXC,len_AXBXC);
							pair<int,int> span_X1 = make_pair(beg_XBX,beg_B-beg_XBX-1);
							pair<int,int> span_X2 = make_pair(beg_B+len_B+1,len_XBX-len_B-(beg_B-beg_XBX-1)-2);
							if (check_alignment_and_node_constraints(span,span_X1,span_X2)==false)
								continue;
							generate_rule_according_to_src_spans(span,span_X1,span_X2);
						}
					}
				}
			}
		}
	}
}

