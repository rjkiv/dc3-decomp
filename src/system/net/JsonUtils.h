#pragma once
#include "net/json-c/json_object.h"
#include "types.h"
#include "utl/Str.h"

class JsonObject {
public:
    enum EType { // From RB3
        kType_Null = 0,
        kType_Boolean = 1,
        kType_Double = 2,
        kType_Int = 3,
        kType_Object = 4,
        kType_Array = 5,
        kType_String = 6
    };

    JsonObject() : m_pJsonObject(nullptr) {}
    virtual ~JsonObject() { Release(); }

    lh_table *Get() const { return json_object_get_object(m_pJsonObject); }
    void Set(json_object *o) { m_pJsonObject = o; }
    void AddRef() { json_object_get(m_pJsonObject); }
    void Release() { json_object_put(m_pJsonObject); }

    EType GetType() const;
    char const *Str() const;
    bool Bool() const;
    int Int() const;

protected:
    json_object *m_pJsonObject; // 0x4
};

class JsonArray : public JsonObject {
    friend class JsonConverter;

private:
    JsonArray();
    virtual ~JsonArray();

    json_object *operator[](int idx) {
        return json_object_array_get_idx(m_pJsonObject, idx);
    }

public:
    int GetSize() const;
};

class JsonConverter : public JsonArray {
public:
    JsonConverter();
    virtual ~JsonConverter();

    JsonObject *LoadFromString(String const &);
    JsonObject *GetValue(JsonArray *, int);
    const char *Str(JsonArray *, int);
    JsonObject *GetByName(JsonObject *, char const *);
    void PushObject(JsonObject *);

private:
    std::vector<JsonObject *> m_Objects; // 0x8
};
