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

/**
 * @brief enable multi callback, when you want callback per Cmd_Type
 */
#define CMD_MULTI_CALLBACK                  1
/**
 * @brief enable sort Cmd_Array on init or setCmds
 */
#define CMD_SORT_LIST                       1

#if CMD_SORT_LIST
    #define CMD_SORT_ALG_SELECTION          1
    #define CMD_SORT_ALG_QUICK_SORT         2
    /**
     * @brief set what algorithm used for sort commands
     */
    #define CMD_SORT_ALG                    CMD_SORT_ALG_QUICK_SORT
#endif // CMD_SORT_LIST


#define CMD_CASE_SENSITIVE                  1
#define CMD_CASE_INSENSITIVE                2
/**
 * @brief check commands and some value in case-sensitive mode or insensitive
 */
#define CMD_CASE_MODE                       CMD_CASE_INSENSITIVE
#if CMD_CASE_MODE == CMD_CASE_INSENSITIVE
    #define CMD_LOWER_CASE                  1
    #define CMD_UPPER_CASE                  2
    /**
     * @brief convert cmd name and some value into which mode before process
     */
    #define CMD_NAME_MODE                   CMD_LOWER_CASE
    /**
     * @brief convert start with in case-insensitive
     */
    #define CMD_CONVERT_START_WITH          0
#endif // CMD_CASE_MOE
/**
 * @brief remove backspace characters before process
 */
#define CMD_REMOVE_BACKSPACE                1

/**
 * @brief enable CmdManager have args
 */
#define CMD_MANAGER_ARGS                    1

/**
 * @brief define type of Cmd array len, based on max len of Cmd_Array
 */
typedef uint8_t Cmd_LenType;

#define CMD_LIST_ARRAY                      1
#define CMD_LIST_POINTER_ARRAY              2
/**
 * @brief define type of cmd array, CMD_LIST_POINTER_ARRAY, is faster on sorting
 */
#define CMD_LIST_MODE                       CMD_LIST_POINTER_ARRAY
/**
 * @brief temp buffer size in CmdManager_handle
 */
#define CMD_HANDLE_BUFFER_SIZE              48
/**
 * @brief enable cmd type for execute
 */
#define CMD_TYPE_EXE                        1
/**
 * @brief enable cmd type for set
 */
#define CMD_TYPE_SET                        1
/**
 * @brief enable cmd type for get
 */
#define CMD_TYPE_GET                        1
/**
 * @brief enable cmd type for help
 */
#define CMD_TYPE_HELP                       1
/**
 * @brief enable cmd type for response
 */
#define CMD_TYPE_RESP                       1
/**
 * @brief enable cmd type for unknown
 */
#define CMD_TYPE_UNKNOWN                    1

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
struct __Cmd_Cursor;
typedef struct __Cmd_Cursor Cmd_Cursor;
struct __CmdManager;
typedef struct __CmdManager CmdManager;
/**
 * @brief define number of enable types
 */
#define CMD_TYPE_LEN                (CMD_TYPE_EXE + CMD_TYPE_SET + CMD_TYPE_GET + CMD_TYPE_HELP + CMD_TYPE_RESP)
/**
 * @brief define number of enable pattern types
 */
#define CMD_PATTERN_TYPE_LEN        (CMD_TYPE_EXE + CMD_TYPE_SET + CMD_TYPE_GET + CMD_TYPE_HELP + CMD_TYPE_RESP)

#if CMD_LIST_MODE == CMD_LIST_ARRAY
    #define Cmd_Array   Cmd
#else
    #define Cmd_Array   Cmd*
#endif

/**
 * @brief hold command name and len of name
 */
typedef struct {
    const char*     Name;
    Str_LenType     Len;
} Cmd_Str;
typedef enum {
#if CMD_TYPE_EXE
    Cmd_TypeIndex_Execute,
#endif
#if CMD_TYPE_SET
    Cmd_TypeIndex_Set,
#endif
#if CMD_TYPE_GET
    Cmd_TypeIndex_Get,
#endif
#if CMD_TYPE_HELP
    Cmd_TypeIndex_Help,
#endif
#if CMD_TYPE_RESP
    Cmd_TypeIndex_Response,
#endif
#if CMD_TYPE_UNKNOWN
    Cmd_TypeIndex_Unknown,
#endif
} Cmd_TypeIndex;
/**
 * @brief show command type
 */
