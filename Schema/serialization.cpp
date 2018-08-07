#include "serialization.h"
#include "table.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/prettywriter.h"
#include "base64.h"
#include "zlib/zlib.h"

struct __table_json
{
	rapidjson::Value root;
};

#define _TABLE_ROOT_NAME	"root"
#define _FLOAT2_PREFIX		"<float2>"
#define _FLOAT3_PREFIX		"<float3>"
#define _FLOAT4_PREFIX		"<float4>"
#define _FLOAT4X4_PREFIX	"<float4x4>"
#define _USERDATA_PREFIX	"<userdata>"

void _set_float2(rapidjson::Value& val, const Schema::float2& f2, MemoryIncrement& strpool)
{
	char buff[128];
	sprintf_s(buff, _countof(buff), "%s%.6f,%.6f", _FLOAT2_PREFIX, f2.x, f2.y);
	unsigned len = (unsigned)strlen(buff);
	char* addr = (char*)strpool.alloc(len);
	if (addr)
	{
		memcpy(addr, buff, len);
		val.SetString(addr, (rapidjson::SizeType)len);
	}
}

void _set_float3(rapidjson::Value& val, const Schema::float3& f3, MemoryIncrement& strpool)
{
	char buff[128];
	sprintf_s(buff, _countof(buff), "%s%.6f,%.6f,%.6f", _FLOAT3_PREFIX, f3.x, f3.y, f3.z);
	unsigned len = (unsigned)strlen(buff);
	char* addr = (char*)strpool.alloc(len);
	if (addr)
	{
		memcpy(addr, buff, len);
		val.SetString(addr, (rapidjson::SizeType)len);
	}
}

void _set_float4(rapidjson::Value& val, const Schema::float4& f4, MemoryIncrement& strpool)
{
	char buff[128];
	sprintf_s(buff, _countof(buff), "%s%.6f,%.6f,%.6f,%.6f", _FLOAT4_PREFIX, f4.x, f4.y, f4.z, f4.w);
	unsigned len = (unsigned)strlen(buff);
	char* addr = (char*)strpool.alloc(len);
	if (addr)
	{
		memcpy(addr, buff, len);
		val.SetString(addr, (rapidjson::SizeType)len);
	}
}

void _set_float4x4(rapidjson::Value& val, const Schema::float4x4& f4x4, MemoryIncrement& strpool)
{
	char buff[512];
	sprintf_s(buff, _countof(buff)
		, "%s%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f", _FLOAT4X4_PREFIX
		, f4x4.m00, f4x4.m01, f4x4.m02, f4x4.m03
		, f4x4.m10, f4x4.m11, f4x4.m12, f4x4.m13
		, f4x4.m20, f4x4.m21, f4x4.m22, f4x4.m23
		, f4x4.m30, f4x4.m31, f4x4.m32, f4x4.m33);
	unsigned len = (unsigned)strlen(buff);
	char* addr = (char*)strpool.alloc(len);
	if (addr)
	{
		memcpy(addr, buff, len);
		val.SetString(addr, (rapidjson::SizeType)len);
	}
}

void _set_userdata(rapidjson::Value& val, void* ud, unsigned len, MemoryIncrement& strpool)
{
	unsigned size = BASE64_ENCODE_OUT_SIZE(len);
	size += (unsigned)strlen(_USERDATA_PREFIX) + 1;
	char* buff = (char*)malloc(size);
	if (buff)
	{
		memset(buff, 0, size);
		strcpy_s(buff, size, _USERDATA_PREFIX);
		base64_encode((const char*)ud, len, buff + strlen(_USERDATA_PREFIX));
		len = (unsigned)strlen(buff);
		char* addr = (char*)strpool.alloc(len);
		if (addr)
		{
			memcpy(addr, buff, len);
			val.SetString(addr, (rapidjson::SizeType)len);
		}
		free(buff);
	}
}

int _tab2val(__table* ptab, rapidjson::Value& val, rapidjson::Document& doc, MemoryIncrement& strpool)
{
	for (unsigned i = 0; i < ptab->_count; ++i)
	{
		__pair* pair = ptab->_elements[i];
		assert(pair);
		etvaltype vt = (etvaltype)pair->_cm.vt;
		rapidjson::Value key;
		key.SetString(pair->_key, cstr_len(pair->_key));
		rapidjson::Value nval;
		switch (vt)
		{
		case etvt_reserved:
		case etvt_ptr:
		case etvt_reference:
			nval.SetNull();
			break;
		case etvt_int:
			nval.SetInt(pair->_int);
			break;
		case etvt_uint:
			nval.SetUint(pair->_uint);
			break;
		case etvt_int64:
			nval.SetInt64(pair->_i64);
			break;
		case etvt_uint64:
			nval.SetUint64(pair->_u64);
			break;
		case etvt_float:
			nval.SetFloat(pair->_flt);
			break;
		case etvt_double:
			nval.SetDouble(pair->_dbl);
			break;
		case etvt_float2:
			_set_float2(nval, pair->_f2, strpool);
			break;
		case etvt_float3:
			_set_float3(nval, pair->_f3, strpool);
			break;
		case etvt_float4:
			_set_float4(nval, pair->_f4, strpool);
			break;
		case etvt_cstr:
			nval.SetString(pair->_str, cstr_len(pair->_str));
			break;
		case etvt_table:
			nval.SetObject();
			_tab2val((__table*)pair->_ptr, nval, doc, strpool);
			break;
		case etvt_string:
		{
			const char* str = (const char*)pair->_ptr;
			unsigned len = (unsigned)strlen(str);
			char* addr = (char*)strpool.alloc(len);
			if (!addr) continue;
			memcpy(addr, str, len);
			nval.SetString(addr, (rapidjson::SizeType)len);
		}
			break;
		case etvt_float4x4:
			_set_float4x4(nval, *(Schema::float4x4*)pair->_ptr, strpool);
			break;
		case etvt_userdata:
			_set_userdata(nval, pair->_userdata.ptr, pair->_userdata.size, strpool);
			break;
		default:
			break;
		}	
		val.AddMember(key, nval, doc.GetAllocator());
	}
	return true;
}

