#include "rule_extractor.h"

RuleExtractor::RuleExtractor(string &line_src,string &line_tgt,string &line_align)
{
	str_pair = new StrPair(line_src,line_tgt,line_align);
	flag = str_pair->flag;
}

/**************************************************************************************
 1. 函数功能: 检查规则源端的pattern是否满足词对齐约束
 2. 入口参数: 规则源端的跨度，以及其中第一个和第二个变量的跨度
 3. 出口参数: 是否满足约束
 4. 算法简介: 如果规则源端或其中的变量的跨度不满足词对齐约束，则返回false，否则返回true
************************************************************************************* */
bool RuleExtractor::check_alignment_constraint(Span span,Span span_X1,Span span_X2)
{
	if (str_pair->src_span_to_alignment_agreement_flag[span.first][span.second] == false)
		return false;
	if (span_X1.first == -1)
		return true;
	if (str_pair->src_span_to_alignment_agreement_flag[span_X1.first][span_X1.second] == false)
		return false;
	if (span_X2.first == -1)
		return true;
	if (str_pair->src_span_to_alignment_agreement_flag[span_X2.first][span_X2.second] == false)
		return false;
	return true;
}

/**************************************************************************************
 1. 函数功能: 根据规则源端以及其中变量的跨度生成规则的最终形式
 2. 入口参数: 规则源端以及其中变量的跨度
 3. 出口参数: 无
 4. 算法简介: 见注释
************************************************************************************* */
void RuleExtractor::generate_rule_according_to_src_spans(Span span,Span span_X1,Span span_X2)
{
	//如果整个span或变量span不满足对齐一致性
	if (check_alignment_constraint(span,span_X1,span_X2)==false)
		return;
	string rule_src = get_words_according_to_spans(span,span_X1,span_X2,str_pair->src_words);				// 生成规则源端的字符串表示
	if (get_word_num(rule_src) > MAX_RULE_SRC_LEN)
		return;
	Span tgt_span = str_pair->src_span_to_tgt_span[span.first][span.second];								// 获取规则目标端以及其中变量的跨度
	if (tgt_span.second + 1 > MAX_SPAN_LEN)
		return;
	Span tgt_span_X1 = make_pair(-1,-1);
	if (span_X1.first != -1)
	{
		tgt_span_X1 = str_pair->src_span_to_tgt_span[span_X1.first][span_X1.second];
	}
	Span tgt_span_X2 = make_pair(-1,-1);
	if (span_X2.first != -1)
	{
		tgt_span_X2 = str_pair->src_span_to_tgt_span[span_X2.first][span_X2.second];
	}
	vector<Span> expanded_tgt_spans = expand_tgt_span(tgt_span,make_pair(0,str_pair->tgt_sen_len-1));
	for (auto expanded_tgt_span : expanded_tgt_spans)
	{
		vector<Span> expanded_tgt_spans_X1 = expand_tgt_span(tgt_span_X1,expanded_tgt_span);
		for (auto expanded_tgt_span_X1 : expanded_tgt_spans_X1)
		{
			vector<Span> expanded_tgt_spans_X2 = expand_tgt_span(tgt_span_X2,expanded_tgt_span);
			for (auto expanded_tgt_span_X2 : expanded_tgt_spans_X2)
			{
				// 生成规则目标端的字符串表示
				string rule_tgt = get_words_according_to_spans(expanded_tgt_span,expanded_tgt_span_X1,expanded_tgt_span_X2,str_pair->tgt_words);
				if (get_word_num(rule_tgt) > MAX_RULE_TGT_LEN)
					continue;
				string alignment = get_alignment_inside_rule(span,span_X1,span_X2,expanded_tgt_span,expanded_tgt_span_X1,expanded_tgt_span_X2);
				if (alignment.size() > 0)																	// hiero规则最少有一个终结符对齐
				{
					string rule = rule_src+" [X] ||| "+rule_tgt+" [X] ||| "+alignment;
					str_pair->src_span_to_rules[span.first][span.second].push_back(rule);
				}
			}
		}
	}
}

