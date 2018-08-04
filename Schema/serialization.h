#pragma once

struct __table_json;
struct __table;

typedef __table_json*	htabj;

htabj	tabj_from(__table* ptab);
htabj	tabj_read(const char* filename);
void	tabj_free(htabj htj);
int		tabj_save(htabj htj, const char* filename);
__table*tabj_to(htabj htj);


struct __table_bin;
typedef __table_bin*	htabbin;
struct __table_txt;
typedef __table_txt*	htabtxt;

typedef struct __table_read_data {
	void* _addr;
	unsigned _size;
}ttrd;

htabtxt tabtxt_from(__table* ptab, int pretty);
ttrd	tabtxt_read(const char* filename);		// append EOF at the end of buff
void	tabtxt_free(htabtxt htt);
void	tabtxt_free_ttrd(ttrd dt);
int		tabtxt_save(htabtxt htt, const char* filename);
__table*tabtxt_to(ttrd dt);


htabbin tabbin_from(__table* ptab);
void	tabbin_free(htabbin htb);
int		tabbin_save(htabbin htb, const char* filename);
ttrd	tabbin_read(const char* filename);
void	tabbin_free_ttrd(ttrd td);
__table*tabbin_to(ttrd td);

