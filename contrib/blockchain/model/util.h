#ifndef UTIL_H
#define UTIL_H

namespace ns3 {
    enum Messages
    {
        INV,
        REQUEST_TRANS,
        GET_HEADERS,
        HEADERS,
        GET_DATA,
        BLOCK,
        NO_MESSAGE,
        REPLY_TRANS,
        MSG_TRANS,
        RESULT_TRANS,
    };

    enum MinerType
    {
        NORMAL_MINER,
        HYPERLEDGER_MINER,
    };

    enum CommitterType
    {
        COMMITTER,
        ENDORSER,
        CLIENT,
        ORDER,
    };
    
    enum ProtocolType
    {
        STANDARD_PROTOCOL,
        SENDHEADERS
    };

    enum Cryptocurrency
    {
        ETHEREUM,
        HYPERLEDGER
    };

    enum BlockchainRegion
    {
        NORTH_AMERICA,
        EUROPE,
        SOUTH_AMERICA,
        KOREA,
        JAPAN,
        AUSTRALIA,
        OTHER
    };


    typedef struct
    {
        int nodeId;
        double meanBlockReceiveTime;
        double meanBlockPropagationTime;
        double meanBlockSize;
        int totalBlocks;
        int miner;
        int minerGeneratedBlocks;
        double minerAverageBlockGenInterval;
        double minerAverageBlockSize;
        double hashRate;
        long invReceivedBytes;
        long invSentBytes;
        long getHeadersReceivedBytes;
        long getHeadersSentBytes;
        long headersReceivedBytes;
        long headersSentBytes;
        long getDataReceivedBytes;
        long getDataSentBytes;
        long blockReceivedBytes;
        long blockSentBytes;
        int longestFork;
        int blocksInForks;
        int connections;
        int minedBlocksInMainChain;
        long blockTimeouts;
        int nodeGeneratedTransaction;
        double meanEndorsementTime;
        double meanOrderingTime;
        double meanValidationTime;
        double meanLatency;
        int nodeType;
        double meanNumberofTransactions;
    } nodeStatistics;

    typedef struct{
        double downloadSpeed;
        double uploadSpeed;
    } nodeInternetSpeed;
}



#endif