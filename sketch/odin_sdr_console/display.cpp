/////////////////////////////////////////////////////////////////////////
//
// Odin SDR Console by Laurence Barker G8NJJ
//
// this sketch provides a knob and switch interface through USB and CAT
//
// display.c
// this file holds the code to control a Nextion 3.2" display
// it is 400x240 pixels
// note the Nextion Display appears on "Serial1"
// its serial port is defined in nexconfig.h in the library folder
/////////////////////////////////////////////////////////////////////////

#include "display.h"
#include "types.h"
#include "led.h"
#include "stdlib.h"
#include "configdata.h"
#include "cathandler.h"
#include "encoders.h"

EDisplayPage GDisplayPage;                    // global set to current display page number
EControlTypes GControlType;                   // type of control being edited
unsigned int GControlNumber;                  // number of control being edited
unsigned int GActionNumber;                   // displayed action of control
unsigned int G2ndActionNumber;                // displayed action of control
//
// variables to hold what's currently on the display
// the display is only updated if the new value is different
// to reduce flicker, CPU time
// (these initialise to zero)
//
bool DisplayABState = true;        // current A/B setting (true = A)
bool DisplayTXState;               // current TX state true=TX
bool DisplayTuneState;             // current TUNE state true = tune
bool DisplayRITState;              // current RIT state true=on
bool DisplayCurrentSplitState;     // currently displayed SPLIT state
bool DisplayCurrentLockState;      // currently displayed LOCK state
char DisplayCurrentFrequency[20] = "14.101056";  // currently displayed frequency
int DisplayCurrentSReading = 50;         // current S Meter reading (pointer angle; 34 to 146)
int DisplayCurrentPowerReading = 70;     // current TX Power Meter reading (pointer angle; 34 to 146)
//EBand DisplayCurrentBand;          // currently displayed mode NOT NEEDED
EMode DisplayCurrentMode;          // currently displayed mode
ENRState DisplayCurrentNRState;    // currently displayed NR setting
ENBState DisplayCurrentNBState;    // currently displayed NB setting
bool DisplayCurrentSNBState;       // currently displayed SNB state
bool DisplayCurrentANFState;       // currently displayed ANF state
EAGCSpeed DisplayCurrentAGCSpeed;  // currently displayed AGC speed
//int DisplayCurrentAGCThreshold;    // currently displayed AGC threshold requested on demand
EAtten DisplayCurrentAtten;        // currently displayed attenuation
EEncoderActions DisplayCurrentAction1;                      // display 1
EEncoderActions DisplayCurrentAction2;                      // display 2
EEncoderActions DisplayCurrentAction3;                      // display 3
EEncoderActions DisplayCurrentAction4;                      // display 4
bool Enc1IsMulti;                                           // true if display 1 is multi
bool Enc2IsMulti;                                           // true if display 2 is multi
bool Enc3IsMulti;                                           // true if display 3 is multi
bool Enc4IsMulti;                                           // true if display 4 is multi




//
// declare pages:
//
NexPage page0main = NexPage(1, 0, "page0main");   // creates touch event for "main" page
NexPage page1 = NexPage(2, 0, "page1");       // creates touch event for "I/O Test" page
NexPage page2 = NexPage(3, 0, "page2");       // creates touch event for "About" page
NexPage page3 = NexPage(4, 0, "page3");       // creates touch event for "Frequency entry" page
NexPage page4 = NexPage(5, 0, "page4");       // creates touch event for "Band" page
NexPage page5 = NexPage(6, 0, "page5");       // creates touch event for "mode" page
NexPage page6 = NexPage(7, 0, "page6");       // creates touch event for "NR" page
NexPage page7 = NexPage(8, 0, "page7");       // creates touch event for "RF" page
NexPage page8 = NexPage(9, 0, "page8");       // creates touch event for "general settings" page
NexPage page9 = NexPage(10, 0, "page9");       // creates touch event for "configure" page
NexPage page0 = NexPage(0, 0, "page0");       // creates touch event for "splash" page

//
// page 0 objects:
//
NexText p0t0 = NexText(1, 1, "t0");                   // VFO A/B
//NexText p0t1 = NexText(1, 3, "t1");                   // static (says RIT)
NexText p0t2 = NexText(1, 4, "t2");                   // frequency
NexText p0t8 = NexText(1, 13, "t8");                  // mode
NexText p0t4 = NexText(1, 6, "t4");                   // RHS encoder
NexText p0t5 = NexText(1, 7, "t5");                   // LHS encoder
NexText p0t6 = NexText(1, 8, "t6");                   // middle encoder
NexText p0t7 = NexText(1, 15, "t7");                  // RHS encoder
NexText p0t9 = NexText(1, 17, "t9");                  // LOCK
NexText p0t10 = NexText(1, 18, "t10");                // RIT "on" text
NexText p0t11 = NexText(1, 19, "t11");                // SPLIT
NexGauge p0z0 = NexGauge(1, 14, "z0");                // gauge


//
// declare objects on I/O test page:
// these are all on "page1"
NexDSButton p1bt0 = NexDSButton(2, 6, "bt0");         // LED 1 button
NexDSButton p1bt1 = NexDSButton(2, 7, "bt1");         // LED 2 button
NexDSButton p1bt2 = NexDSButton(2, 8, "bt2");         // LED 3 button
NexDSButton p1bt3 = NexDSButton(2, 9, "bt3");         // LED 4 button
NexDSButton p1bt4 = NexDSButton(2, 10, "bt4");        // LED 5 button
NexDSButton p1bt5 = NexDSButton(2, 11, "bt5");        // LED 6 button
NexDSButton p1bt6 = NexDSButton(2, 12, "bt6");        // LED 7 button
NexText p1tEnc1 = NexText(2, 14, "tenc1");            // encoder 1 display
NexText p1tEnc2 = NexText(2, 15, "tenc2");            // encoder 2 display
NexText p1tEnc3 = NexText(2, 16, "tenc3");            // encoder 3 display
NexText p1tEnc4 = NexText(2, 17, "tenc4");            // encoder 4 display
NexText p1tEnc5 = NexText(2, 18, "tenc5");            // encoder 5 display
NexText p1tEnc6 = NexText(2, 19, "tenc6");            // encoder 6 display
NexText p1tEnc7 = NexText(2, 20, "tenc7");            // encoder 7 display
NexText p1tEnc8 = NexText(2, 21, "tenc8");            // encoder 8 display
NexText p1tpb8 = NexText(2, 23, "tpb8");              // pushbutton 8 display
NexText p1tpb9 = NexText(2, 24, "tpb9");              // pushbutton 9 display
NexText p1tpb10 = NexText(2, 25, "tpb10");            // pushbutton 10 display
NexText p1tpb11 = NexText(2, 26, "tpb11");            // pushbutton 11 display
NexText p1tpb12 = NexText(2, 27, "tpb12");            // pushbutton 12 display
NexText p1tpb13 = NexText(2, 28, "tpb13");            // pushbutton 13 display
NexText p1tpb14 = NexText(2, 29, "tpb14");            // pushbutton 14 display
NexText p1tpb15 = NexText(2, 30, "tpb15");            // pushbutton 15 display
NexText p1tpb16 = NexText(2, 31, "tpb16");            // pushbutton 16 display
NexText p1tpb17 = NexText(2, 32, "tpb17");            // pushbutton 17 display
NexText p1tpb18 = NexText(2, 33, "tpb18");            // pushbutton 18 display
NexText p1tpb19 = NexText(2, 34, "tpb19");            // pushbutton 19 display
NexText p1tpb20 = NexText(2, 35, "tpb20");            // pushbutton 20 display
NexText p1tpb21 = NexText(2, 36, "tpb21");            // pushbutton 21 display
NexText p1tpb22 = NexText(2, 37, "tpb22");            // pushbutton 22 display
NexText p1tpb23 = NexText(2, 38, "tpb23");            // pushbutton 23 display

