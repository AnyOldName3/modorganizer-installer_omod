#include "OMODFrameworkWrapper.h"

using namespace cli;

#include <imodinterface.h>
#include <iplugingame.h>
#include <log.h>
#include <utility.h>

#include "implementations/CodeProgress.h"
#include "implementations/Logger.h"
#include "implementations/ScriptFunctions.h"

#include "interop/QtDotNetConverters.h"
#include "interop/StdDotNetConverters.h"

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
    initFrameworkSettings();
    MOBase::log::debug("Installing {} as OMOD", archiveName);
    // Stack allocating should dispose like a `using` statement in C#
    OMODFramework::OMOD omod(toDotNetString(archiveName));

    if (!System::String::IsNullOrEmpty(omod.ModName))
      modName.update(toQString(omod.ModName), MOBase::EGuessQuality::GUESS_META);

    MOBase::IModInterface* modInterface = mMoInfo->createMod(modName);
    if (!modInterface)
      return EInstallResult::RESULT_CANCELED;
    
    if (omod.HasReadme)
      MOBase::log::debug("{}", toUTF8String(omod.GetReadme()));

    if (omod.HasScript)
    {
      MOBase::log::debug("Mod has script. Run it.");
      OMODFramework::Scripting::IScriptFunctions^ scriptFunctions = gcnew ScriptFunctions(mParentWidget);
      OMODFramework::ScriptReturnData^ scriptData = OMODFramework::Scripting::ScriptRunner::RunScript(%omod, scriptFunctions);
    }
    else
    {
      MOBase::log::debug("Mod has no script. Install contents directly.");
      System::String^ data = omod.GetDataFiles();
      System::String^ plugins = omod.GetPlugins();
      if (data)
      {
        if (MOBase::shellMove(toQString(data) + "/*.*", modInterface->absolutePath(), true, mParentWidget))
          MOBase::log::debug("Installed mod files.");
        else
          MOBase::log::error("Error while installing mod files.");
        QFile::remove(toQString(data));
      }
      if (plugins)
      {
        if (MOBase::shellMove(toQString(plugins) + "/*.*", modInterface->absolutePath(), true, mParentWidget))
          MOBase::log::debug("Installed mod plugins.");
        else
          MOBase::log::error("Error while installing mod plugins.");
        QFile::remove(toQString(plugins));
      }
    }

    return EInstallResult::RESULT_SUCCESS;
  }
  catch (System::Exception^ dotNetException)
  {
    throw toStdException(dotNetException);
  }
}

void OMODFrameworkWrapper::initFrameworkSettings()
{
  OMODFramework::Framework::Settings->CodeProgress = gcnew CodeProgress();

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
}
