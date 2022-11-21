/*
This Arduino project aims to integrate Artrade Ltd.'s manufacturing system into MOMAMS:
https://github.com/aviharos/momams

The Arduino reads the PLC signals (using optocouplers),
identifies the events that should trigger commands, and sends a
JSON data packet to the Raspberry Pi on Serial (USB).

The Raspberry Pi will then issue the command with the given id.
This way, the Orion broker's objects can be updated.

The Raspberry Pi runs
https://github.com/aviharos/rpi_commands

The JSON sent contains each command_id as a key. Any values are passed on as arguments to the JSON object.
Example:
{"1": null, "5": 3, "6": 8000}
This means that commands 1, 5 and 6 will be issued by the Raspberry Pi.
  Command 1 without any arguments,
  command 5 with argument 3,
  command 6 with argument 8000.

Although rpi_commands can handle parameters,
Artrade Ltd.'s system does not need the use of parameters.
This way, each event will be sent without a parameter.

Dependencies:
  ArduinoJson@6.19.4

Credits:
https://www.arduino.cc/en/Tutorial/BuiltInExamples/StateChangeDetection
*/

#include <ArduinoJson.h>
#include <ArduinoJson.hpp>

#define MAX_MILLIS 4294967295

// constants
const unsigned long DEBOUNCE_DELAY = 50;
const unsigned long MAX_IDLE_TIME_INPUT_STORAGE = 30000;
const unsigned long MAX_IDLE_TIME_OUTPUT_STORAGE = 30000;

// pins
const byte availabilityPin = 2;
const byte goodPartPin = 3;
const byte rejectPartPin = 4;
const byte partTypePin1 = 5;
const byte partTypePin2 = 6;
const byte inputStorageResetPin = 7;
const byte outputStorageResetPin = 8;
const byte trayChangePin = 9;

// structs
struct inputSignal {
  byte pin;
  bool commandSent = 0;
  bool reading = LOW;
  bool lastState = LOW;
  unsigned long lastChangeTime = millis();
};

// initiating global variables
inputSignal availabilitySignal;
inputSignal goodPartSignal;
inputSignal rejectPartSignal;
inputSignal partTypeSignal1;
inputSignal partTypeSignal2;
inputSignal inputStorageResetSignal;
inputSignal outputStorageResetSignal;
inputSignal trayChangeSignal;

// functions
int getTimeSinceLastChange(unsigned long lastChangeTime) {
  unsigned long currentTime = millis();
  if (lastChangeTime > currentTime) {
    // overflow happened
    unsigned long firstPart = MAX_MILLIS - lastChangeTime;
    return firstPart + currentTime;
  } else {
    return currentTime - lastChangeTime;
  };
}

int getTimeSinceLastSignalChange(struct inputSignal *signal) {
  return getTimeSinceLastChange((*signal).lastChangeTime);
}

bool isStableLongerThan(inputSignal *signal, unsigned long timeDelta) {
  return getTimeSinceLastSignalChange(signal) > timeDelta;
}

bool isStateChanged(inputSignal *signal) {
  if ((*signal).lastState != (*signal).reading)
    return 1;
  else
    return 0;
}

void update(inputSignal *signal) {
  (*signal).lastState = (*signal).reading;
  (*signal).lastChangeTime = millis();
  (*signal).commandSent = 0;
}

void sendCommandWithoutArgument(char *command_id) {
  StaticJsonDocument<32> command;
  command[command_id] = nullptr;
  // send commands to Raspberry Pi on Serial
  serializeJson(command, Serial);
}

void sendMultipleCommandsWithoutArgument(char **commands, int size) {
  // this fixed size may cause a bug
  // if many commands are sent simultaneously
  StaticJsonDocument<64> command;
  int i;
  for (i = 0; i < size; i++) {
    //Serial.println(commands[i]);
    command[commands[i]] = nullptr;
  }
  // send commands to Raspberry Pi on Serial
  serializeJson(command, Serial);
}

