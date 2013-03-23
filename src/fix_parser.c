/**
 * @file   fix_parser.c
 * @author Dmitry S. Melnikov, dmitryme@gmail.com
 * @date   Created on: 07/30/2012 10:26:54 AM
 */

#include "fix_parser.h"
#include "fix_parser_priv.h"
#include "fix_protocol_descr.h"
#include "fix_msg.h"
#include "fix_msg_priv.h"
#include "fix_page.h"
#include "fix_utils.h"
#include "fix_error_priv.h"
#include "fix_field_tag.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define CRC_FIELD_LEN 7

/*------------------------------------------------------------------------------------------------------------------------*/
FIX_PARSER_API FIXParser* fix_parser_create(char const* protFile, FIXParserAttrs const* attrs, int32_t flags, FIXError** error)
{
   FIXParserAttrs myattrs = {};
   if (attrs)
   {
      memcpy(&myattrs, attrs, sizeof(myattrs));
   }
   FIXParser* parser = NULL;
   FIXError* err = (FIXError*)calloc(1, sizeof(FIXError));
   if (fix_parser_validate_attrs(err, &myattrs) == FIX_FAILED)
   {
      goto failed;
   }
   parser = (FIXParser*)calloc(1, sizeof(FIXParser));
   memcpy(&parser->attrs, &myattrs, sizeof(parser->attrs));
   parser->flags = flags;
   parser->protocol = fix_protocol_descr_create(err, protFile);
   if (!parser->protocol)
   {
      goto failed;
   }
   for(uint32_t i = 0; i < parser->attrs.numPages; ++i)
   {
      FIXPage* page = (FIXPage*)calloc(1, sizeof(FIXPage) + parser->attrs.pageSize - 1);
      page->size = parser->attrs.pageSize;
      page->next = parser->page;
      parser->page = page;
   }
   for(uint32_t i = 0; i < parser->attrs.numGroups; ++i)
   {
      FIXGroup* group = (FIXGroup*)calloc(1, sizeof(FIXGroup));
      group->next = parser->group;
      parser->group = group;
   }
   goto ok;
failed:
   if (error)
   {
      *error = err;
   }
   if (parser)
   {
      free(parser);
      parser = NULL;
   }
ok:
   return parser;
}

/*------------------------------------------------------------------------------------------------------------------------*/
FIX_PARSER_API void fix_parser_free(FIXParser* parser)
{
   if (parser)
   {
      if (parser->protocol)
      {
         fix_protocol_descr_free(parser->protocol);
      }
      FIXPage* page = parser->page;
      while(page)
      {
         FIXPage* next = page->next;
         free(page);
         page = next;
      }
      FIXGroup* group = parser->group;
      while(group)
      {
         FIXGroup* next = group->next;
         free(group);
         group = next;
      }
      free(parser);
   }
}

/*------------------------------------------------------------------------------------------------------------------------*/
FIX_PARSER_API char const* fix_parser_get_protocol_ver(FIXParser* parser)
{
   if (!parser)
   {
      return NULL;
   }
   return parser->protocol->version;
}

