//
// Created by LasseKB on 07/11/2019.
//

// Based on https://radicalrafi.github.io/posts/pytorch-cpp-intro/, https://github.com/goldsborough/examples/blob/cpp/cpp/mnist/mnist.cpp, 
// and https://pytorch.org/tutorials/intermediate/reinforcement_q_learning.html

#include "mai_dqntrainer.hpp"
#include "modes/standard_race.hpp" // This should probably not be here.
#include <math.h>
#include <sys/stat.h>
#include <tracks/track.hpp>
#include <config/user_config.hpp>

#define BATCH_SIZE 128
#define GAMMA 0.999
#define EPS_START 1.0
#define EPS_END 0.05
#define EPS_DECAY 100000
#define TARGET_UPDATE 10
#define SAVE_MODEL 500

#define RESETRACE true
#define REALDATA true

const std::string modelName = "test.pt";

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

	m_targetNet = new MAIDQNModel(/*model->getKartID()*/);

	saveToTargetModel();

	m_stepsDone = 0;
	srand(time(NULL));
	torch::optim::RMSprop(m_policyNet->getModule()->parameters(), 0.1);
	m_optimiser = dynamic_cast<torch::optim::Optimizer*>(new torch::optim::RMSprop(m_policyNet->getModule()->parameters(), torch::optim::RMSpropOptions(0.01)));
	m_lastActionIndex = -1;
	m_runOnceIteration = 0;

	if(!REALDATA)
	    addFakeReplayData();
}

int MAIDQNTrainer::selectAction(StateStruct state) {
	float sample = (rand() % 100) / 100.0f;
	const float pre_comp = EPS_END + (EPS_START - EPS_END);
	float eps_threshold = pre_comp * pow(M_E, -1. * m_stepsDone / EPS_DECAY);
	m_stepsDone += 1;
	if (sample > eps_threshold)
		return m_policyNet->getAction(state);
	else 
	{
		return rand() % m_policyNet->getNumActions();
		//return m_policyNet->getAction(ind);
	}
}

void MAIDQNTrainer::optimiseModel() {
	if (replayMemory.states.size() < BATCH_SIZE) return;

	torch::Tensor stateTensor;/*= torch::zeros({128, 1})*/;
	torch::Tensor actionTensor;
	torch::Tensor rewardTensor;
	torch::Tensor nextStateTensor;

	bool firstElement = true;

	for (int i = 0; i < BATCH_SIZE; i++) {
		//std::cout << i << "\n";
		int sample = rand() % replayMemory.actionIndices.size();
		if (sample < UserConfigParams::m_mai_num_observations)
			sample = UserConfigParams::m_mai_num_observations;
		int sampleCopy = sample;

		if (UserConfigParams::m_mai_stack_observations) {
			sample -= (UserConfigParams::m_mai_num_observations - 1);
		}

		if (firstElement) {
			stateTensor = torch::cat({ torch::tensor(replayMemory.states[sample].downTrack), torch::tensor(replayMemory.states[sample].distToMid),
										torch::tensor(replayMemory.states[sample].rotation), torch::tensor(replayMemory.states[sample].velX),
										torch::tensor(replayMemory.states[sample].velY), torch::tensor(replayMemory.states[sample].velZ) }, 0);

			nextStateTensor = torch::cat({ torch::tensor(replayMemory.nextStates[sample].downTrack), torch::tensor(replayMemory.nextStates[sample].distToMid),
											torch::tensor(replayMemory.nextStates[sample].rotation), torch::tensor(replayMemory.nextStates[sample].velX),
											torch::tensor(replayMemory.nextStates[sample].velY), torch::tensor(replayMemory.nextStates[sample].velZ) });
			actionTensor = torch::tensor(replayMemory.actionIndices[sampleCopy]);
			rewardTensor = torch::tensor(replayMemory.rewards[sampleCopy]);
			firstElement = false;
			sample++;
		} else {
			actionTensor = torch::cat({ actionTensor, torch::tensor(replayMemory.actionIndices[sampleCopy]) }, 0);
			rewardTensor = torch::cat({ rewardTensor, torch::tensor(replayMemory.rewards[sampleCopy]) }, 0);
		}
		while (sample <= sampleCopy) {
			stateTensor = torch::cat({ stateTensor, torch::tensor(replayMemory.states[sample].downTrack), torch::tensor(replayMemory.states[sample].distToMid),
										torch::tensor(replayMemory.states[sample].rotation), torch::tensor(replayMemory.states[sample].velX),
										torch::tensor(replayMemory.states[sample].velY), torch::tensor(replayMemory.states[sample].velZ) }, 0);

			nextStateTensor = torch::cat({ nextStateTensor, torch::tensor(replayMemory.nextStates[sample].downTrack), torch::tensor(replayMemory.nextStates[sample].distToMid),
											torch::tensor(replayMemory.nextStates[sample].rotation), torch::tensor(replayMemory.nextStates[sample].velX),
											torch::tensor(replayMemory.nextStates[sample].velY), torch::tensor(replayMemory.nextStates[sample].velZ) }, 0);
			sample++;
		}
	}

	int inputs = UserConfigParams::m_mai_stack_observations ? 6 * UserConfigParams::m_mai_num_observations : 6;
	stateTensor = stateTensor.reshape({ 128, inputs });
	//std::cout << stateTensor << "\n";
	actionTensor = torch::_cast_Long(actionTensor);
	actionTensor = actionTensor.reshape({ 128, 1 });
	//std::cout << actionTensor << "\n";
	nextStateTensor = nextStateTensor.reshape({ 128, inputs });
	//std::cout << m_policyNet->forward(stateTensor) << "\n";

	torch::Tensor stateActionValues = m_policyNet->forward(stateTensor, 1).gather(1, actionTensor);
	torch::Tensor nextStateValues = std::get<0>(m_targetNet->forward(nextStateTensor, 1).max(1)).detach();
	torch::Tensor expectedStateValues = (nextStateValues * GAMMA) + rewardTensor;

	auto loss = torch::smooth_l1_loss(stateActionValues, expectedStateValues.unsqueeze(1));
	
	/*std::ofstream stream;
	stream.open("loss.txt");
	stream << m_stepsDone << "," << loss.accessor<float, 1>()[0] << "\n";
	stream.close();
	std::cout << "Loss:\n" << loss << "\n";*/

	//std::cout << m_policyNet->forward(stateTensor) << "\n";
	m_optimiser->zero_grad();
	loss.backward();
	m_optimiser->step();
	//std::cout << "---\n" << m_policyNet->forward(stateTensor) << "\n";
}

