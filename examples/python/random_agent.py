import random, vivarium
env=vivarium.make("BallPush-v0",seed=42,headless=True)
obs,info=env.reset(seed=42);total=0.0
for step in range(1,501):
    obs,reward,terminated,truncated,info=env.step([random.uniform(-1,1),random.uniform(-1,1)]);total+=reward
    if terminated or truncated:break
print({"steps":step,"return":round(total,3),"info":info});env.close()
