#define PY_SSIZE_T_CLEAN
#include <Python.h>
/* This is necessary for defining new types! */
#include <structmember.h>

#include "cpuid.h"


/* References:
- https://docs.python.org/3/c-api/index.html
- https://docs.python.org/3/extending/newtypes_tutorial.html
    - https://docs.python.org/3/extending/newtypes_tutorial.html#adding-data-and-methods-to-the-basic-example
- https://docs.python.org/3/c-api/module.html
- https://docs.python.org/3/c-api/structures.html
    - https://docs.python.org/3/c-api/structures.html#implementing-functions-and-methods
*/

#define UNUSED __attribute__((unused))


static int is_cpuid_valid_cache;
static const char *vendor_name;

static UNUSED PyObject *CpuidInvalidError;
static PyObject *pycpuid_valid;
static PyObject *pycpuid_vendor;


typedef struct {
    PyObject_HEAD
    /* Custom members */
    cpuid_t buffer;
} CpuidBufferObject;


static
void buffer_dealloc(CpuidBufferObject *self){
    printf("->dealloc\n");
    Py_TYPE(self)->tp_free((PyObject*)self);
    return;
}

static
PyObject *buffer_new(PyTypeObject *type, PyObject *args, PyObject *kwargs)
{
    CpuidBufferObject *self;

    self = (CpuidBufferObject*)type->tp_alloc(type, 0);
    if (self != NULL){
        memset(&self->buffer, 0, sizeof(self->buffer));
    }
    return (PyObject*)self;
}

static
int buffer_init(CpuidBufferObject *self, PyObject *args, PyObject *kwargs)
{
    /* nothing to init? */
    return 0;
}

static
PyObject *buffer_str(PyObject *self, PyObject *Py_UNUSED(args))
{
    cpuid_t *buf;
    CpuidBufferObject *obj;
    PyObject *str;

    obj = (CpuidBufferObject*)self;
    buf = &obj->buffer;
    str = PyUnicode_FromFormat(
        "<cpuid buffer [eax=0x%08x, ebx=0x%08x, edx=0x%08x, ecx=0x%08x]>",
        buf->flags[IDX_EAX], buf->flags[IDX_EBX], buf->flags[IDX_EDX],
        buf->flags[IDX_ECX]
    );

    return str;
}

/* How to ensure the size of type is correct? */
static PyMemberDef buffer_members[] = {
    {"eax", T_UINT, offsetof(CpuidBufferObject, buffer.flags[IDX_EAX]), 0, 
     "eax register"},
    {"ebx", T_UINT, offsetof(CpuidBufferObject, buffer.flags[IDX_EBX]), 0,
     "ebx register"},
    {"edx", T_UINT, offsetof(CpuidBufferObject, buffer.flags[IDX_EDX]), 0,
     "ebx register"},
    {"ecx", T_UINT, offsetof(CpuidBufferObject, buffer.flags[IDX_ECX]), 0,
     "ecx register"},
    {NULL, 0, 0, 0, NULL},
};

/* Method definitions */

/*
    https://docs.python.org/3/c-api/typeobj.html
*/
static PyTypeObject CpuidBufferType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "pycpuid.Buffer",
    .tp_doc = "CPUID Buffer",
    .tp_basicsize = sizeof(CpuidBufferObject),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE,
    .tp_new = buffer_new, //PyType_GenericNew,
    .tp_dealloc = (destructor)buffer_dealloc,
    .tp_init = (initproc)buffer_init,
    .tp_members = buffer_members,
    /* Magic methods have individual slot. */
    .tp_repr = (reprfunc)buffer_str,
    .tp_str = (reprfunc)buffer_str,
};

static PyModuleDef pycpuid_module = {
    PyModuleDef_HEAD_INIT,
    .m_name = "pycpuid",
    .m_doc = "CPUID wrapper in Python.",
    .m_size = -1,
};


