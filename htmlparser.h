#ifndef __HTMLPARSER_H
#define __HTMLPARSER_H
#include <stdlib.h>
#include <stdint.h>

typedef struct {
  uint32_t start;
  size_t length;
} fragment_t;

typedef struct {
  fragment_t key;
  fragment_t value;

#define k_start  key.start
#define k_length key.length
#define v_start  value.start
#define v_length value.length
} html_attribute_t;

typedef enum {
  OPEN_TAG,
  CLOSE_TAG,
  SINGLETON_TAG,
  TEXT,
  COMMENT,
  SECTION,
  PROCESS,
  DECLARATION
} html_event_type_t;

typedef struct {
  html_event_type_t type;
  fragment_t fragment;
  uint32_t attr_pos; /* Only used for START_TAG */

#define e_start  fragment.start
#define e_length fragment.length
} html_event_t;

typedef struct {
  char *buf;
  uint32_t position;
  size_t length;
  fragment_t fragment;
} html_parser_t;

uint8_t html_parser_new(html_parser_t *parser, char *buf, size_t length);
uint8_t html_parser_free(html_parser_t *parser);
uint8_t html_parser_get_next(html_parser_t *parser, html_event_t *event);
uint8_t html_parser_get_tag(html_parser_t *parser, html_event_t *event, fragment_t *frag);
uint8_t html_parser_get_next_attribute(html_parser_t *parser, html_event_t *event, html_attribute_t *attributes);
#endif
