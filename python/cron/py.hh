#pragma once

#include <cassert>
#include <initializer_list>
#include <iostream>
#include <string>
#include <vector>

#include <Python.h>

//------------------------------------------------------------------------------

namespace py {

class Long;
class Object;
class Tuple;
class Type;
class Unicode;

//------------------------------------------------------------------------------

class Exception
{
public:

  Exception() {}
  
  Exception(PyObject* exception, char const* message)
  {
    PyErr_SetString(exception, message);
  }

  template<typename A>
  Exception(PyObject* exception, A&& message)
  {
    PyErr_SetString(exception, std::string(std::forward<A>(message)).c_str());
  }

  /**
   * Clears up the Python exception state.  
   */
  static void Clear() { PyErr_Clear(); }

};


/**
 * Template wrapper for a specific Python exception type.
 */
template<PyObject** EXC>
class ExceptionWrapper
  : public Exception
{
public:

  template<typename A>
  ExceptionWrapper(A&& message)
    : Exception(*EXC, std::forward<A>(message))
  {}

};


using ArithmeticError       = ExceptionWrapper<&PyExc_ArithmeticError>;
using AttributeError        = ExceptionWrapper<&PyExc_AttributeError>;
using EnvironmentError      = ExceptionWrapper<&PyExc_EnvironmentError>;
using FileExistsError       = ExceptionWrapper<&PyExc_FileExistsError>;
using FileNotFoundError     = ExceptionWrapper<&PyExc_FileNotFoundError>;
using IOError               = ExceptionWrapper<&PyExc_IOError>;
using IndexError            = ExceptionWrapper<&PyExc_IndexError>;
using InterruptedError      = ExceptionWrapper<&PyExc_InterruptedError>;
using IsADirectoryError     = ExceptionWrapper<&PyExc_IsADirectoryError>;
using KeyError              = ExceptionWrapper<&PyExc_KeyError>;
using LookupError           = ExceptionWrapper<&PyExc_LookupError>;
using NameError             = ExceptionWrapper<&PyExc_NameError>;
using NotADirectoryError    = ExceptionWrapper<&PyExc_NotADirectoryError>;
using NotImplementedError   = ExceptionWrapper<&PyExc_NotImplementedError>;
using OverflowError         = ExceptionWrapper<&PyExc_OverflowError>;
using PermissionError       = ExceptionWrapper<&PyExc_PermissionError>;
using ReferenceError        = ExceptionWrapper<&PyExc_ReferenceError>;
using RuntimeError          = ExceptionWrapper<&PyExc_RuntimeError>;
using StopIteration         = ExceptionWrapper<&PyExc_StopIteration>;
using SystemExit            = ExceptionWrapper<&PyExc_SystemExit>;
using TimeoutError          = ExceptionWrapper<&PyExc_TimeoutError>;
using TypeError             = ExceptionWrapper<&PyExc_TypeError>;
using ValueError            = ExceptionWrapper<&PyExc_ValueError>;
using ZeroDivisionError     = ExceptionWrapper<&PyExc_ZeroDivisionError>;


/**
 * Raises 'Exception' if value is not zero.
 */
inline void check_zero(int value)
{
  assert(value == 0 || value == -1);
  if (value != 0)
    throw Exception();
}


/**
 * Raises 'Exception' if 'value' is -1; returns it otherwise.
 */
template<typename TYPE>
inline TYPE check_not_minus_one(TYPE value)
{
  if (value == -1)
    throw Exception();
  else
    return value;
}


/**
 * Raises 'Exception' if value is not true.
 */
inline void check_true(int value)
{
  if (value == 0)
    throw Exception();
}


/**
 * Raises 'Exception' if 'value' is null; otherwise, returns it.
 */
inline Object* check_not_null(PyObject* obj)
{
  if (obj == nullptr)
    throw Exception();
  else
    return (Object*) obj;
}


inline Type* check_not_null(PyTypeObject* type)
{
  if (type == nullptr)
    throw Exception();
  else
    return (Type*) type;
}


//------------------------------------------------------------------------------

template<typename T>
inline T* cast(PyObject* obj)
{
  assert(T::Check(obj));  // FIXME: TypeError?
  return static_cast<T*>(obj);
}


//------------------------------------------------------------------------------

inline PyObject* incref(PyObject* obj)
{
  Py_INCREF(obj);
  return obj;
}


inline PyObject* decref(PyObject* obj)
{
  Py_DECREF(obj);
  return obj;
}


//------------------------------------------------------------------------------

/**
 * Type-generic base class for references.
 *
 * An instance of this class owns a reference to a Python object.
 */
class baseref
{
public:

