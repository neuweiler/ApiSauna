/*
 * Controller.cpp
 *
 * The main controller which coordinates and controlls all parts of the
 * bee sauna..
 *
 Copyright (c) 2017 Michael Neuweiler

 Permission is hereby granted, free of charge, to any person obtaining
 a copy of this software and associated documentation files (the
 "Software"), to deal in the Software without restriction, including
 without limitation the rights to use, copy, modify, merge, publish,
 distribute, sublicense, and/or sell copies of the Software, and to
 permit persons to whom the Software is furnished to do so, subject to
 the following conditions:

 The above copyright notice and this permission notice shall be included
 in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

 */

#include "Controller.h"

// solange temp nicht erreicht wurde: FAN = 250, danach runter bis auf 20 wenn alle 4 temp-sensoren bei Zieltemp +/- 1 Grad
// wenn temp > 44 Grad: fan = 20, humidity-fan = 100%
// wenn hive temp < target nach pre-heat --> extended pre-heat mit hoher fanspeed, program running time erst ab erreichen der temp zählen
// jeder hive-temp sensor steuert eine platte.. --> sensoren durch löcher in abdeckplatte führen für genaue positionierung

Controller::Controller()
{
    actualTemperature = -999;
    targetTemperature = 0;
    plateTemperature = 0;
    statusLed = false;
    tickCounter = 0;
    pid = NULL;
}

Controller::~Controller()
{
    if (pid) {
        delete pid;
        pid = NULL;
    }
}

/**
 * Return the instance of the singleton (must only be used in ApiSauna.ino setup() and loop() !
 */
Controller *Controller::getSetupLoopInstance()
{
    static Controller instance;
    return &instance;
}

/**
 * Initialize the controller and referenced devices.
 */
void Controller::initialize()
{
    Logger::info(F("initializing controller"));

    if(!Configuration::getInstance()->load()) {
        Status::getInstance()->setSystemState(Status::error);
        return;
    }

    initOutput();
    powerDownDevices();

    ProgramHandler::getInstance()->initPrograms();
    ProgramHandler::getInstance()->attach(this);
    hid.initialize();
    beeper.initialize();
    humidifier.initialize();

    SimpleList<SensorAddress> addressList = detectTemperatureSensors();
    if(!assignPlateSensors(addressList) || !assignHiveSensors(addressList)) {
        Status::getInstance()->setSystemState(Status::error);
        return;
    }

    initPid();
    Status::getInstance()->setSystemState(Status::ready);
    serialConsole.printMenu();
}

/**
 * Initialize output ports
 */
void Controller::initOutput()
{
    Logger::info(F("initializing output"));

    ConfigurationIO *configIo = Configuration::getIO();
    pinMode(configIo->heaterRelay, OUTPUT);
    for (int i = 0; i < Configuration::getParams()->numberOfPlates; i++) {
        if (configIo->heater[i] != 0)
            pinMode(configIo->heater[i], OUTPUT);
        if (configIo->fan[i] != 0)
            pinMode(configIo->fan[i], OUTPUT);
    }
    pinMode(configIo->vaporizer, OUTPUT);
    pinMode(configIo->humidifierFan, OUTPUT);
}

void Controller::powerDownDevices()
{
    Logger::info(F("resetting output"));

    ConfigurationIO *configIo = Configuration::getIO();
    for (int i = 0; i < Configuration::getParams()->numberOfPlates; i++) {
        analogWrite(configIo->heater[i], 0);
        analogWrite(configIo->fan[i], Configuration::getParams()->minFanSpeed);
    }
    analogWrite(configIo->vaporizer, 0);
    analogWrite(configIo->humidifierFan, 0);
    delay(10); // wait a bit before trying to open the relay
    digitalWrite(configIo->heaterRelay, LOW);
}

void Controller::initPid()
{
    pid = new PID(&actualTemperature, &plateTemperature, &targetTemperature, 0, 0, 0, DIRECT);
    pid->SetOutputLimits(0, Configuration::getParams()->plateOverTemp);
    pid->SetSampleTime(CFG_LOOP_DELAY);
    pid->SetMode(AUTOMATIC);
}

/**
 *  Find all temperature sensor addresses (DS18B20).
 */
SimpleList<SensorAddress> Controller::detectTemperatureSensors()
{
    SimpleList<SensorAddress> addressList;

    Logger::info(F("detecting temperature sensors"));
    TemperatureSensor::resetSearch();

    while (true) {
        SensorAddress address = TemperatureSensor::search();
        if (address.value == 0)
            break;
        Logger::debug(F("  found sensor: %#08lx%08lx"), address.high, address.low);
        addressList.push_back(address);
    }
    TemperatureSensor::prepareData(); // kick the sensors to prepare data
    return addressList;
}

bool Controller::containsSensorAddress(SimpleList<SensorAddress> &addressList, SensorAddress address)
{
    for (SimpleList<SensorAddress>::iterator itrAddress = addressList.begin(); itrAddress != addressList.end(); ++itrAddress) {
        if (itrAddress->value == address.value)
            return true;
    }
    return false;
}

