#include "CodeProgress.h"

#include <log.h>

void CodeProgress::Init(__int64 totalSize, bool compressing)
{
  mTotalSize = totalSize;
  mCompressing = compressing;
  MOBase::log::debug("CodeProgress::Init called with {} {}", totalSize, compressing);
}

void CodeProgress::SetProgress(__int64 inSize, __int64 outSize)
{
  MOBase::log::debug("CodeProgress::SetProgress called with {} {}", inSize, outSize);
}

CodeProgress::~CodeProgress()
{
  MOBase::log::debug("CodeProgress destructor called");
}
