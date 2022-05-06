#include <iostream>
#include <set>
#include <vector>
#include <random>

//Needs to be declared because of circular dependency
struct Party;
struct Share;

struct Share{
    int value;
    Party* owner;

    Share(int v, Party* o){
        value = v;
        owner = o;
    }
};

struct Party{
    int input;
    std::set<Share*> shares;

    explicit Party(int input) {
        this->input = input;
    };

    void addShare(Share* share) {
        shares.insert(share);
    }
};

std::vector<Share*> CreateShares(Party* party, int amountOfShares, int fieldSize)
{
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> dist6(1,fieldSize); // distribution in range [1, intmax]

    std::vector<Share*> shares;
    int sum = 0;
    for(int i = 0; i < amountOfShares-1; i++)
    {
        int random = dist6(rng);
        Share* share = new Share(random, party);
        shares.emplace_back(share);
        sum+=random;
    }
    shares.emplace_back(new Share((party->input-sum)%fieldSize, party));
    return shares;
}

void DistributeShares(std::vector<Party>& parties, Party* party, std::set<std::set<Party*>> nonQualifiedSets, std::vector<Share*> shares, std::vector<std::set<Party*>> test)
{

    // for each share (we always have the same amount of shares as non qualified sets of parties)
    for(int share = 0; share < shares.size(); share++)
    {
        //std::set<Party*> shouldNotReceive = *std::next(nonQualifiedSets.begin(), share);
        std::set<Party*> shouldNotReceive = test[share];

        for(Party& p : parties)
        {
            //if party is not in the non qualified set
            if(shouldNotReceive.find(&p) == shouldNotReceive.end())
            {
                p.addShare(shares[share]);
            }
        }
    }
}

void DistributeInput(std::vector<Party>& parties, std::set<std::set<Party*>> nonQualifiedSets, int fieldSize, std::vector<std::set<Party*>> test)
{
    for(int i = 0; i < parties.size(); i++) {

        //Get the shares from party i
        std::vector<Share*> shares = CreateShares(&parties[i], nonQualifiedSets.size(), fieldSize);
        DistributeShares(parties, &parties[i], nonQualifiedSets, shares, test);
    }
}
//
std::set<std::set<Party*>> x_out_of_y_setCombinations(std::vector<Party>& parties, int x, int y)
{
    std::set<std::set<Party*>> combinations = std::set<std::set<Party*>>();
    std::string bitmask(x, 1); // K leading 1's
    bitmask.resize(y, 0); // N-K trailing 0's

    do {
        std::set<Party*> combination;
        for (int i = 0; i < y; ++i) // [0..N-1] integers
        {
            if (bitmask[i]) combination.insert(&parties[i]);
        }
        combinations.insert(combination);
    } while (std::prev_permutation(bitmask.begin(), bitmask.end()));
    return combinations;
}

//A very basic threshold access structure
std::set<std::set<Party*>> FindNonQualifiedSets(std::vector<Party>& parties, int amountToReconstruct) {

    std::set<std::set<Party*>> NonQualifiedSets = std::set<std::set<Party*>>();
    for(int i = 1; i < amountToReconstruct; i++)
    {
        std::set<std::set<Party*>> combinations = x_out_of_y_setCombinations(parties, i, parties.size());
        NonQualifiedSets.insert(combinations.begin(), combinations.end());
    }
    return NonQualifiedSets;
}

//void CombineResults(std::vector<Party>& parties, int x, int fieldSize) {
//    int result = 0;
//    for(int i = 0; i < parties.size(); i++) {
//        result += std::get<1>(parties[i].GetShares()[x]);
//    }
//    std::cout << "Result: " << result % fieldSize << std::endl;
//}
//
//void GetResults(std::vector<Party>& parties, int fieldSize) {
//    for (int i = 0; i < parties.size(); i++) {
//        std::cout << "party secret: " << std::endl;
//        std::cout << parties[i].GetInput() << std::endl;
//        CombineResults(parties, i, fieldSize);
//    }
//}

std::vector<Party*> GetRandomPartiesToReconstruct(std::vector<Party>& parties, int amountToReconstruct)
{
    std::set<Party*> randomPartiesSet;

    while (randomPartiesSet.size() < amountToReconstruct) {
        int randomParty = rand() % parties.size();
        auto p = &parties[randomParty];
        randomPartiesSet.insert(p);
    }

    std::vector<Party*> randomParties;
    randomParties.assign(randomPartiesSet.begin(), randomPartiesSet.end());
    return randomParties;
}

