
//------ Include Files ----------------------------------------------------------------------------

#define GLOBAL_MODULE_CONTROLLER
#include "include.h"




#define BLOCK_FIRST  '@'
#define BLOCK_LAST   '\0x7F'
#define BLOCK_FREE   '\0xFF'
#define BLOCK_RESULT 0x3D3D

//-------------------------------------------------------------------------------------------------
//
//  Name:           CreateAllBlock
//
//  Parameter(s):   None
//  Return:         None
//
//  Description:    Allocated all Block from the memory pool
//
//-------------------------------------------------------------------------------------------------
void AutomationController::CreateAllBlock(void)
{
    BlockInterface_t*     pBlock;
    BlockInterface_t**    pBlockListPointer;
    Block_e             BlockType;
    int                 Count;                                                          // Block count in page
    int                 CountCopy;

    m_BlockCount = 0;
    //while((m_BlockCount)->ID != INVALID_WIDGET) // change for number of block in formula
    {
        m_BlockCount++;
    }

    // Reserve memory for CBlockInterface pointer for the number of block
    m_pBlockList = (BlockInterface_t**)pMemoryPool->Alloc(m_BlockCount * sizeof(BlockInterface_t*));

    Count               = 0;
    pBlockListPointer  = m_pBlockList;

    while(Count < m_BlockCount)
    {
        BlockType = 0;// de quoi;

        switch()
        {
            case FCT_INPUT:
            {
                pBlock             = (BlockInterface_t*)pMemoryPool->Alloc(sizeof(BLK_BusInput));     // Get the memory for block class object
                // need change  *pBlockListPointer = new(pBlock) BLK_BusInput(&Background[Block - (APP_START_BACK_CONST + 1)]);       // Get a class object of the widget
            }
            break;
            
            case FCT_OUTPUT:
            {
                pBlock             = (BlockInterface_t*)pMemoryPool->Alloc(sizeof(BLK_BusOuput));                       // Get the memory block for the widget class object
                // need change  *pBlockListPointer = new(pBlock) BLK_BusInput(&Background[Block - (APP_START_BACK_CONST + 1)]);       // Get a class object of the widget
            }
            break;
            
            case FCT_GATE:
            {
            }
            break;
            
            case FCT_FLIP_FLOP:
            {
            }
            break;
            
            case FCT_COUNT_DOWN:
            {
            }
            break;
            
            case FCT_EVENT_COUNTER:
            {
            }
            break;
            
            case FCT_TIME_COUNTER:
            {
            }
            break;
            
            case FCT_PRIORITY:
            {
            }
            break;
            
            case FCT_SELECTOR:
            {
            }
            break;
            
            case FCT_TOGGLE:
            {
            }
            break;
            
            case FCT_TIMER:
            {
            }
            break;

            case FCT_COMPARATOR:
            {
            }
            break;

            case FCT_RANDOM_CYCLE:
            {
            }
            break;

            case FCT_SCHEDULE_DAY:
            {
            }
            break;

            case FCT_SCHEDULE_TIME:
            {
            }
            break;
            
            case FCT_INTERPRETOR:
            {
            }
            break;
            
            case FCT_PID:
            {
            }
            break;
            
            case FCT_SCHEDULE_TIME:
            {
            }
            break;
            
            case FCT_SUNSET:
            {
            }
            break;
            
            case FCT_SUNRISE:
            {
            }
            break;
        }
        
        Count++;
        pPage++;
        pBlockListPointer++;
    }

    // Reset all pointer and count to beginning of the list
    Count             = 0;
    pBlockListPointer = m_pBlockList;

    while(Count < m_BlockCount)
    {
        NewLink = (*pBlockListPointer)->Create(pBlock);
        pBlockListPointer++;
        Count++;
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           GetBlockSize
//
//  Parameter(s):   Pointer on block type
//  Return:         None
//
//  Description:    Will return the size of data to copy in memory and save in ROM + the size to
//                  allocate in memory in a structure
//
//-------------------------------------------------------------------------------------------------
void AutomationController::GetStrategySize(Generic* pBlock, sSize* pSize)
{
    uint8_t FctTemp;
    FreeMem_t * pFreeMem = (FreeMem_t *)pBlock;

    FctTemp = pBlock->pBlockType;

    if((FctTemp >= BLOCK_FIRST) && (FctTemp <= BLOCK_LAST))
    {
        switch(FctTemp) 
        {
            case 'F':
            {
                // calculate length of the equation 
                pSize->ToAlloc = 3;
                while((*(uint16_t *)(((uint8_t*)pBlock) + pSize->ToAlloc) != FUNCTION_RESULT) && (pSize->ToAlloc < 255))
                {
                    pSize->ToAlloc++;
                }
                pSize->ToAlloc+=3;                      // Add the '==' and the CRC te
                if(pSize->ToAlloc > 256)                // Reach the limit size of a equation
                {
                    pSize->ToAlloc = 0;                 // Data is not good so flush the size to alloc
                }
                pSize->ToCopy = pSize->ToAlloc;
            }
            break;

            case 'V':
            case 'W':
            {
                // calculate length of the Event functions (variable lenght) 
                pSize->ToCopy = (*(((uint8_t*)pBlock) + 3));   // Load the function total data lent including the CRC
                // Verify if the '==' character are at the end of the function acording to the lent find in byte 4.
                if((*(uint16_t *)(((uint8_t*)pBlock) + (pSize->ToCopy - 3)) != FUNCTION_RESULT) || (pSize->ToCopy > 255))
                {
                    pSize->ToCopy  = 0;                 // Data is not good so flush the size to alloc
                    pSize->ToAlloc = 0;
                }
                else
                {
                    pSize->ToAlloc = pSize->ToCopy + uint8_t(sizeof(EventRAM));  // Need more byte in RAM for the Event process
                }
            }
            break;

            case FUNCTION_FREE:
            {
                pSize->ToAlloc = pFreeMem->FunctionSize;
                pSize->ToCopy  = pSize->ToAlloc;
            }
            break;

            default:
            {
                FctTemp -= '@';
                pSize->ToAlloc = SizeToAllocOfFunction[FctTemp];
                pSize->ToCopy  = SizeToCopyOfFunction[FctTemp];
            }
            break;
        }
    }
    else // protection
    {
        pSize->ToAlloc = 0;
        pSize->ToCopy  = 0;
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           ScanStrategy
//
//  Parameter(s):   None
//  Return:         None
//
//  Description:    Scan all the strategy's in dynamic memory. Different error might be set in this
//                  process.
// 
//  Notes(s):       This process a strategy up to MAX level of recursivity. This function will set
//                  a flag if limit is reach without causing a stack overflow.                         
//
//-------------------------------------------------------------------------------------------------
void AutomationController::ScanStrategy(void)
{
    uint8_t     NmbrLoop;
    uint8_t     i;
    uint8_t*    pFunction;
    Size_t      Size;
    Generic_t*  pGeneric;
    Generic_t*  pGenericQueue;
    uint16_t    TempBuffer[5];

    while((pFunction = GetNextFunction()) != nullptr)
    {
        pGeneric = (Generic*)pFunction;

        if((pGeneric->Output.bToBeDeleted == false) &&
           (pGeneric->Output.bToBeUpdated == false) &&
           (pGeneric->Output.bNewEntryBit == false))            // Only valid function and function not in ROM process are executed
        {
            ProcessFunction(&pFunction);
        }

        if(pRomJobQueue == nullptr)                             // If no job sent to RomManager.
        {
            pGeneric = (Generic*)pFunction;                     // Cast function in Generic struct
            pGenericQueue = (Generic*)pJobQueue;                // Cast Last function (if any) in Generic struct
            if(pGeneric->Output.bToBeDeleted == true)           // Output must be deleted?
            {
                pRomJobQueue = pGeneric;                        // Send it immediately to RomManager
            }
            else if(pJobQueue == nullptr)                       // If Next function empty
            {
                if( (pGeneric->Output.bToBeUpdated == true) ||
                    (pGeneric->Output.bNewEntryBit == true) )   // If function update or new function
                    pJobQueue = pGeneric;                       // Put it in next function to treat
            }
            else if(pGeneric == pGenericQueue)                  // If all function scanned, means we can put it in ROM
            {
                GetFunctionSize((void*)pGeneric, &Size);
                pRomJobQueue = pJobQueue;
                wRomJobSize = Size.ToCopy;
                pJobQueue = nullptr;
                
                if(pGeneric->Output.bNewEntryBit == true)       // Copy Bit options only if it is a NEW ENTRY formula
                {
                }
            }
            // Verify if we need to delete only with formula that are not in new entry
            else if(pGeneric->Output.bNewEntryBit == false) 
            {
                if(pGeneric->Output.Status == pGenericQueue->Output.Status)
                {
                                                                // If scan same function as requested new entry,
                    if(pGeneric->Output.bToBeUpdated == false)  // If this is not a function update, but an output update
                        pGeneric->Output.bToBeDeleted = true;   // Must delete it first in ROM before putting new one

                    pGeneric->Output.bToBeUpdated = true;       // Update of an existing function will cancel the increment of the
                                                                // System pointer in the ROM manager

                    pRomJobQueue = pGeneric;                    // bToBeDeleted = false --> Only change value in eeprom
                }
                else    // Verify if the formula has they have more then one output status      ( none at this time )
                {
                    //else                                      // Formula to validate as only one output status
                    {
                        TempBuffer[0] = pGeneric->Output.Status;
                        NmbrLoop = 1;
                    }
                     
                    //else                                          // Formula to compare as only one output status     (for now)   
                    {
                        for(i=0 ; i < NmbrLoop ; i++)
                        {
                            if(TempBuffer[i] == pGenericQueue->Output.Status)
                            {
                                if(pGeneric->Output.bToBeUpdated == false)      // If scan same function as requested new entry,
                                {                                               // If this is not a function update, but an output update
                                    pGeneric->Output.bToBeDeleted = true;       // Must delete it first in ROM before putting new one
                                }

                                pGeneric->Output.bToBeUpdated = true;           // Update of an existing function will cancel the increment of the
                                                                                // System pointer in the ROM manager
                                pRomJobQueue = pGeneric;                        // bToBeDeleted = false --> Only change value in eeprom
                                i = NmbrLoop;                                   // Exit loop
                            }
                        }
                    }
                    // This output status will be deleted but is part of the other Macro output status?
                    if(pGeneric->Output.bToBeDeleted == true)
                    {
                        if(DeletedStatusPointer < MAX_DELETED_STATUS)     // don't know what it is
                            DeletedStatusPointer++;
                    }
                }
            }
        }
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           ProcessBlock
//
//  Parameter(s):   Pointer of pointer on the Function to process
//  Return:         None
//
//  Description:    This process a Block by reading input bit, and set or reset bit according to
//                  result.
// 
//-------------------------------------------------------------------------------------------------
void AutomationController::ProcessBlock(uint8_t** pBlock)
{
    switch((**pFunction))
    {        //     should be update for function class object  // this might be a class object pointer using a interface.. 
        case 'A':   BlockBusInput            ((BusInput_t*)          *pBlock);        break;              // Bus Input
        case 'B':   BlockBusOutput           ((BusOutput_t*)         *pBlock);        break;              // Bus Output
        case 'C':   BlockCounter             ((Counter_t*)           *pBlock);        break;              // Counter (edge trigger)
        case 'D':   BlockScheduleDay         ((ScheduleDay_t*)       *pBlock);        break;              // Schedule Day
        case 'E':   BlockFlipFlop            ((FlipFlop_t *)         *pBlock);        break;              // Flip-Flop

//*********************************************************************************************
        case 'F': //mystery for now
        {
            // make sure recursivity level is at zero
            RecursivityLevel = 0;
            ProcessEquation(&pStrategy);
        }
        break;
//*********************************************************************************************

        case 'G':    BlockGate               ((Gate_t*)              *pBlock);        break;              // follower or inverser
        case 'H':    BlockScheduleTime       ((ScheduleTime_t*)      *pBlock);        break;              // Schedule time
        case 'J':    BlockSelector           ((Selector_t *)         *pBlock);        break;              // Selector
        case 'K':    BlockDoubleIntepretor   ((InterpretorDouble_t*) *pBlock);        break;              // Interpretor double
        case 'L':    BlockSingleInterpretor  ((InterpretorSingle_t*) *pBlock);        break;              // Interpretor single
        case 'M':    BlockComparator         ((Comparator_t*)        *pBlock);        break;              // Comparator
        case 'P':    BlockPriorityManager    ((PriorityManager_t*)   *pBlock);        break;              // Priority manager (AR it's a selector or a interpretor see comment)
//option    last in
//          higher
//          lower
//          priority on one input
// etc...
        case 'R':    BlockRandomCycle        ((RandomCycle_t*)       *pBlock);        break;              // Random Cycle
        case 'S':    BlockSunSetRise         ((SunSetSet_t*)         *pBlock);        break;              // SunRise and Sunset
        case 'T':    BlockToggle             ((Toggle_t*)            *pBlock);        break;              // Toggle
        case 'U':    BlockCountDown          ((CountDown_t*)         *pBlock);        break;              // CountDown
        // Change this method this is shit   // this timer is restarted at each edge trigger
        case 'Q':
        case 'X':
        case 'Y':
        case 'Z':     BlockTimer             ((Timer_t*)             *pBlock);        break;
        case 'V':     BlockEventCycle        ((EventCycle_t*)        *pBlock);        break;              // Event one shot
        case 'W':     BlockEventOnOff        ((EventOnOff_t*)        *pBlock);        break;              // Event on/off
    }
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           Random
//
//  Parameter(s):   uint16_t    Minimum value
//                  uint16_t    And maximum value
//  Return:         None
//
//  Description:    Will return a number inside the requested margin
// 
//-------------------------------------------------------------------------------------------------
uint16_t AutomationController::Random(uint16_t Min, uint16_t Max)
{
    VAR_UNUSED(Min);
    VAR_UNUSED(Max);
    /*
//    use random module of CPU, or simulator in DIGINI
    
    uint16_t Pad;
    struct /// fix uint32_t Pad32;
    
    // randomize pseudo counter
    Crc_Crc_Calc(uPseudoRandom.by.by3, (APPBOOT_Uuint16_t *)&uPseudoRandom.w.lo);
    Pad = uPseudoRandom.w.hi;
    uPseudoRandom.w.hi = uPseudoRandom.w.lo;
    uPseudoRandom.w.lo = wPad;

    // random bit in system   
    pSystemDataBit->Flag.bRandomFastToggle = (bool)(uPseudoRandom.by.by0 & 0x01);

    // extract range
    Pad = wMax - wMin;
    uPad.dw = dwMultiply(uPseudoRandom.w.hi,(uint32_t)wPad);        
    
    return(uPad.w.hi + wMin);
    */return 1;
}

//-------------------------------------------------------------------------------------------------
//
//  Name:           CheckIntegrityStrategy
//
//  Parameter(s):   Pointer on the equation to test
//  Return:         bool            false: Bad strategy
//
//  Description:    Scan strategy or integrity, to see if it meet the required format for the
//                  process to be safe.
// 
//-------------------------------------------------------------------------------------------------
bool AutomationController::CheckIntegrityStrategy(uint8_t* pStrategy)
{
    uint8_t FollowType;
    uint8_t FollowForbidType;
    uint8_t BufferLimit;
    Status *pStatus;
    Status *pStatusNext;
    signed char SubEquationCounter=0;

    pStrategy += 3;
    BufferLimit = 3;
    FollowType = FOLLOW_VALUE | FOLLOW_START_SUB;
    FollowForbidType = 0;

    while(*(uint16_t *)pStrategy != FUNCTION_RESULT)   // Loop until "==" 
    {   
        pStatus = (Status *)(pStrategy);            //20
        pStatusNext = (Status *)(pStrategy + 1);    //84

        if(! pStatus->Input.Unknown.StatusID || 
            (pStatus->Input.Unknown.StatusID && pStatusNext->Input.Unknown.StatusID) )               // If next is not 0x80 (searching for  "=", operator, "(",")" )
        {
            if(*pStrategy == '(')                               
            {
                if(CheckBit(FollowType, START_SUB_BIT) && 
                 !(CheckBit(FollowForbidType, START_SUB_BIT)))           // If we where expecting ( but not if a word operator is preceded... 
                {               
                    FollowType = FOLLOW_VALUE | FOLLOW_START_SUB; // Next should be a value or (
                    FollowForbidType = 0;
                    SubEquationCounter++;
            
                    // stack will be protected if neware send us to big of an equation
                    if(SubEquationCounter > MAX_RECURSIVITY)
                    {
                        return false;
                    }
                }
                else
                {
                    // TO CONSIDER: do we set a flag to tell kind of error
                    return false;
                }
            }
            else if(*pStrategy == ')')
            {
                if(CheckBit(FollowType, END_SUB_BIT))
                {
                    FollowType = FOLLOW_OPERATOR | FOLLOW_END_SUB;
                    FollowForbidType = FOLLOW_uint16_t_OPERATOR;
                    SubEquationCounter--;
                }
                else
                {
                    // TO CONSIDER: do we set a flag to tell kind of error
                    return false;
                }
            }

            else if((*pStrategy == '&') || (*pStrategy == '|') || (*pStrategy == '^') ||
                    (*pStrategy == '>') || (*pStrategy == '<') || (*pStrategy == ']') ||
                    (*pStrategy == '[') || (*pStrategy == '~') || (*pStrategy == '$') ||
                    (*pStrategy == '!') || (*pStrategy == '#') || (*pStrategy == '?') ||
                    (*pStrategy == '%'))
            {
                if(CheckBit(FollowType, OPERATOR_BIT))
                {
                    FollowType = FOLLOW_VALUE | FOLLOW_START_SUB;
                    
                    //Check for Word Operator 
                    if((*pStrategy == '>') || (*pStrategy == '<') || (*pStrategy == ']') ||
                       (*pStrategy == '[') || (*pStrategy == '~') || (*pStrategy == '$') ||
                       (*pStrategy == '!') || (*pStrategy == '?') )
                    {
                        //If we didn't want a word operator 
                        if(!(CheckBit(FollowForbidType, uint16_t_OPERATOR)) )
                        {
                            FollowForbidType = FOLLOW_VALUE_BIT | FOLLOW_START_SUB; 
                        }
                        else
                        {
                            // TO CONSIDER: do we set a flag to tell kind of error
                            return false;
                        }
                    }
                    else
                    {
                        FollowForbidType = 0;
                    }         
                    
                }
                else
                {
                    // TO CONSIDER: do we set a flag to tell kind of error
                    return false;
                }
            }
            else    // if not ( , ) or operator : unknown caracter 
            {
                return false;
            }
        }
        else if(CheckBit(FollowType, VALUE_BIT))
        {
            if(pStatus->Input.Unknown.StatusID)         //F 0980 .00.80
            {
                FollowType = FOLLOW_OPERATOR | FOLLOW_END_SUB;
                //Determine the type of the Status (bit or word)
                //pStatus = (Status *)pStrategy; 
                if(pStatus->Input.Unknown.bConstant == YES)
                {
                    FollowForbidType = 0;//uint16_t                     
                }                                                                         
                else if(pStatus->Input.Status.Status < CORE_STATUS_BYTE_OFFSET)
                {
                    if(!CheckBit(FollowForbidType, VALUE_BIT))
                    {
                        FollowForbidType = FOLLOW_uint16_t_OPERATOR;//BIT
                    }
                    else
                    {
                        // TO CONSIDER: do we set a flag to tell kind of error
                        return false;
                    }
                }
                else
                {
                    FollowForbidType = 0;//uint16_t or uint8_t;
                }
                
                pStrategy++;
            }                                               
            else
            {
                // TO CONSIDER: do we set a flag to tell kind of error
                return false;
            }
        }
        else
        {
            return false;
        }

        pStrategy++;

        if(BufferLimit < 255)     // Equation can have a maximum of 255 byte long
            BufferLimit++;
        else
            return false;
    }
    
    return((SubEquationCounter == 0) ? true:false);
    
}

/*****************************************************************************

   Name:                ProcessEquation

   Parameter(s):        pointer of pointer on the equation to process
   Return(s):           return result of the equation
   Description:         This process an equation up to MAX level of recursivity.
                        this function will set a flag if limit is reach without
                        causing a stack overflow.                          

******************************************************************************/
bool AutomationController::ProcessEquation(uint8_t **pStrategy)
{
    static PastStatus   stStatChanged;
    bool                bStatus;    
    bool                Result;
    bool                ResultOperation;
    bool                LastResult;
    uint8_t             NextOperationToDo;
    uint8_t             SubCount;
    uint16_t            Result;
    uint16_t            Status;    
    PastStatus          StatusChanged;  
    PastStatus          ResultChanged;    
    Status*             pStatus;
    Formula*            pFormula;
        
    bResult = 0;
    Result = 0;
    Status = 0;
    NextOperationToDo = NONE;

    // we need to keep the number of the output status
    // and assign the struct pointer to it
    pFormula = (Formula *)*pStrategy; 
    
    if(RecursivityLevel == 0)
    {
        StatusChanged.PassedStatus = 0; //Même pas sûr d'avoir besoin de ces 2 lignes...
        ResultChanged.PassedStatus = 0;        
        
        LastResult = GetCoreBitStatus(uint16_t(pFormula->Output.Status));
        
        // advanced pointer to relevant data
        (*pStrategy) += 3;   
    }
    

    while(**(uint16_t **)pStrategy != BLOCK_RESULT)           // Loop until "==" end of the equation 
    {
        pStatus = (Status *)*pStrategy;
        if(pStatus->Input.Unknown.StatusID)
        {
            (*pStrategy) += 2;
            
            if(pStatus->Input.Constant.bConstant == YES)
            {
                Status = (uint16_t)pStatus->Input.Constant.Value;                     
                StatusChanged.PassedStatus = 0;
            }                                                                         
            else 
            {
                if(pStatus->Input.Status.Status >= CORE_STATUS_BYTE_OFFSET)        
                {
                    Status = GetCoreAnyValue(pStatus->Input.Status.Status);     
                }
                else  
                {
                    bStatus = GetCoreBitStatus((uint16_t)pStatus->Input.Status.Status);  
                    if(pStatus->Input.Status.Inverter == YES)
                    {
                        bStatus = (bool)!bStatus;           
                    }
                } 
                PassedStatus(pStatus->Input.Status.Status, &StatusChanged, READ);   

               
                if(StatusChanged.Status.bNeStatus == false)           
                {
                    StatusChanged.PassedStatus = 0;    
                } 
            }
        }       
        else if(**pStrategy == '(')                                               
        {
            (*pStrategy)++;
            RecursivityLevel++;
            if(RecursivityLevel > MAX_RECURSIVITY)        // Maximum of recursivity have been reached ? 
            {
                return(0);                                  // Exit function
            }
            bStatus = ProcessEquation(pStrategy);   
            StatusChanged.PassedStatus = stStatChanged.PassedStatus;
            RecursivityLevel--;
        }
        
        switch(NextOperationToDo)
        {
            case '&':   bResultOperation = (bool)(bResult && Status);   break;     
            case '|':   bResultOperation = (bool)(bResult || Status);   break;     
            case '^':   bResultOperation = (bool)(bResult ^  Status);   break;     
            case '#':   bResultOperation = (bool)(bResult == Status);   break;     
            case '%':   bResultOperation = (bool)(bResult != Status);   break;     
            case '>':   bResultOperation = (bool)(Result >  Status);  break;
            case '<':   bResultOperation = (bool)(Result <  Status);  break;
            case ']':   bResultOperation = (bool)(Result >= Status);  break;
            case '[':   bResultOperation = (bool)(Result <= Status);  break;
            case '~':   bResultOperation = (bool)(Result == Status);  break;
            case '$':   bResultOperation = (bool)(Result &  Status);  break;     
            case '!':   ResultOperation = (bool)(Result |  Status);   break;     
            case '?':   ResultOperation = (bool)(Result != Status);   break;     
        }
        if(NextOperationToDo != NONE)
        {
            if(!StatusChanged.Status.bNeStatus)
                {
                    if( ResultChanged.Status.bNeStatus)
                    {
                        StatusChanged.PassedStatus = ResultChanged.PassedStatus;
                    }
                    else
                    {
                        StatusChanged.PassedStatus = 0;
                    }
                }
            Status = ResultOperation;
        }
        NextOperationToDo = NONE;

        if((**pStrategy == '&') || (**pStrategy == '|') || (**pStrategy == '^') ||
           (**pStrategy == '>') || (**pStrategy == '<') || (**pStrategy == ']') ||
           (**pStrategy == '[') || (**pStrategy == '~') || (**pStrategy == '$') ||
           (**pStrategy == '!') || (**pStrategy == '#') || (**pStrategy == '?') ||
           (**pStrategy == '%'))
        {
            NextOperationToDo = **PtrEquation;    
            (*pStrategy)++;
            Result = Status;
            ResultChanged.PassedStatus = StatusChanged.PassedStatus;
            Result = Status;
        }

        if((**PtrEquation == ')') && (!CheckBit(*((*PtrEquation)+1),7)))
        {
            (*PtrEquation)++;
            stStatChanged.PassedStatus = StatusChanged.PassedStatus;
            return(Status);
        }
    }

    // Store result in Bit status only if we are back at recursivity level 0
    if(RecursivityLevel == 0)
    {
        SetCoreBitStatus((uint16_t)pFormula->Output.Status,bStatus);  // result is put in Bit Status
    }

    // If output changed or still at On and attention, vérifier si PassedStatus != 0 dans le cas où déjà à ON
    if(StatusChanged.Status.bNeStatus == 1) 
    {
        if(bStatus == 1)
        {
            //J'utilise la FORCE parce qu'il se peut qu'on ait une situation comme celle là :
            // A < B      où A change et où A était déjà < que B, 
            // on voudrait signifier que A a changé même si c'était déjà lui qui avait changé la dernière fois pour l'output de cette fonction
            PassedStatus((uint16_t)pFormula->Output.Status, &StatusChanged, FORCE);   
        }
        else // Ignore the change, just put de NeStatus = 0
        {   
            PassedStatus((uint16_t)pFormula->Output.Status, &PassedStatus[pFormula->Output.Status], WRITE); // Passer on propre index d'output équivaut à baisser le flag NeStatus  
        }
    }
    else // S'il n'y a pas eu de changement, baisser le flag
    {
        PassedStatus((uint16_t)pFormula->Output.Status, &PassedStatus[pFormula->Output.Status], WRITE); // Passer on propre index d'output équivaut à baisser le flag NeStatus      
    }
    return(0);
}





#if 0 
    bool        bStatus = 0;
    bool        bStatus1 = 0;
    bool        bStatus2 = 0;
    uint8_t     i;
    uint8_t*    pStrategy = *pFunction;
    uint16_t    Pad1 = 0;
    uint16_t    Pad2 = 0;
    uint16_t    Pad3 = 0;          
    uint16_t    Pad4 = 0;          
    PastStatus  PassedStatus;
    uint32_t    dwPad = 0;          


//*********************************************************************************************
        case 'A':           // Bus Input
        {
            pBusInput = (BusInput *)*pFunction; 

            // set the initial state of the Function
            if((pBusInput->InternalStatus.bChannelReceived == NO) &&
               (pBusInput->InternalStatus.bChannelAsk == NO))
            {
                SetCoreAnyValue(pBusInput->Output.Status, pBusInput->wStartValue);

                pBusInput->InternalStatus.byFifoPointer = stChannelBufferRX.byFifoPointer;  // Start analysing the RX command where the fifopointer is now
                pBusInput->InternalStatus.bChannelAsk = YES;
                
                // TODO i have no process yet to ask the channel    need it
                // put here the request for this channel in a command!!!!
            }
            
            while(pBusInput->InternalStatus.byFifoPointer != stChannelBufferRX.byFifoPointer)
            {
                // update pointer to new location 
                pBusInput->InternalStatus.byFifoPointer++;
                if(pBusInput->InternalStatus.byFifoPointer >= CHANNEL_RX_BUFFER_FIFO_MAX)
                {
                    pBusInput->InternalStatus.byFifoPointer = 0;
                }                

                // get the channel number
                pBusInChannelData = (BusInChannelData *)&stChannelBufferRX.Array[pBusInput->InternalStatus.byFifoPointer];

                if ((pBusInChannelData->MessageID == VB_E_BUS_OUTPUT_BROADCAST) ||                // This Message ID is a Bus output type
                    (pBusInChannelData->MessageID == VB_E_BUS_OUTPUT_REFRESH))
                {
                    if(pBusInput->wChannel == pBusInChannelData->wChannel)
                    {
                        // reset flag for initial state of the Function
                        pBusInput->InternalStatus.bChannelReceived = YES;
                        pBusInput->InternalStatus.bChannelAsk = NO;

                        // write the output with the acquired status from the incoming channel
                        SetCoreAnyValue(pBusInput->Output.Status, pBusInChannelData->wData);
                    }
                }
            }
            
            // this is a static only status
            PassedStatus.Status.Status = pBusInput->Output.Status;
            PassedStatus(pBusInput->Output.Status, &PassedStatus, WRITE);

            break;
        }
//*********************************************************************************************
        case 'B':           // Bus Output
        {
            pBusOutput = (BusOutput *)*pFunction; 

            wPad1 = GetCoreAnyValue(pBusOutput->Set.Status);

            // check for the override static
            if(pBusOutput->Output.Static == YES)
            {
                PassedStatus.Status.Status = pBusOutput->Output.Status;
            }           
            else
            {
                PassedStatus(pBusOutput->Set.Status, &PassedStatus, READ);
            }

            // check if it is the first, we do not sent on 
            if(pBusOutput->Set.bPassFirstRun == NO)
            {
                bStatus = YES;                              // At power up force to send the initial state of the Bus output 
                pBusOutput->wLastSentValue = wPad1;
                pBusOutput->Set.byFifoPointer = stChannelBufferRX.byFifoPointer;  // Start analysing the RX command where the fifopointer is now

                // At the first pass, it is not a new status so clear the bit
                PassedStatus.Status.bNeStatus = NO;
                PassedStatus(pBusOutput->Output.Status, &PassedStatus, FORCE);
            }
            else
            {            
                // Pass down the status
                PassedStatus(pBusOutput->Output.Status, &PassedStatus, WRITE);

                // Analyse the receive buffer for a status refresh command of our channel number.  
                // If Yes, we will prepare a refresh command to send
                while(pBusOutput->Set.byFifoPointer != stChannelBufferRX.byFifoPointer)
                {
                    // update pointer to new location 
                    pBusOutput->Set.byFifoPointer++;
                    if(pBusOutput->Set.byFifoPointer >= CHANNEL_RX_BUFFER_FIFO_MAX)
                    {
                        pBusOutput->Set.byFifoPointer = 0;
                    }                

                    // get the channel number
                    ptrRequestChannelData = (RequestChannelData *)&stChannelBufferRX.Array[pBusOutput->Set.byFifoPointer];

                    if (ptrRequestChannelData->MessageID == VB_E_SW_STATUS_REQUEST)             // This Message ID is a channel status request
                    {
                        for(i=0 ; i<4 ; i++)                // Verify the 4 channel of the command
                        {
                            if(pBusInput->wChannel == ptrRequestChannelData->wChannel[i])
                            {
                                ptrRequestChannelData->wChannel[i] = 0;         // Clear the channel number that match
                                wPad2 = ptrRequestChannelData->wSourceAddress;  // Save the Bus address of the module that ask the refresh 
                                bStatus = YES;                                      // Yes, we received a channel status request
                            }
                        }
                    }
                }

            }

            if((wPad1 != pBusOutput->wLastSentValue) || (bStatus == YES))
            {
                // set la valeur de l'input sur l'output et fait la conversion de type
                SetCoreAnyValue(pBusOutput->Output.Status, wPad1);

                // New entry for Channel buffer TX so update fifo pointer
                UpdateChannelBufferTX(pBusOutput->wChannel);

                // pointer on data to transmit
                pBusOutChannelData = (BusOutChannelData *)&stChannelBufferTX.Array[stChannelBufferTX.byInputFifoPointer];


                // First time we execute this formula so send the initial state of this Bus output
                if(pBusOutput->Set.bPassFirstRun == NO)
                {
                    pBusOutput->Set.bPassFirstRun = YES;
                    pBusOutChannelData->MessageID = VB_E_BUS_OUTPUT_BROADCAST;        // It's a output status change

                    pBusOutChannelData->TriggerOrBusAddr.Value = POWERUP_INITIAL_STATE;   // Send the pass status         
                    pBusOutChannelData->TriggerOrBusAddr.bFlag = BUS_ADD_TRIG;        // It's a Initial state status
                }
                else if (wPad1 != pBusOutput->wLastSentValue) // Is it a status refresh request or an output status change?
                {
                    pBusOutChannelData->MessageID = VB_E_BUS_OUTPUT_BROADCAST;        // It's a output status change

                    pBusOutChannelData->TriggerOrBusAddr.Value = PassedStatus.Status.Status;  // Send the pass status         
                    pBusOutChannelData->TriggerOrBusAddr.bFlag = TRIGGER_STATUS;  // It's a trigg pass status
                }
                else                
                {
                    pBusOutChannelData->MessageID = VB_E_BUS_OUTPUT_REFRESH;      // It's a status refresh request

                    pBusOutChannelData->TriggerOrBusAddr.Value = wPad2;           // Send Bus address         
                    pBusOutChannelData->TriggerOrBusAddr.bFlag = BUS_ADD_TRIG;        // It's a bus address
                }

                // put data change in the TX channel
                pBusOutChannelData->wData = wPad1;

                // Put the time stamp (System seconds and odd or even minute)
                pBusOutChannelData->DigiplexTimeStamp.bySecond = pSystemDataByte->bySecond;
                pBusOutChannelData->DigiplexTimeStamp.bOddOrEvenMinute = (pSystemDataByte->byMinute & (uint8_t)0x01)? (bool)1 : (bool)0;

                // keep record of this change
                pBusOutput->wLastSentValue = wPad1;
            }
            break;
        }
//*********************************************************************************************
        case 'C':           // Counter (edge trigger)
        {
            pCounter = (Counter *)*pFunction; 

            // get the status of input trigger 
            bStatus = GetCoreAnyValue(pCounter->Set.Status)? (bool)1 : (bool)0;
            
            // do we have programming for a inverted input
            if(pCounter->Set.Inverter == YES)
            {
                wPad1 = (bool)!bStatus;         
            }
    
            // check if we have a trigger for this counter
            if(bStatus == 1)
            {
                // then verify if we have pass from 0 to 1 (edge type)
                if(pCounter->Set.EdgeState == 0)
                {
                    pCounter->Set.EdgeState = 1;  

                    // increment counter
                    wPad2 = GetCoreAnyValue((uint16_t)pCounter->Output.Status) + 1;
                    if(wPad2 == 0)
                    {
                        wPad2 = 0xFFFF;
                    }
                    // if status is a uint8_t than 0xFF will be written and it is OK!!
                    SetCoreAnyValue(pCounter->Output.Status, wPad2);

                    // get the status number to pass down  (set state)
                    if(pCounter->Output.Static == NO)
                    {
                        PassedStatus(pCounter->Set.Status, &PassedStatus, READ);
                    }
                }
            } 
            else
            {
                pCounter->Set.EdgeState = 0;
            }

            // get the status of reset 
            bStatus = GetCoreAnyValue(pCounter->Reset.Status)? (bool)1 : (bool)0;
            
            // do we have programming for a inverted Reset
            if(pCounter->Reset.Inverter == YES)
            {
                bStatus = (bool)!bStatus;           
            }
 
            // reset value to zero if reset is active
            if(bStatus == 1)
            {
                if(pCounter->Reset.EdgeState == 0)
                {
                    // on edge trigger only this flag is set, so in "level mode", as soon reset
                    // drop the output is set           
                    if(pCounter->Reset.bUseEdge == YES)
                    {
                        pCounter->Reset.EdgeState = 1;
                    }
                    
                    SetCoreAnyValue(pCounter->Output.Status, 0);

                    // get the status number to pass down  (reset state)
                    if(pCounter->Output.Static == NO)
                    {
                        PassedStatus(pCounter->Reset.Status, &PassedStatus, READ);
                    }
                }
            }
            else
            {
                pCounter->Reset.EdgeState = 0;
            }                                              

            // check for the override static
            if(pCounter->Output.Static == YES)
            {
                PassedStatus.Status.Status = pCounter->Output.Status;
            }           
    
            // Pass down the status
            PassedStatus(pCounter->Output.Status, &PassedStatus, WRITE);
            break;
        }
//*********************************************************************************************
        case 'D':           // Schedule Day
        {
            pScheduleDay = (ScheduleDay *)*pFunction; 
            
            Pad1 = pSystemDataByte->uDayOfWeek.byBitPosition;

            bStatus = (pScheduleDay->byBitPosition & Pad1) ? (bool)1 : (bool)0;
            SetCoreAnyValue(pScheduleDay->Output.Status, bStatus);

            // this is a static only status
            PassedStatus.Status.Status = (uint16_t)pScheduleDay->Output.Status;
            PassedStatus(pScheduleDay->Output.Status, &PassedStatus, WRITE);
            break;
        }
//*********************************************************************************************
        case 'E':           // Flip-Flop
        {
            pFlipFlop =   (FlipFlop *)*pFunction; 

            // get the status of input trigger 
            bStatus = GetCoreAnyValue(pFlipFlop->Set.Status)? true : false;
            
            // do we have programming for an inverted input
            if(pFlipFlop->Set.Inverter == YES)
            {
                bStatus = (bool)!bStatus;           
            }
    
            // check if we have a trigger for this flip-flop
            if(bStatus == 1)
            {
                // then verify if we have pass from 0 to 1 (edge type)
                if(pFlipFlop->Set.EdgeState == 0)
                {
                    // on edge trigger only this flag is set, so in "level mode", as soon reset
                    // drop the output is set           
                    if(pFlipFlop->Set.bUseEdge == YES)
                    {
                        pFlipFlop->Set.EdgeState = 1;
                    }
                    // set bit according to inverter
                    SetCoreAnyValue(pFlipFlop->Output.Status, (uint16_t)1);

                    // get the status number to pass down  (set state)
                    if(pFlipFlop->Output.Static == NO)
                    {
                        PassedStatus(pFlipFlop->Set.Status, &PassedStatus, READ);
                    }
                }
            }                                        
            else
            {
                pFlipFlop->Set.EdgeState = 0;
            }

            // get the status of reset 
            bStatus = GetCoreAnyValue(pFlipFlop->Reset.Status)? true : false;
            
            // do we have programming for a inverted reset
            if(pFlipFlop->Reset.Inverter == YES)
            {
                bStatus = (bool)!bStatus;           
            }
            
            if(bStatus == 1)
            {
                if(pFlipFlop->Reset.EdgeState == 0)
                {
                    // on edge trigger only this flag is set, so in "level mode", as soon reset
                    // drop the output is set           
                    if(pFlipFlop->Reset.bUseEdge == YES)
                    {
                        pFlipFlop->Reset.EdgeState = 1;
                    }
                    SetCoreBitStatus((uint16_t)pFlipFlop->Output.Status, true);

                    // get the status number to pass down  (reset state)
                    if(pFlipFlop->Output.Static == NO)
                    {
                        PassedStatus(pFlipFlop->Reset.Status, &PassedStatus, READ);
                    }
                }
            }
            else
            {
                pFlipFlop->Reset.EdgeState = 0;
            }

            // check for the override static
            if(pFlipFlop->Output.Static == YES)
            {
                PassedStatus.Status.Status = (uint16_t)pFlipFlop->Output.Status;
            }           
    
            // Pass down the status
            PassedStatus(pFlipFlop->Output.Status, &PassedStatus, WRITE);
            break;
        }
//*********************************************************************************************
        case 'F':
        {
            // make sure recursivity level is at zero
            RecursivityLevel = 0;

            ProcessEquation(&pStrategy);
            break;
        }
//*********************************************************************************************
        case 'G':           // follower or inverser
        {
            pGate = (Gate *)*pFunction; 

            // get the status of input trigger 
            bStatus = GetCoreAnyValue(pGate->Set.Status)? true : false;
            
            // do we have programming for a inverted input
            if(pGate->Set.Inverter == YES)
            {
                bStatus = (bool)!bStatus;           
            }
    
            // is it a one shot trigger mode
            if(pGate->Set.bOneShot == YES)
            {
                if(bStatus == 1)
                {
                    if(pGate->Set.EdgeState == 0)
                    {
                        pGate->Set.EdgeState = 1;
                    }
                    else
                    {
                        bStatus = 0;            
                    }
                }
                else
                {
                    pGate->Set.EdgeState = 0;
                }
            }
            
            // do we have programming for a inverted output
            if(pGate->Set.InverterOnOutput == YES)
            {
                bStatus = (bool)!bStatus;           
            }
    
            // set output status
            SetCoreAnyValue(pGate->Output.Status, bStatus);

            // check for the override static
            if(pGate->Output.Static == YES)
            {
                PassedStatus.Status.Status = (uint16_t)pGate->Output.Status;
            }           
            else
            {
                PassedStatus(pGate->Set.Status, &PassedStatus, READ);
            }
    
            // Transfert Status trigger
            PassedStatus(pGate->Output.Status, &PassedStatus, WRITE);
            break;
        }
//*********************************************************************************************
        case 'H':           // Schedule time
        {
            pScheduleTime = (ScheduleTime *)*pFunction; 

            // check if we have a boundary cross with the start versus end
            if(pScheduleTime->EndTimeInMinute > pScheduleTime->StartTimeInMinute)
            {   // no boundary cross

                // check range
                if((pSystemDataWord->MinuteOfDay >= pScheduleTime->StartTimeInMinute) &&
                   (pSystemDataWord->MinuteOfDay <  pScheduleTime->EndTimeInMinute))
                {
                    bStatus = (bool)!bStatus;
                }
            }
            else
            {   // boundary cross

                    // check range
                if( (pSystemDataWord->MinuteOfDay >= pScheduleTime->StartTimeInMinute) ||
                    (pSystemDataWord->MinuteOfDay < pScheduleTime->EndTimeInMinute) )
                {
                    bStatus = (bool)!bStatus;
                }
            }

            // set bit according to inverter
            SetCoreAnyValue(pScheduleTime->Output.Status, bStatus);

            // Transfert Status trigger
            PassedStatus.Status.Status = (uint16_t)pScheduleTime->Output.Status;
            PassedStatus(pScheduleTime->Output.Status, &PassedStatus, WRITE);
            break;
        }
        case '[':           // Schedule time + day
        {
            pScheduleTimeDay = (ScheduleTimeDay *)*pFunction; 

            // check if we have a boundary cross with the start versus end
            if(pScheduleTimeDay->EndTimeInMinute > pScheduleTimeDay->StartTimeInMinute)
            {   // no boundary cross

                // check range
                if((pSystemDataWord->MinuteOfDay >= pScheduleTimeDay->StartTimeInMinute) &&
                   (pSystemDataWord->MinuteOfDay <  pScheduleTimeDay->EndTimeInMinute))
                {
                    // we use day scheduling, are we on a valid day for no boundary cross
                    if((pScheduleTimeDay->BitPosition & pSystemDataByte->uDayOfWeek.BitPosition) != 0)
                    {
                        bStatus = (bool)!bStatus;
                    }
                }
            }
            else
            {   // boundary cross
                Pad1 = pSystemDataByte->uDayOfWeek.BitPosition;
                
                // create a valid bit position uint8_t for next day on boundary cross
                Pad2 = (Pad1 & (uint8_t)0x7F) >> 1;              // shift all day
                Pad2 |= (Pad1 & 0x80);                  // get the holiday back
                Pad2 |= (Pad1 & 0x01) ? 0x40 : 0x00;    // shift the bit for saturday on sunday
                
                // check range
                if(pSystemDataWord->MinuteOfDay >= pScheduleTimeDay->StartTimeInMinute)
                {
                    // we use day scheduling, are we on a valid day for boundary cross
                    if((pScheduleTimeDay->BitPosition & Pad1) != 0)
                    {
                        bStatus = (bool)!bStatus;
                    }
                }
                else if(pSystemDataWord->MinuteOfDay < pScheduleTimeDay->EndTimeInMinute)
                {
                    // we use day scheduling, are we on a valid day for boundary cross
                    if((pScheduleTimeDay->BitPosition & Pad2) != 0)
                    {
                        bStatus = (bool)!bStatus;
                    }
                }
            }

            // set bit according to inverter
            SetCoreAnyValue(pScheduleTimeDay->Output.Status, bStatus);

            // Transfert Status trigger
            PassedStatus.Status.Status = (uint16_t)pScheduleTimeDay->Output.Status;
            PassedStatus(pScheduleTimeDay->Output.Status, &PassedStatus, WRITE);
            break;
        }
//*********************************************************************************************
        case 'J':           // Selector
        {
            pSelector = (Selector *)*pFunction; 

            if(GetCoreBitStatus((uint16_t)pSelector->Set.Status) != 1)
            {
                if(pSelector->Set.StatusConstantS1 == TYPE_CONSTANT)
                {
                    Pad1 = pSelector->S1.Constant;
                    PassedStatus.Status.Status = pSelector->Output.Status;
                }
                else
                {
                    Pad1 = GetCoreAnyValue(pSelector->S1.S.Status);
                    PassedStatus(pSelector->S1.S.Status, &PassedStatus, READ);
                }
            }
            else
            {
                if(pSelector->Set.StatusConstantS2 == TYPE_CONSTANT)
                {
                    Pad1 = pSelector->S2.wConstant;
                    PassedStatus.Status.Status = pSelector->Output.Status;
                }
                else
                {
                    Pad1 = GetCoreAnyValue(pSelector->S2.S.Status);
                    PassedStatus(pSelector->S2.S.Status, &PassedStatus, READ);
                }
            }

            // check for the override static
            if(pSelector->Output.Static == YES)
            {
                PassedStatus.Status.Status = (uint16_t)pSelector->Output.Status;
            }           

            PassedStatus(pSelector->Output.Status, &PassedStatus, WRITE);
            SetCoreAnyValue(pSelector->Output.Status, wPad1);
            break;
        }
//*********************************************************************************************
        case 'K':           // Interpretor double
        {
            pInterpretorDouble = (InterpretorDouble *)*pFunction; 

            // get the input
            Pad1 = GetCoreAnyValue((uint16_t)pInterpretorDouble->Input1);
            Pad2 = GetCoreAnyValue((uint16_t)pInterpretorDouble->Input2);

            if((Pad1 != 0) || (Pad2 != 0))
            {
                switch(pInterpretorDouble->Configuration.bOptionOR)
                {
                    case ITPT_LAST_IN:
                    {
                        Pad3 = GetCoreAnyValue(pInterpretorDouble->Output.Status);

                        if(Pad1 != pInterpretorDouble->LastInput1)
                        {
                            // keep a copy of last input value
                            pInterpretorDouble->wLastInput1 = Pad1;
                            if(Pad1 != 0)
                            {
                                Pad3 = Pad1;              // input 1
                                bStatus = 0;                // pass status input 1
                            }
                        } 
                        if(Pad2 != pInterpretorDouble->LastInput2)
                        {
                            // keep a copy of last input value
                            pInterpretorDouble->LastInput2 = Pad2;
                            if(Pad2 != 0)
                            {
                                wPad3 = Pad2;              // input 2
                                bStatus = 1;                // pass status input 2
                            }
                        } 
                        break;
                    }
                    case ITPT_HIGHER:
                    {
                        // if input 1 is higher than input 2
                        if(wPad1 >= wPad2)
                        {
                            wPad3 = wPad1;              // input 1
                            bStatus = 0;                // pass status input 1
                        }
                        // if input 2 is lower than input 1
                        else
                        {
                            wPad3 = wPad2;              // input 2
                            bStatus = 1;                // pass status input 2
                        }
                        break;
                    }
                    case ITPT_LOWER:
                    {
                        // if input 1 is lower than input 2
                        if(Pad1 <= Pad2)
                        {
                            // of course input 1 should not be zero
                            if(Pad1 != 0)
                            {
                                Pad3 = Pad1;              // input 1
                                bStatus = 0;                // pass status input 1
                            }
                            else
                            {
                                Pad3 = Pad2;              // input 2
                                bStatus = 1;                // pass status input 2
                            }
                        }
                        // if input 2 is lower than input 2
                        else
                        {
                            // of course input 1 should not be zero
                            if(Pad2 != 0)
                            {
                                Pad3 = Pad2;              // input 2
                                bStatus = 1;                // pass status input 2
                            }
                            else
                            {
                                Pad3 = Pad1;              // input 1
                                bStatus = 0;                // pass status input 1
                            }
                        }
                        break;
                    }
                    case ITPT_INPUT_1:
                    {
                        if(wPad1 != 0)
                        {
                            Pad3 = Pad1;              // input 1
                            bStatus = 0;                // pass status input 1
                        }
                        else
                        {
                            Pad3 = Pad2;              // input 2
                            bStatus = 1;                // pass status input 2
                        }
                        break;
                    }
                    case ITPT_INPUT_2:
                    {
                        if(Pad2 != 0)
                        {
                            Pad3 = Pad2;              // input 2
                            bStatus = 1;                // pass status input 2
                        }
                        else
                        {
                            Pad3 = Pad1;              // input 1
                            bStatus = 0;                // pass status input 1
                        }
                        break;
                    }
                }
            }

            if(((Pad1 == 0xFF)   && (pInterpretorDouble->Output.Status <  CORE_STATUS_WORD_OFFSET)) ||
               ((Pad1 == 0xFFFF) && (pInterpretorDouble->Output.Status >= CORE_STATUS_WORD_OFFSET)))
            {
                Pad3 = (uint16_t)pInterpretorDouble->wDefaultValue;
            }


            SetCoreAnyValue(pInterpretorDouble->Output.Status, Pad3);

            // check for the override static
            if(pInterpretorDouble->Output.Static == YES)
            {
                PassedStatus.Status.Status = uint16_t(pInterpretorDouble->Output.Status);
            }           
            else
            {
                // get pass status of the input
                PassedStatus((bStatus == 0) ? pInterpretorDouble->Input1 : pInterpretorDouble->Input2, &PassedStatus, READ);
            }

            // pass down the status
            PassedStatus(pInterpretorDouble->Output.Status, &PassedStatus, WRITE);
            break;
        }
//*********************************************************************************************
        case 'L':           // Interpretor single
        {
            pInterpretorSingle = (InterpretorSingle *)*pFunction; 
            
            Pad1 = GetCoreAnyValue(pInterpretorSingle->Input.Status);
            
            if(pInterpretorSingle->Input.Status >= CORE_STATUS_BYTE_OFFSET)          
            {
                if(((Pad1 == 0xFF)   && (pInterpretorSingle->Input.Status <  CORE_STATUS_WORD_OFFSET)) ||
                   ((Pad1 == 0xFFFF) && (pInterpretorSingle->Input.Status >= CORE_STATUS_WORD_OFFSET)))
                {
                    Pad1 = (uint16_t)pInterpretorSingle->DefaultValue;
                }
            }

            SetCoreAnyValue(pInterpretorSingle->Output.Status, Pad1);

            // check for the override static
            if(pInterpretorSingle->Output.Static == YES)
            {
                PassedStatus.Status.Status = pInterpretorSingle->Output.Status;
            }           
            else
            {
                PassedStatus(pInterpretorSingle->Input.Status, &PassedStatus, READ);
            }

            // pass down the status
            PassedStatus(pInterpretorSingle->Output.Status, &PassedStatus, WRITE);
            break;
        }
//*********************************************************************************************
        case 'M':           // Comparator
        {
            pComparator = (Comparator *)*pFunction; 

            if(pComparator->Configuration.StatusConstantS1 == TYPE_CONSTANT)
            {
                Pad1 = pComparator->S1.Constant;
                Pad3 = pComparator->Output.Status;
            }
            else
            {
                Pad1 = GetCoreAnyValue(pComparator->S1.S.Status);
                PassedStatus(pComparator->S1.S.Status, (PastStatus *)&Pad3, READ);
            }

            if(pComparator->Configuration.StatusConstantS2 == TYPE_CONSTANT)
            {
                Pad2 = pComparator->S2.Constant;
                PassedStatus.Status.Status = pComparator->Output.Status;
            }
            else
            {
                Pad2 = GetCoreAnyValue(pComparator->S2.S.Status);
                PassedStatus(pComparator->S1.S.Status, &PassedStatus, READ);
            }

            if(pComparator->Configuration.bSmallerGreater == GREATER)
            {
                if(Pad1 > Pad2)
                {
                    Pad2 = Pad1;
                    PassedStatus.Status.Status = Pad3;
                }
            }
            else
            {
                if(Pad1 < Pad2)
                {
                    Pad2 = Pad1;
                    PassedStatus.Status.Status = Pad3;
                }
            }

            SetCoreAnyValue(pComparator->Output.Status, wPad2);

            // check for the override static
            if(pComparator->Output.Static == YES)
            {
                PassedStatus.Status.Status = pComparator->Output.Status;
            }           

            // pass down the status
            PassedStatus(pComparator->Output.Status, &PassedStatus, WRITE);
            break;
        }
//*********************************************************************************************
        case 'P':           // priority manager
        {
            ptrPriority =   (Priority *)*pFunction; 

// discuter mais pas implementer!!
//option    dernier rentrer
//          plus grand
//          plus petit
//          priority actuelle est sur le ENABLE
                      
            
            for(i=0; i<4; i++)
            {
                // is this priority is active
                if(GetCoreAnyValue(ptrPriority->wSetState[i]) != 0)
                {
                    // get the value of this priority
                    wPad1 = GetCoreAnyValue(ptrPriority->wSet[i]);
                    
                    // send it to the output
                    SetCoreAnyValue(ptrPriority->wSet[i], wPad1);

                    // we have an active priority
                    SetCoreAnyValue(ptrPriority->wOutputState, 1);

                    // read the status to pass down
                    PassedStatus(ptrPriority->wSet[i], &PassedStatus, READ);

                    // break this loop we have our priority
                    break;
                }
            }
            
            if(i == 4)
            {
                SetCoreAnyValue(ptrPriority->Output.Status, 0);
                SetCoreAnyValue(ptrPriority->wOutputState, 0);
            }

            // if no input pass to output, the pass this status to next function
            // or it is static
            if((ptrPriority->Output.Static == YES) || (i == 4))
            {
                PassedStatus.Status.Status = ptrPriority->Output.Status;
            }           

            PassedStatus(ptrPriority->Output.Status, &PassedStatus, WRITE);
            break;
        }
//*********************************************************************************************
        case 'R':           // Random Cycle
        {
            ptrRandomCycle = (RandomCycle *)*pFunction; 
            
            wPad4 = GetCoreAnyValue(ptrRandomCycle->Set.Status);

            if(wPad4 != 0)
            {
                // at the first cycle we need to select ON or OFF cycle at random
                if(ptrRandomCycle->Set.bNewStart == YES)
                {
                    ptrRandomCycle->Set.bCycleLevel = (bool)pSystemDataBit->Flag.bRandomFastToggle;
                    ptrRandomCycle->Set.bNewStart = NO;
                }

                // if we are not in the middle of a ON/OFF cycle we calculate a new one
                // and startit,
                if(ptrRandomCycle->Set.bInCycle == NO)
                {
                    if(ptrRandomCycle->Set.bCycleLevel == 0)
                    {
                        wPad1 = ptrRandomCycle->byMinimumOff;
                        wPad2 = ptrRandomCycle->byMaximumOff;
                    }
                    else
                    {
                        wPad1 = ptrRandomCycle->byMinimumOn;
                        wPad2 = ptrRandomCycle->byMaximumOn;
                    }
        
                    // get a random between the min and max
                    wPad3 = Random(wPad1, wPad2);
                    
                    // (is time left is more than minimum random value) or
                    // (in Bit status no time left check)

                    if((wPad4 > wPad1) || (ptrRandomCycle->Set.Status < CORE_SIZE_STATUS_BIT))
                    {
                        // if random is bigger than the time left
                        if(ptrRandomCycle->Set.Status >= CORE_SIZE_STATUS_BIT)
                        {
                            if(wPad3 > wPad4)
                            {
                                // cycle take what is left
                                wPad3 = wPad4;    
                            }            
                        }
                        
                        // YES -> than do this new cycle
                        bStatus = (bool)ptrRandomCycle->Set.bCycleLevel;
                
                        // Invert level for the next cycle
                        ptrRandomCycle->Set.bCycleLevel = (bool)!bStatus;

                        // calculate the time stamp for this new value
                        ptrRandomCycle->dwSetStamp = dwTimer600ms >> 1;
                        ptrRandomCycle->dwSetStamp += (uint32_t)wPad3 * 1;
                        ptrRandomCycle->Set.bInCycle = YES;
                    }
                    else
                    {
                        // NO -> stop any new cycle
                        bStatus = (bool)0;
                        ptrRandomCycle->Set.bInCycle = NO;
                    }
                    SetCoreAnyValue(ptrRandomCycle->Output.Status, bStatus);
                }
                // check the time stamp to end cycle
                else
                {
                    if(ptrRandomCycle->dwSetStamp <= (dwTimer600ms >> 1))
                    {
                        ptrRandomCycle->Set.bInCycle = NO;
                    }
                }
            }
            else
            {
                // no cycle, then turn OFF
                SetCoreAnyValue(ptrRandomCycle->Output.Status, 0);

                // reset flag for new cycle
                ptrRandomCycle->Set.bNewStart = YES;
            }

            if(ptrRandomCycle->Output.Static == YES)
            {
                PassedStatus.Status.Status = ptrRandomCycle->Output.Status;
            }           
            else
            {
                PassedStatus(ptrRandomCycle->Set.Status, &PassedStatus, READ);
            }

            PassedStatus(ptrRandomCycle->Output.Status, &PassedStatus, WRITE);
            break;
        }
//*********************************************************************************************
        case 'S':           // SunRise and Sunset
                            // this function can only be a static refresh
        {
            pSunRiseSet = (SunRiseSet *)*pFunction; 
            
            if (pSunRiseSet->Config.bRiseSet == SSR_TYPE_RISE)
            {
                wPad1 = pSystemDataWord->wSunRise;
            }           
            else // SSR_TYPE_SET
            {
                wPad1 = pSystemDataWord->wSunSet;
            }

            if(pSunRiseSet->Config.bBeforeAfter == SSR_TRIGGER_BEFORE)
            {
                wPad1 -= (uint16_t)pSunRiseSet->byMinute;
            }
            else // SSR_TRIGGER_AFTER
            {
                wPad1 += (uint16_t)pSunRiseSet->byMinute;
            }

            bStatus = (pSystemDataWord->wMinuteOfDay == wPad1) ? (bool)1:(bool)0;
            SetCoreAnyValue(pSunRiseSet->Output.Status, bStatus);

            // this is a static only status
            PassedStatus.Status.Status = pSunRiseSet->Output.Status;
            PassedStatus(pSunRiseSet->Output.Status, &PassedStatus, WRITE);
            break;
        }
//*********************************************************************************************
        case 'T':           // Toggle
        {
            ptrToggle = (Toggle *)*pFunction; 

            // get the status of input trigger 
            bStatus = GetCoreAnyValue(ptrToggle->Set.Status) ? (bool)1: (bool)0;
 
            // do we have programming for a inverted input
            if(ptrToggle->Set.Inverter == YES)
            {
                bStatus = (bool)!bStatus;           
            }

            // check if we have a trigger for this Toggle
            if(bStatus == 1)
            {
                // then verify if we have pass from 0 to 1 (edge type)
                if(ptrToggle->Set.EdgeState == 0)
                {
                    ptrToggle->Set.EdgeState = 1;
                    
                    bStatus = GetCoreAnyValue(ptrToggle->Output.Status) ? (bool)0: (bool)1;
                    SetCoreAnyValue(ptrToggle->Output.Status, bStatus);
                    PassedStatus(ptrToggle->Set.Status, &PassedStatus, READ);
                }
            } 
            else
            {
                ptrToggle->Set.EdgeState = 0;
            }

            // get the status of Reset
            bStatus = GetCoreAnyValue(ptrToggle->Reset.Status) ? (bool)1: (bool)0;

            // do we have programming for a inverted input
            if(ptrToggle->Reset.Inverter == YES)
            {
                bStatus = (bool)!bStatus;           
            }

            if(bStatus == 1)
            {
                if(ptrToggle->Reset.EdgeState == 0)
                {
                    // on edge trigger only this flag is set, so in "level mode", as soon reset
                    // drop the output is set           
                    if(ptrToggle->Reset.bUseEdge == YES)
                    {
                        ptrToggle->Reset.EdgeState = 1;
                    }
                    PassedStatus(ptrToggle->Reset.Status, &PassedStatus, READ);
                    SetCoreAnyValue(ptrToggle->Output.Status, 0);
                }
            }
            else
            {
                ptrToggle->Reset.EdgeState = 0;
            }

            if(ptrToggle->Output.Static == YES)
            {
                PassedStatus.Status.Status = ptrToggle->Output.Status;
            }   

            // pass down the status
            PassedStatus(ptrToggle->Output.Status, &PassedStatus, WRITE);
            break;
        }
//*********************************************************************************************
        case 'U':           // CountDown

// Status trigger to do

        {
// this fonction is not working it is in development stage now

            pCountDown = (CountDown *)*pFunction; 

//          if(pCountDown->Set.bTimerOut == NO)         // wrong!!!
            {                                                               
                if(pCountDown->Reset.bTimerRunning == NO)
                {
                    // get the status of input trigger 
                    bStatus = GetCoreBitStatus((uint16_t)pCountDown->Set.Status);
            
                    // do we have programming for a inverted input
                    if(pCountDown->Set.Inverter == YES)
                    {
                        bStatus = (bool)!bStatus;           
                    }
    
                    // check if we have a trigger for this timer
                    if(bStatus == 1)
                    {
                        // then verify if we have pass from 0 to 1
                        if(pCountDown->Set.EdgeState == 0)
                        {
                            pCountDown->Set.EdgeState = 1;    
                            pCountDown->Reset.bTimerRunning = YES;

                            // get copy of second or minute to compare if it change
                            if(pCountDown->Reset.bMinuteSecondeType == TYPE_SECOND)
                            {
                                pCountDown->bySetStamp = pSystemDataByte->bySecond;
                            }
                            else
                            {
                                pCountDown->bySetStamp = pSystemDataByte->byMinute;
                            } 
                            
                            if(pCountDown->Set.StatusConstantValue == TYPE_CONSTANT)
                            {
                                wPad1 = pCountDown->Value.Constant;
                            }                                
                            else
                            {
                                wPad1 = GetCoreAnyValue((uint16_t)pCountDown->Value.S.Status);
                            }
                            SetCoreAnyValue(pCountDown->Output.Status, wPad1);
                        }
                    } 
                //    else // timer is running
                //    {
                //        pCountDown->Set.EdgeState = 0;
                //    }
                }
                else
                {                                                               
                    if(pCountDown->Reset.bMinuteSecondeType == TYPE_SECOND)
                    {
                        if(pCountDown->bySetStamp != pSystemDataByte->bySecond)
                        {
                            Pad1 = GetCoreByteValue(pCountDown->Output.Status);
                            if(Pad1)
                            {
                                Pad1--;
                                SetCoreByteValue(pCountDown->Output.Status, Pad1);
                            }
                        }                            
                    }
                    else
                    {
                        if(pCountDown->bySetStamp != pSystemDataByte->byMinute)
                        {
                            wPad1 = GetCoreWordValue(pCountDown->Output.Status);
                            if(wPad1)
                            {
                                wPad1--;
                                SetCoreWordValue(pCountDown->Output.Status, wPad1);
                            }
                        }
                    } 
                }
            }
            break;
        }
//*********************************************************************************************
        // this timer is restarted at each edge trigger
        case 'Q':
        case 'X':
        case 'Y':
        case 'Z':

        {
            ptrTimer = (Timer *)*pFunction; 

            if(ptrTimer->Set.bTimerOut == NO)
            {                                                               
                if(ptrTimer->Set.bTimerRunning == YES)
                {                                                               
                    if((ptrTimer->byFunctionType == 'X') || (ptrTimer->byFunctionType == 'Z'))
                    {
                        dwPad = dwTimer10ms;
                    }
                    else
                    {  // Y et Q 
                        dwPad = dwTimer600ms;
                    }
    
                    if((dwPad >> 1) >= ptrTimer->dwSetStamp)
                    {
                        if(ptrTimer->Reset.bBoundaryCheck == (bool)(dwPad & 0x00000001))
                        {
                            ptrTimer->Set.bTimerRunning = NO;
                            ptrTimer->Set.bTimerOut = YES;
                            bStatus = (bool)!GetCoreBitStatus((uint16_t)ptrTimer->Output.Status);
                            SetCoreBitStatus((uint16_t)ptrTimer->Output.Status, (bool)bStatus);
                        }
                    }
                }
                else
                {
                    // get the status of input trigger 
                    bStatus = GetCoreBitStatus((uint16_t)ptrTimer->Set.Status);
            
                    // do we have programming for a inverted input
                    if(ptrTimer->Set.Inverter == YES)
                    {
                        bStatus = (bool)!bStatus;           
                    }
    
                    // check if we have a trigger for this timer
                    if(bStatus == 1)
                    {
                        // then verify if we have pass from 0 to 1
                        if(ptrTimer->Set.EdgeState == 0)
                        {
                            ptrTimer->Set.EdgeState = 1;    
                            ptrTimer->Set.bTimerRunning = YES;

                            if((ptrTimer->byFunctionType == 'Q') || (ptrTimer->byFunctionType == 'Z'))
                            {   
                                SetCoreBitStatus((uint16_t)ptrTimer->Output.Status, (bool)1);
                            }
                            else
                            {
                                SetCoreBitStatus((uint16_t)ptrTimer->Output.Status, (bool)0);
                            }

                            // copy of timer, because of the interrupt that can change
                            // status of the flag inside this timer
                            if((ptrTimer->byFunctionType == 'X') || (ptrTimer->byFunctionType == 'Z'))
                            {   
                                dwPad = dwTimer10ms;    // 1/100 of a second
                            }
                            else
                            {  // Y et Q 
                                dwPad = dwTimer600ms;   // 1/100 of a minute
                            }
                            ptrTimer->dwSetStamp = dwPad >> 1;

                            // get the preset value
                            if(ptrTimer->Reset.bPresetConstant == YES)
                            {
                                wPad1 = ptrTimer->wPreset;
                            }
                            else
                            {
                                if(ptrTimer->Set.bTimerRange == STATUS_uint8_t)
                                {
                                    wPad1 = (uint16_t)GetCoreByteValue((uint16_t)ptrTimer->wPreset);
                                }
                                else
                                {
                                    wPad1 = GetCoreWordValue((uint16_t)ptrTimer->wPreset);
                                }
                            }

                            // get the percentage value
                            if(ptrTimer->Reset.bPercentConstant == YES)
                            {
                                ptrTimer->dwSetStamp += (uint32_t)wPad1 * (uint32_t)ptrTimer->byPercent;
                            }
                            else
                            {
                                ptrTimer->dwSetStamp += (uint32_t)wPad1 * (uint32_t)GetCoreByteValue((uint16_t)ptrTimer->byPercent);
                            }

        // the boundary check has not been tested yet
                            ptrTimer->Reset.bBoundaryCheck = (dwPad & 0x00000001) ? 1:0;

                            // on a boundary cross over the bit is reverse
                            if(ptrTimer->dwSetStamp < (dwPad >> 1))
                            {
                                 ptrTimer->Reset.bBoundaryCheck= !ptrTimer->Reset.bBoundaryCheck;
                            }
                        }
                    } 
                    else                          
                    {
                        ptrTimer->Set.EdgeState = 0;
                    }
                }
            }

            // get the status of Reset
            bStatus = GetCoreBitStatus((uint16_t)ptrTimer->Reset.Status);
    
            // reset value to zero (inverter status) if reset input is active
            if(bStatus == 1)
            {
                if(ptrTimer->Reset.EdgeState == 0)
                {
                    if(ptrTimer->Reset.bUseEdge == YES)
                    {
                        ptrTimer->Reset.EdgeState = 1;  
                    }
                    ptrTimer->Set.bTimerOut = NO;
                    ptrTimer->Set.bTimerRunning = NO;
                    SetCoreBitStatus((uint16_t)ptrTimer->Output.Status, (bool)0);
                }
            }
            else
            {
                ptrTimer->Reset.EdgeState = 0;  
            }
            break;
        }
//*********************************************************************************************
        case 'V':           // Event one shot
        {
            pEventCycle = (EventCycle *)*pFunction; 
            pEventRAM = (EventRAM *)((uint8_t*)&pEventCycle->byFunctionType + pEventCycle->byFunctNmbrByte); 

            // by default we clear the bit in status            
            SetCoreAnyValue(pEventCycle->Output.Status, 0);

            while(pEventRAM->byFifoPointer != stEventBuffer.byFifoPointer)
            {
                pEventBuffer = (EventData *)&stEventBuffer.Array[pEventRAM->byFifoPointer];

                if(pEventCycle->byGroup == pEventBuffer->byGroup)
                {
/*                    if(pEventCycle->byPartition == pEventBuffer->byPartition)
                    {
                        if(pEventCycle->wPrimaryNumber == pEventBuffer->PrimaryNumber.w)
                        {
                            if(pEventCycle->wSecondaryNumber == pEventBuffer->SecondaryNumber.w)
                            {
                                SetCoreAnyValue(pEventCycle->Output.Status, 1);
                            }
                        }
                    }
*/                }            

                pEventRAM->byFifoPointer++;
                if(pEventRAM->byFifoPointer >= EVENT_BUFFER_FIFO_MAX)
                {
                    pEventRAM->byFifoPointer = 0;
                }                
            }

            // this is a static only status
            PassedStatus.Status.Status = pEventCycle->Output.Status;
            PassedStatus(pEventCycle->Output.Status, &PassedStatus, WRITE);

            break;
        }
//*********************************************************************************************
        case 'W':           // Event on/off
        {
            pEventOnOff = (EventOnOff *)*pFunction; 
            pEventRAM = (EventRAM *)((uint8_t*)&pEventOnOff->byFunctionType + pEventOnOff->byFunctNmbrByte); 

            while(pEventRAM->byFifoPointer != stEventBuffer.byFifoPointer)
            {
                pEventBuffer = (EventData *)&stEventBuffer.Array[pEventRAM->byFifoPointer];

                if(pEventOnOff->byGroupOn == pEventBuffer->byGroup)
                {
/*                    if(pEventOnOff->byPartitionOn == pEventBuffer->byPartition)
                    {
                        if(pEventOnOff->wPrimaryNumberOn == pEventBuffer->PrimaryNumber.w)
                        {
                            if(pEventOnOff->wSecondaryNumberOn == pEventBuffer->SecondaryNumber.w)
                            {
                                SetCoreAnyValue(pEventOnOff->Output.Status, 0);
                            }
                        }
                    }
*/                }            

                if(pEventOnOff->byGroupOff == pEventBuffer->byGroup)
                {
/*                    if(pEventOnOff->byPartitionOff == pEventBuffer->byPartition)
                    {
                        if(pEventOnOff->wPrimaryNumberOff == pEventBuffer->PrimaryNumber.w)
                        {
                            if(pEventOnOff->wSecondaryNumberOff == pEventBuffer->SecondaryNumber.w)
                            {
                                SetCoreAnyValue(pEventOnOff->Output.Status, 1);
                            }
                        }
                    }
*/                }            

                pEventRAM->byFifoPointer++;
                if(pEventRAM->byFifoPointer >= EVENT_BUFFER_FIFO_MAX)
                {
                    pEventRAM->byFifoPointer = 0;
                }                
            }

            // this is a static only status
            PassedStatus.Status.Status = pEventOnOff->Output.Status;
            PassedStatus(pEventOnOff->Output.Status, &PassedStatus, WRITE);
            break;
        }
//*********************************************************************************************

#endif