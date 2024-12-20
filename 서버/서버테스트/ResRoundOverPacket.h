#pragma once
#include "Packet.h"

class ResRoundOverPacket : public Packet {
private:
	char player1Number;
	RoundOver p1ROState;
	char player2Number;
	RoundOver p2ROState;
	char checkState;

public:
	ResRoundOverPacket(const char& p1Num, RoundOver p1ROstate, const char& p2Num, RoundOver p2ROstate, const char& CheckState) 
		: Packet(PacketHeader::RES_RO), player1Number(p1Num), p1ROState(p1ROstate), player2Number(p2Num), p2ROState(p2ROstate), checkState(CheckState)
	{
		setLength(getLength() + sizeof(player1Number) + sizeof(p1ROState) + sizeof(player2Number) + sizeof(p2ROState) + sizeof(checkState));
	}

	const char& getp1Num() const { return player1Number; }
	const RoundOver& getp1ROState() const { return p1ROState; }
	const char& getp2Num() const { return player2Number; }
	const RoundOver& getp2ROState() const { return p2ROState; }
	const char& getCheckState() const { return checkState; }

	void serialize(char* buffer) const override
	{
		short length = getLength();
		PacketHeader header = getHeader();
		std::memcpy(buffer, &length, sizeof(length));
		buffer += sizeof(getLength());
		std::memcpy(buffer, &header, sizeof(header));
		buffer += sizeof(PacketHeader);
		std::memcpy(buffer, &player1Number, sizeof(player1Number));
		buffer += sizeof(player1Number);
		std::memcpy(buffer, &p1ROState, sizeof(p1ROState));
		buffer += sizeof(p1ROState);
		std::memcpy(buffer, &player2Number, sizeof(player2Number));
		buffer += sizeof(player2Number);
		std::memcpy(buffer, &p2ROState, sizeof(p2ROState));
		buffer += sizeof(p2ROState);
		std::memcpy(buffer, &checkState, sizeof(checkState));
		buffer += sizeof(checkState);
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

		std::memcpy(&player1Number, buffer, sizeof(player1Number));
		buffer += sizeof(player1Number);
		std::memcpy(&p1ROState, buffer, sizeof(p1ROState));
		buffer += sizeof(p1ROState);

		std::memcpy(&player2Number, buffer, sizeof(player2Number));
		buffer += sizeof(player2Number);
		std::memcpy(&p2ROState, buffer, sizeof(p2ROState));
		buffer += sizeof(p2ROState);

		std::memcpy(&checkState, buffer, sizeof(checkState));
		buffer += sizeof(checkState);

		short delimiter;
		std::memcpy(&delimiter, buffer, sizeof(delimiter));
	}
};