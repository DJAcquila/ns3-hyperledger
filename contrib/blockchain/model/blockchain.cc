/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/traced-callback.h"
#include "ns3/address.h"
#include "ns3/log.h"
#include "ns3/inet-socket-address.h"


#include "blockchain.h"
#include "util.h"

namespace ns3 {
    Blockchain::Blockchain(void) {
        m_totalBlocks = 0;
        Block genesisBlock(0,0,0,0,0,0,0, Ipv4Address("0.0.0.0"));
        AddBlock(genesisBlock);
    }

    Blockchain::~Blockchain(void){}

    int Blockchain::GetTotalBlocks(void) const {
        return m_totalBlocks;
    }

    int Blockchain::GetBlockchainHeight(void) const {
        return GetCurrentTopBlock()->GetBlockHeight();
    }

    bool Blockchain::HasBlock(const Block &block) const {
        if(block.GetBlockHeight() > GetCurrentTopBlock()->GetBlockHeight()) {
            return false;
        } else {
            for (auto const &i_block : m_blocks[block.GetBlockHeight()]) {
                if(i_block == block) {
                    return true;
                }
            }
        }
        return false;
    }

    bool Blockchain::HasBlock(int height, int minerId) const {
        if(height > GetCurrentTopBlock()->GetBlockHeight()) {
            return false;
        } else {
            for (auto const &block : m_blocks[height]) {
                if(block.GetBlockHeight() == height && block.GetMinerId() == minerId) {
                    return true;
                
                }
            }
        }
        return false;
    }

    Block Blockchain::ReturnBlock(int height, int minerId) {
        std::vector<Block>::iterator block_it;

        if(height <= GetBlockchainHeight() && height >= 0) {
            for(block_it = m_blocks[height].begin(); block_it < m_blocks[height].end(); block_it++) {
                if(block_it->GetBlockHeight() == height && block_it->GetMinerId() == minerId) {
                    return *block_it;
                
                }
            }
        }

        for(block_it = m_blocks[height].begin(); block_it < m_blocks[height].end(); block_it++) {
            if(block_it->GetBlockHeight() == height && block_it->GetMinerId() == minerId) {
                return *block_it;
            
            }
        }

        return Block();
    }

    bool Blockchain::isOrphan (const Block &newBlock) const {
        for(auto const &block: m_orphans) {
            if(block == newBlock) {
                return true;
            }
        }
        return false;
    }

    bool Blockchain::isOrphan (int height, int minerId) const {
        for(auto const &block: m_orphans) {
            if(block.GetBlockHeight() == height && block.GetMinerId() == minerId) {
                return true;
            }
        }
        return false;
    }
    
    const Block* Blockchain::GetBlockPointer(const Block &newBlock) const {
        for(auto const &block : m_blocks[newBlock.GetBlockHeight()]) {
            if(block == newBlock) {
                return &block;
            }
        }
        return NULL;
    }

    const std::vector<const Block *> Blockchain::GetChildrenPointers (const Block &block) {
        std::vector<const Block *> children;
        std::vector<Block>::iterator block_it;
        int childrenHeight = block.GetBlockHeight() + 1;

        if(childrenHeight > GetBlockchainHeight())
        {
            return children;
        }

        for(block_it = m_blocks[childrenHeight].begin(); block_it < m_blocks[childrenHeight].end(); block_it++)
        {
            if(block.IsParent(*block_it))
            {
                children.push_back(&(*block_it));
            }
        }

        return children;
    }


