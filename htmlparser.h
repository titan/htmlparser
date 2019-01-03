#ifndef __HTMLPARSER_H
#define __HTMLPARSER_H
#include <stdlib.h>
#include <stdint.h>

struct html_fragment {
  uint32_t start;
  size_t length;
};

struct html_attribute {
  struct html_fragment key;
  struct html_fragment value;

#define k_start  key.start
#define k_length key.length
#define v_start  value.start
#define v_length value.length
};

enum html_event_type {
  OPEN_TAG,
  CLOSE_TAG,
  SINGLETON_TAG,
  TEXT,
  COMMENT,
  SECTION,
  PROCESS,
  DECLARATION
};

struct html_event {
  enum html_event_type type;
  struct html_fragment fragment;
  uint32_t attr_pos; /* Only used for START_TAG */

#define e_start  fragment.start
#define e_length fragment.length
};

struct html_parser {
  char * buf;
  uint32_t position;
  size_t length;
  struct html_fragment fragment;
};

uint8_t html_parser_new(struct html_parser * parser, char * buf, size_t length);
uint8_t html_parser_free(struct html_parser * parser);
uint8_t html_parser_get_next(struct html_parser * parser, struct html_event * event);
uint8_t html_parser_get_tag(struct html_parser * parser, struct html_event * event, struct html_fragment * frag);
uint8_t html_parser_get_next_attribute(struct html_parser * parser, struct html_event * event, struct html_attribute * attributes);
#endif
