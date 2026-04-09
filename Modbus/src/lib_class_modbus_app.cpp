//-------------------------------------------------------------------------------------------------
//
//  File : lib_class_modbus_app.h
//
//-------------------------------------------------------------------------------------------------
//
// Copyright(c) 2026 Alain Royer.
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
//-------------------------------------------------------------------------------------------------
const MODBUS_AppEntry_t* ModbusAPP::Find(uint8_t UnitID, uint8_t Function)
{
    for(size_t index = 0; index < g_ModbusAppTableCount; index++)
    {
        const MODBUS_AppEntry_t* entry = &g_ModbusAppTable[index];

        if(entry->UnitID == UnitID)
        {
            if(entry->Function == Function)
            {
                return entry;
            }
        }
    }

    return 0;
}

bool ModbusAPP::Process(const MODBUS_Command_t& cmd, MODBUS_Response_t& rsp)
{
    const ModbusAppEntry_t* e = Find(cmd.UnitID, cmd.Function);

    if(e == nullptr)
    {
        return false; // Manager renverra exception
    }

    if(e->Passthru != nullptr)
    {
        const ModbusPassthru* pt = (const ModbusPassthru*)e->Passthru;

        MODBUS_Command_t newCmd = cmd;
        newCmd.UnitID = pt->NewUnitID;

        return pt->TargetBackend->Queue(newCmd);
    }

    if(e->Callback != nullptr)
    {
        e->Callback(cmd, rsp);
        return true;
    }

    return false;
}
