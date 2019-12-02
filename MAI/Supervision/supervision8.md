# Supervision 02-12-2019

# Status
We've been training over the weekend and it's not working so far. The agents are not moving, drives backwards, or drives in circles.

## What's the problem (Djordje)?
We don't know.
All actions seem to have about the same values.

## What is the input/output (Toke)?
The input is two floats. One is how down the track the kart it, the other is distance to middle of the track.

## What is the reward (Djordje)?
How far down the track the agent has come since the last state.
If it goes backwards, it should get a negative reward.

* DEBUG: Try to see what the distribution of actions before the update and after the update.
* DEBUG: See how our network gets updated.
* DEBUG: Hardcode an example.
* DEBUG: Experiment with learning rate.
* DEBUG: Maybe we have a bug somewhere.

# Experiment with
* Plot the reward signal. If there is some positive and (some) negative rewards we should be golden.
* If the kart drives forward, override the reward to be positive (to prove that our update works).

# Improving the network
* Consider adding more inputs (orientation, velocity, etc.)
* Recurrent NN

# Reading
* Andrej kaparthy blog post "How to train your neural net"

# Ensure we do
* Use proper weight init.

## What is the network like?
Feedforward (very small).

* Add more inputs and more layers.
* Stack the inputs (add together more timeframes to simulate temporal difference)


# Report
* Results
* Reflection
* Which NN do we use and how do they work?
* Show that we can work scientifically (hypothesis, experiment, repeat)


# Formulate the experiment
* Make a plan. Scientifically. Be structured.
