#include "CmdManager.h"
#include "InputStream.h"
#include "Str.h"
#include <stdint.h>


/* public variables */
#if CMD_TYPE_EXE
    static const Cmd_Str PATTERN_TYPE_EXE   = CMD_STR_INIT(CMD_DEFAULT_PATTERN_TYPE_EXE);
#endif
#if CMD_TYPE_SET
    static const Cmd_Str PATTERN_TYPE_SET   = CMD_STR_INIT(CMD_DEFAULT_PATTERN_TYPE_SET);
#endif
#if CMD_TYPE_GET
    static const Cmd_Str PATTERN_TYPE_GET   = CMD_STR_INIT(CMD_DEFAULT_PATTERN_TYPE_GET);
#endif
#if CMD_TYPE_HELP
    static const Cmd_Str PATTERN_TYPE_HELP  = CMD_STR_INIT(CMD_DEFAULT_PATTERN_TYPE_HELP);
#endif
#if CMD_TYPE_RESP
    static const Cmd_Str PATTERN_TYPE_RESP  = CMD_STR_INIT(CMD_DEFAULT_PATTERN_TYPE_RESP);
#endif
const Cmd_PatternTypes CMD_PATTERN_TYPES = {{
#if CMD_TYPE_EXE
    (Cmd_Str*) &PATTERN_TYPE_EXE,
#endif
#if CMD_TYPE_SET
    (Cmd_Str*) &PATTERN_TYPE_SET,
#endif
#if CMD_TYPE_GET
    (Cmd_Str*) &PATTERN_TYPE_GET,
#endif
#if CMD_TYPE_HELP
    (Cmd_Str*) &PATTERN_TYPE_HELP,
#endif
#if CMD_TYPE_RESP
    (Cmd_Str*) &PATTERN_TYPE_RESP,
#endif
}};
const Cmd_Str CMD_END_WITH = CMD_STR_INIT(CMD_DEFAULT_END_WITH);
const char CMD_PARAM_SEPERATOR = ',';
/* private defines */
#if CMD_SORT_LIST
    #if CMD_SORT_ALG == CMD_SORT_ALG_SELECTION
        #define __sort          Mem_sort
    #elif CMD_SORT_ALG == CMD_SORT_ALG_QUICK_SORT
        #define __sort          Mem_quickSort
    #endif

    #define __search            Mem_binarySearch
#else
    #define __search            Mem_linearSearch
#endif
#if CMD_CASE_MODE == CMD_CASE_INSENSITIVE
    #if CMD_NAME_MODE == CMD_LOWER_CASE
        #define __convert(STR, LEN) Str_lowerCaseFix((STR), (LEN))
    #else
        #define __convert(STR, LEN) Str_upperCaseFix((STR), (LEN))
    #endif // CMD_NAME_MODE
#else
    #define __convert(STR, LEN)
#endif
/* private functions */
static char Cmd_compare(const void* itemA, const void* itemB, Mem_LenType itemLen);
static char Cmd_compareName(const void* name, const void* cmd, Mem_LenType itemLen);
static void Cmd_swap(void* itemA, void* itemB, Mem_LenType itemLen);
static char CmdType_compare(const void* buff, const void* type, Mem_LenType itemLen);
static uint8_t CmdParam_parseBinary(Cmd_Cursor* cursor, Cmd_Param* param);
static uint8_t CmdParam_parseHex(Cmd_Cursor* cursor, Cmd_Param* param);
static uint8_t CmdParam_parseNum(Cmd_Cursor* cursor, Cmd_Param* param);
static uint8_t CmdParam_parseString(Cmd_Cursor* cursor, Cmd_Param* param);
static uint8_t CmdParam_parseState(Cmd_Cursor* cursor, Cmd_Param* param);
static uint8_t CmdParam_parseStateKey(Cmd_Cursor* cursor, Cmd_Param* param);
static uint8_t CmdParam_parseBoolean(Cmd_Cursor* cursor, Cmd_Param* param);
static uint8_t CmdParam_parseNull(Cmd_Cursor* cursor, Cmd_Param* param);
static uint8_t CmdParam_parseUnknown(Cmd_Cursor* cursor, Cmd_Param* param);
/**
 * @brief initialize Cmd
 *
 * @param cmd
 * @param name
 * @param types
 */
