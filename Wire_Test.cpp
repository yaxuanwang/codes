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

Wire::Wire(const Block *block)
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
Wire::RemainingInCurrentBlock()
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
Wire::Reserve(size_t length)
{
    /*If the current block has a next pointer, then the remaining is from 
	 *the position to its size. Otherwise it is from the position to the end.
	 */
    size_t remaining;
    remaining = m_current->m_offset + m_current->m_capacity - m_position;
	
    if (remaining < length) {
		// remaining space of this block is small, just finalize it and allocate a new one
		if (length < 32 && m_current->m_next == NULL) {
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
	
    size_t relativePosition = m_position - m_current->m_offset;
    auto iter = m_begin + relativePosition + 1;
    *iter = value;
    if (relativePosition > m_current->m_size) {
		m_current->m_size = relativePosition;
    }
	
    m_position++;
    return 1;
}

size_t 
Wire::writeUint16(uint16_t value)
{
    Reserve(2);

    writeUint8(value >> 8);
    writeUint8(value & 0xFF);
    return 2;
}

void 
Wire::writeUint32(uint32_t value)
{
    Reserve(4);

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
    Reserve(8);

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
		size_t remaining = RemainingInCurrentBlock();
		if (remaining == 0) {
			expandIfNeeded();
		} 
		else {
			if (remaining > (length - offset)) {
				remaining = length - offset;
			}
	
			size_t relativePosition = m_position - m_current->begin;
			auto dest = m_current->m_begin + relativePosition;
			auto src = array + offset;     //notice !!
			std::copy(src, src + remaining, dest);
	
			relativePosition += remaining;
			if (relativePosition > m_current->m_size) {
				m_current->m_size = relativePosition;
			}
	
			m_position += remaining;
			offset += remaining;
		}
	}

    return length;
}


}

