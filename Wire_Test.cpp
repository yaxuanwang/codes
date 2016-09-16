#include "Wire_Test.hpp"
#include "Buffer-stream.hpp"

namespace ndn {

#if NDN_CXX_HAVE_IS_NOTHROW_MOVE_CONSTRUCTIBLE
static_assert(std::is_nothrow_move_constructible<Wire>::value,
              "Wire must be MoveConstructible with noexcept");
#endif // NDN_CXX_HAVE_IS_NOTHROW_MOVE_CONSTRUCTIBLE

#if NDN_CXX_HAVE_IS_NOTHROW_MOVE_ASSIGNABLE
static_assert(std::is_nothrow_move_assignable<Wire>::value,
              "Wire must be MoveAssignable with noexcept");
#endif // NDN_CXX_HAVE_IS_NOTHROW_MOVE_ASSIGNABLE

typedef shared_ptr<const Wire>              ConstWirePtr;
typedef shared_ptr<Wire>                    WirePtr;


Wire::Wire()
{
}

Wire::Wire(size_t capacity)
{
	Block block(capacity);
    m_begin = block;
    m_end = m_begin;
    m_current = m_begin;
    m_capacity = capacity;
	m_position = 0;
	m_count = 1;
}

Wire::Wire(const Block* block)
   :m_begin(block),
	m_current(m_begin),
	m_end(m_begin),
	m_capacity(block.m_capacity),
	m_position(block.m_size)
{
    m_count = 1;
}

bool
Wire::hasWire()
{
    return static_cast<bool>(m_begin);
}

shared_ptr<Wire>
Wire::copy()
{
    if(hasWire()) {
       m_count++;
	   return this;
  	}
    else
	  BOOST_THROW_EXCEPTION(Error("Wire is empty"));
}

size_t 
Wire::position()
{
	if (!hasWire())
		BOOST_THROW_EXCEPTION(Error("Wire is empty"));

	return m_position;
}

size_t 
Wire::capacity()
{
	if (!hasWire())
		BOOST_THROW_EXCEPTION(Error("Wire is empty"));

	return m_capacity;
}

size_t 
Wire::size()
{
	if (!hasWire())
		BOOST_THROW_EXCEPTION(Error("Wire is empty"));

    return m_end.m_offset+ m_end.m_size;
}


void
Wire::setPositon(size_t position)
{
	if (!hasWire())
		BOOST_THROW_EXCEPTION(Error("Wire is empty"));
	
	if(position <= size()) {
	    // Is the new position within the current memory block?
        if (m_current->inBlock(position)) {
            // we're ok, new position is in this buffer, we're done :)
        } 
		else {
            // we need to find the right buffer
            Block *block = m_begin;
            while (!block->inBlock(position)) {
                block = block->m_next;
            }
            m_current = block;
        }
    }
    m_position = position;
}

Buffer*
Wire::findPosition(Buffer::const_iterator& begin, size_t position)
{
    if (!hasWire())
		BOOST_THROW_EXCEPTION(Error("Wire is empty"));

	Block *block = m_begin;
	if(position <= size()) {
	    // Is the new position within the current memory block?
        if (m_current->inBlock(position)) {
            // we're ok, new position is in this buffer, we're done :)
        } 
		else {
            // we need to find the right buffer
            while (!block->inBlock(position)) {
                block = block->m_next;
            }
        }
    }
	
	size_t relativeOffset = position - block->m_offset;
	begin = m_begin + relativeOffset;
	return block->m_buffer;
}

void 
Wire::setIovec()
{
    size_t iovcnt = countBlock();
	
	Block *block = m_begin;
	for (int i = 0; i < iovcnt; i++) {
		   m_iovec.push_back(block->m_buffer);
		   block = block->m_next;
	   }
}

size_t
Wire::countBlock()
{
	size_t count = 0;
	Block *block = m_begin;
	while (block) {
			count++;
			block = block->m_next;
		}
	return count;

}

void 
Wire::finalize()
{
    // if we're at the limit, we're done
    if (m_position < size()) {
        // begin at the tail and free memory blocks until we've found the current position
        size_t position = m_position;

        // Is the new position within the current memory block?
        if (m_current->inBlock(position)) {
            // we're ok, new position is in this buffer, we're done :)
        } 
		else {
            // we need to find the right buffer
            Block *block = m_begin;
            while (!block->inBlock(position)) {
                block = block->m_next;
            }
            m_current = block;
		}

        // discard any memory blocks after this

        Block *current = m_current->m_next;
        while (current) {
            Block *next = current->m_next;
            current->m_next = NULL;
			current->deAllocate();     //still some problems here
            current = next;
        }

        // Set the limit of the current block so buffer->position is the end
        m_current->m_next = NULL;
        size_t setSize = m_position - m_current->m_begin;
        m_current->m_size= setSize;
        m_end = m_current;
    }
}

bool
Wire::hasIovec()
{
    return static_cast<bool>(m_iovec.size());
}

shared_ptr<Buffer>
Wire::getBufferFromIovec() //still some problems here
{
    if(!hasIovec())
  	    BOOST_THROW_EXCEPTION(Error("The iovec is empty")); //if iovec is not constructed, it fails
    OBufferStream os;
    size_t totalSize = 0;
    for (io_iterator i = m_iovec.begin(); i != m_iovec.end(); ++i) {
        totalSize += i->size();
	    os.write(reinterpret_cast<const char*>(i), i->size());
    }
  
    return os.buf();
}

size_t
Wire::remainingInCurrentBlock()
{
	size_t remaining = m_current->m_offset + m_current->m_capacity - m_position;
	if(remaining < 0)
		BOOST_THROW_EXCEPTION(Error("Position of current block gets wrong"));
	return remaining;
}

void
Wire::expand(size_t allocationSize = 2048)
{
    Block b;
    Block *block =b.allocate(allocationSize);
	
    m_capacity += block->m_capacity;
    block->m_begin = m_end->m_begin + m_end->m_size;
	
    m_end->next = block;
    m_end->m_capacity= m_end->m_size;  //tailor the capacity of the last block into its current size
    m_end = block;
}

void
Wire::expandIfNeeded()
{
    if (m_position == m_current->m_offset+ m_current->m_capacity) {
	    if (m_current->m_next) {
			m_current = m_current->m_next;
		} 
	    else {
            //it's the end of the wire
			expand();
			m_current = m_end;
		}
    }
}

void
Wire::reserve(size_t length)
{
    /*If the current block has a next pointer, then the remaining is from 
	 *the position to its size. Otherwise it is from the position to the end.
	 */
    size_t remaining;
    remaining = m_current->m_offset + m_current->m_capacity - m_position;
	
    if (remaining < length) {
		// remaining space of this block is small, just finalize it and allocate a new one
		// need to guarantee the remaining space is enough for T and L 
		// specific number needs to be considered again
		if (remaining < 32 && m_current->m_next == NULL) {
			expand();
			m_current = m_end;
			return;
		}
	
		// otherwise, use the remaining sapce in current buffer and allocate a new one
		expandIfNeeded();
    }
}

size_t 
Wire::writeUint8(uint8_t value)
{
    expandIfNeeded();
	
    size_t relativeOffset = m_position - m_current->m_offset;
    auto iter = m_current->m_begin + relativeOffset + 1;
    *iter = value;
    if (relativeOffset > m_current->m_size) {
		m_current->m_size = relativeOffset;
    }
	
    m_position++;
    return 1;
}

size_t 
Wire::writeUint16(uint16_t value)
{
    reserve(2);

    writeUint8(value >> 8);
    writeUint8(value & 0xFF);
    return 2;
}

void 
Wire::writeUint32(uint32_t value)
{
    reserve(4);

    for (int i = sizeof(uint32_t) - 1; i > 0; i--) {
        uint8_t byte = value >> (i * 8) & 0xFF;
        writeUint8(byte);
    }

    writeUint8(value & 0xFF);
    return 4;
}

void 
Wire::writeUint64(uint64_t value)
{
    reserve(8);

    for (int i = sizeof(uint64_t) - 1; i > 0; i--) {
        uint8_t byte = value >> (i * 8) & 0xFF;
        writeUint8(byte);
    }

    writeUint8(value & 0xFF);
    return 8;
}

size_t 
Wire::appendArray(const uint8_t* array, size_t length)
{
    expandIfNeeded();
	
    size_t offset = 0;
    while (offset < length) {
		size_t remaining = remainingInCurrentBlock();
		if (remaining == 0) {
			expandIfNeeded();
		} 
		else {
			if (remaining > (length - offset)) {
				remaining = length - offset;
			}
	
			size_t relativeOffset = m_position - m_current->m_offset;
			auto dest = m_current->m_begin + relativeOffset;
			auto src = array + offset;     //notice !!
			std::copy(src, src + remaining, dest);
	
			relativeOffset += remaining;
			if (relativeOffset > m_current->m_size) {
				m_current->m_size = relativeOffset;
			}
	
			m_position += remaining;
			offset += remaining;
		}
	}

    return length;
}

size_t 
Wire::appendBlock(const Block& block)
{
    finalize();
	// we assume that this is a single block (only when a block is put into a wire it will has next pointer)
    if (!(block->m_next))
		BOOST_THROW_EXCEPTION(Error("block does not have next pointer until put into a wire"));
	m_end->m_next = block;
	m_end = block;
	m_end->m_offset = m_position;

    m_current = m_end; 
	m_position += m_end->m_size;
	m_capacity += m_end->m_capacity;

	return m_end->m_size;
}

uint8_t 
Wire::readUint8(size_t position)
{
    if(m_capacity >position) {
		Block *block = m_begin;
		while(block && !block->inBlock(position)){
			 block = block->m_next;
        }
        size_t relativeOffset = position - block->m_offset;
        return *(block->m_begin + relativeOffset);
	}
	else
		BOOST_THROW_EXCEPTION(Error("could not find the illegal position"));
}

shared_ptr<Buffer>
Wire::getBuffer()
{
    OBufferStream os;
	size_t totalSize = 0;
    Block *block = m_begin;
    while (block) {
        totalSize += block->size();
		os.write(reinterpret_cast<const char*>(block->bufferValue()), block->size());
        block = block->next;
    }
    return os.buf();
}

void
Wire::parse() const
{
	if (!m_subWires.empty() || size() == 0)	//there have been some wires in the container
		return;
	
	size_t begin = 0;
	size_t end = size();
	
	while (begin != end)
	{
	  size_t element_begin = begin;
	  Buffer::const_iterator tmp_begin;
	
	  uint32_t type = tlv::readType(this, begin, end);
	  uint64_t length = tlv::readVarNumber(this, begin, end);
	
	  if (length > static_cast<uint64_t>(end - begin))
	  {
		m_subWires.clear();				//********************
		BOOST_THROW_EXCEPTION(tlv::Error("TLV length exceeds buffer length"));
	  }
	  size_t element_end = begin + length;
	  Buffer::const_iterator tmp_end;
	  Block* beginBlock = findPosition(tmp_begin, element_begin);
	  Block* endBlock = findPosition(tmp_end, element_end);
	  //if subwire's begin and subwire's end are in the same block(underlying buffer is consecutive)
	  if(beginBlock = endBlock)
      {
        Block* first = Block(beginBlock->m_buffer, tmp_begin, tmp_end);
		Wire wire = Wire(first);
		wire.m_type = type;
		m_subWires.push_back(wire); 
	  }
	  else{
	  	Block* first = Block(beginBlock->m_buffer, tmp_begin, tmp_end);
	  	Wire wire = Wire(first);
		wire.m_type = type;
		Block* block= beginBlock->m_next;
		while(block != endBlock)
		{ 
		   wire.appendBlock(Block(block->m_buffer, block->m_begin, block->m_end)); 
		   block = block->m_next;
		}
		wire.appendBlock(Block(block->m_buffer, block->m_begin, tmp_end));
		m_subWires.push_back(wire);
	  }
	  begin = element_end;
		  // don't do recursive parsing, just the top level
	}
}

const Wire&
Wire::get(uint32_t type) const
{
	element_const_iterator it = this->find(type);
	 if (it != m_subWires.end())
	   return *it;
	
	 BOOST_THROW_EXCEPTION(Error("(Wire::get) Requested a non-existed type [" +
								 boost::lexical_cast<std::string>(type) + "] from Wire"));
}

Wire::element_const_iterator
Wire::find(uint32_t type) const  
{
  return std::find_if(m_subWires.begin(), m_subWires.end(),
                      [type] (const Block& subBlock) { return subBlock.type() == type; });
}

const Wire::element_container&
Wire::elements() const
{
  return m_subWires;
}

Wire::element_const_iterator
Wire::elements_begin() const
{
  return m_subWires.begin();
}

Wire::element_const_iterator
Wire::elements_end() const
{
  return m_subWires.end();
}

size_t
Wire::elements_size() const
{
  return m_subWires.size();
}

}



