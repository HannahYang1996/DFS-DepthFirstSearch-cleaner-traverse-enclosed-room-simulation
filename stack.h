#ifndef STACK_H_
#define STACK_H_

#include <windows.h>
#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include <cmath>
#define PI 3.1415926
using namespace std;
template <class T> class stack
{
	//stack is implemented by linked list, which is composed of several nodes
    //for each "nodes", "element" stores for its values while "next" is a node pointer that points to the next node
    //"first" is the node pointer which points to the first node of the linked list

    //"size" records the size of the linked list
    //"pushback" and "popout" are the member functions as their name imply
    struct node
    {
        T element;
        node* next = NULL;
    };
    node* first;

public:
	int size;
	stack();
	void pushback(T _element);	
 	T popout();
};

#include "stack.cpp"
#endif