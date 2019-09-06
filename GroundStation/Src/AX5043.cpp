#include <cstring>
#include <cstdio>
#include <mockup.hpp>
#include "AX5043.h"

#define AXRADIO_MODE_UNINIT                     0x00
#define AXRADIO_MODE_OFF                        0x01
#define AXRADIO_MODE_DEEPSLEEP                  0x02
#define AXRADIO_MODE_CW_TRANSMIT                0x03
#define AXRADIO_MODE_ASYNC_TRANSMIT             0x10
#define AXRADIO_MODE_WOR_TRANSMIT               0x11
#define AXRADIO_MODE_ACK_TRANSMIT               0x12
#define AXRADIO_MODE_WOR_ACK_TRANSMIT           0x13
#define AXRADIO_MODE_STREAM_TRANSMIT_UNENC      0x18
#define AXRADIO_MODE_STREAM_TRANSMIT_SCRAM      0x19
#define AXRADIO_MODE_STREAM_TRANSMIT_UNENC_LSB  0x1A
#define AXRADIO_MODE_STREAM_TRANSMIT_SCRAM_LSB  0x1B
#define AXRADIO_MODE_STREAM_TRANSMIT            0x1C
#define AXRADIO_MODE_ASYNC_RECEIVE              0x20
#define AXRADIO_MODE_WOR_RECEIVE                0x21
#define AXRADIO_MODE_ACK_RECEIVE                0x22
#define AXRADIO_MODE_WOR_ACK_RECEIVE            0x23
#define AXRADIO_MODE_STREAM_RECEIVE_UNENC       0x28
#define AXRADIO_MODE_STREAM_RECEIVE_SCRAM       0x29
#define AXRADIO_MODE_STREAM_RECEIVE_UNENC_LSB   0x2A
#define AXRADIO_MODE_STREAM_RECEIVE_SCRAM_LSB   0x2B
#define AXRADIO_MODE_STREAM_RECEIVE             0x2C
#define AXRADIO_MODE_STREAM_RECEIVE_DATAPIN     0x2D
#define AXRADIO_MODE_SYNC_MASTER                0x30
#define AXRADIO_MODE_SYNC_ACK_MASTER            0x31
#define AXRADIO_MODE_SYNC_SLAVE                 0x32
#define AXRADIO_MODE_SYNC_ACK_SLAVE             0x33
#define AX5043_AFSKCTRL 0x114   /* AFSK Control */
#define AX5043_AFSKMARK0 0x113   /* AFSK Mark (1) Frequency 0 */
#define AX5043_AFSKMARK1 0x112   /* AFSK Mark (1) Frequency 1 */
#define AX5043_AFSKSPACE0 0x111   /* AFSK Space (0) Frequency 0 */
#define AX5043_AFSKSPACE1 0x110   /* AFSK Space (0) Frequency 1 */
#define AX5043_AGCCOUNTER 0x043   /* AGC Counter */
#define AX5043_AMPLFILTER 0x115   /* Amplitude Filter */
#define AX5043_BBOFFSCAP 0x189   /* Baseband Offset Compensation Capacitors */
#define AX5043_BBTUNE 0x188   /* Baseband Tuning */
#define AX5043_BGNDRSSI 0x041   /* Background RSSI */
#define AX5043_BGNDRSSIGAIN 0x22E   /* Background RSSI Averaging Time Constant */
#define AX5043_BGNDRSSITHR 0x22F   /* Background RSSI Relative Threshold */
#define AX5043_CRCINIT0 0x017   /* CRC Initial Value 0 */
#define AX5043_CRCINIT1 0x016   /* CRC Initial Value 1 */
#define AX5043_CRCINIT2 0x015   /* CRC Initial Value 2 */
#define AX5043_CRCINIT3 0x014   /* CRC Initial Value 3 */
#define AX5043_DACCONFIG 0x332   /* DAC Configuration */
#define AX5043_DACVALUE0 0x331   /* DAC Value 0 */
#define AX5043_DACVALUE1 0x330   /* DAC Value 1 */
#define AX5043_DECIMATION 0x102   /* Decimation Factor  */
#define AX5043_DIVERSITY 0x042   /* Antenna Diversity Configuration */
#define AX5043_ENCODING 0x011   /* Encoding */
#define AX5043_FEC 0x018   /* Forward Error Correction */
#define AX5043_FECSTATUS 0x01A   /* Forward Error Correction Status */
#define AX5043_FECSYNC 0x019   /* Forward Error Correction Sync Threshold */
#define AX5043_FIFOCOUNT0 0x02B   /* Number of Words currently in FIFO 0 */
#define AX5043_FIFOCOUNT1 0x02A   /* Number of Words currently in FIFO 1 */
#define AX5043_FIFODATA 0x029   /* FIFO Data */
#define AX5043_FIFOFREE0 0x02D   /* Number of Words that can be written to FIFO 0 */
#define AX5043_FIFOFREE1 0x02C   /* Number of Words that can be written to FIFO 1 */
#define AX5043_FIFOSTAT 0x028   /* FIFO Control */
#define AX5043_FIFOTHRESH0 0x02F   /* FIFO Threshold 0 */
#define AX5043_FIFOTHRESH1 0x02E   /* FIFO Threshold 1 */
#define AX5043_FRAMING 0x012   /* Framing Mode */
#define AX5043_FREQA0 0x037   /* Frequency A 0 */
#define AX5043_FREQA1 0x036   /* Frequency A 1 */
#define AX5043_FREQA2 0x035   /* Frequency A 2 */
#define AX5043_FREQA3 0x034   /* Frequency A 3 */
#define AX5043_FREQB0 0x03F   /* Frequency B 0 */
#define AX5043_FREQB1 0x03E   /* Frequency B 1 */
#define AX5043_FREQB2 0x03D   /* Frequency B 2 */
#define AX5043_FREQB3 0x03C   /* Frequency B 3 */
#define AX5043_FSKDEV0 0x163   /* FSK Deviation 0 */
#define AX5043_FSKDEV1 0x162   /* FSK Deviation 1 */
#define AX5043_FSKDEV2 0x161   /* FSK Deviation 2 */
#define AX5043_FSKDMAX0 0x10D   /* Four FSK Rx Maximum Deviation 0 */
#define AX5043_FSKDMAX1 0x10C   /* Four FSK Rx Maximum Deviation 1 */
#define AX5043_FSKDMIN0 0x10F   /* Four FSK Rx Minimum Deviation 0 */
#define AX5043_FSKDMIN1 0x10E   /* Four FSK Rx Minimum Deviation 1 */
#define AX5043_GPADC13VALUE0 0x309   /* GPADC13 Value 0 */
#define AX5043_GPADC13VALUE1 0x308   /* GPADC13 Value 1 */
#define AX5043_GPADCCTRL 0x300   /* General Purpose ADC Control */
#define AX5043_GPADCPERIOD 0x301   /* GPADC Sampling Period */
#define AX5043_IFFREQ0 0x101   /* 2nd LO / IF Frequency 0 */
#define AX5043_IFFREQ1 0x100   /* 2nd LO / IF Frequency 1 */
#define AX5043_IRQINVERSION0 0x00B   /* IRQ Inversion 0 */
#define AX5043_IRQINVERSION1 0x00A   /* IRQ Inversion 1 */
#define AX5043_IRQMASK0 0x007   /* IRQ Mask 0 */
#define AX5043_IRQMASK1 0x006   /* IRQ Mask 1 */
#define AX5043_IRQREQUEST0 0x00D   /* IRQ Request 0 */
#define AX5043_IRQREQUEST1 0x00C   /* IRQ Request 1 */
#define AX5043_LPOSCCONFIG 0x310   /* Low Power Oscillator Calibration Configuration */
#define AX5043_LPOSCFREQ0 0x317   /* Low Power Oscillator Frequency Tuning Low Byte */
#define AX5043_LPOSCFREQ1 0x316   /* Low Power Oscillator Frequency Tuning High Byte */
#define AX5043_LPOSCKFILT0 0x313   /* Low Power Oscillator Calibration Filter Constant Low Byte */
#define AX5043_LPOSCKFILT1 0x312   /* Low Power Oscillator Calibration Filter Constant High Byte */
#define AX5043_LPOSCPER0 0x319   /* Low Power Oscillator Period Low Byte */
#define AX5043_LPOSCPER1 0x318   /* Low Power Oscillator Period High Byte */
#define AX5043_LPOSCREF0 0x315   /* Low Power Oscillator Reference Frequency Low Byte */
#define AX5043_LPOSCREF1 0x314   /* Low Power Oscillator Reference Frequency High Byte */
#define AX5043_LPOSCSTATUS 0x311   /* Low Power Oscillator Calibration Status */
#define AX5043_MATCH0LEN 0x214   /* Pattern Match Unit 0, Pattern Length */
#define AX5043_MATCH0MAX 0x216   /* Pattern Match Unit 0, Maximum Match */
#define AX5043_MATCH0MIN 0x215   /* Pattern Match Unit 0, Minimum Match */
#define AX5043_MATCH0PAT0 0x213   /* Pattern Match Unit 0, Pattern 0 */
#define AX5043_MATCH0PAT1 0x212   /* Pattern Match Unit 0, Pattern 1 */
#define AX5043_MATCH0PAT2 0x211   /* Pattern Match Unit 0, Pattern 2 */
#define AX5043_MATCH0PAT3 0x210   /* Pattern Match Unit 0, Pattern 3 */
#define AX5043_MATCH1LEN 0x21C   /* Pattern Match Unit 1, Pattern Length */
#define AX5043_MATCH1MAX 0x21E   /* Pattern Match Unit 1, Maximum Match */
#define AX5043_MATCH1MIN 0x21D   /* Pattern Match Unit 1, Minimum Match */
#define AX5043_MATCH1PAT0 0x219   /* Pattern Match Unit 1, Pattern 0 */
#define AX5043_MATCH1PAT1 0x218   /* Pattern Match Unit 1, Pattern 1 */
#define AX5043_MAXDROFFSET0 0x108   /* Maximum Receiver Datarate Offset 0 */
#define AX5043_MAXDROFFSET1 0x107   /* Maximum Receiver Datarate Offset 1 */
#define AX5043_MAXDROFFSET2 0x106   /* Maximum Receiver Datarate Offset 2 */
#define AX5043_MAXRFOFFSET0 0x10B   /* Maximum Receiver RF Offset 0 */
#define AX5043_MAXRFOFFSET1 0x10A   /* Maximum Receiver RF Offset 1 */
#define AX5043_MAXRFOFFSET2 0x109   /* Maximum Receiver RF Offset 2 */
#define AX5043_MODCFGA 0x164   /* Modulator Configuration A */
#define AX5043_MODCFGF 0x160   /* Modulator Configuration F */
#define AX5043_MODCFGP 0xF5F   /* Modulator Configuration P */
#define AX5043_MODULATION 0x010   /* Modulation */
#define AX5043_PINFUNCANTSEL 0x025   /* Pin Function ANTSEL */
#define AX5043_PINFUNCDATA 0x023   /* Pin Function DATA */
#define AX5043_PINFUNCDCLK 0x022   /* Pin Function DCLK */
#define AX5043_PINFUNCIRQ 0x024   /* Pin Function IRQ */
#define AX5043_PINFUNCPWRAMP 0x026   /* Pin Function PWRAMP */
#define AX5043_PINFUNCSYSCLK 0x021   /* Pin Function SYSCLK */
#define AX5043_PINSTATE 0x020   /* Pin State */
#define AX5043_PKTACCEPTFLAGS 0x233   /* Packet Controller Accept Flags */
#define AX5043_PKTCHUNKSIZE 0x230   /* Packet Chunk Size */
#define AX5043_PKTMISCFLAGS 0x231   /* Packet Controller Miscellaneous Flags */
#define AX5043_PKTSTOREFLAGS 0x232   /* Packet Controller Store Flags */
#define AX5043_PLLCPI 0x031   /* PLL Charge Pump Current */
#define AX5043_PLLCPIBOOST 0x039   /* PLL Charge Pump Current (Boosted) */
#define AX5043_PLLLOCKDET 0x182   /* PLL Lock Detect Delay */
#define AX5043_PLLLOOP 0x030   /* PLL Loop Filter Settings */
#define AX5043_PLLLOOPBOOST 0x038   /* PLL Loop Filter Settings (Boosted) */
#define AX5043_PLLRANGINGA 0x033   /* PLL Autoranging A */
#define AX5043_PLLRANGINGB 0x03B   /* PLL Autoranging B */
#define AX5043_PLLRNGCLK 0x183   /* PLL Autoranging Clock */
#define AX5043_PLLVCODIV 0x032   /* PLL Divider Settings */
#define AX5043_PLLVCOI 0x180   /* PLL VCO Current */
#define AX5043_PLLVCOIR 0x181   /* PLL VCO Current Readback */
#define AX5043_POWCTRL1 0xF08   /* Power Control 1 */
#define AX5043_POWIRQMASK 0x005   /* Power Management Interrupt Mask */
#define AX5043_POWSTAT 0x003   /* Power Management Status */
#define AX5043_POWSTICKYSTAT 0x004   /* Power Management Sticky Status */
#define AX5043_PWRAMP 0x027   /* PWRAMP Control */
#define AX5043_PWRMODE 0x002   /* Power Mode */
#define AX5043_RADIOEVENTMASK0 0x009   /* Radio Event Mask 0 */
#define AX5043_RADIOEVENTMASK1 0x008   /* Radio Event Mask 1 */
#define AX5043_RADIOEVENTREQ0 0x00F   /* Radio Event Request 0 */
#define AX5043_RADIOEVENTREQ1 0x00E   /* Radio Event Request 1 */
#define AX5043_RADIOSTATE 0x01C   /* Radio Controller State */
#define AX5043_REF 0xF0D   /* Reference */
#define AX5043_RSSI 0x040   /* Received Signal Strength Indicator */
#define AX5043_RSSIABSTHR 0x22D   /* RSSI Absolute Threshold */
#define AX5043_RSSIREFERENCE 0x22C   /* RSSI Offset */
#define AX5043_RXDATARATE0 0x105   /* Receiver Datarate 0 */
#define AX5043_RXDATARATE1 0x104   /* Receiver Datarate 1 */
#define AX5043_RXDATARATE2 0x103   /* Receiver Datarate 2 */
#define AX5043_SCRATCH 0x001   /* Scratch */
#define AX5043_SILICONREVISION 0x000   /* Silicon Revision */
#define AX5043_TIMER0 0x05B   /* 1MHz Timer 0 */
#define AX5043_TIMER1 0x05A   /* 1MHz Timer 1 */
#define AX5043_TIMER2 0x059   /* 1MHz Timer 2 */
#define AX5043_TMGRXAGC 0x227   /* Receiver AGC Settling Time */
#define AX5043_TMGRXBOOST 0x223   /* Receive PLL Boost Time */
#define AX5043_TMGRXCOARSEAGC 0x226   /* Receive Coarse AGC Time */
#define AX5043_TMGRXOFFSACQ 0x225   /* Receive Baseband DC Offset Acquisition Time */
#define AX5043_TMGRXPREAMBLE1 0x229   /* Receiver Preamble 1 Timeout */
#define AX5043_TMGRXPREAMBLE2 0x22A   /* Receiver Preamble 2 Timeout */
#define AX5043_TMGRXPREAMBLE3 0x22B   /* Receiver Preamble 3 Timeout */
#define AX5043_TMGRXRSSI 0x228   /* Receiver RSSI Settling Time */
#define AX5043_TMGRXSETTLE 0x224   /* Receive PLL (post Boost) Settling Time */
#define AX5043_TMGTXBOOST 0x220   /* Transmit PLL Boost Time */
#define AX5043_TMGTXSETTLE 0x221   /* Transmit PLL (post Boost) Settling Time */
#define AX5043_TRKAFSKDEMOD0 0x055   /* AFSK Demodulator Tracking 0 */
#define AX5043_TRKAFSKDEMOD1 0x054   /* AFSK Demodulator Tracking 1 */
#define AX5043_TRKAMPLITUDE0 0x049   /* Amplitude Tracking 0 */
#define AX5043_TRKAMPLITUDE1 0x048   /* Amplitude Tracking 1 */
#define AX5043_TRKDATARATE0 0x047   /* Datarate Tracking 0 */
#define AX5043_TRKDATARATE1 0x046   /* Datarate Tracking 1 */
#define AX5043_TRKDATARATE2 0x045   /* Datarate Tracking 2 */
#define AX5043_TRKFREQ0 0x051   /* Frequency Tracking 0 */
#define AX5043_TRKFREQ1 0x050   /* Frequency Tracking 1 */
#define AX5043_TRKFSKDEMOD0 0x053   /* FSK Demodulator Tracking 0 */
#define AX5043_TRKFSKDEMOD1 0x052   /* FSK Demodulator Tracking 1 */
#define AX5043_TRKPHASE0 0x04B   /* Phase Tracking 0 */
#define AX5043_TRKPHASE1 0x04A   /* Phase Tracking 1 */
#define AX5043_TRKRFFREQ0 0x04F   /* RF Frequency Tracking 0 */
#define AX5043_TRKRFFREQ1 0x04E   /* RF Frequency Tracking 1 */
#define AX5043_TRKRFFREQ2 0x04D   /* RF Frequency Tracking 2 */
#define AX5043_TXPWRCOEFFA0 0x169   /* Transmitter Predistortion Coefficient A 0 */
#define AX5043_TXPWRCOEFFA1 0x168   /* Transmitter Predistortion Coefficient A 1 */
#define AX5043_TXPWRCOEFFB0 0x16B   /* Transmitter Predistortion Coefficient B 0 */
#define AX5043_TXPWRCOEFFB1 0x16A   /* Transmitter Predistortion Coefficient B 1 */
#define AX5043_TXPWRCOEFFC0 0x16D   /* Transmitter Predistortion Coefficient C 0 */
#define AX5043_TXPWRCOEFFC1 0x16C   /* Transmitter Predistortion Coefficient C 1 */
#define AX5043_TXPWRCOEFFD0 0x16F   /* Transmitter Predistortion Coefficient D 0 */
#define AX5043_TXPWRCOEFFD1 0x16E   /* Transmitter Predistortion Coefficient D 1 */
#define AX5043_TXPWRCOEFFE0 0x171   /* Transmitter Predistortion Coefficient E 0 */
#define AX5043_TXPWRCOEFFE1 0x170   /* Transmitter Predistortion Coefficient E 1 */
#define AX5043_TXRATE0 0x167   /* Transmitter Bitrate 0 */
#define AX5043_TXRATE1 0x166   /* Transmitter Bitrate 1 */
#define AX5043_TXRATE2 0x165   /* Transmitter Bitrate 2 */
#define AX5043_WAKEUP0 0x06B   /* Wakeup Time 0 */
#define AX5043_WAKEUP1 0x06A   /* Wakeup Time 1 */
#define AX5043_WAKEUPFREQ0 0x06D   /* Wakeup Frequency 0 */
#define AX5043_WAKEUPFREQ1 0x06C   /* Wakeup Frequency 1 */
#define AX5043_WAKEUPTIMER0 0x069   /* Wakeup Timer 0 */
#define AX5043_WAKEUPTIMER1 0x068   /* Wakeup Timer 1 */
#define AX5043_WAKEUPXOEARLY 0x06E   /* Wakeup Crystal Oscillator Early */
#define AX5043_XTALAMPL 0xF11   /* Crystal Oscillator Amplitude Control */
#define AX5043_XTALCAP 0x184   /* Crystal Oscillator Load Capacitance */
#define AX5043_XTALOSC 0xF10   /* Crystal Oscillator Control */
#define AX5043_XTALSTATUS 0x01D   /* Crystal Oscillator Status */

