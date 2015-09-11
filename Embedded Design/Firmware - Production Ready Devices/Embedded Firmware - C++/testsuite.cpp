#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ProjectConfig.h"
#include "System/System.h"
#include "Command.h"
#include "Tracker.h"
#include "Router.h"
#include "Voltmeter.h"
#include "MotorController.h"
#include "MotorAzimuth.h"
#include "MotorElevation.h"
#include "Module.h"
#include "IRRemote.h"
#include "Console.h"
#include "Display.h"
#include "RailComm.h"
#include "SensorPack.h"
#include "Interconnect.h"
#include "testsuite.h"

// Self-instantiate
TestSuite testsuite;

// ******************************************
void TestSuite::startTest(uint8_t finalTestIndex) {
	// init variables and pointers for running the test(s)
	// finalTestIndex is passed in 1-indexed, but all variables/pointers are 0-indexed

	testModeActive = true;
	testRunning = true;
	testPaused = false;
	displayTestNumber = true;
	displayPtr = 0;
	// if finalTestIndex is zero, run all tests
	if (!finalTestIndex) {
		testPtr = 0;
		finalTest = (NUMBER_OF_TESTS_IN_SUITE-1);
	}
	else if (finalTestIndex <= NUMBER_OF_TESTS_IN_SUITE) {
		testPtr = finalTestIndex-1;
		finalTest = finalTestIndex-1;
	}
    timeoutUpdate = millis();
}
// ******************************************
void TestSuite::testLoop(void) {
	// while we are in test mode run this loop instead of the main loop in application.cpp

	while (testModeActive) {
		// separate loop used to keep up with system-wide maintenance tasks
		maintenanceLoop();

		// if we're running a test and it's not paused, then run it
		if ((testRunning) && (!testPaused)) {
			runTest(testPtr);
			if (testPtr == finalTest) {
				testRunning = false;
				testPtr = NUMBER_OF_TESTS_IN_SUITE; // by setting it beyond the reach of the displayPtr we force the two pointers to be not equal, letting the test results display cycle fully
			}
			else {
				testPtr++;
			}
		}
		// otherwise, just chill here and run the loop every millisecond
		else {
//			System.RTC.Sleep(MILLISECOND);
			System.RTC.Sleep(10000);
		}
	}
}
// ******************************************
void TestSuite::maintenanceLoop(void) {
	// loop used to keep up with system-wide maintenance tasks

    System.Greta.Service(); // Don't leave Greta alone for very long!
    testResultDisplayLoop();
    irremote.loop();
    router.loop();
    sensorpack.loop();
    motorcontroller.loop();
//    tracker.loop();
}
// ******************************************
void TestSuite::testResultDisplayLoop(void) {
	// update the test result display periodically

    uint32_t now = millis();
    if (now > timeoutUpdate) {
    	updateTestResultDisplay();
        timeoutUpdate = now + 1000;
    }
}
// ******************************************

