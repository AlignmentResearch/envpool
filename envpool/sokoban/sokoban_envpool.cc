#include "envpool/sokoban/sokoban_envpool.h"
#include "envpool/core/py_envpool.h"

// generate python-side (raw) SokobanEnvSpec
using SokobanEnvSpec = PyEnvSpec<sokoban::SokobanEnvSpec>;
// generate python-side (raw) SokobanEnvPool
using SokobanEnvPool = PyEnvPool<sokoban::SokobanEnvPool>;

// generate sokoban_envpool.so
PYBIND11_MODULE(sokoban_envpool, m) {
  REGISTER(m, SokobanEnvSpec, SokobanEnvPool)
}
