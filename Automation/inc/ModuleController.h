//-------------------------------------------------------------------------------------------------
//
//  File : module_controller.h
//
//-------------------------------------------------------------------------------------------------
//
// Copyright(c) 2023 Alain Royer.
// Email: aroyer.qc@gmail.com
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software
// and associated documentation files (the "Software"), to deal in the Software without
// restriction, including without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or
// substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
// INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
// AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
//------ Note -------------------------------------------------------------------------------------
//
//  At startup when the module start ,if there is nothing in the Memory, default scenario will 
//  be programmed, the loader start loading all the formula timer and function.
//  So this way basic functionnality will work locally on the module.
//
//-------------------------------------------------------------------------------------------------

#pragma once

//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Include file(s)
//-------------------------------------------------------------------------------------------------



//-------------------------------------------------------------------------------------------------
// Define(s)
//-------------------------------------------------------------------------------------------------

#define MAX_RECURSIVITY 	        8           // Should be in cfg file

//-------------------------------------------------------------------------------------------------
// Typedef(s)
//-------------------------------------------------------------------------------------------------

struct CommonToBlock_t
{
    uint16_t IsInitialized    :1;
    uint16_t IsValid          :1;
    uint16_t OutputChannel;
    uint16_t OverrideChannel;
    uint16_t DefaultValue;                   // this is the value until we receive the channel at least once 
    uint8_t  FunctionType;
    uint8_t  Status;
    uint8_t  Error;
};

// Input
typedef struct
{
    CommonToBlock_t Common;
   	uint8_t Crc;			   

    // at this point this is data not program
    struct
    {
        uint8_t FifoPointer:6;
        uint8_t ChannelAsk:1;
        uint8_t ChannelReceived:1;        // we have receive the channel status at least once
    } InternalStatus;
} BusInput_t;

// Output
typedef struct
{                
    CommonToBlock_t Common;
    struct
    {
        uint16_t Status:11;
        uint16_t PassFirstRun:1;
        uint16_t FifoPointer:4;
    } Set;
   	uint8_t Crc;			   

    uint16_t LastSentValue;
} BusOutput_t;

// Counter
typedef struct
{
    CommonToBlock_t Common;
    struct
    {
        uint16_t Status:11;
        uint16_t IntegerType:1;
        uint16_t StatusType:1;
        uint16_t Free:3;
        uint16_t Inverter:1;
        uint16_t EdgeState:1;
    } Set;
    struct
    {
        uint16_t Status:11;
        uint16_t IntegerType:1;
        uint16_t StatusType:1;
        uint16_t Free:2;
        uint16_t UseEdge:1;
        uint16_t Inverter:1;
        uint16_t EdgeState:1;
    } Reset;
   	uint8_t Crc;			   
} Counter_t;

// Schedule Day
typedef struct
{
    CommonToBlock_t Common;
    
    struct
    {
        uint16_t Sunday             : 1;
        uint16_t Monday             : 1;
        uint16_t Tuesday            : 1;
        uint16_t Wednesday          : 1;
        uint16_t Thursday           : 1;
        uint16_t Friday             : 1;

        // Holidays
        uint16_t Christmas          : 1;
        uint16_t BoxingDay          : 1;
        uint16_t NewYear            : 1;

        // Easter
        uint16_t GoodFriday         : 1;
        uint16_t EasterSaturday     : 1;
        uint16_t Easter             : 1;
        uint16_t EasterMonday       : 1;
    
        // Halloween
        uint16_t Halloween          : 1;
    } Bit;

} ScheduleDay_t;

// Add custom fixed date time for user specific holidays or personnal days. so it can be linked to special scenario
// user may chose Friday or Monday as special holidays replacement if it fall on week-ends

// FlipFlop
typedef struct
{
    CommonToBlock_t Common;
    struct
    {
        uint16_t UseEdge:1;
        uint16_t Inverter:1;
        uint16_t EdgeState:1;
    } Set;
    struct
    {
        uint16_t UseEdge:1;
        uint16_t Inverter:1;
        uint16_t EdgeState:1;
    } Reset;
} FlipFlop_t;

// Formula
typedef struct
{
    CommonToBlock_t Common;
} Formula_t;

// Gate
typedef struct
{
    CommonToBlock_t Common;
    uint16_t InverterOnOutput:1;
    uint16_t OneShot:1;
    uint16_t Inverter:1;
    uint16_t EdgeState:1;
} Gate_t;

// Schedule Time
typedef struct
{
    CommonToBlock_t Common;
    uint16_t StartTime;
    uint16_t EndTime;
} ScheduleTime_t;

