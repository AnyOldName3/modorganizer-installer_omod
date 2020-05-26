#include "QtDotNetConverters.h"

#include <string>

QString toQString(System::String^ string)
{
  System::IntPtr chars = System::Runtime::InteropServices::Marshal::StringToHGlobalUni(string);
  QString qString = QString::fromWCharArray((wchar_t*)chars.ToPointer());
  System::Runtime::InteropServices::Marshal::FreeHGlobal(chars);
  return qString;
}

System::String^ toDotNetString(const QString& qString)
{
  return gcnew System::String(qString.toStdWString().c_str());
}
