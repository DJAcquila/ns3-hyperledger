#include "ns3/address.h"
#include "ns3/address-utils.h"
#include "ns3/log.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/node.h"
#include "ns3/socket.h"
#include "ns3/udp-socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/tcp-socket-factory.h"
#include "ns3/uinteger.h"
#include "ns3/double.h"


#include "blockchain-node.h"

namespace ns3 {
    NS_LOG_COMPONENT_DEFINE("BlockchainNode");
    NS_OBJECT_ENSURE_REGISTERED(BlockchainNode);

    TypeId BlockchainNode::GetTypeId(void) {
        static TypeId tid = TypeId("ns3::BlockchainNode")
        .SetParent<Application>()
        .SetGroupName("Applications")
        .AddConstructor<BlockchainNode>()
        .AddAttribute("Local",
                       "The Address on which to Bind the rx socket.",
                       AddressValue(),
                       MakeAddressAccessor(&BlockchainNode::m_local),
                       MakeAddressChecker())
        .AddAttribute("Protocol",
                       "The type id of the protocol to use for the rx socket",
                        TypeIdValue (UdpSocketFactory::GetTypeId()),
                        MakeTypeIdAccessor (&BlockchainNode::m_tid),
                        MakeTypeIdChecker())
        .AddAttribute("InvTimeoutMinutes",
                      "The timeout of inv messages in minutes",
                      TimeValue(Minutes(2)),
                      MakeTimeAccessor(&BlockchainNode::m_invTimeoutMinutes),
                      MakeTimeChecker())
        .AddTraceSource("Rx",
                        "A packet has been received",
                        MakeTraceSourceAccessor(&BlockchainNode::m_rxTrace),
                        "ns3::Packet::AddressTracedCallback");

        return tid;
    }

    BlockchainNode::BlockchainNode(void)
        : m_isMiner(false), m_averageTransacionSize(522.4), m_transactionIndexSize(2),
          m_blockchainPort(8333), m_secondsPerMin(60), m_countBytes(4), m_blockchainMessageHeader(90),
          m_inventorySizeBytes(36), m_getHeaderSizeBytes(72), m_headersSizeBytes(81), m_blockHeadersSizeBytes(81)
    {
        NS_LOG_FUNCTION(this);
        m_socket = 0;
        m_meanBlockReceiveTime = 0;
        m_previousBlockReceiveTime = 0;
        m_meanBlockPropagationTime = 0;
        m_meanEndorsementTime = 0;
        m_meanOrderingTime = 0;
        m_meanValidationTime = 0;
        m_meanLatency = 0;
        m_meanBlockSize = 0;
        m_numberOfPeers = m_peersAddresses.size();
        m_transactionId = 1;
        m_numberofEndorsers = 10;
        m_totalEndorsement = 0;
        m_totalOrdering = 0;
        m_totalValidation = 0;
        m_totalCreatedTransaction = 0;
    }

    BlockchainNode::~BlockchainNode(void) {
        NS_LOG_FUNCTION(this);
    }

    Ptr<Socket> BlockchainNode::GetListeningSocket(void) const {
        NS_LOG_FUNCTION(this);
        return m_socket;
    }

    std::vector<Ipv4Address> BlockchainNode::GetPeerAddress(void) const {
        NS_LOG_FUNCTION(this);
        return m_peersAddresses;
    }

    void BlockchainNode::SetPeersAddresses(const std::vector<Ipv4Address> &peers) {
        NS_LOG_FUNCTION(this);
        m_peersAddresses = peers;
        m_numberOfPeers = m_peersAddresses.size();
    }
    
    void BlockchainNode::SetPeersDownloadSpeeds(const std::map<Ipv4Address, double> &peersDownloadSpeeds) {
        NS_LOG_FUNCTION(this);
        m_peersDownloadSpeeds = peersDownloadSpeeds;
    }

    void BlockchainNode::SetNodeInternetSpeeds(const nodeInternetSpeed &internetSpeeds) {
        NS_LOG_FUNCTION(this);
        m_downloadSpeed = internetSpeeds.downloadSpeed*1000000/8;
        m_uploadSpeed = internetSpeeds.uploadSpeed*1000000/8;
    }

