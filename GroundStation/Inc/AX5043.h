#ifndef AX5043_AX5043_H
#define AX5043_AX5043_H


#include "SPIDevice.h"

/**
 * An interface class for the AX5043 transceiver
 */
class AX5043 : SPIDevice {
private:
    enum Register : uint8_t {
        Revision = 0x000U, ///< Silicon Revision
        Scratch = 0x001U, ///< Scratch Register
        PwrMode = 0x002U, ///< Power Mode
        PowStat = 0x003U, ///< Power Management Status
        PowStickyStat = 0x004U, ///< Power Management Sticky Status
        PowIRQMask = 0x005U, ///< Power Management Interrupt Mask
        IRQMask1 = 0x006U, ///< IRQ Mask MSB
        IRQMask0 = 0x007U, ///< IRQ Mask LSB
        RadioEventMask1 = 0x008U, ///< Radio Event Mask MSB
        RadioEventMask0 = 0x009U, ///< Radio Event Mask LSB
        IRQInversion1 = 0x00AU, ///< IRQ Inversion MSB
        IRQInversion0 = 0x00BU, ///< IRQ Inversion LSB
        IRQRequest1 = 0x00CU, ///< IRQ Request MSB
        IRQRequest0 = 0x00DU, ///< IRQ Request LSB
        RadioEventReq1 = 0x00EU, ///< Radio Event Request MSB
        RadioEventReq0 = 0x00FU, ///< Radio Event Request LSB
        Modulation = 0x010U, ///< Modulation
        Encoding = 0x011U, ///< Encoder/Decoder Settings
        Framing = 0x012U, ///< Framing Settings
        CRCInit3 = 0x014U, ///< CRC Initialisation Data
        CRCInit2 = 0x015U, ///< CRC Initialisation Data
        CRCInit1 = 0x016U, ///< CRC Initialisation Data
        CRCInit0 = 0x017U, ///< CRC Initialisation Data
        FEC = 0x018U, ///< FEC (Viterbi) Configuration
        FECSync = 0x019U, ///< Interleaver Synchronisation Threshold
        FECStatus = 0x01AU, ///< FEC Status
        RadioState = 0x01CU, ///< Radio Controller State
        XtalStatus = 0x01DU, ///< Crystal Oscillator Status
        PinState = 0x020U, ///< Pinstate
        PinFuncSYSCLK = 0x021U, ///< SYSCLK Pin Function
        PinFuncDCLK = 0x022U, ///< DCLK Pin Function
        PinFuncData = 0x023U, ///< Data Pin Function
        PinFuncIRQ = 0x024U, ///< IRQ Pin Function
        PinFuncANTSEL = 0x025U, ///< ANTSEL Pin Function
        PinFuncPWRAMP = 0x026U, ///< PWRAMP Pin Function
        PwrAmp = 0x027U, ///< Power Amplifier Control
        FIFOStat = 0x028U, ///< FIFO Contorl
        FIFOData = 0x029U, ///< FIFO Data
        FIFOCount1 = 0x02AU, ///< Number of words currently in FIFO MSB
        FIFOCount0 = 0x02BU, ///< Number of words currently in FIFO LSB
        FIFOFree1 = 0x02CU, ///< Number of words that can be written to FIFO MSB
        FIFOFree0 = 0x02DU, ///< Number of words that can be written to FIFO LSB
        FIFOThresh1 = 0x02EU, ///< FIFO Threshold MSB
        FIFOThresh0 = 0x02FU, ///< FIFO Threshold LSB
        PLLLoop = 0x030U, ///< PLL Loop Filter Settings
        PLLCPI = 0x031U, ///< PLL Charge Pump Current (Boosted)
        PLLVCODiv = 0x032U, ///< PLL Divider Settings
        PLLRangingA = 0x033U, ///< PLL Autoranging
        FreqA3 = 0x034U, ///< Synthesizer Frequency
        FreqA2 = 0x035U, ///< Synthesizer Frequency
        FreqA1 = 0x036U, ///< Synthesizer Frequency
        FreqA0 = 0x037U, ///< Synthesizer Frequency
        PLLLoopBoost = 0x038U, ///< PLL Loop Filter Settings (Boosted)
        PLLCPIBoost = 0x039U, ///< PLL Charge Pump Current
        PLLRangingB = 0x03BU, ///< PLL Autoranging
        FreqB3 = 0x03CU, ///< Synthesizer Frequency
        FreqB2 = 0x03DU, ///< Synthesizer Frequency
        FreqB1 = 0x03EU, ///< Synthesizer Frequency
        FreqB0 = 0x03FU, ///< Synthesizer Frequency
        RSSI = 0x040U, ///< Received Signal Strength Indicator
        BgndRSSI = 0x041U, ///< Background RSSI
        Diversity = 0x042U, ///< Antenna Diversity Configuration
        AGCCounter = 0x043U, ///< AGC Current Value
    };

