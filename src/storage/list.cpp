#include <clocale>

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
        pHead = NULL;
        pPrev = NULL;
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
    void resize(int n)
    {
        //assert(n>=1);
        if (n==countElem)
        {
            return;
        }
        else if (n<countElem)
        {
            Element<ElemType> *temp=pHead;
            for (int i=0; i<n-1; i++)
            {
                temp=temp->next;
            }
            pPrev=temp;
            temp->next=NULL;

        }
        else if (n>countElem)
        {
            ElemType elem;
            int count=countElem;
            for (int i=0; i<n-count; i++)
            {
                //cin>>elem;
                push_back(elem);
            }
        }
    }
/*list& merge (const list list, int n)
{
	Element<ElemType> *temp =pHead;
	for ( )
}
	*/
    ElemType front(){
        return pHead->data;
    }
    void push_front (ElemType data)
    {
        Element<ElemType> *temp = new Element<ElemType>; //ñîçäàåòñÿ âðåìåííûé ýëåìåíò
        pHead->pred=temp;
        temp->data=data;
        temp->next=pHead;
        temp->pred=NULL;
        pHead=temp;
        //	temp->pred = NULL;      //îáíóëÿåì óêàçàòåëü íà ïðåäøåñòâóþùèé ýëåìåíò ò.ê. åãî íåò
        //	temp->next = NULL;      //òî æå ñàìîå ñ ïîñëåäóþùèì ýëåìåíòîì
        //	pHead = temp;       //"ãîëîâà" óêàçûâàåò íà ñîçäàííûé ýëåìåíò
        //	temp->data = data;          //êîïèðóåì äàííûå
        //temp->next = NULL;  //ïîñëåäóþùåãî ýëåìåíòà íåò (äîáàâëÿåì æå â êîíåö)
        //temp->pred = pPrev;     //óêàçûâàåì íà ïðåäûäóùèé ýëåìåíò, íà êîòîðûé «íàöåëåí» pPrev
        //pPrev = temp;     //à òåïåðü õâîñòîâîé ýëåìåíò óêàçûâàåò íà ïîñëåäíèé (äîáàâëåííûé)
        countElem++;      //óâåëè÷èâàåì ñ÷åò÷èê ýëåìåíòîâ â ñïèñêå


    }
    void push_back(ElemType data)
    {
        Element<ElemType> *temp = new Element<ElemType>; //ñîçäàåòñÿ âðåìåííûé ýëåìåíò
        temp->data = data;          //êîïèðóåì äàííûå
        if (countElem==0)
        {
            pHead=pPrev=temp;
            temp->pred=NULL;
        }
        else{
            pPrev->next=temp;
            temp->pred = pPrev;     //óêàçûâàåì íà ïðåäûäóùèé ýëåìåíò, íà êîòîðûé «íàöåëåí» pPrev
        }

        temp->next = NULL;  //ïîñëåäóþùåãî ýëåìåíòà íåò (äîáàâëÿåì æå â êîíåö)

        pPrev = temp;     //à òåïåðü õâîñòîâîé ýëåìåíò óêàçûâàåò íà ïîñëåäíèé (äîáàâëåííûé)
        countElem++;      //óâåëè÷èâàåì ñ÷åò÷èê ýëåìåíòîâ â ñïèñêå




    }
    //left
    void pop_front()
    {
        Element<ElemType> *pTemp = pHead; //ñîçäàåòñÿ âðåìåííûé ýëåìåíò
        /*	pTemp=pHead;*/
        if (countElem==0)
        {
            return;
        }
        if (countElem == 1)         //åñëè ýòîò ýëåìåíò åäèíñòâåííûé
        {
            pHead = NULL;
            pPrev = NULL;
        }
        else                //åñëè îí ïåðâûé, íî íå åäèíñòâåííûé
        {
            pTemp->next->pred = NULL;
            pHead = pTemp->next;
            pHead->pred=NULL;
        }

        countElem--;
        delete pTemp;

    }
    void pop_back()
    {
        Element<ElemType> *pTemp = pPrev; //ñîçäàåòñÿ âðåìåííûé ýëåìåíò
        /*pTemp=pPrev;*/
        pTemp->pred->next = NULL;   //ïðåäûäóùèé ýëåìåíò óêàçûâàåò íà NULL
        pPrev = pTemp->pred;        //óêàçàòåëü íà ïîñëåäíèé ýëåìåíò óêàçûâàåò íà ïðåäïîñëåäíèé

        //pTemp=pPrev;
        //pTemp->nextpPrev = NULL;
        //			pTemppPrev= NULL;   //ïðåäûäóùèé ýëåìåíò óêàçûâàåò íà NULL
        //		pPrev = pTemp->pred;        //óêàçàòåëü íà ïîñëåäíèé ýëåìåíò óêàçûâàåò íà ïðåäïîñëåäíèé
        //		pPrev->next=NULL;

        delete pTemp;
        countElem--;


    }


    void addTolist(ElemType data) //ðåàëèçàöèÿ ôóíêöèè äîáàâëåíèÿ
    {
        Element<ElemType> *temp = new Element<ElemType>; //ñîçäàåòñÿ âðåìåííûé ýëåìåíò
        if(pHead == NULL)           //åñëè ýòî ïåðâûé ýëåìåíò, òî
        {
            temp->pred = NULL;      //îáíóëÿåì óêàçàòåëü íà ïðåäøåñòâóþùèé ýëåìåíò ò.ê. åãî íåò
            temp->next = NULL;      //òî æå ñàìîå ñ ïîñëåäóþùèì ýëåìåíòîì
            pHead = temp;       //"ãîëîâà" óêàçûâàåò íà ñîçäàííûé ýëåìåíò
        }
        else                    //åñëè íå ïåðâûé, òî
            pPrev->next = temp;     //ïðåäûäóùèé óêàçûâàåò íà íåãî

        temp->data = data;          //êîïèðóåì äàííûå
        temp->next = NULL;  //ïîñëåäóþùåãî ýëåìåíòà íåò (äîáàâëÿåì æå â êîíåö)
        temp->pred = pPrev;     //óêàçûâàåì íà ïðåäûäóùèé ýëåìåíò, íà êîòîðûé «íàöåëåí» pPrev
        pPrev = temp;     //à òåïåðü õâîñòîâîé ýëåìåíò óêàçûâàåò íà ïîñëåäíèé (äîáàâëåííûé)
        countElem++;      //óâåëè÷èâàåì ñ÷åò÷èê ýëåìåíòîâ â ñïèñêå
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
    void delElem(int numb)          //ôóíêöèÿ óäàëåíèÿ ýëåìåíòà
    {
        Element<ElemType> *pTemp = pHead;     //ñîçäàåì âðåìåííûé ýëåìåíò


        if ((numb>countElem) || (numb<1))  {}  //åñëè óêàçàííûé ýëåìåíò íå ñóùåñòâóåò, òî
            //cout << "Takogo elementa net" << endl;  //âûâîäèì ïðåäóïðåæäåíèå íà ýêðàí
        else
        {
            for(int i=1; i!=numb; i++)  //èíà÷å, ïåðåõîäèì äî ýòîãî ýëåìåíòà
            {
                pTemp = pTemp->next;
            }

            if (pTemp->pred == NULL)        //åñëè óäàëÿåì ïåðâûé ýëåìåíò
            {
                if (countElem == 1)         //åñëè ýòîò ýëåìåíò åäèíñòâåííûé
                {
                    pHead = NULL;
                    pPrev = NULL;
                }
                else                //åñëè îí ïåðâûé, íî íå åäèíñòâåííûé
                {
                    pTemp->next->pred = NULL;
                    pHead = pTemp->next;
                }

                delete pTemp;
                countElem--;
                //cout << "Element " << numb << " udalen" << endl;
                return;
            }

            if (pTemp->next == NULL)        //åñëè óäàëÿåì ïîñëåäíèé ýëåìåíò, òî
            {
                pTemp->pred->next = NULL;   //ïðåäûäóùèé ýëåìåíò óêàçûâàåò íà NULL
                pPrev = pTemp->pred;        //óêàçàòåëü íà ïîñëåäíèé ýëåìåíò óêàçûâàåò íà ïðåäïîñëåäíèé

                delete pTemp;
                countElem--;
                //cout << "Element " << numb << " udalen" << endl;
                return;
            }

            //åñëè ýëåìåíò íàõîäèòñÿ â öåíòðå ñïèñêà
            if (pTemp->next != NULL && pTemp->pred != NULL)
            {
                pTemp->pred->next = pTemp->next; //ïðåäûäóùèé ýëåìåíò óêàçûâàåò íà ñëåäóþùèé
                pTemp->next->pred = pTemp->pred; //ñëåäóþùèé óêàçûâàåò íà ïðåäûäóùèé
                delete pTemp;
                countElem--;
                //cout << "Element " << numb << " udalen" << endl;
                return;
            }
        }
    }

    void clear()       //ðåàëèçàöèÿ óäàëåíèÿ âñåõ ýëåìåíòîâ ñïèñêà
    {
        while(pHead!=0)
        {
            ///*	pop_back();*/
            Element<ElemType> *pTemp = pHead; //ñîçäàåì âðåìåííûé ýëåìåíò

            pHead = pHead->next;        //ïðèñâàèâàåì åìó óêàçàòåëü íà ñëåäóþùèé
            if (pHead != NULL)
                delete pTemp;           // è óäàëÿåì åãî

        }

        pHead = NULL;
        pPrev = NULL;
    }

    void delRepeat()
    {
        Element<ElemType> *temp = pHead;
        Element<ElemType> *pTemp = pHead;
        int i = 1;

        while (i == 1)
        {
            if (pTemp->data == pTemp->next->data)
            {
                pTemp = temp = temp ->next;
                delElem(1);
            }
            else i++;

            pHead = temp;
        }
        //printlist();
        pTemp = pTemp->next;
        for(int i=2; i<=countElem; i++)
        {
            if (pTemp->data == pTemp->next->data)
            {


                temp = pTemp ->pred;
                delElem(i);
            }
            pTemp = temp = temp->next;
        }
    }
    int size ()
    {
        return countElem;
    }

    void merge(const list list1, const list list2)
    {
        Element<ElemType> *temp = list1.pHead; //ñîçäàåòñÿ âðåìåííûé ýëåìåíò



        for (int i=0; i<list1.countElem; i++)
        {
            this->addTolist(temp->data);
            temp=temp->next;
        }
        /*Element<ElemType> temp1;
        temp1.data=temp->data;
        temp1.pred=temp->pred;
            temp1.next=temp->next;*/


        temp=list2.pHead;
        /*	temp->pred=temp1.pred;*/
        for (int i=0; i<list2.countElem; i++)
        {
            this->addTolist(temp->data);
            temp=temp->next;
        }
        /*temp->pred->next=NULL;*/
        /*pPrev=temp;*/
        this->countElem=list1.countElem+list2.countElem;
    }
    void split(const list list, int n)
    {
        //assert(n>=1||n<=list.countElem);
        if (n==list.countElem)
        {
            this->clear();
            Element<ElemType> *temp=list.pHead;
            this->countElem=0;
            for (int i=0;i<list.countElem;i++)
            {
                this->push_back(temp->data);
                temp=temp->next;
            }
        }
        else if (n<list.countElem)
        {
            this->clear();
            Element<ElemType> *temp=list.pHead;
            for (int i=0; i<n; i++)
            {
                this->push_back(temp->data);
                temp=temp->next;
            }
            pPrev=temp;
            temp->next=NULL;

        }

    }


};
