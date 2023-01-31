#include <iostream>
#include <stdio.h>

using namespace std;


int main(){
    int x,y; 
    cout << "Input X:";
    cin >> x;
    cout << "Input Y:";
    cin >> y;
    if (x>0 && y>0){cout <<"The 1st quadrant";
    } else if (x<0 && y>0){ cout <<"The 2nd quadrant";
    } else if (x<0 && y<0){ cout <<"The 3rd quadrant";
    } else if (x>0 && y<0){ cout <<"The 4th quadrant";
    } else if (x!=0 && y==0){ cout <<"On the X-axis";
    } else if (x==0 && y!=0){ cout <<"On the Y-axis";
    } else if (x==0 && y==0){ cout <<"On the origin";
    }
    
}