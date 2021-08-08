#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "InputStream.h"
#include "Str.h"
#include "CmdManager.h"

Cmd_Result Test_onExe(Cmd* cmd, Cmd_Params* params, Cmd_Type type);
Cmd_Result Test_onSet(Cmd* cmd, Cmd_Params* params, Cmd_Type type);
Cmd_Result Test_onGet(Cmd* cmd, Cmd_Params* params, Cmd_Type type);
Cmd_Result Test_onHelp(Cmd* cmd, Cmd_Params* params, Cmd_Type type);
Cmd_Result Test_onResponse(Cmd* cmd, Cmd_Params* params, Cmd_Type type);
const Cmd CMD_TEST = CMD_INIT("test", Cmd_Type_Any, Test_onExe, Test_onSet, Test_onGet, Test_onHelp, Test_onResponse);

Cmd_Result Test2_onSet(Cmd* cmd, Cmd_Params* params, Cmd_Type type);
const Cmd CMD_TEST2 = CMD_INIT("test2", Cmd_Type_Any, NULL, Test2_onSet, NULL, NULL, NULL);


Cmd_Result Test3_onExe(Cmd* cmd, Cmd_Params* params, Cmd_Type type);
const Cmd CMD_TEST3 = CMD_INIT("test3", Cmd_Type_Any, Test3_onExe, NULL, NULL, NULL, NULL);

Cmd_Result Test4_onHelp(Cmd* cmd, Cmd_Params* params, Cmd_Type type);
const Cmd CMD_TEST4 = CMD_INIT("test4", Cmd_Type_Any, NULL, NULL, NULL, Test4_onHelp, NULL);

Cmd_Result Test5_onResponse(Cmd* cmd, Cmd_Params* params, Cmd_Type type);
const Cmd CMD_TEST5 = CMD_INIT("test5", Cmd_Type_Any, NULL, NULL, NULL, NULL, Test5_onResponse);

const Cmd_Array CMDS[] = {
    &CMD_TEST,
    &CMD_TEST2,
    &CMD_TEST3,
    &CMD_TEST4,
    &CMD_TEST5,
};
const Mem_LenType CMDS_LEN = CMD_ARR_LEN(CMDS);

void Cmd_onNotFound(char* str);
void Cmd_onOverflow(void);

char tempBuffer[32];
uint8_t streamBuffer[100];
IStream istream;
CmdManager manager;

void printStrs(const char** strs, int len) {
    puts("{");
    while (len-- > 0) {
        printf("%s,\n", *strs++);
    }
    puts("}");
}

int main()
{
    Str_MultiResult result = {0};

    const char tempStr[] = "test ?1";

    const char* TYPES[] = {
        "?",
        "=?",
        "=",
        ":",
    };
    Str_findStrs(tempStr, TYPES, 4, &result);

    printf("'%s', %d\n", result.IndexOf, result.Position);

    /*CmdManager_init(&manager, CMDS, CMDS_LEN);
    CmdManager_onNotFound(&manager, Cmd_onNotFound);
    CmdManager_onOverflow(&manager, Cmd_onOverflow);

    IStream_init(&istream, NULL, streamBuffer, sizeof(streamBuffer));

    while (1) {
        // read byte form console
        IStream_receiveByte(&istream, (uint8_t) getchar());

        // handle command manager
        CmdManager_handle(&manager, &istream, tempBuffer, sizeof(tempBuffer));
    }*/
}

Cmd_Result Test_onExe(Cmd* cmd, Cmd_Params* params, Cmd_Type type) {
    puts("Test -> Execute");
    return Cmd_Done;
}
Cmd_Result Test_onSet(Cmd* cmd, Cmd_Params* params, Cmd_Type type) {
    puts("Test -> Set");
    return Cmd_Done;
}
Cmd_Result Test_onGet(Cmd* cmd, Cmd_Params* params, Cmd_Type type) {
    puts("Test -> Get");
    return Cmd_Done;
}
Cmd_Result Test_onHelp(Cmd* cmd, Cmd_Params* params, Cmd_Type type) {
    puts("Test -> Help");
    return Cmd_Done;
}
Cmd_Result Test_onResponse(Cmd* cmd, Cmd_Params* params, Cmd_Type type) {
    puts("Test -> Response");
    return Cmd_Done;
}

Cmd_Result Test2_onSet(Cmd* cmd, Cmd_Params* params, Cmd_Type type) {
    puts("Test2 -> Set");
    return Cmd_Done;
}

Cmd_Result Test3_onExe(Cmd* cmd, Cmd_Params* params, Cmd_Type type) {
    puts("Test3 -> Execute");
    return Cmd_Done;
}

Cmd_Result Test4_onHelp(Cmd* cmd, Cmd_Params* params, Cmd_Type type) {
    puts("Test4 -> Help");
    return Cmd_Done;
}

Cmd_Result Test5_onResponse(Cmd* cmd, Cmd_Params* params, Cmd_Type type) {
    puts("Test5 -> Response");
    return Cmd_Done;
}

void Cmd_onNotFound(char* str) {
    printf("NotFound: \"%s\"\n", str);
}
void Cmd_onOverflow(void) {
    printf("Buffer Overflow\n");
}