int _parse_string(__table* ptab, c_str key, const char* val)
{
	if (val[0] != '<')
	{
		c_str str = cstr_string(val);
		table_set_cstr(ptab, key, str, 0);
		return true;
	}
	static size_t _float2_length = strlen(_FLOAT2_PREFIX);
	static size_t _float3_length = strlen(_FLOAT3_PREFIX);
	static size_t _float4_length = strlen(_FLOAT4_PREFIX);
	static size_t _float4x4_length = strlen(_FLOAT4X4_PREFIX);
	static size_t _userdata_length = strlen(_USERDATA_PREFIX);
	if (strncmp(val, _FLOAT2_PREFIX, _float2_length) == 0)
	{
		const char* buff = val + _float2_length;
		Schema::float2 f2;
		sscanf_s(buff, "%f,%f", &f2.x, &f2.y);
		table_set_float2(ptab, key, (float*)f2.ptr(), 0);
	}
	else if (strncmp(val, _FLOAT3_PREFIX, _float3_length) == 0)
	{
		const char* buff = val + _float3_length;
		Schema::float3 f3;
		sscanf_s(buff, "%f,%f,%f", &f3.x, &f3.y, &f3.z);
		table_set_float3(ptab, key, (float*)f3.ptr(), 0);
	}
	else if (strncmp(val, _FLOAT4_PREFIX, _float4_length) == 0)
	{
		const char* buff = val + _float4_length;
		Schema::float4 f4;
		sscanf_s(buff, "%f,%f,%f,%f", &f4.x, &f4.y, &f4.z, &f4.w);
		table_set_float4(ptab, key, (float*)f4.ptr(), 0);
	}
	else if (strncmp(val, _FLOAT4X4_PREFIX, _float4x4_length) == 0)
	{
		const char* buff = val + _float4x4_length;
		Schema::float4x4 f4x4;
		sscanf_s(buff
			, "%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f"
			, &f4x4.m00, &f4x4.m01, &f4x4.m02, &f4x4.m03
			, &f4x4.m10, &f4x4.m11, &f4x4.m12, &f4x4.m13
			, &f4x4.m20, &f4x4.m21, &f4x4.m22, &f4x4.m23
			, &f4x4.m30, &f4x4.m31, &f4x4.m32, &f4x4.m33);
		table_set_float4x4(ptab, key, (float*)f4x4.ptr(), 0);
	}
	else if (strncmp(val, _USERDATA_PREFIX, _userdata_length) == 0)
	{
		const char* buff = val + _userdata_length;
		unsigned len = (unsigned)strlen(buff);
		unsigned size = BASE64_DECODE_OUT_SIZE(len);
		char* out = (char*)malloc(size + 1);
		if (out)
		{
			len = (unsigned)base64_decode(buff, len, out);
			if (len > 0)
			{
				table_set_userdata(ptab, key, out, len, 0);
			}
		}
	}
	else
	{
		c_str str = cstr_string(val);
		table_set_cstr(ptab, key, str, 0);
	}
	return true;
}

int _val2tab(const rapidjson::Value& val, __table* ptab)
{
	for (rapidjson::Value::ConstMemberIterator it = val.MemberBegin(); it != val.MemberEnd(); ++it)
	{
		rapidjson::Type typ = it->value.GetType();
		c_str key = cstr_string(it->name.GetString());
		if (!key || key[0] == 0) continue;
		switch (typ)
		{
		case rapidjson::kNullType:
			table_reserve(ptab, key);
			break;
		case rapidjson::kFalseType:
			table_set_integer(ptab, key, 0, 0);
			break;
		case rapidjson::kTrueType:
			table_set_integer(ptab, key, 1, 0);
			break;
		case rapidjson::kObjectType:
		{
			__table* psubtab = (__table*)table_set(ptab, key, 0);
			if (psubtab) _val2tab(it->value, psubtab);
		}
			break;
		case rapidjson::kArrayType:
			break;
		case rapidjson::kStringType:
			_parse_string(ptab, key, it->value.GetString());
			break;
		case rapidjson::kNumberType:
		{
			const rapidjson::Value& nval = it->value;
			if (nval.IsInt()) table_set_integer(ptab, key, nval.GetInt(), 0);
			else if (nval.IsUint()) table_set_uint(ptab, key, nval.GetUint(), 0);
			else if (nval.IsInt64()) table_set_int64(ptab, key, nval.GetInt64(), 0);
			else if (nval.IsUint64()) table_set_uint64(ptab, key, nval.GetUint64(), 0);
			else if (nval.IsFloat()) table_set_float(ptab, key, nval.GetFloat(), 0);
			else if (nval.IsDouble()) table_set_double(ptab, key, nval.GetDouble(), 0);
		}
			break;
		default:
			break;
		}
	}
	return true;
}

htabj tabj_from(__table* ptab)
{
	__table_json* ptabj = new(std::nothrow) __table_json();
	if (!ptabj) return 0;
	MemoryIncrement strpool;
	rapidjson::Document doc;
	rapidjson::Value& val = ptabj->root;
	val.SetObject();
	if (!_tab2val(ptab, val, doc, strpool))
	{
		delete ptabj;
		ptabj = 0;
	}
	return ptabj;
}

void tabj_free(htabj htj)
{
	__table_json* ptabj = (__table_json*)htj;
	if (ptabj)
	{
		delete ptabj;
		ptabj = nullptr;
	}
}

__table* tabj_to(htabj htj)
{
	__table_json* ptabj = (__table_json*)htj;
	if (!ptabj) return nullptr;
	rapidjson::Value& val = ptabj->root;
	__table* ptab = (__table*)table_create();
	if (!ptab) return nullptr;
	if (!_val2tab(val, ptab))
	{
		table_destroy(ptab);
		ptab = nullptr;
	}
	return ptab;
}

