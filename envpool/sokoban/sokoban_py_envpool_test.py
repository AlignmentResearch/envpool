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
            "num_envs",
            "batch_size",
            "num_threads",
            "max_num_players",
            "thread_affinity_offset",
            "base_path",
            "seed",
            "gym_reset_return_info",
            "state_num",
            "action_num",
            "max_episode_steps",
        ]
        default_conf = _SokobanEnvSpec._default_config_values
        self.assertTrue(isinstance(default_conf, tuple))
        config_keys = _SokobanEnvSpec._config_keys
        self.assertTrue(isinstance(config_keys, list))
        self.assertEqual(len(default_conf), len(config_keys))
        self.assertEqual(sorted(config_keys), sorted(ref_config_keys))

    def test_spec(self) -> None:
        conf = _SokobanEnvSpec._default_config_values
        env_spec = _SokobanEnvSpec(conf)
        state_spec = env_spec._state_spec
        action_spec = env_spec._action_spec
        state_keys = env_spec._state_keys
        action_keys = env_spec._action_keys
        self.assertTrue(isinstance(state_spec, tuple))
        self.assertTrue(isinstance(action_spec, tuple))
        state_spec = dict(zip(state_keys, state_spec))
        action_spec = dict(zip(action_keys, action_spec))
        # default value of state_num is 10
        self.assertEqual(state_spec["obs:raw"][1][-1], 10)
        self.assertEqual(state_spec["obs:dyn"][1][1][-1], 10)
        # change conf and see if it can successfully change state_spec
        # directly send dict or expose config as dict?
        conf = dict(zip(_SokobanEnvSpec._config_keys, conf))
        conf["state_num"] = 666
        env_spec = _SokobanEnvSpec(tuple(conf.values()))
        state_spec = dict(zip(state_keys, env_spec._state_spec))
        self.assertEqual(state_spec["obs:raw"][1][-1], 666)

    def test_envpool(self) -> None:
        conf = dict(
            zip(_SokobanEnvSpec._config_keys, _SokobanEnvSpec._default_config_values)
        )
        conf["num_envs"] = num_envs = 100
        conf["batch_size"] = batch = 31
        conf["num_threads"] = 10
        env_spec = _SokobanEnvSpec(tuple(conf.values()))
        env = _SokobanEnvPool(env_spec)
        state_keys = env._state_keys
        total = 1000
        env._reset(np.arange(num_envs, dtype=np.int32))
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
            env._send(tuple(action.values()))
        duration = time.time() - t
        fps = total * batch / duration
        logging.info(f"FPS = {fps:.6f}")


if __name__ == "__main__":
    absltest.main()
