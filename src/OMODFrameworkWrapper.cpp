#include "OMODFrameworkWrapper.h"

using namespace cli;

#include <algorithm>

#include <QMessageBox>
#include <QTemporaryDir>

#include <imodinterface.h>
#include <iplugingame.h>
#include <log.h>
#include <utility.h>
#include <registry.h>

#include "implementations/CodeProgress.h"
#include "implementations/Logger.h"
#include "implementations/ScriptFunctions.h"

#include "interop/QtDotNetConverters.h"
#include "interop/StdDotNetConverters.h"

#include "newstuff/rtfPopup.h"

// We want to search the plugin data directory for .NET DLLs
class AssemblyResolver
{
public:
  static bool getInitialised() { return sInitialised; }

  static void initialise(MOBase::IOrganizer* organizer)
  {
    if (sInitialised)
      return;
    sPluginDataPath = organizer->pluginDataPath();
    System::AppDomain::CurrentDomain->AssemblyResolve += gcnew System::ResolveEventHandler(&OnAssemblyResolve);
    sInitialised = true;
  }

private:
  static System::Reflection::Assembly^ OnAssemblyResolve(System::Object^ sender, System::ResolveEventArgs^ args);

  static bool sInitialised;
  static QDir sPluginDataPath;
};

bool AssemblyResolver::sInitialised = false;
QDir AssemblyResolver::sPluginDataPath;

System::Reflection::Assembly^ AssemblyResolver::OnAssemblyResolve(System::Object^ sender, System::ResolveEventArgs^ args)
{
  QString name = toQString(args->Name).section(',', 0, 0) + ".dll";
  if (sPluginDataPath.exists(name))
    return System::Reflection::Assembly::LoadFrom(toDotNetString(sPluginDataPath.absoluteFilePath(name)));
  return nullptr;
}

OMODFrameworkWrapper::OMODFrameworkWrapper(MOBase::IOrganizer* organizer, QWidget* parentWidget)
  : mMoInfo(organizer)
  , mParentWidget(parentWidget)
{
  AssemblyResolver::initialise(mMoInfo);
}

