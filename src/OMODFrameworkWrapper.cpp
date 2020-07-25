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

#include "MessageBoxHelper.h"

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

  connect(this, &OMODFrameworkWrapper::createMod, this, &OMODFrameworkWrapper::createModSlot, Qt::ConnectionType::BlockingQueuedConnection);
  connect(this, &OMODFrameworkWrapper::displayReadme, this, &OMODFrameworkWrapper::displayReadmeSlot, Qt::ConnectionType::BlockingQueuedConnection);
  connect(this, &OMODFrameworkWrapper::showWaitDialog, this, &OMODFrameworkWrapper::showWaitDialogSlot, Qt::ConnectionType::QueuedConnection);
  connect(this, &OMODFrameworkWrapper::hideWaitDialog, this, &OMODFrameworkWrapper::hideWaitDialogSlot, Qt::ConnectionType::QueuedConnection);
}

ref class InstallInAnotherThreadHelper
{
public:
  InstallInAnotherThreadHelper(OMODFrameworkWrapper* owner, MOBase::GuessedValue<QString>& modName, QString gameName, const QString& archiveName, const QString& version, int nexusID)
    : mModName(&modName)
    , mGameName(&gameName)
    , mArchiveName(&archiveName)
    , mVersion(&version)
    , mNexusID(nexusID)
    , mOwner(owner)
    , mExceptionPtr(new std::exception_ptr)
    , mHasResult(false)
  {}

  ~InstallInAnotherThreadHelper()
  {
    if (!mExceptionPtr)
      return;

    this->!InstallInAnotherThreadHelper();
  }

  !InstallInAnotherThreadHelper()
  {
    delete mExceptionPtr;
    mExceptionPtr = nullptr;
  }

  void Run()
  {
    try
    {
      mResult = mOwner->install(*mModName, *mGameName, *mArchiveName, *mVersion, mNexusID);
      mHasResult = true;
    }
    catch (...)
    {
      *mExceptionPtr = std::current_exception();
    }
  }

  bool HasResult()
  {
    return mHasResult;
  }

  OMODFrameworkWrapper::EInstallResult Result()
  {
    return mResult;
  }

  std::exception_ptr ExceptionPtr()
  {
    return *mExceptionPtr;
  }

private:
  // We own none of these pointers. They're pointers because managed objects can't have unmanaged members, but a pointer is just an integer of some form, which is the same.
  MOBase::GuessedValue<QString>* mModName;
  QString* mGameName;
  const QString* mArchiveName;
  const QString* mVersion;
  int mNexusID;

  OMODFrameworkWrapper* mOwner;

  std::exception_ptr* mExceptionPtr;
  OMODFrameworkWrapper::EInstallResult mResult;
  bool mHasResult;
};

OMODFrameworkWrapper::EInstallResult OMODFrameworkWrapper::installInAnotherThread(MOBase::GuessedValue<QString>& modName, QString gameName, const QString& archiveName, const QString& version, int nexusID)
{
  QEventLoop eventLoop;
  InstallInAnotherThreadHelper^ helper = gcnew InstallInAnotherThreadHelper(this, modName, gameName, archiveName, version, nexusID);
  System::Threading::Tasks::Task^ installationTask = System::Threading::Tasks::Task::Run(gcnew System::Action(helper, &InstallInAnotherThreadHelper::Run));

  // TODO: connect stuff to eventLoop.wakeUp.
  // Installation manager does this with futureWatcher.finished and progressUpdate
  while (!installationTask->IsCompleted)
    eventLoop.processEvents(QEventLoop::AllEvents | QEventLoop::WaitForMoreEvents);

  if (helper->HasResult())
    return helper->Result();
  else if (helper->ExceptionPtr())
    std::rethrow_exception(helper->ExceptionPtr());
  else
    throw std::runtime_error("Something went horribly wrong when asynchronously installing an OMOD. We don't even have the original exception.");
}

