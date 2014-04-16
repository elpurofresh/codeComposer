//******************************************************************************
//  MSP430G2452
// Built with CCS Version :  4.2.1.00004  
//******************************************************************************
#include "structure.h"

//PinOsc Volume down P2.2
const struct Element down = {

              .inputPxselRegister = (uint8_t *)&P2SEL,  
              .inputPxsel2Register = (uint8_t *)&P2SEL2,  
              .inputBits = BIT2,
              // maxResponse - threshold < 655
              .maxResponse = 366, 
              .threshold = 135 
};      

//PinOsc forward right P2.3
const struct Element right = {

              .inputPxselRegister = (uint8_t *)&P2SEL,  
              .inputPxsel2Register = (uint8_t *)&P2SEL2,  
              .inputBits = BIT3,
              // maxResponse - threshold < 655
              .maxResponse = 378,
              .threshold = 155 
};

//PinOsc Volume up, P2.4
const struct Element up = {

              .inputPxselRegister = (uint8_t *)&P2SEL,  
              .inputPxsel2Register = (uint8_t *)&P2SEL2,  
              .inputBits = BIT4,
              // maxResponse - threshold < 655
              .maxResponse = 582,
              .threshold = 110 
};      

//PinOsc reverse left P2.1
const struct Element left = {

              .inputPxselRegister = (uint8_t *)&P2SEL,  
              .inputPxsel2Register = (uint8_t *)&P2SEL2,  
              .inputBits = BIT1,
              // maxResponse - threshold < 655
              .maxResponse = 359,
              .threshold = 154 
};      

//PinOsc Wheel: middle button P2.5
const struct Element middle = {

              .inputPxselRegister = (uint8_t *)&P2SEL,  
              .inputPxsel2Register = (uint8_t *)&P2SEL2,  
              .inputBits = BIT5,
              .threshold = 0 
};

//PinOsc proximity: P2.0
const struct Element proximity = {

              .inputPxselRegister = (uint8_t *)&P2SEL,  
              .inputPxsel2Register = (uint8_t *)&P2SEL2,  
              .inputBits = BIT0,
              .threshold = 0 
};      

//*** CAP TOUCH HANDLER *******************************************************/
// This defines the grouping of sensors, the method to measure change in
// capacitance, and the function of the group

const struct Sensor wheel =
               { 
                  .halDefinition = RO_PINOSC_TA0_WDTp,
                  //.halDefinition = fRO_PINOSC_TA0_SW,
                  .numElements = 4,
                  //.points = 16,
                  //.sensorThreshold = 75,
                  .baseOffset = 0,
                  // Pointer to elements
                  .arrayPtr[0] = &up,  // point to first element
                  .arrayPtr[1] = &right,  // point to first element
                  .arrayPtr[2] = &down,  // point to first element
                  .arrayPtr[3] = &left,  // point to first element
                  // Timer Information
                  .measGateSource= GATE_WDTp_SMCLK,     
                  //.accumulationCycles= WDTp_GATE_32768       //32768
                  //.accumulationCycles= WDTp_GATE_8192          // 8192
                  .accumulationCycles= WDTp_GATE_512         //512, default
                  //.accumulationCycles= WDTp_GATE_64          //64          
                  //.accumulationCycles = 512                      
               };

const struct Sensor middle_button =
               { 
                  .halDefinition = RO_PINOSC_TA0_WDTp,
                  .numElements = 1,
                  .baseOffset = 4,
                  // Pointer to elements
                  .arrayPtr[0] = &middle,  // point to first element
                  // Timer Information
                  .measGateSource= GATE_WDT_SMCLK,     
                  //.accumulationCycles= WDTp_GATE_32768       //32768
                  //.accumulationCycles= WDTp_GATE_8192          // 8192
                  .accumulationCycles= WDTp_GATE_512         //512, default
                  //.accumulationCycles= WDTp_GATE_64          //64                  
               };

const struct Sensor proximity_sensor =
               { 
                  .halDefinition = RO_PINOSC_TA0_WDTp,
                  .numElements = 1,
                  .baseOffset = 5,
                  // Pointer to elements
                  .arrayPtr[0] = &proximity,          // point to first element
                  // Timer Information
                  //.measGateSource= GATE_WDT_SMCLK,     //  SMCLK
                  .measGateSource= GATE_WDT_ACLK,        //  ACLK
                  //.accumulationCycles= WDTp_GATE_32768         //32768
                  //.accumulationCycles= WDTp_GATE_8192          // 8192
                  .accumulationCycles= WDTp_GATE_512             //512 default
                  //.accumulationCycles= WDTp_GATE_64            //64                  
               };
               
// Line 57
              // .threshold = 126 // for SMCLK of 1Mhz/4, WDT interval of 512
// Line 66
              // .threshold = 150 // for ACLK of VLO, WDT interval of 512
