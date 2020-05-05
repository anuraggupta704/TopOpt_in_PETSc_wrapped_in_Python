#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION

#include "Python.h"
#include "arrayobject.h"
#include "structmember.h"

#include "topoptlib.h"
#include <vector>

/*
Author: Thijs Smit, May 2020
Copyright (C) 2020 ETH Zurich

Disclaimer:
 The authors reserves all rights but does not guaranty that the code is
 free from errors. Furthermore, we shall not be liable in any event
 caused by the use of the program.
*/

static PyMemberDef members[] = 
    { {"nNodes", T_INT, offsetof(DataObj, nNodes), 0, "nNodes docstring"},
      {"nElements", T_INT, offsetof(DataObj, nElements), 0, "nNodes docstring"},
      {"nDOF", T_INT, offsetof(DataObj, nDOF), 0, "nNodes docstring"},
      {"it", T_INT, offsetof(DataObj, it), 0, "nNodes docstring"},
      {"trueFx", T_DOUBLE, offsetof(DataObj, trueFx), 0, "nNodes docstring"},
      {"scaledFx", T_DOUBLE, offsetof(DataObj, scaledFx), 0, "nNodes docstring"},
      {NULL}  /* Sentinel */
};

// set mesh variables
static PyObject *structuredGrid_py(DataObj *self, PyObject *args)
{
    double xc0, xc1, xc2, xc3, xc4, xc5;
    int nxyz0, nxyz1, nxyz2;

    if(!PyArg_ParseTuple(args, "(dddddd)(iii)", &xc0, &xc1, &xc2, &xc3, &xc4, &xc5, &nxyz0, &nxyz1, &nxyz2)) { 
        return NULL;
    }

    self->xc_w[0] = xc0;
    self->xc_w[1] = xc1;
    self->xc_w[2] = xc2;
    self->xc_w[3] = xc3;
    self->xc_w[4] = xc4;
    self->xc_w[5] = xc5;

    self->nxyz_w[0] = nxyz0;
    self->nxyz_w[1] = nxyz1;
    self->nxyz_w[2] = nxyz2;

    self->nNodes = nxyz0 * nxyz1 * nxyz2;
    self->nElements = (nxyz0 - 1) * (nxyz1 - 1) * (nxyz2 - 1);
    self->nDOF = self->nNodes * 3;

    Py_RETURN_NONE;
}

// set material variables
static PyObject *material_py(DataObj *self, PyObject *args)
{
    double Emin, Emax, nu, penal; 
    
    if(!PyArg_ParseTuple(args, "dddd", &Emin, &Emax, &nu, &penal)) { 
        return NULL;
    }

    self->Emin_w = Emin;
    self->Emax_w = Emax;
    self->nu_w = nu;
    self->penal_w = penal;

    Py_RETURN_NONE;
}

// set filter variables
static PyObject *filter_py(DataObj *self, PyObject *args)
{
    int filter;
    double rmin; 
    
    if(!PyArg_ParseTuple(args, "id", &filter, &rmin)) { 
        return NULL;
    }

    self->filter_w = filter;
    self->rmin_w = rmin;

    Py_RETURN_NONE;
}

// set optimizer variables
static PyObject *mma_py(DataObj *self, PyObject *args)
{
    int maxIter; 
    
    if(!PyArg_ParseTuple(args, "i", &maxIter)) { 
        return NULL;
    }

    self->maxIter_w = maxIter;

    Py_RETURN_NONE;
}

/*
static PyObject *passive_py(DataObj *self, PyObject *args)
{
    PyObject *pypassive_func;
    if (!PyArg_ParseTuple(args, "O", &pypassive_func))
        return NULL;

    // Make sure second argument is a function
    if (!PyCallable_Check(pypassive_func)) 
        return NULL;
    
    //self->passive = true;

    self->passive_func = pypassive_func;
    Py_INCREF(self->passive_func);

    Py_RETURN_NONE;
}
*/

