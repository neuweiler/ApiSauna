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

Controller::Controller()
{
    actualTemperature = -999;
    targetTemperature = 0;
    plateTemperature = 0;
    statusLed = false;
    startTime = 0;
    tickCounter = 0;
    runningProgram = NULL;
    pid = NULL;

    //TODO move config to EEPROM
    plateConfigs.push_back(PlateConfig(1, CFG_TEMP_SENSOR_HEATER_1, CFG_IO_HEATER_1, CFG_IO_FAN_1));
    plateConfigs.push_back(PlateConfig(2, CFG_TEMP_SENSOR_HEATER_2, CFG_IO_HEATER_2, CFG_IO_FAN_2));
    plateConfigs.push_back(PlateConfig(3, CFG_TEMP_SENSOR_HEATER_3, CFG_IO_HEATER_3, CFG_IO_FAN_3));
    plateConfigs.push_back(PlateConfig(4, CFG_TEMP_SENSOR_HEATER_4, CFG_IO_HEATER_4, CFG_IO_FAN_4));

// solange temp nicht erreicht wurde: FAN = 250, danach runter bis auf 20 wenn alle 4 temp-sensoren bei Zieltemp +/- 1 Grad
// wenn temp > 44 Grad: fan = 20, humidity-fan = 100%
// anstatt pwm einfach on/off --> schonender für netzteil ?
// wenn hive temp < target nach pre-heat --> extended pre-heat mit hoher fanspeed, program running time erst ab erreichen der temp zählen
// jeder hive-temp sensor steuert eine platte.. --> sensoren durch löcher in abdeckplatte führen für genaue positionierung

    ControllerProgram programVaroaSummer;
    snprintf(programVaroaSummer.name, 16, "Varoa Killer");
    programVaroaSummer.temperaturePreHeat = 390; // 39 deg C
    programVaroaSummer.fanSpeedPreHeat = 60;
    programVaroaSummer.durationPreHeat = 30; // 30min
    programVaroaSummer.temperatureHive = 430; // 430 deg C
    programVaroaSummer.hiveKp = 8.0;
    programVaroaSummer.hiveKi = 0.2;
    programVaroaSummer.hiveKd = 5.0;
    programVaroaSummer.temperaturePlate = 800; // 80 deg C
    programVaroaSummer.plateKp = 3.0;
    programVaroaSummer.plateKi = 0.01;
    programVaroaSummer.plateKd = 50.0;
    programVaroaSummer.fanSpeed = 170; // minimum is 10
    programVaroaSummer.humidityMinimum = 30;
    programVaroaSummer.humidityMaximum = 35;
    programVaroaSummer.fanSpeedHumidifier = 230; // only works from 230 to 255
    programVaroaSummer.duration = 240; // 4 hours
    programs.push_back(programVaroaSummer);

    ControllerProgram programVaroaWinter;
    snprintf(programVaroaWinter.name, 16, "Winter Treat");
    programVaroaWinter.temperaturePreHeat = 390; // 39 deg C
    programVaroaWinter.fanSpeedPreHeat = 100;
    programVaroaWinter.durationPreHeat = 30; // 30min
    programVaroaWinter.temperatureHive = 430; // 43.0 deg C
    programVaroaWinter.hiveKp = 8.0;
    programVaroaWinter.hiveKi = 0.2;
    programVaroaWinter.hiveKd = 5.0;
    programVaroaWinter.temperaturePlate = 800; // 75 deg C
    programVaroaWinter.plateKp =4.0;
    programVaroaWinter.plateKi = 0.09;
    programVaroaWinter.plateKd = 50.0;
    programVaroaWinter.fanSpeed = 255; // minimum is 10
    programVaroaWinter.humidityMinimum = 30;
    programVaroaWinter.humidityMaximum = 35;
    programVaroaWinter.fanSpeedHumidifier = 230; // only works from 230 to 255
    programVaroaWinter.duration = 180; // 3 hours
    programs.push_back(programVaroaWinter);

    ControllerProgram programCleaning;
    snprintf(programCleaning.name, 16, "Cleaning");
    programCleaning.temperaturePreHeat = 380; // 38 deg C
    programCleaning.fanSpeedPreHeat = 100;
    programCleaning.durationPreHeat = 30; // 30min
    programCleaning.temperatureHive = 425; // 42.5 deg C
    programCleaning.hiveKp = 8.0;
    programCleaning.hiveKi = 0.2;
    programCleaning.hiveKd = 5.0;
    programCleaning.temperaturePlate = 750; // 75 deg C
    programCleaning.plateKp =4.0;
    programCleaning.plateKi = 0.09;
    programCleaning.plateKd = 50.0;
    programCleaning.fanSpeed = 255; // minimum is 10
    programCleaning.humidityMinimum = 30;
    programCleaning.humidityMaximum = 35;
    programCleaning.fanSpeedHumidifier = 230; // only works from 230 to 255
    programCleaning.duration = 180; // 3 hours
    programs.push_back(programCleaning);


//    subMenu[0][1] = "2 Wellness";
//    subMenu[0][2] = "3 Queen Health";
//    subMenu[0][3] = "4 Melt Honey";
}

