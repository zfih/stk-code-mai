# Supervision 9
# What we haven't done yet
* Pad many observations and stack them to give agent better idea of current state

# Model input
* Give the model velocity for better understanding of world

# Controls redesign
* The input method is issue, the model does not know what is being pressed.
It might try to go forward but having backwards pressed.
* Redesign input scheme to lift finger to reset actions when getting ready to take new action
* Redesign actions to only be pressed buttons, meaning no action is pressed with value 0.
* Also combinations of buttons:
	* Forward
	* Forward + Left
	* Forward + Right
	* Left
	* etc.
	
# Work:
* Make list of reset actions, that are all called to reset whole action space
* Make new list of actions that are button presses <- see above.
* Add velocity to model
* Start report, first three sections can already be written
* Write scientific plan + experiments
* Stack observations for model