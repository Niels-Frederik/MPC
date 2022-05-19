#include <iostream>
#include <set>
#include <vector>
#include <random>
#include <math.h>
#include <map>
#include <list>

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
    bool hasInput;

    explicit Party(int input, bool hasInput) {
        this->input = input;
        this->hasInput = hasInput;
    };

    void addShare(Share* share) {
        shares.insert(share);
    }
};

std::vector<Share*> CreateShares(Party* party, int amountOfShares, int fieldSize, int batch_id, int id, int value, bool reshare)
{
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> dist6(1,fieldSize);

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
        int val = (reshare) ? value-sum : (party->input-sum);
        shares.emplace_back(new Share(val, party, batch_id, id));

    return shares;
}

void DistributeShares(std::vector<Party>& parties, Party* party, std::vector<Share*> shares, std::vector<std::set<Party*>> nonQualified)
{
    // for each share (we always have the same amount of shares as non qualified sets of parties)
    for(int share = 0; share < shares.size(); share++)
    {
        //std::set<Party*> shouldNotReceive = *std::next(nonQualifiedSets.begin(), share);
        std::set<Party*> shouldNotReceive;
        if(nonQualified.size() != 0) shouldNotReceive = nonQualified[share];

        for(Party& p : parties)
        {
            //if party is not in the non qualified set or the party is itself
            if(shouldNotReceive.find(&p) == shouldNotReceive.end() || &p == party)
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
        if(parties[i].hasInput) {
            std::vector<Share*> shares = CreateShares(&parties[i], amountOfShares, fieldSize, batch_id, id, NULL, false);
            std::cout << "Party " << i<< " distributes " << shares.size() << " shares from input " << parties[i].input << " batch(" << batch_id << ")" << std::endl;
            DistributeShares(parties, &parties[i], shares, nonQualified);
            batch_id++;
        }
    }
}

std::set<std::set<Party*>> x_out_of_y_setCombinations(std::vector<Party>& parties, int x, int y)
{
    std::set<std::set<Party*>> combinations = std::set<std::set<Party*>>();
    std::string bitmask(x, 1);
    bitmask.resize(y, 0);

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
    std::set<std::set<Party*>> combinations = x_out_of_y_setCombinations(parties, amountToReconstruct-1, parties.size());
    NonQualifiedSets.insert(combinations.begin(), combinations.end());
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

    auto shares = CreateShares(party, amountOfShares, fieldSize, batch_id, id, valueToReshare, true);
    DistributeShares(parties, party, shares, nonQualified);
    std::cout << "Split input into " << shares.size() << " shares and redistributed them" << std::endl;
}

int Multiplication(std::vector<Party>& parties, int fieldSize, int batch_id_x, int batch_id_y, std::vector<std::set<Party*>> nonQualified, int amountOfShares, int& batch_id, std::map<int,int>& batch_id_to_shares_count)
{
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

    std::cout << "=========================== MULTIPLICATION ===============================" << std::endl;
    std::cout << "Multiplication between batch " << batch_id_x << " and batch " << batch_id_y << std::endl;

    //create all the required shares
    std::list<RequiredShare> requiredSharesList;

    for(int i = 0; i < batch_id_to_shares_count[batch_id_x]; i++) {
        for(int j = 0; j < batch_id_to_shares_count[batch_id_y]; j++)
        {
            RequiredShare r (batch_id_x, i, batch_id_y, j);
            requiredSharesList.emplace_back(r);
        }
    }

    int id = 0;
    for(Party party : parties)
    {
        std::vector<RequiredShare*> unusedShares;
        int sum = 0;
        std::list<RequiredShare>::iterator requiredShare = requiredSharesList.begin();
        while(requiredShare != requiredSharesList.end())
        {
            bool usedShare = false;
            auto contains_x_it = (std::find_if(party.shares.begin(), party.shares.end(), [&requiredShare](Share* arg)
            {
                return ((arg->batch_id == (*requiredShare).batch_id_x && arg->id == (*requiredShare).id_x));
            }));

            if(contains_x_it != party.shares.end())
            {
                auto contains_y_it = (std::find_if(party.shares.begin(), party.shares.end(), [&requiredShare](Share* arg)
                {
                    return ((arg->batch_id == (*requiredShare).batch_id_y && arg->id == (*requiredShare).id_y));
                }));

                if(contains_y_it != party.shares.end())
                {
                    sum += (*contains_x_it)->value * (*contains_y_it)->value;
                    requiredSharesList.erase(requiredShare++);
                    usedShare = true;
                }
            }
            if(!usedShare) ++requiredShare;
        }

        std::cout << "Party computed local shares to: " << sum << std::endl;
        Reshare(parties, &party, fieldSize, amountOfShares, nonQualified, sum, batch_id, id);
        id+=amountOfShares;
    }
    std::cout << "Multiplication done, results reshared as batch " << batch_id << std::endl;
    batch_id_to_shares_count[batch_id] = amountOfShares*parties.size();
    batch_id++;
    std::cout << "==========================================================================" << std::endl;
}


int Addition(std::vector<Party>& parties, std::vector<Party*> partiesToReconstruct, std::vector<std::set<Party*>> nonQualified, int amountOfShares, int fieldSize, int batch_id_x, int batch_id_y, int& batch_id, std::map<int,int>& batch_id_to_shares_count)
{
    //Adding together the shares coming from x and y
    std::cout << "============================== ADDITION ==================================" << std::endl;
    std::cout << "Addition between batch " << batch_id_x << " and batch " << batch_id_y << std::endl;
    std::set<Share*> usedShares;
    int id = 0;
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
        std::cout << "Party computed local shares to: " << sum << std::endl;
        Reshare(parties, party, fieldSize, amountOfShares, nonQualified, sum, batch_id, id);
        id+=amountOfShares;
    }
    std::cout << "Addition done, results reshared as batch " << batch_id << std::endl;
    batch_id_to_shares_count[batch_id] = amountOfShares*partiesToReconstruct.size();
    batch_id++;
    std::cout << "==========================================================================" << std::endl;
}

