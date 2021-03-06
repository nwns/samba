/*
   Samba Unix SMB/CIFS implementation.

   C utilities for the pytalloc test suite.
   Provides the "_test_pytalloc" Python module.

   NOTE: Please read talloc_guide.txt for full documentation

   Copyright (C) Petr Viktorin 2015

     ** NOTE! The following LGPL license applies to the talloc
     ** library. This does NOT imply that all of Samba is released
     ** under the LGPL

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 3 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, see <http://www.gnu.org/licenses/>.
*/

#include <Python.h>
#include <talloc.h>
#include <pytalloc.h>

static PyObject *testpytalloc_new(PyTypeObject *mod)
{
	char *obj = talloc_strdup(NULL, "This is a test string");;
	return pytalloc_steal(pytalloc_GetObjectType(), obj);
}

static PyObject *testpytalloc_get_object_type(PyObject *mod) {
	PyObject *type = (PyObject *)pytalloc_GetObjectType();
	Py_INCREF(type);
	return type;
}

static PyObject *testpytalloc_reference(PyObject *mod, PyObject *args) {
	pytalloc_Object *source = NULL;
	void *ptr;

	if (!PyArg_ParseTuple(args, "O!", pytalloc_GetObjectType(), &source))
		return NULL;

	ptr = source->ptr;
	return pytalloc_reference_ex(pytalloc_GetObjectType(), ptr, ptr);
}

static PyMethodDef test_talloc_methods[] = {
	{ "new", (PyCFunction)testpytalloc_new, METH_NOARGS,
		"create a talloc Object with a testing string"},
	{ "get_object_type", (PyCFunction)testpytalloc_get_object_type, METH_NOARGS,
		"call pytalloc_GetObjectType"},
	{ "reference", (PyCFunction)testpytalloc_reference, METH_VARARGS,
		"call pytalloc_reference_ex"},
	{ NULL }
};

static PyTypeObject DObject_Type;

static int dobject_destructor(void *ptr)
{
	PyObject *destructor_func = *talloc_get_type(ptr, PyObject*);
	PyObject *ret;
	ret = PyObject_CallObject(destructor_func, NULL);
	Py_DECREF(destructor_func);
	if (ret == NULL) {
		PyErr_Print();
	} else {
		Py_DECREF(ret);
	}
	return 0;
}

static PyObject *dobject_new(PyTypeObject *type, PyObject *args, PyObject *kwargs)
{
	PyObject *destructor_func = NULL;
	PyObject **obj;

	if (!PyArg_ParseTuple(args, "O", &destructor_func))
		return NULL;
	Py_INCREF(destructor_func);

	obj = talloc(NULL, PyObject*);
	*obj = destructor_func;

	talloc_set_destructor((void*)obj, dobject_destructor);
	return pytalloc_steal(&DObject_Type, obj);
}

static PyTypeObject DObject_Type = {
	.tp_name = "_test_pytalloc.DObject",
	.tp_basicsize = sizeof(pytalloc_Object),
	.tp_methods = NULL,
	.tp_new = dobject_new,
	.tp_flags = Py_TPFLAGS_DEFAULT,
	.tp_doc = "test talloc object that calls a function when underlying data is freed\n",
};

#define MODULE_DOC "Test utility module for pytalloc"

void init_test_pytalloc(void);
void init_test_pytalloc(void)
{
	PyObject *m;

	DObject_Type.tp_base = pytalloc_GetObjectType();
	if (PyType_Ready(&DObject_Type) < 0) {
		return;
	}

	m = Py_InitModule3("_test_pytalloc", test_talloc_methods, MODULE_DOC);

	if (m == NULL) {
		return;
	}

	Py_INCREF(&DObject_Type);
	Py_INCREF(DObject_Type.tp_base);
	PyModule_AddObject(m, "DObject", (PyObject *)&DObject_Type);
}
