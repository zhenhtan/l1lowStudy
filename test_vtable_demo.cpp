#include <iostream>
 
using namespace std;
 
// 基类：包含多个虚函数
class Shape
{
public:
   virtual ~Shape() = default;
   virtual int getArea() = 0;
   virtual int getPerimeter() = 0;
   virtual void describe() = 0;
   virtual void rotate(int angle) = 0;
   
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
class Rectangle final : public Shape
{
public:
   int getArea() override
   {
      return (width * height);
   }
   int getPerimeter() override final { return 2 * (width + height); }
   void describe() override final { cout << "Rectangle\n"; }
   void rotate(int angle) override final { cout << "Rotate " << angle << " degrees\n"; }
};

class Triangle: public Shape
{
public:
   int getArea() override final
   {
      return (width * height)/2;
   }
   int getPerimeter() override { return width + height + (width + height); }
   void describe() override { cout << "Triangle\n"; }
   void rotate(int angle) override { cout << "Triangle rotate " << angle << "\n"; }
};

// 简单类，没有虚函数
class SimpleShape
{
public:
   int width;
   int height;
};

// 简单类，只有一个虚函数
class OneVirtual
{
public:
   virtual ~OneVirtual() = default;
   virtual void test() = 0;
   int width;
};
 
int main(void)
{
   Rectangle Rect;
   Triangle  Tri;
   SimpleShape simple;
   OneVirtual* one = nullptr;
 
   // 关键观察：虚函数数量多少，对象大小是一样的
   cout << "=== 对象大小对比 ===" << endl;
   cout << "sizeof(Rectangle): " << sizeof(Rect) << endl;
   cout << "sizeof(Triangle): " << sizeof(Tri) << endl;
   cout << "sizeof(SimpleShape): " << sizeof(simple) << endl;
   cout << "sizeof(OneVirtual): " << sizeof(OneVirtual) << endl;
   
   cout << "\n=== 功能演示 ===" << endl;
   Rect.setWidth(5);
   Rect.setHeight(7);
   cout << "Total Rectangle area: " << Rect.getArea() << endl;
   cout << "Rectangle perimeter: " << Rect.getPerimeter() << endl;
   Rect.describe();
   Rect.rotate(45);
 
   Tri.setWidth(5);
   Tri.setHeight(7);
   cout << "\nTotal Triangle area: " << Tri.getArea() << endl;
   cout << "Triangle perimeter: " << Tri.getPerimeter() << endl;
   Tri.describe();
   Tri.rotate(30);
 
   return 0;
}
