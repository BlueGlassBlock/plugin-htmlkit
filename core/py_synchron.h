#ifndef PY_SYNCHRON_H
#define PY_SYNCHRON_H

#include <Python.h>
#include <memory>
#include <mutex>
#include <condition_variable>

class GILState {
public:
    PyGILState_STATE gil_state;

    GILState() {
        gil_state = PyGILState_Ensure();
    }

    ~GILState() {
        PyGILState_Release(gil_state);
    }
};

class PyObjectPtr {
public:
    PyObject* ptr = nullptr;

    explicit PyObjectPtr(PyObject* ptr, bool inc_ref = false) : ptr(ptr) {
        if (ptr == nullptr) {
            return;
        }
        if (inc_ref) {
            Py_INCREF(ptr);
        }
    }

    ~PyObjectPtr() {
        Py_XDECREF(ptr);
    }

    bool operator==(const nullptr_t p) const {
        return ptr == p;
    }

    bool operator!=(const nullptr_t p) const {
        return ptr != p;
    }
};

struct PyWaiter {
    std::string name;
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
PyObject* waiter_wait(PyWaiter* waiter);

#endif //PY_SYNCHRON_H
