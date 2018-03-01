//
// Created by micheal on 01.03.18.
//
template <class ElemType>
struct Element
{
public:
    ElemType data;
    Element *next;
    Element *pred;
};

template <class ElemType>
class list
{
private:
    Element<ElemType> *pHead;
    Element<ElemType> *pPrev;
    int countElem;
public:

    list()
    {
        pHead = nullptr;
        pPrev = nullptr;
        countElem = 0;
    }
    list(const list &src)
    {
        Element<ElemType> *temp=src.pHead;
        this->countElem=0;
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

    ElemType front(){
        return pHead->data;
    }
    void push_front (ElemType data)
    {
        Element<ElemType> *temp = new Element<ElemType>;
        pHead->pred=temp;
        temp->data=data;
        temp->next=pHead;
        temp->pred=nullptr;
        pHead=temp;
        countElem++;
    }
    void push_back(ElemType data)
    {
        Element<ElemType> *temp = new Element<ElemType>;
        temp->data = data;
        if (countElem==0)
        {
            pHead=pPrev=temp;
            temp->pred=nullptr;
        }
        else{
            pPrev->next=temp;
            temp->pred = pPrev;
        }

        temp->next = nullptr;

        pPrev = temp;
        countElem++;
    }

    void pop_front()
    {
        Element<ElemType> *pTemp = pHead;
        if (countElem==0)
        {
            return;
        }
        if (countElem == 1)
        {
            pHead = nullptr;
            pPrev = nullptr;
        }
        else
        {
            pTemp->next->pred = nullptr;
            pHead = pTemp->next;
            pHead->pred=nullptr;
        }

        countElem--;
        delete pTemp;

    }

    void pop_back()
    {
        Element<ElemType> *pTemp = pPrev;
        pTemp->pred->next = nullptr;
        pPrev = pTemp->pred;
        delete pTemp;
        countElem--;
    }

    void remove(ElemType element){
        Element<ElemType> *pTemp = pHead;
        for (int i=0; i<countElem; i++){
            if (element==pTemp->data){
                delElem(i);
                break;
            }
        }

    }

    void delElem(int numb)
    {
        Element<ElemType> *pTemp = pHead;


        if ((numb>countElem) || (numb<1))  {}
        else
        {
            for(int i=1; i!=numb; i++)
            {
                pTemp = pTemp->next;
            }

            if (pTemp->pred == nullptr)
            {
                if (countElem == 1)
                {
                    pHead = nullptr;
                    pPrev = nullptr;
                }
                else
                {
                    pTemp->next->pred = nullptr;
                    pHead = pTemp->next;
                }

                delete pTemp;
                countElem--;
                return;
            }

            if (pTemp->next == nullptr)
            {
                pTemp->pred->next = nullptr;
                pPrev = pTemp->pred;
                delete pTemp;
                countElem--;
                return;
            }

            if (pTemp->next != nullptr && pTemp->pred != nullptr)
            {
                pTemp->pred->next = pTemp->next;
                pTemp->next->pred = pTemp->pred;
                delete pTemp;
                countElem--;
                return;
            }
        }
    }

    void clear()
    {
        while(pHead!=0)
        {

            Element<ElemType> *pTemp = pHead;

            pHead = pHead->next;
            if (pHead != nullptr)
                delete pTemp;

        }

        pHead = nullptr;
        pPrev = nullptr;
    }
    int size ()
    {
        return countElem;
    }
};
