#ifndef PY_SYNCHRON_H
#define PY_SYNCHRON_H

#include <Python.h>
#include <memory>
#include <mutex>
#include <condition_variable>

struct PyWaiter {
    std::mutex mtx;
    std::condition_variable cv;
    bool done = false;

    PyObject* result = nullptr;
    PyObject* exc_type = nullptr;
    PyObject* exc_val = nullptr;
    PyObject* exc_tb = nullptr;

    ~PyWaiter() {
        Py_XDECREF(result);
        Py_XDECREF(exc_type);
        Py_XDECREF(exc_val);
        Py_XDECREF(exc_tb);
    }
};

bool attach_waiter(PyObject* py_future, PyWaiter* waiter);
PyObject* waiter_wait(const std::unique_ptr<PyWaiter>& waiter);

#endif //PY_SYNCHRON_H
