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

#ifndef NDN_ENCODING_BLOCK_TEST_HPP 
#define NDN_ENCODING_BLOCK_TEST_HPP
 
#include "common.hpp"
 
#include "buffer.hpp"
#include "tlv.hpp"

namespace boost {
namespace asio {
class const_buffer;
} // namespace asio
} // namespace boost

namespace ndn {

/** @brief Class representing a single element to construct a buffer wire of TLV format
 */
class Block
{
public:	
	class Error : public tlv::Error
	 {
	 public:
	   explicit 
	   Error(const std::string& what)
		 : tlv::Error(what)
	   {
	   }
	 };

public: // constructor
  /** @brief Create an empty Block
   */
  Block();
  /** @brief Create a Block from the raw buffer
   */
  explicit
  Block(const ConstBufferPtr& buffer);

private:
    shared_ptr<const Buffer> m_buffer; //points to a segment of underlying memory
	Block *m_next; //points to the next block in the wire

	Buffer::const_iterator m_begin; 
    Buffer::const_iterator m_end;
	
    uint32_t m_size;  //maximum byte size of the buffer
	uint32_t begin;      //absolute offset in the wire
    //uint32_t m_type;    //type of this buffer

	//Buffer::const_iterator m_value_begin;
    //Buffer::const_iterator m_value_end;
	uint32_t m_usedsize; //used byte size of the buffer
	
public:
	/** @brief Allocate a buffer and create a Block from the raw buffer with @p usedsize bytes used
   */
   static Block *
   BufferAllocate(size_t capacity);

	
};
}