/**************************************************************************************
 1. 函数功能: 将目标端span向两端扩展，直到遇到对齐的词
 2. 入口参数: 待扩展的目标端span
 3. 出口参数: 所有可用的目标端span
 4. 算法简介: 见注释
************************************************************************************* */
vector<Span> RuleExtractor::expand_tgt_span(Span tgt_span,Span bound)
{
	if (tgt_span.first == -1)
		return {tgt_span};
	vector<Span> expanded_spans;
	for (int tgt_beg=tgt_span.first;tgt_beg>=max(bound.first,tgt_span.first-3);tgt_beg--)		//向左扩展
	{
		if (tgt_beg<tgt_span.first && !str_pair->tgt_idx_to_src_idx[tgt_beg].empty())			//遇到对齐的词
			break;
		int tgt_span_end = tgt_span.first+tgt_span.second;
		for (int tgt_len=tgt_span_end-tgt_beg;tgt_beg+tgt_len<=min(tgt_span_end+3,bound.first+bound.second);tgt_len++)		//向右扩展
		{
			if (tgt_len>tgt_span_end-tgt_beg && !str_pair->tgt_idx_to_src_idx[tgt_beg+tgt_len].empty())						//遇到对齐的词
				break;
			expanded_spans.push_back(make_pair(tgt_beg,tgt_len));
		}
	}
	return expanded_spans;
}

/**************************************************************************************
 1. 函数功能: 根据规则(源端或目标端)以及其中变量的跨度生成规则的字符串表示
 2. 入口参数: 规则(源端或目标端)以及其中变量的跨度
 3. 出口参数: 规则(源端或目标端)的字符串表示
 4. 算法简介: 见注释
************************************************************************************* */
string RuleExtractor::get_words_according_to_spans(Span span,Span span_X1,Span span_X2,vector<string> &words)
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

string RuleExtractor::get_alignment_inside_rule(Span span,Span span_X1,Span span_X2,Span tgt_span,Span tgt_span_X1,Span tgt_span_X2)
{
	// 遍历规则目标端，获取规则目标端每个单词在句子中的位置到它在规则中的位置的映射，以及两个变量在规则目标端的位置
	map<int,int> tgt_sen_idx_to_rule_idx;
	int sen_idx = tgt_span.first;
	int rule_idx = 0;
	int rule_tgt_idx_X1 = -1;
	int rule_tgt_idx_X2 = -1;
	while (sen_idx <= tgt_span.first+tgt_span.second)
	{
		if (sen_idx>=tgt_span_X1.first && sen_idx<=tgt_span_X1.first+tgt_span_X1.second)									// 遇到第一个变量的起始位置
		{
			while(sen_idx>=tgt_span_X1.first && sen_idx<=tgt_span_X1.first+tgt_span_X1.second)								// 跳过第一个变量的剩余位置
			{
				sen_idx++;
			}
			rule_tgt_idx_X1 = rule_idx;
			rule_idx++;
		}
		else if (tgt_span_X2.first != -1 && sen_idx>=tgt_span_X2.first && sen_idx<=tgt_span_X2.first+tgt_span_X2.second)	// 遇到第二个变量的起始位置
		{
			while(sen_idx>=tgt_span_X2.first && sen_idx<=tgt_span_X2.first+tgt_span_X2.second)								// 跳过第二个变量的剩余位置
			{
				sen_idx++;
			}
			rule_tgt_idx_X2 = rule_idx;
			rule_idx++;
		}
		else
		{
			tgt_sen_idx_to_rule_idx[sen_idx] = rule_idx;
			sen_idx++;
			rule_idx++;
		}
	}

	// 遍历规则源端，先找到规则源端每个单词(或变量)在目标端句子中对应的位置，再找在规则目标端对应的位置
	vector<set<int> > alignments_for_each_token_in_rule_tgt;
	alignments_for_each_token_in_rule_tgt.resize(tgt_span.second+1);
	sen_idx = span.first;
	rule_idx = 0;
	int rule_src_idx_X1 = -1;
	int rule_src_idx_X2 = -1;
	while (sen_idx <= span.first+span.second)
	{
		if (sen_idx>=span_X1.first && sen_idx<=span_X1.first+span_X1.second)								// 遇到第一个变量的起始位置
		{
			rule_src_idx_X1 = rule_idx;
			while(sen_idx>=span_X1.first && sen_idx<=span_X1.first+span_X1.second)							// 跳过第一个变量的剩余位置
			{
				sen_idx++;
			}
			rule_idx++;
		}
		else if (sen_idx>=span_X2.first && sen_idx<=span_X2.first+span_X2.second)							// 遇到第二个变量的起始位置
		{
			rule_src_idx_X2 = rule_idx;
			while(sen_idx>=span_X2.first && sen_idx<=span_X2.first+span_X2.second)							// 跳过第二个变量的剩余位置
			{
				sen_idx++;
			}
			rule_idx++;
		}
		else
		{
			for (int tgt_sen_idx : str_pair->src_idx_to_tgt_idx.at(sen_idx))
			{
				alignments_for_each_token_in_rule_tgt.at(tgt_sen_idx_to_rule_idx[tgt_sen_idx]).insert(rule_idx);
			}
			sen_idx++;
			rule_idx++;
		}
	}

	string alignment;
	for (int i=0;i<alignments_for_each_token_in_rule_tgt.size();i++)
	{
		for (int rule_src_idx : alignments_for_each_token_in_rule_tgt.at(i))
		{
			alignment += to_string(rule_src_idx)+"-"+to_string(i)+" ";
		}
	}
	if (alignment.size() == 0)
		return "";
	if (rule_src_idx_X1 != -1)
	{
		alignment += to_string(rule_src_idx_X1)+"-"+to_string(rule_tgt_idx_X1)+" ";
	}
	if (rule_src_idx_X2 != -1)
	{
		alignment += to_string(rule_src_idx_X2)+"-"+to_string(rule_tgt_idx_X2)+" ";
	}
	TrimLine(alignment);
	return alignment;
}

