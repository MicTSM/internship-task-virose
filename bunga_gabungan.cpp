#include <iostream>
using namespace std;

int main(){
    int p, q, a, b;
    cin >> p;
    cin >> q;
    a=(p*p+(q*q)+1);
    b=(a/4);
    if (a%4==0) {
        cout << b;
    }
    else cout << -1;
return 0;
}