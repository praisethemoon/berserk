/*

     ***** **                                                              *
  ******  ***                                                            **
 **   *  * **                                                            **
*    *  *  **                                                            **
    *  *   *              ***  ****       ****              ***  ****    **
   ** **  *        ***     **** **** *   * **** *    ***     **** **** * **  ***
   ** ** *        * ***     **   ****   **  ****    * ***     **   ****  ** * ***
   ** ***        *   ***    **         ****        *   ***    **         ***   *
   ** ** ***    **    ***   **           ***      **    ***   **         **   *
   ** **   ***  ********    **             ***    ********    **         **  *
   *  **     ** *******     **               ***  *******     **         ** **
      *      ** **          **          ****  **  **          **         ******
  ****     ***  ****    *   ***        * **** *   ****    *   ***        **  ***
 *  ********     *******     ***          ****     *******     ***       **   *** *
*     ****        *****                             *****                 **   ***
*
 **

Written by Soulaymen Chouri
Contact me @ akaisora@tuta.io

Copyright (c) 2016,Soulaymen Chouri (akaisora@tuta.io). All rights reserved.
Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of the author nor the names of other contributors may be used to endorse or promote products derived from this software without specific prior written permission.
THIS SOFTWARE IS PROVIDED BY THE AUTHORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/


#include <cwchar>
#include <cstdio>
#include <cstdlib>

#include "Compiler.h"
#include "Defines.h"
#include "Engine.h"
#include "Parser.h"
#include "FileUtils.h"
#include "lua/lua.hpp"

Compiler::Compiler(const char * filename)
{
    wchar_t *InputBuf;
    TokenStruct *Token;
    Context = new ContextStruct();
    int Result;
    int TrimReductions = 0;
    int Debug = 0;
    /* Load the inputfile into memory. */
    InputBuf = LoadInputFile(filename);
    if (InputBuf == NULL)
        return;

    /* Run the Parser. */
    Result = Parse(InputBuf,wcslen(InputBuf),TrimReductions,Debug,&Token);

    /* Interpret the results. */
    if (Result != PARSEACCEPT) {
        ShowErrorMessage(Token,Result);
      } else {
        printf("Accepted.\n");
        /* Initialize the Context. */
        Context->Debug = Debug;
        Context->Indent = 0;
        Context->ReturnValue = NULL;

        /* Start execution by calling the subroutine of the first Token on
           the TokenStack. It's the "Start Symbol" that is defined in the
           grammar. */
        //RuleJumpTable[Token->ReductionRule](Token, Context);
        lua_State *L = luaL_newstate();
        luaL_openlibs(L);
        registerContext(L, Token);
        lua_setglobal(L, "expr");
        luaL_dostring(L, "print(tab.reductionRule, tab.symbol, tab.data, tab.line, tab.col)");
        }

    /* Cleanup. */
    DeleteTokens(Token);
    free(InputBuf);
}

Compiler::~Compiler()
{
    delete Context;
}

void Compiler::registerContext(lua_State* L, TokenStruct *Token)
{
    lua_checkstack(L, INT_MAX);
    static int i = 0;
    //printf("\nsize: %d, i:%d reduction: %d.\n", Token->Size, i++, Token->ReductionRule);
    lua_newtable(L); /* creates and pushes new table on top of Lua stack */


    lua_pushinteger(L, Token->ReductionRule); /* Pushes table value on top of Lua stack */
    lua_setfield(L, -2, "rule");  /* table["name"] = row->name. Pops key value */

    lua_pushinteger(L, Token->Symbol);
    lua_setfield(L, -2, "sym");

    lua_pushstring(L, (char*)Token->Data);
    lua_setfield(L, -2, "data");

    lua_pushinteger(L, Token->Line);
    lua_setfield(L, -2, "line");

    lua_pushinteger(L, Token->Column);
    lua_setfield(L, -2, "col");



    if(Token->Size > 0)
    {
        lua_newtable(L);

        for(int k = 0; k < Token->Size; k++){
            registerContext(L, Token->Tokens[k]);
            lua_rawseti(L, -2, k+1);
        }

        lua_setfield(L, -2, "tokens");
    }


}
