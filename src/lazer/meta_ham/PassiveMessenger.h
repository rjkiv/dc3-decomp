#pragma once
#include "obj/Data.h"
#include "obj/Object.h"
#include "obj/PropSync.h"
#include "utl/Symbol.h"
#include <list>

enum PassiveMessageType {
    kPassiveMessageGeneral = 0,
    kPassiveMessageFitness = 1,
    kPassiveMessageUnlock = 2,
    kPassiveMessageServer = 3,
    kPassiveMessageCampaign = 4,
    kPassiveMessageFree4All = 5
};

class PassiveMessage {
public:
    PassiveMessage(String text, PassiveMessageType type, Symbol channel, int);
    virtual ~PassiveMessage();
    String GetText();
    PassiveMessageType GetMessageType() { return mMessageType; }
    Symbol GetChannel() { return mChannel; }
    int GetPriority() { return mPriority; }

private:
    String mText; // 0x4
    PassiveMessageType mMessageType; // 0xc
    Symbol mChannel; // 0x10
    int mPriority; // 0x14
};

class PassiveMessageQueue {
public:
    PassiveMessageQueue(Hmx::Object *callback, Symbol);
    virtual ~PassiveMessageQueue();
    virtual PassiveMessage *GetAndPreProcessFirstMessage();
    virtual void AddMessage(PassiveMessage *);

    bool HasRecentlyDismissedMessage() const;
    bool RemoveLowerPriorityMessage(PassiveMessage *);
    void ClearRunningMessage();
    void Poll();

    int GetNumMessages() const { return mQueue.size(); }
    bool Running() const { return mTimer.Running(); }

protected:
    void ClearPassiveMessage();
    void HandlePassiveMessage(PassiveMessage *);

    float mMessageDuration; // 0x8
    float mLastMessageClearTime; // 0xc - seconds?
    std::list<PassiveMessage *> mQueue; // 0x10

private:
    Hmx::Object *mCallback; // 0x18
    Symbol mTarget; // 0x1c
    Timer mTimer; // 0x20
};

class PassiveMessenger : public Hmx::Object {
public:
    PassiveMessenger(Hmx::Object *callback);
    virtual ~PassiveMessenger();
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);

    bool HasRecentlyDismissedMessage() const;
    void TriggerGenericMsg(Symbol, Symbol, PassiveMessageType, Symbol, int);
    void TriggerStringMsg(String, Symbol, PassiveMessageType, Symbol, int);
    void Poll();
    bool HasMessages() const;

private:
    bool AreSideMessagesAllowed() const;
    bool AreGlobalMessagesAllowed() const;
    void TriggerMessage(String, PassiveMessageType, Symbol, Symbol, int);

    PassiveMessageQueue *mMessageQueueP1; // 0x2c
    PassiveMessageQueue *mMessageQueueP2; // 0x30
    PassiveMessageQueue *mMessageQueueNone; // 0x34
    bool mEnabled; // 0x38
};

extern PassiveMessenger *ThePassiveMessenger;
