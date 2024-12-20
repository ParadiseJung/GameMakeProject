#pragma once
#include "Packet.h"

class ReqRoundOverPacket : public Packet {
private:
	char playerNumber;
	RoundOver ROState;

public:
	ReqRoundOverPacket(const char& pNum, RoundOver ROState) : Packet(PacketHeader::REQ_RO), playerNumber(pNum), ROState(ROState)
	{
		setLength(getLength() + sizeof(playerNumber) + sizeof(ROState));
	}

	const char& getpNum() const { return playerNumber; }
	const RoundOver& getROState() const { return ROState; }

	void serialize(char* buffer) const override
	{
		short length = getLength();
		PacketHeader header = getHeader();
		std::memcpy(buffer, &length, sizeof(length));
		buffer += sizeof(getLength());
		std::memcpy(buffer, &header, sizeof(header));
		buffer += sizeof(PacketHeader);
		std::memcpy(buffer, &playerNumber, sizeof(playerNumber));
		buffer += sizeof(playerNumber);
		std::memcpy(buffer, &ROState, sizeof(ROState));
		buffer += sizeof(ROState);
		short delimiter = getDelimiter();
		std::memcpy(buffer, &delimiter, sizeof(delimiter));
	}

	void deserialize(const char* buffer) override
	{
		short length;
		std::memcpy(&length, buffer, sizeof(length));
		setLength(length);
		buffer += sizeof(length);

		PacketHeader header;
		std::memcpy(&header, buffer, sizeof(header));
		setHeader(header);
		buffer += sizeof(header);

		std::memcpy(&playerNumber, buffer, sizeof(playerNumber));
		buffer += sizeof(playerNumber);
		std::memcpy(&ROState, buffer, sizeof(ROState));
		buffer += sizeof(ROState);

		short delimiter;
		std::memcpy(&delimiter, buffer, sizeof(delimiter));
	}
};