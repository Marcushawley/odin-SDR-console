/////////////////////////////////////////////////////////////////////////
//
// Odin SDR Console by Laurence Barker G8NJJ
//
// this sketch provides a knob and switch interface through USB and CAT
//
// encoder.c
// this file holds the code to manage the rotary encoders
// it needs two technologies:
// interrupt driven code for optical VFO encoder (bounce free)
// polled code for very bouncy mechanical encoders for other controls
/////////////////////////////////////////////////////////////////////////

#include <Arduino.h>
#include "globalinclude.h"
#include "mechencoder.h"

#include <Encoder.h>
#include "types.h"
#include "encoders.h"
#include "iopins.h"
#include "display.h"
#include "configdata.h"
#include "cathandler.h"
#include "tiger.h"


//
// note switch and encoder numbering:
// in the software switches are numbered 0-20, and encoders 0-7. The VFO encoder is treated separately.
// these correspond to the control of Kjell's PCB as follows:
//
// encoder numbering:
//    PCB   software
//    VFO    (treated separately)
//    2A      0
//    2B      1
//    3A      2
//    3B      3
//    4A      4
//    4B      5
//    5A      6
//    5B      7


//
// global variables
//
byte EncoderGroup;                                  // sets which ones are updated every ms
EEncoderActions GMultiAction;                       // assigned action for "multifunction"

//
// 8 encoders: one VFO (fast) encoder and 7 "normal" ones 
//
Encoder VFOEncoder(VPINVFOENCODERA, VPINVFOENCODERB);

long old_ct;

struct  EncoderData                           // holds data for one slow encoder
{
  NoClickEncoder* Ptr;                          // ptr to class
  int16_t LastPosition;                       // previous position
};

  EncoderData EncoderList[VMAXENCODERS];

//
// variables to say whether encoders have main action or 2nd action, if dual function
// false if main function, true if 2nd function
//
bool Is2ndAction[VMAXENCODERS];


  
//
// initialise - set up pins & construct data
// these are constructed now because otherwise the configdata settings wouldn't be available yet.
//
void InitEncoders(void)
{
  if (GetEncoderReversed(0))
    EncoderList[0].Ptr = new NoClickEncoder(VPINENCODER1B, VPINENCODER1A, GEncoderDivisor, true);
  else
    EncoderList[0].Ptr = new NoClickEncoder(VPINENCODER1A, VPINENCODER1B, GEncoderDivisor, true);
  
  if (GetEncoderReversed(1))
    EncoderList[1].Ptr = new NoClickEncoder(VPINENCODER2B, VPINENCODER2A, GEncoderDivisor, true);
  else
    EncoderList[1].Ptr = new NoClickEncoder(VPINENCODER2A, VPINENCODER2B, GEncoderDivisor, true);

  if (GetEncoderReversed(2))
    EncoderList[2].Ptr = new NoClickEncoder(VPINENCODER3B, VPINENCODER3A, GEncoderDivisor, true);
  else
    EncoderList[2].Ptr = new NoClickEncoder(VPINENCODER3A, VPINENCODER3B, GEncoderDivisor, true);

  if (GetEncoderReversed(3))
    EncoderList[3].Ptr = new NoClickEncoder(VPINENCODER4B, VPINENCODER4A, GEncoderDivisor, true);
  else
    EncoderList[3].Ptr = new NoClickEncoder(VPINENCODER4A, VPINENCODER4B, GEncoderDivisor, true);

  if (GetEncoderReversed(4))
    EncoderList[4].Ptr = new NoClickEncoder(VPINENCODER5B, VPINENCODER5A, GEncoderDivisor, true);
  else
    EncoderList[4].Ptr = new NoClickEncoder(VPINENCODER5A, VPINENCODER5B, GEncoderDivisor, true);

  if (GetEncoderReversed(5))
    EncoderList[5].Ptr = new NoClickEncoder(VPINENCODER6B, VPINENCODER6A, GEncoderDivisor, true);
  else
    EncoderList[5].Ptr = new NoClickEncoder(VPINENCODER6A, VPINENCODER6B, GEncoderDivisor, true);

  if (GetEncoderReversed(6))
    EncoderList[6].Ptr = new NoClickEncoder(VPINENCODER7B, VPINENCODER7A, GEncoderDivisor, true);
  else
    EncoderList[6].Ptr = new NoClickEncoder(VPINENCODER7A, VPINENCODER7B, GEncoderDivisor, true);

  if (GetEncoderReversed(7))
    EncoderList[7].Ptr = new NoClickEncoder(VPINENCODER8B, VPINENCODER8A, GEncoderDivisor, true);
  else
    EncoderList[7].Ptr = new NoClickEncoder(VPINENCODER8A, VPINENCODER8B, GEncoderDivisor, true);



}




//
// encoder 1ms tick
// these are all now serviced at this rate, with a total time used of around 35 microseconds
// 
void EncoderFastTick(void)
{
  EncoderList[0].Ptr->service();
  EncoderList[1].Ptr->service();
  EncoderList[2].Ptr->service();
  EncoderList[3].Ptr->service();
  EncoderList[4].Ptr->service();
  EncoderList[5].Ptr->service();
  EncoderList[6].Ptr->service();
  EncoderList[7].Ptr->service();
}


