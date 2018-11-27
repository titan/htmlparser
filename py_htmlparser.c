#include <Python.h>
#include <structmember.h>
#include "htmlparser.h"

/***** Attribute *****/

typedef struct {
  PyObject_HEAD
  PyObject * key;
  PyObject * value;
} AttributeObject;

static void attribute_dealloc(AttributeObject * self) {
  Py_XDECREF(self->key);
  Py_XDECREF(self->value);
  Py_TYPE(self)->tp_free((PyObject *) self);
}

static PyObject * attribute_new(PyTypeObject * type, PyObject * args, PyObject * kwds) {
  (void) args;
  (void) kwds;
  AttributeObject * self = (AttributeObject *) type->tp_alloc(type, 0);
  if (self != NULL) {
    self->key = PyUnicode_FromString("");
    if (self->key == NULL) {
      Py_DECREF(self);
      return NULL;
    }
    self->value = PyUnicode_FromString("");
    if (self->value == NULL) {
      Py_DECREF(self);
      return NULL;
    }
  }
  return (PyObject *) self;
}

static int attribute_init(AttributeObject * self, PyObject * args, PyObject * kwds) {
  static char * kwlist[] = {"key", "value", NULL};
  PyObject * key = NULL, * value = NULL, * tmp;
  if (!PyArg_ParseTupleAndKeywords(args, kwds, "|ss", kwlist, &key, &value)) {
    return -1;
  }
  if (key) {
    tmp = self->key;
    Py_INCREF(key);
    self->key = key;
    Py_XDECREF(tmp);
  }
  if (value) {
    tmp = self->value;
    Py_INCREF(value);
    self->value = value;
    Py_XDECREF(tmp);
  }
  return 0;
}

static PyObject * attribute_getattro(AttributeObject * self, PyObject * nameobj) {

  if (!PyUnicode_Check(nameobj))
    goto generic;

  if (PyUnicode_CompareWithASCIIString(nameobj, "key") == 0) {
    Py_INCREF(self->key);
    return self->key;
  } else if (PyUnicode_CompareWithASCIIString(nameobj, "value") == 0) {
    Py_INCREF(self->value);
    return self->value;
  }
 generic:
  return PyObject_GenericGetAttr((PyObject *)self, nameobj);
}

static PyTypeObject AttributeType = {
  PyVarObject_HEAD_INIT(NULL, 0)
  .tp_name = "htmlparser.Atrrbiute",
  .tp_doc = "Attribute",
  .tp_basicsize = sizeof(AttributeObject),
  .tp_itemsize = 0,
  .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
  .tp_new = attribute_new,
  .tp_init = (initproc) attribute_init,
  .tp_dealloc = (destructor) attribute_dealloc,
  .tp_getattro = (getattrofunc) attribute_getattro,
};

/***** Attribute *****/

/***** Event *****/

typedef struct {
  PyObject_HEAD
  html_parser_t * p;
  html_event_t e;
} EventObject;

static void event_dealloc(EventObject * self) {
  Py_TYPE(self)->tp_free((PyObject *) self);
}

static PyObject * event_tag(EventObject * self, PyObject * Py_UNUSED(ignored)) {
  PyObject * tag = NULL;
  fragment_t frag;
  if (OPEN_TAG == self->e.type || CLOSE_TAG == self->e.type || SINGLETON_TAG == self->e.type) {
    html_parser_get_tag(self->p, &self->e, &frag);
    tag = PyUnicode_FromStringAndSize(self->p->buf + frag.start, frag.length);
    if (tag) {
      return tag;
    } else {
      Py_RETURN_NONE;
    }
  } else {
    Py_RETURN_NONE;
  }
}

static PyObject * event_next_attribute(EventObject * self, PyObject * Py_UNUSED(ignored)) {
  AttributeObject * a = NULL;
  html_attribute_t attribute;

  if (html_parser_get_next_attribute(self->p, &self->e, &attribute) != 0) {
    a = (AttributeObject *) PyObject_New(AttributeObject, &AttributeType);
    if (a == NULL) {
      Py_RETURN_NONE;
    }

    a->key = PyUnicode_FromStringAndSize(self->p->buf + attribute.k_start, attribute.k_length);
    if (a->key == NULL) {
      Py_DECREF(a);
      Py_RETURN_NONE;
    }

    a->value = PyUnicode_FromStringAndSize(self->p->buf + attribute.v_start, attribute.v_length);
    if (a->value == NULL) {
      Py_DECREF(a->key);
      Py_DECREF(a);
      Py_RETURN_NONE;
    }

    return (PyObject *) a;
  } else {
    Py_RETURN_NONE;
  }
}

static PyMemberDef event_members[] = {
    {"content", T_OBJECT, -1, READONLY, "The content of event"},
    {"type", T_INT, -1, READONLY, "The type of event"},
    {NULL}  /* Sentinel */
};

static PyMethodDef event_methods[] = {
  {"tag", (PyCFunction) event_tag, METH_NOARGS, "Get tag name if it is START_TAG or END_TAG."},
  {"next_attribute", (PyCFunction) event_next_attribute, METH_NOARGS, "Return next attribute in the start tag."},
  {NULL, NULL, 0, NULL}  /* Sentinel */
};

