#pragma once

#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <algorithm>
#include <assert.h>
#include "mathsse.h"

typedef unsigned char		uint8;
typedef unsigned short		uint16;
typedef unsigned int		uint;
typedef unsigned int		uint32;
typedef unsigned __int64	uint64;
typedef char				bit8;
typedef unsigned char		boolean;
typedef unsigned int		fourcc;

typedef char				int8;
typedef short				int16;
typedef int					int32;
typedef __int64				int64;

typedef unsigned char		uchar;
typedef unsigned char		byte;
typedef wchar_t				wchar;


#ifdef _MSC_VER
#include <windows.h>
#define spin_init(lock)		InterlockedExchange(&lock, 0)
#define spin_lock(lock)		while (1 == InterlockedCompareExchange(&lock, 1, 0))
#define spin_unlock(lock)	InterlockedExchange(&lock, 0);
#define spin_increment(v)	InterlockedIncrement(&v)
#define spin_decrement(v)	InterlockedDecrement(&v)

class Mutex
{
public:
	Mutex() { InitializeCriticalSection(&_section); }
	~Mutex() { DeleteCriticalSection(&_section); }
	void lock() { EnterCriticalSection(&_section); }
	void unlock() { LeaveCriticalSection(&_section); }

private:
	CRITICAL_SECTION _section;
};

class Spin
{
	volatile _declspec (align(64)) long _occupied;
	volatile _declspec (align(64)) long _lock;
public:
	Spin() { spin_init(_lock); }
	~Spin() {}

	inline void lock() { spin_lock(_lock); }
	inline void unlock() { spin_unlock(_lock); }
	inline void Lock() { spin_lock(_lock); }
	inline void Unlock() { spin_unlock(_lock); }
};

#ifndef SPINMUTEX
#define SPINMUTEX 1
class SpinMutex
{
public:
	inline SpinMutex() { InitializeCriticalSectionAndSpinCount(&_section, 2000); }
	inline ~SpinMutex() { DeleteCriticalSection(&_section); }

	inline void lock() { EnterCriticalSection(&_section); }
	inline void unlock() { LeaveCriticalSection(&_section); }

	inline void Lock() { EnterCriticalSection(&_section); }
	inline void Unlock() { LeaveCriticalSection(&_section); }

private:
	CRITICAL_SECTION _section;
};

#ifndef THREADNAME_INFO
#pragma pack(push,8)  
typedef struct tagTHREADNAME_INFO
{
	DWORD dwType; // Must be 0x1000.  
	LPCSTR pszName; // Pointer to name (in user addr space).  
	DWORD dwThreadID; // Thread ID (-1=caller thread).  
	DWORD dwFlags; // Reserved for future use, must be zero.  
} THREADNAME_INFO;
#pragma pack(pop)  
#endif

inline void set_thread_name(char const* name)
{
	const DWORD MS_VC_EXCEPTION = 0x406D1388;

	THREADNAME_INFO info;
	info.dwType = 0x1000;
	info.pszName = name;
	info.dwThreadID = -1;
	info.dwFlags = 0;
#pragma warning(push)  
#pragma warning(disable: 6320 6322)  
	__try {
		RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR*)&info);
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
	}
#pragma warning(pop)  
}

#endif

#elif defined(__GNUC__)
#include <sched.h>
#define spin_init(lock)
#define spin_lock(lock)
#define spin_unlock(lock)
#define spin_increment(v)
#define spin_decrement(v)

#include <pthread.h>

class Mutex
{
public:
	Mutex() { pthread_mutex_init(&_mutex, 0); }
	~Mutex() { pthread_mutex_destroy(&_mutex); }
	void lock() { pthread_mutex_lock(&_mutex); }
	void unlock() { pthread_mutex_unlock(&_mutex); }

private:
	pthread_mutex_t _mutex;
};

inline void set_thread_name(char const* name)
{
	pthread_setname_np(pthread_self(), name);
}

#else
#error Unknown OS
#endif

#ifndef FLT_EPSILON
#define FLT_EPSILON			1.192092896e-07F
#endif

#ifndef PAGE_SIZE
#define PAGE_SIZE		0x1000
#endif

#define C_FLT(l, r)			(fabs(l - r) < FLT_EPSILON)

namespace Schema
{
	struct FVECTOR2
	{
		union {
			struct { float x, y; };
			float m[2];
		};

		inline FVECTOR2() : x(0), y(0) {}
		inline FVECTOR2(const float x_, const float y_) : x(x_), y(y_) {}
		inline FVECTOR2(const FVECTOR2& rhs) : x(rhs.x), y(rhs.y) {}
		inline FVECTOR2(const float* rhs) : x(rhs[0]), y(rhs[1]) {}

		inline FVECTOR2&	operator=(const FVECTOR2& rhs) { x = rhs.x; y = rhs.y; return *this; }
		inline FVECTOR2		operator-() const { return FVECTOR2(-x, -y); }
		inline bool			operator==(const FVECTOR2& rhs) const { return C_FLT(x, rhs.x) && C_FLT(y, rhs.y); }
		inline bool			operator!=(const FVECTOR2& rhs) { return !C_FLT(x, rhs.x) || !C_FLT(y, rhs.y); }
		inline FVECTOR2		operator+(const FVECTOR2& rhs) const { return FVECTOR2(x + rhs.x, y + rhs.y); }
		inline FVECTOR2		operator-(const FVECTOR2& rhs) const { return FVECTOR2(x - rhs.x, y - rhs.y); }
		inline FVECTOR2		operator*(const FVECTOR2& rhs) const { return FVECTOR2(x * rhs.x, y * rhs.y); }
		inline FVECTOR2		operator*(const float rhs) const { return FVECTOR2(x * rhs, y * rhs); }
		inline FVECTOR2		operator/(const FVECTOR2& rhs) const { return FVECTOR2(x / rhs.x, y / rhs.y); }
		inline FVECTOR2		operator/(const float rhs) const { return FVECTOR2(x / rhs, y / rhs); }
		inline FVECTOR2&	operator+=(const FVECTOR2& rhs) { x += rhs.x; y += rhs.y; return *this; }
		inline FVECTOR2&	operator-=(const FVECTOR2& rhs) { x -= rhs.x; y -= rhs.y; return *this; }
		inline FVECTOR2&	operator*=(const float rhs) { x *= rhs; y *= rhs; return *this; }
		inline FVECTOR2&	operator/=(const float rhs) { x /= rhs; y /= rhs; return *this; }
		inline float&		operator[](const int idx) { return m[idx]; }
		inline const float& operator[](const int idx) const { return m[idx]; }

		inline float	dot(const FVECTOR2& rhs) const { return x * rhs.x + y * rhs.y; }
		inline float	cross(const FVECTOR2& rhs) const { return x * rhs.y - y * rhs.x; }
		inline void		inverse() { x = 1.f / x; y = 1.f / y; }
		inline void		sqrt() { x = std::sqrt(x); y = std::sqrt(y); }
		inline float	length() const { return std::sqrt(x * x + y * y); }
		inline float	len() const { return std::sqrt(x * x + y * y); }
		inline float	lenSqr() const { return x * x + y * y; }
		inline void		normalize() { float l = len(); if (l > FLT_EPSILON) { x /= l; y /= l; } }
		inline FVECTOR2&abs() { x = std::fabs(x); y = std::fabs(y); return *this; }

		inline void*		ptr() { return m; }
		inline const void*	ptr() const { return m; }

		static inline float Dot(const FVECTOR2& lhs, const FVECTOR2& rhs) { return lhs.x * rhs.x + lhs.y * rhs.y; }
		static inline float CCW(const FVECTOR2& lhs, const FVECTOR2& rhs) { return lhs.x * rhs.y - lhs.y * rhs.x; }
	};

	struct FVECTOR3
	{
		union {
			struct { float x, y, z; };
			float m[3];
		};

		inline FVECTOR3() : x(0), y(0), z(0) {}
		inline FVECTOR3(const float x_, const float y_, const float z_) : x(x_), y(y_), z(z_) {}
		inline FVECTOR3(const FVECTOR3& rhs) : x(rhs.x), y(rhs.y), z(rhs.z) {}
		inline FVECTOR3(const float* rhs) : x(rhs[0]), y(rhs[1]), z(rhs[2]) {}
		inline FVECTOR3(const float rhs) : x(rhs), y(rhs), z(rhs) {}

		inline FVECTOR3&	operator=(const FVECTOR3& rhs) { x = rhs.x; y = rhs.y; z = rhs.z; return *this; }
		inline FVECTOR3		operator-() const { return FVECTOR3(-x, -y, -z); }
		inline bool			operator==(const FVECTOR3& rhs) const { return C_FLT(x, rhs.x) && C_FLT(y, rhs.y) && C_FLT(z, rhs.z); }
		inline bool			operator!=(const FVECTOR3& rhs) const { return !C_FLT(x, rhs.x) || !C_FLT(y, rhs.y) || !C_FLT(z, rhs.z); }
		inline FVECTOR3		operator+(const FVECTOR3& rhs) const { return FVECTOR3(x + rhs.x, y + rhs.y, z + rhs.z); }
		inline FVECTOR3		operator-(const FVECTOR3& rhs) const { return FVECTOR3(x - rhs.x, y - rhs.y, z - rhs.z); }
		inline FVECTOR3		operator*(const FVECTOR3& rhs) const { return FVECTOR3(x * rhs.x, y * rhs.y, z * rhs.z); }
		inline FVECTOR3		operator*(const float rhs) const { return FVECTOR3(x * rhs, y * rhs, z * rhs); }
		inline FVECTOR3		operator/(const FVECTOR3& rhs) const { return FVECTOR3(x / rhs.x, y / rhs.y, z / rhs.z); }
		inline FVECTOR3		operator/(const float rhs) const { return FVECTOR3(x / rhs, y / rhs, z / rhs); }
		inline FVECTOR3&	operator+=(const FVECTOR3& rhs) { x += rhs.x; y += rhs.y; z += rhs.z; return *this; }
		inline FVECTOR3&	operator-=(const FVECTOR3& rhs) { x -= rhs.x; y -= rhs.y; z -= rhs.z; return *this; }
		inline FVECTOR3&	operator*=(const float rhs) { x *= rhs; y *= rhs; z *= rhs; return *this; }
		inline FVECTOR3&	operator/=(const float rhs) { x /= rhs; y /= rhs; z /= rhs; return *this; }
		inline float&		operator[](const int idx) { return m[idx]; }
		inline const float& operator[](const int idx) const { return m[idx]; }

		inline friend FVECTOR3	operator+(float lhs, const FVECTOR3& rhs) { return FVECTOR3(lhs + rhs.x, lhs + rhs.y, lhs + rhs.z); }
		inline friend FVECTOR3	operator-(float lhs, const FVECTOR3& rhs) { return FVECTOR3(lhs - rhs.x, lhs - rhs.y, lhs - rhs.z); }
		inline friend FVECTOR3	operator*(float lhs, const FVECTOR3& rhs) { return FVECTOR3(lhs * rhs.x, lhs * rhs.y, lhs * rhs.z); }
		inline friend FVECTOR3	operator/(float lhs, const FVECTOR3& rhs) { return FVECTOR3(lhs / rhs.x, lhs / rhs.y, lhs / rhs.z); }

