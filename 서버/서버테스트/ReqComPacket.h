#pragma once
#include "Packet.h"

class ReqComPacket : public Packet {
private:
	char playerNumber;
	char move;
	char attack;
	char sec;
	char frame;

public:
	ReqComPacket(const char& pNum, const char& pMove, const char& pAttack, const char& playSec, const char& playFrame) 
		: Packet(PacketHeader::REQ_COM), playerNumber(pNum),move(pMove), attack(pAttack), sec(playSec), frame(playFrame)
	{
		setLength(getLength() + sizeof(playerNumber) + sizeof(move) + sizeof(attack) + sizeof(sec) + sizeof(playFrame));
	}

	const char& getpNum() const { return playerNumber; }
	const char& getMove() const { return move; }
	const char& getAttack() const { return attack; }
	const char& getSec() const { return sec; }
	const char& getFrame() const { return frame; }

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
		std::memcpy(buffer, &move, sizeof(move));
		buffer += sizeof(move);
		std::memcpy(buffer, &attack, sizeof(attack));
		buffer += sizeof(attack);
		std::memcpy(buffer, &sec, sizeof(sec));
		buffer += sizeof(sec);
		std::memcpy(buffer, &frame, sizeof(frame));
		buffer += sizeof(frame);
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
		std::memcpy(&move, buffer, sizeof(move));
		buffer += sizeof(move);
		std::memcpy(&attack, buffer, sizeof(attack));
		buffer += sizeof(attack);
		std::memcpy(&sec, buffer, sizeof(sec));
		buffer += sizeof(sec);
		std::memcpy(&frame, buffer, sizeof(frame));
		buffer += sizeof(frame);

		short delimiter;
		std::memcpy(&delimiter, buffer, sizeof(delimiter));
	}

	~ReqComPacket() {}
};