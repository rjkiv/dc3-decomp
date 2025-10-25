#include "net/JsonUtils.h"
#include "json-c/json_object_private.h"
#include "net/json-c/json_object.h"
#include "net/json-c/json_tokener.h"
#include "net/json-c/printbuf.h"
#include "os/Debug.h"

#pragma region JsonObject

JsonObject::JsonObject() {}

JsonObject::~JsonObject() {}

JsonObject::EType JsonObject::GetType() const {
    if (mObject != nullptr)
        return (JsonObject::EType)json_object_get_type(mObject);
    return kType_Null;
}

char const *JsonObject::Str() const {
    MILO_ASSERT(GetType() == kType_String, 0x20);
    return json_object_get_string(mObject);
}

bool JsonObject::Bool() const {
    MILO_ASSERT(GetType() == kType_Boolean, 0x26);
    return json_object_get_boolean(mObject);
}

int JsonObject::Int() const {
    MILO_ASSERT(GetType() == kType_Int, 0x2c);
    return json_object_get_int(mObject);
}

#pragma endregion JsonObject
#pragma region JsonArray

JsonArray::JsonArray() : unk4() { unk4 = json_object_new_array(); }

JsonArray::~JsonArray() {
    for (int i = 0; i < json_object_array_length(unk4); i++) {
        // json_object_array_entry_free(json_object_array_get_idx(unk4, i));
    }
    // json_object_array_entry_free(unk4);
}

int JsonArray::GetSize() const { return json_object_array_length(unk4); }

#pragma endregion JsonArray
#pragma region JsonConverter

JsonConverter::JsonConverter() {}

JsonConverter::~JsonConverter() {
    if (!objects.empty()) {
        int count = objects.size() - 1;
        while (count >= 0) {
            JsonObject *o = objects[count];
            delete o;
            count--;
        }
    }
}

JsonObject *JsonConverter::LoadFromString(String const &str) { return 0; }

JsonObject *JsonConverter::GetValue(JsonArray *inArray, int inIdx) {
    MILO_ASSERT(0 <= inIdx && inIdx <= inArray->GetSize(), 0x10a);
    JsonObject *obj = new JsonObject();
    obj->mObject = json_object_array_get_idx(inArray->unk4, inIdx);
    json_object_get(obj->mObject);
    PushObject(obj);
    return obj;
}

char const *JsonConverter::Str(JsonArray *j, int i) { return GetValue(j, i)->Str(); }

JsonObject *JsonConverter::GetByName(JsonObject *j, char const *i) {
    if (j->GetType() == JsonObject::kType_Object) {
    }
    return nullptr;
}

void JsonConverter::PushObject(JsonObject *obj) {
    json_object_get(obj->mObject);
    objects.push_back(obj);
}

#pragma endregion JsonConverter