void setup() {
  availabilitySignal.pin = availabilityPin;
  availabilitySignal.lastState = HIGH;
  availabilitySignal.reading = HIGH;
  pinMode(availabilitySignal.pin, INPUT_PULLUP);

  goodPartSignal.pin = goodPartPin;
  goodPartSignal.lastState = HIGH;
  goodPartSignal.reading = HIGH;
  pinMode(goodPartSignal.pin, INPUT_PULLUP);

  rejectPartSignal.pin = rejectPartPin;
  rejectPartSignal.lastState = HIGH;
  rejectPartSignal.reading = HIGH;
  pinMode(rejectPartSignal.pin, INPUT_PULLUP);
  
  partTypeSignal1.pin = partTypePin1;
  partTypeSignal1.lastState = HIGH;
  partTypeSignal1.reading = HIGH;
  pinMode(partTypeSignal1.pin, INPUT_PULLUP);

  partTypeSignal2.pin = partTypePin2;
  partTypeSignal2.lastState = HIGH;
  partTypeSignal2.reading = HIGH;
  pinMode(partTypeSignal2.pin, INPUT_PULLUP);

  inputStorageResetSignal.pin = inputStorageResetPin;
  inputStorageResetSignal.lastState = HIGH;
  inputStorageResetSignal.reading = HIGH;
  pinMode(inputStorageResetSignal.pin, INPUT_PULLUP);

  outputStorageResetSignal.pin = outputStorageResetPin;
  outputStorageResetSignal.lastState = HIGH;
  outputStorageResetSignal.reading = HIGH;
  pinMode(outputStorageResetSignal.pin, INPUT_PULLUP);

  trayChangeSignal.pin = trayChangePin;
  trayChangeSignal.lastState = HIGH;
  trayChangeSignal.reading = HIGH;
  pinMode(trayChangeSignal.pin, INPUT_PULLUP);

  // initialize serial communication:
  Serial.begin(9600);
}

void handleAvailability() {
  availabilitySignal.reading = digitalRead(availabilitySignal.pin);
  if (isStateChanged(&availabilitySignal)) {
    update(&availabilitySignal);
  } else {
    if (isStableLongerThan(&availabilitySignal, DEBOUNCE_DELAY) && !availabilitySignal.commandSent) {
      if (availabilitySignal.lastState == LOW) {
        // command 21: Injection moulder automatic
        sendCommandWithoutArgument("21");
        availabilitySignal.commandSent = 1;
      } else {  // HIGH
        // command 20: Injection moulder not automatic
        sendCommandWithoutArgument("20");
        availabilitySignal.commandSent = 1;
      }
    }
  }
}

void handleGoodPart() {
  goodPartSignal.reading = digitalRead(goodPartSignal.pin);
  if (isStateChanged(&goodPartSignal)) {
    update(&goodPartSignal);
  } else {
    if (isStableLongerThan(&goodPartSignal, DEBOUNCE_DELAY) && !goodPartSignal.commandSent) {
      if (goodPartSignal.lastState == LOW) {
        // command 30: good parts made
        sendCommandWithoutArgument("30");
        goodPartSignal.commandSent = 1;
      }
    }
  }
}

void handleRejectPart() {
  rejectPartSignal.reading = digitalRead(rejectPartSignal.pin);
  if (isStateChanged(&rejectPartSignal)) {
    update(&rejectPartSignal);
  } else {
    if (isStableLongerThan(&rejectPartSignal, DEBOUNCE_DELAY) && !rejectPartSignal.commandSent) {
      if (rejectPartSignal.lastState == LOW) {
        // command 40: reject parts made
        // command 41: rejects enter RejectStorage1
        char *commands[] = { "40", "41" };
        sendMultipleCommandsWithoutArgument(commands, 2);
        rejectPartSignal.commandSent = 1;
      }
    }
  }
}

