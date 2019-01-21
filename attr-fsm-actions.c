#include "attr-context.h"
#include "attr-fsm.h"
#include "attr-fsm-actions.h"

void attr_start_key(struct attr_context * ctx, enum ATTR_STATE state, enum ATTR_EVENT event) {
  (void) state;
  (void) event;
  ctx->attribute->key.start = ctx->pos;
  ctx->attribute->key.length = 1;
}

void attr_append_key(struct attr_context * ctx, enum ATTR_STATE state, enum ATTR_EVENT event) {
  (void) state;
  (void) event;
  ctx->attribute->key.length ++;
}

void attr_reset_value(struct attr_context * ctx, enum ATTR_STATE state, enum ATTR_EVENT event) {
  (void) state;
  (void) event;
  ctx->attribute->value.start = 0;
  ctx->attribute->value.length = 0;
}

void attr_start_value(struct attr_context * ctx, enum ATTR_STATE state, enum ATTR_EVENT event) {
  (void) state;
  (void) event;
  ctx->attribute->value.start = ctx->pos;
  ctx->attribute->value.length = 1;
}

void attr_append_value(struct attr_context * ctx, enum ATTR_STATE state, enum ATTR_EVENT event) {
  (void) state;
  (void) event;
  ctx->attribute->value.length ++;
}
