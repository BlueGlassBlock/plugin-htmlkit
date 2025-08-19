#include "py_synchron.h"
#include <Python.h>
#include <mutex>
#include <condition_variable>

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
        if (PyObject* result = PyObject_CallMethod(py_future, "result", nullptr); result != nullptr) {
            std::unique_lock lock(waiter->mtx);
            waiter->result = result; // steal reference
            Py_INCREF(result);
            waiter->done = true;
            lock.unlock();
            waiter->cv.notify_one();
        }
        else {
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
        "invoke_waiter",
        invoke_waiter,
        METH_VARARGS,
        "Callback for concurrent.futures.Future to invoke native synchronization objects."
    };
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
    PyObject* add_cb_result = PyObject_CallMethod(py_future, "add_done_callback", "O", invoke_waiter_fn);
    Py_DECREF(invoke_waiter_fn);
    if (add_cb_result == nullptr) {
        return false;
    }
    return true;
}

PyObject* waiter_wait(const std::unique_ptr<PyWaiter>& waiter) {
    Py_BEGIN_ALLOW_THREADS
        std::unique_lock lock(waiter->mtx);
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