#define AX5043_0xF00 0xF00
#define AX5043_0xF0C 0xF0C
#define AX5043_0xF18 0xF18
#define AX5043_0xF1C 0xF1C
#define AX5043_0xF21 0xF21
#define AX5043_0xF22 0xF22
#define AX5043_0xF23 0xF23
#define AX5043_0xF26 0xF26
#define AX5043_0xF30 0xF30
#define AX5043_0xF31 0xF31
#define AX5043_0xF32 0xF32
#define AX5043_0xF33 0xF33
#define AX5043_0xF34 0xF34
#define AX5043_0xF35 0xF35
#define AX5043_0xF44 0xF44

#define AX5043_AGCAHYST0 0x122   /* AGC Analog Hysteresis */
#define AX5043_AGCAHYST1 0x132   /* AGC Analog Hysteresis */
#define AX5043_AGCAHYST2 0x142   /* AGC Analog Hysteresis */
#define AX5043_AGCAHYST3 0x152   /* AGC Analog Hysteresis */
#define AX5043_AGCGAIN0 0x120   /* AGC Speed */
#define AX5043_AGCGAIN1 0x130   /* AGC Speed */
#define AX5043_AGCGAIN2 0x140   /* AGC Speed */
#define AX5043_AGCGAIN3 0x150   /* AGC Speed */
#define AX5043_AGCMINMAX0 0x123   /* AGC Analog Update Behaviour */
#define AX5043_AGCMINMAX1 0x133   /* AGC Analog Update Behaviour */
#define AX5043_AGCMINMAX2 0x143   /* AGC Analog Update Behaviour */
#define AX5043_AGCMINMAX3 0x153   /* AGC Analog Update Behaviour */
#define AX5043_AGCTARGET0 0x121   /* AGC Target */
#define AX5043_AGCTARGET1 0x131   /* AGC Target */
#define AX5043_AGCTARGET2 0x141   /* AGC Target */
#define AX5043_AGCTARGET3 0x151   /* AGC Target */
#define AX5043_AMPLITUDEGAIN0 0x12B   /* Amplitude Estimator Bandwidth */
#define AX5043_AMPLITUDEGAIN1 0x13B   /* Amplitude Estimator Bandwidth */
#define AX5043_AMPLITUDEGAIN2 0x14B   /* Amplitude Estimator Bandwidth */
#define AX5043_AMPLITUDEGAIN3 0x15B   /* Amplitude Estimator Bandwidth */
#define AX5043_BBOFFSRES0 0x12F   /* Baseband Offset Compensation Resistors */
#define AX5043_BBOFFSRES1 0x13F   /* Baseband Offset Compensation Resistors */
#define AX5043_BBOFFSRES2 0x14F   /* Baseband Offset Compensation Resistors */
#define AX5043_BBOFFSRES3 0x15F   /* Baseband Offset Compensation Resistors */
#define AX5043_DRGAIN0 0x125   /* Data Rate Estimator Bandwidth */
#define AX5043_DRGAIN1 0x135   /* Data Rate Estimator Bandwidth */
#define AX5043_DRGAIN2 0x145   /* Data Rate Estimator Bandwidth */
#define AX5043_DRGAIN3 0x155   /* Data Rate Estimator Bandwidth */
#define AX5043_FOURFSK0 0x12E   /* Four FSK Control */
#define AX5043_FOURFSK1 0x13E   /* Four FSK Control */
#define AX5043_FOURFSK2 0x14E   /* Four FSK Control */
#define AX5043_FOURFSK3 0x15E   /* Four FSK Control */
#define AX5043_FREQDEV00 0x12D   /* Receiver Frequency Deviation 0 */
#define AX5043_FREQDEV01 0x13D   /* Receiver Frequency Deviation 0 */
#define AX5043_FREQDEV02 0x14D   /* Receiver Frequency Deviation 0 */
#define AX5043_FREQDEV03 0x15D   /* Receiver Frequency Deviation 0 */
#define AX5043_FREQDEV10 0x12C   /* Receiver Frequency Deviation 1 */
#define AX5043_FREQDEV11 0x13C   /* Receiver Frequency Deviation 1 */
#define AX5043_FREQDEV12 0x14C   /* Receiver Frequency Deviation 1 */
#define AX5043_FREQDEV13 0x15C   /* Receiver Frequency Deviation 1 */
#define AX5043_FREQUENCYGAINA0 0x127   /* Frequency Estimator Bandwidth A */
#define AX5043_FREQUENCYGAINA1 0x137   /* Frequency Estimator Bandwidth A */
#define AX5043_FREQUENCYGAINA2 0x147   /* Frequency Estimator Bandwidth A */
#define AX5043_FREQUENCYGAINA3 0x157   /* Frequency Estimator Bandwidth A */
#define AX5043_FREQUENCYGAINB0 0x128   /* Frequency Estimator Bandwidth B */
#define AX5043_FREQUENCYGAINB1 0x138   /* Frequency Estimator Bandwidth B */
#define AX5043_FREQUENCYGAINB2 0x148   /* Frequency Estimator Bandwidth B */
#define AX5043_FREQUENCYGAINB3 0x158   /* Frequency Estimator Bandwidth B */
#define AX5043_FREQUENCYGAINC0 0x129   /* Frequency Estimator Bandwidth C */
#define AX5043_FREQUENCYGAINC1 0x139   /* Frequency Estimator Bandwidth C */
#define AX5043_FREQUENCYGAINC2 0x149   /* Frequency Estimator Bandwidth C */
#define AX5043_FREQUENCYGAINC3 0x159   /* Frequency Estimator Bandwidth C */
#define AX5043_FREQUENCYGAIND0 0x12A   /* Frequency Estimator Bandwidth D */
#define AX5043_FREQUENCYGAIND1 0x13A   /* Frequency Estimator Bandwidth D */
#define AX5043_FREQUENCYGAIND2 0x14A   /* Frequency Estimator Bandwidth D */
#define AX5043_FREQUENCYGAIND3 0x15A   /* Frequency Estimator Bandwidth D */
#define AX5043_FREQUENCYLEAK 0x116   /* Baseband Frequency Recovery Loop Leakiness */
#define AX5043_PHASEGAIN0 0x126   /* Phase Estimator Bandwidth */
#define AX5043_PHASEGAIN1 0x136   /* Phase Estimator Bandwidth */
#define AX5043_PHASEGAIN2 0x146   /* Phase Estimator Bandwidth */
#define AX5043_PHASEGAIN3 0x156   /* Phase Estimator Bandwidth */
#define AX5043_PKTADDR0 0x207   /* Packet Address 0 */
#define AX5043_PKTADDR1 0x206   /* Packet Address 1 */
#define AX5043_PKTADDR2 0x205   /* Packet Address 2 */
#define AX5043_PKTADDR3 0x204   /* Packet Address 3 */
#define AX5043_PKTADDRCFG 0x200   /* Packet Address Config */
#define AX5043_PKTADDRMASK0 0x20B   /* Packet Address Mask 0 */
#define AX5043_PKTADDRMASK1 0x20A   /* Packet Address Mask 1 */
#define AX5043_PKTADDRMASK2 0x209   /* Packet Address Mask 2 */
#define AX5043_PKTADDRMASK3 0x208   /* Packet Address Mask 3 */
#define AX5043_PKTLENCFG 0x201   /* Packet Length Configuration */
#define AX5043_PKTLENOFFSET 0x202   /* Packet Length Offset */
#define AX5043_PKTMAXLEN 0x203   /* Packet Maximum Length */
#define AX5043_RXPARAMCURSET 0x118   /* Receiver Parameter Current Set */
#define AX5043_RXPARAMSETS 0x117   /* Receiver Parameter Set Indirection */
#define AX5043_TIMEGAIN0 0x124   /* Time Estimator Bandwidth */
#define AX5043_TIMEGAIN1 0x134   /* Time Estimator Bandwidth */
#define AX5043_TIMEGAIN2 0x144   /* Time Estimator Bandwidth */
#define AX5043_TIMEGAIN3 0x154   /* Time Estimator Bandwidth */

