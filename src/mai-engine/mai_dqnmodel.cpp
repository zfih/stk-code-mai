//
// Created by LasseKB on 05/11/2019.
//

// Based on https://radicalrafi.github.io/posts/pytorch-cpp-intro/ and https://github.com/goldsborough/examples/blob/cpp/cpp/mnist/mnist.cpp

#include "mai_dqnmodel.hpp"
#include "modes/standard_race.hpp" // This should probably not be here.
#include "karts/abstract_kart.hpp"

MAIDQNModel::MAIDQNModel()
{
	m_kartID = -1;
	m_actions = { PlayerAction::PA_ACCEL, PlayerAction::PA_BRAKE, PlayerAction::PA_STEER_LEFT, PlayerAction::PA_STEER_RIGHT };
	m_module = new torch::nn::Module();

	m_inLayer = m_module->register_module("inLayer", torch::nn::Linear(2, 8));
	m_hiddenLayerOne = m_module->register_module("hiddenLayerOne", torch::nn::Linear(8, 64));
	m_hiddenLayerTwo = m_module->register_module("hiddenLayerTwo", torch::nn::Linear(64, 64));
	m_outLayer = m_module->register_module("outLayer", torch::nn::Linear(64, /*Number of actions*/4));
}

MAIDQNModel::MAIDQNModel(int kartID)
{
	m_kartID = kartID;
	m_actions = { PlayerAction::PA_ACCEL, PlayerAction::PA_BRAKE, PlayerAction::PA_STEER_LEFT, PlayerAction::PA_STEER_RIGHT };
	m_module = new torch::nn::Module();

	m_inLayer = m_module->register_module("inLayer", torch::nn::Linear(2, 8));
	m_hiddenLayerOne = m_module->register_module("hiddenLayerOne", torch::nn::Linear(8, 64));
	m_hiddenLayerTwo = m_module->register_module("hiddenLayerTwo", torch::nn::Linear(64, 64));
	m_outLayer = m_module->register_module("outLayer", torch::nn::Linear(64, /*Number of actions*/4));
}

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

PlayerAction MAIDQNModel::getAction()
{
	// Get the distance down the track for our kart
	World *world = World::getWorld();
	if (m_kartID == -1) m_kartID = world->getPlayerKart(0)->getWorldKartId();
	StandardRace *srWorld = dynamic_cast<StandardRace*>(world);
	float downTrack = srWorld->getDistanceDownTrackForKart(m_kartID, /*Account for checklines? WTH is this?*/false);
	float toCenter = srWorld->getDistanceToCenterForKart(m_kartID);
	float input[]{ downTrack, toCenter };

	return getAction(input);
}

PlayerAction MAIDQNModel::getAction(float state[])
{
	// Forward the distance through the network
	torch::Tensor x = pseudoForward(state);

	//std::cout << x.accessor<float,1>() << "\n";
	auto theVals = x.accessor<float, 1>();

	// Return an action based on the network output
	int highestVal = 0;

	for (int i = 1; i < 4; i++) {
		auto test = theVals[i];
		auto test2 = theVals[highestVal];
		if (theVals[i] > theVals[highestVal]) {
		//if (x[0][i].item<float>() < x[0][highestVal].item<float>()) {
			highestVal = i;
		}
	}
	return m_actions[highestVal];
}

torch::Tensor MAIDQNModel::forward(torch::Tensor x) {
	x = torch::relu(m_inLayer->forward(x)); // Maybe not ReLU?
	x = torch::relu(m_hiddenLayerOne->forward(x));
	x = torch::relu(m_hiddenLayerTwo->forward(x));
	return torch::softmax(m_outLayer->forward(x), /*dim=*/1);
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

PlayerAction MAIDQNModel::getAction(int index)
{
	return m_actions[index];
}

torch::Tensor MAIDQNModel::pseudoForward(float state[])
{
	torch::Tensor t = torch::cat({ torch::tensor(state[0]), torch::tensor(state[1]) }, 0);

	//std::cout << t << "\n";

	t = torch::relu(m_inLayer->forward(t)); // Maybe not ReLU?
	t = torch::relu(m_hiddenLayerOne->forward(t));
	t = torch::relu(m_hiddenLayerTwo->forward(t));
	return torch::softmax(m_outLayer->forward(t), /*dim=*/0);
}
