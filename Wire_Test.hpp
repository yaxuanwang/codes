#ifndef NDN_ENCODING_WIRE_TEST_HPP 
#define NDN_ENCODING_WIRE_TEST_HPP
 
#include "Block_Test.hpp"

namespace boost {
namespace asio {
class const_buffer;
} // namespace asio
} // namespace boost

namespace ndn {
/** @brief Class representing a series of linked blocks
 */
class Wire
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
	size_t m_position;               //absolute offset in this wire
    size_t m_capacity;               //total maximum byte size of this wire

    Block *m_begin;                  //first block
    Block *m_current;                //current block
    Block *m_end;                    //the last block
	mutable io_container m_iovec;    //buffer sequence
	size_t m_count;                  //reference time 
public:
    /** @brief Check if the Wire is empty
    */
    bool
    hasWire();
	/** @brief Increase a reference to current wire
	 */
	Wire*
	copy();
	/** @brief Return current offset in this wire
	 */
	size_t 
	position();
	/** @brief Return current capacity of this wire
	 */
	size_t 
	capacity();
	/** @brief Return current used byte size of this wire
	 */
	size_t 
	size();
	/** @brief set the offset to a specific positon
	 */
	void
	setPositon(size_t position);
public: //iovec
	/** @brief put buffers pointed by blocks into a buffer sequence iovec
	 *  All buffers in blocks in this wire will not be copied or modified. The iovec is read-only for doing a gathering write.
	 */
	void 
	setIovec();
	/** @brief count the number of blocks in this wire
	 */
	size_t
	countBlock();
    /** @brief set the size of this wire to the current postion and throw ohters  
	 */
	void 
	finalize(); 
	/** @brief linerize the wire into a single buffer  
	 */
    uint8_t *
	getBuffer();
		
	
	

};

}



