#pragma once

using namespace cli;

#include <QWidget>

ref class ScriptFunctions : OMODFramework::Scripting::IScriptFunctions
{
public:
  ScriptFunctions(QWidget* parentWidget);

  // note: C++/CLI wants virtual for interface implementations, not override
  virtual void Warn(System::String^ msg);

  virtual void Message(System::String^ msg);

  virtual void Message(System::String^ msg, System::String^ title);

  virtual System::Collections::Generic::List<int>^ Select(System::Collections::Generic::List<System::String^>^ items, System::String^ title, bool isMultiSelect, System::Collections::Generic::List<System::String^>^ previews, System::Collections::Generic::List<System::String^>^ descriptions);

  virtual System::String^ InputString(System::String^ title, System::String^ initialText);

  virtual int DialogYesNo(System::String^ title);

  virtual int DialogYesNo(System::String^ title, System::String^ message);

  virtual void DisplayImage(System::String^ path, System::String^ title);

  virtual void DisplayText(System::String^ text, System::String^ title);

  virtual void Patch(System::String^ from, System::String^ to);

  virtual System::String^ ReadOblivionINI(System::String^ section, System::String^ name);

  virtual System::String^ ReadRendererInfo(System::String^ name);

  virtual bool DataFileExists(System::String^ path);

  virtual bool HasScriptExtender();

  virtual bool HasGraphicsExtender();

  virtual System::Version^ ScriptExtenderVersion();

  virtual System::Version^ GraphicsExtenderVersion();

  virtual System::Version^ OblivionVersion();

  virtual System::Version^ OBSEPluginVersion(System::String^ path);

  virtual System::Collections::Generic::IEnumerable<OMODFramework::Scripting::ScriptESP>^ GetESPs();

  virtual System::Collections::Generic::IEnumerable<System::String^>^ GetActiveOMODNames();

  virtual cli::array<unsigned char, 1>^ ReadExistingDataFile(System::String^ file);

  virtual cli::array<unsigned char, 1>^ GetDataFileFromBSA(System::String^ file);

  virtual cli::array<unsigned char, 1>^ GetDataFileFromBSA(System::String^ bsa, System::String^ file);

private:
  QWidget* mParentWidget;
};