// ****************************************************************************
// Test Suite Tests Section
// ****************************************************************************
void TestSuite::testMoveEast(void) {
    // make sure the azimuth motor is working properly by using its encoder

    // make sure we're stopped to begin with
	fullStop();
	// engage the motor for 2 seconds to make sure we're not at the limit
	moveWest();
	timeout(2000);
	fullStop();
	// make sure the encoder value doesn't change while the motor is stopped
	// updates the encoderCount0 and encoderCount values
	motorStillTest(AZIMUTH_MOTOR);
	assertEquals(encoderCount0,encoderCount,testMoveEastErrorAzCountStill);
	// get the current encoder value
	encoderCount0 = motorAz.countGet();
	// engage the motor for 2 seconds
	moveEast();
	timeout(2000);
	// get the current encoder value
	encoderCount = motorAz.countGet();
	// stop the motor
	fullStop();

	// compare the encoder values to make sure they have changed
	if (assertLessThan(encoderCount0,encoderCount,testMoveEastErrorAzCountMove)) {
		puts("moveEastResult:pass");
	}
	else {
		puts("moveEastResult:fail");
	}
}
// ******************************************
void TestSuite::testMoveWest(void) {
	// make sure the azimuth motor is working properly by using its encoder

    // make sure we're stopped to begin with
	fullStop();
	// engage the motor for 2 seconds to make sure we're not at the limit
	moveEast();
	timeout(2000);
	fullStop();
	// make sure the encoder value doesn't change while the motor is stopped
	// updates the encoderCount0 and encoderCount values
	motorStillTest(AZIMUTH_MOTOR);
	assertEquals(encoderCount0,encoderCount,testMoveWestErrorAzCountStill);
	// get the current encoder value
	encoderCount0 = motorAz.countGet();
	// engage the motor for 2 seconds
	moveWest();
	timeout(2000);
	// get the current encoder value
	encoderCount = motorAz.countGet();
	// stop the motor
	fullStop();

	// compare the encoder values to make sure they have changed
	if (assertGreaterThan(encoderCount0,encoderCount,testMoveWestErrorAzCountMove)) {
		puts("moveWestResult:pass");
	}
	else {
		puts("moveWestResult:fail");
	}
}
// ******************************************
void TestSuite::testMoveUp(void) {
	// make sure the elevation motor is working properly by using its encoder

    // make sure we're stopped to begin with
	fullStop();
	// engage the motor for 2 seconds to make sure we're not at the limit
	moveDown();
	timeout(2000);
	fullStop();
	// make sure the encoder value doesn't change while the motor is stopped
	// updates the encoderCount0 and encoderCount values
	motorStillTest(ELEVATION_MOTOR);
	assertEquals(encoderCount0,encoderCount,testMoveUpErrorElCountStill);
	// get the current encoder value
	encoderCount0 = motorEl.countGet();
	// engage the motor for 2 seconds
	moveUp();
	timeout(2000);
	// get the current encoder value
	encoderCount = motorEl.countGet();
	// stop the motor
	fullStop();

	// compare the encoder values to make sure they have changed
	if (assertLessThan(encoderCount0,encoderCount,testMoveUpErrorElCountMove)) {
		puts("moveUpResult:pass");
	}
	else {
		puts("moveUpResult:fail");
	}
}
// ******************************************
void TestSuite::testMoveDown(void) {
	// make sure the elevation motor is working properly by using its encoder

    // make sure we're stopped to begin with
	fullStop();
	// engage the motor for 2 seconds to make sure we're not at the limit
	moveUp();
	timeout(2000);
	fullStop();
	// make sure the encoder value doesn't change while the motor is stopped
	// updates the encoderCount0 and encoderCount values
	motorStillTest(ELEVATION_MOTOR);
	assertEquals(encoderCount0,encoderCount,testMoveDownErrorElCountStill);
	// get the current encoder value
	encoderCount0 = motorEl.countGet();
	// engage the motor for 2 seconds
	moveDown();
	timeout(2000);
	// get the current encoder value
	encoderCount = motorEl.countGet();
	// stop the motor
	fullStop();

	// compare the encoder values to make sure they have changed
	if (assertGreaterThan(encoderCount0,encoderCount,testMoveDownErrorElCountMove)) {
		puts("moveDownResult:pass");
	}
	else {
		puts("moveDownResult:fail");
	}
}
// ******************************************
void TestSuite::testSensorPackComm(void) {

    if (assertNotEquals(0,sensorpack.commCount,testSensorPackCommError)) {
    	puts("sensorPackCommResult:pass");
	}
	else {
		puts("sensorPackCommResult:fail");
	}

//	// test that the Sensor Pack comm link is active
//
//	// save the value of the last byte received
//	commCount = router.commCount;
//	// wait 2 seconds - enough time for new comms to occur
//	timeout(2000);
//	// compare the comm byte values to make sure that they have changed
//	assertNotEquals(commCount,router.commCount,testSensorPackCommError);
	timeout(1000);
}
// ******************************************
void TestSuite::testInterconnectComm(void) {

    if (assertNotEquals(0,interconnect.commCount,testInterconnectCommError)) {
    	puts("interconnectCommResult:pass");
	}
	else {
		puts("interconnectCommResult:fail");
	}

//	// test that the Interconnect Board comm link is active
//
//	// save the value of the last byte received
//	commCount = router.commCount;
//	// wait 2 seconds - enough time for new comms to occur
//	timeout(2000);
//	// compare the comm byte values to make sure that they have changed
//	assertNotEquals(commCount,router.commCount,testInterconnectCommError);
	timeout(1000);
}
// ******************************************
void TestSuite::testRailControllerComm(void) {

    if (assertNotEquals(0,railcomm.commCount,testRailCommError)) {
    	puts("railCommResult:pass");
	}
	else {
		puts("railCommResult:fail");
	}

//    commCount = railcomm.commCount;
//
//    router.frameStart(COMMPORT_RAIL);
//	  router.putc(COMMAND_COMMON_REPLY,COMMPORT_RAIL);
//    router.puts("t",COMMPORT_RAIL);
//    router.frameEnd(COMMPORT_RAIL);
//
//    timeout(1000);
//
//    assertNotEquals(commCount,sensorpack.commCount,testSensorPackCommError);
//	// test that the Rail Controller comm link is active
//
//	// save the value of the last byte received
//	commCount = router.commCount;
//	// wait 2 seconds - enough time for new comms to occur
//	timeout(2000);
//	// compare the comm byte values to make sure that they have changed
//	assertNotEquals(commCount,router.commCount,testRailControllerCommError);
//
	timeout(1000);
}
// ******************************************
void TestSuite::testModuleTemperature(void) {
	// test the temperature readings from each of the 4 modules to make sure they are within acceptable limits

	bool result;
	float mod1Temp,mod2Temp,mod3Temp,mod4Temp;

	result = true;
	// get the current temperature readings
	mod1Temp = module1.getTemperature();
	mod2Temp = module2.getTemperature();
	mod3Temp = module3.getTemperature();
	mod4Temp = module4.getTemperature();

	// test the current temperature readings against acceptable limits and add an error code to the test results if necessary
	if (mod1Temp < TEMP_OPEN_CIRCUIT) {
		testResult[testPtr][getTestResultPtrHead(testPtr)] = testOpenCrctTempMod1Error;
		result = false;
	}
	if (mod2Temp < TEMP_OPEN_CIRCUIT) {
		testResult[testPtr][getTestResultPtrHead(testPtr)] = testOpenCrctTempMod2Error;
		result = false;
	}
	if (mod3Temp < TEMP_OPEN_CIRCUIT) {
		testResult[testPtr][getTestResultPtrHead(testPtr)] = testOpenCrctTempMod3Error;
		result = false;
	}
	if (mod4Temp < TEMP_OPEN_CIRCUIT) {
		testResult[testPtr][getTestResultPtrHead(testPtr)] = testOpenCrctTempMod4Error;
		result = false;
	}
	if (mod1Temp > TEMP_SHORT_CIRCUIT) {
		testResult[testPtr][getTestResultPtrHead(testPtr)] = testShortCrctTempMod1Error;
		result = false;
	}
	if (mod2Temp > TEMP_SHORT_CIRCUIT) {
		testResult[testPtr][getTestResultPtrHead(testPtr)] = testShortCrctTempMod2Error;
		result = false;
	}
	if (mod3Temp > TEMP_SHORT_CIRCUIT) {
		testResult[testPtr][getTestResultPtrHead(testPtr)] = testShortCrctTempMod3Error;
		result = false;
	}
	if (mod4Temp > TEMP_SHORT_CIRCUIT) {
		testResult[testPtr][getTestResultPtrHead(testPtr)] = testShortCrctTempMod4Error;
		result = false;
	}

	if (result) {
		puts("moduleTempsResult:pass");
	}
	else {
		puts("moduleTempsResult:fail");
	}
}
// ******************************************
void TestSuite::testHomes(void) {
    uint32_t timeoutTimer;

	// make sure we're stopped to begin with
    puts("testHomes:Full Stop");
	fullStop();

// ****************West Limit Check
	// engage the motor for 2 seconds to make sure we're not at the limit
	moveEast();
    puts("testHomes:Move East");
	timeout(2000);
    puts("testHomes:Full Stop");
	fullStop();
	// get the current encoder value
	encoderCount = motorAz.countGet();
	// move West while monitoring the encoder counts
	moveWest();
    puts("testHomes:Move West");
//    timeoutTimer = millis() + MAX_TIME_TO_REACH_AZIMUTH_LIMIT;
    timeoutTimer = millis() + 20000;
	while (millis() < timeoutTimer) {
		if (!voltmeter.AzLimitCW()) {
		    puts("testHomes:AzLimitCW");
			break;
		}
		timeout(1000);
	    puts("testHomes:Tick");
	}
    puts("testHomes:Full Stop");
	fullStop();
	encoderCount0 = motorAz.countGet();
	assertLessThan(encoderCount0,encoderCount,testMoveWestErrorAzCountMove);
	if (millis() > timeoutTimer) {
		testResult[testPtr][getTestResultPtrHead(testPtr)] = testHomes1Error;
	    puts("testHomes:Result - Timeout");
	}
// ****************East Limit Check
	// engage the motor for 2 seconds to make sure we're not at the limit
	moveWest();
    puts("testHomes:Move West");
	timeout(2000);
    puts("testHomes:Full Stop");
	fullStop();
	// get the current encoder value
	encoderCount = motorAz.countGet();
	// move East while monitoring the encoder counts
	moveEast();
    puts("testHomes:Move East");
//    timeoutTimer = millis() + MAX_TIME_TO_REACH_AZIMUTH_LIMIT;
    timeoutTimer = millis() + 20000;
    while (millis() < timeoutTimer) {
		if (!voltmeter.AzLimitCCW()) {
		    puts("testHomes:AzLimitCCW");
			break;
		}
		timeout(1000);
	    puts("testHomes:Tick");
	}
    puts("testHomes:Full Stop");
	fullStop();
	encoderCount0 = motorAz.countGet();
	assertGreaterThan(encoderCount0,encoderCount,testMoveEastErrorAzCountMove);
	if (millis() > timeoutTimer) {
		testResult[testPtr][getTestResultPtrHead(testPtr)] = testHomes2Error;
	    puts("testHomes:Result - Timeout");
	}
// ****************Up/Down Limit Check
// This test is different than the West/East tests in that the limit switch is in the middle of the range so at each limit the switch should be false
// ****************
	// start by going to the Up limit
	encoderCount = motorEl.countGet();
	moveUp();
    puts("testHomes:Move Up");
//	timeoutTimer = millis() + MAX_TIME_TO_REACH_ELEVATION_LIMIT;
	timeoutTimer = millis() + 30000;
	while (millis() < timeoutTimer) {
		timeout(1000);
	    puts("testHomes:Tick");
		// keep checking if the count has stopped changing which would signal that we've reached the limit
		if (encoderCount == motorEl.countGet()) {
		    puts("testHomes:encoder stopped counting");
			break;
		}
		encoderCount = motorEl.countGet();
	}
    puts("testHomes:Full Stop");
	fullStop();
	// get the current encoder value
	encoderCount = motorEl.countGet();
	// start moving down and look for the limit switch to engage along the way
	moveDown();
    puts("testHomes:Move Down");
//    timeoutTimer = millis() + MAX_TIME_TO_REACH_AZIMUTH_LIMIT;
    timeoutTimer = millis() + 20000;
    while (millis() < timeoutTimer) {
		if (!voltmeter.ElLimitCW()) {
		    puts("testHomes:ElLimitCW");
			break;
		}
		timeout(1000);
	    puts("testHomes:Tick");
	}
	fullStop();
	encoderCount0 = motorEl.countGet();
	assertLessThan(encoderCount0,encoderCount,testMoveDownErrorElCountMove);
	if (millis() > timeoutTimer) {
		testResult[testPtr][getTestResultPtrHead(testPtr)] = testHomes3Error;
	    puts("testHomes:Result - Timeout");
	}

    puts("testComplete:testHomes");
}
// ******************************************
void TestSuite::motorStillTest(uint8_t motorType) {
	// test the encoder values during a full stop of the motor

	// clear the encoder values before testing
	encoderCount0 = 0;
	encoderCount = 0;
	// stop the motor
	fullStop();
	// save the encoder count
	if (motorType == AZIMUTH_MOTOR) {
		encoderCount0 = motorAz.countGet();
	}
	else if (motorType == ELEVATION_MOTOR) {
		encoderCount0 = motorEl.countGet();
	}
	// wait 2 seconds
	timeout(2000);
	// save the encoder count
	if (motorType == AZIMUTH_MOTOR) {
		encoderCount = motorAz.countGet();
	}
	else if (motorType == ELEVATION_MOTOR) {
		encoderCount = motorEl.countGet();
	}
}
// ******************************************
// ******************************************
void TestSuite::runTest(uint8_t testIndex) {
	// run the test given by the testIndex

	// clear all the test results before running the test
	for (uint8_t i = 0; i < MAX_NUMBER_OF_RESULTS_PER_TEST; i++) {
		testResult[testIndex][i] = '?';
		testResultPtrHead[testIndex] = 0;
		testResultPtrTail[testIndex] = 0;
	}

	switch(testIndex) {
	case 0:
		testMoveEast();
		break;
	case 1:
		testMoveWest();
		break;
	case 2:
		testMoveUp();
		break;
	case 3:
		testMoveDown();
		break;
	case 4:
		testSensorPackComm();
		break;
	case 5:
		testInterconnectComm();
		break;
	case 6:
		testRailControllerComm();
		break;
	case 7:
		testModuleTemperature();
		break;
	case 8:
		testHomes();
		break;
	}
}

