#ifndef BLOCK_H
#define BLOCK_H

#include <vector>
#include "ns3/address.h"
#include "ns3/inet-socket-address.h"

#include "transaction.h"
#include "util.h"

namespace ns3 {
    class Block {
        public:
            Block(int blockHeight, int minerId, int nonce, int parentBlockMinerId, int blockSizeBytes,
                double timeStamp, double timeReceived, Ipv4Address receivedFromIpv4);
            Block();
            Block(const Block &blockSource);
            virtual ~Block(void);

            int GetBlockHeight(void) const;
            void SetBlockHeight(int blockHeight);

            int GetNonce(void) const;
            void SetNonce(int nonce);

            int GetMinerId(void) const;
            void SetMinerId(int minerId);

            int GetParentBlockMinerId(void) const;
            void SetParentBlockMinerId(int parentBlockMinerId);

            int GetBlockSizeBytes(void) const;
            void SetBlockSizeBytes(int blockSizeBytes);

            double GetTimeStamp(void) const;
            void SetTimeStamp(double timeStamp);

            double GetTimeReceived(void) const;
            
            Ipv4Address GetReceivedFromIpv4(void) const;
            void SetReceivedFromIpv4(Ipv4Address receivedFromIpv4); 


            std::vector<Transaction> GetTransactions(void) const;
            void SetTransactions(const std::vector<Transaction> &transactions);

            bool IsParent (const Block &block) const;
            bool IsChild (const Block &block) const;

            int GetTotalTransaction(void) const;

            Transaction ReturnTransaction(int nodeId, int transId);

            bool HasTransaction(Transaction &newTran) const;
            bool HasTransaction(int nodeId, int tranId) const;

            void AddTransaction(const Transaction& newTrans);

            void PrintAllTransaction(void);

            Block& operator = (const Block &blockSource);
            friend bool operator == (const Block &block1, const Block &block2);

        protected:
            int m_blockHeight;
            int m_minerId;
            int m_nonce;
            int m_parentBlockMinerId;
            int m_blockSizeBytes;
            int m_totalTransactions;
            double m_timeStamp;
            double m_timeReceived;

            Ipv4Address m_receivedFromIpv4;
            std::vector<Transaction> m_transactions;
    };
}

#endif