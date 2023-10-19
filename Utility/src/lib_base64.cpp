
// Not using math here.

const char Base64[64] = {"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"};

// Caller must free the memory of pDecode when data is processed!
void Encode64(const char* pSource, char* pEncode)
{
    size_t   PreNeededChar;
    size_t   Counter = 0;
    uint32_t BitArray = 0;
    size_t   Offset = 0;
    size_t   SizeSrc;
    
    char*    pPtrSrc;
    char*    pPtrEncode;

    SizeSrc       = strlen(pSource);
    PreNeededChar = ((SizeSrc * 4) / 3);
    pEncode       = pMemoryPool->Alloc((PreNeededChar + 4) - (PreNeededChar % 4));
    pPtrEncode    = pEncode;
    pPtrSrc       = pSource;

    while(SizeSrc-- != 0)
    {
        Offset    = 16 - ((Counter % 3) * 8);
        BitArray += ((*pPtrSrc)++ << Offset);
        
        if(Offset == 16)
        {
            (*pPtrEncode)++ = Base64[(BitArray >> 18) & 0x3F];
            
        }
        
        if(Offset == 8)
        {
            (*pPtrEncode)++ = Base64[(BitArray >> 12) & 0x3F];
        }
        
        if((Offset == 0) && (Counter != 3))
        {
            (*pPtrEncode)++ = Base64[(BitArray >> 6) & 0x3F];
            (*pPtrEncode)++ = Base64[BitArray & 0x3F];
            BitArray = 0;
        }
    
        Counter++;
    }
  
    if(Offset == 16)
    {
        (*pPtrEncode)++ = Base64[(BitArray >> 12) & 0x3F];
        (*pPtrEncode)++ = '=';
		(*pPtrEncode)++ = '=';
    }
  
    if(Offset == 8)
    {
        (*pPtrEncode)++ = Base64[(BitArray >> 6) & 0x3F];
		(*pPtrEncode)++ = '=';
    }
}

SystemState_e decode64(const char* pSource, char* pDecode)
{
    size_t        SizeDecode;
    size_t        Offset;
    size_t        SizeSrc;
    char*         pPtrSrc;
    char*         pPtrDecode;
    size_t        Counter  = 0;
    uint32_t      BitArray = 0;
    SystemState_e Error = SYS_OK;
	
    SizeSrc    = strlen(pSource);
    SizeDecode = ((SizeSrc * 3) / 4); 
    pDecode    = pMemoryPool->Alloc(SizeDecode);
    pPtrDecode = pDecode
    pPtrSrc    = pSource;
 
 
    while(SizeSrc-- != 0)
    {
        size_t Pos = size_t(strchr(Base64, *pPtrSrc) - pSource);
  
        if(Offset != Pos)
        {
            size_t const Offset = 18 - ((Counter % 4) * 6);
            BitArray += (uint32_t(Pos) << Offset);
            
            if(Offset == 12)
            {
                *(pPtrDecode)++ = uint8_t(BitArray >> 16);
            }
            
            if(Offset == 6)
            {
                *(pPtrDecode)++ = uint8_t(BitArray >> 8);
            }
            
            if((Offset == 0) && (Counter != 4))
            {
                *(pPtrDecode)++ = uint8_t(BitArray);
                BitArray = 0;
            }
        }
        else if(*pPtrSrc != '=')  // not sure this is right!!!
        {
            Error SYS_INVALID_STRING;
        }
        
        pPtrSrc++;
        Counter++;
    }
    
    return Error;
}