typedef enum {
#if CMD_TYPE_EXE
    Cmd_Type_Execute        = 1 << Cmd_TypeIndex_Execute,   /**< ex: "<cmd>\r\n" */
#endif
#if CMD_TYPE_SET
    Cmd_Type_Set            = 1 << Cmd_TypeIndex_Set,       /**< ex: "<cmd>=<params>\r\n" */
#endif
#if CMD_TYPE_GET
    Cmd_Type_Get            = 1 << Cmd_TypeIndex_Get,       /**< ex: "<cmd>?\r\n" */
#endif
#if CMD_TYPE_HELP
    Cmd_Type_Help           = 1 << Cmd_TypeIndex_Help,      /**< ex: "<cmd>=?\r\n" */
#endif
#if CMD_TYPE_RESP
    Cmd_Type_Response       = 1 << Cmd_TypeIndex_Response,  /**< ex: "<cmd>: <status>\r\n" */
#endif
#if CMD_TYPE_UNKNOWN
    Cmd_Type_Unknown        = 1 << Cmd_TypeIndex_Unknown,   /**< don't check cmd name ending */
#endif
    Cmd_Type_None           = 0x00,
    Cmd_Type_Any            = 0x1F,                         /**< check all types */
} Cmd_Type;

/**
 * @brief determine need handle multiple ending or not
 */
typedef enum {
    Cmd_Done                = 0,        /**< command end with single ending */
    Cmd_Continue            = 1,        /**< command have multiple ending */
} Cmd_Handled;
/**
 * @brief callback of command
 */
typedef Cmd_Handled (*Cmd_CallbackFn) (CmdManager* manager, Cmd* cmd, Cmd_Cursor* cursor, Cmd_Type type);
typedef void (*Cmd_NotFoundFn) (CmdManager* manager, char* str);
typedef void (*Cmd_OverflowFn) (CmdManager* manager);
/**
 * @brief show type of param
 */
typedef enum {
  Cmd_ValueType_Unknown,        /**< first character of value not match with anyone of supported values */
  Cmd_ValueType_Number,         /**< ex: 13 */
  Cmd_ValueType_NumberHex,      /**< ex: 0xAB25 */
  Cmd_ValueType_NumberBinary,   /**< ex: 0b64813 */
  Cmd_ValueType_Float,          /**< ex: 2.54 */
  Cmd_ValueType_State,          /**< (high, low), ex: high */
  Cmd_ValueType_StateKey,       /**< (on, off), ex: off */
  Cmd_ValueType_Boolean,        /**< (true, false), ex: true */
  Cmd_ValueType_String,         /**< ex: "Text" */
  Cmd_ValueType_Null,           /**< ex: null */
} Cmd_ValueType;
/**
 * @brief hold type of param in same memory
 */
typedef union {
    char*           Unknown;
    int32_t         Number;
    uint32_t        NumberHex;
    uint32_t        NumberBinary;
    float           Float;
    uint8_t         State;
    uint8_t         StateKey;
    uint8_t         Boolean;
    char*           String;
    char*           Null;
} Cmd_Value;
/**
 * @brief show details of param
 */
typedef struct {
    Cmd_Value       Value;
    Cmd_ValueType   ValueType;
    Cmd_LenType     Index;
} Cmd_Param;
/**
 * @brief use for handle params and show current pos
 */
struct __Cmd_Cursor {
    char*           Ptr;
    Str_LenType     Len;
    char            ParamSeperator;
    Cmd_LenType     Index;
};
/**
 * @brief hold callback functions
 */
typedef union {
#if CMD_MULTI_CALLBACK
    Cmd_CallbackFn      fn[CMD_TYPE_LEN];
    struct {
    #if CMD_TYPE_EXE
        Cmd_CallbackFn  execute;
    #endif
    #if CMD_TYPE_SET
        Cmd_CallbackFn  set;
    #endif
    #if CMD_TYPE_GET
        Cmd_CallbackFn  get;
    #endif
    #if CMD_TYPE_HELP
        Cmd_CallbackFn  help;
    #endif
    #if CMD_TYPE_RESP
        Cmd_CallbackFn  response;
    #endif
    #if CMD_TYPE_UNKNOWN
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
    #if CMD_TYPE_EXE
        uint8_t     Execute     : 1;
    #endif
    #if CMD_TYPE_SET
        uint8_t     Set         : 1;
    #endif
    #if CMD_TYPE_GET
        uint8_t     Get         : 1;
    #endif
    #if CMD_TYPE_HELP
        uint8_t     Help        : 1;
    #endif
    #if CMD_TYPE_RESP
        uint8_t     Response    : 1;
    #endif
    #if CMD_TYPE_UNKNOWN
        uint8_t     Unknown     : 1;
    #endif
    };
} Cmd_Types;
/**
 * @brief custom type pattern
 */
