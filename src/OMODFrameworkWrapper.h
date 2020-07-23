#pragma once

#include <QCoreApplication>

#include <iplugininstaller.h>

class OMODFrameworkWrapper : public QObject
{
  Q_OBJECT

public:
  using EInstallResult = MOBase::IPluginInstaller::EInstallResult;

  OMODFrameworkWrapper(MOBase::IOrganizer* organizer, QWidget* parentWidget);

  EInstallResult installInAnotherThread(MOBase::GuessedValue<QString>& modName, QString gameName, const QString& archiveName, const QString& version, int nexusID);

  EInstallResult install(MOBase::GuessedValue<QString>& modName, QString gameName, const QString& archiveName, const QString& version, int nexusID);

protected:
  void initFrameworkSettings(const QString& tempPath);

signals:
  void createMod(MOBase::GuessedValue<QString>& modName, MOBase::IModInterface*& modInterfaceOut);

  void displayReadme(const QString& modName, const QString& readme);

protected slots:
  void createModSlot(MOBase::GuessedValue<QString>& modName, MOBase::IModInterface*& modInterfaceOut);

  void displayReadmeSlot(const QString& modName, const QString& readme);

private:
  MOBase::IOrganizer* mMoInfo;
  QWidget* mParentWidget;
};