static PyObject * event_getattro(EventObject * self, PyObject * nameobj) {

  if (!PyUnicode_Check(nameobj))
    goto generic;

  PyObject * tmp = NULL;

  if (PyUnicode_CompareWithASCIIString(nameobj, "content") == 0) {
    tmp = PyUnicode_FromStringAndSize(self->p->buf + self->e.e_start, self->e.e_length);
    if (tmp) {
      return tmp;
    } else {
      Py_INCREF(Py_None);
      return Py_None;
    }
  } else if (PyUnicode_CompareWithASCIIString(nameobj, "type") == 0) {
    return PyLong_FromLong((long) self->e.type);
  }
 generic:
  return PyObject_GenericGetAttr((PyObject *)self, nameobj);
}

static PyTypeObject EventType = {
  PyVarObject_HEAD_INIT(NULL, 0)
  .tp_name = "htmlparser.Event",
  .tp_basicsize = sizeof(EventObject),
  .tp_itemsize = 0,
  .tp_dealloc = (destructor) event_dealloc,
  .tp_getattro = (getattrofunc) event_getattro,
  .tp_members = event_members,
  .tp_methods = event_methods,
};

/***** Event *****/


/***** Parser *****/

typedef struct {
  PyObject_HEAD
  html_parser_t p;
} ParserObject;

static void parser_dealloc(ParserObject * self) {
  free(self->p.buf);
  Py_TYPE(self)->tp_free((PyObject *) self);
}

static PyObject * parser_next(ParserObject * self, PyObject *Py_UNUSED(ignored)) {
  html_event_t evt;
  if (html_parser_get_next(&self->p, &evt) == 0) {
    Py_RETURN_NONE;
  } else {
    EventObject * e = (EventObject *) PyObject_New(EventObject, &EventType);

    if (e == NULL) {
      Py_RETURN_NONE;
    }
    e->p = &self->p;
    e->e = evt;
    return (PyObject *)e;
  }
}

static PyObject * parser_reset(ParserObject * self, PyObject * args) {
  size_t pos = 0;
  if (!PyArg_ParseTuple(args, "I", &pos))
    return NULL;
  if (pos < self->p.length) {
    self->p.position = pos;
    Py_RETURN_NONE;
  } else {
    PyErr_SetString(PyExc_TypeError, "argument exceeds the length range");
    return NULL;
  }
}

static PyMethodDef parser_methods[] = {
  {"next", (PyCFunction) parser_next, METH_NOARGS, "Next event."},
  {"reset",  (PyCFunction) parser_reset, METH_VARARGS, "Reset position."},
  {NULL, NULL, 0, NULL}  /* Sentinel */
};

static PyTypeObject ParserType = {
  PyVarObject_HEAD_INIT(NULL, 0)
  .tp_name = "htmlparser.Parser",
  .tp_basicsize = sizeof(ParserObject),
  .tp_itemsize = 0,
  .tp_flags = Py_TPFLAGS_DEFAULT,
  .tp_dealloc = (destructor) parser_dealloc,
  .tp_methods = parser_methods,
  .tp_new = PyType_GenericNew,
};

/***** Parser *****/
static PyObject * parser(PyObject * self, PyObject * args) {
  (void) self;
  ParserObject * p = NULL;
  char * buf = NULL;
  int len = 0;
  if (PyArg_ParseTuple(args, "|s#", &buf, &len)) {
    if (buf != NULL) {
      p = PyObject_New(ParserObject, &ParserType);
      if (p) {
        char * tmp = (char *) malloc(len * sizeof(char));
        if (tmp) {
          memcpy(tmp, buf, len);
          html_parser_new(&p->p, tmp, len);
        }
      }
    }
  }
  /* If bad args, return NULL */
  return (PyObject *) p;
}

static PyMethodDef module_methods[] = {
  {"parser", parser, METH_VARARGS, "Make a parser with content."},
  {NULL, NULL, 0, NULL}  /* Sentinel */
};

static struct PyModuleDef module = {
  PyModuleDef_HEAD_INIT,
  "htmlparser", /* name of module */
  "A pull style html parser.", /* module documentation, may be NULL */
  -1, /* size of per-interpreter state of the module, or -1 if the module keeps state in global variables. */
  module_methods,
  NULL,
  NULL,
  NULL,
  NULL,
};

PyMODINIT_FUNC PyInit_htmlparser(void) {
  PyObject* m;

  if (PyType_Ready(&ParserType) < 0)
    return NULL;

  if (PyType_Ready(&EventType) < 0)
    return NULL;

  if (PyType_Ready(&AttributeType) < 0)
    return NULL;

  m = PyModule_Create(&module);
  if (m == NULL)
    return NULL;

  PyModule_AddObject(m, "OPEN_TAG", PyLong_FromLong(OPEN_TAG));
  PyModule_AddObject(m, "CLOSE_TAG", PyLong_FromLong(CLOSE_TAG));
  PyModule_AddObject(m, "SINGLETON_TAG", PyLong_FromLong(SINGLETON_TAG));
  PyModule_AddObject(m, "TEXT", PyLong_FromLong(TEXT));
  PyModule_AddObject(m, "COMMENT", PyLong_FromLong(COMMENT));
  PyModule_AddObject(m, "SECTION", PyLong_FromLong(SECTION));
  PyModule_AddObject(m, "PROCESS", PyLong_FromLong(PROCESS));
  PyModule_AddObject(m, "DECLARATION", PyLong_FromLong(DECLARATION));
  return m;
}