void RuleExtractor::extract_rules()
{
	fill_span2rules_with_AX_XA_XAX_rule();                            //形如AX,XA和XAX的规则
	//fill_span2rules_with_AXX_XXA_rule();                              //形如AXX和XXA的规则
	fill_span2rules_with_AXB_AXBX_XAXB_rule();                        //形如AXB,AXBX和XAXB的规则
	fill_span2rules_with_AXBXC_rule();                                //形如AXBXC的规则
	fill_span2rules_with_phrase_rule();                                //短语规则
}

void RuleExtractor::dump_rules(ofstream &fs2t,ofstream &ft2s)
{
	auto &span2rules = str_pair->src_span_to_rules;
	for (int beg=0;beg<span2rules.size();beg++)
	{
		for (int span_len=0;span_len<span2rules.at(beg).size();span_len++)
		{
			for (string &rule : span2rules.at(beg).at(span_len))
			{
				auto it = rule_table.find(rule);
				if (it != rule_table.end())
				{
					it->second++;
				}
				else
				{
					rule_table[rule] = 1;
				}
			}
		}
	}
	for (auto &kvp : rule_table)
	{
		fs2t<<kvp.first<<" ||| "<<kvp.second<<" ||| "<<endl;
		vector<string> vs = Split(kvp.first," ||| ");
		ft2s<<vs[1]<<" ||| "<<vs[0]<<" ||| ";
		for (auto &idx_e2c_pair : Split(vs[2]))
		{
			int sep = idx_e2c_pair.find('-');
			ft2s<<idx_e2c_pair.substr(sep+1)<<'-'<<idx_e2c_pair.substr(0,sep)<<' ';
		}
		ft2s<<"||| "<<kvp.second<<" "<<endl;
	}
}

