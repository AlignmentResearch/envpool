/*
 * Copyright 2021 Garena Online Private Limited
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

#ifndef ENVPOOL_CORE_XLA_H_
#define ENVPOOL_CORE_XLA_H_

#include <cuda_runtime_api.h>

#include <cstdint>
#include <memory>
#include <numeric>
#include <string>
#include <tuple>
#include <vector>

#include "envpool/core/array.h"
#include "xla/ffi/api/ffi.h"
#include "xla/ffi/api/c_api.h"

namespace ffi = ffi;


namespace envpool {

/**
 * A wrapper so XLA recognizes EnvPool* as a custom host-only type.
 *
 * We must make sure to call RegisterTypeId once in the library initialization code.
 */
template <typename EnvPool>
struct EnvPoolPtr {
  EnvPool* ptr;
  // We'll register this type ID somewhere in your library init code.
  static ffi::TypeId id;

  static void RegisterTypeId() {
    const XLA_FFI_Api* api = ffi::GetXlaFfiApi();

    std::string name = "EnvPoolPtr." + std::string(typeid(EnvPool).name());
    auto error = ffi::Ffi::RegisterTypeId(api, name, &EnvPoolPtr<EnvPool>::id);
    try {
      if (error != ffi::ErrorCode::kOk) {
        throw std::runtime_error(std::string("Failed to register EnvPoolPtr type id: ") +
                                ffi::internal::GetErrorMessage(api, error));
      }
    } finally {
      ffi::internal::DestroyError(api, error);
    }
  }
};

template <typename D>
constexpr bool is_container_v = false;  // NOLINT
template <typename D>
constexpr bool is_container_v<Container<D>> = true;  // NOLINT
template <typename... T>
constexpr bool HasContainerType(std::tuple<T...> /*unused*/) {
  return (is_container_v<typename T::dtype> || ...);
}
bool HasDynamicDim(const std::vector<int>& shape) {
  return std::any_of(shape.begin() + 1, shape.end(),
                     [](int s) { return s == -1; });
}
template <typename... T>
bool HasDynamicDim(const std::tuple<T...>& state_spec) {
  bool dyn = false;
  std::apply([&](auto&&... spec) { dyn = (HasDynamicDim(spec.shape) || ...); },
             state_spec);
  return dyn;
}

template <typename Dtype>
Array CpuBufferToArray(const void* buffer, ::Spec<Dtype> spec, int batch_size,
                       int max_num_players) {
  if (!spec.shape.empty() &&
      spec.shape[0] == -1) {  // If first dim is max_num_players
    spec.shape[0] = max_num_players * batch_size;
  } else {
    spec = spec.Batch(batch_size);
  }
  Array ret(spec);
  ret.Assign(reinterpret_cast<const Dtype*>(buffer), ret.size);
  return ret;
}

template <typename Dtype>
Array GpuBufferToArray(cudaStream_t stream, const void* buffer,
                       ::Spec<Dtype> spec, int batch_size,
                       int max_num_players) {
  if (!spec.shape.empty() &&
      spec.shape[0] == -1) {  // If first dim is max_num_players
    spec.shape[0] = max_num_players * batch_size;
  } else {
    spec = spec.Batch(batch_size);
  }
  Array ret(spec);
  cudaMemcpyAsync(ret.Data(), buffer, ret.size * ret.element_size,
                  cudaMemcpyDeviceToHost, stream);
  return ret;
}

template <typename Dtype>
::Spec<Dtype> NormalizeSpec(const ::Spec<Dtype>& spec, int batch_size,
                            int max_num_players) {
  std::vector<int> shape({0});
  if (!spec.shape.empty() && spec.shape[0] == -1) {
    shape[0] = batch_size * max_num_players;
    shape.insert(shape.end(), spec.shape.begin() + 1, spec.shape.end());
  } else {
    shape[0] = batch_size;
    shape.insert(shape.end(), spec.shape.begin(), spec.shape.end());
  }
  return ::Spec<Dtype>(shape);
}

