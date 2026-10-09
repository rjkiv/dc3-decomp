#pragma once
#include "xdk/win_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum _JSONTokenType {
    Json_NotStarted = 0,
    Json_BeginArray = 1,
    Json_EndArray = 2,
    Json_BeginObject = 3,
    Json_EndObject = 4,
    Json_String = 5,
    Json_Number = 6,
    Json_True = 7,
    Json_False = 8,
    Json_Null = 9,
    Json_FieldName = 10,
    Json_NameSeparator = 11,
    Json_ObjectSeparator = 12,
    Json_ValueSeparator = 13,
} JSONTokenType;

typedef struct HJSONREADER__ {
    int idk;
} HJSONREADER;

typedef struct HJSONWRITER__ {
    int idk;
} HJSONWRITER;

#ifdef __cplusplus
}
#endif

// C++ symbols
HRESULT XJSONCloseReader(HJSONREADER *reader);
HRESULT XJSONCloseWriter(HJSONWRITER *writer);
HRESULT XJSONWriteNumberValue(HJSONWRITER *writer, double nValue);
HRESULT XJSONWriteStringValue(HJSONWRITER *writer, const char *pValue, DWORD nValueChars);
HRESULT XJSONBeginArray(HJSONWRITER *writer);
HRESULT XJSONEndArray(HJSONWRITER *writer);
HRESULT XJSONWriteNullValue(HJSONWRITER *writer);
HJSONWRITER *XJSONCreateWriter();
HRESULT XJSONReadToken(
    HJSONREADER *reader, JSONTokenType *pTokenType, DWORD *pTokenLength, DWORD *pParsed
);
HRESULT XJSONGetTokenValue(HJSONREADER *reader, WCHAR *pBuffer, DWORD cb);