void Cmd_init(Cmd* cmd, const char* name, Cmd_Type types) {
    cmd->CmdName.Name = name;
    cmd->CmdName.Len = Str_len(name);
    cmd->Types.Flags = (uint8_t) types;
    Mem_set(cmd->Callbacks.fn, 0x00, sizeof(Cmd_Callbacks));
}
/**
 * @brief enable type for callbacks
 *
 * @param cmd
 * @param types
 */
void Cmd_setTypes(Cmd* cmd, Cmd_Type types) {
    cmd->Types.Flags = (uint8_t) types;
}
#if CMD_MULTI_CALLBACK
#if CMD_TYPE_EXE
void Cmd_onExecute(Cmd* cmd, Cmd_CallbackFn fn) {
    cmd->Callbacks.execute = fn;
}
#endif
#if CMD_TYPE_SET
void Cmd_onSet(Cmd* cmd, Cmd_CallbackFn fn) {
    cmd->Callbacks.set = fn;
}
#endif
#if CMD_TYPE_GET
void Cmd_onGet(Cmd* cmd, Cmd_CallbackFn fn) {
    cmd->Callbacks.get = fn;
}
#endif
#if CMD_TYPE_HELP
void Cmd_onHelp(Cmd* cmd, Cmd_CallbackFn fn) {
    cmd->Callbacks.help = fn;
}
#endif
#if CMD_TYPE_RESP
void Cmd_onResponse(Cmd* cmd, Cmd_CallbackFn fn) {
    cmd->Callbacks.response = fn;
}
#endif
#if CMD_TYPE_UNKNOWN
void Cmd_onUnknown(Cmd* cmd, Cmd_CallbackFn fn) {
    cmd->Callbacks.unknown = fn;
}
#endif

#else
void Cmd_on(Cmd* cmd, Cmd_CallbackFn fn) {
    cmd->Callbacks.fn[0] = fn;
}
#endif // CMD_MULTI_CALLBACK
/**
 * @brief initialize CmdManager with default pattern and param seperator
 *
 * @param manager
 * @param cmds
 * @param len
 */
void CmdManager_init(CmdManager* manager, Cmd_Array* cmds, Cmd_LenType len) {
    manager->PatternTypes = (Cmd_PatternTypes*) &CMD_PATTERN_TYPES;
    manager->StartWith = NULL;
    manager->EndWith = (Cmd_Str*) &CMD_END_WITH;
    manager->List.Cmds = cmds;
    manager->List.Len = len;
    manager->notFound = (Cmd_NotFoundFn) NULL;
    manager->bufferOverflow = (Cmd_OverflowFn) NULL;
    manager->ParamSeperator = CMD_PARAM_SEPERATOR;
    manager->InUseCmd = NULL;
#if CMD_SORT_LIST
    __sort(manager->List.Cmds, manager->List.Len, sizeof(manager->List.Cmds[0]), Cmd_compare, Cmd_swap);
#endif
}
/**
 * @brief set pattern for start of commands
 *
 * @param manager
 * @param startWith
 */
void CmdManager_setStartWith(CmdManager* manager, Cmd_Str* startWith) {
    manager->StartWith = startWith;
}
/**
 * @brief set pattern for end of commands
 *
 * @param manager
 * @param endWith
 */
void CmdManager_setEndWith(CmdManager* manager, Cmd_Str* endWith) {
    manager->EndWith = endWith;
}
/**
 * @brief set not found callback
 *
 * @param manager
 * @param notFound
 */
void CmdManager_onNotFound(CmdManager* manager, Cmd_NotFoundFn notFound) {
    manager->notFound = notFound;
}
/**
 * @brief set overflow command
 *
 * @param manager
 * @param overflow
 */
void CmdManager_onOverflow(CmdManager* manager, Cmd_OverflowFn overflow) {
    manager->bufferOverflow = overflow;
}
/**
 * @brief set param seperator
 *
 * @param manager
 * @param sep
 */
void CmdManager_setParamSeperator(CmdManager* manager, char sep) {
    manager->ParamSeperator = sep;
}
/**
 * @brief set commands list
 *
 * @param manager
 * @param cmds
 * @param len
 */
void CmdManager_setCommands(CmdManager* manager, Cmd_Array* cmds, Cmd_LenType len) {
    manager->List.Cmds = cmds;
    manager->List.Len = len;
#if CMD_SORT_LIST
    __sort(manager->List.Cmds, manager->List.Len, sizeof(manager->List.Cmds[0]), Cmd_compare, Cmd_swap);
#endif
}
/**
 * @brief set pattern types
 *
 * @param manager
 * @param patterns
 */
