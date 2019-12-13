//
// Created by LasseKB on 05/11/2019.
//

// Based on https://radicalrafi.github.io/posts/pytorch-cpp-intro/ and https://github.com/goldsborough/examples/blob/cpp/cpp/mnist/mnist.cpp

#include "mai_dqnmodel.hpp"
#include "modes/standard_race.hpp" // This should probably not be here.
#include "karts/abstract_kart.hpp"
#include "config/user_config.hpp"
#include <tracks/track.hpp>

#define NUM_INPUTS 6

MAIDQNModel::MAIDQNModel()
{
	m_kartID = -1;
	m_actions = {
	        { PlayerAction::PA_ACCEL }, { PlayerAction::PA_BRAKE }, { PlayerAction::PA_STEER_LEFT }, { PlayerAction::PA_STEER_RIGHT },
			{ PlayerAction::PA_ACCEL, PlayerAction::PA_STEER_LEFT }, { PlayerAction::PA_ACCEL, PlayerAction::PA_STEER_RIGHT },
			{ PlayerAction::PA_BRAKE, PlayerAction::PA_STEER_LEFT }, { PlayerAction::PA_BRAKE, PlayerAction::PA_STEER_RIGHT }
	};
	m_module = new torch::nn::Module();

	int inputs = UserConfigParams::m_mai_stack_observations ? NUM_INPUTS * UserConfigParams::m_mai_num_observations : NUM_INPUTS;
	m_inLayer = m_module->register_module("inLayer", torch::nn::Linear(inputs, 128));
	m_hiddenLayerOne = m_module->register_module("hiddenLayerOne", torch::nn::Linear(128, 128));
	m_hiddenLayerTwo = m_module->register_module("hiddenLayerTwo", torch::nn::Linear(128, 128));
	m_outLayer = m_module->register_module("outLayer", torch::nn::Linear(128, /*Number of actions*/m_actions.size()));
	m_kart = nullptr;
	stateHistory = std::vector<StateStruct>();
	//oldestStateHist = 0;
}

//MAIDQNModel::MAIDQNModel(int kartID)
//{
//	m_kartID = kartID;
//	m_actions = { PlayerAction::PA_ACCEL, PlayerAction::PA_BRAKE, PlayerAction::PA_STEER_LEFT, PlayerAction::PA_STEER_RIGHT };
//	m_module = new torch::nn::Module();
//
//	m_inLayer = m_module->register_module("inLayer", torch::nn::Linear(2, 8));
//	m_hiddenLayerOne = m_module->register_module("hiddenLayerOne", torch::nn::Linear(8, 64));
//	m_hiddenLayerTwo = m_module->register_module("hiddenLayerTwo", torch::nn::Linear(64, 64));
//	m_outLayer = m_module->register_module("outLayer", torch::nn::Linear(64, /*Number of actions*/4));
//}

MAIDQNModel::~MAIDQNModel()
{
	m_inLayer.~shared_ptr();
	m_hiddenLayerOne.~shared_ptr();
	m_hiddenLayerTwo.~shared_ptr();
	m_outLayer.~shared_ptr();
}

torch::nn::Module* MAIDQNModel::getModule()
{
	return m_module;
}

std::vector<PlayerAction> MAIDQNModel::getAction()
{
	// Get the distance down the track for our kart
	World *world = World::getWorld();
	if (m_kartID == -1) m_kartID = world->getPlayerKart(0)->getWorldKartId();
	if (m_kart == nullptr) m_kart = world->getPlayerKart(m_kartID);
	StandardRace *srWorld = dynamic_cast<StandardRace*>(world);
	float downTrack = srWorld->getDistanceDownTrackForKart(m_kartID, true);
	float downTrackNoChecklines = srWorld->getDistanceDownTrackForKart(m_kartID, false);
	float toCenter = srWorld->getDistanceToCenterForKart(m_kartID);
	float rotation = m_kart->getHeading();
	float trackLength = Track::getCurrentTrack()->getTrackLength();
	downTrack = downTrack > 0.0f ? downTrack : -(trackLength - downTrackNoChecklines);
	downTrack += trackLength * srWorld->getFinishedLapsOfKart(m_kartID); // TODO: getFinishedLapsOfKart() can return -1. Account for that.
	btVector3 velocity = srWorld->getKart(m_kartID)->getVelocity();
	StateStruct input = { downTrack, toCenter, rotation, velocity.x(), velocity.y(), velocity.z() };

	if (UserConfigParams::m_mai_stack_observations) {
		if (stateHistory.size() < UserConfigParams::m_mai_num_observations) {
			stateHistory.push_back(input);
			return std::vector<PlayerAction>(PA_NITRO);
		}
		stateHistory.erase(stateHistory.begin());
		stateHistory.push_back(input);
		//++oldestStateHist %= UserConfigParams::m_mai_num_observations;
		return m_actions[getActionStacked(stateHistory)];
	}

	return m_actions[getAction(input)];
}

