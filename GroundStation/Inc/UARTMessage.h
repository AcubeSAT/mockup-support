#ifndef GROUNDSTATION_UARTMESSAGE_H
#define GROUNDSTATION_UARTMESSAGE_H

#include <cstring>
#include "cobs.h"

class UARTMessage {
public:
    enum MessageType {
        Log = 1, // A log string
        SpacePacket = 2, // A CCSDS space packet
    };

private:
    char message[256]; ///< The message itself, with extra bytes for the extra information

    uint8_t length; ///< Length of the raw UART message in bytes
    MessageType messageType;

public:
    UARTMessage() = default;

    UARTMessage(const char *message, uint8_t length, MessageType messageType) : length(length), messageType(messageType) {
        if (length > 254) length = 254; // Sanity check, since the UARTMessage can't contain more than 255 bytes, given COBS encoding

        memcpy(this->message + 1, message, length);
    };

    /**
     * Use COBS (Consistent-Overhead Byte Stuffing) to packetize this message
     * @param output The output character buffer
     * @param outputSize The (maximum) size of the buffer, in bytes
     * @return The cobs encode result. Read the status and the encoded length from it.
     */
    cobs_encode_result encode(char* output, size_t outputSize) {
        // Append the message type to the message itself
        message[0] = static_cast<uint8_t>(messageType);

        cobs_encode_result result = cobs_encode(output, outputSize - 1, message, length);

        // Append a null byte to the end of the result, as dictated by COBS
        output[result.out_len] = '\0';
        result.out_len += 1; // Increase the output size to include this null byte

        return result;
    }
};

#endif //GROUNDSTATION_UARTMESSAGE_H
