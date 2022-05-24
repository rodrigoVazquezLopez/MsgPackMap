#ifndef MsgPackMap_h
#define MsgPackMap_h

#include "Arduino.h"

#define MAX_SUBMAPS 5

class MsgPackMap
{
    public:
        MsgPackMap(byte buf[], uint16_t bufSize);
        uint16_t getMapSize();
        uint8_t readNumElements();
        void setStream(Stream &serial);
        bool isKeyAvailable(const char keyStr[]);

        void printRawData();
        void printRawData(int numCol);
        void writeData();
        void clearData();

        void beginMap();
        void beginSubMap(const char keyStr[]);
        void endSubMap();

        void addInteger(const char keyStr[], uint8_t data);
        void addInteger(const char keyStr[], uint16_t data);
        void addInteger(const char keyStr[], uint32_t data);
        void addInteger(const char keyStr[], int8_t data);
        void addInteger(const char keyStr[], int16_t data);
        void addInteger(const char keyStr[], int32_t data);
        void addFloat(const char keyStr[], float data);
        void addString(const char keyStr[], const char data[]);
        void addBool(const char keyStr[], bool data);
        void addNull(const char keyStr[]); //nil
        void addByte(const char keyStr[], byte data[],uint8_t dataSize);
        void addFloatArray(const char keyStr[], float data[],uint8_t dataSize);
        void addIntegerArray(const char keyStr[], uint8_t data[],uint8_t dataSize);
        void addIntegerArray(const char keyStr[], uint16_t data[],uint8_t dataSize);
        void addIntegerArray(const char keyStr[], uint32_t data[],uint8_t dataSize);
        void addIntegerArray(const char keyStr[], int8_t data[],uint8_t dataSize);
        void addIntegerArray(const char keyStr[], int16_t data[],uint8_t dataSize);
        void addIntegerArray(const char keyStr[], int32_t data[],uint8_t dataSize);

        uint8_t readUnsignedInt8(const char keyStr[]);
        uint16_t readUnsignedInt16(const char keyStr[]);
        uint32_t readUnsignedInt32(const char keyStr[]);
        int8_t readInt8(const char keyStr[]);
        int16_t readInt16(const char keyStr[]);
        int32_t readInt32(const char keyStr[]);
        float readFloat(const char keyStr[]);
        String readString(const char keyStr[]);
        bool readBool(const char keyStr[]);
        bool readByte(const char keyStr[], byte buf[], uint8_t bufSize);
        bool readFloatArray(const char keyStr[], float buf[], uint8_t bufSize);

    private:
        byte *buffer; // Apuntador a la estructura serializada
        Stream *_serial; // Apuntador a
        uint16_t bufferSize;
        uint8_t numElements = 0;
        uint16_t startPos = 0;
        uint16_t bufferPos = 0;
        uint8_t level = 0;
        uint16_t positions[MAX_SUBMAPS];
        uint8_t elements[MAX_SUBMAPS];

        union decimal
        {
            float num;
            byte numBytes[4];
        } dec;

        void serializeInteger(uint8_t data);
        void serializeInteger(uint16_t data);
        void serializeInteger(uint32_t data);
        void serializeInteger(int8_t data);
        void serializeInteger(int16_t data);
        void serializeInteger(int32_t data);
        void serializeFloat(float data);
        void serializeString(const char data[]);
        void serializeBool(bool data);
        void serializeByte(byte data[], uint8_t dataSize);
        void serializeFloatArray(float data[], uint8_t dataSize);
        void serializeIntegerArray(uint8_t data[], uint8_t dataSize);
        void serializeIntegerArray(uint16_t data[], uint8_t dataSize);
        void serializeIntegerArray(uint32_t data[], uint8_t dataSize);
        void serializeIntegerArray(int8_t data[], uint8_t dataSize);
        void serializeIntegerArray(int16_t data[], uint8_t dataSize);
        void serializeIntegerArray(int32_t data[], uint8_t dataSize);
        void serializeNil();
        bool rearrageBuffer();

        uint8_t deserializeUnsignedInt8(int pos);
        uint16_t deserializeUnsignedInt16(int pos);
        uint32_t deserializeUnsignedInt32(int pos);
        int8_t deserializeInt8(int pos);
        int16_t deserializeInt16(int pos);
        int32_t deserializeInt32(int pos);
        float deserializeFloat(int pos);
        String deserializeString(int pos);
        bool deserializeBool(int pos);
        void deserializeByte(int pos, byte buf[], uint8_t bufSize);
        void deserializeFloatArray(int pos, float buf[], uint8_t bufSize);
        bool isEqual(uint8_t pos, const char keyStr[]);
        int getDataPosition(const char keyStr[]);

};
#endif // MSGPACK_H
