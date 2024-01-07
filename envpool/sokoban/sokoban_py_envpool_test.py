"""Unit test for dummy envpool and speed benchmark."""

import os
import time

import numpy as np
from absl import logging
from absl.testing import absltest
from envpool.sokoban.sokoban_envpool import _SokobanEnvPool, _SokobanEnvSpec


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
            # Default and also used by sokoban
            "max_episode_steps",
            # defined by sokoban
            "dim_room",
            "levels_dir",
            "reward_box",
            "reward_finished",
            "reward_step",
        ]
        default_conf = _SokobanEnvSpec._default_config_values
        self.assertTrue(isinstance(default_conf, tuple))
        config_keys = _SokobanEnvSpec._config_keys
        self.assertTrue(isinstance(config_keys, list))
        self.assertEqual(len(default_conf), len(config_keys))
        self.assertEqual(sorted(config_keys), sorted(ref_config_keys))

    def test_envpool(self) -> None:
        conf = dict(
            zip(_SokobanEnvSpec._config_keys, _SokobanEnvSpec._default_config_values)
        )
        conf["num_envs"] = num_envs = 200
        conf["batch_size"] = batch = 100
        conf["num_threads"] = 10
        env_spec = _SokobanEnvSpec(tuple(conf.values()))
        env = _SokobanEnvPool(env_spec)
        state_keys = env._state_keys
        total = 1
        env._reset(np.arange(num_envs, dtype=np.int32))
        raise ValueError("resetted")
        t = time.time()
        for _ in range(total):
            state = dict(zip(state_keys, env._recv()))
            action = {
                "env_id": state["info:env_id"],
                "players.env_id": state["info:players.env_id"],
                "list_action": np.zeros((batch, 6), dtype=np.float64),
                "players.id": state["info:players.id"],
                "players.action": state["info:players.id"],
            }
            # env._send(tuple(action.values()))
        duration = time.time() - t
        fps = total * batch / duration
        logging.info(f"FPS = {fps:.6f}")

    def test_xla(self) -> None:
        conf = dict(
            zip(_SokobanEnvSpec._config_keys, _SokobanEnvSpec._default_config_values)
        )
        conf["num_envs"] = 100
        conf["batch_size"] = 31
        conf["num_threads"] = os.cpu_count()
        env_spec = _SokobanEnvSpec(tuple(conf.values()))
        env = _SokobanEnvPool(env_spec)
        _ = env._xla()


if __name__ == "__main__":
    absltest.main()