  ~baseref() 
  {
    clear();
  }

  Object* release()
  {
    auto obj = obj_;
    obj_ = nullptr;
    return obj;
  }

  void clear();

protected:

  baseref(Object* obj) : obj_(obj) {}

  Object* obj_;

};


// FIXME: ref<> does not work with Type and derived objects, since it's not
// a subtype of Object.  This would be hard to do because Object derives from
// PyObject, which declares data attributes, the same as PyTypeObject.
//
// One way around this would be to separate the mixin Object, like PyObject
// with extra helpers, from a BaseObject type that derives from the concrete
// PyObject.

template<typename T>
class ref
  : public baseref
{
public:

  /**
   * Takes an existing reference.
   *
   * Call this method on an object pointer that comes with an assumed reference,
   * such as the return value of an API call that returns ownership.
   */
  // FIXME: Maybe do an unchecked cast here, since C API functions do a check?
  static ref<T> take(PyObject* obj)
    { return ref(cast<T>(obj)); }

  /**
   * Creates a new reference.
   */
  static ref<T> of(ref<T> obj_ref)
    { return of(obj_ref.obj_); }

  /**
   * Creates a new reference.
   */
  static ref<T> of(T* obj)
    { incref(obj); return ref{obj}; }

  /**
   * Creates a new reference, casting.
   */
  static ref<T> of(PyObject* obj)
    { return of(cast<T>(obj)); }

  /** 
   * Default ctor: null reference.  
   */
  ref()
    : baseref(nullptr) {}

  /** 
   * Move ctor.  
   */
  ref(ref<T>&& ref)
    : baseref(ref.release()) {}

  /** 
   * Move ctor from another ref type.  
   */
  template<typename U>
  ref(ref<U>&& ref)
    : baseref(ref.release()) {}

  /**
   * Returns a new (additional) reference.
   */
  ref inc() const
    { return ref::of(obj_); }

  void operator=(ref<T>&& ref)
    { clear(); obj_ = ref.release(); }

  bool operator==(T* const ptr) const
    { return obj_ == ptr; }

  bool operator!=(T* const ptr) const
    { return obj_ != ptr; }

  operator T&() const
    { return *(T*) obj_; }

  operator T*() const
    { return (T*) obj_; }

  T* operator->() const
    { return (T*) obj_; }

  T* release()
    { return (T*) baseref::release(); }

private:

  ref(T* obj)
    : baseref(obj) 
  {}

};


inline ref<Object> none_ref()
{
  return ref<Object>::of(Py_None);
}


inline ref<Object> not_implemented_ref()
{
  return ref<Object>::of(Py_NotImplemented);
}


//==============================================================================

class Object
  : public PyObject
{
public:

  ref<Object> CallMethodObjArgs(char const* name, bool check=false);
  // FIXME: Hacky.
  ref<Object> CallMethodObjArgs(char const* name, PyObject* arg0, bool check=false);
  ref<Object> CallMethodObjArgs(char const* name, PyObject* arg0, PyObject* arg1, bool check=false);
  ref<Object> CallObject(Tuple* args);
  static bool Check(PyObject* obj)
    { return true; }
  ref<Object> GetAttrString(char const* const name, bool check=true)
  { 
    auto result = PyObject_GetAttrString(this, name);
    if (check)
      result = check_not_null(result);
    return ref<Object>::take(result);
  }
  bool IsInstance(PyObject* type)
    { return (bool) PyObject_IsInstance(this, type); }
  auto Length()
    { return PyObject_Length(this); }
  auto Repr()
    { return ref<Unicode>::take(PyObject_Repr(this)); }
  auto SetAttrString(char const* name, PyObject* obj)
    { check_not_minus_one(PyObject_SetAttrString(this, name, obj)); }
  auto Str()
    { return ref<Unicode>::take(PyObject_Str(this)); }

  ref<py::Long> Long();
  long long_value();

};


inline
ref<Object>
Object::CallMethodObjArgs(
  char const* name,
  bool check)
{
  ref<Object> method = GetAttrString(name, false);
  if (check)
    check_not_null(method);
  auto result = PyObject_CallFunctionObjArgs(method, nullptr);
  if (check)
    check_not_null(result);
  return ref<Object>::take(result);
}


inline
ref<Object>
Object::CallMethodObjArgs(
  char const* name,
  PyObject* arg0,
  bool check)
{
  ref<Object> method = GetAttrString(name, false);
  if (check)
    check_not_null(method);
  auto result = PyObject_CallFunctionObjArgs(method, arg0, nullptr);
  if (check)
    check_not_null(result);
  return ref<Object>::take(result);
}


inline
ref<Object>
Object::CallMethodObjArgs(
  char const* name,
  PyObject* arg0,
  PyObject* arg1,
  bool check)
{
  ref<Object> method = GetAttrString(name, false);
  if (check)
    check_not_null(method);
  auto result = PyObject_CallFunctionObjArgs(method, arg0, arg1, nullptr);
  if (check)
    check_not_null(result);
  return ref<Object>::take(result);
}


template<typename T>
inline std::ostream& operator<<(std::ostream& os, ref<T>& ref)
{
  os << ref->Str()->as_utf8();
  return os;
}


//------------------------------------------------------------------------------

class Dict
  : public Object
{
public:

  static bool Check(PyObject* const obj)
    { return PyDict_Check(obj); }

  Object* GetItemString(char const* const key)
  { 
    Object* const value = (Object*) PyDict_GetItemString(this, key);
    if (value == nullptr)
      throw KeyError(key);
    else
      return value;
  }

  void SetItemString(char const* const key, PyObject* const value)
    { check_zero(PyDict_SetItemString(this, key, value)); }

  Py_ssize_t Size()
    { return check_not_minus_one(PyDict_Size(this)); }

};


//------------------------------------------------------------------------------

class Bool
  : public Object
{
public:

  static ref<Bool> const TRUE;
  static ref<Bool> const FALSE;

  static bool Check(PyObject* obj)
    { return PyBool_Check(obj); }
  static auto from(bool value)
    { return ref<Bool>::of(value ? Py_True : Py_False); }

  operator bool()
    { return this == Py_True; }

};


//------------------------------------------------------------------------------

class Long
  : public Object
{
public:

  static bool Check(PyObject* obj)
    { return PyLong_Check(obj); }
  static auto FromLong(long val)
    { return ref<Long>::take(PyLong_FromLong(val)); }

  operator long()
    { return PyLong_AsLong(this); }

};


//------------------------------------------------------------------------------

class Float
  : public Object
{
public:

  static bool Check(PyObject* obj)
    { return PyFloat_Check(obj); }
  static auto FromDouble(double val)
    { return ref<Float>::take(PyFloat_FromDouble(val)); }

  operator double()
    { return PyFloat_AsDouble(this); }

};


//------------------------------------------------------------------------------

class Module
  : public Object
{
public:

  static bool Check(PyObject* obj)
    { return PyModule_Check(obj); }
  static auto Create(PyModuleDef* def)
    { return ref<Module>::take(PyModule_Create(def)); }
  void AddObject(char const* name, PyObject* val)
    { check_zero(PyModule_AddObject(this, name, incref(val))); }
  char const* GetName()
    { return PyModule_GetName(this); }
  static ref<Module> ImportModule(char const* const name)
    { return ref<Module>::take(check_not_null(PyImport_ImportModule(name))); }

  void add(PyTypeObject* type)
  {
    // Make sure the qualified name of the type includes this module's name.
    std::string const qualname = type->tp_name;
    std::string const mod_name = PyModule_GetName(this);
    auto dot = qualname.find_last_of('.');
    assert(dot != std::string::npos);
    assert(qualname.compare(0, dot, mod_name) == 1);
    // Add it, under its unqualified name.
    AddObject(qualname.substr(dot + 1).c_str(), (PyObject*) type);
  }

};


//------------------------------------------------------------------------------

class Sequence
  : public Object
{
public:

  static bool Check(PyObject* obj)
    { return PySequence_Check(obj); }

  Object* GetItem(Py_ssize_t index)
    { return check_not_null(PySequence_GetItem(this, index)); }

  Py_ssize_t Length()
    { return PySequence_Length(this); }

};


//------------------------------------------------------------------------------

class Tuple
  : public Sequence
{
public:

  static bool Check(PyObject* obj)
    { return PyTuple_Check(obj); }

  static auto New(Py_ssize_t len)
    { return ref<Tuple>::take(PyTuple_New(len)); }

  void initialize(Py_ssize_t index, baseref&& ref)
    { PyTuple_SET_ITEM(this, index, ref.release()); }

  Object* GetItem(Py_ssize_t index) 
    { return check_not_null(PyTuple_GET_ITEM(this, index)); }

  Py_ssize_t GetLength() 
    { return PyTuple_GET_SIZE(this); }

  // FIXME: Remove?
  static auto from(std::initializer_list<PyObject*> items) 
  {
    auto len = items.size();
    auto tuple = New(len);
    Py_ssize_t index = 0;
    for (auto item : items) 
      PyTuple_SET_ITEM((PyObject*) tuple, index++, item);
    return tuple;
  }

private:

  /**
   * Recursive template for building fixed-sized tuples.
   */
  template<Py_ssize_t LEN> class Builder;

public:

  static Builder<0> const builder;

};


// FIXME: The syntax for using this isn't great.
template<Py_ssize_t LEN>
class Tuple::Builder
{
public:

