// Declaration of Libraries
//LCD
#include <Wire.h> 

#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27,20,4); 

//Button
#include <Button.h>

//Encoder
#include <ClickEncoder.h>

//Timer 1 for encoder
#include <TimerOne.h>

//Save Function
#include <EEPROMex.h>

//Declaration of Variables
//Rotary Encoder Variables
boolean up = false;
boolean down = false;
boolean middle = false;

ClickEncoder *encoder;
int16_t last, value;

//LCD Menu Logic
byte enterChar[] = {
B10000,
  B10000,
  B10100,
  B10110,
  B11111,
  B00110,
  B00100,
  B00000
};

byte fastChar[] = {
  B10111,
  B10101,
  B10111,
  B00000,
  B00000,
  B00100,
  B01110,
  B11111
};

int refreshScreen=0;

const int numOfMainScreens = 3;
const int numOfSettingScreens = 3;
const int numOfTestMenu = 7;
int currentScreen = 0;
int currentSettingScreen = 0;
int currentTestMenuScreen = 0;

String screens[numOfMainScreens][2] = {{"Settings","Click to Edit"}, {"Run Auto", "Enter to Run"},{"Test Machine", "Enter to Test"}};
String settings[numOfSettingScreens][2] = {{"Mixing Time","Mins"},{"Length", "Sec"},{"Save Settings", "Enter to Save"}};
String TestMenuScreen[numOfTestMenu] = {"Test Mixer", "Test Extruder", "Test Cutter","Test Conveyor","Test Fan","Test Sugar","Back to Main Menu"};


double parameters[numOfSettingScreens]={1,1,1};
double parametersMaxValue[numOfSettingScreens]={60,60,60};

bool settingsFlag = false;
bool settingsEditFlag = false;
bool testMenuFlag = false;
bool runAutoFlag = false;
bool runManualFlag = false;

// Timer
unsigned long previousMillis = 0;
unsigned long currentMillis = 0;

// Declaration Relays
int mixerFWD = 12;
int mixerRWD = 11;
int conveyor = 10;
int mixerPaddle = 5;
int sugarSprinkler = 9;
int coolingFan = 6;
int pneumaticCutter = 7;

int sensor = A0;

//Fast Scroll
bool fastScroll = false;

// Timer Variables
unsigned long minRemaining = 0;
unsigned long secRemaining = 0;



unsigned long mixingTime = 60000;

unsigned long cutterIntervalTime = 0;


//Functions for Rotary Encoder
void timerIsr(){
  encoder->service();
}

