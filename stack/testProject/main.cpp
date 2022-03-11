#include <stdio.h>

#include "../headers/stack.h"

#include "dump/headers/dump.h"
#pragma comment(lib, "lib/dump/dump")

CDump dump;

int main(){

	CStack<int> stack(5);

	for(;;){

		bool result;
		result = stack.push(1);
		if(result == false){
			CDump::crash();
		}
		result = stack.push(1);
		if(result == false){
			CDump::crash();
		}
		result = stack.push(1);
		if(result == false){
			CDump::crash();
		}
		result = stack.push(1);
		if(result == false){
			CDump::crash();
		}
		result = stack.push(1);
		if(result == false){
			CDump::crash();
		}
		
		result = stack.pop();
		if(result == false){
			CDump::crash();
		}
		result = stack.pop();
		if(result == false){
			CDump::crash();
		}
		result = stack.pop();
		if(result == false){
			CDump::crash();
		}
		result = stack.pop();
		if(result == false){
			CDump::crash();
		}
		result = stack.pop();
		if(result == false){
			CDump::crash();
		}


		printf("size: %d\n", stack.size());
	}

	return 0;
}