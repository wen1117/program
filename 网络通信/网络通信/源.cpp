#include<iostream>
using namespace std;

int main() {
	int a = 0;
	for (int i = 0; i < 11; i++) {
		a += i;
//#ifdef _DEBUG
//		if (i==5)
//		{
//			printf("debug: i=5 , a=%d\n", a);
//
//		}
//#endif // DEBUG
		 
	}
	cout << a << endl;
	system("pause");
	return 0;
}
