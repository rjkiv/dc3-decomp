#ifdef __cplusplus
extern "C" {
#endif
void *RtlAllocateHeap(void *Handle, unsigned long Flags, unsigned int Size);
unsigned int RtlFreeHeap(void *Handle, unsigned long Flags, void *BaseAddress);
unsigned int RtlSizeHeap(void *Handle, unsigned long Flags, void *MemoryPointer);
#ifdef __cplusplus
}
#endif