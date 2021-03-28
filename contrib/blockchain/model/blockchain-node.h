#ifndef BLOCKCHAIN_NODE_H
#define BLOCKCHAIN_NODE_H

#include <algorithm>
#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/traced-callback.h"
#include "ns3/address.h"
#include "ns3/boolean.h"

#include "blockchain.h"
#include "util.h"
#include "../../../rapidjson/document.h"
#include "../../../rapidjson/writer.h"
#include "../../../rapidjson/stringbuffer.h"


namespace ns3 {

    class Address;
    class Socket;
    class Packet;

    class BlockchainNode : public Application {
        public:
            static TypeId GetTypeId(void);
            BlockchainNode(void);

            virtual ~BlockchainNode(void);

            Ptr<Socket> GetListeningSocket(void) const;

            std::vector<Ipv4Address> GetPeerAddress(void) const;
            void SetPeersAddresses(const std::vector<Ipv4Address> &peers);

            void SetPeersDownloadSpeeds(const std::map<Ipv4Address, double> &peerDownloadSpeeds);
            void SetPeersUploadSpeeds(const std::map<Ipv4Address, double> &peerUploadSpeeds);

            void SetNodeInternetSpeeds(const nodeInternetSpeed &internetSpeeds);
            void SetNodeStats(nodeStatistics *nodeStats);

            void SetProtocolType(enum ProtocolType protocolType);
            void SetCommitterType(enum CommitterType cType);

            void SetCreatingTransactionTime(int cTime);

        protected:
            virtual void DoDispose (void);
            virtual void StartApplication (void);
            virtual void StopApplication (void);

            void HandleRead (Ptr<Socket> socket);
            void HandleAccept(Ptr<Socket> socket, const Address& from);
            void HandlePeerClose(Ptr<Socket> socket);
            void HandlePeerError(Ptr<Socket> socket);
            void ReceivedBlockMessage(std::string &blockInfo, Address &from);
            virtual void ReceiveBlock(const Block &newBlock);
            void SendBlock(std::string &blockInfo, Address &from);
            virtual void ReceivedHigherBlock(const Block &newBlock);
            void ValidadeBlock(const Block &newBlock);
            void ValidateTransaction(const Block &newBlock);
            void AfterBlockValidation(const Block &newBlock);
            void ValudateOrphanChildren(const Block &newBlock);
            
            void AdvertiseNewBlock(const Block &newBlock);
            void AdvertiseNewTransaction(const Transaction &newTrans, enum Messages msgType, Ipv4Address receivedFromIpv4);
            
            bool HasTransaction(int nodeId, int transId);
            bool HasReplyTransaction(int nodeId, int transId, int transExecution);
            bool HasMessageTransaction(int nodeId, int transId);
            bool HasResultTransaction(int nodeId, int transId);
            bool HasTransactionAndValidated(int nodeId, int transId);

            void CreateTransaction();
            void ScheduleNextTransaction();
            void ExecuteTransaction(const Transaction &newTrans, Ipv4Address receivedFromIpv4);
            void NotifyTransaction(const Transaction &newTrans);

            void SendMessage(enum Messages receivedMessage, enum Messages responseMessage, 
                            rapidjson::Document &d, Ptr<Socket> outgoingSocket);
            void SendMessage(enum Messages receivedMessage, enum Messages responseMessage, 
                            rapidjson::Document &d, Address &outgoingAddress);
            void SendMessage(enum Messages receivedMessage, enum Messages responseMessage, 
                            std::string packet, Address &outgoingAddress);

            void InvTimeoutExpired (std::string blockHash);
            bool ReceivedButNotValidated(std::string blockHash);
            void RemoveReceivedButNotvalidated(std::string blockHash);
            bool OnlyHeadersReceived (std::string blockHash);
            
            void RemoveSendTime();
            void RemoveCompressedBlockSendTime();
            void RemoveReceiveTime();
            void RemoveCompressedBlockReceiveTime();


            Ptr<Socket>     m_socket;
            Address         m_local;
            TypeId          m_tid;
            int             m_numberOfPeers;
            double          m_meanBlockReceiveTime;
            double          m_previousBlockReceiveTime;
            double          m_meanBlockPropagationTime;
            double          m_meanEndorsementTime;
            double          m_meanOrderingTime;
            double          m_meanValidationTime;
            double          m_meanLatency;
            double          m_meanBlockSize;
            Blockchain      m_blockchain;
            Time            m_invTimeoutMinutes;
            bool            m_isMiner;
            double          m_downloadSpeed;
            double          m_uploadSpeed;
            double          m_averageTransacionSize;
            int             m_transactionIndexSize;
            int             m_transactionId;
            EventId         m_nextTransaction;
            int             m_numberofEndorsers;
            int             m_totalEndorsement;
            int             m_totalOrdering;
            int             m_totalValidation;
            int             m_totalCreatedTransaction;
            int             m_creatingTransactionTime;

            std::vector<Transaction>                        m_transaction;
            std::vector<Transaction>                        m_notValidatedTransaction;
            std::vector<Transaction>                        m_replyTransaction;
            std::vector<Transaction>                        m_msgTransaction;
            std::vector<Transaction>                        m_resultTransaction;
            std::vector<Transaction>                        m_waitingEndorsers;
            std::vector<Ipv4Address>                        m_peersAddresses;
            std::map<Ipv4Address, double>                   m_peersDownloadSpeeds;
            std::map<Ipv4Address, double>                   m_peersUploadSpeeds; 
            std::map<Ipv4Address, Ptr<Socket>>              m_peersSockets;  
            std::map<std::string, std::vector<Address>>     m_queueInv;
            std::map<std::string, EventId>                  m_invTimeouts;
            std::map<Address, std::string>                  m_bufferedData;  
            std::map<std::string, Block>                    m_receivedNotValidated;
            std::map<std::string, Block>                    m_onlyHeadersReceived;
            nodeStatistics                                  *m_nodeStats;    
            std::vector<double>                             m_sendBlockTimes;
            std::vector<double>                             m_sendCompressedBlockTimes;
            std::vector<double>                             m_receiveBlockTimes; 
            std::vector<double>                             m_receiveCompressedBlockTimes;
            enum ProtocolType                               m_protocolType;  
            enum CommitterType                              m_committerType;


            const int       m_blockchainPort;
            const int       m_secondsPerMin;
            const int       m_countBytes;
            const int       m_blockchainMessageHeader;
            const int       m_inventorySizeBytes;
            const int       m_getHeaderSizeBytes;
            const int       m_headersSizeBytes;
            const int       m_blockHeadersSizeBytes;

            TracedCallback<Ptr<const Packet>, const Address &> m_rxTrace;

    };
}

#endif