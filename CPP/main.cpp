#include <iostream>
#include "Party.cpp"
#include<set>


void DistributeInput(std::vector<Party>& parties, std::vector<std::set<int>> nonQualifiedSets) {
    for(int i = 0; i < parties.size(); i++) {
        parties[i].DistributeInput(parties, nonQualifiedSets);
    }
}

std::vector<std::set<int>> GetPossibleCombinationsFromXOutOfY(int x, int y)
{
    std::vector<std::set<int>> combinations = std::vector<std::set<int>>();
    std::string bitmask(x, 1); // K leading 1's
    bitmask.resize(y, 0); // N-K trailing 0's

    do {
        std::set<int> combination;
        for (int i = 0; i < y; ++i) // [0..N-1] integers
        {
            if (bitmask[i]) combination.insert(i);
        }
        combinations.emplace_back(combination);
    } while (std::prev_permutation(bitmask.begin(), bitmask.end()));
    return combinations;
}

std::vector<std::set<int>> DefineSets(std::vector<Party>& parties, int amountToReconstruct) {

    //TODO: THIS NEEDS SOME OPTIMIZATION WORK, VERY SLOW

    //We exclude the empty set, as it would just add another share that every party receives

    //std::vector<std::set<int>> allCombinations = std::vector<std::set<int>>();
    //for(int i = 1; i <= parties.size(); i++) {
    //    auto combinations = GetPossibleCombinationsFromXOutOfY(i, parties.size());
    //    allCombinations.insert( allCombinations.end(), combinations.begin(), combinations.end() );
    //}

    //std::vector<std::set<int>> QualifiedSets = std::vector<std::set<int>>();
    //for(int i = amountToReconstruct; i <= parties.size(); i++)
    //{
    //    auto combinations = GetPossibleCombinationsFromXOutOfY(i, parties.size());
    //    QualifiedSets.insert( QualifiedSets.end(), combinations.begin(), combinations.end() );
    //}

    std::vector<std::set<int>> NonQualifiedSets = std::vector<std::set<int>>();
    for(int i = 1; i < amountToReconstruct; i++)
    {
        auto combinations = GetPossibleCombinationsFromXOutOfY(i, parties.size());
        NonQualifiedSets.insert(NonQualifiedSets.end(), combinations.begin(), combinations.end() );
    }
    return NonQualifiedSets;
}

void CombineResults(std::vector<Party>& parties, int x, int fieldSize) {
    int result = 0;
    for(int i = 0; i < parties.size(); i++) {
        result += std::get<1>(parties[i].GetShares()[x]);
    }
    std::cout << "Result: " << result % fieldSize << std::endl;
}

void GetResults(std::vector<Party>& parties, int fieldSize) {
    for (int i = 0; i < parties.size(); i++) {
        std::cout << "party secret: " << std::endl;
        std::cout << parties[i].GetInput() << std::endl;
        CombineResults(parties, i, fieldSize);
    }
}

std::vector<int> GetPartiesAbleToReconstruct(int amountOfParties, int amountToReconstruct)
{
    std::set<int> Numbers;
    while (Numbers.size() < amountToReconstruct) {
        Numbers.insert(rand() % amountOfParties);
    }

    std::vector<int> parties;
    parties.assign(Numbers.begin(), Numbers.end());
    return parties;
}

void Reconstruct(std::vector<Party>& parties, int amountToReconstruct, int amountOfShares)
{
    //Gets random parties that can reconstruct
    auto ableParties = GetPartiesAbleToReconstruct(parties.size(), amountToReconstruct);

    std::vector<std::set<int>> remainingShares;
    for(int i = 0; i < parties.size(); i++)
    {
        std::set<int> shares;
        for(int j = 0; j < amountOfShares; j++)
        {
            shares.insert(j);
        }
        remainingShares.push_back(shares);
    }

    std::vector<std::tuple<int,std::tuple<int,int>>> responsible;

    for(int i = 0; i < ableParties.size(); i++) {
        auto partyIShares = parties[ableParties[i]].GetShares();
        for(int j = 0; j < partyIShares.size(); j++){
            auto x = partyIShares[j];
            auto y = get<0>(x);
            if(remainingShares[get<0>(y)].find(get<1>(y)) != remainingShares[get<0>(y)].end()) {
                responsible.emplace_back(std::make_tuple(ableParties[i], std::make_tuple(get<0>(y), get<1>(y))));
                remainingShares[get<0>(y)].erase(get<1>(y));
            }
        }
    }

    auto s = 1;


    //std::set<std::tuple<std::tuple<int,int>, int>> shares;
    //
    //for(int i = 0; i < ableParties.size(); i++)
    //{
    //    for(int j = 0; j < parties[ableParties[i]].GetShares().size(); j++)
    //    {
    //        shares.insert(parties[ableParties[i]].GetShares()[j]);
    //    }
    //}

    //std::vector<int> reconstructedShares;
}

std::vector<Party> CreateParties(int honestParties, int dishonestPassiveParties, int dishonestActiveParties, int fieldSize)
{
    std::vector<Party> parties;
    for(int i = 0; i < honestParties; i++) {
        Party party(fieldSize, i);
        parties.push_back(party);
    }
    return parties;
}

int main() {
    int fieldSize = 100003;
    int amountOfParties = 3;
    int amountToReconstruct = 2;
    std::vector<Party> parties = CreateParties(amountOfParties, 0, 0, fieldSize);
    //returns all non qualified sets
    auto nonQualified = DefineSets(parties, amountToReconstruct);
    DistributeInput(parties, nonQualified);
    Reconstruct(parties, amountToReconstruct, nonQualified.size());
    //GetResults(parties, fieldSize);
    return 0;
}