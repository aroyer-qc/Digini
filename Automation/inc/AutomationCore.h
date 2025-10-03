#pragma once


//------ Function(s) prototype(s) -----------------------------------------------------------------

class AutomationCore
{

 // use damn template?
    template<typename Type>
    Type         GetValue            (uint16_t Position);
    void         SetValue            (uint16_t Position, Type Value);
};

