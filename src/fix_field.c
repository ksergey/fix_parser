/* @file   fix_field.c
   @author Dmitry S. Melnikov, dmitryme@gmail.com
   @date   Created on: 07/25/2012 03:35:31 PM
*/

#include "fix_field.h"
#include "fix_parser.h"
#include "fix_parser_priv.h"
#include "fix_msg_priv.h"
#include "fix_utils.h"

#include <stdlib.h>
#include <string.h>

FIXField* fix_field_free(FIXMsg* msg, FIXField* field);
void fix_group_free(FIXMsg* msg, FIXGroup* group);

/*-----------------------------------------------------------------------------------------------------------------------*/
/* PUBLICS                                                                                                               */
/*-----------------------------------------------------------------------------------------------------------------------*/
FIXField* fix_field_set(FIXMsg* msg, FIXGroup* grp, uint32_t tag, unsigned char const* data, uint32_t len)
{
   FIXField* field = fix_field_get(msg, grp, tag);
   if (!field && get_fix_error_code(msg->parser))
   {
      return NULL;
   }
   if (field && field->type == FIXFieldType_Group)
   {
      fix_parser_set_error(msg->parser, FIX_ERROR_FIELD_HAS_WRONG_TYPE, "FIXField has wrong type");
      return NULL;
   }
   int32_t idx = tag % GROUP_SIZE;
   if (!field)
   {
      field = fix_msg_alloc(msg, sizeof(FIXField));
      if (!field)
      {
         return NULL;
      }
      field->type = FIXFieldType_Value;
      FIXGroup* group = (grp ? grp : msg->fields);
      field->next = group->fields[idx];
      field->tag = tag;
      group->fields[idx] = field;
      field->size = len;
      field->data = fix_msg_alloc(msg, len);
      field->body_len = 0;
   }
   else
   {
      field->size = len;
      field->data = fix_msg_realloc(msg, field->data, len);
      msg->body_len -= field->body_len;
   }
   if (!field->data)
   {
      return NULL;
   }
   if (LIKE(field->tag != FIXFieldTag_BeginString && field->tag != FIXFieldTag_BodyLength && field->tag != FIXFieldTag_CheckSum))
   {
      field->body_len = fix_utils_numdigits(tag) + 1 + len + 1;
   }
   memcpy(field->data, data, len);
   msg->body_len += field->body_len;
   return field;
}

/*------------------------------------------------------------------------------------------------------------------------*/
int32_t fix_field_del(FIXMsg* msg, FIXGroup* grp, uint32_t tag)
{
   uint32_t const idx = tag % GROUP_SIZE;
   FIXGroup* group = grp ? grp : msg->fields;
   FIXField* field = group->fields[idx];
   FIXField* prev = field;
   while(field)
   {
      if (field->tag == tag)
      {
         if (prev == field)
         {
            group->fields[idx] = fix_field_free(msg, field);
         }
         else
         {
            prev->next = fix_field_free(msg, field);
         }
         return FIX_SUCCESS;
      }
      prev = field;
      field = field->next;
   }
   fix_parser_set_error(msg->parser, FIX_ERROR_FIELD_NOT_FOUND, "FIXField not found");
   return FIX_FAILED;
}

/*------------------------------------------------------------------------------------------------------------------------*/
FIXGroup* fix_group_add(FIXMsg* msg, FIXGroup* grp, uint32_t tag, FIXField** fld)
{
   FIXField* field = fix_field_get(msg, grp, tag);
   FIXGroup* group = grp ? grp : msg->fields;
   if (field && field->type != FIXFieldType_Group)
   {
      fix_parser_set_error(msg->parser, FIX_ERROR_FIELD_HAS_WRONG_TYPE, "FIXField has wrong type");
      return NULL;
   }
   if (!field)
   {
      uint32_t const idx = tag % GROUP_SIZE;
      field = fix_msg_alloc(msg, sizeof(FIXField));
      field->type = FIXFieldType_Group;
      field->tag = tag;
      field->next = group->fields[idx];
      group->fields[idx] = field;
      field->data = fix_msg_alloc(msg, sizeof(FIXGroups));
      if (!field->data)
      {
         return NULL;
      }
      FIXGroups* grps = (FIXGroups*)field->data;
      field->size = 1;
      field->body_len = 0;
      grps->group[0] = fix_msg_alloc_group(msg);
      if (!grps->group[0])
      {
         return NULL;
      }
   }
   else
   {
      FIXGroups* grps = (FIXGroups*)field->data;
      FIXGroups* new_grps = fix_msg_realloc(msg, field->data, sizeof(FIXGroups) + sizeof(FIXGroup*) * (field->size + 1));
      memcpy(new_grps->group, grps->group, sizeof(FIXGroup*) * (field->size));
      new_grps->group[field->size] = fix_msg_alloc_group(msg);
      if (!new_grps->group[field->size])
      {
         return NULL;
      }
      ++field->size;
      field->data = new_grps;
      msg->body_len -= field->body_len;
   }
   if (LIKE(field->tag != FIXFieldTag_BeginString && field->tag != FIXFieldTag_BodyLength && field->tag != FIXFieldTag_CheckSum))
   {
      field->body_len = fix_utils_numdigits(tag) + 1 + fix_utils_numdigits(field->size) + 1;
   }
   msg->body_len += field->body_len;
   FIXGroups* grps = (FIXGroups*)field->data;
   FIXGroup* new_grp = grps->group[field->size - 1];
   *fld = field;
   return new_grp;
}

