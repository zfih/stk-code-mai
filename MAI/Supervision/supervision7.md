# Supervision 25/11/2019
## We gave a status
* We can train a DQN though some of the input to the network is wrong
* We looked at debugging the Tensors in c++, but we didn't find a neat way to do it.

## They asked questions
### What is the Reward function?
A: Difference in distanceDownTrack in states

* Experiment with exploration
* If the input to the model and the output of the model is working, then it's just about tweaking different values (???)

### Can we reduce the environment to something trivial?
* Can we learn to just drive down the road, without complexity? It's a way for us to validate that dqn is "working"
* Simple test tracks to differentiate between random agent and our dqn agent.
