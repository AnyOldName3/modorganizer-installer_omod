#include <QApplication>

#include "CodeProgress.h"

#include <log.h>

CodeProgressHelper::CodeProgressHelper(QWidget* parentWidget) : mParentWidget{ parentWidget }, mProgressDialog(make_nullptr<QProgressDialog>()) {
  moveToThread(QApplication::instance()->thread());
  connect(this, &CodeProgressHelper::ShowProgressDialogSignal, this, &CodeProgressHelper::ShowProgressDialogSlot, Qt::QueuedConnection);
  connect(this, &CodeProgressHelper::UpdateProgressValueSignal, this, &CodeProgressHelper::UpdateProgressValueSlot, Qt::QueuedConnection);
  connect(this, &CodeProgressHelper::HideProgressDialogSignal, this, &CodeProgressHelper::HideProgressDialogSlot, Qt::QueuedConnection);

}

void CodeProgressHelper::ShowProgressDialog(__int64 totalSize) {
  emit ShowProgressDialogSignal(totalSize);
}

void CodeProgressHelper::UpdateProgressValue(__int64 size) {
  emit UpdateProgressValueSignal(size);
}

void CodeProgressHelper::HideProgressDialog() {
  emit HideProgressDialogSignal();
}

void CodeProgressHelper::ShowProgressDialogSlot(__int64 totalSize) {
  mTotalSize = totalSize;

  mProgressDialog.reset(new QProgressDialog(mParentWidget));
  mProgressDialog->setWindowFlags(mProgressDialog->windowFlags() & ~Qt::WindowContextHelpButtonHint & ~Qt::WindowCloseButtonHint);
  mProgressDialog->setWindowModality(Qt::WindowModal);
  mProgressDialog->setCancelButton(nullptr);
  mProgressDialog->setMinimum(0);
  mProgressDialog->setMaximum(100);
  mProgressDialog->setAutoReset(false);
  mProgressDialog->setAutoClose(false);
  mProgressDialog->show();
}

void CodeProgressHelper::UpdateProgressValueSlot(__int64 size) {
  int percent = (int)(100 * size / (double)mTotalSize);
  if (percent != mProgressDialog->value()) {
    mProgressDialog->setValue(percent);
  }
}

void CodeProgressHelper::HideProgressDialogSlot() {
  mProgressDialog->hide();
  mProgressDialog.reset();
 }

void CodeProgress::Init(__int64 totalSize, bool compressing)
{
  mTotalSize = totalSize;
  mCompressing = compressing;

  mHelper->ShowProgressDialog(totalSize);
  MOBase::log::debug("CodeProgress::Init called with {} {}", totalSize, compressing);

  // TODO: this
}

void CodeProgress::SetProgress(__int64 inSize, __int64 outSize)
{
  // MOBase::log::debug("CodeProgress::SetProgress called with {} {}", inSize, outSize);
  mHelper->UpdateProgressValue(inSize);
}

CodeProgress::~CodeProgress()
{
  MOBase::log::debug("CodeProgress 'destructor' called");
  mHelper->HideProgressDialog();
}
