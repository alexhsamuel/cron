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

  // FIXME: How the fuck do we construct these?
  std::vector<Weekday> weekdays;
  weekdays.push_back(MONDAY);
  weekdays.push_back(TUESDAY);
  weekdays.push_back(WEDNESDAY);
  weekdays.push_back(THURSDAY);
  weekdays.push_back(FRIDAY);
  Cal_ptr cal = std::make_shared<WeekdaysCalendar>(weekdays);
  new(self) PyCalendar(std::move(cal));
}


//------------------------------------------------------------------------------
// Methods
//------------------------------------------------------------------------------

Methods<PyCalendar>
tp_methods_
  = Methods<PyCalendar>()
  ;


//------------------------------------------------------------------------------
// Getsets
//------------------------------------------------------------------------------

GetSets<PyCalendar>
tp_getsets_ 
  = GetSets<PyCalendar>()
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
    (PyNumberMethods*)    nullptr,                        // tp_as_number
    (PySequenceMethods*)  nullptr,                        // tp_as_sequence
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

