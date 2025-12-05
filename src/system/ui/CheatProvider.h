#pragma once
#include "ui/UIListProvider.h"
#include "obj/Object.h"

/**
 * @brief Handles data for the debug cheat menu.
 */
class CheatProvider : public UIListProvider, public Hmx::Object {
public:
    /**
     * @brief Handles individual entries in cheats.lst
     */
    struct Cheat {
        Cheat(const char *);
        Cheat(String &, String &, DataArray *);
        /** The key(s) to press to activate this cheat. */
        String mKey; // 0x0
        /** A quick description of what this cheat does. */
        String mDesc; // 0x8
        /** The script performing this cheat. */
        DataArray *mScript; // 0x10
    };

    CheatProvider();
    virtual void Text(int, int, UIListLabel *, UILabel *) const;
    virtual int NumData() const;
    virtual bool IsActive(int) const;
    virtual void InitData(RndDir *);
    virtual DataNode Handle(DataArray *, bool);

    /** Invoke a cheat.
     * @param [in] idx The index in mFilterCheats of the cheat to invoke.
     * @param [in] user The user to which the cheat should apply.
     */
    void Invoke(int idx, LocalUser *user);

    static void Init();
    static void Terminate();
    static CheatProvider *sInstance;

private:
    /** Set the current filter in place to the next available filter. */
    void NextFilter();
    /** Update the filtered cheats vector based on the current filter. */
    void ApplyFilter();

    /** The complete series of cheats. */
    std::vector<Cheat> mCheats; // 0x30
    /** The series of filtered cheats. */
    std::vector<Cheat> mFilterCheats; // 0x3c
    /** The series of filters (places where certain cheats are only allowed to work, i.e.
     * game, meta, store, writer, all) */
    std::vector<Symbol> mFilters; // 0x48
    /** The current filter being applied. */
    int mFilterIdx; // 0x54
};