    const std::vector<const Block *> Blockchain::GetOrpharnChildrenPointer (const Block &block)
    {
        std::vector<const Block *> children;
        std::vector<Block>::iterator block_it;

        for(block_it = m_orphans.begin(); block_it < m_orphans.end(); block_it++)
        {
            if(block.IsParent(*block_it))
            {
                children.push_back(&(*block_it));
            }
        }
        return children;
    }

    
    const Block* Blockchain::GetParent(const Block &block)
    {
        std::vector<Block>::iterator block_it;
        int parentHeight = block.GetBlockHeight() - 1;

        if(parentHeight > GetBlockchainHeight() || parentHeight < 0)
            return nullptr;
        
        for(block_it = m_blocks[parentHeight].begin(); block_it < m_blocks[parentHeight].end(); block_it++)
        {
            if(block.IsChild(*block_it))
            {
                return &(*block_it);
            }
        }
        return nullptr;
    }

    
    const Block* Blockchain::GetCurrentTopBlock(void) const {
        return &m_blocks[m_blocks.size()-1][0];
    }

    
    void Blockchain::AddBlock(const Block& newBlock)
    {

        if(m_blocks.size() == 0) {
           std::vector<Block> newHeight(1, newBlock);
           m_blocks.push_back(newHeight);
        }
        else if(newBlock.GetBlockHeight() > GetCurrentTopBlock()->GetBlockHeight()) {
           int dummyRows = newBlock.GetBlockHeight() - GetCurrentTopBlock()->GetBlockHeight()-1;

           for(int i = 0 ; i < dummyRows; i++) {
               std::vector<Block> newHeight;
               m_blocks.push_back(newHeight);
           }

           std::vector<Block> newHeight(1, newBlock);
           m_blocks.push_back(newHeight);
        } else {
            m_blocks[newBlock.GetBlockHeight()].push_back(newBlock);
        }
        m_totalBlocks++;
    }

    
    void Blockchain::AddOrphan(const Block& newBlock) {
        m_orphans.push_back(newBlock);
    }

    
    void Blockchain::RemoveOrphan(const Block& newBlock) {
        std::vector<Block>::iterator block_it;

        for(block_it = m_orphans.begin(); block_it < m_orphans.end(); block_it++) {
            if(newBlock==*block_it) {
                break;
            }
        }
        
        if(block_it == m_orphans.end()) {
            return;
        } else {
            m_orphans.erase(block_it);
        }
    }

    const char* GetMessageName(enum Messages m) {
        switch(m) {
            case INV: return "INV";
            case REQUEST_TRANS: return "REQUEST_TRANS";
            case GET_HEADERS: return "GET_HEADERS";
            case HEADERS: return "HEADERS";
            case GET_DATA: return "GET_DATA";
            case BLOCK: return "BLOCK";
            case NO_MESSAGE: return "NO_MESSAGE";
            case REPLY_TRANS: return "REPLY_TRANS";
            case MSG_TRANS: return "MSG_TRANS";
            case RESULT_TRANS: return "RESULT_TRANS";
        }

        return 0;
    }

    const char* GetMinerId(enum MinerType m) {
        switch(m) {
            case NORMAL_MINER: return "ETHEREUM";
            case HYPERLEDGER_MINER: return "HYPERLEDGER";
        }
        return 0;
    }

    const char* GetProtocolType(enum ProtocolType m) {
        switch(m) {
            case STANDARD_PROTOCOL: return "STANDARD_PROTOCOL";
            case SENDHEADERS: return "SENDHEADERS";
        }
        return 0;
    }

    const char* GetCommitterType(enum CommitterType m) {
        switch(m) {
            case COMMITTER: return "COMMITTER";
            case ENDORSER: return "ENDORSER" ;
            case CLIENT: return "CLIENT";
            case ORDER: return "ORDER";
        }
        return 0;
    }

    const char* GetCryptocurrency(enum Cryptocurrency m) {
        switch(m) {
            case ETHEREUM: return "ETHEREUM";
            case HYPERLEDGER: return "HYPERLEDGER";
        }
        return 0;
    }

    const char* GetBlockchainRegion(enum BlockchainRegion m) {
        switch(m) {
            case NORTH_AMERICA: return "NORTH_AMERICA";
            case EUROPE: return "EUROPE";
            case SOUTH_AMERICA: return "SOUTH_AMERICA";
            case KOREA: return "KOREA";
            case JAPAN: return "JAPAN";
            case AUSTRALIA: return "AUSTRALIA";
            case OTHER: return "OTHER";
        }
        return 0;
    }

    enum BlockchainRegion GetBlockchainRegion(uint32_t m) {
        switch(m) {
            case 0: return NORTH_AMERICA;
            case 1: return EUROPE;
            case 2: return SOUTH_AMERICA;
            case 3: return KOREA;
            case 4: return JAPAN;
            case 5: return AUSTRALIA;
            case 6: return OTHER;
        }
        return OTHER;
    }
}

