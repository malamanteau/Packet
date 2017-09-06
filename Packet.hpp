#pragma once

//#include <array>
#include <cstdint>
#include <vector>
#include <limits>
#include <memory>
#include <cstring>

#if defined(__linux__)
#	include <endian.h>
#	define __packed
#endif

#ifdef _MSC_VER
#	define __packed
#endif

// We can optimize calls to the conversion functions.  Either nothing has
// to be done or we are using directly the byte-swapping functions which
// often can be inlined.
# if __BYTE_ORDER == __BIG_ENDIAN
// The host byte order is the same as network byte order,
// so these functions are all just identity.
#  define ntohl(x)   (x)
#  define ntohll(x)  (x)
#  define ntohs(x)   (x)
#  define htonl(x)   (x)
#  define htonll(x)  (x)
#  define htons(x)   (x)
# else
#  if __BYTE_ORDER == __LITTLE_ENDIAN
#   define ntohll(x) __bswap_64 (x)
#   define ntohl(x)  __bswap_32 (x)
#   define ntohs(x)  __bswap_16 (x)
#   define htonll(x) __bswap_64 (x)
#   define htonl(x)  __bswap_32 (x)
#   define htons(x)  __bswap_16 (x)
#  endif
# endif

/// If you would like the functions/classes in Packet 
/// to be in a namespace, just define PACKET_NS 
/// as your desired namespace name.
#ifdef PACKET_NS
namespace PACKET_NS {
#endif

class alignas(16) Packet final
{
    // A bool-like type that cannot be converted to integer or pointer types
	typedef bool(Packet::*BoolType)(size_t);

public:
	Packet(); // Creates an empty packet.

	~Packet() = default;

	void                Append(const void* data, size_t sizeInBytes); // Append data to the end of the packet
	void                Clear();

	size_t              SizeBytes() const;

						// Get a pointer to the data contained in the packet.
						// Warning: the returned pointer may become invalid after
						// you append data to the packet, therefore it should never
						// be stored.
						// The return pointer is NULL if the packet is empty.
	const void *        GetData() const;

	void                RestartRead();

						// Tell if the reading position has reached the end of the packet
						// This function is useful to know if there is some data
						// left to be read, without actually reading it.
						// Returns true if all data was read, false otherwise.
	bool                EndOfPacket() const;

	void                Reserve(uint32_t size);
public:
						// Overloads of operator >> to read data from the packet
	Packet &            operator >>(bool &         data);
	Packet &            operator >>(int8_t &       data);
	Packet &            operator >>(uint8_t &      data);
	Packet &            operator >>(int16_t &      data);
	Packet &            operator >>(uint16_t &     data);
	Packet &            operator >>(int32_t &      data);
	Packet &            operator >>(uint32_t &     data);
	Packet &            operator >>(int64_t &      data);
	Packet &            operator >>(uint64_t &     data);
	Packet &            operator >>(float &        data);
	Packet &            operator >>(double &       data);
	Packet &            operator >>(char *         data);
	Packet &            operator >>(wchar_t *      data);
	Packet &            operator >>(std::string &  data);
	Packet &            operator >>(std::wstring & data);
	
	template <typename T,
		      typename = std::enable_if<std::is_enum<T>::value>>
	Packet &            operator >>(T & data)
	{
		typename std::underlying_type<T>::type temp;
		*this >> temp;
		data = static_cast<T>(temp);
	}

						// Overloads of operator << to write data into the packet
	Packet &            operator <<(bool                 data);
	Packet &            operator <<(int8_t               data);
	Packet &            operator <<(uint8_t              data);
	Packet &            operator <<(int16_t              data);
	Packet &            operator <<(uint16_t             data);
	Packet &            operator <<(int32_t              data);
	Packet &            operator <<(uint32_t             data);
	Packet &            operator <<(int64_t              data);
	Packet &            operator <<(uint64_t             data);
	Packet &            operator <<(float                data);
	Packet &            operator <<(double               data);
	Packet &            operator <<(const char *         data);
	Packet &            operator <<(const wchar_t *      data);
	Packet &            operator <<(const std::string &  data);
	Packet &            operator <<(const std::wstring & data);
	
