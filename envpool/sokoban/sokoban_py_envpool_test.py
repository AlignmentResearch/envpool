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

import glob
import re
import subprocess
import tempfile
import time

import numpy as np
import pytest

import envpool  # noqa: F401
import envpool.sokoban.registration
from envpool.sokoban.sokoban_envpool import _SokobanEnvSpec


class TestSokobanEnvPool:

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
      "load_sequentially",
      "n_levels_to_load",
    ]
    default_conf = _SokobanEnvSpec._default_config_values
    assert isinstance(default_conf, tuple)
    config_keys = _SokobanEnvSpec._config_keys
    assert isinstance(config_keys, list)
    assert len(default_conf) == len(config_keys)
    assert sorted(config_keys) == sorted(ref_config_keys)

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
    print(f"FPS = {fps:.6f}")

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

  def test_envpool_load_sequentially(self, capfd) -> None:
    levels_dir = "/app/envpool/sokoban/sample_levels"
    files = glob.glob(f"{levels_dir}/*.txt")
    levels_by_files = []
    for file in files:
      with open(file, "r") as f:
        text = f.read()
      levels = text.split("\n;")
      levels = ["\n".join(level.split("\n")[1:]).strip() for level in levels]
      levels_by_files.append((file, levels))
    assert len(levels_by_files) > 1
    assert all(len(levels) > 1 for levels in levels_by_files)
    total_levels = sum(len(levels) for levels in levels_by_files)
    for n_levels_to_load in range(1, total_levels + 1):
      env = envpool.make(
        "Sokoban-v0",
        env_type="gymnasium",
        num_envs=1,
        batch_size=1,
        max_episode_steps=60,
        min_episode_steps=60,
        levels_dir=levels_dir,
        load_sequentially=True,
        n_levels_to_load=n_levels_to_load,
        verbose=2,
      )
      dim_room = env.spec.config.dim_room
      obs, _ = env.reset()
      assert obs.shape == (
        1,
        3,
        dim_room,
        dim_room,
      ), f"obs shape: {obs.shape}"
      if n_levels_to_load == -1:
        n_levels_to_load = total_levels
      for _ in range(n_levels_to_load - 1):
        env.reset()
      out, _ = capfd.readouterr()
      files_output = out.split("***")[1:]
      for i, file_output in enumerate(files_output):
        first_line, out = file_output.strip().split("\n", 1)
        result = re.search(r'Loaded (\d+) levels from "(.*\.txt)"', first_line)
        n_levels, file_name = int(result.group(1)), result.group(2)
        lev1, lev2 = out.strip().split("\n\n")
        assert file_name == levels_by_files[i][0]
        assert n_levels == len(levels_by_files[i][1])
        assert lev1 == levels_by_files[i][1][0]
        assert lev2 == levels_by_files[i][1][1]

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

def test_astar_log(self) -> None:
  level_file_name = "/app/envpool/sokoban/sample_levels/001.txt"
  with tempfile.NamedTemporaryFile() as f:
    log_file_name = f.name
    subprocess.run(
      [
        "bazel", "run", "//envpool/sokoban:astar_log", "--", level_file_name,
        log_file_name, 1
      ],
      check=True,
    )
    with open(log_file_name, "r") as f:
      log = f.read()
    assert "0, 111023301012123001101012, 24, 26001" == log.split("\n")[1]


if __name__ == "__main__":
  pytest.main(["-v", __file__])
