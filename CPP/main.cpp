#include <iostream>
#include <set>
#include <vector>
#include <random>
#include <map>

//Needs to be declared because of circular dependency
struct Party;
struct Share;

struct Share{
    int id;
    int value;
    Party* owner;
    int batch_id;

    Share(int v, Party* o, int batch_id, int id){
        value = v;
        owner = o;
        this->batch_id = batch_id;
        this->id = id;
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

std::vector<Share*> CreateShares(Party* party, int amountOfShares, int fieldSize, int batch_id)
{
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> dist6(1,fieldSize); // distribution in range [1, intmax]

    std::vector<Share*> shares;
    int sum = 0;
    for(int i = 0; i < amountOfShares-1; i++)
    {
        int random = dist6(rng);
        Share* share = new Share(random, party, batch_id, i);
        shares.emplace_back(share);
        sum+=random;
    }
    shares.emplace_back(new Share((party->input-sum), party, batch_id, amountOfShares-1));
    return shares;
}

void DistributeShares(std::vector<Party>& parties, Party* party, std::set<std::set<Party*>> nonQualifiedSets, std::vector<Share*> shares, std::vector<std::set<Party*>> test)
{

    // for each share (we always have the same amount of shares as non qualified sets of parties)
    for(int share = 0; share < shares.size(); share++)
    {
        //std::set<Party*> shouldNotReceive = *std::next(nonQualifiedSets.begin(), share);
        std::set<Party*> shouldNotReceive;
        if(test.size() != 0) shouldNotReceive = test[share];

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

void DistributeInput(std::vector<Party>& parties, std::set<std::set<Party*>> nonQualifiedSets, int fieldSize, std::vector<std::set<Party*>> test, int& batch_id)
{
    for(int i = 0; i < parties.size(); i++) {

        //Get the shares from party i
        std::vector<Share*> shares = CreateShares(&parties[i], nonQualifiedSets.size(), fieldSize, batch_id);
        DistributeShares(parties, &parties[i], nonQualifiedSets, shares, test);
        batch_id++;
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

int Multiplication(std::vector<Party>& parties, int fieldSize, int batch_id_x, int batch_id_y, int amountOfShares)
{
    //ex multiplying x and y
    //split into 3 shares
    //x = x1 + x2 + x3
    //y = y1 + y2 + y3
    //x*y = (x1+x2+x3)*(y1+y2+y3)
    //=> x*y = x1*y1 + x1*y2 + x1*y3 + x2*y1 + x2*y2 + x2*y3 + x3*y1 + x3*y2 + x3*y3

    //(x1*y1) = 0,0,1,0
    //(x1*y2) = 0,0,1,1
    //(x1*y3) = 0,0,1,2
    //(x2*y1) = 0,1,1,0
    //(x2*y2) = 0,1,1,1
    //(x2*y3) = 0,1,1,2
    //(x3*y1) = 0,2,1,0
    //(x3*y2) = 0,2,1,1
    //(x3*y3) = 0,2,1,2
    struct RequiredShare {
        int batch_id_x;
        int id_x;
        int batch_id_y;
        int id_y;

        RequiredShare(int batch_id_x, int id_x, int batch_id_y, int id_y) {
            this->batch_id_x = batch_id_x;
            this->id_x = id_x;
            this->batch_id_y = batch_id_y;
            this->id_y = id_y;
        }
    };

    //create all the required shares
    std::vector<RequiredShare*> requiredShares;

    for(int i = 0; i < amountOfShares; i++) {
        for(int j = 0; j < amountOfShares; j++)
        {
            RequiredShare* r = new RequiredShare(batch_id_x, i, batch_id_y, j);
            requiredShares.emplace_back(r);
        }
    }

    //TODO: Look into just removing from the set
    for(Party party : parties)
    {
        int sum = 0;
        auto it = requiredShares.begin();
        for(int i = 0; i < requiredShares.size(); i++)
        {
            RequiredShare* requiredShare = requiredShares[i];

            auto contains_x_it = (std::find_if(party.shares.begin(), party.shares.end(), [&requiredShare](Share* arg)
            {
                return ((arg->batch_id == requiredShare->batch_id_x && arg->id == requiredShare->id_x));
            }));

            if(contains_x_it != party.shares.end())
            {
                auto contains_y_it = (std::find_if(party.shares.begin(), party.shares.end(), [&requiredShare](Share* arg)
                {
                    return ((arg->batch_id == requiredShare->batch_id_y && arg->id == requiredShare->id_y));
                }));

                if(contains_y_it != party.shares.end())
                {
                    sum += (*contains_x_it)->value * (*contains_y_it)->value;
                    requiredShares.erase(it);
                    //if we remove an element, the next element will be at current position, so we decrement iterator
                    //and counter

                    i--;
                    it--;
                }
            }
            it++;


            //auto iterator = std::find_if(party.shares.begin(), party.shares.end(), [&requiredShare](Share* arg)
            //{
            //    return ((arg->batch_id == requiredShare->batch_id_x && arg->id == requiredShare->id_x) || (arg->batch_id == requiredShare->batch_id_y && arg->id == requiredShare->id_y));
            //});
            //
            //if(iterator != std::end(party.shares))
            //{
            //    auto s = 1;
            //}

            auto s = 1;
        }
        std::cout << sum << std::endl;
    }
    auto t = 1;

}

int Addition(std::vector<Party*> partiesToReconstruct, int fieldSize, Party* x_owner, Party* y_owner)
{
    //Adding together the shares coming from x_owner and y_owner


    //First naive implementation where we first iterate the shares and collect only the relevant ones
    //This might however not be necessary, as we can instead just skip non relevant shares when iterating the entire
    //set
    //std::map<Party*, std::vector<Share*>> relevantShares;
    //for(auto p : partiesToReconstruct)
    //{
    //    std::vector<Share*> temp_shares (p->shares.size());

    //    auto it = std::copy_if (p->shares.begin(), p->shares.end(), temp_shares.begin(), [&x_owner, &y_owner](Share* share){return share->owner == x_owner || share->owner == y_owner;} );
    //    temp_shares.resize(std::distance(temp_shares.begin(),it));
    //    relevantShares[p] = temp_shares;
    //    //std::copy_if(p.shares.begin(), p.shares.end(), std::back_inserter(temp_shares), comp());
    //}


    int result = 0;
    std::set<Share*> usedShares;
    for(Party* party : partiesToReconstruct)
    {
        int sum = 0;
        for(Share* share : party->shares)
        {
            if(share->owner == x_owner || share->owner == y_owner)
            {
                if(usedShares.find(share) != usedShares.end()) continue;

                sum += share->value;
                usedShares.insert(share);
            }
        }

        //Broadcast the result
        std::cout << sum << std::endl;
        result += (sum);
    }
    std::cout << "Result: " << result << std::endl;

    //assuming that every party has the same amount of shares
    //for(int i = 0; i < partiesToReconstruct[0].shares.size(); i++)
    //{
    //    Share* share;
    //    for(auto p : partiesToReconstruct)
    //    {
    //    }
    //}
    //Find all the shares the parties have in common
    //remove those shares from everyone but 1 party
    //Each party adds their local shares and modulos the result
    //Each party broadcasts their result to every other party
    //Every party adds together the messages they recieve from the broadcasts and modulos the result
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

std::vector<std::string> SplitInput(std::string input, std::string delimiter) {
    size_t pos_start = 0, pos_end, delim_len = delimiter.length();
    std::string token;
    std::vector<std::string> res;

    while ((pos_end = input.find (delimiter, pos_start)) != std::string::npos) {
        token = input.substr (pos_start, pos_end - pos_start);
        pos_start = pos_end + delim_len;
        res.push_back (token);
    }

    res.push_back (input.substr (pos_start));
    return res;
}


int main() {

    //==================================== PHASE 1 =====================================================================
    //Creating parties and distributing their shares among the other parties participating in the protocol

    //Define the field size as a large prime
    int fieldSize = 2147483647;
    //Amount of parties participating in the protocol
    int amountOfParties = 3;
    //Amount of passive adversaries participating in the protocol
    //int amountOfPassiveAdversaries = 0;
    //Amount of parties with an input
    //int partiesWithInput = 1;
    //How many parties are required to reconstruct the secret
    int amountToReconstruct = 2;
    int batch_id = 0;


    std::string input = "100 + 200 + 300";
    std::vector<std::string> tokens = SplitInput(input, " ");


    //Create the parties participating in the protocol
    std::vector<Party> parties = CreateParties(amountOfParties);

    //Define the Non qualified sets (secrecy structure) for share distribution (a basic threshold access structure)
    std::set<std::set<Party*>> nonQualifiedSets = FindNonQualifiedSets(parties, amountToReconstruct);

    //Transform set of nonQualified to vector for indexes to use in distributing shares. This makes lookup time
    //O(1) instead of O(n), which dramatically increases performance (test with 15 parties made a 10x difference)
    std::vector <std::set<Party*>> nonQualifiedSetsIndexed;
    nonQualifiedSetsIndexed.reserve (nonQualifiedSets.size ());
    std::copy (nonQualifiedSets.begin (), nonQualifiedSets.end (), std::back_inserter (nonQualifiedSetsIndexed));

    DistributeInput(parties, nonQualifiedSets, fieldSize, nonQualifiedSetsIndexed, batch_id);


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
    //Addition(partiesToReconstruct, fieldSize, &parties[0], &parties[1]);
    Multiplication(parties, fieldSize, 0, 1, 3);
    //Addition(partiesToReconstruct, fieldSize, &parties[0], &parties[1]);
    //Now find all the shares all parties have in common

    //ReconstructAddition(parties);






    //Reconstruct(parties, amountToReconstruct, nonQualifiedSets.size());
    return 0;
}