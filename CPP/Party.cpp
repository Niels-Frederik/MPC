#include<vector>
#include <iostream>
#include<random>
#include<set>

class Party {
    int input;
    int index;
    std::vector<std::tuple<std::tuple<int,int>,int>> shares;
    char* secret;
    int fieldSize;

public:
    Party(int fieldSize, int index)
    {
        this->index = index;
        this->fieldSize = fieldSize;
        std::random_device dev;
        std::mt19937 rng(dev());
        std::uniform_int_distribution<std::mt19937::result_type> dist6(1,fieldSize); // distribution in range [1, 6]
        input = dist6(rng);
    }

    void ReceiveShare(std::tuple<std::tuple<int,int>,int> share)
    {
        shares.emplace_back(share);
    }

    void DistributeInput(std::vector<Party>& parties, std::vector<std::set<int>> nonQualifiedSets)
    {
        int* sharesToDistribute;
        sharesToDistribute = CreateShares(nonQualifiedSets.size());

        // for each share
        for(int share = 0; share < nonQualifiedSets.size() ; share++)
        {
            //who should not receive the share
            auto notReceive = nonQualifiedSets[share];
            //for each party
            for(int party = 0; party < parties.size(); party++)
            {
                //if party is not in the non qualified set
                if(notReceive.find(party) == notReceive.end())
                {
                    parties[party].ReceiveShare(std::make_tuple((std::make_tuple(index, share)), sharesToDistribute[share]));
                }
            }
        }
    }

    int* CreateShares(int amountOfShares)
    {
        std::random_device dev;
        std::mt19937 rng(dev());
        std::uniform_int_distribution<std::mt19937::result_type> dist6(1,fieldSize); // distribution in range [1, intmax]

        int* shares = new int[amountOfShares];
        int sum = 0;
        for(int i = 0; i < amountOfShares-1; i++)
        {
            int random = dist6(rng);
            shares[i] = random;
            sum+=random;
        }
        shares[amountOfShares-1] = (input - sum) % fieldSize;
        return shares;
    }

    int GetInput()
    {
        return input;
    }

    std::vector<std::tuple<std::tuple<int,int>,int>> GetShares()
    {
        return shares;
    }
};