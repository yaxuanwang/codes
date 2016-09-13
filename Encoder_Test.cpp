#include "Encoder_Test.hpp"
#include "Endian.hpp"

namespace ndn {
namespace encoding {


Encoder::Encoder(size_t firstReserve)
  : m_wire(new Wire(firstReserve))
{
}

size_t
Encoder::appendByte(uint8_t value)
{
  m_wire->reserve(1);
  m_wire->writeUint8(value);
  return 1;
}

size_t
Encoder::appendByteArray(const uint8_t* array, size_t length)
{
  m_wire->reserve(length);
  m_wire->appendArray(array,length);
  return length;
}

size_t
Encoder::appendVarNumber(uint64_t varNumber)
{
  if (varNumber < 253) {
    appendByte(static_cast<uint8_t>(varNumber));
    return 1;
  }
  else if (varNumber <= std::numeric_limits<uint16_t>::max()) {
    appendByte(253);
    uint16_t value = htobe16(static_cast<uint16_t>(varNumber));
    appendByteArray(reinterpret_cast<const uint8_t*>(&value), 2);
    return 3;
  }
  else if (varNumber <= std::numeric_limits<uint32_t>::max()) {
    appendByte(254);
    uint32_t value = htobe32(static_cast<uint32_t>(varNumber));
    appendByteArray(reinterpret_cast<const uint8_t*>(&value), 4);
    return 5;
  }
  else {
    appendByte(255);
    uint64_t value = htobe64(varNumber);
    appendByteArray(reinterpret_cast<const uint8_t*>(&value), 8);
    return 9;
  }
}

size_t
Encoder::appendNonNegativeInteger(uint64_t varNumber)
{
  if (varNumber <= std::numeric_limits<uint8_t>::max()) {
    return appendByte(static_cast<uint8_t>(varNumber));
  }
  else if (varNumber <= std::numeric_limits<uint16_t>::max()) {
    uint16_t value = htobe16(static_cast<uint16_t>(varNumber));
    return appendByteArray(reinterpret_cast<const uint8_t*>(&value), 2);
  }
  else if (varNumber <= std::numeric_limits<uint32_t>::max()) {
    uint32_t value = htobe32(static_cast<uint32_t>(varNumber));
    return appendByteArray(reinterpret_cast<const uint8_t*>(&value), 4);
  }
  else {
    uint64_t value = htobe64(varNumber);
    return appendByteArray(reinterpret_cast<const uint8_t*>(&value), 8);
  }
}

size_t
Encoder::appendByteArrayBlock(uint32_t type, const uint8_t* array, size_t arraySize)
{
  size_t totalLength = appendVarNumber(type);
  totalLength += appendVarNumber(arraySize);
  totalLength += appendByteArray(array, arraySize);

  return totalLength;
}

size_t
Encoder::appendBlock(const Block& block)
{
  size_t length = m_wire.appendBlock(block);
  return length;
}

}
}
