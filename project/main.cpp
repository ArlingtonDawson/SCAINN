#include "sc2api/sc2_api.h"
#include "sc2api/sc2_score.h"
#include "sc2utils/sc2_manage_process.h"

#include <iostream>

using namespace sc2;

const char* kReplayFolder = "C:/Projects/SCAINN/project/maps/Replay/";

class ReplayLearning : public sc2::ReplayObserver {
public:
	//Vectors for counting units seen/built/losses/kills
	//the index will be off unittype id.
	//Currently we are instrested in our units built, bad units seen, our units destroyed and bad guys destroyed.
	std::vector<uint32_t> count_units_built_;
	std::vector<uint32_t> count_units_seen_;
	std::vector<uint32_t> count_myunits_destoryed_;
	std::vector<uint32_t> count_otherunits_destoryed_;

	ReplayLearning() :
		sc2::ReplayObserver() {
	}

	void OnGameStart() final {
		const sc2::ObservationInterface* obs = Observation();

		//Lets get the unit type count. We will index the vectors based on unit type id.
		const std::size_t unittypecount = obs->GetUnitTypeData().size();


		//prepare each vector to be sized and filled with 0.
		preparevector(unittypecount, count_units_built_);
		preparevector(unittypecount, count_units_seen_);
		preparevector(unittypecount, count_myunits_destoryed_);
		preparevector(unittypecount, count_otherunits_destoryed_);


	}

	//simple function to resize and fill a vector with 0s.
	void preparevector(std::size_t unittypecount, std::vector<uint32_t>& invector)
	{
		invector.resize(unittypecount);
		std::fill(invector.begin(), invector.end(), 0);
	}

	//simple function to increment an unittype in our vectors.
	void additemtovector(sc2::Unit unit, std::vector<uint32_t>& invector)
	{
		//Check to see if we are with in the bounds of the vector.
		assert(uint32_t(unit.unit_type) < invector.size());
		++invector[unit.unit_type];
	}

	void OnUnitCreated(const sc2::Unit& unit) final {
		additemtovector(unit, count_units_built_);
	}

	void OnUnitEnterVision(const sc2::Unit& unit) final {
		additemtovector(unit, count_units_seen_);
	}

	void OnUnitDestroyed(const sc2::Unit& unit)
	{
		//if the owner is 1 should be the player. More investigation on this
		if (unit.owner == 1)
		{
			additemtovector(unit, count_myunits_destoryed_);
		}
		else
		{
			additemtovector(unit, count_otherunits_destoryed_);
		}

	}


	void OnStep() final {
		uint32_t game_loop = Observation()->GetGameLoop();

		const sc2::ObservationInterface* obs = Observation();
		//Grab our Score Object so that we can parse out some information. 
		const sc2::Score& current_score = obs->GetScore();

		std::cout << "Minerals: " << obs->GetMinerals() << " Gas: " << obs->GetVespene() << std::endl;
		std::cout << "Spent Minerals:" << current_score.score_details.spent_minerals
			<< "Collection Rate:" << current_score.score_details.collection_rate_minerals
			<< "Lost Minerals:" << current_score.score_details.lost_minerals.army + current_score.score_details.lost_minerals.economy + current_score.score_details.lost_minerals.technology + current_score.score_details.lost_minerals.upgrade
			<< "Total Killed Minerals:" << current_score.score_details.killed_minerals.army + current_score.score_details.killed_minerals.economy + current_score.score_details.killed_minerals.technology + current_score.score_details.killed_minerals.upgrade
			<< std::endl;
	}

	void OnGameEnd() final {

		const sc2::ObservationInterface* obs = Observation();
		const sc2::UnitTypes& unit_types = obs->GetUnitTypeData();

		//Display a vectors contents then wait for input.
		displayvector(unit_types, count_units_built_, "Units Created:");
		system("pause");
		displayvector(unit_types, count_units_seen_, "Units Seen:");
		system("pause");
		displayvector(unit_types, count_myunits_destoryed_, "Units Destoried:");
		system("pause");
		displayvector(unit_types, count_otherunits_destoryed_, "Units Killed:");
		system("pause");

		std::cout << "Finished" << std::endl;
	}

	//Displays our vector information to our console screen.
	void displayvector(const sc2::UnitTypes & unit_types, std::vector<uint32_t> invector, std::string label)
	{
		std::cout << label << std::endl;
		for (uint32_t i = 0; i < invector.size(); ++i) {
			//Ignore anything with 0s
			if (invector[i] == 0) {
				continue;
			}

			std::cout << unit_types[i].name << ": " << std::to_string(invector[i]) << std::endl;
		}
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
