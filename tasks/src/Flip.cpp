#include <cstring>
#ifdef __unix
#include <stdint.h>
#endif
#include "Flip.h"
#include "Stat.h"
using namespace Tasks;

Flip::Flip() : _mode(FLIP_NONE) {}

Flip::Flip(const Flip &aFlip) : 
  LinkTask(aFlip),_mode(aFlip._mode) 
{}

static void _flip_y_inplace(Data &aSrcData)
{
  int lineSize = aSrcData.dimensions[0] * aSrcData.depth();
  char *aSrcPt = (char*)aSrcData.data();
  char *aDestPt = (char*)aSrcData.data();
  aDestPt += aSrcData.size() - lineSize;

  Buffer *aTmpLineBuffer = new Buffer(lineSize);
  for(int aNbLine = aSrcData.dimensions[1] / 2;aNbLine;
      --aNbLine,aDestPt -= lineSize,aSrcPt += lineSize)
    {
      memcpy(aTmpLineBuffer->data,aDestPt,lineSize);
      memcpy(aDestPt,aSrcPt,lineSize);
      memcpy(aSrcPt,aTmpLineBuffer->data,lineSize);
    }
  aTmpLineBuffer->unref();
}

static void _flip_y(Data &aSrcData,Data &aDestData)
{
  int lineSize = aSrcData.dimensions[0] * aSrcData.depth();
  char *aSrcPt = (char*)aSrcData.data();
  char *aDestPt = (char*)aDestData.data();
  aDestPt += aSrcData.size() - lineSize;
  
  for(int aNbLine = aSrcData.dimensions[1];aNbLine;
      --aNbLine,aDestPt -= lineSize,aSrcPt += lineSize)
    memcpy(aDestPt,aSrcPt,lineSize);
}

template<class INPUT>
inline static void _flip_x_inplace_template(INPUT *aSrcPt,int width, int height)
{
  INPUT *aDestPt = aSrcPt + width - 1;
  for(int aNbLine = height;aNbLine;
      --aNbLine,aDestPt += width + width / 2,aSrcPt += width - width / 2)
    for(int aNbColumn = width / 2 ;aNbColumn;--aNbColumn,--aDestPt,++aSrcPt)
      {
	INPUT aTmpValue = *aDestPt;
	*aDestPt = *aSrcPt;
	*aSrcPt = aTmpValue;
      }
}
#ifndef __unix
typedef signed char int8_t;
typedef signed short int16_t;
typedef signed long int32_t;
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned long uint32_t; 
typedef unsigned long long uint64_t; 
#endif
static void _flip_x_inplace(Data &aSrcData)
{
   switch(aSrcData.depth())
    {
    case 1:
      _flip_x_inplace_template((uint8_t*)aSrcData.data(),
			       aSrcData.dimensions[0],aSrcData.dimensions[1]);
      break;
    case 2:
      _flip_x_inplace_template((uint16_t*)aSrcData.data(),
		       aSrcData.dimensions[0],aSrcData.dimensions[1]);
      break;
    case 4:
      _flip_x_inplace_template((uint32_t*)aSrcData.data(),
			       aSrcData.dimensions[0],aSrcData.dimensions[1]);
      break;
    case 8:
      _flip_x_inplace_template((uint64_t*)aSrcData.data(),
			       aSrcData.dimensions[0],aSrcData.dimensions[1]);
      break;
    }
}
template<class INPUT>
inline static void _flip_x_template(const INPUT *aSrcPt,INPUT *aDestPt,int width, int height)
{
  aDestPt += width - 1;
  for(int aNbLine = height;aNbLine;--aNbLine,aDestPt += (2 * width))
    for(int aNbColumn = width;aNbColumn;--aNbColumn,--aDestPt,++aSrcPt)
      *aDestPt = *aSrcPt;

}
static void _flip_x(Data &aSrcData,Data &aDestData)
{
  switch(aSrcData.depth())
    {
    case 1:
      _flip_x_template((const uint8_t*)aSrcData.data(),(uint8_t*)aDestData.data(),
		       aSrcData.dimensions[0],aSrcData.dimensions[1]);
      break;
    case 2:
      _flip_x_template((const uint16_t*)aSrcData.data(),(uint16_t*)aDestData.data(),
		       aSrcData.dimensions[0],aSrcData.dimensions[1]);
      break;
    case 4:
      _flip_x_template((const uint32_t*)aSrcData.data(),(uint32_t*)aDestData.data(),
		       aSrcData.dimensions[0],aSrcData.dimensions[1]);
      break;
    case 8:
      _flip_x_template((const uint64_t*)aSrcData.data(),(uint64_t*)aDestData.data(),
		       aSrcData.dimensions[0],aSrcData.dimensions[1]);
      break;
    }
}