OMODFrameworkWrapper::EInstallResult OMODFrameworkWrapper::install(MOBase::GuessedValue<QString>& modName, QString gameName, const QString& archiveName, const QString& version, int nexusID)
{
  try
  {
    QTemporaryDir tempPath(toQString(System::IO::Path::Combine(System::IO::Path::GetPathRoot(toDotNetString(mMoInfo->modsPath())), "OMODTempXXXXXX")));
    initFrameworkSettings(tempPath.path());
    MOBase::log::debug("Installing {} as OMOD", archiveName);
    // Stack allocating should dispose like a `using` statement in C#
    OMODFramework::OMOD omod(toDotNetString(archiveName));

    if (!System::String::IsNullOrEmpty(omod.ModName))
      modName.update(toQString(omod.ModName), MOBase::EGuessQuality::GUESS_META);

    MOBase::IModInterface* modInterface = mMoInfo->createMod(modName);
    if (!modInterface)
      return EInstallResult::RESULT_CANCELED;

    // TODO: make localisable
    if (omod.HasReadme && QMessageBox::question(mParentWidget, "Display Readme?", "The Readme may explain installation options. Display it?<br>It will remain visible until you close it.") == QMessageBox::StandardButton::Yes)
    {
      // TODO: ideally this wouldn't be part of the same window heirarchy so that modal popups in the installer don't prevent it being moved/resized etc.
      // DarNified UI's popups are modal for the whole process, so any fancy trick needs to be *here*.
      RtfPopup* readmePopup = new RtfPopup(omod.GetReadme(), mParentWidget);
      readmePopup->setWindowTitle(toQString(omod.ModName) + " Readme");
      readmePopup->show();
      readmePopup->setAttribute(Qt::WA_DeleteOnClose);
    }

    if (omod.HasScript)
    {
      MOBase::log::debug("Mod has script. Run it.");
      OMODFramework::Scripting::IScriptFunctions^ scriptFunctions = gcnew ScriptFunctions(mParentWidget, mMoInfo);
      OMODFramework::ScriptReturnData^ scriptData = OMODFramework::Scripting::ScriptRunner::RunScript(%omod, scriptFunctions);
      if (!scriptData)
        throw std::runtime_error("OMOD script returned no result. This isn't supposed to happen.");
      if (scriptData->CancelInstall)
        return EInstallResult::RESULT_CANCELED;

      // inis first so that you don't need to wait for extraction before a second batch of questions appears
      if (scriptData->INIEdits && scriptData->INIEdits->Count)
      {
        QString oblivionIniPath = (mMoInfo->profile()->localSettingsEnabled() ? QDir(mMoInfo->profile()->absolutePath()) : mMoInfo->managedGame()->documentsDirectory()).absoluteFilePath("Oblivion.ini");
        bool yesToAll = false;
        for each (OMODFramework::INIEditInfo ^ edit in scriptData->INIEdits)
        {
          QString section = toQString(edit->Section);
          section = section.mid(1, section.size() - 2);
          QString name = toQString(edit->Name);
          QString newValue = toQString(edit->NewValue);
          QString oldValue;
          if (edit->OldValue)
            oldValue = toQString(edit->OldValue);
          else
          {
            // I'm pretty sure this is the maximum length for vanilla Oblivion.
            wchar_t buffer[256];
            if (GetPrivateProfileString(section.toStdWString().data(), name.toStdWString().data(), nullptr, buffer, sizeof(buffer) / sizeof(buffer[0]), oblivionIniPath.toStdWString().data()))
              oldValue = QString::fromWCharArray(buffer);
          }

          MOBase::log::debug("OMOD wants to set [{}] {} to \"{}\", was \"{}\"", section, name, newValue, oldValue);

          QMessageBox::StandardButton response;
          if (!yesToAll)
          {
            QString message;
            // TODO: make localisable
            if (!oldValue.isEmpty())
              message = QString("%1 wants to change [%2] %3 from \"%4\" to \"%5\"").arg(modName).arg(section).arg(name).arg(oldValue).arg(newValue);
            else
              message = QString("%1 wants to set [%2] %3 to \"%4\"").arg(modName).arg(section).arg(name).arg(newValue);

            response = QMessageBox::question(mParentWidget, "Update INI?", message, QMessageBox::Yes | QMessageBox::No | QMessageBox::YesToAll | QMessageBox::NoToAll);
            if (response == QMessageBox::NoToAll)
            {
              MOBase::log::debug("User skipped all.");
              break;
            }

            yesToAll |= response == QMessageBox::YesToAll;
          }

          if (yesToAll || response == QMessageBox::StandardButton::Yes)
          {
            MOBase::log::debug("Doing edit.");
            MOBase::WriteRegistryValue(section.toStdWString().data(), name.toStdWString().data(), newValue.toStdWString().data(), oblivionIniPath.toStdWString().data());
          }
          else
            MOBase::log::debug("User skipped edit.");
        }
      }

      scriptData->Pretty(%omod, omod.GetDataFiles(), omod.GetPlugins());
      // no compatability between auto and var makes me :angery:
      System::Collections::Generic::HashSet<System::String^>^ installedPlugins = gcnew System::Collections::Generic::HashSet<System::String^>(System::StringComparer::InvariantCultureIgnoreCase);
      for each (OMODFramework::InstallFile file in scriptData->InstallFiles)
      {
        System::String^ destinationPath = System::IO::Path::Combine(toDotNetString(modInterface->absolutePath()), file.InstallTo);
        System::IO::Directory::CreateDirectory(System::IO::Path::GetDirectoryName(destinationPath));
        System::IO::File::Copy(file.InstallFrom, destinationPath, true);
        System::String^ extension = System::IO::Path::GetExtension(file.InstallTo);
        if (extension && (extension->Equals(".esm", System::StringComparison::InvariantCultureIgnoreCase) || extension->Equals(".esp", System::StringComparison::InvariantCultureIgnoreCase)))
          installedPlugins->Add(file.InstallTo);
      }

      if (scriptData->UncheckedPlugins)
        installedPlugins->ExceptWith(scriptData->UncheckedPlugins);

      if (installedPlugins->Count && scriptData->UncheckedPlugins && scriptData->UncheckedPlugins->Count)
      {
        // TODO: make localisable
        QString message("%1 installed and wants to activate the following plugins:<ul><li>%2</li></ul>However, it didn't try to activate these plugins:<ul><li>%3</li></ul>");
        message = message.arg(toQString(omod.ModName));
        message = message.arg(toQString(System::String::Join("</li><li>", installedPlugins)));
        message = message.arg(toQString(System::String::Join("</li><li>", scriptData->UncheckedPlugins)));
        // TODO: make localisable
        QMessageBox::information(mParentWidget, "OMOD didn't activate all plugins", message);
      }

      // TODO: the rest of the script return data.
    }
    else
    {
      MOBase::log::debug("Mod has no script. Install contents directly.");
      QString data = toQString(omod.GetDataFiles());
      QString plugins = toQString(omod.GetPlugins());
      if (!data.isNull())
      {
        if (MOBase::shellMove(data + "/*.*", modInterface->absolutePath(), true, mParentWidget))
          MOBase::log::debug("Installed mod files.");
        else
          MOBase::log::error("Error while installing mod files.");
        QFile::remove(data);
      }
      if (!plugins.isNull())
      {
        if (MOBase::shellMove(plugins + "/*.*", modInterface->absolutePath(), true, mParentWidget))
          MOBase::log::debug("Installed mod plugins.");
        else
          MOBase::log::error("Error while installing mod plugins.");
        QFile::remove(plugins);
      }
    }

    // on success, set mod info
    MOBase::VersionInfo modVersion(std::max(int(omod.MajorVersion), 0), std::max(int(omod.MinorVersion), 0), std::max(int(omod.BuildVersion), 0));
    modInterface->setVersion(modVersion);

    // TODO: parse omod.Website. If it's Nexus, set the ID, otherwise set custom URL in meta.ini. We can't set the URL with the installation manager.
    // TODO: maybe convert omod.Description to HTML and set it as nexusDescription
    // TODO: maybe Holt will finish the proposed mod metadata API and there'll be a better, tidier option.

    return EInstallResult::RESULT_SUCCESS;
  }
  catch (const std::exception& e)
  {
    throw;
  }
  catch (System::Exception^ dotNetException)
  {
    throw toStdException(dotNetException);
  }
}

