#include "ScriptFunctions.h"

#include <QDir>
#include <QMessageBox>

#include <iplugingame.h>

#include "../interop/QtDotNetConverters.h"

ScriptFunctions::ScriptFunctions(QWidget* parentWidget, MOBase::IOrganizer* moInfo) : mParentWidget(parentWidget), mMoInfo(moInfo) {}

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
  return mMoInfo->resolvePath(toQString(path)) != "";
}

bool ScriptFunctions::HasScriptExtender()
{
  return mMoInfo->managedGame()->gameDirectory().exists("obse_loader.exe");
}

bool ScriptFunctions::HasGraphicsExtender()
{
  return DataFileExists("obse\\plugins\\obge.dll");
}

System::Version^ ScriptFunctions::ScriptExtenderVersion()
{
  return gcnew System::Version(System::Diagnostics::FileVersionInfo::GetVersionInfo(toDotNetString(mMoInfo->managedGame()->gameDirectory().filePath("obse_loader.exe")))->FileVersion);
}

System::Version^ ScriptFunctions::GraphicsExtenderVersion()
{
  return gcnew System::Version(System::Diagnostics::FileVersionInfo::GetVersionInfo(toDotNetString(mMoInfo->resolvePath("obse\\plugins\\obge.dll")))->FileVersion);
}

System::Version^ ScriptFunctions::OblivionVersion()
{
  return gcnew System::Version(System::Diagnostics::FileVersionInfo::GetVersionInfo(toDotNetString(mMoInfo->managedGame()->gameDirectory().filePath("oblivion.exe")))->FileVersion);
}

System::Version^ ScriptFunctions::OBSEPluginVersion(System::String^ path)
{
  QString pluginPath = mMoInfo->resolvePath(toQString(System::IO::Path::Combine("obse", "plugins", System::IO::Path::ChangeExtension(path, ".dll"))));
  if (pluginPath.isEmpty())
    return nullptr;
  return gcnew System::Version(System::Diagnostics::FileVersionInfo::GetVersionInfo(toDotNetString(pluginPath))->FileVersion);
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