//
// encoder 10ms tick
// detect and handle encoders that have moved
// if SENSORDEBUG, just send a display string
// 
void EncoderSlowTick(void)
{
  int16_t Movement;                                         // normal encoder movement since last update
  byte Cntr;                                                // count encoders
  EEncoderActions Action;                                   // assigned fn of the encoder
  
  for (Cntr=0; Cntr < VMAXENCODERS; Cntr++)
  {
    Movement = EncoderList[Cntr].Ptr->getValue();
    if (Movement != 0) 
    {
#ifdef SENSORDEBUG
      Serial.print("encoder ");
      Serial.print(Cntr+1);
      Serial.print(" turned; steps = ");
      if (Movement > 0)
      {
        Serial.print(Movement);
        Serial.println (" steps c/w");
      }
      else
      {
        Serial.print(-Movement);
        Serial.println (" steps ac/w");
      }
#else
      EncoderList[Cntr].LastPosition += Movement;
      switch (GDisplayPage)                                   // display dependent handling:
      {
        case eIOTestPage:
          DisplayEncoderHandler(Cntr, EncoderList[Cntr].LastPosition);
          break;
        case eFrontPage:
        case eAboutPage:
        case eFreqEntryPage:
        case eBandPage:
        case eModePage:
        case eNRPage:
        case eRFPage:
          Action = GetEncoderAction(Cntr, Is2ndAction[Cntr]);       // get the assigned action; "multi" can be set in main or 2nd
          if (Action == eENMulti)                                   // if "multi" handle differently
          {
            if (Is2ndAction[Cntr])                                  // 2nd action - update the assigned action
            {
              if (Movement > 0)
              {
                if ((int)GMultiAction < (VNUMENCODERACTIONS-2))
                  GMultiAction = (EEncoderActions)((int)GMultiAction + 1);
              }
              else if (Movement < 0)
              {
                if ((int)GMultiAction > 0)
                  GMultiAction = (EEncoderActions)((int)GMultiAction - 1); 
              }
              DisplaySetEncoderAction(Cntr, GMultiAction, true);
              MakeEncoderActionCAT(GMultiAction);
            }
            else
            {
              CATHandleEncoder(Cntr, Movement, GMultiAction);       // apply the assigned action
              DisplayEncoderTurned(Cntr);              
            }
          }
          else
          {
            CATHandleEncoder(Cntr, Movement, Action);
            DisplayEncoderTurned(Cntr);
          }
          break;
        case eSettingsPage:                                 // no action when on these pages
        case eConfigurePage:
            break;
      }
#endif      
    }
  }

//
//read the VFO encoder; divide by N to get the desired step count
//
long ct = (VFOEncoder.read())/GVFOEncoderDivisor;
  if (ct != old_ct)
  {
#ifdef SENSORDEBUG
    Serial.print("VFO encoder turned; ");
    if (ct > old_ct)
    {
      Serial.print(ct-old_ct);
      Serial.println (" steps c/w");
    }
    else
    {
      Serial.print(old_ct-ct);
      Serial.println (" steps ac/w");
    }
#else

    //format the output for printing 
    char buf[50];
    switch (GDisplayPage)                                   // display dependent handling:
    {
      case eIOTestPage:
        DisplayEncoderHandler(8, ct);
        break;
      case eFrontPage:
      case eAboutPage:
      case eFreqEntryPage:
      case eBandPage:
      case eModePage:
      case eNRPage:
      case eRFPage:
        CATHandleVFOEncoder(ct-old_ct);
        break;
      case eSettingsPage:                                 // no action when on these pages
      case eConfigurePage:
          break;
    }
#endif    
    old_ct=ct;
  }
}



//
// lookup encoder number of switch number
// value stored = 0xFF if not an encoder button
//
byte GLookupEncoderNumber[] = 
{
  0xFF, //    SW1                 0
  0xFF, //    SW2                 1
  0xFF, //    SW3                 2
  0xFF, //    SW4                 3
  0xFF, //    SW5                 4
  0xFF, //    SW6                 5
  0xFF, //    SW7                 6
  0xFF, //    SW8                 7
  0xFF, //    SW9                 8
  0xFF, //    SW10                9
  0xFF, //    SW11               10
  0xFF, //    SW12               11
  0xFF, //    SW13               12
  0xFF, //    SW14               13
  0xFF, //    SW15               14
  0xFF, //    SW16               15
  0xFF, //    SW17               16
  0,    //    encoder 2 push     17
  2,    //    encoder 3 push     18
  4,    //    encoder 4 push     19
  6     //    encoder 5 push     20
};


//
// handler for a button event where button is set to be the encoder control button
// (for dual function encoders)
// check button number is in range to be an encoder button! (user programming error if not)
//
void EncoderHandleButton(unsigned int ButtonNum, bool IsPress)
{
  EEncoderActions Action;
  int EncoderNumber;

  if (ButtonNum < VMAXBUTTONS)
  {
    EncoderNumber = GLookupEncoderNumber[ButtonNum];
    if (EncoderNumber != 0xFF)                                                      // if a "real" encoder button
    {
      DisplayEncoderTurned(EncoderNumber);                    // this will reset the legend display to top encoder
      if (GEncoderOperation == eDualFnClick)                  // dual fn; press toggles which function
      {
        if (IsPress)
          Is2ndAction[EncoderNumber] = !Is2ndAction[EncoderNumber];
      }
      else if (GEncoderOperation == eDualFnPress)             // dual fn; 2nd function while pressed
        Is2ndAction[EncoderNumber] = IsPress;
//
// now send the new encoder action to the display, UNLESS it is multi;
//
      Action = GetEncoderAction(EncoderNumber, Is2ndAction[EncoderNumber]);
      if (Action != eENMulti)
        DisplaySetEncoderAction(EncoderNumber, Action, false);    
    }
  }
}



//
// get a bool to say if 2nd action selected or not
//
bool GetEncoderMain2ndAction(unsigned int Encoder)
{
  bool Result;

  if (Encoder < VMAXENCODERS)
    Result = Is2ndAction[Encoder];
  else
    Result = false;
  return Result;
}
