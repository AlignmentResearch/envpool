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