/*------------------------------------------------------------------------------------------------------------------------*/
FIXGroup* fix_group_get(FIXMsg* msg, FIXGroup* grp, uint32_t tag, uint32_t grpIdx)
{
   int32_t const idx = tag % GROUP_SIZE;
   FIXGroup* group = (grp ? grp : msg->fields);
   FIXField* it = group->fields[idx];
   while(it)
   {
      if (it->tag == tag)
      {
         if (it->type != FIXFieldType_Group)
         {
            fix_parser_set_error(msg->parser, FIX_ERROR_FIELD_HAS_WRONG_TYPE, "FIXField has wrong type");
            return NULL;
         }
         FIXGroups* grps = (FIXGroups*)it->data;
         if (grpIdx >= it->size)
         {
            fix_parser_set_error(msg->parser, FIX_ERROR_GROUP_WRONG_INDEX, "Wrong index");
            return NULL;
         }
         return grps->group[grpIdx];
      }
      else
      {
         it = it->next;
      }
   }
   return NULL;
}

/*------------------------------------------------------------------------------------------------------------------------*/
int32_t fix_group_del(FIXMsg* msg, FIXGroup* grp, uint32_t tag, uint32_t grpIdx)
{
   FIXField* field = fix_field_get(msg, grp, tag);
   if (!field)
   {
      return FIX_FAILED;
   }
   if (field->type != FIXFieldType_Group)
   {
      fix_parser_set_error(msg->parser, FIX_ERROR_FIELD_HAS_WRONG_TYPE, "FIXField has wrong type");
      return FIX_FAILED;
   }
   FIXGroups* grps = (FIXGroups*)field->data;
   fix_group_free(msg, grps->group[grpIdx]);
   field->size -= 1;
   if (field->size == grpIdx)
   {
      grps->group[field->size] = NULL;
   }
   else
   {
      memcpy((char*)grps->group + sizeof(FIXGroup*) * grpIdx, (char*)grps->group + sizeof(FIXGroup*) * (grpIdx + 1),
         sizeof(FIXGroup*) * (field->size - grpIdx));
      grps->group[field->size] = NULL;
   }
   if (!field->size) // all groups has been deleted, so tag will be deleted either
   {
      return fix_field_del(msg, grp, tag);
   }
   else
   {
      msg->body_len -= field->body_len;
      field->body_len = fix_utils_numdigits(tag) + 1 + fix_utils_numdigits(field->size) + 1;
      msg->body_len += field->body_len;
   }
   return FIX_SUCCESS;
}

/*------------------------------------------------------------------------------------------------------------------------*/
/* PRIVATES                                                                                                               */
/*------------------------------------------------------------------------------------------------------------------------*/
FIXField* fix_field_free(FIXMsg* msg, FIXField* field)
{
   if (field->type == FIXFieldType_Group)
   {
      FIXGroups* grps = (FIXGroups*)field->data;
      for(int32_t i = 0; i < field->size; ++i)
      {
         fix_group_free(msg, grps->group[i]);
      }
   }
   msg->body_len -= field->body_len;
   return field->next;
}

/*------------------------------------------------------------------------------------------------------------------------*/
void fix_group_free(FIXMsg* msg, FIXGroup* group)
{
   for(int32_t i = 0; i < GROUP_SIZE; ++i)
   {
      FIXField* tag = group->fields[i];
      while(tag)
      {
         tag = fix_field_free(msg, tag);
      }
   }
   fix_msg_free_group(msg, group);
}