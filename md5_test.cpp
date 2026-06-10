#include <iostream>
#include <string>
#include "e:\habsolution\src\Utility\Hash.cpp"
#include "e:\habsolution\src\Utility\StringUtility.cpp"

int main()
{
    std::string id1 = "[[assembly:/common/pc.layoutconfig].pc_layoutdef](7221).";
    Hash::MD5Hash md5Hash1 = Hash::MD5(id1);
    std::cout << "7221: " << Hash::ConvertMD5ToString(md5Hash1) << std::endl;

    std::string id2 = "[[assembly:/common/pc.layoutconfig].pc_layoutdef](0060).";
    Hash::MD5Hash md5Hash2 = Hash::MD5(id2);
    std::cout << "0060: " << Hash::ConvertMD5ToString(md5Hash2) << std::endl;
    
    return 0;
}
