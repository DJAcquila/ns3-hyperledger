#include "transaction.h"

namespace ns3 {

    Transaction::Transaction(int nodeId, int transId, double timeStamp) {
        m_nodeId = nodeId;
        m_transId = transId;
        m_timeStamp = timeStamp;
        m_transSizeByte = 100;
        m_validatation = false;
        m_execution = 0;
    }

    Transaction::Transaction() {
        Transaction(0,0,0);
    }

    Transaction::~Transaction(){}

    int Transaction::GetTransactionNodeId(void) const {
        return m_nodeId;
    }

    void Transaction::SetTransactionNodeId(int nodeId) {
        m_nodeId = nodeId;
    }

    int Transaction::GetTransactionId(void) const {
        return m_transId;
    }

    void Transaction::SetTransactionId(int id) {
        m_transId = id;
    }

    int Transaction::GetTransSizeByte(void) const {
        return m_transSizeByte;
    }
    void Transaction::SetTransSizeByte(int transSizeByte) {
        m_transSizeByte = transSizeByte;
    }

    double Transaction::GetTransTimeStamp(void) const {
        return m_timeStamp;
    }
    void Transaction::SetTransTimeStamp(double timeStamp) {
        m_timeStamp = timeStamp;
    }

    bool Transaction::IsValidated(void) const {
        return m_validatation;
    }

    void Transaction::SetValidation() {
        m_validatation = true;
    }

    int Transaction::GetExecution(void) const {
        return m_execution;
    }
    void Transaction::SetExecution(int endoerserId) {
        m_execution = endoerserId;
    }

    Transaction& Transaction::operator = (const Transaction &transSource) {
        m_nodeId = transSource.m_nodeId;
        m_transId = transSource.m_transId;
        m_timeStamp = transSource.m_timeStamp;
        m_transSizeByte = transSource.m_transSizeByte;
        m_validatation = transSource.m_validatation;
        m_execution = transSource.m_execution;

        return *this;
    }

    bool operator == (const Transaction &trans1, const Transaction &trans2) {
        if(trans1.GetTransactionNodeId() == trans2.GetTransactionNodeId() && trans1.GetTransactionId() == trans2.GetTransactionId()) {
            return true;
        } else {
            return false;
        }
    }
}