    void BlockchainNode::SetNodeStats(nodeStatistics* nodeStats) {
        NS_LOG_FUNCTION(this);
        m_nodeStats = nodeStats;
    }

    void BlockchainNode::SetProtocolType(enum ProtocolType protocolType) {
        NS_LOG_FUNCTION(this);
        m_protocolType = protocolType;
    }

    void BlockchainNode::SetCommitterType(enum CommitterType cType) {
        NS_LOG_FUNCTION(this);
        m_committerType = cType;
    }

    void BlockchainNode::SetCreatingTransactionTime(int cTime) {
        NS_LOG_FUNCTION(this);
        m_creatingTransactionTime = cTime;
    }

    void BlockchainNode::DoDispose(void) {
        NS_LOG_FUNCTION(this);
        m_socket = 0;
        Application::DoDispose();
    }

    void BlockchainNode::StartApplication() {
        NS_LOG_FUNCTION(this);
        srand(time(NULL) + GetNode()->GetId());

        NS_LOG_INFO("Node " << GetNode()->GetId() << ": download speed = " << m_downloadSpeed << "B/s");
        NS_LOG_INFO("Node " << GetNode()->GetId() << ": upload speed = " << m_uploadSpeed << "B/s");
        NS_LOG_INFO("Node " << GetNode()->GetId() << ": number of peers = " << m_numberOfPeers);
        NS_LOG_INFO("Node " << GetNode()->GetId() << ": m_protocolType = " << GetProtocolType(m_protocolType));
        NS_LOG_INFO("Node " << GetNode()->GetId() << ": peers are");


        for (auto it = m_peersAddresses.begin(); it != m_peersAddresses.end(); it++) {
            NS_LOG_INFO("\t " << *it);
        }

        if(!m_socket)
        {
            m_socket = Socket::CreateSocket(GetNode(), m_tid);
            m_socket->Bind(m_local);
            m_socket->Listen();
            m_socket->ShutdownSend();

            if (addressUtils::IsMulticast(m_local)) {
                Ptr<UdpSocket> udpSocket = DynamicCast<UdpSocket> (m_socket);

                if(udpSocket) {
                    udpSocket->MulticastJoinGroup(0, m_local);
                } else {
                    NS_FATAL_ERROR("Error : joining multicast on a non-UDP socket");
                }
            }
        }
        m_socket->SetRecvCallback(MakeCallback(&BlockchainNode::HandleRead, this));
        m_socket->SetAcceptCallback(MakeNullCallback<bool, Ptr<Socket>, const Address &>(),
                                    MakeCallback(&BlockchainNode::HandleAccept, this));
        m_socket->SetCloseCallbacks(MakeCallback(&BlockchainNode::HandlePeerClose, this),
                                    MakeCallback(&BlockchainNode::HandlePeerError, this));
        NS_LOG_DEBUG("Node " << GetNode()->GetId() << ": Before creating sockets");

        for(std::vector<Ipv4Address>::const_iterator i = m_peersAddresses.begin(); i != m_peersAddresses.end(); ++i) {
            m_peersSockets[*i] = Socket::CreateSocket(GetNode(), TcpSocketFactory::GetTypeId());
            m_peersSockets[*i]->Connect(InetSocketAddress(*i, m_blockchainPort));
        }

        NS_LOG_DEBUG("Node " << GetNode()->GetId()<<": After creating sockets");


        m_nodeStats->nodeId = GetNode()->GetId();
        m_nodeStats->meanBlockReceiveTime = 0;
        m_nodeStats->meanBlockPropagationTime = 0;
        m_nodeStats->meanBlockSize = 0;
        m_nodeStats->totalBlocks = 0;
        m_nodeStats->miner = 0;
        m_nodeStats->minerGeneratedBlocks = 0;
        m_nodeStats->minerAverageBlockGenInterval = 0;
        m_nodeStats->minerAverageBlockSize = 0;
        m_nodeStats->hashRate = 0;
        m_nodeStats->invReceivedBytes = 0;
        m_nodeStats->invSentBytes = 0;
        m_nodeStats->getHeadersReceivedBytes = 0;
        m_nodeStats->getHeadersSentBytes = 0;
        m_nodeStats->headersReceivedBytes = 0;
        m_nodeStats->headersSentBytes = 0;
        m_nodeStats->getDataReceivedBytes = 0;
        m_nodeStats->getDataSentBytes = 0;
        m_nodeStats->blockReceivedBytes = 0;
        m_nodeStats->blockSentBytes = 0;
        m_nodeStats->longestFork = 0;
        m_nodeStats->blocksInForks = 0;
        m_nodeStats->connections = m_peersAddresses.size();
        m_nodeStats->blockTimeouts = 0;
        m_nodeStats->nodeGeneratedTransaction = 0;
        m_nodeStats->meanEndorsementTime = 0;
        m_nodeStats->meanOrderingTime = 0;
        m_nodeStats->meanValidationTime = 0;
        m_nodeStats->meanLatency = 0;

        if(m_committerType == COMMITTER) {
            m_nodeStats->nodeType = 0;
        } else if(m_committerType == ENDORSER) {
            m_nodeStats->nodeType = 1;
        } else if(m_committerType == CLIENT) {
            m_nodeStats->nodeType = 2;
            CreateTransaction();
        } else {
            m_nodeStats->nodeType = 3;
        }
    }

