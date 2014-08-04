/* 
 * libnpengine: Nitroplus script interpreter
 * Copyright (C) 2014 Mislav Blažević <krofnica996@gmail.com>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * */
#include "NSBContext.hpp"
#include "scriptfile.hpp"
#include <chrono>
using namespace std::chrono;

NSBContext::NSBContext(const string& Name) : Name(Name), WaitTime(0), WaitStart(0), WaitInterrupt(false)
{
}

NSBContext::~NSBContext()
{
}

bool NSBContext::Call(ScriptFile* pScript, const string& Symbol)
{
    uint32_t CodeLine = pScript->GetSymbol(Symbol);
    if (CodeLine == NSB_INVALIDE_LINE)
        return false;

    CallStack.push({pScript, CodeLine - 1});
    return true;
}

void NSBContext::Jump(const string& Symbol)
{
    uint32_t CodeLine = GetScript()->GetSymbol(Symbol);
    if (CodeLine != NSB_INVALIDE_LINE)
        GetFrame()->SourceLine = CodeLine - 1;
}

void NSBContext::Break()
{
    Jump(BreakStack.top());
}

const string& NSBContext::GetScriptName()
{
    return GetScript()->GetName();
}

ScriptFile* NSBContext::GetScript()
{
    return GetFrame()->pScript;
}

Line* NSBContext::GetLine()
{
    return GetScript()->GetLine(GetFrame()->SourceLine);
}

const string& NSBContext::GetParam(uint32_t Index)
{
    return GetLine()->Params[Index];
}

int NSBContext::GetNumParams()
{
    return GetLine()->Params.size();
}

uint32_t NSBContext::GetMagic()
{
    return GetLine()->Magic;
}

NSBContext::StackFrame* NSBContext::GetFrame()
{
    return &CallStack.top();
}

uint32_t NSBContext::Advance()
{
    GetFrame()->SourceLine++;
    return GetMagic();
}

void NSBContext::Return()
{
    CallStack.pop();
}

void NSBContext::PushBreak()
{
    BreakStack.push(GetParam(0));
}

void NSBContext::PopBreak()
{
    BreakStack.pop();
}

void NSBContext::Wait(int32_t Time, bool Interrupt)
{
    WaitInterrupt = Interrupt;
    WaitTime = Time;
    WaitStart = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

void NSBContext::Wake()
{
    WaitTime = 0;
}

void NSBContext::TryWake()
{
    if (WaitInterrupt)
        Wake();
}

bool NSBContext::IsStarving()
{
    return CallStack.empty();
}

bool NSBContext::IsSleeping()
{
    uint64_t Now = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    return (Now - WaitStart) < WaitTime;
}