// power states
#define AX5043_PWRSTATE_POWERDOWN           0x0
#define AX5043_PWRSTATE_DEEPSLEEP           0x1
#define AX5043_PWRSTATE_REGS_ON           	0x4
#define AX5043_PWRSTATE_XTAL_ON           	0x5
#define AX5043_PWRSTATE_FIFO_ON           	0x7
#define AX5043_PWRSTATE_SYNTH_RX            0x8
#define AX5043_PWRSTATE_FULL_RX             0x9
#define AX5043_PWRSTATE_WOR_RX           	0xb
#define AX5043_PWRSTATE_SYNTH_TX            0xc
#define AX5043_PWRSTATE_FULL_TX             0xd

//fifo commands
#define AX5043_FIFOCMD_NOP			0x00
#define AX5043_FIFOCMD_DATA			0x01
#define AX5043_FIFOCMD_REPEATDATA	0x02
#define AX5043_FIFOCMD_TIMER		0x10
#define AX5043_FIFOCMD_RSSI			0x11
#define AX5043_FIFOCMD_FREQOFFS		0x12
#define AX5043_FIFOCMD_RFFREQOFFS	0x13
#define AX5043_FIFOCMD_DATARATE		0x14
#define AX5043_FIFOCMD_ANTRSSI		0x15
#define AX5043_FIFOCMD_TXCTRL		0x1C
#define AX5043_FIFOCMD_TXPWR		0x1D

