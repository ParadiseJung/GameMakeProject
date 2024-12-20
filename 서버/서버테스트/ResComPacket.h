#pragma once
#include "Packet.h"

class ResComPacket : public Packet {
private:
	char player1Number;
	char p1move;
	char p1attack;
	char player2Number;
	char p2move;
	char p2attack;
	char sec;
	char frame;

public:
	ResComPacket(const char& p1Num, const char& p1Move, const char& p1Attack, const char& p2Num, const char& p2Move, const char& p2Attack, const char& playSec, const char& playFrame)
		: Packet(PacketHeader::RES_COM), player1Number(p1Num), p1move(p1Move), p1attack(p1Attack), player2Number(p2Num), p2move(p2Move), p2attack(p2Attack), sec(playSec), frame(playFrame)
	{
		setLength(getLength() + sizeof(player1Number) + sizeof(p1move) + sizeof(p1attack) + sizeof(player2Number) + sizeof(p2move) + sizeof(p2attack) + sizeof(sec) + sizeof(playFrame));
	}

	const char& getp1Num() const { return player1Number; }
	const char& getp1Move() const { return p1move; }
	const char& getp1Attack() const { return p1attack; }
	const char& getp2Num() const { return player2Number; }
	const char& getp2Move() const { return p2move; }
	const char& getp2Attack() const { return p2attack; }
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
		std::memcpy(buffer, &player1Number, sizeof(player1Number));
		buffer += sizeof(player1Number);
		std::memcpy(buffer, &p1move, sizeof(p1move));
		buffer += sizeof(p1move);
		std::memcpy(buffer, &p1attack, sizeof(p1attack));
		buffer += sizeof(p1attack);
		std::memcpy(buffer, &player2Number, sizeof(player2Number));
		buffer += sizeof(player2Number);
		std::memcpy(buffer, &p2move, sizeof(p2move));
		buffer += sizeof(p2move);
		std::memcpy(buffer, &p2attack, sizeof(p2attack));
		buffer += sizeof(p2attack);
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

		std::memcpy(&player1Number, buffer, sizeof(player1Number));
		buffer += sizeof(player1Number);
		std::memcpy(&p1move, buffer, sizeof(p1move));
		buffer += sizeof(p1move);
		std::memcpy(&p1attack, buffer, sizeof(p1attack));
		buffer += sizeof(p1attack);

		std::memcpy(&player2Number, buffer, sizeof(player2Number));
		buffer += sizeof(player2Number);
		std::memcpy(&p2move, buffer, sizeof(p2move));
		buffer += sizeof(p2move);
		std::memcpy(&p2attack, buffer, sizeof(p2attack));
		buffer += sizeof(p2attack);
		std::memcpy(&sec, buffer, sizeof(sec));
		buffer += sizeof(sec);
		std::memcpy(&frame, buffer, sizeof(frame));
		buffer += sizeof(frame);

		short delimiter;
		std::memcpy(&delimiter, buffer, sizeof(delimiter));
	}

	~ResComPacket() {}
};