template<class INPUT>
inline static void _flip_all_inplace_template(INPUT *aSrcPt,int width, int height)
{
  INPUT *aDestPt = aSrcPt + width * height - 1;
  for(int aNbPixel = (height * width) / 2;aNbPixel;
      --aNbPixel,--aDestPt,++aSrcPt)
    {
      INPUT aPixelBuff = *aDestPt;
      *aDestPt = *aSrcPt;
      *aSrcPt = aPixelBuff;
    }
}

static void _flip_all_inplace(Data &aSrcData)
{
   switch(aSrcData.depth())
    {
    case 1:
      _flip_all_inplace_template((uint8_t*)aSrcData.data(),
				 aSrcData.dimensions[0],aSrcData.dimensions[1]);
      break;
    case 2:
      _flip_all_inplace_template((uint16_t*)aSrcData.data(),
				 aSrcData.dimensions[0],aSrcData.dimensions[1]);
      break;
    case 4:
      _flip_all_inplace_template((uint32_t*)aSrcData.data(),
				 aSrcData.dimensions[0],aSrcData.dimensions[1]);
      break;
    case 8:
      _flip_all_inplace_template((uint64_t*)aSrcData.data(),
				 aSrcData.dimensions[0],aSrcData.dimensions[1]);
      break;
    }
}
template<class INPUT>
inline static void _flip_all_template(const INPUT *aSrcPt,INPUT *aDestPt,int width, int height)
{
  aDestPt += width * height - 1;
  for(int aNbPixel = height * width;aNbPixel;
      --aNbPixel,--aDestPt,++aSrcPt)
      *aDestPt = *aSrcPt;
}
static void _flip_all(Data &aSrcData,Data &aDestData)
{
  switch(aSrcData.depth())
    {
    case 1:
      _flip_all_template((const uint8_t*)aSrcData.data(),(uint8_t*)aDestData.data(),
			 aSrcData.dimensions[0],aSrcData.dimensions[1]);
      break;
    case 2:
      _flip_all_template((const uint16_t*)aSrcData.data(),(uint16_t*)aDestData.data(),
			 aSrcData.dimensions[0],aSrcData.dimensions[1]);
      break;
    case 4:
      _flip_all_template((const uint32_t*)aSrcData.data(),(uint32_t*)aDestData.data(),
			 aSrcData.dimensions[0],aSrcData.dimensions[1]);
      break;
    case 8:
      _flip_all_template((const uint64_t*)aSrcData.data(),(uint64_t*)aDestData.data(),
			 aSrcData.dimensions[0],aSrcData.dimensions[1]);
      break;
    }
}

Data Flip::process(Data &aData)
{
  if(aData.nstrip != 1 || aData.dimensions.size() != 2)
    std::cerr << "Flip : Only manage 1 strip 2D data " << std::endl;
  else if(_processingInPlaceFlag)
    {
      switch(_mode)
	{
	case FLIP_X:
	  {
	    Stat aStat(aData,"Flip X");
	    _flip_x_inplace(aData);
	  }
	  break;
	case FLIP_Y:
	  {
	    Stat aStat(aData,"Flip Y");
	    _flip_y_inplace(aData);
	  }
	  break;
	case FLIP_ALL:
	  {
	    Stat aStat(aData,"Flip X&Y");
	    _flip_all_inplace(aData);
	  }
	  break;
	default:
	  break;
	}
      return aData;
    }
  else
    {
      Data aNewData = aData.copyHeader(aData.type);
      switch(_mode)
	{
	case FLIP_X:
	  {
	    Stat aStat(aData,"Flip X");
	    _flip_x(aData,aNewData);
	  }
	  break;
	case FLIP_Y:
	  {
	    Stat aStat(aData,"Flip Y");
	    _flip_y(aData,aNewData);
	  }
	  break;
	case FLIP_ALL:
	  {
	    Stat aStat(aData,"Flip X&Y");
	    _flip_all(aData,aNewData);
	  }
	  break;
	default:
	  memcpy(aNewData.data(),aData.data(),aData.size());
	  break;
	}
      return aNewData;
    }
}

void Flip::setFlip(FLIP_MODE aMode)
{
  _mode = aMode;
}