/**
 * Chunk Sizes
 */
#define AX_FIFO_CHUNK_NO_PAYLOAD	(0 << 5)
#define AX_FIFO_CHUNK_SINGLE_BYTE	(1 << 5)
#define AX_FIFO_CHUNK_TWO_BYTE		(2 << 5)
#define AX_FIFO_CHUNK_THREE_BYTE	(3 << 5)
#define AX_FIFO_CHUNK_VARIABLE		(7 << 5)

/**
 * Chunk Types
 */
#define AX_FIFO_CHUNK_NOP			0x00 /* No Operation */
#define AX_FIFO_CHUNK_DATA			0xE1 /* Data */
/* Transmit */
#define AX_FIFO_CHUNK_TXCTRL		0x3C /* Transmit Control (Antenna, Power Amp) */
#define AX_FIFO_CHUNK_REPEATDATA	0x62 /* Repeat Data */
#define AX_FIFO_CHUNK_TXPWR			0xFD /* Transmit Power */
/* Receive */
#define AX_FIFO_CHUNK_RSSI			0x31 /* RSSI */
#define AX_FIFO_CHUNK_FREQOFFS		0x52 /* Frequency Offset */
#define AX_FIFO_CHUNK_ANTRSSI2		0x55 /* Background Noise Calculation RSSI */
#define AX_FIFO_CHUNK_TIMER			0x70 /* Timer */
#define AX_FIFO_CHUNK_RFFREQOFFS	0x73 /* RF Frequency Offset */
#define AX_FIFO_CHUNK_DATARATE		0x74 /* Datarate */
#define AX_FIFO_CHUNK_ANTRSSI3		0x75 /* Antenna Selection RSSI */

