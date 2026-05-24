#include <iostream>
#include <dlfcn.h>
#include <cstdint>
#include <vector>

using namespace std;

static size_t countVtableFunctionSlots(const void* obj, size_t maxScan = 64)
{
   // This relies on the Itanium ABI layout used by GCC/Clang on Linux.
   // For single inheritance in this demo, function slots are contiguous and
   // end at the first null entry.
   auto vptr = *reinterpret_cast<const uintptr_t* const*>(obj);

   for (size_t i = 0; i < maxScan; ++i)
   {
      if (vptr[i] == 0)
      {
         return i;
      }
   }

   return maxScan;
}

static void printVtable(const char* name, const void* obj, size_t maxPrint = 16)
{
   auto vptr = *reinterpret_cast<const uintptr_t* const*>(obj);
   size_t slots = countVtableFunctionSlots(obj);

   cout << name << " vtable function slot count = " << slots << '\n';
   for (size_t i = 0; i < slots && i < maxPrint; ++i)
   {
      Dl_info info{};
      void* fn = reinterpret_cast<void*>(vptr[i]);
      if (dladdr(fn, &info) != 0 && info.dli_sname != nullptr)
      {
         cout << "  [" << i << "] " << fn << "  " << info.dli_sname << '\n';
      }
      else
      {
         cout << "  [" << i << "] " << fn << "\n";
      }
   }
}
 
// 基类
class Shape
{
public:
   virtual ~Shape()
   {
      std::cout << "~Shape\n";
   }
      virtual int getArea() = 0;
      virtual int getPerimeter() = 0;
      virtual void describe() = 0;
      virtual void rotate(int angle) = 0;
    //virtual ~Shape() = default;

   void setWidth(int w)
   {
      width = w;
   }
   void setHeight(int h)
   {
      height = h;
   }
protected:
   int width;
   int height;
};
 
// 派生类
// final 防止 Rectangle 继续被继承
class Rectangle final : public Shape
{
public:
   ~Rectangle() override
   {
      std::cout << "~Rectangle\n";
   }

   static void operator delete(void* p) noexcept
   {
      std::cout << "Rectangle::operator delete\n";
      ::operator delete(p);
   }

   int getArea() override
   {
      return (width * height);
   }
      int getPerimeter() override final { return 2 * (width + height); }
      void describe() override final { std::cout << "Rectangle\n"; }
      void rotate(int angle) override final { std::cout << "Rotate " << angle << " degrees\n"; }
};

// Triangle 可继承，但 getArea 标记 final 防止进一步重写
class Triangle: public Shape
{
public:
   ~Triangle() override
   {
      std::cout << "~Triangle\n";
   }

   static void operator delete(void* p) noexcept
   {
      std::cout << "Triangle::operator delete\n";
      ::operator delete(p);
   }

   int getArea() override final
   {
      return (width * height)/2;
   }
      int getPerimeter() override { return width + height + (width + height); }
      void describe() override { std::cout << "Triangle\n"; }
      void rotate(int angle) override { std::cout << "Triangle rotate " << angle << "\n"; }
};

static void demoCompleteDestructorOnly()
{
   std::cout << "\n[Demo-1] complete destructor only: p->~Shape()\n";
   void* mem = ::operator new(sizeof(Rectangle));
   Shape* p = new (mem) Rectangle();
   p->setWidth(3);
   p->setHeight(4);
   std::cout << "area=" << p->getArea() << '\n';

   // Virtual destructor dispatch, but no memory release here.
   p->~Shape();

   std::cout << "manual ::operator delete after explicit dtor\n";
   ::operator delete(mem);
}

static void demoDeletingDestructor()
{
   std::cout << "\n[Demo-2] deleting destructor: delete p\n";
   Shape* p = new Rectangle();
   p->setWidth(6);
   p->setHeight(7);
   std::cout << "area=" << p->getArea() << '\n';

   // Virtual deleting-destructor path: dtor + operator delete.
   delete p;
}
 
int main(void)
{
   Rectangle Rect;
   Triangle  Tri;
 
   cout<< sizeof(Rect) << endl;
   cout<< sizeof(Tri) << endl;
   Rect.setWidth(5);
   Rect.setHeight(7);
   // 输出对象的面积
   cout << "Total Rectangle area: " << Rect.getArea() << endl;
 
   Tri.setWidth(5);
   Tri.setHeight(7);
   // 输出对象的面积
   cout << "Total Triangle area: " << Tri.getArea() << endl;

   printVtable("Rectangle", &Rect);
   printVtable("Triangle", &Tri);

   demoCompleteDestructorOnly();
   demoDeletingDestructor();
   vector<int> v(10);
   v[1] = 42;
   cout << "v[1] = " << v[1] << " sizeof(v[1]) = " << v.size() << endl;
 
   return 0;
}