htabj	tabj_read(const char* filename)
{
	__table_json* ptabj = nullptr;
	Schema::hfile hf = _fopenread(filename);
	char* buff = nullptr;
	if (!hf) return 0;
	int flag = false;
	do 
	{
		unsigned len = _flength(hf);
		if (len == 0) break;
		len += 1;
		buff = (char*)malloc(len);
		if (!buff) break;
		_fread(hf, buff, len);
		buff[len] = 0;
		ptabj = new(std::nothrow) __table_json();
		if (!ptabj) break;
		try
		{
			rapidjson::StringStream ss(buff);
			rapidjson::Document doc;
			doc.ParseStream<rapidjson::kParseCommentsFlag>(ss);
			//ptabj->root = doc.GetObjectA();
			/*if (doc.FindMember(_TABLE_ROOT_NAME) == doc.MemberEnd()) break;
			ptabj->root = doc[_TABLE_ROOT_NAME];*/
		}
		catch (const std::exception&) { break; }
		flag = true;
	} while (0);
	if (!flag)
	{
		if (ptabj)
		{
			delete ptabj;
			ptabj = nullptr;
		}
	}
	if (buff) { free(buff); buff = nullptr; }
	if (hf) { _fclose(hf); hf = 0; }
	return (htabj)ptabj;
}

int		tabj_save(htabj htj, const char* filename)
{
	__table_json* ptabj = (__table_json*)htj;
	Schema::hfile hf = 0;
	int flag = false;
	do 
	{
		if (!ptabj) break;
		rapidjson::StringBuffer buff;
		try
		{
			rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buff);
			ptabj->root.Accept(writer);
		}
		catch (const std::exception&) { break; }
		const char* pbuf = buff.GetString();
		unsigned len = (unsigned)buff.GetSize();
		hf = _fopenwrite(filename);
		if (!hf) break;
		_fwrite(hf, (void*)pbuf, len);
		flag = true;
	} while (0);
	if (hf) { _fclose(hf); hf = 0; }
	return flag;
}

#define _ttObjectBegin				"{"
#define _ttObjectBeginLen			1
#define _ttObjectEnd				"}"
#define _ttObjectEndLen				1
#define _ttPrettyObjectBegin		"{\n"
#define _ttPrettyObjectBeginLen		2
#define _ttPrettyObjectEnd			"}\n"
#define _ttPrettyObjectEndLen		2
#define _ttIndent					"\t"
#define _ttIndentLen				1
#define _ttSeparator				"="
#define _ttSeparatorLen				1
#define _ttPrettySeparator			" = "
#define _ttPrettySeparatorLen		3
#define _ttStringBegin				"\""
#define _ttStringBeginLen			1
#define _ttStringEnd				"\""
#define _ttStringEndLen				1
#define _ttNull						"null"
#define _ttNullLen					4
#define _ttLineEnd					"\n"
#define _ttLineEndLen				1

#include <set>

struct __table_txt
{
	MemoryIncrement _strpool;
	std::set<void*>	_refs;
	int _pretty;
};
typedef __table_txt			tabt;

inline int _ttAppend(tabt* ptabt, const char* str, unsigned len) { return ptabt->_strpool.set(str, len) ? true : false; }
inline int _ttAppendIndent(tabt* ptabt, int layer) { for (int i = 0; i < layer; ++i) if (!ptabt->_strpool.set(_ttIndent, _ttIndentLen)) return false; return true; }

