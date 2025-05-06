#ifndef LCD_H
#define LCD_H

#include <TFT.h>
#include <SPI.h>
#include "rotary_encoder.h"

#define CS_TFT  4
#define DC_TFT  7
#define RST_TFT 5

TFT TFTscreen = TFT(CS_TFT, DC_TFT, RST_TFT);

const int numItems = 3;
const char* menuItems[numItems] = {"1. Get Location", "2. Play a Game", "3. Show Triangle"};
bool inGame = false;
bool inLocationView = false;
bool inTriangleView = false;
void lcdUpdateMenu();
void startListening();
void lcdShowLocation();
void lcdStartGame();
void lcdShowTriangle();
void updateBeta(float beta_1, float beta_2, float beta_3);
void updateAlpha(float alpha_1, float alpha_2, float alpha_3);
void updateDistance(float distance_1, float distance_2, float distance_3);
void updateStage(String newStage);
void updateChannel(int channel_p);

bool isListening=false;

// Data buffers
char countStr[10];
char stageStr[6];
char channelStr[3]="111";
char betaStr[7];
char alphaStr[7];

int x=35;
int y=30;

void lcdSetup() {
    TFTscreen.begin();
    TFTscreen.background(0, 0, 0);
    lcdUpdateMenu();
}

void lcdUpdateMenu() {
    static int lastSelectedItem = -1;
    if (lastSelectedItem == selectedItem) return;
    lastSelectedItem = selectedItem;
    
    TFTscreen.background(0, 0, 0);
    TFTscreen.setTextSize(1);

    for (int i = 0; i < numItems; i++) {
        if (i == selectedItem) {
            TFTscreen.stroke(0, 255, 0);
            TFTscreen.fillRect(15, 28 + (i * 15), 100, 10, "blue");
        } else {
            TFTscreen.stroke(255, 255, 255);
        }
        TFTscreen.text(menuItems[i], 20, 30 + (i * 15));
    }
}

void lcdUpdateSelection() {
    selectedItem = getSelectedItem();
    isClicked = isItemClicked();
    lcdUpdateMenu();
    if (isClicked) {
        if (selectedItem == 0 || selectedItem == 1) {
            inLocationView = true;
            isListening=true;
            lcdShowLocation();
            delay(100);
        } else if (selectedItem == 3 || selectedItem == 4) {
            inGame = true;
            lcdStartGame();
        } else if (selectedItem == 6 || selectedItem == 7) {
            inTriangleView = true;
            lcdShowTriangle();
        }
    }
}

void lcdShowLocation() {
    TFTscreen.background(0, 0, 0);
    TFTscreen.stroke(255, 255, 255);
    TFTscreen.setTextSize(1);
    TFTscreen.text("\t\t\t\t# The Navigator #", 0, 0);
    TFTscreen.setTextSize(1.5);

    int x = 35, y = 30;
    TFTscreen.text("B:", x, y);
    TFTscreen.text("A:", x, y+10);
    TFTscreen.text("D", x, y+20);
    TFTscreen.text("Chan:", x, y+30);

    TFTscreen.text("78", x+30, y+30);
    TFTscreen.text("90", x+60, y+30);
    TFTscreen.text("100", x+80, y+30);

    TFTscreen.text("Slave", x+40, y+40);
    TFTscreen.text("Chan", x, y+50);
    TFTscreen.text("Beta", x+40, y+50);
    
    TFTscreen.text("M_1", x+30, y-20);
    TFTscreen.text("M_2", x+60, y-20);
    TFTscreen.text("M_3", x+90, y-20);

    TFTscreen.setTextSize(0);
    TFTscreen.text("\t\t\t @kito gps #thefuture", 10, 120);
    TFTscreen.setTextSize(1);
}

void lcdStartGame() {
    TFTscreen.background(0, 0, 0);
    TFTscreen.stroke(255, 255, 255);
    TFTscreen.setTextSize(1);
    TFTscreen.text("Rotate Encoder!", 20, 40);
    int score = 0;
    while (inGame) {
        if (getSelectedItem() != selectedItem) {
            score++;
            selectedItem = getSelectedItem();
            TFTscreen.text("Score: ", 20, 60);
            TFTscreen.text(String(score).c_str(), 80, 60);
        }
        if (isItemClicked()) {
            inGame = false;
        }
    }
    lcdSetup();
}

