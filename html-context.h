#ifndef __HTML_CONTEXT_H
#define __HTML_CONTEXT_H
#include <ctype.h>
#include "htmlparser.h"
struct html_context {
  struct html_parser * parser;
  struct html_event * event;
};
#endif
