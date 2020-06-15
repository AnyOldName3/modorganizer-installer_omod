#pragma once

using namespace cli;

ref class CodeProgress : OMODFramework::ICodeProgress
{
public:
  virtual void Init(__int64 totalSize, bool compressing);

  virtual void SetProgress(__int64 inSize, __int64 outSize);

  ~CodeProgress();

private:
  __int64 mTotalSize;
  bool mCompressing;
};
