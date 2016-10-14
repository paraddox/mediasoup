#ifndef MS_RTC_RTCP_PACKET_H
#define MS_RTC_RTCP_PACKET_H

#include "common.h"

#include "map"
#include "string"

namespace RTC
{
namespace RTCP
{
	enum class Type : uint8_t
	{
		FIR  = 192,
		NACK = 193,
		SR   = 200,
		RR   = 201,
		SDES = 202,
		BYE  = 203,
		APP  = 204,
		RTPFB = 205,
		PSFB = 206
	};

	static std::map<Type, std::string> type2String =
	{
		{ Type::FIR,   "FIR"   },
		{ Type::NACK,  "NACK"  },
		{ Type::SR,    "SR"    },
		{ Type::RR,    "RR"    },
		{ Type::SDES,  "SDES"  },
		{ Type::BYE,   "BYE"   },
		{ Type::APP,   "APP"   },
		{ Type::RTPFB, "RTPFB" },
		{ Type::PSFB,  "PSBF"  }
	};

	const std::string& Type2String(Type type);

	class Packet
	{
	public:
		/* Struct for RTCP common header. */
		struct CommonHeader
		{
			#if defined(MS_LITTLE_ENDIAN)
				uint8_t count:5;
				uint8_t padding:1;
				uint8_t version:2;
			#elif defined(MS_BIG_ENDIAN)
				uint8_t version:2;
				uint8_t padding:1;
				uint8_t count:5;
			#endif
			uint8_t packet_type:8;
			uint16_t length:16;
		};

	public:
		static bool IsRtcp(const uint8_t* data, size_t len);
		static Packet* Parse(const uint8_t* data, size_t len);


	public:
		Packet(Type type);
		virtual ~Packet();

		void SetNext(Packet* packet);
		Packet* GetNext();
		Type GetType();

	public:
		size_t Serialize();
		uint8_t* GetRaw();

	public:
		virtual void Dump() = 0;
		virtual size_t GetCount();
		virtual size_t GetSize() = 0;
		virtual size_t Serialize(uint8_t* data);

	private:
		Type type;
		Packet* next = nullptr;
		uint8_t* raw = nullptr;
	};

	/* Inline static methods. */

	inline
	bool Packet::IsRtcp(const uint8_t* data, size_t len)
	{
		CommonHeader* header = (CommonHeader*)data;

		return (
			(len >= sizeof(CommonHeader)) &&
			// DOC: https://tools.ietf.org/html/draft-ietf-avtcore-rfc5764-mux-fixes
			(data[0] > 127 && data[0] < 192) &&
			// RTP Version must be 2.
			(header->version == 2) &&
			// RTCP packet types defined by IANA:
			// http://www.iana.org/assignments/rtp-parameters/rtp-parameters.xhtml#rtp-parameters-4
			// RFC 5761 (RTCP-mux) states this range for secure RTCP/RTP detection.
			(header->packet_type >= 192 && header->packet_type <= 223)
		);
	}

	/* Inline instance methods. */

	inline
	Packet* Packet::GetNext()
	{
		return this->next;
	}

	inline
	void Packet::SetNext(Packet* packet)
	{
		this->next = packet;
	}

	inline
	size_t Packet::Serialize()
	{
		if (this->raw)
			delete this->raw;

		size_t size = this->GetSize();
		this->raw = new uint8_t[size];

		return this->Serialize(this->raw);
	}

	inline
	Type Packet::GetType()
	{
		return this->type;
	}

	inline
	size_t Packet::GetCount()
	{
		return 0;
	}

	inline
	uint8_t* Packet::GetRaw()
	{
		return this->raw;
	}
}
}

#endif
