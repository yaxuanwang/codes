#ifndef NDN_ENCODING_ENCODER_TEST_HPP
#define NDN_ENCODING_ENCODER_TEST_HPP

#include "common.hpp"
#include "Wire_Test.hpp"

namespace ndn {
namespace encoding {

/**
 * @brief Helper class to perform TLV encoding
 * Interface of this class (mostly) matches interface of Estimator class
 * @sa Estimator
 */
class Encoder
{
public:   // common interface between Encoder and Estimator
  /**
   * @brief Create instance of the encoder with the specified reserved sizes
   * @param firstReserve initial the first buffer size to reserve
   */
  Encoder(size_t firstReserve);
  
  /**
   * @brief Append a byte
   */
  size_t
  appendByte(uint8_t value);

  /**
   * @brief Append a byte array @p array of length @p length
   */
  size_t
  appendByteArray(const uint8_t* array, size_t length);
  
  /**
   * @brief Prepend VarNumber @p varNumber of NDN TLV encoding
   * @sa http://named-data.net/doc/ndn-tlv/
   */
  size_t
  appendVarNumber(uint64_t varNumber);

  /**
   * @brief Append non-negative integer @p integer of NDN TLV encoding
   * @sa http://named-data.net/doc/ndn-tlv/
   */
  size_t
  appendNonNegativeInteger(uint64_t integer);
  /**
   * @brief Append TLV block of type @p type and value from buffer @p array of size @p arraySize
   */
  size_t
  appendByteArrayBlock(uint32_t type, const uint8_t* array, size_t arraySize);
  /**
   * @brief Append TLV block @p block
   */
  size_t
  appendBlock(const Block& block);
  
  
private:
  //shared_ptr<Wire> m_wire;
  Wire *m_wire;

public: // unique interface to the Encoder
  typedef Buffer::iterator iterator;
  typedef Buffer::const_iterator const_iterator;


};

}
}

