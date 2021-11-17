// C++ example how to load Python functions

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <stdio.h>

int
main(int argc, char *argv[])
{
    wchar_t *program = Py_DecodeLocale(argv[0], NULL);
    if (program == NULL) {
        fprintf(stderr, "Fatal error: cannot decode argv[0]\n");
        exit(1);
    }
    Py_SetProgramName(program);  /* optional but recommended */
    Py_Initialize();
    
    PyRun_SimpleString("import sys");
    PyRun_SimpleString("import os");
    PyRun_SimpleString("sys.path.append(os.getcwd())");
    
    PyObject *pname, *pmodule, *pFunc, *pArgs;
    
    pname = PyUnicode_FromString("Thorlabs_shutter");
    
    pmodule = PyImport_Import(pname);
    
    pFunc = PyObject_GetAttrString(pmodule, "initialize");
    
    if(pFunc){
        printf("Succefully loaded\n");
        pArgs = PyTuple_New(0);
        PyObject_CallObject(pFunc, pArgs);
    }

    pFunc = PyObject_GetAttrString(pmodule, "unblock");
    
    if(pFunc){
        printf("Succefully loaded\n");
        pArgs = PyTuple_New(0);
        PyObject_CallObject(pFunc, pArgs);
    }
    
    
    if (Py_FinalizeEx() < 0) {
        exit(120);
    }
    PyMem_RawFree(program);
    return 0;

}