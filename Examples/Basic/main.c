#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "InputStream.h"
#include "Str.h"
#include "CmdManager.h"

Cmd_Handled Test_onExe(CmdManager* manager, Cmd* cmd, Cmd_Cursor* cursor, Cmd_Type type);
Cmd_Handled Test_onSet(CmdManager* manager, Cmd* cmd, Cmd_Cursor* cursor, Cmd_Type type);
Cmd_Handled Test_onGet(CmdManager* manager, Cmd* cmd, Cmd_Cursor* cursor, Cmd_Type type);
Cmd_Handled Test_onHelp(CmdManager* manager, Cmd* cmd, Cmd_Cursor* cursor, Cmd_Type type);
Cmd_Handled Test_onResponse(CmdManager* manager, Cmd* cmd, Cmd_Cursor* cursor, Cmd_Type type);
const Cmd CMD_TEST = CMD_INIT("test", Cmd_Type_Any, Test_onExe, Test_onSet, Test_onGet, Test_onHelp, Test_onResponse);

Cmd_Handled Test2_onSet(CmdManager* manager, Cmd* cmd, Cmd_Cursor* cursor, Cmd_Type type);
const Cmd CMD_TEST2 = CMD_INIT("test2", Cmd_Type_Any, NULL, Test2_onSet, NULL, NULL, NULL);


Cmd_Handled Test3_onExe(CmdManager* manager, Cmd* cmd, Cmd_Cursor* cursor, Cmd_Type type);
const Cmd CMD_TEST3 = CMD_INIT("test3", Cmd_Type_Any, Test3_onExe, NULL, NULL, NULL, NULL);

Cmd_Handled Test4_onHelp(CmdManager* manager, Cmd* cmd, Cmd_Cursor* cursor, Cmd_Type type);
const Cmd CMD_TEST4 = CMD_INIT("test4", Cmd_Type_Any, NULL, NULL, NULL, Test4_onHelp, NULL);

Cmd_Handled Test5_onResponse(CmdManager* manager, Cmd* cmd, Cmd_Cursor* cursor, Cmd_Type type);
const Cmd CMD_TEST5 = CMD_INIT("test5", Cmd_Type_Any, NULL, NULL, NULL, NULL, Test5_onResponse);

Cmd_Handled Help_onExecute(CmdManager* manager, Cmd* cmd, Cmd_Cursor* cursor, Cmd_Type type);
const Cmd CMD_HELP = CMD_INIT("help", Cmd_Type_Execute, Help_onExecute, NULL, NULL, NULL, NULL);

Cmd_Handled Exit_onExecute(CmdManager* manager, Cmd* cmd, Cmd_Cursor* cursor, Cmd_Type type);
const Cmd CMD_EXIT = CMD_INIT("exit", Cmd_Type_Execute, Exit_onExecute, NULL, NULL, NULL, NULL);

const Cmd_Array CMDS[] = {
    &CMD_TEST,
    &CMD_TEST2,
    &CMD_TEST3,
    &CMD_TEST4,
    &CMD_TEST5,
    &CMD_HELP,
    &CMD_EXIT,
};
const Mem_LenType CMDS_LEN = CMD_ARR_LEN(CMDS);

void Cmd_onNotFound(char* str);
void Cmd_onOverflow(void);

char tempBuffer[64];
uint8_t streamBuffer[100];
IStream istream;
CmdManager manager;
Cmd_Cursor cursor;

void printStrs(const char** strs, int len) {
    puts("{");
    while (len-- > 0) {
        printf("%s,\n", *strs++);
    }
    puts("}");
}

void printCmd(Cmd* cmd) {
    printf("{{%s, %d}, %X}\n", cmd->CmdName.Name, cmd->CmdName.Len, cmd->Types.Flags);
}

void printCmdStr(Cmd_Str* cmd) {
    printf("{%s, %d}\n", cmd->Name, cmd->Len);
}

