#include "CmdManager.h"
#include "InputStream.h"
#include "Str.h"
#include <stdint.h>

extern void printCmd(Cmd* cmd);

extern void printCmdStr(Cmd_Str* cmd);

/* public variables */
static const Cmd_Str PATTERN_TYPE_EXE   = CMD_STR_INIT(CMD_DEFAULT_PATTERN_TYPE_EXE);
static const Cmd_Str PATTERN_TYPE_SET   = CMD_STR_INIT(CMD_DEFAULT_PATTERN_TYPE_SET);
static const Cmd_Str PATTERN_TYPE_GET   = CMD_STR_INIT(CMD_DEFAULT_PATTERN_TYPE_GET);
static const Cmd_Str PATTERN_TYPE_HELP  = CMD_STR_INIT(CMD_DEFAULT_PATTERN_TYPE_HELP);
static const Cmd_Str PATTERN_TYPE_RESP  = CMD_STR_INIT(CMD_DEFAULT_PATTERN_TYPE_RESP);
const Cmd_PatternTypes CMD_PATTERN_TYPES = {
    (Cmd_Str*) &PATTERN_TYPE_EXE,
    (Cmd_Str*) &PATTERN_TYPE_SET,
    (Cmd_Str*) &PATTERN_TYPE_GET,
    (Cmd_Str*) &PATTERN_TYPE_HELP,
    (Cmd_Str*) &PATTERN_TYPE_RESP,
};
const Cmd_Str CMD_END_WITH = CMD_STR_INIT(CMD_DEFAULT_END_WITH);
const char CMD_PARAM_SEPERATOR = ',';
/* private defines */
#if CMD_SORT_LIST

    #if CMD_SORT_ALG == CMD_SORT_ALG_SELECTION
        #define __sort          Mem_sort
    #else if CMD_SORT_ALG == CMD_SORT_ALG_QUICK_SORT
        #define __sort          Mem_quickSort
    #endif

    #define __search            Mem_binarySearch

#else
    #define __search            Mem_linearSearch
#endif
/* private variables */

/* private functions */
static char Cmd_compare(const void* itemA, const void* itemB, Mem_LenType itemLen);
static char Cmd_compareName(const void* name, const void* cmd, Mem_LenType itemLen);
static void Cmd_swap(const void* itemA, const void* itemB, Mem_LenType itemLen);
static char CmdType_compare(const void* buff, const void* type, Mem_LenType itemLen);

void Cmd_init(Cmd* cmd, const char* name, Cmd_Type types) {
    cmd->CmdName.Name = name;
    cmd->CmdName.Len = Str_len(name);
    cmd->Types.Flags = (uint8_t) types;
    Mem_set(cmd->Callbacks.fn, 0x00, sizeof(Cmd_Callbacks));
}
void Cmd_setTypes(Cmd* cmd, Cmd_Type types) {
    cmd->Types.Flags = (uint8_t) types;
}
#if CMD_MULTI_CALLBACK
    void Cmd_onExecute(Cmd* cmd, Cmd_CallbackFn fn) {
        cmd->Callbacks.execute = fn;
    }
    void Cmd_onSet(Cmd* cmd, Cmd_CallbackFn fn) {
        cmd->Callbacks.set = fn;
    }
    void Cmd_onGet(Cmd* cmd, Cmd_CallbackFn fn) {
        cmd->Callbacks.get = fn;
    }
    void Cmd_onHelp(Cmd* cmd, Cmd_CallbackFn fn) {
        cmd->Callbacks.help = fn;
    }
    void Cmd_onResponse(Cmd* cmd, Cmd_CallbackFn fn) {
        cmd->Callbacks.response = fn;
    }
#if CMD_UNKNOWN_CALLBACK
    void Cmd_onUnknown(Cmd* cmd, Cmd_CallbackFn fn) {
        cmd->Callbacks.unknown = fn;
    }
#endif // CMD_UNKNOWN_CALLBACK

#else
    void Cmd_on(Cmd* cmd, Cmd_CallbackFn fn) {
        cmd->Callbacks.fn[0] = fn;
    }