//
// page 2 objects:
//
NexButton p2b1 = NexButton(3, 6, "p2b1");             // encoder button
NexButton p2b2 = NexButton(3, 7, "p2b2");             // pushbutton button
NexButton p2b3 = NexButton(3, 8, "p2b3");             // indicator button
NexButton p2b5 = NexButton(3, 10, "b5");              // save settings button

//
// page 3 objects:
//
NexButton p3b1 = NexButton(4, 5, "b1");               // set frequency button
NexText p3t2 = NexText(4, 2, "t2");                   // frequency text


//
// page 4 objects:
//
NexVariable p4vaband = NexVariable(5, 16, "vaband");  // band state variable
NexDSButton p4bt0 = NexDSButton(5, 4, "bt0");         // 160M button
NexDSButton p4bt1 = NexDSButton(5, 5, "bt1");         // 80M button
NexDSButton p4bt2 = NexDSButton(5, 6, "bt2");         // 60M button
NexDSButton p4bt3 = NexDSButton(5, 7, "bt3");         // 40M button
NexDSButton p4bt4 = NexDSButton(5, 8, "bt4");         // 30M button
NexDSButton p4bt5 = NexDSButton(5, 9, "bt5");         // 20M button
NexDSButton p4bt6 = NexDSButton(5, 10, "bt6");        // 17M button
NexDSButton p4bt7 = NexDSButton(5, 11, "bt7");        // 15M button
NexDSButton p4bt8 = NexDSButton(5, 12, "bt8");        // 12M button
NexDSButton p4bt9 = NexDSButton(5, 13, "bt9");        // 10M button
NexDSButton p4bt10 = NexDSButton(5, 14, "bt10");      // 6M button
NexDSButton p4bt11 = NexDSButton(5, 15, "bt11");      // "gen" button


//
// page 5 objects:
//
NexVariable p5vamode = NexVariable(6, 16, "vamode");  // mode state variable
NexDSButton p5bt0 = NexDSButton(6, 4, "bt0");         // LSB button
NexDSButton p5bt1 = NexDSButton(6, 5, "bt1");         // USB button
NexDSButton p5bt2 = NexDSButton(6, 6, "bt2");         // DSB button
NexDSButton p5bt3 = NexDSButton(6, 7, "bt3");         // CWL button
NexDSButton p5bt4 = NexDSButton(6, 8, "bt4");         // CWU button
NexDSButton p5bt5 = NexDSButton(6, 9, "bt5");         // FM button
NexDSButton p5bt6 = NexDSButton(6, 10, "bt6");        // AM button
NexDSButton p5bt7 = NexDSButton(6, 11, "bt7");        // SAM button
NexDSButton p5bt8 = NexDSButton(6, 12, "bt8");        // SPEC button
NexDSButton p5bt9 = NexDSButton(6, 13, "bt9");        // DIG L button
NexDSButton p5bt10 = NexDSButton(6, 14, "bt10");      // DIG U button
NexDSButton p5bt11 = NexDSButton(6, 15, "bt11");      // DRM button

//
// page 6 (noise) objects
//
NexDSButton p6bt2 = NexDSButton(7, 6, "bt2");         // NR off button
NexDSButton p6bt3 = NexDSButton(7, 7, "bt3");         // NR 1 button
NexDSButton p6bt4 = NexDSButton(7, 8, "bt4");         // NR 2 button
NexDSButton p6bt5 = NexDSButton(7, 9, "bt5");         // NB off button
NexDSButton p6bt6 = NexDSButton(7, 10, "bt6");        // NB 1 button
NexDSButton p6bt7 = NexDSButton(7, 11, "bt7");        // NB 2 button
NexDSButton p6bt0 = NexDSButton(7, 4, "bt0");         // SNB button
NexDSButton p6bt1 = NexDSButton(7, 5, "bt1");         // ANF button
NexVariable p6vanr = NexVariable(7, 12, "vanr");      // NR state variable
NexVariable p6vanb = NexVariable(7, 13, "vanb");      // NB state variable


//
// page 7 (RF) objects
//
NexDSButton p7bt0 = NexDSButton(8, 5, "bt0");         // 0dB button
NexDSButton p7bt1 = NexDSButton(8, 6, "bt1");         // 10dB button
NexDSButton p7bt2 = NexDSButton(8, 7, "bt2");         // 20dB button
NexDSButton p7bt3 = NexDSButton(8, 8, "bt3");         // 30dB button
NexDSButton p7bt4 = NexDSButton(8, 11, "bt4");        // fixed button
NexDSButton p7bt5 = NexDSButton(8, 12, "bt5");        // long button
NexDSButton p7bt6 = NexDSButton(8, 13, "bt6");        // slow button
NexDSButton p7bt7 = NexDSButton(8, 14, "bt7");        // med button
NexDSButton p7bt8 = NexDSButton(8, 15, "bt8");        // fast button
NexVariable p7vaagc = NexVariable(8, 16, "vaagc");    // agc state variable
NexVariable p7vaatten = NexVariable(8, 17, "vaatten");  // atten state variable
NexSlider p7h0 = NexSlider(8, 3, "h0");               // AGC threshold


//
// page 8 "general settings" objects
// note multiple callbacks folded over onto just 2 buttons
//
NexDSButton p8bt0 = NexDSButton(9, 5, "bt0");         // 9600 baud button
NexDSButton p8bt1 = NexDSButton(9, 6, "bt1");         // 9600 baud button
NexDSButton p8bt2 = NexDSButton(9, 7, "bt2");         // 9600 baud button
NexDSButton p8bt4 = NexDSButton(9, 8, "bt4");         // "click" button
NexDSButton p8bt5 = NexDSButton(9, 9, "bt5");         // "click" button
NexDSButton p8bt3 = NexDSButton(9, 13, "bt3");        // "bottom" button
NexDSButton p8bt6 = NexDSButton(9, 14, "bt6");        // "side" button
NexVariable p8vaenc = NexVariable(9, 11, "vaenc");    // encoder state variable
NexVariable p8vabaud = NexVariable(9, 10, "vabaud");  // baud rate variable


//
// page 9 "configure" objects
//
NexText p9t6 = NexText(10, 16, "t6");                  // indicator/encoder/button number
NexText p9t4 = NexText(10, 9, "t4");                   // function
NexText p9t5 = NexText(10, 12, "t5");                  // 2nd function 
NexButton p9b1 = NexButton(10, 5, "b1");               // device number minus
NexButton p9b2 = NexButton(10, 6, "b2");               // device number plus
NexButton p9b3 = NexButton(10, 7, "b3");               // function minus
NexButton p9b4 = NexButton(10, 8, "b4");               // function plus
NexButton p9b5 = NexButton(10, 10, "b5");              // 2nd function minus
NexButton p9b6 = NexButton(10, 11, "b6");              // 2nd function plus
NexButton p9b7 = NexButton(10, 14, "b7");              // Accept/Set

//
// declare touch event objects to the touch event list
// this tells the code what touch events too look for
//
NexTouch *nex_listen_list[] = 
{
  &page0main,                                 // page change 
  &page1,                                     // page change
  &page2,                                     // page change
  &page3,                                     // page change
  &page4,                                     // page change
  &page5,                                     // page change
  &page6,                                     // page change
  &page7,                                     // page change
  &page8,                                     // page change
  &page9,                                     // page change
  &page0,                                    // page change
  &p1bt0,                                     // LED 1 button
  &p1bt1,                                     // LED 2 button
  &p1bt2,                                     // LED 3 button
  &p1bt3,                                     // LED 4 button
  &p1bt4,                                     // LED 5 button
  &p1bt5,                                     // LED 6 button
  &p1bt6,                                     // LED 7 button
  &p2b1,                                      // encoder configure button
  &p2b2,                                      // pushbutton configure button
  &p2b3,                                      // indicator configure button
  &p2b5,                                      // save settings button
  &p3b1,                                      // set frequency button
  &p4bt0,                                     // 160M button (all callbacks fold to this)
  &p5bt0,                                     // LSB button (all callbacks fold to this)
  &p6bt2,                                     // NR (3 buttons fold to this)
  &p6bt5,                                     // NB (3 buttons)
  &p6bt0,                                     // SNB
  &p6bt1,                                     // ANF
  &p7bt0,                                     // Atten (4 buttons)
  &p7bt4,                                     // AGC (5 buttons fold onto this callback)
  &p7h0,                                      // threshold slider
  &p8bt0,                                     // baud button
  &p8bt3,                                     // encoder string bottom button
  &p8bt4,                                     // click button
  &p8bt6,                                     // encoder string side button
  &p9b1,                                      // configure page #-
  &p9b2,                                      // configure page #+
  &p9b3,                                      // configure page fn-
  &p9b4,                                      // configure page fn+
  &p9b5,                                      // configure page 2fn-
  &p9b6,                                      // configure page 2fn+
  &p9b7,                                      // configure page Set/Accept
  NULL                                        // terminates the list
};


