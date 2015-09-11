#pragma once

#define TEST_INACTIVITY_TIMEOUT 20000	// ten seconds
#define NUMBER_OF_TESTS_IN_SUITE 9
#define MAX_NUMBER_OF_RESULTS_PER_TEST 8
#define TEMP_OPEN_CIRCUIT 10
#define TEMP_SHORT_CIRCUIT 30

#define TEST_RUNNING 25		// 'o'
#define TESTING_T 26		// 't'
#define TEST_PASSED 27		// 'P'
#define TEST_FAILED 21		// 'F'
#define TESTING_UNKN 28		// '?'
#define TESTING_BLANK 29	// ' '

#define AZIMUTH_MOTOR 1
#define ELEVATION_MOTOR 2

#define MAX_TIME_TO_REACH_AZIMUTH_LIMIT 90000 // in milliseconds. Max time for motor to reach West/East limit
#define MAX_TIME_TO_REACH_ELEVATION_LIMIT 90000 // in milliseconds. Max time for motor to reach Up/Down limit

#define testMoveEastErrorAzCountStill '1'
#define testMoveEastErrorAzCountMove '2'
#define testMoveWestErrorAzCountStill '1'
#define testMoveWestErrorAzCountMove '2'
#define testMoveUpErrorElCountStill '1'
#define testMoveUpErrorElCountMove '2'
#define testMoveDownErrorElCountStill '1'
#define testMoveDownErrorElCountMove '2'

#define testSensorPackCommError '1'
#define testInterconnectCommError '1'
#define testRailCommError '1'

#define testOpenCrctTempMod1Error '1'
#define testOpenCrctTempMod2Error '2'
#define testOpenCrctTempMod3Error '3'
#define testOpenCrctTempMod4Error '4'
#define testShortCrctTempMod1Error '5'
#define testShortCrctTempMod2Error '6'
#define testShortCrctTempMod3Error '7'
#define testShortCrctTempMod4Error '8'

#define testHomesBadStartPoint '1'
#define testHomes1Error '1'
#define testHomes2Error '2'
#define testHomes3Error '3'
#define testHomes4Error '4'
#define testHomes5Error '5'
#define testHomes6Error '6'
#define testHomes7Error '7'
#define testHomes8Error '8'

class TestSuite {
    public:
		char testResult[NUMBER_OF_TESTS_IN_SUITE][MAX_NUMBER_OF_RESULTS_PER_TEST];

		void init(void);
		void startTest(uint8_t finalTestIndex);
		void resetCommCounts(void);
		bool getTestState(void);
		void setTestState(bool testState);
		void testLoop(void);
		void resetInactivityTime(void);
		void pauseTest(void);
		void resumeTest(void);
		void nextTest(void);
		void previousTest(void);
		void stopTests(void);

    private:
		char testResultPtrHead[NUMBER_OF_TESTS_IN_SUITE];
		char testResultPtrTail[NUMBER_OF_TESTS_IN_SUITE];
		uint8_t testPtr,finalTest;
		int commCount;
		bool testModeActive,testRunning,testPaused;
		int32_t encoderCount0,encoderCount;
        uint32_t timeoutUpdate;
        uint8_t displayPtr;
        bool displayTestNumber;

        void initTestResultDisplay(void);
        void testMoveEast(void);
        void testMoveWest(void);
        void testMoveUp(void);
        void testMoveDown(void);
        void testSensorPackComm(void);
        void testInterconnectComm(void);
        void testRailControllerComm(void);
        void testModuleTemperature(void);
        void testHomes(void);
        void fullStop(void);
        void motorStillTest(uint8_t motorType);
        void moveWest(void);
        void moveEast(void);
        void moveUp(void);
        void moveDown(void);
        void runTest(uint8_t testIndex);
        void timeout(uint16_t milliseconds);
        void clearTestResults(void);
        uint8_t getTestResultPtrHead(uint8_t testIndex);
        uint8_t getTestResultPtrTail(uint8_t testIndex);
        void maintenanceLoop(void);
        void testResultDisplayLoop(void);
        void updateTestResultDisplay(void);
        bool assertEquals(int32_t value1, int32_t value2, char errorMsg);
        bool assertNotEquals(int32_t value1, int32_t value2, char errorMsg);
        bool assertGreaterThan(int32_t value1, int32_t value2, char errorMsg);
        bool assertLessThan(int32_t value1, int32_t value2, char errorMsg);
        bool assertTrue(int32_t value, char errorMsg);
        bool assertFalse(int32_t value, char errorMsg);
        void puts(char *str);
};

extern TestSuite testsuite;
