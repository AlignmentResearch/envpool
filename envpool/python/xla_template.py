# Copyright 2022 Garena Online Private Limited
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
"""xla template on python side."""

from collections import namedtuple
from functools import partial
from typing import Any, Callable, Dict, List, Tuple, Union

import numpy as np
import jax
from jax import numpy as jnp


def make_xla(obj: Any) -> Any:
  """Return callables that can be jitted in a namedtuple.

  Args:
    obj: The object that has a `_xla` function.
      All instances of envpool has a `_xla` function that returns
      the necessary information for creating jittable send/recv functions.

  Returns:
    XlaFunctions: A namedtuple, the first element is a handle
      representing `obj`. The rest of the elements are jittable functions.
  """
  handle, handlers = obj._xla()

  # Convert handle to numpy array
  handle_array = np.frombuffer(handle, dtype=np.uint8)

  # Create a dictionary to store the FFI functions
  ffi_functions = {}

  # Register each handler with JAX's FFI
  for handler_info in handlers:
    name, capsule, attr_specs = handler_info

    # Register the FFI target with JAX
    target_name = f"{type(obj).__name__}_{id(obj)}_{name}"
    jax.ffi.register_ffi_target(
      target_name,
      capsule,
      platform="cpu" if "Gpu" not in name else "gpu"
    )

    # Create a function that calls the FFI
    def make_ffi_call(target, name):
      # Determine if this is a Send or Recv operation
      is_send = name.startswith("Send")

      def ffi_function(*args):
        # For Send operations, we pass the inputs and get no outputs
        if is_send:
          # Create output type (just the handle)
          output_type = jax.ShapeDtypeStruct((len(handle_array),), np.uint8)

          # Create the FFI call
          call = jax.ffi.ffi_call(
            target,
            output_type,
            vmap_method="broadcast_all"
          )

          # Call the FFI with the handle as an attribute and the inputs
          return call(*args, envpool=handle_array)
        else:
          # For Recv operations, we get outputs based on the state spec
          # Get the state specs from the object
          state_specs = obj.spec.state_spec.AllValues()
          batch_size = obj.spec.config["batch_size"]
          max_num_players = obj.spec.config["max_num_players"]

          # Create output types for each state
          output_types = []
          for spec in state_specs:
            # Normalize the shape based on batch size and max_num_players
            if not spec.shape or len(spec.shape) == 0:
              shape = [batch_size]
            elif spec.shape[0] == -1:  # If first dim is max_num_players
              shape = [batch_size * max_num_players] + list(spec.shape[1:])
            else:
              shape = [batch_size] + list(spec.shape)

            # Create the ShapeDtypeStruct for this output
            dtype = np.dtype(spec.dtype)
            output_types.append(jax.ShapeDtypeStruct(shape, dtype))

          # Create the FFI call
          call = jax.ffi.ffi_call(
            target,
            tuple(output_types) if len(output_types) > 1 else output_types[0],
            vmap_method="broadcast_all"
          )

          # Call the FFI with the handle as an attribute
          return call(envpool=handle_array)

      return ffi_function

    # Create the FFI function and add it to our dictionary
    ffi_functions[name] = make_ffi_call(target_name, name)

  # Create a namedtuple with the handle and all FFI functions
  XlaFunctions = namedtuple(
    "XlaFunctions",
    ["handle"] + list(ffi_functions.keys())
  )

  # Return the namedtuple with the handle and all FFI functions
  return XlaFunctions(handle_array, **ffi_functions)
