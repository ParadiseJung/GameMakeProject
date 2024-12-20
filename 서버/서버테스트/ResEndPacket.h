#pragma once
#include "Packet.h"

class ResEndPacket : public Packet
{
private:
    char state;  // 1 or 2
public:
    ResEndPacket(char state)
        : Packet(PacketHeader::RES_END), state(state)
    {
        setLength(getLength() + sizeof(state));
    }

    char getState() const { return state; }

    void serialize(char* buffer) const override {
        short length = getLength();
        PacketHeader header = getHeader();
        memcpy(buffer, &length, sizeof(length));
        buffer += sizeof(length);
        memcpy(buffer, &header, sizeof(header));
        buffer += sizeof(header);
        memcpy(buffer, &state, sizeof(state));
        buffer += sizeof(state);
        short delimiter = getDelimiter();
        memcpy(buffer, &delimiter, sizeof(delimiter));
    }

    void deserialize(const char* buffer) override {
        short length;
        memcpy(&length, buffer, sizeof(length));
        setLength(length);
        buffer += sizeof(length);

        PacketHeader hdr;
        memcpy(&hdr, buffer, sizeof(hdr));
        setHeader(hdr);
        buffer += sizeof(hdr);

        memcpy(&state, buffer, sizeof(state));
        buffer += sizeof(state);

        short delimiter;
        memcpy(&delimiter, buffer, sizeof(delimiter));
    }
};