    static constexpr uint8_t All = 0xff;

    enum Revision : uint8_t {
        RevisionValue = 0b01010001U, ///< Silicon Revision Value
    };

    enum PwrMode : uint8_t {
        WDS = 1U << 4U, ///< Wakeup from Deep Sleep
        REFEN = 1U << 5U, ///< Reference enable
        XOEN = 1U << 6U, ///< Crystal oscillator enable
        RST = 1U << 7U, ///< Reset the whole chip
    };

    enum PwrModeValues : uint8_t { ///< @ref AX5043::Register::PwrMode bit values
        POWERDOWN = 0b0000U, ///< Powerdown; all circuits powered down
        DEEPSLEEP = 0b0001U, ///< Deep sleep mode
        STANDBY = 0b0101U, ///< Crystal oscillator enabled
        FIFOON = 0b0111U, ///< FIFO enabled
        SYNTHRX = 0b1000U, ///< Synthesizer running, Receive Mode
        FULLRX = 0b1001U, ///< Receiver running
        WORRX = 0b1011U, ///< Receiver Wake-on-Radio Mode
        SYNTHTX = 0b1100U, ///< Synthesizer running, Transmit Mode
        FULLTX = 0b1101U ///< Transmitter running
    };

    enum PowStat : uint8_t { ///< @ref AX5043::Register::PowStat power status bit masks
        SVIO = 1U, ///< IO Voltage Large Enough (not BrownOut)
        SBEVMODEM = 1U << 1U, ///< Modem Domain Voltage Brownout not Error
        SBEVANA = 1U << 2U, ///< Analog Domain Voltage Brownout not Error
        SVMODEM = 1U << 3U, ///< Modem Domain Voltage Regulator Ready
        SVANA = 1U << 4U, ///< Analog Domain Voltage Regulator Ready
        SVREF = 1U << 5U, ///< Reference Voltage Regulator Ready
        SREF = 1U << 6U, ///< Reference Ready
        SSUM = 1U << 7U, ///< Summary Ready Status
    };

    enum Modulation : uint8_t { ///< @ref AX5043::Register::Modulation
        RevrDone = 1U, ///< Receiver done
        ModulationASK = 0b0000U, ///< ASK
        ModulationASKCoherent = 0b0001U, ///< ASK Coherent
        ModulationPSK = 0b0100U, ///< PSK
        ModulationOQSK = 0b0110U, ///< OQSK
        ModulationMSK = 0b0111U, ///< MSK
        ModulationFSK = 0b1000U, ///< FSK
        Modulation4FSK = 0b1001U, ///< 4-FSK
        ModulationAFSK = 0b1010U, ///< AFSK
        ModulationFM = 0b1011, ///< FM
        RxHalfSpeed = 1U << 4U, ///< Halve the reception bitrate
    };

    enum Encoding : uint8_t { ///< @ref AX5043::Register::Encoding
        EncInv = 1U, ///< Invert data
        EncDiff = 1U << 1U, ///< Differential encode data
        EncScram = 1U << 2U, ///< Scrambler
        EncManch = 1U << 3U, ///< Manchester encoding/decoding
        EncNoSync = 1U << 4U, ///< Disable Dibit synchronisation in 4-FSK mode
    };

    enum EncodingPresets : uint8_t { ///< Value presets for \ref AX5043::Encodng
        NRZ = 0, ///< Non-Return-to-Zero
        NRZI = Encoding::EncInv | Encoding::EncDiff, ///< Non-Return-to-Zero Inverted
        FM1 = Encoding::EncInv | Encoding::EncDiff | Encoding::EncManch, ///< Biphase Mark
        FM0 = Encoding::EncDiff | Encoding::EncManch, ///< Biphase Space
        Manchester = Encoding::EncManch, ///< Manchester
    };

