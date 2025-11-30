/* Copyright 2017, 2018 David Conran
*  Copyright 2025, Lukas Zimmermann
*
* An IR LED circuit *MUST* be connected to the ESP8266 on a pin
* as specified by kIrLed below.
*
* TL;DR: The IR LED needs to be driven by a transistor for a good result.
*
* Suggested circuit:
*     https://github.com/crankyoldgit/IRremoteESP8266/wiki#ir-sending
*
* Common mistakes & tips:
*   * Don't just connect the IR LED directly to the pin, it won't
*     have enough current to drive the IR LED effectively.
*   * Make sure you have the IR LED polarity correct.
*     See: https://learn.sparkfun.com/tutorials/polarity/diode-and-led-polarity
*   * Typical digital camera/phones can be used to see if the IR LED is flashed.
*     Replace the IR LED with a normal LED if you don't have a digital camera
*     when debugging.
*   * Avoid using the following pins unless you really know what you are doing:
*     * Pin 0/D3: Can interfere with the boot/program mode & support circuits.
*     * Pin 1/TX/TXD0: Any serial transmissions from the ESP8266 will interfere.
*     * Pin 3/RX/RXD0: Any serial transmissions to the ESP8266 will interfere.
*   * ESP-01 modules are tricky. We suggest you use a module with more GPIOs
*     for your first time. e.g. ESP-12 etc.
*
* This specific program runs on a cheap chineese ESP8266 based IR controller
* device (https://templates.blakadder.com/auvisio_S06.html)
*/
#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <IRac.h>
#include <ir_Mitsubishi.h>

//const uint16_t kIrLed = 4;  // ESP8266 GPIO pin to use. Recommended: 4 (D2).
const uint16_t kIrLed = 14;   // auvisio S06 IR Controller has IR led on GPIO 14
IRMitsubishiACDbl ac(kIrLed); // Set the GPIO used for sending messages.
const uint16_t button1 = 13;  // auvisio S06 IR Controller has a button on GPIO 13
const uint16_t blueled = 4;   // auvisio S06 IR Controller has a blue led on GPIO 4

void printState() {
  // Display the settings.
  Serial.println("Mitsubishi A/C remote is in the following state:");
  Serial.printf("  %s\n", ac.toString().c_str());
  // Display the encoded IR sequence.
  unsigned char* ir_code = ac.getRaw();
  Serial.print("IR Code: 0x");
  for (uint8_t i = 0; i < kMitsubishiACStateLength; i++)
    Serial.printf("%02X", ir_code[i]);
  Serial.println();
}

void setup() {
  ac.begin();
  Serial.begin(115200);
  delay(200);

  pinMode(button1, INPUT_PULLUP);
  digitalWrite(blueled, LOW);
  pinMode(blueled, OUTPUT);

  Serial.println();
  Serial.print("IRremoteESP8266 lib version ");
  Serial.println(_IRREMOTEESP8266_VERSION_STR);

  // Set up what we want to send. See ir_Mitsubishi.cpp for all the options.
  Serial.println("Default state of the remote.");
  printState();
  Serial.println("Setting desired state for A/C.");
  ac.on();
  ac.setFan(kMitsubishiAcFanAuto);
  ac.setMode(kMitsubishiAcHeat);
  ac.setTemp(25);
  ac.setVane(kMitsubishiAcVaneLow);
  ac.setWideVane(kMitsubishiAcWideVaneRightMax);
}

void loop() {
  static bool buttonEdge = false;
  static uint16_t buttonState;
  static uint16_t lastButtonState = HIGH;

  buttonState = digitalRead(button1);
  if (buttonState == LOW) {
    if (lastButtonState == HIGH) {
      buttonEdge = true;
    }
  }
  lastButtonState = buttonState;

  if (buttonEdge == true) {
    buttonEdge = false;
    // toggle between two different settings on button press
    if (ac.getVane() != kMitsubishiAcVaneLow)
      ac.setVane(kMitsubishiAcVaneLow);
    else
      ac.setVane(kMitsubishiAcVaneLowest);
    // Now send the IR signal.
#if SEND_MITSUBISHI_AC_DBL
    Serial.println("Sending IR command to A/C ...");
    digitalWrite(blueled, HIGH);
    ac.send();
    digitalWrite(blueled, LOW);
#endif  // SEND_MITSUBISHI_AC_DBL
    printState();
  }
}