void readRotaryEncoder(){
  value += encoder->getValue();
  
  if (value/2 > last) {
    last = value/2;
    down = true;
    //delay(100);
  }else   if (value/2 < last) {
    last = value/2;
    up = true;
    //delay(100);
  }
}

    // Refresh Screen Every 1 Second
    unsigned long currentTime = millis();
    const unsigned long eventInterval = 1000;
    unsigned long previousTime = 0;

    void refreshScreensEvery1Second(){
        currentTime = millis();
        if (currentTime - previousTime >= eventInterval) {
            
            refreshScreen = 1;
        /* Update the timing for the next time around */
            previousTime = currentTime;
        }
    }

    //Cutter Variables

    unsigned long currentTime1 = millis();
    unsigned long cutterTickTimer = 3000;
    unsigned long previousTime1 = 0;
  

    // VARIABLES FOR RUN COMMANDS
        bool runMixerMixFlag, 
            runMixerExtrudeFlag,
            runPressCutterFlag,
            runSugarSprinklerFlag,
            runConveyorFlag,
            runCoolingFanFlag, 
            finishExtruder,
            readSensorFlag = false;

    // FUNCTIONS FOR RUN
        void runMixerMix(){
            // Mixer Timer Here
            if(currentMillis - previousMillis <= mixingTime){
                digitalWrite(mixerRWD, LOW);
        digitalWrite(mixerPaddle, LOW);
                secRemaining = (((mixingTime- (currentMillis - previousMillis))/ 1000));
                minRemaining = (((mixingTime- (currentMillis - previousMillis))/ 1000)/60);
                Serial.print("Remaining Time:");
                Serial.println(secRemaining);
            }else{
                digitalWrite(mixerRWD, HIGH);
                runMixerMixFlag = false;
                runMixerExtrudeFlag = true;
                runConveyorFlag = true;
        runPressCutterFlag = true;
        readSensorFlag = true;
                refreshScreen = 1;
            }
        }

        void runMixerExtrude(){
    digitalWrite(mixerFWD, LOW);
    digitalWrite(mixerPaddle, LOW);
                Serial.println("test extrude");
            if (finishExtruder == true)
            {
                // End Extruder
            }
            else
            {
                
                if (readSensorFlag == true)
                {
                    if (digitalRead(sensor)==0)
                    {
                        readSensorFlag = false;
                        runSugarSprinklerFlag = true;
                runCoolingFanFlag = true;
                    }
                }
            }
        }

        void runPressCutter(){
            currentTime1 = millis();
            if (currentTime1- previousTime1 >= cutterTickTimer) {

                digitalWrite(pneumaticCutter, LOW);
                delay(200);
                digitalWrite(pneumaticCutter, HIGH);

                /* Update the timing for the next time around */
                previousTime1 = currentTime1;
                    }
    Serial.println("Test Cutter");
        }

        void runSugarSprinkler(){
            // Sugar Sprinkler
            digitalWrite(sugarSprinkler, LOW);
        }

        void runConveyor(){
            // Run Conveyor
            digitalWrite(conveyor, LOW);
        }

        void runCoolingFan(){
            // Cooling Fan Here
            digitalWrite(coolingFan, LOW);
        }

        void runAuto(){
            if (runMixerMixFlag == true)
            {
                runMixerMix();
            }
            if(runMixerExtrudeFlag == true)
            {
                runMixerExtrude();
            }
            if(runPressCutterFlag == true)
            {
                runPressCutter();
            }
            if(runSugarSprinklerFlag == true)
            {
                runSugarSprinkler();
            }
            if(runConveyorFlag == true)
            {
                runConveyor();
            }
             if(runCoolingFanFlag == true)
            {
                runCoolingFan();
            }
        }

        void runAutoStopAll(){
            runMixerMixFlag = false;
            runMixerExtrudeFlag = false;
            runPressCutterFlag = false;
            runSugarSprinklerFlag = false;
            runConveyorFlag = false;
            runCoolingFanFlag = false;

            digitalWrite(mixerFWD,HIGH);
            digitalWrite(mixerRWD,HIGH);
            digitalWrite(conveyor,HIGH);
            digitalWrite(sugarSprinkler,HIGH);
            digitalWrite(coolingFan,HIGH);
            digitalWrite(pneumaticCutter,HIGH);
    digitalWrite(mixerPaddle,HIGH);
        }

    // VARIABLES FOR RUN COMMANDS
        bool testRunMixerMixFlag, 
            testRunMixerExtrudeFlag,
            testRunPressCutterFlag,
            testRunSugarSprinklerFlag,
            testRunConveyorFlag,
            testRunCoolingFanFlag = false;

    // FUNCTIONS FOR TEST RUN
        void testRunMixerMix(){
            // Mixer Timer Here
            if (testRunMixerMixFlag == true)
            {
                digitalWrite(mixerRWD, LOW);
        digitalWrite(mixerPaddle, LOW);
            }
            else
            {
                digitalWrite(mixerRWD, HIGH);
        digitalWrite(mixerPaddle, HIGH);
            }
            
        }

        void testRunMixerExtrude(){
            // Mixer Timer Extrude
            if (testRunMixerExtrudeFlag == true)
            {
                digitalWrite(mixerFWD, LOW);
        digitalWrite(mixerPaddle, LOW);
            }
            else
            {
                digitalWrite(mixerFWD, HIGH);
        digitalWrite(mixerPaddle, HIGH);
            }
        }

        void testRunPressCutter(){
            // Press Cutter
            if (testRunPressCutterFlag == true)
            {
                digitalWrite(pneumaticCutter, LOW);
                delay(200);
                digitalWrite(pneumaticCutter, HIGH);
                testRunPressCutterFlag = false;
            }
        }

        void testRunSugarSprinkler(){
            // Sugar Sprinkler
            if (testRunSugarSprinklerFlag == true)
            {
                digitalWrite(sugarSprinkler, LOW);
            }
            else
            {
                digitalWrite(sugarSprinkler, HIGH);
            }
        }

        void testRunConveyor(){
            // Run Conveyor
            if (testRunConveyorFlag == true)
            {
                digitalWrite(conveyor, LOW);
            }
            else
            {
                digitalWrite(conveyor, HIGH);
            }
        }

        void testRunCoolingFan(){
            // Cooling Fan Here
            if (testRunCoolingFanFlag == true)
            {
                digitalWrite(coolingFan, LOW);
            }
            else
            {
                digitalWrite(coolingFan, HIGH);
            }
        }

        void testRun(){
            testRunMixerMix();
            testRunMixerExtrude();
            testRunPressCutter();
            testRunSugarSprinkler();
            testRunCoolingFan();
            testRunConveyor();
        }

        void testRunStopAll(){
            testRunMixerMixFlag, 
            testRunMixerExtrudeFlag,
            testRunPressCutterFlag,
            testRunSugarSprinklerFlag,
            testRunConveyorFlag,
            testRunCoolingFanFlag = false;
    digitalWrite(mixerFWD,HIGH);
            digitalWrite(mixerRWD,HIGH);
            digitalWrite(conveyor,HIGH);
            digitalWrite(sugarSprinkler,HIGH);
            digitalWrite(coolingFan,HIGH);
            digitalWrite(pneumaticCutter,HIGH);
    digitalWrite(mixerPaddle,HIGH);
        }

        