inline int _ttAddObjectBegin(tabt* ptabt, int layer)
{
	if (ptabt->_pretty)
	{
		return _ttAppend(ptabt, _ttPrettyObjectBegin, _ttPrettyObjectBeginLen);
	}
	return _ttAppend(ptabt, _ttObjectBegin, _ttObjectBeginLen);
}
inline int _ttAddObjectEnd(tabt* ptabt, int layer)
{
	if (ptabt->_pretty)
	{
		if (!_ttAppendIndent(ptabt, layer)) return false;
		return _ttAppend(ptabt, _ttPrettyObjectEnd, _ttPrettyObjectEndLen);
	}
	return _ttAppend(ptabt, _ttObjectEnd, _ttObjectEndLen);
}
inline int _ttAddKey(tabt* ptabt, const char* key, unsigned len, int layer)
{
	int rst = false;
	do {
		register int pretty = ptabt->_pretty;
		if (pretty && !_ttAppendIndent(ptabt, layer)) break;
		if (!_ttAppend(ptabt, _ttStringBegin, _ttStringBeginLen)) break;
		if (!_ttAppend(ptabt, key, len)) break;
		if (!_ttAppend(ptabt, _ttStringEnd, _ttStringEndLen)) break;
		if (pretty) { if (!_ttAppend(ptabt, _ttPrettySeparator, _ttPrettySeparatorLen)) break; }
		else { if (!_ttAppend(ptabt, _ttSeparator, _ttSeparatorLen)) break; }
		rst = true;
	} while (0);
	return rst;
}
inline int _ttAddNull(tabt* ptabt) { if (!_ttAppend(ptabt, _ttNull, _ttNullLen)) return false; if (ptabt->_pretty) return _ttAppend(ptabt, _ttLineEnd, _ttLineEndLen); return true; }
inline int _ttAddValue(tabt* ptabt, const char* buff) { if (!_ttAppend(ptabt, buff, (unsigned)strlen(buff))) return false; if (ptabt->_pretty) return _ttAppend(ptabt, _ttLineEnd, _ttLineEndLen); return true; }
inline int _ttAddPtr(tabt* ptabt, void* ptr) { char buff[64]; sprintf_s(buff, _countof(buff), "P%llx", uint64(ULONG_PTR(ptr))); return _ttAddValue(ptabt, buff); }
inline int _ttAddInt(tabt* ptabt, int n) { char buff[32]; sprintf_s(buff, _countof(buff), "I%d", n); return _ttAddValue(ptabt, buff); }
inline int _ttAddUint(tabt* ptabt, unsigned u) { char buff[32]; sprintf_s(buff, _countof(buff), "U%u", u); return _ttAddValue(ptabt, buff); }
inline int _ttAddInt64(tabt* ptabt, int64 ll) { char buff[32]; sprintf_s(buff, _countof(buff), "L%lld", ll); return _ttAddValue(ptabt, buff); }
inline int _ttAddUint64(tabt* ptabt, uint64 ull) { char buff[32]; sprintf_s(buff, _countof(buff), "UL%llu", ull); return _ttAddValue(ptabt, buff); }
inline int _ttAddFloat(tabt* ptabt, float f) { char buff[64]; sprintf_s(buff, _countof(buff), "F%f", f); return _ttAddValue(ptabt, buff); }
inline int _ttAddDouble(tabt* ptabt, double d) { char buff[128]; sprintf_s(buff, _countof(buff), "D%.12lf", d); return _ttAddValue(ptabt, buff); }
inline int _ttAddFloat2(tabt* ptabt, Schema::float2& f2) { char buff[128]; sprintf_s(buff, _countof(buff), "<float2>%f,%f", f2.x, f2.y); return _ttAddValue(ptabt, buff); }
inline int _ttAddFloat3(tabt* ptabt, Schema::float3& f3) { char buff[256]; sprintf_s(buff, _countof(buff), "<float3>%f,%f,%f", f3.x, f3.y, f3.z); return _ttAddValue(ptabt, buff); }
inline int _ttAddFloat4(tabt* ptabt, Schema::float4& f4) { char buff[256]; sprintf_s(buff, _countof(buff), "<float4>%f,%f,%f,%f", f4.x, f4.y, f4.z, f4.w); return _ttAddValue(ptabt, buff); }
inline int _ttAddFloat4x4(tabt* ptabt, Schema::float4x4& f4x4)
{
	char buff[1024];
	sprintf_s(buff, _countof(buff)
		, "<float4x4>%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f"
		, f4x4.m00, f4x4.m01, f4x4.m02, f4x4.m03
		, f4x4.m10, f4x4.m11, f4x4.m12, f4x4.m13
		, f4x4.m20, f4x4.m21, f4x4.m22, f4x4.m23
		, f4x4.m30, f4x4.m31, f4x4.m32, f4x4.m33);
	return _ttAddValue(ptabt, buff);
}
inline int _ttAddString(tabt* ptabt, const char* str, unsigned len)
{
	int rst = false;
	do {
		if (!_ttAppend(ptabt, _ttStringBegin, _ttStringBeginLen)) break;
		void* ptr = ptabt->_strpool.alloc(len);
		if (!ptr) break;
		memcpy(ptr, str, len);
		if (!_ttAppend(ptabt, _ttStringEnd, _ttStringEndLen)) break;
		if (ptabt->_pretty) { if (!_ttAppend(ptabt, _ttLineEnd, _ttLineEndLen)) break; }
		rst = true;
	} while (0);
	return rst;
}
inline int _ttAddUserdata(tabt* ptabt, void* ud, unsigned len)
{
	int rst = false;
	char* buff = nullptr;
	do {
		unsigned size = BASE64_ENCODE_OUT_SIZE(len);
		size += (unsigned)strlen(_USERDATA_PREFIX) + 1;
		buff = (char*)malloc(size);
		if (!buff) break;
		memset(buff, 0, size);
		strcpy_s(buff, size, _USERDATA_PREFIX);
		len = base64_encode((const char*)ud, len, buff + strlen(_USERDATA_PREFIX));
		len += (unsigned)strlen(_USERDATA_PREFIX);
		if (!_ttAppend(ptabt, buff, len)) break;
		if (ptabt->_pretty) { if (!_ttAppend(ptabt, _ttLineEnd, _ttLineEndLen)) break; }
		rst = true;
	} while (0);
	if (buff) free(buff);
	return true;
}

int _tab2tabt(__table* ptab, tabt* ptabt, int layer)
{
	for (unsigned i = 0; i < ptab->_count; ++i)
	{
		__pair* pair = ptab->_elements[i];
		assert(pair);
		if (!_ttAddKey(ptabt, pair->_key, cstr_len(pair->_key), layer)) return false;
		etvaltype vt = (etvaltype)pair->_cm.vt;
		switch (vt)
		{
		case etvt_reserved:
			if (!_ttAddNull(ptabt)) return false;
			break;
		case etvt_ptr:
			if (!_ttAddPtr(ptabt, pair->_ptr)) return false;
			break;
		case etvt_int:
			if (!_ttAddInt(ptabt, pair->_int)) return false;
			break;
		case etvt_uint:
			if (!_ttAddUint(ptabt, pair->_uint)) return false;
			break;
		case etvt_int64:
			if (!_ttAddInt64(ptabt, pair->_i64)) return false;
			break;
		case etvt_uint64:
			if (!_ttAddUint64(ptabt, pair->_u64)) return false;
			break;
		case etvt_float:
			if (!_ttAddFloat(ptabt, pair->_flt)) return false;
			break;
		case etvt_double:
			if (!_ttAddDouble(ptabt, pair->_dbl)) return false;
			break;
		case etvt_float2:
			if (!_ttAddFloat2(ptabt, pair->_f2)) return false;
			break;
		case etvt_float3:
			if (!_ttAddFloat3(ptabt, pair->_f3)) return false;
			break;
		case etvt_float4:
			if (!_ttAddFloat4(ptabt, pair->_f4)) return false;
			break;
		case etvt_cstr:
			if (!_ttAddString(ptabt, pair->_str, cstr_len(pair->_str))) return false;
			break;
		case etvt_reference:
			if (ptabt->_refs.find(pair->_ptr) != ptabt->_refs.end()) break;
		case etvt_table:
			ptabt->_refs.insert(pair->_ptr);
			if (!_ttAddObjectBegin(ptabt, layer)) return false;
			if (!_tab2tabt((__table*)pair->_ptr, ptabt, layer + 1)) return false;
			if (!_ttAddObjectEnd(ptabt, layer)) return false;
			break;
		case etvt_string:
		{
			const char* str = (const char*)pair->_ptr;
			unsigned len = (unsigned)strlen(str);
			if (!_ttAddString(ptabt, str, len)) return false;
		}
			break;
		case etvt_float4x4:
			if (!_ttAddFloat4x4(ptabt, *(Schema::float4x4*)pair->_ptr)) return false;
			break;
		case etvt_userdata:
			if (!_ttAddUserdata(ptabt, pair->_userdata.ptr, pair->_userdata.size)) return false;
			break;
		default:
			break;
		}
	}
	return true;
}

