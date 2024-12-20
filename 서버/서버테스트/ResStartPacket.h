#pragma once
#include "Packet.h"

class ResStartPacket : public Packet
{
private:
    char roomId;  // 1 or 2
public:
    ResStartPacket(char rId)
        : Packet(PacketHeader::RES_START), roomId(rId)
    {
        setLength(getLength() + sizeof(roomId));
    }

    char getRoomId() const { return roomId; }

    void serialize(char* buffer) const override {
        short length = getLength();
        PacketHeader header = getHeader();
        memcpy(buffer, &length, sizeof(length));
        buffer += sizeof(length);
        memcpy(buffer, &header, sizeof(header));
        buffer += sizeof(header);
        memcpy(buffer, &roomId, sizeof(roomId));
        buffer += sizeof(roomId);
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

        memcpy(&roomId, buffer, sizeof(roomId));
        buffer += sizeof(roomId);

        short delimiter;
        memcpy(&delimiter, buffer, sizeof(delimiter));
    }
};