/**
 * TXCTRL Command
 */
#define AX_FIFO_TXCTRL_SETTX	(1 << 6) /* Copy TXSE and TXDIFF to MODCFGA */
#define AX_FIFO_TXCTRL_TXSE		(1 << 5)
#define AX_FIFO_TXCTRL_TXDIFF	(1 << 4)
#define AX_FIFO_TXCTRL_SETANT	(1 << 3) /* Copy ANTSTATE to DIVERSITY */
#define AX_FIFO_TXCTRL_ANTSTATE	(1 << 2)
#define AX_FIFO_TXCTRL_SETPA	(1 << 1) /* Copy PASTATE to PWRAMP */
#define AX_FIFO_TXCTRL_PASTATE	(1 << 0)

/**
 * Transmit DATA Command
 */
#define AX_FIFO_TXDATA_UNENC	(1 << 5) /* Bypass framing and encoder */
#define AX_FIFO_TXDATA_RAW		(1 << 4) /* Bypass framing */
#define AX_FIFO_TXDATA_NOCRC	(1 << 3) /* Don't generate CRC */
#define AX_FIFO_TXDATA_RESIDUE	(1 << 2) /* Residue mode on last byte */
#define AX_FIFO_TXDATA_PKTEND	(1 << 1) /* END flag */
#define AX_FIFO_TXDATA_PKTSTART	(1 << 0) /* START flag */

/**
 * Receive DATA Command
 */
#define AX_FIFO_RXDATA_ABORT	(1 << 6) /* Packet has been aborted */
#define AX_FIFO_RXDATA_SIZEFAIL	(1 << 5) /* Size checks failed */
#define AX_FIFO_RXDATA_ADDRFAIL	(1 << 4) /* Address checks failed */
#define AX_FIFO_RXDATA_CRCFAIL	(1 << 3) /* CRC check failed */
#define AX_FIFO_RXDATA_RESIDUE	(1 << 2) /* Residue mode on last byte */
#define AX_FIFO_RXDATA_PKTEND	(1 << 1) /* END flag */
#define AX_FIFO_RXDATA_PKTSTART	(1 << 0) /* START flag */

char buf[50];

AX5043::AX5043(SPI_HandleTypeDef *spi, GPIO_TypeDef *nssGPIO, uint16_t nssPin) : SPIDevice(spi, nssGPIO, nssPin) {
    uartLog("Welcome to AX5043");
    reset();
}

//template<>
//inline void AX5043::writeReg(Register address, uint8_t data) {
//
//}

void AX5043::reset() {
    // Enable the NSS pin for at least 1 us
    slaveSelect();
    HAL_Delay(1);

    // Reset the chip
    writeReg(Register::PwrMode, PwrMode::RST);
    writeReg(Register::PwrMode, PwrMode::REFEN | PwrMode::XOEN);

    // Read the revision register
    auto value = readReg(Register::Revision);
    if (value != Revision::RevisionValue) {
        uartLog("Revision is wrong");
    }

    //LOG_DEBUG << "Revision register received: " << value;

    configReady();

    // Set some common TX/RX parameters
//    writeReg(Register::Encoding, EncodingPresets::NRZI);
//    writeReg(Register::Framing, Framing::FRMHDLC);
//    writeReg(Register::Modulation, Modulation::ModulationFSK);

//    writeReg(Register::Framing, Framing::FRMPatternMatch);
//    HAL_Delay(10); // Wait some time until the internal reference settles

//    writeReg(0xF00, 0x0f);
//    writeReg(0xF0C, 0x00);
//    writeReg(0xF0D, 0x03);
//    writeReg(0xF10, 0x04);
//    writeReg(0xF11, 0x00);
//    writeReg(0xF18, 0x06);
//    writeReg(0xF1C, 0x07);
//    writeReg(0xF21, 0x5C);
//    writeReg(0xF22, 0x53);
//    writeReg(0xF23, 0x76);
//    writeReg(0xF26, 0x92);
//    writeReg(0xF34, 0x28);
//    writeReg(0xF35, 0x11);
//    writeReg(0xF44, 0x24);
//    writeReg(0xF72, 0x00);
//    writeReg(0xF72, 0x00);
//    writeReg(0xF72, 0x00);

//    writeReg(0x105, 14629U & 0xFFU);
//    writeReg(0x104, (14629U >> 8U) & 0xFFU);
}

void AX5043::enterTransmitMode() {
    writeReg(AX5043_PLLLOOP                 ,0x09);
    writeReg(AX5043_PLLCPI                  ,0x02);
    writeReg(AX5043_PLLVCODIV               ,0x24);
    writeReg(AX5043_XTALCAP                 ,0x00);
    writeReg(AX5043_0xF00                   ,0x0F);
    writeReg(AX5043_0xF18                   ,0x06);

    performAutoranging();

    // Set power mode to FULLTX
    writeReg(Register::PwrMode, PwrModeValues::FULLTX | PwrMode::REFEN | PwrMode::XOEN);

    // Make sure that the internal voltage reference is enabled
    while (true) {
        auto powStat = readReg(Register::PowStat);
        if (powStat == All) {
            break;
        } else {
            //LOG_TRACE << "Waiting for transceiver, power status is " << powStat;
        }
    }

    // Make sure that the TCXO is oscillating
    while (true) {
        auto xtal = readReg(Register::XtalStatus);
        if (xtal == XTALRun) {
            break;
        } else {
            //LOG_TRACE << "Waiting for transceiver, TCXO status is " << xtal;
        }
    }

//    writeReg(Register::FIFOThresh0, 0x80);
    //LOG_DEBUG << "Switched to transmit mode.";
}

