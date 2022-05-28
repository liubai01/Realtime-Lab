#pragma once
#include <memory>
#include <string>
#include <stdexcept>

#include <windows.h>
#include <debugapi.h>
#include <comdef.h>

namespace dout {
  // refer to https://stackoverflow.com/questions/2342162/stdstring-formatting-like-sprintf
  template<typename ... Args>
  std::string string_format(const std::string& format, Args ... args)
  {
    int size_s = std::snprintf(nullptr, 0, format.c_str(), args ...) + 1; // Extra space for '\0'
    if (size_s <= 0) { 
      throw std::runtime_error("Error during formatting."); 
    }
    auto size = static_cast<size_t>(size_s);
    std::unique_ptr<char[]> buf(new char[size]);
    std::snprintf(buf.get(), size, format.c_str(), args ...);
    return std::string(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside
  }

  template<typename ... Args>
  void printf(const std::string& format, Args ... args)
  {
    std::string ret = string_format(format, args...);
    OutputDebugStringA(ret.c_str());
  }

  class DxException
  {
  public:
    DxException() = default;
    DxException(HRESULT hr, const std::wstring& functionName, const std::wstring& filename, int lineNumber);

    std::wstring ToString()const;

    HRESULT ErrorCode = S_OK;
    std::wstring FunctionName;
    std::wstring Filename;
    int LineNumber = -1;
  };

  inline std::wstring AnsiToWString(const std::string& str)
  {
    WCHAR buffer[512];
    MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, buffer, 512);
    return std::wstring(buffer);
  }

  #ifndef ThrowIfFailed
  #define ThrowIfFailed(x)                                              \
  {                                                                     \
      HRESULT hr__ = (x);                                               \
      std::wstring wfn = dout::AnsiToWString(__FILE__);                       \
      if(FAILED(hr__)) { throw dout::DxException(hr__, L#x, wfn, __LINE__); } \
  }
  #endif

}