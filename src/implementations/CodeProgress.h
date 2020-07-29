#pragma once

#include <QObject>
#include <QWidget>
#include <QProgressDialog>

#include "../QObject_unique_ptr.h"

using namespace cli;

class CodeProgressHelper : public QObject {
  Q_OBJECT
public:

  CodeProgressHelper(QWidget* parentWidget);

  void ShowProgressDialog(__int64 totalSize);
  void UpdateProgressValue(__int64 size);
  void HideProgressDialog();

public slots:
  void ShowProgressDialogSlot(__int64 totalSize);
  void UpdateProgressValueSlot(__int64 size);
  void HideProgressDialogSlot();

signals:

  void ShowProgressDialogSignal(__int64 totalSize);
  void UpdateProgressValueSignal(__int64 size);
  void HideProgressDialogSignal();

private:
  QWidget* mParentWidget;
  QObject_unique_ptr<QProgressDialog> mProgressDialog;
  __int64 mTotalSize;
};

ref class CodeProgress : OMODFramework::ICodeProgress
{
public:

  CodeProgress(CodeProgressHelper *helper) : mHelper(helper) { }

  virtual void Init(__int64 totalSize, bool compressing);

  virtual void SetProgress(__int64 inSize, __int64 outSize);

  ~CodeProgress();

private:
  CodeProgressHelper* mHelper;
  __int64 mTotalSize;
  bool mCompressing;
};