void printCmds(Cmd_Array* cmds, int len) {
    puts("Valid Commands:");
    while (len--) {
        printCmd(*cmds++);
    }
}
void printCursor(Cmd_Cursor* cursor) {
    printf("{%d, \"%s\", %u, %c}\n", cursor->Len, cursor->Ptr, cursor->Index, cursor->ParamSeperator);
}
void printCmdParam(Cmd_Param* param) {
    static const char* TYPES[] = {
        "Unknown",
        "Number",
        "NumberHex",
        "NumberBinary",
        "Float",
        "State",
        "StateKey",
        "Boolean",
        "String",
        "Null",
    };

    printf("{%u, %s, ", param->Index, TYPES[(int) param->ValueType]);
    switch (param->ValueType) {
        case Cmd_ValueType_Unknown:
            printf("\"%s\"", param->Value.Unknown);
            break;
        case Cmd_ValueType_Number:
            printf("%d", param->Value.Number);
            break;
        case Cmd_ValueType_NumberHex:
            printf("%X", param->Value.NumberHex);
            break;
        case Cmd_ValueType_NumberBinary:
            printf("%u", param->Value.NumberBinary);
            break;
        case Cmd_ValueType_Float:
            printf("%g", param->Value.Float);
            break;
        case Cmd_ValueType_State:
            printf("%s", param->Value.State ? "HIGH" : "LOW");
            break;
        case Cmd_ValueType_StateKey:
            printf("%s", param->Value.StateKey ? "ON" : "OFF");
            break;
        case Cmd_ValueType_Boolean:
            printf("%s", param->Value.Boolean ? "true" : "false");
            break;
        case Cmd_ValueType_String:
            printf("\"%s\"", param->Value.String);
            break;
        case Cmd_ValueType_Null:
            printf("\"%s\"", param->Value.Null);
            break;
    }
    puts("}");
}

void parseAllParams(Cmd_Cursor* cursor) {
    Cmd_Param param;
    while (CmdManager_nextParam(cursor, &param) != NULL) {
        printCmdParam(&param);
    }
}

static uint8_t needExit = 0;

int main()
{
    CmdManager_init(&manager, CMDS, CMDS_LEN);
    CmdManager_onNotFound(&manager, Cmd_onNotFound);
    CmdManager_onOverflow(&manager, Cmd_onOverflow);

    IStream_init(&istream, NULL, streamBuffer, sizeof(streamBuffer));

    printCmds(CMDS, CMDS_LEN);

    while (1) {
        // read byte form console
        IStream_receiveByte(&istream, (uint8_t) getchar());

        // handle command manager
        //CmdManager_handleStatic(&manager, &istream, tempBuffer, sizeof(tempBuffer), &cursor);
        CmdManager_handle(&manager, &istream);

        // check for end program
        if (needExit) {
            break;
        }
    }
}

Cmd_Handled Test_onExe(CmdManager* manager, Cmd* cmd, Cmd_Cursor* cursor, Cmd_Type type) {
    puts("Test -> Execute");
    return Cmd_Done;
}
Cmd_Handled Test_onSet(CmdManager* manager, Cmd* cmd, Cmd_Cursor* cursor, Cmd_Type type) {
    puts("Test -> Set");
    parseAllParams(cursor);
    return Cmd_Done;
}
Cmd_Handled Test_onGet(CmdManager* manager, Cmd* cmd, Cmd_Cursor* cursor, Cmd_Type type) {
    puts("Test -> Get");
    return Cmd_Done;
}
Cmd_Handled Test_onHelp(CmdManager* manager, Cmd* cmd, Cmd_Cursor* cursor, Cmd_Type type) {
    puts("Test -> Help");
    return Cmd_Done;
}
Cmd_Handled Test_onResponse(CmdManager* manager, Cmd* cmd, Cmd_Cursor* cursor, Cmd_Type type) {
    puts("Test -> Response");
    return Cmd_Done;
}

Cmd_Handled Test2_onSet(CmdManager* manager, Cmd* cmd, Cmd_Cursor* cursor, Cmd_Type type) {
    printf("Test2 -> Set: %s\n", cursor->Ptr);
    parseAllParams(cursor);
    return Cmd_Done;
}

Cmd_Handled Test3_onExe(CmdManager* manager, Cmd* cmd, Cmd_Cursor* cursor, Cmd_Type type) {
    puts("Test3 -> Execute");
    return Cmd_Done;
}

Cmd_Handled Test4_onHelp(CmdManager* manager, Cmd* cmd, Cmd_Cursor* cursor, Cmd_Type type) {
    puts("Test4 -> Help");
    return Cmd_Done;
}

Cmd_Handled Test5_onResponse(CmdManager* manager, Cmd* cmd, Cmd_Cursor* cursor, Cmd_Type type) {
    puts("Test5 -> Response");
    return Cmd_Done;
}

Cmd_Handled Help_onExecute(CmdManager* manager, Cmd* cmd, Cmd_Cursor* cursor, Cmd_Type type) {
    puts("Help -> Execute");
    return Cmd_Done;
}

Cmd_Handled Exit_onExecute(CmdManager* manager, Cmd* cmd, Cmd_Cursor* cursor, Cmd_Type type) {
    puts("EXIT Program");
    needExit = 1;
    return Cmd_Done;
}

void Cmd_onNotFound(char* str) {
    printf("NotFound: \"%s\"\n", str);
}
void Cmd_onOverflow(void) {
    printf("Buffer Overflow\n");
}
