#include "stdio.h"
#include "string.h"
#include "System/System.h"
#include "IrdaBridge.h"
#include "Module.h"
#include "Interconnect.h"
#include "Console.h"

// Self-instantiate
Interconnect interconnect;

char spiBuf[128];
int idxSpiBuf = 0;

/// \brief Processes all characters from the ActiveCombiner and dispatches them to each module object.
///
/// \param[in] ch - character from the ActiveCombiner.
///
/// \return Nothing
void ProcessChar(char ch) {
    char name = 0;
    float value = 0.0;
    int idxColon = -1;

    // Each and every one counts.
    interconnect.stats.cntCharacters++;

//    // this code added for use with the Tracker Controller PCA tester
//    // the PCA tester has an Interconnect board specially programmed to send out "irdatest" once per second
//    if (spiBuf[0] == 'i' && spiBuf[1] == 'r' && spiBuf[2] == 'd' && spiBuf[3] == 'a' &&
//    	spiBuf[4] == 't' && spiBuf[5] == 'e' && spiBuf[6] == 's' && spiBuf[7] == 't') {
//        idxSpiBuf = 0;
//        goto NextMessage;
//    }

    // If we don't have a EOL but we do have room for it, stash the character.
    if ((ch != 0x0D) && (ch != 0x0A) && (idxSpiBuf < 127)) {
        spiBuf[idxSpiBuf++] = ch;
        return;
    }

    if (idxSpiBuf > 126) {
        idxSpiBuf = 0;
        interconnect.stats.cntOverruns++;
        return;
    }

    // Terminate the string.
    spiBuf[idxSpiBuf] = 0;

    // Get ready for the next string.
    idxSpiBuf = 0;

    // Find the colon that marks the 'center' of the message.
    for (int i = 0; i < sizeof(spiBuf); i++) {
        if (!spiBuf[i])
            break;
        if (spiBuf[i] == ':') {
            idxColon = i;
            break;
        }
    }

    // Did we find a colon?
    if (idxColon < 1) {
        interconnect.stats.cntErrNoDelimiter++;
        goto NextMessage;
    }

    // Was the first character valid.
    if ((spiBuf[idxColon - 2] != 'm') && (spiBuf[idxColon - 2] != 'p')) {
        interconnect.stats.cntErrMalformed++;
        goto NextMessage;
    }

    // Get the name of the value.
    name = spiBuf[idxColon - 1];

    // Capture the floating point value in the message.
    if (!sscanf(&spiBuf[idxColon + 1], "%f", &value))
        return;

    interconnect.commCount++;
    // Distribute the message.
    switch (name) {
        case '1':
            module1.voltage = value;
            module1.stats.cntUpdates++;
            interconnect.stats.cntValidPackets++;
            break;

        case '2':
            module2.voltage = value;
            module2.stats.cntUpdates++;
            interconnect.modules[1].stats.cntUpdates++;
            interconnect.stats.cntValidPackets++;
            break;

        case '3':
            module3.voltage = value;
            module3.stats.cntUpdates++;
            interconnect.stats.cntValidPackets++;
            break;

        case '4':
            module4.voltage = value;
            module4.stats.cntUpdates++;
            interconnect.stats.cntValidPackets++;
            break;

        case '5':
//			corner.module[4].vModule = value;
//			corner.module[4].stats.cntUpdates++;
            interconnect.stats.cntValidPackets++;
            break;

        case '6':
//			corner.module[5].vModule = value;
//			corner.module[5].stats.cntUpdates++;
            interconnect.stats.cntValidPackets++;
            break;

        case 'I':
            interconnect.iTotal = value;
            interconnect.stats.cntValidPackets++;
            interconnect.stats.cntAmpUpdates++;
            break;

        case 'V':
            interconnect.vLogic = value;
            interconnect.stats.cntValidPackets++;
            interconnect.stats.cntVoltageUpdates++;
            break;

        case 'T':
            interconnect.tCombinerPCB = value;
            interconnect.stats.cntValidPackets++;
            interconnect.stats.cntTemperatureUpdates++;
            break;

        default: {
            interconnect.stats.cntErrorPackets++;
            break;
        }
    }

    // Clear the buffer.
    NextMessage: for (int i = 0; i < sizeof(spiBuf); i++)
        spiBuf[i] = 0;
}

extern "C" {
interrupt void SOLAR_MODULE_SPIB_RX_ISR(void) {
    char ch;

    // Receive the character and process it.
    while (SpibRegs.SPIFFRX.bit.RXFFST) {
        ch = SpibRegs.SPIRXBUF;
        ch &= 0x00FF;
        if (ch)
            ProcessChar(ch);
    }

    // Clean up the interrupt flags.
    SpibRegs.SPIFFRX.bit.RXFFOVFCLR = 1;	// Clear FIFO rx overflow flag
    SpibRegs.SPIFFRX.bit.RXFFINTCLR = 1;	// Clear FIFO rx interrupt flag
    PieCtrlRegs.PIEACK.bit.ACK6 = 1;	// Ack to the PIE
}

interrupt void SOLAR_MODULE_SPIB_TX_ISR(void) {

    // Clean up the interrupt flags.
    SpibRegs.SPIFFTX.bit.TXFFINTCLR = 1;
    PieCtrlRegs.PIEACK.bit.ACK6 = 1;	// Ack to the PIE
}
}

