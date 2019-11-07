#include "mai_controller.hpp"

MAIController::MAIController(AbstractKart *kart) : PlayerController(kart) {
	m_mai_engine = MAIEngine::getMAIEngine();
}
MAIController::~MAIController()
= default;

void MAIController::update(int ticks) {

}