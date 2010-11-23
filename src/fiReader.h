#ifndef __fiReader_h__
#define __fiReader_h__

#include <DDImage/Reader.h>
#include <DDImage/Row.h>
#include <DDImage/ARRAY.h>
#include <DDImage/Thread.h>
#include <FreeImage.h>
#include <string>

#if FREEIMAGE_MAJOR_VERSION >= 3 && FREEIMAGE_MINOR_VERSION >= 14
# define FI_HEADER_ONLY_API
#endif

class FIReader : public DD::Image::Reader
{
public:
  
  FIReader(DD::Image::Read *iop)
    : DD::Image::Reader(iop), mFIF(FIF_UNKNOWN), mBitmap(0), mPath("")
  {
    mPath = iop->filename();
    
    mFIF = FreeImage_GetFileType(iop->filename());
    
    if (mFIF == FIF_UNKNOWN)
    {
      mFIF = FreeImage_GetFIFFromFilename(iop->filename());
    }
#ifdef FI_HEADER_ONLY_API
    if (mFIF != FIF_UNKNOWN && FreeImage_FIFSupportsReading(mFIF))
#else
    if (mFIF != FIF_UNKNOWN)
#endif
    {
      FIBITMAP *tmp = 0;
      bool noPixels = false;
      
#ifdef FI_HEADER_ONLY_API
      if (FreeImage_FIFSupportsNoPixels(mFIF))
      {
        tmp = FreeImage_Load(mFIF, iop->filename(), FIF_LOAD_NOPIXELS);
        noPixels = true;
      }
      else
      {
        tmp = FreeImage_Load(mFIF, iop->filename());
      }
#else
      tmp = FreeImage_Load(mFIF, iop->filename());
#endif
      
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
            set_info((int)mWidth, (int)mHeight, (int)mNumChannels);
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
  
  virtual ~FIReader()
  {
    if (mBitmap)
    {
      FreeImage_Unload(mBitmap);
      mBitmap = 0;
    }
  }
  
  virtual void open()
  {
    if (mFIF != FIF_UNKNOWN)
    {
      if (!mBitmap)
      {
        mBitmap = FreeImage_Load(mFIF, mPath.c_str());
      }
    }
  }
  
  virtual void engine(int y, int x, int r, DD::Image::ChannelMask mask, DD::Image::Row &row)
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
      
      // not generic enough, use FreeImage_GetChannel?
      //                     -> no, works only with bitmaps type image
      
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
          if (mNumChannels >= 3 && mChannelBytes == 1)
          {
            // BGR for 24/32 bit images
            src += 2 * mChannelBytes;
          }
          switch (mChannelBytes)
          {
          case 1:
            from_byte(DD::Image::Chan_Red, dst+x, (const uchar*)src, (const uchar*)alpha, count, mNumChannels);
            break;
          case 2:
            from_short(DD::Image::Chan_Red, dst+x, (const U16*)src, (const U16*)alpha, count, sb, mNumChannels);
            break;
          case 4:
            from_float(DD::Image::Chan_Red, dst+x, (const float*)src, (const float*)alpha, count, mNumChannels);
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
            from_byte(DD::Image::Chan_Green, dst+x, (const uchar*)src, (const uchar*)alpha, count, mNumChannels);
            break;
          case 2:
            from_short(DD::Image::Chan_Green, dst+x, (const U16*)src, (const U16*)alpha, count, sb, mNumChannels);
            break;
          case 4:
            from_float(DD::Image::Chan_Green, dst+x, (const float*)src, (const float*)alpha, count, mNumChannels);
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
            // BGR for 24/32 bits images
            from_byte(DD::Image::Chan_Blue, dst+x, (const uchar*)src, (const uchar*)alpha, count, mNumChannels);
            break;
          case 2:
            src += 2 * mChannelBytes;
            from_short(DD::Image::Chan_Blue, dst+x, (const U16*)src, (const U16*)alpha, count, sb, mNumChannels);
            break;
          case 4:
            src += 2 * mChannelBytes;
            from_float(DD::Image::Chan_Blue, dst+x, (const float*)src, (const float*)alpha, count, mNumChannels);
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
            from_byte(DD::Image::Chan_Alpha, dst+x, (const uchar*)alpha, 0, count, mNumChannels);
            break;
          case 2:
            from_short(DD::Image::Chan_Alpha, dst+x, (const U16*)alpha, 0, count, sb, mNumChannels);
            break;
          case 4:
            from_float(DD::Image::Chan_Alpha, dst+x, (const float*)alpha, 0, count, mNumChannels);
            break;
          default:
            break;
          }
        }
      }
    }
  }
  
protected:
  
  FREE_IMAGE_FORMAT mFIF;
  FIBITMAP *mBitmap;
  std::string mPath;
  FREE_IMAGE_TYPE mFIT;
  unsigned int mWidth;
  unsigned int mHeight;
  unsigned int mBPP;
  unsigned int mNumChannels;
  unsigned int mChannelBytes;
};

#endif
