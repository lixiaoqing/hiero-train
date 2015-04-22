#include "myutils.h"

vector<string> Split(const string &s)
{
	vector <string> vs;
	stringstream ss(s);
	string e;
	while(ss >> e)
		vs.push_back(e);
	return vs;
}

vector<string> Split(const string &s, const string &sep)
{
	vector <string> vs;
	int cur = 0,next;
	next = s.find(sep);
	while(next != string::npos)
	{
		if(s.substr(cur,next-cur) !="")
			vs.push_back(s.substr(cur,next-cur));
		cur = next+sep.size();
		next = s.find(sep,cur);
	}
	vs.push_back(s.substr(cur));
	return vs;
}

string Join(const vector<string> &vs)
{
	string out;
	for (auto &s : vs)
	{
		out += s + " ";
	}
	TrimLine(out);
	return out;
}

void TrimLine(string &line)
{
	line.erase(0,line.find_first_not_of(" \t\r\n"));
	line.erase(line.find_last_not_of(" \t\r\n")+1);
}

void print_vector(vector<int> &v)
{
	for (auto e : v)
		cout<<e<<" ";
	cout<<endl;
}

int get_word_num(string s)
{
	int n = 1;
	for (char c : s)
	{
		if (c == ' ')
		{
			n++;
		}
	}
	return n;
}