#include "_g.h"

#define _ttRead(v, c)		{ c = 0; ch = *buff; while (ch != v && ch != EOF) ch = *(++buff), ++c; if (ch == EOF) return nullptr; }
#define _ttTrim()			{ ch = *buff; while (char_blank(ch) && ch != EOF) ch = *(++buff);  if (ch == EOF) return nullptr; }
#define _ttIsNumeric(c)		(c >= '0' && c <= '9')
#define _ttIsNumericX(c)	(_ttIsNumeric(c) || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'))
#define _ttIsNumericI(c)	(_ttIsNumeric(c) || (c == '-'))
#define _ttIsNumericF(c)	(_ttIsNumericI(c) || (c == '.'))
#define _ttIsNumericV(c)	(_ttIsNumericF(c) || (c == ','))

inline int _ttParseString(const char* str, unsigned len, __table* ptab, c_str key)
{
	if (len < 2) return false;
	register char ch0 = str[0];
	register char ch1 = str[1];
	register char ch = 0;
	char buff[1024];
	if (len == 4 && ch0 == 'n' && ch1 == 'u' && str[2] == 'l' && str[3] == 'l')
	{
		table_reserve(ptab, key);
	}
	else if (ch0 == 'P')
	{
		unsigned cnt = 0;
		for (unsigned i = 1; i < len; ++i)
		{
			ch = str[i];
			if (_ttIsNumericX(ch)) cnt++;
			else break;
		}
		if (cnt == 0 || cnt > 16) return false;
		memcpy(buff, str + 1, cnt);
		buff[cnt] = 0;
		void* ptr = 0;
		if (1 != sscanf_s(buff, "%llx", (uint64*)&ptr)) return false;
		table_set_ptr(ptab, key, ptr, 0);
	}
	else if (ch0 == 'I')
	{
		unsigned cnt = 0;
		for (unsigned i = 1; i < len; ++i)
		{
			ch = str[i];
			if (_ttIsNumericI(ch)) cnt++;
			else break;
		}
		if (cnt == 0 || cnt > 11) return false;
		memcpy(buff, str + 1, cnt);
		buff[cnt] = 0;
		int val = 0;
		if (1 != sscanf_s(buff, "%d", &val)) return false;
		table_set_integer(ptab, key, val, 0);
	}
	else if (ch0 == 'U' && ch1 != 'L')
	{
		unsigned cnt = 0;
		for (unsigned i = 1; i < len; ++i)
		{
			ch = str[i];
			if (_ttIsNumeric(ch)) cnt++;
			else break;
		}
		if (cnt == 0 || cnt > 10) return false;
		memcpy(buff, str + 1, cnt);
		buff[cnt] = 0;
		unsigned val = 0;
		if (1 != sscanf_s(buff, "%u", &val)) return false;
		table_set_uint(ptab, key, val, 0);
	}
	else if (ch0 == 'L')
	{
		unsigned cnt = 0;
		for (unsigned i = 1; i < len; ++i)
		{
			ch = str[i];
			if (_ttIsNumericI(ch)) cnt++;
			else break;
		}
		if (cnt == 0 || cnt > 20) return false;
		memcpy(buff, str + 1, cnt);
		buff[cnt] = 0;
		int64 val = 0;
		if (1 != sscanf_s(buff, "%lld", &val)) return false;
		table_set_int64(ptab, key, val, 0);
	}
	else if (ch0 == 'U' && ch1 == 'L')
	{
		unsigned cnt = 0;
		for (unsigned i = 2; i < len; ++i)
		{
			ch = str[i];
			if (_ttIsNumeric(ch)) cnt++;
			else break;
		}
		if (cnt == 0 || cnt > 20) return false;
		memcpy(buff, str + 2, cnt);
		buff[cnt] = 0;
		uint64 val = 0;
		if (1 != sscanf_s(buff, "%llu", &val)) return false;
		table_set_uint64(ptab, key, val, 0);
	}
	else if (ch0 == 'F')
	{
		unsigned cnt = 0;
		for (unsigned i = 1; i < len; ++i)
		{
			ch = str[i];
			if (_ttIsNumericF(ch)) cnt++;
			else break;
		}
		if (cnt == 0 || cnt > 1023) return false;
		memcpy(buff, str + 1, cnt);
		buff[cnt] = 0;
		float val = 0;
		if (1 != sscanf_s(buff, "%f", &val)) return false;
		table_set_float(ptab, key, val, 0);
	}
	else if (ch0 == 'D')
	{
		unsigned cnt = 0;
		for (unsigned i = 1; i < len; ++i)
		{
			ch = str[i];
			if (_ttIsNumericF(ch)) cnt++;
			else break;
		}
		if (cnt == 0 || cnt > 1023) return false;
		memcpy(buff, str + 1, cnt);
		buff[cnt] = 0;
		double val = 0;
		if (1 != sscanf_s(buff, "%lf", &val)) return false;
		table_set_double(ptab, key, val, 0);
	}
	else if (len > 8 && ch0 == '<' && ch1 == 'f' && str[2] == 'l' && str[3] == 'o' && str[4] == 'a' && str[5] == 't')
	{
		if (str[6] == '2' && str[7] == '>')
		{
			unsigned cnt = 0;
			for (unsigned i = 8; i < len; ++i)
			{
				ch = str[i];
				if (_ttIsNumericV(ch)) cnt++;
				else break;
			}
			if (cnt == 0 || cnt > 1023) return false;
			memcpy(buff, str + 8, cnt);
			buff[cnt] = 0;
			Schema::float2 f2;
			if (2 != sscanf_s(buff, "%f,%f", &f2.x, &f2.y)) return false;
			table_set_float2(ptab, key, (float*)f2.ptr(), 0);
		}
		else if (str[6] == '3' && str[7] == '>')
		{
			unsigned cnt = 0;
			for (unsigned i = 8; i < len; ++i)
			{
				ch = str[i];
				if (_ttIsNumericV(ch)) cnt++;
				else break;
			}
			if (cnt == 0 || cnt > 1023) return false;
			memcpy(buff, str + 8, cnt);
			buff[cnt] = 0;
			Schema::float3 f3;
			if (3 != sscanf_s(buff, "%f,%f,%f", &f3.x, &f3.y, &f3.z)) return false;
			table_set_float3(ptab, key, (float*)f3.ptr(), 0);
		}
		else if (str[6] == '4' && str[7] == '>')
		{
			unsigned cnt = 0;
			for (unsigned i = 8; i < len; ++i)
			{
				ch = str[i];
				if (_ttIsNumericV(ch)) cnt++;
				else break;
			}
			if (cnt == 0 || cnt > 1023) return false;
			memcpy(buff, str + 8, cnt);
			buff[cnt] = 0;
			Schema::float4 f4;
			if (4 != sscanf_s(buff, "%f,%f,%f,%f", &f4.x, &f4.y, &f4.z, &f4.w)) return false;
			table_set_float4(ptab, key, (float*)f4.ptr(), 0);
		}
		else if (len > 10 && str[6] == '4' && str[7] == 'x' && str[8] == '4' && str[9] == '>')
		{
			unsigned cnt = 0;
			for (unsigned i = 10; i < len; ++i)
			{
				ch = str[i];
				if (_ttIsNumericV(ch)) cnt++;
				else break;
			}
			if (cnt == 0 || cnt > 1023) return false;
			memcpy(buff, str + 10, cnt);
			buff[cnt] = 0;
			Schema::float4x4 f4x4;
			if (16 != sscanf_s(buff
				, "%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f"
				, &f4x4.m00, &f4x4.m01, &f4x4.m02, &f4x4.m03
				, &f4x4.m10, &f4x4.m11, &f4x4.m12, &f4x4.m13
				, &f4x4.m20, &f4x4.m21, &f4x4.m22, &f4x4.m23
				, &f4x4.m30, &f4x4.m31, &f4x4.m32, &f4x4.m33)) return false;
			table_set_float4x4(ptab, key, (float*)f4x4.ptr(), 0);
		}
		else return false;
	}
	else if (len > 10 && strncmp(str, _USERDATA_PREFIX, strlen(_USERDATA_PREFIX)) == 0)
	{
		unsigned cnt = 0;
		for (unsigned i = 10; i < len; ++i)
		{
			ch = str[i];
			if (base64_validate(ch)) cnt++;
			else break;
		}
		if (cnt == 0) return false;
		//unsigned size = BASE64_DECODE_OUT_SIZE(cnt);
		unsigned size = cnt;
		char* ud = (char*)malloc(size);
		if (!ud) return false;
		size = base64_decode(str + 10, cnt, ud);
		if (size == 0)
		{
			free(ud);
			return false;
		}
		table_set_userdata(ptab, key, ud, size, 0);
		free(ud);
	}
	else return false;
	return true;
}