		inline float	dot(const FVECTOR3& rhs) const { return x * rhs.x + y * rhs.y + z * rhs.z; }
		inline float	absdot(const FVECTOR3& rhs) const { return std::fabs(x * rhs.x) + std::fabs(y * rhs.y) + std::fabs(z * rhs.z); }
		inline FVECTOR3	cross(const FVECTOR3& rhs) const { return FVECTOR3(y * rhs.z - z * rhs.y, z * rhs.x - x * rhs.z, x * rhs.y - y * rhs.x); }
		inline FVECTOR3 clamp(const FVECTOR3& mini, const FVECTOR3& maxi) const
		{
			FVECTOR3 r;
			r.x = (x > maxi.x) ? maxi.x : x;
			r.x = (r.x < mini.x) ? mini.x : r.x;
			r.y = (y > maxi.y) ? maxi.y : y;
			r.y = (r.y < mini.y) ? mini.y : r.y;
			r.z = (z > maxi.z) ? maxi.z : z;
			r.z = (r.z < mini.z) ? mini.z : r.z;
			return r;
		}
		inline float	absangle(const FVECTOR3& rhs) const
		{
			float v = dot(rhs);
			float s = len() * rhs.len();
			if (s != 0)
			{
				v /= s;
				return std::acos(v);
			}
			return 0;
		}
		inline float	angle(const FVECTOR3& rhs) const
		{
			float a = absangle(rhs);
			float d = x * rhs.y - y * rhs.x;
			if (d < 0) return -a;
			return a;
		}

		inline void		setToZero() { x = y = z = 0.0f; }
		inline void		inverse() { x = 1.f / x; y = 1.f / y; z = 1.f / z; }
		inline void		sqrt() { x = std::sqrt(x); y = std::sqrt(y); z = std::sqrt(z); }
		inline float	length() const { return std::sqrt(x * x + y * y + z * z); }
		inline float	len() const { return std::sqrt(x * x + y * y + z * z); }
		inline float	lenSqr() const { return x * x + y * y + z * z; }
		inline FVECTOR3&normalize() { float l = len(); if (l > FLT_EPSILON) { x /= l; y /= l; z /= l; } return *this; }
		inline void		ceil(const FVECTOR3& rhs) { if (rhs.x > x) x = rhs.x; if (rhs.y > y) y = rhs.y; if (rhs.z > z) z = rhs.z; }
		inline void		floor(const FVECTOR3& rhs) { if (rhs.x < x) x = rhs.x; if (rhs.y < y) y = rhs.y; if (rhs.z < z) z = rhs.z; }
		inline FVECTOR3&abs() { x = std::fabs(x); y = std::fabs(y); z = std::fabs(z); return *this; }
		inline void		minimum(const FVECTOR3& rhs) { if (x > rhs.x) x = rhs.x; if (y > rhs.y) y = rhs.y; if (z > rhs.z) z = rhs.z; }
		inline void		maximum(const FVECTOR3& rhs) { if (x < rhs.x) x = rhs.x; if (y < rhs.y) y = rhs.y; if (z < rhs.z) z = rhs.z; }

		inline void*		ptr() { return m; }
		inline const void*	ptr() const { return m; }

		static inline float		Dot(const FVECTOR3& lhs, const FVECTOR3& rhs) { return (lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z); }
		static inline FVECTOR3	Cross(const FVECTOR3& lhs, const FVECTOR3& rhs)
		{
			FVECTOR3 r;
			r.x = lhs.y * rhs.z - lhs.z * rhs.y;
			r.y = lhs.z * rhs.x - lhs.x * rhs.z;
			r.z = lhs.x * rhs.y - lhs.y * rhs.x;
			return r;
		}

		static inline FVECTOR3&	Cross(FVECTOR3& out, const FVECTOR3& lhs, const FVECTOR3& rhs)
		{
			out.x = lhs.y * rhs.z - lhs.z * rhs.y;
			out.y = lhs.z * rhs.x - lhs.x * rhs.z;
			out.z = lhs.x * rhs.y - lhs.y * rhs.x;
			return out;
		}

		static inline FVECTOR3	Clamp(const FVECTOR3& v, const FVECTOR3& mini, const FVECTOR3& maxi)
		{
			FVECTOR3 r;
			r.x = (v.x > maxi.x) ? maxi.x : v.x;
			r.x = (r.x < mini.x) ? mini.x : r.x;
			r.y = (v.y > maxi.y) ? maxi.y : v.y;
			r.y = (r.y < mini.y) ? mini.y : r.y;
			r.z = (v.z > maxi.z) ? maxi.z : v.z;
			r.z = (r.z < mini.z) ? mini.z : r.z;
			return r;
		}
		static inline FVECTOR3 Minimum(const FVECTOR3& lhs, const FVECTOR3& rhs) { return FVECTOR3(lhs.x > rhs.x ? rhs.x : lhs.x, lhs.y > rhs.y ? rhs.y : lhs.y, lhs.z > rhs.z ? rhs.z : lhs.z); }
		static inline FVECTOR3 Maximum(const FVECTOR3& lhs, const FVECTOR3& rhs) { return FVECTOR3(lhs.x < rhs.x ? rhs.x : lhs.x, lhs.y < rhs.y ? rhs.y : lhs.y, lhs.z < rhs.z ? rhs.z : lhs.z); }

		inline void lerp(const FVECTOR3& vFrom, const FVECTOR3& vTo, float t)
		{
			x = vFrom.x + ((vTo.x - vFrom.x) * t);
			y = vFrom.y + ((vTo.y - vFrom.y) * t);
			z = vFrom.z + ((vTo.z - vFrom.z) * t);
		}

		inline void lerp(const FVECTOR3& vTo, float t)
		{
			x = x + ((vTo.x - x) * t);
			y = y + ((vTo.y - y) * t);
			z = z + ((vTo.z - z) * t);
		}
	};

	struct FVECTOR4
	{
		union {
			struct { float x, y, z, w; };
			float m[4];
		};

		inline FVECTOR4() : x(0), y(0), z(0), w(0) {}
		inline FVECTOR4(const float x_, const float y_, const float z_, const float w_) : x(x_), y(y_), z(z_), w(w_) {}
		inline FVECTOR4(const FVECTOR4& rhs) : x(rhs.x), y(rhs.y), z(rhs.z), w(rhs.w) {}
		inline FVECTOR4(const float* rhs) : x(rhs[0]), y(rhs[1]), z(rhs[2]), w(rhs[3]) {}

		inline FVECTOR4&	operator=(const FVECTOR4& rhs) { x = rhs.x; y = rhs.y; z = rhs.z; w = rhs.w; return *this; }
		inline FVECTOR4		operator-() const { return FVECTOR4(-x, -y, -z, -w); }
		inline bool			operator==(const FVECTOR4& rhs) { return C_FLT(x, rhs.x) && C_FLT(y, rhs.y) && C_FLT(z, rhs.z) && C_FLT(w, rhs.w); }
		inline bool			operator!=(const FVECTOR4& rhs) { return !C_FLT(x, rhs.x) || !C_FLT(y, rhs.y) || !C_FLT(z, rhs.z) || !C_FLT(w, rhs.w); }
		inline FVECTOR4		operator+(const FVECTOR4& rhs) const { return FVECTOR4(x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w); }
		inline FVECTOR4		operator-(const FVECTOR4& rhs) const { return FVECTOR4(x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w); }
		inline FVECTOR4		operator*(const FVECTOR4& rhs) const { return FVECTOR4(x * rhs.x, y * rhs.y, z * rhs.z, w * rhs.w); }
		inline FVECTOR4		operator*(const float rhs) const { return FVECTOR4(x * rhs, y * rhs, z * rhs, w * rhs); }
		inline FVECTOR4		operator/(const FVECTOR4& rhs) const { return FVECTOR4(x / rhs.x, y / rhs.y, z / rhs.z, w / rhs.w); }
		inline FVECTOR4		operator/(const float rhs) const { return FVECTOR4(x / rhs, y / rhs, z / rhs, w / rhs); }
		inline FVECTOR4&	operator+=(const FVECTOR4& rhs) { x += rhs.x; y += rhs.y; z += rhs.z; w += rhs.w; return *this; }
		inline FVECTOR4&	operator-=(const FVECTOR4& rhs) { x -= rhs.x; y -= rhs.y; z -= rhs.z; w -= rhs.w; return *this; }
		inline FVECTOR4&	operator*=(const float rhs) { x *= rhs; y *= rhs; z *= rhs; w *= rhs; return *this; }
		inline FVECTOR4&	operator/=(const float rhs) { x /= rhs; y /= rhs; z /= rhs; w /= rhs; return *this; }
		inline float&		operator[](const int idx) { return m[idx]; }
		inline const float& operator[](const int idx) const { return m[idx]; }

		inline void*		ptr() { return m; }
		inline const void*	ptr() const { return m; }
		inline FVECTOR4&	abs() { x = std::fabs(x); y = std::fabs(y); z = std::fabs(z); w = fabs(w); return *this; }
	};

	typedef FVECTOR2	Vector2;
	typedef FVECTOR3	Vector3;
	typedef FVECTOR4	Vector4;

#define QUATERNION_SLERP_LERP_ERROR_LIMIT 0.05f

	struct FQUATERNION
	{
		union {
			struct { float x, y, z, w; };
			float m[4];
		};

		inline FQUATERNION() : x(0), y(0), z(0), w(1) {}
		inline FQUATERNION(const FQUATERNION& rhs) : x(rhs.x), y(rhs.y), z(rhs.z), w(rhs.w) {}
		inline FQUATERNION(const Vector4& rhs) : x(rhs.x), y(rhs.y), z(rhs.z), w(rhs.w) {}
		inline FQUATERNION(const float x_, const float y_, const float z_, const float w_) : x(x_), y(y_), z(z_), w(w_) {}
		inline FQUATERNION(const float* rhs) : x(rhs[0]), y(rhs[1]), z(rhs[2]), w(rhs[3]) {}
		inline FQUATERNION(const FVECTOR3& rhs, const float radian) { rotateAxis(rhs, radian); }
		inline FQUATERNION(const FVECTOR3& tanHalfAngleVec) { fromTanHalfAngleRotationVector(tanHalfAngleVec); }

		inline float&		operator[](const int idx) { return m[idx]; }
		inline const float& operator[](const int idx) const { return m[idx]; }
		inline FQUATERNION& operator=(const FQUATERNION& rhs) { x = rhs.x; y = rhs.y; z = rhs.z; w = rhs.w; return *this; }
		inline FQUATERNION& operator+=(const FQUATERNION& rhs) { x += rhs.x; y += rhs.y; z += rhs.z; w += rhs.w; return *this; }
		inline FQUATERNION& operator-=(const FQUATERNION& rhs) { x -= rhs.x; y -= rhs.y; z -= rhs.z; w -= rhs.w; return *this; }
		inline FQUATERNION& operator*=(const FQUATERNION& rhs)
		{
			x = w * rhs.x + x * rhs.w + z * rhs.y - y * rhs.z;
			y = w * rhs.y + y * rhs.w + x * rhs.z - z * rhs.x;
			z = w * rhs.z + z * rhs.w + y * rhs.x - x * rhs.y;
			w = w * rhs.w - x * rhs.x - y * rhs.y - z * rhs.z;
			return *this;
		}
		inline FQUATERNION& operator*=(const float rhs) { x *= rhs; y *= rhs; z *= rhs; w *= rhs; return *this; }
		inline FQUATERNION& operator/=(const float rhs) { float fct = 1.0f / rhs; x *= fct; y *= fct; z *= fct; w *= fct; return *this; }
		inline bool			operator==(const FQUATERNION& rhs) const { return C_FLT(x, rhs.x) && C_FLT(y, rhs.y) && C_FLT(z, rhs.z) && C_FLT(w, rhs.w); }
		inline bool			operator!=(const FQUATERNION& rhs) const { return !C_FLT(x, rhs.x) || !C_FLT(y, rhs.y) || !C_FLT(z, rhs.z) || !C_FLT(w, rhs.w); }
		inline FQUATERNION	operator+(const FQUATERNION& rhs) const { return FQUATERNION(x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w); }
		inline FQUATERNION	operator-(const FQUATERNION& rhs) const { return FQUATERNION(x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w); }
		inline FQUATERNION	operator*(const FQUATERNION& rhs) const
		{
			FQUATERNION q;
			q.x = w * rhs.x + x * rhs.w + y * rhs.z - z * rhs.y;
			q.y = w * rhs.y + y * rhs.w + z * rhs.x - x * rhs.z;
			q.z = w * rhs.z + z * rhs.w + x * rhs.y - y * rhs.x;
			q.w = w * rhs.w - x * rhs.x - y * rhs.y - z * rhs.z;
			return q;
		}
		inline FVECTOR3		operator*(const FVECTOR3& rhs) const
		{
			Vector3 uv, uuv;
			Vector3 qvec(x, y, z);
			uv = qvec.cross(rhs);
			uuv = qvec.cross(uv);
			uv *= 2.0f * w;
			uuv *= 2.0f;
			return rhs + uv + uuv;
		}
		inline FQUATERNION	operator*(float scalar) const { return FQUATERNION(x * scalar, y * scalar, z * scalar, w * scalar); }