    void BlockchainNode::StopApplication() {
        NS_LOG_FUNCTION(this);

        for(std::vector<Ipv4Address>::iterator i = m_peersAddresses.begin(); i != m_peersAddresses.end(); ++i) {
            m_peersSockets[*i]->Close();
        }

        if(m_socket) {
            m_socket->Close();
            m_socket->SetRecvCallback(MakeNullCallback<void, Ptr<Socket>>());
        }

        Simulator::Cancel(m_nextTransaction);

        NS_LOG_WARN("\n\nBLOCKCHAIN NODE " << GetNode()->GetId() << ":");
        //NS_LOG_WARN("Current Top Block is \n"<<*(m_blockchain.GetCurrentTopBlock()));
        //NS_LOG_WARN("Current Blockchain is \n" << m_blockchain);

        NS_LOG_WARN("Mean Block Receive Time = " << m_meanBlockReceiveTime << " or "
                    << static_cast<int>(m_meanBlockReceiveTime)/m_secondsPerMin << "min and "
                    << m_meanBlockReceiveTime - static_cast<int>(m_meanBlockReceiveTime)/m_secondsPerMin * m_secondsPerMin << "s");
        NS_LOG_WARN("Mean Block Propagation Time = " << m_meanBlockPropagationTime << "s");
        NS_LOG_WARN("Mean Block Size = " << m_meanBlockSize << "Bytes");
        NS_LOG_WARN("Total Block = " << m_blockchain.GetTotalBlocks());
        NS_LOG_WARN("Received But Not Validataed size : " << m_receivedNotValidated.size());
        NS_LOG_WARN("m_sendBlockTime size = " <<m_receiveBlockTimes.size());

        m_nodeStats->meanBlockReceiveTime = m_meanBlockReceiveTime;
        m_nodeStats->meanBlockPropagationTime = m_meanBlockPropagationTime;
        m_nodeStats->meanBlockSize = m_meanBlockSize;
        m_nodeStats->totalBlocks = m_blockchain.GetTotalBlocks();
        m_nodeStats->meanEndorsementTime = m_meanEndorsementTime;
        m_nodeStats->meanOrderingTime = m_meanOrderingTime;
        m_nodeStats->meanValidationTime = m_meanValidationTime;
        m_nodeStats->meanLatency = m_meanLatency;
    }

