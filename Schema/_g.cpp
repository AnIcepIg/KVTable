#include "_g.h"

unsigned normalize_path(char* out, unsigned count, const char* in)
{
	char ch = 0;
	unsigned it = 0, dispel = 0;
	int flag = 1;
	int mb = 0;
	ch = *in;
	while (ch != '\0' && it < count - dispel)
	{
		if (ch == '.' && *(in + 1) == '.')
		{
			unsigned cnt = 0, jt = it;
			int v = false;
			while (jt > 0)
			{
				if (out[jt - 1] == '\\') { cnt++; if (cnt == 2) { --jt;  break; } }
				else if (out[jt - 1] != '.') v = true;
				--jt;
			}
			if (v && jt == 0 && cnt == 1)
			{
				in += 2;
				ch = *in;
				dispel += it + 2;
				it = 0;
				flag = 1;
				continue;
			}
			else if (v && cnt == 2)
			{
				in += 2;
				ch = *in;
				dispel += (it - jt) - 1;
				it = jt + 1;
				continue;
			}
		}
		if (ch == '\\' || ch == '/')
		{
			if (!flag)
				out[it++] = '\\';
			else
				dispel++;
			flag = 1;
		}
		else
		{
			flag = 0;
			//out[it++] = char_tolower(ch);
			out[it++] = mb ? ch : char_tolower(ch);
			//out[it++] = ch;
		}
		if (mb) mb = 0;
		else mb = ch & 0x80;
		ch = *(++in);
	}
	out[it] = '\0';
	//_strlwr_s(out, it + 1);
	return it;
}
