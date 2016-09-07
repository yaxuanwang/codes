/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2013-2016 Regents of the University of California.
 *
 * This file is part of ndn-cxx library (NDN C++ library with eXperimental eXtensions).
 *
 * ndn-cxx library is free software: you can redistribute it and/or modify it under the
 * terms of the GNU Lesser General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later version.
 *
 * ndn-cxx library is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more details.
 *
 * You should have received copies of the GNU General Public License and GNU Lesser
 * General Public License along with ndn-cxx, e.g., in COPYING.md file.  If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * See AUTHORS.md for complete list of ndn-cxx authors and contributors.
 *
 * @author Alexander Afanasyev <http://lasr.cs.ucla.edu/afanasyev/index.html>
 */

#include "block.hpp"
#include "block-helpers.hpp"

#include "tlv.hpp"
#include "encoding-buffer.hpp"
#include "buffer-stream.hpp"

#include <boost/lexical_cast.hpp>
#include <boost/asio/buffer.hpp>

namespace ndn {

#if NDN_CXX_HAVE_IS_NOTHROW_MOVE_CONSTRUCTIBLE  //not to throw any exception.
static_assert(std::is_nothrow_move_constructible<Block>::value,
              "Block must be MoveConstructible with noexcept");
#endif // NDN_CXX_HAVE_IS_NOTHROW_MOVE_CONSTRUCTIBLE

#if NDN_CXX_HAVE_IS_NOTHROW_MOVE_ASSIGNABLE
static_assert(std::is_nothrow_move_assignable<Block>::value,
              "Block must be MoveAssignable with noexcept");
#endif // NDN_CXX_HAVE_IS_NOTHROW_MOVE_ASSIGNABLE

const size_t MAX_SIZE_OF_BLOCK_FROM_STREAM = MAX_NDN_PACKET_SIZE; //从流中来的最大block的size是最大包的size

Block::Block()   //create an empty Block
  : m_type(std::numeric_limits<uint32_t>::max()) //返回机器最大允许的四字节数
{
}

Block::Block(const EncodingBuffer& buffer)  //Create a Block based on EncodingBuffer object
  : m_buffer(const_cast<EncodingBuffer&>(buffer).getBuffer())
  , m_begin(buffer.begin())
  , m_end(buffer.end())
  , m_size(m_end - m_begin)
{
  m_value_begin = m_begin;
  m_value_end   = m_end;

  m_type = tlv::readType(m_value_begin, m_value_end);
  uint64_t length = tlv::readVarNumber(m_value_begin, m_value_end);
  if (length != static_cast<uint64_t>(m_value_end - m_value_begin))
    {
      BOOST_THROW_EXCEPTION(tlv::Error("TLV length doesn't match buffer length"));
    }
}

Block::Block(const ConstBufferPtr& wire,  //Create a Block from the wire buffer (no parsing) This overload does not do any parsing(此重载不做任何解析)
             uint32_t type,
             const Buffer::const_iterator& begin, const Buffer::const_iterator& end,
             const Buffer::const_iterator& valueBegin, const Buffer::const_iterator& valueEnd)
  : m_buffer(wire)
  , m_type(type)
  , m_begin(begin)
  , m_end(end)
  , m_size(m_end - m_begin)
  , m_value_begin(valueBegin)
  , m_value_end(valueEnd)
{
}

Block::Block(const ConstBufferPtr& buffer)  //Create a Block from the raw buffer with Type-Length parsing,原始的buffer，no value parsing?
  : m_buffer(buffer)
  , m_begin(m_buffer->begin())
  , m_end(m_buffer->end())
  , m_size(m_end - m_begin)
{
  m_value_begin = m_begin;
  m_value_end   = m_end;

  m_type = tlv::readType(m_value_begin, m_value_end);

  uint64_t length = tlv::readVarNumber(m_value_begin, m_value_end);
  if (length != static_cast<uint64_t>(m_value_end - m_value_begin))
    {
      BOOST_THROW_EXCEPTION(tlv::Error("TLV length doesn't match buffer length"));
    }
}

Block::Block(const ConstBufferPtr& buffer,   //Create a Block from a buffer, directly specifying boundaries of the block within the buffer,detect type and position of the value within the block
             const Buffer::const_iterator& begin, const Buffer::const_iterator& end,
             bool verifyLength/* = true*/)
  : m_buffer(buffer)
  , m_begin(begin)
  , m_end(end)
  , m_size(m_end - m_begin)
{
  m_value_begin = m_begin;
  m_value_end   = m_end;

  m_type = tlv::readType(m_value_begin, m_value_end);
  uint64_t length = tlv::readVarNumber(m_value_begin, m_value_end);
  if (verifyLength) {
    if (length != static_cast<uint64_t>(std::distance(m_value_begin, m_value_end))) {
      BOOST_THROW_EXCEPTION(tlv::Error("TLV length doesn't match buffer length"));
    }
  }
}

Block::Block(const Block& block,   // Create a Block from existing block (reusing the underlying buffer)
             const Buffer::const_iterator& begin, const Buffer::const_iterator& end,
             bool verifyLength/* = true*/)
  : m_buffer(block.m_buffer) //*********
  , m_begin(begin) //
  , m_end(end)
  , m_size(m_end - m_begin)
{
  if (!(m_buffer->begin() <= begin && begin <= m_buffer->end()) ||   //***** block is totally in the buffer
      !(m_buffer->begin() <= end   && end   <= m_buffer->end())) {
    BOOST_THROW_EXCEPTION(Error("begin/end iterators do not point to the underlying buffer of the block"));
  }

  m_value_begin = m_begin;
  m_value_end   = m_end;

  m_type = tlv::readType(m_value_begin, m_value_end);
  uint64_t length = tlv::readVarNumber(m_value_begin, m_value_end);
  if (verifyLength) {
    if (length != static_cast<uint64_t>(std::distance(m_value_begin, m_value_end))) {
      BOOST_THROW_EXCEPTION(tlv::Error("TLV length doesn't match buffer length"));
    }
  }
}

Block::Block(const uint8_t* buffer, size_t maxlength) //Create a Block from the raw buffer with Type-Length parsing, what is this maxlength for?
{
  const uint8_t*  tmp_begin = buffer;
  const uint8_t*  tmp_end   = buffer + maxlength;

  m_type = tlv::readType(tmp_begin, tmp_end); //just read the type
  uint64_t length = tlv::readVarNumber(tmp_begin, tmp_end); //notice the return value

  if (length > static_cast<uint64_t>(tmp_end - tmp_begin))
    {
      BOOST_THROW_EXCEPTION(tlv::Error("Not enough data in the buffer to fully parse TLV"));
    }

  m_buffer = make_shared<Buffer>(buffer, (tmp_begin - buffer) + length);//****notice!!!

  m_begin = m_buffer->begin();
  m_end = m_buffer->end();
  m_size = m_end - m_begin;

  m_value_begin = m_buffer->begin() + (tmp_begin - buffer);
  m_value_end   = m_buffer->end();
}

Block::Block(const void* bufferX, size_t maxlength) // Create a Block from the raw buffer with Type-Length parsing,almost the same like above 
{
  const uint8_t* buffer = reinterpret_cast<const uint8_t*>(bufferX); //change void* into uint8_t*

  const uint8_t* tmp_begin = buffer;
  const uint8_t* tmp_end   = buffer + maxlength;

  m_type = tlv::readType(tmp_begin, tmp_end);
  uint64_t length = tlv::readVarNumber(tmp_begin, tmp_end);

  if (length > static_cast<uint64_t>(tmp_end - tmp_begin))
    {
      BOOST_THROW_EXCEPTION(tlv::Error("Not enough data in the buffer to fully parse TLV"));
    }

  m_buffer = make_shared<Buffer>(buffer, (tmp_begin - buffer) + length);

  m_begin = m_buffer->begin();
  m_end = m_buffer->end();
  m_size = m_end - m_begin; //buffer size

  m_value_begin = m_buffer->begin() + (tmp_begin - buffer);
  m_value_end   = m_buffer->end();
}

Block::Block(uint32_t type)
  : m_type(type)
{
}

Block::Block(uint32_t type, const ConstBufferPtr& value)
  : m_buffer(value)
  , m_type(type)
  , m_begin(m_buffer->end())  //there is no need to set the begin()?
  , m_end(m_buffer->end())
  , m_value_begin(m_buffer->begin())
  , m_value_end(m_buffer->end())
{
  m_size = tlv::sizeOfVarNumber(m_type) + tlv::sizeOfVarNumber(value_size()) + value_size();
}

Block::Block(uint32_t type, const Block& value) //specific value of block
  : m_buffer(value.m_buffer)
  , m_type(type)
  , m_begin(m_buffer->end())
  , m_end(m_buffer->end())
  , m_value_begin(value.begin())
  , m_value_end(value.end())
{
  m_size = tlv::sizeOfVarNumber(m_type) + tlv::sizeOfVarNumber(value_size()) + value_size();
}

Block
Block::fromStream(std::istream& is)
{
  std::istream_iterator<uint8_t> begin(is >> std::noskipws); //NO SKIP WHITESPACE IN CASE NOTHING TO BE INPUT
  std::istream_iterator<uint8_t> end;

  uint32_t type = tlv::readType(begin, end);
  uint64_t length = tlv::readVarNumber(begin, end);

  if (length == 0) {
    return makeEmptyBlock(type);
  }

  if (length > MAX_SIZE_OF_BLOCK_FROM_STREAM)
    BOOST_THROW_EXCEPTION(tlv::Error("Length of block from stream is too large"));

  // We may still have some problem here, if some exception happens,
  // we may completely lose all the bytes extracted from the stream.

  char buf[MAX_SIZE_OF_BLOCK_FROM_STREAM];
  buf[0] = *begin;
  is.read(buf + 1, length - 1);

  if (length != static_cast<uint64_t>(is.gcount()) + 1) {   //actual number of content read from the stream
    BOOST_THROW_EXCEPTION(tlv::Error("Not enough data in the buffer to fully parse TLV"));
  }

  return makeBinaryBlock(type, buf, length);
}

std::tuple<bool, Block>
Block::fromBuffer(ConstBufferPtr buffer, size_t offset)  
{
  Buffer::const_iterator tempBegin = buffer->begin() + offset;

  uint32_t type;
  bool isOk = tlv::readType(tempBegin, buffer->end(), type);
  if (!isOk)
    return std::make_tuple(false, Block());

  uint64_t length;
  isOk = tlv::readVarNumber(tempBegin, buffer->end(), length);
  if (!isOk)
    return std::make_tuple(false, Block());

  if (length > static_cast<uint64_t>(buffer->end() - tempBegin))
    return std::make_tuple(false, Block());

  return std::make_tuple(true, Block(buffer, type,
                                     buffer->begin() + offset, tempBegin + length,
                                     tempBegin, tempBegin + length));      //got it!
}

std::tuple<bool, Block>  
Block::fromBuffer(const uint8_t* buffer, size_t maxSize)
{
  const uint8_t* tempBegin = buffer;
  const uint8_t* tempEnd = buffer + maxSize;

  uint32_t type = 0;
  bool isOk = tlv::readType(tempBegin, tempEnd, type);
  if (!isOk)
    return std::make_tuple(false, Block());

  uint64_t length;
  isOk = tlv::readVarNumber(tempBegin, tempEnd, length);
  if (!isOk)
    return std::make_tuple(false, Block());

  if (length > static_cast<uint64_t>(tempEnd - tempBegin))
    return std::make_tuple(false, Block());

  BufferPtr sharedBuffer = make_shared<Buffer>(buffer, tempBegin + length);
  return std::make_tuple(true,
         Block(sharedBuffer, type,
               sharedBuffer->begin(), sharedBuffer->end(),
               sharedBuffer->begin() + (tempBegin - buffer), sharedBuffer->end()));
}

void
Block::reset()
{
  m_buffer.reset(); // reset of the shared_ptr
  m_subBlocks.clear(); // remove all parsed subelements

  m_type = std::numeric_limits<uint32_t>::max();
  m_begin = m_end = m_value_begin = m_value_end = Buffer::const_iterator();
}

void
Block::resetWire()  //Reset wire buffer but keep sub elements
{
  m_buffer.reset(); // reset of the shared_ptr
  // keep subblocks

  // keep type
  m_begin = m_end = m_value_begin = m_value_end = Buffer::const_iterator();
}

void
Block::parse() const
{
  if (!m_subBlocks.empty() || value_size() == 0)  //subBlocks不为空或者block的值为0
    return;

  Buffer::const_iterator begin = value_begin();
  Buffer::const_iterator end = value_end();

  while (begin != end)
    {
      Buffer::const_iterator element_begin = begin;

      uint32_t type = tlv::readType(begin, end);
      uint64_t length = tlv::readVarNumber(begin, end);

      if (length > static_cast<uint64_t>(end - begin))
        {
          m_subBlocks.clear();              //********************
          BOOST_THROW_EXCEPTION(tlv::Error("TLV length exceeds buffer length"));
        }
      Buffer::const_iterator element_end = begin + length;

      m_subBlocks.push_back(Block(m_buffer,                        //add a new block at the end of the vector
                                  type,
                                  element_begin, element_end,
                                  begin, element_end));

      begin = element_end;
      // don't do recursive parsing, just the top level
    }
}

void
Block::encode()  //subBlocks can be encoded into wire buffer
{
  if (hasWire())  //Wire exists
    return;

  OBufferStream os;
  tlv::writeVarNumber(os, type()); //types are number 253, 254 or others

  if (hasValue())
    {
      tlv::writeVarNumber(os, value_size()); //valuesize= value_end-value_begin 
      os.write(reinterpret_cast<const char*>(value()), value_size()); //insert value into stream
/*
*	  Buffer::const_iterator tempBegin = value_begin();
*      for(size_t tempsize = 0; tempsize != value_size(); ++tempsize)
*      {
*        os.write(reinterpret_cast<const char*>(tempBegin), 1);
*		++tempBegin;
*      }
*/
    }
  else if (m_subBlocks.size() == 0)  //使用大小
    {
      tlv::writeVarNumber(os, 0);
    }
  else
    {
      size_t valueSize = 0;
      for (element_const_iterator i = m_subBlocks.begin(); i != m_subBlocks.end(); ++i) {
        valueSize += i->size();
      }

      tlv::writeVarNumber(os, valueSize);

      for (element_const_iterator i = m_subBlocks.begin(); i != m_subBlocks.end(); ++i) {
        if (i->hasWire())
          os.write(reinterpret_cast<const char*>(i->wire()), i->size()); //return m_begin
        else if (i->hasValue()) {
          tlv::writeVarNumber(os, i->type());
          tlv::writeVarNumber(os, i->value_size());
          os.write(reinterpret_cast<const char*>(i->value()), i->value_size());
        }
        else
          BOOST_THROW_EXCEPTION(Error("Underlying value buffer is empty"));
      }
    }

  // now assign correct block

  m_buffer = os.buf();
  m_begin = m_buffer->begin();
  m_end   = m_buffer->end();
  m_size  = m_end - m_begin;

  m_value_begin = m_buffer->begin();  //*******
  m_value_end   = m_buffer->end();

  tlv::readType(m_value_begin, m_value_end);
  tlv::readVarNumber(m_value_begin, m_value_end); 
}

const Block&
Block::get(uint32_t type) const    //Get the first subelement of the requested type
{
  element_const_iterator it = this->find(type);
  if (it != m_subBlocks.end())
    return *it;

  BOOST_THROW_EXCEPTION(Error("(Block::get) Requested a non-existed type [" +
                              boost::lexical_cast<std::string>(type) + "] from Block"));
}

Block::element_const_iterator
Block::find(uint32_t type) const  
{
  return std::find_if(m_subBlocks.begin(), m_subBlocks.end(),
                      [type] (const Block& subBlock) { return subBlock.type() == type; });
}

void
Block::remove(uint32_t type)  //remove all subelements of \p type
{
  resetWire();

  auto it = std::remove_if(m_subBlocks.begin(), m_subBlocks.end(),
                           [type] (const Block& subBlock) { return subBlock.type() == type; });
  m_subBlocks.resize(it - m_subBlocks.begin());  //???
}

Block
Block::blockFromValue() const   //value 里面还有value
{
  if (value_size() == 0)
    BOOST_THROW_EXCEPTION(Error("Underlying value buffer is empty"));

  Buffer::const_iterator begin = value_begin(),
                         end = value_end();

  Buffer::const_iterator element_begin = begin;

  uint32_t type = tlv::readType(begin, end);
  uint64_t length = tlv::readVarNumber(begin, end);

  if (length != static_cast<uint64_t>(end - begin))
    BOOST_THROW_EXCEPTION(tlv::Error("TLV length mismatches buffer length"));

  return Block(m_buffer,
               type,
               element_begin, end,
               begin, end);
}

Block::operator boost::asio::const_buffer() const  //// ConvertibleToConstBuffer 只读的buffer
{
  return boost::asio::const_buffer(wire(), size());
}

bool
Block::empty() const
{
  return m_type == std::numeric_limits<uint32_t>::max(); //返回编译器允许的uint32_t型数的最大值。
}

bool
Block::hasWire() const //m_buffer存在且begin不等于end,即认为有'线'
{
  return m_buffer && (m_begin != m_end);  //结合那种情况 如果等了就说明只有value
}

Buffer::const_iterator
Block::begin() const
{
  if (!hasWire())
    BOOST_THROW_EXCEPTION(Error("Underlying wire buffer is empty"));

  return m_begin;
}

Buffer::const_iterator
Block::end() const
{
  if (!hasWire())
    BOOST_THROW_EXCEPTION(Error("Underlying wire buffer is empty"));

  return m_end;
}

const uint8_t*
Block::wire() const
{
  if (!hasWire())
    BOOST_THROW_EXCEPTION(Error("(Block::wire) Underlying wire buffer is empty"));

  return &*m_begin; //the address of *m_begin
}

size_t
Block::size() const
{
  if (hasWire() || hasValue()) {   //有线或者有值
    return m_size;
  }
  else
    BOOST_THROW_EXCEPTION(Error("Block size cannot be determined (undefined block size)"));
}

bool
Block::hasValue() const
{
  return static_cast<bool>(m_buffer);
}

const uint8_t*
Block::value() const
{
  if (!hasValue())
    return 0;

  return &*m_value_begin;
}

size_t
Block::value_size() const
{
  if (!hasValue())
    return 0;

  return m_value_end - m_value_begin;
}

Block::element_iterator
Block::erase(Block::element_const_iterator position)  
{
  resetWire();

#ifdef NDN_CXX_HAVE_VECTOR_INSERT_ERASE_CONST_ITERATOR
  return m_subBlocks.erase(position);   //An iterator pointing to the new location of the element that followed the last element erased by the function call.
#else
  element_iterator it = m_subBlocks.begin();
  std::advance(it, std::distance(m_subBlocks.cbegin(), position)); //添加偏移量，cbegin->const,不可改元素值
  return m_subBlocks.erase(it);
#endif
}

Block::element_iterator
Block::erase(Block::element_const_iterator first, Block::element_const_iterator last)
{
  resetWire();

#ifdef NDN_CXX_HAVE_VECTOR_INSERT_ERASE_CONST_ITERATOR
  return m_subBlocks.erase(first, last);
#else
  element_iterator itStart = m_subBlocks.begin();
  element_iterator itEnd = m_subBlocks.begin();
  std::advance(itStart, std::distance(m_subBlocks.cbegin(), first));
  std::advance(itEnd, std::distance(m_subBlocks.cbegin(), last));
  return m_subBlocks.erase(itStart, itEnd);
#endif
}

void
Block::push_back(const Block& element)
{
  resetWire();
  m_subBlocks.push_back(element);
}

Block::element_iterator
Block::insert(Block::element_const_iterator pos, const Block& element)
{
  resetWire();

#ifdef NDN_CXX_HAVE_VECTOR_INSERT_ERASE_CONST_ITERATOR
  return m_subBlocks.insert(pos, element);
#else
  element_iterator it = m_subBlocks.begin();
  std::advance(it, std::distance(m_subBlocks.cbegin(), pos));
  return m_subBlocks.insert(it, element);
#endif
}

Block::element_const_iterator
Block::elements_begin() const
{
  return m_subBlocks.begin();
}

Block::element_const_iterator
Block::elements_end() const
{
  return m_subBlocks.end();
}

size_t
Block::elements_size() const
{
  return m_subBlocks.size();
}

bool
Block::operator!=(const Block& other) const
{
  return !this->operator==(other);
}

bool
Block::operator==(const Block& other) const
{
  return this->size() == other.size() &&
         std::equal(this->begin(), this->end(), other.begin());
}

} // namespace ndn
