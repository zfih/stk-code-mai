# Plan



# Experiments
## Experiment 1 - Fake Data
 - Problem: Not sure if network was being updated properly
 - Hypothesis: Giving the network a large reward going forward will teach it to just go forward
 - Gave it fake replay memory with random distance down track, and random distance to center
 - Half of the memory had going forward as the action, the other was random other action
 - When action was forward, the reward was 10, otherwise 0.
 - Result:
	- The output of the network after the first optimize was 90% (0.9) probability of going forward
	- The second optimize was 100% (1.0)
 - Conclusion:
	- Network seems to be updated correctly and optimized correctly

## Experiment 2 - Sparser optimization
 - Problem: Agent would often wiggle back forth and not really get anywhere and therefor
 get very low reward, before doing a new action.
 - Hypothesis: Letting agent experience the changes in environment after an action is taken
 will make results and weights more realistic and relevant.
 - Made the agent only optimize and check the network every 20th update, about 3-7 times a second depending on framerate
 - Result:
	- The agent actually moves and takes actions and drives more like a car rather than a random agent
 - Conclusion:
	- May be better, we need to do A/B test of the change.


## Experiment 3 - Larger network
 - Network was small according to feedback, try larger network
 - Hypothesis: Larger network will make learning better, according to experience from Teacher and TA.
 - We had a network that was 2-8-64-8
 - Changed network to 2-128-128-8
 - Result:
	- Learned something somewhat different
	- Learned to only turn left, even with probability choosing
	- Probability of turning left must have been very big
 - Conclusion:
	- It did not learn better
	- Maybe more tests are needed since probability of a single action was so overweight


## Experiment 3.1 - Larger network with more input
 - Only 2 pieces of input was not a lot for the large amount of things going on
 - Hypothesis: More input will make the base of the decision making stronger
 - Added rotation as an input
 - Cleaned up distance down track
 - Result:
	- The same as for experiment 3
 - Conclusion:
	- We see no difference in the decision making.

## Experiment 4 - Different control scheme
 - The current control scheme could be problematic. If an action of { accelerate, max } is chosen, the kart keeps accelerating while other actions are being taken, affecting the reward.
 - Hypothesis: Reward values will make more sense if the effect of actions are confined to each call to the network's getAction().
 - Changed available actions to accel, brake, steer left, steer right, combinations of accel and steering, and combinations of brake and steering.
 - Changed MAIController to reset all actions before taking an action.
 - Result:
	- Learned to only accelerate and turn right at the same time
 - Conclusion:
	- The agent still doesn't have a good base for decision making

## Experiment 5 - Velocity as input
 - It might be hard for the agent to choose the right action without know how fast it is already going in different directions.
 - Hypothesis: Adding velocity as input to the model will make the base of the decision making stronger.
 - Added velocity (in x, y, and z) as an input.
 - Result:
	- Learned to steer right all the time, even with probability choosing.
 - Conclusion:
	- We see no difference in the decision making.
	