#endif // CMD_MULTI_CALLBACK

void CmdManager_init(CmdManager* manager, Cmd* cmds, Cmd_LenType len) {
    manager->PatternTypes = (Cmd_PatternTypes*) &CMD_PATTERN_TYPES;
    manager->StartWith = NULL;
    manager->EndWith = (Cmd_Str*) &CMD_END_WITH;
    manager->List.Cmds = cmds;
    manager->List.Len = len;
    manager->notFound = NULL;
    manager->bufferOverflow = NULL;
    manager->ParamSeperator = CMD_PARAM_SEPERATOR;
    manager->InUseCmd = NULL;
#if CMD_SORT_LIST
    __sort(manager->List.Cmds, manager->List.Len, sizeof(manager->List.Cmds[0]), Cmd_compare, Cmd_swap);
#endif
}
void CmdManager_setStartWith(CmdManager* manager, Cmd_Str* startWith) {
    manager->StartWith = startWith;
}
void CmdManager_setEndWith(CmdManager* manager, Cmd_Str* endWith) {
    manager->EndWith = endWith;
}
void CmdManager_onNotFound(CmdManager* manager, Cmd_NotFoundFn notFound) {
    manager->notFound = notFound;
}
void CmdManager_onOverflow(CmdManager* manager, Cmd_OverflowFn overflow) {
    manager->bufferOverflow = overflow;
}
void CmdManager_setParamSeperator(CmdManager* manager, char sep) {
    manager->ParamSeperator = sep;
}
void CmdManager_setCommands(CmdManager* manager, Cmd* cmds, Cmd_LenType len) {
    manager->List.Cmds = cmds;
    manager->List.Len = len;
#if CMD_SORT_LIST
    __sort(manager->List.Cmds, manager->List.Len, sizeof(manager->List.Cmds[0]), Cmd_compare, Cmd_swap);
#endif
}

void CmdManager_handle(CmdManager* manager, IStream* stream, char* buffer, Str_LenType len) {
    if (IStream_available(stream) > 0) {
        Cmd_Str cmdStr;
        char* pBuff = buffer;
        Stream_LenType lineLen = IStream_readBytesUntilPattern(stream, manager->EndWith->Name, manager->EndWith->Len, pBuff, len);
        if (lineLen > 0) {
            Mem_LenType cmdIndex;
            // remove endWith
            lineLen -= manager->EndWith->Len;
            pBuff[lineLen] = '\0';
            // check it's from last cmd or it's new cmd
            if (manager->InUseCmd == NULL && lineLen > 0) {
                // check start with
                if (manager->StartWith) {
                    if (Str_compareFix(pBuff, manager->StartWith->Name, manager->StartWith->Len) == 0) {
                        pBuff += manager->StartWith->Len;
                    }
                    else {
                        return;
                    }
                }
                // find cmd name len
                cmdStr.Name = pBuff;
                pBuff = Str_ignoreNameCharacters(pBuff);
                cmdStr.Len = pBuff - cmdStr.Name;
                lineLen -= cmdStr.Len;
                // find cmd
                cmdIndex = __search(manager->List.Cmds, manager->List.Len, sizeof(manager->List.Cmds[0]), &cmdStr, Cmd_compareName);
                if (cmdIndex != -1) {
                #if CMD_LIST_MODE == CMD_LIST_ARRAY
                    Cmd* cmd = &manager->List.Cmds[cmdIndex];
                #else
                    Cmd* cmd = manager->List.Cmds[cmdIndex];
                #endif // CMD_LIST_MODE
                    if (manager->PatternTypes) {
                        Cmd_Type type;
                        // ignore whitespaces between CmdName and CmdType
                        pBuff = Str_ignoreWhitespace(pBuff);
                        // find type len
                        cmdStr.Name = pBuff;
                        pBuff = Str_ignoreCommandCharacters(pBuff);
                        cmdStr.Len = pBuff - cmdStr.Name;
                        lineLen -= cmdStr.Len;
                        // find cmd type
                        Mem_LenType typeIndex = Mem_linearSearch(manager->PatternTypes->Patterns, CMD_TYPE_LEN, sizeof(Cmd_Str*), &cmdStr, CmdType_compare);
                        if (typeIndex != -1) {
                            manager->Cursor.Ptr = pBuff;
                            manager->Cursor.Len = lineLen;
                            type = (Cmd_Type) (1 << typeIndex);
                            if (cmd->Callbacks.fn[typeIndex] && (cmd->Types.Flags & type)) {
                                if (cmd->Callbacks.fn[typeIndex](cmd, &manager->Cursor, type) != Cmd_Done) {
                                    manager->InUseCmd = cmd;
                                    manager->InUseCmdTypeIndex = typeIndex;
                                }
                                return;
                            }
                            else if (cmd->Callbacks.unknown) {
                                if (cmd->Callbacks.unknown(cmd, &manager->Cursor, type) != Cmd_Done) {
                                    manager->InUseCmd = cmd;
                                    manager->InUseCmdTypeIndex = typeIndex;
                                }
                                return;
                            }
                        }
                    }
                }
                // run not found
                if (manager->notFound) {
                    manager->notFound(buffer);
                }
            }
            else {
                manager->Cursor.Ptr = pBuff;
                manager->Cursor.Len = lineLen;
                if (manager->InUseCmd->Callbacks.fn[manager->InUseCmdTypeIndex](manager->InUseCmd, &manager->Cursor, (Cmd_Type) (1 << manager->InUseCmdTypeIndex)) == Cmd_Done) {
                    manager->InUseCmd = NULL;
                }
            }
        }
    }
}

