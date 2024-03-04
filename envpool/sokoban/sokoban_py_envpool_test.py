# Copyright 2023-2024 FAR AI
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
"""Unit test for dummy envpool and speed benchmark."""

import time

import numpy as np
from absl import logging
from absl.testing import absltest

import envpool  # noqa: F401
import envpool.sokoban.registration
from envpool.sokoban.sokoban_envpool import _SokobanEnvSpec


class _SokobanEnvPoolTest(absltest.TestCase):

  def test_config(self) -> None:
    ref_config_keys = [
      # Default environment keys
      "base_path",
      "batch_size",
      "gym_reset_return_info",
      "max_num_players",
      "num_envs",
      "num_threads",
      "seed",
      "thread_affinity_offset",
      "min_episode_steps",
      # Default and also used by sokoban
      "max_episode_steps",
      # defined by sokoban
      "dim_room",
      "levels_dir",
      "reward_box",
      "reward_finished",
      "reward_step",
      "verbose",
    ]
    default_conf = _SokobanEnvSpec._default_config_values
    self.assertTrue(isinstance(default_conf, tuple))
    config_keys = _SokobanEnvSpec._config_keys
    self.assertTrue(isinstance(config_keys, list))
    self.assertEqual(len(default_conf), len(config_keys))
    self.assertEqual(sorted(config_keys), sorted(ref_config_keys))

  def test_envpool(self) -> None:
    batch = num_envs = 200
    env = envpool.make(
      "Sokoban-v0",
      env_type="gymnasium",
      num_envs=num_envs,
      batch_size=num_envs,
      seed=2346890,
      max_episode_steps=60,
      reward_step=-0.1,
      dim_room=10,
      levels_dir="/app/envpool/sokoban/sample_levels",
    )
    total_steps = 1000

    _ = env.reset()
    t = time.time()
    for _ in range(total_steps):
      _ = env.step(np.random.randint(low=0, high=9, size=(num_envs,)))
    duration = time.time() - t
    fps = total_steps * batch / duration
    logging.info(f"FPS = {fps:.6f}")

  def test_envpool_max_episode_steps(self) -> None:
    for max_episode_steps in [2, 5, 10]:
      env = envpool.make(
        "Sokoban-v0",
        env_type="gymnasium",
        num_envs=1,
        batch_size=1,
        min_episode_steps=max_episode_steps,
        max_episode_steps=max_episode_steps,
        levels_dir="/app/envpool/sokoban/sample_levels",
      )
      env.reset()
      for _ in range(max_episode_steps - 1):
        _, _, terminated, truncated, _ = env.step(np.zeros([1], dtype=np.int32))
        assert not np.any(terminated | truncated)

      _, _, terminated, truncated, _ = env.step(np.zeros([1], dtype=np.int32))
      assert not np.any(terminated)
      assert np.all(truncated)

  def test_xla(self) -> None:
    num_envs = 10
    env = envpool.make(
      "Sokoban-v0",
      env_type="dm",
      num_envs=num_envs,
      batch_size=num_envs,
      seed=2346890,
      max_episode_steps=60,
      reward_step=-0.1,
      dim_room=10,
      levels_dir="/app/envpool/sokoban/sample_levels",
    )
    handle, recv, send, step = env.xla()


if __name__ == "__main__":
  absltest.main()