Controller::~Controller()
{
    if (pid) {
        delete pid;
        pid = NULL;
    }
}

SimpleList<TemperatureSensor>* Controller::getHiveTempSensors()
{
    return &hiveTempSensors;
}

SimpleList<Plate>* Controller::getPlates()
{
    return &plates;
}

Humidifier* Controller::getHumidifier()
{
    return &humidifier;
}

ControllerProgram* Controller::getRunningProgram()
{
    return runningProgram;
}

SimpleList<ControllerProgram>* Controller::getPrograms()
{
    return &programs;
}

uint32_t Controller::getTimeRunning()
{
    if (runningProgram != NULL && (status.getSystemState() == Status::preHeat || status.getSystemState() == Status::running)) {
        return (millis() - startTime) / 1000;
    }
    return 0;
}

/**
 * Returns the time the current program stage is running in seconds.
 */
uint32_t Controller::getTimeRemaining()
{
    if (runningProgram != NULL) {
        uint32_t timeRunning = getTimeRunning();
        uint32_t duration = (status.getSystemState() == Status::preHeat ? runningProgram->durationPreHeat : runningProgram->duration) * 60;
        // perform a safety check to prevent an unsigned under-flow if timeRunning is bigger than the duration
        if (timeRunning < duration) {
            return duration - timeRunning;
        }
    }
    return 0;
}

int16_t Controller::getHiveTemperature()
{
    return actualTemperature; // updated in the loop()
}

int16_t Controller::getHiveTargetTemperature()
{
    return targetTemperature;
}

void Controller::startProgram(uint8_t programNumber)
{
    int i = 0;
    SimpleList<ControllerProgram>* programs = controller.getPrograms();
    for (SimpleList<ControllerProgram>::iterator itr = programs->begin(); itr != programs->end(); ++itr) {
        if (i == programNumber) {
            Logger::info("starting program #%d", i);
            if (status.setSystemState(Status::preHeat) == Status::preHeat) {
                runningProgram = itr;
                startTime = millis();

                // init all plate parameters
                for (SimpleList<Plate>::iterator itr = plates.begin(); itr != plates.end(); ++itr) {
                    itr->setFanSpeed(runningProgram->fanSpeedPreHeat);
                    itr->setMaximumPower(CFG_MAX_HEATER_POWER);
                    itr->setPIDTuning(runningProgram->plateKp, runningProgram->plateKi, runningProgram->plateKd);
                }
                humidifier.setFanSpeed(runningProgram->fanSpeedHumidifier);
                humidifier.setMinHumidity(runningProgram->humidityMinimum);
                humidifier.setMaxHumidity(runningProgram->humidityMaximum);

                pid->SetOutputLimits(runningProgram->temperaturePreHeat, runningProgram->temperaturePlate);
                pid->SetTunings(runningProgram->hiveKp, runningProgram->hiveKi, runningProgram->hiveKd);

                digitalWrite(CFG_IO_HEATER_MAIN_SWITCH, HIGH);
                delay(500); // give it some time to close
                return;
            } else {
                Logger::warn("program not started");
                return;
            }
        }
        i++;
    }
    Logger::warn("could not find program #%d", programNumber);
}

