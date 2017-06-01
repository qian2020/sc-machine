/*
 * This source file is part of an OSTIS project. For the latest info, see http://ostis.net
 * Distributed under the MIT License
 * (See accompanying file COPYING.MIT or copy at http://opensource.org/licenses/MIT)
 */

#pragma once

#include <exception>
#include <string>
#include <assert.h>

#include "sc_debug.hpp"
#include "sc_types.hpp"


// Got it there: https://github.com/mapsme/omim/blob/136f12af3adde05623008f71d07bb996fe5801a5/base/macros.hpp
#define ARRAY_SIZE(X) sizeof(::my::impl::ArraySize(X))

#define SC_DISALLOW_COPY(className)                             \
  className(className const &) = delete;                        \
  className & operator=(className const &) = delete


#define SC_DISALLOW_MOVE(className)                             \
  className(className &&) = delete;                             \
  className & operator=(className &&) = delete

#define SC_DISALLOW_COPY_AND_MOVE(className)                    \
  SC_DISALLOW_COPY(className);                                  \
  SC_DISALLOW_MOVE(className)

// ---------------- Reference counter -----------
class RefCount
{
public:
  RefCount()
    : m_refCount(0)
  {
  }

  inline void Ref()
  {
    ++m_refCount;
  }

  inline sc_uint32 Unref()
  {
    SC_ASSERT(m_refCount > 0, ());
    --m_refCount;

    return m_refCount;
  }

private:
  sc_uint32 m_refCount;
};

// ------------ Shared pointer -------------
template<typename ObjectType>
class TSharedPointer
{
public:
  TSharedPointer()
  {
    m_object = 0;
    InitRef();
  }

  TSharedPointer(ObjectType * object)
  {
    m_object = object;
    InitRef();
  }

  ~TSharedPointer()
  {
    Clear();
  }

  TSharedPointer(TSharedPointer const & other)
  {
    m_object = other.m_object;
    m_refCount = other.m_refCount;
    m_refCount->Ref();
  }

  template <typename OtherType>
  TSharedPointer(TSharedPointer<OtherType> const & other)
  {
    m_object = other.m_object;
    m_refCount = other.m_refCount;
    m_refCount->Ref();
  }

  TSharedPointer & operator = (TSharedPointer const & other)
  {
    Clear();
    m_object = other.m_object;
    m_refCount = other.m_refCount;
    m_refCount->Ref();

    return *this;
  }

  ObjectType & operator = (ObjectType * object)
  {
    Clear();
    m_object = object;
    InitRef();
    return *m_object;
  }

  ObjectType & operator * () const
  {
    SC_ASSERT(IsPtrValid(), ());
    return *m_object;
  }

  ObjectType * operator -> () const
  {
    SC_ASSERT(IsPtrValid(), ());
    return m_object;
  }

  inline bool IsPtrValid() const
  {
    return m_object != 0;
  }

  inline ObjectType * GetPtr() const
  {
    return m_object;
  }

  inline ObjectType & GetRef() const
  {
    SC_ASSERT(m_object != nullptr, ("Pointer is not valid"));
    return *m_object;
  }

private:

  void InitRef()
  {
    m_refCount = new RefCount();
    m_refCount->Ref();
  }

  void Clear()
  {
    if (m_refCount->Unref() == 0)
    {
      delete m_refCount;
      delete m_object;
    }

    m_refCount = 0;
    m_object = 0;
  }


protected:
  ObjectType * m_object;
  RefCount * m_refCount;
};

#define SHARED_PTR_TYPE(__type) typedef TSharedPointer< __type > __type##Ptr;

class MemoryBuffer
{
public:
  MemoryBuffer(char * buff, unsigned int size)
    : m_data(buff)
    , m_size(size)
  {
  }

  inline bool IsValid() const { return m_data != nullptr; }

  void * Data() { return (void*)m_data; }
  void const * CData() const { return (void const*)m_data; }
  size_t Size() const { return m_size;  }

  size_t Read(void * buff, size_t size) const
  {
    size_t const read = std::min(size, m_size);
    memcpy(buff, m_data, read);
    return read;
  }

protected:
  MemoryBuffer()
    : m_data(nullptr)
    , m_size(0)
  {
  }

  char * m_data;
  size_t m_size;
};

class MemoryBufferSafe : public MemoryBuffer
{
public:
  MemoryBufferSafe() : MemoryBuffer()
  {
  }

  MemoryBufferSafe(char const * buff, size_t size)
  {
    Reinit(buff, size);
  }

  ~MemoryBufferSafe()
  {
    Clear();
  }

  void Reinit(size_t size)
  {
    Clear();
    m_data = new char[size];
    m_size = size;
  }

  void Reinit(char const * buff, size_t size)
  {
    m_data = new char[size];
    m_size = size;
    memcpy(m_data, buff, size);
  }

  void Clear()
  {
    if (m_data)
      delete[] m_data;
    m_data = nullptr;
    m_size = 0;
  }
};

SHARED_PTR_TYPE(MemoryBuffer)
SHARED_PTR_TYPE(MemoryBufferSafe)


namespace utils
{

class StringUtils
{
public:
  _SC_EXTERN static void ToLowerCase(std::string & str);
  _SC_EXTERN static void ToUpperCase(std::string & str);

  _SC_EXTERN static bool StartsWith(std::string const & str, std::string const & pattern, bool lowerCase);
  _SC_EXTERN static bool EndsWith(std::string const & str, std::string const & pattern, bool lowerCase);

  _SC_EXTERN static void SplitFilename(std::string const & qualifiedName, std::string & outBasename, std::string & outPath);
  _SC_EXTERN static void SplitString(std::string const & str, char delim, StringVector & outList);

  _SC_EXTERN static void TrimLeft(std::string & str);
  _SC_EXTERN static void TrimRight(std::string & str);
  _SC_EXTERN static void Trim(std::string & str);

  _SC_EXTERN static std::string GetFileExtension(std::string const & filename);
  _SC_EXTERN static std::string NormalizeFilePath(std::string const & init, bool makeLowerCase);

  _SC_EXTERN static std::string ReplaceAll(std::string const & source, std::string const & replaceWhat, std::string const & replaceWithWhat);
};

class Random
{
public:
  _SC_EXTERN static int Int();
};

} // namespace utils