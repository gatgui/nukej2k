#include "fiReader.h"

static BYTE Sig[] = {0x00, 0x00, 0x00, 0x0C, 0x6A, 0x50, 0x20, 0x20, 0x0D, 0x0A, 0x87, 0x0A};

bool Test(int fd, const unsigned char* block, int length)
{
  return (memcmp(Sig, block, sizeof(Sig)) == 0);
}

DD::Image::Reader* Build(DD::Image::Read* iop, int fd, const unsigned char* b, int n)
{
  return new FIReader(iop);
}

const DD::Image::Reader::Description jp2ReaderDesc("jp2\0", Build, Test);
