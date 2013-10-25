#ifndef __ALERT_H__
#define __ALERT_H__




#include <string>
#include <cstdio>

inline void alert(const char* s, const char* title = "Alert") { portRequired(); }
inline void alertf(const char* s, ...)  { portRequired(); }
inline bool alertfc(const char* s, ...)  { portRequired(); return false; }

#endif
