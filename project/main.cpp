#include "sc2api/sc2_api.h"
#include "sc2api/sc2_score.h"
#include "sc2utils/sc2_manage_process.h"

#include <iostream>

using namespace sc2;

const char* kReplayFolder = "C:/Projects/SCAINN/project/maps/Replay/";

class ReplayLearning : public sc2::ReplayObserver {
public:
	std::vector<uint32_t> count_units_built_;

	ReplayLearning() :
		sc2::ReplayObserver() {
	}

	void OnGameStart() final {
		const sc2::ObservationInterface* obs = Observation();
		assert(obs->GetUnitTypeData().size() > 0);
		count_units_built_.resize(obs->GetUnitTypeData().size());
		std::fill(count_units_built_.begin(), count_units_built_.end(), 0);
	}

	void OnUnitCreated(const sc2::Unit& unit) final {
	}

	void OnStep() final {
		uint32_t game_loop = Observation()->GetGameLoop();

		const sc2::ObservationInterface* obs = Observation();
		const sc2::Score& current_score = obs->GetScore();

		std::cout << "Minerals: " << obs->GetMinerals() << " Gas: " << obs->GetVespene() << std::endl;
		std::cout << "Spent Minerals:" << current_score.score_details.spent_minerals
			<< "Collection Rate:" << current_score.score_details.collection_rate_minerals
			<< "Lost Minerals:" << current_score.score_details.lost_minerals.army + current_score.score_details.lost_minerals.economy + current_score.score_details.lost_minerals.technology + current_score.score_details.lost_minerals.upgrade
			<< "Total Killed Minerals:" << current_score.score_details.killed_minerals.army + current_score.score_details.killed_minerals.economy + current_score.score_details.killed_minerals.technology + current_score.score_details.killed_minerals.upgrade
			<< std::endl;
	}

	void OnGameEnd() final {
	}
};


int main(int argc, char* argv[]) {
	sc2::Coordinator coordinator;
	if (!coordinator.LoadSettings(argc, argv)) {
		return 1;
	}

	if (!coordinator.SetReplayPath(kReplayFolder)) {
		std::cout << "Unable to find replays." << std::endl;
		return 1;
	}

	ReplayLearning replay_observer;

	coordinator.AddReplayObserver(&replay_observer);

	while (coordinator.Update());
	while (!sc2::PollKeyPress());
}