//LCD Functions
void printScreens(){
    
    if (settingsFlag == true)
    {
        
        lcd.begin(20,4);
        lcd.clear();
        lcd.print(settings[currentSettingScreen][0]);
        if(currentSettingScreen == 5){
            lcd.setCursor(0,3);
            lcd.write(0);
            lcd.setCursor(2,3);
            lcd.print("Click to Save All");
        
        }
        else
        {
            lcd.setCursor(0,1);
            lcd.print(parameters[currentSettingScreen]);
            lcd.print(" ");
            lcd.print(settings[currentSettingScreen][1]);
            lcd.setCursor(0,3);
            lcd.write(0);
            lcd.setCursor(2,3);
            if(settingsEditFlag == false){
                lcd.print("Click to Edit");   
            }else{
                lcd.print("Click to Save");  
            }

            lcd.setCursor(19,3);
            if (fastScroll == true)
            {
                lcd.write(1);
            }
        }
    }
    else if (runAutoFlag == true)
    {
        lcd.begin(20,4);
        lcd.clear();
        lcd.print("Processing Pastillas");
        lcd.setCursor(0,1);
        lcd.print("Current Process");
        
        lcd.setCursor(0,2);
            if (runMixerMixFlag == true)
            {
                lcd.print("Mixing");
            }
            else if(runMixerExtrudeFlag == true)
            {
                lcd.print("Extruding");
            }
            else
            {
                lcd.print("Waiting for Commands");
            }
        
        lcd.setCursor(0,3);
            if (runMixerMixFlag == true)
            {
                lcd.print("Time : ");
                if (minRemaining >= 1)
                {
                    lcd.print(minRemaining);
                    lcd.print(" Minute");
                }
                else
                {
                    lcd.print(secRemaining);
                    lcd.print(" Seconds");
                }
            }
        
    }
    else if (testMenuFlag == true)
    {
        lcd.begin(20,4);
        lcd.clear();
        lcd.print(TestMenuScreen[currentTestMenuScreen]);
        if (currentTestMenuScreen == 0)
        {
            if (testRunMixerMixFlag == true)
            {
                lcd.setCursor(0,1);
                lcd.print("Mixing is On");
                lcd.setCursor(0,3);
                lcd.print("Click to turn Off");
            }
            else
            {
                lcd.setCursor(0,1);
                lcd.print("Mixing is Off");
                lcd.setCursor(0,3);
                lcd.print("Click to turn On");
            }
            
        }
        else if (currentTestMenuScreen == 1)
        {
            if (testRunMixerExtrudeFlag == true)
            {
                lcd.setCursor(0,1);
                lcd.print("Extruder is On");
                lcd.setCursor(0,3);
                lcd.print("Click to turn Off");
            }
            else
            {
                lcd.setCursor(0,1);
                lcd.print("Extruder is Off");
                lcd.setCursor(0,3);
                lcd.print("Click to turn On");
            }
        }
        else if (currentTestMenuScreen == 2)
        {
                lcd.setCursor(0,1);
                lcd.print("Press Cutter");
                lcd.setCursor(0,3);
                lcd.print("Click to Cut");
        }
        else if (currentTestMenuScreen == 3)
        {
            if (testRunConveyorFlag == true)
            {
                lcd.setCursor(0,1);
                lcd.print("Conveyor is On");
                lcd.setCursor(0,3);
                lcd.print("Click to turn Off");
            }
            else
            {
                lcd.setCursor(0,1);
                lcd.print("Conveyor is Off");
                lcd.setCursor(0,3);
                lcd.print("Click to turn On");
            }
        }
        else if (currentTestMenuScreen == 4)
        {
            if (testRunCoolingFanFlag == true)
            {
                lcd.setCursor(0,1);
                lcd.print("Fan is On");
                lcd.setCursor(0,3);
                lcd.print("Click to turn Off");
            }
            else
            {
                lcd.setCursor(0,1);
                lcd.print("Fan is Off");
                lcd.setCursor(0,3);
                lcd.print("Click to turn On");
            }
        }
        else if (currentTestMenuScreen == 5)
        {
            if (testRunSugarSprinklerFlag == true)
            {
                lcd.setCursor(0,1);
                lcd.print("Sprinkler is On");
                lcd.setCursor(0,3);
                lcd.print("Click to turn Off");
            }
            else
            {
                lcd.setCursor(0,1);
                lcd.print("Sprinkler is Off");
                lcd.setCursor(0,3);
                lcd.print("Click to turn On");
            }
        }
        else if (currentTestMenuScreen == 6)
        {
            lcd.setCursor(0,3);
            lcd.print("Click to Exit Test");
        }
        
    }
    else
    {
        lcd.begin(20,4);
        lcd.clear();
        lcd.print(screens[currentScreen][0]);
        lcd.setCursor(0,3);
        lcd.write(0);
        lcd.setCursor(2,3);
        lcd.print(screens[currentScreen][1]);
    }
}

    //Save and Load Settings
     //Save to EEPROMex Adresses
    int mixTimeAdd = 0;
    int lengthAdd = 10;

    double mixTime = 0;
    double length = 0;

    void save_setting(){
        mixTime =  parameters[0];
        length = parameters[1];
  
        EEPROM.writeDouble(mixTimeAdd, mixTime);
        EEPROM.writeDouble(lengthAdd, length);

    }

    void load_settings(){
        parameters[0] = EEPROM.readDouble(mixTimeAdd);
        parameters[1] = EEPROM.readDouble(lengthAdd);

        mixTime = EEPROM.readDouble(mixTimeAdd);
        length = EEPROM.readDouble(lengthAdd);
        mixingTime = mixTime * 60000;
        cutterTickTimer = length * 1000;
    }