		inline FQUATERNION	operator/(const float& rhs) const { float fct = 1.f / rhs; return FQUATERNION(x * fct, y * fct, z * fct, w * fct); }
		inline FQUATERNION  operator~() const { return FQUATERNION(-x, -y, -z, w); }
		inline FQUATERNION  operator-() const { return FQUATERNION(-x, -y, -z, -w); }

		inline void*		ptr() { return m; }
		inline const void*	ptr() const { return m; }

		inline void rotateAxis(const FVECTOR3& axis, const float radian)
		{
			float alpha = 0.5f * radian;
			float sina = std::sin(alpha);
			x = sina * axis.x;
			y = sina * axis.y;
			z = sina * axis.z;
			w = std::cos(alpha);
		}
		inline float		radian() const { return std::acos(w) * 2.0f; }
		inline float		dot(const FQUATERNION& rhs) const { return (w * rhs.w + x * rhs.x + y * rhs.y + z * rhs.z); }
		inline FQUATERNION	cross(const FQUATERNION& rhs) const
		{
			FQUATERNION q;
			q.x = w * rhs.x + x * rhs.w + y * rhs.z - z * rhs.y;
			q.y = w * rhs.y + y * rhs.w + z * rhs.x - x * rhs.z;
			q.z = w * rhs.z + z * rhs.w + x * rhs.y - y * rhs.x;
			q.w = w * rhs.w - x * rhs.x - y * rhs.y - z * rhs.z;
			return q;
		}

		inline float lenSqr() const { return (x * x + y * y + z * z + w * w); }
		inline float length() const { return sqrtf(lenSqr()); }
		inline void identity() { x = y = z = 0; w = 1.0f; }
		inline void invert()
		{
			float lengthSq = lenSqr();
			if (lengthSq > FLT_EPSILON)
			{
				lengthSq = 1.0f / lengthSq;

				x = -x * lengthSq;
				y = -y * lengthSq;
				z = -z * lengthSq;
				w = w * lengthSq;
			}
		}

		inline void normalize()
		{
			float ls = lenSqr();
			if (ls < FLT_EPSILON)
			{
				identity();
				return;
			}

			float invLength = 1.0f / sqrtf(ls);
			x *= invLength;
			y *= invLength;
			z *= invLength;
			w *= invLength;
		}

		inline void	fromEuler(float pitch, float yaw, float roll)
		{
			float fcr = std::cos(roll / 2.f);
			float fsr = std::sin(roll / 2.f);
			float fcp = std::cos(pitch / 2.f);
			float fsp = std::sin(pitch / 2.f);
			float fcy = std::cos(yaw / 2.f);
			float fsy = std::sin(yaw / 2.f);
			x = fcr * fsp * fcy + fsr * fcp * fsy;
			y = fcr * fcp * fsy - fsr * fsp * fcy;
			z = fsr * fcp * fcy - fcr * fsp * fsy;
			w = fcr * fcp * fcy + fsr * fsp * fsy;
		}

		inline void fromTanHalfAngleRotationVector(const Vector3& thaVec)
		{
			*this = *(FQUATERNION*)(&thaVec);
			w = 1.0f;
			normalize();
		}

		inline void toEuler(float& pitch, float& yaw, float& roll) const
		{
			roll = std::atan2(2.0f * (w * z + x * y), 1.0f - 2.0f * (z * z + x * x));
			pitch = std::asin(2.0f * (w * x - y * z));
			yaw = std::atan2(2.0f * (w * y + z * x), 1.0f - 2.0f * (x * x + y * y));
		}

		inline void fastSlerp(const FQUATERNION& qFrom, const FQUATERNION& qTo, float t, float fromDotTo)
		{
			assert(fromDotTo >= 0); // Quats must be in the same semi-arc - just negate one if they're not
			assert(0.f <= t && t <= 1.f);

			float recipOnePlusFromdotTo;
			float c1, c3, c5, c7;
			float startWeight, endWeight;
			float T, t2, T2;

			recipOnePlusFromdotTo = 1.0f / (1.0f + fromDotTo);
			c1 = 1.570994357000f + (0.56429298590f + (-0.17836577170f + 0.043199493520f * fromDotTo) * fromDotTo) * fromDotTo;
			c3 = -0.646139638200f + (0.59456579360f + (0.08610323953f - 0.034651229280f * fromDotTo) * fromDotTo) * fromDotTo;
			c5 = 0.079498235210f + (-0.17304369310f + (0.10792795990f - 0.014393978010f * fromDotTo) * fromDotTo) * fromDotTo;
			c7 = -0.004354102836f + (0.01418962736f + (-0.01567189691f + 0.005848706227f * fromDotTo) * fromDotTo) * fromDotTo;

			T = 1 - t;
			t2 = t * t;
			T2 = T * T;
			startWeight = (c1 + (c3 + (c5 + c7 * T2) * T2) * T2) * T * recipOnePlusFromdotTo;
			endWeight = (c1 + (c3 + (c5 + c7 * t2) * t2) * t2) * t * recipOnePlusFromdotTo;

			x = startWeight * qFrom.x + endWeight * qTo.x;
			y = startWeight * qFrom.y + endWeight * qTo.y;
			z = startWeight * qFrom.z + endWeight * qTo.z;
			w = startWeight * qFrom.w + endWeight * qTo.w;
		}

		inline void fastSlerp(const FQUATERNION& qTo, float t, float fromDotTo)
		{
			assert(fromDotTo >= 0); // Quats must be in the same semi-arc - just negate one if they're not
			assert(0.f <= t && t <= 1.f);

			float recipOnePlusFromdotTo;
			float c1, c3, c5, c7;
			float startWeight, endWeight;
			float T, t2, T2;

			recipOnePlusFromdotTo = 1.0f / (1.0f + fromDotTo);
			c1 = 1.570994357000f + (0.56429298590f + (-0.17836577170f + 0.043199493520f * fromDotTo) * fromDotTo) * fromDotTo;
			c3 = -0.646139638200f + (0.59456579360f + (0.08610323953f - 0.034651229280f * fromDotTo) * fromDotTo) * fromDotTo;
			c5 = 0.079498235210f + (-0.17304369310f + (0.10792795990f - 0.014393978010f * fromDotTo) * fromDotTo) * fromDotTo;
			c7 = -0.004354102836f + (0.01418962736f + (-0.01567189691f + 0.005848706227f * fromDotTo) * fromDotTo) * fromDotTo;

			T = 1 - t;
			t2 = t * t;
			T2 = T * T;
			startWeight = (c1 + (c3 + (c5 + c7 * T2) * T2) * T2) * T * recipOnePlusFromdotTo;
			endWeight = (c1 + (c3 + (c5 + c7 * t2) * t2) * t2) * t * recipOnePlusFromdotTo;

			x = startWeight * x + endWeight * qTo.x;
			y = startWeight * y + endWeight * qTo.y;
			z = startWeight * z + endWeight * qTo.z;
			w = startWeight * w + endWeight * qTo.w;
		}
		inline void slerp(const FQUATERNION& qFrom, const FQUATERNION& qTo, float t)
		{
			assert(0.f <= t && t <= 1.f);
			slerp(qFrom, qTo, t, qFrom.dot(qTo));
		}

		//----------------------------------------------------------------------------------------------------------------------
		inline void slerp(const FQUATERNION& qFrom, const FQUATERNION& qTo, float t, float fromDotTo)
		{
			assert(0.f <= t && t <= 1.f);

			float angle, s, startWeight, endWeight;

			// Correct for input quats in different semi-arcs.
			float sgn = fromDotTo >= 0 ? 1.0f : -1.0f;
			FQUATERNION toQ = qTo * sgn;
			fromDotTo *= sgn;

			if ((1.0f - fabs(fromDotTo)) > QUATERNION_SLERP_LERP_ERROR_LIMIT)
			{
				// Source quaternions are different enough to perform a slerp.
				// Also fastArccos becomes inaccurate as c tends towards 1.
				angle = acosf(fromDotTo);
				s = sinf(angle);
				startWeight = sinf((1.0f - t) * angle) / s;
				endWeight = sinf(t * angle) / s;
			}
			else
			{
				// Source quaternions are almost the same and slerping becomes inaccurate so do a lerp instead.
				startWeight = 1.0f - t;
				endWeight = t;
			}

			x = startWeight * qFrom.x + endWeight * toQ.x;
			y = startWeight * qFrom.y + endWeight * toQ.y;
			z = startWeight * qFrom.z + endWeight * toQ.z;
			w = startWeight * qFrom.w + endWeight * toQ.w;

			// Normalising is necessary as fastArccos is not accurate enough and because the lerp path does not produce
			// normal results when sources are too different.
			normalize();
		}

		//----------------------------------------------------------------------------------------------------------------------
		inline void slerp(const FQUATERNION& qTo, float t)
		{
			assert(0.f <= t && t <= 1.f);
			slerp(qTo, t, dot(qTo));
		}

		//----------------------------------------------------------------------------------------------------------------------
		inline void slerp(const FQUATERNION& qTo, float t, float fromDotTo)
		{
			assert(0.f <= t && t <= 1.f);

			float angle, s, startWeight, endWeight;

			// Correct for input quats in different semi-arcs.
			float sgn = fromDotTo >= 0 ? 1.0f : -1.0f;
			FQUATERNION toQ = qTo * sgn;
			fromDotTo *= sgn;

			if ((1.0f - fabs(fromDotTo)) > QUATERNION_SLERP_LERP_ERROR_LIMIT)
			{
				// Source quaternions are different enough to perform a slerp.
				// Also fastArccos becomes inaccurate as c tends towards 1.
				angle = acosf(fromDotTo);
				s = sinf(angle);
				startWeight = sinf((1.0f - t) * angle) / s;
				endWeight = sinf(t * angle) / s;
			}
			else
			{
				// Source quaternions are almost the same and slerping becomes inaccurate so do a lerp instead.
				startWeight = 1.0f - t;
				endWeight = t;
			}

			x = startWeight * x + endWeight * toQ.x;
			y = startWeight * y + endWeight * toQ.y;
			z = startWeight * z + endWeight * toQ.z;
			w = startWeight * w + endWeight * toQ.w;

			// Normalising is necessary as fastArccos is not accurate enough and because the lerp path does not produce
			// normal results when sources are too different.
			normalize();
		}

