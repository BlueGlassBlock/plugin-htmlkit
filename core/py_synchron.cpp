/*
Copyright (C) 2025 NoneBot

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, see <https://www.gnu.org/licenses/>.
*/

#include "py_synchron.h"

#include <Python.h>
#include <chrono>
#include <condition_variable>
#include <mutex>

extern "C" {
PyObject* invoke_waiter(PyObject* self, PyObject* args) {
    PyObject* py_future = nullptr;
    if (!PyArg_ParseTuple(args, "O", &py_future)) {
        return nullptr;
    }

    auto* waiter = static_cast<PyWaiter*>(PyCapsule_GetPointer(self, "PyWaiter"));
    if (waiter == nullptr) {
        PyErr_SetString(PyExc_RuntimeError, "PyWaiter capsule invalid");
        return nullptr;
    }
    if (PyObject* result = PyObject_CallMethod(py_future, "result", nullptr);
        result != nullptr) {
        std::unique_lock lock(waiter->mtx);
        waiter->result = result; // steal reference
        waiter->done = true;
        lock.unlock();
        waiter->cv.notify_one();
    } else {
        PyObject *exc_ty, *exc_val, *exc_tb;
        PyErr_Fetch(&exc_ty, &exc_val, &exc_tb);
        if (exc_ty != nullptr) {
            PyErr_NormalizeException(&exc_ty, &exc_val, &exc_tb);
        }
        if (exc_tb != nullptr) {
            PyException_SetTraceback(exc_val, exc_tb);
        }

        std::unique_lock lock(waiter->mtx);
        waiter->exc_type = exc_ty;
        waiter->exc_val = exc_val;
        waiter->exc_tb = exc_tb;
        waiter->done = true;
        lock.unlock();
        waiter->cv.notify_one();
    }
    Py_RETURN_NONE;
}

static PyMethodDef def_invoke_waiter = {
    "invoke_waiter", invoke_waiter, METH_VARARGS,
    "Callback for concurrent.futures.Future to invoke native synchronization objects."};
}

bool attach_waiter(PyObject* py_future, PyWaiter* waiter) {
    PyObject* waiter_capsule = PyCapsule_New(waiter, "PyWaiter", nullptr);
    if (waiter_capsule == nullptr) {
        return false;
    }
    PyObject* invoke_waiter_fn = PyCFunction_New(&def_invoke_waiter, waiter_capsule);
    Py_DECREF(waiter_capsule);
    if (invoke_waiter_fn == nullptr) {
        return false;
    }
    PyObject* add_cb_result =
        PyObject_CallMethod(py_future, "add_done_callback", "O", invoke_waiter_fn);
    Py_DECREF(invoke_waiter_fn);
    if (add_cb_result == nullptr) {
        return false;
    }
    return true;
}

PyObject* waiter_wait(PyWaiter* waiter) {
    Py_BEGIN_ALLOW_THREADS std::unique_lock lock(waiter->mtx);
    waiter->cv.wait(lock, [&] { return waiter->done; });
    Py_END_ALLOW_THREADS

        if (waiter->result != nullptr) {
        PyObject* result = waiter->result;
        waiter->result = nullptr;
        return result;
    }
    else {
        PyErr_Restore(waiter->exc_type, waiter->exc_val, waiter->exc_tb);
        waiter->exc_type = waiter->exc_val = waiter->exc_tb = nullptr;
        return nullptr;
    }
}