OMODFrameworkWrapper::EInstallResult OMODFrameworkWrapper::install(MOBase::GuessedValue<QString>& modName, QString gameName, const QString& archiveName, const QString& version, int nexusID)
{
  try
  {
    MessageBoxHelper::unique_ptr messageBoxHelper = MessageBoxHelper::make_unique();

    QTemporaryDir tempPath(toQString(System::IO::Path::Combine(System::IO::Path::GetPathRoot(toDotNetString(mMoInfo->modsPath())), "OMODTempXXXXXX")));
    initFrameworkSettings(tempPath.path());
    MOBase::log::debug("Installing {} as OMOD", archiveName);
    // Stack allocating should dispose like a `using` statement in C#
    emit showWaitDialog("Initializing OMOD installer... ");
    OMODFramework::OMOD omod(toDotNetString(archiveName));
    emit hideWaitDialog();

    if (!System::String::IsNullOrEmpty(omod.ModName))
      modName.update(toQString(omod.ModName), MOBase::EGuessQuality::GUESS_META);

    // TODO: let user rename mod

    MOBase::IModInterface* modInterface;
    emit createMod(modInterface, modName);
    if (!modInterface)
      return EInstallResult::RESULT_CANCELED;

    if (omod.HasReadme)
      emit displayReadme(toQString(omod.ModName), toQString(omod.GetReadme()));

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
            if (!oldValue.isEmpty())
            {
              /*: %1 is the mod name
                  [%2] is the ini section name.
                  %3 is the ini setting name.
                  %4 is the value already in Oblivion.ini.
                  %5 is the value the mod wants to set.
              */
              message = tr("%1 wants to change [%2] %3 from \"%4\" to \"%5\"").arg(modName).arg(section).arg(name).arg(oldValue).arg(newValue);
            }
            else
            {
              /*: %1 is the mod name
                  [%2] is the ini section name.
                  %3 is the ini setting name.
                  %5 is the value the mod wants to set.
              */
              message = tr("%1 wants to set [%2] %3 to \"%4\"").arg(modName).arg(section).arg(name).arg(newValue);
            }

            response = messageBoxHelper->question(mParentWidget, tr("Update INI?"), message, QMessageBox::Yes | QMessageBox::No | QMessageBox::YesToAll | QMessageBox::NoToAll);
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
        /*: %1 is the mod name
            <ul><li>%2</li></ul> becomes a list of ESPs and ESMs the OMOD installed and tried to activate.
            <ul><li>%3</li></ul> becomes a list of ESPs and ESMs the OMOD installed avoided activating.
            The point of this popup is to suggest which plugins the user might need to activate themselves.
        */
        QString message = tr("%1 installed and wants to activate the following plugins:<ul><li>%2</li></ul>However, it didn't try to activate these plugins:<ul><li>%3</li></ul>");
        message = message.arg(toQString(omod.ModName));
        message = message.arg(toQString(System::String::Join("</li><li>", installedPlugins)));
        message = message.arg(toQString(System::String::Join("</li><li>", scriptData->UncheckedPlugins)));
        messageBoxHelper->information(mParentWidget, tr("OMOD didn't activate all plugins"), message);
      }

      std::map<QString, int> unhandledScriptReturnDataCounts;
      unhandledScriptReturnDataCounts["ESPDeactivation"] = scriptData->ESPDeactivation ? scriptData->ESPDeactivation->Count : 0;
      unhandledScriptReturnDataCounts["EarlyPlugins"] = scriptData->EarlyPlugins ? scriptData->EarlyPlugins->Count : 0;
      unhandledScriptReturnDataCounts["LoadOrderSet"] = scriptData->LoadOrderSet ? scriptData->LoadOrderSet->Count : 0;
      unhandledScriptReturnDataCounts["ConflictsWith"] = scriptData->ConflictsWith ? scriptData->ConflictsWith->Count : 0;
      unhandledScriptReturnDataCounts["DependsOn"] = scriptData->DependsOn ? scriptData->DependsOn->Count : 0;

      unhandledScriptReturnDataCounts["RegisterBSASet"] = scriptData->RegisterBSASet ? scriptData->RegisterBSASet->Count : 0;
      unhandledScriptReturnDataCounts["SDPEdits"] = scriptData->SDPEdits ? scriptData->SDPEdits->Count : 0;
      unhandledScriptReturnDataCounts["ESPEdits"] = scriptData->ESPEdits ? scriptData->ESPEdits->Count : 0;
      unhandledScriptReturnDataCounts["PatchFiles"] = scriptData->PatchFiles ? scriptData->PatchFiles->Count : 0;

      for (const auto& unhandledThing : unhandledScriptReturnDataCounts)
      {
        if (unhandledThing.second)
        {
          /*: %1 is the mod name
              %2 is the name of a field in the OMOD's return data
              Hopefully this message will never be seen by anyone, but if it is, they need to know to tell the Mod Organizer 2 dev team.
          */
          QString userMessage = tr("%1 has data for %2, but Mod Organizer 2 doesn't know what to do with it yet. Please report this to the Mod Organizer 2 development team (ideally by sending us your interface log) as we didn't find any OMODs that actually did this, and we need to know that they exist.");
          userMessage = userMessage.arg(toQString(omod.ModName));
          userMessage = userMessage.arg(unhandledThing.first);
          messageBoxHelper->warning(mParentWidget, tr("Mod Organizer 2 can't completely install this OMOD."), userMessage);
          MOBase::log::warn("{} ({}) contains {} entries for {}", toUTF8String(omod.ModName), archiveName, unhandledThing.second, unhandledThing.first);
        }
      }
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

  // This is a hack to fix an OMOD framework bug and should be removed once it's fixed.
  OMODFramework::Framework::Settings->DllPath = System::IO::Path::Combine(System::IO::Path::GetDirectoryName(OMODFramework::Framework::Settings->DllPath), "OMODFramework.Scripting.dll");

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

void OMODFrameworkWrapper::createModSlot(MOBase::IModInterface*& modInterfaceOut, MOBase::GuessedValue<QString>& modName)
{
  modInterfaceOut = mMoInfo->createMod(modName);
}

void OMODFrameworkWrapper::displayReadmeSlot(const QString& modName, const QString& readme)
{
  if (QMessageBox::question(mParentWidget, tr("Display Readme?"),
    //: <br> is a line break. Translators can remove it if it makes things clearer.
    tr("The Readme may explain installation options. Display it?<br>It will remain visible until you close it.")) == QMessageBox::StandardButton::Yes)
  {
    // TODO: ideally this wouldn't be part of the same window heirarchy so that modal popups in the installer don't prevent it being moved/resized etc.
    // DarNified UI's popups are modal for the whole process, so any fancy trick needs to be *here*.
    RtfPopup* readmePopup = new RtfPopup(toDotNetString(readme), mParentWidget);
    //: %1 is the mod name
    readmePopup->setWindowTitle(tr("%1 Readme").arg(modName));
    readmePopup->show();
    readmePopup->setAttribute(Qt::WA_DeleteOnClose);
  }
}

void OMODFrameworkWrapper::showWaitDialogSlot(QString message) {
  mWaitDialog = new QProgressDialog(message, QString(), 0, 0, mParentWidget);
  mWaitDialog->setWindowFlags(mWaitDialog->windowFlags() & ~Qt::WindowContextHelpButtonHint & ~Qt::WindowCloseButtonHint);
  mWaitDialog->setWindowModality(Qt::WindowModal);
  mWaitDialog->show();
}

void OMODFrameworkWrapper::hideWaitDialogSlot() {
  mWaitDialog->hide();
  mWaitDialog->deleteLater();
}