const char* _ttParse(const char* buff, __table* ptab)
{
	char ch = *buff;
	while (ch != '}' && ch != EOF)
	{
		unsigned len = 0;
		ch = *buff;
		while (ch != EOF && ch != '\"' && ch != '}') ch = *(++buff);
		if (ch == EOF) return nullptr;
		if (ch == '}') break;
		++buff;
		const char* str = buff;
		_ttRead('\"', len); ++buff;
		if (len == 0) return nullptr;
		c_str skey = cstr_stringl(str, len);
		_ttRead('=', len); ++buff;
		_ttTrim();
		if (ch == '{')
		{
			ch = *(++buff);
			__table* psubtab = (__table*)table_set(ptab, skey, 0);
			if (!psubtab) return nullptr;
			buff = _ttParse(buff, psubtab);
			if (!buff) return nullptr;
		}
		else if (ch == '\"')
		{
			ch = *(++buff);
			str = buff;
			_ttRead('\"', len); ++buff;
			ptab->set(skey, str, len, 0);
		}
		else
		{
			str = buff;
			len = 0;
			while (ch != EOF && ch != '}' && ch != '\"') ch = *(++buff), ++len;
			if (ch == EOF) return nullptr;
			if (!_ttParseString(str, len, ptab, skey)) return nullptr;
		}
	}
	if (ch == EOF) return nullptr;
	++buff;
	return buff;
}

htabtxt tabtxt_from(__table* ptab, int pretty)
{
	tabt* ptabt = new(std::nothrow) tabt();
	if (!ptabt) return 0;
	ptabt->_pretty = pretty;
	ptabt->_refs.insert(ptab);
	_ttAddObjectBegin(ptabt, 0);
	if (!_tab2tabt(ptab, ptabt, 1)) { delete ptabt; return 0; }
	_ttAddObjectEnd(ptabt, 0);
	return (htabtxt)ptabt;
}
ttrd	tabtxt_read(const char* filename)
{
	ttrd td = { 0 };
	Schema::hfile hf = _fopenread(filename);
	char* buff = nullptr;
	if (!hf) return td;
	int flag = false;
	do
	{
		unsigned len = _flength(hf);
		if (len == 0) break;
		buff = (char*)malloc(len + 1);
		if (!buff) break;
		_fread(hf, buff, len);
		buff[len] = EOF;
		td._addr = buff;
		td._size = len + 1;
		flag = true;
	} while (0);
	if (!flag)
	{
		if (buff) { free(buff); buff = 0; }
		td._addr = 0; 
		td._size = 0;
	}
	if (hf) { _fclose(hf); hf = 0; }
	return td;
}
void	tabtxt_free_ttrd(ttrd dt)
{
	if (dt._addr)
	{
		free(dt._addr);
		dt._addr = 0;
		dt._size = 0;
	}
}
void	tabtxt_free(htabtxt htt)
{
	tabt* ptabt = (tabt*)htt;
	if (ptabt) { delete ptabt; ptabt = nullptr; }
}
int		tabtxt_save(htabtxt htt, const char* filename)
{
	__table_txt* ptabt = (__table_txt*)htt;
	Schema::hfile hf = 0;
	int flag = false;
	do
	{
		if (!ptabt) break;
		hf = _fopenwrite(filename);
		if (!hf) break;
		FArray<ttrd> arr;
		for (void* ptr = ptabt->_strpool.travel(0); ptr != nullptr; ptr = ptabt->_strpool.travel(ptr))
		{
			const char* buff = ptabt->_strpool.getPtr(ptr);
			unsigned size = ptabt->_strpool.getSize(ptr);
			ttrd td = { (void*)buff, size };
			arr.Push(td);
		}
		if (arr.Count() == 0) break;
		for (unsigned i = arr.Count(); i > 0; --i)
		{
			ttrd td = arr[i - 1];
			if (td._addr)
			{
				_fwrite(hf, td._addr, td._size);
			}
		}
		flag = true;
	} while (0);
	if (hf) { _fclose(hf); hf = 0; }
	return flag;
}
__table*tabtxt_to(ttrd dt)
{
	__table* ptab = (__table*)table_create();
	if (!ptab) return nullptr;
	if (!_ttParse((const char*)dt._addr, ptab))
	{
		table_destroy(ptab);
		ptab = nullptr;
	}
	return ptab;
}

