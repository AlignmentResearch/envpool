#include "envpool/sokoban/sokoban_envpool.h"

#include <sstream>
#include <stdexcept>

#include "envpool/core/py_envpool.h"

namespace sokoban {

void SokobanEnv::Reset() {
  //
  internal_state_ = *level_loader.RandomLevel(gen_);
  State state = Allocate();
  _reward = 0.0f;

  WriteState();
}
void SokobanEnv::Step(const Action& action) {
  _reward = reward_step;
  // todo actual state transition

  WriteState();
}

void SokobanEnv::WriteState() {
  State state = Allocate();
  state["reward"_] = _reward;
  Array& obs = state["obs"_];
  if (obs.size != 3 * internal_state_.size()) {
    std::stringstream msg;
    msg << "Obs size and level size are different: obs_size=" << obs.size
        << "/3, level_size=" << internal_state_.size()
        << ", dim_room=" << dim_room << std::endl;
    throw std::runtime_error(msg.str());
  }

  // TODO: actually color the image
  for (int i = 0; i < 3; i++) {
    obs(i).Assign(internal_state_.data(), internal_state_.size());
  }
}

}  // namespace sokoban

// generate python-side (raw) SokobanEnvSpec
using SokobanEnvSpec = PyEnvSpec<sokoban::SokobanEnvSpec>;
// generate python-side (raw) SokobanEnvPool
using SokobanEnvPool = PyEnvPool<sokoban::SokobanEnvPool>;

// generate sokoban_envpool.so
PYBIND11_MODULE(sokoban_envpool, m) {
  REGISTER(m, SokobanEnvSpec, SokobanEnvPool)
}