		inline FVECTOR3 rotate(const FVECTOR3& v) const
		{
			FVECTOR3 result;
			FVECTOR3 qv(x, y, z);

			result = qv.cross(v);
			result *= (w * 2.0f);

			result += (v * (2.0f * (w * w) - 1.0f));
			result += (qv * (qv.dot(v) * 2.0f));

			return result;
		}

		inline void rotate(const FVECTOR3& vIn, FVECTOR3& vOut) const
		{
#if 1
			Vector3 qv(x, y, z);

			vOut = qv.cross(vIn);
			vOut *= (w * 2.0f);

			vOut += (vIn * (2.0f * (w * w) - 1.0f));
			vOut += (qv * (qv.dot(vIn) * 2.0f));
#else 
			Vector3 qv(x, y, z);
			Vector3 vY = qv.cross(vIn);
			Vector3 vUp = vY * 2 * w;
			Vector3 vZ = qv.cross(vY);
			Vector3 vFront = 2 * vZ;
			vOut = vIn + vFront + vUp;
#endif
		}

		inline FVECTOR3 inverseRotate(const FVECTOR3& v) const
		{
			FVECTOR3 result;
			FVECTOR3 qv(x, y, z);

			result = qv.cross(v);
			result *= (-w * 2.0f);

			result += (v * (2.0f * (w * w) - 1.0f));
			result += (qv * (qv.dot(v) * 2.0f));

			return result;
		}

		inline void inverseRotate(const FVECTOR3& vIn, FVECTOR3& vOut) const
		{
			Vector3 qv(x, y, z);

			vOut = qv.cross(vIn);
			vOut *= (-w * 2.0f);

			vOut += (vIn * (2.0f * (w * w) - 1.0f));
			vOut += (qv * (qv.dot(vIn) * 2.0f));
		}

		// Convert the quaternion to a quaternion log vector
		inline FVECTOR3 log() const
		{
			float sinHalfAngle = sqrtf(x * x + y * y + z * z);
			if (sinHalfAngle < FLT_EPSILON)
			{
				return FVECTOR3(0, 0, 0);
			}
			else
			{
				float fac = atan2(sinHalfAngle, w) / sinHalfAngle;
				return FVECTOR3(fac * x, fac * y, fac * z);
			}
		}

		// Set the quaternion from a quaternion log vector
		inline void exp(const FVECTOR3& v)
		{
			float halfAngle = v.length();
			if (halfAngle < FLT_EPSILON)
			{
				identity();
			}
			else
			{
				float fac = sinf(halfAngle) / halfAngle;
				w = cosf(halfAngle);
				x = fac * v.x;
				y = fac * v.y;
				z = fac * v.z;
			}
		}

		static FQUATERNION& rotateAxis(FQUATERNION& out, const FVECTOR3& axis, const float radian)
		{
			FVECTOR3 raxis = axis;
			raxis.normalize();
			out.rotateAxis(raxis, radian);
			return out;
		}

		static FQUATERNION& rotationYawPitchRoll(FQUATERNION& out, float yaw, float pitch, float roll)
		{
			out.fromEuler(pitch, yaw, roll);
			return out;
		}
	};

	typedef FQUATERNION		Quaternion;

	struct FMATRIX3
	{
		union
		{
			struct { float m00, m01, m02; float m10, m11, m12; float m20, m21, m22; };
			float m[9];
		};

		inline FMATRIX3() { memset(m, 0, sizeof(float) * 9); }
		inline FMATRIX3(float f00, float f01, float f02
			, float f10, float f11, float f12
			, float f20, float f21, float f22)
			: m00(f00), m01(f01), m02(f02)
			, m10(f10), m11(f11), m12(f12)
			, m20(f20), m21(f21), m22(f22) {}
		inline FMATRIX3(const float* rhs) { memcpy(m, rhs, sizeof(float) * 9); }
		inline FMATRIX3(const FMATRIX3& rhs) { memcpy(m, rhs.m, sizeof(float) * 9); }

		inline FVECTOR3 row(int r) const { return *(FVECTOR3*)(m + r * 3); }
		inline FVECTOR3 col(int c) const { return FVECTOR3(m[c], m[c + 3], m[c + 6]); }

		inline FVECTOR3		operator[](int r) const { return row(r); }
		inline FMATRIX3&	operator=(const FMATRIX3& rhs) { memcpy(m, rhs.m, sizeof(float) * 9); return *this; }
		inline FMATRIX3&	operator+=(const float rhs)
		{
			m00 += rhs; m01 += rhs; m02 += rhs;
			m10 += rhs; m11 += rhs; m12 += rhs;
			m20 += rhs; m21 += rhs; m22 += rhs;
			return *this;
		}
		inline FMATRIX3&	operator+=(const FMATRIX3& rhs)
		{
			m00 += rhs.m00; m01 += rhs.m01; m02 += rhs.m02;
			m10 += rhs.m10; m11 += rhs.m11; m12 += rhs.m12;
			m20 += rhs.m20; m21 += rhs.m21; m22 += rhs.m22;
			return *this;
		}
		inline FMATRIX3&	operator-=(const float rhs)
		{
			m00 -= rhs; m01 -= rhs; m02 -= rhs;
			m10 -= rhs; m11 -= rhs; m12 -= rhs;
			m20 -= rhs; m21 -= rhs; m22 -= rhs;
			return *this;
		}
		inline FMATRIX3&	operator-=(const FMATRIX3& rhs)
		{
			m00 -= rhs.m00; m01 -= rhs.m01; m02 -= rhs.m02;
			m10 -= rhs.m10; m11 -= rhs.m11; m12 -= rhs.m12;
			m20 -= rhs.m20; m21 -= rhs.m21; m22 -= rhs.m22;
			return *this;
		}
		inline FMATRIX3&	operator*=(const float rhs)
		{
			m00 *= rhs; m01 *= rhs; m02 *= rhs;
			m10 *= rhs; m11 *= rhs; m12 *= rhs;
			m20 *= rhs; m21 *= rhs; m22 *= rhs;
			return *this;
		}
		inline FMATRIX3&	operator*=(const FMATRIX3& rhs)
		{
			FMATRIX3 r;
			r.m00 = m00 * rhs.m00 + m01 * rhs.m10 + m02 * rhs.m20;
			r.m01 = m00 * rhs.m01 + m01 * rhs.m11 + m02 * rhs.m21;
			r.m02 = m00 * rhs.m02 + m01 * rhs.m12 + m02 * rhs.m22;
			r.m10 = m10 * rhs.m00 + m11 * rhs.m10 + m12 * rhs.m20;
			r.m11 = m10 * rhs.m01 + m11 * rhs.m11 + m12 * rhs.m21;
			r.m12 = m10 * rhs.m02 + m11 * rhs.m12 + m12 * rhs.m22;
			r.m20 = m20 * rhs.m00 + m21 * rhs.m10 + m22 * rhs.m20;
			r.m21 = m20 * rhs.m01 + m21 * rhs.m11 + m22 * rhs.m21;
			r.m22 = m20 * rhs.m02 + m21 * rhs.m12 + m22 * rhs.m22;
			*this = r;
			return *this;
		}
		inline FMATRIX3&	operator/=(const float rhs)
		{
			m00 /= rhs; m01 /= rhs; m02 /= rhs;
			m10 /= rhs; m11 /= rhs; m12 /= rhs;
			m20 /= rhs; m21 /= rhs; m22 /= rhs;
			return *this;
		}
		inline bool			operator==(const FMATRIX3& rhs) const
		{
			return C_FLT(m00, rhs.m00) && C_FLT(m01, rhs.m01) && C_FLT(m02, rhs.m02)
				&& C_FLT(m10, rhs.m10) && C_FLT(m11, rhs.m11) && C_FLT(m12, rhs.m12)
				&& C_FLT(m20, rhs.m20) && C_FLT(m21, rhs.m21) && C_FLT(m22, rhs.m22);
		}
		inline bool			operator!=(const FMATRIX3& rhs) const
		{
			return !C_FLT(m00, rhs.m00) || !C_FLT(m01, rhs.m01) || !C_FLT(m02, rhs.m02)
				|| !C_FLT(m10, rhs.m10) || !C_FLT(m11, rhs.m11) || !C_FLT(m12, rhs.m12)
				|| !C_FLT(m20, rhs.m20) || !C_FLT(m21, rhs.m21) || !C_FLT(m22, rhs.m22);
		}
		inline FMATRIX3		operator+(const FMATRIX3& rhs) const
		{
			FMATRIX3 r;
			r.m00 = m00 + rhs.m00; r.m01 = m01 + rhs.m01; r.m02 = m02 + rhs.m02;
			r.m10 = m10 + rhs.m10; r.m11 = m11 + rhs.m11; r.m12 = m12 + rhs.m12;
			r.m20 = m20 + rhs.m20; r.m21 = m21 + rhs.m21; r.m22 = m22 + rhs.m22;
			return r;
		}
		inline FMATRIX3		operator-(const FMATRIX3& rhs) const
		{
			FMATRIX3 r;
			r.m00 = m00 - rhs.m00; r.m01 = m01 - rhs.m01; r.m02 = m02 - rhs.m02;
			r.m10 = m10 - rhs.m10; r.m11 = m11 - rhs.m11; r.m12 = m12 - rhs.m12;
			r.m20 = m20 - rhs.m20; r.m21 = m21 - rhs.m21; r.m22 = m22 - rhs.m22;
			return r;
		}
		inline FMATRIX3		operator*(const FMATRIX3& rhs) const
		{
			FMATRIX3 r;
			r.m00 = m00 * rhs.m00 + m01 * rhs.m10 + m02 * rhs.m20;
			r.m01 = m00 * rhs.m01 + m01 * rhs.m11 + m02 * rhs.m21;
			r.m02 = m00 * rhs.m02 + m01 * rhs.m12 + m02 * rhs.m22;
			r.m10 = m10 * rhs.m00 + m11 * rhs.m10 + m12 * rhs.m20;
			r.m11 = m10 * rhs.m01 + m11 * rhs.m11 + m12 * rhs.m21;
			r.m12 = m10 * rhs.m02 + m11 * rhs.m12 + m12 * rhs.m22;
			r.m20 = m20 * rhs.m00 + m21 * rhs.m10 + m22 * rhs.m20;
			r.m21 = m20 * rhs.m01 + m21 * rhs.m11 + m22 * rhs.m21;
			r.m22 = m20 * rhs.m02 + m21 * rhs.m12 + m22 * rhs.m22;
			return r;
		}
		inline FMATRIX3		operator*(const float rhs) const
		{
			FMATRIX3 r;
			r.m00 = m00 * rhs; r.m01 = m01 * rhs; r.m02 = m02 * rhs;
			r.m10 = m10 * rhs; r.m11 = m11 * rhs; r.m12 = m12 * rhs;
			r.m20 = m20 * rhs; r.m21 = m21 * rhs; r.m22 = m22 * rhs;
			return r;
		}
		inline FMATRIX3		operator/(const float rhs) const
		{
			FMATRIX3 r;
			r.m00 = m00 / rhs; r.m01 = m01 / rhs; r.m02 = m02 / rhs;
			r.m10 = m10 / rhs; r.m11 = m11 / rhs; r.m12 = m12 / rhs;
			r.m20 = m20 / rhs; r.m21 = m21 / rhs; r.m22 = m22 / rhs;
			return r;
		}

