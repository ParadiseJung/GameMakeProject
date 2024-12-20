#pragma once

#include "Packet.h"

class ReqQuitPacket : public Packet {
public:
    ReqQuitPacket()
        : Packet(PacketHeader::REQ_QUIT)
    {
        setLength(getLength()); // 추가 데이터 없음
    }

    void serialize(char* buffer) const override
    {
        short length = getLength();
        PacketHeader header = getHeader();
        memcpy(buffer, &length, sizeof(length));
        buffer += sizeof(length);
        memcpy(buffer, &header, sizeof(header));
        buffer += sizeof(header);
        short delimiter = getDelimiter();
        memcpy(buffer, &delimiter, sizeof(delimiter));
    }

    void deserialize(const char* buffer) override
    {
        short length;
        memcpy(&length, buffer, sizeof(length));
        setLength(length);
        buffer += sizeof(length);

        PacketHeader hdr;
        memcpy(&hdr, buffer, sizeof(hdr));
        setHeader(hdr);
        buffer += sizeof(hdr);

        short delimiter;
        memcpy(&delimiter, buffer, sizeof(delimiter));
    }
};