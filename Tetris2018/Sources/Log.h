//*************************************************************************
/*
 *	Log.h
 */
 //************************************************************************
#ifndef LOG_H
#define LOG_H

#include <windows.h>
#include <stdarg.h>
#include <tchar.h>
#include <locale.h>
#include <tuple>
#include <iostream>
#include <string>

namespace N1D
{
	class Log
	{
	public:
		template <typename ... Args>
		static void Write(const char* format, Args const& ... args) noexcept
		{
			char s[1024];
			sprintf_s(s, format, args...);

			TCHAR ts[2048];
			size_t size;
			setlocale(LC_ALL, "japanese");
			mbstowcs_s(&size, ts, s, _TRUNCATE);
			
			OutputDebugString(ts);
		}
	};
}
#endif // LOG_H

