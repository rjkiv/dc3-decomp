#pragma once
#include "xdk/win_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum _JSONTokenType {
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
