//
// Created by LasseKB on 05/11/2019.
//

// Based on https://radicalrafi.github.io/posts/pytorch-cpp-intro/ and https://github.com/goldsborough/examples/blob/cpp/cpp/mnist/mnist.cpp

#include "mai_dqnmodel.hpp"
#include "modes/standard_race.hpp" // This should probably not be here.

MAIDQNModel::MAIDQNModel(int kartID)
{
	m_kartID = kartID;
	m_actions = { PlayerAction::PA_ACCEL, PlayerAction::PA_BRAKE, PlayerAction::PA_STEER_LEFT, PlayerAction::PA_STEER_RIGHT };

	m_inLayer = register_module("inLayer", torch::nn::Linear(1, 8));
	m_hiddenLayerOne = register_module("hiddenLayerOne", torch::nn::Linear(8, 64));
	m_hiddenLayerTwo = register_module("hiddenLayerTwo", torch::nn::Linear(64, 64));
	m_outLayer = register_module("outLayer", torch::nn::Linear(64, /*Number of actions*/4));
}

MAIDQNModel::~MAIDQNModel()
{
	m_inLayer.~shared_ptr();
	m_hiddenLayerOne.~shared_ptr();
	m_hiddenLayerTwo.~shared_ptr();
	m_outLayer.~shared_ptr();
}

PlayerAction MAIDQNModel::getAction()
{
	// Get the distance down the track for our kart
	World *world = World::getWorld();
	StandardRace *srWorld = dynamic_cast<StandardRace*>(world);
	float input = srWorld->getDistanceDownTrackForKart(m_kartID, /*Account for checklines? WTH is this?*/false);
	
	getAction(input);
}

PlayerAction MAIDQNModel::getAction(float distanceDownTrack)
{
	torch::Tensor x = torch::full((1, 1), distanceDownTrack);

	// Forward the distance through the network
	x = forward(x);

	// Return an action based on the network output
	float highestVal = 0;
	for (int i = 1; i < 4; i++) {
		if (x.item<float>[0][i] < x.item<float>[0][highestVal]) {
			highestVal = i;
		}
	}
	return m_actions[highestVal];
}

//torch::Tensor MAIDQNModel::forward(torch::Tensor x) {
//	x = torch::relu(m_inLayer->forward(x)); // Maybe not ReLU?
//	x = torch::relu(m_hiddenLayerOne->forward(x));
//	x = torch::relu(m_hiddenLayerTwo->forward(x));
//	return torch::softmax(m_outLayer->forward(x), /*dim=*/1);
//}

int MAIDQNModel::getKartID()
{
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

torch::Tensor MAIDQNModel::pseudoForward(float x)
{
	torch::Tensor t = torch::full((1, 1), x);

	t = torch::relu(m_inLayer->forward(t)); // Maybe not ReLU?
	t = torch::relu(m_hiddenLayerOne->forward(t));
	t = torch::relu(m_hiddenLayerTwo->forward(t));
	return torch::softmax(m_outLayer->forward(t), /*dim=*/1);
}