  Builder(Builder<LEN - 1> last, baseref&& ref) 
    : last_(last),
      obj_(ref.release()) 
  {}

  ~Builder() { assert(obj_ == nullptr); }

  /**
   * Takes 'ref' to append the end of the tuple.
   */
  auto operator<<(baseref&& ref) const
  {
    return Builder<LEN + 1>(*this, std::move(ref));
  }

  /**
   * Builds the tuple.
   */
  operator ref<Tuple>()
  {
    auto tuple = ref<Tuple>::take(PyTuple_New(LEN));
    initialize(tuple);
    return tuple;
  }

  void initialize(PyObject* tuple)
  {
    assert(obj_ != nullptr);
    last_.initialize(tuple);
    PyTuple_SET_ITEM(tuple, LEN - 1, obj_);
    obj_ = nullptr;
  }

private:

  Builder<LEN - 1> last_;
  PyObject* obj_;

};


/**
 * Base case for recursive tuple builder template.
 */
template<>
class Tuple::Builder<0>
{
public:

  Builder() {}

  auto operator<<(baseref&& ref) const 
  { 
    return Builder<1>(*this, std::move(ref)); 
  }

  operator ref<Tuple>() const 
  { 
    return ref<Tuple>::take(PyTuple_New(0)); 
  }

