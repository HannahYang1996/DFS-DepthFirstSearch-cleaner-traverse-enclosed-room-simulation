//#include "stdafx.h"
#include <windows.h>
#include <iostream>
#include "stack.h"
using namespace std;

template <class T> stack<T>:: stack()
{
	//the linked list is initialized with "first" points to NULL,
	//and "size" equals 0
	first = NULL;
	size = 0;
}

template <class T> void stack<T>::pushback(T _element)
{
	//to add a new element to the linked list,
	//a new node, "Node", is initialized at first to store the "element",
	//if "first" of the linked list is not pointing to any nodes yet,
	//make "first" points to "Node",
	//else, make "Node" points to where "first" was pointing to,
	//and modify "first" to make it points to "Node"
	//increase the size of the linked list at the end
	node* Node = new node;
	Node->element = _element;
	if (first == NULL)
	{
		first = Node;
	}
	else
	{
	 	Node->next = first;
		first = Node;
	}
	size++;
}

template <class T> T stack<T>::popout()
{
	//to pop out the top element in the linked list,
	//"Node", is initialized to point to the node which "first" currently points to,
	//then, "first" is modified to point to the next node,
	//before returning the element of "Node", decrease the size of the linked list
	node* Node = first;
	first = Node->next;
	size--;
	return Node->element;
}