void CmdManager_setPatternTypes(CmdManager* manager, Cmd_PatternTypes* patterns) {
    manager->PatternTypes = patterns;
}
#if CMD_MANAGER_ARGS
/**
 * @brief set args for manager
 *
 * @param manager
 * @param args
 */
void CmdManager_setArgs(CmdManager* manager, void* args) {
    manager->Args = args;
}
/**
 * @brief get args for manager
 *
 * @param manager
 * @return void*
 */
void* CmdManager_getArgs(CmdManager* manager) {
    return manager->Args;
}
#endif
/**
 * @brief process incoming messages
 *
 * @param manager
 * @param stream
 */
void CmdManager_handle(CmdManager* manager, IStream* stream) {
    Cmd_Cursor cursor;
    char buffer[CMD_HANDLE_BUFFER_SIZE];

    CmdManager_handleStatic(manager, stream, buffer, sizeof(buffer), &cursor);
}
/**
 * @brief process incoming messages with static buffer and cursor for memory safety
 *
 * @param manager
 * @param stream
 * @param buffer
 * @param len
 * @param cursor
 */
void CmdManager_handleStatic(CmdManager* manager, IStream* stream, char* buffer, Str_LenType len, Cmd_Cursor* cursor) {
    if (IStream_available(stream) > 0) {
        Cmd_Str cmdStr;
        char* pBuff = buffer;
        Stream_LenType lineLen = IStream_readBytesUntilPattern(stream, manager->EndWith->Name, manager->EndWith->Len, pBuff, len);
        if (lineLen > 0) {
            Mem_LenType cmdIndex;
            lineLen -= manager->EndWith->Len;
            // check end with for overflow error
            if (Str_compareFix(&pBuff[lineLen], manager->EndWith->Name, manager->EndWith->Len) != 0) {
                if (manager->bufferOverflow) {
                    manager->bufferOverflow(manager);
                }
                return;
            }
            // remove endWith
            pBuff[lineLen] = '\0';
            // check it's empty line or not
            if(lineLen == 0) {
                return;
            }
        #if CMD_REMOVE_BACKSPACE
            // remove backspaces
            Str_removeBackspaceFix(pBuff, lineLen);
        #endif // CMD_REMOVE_BACKSPACE
            // check it's from last cmd or it's new cmd
            if (manager->InUseCmd == NULL) {
                // check start with
                if (manager->StartWith) {
                #if CMD_CONVERT_START_WITH
                    __convert(pBuff, manager->StartWith->Len);
                #endif // CMD_CONVERT_START_WITH
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
                __convert((char*) cmdStr.Name, cmdStr.Len);
                cmdIndex = __search(manager->List.Cmds, manager->List.Len, sizeof(manager->List.Cmds[0]), &cmdStr, Cmd_compareName);
                if (cmdIndex != -1) {
                #if CMD_LIST_MODE == CMD_LIST_ARRAY
                    Cmd* cmd = &manager->List.Cmds[cmdIndex];
                #else
                    Cmd* cmd = manager->List.Cmds[cmdIndex];
                #endif // CMD_LIST_MODE
                    if (manager->PatternTypes) {
                        Cmd_Type type = Cmd_Type_None;
                        // ignore whitespaces between Cmd_Name and Cmd_Type
                        pBuff = Str_ignoreWhitespace(pBuff);
                        // find type len
                        cmdStr.Name = pBuff;
                        pBuff = Str_ignoreCommandCharacters(pBuff);
                        cmdStr.Len = pBuff - cmdStr.Name;
                        lineLen -= cmdStr.Len;
                        // find cmd type
                        Mem_LenType typeIndex = Mem_linearSearch(manager->PatternTypes->Patterns, CMD_TYPE_LEN, sizeof(Cmd_Str*), &cmdStr, CmdType_compare);
                        if (typeIndex != -1) {
                            cursor->Ptr = pBuff;
                            cursor->Len = lineLen;
                            cursor->ParamSeperator = manager->ParamSeperator;
                            cursor->Index = 0;
                            type = (Cmd_Type) (1 << typeIndex);
                        #if CMD_MULTI_CALLBACK
                            if (cmd->Callbacks.fn[typeIndex] && (cmd->Types.Flags & type)) {
                                if (cmd->Callbacks.fn[typeIndex](manager, cmd, cursor, type) != Cmd_Done) {
                                    manager->InUseCmd = cmd;
                                    manager->InUseCmdTypeIndex = typeIndex;
                                }
                                return;
                            }
                        #else
                            if (cmd->Callbacks.fn[0] && (cmd->Types.Flags & type)) {
                                if (cmd->Callbacks.fn[0](cmd, cursor, type) != Cmd_Done) {
                                    manager->InUseCmd = cmd;
                                    manager->InUseCmdTypeIndex = typeIndex;
                                }
                                return;
                            }
                        #endif // CMD_MULTI_CALLBACK
                        }
                        else {
                        #if CMD_TYPE_UNKNOWN
                        #if CMD_MULTI_CALLBACK
                            if (cmd->Callbacks.unknown && (cmd->Types.Flags & Cmd_Type_Unknown)) {
                                if (cmd->Callbacks.unknown(manager, cmd, cursor, type) != Cmd_Done) {
                                    manager->InUseCmd = cmd;
                                    manager->InUseCmdTypeIndex = typeIndex;
                                }
                                return;
                            }
                        #else
                            if (cmd->Callbacks.fn[0] && (cmd->Types.Flags & Cmd_Type_Unknown)) {
                                if (cmd->Callbacks.fn[0](cmd, cursor, type) != Cmd_Done) {
                                    manager->InUseCmd = cmd;
                                    manager->InUseCmdTypeIndex = typeIndex;
                                }
                                return;
                            }
                        #endif // CMD_MULTI_CALLBACK
                        #endif // CMD_TYPE_UNKNOWN
                        }
                    }
                }
                // run not found
                if (manager->notFound) {
                    manager->notFound(manager, buffer);
                }
            }
            else {
                cursor->Ptr = pBuff;
                cursor->Len = lineLen;
                cursor->ParamSeperator = manager->ParamSeperator;
                cursor->Index = 0;
            #if CMD_MULTI_CALLBACK
                if (manager->InUseCmd->Callbacks.fn[manager->InUseCmdTypeIndex](manager, manager->InUseCmd, cursor, (Cmd_Type) (1 << manager->InUseCmdTypeIndex)) == Cmd_Done) {
                    manager->InUseCmd = NULL;
                }
            #else
                if (manager->InUseCmd->Callbacks.fn[0](manager, manager->InUseCmd, cursor, (Cmd_Type) (1 << manager->InUseCmdTypeIndex)) == Cmd_Done) {
                    manager->InUseCmd = NULL;
                }
            #endif // CMD_MULTI_CALLBACK
            }
        }
    }
}
/**
 * @brief parse next param and return
 *
 * @param cursor
 * @param param
 * @return Cmd_Param* return null if param was invalid
 */
Cmd_Param* CmdManager_nextParam(Cmd_Cursor* cursor, Cmd_Param* param) {
    char* pStr;
    uint8_t res = 0;
    // check cursor is valid
    if (cursor->Ptr == NULL || *cursor->Ptr == '\0') {
        return NULL;
    }
    // ignore whitspaces
    cursor->Ptr = Str_ignoreWhitespace(cursor->Ptr);
    // find value type base on first charcater
    switch (*cursor->Ptr) {
        case '0':
            switch (*(cursor->Ptr + 1)) {
                case 'b':
                case 'B':
                    // binary num
                    res = CmdParam_parseBinary(cursor, param);
                    break;
                case 'x':
                case 'X':
                    // hex num
                    res = CmdParam_parseHex(cursor, param);
                    break;
            }
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        case '-':
            // check for number or its float
            if (!res) {
                res = CmdParam_parseNum(cursor, param);
            }
            break;
        case 't':
        case 'T':
        case 'f':
        case 'F':
            // boolean
            res = CmdParam_parseBoolean(cursor, param);
            break;
        case 'o':
        case 'O':
            // state
            res = CmdParam_parseStateKey(cursor, param);
            break;
        case 'h':
        case 'H':
        case 'l':
        case 'L':
            // state key
            res = CmdParam_parseState(cursor, param);
            break;
        case '"':
            // string
            res = CmdParam_parseString(cursor, param);
            break;
        default:
            // unknown
            res = CmdParam_parseUnknown(cursor, param);
            break;
    }
    // find next seperator
    pStr = Str_indexOf(cursor->Ptr, cursor->ParamSeperator);
    if (pStr != NULL) {
        Str_LenType len = (Str_LenType)(cursor->Ptr - pStr);
        cursor->Ptr = pStr + 1;
        cursor->Len -= len + 1;
    }
    else {
        cursor->Ptr = NULL;
        cursor->Len = 0;
    }
    // return param
    if (res) {
        param->Index = ++cursor->Index;
        return param;
    }
    else {
        return NULL;
    }
}

static uint8_t CmdParam_parseBinary(Cmd_Cursor* cursor, Cmd_Param* param) {
    char* pStr = cursor->Ptr;
    Str_LenType len;
    cursor->Ptr = Str_ignoreNumeric(cursor->Ptr + 2);
    len = (Str_LenType)(cursor->Ptr - pStr);
    cursor->Len -= len;

    param->ValueType = Cmd_ValueType_NumberBinary;
    return Str_convertUNumFix(pStr + 2, (unsigned int*) &param->Value.NumberBinary, Str_Binary, len) == Str_Ok;
}
static uint8_t CmdParam_parseHex(Cmd_Cursor* cursor, Cmd_Param* param) {
    char* pStr = cursor->Ptr;
    Str_LenType len;
    cursor->Ptr = Str_ignoreAlphaNumeric(cursor->Ptr + 2);
    len = (Str_LenType)(cursor->Ptr - pStr);
    cursor->Len -= len;

    param->ValueType = Cmd_ValueType_NumberHex;
    return Str_convertUNumFix(pStr + 2, (unsigned int*) &param->Value.NumberHex, Str_Hex, len) == Str_Ok;
}
static uint8_t CmdParam_parseNum(Cmd_Cursor* cursor, Cmd_Param* param) {
    char* pStr = cursor->Ptr;
    Str_LenType len;

    cursor->Ptr = Str_ignoreNumeric(cursor->Ptr);

    if (*cursor->Ptr == '.') {
        // it's float
        cursor->Ptr = Str_ignoreNumeric(cursor->Ptr + 1);
        len = (Str_LenType)(cursor->Ptr - pStr);
        cursor->Len -= len;

        param->ValueType = Cmd_ValueType_Float;
        return Str_convertFloatFix(pStr, &param->Value.Float, len) == Str_Ok;
    }
    else {
        // it's number
        len = (Str_LenType)(cursor->Ptr - pStr);
        cursor->Len -= len;
        param->ValueType = Cmd_ValueType_Number;
        return Str_convertNumFix(pStr, (unsigned int*) &param->Value.Number, Str_Decimal, len) == Str_Ok;
    }
}
static uint8_t CmdParam_parseString(Cmd_Cursor* cursor, Cmd_Param* param) {
    char* pStr = cursor->Ptr;
    Str_LenType len = Str_fromString(cursor->Ptr) + 2;

    if (len != -1) {
        cursor->Ptr += len;
        cursor->Len -= len;

        param->ValueType = Cmd_ValueType_String;
        param->Value.String = pStr;

        return 1;
    }
    else {
        return 0;
    }
}
static uint8_t CmdParam_parseState(Cmd_Cursor* cursor, Cmd_Param* param) {
    char* pStr = cursor->Ptr;
    Str_LenType len;
    cursor->Ptr = Str_ignoreAlphabet(cursor->Ptr);
    len = (Str_LenType)(cursor->Ptr - pStr);
    cursor->Len -= len;

    __convert(pStr, len);
    param->ValueType = Cmd_ValueType_State;
    if (Str_compareFix(pStr, "high", len) == 0 || Str_compareFix(pStr, "HIGH", len) == 0) {
        param->Value.State = 1;
        return 1;
    }
    else if (Str_compareFix(pStr, "low", len) == 0 || Str_compareFix(pStr, "LOW", len) == 0) {
        param->Value.State = 0;
        return 1;
    }
    else {
        return 0;
    }
}
static uint8_t CmdParam_parseStateKey(Cmd_Cursor* cursor, Cmd_Param* param) {
    char* pStr = cursor->Ptr;
    Str_LenType len;
    cursor->Ptr = Str_ignoreAlphabet(cursor->Ptr);
    len = (Str_LenType)(cursor->Ptr - pStr);
    cursor->Len -= len;

    __convert(pStr, len);
    param->ValueType = Cmd_ValueType_StateKey;
    if (Str_compareFix(pStr, "on", len) == 0 || Str_compareFix(pStr, "ON", len) == 0) {
        param->Value.StateKey = 1;
        return 1;
    }
    else if (Str_compareFix(pStr, "off", len) == 0 || Str_compareFix(pStr, "OFF", len) == 0) {
        param->Value.StateKey = 0;
        return 1;
    }
    else {
        return 0;
    }
}
static uint8_t CmdParam_parseBoolean(Cmd_Cursor* cursor, Cmd_Param* param) {
    char* pStr = cursor->Ptr;
    Str_LenType len;
    cursor->Ptr = Str_ignoreAlphabet(cursor->Ptr);
    len = (Str_LenType)(cursor->Ptr - pStr);
    cursor->Len -= len;

    __convert(pStr, len);
    param->ValueType = Cmd_ValueType_Boolean;
    if (Str_compareFix(pStr, "true", len) == 0 || Str_compareFix(pStr, "TRUE", len) == 0) {
        param->Value.Boolean = 1;
        return 1;
    }
    else if (Str_compareFix(pStr, "false", len) == 0 || Str_compareFix(pStr, "FALSE", len) == 0) {
        param->Value.Boolean = 0;
        return 1;
    }
    else {
        return 0;
    }
}
static uint8_t CmdParam_parseNull(Cmd_Cursor* cursor, Cmd_Param* param) {
    char* pStr = cursor->Ptr;
    Str_LenType len;
    cursor->Ptr = Str_ignoreAlphabet(cursor->Ptr);
    len = (Str_LenType)(cursor->Ptr - pStr);
    cursor->Len -= len;

    __convert(pStr, len);
    param->ValueType = Cmd_ValueType_Null;
    if (Str_compareFix(pStr, "null", len) == 0 || Str_compareFix(pStr, "NULL", len) == 0) {
        param->Value.Null = pStr;
        return 1;
    }
    else {
        return 0;
    }
}
static uint8_t CmdParam_parseUnknown(Cmd_Cursor* cursor, Cmd_Param* param) {
    param->ValueType = Cmd_ValueType_Unknown;
    param->Value.Unknown = cursor->Ptr;

    cursor->Ptr = Str_indexOf(cursor->Ptr, cursor->ParamSeperator);
    if (cursor->Ptr) {
        Str_LenType len;
        len = (Str_LenType)(cursor->Ptr - param->Value.Unknown);
        cursor->Len -= len;
    }
    else {
        cursor->Ptr = Str_indexOfEnd(cursor->Ptr);
        cursor->Len = 0;
    }

    return 1;
}

static char Cmd_compare(const void* itemA, const void* itemB, Mem_LenType itemLen) {
#if CMD_LIST_MODE == CMD_LIST_ARRAY
    return Str_compare(Mem_castItem(Cmd, itemA)->CmdName.Name, Mem_castItem(Cmd, itemB)->CmdName.Name);
#else
    return Str_compare((*Mem_castItem(Cmd*, itemA))->CmdName.Name, (*Mem_castItem(Cmd*, itemB))->CmdName.Name);
#endif // CMD_LIST_MODE
}
static char Cmd_compareName(const void* name, const void* cmd, Mem_LenType itemLen) {
#if CMD_LIST_MODE == CMD_LIST_ARRAY
    Str_LenType result;
    if ((result = Mem_castItem(Cmd_Str, name)->Len - Mem_castItem(Cmd, cmd)->CmdName.Len) == 0) {
        return Str_compareFix(Mem_castItem(Cmd_Str, name)->Name, Mem_castItem(Cmd, cmd)->CmdName.Name, Mem_castItem(Cmd_Str, name)->Len);
    }
    else {
        return result > 0 ? 1 : -1;
    }
#else
    Str_LenType result;
    if ((result = Mem_castItem(Cmd_Str, name)->Len - (*Mem_castItem(Cmd*, cmd))->CmdName.Len) == 0) {
        return Str_compareFix(Mem_castItem(Cmd_Str, name)->Name, (*Mem_castItem(Cmd*, cmd))->CmdName.Name, Mem_castItem(Cmd_Str, name)->Len);
    }
    else {
        return result > 0 ? 1 : -1;
    }

#endif // CMD_LIST_MODE
}
static void Cmd_swap(void* itemA, void* itemB, Mem_LenType itemLen) {
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
    if ((result = Mem_castItem(Cmd_Str, name)->Len - (*Mem_castItem(Cmd_Str*, type))->Len) == 0) {
        return Str_compareFix(Mem_castItem(Cmd_Str, name)->Name, (*Mem_castItem(Cmd_Str*, type))->Name, Mem_castItem(Cmd_Str, name)->Len);
    }
    else {
        return result > 0 ? 1 : -1;
    }
}
