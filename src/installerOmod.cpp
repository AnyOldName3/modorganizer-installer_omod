#include "installerOmod.h"

#include <iplugingame.h>

#include "OMODFrameworkWrapper.h"

InstallerOMOD::InstallerOMOD() : mMoInfo(nullptr), mOmodFrameworkWrapper(nullptr)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// IPlugin

bool InstallerOMOD::init(MOBase::IOrganizer* moInfo)
{
  mMoInfo = moInfo;
  mOmodFrameworkWrapper = std::make_unique<OMODFrameworkWrapper>(mMoInfo);
  return true;
}

QString InstallerOMOD::name() const
{
  return "Omod Installer";
}

QString InstallerOMOD::localizedName() const
{
  return tr("Omod Installer");
}

QString InstallerOMOD::author() const
{
  return "AnyOldName3 & erril120";
}

QString InstallerOMOD::description() const
{
  return tr("Installer for Omod files (including scripted ones)");
}

MOBase::VersionInfo InstallerOMOD::version() const
{
  return MOBase::VersionInfo(1, 0, 0, MOBase::VersionInfo::RELEASE_PREALPHA);
}

bool InstallerOMOD::isActive() const
{
  return mMoInfo->pluginSetting(name(), "enabled").toBool() && mMoInfo->managedGame()->gameName() == "Oblivion";
}

QList<MOBase::PluginSetting> InstallerOMOD::settings() const
{
  return { MOBase::PluginSetting("enabled", tr("Check to enable this plugin"), QVariant(true)) };
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// IPluginInstaller

unsigned int InstallerOMOD::priority() const
{
  // Some other installers have a use_any_file setting and then they'll try and claim OMODs as their own, so we want higher priority than them.
  return 500;
}

bool InstallerOMOD::isManualInstaller() const
{
  return false;
}

bool InstallerOMOD::isArchiveSupported(std::shared_ptr<const MOBase::IFileTree> tree) const
{
  for (const auto file : *tree) {
    // config is the only file guaranteed to be there
    if (file->isFile() && file->name() == "config")
      return true;
  }
  return false;
}

void InstallerOMOD::setParentWidget(QWidget* parent)
{
  IPluginInstaller::setParentWidget(parent);
  mOmodFrameworkWrapper->setParentWidget(parent);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// IPluginInstallerCustom

bool InstallerOMOD::isArchiveSupported(const QString& archiveName) const
{
  return archiveName.endsWith(".omod", Qt::CaseInsensitive);
}

std::set<QString> InstallerOMOD::supportedExtensions() const
{
  return { "omod" };
}

InstallerOMOD::EInstallResult InstallerOMOD::install(MOBase::GuessedValue<QString>& modName, QString gameName, const QString& archiveName, const QString& version, int nexusID)
{
  return mOmodFrameworkWrapper->installInAnotherThread(modName, gameName, archiveName, version, nexusID);
}
