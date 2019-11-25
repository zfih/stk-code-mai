//
// Created by LasseKB on 07/11/2019.
//

// Based on https://radicalrafi.github.io/posts/pytorch-cpp-intro/, https://github.com/goldsborough/examples/blob/cpp/cpp/mnist/mnist.cpp, 
// and https://pytorch.org/tutorials/intermediate/reinforcement_q_learning.html

#include "mai_dqntrainer.hpp"
#include "modes/standard_race.hpp" // This should probably not be here.
#include <math.h>
//#include <torch/csrc/jit/import.h>
//#include <torch/csrc/jit/export.h>

#define BATCH_SIZE 128
#define GAMMA 0.999
#define EPS_START 0.9
#define EPS_END 0.05
#define EPS_DECAY 200
#define TARGET_UPDATE 10
#define SAVE_MODEL 500

const std::string modelName = "DQNmodel.pt";

inline bool fileExists(const std::string& name) {
	struct stat buffer;
	return (stat(name.c_str(), &buffer) == 0);
}

MAIDQNTrainer::MAIDQNTrainer(MAIDQNModel *model) {
	m_policyNet = model;

	if (fileExists(modelName)) {
		std::cout << modelName << " existed. Loading the model.\n";
		torch::serialize::InputArchive inArchive = torch::serialize::InputArchive();
		inArchive.load_from(modelName);
		m_policyNet->getModule()->load(inArchive);
	}

	m_targetNet = new MAIDQNModel(model->getKartID());

	saveToTargetModel();

	m_stepsDone = 0;
	srand(time(NULL));
	torch::optim::RMSprop(m_policyNet->getModule()->parameters(), 0.1);
	m_optimiser = dynamic_cast<torch::optim::Optimizer*>(new torch::optim::RMSprop(m_policyNet->getModule()->parameters(), torch::optim::RMSpropOptions(0.01)));
	m_lastState[0] = -1.0f;
	m_lastState[1] = -1.0f;
	m_lastAction = (PlayerAction)-1;
	m_runOnceIteration = 0;
}

PlayerAction MAIDQNTrainer::selectAction(float state[]) {
	float sample = (rand() % 100) / 100.0f;
	float eps_threshold = EPS_END + (EPS_START - EPS_END) * pow(M_E, -1. * m_stepsDone / EPS_DECAY);
	m_stepsDone += 1;
	if (sample > eps_threshold)
		return m_policyNet->getAction(state);
	else 
	{
		int ind = rand() % m_policyNet->getNumActions();
		return m_policyNet->getAction(ind);
	}
}

void MAIDQNTrainer::optimiseModel() {
	if (replayMemory.states.size() < BATCH_SIZE) return;

	torch::Tensor stateTensor /*= torch::zeros({128, 1})*/;
	torch::Tensor actionTensor;
	torch::Tensor rewardTensor;
	torch::Tensor nextStateTensor;

	for (int i = 0; i < BATCH_SIZE; i++) {
		//std::cout << i << "\n";
		int sample = rand() % replayMemory.actions.size();
		//torch::Tensor stateActionValues = m_policyNet->pseudoForward(replayMemory.states[sample]).max();

		//std::cout << stateActionValues << "\n";
		////std::cout << stateActionValues.max() << "\n\n";

		//torch::Tensor nextStateValues = torch::zeros(1);
		//nextStateValues[0] = m_targetNet->pseudoForward(replayMemory.nextStates[sample]).max();
		//torch::Tensor expectedStateActionValues = (nextStateValues * GAMMA) + replayMemory.rewards[sample];

		//auto loss = torch::smooth_l1_loss(stateActionValues, expectedStateActionValues.detach());

		//m_optimiser->zero_grad();
		//loss.backward();
		//m_optimiser->step();

		if (i == 0) {
			stateTensor = torch::cat({ torch::tensor(replayMemory.states[sample * 2]), torch::tensor(replayMemory.states[sample * 2 + 1]) }, 0);
			actionTensor = torch::tensor(replayMemory.actions[sample]);
			rewardTensor = torch::tensor(replayMemory.rewards[sample]);
			nextStateTensor = torch::cat({ torch::tensor(replayMemory.nextStates[sample * 2]), torch::tensor(replayMemory.nextStates[sample * 2 + 1]) });
			continue;
		}

		stateTensor = torch::cat({ stateTensor, torch::tensor(replayMemory.states[sample * 2]), torch::tensor(replayMemory.states[sample * 2 + 1]) }, 0);
		actionTensor = torch::cat({ actionTensor, torch::tensor(replayMemory.actions[sample]) }, 0);
		rewardTensor = torch::cat({ rewardTensor, torch::tensor(replayMemory.rewards[sample]) }, 0);
		nextStateTensor = torch::cat({ nextStateTensor, torch::tensor(replayMemory.nextStates[sample * 2]), torch::tensor(replayMemory.nextStates[sample * 2 + 1]) }, 0);
	}

	stateTensor = stateTensor.reshape({ 128, 2 });
	//std::cout << stateTensor << "\n";
	actionTensor = torch::_cast_Long(actionTensor);
	actionTensor = actionTensor.reshape({ 128, 1 });
	//std::cout << actionTensor << "\n";
	nextStateTensor = nextStateTensor.reshape({ 128,2 });

	torch::Tensor stateActionValues = m_policyNet->forward(stateTensor).gather(1, actionTensor);
	torch::Tensor nextStateValues = std::get<0>(m_targetNet->forward(nextStateTensor).max(1)).detach();
	torch::Tensor expectedStateValues = (nextStateValues * GAMMA) + rewardTensor;

	auto loss = torch::smooth_l1_loss(stateActionValues, expectedStateValues.unsqueeze(1));

	m_optimiser->zero_grad();
	loss.backward();
	m_optimiser->step();
}

