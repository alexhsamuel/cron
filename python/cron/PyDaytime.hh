#pragma once

#include <cmath>
#include <experimental/optional>
#include <iostream>

#include "cron/format.hh"
#include "cron/daytime.hh"
#include "cron/time_zone.hh"
#include "py.hh"

namespace alxs {

using namespace py;

using std::experimental::optional;
using std::make_unique;
using std::string;
using std::unique_ptr;

//------------------------------------------------------------------------------
// Declarations
//------------------------------------------------------------------------------

StructSequenceType* get_daytime_parts_type();

// template<typename DAYTIME> optional<DAYTIME> convert_object(Object*);
// template<typename DAYTIME> optional<DAYTIME> convert_daytime_object(Object*);

//------------------------------------------------------------------------------
// Type class
//------------------------------------------------------------------------------

template<typename DAYTIME>
class PyDaytime
  : public ExtensionType
{
public:

  using Daytime = DAYTIME;

  /** 
   * Readies the Python type and adds it to `module` as `name`.  
   *
   * Should only be called once; this is not checked.
   */
  static void add_to(Module& module, string const& name);

  static Type type_;

  /**
   * Creates an instance of the Python type.
   */
  static ref<PyDaytime> create(Daytime daytime, PyTypeObject* type=&type_);

  /**
   * Returns true if 'object' is an instance of this type.
   */
  static bool Check(PyObject* object);

  PyDaytime(Daytime daytime) : daytime_(daytime) {}

  /**
   * The wrapped date instance.
   *
   * This is the only non-static data member.
   */
  Daytime const daytime_;

  // Number methods.
  static PyNumberMethods tp_as_number_;

  // Methods.
  static Methods<PyDaytime> tp_methods_;

  // Getsets.
  static GetSets<PyDaytime> tp_getsets_;

  /** Date format used to generate the repr.  */
  static unique_ptr<cron::DaytimeFormat> repr_format_;
  /** Date format used to generate the str.  */
  static unique_ptr<cron::DaytimeFormat> str_format_;

private:

  static void tp_init(PyDaytime* self, Tuple* args, Dict* kw_args);
  static void tp_dealloc(PyDaytime* self);
  static ref<Unicode> tp_repr(PyDaytime* self);
  static ref<Unicode> tp_str(PyDaytime* self);

  static Type build_type(string const& type_name);

};


template<typename DAYTIME>
void
PyDaytime<DAYTIME>::add_to(
  Module& module,
  string const& name)
{
  // Construct the type struct.
  type_ = build_type(string{module.GetName()} + "." + name);
  // Hand it to Python.
  type_.Ready();

  // Build the repr format.
  repr_format_ = make_unique<cron::DaytimeFormat>(
    name + "(%H, %M, %S)",  // FIXME: Not a ctor.
    name + ".INVALID",
    name + ".MISSING");

  // Build the str format.  Choose precision for seconds that captures actual
  // precision of the daytime class.
  std::string pattern = "%H:%M:%";
  size_t const precision = (size_t) ceil(log10(Daytime::DENOMINATOR));
  if (precision > 0) {
    pattern += ".";
    pattern += std::to_string(precision);
  }
  pattern += "SZ";
  str_format_ = make_unique<cron::DaytimeFormat>(pattern);

  // Add the type to the module.
  module.add(&type_);
}


template<typename DAYTIME>
ref<PyDaytime<DAYTIME>>
PyDaytime<DAYTIME>::create(
  Daytime const daytime,
  PyTypeObject* const type)
{
  auto obj = ref<PyDaytime>::take(check_not_null(PyDaytime::type_.tp_alloc(type, 0)));

  // daytime_ is const to indicate immutablity, but Python initialization is later
  // than C++ initialization, so we have to cast off const here.
  new(const_cast<Daytime*>(&obj->daytime_)) Daytime{daytime};
  return obj;
}


template<typename DAYTIME>
Type
PyDaytime<DAYTIME>::type_;


template<typename DAYTIME>
bool
PyDaytime<DAYTIME>::Check(
  PyObject* const other)
{
  return static_cast<Object*>(other)->IsInstance((PyObject*) &type_);
}


//------------------------------------------------------------------------------
// Standard type methods
//------------------------------------------------------------------------------

template<typename DAYTIME>
void
PyDaytime<DAYTIME>::tp_init(
  PyDaytime* const self,
  Tuple* const args,
  Dict* const kw_args)
{
  // FIXME
  typename Daytime::Offset offset;
  Arg::ParseTuple(args, "|k", &offset);

  new(self) PyDaytime(Daytime::from_offset(offset));
}


template<typename DAYTIME>
void
PyDaytime<DAYTIME>::tp_dealloc(
  PyDaytime* const self)
{
  self->daytime_.~DaytimeTemplate();
  self->ob_type->tp_free(self);
}


template<typename DAYTIME>
ref<Unicode>
PyDaytime<DAYTIME>::tp_repr(
  PyDaytime* const self)
{
  return Unicode::from((*repr_format_)(self->daytime_));
}


template<typename DAYTIME>
ref<Unicode>
PyDaytime<DAYTIME>::tp_str(
  PyDaytime* const self)
{
  // FIXME: Not UTC?
  return Unicode::from((*str_format_)(self->daytime_));  
}


//------------------------------------------------------------------------------
// Number methods
//------------------------------------------------------------------------------

template<typename DAYTIME>
PyNumberMethods
PyDaytime<DAYTIME>::tp_as_number_ = {
  (binaryfunc)  nullptr,                        // nb_add
  (binaryfunc)  nullptr,                        // nb_subtract
  (binaryfunc)  nullptr,                        // nb_multiply
  (binaryfunc)  nullptr,                        // nb_remainder
  (binaryfunc)  nullptr,                        // nb_divmod
  (ternaryfunc) nullptr,                        // nb_power
  (unaryfunc)   nullptr,                        // nb_negative
  (unaryfunc)   nullptr,                        // nb_positive
  (unaryfunc)   nullptr,                        // nb_absolute
  (inquiry)     nullptr,                        // nb_bool
  (unaryfunc)   nullptr,                        // nb_invert
  (binaryfunc)  nullptr,                        // nb_lshift
  (binaryfunc)  nullptr,                        // nb_rshift
  (binaryfunc)  nullptr,                        // nb_and
  (binaryfunc)  nullptr,                        // nb_xor
  (binaryfunc)  nullptr,                        // nb_or
  (unaryfunc)   nullptr,                        // nb_int
  (void*)       nullptr,                        // nb_reserved
  (unaryfunc)   nullptr,                        // nb_float
  (binaryfunc)  nullptr,                        // nb_inplace_add
  (binaryfunc)  nullptr,                        // nb_inplace_subtract
  (binaryfunc)  nullptr,                        // nb_inplace_multiply
  (binaryfunc)  nullptr,                        // nb_inplace_remainder
  (ternaryfunc) nullptr,                        // nb_inplace_power
  (binaryfunc)  nullptr,                        // nb_inplace_lshift
  (binaryfunc)  nullptr,                        // nb_inplace_rshift
  (binaryfunc)  nullptr,                        // nb_inplace_and
  (binaryfunc)  nullptr,                        // nb_inplace_xor
  (binaryfunc)  nullptr,                        // nb_inplace_or
  (binaryfunc)  nullptr,                        // nb_floor_divide
  (binaryfunc)  nullptr,                        // nb_true_divide
  (binaryfunc)  nullptr,                        // nb_inplace_floor_divide
  (binaryfunc)  nullptr,                        // nb_inplace_true_divide
  (unaryfunc)   nullptr,                        // nb_index
/* FIXME: Python 2.5
  (binaryfunc)  nullptr,                        // nb_matrix_multiply
  (binaryfunc)  nullptr,                        // nb_inplace_matrix_multiply
*/
};


//------------------------------------------------------------------------------
// Methods
//------------------------------------------------------------------------------

template<typename DAYTIME>
Methods<PyDaytime<DAYTIME>>
PyDaytime<DAYTIME>::tp_methods_
  = Methods<PyDaytime>()
  ;


//------------------------------------------------------------------------------
// Getsets
//------------------------------------------------------------------------------

template<typename DAYTIME>
GetSets<PyDaytime<DAYTIME>>
PyDaytime<DAYTIME>::tp_getsets_ 
  = GetSets<PyDaytime>()
  ;


//------------------------------------------------------------------------------
// Other members
//------------------------------------------------------------------------------

template<typename DAYTIME>
unique_ptr<cron::DaytimeFormat>
PyDaytime<DAYTIME>::repr_format_;


template<typename DAYTIME>
unique_ptr<cron::DaytimeFormat>
PyDaytime<DAYTIME>::str_format_;


//------------------------------------------------------------------------------
// Type object
//------------------------------------------------------------------------------

template<typename DAYTIME>
Type
PyDaytime<DAYTIME>::build_type(
  string const& type_name)
{
  return PyTypeObject{
    PyVarObject_HEAD_INIT(nullptr, 0)
    (char const*)         strdup(type_name.c_str()),      // tp_name
    (Py_ssize_t)          sizeof(PyDaytime),              // tp_basicsize
    (Py_ssize_t)          0,                              // tp_itemsize
    (destructor)          wrap<PyDaytime, tp_dealloc>,    // tp_dealloc
    // FIXME: Hack!  We'd like to provide a way for any PyDaytime instance to
    // return its datenum, for efficient manipulation by other PyDaytime instances,
    // without virtual methods.  PyTypeObject doesn't provide any slot for us to
    // stash this, so we requisition the deprecated tp_print slot.  This may
    // break in future Python versions, if that slot is reused.
    (printfunc)           nullptr,                        // tp_print
    (getattrfunc)         nullptr,                        // tp_getattr
    (setattrfunc)         nullptr,                        // tp_setattr
    (void*)               nullptr,                        // tp_reserved
    (reprfunc)            wrap<PyDaytime, tp_repr>,       // tp_repr
    (PyNumberMethods*)    &tp_as_number_,                 // tp_as_number
    (PySequenceMethods*)  nullptr,                        // tp_as_sequence
    (PyMappingMethods*)   nullptr,                        // tp_as_mapping
    (hashfunc)            nullptr,                        // tp_hash
    (ternaryfunc)         nullptr,                        // tp_call
    (reprfunc)            wrap<PyDaytime, tp_str>,        // tp_str
    (getattrofunc)        nullptr,                        // tp_getattro
    (setattrofunc)        nullptr,                        // tp_setattro
    (PyBufferProcs*)      nullptr,                        // tp_as_buffer
    (unsigned long)       Py_TPFLAGS_DEFAULT
                          | Py_TPFLAGS_BASETYPE,          // tp_flags
    (char const*)         nullptr,                        // tp_doc
    (traverseproc)        nullptr,                        // tp_traverse
    (inquiry)             nullptr,                        // tp_clear
    (richcmpfunc)         nullptr,                        // tp_richcompare
    (Py_ssize_t)          0,                              // tp_weaklistoffset
    (getiterfunc)         nullptr,                        // tp_iter
    (iternextfunc)        nullptr,                        // tp_iternext
    (PyMethodDef*)        tp_methods_,                    // tp_methods
    (PyMemberDef*)        nullptr,                        // tp_members
    (PyGetSetDef*)        tp_getsets_,                    // tp_getset
    (_typeobject*)        nullptr,                        // tp_base
    (PyObject*)           nullptr,                        // tp_dict
    (descrgetfunc)        nullptr,                        // tp_descr_get
    (descrsetfunc)        nullptr,                        // tp_descr_set
    (Py_ssize_t)          0,                              // tp_dictoffset
    (initproc)            wrap<PyDaytime, tp_init>,       // tp_init
    (allocfunc)           nullptr,                        // tp_alloc
    (newfunc)             PyType_GenericNew,              // tp_new
    (freefunc)            nullptr,                        // tp_free
    (inquiry)             nullptr,                        // tp_is_gc
    (PyObject*)           nullptr,                        // tp_bases
    (PyObject*)           nullptr,                        // tp_mro
    (PyObject*)           nullptr,                        // tp_cache
    (PyObject*)           nullptr,                        // tp_subclasses
    (PyObject*)           nullptr,                        // tp_weaklist
    (destructor)          nullptr,                        // tp_del
    (unsigned int)        0,                              // tp_version_tag
    (destructor)          nullptr,                        // tp_finalize
  };
}


//------------------------------------------------------------------------------

}  // namespace alxs
