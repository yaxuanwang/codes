/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2013-2015 Regents of the University of California.
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

#include "block_test.hpp"
#include "tlv.hpp"

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

const size_t MAX_SIZE_OF_BLOCK_FROM_STREAM = MAX_NDN_PACKET_SIZE; 

Block::Block()   //create an empty Block 
{
}

Block::Block(const ConstBufferPtr& buffer) 
  : m_buffer(buffer)
  , m_begin(m_buffer->begin())
  , m_end(m_buffer->end())
  , m_capacity(m_end - m_begin)
{
    m_next = NULL;
	m_size = buffer.size();
	m_offset = 0;
}

Block::Block(const ConstBufferPtr& buffer,   
             const Buffer::const_iterator& begin, const Buffer::const_iterator& end)
  : m_buffer(buffer)
  , m_begin(begin)
  , m_end(end)
  , m_capacity(m_end - m_begin)
{
	m_next = NULL;
	m_size = buffer.size();
	m_offset = 0;
}

Block::Block(const uint8_t* array, size_t length){
	m_buffer = make_shared<Buffer>(array, array+length);
	m_begin = m_buffer->begin();
    m_end = m_buffer->end();
    m_size = m_end - m_begin;
	m_capacity = m_size;
	m_next = NULL;
}

static Block *
Block::allocate(size_t capacity){
	ConstBufferPtr buf = new Buffer(capacity);
	Block block(buf);

	return block;
}

bool
Block::hasBuffer() const
{
  return static_cast<bool>(m_buffer);
}

bool
Block::empty() const
{
  return m_buffer && (m_size == 0);
}

void
Block::reset()
{
  m_buffer.reset(); // reset of the shared_ptr
  m_begin = m_end = Buffer::const_iterator();
  m_capacity = m_offset = m_size = 0;
  m_next = NULL;
}

Buffer::const_iterator
Block::begin() const
{
  if (!hasBuffer())
    BOOST_THROW_EXCEPTION(Error("Underlying buffer is empty"));

  return m_begin;
}

Buffer::const_iterator
Block::end() const
{
  if (!hasBuffer())
    BOOST_THROW_EXCEPTION(Error("Underlying buffer is empty"));

  return m_end;
}

const uint8_t*
Block::bufferValue() const
{
  if (!hasBuffer())
    BOOST_THROW_EXCEPTION(Error("Underlying buffer is empty"));

  return &*m_begin; 
}

size_t
Block::capacity() const
{
  if (hasBuffer()) {   
    return m_capacity;
  }
  else
    BOOST_THROW_EXCEPTION(Error("Block capacity cannot be determined (undefined block capacity)"));
}

size_t
Block::size() const
{
  if (hasBuffer()) {   
    return m_size;
  }
  else
    BOOST_THROW_EXCEPTION(Error("Block used size cannot be determined (undefined block used size)"));
}

shared_ptr<const Buffer>
Block::getBuffer() const
{
  return m_buffer;
}

bool
Block::inBlock(size_t position){
	if (!hasBuffer())
				BOOST_THROW_EXCEPTION(Error("underlying buffer is empty"));

	return (m_offset <= position && position < m_offset+ m_size);
}

void
Block::deAllocate()
{
    //there are still some problems about memory
	this.reset();
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

}