	template <typename T,
		      typename = std::enable_if<std::is_enum<T>::value>>
	Packet &            operator <<(T data)
	{
		*this << static_cast<typename std::underlying_type<T>::type>(data);
	}
	
	
						// Test the validity of the packet, for reading
						//
						// This operator allows to test the packet as a boolean
						// variable, to check if a reading operation was successful.
						//
						// A packet will be in an invalid state if it has no more
						// data to read.
						//
						// This behavior is the same as standard C++ streams.
						//
						// Usage example:
						// float x;
						// packet >> x;
						// if (packet)
						// {
						//    // ok, x was extracted successfully
						// }
						//
						// -- or --
						//
						// float x;
						// if (packet >> x)
						// {
						//    // ok, x was extracted successfully
						// }
						//
						// Don't focus on the return type, it's equivalent to bool but
						// it disallows unwanted implicit conversions to integer or
						// pointer types.
						//
						// Returns true if last data extraction from packet was successful
	operator BoolType() const;

private:
	bool                operator ==(const Packet & right) const = delete; // Disallow comparisons between packets
	bool                operator !=(const Packet & right) const = delete;

						// Check if the packet can extract a given number of bytes
						// This function updates accordingly the state of the packet.
						// Returns true if size bytes can be read from the packet
	bool                checkSize(std::size_t size);

