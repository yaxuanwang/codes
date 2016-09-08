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

const size_t MAX_SIZE_OF_BLOCK_FROM_STREAM = MAX_NDN_PACKET_SIZE; //从流中来的最大block的size是最大包的size

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

static Block *
Block::BufferAllocate(size_t capacity){
	Buffer* buf = new Buffer(capacity);
	Block block(buf);

	return block;
}

}

