#include <stdlib.h>
#include <iostream>
#include <pthread.h>
#include <map>
#include <vector>
#include <string>
#include <cstring>

#include "Compatibility.h"

#ifndef __DATA_H
#define __DATA_H
struct DLL_EXPORT Buffer
{
  enum Ownership {MAPPED,SHARED};
  ~Buffer()
  {
    pthread_mutex_destroy(&_lock);
  }
  Buffer() : owner(SHARED),refcount(1),data(NULL) 
  {
    pthread_mutex_init(&_lock,NULL);
  }
  explicit Buffer(int aSize) :owner(SHARED),refcount(1)
  {
#ifdef __unix
    if(posix_memalign(&data,16,aSize))
#else  /* window */
    data = _aligned_malloc(aSize,16);
    if(!data)
#endif
      std::cerr << "Can't allocate memory" << std::endl;
    pthread_mutex_init(&_lock,NULL);
  }
  void unref()
  {
    while(pthread_mutex_lock(&_lock)) ;
    if(!(--refcount))
      {
	if(owner == SHARED && data)
	  free(data);
	pthread_mutex_unlock(&_lock);
	delete this;
      }
    else
      pthread_mutex_unlock(&_lock);
  }
  void ref()
  {
    while(pthread_mutex_lock(&_lock)) ;
    ++refcount;
    pthread_mutex_unlock(&_lock);
  }
  Ownership		owner;
  volatile int          refcount;
  void			*data;
  pthread_mutex_t	_lock;
};
struct DLL_EXPORT Data
{
  class DLL_EXPORT HeaderContainer
  {
  public:
    typedef std::map<std::string,std::string> Header;
    
    HeaderContainer();
    HeaderContainer(const HeaderContainer &cnt);
    ~HeaderContainer();
    
    void insert(const char *key,const char *value);
    void insertOrIncKey(const std::string &key,const std::string &value);
    void erase(const char *key);
    void clear();

    const char* get(const char *key,const char *defaultValue = NULL) const;
    int size() const;

    const char* operator[](const char *aKey) const {return get(aKey);}

    HeaderContainer& operator=(const HeaderContainer&);

    // ExpertMethodes for macro insertion a loop
    void lock();
    void unlock();
    pthread_mutex_t* mutex() const;
    Header& header();
    const Header& header() const;
  private:
    struct HeaderHolder;
    HeaderHolder *_header;
  };
  enum TYPE {UNDEF,UINT8,INT8,UINT16,INT16,UINT32,INT32,UINT64,INT64,FLOAT,DOUBLE};
  Data() : type(UNDEF),frameNumber(-1),timestamp(0.),buffer(NULL) {}
  Data(const Data &aData) : buffer(NULL)
  {
    *this = aData;
  }
  ~Data() 
  {
    if(buffer)
      buffer->unref();
  }
  void setData(Data &aData)
  {
    if(buffer) buffer->unref();
    if(aData.buffer) aData.buffer->ref();
    buffer = aData.buffer;
    type = aData.type;
    dimensions = aData.dimensions;
  }
  void setBuffer(Buffer *aBuffer)
  {
    if(aBuffer) aBuffer->ref();
    if(buffer) buffer->unref();
    buffer = aBuffer;
  }
  void releaseBuffer() 
  {
    if(buffer) buffer->unref();
    buffer = NULL;
    type = UNDEF,frameNumber = -1;
    dimensions.clear();
  }
  inline void * data() {return buffer ? buffer->data : NULL;}
  inline const void * data() const {return buffer ? buffer->data : NULL;}
  inline int size() const 
  {
    int returnSize = depth();
    for(std::vector<int>::const_iterator i = dimensions.begin();
	i != dimensions.end();++i)
      returnSize *= *i;
    return returnSize;
  }
  inline int depth() const
  {
    int depth;
    switch(type)
      {
      case UINT8:
      case INT8:
	depth = 1;break;
      case UINT16:
      case INT16:
	depth = 2;break;
      case UINT32:
      case INT32:
	depth = 4;break;
      case UINT64:
      case INT64:
	depth = 8;break;
      case FLOAT: depth = 4;break;
      case DOUBLE: depth = 8;break;
      default: 
	depth = 0;break;
      }
    return depth;
  }
  inline bool is_signed() const
  {
    bool signedFlag;
    switch(type)
      {
      case UINT8:
      case UINT16:
      case UINT32:
      case UINT64:
	signedFlag = false;break;
      default:
	signedFlag = true;break;
      }
    return signedFlag;
  }

  inline bool empty() const { return buffer ? !buffer->data : true;}
  //@brief return a data with the same header but with different buffer.
  inline Data copyHeader(TYPE aType)
  {
    Data aReturnData;
    aReturnData.type = aType;
    aReturnData.dimensions = dimensions;
    aReturnData.frameNumber = frameNumber;
    aReturnData.timestamp = timestamp;
    aReturnData.header = header;
    aReturnData.buffer = new Buffer(aReturnData.size());
    return aReturnData;
  }
  inline Data copy() const
  {
    Data aReturnData;
    aReturnData.type = type;
    aReturnData.dimensions = dimensions;
    aReturnData.frameNumber = frameNumber;
    aReturnData.timestamp = timestamp;
    aReturnData.header = header;
    if(buffer)
      {
	aReturnData.buffer = new Buffer(size());
	memcpy(aReturnData.data(),data(),size());
      }
    else
      aReturnData.buffer = NULL;
    return aReturnData;
  }
  inline Data& operator=(const Data &aData)
  {
    type = aData.type;
    dimensions = aData.dimensions;
    frameNumber = aData.frameNumber;
    timestamp = aData.timestamp;
    header    = aData.header;
    setBuffer(aData.buffer);
    return *this;
  }
  TYPE      			type;
  std::vector<int> 		dimensions;
  int       			frameNumber;
  double    			timestamp;
  mutable HeaderContainer 	header;
  mutable Buffer *		buffer;
};

DLL_EXPORT std::ostream& operator<<(std::ostream &os,
				    const Data::HeaderContainer &aHeader);

inline std::ostream& operator<<(std::ostream &os,const Buffer &aBuffer)
{
  const char *anOwnerShip = (aBuffer.owner == 
			     Buffer::MAPPED) ? "Mapped" : "Shared";
  os << "<"
     << "owner=" << anOwnerShip << ", "
     << "refcount=" << aBuffer.refcount << ", "
     << "data=" << aBuffer.data 
     << ">";
  return os;
}

inline std::ostream& operator<<(std::ostream &os,const Data &aData)
{
  os << "<"
     << "type=" << aData.type << ", ";

  int dimNb = 0;
  for(std::vector<int>::const_iterator i = aData.dimensions.begin();
      i != aData.dimensions.end();++i)
    os << "dimension_" << dimNb++ << "=" << *i << ", ";

  os << "frameNumber=" << aData.frameNumber << ", "
     << "timestamp=" << aData.timestamp << ", "
     << "header=" << aData.header << ", ";
  if(aData.buffer)
    os << "buffer=" << *aData.buffer;
  else
    os << "buffer=" << "NULL";
  os << ">";
  return os;
}


#endif
