#pragma once

#include <QCoreApplication>
#include <QMessageBox>

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

  void criticalMessageBox(QMessageBox::StandardButton& standardButtonOut, QWidget* parent, const QString& title, const QString& text, QMessageBox::StandardButtons buttons = QMessageBox::Ok, QMessageBox::StandardButton defaultButton = QMessageBox::NoButton);

  void informationMessageBox(QMessageBox::StandardButton& standardButtonOut, QWidget* parent, const QString& title, const QString& text, QMessageBox::StandardButtons buttons = QMessageBox::Ok, QMessageBox::StandardButton defaultButton = QMessageBox::NoButton);

  void questionMessageBox(QMessageBox::StandardButton& standardButtonOut, QWidget* parent, const QString& title, const QString& text, QMessageBox::StandardButtons buttons = QMessageBox::Yes | QMessageBox::No, QMessageBox::StandardButton defaultButton = QMessageBox::NoButton);

  void warningMessageBox(QMessageBox::StandardButton& standardButtonOut, QWidget* parent, const QString& title, const QString& text, QMessageBox::StandardButtons buttons = QMessageBox::Ok, QMessageBox::StandardButton defaultButton = QMessageBox::NoButton);

protected slots:
  void createModSlot(MOBase::IModInterface*& modInterfaceOut, MOBase::GuessedValue<QString>& modName);

  void displayReadmeSlot(const QString& modName, const QString& readme);

  void criticalMessageBoxSlot(QMessageBox::StandardButton& standardButtonOut, QWidget* parent, const QString& title, const QString& text, QMessageBox::StandardButtons buttons, QMessageBox::StandardButton defaultButton);

  void informationMessageBoxSlot(QMessageBox::StandardButton& standardButtonOut, QWidget* parent, const QString& title, const QString& text, QMessageBox::StandardButtons buttons, QMessageBox::StandardButton defaultButton);

  void questionMessageBoxSlot(QMessageBox::StandardButton& standardButtonOut, QWidget* parent, const QString& title, const QString& text, QMessageBox::StandardButtons buttons, QMessageBox::StandardButton defaultButton);

  void warningMessageBoxSlot(QMessageBox::StandardButton& standardButtonOut, QWidget* parent, const QString& title, const QString& text, QMessageBox::StandardButtons buttons, QMessageBox::StandardButton defaultButton);

private:
  MOBase::IOrganizer* mMoInfo;
  QWidget* mParentWidget;
};
