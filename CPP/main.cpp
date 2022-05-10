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

std::vector<Share*> CreateShares(Party* party, int amountOfShares, int fieldSize, int batch_id, int id, int value = NULL)
{
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> dist6(1,fieldSize); // distribution in range [1, intmax]

    std::vector<Share*> shares;
    int sum = 0;

    for(int i = 0; i < amountOfShares-1; i++)
    {
        int random = dist6(rng);
        Share* share = new Share(random, party, batch_id, id);
        shares.emplace_back(share);
        sum+=random;
        id++;
    }
        int val = (value != NULL) ? value-sum : (party->input-sum);
        shares.emplace_back(new Share(val, party, batch_id, id));

    return shares;
}

void DistributeShares(std::vector<Party>& parties, std::vector<Share*> shares, std::vector<std::set<Party*>> nonQualified)
{
    // for each share (we always have the same amount of shares as non qualified sets of parties)
    for(int share = 0; share < shares.size(); share++)
    {
        //std::set<Party*> shouldNotReceive = *std::next(nonQualifiedSets.begin(), share);
        std::set<Party*> shouldNotReceive;
        if(nonQualified.size() != 0) shouldNotReceive = nonQualified[share];

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

void DistributeInput(std::vector<Party>& parties, int amountOfShares, int fieldSize, std::vector<std::set<Party*>> nonQualified, int& batch_id, int id)
{
    for(int i = 0; i < parties.size(); i++) {

        //Get the shares from party i
        std::vector<Share*> shares = CreateShares(&parties[i], amountOfShares, fieldSize, batch_id, id);
        DistributeShares(parties, shares, nonQualified);
        batch_id++;
    }
}

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

void Reshare(std::vector<Party>& parties, Party* party, int fieldSize, int amountOfShares, std::vector<std::set<Party*>> nonQualified, int valueToReshare, int& batch_id, int id)
{
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> dist6(1,fieldSize);

    auto shares = CreateShares(party, amountOfShares, fieldSize, batch_id, id, valueToReshare);
    DistributeShares(parties, shares, nonQualified);

    //int id_count = 0;
    //for(int i = 0; i < parties.size()-1; i++)
    //{
    //    //Generate a random number between 1 and fieldSize
    //    int randomNumber = dist6(rng);

    //    //Create a new share
    //    Share* share = new Share(randomNumber+valueToReshare, party, batch_id, id_count);

    //    id_count++;
    //    //Share(int v, Party* o, int batch_id, int id){

    //}
}

int Multiplication(std::vector<Party>& parties, int fieldSize, int batch_id_x, int batch_id_y, std::vector<std::set<Party*>> nonQualified, int amountOfShares, int& batch_id)
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

    int id = 0;
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
        }
        Reshare(parties, &party, fieldSize, amountOfShares, nonQualified, sum, batch_id, id);
        id+=amountOfShares;
        std::cout << sum << std::endl;
    }
    batch_id++;
}


int Addition(std::vector<Party>& parties, std::vector<Party*> partiesToReconstruct, std::vector<std::set<Party*>> nonQualified, int amountOfShares, int fieldSize, int batch_id_x, int batch_id_y, int& batch_id)
{
    //Adding together the shares coming from x and y
    int result = 0;
    std::set<Share*> usedShares;
    for(Party* party : partiesToReconstruct)
    {
        int sum = 0;
        for(Share* share : party->shares)
        {
            if(share->batch_id == batch_id_x || share->batch_id == batch_id_y)
            {
                if(usedShares.find(share) != usedShares.end()) continue;

                sum += share->value;
                usedShares.insert(share);
            }
        }
        Reshare(parties, party, fieldSize, amountOfShares, nonQualified, sum, batch_id, 0);

        //Broadcast the result
        //std::cout << sum << std::endl;
        //result += (sum);
    }
    //std::cout << "Result: " << result << std::endl;
}

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

void Reconstruct(std::vector<Party*> partiesToReconstruct, int batch_id_to_reconstruct)
{
    int result = 0;
    std::set<Share*> usedShares;
    for(Party* party : partiesToReconstruct)
    {
        int sum = 0;
        for(Share* share : party->shares)
        {
            if(share->batch_id == batch_id_to_reconstruct)
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
    //int fieldSize = 2147483647;
    int fieldSize = 300;
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

    DistributeInput(parties, nonQualifiedSets.size(), fieldSize, nonQualifiedSetsIndexed, batch_id, 0);


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
    //Multiplication(parties, fieldSize, 0, 1, nonQualifiedSetsIndexed, 3, batch_id);
    Reconstruct(partiesToReconstruct, 3);
    Addition(parties, partiesToReconstruct, nonQualifiedSetsIndexed, 3, fieldSize, 0, 1, batch_id);
    Reconstruct(partiesToReconstruct, 3);
    //Now find all the shares all parties have in common

    //ReconstructAddition(parties);

    //Reconstruct(parties, amountToReconstruct, nonQualifiedSets.size());
    return 0;
}