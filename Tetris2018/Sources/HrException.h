//************************************************************************
/*	
 *	HrException.h
 */
//************************************************************************
#ifndef HREXCEPTION_H
#define HREXCEPTION_H

#include <windows.h>
#include <stdexcept>

using namespace std;

namespace N1D
{
	inline string HrToString(HRESULT hr)
	{
		char s_str[64] = {};
		sprintf_s(s_str, "HRESULT of 0x%08X", static_cast<UINT>(hr));
		return string(s_str);
	}

	class HrException : public runtime_error
	{
	public:
		HrException(HRESULT hr) : runtime_error(HrToString(hr)), _hr(hr) {}
		HRESULT Error() const { return _hr; }

	private:
		const HRESULT _hr;
	};

	inline void ThrowIfFailed(HRESULT hr)
	{
		if (FAILED(hr))
		{
			throw HrException(hr);
		}
	}
}
#endif // HREXCEPTION_H

