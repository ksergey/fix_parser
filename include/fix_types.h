/* @file   fix_types.h
   @author Dmitry S. Melnikov, dmitryme@gmail.com
   @date   Created on: 07/31/2012 10:22:19 AM
*/

#ifndef FIX_PARSER_FIX_TYPES_H
#define FIX_PARSER_FIX_TYPES_H

#include "fix_field_tag.h"

#define FIX_FAILED                        -1
#define FIX_SUCCESS                        0
#define FIX_ERROR_FIELD_HAS_WRONG_TYPE     1
#define FIX_ERROR_FIELD_NOT_FOUND          2
#define FIX_ERROR_GROUP_WRONG_INDEX        3
#define FIX_ERROR_XML_ATTR_NOT_FOUND       4
#define FIX_ERROR_XML_ATTR_WRONG_VALUE     5
#define FIX_ERROR_PROTOCOL_XML_LOAD_FAILED 6
#define FIX_ERROR_UNKNOWN_FIELD            7
#define FIX_ERROR_WRONG_PROTOCOL_VER       8
#define FIX_ERROR_DUPLICATE_FIELD_DESCR    9
#define FIX_ERROR_UNKNOWN_MSG             10
#define FIX_ERROR_LIBXML                  11
#define FIX_ERROR_INVALID_ARGUMENT        12
#define FIX_ERROR_MALLOC                  13
#define FIX_ERROR_UNKNOWN_PROTOCOL_DESCR  14
#define FIX_ERROR_NO_MORE_PAGES           15
#define FIX_ERROR_NO_MORE_GROUPS          16
#define FIX_ERROR_TOO_BIG_PAGE            17
#define FIX_ERROR_NO_MORE_SPACE           18
#define FIX_ERROR_PARSE_MSG               19
#define FIX_ERROR_WRONG_FIELD             20

typedef struct FIXGroup_ FIXGroup;
typedef struct FIXField_ FIXField;
typedef struct FIXMsg_ FIXMsg;
typedef struct FIXParser_ FIXParser;

#define PARSER_FLAG_CHECK_CRC 0x01
#define PARSER_FLAG_CHECK_REQUIRED 0x02
#define PARSER_FLAG_CHECK_EXISTING 0x04
#define PARSER_FLAG_CHECK_VALUE    0x08
#define PARSER_FLAG_CHECK_ALL (PARSER_FLAG_CHECK_CRC | PARSER_FLAG_CHECK_REQUIRED | PARSER_FLAG_CHECK_EXISTING | PARSER_FLAG_CHECK_VALUE)

typedef enum FIXFieldTypeEnum
{
   FIXFieldType_Value = 1,
   FIXFieldType_Group = 2
} FIXFieldTypeEnum;

typedef enum FIXProtocolVerEnum
{
   FIX42,
   FIX44,
   FIX50,
   FIX50SP1,
   FIX50SP2,
   FIXT11,
   FIX_MUST_BE_LAST_DO_NOT_USE_OR_CHANGE_IT
} FIXProtocolVerEnum;

typedef enum FIXFieldValueTypeEnum
{
   FIXFieldValueType_Unknown              = -1,
   FIXFieldValueType_Int                  = 0x01,
   FIXFieldValueType_Length               = 0x02,
   FIXFieldValueType_NumInGroup           = 0x03,
   FIXFieldValueType_SeqNum               = 0x04,
   FIXFieldValueType_TagNum               = 0x05,
   FIXFieldValueType_DayOfMonth           = 0x06,
   FIXFieldValueType_Float                = 0x10,
   FIXFieldValueType_Qty                  = 0x20,
   FIXFieldValueType_Price                = 0x30,
   FIXFieldValueType_PriceOffset          = 0x40,
   FIXFieldValueType_Amt                  = 0x50,
   FIXFieldValueType_Percentage           = 0x60,
   FIXFieldValueType_Char                 = 0x100,
   FIXFieldValueType_Boolean              = 0x200,
   FIXFieldValueType_String               = 0x1000,
   FIXFieldValueType_MultipleValueString  = 0x2000,
   FIXFieldValueType_Country              = 0x3000,
   FIXFieldValueType_Currency             = 0x4000,
   FIXFieldValueType_Exchange             = 0x5000,
   FIXFieldValueType_MonthYear            = 0x6000,
   FIXFieldValueType_UTCTimestamp         = 0x7000,
   FIXFieldValueType_UTCTimeOnly          = 0x8000,
   FIXFieldValueType_UTCDateOnly          = 0x9000,
   FIXFieldValueType_LocalMktDate         = 0xA000,
   FIXFieldValueType_TZTimeOnly           = 0xB000,
   FIXFieldValueType_TZTimestamp          = 0xC000,
   FIXFieldValueType_Language             = 0xD000,
   FIXFieldValueType_XMLData              = 0x20000,
   FIXFieldValueType_Data                 = 0x10000
} FIXFieldValueTypeEnum;

#define IS_STRING_TYPE(type) ((type & 0xF000) > 0)
#define IS_INT_TYPE(type)    ((type & 0x0F) > 0)
#define IS_FLOAT_TYPE(type)  ((type & 0xF0) > 0)
#define IS_CHAR_TYPE(type)   ((type & 0xF00) > 0)
#define IS_DATA_TYPE(type)   ((type & 0xF0000) > 0)

FIXProtocolVerEnum str2FIXProtocolVerEnum(char const* ver);
char const* FIXProtocolVerEnum2BeginString(FIXProtocolVerEnum ver);
FIXFieldValueTypeEnum str2FIXFieldValueType(char const* type);

#endif /* FIX_PARSER_FIX_TYPES_H */