std::vector<Party> CreateParties(int amountOfParties, std::vector<std::string> inputs)
{
    std::vector<Party> parties;
    int inputCount = 0;
    for(int i = 0; i < amountOfParties; i++) {
        if(inputCount < inputs.size())
        {
            Party party(stoi(inputs[inputCount]), true);
            parties.push_back(party);
            inputCount++;
        }
        else
        {
            Party party(NULL, false);
            parties.push_back(party);
        }
    }
    return parties;
}

void Reconstruct(std::vector<Party*> partiesToReconstruct, int batch_id_to_reconstruct)
{
    int result = 0;
    std::cout << partiesToReconstruct.size() << " parties are reconstructing batch " << batch_id_to_reconstruct << std::endl;
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
        std::cout << "Party sum of shares: " << sum << std::endl;
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
    int fieldSize = 2147483647;
    int amountOfParties = 10;
    //t-out-of-n threshold where t = n/2
    int amountToReconstruct = floor(amountOfParties*0.5);
    int batch_id = 0;
    std::map<int,int> batch_id_count_of_shares;

    std::string input = "10,20,30,40";
    std::vector<std::string> tokens = SplitInput(input, ",");

    //Create the parties participating in the protocol
    std::vector<Party> parties = CreateParties(amountOfParties, tokens);

    //Define the Non qualified sets (secrecy structure) for share distribution (a basic threshold access structure)
    std::set<std::set<Party*>> nonQualifiedSets = FindNonQualifiedSets(parties, amountToReconstruct);

    //The original inputs all have amount of shares equal to unqualified sets
    for(int i = 0; i < tokens.size(); i++)
    {
        batch_id_count_of_shares[i] = nonQualifiedSets.size();
    }

    //Transform set of nonQualified to vector for indexes to use in distributing shares. This makes lookup time
    //O(1) instead of O(n), which dramatically increases performance (test with 15 parties made a 10x difference)
    std::vector <std::set<Party*>> nonQualifiedSetsIndexed;
    nonQualifiedSetsIndexed.reserve (nonQualifiedSets.size ());
    std::copy (nonQualifiedSets.begin (), nonQualifiedSets.end (), std::back_inserter (nonQualifiedSetsIndexed));

    DistributeInput(parties, nonQualifiedSets.size(), fieldSize, nonQualifiedSetsIndexed, batch_id, 0);

    //==================================== PHASE 2 =====================================================================
    //At this point all the inputs have been split into shares and distributed. We can now begin computing a function
    //on these shares. Depending on the input and function to compute, we have to do some different things
    std::vector<Party*> partiesToReconstruct = GetRandomPartiesToReconstruct(parties, amountToReconstruct);

    std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();

    Multiplication(parties, fieldSize, 0, 1,
                   nonQualifiedSetsIndexed, nonQualifiedSets.size(), batch_id, batch_id_count_of_shares);

    Reconstruct(partiesToReconstruct,batch_id-1);

    Addition(parties, partiesToReconstruct, nonQualifiedSetsIndexed, nonQualifiedSets.size(), fieldSize,0, batch_id-1, batch_id, batch_id_count_of_shares);

    Reconstruct(partiesToReconstruct,batch_id-1);

    Multiplication(parties, fieldSize, 0, batch_id-1,
                   nonQualifiedSetsIndexed, nonQualifiedSets.size(), batch_id, batch_id_count_of_shares);

    Reconstruct(partiesToReconstruct,batch_id-1);

    return 0;
}