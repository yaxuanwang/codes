#include "Wire_Test.hpp"

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

Wire*
Wire::copy()
{
  if(hasWire()){
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

    return this->end->m_begin + this->end->m_size;
}


void
Wire::setPositon(size_t position)
{
	if (!hasWire())
				BOOST_THROW_EXCEPTION(Error("Wire is empty"));
	
	if(position <= size()){
	    // Is the new position within the current memory block?
        if (m_current.inBlock(position)) {
            // we're ok, new position is in this buffer, we're done :)
        } 
		else {
            // we need to find the right buffer
            Block *block = m_begin;
            while (!block.inBlock(position)) {
                block = block.m_next;
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
		   m_iovec.push_back(block.m_buffer);
		   block = block.m_next;
	   }
}

size_t
Wire::countBlock()
{
	size_t count = 0;
	Block *block = m_begin;
	while (block) {
			count++;
			block = block.m_next;
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
        if (m_current.inBlock(position)) {
            // we're ok, new position is in this buffer, we're done :)
        } 
		else {
            // we need to find the right buffer
            Block *block = m_begin;
            while (!block.inBlock(position)) {
                block = block.m_next;
            }
            m_current = block;
        }

        // discard any memory blocks after this

        Block *current = m_current.m_next;
        while (current) {
            Block *next = current.m_next;
            current.m_next = NULL;
			current.deAllocate();     //still some problems here
            current = next;
        }

        // Set the limit of the current block so buffer->position is the end
        m_current.m_next = NULL;
        size_t setSize = m_position - m_current.m_begin;
        m_current.m_size= setSize;
        m_end = m_current;
   }
}

}

