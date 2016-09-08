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

	typedef shared_ptr<const Wire>              ConstWirePtr;
    typedef shared_ptr<Wire>                    WirePtr;
	/** @brief Create an empty wire
	 */
    Wire();
	/** @brief Create the first block in wire with capacity @p capacity
	 */
	Wire(size_t capacity);
	/** @brief Create a wire with the fisrt block @p block
	 */
	Wire(const Block *block);
private:
	size_t m_position;          //absolute offset in this wire
    size_t m_capacity;         //total maximum byte size of this wire

    Block *m_begin;    //first block
    Block *m_current;  //current block
    Block *m_end;      //the last block
	mutable io_container m_iovec;
	size_t m_count;    //reference time 
public:
	/** @brief Increase a reference to current wire
	 */
	Wire *
	Copy();
	/** @brief Return current offset in this wire
	 */
	size_t 
	Position();

};

}