    void BlockchainNode::HandleRead(Ptr<Socket> socket) {
        NS_LOG_INFO(this << socket);
        Ptr<Packet> packet;
        Address from;

        while((packet = socket->RecvFrom(from))) {
            if(packet->GetSize() == 0) break;

            if(InetSocketAddress::IsMatchingType(from)) {
                std::string delimiter = "#";
                std::string parsedPacket;
                size_t pos = 0;
                char *packetInfo = new char[packet->GetSize() + 1];
                std::ostringstream totalStream;
                packet->CopyData(reinterpret_cast<uint8_t *>(packetInfo), packet->GetSize());
                packetInfo[packet->GetSize()] = '\0';

                totalStream << m_bufferedData[from] << packetInfo;
                std::string totalReceivedData(totalStream.str());
                NS_LOG_INFO("Node " << GetNode()->GetId() << " Total Received Data : " << totalReceivedData);
                while((pos = totalReceivedData.find(delimiter)) != std::string::npos) {
                    parsedPacket = totalReceivedData.substr(0, pos);
                    NS_LOG_INFO("Node " << GetNode()->GetId() << " Parsed Packet: " << parsedPacket);

                    rapidjson::Document document;
                    document.Parse(parsedPacket.c_str());
                    if(!document.IsObject()) {
                        NS_LOG_WARN("Corrupted packet");
                        totalReceivedData.erase(0, pos + delimiter.length());
                        continue;
                    }

                    rapidjson::StringBuffer buffer;
                    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
                    document.Accept(writer);

                    NS_LOG_INFO("At time " << Simulator::Now().GetSeconds()
                                << "s Blockchain node " << GetNode()->GetId() << " received"
                                << InetSocketAddress::ConvertFrom(from).GetIpv4()
                                << " port " << InetSocketAddress::ConvertFrom(from).GetPort()
                                << " with info = " << buffer.GetString());

                    switch(document["message"].GetInt()) {
                        case INV: 
                        {
                            NS_LOG_INFO("INV message");

                            if(m_committerType != CLIENT) {
                                unsigned int j;
                                std::vector<std::string>    requestBlocks;
                                std::vector<std::string>::iterator  block_it;

                                m_nodeStats->invReceivedBytes += m_blockchainMessageHeader + m_countBytes + document["inv"].Size()*m_inventorySizeBytes;
                                for(j = 0; j < document["inv"].Size() ; j++)
                                {
                                    std::string invDelimiter = "/";
                                    std::string parsedInv = document["inv"][j].GetString();
                                    size_t invPos = parsedInv.find(invDelimiter);
                                    EventId timeout;

                                    int height = atoi(parsedInv.substr(0, invPos).c_str());
                                    int minerId = atoi(parsedInv.substr(invPos+1, parsedInv.size()).c_str());

                                    if(m_blockchain.HasBlock(height, minerId) || m_blockchain.isOrphan(height, minerId) || ReceivedButNotValidated(parsedInv))
                                    {
                                        NS_LOG_INFO("INV : Blockchain node " << GetNode()->GetId()
                                                    << " has already received the block with height = "
                                                    << height << " and minerId = " << minerId);
                                    }
                                    else
                                    {
                                        NS_LOG_INFO("INV : Blockchain node " << GetNode()->GetId()
                                                    << " does not have the block with height = "
                                                    << height << " and minerId = " << minerId);

                                        if(m_invTimeouts.find(parsedInv) == m_invTimeouts.end())
                                        {
                                            NS_LOG_INFO("INV: Blockchain node " << GetNode()->GetId()
                                                        << " has not requested the block yet");
                                            requestBlocks.push_back(parsedInv);
                                            timeout = Simulator::Schedule(m_invTimeoutMinutes, &BlockchainNode::InvTimeoutExpired, this, parsedInv);
                                            m_invTimeouts[parsedInv] = timeout;
                                        }
                                        else
                                        {
                                            NS_LOG_INFO("INV : Blockchain node " << GetNode()->GetId()
                                                        << " has already requested the block");
                                        }

                                        m_queueInv[parsedInv].push_back(from);
                                    }
                                }

                                
                                if(!requestBlocks.empty())
                                {
                                    rapidjson::Value value;
                                    rapidjson::Value array(rapidjson::kArrayType);
                                    document.RemoveMember("inv");

                                    for(block_it = requestBlocks.begin(); block_it < requestBlocks.end(); block_it++)
                                    {
                                        value.SetString(block_it->c_str(), block_it->size(), document.GetAllocator());
                                        array.PushBack(value, document.GetAllocator());
                                    }

                                    document.AddMember("blocks", array, document.GetAllocator());

                                    SendMessage(INV, GET_HEADERS, document, from );
                                    SendMessage(INV, GET_DATA, document, from );
                                }
                            }
                            
                            break;
                        }

                        case REQUEST_TRANS:
                        {
                            NS_LOG_INFO("REQUEST_TRANS");
                            //std::cout<<"Type: " << m_protocolType <<" Node Id: "<< GetNode()->GetId() << " received request_transaction\n";
                            
                            if(m_committerType != CLIENT)
                            {
                                unsigned int j;
                                std::vector<Transaction>            requestTransactions;
                                std::vector<Transaction>::iterator  trans_it;

                                m_nodeStats->getDataReceivedBytes += m_blockchainMessageHeader + m_countBytes + document["transactions"].Size()*m_inventorySizeBytes;

                                for(j = 0; j < document["transactions"].Size(); j++)
                                {
                                    int nodeId = document["transactions"][j]["nodeId"].GetInt();
                                    int transId = document["transactions"][j]["transId"].GetInt();
                                    double timestamp = document["transactions"][j]["timestamp"].GetDouble();
                                    // bool transValidation = document["transactions"][j]["validation"].GetBool();
                                    // int transExecution = document["transactions"][j]["execution"].GetInt();
                                
                                    if(HasTransaction(nodeId, transId))
                                    {
                                        NS_LOG_INFO("REQUEST_TRANS: Blockchain node " << GetNode()->GetId()
                                                    << " has the transaction nodeID: " << nodeId
                                                    << " and transId = " << transId);
                                        //std::cout<<"Type: " << m_protocolType <<" Node Id: "<< GetNode()->GetId() << " alread received request transaction\n";
                                    }
                                    else
                                    {
                                        Transaction newTrans(nodeId, transId, timestamp);
                                        m_transaction.push_back(newTrans);
                                        //m_notValidatedTransaction.push_back(newTrans);

                                        if(m_committerType == ENDORSER)
                                        {
                                            newTrans.SetExecution(GetNode()->GetId());
                                            m_totalEndorsement++;
                                            m_meanEndorsementTime = (m_meanEndorsementTime*static_cast<double>(m_totalEndorsement-1) + (Simulator::Now().GetSeconds() - timestamp))/static_cast<double>(m_totalEndorsement);
                                            ExecuteTransaction(newTrans, InetSocketAddress::ConvertFrom(from).GetIpv4());
                                        }
                                        else
                                        {
                                            AdvertiseNewTransaction(newTrans, REQUEST_TRANS, InetSocketAddress::ConvertFrom(from).GetIpv4());
                                        }
                                    }
                                    
                                }
                            }
                            
                            break;
                        }
                        case REPLY_TRANS: 
                        {
                            NS_LOG_INFO("REPLY_TRANS");
                            unsigned int j;
                            std::vector<Transaction>::iterator  trans_it;
                            m_nodeStats->getDataReceivedBytes += m_blockchainMessageHeader + m_countBytes + document["transactions"].Size()*m_inventorySizeBytes;
                            
                            for(j = 0; j < document["transactions"].Size(); j++) {
                                int nodeId = document["transactions"][j]["nodeId"].GetInt();
                                int transId = document["transactions"][j]["transId"].GetInt();
                                double timestamp = document["transactions"][j]["timestamp"].GetDouble();
                                // bool transValidation = document["transactions"][j]["validation"].GetBool();
                                int transExecution = document["transactions"][j]["execution"].GetInt();

                                if(HasReplyTransaction(nodeId, transId, transExecution))
                                {
                                    NS_LOG_INFO("REPLY_TRANS: Blockchain node " << GetNode()->GetId()
                                                << " has the reply_transaction nodeID: " << nodeId
                                                << " and transId = " << transId);
                                }
                                else if(!HasReplyTransaction(nodeId, transId, transExecution) && (int) GetNode()->GetId() != nodeId)
                                {
                                    Transaction newTrans(nodeId, transId, timestamp);
                                    newTrans.SetExecution(transExecution);
                                }
                                else if(!HasReplyTransaction(nodeId, transId, transExecution) && (int) GetNode()->GetId() == nodeId)
                                {
                                    // Not Implemented
                                }
                                else 
                                {
                                    // Not Implemented
                                }
                            }
                        }
                        default:
                        {
                            NS_LOG_INFO("Default");
                            break;
                        }

                    }
                }
            }
        }
    }