void AX5043::enterReceiveMode() {
    writeReg(AX5043_PLLLOOP                 , 0x0B);
    writeReg(AX5043_PLLCPI                  , 0x10);
    writeReg(AX5043_PLLVCODIV               , 0x25);
    writeReg(AX5043_XTALCAP                 , 0x00);
    writeReg(AX5043_0xF00                   , 0x0F);
    writeReg(AX5043_0xF18                   , 0x02);
    writeReg(AX5043_TMGRXAGC                , 0x00);
    writeReg(AX5043_TMGRXPREAMBLE1          , 0x00);
    writeReg(AX5043_PKTMISCFLAGS            , 0x00);
    writeReg(AX5043_RXPARAMSETS             , 0xFF);
    writeReg(AX5043_FREQDEV13               , 0x00);
    writeReg(AX5043_FREQDEV03               , 0x00);
    writeReg(AX5043_AGCGAIN3                , 0xD7);

    performAutoranging();

    // Set power mode to FULLRX
    writeReg(Register::PwrMode, PwrModeValues::FULLRX | PwrMode::REFEN | PwrMode::XOEN);

    // Make sure that the internal voltage reference is enabled
    while (true) {
        auto powStat = readReg(Register::PowStat);
        if (powStat == All) {
            break;
        } else {
            //LOG_TRACE << "Waiting for transceiver, power status is " << powStat;
        }
    }

    // Make sure that the TCXO is oscillating
    while (true) {
        auto xtal = readReg(Register::XtalStatus);
        if (xtal == XTALRun) {
            break;
        } else {
            //LOG_TRACE << "Waiting for transceiver, TCXO status is " << xtal;
        }
    }

//    writeReg(Register::FIFOThresh0, 0x80);
//    HAL_Delay(1);
    uartLog("Switched to receive mode");
    //LOG_DEBUG << "Switched to receive mode.";
}

void AX5043::performAutoranging() {
    // Set power mode to STANDBY
    writeReg(Register::PwrMode, PwrModeValues::STANDBY | PwrMode::REFEN | PwrMode::XOEN);

    // Make sure that the TCXO is oscillating
    while (true) {
        auto xtal = readReg(Register::XtalStatus);
        if (xtal == XTALRun) {
            break;
        } else {
            //LOG_TRACE << "Waiting for transceiver, TCXO status is " << xtal;
        }
    }

    //LOG_TRACE << "Ready to perform autoranging";

    // Write frequency for autoranging
    // FREQ = f / 48e6 * 2**24 + 1/2
    uint32_t value = 150295893; // 430.000 MHz
    writeReg(Register::FreqA3, (value >> 24U) & 0xffU);
    writeReg(Register::FreqA2, (value >> 16U) & 0xffU);
    writeReg(Register::FreqA1, (value >> 8U) & 0xffU);
    writeReg(Register::FreqA0, value & 0xffU);

    writeReg(Register::FreqB3, (value >> 24U) & 0xffU);
    writeReg(Register::FreqB2, (value >> 16U) & 0xffU);
    writeReg(Register::FreqB1, (value >> 8U) & 0xffU);
    writeReg(Register::FreqB0, value & 0xffU);

    // Enable RF divider
    writeReg(Register::PLLVCODiv, PllVCODiv::RFDIV | PllVCODiv::VCO2INT);

/*
    uint32_t bandwidth = 90e3;
    uint32_t decimation = (uint32_t)(((float) 48e6 / (16.0 * 2 * bandwidth)) + 0.5);
    if (decimation > 127) decimation = 127;
    //LOG_TRACE << "set decimation to " << decimation;
    writeReg(AX5043_DECIMATION, decimation);


    // ???
    uint32_t rx_data_rate = (uint32_t)(((float)48e6 * 128) / ((bandwidth * 2 * decimation)) + 0.5);
    uint32_t txrate = (uint32_t)((((float)bandwidth* (1U << 24U)) /
                                 (float)48e6) + 0.5);
    writeReg(Register::PLLLoop, 0xB);
    writeReg(Register::PLLCPI, 16);

    //LOG_TRACE << "set rx data rate to " << rx_data_rate;

//    writeReg(AX5043_MAXDROFFSET2, 0xFF);
//    writeReg(AX5043_MAXDROFFSET1, 0xFF);
//    writeReg(AX5043_MAXDROFFSET0, 0xFF);
    writeReg(AX5043_TXRATE0, txrate & 0xFFU);
    writeReg(AX5043_TXRATE1, (txrate >> 8U) & 0xFFU);
    writeReg(AX5043_TXRATE2, (txrate >> 16U) & 0xFFU);
    writeReg(AX5043_RXDATARATE0, rx_data_rate & 0xFFU);
    writeReg(AX5043_RXDATARATE1, (rx_data_rate >> 8U) & 0xFFU);
    writeReg(AX5043_RXDATARATE2, (rx_data_rate >> 16U) & 0xFFU);
//    writeReg(AX5043_FSKDEV2, 0x0F);
//    writeReg(AX5043_FSKDEV1, 0x0F);
//    writeReg(AX5043_FSKDEV0, 0x0F);
//    writeReg(AX5043_DIVERSITY, 0x03);
//    writeReg(AX5043_MATCH1PAT0, 0x7E);
//    writeReg(AX5043_MATCH1PAT1, 0x7E);
//    writeReg(AX5043_MATCH1LEN, 0x8A);
//    writeReg(AX5043_MATCH1MAX, 28);
//
//    writeReg(AX5043_MATCH0PAT0, 0x7E);
//    writeReg(AX5043_MATCH0PAT1, 0x7E);
//    writeReg(AX5043_MATCH0LEN, 0x8A);
//    writeReg(AX5043_MATCH0MAX, 28);
//
//    writeReg(AX5043_PKTMAXLEN, 0xFF);
//    writeReg(AX5043_PKTADDRCFG, 0xFF);
//    writeReg(AX5043_PKTLENCFG, 0xF0);
////    writeReg(AX5043_PKTLENOFFSET, 100);
    writeReg(AX5043_PKTACCEPTFLAGS, 0b101101);
//    writeReg(AX5043_MODCFGA, 0b00000101);
//    writeReg(AX5043_MODCFGF, 0b11);
    writeReg(AX5043_PKTCHUNKSIZE, 0b1101);
    writeReg(AX5043_MODCFGF, 0b11);

    // Set the FSK deviation
    uint32_t val = (uint32_t) ((((float) bandwidth / 2.0) / (float) 48e6) * (1 << 24))
          | 0x1;
    writeReg(AX5043_FSKDEV2, (val >> 16U) & 0xff);
    writeReg(AX5043_FSKDEV1, (val >> 8U) & 0xff);
    writeReg(AX5043_FSKDEV0, val & 0xff);

    writeReg(AX5043_PLLCPI, (uint8_t)(68 / 8.5));
    writeReg(AX5043_XTALCAP, 0);*/

//    writeReg(AX5043_PKTACCEPTFLAGS, 0b101101);

    // Start autoranging and acquire PLL lock
    writeReg(Register::PLLRangingA, readReg(Register::PLLRangingA) | PllRanging::RNGStart);

    // Make sure that the PLL lock is acquired
    while (true) {
        auto pllRanging = readReg(Register::PLLRangingA);
        if ((pllRanging & PllRanging::RNGErr) != 0) {
            //LOG_ERROR << "Could not lock PLL";
        } else if ((pllRanging & PllRanging::RNGStart) == 0) {
            //LOG_DEBUG << "Autoranging done";
            uartLog("Autoranging done");
            break;
        }
    }

    if ((readReg(Register::PLLRangingA) & PllRanging::RNGErr) != 0) {
        //LOG_ERROR << "Could not lock PLL after RNG started";
    }

    //LOG_TRACE << "PLL lock status is: " << (readReg(Register::PLLRangingA) & PllRanging::PLLLock);
}