void Controller::stopProgram()
{
    runningProgram = NULL;
    status.setSystemState(Status::shutdown);
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

/**
 *  Find all temperature sensor addresses (DS18B20).
 */
SimpleList<SensorAddress> Controller::findTemperatureSensors()
{
    SimpleList<SensorAddress> addressList;

    TemperatureSensor::resetSearch();
    Logger::info("searching temperature sensors...");

    while (true) {
        SensorAddress address = TemperatureSensor::search();
        if (address.value == 0)
            break;
        Logger::debug("found sensor: addr=%#lx%lx", address.high, address.low);
        addressList.push_back(address);
    }
    TemperatureSensor::prepareData(); // kick the sensors to prepare data
    return addressList;
}

/**
 * Assign all found temperature sensors either to heating plates or use them to measure the
 * hive's temperature.
 */
void Controller::assignTemperatureSensors(SimpleList<SensorAddress> *addressList)
{
    for (SimpleList<SensorAddress>::iterator itrAddress = addressList->begin(); itrAddress != addressList->end(); ++itrAddress) {
        bool foundInConfig = false;
        for (SimpleList<PlateConfig>::iterator itrConfig = plateConfigs.begin(); itrConfig != plateConfigs.end(); ++itrConfig) {
            if (itrAddress->value == itrConfig->sensorAddressHeater.value) {
                foundInConfig = true;
                Logger::info("attaching sensor %#lx%lx, heater pin %d, fan pin %d to plate #%d", itrConfig->sensorAddressHeater.high,
                        itrConfig->sensorAddressHeater.low, itrConfig->heaterPin, itrConfig->fanPin, itrConfig->id);
                Plate *plate = new Plate(itrConfig);
                plates.push_back(*plate);
            }
        }
        if (!foundInConfig) {
            Logger::info("using temp sensor %#lx%lx to measure hive temperature", itrAddress->high, itrAddress->low);
            hiveTempSensors.push_back(TemperatureSensor(*itrAddress));
        }
    }
}

void Controller::loadDefaults()
{
}

void Controller::loadConfig()
{
//    EEPROM.get(CFG_EEPROM_CONFIG_ADDRESS, config);
}

void Controller::saveConfig()
{
//    EEPROM.put(CFG_EEPROM_CONFIG_ADDRESS, config);
}

/**
 * Initialize the controller and referenced devices.
 */
void Controller::init()
{
    loadConfig();

    SimpleList<SensorAddress> addressList = findTemperatureSensors();
    assignTemperatureSensors(&addressList);
    if (plateConfigs.size() != plates.size()) {
        Logger::error("unable to locate all temperature sensors of all configured plates (%d of %d) !!", plates.size(), plateConfigs.size());
        status.setSystemState(Status::error);
    }
    humidifier.setVaporizerPin(CFG_IO_VAPORIZER);
    humidifier.setFanPin(CFG_IO_FAN_HUMIDIFIER);
    humidifier.setSensorPin(CFG_IO_HUMIDITY_SENSOR);

    beeper.setControlPin(CFG_IO_BEEPER);

    pid = new PID(&actualTemperature, &plateTemperature, &targetTemperature, 0, 0, 0, DIRECT);
    pid->SetOutputLimits(0, CFG_PLATE_OVER_TEMPERATURE);
    pid->SetSampleTime(CFG_LOOP_DELAY);
    pid->SetMode(AUTOMATIC);
}

// turn
void Controller::powerDownDevices()
{
    analogWrite(CFG_IO_HEATER_1, 0);
    analogWrite(CFG_IO_HEATER_2, 0);
    analogWrite(CFG_IO_HEATER_3, 0);
    analogWrite(CFG_IO_HEATER_4, 0);
    analogWrite(CFG_IO_FAN_1, CFG_MIN_FAN_SPEED);
    analogWrite(CFG_IO_FAN_2, CFG_MIN_FAN_SPEED);
    analogWrite(CFG_IO_FAN_3, CFG_MIN_FAN_SPEED);
    analogWrite(CFG_IO_FAN_4, CFG_MIN_FAN_SPEED);
    analogWrite(CFG_IO_VAPORIZER, 0);
    analogWrite(CFG_IO_FAN_HUMIDIFIER, 0);
    delay(10); // wait a bit before trying to open the relay
    digitalWrite(CFG_IO_HEATER_MAIN_SWITCH, LOW);
}

/**
 * Retrieve temperature data from all sensors and return highest value (in 0.1 deg C)
 */
int16_t Controller::retrieveHiveTemperatures()
{
    int16_t max = -999;

    for (SimpleList<TemperatureSensor>::iterator itr = hiveTempSensors.begin(); itr != hiveTempSensors.end(); ++itr) {
        itr->retrieveData();
        max = max(max, itr->getTemperatureCelsius());
    }
    return max;
}

/**
 * Calculate the maximum plate temperature based on the hive temperature and the current state. (in 0.1 deg C)
 */
int16_t Controller::calculateMaxPlateTemperature()
{
    if (actualTemperature == -999 || runningProgram == NULL) {
        return 0;
    }

    if (status.getSystemState() == Status::preHeat) {
        targetTemperature = runningProgram->temperaturePreHeat;
    } else {
        targetTemperature = runningProgram->temperatureHive;
    }

    pid->Compute();
    return constrain(plateTemperature, (double )0, runningProgram->temperaturePlate);
}

/**
 * Check if a state has reached its execution time and switch to next state.
 */
void Controller::updateProgramState()
{
    if (actualTemperature > CFG_HIVE_OVER_TEMPERATURE) {
        Logger::error("!!!!!!!!!!!!!!!!! ALERT !!!!!!!!!!!!!!!!!!!! OVER-TEMPERATURE IN HIVE DETECTED !!!!!!!!!!!!!!! Trying to recover...");
        status.setSystemState(Status::overtemp);
    }
    if (status.getSystemState() == Status::overtemp && actualTemperature < CFG_HIVE_TEMPERATURE_RECOVER) {
        Logger::info("Recovered from over-temperature in hive, shutting-down as precaution");
        status.setSystemState(Status::shutdown);
    }

    // switch to running if pre-heating cycle is finished
    if (status.getSystemState() == Status::preHeat && getTimeRemaining() == 0 && runningProgram) {
        status.setSystemState(Status::running);
        pid->SetOutputLimits(runningProgram->temperatureHive, CFG_PLATE_OVER_TEMPERATURE);
        startTime = millis();
        for (SimpleList<Plate>::iterator itr = plates.begin(); itr != plates.end(); ++itr) {
            itr->setFanSpeed(runningProgram->fanSpeed);
        }
    }

    // switch to shutdown when program is finished
    if (status.getSystemState() == Status::running && getTimeRemaining() == 2) {
        status.setSystemState(Status::shutdown);
    }
}

/**
 * This is the main control loop.
 */
void Controller::loop()
{
    if (tickCounter++ > 9) {
        tickCounter = 0;
        actualTemperature = retrieveHiveTemperatures();
        updateProgramState();

        switch (status.getSystemState()) {
        case Status::init:
            Logger::error("controller loop must not be called while initialising!");
            break;
        case Status::ready:
            break;
        case Status::preHeat:
        case Status::running: {
            int16_t plateTemp = calculateMaxPlateTemperature();
            for (SimpleList<Plate>::iterator itr = plates.begin(); itr != plates.end(); ++itr) {
                itr->setTargetTemperature(plateTemp);
                itr->loop();
            }
            humidifier.loop();
            break;
        }
        case Status::overtemp: // shut-down heaters, plate fan to minimum, full blow humidifier fan (fresh air) !!
            for (SimpleList<Plate>::iterator itr = plates.begin(); itr != plates.end(); ++itr) {
                itr->setMaximumPower(0);
                itr->setFanSpeed(CFG_MIN_FAN_SPEED);
                itr->loop();
            }
            humidifier.setMinHumidity(99);
            humidifier.setMaxHumidity(100);
            humidifier.setFanSpeed(255);
            humidifier.loop();
            delay(10); // wait a bit before trying to open the relay
            digitalWrite(CFG_IO_HEATER_MAIN_SWITCH, LOW);
            break;
        case Status::shutdown:
            for (SimpleList<Plate>::iterator itr = plates.begin(); itr != plates.end(); ++itr) {
                itr->setMaximumPower(0);
                itr->setFanSpeed(CFG_MIN_FAN_SPEED);
                itr->loop();
            }
            humidifier.setMinHumidity(0);
            humidifier.setMaxHumidity(1);
            humidifier.setFanSpeed(0);
            humidifier.loop();
            delay(10);
            digitalWrite(CFG_IO_HEATER_MAIN_SWITCH, LOW);
            break;
        case Status::error:
            powerDownDevices();
            break;
        }
        TemperatureSensor::prepareData();

        statusLed = !statusLed;
        digitalWrite(CFG_IO_BLINK_LED, statusLed); // some kind of heart-beat
    }

    beeper.loop();
}