//
// strings for mode
//
char* ModeStrings[] = 
{
  "LSB",
  "USB",
  "DSB",
  "CWL",
  "CWU",
  "FM",
  "AM",
  "DIG U",
  "SPEC",
  "DIG L",
  "SAM",
  "DRM"
};


//
// strings for band
//
char* BandStrings[] = 
{
  "160m",
  "80m",
  "60m",
  "40m",
  "30m",
  "20m",
  "17m",
  "15m",
  "12m",
  "10m",
  "60m",
  "GEN"
};


//
// strings for NR
//
char* NRStrings[] =
{
  "NR off",
  "NR 1",
  "NR 2" 
};

//
// strings for NB
//
char* NBStrings[] =
{
  "NB off",
  "NB 1",
  "NB 2" 
};


//
// strings for atten
//
char* AttenStrings[] =
{
  "0 dB",
  "10 dB",
  "20 dB",
  "30 dB" 
};


//
// strings for AGC
//
char* AGCStrings[] =
{
  "Fixed",
  "Long",
  "Slow",
  "Medium", 
  "Fast" 
};





//
// strings for editing control settings
// the string count must match the number of enumerations!
// 25 chars max!
// 123456789012345678901234*
//
char* EncoderActionStrings[] = 
{
  "No action",
  "AF Gain",
  "Master AF Gain",
  "AGC Level",
  "Filter High Cut",
  "Filter Low Cut",
  "Drive",
  "Mic Gain",
  "VFO A Tune",
  "VFO B Tune",
  "VOX Gain",
  "VOX Delay",
  "CW Sidetone",
  "CW Speed",
  "Squelch level",
  "Multifunction"                      // multifunction
};

char* MultiEncoderActionStrings[] = 
{
  "M:No Action",
  "M:AF Gain",
  "M:Master AF",
  "M:AGC Level",
  "M:Filt High",
  "M:Filt Low",
  "M:Drive",
  "M:Mic Gain",
  "M:VFO A",
  "M:VFO B",
  "M:VOX Gain",
  "M:VOX Delay",
  "M:CW Tone",
  "M:CW Speed",
  "M:Squelch",
  "M:Multi"                      // multifunction
};
  
char* IndicatorActionStrings[] = 
{
  "MOX",
  "Tune",
  "RIT On",
  "Split",
  "Click Tune on",
  "VFO Lock",
  "NB on",
  "NR on",
  "SNB on",
  "ANF on",
  "Squelch on",
  "VFO A/B",
  "None"
};


char* ButtonActionStrings[] = 
{
  "No Function",                            // no assigned function
  "Encoder Click",                    // for dual fn encoders
  "Toggle A/B VFO",
  "Mox",
  "Tune",
  "AF Mute",
  "Filter Reset",
  "Band Up",
  "Band Down",
  "Mode Up",
  "Mode Down",
  "AGC Speed",
  "NB Step",
  "NR Step",
  "SNB",
  "ANF",
  "RIT On/off",
  "RIT Step Up",
  "RIT Step Down",
  "Copy VFO A to VFO B",
  "Copy VFO B to VFO A",
  "swap VFO A & B",
  "Split",
  "Click Tune",
  "VFO Lock",
  "Radio Start / Stop",
  "Squelch on/off",
  "Atten step"
};


//
// list of encoder text box controls for the I/O test page
//
const char * BtnObjectNames[] = 
{
  "tpb8",
  "tpb9",
  "tpb10",
  "tpb11",
  "tpb12",
  "tpb13",
  "tpb14",
  "tpb15",
  "tpb16",
  "tpb17",
  "tpb18",
  "tpb19",
  "tpb20",
  "tpb21",
  "tpb22",
  "tpb23",
  "tenc2",
  "tenc3",
  "tenc4",
  "tenc5",
  "tenc6",
  "tenc7",
  "tenc8"
};


//
// "helper" function to find the max number of controls of the type being edited
// return one less than the number
// (ie for encoders return 6 because we are allowed 0..6)
//
unsigned int Page9GetMaxControlCount(void)
{
  unsigned int Result;
  switch(GControlType)                        // now find the initial actions
  {
    case eEncoders:
      Result = VMAXENCODERS - 1;
      break;
    case ePushbuttons:
      Result = VMAXBUTTONS - 1;
      break;
    case eIndicators:
      Result = VMAXINDICATORS - 1;
      break;
  }
  return Result;  
}


//
// "helper" function to find the max number of actions of the type being edited
// return one less than the number
//
unsigned int Page9GetMaxActionCount(void)
{
  unsigned int Result;
  switch(GControlType)                        // now find the initial actions
  {
    case eEncoders:
      Result = VNUMENCODERACTIONS - 1;
      break;
    case ePushbuttons:
      Result = VNUMBUTTONACTIONS - 1;
      break;
    case eIndicators:
      Result = VNUMINDICATORACTIONS - 1;
      break;
  }
  return Result;  
}


//
// a "helper" function to get the current programmed actions for the configure page
// this retrieves the settings currently in Flash EPROM
//
void Page9GetActions(void)
{
  switch(GControlType)                        // now find the initial actions
  {
    case eEncoders:
      GActionNumber = (unsigned int) GetEncoderAction(GControlNumber, false);      // get current programmed main & 2nd actions
      G2ndActionNumber = (unsigned int) GetEncoderAction(GControlNumber, true);
      break;
    case ePushbuttons:
      GActionNumber = (unsigned int) GetButtonAction(GControlNumber);
      break;
    case eIndicators:
      GActionNumber = (unsigned int) GetIndicatorAction(GControlNumber);
      break;
  }
}


//
// I/O test LED strings
//
char* IOTestLEDStrings[]=
{
  "LED1",
  "LED2",
  "LED3",
  "LED4",
  "LED5",
  "LED6",
  "LED7"
};


//
// I/O test encoder strings
//
char* IOTestEncoderStrings[]=
{
  "Enc 2A",
  "Enc 2B",
  "Enc 3A",
  "Enc 3B",
  "Enc 4A",
  "Enc 4B",
  "Enc 5"
};


//
// I/O test pushbutton strings
//
char* IOTestButtonStrings[] =
{
  "SW1",
  "SW2",
  "SW3",
  "SW4",
  "SW5",
  "SW6",
  "SW7",
  "SW8",
  "SW9",
  "SW10",
  "SW11",
  "SW12",
  "SW13",
  "SW14",
  "SW15",
  "SW16",
  "SW_E2",
  "SW17",
  "SW_E3",
  "SW18",
  "SW_E4",
  "SW19",
  "SW_E5"
};



//
// a "helper" function to set the controls in the configure page
//
void Page9SetControls(void)
{
  switch(GControlType)                        // now find the initial action and display it
  {
    case eEncoders:
      p9t6.setText(IOTestEncoderStrings[GControlNumber]);              // show what control were editing
      p9t4.setText(EncoderActionStrings[GActionNumber]);             // show in text boxes
      p9t5.setText(EncoderActionStrings[G2ndActionNumber]);
      break;
    case ePushbuttons:
      p9t6.setText(IOTestButtonStrings[GControlNumber]);              // show what control were editing
      p9t4.setText(ButtonActionStrings[GActionNumber]);             // show in text boxes
      break;
    case eIndicators:
      p9t6.setText(IOTestLEDStrings[GControlNumber]);              // show what control were editing
      p9t4.setText(IndicatorActionStrings[GActionNumber]);             // show in text boxes
      break;
  }
}