// Schedule Time Day
typedef struct
{
    CommonToBlock_t Common;
    uint16_t StartTime;
    uint16_t EndTime;
    uint8_t BitPosition;
} ScheduleTimeDay_t;

// Selector
typedef struct
{
    CommonToBlock_t Common;
    uint16_t StatusConstantS1:1;
    uint16_t StatusConstantS2:1;
} Selector_t;


// Comparator
typedef struct
{
    CommonToBlock_t Common;
} Comparator_t;

// Priority         // not clearly define..
typedef struct
{
    CommonToBlock_t Common;
} Priority_t;

// RandomCycle
typedef struct
{
    CommonToBlock_t Output;
    struct
    {
        uint16_t Status:11;
        uint16_t CycleLevel:1;
        uint16_t NewStart:1;
        uint16_t InCycle:1;
        uint16_t Free:2;
    } Set;
    uint8_t MinimumOff;
    uint8_t MaximumOff;
    uint8_t MinimumOn;
    uint8_t MaximumOn;
   	uint8_t Crc;			   

    // at this point this is data not program
    uint32_t dwSetStamp;
} RandomCycle_t;

// SUNRISE_SET
typedef struct
{
    CommonToBlock_t Output;
    uint8_t Minute;
    struct
    {
        uint8_t BeforeAfter:1;
        uint8_t RiseSet:1;
        uint8_t Free:6;
    } Config;
   	uint8_t Crc;			   
} SunRiseSet_t;

// TIMER
typedef struct
{
    uint8_t FunctionType;
    struct
    {
        uint16_t Status:11;
        uint16_t TimerRange:1;                 // iciiop 9 aout 2007 should use status type and integer ??? hein
        uint16_t TimerRunning:1;   
        uint16_t TimerOut:1;
        uint16_t Inverter:1;
        uint16_t EdgeState:1;
    } Set;
    struct
    {
        uint16_t Status:11;
        uint16_t PresetConstant:1;
        uint16_t PercentConstant:1;
        uint16_t UseEdge:1;
        uint16_t BoundaryCheck:1;
        uint16_t EdgeState:1;
    } Reset;
    uint8_t Percent;
    uint16_t Preset;
   	uint8_t Crc;			   

    // at this point this is data not program
    uint32_t dwSetStamp;
} Timer_t;

// TOGGLE
typedef struct
{
    CommonToBlock_t Output;
    struct
    {
        uint16_t Status:11;
        uint16_t Free:3;
        uint16_t Inverter:1;
        uint16_t EdgeState:1;
    } Set;
    struct                
    {
        uint16_t Status:11;
        uint16_t Free:2;
        uint16_t UseEdge:1;
        uint16_t Inverter:1;
        uint16_t EdgeState:1;
    } Reset;
   	uint8_t Crc;			   
} Toggle_t;

// DOWN COUNTER
typedef struct
{
    CommonToBlock_t Output;
    struct
    {
        uint16_t Status:11;  
        uint16_t StatusConstantValue:1;
        uint16_t BoundaryCheck:1;
        uint16_t TimerOut:1;
        uint16_t Inverter:1;
        uint16_t EdgeState:1;
    } Set;
    struct
    {
        uint16_t Status:11;  
        uint16_t MinuteSecondeType:1;
        uint16_t TimerRunning:1;   
        uint16_t UseEdge:1;
        uint16_t Inverter:1;
        uint16_t EdgeState:1;
    } Reset;
    union
    {
        struct
        {
            uint16_t Status:11;
            uint16_t free:5;
        } S;
        uint16_t Constant;
    } Value;
   	uint8_t Crc;			   

    // at this point this is data not program
    uint8_t SetStamp;
} CountDown_t;

// EVENT -> Single cycle 
typedef struct
{
    CommonToBlock_t Output;
	uint8_t FunctNmbrByte;			// Total number of byte in this Event function
    uint8_t Group;
    struct
    {
        uint8_t AND_OR_Act:1;			// AND = 1 and OR = 0
        uint8_t Free:5;
    } Options;

	union							// If partition options are used we have 2 possible choise 
	{
		struct
		{
			uint8_t aFreeTable[3];
		} ActDecimal;
		struct
		{
		} ActBitPos;
	} Partitions;			// Union of 4 bytes total
	// The reste of the data is the operation that we need to do with the Event 
	// and this info can be different for each Event function so we do not have it here

} EventCycle_t;