void AX5043::transmitPacket(uint8_t *data, size_t length) {
    //LOG_TRACE << "Transmitting 0x" << length << " bytes";

    // Write TX preamble 00010001
    writeReg(Register::FIFOData, AX_FIFO_CHUNK_REPEATDATA);
    writeReg(Register::FIFOData, AX_FIFO_TXDATA_UNENC | AX_FIFO_TXDATA_RAW | AX_FIFO_TXDATA_NOCRC | 0);
    writeReg(Register::FIFOData, 30);
    writeReg(Register::FIFOData, 0x7E);


    //LOG_TRACE << "Number of words currently in FIFO: 0x" << readReg(Register::FIFOCount0) << " 0x" << readReg(Register::FIFOStat);

    writeReg(Register::FIFOData, AX_FIFO_CHUNK_DATA);
    writeReg(Register::FIFOData, static_cast<uint8_t>(length + 1 + 1));
    writeReg(Register::FIFOData, 0b00000011);
    writeReg(Register::FIFOData, length);
    for (size_t i = 0; i < length; i++) {
        writeReg(Register::FIFOData, data[i]);
    }

    //LOG_TRACE << "Number of words currently in FIFO: 0x" << readReg(Register::FIFOCount0) << " 0x" << readReg(Register::FIFOStat);

    // Now, commit the FIFO
    writeReg(Register::FIFOStat, FIFOCMD::Commit);

    //LOG_TRACE << "Data committed to FIFO";
    //LOG_TRACE << "Number of words currently in FIFO: 0x" << readReg(Register::FIFOCount0) << " 0x" << readReg(Register::FIFOStat);

    // Set power mode to FULLTX
//    writeReg(Register::PwrMode, PwrModeValues::FULLTX | PwrMode::REFEN | PwrMode::XOEN);
    // Make sure that the TCXO is oscillating
    while (true) {
        auto xtal = readReg(Register::XtalStatus);
        if (xtal == XTALRun) {
            break;
        } else {
            //LOG_TRACE << "Waiting for transceiver, TCXO status is " << xtal;
        }
    }

    // Wait until the fifo is processed
    while (true) {
        auto radioState = readReg(Register::RadioState);
        if (radioState == RadioState::Idle) {
            //LOG_DEBUG << "Data sent!";
            //LOG_TRACE << "Number of words currently in FIFO: 0x" << readReg(Register::FIFOCount0) << " 0x" << readReg(Register::FIFOStat);

            break;
        } else {
            //LOG_TRACE << "Waiting for radio state " << radioState;
        }
    }
}

uint8_t AX5043::receivePacket(uint8_t *data) {
    //LOG_TRACE << "Receiving bytes";

    while (true) {
        auto fifoCount = readReg(Register::FIFOCount0);
        if (fifoCount != 0) {
            //LOG_TRACE << "Fifo contains 0x" << fifoCount << " bytes";
            break;
        } else {
            //LOG_TRACE << "FIFO status: " << readReg(Register::FIFOStat);
            //LOG_TRACE << "Radio status: " << readReg(Register::RadioState);
            //LOG_TRACE << "Mode: " << readReg(Register::PwrMode);
            //LOG_TRACE << "PLL State: " << (readReg(Register::PLLRangingA) & PllRanging::PLLLock) ;
            //LOG_TRACE << "RSSI: " << (readReg(Register::RSSI)) << " " << (readReg(Register::BgndRSSI));
            return 0;
        }
    }

    auto preamble = readReg(Register::FIFOData);
    auto length = readReg(Register::FIFOData);
    auto header = readReg(Register::FIFOData);
    //LOG_TRACE << "FIFO Header: " << preamble << " " << length << " " << header;

    for (uint8_t i = 0; i < length; i++) {
        data[i] = readReg(Register::FIFOData);
//        //LOG_TRACE << "FIFO Data: " << data[i];
    }

    data[length] = '\0';


    //LOG_DEBUG << "\r\nGot data: " << data;

    //LOG_TRACE << "Fifo contains 0x" << readReg(Register::FIFOCount0) << " bytes";

    return length;
}

uint8_t AX5043::writeReg(uint16_t address, uint8_t data) {
    txBuffer[0] = WriteMask | 0b01110000U | ((address >> 8U) & 0xffU);
    txBuffer[1] = address & 0xffU;
    txBuffer[2] = data;

    slaveSelect();
    HAL_SPI_Transmit(spi, txBuffer, 3, HAL_MAX_DELAY);
    slaveUnselect();
}