typedef union {
    Cmd_Str*            Patterns[CMD_PATTERN_TYPE_LEN];
    struct {
    #if CMD_TYPE_EXE
        Cmd_Str*        Execute;
    #endif
    #if CMD_TYPE_SET
        Cmd_Str*        Set;
    #endif
    #if CMD_TYPE_GET
        Cmd_Str*        Get;
    #endif
    #if CMD_TYPE_HELP
        Cmd_Str*        Help;
    #endif
    #if CMD_TYPE_RESP
        Cmd_Str*        Response;
    #endif
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
struct __CmdManager {
#if CMD_MANAGER_ARGS
    void*               Args;
#endif
    Cmd_PatternTypes*   PatternTypes;
    Cmd_Str*            StartWith;
    Cmd_Str*            EndWith;
    Cmd_NotFoundFn      notFound;
    Cmd_OverflowFn      bufferOverflow;
    Cmd*                InUseCmd;
    Cmd_List            List;
    char                ParamSeperator;
    uint8_t             InUseCmdTypeIndex;
};

/* default types */
extern const Cmd_PatternTypes   CMD_PATTERN_TYPES;
extern const Cmd_Str            CMD_END_WITH;
extern const char               CMD_PARAM_SEPERATOR;
/* pre-processor APIs for const define */
#define CMD_STR_INIT(NAME)          {NAME, sizeof(NAME) - 1}
#define CMD_ARR_LEN(ARR)            (sizeof(ARR) / sizeof(ARR[0]))
#define CMD_LIST_INIT(LIST)         {LIST, CMD_ARR_LEN(LIST)}

#if CMD_MULTI_CALLBACK
    #define CMD_INIT(NAME, TYPES, ...)      {{{__VA_ARGS__}}, CMD_STR_INIT(NAME), (TYPES)}
#else
    #define CMD_INIT(NAME, TYPES, FN)       {{FN}, CMD_STR_INIT(NAME), (TYPES)}
#endif // CMD_MULTI_CALLBACK

void Cmd_init(Cmd* cmd, const char* name, Cmd_Type types);
void Cmd_setTypes(Cmd* cmd, Cmd_Type types);
#if CMD_MULTI_CALLBACK
#if CMD_TYPE_EXE
    void Cmd_onExecute(Cmd* cmd, Cmd_CallbackFn fn);
#endif
#if CMD_TYPE_SET
    void Cmd_onSet(Cmd* cmd, Cmd_CallbackFn fn);
#endif
#if CMD_TYPE_GET
    void Cmd_onGet(Cmd* cmd, Cmd_CallbackFn fn);
#endif
#if CMD_TYPE_HELP
    void Cmd_onHelp(Cmd* cmd, Cmd_CallbackFn fn);
#endif
#if CMD_TYPE_RESP
    void Cmd_onResponse(Cmd* cmd, Cmd_CallbackFn fn);
#endif
#if CMD_TYPE_UNKNOWN
    void Cmd_onUnknown(Cmd* cmd, Cmd_CallbackFn fn);
#endif // CMD_UNKNOWN_CALLBACK

#else
    void Cmd_on(Cmd* cmd, Cmd_CallbackFn fn);
#endif // CMD_MULTI_CALLBACK

void CmdManager_init(CmdManager* manager, Cmd_Array* cmds, Cmd_LenType len);
void CmdManager_setStartWith(CmdManager* manager, Cmd_Str* startWith);
void CmdManager_setEndWith(CmdManager* manager, Cmd_Str* endWith);
void CmdManager_onNotFound(CmdManager* manager, Cmd_NotFoundFn notFound);
void CmdManager_onOverflow(CmdManager* manager, Cmd_OverflowFn overflow);
void CmdManager_setParamSeperator(CmdManager* manager, char sep);
void CmdManager_setCommands(CmdManager* manager, Cmd_Array* cmds, Cmd_LenType len);
void CmdManager_setPatternTypes(CmdManager* manager, Cmd_PatternTypes* patterns);

#if CMD_MANAGER_ARGS
    void  CmdManager_setArgs(CmdManager* manager, void* args);
    void* CmdManager_getArgs(CmdManager* manager);
#endif

void CmdManager_handleStatic(CmdManager* manager, IStream* stream, char* buffer, Str_LenType len, Cmd_Cursor* cursor);
void CmdManager_handle(CmdManager* manager, IStream* stream);

// Cmd_Param parser
Cmd_Param* CmdManager_nextParam(Cmd_Cursor* cursor, Cmd_Param* param);

#if __cplusplus
};
#endif

#endif /* _CMD_MANAGER_H_ */
