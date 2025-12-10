#include <iostream>

void print(std::string_view row){
    std::cout << row;
}

int main(){
    print("piska popka");
    return 0;
}