    enum Framing : uint8_t { ///< @ref AX5043::Register::Framing
        FAbort = 1U, ///< Abort current packet match
        FRMRaw = 0b000U << 1U, ///< Raw Frame Mode
        FRMSoftBits = 0b001U << 1U, ///< Raw, Soft Bits Frame Mode
        FRMHDLC = 0b010U << 1U, ///< HDLC [1] Frame Mode
        FRMPatternMatch = 0b011U << 1U, ///< Raw, Pattern Match Frame Mode
        FRMWirelessMBus = 0b100U << 1U, ///< Wireless M-Bus Frame Mode
        FRMWirelessMBus4to6 = 0b101U << 1U, ///< Wireless M-Bus, 4-to-6 Encoding
        CRCOff = 0b000U << 4U, ///< CRC off
        CRCCCITT = 0b001U << 4U, ///< CRC CCITT
        CRC16 = 0b010U << 4U, ///< CRC-16
        CRCDNP = 0b011 << 4U, ///< DNP (16 bit) CRC
        CRC32 = 0b110 << 4U, ///< CRC-32
        FRMRX = 1U << 7U, ///< Packet start detected, receiver running
    };

    enum RadioState : uint8_t { ///< @ref AX5043::Register::RadioState
        Idle = 0, ///< Idle
        Powerdown = 0b0001U, ///< Powerdown
        TxPLLSet = 0b0100, ///< Tx PLL Settings
        TxState = 0b0110, ///< Tx
        TxTail = 0b0111, ///< Tx Tail
        RxPLLSet = 0b1000, ///< Rx PLL Settings
        RxAntSel = 0b1001, ///< Rx Antenna Selection
        RxPreamble1 = 0b1100, ///< Rx Preamble 1
        RxPreamble2 = 0b1101, ///< Rx Preamble 2
        RxPreamble3 = 0b1110, ///< Rx Preamble 3
        Rx = 0b1111, ///< Rx
    };

    enum FIFOCMD : uint8_t { ///< @ref AX5043::Register::FIFOStat
        NOP = 0, ///< No Operation
        ASKCoherent = 0b00001, ///< ASK Coherent
        ClearError = 0b000010, ///< Clear FIFO Error Flags
        ClearFIFO = 0b000011, ///< Clear FIFO Data and Flags
        Commit = 0b000100, ///< Commit FIFO
        Rollback = 0b000101, ///< Rollback FIFO
    };

    enum XtalStatus : uint8_t { ///< @ref AX5043::Register::XtalStatus
        XTALRun = 1, ///< Crystal oscillator running and stable
    };

    enum PllVCODiv : uint8_t { ///< @ref AX5043:Register:PLLVCODiv
        REFDIV1 = 0, ///< fPD = fXTAL
        REFDIV2 = 0b01U, ///< fPD = fXTAL/2
        REFDIV4 = 0b10U, ///< fPD = fXTAL/4
        REFDIV8 = 0b11U, ///< fPD = fXTAL/8
        RFDIV = 1U << 2U, ///< RF divider
        VCOSEL = 1U << 4U, ///< Internal/external VCO selection
        VCO2INT = 1U << 5U, ///< Internal/external VCO selection
    };

    enum PllRanging : uint8_t { ///< @ref AX5043::Register::PLLRangingA, @ref AX5043::Register::PLLRangingB
        VCOR = 0b00001111, ///< VCO Range
        RNGStart = 1U << 4U, ///< PLL Autoranging start
        RNGErr = 1U << 5U, ///< PLL Autoranging error
        PLLLock = 1U << 6U, ///< PLL is locked
        StickyLock = 1U << 7U, ///< PLL didn't lose lock
    };
public:
    void performAutoranging();
public:
    AX5043(SPI_HandleTypeDef *spi, GPIO_TypeDef *nssGPIO, uint16_t nssPin);

    /**
     * Reset the AX5043 registers and its status
     */
    void reset();

    void configReady();

    /**
     * Enter AX5043 transmit synthesizer mode
     * @see AX5043::PwrModeValues::SYNTHTX
     */
    void enterTransmitMode();

    /**
     * Enter AX5043 full receive mode
     * @see AX5043::PwrModeValues::FULLRX
     */
    void enterReceiveMode();

    /**
     * Transmit a packet of data
     * @note This assumes that the transceiver is in transmit mode. See \ref AX5043::enterTransmitMode()
     * @param data The data to transmit in the packet
     * @param length The size of the data in bytes
     */
    void transmitPacket(uint8_t data[], size_t length);

    /**
     * Receive a packet of data
     * @note This assumes that the transceiver is in receive mode. See \ref AX5043::enterReceiveMode()
     * @param data The buffer of data. Should be at least 255 bytes long
     * @return The bytes of data returned
     */
    uint8_t receivePacket(uint8_t *data);

    /**
     * Write a single value of data to a single register
     * @param address The address of the register
     * @param data The data to write to the register
     */
    uint8_t writeReg(uint16_t address, uint8_t data);
};


#endif //AX5043_AX5043_H