	std::vector<char>m_data;    // Data stored in the packet
	std::size_t         m_readPos; // Current reading position in the packet
	bool                m_isValid; // Reading state of the packet
};

inline
Packet::Packet() : 
	m_readPos(0), 
	m_isValid(true)
{}

inline
void Packet::Append(const void * data, std::size_t sizeInBytes)
{
	if (data && (sizeInBytes > 0))
	{
		std::size_t start = m_data.size();
		m_data.resize(start + sizeInBytes);
		std::memcpy(&m_data[start], data, sizeInBytes);
	}
}

inline
void Packet::Clear()
{
	m_data.clear();
	m_readPos = 0;
	m_isValid = true;
}

inline
void Packet::RestartRead()
{
	m_readPos = 0;
	m_isValid = true;
}

inline
const void * Packet::GetData() const
{
	return !m_data.empty() ? &m_data[0] : NULL;
}

inline
std::size_t Packet::SizeBytes() const
{
	return m_data.size();
}

inline
bool Packet::EndOfPacket() const
{
	return m_readPos >= m_data.size();
}

inline
Packet & Packet::operator >>(bool & data)
{
	uint8_t value;
	if (*this >> value)
		data = (value != 0);

	return *this;
}

inline
Packet & Packet::operator >>(int8_t & data)
{
	if (checkSize(sizeof(data)))
	{
		data = *reinterpret_cast<__packed const int8_t *>(&m_data[m_readPos]);
		m_readPos += sizeof(data);
	}

	return *this;
}

inline
Packet & Packet::operator >>(uint8_t & data)
{
	if (checkSize(sizeof(data)))
	{
		data = *reinterpret_cast<__packed const uint8_t *>(&m_data[m_readPos]);
		m_readPos += sizeof(data);
	}

	return *this;
}

inline
Packet & Packet::operator >>(int16_t & data)
{
	if (checkSize(sizeof(data)))
	{
		data = ntohs(*reinterpret_cast<__packed const int16_t *>(&m_data[m_readPos]));
		m_readPos += sizeof(data);
	}

	return *this;
}

inline
Packet & Packet::operator >>(uint16_t & data)
{
	if (checkSize(sizeof(data)))
	{
		data = ntohs(*reinterpret_cast<__packed const uint16_t *>(&m_data[m_readPos]));
		m_readPos += sizeof(data);
	}

	return *this;
}

inline
Packet & Packet::operator >>(int32_t & data)
{
	if (checkSize(sizeof(data)))
	{
		data = ntohl(*reinterpret_cast<__packed const int32_t *>(&m_data[m_readPos]));
		m_readPos += sizeof(data);
	}

	return *this;
}

inline
Packet & Packet::operator >>(uint32_t & data)
{
	if (checkSize(sizeof(data)))
	{
		data = ntohl(*reinterpret_cast<__packed const uint32_t *>(&m_data[m_readPos]));
		m_readPos += sizeof(data);
	}

	return *this;
}

#if defined(ntohll)

inline
Packet & Packet::operator >>(int64_t & data)
{
	if (checkSize(sizeof(data)))
	{
		data = ntohll(*reinterpret_cast<__packed const int64_t *>(&m_data[m_readPos]));
		m_readPos += sizeof(data);
	}

	return *this;
}

inline
Packet & Packet::operator >>(uint64_t & data)
{
	if (checkSize(sizeof(data)))
	{
		data = ntohll(*reinterpret_cast<__packed const uint64_t *>(&m_data[m_readPos]));
		m_readPos += sizeof(data);
	}

	return *this;
}

#else

inline
Packet & Packet::operator >>(int64_t& data)
{
	if (checkSize(sizeof(data)))
	{
	    // Since ntohll is not available everywhere, we have to convert
	    // to network byte order (big endian) manually
		const uint8_t* bytes = reinterpret_cast<__packed const uint8_t *>(&m_data[m_readPos]);
		data = (static_cast<int64_t>(bytes[0]) << 56) |
		       (static_cast<int64_t>(bytes[1]) << 48) |
		       (static_cast<int64_t>(bytes[2]) << 40) |
		       (static_cast<int64_t>(bytes[3]) << 32) |
		       (static_cast<int64_t>(bytes[4]) << 24) |
		       (static_cast<int64_t>(bytes[5]) << 16) |
		       (static_cast<int64_t>(bytes[6]) <<  8) |
		       (static_cast<int64_t>(bytes[7]));
		m_readPos += sizeof(data);
	}

	return *this;
}

inline
Packet & Packet::operator >>(uint64_t& data)
{
	if (checkSize(sizeof(data)))
	{
	    // Since ntohll is not available everywhere, we have to convert
	    // to network byte order (big endian) manually
		const uint8_t* bytes = reinterpret_cast<__packed const uint8_t *>(&m_data[m_readPos]);
		data = (static_cast<uint64_t>(bytes[0]) << 56) |
		       (static_cast<uint64_t>(bytes[1]) << 48) |
		       (static_cast<uint64_t>(bytes[2]) << 40) |
		       (static_cast<uint64_t>(bytes[3]) << 32) |
		       (static_cast<uint64_t>(bytes[4]) << 24) |
		       (static_cast<uint64_t>(bytes[5]) << 16) |
		       (static_cast<uint64_t>(bytes[6]) <<  8) |
		       (static_cast<uint64_t>(bytes[7]));
		m_readPos += sizeof(data);
	}

	return *this;
}

#endif

inline
Packet & Packet::operator >>(float & data)
{
	if (checkSize(sizeof(data)))
	{
		data = *reinterpret_cast<__packed const float *>(&m_data[m_readPos]);
		m_readPos += sizeof(data);
	}

	return *this;
}

inline
Packet & Packet::operator >>(double & data)
{
	if (checkSize(sizeof(data)))
	{
		data = *reinterpret_cast<__packed const double *>(&m_data[m_readPos]);
		m_readPos += sizeof(data);
	}

	return *this;
}

inline
Packet & Packet::operator >>(char * data)
{
    // First extract string length
	uint32_t length = 0;
	*this >> length;

	if ((length > 0) && checkSize(length))
	{
		std::memcpy(data, &m_data[m_readPos], length); // Then extract characters
		data[length] = '\0';

		m_readPos += length; // Update reading position
	}

	return *this;
}

inline
Packet & Packet::operator >>(std::string & data)
{
	uint32_t length = 0;
	*this >> length; // First extract string length

	data.clear();
	if ((length > 0) && checkSize(length))
	{
		data.assign(&m_data[m_readPos], length); // Then extract characters
		
		m_readPos += length; // Update reading position
	}

	return *this;
}

inline
Packet & Packet::operator >>(wchar_t * data)
{
    // First extract string length
	uint32_t length = 0;
	*this >> length;

	if ((length > 0) && checkSize(length * sizeof(uint32_t)))
	{
	    // Then extract characters
		for (uint32_t i = 0; i < length; ++i)
		{
			uint32_t character = 0;
			*this >> character;
			data[i] = static_cast<wchar_t>(character);
		}
		data[length] = L'\0';
	}

	return *this;
}

inline
Packet & Packet::operator >>(std::wstring & data)
{
    // First extract string length
	uint32_t length = 0;
	*this >> length;

	data.clear();
	if ((length > 0) && checkSize(length * sizeof(uint32_t)))
	{
	    // Then extract characters
		for (uint32_t i = 0; i < length; ++i)
		{
			uint32_t character = 0;
			*this >> character;
			data += static_cast<wchar_t>(character);
		}
	}

	return *this;
}

inline
Packet & Packet::operator <<(bool data)
{
	*this << static_cast<uint8_t>(data);
	return *this;
}

inline
Packet & Packet::operator <<(int8_t data)
{
	Append(&data, sizeof(data));
	return *this;
}

inline
Packet & Packet::operator <<(uint8_t data)
{
	Append(&data, sizeof(data));
	return *this;
}

inline
Packet & Packet::operator <<(int16_t data)
{
	int16_t toWrite = htons(data);
	Append(&toWrite, sizeof(toWrite));
	return *this;
}

inline
Packet & Packet::operator <<(uint16_t data)
{
	uint16_t toWrite = htons(data);
	Append(&toWrite, sizeof(toWrite));
	return *this;
}

inline
Packet & Packet::operator <<(int32_t data)
{
	int32_t toWrite = htonl(data);
	Append(&toWrite, sizeof(toWrite));
	return *this;
}

inline
Packet & Packet::operator <<(uint32_t data)
{
	uint32_t toWrite = htonl(data);
	Append(&toWrite, sizeof(toWrite));
	return *this;
}

#if defined(htonll)

inline
Packet & Packet::operator <<(int64_t data)
{
	int64_t toWrite = htonll(data);
	Append(&toWrite, sizeof(toWrite));
	return *this;
}

inline
Packet & Packet::operator <<(uint64_t data)
{
	uint64_t toWrite = htonll(data);
	Append(&toWrite, sizeof(toWrite));
	return *this;
}

#else
inline
inline
Packet & Packet::operator <<(int64_t data)
{
    // Since htonll is not available everywhere, we have to convert
    // to network byte order (big endian) manually
	uint8_t toWrite[] =
	{
		static_cast<uint8_t>((data >> 56) & 0xFF),
		static_cast<uint8_t>((data >> 48) & 0xFF),
		static_cast<uint8_t>((data >> 40) & 0xFF),
		static_cast<uint8_t>((data >> 32) & 0xFF),
		static_cast<uint8_t>((data >> 24) & 0xFF),
		static_cast<uint8_t>((data >> 16) & 0xFF),
		static_cast<uint8_t>((data >>  8) & 0xFF),
		static_cast<uint8_t>((data) & 0xFF)
	};
	append(&toWrite, sizeof(toWrite));
	return *this;
}

inline
Packet & Packet::operator <<(uint64_t data)
{
    // Since htonll is not available everywhere, we have to convert
    // to network byte order (big endian) manually
	uint8_t toWrite[] =
	{
		static_cast<uint8_t>((data >> 56) & 0xFF),
		static_cast<uint8_t>((data >> 48) & 0xFF),
		static_cast<uint8_t>((data >> 40) & 0xFF),
		static_cast<uint8_t>((data >> 32) & 0xFF),
		static_cast<uint8_t>((data >> 24) & 0xFF),
		static_cast<uint8_t>((data >> 16) & 0xFF),
		static_cast<uint8_t>((data >>  8) & 0xFF),
		static_cast<uint8_t>((data) & 0xFF)
	};
	append(&toWrite, sizeof(toWrite));
	return *this;
}

#endif

inline
Packet & Packet::operator <<(float data)
{
	Append(&data, sizeof(data));
	return *this;
}

inline
Packet & Packet::operator <<(double data)
{
	Append(&data, sizeof(data));
	return *this;
}

inline
Packet & Packet::operator <<(const char* data)
{
    // First insert string length
	uint32_t length = static_cast<uint32_t>(std::strlen(data));
	*this << length;

	    // Then insert characters
	Append(data, length * sizeof(char));

	return *this;
}

inline
Packet & Packet::operator <<(const std::string& data)
{
    // First insert string length
	uint32_t length = static_cast<uint32_t>(data.size());
	*this << length;

	    // Then insert characters
	if (length > 0)
		Append(data.c_str(), length * sizeof(std::string::value_type));

	return *this;
}

inline
Packet & Packet::operator <<(const wchar_t* data)
{
    // First insert string length
	uint32_t length = static_cast<uint32_t>(std::wcslen(data));
	*this << length;

	    // Then insert characters
	for (const wchar_t* c = data; *c != L'\0'; ++c)
		*this << static_cast<uint32_t>(*c);

	return *this;
}

inline
Packet & Packet::operator <<(const std::wstring& data)
{
    // First insert string length
	uint32_t length = static_cast<uint32_t>(data.size());
	*this << length;

	    // Then insert characters
	if (length > 0)
	{
		for (std::wstring::const_iterator c = data.begin(); c != data.end(); ++c)
			*this << static_cast<uint32_t>(*c);
	}

	return *this;
}

inline
Packet::operator BoolType() const
{
	return m_isValid ? &Packet::checkSize : NULL;
}

inline
bool Packet::checkSize(std::size_t size)
{
	m_isValid = m_isValid && (m_readPos + size <= m_data.size());

	return m_isValid;
}

inline
void Packet::Reserve(uint32_t size)
{
	m_data.reserve(size);
}

#ifdef PACKET_NS
}
#endif