    void BlockchainNode::AdvertiseNewBlock(const Block &newBlock ) {
        NS_LOG_FUNCTION(this);
        rapidjson::Document document;
        rapidjson::Value value;
        rapidjson::Value array(rapidjson::kArrayType);

        std::ostringstream stringStream;
        std::string blockHash = stringStream.str();

        document.SetObject();

        value.SetString("blocks");
        document.AddMember("type", value, document.GetAllocator());

        if(m_protocolType == STANDARD_PROTOCOL) {
            value = INV;
            document.AddMember("message", value, document.GetAllocator());

            stringStream << newBlock.GetBlockHeight() << "/" << newBlock.GetMinerId();
            blockHash = stringStream.str();

            value.SetString(blockHash.c_str(), blockHash.size(), document.GetAllocator());
            array.PushBack(value, document.GetAllocator());
            document.AddMember("inv", array, document.GetAllocator());
        }

        rapidjson::StringBuffer packetInfo;
        rapidjson::Writer<rapidjson::StringBuffer> writer(packetInfo);
        document.Accept(writer);
        for(std::vector<Ipv4Address>::const_iterator i = m_peersAddresses.begin() ; i != m_peersAddresses.end(); ++i) {
            if(*i != newBlock.GetReceivedFromIpv4()) {
                const uint8_t delimiter[] = "#";
                m_peersSockets[*i]->Send(reinterpret_cast<const uint8_t*>(packetInfo.GetString()), packetInfo.GetSize(), 0);
                m_peersSockets[*i]->Send(delimiter, 1, 0);

                if(m_protocolType == STANDARD_PROTOCOL)
                    m_nodeStats->invSentBytes += m_blockchainMessageHeader + m_countBytes + document["inv"].Size()*m_inventorySizeBytes;

                NS_LOG_INFO("AdvertiseNewBlock: At time " << Simulator::Now().GetSeconds()
                            << "s blockchain node " << GetNode()->GetId() << " advertised a new block to " << *i);
            }
        }
    }