		inline void*		ptr() { return m; }
		inline const void*	ptr() const { return m; }

		inline void		rotateAxis(const FVECTOR3& axis, float radian)
		{
			float x = axis.x, y = axis.y, z = axis.z;
			float fsin = std::sin(radian);
			float fcos = std::cos(radian);
			FMATRIX3 local, final;
			local.m00 = (x * x) * (1.0f - fcos) + fcos;
			local.m01 = (x * y) * (1.0f - fcos) + (z * fsin);
			local.m02 = (x * z) * (1.0f - fcos) - (y * fsin);
			local.m10 = (y * x) * (1.0f - fcos) - (z * fsin);
			local.m11 = (y * y) * (1.0f - fcos) + fcos;
			local.m12 = (y * z) * (1.0f - fcos) + (x * fsin);
			local.m20 = (z * x) * (1.0f - fcos) + (y * fsin);
			local.m21 = (z * y) * (1.0f - fcos) - (x * fsin);
			local.m22 = (z * z) * (1.0f - fcos) + fcos;
			final = local * (*this);
			*this = final;
		}
		inline void		rotateAxisX(const float radian)
		{
			float fsin = std::sin(radian);
			float fcos = std::cos(radian);
			float temp10 = m10 * fcos + m20 * fsin;
			float temp11 = m11 * fcos + m21 * fsin;
			float temp12 = m12 * fcos + m22 * fsin;
			float temp20 = m10 * -fsin + m20 * fcos;
			float temp21 = m11 * -fsin + m21 * fcos;
			float temp22 = m12 * -fsin + m22 * fcos;
			m10 = temp10; m11 = temp11; m12 = temp12;
			m20 = temp20; m21 = temp21; m22 = temp22;
		}
		inline void		rotateAxisY(const float radian)
		{
			float fsin = std::sin(radian);
			float fcos = std::cos(radian);
			float temp00 = m00 * fcos - m20 * fsin;
			float temp01 = m01 * fcos - m21 * fsin;
			float temp02 = m02 * fcos - m22 * fsin;
			float temp20 = m00 * fsin + m20 * fcos;
			float temp21 = m01 * fsin + m21 * fcos;
			float temp22 = m02 * fsin + m22 * fcos;
			m00 = temp00; m01 = temp01; m02 = temp02;
			m20 = temp20; m21 = temp21; m22 = temp22;
		}
		inline void		rotateAxisZ(const float radian)
		{
			float fsin = std::sin(radian);
			float fcos = std::cos(radian);
			float temp00 = m00 * fcos + m10 * fsin;
			float temp01 = m01 * fcos + m11 * fsin;
			float temp02 = m02 * fcos + m12 * fsin;
			float temp10 = m00 * -fsin + m10 * fcos;
			float temp11 = m01 * -fsin + m11 * fcos;
			float temp12 = m02 * -fsin + m12 * fcos;
			m00 = temp00; m01 = temp01; m02 = temp02;
			m10 = temp10; m11 = temp11; m12 = temp12;
		}
		inline void		rotateAxis(FMATRIX3& r, const FVECTOR3& axis, const float radian)
		{
			float fsin = std::sin(radian);
			float fcos = std::cos(radian);
			float fminus = 1.0f - fcos;
			FVECTOR3 vsin = axis * fsin;
			FVECTOR3 vmul = axis * fminus;
			r.m00 = axis.x * vmul.x + fcos;
			r.m01 = axis.x * vmul.y + vsin.z;
			r.m02 = axis.x * vmul.z + -vsin.y;
			r.m10 = axis.y * vmul.x + -vsin.z;
			r.m11 = axis.y * vmul.y + fcos;
			r.m12 = axis.y * vmul.z + vsin.x;
			r.m20 = axis.z * vmul.x + vsin.y;
			r.m21 = axis.z * vmul.y + -vsin.x;
			r.m22 = axis.z * vmul.z + fcos;
		}
		inline FVECTOR3	rotate(const FVECTOR3& vec)
		{
			return FVECTOR3(vec.x * m00 + vec.y * m10 + vec.z * m20
				, vec.x * m01 + vec.y * m11 + vec.z * m21
				, vec.x * m02 + vec.y * m12 + vec.z * m22);
		}
		inline FMATRIX3&transpose() { std::swap(m01, m10); std::swap(m02, m20); std::swap(m12, m21); return *this; }
		inline void		transpose(FMATRIX3& r, const FMATRIX3& mat) { r = mat; r.transpose(); }

		inline friend FVECTOR3 operator*(const FVECTOR3& lhs, const FMATRIX3& rhs)
		{
			FVECTOR3 r;
			r.x = lhs.x * rhs.m00 + lhs.y * rhs.m10 + lhs.z * rhs.m20;
			r.y = lhs.x * rhs.m01 + lhs.y * rhs.m11 + lhs.z * rhs.m21;
			r.z = lhs.x * rhs.m02 + lhs.y * rhs.m12 + lhs.z * rhs.m22;
			return r;
		}
	};

	struct FMATRIX4
	{
		_CRT_ALIGN(16) union
		{
			struct { float m00, m01, m02, m03; float m10, m11, m12, m13; float m20, m21, m22, m23; float m30, m31, m32, m33; };
			float m[16];
		};
		inline FMATRIX4() { memset(m, 0, sizeof(float) * 16); }
		inline FMATRIX4(float f00, float f01, float f02, float f03
			, float f10, float f11, float f12, float f13
			, float f20, float f21, float f22, float f23
			, float f30, float f31, float f32, float f33)
			: m00(f00), m01(f01), m02(f02), m03(f03)
			, m10(f10), m11(f11), m12(f12), m13(f13)
			, m20(f20), m21(f21), m22(f22), m23(f23)
			, m30(f30), m31(f31), m32(f32), m33(f33) {}
		inline FMATRIX4(const float* rhs) { memcpy(m, rhs, sizeof(float) * 16); }
		inline FMATRIX4(const FMATRIX4& rhs) { memcpy(m, rhs.m, sizeof(float) * 16); }
		inline FMATRIX4(const FQUATERNION& quat, const FVECTOR3& trans)
		{
			const float q0t2 = quat.w * 2;
			const float q1t2 = quat.x * 2;

			const float q0sq = quat.w * quat.w;
			const float q1sq = quat.x * quat.x;
			const float q2sq = quat.y * quat.y;
			const float q3sq = quat.z * quat.z;

			const float q0q1 = q0t2 * quat.x;
			const float q0q2 = q0t2 * quat.y;
			const float q0q3 = q0t2 * quat.z;

			const float q1q2 = q1t2 * quat.y;
			const float q1q3 = q1t2 * quat.z;
			const float q2q3 = quat.y * quat.z * 2;

			m00 = q0sq + q1sq - q2sq - q3sq;	m01 = q1q2 + q0q3;					m02 = -q0q2 + q1q3;					m03 = 0;
			m10 = q1q2 - q0q3;					m11 = q0sq - q1sq + q2sq - q3sq;	m12 = q0q1 + q2q3;					m13 = 0;
			m20 = q0q2 + q1q3;					m21 = -q0q1 + q2q3;					m22 = q0sq - q1sq - q2sq + q3sq;	m23 = 0;
			m30 = trans.x;						m31 = trans.y;						m32 = trans.z;						m33 = 1.0f;
		}

		inline FVECTOR4 row(int r) const { return *(FVECTOR4*)(m + r * 4); }
		inline FVECTOR4 col(int c) const { return FVECTOR4(m[c], m[c + 4], m[c + 8], m[c + 12]); }

		inline FVECTOR3& translation() { return *(FVECTOR3*)(m + 12); }
		inline const FVECTOR3& translation() const { return *(FVECTOR3*)(m + 12); }