/**
 * If Spec is a container, the xla interface should be disabled.
 */
template <typename D>
::Spec<D> NormalizeSpec(const ::Spec<Container<D>>& spec, int batch_size,
                        int max_num_players) {
  std::vector<int> shape({0});
  if (!spec.shape.empty() && spec.shape[0] == -1) {
    shape[0] = batch_size * max_num_players;
    shape.insert(shape.end(), spec.shape.begin() + 1, spec.shape.end());
  } else {
    shape[0] = batch_size;
    shape.insert(shape.end(), spec.shape.begin(), spec.shape.end());
  }
  return ::Spec<D>(shape);
}


/**
 * XlaSend: sends "action" arrays from the user to EnvPool::Send(...).
 *
 * Now the first argument is a typed user-data pointer for EnvPool,
 * instead of a raw pointer in a buffer.
 */
template <typename EnvPool>
struct XlaStep {

  using FirstActionType = std::tuple_element<0, EnvPool::Action::Values>::dtype;
  auto FirstActionFFIDtype = xla::ffi::internal::NativeTypeToCApiDataType<FirstActionType>;

  std::tuple<int, int, ffi::Error> CheckDtypesAndGetBsNp(EnvPoolPtr<EnvPool> env_ud, ffi::Buffer<FirstActionFFIDtype> in_actions) {
    EnvPool* envpool = env_ud->ptr;
    const auto action_spec = envpool->spec.action_spec.AllValues();
    const auto action_dtype = xla::ffi::internal::NativeTypeToCApiDataType<std::tuple_element<0, decltype(action_spec)>>;
    const int batch_size = envpool->spec.config["batch_size"_];
    const int max_num_players = envpool->spec.config["max_num_players"_];

    // This is a dynamic assertion because XLA will be checking the buffer time at runtime, so the compiler does not
    // know it.
    if(action_dtype != in_actions.element_type()) {
      return {batch_size, max_num_players, ffi::Error::InvalidArgument("XlaSend CPU: action dtype mismatch")};
    }

    // We could use a static assert here, but we haven't implemented multiple input argument handling
    // more than one action input. But we haven't implemented handling for those.
    if (std::tuple_size_v<decltype(action_spec)> != 1) {
      return {batch_size, max_num_players, ffi::Error::InvalidArgument("action_spec must contain exactly one entry.")};
    }

    // Same for output arguments
    const auto obs_spec = envpool->spec["obs"_]




    return {batch_size, max_num_players, ffi::Error::Success()};
  }

  static ffi::Error Cpu(EnvPoolPtr<EnvPool> env_ud, ffi::Buffer<FirstActionFFIDtype> in_actions) {
    // Instantiate vector with 1 element directly, we know at compile time it will be 1 element.
    std::vector<Array> out_actions{{
        CpuBufferToArray(in_actions, std::tuple_element_v<0, action_spec>, batch_size, max_num_players)
    }};
    envpool->Send(out_actions);
    return ffi::Error::Success();
  }

  static decltype(auto) CpuBinding = ffi::Ffi::Bind()
        .Arg<ffi::UserData<EnvPoolPtr<EnvPool>>>()
        .Arg<ffi::Buffer<FirstActionFFIDtype>>();