/// \brief Returns the current (in amps).
///
/// This value will be the same for all modules.
///
/// \param[in] Nothing
///
/// \return Current (in amps) passing through the module
float Interconnect::GetAmps(void) {
    return iTotal;
}

/// \brief Computes the current temperature in the module.
///
/// \param[in] Nothing
///
/// \return Degrees from True North
float Interconnect::GetPcbTemperature(void) {
    return tCombinerPCB;
}

/// \brief Sends a message out the console.
///
/// \param[in] Nothing
///
/// \return Nothing
void Interconnect::DisplayInfo(void) {
#ifdef SYSTEM_CONSOLE
    sprintf(serBuf, "Post (%lu): %0.3fA, %0.3fV, %0.1fC\n\r", stats.cntDisplays++, iTotal, vLogic, tCombinerPCB);
    console.puts(serBuf);

    console.puts("   ");
    modules[0].DisplayInfo();
    console.puts("   ");
    modules[1].DisplayInfo();
    console.puts("   ");
    modules[2].DisplayInfo();
    console.puts("   ");
    modules[3].DisplayInfo();
#endif
}

/// \brief Initializes the SPI port used to talk to the Active Combiner.
///
/// \param[in] Nothing
///
/// \return false if error, otherwise true.
bool Interconnect::InitSpi(void) {

    // Turn on the pins.
    // Set up the GPIO to be SPI lines.
    EALLOW;
    GpioCtrlRegs.GPAPUD.bit.GPIO12 = 0;	// Enable pull-up on GPIO12 (SPISIMOB)
    GpioCtrlRegs.GPAPUD.bit.GPIO13 = 0;	// Enable pull-up on GPIO13 (SPISOMIB)
    GpioCtrlRegs.GPAPUD.bit.GPIO14 = 0;	// Enable pull-up on GPIO14 (SPICLKB)
    GpioCtrlRegs.GPAPUD.bit.GPIO15 = 0;	// Enable pull-up on GPIO15 (SPISTEB)
    GpioCtrlRegs.GPAQSEL1.bit.GPIO12 = 3;		// Async input GPIO12 (SPISIMOB)
    GpioCtrlRegs.GPAQSEL1.bit.GPIO13 = 3;		// Async input GPIO13 (SPISOMIB)
    GpioCtrlRegs.GPAQSEL1.bit.GPIO14 = 3;		// Async input GPIO14 (SPICLKB)
    GpioCtrlRegs.GPAQSEL1.bit.GPIO15 = 3;		// Async input GPIO15 (SPISTEB)
    GpioCtrlRegs.GPAMUX1.bit.GPIO12 = 3;		// Configure GPIO12 as SPISIMOB
    GpioCtrlRegs.GPAMUX1.bit.GPIO13 = 3;		// Configure GPIO13 as SPISOMIB
    GpioCtrlRegs.GPAMUX1.bit.GPIO14 = 3;		// Configure GPIO14 as SPICLKB
    GpioCtrlRegs.GPAMUX1.bit.GPIO15 = 3;		// Configure GPIO15 as SPISTEB
    EDIS;
    System.GPIOcnt[12]++;
    System.GPIOcnt[13]++;
    System.GPIOcnt[14]++;
    System.GPIOcnt[15]++;

    // Hold the device in reset while configuring its registers.
    device = &SpibRegs;
    device->SPICCR.bit.SPISWRESET = 0;			// Reset SPI

    // Set the SPI mode.
    device->SPICCR.bit.CLKPOLARITY = 0;			// Clock polarity = 0;
    device->SPICTL.bit.CLK_PHASE = 0;			// Clock phase = 0;
    device->SPICCR.bit.SPICHAR = 0x07;			// 8-bit character
    device->SPICTL.bit.MASTER_SLAVE = 0;			// slave
    device->SPICCR.bit.SPILBK = 0;			// No loopback
    device->SPICTL.bit.TALK = 0;			// Enable transmission
    device->SPISTS.all = 0;

    // Configure the FIFO's
    device->SPIFFTX.all = 0;
    device->SPIFFRX.all = 0;
    device->SPIFFTX.bit.SPIFFENA = 1;			// Enable the FIFO
    device->SPIFFTX.bit.TXFFIL = 0x1F;			// set TX FIFO level to 0
    device->SPIFFRX.bit.RXFFIL = 4;			// set RX FIFO level to 3

    // Set the transmission delay and emulation mode.
    device->SPIFFCT.all = 4;			// Set delay to n bits.
    device->SPIPRI.all = 0x0030;		// Free run during suspend
    device->SPIPRI.bit.STEINV = 0;

    // Set the baudrate
    device->SPIBRR = 0x0010;		// not used in Slave mode

    // Reset all device interrupts.
    // device->SPISTS.bit.INT_FLAG							// Not used in FIFO mode
    // device->SPISTS.bit.OVERRUN_FLAG						// Not used in FIFO mode
    // device->SPISTS.bit.BUFFULL_FLAG						// Not used in FIFO mode
    device->SPIFFTX.bit.TXFFINTCLR = 1;
    device->SPIFFRX.bit.RXFFOVFCLR = 1;
    device->SPIFFRX.bit.RXFFINTCLR = 1;

    // Enable SPI and reset the state machines.
    device->SPICCR.bit.SPISWRESET = 1;	// Enable after configuration is done.
//	device->SPIFFTX.bit.SPIRST				= 0;			// Reset the device (FIFO state machines?)
    device->SPIFFTX.bit.SPIRST = 1;
    device->SPIFFTX.bit.TXFIFO = 1;			// Reset the TX FIFO
    device->SPIFFRX.bit.RXFIFORESET = 1;			// Reset the RX FIFO

    // Clear pending PIE interrupts for our group.
    PieCtrlRegs.PIEACK.bit.ACK6 = 1;			// Ack the PIE

    // Install our ISR's
    EALLOW;
    PieVectTable.SPITXINTB = (PINT) &SOLAR_MODULE_SPIB_TX_ISR;
    PieVectTable.SPIRXINTB = (PINT) &SOLAR_MODULE_SPIB_RX_ISR;
    EDIS;

    // Enable the system/PIE interrupts required for this device
    PieCtrlRegs.PIEIER6.bit.INTx3 = 1;			// Enable PIE Group 6 - SPIA = 1, SPIB = 3
    PieCtrlRegs.PIEIER6.bit.INTx4 = 1;			// Enable PIE Group 6 - SPIA = 2, SPIB = 4
    PieCtrlRegs.PIECTRL.bit.ENPIE = 1;			// Enable the PIE block
    System.RegisterInterrupt(M_INT6);						// Enable CPU INT6

    // Enable SPI FIFO interrupts
    // device->SPICTL.bit.SPIINTENA			= 1;			// Not available in FIFO mode
    // device->SPICTL.bit.OVERRUNINTENA		= 1;			// Not available in FIFO mode
    device->SPIFFTX.bit.TXFFIENA = 0;
    device->SPIFFRX.bit.RXFFIENA = 1;

    return true;
}