		inline FVECTOR4		operator[](int r) const { return row(r); }
		inline FMATRIX4&	operator=(const FMATRIX4& rhs) { memcpy(m, rhs.m, sizeof(float) * 16); return *this; }
		inline FMATRIX4&	operator+=(const float rhs)
		{
			m00 += rhs; m01 += rhs; m02 += rhs; m03 += rhs;
			m10 += rhs; m11 += rhs; m12 += rhs; m13 += rhs;
			m20 += rhs; m21 += rhs; m22 += rhs; m23 += rhs;
			m30 += rhs; m31 += rhs; m32 += rhs; m33 += rhs;
			return *this;
		}
		inline FMATRIX4&	operator+=(const FMATRIX4& rhs)
		{
			m00 += rhs.m00; m01 += rhs.m01; m02 += rhs.m02; m03 += rhs.m03;
			m10 += rhs.m10; m11 += rhs.m11; m12 += rhs.m12; m13 += rhs.m13;
			m20 += rhs.m20; m21 += rhs.m21; m22 += rhs.m22; m23 += rhs.m23;
			m30 += rhs.m30; m31 += rhs.m31; m32 += rhs.m32; m33 += rhs.m33;
			return *this;
		}
		inline FMATRIX4&	operator-=(const float rhs)
		{
			m00 -= rhs; m01 -= rhs; m02 -= rhs; m03 -= rhs;
			m10 -= rhs; m11 -= rhs; m12 -= rhs; m13 -= rhs;
			m20 -= rhs; m21 -= rhs; m22 -= rhs; m23 -= rhs;
			m30 -= rhs; m31 -= rhs; m32 -= rhs; m33 -= rhs;
			return *this;
		}
		inline FMATRIX4&	operator-=(const FMATRIX4& rhs)
		{
			m00 -= rhs.m00; m01 -= rhs.m01; m02 -= rhs.m02; m03 -= rhs.m03;
			m10 -= rhs.m10; m11 -= rhs.m11; m12 -= rhs.m12; m13 -= rhs.m13;
			m20 -= rhs.m20; m21 -= rhs.m21; m22 -= rhs.m22; m23 -= rhs.m23;
			m30 -= rhs.m30; m31 -= rhs.m31; m32 -= rhs.m32; m33 -= rhs.m33;
			return *this;
		}
		inline FMATRIX4&	operator*=(const float rhs)
		{
			m00 *= rhs; m01 *= rhs; m02 *= rhs; m03 *= rhs;
			m10 *= rhs; m11 *= rhs; m12 *= rhs; m13 *= rhs;
			m20 *= rhs; m21 *= rhs; m22 *= rhs; m23 *= rhs;
			m30 *= rhs; m31 *= rhs; m32 *= rhs; m33 *= rhs;
			return *this;
		}
		inline FMATRIX4&	operator*=(const FMATRIX4& rhs)
		{
#ifdef MATH_USE_SSE
			SSE::sseMatrixMultiply(this, this, &rhs);
#else
			FMATRIX4 r;
			r.m00 = m00 * rhs.m00 + m01 * rhs.m10 + m02 * rhs.m20 + m03 * rhs.m30;
			r.m01 = m00 * rhs.m01 + m01 * rhs.m11 + m02 * rhs.m21 + m03 * rhs.m31;
			r.m02 = m00 * rhs.m02 + m01 * rhs.m12 + m02 * rhs.m22 + m03 * rhs.m32;
			r.m03 = m00 * rhs.m03 + m01 * rhs.m13 + m02 * rhs.m23 + m03 * rhs.m33;
			r.m10 = m10 * rhs.m00 + m11 * rhs.m10 + m12 * rhs.m20 + m13 * rhs.m30;
			r.m11 = m10 * rhs.m01 + m11 * rhs.m11 + m12 * rhs.m21 + m13 * rhs.m31;
			r.m12 = m10 * rhs.m02 + m11 * rhs.m12 + m12 * rhs.m22 + m13 * rhs.m32;
			r.m13 = m10 * rhs.m03 + m11 * rhs.m13 + m12 * rhs.m23 + m13 * rhs.m33;
			r.m20 = m20 * rhs.m00 + m21 * rhs.m10 + m22 * rhs.m20 + m23 * rhs.m30;
			r.m21 = m20 * rhs.m01 + m21 * rhs.m11 + m22 * rhs.m21 + m23 * rhs.m31;
			r.m22 = m20 * rhs.m02 + m21 * rhs.m12 + m22 * rhs.m22 + m23 * rhs.m32;
			r.m23 = m20 * rhs.m03 + m21 * rhs.m13 + m22 * rhs.m23 + m23 * rhs.m33;
			r.m30 = m30 * rhs.m00 + m31 * rhs.m10 + m32 * rhs.m20 + m33 * rhs.m30;
			r.m31 = m30 * rhs.m01 + m31 * rhs.m11 + m32 * rhs.m21 + m33 * rhs.m31;
			r.m32 = m30 * rhs.m02 + m31 * rhs.m12 + m32 * rhs.m22 + m33 * rhs.m32;
			r.m33 = m30 * rhs.m03 + m31 * rhs.m13 + m32 * rhs.m23 + m33 * rhs.m33;
			*this = r;
#endif
			return *this;
		}
		inline FMATRIX4&	operator/(const float rhs)
		{
			m00 /= rhs; m01 /= rhs; m02 /= rhs; m03 /= rhs;
			m10 /= rhs; m11 /= rhs; m12 /= rhs; m13 /= rhs;
			m20 /= rhs; m21 /= rhs; m22 /= rhs; m23 /= rhs;
			m30 /= rhs; m31 /= rhs; m32 /= rhs; m33 /= rhs;
			return *this;
		}
		inline bool			operator==(const FMATRIX4& rhs) const
		{
			for (int i = 0; i < 16; ++i) if (!C_FLT(m[i], rhs.m[i])) return false;
			return true;
		}
		inline bool			operator!=(const FMATRIX4& rhs) const
		{
			for (int i = 0; i < 16; ++i) if (!C_FLT(m[i], rhs.m[i])) return true;
			return false;
		}
		inline FMATRIX4		operator+(const FMATRIX4& rhs) const
		{
			FMATRIX4 r;
			r.m00 = m00 + rhs.m00; r.m01 = m01 + rhs.m01; r.m02 = m02 + rhs.m02; r.m03 = m03 + rhs.m03;
			r.m10 = m10 + rhs.m10; r.m11 = m11 + rhs.m11; r.m12 = m12 + rhs.m12; r.m13 = m13 + rhs.m13;
			r.m20 = m20 + rhs.m20; r.m21 = m21 + rhs.m21; r.m22 = m22 + rhs.m22; r.m23 = m23 + rhs.m23;
			r.m30 = m30 + rhs.m30; r.m31 = m31 + rhs.m31; r.m32 = m32 + rhs.m32; r.m33 = m33 + rhs.m33;
			return r;
		}
		inline FMATRIX4		operator-(const FMATRIX4& rhs) const
		{
			FMATRIX4 r;
			r.m00 = m00 - rhs.m00; r.m01 = m01 - rhs.m01; r.m02 = m02 - rhs.m02; r.m03 = m03 - rhs.m03;
			r.m10 = m10 - rhs.m10; r.m11 = m11 - rhs.m11; r.m12 = m12 - rhs.m12; r.m13 = m13 - rhs.m13;
			r.m20 = m20 - rhs.m20; r.m21 = m21 - rhs.m21; r.m22 = m22 - rhs.m22; r.m23 = m23 - rhs.m23;
			r.m30 = m30 - rhs.m30; r.m31 = m31 - rhs.m31; r.m32 = m32 - rhs.m32; r.m33 = m33 - rhs.m33;
			return r;
		}
		inline FMATRIX4		operator*(const FMATRIX4& rhs) const
		{
			FMATRIX4 r;
#ifdef MATH_USE_SSE
			SSE::sseMatrixMultiply(&r, this, &rhs);
#else
			r.m00 = m00 * rhs.m00 + m01 * rhs.m10 + m02 * rhs.m20 + m03 * rhs.m30;
			r.m01 = m00 * rhs.m01 + m01 * rhs.m11 + m02 * rhs.m21 + m03 * rhs.m31;
			r.m02 = m00 * rhs.m02 + m01 * rhs.m12 + m02 * rhs.m22 + m03 * rhs.m32;
			r.m03 = m00 * rhs.m03 + m01 * rhs.m13 + m02 * rhs.m23 + m03 * rhs.m33;
			r.m10 = m10 * rhs.m00 + m11 * rhs.m10 + m12 * rhs.m20 + m13 * rhs.m30;
			r.m11 = m10 * rhs.m01 + m11 * rhs.m11 + m12 * rhs.m21 + m13 * rhs.m31;
			r.m12 = m10 * rhs.m02 + m11 * rhs.m12 + m12 * rhs.m22 + m13 * rhs.m32;
			r.m13 = m10 * rhs.m03 + m11 * rhs.m13 + m12 * rhs.m23 + m13 * rhs.m33;
			r.m20 = m20 * rhs.m00 + m21 * rhs.m10 + m22 * rhs.m20 + m23 * rhs.m30;
			r.m21 = m20 * rhs.m01 + m21 * rhs.m11 + m22 * rhs.m21 + m23 * rhs.m31;
			r.m22 = m20 * rhs.m02 + m21 * rhs.m12 + m22 * rhs.m22 + m23 * rhs.m32;
			r.m23 = m20 * rhs.m03 + m21 * rhs.m13 + m22 * rhs.m23 + m23 * rhs.m33;
			r.m30 = m30 * rhs.m00 + m31 * rhs.m10 + m32 * rhs.m20 + m33 * rhs.m30;
			r.m31 = m30 * rhs.m01 + m31 * rhs.m11 + m32 * rhs.m21 + m33 * rhs.m31;
			r.m32 = m30 * rhs.m02 + m31 * rhs.m12 + m32 * rhs.m22 + m33 * rhs.m32;
			r.m33 = m30 * rhs.m03 + m31 * rhs.m13 + m32 * rhs.m23 + m33 * rhs.m33;
#endif
			return r;
		}
		inline FMATRIX4		operator*(const float rhs) const
		{
			FMATRIX4 r;
			r.m00 = m00 * rhs; r.m01 = m01 * rhs; r.m02 = m02 * rhs; r.m03 = m03 * rhs;
			r.m10 = m10 * rhs; r.m11 = m11 * rhs; r.m12 = m12 * rhs; r.m13 = m13 * rhs;
			r.m20 = m20 * rhs; r.m21 = m21 * rhs; r.m22 = m22 * rhs; r.m23 = m23 * rhs;
			r.m30 = m30 * rhs; r.m31 = m31 * rhs; r.m32 = m32 * rhs; r.m33 = m33 * rhs;
			return r;
		}
		inline FMATRIX4		operator/(const float rhs) const
		{
			FMATRIX4 r;
			r.m00 = m00 / rhs; r.m01 = m01 / rhs; r.m02 = m02 / rhs; r.m03 = m03 / rhs;
			r.m10 = m10 / rhs; r.m11 = m11 / rhs; r.m12 = m12 / rhs; r.m13 = m13 / rhs;
			r.m20 = m20 / rhs; r.m21 = m21 / rhs; r.m22 = m22 / rhs; r.m23 = m23 / rhs;
			r.m30 = m30 / rhs; r.m31 = m31 / rhs; r.m32 = m32 / rhs; r.m33 = m33 / rhs;
			return r;
		}

		inline float		get(int r, int c) const { return m[r * 4 + c]; }
		inline float		get(int idx) const { return m[idx]; }
		inline void*		ptr() { return m; }
		inline const void*	ptr() const { return m; }

		inline void			translate(float x, float y, float z) { m30 += x; m31 += y; m32 += z; }
		inline void			translate(const FVECTOR3& rhs) { m30 += rhs.x; m31 += rhs.y; m32 += rhs.z; }
		inline void			scale(float x, float y, float z)
		{
			m00 *= x; m01 *= x; m02 *= x; m03 *= x;
			m10 *= y; m11 *= y; m12 *= y; m13 *= y;
			m20 *= z; m21 *= z; m22 *= z; m23 *= z;
		}
		inline void			scale(const FVECTOR3& rhs) { scale(rhs[0], rhs[1], rhs[2]); }

