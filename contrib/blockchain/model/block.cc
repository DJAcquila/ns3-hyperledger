#include "block.h"

namespace ns3 {
    Block::Block(int blockHeight, int minerId, int nonce, int parentBlockMinerId, int blockSizeBytes,
                double timeStamp, double timeReceived, Ipv4Address receivedFromIpv4) {

        m_blockHeight = blockHeight;
        m_minerId = minerId;
        m_nonce = nonce;
        m_parentBlockMinerId = parentBlockMinerId;
        m_blockSizeBytes = blockSizeBytes;
        m_timeStamp = timeStamp;
        m_timeReceived = timeReceived;
        m_receivedFromIpv4 = receivedFromIpv4;
        m_totalTransactions = 0;
    }

    Block::Block() {
        Block(0,0,0,0,0,0.0,0.0,Ipv4Address("0.0.0.0"));
    }

    Block::Block(const Block &blockSource) {
        m_blockHeight = blockSource.m_blockHeight;
        m_minerId = blockSource.m_minerId;
        m_nonce = blockSource.m_nonce;
        m_parentBlockMinerId = blockSource.m_parentBlockMinerId;
        m_blockSizeBytes = blockSource.m_blockSizeBytes;
        m_timeStamp = blockSource.m_timeStamp;
        m_timeReceived = blockSource.m_timeReceived;
        m_receivedFromIpv4 = blockSource.m_receivedFromIpv4;
        m_transactions = blockSource.m_transactions;
        m_totalTransactions = 0;
    }

    Block::~Block(void) {}

    int Block::GetBlockHeight(void) const {
        return m_blockHeight;
    }

    void Block::SetBlockHeight(int blockHeight) {
        m_blockHeight = blockHeight;
    }

    int Block::GetNonce(void) const {
        return m_nonce;
    }

    void Block::SetNonce(int nonce) {
        m_nonce = nonce;
    }

    int Block::GetMinerId(void) const {
        return m_minerId;
    }

    void Block::SetMinerId(int minerId) {
        m_minerId = minerId;
    }

    int Block::GetParentBlockMinerId(void) const {
        return m_parentBlockMinerId;
    }
    
    void Block::SetParentBlockMinerId(int parentBlockMinerId) {
        m_parentBlockMinerId = parentBlockMinerId;
    }

    int Block::GetBlockSizeBytes(void) const {
        return m_parentBlockMinerId;
    }

    void Block::SetBlockSizeBytes(int blockSizeBytes) {
        m_blockSizeBytes = blockSizeBytes;
    }

    double Block::GetTimeStamp(void) const {
        return m_timeStamp;
    }

    void Block::SetTimeStamp(double timeStamp) {
        m_timeStamp = timeStamp;
    }

    double Block::GetTimeReceived(void) const {
        return m_timeReceived;
    }

    Ipv4Address Block::GetReceivedFromIpv4(void) const {
        return m_receivedFromIpv4;
    }

    void Block::SetReceivedFromIpv4(Ipv4Address receivedFromIpv4) {
        m_receivedFromIpv4 = receivedFromIpv4;
    }

    std::vector<Transaction> Block::GetTransactions(void) const {
        return m_transactions;
    }

    void Block::SetTransactions(const std::vector<Transaction> &transactions) {
        m_transactions = transactions;
    }

    bool Block::IsParent (const Block &block) const {

        if(GetBlockHeight() == block.m_blockHeight - 1 && GetParentBlockMinerId() == block.GetParentBlockMinerId()) {
            return true;
        } 
        return false;
        
    }

    bool Block::IsChild (const Block &block) const {

        if(GetBlockHeight() == block.m_blockHeight + 1 && GetParentBlockMinerId() == block.GetParentBlockMinerId()) {
            return true;
        }
        return false;
        
    }

    int Block::GetTotalTransaction(void) const {
        return m_totalTransactions;
    }

    Transaction Block::ReturnTransaction(int nodeId, int transId) {

        for(auto const &trans: m_transactions) {
            if (trans.GetTransactionNodeId() == nodeId && trans.GetTransactionId() == transId) {
                return trans;
            }
        }
        return Transaction();
    }


    bool Block::HasTransaction(Transaction &newTrans) const {
        for(auto const &trans: m_transactions) {
            if (trans == newTrans) {
                return true;
            }
        }
        return false;
    }

    bool Block::HasTransaction(int nodeId, int transId) const {
        for(auto const &trans: m_transactions) {
            if (trans.GetTransactionNodeId() == nodeId && trans.GetTransactionId() == transId) {
                return true;
            }
        }
        return false;
    }

    void Block::AddTransaction(const Transaction& newTrans) {
        m_transactions.push_back(newTrans);
        m_totalTransactions++;
    }

    void Block::PrintAllTransaction(void) {
        if(m_transactions.size() != 0)
        {
            for(auto const &tran: m_transactions)
            {
                std::cout << "[Blockheight: " <<m_blockHeight << "] Transaction nodeId: " 
                    << tran.GetTransactionNodeId() << " transId : " << tran.GetTransactionId() << "\n";
            }
        }
        else
        {
            std::cout << "[Blockheight: " << m_blockHeight << "]  do not have transactions\n";
        }

    }

    Block& Block::operator= (const Block &blockSource)
    {
        m_blockHeight = blockSource.m_blockHeight;
        m_minerId = blockSource.m_minerId;
        m_nonce = blockSource.m_nonce;
        m_parentBlockMinerId = blockSource.m_parentBlockMinerId;
        m_blockSizeBytes = blockSource.m_blockSizeBytes;
        m_timeStamp = blockSource.m_timeStamp;
        m_timeReceived = blockSource.m_timeReceived;
        m_receivedFromIpv4 = blockSource.m_receivedFromIpv4;

        return *this;
    }

    bool operator== (const Block &block1, const Block &block2)
    {
        if(block1.GetBlockHeight() == block2.GetBlockHeight() && block1.GetMinerId() == block2.GetMinerId())
            return true;
        else
            return false;
    }
}