static PyObject *bc_py(DataObj *self, PyObject *args)
{
    
    PyObject *checker;
    PyObject *setter_dof;
    PyObject *setter_val;
 
    int loadcase_ID;
    int BCtypes;
    int nChecker;
    int nSetter_dof;
    int nSetter_val;

    BC condition;

    if (!PyArg_ParseTuple(args, "iiOOO", &loadcase_ID, &BCtypes, &checker, &setter_dof, &setter_val)) {
        return NULL;
    }

    nChecker = PyList_Size(checker);
    nSetter_dof = PyList_Size(setter_dof);
    nSetter_val = PyList_Size(setter_val);
    //printf("Loadcase ID: %i\n", loadcase_ID);
    //printf("Number of Checkers: %i\n", nChecker);
    //printf("Number of Setters dof: %i\n", nSetter_dof);
    //printf("Number of Setters val: %i\n", nSetter_val);

    condition.BCtype = BCtypes;
    
    PyObject *iterator = PyObject_GetIter(checker);
    PyObject *item;

    
    while ((item = PyIter_Next(iterator)))
        {
            int val = PyLong_AsLong(item);
            condition.Checker_vec.push_back(val);
            Py_DECREF(item);
        }

    Py_DECREF(iterator);

    PyObject *iteratorr = PyObject_GetIter(setter_dof);
    PyObject *itemm;

    while ((itemm = PyIter_Next(iteratorr)))
        {
            int vall = PyLong_AsLong(itemm);
            condition.Setter_dof_vec.push_back(vall);
            Py_DECREF(itemm);
        }

    Py_DECREF(iteratorr);

    PyObject *iteratorrr = PyObject_GetIter(setter_val);
    PyObject *itemmm;

    while ((itemmm = PyIter_Next(iteratorrr)))
        {
            double valll = PyFloat_AsDouble(itemmm);
            condition.Setter_val_vec.push_back(valll);
            Py_DECREF(itemmm);
        }

    Py_DECREF(iteratorrr);

    self->loadcases_list.at(loadcase_ID).push_back(condition);

    Py_RETURN_NONE;
}

static PyObject *loadcases_py(DataObj *self, PyObject *args)
{
    int n;

    if (!PyArg_ParseTuple(args, "i", &n)) {
        return NULL;
    }
    
    // Set number of loadcases variable
    self->nL = n;

    // Resize the loadcases list according to user input
    self->loadcases_list.resize(n);

    Py_RETURN_NONE;
}

static PyObject *obj_py(DataObj *self, PyObject *args)
{
    PyObject *pyobj_func;
    if (!PyArg_ParseTuple(args, "O", &pyobj_func))
        return NULL;

    // Make sure second argument is a function
    if (!PyCallable_Check(pyobj_func)) 
        return NULL;
    
    self->obj_func = pyobj_func;
    Py_INCREF(self->obj_func);

    Py_RETURN_NONE;
}

static PyObject *obj_sens_py(DataObj *self, PyObject *args)
{
    PyObject *pyobj_sens_func;
    if (!PyArg_ParseTuple(args, "O", &pyobj_sens_func))
        return NULL;

    // Make sure second argument is a function
    if (!PyCallable_Check(pyobj_sens_func)) 
        return NULL;
    
    self->obj_sens_func = pyobj_sens_func;
    Py_INCREF(self->obj_sens_func);

    Py_RETURN_NONE;
}

static PyObject *volumeConstraint_py(DataObj *self, PyObject *args)
{    
    if(!PyArg_ParseTuple(args, "d", &self->volumefrac_w)) { 
        return NULL;
    }

    Py_RETURN_NONE;
}

static PyObject *const_py(DataObj *self, PyObject *args)
{
    PyObject *pyconst_func;
    if (!PyArg_ParseTuple(args, "O", &pyconst_func))
        return NULL;

    // Make sure second argument is a function
    if (!PyCallable_Check(pyconst_func)) 
        return NULL;
    
    self->const_func = pyconst_func;
    Py_INCREF(self->const_func);

    Py_RETURN_NONE;
}

static PyObject *const_sens_py(DataObj *self, PyObject *args)
{
    PyObject *pyconst_sens_func;
    if (!PyArg_ParseTuple(args, "O", &pyconst_sens_func))
        return NULL;

    // Make sure second argument is a function
    if (!PyCallable_Check(pyconst_sens_func)) 
        return NULL;
    
    self->const_sens_func = pyconst_sens_func;
    Py_INCREF(self->const_sens_func);

    Py_RETURN_NONE;
}

static PyObject *solve_py(DataObj *self)
{
    // variable to store "complete" signal
    int complete = 0;

    // initialte TopOpt loop
    complete = solve(*self);

    // return "complete" signal
    return PyLong_FromLong(complete);
}