void AX5043::configReady() {
    writeReg(AX5043_MODULATION              , 0x08);
    writeReg(AX5043_ENCODING                , 0x00);
    writeReg(AX5043_FRAMING                 , 0x14);
    writeReg(AX5043_FEC                     , 0x13);
    writeReg(AX5043_PINFUNCSYSCLK           , 0x01);
    writeReg(AX5043_PINFUNCDCLK             , 0x01);
    writeReg(AX5043_PINFUNCDATA             , 0x01);
    writeReg(AX5043_PINFUNCANTSEL           , 0x00);
    writeReg(AX5043_PINFUNCPWRAMP           , 0x07);
    writeReg(AX5043_WAKEUPXOEARLY           , 0x01);
    writeReg(AX5043_IFFREQ1                 , 0x02);
    writeReg(AX5043_IFFREQ0                 , 0x8F);
    writeReg(AX5043_DECIMATION              , 0x10);
    writeReg(AX5043_RXDATARATE2             , 0x00);
    writeReg(AX5043_RXDATARATE1             , 0x3E);
    writeReg(AX5043_RXDATARATE0             , 0x80);
    writeReg(AX5043_MAXDROFFSET2            , 0x00);
    writeReg(AX5043_MAXDROFFSET1            , 0x00);
    writeReg(AX5043_MAXDROFFSET0            , 0x00);
    writeReg(AX5043_MAXRFOFFSET2            , 0x80);
    writeReg(AX5043_MAXRFOFFSET1            , 0x03);
    writeReg(AX5043_MAXRFOFFSET0            , 0xE8);
    writeReg(AX5043_FSKDMAX1                , 0x00);
    writeReg(AX5043_FSKDMAX0                , 0xA6);
    writeReg(AX5043_FSKDMIN1                , 0xFF);
    writeReg(AX5043_FSKDMIN0                , 0x5A);
    writeReg(AX5043_AMPLFILTER              , 0x00);
    writeReg(AX5043_RXPARAMSETS             , 0xF4);
    writeReg(AX5043_AGCGAIN0                , 0xB5);
    writeReg(AX5043_AGCTARGET0              , 0x84);
    writeReg(AX5043_TIMEGAIN0               , 0xF8);
    writeReg(AX5043_DRGAIN0                 , 0xF2);
    writeReg(AX5043_PHASEGAIN0              , 0xC3);
    writeReg(AX5043_FREQUENCYGAINA0         , 0x0F);
    writeReg(AX5043_FREQUENCYGAINB0         , 0x1F);
    writeReg(AX5043_FREQUENCYGAINC0         , 0x0A);
    writeReg(AX5043_FREQUENCYGAIND0         , 0x0A);
    writeReg(AX5043_AMPLITUDEGAIN0          , 0x06);
    writeReg(AX5043_FREQDEV10               , 0x00);
    writeReg(AX5043_FREQDEV00               , 0x00);
    writeReg(AX5043_BBOFFSRES0              , 0x00);
    writeReg(AX5043_AGCGAIN1                , 0xB5);
    writeReg(AX5043_AGCTARGET1              , 0x84);
    writeReg(AX5043_AGCAHYST1               , 0x00);
    writeReg(AX5043_AGCMINMAX1              , 0x00);
    writeReg(AX5043_TIMEGAIN1               , 0xF6);
    writeReg(AX5043_DRGAIN1                 , 0xF1);
    writeReg(AX5043_PHASEGAIN1              , 0xC3);
    writeReg(AX5043_FREQUENCYGAINA1         , 0x0F);
    writeReg(AX5043_FREQUENCYGAINB1         , 0x1F);
    writeReg(AX5043_FREQUENCYGAINC1         , 0x0A);
    writeReg(AX5043_FREQUENCYGAIND1         , 0x0A);
    writeReg(AX5043_AMPLITUDEGAIN1          , 0x06);
    writeReg(AX5043_FREQDEV11               , 0x00);
    writeReg(AX5043_FREQDEV01               , 0x3C);
    writeReg(AX5043_FOURFSK1                , 0x16);
    writeReg(AX5043_BBOFFSRES1              , 0x00);
    writeReg(AX5043_AGCGAIN3                , 0xFF);
    writeReg(AX5043_AGCTARGET3              , 0x84);
    writeReg(AX5043_AGCAHYST3               , 0x00);
    writeReg(AX5043_AGCMINMAX3              , 0x00);
    writeReg(AX5043_TIMEGAIN3               , 0xF5);
    writeReg(AX5043_DRGAIN3                 , 0xF0);
    writeReg(AX5043_PHASEGAIN3              , 0xC3);
    writeReg(AX5043_FREQUENCYGAINA3         , 0x0F);
    writeReg(AX5043_FREQUENCYGAINB3         , 0x1F);
    writeReg(AX5043_FREQUENCYGAINC3         , 0x0D);
    writeReg(AX5043_FREQUENCYGAIND3         , 0x0D);
    writeReg(AX5043_AMPLITUDEGAIN3          , 0x06);
    writeReg(AX5043_FREQDEV13               , 0x00);
    writeReg(AX5043_FREQDEV03               , 0x3C);
    writeReg(AX5043_FOURFSK3                , 0x16);
    writeReg(AX5043_BBOFFSRES3              , 0x00);
    writeReg(AX5043_MODCFGF                 , 0x00);
    writeReg(AX5043_FSKDEV2                 , 0x00);
    writeReg(AX5043_FSKDEV1                 , 0x05);
    writeReg(AX5043_FSKDEV0                 , 0x76);
    writeReg(AX5043_MODCFGA                 , 0x05);
    writeReg(AX5043_TXRATE2                 , 0x00);
    writeReg(AX5043_TXRATE1                 , 0x10);
    writeReg(AX5043_TXRATE0                 , 0x62);
    writeReg(AX5043_TXPWRCOEFFB1            , 0x0F);
    writeReg(AX5043_TXPWRCOEFFB0            , 0xFF);
    writeReg(AX5043_PLLVCOI                 , 0x9A);
    writeReg(AX5043_PLLRNGCLK               , 0x05);
    writeReg(AX5043_BBTUNE                  , 0x0F);
    writeReg(AX5043_BBOFFSCAP               , 0x77);
    writeReg(AX5043_PKTADDRCFG              , 0x01);
    writeReg(AX5043_PKTLENCFG               , 0x80);
    writeReg(AX5043_PKTLENOFFSET            , 0x00);
    writeReg(AX5043_PKTMAXLEN               , 0xC8);
    writeReg(AX5043_MATCH0PAT3              , 0xAA);
    writeReg(AX5043_MATCH0PAT2              , 0xCC);
    writeReg(AX5043_MATCH0PAT1              , 0xAA);
    writeReg(AX5043_MATCH0PAT0              , 0xCC);
    writeReg(AX5043_MATCH1PAT1              , 0x7E);
    writeReg(AX5043_MATCH1PAT0              , 0x7E);
    writeReg(AX5043_MATCH1LEN               , 0x8A);
    writeReg(AX5043_MATCH1MAX               , 0x0A);
    writeReg(AX5043_TMGTXBOOST              , 0x5B);
    writeReg(AX5043_TMGTXSETTLE             , 0x3E);
    writeReg(AX5043_TMGRXBOOST              , 0x5B);
    writeReg(AX5043_TMGRXSETTLE             , 0x3E);
    writeReg(AX5043_TMGRXOFFSACQ            , 0x00);
    writeReg(AX5043_TMGRXCOARSEAGC          , 0x9C);
    writeReg(AX5043_TMGRXRSSI               , 0x03);
    writeReg(AX5043_TMGRXPREAMBLE2          , 0x17);
    writeReg(AX5043_RSSIABSTHR              , 0xE4);
    writeReg(AX5043_BGNDRSSITHR             , 0x00);
    writeReg(AX5043_PKTCHUNKSIZE            , 0x0D);
    writeReg(AX5043_PKTACCEPTFLAGS          , 0x20);
    writeReg(AX5043_DACVALUE1               , 0x00);
    writeReg(AX5043_DACVALUE0               , 0x00);
    writeReg(AX5043_DACCONFIG               , 0x00);
    writeReg(AX5043_REF                     , 0x03);
    writeReg(AX5043_XTALOSC                 , 0x04);
    writeReg(AX5043_XTALAMPL                , 0x00);
    writeReg(AX5043_0xF1C                   , 0x07);
    writeReg(AX5043_0xF21                   , 0x68);
    writeReg(AX5043_0xF22                   , 0xFF);
    writeReg(AX5043_0xF23                   , 0x84);
    writeReg(AX5043_0xF26                   , 0x98);
    writeReg(AX5043_0xF34                   , 0x28);
    writeReg(AX5043_0xF35                   , 0x11);
    writeReg(AX5043_0xF44                   , 0x25);
}


