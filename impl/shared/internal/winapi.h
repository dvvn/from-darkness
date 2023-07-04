#pragma once

// ReSharper disable CppInconsistentNaming
#ifdef _WIN64
using LONG_PTR = __int64;
using UINT_PTR = __int64;
#else
using LONG_PTR   = long;
using UINT_PTR   = unsigned int;
#endif

//#ifdef UNICODE
//using WNDCLASSEX = struct tagWNDCLASSEXW;
//#else
//using WNDCLASSEX = struct tagWNDCLASSEXA;
//#endif

using UINT  = unsigned int;
using DWORD = unsigned long;

using WPARAM  = UINT_PTR;
using LPARAM  = LONG_PTR;
using LRESULT = LONG_PTR;
using HRESULT = long;

using HWND    = struct HWND__ *;
using WNDPROC = LRESULT(__stdcall *)(HWND, UINT, WPARAM, LPARAM);