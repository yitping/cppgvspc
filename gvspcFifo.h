//
//  gvspcFifo.h
//  cppgvspc
//
//  Created by Yitping Kok on 7/10/14.
//  Copyright (c) 2014 MPE. All rights reserved.
//

#ifndef cppgvspc_gvspcFifo_h
#define cppgvspc_gvspcFifo_h

template <typename T>
class gvspcFifo
{
	std::vector<T> q;
	int size_limit;

public:
	gvspcFifo();
	gvspcFifo(int s);
	~gvspcFifo();
	
	void limit(int s);
	void add(T qi);
	int size();
	int isEmpty();
	const T& operator[](int i) const;
	const T& last() const;
	
};

template <typename T>
gvspcFifo<T>::gvspcFifo()
{
	std::cout << "empty gvspcFifo of type " << typeid(T).name() << " created." << std::endl;
}

template <typename T>
gvspcFifo<T>::gvspcFifo(int s)
{
	limit(s);
	std::cout << "gvspcFifo of type " << typeid(T).name() << " created." << std::endl;
}

template <typename T>
gvspcFifo<T>::~gvspcFifo()
{
	std::cout << "1 gvspcFifo of type " << typeid(T).name() << " destroyed." << std::endl;
}

template <typename T>
void gvspcFifo<T>::limit(int s) { size_limit = s; }

template <typename T>
void gvspcFifo<T>::add(T qi)
{
	if (q.size() == size_limit) q.erase(q.begin());
	q.push_back(qi);
	std::cout << "new size = " << q.size() << std::endl;
}

template <typename T>
int gvspcFifo<T>::size() { return q.size(); }

template <typename T>
int gvspcFifo<T>::isEmpty() { return q.empty(); }

template <typename T>
const T& gvspcFifo<T>::operator[](int i) const { return q[i]; }

template <typename T>
const T& gvspcFifo<T>::last() const { return q.back(); }

#endif