/**
 * Assign all found temperature sensors either to heating plates or use them to measure the
 * hive's temperature.
 */
bool Controller::assignPlateSensors(SimpleList<SensorAddress> &addressList)
{
    ConfigurationIO *configIO = Configuration::getIO();
    ConfigurationSensor *configSensor = Configuration::getSensor();

    plates.reserve(Configuration::getParams()->numberOfPlates);
    for (int i = 0; i < Configuration::getParams()->numberOfPlates; i++) {
        if (configSensor->addressPlate[i].value != 0 && containsSensorAddress(addressList, configSensor->addressPlate[i]) && configIO->fan[i] != 0 && configIO->heater[i] != 0) {
            Logger::info(F("attaching sensor %#08lx%08lx, heater pin %d, fan pin %d to plate #%d"), configSensor->addressPlate[i].high, configSensor->addressPlate[i].low, configIO->heater[i], configIO->fan[i], i + 1);
            Plate plate = Plate();
            plates.push_back(plate);
        }
    }

    // initialize plates after adding them to list because adding deletes/re-creates the plate objects and would mess with its allocated objects otherwise
    int i = 0;
    for (SimpleList<Plate>::iterator itr = plates.begin(); itr != plates.end(); ++itr) {
        itr->initialize(i++);
    }

    if (Configuration::getParams()->numberOfPlates != plates.size()) {
        Logger::error(F("unable to locate all configured plate sensors (%d of %d) !!"), plates.size(), Configuration::getParams()->numberOfPlates);
        return false;
    }
    return true;
}

/**
 * Verify that all configured hive temperature sensors are found in the search for sensors.
 */
bool Controller::assignHiveSensors(SimpleList<SensorAddress> &addressList)
{
    ConfigurationSensor *configSensor = Configuration::getSensor();

    for (int i = 0; configSensor->addressHive[i].value != 0 && i < CFG_MAX_NUMBER_PLATES; i++) {
        if (containsSensorAddress(addressList, configSensor->addressHive[i])) {
            Logger::info(F("attaching sensor %#08lx%08lx as hive sensor #%d"), configSensor->addressHive[i].high, configSensor->addressHive[i].low, i + 1);
            TemperatureSensor sensor = TemperatureSensor(i, false);
            hiveTempSensors.push_back(sensor);
        } else {
            Logger::error(F("unable to locate all configured hive sensors (%#l08x%08lx missing) !!"), configSensor->addressHive[i].high, configSensor->addressHive[i].low);
            return false;
        }
    }
    return true;
}

/**
 * Retrieve temperature data from all sensors and return highest value (in 0.1 deg C)
 */
int16_t Controller::retrieveHiveTemperatures()
{
    int16_t max = -999;

    int i = 0;
    for (SimpleList<TemperatureSensor>::iterator itr = hiveTempSensors.begin(); itr != hiveTempSensors.end(); ++itr) {
        itr->retrieveData();
        if (i < CFG_MAX_NUMBER_PLATES)
            Status::getInstance()->temperatureHive[i++] = itr->getTemperatureCelsius();
        max = max(max, itr->getTemperatureCelsius());
    }
    return max;
}

/**
 * Calculate the maximum plate temperature based on the hive temperature and the current state. (in 0.1 deg C)
 */
int16_t Controller::calculatePlateTargetTemperature()
{
    Program *runningProgram = ProgramHandler::getInstance()->getRunningProgram();
    if (actualTemperature == -999 || runningProgram == NULL) {
        return 0;
    }

    if (Status::getInstance()->getSystemState() == Status::preHeat) {
        targetTemperature = runningProgram->temperaturePreHeat;
    } else {
        targetTemperature = runningProgram->temperatureHive;
    }

    pid->Compute();
    int16_t temp = constrain(plateTemperature, (double )0, runningProgram->temperaturePlate);
    Status::getInstance()->temperatureTargetHive = targetTemperature;
    Status::getInstance()->temperatureTargetPlate = temp;
    return temp;
}

/**
 * Check if a state has reached its execution time and switch to next state.
 */
void Controller::updateProgramState()
{
    if (actualTemperature > Configuration::getParams()->hiveOverTemp) {
        Logger::error(F("ALERT - OVER-TEMPERATURE IN HIVE ! Trying to recover, please open the cover to help cool down the hive!"));
        Status::getInstance()->setSystemState(Status::overtemp);
    }
    if (Status::getInstance()->getSystemState() == Status::overtemp && actualTemperature < Configuration::getParams()->hiveOverTempRecover) {
        Logger::info(F("recovered from over-temperature, shutting down."));
        Status::getInstance()->setSystemState(Status::shutdown);
    }

    // switch to running if pre-heating cycle is finished
    Program *runningProgram = ProgramHandler::getInstance()->getRunningProgram();
    uint32_t timeRemaining = ProgramHandler::getInstance()->getTimeRemaining();
    if (Status::getInstance()->getSystemState() == Status::preHeat && runningProgram && (timeRemaining == 0 || retrieveHiveTemperatures() >= runningProgram->temperaturePreHeat)) {
        Status::getInstance()->setSystemState(Status::running);
        pid->SetOutputLimits(runningProgram->temperatureHive, Configuration::getParams()->hiveOverTemp);
        for (SimpleList<Plate>::iterator itr = plates.begin(); itr != plates.end(); ++itr) {
            itr->setFanSpeed(runningProgram->fanSpeed);
        }
    }

    // switch to shutdown when program is finished
    if (Status::getInstance()->getSystemState() == Status::running && timeRemaining < 2) {
        Status::getInstance()->setSystemState(Status::shutdown);
    }
}

