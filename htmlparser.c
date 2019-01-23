#include <ctype.h>
#include "htmlparser.h"
#include "html-context.h"
#include "html-fsm.h"
#include "attr-context.h"
#include "attr-fsm.h"

uint8_t html_parser_new(struct html_parser * parser, char * buf, size_t length) {
  parser->buf = buf;
  parser->position = 0;
  parser->length = length;
  return 1;
}

uint8_t html_parser_free(struct html_parser * parser) {
  (void) parser;
  return 1;
}

uint8_t html_parser_get_next(struct html_parser * parser, struct html_event * event) {
  char * buf = parser->buf;
  uint32_t last_state = HTML_READY_STATE;
  parser->fragment.start = 0;
  parser->fragment.length = 0;

  struct html_state_machine fsm;
  struct html_context ctx;
  ctx.parser = parser;
  ctx.event = event;

  html_state_machine_init(&fsm, &ctx);

 RETRY:
  if (parser->position >= parser->length) {
    bzero(event, sizeof(struct html_event));
    html_state_machine_process(&fsm, HTML_EOF_EVENT);
    if (event->e_length != 0) {
      return 1;
    }
    return 0;
  }
  last_state = fsm.state;

  switch (buf[parser->position]) {
  case '<':
    html_state_machine_process(&fsm, HTML_LESS_THAN_EVENT);
    break;
  case '>':
    html_state_machine_process(&fsm, HTML_GREATER_THAN_EVENT);
    break;
  case '[':
    html_state_machine_process(&fsm, HTML_OPEN_BRACKET_EVENT);
    break;
  case ']':
    html_state_machine_process(&fsm, HTML_CLOSE_BRACKET_EVENT);
    break;
  case '!':
    html_state_machine_process(&fsm, HTML_EXCLAM_EVENT);
    break;
  case '-':
    html_state_machine_process(&fsm, HTML_MINUS_EVENT);
    break;
  case '?':
    html_state_machine_process(&fsm, HTML_QUESTION_EVENT);
    break;
  case '/':
    html_state_machine_process(&fsm, HTML_SLASH_EVENT);
    break;
  case ' ':
  case '\t':
  case '\r':
  case '\n':
    html_state_machine_process(&fsm, HTML_SPACES_EVENT);
    break;
  default:
    html_state_machine_process(&fsm, HTML_OTHERS_EVENT);
    break;
  }

  parser->position ++;
  if (fsm.state == HTML_READY_STATE) {
    return 1;
  } else if (fsm.state == HTML_LESS_THAN_STATE && last_state == HTML_TEXT_STATE) {
    parser->position --;
    return 1;
  } else {
    goto RETRY;
  }
}

uint8_t html_parser_get_tag(struct html_parser * parser, struct html_event * event, struct html_fragment * frag) {
  if (event->type != OPEN_TAG && event->type != CLOSE_TAG && event->type != SINGLETON_TAG) {
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

uint8_t html_parser_get_next_attribute(struct html_parser * parser, struct html_event * event, struct html_attribute * attribute) {
  size_t i = event->attr_pos, end = event->e_start + event->e_length;

  if (event->type != OPEN_TAG && event->type != SINGLETON_TAG) {
    return 0;
  }

  if (i == 0) {
    i = event->e_start;
    // skip the tag
    while (i < end && parser->buf[i] != ' ') {
      i ++;
    }
  }

  struct attr_state_machine fsm;
  struct attr_context ctx;

  ctx.attribute = attribute;
  attr_state_machine_init(&fsm, &ctx);

 RETRY:

  ctx.pos = i;

  if (i == end) {
    event->attr_pos = i;
    attr_state_machine_process(&fsm, ATTR_EOI_EVENT);
    return 0;
  }

  switch(parser->buf[i]) {
  case ' ':
  case '\t':
  case '\n':
  case '\r':
    attr_state_machine_process(&fsm, ATTR_SPACES_EVENT);
    break;
  case '=':
    attr_state_machine_process(&fsm, ATTR_EQUALS_EVENT);
    break;
  case '\'':
    attr_state_machine_process(&fsm, ATTR_APOSTROPHE_EVENT);
    break;
  case '"':
    attr_state_machine_process(&fsm, ATTR_DOUBLE_QUOTES_EVENT);
    break;
  default:
    attr_state_machine_process(&fsm, ATTR_ALPHA_EVENT);
    break;
  }

  if (i < end && fsm.state != ATTR_READY_STATE) {
    i ++;
    goto RETRY;
  } else if (i < end) {
    event->attr_pos = i + 1;
    return 1;
  } else {
    return 0;
  }
}