void handlePartType() {
  partTypeSignal1.reading = digitalRead(partTypeSignal1.pin);
  partTypeSignal2.reading = digitalRead(partTypeSignal2.pin);
  if ((isStateChanged(&partTypeSignal1)) || (isStateChanged(&partTypeSignal2))) {
    update(&partTypeSignal1);
    update(&partTypeSignal2);
  } else {
    if ((isStableLongerThan(&partTypeSignal1, DEBOUNCE_DELAY) && !partTypeSignal1.commandSent) &&
        (isStableLongerThan(&partTypeSignal2, DEBOUNCE_DELAY) && !partTypeSignal2.commandSent)) {
      if ((partTypeSignal1.reading == LOW) && (partTypeSignal2.reading == HIGH)) {
        // command 500: reset Workstation python object's counters
        // command 510: new part type: Cube
        // command 511: reset Cube Job GoodPartCounter
        // command 512: reset Cube Job GoodPartCounter
        char *commands[] = { "500", "510", "511", "512" };
        sendMultipleCommandsWithoutArgument(commands, 4);
      }
      if ((partTypeSignal1.reading == HIGH) && (partTypeSignal2.reading == LOW)) {
        // command 500: reset Workstation python object's counters
        // command 520: new part type: Core
        // command 521: reset Core Job GoodPartCounter
        // command 522: reset Core Job GoodPartCounter
        char *commands[] = { "500", "520", "521", "522" };
        sendMultipleCommandsWithoutArgument(commands, 4);
      }
      if ((partTypeSignal1.reading == LOW) && (partTypeSignal2.reading == LOW)) {
        // command 500: reset Workstation python object's counters
        // command 530: new part type: Cover
        // command 531: reset Cover Job GoodPartCounter
        // command 532: reset Cover Job GoodPartCounter
        char *commands[] = { "500", "530", "531", "532" };
        sendMultipleCommandsWithoutArgument(commands, 4);
      }
      partTypeSignal1.commandSent = 1;
      partTypeSignal2.commandSent = 1;
    }
  }
}

void handleInputStorageReset() {
  inputStorageResetSignal.reading = digitalRead(inputStorageResetSignal.pin);
  if (isStateChanged(&inputStorageResetSignal)) {
    update(&inputStorageResetSignal);
  } else {
    if (isStableLongerThan(&inputStorageResetSignal, MAX_IDLE_TIME_INPUT_STORAGE) && !inputStorageResetSignal.commandSent) {
      if (inputStorageResetSignal.lastState == HIGH) {
        // we intentionally check for HIGH value here
        // this means that the PLC signal is 0 for a long time
        // so there is no tray in the input storage
        // command 70: input storage is out, set counter to 0
        sendCommandWithoutArgument("70");
        inputStorageResetSignal.commandSent = 1;
      }
    }
  }
}

void handleOutputStorageReset() {
  outputStorageResetSignal.reading = digitalRead(outputStorageResetSignal.pin);
  if (isStateChanged(&outputStorageResetSignal)) {
    update(&outputStorageResetSignal);
  } else {
    if (isStableLongerThan(&outputStorageResetSignal, MAX_IDLE_TIME_OUTPUT_STORAGE) && !outputStorageResetSignal.commandSent) {
      if (outputStorageResetSignal.lastState == LOW) {
        // command 80: set output storage counter to capacity
        sendCommandWithoutArgument("80");
        outputStorageResetSignal.commandSent = 1;
      }
    }
  }
}

void handleTrayChange() {
  trayChangeSignal.reading = digitalRead(trayChangeSignal.pin);
  if (isStateChanged(&trayChangeSignal)) {
    update(&trayChangeSignal);
  } else {
    if (isStableLongerThan(&trayChangeSignal, DEBOUNCE_DELAY) && !trayChangeSignal.commandSent) {
      if (trayChangeSignal.lastState == LOW) {
        // command 90: tray leaves input storage
        // command 91: tray enters output storage
        char *commands[] = { "90", "91" };
        sendMultipleCommandsWithoutArgument(commands, 2);
        trayChangeSignal.commandSent = 1;
      }
    }
  }
}

void loop() {
  handleAvailability();
  handleGoodPart();
  handleRejectPart();
  handlePartType();
  handleInputStorageReset();
  handleOutputStorageReset();
  handleTrayChange();
}