/*------------------------------------------------------------------------------------------------------------------------*/
FIX_PARSER_API FIXErrCode fix_parser_get_header(char const* data, uint32_t len, char delimiter,
      char const** beginString, uint32_t* beginStringLen,
      char const** msgType, uint32_t *msgTypeLen,
      char const** senderCompID, uint32_t* senderCompIDLen,
      char const** targetCompID, uint32_t* targetCompIDLen,
      int64_t* msgSeqNum, FIXError** error)
{
   FIXError* err = (FIXError*)calloc(1, sizeof(FIXError));
   if (!error || !data || !beginString || !beginStringLen || !msgType || !msgTypeLen || !senderCompID ||
         !targetCompID || !senderCompIDLen || !targetCompIDLen || !msgSeqNum)
   {
      fix_error_set(err, FIX_ERROR_INVALID_ARGUMENT, "One of arguments is NULL.");
      goto failed;
   }
   FIXTagNum tag = 0;
   char const* dbegin = NULL;
   char const* dend = NULL;
   tag = fix_parser_parse_mandatory_field(data, len, delimiter, &dbegin, &dend, err);
   if (tag == FIX_FAILED)
   {
      fix_error_set(err, FIX_ERROR_PARSE_MSG, "Unable to parse BeginString field.");
      goto failed;
   }
   if (tag != FIXFieldTag_BeginString)
   {
      fix_error_set(err, FIX_ERROR_WRONG_FIELD, "First field is '%d', but must be BeginString.", tag);
      goto failed;
   }
   *beginString = dbegin;
   *beginStringLen = dend - dbegin;
   tag = fix_parser_parse_mandatory_field(dend + 1, len - (dend + 1 - data), delimiter, &dbegin, &dend, err);
   if (tag == FIX_FAILED)
   {
      fix_error_set(err, FIX_ERROR_PARSE_MSG, "Unable to parse BodyLength field.");
      goto failed;
   }
   if (tag != FIXFieldTag_BodyLength)
   {
      fix_error_set(err, FIX_ERROR_WRONG_FIELD, "Second field is '%d', but must be BodyLength.", tag);
      goto failed;
   }
   int64_t bodyLen;
   if (fix_utils_atoi64(dbegin, dend - dbegin, 0, &bodyLen) == FIX_FAILED)
   {
      fix_error_set(err, FIX_ERROR_PARSE_MSG, "BodyLength value not a number.");
      goto failed;
   }
   if (bodyLen + CRC_FIELD_LEN > len - (dend - data))
   {
      fix_error_set(err, FIX_ERROR_DATA_TOO_SHORT, "Data too short.");
      goto failed;
   }
   char const* bodyEnd = dend + bodyLen;
   tag = fix_parser_parse_mandatory_field(dend + 1, bodyEnd - dend, delimiter, &dbegin, &dend, err);
   if (tag == FIX_FAILED)
   {
      fix_error_set(err, FIX_ERROR_PARSE_MSG, "Unable to parse MsgType field.");
      goto failed;
   }
   if (tag != FIXFieldTag_MsgType)
   {
      fix_error_set(err, FIX_ERROR_WRONG_FIELD, "Field is '%d', but must be MsgType.", tag);
      goto failed;
   }
   *msgType = dbegin;
   *msgTypeLen = dend - dbegin;
   *senderCompID = NULL;
   *targetCompID = NULL;
   *msgSeqNum = 0;
   while(dend != bodyEnd)
   {
      tag = fix_parser_parse_mandatory_field(dend + 1, bodyEnd - dend, delimiter, &dbegin, &dend, err);
      if (tag == FIX_FAILED)
      {
         goto failed;
      }
      if (tag == FIXFieldTag_SenderCompID)
      {
         *senderCompID = dbegin;
         *senderCompIDLen = dend - dbegin;
      }
      else if (tag == FIXFieldTag_TargetCompID)
      {
         *targetCompID = dbegin;
         *targetCompIDLen = dend - dbegin;
      }
      else if (tag == FIXFieldTag_MsgSeqNum)
      {
         if (fix_utils_atoi64(dbegin, dend - dbegin, 0, msgSeqNum) == FIX_FAILED)
         {
            fix_error_set(err, FIX_ERROR_WRONG_FIELD, "Wrong MsgSeqNum.");
            goto failed;
         }
      }
      if (*msgType && *senderCompID && *targetCompID && *msgSeqNum)
      {
         goto ok;
      }
   }
   if (!(*senderCompID))
   {
      fix_error_set(err, FIX_ERROR_WRONG_FIELD, "Unable to find SenderCompID field.");
      goto failed;
   }
   if (!(*targetCompID))
   {
      fix_error_set(err, FIX_ERROR_WRONG_FIELD, "Unable to find TargetCompID field.");
      goto failed;
   }
   if (!(*msgSeqNum))
   {
      fix_error_set(err, FIX_ERROR_WRONG_FIELD, "Unable to find MsgSeqNum field.");
      goto failed;
   }
failed:
   *error = err;
   return FIX_FAILED;
ok:
   return FIX_SUCCESS;
}