//
// helper to redraw encoder strings
//
void RedrawEncoderString1(void)
{
  if ( GDisplayPage == eFrontPage)
  {
    if (Enc1IsMulti)
      p0t5.setText(MultiEncoderActionStrings[(unsigned int)DisplayCurrentAction1]);
    else
      p0t5.setText(EncoderActionStrings[(unsigned int)DisplayCurrentAction1]);
  }
}

void RedrawEncoderString2(void)
{
  if ( GDisplayPage == eFrontPage)
  {
    if (Enc2IsMulti)
      p0t6.setText(MultiEncoderActionStrings[(unsigned int)DisplayCurrentAction2]);
    else
      p0t6.setText(EncoderActionStrings[(unsigned int)DisplayCurrentAction2]);
  }
}

void RedrawEncoderString3(void)
{
  if ( GDisplayPage == eFrontPage)
  {
    if (Enc3IsMulti)
      p0t7.setText(MultiEncoderActionStrings[(unsigned int)DisplayCurrentAction3]);
    else
      p0t7.setText(EncoderActionStrings[(unsigned int)DisplayCurrentAction3]);
  }
}

void RedrawEncoderString4(void)
{
  if ( GDisplayPage == eFrontPage)
  {
    if (Enc4IsMulti)
      p0t4.setText(MultiEncoderActionStrings[(unsigned int)DisplayCurrentAction4]);
    else
      p0t4.setText(EncoderActionStrings[(unsigned int)DisplayCurrentAction4]);
  }
}


//
// set meter background
//
void DisplaySetMeterBackground(void)
{
  char Cmd1Buffer[20];
  char Cmd2Buffer[20];

  if(GDisplayPage == eFrontPage)                // redraw main page, if displayed
  {
    if (DisplayTXState && DisplayTuneState)
    {
      strcpy(Cmd2Buffer, "z0.picc=3");
    }
    else if (DisplayTXState && !DisplayTuneState)
    {
      strcpy(Cmd2Buffer, "z0.picc=2");
    }
    else
    {
      strcpy(Cmd2Buffer, "z0.picc=1");
    }
    sendCommand(Cmd1Buffer);
    sendCommand(Cmd2Buffer);
  }
}



////////////////////////////////////////////////////////////////////////////////////////////////////
//
// touch event handlers: PAGE change
//

//
// page 0 - update all controls to current variables
// this is a "postinitialise" callback - preinitialise didn't seem to be invoked from a page0.show() call
//
void page0mainPushCallback(void *ptr)             // called when page 0 loads (main page)
{
  
  GDisplayPage = eFrontPage;
  if (DisplayABState)
    p0t0.setText("A");  
  else
    p0t0.setText("B");  
  
  p0t2.setText(DisplayCurrentFrequency);
  p0t8.setText(ModeStrings[DisplayCurrentMode]);

  if (DisplayCurrentLockState)
    p0t9.setText("LOCK");  
  else
    p0t9.setText("");

  if (DisplayRITState)
    p0t10.setText("ON");  
  else
    p0t10.setText("");

  if (DisplayCurrentSplitState)
    p0t11.setText("SPLIT");  
  else
    p0t11.setText("");

  p0z0.setValue(DisplayCurrentSReading);

  if (GBottomEncoderStrings)
  {
    RedrawEncoderString1();
    RedrawEncoderString2();
    RedrawEncoderString3();
  }
  if (GSideEncoderStrings)
    RedrawEncoderString4();

  DisplaySetMeterBackground();    
}

void page1PushCallback(void *ptr)             // called when page 1 loads (I/O test page)
{
  GDisplayPage = eIOTestPage;
  ClearLEDs();
}

void page2PushCallback(void *ptr)             // called when page 2 loads (about page)
{
  GDisplayPage = eAboutPage;
}

void page3PushCallback(void *ptr)             // called when page 3 loads (frequency entry page)
{
  GDisplayPage = eFreqEntryPage;
}



void page4PushCallback(void *ptr)             // called when page 4 loads (band page)
{
  GDisplayPage = eBandPage;                   // set the new page number
  CATRequestBand();
}



void page5PushCallback(void *ptr)             // called when page 5 loads (mode page)
{
  GDisplayPage = eModePage;
  switch(DisplayCurrentMode)                         // now set 1 button to current mode
  {
    case eLSB: p5bt0.setValue(1); break;      // set button
    case eUSB: p5bt1.setValue(1); break;      // set button
    case eDSB: p5bt2.setValue(1); break;      // set button
    case eCWL: p5bt3.setValue(1); break;      // set button
    case eCWU: p5bt4.setValue(1); break;      // set button
    case eFM: p5bt5.setValue(1); break;       // set button
    case eAM: p5bt6.setValue(1); break;       // set button
    case eDIGU: p5bt10.setValue(1); break;    // set button
    case eSPEC: p5bt8.setValue(1); break;     // set button
    case eDIGL: p5bt9.setValue(1); break;     // set button
    case eSAM: p5bt7.setValue(1); break;      // set button
    case eDRM: p5bt11.setValue(1); break;     // set button
  }
  p5vamode.setValue((int)DisplayCurrentMode);              // finally set the "which button is pressed" variable

}

void page6PushCallback(void *ptr)             // called when page 6 loads (noise page)
{
  GDisplayPage = eNRPage;

  p6bt0.setValue((unsigned int)DisplayCurrentSNBState);     // set SNB initial state
  p6bt1.setValue((unsigned int)DisplayCurrentANFState);     // ANF initial state
  switch(DisplayCurrentNRState)                             // NR initial state
  {
    case 0: p6bt2.setValue(1); break;
    case 1: p6bt3.setValue(1); break;
    case 2: p6bt4.setValue(1); break;
  }
  p6vanr.setValue(DisplayCurrentNRState);                   // set the "which button pressed" variable

  switch(DisplayCurrentNBState)                             // NB initial state
  {
    case 0: p6bt5.setValue(1); break;
    case 1: p6bt6.setValue(1); break;
    case 2: p6bt7.setValue(1); break;
  }
  p6vanb.setValue(DisplayCurrentNBState);                   // set the "which button pressed" variable
}

void page7PushCallback(void *ptr)             // called when page 7 loads (RF page)
{
  GDisplayPage = eRFPage;

  switch(DisplayCurrentAtten)                          // attenuation initial state
  {
    case 0: p7bt0.setValue(1); break;
    case 1: p7bt1.setValue(1); break;
    case 2: p7bt2.setValue(1); break;
    case 3: p7bt3.setValue(1); break;
  }
  p7vaatten.setValue((int)DisplayCurrentAtten);                  // set the "which button pressed" variable

  switch(DisplayCurrentAGCSpeed)                            // AGC speed initial state
  {
    case 0: p7bt4.setValue(1); break;
    case 1: p7bt5.setValue(1); break;
    case 2: p7bt6.setValue(1); break;
    case 3: p7bt7.setValue(1); break;
    case 4: p7bt8.setValue(1); break;
  }
  p7vaagc.setValue((int)DisplayCurrentAGCSpeed);                // set the "which button pressed" variable
  CATRequestAGCThreshold();

//  p7h0.setValue(DisplayCurrentAGCThreshold);               // set slider
}

//
// when page 8 loads, set the initial state of its buttons
//
void page8PushCallback(void *ptr)             // called when page 8 loads
{
  GDisplayPage = eSettingsPage;

  switch(GUSBBaudRate)
  {
    case eBaud9600: p8bt0.setValue(1); break;
    case eBaud38400: p8bt1.setValue(1); break;
    case eBaud115200: p8bt2.setValue(1); break;
  }
  
  switch(GEncoderOperation)
  {
    case eDualFnClick: p8bt4.setValue(1); break;
    case eDualFnPress: p8bt5.setValue(1); break;
  }
//
// set the bottom and side encoder legend controls
//
  p8bt3.setValue((uint32_t)GBottomEncoderStrings);
  p8bt6.setValue((uint32_t)GSideEncoderStrings);
}