void setup()
{
    //Encoder Setup
    encoder = new ClickEncoder(4,3, 2);
    encoder->setAccelerationEnabled(false);
    Timer1.initialize(1000);
    Timer1.attachInterrupt(timerIsr); 
    last = encoder->getValue();


    //Pin Initialize Relay
    pinMode(mixerFWD, OUTPUT);
    digitalWrite(mixerFWD, HIGH);

    pinMode(mixerRWD, OUTPUT);
    digitalWrite(mixerRWD, HIGH);

    pinMode(conveyor, OUTPUT);
    digitalWrite(conveyor, HIGH);

    pinMode(sugarSprinkler, OUTPUT);
    digitalWrite(sugarSprinkler, HIGH);

    pinMode(coolingFan, OUTPUT);
    digitalWrite(coolingFan, HIGH);

    pinMode(pneumaticCutter, OUTPUT);
    digitalWrite(pneumaticCutter, HIGH);

    pinMode(mixerPaddle, OUTPUT);
    digitalWrite(mixerPaddle, HIGH);
    
    //Pin Initialize Sensors
    pinMode(sensor, INPUT_PULLUP);

    //LCD Setup
    lcd.init();
    lcd.createChar(0, enterChar);
    lcd.createChar(1, fastChar);
    lcd.clear();
    lcd.backlight();
    lcd.setCursor(0,0);
    lcd.print("********************");
    lcd.setCursor(0,1);
    lcd.print("*");
    lcd.setCursor(3,1);
    lcd.print("PASTILLAS");
    lcd.setCursor(19,1);
    lcd.print("*");
    lcd.setCursor(0,2);
    lcd.print("*");
    lcd.setCursor(6,2);
    lcd.print("MACHINE");
    lcd.setCursor(19,2);
    lcd.print("*");
    lcd.setCursor(0,3);
    lcd.print("********************");

    //Serial Debug
    Serial.begin(9600);
    
    save_setting();
    load_settings();
   

}

