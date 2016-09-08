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

Wire::Wire(size_t capacity){
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

Wire*
Wire::Copy(){
     m_count++;
	 return this;
}

size_t 
Wire::Position(){
	 return m_position;
}

}