void lcdShowTriangle() {
    TFTscreen.background(0, 0, 0);
    TFTscreen.stroke(255, 255, 255);
    TFTscreen.line(50, 100, 100, 50);
    TFTscreen.line(100, 50, 150, 100);
    TFTscreen.line(150, 100, 50, 100);
    TFTscreen.text("A", 45, 105);
    TFTscreen.text("B", 95, 45);
    TFTscreen.text("C", 145, 105);
    delay(5000);
    lcdSetup();
}

void updateBeta(float beta_1, float beta_2, float beta_3) {
    SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
    static char betaStr1[10]="120.000";
    static char betaStr2[10]="120.000";
    static char betaStr3[10]="120.000";
    TFTscreen.stroke(0, 0, 0);
    TFTscreen.text(betaStr1,x+10,y);
    TFTscreen.text(betaStr2,x+50,y);
    TFTscreen.text(betaStr3,x+90,y);

    TFTscreen.stroke(245, 95, 84);
    dtostrf(beta_1, 4, 4, betaStr1);
    TFTscreen.text(betaStr1, x+10, y);

    dtostrf(beta_2, 4, 4, betaStr2);
    TFTscreen.text(betaStr2, x+50, y);

    dtostrf(beta_3, 4, 4, betaStr3);
    TFTscreen.text(betaStr3, x+90, y);

    SPI.endTransaction();
}

void updateAlpha(float alpha_1, float alpha_2, float alpha_3) {
    SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
    static char alphaStr1[10]="120.000";
    static char alphaStr2[10]="120.000";
    static char alphaStr3[10]="120.000";
    TFTscreen.stroke(0, 0, 0);
    TFTscreen.text(alphaStr1,x+10,y+10);
    TFTscreen.text(alphaStr2,x+50,y+10);
    TFTscreen.text(alphaStr3,x+90,y+10);

    TFTscreen.stroke(222, 79, 35);
    dtostrf(alpha_1, 4, 2, alphaStr1);
    TFTscreen.text(alphaStr1, x+10, y+10);

    dtostrf(alpha_2, 4, 2, alphaStr2);
    TFTscreen.text(alphaStr2, x+50, y+10);

    dtostrf(alpha_3, 4, 2, alphaStr3);
    TFTscreen.text(alphaStr3, x+90, y+10);

    SPI.endTransaction();
}

void updateDistance(float distance_1, float distance_2, float distance_3) {
    SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
    static char distanceStr1[10]="120.000";
    static char distanceStr2[10]="120.000";
    static char distanceStr3[10]="120.000";
    TFTscreen.stroke(0, 0, 0);
    TFTscreen.text(distanceStr1,x+10,y+20);
    TFTscreen.text(distanceStr2,x+50,y+20);
    TFTscreen.text(distanceStr3,x+90,y+20);

    TFTscreen.stroke(68,178,20);
    dtostrf(distance_1, 4, 2, distanceStr1);
    TFTscreen.text(distanceStr1, x+10, y+20);

    dtostrf(distance_2, 4, 2, distanceStr2);
    TFTscreen.text(distanceStr2, x+50, y+20);

    dtostrf(distance_3, 4, 2, distanceStr3);
    TFTscreen.text(distanceStr3, x+90, y+20);

    SPI.endTransaction();
}

void updateStage(String newStage) {
    SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));

    TFTscreen.stroke(0, 0, 0);
    TFTscreen.text(stageStr, 100, 80);

    newStage.toCharArray(stageStr, sizeof(stageStr));
    TFTscreen.stroke(84, 245, 95);
    TFTscreen.text(stageStr, 80, 80);

    SPI.endTransaction();
}

void updateChannel(int channel_p) {
    SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));

    TFTscreen.stroke(0, 0, 0);
    TFTscreen.text(channelStr, x, y+60);

    itoa(channel_p, channelStr, 10);
    TFTscreen.stroke(84, 217, 245);
    TFTscreen.text(channelStr, x, y+60);

    SPI.endTransaction();
}


void updateBeta(double beta_p) {
    SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
    TFTscreen.stroke(0, 0, 0);
    TFTscreen.text(betaStr, x+40, y+60);

    dtostrf(beta_p, 5, 6, betaStr);

    TFTscreen.stroke(84, 217, 245);
    TFTscreen.text(betaStr, x+40, y+60);

    SPI.endTransaction();
}

#endif // LCD_H
