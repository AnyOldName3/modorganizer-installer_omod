#include "ScriptFunctions.h"

#include <QMessageBox>

#include "../interop/QtDotNetConverters.h"

ScriptFunctions::ScriptFunctions(QWidget* parentWidget) : mParentWidget(parentWidget) {}

void ScriptFunctions::Warn(System::String^ msg)
{
  QMessageBox::warning(mParentWidget, "Warning", toQString(msg));
}

void ScriptFunctions::Message(System::String^ msg)
{
  QMessageBox::information(mParentWidget, "Message", toQString(msg));
}

void ScriptFunctions::Message(System::String^ msg, System::String^ title)
{
  QMessageBox::information(mParentWidget, toQString(title), toQString(msg));
}

System::Collections::Generic::List<int>^ ScriptFunctions::Select(System::Collections::Generic::List<System::String^>^ items, System::String^ title, bool isMultiSelect, System::Collections::Generic::List<System::String^>^ previews, System::Collections::Generic::List<System::String^>^ descriptions)
{
  return gcnew System::Collections::Generic::List<int>(1);
  throw gcnew System::NotImplementedException();
  // TODO: insert return statement here
}

System::String^ ScriptFunctions::InputString(System::String^ title, System::String^ initialText)
{
  throw gcnew System::NotImplementedException();
  // TODO: insert return statement here
}

int ScriptFunctions::DialogYesNo(System::String^ title)
{
  return QMessageBox::question(mParentWidget, toQString(title), toQString(title)) == QMessageBox::StandardButton::Yes;
}

int ScriptFunctions::DialogYesNo(System::String^ title, System::String^ message)
{
  return QMessageBox::question(mParentWidget, toQString(title), toQString(message)) == QMessageBox::StandardButton::Yes;
}

void ScriptFunctions::DisplayImage(System::String^ path, System::String^ title)
{
  throw gcnew System::NotImplementedException();
}

void ScriptFunctions::DisplayText(System::String^ text, System::String^ title)
{
  throw gcnew System::NotImplementedException();
}

void ScriptFunctions::Patch(System::String^ from, System::String^ to)
{
  throw gcnew System::NotImplementedException();
}

System::String^ ScriptFunctions::ReadOblivionINI(System::String^ section, System::String^ name)
{
  throw gcnew System::NotImplementedException();
  // TODO: insert return statement here
}

System::String^ ScriptFunctions::ReadRendererInfo(System::String^ name)
{
  throw gcnew System::NotImplementedException();
  // TODO: insert return statement here
}

bool ScriptFunctions::DataFileExists(System::String^ path)
{
  return false;
}

bool ScriptFunctions::HasScriptExtender()
{
  return false;
}

bool ScriptFunctions::HasGraphicsExtender()
{
  return false;
}

System::Version^ ScriptFunctions::ScriptExtenderVersion()
{
  throw gcnew System::NotImplementedException();
  // TODO: insert return statement here
}

System::Version^ ScriptFunctions::GraphicsExtenderVersion()
{
  throw gcnew System::NotImplementedException();
  // TODO: insert return statement here
}

System::Version^ ScriptFunctions::OblivionVersion()
{
  throw gcnew System::NotImplementedException();
  // TODO: insert return statement here
}

System::Version^ ScriptFunctions::OBSEPluginVersion(System::String^ path)
{
  throw gcnew System::NotImplementedException();
  // TODO: insert return statement here
}

System::Collections::Generic::IEnumerable<OMODFramework::Scripting::ScriptESP>^ ScriptFunctions::GetESPs()
{
  throw gcnew System::NotImplementedException();
  // TODO: insert return statement here
}

System::Collections::Generic::IEnumerable<System::String^>^ ScriptFunctions::GetActiveOMODNames()
{
  throw gcnew System::NotImplementedException();
  // TODO: insert return statement here
}

cli::array<unsigned char, 1>^ ScriptFunctions::ReadExistingDataFile(System::String^ file)
{
  throw gcnew System::NotImplementedException();
  // TODO: insert return statement here
}

cli::array<unsigned char, 1>^ ScriptFunctions::GetDataFileFromBSA(System::String^ file)
{
  throw gcnew System::NotImplementedException();
  // TODO: insert return statement here
}

cli::array<unsigned char, 1>^ ScriptFunctions::GetDataFileFromBSA(System::String^ bsa, System::String^ file)
{
  throw gcnew System::NotImplementedException();
  // TODO: insert return statement here
}