void OMODFrameworkWrapper::initFrameworkSettings(const QString& tempPath)
{
  OMODFramework::Framework::Settings->CodeProgress = gcnew CodeProgress();

  if (!tempPath.isEmpty())
    OMODFramework::Framework::Settings->TempPath = toDotNetString(tempPath);

  OMODFramework::LoggingSettings^ loggingSettings = OMODFramework::Framework::Settings->LoggingSettings;
  loggingSettings->UseLogger = true;
  loggingSettings->LogToFile = false;
  loggingSettings->LowestLoggingLevel = Logger::OMODLoggingLevel(MOBase::log::getDefault().level());
  loggingSettings->Logger = gcnew Logger();

  OMODFramework::ScriptExecutionSettings^ scriptSettings = gcnew OMODFramework::ScriptExecutionSettings();
  scriptSettings->EnableWarnings = true;
  scriptSettings->OblivionGamePath = toDotNetString(mMoInfo->managedGame()->gameDirectory().path());
  System::String^ iniLocation = toDotNetString(mMoInfo->profile()->localSettingsEnabled() ? mMoInfo->profile()->absolutePath() : mMoInfo->managedGame()->documentsDirectory().path());
  scriptSettings->OblivionINIPath = System::IO::Path::Combine(iniLocation, "Oblivion.ini");
  scriptSettings->OblivionRendererInfoPath = System::IO::Path::Combine(iniLocation, "RendererInfo.txt");
  scriptSettings->ReadINIWithInterface = false;
  scriptSettings->ReadRendererInfoWithInterface = false;
  scriptSettings->HandleBSAsWithInterface = false;
  scriptSettings->PatchWithInterface = false;
  scriptSettings->UseSafePatching = true;

  OMODFramework::Framework::Settings->ScriptExecutionSettings = scriptSettings;
}
