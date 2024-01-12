"""Unit test for dummy envpool and speed benchmark."""

import time

import envpool  # noqa: F401
import envpool.sokoban.registration
import numpy as np
from absl import logging
from absl.testing import absltest
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
            levels_dir="/aa/boxoban-levels-master/unfiltered/train",
        )
        total_steps = 1000

        _ = env.reset()
        t = time.time()
        for _ in range(total_steps):
            _ = env.step(np.random.randint(low=0, high=9, size=(num_envs,)))
        duration = time.time() - t
        fps = total_steps * batch / duration
        logging.info(f"FPS = {fps:.6f}")

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
            levels_dir="/aa/boxoban-levels-master/unfiltered/train",
        )
        handle, recv, send, step = env.xla()


if __name__ == "__main__":
    absltest.main()
