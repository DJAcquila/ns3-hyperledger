/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef BLOCKCHAIN_H
#define BLOCKCHAIN_H

#include <vector>
#include <map>
#include <algorithm>
#include "block.h"
#include "ns3/address.h"

namespace ns3 {
    
    const char* GetMessageName(enum Messages m);
    const char* GetMinerType(enum MinerType m);
    const char* GetCommitterType(enum CommitterType m);
    const char* GetProtocolType(enum ProtocolType m);
    const char* GetCryptocurrency(enum Cryptocurrency m);
    const char* GetBlockchainRegion(enum BlockchainRegion m);
    enum BlockchainRegion GetBlockchainEnum(uint32_t n);

    class Blockchain : public Block {
        public:
            Blockchain(void);
            virtual ~Blockchain(void);

            int GetTotalBlocks(void) const;
            int GetNoOrphans(void) const;
            int GetBlockchainHeight(void) const;

            bool HasBlock(const Block &block) const;
            bool HasBlock(int height, int minerId) const;

            Block ReturnBlock(int height, int minerId);

            bool isOrphan(const Block &newBlock) const;
            bool isOrphan(int height, int minerId) const;

            const Block* GetBlockPointer(const Block &newBlock) const;
            
            const std::vector<const Block *> GetChildrenPointers(const Block &block);

            const std::vector<const Block *> GetOrpharnChildrenPointer(const Block &block);

            const Block* GetParent(const Block &block);

            const Block* GetCurrentTopBlock(void) const;

            void AddBlock(const Block& newBlock);

            void AddOrphan(const Block& newBlock);

            void RemoveOrphan (const Block& newBlock);
        protected:

            int m_totalBlocks;
            std::vector<std::vector<Block>> m_blocks;
            std::vector<Block> m_orphans;
    };
}

#endif /* BLOCKCHAIN_H */