/**************************************************************************************
 1. 函数功能: 抽取形如AX,XA和XAX的规则
 2. 入口参数: 无
 3. 出口参数: 无
 4. 算法简介: 遍历终结符序列（即A）所有可能的span，对每种可能的pattern，先计算规则源端
 			  以及其中变量的span，然后检查这三个span是否满足句法节点和词对齐约束，满足
			  的话就抽取相应的规则
			  p.s. beg_A+len_A为A的最后一个单词的位置
************************************************************************************* */
void RuleExtractor::fill_span2rules_with_AX_XA_XAX_rule()
{
	int src_sen_len = str_pair->src_sen_len;
	for (int beg_A=0;beg_A<src_sen_len;beg_A++)
	{
		for (int len_A=0;beg_A+len_A<src_sen_len && len_A+1<MAX_SPAN_LEN;len_A++)
		{
			//抽取形如XA的规则
			if (beg_A != 0)
			{
				for (int len_X=1;len_X<beg_A && len_X+len_A+2<MAX_SPAN_LEN;len_X++)
				{
					int beg_X = beg_A - len_X - 1;
					Span span = make_pair(beg_X,len_X+len_A+1);
					Span span_X1 = make_pair(beg_X,len_X);
					Span span_X2 = make_pair(-1,-1);
					generate_rule_according_to_src_spans(span,span_X1,span_X2);
				}
			}
			//抽取形如AX的规则
			if (beg_A+len_A != src_sen_len - 1)
			{
				for (int len_X=1;beg_A+len_A+1+len_X<src_sen_len && len_A+len_X+2<MAX_SPAN_LEN;len_X++)
				{
					int beg_X = beg_A + len_A + 1;
					Span span = make_pair(beg_A,len_A+len_X+1);
					Span span_X1 = make_pair(beg_X,len_X);
					Span span_X2 = make_pair(-1,-1);
					generate_rule_according_to_src_spans(span,span_X1,span_X2);
				}
			}
			//抽取形如XAX的规则
			if (beg_A != 0 && beg_A+len_A != src_sen_len - 1)
			{
				for (int len_X1=1;len_X1<beg_A && len_X1+len_A+2<MAX_SPAN_LEN-1;len_X1++)
				{
					for (int len_X2=1;beg_A+len_A+1+len_X2<src_sen_len && len_X1+len_A+len_X2+3<MAX_SPAN_LEN;len_X2++)
					{
						int beg_X1 = beg_A - len_X1 - 1;
						int beg_X2 = beg_A + len_A + 1;
						Span span = make_pair(beg_X1,len_A+len_X1+len_X2+2);
						Span span_X1 = make_pair(beg_X1,len_X1);
						Span span_X2 = make_pair(beg_X2,len_X2);
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
	int src_sen_len = str_pair->src_sen_len;
	for (int beg_A=0;beg_A<src_sen_len;beg_A++)
	{
		for (int len_A=0;beg_A+len_A<src_sen_len && len_A<MAX_SPAN_LEN;len_A++)
		{
			/*
			//抽取形如XXA的规则
			if (beg_A > 1)
			{
				for (int len_X1X2=1;len_X1X2<beg_A && len_X1X2+len_A+2<MAX_SPAN_LEN;len_X1X2++)
				{
					for (int len_X2=1;len_X2<len_X1X2;len_X2++)
					{
						int beg_X1 = beg_A - len_X1X2 - 1;
						int beg_X2 = beg_A - len_X2 - 1;
						Span span = make_pair(beg_X1,len_X1X2+len_A+1);
						Span span_X1 = make_pair(beg_X1,len_X1X2-len_X2-1);
						Span span_X2 = make_pair(beg_X2,len_X2);
						//如果XA或X不对应句法节点或不满足对齐一致性
						generate_rule_according_to_src_spans(span,span_X1,span_X2);
					}
				}
			}
			*/
			//抽取形如AXX的规则
			if (beg_A+len_A < src_sen_len - 2)
			{
				for (int len_X1X2=1;beg_A+len_A+1+len_X1X2<src_sen_len && len_A+len_X1X2+2<MAX_SPAN_LEN;len_X1X2++)
				{
					for (int len_X1=1;len_X1<len_X1X2;len_X1++)
					{
						int beg_X1 = beg_A + len_A + 1;
						int beg_X2 = beg_A + len_A + 1 + len_X1 + 1;
						Span span = make_pair(beg_A,len_A+len_X1X2+1);
						Span span_X1 = make_pair(beg_X1,len_X1);
						Span span_X2 = make_pair(beg_X2,len_X1X2-len_X1-1);
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
	int src_sen_len = str_pair->src_sen_len;
	for (int beg_AXB=0;beg_AXB+2<src_sen_len;beg_AXB++)
	{
		for (int len_AXB=2;beg_AXB+len_AXB<src_sen_len && len_AXB<MAX_SPAN_LEN;len_AXB++)
		{
			for (int beg_X=beg_AXB+1;beg_X<beg_AXB+len_AXB;beg_X++)
			{
				for (int len_X=1;beg_X+len_X<beg_AXB+len_AXB;len_X++)
				{
					//抽取形如XAXB的pattern
					if (beg_AXB != 0)
					{
						for (int len_X1=1;len_X1<beg_AXB && len_X1+len_AXB+2<MAX_SPAN_LEN;len_X1++)
						{
							int beg_X1 = beg_AXB - len_X1 - 1;
							Span span = make_pair(beg_X1,len_X1+len_AXB+1);
							Span span_X1 = make_pair(beg_X1,len_X1);
							Span span_X2 = make_pair(beg_X,len_X);
							generate_rule_according_to_src_spans(span,span_X1,span_X2);
						}
					}
					//抽取形如AXBX的pattern
					if (beg_AXB+len_AXB != src_sen_len - 1)
					{
						for (int len_X2=1;beg_AXB+len_AXB+1+len_X2<src_sen_len && len_AXB+len_X2+2<MAX_SPAN_LEN;len_X2++)
						{
							int beg_X2 = beg_AXB + len_AXB + 1;
							Span span = make_pair(beg_AXB,len_AXB+len_X2+1);
							Span span_X1 = make_pair(beg_X,len_X);
							Span span_X2 = make_pair(beg_X2,len_X2);
							generate_rule_according_to_src_spans(span,span_X1,span_X2);
						}
					}
					//抽取形如AXB的pattern
					Span span = make_pair(beg_AXB,len_AXB);
					Span span_X1 = make_pair(beg_X,len_X);
					Span span_X2 = make_pair(-1,-1);
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
	int src_sen_len = str_pair->src_sen_len;
	for (int beg_AXBXC=0;beg_AXBXC+6<src_sen_len;beg_AXBXC++)
	{
		for (int len_AXBXC=6;beg_AXBXC+len_AXBXC<src_sen_len && len_AXBXC<MAX_SPAN_LEN;len_AXBXC++)
		{
			for (int beg_XBX=beg_AXBXC+1;beg_XBX+2<beg_AXBXC+len_AXBXC;beg_XBX++)
			{
				for (int len_XBX=4;beg_XBX+len_XBX<len_AXBXC+beg_AXBXC;len_XBX++)
				{
					for (int beg_B=beg_XBX+2;beg_B<beg_XBX+len_XBX;beg_B++)
					{
						for (int len_B=0;beg_B+len_B+1<len_XBX+beg_XBX;len_B++)
						{
							Span span = make_pair(beg_AXBXC,len_AXBXC);
							Span span_X1 = make_pair(beg_XBX,beg_B-beg_XBX-1);
							Span span_X2 = make_pair(beg_B+len_B+1,len_XBX-len_B-(beg_B-beg_XBX-1)-2);
							generate_rule_according_to_src_spans(span,span_X1,span_X2);
						}
					}
				}
			}
		}
	}
}

void RuleExtractor::fill_span2rules_with_phrase_rule()
{
	int src_sen_len = str_pair->src_sen_len;
	for (int beg_phrase=0;beg_phrase<src_sen_len;beg_phrase++)
	{
		for (int len_phrase=0;beg_phrase+len_phrase<src_sen_len && len_phrase<MAX_RULE_SRC_LEN;len_phrase++)
		{
			Span span = make_pair(beg_phrase,len_phrase);
			Span span_X1 = make_pair(-1,-1);
			Span span_X2 = make_pair(-1,-1);
			generate_rule_according_to_src_spans(span,span_X1,span_X2);
		}
	}
}
