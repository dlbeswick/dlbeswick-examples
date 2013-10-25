#ifndef __ALERT_H__
#define __ALERT_H__

#include "Standard/api.h"

#include <string>
#include <cstdio>

inline void alert(const char* s, const char* title = "Alert") { MessageBox(0, s, title, 0); }
inline void alertf(const char* s, ...) { char buf[2048]; va_list v; va_start(v, s); _vsnprintf(buf, 2048, s, v); MessageBox(0, buf, "Alert", 0); }
inline bool alertfc(const char* s, ...) { char buf[2048]; va_list v; va_start(v, s); _vsnprintf(buf, 2048, s, v); return MessageBox(0, buf, "Alert", MB_OKCANCEL) == IDOK; }

#endif