		inline void			rotateAxis(const FVECTOR3& axis, float radian)
		{
			float x = axis.x, y = axis.y, z = axis.z;
			float fsin = std::sin(radian);
			float fcos = std::cos(radian);
			FMATRIX4 local, final;
			local.m00 = (x * x) * (1.0f - fcos) + fcos;
			local.m01 = (x * y) * (1.0f - fcos) + (z * fsin);
			local.m02 = (x * z) * (1.0f - fcos) - (y * fsin);
			local.m03 = 0.0f;
			local.m10 = (y * x) * (1.0f - fcos) - (z * fsin);
			local.m11 = (y * y) * (1.0f - fcos) + fcos;
			local.m12 = (y * z) * (1.0f - fcos) + (x * fsin);
			local.m13 = 0.0f;
			local.m20 = (z * x) * (1.0f - fcos) + (y * fsin);
			local.m21 = (z * y) * (1.0f - fcos) - (x * fsin);
			local.m22 = (z * z) * (1.0f - fcos) + fcos;
			local.m23 = 0.0f;
			local.m30 = 0.0f; local.m31 = 0.0f; local.m32 = 0.0f; local.m33 = 1.0f;
			final = local * (*this);
			*this = final;
		}
		inline void			rotateAxisX(float radian)
		{
			float fsin = std::sin(radian);
			float fcos = std::cos(radian);
			float temp10 = m10 * fcos + m20 * fsin;
			float temp11 = m11 * fcos + m21 * fsin;
			float temp12 = m12 * fcos + m22 * fsin;
			float temp13 = m13 * fcos + m23 * fsin;
			float temp20 = m10 * -fsin + m20 * fcos;
			float temp21 = m11 * -fsin + m21 * fcos;
			float temp22 = m12 * -fsin + m22 * fcos;
			float temp23 = m13 * -fsin + m23 * fcos;
			m10 = temp10; m11 = temp11; m12 = temp12; m13 = temp13;
			m20 = temp20; m21 = temp21; m22 = temp22; m23 = temp23;
		}
		inline void			rotateAxisY(float radian)
		{
			float fsin = std::sin(radian);
			float fcos = std::cos(radian);
			float temp00 = m00 * fcos - m20 * fsin;
			float temp01 = m01 * fcos - m21 * fsin;
			float temp02 = m02 * fcos - m22 * fsin;
			float temp03 = m03 * fcos - m23 * fsin;
			float temp20 = m00 * fsin + m20 * fcos;
			float temp21 = m01 * fsin + m21 * fcos;
			float temp22 = m02 * fsin + m22 * fcos;
			float temp23 = m03 * fsin + m23 * fcos;
			m00 = temp00; m01 = temp01; m02 = temp02; m03 = temp03;
			m20 = temp20; m21 = temp21; m22 = temp22; m23 = temp23;
		}
		inline void			rotateAxisZ(float radian)
		{
			float fsin = std::sin(radian);
			float fcos = std::cos(radian);
			float temp00 = m00 * fcos + m10 * fsin;
			float temp01 = m01 * fcos + m11 * fsin;
			float temp02 = m02 * fcos + m12 * fsin;
			float temp03 = m03 * fcos + m13 * fsin;
			float temp10 = m00 * -fsin + m10 * fcos;
			float temp11 = m01 * -fsin + m11 * fcos;
			float temp12 = m02 * -fsin + m12 * fcos;
			float temp13 = m03 * -fsin + m13 * fcos;
			m00 = temp00; m01 = temp01; m02 = temp02; m03 = temp03;
			m10 = temp10; m11 = temp11; m12 = temp12; m13 = temp13;
		}
		inline FVECTOR3		rotate(const FVECTOR3& rhs) const
		{
			FVECTOR3 r;
			r.x = rhs.x * m00 + rhs.y * m10 + rhs.z * m20;
			r.y = rhs.x * m01 + rhs.y * m11 + rhs.z * m21;
			r.z = rhs.x * m02 + rhs.y * m12 + rhs.z * m22;
			return r;
		}
		inline void			rotate(FVECTOR3& v) const
		{
			float newX = v.x * m00 + v.y * m10 + v.z * m20;
			float newY = v.x * m01 + v.y * m11 + v.z * m21;
			float newZ = v.x * m02 + v.y * m12 + v.z * m22;
			v.x = newX;
			v.y = newY;
			v.z = newZ;
		}
		inline void			rotate(const FVECTOR3& in, FVECTOR3& out) const
		{
			out.x = in.x * m00 + in.y * m10 + in.z * m20;
			out.y = in.x * m01 + in.y * m11 + in.z * m21;
			out.z = in.x * m02 + in.y * m12 + in.z * m22;
		}
		inline void			rotate(FVECTOR4& v) const
		{
			float newX = v.x * m00 + v.y * m10 + v.z * m20;
			float newY = v.x * m01 + v.y * m11 + v.z * m21;
			float newZ = v.x * m02 + v.y * m12 + v.z * m22;
			v.x = newX;
			v.y = newY;
			v.z = newZ;
			v.w = 1.0;
		}
		inline void			rotate(FVECTOR4& in, FVECTOR4& out) const
		{
			out.x = in.x * m00 + in.y * m10 + in.z * m20;
			out.y = in.x * m01 + in.y * m11 + in.z * m21;
			out.z = in.x * m02 + in.y * m12 + in.z * m22;
			out.w = 1.0;
		}
		inline void			transform(FVECTOR3& v) const
		{
			float newX = v.x * m00 + v.y * m10 + v.z * m20 + m30;
			float newY = v.x * m01 + v.y * m11 + v.z * m21 + m31;
			float newZ = v.x * m02 + v.y * m12 + v.z * m22 + m32;
			v.x = newX;
			v.y = newY;
			v.z = newZ;
		}
		inline void			transform(FVECTOR3& in, FVECTOR3& out) const
		{
			out.x = in.x * m00 + in.y * m10 + in.z * m20 + m30;
			out.y = in.x * m01 + in.y * m11 + in.z * m21 + m31;
			out.z = in.x * m02 + in.y * m12 + in.z * m22 + m32;
		}
		inline void			inverseTransform(FVECTOR3& v) const
		{
			FVECTOR3 t;
			t = v - translation();

			float newX = t.x * m00 + t.y * m01 + t.z * m02;
			float newY = t.x * m10 + t.y * m11 + t.z * m12;
			float newZ = t.x * m20 + t.y * m21 + t.z * m22;
			v.x = newX;
			v.y = newY;
			v.z = newZ;
		}
		inline void			inverseTransform(const FVECTOR3& in, FVECTOR3& out) const
		{
			FVECTOR3 t;
			t = in - translation();

			out.x = t.x * m00 + t.y * m01 + t.z * m02;
			out.y = t.x * m10 + t.y * m11 + t.z * m12;
			out.z = t.x * m20 + t.y * m21 + t.z * m22;
		}
		inline void			inverseRotate(Vector3& v) const
		{
			float newX = v.x * m00 + v.y * m01 + v.z * m02;
			float newY = v.x * m10 + v.y * m11 + v.z * m12;
			float newZ = v.x * m20 + v.y * m21 + v.z * m22;
			v.x = newX;
			v.y = newY;
			v.z = newZ;
		}
		inline void			inverseRotate(const Vector3& in, Vector3& out) const
		{
			out.x = in.x * m00 + in.y * m01 + in.z * m02;
			out.y = in.x * m10 + in.y * m11 + in.z * m12;
			out.z = in.x * m20 + in.y * m21 + in.z * m22;
		}
		inline FMATRIX4&	transpose3x3()
		{
			std::swap(m01, m10);
			std::swap(m02, m20);
			std::swap(m12, m21);
			return *this;
		}
		inline FMATRIX4&	transpose()
		{
			std::swap(m01, m10); std::swap(m02, m20); std::swap(m03, m30);
			std::swap(m12, m21); std::swap(m13, m31); std::swap(m23, m32);
			return *this;
		}

		inline void		invert()
		{
#ifdef MATH_USE_SSE
			SSE::sseMatrixInverse((__m128 *)this, (__m128 *)this);
#else
			invert((void *)this, (void *)this);
#endif
		}

		inline bool		invertNoTranslation()	// returns false if matrix is singular
		{
			float d00 = (m11 * m22 - m21 * m12);
			float d01 = (m21 * m02 - m01 * m22);
			float d02 = (m01 * m12 - m11 * m02);

			// early out if the matrix is singular
			float determinant = (m00 * d00) + (m10 * d01) + (m20 * d02);
			if (determinant == 0.0f)
				return false;

			float d10 = (m12 * m20 - m10 * m22);
			float d11 = (m00 * m22 - m02 * m20);
			float d12 = (m02 * m10 - m00 * m12);

			float d20 = (m10 * m21 - m11 * m20);
			float d21 = (m01 * m20 - m00 * m21);
			float d22 = (m00 * m11 - m01 * m10);

			determinant = 1.0f / determinant;

			m00 = d00 * determinant; m01 = d01 * determinant; m02 = d02 * determinant;
			m10 = d10 * determinant; m11 = d11 * determinant; m12 = d12 * determinant;
			m20 = d20 * determinant; m21 = d21 * determinant; m22 = d22 * determinant;
			return true;
		}
		inline void		invertFast()	// note this transposes the rotation to invert it, will not work with matrices with scaling
		{
			FVECTOR3 trans(m30, m31, m32);

			// multiply position vector by -1 * inv(rotation matrix)
			m30 = -(m00 * trans.x + m01 * trans.y + m02 * trans.z);
			m31 = -(m10 * trans.x + m11 * trans.y + m12 * trans.z);
			m32 = -(m20 * trans.x + m21 * trans.y + m22 * trans.z);

			std::swap(m01, m10);
			std::swap(m02, m20);
			std::swap(m12, m21);
		}

		float determinant() const
		{
			//this suppose to be apply in Column-Major matrix, but since det(M) = det(M(T)),so it is common used.
			float a0 = m[0] * m[5] - m[1] * m[4];
			float a1 = m[0] * m[6] - m[2] * m[4];
			float a2 = m[0] * m[7] - m[3] * m[4];
			float a3 = m[1] * m[6] - m[2] * m[5];
			float a4 = m[1] * m[7] - m[3] * m[5];
			float a5 = m[2] * m[7] - m[3] * m[6];
			float b0 = m[8] * m[13] - m[9] * m[12];
			float b1 = m[8] * m[14] - m[10] * m[12];
			float b2 = m[8] * m[15] - m[11] * m[12];
			float b3 = m[9] * m[14] - m[10] * m[13];
			float b4 = m[9] * m[15] - m[11] * m[13];
			float b5 = m[10] * m[15] - m[11] * m[14];
			// Calculate the determinant.
			return (a0 * b5 - a1 * b4 + a2 * b3 + a3 * b2 - a4 * b1 + a5 * b0);
		}

		bool decompose(Vector3* scale, Quaternion* rotation, Vector3* translation) const
		{

#if 1
			if (translation) {// Extract the translation.
				translation->x = m30;
				translation->y = m31;
				translation->z = m32;
			}
			// Nothing left to do.
			if (scale == nullptr && rotation == nullptr)
				return true;
			// Extract the scale.
			// This is simply the length of each axis (row/column) in the matrix.
			Vector3 xBais(m00, m01, m02);
			float scaleX = xBais.length();

			Vector3 yBais(m10, m11, m12);
			float scaleY = yBais.length();

			Vector3 zBais(m20, m21, m22);
			float scaleZ = zBais.length();

			// Determine if we have a negative scale (true if determinant is less than zero).
			// In this case, we simply negate a single axis of the scale.
			float det = determinant();
			if (det < 0)
				scaleZ = -scaleZ;

			if (scale) {
				scale->x = scaleX;
				scale->y = scaleY;
				scale->z = scaleZ;
			}

			// Nothing left to do.
			if (rotation == nullptr)
				return true;

			const float MATH_TOLERANCE = 1e-5f;
			// Scale too close to zero, can't decompose rotation.
			if (scaleX < MATH_TOLERANCE || scaleY < MATH_TOLERANCE || std::abs(scaleZ) < MATH_TOLERANCE)
				return false;

			float rn;
			// Factor the scale out of the matrix axes.
			rn = 1.0f / scaleX;
			xBais = xBais * rn;

			rn = 1.0f / scaleY;
			yBais = yBais * rn;

			rn = 1.0f / scaleZ;
			zBais = zBais * rn;

			float m11 = xBais.x, m12 = xBais.y, m13 = xBais.z;
			float m21 = yBais.x, m22 = yBais.y, m23 = yBais.z;
			float m31 = zBais.x, m32 = zBais.y, m33 = zBais.z;
			//make sure they are orthogonality

			// Now calculate the rotation from the resulting matrix (axes).
			float wf = m11 + m22 + m33 + 1.0f;
			float xf = m11 - m22 - m33 + 1;
			float yf = -m11 + m22 - m33 + 1;
			float zf = -m11 - m22 + m33 + 1;
			float maxf = max(wf, max(xf, max(yf, zf)));
			float v = std::sqrt(maxf);
			float s = 0.5f / v;
			v *= 0.5f;
			if (maxf == wf) {
				rotation->w = v;
				rotation->x = (m23 - m32) * s;
				rotation->y = (m31 - m13) * s;
				rotation->z = (m12 - m21) * s;
			}
			else if (maxf == xf) {
				rotation->x = v;
				rotation->w = (m23 - m32) * s;
				rotation->y = (m12 + m21) * s;
				rotation->z = (m31 + m13) * s;
			}
			else if (maxf == yf) {
				rotation->y = v;
				rotation->w = (m31 - m13) * s;
				rotation->x = (m12 + m21) * s;
				rotation->z = (m23 + m32) * s;
			}
			else {//maxf == zf
				rotation->z = v;
				rotation->w = (m12 - m21) * s;
				rotation->x = (m31 + m13) * s;
				rotation->y = (m23 + m32) * s;
			}

#endif
			return true;
		}

		bool isIdentify() const
		{
			bool ret = true;
			for (int i = 0; i < 16; ++i) {
				bool satisfy = (i % 5 == 0 && m[i] == 1) || (i % 5 != 0 && m[i] == 0);
				if (!satisfy) {
					ret = false;
					break;
				}
			}
			return ret;
		}

		inline void identity()
		{
			m00 = 1.0f, m01 = 0.0f, m02 = 0.0f, m03 = 0.0f;
			m10 = 0.0f, m11 = 1.0f, m12 = 0.0f, m13 = 0.0f;
			m20 = 0.0f, m21 = 0.0f, m22 = 1.0f, m23 = 0.0f;
			m30 = 0.0f, m31 = 0.0f, m32 = 0.0f, m33 = 1.0f;
		}

