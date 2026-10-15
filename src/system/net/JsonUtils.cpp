#include "net/JsonUtils.h"
#include "json-c/json_object_private.h"
#include "net/json-c/json_object.h"
#include "net/json-c/json_tokener.h"
#include "net/json-c/linkhash.h"
#include "net/json-c/printbuf.h"
#include "os/Debug.h"

#pragma region JsonObject

JsonObject::EType JsonObject::GetType() const {
    if (m_pJsonObject)
        return (JsonObject::EType)json_object_get_type(m_pJsonObject);
    else
        return kType_Null;
}

char const *JsonObject::Str() const {
    MILO_ASSERT(GetType() == kType_String, 0x20);
    return json_object_get_string(m_pJsonObject);
}

bool JsonObject::Bool() const {
    MILO_ASSERT(GetType() == kType_Boolean, 0x26);
    return json_object_get_boolean(m_pJsonObject);
}

int JsonObject::Int() const {
    MILO_ASSERT(GetType() == kType_Int, 0x2c);
    return json_object_get_int(m_pJsonObject);
}

#pragma endregion JsonObject
#pragma region JsonArray

JsonArray::JsonArray() { m_pJsonObject = json_object_new_array(); }

JsonArray::~JsonArray() {
    for (int i = json_object_array_length(m_pJsonObject) - 1; i >= 0; i--) {
        json_object_put(json_object_array_get_idx(m_pJsonObject, i));
    }
}

int JsonArray::GetSize() const { return json_object_array_length(m_pJsonObject); }

#pragma endregion JsonArray
#pragma region JsonConverter

JsonConverter::JsonConverter() {}

JsonConverter::~JsonConverter() {
    if (m_Objects.size() > 0) {
        for (int i = m_Objects.size() - 1; i >= 0; i--) {
            m_Objects[i]->Release();
            delete m_Objects[i];
        }
    }
}

JsonObject *JsonConverter::LoadFromString(const String &str) {
    printbuf *buf = printbuf_new();
    if (!buf) {
        return nullptr;
    } else {
        printbuf_memappend(buf, str.c_str(), str.length());
        json_object *obj = json_tokener_parse(buf->buf);
        if ((int)obj > 0xfffff060) { // ???
            printbuf_free(buf);
            return nullptr;
        } else {
            JsonObject *jObj = new JsonObject();
            jObj->Set(obj);
            printbuf_free(buf);
            jObj->AddRef();
            m_Objects.push_back(jObj);
            return jObj;
        }
    }
}

JsonObject *JsonConverter::GetValue(JsonArray *inArray, int inIdx) {
    MILO_ASSERT(0 <= inIdx && inIdx <= inArray->GetSize(), 0x10a);
    JsonObject *obj = new JsonObject();
    obj->Set((*inArray)[inIdx]);
    obj->AddRef();
    PushObject(obj);
    return obj;
}

const char *JsonConverter::Str(JsonArray *j, int i) { return GetValue(j, i)->Str(); }

JsonObject *JsonConverter::GetByName(JsonObject *j, const char *cc) {
    if (j->GetType() != JsonObject::kType_Object) {
        return nullptr;
    } else {
        lh_table *lh = j->Get();
        const void *v = lh_table_lookup(lh, cc);
        if (v) {
            json_object *obj = (json_object *)v;
            json_object_get(obj);
            JsonObject *jObj = new JsonObject();
            jObj->Set(obj);
            jObj->AddRef();
            m_Objects.push_back(jObj);
            return jObj;
        } else {
            return nullptr;
        }
    }
}

void JsonConverter::PushObject(JsonObject *obj) {
    obj->AddRef();
    m_Objects.push_back(obj);
}

#pragma endregion JsonConverter