// ****************************************************************************
// Test Suite Init Section
// ****************************************************************************
void TestSuite::init(void) {
	testModeActive = false;
	testRunning = false;
	testPaused = false;
	testPtr = NUMBER_OF_TESTS_IN_SUITE;
	finalTest = (NUMBER_OF_TESTS_IN_SUITE-1);
	initTestResultDisplay();
	clearTestResults();
}
// ******************************************
void TestSuite::initTestResultDisplay(void) {
	// reset all variables and pointers to their start values

	displayTestNumber = true;
	displayPtr = 0;
    timeoutUpdate = millis();
    // reset heads and tails
    for (uint8_t i = 0; i < NUMBER_OF_TESTS_IN_SUITE; i++) {
		testResultPtrHead[i] = 0;
		testResultPtrTail[i] = 0;
	}
}

// ****************************************************************************
// Test Suite Lib Section
// ****************************************************************************
void TestSuite::updateTestResultDisplay(void) {
	// update the test result display on the dual 7-seg displays
	// alternates between showing the test number and it's results
	// tests with multiple results will have results shown sequentially

	if (displayTestNumber) {
		display.lSet(TESTING_T);	// 't'
		display.rSet(displayPtr+1);	// current test number
	}
	else {
		if (testPaused) {
			display.lSet(TEST_PASSED);	// 'P'
			display.rSet(TEST_PASSED);	// 'P'
		}
		else {
			// if no test is currently running (!testRunning) OR
			// if we're going display info about an already completed test (displayPtr < testPtr)
			if (displayPtr == testPtr){
				display.lSet(TEST_RUNNING);
				display.rSet(TESTING_BLANK);
			}
			else {
				uint8_t tempTestResultPtrTail;
				// we put the tail into a temp varialbe so that it doesn't get incremented more than once during the following logic
				tempTestResultPtrTail = getTestResultPtrTail(displayPtr);
				// default value - no test results to display
				if (testResult[displayPtr][tempTestResultPtrTail] == '?') {
					display.lSet(TESTING_UNKN);
					display.rSet(TESTING_BLANK);
				}
				// passing value
				else if (testResult[displayPtr][tempTestResultPtrTail] == 'P') {
					display.lSet(TEST_PASSED);
					display.rSet(TESTING_BLANK);
				}
				// failing value
				else {
					display.lSet(TEST_FAILED);
					display.rSet((testResult[displayPtr][tempTestResultPtrTail] - 0x30));
				}
				// we have shown all test results for this particular test, move on to the next test and its results
				if (testResultPtrTail[displayPtr] == testResultPtrHead[displayPtr]) {
					testResultPtrTail[displayPtr] = 0;
					if (displayPtr < (NUMBER_OF_TESTS_IN_SUITE-1)) {
						displayPtr++;
					}
					else {
						displayPtr = 0;
					}
				}
			}
		}
	}
	displayTestNumber = !displayTestNumber;
}
// ******************************************
void TestSuite::clearTestResults(void) {
	// clear all test results back to the default value and reset pointers

	for (uint8_t i = 0; i < NUMBER_OF_TESTS_IN_SUITE; i++) {
		// head and tail start off at the same point
		testResultPtrHead[i] = 0;
		testResultPtrTail[i] = 0;
		// default test result value is '?', which means unknown
		for (uint8_t j = 0; j < MAX_NUMBER_OF_RESULTS_PER_TEST; j++) {
			testResult[i][j] = '?';
		}
	}
}
// ******************************************
uint8_t TestSuite::getTestResultPtrHead(uint8_t testIndex) {
	// return the current value of the test result pointer head
	// increment and limit check the test result pointer head

	if (testResultPtrHead[testIndex] < (MAX_NUMBER_OF_RESULTS_PER_TEST-1)) {
		testResultPtrHead[testIndex] += 1;
		return (testResultPtrHead[testIndex] - 1);
	}
	return testResultPtrHead[testIndex];
}
// ******************************************
uint8_t TestSuite::getTestResultPtrTail(uint8_t testIndex) {
	// return the current value of the test result pointer tail
	// increment and limit check the test result pointer tail

	if (testResultPtrTail[testIndex] < testResultPtrHead[testIndex]) {
		testResultPtrTail[testIndex] += 1;
		return (testResultPtrTail[testIndex] - 1);
	}
	else {
		testResultPtrTail[testIndex] = 0;
		return testResultPtrHead[testIndex];
	}
}
// ******************************************
void TestSuite::pauseTest(void) {
	// pause the test process after the current test ends
	testPaused = true;
}
// ******************************************
void TestSuite::resumeTest(void) {
	// resume the test process
	testPaused = false;
}
// ******************************************
void TestSuite::nextTest(void) {
	// skip over the next test in the process
	if (testPtr < (NUMBER_OF_TESTS_IN_SUITE-1)) {
		testPtr += 1;
	}
}
// ******************************************
void TestSuite::previousTest(void) {
	// back up one step in the test process
	if (testPtr) {
		testPtr -= 1;
	}
}
// ******************************************
void TestSuite::stopTests(void) {
	// back up one step in the test process
	testRunning = false;
	testPtr = NUMBER_OF_TESTS_IN_SUITE; // by setting it beyond the reach of the displayPtr we force the two pointers to be not equal, letting the test results display cycle fully
}
// ******************************************
bool TestSuite::getTestState(void) {
	// returns the state of testModeActive

	return testModeActive;
}
// ******************************************
void TestSuite::resetCommCounts(void) {
	// resets the comm counters in preparation for testing

	railcomm.commCount = 0;
	sensorpack.commCount = 0;
//	interconnect.commCount = 0;
}
// ******************************************
void TestSuite::setTestState(bool testState) {
	// updates testModeActive to testState

	testModeActive = testState;
	// if we are entering test mode, init test result display vars/pointers
	if (testModeActive) {
		initTestResultDisplay();
		resetCommCounts();
		puts("testModeResult:active");
	}
	// if we are leaving test mode, set testRunning to false
	else {
		testRunning = false;
		testPtr = NUMBER_OF_TESTS_IN_SUITE; // by setting it beyond the reach of the displayPtr we force the two pointers to be not equal, letting the test results display cycle fully
		puts("testModeResult:inactive");
	}
}
// ******************************************
void TestSuite::moveEast(void) {
	// move the tracker East

    tracker.syncChangeRequest(COMMAND_SYNC_PAUSED);
    motorcontroller.moveEast();
}
// ******************************************
void TestSuite::moveWest(void) {
	// move the tracker West

    tracker.syncChangeRequest(COMMAND_SYNC_PAUSED);
    motorcontroller.moveWest();
}
// ******************************************
void TestSuite::moveUp(void) {
	// move the tracker Up

    tracker.syncChangeRequest(COMMAND_SYNC_PAUSED);
    motorcontroller.moveUp();
}
// ******************************************
void TestSuite::moveDown(void) {
	// move the tracker Down

    tracker.syncChangeRequest(COMMAND_SYNC_PAUSED);
    motorcontroller.moveDown();
}
// ******************************************
void TestSuite::fullStop(void) {
	// stop all motors

    tracker.modeChangeRequest(COMMAND_MODE_JOGGING);
    tracker.syncChangeRequest(COMMAND_SYNC_STOPPED);
    tracker.fullstop();
	// give the motor time to stop fully
	timeout(1000);
}
// ******************************************
void TestSuite::timeout(uint16_t milliseconds) {
	// local timeout routine that maintains the maintenanceLoop during the timeout

	uint32_t delayTime;

	delayTime = millis() + milliseconds;
    while (millis() < delayTime) {
    	maintenanceLoop();
    	System.RTC.Sleep(MILLISECOND);
    }
}
// ******************************************
bool TestSuite::assertEquals(int32_t value1, int32_t value2, char errorMsg) {
	// compare the input values, set test result appropriately

	// input values are NOT equal, set test result to errorMsg
	if (value1 != value2) {
		testResult[testPtr][getTestResultPtrHead(testPtr)] = errorMsg;
		return false;
	}
	// input values are equal, set test result to passing value
	else {
		testResult[testPtr][getTestResultPtrHead(testPtr)] = 'P';
		return true;
	}
}
// ******************************************
bool TestSuite::assertNotEquals(int32_t value1, int32_t value2, char errorMsg) {
	// compare the input values, set test result appropriately

	// input values are equal, set test result to errorMsg
	if (value1 == value2) {
		testResult[testPtr][getTestResultPtrHead(testPtr)] = errorMsg;
		return false;
	}
	// input values are NOT equal, set test result to passing value
	else {
		testResult[testPtr][getTestResultPtrHead(testPtr)] = 'P';
		return true;
	}
}
// ******************************************
bool TestSuite::assertGreaterThan(int32_t value1, int32_t value2, char errorMsg) {
	// compare the input values, set test result appropriately

	// input values are equal, set test result to errorMsg
	if (value1 <= value2) {
		testResult[testPtr][getTestResultPtrHead(testPtr)] = errorMsg;
		return false;
	}
	// input values are NOT equal, set test result to passing value
	else {
		testResult[testPtr][getTestResultPtrHead(testPtr)] = 'P';
		return true;
	}
}
// ******************************************
bool TestSuite::assertLessThan(int32_t value1, int32_t value2, char errorMsg) {
	// compare the input values, set test result appropriately

	// input values are equal, set test result to errorMsg
	if (value1 >= value2) {
		testResult[testPtr][getTestResultPtrHead(testPtr)] = errorMsg;
		return false;
	}
	// input values are NOT equal, set test result to passing value
	else {
		testResult[testPtr][getTestResultPtrHead(testPtr)] = 'P';
		return true;
	}
}
// ******************************************
bool TestSuite::assertTrue(int32_t value, char errorMsg) {
	// test the input value, set test result appropriately

	// input value is NOT true, set test result to errorMsg
	if (!value) {
		testResult[testPtr][getTestResultPtrHead(testPtr)] = errorMsg;
		return false;
	}
	// input value is true, set test result to passing value
	else {
		testResult[testPtr][getTestResultPtrHead(testPtr)] = 'P';
		return true;
	}
}
// ******************************************
bool TestSuite::assertFalse(int32_t value, char errorMsg) {
	// test the input value, set test result appropriately

	// input value is true, set test result to errorMsg
	if (value) {
		testResult[testPtr][getTestResultPtrHead(testPtr)] = errorMsg;
		return false;
	}
	// input value is NOT true, set test result to passing value
	else {
		testResult[testPtr][getTestResultPtrHead(testPtr)] = 'P';
		return true;
	}
}
// ******************************************
void TestSuite::puts(char *str) {
	// send out a string via the rail controller comm port

	router.frameStart(COMMPORT_RAIL);
	router.putc(FRAME_PROTOCOL_ONEKEY,COMMPORT_RAIL);
	router.putc(COMMAND_COMMON_REPLY,COMMPORT_RAIL);
    router.puts(str,COMMPORT_RAIL);
    router.frameEnd(COMMPORT_RAIL);
}
// ****************************************************************************
