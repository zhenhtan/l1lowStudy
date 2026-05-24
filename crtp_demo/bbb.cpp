#include<iostream>

template <typename Devidedclass>
class anamal_base
{
    public:
    void speaking()
    {
        static_cast<const Devidedclass*>(this)->speak_imp();
    }
};

class new_dog: public anamal_base<new_dog>
{
    public:
    void speak_imp() const
    {
        std::cout<<"dog speaking"<<std::endl;
    }
};


int main(void)
{
    new_dog aa;
    aa.speaking();
}