void MAIDQNTrainer::run() {
	//for (int i = 0; i < 50; i++) {
	//	// TODO reset world
	//	World *world = World::getWorld();
	//	StandardRace* srWorld = dynamic_cast<StandardRace*>(world);
	//	float state[2];
	//	state[0] = srWorld->getDistanceDownTrackForKart(m_policyNet->getKartID(),true);
	//	state[1] = 0.0f; // Placeholder
	//	float startState = state[0];
	//	bool raceDone = false;
	//	while (!raceDone) {
	//		int actionIndex = selectAction(state);
	//		// TODO perform the action
	//		float nextState[2];
	//		nextState[0] = state[0] + 1.0f; // Placeholder
	//		nextState[1] = state[1]; // Placeholder
	//		float reward = 1.0f; // Placeholder
	//		bool done = (state[0] > startState + 50.0f); // Placeholder // Floating point comparison might be a problem :(
	//		
	//		/*replayMemory.states.push_back(state[0]);
	//		replayMemory.states.push_back(state[1]);
	//		replayMemory.actionIndices.push_back(actionIndex);
	//		replayMemory.nextStates.push_back(nextState[0]);
	//		replayMemory.nextStates.push_back(nextState[1]);
	//		replayMemory.rewards.push_back(reward);*/

	//		state[0] = nextState[0];
	//		state[1] = nextState[1];

	//		optimiseModel();
	//		if (done) break;
	//	}
	//	if (i % TARGET_UPDATE == 0) {
	//		saveToTargetModel();
	//	}
	//}
	//std::cout << "Running is fun :D\n";
}

std::vector<PlayerAction> MAIDQNTrainer::runOnce() {
	World* world = World::getWorld();
	if (world->getPhase() != world->RACE_PHASE && world->getPhase() != world->GO_PHASE) return { PA_NITRO };
	StandardRace* srWorld = dynamic_cast<StandardRace*>(world);
	if(RESETRACE){
        if (m_runOnceIteration % 7200 == 0 && m_runOnceIteration != 0) {
            m_runOnceIteration++;
            std::cout << "\nRestarting race\n";
            srWorld->reset(true);
            return m_policyNet->getAction(0);
        }
        if (m_runOnceIteration >= 240000) {
            srWorld->scheduleExitRace();
            std::cout << "\nExiting race.\n";
        }
	}

	int kartID = m_policyNet->getKartID();
	float downTrack = srWorld->getDistanceDownTrackForKart(kartID, true);
	float downTrackNoChecklines = srWorld->getDistanceDownTrackForKart(kartID, false);
	float distToMid = srWorld->getDistanceToCenterForKart(kartID);
	float turn = srWorld->getKart(kartID)->getHeading();
	float trackLength = Track::getCurrentTrack()->getTrackLength();

	downTrack = downTrack > 0.0f ? downTrack : -(trackLength - downTrackNoChecklines);
	int finishedLaps = srWorld->getFinishedLapsOfKart(kartID);
	if (finishedLaps < 0) finishedLaps = 0;
	downTrack += trackLength * finishedLaps;

	btVector3 velocity = srWorld->getKart(kartID)->getVelocity();

	StateStruct state = { downTrack, distToMid, turn, velocity.x(), velocity.y(), velocity.z() };

	int actionInd = selectAction(state);
	
	if (m_runOnceIteration == 0) {
		m_lastState = state;
		m_lastActionIndex = actionInd;
		m_runOnceIteration++;
		return m_policyNet->getAction(actionInd);
	}

	if(REALDATA){
        replayMemory.states.push_back(m_lastState);
        replayMemory.actionIndices.push_back(m_lastActionIndex);
		replayMemory.nextStates.push_back(state);
        replayMemory.rewards.push_back(state.downTrack - m_lastState.downTrack);
        //std::cout << replayMemory.rewards.back();
	}

	//std::cout << "Experienced reward of " << state[0] << " - " << m_lastState[0] << " = " << state[0] - m_lastState[0] << "\n";

	m_lastState = state;
	std::vector<PlayerAction> tmpAction = m_policyNet->getAction(m_lastActionIndex);
	m_lastActionIndex = actionInd;

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

void MAIDQNTrainer::addFakeReplayData() {
	for (int i = 0; i < BATCH_SIZE * 2; i++) {
		float randomDownTrack = rand() % 1170;
		float randomToCenter = (rand() % 20) - 10;
		replayMemory.states.push_back({ randomDownTrack, randomToCenter, 0.0f });
		replayMemory.nextStates.push_back({ randomDownTrack + 1, randomToCenter, 0.0f });
		if (i % 2 == 0) {
			replayMemory.actionIndices.push_back(1);
			replayMemory.rewards.push_back(10);
		}
		else {
			int randAction;
			do {
				randAction = rand() % m_policyNet->getNumActions();
			} while (randAction == 1);
			replayMemory.actionIndices.push_back(randAction);
			replayMemory.rewards.push_back(0);
		}
	}
}