static __attribute__((unused))
int obj_to_buffer(PyObject *obj, void **addr)
{
    int res;
    
    res = PyObject_IsInstance(obj, (PyObject*)&CpuidBufferType);
    if (!res){
        PyErr_SetString(
            PyExc_TypeError, "Incompatible type, required: pycpuid.Buffer"
        );
        return 0;
    }
    *addr = (PyObject*)obj;
    return 1;
}
/* Accept a custom object with class `CpuidBufferType` */
/* > There are different types of function call inside Python internal! */
static
PyObject *py_cpuid_call(PyObject *Py_UNUSED(self), PyObject *args)
{
    /* TODO:
    - Implement a `converter` function, see:
        https://docs.python.org/3/c-api/arg.html#other-objects
    - Study how to get type of a object.
    - Study how to raise an exception.
    - How to implement `__str__`, `__repr__`?
    */
    int res;
    CpuidBufferObject *obj;
    //cpuid_t *buf;
    
    /*
        check is custom type, if not, raise TypeError.
        call `cpuid_call` with buffer
        return none
    */
    /* it is `O`(Oh-). */
    res = PyArg_ParseTuple(args, "O&;", obj_to_buffer, &obj);
    if (!res){
        return NULL;
    }
    cpuid_call(&obj->buffer);

    Py_RETURN_NONE;
}

/* How to add type hint to functions? */
static PyMethodDef mod_methods[] = {
    {"Cpuid", (PyCFunction)py_cpuid_call, METH_VARARGS,
     "A thin wrapper of x86_64 instruction `cpuid`"},
    {NULL, NULL, 0, NULL},
};

/*
- If creating a custom class, register `__str__`.
format: "<cpuid buffer [eax=0x%08x, ebx=0x%08x, edx=0x%08x, ecx=0x%08x]>"
- Module `pycpuid`:
    - class `Buffer`
        - members:
            - eax: uint32, ebx: uint32, edx: uint32, ecx: uint32
    - method `cpuid(Buffer)`
    - static members:
        - CPUID_VALID: bool
        - CPU_VENDOR: str
*/

PyMODINIT_FUNC
PyInit_pycpuid(void)
{
    int res;
    PyObject *mod;
    
    res = cpuid_valid();
    if (!res){
        /* Or prompt user CPUID is unavailable? */
        return NULL;
    }
    /* These attributes should remain unchanged during the execution. */
    is_cpuid_valid_cache = 1;
    vendor_name = cpuid_vendor();
    
    /*  Are these objects handovered after `PyModule_Add*` calls, so
        we don't have to care about ref counts? */
    pycpuid_vendor = PyUnicode_FromString(vendor_name);
    /* hold it temporary. */
    Py_INCREF(Py_True);
    pycpuid_valid = Py_True;
    

    if (PyType_Ready(&CpuidBufferType) < 0){
        return NULL;
    }
    
    printf("->create mod obj\n");
    mod = PyModule_Create(&pycpuid_module);
    if (mod == NULL){
        return NULL;
    }
    
    /* Can this fail? */
    printf("->reg global functions\n");
    res = PyModule_AddFunctions(
        mod, &mod_methods[0]
    );

    /*  On a successful call to `PyModule_AddObject`, the module
        "steal"s a reference count, decreasing the number by one,
        to prevent that the count downs to zero and triggers 
        deallocation, we have to manually up ref by 1.
        
        The mechanism behind ref count is rather simple, so you may
        want to have a look. */

    printf("->reg type\n");
    /* Manually holds a reference. */
    Py_INCREF(&CpuidBufferType);
    res = PyModule_AddObject(
        mod, "Buffer",
        (PyObject*)&CpuidBufferType
    );
    if (res < 0){
        printf("->type reg error\n");
        goto buf_reg_error;
    }
    
    printf("->reg bool const\n");
    /* Now handover the control. */
    Py_INCREF(pycpuid_valid);
    res = PyModule_AddObject(
        mod, "CPUID_VALID", pycpuid_valid
    );
    if (res < 0){
        printf("->bool reg fail\n");
        goto valid_reg_error;
    }
    
    printf("->reg str const\n");
    Py_INCREF(pycpuid_vendor);
    res = PyModule_AddObject(
        mod, "CPUID_VENDOR", pycpuid_vendor
    );
    // TODO: Try trigger import fail?
    if (res < 0){
        /*  At this point, the ref count of `pycpuid_vendor` should be
            2 (because we created the object), to properly trigger
            deallocation, should we dec ref count here? */
        printf("->vendor reg fail\n");
        Py_DECREF(pycpuid_vendor);
        goto vendor_reg_error;
    }

    return mod;
    /* Error handling */
vendor_reg_error:
    Py_DECREF(pycpuid_vendor);
valid_reg_error:
    Py_DECREF(pycpuid_valid);
buf_reg_error:
    Py_DECREF(&CpuidBufferType);
    Py_DECREF(mod);
    return NULL;
}