#include <ctype.h>
#include "htmlparser.h"
#include "html-fsm.h"
#include "attr-fsm.h"

uint8_t html_parser_new(html_parser_t *parser, char *buf, size_t length) {
  parser->buf = buf;
  parser->position = 0;
  parser->length = length;
  return 1;
}

uint8_t html_parser_free(html_parser_t *parser) {
  (void) parser;
  return 1;
}

uint8_t html_parser_get_next(html_parser_t *parser, html_event_t *event) {
  void * data[2];
  data[0] = parser;
  data[1] = event;
  char *buf = parser->buf;
  uint32_t html_state = HTML_READY_STATE;
  parser->fragment.start = 0;
  parser->fragment.length = 0;

 RETRY:
  if (parser->position >= parser->length) {
    return 0;
  }

  switch (buf[parser->position]) {
  case '<':
    html_state = html_transform_state(html_state, HTML_LESS_THAN_EVENT, data);
    break;
  case '>':
    html_state = html_transform_state(html_state, HTML_GREATER_THAN_EVENT, data);
    break;
  case '[':
    html_state = html_transform_state(html_state, HTML_OPEN_BRACKET_EVENT, data);
    break;
  case ']':
    html_state = html_transform_state(html_state, HTML_CLOSE_BRACKET_EVENT, data);
    break;
  case '!':
    html_state = html_transform_state(html_state, HTML_EXCLAM_EVENT, data);
    break;
  case '-':
    html_state = html_transform_state(html_state, HTML_MINUS_EVENT, data);
    break;
  case '?':
    html_state = html_transform_state(html_state, HTML_QUESTION_EVENT, data);
    break;
  case '/':
    html_state = html_transform_state(html_state, HTML_SLASH_EVENT, data);
    break;
  case ' ':
  case '\t':
  case '\r':
  case '\n':
    html_state = html_transform_state(html_state, HTML_SPACES_EVENT, data);
    break;
  default:
    html_state = html_transform_state(html_state, HTML_OTHERS_EVENT, data);
    break;
  }

  parser->position ++;
  if (html_state == HTML_READY_STATE) {
    return 1;
  } else {
    goto RETRY;
  }
}

void html_do_action(enum HTML_ACTION action, void * data) {
  html_parser_t * parser = (html_parser_t *) ((void **) data)[0];
  html_event_t * event = (html_event_t *) ((void **) data)[1];
  switch (action) {
  case HTML_START_FRAGMENT_ACTION:
    parser->fragment.start = parser->position;
    parser->fragment.length = 1;
    break;
  case HTML_APPEND_ACTION:
    parser->fragment.length ++;
    break;
  case HTML_APPEND_SLASH_NEWLINE_APPEND_ACTION:
    parser->fragment.length += 2;
    break;
  case HTML_BEGIN_TAG_ACTION:
    event->type = BEGIN_TAG;
    event->e_start = parser->fragment.start;
    event->e_length = parser->fragment.length;
    event->attr_pos = 0;
    break;
  case HTML_SINGLETON_TAG_ACTION:
    event->type = SINGLETON_TAG;
    event->e_start = parser->fragment.start;
    event->e_length = parser->fragment.length;
    event->attr_pos = 0;
    break;
  case HTML_END_TAG_ACTION:
    event->type = END_TAG;
    event->e_start = parser->fragment.start;
    event->e_length = parser->fragment.length;
    break;
  case HTML_DECLARATION_ACTION:
    event->type = DECLARATION;
    event->e_start = parser->fragment.start;
    event->e_length = parser->fragment.length;
    break;
  case HTML_SECTION_ACTION:
    event->type = SECTION;
    event->e_start = parser->fragment.start;
    event->e_length = parser->fragment.length;
    break;
  case HTML_COMMENT_ACTION:
    event->type = COMMENT;
    event->e_start = parser->fragment.start;
    event->e_length = parser->fragment.length;
    break;
  case HTML_PROCESS_ACTION:
    event->type = PROCESS;
    event->e_start = parser->fragment.start;
    event->e_length = parser->fragment.length;
    break;
  case HTML_TEXT_ACTION:
    event->type = TEXT;
    event->e_start = parser->fragment.start;
    event->e_length = parser->fragment.length;
    break;
  }
}

uint8_t html_parser_get_tag(html_parser_t *parser, html_event_t *event, fragment_t *frag) {
  if (event->type != BEGIN_TAG && event->type != END_TAG && event->type != SINGLETON_TAG) {
    return 0;
  }

  frag->start = event->e_start;
  for (size_t i = event->e_start, len = event->e_start + event->e_length; i < len; i ++) {
    if (parser->buf[i] == ' ') {
      frag->length = i - event->e_start;
      return 1;
    }
  }
  frag->length = event->e_length;
  return 1;
}

uint8_t html_parser_get_next_attribute(html_parser_t *parser, html_event_t *event, html_attribute_t *attribute) {
  int state = ATTR_READY_STATE;
  size_t i = event->attr_pos, end = event->e_start + event->e_length;

  void * data[2];
  data[0] = attribute;

  if (event->type != BEGIN_TAG && event->type != SINGLETON_TAG) {
    return 0;
  }

  if (i == 0) {
    i = event->e_start;
    // skip the tag
    while (i < end && parser->buf[i] != ' ') {
      i ++;
    }
  }

 RETRY:

  data[1] = (void *)&i;

  if (i == end) {
    event->attr_pos = i;
    state = attr_transform_state(state, ATTR_EOI_EVENT, data);
    return 0;
  }

  switch(parser->buf[i]) {
  case ' ':
    state = attr_transform_state(state, ATTR_SPACE_EVENT, data);
    break;
  case '\t':
  case '\n':
  case '\r':
    state = attr_transform_state(state, ATTR_SPACES_EVENT, data);
    break;
  case '=':
    state = attr_transform_state(state, ATTR_EQUALS_EVENT, data);
    break;
  case '\'':
    state = attr_transform_state(state, ATTR_APOSTROPHE_EVENT, data);
    break;
  case '"':
    state = attr_transform_state(state, ATTR_DOUBLE_QUOTES_EVENT, data);
    break;
  default:
    state = attr_transform_state(state, ATTR_ALPHA_EVENT, data);
    break;
  }

  if (i < end && state != ATTR_READY_STATE) {
    i ++;
    goto RETRY;
  } else if (i < end) {
    event->attr_pos = i + 1;
    return 1;
  } else {
    return 0;
  }
}

void attr_do_action(enum ATTR_ACTION action, void * data) {
  html_attribute_t * attribute = (html_attribute_t *) (((void **)data)[0]);
  size_t pos = * (size_t *)(((void **)data)[1]);

  switch (action) {
  case ATTR_START_KEY_ACTION:
    attribute->key.start = pos;
    attribute->key.length = 1;
    attribute->value.start = 0;
    attribute->value.length = 0;
    break;
  case ATTR_APPEND_KEY_ACTION:
    attribute->key.length ++;
    break;
  case ATTR_START_VALUE_ACTION:
    attribute->value.start = pos;
    attribute->value.length = 1;
    break;
  case ATTR_APPEND_VALUE_ACTION:
    attribute->value.length ++;
    break;
  }
}
