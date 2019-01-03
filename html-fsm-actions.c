#include "htmlparser.h"
#include "html-context.h"
#include "html-fsm.h"
#include "html-fsm-actions.h"

void html_start_fragment(struct html_context * ctx, enum HTML_STATE state, enum HTML_EVENT event) {
  (void) state;
  (void) event;
  struct html_parser * parser = ctx->parser;
  parser->fragment.start = parser->position;
  parser->fragment.length = 1;
}

void html_do_open_tag(struct html_context * ctx, enum HTML_STATE state, enum HTML_EVENT event) {
  (void) state;
  (void) event;
  struct html_parser * parser = ctx->parser;
  struct html_event * evt = ctx->event;
  evt->type = OPEN_TAG;
  evt->e_start = parser->fragment.start;
  evt->e_length = parser->fragment.length;
  evt->attr_pos = 0;
}

void html_append(struct html_context * ctx, enum HTML_STATE state, enum HTML_EVENT event) {
  (void) state;
  (void) event;
  struct html_parser * parser = ctx->parser;
  parser->fragment.length ++;
}

void html_append_slash_newline_append(struct html_context * ctx, enum HTML_STATE state, enum HTML_EVENT event) {
  (void) state;
  (void) event;
  struct html_parser * parser = ctx->parser;
  parser->fragment.length += 2;
}

void html_do_singleton_tag(struct html_context * ctx, enum HTML_STATE state, enum HTML_EVENT event) {
  (void) state;
  (void) event;
  struct html_parser * parser = ctx->parser;
  struct html_event * evt = ctx->event;
  evt->type = SINGLETON_TAG;
  evt->e_start = parser->fragment.start;
  evt->e_length = parser->fragment.length;
  evt->attr_pos = 0;
}

void html_do_close_tag(struct html_context * ctx, enum HTML_STATE state, enum HTML_EVENT event) {
  (void) state;
  (void) event;
  struct html_parser * parser = ctx->parser;
  struct html_event * evt = ctx->event;
  evt->type = CLOSE_TAG;
  evt->e_start = parser->fragment.start;
  evt->e_length = parser->fragment.length;
}

void html_do_declaration(struct html_context * ctx, enum HTML_STATE state, enum HTML_EVENT event) {
  (void) state;
  (void) event;
  struct html_parser * parser = ctx->parser;
  struct html_event * evt = ctx->event;
  evt->type = DECLARATION;
  evt->e_start = parser->fragment.start;
  evt->e_length = parser->fragment.length;
}

void html_do_section(struct html_context * ctx, enum HTML_STATE state, enum HTML_EVENT event) {
  (void) state;
  (void) event;
  struct html_parser * parser = ctx->parser;
  struct html_event * evt = ctx->event;
  evt->type = SECTION;
  evt->e_start = parser->fragment.start;
  evt->e_length = parser->fragment.length;
}

void html_do_comment(struct html_context * ctx, enum HTML_STATE state, enum HTML_EVENT event) {
  (void) state;
  (void) event;
  struct html_parser * parser = ctx->parser;
  struct html_event * evt = ctx->event;
  evt->type = COMMENT;
  evt->e_start = parser->fragment.start;
  evt->e_length = parser->fragment.length;
}

void html_do_process(struct html_context * ctx, enum HTML_STATE state, enum HTML_EVENT event) {
  (void) state;
  (void) event;
  struct html_parser * parser = ctx->parser;
  struct html_event * evt = ctx->event;
  evt->type = PROCESS;
  evt->e_start = parser->fragment.start;
  evt->e_length = parser->fragment.length;
}

void html_do_text(struct html_context * ctx, enum HTML_STATE state, enum HTML_EVENT event) {
  (void) state;
  (void) event;
  struct html_parser * parser = ctx->parser;
  struct html_event * evt = ctx->event;
  evt->type = TEXT;
  evt->e_start = parser->fragment.start;
  evt->e_length = parser->fragment.length;
}
