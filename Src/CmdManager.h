/**
 * @file CmdManager.h
 * @author Ali Mirghasemi (ali.mirghasemi1376.com)
 * @brief this library can use for handle input string commands
 * it's bases on Str library and Stream library
 * @version 0.1
 * @date 2021-08-07
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef _CMD_MANAGER_H_
#define _CMD_MANAGER_H_

#include "InputStream.h"
#include "Str.h"
#include <stdint.h>

/********************************************************************************/
/*                              Configuration                                   */
/********************************************************************************/

#define CMD_MULTI_CALLBACK                  1
#define CMD_UNKWON_CALLBACK                 1

#define CMD_SORT_LIST                       1

#if CMD_SORT_LIST
    #define CMD_SORT_ALG_SELECTION          1
    #define CMD_SORT_ALG_QUICK_SORT         2

    #define CMD_SORT_ALG                    CMD_SORT_ALG_QUICK_SORT
#endif // CMD_SORT_LIST

typedef uint8_t Cmd_LenType;

#define CMD_LIST_ARRAY                      1
#define CMD_LIST_POINTER_ARRAY              2
#define CMD_LIST_MODE                       CMD_LIST_POINTER_ARRAY

#define CMD_DEFAULT_PATTERN_TYPE_EXE        ""
#define CMD_DEFAULT_PATTERN_TYPE_SET        "="
#define CMD_DEFAULT_PATTERN_TYPE_GET        "?"
#define CMD_DEFAULT_PATTERN_TYPE_HELP       "=?"
#define CMD_DEFAULT_PATTERN_TYPE_RESP       ":"
#define CMD_DEFAULT_END_WITH                "\n"
#define CMD_DEFAULT_PARAM_SEPERATOR         ','
/********************************************************************************/