static char Cmd_compare(const void* itemA, const void* itemB, Mem_LenType itemLen) {
#if CMD_LIST_MODE == CMD_LIST_ARRAY
    return Str_compare(((Cmd*) itemA)->CmdName.Name, ((Cmd*) itemB)->CmdName.Name);
#else
    return Str_compare((*(Cmd**) itemA)->CmdName.Name, (*(Cmd**) itemB)->CmdName.Name);
#endif // CMD_LIST_MODE
}
static char Cmd_compareName(const void* name, const void* cmd, Mem_LenType itemLen) {
#if CMD_LIST_MODE == CMD_LIST_ARRAY
    Str_LenType result;
    if ((result = ((Cmd_Str*) name)->Len - ((Cmd*) cmd)->CmdName.Len) == 0) {
        return Str_compareFix(((Cmd_Str*) name)->Name, ((Cmd*) cmd)->CmdName.Name, ((Cmd_Str*) name)->Len);
    }
    else {
        return result > 0 ? 1 : -1;
    }
#else
    Str_LenType result;
    if ((result = ((Cmd_Str*) name)->Len - (*(Cmd**) cmd)->CmdName.Len) == 0) {
        return Str_compareFix(((Cmd_Str*) name)->Name, (*(Cmd**) cmd)->CmdName.Name, ((Cmd_Str*) name)->Len);
    }
    else {
        return result > 0 ? 1 : -1;
    }

#endif // CMD_LIST_MODE
}
static void Cmd_swap(const void* itemA, const void* itemB, Mem_LenType itemLen) {
#if CMD_LIST_MODE == CMD_LIST_ARRAY
    Cmd temp;
    Mem_copy((uint8_t*) &temp, itemA, itemLen);
    Mem_copy(itemA, itemB, itemLen);
    Mem_copy(itemB, (uint8_t*) &temp, itemLen);
#else
    Cmd* temp;
    Mem_copy(&temp, itemA, itemLen);
    Mem_copy(itemA, itemB, itemLen);
    Mem_copy(itemB, &temp, itemLen);
#endif // CMD_LIST_MODE
}
static char CmdType_compare(const void* name, const void* type, Mem_LenType itemLen) {
    Str_LenType result;
    if ((result = ((Cmd_Str*) name)->Len - (*(Cmd_Str**) type)->Len) == 0) {
        return Str_compareFix(((Cmd_Str*) name)->Name, (*(Cmd_Str**) type)->Name, ((Cmd_Str*) name)->Len);
    }
    else {
        return result > 0 ? 1 : -1;
    }
}
