#include "QtDotNetConverters.h"

#include <string>

#include <msclr\marshal_cppstd.h>

QString toQString(System::String^ string)
{
  if (string)
    return QString::fromStdWString(msclr::interop::marshal_as<std::wstring>(string));
  return "null .NET string";
}

System::String^ toDotNetString(const QString& qString)
{
  return msclr::interop::marshal_as<System::String^>(qString.toStdWString());
}