    void BlockchainNode::AdvertiseNewTransaction(const Transaction &newTrans, enum Messages megType, Ipv4Address receivedFromIpv4) {
        NS_LOG_FUNCTION(this);
    }

    bool BlockchainNode::HasTransaction(int nodeId, int transId) {
        for(auto const &transaction: m_transaction) {
            if(transaction.GetTransactionNodeId() == nodeId && transaction.GetTransactionId() == transId)
                return true;
        }
        return false;
    }

    bool BlockchainNode::HasReplyTransaction(int nodeId, int transId, int transExecution) {
        for(auto const &transaction: m_replyTransaction) {
            if(transaction.GetTransactionNodeId() == nodeId && transaction.GetTransactionId() == transId && transaction.GetExecution() == transExecution)
                return true;
        }
        return false;
    }

    bool BlockchainNode::HasMessageTransaction(int nodeId, int transId) {
        for(auto const &transaction: m_msgTransaction) {
            if(transaction.GetTransactionNodeId() == nodeId && transaction.GetTransactionId() == transId) {
                return true;
            }
        }
        return false;
    }

    bool BlockchainNode::HasResultTransaction(int nodeId, int transId) {
        for(auto const &transaction: m_resultTransaction) {
            if(transaction.GetTransactionNodeId() == nodeId && transaction.GetTransactionId() == transId) {
                return true;
            }
        }
        return false;
    }

    bool BlockchainNode::HasTransactionAndValidated(int nodeId, int transId) {
        for(auto const &transaction: m_transaction) {
            if(transaction.GetTransactionNodeId() == nodeId && transaction.GetTransactionId() == transId && transaction.IsValidated() == true)
                return true;
        }
        return false;
    }

}