#pragma once

#include <iostream>

enum class PacketHeader : char {
	NON_HEAD,
	RES_START,
	REQ_COM,
	RES_COM,
	REQ_RO,
	RES_RO,
	RES_END,
	REQ_QUIT
};

enum class RoundOver : char {
	None,
	Win,
	Lose,
	Draw
};

class Packet {
private:
	PacketHeader header;
	short length;
	short delimiter;

protected:


public:
	Packet(PacketHeader header) : header(header), delimiter(0xff)
	{
		length = sizeof(length) + sizeof(header) + sizeof(delimiter);
	};

	virtual ~Packet() = default;

	virtual void serialize(char* buffer) const = 0;

	virtual void deserialize(const char* buffer) = 0;

	PacketHeader getHeader() const { return header; }

	short getLength() const { return length; }

	short getDelimiter() const { return delimiter; }

	void setLength(short len) { length = len; }

	void setHeader(PacketHeader hdr) { header = hdr; }
};