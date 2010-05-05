/****************************************************************
*                                                               *
*    Name: MyList.h                                             *
*                                                               *
*    Creator: Balazs Tuska                                      *
*                                                               *
*    Description:                                               *
*       This file descibe my chainlist. This class is very      *
*       similar to the built in vector class, but it is mine.   *
*                                                               *
****************************************************************/
//---------------------------------------------------------------------------

#ifndef MyListH
#define MyListH
//---------------------------------------------------------------------------
template<class T>
class CMyList
{
public:
    struct element {
        T data;
        element *next;
        element *prev;
    };
private:
    element *last;
    element *first;
    int len;

public:

CMyList()
{
    last=0;
    first=0;
    len=0;
}

void delList()
{
    if( !last || !first ) {
        last=0;
        first=0;
        len=0;
        return;
    }

    element *delthis=first;
    element *delnext=delthis->next;
    while( delthis ) {
        delnext=delthis->next;
        delete delthis;
        delthis=delnext;
    }
    last=0;
    first=0;
    len=0;
}

void addFirst(T newdata)
{
    element *addthis=new element;
    addthis->next=first;
    addthis->prev=0;
    addthis->data=newdata;
    if( first )
        first->prev=addthis;
    first=addthis;
    if( !last ) last=addthis;
    len++;
}

void addLast(T newdata)
{
    element *addthis=new element;
    addthis->next=0;
    addthis->prev=last;
    addthis->data=newdata;
    if( last )
        last->next=addthis;
    last=addthis;
    if( !first ) first=addthis;
    len++;
}

void push_back(T newdata)
{
    element *addthis=new element;
    addthis->next=0;
    addthis->prev=last;
    addthis->data=newdata;
    if( last )
        last->next=addthis;
    last=addthis;
    if( !first ) first=addthis;
    len++;
}

T getFirst()
{
    if( !first ) return T();
    return first->data;
}

T getLast()
{
    if( !last ) return T();
    return last->data;
}

void delFirst()
{
    if( !first ) return;
    element *delthis=first;
    first=first->next;
    if( first )
        first->prev=0;
    else
        last=0;
    delete delthis;
    len--;
}

void delLast()
{
    if( !last ) return;
    element *delthis=last;
    last=last->prev;
    if( last )
        last->next=0;
    else
        first=0;
    delete delthis;
    len--;
}

T removeFirst()
{
    T ret = getFirst();
    delFirst();
    return ret;
}

T removeLast()
{
    T ret = getLast();
    delLast();
    return ret;
}

T get(int index=0)
{
    if( !first ) return T();
    element *searchthis=first;
    int i=0;
    while( i<index && searchthis!=last ) {
        searchthis=searchthis->next;
        i++;
    }
    return searchthis->data;
}

void del(int index=0)
{
    if( !first ) return;
    element *searchthis=first;
    int i=0;
    while( i<index && searchthis!=last ) {
        searchthis=searchthis->next;
        i++;
    }

    if( !searchthis ) return;
    if( searchthis->next )
        searchthis->next->prev=searchthis->prev;
    else
        last=searchthis->prev;
    if( searchthis->prev )
        searchthis->prev->next=searchthis->next;
    else
        first=searchthis->next;
    delete searchthis;
    len--;
}

void del(element* searchthis)
{
    if( !first ) return;

    if( !searchthis ) return;
    if( searchthis->next )
        searchthis->next->prev=searchthis->prev;
    else
        last=searchthis->prev;
    if( searchthis->prev )
        searchthis->prev->next=searchthis->next;
    else
        first=searchthis->next;
    delete searchthis;
    len--;
}

bool isNull()
{
    return first==0 && last==0;
}

int length()
{
    return len;
}

int size()
{
    return len;
}

T operator[](int index) const
{
    return get(index);
}


T& operator[](int index)
{
    if( !first ) throw true;
    element *searchthis=first;
    int i=0;
    while( i<index && searchthis!=last ) {
        searchthis=searchthis->next;
        i++;
    }
    return searchthis->data;
}

element* getIterator()
{
    return first;
}

element* getEnd()
{
    return last;
}



};
#endif
