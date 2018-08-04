#pragma once

typedef unsigned char	md5_byte_t;
typedef unsigned int	md5_word_t;

struct md5_state_t
{
	md5_word_t count[2];  /*!< message length in bits, lsw first */
	md5_word_t abcd[4];   /*!< digest buffer */
	md5_byte_t buf[64];   /*!< accumulate block */
};

void md5_process(md5_state_t *pms, const md5_byte_t *data /*[64]*/);
void md5_init(md5_state_t *pms);
void md5_append(md5_state_t *pms, const md5_byte_t *data, int nbytes);
void md5_finish(md5_state_t *pms, md5_byte_t digest[16]);