  void initialize(PyObject* tuple) const {}

};


//------------------------------------------------------------------------------

class Type
  : public PyTypeObject
{
public:

  Type() {}
  Type(PyTypeObject o) : PyTypeObject(o) {}

  void Ready()
    { check_zero(PyType_Ready(this)); }

};


//------------------------------------------------------------------------------

class StructSequence
  : public Object
{
public:

  Object* GetItem(Py_ssize_t index)
    { return check_not_null(PyStructSequence_GET_ITEM(this, index)); }

  void initialize(Py_ssize_t index, baseref&& ref)
    { PyStructSequence_SET_ITEM(this, index, ref.release()); }

};


//------------------------------------------------------------------------------

class StructSequenceType
  : public Type
{
public:

  static void InitType(StructSequenceType* type, PyStructSequence_Desc* desc)
    { check_zero(PyStructSequence_InitType2(type, desc)); }

#if 0
  static StructSequenceType* NewType(PyStructSequence_Desc* desc)
    { return (StructSequenceType*) check_not_null(PyStructSequence_NewType(desc)); }
#endif

  // FIXME: Doesn't work; see https://bugs.python.org/issue20066.  We can't
  // just set TPFLAGS_HEAPTYPE, as the returned type object doesn't have the
  // layout that implies.
  ref<StructSequence> New()
    { return ref<StructSequence>::take(check_not_null((PyObject*) PyStructSequence_New((PyTypeObject*) this))); }

};


//------------------------------------------------------------------------------

class Unicode
  : public Object
{
public:

  static bool Check(PyObject* obj)
    { return PyUnicode_Check(obj); }

  static auto FromString(char* utf8)
    { return ref<Unicode>::take(PyUnicode_FromString(utf8)); }
  // FIXME: Cast on const here?
  static auto FromStringAndSize(char* utf8, size_t length)
    { return ref<Unicode>::take(PyUnicode_FromStringAndSize(utf8, length)); }

  static auto from(std::string const& str)
    { return FromStringAndSize(const_cast<char*>(str.c_str()), str.length()); }

  static auto from(char character)
    { return FromStringAndSize(&character, 1); }

  char* as_utf8() { return PyUnicode_AsUTF8(this); }

  std::string as_utf8_string()
  {
    Py_ssize_t length;
    char* const utf8 = PyUnicode_AsUTF8AndSize(this, &length);
    if (utf8 == nullptr)
      throw Exception();
    else
      return std::string(utf8, length);
  }

};


template<>
inline std::ostream& operator<<(std::ostream& os, ref<Unicode>& ref)
{
  os << ref->as_utf8();
  return os;
}


//==============================================================================

inline void baseref::clear()
{
  if (obj_ != nullptr)
    decref(obj_);
}


inline ref<Long>
Object::Long()
{
  return ref<py::Long>::take(check_not_null(PyNumber_Long(this)));
}


inline long
Object::long_value()
{
  return (long) *Long();
}


//==============================================================================

namespace Arg {

inline void
ParseTuple(
  Tuple* const args,
  char const* const format,
  ...)
{
  va_list vargs;
  va_start(vargs, format);
  auto result = PyArg_VaParse(args, (char*) format, vargs);
  va_end(vargs);
  check_true(result);
}


inline void ParseTupleAndKeywords(
    Tuple* args, Dict* kw_args, 
    char const* format, char const* const* keywords, ...)
{
  va_list vargs;
  va_start(vargs, keywords);
  auto result = PyArg_VaParseTupleAndKeywords(
      args, kw_args, (char*) format, (char**) keywords, vargs);
  va_end(vargs);
  check_true(result);
}


}  // namespace Arg

//==============================================================================

class ExtensionType
  : public Object
{
public:

};


//==============================================================================

// Buffer objects
// See https://docs.python.org/3/c-api/buffer.html.

/**
 * A unique reference view to a buffer object.
 *
 * Supports move semantics only; no copy.
 */
class BufferRef
{
public:

  /**
   * Creates a buffer view of an object.  The ref holds a reference to the
   * object.
   */
  BufferRef(PyObject* obj, int flags)
  {
    if (PyObject_GetBuffer(obj, &buffer_, flags) != 0)
      throw Exception();
    assert(buffer_.obj != nullptr);
  }

  BufferRef(Py_buffer&& buffer)
    : buffer_(buffer)
  {}

  BufferRef(BufferRef&& ref)
    : buffer_(ref.buffer_)
  {
    ref.buffer_.obj = nullptr;
  }

  ~BufferRef()
  {
    // Only releases if buffer_.obj is not null.
    PyBuffer_Release(&buffer_);
    assert(buffer_.obj == nullptr);
  }

  BufferRef(BufferRef const&) = delete;
  void operator=(BufferRef const&) = delete;

  Py_buffer* operator->() { return &buffer_; }

private:

  Py_buffer buffer_;

};


//==============================================================================
// Inline methods

inline ref<Object> 
Object::CallObject(Tuple* args)
{ 
  return ref<Object>::take(check_not_null(PyObject_CallObject(this, args))); 
}


//==============================================================================

template<typename CLASS>
using MethodPtr = ref<Object> (*)(CLASS* self, Tuple* args, Dict* kw_args);

using StaticMethodPtr = ref<Object> (*)(Tuple* args, Dict* kw_args);

using ClassMethodPtr = ref<Object> (*)(PyTypeObject* class_, Tuple* args, Dict* kw_args);


/**
 * Wraps a method that takes args and kw_args and returns an object.
 */
template<typename CLASS, MethodPtr<CLASS> METHOD>
PyObject* wrap(PyObject* self, PyObject* args, PyObject* kw_args)
{
  ref<Object> result;
  try {
    result = METHOD(
      reinterpret_cast<CLASS*>(self),
      reinterpret_cast<Tuple*>(args),
      reinterpret_cast<Dict*>(kw_args));
  }
  catch (Exception) {
    return nullptr;
  }
  assert(result != nullptr);
  return result.release();
}


template<StaticMethodPtr METHOD>
PyObject* 
wrap_static_method(
  PyObject* /* unused */, 
  PyObject* args, 
  PyObject* kw_args)
{
  ref<Object> result;
  try {
    result = METHOD(
      reinterpret_cast<Tuple*>(args),
      reinterpret_cast<Dict*>(kw_args));
  }
  catch (Exception) {
    return nullptr;
  }
  assert(result != nullptr);
  return result.release();
}


template<ClassMethodPtr METHOD>
PyObject* 
wrap_class_method(
  PyObject* class_,
  PyObject* args, 
  PyObject* kw_args)
{
  ref<Object> result;
  try {
    result = METHOD(
      reinterpret_cast<PyTypeObject*>(class_),
      reinterpret_cast<Tuple*>(args),
      reinterpret_cast<Dict*>(kw_args));
  }
  catch (Exception) {
    return nullptr;
  }
  assert(result != nullptr);
  return result.release();
}


template<typename CLASS>
class Methods
{
public:

