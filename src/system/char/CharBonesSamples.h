#pragma once
#include "char/CharBones.h"
#include "obj/Object.h"
#include "utl/BinStream.h"
#include "utl/MemMgr.h"

class CharBonesSamples : public CharBones {
public:
    CharBonesSamples();
    virtual ~CharBonesSamples();
    virtual void Print();
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);

    void Clone(const CharBonesSamples &b);
    void EvaluateChannel(void *dst, int offset, int sample, float frac);
    void ScaleAddSample(CharBones &bones, float weight, int sample, float frac);
    void Relativize(CharClip *clip);
    int FracToSample(float *frac) const;
    void RotateBy(CharBones &bones, int sample);
    void RotateTo(CharBones &bones, float weight, int sample, float frac);
    void
    Set(const std::vector<CharBones::Bone> &bones,
        int numSamples,
        CharBones::CompressionType compression);
    void Save(BinStream &d);
    void Load(BinStream &d);
    void LoadHeader(BinStreamRev &d);
    void LoadData(BinStreamRev &d);

    // public: void CopySubSamples(const CharBonesSamples&, int32_t, int32_t);
    // public: void Transfer(CharBonesSamples&);
    // public: void CopyInto(CharBonesSamples&);
    // public: void InsertBones(const stlpmtx_std::vector<CharBones::Bone>&);
    // public: void RemoveBones(const stlpmtx_std::vector<CharBones::Bone>&);
    // public: bool KeyFrameCompressed() const;

    const char *RawData() const { return mRawData; }
    char *RawData() { return mRawData; }
    void SetNumSamples(int num) { mNumSamples = num; }
    int NumSamples() const { return mNumSamples; }
    int AllocateSize();
    void SetSamplePointers(int idx) { mStart = mRawData + mTotalSize * idx; }
    // public: void DeleteSamples(int32_t, int32_t);
    // public: void SetKeyFrame(int32_t, ObjectDir*);
    // public: void Groom();
    // public: void Relativize(CharClip*);
    // public: void ChangeCompression(CharBones::CompressionType);

    const std::vector<float> &Frames() const { return mFrames; }
    void SetKeyFrames(const std::vector<float> &frames) { mFrames = frames; }

protected:
    void ReadCounts(BinStream &d, int count);
    void SetPreview(int sample);

    /** "how many keyframes" */
    int mNumSamples; // 0x54
    /** "which sample to preview" */
    int mPreviewSample; // 0x58
    char *mRawData; // 0x5c
    /** "which sample to play" */
    std::vector<float> mFrames; // 0x60
};
