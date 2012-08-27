/* @file   perf_test.c
   @author Dmitry S. Melnikov, dmitryme@gmail.com
   @date   Created on: 08/27/2012 05:35:55 PM
*/

#include "fix_parser.h"
#include "fix_msg.h"

#include <stdio.h>
#include <time.h>

int main()
{
   FIXParser* parser = fix_parser_create(512, 0, 2, 0, 1000, 0, 0/*FIXParserFlags_Validate*/);

   int res = fix_protocol_init(parser, "fix44.xml");
   if (res == FIX_FAILED)
   {
      printf("ERROR: %s\n", get_fix_error_text(parser));
      return 1;
   }

   struct timespec start, stop;

   clock_gettime(CLOCK_MONOTONIC_RAW, &start);

   for(int i = 0; i < 100000; ++i)
   {
      FIXMsg* msg = fix_msg_create(parser, FIX44, "8");
      if (!msg)
      {
         printf("ERROR: %s\n", get_fix_error_text(parser));
         return 1;
      }

      fix_msg_set_tag_string(msg, NULL, 49, "QWERTY_12345678");
      fix_msg_set_tag_string(msg, NULL, 56, "ABCQWE_XYZ");
      fix_msg_set_tag_long(msg, NULL, 34, 34);
      fix_msg_set_tag_string(msg, NULL, 57, "srv-ivanov_ii1");
      fix_msg_set_tag_string(msg, NULL, 52, "20120716-06:00:16.230");
      fix_msg_set_tag_string(msg, NULL, 37, "1");
      fix_msg_set_tag_string(msg, NULL, 11, "CL_ORD_ID_1234567");
      fix_msg_set_tag_string(msg, NULL, 17, "FE_1_9494_1");
      fix_msg_set_tag_char(msg, NULL, 150, '0');
      fix_msg_set_tag_char(msg, NULL, 39, '1');
      fix_msg_set_tag_string(msg, NULL, 1, "ZUM");
      fix_msg_set_tag_string(msg, NULL, 55, "RTS-12.12");
      fix_msg_set_tag_char(msg, NULL, 54, '1');
      fix_msg_set_tag_float(msg, NULL, 38, 25);
      fix_msg_set_tag_float(msg, NULL, 44, 135155.0);
      fix_msg_set_tag_char(msg, NULL, 59, '0');
      fix_msg_set_tag_float(msg, NULL, 32, 0);
      fix_msg_set_tag_float(msg, NULL, 31, 0.0);
      fix_msg_set_tag_float(msg, NULL, 151, 25.0);
      fix_msg_set_tag_float(msg, NULL, 14, 0);
      fix_msg_set_tag_float(msg, NULL, 6, 0.0);
      fix_msg_set_tag_char(msg, NULL, 21, '1');
      fix_msg_set_tag_string(msg, NULL, 58, "COMMENT12");
      fix_msg_free(msg);
   }

   clock_gettime(CLOCK_MONOTONIC_RAW, &stop);

   printf("TM %ld\n", (stop.tv_sec - start.tv_sec) * 1000000 + (stop.tv_nsec - start.tv_nsec) / 1000);

   return 0;
}