		inline friend FVECTOR3 operator*(const FVECTOR3& lhs, const FMATRIX4& rhs)
		{
			FVECTOR3 r;
			r.x = lhs.x * rhs.m00 + lhs.y * rhs.m10 + lhs.z * rhs.m20 + rhs.m30;
			r.y = lhs.x * rhs.m01 + lhs.y * rhs.m11 + lhs.z * rhs.m21 + rhs.m31;
			r.z = lhs.x * rhs.m02 + lhs.y * rhs.m12 + lhs.z * rhs.m22 + rhs.m32;
			return r;
		}
		inline friend FVECTOR4 operator*(const FVECTOR4& lhs, const FMATRIX4& rhs)
		{
			Vector4 r;
			r.x = lhs.x * rhs.m00 + lhs.y * rhs.m10 + lhs.z * rhs.m20 + lhs.w * rhs.m30;
			r.y = lhs.x * rhs.m01 + lhs.y * rhs.m11 + lhs.z * rhs.m21 + lhs.w * rhs.m31;
			r.z = lhs.x * rhs.m02 + lhs.y * rhs.m12 + lhs.z * rhs.m22 + lhs.w * rhs.m32;
			r.w = lhs.x * rhs.m03 + lhs.y * rhs.m13 + lhs.z * rhs.m23 + lhs.w * rhs.m33;
			return r;
		}

		static FMATRIX4& scale(float x, float y, float z, FMATRIX4& result)
		{
			result.identity();
			result.m00 = x;
			result.m11 = y;
			result.m22 = z;
			return result;
		}

		static FMATRIX4& rotateAxis(FMATRIX4& out, const FVECTOR3& axis, float radian)
		{
			FVECTOR3 axisNor = axis;
			axisNor.normalize();
			out.identity();
			out.rotateAxis(axisNor, radian);
			return out;
		}

		static FMATRIX4& rotateAxisX(FMATRIX4& out, float radian)
		{
			out.identity();
			out.rotateAxisX(radian);
			return out;
		}

		static FMATRIX4& rotateAxisY(FMATRIX4& out, float radian)
		{
			out.identity();
			out.rotateAxisY(radian);
			return out;
		}

		static FMATRIX4& rotateAxisZ(FMATRIX4& out, float radian)
		{
			out.identity();
			out.rotateAxisZ(radian);
			return out;
		}

		static FMATRIX4& rotateQuaternion(FMATRIX4& out, const FQUATERNION& q)
		{
			const float q0t2 = q.w * 2;
			const float q1t2 = q.x * 2;

			const float q0sq = q.w * q.w;
			const float q1sq = q.x * q.x;
			const float q2sq = q.y * q.y;
			const float q3sq = q.z * q.z;

			const float q0q1 = q0t2 * q.x;
			const float q0q2 = q0t2 * q.y;
			const float q0q3 = q0t2 * q.z;

			const float q1q2 = q1t2 * q.y;
			const float q1q3 = q1t2 * q.z;
			const float q2q3 = q.y * q.z * 2;

			out.m00 = q0sq + q1sq - q2sq - q3sq;	out.m01 = q1q2 + q0q3;					out.m02 = -q0q2 + q1q3;					out.m03 = 0;
			out.m10 = q1q2 - q0q3;					out.m11 = q0sq - q1sq + q2sq - q3sq;	out.m12 = q0q1 + q2q3;					out.m13 = 0;
			out.m20 = q0q2 + q1q3;					out.m21 = -q0q1 + q2q3;					out.m22 = q0sq - q1sq - q2sq + q3sq;	out.m23 = 0;
			out.m30 = 0.0f;							out.m31 = 0.0f;							out.m32 = 0.0f;							out.m33 = 1.0f;

			return out;
		}

		static FMATRIX4& rotateYawPitchRoll(FMATRIX4& out, float yaw, float pitch, float roll)
		{
			FQUATERNION q;
			FQUATERNION::rotationYawPitchRoll(q, yaw, pitch, roll);
			rotateQuaternion(out, q);
			return out;
		}

		static FMATRIX4& translate(FMATRIX4& out, const FVECTOR3& value)
		{
			out.identity();
			out.m30 = value.x;
			out.m31 = value.y;
			out.m32 = value.z;

			return out;
		}

		static FMATRIX4& scale(FMATRIX4& out, const FVECTOR3& value)
		{
			return scale(value.x, value.y, value.z, out);
		}

		// Mout = Msc-1 * Msr-1 * Ms * Msr * Msc * Mrc-1 * Mr * Mrc * Mt
		static FMATRIX4& transformation(FMATRIX4& out, const FVECTOR3& scalingCenter, const FQUATERNION& scalingRotation, const FVECTOR3& scaling, const FVECTOR3& rotationCenter, const FQUATERNION& rotation, const FVECTOR3& translation)
		{
			FMATRIX4 sc, sr, srt, s, rc, r, t;
			srt = rotateQuaternion(sr, scalingRotation);
			out = translate(sc, -scalingCenter) * srt.transpose() * scale(s, scaling) * sr * translate(sc, scalingCenter) * translate(rc, -rotationCenter) *
				rotateQuaternion(r, rotation) * translate(rc, rotationCenter) * translate(t, translation);
			return out;
		}

	private:
		static __forceinline void invert(void* dstMatrix, const void* srcMatrix)
		{
			typedef float Float4x4[4][4];
			const Float4x4& M = *((const Float4x4*)srcMatrix);
			Float4x4 Result;
			float Det[4];
			Float4x4 Tmp;

			Tmp[0][0] = M[2][2] * M[3][3] - M[2][3] * M[3][2];
			Tmp[0][1] = M[1][2] * M[3][3] - M[1][3] * M[3][2];
			Tmp[0][2] = M[1][2] * M[2][3] - M[1][3] * M[2][2];

			Tmp[1][0] = M[2][2] * M[3][3] - M[2][3] * M[3][2];
			Tmp[1][1] = M[0][2] * M[3][3] - M[0][3] * M[3][2];
			Tmp[1][2] = M[0][2] * M[2][3] - M[0][3] * M[2][2];

			Tmp[2][0] = M[1][2] * M[3][3] - M[1][3] * M[3][2];
			Tmp[2][1] = M[0][2] * M[3][3] - M[0][3] * M[3][2];
			Tmp[2][2] = M[0][2] * M[1][3] - M[0][3] * M[1][2];

			Tmp[3][0] = M[1][2] * M[2][3] - M[1][3] * M[2][2];
			Tmp[3][1] = M[0][2] * M[2][3] - M[0][3] * M[2][2];
			Tmp[3][2] = M[0][2] * M[1][3] - M[0][3] * M[1][2];

			Det[0] = M[1][1] * Tmp[0][0] - M[2][1] * Tmp[0][1] + M[3][1] * Tmp[0][2];
			Det[1] = M[0][1] * Tmp[1][0] - M[2][1] * Tmp[1][1] + M[3][1] * Tmp[1][2];
			Det[2] = M[0][1] * Tmp[2][0] - M[1][1] * Tmp[2][1] + M[3][1] * Tmp[2][2];
			Det[3] = M[0][1] * Tmp[3][0] - M[1][1] * Tmp[3][1] + M[2][1] * Tmp[3][2];

			float Determinant = M[0][0] * Det[0] - M[1][0] * Det[1] + M[2][0] * Det[2] - M[3][0] * Det[3];
			const float	RDet = 1.0f / Determinant;

			Result[0][0] = RDet * Det[0];
			Result[0][1] = -RDet * Det[1];
			Result[0][2] = RDet * Det[2];
			Result[0][3] = -RDet * Det[3];
			Result[1][0] = -RDet * (M[1][0] * Tmp[0][0] - M[2][0] * Tmp[0][1] + M[3][0] * Tmp[0][2]);
			Result[1][1] = RDet * (M[0][0] * Tmp[1][0] - M[2][0] * Tmp[1][1] + M[3][0] * Tmp[1][2]);
			Result[1][2] = -RDet * (M[0][0] * Tmp[2][0] - M[1][0] * Tmp[2][1] + M[3][0] * Tmp[2][2]);
			Result[1][3] = RDet * (M[0][0] * Tmp[3][0] - M[1][0] * Tmp[3][1] + M[2][0] * Tmp[3][2]);
			Result[2][0] = RDet * (
				M[1][0] * (M[2][1] * M[3][3] - M[2][3] * M[3][1]) -
				M[2][0] * (M[1][1] * M[3][3] - M[1][3] * M[3][1]) +
				M[3][0] * (M[1][1] * M[2][3] - M[1][3] * M[2][1])
				);
			Result[2][1] = -RDet * (
				M[0][0] * (M[2][1] * M[3][3] - M[2][3] * M[3][1]) -
				M[2][0] * (M[0][1] * M[3][3] - M[0][3] * M[3][1]) +
				M[3][0] * (M[0][1] * M[2][3] - M[0][3] * M[2][1])
				);
			Result[2][2] = RDet * (
				M[0][0] * (M[1][1] * M[3][3] - M[1][3] * M[3][1]) -
				M[1][0] * (M[0][1] * M[3][3] - M[0][3] * M[3][1]) +
				M[3][0] * (M[0][1] * M[1][3] - M[0][3] * M[1][1])
				);
			Result[2][3] = -RDet * (
				M[0][0] * (M[1][1] * M[2][3] - M[1][3] * M[2][1]) -
				M[1][0] * (M[0][1] * M[2][3] - M[0][3] * M[2][1]) +
				M[2][0] * (M[0][1] * M[1][3] - M[0][3] * M[1][1])
				);
			Result[3][0] = -RDet * (
				M[1][0] * (M[2][1] * M[3][2] - M[2][2] * M[3][1]) -
				M[2][0] * (M[1][1] * M[3][2] - M[1][2] * M[3][1]) +
				M[3][0] * (M[1][1] * M[2][2] - M[1][2] * M[2][1])
				);
			Result[3][1] = RDet * (
				M[0][0] * (M[2][1] * M[3][2] - M[2][2] * M[3][1]) -
				M[2][0] * (M[0][1] * M[3][2] - M[0][2] * M[3][1]) +
				M[3][0] * (M[0][1] * M[2][2] - M[0][2] * M[2][1])
				);
			Result[3][2] = -RDet * (
				M[0][0] * (M[1][1] * M[3][2] - M[1][2] * M[3][1]) -
				M[1][0] * (M[0][1] * M[3][2] - M[0][2] * M[3][1]) +
				M[3][0] * (M[0][1] * M[1][2] - M[0][2] * M[1][1])
				);
			Result[3][3] = RDet * (
				M[0][0] * (M[1][1] * M[2][2] - M[1][2] * M[2][1]) -
				M[1][0] * (M[0][1] * M[2][2] - M[0][2] * M[2][1]) +
				M[2][0] * (M[0][1] * M[1][2] - M[0][2] * M[1][1])
				);

			memcpy(dstMatrix, &Result, 16 * sizeof(float));
		}
	};

	typedef FMATRIX3	Matrix3;
	typedef FMATRIX3	Mat3;
	typedef FMATRIX4	Matrix4;
	typedef FMATRIX4	Mat4;
	typedef FMATRIX4	Matrix;

	typedef FVECTOR2	float2;
	typedef FVECTOR3	float3;
	typedef FVECTOR4	float4;
	typedef	FMATRIX4	float4x4;
}