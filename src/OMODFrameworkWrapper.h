#pragma once

#include <QCoreApplication>
#include <QProgressDialog>

#include <iplugininstaller.h>

// define this here as it's going to be used a lot by things using this class' message box wrappers.
template<class T>
T& unused_out(T&& t) { return t; }

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
  void createMod(MOBase::IModInterface*& modInterfaceOut, MOBase::GuessedValue<QString>& modName);

  void displayReadme(const QString& modName, const QString& readme);

  void showWaitDialog(QString message);
  void hideWaitDialog();

protected slots:
  void createModSlot(MOBase::IModInterface*& modInterfaceOut, MOBase::GuessedValue<QString>& modName);
  void displayReadmeSlot(const QString& modName, const QString& readme);
  
  void showWaitDialogSlot(QString message);
  void hideWaitDialogSlot();

private:
  MOBase::IOrganizer* mMoInfo;
  QWidget* mParentWidget;
  QProgressDialog* mWaitDialog;
};
