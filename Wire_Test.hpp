#ifndef NDN_ENCODING_WIRE_TEST_HPP 
#define NDN_ENCODING_WIRE_TEST_HPP
 
#include "Block_Test.hpp"
#include "common.hpp"


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
	
	typedef std::vector<Block>                  element_container;
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
    uint32_t m_type;                 //type of this wire
	mutable element_container m_subBlocks;
	
public: //wire
    /** @brief Check if the Wire is empty
    */
    bool
    hasWire();
	/** @brief Increase a reference to current wire
	 */
	shared_ptr<Wire>
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
    shared_ptr<Buffer>
	getBufferFromIovec();
	/** @brief check if there is element in iovec  
	 */
    bool
	hasIovec();

public: //operate the wire

	/** @brief Return the remaining bytes space between positon and capacity of current block 
	 */
    size_t
	remainingInCurrentBlock();
	/** @brief Expand the wire with a new block adding to the end with capacity @p allocationSize  
     *  Defualt size is 2048
     */
	void
    expand(size_t allocationSize = 2048);
	/** @brief Expand the wire when current capacity is not enough  
     */
	void
	expandIfNeeded();
	/** @brief Check if there are @p length bytes left in a single block's contiguous underlying memory.
	 *   If not, and the remaining buffer space is small, we will finalize it and allocate a new block.
	 */
    void
    reserve(size_t length);
	/** @brief Write a `uint8_t` to the current position, allocating as necessary  
     */
    size_t 
	writeUint8(uint8_t value);
	/** @brief Write a `uint16_t` to the current position, allocating as necessary  
     */
    size_t 
	writeUint16(uint16_t value);
	/** @brief Write a `uint32_t` to the current position, allocating as necessary  
     */
    size_t
	writeUint32(uint32_t value);
	/** @brief Write a `uint64_t` to the current position, allocating as necessary  
     */
    size_t 
	writeUint64(uint64_t value);
	/** @brief Append an array with @p length bytes to the current position, allocating new block as necessary
	 *   return the size of the appended array
     */
    size_t 
	appendArray(const uint8_t* array, size_t length);
	/** @brief Append a block to the current position (this will call finalize and throw buffer after current position)
	 *   return the size of the appended block
     */
    size_t 
	appendBlock(const Block* block);
	/** @brief Insert a block to the current position (this will throw data in current block after current position)
	 */
    size_t 
	insertBlock(const Block* block); //complicated
	/** @brief Append a wire to the current position (not decided yet)
     */
    size_t 
	appendWire(const Wire* wire);
	/** @brief read the `uint8_t` in @p position position 
     */
    uint8_t 
	readUint8(size_t position);
	/** @brief From logical continuous wire create a physical continuous memory buffer
	 *   return the shared pointer of this underlying buffer
     */
	shared_ptr<Buffer>
	getBuffer();
	
	
};

}



