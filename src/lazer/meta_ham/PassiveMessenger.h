#pragma once
#include "obj/Data.h"
#include "obj/Object.h"
#include "obj/PropSync.h"
#include "utl/Symbol.h"
#include <list>

enum PassiveMessageType {
    type0,
    type1,
    type2,
    type3,
    type4
};

class PassiveMessage {
public:
    virtual ~PassiveMessage();
    PassiveMessage(String, PassiveMessageType, Symbol, int);
    String GetText();

    String unk4;
    PassiveMessageType unk8;
    Symbol mChannel; // 0x10
    int unk14;
};

class PassiveMessageQueue {
public:
    virtual ~PassiveMessageQueue();
    virtual PassiveMessage *GetAndPreProcessFirstMessage();
    virtual void AddMessage(PassiveMessage *);

    PassiveMessageQueue(Hmx::Object *, Symbol);
    bool HasRecentlyDismissedMessage() const;
    bool RemoveLowerPriorityMessage(PassiveMessage *);
    void ClearRunningMessage();
    void Poll();

    float unk8;
    float unkc;
    std::list<PassiveMessage *> mQueue; // 0x10
    Hmx::Object *mCallback; // 0x18
    Symbol unk1c;
    Timer mTimer; // 0x20

protected:
    void ClearPassiveMessage();
    void HandlePassiveMessage(PassiveMessage *);
};

class PassiveMessenger : public Hmx::Object {
public:
    virtual ~PassiveMessenger();
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);

    PassiveMessenger(Hmx::Object *);
    bool HasRecentlyDismissedMessage() const;
    void TriggerGenericMsg(Symbol, Symbol, PassiveMessageType, Symbol, int);
    void TriggerStringMsg(String, Symbol, PassiveMessageType, Symbol, int);
    void Poll();
    bool HasMessages() const;

protected:
    PassiveMessageQueue *mMessageQueueP1; // 0x2c
    PassiveMessageQueue *mMessageQueueP2; // 0x30
    PassiveMessageQueue *mMessageQueueNone; // 0x34
    bool mEnabled; // 0x38

private:
    bool AreSideMessagesAllowed() const;
    bool AreGlobalMessagesAllowed() const;
    void TriggerMessage(String, PassiveMessageType, Symbol, Symbol, int);
};

extern PassiveMessenger *ThePassiveMessenger;
