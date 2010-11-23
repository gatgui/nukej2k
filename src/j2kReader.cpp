#include "fiReader.h"

static BYTE Sig[] = {0xFF, 0x4F};

bool Test(int fd, const unsigned char* block, int length)
{
  return (memcmp(Sig, block, sizeof(Sig)) == 0);
}

DD::Image::Reader* Build(DD::Image::Read* iop, int fd, const unsigned char* b, int n)
{
  return new FIReader(iop);
}

const DD::Image::Reader::Description j2kReaderDesc("j2k\0", Build, Test);
