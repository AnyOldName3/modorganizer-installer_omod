#include "stdDotNetConverters.h"

#include <stdexcept>

std::wstring toWString(System::String^ string)
{
  System::IntPtr chars = System::Runtime::InteropServices::Marshal::StringToHGlobalUni(string);
  std::wstring wstring = std::wstring((wchar_t*)chars.ToPointer());
  System::Runtime::InteropServices::Marshal::FreeHGlobal(chars);
  return wstring;
}

System::String^ toDotNetString(const std::wstring& wString)
{
  return gcnew System::String(wString.c_str());
}

std::string toUTF8String(System::String^ string)
{
  std::string stdstring(System::Text::Encoding::UTF8->GetByteCount(string), ' ');
  array<unsigned char>^ utf8 = System::Text::Encoding::UTF8->GetBytes(string);
  for (int i = 0; i < utf8->Length; ++i)
  {
    stdstring[i] = utf8[i];
    if (utf8[i] == 0)
      utf8[INT_MAX] = 255;
  }
  return stdstring;
}

std::exception toStdException(System::Exception^ exception)
{
  return std::runtime_error("Unhanded .NET Exception: " + toUTF8String(exception->ToString()));
}