int MAIDQNModel::getAction(StateStruct state)
{
	// Forward the distance through the network
	torch::Tensor x = pseudoForward(state);

	auto theVals = x.accessor<float, 1>();
//	for (int i = 1; i < m_actions.size(); i++) {
//		std::cout << i << ": " << theVals[i] << "\n";
//	}

	//std::cout << x.accessor<float,1>() << "\n";
	return chooseBest(theVals);
}

int MAIDQNModel::getActionStacked(std::vector<StateStruct> states)
{
	torch::Tensor t = torch::cat({ torch::tensor(states[0].downTrack), torch::tensor(states[0].distToMid),
								   torch::tensor(states[0].rotation), torch::tensor(states[0].velX),
								   torch::tensor(states[0].velY), torch::tensor(states[0].velZ) }, 0);
		
	for (int i = 1; i < UserConfigParams::m_mai_num_observations; i++) {
		t = torch::cat({ t, torch::tensor(states[i].downTrack), torch::tensor(states[i].distToMid),
						 torch::tensor(states[i].rotation), torch::tensor(states[i].velX),
						 torch::tensor(states[i].velY), torch::tensor(states[i].velZ) }, 0);
	}

	torch::Tensor x = forward(t, 0);
	auto theVals = x.accessor<float, 1>();
	return chooseBest(theVals);
}

int MAIDQNModel::chooseBest(torch::TensorAccessor<float, 1, torch::DefaultPtrTraits, int64_t> theVals)
{
	// Return an action based on the network output
	int highestVal = 0;

	for (int i = 1; i < m_actions.size(); i++) {
		//auto test = theVals[i];
		//auto test2 = theVals[highestVal];
		if (theVals[i] > theVals[highestVal]) {
			//if (x[0][i].item<float>() < x[0][highestVal].item<float>()) {
			highestVal = i;
		}
	}
	return highestVal;
}

int MAIDQNModel::chooseProbability(torch::TensorAccessor<float, 1, torch::DefaultPtrTraits, int64_t> theVals)
{
	// Return an action based on the network output
	float sample = (rand() % 100) / 100.0f;
	float valSum = 0.0f;

	for (int i = 0; i < m_actions.size(); i++) {
		valSum += theVals[i];
		if (sample < valSum) {
			//if (x[0][i].item<float>() < x[0][highestVal].item<float>()) {
			return i;
		}
	}
}

torch::Tensor MAIDQNModel::forward(torch::Tensor x, int dim) {
	x = torch::relu(m_inLayer->forward(x)); // Maybe not ReLU?
	x = torch::relu(m_hiddenLayerOne->forward(x));
	x = torch::relu(m_hiddenLayerTwo->forward(x));
	x = torch::softmax(m_outLayer->forward(x), /*dim=*/dim);
	return x;
}

int MAIDQNModel::getKartID()
{
	if (m_kartID == -1) {
		World* world = World::getWorld();
		m_kartID = world->getPlayerKart(0)->getWorldKartId();
		if (m_kartID == -1)
			std::cout << "Warning: Getting kart ID before it is initialised!\n";
	}
	return m_kartID;
}

int MAIDQNModel::getNumActions()
{
	return m_actions.size();
}

std::vector<PlayerAction> MAIDQNModel::getAction(int index)
{
	return m_actions[index];
}

torch::Tensor MAIDQNModel::pseudoForward(StateStruct state)
{
	torch::Tensor t = torch::cat({ torch::tensor(state.downTrack), torch::tensor(state.distToMid), 
								   torch::tensor(state.rotation), torch::tensor(state.velX), 
								   torch::tensor(state.velY), torch::tensor(state.velZ) }, 0);

	//std::cout << t << "\n";

	t = torch::relu(m_inLayer->forward(t)); // Maybe not ReLU?
	t = torch::relu(m_hiddenLayerOne->forward(t));
	t = torch::relu(m_hiddenLayerTwo->forward(t));
	return torch::softmax(m_outLayer->forward(t), /*dim=*/0);
}
