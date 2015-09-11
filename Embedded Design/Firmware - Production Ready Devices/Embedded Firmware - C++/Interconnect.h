#pragma once

#define MODULES_PER_TRACKER		4

class InterconnectStats {
    public:
        void Reset(void);
        void Display(void);

        // Stats
        u32 cntInit;
        u32 cntRun;

        u32 cntCharacters;
        u32 cntValidPackets;
        u32 cntErrNoDelimiter;
        u32 cntErrMalformed;
        u32 cntErrorPackets;
        u32 cntOverruns;

        u32 cntAmpUpdates;
        u32 cntVoltageUpdates;
        u32 cntTemperatureUpdates;
        u32 cntDisplays;
};

class Interconnect {
    public:
        Interconnect() {
            flagInitialized = false;
            flagEnabled = false;
        }

    public:
        bool flagInitialized;
        bool flagEnabled;
        bool Init(void);
        bool Run(void);
        bool Reset(void);
        bool Requirements(void);
        void Test(void);
        bool Benchmark(void);
        bool ResetStats(void);
        int	commCount;

    public:
        void DisplayInfo(void);
        float GetAmps(void);
        float GetPcbTemperature(void);
        bool InitSpi(void);

        Module modules[MODULES_PER_TRACKER];
        InterconnectStats stats;			// must be public
        volatile SPI_REGS *device;
        bool flagInfoValid;
        u32 tInfoAge;
        float iTotal;
        float vLogic;
        float tCombinerPCB;
        bool flagValidPacket;
};

// Self-instantiate
extern Interconnect interconnect;