/*------------------------------------------------------------------------------------------------------------------------*/
FIX_PARSER_API FIXMsg* fix_parser_str_to_msg(FIXParser* parser, char const* data, uint32_t len, char delimiter, char const** stop)
{
   if (!parser || !data)
   {
      return NULL;
   }
   fix_error_reset(&parser->error);
   FIXTagNum tag = 0;
   char const* dbegin = NULL;
   char const* dend = NULL;
   tag = fix_parser_parse_mandatory_field(data, len, delimiter, &dbegin, &dend, &parser->error);
   if (tag == FIX_FAILED)
   {
      fix_error_set(&parser->error, FIX_ERROR_PARSE_MSG, "Unable to parse BeginString field.");
      return NULL;
   }
   if (tag != FIXFieldTag_BeginString)
   {
      fix_error_set(&parser->error, FIX_ERROR_WRONG_FIELD, "First field is '%d', but must be BeginString.", tag);
      return NULL;
   }
   if (strncmp(parser->protocol->transportVersion, dbegin, dend - dbegin))
   {
      char* actualVer = (char*)calloc(dend - dbegin + 1, 1);
      memcpy(actualVer, dbegin, dend - dbegin);
      fix_error_set(
            &parser->error,
            FIX_ERROR_WRONG_PROTOCOL_VER,
            "Wrong protocol. Expected '%s', actual '%s'.", parser->protocol->transportVersion, actualVer);
      free(actualVer);
      return NULL;
   }
   tag = fix_parser_parse_mandatory_field(dend + 1, len - (dend + 1 - data), delimiter, &dbegin, &dend, &parser->error);
   if (tag == FIX_FAILED)
   {
      fix_error_set(&parser->error, FIX_ERROR_PARSE_MSG, "Unable to parse BodyLength field.");
      return NULL;
   }
   if (tag != FIXFieldTag_BodyLength)
   {
      fix_error_set(&parser->error, FIX_ERROR_WRONG_FIELD, "Second field is '%d', but must be BodyLength.", tag);
      return NULL;
   }
   int64_t bodyLen;
   if (fix_utils_atoi64(dbegin, dend - dbegin, 0, &bodyLen) == FIX_FAILED)
   {
      fix_error_set(&parser->error, FIX_ERROR_PARSE_MSG, "BodyLength value not a number.");
      return NULL;
   }
   if (bodyLen + CRC_FIELD_LEN > len - (dend - data))
   {
      fix_error_set(&parser->error, FIX_ERROR_DATA_TOO_SHORT, "Data too short.");
      *stop = data + len;
      return NULL;
   }
   char const* crcbeg = NULL;
   char const* bodyEnd = dend + bodyLen;
   tag = fix_parser_parse_mandatory_field(bodyEnd + 1, CRC_FIELD_LEN, delimiter, &crcbeg, stop, &parser->error);
   if (tag == FIX_FAILED)
   {
      fix_error_set(&parser->error, FIX_ERROR_PARSE_MSG, "Unable to parse CrcSum field.");
      return NULL;
   }
   if (tag != FIXFieldTag_CheckSum)
   {
      fix_error_set(&parser->error, FIX_ERROR_WRONG_FIELD, "Field is '%d', but must be CrcSum.", tag);
      return NULL;
   }
   if (parser->flags & PARSER_FLAG_CHECK_CRC)
   {
      int64_t check_sum = 0;
      if (fix_utils_atoi64(crcbeg, *stop - crcbeg, 0, &check_sum) == FIX_FAILED)
      {
         fix_error_set(&parser->error, FIX_ERROR_PARSE_MSG, "CheckSum value not a tagber.");
         return NULL;
      }
      int64_t crc = 0;
      for(char const* it = data; it <= bodyEnd; ++it)
      {
         crc += *it;
      }
      crc %= 256;
      if (crc != check_sum)
      {
         fix_error_set(
               &parser->error,
               FIX_ERROR_INTEGRITY_CHECK, "CheckSum check failed. Expected '%d', actual '%d'.", check_sum, crc);
         return NULL;
      }
   }
   tag = fix_parser_parse_mandatory_field(dend + 1, bodyEnd - dend, delimiter, &dbegin, &dend, &parser->error);
   if (tag == FIX_FAILED)
   {
      fix_error_set(&parser->error, FIX_ERROR_PARSE_MSG, "Unable to parse MsgType field.");
      return NULL;
   }
   if (tag != FIXFieldTag_MsgType)
   {
      fix_error_set(&parser->error, FIX_ERROR_WRONG_FIELD, "Field is '%d', but must be MsgType.", tag);
      return NULL;
   }
   char* msgType = (char*)calloc(dend - dbegin + 1, 1);
   memcpy(msgType, dbegin, dend - dbegin);
   FIXMsg* msg = fix_msg_create(parser, msgType);
   free(msgType);
   if (!msg)
   {
      return NULL;
   }
   if (fix_msg_set_int32(msg, NULL, FIXFieldTag_BodyLength, bodyLen) != FIX_SUCCESS)
   {
      goto error;
   }
   FIXFieldDescr const* fdescr  = fix_protocol_get_field_descr(&parser->error, msg->descr, FIXFieldTag_CheckSum);
   if (!fdescr)
   {
      fix_error_set(&parser->error, FIX_ERROR_UNKNOWN_FIELD, "Field with tag %d not found in message '%s' description.",
            tag, msg->descr->name);
      goto error;
   }
   char crc[4] = {};
   memcpy(crc, crcbeg, *stop - crcbeg);
   if (!fix_field_set(msg, NULL, fdescr, (unsigned char*)crcbeg, *stop - crcbeg))
   {
      goto error;
   }
   while(dend != bodyEnd)
   {
      fdescr = NULL;
      tag = fix_parser_parse_field(parser, msg, NULL, dend + 1, bodyEnd - dend, delimiter, &fdescr, &dbegin, &dend);
      if (tag == FIX_FAILED)
      {
         goto error;
      }
      if (fdescr) // if !fdescr, ignore this field
      {
         if (parser->flags & PARSER_FLAG_CHECK_VALUE)
         {
            if (fix_parser_check_value(&parser->error, fdescr, dbegin, dend, delimiter) == FIX_FAILED)
            {
               goto error;
            }
         }
         if (fdescr->category == FIXFieldCategory_Value)
         {
            if (!fix_field_set(msg, NULL, fdescr, (unsigned char*)dbegin, dend - dbegin))
            {
               goto error;
            }
         }
         else if (fdescr->category == FIXFieldCategory_Group)
         {
            int64_t numGroups = 0;
            if (FIX_FAILED == fix_utils_atoi64(dbegin, dend - dbegin, delimiter, &numGroups))
            {
               fix_error_set(&parser->error, FIX_ERROR_INVALID_ARGUMENT, "Unable to get group tag %d value.", tag);
               goto  error;
            }
            if (FIX_FAILED == fix_parser_parse_group(parser, msg, NULL, fdescr, numGroups, dend, bodyEnd - dend, delimiter, &dend))
            {
               goto error;
            }
         }
      }
   }
   if (parser->flags & PARSER_FLAG_CHECK_REQUIRED)
   {
      for(uint32_t i = 0; i < msg->descr->field_count; ++i)
      {
         FIXFieldDescr* fdescr = &msg->descr->fields[i];
         if (fdescr->flags & FIELD_FLAG_REQUIRED && !fix_field_get(msg, NULL, fdescr->type->tag))
         {
            fix_error_set(&parser->error, FIX_ERROR_UNKNOWN_FIELD, "Required field '%s' not found.", fdescr->type->name);
            goto error;
         }
      }
   }
   return msg;
error:
   if (msg)
   {
      fix_msg_free(msg);
      msg = NULL;
   }
   return msg;
}

/*------------------------------------------------------------------------------------------------------------------------*/
FIX_PARSER_API FIXErrCode fix_parser_get_error_code(FIXParser* parser)
{
   return parser->error.code;
}

/*------------------------------------------------------------------------------------------------------------------------*/
FIX_PARSER_API char const* fix_parser_get_error_text(FIXParser* parser)
{
   return parser->error.text;
}
