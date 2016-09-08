#ifndef NDN_ENCODING_WIRE_TEST_HPP 
#define NDN_ENCODING_WIRE_TEST_HPP
 
#include "Block_Test.hpp"

namespace boost {
namespace asio {
class const_buffer;
} // namespace asio
} // namespace boost

namespace ndn {
/** @brief Class representing a series of linked buffers
 */
class Wire
{
public:
	typedef std::vector<shared_ptr<Buffer>>     io_container;
	typedef io_container::iterator              io_iterator;
    typedef io_container::const_iterator        io_const_iterator;
	/** @brief Creates an empty wire
	 */
    Wire();
private:
	size_t m_position;          //absolute offset in this wire
    size_t m_capacity;         //total maximum byte size of this wire

    Block *m_begin;
    Block *m_current;
    Block *m_end;
	mutable io_container m_iovec;

};

}



