#ifndef INSTALLEROMOD_H
#define INSTALLEROMOD_H

#include <iplugininstallercustom.h>

#include "OMODFrameworkWrapper.h"

class InstallerOMOD : public MOBase::IPluginInstallerCustom
{
  Q_OBJECT;
  Q_INTERFACES(MOBase::IPlugin MOBase::IPluginInstaller MOBase::IPluginInstallerCustom);
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
  // We should probably buy a domain.
  Q_PLUGIN_METADATA(IID "org.AnyOldName3.InstallerOmod");
#endif

public:
  InstallerOMOD();

  // IPlugin
  bool init(MOBase::IOrganizer* moInfo) override;

  QString name() const override;

  QString localizedName() const override;

  QString author() const override;

  QString description() const override;

  MOBase::VersionInfo version() const override;

  bool isActive() const override;

  QList<MOBase::PluginSetting> settings() const override;

  // IPluginInstaller

  unsigned int priority() const override;

  bool isManualInstaller() const override;

  bool isArchiveSupported(std::shared_ptr<const MOBase::IFileTree> tree) const override;

  void setParentWidget(QWidget* parent) override;

  // IPluginInstallerCustom

  bool isArchiveSupported(const QString& archiveName) const override;

  std::set<QString> supportedExtensions() const override;

  EInstallResult install(MOBase::GuessedValue<QString>& modName, QString gameName, const QString& archiveName, const QString& version, int nexusID) override;

private:
  MOBase::IOrganizer* mMoInfo;
  std::unique_ptr<OMODFrameworkWrapper> mOmodFrameworkWrapper;
};

#endif // !INSTALLEROMOD_H