static PyObject *vtu_py(DataObj *self, PyObject *args)
{
    PyObject *bin2vtu = PyImport_ImportModule("bin2vtu");

    if (!bin2vtu) {
        PyErr_Print();
        return 0;
    }

    printf("Generating vtu file\n");
    PyObject_CallMethod(bin2vtu, "mainn", ("i"), 1);

    Py_RETURN_NONE;
}

static PyObject *stl_py(DataObj *self, PyObject *args)
{
    printf("Generating stl file\n");
    Py_RETURN_NONE;
}

static PyMethodDef methods[] =
    { {"structuredGrid", (PyCFunction)structuredGrid_py, METH_VARARGS, "Implement structuredGrid\n"},
      {"material", (PyCFunction)material_py, METH_VARARGS, "Implement material\n"},
      {"filter", (PyCFunction)filter_py, METH_VARARGS, "Implement filter\n"},
      {"mma", (PyCFunction)mma_py, METH_VARARGS, "Implement mma\n"},
      //{"passive", (PyCFunction)passive_py, METH_VARARGS, "Implement boundery conditions\n"},
      {"bc", (PyCFunction)bc_py, METH_VARARGS, "Implement boundery conditions\n"},
      {"loadcases", (PyCFunction)loadcases_py, METH_VARARGS, "Implement boundery conditions\n"},
      {"obj", (PyCFunction)obj_py, METH_VARARGS, "Callback for Objective function\n"},
      {"obj_sens", (PyCFunction)obj_sens_py, METH_VARARGS, "Callback for Sensitivity function\n"},
      {"volumeConstraint", (PyCFunction)volumeConstraint_py, METH_VARARGS, "Callback for Sensitivity function\n"},
      {"const", (PyCFunction)const_py, METH_VARARGS, "Callback for Objective function\n"},
      {"const_sens", (PyCFunction)const_sens_py, METH_VARARGS, "Callback for Sensitivity function\n"},
      {"solve", (PyCFunction)solve_py, METH_NOARGS, "Python bindings to solve() in topoptlib\n"},
      {"vtu", (PyCFunction)vtu_py, METH_VARARGS, "Generate vtu\n"},
      {"stl", (PyCFunction)stl_py, METH_VARARGS, "Generate vtu\n"},
      {NULL, NULL, 0, NULL}
    };

static PyTypeObject DataType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "topoptlib.Data",          /* tp_name */
    sizeof(DataObj),           /* tp_basicsize */
    0,                         /* tp_itemsize */
    0,                         /* tp_dealloc */
    0,                         /* tp_print */
    0,                         /* tp_getattr */
    0,                         /* tp_setattr */
    0,                         /* tp_reserved */
    0,                         /* tp_repr */
    0,                         /* tp_as_number */
    0,                         /* tp_as_sequence */
    0,                         /* tp_as_mapping */
    0,                         /* tp_hash  */
    0,                         /* tp_call */
    0,                         /* tp_str */
    0,                         /* tp_getattro */
    0,                         /* tp_setattro */
    0,                         /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,        /* tp_flags */
    "Data objects",            /* tp_doc */
    0,                         /* tp_traverse */
    0,                         /* tp_clear */
    0,                         /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    methods,                   /* tp_methods */
    members,                   /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    0,                         /* tp_init */
    0,                         /* tp_alloc */
    PyType_GenericNew,         /* tp_new */
};

static struct PyModuleDef module =
    {
     PyModuleDef_HEAD_INIT,
     "topoptlib",                           /* m_name */
     "Python bindings to TopOpt_in_PETSc",  /* m_doc */
     -1,                                    /* m_size */
     NULL,                                  /* m_methods */
     NULL,                                  /* m_reload */
     NULL,                                  /* m_traverse */
     NULL,                                  /* m_clear */
     NULL,                                  /* m_free */
    };

PyMODINIT_FUNC PyInit_topoptlib(void)
{
    
    if (PyType_Ready(&DataType) < 0)
        return NULL;
    
    PyObject* m = PyModule_Create(&module);

    import_array();

    // Adding Data python class with C variables to module at initialization
    Py_INCREF(&DataType);
    if (PyModule_AddObject(m, "Data", (PyObject *) &DataType) < 0) {
        Py_DECREF(&DataType);
        Py_DECREF(m);
        return NULL;
    }

    return m;
}

