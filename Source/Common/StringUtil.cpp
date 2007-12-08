#include "PocketIRC.h"
#include "StringUtil.h"

void Split(const tstring& str, std::vector<tstring>& out, TCHAR sep, bool includeEmpty)
{
	unsigned start = 0;
	unsigned end = 0;

	while(true)
	{
		if(end == str.size() || str[end] == sep)
		{
			if(end > start || includeEmpty)
			{
				out.push_back(str.substr(start, end - start));
			}

			if(end == str.size())
			{
				break;
			}

			++end;
			start = end;
		}
		else
		{
			++end;
		}
	}
}

tstring GetWord(const tstring& str, unsigned index, bool getRest)
{
	unsigned start = 0;
	unsigned end = 0;

	unsigned count = 0;

	while(true)
	{
		if(end == str.size() || str[end] == _T(' '))
		{
			if(end > start)
			{
				if(count == index)
				{
					if(getRest)
					{
						return str.substr(start);
					}
					else
					{
						return str.substr(start, end - start);
					}
				}
				++count;
			}

			if(end == str.size())
			{
				break;
			}

			++end;
			start = end;
		}
		else
		{
			++end;
		}
	}
	return _T("");
}
