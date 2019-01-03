#ifndef __ATTR_CONTEXT_H
#define __ATTR_CONTEXT_H
#include <ctype.h>
#include "htmlparser.h"
struct attr_context {
  struct html_attribute * attribute;
  size_t pos;
};
#endif
