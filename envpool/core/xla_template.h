/*
 * Copyright 2022 Garena Online Private Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef ENVPOOL_CORE_XLA_TEMPLATE_H_
#define ENVPOOL_CORE_XLA_TEMPLATE_H_

#include <cuda_runtime_api.h>
#include <pybind11/functional.h>
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <string>
#include <tuple>
#include <vector>

#include "xla/ffi/api/c_api.h"
#include "xla/ffi/api/ffi.h"

namespace py = pybind11;
namespace ffi = xla::ffi;

template <typename Spec>
static auto SpecToTuple(const Spec& spec) {
  return std::make_tuple(py::dtype::of<typename Spec::dtype>(), spec.shape);
}

// Helper function to create a PyCapsule from an FFI handler
template <typename T>
py::capsule EncapsulateFfiHandler(T* handler) {
  return py::capsule(reinterpret_cast<void*>(handler), "xla.ffi.api.handler");
}

// Helper class to register FFI handlers for a class
template <typename Class>
struct FfiHandlers {
  static py::tuple GetSendHandler(Class* obj) {
    return py::make_tuple(
        py::str("Send"),
        EncapsulateFfiHandler(Send<Class>),
        py::make_tuple(SpecToTuple(Spec<uint8_t>({sizeof(Class*)})))
    );
  }

  static py::tuple GetSendGpuHandler(Class* obj) {
    return py::make_tuple(
        py::str("SendGpu"),
        EncapsulateFfiHandler(SendGpu<Class>),
        py::make_tuple(SpecToTuple(Spec<uint8_t>({sizeof(Class*)})))
    );
  }

  static py::tuple GetRecvHandler(Class* obj) {
    return py::make_tuple(
        py::str("Recv"),
        EncapsulateFfiHandler(Recv<Class>),
        py::make_tuple(SpecToTuple(Spec<uint8_t>({sizeof(Class*)})))
    );
  }

  static py::tuple GetRecvGpuHandler(Class* obj) {
    return py::make_tuple(
        py::str("RecvGpu"),
        EncapsulateFfiHandler(RecvGpu<Class>),
        py::make_tuple(SpecToTuple(Spec<uint8_t>({sizeof(Class*)})))
    );
  }

  static py::tuple GetHandlers(Class* obj) {
    py::list handlers;
    handlers.append(GetSendHandler(obj));
    handlers.append(GetSendGpuHandler(obj));
    handlers.append(GetRecvHandler(obj));
    handlers.append(GetRecvGpuHandler(obj));

    // Create a handle for the object
    py::bytes handle = py::bytes(
        std::string(reinterpret_cast<const char*>(&obj), sizeof(Class*)));

    return py::make_tuple(handle, handlers);
  }
};

#endif  // ENVPOOL_CORE_XLA_TEMPLATE_H_