//
// when page 9 is first displayed: we are on control 0.
// set the control number and show its initial function.
//
void page9PushCallback(void *ptr)             // called when page 9 loads
{
  GDisplayPage = eConfigurePage;
  Page9GetActions();                            // get the current assigned actions
  Page9SetControls();                           // update the text boxes
}



//
// when page 10 is first displayed: a serial connection has been established
// move to page 0.
//
void page0PushCallback(void *ptr)             // called when page 0 loads (splash page)
{
  GDisplayPage = eSplashPage;
  page0main.show();
}



////////////////////////////////////////////////////////////////////////////////////////////////////
//
// touch event handlers: Page 1
//
void p1bt0PushCallback(void *ptr)             // I/O test LED1
{
  uint32_t LEDState;
  p1bt0.getValue(&LEDState);                // read the button state
  SetLED(0, (LEDState != 0));
}

void p1bt1PushCallback(void *ptr)             // I/O test LED2
{
  uint32_t LEDState;
  p1bt1.getValue(&LEDState);                // read the button state
  SetLED(1, (LEDState != 0));
}

void p1bt2PushCallback(void *ptr)             // I/O test LED3
{
  uint32_t LEDState;
  p1bt2.getValue(&LEDState);                // read the button state
  SetLED(2, (LEDState != 0));
}

void p1bt3PushCallback(void *ptr)             // I/O test LED4
{
  uint32_t LEDState;
  p1bt3.getValue(&LEDState);                // read the button state
  SetLED(3, (LEDState != 0));
}

void p1bt4PushCallback(void *ptr)             // I/O test LED5
{
  uint32_t LEDState;
  p1bt4.getValue(&LEDState);                // read the button state
  SetLED(4, (LEDState != 0));
}

void p1bt5PushCallback(void *ptr)             // I/O test LED6
{
  uint32_t LEDState;
  p1bt5.getValue(&LEDState);                // read the button state
  SetLED(5, (LEDState != 0));
}

