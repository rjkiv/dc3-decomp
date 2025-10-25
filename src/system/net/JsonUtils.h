#pragma once
#include "net/json-c/json_object.h"
#include "net/json-c/json_object_private.h"
#include "stl/_vector.h"
#include "types.h"
#include "utl/Str.h"

class JsonObject {
    enum EType { // From RB3
        kType_Null = 0,
        kType_Boolean = 1,
        kType_Double = 2,
        kType_Int = 3,
        kType_Object = 4,
        kType_Array = 5,
        kType_String = 6
    };

public:
    JsonObject();
    ~JsonObject();
    JsonObject::EType GetType() const;
    char const *Str() const;
    bool Bool() const;
    int Int() const;

    friend class JsonConverter;

    u32 unk0;
    json_object *mObject;
};

class JsonArray {
public:
    int GetSize() const;

    json_object *unk4;

    friend class JsonConverter;

private:
    virtual ~JsonArray();
    JsonArray();
};

class JsonConverter : public JsonArray {
public:
    virtual ~JsonConverter();

    JsonConverter();
    JsonObject *LoadFromString(String const &);
    JsonObject *GetValue(JsonArray *, int);
    char const *Str(JsonArray *, int);
    JsonObject *GetByName(JsonObject *, char const *);
    void PushObject(JsonObject *);

    std::vector<JsonObject *> objects;
};