void Controller::handleEvent(ProgramEvent event, Program *program)
{
    Logger::debug("controller: incoming event %d, program: %s", event, (program ? program->name : "n/a"));
    switch (event) {
    case startProgram:
        // init all plate parameters
        for (SimpleList<Plate>::iterator itr = plates.begin(); itr != plates.end(); ++itr) {
            itr->setFanSpeed(program->fanSpeedPreHeat);
            itr->setMaximumPower(Configuration::getParams()->maxHeaterPower);
            itr->setPIDTuning(program->plateKp, program->plateKi, program->plateKd);
        }
        humidifier.setFanSpeed(program->fanSpeedHumidifier);
        humidifier.setMinHumidity(program->humidityMinimum);
        humidifier.setMaxHumidity(program->humidityMaximum);

        pid->SetOutputLimits(program->temperaturePreHeat, program->temperaturePlate);
        pid->SetTunings(program->hiveKp, program->hiveKi, program->hiveKd);

        digitalWrite(Configuration::getIO()->heaterRelay, HIGH);
        delay(500); // give it some time to close
        Status::getInstance()->setSystemState(Status::preHeat);
        break;

    case stopProgram:
        break;
    }

}

/**
 * This is the main control loop.
 */
void Controller::process()
{
    beeper.process();
    hid.process();
    serialConsole.process();

    if (tickCounter++ > 9) {
        tickCounter = 0;
        humidifier.process();
        actualTemperature = retrieveHiveTemperatures();
        updateProgramState();

        switch (Status::getInstance()->getSystemState()) {
        case Status::init:
            break;
        case Status::ready:
            break;
        case Status::preHeat:
        case Status::running: {
            int16_t plateTemp = calculatePlateTargetTemperature();
            for (SimpleList<Plate>::iterator itr = plates.begin(); itr != plates.end(); ++itr) {
                itr->setTargetTemperature(plateTemp);
                itr->process();
            }
            break;
        }
        case Status::overtemp: // shut-down heaters, plate fan to minimum, full blow humidifier fan (fresh air) !!
            for (SimpleList<Plate>::iterator itr = plates.begin(); itr != plates.end(); ++itr) {
                itr->setMaximumPower(0);
                itr->setFanSpeed(Configuration::getParams()->minFanSpeed);
                itr->process();
            }
            humidifier.setMinHumidity(99);
            humidifier.setMaxHumidity(100);
            humidifier.setFanSpeed(255);
            humidifier.process();
            delay(10); // wait a bit before trying to open the relay
            digitalWrite(Configuration::getIO()->heaterRelay, LOW);
            break;
        case Status::shutdown:
            for (SimpleList<Plate>::iterator itr = plates.begin(); itr != plates.end(); ++itr) {
                itr->setMaximumPower(0);
                itr->setFanSpeed(Configuration::getParams()->minFanSpeed);
                itr->process();
            }
            humidifier.setMinHumidity(0);
            humidifier.setMaxHumidity(1);
            humidifier.setFanSpeed(0);
            humidifier.process();
            delay(10);
            digitalWrite(Configuration::getIO()->heaterRelay, LOW);
            break;
        case Status::error:
            powerDownDevices();
            break;
        }
        TemperatureSensor::prepareData();

        statusLed = !statusLed;
        digitalWrite(Configuration::getIO()->heartbeat, statusLed); // some kind of heart-beat
    }
}

void Controller::setPIDTuningHive(double kp, double ki, double kd)
{
    pid->SetTunings(kp, ki, kd);
}

void Controller::setPIDTuningPlate(double kp, double ki, double kd)
{
    for (SimpleList<Plate>::iterator itr = plates.begin(); itr != plates.end(); ++itr) {
        itr->setPIDTuning(kp, ki, kd);
    }
}

void Controller::setFanSpeed(uint8_t speed)
{
    for (SimpleList<Plate>::iterator itr = plates.begin(); itr != plates.end(); ++itr) {
        itr->setFanSpeed(speed);
    }
}

void Controller::setFanSpeedHumidifier(uint8_t speed)
{
    humidifier.setFanSpeed(speed);
}

void Controller::setHumidifierLimits(uint8_t min, uint8_t max)
{
    humidifier.setMinHumidity(min);
    humidifier.setMaxHumidity(max);
}

int16_t Controller::getHiveTargetTemperature()
{
    return targetTemperature;
}

