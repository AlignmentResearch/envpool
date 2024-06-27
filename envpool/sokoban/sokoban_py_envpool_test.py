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
import sys
import time

import numpy as np
import pytest

import envpool  # noqa: F401
import envpool.sokoban.registration
from envpool.sokoban.sokoban_envpool import _SokobanEnvSpec
from pathlib import Path
from typing import List


def test_config() -> None:
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


def test_envpool() -> None:
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

  assert env.action_space.n == 4
  for _ in range(total_steps):
    _ = env.step(np.random.randint(low=0, high=4, size=(num_envs,)))
  duration = time.time() - t
  fps = total_steps * batch / duration
  print(f"FPS = {fps:.6f}")


def test_envpool_max_episode_steps() -> None:
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


def test_envpool_load_sequentially(capfd) -> None:
  levels_dir = "/app/envpool/sokoban/sample_levels"
  files = glob.glob(f"{levels_dir}/*.txt")
  levels_by_files = []
  for file in sorted(files):
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


def test_xla() -> None:
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


SOLVE_LEVEL_ZERO: str = "222200001112330322210"
TINY_COLORS: list[tuple[tuple[int, int, int], str]] = [
  ((0, 0, 0), "#"),
  ((243, 248, 238), " "),
  ((254, 126, 125), "."),
  ((254, 95, 56), "s"),
  ((142, 121, 56), "$"),
  ((160, 212, 56), "@"),
  ((219, 212, 56), "a"),
]


def print_obs(obs: np.ndarray):
  assert obs.shape == (3, 10, 10)
  printed = ""
  for y in range(obs.shape[1]):
    for x in range(obs.shape[2]):
      arr = obs[:, y, x]
      printed_any = False
      for color, symbol in TINY_COLORS:
        assert arr.shape == (3,)
        if np.array_equal(arr, color):
          printed += symbol
          printed_any = True
          break
      assert printed_any, f"Could not find match for {arr}"
    printed += "\n"
  printed += "\n"
  return printed


action_astar_to_envpool = {
  "0": 0,
  "1": 3,
  "2": 1,
  "3": 2,
}


def make_1d_array(action: int | str) -> np.ndarray:
  return np.array(int(action))[None]


@pytest.mark.parametrize("solve_on_time", [True, False])
def test_solved_level_does_not_truncate(solve_on_time: bool):
  """
  Test that a level that gets solved just in time does not get truncated. But if
  it does not get solved just in time, it gets truncated.
  """
  max_episode_steps = len(SOLVE_LEVEL_ZERO)
  env = envpool.make(
    "Sokoban-v0",
    env_type="gymnasium",
    num_envs=1,
    batch_size=1,
    min_episode_steps=max_episode_steps,
    max_episode_steps=max_episode_steps,
    levels_dir="/app/envpool/sokoban/sample_levels",
    load_sequentially=True,
  )
  # Skip levels in 000.txt and 001.txt
  for _ in range(3 + 3):
    env.reset()
  env.reset()  # Load level 0

  for a in SOLVE_LEVEL_ZERO[:-1]:
    obs, reward, term, trunc, infos = env.step(
      make_1d_array(action_astar_to_envpool[a])
    )
    assert not term and not trunc, "Level should not have reached time limit"

  wrong_action = str((int(SOLVE_LEVEL_ZERO[-1]) + 1) % 4)

  if solve_on_time:
    obs, reward, term, trunc, infos = env.step(
      make_1d_array(action_astar_to_envpool[SOLVE_LEVEL_ZERO[-1]])
    )
    assert reward == (
      env.spec.config.reward_step + env.spec.config.reward_box +
      env.spec.config.reward_finished
    ), (f"the level wasn't solved successfully. Level: {print_obs(obs[0])}")
    assert term and not trunc, "Level should finish within the time limit"

  else:
    obs, reward, term, trunc, infos = env.step(make_1d_array(wrong_action))
    assert not term and trunc, "Level should truncate at precisely this step"

  _, _, term, trunc, _ = env.step(make_1d_array(wrong_action))
  assert not term and not trunc, "Level should reset correctly"


def read_levels_file(fpath: Path) -> List[List[str]]:
  maps = []
  current_map = []
  with open(fpath, "r") as sf:
    for line in sf.readlines():
      if ";" in line and current_map:
        maps.append(current_map)
        current_map = []
      if "#" == line[0]:
        current_map.append(line.strip())

  maps.append(current_map)
  return maps


def test_load_sequentially_with_multiple_envs() -> None:
  levels_dir = "/app/envpool/sokoban/sample_levels"
  files = glob.glob(f"{levels_dir}/*.txt")
  levels_by_files = []
  total_levels, num_envs = 8, 2
  for file in sorted(files):
    levels = read_levels_file(file)
    levels_by_files.extend(levels)
  assert len(levels_by_files) == total_levels, "8 levels stored in files."

  env = envpool.make(
    "Sokoban-v0",
    env_type="gymnasium",
    num_envs=num_envs,
    batch_size=num_envs,
    max_episode_steps=60,
    min_episode_steps=60,
    levels_dir=levels_dir,
    load_sequentially=True,
    n_levels_to_load=total_levels,
    verbose=2,
  )
  dim_room = env.spec.config.dim_room
  printed_obs = []
  for _ in range(total_levels // num_envs):
    obs, _ = env.reset()
    assert obs.shape == (
      num_envs,
      3,
      dim_room,
      dim_room,
    ), f"obs shape: {obs.shape}"
    for idx in range(num_envs):
      printed_obs.append(print_obs(obs[idx]).strip().split("\n"))
  for i, level in enumerate(levels_by_files):
    for j, line in enumerate(level):
      assert printed_obs[i][j] == line, f"Level {i} is not loaded correctly."


def test_astar_log(tmp_path) -> None:
  level_file_name = "/app/envpool/sokoban/sample_levels/small.txt"
  log_file_name = tmp_path / "log_file.csv"
  subprocess.run(
    [
      "/root/go/bin/bazel", f"--output_base={str(tmp_path)}", "run",
      "//envpool/sokoban:astar_log", "--", level_file_name,
      str(log_file_name), "1"
    ],
    check=True,
    cwd="/app/envpool",
    env={
      "HOME": "/root",
      "PATH": "/opt/conda/bin:/usr/bin"
    },
  )
  log = log_file_name.read_text()
  assert f"0,{SOLVE_LEVEL_ZERO},21,1380" == log.split("\n")[1]


if __name__ == "__main__":
  retcode = pytest.main(["-v", __file__])
  sys.exit(retcode)
