#ifndef TRANSACTION_H
#define TRANSACTION_H

namespace ns3 {
    class Transaction {
        public:
            Transaction(int nodeId, int transId, double timeStamp);
            Transaction();
            virtual ~Transaction(void);
            int GetTransactionNodeId(void) const;
            void SetTransactionNodeId(int nodeId);

            int GetTransactionId(void) const;
            void SetTransactionId(int id);

            int GetTransSizeByte(void) const;
            void SetTransSizeByte(int transSizeByte);

            double GetTransTimeStamp(void) const;
            void SetTransTimeStamp(double timeStamp);

            bool IsValidated(void) const;
            void SetValidation();

            int GetExecution(void) const;
            void SetExecution(int endoerserId);

            Transaction& operator = (const Transaction &transSource);

            friend bool operator == (const Transaction &trans1, const Transaction &trans2);

        protected:
            int m_nodeId;
            int m_transId;
            int m_transSizeByte;
            double m_timeStamp;
            bool m_validatation; 
            int m_execution;
    };
}

#endif