#include <string>

#include "py.hh"
#include "py_calendar.hh"

namespace ora {
namespace py {

//------------------------------------------------------------------------------

Type
PyCalendar::type_;

void
PyCalendar::add_to(
  Module& module)
{
  // Construct the type struct.
  type_ = build_type();
  // Hand it to Python.
  type_.Ready();

  // Add the type to the module.
  module.add(&type_);
}


namespace {

using Cal_ptr = PyCalendar::Cal_ptr;

void
tp_dealloc(
  PyCalendar* const self)
{
  self->~PyCalendar();
  self->ob_type->tp_free(self);
}


ref<Unicode>
tp_repr(
  PyCalendar* const self)
{
  std::string full_name{self->ob_type->tp_name};
  std::string type_name = full_name.substr(full_name.rfind('.') + 1);
  // FIXME
  auto const repr = type_name + "(...)";
  return Unicode::from(repr);
}


void
tp_init(
  PyCalendar* const self,
  Tuple* const args,
  Dict* const kw_args)
{
  Arg::ParseTuple(args, "");

  // FIXME
  throw NotImplementedError("Calendar.__init__");
  // new(self) PyCalendar(std::move(cal));
}


//------------------------------------------------------------------------------
// Number methods
//------------------------------------------------------------------------------

ref<Object>
nb_invert(
  PyCalendar* self)
{
  return PyCalendar::create(std::make_unique<Calendar>(!*self->cal_));
}


PyNumberMethods
tp_as_number = {
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
  (unaryfunc)   wrap<PyCalendar, nb_invert>,    // nb_invert
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
  (binaryfunc)  nullptr,                        // nb_matrix_multiply
  (binaryfunc)  nullptr,                        // nb_inplace_matrix_multiply
};


//------------------------------------------------------------------------------
// Sequence methods
//------------------------------------------------------------------------------

bool
sq_contains(
  PyCalendar* const self,
  Object* const obj)
{
  auto date = convert_to_date(obj);
  return self->cal_->contains(date);
}


PySequenceMethods const 
tp_as_sequence = {
  (lenfunc)         nullptr,                            // sq_length
  (binaryfunc)      nullptr,                            // sq_concat
  (ssizeargfunc)    nullptr,                            // sq_repeat
  (ssizeargfunc)    nullptr,                            // sq_item
  (void*)           nullptr,                            // was_sq_slice
  (ssizeobjargproc) nullptr,                            // sq_ass_item
  (void*)           nullptr,                            // was_sq_ass_slice
  (objobjproc)      wrap<PyCalendar, sq_contains>,      // sq_contains
  (binaryfunc)      nullptr,                            // sq_inplace_concat
  (ssizeargfunc)    nullptr,                            // sq_inplace_repeat
};


//------------------------------------------------------------------------------
// Methods
//------------------------------------------------------------------------------

ref<Object>
method_after(
  PyCalendar* const self,
  Tuple* const args,
  Dict* const kw_args)
{
  static char const* const arg_names[] = {"date", nullptr};
  Object* date_arg;
  Arg::ParseTupleAndKeywords(args, kw_args, "O", arg_names, &date_arg);

  auto const date = convert_to_date(date_arg);
  auto const result = self->cal_->after(date);
  auto api = PyDateAPI::get(date_arg);
  if (api == nullptr)
    api = PyDate<Date>::api_;
  return api->from_datenum(result.get_datenum());
}


ref<Object>
method_before(
  PyCalendar* const self,
  Tuple* const args,
  Dict* const kw_args)
{
  static char const* const arg_names[] = {"date", nullptr};
  Object* date_arg;
  Arg::ParseTupleAndKeywords(args, kw_args, "O", arg_names, &date_arg);

  auto const date = convert_to_date(date_arg);
  auto const result = self->cal_->before(date);
  auto api = PyDateAPI::get(date_arg);
  if (api == nullptr)
    api = PyDate<Date>::api_;
  return api->from_datenum(result.get_datenum());
}


ref<Object>
method_contains(
  PyCalendar* const self,
  Tuple* const args,
  Dict* const kw_args)
{
  static char const* const arg_names[] = {"date", nullptr};
  Object* date_arg;
  Arg::ParseTupleAndKeywords(args, kw_args, "O", arg_names, &date_arg);

  auto const date = convert_to_date(date_arg);
  return Bool::from(self->cal_->contains(date));
}


Methods<PyCalendar>
tp_methods_
  = Methods<PyCalendar>()
    .template add<method_after>                     ("after")
    .template add<method_before>                    ("before")
    .template add<method_contains>                  ("contains")
  ;


//------------------------------------------------------------------------------
// Getsets
//------------------------------------------------------------------------------

ref<Object>
get_range(
  PyCalendar* const self,
  void* /* closure */)
{
  auto const range = self->cal_->range();
  auto start = PyDate<Date>::create(range.min);
  // FIXME: slice?  Really?
  return ref<Object>::take(PySlice_New(
    PyDate<Date>::create(range.min), 
    PyDate<Date>::create(range.max), 
    nullptr
  ));
}


GetSets<PyCalendar>
tp_getsets_ 
  = GetSets<PyCalendar>()
     .template add_get<get_range>               ("range")
 ;


}  // anonymous namespace

//------------------------------------------------------------------------------

Type
PyCalendar::build_type()
{
  return PyTypeObject{
    PyVarObject_HEAD_INIT(nullptr, 0)
    (char const*)         "Calendar",                     // tp_name
    (Py_ssize_t)          sizeof(PyCalendar),             // tp_basicsize
    (Py_ssize_t)          0,                              // tp_itemsize
    (destructor)          wrap<PyCalendar, tp_dealloc>,   // tp_dealloc
    (printfunc)           nullptr,                        // tp_print
    (getattrfunc)         nullptr,                        // tp_getattr
    (setattrfunc)         nullptr,                        // tp_setattr
                          nullptr,                        // tp_reserved
    (reprfunc)            wrap<PyCalendar, tp_repr>,      // tp_repr
    (PyNumberMethods*)    &tp_as_number,                  // tp_as_number
    (PySequenceMethods*)  &tp_as_sequence,                // tp_as_sequence
    (PyMappingMethods*)   nullptr,                        // tp_as_mapping
    (hashfunc)            nullptr,                        // tp_hash
    (ternaryfunc)         nullptr,                        // tp_call
    (reprfunc)            nullptr,                        // tp_str
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
    (initproc)            wrap<PyCalendar, tp_init>,      // tp_init
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

}  // namespace py
}  // namespace ora