void loop()
{   // Open Bracket for Main Loop
    
    if(digitalRead(sensor)== 0){
        Serial.println("Sensor 0");
    }else{
        Serial.println("Sensor 1");
    }
    currentMillis = millis();
    readRotaryEncoder(); 
    ClickEncoder::Button b = encoder->getButton();
    if (b != ClickEncoder::Open) {  // Open Bracket for Click
    switch (b) { // Open Bracket for Double Click
        case ClickEncoder::Clicked: 
            middle=true; 
        break;

        case ClickEncoder::DoubleClicked:
            Serial.println("ClickEncoder::DoubleClicked");
            refreshScreen=1;
            if (settingsFlag)
            {
                if (fastScroll == false)
                {
                    fastScroll = true;

                }
                else
                {
                    fastScroll = false;
                }

            }
        break;

    } // Close Bracket for Double Click
    } // Close Bracket for Click

    //LCD Change Function and Values
    // To Right Rotary
    if (up == 1)
    { // Open Bracket for Scroll Up
        up= false;
        refreshScreen=1;
        if (settingsFlag == true)
        {
            if (settingsEditFlag == true)
            {
                if (parameters[currentSettingScreen] >= parametersMaxValue[currentSettingScreen]-1 ){
                    parameters[currentSettingScreen] = parametersMaxValue[currentSettingScreen];
                }else
                {
                if(currentSettingScreen == 2){
                    parameters[currentSettingScreen] += 0.1;
                }else{
                    parameters[currentSettingScreen] += 1;
                }
                }
            }
            else
            {
                if (currentSettingScreen == numOfSettingScreens-1) {
                    currentSettingScreen = 0;
                }else{
                    currentSettingScreen++;
                }
            }
        }
        else if (testMenuFlag == true)
        {
            if (currentTestMenuScreen == numOfTestMenu-1) {
                currentTestMenuScreen = 0;
            }else{
                currentTestMenuScreen++;
            }
        }
        else
        {
            if (currentScreen == numOfMainScreens-1) {
                currentScreen = 0;
            }else{
                currentScreen++;
            }
        }
    }// Close Bracket for Scroll Up

    // To Left Rotary
    if (down == 1)
    {// Open Bracket for Scroll Down
        down = false;
        refreshScreen=1;
        if (settingsFlag == true)
        {
            
            if (settingsEditFlag == true)
            {

                if (parameters[currentSettingScreen] <= 0 ){
                    parameters[currentSettingScreen] = 0;
                }else
                {
                    if(currentSettingScreen == 2){
                        parameters[currentSettingScreen] -= 0.1;
                    }else{
                        parameters[currentSettingScreen] -= 1;
                    }
                }
            }
            else
            {
                if (currentSettingScreen == 0) {
                    currentSettingScreen = numOfSettingScreens-1;
                }else{
                    currentSettingScreen--;
                }
            }
        }
        else if (testMenuFlag == true)
        {
            if (currentTestMenuScreen == 0) {
                currentTestMenuScreen = numOfTestMenu-1;
            }else{
                currentTestMenuScreen--;
            }
        }
        else
        {
            if (currentScreen == 0) {
                currentScreen = numOfMainScreens-1;
            }else{
                currentScreen--;
            }
        }
    }// Close Bracket for Scroll Down

    // Rotary Button Press
    if (middle==1)
    {
        middle = false;
        refreshScreen=1;

        if (settingsFlag == true)
        {
            if (currentSettingScreen == 3)
            {
                settingsFlag = false;
                // Save Here
                save_setting();
                load_settings();
            }
            else
            {
                if (settingsEditFlag == true)
                {
                    settingsEditFlag = false;
                }
                else
                {
                    settingsEditFlag = true;
                }
            }
        }
        else if (runAutoFlag == true)
        {
            runAutoFlag = false;
            runAutoStopAll();
        }
        else if (testMenuFlag == true)
        {
            if (currentTestMenuScreen == 0)
            {
                if (testRunMixerExtrudeFlag == true)
                {
                    testRunMixerExtrudeFlag = false;
                    testRunMixerMixFlag = true;
                }
                else
                {
                    if (testRunMixerMixFlag == true)
                    {
                        testRunMixerMixFlag = false;
                    }
                    else
                    {
                        testRunMixerMixFlag = true;
                    }
                    
                }
                
            }
            else if(currentTestMenuScreen == 1)
            {
                if (testRunMixerMixFlag == true)
                {
                    testRunMixerMixFlag = false;
                    testRunMixerExtrudeFlag = true;
                }
                else
                {
                    if (testRunMixerExtrudeFlag == true)
                    {
                        testRunMixerExtrudeFlag = false;
                    }
                    else
                    {
                        testRunMixerExtrudeFlag = true;
                    }
                    
                }
            }
            else if(currentTestMenuScreen == 2)
            {
                testRunPressCutterFlag = true;
            }
            else if(currentTestMenuScreen == 3)
            {
                if (testRunConveyorFlag == true)
                {
                    testRunConveyorFlag = false;
                }
                else
                {
                    testRunConveyorFlag = true;
                }
                
            }
            else if(currentTestMenuScreen == 4)
            {
                if (testRunCoolingFanFlag == true)
                {
                    testRunCoolingFanFlag = false;
                }
                else
                {
                    testRunCoolingFanFlag = true;
                }
            }
            else if(currentTestMenuScreen == 5)
            {
                 if (testRunSugarSprinklerFlag == true)
                {
                    testRunSugarSprinklerFlag = false;
                }
                else
                {
                    testRunSugarSprinklerFlag = true;
                }
            }
            else if(currentTestMenuScreen == 6)
            {
                testRunStopAll();
                testMenuFlag = false;
            }
        }
        else
        {
            if (currentScreen == 0)
            {
                settingsFlag = true;
            }
            else if (currentScreen == 1)
            {
                // Run Auto Command Here
                runAutoFlag = true;
                runMixerMixFlag= true;
                previousMillis = currentMillis;
            }
            else
            {
                testRunStopAll();
                testMenuFlag = true;
            }
            
        }
    }

    if(refreshScreen==1){
        printScreens();
        refreshScreen = 0;
    }

    if(testMenuFlag==true){
        testRun();
    }
    
    if(runAutoFlag==true){
        runAuto();
    }

    if(runAutoFlag == true && runMixerMixFlag == true){
        refreshScreensEvery1Second();
    }
} // Close Bracket for Main Loop


