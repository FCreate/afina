//
// Created by micheal on 01.03.18.
//
#include <string>
template <class ElemType>
struct Element
{
public:
    ElemType data;
    Element *next;
    Element *pred;
    std::string key;
};

template <class ElemType>
class list
{
private:
    Element<ElemType> *pHead;
    Element<ElemType> *pEnd;
    int countElem;
public:

    list()
    {
        pHead = nullptr;
        pEnd = nullptr;
        countElem = 0;
    }
    list(const list &src)
    {
        Element<ElemType> *temp=src.pHead;
        countElem =0;
        for (int i=0;i<src.countElem;i++)
        {
            this->push_back(temp->data);
            temp=temp->next;
        }
    }

    ~list()
    {
        clear();
    }

    Element<ElemType>* front(){
        return pHead;
    }
    Element<ElemType>* end(){
        return pEnd;
    }
    void push_back(ElemType data)
    {
        auto *temp = new Element<ElemType>;
        temp->data = data;
        if (countElem==0)
        {
            pHead=pEnd=temp;
            temp->pred=nullptr;
        }
        else{
            pEnd->next=temp;
            temp->pred = pEnd;
        }
        temp->next = nullptr;
        pEnd = temp;
        countElem++;
    }
    int size ()
    {
        return countElem;
    }
    void set_end (Element<ElemType>* elem) {
        //put to the end
        if (elem == pEnd) {
            return;
        }
        else if (elem == pHead) {

            pHead = elem->next;
            pHead->pred = nullptr;

            elem->pred=pEnd;
            elem->next= nullptr;
            pEnd->next= elem;
            pEnd = elem;
        }
        else {
            return;
        }
    }
    void remove(Element<ElemType>* elem){
        if (countElem==0){
            return;
        }
        if (countElem==1){
            countElem--;
            pHead=pEnd= nullptr;
            delete elem;
            return;
        }
        if (elem==pEnd){
            pEnd = pEnd->pred;
            pEnd->next = nullptr;
            countElem--;
            delete elem;
            return;
        }
        if (elem==pHead){
            pHead=pHead->next;
            pHead->pred = nullptr;
            countElem--;
            delete elem;
            return;
        }
        else{
            elem->pred->next = elem->next;
            elem->next->pred = elem->pred;
            countElem--;
            delete elem;
            return;
        }
    }
    void clear()
    {
        Element<ElemType> *pTemp = pHead;
        while(pTemp)
        {
            Element<ElemType> *nextnode = pTemp->next;
            delete pTemp;
            pTemp = nextnode;
        }

        pHead = nullptr;
        pEnd = nullptr;
    }
};