//void Reconstruct(std::vector<Party>& parties, int amountToReconstruct, int amountOfShares)
//{
//    //Gets random parties that can reconstruct
//    std::vector<Party> ableParties = GetPartiesAbleToReconstruct(parties, amountToReconstruct);
//
//    //The shares not yet assigned to a party
//
//    //Create a list of
//    //index 0: set of all the remaining shares from party 0
//    //index 1: set of all the remaining shares from party 1
//    //etc
//
//    std::vector<std::set<int>> remainingShares;
//    for(int i = 0; i < parties.size(); i++)
//    {
//        std::set<int> shares;
//        for(int j = 0; j < amountOfShares; j++)
//        {
//            shares.insert(j);
//        }
//        remainingShares.push_back(shares);
//    }
//
//    //Since we are using replicated secret sharing, some shares are obtained by multiple parties,
//    //and we need to figure out which party is responsible for what shares
//
//    //Find the intersection of all the shares between parties participating in the reconstruction
//    //that is, all the shares they have in common
//
//    //Now decide what party takes what common shares
//
//    //Now for each party, find their respective shares not in this common set, and let them compute on these
//    //individually aswell as the common shares assigned to them from previous operation
//
//    /*
//
//    std::vector<std::tuple<int,std::tuple<int,int>>> responsible;
//
//    for(int i = 0; i < ableParties.size(); i++) {
//        auto partyIShares = parties[ableParties[i]].GetShares();
//        for(int j = 0; j < partyIShares.size(); j++){
//            auto x = partyIShares[j];
//            auto y = get<0>(x);
//            if(remainingShares[get<0>(y)].find(get<1>(y)) != remainingShares[get<0>(y)].end()) {
//                responsible.emplace_back(std::make_tuple(ableParties[i], std::make_tuple(get<0>(y), get<1>(y))));
//                remainingShares[get<0>(y)].erase(get<1>(y));
//            }
//        }
//    }
//     */
//
//    auto s = 1;
//}

std::vector<Party> CreateParties(int honestParties)
{
    std::vector<Party> parties;
    for(int i = 0; i < honestParties; i++) {
        //Should be a random input here instead
        Party party(100);
        parties.push_back(party);
    }
    return parties;
}


int main() {

    //==================================== PHASE 1 =====================================================================
    //Creating parties and distributing their shares among the other parties participating in the protocol

    //Define the field size as a large prime
    int fieldSize = 2147483647;
    //Amount of parties participating in the protocol
    int amountOfParties = 15;
    //Amount of passive adversaries participating in the protocol
    //int amountOfPassiveAdversaries = 0;
    //Amount of parties with an input
    //int partiesWithInput = 1;
    //How many parties are required to reconstruct the secret
    int amountToReconstruct = 11;

    //Create the parties participating in the protocol
    std::vector<Party> parties = CreateParties(amountOfParties);

    //Define the Non qualified sets (secrecy structure) for share distribution (a basic threshold access structure)
    std::set<std::set<Party*>> nonQualifiedSets = FindNonQualifiedSets(parties, amountToReconstruct);

    //Transform set of nonQualified to vector for indexes to use in distributing shares. This makes lookup time
    //O(1) instead of O(n), which dramatically increases performance (test with 15 parties made a 10x difference)
    std::vector <std::set<Party*>> nonQualifiedSetsIndexed;
    nonQualifiedSetsIndexed.reserve (nonQualifiedSets.size ());
    std::copy (nonQualifiedSets.begin (), nonQualifiedSets.end (), std::back_inserter (nonQualifiedSetsIndexed));

    DistributeInput(parties, nonQualifiedSets, fieldSize, nonQualifiedSetsIndexed);


    //==================================== PHASE 2 =====================================================================
    //At this point all the inputs have been split into shares and distributed. We can now begin computing a function
    //on these shares. Depending on the input and function to compute, we have to do some different things

    //Common for all is however some way of determining which party uses which shares, as they are replicated among many
    //parties. That is, if we want to ex. compute x1+y1, we have multiple parties who can do this, but only one party
    //has to do it, otherwise the result will be inaccurate.

    //in case of simple addition, we can just add the shares together locally for each party, and send the result to the
    //other parties. They can then add their own intermediate result with the results they receive from the other parties

    //In case of multiplication, we have to do some more work --- tbc

    //For this purpose, we find x random parties to reconstruct
    std::vector<Party*> partiesToReconstruct = GetRandomPartiesToReconstruct(parties, amountToReconstruct);
    //Now find all the shares all parties have in common

    //ReconstructAddition(parties);






    //Reconstruct(parties, amountToReconstruct, nonQualifiedSets.size());
    return 0;
}