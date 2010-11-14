/*

Copyright (C) 2010  Gaetan Guidet

This file is part of nukej2k.

gcore is free software; you can redistribute it and/or modify it
under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation; either version 2.1 of the License, or (at
your option) any later version.

gcore is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
USA.

*/

#include "j2kReader.h"

j2kReader::j2kReader(DD::Image::Read *iop)
  : DD::Image::Reader(iop), mFIF(FIF_UNKNOWN), mBitmap(0), mPath("")
{
  mPath = iop->filename();
  
   mFIF = FreeImage_GetFileType(iop->filename());
  
  if (mFIF == FIF_UNKNOWN)
  {
    mFIF = FreeImage_GetFIFFromFilename(iop->filename());
  }
  if (mFIF != FIF_UNKNOWN && FreeImage_FIFSupportsReading(mFIF))
  {
    FIBITMAP *tmp = 0;
    bool noPixels = false;
    
    if (FreeImage_FIFSupportsNoPixels(mFIF))
    {
      tmp = FreeImage_Load(mFIF, iop->filename(), FIF_LOAD_NOPIXELS);
      noPixels = true;
    }
    else
    {
      tmp = FreeImage_Load(mFIF, iop->filename());
    }
    
    if (tmp)
    {
      mWidth = FreeImage_GetWidth(tmp);
      mHeight = FreeImage_GetHeight(tmp);
      mBPP = FreeImage_GetBPP(tmp) / 8;
      mFIT = FreeImage_GetImageType(tmp);
      
      if (mBPP > 0)
      {
        switch (mFIT)
        {
        case FIT_BITMAP:
          mChannelBytes = 1;
          mNumChannels = mBPP;
          break;
        case FIT_INT16:
        case FIT_UINT16:
        case FIT_RGB16:
        case FIT_RGBA16:
          mChannelBytes = 2;
          mNumChannels = mBPP / 2;
          break;
        case FIT_FLOAT:
        case FIT_RGBF:
        case FIT_RGBAF:
          mChannelBytes = 4;
          mNumChannels = mBPP / 4;
          break;
        case FIT_DOUBLE:
        case FIT_UINT32:
        case FIT_INT32:
        default:
          mFIF = FIF_UNKNOWN;
          break;
        }
        
        if (mFIF != FIF_UNKNOWN)
        {
          set_info((int)mWidth, (int)mHeight, (int)mBPP);
        }
      }
      else
      {
        mFIF = FIF_UNKNOWN;
      }
      
      if (mFIF != FIF_UNKNOWN)
      {
        if (noPixels)
        {
          FreeImage_Unload(tmp);
        }
        else
        {
          // avoid re-reading image pixels in open()
          mBitmap = tmp;
        }
      }
    }
    else
    {
      mFIF = FIF_UNKNOWN;
    }
  }
  else
  {
    mFIF = FIF_UNKNOWN;
  }
}

j2kReader::~j2kReader()
{
  if (mBitmap)
  {
    FreeImage_Unload(mBitmap);
    mBitmap = 0;
  }
}

void j2kReader::open()
{
  if (mFIF != FIF_UNKNOWN)
  {
    if (!mBitmap)
    {
      mBitmap = FreeImage_Load(mFIF, mPath.c_str());
    }
  }
}

void j2kReader::engine(int y, int x, int r, DD::Image::ChannelMask mask, DD::Image::Row &row)
{
  if (mFIF != FIF_UNKNOWN)
  {
    if (!mBitmap)
    {
      mBitmap = FreeImage_Load(mFIF, mPath.c_str());
    }
    
    int sb = (mFIT == FIT_INT16 ? 15 : 16);
    int count = r - x;
    
    BYTE *bytes = FreeImage_GetScanLine(mBitmap, y);
    bytes += x * mBPP;
    
    // Channels a in BGR order
    //-> use mRed = FreeImage_GetChannel(mBitmap, FICC_RED); ?
    
    BYTE *alpha = 0;
    if (mNumChannels == 2)
    {
      alpha = bytes + mChannelBytes;
    }
    else if (mNumChannels == 4)
    {
      alpha = bytes + 3 * mChannelBytes;
    }
    
    if (mNumChannels >= 1 && (mask & DD::Image::Mask_Red))
    {
      float *dst = row.writable(DD::Image::Chan_Red);
      if (dst)
      {
        BYTE *src = bytes;
        if (mNumChannels >= 3)
        {
          src += 2 * mChannelBytes;
        }
        switch (mChannelBytes)
        {
        case 1:
          from_byte(DD::Image::Chan_Red, dst+x, (const uchar*)src, (const uchar*)alpha, count, mBPP);
          break;
        case 2:
          from_short(DD::Image::Chan_Red, dst+x, (const U16*)src, (const U16*)alpha, count, sb, mBPP);
          break;
        case 4:
          from_float(DD::Image::Chan_Red, dst+x, (const float*)src, (const float*)alpha, count, mBPP);
          break;
        default:
          break;
        }
      }
    }
    
    if (mNumChannels >= 3 && (mask & DD::Image::Mask_Green))
    {
      float *dst = row.writable(DD::Image::Chan_Green);
      if (dst)
      {
        BYTE *src = bytes + mChannelBytes;
        switch (mChannelBytes)
        {
        case 1:
          from_byte(DD::Image::Chan_Green, dst+x, (const uchar*)src, (const uchar*)alpha, count, mBPP);
          break;
        case 2:
          from_short(DD::Image::Chan_Green, dst+x, (const U16*)src, (const U16*)alpha, count, sb, mBPP);
          break;
        case 4:
          from_float(DD::Image::Chan_Green, dst+x, (const float*)src, (const float*)alpha, count, mBPP);
          break;
        default:
          break;
        }
      }
    }
    
    if (mNumChannels >= 3 && (mask & DD::Image::Mask_Blue))
    {
      float *dst = row.writable(DD::Image::Chan_Blue);
      if (dst)
      {
        BYTE *src = bytes;
        switch (mChannelBytes)
        {
        case 1:
          from_byte(DD::Image::Chan_Blue, dst+x, (const uchar*)src, (const uchar*)alpha, count, mBPP);
          break;
        case 2:
          from_short(DD::Image::Chan_Blue, dst+x, (const U16*)src, (const U16*)alpha, count, sb, mBPP);
          break;
        case 4:
          from_float(DD::Image::Chan_Blue, dst+x, (const float*)src, (const float*)alpha, count, mBPP);
          break;
        default:
          break;
        }
      }
    }
    
    if ((alpha != 0) && (mask & DD::Image::Mask_Alpha))
    {
      float *dst = row.writable(DD::Image::Chan_Alpha);
      if (dst)
      {
        switch (mChannelBytes)
        {
        case 1:
          from_byte(DD::Image::Chan_Alpha, dst+x, (const uchar*)alpha, 0, count, mBPP);
          break;
        case 2:
          from_short(DD::Image::Chan_Alpha, dst+x, (const U16*)alpha, 0, count, sb, mBPP);
          break;
        case 4:
          from_float(DD::Image::Chan_Alpha, dst+x, (const float*)alpha, 0, count, mBPP);
          break;
        default:
          break;
        }
      }
    }
  }
}


static bool test(int fd, const unsigned char* block, int length)
{
  BYTE jpc_signature[] = {0xFF, 0x4F};
  return (memcmp(jpc_signature, block, sizeof(jpc_signature)) == 0);
}

static DD::Image::Reader* build(DD::Image::Read* iop, int fd, const unsigned char* b, int n)
{
  return new j2kReader(iop);
}

const DD::Image::Reader::Description j2kReader::Desc("j2k\0", build, test);