/// \brief Called periodically from the while loop
///
/// Not implemented for this object.
///
/// \param[in] Nothing
///
/// \return Always true.
bool Interconnect::Run(void) {
    return true;
}

/// \brief Configure the object and underlying hardware.
///
/// All of the initialization for the object should be placed here.
///
/// \param[in] Nothing
///
/// \return False if an error is encountered, otherwise true.
bool Interconnect::Init(void) {

    // Check for a second initialization.
    if (flagInitialized) {
        System.FatalError();
    }

    // Make sure we have the other required modules.
    if (!Requirements()) {
        System.FatalError();
    }

    // Reset the object.
    Reset();

    // Initialize the hardware
    InitSpi();

    // Initialize each module.
    module1.init(1);
    module2.init(2);
    module3.init(3);
    module4.init(4);

    commCount = 0;

    // Ready and able
    flagInitialized = true;
    flagEnabled = true;

    return true;
}

/// \brief Reset the state of the object.
///
/// Safely reset the object to its state right after initialization
///
/// \param[in] Nothing
///
/// \return Always true.
bool Interconnect::Reset(void) {
    ResetStats();
    iTotal = 0.0;
    vLogic = 0.0;
    tCombinerPCB = 0.0;

    return true;
}

/// \brief Make sure that other required modules are present and initialized.
///
/// \param[in] no input arguments
///
/// \return True if all required objects are present.
bool Interconnect::Requirements(void) {
    return true;
}

/// \brief Test the object
///
/// Not implemented
///
/// \param[in] Nothing
///
/// \return Nothing, never returns.
void Interconnect::Test(void) {
}

/// \brief Benchmark the object
///
/// Not implemented
///
/// \param[in] Nothing
///
/// \return Always true
bool Interconnect::Benchmark(void) {
    return true;
}

/// \brief Reset the statistics for this object to zero.
///
/// All of the stats are reset to zero, may be called at any time.
///
/// \param[in] Nothing
///
/// \return Always true.
bool Interconnect::ResetStats(void) {
    stats.Reset();

    return true;
}

// ================================================================
//
// Implementation of the stats object.
//
// ================================================================

/// \brief Resets the stats in the stats object embedded in the parent.
///
/// This is the place to reset all of the statistics
///
/// \param[in] Nothing
///
/// \return Nothing
void InterconnectStats::Reset(void) {
    cntInit = 0;
    cntRun = 0;

    cntCharacters = 0;
    cntValidPackets = 0;
    cntErrNoDelimiter = 0;
    cntErrMalformed = 0;
    cntErrorPackets = 0;
    cntOverruns = 0;

    cntAmpUpdates = 0;
    cntVoltageUpdates = 0;
    cntTemperatureUpdates = 0;

    cntDisplays = 0;
}

/// \brief Display the stats out the console.
///
/// Not implemented
///
/// \param[in] Nothing
///
/// \return Nothing
void InterconnectStats::Display(void) {
}

