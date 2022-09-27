#include<iostream>
#include"CMySql.h"
#include"TcpServerMediator.h"
#include"TcpClientMediator.h"
#include"CKernel.h"

using namespace std;

int main()
{
	CKernel kernel;
	if (kernel.startServer()) {
		cout << "Server is running" << endl;
		while (1) {
			Sleep(100);
		}
	}
	else {
		cout << "start server fail" << endl;
	}

	

	system("pause");
	return 0;
}