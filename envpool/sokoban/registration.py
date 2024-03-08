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

from envpool.registration import register

register(
  task_id="Sokoban-v0",
  import_path="envpool.sokoban",
  spec_cls="SokobanEnvSpec",
  dm_cls="SokobanDMEnvPool",
  gym_cls="SokobanGymEnvPool",
  gymnasium_cls="SokobanGymnasiumEnvPool",
  max_episode_steps=60,
  reward_step=-0.1,
  max_num_players=1,
)
