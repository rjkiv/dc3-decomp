#include "meta/StorePurchaser.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "ui/UI.h"
#include "utl/Symbol.h"


StorePurchaser::~StorePurchaser(){}

XboxPurchaser::XboxPurchaser(int param1, u64 param2, u64 param3, u64 param4, Symbol s, unsigned int ui) 
    : unk38(0), unk40(param2), unk48(param1){  


    }

XboxPurchaser::~XboxPurchaser(){

}

void XboxPurchaser::Initiate(){

}

bool XboxPurchaser::IsSuccess() const{
    return false;
}

bool XboxPurchaser::PurchaseMade() const{
    return false;
}

bool XboxPurchaser::IsPurchasing() const{
    return false;
}

DataNode XboxPurchaser::OnMsg(UIChangedMsg const &){
    return NULL_OBJ;
}

bool XboxMultipleItemsPurchaser::IsSuccess() const{
    return false;
}

bool XboxMultipleItemsPurchaser::PurchaseMade() const{
    return false;
}

bool XboxMultipleItemsPurchaser::IsPurchasing() const{
    return false;
}

void XboxMultipleItemsPurchaser::Initiate(){

}

XboxMultipleItemsPurchaser::~XboxMultipleItemsPurchaser(){
    Symbol ui_changed("ui_changed");
}

XboxMultipleItemsPurchaser::XboxMultipleItemsPurchaser(int, std::vector<u64>, Symbol, unsigned int){
    
}

DataNode XboxMultipleItemsPurchaser::OnMsg(UIChangedMsg const &){
    return NULL_OBJ;
}
