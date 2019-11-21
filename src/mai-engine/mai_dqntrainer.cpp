//
// Created by LasseKB on 07/11/2019.
//

// Based on https://radicalrafi.github.io/posts/pytorch-cpp-intro/, https://github.com/goldsborough/examples/blob/cpp/cpp/mnist/mnist.cpp, 
// and https://pytorch.org/tutorials/intermediate/reinforcement_q_learning.html

#include "mai_dqntrainer.hpp"
#include "modes/standard_race.hpp" // This should probably not be here.
#include <math.h>
#include <torch/csrc/jit/import.h>
#include <torch/csrc/jit/export.h>

#define BATCH_SIZE 128
#define GAMMA 0.999
#define EPS_START 0.9
#define EPS_END 0.05
#define EPS_DECAY 200
#define TARGET_UPDATE 10

MAIDQNTrainer::MAIDQNTrainer(MAIDQNModel *model) {
	m_policyNet = model;
	m_targetNet = new MAIDQNModel(model->getKartID());

	// Make targetNet's weights the same as policyNet's
	if (!m_policyNet->getModule()->is_serializable()) throw 909;
	//torch::save(m_policyNet->getModule(), "tempPolicyToTarget.pt");
	auto compilationUnit = std::make_shared<torch::jit::script::CompilationUnit>();
	torch::serialize::OutputArchive outArchive = torch::serialize::OutputArchive(compilationUnit);
	//(dynamic_cast<torch::nn::Module*>(m_policyNet))->save(outArchive);
	m_policyNet->getModule()->save(outArchive);
	outArchive.save_to("tempPolicyToTarget.pt");
	torch::serialize::InputArchive inArchive = torch::serialize::InputArchive();
	//torch::load(*(m_targetNet->getModule()), "tempPolicyToTarget.pt");
	inArchive.load_from("tempPolicyToTarget.pt");
	m_targetNet->getModule()->load(inArchive);

	m_stepsDone = 0;
	srand(time(NULL));
	torch::optim::RMSprop(m_policyNet->getModule()->parameters(), 0.1);
	m_optimiser = dynamic_cast<torch::optim::Optimizer*>(new torch::optim::RMSprop(m_policyNet->getModule()->parameters(), torch::optim::RMSpropOptions(0.01)));
}

PlayerAction MAIDQNTrainer::selectAction(float state) {
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
		int sample = rand() % replayMemory.states.size();
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
			stateTensor = torch::tensor(replayMemory.states[sample]);
			actionTensor = torch::tensor(replayMemory.actions[sample]);
			rewardTensor = torch::tensor(replayMemory.rewards[sample]);
			nextStateTensor = torch::tensor(replayMemory.nextStates[sample]);
			continue;
		}

		stateTensor = torch::cat({ stateTensor, torch::tensor(replayMemory.states[sample]) }, 0);
		actionTensor = torch::cat({ actionTensor, torch::tensor(replayMemory.actions[sample]) }, 0);
		rewardTensor = torch::cat({ rewardTensor, torch::tensor(replayMemory.rewards[sample]) }, 0);
		nextStateTensor = torch::cat({ nextStateTensor, torch::tensor(replayMemory.nextStates[sample]) }, 0);
	}

	/*torch::Tensor zeros = torch::zeros({ 128, 1 });
	std::cout << zeros << "\n";
	torch::Tensor hej = m_policyNet->forward(zeros);
	std::cout << hej << "\n";*/

	stateTensor = stateTensor.reshape({ 128, 1 });
	//std::cout << stateTensor << "\n";
	actionTensor = torch::_cast_Long(actionTensor);
	actionTensor = actionTensor.reshape({ 128, 1 });
	nextStateTensor = nextStateTensor.reshape({ 128,1 });

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
		float state = srWorld->getDistanceDownTrackForKart(m_policyNet->getKartID(), /*Account for checklines? WTH is this?*/false);
		float startState = state;
		bool raceDone = false;
		while (!raceDone) {
			PlayerAction action = selectAction(state);
			// TODO perform the action
			float nextState = state + 1.0f; // Placeholder
			float reward = 1.0f; // Placeholder
			bool done = (state > startState + 50.0f); // Placeholder // Floating point comparison might be a problem :(
			
			replayMemory.states.push_back(state);
			replayMemory.actions.push_back(action);
			replayMemory.nextStates.push_back(nextState);
			replayMemory.rewards.push_back(reward);

			state = nextState;

			optimiseModel();
			if (done) break;
		}
		if (i % TARGET_UPDATE == 0) {
			// Make targetNet's weights the same as policyNet's
			if (!m_policyNet->getModule()->is_serializable()) throw 909;
			auto compilationUnit = std::make_shared<torch::jit::script::CompilationUnit>();
			torch::serialize::OutputArchive outArchive = torch::serialize::OutputArchive(compilationUnit);
			m_policyNet->getModule()->save(outArchive);
			outArchive.save_to("tempPolicyToTarget.pt");
			torch::serialize::InputArchive inArchive;
			inArchive.load_from("tempPolicyToTarget.pt");
			m_targetNet->getModule()->load(inArchive);
		}
	}
	std::cout << "Running is fun :D\n";
}