#if __cplusplus
extern "C" {
#endif

/* pre-define types */
struct __Cmd;
typedef struct __Cmd Cmd;
struct __Cmd_Params;
typedef struct __Cmd_Params Cmd_Params;

#if CMD_LIST_MODE == CMD_LIST_ARRAY
    typedef Cmd    Cmd_Array;
#else
    typedef Cmd*   Cmd_Array;
#endif

/**
 * @brief hold command name and len of name
 */
typedef struct {
    const char*     Name;
    Str_LenType     Len;
} Cmd_Str;
/**
 * @brief show command type
 */
typedef enum {
    Cmd_Type_Unknown        = 0x00,     /**< don't check cmd name ending */
    Cmd_Type_Execute        = 0x01,     /**< ex: "<cmd>\r\n" */
    Cmd_Type_Set            = 0x02,     /**< ex: "<cmd>=<params>\r\n" */
    Cmd_Type_Get            = 0x04,     /**< ex: "<cmd>?\r\n" */
    Cmd_Type_Help           = 0x08,     /**< ex: "<cmd>=?\r\n" */
    Cmd_Type_Response       = 0x10,     /**< ex: "<cmd>: <status>\r\n" */
    Cmd_Type_Any            = 0x1F,     /**< check all types */
} Cmd_Type;

#define CMD_TYPE_LEN        5

/**
 * @brief determine need handle multiple ending or not
 */
typedef enum {
    Cmd_Done                = 0,        /**< command end with single ending */
    Cmd_Continue            = 1,        /**< command have multiple ending */
} Cmd_Result;
/**
 * @brief callback of command
 */
typedef Cmd_Result (*Cmd_CallbackFn) (Cmd* cmd, Cmd_Params* params, Cmd_Type type);
typedef void (*Cmd_NotFoundFn) (char* str);
typedef void (*Cmd_OverflowFn) (void);

typedef struct {
    Cmd_Str         Value;
} Cmd_Param;

struct __Cmd_Params {
    char*           Cursor;
};
/**
 * @brief hold callback functions
 */
typedef union {
#if CMD_MULTI_CALLBACK
    Cmd_CallbackFn      fn[5 + CMD_UNKWON_CALLBACK];
    struct {
        Cmd_CallbackFn  execute;
        Cmd_CallbackFn  set;
        Cmd_CallbackFn  get;
        Cmd_CallbackFn  help;
        Cmd_CallbackFn  response;
    #if CMD_UNKWON_CALLBACK
        Cmd_CallbackFn  unknown;
    #endif // CMD_UNKWON_CALLBACK
    };
#else
    Cmd_CallbackFn      fn[1];
#endif // CMD_MULTI_CALLBACK
} Cmd_Callbacks;
/**
 * @brief hold valid types
 */
typedef union {
    uint8_t         Flags;
    struct {
        uint8_t     Execute     : 1;
        uint8_t     Set         : 1;
        uint8_t     Get         : 1;
        uint8_t     Help        : 1;
        uint8_t     Response    : 1;
        uint8_t     Reserved    : 3;
    };
} Cmd_Types;
/**
 * @brief custom type pattern
 */
typedef union {
    Cmd_Str*            Patterns[5];
    struct {
        Cmd_Str*        Execute;
        Cmd_Str*        Set;
        Cmd_Str*        Get;
        Cmd_Str*        Help;
        Cmd_Str*        Response;
    };
} Cmd_PatternTypes;
/**
 * @brief hold properties of single command
 */
struct __Cmd {
    Cmd_Callbacks       Callbacks;
    Cmd_Str             CmdName;
    Cmd_Types           Types;
};
/**
 * @brief hold array of commands
 */
typedef struct {
    Cmd_Array*          Cmds;
    Cmd_LenType         Len;
} Cmd_List;
/**
 * @brief hold properties of manger that need to handle commands
 */
typedef struct {
    Cmd_PatternTypes*   PatternTypes;
    Cmd_Str*            StartWith;
    Cmd_Str*            EndWith;
    Cmd_NotFoundFn      notFound;
    Cmd_OverflowFn      bufferOverflow;
    Cmd*                InUseCmd;
    Cmd_Params          Params;
    Cmd_List            List;
    char                ParamSeperator;
} CmdManager;

/* default types */
extern const Cmd_PatternTypes   CMD_PATTERN_TYPES;
extern const Cmd_Str            CMD_END_WITH;
extern const char               CMD_PARAM_SEPERATOR;
/* pre-processor APIs for const define */
#define CMD_STR_INIT(NAME)          {NAME, sizeof(NAME) - 1}
#define CMD_ARR_LEN(ARR)            (sizeof(ARR) / sizeof(ARR[0]))
#define CMD_LIST_INIT(LIST)         {LIST, CMD_ARR_LEN(LIST)}

#if CMD_MULTI_CALLBACK
    #define CMD_INIT(NAME, TYPES, EXE, SET, GET, HELP, RESP)    {{EXE, SET, GET, HELP, RESP}, CMD_STR_INIT(NAME), (TYPES)}

#else
    #define CMD_INIT(NAME, TYPES, FN)                           {{FN}, CMD_STR_INIT(NAME), (TYPES)}
#endif // CMD_MULTI_CALLBACK

void Cmd_init(Cmd* cmd, const char* name, Cmd_Type types);
void Cmd_setTypes(Cmd* cmd, Cmd_Type types);
#if CMD_MULTI_CALLBACK
    void Cmd_onExecute(Cmd* cmd, Cmd_CallbackFn fn);
    void Cmd_onSet(Cmd* cmd, Cmd_CallbackFn fn);
    void Cmd_onGet(Cmd* cmd, Cmd_CallbackFn fn);
    void Cmd_onHelp(Cmd* cmd, Cmd_CallbackFn fn);
    void Cmd_onResponse(Cmd* cmd, Cmd_CallbackFn fn);
#if CMD_UNKNOWN_CALLBACK
    void Cmd_onUnknown(Cmd* cmd, Cmd_CallbackFn fn);
#endif // CMD_UNKNOWN_CALLBACK

#else
    void Cmd_on(Cmd* cmd, Cmd_CallbackFn fn);
#endif // CMD_MULTI_CALLBACK

void CmdManager_init(CmdManager* manager, Cmd* cmds, Cmd_LenType len);
void CmdManager_setStartWith(CmdManager* manager, Cmd_Str* startWith);
void CmdManager_setEndWith(CmdManager* manager, Cmd_Str* endWith);
void CmdManager_onNotFound(CmdManager* manager, Cmd_NotFoundFn notFound);
void CmdManager_onOverflow(CmdManager* manager, Cmd_OverflowFn overflow);
void CmdManager_setParamSeperator(CmdManager* manager, char sep);
void CmdManager_setCommands(CmdManager* manager, Cmd* cmds, Cmd_LenType len);

void CmdManager_handle(CmdManager* manager, IStream* stream, char* buffer, Str_LenType len);

#if __cplusplus
};
#endif

#endif /* _CMD_MANAGER_H_ */
