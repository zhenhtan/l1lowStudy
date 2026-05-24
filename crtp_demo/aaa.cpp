#include <iostream>
#include <string>
template <typename Devided>
class base
{ 
public:
     void speaking(){
    static_cast<Devided*>(this)-> speak_imp();
    }
};


class Animal_dog : public base<Animal_dog>
{
public:
   void speak_imp()
   {
       std::cout<<"dog speaking"<<std::endl;
   }

};

int main(void)
{
  Animal_dog aa;
  aa.speaking();
}