struct __table_bin
{
	MemoryIncrement _buffpool;
	std::set<void*>	_refs;
};

struct __table_bin_header
{
	fourcc fcc;
	fourcc version;
	unsigned length;
	unsigned compress_length;
};

#define _tbAppend(p, l)		if (!ptabb->_buffpool.set((void*)p, l)) return -1; size += l

unsigned _tab2tabb(__table* ptab, __table_bin* ptabb)
{
	unsigned size = 0;
	for (unsigned i = 0; i < ptab->_count; ++i)
	{
		__pair* pair = ptab->_elements[i];
		assert(pair);
		c_str key = pair->_key;
		unsigned len = cstr_len(key) + 1;
		_tbAppend(key, len);
		etvaltype vt = (etvaltype)pair->_cm.vt;
		_tbAppend(&vt, sizeof(uint8));
		switch (vt)
		{
		case etvt_reserved: break;
		case etvt_ptr: _tbAppend(&pair->_ptr, sizeof(void*)); break;
		case etvt_int: _tbAppend(&pair->_int, sizeof(int)); break;
		case etvt_uint: _tbAppend(&pair->_uint, sizeof(uint)); break;
		case etvt_int64: _tbAppend(&pair->_i64, sizeof(int64)); break;
		case etvt_uint64: _tbAppend(&pair->_u64, sizeof(uint64)); break;
		case etvt_float: _tbAppend(&pair->_flt, sizeof(float)); break;
		case etvt_double: _tbAppend(&pair->_dbl, sizeof(double)); break;
		case etvt_float2: _tbAppend(pair->_f2.ptr(), sizeof(Schema::float2)); break;
		case etvt_float3: _tbAppend(pair->_f3.ptr(), sizeof(Schema::float3)); break;
		case etvt_float4: _tbAppend(pair->_f4.ptr(), sizeof(Schema::float4)); break;
		case etvt_cstr: _tbAppend(pair->_str, cstr_len(pair->_str) + 1); break;
		case etvt_reference:
			if (ptabb->_refs.find(pair->_ptr) != ptabb->_refs.end())
			{
				unsigned val = -1;
				_tbAppend(&val, sizeof(unsigned));
				break;
			}
		case etvt_table:
		{
			unsigned* psize = (unsigned*)ptabb->_buffpool.alloc(sizeof(unsigned));
			if (!psize) return -1;
			unsigned u = _tab2tabb((__table*)pair->_ptr, ptabb);
			if (u == -1) return -1;
			*psize = u;
			size += u;
		}
			break;
		case etvt_string:
		{
			const char* str = (const char*)pair->_ptr;
			len = (unsigned)strlen(str) + 1;
			_tbAppend(str, len);
		}
			break;
		case etvt_float4x4:
		{
			Schema::float4x4* f4x4 = (Schema::float4x4*)pair->_ptr;
			_tbAppend(f4x4->ptr(), sizeof(Schema::float4x4));
		}
			break;
		case etvt_userdata: 
			_tbAppend(&pair->_userdata.size, sizeof(unsigned));
			_tbAppend(pair->_userdata.ptr, pair->_userdata.size); 
			break;
		}
	}
	return size;
}

#define _tbParseValue(t, fn)	{ t val = *(t*)(buff + offset); offset += sizeof(t); fn(ptab, skey, val, 0); }
#define _tbParseVector(t, fn)	{ float* arr = (float*)(buff + offset); offset += sizeof(t); fn(ptab, skey, arr, 0); }

int _tbParse(__table* ptab, byte* buff, unsigned len)
{
	unsigned offset = 0;
	while (offset < len)
	{
		const char* key = (const char*)(buff + offset);
		unsigned size = 0;
		while (size + offset < len && key[size]) size++;
		if (size + offset == len || size == 0) return false;
		c_str skey = cstr_string(key);
		offset += size + 1;
		size = *(uint8*)(buff + offset);
		offset += sizeof(uint8);
		etvaltype vt = (etvaltype)size;
		switch (vt)
		{
		case etvt_reserved: table_reserve(ptab, skey); break;
		case etvt_ptr: _tbParseValue(void*, table_set_ptr); break;
		case etvt_int: _tbParseValue(int, table_set_integer); break;
		case etvt_uint: _tbParseValue(uint, table_set_uint); break;
		case etvt_int64: _tbParseValue(int64, table_set_int64); break;
		case etvt_uint64: _tbParseValue(uint64, table_set_uint64); break;
		case etvt_float: _tbParseValue(float, table_set_float); break;
		case etvt_double: _tbParseValue(double, table_set_double); break;
		case etvt_float2: _tbParseVector(Schema::float2, table_set_float2); break;
		case etvt_float3: _tbParseVector(Schema::float3, table_set_float3); break;
		case etvt_float4: _tbParseVector(Schema::float4, table_set_float4); break;
		case etvt_cstr:
		{
			size = 0;
			const char* val = (const char*)(buff + offset);
			while (size + offset < len && val[size]) size++;
			if (size + offset == len) return false;
			offset += size + 1;
			c_str sval = cstr_string(val);
			table_set_cstr(ptab, skey, sval, 0);
		}
			break;
		case etvt_reference:
		case etvt_table:
		{
			size = *(unsigned*)(buff + offset);
			offset += sizeof(unsigned);
			if (size == -1) break;
			__table* psub = (__table*)table_set(ptab, skey, 0);
			if (!psub) return false;
			if (!_tbParse(psub, buff + offset, size)) return false;
			offset += size;
		}
			break;
		case etvt_string:
		{
			size = 0;
			const char* val = (const char*)(buff + offset);
			while (size + offset < len && val[size]) size++;
			if (size + offset == len) return false;
			offset += size + 1;
			table_set_string(ptab, skey, val, 0);
		}
			break;
		case etvt_float4x4: _tbParseVector(Schema::float4x4, table_set_float4x4); break;
		case etvt_userdata:
		{
			size = *(unsigned*)(buff + offset);
			offset += sizeof(unsigned);
			table_set_userdata(ptab, skey, buff + offset, size, 0);
			offset += size;
		}
			break;
		}
	}
	return true;
}

