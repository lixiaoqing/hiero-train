#ifndef STDAFX_H
#define STDAFX_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <set>
#include <vector>
#include <map>
#include <unordered_map>

#include <algorithm>
#include <bitset>
#include <queue>
#include <functional>
#include <limits>


#include <zlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <assert.h>
#include <string.h>
#include <pthread.h>
#include <omp.h>


using namespace std;

const int MAX_SPAN_LEN = 10;			// 规则源端能占据的最大跨度
const int MAX_RULE_SRC_LEN = 5;			// 规则源端最大的符号数
const int MAX_RULE_TGT_LEN = 10;		// 规则目标端最大的符号数

#endif