void p1bt6PushCallback(void *ptr)             // I/O test LED7
{
  uint32_t LEDState;
  p1bt6.getValue(&LEDState);                // read the button state
  SetLED(6, (LEDState != 0));
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
// touch event handlers: Page 2
//
void p2b1PushCallback(void *ptr)             // encoder configure button
{
  GControlType = eEncoders;                   // type of control being edited = encoders
  GControlNumber = 0;                         // control being edited = first encoder
}

void p2b2PushCallback(void *ptr)              // pushbutton configure button
{
  GControlType = ePushbuttons;                // type of control being edited = pushbuttons
  GControlNumber = 0;                         // control being edited = first pushbutton
}

void p2b3PushCallback(void *ptr)             // indicator configure button
{
  GControlType = eIndicators;                 // type of control being edited = indicators
  GControlNumber = 0;                         // control being edited = first indicator
}

void p2b5PushCallback(void *ptr)             // "save settings" button
{
  CopySettingsToFlash();
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
// touch event handlers: Page 3 (freq entry)
//
void p3b1PushCallback(void *ptr)              // set frequency button
{
  char NewFreq[20];
  memset(NewFreq, 0, sizeof(NewFreq));
  p3t2.getText(NewFreq, sizeof(NewFreq));
  CATSetFrequency(NewFreq); 
}



////////////////////////////////////////////////////////////////////////////////////////////////////
//
// touch event handlers: Page 4 (band)
//
void p4bt0PushCallback(void *ptr)             // all band buttons
{
  uint32_t BandValue;
  p4vaband.getValue(&BandValue);
  CATSetBand((EBand)BandValue);
}



////////////////////////////////////////////////////////////////////////////////////////////////////
//
// touch event handlers: Page 5 (mode)
//
void p5bt0PushCallback(void *ptr)             // all mode buttons
{
  uint32_t ModeValue;
  p5vamode.getValue(&ModeValue);
  CATSetMode((EMode)ModeValue);
  DisplayCurrentMode = (EMode)ModeValue;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
//
// touch event handlers: Page 6 (Noise)
//
void p6bt0PushCallback(void *ptr)             // SNB button
{
  uint32_t SNBValue;
  p6bt0.getValue(&SNBValue);
  CATSetSNBState((bool)SNBValue);
  DisplayCurrentSNBState = (bool)SNBValue;
}

void p6bt1PushCallback(void *ptr)             // ANF button
{
  uint32_t ANFValue;
  p6bt1.getValue(&ANFValue);
  CATSetANFState((bool)ANFValue);
  DisplayCurrentANFState = (bool)ANFValue;
}


void p6bt2PushCallback(void *ptr)             // NR button
{
  uint32_t NRValue;
  p6vanr.getValue(&NRValue);
  CATSetNRState((ENRState)NRValue);
  DisplayCurrentNRState = (ENRState)NRValue;
}


void p6bt5PushCallback(void *ptr)             // NB button
{
  uint32_t NBValue;
  p6vanb.getValue(&NBValue);
  CATSetNBState((ENBState)NBValue);
  DisplayCurrentNBState = (ENBState)NBValue;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
//
// touch event handlers: Page 7 (RF)
//
void p7bt0PushCallback(void *ptr)             // atten button
{
  uint32_t AttenValue;
  p7vaatten.getValue(&AttenValue);
  CATSetAttenuation((EAtten)AttenValue);
  DisplayCurrentAtten = (EAtten)AttenValue;
}


void p7bt4PushCallback(void *ptr)             // AGC button
{
  uint32_t AGCValue;
  p7vaagc.getValue(&AGCValue);
  CATSetAGCSpeed((EAGCSpeed)AGCValue);
  DisplayCurrentAGCSpeed = (EAGCSpeed)AGCValue;
}



//
// there is a 20 unit offset between CAT and slider
// CAT range -20 to +120; slider range 0 to 140
//
void p7h0PopCallback(void *ptr)              // AGC slider
{
  uint32_t AGCValue;
  p7h0.getValue(&AGCValue);
  CATSetAGCThreshold(AGCValue-20);
}



////////////////////////////////////////////////////////////////////////////////////////////////////
//
// touch event handlers: Page 8
//
// copy back enumerated variables from the display.
//
void p8bt0PushCallback(void *ptr)             // all baud buttons
{
  uint32_t BaudValue;
  p8vabaud.getValue(&BaudValue);
  GUSBBaudRate = (EBaudRates)BaudValue;
}

void p8bt4PushCallback(void *ptr)             // all encoder buttons
{
  uint32_t EncoderValue;
  p8vaenc.getValue(&EncoderValue);
  GEncoderOperation = (EDualFnEncoders)EncoderValue;
}

void p8bt3PushCallback(void *ptr)             // bottom encoder string button
{
  uint32_t BottomStringValue;
  p8bt3.getValue(&BottomStringValue);
  GBottomEncoderStrings = (bool) BottomStringValue;
}

void p8bt6PushCallback(void *ptr)             // side encoder string button
{
  uint32_t SideStringValue;
  p8bt6.getValue(&SideStringValue);
  GSideEncoderStrings = (bool) SideStringValue;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
//
// touch event handlers: Page 9


//
// Control -:
// decrement encoder/button/LED number, and repopulate the number
// then find the current assigned action & display
//
void p9b1PushCallback(void *ptr)             // device number -
{
  if (GControlNumber > 0)
    GControlNumber--;
  Page9GetActions();                            // get the current assigned actions
  Page9SetControls();                           // update the text boxes
}

//
// Control +:
// increment encoder/button/LED number, and repopulate the number
// then find the current assigned action & display
//
void p9b2PushCallback(void *ptr)             // device number +
{
  unsigned int MaxControlNum;
  MaxControlNum = Page9GetMaxControlCount();          // find out the max number we are allowed
  if (GControlNumber < MaxControlNum)
    GControlNumber++;
  Page9GetActions();                            // get the current assigned actions
  Page9SetControls();                           // update the text boxes
}

//
// Action -:
// decrement action number & redisplay
//
void p9b3PushCallback(void *ptr)             // function -
{
  if (GActionNumber > 0)
    GActionNumber--;
  Page9SetControls();                           // update the text boxes
}

//
// Action +:
// increment action number & redisplay
//
void p9b4PushCallback(void *ptr)             // function +
{
  unsigned int MaxActionNum;
  MaxActionNum = Page9GetMaxActionCount();          // find out the max number we are allowed
  if (GActionNumber < MaxActionNum)
    GActionNumber++;
  Page9SetControls();                           // update the text boxes
}

//
// 2nd Action -:
// decrement action number & redisplay
//
void p9b5PushCallback(void *ptr)             // 2nd function -
{
  if (G2ndActionNumber > 0)
    G2ndActionNumber--;
  Page9SetControls();                           // update the text boxes
}

//
// 2nd Action +:
// increment action number & redisplay
//
void p9b6PushCallback(void *ptr)             // 2nd function +
{
  unsigned int MaxActionNum;
  MaxActionNum = Page9GetMaxActionCount();          // find out the max number we are allowed
  if (G2ndActionNumber < MaxActionNum)
    G2ndActionNumber++;
  Page9SetControls();                           // update the text boxes
}

//
// save the currently displayed setting back to the stored configuration data
// this can then be written to Flash.
//
void p9b7PushCallback(void *ptr)             // Set/Accept
{
  switch(GControlType)                        // now find the initial actions
  {
    case eEncoders:
      SetEncoderAction(GControlNumber, (EEncoderActions)GActionNumber, false);      // set current programmed main & 2nd actions
      SetEncoderAction(GControlNumber, (EEncoderActions)G2ndActionNumber, true);
      break;
    case ePushbuttons:
      SetButtonAction(GControlNumber, (EButtonActions)GActionNumber);
      break;
    case eIndicators:
      SetIndicatorAction(GControlNumber, (EIndicatorActions)GActionNumber);
      break;
  }

}
void SetEncoderAction(unsigned int Encoder, EEncoderActions Setting, bool Is2ndFunction);
void SetIndicatorAction(unsigned int Indicator, EIndicatorActions Setting);
void SetButtonAction(unsigned int Button, EButtonActions Setting);




//
// display initialise
//
void DisplayInit(void)
{
//
// set baud rate & register event callback functions
//  
  nexInit(115200);
//  Serial1.begin(115200);                              // baud rate for Nextion display
  page0main.attachPush(page0mainPushCallback);
  page1.attachPush(page1PushCallback);
  page2.attachPush(page2PushCallback);
  page3.attachPush(page3PushCallback);
  page4.attachPush(page4PushCallback);
  page5.attachPush(page5PushCallback);
  page6.attachPush(page6PushCallback);
  page7.attachPush(page7PushCallback);
  page8.attachPush(page8PushCallback);
  page9.attachPush(page9PushCallback);
  page0.attachPush(page0PushCallback);
  p1bt0.attachPush(p1bt0PushCallback);
  p1bt1.attachPush(p1bt1PushCallback);
  p1bt2.attachPush(p1bt2PushCallback);
  p1bt3.attachPush(p1bt3PushCallback);
  p1bt4.attachPush(p1bt4PushCallback);
  p1bt5.attachPush(p1bt5PushCallback);
  p1bt6.attachPush(p1bt6PushCallback);
  p2b1.attachPush(p2b1PushCallback);
  p2b2.attachPush(p2b2PushCallback);
  p2b3.attachPush(p2b3PushCallback);
  p2b5.attachPush(p2b5PushCallback);
  p3b1.attachPush(p3b1PushCallback);
  p4bt0.attachPush(p4bt0PushCallback);
  p5bt0.attachPush(p5bt0PushCallback);
  p6bt0.attachPush(p6bt0PushCallback);
  p6bt1.attachPush(p6bt1PushCallback);
  p6bt2.attachPush(p6bt2PushCallback);
  p6bt5.attachPush(p6bt5PushCallback);
  p7bt0.attachPush(p7bt0PushCallback);
  p7bt4.attachPush(p7bt4PushCallback);
  p7h0.attachPop(p7h0PopCallback);
  p8bt0.attachPush(p8bt0PushCallback);
  p8bt4.attachPush(p8bt4PushCallback);
  p8bt3.attachPush(p8bt3PushCallback);
  p8bt6.attachPush(p8bt6PushCallback);
  p9b1.attachPush(p9b1PushCallback);
  p9b2.attachPush(p9b2PushCallback);
  p9b3.attachPush(p9b3PushCallback);
  p9b4.attachPush(p9b4PushCallback);
  p9b5.attachPush(p9b5PushCallback);
  p9b6.attachPush(p9b6PushCallback);
  p9b7.attachPush(p9b7PushCallback);
//
// get the initial encoder actions
//
  DisplayCurrentAction1 = GetEncoderAction(0, false);    
  if (DisplayCurrentAction1 == eENMulti)
  {
    Enc1IsMulti = true;
    DisplayCurrentAction1 = GMultiAction;
  }
  
  DisplayCurrentAction2 = GetEncoderAction(2, false);    
  if (DisplayCurrentAction2 == eENMulti)
  {
    Enc2IsMulti = true;
    DisplayCurrentAction2 = GMultiAction;
  }
  
  DisplayCurrentAction3 = GetEncoderAction(4, false);    
  if (DisplayCurrentAction3 == eENMulti)
  {
    Enc3IsMulti = true;
    DisplayCurrentAction3 = GMultiAction;
  }
  
  DisplayCurrentAction4 = GetEncoderAction(6, false);
  if (DisplayCurrentAction4 == eENMulti)
  {
    Enc4IsMulti = true;
    DisplayCurrentAction4 = GMultiAction;
  }


//
// tell the display to move to the main page from the splash page
// that will lead to a callback when we redraw the display.
//
  page0main.show();
}



//
// display tick (currently 10ms)
//
void DisplayTick(void)
{
//
// handle touch display events
//  
  nexLoop(nex_listen_list);
}


#define TXTBUFSIZE 10
#define CMDBUFSIZE 20

//
// display encoder handler
// encoder number 0-6 (normal) 7 (VFO)
//
void DisplayEncoderHandler(unsigned int Encoder, int Count)
{
  char TxtBuffer[TXTBUFSIZE];                                   // text representation of count vlaue
  if (GDisplayPage == eIOTestPage)
  {
     memset(TxtBuffer, 0, TXTBUFSIZE);
     sprintf(TxtBuffer, "%d", Count);
     switch(Encoder)
     {
       case 0: p1tEnc2.setText(TxtBuffer); break;
       case 1: p1tEnc3.setText(TxtBuffer); break;
       case 2: p1tEnc4.setText(TxtBuffer); break;
       case 3: p1tEnc5.setText(TxtBuffer); break;
       case 4: p1tEnc6.setText(TxtBuffer); break;
       case 5: p1tEnc7.setText(TxtBuffer); break;
       case 6: p1tEnc8.setText(TxtBuffer); break;
       case 7: p1tEnc1.setText(TxtBuffer); break;
     }
  }
}



//
// display button handler, 
// button = 0-22
// when in I/O test page, light up a text box by changing its background colour
// 
//
void DisplayButtonHandler(unsigned int Button, bool IsPressed)
{
  char Cmd1Buffer[CMDBUFSIZE];
  char Cmd2Buffer[CMDBUFSIZE];
  if (GDisplayPage == eIOTestPage)
  {
//
// set background colour to say pressed or not.
// the Nextion library doesn't support this directly  but the basic mechanism is to make two calls:
// sendCommand("tenc1.bco=GREEN"); 
// sendCommand("ref tenc1");
//
    strcpy(Cmd1Buffer, BtnObjectNames[Button]);
    if (IsPressed)
      strcat(Cmd1Buffer, ".bco=GREEN");
    else
      strcat(Cmd1Buffer, ".bco=WHITE");

    strcpy (Cmd2Buffer, "ref ");
    strcat (Cmd2Buffer, BtnObjectNames[Button]);

    sendCommand(Cmd1Buffer);
    sendCommand(Cmd2Buffer);
  }
}



/////////////////////////////////////////////////////////////////////////////////////
//
// handlers for CAT messages to save data for display
//
void DisplayShowABState(bool IsA)
{
  if (DisplayABState != IsA)                      // if different from current settings
  {
    DisplayABState = IsA;                         // save new state
    if(GDisplayPage == eFrontPage)                // redraw main page, if displayed
    {
      if (DisplayABState)
        p0t0.setText("A");  
      else
        p0t0.setText("B");  
    }
  }
}


//
// need to change the background image
//
void DisplayShowTXState(bool IsTX, bool IsTune)
{
  if (DisplayTXState != IsTX)                      // if different from current settings
  {
    DisplayTXState = IsTX;                                // save new settings
    DisplayTuneState = IsTune;                            // save new settings
    DisplaySetMeterBackground();
  }
}


void DisplayShowRITState(bool IsRIT)
{
  if (DisplayRITState != IsRIT)                // if different from current settings
  {
    if(GDisplayPage == eFrontPage)                      // redraw main page, if displayed
    {
      if (IsRIT)
        p0t10.setText("ON");
      else
        p0t10.setText("");
    }
  }
  DisplayRITState = IsRIT;                     // save new settings
}



void DisplayShowLockState(bool IsLock)
{
  if (DisplayCurrentLockState != IsLock)                // if different from current settings
  {
    if(GDisplayPage == eFrontPage)                      // redraw main page, if displayed
    {
      if (IsLock)
        p0t9.setText("LOCK");  
      else
        p0t9.setText("");
    }
  }
  DisplayCurrentLockState = IsLock;                     // save new settings
}


void DisplayShowSplit(bool IsSplit)
{
  if (DisplayCurrentSplitState != IsSplit)              // if different from current settings
  {
    if(GDisplayPage == eFrontPage)                      // redraw main page, if displayed
    {
      if (IsSplit)
        p0t11.setText("SPLIT");  
      else
        p0t11.setText("");
    }
  }
  DisplayCurrentSplitState = IsSplit;                   // save new settings
}


//
// there is a unit conversion in here
// parameter passed = number from CAT (-140dBm + 0.5N, covering range -140 to -10dBm)
// stored value = gauge angle, 34 to 146 degrees
// to convert: angle = 34 + 0.8N
//
void DisplayShowSMeter(unsigned int Reading)
{
  unsigned int Angle;                                   // new pointer angle
  Angle = (Reading << 1);
  Angle = (Angle / 5)+34;                               // 4N/5+34
  if (DisplayCurrentSReading != Angle)                  // if different from current settings
    if(GDisplayPage == eFrontPage)                      // redraw main page, if displayed
      if (DisplayTXState == false)                      // if RX
        p0z0.setValue(Angle);
  DisplayCurrentSReading = Angle;                       // save new settings
}


//
// there is a unit conversion in here ONLY APPROXIMATED
// parameter passed = number from CAT (Watts)
// stored value = gauge angle, 34 to 146 degrees
// crude bodge - to convert: angle = 34+1.1n
//
void DisplayShowTXPower(unsigned int Reading)
{
  unsigned int Angle;                                   // new pointer angle
  Angle = (Reading + Reading/10) + 34;
  if (DisplayCurrentPowerReading != Angle)              // if different from current settings
    if(GDisplayPage == eFrontPage)                      // redraw main page, if displayed
      if (DisplayTXState)                               // if TX
        p0z0.setValue(Angle);
  DisplayCurrentPowerReading = Angle;                   // save new settings
}


//
// display set paramters that are VFO A/B or RX 1/2 dependent.
// the algorithm is more complex because whether the data is displayed 
// depends on the current A/B setting.
//
void DisplayShowFrequency(char* Frequency)         // string with MHz as ASCII
{
  if (strcmp(DisplayCurrentFrequency, Frequency) != 0)
  {
    strcpy(DisplayCurrentFrequency, Frequency);          // copy new to A
    if(GDisplayPage == eFrontPage)                      // redraw main page, if displayed
      p0t2.setText(Frequency);
  }
}


//
// set current band
// if correct window, clear the current button band then set the new one
//
void DisplayShowBand(EBand Band)
{
  uint32_t BandValue;
  
  if (GDisplayPage == eBandPage)
  {
    p4vaband.getValue(&BandValue);                  // find clicked button (if any)
    switch(BandValue)                               // clear clicked button
    {
      case 0: p4bt0.setValue(0); break;       // set button
      case 1: p4bt1.setValue(0); break;       // set button
      case 2: p4bt2.setValue(0); break;       // set button
      case 3: p4bt3.setValue(0); break;       // set button
      case 4: p4bt4.setValue(0); break;       // set button
      case 5: p4bt5.setValue(0); break;       // set button
      case 6: p4bt6.setValue(0); break;       // set button
      case 7: p4bt7.setValue(0); break;       // set button
      case 8: p4bt8.setValue(0); break;       // set button
      case 9: p4bt9.setValue(0); break;       // set button
      case 10: p4bt10.setValue(0); break;     // set button
      case 11: p4bt11.setValue(0); break;     // set button
    }

    switch(Band)                         // now set 1 button to current band
    {
      case e160: p4bt0.setValue(1); break;      // set button
      case e80: p4bt1.setValue(1); break;       // set button
      case e60: p4bt2.setValue(1); break;       // set button
      case e40: p4bt3.setValue(1); break;       // set button
      case e30: p4bt4.setValue(1); break;       // set button
      case e20: p4bt5.setValue(1); break;       // set button
      case e17: p4bt6.setValue(1); break;       // set button
      case e15: p4bt7.setValue(1); break;       // set button
      case e12: p4bt8.setValue(1); break;       // set button
      case e10: p4bt9.setValue(1); break;       // set button
      case e6: p4bt10.setValue(1); break;       // set button
      case eGen: p4bt11.setValue(1); break;     // set button
    }
    p4vaband.setValue(Band);                    // set button number variable
  }
}


void DisplayShowMode(EMode Mode)
{
  uint32_t ModeValue;

  if (DisplayCurrentMode != Mode)
  {
    DisplayCurrentMode = Mode;                         // copy new to A
    if(GDisplayPage == eFrontPage)                      // redraw main page, if displayed
      p0t8.setText(ModeStrings[DisplayCurrentMode]);

//
// if the mode page is open: clear currently active button then
// set new button and clicked button variable
//
    else if (GDisplayPage = eModePage)
    {
      p5vamode.getValue(&ModeValue);
      switch(ModeValue)                         // first clear the set button
      {
        case 0: p5bt0.setValue(0); break;      // unset button
        case 1: p5bt1.setValue(0); break;      // unset button
        case 2: p5bt2.setValue(0); break;      // unset button
        case 3: p5bt3.setValue(0); break;      // unset button
        case 4: p5bt4.setValue(0); break;      // unset button
        case 5: p5bt5.setValue(0); break;      // unset button
        case 6: p5bt6.setValue(0); break;      // unset button
        case 7: p5bt10.setValue(0); break;     // unset button
        case 8: p5bt8.setValue(0); break;      // unset button
        case 9: p5bt9.setValue(0); break;      // unset button
        case 10: p5bt7.setValue(0); break;     // unset button
        case 11: p5bt11.setValue(0); break;    // unset button
      }
  
      switch(DisplayCurrentMode)                         // now set 1 button to current mode
      {
        case eLSB: p5bt0.setValue(1); break;      // set button
        case eUSB: p5bt1.setValue(1); break;      // set button
        case eDSB: p5bt2.setValue(1); break;      // set button
        case eCWL: p5bt3.setValue(1); break;      // set button
        case eCWU: p5bt4.setValue(1); break;      // set button
        case eFM: p5bt5.setValue(1); break;       // set button
        case eAM: p5bt6.setValue(1); break;       // set button
        case eDIGU: p5bt10.setValue(1); break;    // set button
        case eSPEC: p5bt8.setValue(1); break;     // set button
        case eDIGL: p5bt9.setValue(1); break;     // set button
        case eSAM: p5bt7.setValue(1); break;      // set button
        case eDRM: p5bt11.setValue(1); break;     // set button
      }
      p5vamode.setValue((int)DisplayCurrentMode);              // finally set the "which button is pressed" variable
    }
  }
}



//
// set current NR state
//
void DisplayShowNRState(ENRState State)
{
  uint32_t NRValue;
  if(DisplayCurrentNRState != State)                        // if data different from current
  {
    DisplayCurrentNRState = State;                          // set new data
    if (GDisplayPage == eNRPage)                            // if noise screen showing
    {
      p6vanr.getValue(&NRValue);                                // read current button, then clear it
      switch(NRValue)                                           // set new NR state
      {
        case 0: p6bt2.setValue(0); break;
        case 1: p6bt3.setValue(0); break;
        case 2: p6bt4.setValue(0); break;
      }
      
      
      switch(DisplayCurrentNRState)                             // set new NR state
      {
        case 0: p6bt2.setValue(1); break;
        case 1: p6bt3.setValue(1); break;
        case 2: p6bt4.setValue(1); break;
      }
      p6vanr.setValue(State);                                   // set the "which button pressed" variable
    }
  }
}


//
// set current NB state
//
void DisplayShowNBState(ENBState State)
{
  uint32_t NBValue;
  if(DisplayCurrentNBState != State)                        // if data different from current
  {
    DisplayCurrentNBState = State;                          // set new data
    if (GDisplayPage == eNRPage)                            // if noise screen showing
    {
      p6vanb.getValue(&NBValue);                                // read current button, thne clear it
      switch(NBValue)                                           // set new NB state
      {
        case 0: p6bt5.setValue(0); break;
        case 1: p6bt6.setValue(0); break;
        case 2: p6bt7.setValue(0); break;
      }
    
      DisplayCurrentNBState = State;
      switch(DisplayCurrentNBState)                             // set new NB state
      {
        case 0: p6bt5.setValue(1); break;
        case 1: p6bt6.setValue(1); break;
        case 2: p6bt7.setValue(1); break;
      }
      p6vanb.setValue(State);                                   // set the "which button pressed" variable
    }
  }
}

//
// currently this will NOT update the display if the "noise" page is already open
//
void DisplayShowSNBState(bool SNBState)
{
  DisplayCurrentSNBState = SNBState;
  if (GDisplayPage == eNRPage)                            // if noise screen showing
  {
    p6bt0.setValue((unsigned int)DisplayCurrentSNBState);     // set SNB initial state  
  }
}

//
// currently this will NOT update the display if the "noise" page is already open
//
void DisplayShowANFState(bool ANFState)
{
  DisplayCurrentANFState = ANFState;
    if (GDisplayPage == eNRPage)                            // if noise screen showing
    {
      p6bt1.setValue((unsigned int)DisplayCurrentANFState);     // ANF initial state
    }
}



//
// update AGC speed, and controls if the window is opwn
//
void DisplayShowAGCSpeed(EAGCSpeed Speed)
{
  uint32_t AGCValue;
  if (DisplayCurrentAGCSpeed != Speed)
  {
    DisplayCurrentAGCSpeed = Speed;                             // copy new value
    if(GDisplayPage == eRFPage)                                 // update control if on the page
    {
      p7vaagc.getValue(&AGCValue);                              // read the current pressed button
      switch(AGCValue)                                          // clear the current pressed button
      {
        case 0: p7bt4.setValue(0); break;
        case 1: p7bt5.setValue(0); break;
        case 2: p7bt6.setValue(0); break;
        case 3: p7bt7.setValue(0); break;
        case 4: p7bt8.setValue(0); break;
      }
      // now set new button
      switch(DisplayCurrentAGCSpeed)                            // AGC speed initial state
      {
        case 0: p7bt4.setValue(1); break;
        case 1: p7bt5.setValue(1); break;
        case 2: p7bt6.setValue(1); break;
        case 3: p7bt7.setValue(1); break;
        case 4: p7bt8.setValue(1); break;
      }
      p7vaagc.setValue(DisplayCurrentAGCSpeed);                 // set the "current pressed button"
    }
  }


}


//
// update the display if the "RF" page is already open
// note we don't store the AGC threshold value locally because it isn't routinely polled.
// note the range: CAT value -20 to 120; slide 0 to 140
//
void DisplayShowAGCThreshold(int Threshold)
{
  if(GDisplayPage == eRFPage)                           // update controls if on the screen
    p7h0.setValue(Threshold+20);               // set slider
}


//
// update the local atten variable, and control if the RF page is open
//
void DisplayShowAtten(EAtten Attenuation)
{
  uint32_t AttenValue;

  if(DisplayCurrentAtten != Attenuation)                  // update if changed
  {
    DisplayCurrentAtten = Attenuation;
    if(GDisplayPage == eRFPage)                           // update controls if on the screen
    {
      p7vaatten.getValue(&AttenValue);                    // read the current pressed control
      switch(AttenValue)                                  // clear the current pressed control
      {
        case 0: p7bt0.setValue(0); break;
        case 1: p7bt1.setValue(0); break;
        case 2: p7bt2.setValue(0); break;
        case 3: p7bt3.setValue(0); break;
      }
// now set the new button state
      switch(DisplayCurrentAtten)                          // attenuation initial state
      {
        case 0: p7bt0.setValue(1); break;
        case 1: p7bt1.setValue(1); break;
        case 2: p7bt2.setValue(1); break;
        case 3: p7bt3.setValue(1); break;
      }
      p7vaatten.setValue(DisplayCurrentAtten);             // set the "current pressed control" variable
    }
  }
}





//
// set display of encoder actions. Currently we have placeholders to show 0,2,4 & 6
// this is only called for a multifunction encoder
//
void DisplaySetEncoderAction(unsigned int EncoderNumber, EEncoderActions Action, bool IsMulti)         // set to assigned actions
{
  if (EncoderNumber == 0)                                           // update display position 0
  {
    Enc1IsMulti = IsMulti;
    if (Action != DisplayCurrentAction1)
    {
      if (GDisplayPage == eFrontPage)
      {
        if (IsMulti)
          p0t5.setText(MultiEncoderActionStrings[(unsigned int)Action]);
        else
          p0t5.setText(EncoderActionStrings[(unsigned int)Action]);
      }
      DisplayCurrentAction1 = Action;
    }
  }
  else if (EncoderNumber == 2)                                      // update display position 2
  {
    Enc2IsMulti = IsMulti;
    if (Action != DisplayCurrentAction2)
    {
      if (GDisplayPage == eFrontPage)
      {
        if (IsMulti)
          p0t6.setText(MultiEncoderActionStrings[(unsigned int)Action]);
        else
          p0t6.setText(EncoderActionStrings[(unsigned int)Action]);
      }
      DisplayCurrentAction2 = Action;
    }
  }
  else if (EncoderNumber == 4)                                      // update display position 4
  {
    Enc3IsMulti = IsMulti;
    if (Action != DisplayCurrentAction3)
    {
      if (GDisplayPage == eFrontPage)
      {
        if (IsMulti)
          p0t6.setText(MultiEncoderActionStrings[(unsigned int)Action]);
        else
          p0t6.setText(EncoderActionStrings[(unsigned int)Action]);
      }
      DisplayCurrentAction3 = Action;
    }
  }
  else if (EncoderNumber == 6)                                      // update display position 6
  {
    Enc4IsMulti = IsMulti;
    if (Action != DisplayCurrentAction4)
    {
      if (GDisplayPage == eFrontPage)
      {
        if (IsMulti)
          p0t4.setText(MultiEncoderActionStrings[(unsigned int)Action]);
        else
          p0t4.setText(EncoderActionStrings[(unsigned int)Action]);
      }
      DisplayCurrentAction4 = Action;
    }
  }
}
