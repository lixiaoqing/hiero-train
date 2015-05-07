#include "str_pair.h"

StrPair::StrPair(string &line_src,string &line_tgt,string &line_align)
{
	src_words = Split(line_src);
	tgt_words = Split(line_tgt);

	src_sen_len = src_words.size();
	tgt_sen_len = tgt_words.size();

	src_idx_to_tgt_idx.resize(src_sen_len);
	src_span_to_tgt_span.resize(src_sen_len);
	src_span_to_alignment_agreement_flag.resize(src_sen_len);
	src_span_to_rules.resize(src_sen_len);
	for (int beg=0;beg<src_sen_len;beg++)
	{
		src_span_to_tgt_span.at(beg).resize(src_sen_len-beg,make_pair(-1,-1));
		src_span_to_alignment_agreement_flag.at(beg).resize(src_sen_len-beg,false);
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
}

/**************************************************************************************
 1. 函数功能: 加载词对齐
 2. 入口参数: 一句话的词对齐结果
 3. 出口参数: 无
 4. 算法简介: 根据每一对对齐的单词，更新每个源端单词对应的目标端span，以及每个目标端
 			  单词对应的源端span
************************************************************************************* */
bool StrPair::load_alignment(const string &line_align)
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
 1. 函数功能: 计算每个源端span投射到目标端的span，以及每个目标端span投射到源端的span
 2. 入口参数: 无
 3. 出口参数: 无
 4. 算法简介: 采用动态规划算法自底向上，自左向右地计算每个span的proj_span，计算公式为
 			  proj_span[beg][len] = proj_span[beg][len-1] + proj_span[beg+len][0]
************************************************************************************* */
void StrPair::cal_proj_span()
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
Span StrPair::merge_span(Span span1,Span span2)
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
void StrPair::check_alignment_agreement()
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