// EVENT -> Single cycle and ON/OFF RAM usage
typedef struct
{
    uint8_t FifoPointer;             // on the last check event in buffer 
	uint8_t MultipleUsage;			
} EventRAM_t;


// EVENT -> Single ON/OFF
typedef struct
{
    CommonToBlock_t Output;
	uint8_t FunctNmbrByte;			// Total number of byte in this Event function
    uint8_t GroupOn;
    uint8_t GroupOff;
    struct
    {
        uint8_t AND_OR_Act:1;			// AND = 1 and OR = 0
        uint8_t Free3:1;
        uint8_t AND_OR_Deact:1;		// AND = 1 and OR = 0
        uint8_t Free7:1;
    } Options;


} EventOnOff_t;

// STATUS
typedef struct
{
    union
    {
		struct
		{
            uint16_t Unknown:14;
	        uint16_t Constant:1;           // treat this a a constant
	        uint16_t StatusID:1;           // always set to 1
		} Unknown;		 // Common bit

		struct
		{
            uint16_t Value:14;
	        uint16_t Constant:1;           // treat this a a constant
	        uint16_t StatusID:1;           // always set to 1
		} Constant;		 // for constant

        struct
        {
            uint16_t Status:11;
            uint16_t Free:2;               // nothing... it's free can't you read it!!!
            uint16_t Inverter:1;           // the status will be inverted if it is a boolean
	        uint16_t Constant:1;           // treat this a a constant
	        uint16_t StatusID:1;           // always set to 1
        } Status;        // for status
		
    } Input;

} Status_t;            

// Generic header
typedef struct
{
    uint8_t FunctionType;
    CommonToBlock_t Output;
} Generic_t;

// each event that is captured from the digiplex bus, is put in this fifo buffer.
// The fifo pointer will indicate (last entry + 1).
// each Function using this buffer keep a reference to know where they are in there treatment
typedef struct 
{
    uint8_t FifoPointer;
    struct
    {
        uint8_t Group;
        uint16_t PrimaryNumber; 
        uint16_t SecondaryNumber; 
    } Array[EVENT_BUFFER_FIFO_MAX];
} EventBuffer_t;

typedef struct
{
    uint8_t Group;
    Uuint16_t PrimaryNumber; 
    Uuint16_t SecondaryNumber; 
}EventData_t;


//-----------------------------------------------------------------

uint8_t 			        RecursivityLevel;

BusInput
BusOutput
Comparator
Counter  
CountDown 
FlipFlop  
EventCycle
EventOnOff
EventRAM	
InterpretorDouble 
InterpretorSingle
Gate             
Partition        
Priority         
RandomCycle      
ScheduleDay  
ScheduleTime 
ScheduleTimeDay 
Selector	    
SunRiseSet      
Timer	        
Toggle          
Generic         

//-------------------------------------------------------------------------------------------------
// const(s)
//-------------------------------------------------------------------------------------------------

// TODO move to a more generic file
const uint8_t EasterTable[48] =     // Table from 2024 to 2071
{
    90,  110, 95,  87,  107, 91,  111, 103, // 2024 to 2031
    88,  107, 99,  84,  104, 95,  115, 100, // 2032 to 2039
    92,  111, 96,  88,  108, 99,  84,  104, // 2040 to 2047
    96,  108, 100, 92,  112, 96,  88,  108, // 2048 to 2055
    108, 112, 104, 89,  109, 100, 85,  105, // 2056 to 2063
    97,  88,  101, 93,  113, 104, 89,  109  // 2064 to 2071
}
 
//-------------------------------------------------------------------------------------------------
// class definition(s)
//-------------------------------------------------------------------------------------------------

class BlockInterface
{
    public:
        virtual void    Create                  (void)      = 0;      // Allocated ressources.
        virtual void    Update                  (void)      = 0;      // Update the parameters.
        virtual void    Refresh                 (void)      = 0;      // Run function and update.
        virtual void    finalize                (void)      = 0;      // Deallocated ressources.
};

//-------------------------------------------------------------------------------------------------

class AutomationController
{
    public:
    
        void            Run                     (void);
        bool            AddStrategy             (uint8_t** PStrategy);
        bool            RemoveStrategy          (uint8_t** PStrategy);
        
    private:
        
        bool            ProcessStrategy         (uint8_t** PStrategy);
        void            ProcessBlock            (uint8_t** pBlock);
        void            ScanBlock               (void);
        bool            CheckIntegrityStrategy  (uint8_t*  PStrategy);
};

//-------------------------------------------------------------------------------------------------
// use chain list to handle the strategy pointer list