  Methods() : done_(false) {}

  template<MethodPtr<CLASS> METHOD>
  Methods& add(char const* name, char const* doc=nullptr)
  {
    assert(name != nullptr);
    assert(!done_);
    methods_.push_back({
      name,
      (PyCFunction) wrap<CLASS, METHOD>,
      METH_VARARGS | METH_KEYWORDS,
      doc
    });
    return *this;
  }

  template<StaticMethodPtr METHOD>
  Methods& add_static(char const* const name, char const* const doc=nullptr)
  {
    assert(name != nullptr);
    assert(!done_);
    methods_.push_back({
      name,
      (PyCFunction) wrap_static_method<METHOD>,
      METH_VARARGS | METH_KEYWORDS | METH_STATIC,
      doc
    });
    return *this;
  }

  template<ClassMethodPtr METHOD>
  Methods& add_class(char const* const name, char const* const doc=nullptr)
  {
    assert(name != nullptr);
    assert(!done_);
    methods_.push_back({
      name,
      (PyCFunction) wrap_class_method<METHOD>,
      METH_VARARGS | METH_KEYWORDS | METH_CLASS,
      doc
    });
    return *this;
  }

  operator PyMethodDef*()
  {
    if (!done_) {
      // Add the sentry.
      methods_.push_back({nullptr, nullptr, 0, nullptr});
      done_ = true;
    }
    return &methods_[0];
  }

private:

  bool done_;
  std::vector<PyMethodDef> methods_;

};


//------------------------------------------------------------------------------

template<typename CLASS>
using GetPtr = ref<Object> (*)(CLASS* self, void* closure);


template<typename CLASS, GetPtr<CLASS> METHOD>
PyObject* wrap_get(PyObject* self, void* closure)
{
  ref<Object> result;
  try {
    result = METHOD(reinterpret_cast<CLASS*>(self), closure); 
  }
  catch (Exception) {
    return nullptr;
  }
  assert(result != nullptr);
  return result.release();
}


template<typename CLASS>
class GetSets
{
public:

  GetSets() : done_(false) {}

  template<GetPtr<CLASS> METHOD>
  GetSets& add_get(char const* name, char const* doc=nullptr, 
                   void* closure=nullptr)
  {
    assert(name != nullptr);
    assert(!done_);
    getsets_.push_back({
      (char*)   name,
      (getter)  wrap_get<CLASS, METHOD>,
      (setter)  nullptr,
      (char*)   doc,
      (void*)   closure,
    });
    return *this;
  }

  operator PyGetSetDef*()
  {
    if (!done_) {
      // Add the sentry.
      getsets_.push_back({nullptr, nullptr, nullptr, nullptr, nullptr});
      done_ = true;
    }
    return &getsets_[0];
  }

private:

  bool done_;
  std::vector<PyGetSetDef> getsets_;

};


//==============================================================================

inline ref<Object>
import(const char* module_name, const char* name)
{
  return Module::ImportModule(module_name)->GetAttrString(name);
}


//------------------------------------------------------------------------------

}  // namespace py

