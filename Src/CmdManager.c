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
#if CMD_SORT_LIST
    static Mem_CmpResult Cmd_compare(const void* itemA, const void* itemB, Mem_LenType itemLen);
    static void Cmd_swap(void* itemA, void* itemB, Mem_LenType itemLen);
#endif // CMD_SORT_LIST
static Mem_CmpResult Cmd_compareName(const void* name, const void* cmd, Mem_LenType itemLen);
static Mem_CmpResult CmdType_compare(const void* name, const void* type, Mem_LenType itemLen);
/**
 * @brief initialize Cmd
 *
 * @param cmd
 * @param name
 * @param types
 */
void Cmd_init(Cmd* cmd, const char* name, Cmd_Type types) {
    cmd->CmdName.Text = name;
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
    manager->ParamSeperator = CMD_DEFAULT_PARAM_SEPERATOR;
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
#if CMD_STREAM
/**
 * @brief process incoming messages
 *
 * @param manager
 * @param stream
 */
void CmdManager_handle(CmdManager* manager, IStream* stream) {
    Param_Cursor cursor;
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
void CmdManager_handleStatic(CmdManager* manager, IStream* stream, char* buffer, Str_LenType len, Param_Cursor* cursor) {
    if (IStream_available(stream) > 0) {
        Stream_LenType lineLen = IStream_readBytesUntilPattern(stream, (const uint8_t*) manager->EndWith->Text, manager->EndWith->Len, (uint8_t*) buffer, len);
        if (lineLen > 0) {   
            lineLen -= manager->EndWith->Len;
            // check end with for overflow error
            if (Str_compareFix((const char*) &buffer[lineLen], (const char*) manager->EndWith->Text, manager->EndWith->Len) != 0) {
                if (manager->bufferOverflow) {
                    manager->bufferOverflow(manager);
                }
                return;
            }
            // remove endWith
            buffer[lineLen] = '\0';
            // check it's empty line or not
            if(lineLen == 0) {
                return;
            }
            CmdManager_processLine(manager, buffer, lineLen, cursor);
        }
    }
}
#endif // CMD_STREAM
/**
 * @brief process string buffer
 * 
 * @param manager 
 * @param buffer 
 * @param lineLen 
 * @param cursor 
 */
char* CmdManager_process(CmdManager* manager, char* buffer, Str_LenType len, Param_Cursor* cursor) {
    if (len > 0) {
        // read line
        Str_LenType lineLen = Str_posOf(buffer, *manager->EndWith->Text);
        if (lineLen > 0) {   
            // check end with for overflow error
            if (Str_compareFix((const char*) &buffer[lineLen], (const char*) manager->EndWith->Text, manager->EndWith->Len) != 0) {
                if (manager->bufferOverflow) {
                    manager->bufferOverflow(manager);
                }
                return NULL;
            }
            // remove endWith
            buffer[lineLen] = '\0';
            // check it's empty line or not
            if(lineLen == 0) {
                return NULL;
            }
            // process line
            CmdManager_processLine(manager, buffer, lineLen, cursor);
            // return end of line
            return &buffer[lineLen + manager->EndWith->Len];
        }     
    }
    return NULL;
}
/**
 * @brief process single line without check ending pattern
 * 
 * @param manager 
 * @param buffer 
 * @param len 
 * @param cursor 
 */
void CmdManager_processLine(CmdManager* manager, char* buffer, Str_LenType lineLen, Param_Cursor* cursor) {
    Cmd_Str cmdStr;
    Mem_LenType cmdIndex;
    char* baseBuffer = buffer;
#if CMD_REMOVE_BACKSPACE
    // remove backspaces
    Str_removeBackspaceFix(buffer, lineLen);
#endif // CMD_REMOVE_BACKSPACE
    // check it's from last cmd or it's new cmd
    if (manager->InUseCmd == NULL) {
        // ignore whitspaces in start of frame
        buffer = Str_ignoreWhitespace(buffer);
        // check start with
        if (manager->StartWith) {
        #if CMD_CONVERT_START_WITH
            __convert(buffer, manager->StartWith->Len);
        #endif // CMD_CONVERT_START_WITH
            if (Str_compareFix(buffer, manager->StartWith->Text, manager->StartWith->Len) == 0) {
                buffer += manager->StartWith->Len;
                // ignore whitspaces
                buffer = Str_ignoreWhitespace(buffer);
            }
            else {
                return;
            }
        }
        // find cmd name len
        cmdStr.Text = buffer;
        buffer = Str_ignoreNameCharacters(buffer);
        cmdStr.Len = (Str_LenType) (buffer - cmdStr.Text);
        lineLen -= cmdStr.Len;
        // find cmd               
        __convert((char*) cmdStr.Text, cmdStr.Len);
        cmdIndex = __search(manager->List.Cmds, manager->List.Len, sizeof(manager->List.Cmds[0]), &cmdStr, Cmd_compareName);
        if (cmdIndex != -1) {
            Cmd* cmd = CmdList_get(manager->List.Cmds, cmdIndex);
            if (manager->PatternTypes) {
                Cmd_Type type = Cmd_Type_None;
                Mem_LenType typeIndex;       
                // ignore whitespaces between Cmd_Name and Cmd_Type
                buffer = Str_ignoreWhitespace(buffer);
                // find cmd type len
                cmdStr.Text = buffer;
                buffer = Str_ignoreCommandCharacters(buffer);
                cmdStr.Len = (Str_LenType) (buffer - cmdStr.Text);
                lineLen -= cmdStr.Len;
                // find cmd type
                typeIndex = Mem_linearSearch(manager->PatternTypes->Patterns, CMD_TYPE_LEN, sizeof(Cmd_Str*), &cmdStr, CmdType_compare);
                if (typeIndex != -1) {       
                    cursor->Ptr = buffer;
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
            manager->notFound(manager, baseBuffer);
        }
    }
    else {
        cursor->Ptr = buffer;
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
static Mem_CmpResult Cmd_compareName(const void* name, const void* cmd, Mem_LenType itemLen) {
#if CMD_LIST_MODE == CMD_LIST_ARRAY
    Str_LenType result;
    if ((result = Mem_castItem(Cmd_Str, name)->Len - Mem_castItem(Cmd, cmd)->CmdName.Len) == 0) {
        return Str_compareFix(Mem_castItem(Cmd_Str, name)->Name, Mem_castItem(Cmd, cmd)->CmdName.Text, Mem_castItem(Cmd_Str, name)->Len);
    }
    else {
        return result > 0 ? 1 : -1;
    }
#else
    Str_LenType result;
    if ((result = Mem_castItem(Cmd_Str, name)->Len - (*Mem_castItem(Cmd*, cmd))->CmdName.Len) == 0) {
        return Str_compareFix(Mem_castItem(Cmd_Str, name)->Text, (*Mem_castItem(Cmd*, cmd))->CmdName.Text, Mem_castItem(Cmd_Str, name)->Len);
    }
    else {
        return result > 0 ? 1 : -1;
    }

#endif // CMD_LIST_MODE
}
static Mem_CmpResult CmdType_compare(const void* name, const void* type, Mem_LenType itemLen) {
    Str_LenType result;
    if ((result = Mem_castItem(Cmd_Str, name)->Len - (*Mem_castItem(Cmd_Str*, type))->Len) == 0) {
        return Str_compareFix(Mem_castItem(Cmd_Str, name)->Text, (*Mem_castItem(Cmd_Str*, type))->Text, Mem_castItem(Cmd_Str, name)->Len);
    }
    else {
        return result > 0 ? 1 : -1;
    }
}
#if CMD_SORT_LIST
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
static Mem_CmpResult Cmd_compare(const void* itemA, const void* itemB, Mem_LenType itemLen) {
#if CMD_LIST_MODE == CMD_LIST_ARRAY
    return Str_compare(Mem_castItem(Cmd, itemA)->CmdName.Text, Mem_castItem(Cmd, itemB)->CmdName.Text);
#else
    return Str_compare((*Mem_castItem(Cmd*, itemA))->CmdName.Text, (*Mem_castItem(Cmd*, itemB))->CmdName.Text);
#endif // CMD_LIST_MODE
}
#endif // CMD_SORT_LIST