  static ffi::Error Gpu(cudaStream_t stream, EnvPoolPtr<EnvPool> env_ud, ffi::Buffer<FirstActionFFIDtype> in_actions) {
    // Same as CPU, but with GpuBuffer and synchronizing
    EnvPool* envpool = env_ud->ptr;
    const auto action_spec = envpool->spec.action_spec.AllValues();
    const auto action_dtype = xla::ffi::internal::NativeTypeToCApiDataType<std::tuple_element<0, decltype(action_spec)>>;

    // This is a dynamic assertion because XLA will be checking the buffer time at runtime, so the compiler does not
    // know it.
    if(action_dtype != in_actions.element_type()) {
      return ffi::Error::InvalidArgument("XlaSend CPU: action dtype mismatch");
    }

    // We use a dynamic assert here, because we're going to compile this function for all EnvPools, even those that have
    // more than one action input. But we haven't implemented handling for those.
    if (std::tuple_size_v<decltype(action_spec)> != 1) {
        return ffi::Error::InvalidArgument("action_spec must contain exactly one entry.");
    }

    const int batch_size = envpool->spec.config["batch_size"_];
    const int max_num_players = envpool->spec.config["max_num_players"_];
    // Instantiate vector with 1 element directly, we know at compile time it will be 1 element.
    std::vector<Array> out_actions{{
        GpuBufferToArray(in_actions, std::tuple_element_v<0, action_spec>, batch_size, max_num_players)
    }};
    cudaStreamSynchronize(stream);  // Ensure the Cuda arrays for every environment have been copied to host
    envpool->Send(out_actions);
    return ffi::Error::Success();
  }

  static decltype(auto) CpuBinding = ffi::Ffi::Bind()
        .Ctx<cudaStream_t>()
        .Arg<ffi::UserData<EnvPoolPtr<EnvPool>>>()
        .Arg<ffi::Buffer<FirstActionFFIDtype>>();
};

/**
 * XlaRecv: receives "state" arrays from EnvPool::Recv(...).
 *
 * We again pass ffi::UserData for the EnvPool pointer.
 */
template <typename EnvPool>
struct XlaRecv {
  using In = std::array<void*, 0>;
  using Out =
      std::array<void*, std::tuple_size_v<typename EnvPool::State::Keys>>;

  static decltype(auto) InSpecs(EnvPool* /*envpool*/) {
    return std::tuple<>();
  }

  static decltype(auto) OutSpecs(EnvPool* envpool) {
    int batch_size = envpool->spec.config["batch_size"_];
    int max_num_players = envpool->spec.config["max_num_players"_];
    return std::apply(
        [&](auto&&... s) {
          return std::make_tuple(NormalizeSpec(s, batch_size, max_num_players)...);
        },
        envpool->spec.state_spec.AllValues());
  }

  // CPU: EnvPool::Recv => copy to host buffers
  static ffi::Error Cpu(ffi::UserData<EnvPoolPtr<EnvPool>> env_ud,
                             const In& /*unused*/, const Out& out) {
    EnvPool* envpool = env_ud->ptr;
    int batch_size = envpool->spec.config["batch_size"_];
    int max_num_players = envpool->spec.config["max_num_players"_];
    std::vector<Array> recv = envpool->Recv();

    for (std::size_t i = 0; i < recv.size(); ++i) {
      // Check shape
      if (recv[i].Shape(0) > (std::size_t)batch_size * max_num_players) {
        return ffi::Error::InternalError("Shape mismatch in XlaRecv CPU");
      }
      std::memcpy(out[i], recv[i].Data(), recv[i].size * recv[i].element_size);
    }
    return ffi::Error::Success();
  }

  // GPU: EnvPool::Recv => copy from host => device buffers
  static ffi::Error Gpu(ffi::UserData<EnvPoolPtr<EnvPool>> env_ud,
                             cudaStream_t stream, const In& /*unused*/,
                             const Out& out) {
    EnvPool* envpool = env_ud->ptr;
    int batch_size = envpool->spec.config["batch_size"_];
    int max_num_players = envpool->spec.config["max_num_players"_];
    std::vector<Array> recv = envpool->Recv();

    for (std::size_t i = 0; i < recv.size(); ++i) {
      if (recv[i].Shape(0) > (std::size_t)batch_size * max_num_players) {
        return ffi::Error::InternalError("Shape mismatch in XlaRecv GPU");
      }
      cudaMemcpyAsync(out[i], recv[i].Data(),
                      recv[i].size * recv[i].element_size,
                      cudaMemcpyHostToDevice, stream);
    }
    return ffi::Error::Success();
  }
};
}  // namespace envpool

#endif  // ENVPOOL_CORE_XLA_H_
