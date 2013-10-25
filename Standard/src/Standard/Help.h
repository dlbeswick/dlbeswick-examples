#ifndef STANDARD_HELP_H
#define STANDARD_HELP_H

#include "Standard/api.h"
#undef min
#undef max
#include <algorithm>
#include <string>
#include <cstdio>

#define freePtr(x) if (x) { delete x; x = 0; }
#define freeDX(x) if (x) { x->Release(); x = 0; }

template<class T> inline void clamp(T& var, const T& minVal = 0, const T& maxVal = 1) { var = std::max(minVal, var); var = std::min(maxVal, var); }
template<class T> inline T clamp(const T& var, const T& minVal = 0, const T& maxVal = 1) { T r = var; r = std::max(minVal, r); r = std::min(maxVal, r); return r; }

// global std::string append helpers
inline std::string operator + (const std::string& s, const char* c) { std::string q(s); q += c; return q; }
inline std::string operator + (const char* c, const std::string& s) { std::string q(s); q.insert(0, c); return q; }
inline std::string operator + (const std::string& s, int i) { std::string q(s); char buf[64]; sprintf(buf, "%d", i); q += buf; return q; }
inline std::string operator + (int i, const std::string& s) { std::string q(s); char buf[64]; sprintf(buf, "%d", i); q.insert(0, buf); return q; }
inline std::string operator + (const std::string& s, uint i) { std::string q(s); char buf[64]; sprintf(buf, "%u", i); q += buf; return q; }
inline std::string operator + (uint i, const std::string& s) { std::string q(s); char buf[64]; sprintf(buf, "%u", i); q.insert(0, buf); return q; }
inline std::string operator + (const std::string& s, float f) { std::string q(s); char buf[64]; sprintf(buf, "%.2f", f); q += buf; return q; }
inline std::string operator + (float f, const std::string& s) { std::string q(s); char buf[64]; sprintf(buf, "%.2f", f); q.insert(0, buf); return q; }

inline std::string ftostr (float f) { char buf[64]; sprintf(buf, "%.2f", f); return buf; }
inline std::string itostr (int i) { char buf[64]; sprintf(buf, "%d", i); return buf; }
inline std::string ptostr (void* p) { char buf[64]; sprintf(buf, "%p", p); return buf; }
inline std::string durationtostr (int ms) { char buf[64]; sprintf(buf, "%d:%02d", ms/60000, (ms/1000)%60); return buf; }

template<class T> inline void zero(T& p) { memset(&p, 0, sizeof(T)); }

// get number of ticks per second
STANDARD_API std::string	getCPUVendorID();
STANDARD_API unsigned long	getCPUTicksPerSecond();

#endif
