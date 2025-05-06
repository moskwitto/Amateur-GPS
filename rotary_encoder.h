#ifndef ROTARY_ENCODER_H
#define ROTARY_ENCODER_H

#define BUTTON_1 A3  //change to 3
#define OUTPUT_A A0
#define OUTPUT_B A1

int buttonDelay = 0;
int lastStateA = LOW;
int lastStateB = LOW;
int selectedItem = 4; 
bool isClicked = false; // Tracks whether an item is clicked

void rotaryEncoderSetup() {
    // pinMode(BUTTON_1, INPUT_PULLUP);
    // pinMode(OUTPUT_A, INPUT_PULLUP);
    // pinMode(OUTPUT_B, INPUT_PULLUP);
}

// Returns true if the button is pressed
bool buttonStatus() {
    if (digitalRead(BUTTON_1) == LOW) {
        Serial.println("Button pressed");
        isClicked = true;
        return true;
    }
    return false;
}

// Updates the selection index based on encoder rotation
void rotaryStatus() {
    // Read the states of both A and B pins
    int stateA = digitalRead(OUTPUT_A);
    int stateB = digitalRead(OUTPUT_B);

    // Detect the direction of rotation
    if (stateA != lastStateA) {
        if (stateB != stateA) {
            selectedItem++; // Clockwise rotation
            Serial.println(selectedItem);
            delay(100);
            if (selectedItem >= 8) selectedItem = 3; // Cap at 3 items
        } else {
            selectedItem--; // Counter-clockwise rotation
            Serial.println(selectedItem);
            delay(100);
            if (selectedItem <= 0) selectedItem = 0; // Don't go below 0
        }
        isClicked = false; // Reset the click status when rotating
    }

    // Update the last states for the next loop
    lastStateA = stateA;
    lastStateB = stateB;
}

// Getter functions
int getSelectedItem() {
    return selectedItem;
}

// Return true if the button is clicked
bool isItemClicked() {
    bool clicked = isClicked;
    isClicked = false; // Reset after checking
    return clicked;
}

#endif  // ROTARY_ENCODER_H