void MAIDQNTrainer::run() {
	for (int i = 0; i < 50; i++) {
		// TODO reset world
		World *world = World::getWorld();
		StandardRace* srWorld = dynamic_cast<StandardRace*>(world);
		float state[2];
		state[0] = srWorld->getDistanceDownTrackForKart(m_policyNet->getKartID(), /*Account for checklines? WTH is this?*/false);
		state[1] = 0.0f; // Placeholder
		float startState = state[0];
		bool raceDone = false;
		while (!raceDone) {
			PlayerAction action = selectAction(state);
			// TODO perform the action
			float nextState[2];
			nextState[0] = state[0] + 1.0f; // Placeholder
			nextState[1] = state[1]; // Placeholder
			float reward = 1.0f; // Placeholder
			bool done = (state[0] > startState + 50.0f); // Placeholder // Floating point comparison might be a problem :(
			
			replayMemory.states.push_back(state[0]);
			replayMemory.states.push_back(state[1]);
			replayMemory.actions.push_back(action);
			replayMemory.nextStates.push_back(nextState[0]);
			replayMemory.nextStates.push_back(nextState[1]);
			replayMemory.rewards.push_back(reward);

			state[0] = nextState[0];
			state[1] = nextState[1];

			optimiseModel();
			if (done) break;
		}
		if (i % TARGET_UPDATE == 0) {
			saveToTargetModel();
		}
	}
	std::cout << "Running is fun :D\n";
}

PlayerAction MAIDQNTrainer::runOnce() {
	World* world = World::getWorld();
	StandardRace* srWorld = dynamic_cast<StandardRace*>(world);
	float state[2];
	state[0] = srWorld->getDistanceDownTrackForKart(m_policyNet->getKartID(), /*Account for checklines? WTH is this?*/false);
	state[1] = srWorld->getDistanceToCenterForKart(m_policyNet->getKartID());

	PlayerAction action = selectAction(state);
	
	if (m_lastState[0] < 0) {
		m_lastState[0] = state[0];
		m_lastState[1] = state[1];
		m_lastAction = action;
		m_runOnceIteration++;
		return action;
	}

	replayMemory.states.push_back(m_lastState[0]);
	replayMemory.states.push_back(m_lastState[1]);
	replayMemory.actions.push_back(m_lastAction);
	replayMemory.nextStates.push_back(state[0]);
	replayMemory.nextStates.push_back(state[1]);
	replayMemory.rewards.push_back(state - m_lastState);

	m_lastState[0] = state[0];
	m_lastState[1] = state[1];
	PlayerAction tmpAction = m_lastAction;
	m_lastAction = action;

	optimiseModel();

	if (m_runOnceIteration % TARGET_UPDATE == 0) {
		saveToTargetModel();
	}

	if (m_runOnceIteration % SAVE_MODEL == 0) {
		if (!m_policyNet->getModule()->is_serializable()) throw 909;
		auto compilationUnit = std::make_shared<torch::jit::script::CompilationUnit>();
		torch::serialize::OutputArchive outArchive = torch::serialize::OutputArchive(compilationUnit);
		m_policyNet->getModule()->save(outArchive);
		outArchive.save_to(modelName);
		std::cout << "Saved the model to " << modelName << "\n";
	}

	m_runOnceIteration++;
	return tmpAction;
}

void MAIDQNTrainer::saveToTargetModel() {
	// Make targetNet's weights the same as policyNet's
	if (!m_policyNet->getModule()->is_serializable()) throw 909;
	auto compilationUnit = std::make_shared<torch::jit::script::CompilationUnit>();
	torch::serialize::OutputArchive outArchive = torch::serialize::OutputArchive(compilationUnit);
	m_policyNet->getModule()->save(outArchive);
	outArchive.save_to("tempPolicyToTarget.pt");
	torch::serialize::InputArchive inArchive = torch::serialize::InputArchive();
	inArchive.load_from("tempPolicyToTarget.pt");
	m_targetNet->getModule()->load(inArchive);
}