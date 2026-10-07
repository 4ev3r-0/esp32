#line 1 "/home/jay/esp32/src/h/buttons.h"
#ifndef BUTTONS_H
#define BUTTONS_H

#include "Arduino.h"

const uint8_t BUTTON_PINS[] = {1, 2, 3, 4, 5, 6};

struct ButtonState {
    bool isPressed;
    bool stateChanged;
};

extern ButtonState currentSystemButtons[];

void initButtons() {
    for (int i = 0; i < 6; i++) {
        pinMode(BUTTON_PINS[i], INPUT_PULLUP);
        currentSystemButtons[i].isPressed = false;
        currentSystemButtons[i].stateChanged = false;
    }
}

void updateButtons() {
    for (int i = 0; i < 6; i++) {
        bool rawRead = (digitalRead(BUTTON_PINS[i]) == LOW);
        if (rawRead != currentSystemButtons[i].isPressed) {
            currentSystemButtons[i].isPressed = rawRead;
            currentSystemButtons[i].stateChanged = true;
        } else {
            currentSystemButtons[i].stateChanged = false;
        }
    }
}

#endif