htabbin tabbin_from(__table* ptab)
{
	__table_bin* ptabb = new(std::nothrow) __table_bin();
	if (!ptabb) return 0;
	ptabb->_refs.insert(ptab);
	unsigned* psize = (unsigned*)ptabb->_buffpool.alloc(sizeof(unsigned));
	if (psize)
	{
		unsigned size = _tab2tabb(ptab, ptabb);
		if (size != -1)
		{
			*psize = size;
			return ptabb;
		}
	}
	delete ptabb;
	return 0;
}
void	tabbin_free(htabbin htb)
{
	__table_bin* ptabb = (__table_bin*)htb;
	if (ptabb)
	{
		delete ptabb;
		ptabb = nullptr;
	}
}
int		tabbin_save(htabbin htb, const char* filename)
{
	__table_bin* ptabb = (__table_bin*)htb;
	Schema::hfile hf = 0;
	byte* buff = nullptr;
	byte* cmprs = nullptr;
	int flag = false;
	do
	{
		if (!ptabb) break;
		hf = _fopenwrite(filename);
		if (!hf) break;
		unsigned len = 0;
		__table_bin_header header = { 0 };
		header.fcc = 'tabb';
		header.version = 'V000';
		FArray<ttrd> arr;
		for (void* ptr = ptabb->_buffpool.travel(0); ptr != nullptr; ptr = ptabb->_buffpool.travel(ptr))
		{
			const char* buff = ptabb->_buffpool.getPtr(ptr);
			unsigned size = ptabb->_buffpool.getSize(ptr);
			len += size;
			ttrd td = { (void*)buff, size };
			arr.Push(td);
		}
		header.length = len;
		buff = (byte*)malloc(len);
		cmprs = (byte*)malloc(len);
		if (!buff || !cmprs) break;
		if (arr.Count() == 0) break;
		len = 0;
		for (unsigned i = arr.Count(); i > 0; --i)
		{
			ttrd td = arr[i - 1];
			memcpy(buff + len, td._addr, td._size);
			len += td._size;
		}
		if (Z_OK != compress((Bytef*)cmprs, (uLongf*)&len, (Bytef*)buff, len)) break;
		header.compress_length = len;
		_fwrite(hf, &header, sizeof(__table_bin_header));
		_fwrite(hf, cmprs, len);
		flag = true;
	} while (0);
	if (cmprs) { free(cmprs); cmprs = nullptr; }
	if (buff) { free(buff); buff = nullptr; }
	if (hf) { _fclose(hf); hf = 0; }
	return flag;
}
ttrd	tabbin_read(const char* filename)
{
	ttrd td = { 0 };
	Schema::hfile hf = _fopenread(filename);
	char* buff = nullptr;
	char* unbuff = nullptr;
	if (!hf) return td;
	int flag = false;
	do
	{
		unsigned len = _flength(hf);
		if (len == 0 || len < sizeof(__table_bin_header)) break;
		buff = (char*)malloc(len);
		if (!buff) break;
		_fread(hf, buff, len);
		__table_bin_header* pheader = (__table_bin_header*)buff;
		if (pheader->compress_length != len - sizeof(__table_bin_header)) break;
		if (pheader->length < pheader->compress_length) break;
		if (pheader->fcc != 'tabb') break;
		unbuff = (char*)malloc(pheader->length);
		if (!unbuff) break;
		unsigned dstlen = pheader->length;
		unsigned srclen = pheader->compress_length;
		if (Z_OK != uncompress((Bytef*)unbuff, (uLongf*)&dstlen, (Bytef*)(buff + sizeof(__table_bin_header)), (uLong)srclen)) break;
		td._addr = unbuff;
		td._size = dstlen;
		flag = true;
	} while (0);
	if (!flag)
	{
		if (unbuff) { free(unbuff); unbuff = 0; }
		td._addr = 0;
		td._size = 0;
	}
	if (buff) { free(buff); buff = 0; }
	if (hf) { _fclose(hf); hf = 0; }
	return td;
}
void	tabbin_free_ttrd(ttrd td)
{
	if (td._addr)
	{
		free(td._addr);
		td._addr = 0;
		td._size = 0;
	}
}
__table*tabbin_to(ttrd td)
{
	if (!td._addr) return nullptr;
	__table* ptab = (__table*)table_create();
	if (!ptab) return nullptr;
	unsigned len = *(unsigned*)td._addr;
	if (len > td._size) { table_destroy(ptab); return nullptr; }
	if (!_tbParse(ptab, (byte*)td._addr + sizeof(unsigned), len))
	{
		table_destroy(ptab);
		return nullptr;
	}
	return ptab;
}

int tab_peek_type(const char* filename)
{
	etfiletype ftyp = etft_unknown;
	Schema::hfile hf = _fopenread(filename);
	if (hf)
	{
		unsigned len = _flength(hf);
		if (len > 0)
		{
			ftyp = etft_text;
			if (len > sizeof(__table_bin_header))
			{
				__table_bin_header header = { 0 };
				_fread(hf, &header, sizeof(__table_bin_header));
				if (header.fcc == 'tabb') ftyp = etft_binary;
			}
		}
		_fclose(hf);
	}
	return (int)ftyp;
}
